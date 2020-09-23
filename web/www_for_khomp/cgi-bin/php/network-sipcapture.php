<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/check.inc");

?>

<script type="text/javascript" src="/js/functions.js"></script>
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
exec("ifconfig -a | grep \"^eth\" | awk '{print $1}'", $gsm_ifname);
$interface_type = 'eth0';
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

	var server_file = "./../../cgi-bin/php/ajax_server.php";
	$.ajax({
		url: server_file+"?type=system&action=dump_sipcapture",
		type: 'get',
		success: function(data){
			if(data == 'error'){
				alert("Error");
			}
		}
	});
}

function sip_capture_download(){
	var server_file = "./../../cgi-bin/php/ajax_server.php?type=sip_capture_download";
	
	window.location.href=server_file;
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

<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>
<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<div class="content">
		<span class="title">
			<?php echo language('SIP Capture');?>
			
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
						
						<?php if(is_show_language_help('Method-filter')){ ?>
							<b><?php echo language('Method-filter');?>:</b><br/>
							<?php echo language('Method-filter help','The name of network interface.');?>
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
				<select name="interface_type" id="interface_type" >
				<?php
					foreach($gsm_ifname as $gsm_if) {
				?>
					<option value="<?php echo $gsm_if; ?>" <?php if($interface_type == "$gsm_if") echo "selected=\"selected\""; ?>><?php echo $gsm_if; ?></option>
				<?php 
					}
				?>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Method-filter');?>:
			</span>
			<div class="tab_item_right">
				<input type="checkbox" name="invite" id="invite" checked="checked" ><?php echo language('INVITE');?><br>
				<input type="checkbox" name="options" id="options" ><?php echo language('OPTIONS');?><br>
				<input type="checkbox" name="register" id="register" ><?php echo language('REGISTER');?><br>
				<input type="checkbox" name="all" id="all" onclick="choose_all()" ><?php echo language('All');?><br>
			</div>
			<div class="clear"></div>
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

	<input type="hidden" name="send" id="send" value="" />
	
	<!--
	<div id="button_save">
		<button type="button" onclick="return check_param();" ><?php echo language('Start Capture');?></button>
	</div>
	-->
	
	<div id="preview_dg" title="<?php echo language('Capture Sip')?>" style="display:none;width:470px;height:100px">
		<div>
			<div id="timemsg" style="display:block;width:470px;height:100px;margin:0" contenteditable = "false">
					<div id="time" ></div>
					<div id="prompt_info"></div>
			</div>
		</div>
	</div>
	</form>
	
<script>
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
	sip_capture_download();
});

function get_capture_running_status(){
	$.ajax({
		url:"./../../cgi-bin/php/ajax_server.php?type=get_sip_capture_status",
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

<?php
if($result != '') {
	echo "<h5>";echo language("Output");echo ":</h5>";
	echo $result;
}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>