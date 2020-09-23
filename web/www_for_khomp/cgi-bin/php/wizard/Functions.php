<?php
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");

function saveChangePassword()
{
	$db = new Users();
	session_start();
	
	$old_username = $_SESSION['username'];
	$id = $_SESSION['id'];
	
	$username = trim($_POST['new_username']);
	$password = trim($_POST['new_password']);
	
	$real_password = md5($password.'-'.$username);
	
	if(!check_username_repeat($db,$old_username,$username)){
		echo "User name has been used";
		return false;
	}
	
	$db->update_username_and_password($id,$username,$real_password);
	
	save_user_record($db,"SYSTEM->Setting Wizard:Update User ".$old_username.' to '.$username);
	
	return true;
}

function saveSelectTimezone()
{
	$aql = new aql();
	$setok = $aql->set('basedir', "/etc/asterisk/gw");
	$conf_path = "/etc/asterisk/gw/time.conf";
	$hlock = lock_file($conf_path);

	if(!$aql->open_config_file($conf_path))
	{
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	$time_conf_array = $aql->query("select * from time.conf");

	if(!isset($time_conf_array)){
		$aql->assign_addsection('general', '');
	}
	if(isset($_POST['system_timezone'])){
		$timezone = trim($_POST['system_timezone']);
		if(isset($time_conf_array['general']['timezone'])) {
			$aql->assign_editkey('general', 'timezone', $timezone);
		} else {
			$aql->assign_append('general', 'timezone', $timezone);
		}
	}
	
	if(!$aql->save_config_file('time.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}

	//timezone=Asia/Chongqing@CST-8
	$timezone_split = explode("@", $timezone);
	if ($timezone_split[0]) {
		if ($timezone_split[0] == "-") {
			$zonefile = "UTC";
		} else {
			$zoneinfo = explode("/", $timezone_split[0]);
			$zoneinfo = $zoneinfo[1];
		}
	} else {
		$zoneinfo = "UTC";
	}

	$zoneinfo_file = "/usr/share/zoneinfo/".$zoneinfo;

	if(file_exists($zoneinfo_file)) {
		@file_put_contents("/etc/localtime", @file_get_contents($zoneinfo_file, LOCK_EX), LOCK_EX);
	}

	$web_language_path = "/etc/asterisk/gw/web_language.conf";
	if(isset($_POST['language_type'])){
		$conf_array['general']['language'] = $_POST['language_type'];
		if(modify_conf($web_language_path, $conf_array)){
			exec("/my_tools/web_language_init >/dev/null 2>&1 &");
			
			save_user_record($db,"SYSTEM->Setting Wizard:Timezone Save");
			return true;
		}
	}else{
		return false;
	}
}
function saveLanSettings()
{
	$aql = new aql();
	$setok = $aql->set('basedir', "/etc/asterisk/gw/network");
	$conf_path = "/etc/asterisk/gw/network/lan.conf";
	$hlock = lock_file($conf_path);

	if(!$aql->open_config_file($conf_path))
	{
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	$lan_conf_array = $aql->query("select * from lan.conf");
	if(!is_array($lan_conf_array)){
		$aql->assign_append('general', '');
		$aql->assign_append('ipv4', '');
	}
	$type = isset($_POST['lan_type']) ? trim($_POST['lan_type']) : "static";
	if(isset($lan_conf_array['general']['type'])){
		$aql->assign_editkey('general', 'type', $type);
	} else {
		$aql->assign_append('general', 'type', $type);
	}

	$post_wan_data = array();
	
	$post_wan_data['ipaddr'] = isset($_POST['lan_ip_address']) ? trim($_POST['lan_ip_address']) : "";
	$post_wan_data['netmask'] = isset($_POST['lan_netmask']) ? trim($_POST['lan_netmask']) : "";
	$post_wan_data['gateway'] = isset($_POST['lan_gateway']) ? trim($_POST['lan_gateway']) : "";
	
	foreach ($post_wan_data as $key => $value) {	
		if(isset($lan_conf_array['ipv4'][$key])) {
			$aql->assign_editkey('ipv4', $key, $value);
		} else {
			$aql->assign_append('ipv4', $key, $value);
		}
		
		if($key == 'ipaddr'){
			save_user_record("","SYSTEM->Setting Wizard:Save Lan IP=".$value);
		}
	}
	if(!$aql->save_config_file('lan.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	
	save_to_flash('/etc/asterisk','/etc/cfg');
	exec("/etc/init.d/lighttpd restart > /dev/null 2>&1 &");
	exec("/etc/init.d/lan restart > /dev/null 2>&1 &");
}
function saveDNSSettings()
{
	$dns1 = isset($_POST['lan_dns1']) ? trim($_POST['lan_dns1']) : "";
	$dns2 = isset($_POST['lan_dns2']) ? trim($_POST['lan_dns2']) : "";
	$dns3 = isset($_POST['lan_dns3']) ? trim($_POST['lan_dns3']) : "";

	$dns_conf_path = "/etc/asterisk/gw/network/dns.conf";
	$hlock = fopen($dns_conf_path);
	$fd = fopen($dns_conf_path, "w");
	$dns_contents = "[general]\n";
	$dns_contents .= "dns1=$dns1\n";
	$dns_contents .= "dns2=$dns2\n";
	$dns_contents .= "dns3=$dns3";
	fwrite($fd, $dns_contents);
	fclose($fd);
	unlock_file($hlock);

	$resolv_contents = $dns1 != "" ? "nameserver $dns1\n" : '';
	$resolv_contents .= $dns2 != "" ? "nameserver $dns2\n" : '';
	$resolv_contents .= $dns3 != "" ? "nameserver $dns3\n" : '';

	$resolv_conf_path = "/etc/resolv.conf";
	$hlock = lock_file($resolv_conf_path);
	$fd = fopen($resolv_conf_path, "w");
	fwrite($fd, $resolv_contents);
	fclose($fd);
	unlock_file($hlock);

}
function saveWanSettings()
{
	$aql = new aql();
	$setok = $aql->set('basedir', "/etc/asterisk/gw/network");
	$conf_path = "/etc/asterisk/gw/network/wan.conf";
	$hlock = lock_file($conf_path);

	if(!$aql->open_config_file($conf_path))
	{
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	$wan_conf_array = $aql->query("select * from wan.conf");
	if(!is_array($wan_conf_array)){
		$aql->assign_append('general', '');
		$aql->assign_append('ipv4', '');
	}

	$type = isset($_POST['wan_type']) ? trim($_POST['wan_type']) : "static";
	if(isset($wan_conf_array['general']['type'])) {
		$aql->assign_editkey('general', 'type', $type);
	} else {
		$aql->assign_append('general', 'type', $type);
	}

	$post_wan_data = array();
	$post_wan_data['ipaddr'] = isset($_POST['wan_ip_address']) ? trim($_POST['wan_ip_address']) : "";
	$post_wan_data['netmask'] = isset($_POST['wan_netmask']) ? trim($_POST['wan_netmask']) : "";
	$post_wan_data['gateway'] = isset($_POST['wan_gateway']) ? trim($_POST['wan_gateway']) : "";
	
	foreach ($post_wan_data as $key => $value) {	
		if(isset($wan_conf_array['ipv4'][$key])) {
			$aql->assign_editkey('ipv4', $key, $value);
		} else {
			$aql->assign_append('ipv4', $key, $value);
		}
		
		if($key == 'ipaddr'){
			save_user_record("","SYSTEM->Setting Wizard:Save Wan IP=".$value);
		}
	}

	if(!$aql->save_config_file('wan.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	
	exec("/etc/init.d/wan restart > /dev/null 2>&1 &");
}
function saveNetworkSettings()
{
	saveDNSSettings();
	saveWanSettings();
	saveLanSettings();
	
	reset_user_login_info();
}

function saveSIPEndpoint()
{
	//Save to gw_endpoints.conf
	$aql = new aql();
	//Save to gw_endpoints.conf
	$gw_endpoints_conf_path = '/etc/asterisk/gw_endpoints.conf';
	$hlock = lock_file($gw_endpoints_conf_path);
	if (!file_exists($gw_endpoints_conf_path)) {
		fclose(fopen($gw_endpoints_conf_path,"w"));
	}
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($gw_endpoints_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	$datachunk = '';
	$username = '';
	$password = '';
	$host = '';
	if(!isset($_POST['sip_endpoint_username'])) return false;
	$username = trim($_POST['sip_endpoint_username']);
	if(!isset($_POST['sip_endpoint_password'])) return false; 
	$password = trim($_POST['sip_endpoint_password']); 
	if(!isset($_POST['host'])) return false;
	$host = trim($_POST['host']); 
	
	$registration =  isset($_POST['registration']) ? trim($_POST['registration']) : 'client';
	$transport = isset($_POST['transport']) ? trim($_POST['transport']) : 'udp';
	$nat = isset($_POST['nat']) ? trim($_POST['nat']) : 'yes';

	$datachunk .= 'username=' . $username . "\n";
	$datachunk .= 'registration=' . $registration . "\n";
	$datachunk .= 'allow_anonymous=no' . "\n";
	$datachunk .= 'order=1' . "\n";
	$datachunk .= 'secret=' . $password . "\n";
		
	$datachunk .= 'host=' . $host . "\n";
	$datachunk .= 'transport=' . $transport . "\n";
	$datachunk .= "port=$port\n";
	$datachunk .= "nat=yes\n";
	$datachunk .="fromdomain=\n";
	if($registration == 'client') {
		$datachunk .= 'register_extension=' . $username . "\n";
		$datachunk .= 'register_user=' . $username . "\n";
	}
	$datachunk .= 'qualify=yes' . "\n";
	$datachunk .= 'qualifyfreq=60' . "\n";
	
	$datachunk .= 'dtmfmode=rfc2833' . "\n";
	$datachunk .= 'trustrpid=no' . "\n";
	$datachunk .= 'sendrpid=no' . "\n";
	$datachunk .= 'callingpres=allowed_passed_screen' . "\n";
	$datachunk .= 'progressinband=never' . "\n";
	$datachunk .= 'usereqphone=no' . "\n";
	$datachunk .= 'use_q850_reason=no' . "\n";
	$datachunk .= 'ignoresdpversion=yes' . "\n";
	$datachunk .= 'directmedia=yes' . "\n";
	$datachunk .= 'allowtransfer=yes' . "\n";
	$datachunk .= 'promiscredir=no' . "\n";
	$datachunk .= 'max_forwards=70' . "\n";
	$datachunk .= 'registertrying=no' . "\n";
	$datachunk .= 'timert1=500' . "\n";
	$datachunk .= 'timerb=32000' . "\n";
	$datachunk .= 'session-timers=accept' . "\n";
	$datachunk .= 'session-minse=90' . "\n";
	$datachunk .= 'session-expires=1800' . "\n";
	$datachunk .= 'session-refresher=uas' . "\n";

	if($registration == 'client') {
		if (!empty($port)) {
			$register = $username .':'. $passwd. '@' .$ipaddr. ':' .$port. '/' .$username;
		} else {
			$register = $username. ':'.$passwd. '@' .$ipaddr.$port. '/' .$username;
		}
		$datachunk .= 'register=>'.$register."\n";
	}

	$datachunk .= 'insecure=port,invite' . "\n";
	$datachunk .= 'type=friend' . "\n";
	$datachunk .= 'context=' .$username . "\n";

	$aql->assign_delsection($username);
	$aql->save_config_file('gw_endpoints.conf');
	$aql->assign_addsection($username,$datachunk);
	$aql->save_config_file('gw_endpoints.conf');
	unlock_file($hlock);
	save_endpoints_to_sips();
	save_routings_to_extensions();

	exec("asterisk -rx \"core reload\" > /dev/null 2>&1 &");
	
	save_user_record("","SYSTEM->Setting Wizard:Save Sip endpoint=".$username);
	// wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");
}
function saveGroups(){
	$datachunk = '';
	$section = '';
	if(!isset($_POST['group_name'])) return false;
	$section = trim($_POST['group_name']); 

	$order = isset($_POST['group_order']) ? trim($_POST['group_order']) : '1';
	$policy = isset($_POST['policy']) ? trim($_POST['policy']) : 'ascending';

	$datachunk .= 'order=' . $order . "\n";
	$datachunk .= 'type=gsm' . "\n";
	$datachunk .= 'policy=' . $policy . "\n";

/*
	$member_type = 'gsm_members';
	$members = '';
	if( isset($_POST[$member_type]) && is_array($_POST[$member_type]) ) {
		foreach($_POST[$member_type] as $value) {
			$value = trim($value);
			$members .= $value != '' ? $value . ',' : '';
		}
	}
	if($members == '') return false;
*/
	$members = $_POST['members'];
	$members = rtrim($members,','); 
	$datachunk .= 'members=' . $members . "\n";

	//Save to gw_group.conf
	///////////////////////////////////////////////////
	$gw_group_conf_path = "/etc/asterisk/gw_group.conf";
	$hlock = lock_file($gw_group_conf_path);
	if (!file_exists($gw_group_conf_path)) fclose(fopen($gw_group_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($gw_group_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	$aql->assign_delsection($section);
	$aql->save_config_file('gw_group.conf');
	$aql->assign_addsection($section,$datachunk);
	$aql->save_config_file('gw_group.conf');
	unlock_file($hlock);
	///////////////////////////////////////////////////

	save_routings_to_extensions();
	exec("cd /my_tools/lua/sms_routing; lua read_group.lua");
	exec("asterisk -rx \"core reload\" > /dev/null 2>&1 &");
	// wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");

	save_user_record("","SYSTEM->Setting Wizard:Save Group=".$section);
	return true;
}

function match_dialpattern($str)
{
	if(preg_match('/^[*#+0-9XZN.]+$/',$str)) {
		return true;
	}
	return false;
}

function match_dialpattern_add($str)
{
	if(preg_match('/^[*#+0-9XZN.\-\[\]]+$/',$str)) {
		return true;
	}
	return false;
}

function saveRoutingRules()
{
	$from_datachunk = '';
	$to_datachunk = '';
	$section = '';
	$from_channel = '';
	$to_channel = '';
	if(!isset($_POST['routing_name'])) return false;
	$section = trim($_POST['routing_name']); 
	if(!isset($_POST['from_channel']) && trim($_POST['from_channel']) == 'none') return false;
	$from_channel = trim($_POST['from_channel']);
	if(!isset($_POST['to_channel']) && trim($_POST['to_channel']) == 'none') return false;
	$to_channel = trim($_POST['to_channel']);

	//prepend
	$prepend = $_POST['prepend'];
	$prepend = rtrim($prepend, ',');
	$prepend_temp = explode(',', $prepend);
	for($i=0;$i<count($prepend_temp);$i++){
		$dp_ary[$i]['prepend'] = trim($prepend_temp[$i]);
	}
	
	//prefix
	$prefix = $_POST['prefix'];
	$prefix = rtrim($prefix, ',');
	$prefix_temp = explode(',', $prefix);
	for($i=0;$i<count($prefix_temp);$i++){
		$dp_ary[$i]['prefix'] = trim($prefix_temp[$i]);
	}
	
	//pattern
	$pattern = $_POST['pattern'];
	$pattern = rtrim($pattern, ',');
	$pattern_temp = explode(',', $pattern);
	for($i=0;$i<count($pattern_temp);$i++){
		$dp_ary[$i]['pattern'] = trim($pattern_temp[$i]);
	}
	
	//callerid
	$callerid = $_POST['callerid'];
	$callerid = rtrim($callerid, ',');
	$callerid_temp = explode(',', $callerid);
	for($i=0;$i<count($callerid_temp);$i++){
		$dp_ary[$i]['cid'] = trim($callerid_temp[$i]);
	}
/*
	$i = 0;
	if(isset($_POST['prepend']) && is_array($_POST['prepend']) && $_POST['prepend'] != '') {
		foreach($_POST['prepend'] as $each) {
			$dp_ary[$i++]['prepend'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['prefix']) && is_array($_POST['prefix']) && $_POST['prefix'] != '') {
		foreach($_POST['prefix'] as $each) {
			$dp_ary[$i++]['prefix'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['pattern']) && is_array($_POST['pattern']) && $_POST['pattern'] != '') {
		foreach($_POST['pattern'] as $each) {
			$dp_ary[$i++]['pattern'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['cid']) && is_array($_POST['cid']) && $_POST['cid'] != '') {
		foreach($_POST['cid'] as $each) {
			$dp_ary[$i++]['cid'] = trim($each);
		}
	}
*/

	$dial_pattern = '';

	if(isset($dp_ary) && is_array($dp_ary)) {
		foreach($dp_ary as $each) {
			$prepend = '';
			if(isset($each['prepend'])) {
				$tmp = trim($each['prepend']);
				if(match_dialpattern($tmp)) {
					$prepend =  $tmp;
				}
			}
			$prefix = '';
			if(isset($each['prefix'])) {
				$tmp = trim($each['prefix']);
				if(match_dialpattern_add($tmp)) {
					$prefix =  $tmp;
				}
			}
			$pattern = '';
			if(isset($each['pattern'])) {
				$tmp = trim($each['pattern']);
				if(match_dialpattern_add($tmp)) {
					$pattern =  $tmp;
				}
			}
			$cid = '';
			if(isset($each['cid'])) {
				$tmp = trim($each['cid']);
				if(match_dialpattern($tmp)) {
					$cid =  $tmp;
				}
			}

			if(!($prepend=='' && $prefix=='' && $pattern=='' && $cid=='')) {
				$dp = "$prepend|$prefix|$pattern|$cid";
				$dial_pattern .= $dp.',';
			}
		}
		$dial_pattern = rtrim($dial_pattern,',');
	}
	$forward_number = isset($_POST['forward_number']) ? trim($_POST['forward_number']) : '';

	$from_datachunk .= 'order=1' . "\n";
	$from_datachunk .= 'from_channel=' . $from_channel . "\n";
	$from_datachunk .= 'to_channel=' . $to_channel . "\n";
	$from_datachunk .= 'dial_pattern=' . $dial_pattern . "\n";
	$from_datachunk .= 'time_pattern=' . "\n";
	$from_datachunk .= 'cid_name=' . "\n";
	$from_datachunk .= 'cid_number=' . "\n";
	$from_datachunk .= 'forward_number=' . "\n";
	$from_datachunk .= 'DISA_sw=off' . "\n";
	$from_datachunk .= 'second_dial_sw=off' . "\n";
	$from_datachunk .= 'timeout=' . "\n";
	$from_datachunk .= 'max_passwd_digits=' . "\n";

	if(isset($_POST['create_inbound_routes']) && trim($_POST['create_inbound_routes']) == 'on'){
		$to_section = 'inbound';
		$to_datachunk .= 'order=2' . "\n";
		$to_datachunk .= 'from_channel=' . $to_channel . "\n";
		$to_datachunk .= 'to_channel=' . $from_channel . "\n";
		$to_datachunk .= 'dial_pattern=' . "\n";
		$to_datachunk .= 'time_pattern=' . "\n";
		$to_datachunk .= 'cid_name=' . "\n";
		$to_datachunk .= 'cid_number=' . "\n";
		$to_datachunk .= 'forward_number=' . $forward_number ."\n";
		$to_datachunk .= 'DISA_sw=off' . "\n";
		$to_datachunk .= 'second_dial_sw=off' . "\n";
		$to_datachunk .= 'timeout=' . "\n";
		$to_datachunk .= 'max_passwd_digits=' . "\n";
	}
	//Save to gw_routing.conf
	///////////////////////////////////////////////////
	
	$gw_routing_conf_path = "/etc/asterisk/gw_routing.conf";
	$hlock = lock_file($gw_routing_conf_path);
	if (!file_exists($gw_routing_conf_path)) fclose(fopen($gw_routing_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($gw_routing_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	
	$aql->assign_delsection($section);
	$aql->save_config_file('gw_routing.conf');
	$aql->assign_addsection($section,$from_datachunk);
	if(isset($_POST['create_inbound_routes']) && trim($_POST['create_inbound_routes']) == 'on'){
		$aql->assign_delsection($to_section);
		$aql->save_config_file('gw_routing.conf');
		$aql->assign_addsection($to_section, $to_datachunk);
	}
	$aql->save_config_file('gw_routing.conf');

	unlock_file($hlock);
	save_routings_to_extensions();
	
	save_user_record("","SYSTEM->Setting Wizard:Save from_section=".$section." and to_section=".$to_section);
}
function saveDestination()
{
	saveGroups();
	saveRoutingRules();
}
function closeWizardSetting()
{
	$oem_ver_ctl_path = '/etc/asterisk/gw.conf';
	$hlock = lock_file($oem_ver_ctl_path);
	if(!file_exists($oem_ver_ctl_path)) fclose(fopen($oem_ver_ctl_path, "w"));
	$aql = new aql();
	$aql->set('basedir', '/etc/asterisk');
	if(!$aql->open_config_file($oem_ver_ctl_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}

	$res = $aql->query("select * from gw.conf");
	if(!is_array($res['setup-wizard'])){
		$aql->assign_addsection('setup-wizard', "set_wizard=off\n");
	}else{
		if(!isset($res['setup-wizard']['set_wizard'])){
			$aql->assign_append('setup-wizard', 'set_wizard', 'off');
		} else {
			$aql->assign_editkey('setup-wizard', 'set_wizard', 'off');
		}
	}

	$aql->save_config_file('gw.conf');
	unlock_file($hlock);
	exec("cp /etc/asterisk/gw.conf /etc/cfg/");
}

?>
