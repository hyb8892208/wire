<?php
//AQL
include_once("/www/cgi-bin/inc/aql.php");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
include_once("/www/cgi-bin/inc/config.inc");

function get_cdr_sw()
{
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw.conf');
	$res = $aql->query("select * from gw.conf where section='cdr'");
	unlock_file($hlock);
	if(isset($res['cdr']['switch'])) {
		if(is_true(trim($res['cdr']['switch']))) {
			return true;
		}
	}
	return false;
}

function get_sys_log_sw()
{
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw.conf');
	$res = $aql->query("select * from gw.conf where section='sys-log'");
	unlock_file($hlock);
	if(isset($res['sys-log']['switch'])) {
		if(is_true(trim($res['sys-log']['switch']))) {
			return true;
		}
	}

	return false;
}

function get_sip_log_sw()
{
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw.conf');
	$res = $aql->query("select * from gw.conf where section='sip-log'");
	unlock_file($hlock);
	if(isset($res['sip-log']['switch'])) {
		if(is_true(trim($res['sip-log']['switch']))) {
			return true;
		}
	}

	return false;
}

function get_debugat_log_sw()
{
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw.conf');
	$res = $aql->query("select * from gw.conf where section='debugat-log'");
	unlock_file($hlock);
	if(isset($res['debugat-log']['switch'])) {
		if(is_true(trim($res['debugat-log']['switch']))) {
			return true;
		}
	}

	return false;
}

function get_intercall_sw()
{
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$hlock = lock_file('/etc/asterisk/gw/auto_intercall.conf');
	$res = $aql->query("select * from auto_intercall.conf where section='general'");
	unlock_file($hlock);
	if(isset($res['general']['enable'])) {
		if(is_true(trim($res['general']['enable']))) {
			return true;
		}
	}

	return false;
}

function get_hangupcausecode()
{
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/');
	$hlock = lock_file('/etc/asterisk/sip_general.conf');
	$res = $aql->query("select * from sip_general.conf where section='general'");
	unlock_file($hlock);
	if(isset($res['general']['hangupcausecode'])) {
		return trim($res['general']['hangupcausecode']);
	}

	return '';
}

/*
function get_gsm_dialprefix($gsm_channel)
{
	if(!$gsm_channel)
		return '';

	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_gsm.conf');
	$res = $aql->query("select * from gw_gsm.conf where section=\"$gsm_channel\"");
	unlock_file($hlock);
	if(isset($res[$gsm_channel]['dialprefix'])) {
		return trim($res[$gsm_channel]['dialprefix']);
	}

	return '';
}
*/

function get_smssender_option()
{
	$ret = array();
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$hlock = lock_file('/etc/asterisk/gw/sms.conf');
	$val = $aql->query("select * from sms.conf where section='send'");
	unlock_file($hlock);
	if(isset($val['send']['attempt'])) {
		$ret['attempt'] = trim($val['send']['attempt']);
	}

	if(isset($val['send']['repeat'])) {
		$ret['repeat'] = trim($val['send']['repeat']);
	}

	if(isset($val['send']['verbose'])) {
		$ret['verbose'] = trim($val['send']['verbose']);
	}

	return $ret;
}


function get_sip_username($endpoint_name)
{
/* /etc/asterisk/gw_endpoints.conf */
// [endpoint_name]
// username = 
// registration =  
// register =    like sip.conf [general]
// .......       like all sip.conf

	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_endpoints.conf');
	$res = $aql->query("select * from gw_endpoints.conf where section='$endpoint_name'");
	unlock_file($hlock);

	if(isset($res[$endpoint_name]['username'])) {
		return trim($res[$endpoint_name]['username']);
	}

	return '';
}


function get_sip_username_and_ip($endpoint_name)
{
/* /etc/asterisk/gw_endpoints.conf */
// [endpoint_name]
// username = 
// registration =  
// register =    like sip.conf [general]
// .......       like all sip.conf

/*
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_endpoints.conf');
	$res = $aql->query("select * from gw_endpoints.conf where section='$endpoint_name'");
	unlock_file($hlock);
*/
	$res = get_config("/etc/asterisk/gw_endpoints.conf", $endpoint_name);

	if(isset($res[$endpoint_name]['username'])) {
		$username = trim($res[$endpoint_name]['username']);

		if( isset($res[$endpoint_name]['outboundproxy']) && trim($res[$endpoint_name]['outboundproxy']) != '' && isset($res[$endpoint_name]['enableoutboundtohost']) && trim($res[$endpoint_name]['enableoutboundtohost']) == 'yes'){
			$outboundproxy = explode(":", $res[$endpoint_name]['outboundproxy'], 2);
			$username = trim($res[$endpoint_name]['username']) . '-' . $outboundproxy[0];
			return $username;
		}

		if(isset($res[$endpoint_name]['host'])) {
			$ip = trim($res[$endpoint_name]['host']);
			if($ip != '' && $ip != 'dynamic') {

				if(isset($res[$endpoint_name]['allow_anonymous']) && $res[$endpoint_name]['allow_anonymous'] == 'yes') {
					return 'anonymous-'.$ip;
				}

				return $username.'-'.$ip;
			}

			return $username;
		} else {
			return $username;
		}
	}

	return '';
}

function get_iax_username_and_ip($endpoint_name)
{
/* /etc/asterisk/gw_iax_endpoints.conf */
// [endpoint_name]
// username = 
// registration =  
// register =    like sip.conf [general]
// .......       like all sip.conf

/*
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_iax_endpoints.conf');
	$res = $aql->query("select * from gw_iax_endpoints.conf where section='$endpoint_name'");
	unlock_file($hlock);
*/
	$res = get_config("/etc/asterisk/gw_iax_endpoints.conf", $endpoint_name);

	if(isset($res[$endpoint_name]['username'])) {
		$username = trim($res[$endpoint_name]['username']);

		if(isset($res[$endpoint_name]['host'])) {
			$ip = trim($res[$endpoint_name]['host']);
			//if($ip != '' && $ip != 'dynamic') {
			//	return $username.'-'.$ip;
			//}
			return $username;
		} else {
			return $username;
		}
	}

	return '';
}

function get_sip_registration($endpoint_name)
{
/* /etc/asterisk/gw_endpoints.conf */
// [endpoint_name]
// username = 
// registration =  
// register =    like sip.conf [general]
// .......       like all sip.conf
/*
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_endpoints.conf');
	$res = $aql->query("select * from gw_endpoints.conf where section='$endpoint_name'");
	unlock_file($hlock);
*/
	$res = get_config("/etc/asterisk/gw_endpoints.conf", $endpoint_name);	

	if(isset($res[$endpoint_name]['registration'])) {
		return trim($res[$endpoint_name]['registration']);
	}

	return '';
}

function get_iax_registration($endpoint_name)
{
/* /etc/asterisk/gw_iax_endpoints.conf */
// [endpoint_name]
// username = 
// registration =  
// register =    like sip.conf [general]
// .......       like all sip.conf
/*
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_iax_endpoints.conf');
	$res = $aql->query("select * from gw_iax_endpoints.conf where section='$endpoint_name'");
	unlock_file($hlock);
*/
	$res = get_config("/etc/asterisk/gw_iax_endpoints.conf", $endpoint_name);	

	if(isset($res[$endpoint_name]['registration'])) {
		return trim($res[$endpoint_name]['registration']);
	}

	return '';
}

function get_channel_info($type_and_name)
{//Support gsm,sip,group,borad(other slot gateway)   add iax
	$info['type'] = '';
	$info['name'] = '';
	$info['device'] = '';
	$info['context'] = '';
	$info['registration'] = '';    //GSM not has this.
	$info['channel'] = '';         //SIP not has this.

	global $__GSM_HEAD__;
	global $__SIP_HEAD__;
	global $__BRD_HEAD__;
	global $__GRP_HEAD__;
	global $__IAX_HEAD__;

	if(substr($type_and_name,0,strlen($__SIP_HEAD__)) == $__SIP_HEAD__) { //SIP  sip-sipendpointname
		$name = substr($type_and_name,strlen($__SIP_HEAD__));
		$sip_username_and_ip = get_sip_username_and_ip($name);
		$registration = get_sip_registration($name);
		if($sip_username_and_ip == '') {
			$context = '';
			$device = '';
		} else {
			$context = $__SIP_HEAD__.$sip_username_and_ip;
			$device = "SIP/$sip_username_and_ip";
		}

		$info['type'] = 'sip';
		$info['name'] = $name;
		$info['context'] = $context;
		$info['device'] = $device;
		$info['registration'] = $registration;
	} else if (substr($type_and_name,0,strlen($__IAX_HEAD__)) == $__IAX_HEAD__){
		$name = substr($type_and_name,strlen($__IAX_HEAD__));
		$iax_username_and_ip = get_iax_username_and_ip($name);
		$registration = get_iax_registration($name);
		if ($iax_username_and_ip == '') {
			$context = '';
			$device = '';
		} else {
			$context = $__IAX_HEAD__.$iax_username_and_ip;
			$device = "IAX2/$iax_username_and_ip";
		}
		$info['type'] = 'sip';
		$info['name'] = $name;
		$info['context'] = $context;
		$info['device'] = $device;
		$info['registration'] = $registration;
	} else if(substr($type_and_name,0,strlen($__GSM_HEAD__)) == $__GSM_HEAD__) { //GSM  gsm-1
		$name = substr($type_and_name,strlen($__GSM_HEAD__));
		if($name == '') {
			$context = '';
			$device = '';
		} else {
			$info['channel'] = $name;
			$voice_channel = $name*2-1;
			$context = $__GSM_HEAD__.$name;
			$name = $__GSM_HEAD__.$name;
			$device = "extra/$voice_channel";
		}

		$info['type'] = 'gsm';
		$info['name'] = $name;
		$info['context'] = $context;
		$info['device'] = $device;
	} else if(substr($type_and_name,0,strlen($__BRD_HEAD__)) == $__BRD_HEAD__) { //BOARD GSM  Board-2-gsm-1
		@sscanf($type_and_name,"$__BRD_HEAD__%d-$__GSM_HEAD__%d",$board_num,$gsm_num);

		$context = '';
		$device = '';

		if($gsm_num != '' && $board_num != '') {
			$slave_info = get_slave_info($board_num);
			if($slave_info['password'] != '' && $slave_info['ip'] != '') {
				$context = $__BRD_HEAD__.$board_num.'-'.$__GSM_HEAD__.$gsm_num;

				$device = 'SIP/'.$slave_info['password'].$gsm_num.'-'.$slave_info['ip'];
			}
		}

		$info['type'] = 'brd';
		$info['name'] = $type_and_name;
		$info['context'] = $context;
		$info['device'] = $device;
		$info['channel'] = $gsm_num;
	} else if(substr($type_and_name,0,strlen($__GRP_HEAD__)) == $__GRP_HEAD__) { //group
		$name = substr($type_and_name,strlen($__GRP_HEAD__));
		$info['type'] = 'grp';
		$info['name'] = $name;
	} else if(!strcmp($type_and_name,"anysip")) {
		$info['type'] = 'anysip';
		$info['name'] = 'anysip';
		$info['context'] = 'sipdefault';
	}

	return $info;
}

function get_internal_sip_call()
{
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/sip_general.conf');
	$sip_general = $aql->query("select * from sip_general.conf");
	unlock_file($hlock);
	if( isset($sip_general['general']['enable_internal_sip_call']) &&
		isset($sip_general['general']['internal_sip_call_prefix'])
	) {
		if($sip_general['general']['enable_internal_sip_call'] == 'yes' ) {
			$ret['prefix'] = trim($sip_general['general']['internal_sip_call_prefix']);
			return $ret;
		}
	}
		
	return false;	
}

function get_all_iaxs($sort = false)
{
/* /etc/asterisk/gw_iax_endpoints.conf.conf */
// [endpoint_name]
// order = 1,2,3,4,5....    //Must set
// username = 
// registration =  
// register =    like sip.conf [general]
// .......       like all sip.conf

	$iaxs = array();
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_iax_endpoints.conf');
	$iaxs = $aql->query("select * from gw_iax_endpoints.conf");
	//print_rr($iaxs);
	unlock_file($hlock);
	if($iaxs) {
		foreach($iaxs as $endpoint_name=>$content) {
			/*
			if(!isset($content['order'])) {
				unset($iaxs[$endpoint_name]);
				continue;
			}
			*/
			
			if(!isset($content['username'])) {
				unset($iaxs[$endpoint_name]);
				continue;
			}
			
			if(!isset($content['registration'])) {
				unset($iaxs[$endpoint_name]);
				continue;
			}
			
			$username = trim($content['username']);
			if($username == '') {
				unset($iaxs[$endpoint_name]);
				continue;
			}

			$registration = trim($content['registration']);
			if($registration == '') {
				unset($iaxs[$endpoint_name]);
				continue;
			}

					
			$iaxs[$endpoint_name]['endpoint_name'] = trim($endpoint_name);

			if($sort) {
				//$ary2[$endpoint_name]  = $order;
			}
		}

		if(isset($ary2)) {
			array_multisort($ary2,SORT_ASC,$iaxs);  //It will be change key if your set key is numbers. Freedom Note 2012-11-27
		}
	}

	return $iaxs;
}


function get_all_sips($sort = false)
{
/* /etc/asterisk/gw_endpoints.conf */
// [endpoint_name]
// order = 1,2,3,4,5....    //Must set
// username = 
// registration =  
// register =    like sip.conf [general]
// .......       like all sip.conf

	$sips = array();
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_endpoints.conf');
	$sips = $aql->query("select * from gw_endpoints.conf");
	unlock_file($hlock);

	if($sips) {
		foreach($sips as $endpoint_name=>$content) {
			if(!isset($content['order'])) {
				unset($sips[$endpoint_name]);
				continue;
			}

			if(!isset($content['username'])) {
				unset($sips[$endpoint_name]);
				continue;
			}
			if(!isset($content['registration'])) {
				unset($sips[$endpoint_name]);
				continue;
			}

			$username = trim($content['username']);
			if($username == '') {
				unset($sips[$endpoint_name]);
				continue;
			}

			$registration = trim($content['registration']);
			if($registration == '') {
				unset($sips[$endpoint_name]);
				continue;
			}

			$order = trim($content['order']);
			if($order == '') {
				unset($sips[$endpoint_name]);
				continue;
			}

			$sips[$endpoint_name]['endpoint_name'] = trim($endpoint_name);

			if($sort) {
				$ary2[$endpoint_name]  = $order;
			}
		}

		if(isset($ary2)) {
			array_multisort($ary2,SORT_ASC,$sips);  //It will be change key if your set key is numbers. Freedom Note 2012-11-27
		}
	}

	return $sips;
}


function get_all_routings($sort = false)
{
	// /etc/asterisk/gw_routing.conf
	// [routing_name]
	// order = 1,2,3,4,5....    //Must set
	// from_channel = channel (sip-sipname or gsm-gsmname  type: sip- is SIP, gsm- is GSM)
	// to_channel = channel,channel......
	// dial_pattern = prepend|prefix|match pattern|cid,prepend|prefix|match pattern|cid........
	// time_pattern = time|week|day|month,time|week|day|month........
	// cid_name
	// cid_number
	// forward_number

	$routings = array();
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_routing.conf');
	$routings = $aql->query("select * from gw_routing.conf");
	unlock_file($hlock);

	if($routings) {
		foreach($routings as $key=>$value) {
			
			if(!isset($value['from_channel']) || (trim($value['from_channel']) == '')) {
				unset($routings[$key]);
				continue;
			} 

			if(!isset($value['to_channel']) || (trim($value['to_channel']) == '')) {
				unset($routings[$key]);
				continue;
			}

			if(!isset($value['order']) || (trim($value['order']) == '')) {
				unset($routings[$key]);
				continue;
			}

			$order = trim($value['order']);
			if($order == '') {
				unset($routings[$key]);
				continue;
			}

			$routings[$key]['routing_name'] = trim($key);    //Must save key, because call array_multisort then will change key name. Freedom Note 2012-11-27
			if(!isset($routings[$key]['dial_pattern']))
				$routings[$key]['dial_pattern'] = '';

			if(!isset($routings[$key]['time_pattern']))
				$routings[$key]['time_pattern'] = '';

			if(!isset($routings[$key]['cid_name']))
				$routings[$key]['cid_name'] = '';

			if(!isset($routings[$key]['cid_number']))
				$routings[$key]['cid_number'] = '';

			if(!isset($routings[$key]['forward_number']))
				$routings[$key]['forward_number'] = '';
			
			if(!isset($routings[$key]['DISA_sw']))
				$routings[$key]['DISA_sw'] = '';
			if(!isset($routings[$key]['timeout']))
				$routings[$key]['timeout'] = '';	
							
			if(!isset($routings[$key]['second_dial_sw']))
				$routings[$key]['second_dial_sw'] = '';	
				

			if($sort) {
				$ary2[$key]  = $order;
			}
		}

		if(isset($ary2)) {
			array_multisort($ary2,SORT_ASC,$routings);  //It will be change key if your set key is numbers. Freedom Note 2012-11-27
		}
	}

	if(is_array($routings)) {
		return $routings;
	}

	return false;
}

function get_routings_info(){
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_routing.conf');
	$routings = $aql->query("select * from gw_routing.conf");
	unlock_file($hlock);
	
	return $routings;
}

function get_all_sms_routing($sort = false)
{
	// /etc/asterisk/gw_group.conf
	// [group_name]
	// order = 1,2,3,4,5....    //Must set
	// type = sip or gsm
	// policy = 
	// members = channel,channel......

	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/sms_routing.conf');
	$groups = $aql->query("select * from sms_routing.conf");
	unlock_file($hlock);

	if($groups) {
		foreach($groups as $key=>$value) {
			if(!isset($value['type']) || (trim($value['type']) == '')) {
				unset($groups[$key]);
				continue;
			} 

			if(!isset($value['policy']) || (trim($value['policy']) == '')) {
				unset($groups[$key]);
				continue;
			} 


			if(!isset($value['from_member']) || (trim($value['from_member']) == '')) {
				unset($groups[$key]);
				continue;
			}
			
			if(!isset($value['to_member']) || (trim($value['to_member']) == '')) {
				unset($groups[$key]);
				continue;
			}
			
			if(!isset($value['to_number']) || (trim($value['to_number']) == '')) {
				unset($groups[$key]);
				continue;
			}

			if(!isset($value['order']) || (trim($value['order']) == '')) {
				unset($groups[$key]);
				continue;
			}

			$order = trim($value['order']);
			if($order == '') {
				unset($groups[$key]);
				continue;
			}

			$groups[$key]['routing_name'] = trim($key);    //Must save key, because call array_multisort then will change key name. Freedom Note 2012-11-27

			if($sort) {
				$ary2[$key]  = $order;
			}
		}

		if(isset($ary2)) {
			array_multisort($ary2,SORT_ASC,$groups);  //It will be change key if your set key is numbers. Freedom Note 2012-11-27
		}
	}

	return $groups;
}

function get_all_firewall_rules($sort = false)
{
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$hlock = lock_file('/etc/asterisk/gw/firewall_rules.conf');
	$rules = $aql->query("select * from firewall_rules.conf");
	unlock_file($hlock);
	if($rules) {
		foreach($rules as $key=>$value) {
			if(!isset($value['protocol']) || (trim($value['protocol']) == '')) {
				unset($rules[$key]);
				continue;
			} 
			if(!isset($value['actions']) || (trim($value['actions']) == '')) {
				unset($rules[$key]);
				continue;
			} 
			if(!isset($value['port'])) {
				unset($rules[$key]);
				continue;
			}
			if(!isset($value['ip']) ) {
				unset($rules[$key]);
				continue;
			} 

			if(!isset($value['order']) || (trim($value['order']) == '')) {
				unset($rules[$key]);
				continue;
			}

			$order = trim($value['order']);
			if($order == '') {
				unset($rules[$key]);
				continue;
			}

			$rules[$key]['rule_name'] = trim($key);    //Must save key, because call array_multisort then will change key name. Freedom Note 2012-11-27

			if($sort) {
				$ary2[$key]  = $order;
			}
		}

		if(isset($ary2)) {
			array_multisort($ary2,SORT_ASC,$rules);  //It will be change key if your set key is numbers. Freedom Note 2012-11-27
		}
	}
	return $rules;
}
function update_sipname_from_routing_conf($old_name, $new_name)
{
	global $__SIP_HEAD__;

    $all_routings = get_all_routings(true);
    if (!$all_routings) {
            return;
    }

	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_routing.conf');
	if(!$aql->check_database_exists('gw_routing.conf')) {
	if(!$aql->open_config_file('/etc/asterisk/gw_routing.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
		}
	}

    foreach($all_routings as $routing) {
            $routing_name = trim($routing['routing_name']);
            $from_channel = trim($routing['from_channel']);
            $to_channel = trim($routing['to_channel']);
            $sip_pattern = $__SIP_HEAD__.$old_name;
            if (preg_match("/$sip_pattern/i", $from_channel) || preg_match("/$sip_pattern/i", $to_channel)) {
            //if (($from_channel == $__SIP_HEAD__.$old_name) || ($to_channel == $__SIP_HEAD__.$old_name)) {
                    if ($new_name == '') {
                            $aql->assign_delsection($routing_name);
                    } else {
                            if ($from_channel == $__SIP_HEAD__.$old_name) {
    			        $aql->assign_editkey($routing_name, 'from_channel', $__SIP_HEAD__.$new_name);
                            } else if ($to_channel == $__SIP_HEAD__.$old_name) {
    			        $aql->assign_editkey($routing_name, 'to_channel', $__SIP_HEAD__.$new_name);
                            }
                    }
            }
    }

	if (!$aql->save_config_file('gw_routing.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	unlock_file($hlock);
}

function get_all_groups($sort = false)
{
	// /etc/asterisk/gw_group.conf
	// [group_name]
	// order = 1,2,3,4,5....    //Must set
	// type = sip or gsm
	// policy = 
	// members = channel,channel......

	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_group.conf');
	$groups = $aql->query("select * from gw_group.conf");
	unlock_file($hlock);

	if($groups) {
		foreach($groups as $key=>$value) {
			if(!isset($value['type']) || (trim($value['type']) == '')) {
				unset($groups[$key]);
				continue;
			} 

			if(!isset($value['policy']) || (trim($value['policy']) == '')) {
				unset($groups[$key]);
				continue;
			} 


			if(!isset($value['members']) || (trim($value['members']) == '')) {
				unset($groups[$key]);
				continue;
			}

			if(!isset($value['order']) || (trim($value['order']) == '')) {
				unset($groups[$key]);
				continue;
			}

			$order = trim($value['order']);
			if($order == '') {
				unset($groups[$key]);
				continue;
			}

			$groups[$key]['group_name'] = trim($key);    //Must save key, because call array_multisort then will change key name. Freedom Note 2012-11-27

			if($sort) {
				$ary2[$key]  = $order;
			}
		}

		if(isset($ary2)) {
			array_multisort($ary2,SORT_ASC,$groups);  //It will be change key if your set key is numbers. Freedom Note 2012-11-27
		}
	}

	return $groups;
}


function get_group_info($name,$sort=false)
{
	// /etc/asterisk/gw_group.conf
	// [group_name]
	// order = 1,2,3,4,5....    //Must set
	// type = sip or gsm
	// policy = 
	// members = channel,channel......

	$info = array();
	$members = array();
	$devices = '';

	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_group.conf');
	$groups = $aql->query("select * from gw_group.conf");
	unlock_file($hlock);

	$type = '';
	$policy = '';

	if(isset($groups[$name]['members']) && isset($groups[$name]['type']) && isset($groups[$name]['policy'])) {
		$output = explode(',',$groups[$name]['members']);
		foreach($output as $mb) {
			$channel = trim($mb);
			$members[] = $channel;
			$chan_info = get_channel_info($channel);
			$devices .= $chan_info['device'].'&';
		}
		$devices = rtrim($devices,'&');

		$type = trim($groups[$name]['type']);
		$policy = trim($groups[$name]['policy']);
	}

	$info['members'] = $members;
	$info['type'] = $type;
	$info['policy'] = $policy;
	$info['devices'] = $devices;
	$info['order'] = $groups[$name]['order'];

	return $info;
}


function is_valid_phone_num($str)
{
	if(preg_match('/^[*#+0-9]+$/',$str)) {
		return true;
	}
	return false;
}

function save_routings_to_extensions()
{
	// /etc/asterisk/gw_routing.conf
	// [routing_name]
	// order = 1,2,3,4,5....    //Must set
	// from_channel = channel (sip-sipname or gsm-gsmname  type: sip- is SIP, gsm- is GSM, or Board-gsm- is other board gsm)
	// to_channel = channel,channel......
	// dial_pattern = prepend|prefix|match pattern|cid,prepend|prefix|match pattern|cid........
	// time_pattern = time|week|day|month,time|week|day|month........
	// cid_name
	// cid_number
	// forward_number
	// custom_context

	global $__SIP_HEAD__;
	global $__GSM_HEAD__;
	global $__RTG_HEAD__;
	global $__GSM_SUM__;
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	global $__MODULE_HEAD_ARRAY__;

	$MAX_PREFIX = 32;

	$cdr_sw = get_cdr_sw(); //

	$internalcall_sw = get_intercall_sw();

	$mnpinfo = get_mnp_info();

	$routing_blacklist_sw = trim(`/my_tools/set_config /etc/asterisk/gw_routing_blacklist.conf get option_value general enable_blacklist`);
	
	$hangupcausecode = get_hangupcausecode(); 

	$write_array = array();

	$all_routings = get_all_routings(true);
	$routings_info = get_routings_info();
	
	if($all_routings) {
		foreach($all_routings as $routing) {
			$routing_name = trim($routing['routing_name']);
			$fc = trim($routing['from_channel']);
			$fc_info = get_channel_info($fc);

			// get delay information
			if (isset($routing['transfer_delay'])) {
				$delay_setting = explode(',', $routing['transfer_delay']);	
				if (isset($delay_setting[0])) $delay_enabled = $delay_setting[0];
				if (isset($delay_setting[1])) $delay_min = "$delay_setting[1]";
				if (isset($delay_setting[2])) $delay_max = "$delay_setting[2]";
			}

			//get disa information
			$disa_sw = trim($routing['DISA_sw']);
			$second_dial_sw = trim($routing['second_dial_sw']);
			$disa_timeout = trim($routing['timeout']);
			$passwd_file = '/etc/asterisk/pin/'.$routing_name.'_disa_passwd.conf';
			//$max_passwd_digits = trim($routing['max_passwd_digits']);
			if (isset($routing['max_passwd_digits'])) {
				$max_passwd_digits = trim($routing['max_passwd_digits']);
			}
			
			// get custom context
			$custom_context = "";
			if (isset($routing['custom_context'])) {
				$custom_context = trim($routing['custom_context']);
			}

			$from_channels = array();
			if($fc_info['type'] == 'grp') {
				$grp_info = get_group_info($fc_info['name']);
				$from_channels = $grp_info['members'];
			} else {
				$from_channels[0] = $fc;
			}

			//Parse dial_pattern
			/////////////////////////////////////////////////////
			$dial_pattern = false;
			if(isset($routing['dial_pattern'])) {
				$dial_pattern = parse_dial_pattern(trim($routing['dial_pattern']));
			}
			/////////////////////////////////////////////////////

			//Parse time_pattern
			/////////////////////////////////////////////////////
			$time_pattern = false;
			if(isset($routing['time_pattern'])) {
				$time_pattern = parse_time_pattern(trim($routing['time_pattern']));
			}
			/////////////////////////////////////////////////////

			$routing_index = 0;
			foreach($from_channels as $from_channel) {
				$routing_index++;
				$routing_context = $__RTG_HEAD__.$routing_name.'-'.$routing_index;
				$routing_context_disa = $__RTG_HEAD__.$routing_name.'-disa';
				//echo "routing_disa".$routing_context_disa;

				$from_channel_info = get_channel_info($from_channel);
				$to_channel = trim($routing['to_channel']);

				$grp_policy_act_ary = array();   //For Group policy
				$grp_policy_name = '';

				if(isset($routing['cid_name']))
					$cid_name = trim($routing['cid_name']);
				else
					$cid_name = '';

				if(isset($routing['cid_number']))
					$cid_number = trim($routing['cid_number']);
				else
					$cid_number = '';

				if(isset($routing['forward_number']))
					$forward_number = trim($routing['forward_number']);
				else
					$forward_number = '';

				// [$__RTG_HEAD__ routing_name-$routing_index] context
				////////////////////////////////////////////////////////////////////
				$devices = '';
				$to_channels = explode(',',$to_channel);
				if($to_channels) {
					$to_channels_array = array();
					$to_flag = 0;
					foreach($to_channels as $key => $each) {
						$channel = trim($each);
						$info = get_channel_info($channel);
						if(($key == 0 && $info['type'] == 'gsm') || ($key == 0 && $info['type'] == 'brd')) {
							$to_flag = 1;
						}
						
						if($info['type'] == 'grp') {  //Group, It's need policy
							$grp_name = $info['name'];
							$to_grp_info = get_group_info($grp_name);
							if($key == 0 && $to_grp_info['type'] == 'gsm') {
								$to_flag = 1;
							}
							$grp_policy_name = 'POLICY_'.$grp_name;
						
							$temp = explode('-',$to_channel,2);
							$real_routing_name = $temp[1];
							$group_info = get_group_info($real_routing_name);
							
							if($group_info['type'] == 'sip' && $group_info['policy'] == 'ringall'){//sip-grp & ringall
								if($from_channel_info['type'] == 'sip' || $from_channel_info['type'] == 'anysip' || $second_dial_sw == "on") {
									$n = 0;
								}else{
									$n = 1;
								}
								
								$str = '';
								$cdr_name = '';
								for($i=0;$i<count($group_info['members']);$i++){
									$temp = explode('-',$group_info['members'][$i]);
									
									if($i == 0){
										$cdr_name = $temp[1];
									}
									
									$str .= 'SIP/'.$temp[1].'&';
								}
								$str = rtrim($str,"&");
								
								$devices .= "$str,$n,$cdr_name";
							}else{
								//$grp_policy_act_ary[] = "Set($grp_policy_name=\${SHELL(/my_tools/grppolicy \"$grp_name\" 2> /dev/null)})";
								//$grp_policy_act_ary[] = "Set($grp_policy_name=\${GRPPOLICY($grp_name)})";
								$grp_policy_act_ary[] = "GrpPolicy($grp_name)";
								$devices .= "\${".$grp_policy_name."},";
							}
							
							
							$to_channels_array[] = "\${".$grp_policy_name."}";
						} else {   // General channel
							$cdr_name = $info['name'];
							$device = $info['device'];
							
							//s:server, c:client, t:trunk.
							if($from_channel_info['type'] == 'sip' || $from_channel_info['type'] == 'anysip' || $second_dial_sw == "on") {
								if($info['type'] == 'sip') {
									if($info['registration'] == 'server') {
										$devices .= "$device,0,$cdr_name,";      // SIP(s,c,t) ---> SIP(s)   Transmit exten number
										$to_channels_array[] = "$device,0,$cdr_name";      // SIP(s,c,t) ---> SIP(s)   Transmit exten number
									} else {
										$devices .= "$device,0,$cdr_name,";      // SIP(s,c,t) ---> SIP(c,t) Transmit exten number
										$to_channels_array[] = "$device,0,$cdr_name";      // SIP(s,c,t) ---> SIP(c,t) Transmit exten number
									}
								} else {
									$devices .= "$device,0,$cdr_name,";          // SIP(s,c,t) ---> GSM      Transmit exten number
									$to_channels_array[] = "$device,0,$cdr_name";          // SIP(s,c,t) ---> GSM      Transmit exten number
								}
							} else {
								if($info['type'] == 'sip') {
									if($info['registration'] == 'server') {  
										$devices .= "$device,1,$cdr_name,";      // GSM ---> SIP(s)          Not need transmit exten number
										$to_channels_array[] = "$device,1,$cdr_name";      // GSM ---> SIP(s)          Not need transmit exten number
									} else {
										$devices .= "$device,1,$cdr_name,";      // GSM ---> SIP(c,t)        Not need transmit exten number
										$to_channels_array[] = "$device,1,$cdr_name";      // GSM ---> SIP(c,t)        Not need transmit exten number
									}
								} else {
									$devices .= "$device,1,$cdr_name,";          // GSM ---> GSM             Not need transmit exten number
									$to_channels_array[] = "$device,1,$cdr_name";          // GSM ---> GSM             Not need transmit exten number
								}
							}
						}
					}
					$devices = rtrim($devices,',');
				}

				if(isset($dial_pattern) && is_array($dial_pattern)) {
					$cid='';
					foreach($dial_pattern as $each){
						if(isset($each['cid'])) {
							$cid = $each['cid'];
							if (!is_valid_phone_num($cid)) {
								$cid = '_'.$cid;
							}
						}						
					}
					if($from_channel_info['type'] == 'gsm') {
						$exten['s']['e'] = 's';
						//$exten['s']['d'] = '';
						if (($disa_sw == 'on') && ($second_dial_sw == 'on') ) {
							
							$exten['s']['e'] = 's/'.$cid;
							$exten['s']['d'] = '${EXTEN}';
						} else {
							$exten['s']['e'] = 's/'.$cid;
							$exten['s']['d'] = '';
						}
						
					} else if($from_channel_info['type'] == 'brd') {
						$exten[0]['e'] = '_[*#+0-9].';
						//$exten[0]['d'] = '';
						if (($disa_sw == 'on') && ($second_dial_sw == 'on') ) {
							$exten[0]['e'] = '_[*#+0-9]./'.$cid;
							$exten[0]['d'] = '${EXTEN}';
						} else {
							$exten[0]['e'] = '_[*#+0-9]./'.$cid;
							$exten[0]['d'] = '';
						}
						
					} else {
						$i = 0;
						foreach($dial_pattern as $each) {
							$prepend = '';
							if(isset($each['prepend'])) {
								$prepend = $each['prepend'];
							}
							$prefix = '';
							if(isset($each['prefix'])) {
								$prefix = $each['prefix'];
							}
							$pattern = '';
							if(isset($each['pattern'])) {
								$pattern = $each['pattern'];
							}
							$cid = '';
							if(isset($each['cid'])) {
								$cid = $each['cid'];
							}

							if($prefix == '') {
								if($pattern == '') {
									$exten[$i]['e'] = '_[*#+0-9].';
								} else {
									if(is_valid_phone_num($pattern)) {
										$exten[$i]['e'] = "$pattern";
									} else {
										$exten[$i]['e'] = "_$pattern";
									}
								}
							} else {
								if($pattern == '') {
									if(is_valid_phone_num($prefix)) {
										$exten[$i]['e'] = "$prefix";
									} else {
										$exten[$i]['e'] = "_$prefix";
									}
								} else {
									if(is_valid_phone_num($prefix) && is_valid_phone_num($pattern)) {
										$exten[$i]['e'] = "$prefix$pattern";
									} else {
										$exten[$i]['e'] = "_$prefix$pattern";
									}
								}
							}

							if($cid != '') {
								if(is_valid_phone_num($cid)) {
									$exten[$i]['e'] .= "/$cid"; 
								} else {
									$exten[$i]['e'] .= "/_$cid";
								}
							}

							//$strip = strlen($prefix);
							// the prefix added symbol-"[ ]", recount the length of prefix
							if (preg_match('/\[.+\]/',$prefix)) {
								$prefix_match = preg_replace("/\[[\d\-*#+]+\]/","A",$prefix);
							} else {
								$prefix_match = $prefix;
							}
							$strip = strlen($prefix_match);

							if($prepend == '') {
								if($strip > 0) {
									$exten[$i]['d'] = "\${EXTEN:$strip}";
								} else {
									$exten[$i]['d'] = "\${EXTEN}";
								}
							} else {
								if(is_valid_phone_num($prepend)){
									if($strip > 0) {
										$exten[$i]['d'] = "$prepend\${EXTEN:$strip}";
									} else {
										$exten[$i]['d'] = "$prepend\${EXTEN}";
									}
								} else {
									if($strip > 0) {
										$exten[$i]['d'] = "_$prepend\${EXTEN:$strip}";
									} else {
										$exten[$i]['d'] = "_$prepend\${EXTEN}";
									}
								}
							}
						
							$i++;
						}
					} 
				} else {
					if($from_channel_info['type'] == 'gsm') {
						$exten['s']['e'] = 's';
						//$exten['s']['d'] = '';
						if (($disa_sw == 'on') && ($second_dial_sw == 'on')) {
							$exten['s']['d'] = '${EXTEN}';
						} else {
							$exten['s']['d'] = '';
						}
						
					} else if($from_channel_info['type'] == 'brd') {
						$exten[0]['e'] = '_[*#+0-9].';
						//$exten[0]['d'] = '';
						if (($disa_sw == 'on') && ($second_dial_sw == 'on')) {
							$exten[0]['d'] = '${EXTEN}';
						} else {
							$exten[0]['d'] = '';
						}
						
					} else {
						$exten[0]['e'] = '_[*#+0-9].';
						$exten[0]['d'] = '${EXTEN}';
					}
				}


				$routing_content = '';
				$routing_content_head = '';
				//get disa information 
				
				
				foreach($exten as $each) {
				
					
					if ( $disa_sw == 'on' ) {
						if ( $second_dial_sw == 'on' ) {
							//use sida macro
							
							$routing_content_head .= "exten => $each[e],1,NoOp($each[e] matches Rule $routing_context)\n";
							$routing_content_head .= "exten => $each[e],n,Macro(disa,$routing_context_disa,$passwd_file,$max_passwd_digits,$disa_timeout)". "\n";
							if($from_channel_info['type'] == 'gsm'){
								if($routing_blacklist_sw == 'on'){
									$routing_content .= "exten => $each[e],n,GotoIf(\${BLACKLIST()}?blacklisted)\n";
								}
							}
							$each['e'] = '_[*#+0-9].';
							$routing_content .= "exten => $each[e],1,NoOp($each[e] matches Rule $routing_context)\n";
						} else {
							//use auth macro
							$routing_content .= "exten => $each[e],1,NoOp($each[e] matches Rule $routing_context)\n";
							$routing_content .= "exten => $each[e],n,Macro(Auth,$passwd_file,$max_passwd_digits,$disa_timeout)"."\n";
							if($from_channel_info['type'] == 'gsm'){
								if($routing_blacklist_sw == 'on'){
									$routing_content .= "exten => $each[e],n,GotoIf(\${BLACKLIST()}?blacklisted)\n";
								}
							}
						}			
					} else {
						$routing_content .= "exten => $each[e],1,NoOp($each[e] matches Rule $routing_context)\n";
						if($from_channel_info['type'] == 'gsm'){
							if($routing_blacklist_sw == 'on'){
								$routing_content .= "exten => $each[e],n,GotoIf(\${BLACKLIST()}?blacklisted)\n";
							}
						}
					}
					
					
					if($from_channel_info['type'] == 'brd') {
						//Modify CALLERID(number)
						//SIP From header:
						// "GSM callerid"<sip:hide sip extension@ip>;
						// "GSM callerid" is ${CALLERID(name)}.
						// "hide sip extension" is ${CALLERID(number)}
						//Not need show 'hide sip extension'
						//So neeed change
						$routing_content .= "exten => $each[e],n,Set(CALLERID(number)=\${CALLERID(name)})\n";
						//$routing_content .= "exten => $each[e],n,SIPAddHeader(X-Best-Codec: \${CHANNEL(audionativeformat)})\n";
					}
					
					

					$routing_content .= "exten => $each[e],n,Set(CDR_CALLEEID=\${EXTEN})\n"; 

					//Set CID name and CID number
					//Must use temporay value to save CID name and CID number, If you need swap CID name and CID number.
					///////////////////////////////////////////////////////////////////////////////////////
					if($cid_name != '')
						$routing_content .= "exten => $each[e],n,Set(T_CID_NAME=$cid_name)\n";
					if($cid_number != '')
						$routing_content .= "exten => $each[e],n,Set(T_CID_NUM=$cid_number)\n";
					if($cid_name != '')
						$routing_content .= "exten => $each[e],n,Set(CALLERID(name)=\${T_CID_NAME})\n";
					if($cid_number != '')
						$routing_content .= "exten => $each[e],n,Set(CALLERID(number)=\${T_CID_NUM})\n";
					///////////////////////////////////////////////////////////////////////////////////////

					// set transfer_delay
					if (isset($routing['transfer_delay']) && $delay_enabled && isset($delay_min) && isset($delay_max)) {
						$routing_content .= "exten => $each[e],n,Ringing()\n";
						$routing_content .= "exten => $each[e],n,Wait(\${RAND($delay_min,$delay_max)})\n";
					}

					if($devices != '') {
						$_routing_name = $routing['to_channel'];
						$temp = explode('-',$_routing_name,2);
						$real_routing_name = $temp[1];
						$group_info = get_group_info($real_routing_name);
						
						$group_policy = $group_info['policy'];
						if($group_policy == 'ascending'){
							$policy_code = 'g'.$group_info['order'];
						}else if($group_policy == 'descending'){
							$policy_code = 'G'.$group_info['order'];
						}else if($group_policy == 'roundrobin'){
							$policy_code = 'r'.$group_info['order'];
						}else if($group_policy == 'reverseroundrobin'){
							$policy_code = 'R'.$group_info['order'];
						}else{
							$policy_code = '';
						}
					 
						if (($disa_sw == 'on') && ($second_dial_sw == 'on')) {
							$forward_number = '';
							$each['d'] = '${EXTEN}';
						}
						
						$group_name_temp = ltrim($_routing_name,'grp-');
						$grp_info_temp = get_group_info($group_name_temp);
						if($grp_info_temp['type'] == 'sip'){
						// if(!strstr($_routing_name, 'grp-')){
							foreach($grp_policy_act_ary as $act) {
								$routing_content .= "exten => $each[e],n,$act\n";
							}
						}

						if(is_true($mnpinfo['enable']) && $mnpinfo['manipulation'] == 'before' && $each['e'] != 's') {
							$mnpinfo['url'] = str_replace("\${num}","$each[d]",$mnpinfo['url']);

							$routing_content .= "exten => $each[e],n,Set(CURLOPT(conntimeout)=$mnpinfo[timeout])\n";
							$routing_content .= "exten => $each[e],n,Set(CURLOPT(httptimeout)=$mnpinfo[timeout])\n";
							$routing_content .= "exten => $each[e],n,Set(CURLOPT(ssl_verifypeer)=0)\n";
							$routing_content .= "exten => $each[e],n,Set(NEWEXTEN=\${CURL($mnpinfo[url])})\n";
							$routing_content .= "exten => $each[e],n,GotoIf(\$[\"\${NEWEXTEN}\" != \"\"]?newexten)\n";
							$routing_content .= "exten => $each[e],n,Macro(dial-failover,$forward_number,$each[d],$devices)\n";
							$routing_content .= "exten => $each[e],n,GotoIf(1?out)\n";
							$routing_content .= "exten => $each[e],n(newexten),Macro(dial-failover,$forward_number,\${NEWEXTEN},$devices)\n";
							$routing_content .= "exten => $each[e],n(out),NoOp\n";
						} else {
							//gy add for incoming call wait
							if($to_flag == 1) {
								$to_channels_num = count($to_channels_array);
								if ($to_channels_num <= 1) {
									//to_channel is only one
									if(!strstr($_routing_name, 'grp-')){
										$routing_content .= "exten => $each[e],n,Wait(1)\n";
									}
									
									// group policy
									if ($grp_policy_name != ''){
									
										if(!strstr($_routing_name, 'grp-')){
											$routing_content .= "exten => $each[e],n,Gotoif(\$[\"\${".$grp_policy_name."}\"=\"0\"]?tryagain)\n";
											$routing_content .= "exten => $each[e],n,Gotoif(\$[\"\${".$grp_policy_name."}\"=0]?tryagain)\n";
											$routing_content .= "exten => $each[e],n,Macro(dial-failover,$forward_number,$each[d],RET,\${".$grp_policy_name."})\n";
										}else if(strstr($_routing_name, 'grp-') && $policy_code != ''){
											$routing_content .= "exten => $each[e],n,Macro(dial-failover,$forward_number,$each[d],RET,extra/$policy_code,0,0)\n";
										}
							
										//
										$routing_content .= "exten => $each[e],n,Gotoif(\$[\"\${RET}\"=\"congestion\"]?tryagain:hangup)\n";
									
										$routing_content .= "exten => $each[e],n(tryagain),Progress()\n";

										if(!strstr($_routing_name, 'grp-')){
											foreach($grp_policy_act_ary as $act) {
												$routing_content .= "exten => $each[e],n,$act\n";
											}
											$routing_content .= "exten => $each[e],n,Gotoif(\$[\"\${".$grp_policy_name."}\"=\"0\"]?hangup)\n";
											$routing_content .= "exten => $each[e],n,Gotoif(\$[\"\${".$grp_policy_name."}\"=0]?hangup)\n";
										}
									}
									
									if(!strstr($_routing_name, 'grp-')){
										$routing_content .= "exten => $each[e],n,Macro(dial-failover,$forward_number,$each[d],RET,$devices)\n";
									}else if(strstr($_routing_name, 'grp-') && $policy_code != ''){
										$routing_content .= "exten => $each[e],n,Macro(dial-failover,$forward_number,$each[d],RET,extra/$policy_code,0,0)\n";
									}
								} else {
									$_routing_name = $routing['to_channel'];
									$temp = explode(',',$_routing_name);
									$temp = explode('-',$temp[0],2);
									$real_routing_name = $temp[1];
									$group_info = get_group_info($real_routing_name);
						
									$group_policy = $group_info['policy'];
									if($group_policy == 'ascending'){
										$policy_code = 'g'.$group_info['order'];
									}else if($group_policy == 'descending'){
										$policy_code = 'G'.$group_info['order'];
									}else if($group_policy == 'roundrobin'){
										$policy_code = 'r'.$group_info['order'];
									}else if($group_policy == 'reverseroundrobin'){
										$policy_code = 'R'.$group_info['order'];
									}else{
										$policy_code = '';
									}
									
									// to_channel numbers include failover configurations
									$device_write = '';
									foreach($to_channels_array as $key => $device) {
										if(!preg_match('/POLICY/i', $device)){ // remove the group_policy when handling the failover
											$device_write .= $device;
											if ($key != ($to_channels_num -1)) {
												$device_write .= ',';
											}
										}
									}
									if(!strstr($_routing_name, 'grp-')){
										$routing_content .= "exten => $each[e],n,Wait(1)\n";
									}
									
									// grp process
									if ($grp_policy_name != ''){
										if(!strstr($_routing_name, 'grp-')){
											$routing_content .= "exten => $each[e],n,Gotoif(\$[\"\${".$grp_policy_name."}\"=\"0\"]?tryagain)\n";
											$routing_content .= "exten => $each[e],n,Gotoif(\$[\"\${".$grp_policy_name."}\"=0]?tryagain)\n";
											$routing_content .= "exten => $each[e],n,Macro(dial-failover,$forward_number,$each[d],RET,\${".$grp_policy_name."})\n";
										}else if(strstr($_routing_name, 'grp-') && $policy_code != ''){
											$routing_content .= "exten => $each[e],n,Macro(dial-failover,$forward_number,$each[d],RET,extra/$policy_code,0,0)\n";
										}
					
										$routing_content .= "exten => $each[e],n,Gotoif(\$[\"\${RET}\"=\"congestion\"]?tryagain:hangup)\n";
										$routing_content .= "exten => $each[e],n(tryagain),Progress()\n";
									}
									
									$routing_content .= "exten => $each[e],n,Macro(dial-failover,$forward_number,$each[d],RET,$device_write)\n";
								}
							}
							if ($to_flag != 1){
								if ($custom_context != '' && $to_channel == 'custom') {
									$routing_content .= "exten => $each[e],n,Macro($custom_context)\n";	
								} else {
									$routing_content .= "exten => $each[e],n(dial-failover),Macro(dial-failover,$forward_number,$each[d],RET,$devices)\n";
								}
							}
						}
						$routing_content .= "exten => $each[e],n(hangup),ChannelHangup(\"\${CDR_TOCHAN}\")\n";
						if($routing_blacklist_sw == 'on'){
							if($from_channel_info['type'] == 'gsm'){
								$routing_content .= "exten => $each[e],n(blacklisted),Hangup()\n";
							} else {
								$routing_content .= "exten => $each[e],n,Hangup()\n";
							}
						} else {
							$routing_content .= "exten => $each[e],n,Hangup()\n";
						}
						//$routing_content .= "exten => $each[e],n,Goto(nocdr,s,1)\n";
					}
					
					
				}
				unset($exten);

				//$write_array[$routing_context] = $routing_content;
			
				if ( $disa_sw == 'on' ) {
					if ( $second_dial_sw == 'on' ) {
						
						$write_array[$routing_context] = $routing_content_head;
						$routing_content .= "exten => h,1,ChannelHangup(\"\${CDR_TOCHAN}\")\n";
						$write_array[$routing_context_disa] = $routing_content;
					} else {

						$write_array[$routing_context] = $routing_content;
					}			
				} else {
					$write_array[$routing_context] = $routing_content;
				}
				///////////////////////////////////////////////////////////////////////

				// [from_channel] context
				///////////////////////////////////////////////////////////////////////
				$channel_context = $from_channel_info['context'];
				if($channel_context) {
					$from_channel_content = '';
					if(isset($time_pattern) && is_array($time_pattern)) {
						foreach($time_pattern as $tp) {
							$time='*';
							$week='*';
							$day='*';
							$month='*';
							if($tp['stime'] != '' && $tp['stime'] != ':' && $tp['etime'] != '' && $tp['etime'] != ':')  $time = $tp['stime'].'-'.$tp['etime'];
							if($tp['sweek'] != '' && $tp['sweek'] != '')  $week = $tp['sweek'].'-'.$tp['eweek'];
							if($tp['sday'] != '' && $tp['sday'] != '')  $day = $tp['sday'].'-'.$tp['eday'];
							if($tp['smonth'] != '' && $tp['smonth'] != '')  $month = $tp['smonth'].'-'.$tp['emonth'];

							$gotoiftime_str = ",$time,$week,$day,$month";
							$from_channel_content .= "include => $routing_context$gotoiftime_str\n";
						}
					} else {
						$from_channel_content .= "include => $routing_context\n";
					}

					if(array_key_exists($channel_context,$write_array)) { // [from_channel] context already writed
						$write_array[$channel_context] .= $from_channel_content;
					} else {  //First write
						$write_array[$channel_context] = $from_channel_content;
					}
				}
				///////////////////////////////////////////////////////////////////////
			}
		}
	}


	// Set All Channel context (Defaults)
	//////////////////////////////////////////////////////////////////////////////////
	// GSM
	$sms_info = get_sms_info();

	
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk');
	if (!$setok) {
		echo $aql->get_error();
		return;
	}
	$sms_conf_path = '/etc/asterisk/extra-global.conf';
	$hlock = lock_file($sms_conf_path);
	if(!$aql->open_config_file($sms_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	$exist_array = $aql->query("select * from extra-global.conf");
	if(isset($exist_array['channels']['processsms'])) {
		$processsms=$exist_array['channels']['processsms'];
	}else{
		$processsms='no';
	}
	
	
	// [gsm-12]
	// exten => s,1,NoOp(Nothing to do, Not setting out channel)
	// exten => s,n,Hangup()
	// exten => sms,1,NoOp(SMS In)
	$sms_to_http_url = '';
	$board = '';
	if(is_true($sms_info['sms_to_http']['enable'])){
		if(isset($sms_info['sms_to_http']['url']) && $sms_info['sms_to_http']['url'] != ''){
			$board = get_slotnum();
			$sms_to_http_url = $sms_info['sms_to_http']['url'];
		}
	}
	//Getting all modules parameters from gw_gsm.conf
	$gsm_modules_array = get_gsm_module_info();
	for($i=1; $i<=$__GSM_SUM__; $i++) {
		$key = $__GSM_HEAD__.$i;
		$port_name = $gsm_modules_array[$i]['name'];
		if(!array_key_exists($key,$write_array)) {
			$write_array[$key] = '';
			if($internalcall_sw) {
				$write_array[$key] .= "exten => s,1,Answer(\${RAND(1000,5000)},nocdr)\n";
				$write_array[$key] .= "exten => s,n,Set(X=1)\n";
				$write_array[$key] .= "exten => s,n(loop),PlayBack(/sound/demo-instruct)\n";
				$write_array[$key] .= "exten => s,n,Wait(\${RAND(1,10)})\n";
				$write_array[$key] .= "exten => s,n,Set(X=\${INC(X)})\n";
				$write_array[$key] .= "exten => s,n,GotoIf(\$[\${X}<=200]?loop)\n";
				$write_array[$key] .= "exten => s,n,Hangup()\n";
			} else {
				$write_array[$key] .= "exten => s,1,NoOp(Nothing to do, Not setting out channel)\n";
				$write_array[$key] .= "exten => s,n,Hangup()\n";
			}
		} else {
			if($cdr_sw) {
				$temp = explode('-',$write_array[$key]);
				$_routing_name = $temp[1];
				$to_channel = $routings_info[$_routing_name]['to_channel'];
				
				$write_array[$key] .= "exten => h,1,Gotoif(\$[\"\${CDR_TOCHAN}\"=\"\"]?tohangup)\n";
				if(strstr($to_channel,'grp-')){
					$temp = explode('-',$to_channel,2);
					$real_routing_name = $temp[1];
					$group_info = get_group_info($real_routing_name);
					
					if($group_info['type'] == 'gsm'){
						$write_array[$key] .= "exten => h,n,Set(tmpdst1=\${CDR(dstchannel):6})\n";
						$write_array[$key] .= "exten => h,n,Set(tmpdst2=\${CUT(tmpdst1,-,-1)})\n";
						$write_array[$key] .= "exten => h,n,Set(CDR_TOCHAN=gsm-\$[\$[\${tmpdst2} + 1]/2])\n";
					}else if($group_info['type'] == 'sip'){
						$write_array[$key] .= "exten => h,n,Set(tmpdst1=\${CDR(dstchannel):4})\n";
						$write_array[$key] .= "exten => h,n,Set(tmpdst2=\${CUT(tmpdst1,-,-1)})\n";
						$write_array[$key] .= "exten => h,n,Set(CDR_TOCHAN=sip-\${tmpdst2})\n";
					}
				}
				$write_array[$key] .= "exten => h,n,WriteCDR(\"\${CDR(src)}\",\"\${CDR_CALLEEID}\",\"$__GSM_HEAD__$i\",\"\${CDR_TOCHAN}\",\"\${CDR(start)}\",\"\${CDR(billsec)}\",\"\${CDR(disposition)}\")\n";
				$write_array[$key] .= "exten => h,n(tohangup),ChannelHangup(\"\${CDR_TOCHAN}\")\n";
				$write_array[$key] .= "exten => h,n,ChannelHangup(\"$__GSM_HEAD__$i\")\n";
			} else {
				$write_array[$key] .= "exten => h,1,ChannelHangup(\"\${CDR_TOCHAN}\")\n";
				$write_array[$key] .= "exten => h,n,ChannelHangup(\"$__GSM_HEAD__$i\")\n";
			}
		}

		// sms to http
		$write_array[$key] .= "exten => sms,1,NoOp(SMS In)\n";
		if($sms_to_http_url != ''){
			$board_head = $__MODULE_HEAD_ARRAY__[1][$i];
			$write_array[$key] .= <<<EOF
exten => sms,n,NoOp(SMS CURL)
exten => sms,n,Set(CURLOPT(conntimeout)=5)
exten => sms,n,Set(CURLOPT(httptimeout)=5)
exten => sms,n,Set(CURLOPT(ssl_verifypeer)=0)
exten => sms,n,Set(phonenumber=\${URIENCODE(\${SMSSRC})})
;exten => sms,n,Set(port=\${URIENCODE($__GSM_HEAD__$board.$i)})
;exten => sms,n,Set(port=\${URIENCODE($board_head$board.$i)})
exten => sms,n,Set(port=\${URIENCODE($i)})
exten => sms,n,Set(portname=\${URIENCODE($port_name)})
exten => sms,n,Set(message=\${URIENCODE(\${SMSORITXT})})
exten => sms,n,Set(time=\${URIENCODE(\${SMSTIME})})
exten => sms,n,Set(imsi=\${URIENCODE(\${SIMIMSI})})
;exten => sms,n,Set(RET=\${CURL($sms_to_http_url)})
exten => sms,n,System(wget --no-check-certificate \"$sms_to_http_url\" -O - > /dev/null 2>&1 &)

EOF;
//EOF
		}
		
//		if(is_true($sms_info['local_store']['enable'])){
		if (($processsms=="yes")) {		
			$write_array[$key] .= "exten => sms,n,System(cd /my_tools/lua/sms_receive/ && lua sms_receive.lua \"$i\" \"\${SMSSRC}\" \"\${SMSTIME}\" \"\${SMSTXT}\" \"\${SMSC}\" > /dev/null 2>&1 &)\n";
//			$write_array[$key] .= "exten => sms,n,System(/my_tools/process_sms \"$i\" \"\${SMSSRC}\" \"\${SMSTIME}\" \"\${SMSTXT}\" > /dev/null 2>&1 &)\n";
		}
	//	if (($processsms=="yes") && is_true($sms_info['local_store']['enable'])){		
		if (($processsms=="yes")) {		
			$write_array[$key] .= "exten => sms,n,System(cd /my_tools/lua/sms_routing && lua dialplan2redis.lua \"$i\" \"\${SMSTXT}\" > /dev/null 2>&1 &)\n";
			                                              
		}
	}
	
	//IAX
	$alliaxs = get_all_iaxs();

	if($alliaxs) {
		foreach($alliaxs as $iax) {
			if(isset($iax['context']) && $iax['context']) {
				$key = $iax['context'];
				if(!array_key_exists($key,$write_array)) {
					if(array_key_exists('iaxdefault',$write_array)) {
						$write_array[$key] = "include => sipdefault\n";
					} else {
						$write_array[$key] = "include => nothingtodo\n";
					}
				} else {
					$hangup_start = '1';
					if($cdr_sw) {
						$temp = explode('-',$write_array[$key]);
						$_routing_name = $temp[1];
						$to_channel = $routings_info[$_routing_name]['to_channel'];
						
						$cdr_from = $iax['endpoint_name'];
						$write_array[$key] .= "exten => h,${hangup_start},Gotoif(\$[\"\${CDR_TOCHAN}\"=\"\"]?tohangup)\n";
						if(strstr($to_channel,'grp-')){
							$temp = explode('-',$to_channel,2);
							$real_routing_name = $temp[1];
							$group_info = get_group_info($real_routing_name);
							
							if($group_info['type'] == 'gsm'){
								$write_array[$key] .= "exten => h,n,Set(tmpdst1=\${CDR(dstchannel):6})\n";
								$write_array[$key] .= "exten => h,n,Set(tmpdst2=\${CUT(tmpdst1,-,-1)})\n";
								$write_array[$key] .= "exten => h,n,Set(CDR_TOCHAN=gsm-\$[\$[\${tmpdst2} + 1]/2])\n";
							}else if($group_info['type'] == 'sip'){
								$write_array[$key] .= "exten => h,n,Set(tmpdst1=\${CDR(dstchannel):4})\n";
								$write_array[$key] .= "exten => h,n,Set(tmpdst2=\${CUT(tmpdst1,-,-1)})\n";
								$write_array[$key] .= "exten => h,n,Set(CDR_TOCHAN=sip-\${tmpdst2})\n";
							}
						}
						$write_array[$key] .= "exten => h,n,WriteCDR(\"\${CDR(src)}\",\"\${CDR_CALLEEID}\",\"$cdr_from\",\"\${CDR_TOCHAN}\",\"\${CDR(start)}\",\"\${CDR(billsec)}\",\"\${CDR(disposition)}\")\n";
						$write_array[$key] .= "exten => h,n(tohangup),ChannelHangup(\"\${CDR_TOCHAN}\")\n";
					} else {
						$write_array[$key] .= "exten => h,${hangup_start},ChannelHangup(\"\${CDR_TOCHAN}\")\n";
					}
				}				
			}
		}
	
	}
	
	// SIP
	$internal_sip_call = get_internal_sip_call();
	$allsips = get_all_sips();
	//print_r($allsips);
	$write_array['nothingtodo'] = "exten => _[*#+0-9].,1,NoOp(Nothing to do, Not setting out channel)\n";
//	$write_array['nothingtodo'] .= "exten => _[*#+0-9].,n,Set(CDR_CALLEEID=\${EXTEN})\n";
	$write_array['nothingtodo'] .= "exten => _[*#+0-9].,n,Hangup($hangupcausecode)\n\n";

	if($allsips) {
		foreach($allsips as $sip) {
			if(isset($sip['context']) && $sip['context']) {
				$info = get_channel_info($__SIP_HEAD__.$sip['endpoint_name']);
				$key = $info['context'];
				if(!array_key_exists($key,$write_array)) {

					if(array_key_exists('sipdefault',$write_array)) {
						$write_array[$key] = "include => sipdefault\n";
					} else {
						$write_array[$key] = "include => nothingtodo\n";
					}
				} else {

					$hangup_start = '1';
					if($internalcall_sw) {
						$write_array[$key] .= "exten => h,1,IncraseOutCounter(\${OUTDEV})\n";
						$hangup_start = 'n';
					}

					if($cdr_sw) {
						$temp = explode('-',$write_array[$key]);
						$_routing_name = $temp[1];
						$to_channel = $routings_info[$_routing_name]['to_channel'];
						
						$cdr_from = $sip['endpoint_name'];
						$write_array[$key] .= "exten => h,${hangup_start},Gotoif(\$[\"\${CDR_TOCHAN}\"=\"\"]?tohangup)\n";
						if(strstr($to_channel,'grp-')){
							$temp = explode('-',$to_channel,2);
							$real_routing_name = $temp[1];
							$group_info = get_group_info($real_routing_name);
							
							if($group_info['type'] == 'gsm'){
								$write_array[$key] .= "exten => h,n,Set(tmpdst1=\${CDR(dstchannel):6})\n";
								$write_array[$key] .= "exten => h,n,Set(tmpdst2=\${CUT(tmpdst1,-,-1)})\n";
								$write_array[$key] .= "exten => h,n,Set(CDR_TOCHAN=gsm-\$[\$[\${tmpdst2} + 1]/2])\n";
							}else if($group_info['type'] == 'sip'){
								$write_array[$key] .= "exten => h,n,Set(tmpdst1=\${CDR(dstchannel):4})\n";
								$write_array[$key] .= "exten => h,n,Set(tmpdst2=\${CUT(tmpdst1,-,-1)})\n";
								$write_array[$key] .= "exten => h,n,Set(CDR_TOCHAN=sip-\${tmpdst2})\n";
							}
						}
						$write_array[$key] .= "exten => h,n,WriteCDR(\"\${CDR(src)}\",\"\${CDR_CALLEEID}\",\"$cdr_from\",\"\${CDR_TOCHAN}\",\"\${CDR(start)}\",\"\${CDR(billsec)}\",\"\${CDR(disposition)}\")\n";
						$write_array[$key] .= "exten => h,n(tohangup),ChannelHangup(\"\${CDR_TOCHAN}\")\n";
						$write_array[$key] .= "exten => h,n,Hangup($hangupcausecode)\n";
					} else {
						$write_array[$key] .= "exten => h,${hangup_start},ChannelHangup(\"\${CDR_TOCHAN}\")\n";
						$write_array[$key] .= "exten => h,n,Hangup($hangupcausecode)\n";
					}
				}

				if($internal_sip_call) {
					if($sip['registration'] == 'server') {
						$write_array[$key] = "include => internalsip\n" . $write_array[$key];  //Must put on first line.
					}
				}
			}
		}
		if($internal_sip_call) {
			$prefix = $internal_sip_call['prefix'];
			$len = strlen($prefix);
			$each = '_'.$prefix.'X.';
			$num = '${EXTEN:'.$len."}";
			$write_array['internalsip'] = "exten => $each,1,NoOp(SIP Internal Call\n";
 			$write_array['internalsip'] .= "exten => $each,n,Set(CDR_CALLEEID=$num)\n";
//			$write_array['internalsip'] .= "exten => $each,n,Set(CDR_CALLEEID=\${EXTEN})\n";
			$write_array['internalsip'] .= "exten => $each,n,Set(CDR_TOCHAN=$num)\n";
			$write_array['internalsip'] .= "exten => $each,n,Set(CDR_TOCHAN=\${SHELL(/my_tools/get_sipendpointname \"\${CDR_TOCHAN}\")})\n";
			$write_array['internalsip'] .= "exten => $each,n,Dial(SIP/$num)\n";
			$write_array['internalsip'] .= "exten => $each,n,Hangup($hangupcausecode)\n\n";
		}
	}

	// Board
	$brd_info = get_cluster_info();
	for($b=2; $b<=$__BRD_SUM__; $b++) {
		if(isset($brd_info[$__BRD_HEAD__.$b.'_ip']) && $brd_info[$__BRD_HEAD__.$b.'_ip'] != '' ) {
			for($i=1; $i<=$__GSM_SUM__; $i++) {
				$key = $__BRD_HEAD__.$b.'-'.$__GSM_HEAD__.$i;
				if(!array_key_exists($key,$write_array)) {
					$write_array[$key] = '';
					if($internalcall_sw) {
						$write_array[$key] .= "exten => _[*#+0-9].,1,Answer(\${RAND(1000,5000)},nocdr)\n";
						$write_array[$key] .= "exten => _[*#+0-9].,n,Set(X=1)\n";
						$write_array[$key] .= "exten => _[*#+0-9].,n(loop),PlayBack(/sound/demo-instruct)\n";
						$write_array[$key] .= "exten => _[*#+0-9].,n,Wait(\${RAND(1,10)})\n";
						$write_array[$key] .= "exten => _[*#+0-9].,n,Set(X=\${INC(X)})\n";
						$write_array[$key] .= "exten => _[*#+0-9].,n,GotoIf(\$[\${X}<=200]?loop)\n";
						$write_array[$key] .= "exten => _[*#+0-9].,n,Hangup()\n";
					} else {
						$write_array[$key] .= "exten => _[*#+0-9].,1,NoOp(Nothing to do, Not setting out channel)\n";
						$write_array[$key] .= "exten => _[*#+0-9].,n,Hangup()\n";
					}
				} else {
					if($cdr_sw) {
						$write_array[$key] .= "exten => h,1,Gotoif(\$[\"\${CDR_TOCHAN}\"=\"\"]?tohangup)\n";
						$write_array[$key] .= "exten => h,n,WriteCDR(\"\${CALLERID(name)}\",\"\${CDR_CALLEEID}\",\"$key\",\"\${CDR_TOCHAN}\",\"\${CDR(start)}\",\"\${CDR(billsec)}\",\"\${CDR(disposition)}\")\n";
						$write_array[$key] .= "exten => h,n(tohangup),ChannelHangup(\"\${CDR_TOCHAN}\")\n";
						$write_array[$key] .= "exten => h,n,ChannelHangup(\"$key\")\n";
					} else {
						$write_array[$key] .= "exten => h,1,ChannelHangup(\"\${CDR_TOCHAN}\")\n";
						$write_array[$key] .= "exten => h,n,ChannelHangup(\"$key\")\n";
					}
				}

//				$write_array[$key] .= "exten => smsfromsip,1,NoOp(Do SMS process)\n";
//				$write_array[$key] .= "exten => smsfromsip,n,Set(SMSSRC=\${SIP_HEADER(SMSSRC)})\n";
//				$write_array[$key] .= "exten => smsfromsip,n,Set(SMSTIME=\${SIP_HEADER(SMSTIME)})\n";
//				$write_array[$key] .= "exten => smsfromsip,n,Set(SMSTXT=\${SIP_HEADER(SMSTXT)})\n";
//				$write_array[$key] .= "exten => smsfromsip,n,System(/my_tools/process_sms \"$key\" \"\${SMSSRC}\" \"\${SMSTIME}\" \"\${SMSTXT}\")\n";

//				$write_array[$key] .= "exten => smsfromsip,n,Set(SMSTXT=\${SIP_HEADER(SMSTXT)})\n";
//				$write_array[$key] .= "exten => smsfromsip,n,Set(SMSTXT=\${REPLACE(SMSTXT,\\\\030,\\r)})\n";
//				$write_array[$key] .= "exten => smsfromsip,n,Set(SMSTXT=\${REPLACE(SMSTXT,\\\\031,\\n)})\n";
//				$write_array[$key] .= "exten => smsfromsip,n,System(/my_tools/process_sms \"$key\" \"\${SIP_HEADER(SMSSRC)}\" \"\${SIP_HEADER(SMSTIME)}\" \"\${SMSTXT}\")\n";
//				$write_array[$key] .= "exten => smsfromsip,n,Goto(nocdr,s,1)\n";
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////////

/*
mnp_enable=on
mnp_url=https://s1.bichara.com.br:8181/chkporta.php?user=832700&pwd=sfhawz826&tn=8388166902
mnp_timeout=2
*/
	//For all sip endpoint inbound
	$match_rule = "[*#+0-9].";
	
	if(is_true($mnpinfo['enable']) && $mnpinfo['manipulation'] != 'before' ) {

		$mnpinfo['url'] = str_replace("\${num}","\${EXTEN}",$mnpinfo['url']);

		$sipinbound_content = <<<EOF
exten => _$match_rule,1,NoOp(SIP Inbound)
exten => _$match_rule,n,Set(CURLOPT(conntimeout)=$mnpinfo[timeout])
exten => _$match_rule,n,Set(CURLOPT(httptimeout)=$mnpinfo[timeout])
exten => _$match_rule,n,Set(CURLOPT(ssl_verifypeer)=0)
exten => _$match_rule,n,Set(NEWEXTEN=\${CURL($mnpinfo[url])})
exten => _$match_rule,n,GotoIf(\$["\${NEWEXTEN}" = ""]?oldexten)
exten => _$match_rule,n,GotoIf(\${DIALPLAN_EXISTS(\${SIPROUTE},\${NEWEXTEN},1)}?:nocdr)
exten => _$match_rule,n,Goto(\${SIPROUTE},\${NEWEXTEN},1)
exten => _$match_rule,n(oldexten),GotoIf(\${DIALPLAN_EXISTS(\${SIPROUTE},\${EXTEN},1)}?:nocdr)
exten => _$match_rule,n,Goto(\${SIPROUTE},\${EXTEN},1)
exten => _$match_rule,n(nocdr),Goto(nocdr,s,1)
EOF;

	$iaxinbound_content = <<<EOF
exten => _[*#+0-9].,1,NoOp(IAX Inbound)
exten => _[*#+0-9].,n,Set(CURLOPT(conntimeout)=$mnpinfo[timeout])
exten => _[*#+0-9].,n,Set(CURLOPT(httptimeout)=$mnpinfo[timeout])
exten => _[*#+0-9].,n,Set(CURLOPT(ssl_verifypeer)=0)
exten => _[*#+0-9].,n,Set(NEWEXTEN=\${CURL($mnpinfo[url])})
exten => _[*#+0-9].,n,GotoIf(\$["\${NEWEXTEN}" = ""]?oldexten)
exten => _[*#+0-9].,n,GotoIf(\${DIALPLAN_EXISTS(\${IAXROUTE},\${NEWEXTEN},1)}?:nocdr)
exten => _[*#+0-9].,n,Goto(\${IAXROUTE},\${NEWEXTEN},1)
exten => _[*#+0-9].,n(oldexten),GotoIf(\${DIALPLAN_EXISTS(\${IAXROUTE},\${EXTEN},1)}?:nocdr)
exten => _[*#+0-9].,n,Goto(\${IAXROUTE},\${EXTEN},1)
exten => _[*#+0-9].,n(nocdr),Goto(nocdr,s,1)
EOF;

//EOF
	} else {

	if(!file_exists('/etc/asterisk/gw_general.conf')){
		exec("touch /etc/asterisk/gw_general.conf");
	}
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_general.conf');
	$exist_general_section = $aql->query("select * from gw_general.conf");
	unlock_file($hlock);
	
	$caller_id_1_str = '';
	
	$caller_id_1 = $exist_general_section['general']['caller_id_1'];
	if($caller_id_1 == 'From'){
		if($exist_general_section['general']['caller_id_2'] == 'Name'){
			$caller_id_1_str = "\nexten => _$match_rule,n,Set(CALLERID(num)=\${CALLERID(name)})";
		}
	}else if($caller_id_1 == 'To'){
		if($exist_general_section['general']['caller_id_2'] == 'Name'){
			$caller_id_1_str = "\nexten => _$match_rule,n,Set(CALLERID(num)=\${CUT(SIP_HEADER($caller_id_1),\\\",2)})";
		}else if($exist_general_section['general']['caller_id_2'] == 'Number'){
			$caller_id_1_str = "\nexten => _$match_rule,n,Set(CALLERID(num)=\${CUT(CUT(SIP_HEADER($caller_id_1),@,1),:,2)})";
		}
	}else if($caller_id_1 == 'EXTEN'){
		$caller_id_1_str = "\nexten => _$match_rule,n,Set(CALLERID(num)=\${EXTEN})";
	}
	
	if($exist_general_section['general']['callee_id_1']=='To' || $exist_general_section['general']['callee_id_1']=='From'){
		$callee_id_1 = $exist_general_section['general']['callee_id_1'];
		if($exist_general_section['general']['callee_id_2']=='Number'){
			$callee_id_2_str = "exten => _$match_rule,n,Set(DID=\${CUT(CUT(SIP_HEADER($callee_id_1),@,1),:,2)})";
		}else{
			$callee_id_2_str = "exten => _$match_rule,n,Set(DID=\${CUT(SIP_HEADER($callee_id_1),\\\",2)})";
		}
	
	$sipinbound_content = <<<EOF
exten => _$match_rule,1,NoOp(SIP Inbound)$caller_id_1_str
$callee_id_2_str
exten => _$match_rule,n,GotoIf(\${DIALPLAN_EXISTS(\${SIPROUTE},\${DID},1)}?:nocdr)
exten => _$match_rule,n,Goto(\${SIPROUTE},\${DID},1)
exten => _$match_rule,n(nocdr),Goto(nocdr,s,1)
EOF;
	}else{

	$sipinbound_content = <<<EOF
exten => _$match_rule,1,NoOp(SIP Inbound)$caller_id_1_str
exten => _$match_rule,n,GotoIf(\${DIALPLAN_EXISTS(\${SIPROUTE},\${EXTEN},1)}?:nocdr)
exten => _$match_rule,n,Goto(\${SIPROUTE},\${EXTEN},1)
exten => _$match_rule,n(nocdr),Goto(nocdr,s,1)
EOF;
	}
	
	if($exist_general_section['general']['sipinbound'] == 'on'){
		$sipinbound_content = str_replace('[*#+0-9]','[*#+0-9A-Za-z]',$sipinbound_content);
	}
	
	$iaxinbound_content = <<<EOF
exten => _[*#+0-9].,1,NoOp(IAX Inbound)
exten => _[*#+0-9].,n,GotoIf(\${DIALPLAN_EXISTS(\${IAXROUTE},\${EXTEN},1)}?:nocdr)
exten => _[*#+0-9].,n,Goto(\${IAXROUTE},\${EXTEN},1)
exten => _[*#+0-9].,n(nocdr),Goto(nocdr,s,1)
EOF;
//EOF
	}
	$write_array['sipinbound'] = $sipinbound_content;
	$write_array['iaxinbound'] = $iaxinbound_content;
	


	//For No CDR process.
	//gy add for channelhangup
	$write_array['nocdr'] = "exten => s,1,ChannelHangup(\"\${CDR_TOCHAN}\")\n";
	$write_array['nocdr'] .= "exten => s,n,Hangup($hangupcausecode)\n";


	//For globals variable
	$write_array['globals'] = "SIPROUTE=sipdefault\n";


	if(array_key_exists('sipdefault',$write_array)) {
		$hangup_start = '1';
		if($internalcall_sw) {
			$write_array['sipdefault'] .= "exten => h,1,IncraseOutCounter(\${OUTDEV})\n";
			$hangup_start = 'n';
		}

		//For Any SIP Write CDR
		if($cdr_sw) {
			$write_array['sipdefault'] .= "exten => h,${hangup_start},Gotoif(\$[\"\${CDR_TOCHAN}\"=\"\"]?tohangup)\n";
			$write_array['sipdefault'] .= "exten => h,n,WriteCDR(\"\${CDR(src)}\",\"\${CDR_CALLEEID}\",\"\${CDR(src)}\",\"\${CDR_TOCHAN}\",\"\${CDR(start)}\",\"\${CDR(billsec)}\",\"\${CDR(disposition)}\")\n";
			$write_array['sipdefault'] .= "exten => h,n(tohangup),ChannelHangup(\"\${CDR_TOCHAN}\")\n";
		} else {
			$write_array['sipdefault'] .= "exten => h,1,ChannelHangup(\"\${CDR_TOCHAN}\")\n";
		}
	}


	//For GSM Auto Internal Call Dialplan, Play sound.
	if($internalcall_sw) {
		$write_array['internalcall'] = "exten => s,1,Set(X=1)\n";
		$write_array['internalcall'] .= "exten => s,n(loop),PlayBack(/sound/demo-instruct)\n";
		$write_array['internalcall'] .= "exten => s,n,Wait(\${RAND(1,10)})\n";
		$write_array['internalcall'] .= "exten => s,n,Set(X=\${INC(X)})\n";
		$write_array['internalcall'] .= "exten => s,n,GotoIf(\$[\${X}<=200]?loop)\n";
		$write_array['internalcall'] .= "exten => s,n,Hangup()\n";
	}

	//Write to extensions_routing.conf
	//////////////////////////////////////////////////////////////////////////////////
	$gsm_already_context = array();
	$cfg_file = '/etc/asterisk/extensions_routing.conf';
	$hlock = lock_file($cfg_file);
	$handle = fopen($cfg_file,"w");
	if($write_array) {
		foreach($write_array as $key => $value) {
			$str = "[$key]\n" . $value."\n\n";
			fwrite($handle,$str);
		}
	}
	fclose($handle);
	unlock_file($hlock);
	//////////////////////////////////////////////////////////////////////////////////
}

function del_all_register($type)
{
	$del_register_file = "$type"."_general.conf";
	$type_general_conf = "/etc/asterisk/".$del_register_file;

	if(!file_exists($type_general_conf))
		return;

	$hlock = lock_file($type_general_conf);
	$handle = fopen($type_general_conf,"r");
	$start = false;

	$write_str = '';
	while (!feof($handle)) {
		$line = fgets($handle);

		if(!$start) {
			if(preg_match("/^\s*\[\s*general\s*\]\s*/",$line)) {
				$start = true;
			}
		} else {
			if(preg_match("/^\s*\[\s*.*\s*\]\s*/",$line)) {
				$start = false;
			}
		}

		if($start) {
			if(preg_match("/^\s*register\s*=\s*.*/",$line)) {
				continue;
			}
		}

		$write_str .= $line;
	}
	fclose($handle);

	//Write
	if($write_str != '') {
		$handle = fopen($type_general_conf,"w");
		fwrite($handle,$write_str);
		fclose($handle);
	}
	unlock_file($hlock);
}

function set_sip_general_allowguest()
{
	$sip_general_conf = "/etc/asterisk/sip_general.conf";

	if(!file_exists($sip_general_conf))
		return;

	$hlock = lock_file($sip_general_conf);
	$handle = fopen($sip_general_conf,"r");
	$guest_set = false;
	$context_set =false;

	$write_str = '';
	while (!feof($handle)) {
		$line = fgets($handle);

		if(!$guest_set) {
			if(preg_match("/^\s*allowguest\s*=\s*.*/",$line)) {
				$write_str .= "allowguest=yes\n";
				$guest_set = true;
				continue;
			}
		} 

		if(!$context_set) {
			if(preg_match("/^\s*context\s{0,}=.*/",$line)) {
				$write_str .= "context=sipinbound\n";
				$context_set = true;
				continue;
			}
		}

		$write_str .= $line;
	}
	fclose($handle);

	if(!$guest_set) {
		$write_str .= "allowguest=yes\n";
	}

	if(!$context_set) {
		$write_str .= "context=sipinbound\n";
	}

	//Write
	if($write_str != '') {
		$handle = fopen($sip_general_conf,"w");
		fwrite($handle,$write_str);
		fclose($handle);
	}
	unlock_file($hlock);
}

function save_endpoints_to_sips()
{
/* /etc/asterisk/gw_endpoints.conf */
// [endpoint_name]
// order = 1,2,3,4,5....    //Must set
// username = 
// registration =  
// allow_anonymous = yes|no
// register =    like sip.conf [general]
// .......       like all sip.conf

	global $__SIP_HEAD__;

	$cfg_file = '/etc/asterisk/sip_endpoints.conf';

	del_all_register('sip');
	$all_sips = get_all_sips();

	if($all_sips) {
		$write_str='';
		foreach($all_sips as $sip) {
			$username = $sip['username'];
			$context = $username;
				
			if(isset($sip['host'])) {
				$ip = trim($sip['host']);
				if($ip != '' && $ip != 'dynamic') {
					$context = $username.'-'.$ip;
				}

				if(isset($sip['allow_anonymous']) && $sip['allow_anonymous'] == 'yes')  {
					$context = 'anonymous-'.$ip;
				}
			}
			if(isset($sip['outboundproxy']) && trim($sip['outboundproxy']) != '' && isset($sip['enableoutboundtohost']) && trim($sip['enableoutboundtohost']) == 'yes'){
				$outboundproxy = explode(':', $sip['outboundproxy'], 2);
				$context = $username . "-" . $outboundproxy[0];
			}
			
			$write_str .= "[$context]\n";

			foreach($sip as $key=>$value) {
				if($key == 'allow') {
				    $allow = explode(',',$value);
					foreach($allow as $each) {
						if($each != '')
							$write_str .= "allow=$each\n";
					}
				} else if($key == 'disallow') {
					continue; 
				} else if($key =='context') {
					if(isset($sip['allow_anonymous']) && $sip['allow_anonymous'] == 'yes')  {
						//$write_str .= "context=".$__SIP_HEAD__."anonymous-$context\n"; 
						//Freedom Modify for mnp 2014-01-16 13:23
						//$write_str .= "context=".$__SIP_HEAD__."$context\n";
						$write_str .= "context=sipinbound\n";
						$write_str .= "setvar=SIPROUTE=".$__SIP_HEAD__."$context\n";
					} else {
						//Freedom Modify for mnp 2014-01-16 13:23
						//$write_str .= "context=".$__SIP_HEAD__."$context\n"; 
						$write_str .= "context=sipinbound\n";
						$write_str .= "setvar=SIPROUTE=".$__SIP_HEAD__."$context\n";
					}
				} else if($key == 'register') {
					$register[] = $value;
				} else if($key == 'order') {
					//Not do anythings
				} else if($key == 'registration') {
					//Not do anythings
				} else if($key == 'register_extension') {
				    //Not do anythings
				} else if($key == 'allow_anonymous') {
					//Not do anythings
				} else {
					$write_str .= "$key=$value\n";
				}
			}
			$write_str .= "\n";
		}
		
		

		$hlock = lock_file($cfg_file);
		$handle = fopen($cfg_file,"w");
		fwrite($handle, $write_str);
		fclose($handle);
		unlock_file($hlock);

		if(isset($register)) {
			$aql = new aql();
			$aql->set('basedir','/etc/asterisk');
			$hlock = lock_file('/etc/asterisk/sip_general.conf');
			$exist_general_section = $aql->query("select * from sip_general.conf");
			if(!isset($exist_general_section['general'])) {  // if not exist 'general' section
				$aql->assign_addsection('general','');
				$aql->save_config_file('sip_general.conf');
			}
			foreach($register as $each) {
				$aql->assign_append('general','register','>'.$each);
			}
			$aql->save_config_file('sip_general.conf');
			unlock_file($hlock);
		}
	} else {
		$hlock = lock_file($cfg_file);
		fclose(fopen($cfg_file,"w"));
		unlock_file($hlock);
	}
}

function save_endpoints_to_iaxs()
{
/* /etc/asterisk/gw_endpoints.conf */
// [endpoint_name]
// username = 
// .......       like all sip.conf

	global $__IAX_HEAD__;

	$cfg_file = '/etc/asterisk/iax_endpoints.conf';

	del_all_register('iax');
	$all_iaxs = get_all_iaxs();

	if($all_iaxs) {
		$write_str='';
		foreach($all_iaxs as $iax) {
			$username = $iax['username'];
			$context = $username;
				
			if(isset($iax['host'])) {
				$ip = trim($iax['host']);
				//if($ip != '' && $ip != 'dynamic') {
				//	$context = $username.'-'.$ip;
				//}
				

				if(isset($iax['allow_anonymous']) && $iax['allow_anonymous'] == 'yes')  {
					$context = 'anonymous-'.$ip;
				}
			}
			
			$write_str .= "[$context]\n";

			foreach($iax as $key=>$value) {
				if($key == 'allow') {
				    $allow = explode(',',$value);
					foreach($allow as $each) {
						if($each != '')
							$write_str .= "allow=$each\n";
					}
				} else if($key == 'disallow') {
					continue; 
				} else if($key =='context') {
					if(isset($sip['allow_anonymous']) && $sip['allow_anonymous'] == 'yes')  {
						//$write_str .= "context=".$__IAX_HEAD__."anonymous-$context\n"; 
						//$write_str .= "context=".$__IAX_HEAD__."$context\n";
						$write_str .= "context=iaxinbound\n";
						$write_str .= "setvar=IAXROUTE=".$__IAX_HEAD__."$context\n";
					} else {
						//Freedom Modify for mnp 2014-01-16 13:23
						//$write_str .= "context=".$__IAX_HEAD__."$context\n"; 
						$write_str .= "context=iaxinbound\n";
						$write_str .= "setvar=IAXROUTE=".$__IAX_HEAD__."$context\n";
					}
				} else if($key == 'register') {
					$register[] = $value;
				} else if($key == 'order') {
					//Not do anythings
				} else if($key == 'registration') {
					//Not do anythings
				} else if($key == 'register_extension') {
				    //Not do anythings
				} else if($key == 'allow_anonymous') {
					//Not do anythings
				} else {
					$write_str .= "$key=$value\n";
				}
			}
			$write_str .= "\n";
		}
		
		

		$hlock = lock_file($cfg_file);
		$handle = fopen($cfg_file,"w");
		fwrite($handle, $write_str);
		fclose($handle);
		unlock_file($hlock);

		if(isset($register)) {
			$aql = new aql();
			$aql->set('basedir','/etc/asterisk');
			$hlock = lock_file('/etc/asterisk/iax_general.conf');
			$exist_general_section = $aql->query("select * from iax_general.conf");
			if(!isset($exist_general_section['general'])) {  // if not exist 'general' section
				$aql->assign_addsection('general','');
				$aql->save_config_file('iax_general.conf');
			}
			foreach($register as $each) {
				$aql->assign_append('general','register','>'.$each);
			}
			$aql->save_config_file('iax_general.conf');
			unlock_file($hlock);
		}
	} else {
		$hlock = lock_file($cfg_file);
		fclose(fopen($cfg_file,"w"));
		unlock_file($hlock);
	}
}


function save_gsm_to_extra_conf()
{
/*
Location: /etc/asterisk/extra-channels.conf

group=1
context=gsm-1
signalling = gsm
vol=70
mic=1
lte_txgain=11172
lte_txdgain=11172
lte_rxgain=60000
dacgain=-15
adcgain=-3
debugat=on
band=ALL_BAND
dialprefix=
smscodec=utf-8
;simulatedial=1
;hwdtmfdet=1
switchtype=SIMCOM_SIM840W
channel => 1
dl_step=1               //>0
dl_single_sw=off        //on/off
dl_single_limit=0
dl_total_sw=off         //on/off
dl_total_limit=0
dl_free_time=0          
dl_warning_time=0
dl_warning_num=
dl_warning_describe=
dl_auto_reset_sw=off    //on/off
dl_auto_reset_type=off  //day/week/month
dl_auto_reset_date=     //

group=1
...


*/


	global $__GSM_SUM__;

	$aql = new aql();

	$aql->set('basedir','/etc/asterisk');
	$hlock = lock_file('/etc/asterisk/gw_gsm.conf');
	$res = $aql->query("select * from gw_gsm.conf");
	unlock_file($hlock);

	$write_str = '';

	if(get_debugat_log_sw()) {
		$debugat='on';
	} else {
		$debugat='off';
	}

	$hwinfo = @parse_ini_file('/tmp/hwinfo',true);
	$mcu_module_info = @parse_ini_file('/tmp/mcu_module_map',true);

	//get group in channel
	$group_res = $aql->query("select * from gw_group.conf");
	$group_arr = [];
	foreach($group_res as $key => $val){
		$temp = explode(',',$group_res[$key]['members']);
		for($i=0;$i<count($temp);$i++){
			$tmp = explode('-',$temp[$i]);
			if(!array_key_exists($tmp[1],$group_arr)){
				$group_arr[$tmp[1]] = $group_res[$key]['order'].',';
			}else{
				$group_arr[$tmp[1]] .= $group_res[$key]['order'].',';
			}
		}
	}
	
	foreach($group_arr as $key => $val){
		$new_val = substr($val, 0, -1);
		$group_arr[$key] = $new_val;
	}
	
	for($channel=1; $channel <= $__GSM_SUM__; $channel++) {
		if(isset($group_arr[$channel])){
			$group_order = $group_arr[$channel];
		}else{
			$group_order = '';
		}
		
		if(isset($res[$channel]['vol'])) {
			$vol=trim($res[$channel]['vol']);
		} else {
			$vol='';
		}

		if(isset($res[$channel]['mic'])) {
			$mic=trim($res[$channel]['mic']);
		} else {
			$mic='';
		}
		
		if(isset($res[$channel]['lte_txgain'])) {
			$txgain=trim($res[$channel]['lte_txgain']);
		} else {
			$txgain='11172';
		}
		
		if(isset($res[$channel]['lte_txdgain'])) {
			$txdgain=trim($res[$channel]['lte_txdgain']);
		} else {
			$txdgain='11172';
		}
		
		if(isset($res[$channel]['lte_rxgain'])) {
			$rxgain=trim($res[$channel]['lte_rxgain']);
		} else {
			$rxgain='60000';
		}

		if(isset($res[$channel]['dacgain'])) {
			$dacgain=trim($res[$channel]['dacgain']);
		} else {
			$dacgain='';
		}

		if(isset($res[$channel]['adcgain'])) {
			$adcgain=trim($res[$channel]['adcgain']);
		} else {
			$adcgain='';
		}

		$pin = '';
		if(isset($res[$channel]['needpin'])) {
			if(trim($res[$channel]['needpin']) == "true") {
				if(isset($res[$channel]['pin'])) {
					$pin=trim($res[$channel]['pin']);
				}
			}
		}

		if(isset($res[$channel]['custom_start_at'])) {
			$custom_start_at=trim($res[$channel]['custom_start_at']);
		} else {
			$custom_start_at='';
		}

		if(isset($res[$channel]['gsm_ec'])) {
			$gsm_ec = trim($res[$channel]['gsm_ec']);
		} else {
			$gsm_ec = 'off';
		}

		if(isset($res[$channel]['anonymouscall'])) {
			$anonymouscall = trim($res[$channel]['anonymouscall']);
		} else {
			$anonymouscall = 'off';
		}

		if(isset($res[$channel]['call_waiting'])) {
			$call_waiting = trim($res[$channel]['call_waiting']);
		} else {
			$call_waiting = 'off';
		}
		
		if(isset($res[$channel]['band_mode'])) {
			$band_mode = trim($res[$channel]['band_mode']);
		} else {
			$band_mode = '';
		}

		if(isset($res[$channel]['band'])) {
			$band = trim($res[$channel]['band']);
		} else {
			$band = '';
		}

		if(isset($res[$channel]['dialprefix'])) {
			$dialprefix = trim($res[$channel]['dialprefix']);
		} else {
			$dialprefix = '';
		}
		
		if(isset($res[$channel]['seloperator'])) {
			$seloperator = trim($res[$channel]['seloperator']);
		} else {
			$seloperator = '';
		}

		if(isset($res[$channel]['brd_mcu_serial'])){
			$brd_mcu_serial = trim($res[$channel]['brd_mcu_serial']);
		}
		if(isset($res[$channel]['hw_port'])) {
			$hw_port = trim($res[$channel]['hw_port']);
		} else {
			$hw_port = $channel;
		}
		if(isset($res[$channel]['module_usb_com'])){
			$module_usb_com = trim($res[$channel]['module_usb_com']);
		}
		
		if(isset($res[$channel]['atd_clcc_flag'])){
			$atd_clcc_flag = trim($res[$channel]['atd_clcc_flag']);
		}else{
			$atd_clcc_flag = 'on';
		}
		if(isset($res[$channel]['atd_stk_flag'])){
			$atd_stk_flag = trim($res[$channel]['atd_stk_flag']);
		}else{
			$atd_stk_flag = 'off';
		}
		if(isset($res[$channel]['atd_stk_at'])){
			$atd_stk_at = trim($res[$channel]['atd_stk_at']);
		}else{
			$atd_stk_at = '';
		}
		if(isset($res[$channel]['csq_flag'])){
			$csq_flag = trim($res[$channel]['csq_flag']);
		} else {
			$csq_flag = 'on';
		}
		if(isset($res[$channel]['csq_timeout'])){
			$csq_timeout = trim($res[$channel]['csq_timeout']);
		} else {
			$csq_timeout = '10';
		}
		if(isset($res[$channel]['at_timeout'])){
			$at_timeout = trim($res[$channel]['at_timeout']);
		}else{
			$at_timeout = '';
		}
		if(isset($res[$channel]['calleridtype'])){
			$calleridtype = trim($res[$channel]['calleridtype']);
		}else{
			$calleridtype = 0;
		}
		
		$write_str .= "group=$group_order\n";
		$write_str .= "context=gsm-$channel\n";
		$write_str .= "signalling = gsm\n";
		$write_str .= "vol=$vol\n";
		$write_str .= "mic=$mic\n";
		$write_str .= "lte_txgain=$txgain\n";
		$write_str .= "lte_txdgain=$txdgain\n";
		$write_str .= "lte_rxgain=$rxgain\n";
		$write_str .= "dacgain=$dacgain\n";
		$write_str .= "adcgain=$adcgain\n";
		$write_str .= "debugat=$debugat\n";
		$write_str .= "gsm_ec=$gsm_ec\n";
		$write_str .= "anonymouscall=$anonymouscall\n";
		$write_str .= "call_waiting=$call_waiting\n";
		$write_str .= "band_mode=$band_mode\n";
		$write_str .= "band=$band\n";
		$write_str .= "dialprefix=$dialprefix\n";
		$write_str .= "seloperator=$seloperator\n";
		$write_str .= "smscodec=utf-8\n";
		$write_str .= ";simulatedial=1\n";
		$write_str .= ";hwdtmfdet=1\n";
		if($pin)$write_str .= "pin=$pin\n";
		$write_str .= "custom_start_at=$custom_start_at\n";
		$write_str .= "switchtype=SIMCOM_SIM840W\n";
		$write_str .= "atd_clcc_flag=$atd_clcc_flag\n";
		$write_str .= "atd_stk_flag=$atd_stk_flag\n";
		$write_str .= "atd_stk_at=$atd_stk_at\n";
		$write_str .= 'csq_flag='.$csq_flag."\n";
		$write_str .= 'csq_timeout='.$csq_timeout."\n";
		$write_str .= "at_timeout=$at_timeout\n";
		$write_str .= "calleridtype=$calleridtype\n";
		// board mcu usb serial	
		$write_str .= "brd_mcu_serial=$brd_mcu_serial\n";
		$write_str .= "hw_port=$hw_port\n";
		$write_str .= "module_usb_com=$module_usb_com\n";

		/* Dial Limit Settings 
		$dl_array = array(	'dl_step'=>'60',
					'dl_single_sw'=>'off',
					'dl_single_limit'=>'',
					'dl_total_sw'=>'off',
					'dl_total_limit'=>'',
					'dl_free_time'=>'0',
					'dl_warning_time'=>'0',
					'dl_warning_num'=>'',
					'dl_warning_describe'=>'',
					'dl_auto_reset_sw'=>'off',
					'dl_auto_reset_type'=>'day',
					'dl_auto_reset_date'=>''
				);

		foreach($dl_array as $key => $value){
			if(isset($res[$channel][$key])) {
				$write_str .= "$key=".trim($res[$channel][$key])."\n";
			} else {
				$write_str .= "$key=$value\n";
			}
		}*/
		
		/* BCCH Auto */
		$bcch_array = array(	
			'bcch_type'=>'default',
			'bcch_range'=>'',
			'bcch_fixed'=>'',
			'bcch_calls'=>'',
			'bcch_faileds'=>'',
			'bcch_asr'=>'',
			'bcch_incalls'=>'',
			'bcch_minrxl'=>''
		);
		foreach($bcch_array as $key => $value){
			if(isset($res[$channel][$key])) {
				$write_str .= "$key=".trim($res[$channel][$key])."\n";
			} else {
				$write_str .= "$key=$value\n";
			}
		}		

		$tmp = $channel*2 -1;
		$write_str .= "channel => $tmp\n\n";
	}

	$extra_conf = '/etc/asterisk/extra-channels.conf';
	$hlock = lock_file($extra_conf);
	$handle = fopen($extra_conf,"w");
	fwrite($handle, $write_str);
	fclose($handle);
	unlock_file($hlock);
}

function save_webserver_to_lighttpd()
{
/*
#/etc/asterisk/gw/lighttpdpassword
admin:admin
#/etc/asterisk/gw/lighttpd_https.conf
server.port = 80
*/
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk/gw');
	if (!$setok) {
		echo $aql->get_error();
		return false;
	}
    $cluster_info = get_cluster_info(); 
	$conf_path = '/etc/asterisk/gw/web_server.conf';
	if(!file_exists($conf_path)) {
		fclose(fopen($conf_path,"w"));
	}
	$hlock = lock_file($conf_path);
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	$exist_array = $aql->query("select * from web_server.conf");
	unlock_file($hlock);

	if(isset($exist_array['general']['username']) && isset($exist_array['general']['password'])){
		$username = trim($exist_array['general']['username']);
		$password = trim($exist_array['general']['password']);

		$file_path = "/etc/asterisk/gw/lighttpdpassword_digest";

		$lighttpdpassword_contents = trim(file_get_contents($file_path));
		$lighttpd_user = explode(":",$lighttpdpassword_contents);
		$http_realm = $lighttpd_user[1];
		$hash=md5($username.":".$http_realm.":".$password);

		file_put_contents($file_path, "$username:$http_realm:$hash", LOCK_EX);
	}

	/*
	if(isset($exist_array['general']['port'])){
		$port = trim($exist_array['general']['port']);
		if($port >= 1 && $port <= 65535){
			$file_path = "/etc/asterisk/gw/lighttpd_https.conf";
			$write_str = "server.port = $port\n";
			$flock = lock_file($file_path);
			file_put_contents($file_path, $write_str);
			unlock_file($flock);
		}
	}
	//*/
	$port = '80';
	if (isset($exist_array['general']['port'])) {
		$port = trim($exist_array['general']['port']);
	}
	if (isset($exist_array['general']['login_mode'])) {
		if ($exist_array['general']['login_mode'] == 'http_https') {
			if($port >= 1 && $port <= 65535){
				$file_path = "/etc/asterisk/gw/lighttpd_https.conf";
				$write_str = "server.port = $port\n";
				$write_str .= "\$SERVER[\"socket\"] == \"[::]:$port\" {}\n";
				
				if ($cluster_info['mode'] != "slave") {
					$write_str = "server.bind = \"0.0.0.0\"\n";
					$write_str .= "server.port = $port\n";
					$write_str .= "\$SERVER[\"socket\"] == \"[::]:$port\" {}\n";
					
					$write_str .= "$"."SERVER[\"socket\"] == \"0.0.0.0:443\" {\n";
					$write_str .= "  ssl.engine = \"enable\"\n";
					$write_str .= "	 ssl.pemfile = \"/etc/ssl/server.pem\"\n";
					$write_str .= "}\n";
					
					$write_str .= "\$SERVER[\"socket\"] == \"[::]:443\" {\n";
					$write_str .= "	 ssl.engine = \"enable\"\n";
					$write_str .= "  ssl.pemfile = \"/etc/ssl/server.pem\"\n";
					$write_str .= "}\n";
				} else {
					$write_str = "server.port = 80\n";
					$write_str .= "\$SERVER[\"socket\"] == \"[::]:80\" {}\n";
				}
				//print_rr($write_str);
				
				$flock = lock_file($file_path);
				file_put_contents($file_path, $write_str);
				unlock_file($flock);
			}			
		} else if(($exist_array['general']['login_mode']=="https")){
			$file_path = "/etc/asterisk/gw/lighttpd_https.conf";
			$write_str = "server.port=\"443\"\n";
			if ($cluster_info['mode'] != 'slave') {
				$write_str .= "$"."SERVER[\"socket\"] == \"0.0.0.0:443\" {\n";
				$write_str .= "  ssl.engine = \"enable\"\n";
				$write_str .= "	 ssl.pemfile = \"/etc/ssl/server.pem\"\n";
				$write_str .= "}\n";
				
				$write_str .= "\$SERVER[\"socket\"] == \"[::]:443\" {\n";
				$write_str .= "}\n";
				$write_str .= "ssl.engine = \"enable\"\n";
				$write_str .= "ssl.pemfile = \"/etc/ssl/server.pem\"\n";
			} else {
				$write_str = "server.port = $port\n";
				$write_str .= "\$SERVER[\"socket\"] == \"[::]:$port\" {}\n";
			}
			
			$flock = lock_file($file_path);
			//print_rr($write_str);
			file_put_contents($file_path, $write_str);
			unlock_file($flock);			
		}
	} else {
		$file_path = "/etc/asterisk/gw/lighttpd_https.conf";
		$write_str = "server.bind = \"0.0.0.0\"\n";
		$write_str .= "server.port = $port\n";
		$write_str .= "\$SERVER[\"socket\"] == \"[::]:$port\" {}\n";
		
		if ($cluster_info['mode'] != 'slave') {
			$write_str .= "$"."SERVER[\"socket\"] == \"0.0.0.0:443\" {\n";
			$write_str .= "ssl.engine = \"enable\"\n";
			$write_str .= "ssl.pemfile = \"/etc/ssl/server.pem\"\n";
			$write_str .= "}\n";
			
			$write_str .= "\$SERVER[\"socket\"] == \"[::]:443\" {\n";
			$write_str .= "	 ssl.engine = \"enable\"\n";
			$write_str .= "  ssl.pemfile = \"/etc/ssl/server.pem\"\n";
			$write_str .= "}\n";
		}
		
		$flock = lock_file($file_path);
		//print_rr($write_str);
		file_put_contents($file_path, $write_str);
		unlock_file($flock);			
	}


	return true;
}

/*****************************
This function change_port_form() is used to change the form of 
the port information received from redis when you send sms. 
******************************/
function change_port_form($port_name)
{
	global $__MODULE_HEAD_ARRAY__;
	//port_name : gsm-1.3 gsm-3.4 change to umts-1.3 4g-3.4 ...
	$port_info = explode('-',$port_name);
	if (!empty($port_info)) {
		if (count($port_info) == 2) {
			$port_head_str = $port_info[0];
			$port_tail_str = $port_info[1];
			$module_info = explode('.',$port_tail_str);
			if(count($module_info) == 2) {
				//$board_num = $module_info[0];
				$board_num = 1;
				if(preg_match('/[\(]/', $module_info[1], $matches)){
					$temp_array = explode('(', $module_info[1]);
					$channel_num = $temp_array[0];
				} else {
					$channel_num = trim(substr($module_info[1],0,2));
				}
				$port_head_str = $__MODULE_HEAD_ARRAY__[$board_num][$channel_num];
				if (strstr($port_head_str,'null') || strstr($port_head_str,'none') || strstr($port_head_str,'unknown')) {
					$port_head_str = "null-";
				}
				
				return $port_head_str.$port_tail_str;
			} else {
				return $port_name;
			}
		} else {
			return $port_name;
		}
	}
	
	
}
////////////////////////////////////////////////////////////////////
// 3G module :functions added
/*****************************
This function get_module_type() is used to checkout the module of 
the Wireless Gateway include GSM,3G .etc.
By read the file "/tmp/.module_type",if it not exists,the default is GSM.
******************************/
function get_module_type()
{
	global $__GSM_SUM__;
	$module_array = '';
	$file = '/tmp/.module_type';
	if (file_exists($file)) {
		$str = file_get_contents($file,true);
	 	$strline = explode("\n",$str);
	 	$linenum = count($strline);
	 	for ($i=1;$i<$linenum;$i++) {
			$module = explode(',',$strline[$i-1]);
			$moduleline = count($module);
			for ($j=1;$j<=$moduleline;$j++) {
				$module_str = '';
				$module_name = trim($module[$j-1]);
				if ($module_name=='uc15' || $module_name=='uc15a'|| $module_name=='uc15e' || $module_name=='uc15t' || $module_name=='Quectel_UC15') {
					$module_str = 'umts-';
				} else if ($module_name == 'Quectel_M35' || $module_name == 'm35' || $module_name == 'sim840' || $module_name == 'Quectel_M26') {
					$module_str = 'gsm-';
				} else if ($module_name == 'SIMCOM_SIM6320C') {
					$module_str = 'cdma-';
				} else if($module_name == 'EC20CE' || $module_name == 'EC25EF'){
					$module_str = 'lte-';
				}else{
					$module_str = $module_name.'-';
				}
				$module_array[$i][$j] = $module_str;
			}
		}
	} else {
		for ($i = 1; $i <= $__GSM_SUM__; $i++) {
			$module_array[1][$i] = 'gsm-';
			$module_array[2][$i] = 'gsm-';
		}
	}
	//print_r($module_array);
	return $module_array;
}

function get_module_real_type()
{
	global $__GSM_SUM__;
	
	$module_array = [];
	$flag = 0;
	for ($i = 1; $i <= $__GSM_SUM__; $i++) {
		exec("grep -rn Revision /tmp/gsm/$i", $temp);
		$module_select = explode(": ", $temp[0]);
		$module_name = $module_select[1];
		$temp = [];
		
		if(strstr($module_name, 'uc15') ||
			strstr($module_name, 'uc15a') ||
			strstr($module_name, 'uc15e') ||
			strstr($module_name, 'uc15t') ||
			strstr($module_name, 'Quectel_UC15')){
			$module_array[$i] = 'umts';
		}else if(strstr($module_name, 'Quectel_M35') || 
			strstr($module_name, 'm35') || 
			strstr($module_name, 'sim840') ||
			strstr($module_name, 'Quectel_M26')){
			$module_array[$i] = 'gsm';
		}else if(strstr($module_name, 'SIMCOM_SIM6320C')){
			$module_array[$i] = 'cdma';
		}else if(strstr($module_name, 'EC20CE')){
			$module_array[$i] = 'EC20CE';
			$flag = 1;
		}else if(strstr($module_name, 'EC25EF')){
			$module_array[$i] = 'EC25EF';
			$flag = 1;
		}else if(strstr($module_name, 'EC25AUTL')){
			$module_array[$i] = 'EC25AUTL';
			$flag = 1;
		}else if(strstr($module_name, 'EC25AUT')){
			$module_array[$i] = 'EC25AUT';
			$flag = 1;
		}else if(strstr($module_name, 'EC25AU')){
			$module_array[$i] = 'EC25AU';
			$flag = 1;
		}else if(strstr($module_name, 'EC25A')){
			$module_array[$i] = 'EC25A';
			$flag = 1;
		}else if(strstr($module_name, 'EC25V')){
			$module_array[$i] = 'EC25V';
			$flag = 1;
		}else if(strstr($module_name, 'EC25J')){
			$module_array[$i] = 'EC25J';
			$flag = 1;
		}else{
			$module_array[$i] = 'null';
		}
	}
	
	if($flag == 0){
		$str = file_get_contents('/tmp/.module_type');
		$temp = explode(',', $str);
	}
	for($i=0;$i<count($temp);$i++){
		if($temp[$i]=='uc15' || $temp[$i]=='uc15a' || $temp[$i]=='uc15e' || $temp[$i]=='uc15t' || $temp[$i]=='Quectel_UC15'){
			$module_array[$i+1] = 'umts';
		}else if($temp[$i]=='Quectel_M35' || $temp[$i]=='m35' || $temp[$i]=='sim840' || $temp[$i]=='Quectel_M26'){
			$module_array[$i+1] = 'gsm';
		}else if($temp[$i]=='SIMCOM_SIM6320C'){
			$module_array[$i+1] = 'cdma';
		}else if($temp[$i]=='EC20CE'){
			$module_array[$i+1] = 'EC20CE';
		}else if($temp[$i]=='EC25EF'){
			$module_array[$i+1] = 'EC25EF';
		}else{
			$module_array[$i+1] = 'null';
		}
	}

	return $module_array;
}

//change the show forms about sms_routing  
function change_sms_routing_form($members)
{
	//org: gsm1.1,gsm2.3,gsm2.4  
	//    change to  gsm-1.1,3g-2.3,4g-2.4
	
	$members = str_replace('gsm','gsm-',$members);
	$changed_members = '';
	$members = explode(",",$members);
	foreach ($members as $member) {
		$changed_members .= change_port_form($member).',';
	}
	$changed_members = substr($changed_members,0,-1);
	return $changed_members;
}
function change_sms_routing_form_new($from_members){
	if(strstr($from_members, '(')){
		$members = strstr($from_members, '(', true);
		$members = str_replace('gsm','',$members);
		//$members = explode("(", $members);
		$members = explode(",", $members);
	}else {
		$members = str_replace('gsm','',$from_members);
		$members = explode(",",$members);
	}
	$modified_members = '';
	foreach ($members as $member) {
		$port_info = explode('.', $member);
		$modified_members .= get_gsm_name_by_channel($port_info[1]).',';
	}
	$modified_members = substr($modified_members,0,-1);
	return $modified_members;
}
function change_sms_routing_form_new_for_cdr_excel($from_members,$sys_type){
	if(strstr($from_members, '(')){
		$members = strstr($from_members, '(', true);
		$members = str_replace('gsm','',$members);
		//$members = explode("(", $members);
		$members = explode(",", $members);
	}else {
		$members = str_replace('gsm','',$from_members);
		$members = explode(",",$members);
	}
	$modified_members = '';
	foreach ($members as $member) {
		$port_info = explode('.', $member);
		
		$modified_members .= get_gsm_name_by_channel_for_cdr($port_info[1],1,true,$sys_type).',';
	}
	$modified_members = substr($modified_members,0,-1);
	return $modified_members;
}
///////////////////////////////
//read redis 
function read_redis_info()
{
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	$cluster_info = get_cluster_info();
	$single_server = array(
			'host' => '127.0.0.1',
			'port' => '6379',
	);
	$single_version_info = array(
			'ip' => '127.0.0.1',
			'name' => '',
			'local.product.module.type' => '',
			'local.product.sw.version' => '',
			'local.product.board.version' => '',
			'local.product.sw.buildtime' => '',
			'error' => '',
	);
	$all_version_info = array();
	
	
	//get master info
	$single_server['host'] = "127.0.0.1";
	$redis_client = new Predis\Client($single_server);	
	try{
		foreach ($single_version_info as $key=>$value) {
			//echo $value;
			if ( $key == 'name' ) {
				$single_version_info[$key] = 'master';
			} else if ($key == 'ip') {
				$single_version_info['ip'] = $redis_client->get('local.system.eth0.ip');
			} else {
				$single_version_info[$key] = $redis_client->get($key);
			}
		}	
		$all_version_info['master'] = $single_version_info;
	} catch(Exception $e ){
		$all_version_info['master']['error'] = $e->getMessage();
		$all_version_info['master']['ip'] = '';
		$all_version_info['master']['name'] = '';
	}
	
	
	if ($cluster_info['mode'] == 'master') {
		//do something
		for($b=2; $b<=$__BRD_SUM__; $b++) {
			if( (isset($cluster_info[$__BRD_HEAD__.$b.'_ip'])) && $cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
				try {
					$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
					$single_server['host'] = $slaveip;
					$redis_client = new Predis\Client($single_server);
					foreach ($single_version_info as $key=>$value) {
						if ( $key == 'ip' ) {
							$single_version_info[$key] = $slaveip;
						} else if ( $key == 'name' ) {
							$single_version_info[$key] = 'slave'.$b;
						} else {
							$single_version_info[$key] = $redis_client->get($key);
						}
						//echo "key:".$key."----------- value:".$single_version_info[$key]."<br>";
					}
					$all_version_info['slave'.$b] = $single_version_info;
				} catch(Exception $e) {
					// e->getMessage() : such as :Connect timeout![tcp:172.xx.xx.xx],delete "[xx]"
					$e_msg = $e->getMessage();
					if (strstr($e_msg,"[")) {
						$pos = strpos($e_msg,"[");
						$e_msg = substr($e_msg,0,$pos);
					}
					if ($e_msg == "No route to host " || $e_msg == "Connection timed out ") {
						$e_msg = 'disconnected';
					} else {
						$e_msg = 'unknown';
					}
					$all_version_info['slave'.$b]['error'] = $e_msg;
					$all_version_info['slave'.$b]['ip'] = $slaveip;
					$all_version_info['slave'.$b]['name'] = 'slave'.$b;
					
					$all_version_info['slave'.$b]['local.product.module.type'] = '';
					$all_version_info['slave'.$b]['local.product.sw.version'] = '';
					$all_version_info['slave'.$b]['local.product.board.version'] = '';
					$all_version_info['slave'.$b]['local.product.sw.buildtime'] = '';
				}
			}
		}	
	} else if ($cluster_info['mode'] == 'stand_alone') {
		$all_version_info['master']['name'] = 'stand_alone';
	} else if ($cluster_info['mode'] == 'slave') {
		$all_version_info['master']['name'] = 'slave';
	}

	return $all_version_info;
}

function get_module_info($type,$module) {
	if ($type == 'name') {
		switch($module) {
			case "uc15a":
				echo "VS-GWM400W";break;
			case "uc15e":
				echo "VS-GWM400W";break;
			case "uc15t":
				echo "VS-GWM400W";break;
			case "sim840":
				echo "VS-GGU-E2M0400";break;
			default:
				echo "VS-GGU-E2M0400";break;
		}
	}
	if ($type == 'modem') {
		switch($module) {
			case "uc15a":
				echo "850/1900MHz@UMTS<br>850/900/1800/1900MHz@GSM";break;
			case "uc15e":
				echo "900/2100MHz@UMTS<br>900/1800MHz@GSM";break;
			case "uc15t":
				echo "850/2100MHz@UMTS<br>850/900/1800/1900MHz@GSM";break;
			case "sim840":
				echo "850/900/1800/1900MHz@GSM<br>";break;
			default:
				echo "850/900/1800/1900MHz@GSM<br>";break;
		}		
	}
}
?>
