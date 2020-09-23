<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
require_once("/www/cgi-bin/inc/aql.php");
?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript">
<!--
function enable_tcp_change()
{
	value = document.getElementById('enable_tcp').value == 'yes' ? false : true;
	
	document.getElementById('tcp_bind_port').disabled = value;
	document.getElementById('tcp_authentication_timeout').disabled = value;
	document.getElementById('tcp_authentication_limit').disabled = value;
}

function enable_internal_sip_call_change()
{
	value = document.getElementById('enable_internal_sip_call').value == 'yes' ? false : true;
	document.getElementById('internal_sip_call_prefix').disabled = value;
}

function onload_func()
{
	click_externaddr_readonly();
	enable_tcp_change();
	enable_internal_sip_call_change();
}

var g_bAllowFile = false;

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
		//filesize = obj.files[0].fileSize;  
		filesize = obj.files[0].size;  
	} else if(Sys.ie){
		try {
			obj.select();
			var realpath = document.selection.createRange().text;
			var fileobject = new ActiveXObject ("Scripting.FileSystemObject");
			//var file = fileobject.GetFile (obj.value);
			var file = fileobject.GetFile (realpath);
			var filesize = file.Size;
		} catch(e){
			alert("<?php echo language('System Update IE alert','Please allow ActiveX Scripting File System Object!');?>");
			return false;
		}
	}

	if(filesize > 1024*1024) {
		alert("<?php echo language('Language Package Size alert','Uploaded max file is 1MB!');?>");
		g_bAllowFile = false;
		return false;
	}

	g_bAllowFile = true;
	return true;
} 

function isAllowFile(file_id)
{
	var x = document.getElementById(file_id).value;
	if(x=="")
	{
		alert("<?php echo language('Select File alert','Please select your file first!');?>");
		return false;
	}

	if(g_bAllowFile)
		return true;

	alert("<?php echo language('Language Package Size alert','Uploaded max file is 1MB!');?>");
	return false;
}

function upload_cfg_file2()
{
	if(!isAllowFile('language_file')) {
		return false;
	}

	if( ! confirm("<?php echo language('File Upload confirm','Are you sure to upload configuration files?\nThis will damage the structure of your original configuration files.');?>") ) {
		return false;
	}

	return true;
}
-->
</script>

<?php

function prepare_slave_sip_general_conf($src_file,$des_file)
{
	$str = "register";	
	$str_len = strlen($str);

	$flock1 = lock_file($src_file);
	$flock2 = lock_file($des_file);

	$srch=fopen($src_file,'r');
	$desh=fopen($des_file,'w');
	while(!feof($srch)) {
		//$line = stream_get_line($srch, 1000000, "\n");
		$line = fgets($srch);
		if(strncmp($line,$str,$str_len) == 0) continue;
		if($line) fwrite($desh,$line);
	}
	fclose($desh);
	fclose($srch);

	unlock_file($flock1);
	unlock_file($flock2);
	
	return true;
}

function save_to_sip_general_conf()
{
	global $__BRD_SUM__;	
	global $__BRD_HEAD__;	

	$sip_general_conf_path="/etc/asterisk/sip_general.conf";
	if(!file_exists($sip_general_conf_path)) {
		fclose(fopen($sip_general_conf_path,"w"));
	}

	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk');
	if (!$setok) {
		echo $aql->get_error();
		return;
	}

	$hlock = lock_file($sip_general_conf_path);

	if(!$aql->open_config_file($sip_general_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}

	$exist_array = $aql->query("select * from sip_general.conf where section='general'");

	if(!isset($exist_array['general'])) {
		$aql->assign_addsection('general','');
	}

	$lan_res = get_conf("/etc/asterisk/gw/network/lan.conf");
	
	//POST data pretreatment
	if(isset($_POST['udp_bind_port']) && $_POST['udp_bind_port'] != '') {
		$_udp_bind_port = $_POST['udp_bind_port'];
		if($lan_res['ipv6']['enabled'] == 'on' || $lan_res['ipv6']['enabled'] == ""){
			$_POST['udp_bind_port'] = "[::]:".$_udp_bind_port;
		}else{
			$_POST['udp_bind_port'] = "0.0.0.0:".$_udp_bind_port;
		}
	}
	if(isset($_POST['tcp_bind_port']) && $_POST['tcp_bind_port'] != '') {
		$_tcp_bind_port = $_POST['tcp_bind_port'];
		if($lan_res['ipv6']['enabled'] == 'on' || $lan_res['ipv6']['enabled'] == ""){
			$_POST['tcp_bind_port'] = "[::]:".$_tcp_bind_port;
		}else{
			$_POST['tcp_bind_port'] = "0.0.0.0:".$_tcp_bind_port;
		}
	}
	if(isset($_POST['sdp_owner']) && isset($_POST['disallowed_sip_methods']) && is_array($_POST['disallowed_sip_methods']) && $_POST['disallowed_sip_methods'] != '') {
		$_disallowed_sip_methods = '';
		foreach($_POST['disallowed_sip_methods'] as $each) {
			$_disallowed_sip_methods .= $each.',';
		}
		$_POST['disallowed_sip_methods'] = rtrim($_disallowed_sip_methods,',');
	}else{
		$_POST['disallowed_sip_methods'] = '';
	}
	$post_conf_array = array(
		'udp_bind_port'				=>array('udpbindaddr',''),
		'enable_tcp'				=>array('tcpenable',''),
		'tcp_bind_port'				=>array('tcpbindaddr',''),
		'tcp_authentication_timeout'		=>array('tcpauthtimeout',''),
		'tcp_authentication_limit'		=>array('tcpauthlimit',''),
		'enable_hostname_lookup'		=>array('srvlookup',''),
		'enable_internal_sip_call'		=>array('enable_internal_sip_call',''),
		'internal_sip_call_prefix'		=>array('internal_sip_call_prefix',''),
		'subscribe_network_change_event'	=>array('subscribe_network_change_event',''),
		'match_external_address_locally'	=>array('matchexternaddrlocally',''),
		'dynamic_exclude_static'		=>array('dynamic_exclude_static',''),
		'externally_mapped_tcp_port'		=>array('externtcpport',''),
		'external_address'			=>array('externaddr',''),
		'external_hostname'			=>array('externhost',''),
		'hostname_refresh_interval'		=>array('externrefresh',''),
		'strict_rfc_interpretation'		=>array('pedantic',''),
		'send_compact_headers'			=>array('compactheaders',''),
		'sdp_owner'				=>array('sdpowner',''),
		'ring_mode'				=>array('ring_mode','0'),
		'disallowed_sip_methods'		=>array('disallowed_methods',''),
		'notify_unlimited' 			=>  array('notify_unlimited', 'off'),
		'hangupcausecode'           =>  array('hangupcausecode',''),
		'shrink_caller_id'			=>array('shrinkcallerid',''),
		'fromtype_callid'			=>array('fromtype_callid',''),
		'sipto'					=> array('sipto', ''),
		'maximum_registration_expiry'		=>array('maxexpiry',''),
		'minimum_registration_expiry'		=>array('minexpiry',''),
		'default_registration_expiry'		=>array('defaultexpiry',''),
		'registration_timeout'			=>array('registertimeout',''),
		'number_of_registration_attemptsy'	=>array('registerattempts',''),
		'client_auto_flag'			=>array('client_auto_flag','No'),
		'client_basic_port'			=>array('client_basic_port','5060'),
		'client_port_step'			=>array('client_port_step',''),
		'client_reg_attempts'		=>array('client_reg_attempts','3'),
		'match_auth_username'			=>array('match_auth_username',''),
		'realm'					=>array('realm',''),
		'use_domain_as_realm'			=>array('domainasrealm',''),
		'always_auth_reject'			=>array('alwaysauthreject',''),
		'authenticate_options_requests'		=>array('auth_options_request',''),
		'allow_guest_calling'			=>array('allowguest',''),
		'tos_for_sip_packets'			=>array('tos_sip',''),
		'tos_for_rtp_packets'			=>array('tos_audio',''),
		'rtptimeout'				=>array('rtptimeout',''),
		'language' 					=>array('language','en')
	);
	
	foreach($post_conf_array as $key => $value){
		if(isset($_POST[$key])) {
			$tmp = $_POST[$key];
		}else{
			$tmp = $value[1];
		}
		if($key == 'notify_unlimited' && isset($_POST['notify_unlimited'])){
			$tmp = 'on';
		}
		if($key == 'client_auto_flag' && isset($_POST['client_auto_flag'])){
			$tmp = 'Yes';
		}
		
		if($key == 'external_address' && $_POST['external_address'] != ''){
			if($_POST['external_address_port'] != ''){
				$tmp = $_POST[$key].':'.$_POST['external_address_port'];
			}
		}
		if($key == 'external_address' && isset($_POST['auto_update_externaddr'])){
			$externaddr = $exist_array['general']['externaddr'];
			$temp_ip = explode(':',$externaddr);
			if($_POST['external_address_port'] != ''){
				$tmp = $temp_ip[0].':'.$_POST['external_address_port'];
			}else{
				$tmp = $temp_ip[0];
			}
		}
		
		if(isset($exist_array['general'][$value[0]])) {
			if(trim($tmp) != '') {
				$aql->assign_editkey('general',$value[0],$tmp);
			} else {
				$aql->assign_delkey('general',$value[0]);
			}
		} else {
			$aql->assign_append('general',$value[0],$tmp);
		}
	}

	// get user localnet
	$localnet = array();
	if(isset($_POST['local_network_list']) && is_array($_POST['local_network_list']) && $_POST['local_network_list'] != '') {
		foreach($_POST['local_network_list'] as $each) {
			$localnet[] = $each;
		}
	} 
	$interface_type = `/my_tools/set_config /etc/asterisk/gw/network/lan.conf get option_value general type`;
	if('static' == $interface_type){
		$ip = `/my_tools/set_config /etc/asterisk/gw/network/lan.conf get option_value ipv4 ipaddr`;
		$netmask = `/my_tools/set_config /etc/asterisk/gw/network/lan.conf get option_value ipv4 netmask`;
		$local_net_info = $ip . '/' . $netmask;
		if(!in_array($local_net_info, $localnet)){
			$localnet[] = $local_net_info;
		}
	}

	// set localnet
	if(isset($exist_array['general']['localnet'])){
		$aql->assign_delkey('general','localnet');
	}
	foreach($localnet as $value){
		$aql->assign_append('general','localnet',$value);
	}

	//Fix data
	if(isset($exist_array['general']['context'])) {
		$aql->assign_editkey('general','context','sipinbound');
	} else {
		$aql->assign_append('general','context','sipinbound');
	}

	if(isset($exist_array['general']['useragent'])) {
		$aql->assign_editkey('general','useragent','VoxStack Wireless Gateway');
	} else {
		$aql->assign_append('general','useragent','VoxStack Wireless Gateway');
	}

	if(isset($exist_array['general']['sdpsession'])) {
		$aql->assign_editkey('general','sdpsession','VoxStack Wireless Gateway');
	} else {
		$aql->assign_append('general','sdpsession','VoxStack Wireless Gateway');
	}

	if(isset($exist_array['general']['disallow'])) {
		$aql->assign_editkey('general','disallow','all');
	} else {
		$aql->assign_append('general','disallow','all');
	}

	$allow_val = '';
	for($i=1; $i<=7; $i++) {
		$name='sip_codec_priority'.$i;
		if(isset($_POST[$name]) && $_POST[$name] != '') {
			$val = $_POST[$name];
			$allow_val .= "$val,"; 
		}
	}
	$allow_val = rtrim($allow_val,',');

	if(isset($exist_array['general']['allow'])) {
		$aql->assign_editkey('general','allow',$allow_val);
	} else {
		$aql->assign_append('general','allow',$allow_val);
	}

	if (!$aql->save_config_file('sip_general.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	unlock_file($hlock);
	
	//hangupcausecodeunavaline save to extra-global.conf
	$global_hlock = lock_file("/etc/asterisk/extra-global.conf");
	$extra_global_array = $aql->query("select * from extra-global.conf");
	
	if(!isset($extra_global_array['channels'])){
		$aql->assign_addsection('channels','');
	}
	
	if(isset($extra_global_array['channels']['hangupcausecodeunavaline'])){
		$aql->assign_editkey('channels','hangupcausecodeunavaline',$_POST['hangupcausecodeunavaline']);
	}else{
		$aql->assign_append('channels','hangupcausecodeunavaline',$_POST['hangupcausecodeunavaline']);
	}
	
	if (!$aql->save_config_file('extra-global.conf')) {
		echo $aql->get_error();
		unlock_file($global_hlock);
		return;
	}
	unlock_file($global_hlock);
}


function save_to_rtp_conf()
{
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk');
	if (!$setok) {
		echo $aql->get_error();
		return;
	}

	$rtp_conf_path='/etc/asterisk/rtp.conf';
	$hlock = lock_file($rtp_conf_path);

	if(!$aql->open_config_file($rtp_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}

	$exist_array = $aql->query("select * from rtp.conf where section='general'");

	if(!isset($exist_array['general'])) {
		$aql->assign_addsection('general','');
	}

	if(isset($_POST['start_of_rtp_port_range'])) {
		$val = $_POST['start_of_rtp_port_range'];
		if(isset($exist_array['general']['rtpstart'])) {
			$aql->assign_editkey('general','rtpstart',$val);
		} else {
			$aql->assign_append('general','rtpstart',$val);
		} 
	}

	if(isset($_POST['end_of_rtp_port_range'])) {
		$val = $_POST['end_of_rtp_port_range'];
		if(isset($exist_array['general']['rtpend'])) {
			$aql->assign_editkey('general','rtpend',$val);
		} else {
			$aql->assign_append('general','rtpend',$val);
		} 
	}
	if (!$aql->save_config_file('rtp.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	unlock_file($hlock);
}

function check_auto_update_externaddr(){
	$gw_conf_path = "/etc/asterisk/gw.conf";
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk');
	if (!$setok) {
		echo $aql->get_error();
		return;
	}
	$hlock = lock_file($gw_conf_path);

	if(!$aql->open_config_file($gw_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	$exist_array = $aql->query("select * from gw.conf where section='update-externaddr'");

	if(!isset($exist_array['update-externaddr'])) {
		$aql->assign_addsection('update-externaddr','');
	}
	if(isset($_POST['auto_update_externaddr'])) {
		$val = $_POST['auto_update_externaddr'];
		if(isset($exist_array['update-externaddr']['auto_update_externaddr'])) {
			$aql->assign_editkey('update-externaddr','auto_update_externaddr',$val);
		} else {
			$aql->assign_append('update-externaddr','auto_update_externaddr',$val);
		} 
	} else {
		if(isset($exist_array['update-externaddr']['auto_update_externaddr'])) {
			$aql->assign_editkey('update-externaddr','auto_update_externaddr','');
		} else {
			$aql->assign_append('update-externaddr','auto_update_externaddr','');
		} 
	}
	if (!$aql->save_config_file('gw.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	unlock_file($hlock);
	if(isset($_POST['auto_update_externaddr'])){
		exec("/my_tools/AutoGetPublicIP.sh > /dev/null 2>&1");
		exec("/etc/init.d/checkAutoUpdatePublicIP start > /dev/null 2>&1 &");
	} else {
		exec("/etc/init.d/checkAutoUpdatePublicIP stop > /dev/null 2>&1 &");
	}

}

function save_to_gw_general(){
	if(!file_exists('/etc/asterisk/gw_general.conf')){
		exec("touch /etc/asterisk/gw_general.conf");
	}
	
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_general.conf');
	$exist_general_section = $aql->query("select * from gw_general.conf");
	if(!isset($exist_general_section['general'])){
		$aql->assign_addsection('general','');
	}
	
	$caller_id_1 = $_POST['caller_id_1'];
	
	$caller_id_2 = $_POST['caller_id_2'];
	
	$callee_id_1 = $_POST['callee_id_1'];
	
	$callee_id_2 = $_POST['callee_id_2'];
	
	if(isset($_POST['sipinbound'])){
		$sipinbound = 'on';
	}else{
		$sipinbound = 'off';
	}
	
	if(isset($exist_general_section['general']['caller_id_1'])){
		$aql->assign_editkey('general','caller_id_1',$caller_id_1);
	}else{
		$aql->assign_append('general','caller_id_1',$caller_id_1);
	}
	
	if(isset($exist_general_section['general']['caller_id_2'])){
		$aql->assign_editkey('general','caller_id_2',$caller_id_2);
	}else{
		$aql->assign_append('general','caller_id_2',$caller_id_2);
	}
	
	if(isset($exist_general_section['general']['callee_id_1'])){
		$aql->assign_editkey('general','callee_id_1',$callee_id_1);
	}else{
		$aql->assign_append('general','callee_id_1',$callee_id_1);
	}
	
	if(isset($exist_general_section['general']['callee_id_2'])){
		$aql->assign_editkey('general','callee_id_2',$callee_id_2);
	}else{
		$aql->assign_append('general','callee_id_2',$callee_id_2);
	}
	
	if(isset($exist_general_section['general']['sipinbound'])){
		$aql->assign_editkey('general','sipinbound',$sipinbound);
	}else{
		$aql->assign_append('general','sipinbound',$sipinbound);
	}
	
	//auto_extern
	if(!isset($exist_general_section['auto_extern'])){
		$aql->assign_addsection('auto_extern','');
	}
	
	if(isset($exist_general_section['auto_extern']['externaddr'])){
		$aql->assign_editkey('auto_extern', 'externaddr', '');
	}else{
		$aql->assign_append('auto_extern', 'externaddr', '');
	}
		
	$aql->save_config_file('gw_general.conf');
	unlock_file($hlock);
}

function save_data()
{
	save_to_gw_general();
	save_to_sip_general_conf();
	save_to_rtp_conf();
	save_routings_to_extensions();
	check_auto_update_externaddr();
}

function make_update_file_path(){
	$product_type = get_product_type();
	if($product_type < 4){
		$file_path="/tmp/update_file_".date("YmdHim");
	}else{
		$file_path="/data/update_file_".date("YmdHim");
	}
	$tmp = $file_path;

	$i=0;
	while(file_exists($file_path)) {
		$i++;
		$file_path = $tmp."$i";
	}

	return $file_path;
}

function upload_language_file(){
	if(!$_FILES){
		return false;
	}
	?>
		<div class="modal fade in" id="modal-51">
			<div class="modal-dialog">
				<div class="modal-content">
				
					<div class="modal-header">
						<h4 class="modal-title"><?php echo language('Language').' '.language('File Upload');?></h4>
					</div>
					
					<div class="modal-body" style="min-height:200px;">
						<?php 
						if(isset($_FILES['language_file']['error']) && $_FILES['language_file']['error'] == 0) {
							if(!(isset($_FILES['language_file']['size'])) || $_FILES['language_file']['size'] > 1024*1024) {
								echo '<span style="color:red">'.language('Error').': '.language('Language Package Size alert','Uploaded max file is 1MB!').'</span>';
							}else{
								$store_file = make_update_file_path();

								if (!move_uploaded_file($_FILES['language_file']['tmp_name'], $store_file)) {
									echo '<span style="color:red">'.language('Error').'</span>';
								}else{
									exec("rm -rf /tmp/sip_language_file/");
									exec('mkdir /tmp/sip_language_file');
									exec("tar vxz -f $store_file -C /tmp/sip_language_file");
									exec("find /dev/shm/sip_language_file/", $output);
									$n = 0;
									for($i=0;$i<count($output);$i++){
										if(strstr($output[$i],'.')){
											$temp = explode('/',$output[$i]);
											
											$length = count($temp);
											$file_name = $temp[$length-1];
											if($file_name == 'agent-pass.gsm' || $file_name == 'auth-incorrect.gsm' || $file_name == 'auth-thankyou.gsm' || $file_name == 'vm-goodbye.gsm'){
												$n++;
											}
										}
									}
									
									if($n >= 4){
										if(!dir('/etc/asterisk/sounds/custom/')){
											mkdir('/etc/asterisk/sounds/custom/');
										}
										exec("cp /tmp/sip_language_file/* /etc/asterisk/sounds/custom/ -a");
										save_to_flash('/etc/asterisk','/etc/cfg');
										echo language("Configuration Files Upload Succeeded");
									}else{
										echo language("Configuration Files Upload Failed");
									}
									
									exec("rm -rf /tmp/sip_language_file/");
								}
								exec("rm -rf $store_file");
							}
							echo '<div style="text-align:center;margin-top:30px;"><button id="button_cancel" type="button" style="margin-left:10px;">'.language('Close').'</button></div>';
						} else {
							if(isset($_FILES['language_file']['error'])) {
								switch($_FILES['language_file']['error']) {
								case 1:
									echo '<span style="color:red">'.language('Error').': '.language('System Update error 1',"The file was larger than the server space 80M!").'</span>';
									break;
								case 2:
									echo '<span style="color:red">'.language('Error').': '.language('System Update error 2',"The file was larger than the browser's limit!").'</span>';
									break;
								case 3:
									echo '<span style="color:red">'.language('Error').': '.language('System Update error 3',"The file was only partially uploaded!").'</span>';
									break;
								case 4:
									echo '<span style="color:red">'.language('Error').': '.language('System Update error 4',"Can not find uploaded file!").'</span>';
									break;
								case 5:
									echo '<span style="color:red">'.language('Error').': '.language('System Update error 5',"The server temporarily lost folder!").'</span>';
									break;
								case 6:
									echo '<span style="color:red">'.language('Error').': '.language('System Update error 6',"Failed to write to the temporary folder!").'</span>';
									break;
								}
							}
							echo language("System Update Failed");
							echo '<div style="text-align:center;margin-top:30px;"><button id="button_cancel" type="button" style="margin-left:10px;">'.language('Close').'</button></div>';
						}
						?>
					</div>
					<img src="/images/loading.gif" style="display:none;">
				</div>
			</div>
		</div>
		<div class="modal-backdrop fade_in"></div>
	<?php
}

function download_language_sample(){
	$download_filename = "sound_sample.tar.gz";
	$download_file = "/version/sound_sample.tar.gz";
	$file = fopen($download_file,"r");
	$size = filesize($download_file);
	
	header('Content-Encoding: none');
	header('Content-Type: application/force-download');
	header('Content-Type: application/octet-stream');
	header('Content-Type: application/download');
	header('Content-Description: File Transfer');
	header('Accept-Ranges: bytes');
	header("Accept-Length: $size");
	header('Content-Transfer-Encoding: binary');
	header("Content-Disposition: attachment; filename=$download_filename");
	header('Pragma: no-cache');
	header('Expires: 0');
	ob_clean();
	flush();
	echo fread($file, $size);
	fclose ($file);
}
?>

<?php
//Set Configs
//////////////////////////////////////////////////////////////////////////////////
if($_POST) {
	if(isset($_POST['send']) &&  $_POST['send'] == 'Save'){
		// handle master
		save_data();
		//ast_reload();
		//wait_apply("exec", "/my_tools/cluster_mode gen_sip_cluster_conf > /dev/null 2>&1");
		wait_apply("exec","asterisk -rx \"core reload\" > /dev/null 2>&1 &");

		// handle slave
		if(!prepare_slave_sip_general_conf('/etc/asterisk/sip_general.conf','/tmp/sip_general.conf')){
			return;
		}
	}else if($_POST['send'] == 'File Upload'){
		upload_language_file();
	}else if($_POST['send'] == 'Download Sample'){
		download_language_sample();
	}
}
///////////////////////////////////////////////////////////////////////////////////


//Get Configs
//////////////////////////////////////////////////////////////////////////////////
if($__deal_cluster__){
	$cluster_info = get_cluster_info();
}

$aql = new aql();
$setok = $aql->set('basedir','/etc/asterisk');
if (!$setok) {
	exit(255);
}

$hlock=lock_file("/etc/asterisk/sip_general.conf");
$sip_general_conf = $aql->query("select * from sip_general.conf where section='general'");
unlock_file($hlock);

if(isset($sip_general_conf['general']['udpbindaddr'])) {
	$len = strlen($sip_general_conf['general']['udpbindaddr']);
	if(strstr($sip_general_conf['general']['udpbindaddr'],"::")){
		$pos = strrpos($sip_general_conf['general']['udpbindaddr'],':') + 1;
	}else{
		$pos = strpos($sip_general_conf['general']['udpbindaddr'],':') + 1;
	}
	$udp_bind_port = trim(substr($sip_general_conf['general']['udpbindaddr'],$pos,$len-$pos));
} else {
	$udp_bind_port = 5060;
}

$enable_tcp['yes'] = '';
$enable_tcp['no'] = '';
if(isset($sip_general_conf['general']['tcpenable'])) {
	$val = trim($sip_general_conf['general']['tcpenable']);
	if(strcasecmp($val,"yes")) {
		$enable_tcp['no'] = 'selected';
	} else {
		$enable_tcp['yes'] = 'selected';
	}
} else {
	$enable_tcp['no'] = 'selected';
}

if(isset($sip_general_conf['general']['tcpbindaddr'])) {
	$len = strlen($sip_general_conf['general']['tcpbindaddr']);
	if(strstr($sip_general_conf['general']['tcpbindaddr'],"::")){
		$pos = strrpos($sip_general_conf['general']['tcpbindaddr'],':') + 1;
	}else{
		$pos = strpos($sip_general_conf['general']['tcpbindaddr'],':') + 1;
	}
	$tcp_bind_port = trim(substr($sip_general_conf['general']['tcpbindaddr'],$pos,$len-$pos));
} else {
	$tcp_bind_port = 5060;
}

if(isset($sip_general_conf['general']['tcpauthtimeout'])) {
	$tcp_authentication_timeout = trim($sip_general_conf['general']['tcpauthtimeout']);
} else {
	$tcp_authentication_timeout = '';
}

if(isset($sip_general_conf['general']['tcpauthlimit'])) {
	$tcp_authentication_limit = trim($sip_general_conf['general']['tcpauthlimit']);
} else {
	$tcp_authentication_limit = '';
}

$enable_hostname_lookup['yes'] = '';
$enable_hostname_lookup['no'] = '';
if(isset($sip_general_conf['general']['srvlookup'])) {
	$val = trim($sip_general_conf['general']['srvlookup']);
	if(strcasecmp($val,"yes")) {
		$enable_hostname_lookup['no'] = 'selected';
	} else {
		$enable_hostname_lookup['yes'] = 'selected';
	}
} else {
	$enable_hostname_lookup['no'] = 'selected';
}

$enable_internal_sip_call['yes'] = '';
$enable_internal_sip_call['no'] = '';
if(isset($sip_general_conf['general']['enable_internal_sip_call'])) {
	$val = trim($sip_general_conf['general']['enable_internal_sip_call']);
	if(strcasecmp($val,"yes")) {
		$enable_internal_sip_call['no'] = 'selected';
	} else {
		$enable_internal_sip_call['yes'] = 'selected';
	}
} else {
	$enable_internal_sip_call['no'] = 'selected';
}

if(isset($sip_general_conf['general']['internal_sip_call_prefix'])) {
	$internal_sip_call_prefix = trim($sip_general_conf['general']['internal_sip_call_prefix']);
} else {
	$internal_sip_call_prefix = '';
}

// get cluster localnet
$cluster_localnet = array();
if($cluster_info['mode'] == 'master') {
	if(isset($cluster_info['master_ip']) && $cluster_info['master_ip'] != ''){ 
		$cluster_localnet[] = $cluster_info['master_ip'].'/255.255.255.255';
	}    
	for($b = 2; $b <= $__BRD_SUM__; $b++){
		if(isset($cluster_info[$__BRD_HEAD__.$b.'_ip']) && $cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
			$cluster_localnet[] = $cluster_info[$__BRD_HEAD__.$b.'_ip'].'/255.255.255.255';
		}    
	}    
}else if($cluster_info['mode'] == 'slave'){
	if(isset($cluster_info['slave_ip']) && $cluster_info['slave_ip'] != '') {
		$cluster_localnet[] = $cluster_info['slave_ip'].'/255.255.255.255';
	}    
	if(isset($cluster_info['slave_masterip']) && $cluster_info['slave_masterip'] != ''){ 
		$cluster_localnet[] = $cluster_info['slave_masterip'].'/255.255.255.255';
	}    
}

// get localnet list
$local_network_list = array();
if(isset($sip_general_conf['general']) && is_array($sip_general_conf['general'])){
	foreach($sip_general_conf['general'] as $key => $value){
		if(strstr($key, 'localnet')){
			// skip cluster localnet
			foreach($cluster_localnet as $localnet){
				if(strstr($key, $localnet)){
					continue 2;
				}
			}
			$local_network_list[] = $value;
		}
	}
}

$subscribe_network_change_event['yes'] = '';
$subscribe_network_change_event['no'] = '';
if(isset($sip_general_conf['general']['subscribe_network_change_event'])) {
	$val = trim($sip_general_conf['general']['subscribe_network_change_event']);
	if(strcasecmp($val,"1")) {
		$subscribe_network_change_event['no'] = 'selected';
	} else {
		$subscribe_network_change_event['yes'] = 'selected';
	}
} else {
	$subscribe_network_change_event['no'] = 'selected';
}

$match_external_address_locally['yes'] = '';
$match_external_address_locally['no'] = '';
if(isset($sip_general_conf['general']['matchexternaddrlocally'])) {
	$val = trim($sip_general_conf['general']['matchexternaddrlocally']);
	if(strcasecmp($val,"yes")) {
		$match_external_address_locally['no'] = 'selected';
	} else {
		$match_external_address_locally['yes'] = 'selected';
	}
} else {
	$match_external_address_locally['no'] = 'selected';
}

$dynamic_exclude_static['yes'] = '';
$dynamic_exclude_static['no'] = '';
if(isset($sip_general_conf['general']['dynamic_exclude_static'])) {
	$val = trim($sip_general_conf['general']['dynamic_exclude_static']);
	if(strcasecmp($val,"yes")) {
		$dynamic_exclude_static['no'] = 'selected';
	} else {
		$dynamic_exclude_static['yes'] = 'selected';
	}
} else {
	$dynamic_exclude_static['no'] = 'selected';
}

if(isset($sip_general_conf['general']['externtcpport'])) {
	$externally_mapped_tcp_port = trim($sip_general_conf['general']['externtcpport']);
} else {
	$externally_mapped_tcp_port = '';
}

if(isset($sip_general_conf['general']['externaddr'])) {
	if(strstr($sip_general_conf['general']['externaddr'],':')){
		$temp = explode(':',$sip_general_conf['general']['externaddr']);
		$external_address = $temp[0];
		$external_address_port = $temp[1];
	}else{
		$external_address = trim($sip_general_conf['general']['externaddr']);
	}
} else {
	$external_address = '';
}

if(isset($sip_general_conf['general']['externhost'])) {
	$external_hostname = trim($sip_general_conf['general']['externhost']);
} else {
	$external_hostname = '';
}

if(isset($sip_general_conf['general']['externrefresh'])) {
	$hostname_refresh_interval = trim($sip_general_conf['general']['externrefresh']);
} else {
	$hostname_refresh_interval = '';
}

$strict_rfc_interpretation['yes'] = '';
$strict_rfc_interpretation['no'] = '';
if(isset($sip_general_conf['general']['pedantic'])) {
	$val = trim($sip_general_conf['general']['pedantic']);
	if(strcasecmp($val,"yes")) {
		$strict_rfc_interpretation['no'] = 'selected';
	} else {
		$strict_rfc_interpretation['yes'] = 'selected';
	}
} else {
	$strict_rfc_interpretation['no'] = 'selected';
}

$send_compact_headers['yes'] = '';
$send_compact_headers['no'] = '';
if(isset($sip_general_conf['general']['compactheaders'])) {
	$val = trim($sip_general_conf['general']['compactheaders']);
	if(strcasecmp($val,"yes")) {
		$send_compact_headers['no'] = 'selected';
	} else {
		$send_compact_headers['yes'] = 'selected';
	}
} else {
	$send_compact_headers['no'] = 'selected';
}

if(isset($sip_general_conf['general']['sdpowner'])) {
	$sdp_owner = trim($sip_general_conf['general']['sdpowner']);
} else {
	$sdp_owner = '';
}

if(isset($sip_general_conf['general']['ring_mode'])){
	$ring_mode = trim($sip_general_conf['general']['ring_mode']);
}else{
	$ring_mode = 0;
}

$disallowed_sip_methods['ACK'] = '';
$disallowed_sip_methods['BYE'] = '';
$disallowed_sip_methods['CANCEL'] = '';
$disallowed_sip_methods['INFO'] = '';
$disallowed_sip_methods['INVITE'] = '';
$disallowed_sip_methods['MESSAGE'] = '';
$disallowed_sip_methods['NOTIFY'] = '';
$disallowed_sip_methods['OPTIONS'] = '';
$disallowed_sip_methods['PRACK'] = '';
$disallowed_sip_methods['PUBLISH'] = '';
$disallowed_sip_methods['REFER'] = '';
$disallowed_sip_methods['REGISTER'] = '';
$disallowed_sip_methods['SUBSCRIBE'] = '';
$disallowed_sip_methods['UPDATE'] = '';
if(isset($sip_general_conf['general']['disallowed_methods'])) {
	$array = explode(',', $sip_general_conf['general']['disallowed_methods']);
	foreach($array as $each) {
		$val = trim($each);
		if($val != '') {
			$disallowed_sip_methods[$val] = 'checked';
		}
	}
}

$notify_unlimited_checked = '';
if(isset($sip_general_conf['general']['notify_unlimited'])){
	if($sip_general_conf['general']['notify_unlimited'] == 'on'){
		$notify_unlimited_checked = 'checked';
	}
}

$hangupcausecode['default'] = '';
$hangupcausecode['1'] = '';
$hangupcausecode['3'] = '';
$hangupcausecode['17'] = '';
$hangupcausecode['18'] = '';
$hangupcausecode['19'] = '';
$hangupcausecode['21'] = '';
$hangupcausecode['22'] = '';
$hangupcausecode['27'] = '';
$hangupcausecode['28'] = '';
$hangupcausecode['29'] = '';
$hangupcausecode['38'] = '';
$hangupcausecode['42'] = '';
$hangupcausecode['58'] = '';
$hangupcausecode['127'] = '';
if(isset($sip_general_conf['general']['hangupcausecode'])) {
	$val = trim($sip_general_conf['general']['hangupcausecode']);
	if($val == ''){
		$hangupcausecode['default'] = 'selected';
	} else {
		$hangupcausecode[$val] = 'selected';
	}
} else {
	$hangupcausecode['default'] = 'selected';
}

$extra_global_conf = $aql->query("select * from extra-global.conf");
$hangupcausecodeunavaline['default'] = '';
$hangupcausecodeunavaline['1'] = '';
$hangupcausecodeunavaline['3'] = '';
$hangupcausecodeunavaline['17'] = '';
$hangupcausecodeunavaline['18'] = '';
$hangupcausecodeunavaline['19'] = '';
$hangupcausecodeunavaline['21'] = '';
$hangupcausecodeunavaline['22'] = '';
$hangupcausecodeunavaline['27'] = '';
$hangupcausecodeunavaline['28'] = '';
$hangupcausecodeunavaline['29'] = '';
$hangupcausecodeunavaline['38'] = '';
$hangupcausecodeunavaline['42'] = '';
$hangupcausecodeunavaline['58'] = '';
$hangupcausecodeunavaline['127'] = '';
if(isset($extra_global_conf['channels']['hangupcausecodeunavaline'])) {
	$val = trim($extra_global_conf['channels']['hangupcausecodeunavaline']);
	if($val == ''){
		$hangupcausecodeunavaline['default'] = 'selected';
	} else {
		$hangupcausecodeunavaline[$val] = 'selected';
	}
} else {
	$hangupcausecodeunavaline['default'] = 'selected';
}

$shrink_caller_id['yes'] = '';
$shrink_caller_id['no'] = '';
if(isset($sip_general_conf['general']['shrinkcallerid'])) {
	$val = trim($sip_general_conf['general']['shrinkcallerid']);
	if(strcasecmp($val,"yes")) {
		$shrink_caller_id['no'] = 'selected';
	} else {
		$shrink_caller_id['yes'] = 'selected';
	}
} else {
	$shrink_caller_id['no'] = 'selected';
}

$sipto[0] = '';
$sipto[1] = '';
if(isset($sip_general_conf['general']['sipto'])) {
	$val = trim($sip_general_conf['general']['sipto']);
	if ($val == '0') {
		$sipto[0] = 'selected';
	} else if($val == '1') {
		$sipto[1] = 'selected';
	} else {

	}
}

$sipfrom[0] = '';
$sipfrom[1] = '';
$sipfrom[2] = '';
$sipfrom[3] = '';
if(isset($sip_general_conf['general']['fromtype_callid'])){
	$val = trim($sip_general_conf['general']['fromtype_callid']);
	if($val == '0'){
		$sipfrom[0] = 'selected';
	}else if($val == '1'){
		$sipfrom[1] = 'selected';
	}else if($val == '2'){
		$sipfrom[2] = 'selected';
	}else{
		$sipfrom[3] = 'selected';
	}
}

if(isset($sip_general_conf['general']['maxexpiry'])) {
	$maximum_registration_expiry = trim($sip_general_conf['general']['maxexpiry']);
} else {
	$maximum_registration_expiry = '';
}
 
if(isset($sip_general_conf['general']['minexpiry'])) {
	$minimum_registration_expiry = trim($sip_general_conf['general']['minexpiry']);
} else {
	$minimum_registration_expiry = '';
}

if(isset($sip_general_conf['general']['defaultexpiry'])) {
	$default_registration_expiry = trim($sip_general_conf['general']['defaultexpiry']);
} else {
	$default_registration_expiry = '';
}

if(isset($sip_general_conf['general']['registertimeout'])) {
	$registration_timeout = trim($sip_general_conf['general']['registertimeout']);
} else {
	$registration_timeout = '';
}

if(isset($sip_general_conf['general']['registerattempts'])) {
	$number_of_registration_attemptsy = trim($sip_general_conf['general']['registerattempts']);
} else {
	$number_of_registration_attemptsy = '0';
}

if(isset($sip_general_conf['general']['client_auto_flag']) && $sip_general_conf['general']['client_auto_flag'] == 'Yes'){
	$client_auto_flag_checked = 'checked';
}else{
	$client_auto_flag_checked = '';
}

if(isset($sip_general_conf['general']['client_basic_port'])){
	$client_basic_port = $sip_general_conf['general']['client_basic_port'];
}else{
	$client_basic_port = '5060';
}

if(isset($sip_general_conf['general']['client_port_step'])){
	$client_port_step = $sip_general_conf['general']['client_port_step'];
}else{
	$client_port_step = '';
}

if(isset($sip_general_conf['general']['client_reg_attempts'])){
	$client_reg_attempts = $sip_general_conf['general']['client_reg_attempts'];
}else{
	$client_reg_attempts = '3';
}

$match_auth_username['yes'] = '';
$match_auth_username['no'] = '';
if(isset($sip_general_conf['general']['match_auth_username'])) {
	$val = trim($sip_general_conf['general']['match_auth_username']);
	if(strcasecmp($val,"yes")) {
		$match_auth_username['no'] = 'selected';
	} else {
		$match_auth_username['yes'] = 'selected';
	}
} else {
	$match_auth_username['no'] = 'selected';
}

if(isset($sip_general_conf['general']['realm'])) {
	$realm = trim($sip_general_conf['general']['realm']);
} else {
	$realm = '';
}

$use_domain_as_realm['yes'] = '';
$use_domain_as_realm['no'] = '';
if(isset($sip_general_conf['general']['domainasrealm'])) {
	$val = trim($sip_general_conf['general']['domainasrealm']);
	if(strcasecmp($val,"yes")) {
		$use_domain_as_realm['no'] = 'selected';
	} else {
		$use_domain_as_realm['yes'] = 'selected';
	}
} else {
	$use_domain_as_realm['no'] = 'selected';
}

$always_auth_reject['yes'] = '';
$always_auth_reject['no'] = '';
if(isset($sip_general_conf['general']['alwaysauthreject'])) {
	$val = trim($sip_general_conf['general']['alwaysauthreject']);
	if(strcasecmp($val,"yes")) {
		$always_auth_reject['no'] = 'selected';
	} else {
		$always_auth_reject['yes'] = 'selected';
	}
} else {
	$always_auth_reject['no'] = 'selected';
}

$authenticate_options_requests['yes'] = '';
$authenticate_options_requests['no'] = '';
if(isset($sip_general_conf['general']['auth_options_request'])) {
	$val = trim($sip_general_conf['general']['auth_options_request']);
	if(strcasecmp($val,"yes")) {
		$authenticate_options_requests['no'] = 'selected';
	} else {
		$authenticate_options_requests['yes'] = 'selected';
	}
} else {
	$authenticate_options_requests['no'] = 'selected';
}

$allow_guest_calling['yes'] = '';
$allow_guest_calling['no'] = '';
if(isset($sip_general_conf['general']['allowguest'])) {
	$val = trim($sip_general_conf['general']['allowguest']);
	if(strcasecmp($val,"yes")) {
		$allow_guest_calling['no'] = 'selected';
	} else {
		$allow_guest_calling['yes'] = 'selected';
	}
} else {
	$allow_guest_calling['no'] = 'selected';
}

if(isset($sip_general_conf['general']['tos_sip'])) {
	$tos_for_sip_packets = trim($sip_general_conf['general']['tos_sip']);
} else {
	$tos_for_sip_packets = '';
}

if(isset($sip_general_conf['general']['tos_audio'])) {
	$tos_for_rtp_packets = trim($sip_general_conf['general']['tos_audio']);
} else {
	$tos_for_rtp_packets = '';
}

if(isset($sip_general_conf['general']['rtptimeout'])) {
	$rtptimeout = trim($sip_general_conf['general']['rtptimeout']);
} else {
	$rtptimeout = 120;
}

for($i=1; $i<=7; $i++) {
	$sip_codec_priority[$i]['notuse'] = '';
	$sip_codec_priority[$i]['ulaw'] = '';
	$sip_codec_priority[$i]['alaw'] = '';
	$sip_codec_priority[$i]['gsm'] = '';
	$sip_codec_priority[$i]['g722'] = '';
	$sip_codec_priority[$i]['g723'] = '';
	$sip_codec_priority[$i]['g726'] = '';
	$sip_codec_priority[$i]['g729'] = '';
}

if(isset($sip_general_conf['general']['allow'])) {
	$value = trim($sip_general_conf['general']['allow']);
	$allow = explode(',',$value);

	$i=1;
	foreach($allow as $each) {
		switch($each) {
		case 'ulaw':
			$sip_codec_priority[$i]['ulaw'] = 'selected';
			break;
		case 'alaw':
			$sip_codec_priority[$i]['alaw'] = 'selected';
			break;
		case 'gsm':
			$sip_codec_priority[$i]['gsm'] = 'selected';
			break;
		case 'g722':
			$sip_codec_priority[$i]['g722'] = 'selected';
			break;
		case 'g723':
			$sip_codec_priority[$i]['g723'] = 'selected';
			break;
		case 'g726':
			$sip_codec_priority[$i]['g726'] = 'selected';
			break;
		case 'g729':
			$sip_codec_priority[$i]['g729'] = 'selected';
			break;
		default:
			$sip_codec_priority[$i]['notuse'] = 'selected';
			break;
		}
		if($i++>6)
			break;
	}
} else { //Default must set codec priority
	$sip_codec_priority[1]['ulaw'] = 'selected';
	$sip_codec_priority[2]['alaw'] = 'selected';
	$sip_codec_priority[3]['gsm'] = 'selected';
	$sip_codec_priority[4]['g722'] = 'selected';
	$sip_codec_priority[5]['g723'] = 'selected';
	$sip_codec_priority[6]['g726'] = 'selected';
	$sip_codec_priority[7]['g729'] = 'selected';
}
//gw.conf
$hlock = lock_file("/etc/asterisk/gw.conf");
$update_externaddr = $aql->query("select * from gw.conf where section='update-externaddr'");
unlock_file($hlock);
if(isset($update_externaddr['update-externaddr']['auto_update_externaddr']) && trim($update_externaddr['update-externaddr']['auto_update_externaddr']) != ''){
	$auto_update_externaddr = "yes";
} else {
	$auto_update_externaddr = '';
}

//rtp.conf
$hlock=lock_file("/etc/asterisk/rtp.conf");
$rtp_conf = $aql->query("select * from rtp.conf where section='general'");
unlock_file($hlock);
if(isset($rtp_conf['general']['rtpstart'])) {
	$start_of_rtp_port_range = trim($rtp_conf['general']['rtpstart']);
} else {
	$start_of_rtp_port_range = 10000;
}

if(isset($rtp_conf['general']['rtpend'])) {
	$end_of_rtp_port_range = trim($rtp_conf['general']['rtpend']);
} else {
	$end_of_rtp_port_range = 20000;
}

//gw_general.conf
$hlock=lock_file("/etc/asterisk/gw_general.conf");
$general_conf = $aql->query("select * from gw_general.conf where section='general'");
unlock_file($hlock);

$caller_id_1['EXTEN'] = '';
$caller_id_1['From'] = 'selected';
$caller_id_1['To'] = '';
if(isset($general_conf['general']['caller_id_1'])){
	if($general_conf['general']['caller_id_1'] == 'EXTEN'){
		$caller_id_1['EXTEN'] = 'selected';
		$caller_id_1['From'] = '';
	}else if($general_conf['general']['caller_id_1'] == 'From'){
		$caller_id_1['From'] = 'selected';
	}else if($general_conf['general']['caller_id_1'] == 'To'){
		$caller_id_1['To'] = 'selected';
		$caller_id_1['From'] = '';
	}
}

$caller_id_2['Number'] = '';
$caller_id_2['Name'] = '';
if($general_conf['general']['caller_id_2'] == 'Number'){
	$caller_id_2['Number'] = 'selected';
}else if($general_conf['general']['caller_id_2'] == 'Name'){
	$caller_id_2['Name'] = 'selected';
}

$callee_id_1['EXTEN'] = '';
$callee_id_1['From'] = '';
$callee_id_1['To'] = '';
if(isset($general_conf['general']['callee_id_1'])){
	if($general_conf['general']['callee_id_1'] == 'EXTEN'){
		$callee_id_1['EXTEN'] = 'selected';
	}else if($general_conf['general']['callee_id_1'] == 'From'){
		$callee_id_1['From'] = 'selected';
	}else if($general_conf['general']['callee_id_1'] == 'To'){
		$callee_id_1['To'] = 'selected';
	}
}

$callee_id_2['Number'] = '';
$callee_id_2['Name'] = '';
if(isset($general_conf['general']['callee_id_2'])){
	if($general_conf['general']['callee_id_2'] == 'Number'){
		$callee_id_2['Number'] = 'selected';
	}else if($general_conf['general']['callee_id_2'] == 'Name'){
		$callee_id_2['Name'] = 'selected';
	}
}

$sipinbound_checked = '';
if(isset($general_conf['general']['sipinbound']) && $general_conf['general']['sipinbound'] == 'on'){
	$sipinbound_checked = 'checked';
}

//language sounds
if(isset($sip_general_conf['general']['language'])){
	$sip_language = $sip_general_conf['general']['language'];
}else{
	$sip_language = 'en';
}

///////////////////////////////////////////////////////////////////////////////////
?>

<script type="text/javascript">
function addRow()
{
	value = document.getElementById('local_network').value;
	if(value == "")
		return;

	//添加一行
	var newTr = tab_lnl.insertRow(-1);

	//添加两列
	var newTd0 = newTr.insertCell(0);
	var newTd1 = newTr.insertCell(0);

	//设置列内容和属性
	str = '<button type="button" name="send" value="Delete" style="width:32px;height:32px;" onclick=\'if(confirm("Are you sure to delete you selected ?"))javascript:this.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode);\'><img src="/images/delete.gif"></button>';
	str += '<input type="hidden" name="local_network_list[]" value="' + value + '" />';

	newTd0.innerHTML = str;
	newTd1.innerHTML = value;
}
function click_externaddr_readonly(){
	if(document.getElementById('auto_update_externaddr')){
		var externaddr = document.getElementById('auto_update_externaddr');
		//console.log(externaddr.checked);
		if(externaddr.checked) {
			document.getElementById('external_address').readOnly = true;
		} else {
			document.getElementById('external_address').readOnly = false;
		}
	}
}
</script>

<script type="text/javascript" src="/js/jquery.ibutton.js"></script>
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />
<script type="text/javascript" src="/js/float_btn.js"></script>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_networking')" id="tab_networking_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_networking')"><?php echo language('Networking');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_networking')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>

	<div id="tab_networking" style="display:block">
		<table width="98%" class="tedit" align="right">
			<tr class="ttitle">
				<td colspan = 2>
				<?php echo language('General');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('UDP Bind Port');?>:
						<span class="showhelp">
						<?php echo language('UDP Bind Port help','Choose a port on which to listen for UDP traffic.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="udp_bind_port" value="<?php echo $udp_bind_port;?>" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Enable TCP');?>:
						<span class="showhelp">
						<?php echo language('Enable TCP help','Enable server for incoming TCP connection(default is no).');?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="enable_tcp" id="enable_tcp" onchange="enable_tcp_change();" >
						<option value="no"  <?php echo $enable_tcp['no'] ?> > <?php echo language('_No');?> </option>
						<option value="yes" <?php echo $enable_tcp['yes'] ?> > <?php echo language('_Yes');?> </option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('TCP Bind Port');?>:
						<span class="showhelp">
						<?php echo language('TCP Bind Port help','Choose a port on which to listen for TCP traffic.');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" id="tcp_bind_port" name="tcp_bind_port" value="<?php echo $tcp_bind_port;?>" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('TCP Authentication Timeout');?>:
						<span class="showhelp">
						<?php echo language('TCP Authentication Timeout help','
							The maximum number of seconds a client has to authenticate.<br/>
							If the client does not authenticate before this timeout expires,<br/>
							the client will be disconnected.(default value is: 30 seconds)');
						?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" id="tcp_authentication_timeout" name="tcp_authentication_timeout" value="<?php echo $tcp_authentication_timeout;?>" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('TCP Authentication Limit');?>:
						<span class="showhelp">
						<?php echo language('TCP Authentication Limit help','
							The maiximum number of unauthenticated sessions that will be<br/> 
							allowed to connect at any given time.(default is:50)');
						?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" id="tcp_authentication_limit" name="tcp_authentication_limit" value="<?php echo $tcp_authentication_limit;?>" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Enable Hostname Lookup');?>:
						<span class="showhelp">
						<?php echo language('Enable Hostname Lookup help','
							Enable DNS SRV lookups on outbound calls Note: the gateway only<br/> 
							uses the first host in SRV records Disabling DNS SRV lookups disables<br/>
							the ability to place SIP calls based on domain names to some other SIP<br/>
							users on the Internet Sepcifying a port in a SIP peer definition or when<br/>
							dialing outbound calls with supress SRV lookups for that peer or call.');
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="enable_hostname_lookup" id="enable_hostname_lookup">
						<option value="no"  <?php echo $enable_hostname_lookup['no'];?> > <?php echo language('_No');?> </option>
						<option value="yes" <?php echo $enable_hostname_lookup['yes'];?> > <?php echo language('_Yes');?> </option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Enable Internal SIP Call');?>:
						<span class="showhelp">
						<?php echo language('Enable Internal SIP Call help',"
							Whether enable the internal SIP calls or not when you select the registration option \"Endpoint registers with this gateway\".");
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="enable_internal_sip_call" id="enable_internal_sip_call" onchange="enable_internal_sip_call_change();">
						<option value="no"  <?php echo $enable_internal_sip_call['no'];?> > <?php echo language('_No');?> </option>
						<option value="yes" <?php echo $enable_internal_sip_call['yes'];?> > <?php echo language('_Yes');?> </option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Internal SIP Call Prefix');?>:
						<span class="showhelp">
						<?php echo language('Internal SIP Call Prefix help','Specify a prefix before routing the internal calls.');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" id="internal_sip_call_prefix" name="internal_sip_call_prefix" value="<?php echo $internal_sip_call_prefix;?>" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Sounds');?>:
						<span class="showhelp">
						<?php echo language('Sounds help','Select the required language voice prompt.');?>
						</span>
					</div>
				</th>
				<td>
					<select name="language" id="language">
						<option value="cn" <?php if($sip_language == 'cn') echo 'selected'; ?> ><?php echo language('Chinese');?></option>
						<option value="en" <?php if($sip_language == 'en') echo 'selected'; ?> ><?php echo language('English');?></option>
						<option value="ar" <?php if($sip_language == 'ar') echo 'selected'; ?> ><?php echo language('Arab');?></option>
						<option value="es" <?php if($sip_language == 'es') echo 'selected'; ?> ><?php echo language('Spain');?></option>
						<option value="jp" <?php if($sip_language == 'jp') echo 'selected'; ?> ><?php echo language('Japanese');?></option>
						<option value="fr" <?php if($sip_language == 'fr') echo 'selected'; ?> ><?php echo language('French');?></option>
						<option value="ru" <?php if($sip_language == 'ru') echo 'selected'; ?> ><?php echo language('Russian');?></option>
						<option value="se" <?php if($sip_language == 'se') echo 'selected'; ?> ><?php echo language('Swedish');?></option>
						<option value="it" <?php if($sip_language == 'it') echo 'selected'; ?> ><?php echo language('Italian');?></option>
						
						<?php
						if(file_exists('/etc/asterisk/sounds/custom/agent-pass.gsm') &&
							file_exists('/etc/asterisk/sounds/custom/auth-incorrect.gsm') &&
							file_exists('/etc/asterisk/sounds/custom/auth-thankyou.gsm') &&
							file_exists('/etc/asterisk/sounds/custom/vm-goodbye.gsm')){
						?>
						<option value="custom" <?php if($sip_language == 'custom') echo 'selected'; ?> ><?php echo language('Custom');?></option>
						<?php } ?>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Sounds File Upload');?>:
						<span class="showhelp">
						<?php echo language('Sounds File Upload help','The format of the uploaded file must be tar.gz, <br> the file in the compressed package must contain agent-pass.gsm auth-incorrect.gsm auth-thankyou.gsm vm-goodbye.gsm, <br> and the file size cannot exceed 1m');?>
						</span>
					</div>
				</th>
				<td>
					<input type="file" name="language_file" id="language_file" onchange="return checkFileChange(this)"/>
					<input type="submit" value="<?php echo language('File Upload');?>" style="float:right;margin-right:30px;"
						onclick="document.getElementById('send').value='File Upload';return upload_cfg_file2();"/>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Download Sounds Sample');?>:
						<span class="showhelp">
						<?php echo language('Download Sounds Sample help','Download Sounds Sample');?>
						</span>
					</div>
				</th>
				<td>
					<input type="submit" value="<?php echo language('Download');?>" 
						onclick="document.getElementById('send').value='Download Sample';"/>
				</td>
			</tr>
		</table>
		<div id="newline"></div>

		<table width="98%" class="tedit" align="right">
			<tr class="ttitle">
				<td colspan = 2>
				<?php echo language('NAT Settings');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Local Network');?>:
						<span class="showhelp">
						<?php echo language('Local Network help','
							Format:192.168.0.0/255.255.0.0 or 172.16.0.0./12<br/>
							A list of IP address or IP ranges which are located inside a NATed network.<br/> 
							This gateway will replace the internal IP address in SIP and SDP messages with<br/> 
							the extenal IP address when a NAT exists between the gateway and other endpoings.');
						?>
						</span>
					</div>
				</th>
				<td >
					<input id="local_network" type="text" name="local_network" value="" style="width:300px;" />
					<input type="button" name="send" value="<?php echo language('Add');?>"  onclick="addRow();"/>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Local Network List');?>:
						<span class="showhelp">
						<?php echo language('Local Network List help','Local IP address list that you added.');?>
						</span>
					</div>
				</th>
				<td>
					<table width="100%" class="tshow" id="tab_lnl">
						<tr>
							<th><?php echo language('IP Range');?></th>
							<th style="width:10%"><?php echo language('Action');?></th>
						</tr>

						<?php foreach($local_network_list as $each) {?>
						<tr>
							<td>
								<?php echo $each;?>
							</td>
							<td>
								<?php 
									$interface_type = `/my_tools/set_config /etc/asterisk/gw/network/lan.conf get option_value general type`;
									$ip = `/my_tools/set_config /etc/asterisk/gw/network/lan.conf get option_value ipv4 ipaddr`;
									$netmask = `/my_tools/set_config /etc/asterisk/gw/network/lan.conf get option_value ipv4 netmask`;
									$local_net_info = $ip . '/' . $netmask;
									if(!('static' == $interface_type && $local_net_info == $each)){
								?>
								<button type="button" name="send" value="Delete" style="width:32px;height:32px;" 
									onclick='if(confirm("Are you sure to delete you selected ?"))javascript:this.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode);'>
									<img src="/images/delete.gif">
								</button>
								<?php } ?>
								<input type="hidden" name="local_network_list[]" value="<?php echo $each;?>" />
							</td>
						</tr>
						<?php }?>
					</table>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Subscribe Network Change Event');?>:
						<span class="showhelp">
						<?php echo language('Subscribe Network Change Event help','
							Through the use of the test_stun_monitor module, the gateway has the ability to detect when<br/>
							the perceived external network address has changed. When the stun_monitor is installed and <br/>
							configured, chan_sip will renew all outbound registrations when the monitor detects any sort<br/>
							of network change has occurred. By default this option is enabled, but only takes effect once<br/>
							res_stun_monitor is configured. If res_stun_monitor is enabled and you wish to not generate all<br/>
							outbound registrations on a network change,use the option below to disable this feature.');
						?>
						</span>
					</div>
				</th>
				<td>
					<select size=1 name="subscribe_network_change_event" id="subscribe_network_change_event">
						<option value="0" <?php echo $subscribe_network_change_event['no'];?> > <?php echo language('_No');?> </option>
						<option value="1" <?php echo $subscribe_network_change_event['yes'];?> > <?php echo language('_Yes');?> </option>
					</select>
				</td>
			</tr>
		<!--</table>
		<div id="newline"></div>
		<div id="tab">
			<li class="tb_indent">&nbsp;</li>
			<li class="tb_fold" onclick="lud(this,'tab_adv_nat_set')" id="tab_adv_nat_set_li">&nbsp;</li>
			<li class="tbg_fold" onclick="lud(this,'tab_adv_nat_set')"><?php echo language('Advanced:NAT Settings');?></li>
			<li class="tb2_fold" onclick="lud(this,'tab_adv_nat_set')">&nbsp;</li>
			<li class="tb_end2">&nbsp;</li>
		</div>
		<div id="tab_adv_nat_set" style="display:none">
			<table width="96%" class="tedit" align="right">-->
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Match External Address Locally');?>:
							<span class="showhelp">
							<?php echo language('Match External Address Locally help','Only substitute the externaddr or externhost setting if it matches.');?>
							</span>
						</div>
					</th>
					<td >
						<select size=1 name="match_external_address_locally">
							<option value="no" <?php echo $match_external_address_locally['no'];?> > <?php echo language('_No');?> </option>
							<option value="yes" <?php echo $match_external_address_locally['yes'];?> > <?php echo language('_Yes');?> </option>
						</select>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Dynamic Exclude Static');?>:
							<span class="showhelp">
							<?php echo language('Dynamic Exclude Static help','
								Disallow all dynamic hosts from registering as any IP address<br/> 
								used for staticly defined hosts. This helps avoid the configuration<br/>
								error of allowing your users to register at the same address as a SIP provider.');
							?>
							</span>
						</div>
					</th>
					<td >
						<select size=1 name="dynamic_exclude_static">
							<option value="no" <?php echo $dynamic_exclude_static['no'];?> > <?php echo language('_No');?> </option>
							<option value="yes" <?php echo $dynamic_exclude_static['yes'];?> > <?php echo language('_Yes');?> </option>
						</select>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Externally Mapped TCP Port');?>:
							<span class="showhelp">
							<?php echo language('Externally Mapped TCP Port help','The externally mapped TCP port, when the gateway is behind a static NAT or PAT.');?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" name="externally_mapped_tcp_port" value="<?php echo $externally_mapped_tcp_port;?>" />
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('External Address');?>:
							<span class="showhelp">
							<?php echo language('External Address help','The external address (and optional TCP port) of the NAT.<br>
								External Address = hostname[:port] specifies a static address[:port] to be used in SIP and SDP messages.Examples: <br>
								External Address = 12.34.56.78 <br>
								External Address = 12.34.56.78:9900 <br>
							');?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" name="external_address" id="external_address" value="<?php echo $external_address;?>" />:
						<input type="text" name="external_address_port" id="external_address_port" size="1" value="<?php echo $external_address_port;?>" />
						<input type="checkbox" name="auto_update_externaddr" id="auto_update_externaddr" 
						<?php 
							if(strcmp($auto_update_externaddr, "yes") == 0){
								echo "checked";
							}
						?>
						onchange="click_externaddr_readonly();"/>
						<?php echo language('Auto Update');?>
						<span id="cexternal_address" style="margin-left:10px;"></span>
						<input type="button" id="get_public_ip" value="<?php echo language('Get IP');?>" style="float:right;margin-right:20px;"/>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('External Hostname');?>:
							<span class="showhelp">
							<?php echo language('External Hostname help','The external hostname (and optional TCP port) of the NAT.<br>
								External Hostname = hostname[:port] is similar to "External Address". Examples: <br>
								External Hostname = foo.dyndns.net <br>
							');?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" name="external_hostname" value="<?php echo $external_hostname;?>" />
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Hostname Refresh Interval');?>:
							<span class="showhelp">
							<?php echo language('Hostname Refresh Interval help','
								How often to perform a hostname lookup. This can be useful when your NAT device<br/>
								lets you choose the port mapping, but the IP address is dynamic. Beware, you might<br/>
								suffer from service disruption when the name server resolution fails.');
							?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" name="hostname_refresh_interval" value="<?php echo $hostname_refresh_interval?>" />
					</td>
				</tr>
			</table>
		<!--</div>-->
		<div id="newline"></div>
		<table width="98%" class="tedit" align="right">
			<tr class="ttitle">
				<td colspan = 2>
				<?php echo language('RTP Settings');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Start of RTP Port Range');?>:
						<span class="showhelp">
						<?php echo language('Start of RTP Port Range help','Start of range of port numbers to be used for RTP.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="start_of_rtp_port_range" value="<?php echo $start_of_rtp_port_range?>" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('End of RTP port Range');?>:
						<span class="showhelp">
						<?php echo language('End of RTP port Range help','End of range of port numbers to be used for RTP');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" name="end_of_rtp_port_range" value="<?php echo $end_of_rtp_port_range?>" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('RTP Timeout');?>:
						<span class="showhelp">
						<?php echo language('RTP Timeout help','RTP Timeout');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" name="rtptimeout" value="<?php echo $rtptimeout?>" />
				</td>
			</tr>
		</table>
	</div>
	<div id="newline"></div>
	<div id="tab">
		<li class="tb_fold" onclick="lud(this,'parsing_and_compatibility')" id="parsing_and_compatibility_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'parsing_and_compatibility')"><?php echo language('Parsing and Compatibility');?></li>
		<li class="tb2_fold" onclick="lud(this,'parsing_and_compatibility')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>
	<div id="parsing_and_compatibility" style="display:none">
		<table width="98%" class="tedit" align="right">
			<tr class="ttitle">
				<td colspan = 2>
				<?php echo language('General');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Strict RFC Interpretation');?>:
						<span class="showhelp">
						<?php echo language('Strict RFC Interpretation help','
							Check header tags, character conversion in URIs, and multiline headers<br/>
							for strict SIP compatibility(default is yes)');
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="strict_rfc_interpretation">
						<option value="no" <?php echo $strict_rfc_interpretation['no'];?> > <?php echo language('_No');?> </option>
						<option value="yes" <?php echo $strict_rfc_interpretation['yes'];?> > <?php echo language('_Yes');?> </option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Send Compact Headers');?>:
						<span class="showhelp">
						<?php echo language('Send Compact Headers help','Send compact SIP headers.');?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="send_compact_headers">
						<option value="no" <?php echo $send_compact_headers['no'];?> > <?php echo language('_No');?> </option>
						<option value="yes" <?php echo $send_compact_headers['yes'];?> > <?php echo language('_Yes');?> </option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('SDP Owner');?>:
						<span class="showhelp">
						<?php echo language('SDP Owner help','
							Allows you to change the username filed in the SDP owner string,(o=).<br/>
							This filed MUST NOT contain spaces.');
						?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" name="sdp_owner" value="<?php echo $sdp_owner;?>" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Ring 183 Mode');?>:
						<span class="showhelp">
						<?php echo language('Ring 183 Mode help'); ?>
						</span>
					</div>
				</th>
				<td>
					<select id="ring_mode" name="ring_mode">
						<option value="0" <?php if($ring_mode == 0) echo 'selected';?>><?php echo language('Immediately@sip_adv','Immediately');?></option>
						<option value="1" <?php if($ring_mode == 1) echo 'selected';?>><?php echo language('AfterRing');?></option>
					</select>
				</td>
			</tr>
		</table>
		<div id="newline"></div>
		<table width="98%" class="tedit" align="right">
			<tr class="ttitle">
				<td colspan = 2>
				<?php echo language('SIP Methods');?>
				</td>
			</tr>
			<tr>
				<th rowspan=14 >
					<div class="helptooltips">
						<?php echo language('Disallowed SIP Methods');?>
						<span class="showhelp">
						<?php echo language('Disallowed SIP Methods help','
							When a dialog is started with another SIP endpoint, the other endpoint<br/> 
							should include an Allow header telling us what SIP methods the endpoint<br/> 
							implements. However, some endpoints either do not include an Allow header<br/>
							or lie about what methods they implement.In the former case, the gateway<br/>
							makes the assumption that the endpoint supports all known SIP methods. If<br/>
							you know that your SIP endpoint does not provide support for a specific method,<br/> 
							then you may provide a list of methods that your endpoint does not implement in the<br/>
							disallowed_methods option. Note that if your endpoint is truthful with its Allow header,<br/> 
							then there is no need to set this option.');
						?>
						</span>
					</div>
				</th>
				<td >
					<?php echo language('ACK');?><input type="checkbox" name="disallowed_sip_methods[]" value="ACK" <?php echo $disallowed_sip_methods['ACK']; ?> />
				</td>
			</tr>
			<tr>
				<td >
					<?php echo language('BYE');?><input type="checkbox" name="disallowed_sip_methods[]" value="BYE" <?php echo $disallowed_sip_methods['BYE']; ?> />
				</td>
			</tr>
			<tr>
				<td >
					<?php echo language('CANCEL');?><input type="checkbox" name="disallowed_sip_methods[]" value="CANCEL" <?php echo $disallowed_sip_methods['CANCEL']; ?> />
				</td>
			</tr>
			<tr>
				<td >
					<?php echo language('INFO');?><input type="checkbox" name="disallowed_sip_methods[]" value="INFO" <?php echo $disallowed_sip_methods['INFO']; ?> />
				</td>
			</tr>
			<tr>
				<td >
					<?php echo language('INVITE');?><input type="checkbox" name="disallowed_sip_methods[]" value="INVITE" <?php echo $disallowed_sip_methods['INVITE']; ?> />
				</td>
			</tr>
			<tr>
				<td >
					<?php echo language('MESSAGE');?><input type="checkbox" name="disallowed_sip_methods[]" value="MESSAGE" <?php echo $disallowed_sip_methods['MESSAGE']; ?> />
				</td>
			</tr>
			<tr>
				<td >
					<?php echo language('NOTIFY');?><input type="checkbox" name="disallowed_sip_methods[]" value="NOTIFY" <?php echo $disallowed_sip_methods['NOTIFY']; ?> />
				</td>
			</tr>
			<tr>
				<td >
					<?php echo language('OPTIONS');?><input type="checkbox" name="disallowed_sip_methods[]" value="OPTIONS" <?php echo $disallowed_sip_methods['OPTIONS']; ?> />
				</td>
			</tr>
			<tr>
				<td >
					<?php echo language('PRACK');?><input type="checkbox" name="disallowed_sip_methods[]" value="PRACK" <?php echo $disallowed_sip_methods['PRACK']; ?> />
				</td>
			</tr>
			<tr>
				<td >
					<?php echo language('PUBLISH');?><input type="checkbox" name="disallowed_sip_methods[]" value="PUBLISH" <?php echo $disallowed_sip_methods['PUBLISH']; ?> />
				</td>
			</tr>
			<tr>
				<td >
					<?php echo language('REFER');?><input type="checkbox" name="disallowed_sip_methods[]" value="REFER" <?php echo $disallowed_sip_methods['REFER']; ?> />
				</td>
			</tr>
			<tr>
				<td >
					<?php echo language('REGISTER');?><input type="checkbox" name="disallowed_sip_methods[]" value="REGISTER" <?php echo $disallowed_sip_methods['REGISTER']; ?> />
				</td>
			</tr>
			<tr>
				<td >
					<?php echo language('SUBSCRIBE');?><input type="checkbox" name="disallowed_sip_methods[]" value="SUBSCRIBE" <?php echo $disallowed_sip_methods['SUBSCRIBE']; ?> />
				</td>
			</tr>
			<tr>
				<td >
					<?php echo language('UPDATE');?><input type="checkbox" name="disallowed_sip_methods[]" value="UPDATE" <?php echo $disallowed_sip_methods['UPDATE']; ?> />
				</td>
			</tr>

			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Hangup Cause Code');?>:
						<span class="showhelp">
						<?php echo language('Hangup Cause Code help');
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="hangupcausecode">
						<option value="" <?php echo $hangupcausecode['default'];?> > <?php echo language('default');?> </option>
						<option value="1" <?php echo $hangupcausecode['1'];?> > <?php echo language('404 Not Found');?> </option>
						<option value="3" <?php echo $hangupcausecode['3'];?> > <?php echo language('420 No Route Destination');?> </option>
						<option value="17" <?php echo $hangupcausecode['17'];?> > <?php echo language('486 Busy Here');?> </option>
						<option value="18" <?php echo $hangupcausecode['18'];?> > <?php echo language('408 Request Timeout');?> </option>
						<option value="19" <?php echo $hangupcausecode['19'];?> > <?php echo language('480 Temporarily Unavailable');?> </option>
						<option value="21" <?php echo $hangupcausecode['21'];?> > <?php echo language('403 Forbidden');?> </option>
						<option value="22" <?php echo $hangupcausecode['22'];?> > <?php echo language('410 Gone');?> </option>
						<option value="27" <?php echo $hangupcausecode['27'];?> > <?php echo language('502 Bad Gateway');?> </option>
						<option value="28" <?php echo $hangupcausecode['28'];?> > <?php echo language('484 Address Incomplete');?> </option>
						<option value="29" <?php echo $hangupcausecode['29'];?> > <?php echo language('501 Not Implemented');?> </option>
						<option value="38" <?php echo $hangupcausecode['38'];?> > <?php echo language('500 Server Internal Failure');?> </option>
						<option value="42" <?php echo $hangupcausecode['42'];?> > <?php echo language('503 Service Unavailable');?> </option>
						<option value="58" <?php echo $hangupcausecode['58'];?> > <?php echo language('488 Not Acceptable Here');?> </option>
						<option value="127" <?php echo $hangupcausecode['127'];?> > <?php echo language('603 Declined');?> </option>
					</select>
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Line Unavailable Code');?>:
						<span class="showhelp">
						<?php echo language('Line Unavailable Code help', 'Line Unavailable Code');?>
						</span>
					</div>
				</th>
				<td>
					<select size=1 name="hangupcausecodeunavaline">
						<option value="" <?php echo $hangupcausecodeunavaline['default'];?> > <?php echo language('default');?> </option>
						<option value="1" <?php echo $hangupcausecodeunavaline['1'];?> > <?php echo language('404 Not Found');?> </option>
						<option value="3" <?php echo $hangupcausecodeunavaline['3'];?> > <?php echo language('420 No Route Destination');?> </option>
						<option value="17" <?php echo $hangupcausecodeunavaline['17'];?> > <?php echo language('486 Busy Here');?> </option>
						<option value="18" <?php echo $hangupcausecodeunavaline['18'];?> > <?php echo language('408 Request Timeout');?> </option>
						<option value="19" <?php echo $hangupcausecodeunavaline['19'];?> > <?php echo language('480 Temporarily Unavailable');?> </option>
						<option value="21" <?php echo $hangupcausecodeunavaline['21'];?> > <?php echo language('403 Forbidden');?> </option>
						<option value="22" <?php echo $hangupcausecodeunavaline['22'];?> > <?php echo language('410 Gone');?> </option>
						<option value="27" <?php echo $hangupcausecodeunavaline['27'];?> > <?php echo language('502 Bad Gateway');?> </option>
						<option value="28" <?php echo $hangupcausecodeunavaline['28'];?> > <?php echo language('484 Address Incomplete');?> </option>
						<option value="29" <?php echo $hangupcausecodeunavaline['29'];?> > <?php echo language('501 Not Implemented');?> </option>
						<option value="38" <?php echo $hangupcausecodeunavaline['38'];?> > <?php echo language('500 Server Internal Failure');?> </option>
						<option value="42" <?php echo $hangupcausecodeunavaline['42'];?> > <?php echo language('503 Service Unavailable');?> </option>
						<option value="58" <?php echo $hangupcausecodeunavaline['58'];?> > <?php echo language('488 Not Acceptable Here');?> </option>
						<option value="127" <?php echo $hangupcausecodeunavaline['127'];?> > <?php echo language('603 Declined');?> </option>
					</select>
				</td>
			</tr>

			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Notify Unlimited');?>:
						<span class="showhelp">
						<?php echo language('Notify Unlimited help','on:Received notify, do not restrict, reply directly to 200ok.<br/>off:According to normal processing.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="notify_unlimited" id="notify_unlimited" <?php echo $notify_unlimited_checked;?> />
				</td>
			</tr>
		</table>

		<div id="newline"></div>

		<table width="98%" class="tedit" align="right">
			<tr class="ttitle">
				<td colspan = 2>
				<?php echo language('Caller ID');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Shrink Caller ID');?>:
						<span class="showhelp">
						<?php echo language('Shrink Caller ID help',"
							The shrinkcallerid function removes '(', ' ', ')', non-trailing '.',<br/>
							and '-' not in square brackets. For example,the caller id value 555.5555<br/>
							becomes 5555555 when this option is enabled. Disabling this option results<br/>
							in no modification of the caller id value,which is necessary when the caller<br/> 
							id represents something that must be preserved. By default this option is on.");
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="shrink_caller_id">
						<option value="no" <?php echo $shrink_caller_id['no'];?> > <?php echo language('_No');?> </option>
						<option value="yes" <?php echo $shrink_caller_id['yes'];?> > <?php echo language('_Yes');?> </option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Caller ID');?>:
						<span class="showhelp">
						<?php echo language('Caller ID help',"defalut:Sip From and Number<br/>eg: When selecting SIP From, Name is Peter and Number is 402.<br/>The From mode is: \"Peter\"&lt;sip:402@172.16.6.239;transport=UDP&gt;;tag=bd481672");
						?>
						</span>
					</div>
				</th>
				<td>
					<select size=1 name="caller_id_1" id="caller_id_1" style="margin-right:10px;">
						<option value="EXTEN" <?php echo $caller_id_1['EXTEN'];?> >EXTEN</option>
						<option value="To" <?php echo $caller_id_1['To'];?> >SIP To</option>
						<option value="From" <?php echo $caller_id_1['From'];?> >SIP From</option>
					</select>
					<select size=1 name="caller_id_2" id="caller_id_2">
						<option value="Number" <?php echo $caller_id_2['Number'];?>>Number</option>
						<option value="Name" <?php echo $caller_id_2['Name'];?>>Name</option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('SIP From');?>:
						<span class="showhelp">
						<?php echo language('SIP From help',"Caller ID transfer");
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="fromtype_callid" id="fromtype_callid">
						<option value="0" <?php echo $sipfrom[0];?> >Tel/Tel</option>
						<option value="1" <?php echo $sipfrom[1];?> >Tel/User</option>
						<option value="2" <?php echo $sipfrom[2];?> >User/Tel</option>
						<option value="3" <?php echo $sipfrom[3];?> >User/User</option>
					</select>
				</td>
			</tr>
		</table>

		<div id="newline"></div>
		<table width="98%" class="tedit" align="right">
			<tr class="ttitle">
				<td colspan = 2>
				<?php echo language('Callee ID');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('SIP To');?>:
						<span class="showhelp">
						<?php echo language('SIP To help',"Callee ID transfer.");
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="sipto" id="sipto">
						<option value="0" <?php echo $sipto[0];?> >Tel/Tel</option>
						<option value="1" <?php echo $sipto[1];?> >Tel/User</option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Callee ID');?>:
						<span class="showhelp">
						<?php echo language('Callee ID help',"defalut: EXTEN<br/>eg: When selecting SIP To, Name is Jason and Number is 401.<br/>To mode is: \"Jason\"&lt;sip:401@172.16.6.239;transport=UDP&gt;");
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="callee_id_1" id="callee_id_1" style="margin-right:10px;">
						<option value="EXTEN" <?php echo $callee_id_1['EXTEN'];?>>EXTEN</option>
						<option value="To" <?php echo $callee_id_1['To'];?>>SIP To</option>
						<option value="From" <?php echo $callee_id_1['From'];?>>SIP From</option>
					</select>
					<select size=1 name="callee_id_2" id="callee_id_2">
						<option value="Number" <?php echo $callee_id_2['Number'];?>>Number</option>
						<option value="Name" <?php echo $callee_id_2['Name'];?>>Name</option>
					</select>
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Permit Dialing Letters');?>:
						<span class="showhelp">
						<?php echo language('Permit Dialing Letters help','When the Permit Dialing Letters switch is turned off, the called number is only allowed to use numbers.<br/> When the Permit Dialing Letters switch is turned on, the called number can be numbers and character.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="sipinbound" id="sipinbound" <?php echo $sipinbound_checked;?>/>
				</td>
			</tr>
		</table>
		
		<div id="newline"></div>
		<table width="98%" class="tedit" align="right">
			<tr class="ttitle">
				<td colspan = 2>
				<?php echo language('Timer Configuration');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Maximum Registration Expiry');?>:
						<span class="showhelp">
						<?php echo language('Maximum Registration Expiry help','Maximum allowed time of incoming registrations and subscriptions (seconds)');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" name="maximum_registration_expiry" value="<?php echo $maximum_registration_expiry;?>" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Minimum Registration Expiry');?>:
						<span class="showhelp">
						<?php echo language('Minimum Registration Expiry help','Minimum length of registrations/subscriptions (default 60)');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" name="minimum_registration_expiry" value="<?php echo $minimum_registration_expiry;?>" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Default Registration Expiry');?>:
						<span class="showhelp">
						<?php echo language('Default Registration Expiry help','Default length of incoming/outgoing registration');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" name="default_registration_expiry" value="<?php echo $default_registration_expiry;?>" />
				</td>
			</tr>
		</table>

		<div id="newline"></div>

		<table width="98%" class="tedit" align="right">
			<tr class="ttitle">
				<td colspan = 2>
				<?php echo language('Outbound Registrations');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Registration Timeout');?>:
						<span class="showhelp">
						<?php echo language('Registration Timeout help','How often, in seconds, to retry registration calls. Default 20 seconds.');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" name="registration_timeout" value="<?php echo $registration_timeout;?>" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Number of Registration Attempts');?>:
						<span class="showhelp">
						<?php echo language('Number of Registration Attempts help','
							Number of registration attempts before we give up. 0 = continue forever,<br/> 
							hammering the other server until it accepts the registration. Default is 0 tries,<br/> 
							continue forever');
						?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" name="number_of_registration_attemptsy" value="<?php echo $number_of_registration_attemptsy;?>" />
				</td>
			</tr>
		</table>
		
		<div id="newline"></div>
		
		<table width="98%" class="tedit" align="right">
			<tr class="ttitle">
				<td colspan = 2>
				<?php echo language('Transform Local Port');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Client Auto Flag');?>:
						<span class="showhelp">
						<?php echo language('Client Auto Flag help','Client Auto Flag Switch');?>
						</span>
					</div>
				</th>
				<td >
					<input type="checkbox" name="client_auto_flag" id="client_auto_flag" <?php echo $client_auto_flag_checked;?> />
				</td>
			</tr>
			<tr class="client_flag_show">
				<th>
					<div class="helptooltips">
						<?php echo language('Client Basic Port');?>:
						<span class="showhelp">
						<?php echo language('Client Basic Port help','Configuring the SIP local port can not conflict with other port numbers used by the device.(range 1-9999, default value: 5060)');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" name="client_basic_port" id="client_basic_port" value="<?php echo $client_basic_port;?>" />
					<span id="cclient_basic_port"></span>
				</td>
			</tr>
			<tr class="client_flag_show">
				<th>
					<div class="helptooltips">
						<?php echo language('Current Port');?>:
						<span class="showhelp">
						<?php echo language('Current Port help');?>
						</span>
					</div>
				</th>
				<td>
				<?php 
				$redis_client = new Predis\Client($single_server);
				echo $redis_client->get('app.asterisk.udp.curr_port');
				?>
				</td>
			</tr>
			<tr class="client_flag_show">
				<th>
					<div class="helptooltips">
						<?php echo language('Client Port Step');?>:
						<span class="showhelp">
						<?php echo language('Client Port Step help','1-10: Automatically select different local SIP ports based on this value.');?>
						</span>
					</div>
				</th>
				<td >
					<select name="client_port_step" id="client_port_step">
					<?php 
						for($i=1;$i<=10;$i++){
							if($i == $client_port_step){
								echo "<option value='$i' selected>$i</option>";
							}else{
								echo "<option value='$i'>$i</option>";
							}
						}
					?>
					</select>
				</td>
			</tr>
			<tr class="client_flag_show">
				<th>
					<div class="helptooltips">
						<?php echo language('Client of Registration Attempts');?>:
						<span class="showhelp">
						<?php echo language('Client of Registration Attempts help','The default value is 3. The number of attempts to register, such as setting 3, indicates that the local port will change after 3 failures of registration.');?>
						</span>
					</div>
				</th>
				<td >
					<select name="client_reg_attempts" id="client_reg_attempts">
					<?php
						for($i=1;$i<=10;$i++){
							if($i == $client_reg_attempts){
								echo "<option value='$i' selected>$i</option>";
							}else{
								echo "<option value='$i'>$i</option>";
							}
						}
					?>
					</select>
				</td>
			</tr>
		</table>	
		
	</div>

	<div id="newline"></div>

	<div id="tab">
		<li class="tb_fold" onclick="lud(this,'tab_security')" id="tab_security_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_security')"><?php echo language('Security');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_security')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>
	<div id="tab_security" style="display:none">
		<table width="98%" class="tedit" align="right">
			<tr class="ttitle">
				<td colspan = 2>
				<?php echo language('Authentication Settings');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Match Auth Username');?>:
						<span class="showhelp">
						<?php echo language('Match Auth Username help',"
							If available, match user entry using the 'username' field from the<br/> 
							authentication line instead of the 'from' field.");
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="match_auth_username">
						<option value="no"  <?php echo $match_auth_username['no'];?> > <?php echo language('_No');?> </option>
						<option value="yes" <?php echo $match_auth_username['yes'];?> > <?php echo language('_Yes');?> </option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Realm');?>:
						<span class="showhelp">
						<?php echo language('Realm help','
							Realm for digest authentication. Realms MUST be globally unique according<br/>
							to RFC 3261. Set this to your host name or domain name.');
						?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" name="realm" value="<?php echo $realm;?>" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Use Domain as Realm');?>:
						<span class="showhelp">
						<?php echo language('Use Domain as Realm help',"
							Use the domain from the SIP Domains setting as the realm.<br/>
							In this case, the realm will be based on the request 'to' or 'from'<br/> 
							header and should match one of the domain. Otherwise, the configured 'realm'<br/> 
							value will be used.");
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="use_domain_as_realm">
						<option value="no"  <?php echo $use_domain_as_realm['no'];?> > <?php echo language('_No');?> </option>
						<option value="yes" <?php echo $use_domain_as_realm['yes'];?> > <?php echo language('_Yes');?> </option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Always Auth Reject');?>:
						<span class="showhelp">
						<?php echo language('Always Auth Reject help',"
							When an incoming INVITE or REGISTER is to be rejected, <br/>
							for any reason, always reject with an identical response equivalent<br/> 
							to valid username and invalid password/hash instead of letting the requester<br/> 
							know whether there was a matching user or peer for their request. This reduces<br/>
							the ability of an attacker to scan for valid SIP usernames. This option is set<br/>
							to 'yes' by default.");
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="always_auth_reject">
						<option value="no"  <?php echo $always_auth_reject['no'];?> > <?php echo language('_No');?> </option>
						<option value="yes" <?php echo $always_auth_reject['yes'];?> > <?php echo language('_Yes');?> </option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Authenticate Options Requests');?>:
						<span class="showhelp">
						<?php echo language('Authenticate Options Requests help','
							Enabling this option will authenticate OPTIONS requests just<br/>
							like INVITE requests are. By default this option is disabled.');
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="authenticate_options_requests">
						<option value="no"  <?php echo $authenticate_options_requests['no'];?> > <?php echo language('_No');?> </option>
						<option value="yes" <?php echo $authenticate_options_requests['yes'];?> > <?php echo language('_Yes');?> </option>
					</select>
				</td>
			</tr>
		</table>

		<div id="newline"></div>

		<table width="98%" class="tedit" align="right">
			<tr class="ttitle">
				<td colspan = 2>
				<?php echo language('Guest Calling');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Allow Guest Calling');?>:
						<span class="showhelp">
						<?php echo language('Allow Guest Calling help','
							Allow or reject guest calls (default is yes, to allow). If your gateway<br/> 
							is connected to the Internet and you allow guest calls, you want to check<br/>
							which services you offer everyone out there, by enabling them in the default context');
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="allow_guest_calling">
						<option value="no"  <?php echo $allow_guest_calling['no'];?> > <?php echo language('_No');?> </option>
						<option value="yes" <?php echo $allow_guest_calling['yes'];?> > <?php echo language('_Yes');?> </option>
					</select>
				</td>
			</tr>
		</table>
	</div>

	<div id="newline"></div>

	<div id="tab">
		<li class="tb_fold" onclick="lud(this,'media')" id="media_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'media')"><?php echo language('Media');?></li>
		<li class="tb2_fold" onclick="lud(this,'media')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>
	<div id="media" style="display:none">
		<table width="98%" class="tedit" align="right">
			<tr class="ttitle">
				<td colspan = 2>
				<?php echo language('QoS/ToS');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('TOS for SIP Packets');?>:
						<span class="showhelp">
						<?php echo language('TOS for SIP Packets help','Sets type of service for SIP packets.');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" name="tos_for_sip_packets" value="<?php echo $tos_for_sip_packets;?>" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('TOS for RTP Packets');?>:
						<span class="showhelp">
						<?php echo language('TOS for RTP Packets help','Sets type of service for RTP packets');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" name="tos_for_rtp_packets" value="<?php echo $tos_for_rtp_packets;?>" />
				</td>
			</tr>
		</table>
	</div>

	<div id="newline"></div>

	<div id="tab">
		<li class="tb_fold" onclick="lud(this,'tab_media_set')" id="tab_media_set_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_media_set')">
			<div class="helptooltips">
				<?php echo language('Codec Settings');?>
				<span class="showhelp">
				<?php echo language('Codec Settings help','Select codec from the drop down list. Codecs should be different for each Codec Priority.');?>
				</span>
			</div>
		</li>
		<li class="tb2_fold" onclick="lud(this,'tab_media_set')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>

	<div id="tab_media_set" style="display:none">
		<table width="98%" class="tedit" align="right">
<?php
	for($i=1;$i<=7;$i++) {
?>
			<tr>
				<th><?php echo language('Codec Priority');echo " $i";?>:</th>
				<td >
				<select size=1 name="sip_codec_priority<?php echo $i?>" >
					<option value=''    <?php echo $sip_codec_priority[$i]['notuse']?> >Not Used</option>
					<option value=ulaw  <?php echo $sip_codec_priority[$i]['ulaw']?> >G.711 u-law</option>
					<option value=alaw  <?php echo $sip_codec_priority[$i]['alaw']?> >G.711 a-law</option>
					<option value=gsm   <?php echo $sip_codec_priority[$i]['gsm']?> >GSM</option>
					<option value=g722  <?php echo $sip_codec_priority[$i]['g722']?> >G.722</option>
					<?php 
					$product_type = get_product_type();
					if($product_type >= 4){
					?>
					<option value=g723  <?php echo $sip_codec_priority[$i]['g723']?> >G.723</option>
					<?php } ?>
					<option value=g726  <?php echo $sip_codec_priority[$i]['g726']?> >G.726</option>
					<option value=g729  <?php echo $sip_codec_priority[$i]['g729']?> >G.729</option>
				</select>
			</td>
		</tr>
<?php
	}
?>
		</table>
	</div>

	<div id="newline"></div>

	<br>

	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" class="gen_short_btn float_btn"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td>
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" />
			</td>
		</tr>
	</table>
<script type="text/javascript">
function check(){
	var client_basic_port = document.getElementById('client_basic_port').value;
	if(!isNaN(client_basic_port) && parseInt(client_basic_port)>0 && parseInt(client_basic_port)<9999){
		return true;
	}else{
		$("#cclient_basic_port").html(con_str('*<?php echo language('Client Basic Port Tip', 'Range: 1-9999');?>'));
		$("#client_basic_port").focus();
		return false;
	}
}

var get_ip_time = 0;
function get_public_ip(){
	document.getElementById('cexternal_address').innerHTML = "<img src='/images/mini_loading.gif' />"+con_str("<?php echo language('Getting Public IP');?>");
	
	$.ajax({
		url: "ajax_server.php?type=get_public_ip",
		type: "POST",
		success: function(data){
			var res = data.split("-");
			var externaddr = res[1];
			
			if(res[0] == 1){
				$("#cexternal_address").css("color", "green");
				document.getElementById('cexternal_address').innerHTML = "<?php echo language('Get Public IP Success');?>";
				document.getElementById('external_address').value = externaddr;
			}else if(res[0] == '' && get_ip_time > 60){
				document.getElementById('cexternal_address').innerHTML = con_str("<?php echo language('Timeout@get_ip', 'Timeout');?>");
			}else if(res[0] == ''){
				get_ip_time++;
				setTimeout("get_public_ip()", 1000);
			}else{
				document.getElementById('cexternal_address').innerHTML = con_str("<?php echo language('Get Public IP Error', 'Failed to get external address. Please check network connectivity.');?>");
			}
		}
	});
}

$("#get_public_ip").click(function(){
	get_ip_time = 0;
	get_public_ip();
});

$(document).ready(function(){
	$("#notify_unlimited").iButton();
	$("#client_auto_flag").iButton();
	$("#sipinbound").iButton();
	onload_func();
	
	if($("#caller_id_1").val()=='EXTEN'){
		$("#caller_id_2").hide();
	}else{
		$("#caller_id_2").show();
	}
	
	if($("#callee_id_1").val()=='EXTEN'){
		$("#callee_id_2").hide();
	}else{
		$("#callee_id_2").show();
	}
	
	if(document.getElementById('client_auto_flag').checked){
		$(".client_flag_show").show();
	}else{
		$(".client_flag_show").hide();
	}
	
	if(!document.getElementById('auto_update_externaddr').checked){
		document.getElementById('get_public_ip').disabled = true;
	}
});

$("#caller_id_1").change(function(){
	if($(this).val()=='EXTEN'){
		$("#caller_id_2").hide();
	}else{
		$("#caller_id_2").show();
	}
});

$("#callee_id_1").change(function(){
	if($(this).val()=='EXTEN'){
		$("#callee_id_2").hide();
	}else{
		$("#callee_id_2").show();
	}
});

//client_auto_flag_show
$("#client_auto_flag").change(function(){
	if($(this).attr('checked') == 'checked'){
		$(".client_flag_show").show();
	}else{
		$(".client_flag_show").hide();
	}
});

$(document).on('click','#button_cancel',function(){
	$("#modal-51").hide();
	$(".fade_in").hide();
});

</script>
</form>
<?php require("/www/cgi-bin/inc/boot.inc");?>
<div id="float_btn1" class="float_btn1 sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()">
</div>

