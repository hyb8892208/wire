<?php

class Users extends SQLite3{
	
	/*
	* Coustructor
	*/
	function __construct(){
		$this->open("/data/info/user.db");
		
		$this->check_userdb();
		$this->check_recorddb();
	}
	
	private function try_exec($command,$times = 100){
		while(!($ret = $this->exec($command))){
			usleep(10000);//0.01s
			if($times-- <= 0) {
				break;
			}
		}
		
		return $ret;
	}
	
	private function try_query($command,$times = 100){
		while(!($ret = $this->query($command))) {
			usleep(10000);
			if($times-- <= 0)
				break;
		}

		return $ret;
	}
	
	/*
	* database param: id,username,password,ip_num,auth,create_time,super
	* auth: eg:{"system":[1,2,3,4],"view_system":[1,2,3,4]}  "view_" + module表示only view权限
	* super: 是否为超级管理员权限0/1
	*/
	private function create_userdb(){
		$sql = 'create table user (
					"id" INTEGER primary key autoincrement,
					"username" VARCHAR(32) not null default "",
					"password" VARCHAR(64) not null default "",
					"ip_num" SMALLINT,
					"auth" TEXT,
					"create_time" VARCHAR(32),
					"super" SMALLINT)';
		$this->try_exec($sql);
		
		//init data: insert Super Administrator
		$time = time();
		$sql1 = "insert into user (id,username,password,ip_num,auth,create_time,super) values (null,'khompOwner','c4ee5bcf9a01811c82b9f8fd29cd36d8','1','all','$time','1')";
		$sql2 = "insert into user (id,username,password,ip_num,auth,create_time,super) values (null,'admin','1b899cdf71210372546c8409ff716309','0','a:16:{s:6:\"system\";a:7:{i:0;s:1:\"1\";i:1;s:1:\"2\";i:2;s:1:\"3\";i:3;s:1:\"5\";i:4;s:1:\"6\";i:5;s:1:\"8\";i:6;s:1:\"9\";}s:11:\"view_system\";a:0:{}s:6:\"module\";a:6:{i:0;s:1:\"1\";i:1;s:1:\"2\";i:2;s:1:\"3\";i:3;s:1:\"5\";i:4;s:1:\"6\";i:5;s:1:\"9\";}s:11:\"view_module\";a:0:{}s:4:\"voip\";a:4:{i:0;s:1:\"1\";i:1;s:1:\"2\";i:2;s:1:\"3\";i:3;s:1:\"5\";}s:9:\"view_voip\";a:0:{}s:7:\"routing\";a:6:{i:0;s:1:\"1\";i:1;s:1:\"2\";i:2;s:1:\"3\";i:3;s:1:\"4\";i:4;s:1:\"5\";i:5;s:1:\"6\";}s:12:\"view_routing\";a:0:{}s:3:\"sms\";a:6:{i:0;s:1:\"1\";i:1;s:1:\"2\";i:2;s:1:\"3\";i:3;s:1:\"4\";i:4;s:1:\"5\";i:5;s:1:\"6\";}s:8:\"view_sms\";a:0:{}s:7:\"network\";a:6:{i:0;s:1:\"1\";i:1;s:1:\"3\";i:2;s:1:\"4\";i:3;s:1:\"5\";i:4;s:1:\"6\";i:5;s:1:\"7\";}s:12:\"view_network\";a:0:{}s:8:\"advanced\";a:3:{i:0;s:1:\"6\";i:1;s:1:\"7\";i:2;s:1:\"8\";}s:13:\"view_advanced\";a:0:{}s:4:\"logs\";a:5:{i:0;s:1:\"1\";i:1;s:1:\"2\";i:2;s:1:\"4\";i:3;s:1:\"8\";i:4;s:1:\"9\";}s:9:\"view_logs\";a:0:{}}','$time','0')";
		$this->try_exec($sql1);
		$this->try_exec($sql2);
	}
	
	/*
	* 恢复出厂设置重置用户
	*/
	public function factory_reset_userdb(){
		$sql = "drop table user";
		$this->try_exec($sql);
		
		$this->create_userdb();
	}
	
	/*
	* database param: id,user_id,ip,action,time
	*/
	private function create_recorddb(){
		$sql = 'create table record (
					"id" INTEGER primary key autoincrement,
					"user_id" INTEGER,
					"ip" VARCHAR(32),
					"action" VARCHAR(32),
					"time" VARCHAR(32))';
		$this->try_exec($sql);
	}
	
	//判断是否为超级管理员
	private function is_super_admin(){
		session_start();
		$username = $_SESSION['username'];
		
		$sql = "select super from user where username='".$username."'";
		$res = $this->try_query($sql);
		$info = $res->fetchArray();
		
		if($info['super'] == 1){
			return true;
		}else{
			return false;
		}
	}
	
	//判断是否有用户登录
	private function has_user_login(){
		session_start();
		if(!isset($_SESSION['username']) || $_SESSION['username'] == ""){
			return false;
		}else{
			return true;
		}
	}
	
	public function get_client_ip($type = 0,$client=true) {
        $type       =  $type ? 1 : 0;
        static $ip  =   NULL;
        if ($ip !== NULL) return $ip[$type];
        if($client){
            if (isset($_SERVER['HTTP_X_FORWARDED_FOR'])) {
                $arr    =   explode(',', $_SERVER['HTTP_X_FORWARDED_FOR']);
                $pos    =   array_search('unknown',$arr);
                if(false !== $pos) unset($arr[$pos]);
                $ip     =   trim($arr[0]);
            }elseif (isset($_SERVER['HTTP_CLIENT_IP'])) {
                $ip     =   $_SERVER['HTTP_CLIENT_IP'];
            }elseif (isset($_SERVER['REMOTE_ADDR'])) {
                $ip     =   $_SERVER['REMOTE_ADDR'];
            }
        }elseif (isset($_SERVER['REMOTE_ADDR'])) {
            $ip     =   $_SERVER['REMOTE_ADDR'];
        }
        // 防止IP伪造
        $long = sprintf("%u",ip2long($ip));
        $ip   = $long ? array($ip, $long) : array('0.0.0.0', 0);
        return $ip[$type];
    }
	
	/*
	* Get All User Data
	*/
	public function get_all_user_info(){
		$sql = "select * from user";
		return $this->try_query($sql);
	}
	
	/*
	* Get All Username Data
	*/
	public function get_all_username(){
		$sql = "select username from user";
		return $this->try_query($sql);
	}
	
	/*
	* Get One User By ID
	*/
	public function get_one_user_by_id($id){
		$sql = "select * from user where id='$id'";
		return $this->try_query($sql);
	}
	
	/*
	* Get One User By Username
	*/
	public function get_one_user_by_username($username){
		$sql = "select * from user where username='$username'";
		return $this->try_query($sql);
	}
	
	/*
	* Get All Record Data
	*/
	public function get_all_record_data(){
		//超级管理员使用的接口
		if($this->is_super_admin()){
			$sql = "select * from record";
			return $this->try_query($sql);
		}else{
			return false;
		}
	}
	
	/*
	* Get Record Data By User_ID
	*/
	public function get_record_by_userid($userid,$start_record,$line_counts){
		//超级管理员使用的接口
		if($this->is_super_admin()){
			$sql = "select * from record where user_id = ".$userid." order by id desc limit $start_record,$line_counts";
			return $this->try_query($sql);
		}else{
			return false;
		}
	}
	
	/*
	* Get Record count(*) By User_ID
	*/
	public function get_record_count_by_userid($userid){
		//超级管理员使用的接口
		if($this->is_super_admin()){
			$sql = "select count(*) from record where user_id = ".$userid;
			return $this->try_query($sql);
		}else{
			return false;
		}
	}
	
	/*
	* database param: id,username,password,ip_num,auth,create_time,super
	*/
	public function insert_user($username,$password,$ip_num,$auth){
		//超级管理员使用的接口
		if($this->is_super_admin()){
			$create_time = time();
			$super = 0;
			
			$sql = "insert into user (id,username,password,ip_num,auth,create_time,super) values (null,'$username','$password','$ip_num','$auth','$create_time','$super')";
			$this->try_exec($sql);
		}else{
			return false;
		}
	}
	
	/*
	* Update User By ID
	*/
	public function update_user($id,$username,$password,$ip_num,$auth){
		//有用户登录的才能使用
		if($this->has_user_login()){
			$sql = "update user set username='$username',password='$password',ip_num='$ip_num',auth='$auth' where id='$id'";
			$this->try_exec($sql);
		}
	}
	
	/*
	* Update Username and Password
	*/
	public function update_username_and_password($id,$username,$password){
		//有用户登录的才能使用
		if($this->has_user_login()){
			$sql = "update user set username='$username',password='$password' where id='$id'";
			$this->try_exec($sql);
		}
	}
	
	
	/*
	* Delete User
	*/
	public function delete_user($id){
		//超级管理员使用的接口
		if($this->is_super_admin()){
			$sql = "delete from user where id=$id";
			
			$this->try_exec($sql);
		}else{
			return false;
		}
	}
	
	/*
	* database param: id,user_id,ip,action,time
	*/
	public function insert_record($user_id,$ip,$action){
		$time = time();
		$sql = "insert into record (id,user_id,ip,action,time) values (null,'$user_id','$ip','$action','$time')";
		$this->try_exec($sql);
	}
	
	/*
	* check user table
	*/
	public function check_userdb(){
		$results = $this->try_query("select * from sqlite_master where tbl_name='user' and type='table';");
		if(!$results->fetchArray()) {
			$this->create_userdb();
		}
	}
	
	/*
	* check record table
	*/
	public function check_recorddb(){
		$results = $this->try_query("select * from sqlite_master where tbl_name='record' and type='table';");
		if(!$results->fetchArray()) {
			$this->create_recorddb();
		}
	}
	
}