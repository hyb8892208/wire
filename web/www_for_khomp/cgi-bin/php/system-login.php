<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/check.inc");
include_once("/www/cgi-bin/inc/define.inc");
include_once("/www/cgi-bin/inc/userdb.php");
?>

<?php

$DENY_PORT_LIST="5038,5039,5040,5041,5042,5060,12345,12346,12347,12348,12349,8000,56888";

session_start();
$db = new Users();
$all_res = $db->get_all_user_info();
$js_arr = '[';

while($all_info = $all_res->fetchArray()){
	if($_SESSION['username'] == $all_info['username']) continue;
	
	$js_arr .= '"'.$all_info['username'].'",';
}
$js_arr = rtrim($js_arr,",");
$js_arr .= ']';
?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>

<script type="text/javascript">
function check()
{
	var is_check = false;
	
	var login_mode = document.getElementById("login_mode").value;
	var user = document.getElementById("user").value;
	var pw1 = document.getElementById("pw1").value;
	var pw2 = document.getElementById("pw2").value;
	var all_username = <?php echo $js_arr; ?>;
	var web_server_port = parseInt(document.getElementById("web_server_port").value);

	document.getElementById("cuser").innerHTML = '';
	document.getElementById("cpw1").innerHTML = '';
	document.getElementById("cpw2").innerHTML = '';
	document.getElementById("cweb_server_port").innerHTML = '';

	if(user != "" || pw1 != "" || pw2 != "") {
		for(var i=0;i<all_username.length;i++){
			if(all_username[i] == user){
				$("#user").focus();
				$("#cuser").html(con_str("<?php echo language("User name has been used");?>"));
				is_check = true;
			}
		}
		
		if(!check_diyname(user)) {
			document.getElementById('user').focus();
			document.getElementById("cuser").innerHTML = con_str('<?php echo htmlentities(language('js check diyname','Allowed character must be any of [-_+.<>&0-9a-zA-Z],1 - 32 characters.'));?>');
			is_check = true;
		}

		if(pw1.length < 8){
			document.getElementById('cpw1').focus();
			document.getElementById("cpw1").innerHTML = con_str("<?php echo language("Password len tip", "The password cannot be less than 8 characters.");?>");
			is_check = true;
		}
		var rex=/^(?=.*?[A-Za-z]+)(?=.*?[0-9]+)(?=.*?[A-Z]).*$/;
		if(!rex.test(pw1)) {
			document.getElementById('cpw1').focus();
			document.getElementById("cpw1").innerHTML = con_str('<?php echo htmlentities(language('Password Login Rule', 'The password must meet the following rules:1.At least eight characters.2.At least one number.3.At least one lowercase letter.4.At least one uppercase letter.'));?>');
			is_check = true;
		}

		if(pw1 !== pw2) {
			document.getElementById('cpw2').focus();
			document.getElementById("cpw2").innerHTML = con_str('<?php echo language('Confirm Password warning web','This password must match the password above.');?>');
			is_check = true;
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
			document.getElementById('web_server_port').focus();
			document.getElementById("cweb_server_port").innerHTML = con_str('<?php echo language('js webserver port help','Range: 1024-65535, Default 80. Some ports are forbidding.'); ?>');
			is_check = true;
		}
	}
	
	if(is_check){
		return false;
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
		$db = new Users();
		session_start();
		
		$old_username = $_SESSION['username'];
		$id = $_SESSION['id'];
		
		$username = trim($_POST['user']);
		$password = trim($_POST['pw1']);
		
		$real_password = md5($password.'-'.$username);
		
		if(!check_username_repeat($db,$old_username,$username)){
			echo language("User name has been used");
			return false;
		}
		
		$db->update_username_and_password($id,$username,$real_password);
		
		save_user_record($db,"SYSTEM->Login Settings:Update User ".$old_username.' to '.$username);
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
	if($only_view){
		return false;
	}
	
	if(save2webserver()) {
		wait_apply("exec", "php -r \"include_once('/www/cgi-bin/inc/wrcfg.inc');save_webserver_to_lighttpd();\" > /dev/null 2>&1");
		wait_apply("exec", "/etc/init.d/lighttpd restart > /dev/null 2>&1 &");
		wait_apply("exec", "/etc/init.d/cloud restart > /dev/null 2>&1 &");
		
		session_start();
		unset($_SESSION['show']);
	}
	save_user_record("","SYSTEM->Login Settings:Save");
}

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

	<div class="content">
		<span class="title">
			<?php echo language('Web Login Settings');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('User Name')){ ?>
							<b><?php echo language('User Name','User Name');?>:</b><br/>
							<?php
								$help = "NOTES: Your gateway doesn't have administration role. <br/>
							All you can do here is defining the username and password to manage your gateway.<br/>
							And it has all privileges to operate your gateway.<br/>
							User Name: Allowed characters \"-_+.<>&0-9a-zA-Z\".Length: 1-32 characters.";
								echo language('User Name help', $help);
							?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Password')){ ?>
							<b><?php echo language('Password','Password');?>:</b><br/>
							<?php echo htmlentities(language('Password help',"Allowed character must be any of [0-9a-zA-Z`~!@$%^&*()_+{}|<>?-=[],./],4 - 32 characters.")); ?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Confirm Password')){ ?>
							<b><?php echo language('Confirm Password','Confirm Password');?>:</b><br/>
							<?php echo language('Confirm Password help',"Please input the same password as 'Password' above.");?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Login Mode')){ ?>
							<b><?php echo language('Login Mode');?>:</b><br/>
							<?php echo language('Login Mode help',"Select the mode of login.");?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Port')){ ?>
							<b><?php echo language('Port');?>:</b><br/>
							<?php echo language('Port help',"Specify the web server port number.");?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('User Name','User Name');?>:
			</span>
			<div class="tab_item_right">
				<span id="cuser"></span>
				<input id="user" type="text" name="user" value="" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Password','Password');?>:
			</span>
			<div class="tab_item_right">
				<span id="cpw1"></span>
				<input id="pw1" type="password" name="pw1" value="" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Confirm Password','Confirm Password');?>:
			</span>
			<div class="tab_item_right">
				<span id="cpw2"></span>
				<input id="pw2" type="password" name="pw2" value="" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Login Mode');?>:
			</span>
			<div class="tab_item_right">
				<select id="login_mode" name="login_mode" onchange="login_mode_change();">
					<option value="http_https" <?php if ($login_mode == "http_https"){echo "selected";} ?>>http and https</option>
					<option value="https" <?php if ($login_mode == "https"){echo "selected";} ?>>only https</option>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Port','Port');?>:
			</span>
			<div class="tab_item_right">
				<span id="cweb_server_port"></span>
				<input id="web_server_port" type="text" maxlength=5 name="web_server_port" value="<?php echo $web_server_port ?>" 
					oninput="this.value=this.value.replace(/[^\d]*/g,'')" onkeyup="this.value=this.value.replace(/[^\d]*/g,'')"/>
			</div>
		</div>
	</div>
	
	<br>

	<input type="hidden" name="send" id="send" value="" />
	
	<div id="button_save">
	
		<?php if(!$only_view){ ?>
		<button type="submit" onclick="document.getElementById('send').value='Save';return check();" <?php if($__demo_enable__=='on'){echo 'disabled';}?> ><?php echo language('Save');?></button>
		<?php } ?>
		
	</div>
</form>

<script type="text/javascript">

function login_mode_change()
{
	if (document.getElementById("login_mode").value == "https") {
		document.getElementById("web_server_port").value='443';
		document.getElementById("web_server_port").disabled=true;
		//document.getElementById("web_server_port").style.backgroundColor="#AAAAAA";
	} else {
		document.getElementById("web_server_port").value='<?php echo $web_server_port;?>';
		document.getElementById("web_server_port").disabled=false;
		document.getElementById("web_server_port").style.backgroundColor="";
	}
}

$(document).ready(function (){ 
	login_mode_change();
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
