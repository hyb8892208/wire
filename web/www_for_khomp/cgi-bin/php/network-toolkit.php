<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
?>

<?php 

$gsm_ifname = array();
exec("ifconfig -a | grep \"^eth\" | awk '{print $1}'", $gsm_ifname);
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
	$interface_type = "eth0";
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

<script type="text/javascript" src="/js/functions.js"></script>

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
	var server_file = "./../../cgi-bin/php/ajax_server.php";
	$.ajax({
		url: server_file+"?type=system&action=dump_capture",
		type: 'get',
		success: function(data){
			if(data == 'error'){
				alert("Error");
			}
		},
		error: function(){

		}
	});
}

function download_capture(){
	var server_file = "./../../cgi-bin/php/ajax_server.php?type=pcap_capture_download";
	
	window.location.href=server_file;
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

<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>

<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<div class="content">
		<div class="tab_item">
			<span>
				<?php echo language('GSM IP');?>:
				<?php if(is_show_language_help('GSM IP')){ ?>
					<div class="tip_main">
						<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
						<div class="tip_help">
							<i class="top" ></i>
						
							<div class="tip_content">
								<b><?php echo language('GSM IP');?>:</b><br/>
								<?php echo language('GSM IP help','GSM IP');?>
							</div>
						</div>
					</div>
				<?php } ?>
			</span>
			<div class="tab_item_right">
				<select name="select_gsm_ip" id="select_gsm_ip">
				<?php foreach($gsm_ip as $gsmIp) { ?>
					<option value="<?php echo $gsmIp; ?>"><?php echo $gsmIp; ?></option>
				<?php } ?>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<input type="text" style="height:30px;width:330px;" id="ping_hostname" name="ping_hostname" value='<?php if(isset($ping_host)) echo $ping_host; else echo "google.com"; ?>' />
			<span>
			<div class="tab_item_right">
				<input type="button" value="<?php echo language('Ping');?>" onclick="ping();"/>	
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<input type="text" style="height:30px;width:330px;" id="traceroute_hostname" name="traceroute_hostname" value='<?php if(isset($tracert_host)) echo $tracert_host; else echo "google.com"; ?>' />
			</span>
			<div class="tab_item_right">
				<input type="button" value="<?php echo language('Traceroute');?>" onclick="traceroute();"/>
			</div>
		</div>
	</div>

	<div class="content">
		<span class="title">
			<?php echo language('Channel Recording');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Interface')){ ?>
							<b><?php echo language('Interface');?>:</b><br/>
							<?php echo language('Interface help','The name of network interface.');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Source host')){ ?>
							<b><?php echo language('Source host');?>:</b><br/>
							<?php echo language('Source host help',' .');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Destination host')){ ?>
							<b><?php echo language('Destination host');?>:</b><br/>
							<?php echo language('Destination host help','The name of network interface.');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Port')){ ?>
							<b><?php echo language('Port');?>:</b><br/>
							<?php echo language('Port help','The name of network interface.');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Protocol')){ ?>
							<b><?php echo language('Protocol');?>:</b><br/>
							<?php echo language('Protocol help','The name of network interface.');?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Interface');?>:
			</span>
			<div class="tab_item_right">
				<span id="cinterface_type"></span>
				<select name="interface_type" id="interface_type" >
				<?php
						foreach($gsm_ifname as $gsm_if) {
				?>
						<option value="<?php echo $gsm_if; ?>" <?php if($interface_type == $gsm_if) echo "selected=\"selected\""; ?>><?php echo $gsm_if; ?></option>
				<?php 
						}
				?>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Source host');?>:
			</span>
			<div class="tab_item_right">
				<span id="csource_host"></span>
				<textarea id="source_host" name="source_host" width="100%" rows="3" cols="80" ><?php echo $source_host?></textarea>
			</div>
			<div class="clear"></div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Destination host');?>:
			</span>
			<div class="tab_item_right">
				<span id="cdest_host"></span>
				<textarea id="dest_host" name="dest_host" width="100%" rows="3" cols="80" ><?php echo $destination_host?></textarea>
			</div>
			<div class="clear"></div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Port');?>:
			</span>
			<div class="tab_item_right">
				<span id="ccap_port"></span>
				<input id="cap_port" type="text" name="cap_port" value="<?php echo $cap_port;?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Protocol');?>:
			</span>
			<div class="tab_item_right">
				<select id="protocol" name="protocol" onchange="protocol_change()">
					<option value="all" <?php echo $protocol_type['all'];?>><?php echo language('All');?></option>
					<option value="tcp" <?php echo $protocol_type['tcp'];?>><?php echo language('TCP');?></option>
					<option value="udp" <?php echo $protocol_type['udp'];?>><?php echo language('UDP');?></option>
					<option value="rtp" <?php echo $protocol_type['rtp'];?>><?php echo language('RTP');?></option>
					<option value="rtcp" <?php echo $protocol_type['rtcp'];?>><?php echo language('RTCP');?></option>
					<option value="icmp" <?php echo $protocol_type['icmp'];?>><?php echo language('ICMP');?></option>
					<option value="arp" <?php echo $protocol_type['arp'];?>><?php echo language('ARP');?></option>
					<option value="sip" <?php echo $protocol_type['sip'];?>><?php echo language('SIP');?></option>
				</select>
			</div>
		</div>
		
		<div class="content">
			<span class="title"><?php echo language('Packge Capture');?></span>
			
			<div style="text-align:center;width:800px;margin:auto;">
				<button id="capture_start" class="capture_packge" type="button" style="float:left;"><?php echo language('Start');?></button>
				<button id="capture_stop" class="capture_packge capture_packge_disabled" type="button" disabled><?php echo language('Stop');?></button>
				<button id="capture_download" class="capture_packge" type="button" style="float:right;"><?php echo language('Download');?></button>
			</div>
			<div id="running_time_div" style="width:800px;margin:auto;padding-left:40px;margin-top:20px;display:none;">
				<?php echo language('Running Time');?>:<span id="running_time" style="margin-left:10px;"></span>
			</div>
		</div>
	</div>
	
	<pre id="ping_status"></pre>

	<input type="hidden" name="send" id="send" value="" />
	
	<!--
	<div id="button_save">
		<?php if(!$only_view){ ?>
		<button type="button" align="middle" onclick="preview_dialog()"><?php echo language('Start');?></button>
		<?php } ?>
	</div>
	-->

	<div id="preview_dg" title="<?php echo language('Capture Network Data')?>" style="display:none;width:470px;height:100px">
	<div>
		<div id="timemsg" style="display:block;width:470px;height:100px;margin:0" contenteditable = "false">
			<div id="time" ></div>
			<div id="prompt_info"></div>
		</div>
	</div>
	</div>
</form>

<script>
function return_html(title){
	var str = "<table class='table_show'>";
	str += "<tr><th><b>"+title+"</b></th></tr>";
	str += "<tr>";
	str += "<tr><td style='text-align:left;background-color:#fff;' id='return_data'>";
	str += "</td></tr></table>";
	
	$("#ping_status").html(str);
}

function ping(){
	var select_gsm_ip = document.getElementById('select_gsm_ip').value;
	var ping_hostname = document.getElementById('ping_hostname').value;
	
	if(!checkValue(ping_hostname)){
		return false;
	}
	
	return_html('Ping');
	$.ajax({
		url: "./../../cgi-bin/php/ajax_server.php?type=network_ping_or_traceroute",
		type: 'POST',
		data: {
			'select_ip':select_gsm_ip,
			'ping_hostname':ping_hostname,
			'network_type':'ping'
		},
		success: function(data){
			get_ping_status();
		},
		error: function(){
			alert('ping error');
		}
	});
}

function get_ping_status(){
	$.ajax({
		url: "./../../cgi-bin/php/ajax_server.php?type=get_ping_or_traceroute&network_type=ping",
		type: 'GET',
		success: function(data){
			$("#return_data").html(data);
			if(data.indexOf("OK") == -1){
				setTimeout(get_ping_status, 1000);
			}
		},
		error:function(){
		}
	});
}

function traceroute(){
	var select_gsm_ip = document.getElementById('select_gsm_ip').value;
	var traceroute_hostname = document.getElementById('traceroute_hostname').value;
	
	if(!checkValue(traceroute_hostname)){
		return false;
	}
	
	return_html('Traceroute');
	
	$.ajax({
		url: "./../../cgi-bin/php/ajax_server.php?type=network_ping_or_traceroute",
		type: 'POST',
		data: {
			'select_ip':select_gsm_ip,
			'traceroute_hostname':traceroute_hostname,
			'network_type':'traceroute'
		},
		success: function(data){
			get_traceroute_status();
		},
		error: function(){
			alert('traceroute error');
		}
	});
}

function get_traceroute_status(){
	$.ajax({
		url: "./../../cgi-bin/php/ajax_server.php?type=get_ping_or_traceroute&network_type=traceroute",
		type: 'GET',
		success: function(data){
			$("#return_data").html(data);
			if(data.indexOf("OK") == -1){
				setTimeout(get_traceroute_status, 1000);
			}
		},
		error: function(){
		}
	});
}

function show_start_capture(){
	$("#capture_start").attr("disabled","disabled");
	$("#capture_start").addClass('capture_packge_disabled');
	
	$("#capture_stop").removeAttr("disabled");
	$("#capture_stop").removeClass("capture_packge_disabled");
	
	$("#capture_download").attr("disabled","disabled");
	$("#capture_download").addClass("capture_packge_disabled");
}

function show_stop_capture(){
	$("#capture_start").removeAttr("disabled");
	$("#capture_start").removeClass("capture_packge_disabled");
	
	$("#capture_stop").attr("disabled","disabled");
	$("#capture_stop").addClass("capture_packge_disabled");
	
	$("#capture_download").removeAttr("disabled");
	$("#capture_download").removeClass("capture_packge_disabled");
}

$("#capture_start").click(function(){
	//启动抓包
	show_start_capture();
	send_capture_request();
	get_capture_running_status();
});

$("#capture_stop").click(function(){
	//停止抓包
	show_stop_capture();
	send_dump_capture_request();
});

$("#capture_download").click(function(){
	//下载抓包
	download_capture();
});

function get_capture_running_status(){
	$.ajax({
		url:"./../../cgi-bin/php/ajax_server.php?type=get_pcap_capture_status",
		type:"GET",
		success:function(data){
			if(data != "none"){
				//显示运行了多久
				$("#running_time_div").show();
				$("#running_time").html(data);
				
				show_start_capture();
				setTimeout(get_capture_running_status,1000);
			}else{
				$("#running_time_div").hide();
				show_stop_capture();
			}
		}
	});
	
}

get_capture_running_status();
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>
