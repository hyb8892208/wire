<?php
require_once("/www/cgi-bin/inc/define.inc");
require_once("/www/cgi-bin/inc/function.inc");

//AQL
require_once("/www/cgi-bin/inc/aql.php");

function slave_process_log($host, $log_type, $action_type, $size, $port)
{
	$user = get_web_user();
	$slaveport = get_slave_httpport();

	$url  = "http://$host/cgi-bin/php/ajax_server.php?type=log&log_type=$log_type&board=1&port=$port&size=$size&action_type=$action_type";

	$ch = curl_init();

	curl_setopt($ch, CURLOPT_URL, $url );
	//curl_setopt($ch, CURLOPT_HEADER, 1 );
	//curl_setopt($ch, CURLOPT_PORT, $_SERVER['SERVER_PORT']);
	curl_setopt($ch, CURLOPT_PORT, $slaveport);
	curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
	curl_setopt($ch, CURLOPT_USERPWD, $user['name'].':'.$user['password']);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_HTTPHEADER, array('Expect:'));//For Lighttpd 2013-12-24 09:18
	$buf = curl_exec( $ch );

	return $buf;
}

function update_slave_emu($host, $file_path, $port='')
{
	$user = get_web_user();

	$file = $file_path;
	$url  = "http://$host/cgi-bin/php/system-tools.php";//target url

	//Freedom 2013-12-12 10:59
	//PHP-5.3.10
	//$fields['update_sys_file'] = '@'.$file;

	//PHP-5.5.6
	$fields['update_emu_file'] = curl_file_create($file);

	$fields['send'] = 'Emu Update';

	$port = ($port == '') ? $_SERVER['SERVER_PORT'] : $port;

	$ch = curl_init();

	curl_setopt($ch, CURLOPT_URL, $url );
	curl_setopt($ch, CURLOPT_PORT, $port);
	curl_setopt($ch, CURLOPT_POST, 1 );
	curl_setopt($ch, CURLOPT_POSTFIELDS, $fields );
	curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	curl_setopt($ch, CURLOPT_USERPWD, $user['name'].':'.$user['password']);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_HTTPHEADER, array('Expect:'));//For Lighttpd 2013-12-24 09:18
	$buf = curl_exec( $ch );
	
	//echo "FErom=".$buf;
	
	if(strstr($buf,"Successfully update your Emu!")) {
		$ret = true;
	} else {
		$ret = false;
	}

	if ($error = curl_error($ch) ) {
		curl_close($ch);
		//die($error.'<br>');
		echo 'Update '.$host.' failed('.$error.')<br>';
		return false;
	}

	curl_close($ch);
	return $ret;
}



function update_slave($host, $file_path, $port='')
{
	$user = get_web_user();
	$slaveport = get_slave_httpport();

	$file = $file_path;
	$url  = "http://$host/cgi-bin/php/system-tools.php";//target url

	//Freedom 2013-12-12 10:59
	//PHP-5.3.10
	//$fields['update_sys_file'] = '@'.$file;

	//PHP-5.5.6
	$fields['update_sys_file'] = curl_file_create($file);

	$fields['send'] = 'System Update';

	$port = ($port == '') ? $slaveport : $port;

	$ch = curl_init();

	curl_setopt($ch, CURLOPT_URL, $url );
	curl_setopt($ch, CURLOPT_PORT, $port);
	curl_setopt($ch, CURLOPT_POST, 1 );
	curl_setopt($ch, CURLOPT_POSTFIELDS, $fields );
	curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
	curl_setopt($ch, CURLOPT_USERPWD, $user['name'].':'.$user['password']);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_HTTPHEADER, array('Expect:'));//For Lighttpd 2013-12-24 09:18
	$buf = curl_exec( $ch );

	if(strstr($buf,"Successfully update your system!")) {
		$ret = true;
	} else {
		$ret = false;
	}

	if ($error = curl_error($ch) ) {
		curl_close($ch);
		//die($error.'<br>');
		echo 'Update '.$host.' failed('.$error.')<br>';
		return false;
	}

	curl_close($ch);
	return $ret;
}


function request_slave($ip, $data, $timeout, $ret=true)
{
	$socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	if(!$socket) return false;

	@socket_set_option($socket,SOL_SOCKET,SO_RCVTIMEO,array("sec"=>1, "usec"=>0 ) );
	@socket_set_option($socket,SOL_SOCKET,SO_SNDTIMEO,array("sec"=>1, "usec"=>0 ) );
	$connection = @socket_connect($socket, $ip, 8000);
	if(!$connection) return false;

	@socket_set_option($socket,SOL_SOCKET,SO_RCVTIMEO,array("sec"=>$timeout, "usec"=>0 ) );
	@socket_set_option($socket,SOL_SOCKET,SO_SNDTIMEO,array("sec"=>$timeout, "usec"=>0 ) );

	@socket_write($socket, $data);

	if($ret) {
		$output = '';
		while ($buffer = @socket_read($socket, 1024, PHP_NORMAL_READ)) {
			$output .= $buffer;
		}
		if($output) {
			@socket_close($socket);
			return $output;
		}
	}

	@socket_close($socket);

	return true;
}

/*
function get_slave_file($slaveip,$filepath)
{
	$filename = basename($filepath);
	exec("tftpc $slaveip -i /tmp/$slaveip-$filename -g $filepath > /dev/null 2>&1 || echo $?",$output);
	if($output) {
		return false;
	}

	return true;
}


function set_slave_file($slaveip,$local_filepath,$remote_filepath)
{
	exec("tftpc $slaveip -i $remote_filepath -p $local_filepath > /dev/null 2>&1 || echo $?",$output);
	if($output) {
		return false;
	}

	return true;
}
*/

/*
function get_slave_file($slaveip, $filepath)
{
	$url  = "http://$slaveip/cgi-bin/php/file.php";//target url
	$data['action'] = 'download';
	$data['file_path'] = $filepath;

	$filename = basename($filepath);
	$user = get_web_user();

	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL, $url );
	curl_setopt($ch, CURLOPT_PORT, $_SERVER['SERVER_PORT']);
	curl_setopt($ch, CURLOPT_POST, 1 );
	curl_setopt($ch, CURLOPT_POSTFIELDS, $data );
	curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
	curl_setopt($ch, CURLOPT_USERPWD, $user['name'].':'.$user['password']);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_HTTPHEADER, array('Expect:'));//For Lighttpd 2013-12-24 09:18
	$buf = curl_exec( $ch );

	file_put_contents("/tmp/$slaveip-$filename",$buf);
}

function set_slave_file($slaveip,$local_filepath,$remote_filepath)
{
	$url  = "http://$slaveip/cgi-bin/php/file.php";//target url

	//Freedom 2013-12-12 10:59
	//PHP-5.3.10
	//$data['update_sys_file'] = '@'.$local_filepath;

	//PHP-5.5.6
	$data['remote_file'] = curl_file_create($local_filepath);
	$data['action'] = 'upload';
	$data['file_path'] = $remote_filepath;
	$user = get_web_user();

	$ch = curl_init();

	curl_setopt($ch, CURLOPT_URL, $url );
	curl_setopt($ch, CURLOPT_PORT, $_SERVER['SERVER_PORT']);
	curl_setopt($ch, CURLOPT_POST, 1 );
	curl_setopt($ch, CURLOPT_POSTFIELDS, $data );
	curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
	curl_setopt($ch, CURLOPT_USERPWD, $user['name'].':'.$user['password']);
	curl_setopt($ch, CURLOPT_HTTPHEADER, array('Expect:'));//For Lighttpd 2013-12-24 09:18
	//curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_exec( $ch );
}
*/
function get_slave_file_contents($slaveip, $filepath)
{
//get file 
// 1. client --> server 
//    "getfile:filepath\n" 
// 2. server --> client 
//    "filesize:filesize\n" or "error\n" ..... 
// 3. client --> server 
//    "ready\n" or "error\n" ..... 
// 4. server --> client 
//    read server file send to client 
// 5. client --> server 
//    "over\n" or "error\n" ..... 
// 6. server --> client 
//    "ok\n" or "error\n" .....


	$contents = '';

	if(($socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP)) === false){
		return '';
	}

	socket_set_nonblock($socket);
	$error = NULL;
	$attempts = 0;
	$timeout = 10;  // adjust because we sleeping in 1 millisecond increments
	$connected = false;
	while (!($connected = @socket_connect($socket, $slaveip, 8000)) && ($attempts++ < $timeout)) {
		$error = socket_last_error();
		if ($error != SOCKET_EINPROGRESS && $error != SOCKET_EALREADY) {
			socket_close($socket);
			return '';
		}
		//echo "$slaveip:".socket_strerror($error)."<br>";
		usleep(1000);
	}
	if($connected == false){
		socket_close($socket);
		return '';
	}
	socket_set_block($socket);

	@socket_set_option($socket,SOL_SOCKET,SO_RCVTIMEO,array("sec"=>5, "usec"=>0 ) ); 
	@socket_set_option($socket,SOL_SOCKET,SO_SNDTIMEO,array("sec"=>5, "usec"=>0 ) );

	// 1. client --server "getfile:filepath\n"--> server
	if(@socket_write($socket, "getfile:$filepath\n") === false){
		@socket_close($socket);
		return '';
	}

	// 2. server --"filesize:filesize\n"--> client
	$filesize = 0;
	$buffer = @socket_read($socket, 1024);
	sscanf($buffer,"filesize:%d\n",$filesize);
	if(!is_numeric($filesize) || $filesize <= 0 || $filesize > 128*1024*1024){
		@socket_write($socket, "error\n");
		@socket_close($socket);
		return '';
	}
	
	// 3. client --"ready\n" or "error"--> server
	if(@socket_write($socket, "ready\n") === false){
		@socket_close($socket);
		return '';
	}
		
	// 4. server --file--> client
	while ($filesize > 0){
		if(($buffer = @socket_read($socket, 1024)) === false) {
			@socket_write($socket, "error\n");
			@socket_close($socket);
			return '';
		}
		$contents .= $buffer;
		$filesize -= strlen($buffer);
	}
	
	if($filesize != 0) {
		@socket_write($socket, "error\n");
		@socket_close($socket);
		return '';
	}
	
	
	// 5. client --"over\n" or "error\n"--> server	
	if(@socket_write($socket, "over\n") === false){
		@socket_close($socket);
		return '';
	}

	// 6. server --"ok\n" or "error\n"--> client
	if(($buffer = @socket_read($socket, 1024)) !== "ok\n"){
		@socket_close($socket);
		return '';
	}

	@socket_close($socket);

	return $contents;
}

function get_slave_file($slaveip, $filepath)
{

	$contents = get_slave_file_contents($slaveip, $filepath);
	
	$filename = basename($filepath);
	$savepath = "/tmp/$slaveip-$filename";
	if(file_put_contents($savepath,$contents,LOCK_EX) === false){
		return false;
	}


	return $savepath;
}

function set_slave_file($slaveip,$local_filepath,$remote_filepath)
{
//put file 
// 1. client --> server 
//    "putfile:filepath:filesize\n" 
// 2. server --> client 
//    "ready\n" or "error" ......
// 3. client --> server 
//    read client file send to server 
// 4. server --> client 
//    "ok\n" or "error\n" ...... 

	if(!is_file($local_filepath) || !$slaveip || !$local_filepath){ 
		return false; 
	}

	$contents = '';

	if(($socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP)) === false) {
		return false;
	}

	socket_set_nonblock($socket);
	$error = NULL;
	$attempts = 0;
	$timeout = 10;  // adjust because we sleeping in 1 millisecond increments
	$connected = false;
	while (!($connected = @socket_connect($socket, $slaveip, 8000)) && ($attempts++ < $timeout)) {
		$error = socket_last_error();
		if ($error != SOCKET_EINPROGRESS && $error != SOCKET_EALREADY) {
			socket_close($socket);
			return NULL;
		}
		//echo "$slaveip:".socket_strerror($error)."<br>";
		usleep(1000);
	}
	if($connected == false){
		//insert_warning_html($slaveip.' '.language('connect failed').'<br>');
		socket_close($socket);
		return false;
	}
	socket_set_block($socket);

	@socket_set_option($socket,SOL_SOCKET,SO_RCVTIMEO,array("sec"=>5, "usec"=>0 ) ); 
	@socket_set_option($socket,SOL_SOCKET,SO_SNDTIMEO,array("sec"=>5, "usec"=>0 ) );

	// 1. client --"putfile:filepath:filesize\n"--> server
	$data = "putfile:$remote_filepath:".filesize($local_filepath)."\n";
	if(@socket_write($socket, $data) === false){
		@socket_close($socket);
		return false;
	}

	// 2. server --"ready\n" or "error"--> client
	if(($buffer = @socket_read($socket, 1024)) !== "ready\n"){
		@socket_close($socket);
		return false;
	}
	
	// 3. client --file--> server
	if(($fp = fopen($local_filepath,'r')) === false){
		@socket_close($socket);
		return false;
	}

	while(!feof($fp)){
		if(($contents = fread($fp, 1024)) !== false) {
			if(@socket_write($socket, $contents) === false){
				@socket_close($socket);
				fclose($fp);
				return false;
			}
		}
	}

	fclose($fp);

	// 4. server --"ok\n" or "error\n"--> client
	if(($buffer = @socket_read($socket, 1024)) === "ok\n"){
		@socket_close($socket);
		return true;
	} else {
		@socket_close($socket);
		return false;
	}
}


function getClusterInfo($url)
{
	$user = get_web_user();
	$slaveport = get_slave_httpport();
	$ch = curl_init();			//初始化curl
	curl_setopt($ch, CURLOPT_URL, $url );
	//curl_setopt($ch, CURLOPT_PORT, $_SERVER['SERVER_PORT']);
	curl_setopt($ch, CURLOPT_PORT, $slaveport);
	curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
	curl_setopt($ch, CURLOPT_USERPWD, $user['name'].':'.$user['password']);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);     //设置是否返回信息
	curl_setopt($ch, CURLOPT_HTTPHEADER, array('Expect:'));//For Lighttpd 2013-12-24 09:18
	$buf = curl_exec( $ch );
	curl_close($ch);
	return $buf;
}

?>
