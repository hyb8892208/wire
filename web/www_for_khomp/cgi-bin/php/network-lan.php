<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/network_factory.inc");
?>

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
	if($only_view){
		return false;
	}
	
	save_to_lan_conf();
	save_to_dns_conf();
	sync_sip_general_para();
	
	reset_user_login_info();//reset user login
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
		$factory_ip = $__FACTORY_LAN_IP__;
		$factory_mask = $__FACTORY_LAN_MASK__;
		$factory_gw = $__FACTORY_LAN_GW__;
	
		$_mac = $factory_mac;
		$_ipv4_enable = 'on';
		$_ipaddr = $factory_ip;
		$_netmask = $factory_mask;
		$_gateway = $factory_gw;
		$_netmask = $factory_mask;
		$ipv6_enable = 'on';
		$ipv6_addr = '';
		$ipv6_prefix = '';
		$ipv6_gw = '';
	
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
		
		if(isset($_POST['lan_enable'])){
			$_ipv4_enable = 'on';
		}else{
			$_ipv4_enable = 'off';
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
		
		if(isset($_POST['lan_ipv6_enable'])){
			$ipv6_enable = 'on';
		}else{
			$ipv6_enable = 'off';
		}
		
		if(isset($_POST['lan_ipv6_addr'])){
			$ipv6_addr = $_POST['lan_ipv6_addr'];
		}else{
			$ipv6_addr = '';
		}
		
		if(isset($_POST['lan_ipv6_prefix'])){
			$ipv6_prefix = $_POST['lan_ipv6_prefix'];
		}else{
			$ipv6_prefix = '';
		}
		
		if(isset($_POST['lan_ipv6_gateway'])){
			$ipv6_gw = $_POST['lan_ipv6_gateway'];
		}else{
			$ipv6_gw = '';
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
	$write .= "enabled=$_ipv4_enable\n";
	$write .= "ipaddr=$_ipaddr\n";
	$write .= "netmask=$_netmask\n";
	$write .= "gateway=$_gateway\n";
	
	$write .= "[ipv6]\n";
	$write .= "enabled=$ipv6_enable\n";
	$write .= "ipaddr=$ipv6_addr/$ipv6_prefix\n";
	$write .= "gateway=$ipv6_gw\n";

	$write .= "[reserved]\n";
	$write .= "switch=$_reserved_sw\n";

	$cfg_file = "/etc/asterisk/gw/network/lan.conf"; 
	$hlock = lock_file($cfg_file);
	$fh=fopen($cfg_file,"w");
	fwrite($fh,$write);
	fclose($fh);
	unlock_file($hlock);

	save_user_record("","NETWORK->Lan Settings:Save,ip=".$_ipaddr);
	//exec("/etc/init.d/lan start > /dev/null 2>&1 &");
	wait_apply("exec", "/etc/init.d/lan start > /dev/null 2>&1 &");
}

function save_to_dns_conf()
{
	global $__BRD_HEAD__;
	global $__BRD_SUM__;

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
	}
}

function sync_sip_general_para(){
	if(isset($_POST['lan_type'])) {
		$_type = $_POST['lan_type'];
	} else {
		$_type = 'factory';
	}
	
	if($_type == 'factory'){
		$ipv6_enable = 'on';
	}else{
		if(isset($_POST['lan_ipv6_enable'])){
			$ipv6_enable = 'on';
		}else{
			$ipv6_enable = 'off';
		}
	}
	
	$aql = new aql();

	$setok = $aql->set('basedir','/etc/asterisk/');
	
	$sip_general_res = get_conf("/etc/asterisk/sip_general.conf");
	
	if(!$aql->open_config_file("/etc/asterisk/sip_general.conf")){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	
	if(isset($sip_general_res['general']['udpbindaddr'])){
		$udpbindaddr = $sip_general_res['general']['udpbindaddr'];
		$temp = explode(":", $udpbindaddr);
		$port = $temp[count($temp)-1];
		
		if($ipv6_enable == 'on'){
			$new_udpbindaddr = "[::]:".$port;
		}else{
			$new_udpbindaddr = "0.0.0.0:".$port;
		}
		
		$aql->assign_editkey('general','udpbindaddr',$new_udpbindaddr);
	}
	
	if(isset($sip_general_res['general']['tcpbindaddr'])){
		$tcpbindaddr = $sip_general_res['general']['tcpbindaddr'];
		$temp = explode(":", $tcpbindaddr);
		$port = $temp[count($temp)-1];
		
		if($ipv6_enable == 'on'){
			$new_tcpbindaddr = "[::]:".$port;
		}else{
			$new_tcpbindaddr = "0.0.0.0:".$port;
		}
		
		$aql->assign_editkey('general','tcpbindaddr',$new_tcpbindaddr);
	}
	
	$aql->save_config_file('sip_general.conf');
	
	$sip_general_tls_res = file_get_contents("/etc/asterisk/sip_general_tls.conf");
	$contents = $sip_general_tls_res;
	
	if(strstr($sip_general_tls_res,'tlsbindaddr')){
		if($ipv6_enable == 'on'){
			if(strstr($sip_general_tls_res,'0.0.0.0')){
				$contents = str_replace('0.0.0.0', '[::]', $sip_general_tls_res);
			}
		}else{
			if(strstr($sip_general_tls_res,'[::]')){
				$contents = str_replace('[::]', '0.0.0.0', $sip_general_tls_res);
			}
		}
		
		file_put_contents("/etc/asterisk/sip_general_tls.conf",$contents);
	}
	
	exec("asterisk -rx \"sip reload\"");
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

if(isset($res['ipv4']['enabled'])){
	if($res['ipv4']['enabled'] == 'on'){
		$cf_enable = 'checked';
	}else{
		$cf_enable = '';
	}
}else{
	$cf_enable = 'checked';
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

if(isset($res['ipv6']['enabled'])){
	if($res['ipv6']['enabled'] == 'on'){
		$ipv6_enable = 'checked';
	}else{
		$ipv6_enable = '';
	}
}else{
	$ipv6_enable = 'checked';
}

exec("ifconfig eth0 |grep \"inet6.*Link\"|awk '{print $3}'",$ipv6_eth0_output);
$temp = explode('/',$ipv6_eth0_output[0]);

if(isset($res['ipv6']['ipaddr']) && $res['ipv6']['ipaddr'] != ""){
	$ipv6_ip_temp = explode('/',trim($res['ipv6']['ipaddr']));
	$ipv6_ip = $ipv6_ip_temp[0];
	$ipv6_prefix = $ipv6_ip_temp[1];
}else{
	$ipv6_ip = $temp[0];
	$ipv6_prefix = $temp[1];
}

if(isset($res['ipv6']['gateway'])){
	$ipv6_gw = trim($res['ipv6']['gateway']);
}else{
	$ipv6_gw = "";
}

if(isset($res['reserved']['switch'])) {
	if(is_true(trim($res['reserved']['switch']))) {
		$reserved_sw = 'checked';
	}
}

$factory_mac = trim(file_get_contents('/tmp/.lanfactorymac'));
$slot_num = get_slotnum();
$factory_ip = $__FACTORY_LAN_IP__;
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
	var is_check = false;
	
	var lan_type = document.getElementById("lan_type").value;

	var lan_mac = document.getElementById("lan_mac").value;
	var lan_ipaddr = document.getElementById("lan_ipaddr").value;
	var lan_netmask = document.getElementById("lan_netmask").value;
	var lan_gateway = document.getElementById("lan_gateway").value;
	
	var lan_ipv6_addr = document.getElementById('lan_ipv6_addr').value;
	var lan_ipv6_gateway = document.getElementById('lan_ipv6_gateway').value;

	if(lan_type == 'static' || lan_type == 'dhcp') {
		if(!check_mac(lan_mac)) {
			document.getElementById('lan_mac').focus();
			document.getElementById("clan_mac").innerHTML = con_str('<?php echo language('js check mac','Please input a valid MAC address');?>');
			is_check = true;
		} else {
			document.getElementById("clan_mac").innerHTML = '';
		}
	}

	if(lan_type == 'static') {
		if(!check_ip(lan_ipaddr)) {
			document.getElementById('lan_ipaddr').focus();
			document.getElementById("clan_ipaddr").innerHTML = con_str('<?php echo language('js check ip','Please input a valid IP address');?>');
			is_check = true;
		} else {
			document.getElementById("clan_ipaddr").innerHTML = '';
		}

		if(!check_ip(lan_netmask)) {
			document.getElementById('lan_netmask').focus();
			document.getElementById("clan_netmask").innerHTML = con_str('<?php echo language('js check ip','Please input a valid IP address');?>');
			is_check = true;
		} else {
			document.getElementById("clan_netmask").innerHTML = '';
		}
	
		if(lan_gateway!="" && !check_ip(lan_gateway)) {
			document.getElementById('lan_gateway').focus();
			document.getElementById("clan_gateway").innerHTML = con_str('<?php echo language('js check ip','Please input a valid IP address');?>');
			is_check = true;
		} else {
			document.getElementById("clan_gateway").innerHTML = '';
		}
		
		//ipv6  lan_ipv6_addr  lan_ipv6_gateway
		// if(!check_ip(lan_ipv6_addr)) {
			// document.getElementById("clan_ipv6_addr").innerHTML = con_str('<?php echo language('js check ip','Please input a valid IP address');?>');
			// return false;
		// } else {
			// document.getElementById("clan_ipv6_addr").innerHTML = '';
		// }
		
		// if(lan_ipv6_gateway!="" && !check_ip(lan_ipv6_gateway)) {
			// document.getElementById("clan_ipv6_gateway").innerHTML = con_str('<?php echo language('js check ip','Please input a valid IP address');?>');
			// return false;
		// } else {
			// document.getElementById("clan_ipv6_gateway").innerHTML = '';
		// }
	}

	if(!checkDNS()){
		is_check = true;
	}
	
	if(is_check){
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

	<div class="content">
		<span class="title">
			<?php echo language('LAN IPv4');?>
			
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
						
						<?php if(is_show_language_help('Type')){ ?>
							<b><?php echo language('Type');?>:</b><br/>
							<?php echo language('Type help','
								The method to get IP.<br/>
								Factory: Getting IP address by Slot Number(System-->Information to check slot number).<br/>
								Static: manually set up your gateway IP.<br/>
								DHCP: automatically get IP from your local LAN.');
							?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('MAC')){ ?>
							<b><?php echo language('MAC');?>:</b><br/>
							<?php echo language('MAC help','Physical address of your network interface.');?>
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
				<span>eth0</span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Type');?>:
			</span>
			<div class="tab_item_right">
				<select id="lan_type" name="lan_type" onchange="typechange()">
					<option  value="factory" <?php echo $type['factory'];?> ><?php echo language('Factory');?></option>
					<option  value="static" <?php echo $type['static'];?> ><?php echo language('Static');?></option>
					<option  value="dhcp" <?php echo $type['dhcp'];?> ><?php echo language('DHCP');?></option>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('MAC');?>:
			</span>
			<div class="tab_item_right">
				<span id="clan_mac"></span>
				<input id="lan_mac" type="text" name="lan_mac" value="<?php echo $cf_mac;?>" />
			</div>
		</div>
	</div>
	
	<div class="content">
		<span class="title">
			<?php echo language('IPv4 Settings');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Address')){ ?>
							<b><?php echo language('Address');?>:</b><br/>
							<?php echo language('Address help','The IP address of your gateway.');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Netmask')){ ?>
							<b><?php echo language('Netmask');?>:</b><br/>
							<?php echo language('Netmask help','The subnet mask of your gateway.');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Default Gateway')){ ?>
							<b><?php echo language('Default Gateway');?>:</b><br/>
							<?php echo language('Default Gateway help','Default gateway IP addrress.');?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Enable');?>:
			</span>
			<div class="tab_item_right">
				<span id="clan_enable"></span>
				<input id="lan_enable" type="checkbox" name="lan_enable" <?php echo $cf_enable;?> />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Address');?>:
			</span>
			<div class="tab_item_right">
				<span id="clan_ipaddr"></span>
				<input id="lan_ipaddr" type="text" name="lan_ipaddr" value="<?php echo $cf_ip;?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Netmask');?>:
			</span>
			<div class="tab_item_right">
				<span id="clan_netmask"></span>
				<input id="lan_netmask" type="text" name="lan_netmask" value="<?php echo $cf_mask;?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Default Gateway');?>:
			</span>
			<div class="tab_item_right">
				<span id="clan_gateway"></span>
				<input id="lan_gateway" type="text" name="lan_gateway" value="<?php echo $cf_gw;?>" />
			</div>
		</div>
	</div>
	
	<div class="content">
		<span class="title">
			<?php echo language('IPv6 Settings');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Address')){ ?>
							<b><?php echo language('Address');?>:</b><br/>
							<?php echo language('Address help','The IP address of your gateway.');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Prefix')){ ?>
							<b><?php echo language('Prefix');?>:</b><br/>
							<?php echo language('Prefix help','Prefix');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Default Gateway')){ ?>
							<b><?php echo language('Default Gateway');?>:</b><br/>
							<?php echo language('Default Gateway help','Default gateway IP addrress.');?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Enable');?>:
			</span>
			<div class="tab_item_right">
				<span id="clan_ipv6_enable"></span>
				<input id="lan_ipv6_enable" type="checkbox" name="lan_ipv6_enable" <?php echo $ipv6_enable;?> />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Address');?>:
			</span>
			<div class="tab_item_right">
				<span id="clan_ipv6_addr"></span>
				<input id="lan_ipv6_addr" type="text" name="lan_ipv6_addr" value="<?php echo $ipv6_ip;?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Prefix');?>:
			</span>
			<div class="tab_item_right">
				<span id="clan_ipv6_prefix"></span>
				<input id="lan_ipv6_prefix" type="text" name="lan_ipv6_prefix" value="<?php echo $ipv6_prefix;?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Default Gateway');?>:
			</span>
			<div class="tab_item_right">
				<span id="clan_ipv6_gateway"></span>
				<input id="lan_ipv6_gateway" type="text" name="lan_ipv6_gateway" value="<?php echo $ipv6_gw;?>" />
			</div>
		</div>
	</div>
	
	<div class="content">
		<span class="title">
			<?php echo language('DNS Servers');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<b><?php echo language('DNS Servers');?>:</b><br/>
						<?php echo language('DNS Servers help','
							A list of DNS IP address. <br/>
							Basically this info is from your local network service provider. ');
						?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('DNS Server');?> 1:
			</span>
			<div class="tab_item_right">
				<span id="cdns1"></span>
				<input type="text" name="dns1" id="dns1" value="<?php echo $cf_dns1;?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('DNS Server');?> 2:
			</span>
			<div class="tab_item_right">
				<span id="cdns2"></span>
				<input type="text" name="dns2" id="dns2" value="<?php echo $cf_dns2;?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('DNS Server');?> 3:
			</span>
			<div class="tab_item_right">
				<span id="cdns3"></span>
				<input type="text" name="dns3" id="dns3" value="<?php echo $cf_dns3;?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('DNS Server');?> 4:
			</span>
			<div class="tab_item_right">
				<span id="cdns4"></span>
				<input type="text" name="dns4" id="dns4" value="<?php echo $cf_dns4;?>" />
			</div>
		</div>
	</div>
	
	<div class="content">
		<span class="title">
			<?php echo language('Reserved Access IP');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Reserved Access IP')){ ?>
							<b><?php echo language('Reserved Access IP');?>:</b><br/>
							<?php echo language('Reserved Access IP help','
								A reserved IP address to access in case your gateway IP is not available.<br/>
								Remember to set a similar network segment with the following address of your local PC.');
							?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Enable')){ ?>
							<b><?php echo language('Enable');?>:</b><br/>
							<?php echo language('Enable help','
								A switch to enable the reserved IP address or not.<br/>
								On(enabled),Off(disabled)');
							?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Reserved Address')){ ?>
							<b><?php echo language('Reserved Address');?>:</b><br/>
							<?php echo language('Reserved Address help','The reserved IP address for this gateway.');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Reserved Netmask')){ ?>
							<b><?php echo language('Reserved Netmask');?>:</b><br/>
							<?php echo language('Reserved Netmask help','The subnet mask of the reserved IP address.');?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Enable');?>:
			</span>
			<div class="tab_item_right">
				<span><input type="checkbox" id="reserved_sw" name="reserved_sw" onchange="reservedswchange()" <?php echo $reserved_sw ?> /></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Reserved Address');?>:
			</span>
			<div class="tab_item_right">
				<input id="reserved_ip" type="text" name="reserved_ip" value="<?php echo $reserved_ip;?>" readOnly disabled />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Reserved Netmask');?>:
			</span>
			<div class="tab_item_right">
				<input id="reserved_mask" type="text" name="reserved_mask" value="<?php echo $reserved_mask;?>" readOnly disabled />
			</div>
		</div>
	</div>

	<input type="hidden" name="send" id="send" value="" />
	
	<div id="button_save">
	
		<?php if(!$only_view){ ?>
		<button type="submit" onclick="document.getElementById('send').value='Save';return check();" <?php if($__demo_enable__=='on'){echo 'disabled';}?> ><?php echo language('Save');?></button>
		<?php } ?>
		
	</div>
</form>


<script type="text/javascript"> 
$(document).ready(function (){ 
	onload_func();
});

$("#lan_ipv6_enable").click(function(){
	if($(this).attr('checked') != "checked" && $("#lan_enable").attr("checked") != "checked"){
		alert("<?php echo language("Protocol Open","Select at least one protocol to open"); ?>");
		return false;
	}
});

$("#lan_enable").click(function(){
	if($(this).attr('checked') != "checked" && $("#lan_ipv6_enable").attr("checked") != "checked"){
		alert("<?php echo language("Protocol Open","Select at least one protocol to open"); ?>");
		return false;
	}
});
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>