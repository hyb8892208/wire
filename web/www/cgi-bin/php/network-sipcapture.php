<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/check.inc");

?>

<script type="text/javascript" src="/js/functions.js">
</script>

<script type="text/javascript" src="/js/check.js"></script>

<?php
$command = '';
$result = '';
if($_POST && isset($_POST['send']) && $_POST['send'] == 'Execute') {
	if(isset($_POST['command'])) { 
		$command = trim($_POST['command']);
		if($command != '') {
			$result = execute_astcmd($command);
		}
	}
}
$gsm_ifname = array();

$product_type = get_product_type();

if($product_type < 4){
	exec("ifconfig -a | grep -E 'eth0|br-lan' | awk '{print $1}'|grep -v \"\.\"", $gsm_ifname);
	$interface_type = 'br-lan';
}else{
	exec("ifconfig -a | grep \"^eth\" | awk '{print $1}'", $gsm_ifname);
	$interface_type = 'eth0';
}
?>
<script type="text/javascript">
var hour,minute,second;//时 分 秒
hour=minute=second=0;//初始化
var minutes = '00';
var seconds = '00';
var millisecond=0;//毫秒
var interval;
var len;
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
		if(minute>=3){
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
	}, 1000);

}

function send_capture_request()
{
	var invite, options, register;
	
	var interface = document.getElementById('interface_type').value;
	if(document.getElementById('invite').checked){
		invite = 'INVITE';
	} else {
		invite = ''
	}
	if(document.getElementById('options').checked){
		options = 'OPTIONS';
	} else {
		options = '';
	}
	if(document.getElementById('register').checked){
		register = 'REGISTER';
	} else {
		register = ''
	}
	if(invite == '' && options == '' && register == ''){
		alert("You must be choosing one of them at least !!!");
		return ;
	}
	console.log('invite='+invite+'options='+options+'register='+register);

	var server_file = "/cgi-bin/php/ajax_server.php";
	var request_param = "&interface="+interface+"&invite="+invite+"&options="+options+"&register="+register;
	$.ajax({
			url: server_file+"?random="+Math.random()+"&type=system&action=sipcapture"+request_param,
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
	var server_file = "/cgi-bin/php/ajax_server.php";
	var url=server_file+"?random="+Math.random()+"&type=system&action=dump_sipcapture";

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

function choose_all(){
	if(document.getElementById('all').checked){
		document.getElementById('invite').checked = true;
		document.getElementById('options').checked = true;
		document.getElementById('register').checked	= true;
	} else {
		document.getElementById('invite').checked = false;
		document.getElementById('options').checked = false;
		document.getElementById('register').checked	= false;	
	}
}
function check_param()
{
	var invite = document.getElementById('invite').checked;
	var options = document.getElementById('options').checked;
	var register = document.getElementById('register').checked;
	if(invite || options || register){
		preview_dialog();
		return true;
	} else {
		alert("You must be choosing one of them at least !!!");
		return false;
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

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('SIP Capture');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	<table width="100%" class="tedit" >
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
					<option value="<?php echo $gsm_if; ?>" <?php if($interface_type == "$gsm_if") echo "selected=\"selected\""; ?>><?php echo $gsm_if; ?></option>
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
				<?php echo language('Method-filter');?>:
				<span class="showhelp">
				<?php echo language('Interface help','The name of network interface.');?>
				</span>
				</div>
			</th>
			<td>
				<input type="checkbox" name="invite" id="invite" checked="checked" >INVITE<br>
				<input type="checkbox" name="options" id="options" >OPTIONS<br>
				<input type="checkbox" name="register" id="register" >REGISTER<br>
				<input type="checkbox" name="all" id="all" onclick="choose_all()" >All<br>
				<span id="coption"></span>
			</td>
		</tr>	
	</table>
	<br>
	<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
	<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>
	<input type="hidden" name="send" id="send" value="" />
	<input type="button" value="<?php echo language('Start Capture');?>" onclick="return check_param();" />	
	<div id="preview_dg" title="<?php echo language('Capture Sip')?>" style="display:none;width:470px;height:100px">
		<div>
			<div id="timemsg" style="display:block;width:470px;height:100px;margin:0" contenteditable = "false">
					<div id="time" ></div>
					<div id="prompt_info"></div>
			</div>
		</div>
	</div>
	</form>

<?php
if($result != '') {
	echo "<h5>";echo language("Output");echo ":</h5>";
	echo $result;
}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>
