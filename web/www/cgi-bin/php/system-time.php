<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/check.inc");
require_once("/www/cgi-bin/inc/aql.php");
?>

<!--// load jQuery and the jQuery iButton Plug-in //--> 
<!--<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.3.2/jquery.min.js"></script> -->
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 

<!---// load the iButton CSS stylesheet //--> 
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/float_btn.js"></script>
<script type="text/javascript">
<!--
function modechange()
{
	var tz_info = value('system_timezone');
	if ((tz_info=='') || (tz_info==null)){
		set_value('show_TZ', tz_info);
	}
	else {
		var tz_split = tz_info.split('@');
		set_value('show_TZ', tz_split[1]);
	}
}
var interval;
var running_time=0;
function click(){

	var ntp_server1 = document.getElementById("ntp_server1").value;
	var ntp_server2 = document.getElementById("ntp_server2").value;
	var ntp_server3 = document.getElementById("ntp_server3").value;
	var data;
	$.ajax({
		type:'GET',
		url: "/cgi-bin/php/ajax_server.php?random="+Math.random()+"&type=sync_time_from_ntp",
		data:{'ntp_server1':ntp_server1,'ntp_server2':ntp_server2,'ntp_server3':ntp_server3},
		success: function(data) {
			data = $.trim(data);
			var str = "<p style='text-align:center'>";
			str += "<br><font color='green' size='5px'>"+data+"</font></p>";
			$("#time").html(str);
			window.setInterval(function(){
				$("#preview_dg").dialog("close");
			}, 5000);
			window.setTimeout('window.location.href="/cgi-bin/php/system-time.php";',1000);
			window.clearInterval(interval);
		},
	});

	var str = "<p style='text-align:center'>";
		str += "<br><font color='green' size='5px'><?php echo language('Sychronous time from ntp help', 'Sychronous time from NTP server need some times, please waiting for minutes');?>"+
				"<br/><img src='/images/loading.gif' />"+
				"</font></p>";
	$("#time").html(str);
	interval = setInterval(function() {
		running_time = running_time + 1;
		if(running_time == 60){
			str = "<p style='text-align:center'>";
			str += "<br><font color='green' size='5px'><?php echo language('request timeout help','Request timeout, failed to sychronous time from NTP server.');?></font></p>";
			$("#time").html(str);
			window.setInterval(function(){
				$("#preview_dg").dialog("close");
			}, 6000);
			window.clearInterval(interval);
		}
	}, 3000);

}
function sync_time_from_ntp() 
{
	click();
	$("#preview_dg").dialog({
		resizable: false,
		height:200,
		width:500,
		modal: true,		
	});
	$(".ui-button").click(function(){
		window.clearInterval(interval);
	});
}

function check()
{
	var ntp_server1 = document.getElementById("ntp_server1").value;
	var ntp_server2 = document.getElementById("ntp_server2").value;
	var ntp_server3 = document.getElementById("ntp_server3").value;

	document.getElementById("cntp_server1").innerHTML = '';
	if(ntp_server1 != '') {
		if(!check_domain(ntp_server1)) {
			document.getElementById("cntp_server1").innerHTML = con_str('<?php echo language('js check domain','Invalid domain or IP address.');?>');
			return false;
		} 
	}

	document.getElementById("cntp_server2").innerHTML = '';
	if(ntp_server2 != '') {
		if(!check_domain(ntp_server2)) {
			document.getElementById("cntp_server2").innerHTML = con_str('<?php echo language('js check domain','Invalid domain or IP address.');?>');
			return false;
		} 
	}

	document.getElementById("cntp_server3").innerHTML = '';
	if(ntp_server3 != '') {
		if(!check_domain(ntp_server3)) {
			document.getElementById("cntp_server3").innerHTML = con_str('<?php echo language('js check domain','Invalid domain or IP address.');?>');
			return false;
		} 
	}

	return true;
}

function postLocalTime()
{
	if(!check()) {
		return false;
	}

	var myDate = new Date();
	set_value('local_yea',myDate.getFullYear());
	set_value('local_mon',myDate.getMonth()+1);
	set_value('local_dat',myDate.getDate());
	set_value('local_hou',myDate.getHours());
	set_value('local_min',myDate.getMinutes());
	set_value('local_sec',myDate.getSeconds());

	return true;
}

-->
</script>

<?php
for($i=1; $i<=3; $i++) {
	$ntpserver[$i] = '';
}

function save_data()
{
	if( isset($_POST['ntp_server1']) &&
		isset($_POST['ntp_server2']) &&
		isset($_POST['ntp_server3']) &&
		isset($_POST['system_timezone']) &&
		isset($_POST['show_TZ'])
	) {
		global $ntpserver;

		for($i=1;$i<=3;$i++){
			$ntpserver[$i] = trim($_POST["ntp_server$i"]);
		}

		if(isset($_POST['auto_sw'])) {
			$auto = 'on';
		} else {
			$auto = 'off';
		}
		
		$auto_sync_type = trim($_POST['auto_sync_type']);

		$timezone = trim($_POST['system_timezone']);
		$show_TZ = trim($_POST['show_TZ']);
		$file_path="/etc/asterisk/gw/time.conf";
		$hlock=lock_file($file_path);
		$fh=fopen($file_path,"w");
		$write = "[general]\n";
		$write .= "auto_sync=$auto\n";
		$write .= "auto_sync_type=$auto_sync_type\n";
		$write .= "timezone=$timezone\n";
		for($i=1;$i<=3;$i++){
			$write .= "ntp$i=$ntpserver[$i]\n";
		}
		fwrite($fh,$write);
		fclose($fh);
		unlock_file($hlock);

		$file_path="/etc/cfg/gw/time.conf";
		$hlock=lock_file($file_path);
		$fh=fopen($file_path,"w");
		$write = "[general]\n";
		$write .= "auto_sync=$auto\n";
		$write .= "timezone=$timezone\n";
		for($i=1;$i<=3;$i++){
			$write .= "ntp$i=$ntpserver[$i]\n";
		}
		fwrite($fh,$write);
		fclose($fh);
		unlock_file($hlock);

		$tz_file_path="/etc/TZ";
		$hlock = lock_file($tz_file_path);
		$fh=fopen($tz_file_path,"w");
		fwrite($fh,"$show_TZ\n");
		fclose($fh);
		unlock_file($hlock);
		
		$ps = explode("@", $timezone);
		if ($ps[0]) {
			if ($ps[0] == "-") {
				$zonefile = "UTC";
			} else {
				$zoneinfo = explode("/", $ps[0]);
				$zonefile = $zoneinfo[1];
			}
		} else {
			$zonefile = "UTC";
		}

		$zifile = "/usr/share/zoneinfo/".$zonefile;

		if(file_exists($zifile)) {
			@file_put_contents("/etc/localtime", @file_get_contents($zifile, LOCK_EX), LOCK_EX);
		}
		
		wait_apply('exec','');
		return true;
	}

	return false;
}

if($_POST && isset($_POST['send'])) {
	if($_POST['send'] == 'Save Data') {
		save_data();
	}
}

//Read data 
$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$res = $aql->query('select * from time.conf');

if(isset($res['general']['timezone'])) {
	$cf_timezone = trim($res['general']['timezone']);
} else {
	$cf_timezone = '-@UTC+0';
}

$auto_sw = '';
if(isset($res['general']['auto_sync'])) {
	if(is_true(trim($res['general']['auto_sync']))) {
		$auto_sw = 'checked';
	}
}

for($i=1;$i<=3;$i++){
	if(isset($res['general']["ntp$i"])) {
		$cf_ntp[$i] = trim($res['general']["ntp$i"]);
	} else {
		$cf_ntp[$i] = '';
	}
}

if(isset($res['general']['auto_sync_type'])){
	$auto_sync_type = $res['general']['auto_sync_type'];
}else{
	$auto_sync_type = 'ntp';
}

?>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Time Settings');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tedit" >
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('System Time');?>:
					<span class="showhelp">
					<?php
						$help = 'Your gateway system time.';
						echo language('System Time help@system-time', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				<span id="currenttime"></span>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Time Zone');?>:
					<span class="showhelp">
					<?php
						$help = 'The world time zone. Please select the one which is the same as or the closest to your city.';
						echo language('Time Zone help@system-time', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				<select id="system_timezone" style="width: 253px;" name="system_timezone" onchange="modechange(this)">
					<optgroup label="<?php echo language('Australia');?>">
						<option  value="Australia/Melbourne@EST-10EST,M10.5.0,M3.5.0/3"><?php echo language('Melbourne,Canberra,Sydney');?></option>
						<option  value="Australia/Perth@WST-8"><?php echo language('Perth');?></option>
						<option  value="Australia/Brisbane@EST-10"><?php echo language('Brisbane');?></option>
						<option  value="Australia/Adelaide@CST-9:30CST,M10.5.0,M3.5.0/3"><?php echo language('Adelaide');?></option>
						<option  value="Australia/Darwin@CST-9:30"><?php echo language('Darwin');?></option>
						<option  value="Australia/Hobart@EST-10EST,M10.1.0,M3.5.0/3"><?php echo language('Hobart');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Europe');?>">
						<option  value="Europe/Amsterdam@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Amsterdam,Netherlands');?></option>
						<option  value="Europe/Athens@EET-2EEST,M3.5.0/3,M10.5.0/4"><?php echo language('Athens,Greece');?></option>
						<option  value="Europe/Berlin@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Berlin,Germany');?></option>
						<option  value="Europe/Brussels@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Brussels,Belgium');?></option>
						<option  value="Europe/Bratislava@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Bratislava,Slovakia');?></option>
						<option  value="Europe/Budapest@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Budapest,Hungary');?></option>
						<option  value="Europe/Copenhagen@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Copenhagen,Denmark');?></option>
						<option  value="Europe/Dublin@GMT0IST,M3.5.0/1,M10.5.0"><?php echo language('Dublin,Ireland');?></option>
						<option  value="Europe/Helsinki@EET-2EEST,M3.5.0/3,M10.5.0/4"><?php echo language('Helsinki,Finland');?></option>
						<option  value="Europe/Kiev@EET-2EEST,M3.5.0/3,M10.5.0/4"><?php echo language('Kyiv,Ukraine');?></option>
						<option  value="Europe/Lisbon@WET0WEST,M3.5.0/1,M10.5.0"><?php echo language('Lisbon,Portugal');?></option>
						<option  value="Europe/London@GMT0BST,M3.5.0/1,M10.5.0"><?php echo language('London,GreatBritain');?></option>
						<option  value="Europe/Madrid@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Madrid,Spain');?></option>
						<option  value="Europe/Oslo@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Oslo,Norway');?></option>
						<option  value="Europe/Paris@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Paris,France');?></option>
						<option  value="Europe/Prague@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Prague,CzechRepublic');?></option>
						<option  value="Europe/Rome@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Roma,Italy');?></option>
						<option  value="Europe/Moscow@MSK-3"><?php echo language('Moscow,Russia');?></option>
						<option  value="Europe/Stockholm@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Stockholm,Sweden');?></option>
						<option  value="Europe/Zurich@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Zurich,Switzerland');?></option>
					</optgroup>
					<optgroup label="<?php echo language('New Zealand');?>">
						<option  value="Pacific/Auckland@NZST-12NZDT,M10.1.0,M3.3.0/3"><?php echo language('Auckland, Wellington');?></option>
					</optgroup>
					<optgroup label="<?php echo language('USA and Canada');?>">
						<option  value="Pacific/Honolulu@HST10"><?php echo language('Hawaii Time');?></option>
						<option  value="America/Anchorage@AKST9AKDT,M3.2.0,M11.1.0"><?php echo language('Alaska Time');?></option>
						<option  value="America/Los_Angeles@PST8PDT,M3.2.0,M11.1.0"><?php echo language('Pacific Time');?></option>
						<option  value="America/Denver@MST7MDT,M3.2.0,M11.1.0"><?php echo language('Mountain Time');?></option>
						<option  value="America/Phoenix@MST7"><?php echo language('Mountain Time @Arizona, no DST');?></option>
						<option  value="America/Chicago@CST6CDT,M3.2.0,M11.1.0"><?php echo language('Central Time');?></option>
						<option  value="America/New_York@EST5EDT,M3.2.0,M11.1.0"><?php echo language('Eastern Time');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Atlantic');?>">
						<option  value="Atlantic/Bermuda@AST4ADT,M3.2.0,M11.1.0"><?php echo language('Bermuda');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+1)">
						<option  value="Asia/Anadyr@ANAT-12ANAST,M3.5.0,M10.5.0/3"><?php echo language('Anadyr');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+2)">
						<option  value="Asia/Amman@EET-2EEST,M3.5.4/0,M10.5.5/1"><?php echo language('Amman');?></option>
						<option  value="Asia/Beirut@EET-2EEST,M3.5.0/0,M10.5.0/0"><?php echo language('Beirut');?></option>
						<option  value="Asia/Damascus@EET-2EEST,J91/0,J274/0"><?php echo language('Damascus');?></option>
						<option  value="Asia/Gaza@EET-2EEST,J91/0,M10.3.5/0"><?php echo language('Gaza');?></option>
						<option  value="Asia/Jerusalem@GMT-2"><?php echo language('Jerusalem');?></option>
						<option  value="Asia/Nicosia@EET-2EEST,M3.5.0/3,M10.5.0/4"><?php echo language('Nicosia');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+3)">
						<option  value="Asia/Aden@AST-3"><?php echo language('Aden');?></option>
						<option  value="Asia/Baghdad@AST-3ADT,J91/3,J274/4"><?php echo language('Baghdad');?></option>
						<option  value="Asia/Bahrain@AST-3"><?php echo language('Bahrain');?></option>
						<option  value="Asia/Kuwait@AST-3"><?php echo language('Kuwait');?></option>
						<option  value="Asia/Qatar@AST-3"><?php echo language('Qatar');?></option>
						<option  value="Asia/Riyadh@AST-3"><?php echo language('Riyadh');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+3:30)">
						<option  value="Asia/Tehran@IRST-3:30"><?php echo language('Tehran');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+4)">
						<option  value="Asia/Baku@AZT-4AZST,M3.5.0/4,M10.5.0/5"><?php echo language('Baku');?></option>
						<option  value="Asia/Dubai@GST-4"><?php echo language('Dubai');?></option>
						<option  value="Asia/Muscat@GST-4"><?php echo language('Muscat');?></option>
						<option  value="Asia/Tbilisi@GET-4"><?php echo language('Tbilisi');?></option>
						<option  value="Asia/Yerevan@AMT-4AMST,M3.5.0,M10.5.0/3"><?php echo language('Yerevan');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+4:30)">
						<option  value="Asia/Kabul@AFT-4:30"><?php echo language('Kabul');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+5)">
						<option  value="Asia/Aqtobe@AQTT-5"><?php echo language('Aqtobe');?></option>
						<option  value="Asia/Ashgabat@TMT-5"><?php echo language('Ashgabat');?></option>
						<option  value="Asia/Dushanbe@TJT-5"><?php echo language('Dushanbe');?></option>
						<option  value="Asia/Karachi@PKT-5"><?php echo language('Karachi');?></option>
						<option  value="Asia/Oral@ORAT-5"><?php echo language('Oral');?></option>
						<option  value="Asia/Samarkand@UZT-5"><?php echo language('Samarkand');?></option>
						<option  value="Asia/Tashkent@UZT-5"><?php echo language('Tashkent');?></option>
						<option  value="Asia/Yekaterinburg@YEKT-5"><?php echo language('Yekaterinburg');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+5:30)">
						<option  value="Asia/Calcutta@IST-5:30"><?php echo language('Calcutta');?></option>
						<option  value="Asia/Colombo@IST-5:30"><?php echo language('Colombo');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+6)">
						<option  value="Asia/Almaty@ALMT-6"><?php echo language('Almaty');?></option>
						<option  value="Asia/Bishkek@KGT-6"><?php echo language('Bishkek');?></option>
						<option  value="Asia/Dhaka@BDT-6"><?php echo language('Dhaka');?></option>
						<option  value="Asia/Novosibirsk@NOVT-6"><?php echo language('Novosibirsk');?></option>
						<option  value="Asia/Omsk@OMST-6"><?php echo language('Omsk');?></option>
						<option  value="Asia/Qyzylorda@QYZT-6"><?php echo language('Qyzylorda');?></option>
						<option  value="Asia/Thimphu@BTT-6"><?php echo language('Thimphu');?></option>
					</optgroup>
						<optgroup label="<?php echo language('Asia');?> (UTC+7)">
						<option  value="Asia/Jakarta@WIT-7"><?php echo language('Jakarta');?></option>
						<option  value="Asia/Bangkok@ICT-7"><?php echo language('Bangkok');?></option>
						<option  value="Asia/Vientiane@ICT-7"><?php echo language('Vientiane');?></option>
						<option  value="Asia/Phnom_Penh@ICT-7"><?php echo language('Phnom Penh');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+8)">
						<option  value="Asia/Chongqing@CST-8"><?php echo language('Chongqing');?></option>
						<option  value="Asia/Hong_Kong@HKT-8"><?php echo language('Hong Kong');?></option>
						<option  value="Asia/Shanghai@CST-8"><?php echo language('Shanghai');?></option>
						<option  value="Asia/Singapore@SGT-8"><?php echo language('Singapore');?></option>
						<option  value="Asia/Urumqi@CST-8"><?php echo language('Urumqi');?></option>
						<option  value="Asia/Taipei@CST-8"><?php echo language('Taiwan');?></option>
						<option  value="Asia/Ulaanbaatar@ULAT-8ULAST,M3.5.6,M9.5.6/0"><?php echo language('Ulaanbaatar');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+9)">
						<option  value="Asia/Dili@TLT-9"><?php echo language('Dili');?></option>
						<option  value="Asia/Jayapura@EIT-9"><?php echo language('Jayapura');?></option>
						<option  value="Asia/Pyongyang@KST-9"><?php echo language('Pyongyang');?></option>
						<option  value="Asia/Seoul@KST-9"><?php echo language('Seoul');?></option>
						<option  value="Asia/Tokyo@JST-9"><?php echo language('Tokyo');?></option>
						<option  value="Asia/Yakutsk@YAKT-9YAKST,M3.5.0,M10.5.0/3"><?php echo language('Yakutsk');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Central and South America');?>">
						<option  value="America/Sao_Paulo@BRT3BRST,M11.1.0/0,M2.5.0/0"><?php echo language('Sao Paulo,Brazil');?></option>
						<option  value="America/Buenos_Aires@ART3"><?php echo language('Buenos_Aires,Argentina');?></option>
						<option  value="America/Guatemala@CST6"><?php echo language('Central America @no DST');?></option>
						<option  value="America/Caracas@VET4:30"><?php echo language('Caracas,Venezuela');?></option>
					</optgroup>
				</select>							
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('POSIX TZ String');?>:
					<span class="showhelp">
					<?php
						$help = 'Posix timezone strings.';
						echo language('POSIX TZ String help', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				<input id="show_TZ" type="text" style="width: 250px; height: 1.2em; color: #2f2f2f; background: #ececec; " name="show_TZ" readonly="readonly" value="This field requires the JavaScript support." />
			</td>
		</tr>

<?php
		$help_str[1]='Time server domain or hostname.For example, [time.asia.apple.com]';
		$help_str[2]='The first reserved NTP server.For example, [time.windows.com]';
		$help_str[3]='The second reserved NTP server.For example, [time.nist.gov]';
		for($i=1; $i<=3; $i++) {
			echo "<tr>\n";
			echo "<th>\n";
			echo "<div class=\"helptooltips\">\n";
			echo language("NTP Server $i");
			echo ":<span class=\"showhelp\">\n";
			echo language("NTP Server $i help",$help_str[$i]);
			echo "</span>\n";
			echo "</div>\n";
			echo "</th>\n";
			echo "<td>\n";
			echo "<input id=\"ntp_server${i}\" type=\"text\" name=\"ntp_server${i}\" style=\"width: 250px;\" value=\"$cf_ntp[$i]\" />"; 
			echo "<span id=\"cntp_server${i}\"></span>\n";
			echo "</td>\n";
			echo "</tr>\n";
		}
?>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Auto-Sync from Server');?>:
					<span class="showhelp">
					<?php
						$help = 'Whether enable automatically synchronize from server or not. On(enabled),OFF(disabled).';
						echo language('Auto-Sync from Server help', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				<input type="checkbox" id="auto_sw" name="auto_sw" <?php echo $auto_sw?>  />
			</td>
		</tr>
		
		<tr class="auto_sync_tr">
			<th>
				<div class="helptooltips">
					<?php echo language('Auto-Sync Type');?>:
					<span class="showhelp">
						<?php echo language('Auto-Sync Type help','Synchronization from NTP or from base station.');?>
					</span>
				</div>
			</th>
			<td>
				<select name="auto_sync_type" id="auto_sync_type">
					<option value="ntp" <?php if($auto_sync_type == 'ntp') echo 'selected';?>>NTP</option>
					<option value="station" <?php if($auto_sync_type == 'station') echo 'selected';?>><?php echo language('Station');?></option>
				</select>
			</td>
		</tr>
	</table>
	<br>
	<input type="hidden" name="local_yea" id="local_yea" value="" />
	<input type="hidden" name="local_mon" id="local_mon" value="" />
	<input type="hidden" name="local_dat" id="local_dat" value="" />
	<input type="hidden" name="local_hou" id="local_hou" value="" />
	<input type="hidden" name="local_min" id="local_min" value="" />
	<input type="hidden" name="local_sec" id="local_sec" value="" />

	<input type="hidden" name="send" id="send" value="" />
	<div id="float_btn" class="float_btn">
		<div id="float_btn_tr" class="float_btn_tr">
				<input type="submit" class="gen_long_btn" value="<?php echo language('Save Data');?>" onclick="document.getElementById('send').value='Save Data';return check();" />
				<button type="button" class="gen_long_btn" onclick="sync_time_from_ntp()"><?php echo language('Sync from NTP');?></button>
				<button type="button" class="gen_long_btn" onclick="sync_time_from_client()"><?php echo language('Sync from Client');?></button>
				<button type="button" class="gen_long_btn" onclick="auto_sync_from_station()"><?php echo language('Sync from Station');?></button>
			
		</div>
	</div>
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td width="25px">
				<input type="button" id="float_button_3" class="float_long_button" value="<?php echo language('Save Data');?>" onclick="sync_time_from_client();" />
			</td>
			<td width="25px">
				<input type="submit" id="float_button_1" class="float_long_button" value="<?php echo language('Sync from NTP');?>" onclick="document.getElementById('send').value='Sync from NTP';return check();" />
			</td>
			<td style="padding-left: 10px;">
				<input type="button" id="float_button_2" class="float_long_button" value="<?php echo language('Sync from Client');?>" onclick="sync_time_from_ntp();"/>
			</td>
			<td>
				<input type="button" id="float_button_4" class="float_long_button" value="<?php echo language('Sync from Station');?>" onclick="auto_sync_from_station();" />
			</td>
		</tr>
	</table>
	<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
	<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>
	<div id="preview_dg" title="<?php echo language('Sync Time From NTP');?>" style="display:none;width:470px;height:100px">
	<div>
		<div id="timemsg" style="display:block;width:470px;height:100px;margin:0" contenteditable = "false">
				<div id="time" ></div>
				<div id="prompt_info"></div>
		</div>
	</div>
	</div>
	</form>
	<br>

<script type="text/javascript"> 
$(document).ready(function (){ 
	$(":checkbox").iButton(); 
	onload_func();
	
	//auto sync type
	if(document.getElementById('auto_sw').checked){
		$(".auto_sync_tr").show();
	}else{
		$(".auto_sync_tr").hide();
	}
	$("#auto_sw").change(function(){
		if($(this).attr('checked') == 'checked'){
			$(".auto_sync_tr").show();
		}else{
			$(".auto_sync_tr").hide();
		}
	});
}); 
</script>

<?php
	if(isset($final_show) && $final_show != "") {
		$Report = language('Report');
		$Time_Sync = language('Time Synchronize');
		trace_output_start("$Report", "$Time_Sync");
		trace_output_newline();
		echo $final_show;
		trace_output_end();
	}
?>

<?php
	//date_default_timezone_set("6"+readfile("/etc/TZ"));
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
?>

<script type="text/javascript">
<!--
var c=0;
var Y=<?php echo $year?>;
var M=<?php echo $month?>;
var D=<?php echo $date?>;
var sec=<?php echo $hour*3600+$minute*60+$second?>;
function ctime() {
	sec++;
	H=Math.floor(sec/3600)%24;
	I=Math.floor(sec/60)%60;
	S=sec%60;
	if(S<10) S='0'+S;
	if(I<10) I='0'+I;
	if(H<10) H='0'+H;
	if (H=='00' & I=='00' & S=='00') D=D+1; //日进位
	if (M==2) { //判断是否为二月份******
		if (Y%4==0 && !Y%100==0 || Y%400==0) { //是闰年(二月有28天)
			if (D==30){
				M+=1;D=1;
			} //月份进位
		} else { //非闰年(二月有29天)
			if (D==29) {
				M+=1;D=1;
			} //月份进位
		}
	} else { //不是二月份的月份******
		if (M==4 || M==6 || M==9 || M==11) { //小月(30天)
			if (D==31) {
				M+=1;D=1;
			} //月份进位
		} else { //大月(31天)
			if (D==32) {
				M+=1;D=1;
			} //月份进位
		}
	}

	if (M==13) {
		Y+=1;M=1;
	} //年份进位

	//setInterval(ctime,1000);
	setTimeout("ctime()", 1000);
//	set_value('cur_time', 'ada');
//	document.system_time.cur_time.value = Y+'-'+M+'-'+D+' '+H+':'+I+':'+S;
	document.getElementById("currenttime").innerHTML = Y+'-'+M+'-'+D+' '+H+':'+I+':'+S;
}

function onload_func()
{
	modechange();
	ctime();
}

function sync_time_from_client(){
	var oDate = new Date(); 
	var local_yea=oDate.getFullYear(); 
	var local_mon=oDate.getMonth()+1;
	var local_dat=oDate.getDate(); 
	var local_hou=oDate.getHours(); 
	var local_min=oDate.getMinutes(); 
	var local_sec=oDate.getSeconds();
	
	$.ajax({
		type:"GET",
		url:"ajax_server.php?type=sync_time_from_client",
		data:{'local_mon':local_mon,'local_dat':local_dat,'local_hou':local_hou,'local_min':local_min,'local_yea':local_yea,'local_sec':local_sec},
		success: function(data){
			var data = $.trim(data);
			if(data != ''){
				alert(data);
				window.setTimeout('window.location.href="/cgi-bin/php/system-time.php";',1000); 
			} else {
				alert("<?php echo language('Sync from Client Failed','Failed to synchronize time from Client!');?>");
			}
		},	
	});
	
}
document.getElementById("system_timezone").value = "<?php echo $cf_timezone; ?>";

function auto_sync_from_station(){
	$.ajax({
		type:"GET",
		url:"ajax_server.php?type=sync_time_from_station",
		data:{},
		success:function(data){
			var data = $.trim(data);
			if(data != ''){
				alert(data);
				window.location.reload();
			}else{
				alert("<?php echo language('Sync from Station Failed','Failed to synchronize time from base station!');?>");
			}
		}
	});
}

</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>
<div id="float_btn1" class="sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()">
</div>
