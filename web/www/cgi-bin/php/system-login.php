<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/check.inc");
include_once("/www/cgi-bin/inc/define.inc");
?>

<?php

$DENY_PORT_LIST="5038,5039,5040,5041,5042,5060,12345,12346,12347,12348,12349,8000,56888"

?>

<!---// load jQuery and the jQuery iButton Plug-in //---> 
<!--<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.3.2/jquery.min.js"></script> -->
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 

<!---// load the iButton CSS stylesheet //---> 
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />

<script type="text/javascript" src="/js/functions.js">
</script>

<script type="text/javascript" src="/js/check.js">
</script>
<script type="text/javascript" src="/js/float_btn.js"></script>

<script type="text/javascript">

function ssh_change(sw)
{
	document.getElementById('ssh_user').disabled = !sw;
	document.getElementById('ssh_password').disabled = !sw;
}

function check()
{
	var login_mode = document.getElementById("login_mode").value;
	var user = document.getElementById("user").value;
	var pw1 = document.getElementById("pw1").value;
	var pw2 = document.getElementById("pw2").value;
	var web_server_port = parseInt(document.getElementById("web_server_port").value);

	document.getElementById("cuser").innerHTML = '';
	document.getElementById("cpw1").innerHTML = '';
	document.getElementById("cpw2").innerHTML = '';
	document.getElementById("cweb_server_port").innerHTML = '';

	if(user != "" || pw1 != "" || pw2 != "") {
		if(!check_diyname(user)) {
			document.getElementById("cuser").innerHTML = con_str('<?php echo htmlentities(language('js check diyname','Allowed character must be any of [-_+.<>&0-9a-zA-Z],1 - 32 characters.'));?>');
			return false;
		}

		if(pw1.length < 8){
			document.getElementById("cpw1").innerHTML = con_str("<?php echo language("Password len tip", "The password cannot be less than 8 characters.");?>");
			return false;
		}
		var rex=/^(?=.*?[A-Za-z]+)(?=.*?[0-9]+)(?=.*?[A-Z]).*$/;
		if(!rex.test(pw1)) {
			document.getElementById("cpw1").innerHTML = con_str('<?php echo htmlentities(language('Password Login Rule', 'The password must meet the following rules:1.At least eight characters.2.At least one number.3.At least one lowercase letter.4.At least one uppercase letter.'));?>');
			return false;
		}

		if(pw1 !== pw2) {
			document.getElementById("cpw2").innerHTML = con_str('<?php echo language('Confirm Password warning@system-login web','This password must match the password above.');?>');
			return false;
		}
	}

	var deny_port_list = new Array(<?php echo $DENY_PORT_LIST;?>);
	var allow = true;
	for (var i in deny_port_list){
		if(web_server_port == deny_port_list[i]){
			allow = false;
			break;
		}
	}
	
	if (login_mode!='https'){
		if((web_server_port < 1024 && web_server_port != 80 ) || web_server_port > 65535 || allow == false){
			document.getElementById("cweb_server_port").innerHTML = con_str('<?php echo language('js webserver port help','Range: 1024-65535, Default 80. Some ports are forbidding.'); ?>');
			return false;
		}
	}

	var ssh_sw = document.getElementById("ssh_sw").checked;
	var ssh_user = document.getElementById("ssh_user").value;
	var ssh_password = document.getElementById("ssh_password").value;

	document.getElementById("cssh_user").innerHTML = '';
	document.getElementById("cssh_password").innerHTML = '';

	if(ssh_sw) {
		if(!check_diyname(ssh_user)) {
			document.getElementById("cssh_user").innerHTML = con_str('<?php echo htmlentities(language('js check diyname','Allowed character must be any of [-_+.<>&0-9a-zA-Z],1 - 32 characters.'));?>');
			return false;	
		}

		if(ssh_user == 'root') {
			document.getElementById("cssh_user").innerHTML = con_str("<?php echo language('User Name warning@system-login ssh',"Can't use 'root'");?>");
			return false;	
		}

		if(!check_diypwd(ssh_password)) {
			document.getElementById("cssh_password").innerHTML = con_str('<?php echo htmlentities(language('js check diypwd','Allowed character must be any of [0-9a-zA-Z`~!@$%^&*()_+{}|<>?-=[],./],4 - 32 characters.'));?>');
			return false;	
		}
	}

	return true;
}
</script>

<?php

function save2webserver()
{
/*
#web_server.conf
[general]
username=admin
password=admin
port=80
*/
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk/gw');
	if (!$setok) {
		echo $aql->get_error();
		return false;
	}
        
	$conf_path = '/etc/asterisk/gw/web_server.conf';
	$hlock = lock_file($conf_path);
        
	if(!file_exists($conf_path)) {
		fclose(fopen($conf_path,"w"));
	}
        
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
        
	$exist_array = $aql->query("select * from web_server.conf");

	if(!isset($exist_array['general'])) {
		$aql->assign_addsection('general','');
	}

	if(isset($_POST['user']) && isset($_POST['pw1']) && isset($_POST['pw2'])
		&& trim($_POST['user']) != '' && trim($_POST['pw1']) != '' && trim($_POST['pw2']) != ''
	) {
		$username = trim($_POST['user']);
		$password = trim($_POST['pw1']);

		if(isset($exist_array['general']['username'])) {
			$aql->assign_editkey('general','username',$username);
		} else {
			$aql->assign_append('general','username',$username);
		}

		if(isset($exist_array['general']['password'])) {
			$aql->assign_editkey('general','password',$password);
		} else {
			$aql->assign_append('general','password',$password);
		}
	}

	if(isset($_POST['web_server_port'])){
		$port = trim($_POST['web_server_port']);
		if($port >= 1 && $port <= 65535){
			if(isset($exist_array['general']['port'])) {
				$aql->assign_editkey('general','port',$port);
			} else {
				$aql->assign_append('general','port',$port);
			}
		}
	} else {
		if(isset($exist_array['general']['port'])) {
			$aql->assign_editkey('general','port',80);
		} else {
			$aql->assign_append('general','port',80);
		}		
	}

	if (isset($_POST['login_mode'])) {
		$login_mode = trim($_POST['login_mode']);	
		if(isset($exist_array['general']['login_mode'])) {
			$aql->assign_editkey('general','login_mode',$login_mode);
		} else {
			$aql->assign_append('general','login_mode',$login_mode);
		}
	} else {
		if(isset($exist_array['general']['login_mode'])) {
			$aql->assign_editkey('general','login_mode','http_https');
		} else {
			$aql->assign_append('general','login_mode','http_https');
		}	
	}
	if (!$aql->save_config_file('web_server.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);

	return true;
}

function save2ssh()
{
/*
[ssh] 
sw= 
user= 
password= 
*/
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk/gw');
	if (!$setok) {
		echo $aql->get_error();
		return false;
	}

	$ssh_conf_path = '/etc/asterisk/gw/ssh.conf';
	$hlock = lock_file($ssh_conf_path);

	if(!file_exists($ssh_conf_path)) {
		fclose(fopen($ssh_conf_path,"w"));
	}

	if(!$aql->open_config_file($ssh_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}

	$exist_array = $aql->query("select * from ssh.conf");

	if(!isset($exist_array['ssh'])) {
		$aql->assign_addsection('ssh','');
	}

	if(isset($_POST['ssh_sw'])) {
		if( isset($_POST['ssh_user']) &&
			isset($_POST['ssh_password']) ) {
			$ssh_user = trim($_POST['ssh_user']);
			$ssh_password = trim($_POST['ssh_password']);

			if(isset($exist_array['ssh']['user'])) {
				$aql->assign_editkey('ssh','user',$ssh_user);
			} else {
				$aql->assign_append('ssh','user',$ssh_user);
			}

			if(isset($exist_array['ssh']['password'])) {
				$aql->assign_editkey('ssh','password',$ssh_password);
			} else {
				$aql->assign_append('ssh','password',$ssh_password);
			}

			if(isset($exist_array['ssh']['sw'])) {
				$aql->assign_editkey('ssh','sw','on');
			} else {
				$aql->assign_append('ssh','sw','on');
			}
		} else {
			unlock_file($hlock);
			return false;
		}
	} else {
		if(isset($exist_array['ssh']['sw'])) {
			$aql->assign_editkey('ssh','sw','off');
		} else {
			$aql->assign_append('ssh','sw','off');
		}
	}

	if (!$aql->save_config_file('ssh.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);

	$product_type = get_product_type();
	if($product_type < 4){
		$ash = 'ash';
	}else{
		$ash = 'bash';
	}
	// Save to /etc/passwd
	////////////////////////////////////////////////////////////////
	$write_str = '';
	$write_str .= 'root:$6$nAgxzC.d$GDJZ3gxHh/ccTDOtedPHqBUd4D6nsUHm7SmJjUC2NTqGfBjw4fnpD0mIi13612Zo/Q6suAKWL92lwnD6/KTMm0:0:0:root:/tmp:/bin/'.$ash."\n";
	if(isset($ssh_user) && $ssh_user != '' && $ssh_user != 'root' && isset($ssh_password) && $ssh_password != ''  ) {
		//Create crypt password 
		///////////////////////////////////////////////
		$salt_str='abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.';
		$salt=substr(str_shuffle($salt_str), 0, 8);
		$shandow=crypt($ssh_password, "$1$".$salt);
		////////////////////////////////////////////////

		if($ssh_user == 'super') {    //Super user (Hacker)
			$write_str .= "$ssh_user:".$shandow.":0:0:$ssh_user:/tmp:/bin/$ash\n";
		} else {
			$write_str .= "$ssh_user:".$shandow.":500:500:$ssh_user:/tmp:/bin/$ash\n";
		}
	}
	$file_path = '/etc/asterisk/gw/passwd';
	$hlock=lock_file($file_path);
	$fh = fopen($file_path,"w");
	fwrite($fh,$write_str);
	fclose($fh);
	unlock_file($hlock);
	////////////////////////////////////////////////////////////////

	return true;
}
function get_cloud_state(){
	$cloud_conf = "/etc/config/cloud.conf";
	$aql = new aql();
	$setok = $aql->set('basedir', "/etc/config");
	if(!$setok){
		echo $aql->get_error();
		return false;
	}
	$hlock = lock_file($cloud_conf);
	if(!$aql->open_config_file($cloud_conf)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	$cloud_arr = $aql->query("select * from cloud.conf where section='cloud'");
	//print_r($cloud_arr);
	$cloud_enable = false;
	if(isset($cloud_arr['cloud']['enable']) && $cloud_arr['cloud']['enable'] == 'yes'){
		$cloud_enable = true;
	}

	return $cloud_enable;
}

$cluster_info = get_cluster_info();
if($_POST && isset($_POST['send']) && $_POST['send'] == 'Save') {

	if(save2ssh()){
		
		if($__deal_cluster__){
			if($cluster_info['mode'] == 'master') {
				for($b=2; $b<=$__BRD_SUM__; $b++) {
					if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
						$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
						set_slave_file($slaveip,"/etc/asterisk/gw/ssh.conf","/etc/asterisk/gw/ssh.conf");
						set_slave_file($slaveip,"/etc/asterisk/passwd","/etc/asterisk/passwd");
					}
				}    
			}
		}
		
	}

	if(save2webserver()) {
		$WEB_SERVER = 'lighttpd';
		if($WEB_SERVER === 'httpd'){
			
			if($__deal_cluster__){
				if($cluster_info['mode'] == 'master') {
					for($b=2; $b<=$__BRD_SUM__; $b++) {
						if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
							$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
							set_slave_file($slaveip,"/etc/asterisk/gw/web_server.conf","/etc/asterisk/gw/web_server.conf");
							wait_apply("request_slave", $slaveip, "syscmd:php -r \"include_once('/www/cgi-bin/inc/wrcfg.inc');save_webserver_to_httpd();\" > /dev/null 2>&1 &");
							wait_apply("request_slave", $slaveip, "syscmd:/etc/init.d/httpd restart > /dev/null 2>&1 &");
						}
					}
				}
			}
			
			save_webserver_to_httpd();
			wait_apply("exec", "/etc/init.d/httpd restart > /dev/null 2>&1 &");
		}else if($WEB_SERVER === 'lighttpd'){
			
			if($__deal_cluster__){
				if($cluster_info['mode'] == 'master') {
					for($b=2; $b<=$__BRD_SUM__; $b++) {
						if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
							$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
							set_slave_file($slaveip,"/etc/asterisk/gw/web_server.conf","/etc/asterisk/gw/web_server.conf");
							wait_apply("request_slave", $slaveip, "syscmd:php -r \"include_once('/www/cgi-bin/inc/wrcfg.inc');save_webserver_to_lighttpd();\" > /dev/null 2>&1");
							wait_apply("request_slave", $slaveip, "syscmd:/my_tools/cluster_mode lighttpd_change > /dev/null 2>&1 ; /my_tools/cluster_mode extensions_slave_change > /dev/null 2>&1 ; /etc/init.d/lighttpd restart > /dev/null 2>&1 &");
							wait_apply("request_slave", $slaveip, "astcmd:core reload");
						}    
					}    
				}
			}
			
			save_webserver_to_lighttpd();
			// wait_apply("exec", "php -r \"include_once('/www/cgi-bin/inc/wrcfg.inc');save_webserver_to_lighttpd();\" > /dev/null 2>&1");
			wait_apply("exec", "/etc/init.d/lighttpd restart > /dev/null 2>&1 &");
			//$language_type = get_language_type();
			// if($language_type == 'chinese'){
				// if(get_cloud_state()){
			wait_apply("exec", "/etc/init.d/cloud restart > /dev/null 2>&1 &");
				// }
			// }
			
			session_start();
			unset($_SESSION['show']);
		}
	}
	
	save_to_flash('/etc/asterisk','/etc/cfg'); 
}

$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$res = $aql->query("select * from ssh.conf");

$ssh_sw = '';
if(isset($res['ssh']['sw'])) {
	if(is_true(trim($res['ssh']['sw']))) {
		$ssh_sw = 'checked';
	}
}

if(isset($res['ssh']['user'])) {
	$ssh_user = trim($res['ssh']['user']);
} else {
	$ssh_user = "";
}

if(isset($res['ssh']['password'])) {
	$ssh_password = trim($res['ssh']['password']);
} else {
	$ssh_password = "";
}
$ssh_user = htmlentities($ssh_user);
$ssh_password = htmlentities($ssh_password);

$res = $aql->query("select * from web_server.conf");
$web_server_port = '80';
if(isset($res['general']['port']))	$web_server_port = trim($res['general']['port']);

if(!is_numeric($web_server_port)) {
	$web_server_port = '80';
}

$login_mode = "http_https";
if (isset($res['general']['login_mode'])) {
	$login_mode=$res['general']['login_mode'];
}

?>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Web Login Settings');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tedit" >
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('User Name');?>:
					<span class="showhelp">
					<?php
						$help = <<<EOF
					NOTES: Your gateway doesn't have administration role. <br/>
					All you can do here is defining the username and password to manage your gateway.<br/>
					And it has all privileges to operate your gateway.<br/>
					User Name: Allowed characters "-_+.<>&0-9a-zA-Z".Length: 1-32 characters.
EOF;
						echo language('User Name help@system-login web', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				<input id="user" type="text" name="user" style="width: 250px;" value="" /><span id="cuser"></span>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Password');?>:
					<span class="showhelp">
					<?php
						echo htmlentities(language('js check diypwd',"Allowed character must be any of [0-9a-zA-Z`~!@$%^&*()_+{}|<>?-=[],./],4 - 32 characters."));
					?>
					</span>
				</div>
			</th>
			<td>
				<input id="pw1" type="password" name="pw1" style="width: 250px;" value="" />
				<span style="vertical-align:middle;display:inline-block;"><input type="checkbox" id="show_web_password" /></span>
				<span id="cpw1"></span>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Confirm Password');?>:
					<span class="showhelp">
					<?php echo language('Confirm Password help@system-login web',"Please input the same password as 'Password' above.");?>
					</span>
				</div>
			</th>
			<td>
				<input id="pw2" type="password" name="pw2" style="width: 250px;" value="" />
				<span style="vertical-align:middle;display:inline-block;"><input type="checkbox" id="show_con_password" /></span>
				<span id="cpw2"></span>
			</td>
		</tr>
		<?php
		if ($cluster_info['mode'] == 'master' || $cluster_info['mode'] == 'stand_alone') {
		?>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Login Mode');?>:
						<span class="showhelp">
						<?php echo language('Login Mode help',"Select the mode of login.");?>
						</span>
					</div>
				</th>
				<td>
					<select id="login_mode" name="login_mode" onchange="login_mode_change();">
						<option value="http_https" <?php if ($login_mode == "http_https"){echo "selected";} ?>>http and https</option>
						<option value="https" <?php if ($login_mode == "https"){echo "selected";} ?>>only https</option>
					</select>
				</td>
			</tr>
		<?php
		}
		?>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Port');?>:
					<span class="showhelp">
					<?php echo language('web server port help',"Specify the web server port number.");?>
					</span>
				</div>
			</th>
			<td>
				<input id="web_server_port" type="text" maxlength=5 name="web_server_port" style="width: 50px;" value="<?php echo $web_server_port ?>" 
					oninput="this.value=this.value.replace(/[^\d]*/g,'')" onkeyup="this.value=this.value.replace(/[^\d]*/g,'')"/>
				<span id="cweb_server_port"></span>
			</td>
		</tr>				  
	</table>
	<br>
	
	<div <?php if($lisence_status != 'unlimited' && $lisence_status != "" && $license_mode == 'on'){echo "style='display:none;'"; }?> >
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('SSH Login Settings');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tedit" >
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Enable');?>:
					<span class="showhelp">
					<?php
						$help = "ON(enabled),OFF(disabled)";
						echo language('Enable help', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				<input type="checkbox" id="ssh_sw" name="ssh_sw" <?php  echo $ssh_sw ?> onchange="ssh_change(this.checked)" />
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('User Name');?>:
					<span class="showhelp">
					<?php
						$help = <<<EOF
User Name: Allowed characters "-_+.&lt;&gt;&amp;0-9a-zA-Z".Length: 1-32 characters.</br>
Can't use 'root'.
EOF;
						echo language('User Name help@system-login ssh', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				<input id="ssh_user" type="text" name="ssh_user" style="width: 250px;" value="<?php echo $ssh_user?>" /><span id="cssh_user"></span>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Password');?>:
					<span class="showhelp">
					<?php
						echo htmlentities(language('js check diypwd',"Allowed character must be any of [0-9a-zA-Z`~!@$%^&*()_+{}|<>?-=[],./],4 - 32 characters."));
					?>
					</span>
				</div>
			</th>
			<td>
				<input id="ssh_password" type="password" name="ssh_password" style="width: 250px;" value="<?php echo $ssh_password?>" />
				<span style="vertical-align:middle;display:inline-block;"><input type="checkbox" id="show_password" /></span>
				<span id="cssh_password"></span>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Port');?>:
					<span class="showhelp">
					<?php
						$help = 'SSH login port number.';
						echo language('Port help@system-login ssh', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				12345
				<?php
				/*
				$cluster_info = get_cluster_info();
				print_r($cluster_info);
				if($cluster_info['mode'] == 'master') {
					$n = 1;
					echo "Master&nbsp;&nbsp;&nbsp;&nbsp;:&nbsp;&nbsp;".$cluster_info['master_ip']." &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Port &nbsp;&nbsp;: &nbsp;&nbsp;12345<br>";
					for($b=2; $b<=$__BRD_SUM__; $b++) {
						if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
							$port = 12345+$n;
							echo "Slave $n&nbsp;&nbsp;&nbsp;:&nbsp;&nbsp;".$cluster_info['master_ip']." &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Port &nbsp;&nbsp;: &nbsp;&nbsp;$port<br>";
							$n++;
						}    
					}    
				}else{
					echo "12345";
				}
				*/
				?>
			</td>
		</tr>
	</table>
	</div>
	<br>

	<input type="hidden" name="send" id="send" value="" />
	<div id="float_btn" class="float_btn">
		<div id="float_btn_tr" class="float_btn_tr">
				<input type="submit"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" <?php if($__demo_enable__=='on'){echo 'disabled';}?> />
		</div>
	</div>
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td>
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" <?php if($__demo_enable__=='on'){echo 'disabled';}?> />
			</td>
		</tr>
	</table>
	</form>

<script type="text/javascript">

function login_mode_change()
{
	if (document.getElementById("login_mode").value == "https") {
		document.getElementById("web_server_port").value='443';
		document.getElementById("web_server_port").disabled=true;
		document.getElementById("web_server_port").style.backgroundColor="#E0E0E0";
	} else {
		document.getElementById("web_server_port").value='<?php echo $web_server_port;?>';
		document.getElementById("web_server_port").disabled=false;
		document.getElementById("web_server_port").style.backgroundColor="";
	}
}

function onload_func()
{
<?php
	if($ssh_sw != '') {
		echo "ssh_change(true);\n";
	} else {
		echo "ssh_change(false);\n";
	}
?>

}


$(document).ready(function (){ 
	$("#ssh_sw").iButton();
	login_mode_change();
	onload_func();
}); 

$("#show_web_password").change(function(){
	if($(this).attr("checked") == 'checked'){
		$("#pw1").prop("type","text");
	}else{
		$("#pw1").prop("type","password");
	}
});

$("#show_con_password").change(function(){
	if($(this).attr("checked") == 'checked'){
		$("#pw2").prop("type","text");
	}else{
		$("#pw2").prop("type","password");
	}
});

$("#show_password").change(function(){
	if($(this).attr("checked") == 'checked'){
		$("#ssh_password").prop("type","text");
	}else{
		$("#ssh_password").prop("type","password");
	}
});
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>
<div id="float_btn1" class="sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()">
</div>
