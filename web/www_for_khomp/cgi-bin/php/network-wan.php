<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/network_factory.inc");
?>

<?php
//AQL
include_once("/www/cgi-bin/inc/aql.php");
$aql = new aql();

$setok = $aql->set('basedir','/etc/asterisk/gw/network');
if (!$setok) {
	echo __LINE__.' '.$aql->get_error();
	exit;
}
?>

<?php
if($_POST && isset($_POST['send']) && $_POST['send']=='Save') {
	if($only_view){
		return false;
	}

	if(isset($_POST['wan_type'])) {
		$_type = trim($_POST['wan_type']);
	} else {
		$_type = 'dhcp';
	}

	if(isset($_POST['wan_ipaddr'])) {
		$_ipaddr = trim($_POST['wan_ipaddr']);
	} else {
		$_ipaddr = '';
	}

	if(isset($_POST['wan_netmask'])) {
		$_netmask = trim($_POST['wan_netmask']);
	} else {
		$_netmask = '';
	}

	if(isset($_POST['wan_gateway'])) {
		$_gateway = trim($_POST['wan_gateway']);
	} else {
		$_gateway = '';
	}

	/* delete pppoe
	if(isset($_POST['ppp_username'])) {
		$_username = trim($_POST['ppp_username']);
	} else {
		$_username = '';
	}

	if(isset($_POST['ppp_password'])) {
		$_password = trim($_POST['ppp_password']);
	} else {
		$_password = '';
	}*/

	$write = "[general]\n";
	$write .= "type=$_type\n";

	$write .= "[ipv4]\n";
	$write .= "ipaddr=$_ipaddr\n";
	$write .= "netmask=$_netmask\n";
	$write .= "gateway=$_gateway\n";

	/* delete pppoe
	$write .= "[pppoe]\n";
	$write .= "username=$_username\n";
	$write .= "password=$_password\n";*/
	
	$cfg_file = "/etc/asterisk/gw/network/wan.conf"; 
	$hlock = lock_file($cfg_file);
	$fh=fopen($cfg_file,"w");
	fwrite($fh,$write);
	fclose($fh);
	unlock_file($hlock);

	//exec("/etc/init.d/wan start > /dev/null 2>&1 &");
	wait_apply("exec", "/etc/init.d/wan start > /dev/null 2>&1 &");
	
	save_user_record("","NETWORK->Wan Settings:Save,ip=".$_ipaddr);
}

?>


<?php
$type['disable'] = '';
$type['factory'] = '';
$type['static'] = '';
$type['dhcp'] = '';
$type['pppoe'] = '';

$hlock = lock_file('/etc/asterisk/gw/network/wan.conf');
$res=$aql->query("select * from wan.conf");
unlock_file($hlock);

if(isset($res['general']['type'])) {
	$cf_type=trim($res['general']['type']);
	$type["$cf_type"] = 'selected';
} else {
	$type['dhcp'] = 'selected';
}

if(isset($res['ipv4']['ipaddr'])) {
	$cf_ip=trim($res['ipv4']['ipaddr']);
} else {
	$cf_ip='';
}

if(isset($res['ipv4']['netmask'])) {
	$cf_mask=trim($res['ipv4']['netmask']);
} else {
	$cf_mask='';
}

if(isset($res['ipv4']['gateway'])) {
	$cf_gw=trim($res['ipv4']['gateway']);
} else {
	$cf_gw='';
}

/* delete pppoe
if(isset($res['pppoe']['username'])) {
	$cf_username=trim($res['pppoe']['username']);
} else {
	$cf_username='';
}

if(isset($res['pppoe']['password'])) {
	$cf_password=trim($res['pppoe']['password']);
} else {
	$cf_password='';
}*/

exec("/my_tools/net_tool eth1 2> /dev/null && echo ok",$output);
$cf_mac='';
if(isset($output[2])) $cf_mac = $output[2];
$slot_num = get_slotnum();
$factory_ip = $__FACTORY_WAN_IP__;
$factory_mask = $__FACTORY_WAN_MASK__;
$factory_gw = $__FACTORY_WAN_GW__;
?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript">
<!--
function typechange()
{
	var type = document.getElementById('wan_type').value;

	if(type == 'factory') {
		set_visible('field_wan_ipaddr', true);
		set_visible('field_wan_netmask', true);
		set_visible('field_wan_gateway', true);

		set_visible('field_username', false);
		set_visible('field_passwd', false);

		set_visible('field_wan_mac', true);


		var obj = document.getElementById('wan_mac');
		obj.readOnly = true;
		obj.value = "<?php echo $cf_mac;?>";

		obj = document.getElementById('wan_ipaddr');
		obj.readOnly = true;
		obj.value = "<?php echo $factory_ip;?>";

		obj = document.getElementById('wan_netmask');
		obj.readOnly = true;
		obj.value = "<?php echo $factory_mask;?>";

		obj = document.getElementById('wan_gateway');
		obj.readOnly = true;
		obj.value = "<?php echo $factory_gw;?>";
	} else if (type == 'static') {
		set_visible('field_wan_ipaddr', true);
		set_visible('field_wan_netmask', true);
		set_visible('field_wan_gateway', true);

		set_visible('field_username', false);
		set_visible('field_passwd', false);

		set_visible('field_wan_mac', true);

		var obj = document.getElementById('wan_mac');
		obj.readOnly = false;
		obj.value = "<?php if($cf_mac)echo $cf_mac;else echo $cf_mac; ?>";

		obj = document.getElementById('wan_ipaddr');
		obj.readOnly = false;
		obj.value = "<?php echo $cf_ip;?>";

		obj = document.getElementById('wan_netmask');
		obj.readOnly = false;
		obj.value = "<?php echo $cf_mask;?>";

		obj = document.getElementById('wan_gateway');
		obj.readOnly = false;
		obj.value = "<?php echo $cf_gw;?>";
	} else if (type == 'dhcp') {
		set_visible('field_wan_ipaddr', false);
		set_visible('field_wan_netmask', false);
		set_visible('field_wan_gateway', false);

		set_visible('field_username', false);
		set_visible('field_passwd', false);

		set_visible('field_wan_mac', true);
		var obj = document.getElementById('wan_mac');
		obj.readOnly = false;
		obj.value = "<?php if($cf_mac)echo $cf_mac;else echo $cf_mac; ?>";
	/* delete pppoe
	} else if (type == 'pppoe') {
		set_visible('field_wan_ipaddr', false);
		set_visible('field_wan_netmask', false);
		set_visible('field_wan_gateway', false);

		set_visible('field_username', true);
		set_visible('field_passwd', true);

		set_visible('field_wan_mac', true);
		var obj = document.getElementById('wan_mac');
		obj.readOnly = false;
		obj.value = "<?php if($cf_mac)echo $cf_mac;else echo $cf_mac; ?>";*/
	} else { //Disable
		set_visible('field_wan_ipaddr', false);
		set_visible('field_wan_netmask', false);
		set_visible('field_wan_gateway', false);

		set_visible('field_username', false);
		set_visible('field_passwd', false);

		set_visible('field_wan_mac', false);
	}
}

function check()
{
	var is_check = false;
	
	var wan_type = document.getElementById("wan_type").value;
	var wan_ipaddr = document.getElementById("wan_ipaddr").value;
	var wan_netmask = document.getElementById("wan_netmask").value;
	var wan_gateway = document.getElementById("wan_gateway").value;
	/* delete pppoe
	var ppp_username = document.getElementById("ppp_username").value;
	var ppp_password = document.getElementById("ppp_password").value;*/

	if(wan_type == 'static') {
		if(!check_ip(wan_ipaddr)) {
			document.getElementById('wan_ipaddr').focus();
			document.getElementById("cwan_ipaddr").innerHTML = con_str('<?php echo language('js check ip','Please input a valid IP address');?>');
			is_check = true;
		} else {
			document.getElementById("cwan_ipaddr").innerHTML = '';
		}

		if(!check_ip(wan_netmask)) {
			document.getElementById('wan_netmask').focus();
			document.getElementById("cwan_netmask").innerHTML = con_str('<?php echo language('js check ip','Please input a valid IP address');?>');
			is_check = true;
		} else {
			document.getElementById("cwan_netmask").innerHTML = '';
		}

		if(!check_ip(wan_gateway)) {
			document.getElementById('wan_gateway').focus();
			document.getElementById("cwan_gateway").innerHTML = con_str('<?php echo language('js check ip','Please input a valid IP address');?>');
			is_check = true;
		} else {
			document.getElementById("cwan_gateway").innerHTML = '';
		}
	}
	
	if(is_check){
		return false;
	}

	/* delete pppoe
	if(wan_type == 'pppoe') {
		if(!check_pppoeuser(ppp_username)) {
			document.getElementById("cppp_username").innerHTML = con_str('<?php echo language('js check pppoeuser','Please input valid username');?>');
			return false;
		} else {
			document.getElementById("cppp_username").innerHTML = '';
		}

		if(!check_pppoepwd(ppp_password)) {
			document.getElementById("cppp_password").innerHTML = con_str('<?php echo language('js check pppoepwd','Please input valid password');?>');
			return false;
		} else {
			document.getElementById("cppp_password").innerHTML = '';
		}
	}*/

	return true;
}
-->
</script>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('WAN IPv4');?></li>
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
			<td >
				eth1
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Type');?>:
					<span class="showhelp">
					<?php echo language('Type help@network-wan','
						The method to get IP.<br/>
						Disabled: Disable WAN interface.<br/>
						Factory: Getting IP address by Slot Number(System-->Information to check slot number).<br/>
						Static: manually set up your gateway IP. <br/>
						DHCP: Automatically get IP from your local LAN.<br/> 
						PPPoE: Dial up method to get your gateway IP.<br/>');
					?>
					</span>
				</div>
			</th>
			<td >
				<select id="wan_type" name="wan_type" onchange="typechange()">
					<option  value="dhcp" <?php echo $type['dhcp'];?> ><?php echo language('DHCP');?></option>
					<option  value="static" <?php echo $type['static'];?> ><?php echo language('Static');?></option>
					<option  value="disable" <?php echo $type['disable'];?> ><?php echo language('Disable');?></option>
					<!-- <option  value="pppoe" <?php echo $type['pppoe'];?> ><?php echo language('PPPoE');?></option>-->
				</select>
			</td>
		</tr>				
		<tr id="field_wan_mac" style="display: none">
			<th>
				<div class="helptooltips">
					<?php echo language('MAC');?>:
					<span class="showhelp">
					<?php echo language('MAC help','Physical address of your network interface.');?>
					</span>
				</div>
			</th>
			<td >
				<input id="wan_mac" type="text" name="wan_mac" value="<?php echo $cf_mac;?>" disabled /><span id="cwan_mac"></span>
			</td>
		</tr>
	</table>

	<br>

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('IPv4 Settings');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	<table width="100%" class="tedit">
		<tr id="field_wan_ipaddr" style="display: none">
			<th>
				<div class="helptooltips">
					<?php echo language('Address');?>:
					<span class="showhelp">
					<?php echo language('Address help','The IP address of your gateway.');?>
					</span>
				</div>
			</th>
			<td >
				<input id="wan_ipaddr" type="text" name="wan_ipaddr" value="<?php echo $cf_ip;?>" /><span id="cwan_ipaddr"></span>
			</td>
		</tr>
		<tr id="field_wan_netmask" style="display: none">
			<th>
				<div class="helptooltips">
					<?php echo language('Netmask');?>:
					<span class="showhelp">
					<?php echo language('Netmask help','The subnet mask of your gateway.');?>
					</span>
				</div>
			</th>
			<td >
				<input id="wan_netmask" type="text" name="wan_netmask" value="<?php echo $cf_mask;?>" /><span id="cwan_netmask"></span>
			</td>
		</tr>
		<tr id="field_wan_gateway" style="display: none">
			<th>
				<div class="helptooltips">
					<?php echo language('Default Gateway');?>:
					<span class="showhelp">
					<?php echo language('Default Gateway help','Default gateway IP addrress.');?>
					</span>
				</div>
			</th>
			<td>
				<input id="wan_gateway" type="text" name="wan_gateway" value="<?php echo $cf_gw;?>" /><span id="cwan_gateway"></span>
			</td>
		</tr>
	</table>

	<!-- delete pppoe 
	<br>

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('PPPoE Settings');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
		
	<table width="100%" class="tedit">
		<tr id="field_username" style="display: none">
			<th>
				<div class="helptooltips">
					<?php echo language('User Name');?>: 
					<span class="showhelp">
					<?php echo language('User Name help@network-wan','The user name of your PPPoE account. Basically this provided by your local provider.');?>
					</span>
				</div></th>
			<td >
				<input id="ppp_username" type="text" name="ppp_username" value="<?php echo $cf_username;?>" /><span id="cppp_username"></span>
			</td>
		</tr>
		<tr id="field_passwd" style="display: none">
			<th>
				<div class="helptooltips">
					<?php echo language('Password');?>:
					<span class="showhelp">
					<?php echo language('Password help@network-wan','The password of your PPPoE account.');?>
					</span>
				</div>
			</th>
			<td >
				<input id="ppp_password" type="text" name="ppp_password" value="<?php echo $cf_password;?>" /><span id="cppp_password"></span>
			</td>
		</tr>
	</table>-->
	
	<br>

	<input type="hidden" name="send" id="send" value="" />
	
	<?php if(!$only_view){ ?>
	<input type="submit" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>
	<?php } ?>
	
	</form>

<script type="text/javascript">
$(document).ready(function(){
	typechange();
});
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>
