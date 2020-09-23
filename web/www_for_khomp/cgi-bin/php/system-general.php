<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/network_factory.inc");
require_once("/www/cgi-bin/inc/aql.php");
?>

<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
 
<!---// load the iButton CSS stylesheet //---> 
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />

<?php
//AQL
require_once("/www/cgi-bin/inc/aql.php");
?>

<?php

function save_autoreboot()
{
/* 
/etc/asterisk/gw/autoreboot.conf 
[general]
reboot_sw=off
reboot_type=by_day/by_week/by_month/by_run_time

[by_day]
d_hour=
d_minute=

[by_week]
w_week=
w_hour=
w_minute=

[by_month]
m_month=
m_hour=
m_minute=

[by_run_time]
r_day=
r_hour=
r_minute=

*/

	if(isset($_POST['reboot_sw'])) {
		$reboot_sw = 'on';
	} else {
		$reboot_sw = 'off';
	}

	if(isset($_POST['reboot_type'])) {
		$reboot_type = trim($_POST['reboot_type']);
	} else {
		$reboot_type = '';
	}
	
	/* by day */
	if(isset($_POST['d_minute'])) {
		$d_minute = trim($_POST['d_minute']);
	} else {
		$d_minute = '*';
	}

	if(isset($_POST['d_hour'])) {
		$d_hour = trim($_POST['d_hour']);
	} else {
		$d_hour = '*';
	}

	/* by week */
	if(isset($_POST['w_minute'])) {
		$w_minute = trim($_POST['w_minute']);
	} else {
		$w_minute = '';
	}

	if(isset($_POST['w_hour'])) {
		$w_hour = trim($_POST['w_hour']);
	} else {
		$w_hour = '';
	}

	if(isset($_POST['w_week'])) {
		$w_week = trim($_POST['w_week']);
	} else {
		$w_week = '';
	}

	/* by month */
	if(isset($_POST['m_minute'])) {
		$m_minute = trim($_POST['m_minute']);
	} else {
		$m_minute = '';
	}

	if(isset($_POST['m_hour'])) {
		$m_hour = trim($_POST['m_hour']);
	} else {
		$m_hour = '';
	}

	if(isset($_POST['m_month'])) {
		$m_month = trim($_POST['m_month']);
	} else {
		$m_month = '';
	}

	/* by run time */
	if(isset($_POST['r_hour'])) {
		$r_hour = trim($_POST['r_hour']);
	} else {
		$r_hour = '';
	}

	//Save to "/etc/asterisk/gw/autoreboot.conf"
	///////////////////////////////////////////////////////////////////////
	$write = "[general]\n";
	$write .= "reboot_sw=$reboot_sw\n";
	$write .= "reboot_type=$reboot_type\n";
	
	$write .= "[by_day]\n";
	$write .= "d_minute=$d_minute\n";
	$write .= "d_hour=$d_hour\n";

	$write .= "[by_week]\n";
	$write .= "w_minute=$w_minute\n";
	$write .= "w_hour=$w_hour\n";
	$write .= "w_week=$w_week\n";

	$write .= "[by_month]\n";
	$write .= "m_minute=$m_minute\n";
	$write .= "m_hour=$m_hour\n";
	$write .= "m_month=$m_month\n";

	$write .= "[by_run_time]\n";
	$write .= "r_hour=$r_hour\n";

	$file_path = "/etc/asterisk/gw/autoreboot.conf";	
	$hlock=lock_file($file_path);
	$fh = fopen($file_path,"w");
	fwrite($fh,$write);
	fclose($fh);
	unlock_file($hlock);
	///////////////////////////////////////////////////////////////////////


	//Save to "/etc/crontabs/root"
	///////////////////////////////////////////////////////////////////////
	/* 
format: 
minute hour day month week exec_file
	*/
	$write = '';
	$safe_reboot_tools = 'root /my_tools/safe_reboot';

	if(is_true($reboot_sw)) {
		switch ($reboot_type) {
		case 'by_day':
			$write = "$d_minute $d_hour * * * $safe_reboot_tools";
			break;
		case 'by_week':
			$write = "$w_minute $w_hour * * $w_week $safe_reboot_tools";
			break;
		case 'by_month':
			$write = "$m_minute $m_hour $m_month * * $safe_reboot_tools";
			break;
		case 'by_run_time':
			$r_minute = trim(`date "+%M"`);

			if($r_hour == 0)
				$r_hour = "*";
			else
				$r_hour = "*/$r_hour";

			$r_day = "*";

			$write = "$r_minute $r_hour $r_day * * $safe_reboot_tools";
			break;
		}
	}

	$file_path = "/etc/asterisk/gw/crontabs_root";
	$hlock=lock_file($file_path);
	exec("sed -i '/\/my_tools\/safe_reboot/d' \"$file_path\" 2> /dev/null");
	if($write != '') exec("echo \"$write\" >> $file_path");
	unlock_file($hlock);
	///////////////////////////////////////////////////////////////////////
	//exec("/etc/init.d/cron restart > /dev/null 2>&1");
}

function save_firm_autoupdate(){
	$aql = new aql();
	$conf_path = '/etc/asterisk/gw/autoreboot.conf';
	$hlock = lock_file($conf_path);
	$aql->set('basedir','/etc/asterisk/gw');
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	
	$res = $aql->query("select * from autoreboot.conf");
	
	if(!isset($res['firmware_update'])){
		$aql->assign_addsection('firmware_update','');
	}
	
	if(isset($_POST['firm_reboot_sw']) && $_POST['firm_reboot_sw'] == 'on'){
		$firm_reboot_sw = 'on';
	}else{
		$firm_reboot_sw = 'off';
	}
	if(isset($res['firmware_update']['firm_reboot_sw'])){
		$aql->assign_editkey('firmware_update','firm_reboot_sw',$firm_reboot_sw);
	}else{
		$aql->assign_append('firmware_update','firm_reboot_sw',$firm_reboot_sw);
	}
	
	$firm_reboot_type = '';
	if(isset($_POST['firm_reboot_type'])){
		$firm_reboot_type = $_POST['firm_reboot_type'];
	}
	if(isset($res['firmware_update']['firm_reboot_type'])){
		$aql->assign_editkey('firmware_update','firm_reboot_type',$firm_reboot_type);
	}else{
		$aql->assign_append('firmware_update','firm_reboot_type',$firm_reboot_type);
	}
	
	$firm_d_minute = '';
	if(isset($_POST['firm_d_minute'])){
		$firm_d_minute = $_POST['firm_d_minute'];
	}
	if(isset($res['firmware_update']['firm_d_minute'])){
		$aql->assign_editkey('firmware_update','firm_d_minute',$firm_d_minute);
	}else{
		$aql->assign_append('firmware_update','firm_d_minute',$firm_d_minute);
	}
	
	$firm_d_hour = '';
	if(isset($_POST['firm_d_hour'])){
		$firm_d_hour = $_POST['firm_d_hour'];
	}
	if(isset($res['firmware_update']['firm_d_hour'])){
		$aql->assign_editkey('firmware_update','firm_d_hour',$firm_d_hour);
	}else{
		$aql->assign_append('firmware_update','firm_d_hour',$firm_d_hour);
	}
	
	$firm_w_minute = '';
	if(isset($_POST['firm_w_minute'])){
		$firm_w_minute = $_POST['firm_w_minute'];
	}
	if(isset($res['firmware_update']['firm_w_minute'])){
		$aql->assign_editkey('firmware_update','firm_w_minute',$firm_w_minute);
	}else{
		$aql->assign_append('firmware_update','firm_w_minute',$firm_w_minute);
	}
	
	$firm_w_hour = '';
	if(isset($_POST['firm_w_hour'])){
		$firm_w_hour = $_POST['firm_w_hour'];
	}
	if(isset($res['firmware_update']['firm_w_hour'])){
		$aql->assign_editkey('firmware_update','firm_w_hour',$firm_w_hour);
	}else{
		$aql->assign_append('firmware_update','firm_w_hour',$firm_w_hour);
	}
	
	$firm_w_week = '';
	if(isset($_POST['firm_w_week'])){
		$firm_w_week = $_POST['firm_w_week'];
	}
	if(isset($res['firmware_update']['firm_w_week'])){
		$aql->assign_editkey('firmware_update','firm_w_week',$firm_w_week);
	}else{
		$aql->assign_append('firmware_update','firm_w_week',$firm_w_week);
	}
	
	$firm_m_minute = '';
	if(isset($_POST['firm_m_minute'])){
		$firm_m_minute = $_POST['firm_m_minute'];
	}
	if(isset($res['firmware_update']['firm_m_minute'])){
		$aql->assign_editkey('firmware_update','firm_m_minute',$firm_m_minute);
	}else{
		$aql->assign_append('firmware_update','firm_m_minute',$firm_m_minute);
	}
	
	$firm_m_hour = '';
	if(isset($_POST['firm_m_hour'])){
		$firm_m_hour = $_POST['firm_m_hour'];
	}
	if(isset($res['firmware_update']['firm_m_hour'])){
		$aql->assign_editkey('firmware_update','firm_m_hour',$firm_m_hour);
	}else{
		$aql->assign_append('firmware_update','firm_m_hour',$firm_m_hour);
	}
	
	$firm_m_month = '';
	if(isset($_POST['firm_m_month'])){
		$firm_m_month = $_POST['firm_m_month'];
	}
	if(isset($res['firmware_update']['firm_m_month'])){
		$aql->assign_editkey('firmware_update','firm_m_month',$firm_m_month);
	}else{
		$aql->assign_append('firmware_update','firm_m_month',$firm_m_month);
	}
	
	$firm_r_hour = '';
	if(isset($_POST['firm_r_hour'])){
		$firm_r_hour = $_POST['firm_r_hour'];
	}
	if(isset($res['firmware_update']['firm_r_hour'])){
		$aql->assign_editkey('firmware_update','firm_r_hour',$firm_r_hour);
	}else{
		$aql->assign_append('firmware_update','firm_r_hour',$firm_r_hour);
	}
	
	$firm_r_minute = '';
	if(isset($_POST['firm_r_minute'])){
		$firm_r_minute = $_POST['firm_r_minute'];
	}
	if(isset($res['firmware_update']['firm_r_minute'])){
		$aql->assign_editkey('firmware_update','firm_r_minute',$firm_r_minute);
	}else{
		$aql->assign_append('firmware_update','firm_r_minute',$firm_r_minute);
	}
	
	$firm_link = '';
	if(isset($_POST['firm_link'])){
		$firm_link = urldecode($_POST['firm_link']);
	}
	if(isset($res['firmware_update']['firm_link'])){
		$aql->assign_editkey('firmware_update','firm_link',$firm_link);
	}else{
		$aql->assign_append('firmware_update','firm_link',$firm_link);
	}
	
	if(!$aql->save_config_file('autoreboot.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);

	//crontabs_root
	$write = '';
	$safe_reboot_tools = "root /my_tools/scheduled_firmware_update \\\"$firm_link\\\"";

	if(is_true($firm_reboot_sw)) {
		switch ($firm_reboot_type) {
		case 'by_day':
			$write = "$firm_d_minute $firm_d_hour * * * $safe_reboot_tools";
			break;
		case 'by_week':
			$write = "$firm_w_minute $firm_w_hour * * $firm_w_week $safe_reboot_tools";
			break;
		case 'by_month':
			$write = "$firm_m_minute $firm_m_hour $firm_m_month * * $safe_reboot_tools";
			break;
		case 'by_run_time':

			if($firm_r_hour == 0){
				$firm_r_hour = "*";
				$firm_r_minute = "*/$firm_r_minute";
			}else{
				$firm_r_hour = "*/$firm_r_hour";
			}

			$firm_r_day = "*";

			$write = "$firm_r_minute $firm_r_hour $firm_r_day * * $safe_reboot_tools";
			break;
		}
	}

	$file_path = "/etc/asterisk/gw/crontabs_root";
	$hlock=lock_file($file_path);
	exec("sed -i '/\/my_tools\/scheduled_firmware_update/d' \"$file_path\" 2> /dev/null");
	if($write != '') exec("echo \"$write\" >> $file_path");
}

function save_conf_autoupdate(){
	$aql = new aql();
	$conf_path = '/etc/asterisk/gw/autoreboot.conf';
	$hlock = lock_file($conf_path);
	$aql->set('basedir','/etc/asterisk/gw');
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	
	$res = $aql->query("select * from autoreboot.conf");
	
	if(!isset($res['conf_update'])){
		$aql->assign_addsection('conf_update','');
	}
	
	if(isset($_POST['conf_reboot_sw']) && $_POST['conf_reboot_sw'] == 'on'){
		$conf_reboot_sw = 'on';
	}else{
		$conf_reboot_sw = 'off';
	}
	if(isset($res['conf_update']['conf_reboot_sw'])){
		$aql->assign_editkey('conf_update','conf_reboot_sw',$conf_reboot_sw);
	}else{
		$aql->assign_append('conf_update','conf_reboot_sw',$conf_reboot_sw);
	}
	
	$conf_reboot_type = '';
	if(isset($_POST['conf_reboot_type'])){
		$conf_reboot_type = $_POST['conf_reboot_type'];
	}
	if(isset($res['conf_update']['conf_reboot_type'])){
		$aql->assign_editkey('conf_update','conf_reboot_type',$conf_reboot_type);
	}else{
		$aql->assign_append('conf_update','conf_reboot_type',$conf_reboot_type);
	}
	
	$conf_d_minute = '';
	if(isset($_POST['conf_d_minute'])){
		$conf_d_minute = $_POST['conf_d_minute'];
	}
	if(isset($res['conf_update']['conf_d_minute'])){
		$aql->assign_editkey('conf_update','conf_d_minute',$conf_d_minute);
	}else{
		$aql->assign_append('conf_update','conf_d_minute',$conf_d_minute);
	}
	
	$conf_d_hour = '';
	if(isset($_POST['conf_d_hour'])){
		$conf_d_hour = $_POST['conf_d_hour'];
	}
	if(isset($res['conf_update']['conf_d_hour'])){
		$aql->assign_editkey('conf_update','conf_d_hour',$conf_d_hour);
	}else{
		$aql->assign_append('conf_update','conf_d_hour',$conf_d_hour);
	}
	
	$conf_w_minute = '';
	if(isset($_POST['conf_w_minute'])){
		$conf_w_minute = $_POST['conf_w_minute'];
	}
	if(isset($res['conf_update']['conf_w_minute'])){
		$aql->assign_editkey('conf_update','conf_w_minute',$conf_w_minute);
	}else{
		$aql->assign_append('conf_update','conf_w_minute',$conf_w_minute);
	}
	
	$conf_w_hour = '';
	if(isset($_POST['conf_w_hour'])){
		$conf_w_hour = $_POST['conf_w_hour'];
	}
	if(isset($res['conf_update']['conf_w_hour'])){
		$aql->assign_editkey('conf_update','conf_w_hour',$conf_w_hour);
	}else{
		$aql->assign_append('conf_update','conf_w_hour',$conf_w_hour);
	}
	
	$conf_w_week = '';
	if(isset($_POST['conf_w_week'])){
		$conf_w_week = $_POST['conf_w_week'];
	}
	if(isset($res['conf_update']['conf_w_week'])){
		$aql->assign_editkey('conf_update','conf_w_week',$conf_w_week);
	}else{
		$aql->assign_append('conf_update','conf_w_week',$conf_w_week);
	}
	
	$conf_m_minute = '';
	if(isset($_POST['conf_m_minute'])){
		$conf_m_minute = $_POST['conf_m_minute'];
	}
	if(isset($res['conf_update']['conf_m_minute'])){
		$aql->assign_editkey('conf_update','conf_m_minute',$conf_m_minute);
	}else{
		$aql->assign_append('conf_update','conf_m_minute',$conf_m_minute);
	}
	
	$conf_m_hour = '';
	if(isset($_POST['conf_m_hour'])){
		$conf_m_hour = $_POST['conf_m_hour'];
	}
	if(isset($res['conf_update']['conf_m_hour'])){
		$aql->assign_editkey('conf_update','conf_m_hour',$conf_m_hour);
	}else{
		$aql->assign_append('conf_update','conf_m_hour',$conf_m_hour);
	}
	
	$conf_m_month = '';
	if(isset($_POST['conf_m_month'])){
		$conf_m_month = $_POST['conf_m_month'];
	}
	if(isset($res['conf_update']['conf_m_month'])){
		$aql->assign_editkey('conf_update','conf_m_month',$conf_m_month);
	}else{
		$aql->assign_append('conf_update','conf_m_month',$conf_m_month);
	}
	
	$conf_r_hour = '';
	if(isset($_POST['conf_r_hour'])){
		$conf_r_hour = $_POST['conf_r_hour'];
	}
	if(isset($res['conf_update']['conf_r_hour'])){
		$aql->assign_editkey('conf_update','conf_r_hour',$conf_r_hour);
	}else{
		$aql->assign_append('conf_update','conf_r_hour',$conf_r_hour);
	}
	
	$conf_r_minute = '';
	if(isset($_POST['conf_r_minute'])){
		$conf_r_minute = $_POST['conf_r_minute'];
	}
	if(isset($res['conf_update']['conf_r_minute'])){
		$aql->assign_editkey('conf_update','conf_r_minute',$conf_r_minute);
	}else{
		$aql->assign_append('conf_update','conf_r_minute',$conf_r_minute);
	}
	
	$file_link = '';
	if(isset($_POST['file_link'])){
		$file_link = urldecode($_POST['file_link']);
	}
	if(isset($res['conf_update']['file_link'])){
		$aql->assign_editkey('conf_update','file_link',$file_link);
	}else{
		$aql->assign_append('conf_update','file_link',$file_link);
	}
	
	if(!$aql->save_config_file('autoreboot.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);

	//crontabs_root
	$write = '';
	$safe_reboot_tools = "root /my_tools/scheduled_conf_update \\\"$file_link\\\"";

	if(is_true($conf_reboot_sw)) {
		switch ($conf_reboot_type) {
		case 'by_day':
			$write = "$conf_d_minute $conf_d_hour * * * $safe_reboot_tools";
			break;
		case 'by_week':
			$write = "$conf_w_minute $conf_w_hour * * $conf_w_week $safe_reboot_tools";
			break;
		case 'by_month':
			$write = "$conf_m_minute $conf_m_hour $conf_m_month * * $safe_reboot_tools";
			break;
		case 'by_run_time':
			if($conf_r_hour == 0){
				$conf_r_hour = "*";
				$conf_r_minute = "*/$conf_r_minute";
			}else{
				$conf_r_hour = "*/$conf_r_hour";
			}

			$conf_r_day = "*";

			$write = "$conf_r_minute $conf_r_hour $conf_r_day * * $safe_reboot_tools";
			break;
		}
	}

	$file_path = "/etc/asterisk/gw/crontabs_root";
	$hlock=lock_file($file_path);
	exec("sed -i '/\/my_tools\/scheduled_conf_update/d' \"$file_path\" 2> /dev/null");
	if($write != '') exec("echo \"$write\" >> $file_path");
}

function save_web_language_conf()
{
/*----------------
[general]
language=chinese

[list]
english=English
chinese=汉语
portuguese=Português	
----------------*/
	$conf_file = "/etc/asterisk/gw/web_language.conf";
	if(isset($_POST['language_type'])){
		$conf_array['general']['language'] = $_POST['language_type'];
		return modify_conf($conf_file, $conf_array);
	}else{
		return false;
	}
}

function download_language(&$alert)
{
	if(!isset($_POST['language_type']) || $_POST['language_type'] == ''){
		return false;
	}
	$language_type = $_POST['language_type'];
	if(is_file('/www/lang/'.$language_type))
		$package = '/www/lang/'.$language_type;
	else if(is_file('/etc/asterisk/gw/web_language/'.$language_type))
		$package = '/etc/asterisk/gw/web_language/'.$language_type;
	else
		$package = '';

	if(!file_exists($package)) {
		$alert = language('Language Download error','Can not find Your select language package.');
		return false;
	}

	$file = fopen ($package, 'r');
	$size = filesize($package) ;

	header('Content-Encoding: none');
	header('Content-Type: application/force-download');
	header('Content-Type: application/octet-stream');
	header('Content-Type: application/download');
	header('Content-Description: File Transfer');
	header('Accept-Ranges: bytes');
	header("Accept-Length: $size");
	header('Content-Transfer-Encoding: binary' );
	header("Content-Disposition: attachment; filename=$language_type");
	header('Pragma: no-cache');
	header('Expires: 0');
	ob_clean();
	flush();
	echo fread($file, $size);
	fclose ($file);
	exit(0);

	return true;
}

function delete_language()
{
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $__deal_cluster__;

	if(isset($_POST['language_type']) && $_POST['language_type'] != ''){
		$language_type = $_POST['language_type'];
		$conf_file = "/etc/asterisk/gw/web_language.conf";
		$conf_array = get_conf($conf_file);

		/* check the delete language is current using language */
		if(is_file('/etc/asterisk/gw/web_language/'.$language_type)){
			/* 1.Modify language conf */
			delete_conf($conf_file, 'list',$language_type);
			/* 2.Delete language package */
			unlink('/etc/asterisk/gw/web_language/'.$language_type);
			/* 3.Check language setting */
			if(isset($conf_array['general']['language']) &&  strcmp($conf_array['general']['language'],$language_type)==0){
				$conf_new['general']['language']='english';
				modify_conf($conf_file,$conf_new);
				wait_apply("exec", "/my_tools/web_language_init >/dev/null 2>&1 &");
			}
			
			save_user_record("","SYSTEM->General:Delete Language ".$language_type);
		}else if(is_file('/www/lang/'.$language_type)){
			/* 1.Modify language conf */
			delete_conf($conf_file, 'list',$language_type);
			/* 2.Delete language package */
			unlink('/www/lang/'.$language_type);
			/* 3.Check language setting */
			if(isset($conf_array['general']['language']) &&  strcmp($conf_array['general']['language'],$language_type)==0){
				$conf_new['general']['language']='english';
				modify_conf($conf_file,$conf_new);
				wait_apply("exec", "/my_tools/web_language_init >/dev/null 2>&1 &");
			}
			
			save_user_record("","SYSTEM->General:Delete Language ".$language_type);
		}
	}
}

function store_language($store_file, &$alert)
{
	if(!$_FILES) {
		return;
	}   

	if(isset($_FILES['upload_lang_file']['error']) && $_FILES['upload_lang_file']['error'] == 0) {  //Update successful
		if(!(isset($_FILES['upload_lang_file']['size'])) || $_FILES['upload_lang_file']['size'] > 1*1000*1000) { //Max file size 1Mbyte
			echo language('Language Package Upload Filesize error',"Your uploaded file was larger than 1MB!<br>Uploading language package was failed.<br>");
			return false;
		}   

		if (!move_uploaded_file($_FILES['upload_lang_file']['tmp_name'], $store_file)) {   
			echo language('Language Package Upload Move Failed');  
			return false;
		}
	}else{
		echo language("Language Package Upload Failed");
		return false;
	}
	return true;
}

function add_language(&$alert, &$confirm, $unstore=true)
{
/*
Language Package first line format:
------------------------------
language#chinese#中文
...
------------------------------

*/
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $__deal_cluster__;

	$store_file = "/tmp/web/new_language";
	if($unstore){
		if(!store_language($store_file, $alert)){
			return false;
		}
	}

	/* check package */
	$info = '';
	if(is_language($store_file,$info)){
		$language_key = $info['key'];
		$language_value = $info['value'];
	}else{
		echo language('Language Package Format error');
		return false;
	}

	/* check whether language exists */
	$conf_file = "/etc/asterisk/gw/web_language.conf";
	$conf_array = get_conf($conf_file);
	if($unstore){
		if(isset($conf_array['list']) && is_array($conf_array['list'])){
			foreach($conf_array['list'] as $key => $value){
				if(strcmp($key,$language_key) == 0 || strcmp($value,$language_value) == 0){
					if(is_file('/www/lang/'.$language_key)){
						$alert = language('Add Language overwrite alert','Language already exists!\nReadyonly Language cannot be overwrite!');
						return false;
					}else if(is_file('/etc/asterisk/gw/web_language/'.$language_key)){
						$confirm = language('Add Language overwrite confirm','Language already exists!\nDo you want to overwrite it?');
						return false;
					}
				}
			}
		}
	}
	/* copy language package from "/tmp/web/new_language" to "/etc/asterisk/gw/web_language/x" */
	if(!copy($store_file, '/etc/asterisk/gw/web_language/'.$language_key))return false;

	/* update web language conf */
	$conf_array['list'][$language_key]=$language_value;
	modify_conf($conf_file, $conf_array);

	if(strcmp($conf_array['general']['language'],$language_key) == 0){
		wait_apply("exec", "/my_tools/web_language_init >/dev/null 2>&1 &");
	}
	
	save_user_record("","SYSTEM->General:Add Language ".$language_key);

	return true;
}

function debug_language($debug)
{
	$debug_file = '/tmp/web/language.debug';
	if($debug == 'on'){
		touch($debug_file);
	}else if($debug == 'off'){
		if(file_exists($debug_file)){
			unlink($debug_file);
		}
	}
}

$alert = '';//for javascript alert information when html loaded.
$confirm = '';//for javascript confirm information when html loaded.
if($_POST && isset($_POST['send'])) {
	if($only_view){
		return false;
	}
	
	if($_POST['send'] == 'Save') {
		save_autoreboot();
		save_firm_autoupdate();
		save_conf_autoupdate();
		
		wait_apply("exec", "/etc/init.d/cron restart > /dev/null 2>&1 &");
		if(save_web_language_conf()){
			wait_apply("exec", "/my_tools/web_language_init >/dev/null 2>&1 &");
		}
		
		save_user_record("","SYSTEM->General:Save");

	}else if($_POST['send'] == 'Add'){
		add_language($alert, $confirm);
	}else if($_POST['send'] == 'Download'){
		download_language($alert);
	}else if($_POST['send'] == 'Delete'){
		delete_language();
	}
}
if(isset($_GET['overwrite']) && $_GET['overwrite']=='yes'){
	add_language($alert, $confirm, false);
}

?>


<?php

/*********************************************************/
$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$res = $aql->query('select * from autoreboot.conf');

$week_day['0'] = 'Sun';
$week_day['1'] = 'Mon';
$week_day['2'] = 'Tue';
$week_day['3'] = 'Wed';
$week_day['4'] = 'Thu';
$week_day['5'] = 'Fri';
$week_day['6'] = 'Sat';

$type_selected['by_day'] = '';
$type_selected['by_week'] = '';
$type_selected['by_month'] = '';
$type_selected['by_run_time'] = '';

if(isset($res['general']['reboot_type'])) {
	$type = trim($res['general']['reboot_type']);
	$type_selected[$type]= 'selected';
} else {
	$type_selected['by_run_time'] = 'selected';
}

if(isset($res['general']['reboot_sw'])) {
	$reboot_sw = trim($res['general']['reboot_sw']);
} else {
	$reboot_sw = 'off';
}
if(is_true($reboot_sw))
	$sw_check = 'checked';
else
	$sw_check = '';

if(isset($res['general']['minute'])) {
	$minute = trim($res['general']['minute']);
}else{
	$minute = '';
}

if(isset($res['general']['hour'])) {
	$hour = trim($res['general']['hour']);
}else{
	$hour = '';
}

if(isset($res['general']['day'])) {
	$day = trim($res['general']['day']);
}else{
	$day = '';
}

if(isset($res['general']['month'])) {
	$month = trim($res['general']['month']);
}else{
	$month = '';
}

if(isset($res['general']['week'])) {
	$week = trim($res['general']['week']);
}else{
	$week = '';
}

/* by day */
if(isset($res['by_day']['d_minute'])) {
	$d_minute = trim($res['by_day']['d_minute']);
}else{
	$d_minute = '';
}
if(isset($res['by_day']['d_hour'])) {
	$d_hour = trim($res['by_day']['d_hour']);
}else{
	$d_hour = '';
}

/* by week */
if(isset($res['by_week']['w_minute'])) {
	$w_minute = trim($res['by_week']['w_minute']);
}else{
	$w_minute = '';
}

if(isset($res['by_week']['w_hour'])) {
	$w_hour = trim($res['by_week']['w_hour']);
}else{
	$w_hour = '';
}

if(isset($res['by_week']['w_week'])) {
	$w_week = trim($res['by_week']['w_week']);
}else{
	$w_week = '';
}

/* by month */
if(isset($res['by_month']['m_minute'])) {
	$m_minute = trim($res['by_month']['m_minute']);
}else{
	$m_minute = '';
}

if(isset($res['by_month']['m_hour'])) {
	$m_hour = trim($res['by_month']['m_hour']);
}else{
	$m_hour = '';
}

if(isset($res['by_month']['m_month'])) {
	$m_month = trim($res['by_month']['m_month']);
}else{
	$m_month = '';
}

// by run time 

if(isset($res['by_run_time']['r_hour'])) {
	$r_hour = trim($res['by_run_time']['r_hour']);
}else{
	$r_hour = '';
}

//Scheduled Firmware Update	
if(isset($res['firmware_update']['firm_reboot_sw']) && $res['firmware_update']['firm_reboot_sw'] == 'on'){
	$firm_sw_check = 'checked';
}else{
	$firm_sw_check = '';
}

$firm_type_selected['by_day'] = '';
$firm_type_selected['by_week'] = '';
$firm_type_selected['by_month'] = '';
$firm_type_selected['by_run_time'] = '';

if(isset($res['firmware_update']['firm_reboot_type'])) {
	$type = trim($res['firmware_update']['firm_reboot_type']);
	$firm_type_selected[$type]= 'selected';
} else {
	$firm_type_selected['by_run_time'] = 'selected';
}

if(isset($res['firmware_update']['firm_d_minute'])) {
	$firm_d_minute = trim($res['firmware_update']['firm_d_minute']);
}else{
	$firm_d_minute = '';
}

if(isset($res['firmware_update']['firm_d_hour'])) {
	$firm_d_hour = trim($res['firmware_update']['firm_d_hour']);
}else{
	$firm_d_hour = '';
}

if(isset($res['firmware_update']['firm_w_minute'])) {
	$firm_w_minute = trim($res['firmware_update']['firm_w_minute']);
}else{
	$firm_w_minute = '';
}

if(isset($res['firmware_update']['firm_w_hour'])) {
	$firm_w_hour = trim($res['firmware_update']['firm_w_hour']);
}else{
	$firm_w_hour = '';
}

if(isset($res['firmware_update']['firm_w_week'])) {
	$firm_w_week = trim($res['firmware_update']['firm_w_week']);
}else{
	$firm_w_week = '';
}

if(isset($res['firmware_update']['firm_m_minute'])) {
	$firm_m_minute = trim($res['firmware_update']['firm_m_minute']);
}else{
	$firm_m_minute = '';
}

if(isset($res['firmware_update']['firm_m_hour'])) {
	$firm_m_hour = trim($res['firmware_update']['firm_m_hour']);
}else{
	$firm_m_hour = '';
}

if(isset($res['firmware_update']['firm_m_month'])) {
	$firm_m_month = trim($res['firmware_update']['firm_m_month']);
}else{
	$firm_m_month = '';
}

if(isset($res['firmware_update']['firm_r_hour'])) {
	$firm_r_hour = trim($res['firmware_update']['firm_r_hour']);
}else{
	$firm_r_hour = '';
}

if(isset($res['firmware_update']['firm_r_minute'])){
	$firm_r_minute = trim($res['firmware_update']['firm_r_minute']);
}else{
	$firm_r_minute = '';
}

if(isset($res['firmware_update']['firm_link'])) {
	$firm_link = trim($res['firmware_update']['firm_link']);
}else{
	$firm_link = '';
}

//Scheduled Update Configuration
if(isset($res['conf_update']['conf_reboot_sw']) && $res['conf_update']['conf_reboot_sw'] == 'on'){
	$conf_sw_check = 'checked';
}else{
	$conf_sw_check = '';
}

$conf_type_selected['by_day'] = '';
$conf_type_selected['by_week'] = '';
$conf_type_selected['by_month'] = '';
$conf_type_selected['by_run_time'] = '';

if(isset($res['conf_update']['conf_reboot_type'])) {
	$type = trim($res['conf_update']['conf_reboot_type']);
	$conf_type_selected[$type]= 'selected';
} else {
	$conf_type_selected['by_run_time'] = 'selected';
}

if(isset($res['conf_update']['conf_d_minute'])) {
	$conf_d_minute = trim($res['conf_update']['conf_d_minute']);
}else{
	$conf_d_minute = '';
}

if(isset($res['conf_update']['conf_d_hour'])) {
	$conf_d_hour = trim($res['conf_update']['conf_d_hour']);
}else{
	$conf_d_hour = '';
}

if(isset($res['conf_update']['conf_w_minute'])) {
	$conf_w_minute = trim($res['conf_update']['conf_w_minute']);
}else{
	$conf_w_minute = '';
}

if(isset($res['conf_update']['conf_w_hour'])) {
	$conf_w_hour = trim($res['conf_update']['conf_w_hour']);
}else{
	$conf_w_hour = '';
}

if(isset($res['conf_update']['conf_w_week'])) {
	$conf_w_week = trim($res['conf_update']['conf_w_week']);
}else{
	$conf_w_week = '';
}

if(isset($res['conf_update']['conf_m_minute'])) {
	$conf_m_minute = trim($res['conf_update']['conf_m_minute']);
}else{
	$conf_m_minute = '';
}

if(isset($res['conf_update']['conf_m_hour'])) {
	$conf_m_hour = trim($res['conf_update']['conf_m_hour']);
}else{
	$conf_m_hour = '';
}

if(isset($res['conf_update']['conf_m_month'])) {
	$conf_m_month = trim($res['conf_update']['conf_m_month']);
}else{
	$conf_m_month = '';
}

if(isset($res['conf_update']['conf_r_hour'])) {
	$conf_r_hour = trim($res['conf_update']['conf_r_hour']);
}else{
	$conf_r_hour = '';
}

if(isset($res['conf_update']['conf_r_minute'])){
	$conf_r_minute = trim($res['conf_update']['conf_r_minute']);
}else{
	$conf_r_minute = '';
}

if(isset($res['conf_update']['file_link'])) {
	$file_link = trim($res['conf_update']['file_link']);
}else{
	$file_link = '';
}


if(file_exists("/proc/uptime")) {
	$fh = fopen("/proc/uptime","r");
	$line = fgets($fh);
	fclose($fh);

	$start_time = substr($line,0,strpos($line,'.'));

	$all_time = trim(`date "+%Y:%m:%d:%H:%M:%S"`);
	$item = explode(':', $all_time, 6);
	if(isset($item[5])) {
		$year = $item[0];
		$month = $item[1];
		$date = $item[2];
		$hour = $item[3];
		$minute = $item[4];
		$second = $item[5];
	}
}

$web_language_conf = '/etc/asterisk/gw/web_language.conf';
$language_conf = get_conf($web_language_conf);
?>


<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript">

function typechange()
{
	var type = document.getElementById('reboot_type').value;

	set_visible('by_day_time_table', false);
	set_visible('by_week_table', false);
	set_visible('by_week_time_table', false);
	set_visible('by_month_table', false);
	set_visible('by_month_time_table', false);
	set_visible('by_run_time_table', false);

	if (type == 'by_day') {
		set_visible('by_day_time_table', true);
	} else if (type == 'by_week') {
		set_visible('by_week_table', true);
		set_visible('by_week_time_table', true);
	} else if (type == 'by_month') {
		set_visible('by_month_table', true);
		set_visible('by_month_time_table', true);
	} else if (type == 'by_run_time') {
		set_visible('by_run_time_table', true);
	}
}

function firm_typechange()
{
	var type = document.getElementById('firm_reboot_type').value;

	set_visible('firm_by_day_time_table', false);
	set_visible('firm_by_week_table', false);
	set_visible('firm_by_week_time_table', false);
	set_visible('firm_by_month_table', false);
	set_visible('firm_by_month_time_table', false);
	set_visible('firm_by_run_time_table', false);

	if (type == 'by_day') {
		set_visible('firm_by_day_time_table', true);
	} else if (type == 'by_week') {
		set_visible('firm_by_week_table', true);
		set_visible('firm_by_week_time_table', true);
	} else if (type == 'by_month') {
		set_visible('firm_by_month_table', true);
		set_visible('firm_by_month_time_table', true);
	} else if (type == 'by_run_time') {
		set_visible('firm_by_run_time_table', true);
	}
}

function conf_typechange()
{
	var type = document.getElementById('conf_reboot_type').value;

	set_visible('conf_by_day_time_table', false);
	set_visible('conf_by_week_table', false);
	set_visible('conf_by_week_time_table', false);
	set_visible('conf_by_month_table', false);
	set_visible('conf_by_month_time_table', false);
	set_visible('conf_by_run_time_table', false);

	if (type == 'by_day') {
		set_visible('conf_by_day_time_table', true);
	} else if (type == 'by_week') {
		set_visible('conf_by_week_table', true);
		set_visible('conf_by_week_time_table', true);
	} else if (type == 'by_month') {
		set_visible('conf_by_month_table', true);
		set_visible('conf_by_month_time_table', true);
	} else if (type == 'by_run_time') {
		set_visible('conf_by_run_time_table', true);
	}
}

function onload_func()
{
	typechange();
	firm_typechange();
	conf_typechange();
}

function check_delete_language()
{
	if($('#language_type').attr('value')=='english'){
		alert("<?php echo language('Delete Language alert','Sorry, you can not delete the default language.');?>");
		return false;
	}

	if(!confirm("<?php echo language('Delete Language confirm','Are you sure to delete the selected language package?');?>")) {
		return false;
	}

	return true;
}

function checkFileChange(obj)
{
	var filesize = 0;  
	var  Sys = {};  

	if(navigator.userAgent.indexOf("MSIE")>0){
		Sys.ie=true;  
	} else
	{  
		Sys.firefox=true;  
	}
	   
	if(Sys.firefox){  
		filesize = obj.files[0].fileSize;  
	} else if(Sys.ie){
		try {
			obj.select();
			var realpath = document.selection.createRange().text;
			var fileobject = new ActiveXObject ("Scripting.FileSystemObject");
			var file = fileobject.GetFile (realpath);
			var filesize = file.Size;
		} catch(e){
			alert("Please allow ActiveX Scripting File System Object!");
			return false;
		}
	}

	if(filesize > 1000*1000*1) {
		alert("<?php echo language('Language Package Size alert','Uploaded max file is 1MB!');?>");
		g_bAllowFile = false;
		return false;
	}

	g_bAllowFile = true;
	return true;
}

function change_language(language_type)
{
	<?php
		$language_ro_str = '';
		if(isset($language_conf['list'])&&is_array($language_conf['list'])){
			foreach($language_conf['list'] as $key => $value){
				if(is_file('/www/lang/'.$key))
					$language_ro_str .= "\"$key\",";
			}
			$language_ro_str = trim($language_ro_str,',');
		}
	?>
	var language_array = new Array(<?php echo $language_ro_str;?>);
	for(var key in language_array){
		if(language_array[key]==language_type){
			//$("#delete").attr("disabled","disabled");
			break;
		}else{
			//$("#delete").removeAttr("disabled");
		}
	}
	if(language_type == 'english'){
		$("#delete").attr("disabled","disabled");
	}else{
		$("#delete").removeAttr("disabled");
	}
}

function check_add_language()
{
	if($('#upload_lang_file').attr('value') == '') {
		alert("<?php echo language('Select File alert','Please select your file first!');?>");
		return false;
	}
	return true;
}
</script>

<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<div class="content">
		<span class="title">
			<?php echo language('Language Settings');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Language')){ ?>
							<b><?php echo language('Language');?>:</b><br/>
							<?php echo language('Language help','Language Setting');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Language Download')){ ?>
							<b><?php echo language('Language Download','Download');?>:</b><br/>
							<?php echo language('Language Download help','Download selected language package.');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('language Delete')){ ?>
							<b><?php echo language('language Delete','Delete');?>:</b><br/>
							<?php echo language('language Delete help','Delete selected language.');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Add New Language')){ ?>
							<b><?php echo language('Add New Language');?>:</b><br/>
							<?php echo language('Add New Language help', 'Add New Language');?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Language');?>:
			</span>
			<div class="tab_item_right">
				<select id="language_type" name="language_type" onchange="change_language(this.value)">
				<?php
					if(isset($language_conf['general']['language']) && isset($language_conf['list'])){
						$language_type = $language_conf['general']['language'];
						if(is_array($language_conf['list'])){
							$language_list = $language_conf['list'];
							foreach($language_list as $key => $value){
								if($key == 'chinese') continue;
								if($key === $language_type)
									$selected="selected";
								else 
									$selected="";
								echo "<option value=\"$key\" $selected>$value</option>";
							}   
						}
					}else{ 
						echo "<option value=\"english\" selected>English</option>";
					}
				?>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Language Download','Download');?>:
			</span>
			<div class="tab_item_right">
			
				<?php if(!$only_view){ ?>
				<input type="submit" id="download" style="float:right" value="<?php echo language('Language Download','Download');?>" onclick="document.getElementById('send').value='Download';"/>
				<?php } ?>
				
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('language Delete','Delete');?>:
			</span>
			<div class="tab_item_right">
			
				<?php if(!$only_view){ ?>
				<input type="submit" id="delete" style="float:right" value="<?php echo language('language Delete','Delete');?>" 
					<?php if($language_type == 'english')echo 'disabled';?> 
					onclick="document.getElementById('send').value='Delete';return check_delete_language()"/>
				<?php } ?>
					
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Add New Language');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('New language Package');?>: <input type="file" name="upload_lang_file" id="upload_lang_file" onchange="return checkFileChange(this)"/>
				
				<?php if(!$only_view){ ?>
				<input type="submit" id="add" style="float:right" value="<?php echo language('Add');?>" onclick="document.getElementById('send').value='Add';return check_add_language()"/>
				<?php } ?>
				
			</div>
		</div>
	</div>

	<div class="content">
		<span class="title">
			<?php echo language('Scheduled Reboot');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Enable')){ ?>
							<b><?php echo language('Enable');?>:</b><br/>
							<?php echo language('Enable help','ON(enabled), OFF(disabled)');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Reboot Type')){ ?>
							<b><?php echo language('Reboot Type');?>:</b><br/>
							<?php echo language('Reboot Type help','
								By Day: <br>
									&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Reboot everyday at xx:xx(time). <br>
								By Week: <br>
									&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Reboot everyweek on XXX(week day) at xx:xx(time). <br>
								By Month: <br>
									&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Reboot everymonth on XXX(Month day) at xx:xx(time). <br>
								By Running Time: <br>
									&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Reboot periodically by the run time.');
							?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Enable');?>:
			</span>
			<div class="tab_item_right">
				<span><input type="checkbox" id="reboot_sw" class="checkbox" name="reboot_sw" <?php echo $sw_check ?> /></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Reboot Type');?>:
			</span>
			<div class="tab_item_right">
				<select id="reboot_type" name="reboot_type" onchange="typechange()">
					<option  value="by_day" <?php echo $type_selected['by_day'];?> ><?php echo language('By Day');?></option>
					<option  value="by_week" <?php echo $type_selected['by_week'];?> ><?php echo language('By Week');?></option>
					<option  value="by_month" <?php echo $type_selected['by_month'];?> ><?php echo language('By Month');?></option>
					<option  value="by_run_time" <?php echo $type_selected['by_run_time'];?> ><?php echo language('By Running Time');?></option>   
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="by_day_time_table"  style="display: none">
			<span>
				<?php echo language('Time');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('Hour');?>:
				<select id="d_hour" style="text-align:center;width: 50px;" name="d_hour" >
					<?php
					for($i=0; $i<=23; $i++) {
						if($i == $d_hour)
							$d_hour_selected[$i] = 'selected';
						else
							$d_hour_selected[$i] = '';
						
						echo "<option  value=\"$i\" $d_hour_selected[$i] >$i</option>";
					}
					?>
				</select>
				&nbsp;&nbsp;&nbsp;
				<?php echo language('Minute');?>:
				<select id="d_minute" style="text-align:center;width: 50px;" name="d_minute">
					<?php
					for($i=0; $i<=59; $i++)
					{
						if($i == $d_minute)
							$d_minute_selected[$i] = 'selected';
						else
							$d_minute_selected[$i] = '';
						echo "<option  value=\"$i\" $d_minute_selected[$i] >$i</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="by_week_table" style="display: none">
			<span>
				<?php echo language('Week');?>:
			</span>
			<div class="tab_item_right">
				<select id="w_week" name="w_week" >
					<?php
					for($i=0; $i<7; $i++)
					{
						if($i == $w_week)
							$w_week_selected[$i] = 'selected';
						else
							$w_week_selected[$i] = '';
						
						echo "<option  value=\"$i\" $w_week_selected[$i] >";
						echo language($week_day[$i]);
						echo "</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="by_week_time_table"  style="display: none" >
			<span>
				<?php echo language('Time');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('Hour');?>:
				<select id="w_hour" style="text-align:center;width: 50px;" name="w_hour" >
					<?php
					for($i=0; $i<=23; $i++) {
						if($i == $w_hour)
							$w_hour_selected[$i] = 'selected';
						else
							$w_hour_selected[$i] = '';

						echo "<option  value=\"$i\" $w_hour_selected[$i] >$i</option>";
					}
					?>
				</select>
				&nbsp;&nbsp;&nbsp;
				<?php echo language('Minute');?>:
				<select id="w_minute" style="text-align:center;width: 50px;" name="w_minute">
					<?php
					for($i=0; $i<=59; $i++) {
							if($i == $w_minute)
									$w_minute_selected[$i] = 'selected';
							else
									$w_minute_selected[$i] = '';
							echo "<option  value=\"$i\" $w_minute_selected[$i] >$i</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="by_month_table" style="display: none">
			<span>
				<?php echo language('Date');?>:
			</span>
			<div class="tab_item_right">
				<select id="m_month" style="text-align:center;width: 50px;" name="m_month" >
					<?php
					for($i=1; $i<=31; $i++) {
						if($i == $m_month)
							$m_month_selected[$i] = 'selected';
						else
							$m_month_selected[$i] = '';
						echo "<option  value=\"$i\" $m_month_selected[$i] >$i</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="by_month_time_table"  style="display: none" >
			<span>
				<?php echo language('Time');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('Hour');?>:
				<select id="m_hour" style="text-align:center;width: 50px;" name="m_hour" >
					<?php
					for($i=0; $i<=23; $i++) {
						if($i == $m_hour)
							$m_hour_selected[$i] = 'selected';
						else
							$m_hour_selected[$i] = '';

						echo "<option  value=\"$i\" $m_hour_selected[$i] >$i</option>";
					}
					?>
				</select>
				&nbsp;&nbsp;&nbsp;
				<?php echo language('Minute');?>:
				<select id="m_minute" style="text-align:center;width: 50px;" name="m_minute">
					<?php
					for($i=0; $i<=59; $i++) {
						if($i == $m_minute)
							$m_minute_selected[$i] = 'selected';
						else
							$m_minute_selected[$i] = '';
						echo "<option  value=\"$i\" $m_minute_selected[$i] >$i</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="by_run_time_table" style="display: none" >
			<span>
				<?php echo language('Running Time');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('Hour');?>:
				<select id="r_hour" style="text-align:center;width: 50px;" name="r_hour" >
					<?php
					for($i=0; $i<=23; $i++) {
						if($i == $r_hour)
							$r_hour_selected[$i] = 'selected';
						else
							$r_hour_selected[$i] = '';
						
						echo "<option  value=\"$i\" $r_hour_selected[$i] >$i</option>";
					}
					?>
				</select>
			</div>
		</div>
	</div>
	
	<!-- Scheduled Firmware Update -->
	<div class="content">
		<span class="title">
			<?php echo language('Scheduled Firmware Update');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Enable')){ ?>
							<b><?php echo language('Enable');?>:</b><br/>
							<?php echo language('Enable help','ON(enabled), OFF(disabled)');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Reboot Type')){ ?>
							<b><?php echo language('Reboot Type');?>:</b><br/>
							<?php echo language('Reboot Type help','
								By Day: <br>
									&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Reboot everyday at xx:xx(time). <br>
								By Week: <br>
									&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Reboot everyweek on XXX(week day) at xx:xx(time). <br>
								By Month: <br>
									&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Reboot everymonth on XXX(Month day) at xx:xx(time). <br>
								By Running Time: <br>
									&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Reboot periodically by the run time.');
							?>
						<?php } ?>
						
						<?php if(is_show_language_help('Firmware Link')){ ?>
							<b><?php echo language('Firmware Link'); ?>:</b><br/>
							<?php echo language('Firmware Link help','Firmware Link');?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Enable');?>:
			</span>
			<div class="tab_item_right">
				<span><input type="checkbox" id="firm_reboot_sw" class="checkbox" name="firm_reboot_sw" <?php echo $firm_sw_check; ?> /></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Reboot Type');?>:
			</span>
			<div class="tab_item_right">
				<select id="firm_reboot_type" name="firm_reboot_type" onchange="firm_typechange()">
					<option  value="by_day" <?php echo $firm_type_selected['by_day'];?> ><?php echo language('By Day');?></option>
					<option  value="by_week" <?php echo $firm_type_selected['by_week'];?> ><?php echo language('By Week');?></option>
					<option  value="by_month" <?php echo $firm_type_selected['by_month'];?> ><?php echo language('By Month');?></option>
					<option  value="by_run_time" <?php echo $firm_type_selected['by_run_time'];?> ><?php echo language('By Running Time');?></option>   
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="firm_by_day_time_table"  style="display: none">
			<span>
				<?php echo language('Time');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('Hour');?>:
				<select id="firm_d_hour" style="text-align:center;width: 50px;" name="firm_d_hour" >
					<?php
					for($i=0; $i<=23; $i++) {
						if($i == $firm_d_hour)
							$firm_d_hour_selected[$i] = 'selected';
						else
							$firm_d_hour_selected[$i] = '';
						
						echo "<option  value=\"$i\" $firm_d_hour_selected[$i] >$i</option>";
					}
					?>
				</select>
				&nbsp;&nbsp;&nbsp;
				<?php echo language('Minute');?>:
				<select id="firm_d_minute" style="text-align:center;width: 50px;" name="firm_d_minute">
					<?php
					for($i=0; $i<=59; $i++)
					{
						if($i == $firm_d_minute)
							$firm_d_minute_selected[$i] = 'selected';
						else
							$firm_d_minute_selected[$i] = '';
						echo "<option  value=\"$i\" $firm_d_minute_selected[$i] >$i</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="firm_by_week_table" style="display: none">
			<span>
				<?php echo language('Week');?>:
			</span>
			<div class="tab_item_right">
				<select id="firm_w_week" name="firm_w_week" >
					<?php
					for($i=0; $i<7; $i++)
					{
						if($i == $firm_w_week)
							$firm_w_week_selected[$i] = 'selected';
						else
							$firm_w_week_selected[$i] = '';
						
						echo "<option  value=\"$i\" $firm_w_week_selected[$i] >";
						echo language($week_day[$i]);
						echo "</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="firm_by_week_time_table"  style="display: none" >
			<span>
				<?php echo language('Time');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('Hour');?>:
				<select id="firm_w_hour" style="text-align:center;width: 50px;" name="firm_w_hour" >
					<?php
					for($i=0; $i<=23; $i++) {
						if($i == $firm_w_hour)
							$firm_w_hour_selected[$i] = 'selected';
						else
							$firm_w_hour_selected[$i] = '';

						echo "<option  value=\"$i\" $firm_w_hour_selected[$i] >$i</option>";
					}
					?>
				</select>
				&nbsp;&nbsp;&nbsp;
				<?php echo language('Minute');?>:
				<select id="firm_w_minute" style="text-align:center;width: 50px;" name="firm_w_minute">
					<?php
					for($i=0; $i<=59; $i++) {
							if($i == $firm_w_minute)
									$firm_w_minute_selected[$i] = 'selected';
							else
									$firm_w_minute_selected[$i] = '';
							echo "<option  value=\"$i\" $firm_w_minute_selected[$i] >$i</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="firm_by_month_table" style="display: none">
			<span>
				<?php echo language('Date');?>:
			</span>
			<div class="tab_item_right">
				<select id="firm_m_month" style="text-align:center;width: 50px;" name="firm_m_month" >
					<?php
					for($i=1; $i<=31; $i++) {
						if($i == $firm_m_month)
							$firm_m_month_selected[$i] = 'selected';
						else
							$firm_m_month_selected[$i] = '';
						echo "<option  value=\"$i\" $firm_m_month_selected[$i] >$i</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="firm_by_month_time_table"  style="display: none" >
			<span>
				<?php echo language('Time');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('Hour');?>:
				<select id="firm_m_hour" style="text-align:center;width: 50px;" name="firm_m_hour" >
					<?php
					for($i=0; $i<=23; $i++) {
						if($i == $firm_m_hour)
							$firm_m_hour_selected[$i] = 'selected';
						else
							$firm_m_hour_selected[$i] = '';

						echo "<option  value=\"$i\" $firm_m_hour_selected[$i] >$i</option>";
					}
					?>
				</select>
				&nbsp;&nbsp;&nbsp;
				<?php echo language('Minute');?>:
				<select id="firm_m_minute" style="text-align:center;width: 50px;" name="firm_m_minute">
					<?php
					for($i=0; $i<=59; $i++) {
						if($i == $firm_m_minute)
							$firm_m_minute_selected[$i] = 'selected';
						else
							$firm_m_minute_selected[$i] = '';
						echo "<option  value=\"$i\" $firm_m_minute_selected[$i] >$i</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="firm_by_run_time_table" style="display: none" >
			<span>
				<?php echo language('Running Time');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('Hour');?>:
				<select id="firm_r_hour" style="text-align:center;width: 50px;" name="firm_r_hour" >
					<?php
					for($i=0; $i<=23; $i++) {
						if($i == $firm_r_hour)
							$firm_r_hour_selected[$i] = 'selected';
						else
							$firm_r_hour_selected[$i] = '';
						
						echo "<option  value=\"$i\" $firm_r_hour_selected[$i] >$i</option>";
					}
					?>
				</select>
				&nbsp;&nbsp;&nbsp;
				<?php echo language('Minute');?>:
				<select id="firm_r_minute" style="text-align:center;width: 50px;" name="firm_r_minute">
					<?php
					for($i=0; $i<=59; $i++) {
						if($i == $firm_r_minute)
							$firm_r_minute_selected[$i] = 'selected';
						else
							$firm_r_minute_selected[$i] = '';
						
						echo "<option  value=\"$i\" $firm_r_minute_selected[$i] >$i</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Firmware Link');?>:
			</span>
			<div class="tab_item_right">
				<input type="text" name="firm_link" id="firm_link" value="<?php echo $firm_link; ?>" />
			</div>
		</div>
	</div>
	
	<!-- Scheduled Update Configuration -->
	<div class="content">
		<span class="title">
			<?php echo language('Scheduled Update Configuration');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Enable')){ ?>
							<b><?php echo language('Enable');?>:</b><br/>
							<?php echo language('Enable help','ON(enabled), OFF(disabled)');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Reboot Type')){ ?>
							<b><?php echo language('Reboot Type');?>:</b><br/>
							<?php echo language('Reboot Type help','
								By Day: <br>
									&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Reboot everyday at xx:xx(time). <br>
								By Week: <br>
									&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Reboot everyweek on XXX(week day) at xx:xx(time). <br>
								By Month: <br>
									&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Reboot everymonth on XXX(Month day) at xx:xx(time). <br>
								By Running Time: <br>
									&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Reboot periodically by the run time.');
							?>
						<?php } ?>
						
						<?php if(is_show_language_help('File Link')){ ?>
							<b><?php echo language('File Link'); ?>:</b><br/>
							<?php echo language('File Link help','File Link');?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Enable');?>:
			</span>
			<div class="tab_item_right">
				<span><input type="checkbox" id="conf_reboot_sw" class="checkbox" name="conf_reboot_sw" <?php echo $conf_sw_check; ?> /></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Reboot Type');?>:
			</span>
			<div class="tab_item_right">
				<select id="conf_reboot_type" name="conf_reboot_type" onchange="conf_typechange()">
					<option  value="by_day" <?php echo $conf_type_selected['by_day'];?> ><?php echo language('By Day');?></option>
					<option  value="by_week" <?php echo $conf_type_selected['by_week'];?> ><?php echo language('By Week');?></option>
					<option  value="by_month" <?php echo $conf_type_selected['by_month'];?> ><?php echo language('By Month');?></option>
					<option  value="by_run_time" <?php echo $conf_type_selected['by_run_time'];?> ><?php echo language('By Running Time');?></option>   
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="conf_by_day_time_table"  style="display: none">
			<span>
				<?php echo language('Time');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('Hour');?>:
				<select id="conf_d_hour" style="text-align:center;width: 50px;" name="conf_d_hour" >
					<?php
					for($i=0; $i<=23; $i++) {
						if($i == $conf_d_hour)
							$conf_d_hour_selected[$i] = 'selected';
						else
							$conf_d_hour_selected[$i] = '';
						
						echo "<option  value=\"$i\" $conf_d_hour_selected[$i] >$i</option>";
					}
					?>
				</select>
				&nbsp;&nbsp;&nbsp;
				<?php echo language('Minute');?>:
				<select id="conf_d_minute" style="text-align:center;width: 50px;" name="conf_d_minute">
					<?php
					for($i=0; $i<=59; $i++)
					{
						if($i == $conf_d_minute)
							$conf_d_minute_selected[$i] = 'selected';
						else
							$conf_d_minute_selected[$i] = '';
						echo "<option  value=\"$i\" $conf_d_minute_selected[$i] >$i</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="conf_by_week_table" style="display: none">
			<span>
				<?php echo language('Week');?>:
			</span>
			<div class="tab_item_right">
				<select id="conf_w_week" name="conf_w_week" >
					<?php
					for($i=0; $i<7; $i++)
					{
						if($i == $conf_w_week)
							$conf_w_week_selected[$i] = 'selected';
						else
							$conf_w_week_selected[$i] = '';
						
						echo "<option  value=\"$i\" $conf_w_week_selected[$i] >";
						echo language($week_day[$i]);
						echo "</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="conf_by_week_time_table"  style="display: none" >
			<span>
				<?php echo language('Time');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('Hour');?>:
				<select id="conf_w_hour" style="text-align:center;width: 50px;" name="conf_w_hour" >
					<?php
					for($i=0; $i<=23; $i++) {
						if($i == $conf_w_hour)
							$conf_w_hour_selected[$i] = 'selected';
						else
							$conf_w_hour_selected[$i] = '';

						echo "<option  value=\"$i\" $conf_w_hour_selected[$i] >$i</option>";
					}
					?>
				</select>
				&nbsp;&nbsp;&nbsp;
				<?php echo language('Minute');?>:
				<select id="conf_w_minute" style="text-align:center;width: 50px;" name="conf_w_minute">
					<?php
					for($i=0; $i<=59; $i++) {
							if($i == $conf_w_minute)
									$conf_w_minute_selected[$i] = 'selected';
							else
									$conf_w_minute_selected[$i] = '';
							echo "<option  value=\"$i\" $conf_w_minute_selected[$i] >$i</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="conf_by_month_table" style="display: none">
			<span>
				<?php echo language('Date');?>:
			</span>
			<div class="tab_item_right">
				<select id="conf_m_month" style="text-align:center;width: 50px;" name="conf_m_month" >
					<?php
					for($i=1; $i<=31; $i++) {
						if($i == $conf_m_month)
							$conf_m_month_selected[$i] = 'selected';
						else
							$conf_m_month_selected[$i] = '';
						echo "<option  value=\"$i\" $conf_m_month_selected[$i] >$i</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="conf_by_month_time_table"  style="display: none" >
			<span>
				<?php echo language('Time');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('Hour');?>:
				<select id="conf_m_hour" style="text-align:center;width: 50px;" name="conf_m_hour" >
					<?php
					for($i=0; $i<=23; $i++) {
						if($i == $conf_m_hour)
							$conf_m_hour_selected[$i] = 'selected';
						else
							$conf_m_hour_selected[$i] = '';

						echo "<option  value=\"$i\" $conf_m_hour_selected[$i] >$i</option>";
					}
					?>
				</select>
				&nbsp;&nbsp;&nbsp;
				<?php echo language('Minute');?>:
				<select id="conf_m_minute" style="text-align:center;width: 50px;" name="conf_m_minute">
					<?php
					for($i=0; $i<=59; $i++) {
						if($i == $conf_m_minute)
							$conf_m_minute_selected[$i] = 'selected';
						else
							$conf_m_minute_selected[$i] = '';
						echo "<option  value=\"$i\" $conf_m_minute_selected[$i] >$i</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="conf_by_run_time_table" style="display: none" >
			<span>
				<?php echo language('Running Time');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('Hour');?>:
				<select id="conf_r_hour" style="text-align:center;width: 50px;" name="conf_r_hour" >
					<?php
					for($i=0; $i<=23; $i++) {
						if($i == $conf_r_hour)
							$conf_r_hour_selected[$i] = 'selected';
						else
							$conf_r_hour_selected[$i] = '';
						
						echo "<option  value=\"$i\" $conf_r_hour_selected[$i] >$i</option>";
					}
					?>
				</select>
				&nbsp;&nbsp;&nbsp;
				<?php echo language('Minute');?>:
				<select id="conf_r_minute" style="text-align:center;width: 50px;" name="conf_r_minute">
					<?php
					for($i=0; $i<=59; $i++) {
						if($i == $conf_r_minute){
							$firm_m_minute_selected = 'selected';
						}else{
							$firm_m_minute_selected = '';
						}
						
						echo "<option  value=\"$i\" $firm_m_minute_selected >$i</option>";
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('File Link');?>:
			</span>
			<div class="tab_item_right">
				<input type="text" name="file_link" id="file_link" value="<?php echo $file_link; ?>" />
			</div>
		</div>
	</div>
	
	<div id="button_save">
	
		<?php if(!$only_view){ ?>
		<button type="submit" onclick="document.getElementById('send').value='Save';" <?php if($__demo_enable__=='on'){echo 'disabled';}?> /><?php echo language('Save');?></button>
		<?php } ?>
		
	</div>
	<input type="hidden" name="send" id="send" value="" />
</form>

<script type="text/javascript"> 
$(document).ready(function (){ 
	if(<?php if($alert=='')echo "false";else echo "true";?>){
		alert(<?php echo "\"Warning: ".$alert."\"";?>);
	}else if(<?php if($confirm=='')echo "false";else echo "true";?>){
		if(confirm(<?php echo "\"$confirm\"";?>)){
			window.location.href = "<?php echo get_self();?>"+"?overwrite=yes";
		}
	}
	onload_func();
}); 

function language_debug(debug_status)
{
	window.location.href="<?php echo get_self() ?>"+"?send_debug="+debug_status;
}
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>