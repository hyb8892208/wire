<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");
?>

<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<script type="text/javascript" src="/js/float_btn.js"></script>
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />

<?php
function save_debugat_to_gwgsm_conf($debugat)
{
	global $__GSM_SUM__;
	$conf_path = "/etc/asterisk";
	$gw_gsm_conf = "gw_gsm.conf";
	$conf_path_file = $conf_path."/".$gw_gsm_conf;
	$aql = new aql();
	$setok = $aql->set('basedir',$conf_path);
	if (!$setok) {
		echo $aql->get_error();
	}

	$hlock=lock_file($conf_path_file);
	if(!file_exists($conf_path_file)) {
		fclose(fopen($conf_path_file,"w"));
	}
	if(!$aql->open_config_file($conf_path_file)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	$gw_gsm_conf_arr = $aql->query("select * from $gw_gsm_conf");
	for($i = 1; $i <= $__GSM_SUM__; $i++){
		if(!isset($gw_gsm_conf_arr["$i"]['debugat'])){
			$aql->assign_append($i, "debugat", $debugat);
		} else {
			$aql->assign_editkey($i, "debugat", $debugat);
		}
	}
	if (!$aql->save_config_file($gw_gsm_conf)) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}

	unlock_file($hlock);
}
function set_debugat_to_cfgfile($debugat)
{
	$i = 0;
	$ch = 1;
	$has_debugat = false;
	$file_path = "/etc/asterisk/extra-channels.conf";
	global $__GSM_SUM__;

	if(!file_exists($file_path))
	   return false;

	$hlock = lock_file($file_path);
	$pf = fopen($file_path, "r");
/*
	$debugat[1] = $debugat1;
	$debugat[2] = $debugat2;
	$debugat[3] = $debugat3;
	$debugat[4] = $debugat4;
	$debugat[5] = $debugat5;
	$debugat[6] = $debugat6;
	$debugat[7] = $debugat7;
	$debugat[8] = $debugat8;
*/
	$context = [];
	while(!feof($pf) && ($ch <= $__GSM_SUM__)) {
		$line = fgets($pf);
		if(substr($line,0,7) == 'debugat') {
			$has_debugat = true;
			//if(!isset($debugat[$ch])) {
			//	$line = "";
			//} else {
			//	$line = "debugat=$debugat[$ch]\n";
			//}
			$line = "debugat=$debugat\n";
		}

		if(substr($line,0,7) == 'channel') {
			if(!$has_debugat) {
				//if($debugat[$ch] != "")
				//if(isset($debugat[$ch]))
					//$context[$i++] = "debugat=$debugat[$ch]\n";  //Insert
				$context[$i++] = "debugat=$debugat\n";  //Insert
			} else {
				$has_debugat = false;
			}
			$ch++;
		}

		$context[$i] = $line;
		$i++;
	}

	fclose($pf);

	$pf = fopen($file_path, "w");
	foreach($context as $line) {
		fwrite($pf,$line);
	}
	fclose($pf);

	unlock_file($hlock);

	return true;
}

function save_to_logfile_monitor_conf()
{
/*
Location:/etc/asterisk/logfile_monitor.conf
[sys_log]
autoclean_sw=on
maxsize=1MB

[ast_log]
autoclean_sw=on
maxsize=100KB

[sip_log]
autoclean_sw=on
maxsize=100KB

[iax_log]
autoclean_sw=on
maxsize=100KB

[bsp_log]
autoclean_sw=on
maxsize=100KB

[rri_log]
autoclean_sw=on
maxsize=100KB

[at_log]
autoclean_sw=on
maxsize=100KB

[cdr_log]
autoclean_sw=on
maxsize=100KB

[sms_inbox]
autoclean_sw=on
maxsize=100KB
*/
	$conf_path = '/etc/asterisk/gw';
	$conf_file = 'logfile_monitor.conf';
	$conf_path_file = $conf_path."/".$conf_file;

	$aql = new aql();
	$setok = $aql->set('basedir',$conf_path);
	if (!$setok) {
		echo $aql->get_error();
	}

	$hlock=lock_file($conf_path_file);
	if(!file_exists($conf_path_file)) {
		fclose(fopen($conf_path_file,"w"));
	}
	if(!$aql->open_config_file($conf_path_file)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	$conf_array = $aql->query("select * from $conf_file");

	$log_array = array("sys_log", "ast_log", "sip_log", "iax_log", "bsp_log", "rri_log", "at_log", "cdr_log");
	foreach($log_array as $log){
		if(!isset($conf_array["$log"])) {
			$aql->assign_addsection("$log",'');
		}
		if(isset($_POST[$log."_autoclean_sw"])){
			if(isset($conf_array["$log"]['autoclean_sw']))
				$aql->assign_editkey("$log",'autoclean_sw',"on");
			else
				$aql->assign_append("$log",'autoclean_sw',"on");
			if(isset($_POST[$log."_maxsize"])){
				if(isset($conf_array["$log"]['maxsize']))
					$aql->assign_editkey("$log",'maxsize',$_POST[$log."_maxsize"]);
				else
					$aql->assign_append("$log",'maxsize',$_POST[$log."_maxsize"]);
			}
		}else{
			if(isset($conf_array["$log"]['autoclean_sw']))
				$aql->assign_editkey("$log",'autoclean_sw',"off");
			else
				$aql->assign_append("$log",'autoclean_sw',"off");
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
?>

<?php
function save_to_gw_conf()
{
/*
[sys-log]
switch=on

[cdr]
switch=on

[sip-log]
switch=off

[iax2-log]
switch=off

[bsp-log]
logsw=verbose,track,debug,info,warning,error

[rri-log]
logsw=notice,info,debug,warning,error
pipeswitch=off

[debugat-log]
switch=off
*/
	if(isset($_POST['cdr_sw'])) {
		$_cdr_sw='on';
		$cdr_log=true;
	} else {
		$_cdr_sw='off';
		$cdr_log=false;
	}

	if(isset($_POST['sys_log_sw'])) {
		$_sys_log_sw='on';
	} else {
		$_sys_log_sw='off';
	}

	if(isset($_POST['sip_log_sw'])) {
		$_sip_log_sw='on';
	} else {
		$_sip_log_sw='off';
	}

	if(isset($_POST['iax_log_sw'])) {
		$_iax_log_sw='on';
	} else {
		$_iax_log_sw='off';
	}
	
	$bsp_logsw_str='';
	$bsp_cmd = '/my_tools/bsp_cli server_debug ';
	$bsp_flag = 0;
	if(isset($_POST['bsp_notice_sw'])){
		$bsp_logsw_str .= 'notice,';
		$bsp_cmd .= 'notice,';
		$bsp_flag = 1;
	}
	if(isset($_POST['bsp_debug_sw'])){
		$bsp_logsw_str .= 'debug,';
		$bsp_cmd .= 'debug,';
		$bsp_flag = 1;
	}
	if(isset($_POST['bsp_info_sw'])){
		$bsp_logsw_str .= 'info,';
		$bsp_cmd .= 'info,';
		$bsp_flag = 1;
	} 
	if(isset($_POST['bsp_warning_sw'])){
		$bsp_logsw_str .= 'warning,';
		$bsp_cmd .= 'warning,';
		$bsp_flag = 1;
	}
	if(isset($_POST['bsp_error_sw'])){
		$bsp_logsw_str .= 'error,';
		$bsp_cmd .= 'error,';
		$bsp_flag = 1;
	} 
	$bsp_logsw_str = rtrim($bsp_logsw_str,',');
	$bsp_cmd = rtrim($bsp_cmd,',');
	
	$rri_logsw_str='';
	$rri_cmd = '/my_tools/rri_cli server debug ';
	$rri_flag = 0;
	if(isset($_POST['rri_notice_sw'])){
		$rri_logsw_str .= 'notice,';
		$rri_cmd .= 'notice,';
		$rri_flag = 1;
	}
	if(isset($_POST['rri_debug_sw'])){
		$rri_logsw_str .= 'debug,';
		$rri_cmd .= 'debug,';
		$rri_flag = 1;
	}
	if(isset($_POST['rri_info_sw'])){
		$rri_logsw_str .= 'info,';
		$rri_cmd .= 'info,';
		$rri_flag = 1;
	}
	if(isset($_POST['rri_warning_sw'])){
		$rri_logsw_str .= 'warning,';
		$rri_cmd .= 'warning,';
		$rri_flag = 1;
	}
	if(isset($_POST['rri_error_sw'])){
		$rri_logsw_str .= 'error,';
		$rri_cmd .= 'error,';
		$rri_flag = 1;
	}
	$rri_logsw_str = rtrim($rri_logsw_str,',');
	$rri_cmd = rtrim($rri_cmd,',');
	
	if(isset($_POST['pipe_log_sw'])) {
		$_pipe_log_sw='on';
	} else {
		$_pipe_log_sw='off';
	}
	
	if(isset($_POST['debugat_log_sw'])) {
		$_debugat_log_sw='on';
	} else {
		$_debugat_log_sw='off';
	}

	if(isset($_POST['cdr_log_append_imei_sw'])) {
		$cdr_log_append_imei_sw = 'on';
	} else {
		$cdr_log_append_imei_sw = 'off';
	}
	$setup_wizard = `/my_tools/set_config /etc/asterisk/gw.conf get option_value setup-wizard set_wizard`;

	$write_str = '';
	$write_str .= "[sys-log]\n";
	$write_str .= "switch=$_sys_log_sw\n\n";

	$write_str .= "[cdr]\n";
	$write_str .= "switch=$_cdr_sw\n";
	$write_str .= "append_imei_sw=$cdr_log_append_imei_sw\n\n";

	$write_str .= "[sip-log]\n";
	$write_str .= "switch=$_sip_log_sw\n\n";

	$write_str .= "[iax2-log]\n";
	$write_str .= "switch=$_iax_log_sw\n\n";
	
	$write_str .= "[bsp-log]\n";
	$write_str .= "logsw=$bsp_logsw_str\n\n";
	
	$write_str .= "[rri-log]\n";
	$write_str .= "logsw=$rri_logsw_str\n";
	$write_str .= "pipeswitch=$_pipe_log_sw\n\n";
	
	$write_str .= "[debugat-log]\n";
	$write_str .= "switch=$_debugat_log_sw\n\n";

	$write_str .= "[setup-wizard]\n";
	$write_str .= "set_wizard=$setup_wizard\n\n";

	$gw_conf = '/etc/asterisk/gw.conf';

	$hlock=lock_file($gw_conf);
	$handle = fopen($gw_conf,"w");
	fwrite($handle,$write_str);
	fclose($handle);
	unlock_file($hlock);

	if($bsp_flag == 1){
		wait_apply("exec", $bsp_cmd);
	}else{
		wait_apply("exec", "/my_tools/bsp_cli server_debug none");
	}
	if($rri_flag == 1){
		wait_apply("exec", $rri_cmd);
	}else{
		wait_apply("exec", "/my_tools/rri_cli server debug none");
	}
	if($_pipe_log_sw == 'on'){
		wait_apply("exec", '/my_tools/rri_cli chn_at debug -1 1');
	}else{
		wait_apply("exec", '/my_tools/rri_cli chn_at debug -1 0');
	}
	//set_debugat_to_cfgfile($_debugat_log_sw,$_debugat_log_sw,$_debugat_log_sw,$_debugat_log_sw,$_debugat_log_sw,$_debugat_log_sw,$_debugat_log_sw,$_debugat_log_sw);
	save_debugat_to_gwgsm_conf($_debugat_log_sw);
	set_debugat_to_cfgfile($_debugat_log_sw);
}

function save_to_logger_conf()
{
	$aql = new aql();
	$conf_path = "/etc/asterisk";
	$setok = $aql->set('basedir',$conf_path);
	if (!$setok) {
		echo $aql->get_error();
	}

	$log_str='';
	if(isset($_POST['notice_sw'])) $log_str .= 'notice,';
	if(isset($_POST['warning_sw'])) $log_str .= 'warning,';
	if(isset($_POST['error_sw'])) $log_str .= 'error,';
	if(isset($_POST['debug_sw'])) $log_str .= 'debug,';
	if(isset($_POST['verbose_sw'])) $log_str .= 'verbose,';
	if(isset($_POST['dtmf_sw'])) $log_str .= 'dtmf,';
	$log_str = rtrim($log_str,',');
	
	$write = "[logfiles]\n";
	$write .= "syslog.local3 => $log_str\n";
	$write .= "log4gw => $log_str";

	file_put_contents('/etc/asterisk/logger.conf',$write);
}

function save_gw_syslog(){
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk/gw');
	if (!$setok) {
		echo $aql->get_error();
		return false;
	}

	$syslog_conf_path = '/etc/asterisk/gw/gw_syslog.conf';
	if(!file_exists($syslog_conf_path)){
		exec("touch $syslog_conf_path");
	}
	
	$hlock = lock_file($syslog_conf_path);
	if(!$aql->open_config_file($syslog_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	
	$exist_array = $aql->query("select * from gw_syslog.conf");
	
	if(!isset($exist_array['syslog'])){
		$aql->assign_addsection('syslog','');
	}
	
	isset($_POST['local_sys'])?$syslog = 'on':$syslog = 'off';
	if(isset($exist_array['syslog']['syslog'])){
		$aql->assign_editkey('syslog','syslog',$syslog);
	}else{
		$aql->assign_append('syslog','syslog',$syslog);
	}
	
	if(isset($exist_array['syslog']['serverip'])){
		$aql->assign_editkey('syslog','serverip',$_POST['serverip']);
	}else{
		$aql->assign_append('syslog','serverip',$_POST['serverip']);
	}
	$exist_array['syslog']['serverip'] = $_POST['serverip'];
	
	if(isset($exist_array['syslog']['serverport'])){
		$aql->assign_editkey('syslog','serverport',$_POST['serverport']);
	}else{
		$aql->assign_append('syslog','serverport',$_POST['serverport']);
	}
	
	if(isset($exist_array['syslog']['sysloglevel'])){
		$aql->assign_editkey('syslog','sysloglevel',$_POST['sysloglevel']);
	}else{
		$aql->assign_append('syslog','sysloglevel',$_POST['sysloglevel']);
	}
	$exist_array['syslog']['sysloglevel'] = $_POST['sysloglevel'];
	
	if(isset($exist_array['syslog']['cdrlevel'])){
		$aql->assign_editkey('syslog','cdrlevel',$_POST['cdrlevel']);
	}else{
		$aql->assign_append('syslog','cdrlevel',$_POST['cdrlevel']);
	}
	
	if (!$aql->save_config_file('gw_syslog.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);

	save_to_syslog($exist_array);
	
	if($syslog == 'on'){
		if($exist_array['syslog']['syslog'] == 'on'){
			wait_apply('exec','/etc/init.d/sysklog restart');
		}else{
			wait_apply('exec','/etc/init.d/sysklog start');
		}
	}else{
		file_put_contents('/etc/asterisk/syslog/syslog.conf', '');
		wait_apply('exec','/etc/init.d/sysklog stop');
	}
	
	save_to_cdr_syslog();
}

function save_to_syslog($exist_array){
	$serverip = $exist_array['syslog']['serverip'];
	$sysloglevel = $exist_array['syslog']['sysloglevel'];
	$write = "syslog.$sysloglevel @$serverip\n";
	$write .= "local3.* @$serverip";
	
	if($_POST['cdrlevel'] != 'off'){
		$write .= "\nlocal4.* @$serverip";
	}
	
	file_put_contents('/etc/asterisk/syslog/syslog.conf', $write);
}

function save_to_cdr_syslog(){
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk/');
	if (!$setok) {
		echo $aql->get_error();
		return false;
	}

	$syslog_conf_path = '/etc/asterisk/cdr_syslog.conf';
	if(!file_exists($syslog_conf_path)){
		exec("touch $syslog_conf_path");
	}
	
	$hlock = lock_file($syslog_conf_path);
	if(!$aql->open_config_file($syslog_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	
	$exist_array = $aql->query("select * from cdr_syslog.conf");
	
	if(!isset($exist_array['cdr'])){
		$aql->assign_addsection('cdr','');
	}
	
	if($_POST['cdrlevel'] == 'off') {
		$_POST['cdrlevel'] = '';
		$facility_str = '';
	}else{
		$facility_str = 'local4';
	}
	
	if(isset($exist_array['cdr']['facility'])){
		$aql->assign_editkey('cdr','facility',$facility_str);
	}else{
		$aql->assign_append('cdr','facility',$facility_str);
	}
	
	if(isset($exist_array['cdr']['priority'])){
		$aql->assign_editkey('cdr','priority',$_POST['cdrlevel']);
	}else{
		$aql->assign_append('cdr','priority',$_POST['cdrlevel']);
	}
	
	if(isset($exist_array['cdr']['template'])){
		$aql->assign_editkey('cdr','template','"Caller ID: ${CDR(clid)}","Callee ID: ${CDR(dst)}","From: ${CDR(dcontext)}","Channel: ${CDR(dstchannel)}","Start Date: ${CDR(start)}","Answer Date: ${CDR(answer)}","Duration: ${CDR(billsec)} s","Result: ${CDR(disposition)}"');
	}else{
		$aql->assign_append('cdr','template','"Caller ID: ${CDR(clid)}","Callee ID: ${CDR(dst)}","From: ${CDR(dcontext)}","Channel: ${CDR(dstchannel)}","Start Date: ${CDR(start)}","Answer Date: ${CDR(answer)}","Duration: ${CDR(billsec)} s","Result: ${CDR(disposition)}"');
	}
	
	if (!$aql->save_config_file('cdr_syslog.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
}

function show_logsetting(){
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk');
	if (!$setok) {
		echo __LINE__.' '.$aql->get_error();
		exit;
	}
	
	$notice_sw = '';
	$warning_sw = '';
	$error_sw = '';
	$debug_sw = '';
	$verbose_sw = '';
	$dtmf_sw = '';
	$hlock=lock_file("/etc/asterisk/logger.conf");
	$ast_log = $aql->query("select * from logger.conf where section = 'logfiles'");
	unlock_file($hlock);
	if(isset($ast_log['logfiles']['log4gw'])) {
		$item = explode(",",$ast_log['logfiles']['log4gw'],7);
		foreach($item as $value) {
			$value=trim($value);
			switch($value) {
			case 'verbose':
				$verbose_sw = 'checked';
				break;
			case 'notice':
				$notice_sw = 'checked';
				break;
			case 'warning':
				$warning_sw = 'checked';
				break;
			case 'debug':
				$debug_sw = 'checked';
				break;
			case 'error':
				$error_sw = 'checked';
				break;
			case 'dtmf':
				$dtmf_sw = 'checked';
				break;
			default:
				break;
			}
		}
	}

	/* get gw.conf data */
	$hlock = lock_file('/etc/asterisk/gw.conf');
	$res = $aql->query("select * from gw.conf");
	unlock_file($hlock);

	$sys_log_sw = ''; 
	if(isset($res['sys-log']['switch'])) {
		if(is_true(trim($res['sys-log']['switch']))) {
			$sys_log_sw = 'checked';
		}
	}

	$sip_log_sw = ''; 
	if(isset($res['sip-log']['switch'])) {
		if(is_true(trim($res['sip-log']['switch']))) {
			$sip_log_sw = 'checked';
		}
	}

	$iax_log_sw = ''; 
	if(isset($res['iax2-log']['switch'])) {
		if(is_true(trim($res['iax2-log']['switch']))) {
			$iax_log_sw = 'checked';
		}
	}

	$bsp_notice_sw = '';
	$bsp_debug_sw = '';
	$bsp_info_sw = '';
	$bsp_warning_sw = '';
	$bsp_error_sw = '';
	if(isset($res['bsp-log']['logsw'])) {
		$item = explode(",",$res['bsp-log']['logsw']);
		foreach($item as $value) {
			$value=trim($value);
			switch($value) {
				case 'notice':
					$bsp_notice_sw = 'checked';
					break;
				case 'debug':
					$bsp_debug_sw = 'checked';
					break;
				case 'info':
					$bsp_info_sw = 'checked';
					break;
				case 'warning':
					$bsp_warning_sw = 'checked';
					break;
				case 'error':
					$bsp_error_sw = 'checked';
					break;
				default:
					break;
			}
		}
	}

	$rri_notice_sw = '';
	$rri_debug_sw = '';
	$rri_info_sw = '';
	$rri_warning_sw = '';
	$rri_error_sw = '';
	if(isset($res['rri-log']['logsw'])){
		$item = explode(",", $res['rri-log']['logsw']);
		foreach($item as $value){
			$value=trim($value);
			switch($value){
				case 'notice':
					$rri_notice_sw = 'checked';
					break;
				case 'debug':
					$rri_debug_sw = 'checked';
					break;
				case 'info':
					$rri_info_sw = 'checked';
					break;
				case 'warning':
					$rri_warning_sw = 'checked';
					break;
				case 'error':
					$rri_error_sw = 'checked';
					break;
				default:
					break;
			}
		}
	}

	$pipe_log_sw = '';
	if(isset($res['rri-log']['pipeswitch'])){
		if(is_true($res['rri-log']['pipeswitch'])){
			$pipe_log_sw = 'checked';
		}
	}

	$debugat_log_sw = ''; 
	if(isset($res['debugat-log']['switch'])) {
		if(is_true(trim($res['debugat-log']['switch']))) {
			$debugat_log_sw = 'checked';
		}
	}

	$cdr_sw = ''; 
	if(isset($res['cdr']['switch'])) {
		if(is_true(trim($res['cdr']['switch']))) {
			$cdr_sw = 'checked';
		}
	}

	$cdr_log_append_imei_sw = ''; 
	if(isset($res['cdr']['append_imei_sw'])) {
		if(is_true(trim($res['cdr']['append_imei_sw']))) {
			$cdr_log_append_imei_sw = 'checked';
		}
	}

	/* get auto clean conf */
	$log_array = array("sys_log", "ast_log", "sip_log", "iax_log", "bsp_log", "rri_log", "at_log", "cdr_log");
	$conf_path = '/etc/asterisk/gw';
	$conf_file = 'logfile_monitor.conf';
	$conf_path_file = $conf_path."/".$conf_file;
	$setok = $aql->set('basedir',$conf_path);
	if (!$setok) {
		echo $aql->get_error();
	}
	$hlock = lock_file($conf_path_file);
	$res = $aql->query("select * from $conf_file");
	unlock_file($hlock);

	foreach($log_array as $log){
		${$log."_autoclean_sw"} = "";
		${$log."_maxsize"} = "1000";
		if(isset($res["$log"]['autoclean_sw'])) {
			if(is_true(trim($res["$log"]['autoclean_sw']))) {
				${$log."_autoclean_sw"} = "checked";
			}
		}
		if(isset($res["$log"]['maxsize'])) {
			${$log."_maxsize"} = trim($res["$log"]['maxsize']);		
		}
	}
	
	//syslog
	$local_sys = '';
	$serverip = '';
	$serverport = '';
	$sysloglevel = '';
	$cdrlevel = '';
	if(file_exists('/etc/asterisk/gw/gw_syslog.conf')){
		$setok = $aql->set('basedir','/etc/asterisk/gw');
		$res = $aql->query("select * from gw_syslog.conf");
		if($res['syslog']['syslog'] == 'on'){
			$local_sys = 'checked';
		}
		$serverip = $res['syslog']['serverip'];
		$serverport = $res['syslog']['serverport'];
		$sysloglevel = $res['syslog']['sysloglevel'];
		$cdrlevel = $res['syslog']['cdrlevel'];
	}
	
	global $enable_ast_logsettings;
	if($enable_ast_logsettings == 'on'){
		$show_log = "";
	}else{
		$show_log = "style='display:none;'";
	}
	
?>
	<form id="logsetting_form" enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post" >
		<div id="tab">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('System Logs');?></li>
			<li class="tb2">&nbsp;</li>
		</div>

		<table width="100%" class="tedit" >
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('System Logs')?>:
						<span class="showhelp">
						<?php echo language('System Logs help','Whether enable or disable system log.');?>
						</span>
					</div>
				</th>
				<td >
					<input type=checkbox name="sys_log_sw" <?php echo $sys_log_sw?> >
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Auto clean');?>:
						<span class="showhelp">
						<?php echo language('Auto clean help','
							switch on : when the size of log file reaches the max size, <br> 
							&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; the system will cut a half of the file. New logs will be retained.<br>
							switch off : logs will remain, and the file size will increase gradually. <br>');
							echo language('Auto clean default@System Logs','default on, default size=1MB.');
						?>
						</span>
					</div>
				</th>
				<td>
					<table style="margin:0px;padding:0px;border:0"><tr>
						<td style="margin:0;padding:0;border:0">
							<input type=checkbox id="sys_log_autoclean_sw" name="sys_log_autoclean_sw" <?php echo $sys_log_autoclean_sw?> >
						</td>
						<td style="border:0">
							<?php echo language('maxsize');?> : 
							<select id="sys_log_maxsize" name="sys_log_maxsize" <?php if($sys_log_autoclean_sw != "checked")echo "disabled";?>>
								<?php
									$value_array = array("20KB","50KB","100KB","200KB","500KB","1MB","2MB");
									foreach($value_array as $value){
										$selected = "";
										if($sys_log_maxsize === $value)
											$selected = "selected";
										echo "<option value=\"$value\" $selected>$value</option>";
									}
								?>
							</select>
						</td>
					</tr></table>
				</td>
			</tr>
		</table>

		<br>
		
		<div <?php echo $show_log;?>>
			<div id="tab">
				<li class="tb1">&nbsp;</li>
				<li class="tbg"><?php echo language('Asterisk Logs');?></li>
				<li class="tb2">&nbsp;</li>
			</div>

			<table width="100%" class="tedit" >
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Verbose');?>:
							<span class="showhelp">
							<?php echo language('Asterisk Logs Verbose help','Asterisk console verbose message switch.');?>
							</span>
						</div>
					</th>
					<td >
							<input type=checkbox name="verbose_sw" id="verbose_sw" <?php echo $verbose_sw?> >
					</td>				
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Notice');?>:
							<span class="showhelp">
							<?php echo language('Asterisk Logs Notice help','Asterisk console notice message  switch.');?>
							</span>
						</div>
					</th>
					<td >
						<input type=checkbox name="notice_sw" id="notice_sw" <?php echo $notice_sw?> >
					</td>  			
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Warning');?>:
							<span class="showhelp">
							<?php echo language('Asterisk Logs Warning help','Asterisk console warning message  switch.');?>
							</span>
						</div>
					</th>
					<td >
						<input type=checkbox name="warning_sw" id="warning_sw" <?php echo $warning_sw?> >
					</td>				
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Debug');?>:
							<span class="showhelp">
							<?php echo language('Asterisk Logs Debug help','Asterisk console debug message switch.');?>
							</span>
						</div>
					</th>
					<td >
						<input type=checkbox name="debug_sw" id="debug_sw" <?php echo $debug_sw?>  >
					</td>				
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Error');?>:
							<span class="showhelp">
							<?php echo language('Asterisk Logs Error help','Asterisk console error message  switch.');?>
							</span>
						</div>
					</th>
					<td >
						<input type=checkbox name="error_sw" id="error_sw" <?php echo $error_sw?> >
					</td>			
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('DTMF');?>:
							<span class="showhelp">
							<?php echo language('Asterisk Logs DTMF help','Asterisk console DTMF info switch.');?>
							</span>
						</div>
					</th>
					<td >
						<input type=checkbox name="dtmf_sw" id="dtmf_sw" <?php echo $dtmf_sw?> >
					</td>				
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Auto clean');?>:
							<span class="showhelp">
							<?php echo language('Auto clean help','
								switch on : when the size of log file reaches the max size, <br> 
								&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; the system will cut a half of the file. New logs will be retained.<br>
								switch off : logs will remain, and the file size will increase gradually. <br>');
								echo language('Auto clean default@Asterisk Logs','default on, default size=100KB.');
							?>
							</span>
						</div>
					</th>
					<td >
						<table><tr>
							<td style="margin:0px;padding:0px;border:0">
								<input type=checkbox id="ast_log_autoclean_sw" name="ast_log_autoclean_sw" <?php echo $ast_log_autoclean_sw?> >
							</td>
							<td style="border:0">
								<?php echo language('maxsize');?> : 
								<select id="ast_log_maxsize" name="ast_log_maxsize" <?php if($ast_log_autoclean_sw != "checked")echo "disabled";?>>
									<?php
										$value_array = array("20KB","50KB","100KB","200KB","500KB","1MB","2MB");
										foreach($value_array as $value){
											$selected = "";
											if($ast_log_maxsize === $value)
												$selected = "selected";
											echo "<option value=\"$value\" $selected>$value</option>";
										}
									?>
								</select>
							</td>
						</tr></table>
					</td>
				</tr>
			</table>

			<br>
		</div>
		
		<div id="tab">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('SIP Logs');?></li>
			<li class="tb2">&nbsp;</li>
		</div>

		<table width="100%" class="tedit" >
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('SIP Logs');?>:
						<span class="showhelp">
						<?php echo language('SIP Logs help','Whether enable or disable SIP log.');?>
						</span>
					</div>
				</th>
				<td>
					<input type=checkbox name="sip_log_sw" <?php echo $sip_log_sw?> >
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Auto clean');?>:
						<span class="showhelp">
						<?php echo language('Auto clean help','
							switch on : when the size of log file reaches the max size, <br> 
							&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; the system will cut a half of the file. New logs will be retained.<br>
							switch off : logs will remain, and the file size will increase gradually. <br>');
							echo language('Auto clean default@SIP Logs','default on, default size=100KB.');
						?>
						</span>
					</div>
				</th>
				<td >
					<table><tr>
						<td style="margin:0px;padding:0px;border:0">
							<input type=checkbox id="sip_log_autoclean_sw" name="sip_log_autoclean_sw" <?php echo $sip_log_autoclean_sw?> >
						</td>
						<td style="border:0">
							<?php echo language('maxsize');?> : 
							<select id="sip_log_maxsize" name="sip_log_maxsize" <?php if($sip_log_autoclean_sw != "checked")echo "disabled";?>>
								<?php
									$value_array = array("20KB","50KB","100KB","200KB","500KB","1MB","2MB");
									foreach($value_array as $value){
										$selected = "";
										if($sip_log_maxsize === $value)
											$selected = "selected";
										echo "<option value=\"$value\" $selected>$value</option>";
									}
								?>
							</select>
						</td>
					</tr></table>
				</td>
			</tr>
		</table>

		<br>
		<div id="tab">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('IAX Logs');?></li>
			<li class="tb2">&nbsp;</li>
		</div>

		<table width="100%" class="tedit" >
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('IAX Logs');?>:
						<span class="showhelp">
						<?php echo language('IAX Logs help','Whether enable or disable IAX log.');?>
						</span>
					</div>
				</th>
				<td>
					<input type=checkbox name="iax_log_sw" <?php echo $iax_log_sw?> >
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Auto clean');?>:
						<span class="showhelp">
						<?php echo language('Auto clean help','
							switch on : when the size of log file reaches the max size, <br> 
							&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; the system will cut a half of the file. New logs will be retained.<br>
							switch off : logs will remain, and the file size will increase gradually. <br>');
							echo language('Auto clean default@IAX Logs','default on, default size=100KB.');
						?>
						</span>
					</div>
				</th>
				<td >
					<table><tr>
						<td style="margin:0px;padding:0px;border:0">
							<input type=checkbox id="iax_log_autoclean_sw" name="iax_log_autoclean_sw" <?php echo $iax_log_autoclean_sw?> >
						</td>
						<td style="border:0">
							<?php echo language('maxsize');?> : 
							<select id="iax_log_maxsize" name="iax_log_maxsize" <?php if($iax_log_autoclean_sw != "checked")echo "disabled";?>>
								<?php
									$value_array = array("20KB","50KB","100KB","200KB","500KB","1MB","2MB");
									foreach($value_array as $value){
										$selected = "";
										if($iax_log_maxsize === $value)
											$selected = "selected";
										echo "<option value=\"$value\" $selected>$value</option>";
									}
								?>
							</select>
						</td>
					</tr></table>
				</td>
			</tr>
		</table>
		
		<br>
		
		<div <?php echo $show_log;?>>
			<div id="tab">
				<li class="tb1">&nbsp;</li>
				<li class="tbg"><?php echo language('BSP Logs');?></li>
				<li class="tb2">&nbsp;</li>
			</div>

			<table width="100%" class="tedit" >
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Notice');?>:
							<span class="showhelp">
							<?php echo language('BSP Logs Notice help','BSP console notice message switch.');?>
							</span>
						</div>
					</th>
					<td >
						<input type="checkbox" name="bsp_notice_sw" id="bsp_notice_sw" <?php echo $bsp_notice_sw;?> >
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Debug');?>:
							<span class="showhelp">
							<?php echo language('BSP Logs Debug help','BSP console debug message switch.');?>
							</span>
						</div>
					</th>
					<td >
						<input type="checkbox" name="bsp_debug_sw" id="bsp_debug_sw" <?php echo $bsp_debug_sw;?> >
					</td>				
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Info');?>:
							<span class="showhelp">
							<?php echo language('BSP Logs Info help','BSP console info message switch.');?>
							</span>
						</div>
					</th>
					<td >
						<input type="checkbox" name="bsp_info_sw" id="bsp_info_sw" <?php echo $bsp_info_sw;?>  >
					</td>				
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Warning');?>:
							<span class="showhelp">
							<?php echo language('BSP Logs Warning help','BSP console warning message switch.');?>
							</span>
						</div>
					</th>
					<td >
						<input type="checkbox" name="bsp_warning_sw" id="bsp_warning_sw" <?php echo $bsp_warning_sw;?> >
					</td>			
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Error');?>:
							<span class="showhelp">
							<?php echo language('BSP Logs Error help','BSP console error message switch.');?>
							</span>
						</div>
					</th>
					<td >
						<input type="checkbox" name="bsp_error_sw" id="bsp_error_sw" <?php echo $bsp_error_sw;?> >
					</td>				
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Auto clean');?>:
							<span class="showhelp">
							<?php echo language('Auto clean help','
								switch on : when the size of log file reaches the max size, <br> 
								&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; the system will cut a half of the file. New logs will be retained.<br>
								switch off : logs will remain, and the file size will increase gradually. <br>');
								echo language('Auto clean default@BSP Logs','default on, default size=100KB.');
							?>
							</span>
						</div>
					</th>
					<td>
						<table><tr>
							<td style="margin:0px;padding:0px;border:0">
								<input type="checkbox" id="bsp_log_autoclean_sw" name="bsp_log_autoclean_sw" <?php echo $bsp_log_autoclean_sw?> >
							</td>
							<td style="border:0">
								<?php echo language('maxsize');?> : 
								<select id="bsp_log_maxsize" name="bsp_log_maxsize" <?php if($bsp_log_autoclean_sw != "checked")echo "disabled";?>>
									<?php
										$value_array = array("20KB","50KB","100KB","200KB","500KB","1MB","2MB");
										foreach($value_array as $value){
											$selected = "";
											if($bsp_log_maxsize === $value)
												$selected = "selected";
											echo "<option value=\"$value\" $selected>$value</option>";
										}
									?>
								</select>
							</td>
						</tr></table>
					</td>
				</tr>
			</table>
			
			<br>
			<div id="tab">
				<li class="tb1">&nbsp;</li>
				<li class="tbg"><?php echo language('RRI Logs');?></li>
				<li class="tb2">&nbsp;</li>
			</div>

			<table width="100%" class="tedit" >
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Notice');?>:
							<span class="showhelp">
							<?php echo language('RRI Logs Notice help','RRI console notice message switch.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="rri_notice_sw" id="rri_notice_sw" <?php echo $rri_notice_sw;?> >
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Debug');?>:
							<span class="showhelp">
							<?php echo language('RRI Logs Debug help','RRI console debug message switch.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="rri_debug_sw" id="rri_debug_sw" <?php echo $rri_debug_sw;?> >
					</td>				
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Info');?>:
							<span class="showhelp">
							<?php echo language('RRI Logs Info help','RRI console info message switch.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="rri_info_sw" id="rri_info_sw" <?php echo $rri_info_sw;?> >
					</td>				
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Warning');?>:
							<span class="showhelp">
							<?php echo language('RRI Logs Warning help','RRI console warning message switch.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="rri_warning_sw" id="rri_warning_sw" <?php echo $rri_warning_sw;?> >
					</td>				
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Error');?>:
							<span class="showhelp">
							<?php echo language('RRI Logs Error help','RRI console error message switch.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="rri_error_sw" id="rri_error_sw" <?php echo $rri_error_sw;?> >
					</td>				
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('RRI Pipe Logs');?>:
							<span class="showhelp">
							<?php echo language('RRI Pipe Logs help','Whether enable or disable RRI Pipe log.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="pipe_log_sw" <?php echo $pipe_log_sw;?> >
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Auto clean');?>:
							<span class="showhelp">
							<?php echo language('Auto clean help','
								switch on : when the size of log file reaches the max size, <br> 
								&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; the system will cut a half of the file. New logs will be retained.<br>
								switch off : logs will remain, and the file size will increase gradually. <br>');
								echo language('Auto clean default@RRI Logs','default on, default size=100KB.');
							?>
							</span>
						</div>
					</th>
					<td >
						<table><tr>
							<td style="margin:0px;padding:0px;border:0">
								<input type="checkbox" id="rri_log_autoclean_sw" name="rri_log_autoclean_sw" <?php echo $rri_log_autoclean_sw?> >
							</td>
							<td style="border:0">
								<?php echo language('maxsize');?> : 
								<select id="rri_log_maxsize" name="rri_log_maxsize" <?php if($rri_log_autoclean_sw != "checked")echo "disabled";?>>
									<?php
										$value_array = array("20KB","50KB","100KB","200KB","500KB","1MB","2MB");
										foreach($value_array as $value){
											$selected = "";
											if($rri_log_maxsize === $value)
												$selected = "selected";
											echo "<option value=\"$value\" $selected>$value</option>";
										}
									?>
								</select>
							</td>
						</tr></table>
					</td>
				</tr>
			</table>
			
			<br>
			<div id="tab">
				<li class="tb1">&nbsp;</li>
				<li class="tbg"><?php echo language('AT Commands Logs');?></li>
				<li class="tb2">&nbsp;</li>
			</div>

			<table width="100%" class="tedit" >
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('AT Commands Logs');?>:
							<span class="showhelp">
							<?php echo language('AT Commands Logs help','Displaying GSM module AT messages.');?>
							</span>
						</div>
					</th>
					<td >
						<input type=checkbox name="debugat_log_sw" <?php echo $debugat_log_sw?> >
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Auto clean');?>:
							<span class="showhelp">
							<?php echo language('Auto clean help','
								switch on : when the size of log file reaches the max size, <br> 
								&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; the system will cut a half of the file. New logs will be retained.<br>
								switch off : logs will remain, and the file size will increase gradually. <br>');
								echo language('Auto clean default@AT Commands Logs','default on, default size=100KB.');
							?>
							</span>
						</div>
					</th>
					<td >
						<table><tr>
							<td style="margin:0px;padding:0px;border:0">
								<input type=checkbox id="at_log_autoclean_sw" name="at_log_autoclean_sw" <?php echo $at_log_autoclean_sw?> > 
							</td>
							<td style="border:0">
								<?php echo language('maxsize');?> : 
								<select id="at_log_maxsize" name="at_log_maxsize" <?php if($at_log_autoclean_sw != "checked")echo "disabled";?>>
									<?php
										$value_array = array("20KB","50KB","100KB","200KB","500KB","1MB","2MB");
										foreach($value_array as $value){
											$selected = "";
											if($at_log_maxsize === $value)
												$selected = "selected";
											echo "<option value=\"$value\" $selected>$value</option>";
										}
									?>
								</select>
							</td>
						</tr></table>
					</td>
				</tr>
			</table>

			<br>
		</div>
		
		<div id="tab">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('Call Detail Record');?></li>
			<li class="tb2">&nbsp;</li>
		</div>

		<table width="100%" class="tedit" >
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Call Detail Record');?>:
						<span class="showhelp">
						<?php echo language('Call Detail Record help','Displaying Call Detail Records for each channel.');?>
						</span>
					</div>
				</th>
				<td >
					<input type=checkbox name="cdr_sw" <?php echo $cdr_sw?> >
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Append IMEI');?>:
						<span class="showhelp">
						<?php echo language('Append IMEI help',"
							switch on : IMEI will be appended to the CDR gsm channel in 'From' or 'To'. <br> 
							switch off : No appended IMEI. <br>					
							default off.");
						?>
						</span>
					</div>
				</th>
				<td >
					<input type=checkbox name="cdr_log_append_imei_sw" <?php echo $cdr_log_append_imei_sw?> >
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Auto clean');?>:
						<span class="showhelp">
						<?php echo language('Auto clean help','
							switch on : when the size of log file reaches the max size, <br> 
							&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; the system will cut a half of the file. New logs will be retained.<br>
							switch off : logs will remain, and the file size will increase gradually. <br>');
							echo language('Auto clean default@Call Detail Record','default on, default size=20MB.');
						?>
						</span>
					</div>
				</th>
				<td >
					<table><tr>
						<td style="margin:0px;padding:0px;border:0">
							<input type=checkbox id="cdr_log_autoclean_sw" name="cdr_log_autoclean_sw" <?php echo $cdr_log_autoclean_sw?> > 
						</td>
						<td style="border:0">
							<?php echo language('maxsize');?> : 
							<select id="cdr_log_maxsize" name="cdr_log_maxsize" <?php if($cdr_log_autoclean_sw != "checked")echo "disabled";?>>
								<?php
									$product_type = get_product_type();
									if($product_type < 4){
										$value_array = array("1MB","2MB","5MB");
									}else{
										$value_array = array("1MB","5MB","10MB");
									}
									foreach($value_array as $value){
										$selected = "";
										if($cdr_log_maxsize === $value)
											$selected = "selected";
										echo "<option value=\"$value\" $selected>$value</option>";
									}
								?>
							</select>
						</td>
					</tr></table>
				</td>
			</tr>
		</table>
		
		<br>
		
		<div id="tab">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('Syslog');?></li>
			<li class="tb2">&nbsp;</li>
		</div>

		<table width="100%" class="tedit" >
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Local Syslog')?>:
						<span class="showhelp">
						<?php echo language('Local Syslog help','Local Syslog');?>
						</span>
					</div>
				</th>
				<td >
					<input type=checkbox name="local_sys" <?php echo $local_sys;?> >
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Server Address')?>:
						<span class="showhelp">
						<?php echo language('Server Address help','Server Address');?>
						</span>
					</div>
				</th>
				<td >
					<input type=text name="serverip" value="<?php echo $serverip;?>" >
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Server Port')?>:
						<span class="showhelp">
						<?php echo language('Server Port help','Server Port');?>
						</span>
					</div>
				</th>
				<td >
					<input type=text name="serverport" value="<?php echo $serverport?>" >
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Syslog Level');?>:
						<span class="showhelp">
						<?php echo language('Syslog Level help','Syslog Level');
						?>
						</span>
					</div>
				</th>
				<td>
					<select name="sysloglevel" id="sysloglevel">
						<option value="emerg" <?php if($sysloglevel=='emerg')echo 'selected';?>>EMERG</option>
						<option value="alert" <?php if($sysloglevel=='alert')echo 'selected';?>>ALERT</option>
						<option value="crit" <?php if($sysloglevel=='crit')echo 'selected';?>>CRIT</option>
						<option value="error" <?php if($sysloglevel=='error')echo 'selected';?>>ERROR</option>
						<option value="warning" <?php if($sysloglevel=='warning')echo 'selected';?>>WARNING</option>
						<option value="notice" <?php if($sysloglevel=='nitice')echo 'selected';?>>NOTICE</option>
						<option value="info" <?php if($sysloglevel=='info')echo 'selected';?>>INFO</option>
						<option value="debug" <?php if($sysloglevel=='debug')echo 'selected';?>>DEBUG</option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('CDR Level');?>:
						<span class="showhelp">
						<?php echo language('CDR Level help','CDR Level');
						?>
						</span>
					</div>
				</th>
				<td>
					<select name="cdrlevel" id="cdrlevel">
						<option value="off" <?php if($cdrlevel=='off')echo 'selected';?>>OFF</option>
						<option value="emerg" <?php if($cdrlevel=='emerg')echo 'selected';?>>EMERG</option>
						<option value="alert" <?php if($cdrlevel=='alert')echo 'selected';?>>ALERT</option>
						<option value="crit" <?php if($cdrlevel=='crit')echo 'selected';?>>CRIT</option>
						<option value="error" <?php if($cdrlevel=='error')echo 'selected';?>>ERROR</option>
						<option value="warning" <?php if($cdrlevel=='warning')echo 'selected';?>>WARNING</option>
						<option value="notice" <?php if($cdrlevel=='nitice')echo 'selected';?>>NOTICE</option>
						<option value="info" <?php if($cdrlevel=='info')echo 'selected';?>>INFO</option>
						<option value="debug" <?php if($cdrlevel=='debug')echo 'selected';?>>DEBUG</option>
					</select>
				</td>
			</tr>
		</table>
		
		<br/>
		
		<input type="hidden" name="send" id="send" value="" />
		<table id="float_btn" class="float_btn">
			<tr id="float_btn_tr" class="float_btn_tr">
				<td>
					<input type="submit"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';"/>
				</td>
			</tr>
		</table>
		<table id="float_btn2" style="border:none;" class="float_btn2">
			<tr id="float_btn_tr2" class="float_btn_tr2">
				<td width="20px">
					<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';" />
				</td>
			</tr>
		</table>
	</form>
	
	<script type="text/javascript">
	$(document).ready(function (){
		$(":checkbox").iButton();
		$("#sys_log_autoclean_sw").change(function(){$("#sys_log_maxsize").attr("disabled", !$("#sys_log_autoclean_sw").attr("checked"));});
		$("#ast_log_autoclean_sw").change(function(){$("#ast_log_maxsize").attr("disabled", !$("#ast_log_autoclean_sw").attr("checked"));});
		$("#sip_log_autoclean_sw").change(function(){$("#sip_log_maxsize").attr("disabled", !$("#sip_log_autoclean_sw").attr("checked"));});
		$("#iax_log_autoclean_sw").change(function(){$("#iax_log_maxsize").attr("disabled", !$("#iax_log_autoclean_sw").attr("checked"));});
		$("#bsp_log_autoclean_sw").change(function(){$("#bsp_log_maxsize").attr("disabled", !$("#bsp_log_autoclean_sw").attr("checked"));});
		$("#rri_log_autoclean_sw").change(function(){$("#rri_log_maxsize").attr("disabled", !$("#rri_log_autoclean_sw").attr("checked"));});
		$("#cdr_log_autoclean_sw").change(function(){$("#cdr_log_maxsize").attr("disabled", !$("#cdr_log_autoclean_sw").attr("checked"));});
		$("#at_log_autoclean_sw").change(function(){$("#at_log_maxsize").attr("disabled", !$("#at_log_autoclean_sw").attr("checked"));});
	});
	</script>
	
<?php
}

if(isset($_POST['send']) || isset($_POST['ser_send'])) {
	if($_POST['send'] == 'Save'){
		/* gw.conf */
		save_to_gw_conf();

		/* logger.conf & extra-channels.conf */
		save_to_logger_conf();

		/* logfile_monitor.conf */
		save_to_logfile_monitor_conf();

		/*extensions_routing.conf*/
		save_routings_to_extensions();
		
		/* syslog */
		save_gw_syslog();

		if(isset($_POST['debug_sw'])){
			wait_apply("exec", "asterisk -rx \"core set debug 99\" > /dev/null 2>&1");
		}
		if(isset($_POST['verbose_sw'])){
			wait_apply("exec", "asterisk -rx \"core set verbose 99\" > /dev/null 2>&1");
		}
		
		if(isset($_POST['iax_log_sw'])){
			wait_apply("exec", "asterisk -rx \"iax2 set debug on\" > /dev/null 2>&1");
		}else{	
			wait_apply("exec", "asterisk -rx \"iax2 set debug off\" > /dev/null 2>&1");
		}
		
		wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");
		wait_apply("exec", "asterisk -rx \"extra restart\" > /dev/null 2>&1 &");
		wait_apply("exec", "/etc/init.d/logfile_monitor restart > /dev/null 2>&1 &");

		/* sync slave config 
		$cluster_info = get_cluster_info();
		if($cluster_info['mode'] == 'master') {
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
					set_slave_file($slaveip,"/etc/asterisk/gw.conf","/etc/asterisk/gw.conf");
					set_slave_file($slaveip,"/etc/asterisk/logger.conf","/etc/asterisk/logger.conf");
					set_slave_file($slaveip,"/etc/asterisk/extra-channels.conf","/etc/asterisk/extra-channels.conf");
					set_slave_file($slaveip,"/etc/asterisk/gw/logfile_monitor.conf","/etc/asterisk/gw/logfile_monitor.conf");

					if(isset($_POST['debug_sw'])){
						wait_apply("request_slave", $slaveip, "syscmd:asterisk -rx \"core set debug 99\" > /dev/null 2>&1");
					}
					if( isset($verbose_true) && $verbose_true ) {
						wait_apply("request_slave", $slaveip, "syscmd:asterisk -rx \"core set verbose 99\" > /dev/null 2>&1");
					}
					
					if( isset($iax_log_true)) {
						if ($iax_log_true)
							wait_apply("exec", "asterisk -rx \"iax2 set debug on\" > /dev/null 2>&1");
						else	
							wait_apply("exec", "asterisk -rx \"iax2 set debug off\" > /dev/null 2>&1");
					}
					
					wait_apply("request_slave", $slaveip, "syscmd:asterisk -rx \"core reload\" > /dev/null 2>&1 &");
					wait_apply("request_slave", $slaveip, "syscmd:asterisk -rx \"extra restart\" > /dev/null 2>&1 &");
					wait_apply("request_slave", $slaveip, "syscmd:/etc/init.d/logfile_monitor restart > /dev/null 2>&1 &");
				}
			}
		}
		*/
	}
	show_logsetting();
}else{
	show_logsetting();
}
?>



<?php require("/www/cgi-bin/inc/boot.inc");?>
<div id="float_btn1" class="float_btn1 sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()">
</div>