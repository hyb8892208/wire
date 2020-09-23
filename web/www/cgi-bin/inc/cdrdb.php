<?php
class CDRDB extends SQLite3
{
	function __construct()
	{
		@$this->open('/data/log/cdr.db');
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
		@$this->try_exec('CREATE TABLE cdr ("id" INTEGER PRIMARY KEY, "callerid" VARCHAR(80), "calleeid" VARCHAR(80), "from" VARCHAR(80), "to" VARCHAR(80), "starttime" VARCHAR(32), "duration" INTEGER, "result" VARCHAR(32), "bank" VARCHAR(20), "slot" VARCHAR(20), "simnum" VARCHAR(32))');
	}

	public function create_grppolicy_table()
	{
		@$this->try_exec('CREATE TABLE grppolicy ("name" VARCHAR(80) PRIMARY KEY, "call_durations" INTEGER, "call_times" INTEGER)');
	}

	public function createindex()
	{
		$this->try_exec("CREATE INDEX cdr_idx ON cdr('callerid','calleeid','from','to','starttime','duration','result')");
	}

	public function insert_cdr($callerid, $calleeid, $from, $to, $starttime, $duration, $result)
	{
		@$this->try_exec("INSERT INTO cdr VALUES (NULL,'$callerid','$calleeid','$from','$to','$starttime','$duration','$result')");
	}

	public function checkdb()
	{
		$results = $this->try_query("select count(*) from cdr");
		if(!$results){
			exec("sqlite3 /data/log/cdr.db .dump > /tmp/cdr.sql");
			$content = file_get_contents("/tmp/cdr.sql");
			$content = str_replace('ROLLBACK','COMMIT',$content);
			file_put_contents("/tmp/cdr.sql",$content);
			exec("sqlite3 /data/log/cdr.db < /tmp/cdr.sql");
		}
		
		
		$results = @$this->try_query("select * from sqlite_master where tbl_name='cdr' and type='table';");
		if(!$results->fetchArray()) {
			$this->createdb();
		}

		$results = @$this->try_query("select * from sqlite_master where tbl_name='grppolicy' and type='table';");
		if(!$results->fetchArray()) {
			$this->create_grppolicy_table();
		}
		
		$this->try_exec("alter table cdr add bank verchar(20) default '0'");
		$this->try_exec("alter table cdr add slot verchar(20) default '0'");
		$this->try_exec("alter table cdr add simnum verchar(32) default ''");
		
		/*$results = @$this->query("select * from sqlite_master where name='cdr_idx' and type='index';");
		if(!$results->fetchArray()) {
			$this->createindex();
		}*/	
	}

	public function drop_alldata()
	{
		$this->try_exec("DROP TABLE cdr");
		$this->try_exec("DROP TABLE grppolicy");
		$this->try_exec("vacuum");
		$this->createdb();
		$this->create_grppolicy_table();
	}

	public function get_gsm_callduration_in($channel)
	{
		$results = @$this->try_query("select SUM(duration) from cdr where \"from\" = \"gsm-$channel\"");
		if($res = $results->fetchArray()) {
			if(isset($res[0])) {
				return trim($res[0]);
			}
		}

		return "0";
	}

	public function get_gsm_callduration_out($channel)
	{
		$results = @$this->try_query("select SUM(duration) from cdr where \"to\" = \"gsm-$channel\"");
		if($res = $results->fetchArray()) {
			if(isset($res[0])) {
				return trim($res[0]);
			}
		}

		return "0";
	}

	public function get_call_count_out($name)
	{
		$results = @$this->try_query("select COUNT(*) from cdr where \"to\" = \"$name\"");
		if($res = $results->fetchArray()) {
			if(isset($res[0])) {
				return trim($res[0]);
			}
		}

		return "0";
	}

	public function get_call_count_in($name)
	{
		$results = @$this->try_query("select COUNT(*) from cdr where \"from\" = \"$name\"");
		if($res = $results->fetchArray()) {
			if(isset($res[0])) {
				return trim($res[0]);
			}
		}

		return "0";
	}

	public function get_call_count_in_out($name)
	{
		$results = @$this->try_query("select COUNT(*) from cdr where \"to\" = \"$name\" or \"from\" = \"$name\"");
		if($res = $results->fetchArray()) {
			if(isset($res[0])) {
				return trim($res[0]);
			}
		}

		return "0";
	}

	public function get_callduration_out($name)
	{
		$results = @$this->try_query("select SUM(duration) from cdr where \"to\" = \"$name\"");
		if($res = $results->fetchArray()) {
			if(isset($res[0])) {
				return trim($res[0]);
			}
		}

		return "0";
	}

	public function get_callduration_in($name)
	{
		$results = @$this->try_query("select SUM(duration) from cdr where \"from\" = \"$name\"");
		if($res = $results->fetchArray()) {
			if(isset($res[0])) {
				return trim($res[0]);
			}
		}

		return "0";
	}

	public function get_callduration_in_out($name)
	{
		$results = @$this->try_query("select SUM(duration) from cdr where \"to\" = \"$name\" or \"from\" = \"$name\"");
		if($res = $results->fetchArray()) {
			if(isset($res[0])) {
				return trim($res[0]);
			}
		}

		return "0";
	}
};
?>
