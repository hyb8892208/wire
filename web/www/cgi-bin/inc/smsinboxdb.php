<?php
include_once("/www/cgi-bin/inc/function.inc");

class SMSINBOXDB_CACHE extends SQLite3
{
	function __construct()
	{
		$this->open('/var/log/smsinbox.db');
	}
	
	public function try_exec($command,$times = 100)
	{
		while(!@$this->exec($command)) {
			usleep(10000);
			if($times-- <= 0)
				break;
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
		$this->try_exec('CREATE TABLE sms ("id" INTEGER PRIMARY KEY, "port" VARCHAR(32), "phonenumber" VARCHAR(80), "time" VARCHAR(32), "message" TEXT, "smsc" VARCHAR(32))');
	}

	public function insert_sms($port, $phonenumber, $time, $message, $smsc)
	{
		$message = str_replace("'","''",$message);
		$this->try_exec("INSERT INTO sms VALUES (NULL,'$port','$phonenumber','$time','$message','$smsc')");
	}

	public function checkdb()
	{
		$results = $this->try_query("select count(*) from sms");
		if(!$results){
			exec("sqlite3 /var/log/smsinbox.db .dump > /tmp/var_smsinbox.sql");
			$content = file_get_contents("/tmp/var_smsinbox.sql");
			$content = str_replace('ROLLBACK', 'COMMIT', $content);
			file_put_contents("/tmp/var_smsinbox.sql", $content);
			exec("sqlite3 /var/log/smsinbox.db < /tmp/var_smsinbox.sql");
		}
		
		$results = $this->try_query("select * from sqlite_master where tbl_name='sms' and type='table';");
		if(!$results->fetchArray()) {
			$this->createdb();
		}
		
		$this->try_exec("alter table sms add smsc verchar(32) default ''");
	}

	public function drop_alldata()
	{
		$this->try_exec("DROP TABLE sms");
		$this->try_exec("vacuum");
		$this->createdb();
		$this->checkdb();
	}
}


class SMSINBOXDB extends SQLite3
{
	protected $db_cache; // 接收短信的缓存数据库
	
	function __construct()
	{
		$this->open('/data/log/smsinbox.db');
		$this->db_cache = new SMSINBOXDB_CACHE(); // 在构造函数创建接收短信的缓存数据库
	}
	
	function __destruct() 
	{
       $this->db_cache->close(); // 在析构函数close缓存数据库
    }

	public function try_exec($command,$times = 100)
	{
		while(!@$this->exec($command)) {
			usleep(10000);
			if($times-- <= 0)
				break;
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
		// 也要创建缓存的表
		$this->try_exec('CREATE TABLE sms ("id" INTEGER PRIMARY KEY, "port" VARCHAR(32), "phonenumber" VARCHAR(80), "time" VARCHAR(32), "message" TEXT, "smsc" VARCHAR(32))');
		$this->db_cache->createdb();
	}

	public function insert_sms($port, $phonenumber, $time, $message, $smsc)
	{
		// 插入到缓存
		$message = str_replace("'","''",$message);
		$this->db_cache->try_exec("INSERT INTO sms VALUES (NULL,'$port','$phonenumber','$time','$message','$smsc')");
	}

	public function checkdb()
	{
		$results = $this->try_query("select count(*) from sms");
		if(!$results){
			exec("sqlite3 /data/log/smsinbox.db .dump > /tmp/smsinbox.sql");
			$content = file_get_contents("/tmp/smsinbox.sql");
			$content = str_replace('ROLLBACK', 'COMMIT', $content);
			file_put_contents("/tmp/smsinbox.sql", $content);
			exec("sqlite3 /data/log/smsinbox.db < /tmp/smsinbox.sql");
		}
		
		$times = 100;
		while(!($ret = @$this->query("select * from sqlite_master where tbl_name='sms' and type='table';"))) {
			usleep(10000);
			if($times-- <= 0)
				break;
		}
		
		if(!$ret->fetchArray()) {
			$this->createdb();
		}
		
		$this->try_exec("alter table sms add smsc verchar(32) default ''");
		
		// 缓存也一样的操作
		$this->db_cache->checkdb();
	}

	public function drop_alldata()
	{
		// 缓存一样的操作
		$this->db_cache->drop_alldata();
		
		$this->try_exec("DROP TABLE sms");
		$this->try_exec("vacuum");
		$this->createdb();
		$this->checkdb();
	}

	public function check_cache_DBtable(){
		$result = @$this->db_cache->query("select * from sms");
		if(!$result->fetchArray()){
			return false;
		} else {
			return true;
		}
	}

	public function clean_cache_DBtable(){
		$this->db_cache->exec("DELETE FROM sms;");
	}

	public function sync_cache(){
		$result = @$this->db_cache->try_query("select * from sms");
		
		if($result != false){
			$flock = lock_file("/data/log/smsinbox.db");
			$this->exec("BEGIN");
			$id = 0;
			while($res = $result->fetchArray()){
				$id = $res['id'];
				$port = $res['port'];
				$phonenumber = $res['phonenumber'];
				$time = $res['time'];
				$message = $res['message'];
				$smsc = $res['smsc'];
				$message = str_replace("'","''",$message);
				$sql = "INSERT INTO sms VALUES (NULL, '$port', '$phonenumber', '$time', '$message', '$smsc')";
				$this->exec($sql);
			}
			
			$var_flock = lock_file("var/log/smsinbox.db");
			@$this->db_cache->exec("DELETE FROM sms where id <= '$id'");
			unlock_file($var_flock);
			
			$this->exec("COMMIT");
			unlock_file($flock);
		}
		
		/*
		if($this->check_cache_DBtable()){
			// 把缓存的内容同步过来， 这里定时调用，关机前要先调用
			$result = @$this->db_cache->query("select id from sms order by id desc limit 0,1;");
			$temp = $result->fetchArray();
			$id = $temp['id'];
			$sql = '';
			
			$this->exec("BEGIN");
			for($i = 1; $i <= $id; $i++){
				$query_result = @$this->db_cache->query("select * from sms where id=$i;");
				$smsinbox = $query_result->fetchArray();
				$port = $smsinbox[1];
				$phonenumber = $smsinbox[2];
				$time = $smsinbox[3];
				$message = $smsinbox[4];
				$message = str_replace("'","''",$message);
				$sql = "INSERT INTO sms VALUES (NULL, '$port', '$phonenumber', '$time', '$message')";
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
