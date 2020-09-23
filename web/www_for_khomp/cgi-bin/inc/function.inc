<?php
require_once("/www/cgi-bin/inc/define.inc");
require_once("/www/cgi-bin/inc/cdrdb.php");
require_once("/www/cgi-bin/inc/smsoutboxdb.php");

//AQL
require_once("/www/cgi-bin/inc/aql.php");
include_once("/www/cgi-bin/inc/userdb.php");

function get_oem_info_sepcs($sepc) {

	if (!$sepc) {
		return null;
	}

	$redis_server = array(
			'host'     => '127.0.0.1',
			'port'     => 6379
	);

	$predis_db = new Predis\Client($redis_server);
	if (!$predis_db) {
		return null;
	}

	return $predis_db->hget('local.product.oem.ver.ctl', $sepc);
}

function check_oem_funs($fun_name) {

	$val = get_oem_info_sepcs($fun_name);

	if (!$val)
		return 0;

	return 1;
}

function create_uuid($prefix = ""){    //可以指定前缀
    $str = md5(uniqid(mt_rand(), true));   
    $uuid  = substr($str,0,8) . '-';   
    $uuid .= substr($str,8,4) . '-';   
    $uuid .= substr($str,12,4) . '-';   
    $uuid .= substr($str,16,4) . '-';   
    $uuid .= substr($str,20,12);   
    return $prefix . $uuid;
}

function get_hostname() {
	return exec("hostname");
}

function is_true($str)
{
	if ( strlen($str) <= 0 )
		return false;

	/* Determine if this is a true value */
	if (0 == strcasecmp($str, "yes") ||
	    0 == strcasecmp($str, "true") ||
	    0 == strcasecmp($str, "y") ||
	    0 == strcasecmp($str, "t") ||
	    0 == strcasecmp($str, "1") ||
	    0 == strcasecmp($str, "on"))
		return true;

	return false;
}

function is_ip($str)
{
	if(!$str) return false;

	if(preg_match('/^((2[0-4][0-9]|25[0-5]|[01]?[0-9][0-9]?)\.){3}(2[0-4][0-9]|25[0-5]|[01]?[0-9][0-9]?)$/',$str)) {
		return true;
	}

	return false;
}


function __cut_str($str,$start_str,$end_str)
{
	if( !$str || !$start_str || !$end_str ) {
		return '';
	}

	if( ($find_start=strpos($str,$start_str)) === false ) {
		return '';
	}

	if( ($find_end=strpos($str,$end_str,$find_start)) === false ) {
		return '';
	}
	
	$start_pos = $find_start + strlen($start_str);
	
	if( ($c_str_l = $find_end - $start_pos) <= 0 ) {
		return '';
	}

	return substr($str,$start_pos,$c_str_l);    
}

function ast_running()
{
	if(!file_exists("/var/run/asterisk.ctl")) return false;

	return true;
}


function ast_reload()
{
	exec("asterisk -rx \"core reload\" > /dev/null 2>&1 &");
}

function ast_dialplan_reload()
{
	exec("asterisk -rx \"dialplan reload\" > /dev/null 2>&1 &");
}

function sip_reload()
{
	exec("asterisk -rx \"sip reload\" > /dev/null 2>&1 &");
}

function manager_reload()
{
	exec("asterisk -rx \"manager reload\" > /dev/null 2>&1 &");
}

function ast_cmdprocess($cmd)
{
	$cmd = str_replace("\"","\\\\\\\"",$cmd);
	$cmd = str_replace("?","@",$cmd);
	return $cmd;
}

function get_exec($cmd)
{
	$output = '';
	$file = popen($cmd,"r");
	while(!feof($file)) {
		$line = fgets($file);
		$output .= $line;
	}
	pclose($file);

	return $output;
}

function execute_astcmd($command)
{
	$ret_str = '';
	$command = trim($command);
	$command = ltrim($command,'!');

	$command = str_replace("\"","\\\\\\\"",$command);

	if(strcasecmp($command,"help") == 0 || $command == '?' ) {
		$command = 'core show help';
	} 

	if($command != '') {
		$ast_cmd = "asterisk -rx \"$command\" 2> /dev/null";
		$file = popen($ast_cmd,"r");
		while(!feof($file)) {
			$line = fgets($file);
			$ret_str .= $line."<br/>";
			//ob_flush();
			//flush();
		}
		pclose($file);
	}

	return $ret_str;
}

/*
*
* Set Speaker volume level
* level 0-100
*/
function set_slevel($span,$level)
{
	exec("asterisk -rx \"gsm send at $span \"AT+CLVL=$level\"\" > /dev/null 2>&1 ");
}

/*
*
* Set Microphone gain level
* level: 0-15
*/
function set_mlevel($span,$level)
{
	exec("asterisk -rx \"gsm send at $span \"AT+CMIC=0,$level\"\" > /dev/null 2>&1 ");
}


/*
*
* Set ADC dacgain 
*/
function set_dacgain($span, $value)
{
	exec("asterisk -rx \"gsm set dacgain $span $value\" > /dev/null 2>&1 ");
}


/*
*
* Set ADC adcgain 
*/
function set_adcgain($span, $value)
{
	exec("asterisk -rx \"gsm set adcgain $span $value\" > /dev/null 2>&1 ");
}
/*
 * Get the system language type
 *
 */
function get_language_type(){
	$conf_path = "/etc/asterisk/gw/web_language.conf";
	$conf_array = get_conf($conf_path);
	if(isset($conf_array['general']['language']) && $conf_array['general']['language'] != "")
		$language_type =$conf_array['general']['language'];
	return $language_type;
}
function get_gsm_channel_by_name($name)
{
	//$type_and_name: gsm-1, gsm-5 ... or 1, 2, 3 ... or Board-1-gsm-1 or Board-2-gsm-1 ...

	global $__GSM_HEAD__;
	global $__BRD_HEAD__;
	global $__GSM_SUM__;
	global $__BRD_SUM__;

	$append = '';
	
	if(substr($name,0,strlen($__BRD_HEAD__)) == $__BRD_HEAD__) {
		sscanf($name,"$__BRD_HEAD__%d-$__GSM_HEAD__%d%s",$board,$channel,$append);
	} else if(substr($name,0,strlen($__GSM_HEAD__)) == $__GSM_HEAD__) {
		sscanf($name,"$__GSM_HEAD__%d%s",$channel,$append);
		if(substr($append,0,1) == '.'){
			sscanf($name,"$__GSM_HEAD__%d.%d%s",$board,$channel,$append);
		}
	} else {
		$channel = $name;
	}

	if($channel >= 1 && $channel <= ($__GSM_SUM__ * $__BRD_SUM__)) {
		if($channel > $__GSM_SUM__) {
			$board = ceil($channel/$__GSM_SUM__);
			$channel = $channel - ($board -1) * $__GSM_SUM__;
		} else {
			if(!isset($board)) $board = 1;
		}
	} else {
		$board = '';
		$channel = '';
	}

	return array('board' => $board, 'channel' => $channel, 'append' => $append);
}

function get_channel_type($type_and_name)
{
	
	//SIP-1001 , Board-1-gsm-1, gsm-2
	////////////////////////////
	//change GSM to gsm/3g    ing
	global $__GSM_HEAD__;
	global $__UMTS_HEAD__;
	//////////////////////////
	global $__BRD_HEAD__;
	global $__SIP_HEAD__;
	global $__IAX_HEAD__;
	global $__GRP_HEAD__;
	
	if(substr($type_and_name,0,strlen($__BRD_HEAD__)) == $__BRD_HEAD__) {
		return 'gsm';
	}

	if(substr($type_and_name,0,strlen($__GSM_HEAD__)) == $__GSM_HEAD__) {
		return 'gsm';
	}

	if(substr($type_and_name,0,strlen($__SIP_HEAD__)) == $__SIP_HEAD__) {
		return 'sip';
	}
	
	if(substr($type_and_name,0,strlen($__IAX_HEAD__)) == $__IAX_HEAD__) {
		return 'iax';
	}

	if(substr($type_and_name,0,strlen($__GRP_HEAD__)) == $__GRP_HEAD__) {
		return 'grp';
	}
	
	//add 3g 
	if(substr($type_and_name,0,strlen($__UMTS_HEAD__)) == $__UMTS_HEAD__) {
		return 'umts';
	}	
	
	return '';
}

function get_gsm_aliasname($channel,$board=1)
{
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $__GSM_SUM__;
	global $__GSM_HEAD__;
	static $GSM_ALIASNAME = NULL;
	$file_path = '/etc/asterisk/gw_gsm.conf';
	$conf_str = '';

	$bc = get_gsm_channel_by_name($channel);
	if($bc['board'] != 1) {
		$board = $bc['board'];
	}
	$channel = $bc['channel'];

	if(isset($GSM_ALIASNAME)){
		if(isset($GSM_ALIASNAME[$board][$channel])){
			return $GSM_ALIASNAME[$board][$channel];
		}else{
			return '';
		}
	}
	
	$GSM_ALIASNAME = false;
	
	$conf_str = '';
	$b = 1;

	if(is_file($file_path)){
		$conf_str = file_get_contents($file_path);
		for($c = 1; $c <= $__GSM_SUM__; $c++){
			if(($start = strpos($conf_str,"[$c]\n")) !== false){
				if(($start = strpos($conf_str,'name=',$start)) !== false){ 
					$start += strlen('name='); 
					if(($end = strpos($conf_str,"\n",$start)) !== false){
						if($name = substr($conf_str,$start,$end-$start)){
							$GSM_ALIASNAME[$b][$c] = $name;
						}
					}
				}
			}
		}
	}
	
	
	$cluster_info = get_cluster_info();
	
	if($cluster_info['mode'] == 'master'){	
	
		for($b = 2; $b <= $__BRD_SUM__; $b++){
			if(isset($cluster_info[$__BRD_HEAD__.$b.'_ip']) && $cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
				$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
				//$file_path = '/configs/gw_gsm.conf';
				//$urlfile = 'http://'.$slaveip.$file_path;
				//$conf_str = gw_send_GET($urlfile);
				$conf_str = get_slave_file_contents($slaveip,$file_path);			

				for($c = 1; $c <= $__GSM_SUM__; $c++){
					if(($start = strpos($conf_str,"[$c]\n")) !== false){
						if(($start = strpos($conf_str,'name=',$start)) !== false){ 
							$start += strlen('name='); 
							if(($end = strpos($conf_str,"\n",$start)) !== false){
								if($name = substr($conf_str,$start,$end-$start)){
									$GSM_ALIASNAME[$b][$c] = $name;
								}
							}
						}
					}
				}
			}
		}
	}

	if(isset($GSM_ALIASNAME[$board][$channel])){
		return $GSM_ALIASNAME[$board][$channel];
	}else{
		return '';
	}
}

function get_gsm_name_by_channel($channel,$board=1,$showalias=true)
{
	//change GSM to gsm/3g    ing
	global $__GSM_HEAD__;
	global $__MODULE_HEAD_ARRAY__;
	global $__GSM_SUM__;
	
	$bc = get_gsm_channel_by_name($channel);
	if($bc['board'] != 1) {
		$board = $bc['board'];
	}
	$channel = $bc['channel'];

	if($showalias) {
		$alias = get_gsm_aliasname($channel,$board);
		if($alias) {
			$alias = "($alias)";
		}
	} else {
		$alias = '';
	}

	//Freedom Modify 2014-02-18 23:33
	//Modify gsm-1 to gsm-1.1    gsm-5 to gsm-2.1
	//$port = ($board - 1) * $__GSM_SUM__ + $channel;
	$sys_type = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys sys_type");
	if($sys_type == 3){
		$board_num = intval(($channel-1)/4)+1;
		$port = "${board_num}.${channel}";
	} else {
		$port = "$channel";
	}

	if (isset( $__MODULE_HEAD_ARRAY__[$board][$channel] )) {
		$module_head =  $__MODULE_HEAD_ARRAY__[$board][$channel];
	} else {
		$module_head =  $__MODULE_HEAD_ARRAY__[1][1];
	}
	$name = "$module_head$port$alias".$bc['append'];
	if (strstr($name,'null') || strstr($name,'none') || strstr($name,'unknown')) { 
		$name = preg_replace('/(\w+)-/','null-',$name);
	}
	return $name;
}

function get_gsm_name_by_channel_for_cdr($channel,$board=1,$showalias=true,$sys_type)
{
	//change GSM to gsm/3g    ing
	global $__GSM_HEAD__;
	global $__MODULE_HEAD_ARRAY__;
	global $__GSM_SUM__;
	
	$bc = get_gsm_channel_by_name($channel);
	if($bc['board'] != 1) {
		$board = $bc['board'];
	}
	$channel = $bc['channel'];

	if($showalias) {
		$alias = get_gsm_aliasname($channel,$board);
		if($alias) {
			$alias = "($alias)";
		}
	} else {
		$alias = '';
	}

	//Freedom Modify 2014-02-18 23:33
	//Modify gsm-1 to gsm-1.1    gsm-5 to gsm-2.1
	//$port = ($board - 1) * $__GSM_SUM__ + $channel;
	if($sys_type == 3){
		$board_num = intval(($channel-1)/4)+1;
		$port = "${board_num}.${channel}";
	} else {
		$port = "$channel";
	}

	if (isset( $__MODULE_HEAD_ARRAY__[$board][$channel] )) {
		$module_head =  $__MODULE_HEAD_ARRAY__[$board][$channel];
	} else {
		$module_head =  $__MODULE_HEAD_ARRAY__[1][1];
	}
	$name = "$module_head$port$alias".$bc['append'];
	if (strstr($name,'null') || strstr($name,'none') || strstr($name,'unknown')) { 
		$name = preg_replace('/(\w+)-/','null-',$name);
	}
	return $name;
}

function get_gsm_name_by_channel_for_showtype($channel,$board=1,$showalias=true)
{
	//change GSM to gsm/3g    ing
	global $__GSM_HEAD__;
	global $__MODULE_HEAD_ARRAY__;
	global $__GSM_SUM__;
	
	$bc = get_gsm_channel_by_name($channel);
	if($bc['board'] != 1) {
		$board = $bc['board'];
	}
	$channel = $bc['channel'];

	if($showalias) {
		$alias = get_gsm_aliasname($channel,$board);
		if($alias) {
			$alias = "($alias)";
		}
	} else {
		$alias = '';
	}

	//Freedom Modify 2014-02-18 23:33
	//Modify gsm-1 to gsm-1.1    gsm-5 to gsm-2.1
	//$port = ($board - 1) * $__GSM_SUM__ + $channel;
	$sys_type = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys sys_type");
	if($sys_type == 3){
		$board = intval(($channel-1)/4)+1;
	}

	if($sys_type == 1 || $sys_type == 2 || $sys_type == 5){
		$port = "${channel}";
	}else{
		$port = "${board}.${channel}";
	}
	$name = "$port$alias".$bc['append'];
	if (strstr($name,'null') || strstr($name,'none') || strstr($name,'unknown')) { 
		$name = preg_replace('/(\w+)-/','null-',$name);
	}
	return $name;
}

function get_gsm_type_by_channel($channel, $board=1){
	global $__MODULE_HEAD_ARRAY__;
	
	if (isset( $__MODULE_HEAD_ARRAY__[$board][$channel] )) {
		$module_head =  $__MODULE_HEAD_ARRAY__[$board][$channel];
	} else {
		$module_head =  $__MODULE_HEAD_ARRAY__[1][1];
	}
	
	$result = explode('-', $module_head);
	
	$res = strtoupper($result[0]);
	
	return $res;
}

function get_gsm_value_by_channel($channel,$board='')
{
	global $__GSM_HEAD__;
	global $__BRD_HEAD__;

	$bc = get_gsm_channel_by_name($channel);
	if($bc['board'] != 1) {
		$board = $bc['board'];
	}
	$channel = $bc['channel'];

	if($board) {
		$value = $__BRD_HEAD__.$board.'-'.$__GSM_HEAD__.$channel;
	} else {
		$value = $__GSM_HEAD__.$channel;
	}

	return $value;
}

function get_sip_name_has_head($name)
{
	global $__SIP_HEAD__;
	if(substr($name,0,strlen($__SIP_HEAD__)) == $__SIP_HEAD__) {
		return $name;
	} else {
		return "$__SIP_HEAD__$name";
	}
}

function get_iax_name_has_head($name) 
{
	global $__IAX_HEAD__;
	if(substr($name,0,strlen($__IAX_HEAD__)) == $__IAX_HEAD__) {
		return $name;
	} else {
		return "$__IAX_HEAD__$name";
	}
}

function get_sip_name_no_head($name)
{
	global $__SIP_HEAD__;
	if(substr($name,0,strlen($__SIP_HEAD__)) == $__SIP_HEAD__) {
		return substr($name,strlen($__SIP_HEAD__));
	} else {
		return $name;
	}
}

function get_iax_name_no_head($name)
{
	global $__IAX_HEAD__;
	if(substr($name,0,strlen($__IAX_HEAD__)) == $__IAX_HEAD__) {
		return substr($name,strlen($__IAX_HEAD__));
	} else {
		return $name;
	}
}

function get_grp_name_has_head($name)
{
	global $__GRP_HEAD__;
	if(substr($name,0,strlen($__GRP_HEAD__)) == $__GRP_HEAD__) {
		return $name;
	} else {
		return "$__GRP_HEAD__$name";
	}
}

function get_grp_name_no_head($name)
{
	global $__GRP_HEAD__;
	if(substr($name,0,strlen($__GRP_HEAD__)) == $__GRP_HEAD__) {
		return substr($name,strlen($__GRP_HEAD__));
	} else {
		return $name;
	}
}

function get_channel_name($type_and_name)
{
	//change GSM to 2g/3g    ing
	$type = get_channel_type($type_and_name);
	if($type == 'gsm') {
		return get_gsm_name_by_channel($type_and_name);
	} 
	
	
	if ($type == 'umts') {
		return get_gsm_name_by_channel($type_and_name);
	}

	if($type == 'sip') {
		return get_sip_name_has_head($type_and_name);
	}

	if($type == 'grp') {
		return get_grp_name_has_head($type_and_name);
	}
	
	if($type == 'iax') {
		return get_iax_name_has_head($type_and_name);
	}

	return '';
}


function lock_file($file_path)
{
	$new_name = str_replace("/","_",$file_path);
	$lock_path = "/tmp/lock/" . $new_name . ".lock";
	$fh = fopen($lock_path, "w+");
	if($fh <= 0) {
		return -1;
	}
	flock($fh,LOCK_EX);

	return $fh;
}

function unlock_file($fh)
{
	if($fh > 0) {
		flock($fh,LOCK_UN);
		fclose($fh);
	}
}


function bytesConvert($bytes)
{
	$ext = array('B', 'kB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB');
	$unitCount = 0;
	for(; $bytes > 1024; $unitCount++) $bytes /= 1024;
	return round($bytes,2) ." ". $ext[$unitCount];
}

function get_dns()
{
	$dns_file="/etc/resolv.conf";

	$ret=NULL;

	if(file_exists($dns_file)) {
		$fh=fopen($dns_file,"r");
		while(!feof($fh) && ($line = fgets($fh))) {
			if(substr($line,0,10) == "nameserver") {
				$dns=substr($line,10,strlen($line)-10);
				$dns=trim($dns);
				if(preg_match("/^((2[0-4][0-9]|25[0-5]|[01]?[0-9][0-9]?)\.){3}(2[0-4][0-9]|25[0-5]|[01]?[0-9][0-9]?)$/",$dns)) {
					$ret[]=$dns;
				}
			}
		}
	}

	return $ret;
}


function echo_saved()
{
	echo "<img src= \"../../images/action_ok.gif\"><font color=00FF00>Saved</font>";
}


function get_sip_registrys()
{
	$ret = array();

	$reg=`asterisk -rx "sip show registry" 2> /dev/null`;
	if(!empty($reg)) {

/* Format
Host                                    dnsmgr Username       Refresh State                Reg.Time \n
172.16.47.243:5060                      | N      | 101          |      120 | Request Sent         | \n
172.16.70.121:5060                      | N      | 101          |      105 | Registered           | Thu, 01 Jan 1970 06:49:37\n
2 SIP registrations.\n
\n 
*/ 
		$line = explode("\n",$reg);
		if(!empty($line)) {
			$a = 0;
			//Don't need first line and last 2 lines.
			$line_len = count($line) - 2;
			for($i=1; $i < $line_len; $i++) {
				$item = explode("|",$line[$i],6);
				if(isset($item[0]) && isset($item[2]) && isset($item[4])) {
					$host_and_port = trim($item[0]);
					$temp = explode(":",$host_and_port);
					if(isset($temp[0]))
						$host = $temp[0];
					else
						continue;

					$user = trim($item[2]);

					$status = trim($item[4]);

					//if($status == 'Registered') {
						$ret[$a]['host'] = $host;
						$ret[$a]['username'] = $user;
						$ret[$a]['status'] = $status;
						$a++;
					//}
				}
			}
		}
	}

	return $ret;
}


function get_sip_peers()
{
	$ret = array();

	$reg=`asterisk -rx "sip show peers" 2> /dev/null`;
	if(!empty($reg)) {

/* Format
Name/username              Host                                    Dyn Forcerport ACL Port     Status\n
101-172.16.70.121/101     | 172.16.70.121                           |     |  N  |     | 5060     | Unmonitored |\n
1 sip peers [Monitored: 0 online, 0 offline Unmonitored: 1 online, 0 offline]\n
*/ 
		$line = explode("\n",$reg);
		if(!empty($line)) {
			$a = 0;
			//Don't need first line and last 2 lines.
			$line_len = count($line) - 2;
			for($i=1; $i < $line_len; $i++) {
				$item = explode("|",$line[$i],8);
				if(isset($item[0]) && isset($item[1]) && isset($item[2]) && isset($item[6])) {
					$context_and_name = trim($item[0]);
					$temp = explode("/",$context_and_name);
					if(isset($temp[1]))
						$name = $temp[1];
					else
						continue;

					$host = trim($item[1]);
					$dyn = trim($item[2]);
					$status = trim($item[6]);

					if($dyn == 'D') {
						$ret[$a]['D'] = true;
					} else {
						$ret[$a]['D'] = false;
					}

					$ret[$a]['username'] = $name;
					$ret[$a]['host'] = $host;
					$ret[$a]['status'] = $status;
					$a++;
				}
			}
		}
	}

	return $ret;
}

/*
	input: $username, iax user name
		   $filed is "host" or "status"
	output: ip address or iax status,
*/
function get_iax_peers_new($username,$field)                                                                                                                               
{                                                                                                                                                                          
/*                                                                                                                                                                         
Name/Username   | Host            |    | Mask            | Port     |     | Status      | Description
30002/30002     | 172.16.6.213    | (D) | 255.255.255.255 | 4569     |     | OK (19 ms)  |
30003/30003     | 172.16.100.23   | (S) | 255.255.255.255 | 4569     |     | OK (3 ms)   |
2 iax2 peers [2 online, 0 offline, 0 unmonitored]
*/                                                                                                                                                                         
	$reg=`asterisk -rx "iax2 show peers" 2> /dev/null`; 
	//echo $reg ;
	if(!empty($reg)) {
	//echo $reg;  
		$detect_str = '';
		$line = explode("\n",$reg);   

		$line_len = count($line) - 2; //delete first line and last line, count length
		for($i=1; $i < $line_len; $i++) {		//i=0 is the first line that we don't need
			if (strstr($line[$i],$username)!=false){
				$detect_str=$line[$i];
			}
		}
		$detect_array=explode('|',$detect_str);
		//print_r($detect_array);
		$array_len = count($detect_array);
		if ($field=='host'){
			if (isset($detect_array[1])) {
				$ret=trim($detect_array[1]);
			} else {
				$ret = '';
			}
		}
		if ($field=='status'){
			if (isset($detect_array[6])) {
				$ret = trim($detect_array[6]);
			} else {
				$ret = '';
			}
		}
	} else {
		$ret = '';
	}
	return $ret;
}  

function parse_register($register)
{
	//register => [peer?][transport://]user[@domain][:secret[:authuser]]@host[:port][/extension][~expiry]

	$ary = explode('@',$register);
	if(count($ary) == 2) {  // $register = [peer?][transport://]user[:secret[:authuser]]@host[:port][/extension][~expiry]
		$part1 = $ary[0];  // $part1 = [peer?][transport://]user[:secret[:authuser]]
		$part2 = $ary[1];  //$part2 = host[:port][/extension][~expiry]
		$ary = explode('?',$part1);
		if(count($ary) == 2) { //$part1 = peer?[transport://]user[:secret[:authuser]]
			$reg['peer'] = $ary[0];  //peer
			$part22 = $ary[1];  // $part22 = [transport://]user[:secret[:authuser]]
			$ary = explode('://',$part22);
			if(count($ary) == 2) { //$part22 = transport://user[:secret[:authuser]]
				$reg['transport'] = $ary[0];  //transport
				$part222 = $ary[1];  //$part222 = user[:secret[:authuser]]
				$ary = explode(':',$part222);
				if(count($ary) == 2) {   //$part222 = user:secret
					$reg['user'] = $ary[0];   //user
					$reg['secret'] = $ary[1]; //secret
				} else if(count($ary) == 3) {  //$part222 = user:secret:authuser
					$reg['user'] = $ary[0];   //user
					$reg['secret'] = $ary[1];  //secret
					$reg['authuser'] = $ary[2]; //authuser
				} else if(count($ary) == 1) { //$part222 = user
					$reg_use = $part222;
				}
			} else if(count($ary) == 1) {  //$part22 = user[:secret[:authuser]]
				$ary = explode(':',$part22);
				if(count($ary) == 2) { // $part22 = user:secret
					$reg['user'] = $ary[0];
					$reg['secret'] = $ary[1];
				} else if(count($ary) == 3) { //$part22 = user:secret:authuser
					$reg['user'] = $ary[0];
					$reg['secret'] = $ary[1];
					$reg['authuser'] = $ary[2];
				} else if(count($ary) == 1) { //$part22 = user
					$reg['user'] = $part22;
				}
			}
		} else if(count($ary) == 1) {  // $part1 = [transport://]user[:secret[:authuser]]
			$ary = explode('://',$part1);
			if(count($ary) == 2) {   // $part1 = transport://user[:secret[:authuser]]
				$reg['transport'] = $ary[0];
				$part22 = $ary[1];   //$part22 = user[:secret[:authuser]]
			} else if(count($ary) == 1) {  //$part1 = user[:secret[:authuser]]
				$ary = explode(':',$part1);
				if(count($ary) == 2) { // $part1 = user:secret
					$reg['user'] = $ary[0];
					$reg['secret'] = $ary[1];
				} else if(count($ary) == 3) { //$part1 = user:secret:authuser
					$reg['user'] = $ary[0];
					$reg['secret'] = $ary[1];
					$reg['authuser'] = $ary[2];
				} else if(count($ary) == 1) { //$part1 = user
					$reg['user'] = $part1;
				}
			}
		}
	
		$ary = explode(':',$part2);
		if(count($ary) == 2) {  //$part2 = host:port[/extension][~expiry]
			$reg['host'] = $ary[0];  //host
			$part22 = $ary[1];     //$part22 = port[/extension][~expiry]
			$ary = explode('/',$part22);
			if(count($ary) == 2) { //$part22 = port/extension[~expiry] 
				$reg['port'] = $ary[0];  //port
				$part222 = $ary[1];     //$part222 = extension[~expiry]
				$ary = explode('~',$part222);
				if(count($ary) == 2) {  //$part222 = extension~expiry
					$reg['extension'] = $ary[0];  //extension
					$reg['expiry'] = $ary[1];     //~expiry
				} else if(count($ary) == 1) { //$part2 = extension
					$reg['extension'] = $part222;  //extension
				}
			} else if(count($ary) == 1) { //$part22 = port[~expiry]
				$ary = explode('~',$part22);
				if(count($ary) == 2) { //$part22 = port~expiry
					$reg['port'] = $ary[0];
					$reg['expiry'] = $ary[1];
				} else if(count($ary) == 1) { //$part22 = port
					$reg['port'] = $part22;
				}
			}
		} else if(count($ary) == 1) {  // $part2 = host[/extension][~expiry]
			$ary = explode('/',$part2);
			if(count($ary) == 2) { //$part2 = host/extension[~expiry]
				$reg['host'] = $ary[0];  //host
				$part22 = $ary[1]; // $part22 = extension[~expiry]
				$ary = explode('~',$part22);
				if(count($ary) == 2) { //$part22 = extension~expiry
					$reg['extension'] = $ary[0];
					$reg['expiry'] = $ary[1];
				} else if(count($ary) == 1) { //$part22 = extension
					$reg['extension'] = $part22;
				}
			} else if(count($ary) == 1) { //$part2 = host[~expiry]
				$ary = explode('~',$part2);
				if(count($ary) == 2) {  //$part2 = host~expiry
					$reg['host'] = $ary[0];
					$reg['expiry'] = $ary[1];
				} else if(count($ary) == 1) { //$part2 =host
					$reg['host'] = $part2;
				}
			}
		}
	} else if((count($ary) == 3)) { //  $register = [peer?][transport://]user@domain[:secret[:authuser]]@host[:port][/extension][~expiry]
		$part1 = $ary[0];  //$part1 = [peer?][transport://]user
		$part2 = $ary[1];  //$part2 = domain[:secret[:authuser]]
		$part3 = $ary[2];  // $part3= host[:port][/extension][~expiry]
		$ary = explode('?',$part1);
		if(count($ary) == 2) { //$part1 = peer?[transport://]user
			$reg['peer'] = $ary[0]; // peer
			$part22 = $ary[1];   //$part22 = [transport://]user
			$ary = explode('://',$part22);
			if(count($ary) == 2) { //$part22 = transport://user 
				$reg['transport'] = $ary[0];
				$reg['user'] = $ary[1];
			} else if(count($ary) == 1){ //$part22 = user
				$reg['user'] = $part22;
			}
		} else if(count($ary) == 1) { // $part1 = [transport://]user
			$ary = explode('://',$part1);
			if(count($ary) == 2) { // $part1 = transport://user
				$reg['transport'] = $ary[0];
				$reg['user'] = $ary[1];
			} else if(count($ary) == 1) { //$part1 = user
				$reg['user'] = $part1;
			}
		}

		$ary = explode(':',$part2);
		if(count($ary) == 3) {  // $part2 = domain:secret:authuser
			$reg['domain'] = $ary[0];
			$reg['secret'] = $ary[1];
			$reg['authuser'] = $ary[2];
		} else if(count($ary) == 2) {  //$part2 = domain:secret
			$reg['domain'] = $ary[0];
			$reg['secret'] = $ary[1];
		} else if(count($ary) == 1) { //$part2 = domain
			$reg['domain'] = $part2;
		}

		$ary = explode(':',$part3);
		if(count($ary) == 2) { // $part3 = host:port[/extension][~expiry]
			$reg['host'] = $ary[0];  //host
			$part22 = $ary[1]; // $part22 = port[/extension][~expiry]
			$ary = explode('/',$part22);
			if(count($ary) == 2) {  // $part3 = port/extension[~expiry]
				$reg['port'] = $ary[0];  //port
				$part222 = $ary[1]; // $part222 = extension[~expiry]
				$ary = explode('~',$part222);
				if(count($ary) == 2) { // $part222 = extension~expiry
					$reg['extension'] = $ary[0]; //extension
					$reg['expiry'] = $ary[1];  //expiry
				} else if(count($ary) == 1) { // $part222 = extension
					$reg['extension'] = $part222;
				}
			} else if(count($ary) == 1) { //$part22 = port[~expiry]
				$ary = explode('~',$part22);
				if(count($ary) == 2) { //$part22 = port~expiry
					$reg['port'] = $ary[0];
					$reg['expiry'] = $ary[1];
				} else if(count($ary) == 1) { //$part22 = port
					$reg['port'] = $part22;
				}
			}
		} else if(count($ary) == 1) { //$part3 = host[/extension][~expiry]
			$ary = explode('/',$part3);
			if(count($ary) == 2) { //$part3 = host/extension[~expiry]
				$reg['host'] = $ary[0]; //host
				$part22 = $ary[1];  // $part22 = extension[~expiry]
				$ary = explode('~',$part22);
				if(count($ary) == 2) { //$part22 = extension~expiry
					$reg['extension'] = $ary[0]; //extension
					$reg['expiry'] = $ary[1];  //expiry
				} else if(count($ary) == 1) { // $part2 = extension
					$reg['extension'] = $part22;
				}
			} else if(count($ary) == 1) { //$part3 = host[~expiry]
				$ary = explode('~',$part3);
				if(count($ary) == 2) { //$part3 = host~expiry
					$reg['host'] = $ary[0];
					$reg['expiry'] = $ary[1];
				} else if(count($ary) == 1) { //$part3 = host
					$reg['host'] = $part3;
				}
			}
		}
	}

	return isset($reg) ? $reg : NULL;
}

function format_time($time)
{
	$duration = intval($time);
	if($duration >= 24*3600) {
		$format_time = gmstrftime('%j - %H:%M:%S',$duration);
	} else {
		$format_time = gmstrftime('%H:%M:%S',$duration);
	}
	return $format_time;
}

function get_cluster_info()
{
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	static $CLUSTER_INFO = NULL;

	if(isset($CLUSTER_INFO)){
		return $CLUSTER_INFO;
	}

	$info = array();

	//Get Cluster Information
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$res = $aql->query("select * from cluster.conf");

	if(isset($res['general']['mode'])) {
		$info['mode'] = trim($res['general']['mode']);
	} else {
		$info['mode'] = 'stand_alone';
	}
	
	if(isset($res['general']['boardnum'])) {
		$info['boardnum'] = trim($res['general']['boardnum']);
	} else {
		$info['boardnum'] = '';
	}

	if(isset($res['slave']['password'])) {
		$info['slave_password'] = trim($res['slave']['password']);
	} else {
		$info['slave_password'] = '';
	}

	if(isset($res['slave']['ip'])) {
		$info['slave_ip'] = trim($res['slave']['ip']);
	} else {
		$info['slave_ip'] = '';
	}

	if(isset($res['slave']['masterip'])) {
		$info['slave_masterip'] = trim($res['slave']['masterip']);
	} else {
		$info['slave_masterip'] = '';
	}

	if(isset($res['master']['password'])) {
		$info['master_password'] = trim($res['master']['password']);
	} else {
		$info['master_password'] = '';
	}

	if(isset($res['master']['ip'])) {
		$info['master_ip'] = trim($res['master']['ip']);
	} else {
		$info['master_ip'] = '';
	}

	if(isset($res['slave']['remain_ori_ip'])) {
		$info['remain_ori_ip'] = trim($res['slave']['remain_ori_ip']);
	} else {
		$info['remain_ori_ip'] = 1;
	}

	for($b=2; $b<=$__BRD_SUM__; $b++) {
		if(isset($res['slavelist'][$__BRD_HEAD__.$b.'_ip'])) {
			$info[$__BRD_HEAD__.$b.'_ip'] = trim($res['slavelist'][$__BRD_HEAD__.$b.'_ip']);
		} else {
			$info[$__BRD_HEAD__.$b.'_ip'] = '';
		}

		if(isset($res['slavelist'][$__BRD_HEAD__.$b.'_ori_ip'])) {
			$info[$__BRD_HEAD__.$b.'_ori_ip'] = trim($res['slavelist'][$__BRD_HEAD__.$b.'_ori_ip']);
		} else {
			$info[$__BRD_HEAD__.$b.'_ori_ip'] = '';
		}
	}

	$CLUSTER_INFO = $info;
	return $info;
}


function get_slave_info($slave_number)
{
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	$info = array();

	//Get Cluster Information
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$res = $aql->query("select * from cluster.conf");

	if(isset($res['master']['password'])) {
		$info['password'] = trim($res['master']['password']);
	} else {
		$info['password'] = '';
	}

	if($slave_number >=2 && $slave_number <= $__BRD_SUM__ ) {
		if(isset($res['slavelist'][$__BRD_HEAD__.$slave_number."_ip"])) {
			$info['ip'] = trim($res['slavelist'][$__BRD_HEAD__.$slave_number."_ip"]);
		} else {
			$info['ip'] = '';
		}	
	} else {
		$info['ip'] = '';
	}

	return $info;
}

function get_lanip_info()
{
	$info['ip'] = '';
	$info['mask'] = '';
	$info['gw'] = '';
	$info['mac'] = '';
	exec("/my_tools/net_tool eth0 2> /dev/null && echo ok",$output);
	if(isset($output[11]) && $output[11] == 'ok' && isset($output[1]) && $output[1] == 'Enable') {
		if(isset($output[2])) $info['mac'] = $output[2];
		if(isset($output[3])) $info['ip'] = $output[3];
		if(isset($output[5])) $info['mask'] = $output[5];
		if(isset($output[6])) $info['gw'] = $output[6];
	}
	return $info;
}

function js_reboot_progress($str)
{
echo <<<EOF
	<table align="center" style="width:100%;">
		<tr>
			<td align="center">
				<img src="/images/loading.gif" align="middle"/>
			</td>
		</tr>
		<tr>
			<td align="center">
				$str 
			</td>
		</tr>
	</table>

	<script type="text/javascript">
	function check_network()
	{
		$.ajax({
			type: "GET",
			cache: false,
			url: "../../index.html",
			data: "",
			success:function(){
				//setTimeout("window.location.href='system-status.php'",10000); //Wait gsm start
				window.location.href='../../index.html';
			},
			error:function(){
				setTimeout("check_network()", 1000);
			}
		});
	}
	setTimeout("check_network()", 21000);
	</script>

	<meta http-equiv="refresh" content="60; URL=../../index.html" />
EOF;
//EOF

	ob_flush();
	flush();
}


/**************************************************/
/* start of apply infobar function */

/*
 *    --cmd_id----------($cmd_arg)-----------
 *      exec		exec()
 *	requset_slave	requset_slave()
 *    ---------------------------------------
 *	/tmp/web/wait_apply format:
 *
 *		exec@arg1@arg2
 *		requset_slave@arg1@arg2
 *		exec@arg1@arg2
 *		...
 */

function wait_apply($cmd_id, $arg1="", $arg2="")
{
	$wait_apply_file = "/tmp/web/wait_apply";

	$wait_apply = '';
	$fh = lock_file($wait_apply_file);
	if(file_exists($wait_apply_file)) {
		$wait_apply = file_get_contents($wait_apply_file);
	}

	if(!strstr($wait_apply, "$cmd_id@$arg1@$arg2\n")) {
		$handle = fopen($wait_apply_file, "a");
		fwrite($handle, "$cmd_id@$arg1@$arg2\n");
		fclose($handle);
	}
	unlock_file($fh);
}

/**************************************************/

function save_httpd_to_slave()
{
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	
	//$local_filepath = "/etc/asterisk/gw/httpd.conf";
	//$remote_filepath = "/etc/asterisk/gw/httpd.conf";
	$local_filepath = "/etc/asterisk/gw/lighttpdpassword_digest";
	$remote_filepath = "/etc/asterisk/gw/lighttpdpassword_digest";

	$cluster_info = get_cluster_info();
	if($cluster_info['mode'] == 'master') {
		for($b=2; $b<=$__BRD_SUM__; $b++) {
			if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
				$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
				set_slave_file($slaveip,$local_filepath,$remote_filepath);
				//wait_apply("request_slave", $slaveip, "syscmd:/etc/init.d/httpd restart > /dev/null 2>&1 &");
				wait_apply("request_slave", $slaveip, "syscmd:/etc/init.d/lighttpd restart > /dev/null 2>&1 &");
			}   
		}   
	}   
}

function trace_output_start($title, $head, $line=false)
{
	$output  = "<b style='margin-top:30px;display:block;'>$title</b>";
	if($line) $output .= "<hr/>";
	$output .= "<table class='table_show'>";
	$output .= "<tr>";
	$output .= "<th><b>$head</b>";
	echo $output;
	ob_flush();
	flush();
}

function trace_output_newline()
{
	$output  = "</th></tr>";
	$output .= "<tr><td style='text-align:left'>";
	echo $output;
	ob_flush();
	flush();
}

function trace_output_newhead($head)
{
	$output  = "</td></tr>";
	$output .= "<tr>";
	$output .= "<th>$head</th>";
	$output .= "<tr><td style='text-align:left'>";
	echo $output;
	ob_flush();
	flush();
}


function trace_output_end()
{
	echo "</td></tr></table>";
	ob_flush();
	flush();
}

/**************************************************/

function insert_html_report($contents,$head=false,$container_id=NULL)
{
	if($container_id==NULL){
		$id = 'html_report';
	}else{
		$id = $container_id;
		echo "<div class='div_report' id='$id'></div>";
	}
	echo "<div id='insert_html_report'></div>";
	?>
	<script type="text/javascript" language="javascript">
	var text = document.createTextNode("<?php echo $contents;?>");
	if(document.getElementById("<?php echo $id;?>")){
		var html_report = document.getElementById("<?php echo $id;?>");
	}else{
		var insert_html_report = document.getElementById("insert_html_report");
		var html_report = document.createElement("div");
		html_report.id = '<?php echo $id;?>';
		html_report.className = 'div_report';
		insert_html_report.appendChild(html_report);
	}

	<?php if($head){ ?>
		var html_report_head = document.createElement("div");
		html_report_head.className = 'div_report_head';
		html_report_head.appendChild(text);
		html_report.appendChild(html_report_head);
	<?php }else{ ?>
		var html_report_contents = document.createElement("div");
		html_report_contents.className = 'div_report_contents';
		html_report_contents.appendChild(text);
		html_report.appendChild(html_report_contents);
	<?php } ?>

	</script>
	<?php
}

/********************************************************/  /* AST cli command */

function get_state($channel, $slaveip=NULL)
{
	if($slaveip ==NULL ){
		exec("asterisk -rx \"gsm show span $channel\" 2>/dev/null", $ret);
	}else{
		$data = "astcmd:gsm show span $channel";
		$ret = request_slave($slaveip, $data, 5, true);
	}

	if(is_array($ret)){
		foreach($ret as $value){
			if(strncmp($value, "State: ", 7)==0){
				return trim(substr($value, 7));
			}
		}
	}else if(($pos = strpos($ret, "State: "))!==false){
		$state = trim(substr($ret, $pos+7));
		$state = substr($state, 0, strpos($state, "\n"));
		return trim($state);
	}
	return "";
}

function double_check_state($channel, $state, $check_interval=1, $slaveip=NULL)
{
	if($state == get_state($channel, $slaveip)){
		sleep($check_interval);
		if($state == get_state($channel, $slaveip)){
			return true;
		}
	}
	return false;
}


function write_imei($channel, $imei, $slaveip=NULL, $force)
{
	if(is_true($force)){
		$immediate = 'immediate';
	}else{
		$immediate = '';
	}
	if($slaveip == NULL){
		//exec("asterisk -rx \"gsm send sync at $channel AT+EGMR=1,7,\\\\\\\"$imei\\\\\\\" 10000\" 2>/dev/null", $ret);
		exec("asterisk -rx \"gsm set imei $channel $imei $immediate\" 2>/dev/null", $ret);
	}else{
		//$data = "astcmd:gsm send sync at $channel AT+EGMR=1,7,\\\\\\\"$imei\\\\\\\" 10000";
		$data = "astcmd:gsm set imei $channel $imei $immediate\n";
		$ret = request_slave($slaveip, $data, 5, true);
	}

	sleep(2);	
	/*if(is_array($ret)){
		foreach($ret as $value){
			if(strncmp($value, "OK", 2)==0){
				return true;
			}
		}
	}else if(strpos($ret, "OK")!==false){
		return true;
	}*/
	
	//return false;   //Don't reboot gsm module
	return true;   //Reboot gsm module
}

function read_imei($channel, $slaveip=NULL)
{
	if($slaveip == NULL){
		exec("asterisk -rx \"gsm send sync at $channel AT+CGSN 10000\" 2>/dev/null", $ret);
	}else{
		$data = "astcmd:gsm send sync at $channel AT+CGSN 10000";
		$ret = request_slave($slaveip, $data, 5, true);
	}
	if(is_array($ret)){
		foreach($ret as $value){
			if(preg_match('/^\d{15}/', $value, $output)){
				return trim($output[0]);
			}
		}
	}else if(preg_match('/\d{15}/', $ret, $output)){
		return trim($output[0]);
	}
	return false;
}

function check_netreg($channel, $slaveip=NULL)
{
	if($slaveip == NULL) {
		exec("asterisk -rx \"gsm send sync at $channel AT+CREG? 10000\" 2> /dev/null", $ret);
	} else {
		$data = "astcmd:gsm send sync at $channel AT+CREG? 10000";
		$ret = request_slave($slaveip, $data, 5, true);
	}

	if(is_array($ret)) {
		foreach($ret as $value) {
			if(strncmp($value, "+CREG: 1,1", 10)==0 || strncmp($value, "+CREG: 0,1", 10)==0) {
				return true;
			}
		}
	} else if(strpos($ret, "+CREG: 1,1")!==false || strpos($ret, "+CREG: 0,1")!==false) {
		return true;
	}

	return false;
}

function cfun_open($channel, $slaveip=NULL)
{
	if($slaveip == NULL){
		exec("asterisk -rx \"gsm send sync at $channel AT+CFUN=0 1000\" 2>/dev/null", $ret);
	} else {
		$data = "astcmd:gsm send sync at $channel AT+CFUN=0 10000";
		$ret = request_slave($slaveip, $data, 5, true);
	}

	if(is_array($ret)) {
		foreach($ret as $value) {
			if(strncmp($value, "OK", 2)==0) {
				return true;
			}
		}
	} else if(strpos($ret, "OK")!==false) {
		return true;
	} else {
		return false;
	}
}

function cfun_close($channel, $slaveip=NULL)
{
	if($slaveip == NULL){
		exec("asterisk -rx \"gsm send sync at $channel AT+CFUN=1 1000\" 2>/dev/null", $ret);
	}else{
		$data = "astcmd:gsm send sync at $channel AT+CFUN=1 10000";
		$ret = request_slave($slaveip, $data, 5, true);
	}

	if(is_array($ret)) {
		foreach($ret as $value) {
			if(strncmp($value, "OK", 2)==0) {
				return true;
			}
		}
	} else if(strpos($ret, "OK")!==false) {
		return true;
	}

	return false;
}

function cfun_reset($channel, $slaveip=NULL)
{
	if(cfun_close($channel, $slaveip)){
		sleep(30);
		if(cfun_open($channel, $slaveip)){
			return true;
		}
	}
	return false;
}

function gsm_power_reset($channel, $slaveip=NULL)
{
	if($slaveip == NULL){
		exec("asterisk -rx \"gsm power reset $channel\"", $ret);
	} else {
		$data = "astcmd:gsm power reset $channel";
		$ret = request_slave($slaveip, $data, 5, true);
	}

	//sleep(30);
	if(is_array($ret)){
		foreach($ret as $value){
			if(strpos($value, "success")!==false){
				return true;
			}
		}
	} else if(strpos($ret, "success")!==false) {
		return true;
	}
	return false;
}

/*************************************************
 *	function: modify_imei()
 *
 *	reset_mode:
 *		1.power_reset(default);
 *		2.cfun_reset;
 *	force:
 *		1.yes(default, check for channel ready);
 *		2.no(no check);
 *************************************************/
function modify_imei($channel, $new_imei, $reset_mode='power_reset', $force='no', $slaveip=NULL)
{
	if(!write_imei($channel, $new_imei, $slaveip, $force))
		return 'write imei error';
	if($reset_mode == 'power_reset' && is_true($force)){
		if (!gsm_power_reset($channel, $slaveip))
			return 'Force Mode: gsm power reset error';
	}
	if($reset_mode == 'cfun_reset' && is_true($force) && !cfun_reset($channel, $slaveip))
		return 'cfun reset error';

	for($i = 0; $i < 10; $i ++){
		if($new_imei == read_imei($channel, $slaveip))
			break;
		sleep(10);
	}
	
	if($i == 10)
		return 'imei read is not equal to given';

	if($reset_mode == 'power_reset' && !is_true($force) && !gsm_power_reset($channel, $slaveip))
		return 'No-Force Mode: gsm power reset error';
		
	for($i = 0; $i < 10; $i ++){
		if(check_netreg($channel, $slaveip))
			break;
		sleep(10);
	}
	
	if($i==10)
		return 'net registeration error';
	
	return 'succeed';
}

function get_imei($port, $slaveip=NULL)
{
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	global $__GSM_HEAD__;
	global $__GSM_SUM__;
	$imei = ""; 

	if($slaveip==NULL){
		$run = false;
		if(ast_running()) {
			$run = true;
		}   

		if($run) {
			$handle = @fopen("/tmp/gsm/$port","r");
			@flock($handle,LOCK_EX);
			$states = @fread($handle, @filesize("/tmp/gsm/$port"));
			@flock($handle,LOCK_UN);
			@fclose($handle);
		} else {
			$states = ''; 
		}   
		//$imei = trim(__cut_str($states,"Model IMEI:","Revision: "));
		$imei = trim(__cut_str($states,"Model IMEI:","\n"));
	}else{
		$cluster_info = get_cluster_info();
		if($cluster_info['mode'] == 'master') {
			for($b=2; $b<=$__BRD_SUM__; $b++){
				if($cluster_info[$__BRD_HEAD__.$b.'_ip'] == $slaveip){
					$all_states = request_slave($slaveip,"get_spans\n", 1); 
					$states = __cut_str($all_states,"---Start Span $port---","---End Span $port---");
					//$imei = trim(__cut_str($states,"Model IMEI:","Revision: "));
					$imei = trim(__cut_str($states,"Model IMEI:","\n"));
				}
			}
		}
	}   
	return $imei;
}

function get_conf($conf_path)
{
	$conf_dir = dirname($conf_path);
	$conf_file = basename($conf_path);

	if(!file_exists($conf_path)) {
		return false;
	}   

	require_once("/www/cgi-bin/inc/aql.php");
	$aql = new aql();
	$setok = $aql->set('basedir',$conf_dir);
	if (!$setok) {
		echo __LINE__.' '.$aql->get_error();
		return false;
	}   

	$hlock=lock_file($conf_path);
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}   
	$conf_array = $aql->query("select * from $conf_file");
	unlock_file($hlock);

	return $conf_array;
}

function modify_conf($conf_path,$conf_array_new)
{
	$conf_dir = dirname($conf_path);
	$conf_file = basename($conf_path);

	if(!file_exists($conf_path)) {
		fclose(fopen($conf_path,"w"));
	}   

	require_once("/www/cgi-bin/inc/aql.php");
	$aql = new aql();
	$setok = $aql->set('basedir',$conf_dir);
	if (!$setok) {
		echo __LINE__.' '.$aql->get_error();
		return false;
	}   

	$hlock=lock_file($conf_path);
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}   
	$conf_array_old = $aql->query("select * from $conf_file");
	if(is_array($conf_array_new)){
		foreach($conf_array_new as $section => $value){
			if(!isset($conf_array_old[$section])) {
				$aql->assign_addsection($section,'');
			}   
			if(is_array($conf_array_new[$section])){
				foreach($conf_array_new[$section] as $key => $value){
					if(isset($conf_array_old[$section][$key]))
						$aql->assign_editkey($section,$key,$value);
					else
						$aql->assign_append($section,$key,$value);
				}   
			}           
		}   
	}   
	if (!$aql->save_config_file($conf_file)) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}   
	unlock_file($hlock);

	return true;
}

function delete_conf($conf_path,$section, $key)
{
	$conf_dir = dirname($conf_path);
	$conf_file = basename($conf_path);

	if(!file_exists($conf_path)) {
		return false;
	}   

	require_once("/www/cgi-bin/inc/aql.php");
	$aql = new aql();
	$setok = $aql->set('basedir',$conf_dir);
	if (!$setok) {
		echo __LINE__.' '.$aql->get_error();
		return false;
	}   

	$hlock=lock_file($conf_path);
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}   
	$aql->query("alter table $conf_file drop $key where section='$section'");
	if (!$aql->save_config_file($conf_file)) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}   
	unlock_file($hlock);
	return true;
}


function split_sign($string, $sign)
{
	$ret[0] = '';
	$ret[1] = '';

	$res = explode($sign,$string,2);

	if(isset($res[0])) $ret[0] = trim($res[0]);
	if(isset($res[1])) $ret[1] = trim($res[1]);

	return $ret;
}

function js_float_show_time($date=true, $timezone=false)
{
	//if(!($today = @getdate())) return;
	//if(!($tz = @date_default_timezone_get())) return;
	
	if(!($tz = trim(file_get_contents('/tmp/TZ')))) return;
	if(!($all_time = `date "+%Y:%m:%d:%H:%M:%S"`)) return;
	$item = explode(':', $all_time, 6);
	if(isset($item[5])) {
		$today['year'] = $item[0];
		$today['mon'] = $item[1];
		$today['mday'] = $item[2];
		$today['hours'] = $item[3];
		$today['minutes'] = $item[4];
		$today['seconds'] = $item[5];
	}else{
		return;
	}

	$sec = $today['hours']*3600+$today['minutes']*60+$today['seconds'];

	$head = '\'' . language('Server time') . ':' . '\'';

	$show_tz = $timezone ? 'tz + \' \' ' : '\'\'';
	$show_date = $date ? 'Y+\'-\'+M+\'-\'+D+\' \'' : '\'\'';

echo <<<EOF
	<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>
	<style>
	#system_current_time{
		position:fixed;
		top:0px;
		left:0px;
		font-size:13px;
		font-weight:bold;
		background: none no-repeat fixed 0.3em 0.3em rgb(159, 159, 223);
		border:1px solid rgb(57, 57, 174);
		padding:3px;
		z-index:30;

	}
	</style>
	<div id="system_current_time"></div>
	<script type="text/javascript" language="javascript">
	\$(function(){
		currentTime($today[year],$today[mon],$today[mday],$sec,"$tz");
		\$("#system_current_time").draggable();
		\$("#system_current_time").mousedown(function(){
			\$(this).css('cursor','url(/images/closehand.cur),auto');
		});
		\$("#system_current_time").mouseup(function(){
			\$(this).css('cursor','url(/images/openhand.cur),auto');
		});
		\$("#system_current_time").mouseover(function(){
			\$(this).css('cursor','url(/images/openhand.cur),auto');
		});
	});

	function currentTime(Y,M,D,sec,tz) {
		sec++;
		H=Math.floor(sec/3600)%24;
		I=Math.floor(sec/60)%60;
		S=sec%60;
		if(S<10) S='0'+S;
		if(I<10) I='0'+I;
		if(H<10) H='0'+H;
		if (H=='00' & I=='00' & S=='00') D=D+1;
		if (M==2) {
			if (Y%4==0 && !Y%100==0 || Y%400==0) {
				if (D==30){
					M+=1;D=1;
				}
			} else {
				if (D==29) {
					M+=1;D=1;
				}
			}
		} else {
			if (M==4 || M==6 || M==9 || M==11) {
				if (D==31) {
					M+=1;D=1;
				}
			} else {
				if (D==32) {
					M+=1;D=1;
				}
			}
		}

		if (M==13) {
			Y+=1;M=1;
		}

		document.getElementById("system_current_time").innerHTML = $head + $show_tz + $show_date + H + ':' + I + ':' + S;

		setTimeout("currentTime("+Y+","+M+","+D+","+sec+",'"+tz+"')", 1000);
	}
	</script>
EOF;
//EOF
}


function get_gsm_info($slaveip = '')
{
	global $__GSM_SUM__;
	global $__GSM_HEAD__;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;

/* 
Format:  
---Start Span 1---
D-channel: 2
Status: Power on, Provisioned, Up, Active, Standard
Type: CPE
Manufacturer: SIMCOM_Ltd
Model Name: SIMCOM_SIM840W
Model IMEI: 353511028445509
Revision: Revision:1116B01SIM840W16_MXIC_OPENVOX
Network Name: CHINA MOBILE
Network Status: Registered (Home network)
Signal Quality (0,31): 15
BER value (0,7): 0
SIM IMSI: 460028893042082
SIM SMS Center Number: +8613800755500
Remain Time: 0
PDD: 0
ASR: 0
ACD: 0
Last event: D-Channel Up
State: READY
Last send AT:
---End Span 1---
*/
	for($i=1; $i<=$__GSM_SUM__; $i++) {
		$states[$i] = '';
	}

	if($slaveip) {
		$all_states = request_slave($slaveip, "get_spans\n", 1);
		for($i=1; $i<=$__GSM_SUM__; $i++) {
			$states[$i] = __cut_str($all_states,"---Start Span $i---","---End Span $i---");
		}
	} else {
		if(ast_running()) {
			for($i=1; $i<=$__GSM_SUM__; $i++) {
				$handle = @fopen("/tmp/gsm/$i","r");
				@flock($handle,LOCK_EX);
				$states[$i] = @fread($handle, @filesize("/tmp/gsm/$i"));
				@flock($handle,LOCK_UN);
				@fclose($handle);
			}
		}
	}

	$i = 1;
	foreach($states as $state) {
		/* get all org data */
		$ret[$i]['port']                         = $i;
		/*$ret[$i]['d-channel']                    = trim(__cut_str($state,"D-channel:","Status:"));
		$ret[$i]['status']                       = trim(__cut_str($state,"Status:","Type:"));
		$ret[$i]['type']                         = trim(__cut_str($state,"Type:","Manufacturer:"));
		$ret[$i]['manufacturer']                 = trim(__cut_str($state,"Manufacturer:","Model Name:"));
		$ret[$i]['model_name']                   = trim(__cut_str($state,"Model Name:","Model IMEI:"));
		$ret[$i]['model_imei']                   = trim(__cut_str($state,"Model IMEI:","Revision:"));
		$ret[$i]['revision']                     = trim(__cut_str($state,"Revision:","Network Name:"));
		$ret[$i]['network_name']                 = trim(__cut_str($state,"Network Name:","Network Status:"));
		$ret[$i]['operator']                     = trim(__cut_str($state,"Network Name:","Network Status:"));
		$ret[$i]['network_status']               = trim(__cut_str($state,"Network Status:","Signal Quality (0,31):"));
		$ret[$i]['register']                     = trim(__cut_str($state,"Network Status:","Signal Quality (0,31):"));
		$ret[$i]['signal_quality']               = trim(__cut_str($state,"Signal Quality (0,31):","BER value (0,7):"));
		$ret[$i]['signal']                       = trim(__cut_str($state,"Signal Quality (0,31):","BER value (0,7):"));
		$ret[$i]['ber']                          = trim(__cut_str($state,"BER value (0,7):","SIM IMSI:"));
		$ret[$i]['sim_imsi']                     = trim(__cut_str($state,"SIM IMSI:","SIM SMS Center Number:"));
		$ret[$i]['sim_sms_center_number']        = trim(__cut_str($state,"SIM SMS Center Number:","Own Number:"));
		$ret[$i]['own_number']                   = trim(__cut_str($state,"Own Number:","Remain Time:"));
		$ret[$i]['remain_time']                  = trim(__cut_str($state,"Remain Time:","PDD:"));
		$ret[$i]['pdd']                          = trim(__cut_str($state,"PDD:","ASR:"));
		$ret[$i]['asr']                          = trim(__cut_str($state,"ASR:","ACD:"));
		$ret[$i]['acd']                          = trim(__cut_str($state,"ACD:","Last event:"));
		$ret[$i]['last_event']                   = trim(__cut_str($state,"Last event:","State:"));
		$ret[$i]['state']                        = trim(__cut_str($state,"State:","Last send AT:"));
		$ret[$i]['last_send_at']                 = trim(__cut_str($state,"Last send AT:",'\r\n'));
		*/

		$ret[$i]['d-channel']                    = trim(__cut_str($state,"D-channel:","\n"));
		$ret[$i]['status']                       = trim(__cut_str($state,"Status:","\n"));
		$ret[$i]['type']                         = trim(__cut_str($state,"Type:","\n"));
		$ret[$i]['manufacturer']                 = trim(__cut_str($state,"Manufacturer:","\n"));
		$ret[$i]['model_name']                   = trim(__cut_str($state,"Model Name:","\n"));
		$ret[$i]['model_imei']                   = trim(__cut_str($state,"Model IMEI:","\n"));
		
		//$ret[$i]['revision']                     = trim(__cut_str($state,"Revision:","\n"));
		$revision_temp = exec("grep -rn ^Revision /tmp/gsm/$i");
		$revision_temp = explode('Revision:', $revision_temp);
		$ret[$i]['revision'] 					 = trim($revision_temp[1]);
		
		$ret[$i]['network_name']                 = trim(__cut_str($state,"Network Name:","\n"));
		$ret[$i]['operator']                     = trim(__cut_str($state,"Network Name:","\n"));
		$ret[$i]['network_status']               = trim(__cut_str($state,"Network Status:","\n"));
		$ret[$i]['register']                     = trim(__cut_str($state,"Network Status:","\n"));
		$ret[$i]['signal_quality']               = trim(__cut_str($state,"Signal Quality (0,31):","\n"));
		$ret[$i]['signal']                       = trim(__cut_str($state,"Signal Quality (0,31):","\n"));
		$ret[$i]['ber']                          = trim(__cut_str($state,"BER value (0,7):","\n"));
		$ret[$i]['sim_imsi']                     = trim(__cut_str($state,"SIM IMSI:","\n"));
		$ret[$i]['sim_sms_center_number']        = trim(__cut_str($state,"SIM SMS Center Number:","\n"));
		$ret[$i]['own_number']                   = trim(__cut_str($state,"Own Number:","\n"));
		$ret[$i]['remain_time']                  = trim(__cut_str($state,"Remain Time:","\n"));
		$ret[$i]['pdd']                          = trim(__cut_str($state,"PDD:","\n"));
		$ret[$i]['asr']                          = trim(__cut_str($state,"ASR:","\n"));
		$ret[$i]['acd']                          = trim(__cut_str($state,"ACD:","\n"));
		$ret[$i]['last_event']                   = trim(__cut_str($state,"Last event:","\n"));
		$ret[$i]['state']                        = trim(__cut_str($state,"State:","\n"));
		$ret[$i]['last_send_at']                 = trim(__cut_str($state,"Last send AT:",'\r\n'));


		/* process data which will be shown */
		$ret[$i]['show_status'] = '';
		if(strstr($ret[$i]['status'],"Power off")) {
			$ret[$i]['show_status'] = "<img src=\"../../images/nosim.gif\"/ height=\"22\">";
		} else if(strstr($ret[$i]['status'],"Power on, Provisioned, Down")) {
			$ret[$i]['show_status'] = "<img src=\"../../images/nosim.gif\"/ height=\"22\">";
		} else if(strstr($ret[$i]['status'],"Undetected SIM Card")) {
			$ret[$i]['operator'] = "";
			$ret[$i]['register'] = "Undetected SIM Card";
			//$ret[$i]['State'] = "";
			$ret[$i]['state'] = "";
			$ret[$i]['show_status'] = "<img src=\"../../images/nosim.gif\"/>";
		} else if(strstr($ret[$i]['status'],"Power on, Provisioned, Up")) {
			if($ret[$i]['signal'] <= 0) {
				$ret[$i]['show_status'] = "<img src=\"../../images/wifi0.gif\"/ height=\"22\">";
			} else if ($ret[$i]['signal'] <= 5) {
				$ret[$i]['show_status'] = "<img src=\"../../images/wifi1.gif\"/ height=\"22\">";
			} else if ($ret[$i]['signal'] <= 10) {
				$ret[$i]['show_status'] = "<img src=\"../../images/wifi2.gif\"/ height=\"22\">";
			} else if ($ret[$i]['signal'] <= 15) {
				$ret[$i]['show_status'] = "<img src=\"../../images/wifi3.gif\"/ height=\"22\">";
			} else if ($ret[$i]['signal'] <= 20) {
				$ret[$i]['show_status'] = "<img src=\"../../images/wifi4.gif\"/ height=\"22\">";
			} else if ($ret[$i]['signal'] <= 31) {
				$ret[$i]['show_status'] = "<img src=\"../../images/wifi5.gif\"/ height=\"22\">";
			} else {
				$ret[$i]['show_status'] = "<img src=\"../../images/wifi0.gif\"/ height=\"22\">";
			}
		}
		if($ret[$i]['remain_time'] == ''){
			$ret[$i]['remain_time'] = 'No Limit';
		}

		$ret[$i]['slaveip'] = $slaveip;
		$i++;
	}

	return $ret;
}

function get_all_gsm_info()
{
	global $__GSM_SUM__;
	global $__GSM_HEAD__;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;

	$alldata[1] = get_gsm_info();

	$cluster_info = get_cluster_info();
	if($cluster_info['mode'] == 'master') {
		for($b=2; $b<=$__BRD_SUM__; $b++) {
			if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
				$alldata[$b] = get_gsm_info($cluster_info[$__BRD_HEAD__.$b.'_ip']);
			}
		}
	}

	return $alldata;
}

// rewrite history:
// 		1. get password from web_server.conf instead of lighttpdpassword 
// 		2. date: 2016/3/10
function get_web_user()
{
	//$file_path = '/etc/asterisk/gw/httpd.conf';
	/*
	$file_path = '/etc/asterisk/gw/lighttpdpassword';
	$flock=lock_file($file_path);
	$fh = fopen($file_path,"r");
	$line = fgets($fh);
	fclose($fh);
	unlock_file($flock);
	$auth_array = explode(":", $line);
	 */


	$file_path = '/etc/asterisk/gw/web_server.conf';
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$hlock = lock_file($file_path);
	$res = $aql->query("select * from web_server.conf");
	unlock_file($hlock);

	if(isset($res['general']['username']) && isset($res['general']['password'])) {
		$user['name'] = trim($res['general']['username']);
		$user['password'] = trim($res['general']['password']);
	} else {
		$user['name'] = "admin";
		$user['password'] = "admin";
	}
	
	if(isset($res['general']['port'])) {
		$user['port'] = trim($res['general']['port']);
	} else {
		$user['port'] = 80;
	}

/*	if(isset($auth_array[2])){
		$user['name'] = trim($auth_array[1]);
		$user['password'] = trim($auth_array[2]);
	} else {
		$user['name'] = "admin";
		$user['password'] = "admin";
	}
*/
/*
	if(isset($auth_array[1])){
		$user['name'] = trim($auth_array[0]);
		$user['password'] = trim($auth_array[1]);
	} else {
		$user['name'] = "admin";
		$user['password'] = "admin";
	}
 */

	return $user;
}


/***********************************************************
 *
 *	Function: http_send
 *	1. Get data
 *		1.1 get file contents (to string);
 *		1.2 wait apply.
 *	2. Post data
 *		2.1 Upload file (eg. system update);
 *		2.2 Post data to slave.
 *	3. Downlaod
 *
 ***********************************************************/
function http_send($url,$username='',$password='',$post='',$download='')
{
	$ch = curl_init();
	//slave port only use 80
	$slaveport = 80;

	curl_setopt($ch, CURLOPT_URL, $url);
	//curl_setopt($ch, CURLOPT_PORT, $_SERVER['SERVER_PORT']);
	curl_setopt($ch, CURLOPT_PORT, $slaveport);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	if($username!=''&&$password!=''){
		curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
		curl_setopt($ch, CURLOPT_USERPWD, $username.':'.$password);
	}

	//GET or POST (or upload)
	if($post!=''){
		curl_setopt($ch, CURLOPT_POST, 1 );
		curl_setopt($ch, CURLOPT_POSTFIELDS, $post);
	}
	//Download
	$fh = false;
	if($download!=''){
		if($fh = fopen($download,'w')){
			curl_setopt($ch, CURLOPT_FILE, $fh);
		}
	}

	curl_setopt($ch, CURLOPT_HTTPHEADER, array('Expect:'));//For Lighttpd 2013-12-24 09:18

	$buf = curl_exec( $ch );
	if ($error = curl_error($ch) ) {
		curl_close($ch);
		if($fh)fclose($fh);
		return '';
	}

	$http_code = curl_getinfo($ch,CURLINFO_HTTP_CODE);
	curl_close($ch);
	if($fh)fclose($fh);
	if($http_code == 200)
		return $buf;
	else
		return '';
}

function gw_upload_file($ip,$remote_path,$local_path)
{
	$user = get_web_user();
	$post['send'] = 'File Upload';
	$post['path'] = $remote_path;

	//Freedom 2013-12-12 10:59
	//PHP-5.3.10
	//$post['file'] = '@'.$local_path;
	//PHP-5.5.6
	$post['file'] = curl_file_create($local_path);

	$url = 'http://'.$ip.'/cgi-bin/php/file_server.php';
	$buf = http_send($url,$user['name'],$user['password'],$post);

	if(strpos($buf,"File Uploaded Successfully")!==false) {
		return true;
	} else {
		return false;
	}
}

function gw_download_file($ip,$remote_path,$local_path)
{
	$user = get_web_user();
	$url = 'http://'.$ip.$remote_path;
	return http_send($url,$user['name'],$user['password'],'',$local_path);
}

function gw_send_GET($url)
{
	$user = get_web_user();
	return http_send($url,$user['name'],$user['password']);
}

function gw_send_POST($url,$post)
{
	$user = get_web_user();
	return http_send($url,$user['name'],$user['password'],$post);
}

function get_remain_time_and_auto_reset_date($channel,$slaveip,$board)
{
/*
path: /data/diallimit/gsm/<gsm port>
format:
line1: remain_time
line2: auto_reset_date
*/
	
	$buf = '';
	$ret['remain_time'] = '';
	$ret['auto_reset_date'] = '';
	$file_path = "/data/diallimit/gsm/$channel";

	if($board == 1){	//$slaveip==''
		if(is_file($file_path)){
			$buf = file_get_contents($file_path);
		}
	}else{
		$urlfile = 'http://'.$slaveip.$file_path;
		$buf = gw_send_GET($urlfile);
	}

	$line_array = explode("\n",$buf);
	if(isset($line_array[1])){
		$remain_time = trim($line_array[0]);
		$auto_reset_date = trim($line_array[1]);
		if(preg_match('/[\d]{1,6}/',$remain_time,$match)){
			if(isset($match[0]))
				$ret['remain_time'] = $match[0];
		}
		if(preg_match("/\d{4}-\d{2}-\d{2}-\d{2}:\d{2}:\d{2}/",$auto_reset_date,$match)){
			if(isset($match[0]))
				$ret['auto_reset_date'] = $match[0];
		}
	}

	return $ret;
}

function set_remain_time_and_auto_reset_date($channel,$remain_time,$auto_reset_date)
{
/*
path: /data/diallimit/gsm/<gsm port>
format:
line1: remain_time
line2: auto_reset_date
*/

	//If only modify one parameter, Another parameter should be retained.
	$ret = get_remain_time_and_auto_reset_date($channel,'',1);
	if($remain_time=='' && isset($ret['remain_time']))		$remain_time=$ret['remain_time'];
	if($auto_reset_date=='' && isset($ret['auto_reset_date']))	$auto_reset_date=$ret['auto_reset_date'];

	//Content is the same. Unnecessary to write.
	if(isset($ret['remain_time']) && $remain_time === $ret['remain_time'] &&
		isset($ret['auto_reset_date']) && $auto_reset_date === $ret['auto_reset_date']){
		return true;
	}
	if($remain_time=='' && $auto_reset_date==''){
		return true;
	}

	//check dir
	if(is_file("/data/diallimit") || !is_dir("/data/diallimit")&&!mkdir("/data/diallimit")){
		return false;
	}
	if(is_file("/data/diallimit/gsm") || !is_dir("/data/diallimit/gsm")&&!mkdir("/data/diallimit/gsm")){
		return false;
	}
	$file_path = "/data/diallimit/gsm/$channel";

	//write file
	$handle = @fopen($file_path,"a");
	@flock($handle,LOCK_EX);
	if(@ftruncate($handle,0)){
		@fwrite($handle, $remain_time."\n".$auto_reset_date);
	}
	@flock($handle,LOCK_UN);
	@fclose($handle);
}

function parse_dial_pattern($dial_pattern)
{
	$val = trim($dial_pattern);
	$dp = explode(',',$val);
	if(is_array($dp) && isset($dp[0]) && $dp[0]) {
		$i = 0;
		foreach($dp as $each) {
			$tmp = explode('|',$each);
			if(isset($tmp[0])) {
				$ret[$i]['prepend'] = trim($tmp[0]);
			} else {
				$ret[$i]['prepend'] = '';
			}
			if(isset($tmp[1])) {
				$ret[$i]['prefix'] = trim($tmp[1]);
			} else {
				$ret[$i]['prefix'] = '';
			}
			if(isset($tmp[2])) {
				$ret[$i]['pattern'] = trim($tmp[2]);
			} else {
				$ret[$i]['pattern'] = '';
			}
			if(isset($tmp[3])) {
				$ret[$i]['cid'] = trim($tmp[3]);
			} else {
				$ret[$i]['cid'] = '';
			}
			$i++;
		}
	}

	if(isset($ret)) {
		return $ret;
	}

	return false;
}

function parse_time_pattern($time_pattern)
{
	$val = trim($time_pattern);
	$tp = explode(',',$val);
	if(is_array($tp) && isset($tp[0]) && $tp[0]) {
		$i = 0;
		foreach($tp as $each) {
			$tmp = explode('|',$each);
			if(isset($tmp[0])) {
				$val = split_sign($tmp[0],'-');
				$ret[$i]['stime'] = $val[0];
				$ret[$i]['etime'] = $val[1];
			} else {
				$ret[$i]['stime'] = '';
				$ret[$i]['etime'] = '';
			}
			if(isset($tmp[1])) {
				$val = split_sign($tmp[1],'-');
				$ret[$i]['sweek'] = $val[0];
				$ret[$i]['eweek'] = $val[1];
			} else {
				$ret[$i]['sweek'] = '';
				$ret[$i]['eweek'] = '';
			}
			if(isset($tmp[2])) {
				$val = split_sign($tmp[2],'-');
				$ret[$i]['sday'] = $val[0];
				$ret[$i]['eday'] = $val[1];
			} else {
				$ret[$i]['sday'] = '';
				$ret[$i]['eday'] = '';
			}
			if(isset($tmp[3])) {
				$val = split_sign($tmp[3],'-');
				$ret[$i]['smonth'] = $val[0];
				$ret[$i]['emonth'] = $val[1];
			} else {
				$ret[$i]['smonth'] = '';
				$ret[$i]['emonth'] = '';
			}
			$i++;
		}
	}

	if(isset($ret)) {
		return $ret;
	}

	return false;
}

function get_self() 
{ 
	$pos = strpos($_SERVER['REQUEST_URI'], '?');
	if($pos!==false)
		return substr($_SERVER['REQUEST_URI'], 0, $pos); 
	else
		return $_SERVER['REQUEST_URI'];
}


function webserver_restart()
{
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	$webserver_start_script = '/etc/init.d/lighttpd';

	if(!file_exists($webserver_start_script)){
		return false;
	}

	$info = get_cluster_info();
	if($info['mode'] == 'master') {
		for($b=2; $b<=$__BRD_SUM__; $b++) {
			if(isset($info[$__BRD_HEAD__.$b.'_ip']) && $info[$__BRD_HEAD__.$b.'_ip'] != '') {
				$slaveip = $info[$__BRD_HEAD__.$b.'_ip'];
				request_slave($slaveip, "syscmd:$webserver_start_script restart > /dev/null 2>&1", 3, false);
			}
		}
	}

	//exec("$webserver_start_script restart > /dev/null 2>&1");

	exec("$webserver_start_script restart >> /tmp/lighttpd.log 2>&1");
}

function get_slotnum() 
{
	// $cluster_info = get_cluster_info();	
	// if ($cluster_info['mode'] == 'stand_alone')  {
		// return 1;
	// }
	$board = trim(file_get_contents('/tmp/.slotnum')); 
	if($board == 0){
		$board = 1;
	}
	if(is_numeric($board)){ 
		return $board; 
	} 

	return 1; 
}

function get_mnp_info()
{
	static $MNP_INFO = NULL;
	if(isset($MNP_INFO)){
		return $MNP_INFO;
	}
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk/gw');
	$res = $aql->query("select * from mnp.conf");
	
	$info = array();
	if(isset($res['general']['mnp_enable'])) {
		$info['enable'] = trim($res['general']['mnp_enable']);
 	}else{
 		$info['enable'] = '0';
 	}
	
	if(isset($res['general']['mnp_url'])) {
		$info['url'] = trim($res['general']['mnp_url']);
	} else {
		$info['url'] = '';
	}
		
	if(isset($res['general']['mnp_timeout'])) {
		$info['timeout'] = trim($res['general']['mnp_timeout']);
		if(!is_numeric($info['timeout'])) {
			$info['timeout'] = '5';
		}

		if($info['timeout'] <= 0) {
			$info['timeout'] = '5';
		}
	} else {
		$info['timeout'] = '5';
	}

	if(isset($res['general']['mnp_manipulation'])) {
		$info['manipulation'] = trim($res['general']['mnp_manipulation']);
	} else {
		$info['manipulation'] = 'before';
	}

	$MNP_INFO = $info;
	return $MNP_INFO;
}

function show_modify_imei_dialog()
{
	global $__BRD_HEAD__;
	global $__GSM_HEAD__;
?>
	<script type="text/javascript">
	function modify_imei(board, span, old_imei)
	{
		var server_file = "ajax_server.php";

		$("#manual_modify_old_imei").html(old_imei);
		$("#manual_modify_new_imei").attr("value",old_imei);
		$("#manual_modify_imei").dialog({
			resizable: false,
			height: 230,
			width: 500,
			modal: true,
			dialogClass: "jquery_dialog",
			buttons: {
				"Modify": function() {
					var force_sw = $("#manual_modify_force_sw").attr("checked")?"on":"off";
					var new_imei = $("#manual_modify_new_imei").attr("value");
					if(new_imei == ''){
						alert("No IMEI input!");
						return false;
					}

					$.ajax({
						url: server_file+"?random="+Math.random(),      //request file;
						type: 'GET',                                    //request type: 'GET','POST';
						dataType: 'text',                               //return data type: 'text','xml','json','html','script','jsonp';
						data: {
							"type":"gsm",
							"gsm_type":"modify_imei",
							"board":board,
							"channel":span,
							"new_imei":new_imei,
							"force":force_sw
						},
						error: function(data){},                        //request failed callback function;
						success: function(data){},                      //request success callback function;
						complete: function(){}
					});
					$( this ).dialog( "close" );
					//alert("Please be patient to wait for IMEI modify , about 1 minute");
				},
				Cancel: function() {
					$( this ).dialog( "close" );
				}
			}
		});
	}
	</script>

	<div id="manual_modify_imei" title="Manual Modify IMEI" style="display:none;">
		<?php echo language("Old IMEI");?>:&nbsp;&nbsp;<span id="manual_modify_old_imei"></span>.&nbsp;&nbsp;&nbsp;
		<?php echo language("Please input new IMEI");?>:<br><br>
		<center>
			<input type="text" id="manual_modify_new_imei" style="width:300px;" />
		</center>
		<br>
		<input id="manual_modify_force_sw" type="checkbox" checked/><?php echo language('Force Modify IMEI notice','Force modify even if the port is not ready.');?>
	</div>
<?php
}

function save_cluster_localnet()
{
	global $__BRD_SUM__;
	global $__BRD_HEAD__;

	$cluster_info = get_cluster_info();

        $aql = new aql();
        $setok = $aql->set('basedir','/etc/asterisk');
        if (!$setok) {
                echo $aql->get_error();
                return;
        }    

	$file_path = '/etc/asterisk/sip_general.conf';
	$hlock = lock_file($file_path);
	if(!$aql->open_config_file($file_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	$exist_array = $aql->query("select * from sip_general.conf");
	unlock_file($hlock);

	if(isset($exist_array['general']['externaddr']) 
		&& $exist_array['general']['externaddr'] != '' 
		&& $cluster_info['mode'] == 'master'
	){
		$contents = '';
		if(isset($cluster_info['master_ip']) && $cluster_info['master_ip'] != ''){ 
			$contents .= 'localnet='.trim($cluster_info['master_ip']).'/255.255.255.255'."\n";
		}    
		for($b = 2; $b <= $__BRD_SUM__; $b++){
			if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
				$contents .= 'localnet='.trim($cluster_info[$__BRD_HEAD__.$b.'_ip']).'/255.255.255.255'."\n";
			}
		}

		$file_path = '/etc/asterisk/sip_cluster_localnet.conf';
		$hlock = lock_file($file_path);
		file_put_contents($file_path, $contents);
		unlock_file($file_path);
	}else{
		$file_path = '/etc/asterisk/sip_cluster_localnet.conf';
		$hlock = lock_file($file_path);
		fclose(fopen($file_path, 'w'));
		unlock_file($file_path);
	}
}
function get_gsm_module_info(){

	$aql = new aql();
    $setok = $aql->set('basedir','/etc/asterisk');
    if (!$setok) {
            echo $aql->get_error();
            return;
    }    

	$file_path = '/etc/asterisk/gw_gsm.conf';
	$hlock = lock_file($file_path);
	if(!$aql->open_config_file($file_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	$gsm_modules_array = $aql->query("select * from gw_gsm.conf");
	unlock_file($hlock);
	echo "<pre>";
	//print_r($gsm_modules_array);
	echo "</pre>";
	return $gsm_modules_array;
}

function get_sms_info_for_smssend(){
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$res = $aql->query("select * from sms.conf");

	return $res;
}

function get_sms_info()
{
/*
[send]
attempt=0
repeat=1
verbose=3

[mail]
sw=on
sender=
smtpserver=
smtpport=
smtpuser=
smtppwd=
smail1=
smail2=
smail3=
tls_ssl=
mail_title=
mail_content=

[control]
sw=on
password=

[http_to_sms]
enable=on
use_web_server_user=on
username=admin
password=admin
port=
report=json
timeout_total=20
debug=255
timeout_wait=20
timeout_gsm_send=10000
timeout_socket=2

[sms_to_http]
enable=on
url_host=host
url_port=port
url_path=/path
url_from_num=key
url_to_num=key
url_message=key
url_time=key
url_user_defined=
url=http://host:port/path?key=${from}&key=${to}&key=${message}&key=${time}&
*/
	static $INFO = NULL;
	if(isset($INFO)){
		return $INFO;
	}

	$info = array(
		'send' => array(
			'attempt'	=> 0,
			'repeat'	=> 1,
			'verbose'	=> 3,
		),
                'mail' => array(
			'sw'		=> 'off',
			'sender'	=> '',
			'smtpserver'	=> '',
			'smtpport'	=> '',
			'smtpuser'	=> '',
			'smtppwd'	=> '',
			'smail1'	=> '',
			'smail2'	=> '',
			'smail3'	=> '',
			'tls_ssl'	=> '',
			'mail_title'	=> '',
			'mail_content'	=> '',
                ),
		'control' => array(
			'sw'		=>'off',
			'password'	=> '',
                ),
		'http_to_sms' => array(
			'enable'		=> 'off',
			'use_web_server_user'	=> 'on',
			'username'		=> 'admin',
			'password'		=> 'admin',
			'port'			=> '',
			'report'		=> 'json',
			'timeout_total'		=> '20',
			'debug'			=> '255',
			'timeout_wait'		=> '20',
			'timeout_gsm_send'	=> '10000',
			'timeout_socket'	=> '2',
                ),
		'sms_to_http' => array(
			'enable'		=> 'off',
			'url_host'		=> '',
			'url_port'		=> '',
			'url_path'		=> '',
			'url_from_num'		=> '',
			'url_to_num'		=> '',
			'url_portname'		=> '',
			'url_message'		=> '',
			'url_time'		=> '',
			'url_user_defined'	=> '',
			'url'			=> 'http://host:port/path?key=${from}&key=${to}&key=${message}&key=${time}&',
			'smsreports_to_http_enable' => '',
			'smsresults_to_http_enable' => '',
			'get_smsstatus_enable' => '',
		),                           
		'local_store' => array(
			'enable'		=> 'off',
		),
	);


	$file_path = "/etc/asterisk/gw/sms.conf";

	//return parse_ini_file($file_path);
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$hlock = lock_file($file_path);
	$res = $aql->query("select * from sms.conf");
	unlock_file($hlock);

	foreach($info as $section_key => $section_value){
		foreach($section_value as $key => $value){
			if(isset($res[$section_key][$key]) && $res[$section_key][$key] != ''){
				$info[$section_key][$key] = $res[$section_key][$key];
			}
		}
	}

	$INFO = $info;
	return $INFO;
}

function get_ussd_info(){
	$file_path = "/etc/asterisk/gw/ussd.conf";
	
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	
	$res = $aql->query("select * from ussd.conf");
	
	return $res;
}

function get_web_info()
{
/*
[general]
port=80
username=admin
password=admin

*/
	static $INFO = NULL;
	if(isset($INFO)){
		return $INFO;
	}

	$info = array(
		'general' => array(
			'port' 		=> 80,
			'username' 	=> 'admin',
			'password' 	=> 'admin',
		),
	);	

	$file_path = '/etc/asterisk/gw/web_server.conf';

	//return parse_ini_file($file_path);
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$hlock = lock_file($file_path);
	$res = $aql->query("select * from web_server.conf");
	unlock_file($hlock);

	foreach($info as $section_key => $section_value){
		foreach($section_value as $key => $value){
			if(isset($res[$section_key][$key]) && $res[$section_key][$key] != ''){
				$info[$section_key][$key] = $res[$section_key][$key];
			}
		}
	}

	$INFO = $info;
	return $INFO;
}


/*
 *  sync memory configs file to flash
 *
 */
function save_to_flash($mem_path, $flash_path)
{
	if(is_dir($mem_path)){
		if(!is_dir($flash_path)){
			if(file_exists($flash_path)){
				unlink($flash_path);
			}
			if(!mkdir($flash_path)){
				return false;
			}
		}

		$dir_handle = opendir($mem_path);
		while(($file = readdir($dir_handle)) !== false){
			if($file == '.' || $file == '..') {
				continue;
			}
			//file_put_contents("/tmp/test.txt", "save_to_flash($mem_path.'/'.$file, $flash_path.'/'.$file)\n", FILE_APPEND);
			//echo "save_to_flash($mem_path.'/'.$file, $flash_path.'/'.$file)\n";
			save_to_flash($mem_path.'/'.$file, $flash_path.'/'.$file);
		}   
		closedir($dir_handle);
	}else if(is_file($mem_path)){
		if(is_file($flash_path)){
			clearstatcache();
			if(filesize($mem_path) != filesize($flash_path)){
				/* size is different */
				//file_put_contents("/tmp/test.txt","==========1.copy($mem_path, $flash_path)[".filemtime($mem_path).",".filemtime($flash_path)."]\n",FILE_APPEND);
				//echo "==========1.copy($mem_path, $flash_path)[".filemtime($mem_path).",".filemtime($flash_path)."]\n";
				copy($mem_path, $flash_path);
				touch($mem_path);
			}else{
				/* size is the same */
				if(filemtime($mem_path) != filemtime($flash_path)){
					/* modify time is different */
					$mem_contents = file_get_contents($mem_path);
					$flash_contents = file_get_contents($flash_path);
					if($mem_contents != $flash_contents){
						/* contents is different */
						//file_put_contents("/tmp/test.txt","=========2.copy($mem_path, $flash_path)\n",FILE_APPEND);
						//echo "=========2.copy($mem_path, $flash_path)\n";
						copy($mem_path, $flash_path);
						touch($mem_path);
					}
				/*
				} else {
					file_put_contents("/tmp/test.txt","$mem_path, $flash_path same\n",FILE_APPEND);
				*/
				}
			}
		}else{
			if(file_exists($flash_path)){
				if(is_dir($flash_path)){
					rmdir($flash_path);
				}else{
					unlink($flash_path);
				}
			}
			//file_put_contents("/tmp/test.txt","=========3.copy($mem_path, $flash_path)\n",FILE_APPEND);
			//echo "=========3.copy($mem_path, $flash_path)\n";
			copy($mem_path, $flash_path);
			touch($mem_path);
		}
	}

	return true;
}

/*****************************                                                   
This function trans_to_gsm() is used to change the form of                   
the port show name when need.  
$name:such as : umts-1.2,umts1.2
	change to form of gsm-1.2,gsm1.2                    
******************************/ 
function trans_to_gsm($name) {
	global $__UMTS_HEAD__;
	global $__GSM_HEAD__;
	$match_str = substr($__UMTS_HEAD__,0,-1); // "umts-" to "umts"
	$gsm_head = substr($__GSM_HEAD__,0,-1);   // "gsm-" to "gsm"
	if (strstr($name,$match_str)) {
		$name = str_replace($match_str,$gsm_head,$name);
		return $name;
	} else {
		return $name;
	}
}

function change_gsms_port($channel,$board) {
	$name = get_gsm_name_by_channel($channel,$board);
	if (strstr($name,'null') || strstr($name,'none') || strstr($name,'unknown')) { 
		$name = preg_replace('/(\w+)-/','null-',$name);
	}	
	return $name;
}

function change_gsms_port_for_showtype($channel,$board) {
	$name = get_gsm_name_by_channel_for_showtype($channel,$board);
	if (strstr($name,'null') || strstr($name,'none') || strstr($name,'unknown')) { 
		$name = preg_replace('/(\w+)-/','null-',$name);
	}	
	return $name;
}

//system-status.php dynamic show port string 
function show_port_for_ajax()
{
	$module_array = get_module_type();
	$port_for_ajax = '';
	foreach ($module_array as $board => $channels ) {
		foreach($channels as $channel => $head) {
			$alias = get_gsm_aliasname($channel,$board);
			if($alias) {
				$alias = "($alias)";
			}
			$value = $head .$board.'.'.$channel.$alias;
			if ($head == 'null-') {
				$value = $value.' [Disconnected]';
			} else if ( $head == 'unknown-' || $head == 'none-') {
				$value = 'null-'.$board.'.'.$channel.$alias;
			}
			$port_for_ajax[$board][$channel] = $value;
		}
	}
	return $port_for_ajax;
}

//get slave http port 
function get_slave_httpport()
{
	$slaveport = 80;
	$user = get_web_user();
	if ($user['port'] == 443) {
		$slaveport = 80;
	} else {
		$slaveport = $user['port'];
	}

	return $slaveport;
}

/*
* The function is used to get model name.
* Use in logo-font and system-info page.
*/
function get_model_name(){
	$file_name = '/tmp/hw_info.cfg';
	$tools = 'set_config';
	$sys_type = exec("/my_tools/$tools $file_name get option_value sys sys_type");
	$chan_count = exec("/my_tools/$tools $file_name get option_value sys total_chan_count");
	$product_type = exec("/my_tools/$tools $file_name get option_value sys product_type");
	$hw_ver = exec("/my_tools/$tools $file_name get option_value sys hw_ver");
	
	$model_name = '';
	
	if($product_type == '5' && $hw_ver == '1.0'){
		$model_name = 'SWG-1016';
	}else{
		if($sys_type == 1){ //SWG-2
			if($chan_count <= 16){
				$model_name = 'SWG-2016';
			}else if($chan_count > 16 && $chan_count <= 32){
				$model_name = 'SWG-2032';
			}else if($chan_count > 32 && $chan_count <= 64){
				$model_name = 'SWG-2064';
			}
		}else if($sys_type == 3){ //VS_USB-1
			if($chan_count <= 8){
				$model_name = 'SWG-2008';
			}else if($chan_count > 8 && $chan_count <= 20){
				$model_name = 'VS_USB-1020';
			}else if($chan_count > 20 && $chan_count <= 44){
				$model_name = 'VS_USB-1044';
			}
		}
	}
	
	$model_name = "KVoLTE";
	
	return $model_name;
}

//get system_type.conf->demo
function get_demo_val(){
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$res = $aql->query('select * from system_type.conf');
	if(isset($res['demo']['demo_enable'])) {
		$demo_mode = trim($res['demo']['demo_enable']);
	} else {
		$demo_mode = 'off';
	}
	return $demo_mode;
}

//get module model
function get_module_model(){
	function cover($band_arr, $module_type){
		if($module_type == 'gsm'){
			$module_des = 'GSM '.language('Model Description');
		}else if($module_type == 'lte'){
			$module_des = 'LTE '.language('Model Description');
		}else if($module_type == 'cdma'){
			$module_des = 'CDMA '.language('Model Description');
		}else if($module_type == 'umts'){
			$module_des = 'UMTS '.language('Model Description');
		}else{
			$module_des = language('Model Description');
		}
		
		$band_str = '';
		for($i=0;$i<count($band_arr);$i++){
			$band_str .= "<span>".$band_arr[$i]."</span><br/>";
		}
		
		$html_str = "<tr>";
		$html_str .= "<th>".$module_des.":</th>";
		$html_str .= "<td id='module_model'>";
		$html_str .= $band_str;
		$html_str .= "</td>";
		$html_str .= "</tr>";
		
		return $html_str;
	}
	
	$modules_all_type = array();
	$modules = get_module_type();
	$modules_all_type = $modules[1];
	
	$modules_type_str = '';
	foreach($modules_all_type as $value){
		$modules_type_str .= $value;
	}
	
	//umts
	$module_umts = get_gsm_info();
	$module_umts_str = '';
	for($i=1;$i<=count($module_umts);$i++){
		$module_umts_str .= $module_umts[$i]['revision'];
	}
	if(strstr($module_umts_str, 'UC15A')){
		$umts_band = ["WCDMA: 850/1900MHz",
						"GSM: 850/900/1800/1900MHz"];
	}else if(strstr($module_umts_str, 'UC15E')){
		$umts_band = ["WCDMA: 900/2100MHz",
						"GSM: 900/1800MHz"];
	}else{
		$umts_band = [];
	}
	
	//gsm
	$gsm_band = ["GSM: 850/900/1800/1900MHz"];
	
	//cdma
	$cdma_band = ["CDMA 2000: 800MHz"];
	
	//lte
	$all_4g_modules = get_module_real_type();
	$modules_4g_type_str = '';
	foreach($all_4g_modules as $value){
		$modules_4g_type_str .= $value.'-';
	}
	if(strstr($modules_4g_type_str, 'EC20CE')){
		$lte_band = ["LTE FDD: B1/B3/B5/B8",
						"LTE TDD: B38/B39/B40/B41",
						"TD-SCDMA: B34/B39",
						"CDMA: BC0",
						"WCDMA: 900/2100MHz",
						"GSM: 900/1800MHz"];
	}else if(strstr($modules_4g_type_str, 'EC25E')){
		$lte_band = ["LTE FDD: B1/B3/B5/B7/B8/B20",
						"LTE TDD: B38/B40/B41",
						"WCDMA: 850/900/2100MHz",
						"GSM: 900/1800MHz"];
	}else if(strstr($modules_4g_type_str, 'EC25AUTL')){
		$lte_band = ["LTE FDD: B3/B7/B28"];
	}else if(strstr($modules_4g_type_str, 'EC25AUT')){
		$lte_band = ["LTE FDD: B1/B3/B5/B7/B28",
						"WCDMA: 850/2100MHz"];
	}else if(strstr($modules_4g_type_str, 'EC25AU')){
		$lte_band = ["LTE FDD: B1/B2/B3/B4/B5/B7/B8/B28",
						"LTE TDD: B40",
						"WCDMA: 850/900/1900/2100MHz",
						"GSM: 850/900/1800/1900MHz"];
	}else if(strstr($modules_4g_type_str, 'EC25A')){
		$lte_band = ["LTE FDD: B2/B4/B12",
						"WCDMA: 850/1700/1900/2100MHz"];
	}else if(strstr($modules_4g_type_str, 'EC25V')){
		$lte_band = ["LTE FDD: B4/B13"];
	}else if(strstr($modules_4g_type_str, 'EC25J')){
		$lte_band = ["LTE FDD: B1/B3/B8/B18/B19/B26",
						"LTE TDD: B41",
						"WCDMA: 850(B6)/850(B19)/900/2100MHz"];
	}else{
		$lte_band = [];
	}
	
	$html_str = '';
	if(strstr($modules_type_str, 'lte') && strstr($modules_type_str, 'gsm') ||
		strstr($modules_type_str, 'lte') && strstr($modules_type_str, 'cdma') ||
		strstr($modules_type_str, 'lte') && strstr($modules_type_str, 'umts') ||
		strstr($modules_type_str, 'gsm') && strstr($modules_type_str, 'cdma') ||
		strstr($modules_type_str, 'gsm') && strstr($modules_type_str, 'umts') ||
		strstr($modules_type_str, 'cdma') && strstr($modules_type_str, 'umts')){
		
		if(strstr($modules_type_str, 'umts')){
			$html_str .= cover($umts_band, 'umts');
		}
		
		if(strstr($modules_type_str, 'gsm')){
			$html_str .= cover($gsm_band, 'gsm');
		}
		
		if(strstr($modules_type_str, 'cdma')){
			$html_str .= cover($cdma_band, 'cdma');
		}
		
		if(strstr($modules_type_str, 'lte')){
			$html_str .= cover($lte_band, 'lte');
		}
	}else{
		if(strstr($modules_type_str, 'umts')){
			$html_str .= cover($umts_band, '');
		}
		
		if(strstr($modules_type_str, 'gsm')){
			$html_str .= cover($gsm_band, '');
		}
		
		if(strstr($modules_type_str, 'cdma')){
			$html_str .= cover($cdma_band, '');
		}
		
		if(strstr($modules_type_str, 'lte')){
			$html_str .= cover($lte_band, '');
		}
	}
	
	return $html_str;
}

//for Customized
function get_module_model_for_customized(){
	function cover($band_arr, $module_type){
		if($module_type == 'gsm'){
			$module_des = 'GSM '.language('Model Description');
		}else if($module_type == 'lte'){
			$module_des = 'LTE '.language('Model Description');
		}else if($module_type == 'cdma'){
			$module_des = 'CDMA '.language('Model Description');
		}else if($module_type == 'umts'){
			$module_des = 'UMTS '.language('Model Description');
		}else{
			$module_des = language('Model Description');
		}
		
		$band_str = '';
		for($i=0;$i<count($band_arr);$i++){
			$band_str .= "<span>".$band_arr[$i]."</span><br/>";
		}
		
		$html_str = "<div class='tab_item'>";
		$html_str .= "<span>".$module_des.":</span>";
		$html_str .= "<div class='tab_item_right'>";
		$html_str .= $band_str;
		$html_str .= "</div>";
		$html_str .= "<div style='clear:both;'></div>";
		$html_str .= "</div>";
		
		return $html_str;
	}
	
	$modules_all_type = array();
	$modules = get_module_type();
	$modules_all_type = $modules[1];
	
	$modules_type_str = '';
	foreach($modules_all_type as $value){
		$modules_type_str .= $value;
	}
	
	//umts
	$module_umts = get_gsm_info();
	$module_umts_str = '';
	for($i=1;$i<=count($module_umts);$i++){
		$module_umts_str .= $module_umts[$i]['revision'];
	}
	if(strstr($module_umts_str, 'UC15A')){
		$umts_band = ["WCDMA: 850/1900MHz",
						"GSM: 850/900/1800/1900MHz"];
	}else if(strstr($module_umts_str, 'UC15E')){
		$umts_band = ["WCDMA: 900/2100MHz",
						"GSM: 900/1800MHz"];
	}else{
		$umts_band = [];
	}
	
	//gsm
	$gsm_band = ["GSM: 850/900/1800/1900MHz"];
	
	//cdma
	$cdma_band = ["CDMA 2000: 800MHz"];
	
	//lte
	$all_4g_modules = get_module_real_type();
	$modules_4g_type_str = '';
	foreach($all_4g_modules as $value){
		$modules_4g_type_str .= $value.'-';
	}
	if(strstr($modules_4g_type_str, 'EC20CE')){
		$lte_band = ["LTE FDD: B1/B3/B5/B8",
						"LTE TDD: B38/B39/B40/B41",
						"TD-SCDMA: B34/B39",
						"CDMA: BC0",
						"WCDMA: 900/2100MHz",
						"GSM: 900/1800MHz"];
	}else if(strstr($modules_4g_type_str, 'EC25E')){
		$lte_band = ["LTE FDD: B1/B3/B5/B7/B8/B20",
						"LTE TDD: B38/B40/B41",
						"WCDMA: 850/900/2100MHz",
						"GSM: 900/1800MHz"];
	}else if(strstr($modules_4g_type_str, 'EC25AUTL')){
		$lte_band = ["LTE FDD: B3/B7/B28"];
	}else if(strstr($modules_4g_type_str, 'EC25AUT')){
		$lte_band = ["LTE FDD: B1/B3/B5/B7/B28",
						"WCDMA: 850/2100MHz"];
	}else if(strstr($modules_4g_type_str, 'EC25AU')){
		$lte_band = ["LTE FDD: B1/B2/B3/B4/B5/B7/B8/B28",
						"LTE TDD: B40",
						"WCDMA: 850/900/1900/2100MHz",
						"GSM: 850/900/1800/1900MHz"];
	}else if(strstr($modules_4g_type_str, 'EC25A')){
		$lte_band = ["LTE FDD: B2/B4/B12",
						"WCDMA: 850/1700/1900/2100MHz"];
	}else if(strstr($modules_4g_type_str, 'EC25V')){
		$lte_band = ["LTE FDD: B4/B13"];
	}else if(strstr($modules_4g_type_str, 'EC25J')){
		$lte_band = ["LTE FDD: B1/B3/B8/B18/B19/B26",
						"LTE TDD: B41",
						"WCDMA: 850(B6)/850(B19)/900/2100MHz"];
	}else{
		$lte_band = [];
	}
	
	$html_str = '';
	if(strstr($modules_type_str, 'lte') && strstr($modules_type_str, 'gsm') ||
		strstr($modules_type_str, 'lte') && strstr($modules_type_str, 'cdma') ||
		strstr($modules_type_str, 'lte') && strstr($modules_type_str, 'umts') ||
		strstr($modules_type_str, 'gsm') && strstr($modules_type_str, 'cdma') ||
		strstr($modules_type_str, 'gsm') && strstr($modules_type_str, 'umts') ||
		strstr($modules_type_str, 'cdma') && strstr($modules_type_str, 'umts')){
		
		if(strstr($modules_type_str, 'umts')){
			$html_str .= cover($umts_band, 'umts');
		}
		
		if(strstr($modules_type_str, 'gsm')){
			$html_str .= cover($gsm_band, 'gsm');
		}
		
		if(strstr($modules_type_str, 'cdma')){
			$html_str .= cover($cdma_band, 'cdma');
		}
		
		if(strstr($modules_type_str, 'lte')){
			$html_str .= cover($lte_band, 'lte');
		}
	}else{
		if(strstr($modules_type_str, 'umts')){
			$html_str .= cover($umts_band, '');
		}
		
		if(strstr($modules_type_str, 'gsm')){
			$html_str .= cover($gsm_band, '');
		}
		
		if(strstr($modules_type_str, 'cdma')){
			$html_str .= cover($cdma_band, '');
		}
		
		if(strstr($modules_type_str, 'lte')){
			$html_str .= cover($lte_band, '');
		}
	}
	
	return $html_str;
}

function check_apply(){
	$wait_apply_file = "/tmp/web/wait_apply";
	if(file_exists($wait_apply_file) && filesize($wait_apply_file)>0)
		return true;

	return false;
}

function save_user_record($db,$action){
	session_start();
	
	if($db == ""){//重复new DB会出错，调用之前有DB就传DB，没DB就传""
		$db = new Users();
	}
	
	$ip = $db->get_client_ip();
	
	if(isset($_SESSION['id']) && $_SESSION['id'] != ""){//已登录的操作
		$user_id = $_SESSION['id'];
	}else{//登录用户记录
		$username = $_POST['username'];
		$res = $db->get_one_user_by_username($username);
		$info = $res->fetchArray();
		
		$user_id = $info['id'];
	}
	
	$db->insert_record($user_id,$ip,$action);
}

function check_username_repeat($db,$old_username, $username){
	$results = $db->get_all_user_info();
	while($info = $results->fetchArray()){
		if($old_username == $username) continue;
		if($info['username'] == $username){
			return false;
		}
	}
	
	return true;
}

function get_nav_list(){
	global $enable_ast_logsettings;
	global $__system_type__;
	global $__GSM_SUM__;
	$head_array = get_module_arr();
	session_start();
	
	if ($enable_ast_logsettings == 'on') {
		$log_array = array(
			array("log-settings.php","Log Settings",1),
			array("log-sys.php","System",2),
			array("log-ast.php","Asterisk",3),
			array("log-sip.php","SIP",4),
			array("log-iax.php","IAX2",5),
			array("log-bsp.php","BSP",6),
			array("log-rri.php","RRI",7),
			array("log-debugat.php","AT Commands",8),
			array("log-cdr.php","CDR",9),
			array("log-statistics.php","Statistics",10),
	//		array("log-simemusvr.php","SimEmuSvr"),
		);
		
		if($_SESSION['id'] != 1){
			unset($log_array[9]);//Statistics
			// unset($log_array[8]);//CDR
			// unset($log_array[7]);//AT Commands
			unset($log_array[6]);//RRI
			unset($log_array[5]);//BSP
			unset($log_array[4]);//IAX2
			unset($log_array[2]);//Asterisk
		}
	} else {
		$log_array = array(
			array("log-settings.php","Log Settings",1),
			array("log-sys.php","System",2),
	//		array("log-ast.php","Asterisk",3),
			array("log-sip.php","SIP",4),
			array("log-iax.php","IAX2",5),
	//		array("log-bsp.php","BSP",6),
	//		array("log-rri.php","RRI",7),
			array("log-debugat.php","AT Commands",8),
			array("log-cdr.php","CDR",9),
			array("log-statistics.php","Statistics",10),
	//		array("log-simemusvr.php","SimEmuSvr"),
		);
		
		if($_SESSION['id'] != 1){
			unset($log_array[6]);//Statistics
			// unset($log_array[5]);//CDR
			unset($log_array[3]);//IAX2
		}
	}

	$enable_emu_logsettings = get_oem_info_sepcs('enable_emu_logsettings');
	if($enable_emu_logsettings == 'on'){
		array_push($log_array, array("log-simemusvr.php","SimEmuSvr",9));
	}

	$nav_lists = array(
		"system"=>array(
			array("system-status.php","Status",1),
			array("system-time.php","Time",2),
			array("system-login.php","Login Settings",3),
			array("system-ssh.php","SSH",4),
			array("system-general.php","General",5),
			array("system-tools.php","Tools",6),
			array("system-user.php","User",7),
			array("system-info.php","Information",8),
			array("system-wizard.php", "Setting Wizard",9),
		),
		
		"module"=>$head_array,

		"voip"=>array(
			array("voip-endpoints.php","VoIP Endpoints",1),
			array("sip-bindpoints.php", "Batch SIP Endpoints",2),
			array("sip-adv-settings.php","Advanced SIP Settings",3),
			array("iax-adv-settings.php","Advanced IAX2 Settings",4),
			array("sip-security.php","SIP Account Security",5),
			),

		"routing"=>array(
			array("routing-rules.php","Call Routing Rules",1),
			array("routing-groups.php","Groups",2),
			array("sip-sim-binding.php", "Batch Creating Rules",3),
			array("routing-mnp.php","MNP Settings",4),
			array("routing-blacklist.php","Routing Blacklist",5),
			array("routing-advanced.php", "Advanced",6),
			),

		"sms"=>array(
			array("sms-settings.php","SMS Settings",1),
			array("sms-sender.php","SMS Sender",2),
			array("sms-inbox.php","SMS Inbox",3),
			array("sms-outbox.php","SMS Outbox",4),
			array("sms-routing.php","SMS Forwarding",5),		
			array("sms-ussd.php","USSD",6),
			// array("sms-mms.php","MMS Settings",7),
		),

		"network"=>array(
			array("network-lan.php","LAN Settings",1),
			array("network-wan.php","WAN Settings",2),
			array("network-vpn.php","VPN Settings",3),
			array("network-ddns.php","DDNS Settings",4),
			array("network-toolkit.php","Toolkit",5),
			array("network-switch.php","Security Settings",6),
			array("network-rules.php","Security Rules",7),
			//array("network-sipcapture.php","SIP Capture",8),
		),

		"advanced"=> array(
			array("adv-astapi.php","Asterisk API",1),
			array("adv-astcli.php","Asterisk CLI",2),
			array("adv-astfileeditor.php","Asterisk File Editor",3),
			array("adv-internet.php","Internet",4),
			array("adv-cloud.php", "Cloud",5),
			array("adv-balance.php", "Balance",6),
			array("adv-phonenumber.php", "PhoneNumber",7),
			array("adv-arp.php", "ARP",8),
		),

		"logs"=>$log_array
	);
	
	$has_delete_cloud = 0;
	if($_SESSION['id'] != 1){
		unset($nav_lists['system'][6]);//user
		unset($nav_lists['system'][3]);//ssh
		
		unset($nav_lists['voip'][1]);//sip-bindpoints
		
		unset($nav_lists['voip'][3]);//IAX2
		
		unset($nav_lists['advanced'][0]);//asterisk api
		unset($nav_lists['advanced'][1]);//asterisk cli
		unset($nav_lists['advanced'][2]);//asterisk file
		unset($nav_lists['advanced'][3]);//internet
		unset($nav_lists['advanced'][4]);//cloud
		
		$has_delete_cloud = 1;
	}

	$internet_flag = 0;
	for($i=0;$i<=$__GSM_SUM__;$i++){
		$port_name = get_gsm_name_by_channel($i);
		if(strstr($port_name, 'lte')){
			$internet_flag = 1;
			break;
		}
	}
	if($internet_flag == 0 && $has_delete_cloud == 0){
		unset($nav_lists['advanced'][3]);//delete internet page
		$nav_lists['advanced'] = array_values($nav_lists['advanced']);
	}

	//delete internet page
	$sys_type = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys sys_type");
	$product_type = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys product_type");
	$hw_ver = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys hw_ver");
	if($product_type == '5' && $hw_ver == '1.0'){
		unset($nav_lists['network'][1]);
		$nav_lists['network'] = array_values($nav_lists['network']);
	}else if($sys_type == 3){
		unset($nav_lists['network'][1]);
		$nav_lists['network'] = array_values($nav_lists['network']);
	}
	
	return $nav_lists;
}

function deal_nav_lists_by_auth($nav_lists){
	$user = new Users();
	session_start();
	$username = $_SESSION['username'];
	
	$user = new Users();
	
	$res = $user->get_one_user_by_username($username);
	$info = $res->fetchArray();
	$user_auth = unserialize($info['auth']);
	
	if($info['super'] == 1){
		return $nav_lists;
	}
	
	foreach($nav_lists as $key => $val){
		if(count($user_auth[$key]) == 0){
			unset($nav_lists[$key]);
			continue;
		}
		
		for($i=0;$i<count($val);$i++){
			$num = $val[$i][2];
			
			if(!in_array($num,$user_auth[$key])){
				unset($nav_lists[$key][$i]);
				
			}
		}
		
		$nav_lists[$key] = array_values($nav_lists[$key]);
	}
	
	return $nav_lists;
}

function check_user_auth(){
	session_start();
	$username = $_SESSION['username'];
	
	$user = new Users();
	
	$res = $user->get_one_user_by_username($username);
	$info = $res->fetchArray();
	$user_auth = unserialize($info['auth']);
	
	$temp = explode('/',$_SERVER['PHP_SELF']);
	$page = $temp[count($temp)-1];
	
	$page_num_arr = get_page_num($page);
	
	//save_user_record($user,'p-'.$page_num_arr[2].'-'.$page_num_arr[1]);
	
	if($info['super'] == 1){
		return true;
	}
	
	if($page == 'error-auth.php'){
		return true;
	}
	
	if(in_array($page_num_arr[1],$user_auth[$page_num_arr[0]])){
		return true;
	}else{
		return false;
	}
}

function check_user_onlyview_auth(){
	session_start();
	$username = $_SESSION['username'];
	
	$user = new Users();
	
	$res = $user->get_one_user_by_username($username);
	$info = $res->fetchArray();
	$user_auth = unserialize($info['auth']);
	
	$temp = explode('/',$_SERVER['PHP_SELF']);
	$page = $temp[count($temp)-1];
	
	$page_num_arr = get_page_num($page);
	
	if($info['super'] == 1){
		return false;
	}
	
	if($page == 'error-auth.php'){
		return false;
	}
	
	if(in_array($page_num_arr[1],$user_auth['view_'.$page_num_arr[0]])){
		return true;
	}else{
		return false;
	}
}

function get_page_num($page){
	global $nav_lists;
	
	$n = 1;//用n代表key，为了减少写入文件的数据
	foreach($nav_lists as $key => $val){
		for($i=0;$i<count($val);$i++){
			if($val[$i][0] == $page){
				return [$key,$val[$i][2],$n];
			}
		}
		$n++;
	}
	
	//如果不是nav_lists页面
	return ['hide',$page];
}

function get_module_arr(){
	global $flag_cdma;
	session_start();
	
	$modules_all_type = array();
	$modules = get_module_type();
	$modules_all_type = $modules[1];
	$flag_cdma = false; 
	$modules_type_str = '';
	foreach($modules_all_type as $value){
		$modules_type_str .= $value;
	}
	if(!strstr($modules_type_str, 'lte') && !strstr($modules_type_str, 'gsm') && !strstr($modules_type_str, 'umts')){
		$flag_cdma = true;
	}
	$sys_type = exec("/my_tools/set_config/tmp/hw_info.cfg get option_value sys sys_type");
	$bcch_arr = array(
		array("gsm-settings.php","Module Settings",1),
		//array("gsm-adv-settings.php","Advanced"),
		array("gsm-callforwarding.php","Call Forwarding",2),
		array("gsm-callwaiting.php","Call Waiting",3),
		array("simbank-settings.php","Simbank",4),
		array("gsm-dtmf.php","DTMF",5),
		array("gsm-bcch.php","BCCH",6),
		array("gsm-toolkit.php","Toolkit",7),
		array("module-update.php","Module Update",8),
		array("gsm-calllimit.php","Call And SMS Limit",9),
	);
	if($_SESSION['id'] != 1){
		array_splice($bcch_arr, 7, 1);
		array_splice($bcch_arr, 6, 1);
	}
	if(!strstr($modules_type_str, 'gsm')){
		 array_splice($bcch_arr, 5, 1);
	}
	if($sys_type != 1){
		 array_splice($bcch_arr, 3, 1);
	}
	
	if($flag_cdma){
		$head_array = array(
			array("gsm-settings.php","Module Settings",1),
			//array("gsm-adv-settings.php","Advanced"),
			//array("gsm-callforwarding.php","Call Forwarding",2),
			//array("gsm-callwaiting.php","Call Waiting",3),
			array("simbank-settings.php","Simbank",4),
			array("gsm-dtmf.php","DTMF",5),
			//array("gsm-bcch.php","BCCH",6),
			array("gsm-toolkit.php","Toolkit",7),
			array("module-update.php","Module Update",8),
			array("gsm-calllimit.php","Call And SMS Limit",9),
		);
		if($_SESSION['id'] != 1){
			array_splice($head_array, 4, 1);
			array_splice($head_array, 3, 1);
		}
		if($sys_type != 1){
			 array_splice($head_array, 1, 1);
		}
		
		return $head_array;
	}else {
		return $bcch_arr;
	}
}

function get_data_info(){
	$aql = new aql();
	$setok = $aql->set('basedir','/www/data/info');
	if (!$setok) {
		exit(255);
	}

	$hlock=lock_file("/www/data/info/gw_info.conf");
	$general_conf = $aql->query("select * from gw_info.conf where section='general'");
	unlock_file($hlock);
	
	$arr = [];
	$arr['switch'] = $general_conf['general']['switch'];
	$arr['copyright'] = $general_conf['general']['copyright'];
	$arr['header_image'] = $general_conf['general']['header_image'];
	$arr['footer_image'] = $general_conf['general']['footer_image'];
	
	return $arr;
}

function reset_user_login_info(){
	$redis_cli = new Predis\Client();
	
	session_start();
	session_destroy();
	
	$redis_cli->set("login.password.times","");
	$redis_cli->set("login.check.multiple","");
}
?>
