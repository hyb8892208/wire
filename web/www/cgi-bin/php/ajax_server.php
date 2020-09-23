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
//initialize the language
$language = get_web_language_cache('/tmp/web/language.cache');

if(file_exists('/tmp/web/language.debug')){
	$language_debug = 'true';
}else{
	$language_debug = 'false';
}
function start_capture()
{	
	
	if(isset($_GET['interface_type'])) {
		$interface = trim($_GET['interface_type']);
	} else {
		$interface = 'eth0';
	}
	if(isset($_GET['source_host'])) {
		$source_host = trim($_GET['source_host']);
	} else {
		$source_host = '';
	}
	if(isset($_GET['dest_host'])) {
		$dest_host = trim($_GET['dest_host']);
	} else {
		$dest_host = '';
	}
	if(isset($_GET['protocol'])) {
		$protocol = trim($_GET['protocol']);
	} else {
		$protocol = '';
	}
	if(isset($_GET['cap_port'])) {
		$port = trim($_GET['cap_port']);
	} else {
		$port = '';
	}
	
	$product_type = get_product_type();
	
	$cmd ='';
	
	if($product_type < 4){
		$cmd.= "tcpdump -Z root -i ";
	}else{
		$cmd.= "/my_tools/tcpdump -Z root -i ";
	}

	$cmd.= $interface." ";
	$source = explode(',', $source_host);
	   if ($source[0]!=""){ 
      $cmd.=" src host "; 
      $tmpstr=""; 
      for ($i=0;$i<count($source);$i++){ 
         $tmpstr.=" or ".$source[$i]; 
      }    
      $tmpstr=ltrim($tmpstr,' or '); 
      $cmd.=$tmpstr; 
   } 
   $destination = explode(',', $dest_host);  
   if ($destination[0]!=""){ 
      if ($source[0]!=""){ 
         $cmd.=" and"; 
      }       
      $cmd.=" dst host ";    
      $tmpstr=""; 
      for ($i=0;$i<count($destination);$i++){ 
         $tmpstr.=" or ".$destination[$i]; 
      }    
       
      $tmpstr=substr($tmpstr,strlen(' or '),100000);       
      $cmd.=$tmpstr; 
   }

	if($protocol != '') {
		if(($destination[0] != '' || $source[0] != '') && $protocol != 'all' && $protocol != 'rtp' && $protocol != 'rtcp' && $protocol != 'sip'){
			$cmd .= " and ";
		}

		$tmpstr = '';
		if($protocol != 'all' ) {
			if($protocol == 'rtp' || $protocol == 'rtcp') {
				$tmpstr .= " -T ";
			}
			if ($protocol == 'sip') {
				$tmpstr .= "";
			} else {
				$tmpstr .= $protocol;
			}
		} 
		if($source[0] == "" && $destination[0]){
			$tmpstr .= " ".$tmpstr;
		}
		$cmd .= $tmpstr;
	}	

	if ($port != '') {
		if ($source[0] != '' || $destination[0] != '') {
			$cmd .= " and ";
		}
		$cmd .= " port ".$port;
	}
	$cmd .= " -C 30 -w /tmp/network_data.pcap";
	$cmd .= " > /dev/null 2>&1 &";
	exec($cmd);
//	echo "<br/>"; echo language("Starting capture the message from channel");echo " ".$channel." ......<br/>";	
//	exec("/etc/init.d/capture_monitor start $channel &");
//	exec("/etc/init.d/cron restart > /dev/null 2>&1 &");
}
function stop_capture()
{
	$tcpdump= 'tcpdump';
	//$cmd1 = "kill -INT `ps -ef | grep $tcpdump | grep -v grep | awk '{print $1}'`";
#	$cmd2 = "openvox -rx \"dahdi set tcpdump stop $channel\" > /dev/null 2>&1 &";
	$cmd = "kill -s 2 `pidof tcpdump`";
//	exec("/etc/init.d/capture_monitor stop &");
	exec("$cmd > /dev/null 2>&1 || echo $?", $output1);
	//system("kill -s 2 `pidof tcpdump`");
	if($output1){
		$Error = language('Error');
		$kill_error = "Fail to exec kill";
		$kill_error .= ": ".$output1[0];
		trace_output_start("$Error", "$kill_error");
		trace_output_end();
		return;
	}
	exec("sleep 1");
	back_pcap_file();
}
function back_pcap_file()
{

	//$pcap_file_name = "network_data.pcap";
	$pcap_file_name = "tcpdump_pcap.tar.gz";

	$pcap_file_path = "/tmp/$pcap_file_name";
	
	$pack_cmd = "tar vcz -f $pcap_file_path /tmp/network_data.pcap";
	exec("$pack_cmd > /dev/null 2>&1 || echo $?",$output);
	if($output) {
		echo "</br>$pcap_file_name ";
		echo language("Packing was failed");echo "</br>";
		echo language("Error code");echo ": ".$output[0];
		return;
	}

	if(!file_exists($pcap_file_path)) {
		$Error = language('Error');
		$file_error = "</br>$pcap_file_name";
		$file_error .= language('Can not find');
		trace_output_start("$Error", $file_error);
		trace_output_end();
		return;
	}
	//´ò¿ªÎÄ¼þ  
	$fd = fopen ($pcap_file_path, "r" ); 
	$size = filesize($pcap_file_path) ;

	//ÊäÈëÎÄ¼þ±êÇ© 
	header('Content-Encoding: none');
	header('Content-Type: application/force-download');
	header('Content-Type: application/octet-stream');
	header('Content-Type: application/download');
	header('Content-Description: File Transfer');  
	header('Accept-Ranges: bytes');  
	header( "Accept-Length: $size");  
	header( 'Content-Transfer-Encoding: binary' );
	header( "Content-Disposition: attachment; filename=$pcap_file_name" ); 
	header('Pragma: no-cache');
	header('Expires: 0');
	
	//Êä³öÎÄ¼þÄÚÈÝ   
	//¶ÁÈ¡ÎÄ¼þÄÚÈÝ²¢Ö±½ÓÊä³öµ½ä¯ÀÀÆ÷
	ob_clean();
	flush();
	echo fread($fd, $size);
	fclose ($fd);

	unlink($pcap_file_path);
}
function start_sipdump()
{	
	
	if(isset($_GET['interface'])) {
		$interface = trim($_GET['interface']);
	} else {
		$interface = '';
	}
	if(isset($_GET['invite']) && $_GET['invite'] != '') {
		$parameters['invite'] = trim($_GET['invite']);
	} 
	if(isset($_GET['options']) && $_GET['options'] != '') {
		$parameters['options'] = trim($_GET['options']);
	} 
	if(isset($_GET['register']) && $_GET['register'] != '') {
		$parameters['register'] = trim($_GET['register']);
	}
	// create /data/capture to save sip pcap
	$parameters_num = count($parameters);
	$cmd = "mkdir /data/capture -p";
	exec($cmd);

	$cmd ='';
	$cmd.= "/my_tools/sipdump -i ";

	$cmd.= $interface." ";
	$cmd.= "-d /data/capture ";
	$parameter_content = '';
	if($parameters_num == 1){
		foreach ($parameters as $key => $value) {
			$parameter_content = $value;
		}
		$cmd.= "-m \"^$parameter_content$\"";
	} else {
		foreach ($parameters as $key => $value) {
			$parameter_content .= $value."|";
		}
		$parameter_content = substr($parameter_content, 0, -1); // remove the  suffix '|'
		$cmd.="-m \"^($parameter_content)$\"";
	}
 
	$cmd .= " > /dev/null 2>&1 &";
	exec($cmd);

}
function stop_sipdump()
{
	$cmd = "kill -s 2 `pidof sipdump`";
//	exec("/etc/init.d/capture_monitor stop &");
	exec("$cmd > /dev/null 2>&1 || echo $?", $output1);
	//system("kill -s 2 `pidof tcpdump`");
	if($output1){
		$Error = language('Error');
		$kill_error = "Fail to exec kill";
		$kill_error .= ": ".$output1[0];
		trace_output_start("$Error", "$kill_error");
		trace_output_end();
		return;
	}
	exec("sleep 1");
	back_sippcap_file();
}
function back_sippcap_file()
{

	//$pcap_file_name = "voicemsg_".$date."_".$channel.".pcap";
	$pcap_file_name = "sipdump_pcap.tar.gz";

	$pcap_file_path = "/data/$pcap_file_name";

	$pack_cmd = "tar vcz -f $pcap_file_path -C /data ./capture/";
	exec("$pack_cmd > /dev/null 2>&1 || echo $?",$output);
	//exec("$pack_cmd || echo $?",$output);
	if($output) {
		echo "</br>$pcap_file_name ";
		echo language("Packing was failed");echo "</br>";
		echo language("Error code");echo ": ".$output[0];
		return;
	}
	exec("rm -rf /data/capture"); 
	if(!file_exists($pcap_file_path)) {
		$Error = language('Error');
		$file_error = "</br>$pcap_file_name";
		$file_error .= language('Can not find');
		trace_output_start("$Error", $file_error);
		trace_output_end();
		return;
	}
	//´ò¿ªÎÄ¼þ  
	$fd = fopen ($pcap_file_path, "r" ); 
	$size = filesize($pcap_file_path) ;

	//ÊäÈëÎÄ¼þ±êÇ© 
	header('Content-Encoding: none');
	header('Content-Type: application/force-download');
	header('Content-Type: application/octet-stream');
	header('Content-Type: application/download');
	header('Content-Description: File Transfer');  
	header('Accept-Ranges: bytes');  
	header( "Accept-Length: $size");  
	header( 'Content-Transfer-Encoding: binary' );
	header( "Content-Disposition: attachment; filename=$pcap_file_name" ); 
	header('Pragma: no-cache');
	header('Expires: 0');
	
	//Êä³öÎÄ¼þÄÚÈÝ   
	//¶ÁÈ¡ÎÄ¼þÄÚÈÝ²¢Ö±½ÓÊä³öµ½ä¯ÀÀÆ÷
	ob_clean();
	flush();
	echo fread($fd, $size);
	fclose ($fd);

	unlink($pcap_file_path);
}
function show_gsms()
{
	$alldata = get_all_gsm_info();

	return $alldata;
}



function get_imei_all()
{
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	global $__GSM_HEAD__;
	global $__GSM_SUM__;
	global $__deal_cluster__;
	$imei_array = "";

	$run = false;
	if(ast_running()) {
		$run = true;
	}

	for($i=1; $i<=$__GSM_SUM__; $i++) {
		if($run) {
			$handle = @fopen("/tmp/gsm/$i","r");
			@flock($handle,LOCK_EX);
			$states = @fread($handle, @filesize("/tmp/gsm/$i"));
			@flock($handle,LOCK_UN);
			@fclose($handle);
		} else {
			$states = '';
		}
		//$states = `asterisk -rx "gsm show span $i" 2> /dev/null`;
		$imei_array[1][$i]['imei'] = trim(__cut_str($states,"Model IMEI:","\n"));
		$state = trim(__cut_str($states,"State:","\n"));
		if(trim($state) == 'READY') {
			$imei_array[1][$i]['disable_modifyimei'] = false;
		}else{
			$imei_array[1][$i]['disable_modifyimei'] = true;
		}
	}

	if($__deal_cluster__){
		$cluster_info = get_cluster_info();
		if($cluster_info['mode'] == 'master') {
			for($b=2; $b<=$__BRD_SUM__; $b++){
				if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != ''){
					$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
					$all_states = request_slave($slaveip,"get_spans\n", 1); 
					for($i=1; $i<=$__GSM_SUM__; $i++) {
						$states = __cut_str($all_states,"---Start Span $i---","---End Span $i---");
						$imei_array[$b][$i]['imei'] = trim(__cut_str($states,"Model IMEI:","\n"));
						$state = trim(__cut_str($states,"State:","\n"));
						if(trim($state) == 'READY') {
							$imei_array[$b][$i]['disable_modifyimei'] = false;
						}else{
							$imei_array[$b][$i]['disable_modifyimei'] = true;
						}
					}
				}
			}   
		}
	}
	
	return $imei_array;
}

function manually_modify_imei()
{
	global $__BRD_HEAD__;
	global $__deal_cluster__;
	if(!isset($_GET['board']) || !isset($_GET['channel']) || !isset($_GET['new_imei']) || !isset($_GET['force'])){
		return false;
	}
	$board = trim($_GET['board']);
	$channel = trim($_GET['channel']);
	$new_imei = trim($_GET['new_imei']);
	$force = trim($_GET['force']);

	if($board == 1){
		return modify_imei($channel, $new_imei, 'power_reset',$force);
	}else{
		if($__deal_cluster__){
			$cluster_info = get_cluster_info();
			if($cluster_info['mode'] == 'master') {
				if($cluster_info[$__BRD_HEAD__.$board.'_ip'] != ''){
					$slaveip = $cluster_info[$__BRD_HEAD__.$board.'_ip'];
					return modify_imei($channel, $new_imei, 'power_reset',$force, $slaveip);
				}
			}
		}
	}
	return false;
}

function get_system_time()
{
        $all_time = `date "+%Y:%m:%d:%H:%M:%S"`;
        $item = explode(':', $all_time, 6); 
        if(isset($item[5])) {
                $year = $item[0];
                $month = $item[1];
                $date = $item[2];
                $hour = $item[3];
                $minute = $item[4];
                $second = $item[5];
        }
	return "$year-$month-$date $hour:$minute:$second";
}
function get_state_cloud(){

	 $contents = trim(@file_get_contents('/etc/config/cloud_status'));
	if($contents=='connect cloud Server success'){
		echo  'OK';die;
	}
	else{
		echo $contents;die;
	}
}
function get_pptpvpn_status()
{
	$ppp_arr = array();
	exec("/my_tools/net_tool ppp0", $ppp_arr);
	if($ppp_arr[3] != ''){
		echo 'OK';
	}else{
		echo 'Failed to connect';
	}
}

function get_openvpn_status(){
	$info = file_get_contents('/tmp/log/openvpn.log');
	if(strstr($info,'Initialization Sequence Completed')){
		echo 'OK';
	}else{
		echo "Failed to connect";
	}
}

function get_n2n_status(){
	$info = file_get_contents('/tmp/n2n.status');
	if(strstr($info,'success')){
		echo 'OK';
	}else{
		echo 'Failed to connect';
	}
}

function sync_time_from_client()
{
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $__deal_cluster__;

	$ts = sprintf("%04d%02d%02d%02d%02d.%02d",$_GET['local_yea'],$_GET['local_mon'],$_GET['local_dat'],$_GET['local_hou'],$_GET['local_min'],$_GET['local_sec']);
	$cmd = "date -s \"$ts\"";
	$res = '';
	$final_show ='';
	exec("$cmd > /dev/null 2>&1 && echo -n 'ok'", $res);
	exec("/sbin/hwclock -w > /dev/null 2>&1");
	if($res[0] == 'ok') {
		$final_show  = language("Client Synchronize Succeeded");

		if($__deal_cluster__){
			$cluster_info = get_cluster_info();
			if($cluster_info['mode'] == 'master') {
				for($b=2; $b<=$__BRD_SUM__; $b++) {
					if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
						$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
						set_slave_file($slaveip,"/etc/asterisk/gw/time.conf","/etc/asterisk/gw/time.conf");
						set_slave_file($slaveip,"/etc/cfg/gw/time.conf","/etc/cfg/gw/time.conf");
						set_slave_file($slaveip,"/tmp/TZ","/tmp/TZ");
						$data = "syscmd:date -s \"$ts\" > /dev/null 2>&1 && echo -n 'ok'";
						$ret = request_slave($slaveip, $data, 5, true);
						if(trim($ret) == 'ok'){
							$final_show .= language("Slave");
							$final_show .= " $slaveip ";
							$final_show .= language('Client Synchronize Succeeded');
						}else{
							$final_show .= language("Slave");
							$final_show .= " $slaveip ";
							$final_show .= language('Client Synchronize Failed');
						}
					}    
				}    
			}
		}
		
	}
	echo $final_show;
}
function sync_time_from_ntp()
{
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $__deal_cluster__;
	$language = get_web_language_cache('/tmp/web/language.cache');
	$final_show ='';
	$ntpserver[1] = trim($_GET['ntp_server1']);
	$ntpserver[2] = trim($_GET['ntp_server2']);
	$ntpserver[3] = trim($_GET['ntp_server3']);
	for($i=1;$i<=3;$i++){
		if($ntpserver[$i] != '') {
			$ret = `/my_tools/ntpclient -h "$ntpserver[$i]" -s > /dev/null 2>&1 && echo -n 'ok'`;
			if($ret == 'ok') {
				$final_show  = "NTP: [".$ntpserver[$i]."] ";
				$final_show .= language("Synchronize Succeeded");
				
				if($__deal_cluster__){
					$cluster_info = get_cluster_info();
					if($cluster_info['mode'] == 'master') {
						for($b=2; $b<=$__BRD_SUM__; $b++) {
							if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
								$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
								set_slave_file($slaveip,"/etc/asterisk/gw/time.conf","/etc/asterisk/gw/time.conf");
								set_slave_file($slaveip,"/etc/cfg/gw/time.conf","/etc/cfg/gw/time.conf");
								set_slave_file($slaveip,"/tmp/TZ","/tmp/TZ");
								set_slave_file($slaveip,"/tmp/localtime","/tmp/localtime");
								$str = trim(`date +%m%d%H%M%Y.%S`);
								$data = "syscmd:date -I \"$str\" > /dev/null 2>&1 && echo -n 'ok'";
								$ret = request_slave($slaveip, $data, 5, true);
								if(trim($ret) == 'ok') {
									$final_show .= language("Slave");
									$final_show .= " $slaveip ";
									$final_show .= language("Synchronize Succeeded");
								}
							}    
						}    
					}
				}
					
				break;
			}
		}
	}
	exec("/sbin/hwclock -w > /dev/null 2>&1");
	echo $final_show;

}

function sync_time_from_station(){
	$res = exec("/my_tools/station_time > /dev/null  && echo $?", $output);
	if($res == '0'){
		echo language("Synchronize Succeeded");
	}else{
		echo language('Synchronize Failed');
	}
}

function process_log()
{
	//Test
	//Show
	//http://172.16.99.1/cgi-bin/php/ajax_server.php?type=log&log_type=at_log&board=1&port=1&size=100
	//Delete
	//http://172.16.99.1/cgi-bin/php/ajax_server.php?type=log&log_type=at_log&board=1&port=1&action_type=delete
	//Get contents
	//http://172.16.99.1/cgi-bin/php/ajax_server.php?type=log&log_type=at_log&board=1&port=1&action_type=getcontents
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	global $__GSM_SUM__;
	global $__deal_cluster__;
	$slaveip = '';
	$logfile = '';
	$contents = '';
	$oldsize = 0;

	if(isset($_GET['action'])){
		$redis_client = new Predis\Client();
		if($redis_client) {
			if(trim($_GET['action']) == 'enable_astlog') {
				$redis_client->hset('local.product.oem.ver.ctl', 'enable_ast_logsettings', 'on');
				return  '<p>enable asterisk log settings, AT-log settings, log-ast.php and log-debugat.php</p>';
			} else if (trim($_GET['action']) == 'disable_astlog') {
				$redis_client->hset('local.product.oem.ver.ctl', 'enable_ast_logsettings', 'off');
				return '<p>disable asterisk log settings, AT-log settings, log-ast.php and log-debugat.php</p>';
			}
		}
		
		if($redis_client) {
			if(trim($_GET['action']) == 'enable_emulog'){
				$redis_client->hset('local.product.oem.ver.ctl', 'enable_emu_logsettings', 'on');
				return '<p>enable simemusvr log settings</p>';
			}else if (trim($_GET['action']) == 'disable_emulog'){
				$redis_client->hset('local.product.oem.ver.ctl', 'enable_emu_logsettings', 'off');
				return '<p>disable simemusvr log settings</p>';
			}
		}
	}
	
	if(!isset($_GET['log_type'])) return '';

	if($_GET['log_type'] == 'at_log') {
		if( isset($_GET['board']) && $_GET['board']>=1 && $_GET['board']<=$__BRD_SUM__ &&
		    isset($_GET['port']) && $_GET['port']>=1 && $_GET['port']<=$__GSM_SUM__ ) {
			$board = $_GET['board'];
			$port = $_GET['port'];
			
			if($__deal_cluster__){
				if($board != 1) {
					$cluster_info = get_cluster_info();
					if($cluster_info['mode'] == 'master') {
						if(isset($cluster_info[$__BRD_HEAD__.$board.'_ip']) && $cluster_info[$__BRD_HEAD__.$board.'_ip'] != '') {
							$slaveip = $cluster_info[$__BRD_HEAD__.$board.'_ip'];
							$log_type = $_GET['log_type'];
							$action_type = isset($_GET['action_type']) ? $_GET['action_type'] : '';
							$size = isset($_GET['size']) ? $_GET['size'] : '';
							return slave_process_log($slaveip, $log_type, $action_type, $size, $port);
						}
					}
				}
			}
			
			$logfile = "/tmp/log/asterisk/at/$port";
		}
	} else if($_GET['log_type'] == 'sip_log') {
		if( isset($_GET['board']) && $_GET['board']>=1 && $_GET['board']<=$__BRD_SUM__ ) {
			$board = $_GET['board'];
			
			if($__deal_cluster__){
				if($board != 1) {
					$cluster_info = get_cluster_info();
					if($cluster_info['mode'] == 'master') {
						if(isset($cluster_info[$__BRD_HEAD__.$board.'_ip']) && $cluster_info[$__BRD_HEAD__.$board.'_ip'] != '') {
							$slaveip = $cluster_info[$__BRD_HEAD__.$board.'_ip'];
							$log_type = $_GET['log_type'];
							$action_type = isset($_GET['action_type']) ? $_GET['action_type'] : '';
							$size = isset($_GET['size']) ? $_GET['size'] : '';
							return slave_process_log($slaveip, $log_type, $action_type, $size, 0);
						}
					}
				}
			}
			
			$logfile = '/tmp/log/asterisk/sip-log';
		}
	} else if ($_GET['log_type'] == 'iax_log') {
		if( isset($_GET['board']) && $_GET['board']>=1 && $_GET['board']<=$__BRD_SUM__ ) {
			$board = $_GET['board'];
			
			if($__deal_cluster__){
				if($board != 1) {
					$cluster_info = get_cluster_info();
					if($cluster_info['mode'] == 'master') {
						if(isset($cluster_info[$__BRD_HEAD__.$board.'_ip']) && $cluster_info[$__BRD_HEAD__.$board.'_ip'] != '') {
							$slaveip = $cluster_info[$__BRD_HEAD__.$board.'_ip'];
							$log_type = $_GET['log_type'];
							$action_type = isset($_GET['action_type']) ? $_GET['action_type'] : '';
							$size = isset($_GET['size']) ? $_GET['size'] : '';
							return slave_process_log($slaveip, $log_type, $action_type, $size, 0);
						}
					}
				}
			}
			
			$logfile = '/tmp/log/asterisk/sip-log';
		}	
	} else if($_GET['log_type'] == 'ast_log') {
		if( isset($_GET['board']) && $_GET['board']>=1 && $_GET['board']<=$__BRD_SUM__ ) {
			$board = $_GET['board'];
			
			if($__deal_cluster__){
				if($board != 1) {
					$cluster_info = get_cluster_info();
					if($cluster_info['mode'] == 'master') {
						if(isset($cluster_info[$__BRD_HEAD__.$board.'_ip']) && $cluster_info[$__BRD_HEAD__.$board.'_ip'] != '') {
							$slaveip = $cluster_info[$__BRD_HEAD__.$board.'_ip'];
							$log_type = $_GET['log_type'];
							$action_type = isset($_GET['action_type']) ? $_GET['action_type'] : '';
							$size = isset($_GET['size']) ? $_GET['size'] : '';
							return slave_process_log($slaveip, $log_type, $action_type, $size, 0);
						}
					}
				}
			}
			
			$logfile = '/tmp/log/asterisk/log4gw';
		}
	} else if( $_GET['log_type'] == 'sys_log') {
		if( isset($_GET['board']) && $_GET['board']>=1 && $_GET['board']<=$__BRD_SUM__ ){
			$board = $_GET['board'];
			
			if($__deal_cluster__){
				if($board != 1) {
					$cluster_info = get_cluster_info();
					if($cluster_info['mode'] == 'master') {
						if(isset($cluster_info[$__BRD_HEAD__.$board.'_ip']) && $cluster_info[$__BRD_HEAD__.$board.'_ip'] != '') {
							$slaveip = $cluster_info[$__BRD_HEAD__.$board.'_ip'];
							$log_type = $_GET['log_type'];
							$action_type = isset($_GET['action_type']) ? $_GET['action_type'] : '';
							$size = isset($_GET['size']) ? $_GET['size'] : '';
							return slave_process_log($slaveip, $log_type, $action_type, $size, 0);
						}
					}
				}
			}
			
			$logfile = '/data/log/sys-log';
		}
	}

	if($logfile == '') return '';

	if(isset($_GET['action_type'])) {
		if( $_GET['action_type'] == 'delete' ) {
			exec("cat /dev/null > $logfile");
			return '0&';
		} else if( $_GET['action_type'] == 'getcontents' ) {
			$contents = @file_get_contents($logfile);
			return $contents;
		}
	}

	if(!isset($_GET['size'])) return '';

	$oldsize = $_GET['size'];
	$newsize = @filesize($logfile);
	if( $newsize == $oldsize) {
		$contents = '';
	} else if($newsize > $oldsize ) {
		$contents = @file_get_contents($logfile, false, NULL, $oldsize);
	} else {
		$contents = @file_get_contents($logfile);
	}

	return "$newsize&$contents";
}

function simemusvr(){
	$url = "/tmp/log/SimEmuSvr.log";
	$size=$_GET['size'];
	$filesize=filesize($url);
	
	if ($size==0){
		$content=file_get_contents($url);
		echo $url;
		echo $content;
		$tmpsize=0;
	}else if ($filesize<$size) {
		$content=file_get_contents($url);
		$tmpsize="-".$filesize;
		
	}else{
		$content=file_get_contents($url,false,null,$size,$filesize-$size);
		$content=file_get_contents($url,false,null,$filesize-$size,$size);
		$tmpsize=$filesize;
	}
	
	if ($_GET['method']=='update'){
		if ($size==$filesize) {
			$content='';
		}				
		$content=$tmpsize."&".$content;
	}			
		
	echo $content;
	exit(0);
}

function module_update_exit(){
	sleep(1);
	
	$sys_type = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys sys_type");
	if($sys_type == 3){
		global $__GSM_SUM__;
		exec("/my_tools/vs_usb_channel_set.sh $__GSM_SUM__");
	}
	
	exec("/etc/init.d/asterisk restart");
	$temp = json_decode($_POST['filename']);
	for($i=0;$i<count($temp);$i++){
		if(file_exists("/www/chn_".$temp[$i])){
			unlink("/www/chn_".$temp[$i]);
		}
		if(file_exists("/www/upgrade_module_".$temp[$i].'.log')){
			unlink("/www/upgrade_module_".$temp[$i].'.log');
		}
		exec("/my_tools/exit_upgrade_module_m35.sh $temp[$i]");
		exec("/my_tools/exit_upgrade_module_ec20.sh $temp[$i]");
		exec("/my_tools/exit_upgrade_module_sim6320c.sh $temp[$i]");
	}
	
	$product_type = get_product_type();
	if($product_type < 4){
		exec("rm -rf /tmp/module/");
	}else{
		exec("rm -rf /data/module/");
	}
	echo 'success';
}

function module_update(){
	$channel = $_POST['channel'];
	$filename = $_POST['filename'];
	$module_type = $_POST['module_type'];
	
	$product_type = get_product_type();
	if($product_type < 4){
		$module_path = "/tmp/module/";
	}else{
		$module_path = "/data/module/";
	}
	
	$store_file = $module_path.$filename;
	
	if(file_exists("/www/chn_".$channel)){
		unlink("/www/chn_".$channel);
	}
	if($module_type == 'gsm'){
		exec("/my_tools/exit_upgrade_module_m35.sh $channel");
		exec("/my_tools/upgrade_module_m35.sh $channel $store_file");
	}else if($module_type == 'lte'){
		exec("/my_tools/exit_upgrade_module_ec20.sh $channel");
		exec("/my_tools/upgrade_module_ec20.sh $channel $module_path");
	}else if($module_type == 'cdma'){
		exec("/my_tools/upgrade_module_sim6320c.sh $channel $store_file");
	}else if($module_type == 'umts'){
		exec("/my_tools/upgrade_module_uc15.sh $channel $module_path");
	}
}

function mcu_update(){
	$port = $_POST['channel'];
	$file_name = $_POST['filename'];
	$module_type = $_POST['module_type'];
	$store_file = "/data/module/".$file_name;
	
	if(file_exists("/www/upgrade_module_mcu_".$file_name.".log")){
		unlink("/www/upgrade_module_mcu_".$file_name.".log");
	}
	
	//if($module_type == 'lte'){
		exec("/my_tools/upgrade_module_mcu.sh $port $store_file", $output);
		echo $output[0];
	//}else{
		//echo 'error';
	//}
}

function mcu_update_exit(){
	$temp = json_decode($_POST['filename']);
	for($i=0;$i<count($temp);$i++){
		if(file_exists("/www/upgrade_module_mcu_".$temp[$i].'.log')){
			unlink("/www/upgrade_module_mcu_".$temp[$i].'.log');
		}
	}
	
	exec("/my_tools/exit_upgrade_module_mcu.sh");
	exec("rm -rf /data/module/");
}

function mcu_exit_upgrade(){
	exec("/my_tools/exit_upgrade_module_mcu.sh");
}

function get_internet_used(){
	$aql = new aql();
	$aql->set('basedir', '/etc/asterisk/');
	$res = $aql->query("select * from gw_internet.conf");
	
	$json = $_POST['channel'];
	$channel = json_decode($json);
	$arr = [];
	for($i=0;$i<count($channel);$i++){
		$port = 'port'.$channel[$i];
		if(isset($res[$port]['flow_use_size'])){
			$arr[$channel[$i]] = [$res[$port]['flow_use_size']];
		}
	}
	echo json_encode($arr);
}

function match_test(){
	$query_type = $_POST['query_type'];
	$sms_content = str_replace('"',"'",$_POST['sms_content']);
	$match_key_info = $_POST['match_key_info'];
	
	$cmd = '';
	if(isset($_GET['balance'])){
		$point_char = $_POST['point_char'];
		$thousandth_char = $_POST['thousandth_char'];
		$cmd = "/my_tools/sim_query -o$query_type -m'$sms_content' -k'$match_key_info' -p$point_char -t$thousandth_char ";
	}else if(isset($_GET['phonenum'])){
		$cmd = "/my_tools/sim_query -o$query_type -m'$sms_content' -k'$match_key_info'";
	}
	
	exec($cmd,$output);
	echo $output[0];
}

function get_public_ip(){
	$aql = new aql();
	$aql->set('basedir', '/etc/asterisk/');
	$gw_general_conf_path = '/etc/asterisk/gw_general.conf';
	$hlock = lock_file($gw_general_conf_path);
	if(!file_exists($gw_general_conf_path)) {exec('touch /etc/asterisk/gw_general.conf');}
	
	$res = $aql->query("select * from gw_general.conf");
	unlock_file($hlock);
	
	if(isset($res['auto_extern']['externaddr'])){
		$flag = $res['auto_extern']['externaddr'];
	}else{
		$flag = 0;
	}
	
	$sip_general_conf_path = '/etc/asterisk/sip_general.conf';
	$hlock = lock_file($sip_general_conf_path);
	
	$res = $aql->query("select * from sip_general.conf");
	unlock_file($hlock);
	
	if(isset($res['general']['externaddr'])){
		$externaddr = $res['general']['externaddr'];
	}else{
		$externaddr = '';
	}
	
	$temp = explode(":",$externaddr);
	
	echo $flag."-".$temp[0];
}

function change_language(){
	$conf_file = "/etc/asterisk/gw/web_language.conf";
	if(isset($_GET['language_type'])){
		$aql = new aql();
		$hlock = lock_file($conf_file);
		$aql->set('basedir','/etc/asterisk/gw/');
		if(!$aql->open_config_file($conf_file)){
			echo $aql->get_error();
			unlock_file($hlock);
			return -1;
		}
		
		$res = $aql->query("select * from web_language.conf");
		
		$language = $_GET['language_type'];
		if(isset($res['general']['language'])){
			$aql->assign_editkey('general','language',$language);
		}else{
			$aql->assign_append('general','language',$language);
		}
		
		if(!$aql->save_config_file('web_language.conf')){
			echo $aql->get_error();
			unlock_file($hlock);
			return false;
		}
		unlock_file($hlock);
	}
	exec("/my_tools/web_language_init");
}

function cancel_language(){
	$conf_file = "/etc/asterisk/gw/web_language.conf";
	$aql = new aql();
	$hlock = lock_file($conf_file);
	$aql->set('basedir','/etc/asterisk/gw/');
	if(!$aql->open_config_file($conf_file)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	
	$res = $aql->query("select * from web_language.conf");
	
	if(isset($res['general']['flag'])){
		$aql->assign_editkey('general','flag',1);
	}else{
		$aql->assign_append('general','flag',1);
	}
	
	if(!$aql->save_config_file('web_language.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
}

function sim_unlock(){
	$channel = $_GET['channel'];
	exec("/my_tools/set_calllimit.sh unlock $channel");
}

function sim_unlimit(){
	$channel = $_GET['channel'];
	exec("/my_tools/set_calllimit.sh unlimited $channel");
}

function get_uuid(){
	$mod_brd = $_GET['mod_brd'];
	exec("/my_tools/bsp_cli module uid $mod_brd", $output);
	
	$uuid = explode('UID:',$output[0]);
	echo $uuid[1];
}

function system_update(){
	$store_file = $_GET['store_file'];
	$firmware_name = $_GET['firmware_name'];
	
	exec("/my_tools/unpack.sh $store_file  > /data/log/update.txt || echo $?",$output);
	 
	if (isset($output)) {
		if($output) {
			echo '<span style="color:red">'.language('Error').': '.language("System Update Failed");echo "<br>\n";
			echo language("Error code");echo ": ".$output[0].'</span>';
			echo '<div style="text-align:center;margin-top:30px;"><button id="button_cancel" type="button" style="margin-left:10px;">'.language('Close').'</button></div>';
		} else {
			exec("/my_tools/add_syslog \"System Update\"");
			echo language("System Update Succeeded")."<br>";
			echo language('System Update Succeeded help',"You must reboot system to entry the newer system.")."<br/>";
			echo '<div style="text-align:center;margin-top:30px;">'.language('System Count Dowm','The system will be restarted automatically after 10 seconds countdown').': &nbsp<span id="update_time">10</span>'.language('Second').'<button id="button_cancel" type="button" style="margin-left:10px;">'.language('Cancel').'</button></div>';
			
			delete_db_fields_for_lower_version($firmware_name, 1);
		}
	}
	 
	 exec("rm -rf /data/update_file_*");
}

function system_reboot(){
	exec("reboot -f > /dev/null 2>&1");
}

function dowmload_system_file(){
	$store_file = $_GET['store_file'];
	
	$product_type = get_product_type();
	if($product_type >= 4){
		$url="https://downloads.openvox.cn/pub/firmwares/Wireless_Gateway/SWG-gen2/swg-current.bin";
	}else{
		$url = "https://downloads.openvox.cn/pub/firmwares/Wireless_Gateway/M20X/swg-m20x-current.bin";
	}
	
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

function set_sim_unlimited(){
	$arr = $_POST['arr'];
	for($i=0;$i<count($arr);$i++){
		$channel = $arr[$i]['channel'];
		$sim_id = $arr[$i]['sim_id'];
		
		exec("/my_tools/set_calllimit.sh sim_unlimited $channel $sim_id");
		exec("/my_tools/set_calllimit.sh sim_resetcalltime $channel $sim_id");
	}
}

function set_sim_unlock(){
	$arr = $_POST['arr'];
	for($i=0;$i<count($arr);$i++){
		$channel = $arr[$i]['channel'];
		$sim_id = $arr[$i]['sim_id'];
		
		exec("/my_tools/set_calllimit.sh sim_unlock $channel $sim_id");
	}
}

function set_sim_unmark(){
	$arr = $_POST['arr'];
	for($i=0;$i<count($arr);$i++){
		$channel = $arr[$i]['channel'];
		$sim_id = $arr[$i]['sim_id'];
		
		exec("/my_tools/set_calllimit.sh sim_unmark $channel $sim_id");
	}
}

function set_sms_limit(){
	$arr = $_POST['arr'];
	for($i=0;$i<count($arr);$i++){
		$channel = $arr[$i]['channel'];
		$sim_id = $arr[$i]['sim_id'];
		
		exec("/my_tools/set_calllimit.sh sim_unsmslimit $channel $sim_id");
	}
}

function sim_unlimit_all(){
	exec("/my_tools/set_calllimit.sh allsim_unlimited");
}

function sim_unlock_all(){
	exec("/my_tools/set_calllimit.sh allsim_unlock");
}

function sim_unmark_all(){
	exec("/my_tools/set_calllimit.sh allsim_unmark");
}

function sms_umlimit_all(){
	exec("/my_tools/set_calllimit.sh allsim_unsmslimited");
}

function save_pin_one(){
	$conf_path = "/etc/asterisk/gw/call_limit/sim_info.conf";
	if(!file_exists($conf_path)){
		touch($conf_path);
	}
	
	$aql = new aql();
	$hlock = lock_file($conf_path);
	
	$aql->set('basedir','/etc/asterisk/gw/call_limit/');
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	
	$res = $aql->query("select * from sim_info.conf");
	
	$channel = $_POST['channel'];
	$sim_ind = $_POST['sim_ind'];
	$pin_code = $_POST['pin_code'];
	$need_pin = $_POST['need_pin'];
	
	$port_sim = $channel.'-'.$sim_ind;
	
	if(!isset($res[$port_sim])){
		$aql->assign_addsection($port_sim,'');
	}
	
	if(isset($res[$port_sim]['pincode'])){
		$aql->assign_editkey($port_sim, 'pincode', $pin_code);
	}else{
		$aql->assign_append($port_sim, 'pincode', $pin_code);
	}
	
	if(!$aql->save_config_file('sim_info.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	
	wait_apply('exec','/my_tools/set_calllimit.sh reload');
}

function save_pin_all(){
	$conf_path = "/etc/asterisk/gw/call_limit/sim_info.conf";
	if(!file_exists($conf_path)){
		touch($conf_path);
	}
	
	$aql = new aql();
	$hlock = lock_file($conf_path);
	$aql->set('basedir','/etc/asterisk/gw/call_limit/');
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	
	$res = $aql->query("select * from sim_info.conf");
	
	$arr = $_POST['arr'];
	for($i=0;$i<count($arr);$i++){
		$channel = $arr[$i]['channel'];
		$sim_ind = $arr[$i]['sim_ind'];
		$pin_code = $arr[$i]['pin_code'];

		$port_sim = $channel.'-'.$sim_ind;
		
		if(!isset($res[$port_sim])){
			$aql->assign_addsection($port_sim,'');
		}
		
		if(isset($res[$port_sim]['pincode'])){
			$aql->assign_editkey($port_sim, 'pincode', $pin_code);
		}else{
			$aql->assign_append($port_sim, 'pincode', $pin_code);
		}
	}
	
	if(!$aql->save_config_file('sim_info.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	
	wait_apply('exec','/my_tools/set_calllimit.sh reload');
}

function switch_sim_card(){
	$channel = $_GET['channel'];
	$sim_id = $_GET['sim_id'];
	
	//clear all limit
	exec("/my_tools/set_calllimit.sh sim_unlimited $channel $sim_id");
	exec("/my_tools/set_calllimit.sh sim_resetcalltime $channel $sim_id");
	exec("/my_tools/set_calllimit.sh sim_unlock $channel $sim_id");
	exec("/my_tools/set_calllimit.sh sim_unmark $channel $sim_id");
	exec("/my_tools/set_calllimit.sh sim_unsmslimit $channel $sim_id");
	
	//switch
	exec("/my_tools/set_calllimit.sh sim_switch $channel $sim_id");
}


function slot_test(){
	global $__GSM_SUM__;
	
	exec("/etc/init.d/asterisk stop");
	exec("/etc/init.d/call_limit.sh stop");
	
	exec("rm -rf /tmp/slot_test.log");
	exec("/my_tools/simslot_tools $__GSM_SUM__ &");
}

function slot_test_retry(){
	exec("/etc/init.d/asterisk stop");
	exec("/etc/init.d/call_limit.sh stop");
	
	$channel = $_GET['channel'];
	$slot = $_GET['slot'];
	
	send_at_command($channel,'AT+CFUN=0');
	
	exec("/my_tools/bsp_cli sim sel $channel $slot");
	send_at_command($channel,'AT+CFUN=0');
	
	send_at_command($channel,'AT+CFUN=1');
	sleep(2);
	
	$str = send_at_command($channel,'AT+CPIN?');
	
	exec("/etc/init.d/asterisk start");
	exec("/etc/init.d/call_limit.sh start");
	
	echo $str;
}

function send_at_command($channel,$at){
	exec("/my_tools/rri_cli at $channel $at",$output);
	$str = '';
	$flag = 0;
	for($i=0;$i<count($output);$i++){
		if($output[$i] != ''){
			$str .= $output[$i].' ';
			
			if(strstr($output[$i],'CME ERROR') || strstr($output[$i],'OK')){
				$flag = 1;
			}
		}
	}
	
	if($flag){
		return $str;
	}else{
		send_at_command($channel,$at);
	}
}

function get_slot_test_log(){
	if(file_exists('/tmp/slot_test.log')){
		$str = file_get_contents("/tmp/slot_test.log");
		
		exec("/etc/init.d/asterisk start");
		exec("/etc/init.d/call_limit.sh start");
	}else{
		$str = '';
	}
	
	echo $str;
}

function general_cdr_json(){
	$cdr_log_file = '/data/log/cdr.db';
	$db = new CDRDB();
	
	$filter_sql = $_POST['filter_sql'];
	$sort_sql = $_POST['sort_sql'];
	
	$hlock = lock_file($cdr_log_file);
	$results = $db->try_query("select * from cdr $filter_sql $sort_sql");
	unlock_file($hlock);
	
	$json = [];
	
	while($res = @$results->fetchArray()) {
		if(get_channel_type($res['from']) == 'gsm') {
			$res['from'] = change_sms_routing_form_new($res['from']);
		} 
		if(get_channel_type($res['to']) == 'gsm') {
			$res['to'] = change_sms_routing_form_new($res['to']);
		}
		
		$res_obj = [
			'callerid' => $res['callerid'],
			'calleeid' => $res['calleeid'],
			'from' => $res['from'],
			'to' => $res['to'],
			'starttime' => $res['starttime'],
			'duration' => $res['duration'],
			'result' => $res['result']
		];
		
		array_push($json,$res_obj);
	}
	
	$json_str = json_encode($json);
	echo $json_str;
}

function test_email(){
	require_once('/my_tools/PHPMailer_5.2.2/class.phpmailer.php');
	
	$tls_ssl = $_POST['tls_ssl'];
	$smtp_server = $_POST['smtp_server'];
	$sender = $_POST['sender'];
	$smtp_port = $_POST['smtp_port'];
	$smtp_user = $_POST['smtp_user'];
	$smtp_pwd = $_POST['smtp_pwd'];
	$smail1 = $_POST['smail1'];
	$smail2 = $_POST['smail2'];
	$smail3 = $_POST['smail3'];
	$title = $_POST['title'];
	$content = $_POST['content'];
	
	$mail = new PHPMailer(true);
	try{
		$mail->IsSMTP();
		$mail->SMTPAuth   = true;
		if($tls_ssl == 'tls') {
			$mail->SMTPSecure = 'tls';
		}else if($tls_ssl == 'ssl'){
			$mail->SMTPSecure = 'ssl';
		}
		
		$mail->CharSet	  = 'utf-8';
		$mail->Host       = $smtp_server;
		$mail->SetFrom($sender);
		$mail->Port       = $smtp_port;
		$mail->Username   = $smtp_user;
		$mail->Password   = $smtp_pwd;
		if($smail1 != "")
			$mail->AddAddress($smail1);

		if($smail2 != "")
			$mail->AddAddress($smail2);

		if($smail3 != "")
			$mail->AddAddress($smail3);
		
		$mail->Subject = $title;
		$mail->Body = $content; 
		if($mail->Send()){
			echo 'success';
		}else{
			echo 'failed';
		}
	} catch (phpmailerException $e){
		echo $e->errorMessage();
	} catch (Exception $e){
		echo $e->getMessage();
	}
}

function l2tpvpn_status(){
	exec("ifconfig ppp30 | grep ^ppp30",$output);
	
	if($output[0] != ''){
		echo 'OK';
	}else{
		echo '';
	}
}

function getsn(){
	$predis_db = new Predis\Client($redis_server);
	
	echo $predis_db->get('local.system.serialnumber');
}

if(isset($_GET['type']) && $_GET['type']) {
	switch($_GET['type']) {
	//system
	case 'system' :
		$product_type = get_product_type();
		
		if(isset($_GET['system_type']) && $_GET['system_type']=="system_time"){
			echo get_system_time();
		}else if(isset($_GET['system_type']) && $_GET['system_type']=="newest_sys_version"){
			if($product_type >= 4){
				echo trim(@file_get_contents('https://downloads.openvox.cn/pub/firmwares/Wireless_Gateway/SWG-gen2/current-version'));
			}else{
				echo trim(@file_get_contents('https://downloads.openvox.cn/pub/firmwares/Wireless_Gateway/M20X/current-version'));
			}
		}else if(isset($_GET['system_type']) && $_GET['system_type']=="newest_sys_changelog"){
			//change show forms of "changelog"
			//echo trim(@file_get_contents('https://downloads.openvox.cn/pub/firmwares/GSM%20Gateway/current-changelog'));
			if($product_type >= 4){
				$str = trim(@file_get_contents('https://downloads.openvox.cn/pub/firmwares/Wireless_Gateway/SWG-gen2/current-changelog'));
			}else{
				$str = trim(@file_get_contents('https://downloads.openvox.cn/pub/firmwares/Wireless_Gateway/M20X/current-changelog'));
			}
			$res = str_replace("\n","<br>",$str);
			$res = str_replace('*','&emsp;*',$res);
			$res = preg_replace('/\((\d+)\)/','&emsp;&emsp;>',$res);
			echo $res;
		}else if(isset($_GET['system_type']) && $_GET['system_type']=="sys_changelog"){
			//change show forms of "changelog"
			//echo trim(@file_get_contents('https://downloads.openvox.cn/pub/firmwares/GSM%20Gateway/ChangeLog'));
			if($product_type >= 4){
				$str = trim(@file_get_contents('https://downloads.openvox.cn/pub/firmwares/Wireless_Gateway/SWG-gen2/ChangeLog'));
			}else{
				$str = trim(@file_get_contents('https://downloads.openvox.cn/pub/firmwares/Wireless_Gateway/M20X/ChangeLog'));
			}
			$res = str_replace("\n","<br>",$str);
			$res = str_replace('*','&emsp;*',$res);
			$res = preg_replace('/\((\d+)\)/','&emsp;&emsp;>',$res);
			echo $res;
		//}else if(isset($_GET['dial_pattern']) && $_GET['dial_pattern']=="dial_pattern"){
		//	$str_array = dial_pattern_info();
		//	echo json_encode($str_array);
		}else if(isset($_GET['module_type']) && $_GET['module_type']=="module_type"){
			$str_array = show_port_for_ajax();
			echo json_encode($str_array);
		}else if(isset($_GET['firewall']) && $_GET['firewall']=="firewall_preview"){
			exec("cd /my_tools/lua/info_access && lua firewall_config.lua > /dev/null &");
			exec("lua /my_tools/lua/info_access/firewall_recover.lua > /dev/null &");
			echo "ok";
		}else if(isset($_GET['firewall']) && $_GET['firewall']=="firewall_recover"){
			//copy firewall from RAM to FLASH
			$firewall_conf = "/etc/asterisk/gw/firewall.conf";
			$firewall_conf_r = "/etc/asterisk/gw/firewall*";
			$firewall_path_f = "/etc/cfg/gw/";
			exec("cp $firewall_conf_r $firewall_path_f");
			exec("ps -ef | grep firewall_recover | grep -v grep | awk '{print $1}' | xargs kill -9 ");
			$switch=trim(`/my_tools/set_config "$firewall_conf" get option_value general firewall`);
			if ($switch == 'off') {
				echo $switch;
			} else {
				$res = `xtables-multi iptables -L -n `;
				$res = str_replace("\n","<br>",$res);
				echo $res;
			}
		}else if(isset($_GET['action']) && $_GET['action'] == "capture"){
			start_capture();
		}else if(isset($_GET['action']) && $_GET['action'] == "dump_capture"){
			stop_capture();
		}else if(isset($_GET['action']) && $_GET['action'] == "sipcapture"){
			start_sipdump();
		}else if(isset($_GET['action']) && $_GET['action'] == "dump_sipcapture"){
			stop_sipdump();
		}else if(isset($_GET['action']) && $_GET['action'] == 'set_wizard'){
			if(isset($_GET['value']) && trim($_GET['value']) == 'off'){
				$gw_conf_content = file_get_contents('/etc/asterisk/gw.conf');
				if(!strstr($gw_conf_content, 'setup-wizard')){
				    exec("/my_tools/set_config /etc/asterisk/gw.conf add context setup-wizard");
				}
				if(!strstr($gw_conf_content, 'set_wizard')){
				    exec("sed -i '/setup-wizard/a\set_wizard=' /etc/asterisk/gw.conf");
				}
				exec("/my_tools/set_config /etc/asterisk/gw.conf set option_value setup-wizard set_wizard off && echo -n 'ok'", $output);
				exec("cp /etc/asterisk/gw.conf /etc/cfg/");
				if($output[0] == 'ok'){
					echo 'success';
				} else {
					echo 'failed';
				}
			}else if(isset($_GET['value']) && trim($_GET['value']) == 'on'){
				saveChangePassword();
				saveSelectTimezone();
				saveSIPEndpoint();
				saveDestination();
				closeWizardSetting();
				saveNetworkSettings();
			}
		} else if(isset($_GET['action']) && $_GET['action'] == 'get_httptosms_api_info'){
			$message = file_get_contents('/etc/asterisk/gw/usage_sendsms.info');
			
			echo '<pre>';
			echo $message;
			echo '</pre>';
		} else if(isset($_GET['action']) && $_GET['action'] == 'get_smstohttp_api_info'){
			$message = file_get_contents('/etc/asterisk/gw/usage_smsTohttp.info');
			echo '<pre>';
			echo $message;
			echo '</pre>';
		}else if(isset($_GET['action']) && $_GET['action'] == 'get_http_api_info'){
			$file_name = '/etc/asterisk/gw/'.$_GET['filename'].'.info';
			$message = file_get_contents($file_name);
			
			echo '<pre>';
			echo $message;
			echo '</pre>';
		}else{
			echo "";
		}
		break;
	//gsm
	case 'gsm' :
		if(isset($_GET['gsm_type']) && $_GET['gsm_type']=="imei"){
			$imei_array = get_imei_all();
			echo json_encode($imei_array);
		}else if(isset($_GET['gsm_type']) && $_GET['gsm_type']=="modify_imei"){
			$ret = manually_modify_imei();
			echo $ret;
		}else{
			echo "";
		}
		break;
	//sip
	case 'sip' :
		if(isset($_GET['sip_type']) && $_GET['sip_type']==""){
			echo "";
		}else{
			echo "";
		}
		break;
	//iax
	case 'iax' :
		if(isset($_GET['sip_type']) && $_GET['sip_type']==""){
			echo "";
		}else{
			echo "";
		}
		break;
	//routing
	case 'routing' :
		if(isset($_GET['routing_type']) && $_GET['routing_type']==""){
			echo "";
		}else{
			echo "";
		}
		break;
	//network
	case 'network' :
		if(isset($_GET['network_type']) && $_GET['network_type']==""){
			echo "";
		}else{
			echo "";
		}
		break;
	//advanced
	case 'advanced' :
		if(isset($_GET['advanced_type']) && $_GET['advanced_type']==""){
			echo "";
		}else{
			echo "";
		}
		break;
	//log
	case 'log' :
		echo process_log();
		break;
	case 'SimEmuSvr_log' :
		simemusvr();
		break;
	case 'state_cloud' :
	    get_state_cloud();
		break;
	case 'pptpvpn_status':
		get_pptpvpn_status();
		break;
	case 'openvpn_status':
		get_openvpn_status();
		break;
	case 'sync_time_from_client':
       	sync_time_from_client();
		break;
	case 'sync_time_from_ntp':
		sync_time_from_ntp();
		break;
	case 'sync_time_from_station':
		sync_time_from_station();
		break;
	case 'module_update':
		if(isset($_GET['update'])){
			module_update();
		}else{
			module_update_exit();
		}
		break;
	case 'mcu_update':
		if(isset($_GET['update'])){
			mcu_update();
		}else{
			mcu_update_exit();
		}
		break;
	case 'internet':
		get_internet_used();
		break;
	case 'match_test':
		match_test();
		break;
	case 'get_public_ip':
		get_public_ip();
		break;
	case 'change_language':
		change_language();
		break;
	case 'cancel_language':
		cancel_language();
		break;
	case 'sim_unlock':
		sim_unlock();
		break;
	case 'sim_unlimit':
		sim_unlimit();
		break;
	case 'n2n_status':
		get_n2n_status();
		break;
	case 'get_uuid':
		get_uuid();
		break;
	case 'system_update':
		system_update();
		break;
	case 'system_reboot':
		system_reboot();
		break;
	case 'dowmload_system_file':
		dowmload_system_file();
		break;
	case 'get_system_update_progress':
		get_system_update_progress();
		break;
	case 'connection_status':
		if(file_exists('/tmp/OPlink.status')){
			$status = file_get_contents('/tmp/OPlink.status');
		}else{
			$status = '';
		}
		echo $status;
		
		break;
	case 'set_sim_unlimited':
		set_sim_unlimited();
		break;
	case 'set_sim_unlock':
		set_sim_unlock();
		break;
	case 'set_sim_unmark':
		set_sim_unmark();
		break;
	case 'set_sms_limit':
		set_sms_limit();
		break;
	case 'sim_unlimit_all':
		sim_unlimit_all();
		break;
	case 'sim_unlock_all':
		sim_unlock_all();
		break;
	case 'sim_unmark_all':
		sim_unmark_all();
		break;
	case 'sms_umlimit_all':
		sms_umlimit_all();
		break;
	case 'save_pin_one':
		save_pin_one();
		break;
	case 'save_pin_all':
		save_pin_all();
		break;
	case 'switch_sim_card':
		switch_sim_card();
		break;
	case 'slot_test':
		slot_test();
		break;
	case 'slot_test_retry':
		slot_test_retry();
		break;
	case 'get_slot_test_log':
		get_slot_test_log();
		break;
	case 'general_cdr_json':
		general_cdr_json();
		break;
	case 'test_email':
		test_email();
		break;
	case 'l2tpvpn_status':
		l2tpvpn_status();
		break;
	case 'getsn':
		getsn();
		break;
	case 'mcu_exit_upgrade':
		mcu_exit_upgrade();
		break;
	default :
		echo "";
		break;
	}
	
} else {
	$ret = show_gsms();
	//show_sips();
	//show_routings();
	//show_networks();
	//show_pppoe();

	$status_array['gsm_array'] = $ret;
	//$status_array['sip_array']=$sip_array;
	//$status_array['routing_array']=$routing_array;
	//$status_array['network_array']=$network_array;
	//$status_array['pppoe_array']=$pppoe_array;
	echo json_encode($status_array);
}
exit(0);
?>
