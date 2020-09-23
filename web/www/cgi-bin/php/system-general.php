<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/network_factory.inc");
?>

<!---// load jQuery and the jQuery iButton Plug-in //---> 
<!--<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.3.2/jquery.min.js"></script> -->
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<script type="text/javascript" src="/js/float_btn.js"></script>
 
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
	
	$product_type = get_product_type();
	if($product_type < 4){
		$safe_reboot_tools = '/my_tools/safe_reboot > /tmp/reboot.log';
	}else{
		$safe_reboot_tools = 'root /my_tools/safe_reboot';
	}

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
		}

		/* Slave language sync */
		if($__deal_cluster__){
			$cluster_info = get_cluster_info();
			if($cluster_info['mode'] == 'master') {
				for($b=2; $b<=$__BRD_SUM__; $b++) {
					if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
						$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
						/* 1.Modify slave language conf */
						set_slave_file($slaveip,"/etc/asterisk/gw/web_language.conf","/etc/asterisk/gw/web_language.conf");
						/* 2.Delete slave language package */
						$data = "syscmd:rm /etc/asterisk/gw/web_language/$language_type >/dev/null 2>&1 &";
						request_slave($slaveip, $data, 5, false);
						/* 3.Check language setting */
						if(isset($conf_array['general']['language']) && strcmp($conf_array['general']['language'],$language_type)==0){
							wait_apply('request_slave', $slaveip, "syscmd:/my_tools/web_language_init >/dev/null 2>&1 &");
						}
					}    
				}    
			}
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
	if(!is_dir('/etc/asterisk/gw/web_language/')){
		mkdir('/etc/asterisk/gw/web_language/');
	}
	if(!copy($store_file, '/etc/asterisk/gw/web_language/'.$language_key))return false;

	/* update web language conf */
	$conf_array['list'][$language_key]=$language_value;
	modify_conf($conf_file, $conf_array);

	if(strcmp($conf_array['general']['language'],$language_key) == 0){
		wait_apply("exec", "/my_tools/web_language_init >/dev/null 2>&1 &");
	}

	/* Slave language sync */
	if($__deal_cluster__){
		$cluster_info = get_cluster_info();
		if($cluster_info['mode'] == 'master') {
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
					/* Add slave language package */
					set_slave_file($slaveip,"/etc/asterisk/gw/web_language/$language_key","/etc/asterisk/gw/web_language/$language_key");
					set_slave_file($slaveip,"/etc/asterisk/gw/web_language.conf","/etc/asterisk/gw/web_language.conf");
					if(strcmp($conf_array['general']['language'],$language_key) == 0){
						wait_apply('request_slave', $slaveip, "syscmd:/my_tools/web_language_init >/dev/null 2>&1 &");
					}
				}    
			}    
		}
	}

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
	if($_POST['send'] == 'Save') {
		save_autoreboot();
		wait_apply("exec", "/etc/init.d/cron restart > /dev/null 2>&1 &");
		if(save_web_language_conf()){
			wait_apply("exec", "/my_tools/web_language_init >/dev/null 2>&1 &");
		}

		if($__deal_cluster__){
			$cluster_info = get_cluster_info();
			if($cluster_info['mode'] == 'master') {
				for($b=2; $b<=$__BRD_SUM__; $b++) {
					if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
						$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];

						/* Language conf sync */
						set_slave_file($slaveip,"/etc/asterisk/gw/web_language.conf","/etc/asterisk/gw/web_language.conf");
						wait_apply('request_slave', $slaveip, "syscmd:/my_tools/web_language_init >/dev/null 2>&1 &");

						/* Schedule reboot conf sync */
						set_slave_file($slaveip,"/etc/asterisk/gw/crontabs_root","/etc/asterisk/gw/crontabs_root");
						set_slave_file($slaveip,"/etc/asterisk/gw/autoreboot.conf","/etc/asterisk/gw/autoreboot.conf");
						wait_apply("request_slave", $slaveip, "syscmd:/etc/init.d/cron restart > /dev/null 2>&1 &");
					}    
				}    
			}
		}

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

js_float_show_time();

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

function onload_func()
{
	typechange();
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
	//if(isFirefox=navigator.userAgent.indexOf("Firefox")>0)  
	{  
		Sys.firefox=true;  
	}
	   
	if(Sys.firefox){  
		filesize = obj.files[0].fileSize;  
	} else if(Sys.ie){
		try {
			obj.select();
			var realpath = document.selection.createRange().text;
			//alert(obj.value);
			//alert(realpath);
			var fileobject = new ActiveXObject ("Scripting.FileSystemObject");//»ñÈ¡ÉÏ´«ÎÄ¼þµÄ¶ÔÏó  
			//var file = fileobject.GetFile (obj.value);//»ñÈ¡ÉÏ´«µÄÎÄ¼þ  
			var file = fileobject.GetFile (realpath);//»ñÈ¡ÉÏ´«µÄÎÄ¼þ  
			var filesize = file.Size;//ÎÄ¼þ´óÐ¡  
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
			$("#delete").attr("disabled",true);
		}else{
			$("#delete").attr("disabled",false);
		}
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
	<!-- ### language settings ### -->
	<div id="tab" style="height:30px;">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Language Settings');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	
	<div width="100%" class="div_setting_c">
		<div class="divc_setting_v">
		<table width='100%' class="tedit" style="border:none">
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Language');?>:
					<span class="showhelp">
					<?php echo language('Language help');?>
					</span>
				</div>
			</th>
			<td >
				<table style="width:100%;">
					<tr>
						<td style="border:none;margin:0px;padding:0px;">
							<select id="language_type" name="language_type" onchange="change_language(this.value)">
							<?php
								if(isset($language_conf['general']['language']) && isset($language_conf['list'])){
									$language_type = $language_conf['general']['language'];
									if(is_array($language_conf['list'])){
										$language_list = $language_conf['list'];
										foreach($language_list as $key => $value){
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
						</td>
					</tr>
				</table>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Advanced');?>:
					<span class="showhelp">
					<?php echo language('Advanced help');?>
					</span>
				</div>
			</th>
			<td>
				<input type="checkbox" id="lang_adv_enable" onchange="$('#lang_adv').slideToggle();"/>
			</td>
		</tr>
		</table>
		</div>
		<div id='lang_adv' class='div_setting_d' style="position:relative;top:-2px;">
		<table width='100%' class="tedit" style="border:none">
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Language Debug');?>:
					<span class="showhelp">
					<?php echo language('Language Debug help');?>
					</span>
				</div>
			</th>
			<td>
				<input type="button" id="" onclick="language_debug('on');" value="<?php echo language('TURN ON');?>"/>&nbsp;&nbsp;&nbsp;&nbsp;
				<input type="button" id="" onclick="language_debug('off');" value="<?php echo language('TURN OFF');?>"/>
			</td>
		</tr>
		<tr id="language_download">
			<th>
				<div class="helptooltips">
					<?php echo language('Download');?>:
					<span class="showhelp">
					<?php echo language('Download help');?>
					</span>
				</div>
			</th>
			<td>
				<table style="width:100%;">
					<tr>
						<td style="border:none;margin:0px;padding:0px;">
							<?php echo language('Language Download help','Download selected language package.');?>
						</td>
						<td style="border:none;margin:0px;padding:0px;">
							<input type="submit" id="download" style="float:right" value="<?php echo language('Download');?>" 
								onclick="document.getElementById('send').value='Download';"/>
						</td>
					</tr>
				</table>
			</td>
		</tr>
		<tr id="language_delete">
			<th><?php echo language('Delete');?>:</th>
			<td>
				<table style="width:100%;">
					<tr>
						<td style="border:none;margin:0px;padding:0px;">
							<?php echo language('Delete language help','Delete selected language.');?>
						</td>
						<td style="border:none;margin:0px;padding:0px;">
							<input type="submit" id="delete" style="float:right" value="<?php echo language('Delete');?>" 
								<?php if(is_file('/www/lang/'.$language_type))echo 'disabled';?> 
								onclick="document.getElementById('send').value='Delete';return check_delete_language()"/>
						</td>
					</tr>
				</table>
			</td>
		</tr>	
		<tr id="language_add">
			<th>
				<div class="helptooltips">
					<?php echo language('Add New Language');?>:
					<span class="showhelp">
					<?php echo language('Add New Language help');?>
					</span>
				</div>
			</th>
			<td>
				<table width="100%">
					<tr>
						<td style="border:none;margin:0px;padding:0px;">
							<?php echo language('New language Package');?>: 
							<input type="file" name="upload_lang_file" id="upload_lang_file" onchange="return checkFileChange(this)"/>
						</td>
						<td style="border:none;margin:0px;padding:0px;">
							<input type="submit" id="add" style="float:right" value="<?php echo language('Add');?>" 
								onclick="document.getElementById('send').value='Add';return check_add_language()"/>
						</td>
					</tr>
				</table>
			</td>
		</tr>
		</table>
		</div>
	</div>

	<div id="newline"></div>

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Scheduled Reboot');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tedit" >
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Enable');?>:
					<span class="showhelp">
					<?php echo language('Enable help','ON(enabled), OFF(disabled)');?>
					</span>
				</div>
			</th>
			<td>
				<input type="checkbox" id="reboot_sw" class="checkbox" name="reboot_sw" <?php echo $sw_check ?> onchange="reboot_change(this.checked)"/>
			</td>
		</tr>

		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Reboot Type');?>:
					<span class="showhelp">
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
					</span>
				</div>
			</th>
			<td >
				<select id="reboot_type" name="reboot_type" onchange="typechange()">
					<option  value="by_day" <?php echo $type_selected['by_day'];?> ><?php echo language('By Day');?></option>
					<option  value="by_week" <?php echo $type_selected['by_week'];?> ><?php echo language('By Week');?></option>
					<option  value="by_month" <?php echo $type_selected['by_month'];?> ><?php echo language('By Month');?></option>
					<option  value="by_run_time" <?php echo $type_selected['by_run_time'];?> ><?php echo language('By Running Time');?></option>   
				</select>
			</td>
		</tr>				

		<!-- by the day #################################################################  -->
		<tr id="by_day_time_table"  style="display: none" >
			<th>
				<div class="helptooltips">
					<?php echo language('Time');?>:
					<span class="showhelp">
					<?php echo language('Time help');?>
					</span>
				</div>
			</th>
			<td>
				<table cellpadding="0" cellspacing="0" style="border:none;" >
					<tr>
					<td style="border:none;">
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
					</td>
					</tr>
				</table>
			</td>
		</tr>
		<!-- by week  #################################################################  -->
		<tr id="by_week_table" style="display: none" >
			<th>
				<div class="helptooltips">
					<?php echo language('Week');?>:
					<span class="showhelp">
					<?php echo language('Week help');?>
					</span>
				</div>
			</th>
			<td>
				<table cellpadding="0" cellspacing="0" style="border:none;" >
					<tr>
					<td style="border:none;">
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
					</td>
					</tr>
				</table>
			</td>
		</tr>
		<tr id="by_week_time_table"  style="display: none" >
			<th>
				<div class="helptooltips">
					<?php echo language('Time');?>:
					<span class="showhelp">
					<?php echo language('Time help2');?>
					</span>
				</div>
			</th>
			<td>
				<table cellpadding="0" cellspacing="0" style="border:none;" >
					<tr>
					<td style="border:none;">
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
					</td>
					</tr>
				</table>
			</td>
		</tr>
		<!-- Days #################################################################  -->
		<tr id="by_month_table" style="display: none" >
			<th>
				<div class="helptooltips">
					<?php echo language('Date');?>:
					<span class="showhelp">
					<?php echo language('Date help');?>
					</span>
				</div>
			</th>
			<td>
				<table cellpadding="0" cellspacing="0" style="border:none;" >
					<td style="border:none;">
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
					</td>
				</table>
			</td>
		</tr>
		<tr id="by_month_time_table"  style="display: none" >
			<th>
				<div>
					<?php echo language('Time');?>:
				</div>
			</th>
			<td>
				<table cellpadding="0" cellspacing="0" style="border:none;" >
					<tr>
					<td style="border:none;">
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
					</td>
					</tr>
				</table>
			</td>
		</tr>
		<!-- Months #################################################################  -->
		<tr id="by_run_time_table" style="display: none" >
			<th>
				<div class="helptooltips">
					<?php echo language('Running Time');?>:
					<span class="showhelp">
					<?php echo language('Running Time help');?>
					</span>
				</div>
			</th>
			<td>
				<table cellpadding="0" cellspacing="0" style="border:none;" >
					<tr>
					<td style="border:none;">
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
					</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>

	<div id="newline"></div>	

	<input type="submit" class="float_btn gen_short_btn"  value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';" <?php if($__demo_enable__=='on'){echo 'disabled';}?> />
	<input type="hidden" name="send" id="send" value="" />
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td>
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';" <?php if($__demo_enable__=='on'){echo 'disabled';}?> />
			</td>
		</tr>
	</table>
</form>




<script type="text/javascript"> 
$(document).ready(function (){ 
	$(":checkbox").iButton(); 
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
<div id="float_btn1" class="sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()">
</div>
