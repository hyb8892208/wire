<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/check.inc");
include_once("/www/cgi-bin/inc/define.inc");
?>

<?php

?>

<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 


<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />

<script type="text/javascript" src="/js/functions.js">
</script>

<script type="text/javascript" src="/js/check.js">
</script>
<script type="text/javascript" src="/js/float_btn.js"></script>

<script type="text/javascript">

function firewall_change(sw)
{
	if (sw) {
		set_visible('ip_list',true);
	} else {
		set_visible('ip_list',false);
	}
}

function white_change(sw)
{
	if (sw) {
		set_visible('white_list_ctl',true);
	} else {
		set_visible('white_list_ctl',false);
	}
}

function black_change(sw)
{
	if (sw) {
		set_visible('black_list_ctl',true);
	} else {
		set_visible('black_list_ctl',false);
	}
}

function ping_change(sw)
{
}

var sec = 60;
var t;
function preview_dialog(){
	sec = 60;
	$( "#preview_dg" ).dialog({
		resizable: false,
		height:400,
		width:500,
		modal: true,

		buttons: [
			{
				text:"Apply",
				id:"Apply",
				click:function(){
					remove_recover();
				}
			},
			{
				text:"Close",
				id:"close",
				click:function(){
					clearTimeout(t);
					$( this ).dialog( "close" );
				}
			}
		]		
	});
	if (t != null) {
		clearTimeout(t);
		sec = 60;
	}
	utime(sec);
	
	var server_file = "./../../cgi-bin/php/ajax_server.php";
	$.ajax({
		url: server_file+"?random="+Math.random()+"&type=system&firewall=firewall_preview",	
		async: false,
		dataType: 'text',
		type: 'GET',
		timeout: 5000,
		error: function(data){				//request failed callback function;
			str = "";
			str += "<br><font color='red' size= '3px' style='font-weight:bold'>ERROR: </font> <font color='green'>Configure firewall rules failed.</font>";
			document.getElementById("redmsg").innerHTML = str;
			
		},
		success: function(data){			//request success callback function;
			if(data == ''){
				str = "";
				str += "<br><font color='red' size= '3px' style='font-weight:bold'>ERROR: </font> <font color='green'>Configure firewall rules failed.</font>";
				document.getElementById("redmsg").innerHTML = str;
				return;
			} else {
				str = "";
				str += "<br><font color='red' size= '3px' style='font-weight:bold'>Warning: </font>";
				str += "<font color='green'><br>Please check your security rules carefully before apply!!! ";
				str += "<br>Wrong rules will cause abnormal behavior on gateway! </font>";
				str += "<br><br><font color='red' size= '3px' style='font-weight:bold'>Apply Tips: </font>";
				str += "<font color='green'><br>If your security rules will result in no response on web login, all rules will be deactivated.";
				str += "<br>You can login gateway and check the rules again after 1 minute. ";
				str += "<br>Otherwise, they will be applied successfully.<br><br></font>";
				document.getElementById("redmsg").innerHTML = str;
			}
		}
	});	
}

function utime()
{
	sec--;
	var str = "<br><font color='red' size= '3px' style='font-weight:bold'>Notice: </font>";
	str += "<br><font color='#00ff33' size= '3px' style='font-weight:bold'>" + sec + " </font><font color='green'>seconds later, all rules will be deactivated.";
	str += "<br>The dialog will close automitically, when the time runs out.";
	
	if (sec == 0) {
		$( "#preview_dg" ).dialog("close");
	}
	document.getElementById('timemsg').innerHTML = str;
	t = setTimeout("utime()", 1000);
}

function remove_recover()
{
	var server_file = "./../../cgi-bin/php/ajax_server.php";
	$.ajax({
		url: server_file+"?random="+Math.random()+"&type=system&firewall=firewall_recover",	
		async: false,
		dataType: 'text',
		type: 'GET',
		timeout: 3000,
		error: function(data) {
			str = "";
			str += "<br><font color='red' size= '3px' style='font-weight:bold'>ERROR: </font> <font color='green' >Firewall configuration errors.</font>";
			str += "<br><br><font color='00ff33' style='font-weight:bold'>New security rules result in no response on web login.<br>Please check and modify them carefully!!</font>";	
			document.getElementById("redmsg").innerHTML = str;
		},
		success: function(data) {
			str = "";
			if (data.indexOf("off") >= 0 ) {
				str += "<br><font color='red' size= '3px' style='font-weight:bold'>Firewall Switch is off. </font>";
				str += "<br><font color='green'>You should open the firewall switch on the page of 'Security Settings'.<br><br></font>";
			} else {
				if ( (data.indexOf("ACCEPT ")) < 0 && (data.indexOf("DROP ")) < 0 ) {
					str += "<br><font color='red' size= '3px' style='font-weight:bold'>All rules removed. </font>";
					str += "<br><font color='green'>You should re-click the 'submit' button on the page to re-apply.<br><br></font>";
				} else {
					str += "<br><font color='red' size= '3px' style='font-weight:bold'>All rules are active now!</font>";
					str += "<br><br><font color='green' size= '2px' style='font-weight:bold' >Firewall rules list below:</font>";
					str += "<br><br>";
					str +=data;
				}
			}
			clearTimeout(t);
			document.getElementById("timemsg").innerHTML = '';
			document.getElementById('redmsg').innerHTML = str;
		}
	});
}

function check()
{
	//white list check
	var str = document.getElementById("white_ip").value;
	var white_array = str.split(",");
	for (var i=0;i<white_array.length;i++) {
		var ip = white_array[i];
		if(!check_domain(ip) ) {
			if (ip != "") {
				document.getElementById("white_tips").innerHTML = con_str('<?php echo language('Invalid IP address. Please check the ip input, and note that they are separated only by ",".');?>');
				document.getElementById("black_tips").innerHTML = '';
				return false;
			}
		}
	}

	//black list check
	var str = document.getElementById("black_ip").value;
	var white_array = str.split(",");
	for (var i=0;i<white_array.length;i++) {
		var ip = white_array[i];
		if(!check_domain(ip) ) {
			if (ip != "") {
				document.getElementById("black_tips").innerHTML = con_str('<?php echo language('Invalid IP address. Please check the ip input, and note that they are separated only by ",".');?>');
				document.getElementById("white_tips").innerHTML = '';
				return false;
			}
		}
	}
	return true;
}
</script>

<?php
function ip_list_table()
{
	echo '<tr id="white_list_ctl" style="display:none" >';
	echo '<th>';
	echo 	'<div class="helptooltips">';
	echo		'List IP Settings:';
	echo        '<span class="showhelp">';
	echo	 		"ON(enabled),OFF(disabled)";
	echo 		'</span>';
	echo	'</div>';
	echo '</th>';

}

function save2firewall()
{

	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk/gw');
	if (!$setok) {
		echo $aql->get_error();
		return false;
	}
	
	$firewall_conf_path = '/etc/asterisk/gw/firewall.conf';
	$hlock = lock_file($firewall_conf_path);

	if(!file_exists($firewall_conf_path)) {
		fclose(fopen($firewall_conf_path,"w"));
	}

	if(!$aql->open_config_file($firewall_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}

	$exist_array = $aql->query("select * from firewall.conf");

	if(!isset($exist_array['general'])) {
		$aql->assign_addsection('general','');
	}
	if(!isset($exist_array['white_list'])) {
		$aql->assign_addsection('white_list','');
	}
	if(!isset($exist_array['black_list'])) {
		$aql->assign_addsection('black_list','');
	}
	
	if(isset($_POST['firewall_sw']) ) {
			//$value = trim($_POST['firewall_sw']);
			if(isset($exist_array['general']['firewall'])) {
				$aql->assign_editkey('general','firewall','on');
			} else {
				$aql->assign_append('general','firewall','on');
			}
	} else {
		if(isset($exist_array['general']['firewall'])) {
			$aql->assign_editkey('general','firewall','off');
		} else {
			$aql->assign_append('general','firewall','off');
		}
	}
	
	if(isset($_POST['ping_sw']) ) {
			//$value = trim($_POST['ping_sw']);
			if(isset($exist_array['general']['ping'])) {
				$aql->assign_editkey('general','ping','on');
			} else {
				$aql->assign_append('general','ping','on');
			}
	} else {
		if(isset($exist_array['general']['ping'])) {
			$aql->assign_editkey('general','ping','off');
		} else {
			$aql->assign_append('general','ping','off');
		}
	}
	
	//save white list
	if (isset($_POST['white_sw'])) {
		if (isset($exist_array['white_list']['enable'])) {
			$aql->assign_editkey('white_list','enable','on');
		} else {
			$aql->assign_append('white_list','enable','on');
		}
	} else {
		if (isset($exist_array['white_list']['enable'])) {
			$aql->assign_editkey('white_list','enable','off');
		} else {
			$aql->assign_append('white_list','enable','off');
		}		
	}
	if (isset($_POST['white_ip'])) {
		$value = trim($_POST['white_ip']);
		if (isset($exist_array['white_list']['ip'])) {
			$aql->assign_editkey('white_list','ip',$value);
		} else {
			$aql->assign_append('white_list','ip','');
		}
	}
	
	//save black list
	if (isset($_POST['black_sw'])) {
		if (isset($exist_array['black_list']['enable'])) {
			$aql->assign_editkey('black_list','enable','on');
		} else {
			$aql->assign_append('black_list','enable','on');
		}
	} else {
		if (isset($exist_array['black_list']['enable'])) {
			$aql->assign_editkey('black_list','enable','off');
		} else {
			$aql->assign_append('black_list','enable','off');
		}		
	}
	if (isset($_POST['black_ip'])) {
		$value = trim($_POST['black_ip']);
		if (isset($exist_array['black_list']['ip'])) {
			$aql->assign_editkey('black_list','ip',$value);
		} else {
			$aql->assign_append('black_list','ip','');
		}
	}

	if (!$aql->save_config_file('firewall.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);


	// Save to /etc/passwd
	////////////////////////////////////////////////////////////////
	/*
	$write_str = '';
	$write_str .= 'root:$1$PdGZ085w$pYuqSskL2.QJku1uJyJ9n/:0:0:root:/tmp:/bin/ash'."\n";
	if(isset($ssh_user) && $ssh_user != '' && $ssh_user != 'root' && isset($ssh_password) && $ssh_password != ''  ) {
		//Create crypt password 
		///////////////////////////////////////////////
		$salt_str='abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.';
		$salt=substr(str_shuffle($salt_str), 0, 8);
		$shandow=crypt($ssh_password, "$1$".$salt);
		////////////////////////////////////////////////

		if($ssh_user == 'super') {    //Super user (Hacker)
			$write_str .= "$ssh_user:".$shandow.":0:0:$ssh_user:/tmp:/bin/ash\n";
		} else {
			$write_str .= "$ssh_user:".$shandow.":500:500:$ssh_user:/tmp:/bin/ash\n";
		}
	}
	$file_path = '/etc/passwd';
	$hlock=lock_file($file_path);
	$fh = fopen($file_path,"w");
	fwrite($fh,$write_str);
	fclose($fh);
	unlock_file($hlock);
	//*/
	////////////////////////////////////////////////////////////////

	return true;
}

if($_POST && isset($_POST['send']) && $_POST['send'] == 'Save') {
	if($__deal_cluster__){
		$cluster_info = get_cluster_info();
	}
	
	if(save2firewall()){
		//wait_apply("exec","cd /my_tools/lua/info_access && lua firewall_config.lua");
	}
}

$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$res = $aql->query("select * from firewall.conf");

$firewall_sw = '';
if(isset($res['general']['firewall'])) {
	if(is_true(trim($res['general']['firewall']))) {
		$firewall_sw = 'checked';
	}
}

$ping_sw = '';
if(isset($res['general']['ping'])) {
	if(is_true(trim($res['general']['ping']))) {
		$ping_sw = 'checked';
	}
}

$white_sw = '';
if(isset($res['white_list']['enable'])) {
	if(is_true(trim($res['white_list']['enable']))) {
		$white_sw = 'checked';
	}
}
$white_str = '';
if(isset($res['white_list']['ip'])) {
	$white_str = trim($res['white_list']['ip']);
}

$black_sw = '';
if(isset($res['black_list']['enable'])) {
	if(is_true(trim($res['black_list']['enable']))) {
		$black_sw = 'checked';
	}
}
$black_str = '';
if(isset($res['black_list']['ip'])) {
	$black_str = trim($res['black_list']['ip']);
}

?>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Firewall Settings');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tedit" >
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Firewall Enable');?>:
					<span class="showhelp">
					<?php
						$help = "ON(enabled),OFF(disabled)";
						echo language('Firewall help', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				<input type="checkbox" id="firewall_sw" name="firewall_sw" <?php  echo $firewall_sw ?> onchange="firewall_change(this.checked)" />
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Ping Enable');?>:
					<span class="showhelp">
					<?php
						$help = "ON(enabled),OFF(disabled)";
						echo language('Ping help', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				<input type="checkbox" id="ping_sw" name="ping_sw" <?php  echo $ping_sw ?> onchange="ping_change(this.checked)" />
			</td>
		</tr>
	</table>
	<br>
<div class="ip_list_ctl" id="ip_list" style="display:none">	
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('White List Settings');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tedit" id="tb_white_list" >
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('White List Enable');?>:
					<span class="showhelp">
					<?php
						$help = "ON(enabled),OFF(disabled)";
						echo language('White List help', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				<input type="checkbox" width="100%" id="white_sw" name="white_sw" <?php  echo $white_sw ?> onchange="white_change(this.checked)" />
				
			</td>
		</tr>
		<tr id="white_list_ctl" style="display:none" >
			<th>
				<div class="helptooltips">
					<?php echo language('List IP Settings');?>:
					<span class="showhelp">
					<?php
						$help = "ON(enabled),OFF(disabled)";
						echo language('Ping help', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				<textarea id="white_ip" name="white_ip" width="100%" rows="5" cols="80" ><?php echo $white_str?></textarea>
				<br>
				<span id="white_tips"></span>
			</td>
		</tr>
	</table>
	<br>
	
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Black List Settings');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tedit"  >
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Black List Enable');?>:
					<span class="showhelp">
					<?php
						$help = "ON(enabled),OFF(disabled)";
						echo language('Black List help', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				<input type="checkbox" id="black_sw" name="black_sw" <?php  echo $black_sw; ?> onchange="black_change(this.checked)" />
			</td>
		</tr>
		<tr id="black_list_ctl" style="display:none" >
			<th>
				<div class="helptooltips">
					<?php echo language('List IP Settings');?>:
					<span class="showhelp">
					<?php
						$help = "ON(enabled),OFF(disabled)";
						echo language('Ping help', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				<textarea id="black_ip" name="black_ip" width="100%" rows="5" cols="80" ><?php echo $black_str?></textarea>
				<br>
				<span id="black_tips"></span>
			</td>
		</tr>
	</table>
</div>
	<br>	
	<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
	<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>	
	<input type="hidden" name="send" id="send" value="" />
	<div id="preview_dg" title="Firewall Rules Apply" style="display:none">
		<div>
			<div id="redmsg" style="display:block;width:470px;margin:0 auto;"  contenteditable="false" ></div>
			<br>
			<div id="timemsg" style="display:block;width:470px;margin:0 auto" contenteditable = "false"></div>
		</div>
	</div>
	<div id="float_btn" class="float_btn">
		<div id="float_btn_tr" class="float_btn_tr">
				<input type="submit"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" <?php if($__demo_enable__=='on'){echo 'disabled';}?> />
				<input type="button" id="preview_btn" value="<?php echo language('Submit');?>" onclick="preview_dialog();" <?php if($__demo_enable__=='on'){echo 'disabled';}?> />
		</div>
	</div>

	</form>


	
<script type="text/javascript">
function onload_func()
{
<?php
	if($firewall_sw != '') {
		echo "firewall_change(true);\n";
	} else {
		echo "firewall_change(false);\n";
	}
	if($ping_sw != '') {
		echo "ping_change(true);\n";
	} else {
		echo "ping_change(false);\n";
	}
	if($white_sw != '') {
		echo "white_change(true);\n";
	} else {
		echo "white_change(false);\n";
	}
	if($black_sw != '') {
		echo "black_change(true);\n";
	} else {
		echo "black_change(false);\n";
	}
	
	

?>

}


$(document).ready(function (){ 
	$("#firewall_sw").iButton();
	$("#ping_sw").iButton();
	$("#white_sw").iButton();
	$("#black_sw").iButton();
	onload_func();
}); 
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>
<div id="float_btn1" class="sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()">
</div>
