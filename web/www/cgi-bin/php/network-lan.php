<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/network_factory.inc");
?>

<!---// load jQuery and the jQuery iButton Plug-in //---> 
<!--<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.3.2/jquery.min.js"></script> -->
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<script type="text/javascript" src="/js/float_btn.js"></script>

<!---// load the iButton CSS stylesheet //---> 
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />

<?php
//AQL
require_once("/www/cgi-bin/inc/aql.php");
$aql = new aql();

$setok = $aql->set('basedir','/etc/asterisk/gw/network');
if (!$setok) {
	echo __LINE__.' '.$aql->get_error();
	exit;
}
?>

<?php
if($_POST && isset($_POST['send']) && $_POST['send'] == 'Save') {
	save_to_lan_conf();
	save_to_dns_conf();
	//exec("asterisk -rx \"dnsmgr refresh\" > /dev/null 2>&1 &");
}

function save_to_lan_conf()
{
	if(isset($_POST['lan_type'])) {
		$_type = $_POST['lan_type'];
	} else {
		$_type = 'factory';
	}
	
	if($_type == 'factory'){
		global $__FACTORY_LAN_IP__;
		global $__FACTORY_LAN_MASK__;
		global $__FACTORY_LAN_GW__;
		$factory_mac = trim(file_get_contents('/tmp/.lanfactorymac'));
		$slot_num = get_slotnum();
		$factory_ip = str_replace('X',$slot_num,$__FACTORY_LAN_IP__);
		$factory_mask = $__FACTORY_LAN_MASK__;
		$factory_gw = $__FACTORY_LAN_GW__;
	
		$_mac = $factory_mac;
		$_ipaddr = $factory_ip;
		$_netmask = $factory_mask;
		$_gateway = $factory_gw;
		$_netmask = $factory_mask;
	
		if(isset($_POST['reserved_sw'])) {
			$_reserved_sw = 'on';
		} else {
			$_reserved_sw = 'off';
		}
	}else{
		if(isset($_POST['lan_mac'])) {
			$_mac = $_POST['lan_mac'];
		} else {
			$_mac = '';
		}

		if(isset($_POST['lan_ipaddr'])) {
			$_ipaddr = $_POST['lan_ipaddr'];
		} else {
			$_ipaddr = '';
		}

		if(isset($_POST['lan_netmask'])) {
			$_netmask = $_POST['lan_netmask'];
		} else {
			$_netmask = '';
		}

		if(isset($_POST['lan_gateway'])) {
			$_gateway = $_POST['lan_gateway'];
		} else {
			$_gateway = '';
		}

		if(isset($_POST['reserved_sw'])) {
			$_reserved_sw = 'on';
		} else {
			$_reserved_sw = 'off';
		}
	}

	$write = "[general]\n";
	$write .= "type=$_type\n";
	$write .= "mac=$_mac\n";

	$write .= "[ipv4]\n";
	$write .= "ipaddr=$_ipaddr\n";
	$write .= "netmask=$_netmask\n";
	$write .= "gateway=$_gateway\n";

	$write .= "[reserved]\n";
	$write .= "switch=$_reserved_sw\n";

	$cfg_file = "/etc/asterisk/gw/network/lan.conf"; 
	$hlock = lock_file($cfg_file);
	$fh=fopen($cfg_file,"w");
	fwrite($fh,$write);
	fclose($fh);
	unlock_file($hlock);

	//exec("/etc/init.d/lan start > /dev/null 2>&1 &");
	wait_apply("exec", "/etc/init.d/lan start > /dev/null 2>&1 &");
}

function save_to_dns_conf()
{
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	global $__deal_cluster__;

	if( isset($_POST['dns1']) &&
		isset($_POST['dns2']) &&
		isset($_POST['dns3']) &&
		isset($_POST['dns4'])
		) { //Save data to config file

		$_dns1 = trim($_POST['dns1']);
		$_dns2 = trim($_POST['dns2']);
		$_dns3 = trim($_POST['dns3']);
		$_dns4 = trim($_POST['dns4']);

		$write = "[general]\n";
		$write .= "dns1=$_dns1\n";
		$write .= "dns2=$_dns2\n";
		$write .= "dns3=$_dns3\n";
		$write .= "dns4=$_dns4\n";

		$cfg_file = "/etc/asterisk/gw/network/dns.conf";
		$hlock = lock_file($cfg_file);
		$fh=fopen($cfg_file,"w");
		fwrite($fh,$write);
		fclose($fh);
		unlock_file($hlock);

		$write = "";
		if($_dns1 != "") $write .= "nameserver $_dns1\n";
		if($_dns2 != "") $write .= "nameserver $_dns2\n";
		if($_dns3 != "") $write .= "nameserver $_dns3\n";
		if($_dns4 != "") $write .= "nameserver $_dns4\n";

		$cfg_file = "/etc/resolv.conf";
		$hlock = lock_file($cfg_file);
		$fh=fopen($cfg_file,"w");
		fwrite($fh,$write);
		fclose($fh);
		unlock_file($hlock);

		//DNS Need Restart asterisk.
		wait_apply("exec", "/etc/init.d/asterisk restart > /dev/null 2>&1 &");
		
		if($__deal_cluster__){
			$cluster_info = get_cluster_info();
			if($cluster_info['mode'] == 'master') {
				for($b=2; $b<=$__BRD_SUM__; $b++) {
					if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
						$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
						set_slave_file($slaveip,"/etc/asterisk/gw/network/dns.conf","/etc/asterisk/gw/network/dns.conf");
						set_slave_file($slaveip,"/tmp/resolv.conf","/tmp/resolv.conf");
						//DNS Need Restart asterisk.
						wait_apply("request_slave", $slaveip, "syscmd:/etc/init.d/asterisk restart > /dev/null 2>&1 &");
					}
				}
			}
		}
		
	}
}
?>

<?php
$type['dhcp'] = "";
$type['static'] = "";
$type['factory'] = "";

$reserved_sw = '';

$hlock = lock_file('/etc/asterisk/gw/network/lan.conf');
$res=$aql->query("select * from lan.conf");
unlock_file($hlock);

if(isset($res['general']['mac'])) {
	$cf_mac=trim($res['general']['mac']);
} else {
	$cf_mac="";
}

if(isset($res['general']['type'])) {
	$cf_type=trim($res['general']['type']);
	$type["$cf_type"] = 'selected';
} else {
	$cf_type="";
	$type['factory'] = 'selected';
}

if(isset($res['ipv4']['ipaddr'])) {
	$cf_ip=trim($res['ipv4']['ipaddr']);
} else {
	$cf_ip="";
}

if(isset($res['ipv4']['netmask'])) {
	$cf_mask=trim($res['ipv4']['netmask']);
} else {
	$cf_mask="";
}

if(isset($res['ipv4']['gateway'])) {
	$cf_gw=trim($res['ipv4']['gateway']);
} else {
	$cf_gw="";
}

if(isset($res['reserved']['switch'])) {
	if(is_true(trim($res['reserved']['switch']))) {
		$reserved_sw = 'checked';
	}
}

$factory_mac = trim(file_get_contents('/tmp/.lanfactorymac'));
$slot_num = get_slotnum();
$factory_ip = str_replace('X',$slot_num,$__FACTORY_LAN_IP__);
$factory_mask = $__FACTORY_LAN_MASK__;
$factory_gw = $__FACTORY_LAN_GW__;

$reserved_ip = str_replace('X',$slot_num,$__RESERVED_LAN_IP__);
$reserved_mask = $__RESERVED_LAN_MASK__;

/* Get DNS data */
$hlock = lock_file('/etc/asterisk/gw/network/dns.conf');
$res=$aql->query("select * from dns.conf");
unlock_file($hlock);

if(isset($res['general']['dns1'])) {
	$cf_dns1=trim($res['general']['dns1']);
} else {
	$cf_dns1="";
}

if(isset($res['general']['dns2'])) {
	$cf_dns2=trim($res['general']['dns2']);
} else {
	$cf_dns2="";
}

if(isset($res['general']['dns3'])) {
	$cf_dns3=trim($res['general']['dns3']);
} else {
	$cf_dns3="";
}

if(isset($res['general']['dns4'])) {
	$cf_dns4=trim($res['general']['dns4']);
} else {
	$cf_dns4="";
}

?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript">

function typechange()
{
	var type = document.getElementById('lan_type').value;

	if(type == 'factory') {
		set_visible('field_lan_ipaddr', true);
		set_visible('field_lan_netmask', true);
		set_visible('field_lan_gateway', true);

		obj = document.getElementById('lan_mac');
		obj.disabled = 'disabled';
		obj.value = "<?php echo $factory_mac;?>";

		obj = document.getElementById('lan_ipaddr');
		obj.disabled = 'disabled';
		obj.value = "<?php echo $factory_ip;?>";

		obj = document.getElementById('lan_netmask');
		obj.disabled = 'disabled';
		obj.value = "<?php echo $factory_mask;?>";

		obj = document.getElementById('lan_gateway');
		obj.disabled = 'disabled';
		obj.value = "<?php echo $factory_gw;?>";
	} else if (type == 'static') {
		set_visible('field_lan_ipaddr', true);
		set_visible('field_lan_netmask', true);
		set_visible('field_lan_gateway', true);

		obj = document.getElementById('lan_mac');
		obj.disabled = 'disabled';
		obj.value = "<?php if($cf_mac)echo $cf_mac;else echo $factory_mac;?>";

		obj = document.getElementById('lan_ipaddr');
		obj.readOnly = false;
		obj.disabled = '';
		obj.value = "<?php echo $cf_ip;?>";

		obj = document.getElementById('lan_netmask');
		obj.readOnly = false;
		obj.disabled = '';
		obj.value = "<?php echo $cf_mask;?>";

		obj = document.getElementById('lan_gateway');
		obj.readOnly = false;
		obj.disabled = '';
		obj.value = "<?php echo $cf_gw;?>";
	} else {
		set_visible('field_lan_ipaddr', false);
		set_visible('field_lan_netmask', false);
		set_visible('field_lan_gateway', false);

		obj = document.getElementById('lan_mac');
		obj.disabled = 'disabled';
		obj.value = "<?php if($cf_mac)echo $cf_mac;else echo $factory_mac;?>";
	}
}

function reservedswchange()
{
	var sw = document.getElementById('reserved_sw').checked;

	if(sw) {
		$("#lan_adv").slideDown();
	} else {
		$("#lan_adv").slideUp();
	}
}

function onload_func()
{
	typechange();
	reservedswchange();
}

function check()
{
	var lan_type = document.getElementById("lan_type").value;

	var lan_mac = document.getElementById("lan_mac").value;
	var lan_ipaddr = document.getElementById("lan_ipaddr").value;
	var lan_netmask = document.getElementById("lan_netmask").value;
	var lan_gateway = document.getElementById("lan_gateway").value;

	if(lan_type == 'static' || lan_type == 'dhcp') {
		if(!check_mac(lan_mac)) {
			document.getElementById("clan_mac").innerHTML = con_str('<?php echo language('js check mac','Please input a valid MAC address');?>');
			return false;
		} else {
			document.getElementById("clan_mac").innerHTML = '';
		}
	}

	if(lan_type == 'static') {
		if(!check_ip(lan_ipaddr)) {
			document.getElementById("clan_ipaddr").innerHTML = con_str('<?php echo language('js check ip','Please input a valid IP address');?>');
			return false;
		} else {
			document.getElementById("clan_ipaddr").innerHTML = '';
		}

		if(!check_ip(lan_netmask)) {
			document.getElementById("clan_netmask").innerHTML = con_str('<?php echo language('js check ip','Please input a valid IP address');?>');
			return false;
		} else {
			document.getElementById("clan_netmask").innerHTML = '';
		}
	
		if(lan_gateway!="" && !check_ip(lan_gateway)) {
			document.getElementById("clan_gateway").innerHTML = con_str('<?php echo language('js check ip','Please input a valid IP address');?>');
			return false;
		} else {
			document.getElementById("clan_gateway").innerHTML = '';
		}
	}

	if(!checkDNS()){
		return false;
	}

	return true;
}

function checkDNS()
{
	var dns1 = document.getElementById("dns1").value;
	var dns2 = document.getElementById("dns2").value;
	var dns3 = document.getElementById("dns3").value;
	var dns4 = document.getElementById("dns4").value;

	if(dns1 != '') {
		if(!check_ip(dns1)) {
			document.getElementById("cdns1").innerHTML = con_str('<?php echo language('js check ip','Please input a valid IP address');?>');
			return false;
		} else {
			document.getElementById("cdns1").innerHTML = '';
		}
	}

	if(dns2 != '') {
		if(!check_ip(dns2)) {
			document.getElementById("cdns2").innerHTML = con_str('<?php echo language('js check ip','Please input a valid IP address');?>');
			return false;
		} else {
			document.getElementById("cdns2").innerHTML = '';
		}
	}

	if(dns3 != '') {
		if(!check_ip(dns3)) {
			document.getElementById("cdns3").innerHTML = con_str('<?php echo language('js check ip','Please input a valid IP address');?>');
			return false;
		} else {
			document.getElementById("cdns3").innerHTML = '';
		}
	}

	if(dns4 != '') {
		if(!check_ip(dns4)) {
			document.getElementById("cdns4").innerHTML = con_str('<?php echo language('js check ip','Please input a valid IP address');?>');
			return false;
		} else {
			document.getElementById("cdns4").innerHTML = '';
		}
	}

	return true;
}

</script>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('LAN IPv4');?></li>
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
				eth0
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Type');?>:
					<span class="showhelp">
					<?php echo language('Type help@network-lan','
						The method to get IP.<br/>
						Factory: Getting IP address by Slot Number(System-->Information to check slot number).<br/>
						Static: manually set up your gateway IP.<br/>
						DHCP: automatically get IP from your local LAN.');
					?>
					</span>
				</div>
			</th>
			<td >
				<select id="lan_type" name="lan_type" onchange="typechange()">
					<option  value="factory" <?php echo $type['factory'];?> ><?php echo language('Factory');?></option>
					<option  value="static" <?php echo $type['static'];?> ><?php echo language('Static');?></option>
					<option  value="dhcp" <?php echo $type['dhcp'];?> ><?php echo language('DHCP');?></option>
				</select>
			</td>
		</tr>				
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('MAC');?>:
					<span class="showhelp">
					<?php echo language('MAC help','Physical address of your network interface.');?>
					</span>
				</div>
			</th>
			<td >
				<input id="lan_mac" type="text" name="lan_mac" value="<?php echo $cf_mac;?>" /><span id="clan_mac"></span>
			</td>
		</tr>
	</table>

	<br>

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('IPv4 Settings');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	<table width="100%" class="tedit" >
		<tr id="field_lan_ipaddr" style="display: none">
			<th>
				<div class="helptooltips">
					<?php echo language('Address');?>:
					<span class="showhelp">
					<?php echo language('Address help','The IP address of your gateway.');?>
					</span>
				</div>
			</th>
			<td >
				<input id="lan_ipaddr" type="text" name="lan_ipaddr" value="<?php echo $cf_ip;?>" /><span id="clan_ipaddr"></span>
			</td>
		</tr>
		<tr id="field_lan_netmask" style="display: none">
			<th>
				<div class="helptooltips">
					<?php echo language('Netmask');?>:
					<span class="showhelp">
					<?php echo language('Netmask help','The subnet mask of your gateway.');?>
					</span>
				</div>
			</th>
			<td >
				<input id="lan_netmask" type="text" name="lan_netmask" value="<?php echo $cf_mask;?>" /><span id="clan_netmask"></span>
			</td>
		</tr>
		<tr id="field_lan_gateway" style="display: none">
			<th>
				<div class="helptooltips">
					<?php echo language('Default Gateway');?>:
					<span class="showhelp">
					<?php echo language('Default Gateway help','Default gateway IP addrress.');?>
					</span>
				</div>
			</th>
			<td >
				<input id="lan_gateway" type="text" name="lan_gateway" value="<?php echo $cf_gw;?>" /><span id="clan_gateway"></span>
			</td>
		</tr>
	</table>
		
	<br>

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg">
			<div class="helptooltips">
				<?php echo language('DNS Servers');?>
				<span class="showhelp">
				<?php echo language('DNS Servers help','
					A list of DNS IP address. <br/>
					Basically this info is from your local network service provider. ');
				?>  
				</span>
			</div>
		</li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tedit" >
		<tr>
			<th><?php echo language('DNS Server');?> 1:</th>
			<td >
				<input type="text" name="dns1" id="dns1" value="<?php echo $cf_dns1;?>" /><span id="cdns1"></span>
			</td>
		</tr>
		<tr>
			<th><?php echo language('DNS Server');?> 2:</th>
			<td >
				<input type="text" name="dns2" id="dns2" value="<?php echo $cf_dns2;?>" /><span id="cdns2"></span>
			</td>
		</tr>
		<tr>
			<th><?php echo language('DNS Server');?> 3:</th>
			<td >
				<input type="text" name="dns3" id="dns3" value="<?php echo $cf_dns3;?>" /><span id="cdns3"></span>
			</td>
		</tr>
		<tr>
			<th><?php echo language('DNS Server');?> 4:</th>
			<td >
				<input type="text" name="dns4" id="dns4" value="<?php echo $cf_dns4;?>" /><span id="cdns4"></span>
			</td>
		</tr>
	</table>

	<br>

	<div id="tab" style="height:32px">
		<li class="tb1">&nbsp;</li>
		<li class="tbg">
			<div class="helptooltips">
				<?php echo language('Reserved Access IP');?>
				<span class="showhelp">
				<?php echo language('Reserved Access IP help','
					A reserved IP address to access in case your gateway IP is not available.<br/>
					Remember to set a similar network segment with the following address of your local PC.');
				?>
				</span>
			</div>
		</li>
		<li class="tb2">&nbsp;</li>
	</div>
	<div width="100%" class="div_setting_c">
		<div class="divc_setting_v">
		<table width='100%' class="tedit" style="border:none">
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Enable');?>:
					<span class="showhelp">
					<?php echo language('Enable help@network-lan','
						A switch to enable the reserved IP address or not.<br/>
						On(enabled),Off(disabled)');
					?>
					</span>
				</div>
			</th>
			<td >
				<input type="checkbox" id="reserved_sw" name="reserved_sw" onchange="reservedswchange()" <?php echo $reserved_sw ?> />
			</td>
		</tr>
		</table>
		</div>
		<div id='lan_adv' class='div_setting_d' style="position:relative;top:-2px;">
		<table width='100%' class="tedit" style="border:none">
		<tr id="reserved_ip_tr">
			<th>
				<div class="helptooltips">
					<?php echo language('Reserved Address');?>:
					<span class="showhelp">
					<?php echo language('Reserved Address help','The reserved IP address for this gateway.');?>
					</span>
				</div>
			</th>
			<td >
				<input id="reserved_ip" type="text" name="reserved_ip" value="<?php echo $reserved_ip;?>" readOnly disabled />
			</td>
		</tr>
		<tr id="reserved_mask_tr">
			<th>
				<div class="helptooltips">
					<?php echo language('Reserved Netmask');?>:
					<span class="showhelp">
					<?php echo language('Reserved Netmask help','The subnet mask of the reserved IP address.');?>
					</span>
				</div>
			</th>
			<td >
				<input id="reserved_mask" type="text" name="reserved_mask" value="<?php echo $reserved_mask;?>" readOnly disabled />
			</td>
		</tr>
		</table>
		</div>
	</div>
	<br>

	<input type="hidden" name="send" id="send" value="" />
	<table id="float_btn" class="float_btn">
		<tr id="float_btn_tr" class="float_btn_tr">
			<td>
				<input type="submit"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" <?php if($__demo_enable__=='on'){echo 'disabled';}?> />
			</td>
		</tr>
	</table>
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td width="20px">
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" <?php if($__demo_enable__=='on'){echo 'disabled';}?> />
			</td>
		</tr>
	</table>
</form>


<script type="text/javascript"> 
$(document).ready(function (){ 
	$(":checkbox").iButton(); 
	onload_func();
});
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>
<div id="float_btn1" class="float_btn1 sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()">
</div>
