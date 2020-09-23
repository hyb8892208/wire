<?php
/* get language cache */
require_once("/www/cgi-bin/inc/language.inc");

//load redis database;
require_once("/www/cgi-bin/inc/redis.inc");

//deal with cluster code
$__deal_cluster__=false;

$__system_type__=get_oem_info_sepcs('company');

if ($__system_type__==''){
	$__system_type__='openvox';
}

$language = get_web_language_cache('/tmp/web/language.cache');
if(isset($_GET['send_debug'])){
	if($_GET['send_debug']=='on'){
		debug_language('on');
	}else if($_GET['send_debug']=='off'){
		debug_language('off');
	}
}

if(file_exists('/tmp/web/language.debug')){
	$language_debug = 'true';
}else{
	$language_debug = 'false';
}

$filename = basename($_SERVER['REQUEST_URI']);

$title_array = array(
	"system-status.php" => "System Status",
	"system-time.php" => "Time Settings",
	"system-login.php" => "Login Settings",
	"system-general.php" => "General",
	"system-tools.php" => "System Tools",
	"system-info.php" => "Information",

	"gsm-settings.php" => "Module Settings",
	"simbank-settings.php" => "Simbank Settings",
	//"gsm-adv-settings.php" => "Advanced",
	"gsm-callforwarding.php" => "Call Forwarding",
	"gsm-callwaiting.php" => "Call waiting",
	"gsm-dtmf.php" => "DTMF Settings",
	"gsm-bcch.php" => "BCCH",
	"gsm-toolkit.php" => "GSM Toolkit",
	"module-update.php" => "Module Update",
	"gsm-calllimit.php" => "Call And SMS Limit",
 
	"strategy-switch.php" => "Switch",
	"strategy-limit.php" => "Limit",
	"strategy-lock.php" => "Lock",
	"strategy-sms.php" => "SMS Limit",
	"strategy-callstats.php" => "Call Stats",
	"strategy-smsstats.php" => "SMS Stats",
	"strategy-pin.php" => "Pin Code",
 
	"voip-endpoints.php" => "VoIP Endpoints",
	"sip-bindpoints.php" => "Batch SIP Endpoints",
	"sip-adv-settings.php" => "Advanced SIP Settings",
	"iax-adv-settings.php" => "Advanced IAX2 Settings",
	"sip-security.php" => "Sip Account Security",

	"routing-rules.php" => "Call Routing Rules",
	"routing-groups.php" => "Groups",
	"sip-sim-binding.php" => "Batch Creating Rules",
	"routing-mnp.php" => "MNP Settings",
	"routing-advanced.php" => "Advanced",
	"routing-auto.php" => "Auto",

	"sms-settings.php" => "SMS Settings",
	"sms-sender.php" => "SMS Sender",
	"sms-inbox.php" => "SMS Inbox",
	"sms-outbox.php" => "SMS Outbox",
	"sms-routing.php" => "SMS Forwarding",
	"sms-ussd.php" => "USSD",
	"sms-mms.php" => "MMS Settings",

	"network-lan.php" => "LAN Settings",
	"network-vpn.php" => "VPN Settings",
	"network-wan.php" => "WAN Settings",
	//"network-dns.php" => "DNS Settings",
	"network-ddns.php" => "DDNS Settings",
	"network-toolkit.php" => "Network Toolkit",
	"network-switch.php" => "Security Settings",
	"network-rules.php" => "Security Rules",
	"network-sipcapture.php" => "SIP Capture",

	"adv-astapi.php" => "Asterisk API",
	"adv-astcli.php" => "Asterisk CLI",
	"adv-astfileeditor.php" => "Asterisk File Editor",
	"adv-internet.php" => "Internet",
	"adv-cloud.php" => "Cloud",
	"adv-balance.php" => "Balance",
	"adv-phonenumber.php" => "PhoneNumber",
	"adv-arp.php" => "ARP",

	"log-settings.php" => "Logs Settings",
	"log-sys.php" => "System Logs",
	"log-ast.php" => "Asterisk Logs",
	"log-sip.php" => "SIP Logs",
	"log-iax.php" => "IAX2 Logs",
	"log-bsp.php" => "BSP Logs",
	"log-rri.php" => "RRI Logs",
	"log-debugat.php" => "AT Commands Logs",
	"log-cdr.php" => "CDR",
	"log-statistics.php" => "Statistics",
	"log-simemusvr.php" => "SimEmuSvr",
);

function get_license_mode(){
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$res = $aql->query('select * from system_type.conf');
	return $res['license']['license_mode'];
}

if(isset($title_array[$filename]))
	$title = $title_array[$filename];
else
	$title = "";

$redis_client = new Predis\Client();
$lisence_endtime = $redis_client->get("app.license.endtime");
$lisence_status = $redis_client->get("app.license.status");
$license_mode = get_license_mode();
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
	<head>
	<meta charset="utf-8">
	<meta name="viewport" content="width=device-width, initial-scale=1">
	<title><?php echo language($title);?></title>
	<link rel="icon" href="/images/logo.ico" type="image/x-icon">
	<link rel="shortcut icon" href="/images/logo.ico" type="image/x-icon">
	<link rel="stylesheet"  href="/css/style.css?v=2.7" />
	</head>
	<body>
	<script src="/js/js.js"></script>
	<script src="/js/jquery.js"></script>
	<?php //echo "language:";print_r($language);?>

	<script>
	<?php 
	if($license_mode == 'on'){
		if($lisence_status == 'nearly' || $lisence_status == 'valid'){
			
			$now_time = strtotime(now)+8*60*60;
			$end_time = strtotime($lisence_endtime);
			$remain_time = 3*24*60*60;
			$days = 0;$hours = 0;$mins = 0;
			if(($end_time - $now_time) <= $remain_time){
				//days
				$time_temp = $end_time - $now_time;
				$days = intval($time_temp/86400);
				
				//hours
				$hour_item = $time_temp%86400;
				$hours = intval($hour_item/3600);
				
				//minutes
				$minute_item = $hour_item%3600;
				$mins = intval($minute_item/60);
				
				if($lisence_status == 'nearly'){
					echo "alert('".language("Software expired help1","Your software service is about to expire").
							",".language("Software left","Left").':'.
							$days.language("Day").
							$hours.language("Hour").
							$mins.language("Minute").
							",".language("Software expired help2","Use the new License please!")."');";
				}
			}
		}else if($lisence_status == 'expired'){
			echo 'alert("'.language("Software expired help3","Your software service has expired, please use the new License!").'");';
		}
	}
	?>
	</script>