<?php
include_once("/www/cgi-bin/inc/function.inc");
require_once("/www/cgi-bin/inc/redis.inc");

class SMSOUTBOXDB_CACHE extends SQLite3
{
	function __construct()
	{
		$this->open('/var/log/smsoutbox.db');
	}

	public function try_exec($command,$times = 100)
	{
		while(!@$this->exec($command)) {
			usleep(10000);
			if($times-- <= 0) {
				break;
			}
		}
	}

	public function try_query($command,$times = 100)
	{
		while(!($ret = @$this->query($command))) {
			usleep(10000);
			if($times-- <= 0)
				break;
		}

		return $ret;
	}

	public function createdb()
	{
		$this->try_exec('CREATE TABLE sms_out ("id" INTEGER PRIMARY KEY AUTOINCREMENT, "port" VARCHAR(32), "phonenumber" VARCHAR(80), "time" VARCHAR(32), "mr" INT1, "message" TEXT, "status" CHAR(1), "uuid" CHAR(32), "imsi" CHAR(32), "cause" VARCHAR(32))');
	}
	
	public function create_report_db(){
		$this->try_exec('CREATE TABLE sms_out_report ("id" INTEGER PRIMARY KEY AUTOINCREMENT, "sms_report_port" VARCHAR(32), "sms_report_sender" VARCHAR(80), "status" VARCHAR(32), "cause" VARCHAR(32))');
	}

	public function insert_sms($port, $phonenumber, $time, $message, $status, $uuid, $imsi, $cause)
	{
		$message = str_replace("'","''",$message);
		$this->try_exec("INSERT INTO sms_out VALUES (NULL,'$port','$phonenumber','$time',0,'$message', $status, '$uuid', '$imsi', '$cause')");
	}
	
	public function insert_sms_report($sms_report_port, $sms_report_sender, $status, $cause){
		$this->try_exec("INSERT INTO sms_out_report VALUES (NULL,'$sms_report_port','$sms_report_sender','$status','$cause')");
	}

	public function checkdb()
	{
		$results = $this->try_query("select count(*) from sms_out");
		if(!$results){
			exec("sqlite3 /var/log/smsoutbox.db .dump > /tmp/var_smsoutbox.sql");
			$content = file_get_contents("/tmp/var_smsoutbox.sql");
			$content = str_replace('ROLLBACK', 'COMMIT', $content);
			file_put_contents("/tmp/var_smsoutbox.sql", $content);
			exec("sqlite3 /var/log/smsoutbox.db < /tmp/var_smsoutbox.sql");
		}
		
		$results = $this->try_query("select * from sqlite_master where tbl_name='sms_out' and type='table';");
		if(!$results->fetchArray()) {
			$this->createdb();
		}
		
		$this->try_exec("alter table sms_out add cause verchar(32) default ''");
	}
	
	public function check_report_db(){
		$results = $this->try_query("select count(*) from sms_out");
		if(!$results){
			exec("sqlite3 /data/log/smsoutbox.db .dump > /tmp/report_smsoutbox.sql");
			$content = file_get_contents("/tmp/report_smsoutbox.sql");
			$content = str_replace('ROLLBACK', 'COMMIT', $content);
			file_put_contents("/tmp/report_smsoutbox.sql", $content);
			exec("sqlite3 /data/log/smsoutbox.db < /tmp/report_smsoutbox.sql");
		}
		
		$results = $this->try_query("select * from sqlite_master where tbl_name='sms_out_report' and type='table';");
		if(!$results->fetchArray()) {
			$this->create_report_db();
		}
		
		$this->try_exec("alter table sms_out_report add status verchar(32) default ''");
		$this->try_exec("alter table sms_out_report add cause verchar(32) default ''");
	}

	public function drop_alldata()
	{
		$this->try_exec("DROP TABLE sms_out");
		$this->try_exec("vacuum");
		$this->createdb();
		$this->checkdb();
	}
};


class SMSOUTBOXDB extends SQLite3
{
	protected $cache_outboxdb; 		//发出短信的缓存数据库

	function __construct()
	{
		$this->open('/data/log/smsoutbox.db');
		$this->cache_outboxdb = new SMSOUTBOXDB_CACHE();
	}

	function __destruct()
	{
		$this->cache_outboxdb->close();  //关闭发送短信的缓存数据库连接
	}

	public function try_exec($command,$times = 100)
	{
		while(!@$this->exec($command)) {
			usleep(10000);
			if($times-- <= 0) {
				break;
			}
		}
	}

	public function try_query($command,$times = 100)
	{
		while(!($ret = @$this->query($command))) {
			usleep(10000);
			if($times-- <= 0)
				break;
		}

		return $ret;
	}

	public function createdb()
	{
		$this->try_exec('CREATE TABLE sms_out ("id" INTEGER PRIMARY KEY AUTOINCREMENT, "port" VARCHAR(32), "phonenumber" VARCHAR(80), "time" VARCHAR(32), "mr" INT1, "message" TEXT, "status" CHAR(1), "uuid" CHAR(32), "imsi" CHAR(32), "cause" VARCHAR(32))');
		$this->cache_outboxdb->createdb();
	}

	public function insert_sms($port, $phonenumber, $time, $message, $status, $uuid, $imsi, $cause)
	{
		$message = str_replace("'","''",$message);
		$this->cache_outboxdb->try_exec("INSERT INTO sms_out VALUES (NULL,'$port','$phonenumber','$time',0,'$message', $status, '$uuid', '$imsi', '$cause')");
	}

	public function update($sql)
	{
		$this->cache_outboxdb->try_query($sql);
	}
	public function checkdb()
	{
		$results = $this->try_query("select count(*) from sms_out");
		if(!$results){
			exec("sqlite3 /data/log/smsoutbox.db .dump > /tmp/smsoutbox.sql");
			$content = file_get_contents("/tmp/smsoutbox.sql");
			$content = str_replace('ROLLBACK', 'COMMIT', $content);
			file_put_contents("/tmp/smsoutbox.sql", $content);
			exec("sqlite3 /data/log/smsoutbox.db < /tmp/smsoutbox.sql");
		}
		
		$results = $this->query("select * from sqlite_master where tbl_name='sms_out' and type='table';");
		if(!$results->fetchArray()) {
			$this->createdb();
		}

		$results = $this->query("select * from sms_out order by uuid;");
		if(!$results){
			$this->try_exec('ALTER TABLE sms_out  ADD COLUMN  "uuid" VARCHAR(32)');
			$this->cache_outboxdb->try_exec('ALTER TABLE sms_out  ADD COLUMN  "uuid" VARCHAR(32)'); 
		}

		$results = $this->query("select * from sms_out order by imsi;");                                                                                                                                 
                if(!$results){                                                                                                                                                                                   
                        $this->try_exec('ALTER TABLE sms_out  ADD COLUMN  "imsi" VARCHAR(32)');                                                                                                                  
                        $this->cache_outboxdb->try_exec('ALTER TABLE sms_out  ADD COLUMN  "imsi" VARCHAR(32)');                                                                                                  
                }
				
		$this->try_exec("alter table sms_out add cause verchar(32) default ''");
		
		$this->cache_outboxdb->checkdb();
		$this->cache_outboxdb->check_report_db();
	}

	public function drop_alldata()
	{
		$this->try_exec("DROP TABLE sms_out");
		$this->try_exec("vacuum");
		$this->createdb();
		$this->checkdb();

		$this->cache_outboxdb->drop_alldata();
	}

	public function check_cache_DBtable(){
		$result = $this->cache_outboxdb->query("select * from sms_out");
		if(!$result->fetchArray()){
			return false;
		} else {
			return true;
		}
	} 

	public function clean_cache_DBtable(){
		$this->cache_outboxdb->exec("DELETE FROM sms_out;");
	}

	public function sync_cache(){
		$result = $this->cache_outboxdb->try_query("select * from sms_out");
		
		if($result != false){
			$flock = lock_file("/data/log/smsoutbox.db");
			$this->exec("BEGIN");
			$id = 0;
			
			while($res = $result->fetchArray()){
				$id = $res['id'];
				$port = $res['port'];
				$phonenumber = $res['phonenumber'];
				$time = $res['time'];
				$message = $res['message'];
				$status = $res['status'];
				$uuid = $res['uuid'];
				$imsi = $res['imsi'];
				$cause = $res['cause'];
				
				$message = str_replace("'","''",$message);
				$sql = "INSERT INTO sms_out VALUES (NULL,'$port','$phonenumber','$time',0,'$message', $status, '$uuid', '$imsi', '$cause')";
				$this->exec($sql);
			}
			
			$this->exec("COMMIT");
			unlock_file($flock);
			
			$var_flock = lock_file("/var/log/smsoutbox.db");
			$this->cache_outboxdb->exec("DELETE FROM sms_out where id <= '$id'");
			unlock_file($var_flock);
		}			
		
		//sms report sync to flash
		$this->cache_outboxdb->check_report_db();
		$report_res = $this->cache_outboxdb->try_query("select * from sms_out_report");
		
		if($report_res != false){
			$flock = lock_file("/data/log/smsoutbox.db");
			$this->exec("BEGIN");
			$id = 0;
			
			$redis_client = new Predis\Client();
			
			$arr = [];//push to app.sms.dbreportstohttp.list
			while($rp_res = $report_res->fetchArray()){
				$id = $rp_res['id'];
				$sms_report_port = $rp_res['sms_report_port'];
				$sms_report_sender = $rp_res['sms_report_sender'];
				$status_temp = $rp_res['status'];
				$cause = $rp_res['cause'];
				
				if($status_temp == 'FAIL'){
					$status = '0';
					$status_msg = "Message%20not%20received%20on%20handset";
				}else if($status_temp == 'SUCCESS'){
					$status = '2';
					$status_msg = "Message%20received%20on%20handset";
				}
				
				$sql = "UPDATE sms_out SET status = '$status',cause = '$cause' where id = (select id from sms_out where port = '" . $sms_report_port . "' AND phonenumber like '%" . $sms_report_sender . "' AND status = '1' ORDER BY id DESC limit 1)";
				$this->exec($sql);
				
				$temp_arr = [$sms_report_port, $sms_report_sender];
				array_push($arr, $temp_arr);
			}
			
			$this->exec("COMMIT");
			unlock_file($flock);
			
			$var_flock = lock_file("/var/log/smsoutbox.db");
			$this->cache_outboxdb->exec("DELETE FROM sms_out_report where id <= '$id'");
			unlock_file($var_flock);
			
			//redis app.sms.dbreportstohttp.list
			for($i=0;$i<count($arr);$i++){
				$sms_report_port = $arr[$i][0];
				$sms_report_sender = $arr[$i][1];
				
				$smsout_res = $this->try_query("select * from sms_out where id = (select id from sms_out where port = '" . $sms_report_port . "' AND phonenumber like '%" . $sms_report_sender . "' ORDER BY id DESC limit 1)");
				$smsout_result = $smsout_res->fetchArray();
				
				$port = $smsout_result['port'];
				$phonenumber = $smsout_result['phonenumber'];
				$time = $smsout_result['time'];
				$time = str_replace("-","/",$time);
				$time = str_replace(" ","%20",$time);
				$uuid = $smsout_result['uuid'];
				$imsi = $smsout_result['imsi'];
				$cause = $smsout_result['cause'];
				$redis_str = "{\"port\":\"$port\",\"phonenumber\":\"$phonenumber\",\"time\":\"$time\",\"message\":\"\",\"status\":\"$status_msg\",\"type\":\"SMS-Status-Report\",\"imsi\":\"$imsi\",\"uuid\":\"$uuid\",\"cause\":\"$cause\"}";
			//	$redis_client->rpush("app.sms.dbreportstohttp.list",$redis_str);
			}
		}
		/*
		if($this->check_cache_DBtable()){
			// Getting the starting id number
			$result = $this->cache_outboxdb->query("select id from sms_out order by id asc limit 0,1;");
			$temp = $result->fetchArray();
			$id_start = $temp['id'];
			
			// Getting the ending id number
			$result = $this->cache_outboxdb->query("select id from sms_out order by id desc limit 0,1;");
			$temp = $result->fetchArray();
			$id_end = $temp['id'];
			$sql = '';
			
			$this->exec("BEGIN");
			for($i = $id_start; $i <= $id_end; $i++){
				$query_result = $this->cache_outboxdb->query("select * from sms_out where id=$i;");
				$smsoutbox = $query_result->fetchArray();
				$port = $smsoutbox['port'];
				$phonenumber = $smsoutbox['phonenumber'];
				$time = $smsoutbox['time'];
				$message = $smsoutbox['message'];
				$status = $smsoutbox['status'];
				$uuid = $smsoutbox['uuid'];
				$imsi = $smsoutbox['imsi'];
				if($port == '' ||  $phonenumber == '' || $time == '' || $message == '') {
					continue;
				}
				$message = str_replace("'","''",$message);
				$sql = "INSERT INTO sms_out VALUES (NULL,'$port','$phonenumber','$time',0,'$message', $status, '$uuid', 'imsi')";
				$this->exec($sql);
			} 
			$result = $this->exec("COMMIT");
			if($result){
				$this->clean_cache_DBtable();
			}
		}*/	
	}
};
?>
