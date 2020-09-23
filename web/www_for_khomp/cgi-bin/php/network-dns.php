<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
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

	}
}

?>


<?php

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

<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript">
<!--
function check()
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
-->
</script>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

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

	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>

	</form>

<?php require("/www/cgi-bin/inc/boot.inc");?>
