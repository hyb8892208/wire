<?php

header('Content-type: text/html;charset=utf-8'); 
header("Cache-Control: no-cache, must-revalidate"); // HTTP/1.1
header("Expires: Mon, 26 Jul 1997 05:00:00 GMT"); // Date in the past

include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/language.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
include_once("/www/cgi-bin/inc/redis.inc");
include_once("/www/cgi-bin/inc/aql.php");
include_once("/www/cgi-bin/php/wizard/Functions.php");
include_once("/www/cgi-bin/inc/cdrdb.php");
include_once("/www/cgi-bin/inc/userdb.php");
//initialize the language
$language = get_web_language_cache('/tmp/web/language.cache');

function dowmload_link_file(){
	$store_file = $_GET['store_file'];
	$url = $_POST['url'];
	
	$remote_fh = @fopen ($url, "rb");
	$store_fh = @fopen ($store_file, "wb");
	
	$downlen = 0;
	if ($store_fh){
		while(!feof($remote_fh)) {
			$data=fread($remote_fh, 1024 * 8 );
			if($data==false){
				break;
			}else{
				$downlen += strlen($data);
				fwrite($store_fh, $data, 1024 * 8 );
			}
		}
		fclose($store_fh);
	}else{
		echo language('System Online Update Download error','Download system file failed. Please check the network connection!');
	}
}

function get_system_update_progress(){
	$store_file = $_GET['store_file'];
	
	$data = file_get_contents($store_file);
	$filesize = strlen($data);
	echo $filesize;
}

function update_cfg_file(){
	$store_file = $_GET['store_file'];
	$url = $_POST['url'];
	
	exec("rm -rf /tmp/update_conf_file");
	exec("mkdir /tmp/update_conf_file");
	
	if(strstr($url, 'tar.gz') || strstr($url, '.bin')){
		$cmd = "tar vxz -f $store_file -C /tmp/update_conf_file"; 
	}else if(strstr($url, 'zip')){
		$cmd = "unzip $store_file -d /tmp/update_conf_file";
	}else{
		$cmd = "tar vxf $store_file -C /tmp/update_conf_file";
	}
	exec("$cmd > /dev/null 2>&1 || echo $?",$output);
	
	exec("find /dev/shm/update_conf_file/ -name cfg_version", $output_version);
	$cfg_version = $output_version[0];
	
	if(file_exists($cfg_version)){
		$version=file_get_contents($cfg_version);
		if($version == "") {
			$error_reports = language("Configuration Files Upload version error 2","</br>Unknown configuration version!");
			file_put_contents('/tmp/schedule_conf.log',$error_reports,FILE_APPEND);
			
			exec("rm -rf /tmp/update_conf_file");
			exec("rm $store_file -rf > /dev/null 2>&1");
			echo $error_reports;
			return false;
		}

		$cmd = "( cat /version/cfg_ver_list 2> /dev/null | grep \"$version\" > /dev/null 2>&1 )";
		exec("$cmd || echo $?",$output);
		if($output) {
			$error_reports = language("Configuration Files Upload version error 3","</br>Your current system does not support this configuration version!</br>");
			file_put_contents('/tmp/schedule_conf.log',$error_reports,FILE_APPEND);
			
			exec("rm -rf /tmp/update_conf_file");
			exec("rm $store_file -rf > /dev/null 2>&1");
			echo $error_reports;
			return false;
		}
	}else if(strstr($url,'tar') || strstr($url,'zip')){
		exec("find /dev/shm/update_conf_file/", $output_sourcefiles);
		exec("/etc/init.d/asterisk stop > /dev/null 2>&1");
		for($i=0;$i<count($output_sourcefiles);$i++){
			if(strstr($output_sourcefiles[$i],'.')){
				$temp = explode('/',$output_sourcefiles[$i]);
				
				$length = count($temp);
				$file_name = $temp[$length-1];
				
				unset($output_destination_file);
				exec("find /etc/asterisk/ -name $file_name",$output_destination_file);
				
				if($output_destination_file[0]){
					copy($output_sourcefiles[$i],$output_destination_file[0]);
				}
			}
		}
		exec("/etc/init.d/asterisk start > /dev/null 2>&1");
		
		exec("rm -rf /tmp/update_conf_file");
		exec("rm $store_file -rf > /dev/null 2>&1");
		echo 'success';
		
		return false;
	}else{
		$error_reports = language("Configuration Files Upload version error 1","</br>Can not find configration file!</br>Can not find asterisk/cfg_version!<br>");
		file_put_contents('/tmp/schedule_conf.log',$error_reports,FILE_APPEND);
		
		exec("rm -rf /tmp/update_conf_file");
		exec("rm $store_file -rf > /dev/null 2>&1");
		echo $error_reports;
		return false;
	}

	exec("/etc/init.d/asterisk stop > /dev/null 2>&1");
	exec("rm -rf /etc/cfg/*");
	exec("rm -rf /etc/asterisk/*");
	
	exec("find /dev/shm/update_conf_file/ -name cfg",$output_cfg);
	$cfg_path = $output_cfg[0];
	if($cfg_path == ''){
		$cfg_path = '/dev/shm/update_conf_file/';
	}
	exec("cp $cfg_path/* /etc/cfg/ -a");
	exec("cp /etc/cfg/* /etc/asterisk/ -a");
	
	exec("/etc/init.d/asterisk start > /dev/null 2>&1");

	save_user_record("","System->File Link Update Configuration");

	exec("rm -rf /tmp/update_conf_file");
	exec("rm $store_file -rf > /dev/null 2>&1");
	echo 'success';
}

if(isset($_GET['type']) && $_GET['type']) {
	switch($_GET['type']) {
		case 'get_system_update_progress' :
			get_system_update_progress();
			break;
		case 'dowmload_link_file':
			dowmload_link_file();
			break;
		case 'update_cfg_file':
			update_cfg_file();
			break;
		default :
			echo "";
			break;
	}
}

exit(0);
?>