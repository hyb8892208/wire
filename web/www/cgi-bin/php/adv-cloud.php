<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/check.inc");
//include_once("/www/cgi-bin/php/ajax_server_new.php");
$aql = new aql();
$setok = $aql->set('basedir','/etc/config/');
if (!$setok) {
	echo $aql->get_error();
	return false;
}
$manager_conf_path='/etc/config/cloud.conf';
$hlock = lock_file($manager_conf_path);
if(!$aql->open_config_file($manager_conf_path)){
	echo $aql->get_error();
	unlock_file($hlock);
	return false;
}
if($_POST && isset($_POST['send']) && $_POST['send'] == 'Operation') {
	$enable=isset($_POST['enable'])?'yes':'no';
	$username=$_POST['username'];
	$password=$_POST['password'];

	$china_server = "cloud.openvox.com.cn";
	$america_server = "cloud.openvox.cn";
	$europe_server = "cloud-eur.openvox.cn";

	if(isset($_POST['service_mode'])){
		$service_mode = trim($_POST['service_mode']);
	}
	
	if($service_mode == 'china'){
		$server = $china_server;
	} else if($service_mode == "america"){
		$server = $america_server;
	} else if($service_mode == "europe"){
		$server = $europe_server;
	}

	if(isset($_POST['customize_server']) && $service_mode == "customize"){
		$server_url = trim($_POST['customize_server']);
		if(strstr($server_url, '://')){
			$temp = explode("://", $server_url);
			$temp = explode('/' ,$temp[1]);
			$server = $temp[0];
		}else{
			$temp = explode('/' ,$server_url);
			$server = $temp[0];
		}
		$server = trim($server);
	}

	$exist_array = $aql->query("select * from cloud.conf");
	if(isset($exist_array['cloud']['enable'])) {
		$aql->assign_editkey('cloud','enable',$enable);
	} else {
		$aql->assign_append('cloud','enable',$enable);
	} 
	if(isset($exist_array['cloud']['username'])) {
		$aql->assign_editkey('cloud','username',$username);
	} else {
		$aql->assign_append('cloud','username',$username);
	} 
	if(isset($exist_array['cloud']['password'])) {
		$aql->assign_editkey('cloud','password',$password);
	} else {
		$aql->assign_append('cloud','password',$password);
	} 

	if(isset($exist_array['cloud']['server_country'])) {
		$aql->assign_editkey('cloud','server_country',$service_mode);
	} else {
		$aql->assign_append('cloud','server_country',$service_mode);
	} 
	if(isset($exist_array['cloud']['server_url'])) {
		$aql->assign_editkey('cloud','server_url',$server);
	} else {
		$aql->assign_append('cloud','server_url',$server);
	} 

	if (!$aql->save_config_file('cloud.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}

	unlock_file($hlock);
	if($enable == 'no'){
		exec("/etc/init.d/cloud stop > /dev/null 2>&1 &");
	} else if($enable == 'yes'){
		exec("echo -n '' > /etc/config/cloud_status");
		exec("/etc/init.d/cloud restart > /dev/null 2>&1 &");
	}
}
$exist_array = $aql->query("select * from cloud.conf");
$enable=$exist_array['cloud']['enable'];
if($enable=='yes') $enable='checked';
else $enable='';
$username=$exist_array['cloud']['username'];
$password=$exist_array['cloud']['password'];

$service_mode = array();
$service_mode['china'] = '';
$service_mode['america'] = '';
$service_mode['europe'] = '';
$service_mode['customize'] = '';
/*get the language type */
$conf_path = "/etc/asterisk/gw/web_language.conf";
$conf_array = get_conf($conf_path);
if(isset($conf_array['general']['language']) && $conf_array['general']['language'] != ''){
	$language_type = trim($conf_array['general']['language']);
}

$selecte_mode ='';
if($language_type == 'chinese'){
	$select_mode = 'china';
} else if($language_type == 'english'){
	$select_mode = 'america';
}
if(isset($exist_array['cloud']['server_country'])){
	$select_mode = trim($exist_array['cloud']['server_country']);
}
if ($select_mode == 'customize'){
	if(isset($exist_array['cloud']['server_url'])){
		$server_url = trim($exist_array['cloud']['server_url']);
	}
} else {
	$server_url = '';
}

$service_mode["$select_mode"] = 'selected';
?>
<script type="text/javascript" src="/js/functions.js">
</script>
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
 
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />
<script type="text/javascript" src="/js/check.js"></script>

<script type="text/javascript">
function check_enable_ami_change()
{
	var enable_ami = '<?php echo $enable;?>';
	if (enable_ami != ''){
		get_state();
	} else {
		var connect_no="<span style='color:red'><?php echo language('Cloud Service Disconnected');?></span>";
		$('.connect').html(connect_no);
	}
}
function check()
{
	var command = document.getElementById("command").value;
	if(trim(command) == '') {
		return false;
	}
	return true;
}
function check_op()
{
	return true;
	var channelnum = document.getElementById("channelnum").value;
	var signalling=document.getElementById("signalling").value
	if(trim(channelnum) == ''){
		return false;
	}
	return true;
}
function mode_change(){

	var service_mode = $("#service_mode").val();
	if(service_mode != 'customize'){
		$("#customize_server").hide();
	} else {
		$("#customize_server").show();
	}
}
</script>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<input type="hidden" name="send" id="send" value="" />
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Cloud');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	<table width="100%" class="tedit" >
		<tr>
			<th>
			<?php echo language('Enable Cloud Service');?>:
			</th>
			<td>
			<input type="checkbox" name="enable" id="enable_ami" onchange="check_enable_ami_change()" <?php echo $enable?>  >
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Choose Service');?>:
					<span class="showhelp">
					<?php echo language('Choose Service help', "choosing a remote server the current device will connect");
					?>
					</span>
				</div>
			</th>
			<td>	
					<select name="service_mode" id="service_mode" onchange="mode_change()">
						<option value="china" <?php echo $service_mode['china'];?>><?php echo language('China');?></option>
						<option value="america" <?php echo $service_mode['america'];?>><?php echo language('America');?></option>
						<option value="europe" <?php echo $service_mode['europe'];?>><?php echo language('Europe');?></option>
						<option value="customize" <?php echo $service_mode['customize'];?>><?php echo language('Customize');?></option>
					</select> &nbsp;&nbsp;&nbsp;
					<input class="server_url" size="30px" type="text" name="customize_server" id="customize_server" value="<?php echo $server_url;?>" />		
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Account');?>:
					<span class="showhelp">
					<?php echo language('cloud help@adv-account', "the account which is registered in OpenVox cloud center.");
					?>
					</span>
				</div>
			</th>
			<td>		
					<input type="text" name="username" value="<?php echo $username?>" >		
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<span style="color:red">* &nbsp;</span><?php echo language('Password');?>:
					<span class="showhelp">
					<?php echo language('cloud help@adv-password',"The password of cloud account.");
					?>
					</span>
				</div>
			</th>
			<td>		
				<input type="password" id="password" name="password"   value="<?php echo $password?>" >
				<span style="vertical-align:middle;display:inline-block;"><input type="checkbox" id="show_password" /></span>
			</td>
		</tr>
		<tr class="connect2">
			<th>
				<div class="helptooltips">
					<span style="color:red">* &nbsp;</span><?php echo language('Connection Status');?>:
					<span class="showhelp">
					<?php echo language('cloud help@adv-Connection Status',"Cloud management connection status.");
					?>
					</span>
				</div>
			</th>
			<td class="connect">		
					
			</td>
		</tr>
		<tr>
			<th>				
			</th>
			<td>		
				<input type="submit" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Operation';return check_op();"  style="margin-top:6px;"/>			
				 <p style="float:right;margin-right:40%;" ><?php echo language('cloud signmessage help',"Don't have an account?");?>&nbsp;&nbsp;
				 <?php 
					$language_type = get_language_type(); 
					if($language_type == "chinese"){
				?>
				 <a href="https://cloud.openvox.com.cn" target="black"><?php echo language('cloud sign help',"Sign up");?></a>
				 <?php
				 	} else{
				 ?>
				 <a href="https://cloud.openvox.cn" target="black"><?php echo language('cloud sign help',"Sign up");?></a>
				 <?php
				 	}
				 ?>
				 </p>	
			</td>
		</tr>
	</table>	
	</form>
<script type="text/javascript">
var connect ="<?php echo language('connected');?>";
var connect_n ="<?php echo language('no connected');?>";
var connect_ok="<span style='color:green'>"+connect+"</span>";
var interval;
/*
$(function(){  
    //propertychange监听input里面的字符变化,属性改变事件  
    $('.service_url').bind('input propertychange', function() {  
        var $this = $(this);  
        console.log($this);  
        var text_length = $this.val().length;//获取当前文本框的长度  
        var current_width = parseInt(text_length) * 8;//该16是改变前的宽度除以当前字符串的长度,算出每个字符的长度  
        console.log(current_width)  
        $this.css("width",current_width+"px");  
    });  
})
*/
$(document).ready(function (){ 
var value=$("#enable_ami").attr('checked');
if(value!='checked'){
	$('.connect2').hide();
}else{
	$('.connect2').show();
	get_state();  
	//$('.connect').html('<img src="/images/loading.gif"/>');
	interval = setInterval(get_state, 10000);
} 
$("#enable_ami").change(function(){
	var value=$(this).attr('checked');
	if(value!='checked')$('.connect2').hide();
	else $('.connect2').show();
});
	$("#enable_ami").iButton();
	onload_func();
	mode_change();
}); 

function get_state(){
	//$('.connect').html('');
	$.ajax({
		type:'GET',
		url: '/cgi-bin/php/ajax_server.php?type=state_cloud',
		success: function(data) {
			var data=$.trim(data);
			if(data=='200'){
				$('.connect').html(connect_ok);
				window.clearInterval(interval);
			}else if(data==''){
				$('.connect').html('<img src="/images/loading.gif"/>');
			}else{
				if(data == '440'){
					var  str = "<?php echo language("Password error");?>";
				}else if(data == '441'){
					var str = "<?php echo language("User does not exist");?>";
				}else if(data == '442'){
					var str = "<?php echo language("You have been blacklisted");?>";
				}else{
					var str = "<?php echo language("Connection timed out");?>";
				}
				var connect_no="<span style='color:red'>"+str+"</span>";
				$('.connect').html(connect_no);
			}
			//connect
		}
	});
}

function onload_func()
{
	enable_ami_change();
}
function enable_ami_change()
{
	var sw = document.getElementById('enable_ami').checked;

	if(sw) {
		set_visible('field_manager', true);
		set_visible('field_rights', true);
		set_visible('field_port', true);

		//obj = document.getElementById('port');
		//obj.disabled = false;
	} else {
		set_visible('field_manager', false);
		set_visible('field_rights', false);
		set_visible('field_port', false);

		//obj = document.getElementById('port');
		//obj.disabled = true;
	}
}

$("#show_password").change(function(){
	if($(this).attr("checked") == 'checked'){
		$("#password").prop("type","text");
	}else{
		$("#password").prop("type","password");
	}
});
</script>


<?php require("/www/cgi-bin/inc/boot.inc");?>

