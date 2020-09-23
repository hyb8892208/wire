<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
require_once("/www/cgi-bin/inc/aql.php");
?>

<?php

function set_ddns_to_conf($type, $host, $userid, $userpwd)
{
	$method = $_POST['method'];
	
	if ($type == "inadyn") {
		$write = <<<EOF
[settings]
dyndns_system = $method
update_period_sec = 30
verbose = 0
log_file = /var/log/ddnslog
background = 1
alias = $host
username = $userid
password = $userpwd

EOF;
//EOF
		$cfg_file = "/etc/asterisk/gw/inadyn.conf"; 
		$hlock = lock_file($cfg_file);
		$fh=fopen($cfg_file,"w");
		fwrite($fh,$write);
		fclose($fh);
		unlock_file($hlock);
	} else {

		$write = <<<EOF
[settings]
nicName = eth0
szLog = /var/log/ddnslog
szHost = $host
szUserID = $userid
szUserPWD = $userpwd

EOF;
//EOF
		$cfg_file = "/etc/asterisk/gw/phlinux.conf"; 
		$hlock = lock_file($cfg_file);
		$fh=fopen($cfg_file,"w");
		fwrite($fh,$write);
		fclose($fh);
		unlock_file($hlock);
	}

	return true;
}

function save_ddns_operate_server()
{
//default ddns.conf
/*
[general]
ddns=0
type=inadyn

[inadyn]
host=www.internet.site.com
userid=admin
userpwd=admin

[phddns]
host=PhLinux3.Oray.Net
userid=admin
userpwd=admin
*/

	$type = '';
	$host = '';
	$userid = '';
	$userpwd = '';

	if (isset($_POST['ddns'])) {
		$sw = '1';
		if (!isset($_POST['type'])    ||
			!isset($_POST['host'])    ||
			!isset($_POST['userid'])  ||
			!isset($_POST['userpwd'])) {
			return false;
		}
		$type = trim($_POST['type']);
		$host = trim($_POST['host']);
		$userid = trim($_POST['userid']);
		$userpwd = trim($_POST['userpwd']);

		if ($host == '' || $userid == '' || $userpwd == '') {
			echo "Must input value!!!";
			return false; 
		}

	} else {
		$sw = '0';
	}

	$conf_path = '/etc/asterisk/gw/ddns.conf';
	if(!file_exists($conf_path)) {
		fclose(fopen($conf_path,"w"));
	}

	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$hlock = lock_file($conf_path);
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}

	$exist_array = $aql->query("select * from ddns.conf");

	if(!isset($exist_array['general'])) {
		$aql->assign_addsection('general','');
	}

	if(isset($exist_array['general']['ddns'])) {
		$aql->assign_editkey('general','ddns', $sw);
	} else {
		$aql->assign_append('general','ddns', $sw);
	}

	if($sw == '1') {
		if(isset($exist_array['general']['type'])) {
			$aql->assign_editkey('general','type', $type);
		} else {
			$aql->assign_append('general','type', $type);
		}

		if(!isset($exist_array[$type])) {
			$aql->assign_addsection($type,'');
		}
		
		if($type == 'inadyn'){
			$method = $_POST['method'];
			
			if(isset($exist_array[$type]['method'])){
				$aql->assign_editkey($type,'method',$method);
			}else{
				$aql->assign_append($type,'method',$method);
			}
		}
		
		if(isset($exist_array[$type]['host'])) {
			$aql->assign_editkey($type,'host', $host);
		} else {
			$aql->assign_append($type,'host', $host);
		}

		if(isset($exist_array[$type]['userid'])) {
			$aql->assign_editkey($type,'userid', $userid);
		} else {
			$aql->assign_append($type,'userid', $userid);
		}

		if(isset($exist_array[$type]['userpwd'])) {
			$aql->assign_editkey($type,'userpwd', $userpwd);
		} else {
			$aql->assign_append($type,'userpwd', $userpwd);
		}
	}

	if (!$aql->save_config_file('ddns.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false; 
	}
	unlock_file($hlock);

	if (set_ddns_to_conf($type, $host, $userid, $userpwd)) {
		//exec("/etc/init.d/ddns restart >/dev/null 2>&1");
		wait_apply("exec", "/etc/init.d/ddns restart >/dev/null 2>&1");
		return true;
	}

	 return false;
}
?>

<?php
if($_POST && isset($_POST['send']) && $_POST['send'] == 'Save') {
	if($only_view){
		return false;
	}
	
	save_ddns_operate_server();
	
	save_user_record("","NETWORK->Network DDNS:Save");
}

//Read data 
$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$res = $aql->query('select * from ddns.conf');

if(isset($res['general']['ddns'])) {
	$ddns = trim($res['general']['ddns']);
} else {
	$ddns = '0';
}

if(isset($res['general']['type'])) {
	$type = trim($res['general']['type']);
} else {
	$type = 'inadyn';
}

if(isset($res['inadyn']['method'])){
	$method = $res['inadyn']['method'];
} else {
	$method = 'dyndns@dyndns.org';
}

if(isset($res['inadyn']['host'])) {
	$host['inadyn'] = trim($res['inadyn']['host']);
} else {
	$host['inadyn'] = '';
}

if(isset($res['inadyn']['userid'])) {
	$userid['inadyn'] = trim($res['inadyn']['userid']);
} else {
	$userid['inadyn'] = '';
}

if(isset($res['inadyn']['userpwd'])) {
	$userpwd['inadyn'] = trim($res['inadyn']['userpwd']);
} else {
	$userpwd['inadyn'] = '';
}

if(isset($res['phddns']['host'])) {
	$host['phddns'] = trim($res['phddns']['host']);
} else {
	$host['phddns'] = '';
}

if(isset($res['phddns']['userid'])) {
	$userid['phddns'] = trim($res['phddns']['userid']);
} else {
	$userid['phddns'] = '';
}

if(isset($res['phddns']['userpwd'])) {
	$userpwd['phddns'] = trim($res['phddns']['userpwd']);
} else {
	$userpwd['phddns'] = '';
}

?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript">
<!--
function onload_func()
{
	ddnschange();
	typechange();
}

function typechange()
{
	var type = document.getElementById('type').value;
	if (type == 'inadyn') {
		document.getElementById('host').value = "<?php echo $host['inadyn']; ?>";
		document.getElementById('userid').value = "<?php echo $userid['inadyn']; ?>";
		document.getElementById('userpwd').value = "<?php echo $userpwd['inadyn']; ?>";
		$("#method_show").show();
	} else { /* phddns */
		document.getElementById('host').value = "<?php echo $host['phddns']; ?>";
		document.getElementById('userid').value = "<?php echo $userid['phddns']; ?>";
		document.getElementById('userpwd').value = "<?php echo $userpwd['phddns']; ?>";
		$("#method_show").hide();
	}
}

function ddnschange()
{
	var ddns_checked = document.getElementById('ddns').checked;
	if (ddns_checked) {
		document.getElementById('type').disabled = false;
		document.getElementById('host').disabled = false;
		document.getElementById('userid').disabled = false;
		document.getElementById('userpwd').disabled = false;
	} else {
		document.getElementById('type').disabled = true;
		document.getElementById('host').disabled = true;
		document.getElementById('userid').disabled = true;
		document.getElementById('userpwd').disabled = true;
	}
}

function trim(str)
{
	return str.replace(/(^\s*)|(\s*$)/g, "");
}

function check()
{
	var ddns_checked = document.getElementById('ddns').checked;
	var host = document.getElementById('host').value;
	var userid = document.getElementById('userid').value;
	var userpwd = document.getElementById('userpwd').value;
	var res = true;

	if (!ddns_checked) {
		return res;
	}

	host = trim(host);
	userid = trim(userid);
	userpwd = trim(userpwd);

	if (host.length == 0) {
		document.getElementById('chost').innerHTML = con_str('Please input a domain');
		document.getElementById('host').value = '';
		res = false;
	} else {
		document.getElementById('chost').innerHTML = '';
	}
	if (userid.length == 0) {
		document.getElementById('cuserid').innerHTML = con_str('Please input a user name');
		document.getElementById('userid').value = '';
		res = false;
	} else {
		document.getElementById('cuserid').innerHTML = '';
	}
	if (userpwd.length == 0) {
		document.getElementById('cuserpwd').innerHTML = con_str('Please input a user password');
		document.getElementById('userpwd').value = '';
		res = false;
	} else {
		document.getElementById('cuserpwd').innerHTML = '';
	}

	return res;
}
-->
</script>

<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	
	<div class="content">
		<span class="title">
			<?php echo language('DDNS Settings');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('DDNS')){ ?>
							<b><?php echo language('DDNS');?>:</b><br/>
							<?php echo language('DDNS help','Enable/Disable DDNS (dynamic domain name server). <br/>');?>
							
							<br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Type')){ ?>
							<b><?php echo language('Type');?>:</b><br/>
							<?php echo language('Type help','Set the type of DDNS server. <br/>');?>
							
							<br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Method')){ ?>
							<b><?php echo language('Method');?>:</b><br/>
							<?php echo language('Method help','Method');?>
							
							<br/>
						<?php } ?>
						
						<?php if(is_show_language_help('User Name')){ ?>
							<b><?php echo language('User Name');?>:</b><br/>
							<?php echo language('User Name help',"Your DDNS account's login name.<br/>");?>
							
							<br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Password')){ ?>
							<b><?php echo language('Password');?>:</b><br/>
							<?php echo language('Password help',"Your DDNS account's password.<br/>");?>
							
							<br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Your domain')){ ?>
							<b><?php echo language('Your domain');?>:</b><br/>
							<?php echo language('Your domain help','The domain to which your web server will belong.<br/>');?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('DDNS');?>:
			</span>
			<div class="tab_item_right">
				<span id="cddns"></span>
				<span><input type="checkbox" name="ddns" id="ddns" onchange="ddnschange()" <?php if (!strcmp($ddns,"1"))echo "checked"; ?> /></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Type');?>:
			</span>
			<div class="tab_item_right">
				<span id="ctype"></span>
				<select name="type" id="type" onchange="typechange()">
					<option value="inadyn" <?php  if (!strcmp($type, "inadyn")) echo "selected" ?>><?php echo language('inadyn');?></option>
					<option value="phddns" <?php  if (!strcmp($type, "phddns")) echo "selected" ?>><?php echo language('phddns');?></option>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="method_show" style="display:none;">
			<span>
				<?php echo language('Method'); ?>:
			</span>
			<div class="tab_item_right">
				<span id="cmethod"></span>
				<select name="method" id="method" >
					<option value="dyndns@dyndns.org" <?php if($method == 'dyndns@dyndns.org') echo "selected";?>>Dyndns.org</option>
					<option value="default@no-ip.com" <?php if($method == 'default@no-ip.com') echo "selected";?>>www.no-ip.com</option>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('User Name');?>:
			</span>
			<div class="tab_item_right">
				<span id="cuserid"></span>
				<input type=text name="userid" id="userid" value="<?php echo $userid[$type] ?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Password');?>:
			</span>
			<div class="tab_item_right">
				<span id="cuserpwd"></span>
				<input type="password" name="userpwd" id="userpwd" value="<?php echo $userpwd[$type] ?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Your domain');?>:
			</span>
			<div class="tab_item_right">
				<span id="chost"></span>
				<input type="text" name="host" id="host" value="<?php echo $host[$type] ?>" />
			</div>
		</div>
	</div>

	<input type="hidden" name="send" id="send" value="" />
	
	<div id="button_save">
	
		<?php if(!$only_view){ ?>
		<button type="submit" onclick="document.getElementById('send').value='Save';return check();"><?php echo language('Save');?></button>
		<?php } ?>
		
	</div>

</form>


<script type="text/javascript"> 
$(document).ready(function (){ 
	onload_func();
}); 

$("#show_password").change(function(){
	if($(this).attr("checked") == 'checked'){
		$("#userpwd").prop("type","text");
	}else{
		$("#userpwd").prop("type","password");
	}
});
</script>



<?php require("/www/cgi-bin/inc/boot.inc");?>