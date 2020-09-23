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

$product_type = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys product_type");
if($_POST && isset($_POST['send']) && $_POST['send'] == 'Save') {
	$lan_mac = trim($_POST['lan_mac']);
	$wan_mac = trim($_POST['wan_mac']);
	$lan_pre_mac = trim($_POST['lan_pre_mac']);
	
	if($product_type < 4){
		$burn_cmd = "burn_mac";
	}else{
		$burn_cmd = "burn_mac.sh";
	}
	
	if($wan_mac == ''){
		save_user_record("","NETWORK->Mac:Save,lan=".$lan_mac);
		
		if($lan_pre_mac != ""){
			exec("/my_tools/$burn_cmd -l $lan_mac -h $lan_pre_mac > /dev/null 2>&1 &");
		}else{
			exec("/my_tools/$burn_cmd -l $lan_mac > /dev/null 2>&1 &");
		}
	}else{
		save_user_record("","NETWORK->Mac:Save,lan=".$lan_mac.",wan=".$wan_mac);
		
		if($lan_pre_mac != ""){
			exec("/my_tools/$burn_cmd -l $lan_mac -w $wan_mac -h $lan_pre_mac > /dev/null 2>&1 &");
		}else{
			exec("/my_tools/$burn_cmd -l $lan_mac -w $wan_mac > /dev/null 2>&1 &");
		}
	}
}

if($product_type < 4){
	exec("/my_tools/net_tool eth0.2 2> /dev/null && echo ok",$output);
	$cur_lan_mac = exec("ifconfig |grep br-lan |grep -v br-lan:0 | grep HWaddr|awk '{print $5}'");
	$cur_wan_mac = exec("ifconfig |grep eth0.2 |grep -v eth0.2:0 | grep HWaddr|awk '{print $5}'");
}else{
	exec("/my_tools/net_tool eth1 2> /dev/null && echo ok",$output);
	$cur_lan_mac = exec("ifconfig |grep eth0 |grep -v eth0:0 | grep HWaddr|awk '{print $5}'");
	$cur_wan_mac = exec("ifconfig |grep eth1 |grep -v eth1:0 | grep HWaddr|awk '{print $5}'");
}

$cur_lan_mac = str_replace(':','',$cur_lan_mac);
$cur_wan_mac = str_replace(':','',$cur_wan_mac);

$old_wan_mac = $output[2];
?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript">


function check()
{
	if(confirm("<?php echo language("Burn Confirm",  "After clicking 'burning' to complete, the device will automatically shut down! Do you want to continue?");?>")){
		var lan_mac = document.getElementById("lan_mac").value;
		var wan_mac = document.getElementById("wan_mac").value;
		var lan_pre_mac = document.getElementById("lan_pre_mac").value;
		
		document.getElementById('clan_mac').innerHTML = "";
		document.getElementById('cwan_mac').innerHTML = "";
		document.getElementById('clan_pre_mac').innerHTML = "";
		var rex=/^[0-9a-fA-F]{12}$/i;
		if(!rex.test(lan_mac)) {
			document.getElementById('clan_mac').innerHTML = con_str("<?php echo language("Mac Tip", 'Please enter a 12-bit string with character requirements of 0-9 or a-f or A-F');?>");
			return false;
		}
		if(wan_mac != ''){
			if(!rex.test(wan_mac)) {
				document.getElementById('cwan_mac').innerHTML = con_str("<?php echo language("Mac Tip", 'Please enter a 12-bit string with character requirements of 0-9 or a-f or A-F');?>");
				return false;
			}
		}
		
		var rex_pre=/^[0-9a-fA-F]{8}$/i;
		if(!rex_pre.test(lan_pre_mac) && lan_pre_mac.length != 0) {
			document.getElementById('clan_pre_mac').innerHTML = con_str("<?php echo language("Mac Prefix Tip", 'Please enter a 8-bit string with character requirements of 0-9 or a-f or A-F');?>");
			return false;
		}
	} else {
		return false;
	}

	return true;
}

function mac_show_tip(that, type){
	var rex=/^[0-9a-fA-F]{12}$/i;
	if(type == 'lan'){
		var lan_mac = document.getElementById("lan_mac").value;
		document.getElementById('clan_mac').innerHTML = "";
		
		if(!rex.test(lan_mac)) {
			document.getElementById('clan_mac').innerHTML = con_str("<?php echo language("Mac Tip", 'Please enter a 12-bit string with character requirements of 0-9 or a-f or A-F');?>");
			return false;
		}
	}else{
		var wan_mac = document.getElementById("wan_mac").value;
		document.getElementById('cwan_mac').innerHTML = "";
		
		if(wan_mac != ''){
			if(!rex.test(wan_mac)) {
				document.getElementById('cwan_mac').innerHTML = con_str("<?php echo language("Mac Tip", 'Please enter a 12-bit string with character requirements of 0-9 or a-f or A-F');?>");
				return false;
			}
		}
	}
	
	var new_mac = $.trim(that.value).toLowerCase();
	
	if(type == 'lan'){
		document.getElementById('clan_mac').innerHTML = "";
		var old_mac = "<?php echo $cur_lan_mac; ?>";
	}else{
		document.getElementById('cwan_mac').innerHTML = "";
		var old_mac = "<?php echo $cur_wan_mac; ?>";
	}
	old_mac = $.trim(old_mac).toLowerCase();
	
	if(new_mac != old_mac){
		if(type == 'lan'){
			document.getElementById('clan_mac').innerHTML = "<span class='incorrect'></span>"+con_str("<?php echo language('MAC Address Diff','Note: MAC address is inconsistent with current.');?>&nbsp&nbspMAC:"+old_mac);
		}else{
			document.getElementById('cwan_mac').innerHTML = "<span class='incorrect'></span>"+con_str("<?php echo language('MAC Address Diff','Note: MAC address is inconsistent with current.');?>&nbsp&nbspMAC:"+old_mac);
		}
	}else{
		if(type == 'lan'){
			document.getElementById('clan_mac').innerHTML = "<span class='correct'></span>"+"<span style='color:#008100;'><?php echo language('MAC Address Same','MAC address is the same as the current one.');?>&nbsp&nbspMAC:"+old_mac+"</span>";
		}else{
			document.getElementById('cwan_mac').innerHTML = "<span class='correct'></span>"+"<span style='color:#008100;'><?php echo language('MAC Address Same','MAC address is the same as the current one.');?>&nbsp&nbspMAC:"+old_mac+"</span>";
		}
	}
}
</script>

<style>
.correct:before {
    content: '\2714';
    color: #008100;
	font-weight:bold;
	font-size:16px;
}

.incorrect:before {
    content: '\2716';
    color: #b20610;
	font-weight:bold;
	font-size:16px;
}
</style>

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
			<td>
				<?php if($product_type < 4){ ?>
				br-lan
				<?php }else{ ?>
				eth0
				<?php } ?>
			</td>
		</tr>
		<tr style="display:none;">
			<th>
				<div class="helptooltips">
					<?php echo language('MAC Prefix');?>:
					<span class="showhelp">
					<?php echo language('MAC Prefix help','Physical address of your network interface.').'<br/>'.language("Mac Prefix Tip", 'Please enter a 8-bit string with character requirements of 0-9 or a-f or A-F');?>
					</span>
				</div>
			</th>
			<td >
				<input id="lan_pre_mac" type="text" name="lan_pre_mac" value="<?php echo $lan_pre_mac;?>" readonly />
				<span style="vertical-align:middle;display:inline-block;"><input type="checkbox" id="show_password" /></span>
				<span id="clan_pre_mac"></span>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('MAC');?>:
					<span class="showhelp">
					<?php echo language('MAC help','Physical address of your network interface.').'<br/>'.language("Mac Tip", 'Please enter a 12-bit string with character requirements of 0-9 or a-f or A-F');?>
					</span>
				</div>
			</th>
			<td >
				<input id="lan_mac" type="text" name="lan_mac" value="" oninput="mac_show_tip(this, 'lan')" />
				<span id="clan_mac"></span>
			</td>
		</tr>
	</table>
	
	<br/>
	
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
			<td>
				<?php if($product_type < 4){ ?>
				eth0.2
				<?php }else{ ?>
				eth1
				<?php } ?>
			</td>
		</tr>			
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('MAC');?>:
					<span class="showhelp">
					<?php echo language('MAC help','Physical address of your network interface.').'<br/>'.language("Mac Tip", 'Please enter a 12-bit string with character requirements of 0-9 or a-f or A-F');?>
					</span>
				</div>
			</th>
			<td >
				<input id="wan_mac" type="text" name="wan_mac" oninput="mac_show_tip(this, 'wan')" value="" <?php if($old_wan_mac == '') echo 'disabled';?> />
				<span id="cwan_mac"></span>
			</td>
		</tr>
	</table>
	<input type="hidden" name="send" id="send" value="" />
	<table id="float_btn" class="float_btn">
		<tr id="float_btn_tr" class="float_btn_tr">
			<td>
				<input type="submit"   value="<?php echo language('Burn');?>" onclick="document.getElementById('send').value='Save';return check();" />
			</td>
		</tr>
	</table>
</form>

<script>
$("#show_password").click(function(){
	if($(this).attr("checked") == "checked"){
		$("#lan_pre_mac").removeAttr("readonly");
	}else{
		$("#lan_pre_mac").attr("readonly","readonly");
	}
});
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>
