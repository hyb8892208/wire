<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
?>

<?php 

$product_type = get_product_type();

$gsm_ifname = array();

if($product_type < 4){
	exec("ifconfig -a | grep -E 'eth0|br-lan' | awk '{print $1}'|grep -v \"\.\"", $gsm_ifname);
}else{
	exec("ifconfig -a | grep \"^eth\" | awk '{print $1}'", $gsm_ifname);
}
foreach($gsm_ifname as $gsm_if){
	exec("/my_tools/net_tool $gsm_if 2> /dev/null && echo ok",$output_ip);
	if($output_ip[3] != ''){
		$gsm_ip[] = $output_ip[3];
	}
	$output_ip = '';
}

//tcpdump paramters
require_once("/www/cgi-bin/inc/aql.php");
$aql = new aql();

$setok = $aql->set('basedir', '/etc/asterisk/gw');
if (!$setok) {
	echo __LINE__.' '.$aql->get_error();
	exit;
}
$hlock = lock_file('/etc/asterisk/gw/capture_params.conf');
$res = $aql->query("select * from capture_params.conf");
unlock_file($hlock);
if(isset($res['tcpdump']['interface'])){
	$interface_type = trim($res['tcpdump']['interface']);
} else {
	if($product_type < 4){
		$interface_type = "br-lan";
	}else{
		$interface_type = "eth0";
	}
}

if(isset($res['tcpdump']['source_host'])) {
	$source_host = trim($res['tcpdump']['source_host']);
} else {
	$source_host = '';
}

if(isset($res['tcpdump']['dest_host'])) {
	$destination_host = trim($res['tcpdump']['dest_host']);	
} else {
	$destination_host = '';
}

if(isset($res['tcpdump']['port'])) {
	$cap_port = trim($res['tcpdump']['port']);
} else {
	$cap_port = '';
}

$protocol_type['all'] ='';
$protocol_type['tcp'] = '';
$protocol_type['udp'] = '';
$protocol_type['icmp'] = '';
$protocol_type['rtp'] = '';
$protocol_type['rtcp'] = '';
$protocol_type['sip'] = '';
$protocol_type['arp'] = '';

if(isset($res['tcpdump']['protocol'])) {
	$protocol = trim($res['tcpdump']['protocol']);
	$protocol_type["$protocol"] = "selected";
} else {
	$protocol_type['all'] = "selected";
}
?>

<script type="text/javascript" src="/js/functions.js">
</script>

<script type="text/javascript">
var hour,minute,second;//时 分 秒
     hour=minute=second=0;//初始化
var minutes = '00';
var seconds = '00';
var millisecond=0;//毫秒
var interval;
function click(){
	minutes = '00', seconds = '00';
	var str = "<p style='text-align:center'><font color='#00ff33' size= '6px' style='font-weight:bold'>" + minutes +":"+ seconds + "</font></p>";
		str += "<br><font color='green' size='5px'><?php echo language('tcpdump capture help', 'The maximum duration of this recording is 3 minutes,and the system will stop and download the recording file automatically when time is up');?></font>";
	$("#time").html(str);
	interval = setInterval(function() {
		millisecond=millisecond+1000;
		if(millisecond>=1000)
		{
			millisecond=0;
			second=second+1;
		}
		if(second>=60){
			second=0;
			minute=minute+1;
		}
		if(minute>=60){ 
			minute=0;
		}
		if(minute<10)minutes='0'+minute;
		else minutes=minute;
		if(second<10)seconds='0'+second;
		else seconds=second;
		if(minute>=30){
			clearInterval(interval);
		}
		var str = "<p style='text-align:center'><font color='#00ff33' size= '6px' style='font-weight:bold'>" + minutes +":"+ seconds + "</font></p>";
		str += "<br><font color='green' size='5px'><?php echo language('tcpdump capture help', 'The maximum duration of this recording is 3 minutes,and the system will stop and download the recording file automatically when time is up');?></font>";
		$("#time").html(str);
		if(minute == 3){
			$("#preview_dg").dialog("close");
			send_dump_capture_request();
			window.clearInterval(interval);
		}
		console.log(interval);
	}, 1000);

}

function send_capture_request()
{
	var interface = document.getElementById('interface_type').value;
	var source_host = document.getElementById('source_host').value;
	var dest_host = document.getElementById('dest_host').value;
	var protocol = document.getElementById('protocol').value;
	var cap_port = document.getElementById('cap_port').value;

	var server_file = "./../../cgi-bin/php/ajax_server.php";
	var request_param = "&interface_type="+interface+"&source_host="+source_host+"&dest_host="+dest_host+"&protocol="+protocol+"&cap_port="+cap_port;
	$.ajax({
			url: server_file+"?random="+Math.random()+"&type=system&action=capture"+request_param,	
			async: false,
			dataType: 'text',
			type: 'GET',
			timeout: 5000,
			error: function(data){				//request failed callback function;
				str = "";
				str += "<br><font color='red' size= '3px' style='font-weight:bold'>ERROR: </font> <font color='green'>Configure tcpdump options failed.</font>";
				document.getElementById("time").innerHTML = str;
				
			},
			success: function(data){			//request success callback function;
	
			}
		});

}

function send_dump_capture_request()
{	
	minutes = '00', seconds = '00';
	minute = 0;
	second = 0;
	window.clearInterval(interval);
	var server_file = "./../../cgi-bin/php/ajax_server.php";
	var url=server_file+"?random="+Math.random()+"&type=system&action=dump_capture";

	window.location.href=url;
	// var server_file = "./../../cgi-bin/php/ajax_server.php";
	// $.ajax({
	// 	url: server_file+"?random="+math.random()+"&type=system&action=dump_capture",
	// 	async: false,
	// 	datatype: 'text',
	// 	type: 'get',
	// 	timeout: 5000,
	// 	error: function(){

	// 	},
	// 	success: function(data){

	// 	}
	// });
}

function checkValue(value)
{
	if(trim(value) == "") {
		alert("Please input a domain or IP address\n");
		return false;
	}

	return true;
}
function protocol_change(){
	var protocol_type = document.getElementById('protocol').value;
	if (protocol_type == 'sip') {
		document.getElementById('cap_port').value = '5060';
	} else {
		document.getElementById('cap_port').value = '';
	}
}
function preview_dialog() 
{
	document.getElementById('send').value='Start Capture';
	click();
	send_capture_request();
	$("#preview_dg").dialog({
		resizable: false,
		height:400,
		width:500,
		modal: true,

		buttons: [
			{
				text:"<?php echo language('Stop Capture');?>",
				id:"close",
				style:"text-align: center",
				click:function(){
					document.getElementById('send').value = 'Stop Capture';
					console.log('close');
				    $(this).dialog( "close" );
				    send_dump_capture_request();
				}
			}
		]		
	});
	$(".ui-button").click(function(){
		    hour=minute=second=0;//初始化
			minutes = '00';
			seconds = '00';
			millisecond=0;//毫秒
			window.clearInterval(interval);
			send_dump_capture_request();
	});
}

</script>

<?php
if($_POST) {
	if(isset($_POST['select_gsm_ip'])) {
		$select_gsm_ip = trim($_POST['select_gsm_ip']);
	}

	if(isset($_POST['ping_hostname'])) {
		$ping_host = trim($_POST['ping_hostname']);
	}

	if(isset($_POST['tracert_hostname'])) {
		$tracert_host = trim($_POST['tracert_hostname']);
	}

	if(isset($_POST['send'])) {
		if($_POST['send'] == 'Ping' && isset($ping_host)) {
			//Get Ip Address
			$if = "-I $select_gsm_ip";
			$cmd = "ping $if -c 4 $ping_host";
			$err_info = "<font color=ff0000>Fail to ping [ $ping_host ] !</font>";
			$ok_info = "<font color=008800>Successfully ping [ $ping_host ] .</font>";
		} else if($_POST['send'] == 'Traceroute' && isset($tracert_host)) {
			//Get Ip Address
			$if = "-s $select_gsm_ip";
			$cmd = "traceroute $if $tracert_host";
			$err_info = "<font color=ff0000>Fail to traceroute [ $tracert_host ]!</font>";
			$ok_info = "<font color=008800>Successfully traceroute [ $tracert_host ] .</font>";
		}
	}
}

?>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<table width="100%" class="tedit" >
		<tr class="ttitle">
			<td>
				<div class="helptooltips">
					<?php echo language('GSM IP');?>:
					<span class="showhelp">
					<?php echo language('GSM IP help');?>
					</span>
				</div>
				<select name="select_gsm_ip">
<?php
				foreach($gsm_ip as $gsmIp) {
?>
					<option value="<?php echo $gsmIp; ?>"><?php echo $gsmIp; ?></option>
<?php 
				}
?>
				</select>
			</td>
		</tr>
		<tr>
			<td>
				<input type="text" id="ping_hostname" name="ping_hostname" value='<?php if(isset($ping_host)) echo $ping_host; else echo "google.com"; ?>' />
				<input type="submit" value="<?php echo language('Ping');?>" 
					onclick="document.getElementById('send').value='Ping';return checkValue(document.getElementById('ping_hostname').value);"/>	
			</td>
		</tr>
		<tr>
			<td>
				<input type="text" id="tracert_hostname" name="tracert_hostname" value='<?php if(isset($tracert_host)) echo $tracert_host; else echo "google.com"; ?>' />
				<input type="submit" value="<?php echo language('Traceroute');?>" 
					onclick="document.getElementById('send').value='Traceroute';return checkValue(document.getElementById('tracert_hostname').value);"/>
			</td>
		</tr>
	</table>
	<br/>
	<div id="tab">
		<li class="tb1">&nbsp;</li>	
		<li class="tbg"><?php echo language('Channel Recording');?></li>
		<li class="tb2">&nbsp;</li> 
	</div> 

	<table width="100%" class="tedit">
		<tr>
			<th>
				<div class="helptooltips">
				<?php echo language('Interface');?>:
				<span class="showhelp">
				<?php echo language('Interface help','The name of network interface.');?>
				</span>
				</div>
			</th>
			<td>
				<select name="interface_type" id="interface_type" >
				<?php
						foreach($gsm_ifname as $gsm_if) {
				?>
						<option value="<?php echo $gsm_if; ?>" <?php if($interface_type == $gsm_if) echo "selected=\"selected\""; ?>><?php echo $gsm_if; ?></option>
				<?php 
						}
				?>
				</select>
				<span id="cinterface_type"></span>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
				<?php echo language('Source host');?>:
				<span class="showhelp">
				<?php echo language('Source host help',' .');?>
				</span>
				</div>
			</th>
			<td>
				<textarea id="source_host" name="source_host" width="100%" rows="3" cols="80" ><?php echo $source_host?></textarea>
				<span id="csource_host"></span>
			</td>
		</tr>
		<tr>
			<th>
			<div class="helptooltips">
				<?php echo language('Destination host');?>:
				<span class="showhelp">
				<?php echo language('Destination host help','The name of network interface.');?>
				</span>
				</div>
			</th>
			<td>
				<textarea id="dest_host" name="dest_host" width="100%" rows="3" cols="80" ><?php echo $destination_host?></textarea>
				<span id="cdest_host"></span>
			</td>
		</tr>
		<tr>
			<th>
			<div class="helptooltips">
				<?php echo language('Port');?>:
				<span class="showhelp">
				<?php echo language('Port help','The name of network interface.');?>
				</span>
				</div>
			</th>
			<td>
				<input id="cap_port" type="text" name="cap_port" value="<?php echo $cap_port;?>" /><span id="ccap_port"></span>
			</td>
		</tr>
		<tr>
			<th>
			<div class="helptooltips">
				<?php echo language('Protocol');?>:
				<span class="showhelp">
				<?php echo language('Protocol help','The name of network interface.');?>
				</span>
				</div>
			</th>
			<td>
				<select id="protocol" name="protocol" onchange="protocol_change()">
					<option value="all" <?php echo $protocol_type['all'];?>>All</option>
					<option value="tcp" <?php echo $protocol_type['tcp'];?>>TCP</option>
					<option value="udp" <?php echo $protocol_type['udp'];?>>UDP</option>
					<option value="rtp" <?php echo $protocol_type['rtp'];?>>RTP</option>
					<option value="rtcp" <?php echo $protocol_type['rtcp'];?>>RTCP</option>
					<option value="icmp" <?php echo $protocol_type['icmp'];?>>ICMP</option>
					<option value="arp" <?php echo $protocol_type['arp'];?>>ARP</option>
					<option value="sip" <?php echo $protocol_type['sip'];?>>SIP</option>
				</select>
			</td>
		</tr>
	</table>
	<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
	<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>
	<input type="hidden" name="send" id="send" value="" />
	<br>
		<input type="button" value="<?php echo language('Start');?>" align="middle" onclick="preview_dialog()"/>

		<div id="preview_dg" title="<?php echo language('Capture Network Data')?>" style="display:none;width:470px;height:100px">
		<div>
			<div id="timemsg" style="display:block;width:470px;height:100px;margin:0" contenteditable = "false">
					<div id="time" ></div>
					<div id="prompt_info"></div>
			</div>
		</div>
		</div>
	</form>
	<br>
<?php
if($_POST) {
	if(isset($cmd)) {
		$Report = language('Report');
		$Result = language('Result');
		trace_output_start("$Report", $cmd);
		trace_output_newline();
		$error = false;
		$cmd .= " 2>&1 || echo \"err_flag\"";
		$file = popen($cmd,"r");
		while(!feof($file)) {
			$line = fgets($file);
			if($line == "err_flag\n") {
				$error = true;
				break;
			}
			echo $line."<br>";
			ob_flush();
			flush();
		}
		pclose($file);

		trace_output_newhead("$Result");
		if($error) {
			echo $err_info;
		} else {
			echo $ok_info;
		}
		trace_output_end();
	} else if (isset($err_info)) {
		echo $err_info;
	}

}
?>
<?php require("/www/cgi-bin/inc/boot.inc");?>
