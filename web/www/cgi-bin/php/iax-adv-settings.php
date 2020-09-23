<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
require_once("/www/cgi-bin/inc/aql.php");
//require_once("/www/cgi-bin/php/wsclient.php");
?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript">
<!--
function enable_tcp_change()
{
	value = document.getElementById('enable_tcp').value == 'yes' ? false : true;
	
	document.getElementById('tcp_bind_port').disabled = value;
	document.getElementById('tcp_authentication_timeout').disabled = value;
	document.getElementById('tcp_authentication_limit').disabled = value;
}

function enable_internal_sip_call_change()
{
	value = document.getElementById('enable_internal_sip_call').value == 'yes' ? false : true;
	document.getElementById('internal_sip_call_prefix').disabled = value;
}

function onload_func()
{
	//enable_tcp_change();
	//enable_internal_sip_call_change();
}
-->
</script>

<?php
//no using
function prepare_slave_iax_general_conf($src_file,$des_file)
{
	$str = "register";	
	$str_len = strlen($str);

	$flock1 = lock_file($src_file);
	$flock2 = lock_file($des_file);

	$srch=fopen($src_file,'r');
	$desh=fopen($des_file,'w');
	while(!feof($srch)) {
		//$line = stream_get_line($srch, 1000000, "\n");
		$line = fgets($srch);
		if(strncmp($line,$str,$str_len) == 0) continue;
		if($line) fwrite($desh,$line);
	}
	fclose($desh);
	fclose($srch);

	unlock_file($flock1);
	unlock_file($flock2);
	
	return true;
}

function save_to_iax_general_conf()
{

	$datachunk = '';
	//POST data pretreatment
	
	$section = 'general';
	
	if(isset($_POST['iax_bind_port']) && $_POST['iax_bind_port'] != '') {
		$_iax_bind_port = $_POST['iax_bind_port'];		
		$datachunk .="bindport=".$_iax_bind_port."\n";
	}
	
	if(isset($_POST['iax_bind_addr']) && $_POST['iax_bind_addr'] != '') {
		$_iax_bind_addr = $_POST['iax_bind_addr'];	
		$datachunk .="bindaddr=".$_iax_bind_addr."\n";
	}
	
	if(isset($_POST['enable_iaxcompat']) && $_POST['enable_iaxcompat'] != '') {
		$_enable_iaxcompat = $_POST['enable_iaxcompat'];
		$datachunk .="iaxcompat=".$_enable_iaxcompat."\n";
	} 
	
	if(isset($_POST['enable_nochecksums']) && $_POST['enable_nochecksums'] != '') {
		$_enable_nochecksums = $_POST['enable_nochecksums'];	
		$datachunk .="nochecksums=".$_enable_nochecksums."\n";
	}
	
	
	if(isset($_POST['enable_delayreject']) && $_POST['enable_delayreject'] != '') {
		$_enable_delayreject = $_POST['enable_delayreject'];	
		$datachunk .="delayreject=".$_enable_delayreject."\n";
	} 
	
	
	
	if(isset($_POST['enable_adsi']) && $_POST['enable_adsi'] != '') {
		$_enable_adsi = $_POST['enable_adsi'];	
		$datachunk .="adsi=".$_enable_adsi."\n";
	}
	
	
	if(isset($_POST['enable_srvloopup']) && $_POST['enable_srvloopup'] != '') {
		$_enable_srvloopup = $_POST['enable_srvloopup'];	
		$datachunk .="srvloopup=".$_enable_srvloopup."\n";
	} 
	
	
	if(isset($_POST['iax_amaflags']) && $_POST['iax_amaflags'] != '') {
		$_iax_amaflags = $_POST['iax_amaflags'];	
		$datachunk .="amaflags=".$_iax_amaflags."\n";
	} 
	
	
	if(isset($_POST['enable_autokill']) && $_POST['enable_autokill'] != '') {
		$_enable_autokill = $_POST['enable_autokill'];	
		$datachunk .="autokill=".$_enable_autokill."\n";
	} 
	
		
	if(isset($_POST['iax_language']) && $_POST['iax_language'] != '') {
		$_iax_language = $_POST['iax_language'];	
		$datachunk .="language=".$_iax_language."\n";
	} 
	
	if(isset($_POST['iax_accountcode']) && $_POST['iax_accountcode'] != '') {
		$_iax_accountcode = $_POST['iax_accountcode'];	
		$datachunk .="accountcode=".$_iax_accountcode."\n";
	} 
	
	if(isset($_POST['iax_calltokenoptional']) && $_POST['iax_calltokenoptional'] != '') {
		$_iax_calltokenoptional = $_POST['iax_calltokenoptional'];	
		$datachunk .="calltokenoptional=".$_iax_calltokenoptional."\n";
	} 
	
	if(isset($_POST['iax_description']) && $_POST['iax_description'] != '') {
		$_iax_description = $_POST['iax_description'];	
		$datachunk .="description=".$_iax_description."\n";
	} 
	
	
	//////Musci on Hold/////////////
	if(isset($_POST['iax_mohsuggest']) && $_POST['iax_mohsuggest'] != '') {
		$_iax_mohsuggest = $_POST['iax_mohsuggest'];	
		$datachunk .="mohsuggest=".$_iax_mohsuggest."\n";
	} 
	
	if(isset($_POST['iax_mohinterpret']) && $_POST['iax_mohinterpret'] != '') {
		$_iax_mohinterpret = $_POST['iax_mohinterpret'];	
		$datachunk .="mohinterpret=".$_iax_mohinterpret."\n";
	} 
	
	
	
	
	//codec settings
	
	if(isset($_POST['iax_bandwidth']) && $_POST['iax_bandwidth'] != '') {
		$_bandwidth = $_POST['iax_bandwidth'];	
		$datachunk .="bandwidth=".$_bandwidth."\n";
	}
	
	
	if(isset($_POST['iax_disallow']) && $_POST['iax_disallow'] != '') {
		$_iax_disallow = $_POST['iax_disallow'];	
		$datachunk .="disallow=".$_iax_disallow."\n";
	}
	
	
	$allow_val = '';
	for($i=1; $i<=7; $i++) {
		$name='iax_codec_priority'.$i;
		if(isset($_POST[$name]) && $_POST[$name] != '') {
			$val = $_POST[$name];
			if ($val!="notuse"){ 
			$allow_val .= "$val,"; 
			}
		}
	}
	$allow_val = rtrim($allow_val,',');
	$datachunk .="allow=".$allow_val."\n";
	
	
	
	if(isset($_POST['iax_codecpriority']) && $_POST['iax_codecpriority'] != '') {
		$_iax_codecpriority = $_POST['iax_codecpriority'];	
		$datachunk .="codecpriority=".$_iax_codecpriority."\n";
	}
	
	//Jitter Buffer
	
	if(isset($_POST['enable_jitterbuffer']) && $_POST['enable_jitterbuffer'] != '') {
		$_enable_jitterbuffer = $_POST['enable_jitterbuffer'];	
		$datachunk .="jitterbuffer=".$_enable_jitterbuffer."\n";
	}
	
	if(isset($_POST['enable_forcejitterbuffer']) && $_POST['enable_forcejitterbuffer'] != '') {
		$_enable_forcejitterbuffer = $_POST['enable_forcejitterbuffer'];	
		$datachunk .="forcejitterbuffer=".$_enable_forcejitterbuffer."\n";
	}
	
	if(isset($_POST['iax_maxjitterbuffer']) && $_POST['iax_maxjitterbuffer'] != '') {
		$_iax_maxjitterbuffer = $_POST['iax_maxjitterbuffer'];	
		$datachunk .="maxjitterbuffer=".$_iax_maxjitterbuffer."\n";
	}
	
	if(isset($_POST['iax_resyncthreshold']) && $_POST['iax_resyncthreshold'] != '') {
		$_iax_resyncthreshold = $_POST['iax_resyncthreshold'];	
		$datachunk .="resyncthreshold=".$_iax_resyncthreshold."\n";
	}
	
	if(isset($_POST['iax_maxjitterinterps']) && $_POST['iax_maxjitterinterps'] != '') {
		$_iax_maxjitterinterps = $_POST['iax_maxjitterinterps'];	
		$datachunk .="maxjitterinterps=".$_iax_maxjitterinterps."\n";
	}
	
	if(isset($_POST['iax_jittertargetextra']) && $_POST['iax_jittertargetextra'] != '') {
		$_iax_jittertargetextra = $_POST['iax_jittertargetextra'];	
		$datachunk .="jittertargetextra=".$_iax_jittertargetextra."\n";
	}
	
	//Misc Settings
	
	
	if(isset($_POST['iax_iaxthreadcount']) && $_POST['iax_iaxthreadcount'] != '') {
		$_iax_iaxthreadcount = $_POST['iax_iaxthreadcount'];	
		$datachunk .="iaxthreadcount=".$_iax_iaxthreadcount."\n";
	}
	
	if(isset($_POST['iax_iaxmaxthreadcount']) && $_POST['iax_iaxmaxthreadcount'] != '') {
		$_iaxmaxthreadcount = $_POST['iax_iaxmaxthreadcount'];	
		$datachunk .="iaxmaxthreadcount=".$_iaxmaxthreadcount."\n";
	}
	
	if(isset($_POST['iax_maxcallnumbers']) && $_POST['iax_maxcallnumbers'] != '') {
		$_iax_maxcallnumbers = $_POST['iax_maxcallnumbers'];	
		$datachunk .="maxcallnumbers=".$_iax_maxcallnumbers."\n";
	}
	
	if(isset($_POST['iax_maxcallnumbers_nonvalidated']) && $_POST['iax_maxcallnumbers_nonvalidated'] != '') {
		$_iax_maxcallnumbers_nonvalidated = $_POST['iax_maxcallnumbers_nonvalidated'];	
		$datachunk .="maxcallnumbers_nonvalidated=".$_iax_maxcallnumbers_nonvalidated."\n";
	}
	
	//Media
	if(isset($_POST['iax_tos']) && $_POST['iax_tos'] != '') {
		$_iax_tos = $_POST['iax_tos'];	
		$datachunk .="tos=".$_iax_tos."\n";
	}
	
	if(isset($_POST['iax_cos']) && $_POST['iax_cos'] != '') {
		$_iax_cos = $_POST['iax_cos'];	
		$datachunk .="cos=".$_iax_cos."\n";
	}
	
	
	$iax_general_conf_path = '/etc/asterisk/iax_general.conf';

	//get register infomations
	$file_str = file_get_contents($iax_general_conf_path);
	$line_array = explode("\n",$file_str);
	foreach($line_array as $key => $line_str) {
		if (strstr($line_str,'register=>')) {
			$datachunk .= $line_str."\n";
		}
	}

	$hlock = lock_file($iax_general_conf_path);
	if(!file_exists($iax_general_conf_path)) fclose(fopen($iax_general_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	if (!$aql->open_config_file($iax_general_conf_path)) {

		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	
	$aql->assign_delsection($section);
	$aql->save_config_file('iax_general.conf');
	$aql->assign_addsection($section,$datachunk);
	$aql->save_config_file('iax_general.conf');
	
	
}




function save_data()
{
	save_to_iax_general_conf();
}
?>

<?php
//Set Configs
//////////////////////////////////////////////////////////////////////////////////
if($_POST &&  isset($_POST['send']) &&  $_POST['send'] == 'Save') {
	// handle master	
	save_data();
	wait_apply("exec","asterisk -rx \"core restart now\" > /dev/null 2>&1 &");

	// sync to slave
	if($__deal_cluster__){
		$cluster_info = get_cluster_info();
		if($cluster_info['mode'] == 'master') {
			$slave_cmds = "syscmd:asterisk -rx \"core restart now\"";
			for($b=2;$b<=$__BRD_SUM__;$b++) {
				if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
					set_slave_file($slaveip,'/etc/asterisk/iax_general.conf','/etc/asterisk/iax_general.conf');
					wait_apply("request_slave", $slaveip, $slave_cmds);
				}
			}
		}
	}

	
}
///////////////////////////////////////////////////////////////////////////////////



//$iax_general_conf = IaxGeneralGetAll();
$iax_general_conf_path = '/etc/asterisk/iax_general.conf';
$aql = new aql();
$setok = $aql->set ('basedir','/etc/asterisk');
if (!$setok) {
	echo $aql->get_error();
	return ;
}
$file_head = '';
$hlock = lock_file($iax_general_conf_path);
if (!$aql->open_config_file($iax_general_conf_path)) {
	$file_head = '[general]';
	$handle = fopen($iax_general_conf_path,"w");
	fwrite($handle,$file_head);
	fclose($handle);	
	unlock_file($hlock);
	$hlock = lock_file($iax_general_conf_path);
}
$exist_array = $aql->query("select * from iax_general.conf");
$iax_general_conf = $exist_array;

//$iax_general_conf = '';
//print_rr($iax_general_conf);

if(isset($iax_general_conf['general']['bindport'])) {
	$iax_bind_port= $iax_general_conf['general']['bindport'];	
} else {
	$iax_bind_port = 4569;
}

if(isset($iax_general_conf['general']['bindaddr'])) {
	$iax_bind_addr= $iax_general_conf['general']['bindaddr'];	
} else {
	$iax_bind_addr = "0.0.0.0";
}

$enable_iaxcompat['yes'] = '';
$enable_iaxcompat['no'] = '';
if(isset($iax_general_conf['general']['iaxcompat'])) {
	$val = trim($iax_general_conf['general']['iaxcompat']);
	if(strcasecmp($val,"yes")) {
		$enable_iaxcompat['no'] = 'selected';
	} else {
		$enable_iaxcompat['yes'] = 'selected';
	}
} else {
	$enable_iaxcompat['no'] = 'selected';
}


$enable_nochecksums['yes'] = '';
$enable_nochecksums['no'] = '';
if(isset($iax_general_conf['general']['nochecksums'])) {
	$val = trim($iax_general_conf['general']['nochecksums']);
	if(strcasecmp($val,"yes")) {
		$enable_nochecksums['no'] = 'selected';
	} else {
		$enable_nochecksums['yes'] = 'selected';
	}
} else {
	$enable_nochecksums['no'] = 'selected';
}

$enable_delayreject['yes'] = '';
$enable_delayreject['no'] = '';
if(isset($iax_general_conf['general']['delayreject'])) {
	$val = trim($iax_general_conf['general']['delayreject']);
	if(strcasecmp($val,"yes")) {
		$enable_delayreject['no'] = 'selected';
	} else {
		$enable_delayreject['yes'] = 'selected';
	}
} else {
	$enable_delayreject['no'] = 'selected';
}

$enable_adsi['yes'] = '';
$enable_adsi['no'] = '';
if(isset($iax_general_conf['general']['adsi'])) {
	$val = trim($iax_general_conf['general']['adsi']);
	if(strcasecmp($val,"yes")) {
		$enable_adsi['no'] = 'selected';
	} else {
		$enable_adsi['yes'] = 'selected';
	}
} else {
	$enable_adsi['no'] = 'selected';
}

$enable_srvloopup['yes'] = '';
$enable_srvloopup['no'] = '';
if(isset($iax_general_conf['general']['srvloopup'])) {
	$val = trim($iax_general_conf['general']['srvloopup']);
	if(strcasecmp($val,"yes")) {
		$enable_srvloopup['no'] = 'selected';
	} else {
		$enable_srvloopup['yes'] = 'selected';
	}
} else {
	$enable_srvloopup['no'] = 'selected';
}

$iax_amaflags['default'] = '';
$iax_amaflags['omit'] = '';
$iax_amaflags['billing'] = '';
$iax_amaflags['documentation'] = '';

if(isset($iax_general_conf['general']['amaflags'])) {
	$val = trim($iax_general_conf['general']['amaflags']);	
	if($val=="default") {
		$iax_amaflags['default'] = 'selected';
	}
	if($val=="omit") {
		$iax_amaflags['omit'] = 'selected';
	}
	if($val=="billing") {
		$iax_amaflags['billing'] = 'selected';
	}
	if($val=="documentation") {
		$iax_amaflags['documentation'] = 'selected';
	}
	
} else {
	$iax_amaflags['default'] = 'selected';
}

$enable_autokill['yes'] = '';
$enable_autokill['no'] = '';
if(isset($iax_general_conf['general']['autokill'])) {
	$val = trim($iax_general_conf['general']['autokill']);
	if(strcasecmp($val,"yes")) {
		$enable_autokill['no'] = 'selected';
	} else {
		$enable_autokill['yes'] = 'selected';
	}
} else {
	$enable_autokill['no'] = 'selected';
}


if(isset($iax_general_conf['general']['accountcode'])) {
	$iax_accountcode = $iax_general_conf['general']['accountcode'];	
} else {
	$iax_accountcode = "";
}

if(isset($iax_general_conf['general']['calltokenoptional'])) {
	$iax_calltokenoptional = $iax_general_conf['general']['calltokenoptional'];	
} else {
	$iax_calltokenoptional = "";
}

if(isset($iax_general_conf['general']['description'])) {
	$iax_description = $iax_general_conf['general']['description'];	
} else {
	$iax_description = "";
}


//////music on hold//////
$iax_mohinterpret['default'] = '';
$iax_mohinterpret['passthrough'] = '';
if(isset($iax_general_conf['general']['mohinterpret'])) {
	$val = trim($iax_general_conf['general']['mohinterpret']);
	if($val=="default") {
		$iax_mohinterpret['default'] = 'selected';
	}
	if($val=="passthrough") {
		$iax_mohinterpret['passthrough'] = 'selected';
	}
}

//codec settings
if(isset($iax_general_conf['general']['bandwidth'])) {
	$iax_bandwidth= trim($iax_general_conf['general']['bandwidth']);	
} else {
	$iax_bandwidth = 'low';
}

if(isset($iax_general_conf['general']['allow'])) {
	$iax_allow= trim($iax_general_conf['general']['allow']);		
} else {
	$iax_allow = 'ulaw,alaw,gsm,g722,g723,g729';
}

if(isset($iax_general_conf['general']['codecpriority'])) {
	$iax_codecpriority= trim($iax_general_conf['general']['codecpriority']);	
} else {
	$iax_codecpriority = 'host';
}


$enable_jitterbuffer['yes'] = '';
$enable_jitterbuffer['no'] = '';
if(isset($iax_general_conf['general']['jitterbuffer'])) {
	$val = trim($iax_general_conf['general']['jitterbuffer']);
	if(strcasecmp($val,"yes")) {
		$enable_jitterbuffer['no'] = 'selected';
	} else {
		$enable_jitterbuffer['yes'] = 'selected';
	}
} else {
	$enable_jitterbuffer['no'] = 'selected';
}

$enable_forcejitterbuffer['yes'] = '';
$enable_forcejitterbuffer['no'] = '';
if(isset($iax_general_conf['general']['forcejitterbuffer'])) {
	$val = trim($iax_general_conf['general']['forcejitterbuffer']);
	if(strcasecmp($val,"yes")) {
		$enable_forcejitterbuffer['no'] = 'selected';
	} else {
		$enable_forcejitterbuffer['yes'] = 'selected';
	}
} else {
	$enable_forcejitterbuffer['no'] = 'selected';
}
$iax_maxjitterbuffer = '';
if(isset($iax_general_conf['general']['maxjitterbuffer'])) {
	$iax_maxjitterbuffer= trim($iax_general_conf['general']['maxjitterbuffer']);	
} else {
	$iax_maxjitterbuffer = '';
}

if(isset($iax_general_conf['general']['resyncthreshold'])) {
	$iax_resyncthreshold= trim($iax_general_conf['general']['resyncthreshold']);	
} else {
	$iax_resyncthreshold = '';
}

if(isset($iax_general_conf['general']['maxjitterinterps'])) {
	$iax_maxjitterinterps= trim($iax_general_conf['general']['maxjitterinterps']);	
} else {
	$iax_maxjitterinterps = '';
}
if(isset($iax_general_conf['general']['jittertargetextra'])) {
	$iax_jittertargetextra= trim($iax_general_conf['general']['jittertargetextra']);	
} else {
	$iax_jittertargetextra = '';
}

///Misc Settings
if(isset($iax_general_conf['general']['iaxthreadcount'])) {
	$iax_iaxthreadcount= trim($iax_general_conf['general']['iaxthreadcount']);	
} else {
	$iax_iaxthreadcount = '';
}

if(isset($iax_general_conf['general']['iaxmaxthreadcount'])) {
	$iax_iaxmaxthreadcount= trim($iax_general_conf['general']['iaxmaxthreadcount']);	
} else {
	$iax_iaxmaxthreadcount = '';
}
if(isset($iax_general_conf['general']['maxcallnumbers'])) {
	$iax_maxcallnumbers= trim($iax_general_conf['general']['maxcallnumbers']);	
} else {
	$iax_maxcallnumbers = '';
}
if(isset($iax_general_conf['general']['maxcallnumbers_nonvalidated'])) {
	$iax_maxcallnumbers_nonvalidated= trim($iax_general_conf['general']['maxcallnumbers_nonvalidated']);	
} else {
	$iax_maxcallnumbers_nonvalidated = '';
}

//Media




if(isset($iax_general_conf['general']['cos'])) {
	$iax_cos= trim($iax_general_conf['general']['cos']);	
} else {
	$iax_cos = '';
}


$iax_tos['normalservice']="";
$iax_tos['lowdelay']="";
$iax_tos['highthroughput']="";
$iax_tos['highreliability']="";
$iax_tos['mincost']="";



if(isset($iax_general_conf['general']['tos'])) {
	$val = trim($iax_general_conf['general']['tos']);
	
	if($val=="0x0") {
		$iax_tos['normalservice']="selected";
	}
	if($val=="0x10") {
		$iax_tos['lowdelay'] = 'selected';
	}
	if($val=="0x8") {
		$iax_tos['highthroughput'] = 'selected';
	}
	if($val=="0x4") {
		$iax_tos['highreliability'] = 'selected';
	}
	if($val=="0x2") {
		$iax_tos['mincost'] = 'selected';
	}
	
} else {
	$iax_tos['normalservice']='selected';
}



?>

<script type="text/javascript">
function addRow()
{
	value = document.getElementById('local_network').value;
	if(value == "")
		return;

	//添加一行
	var newTr = tab_lnl.insertRow(-1);

	//添加两列
	var newTd0 = newTr.insertCell(0);
	var newTd1 = newTr.insertCell(0);

	//设置列内容和属性
	str = '<button type="button" name="send" value="Delete" style="width:32px;height:32px;" onclick=\'if(confirm("Are you sure to delete you selected ?"))javascript:this.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode);\'><img src="/images/delete.gif"></button>';
	str += '<input type="hidden" name="local_network_list[]" value="' + value + '" />';

	newTd0.innerHTML = str;
	newTd1.innerHTML = value;
}
</script>

<script type="text/javascript" src="/js/float_btn.js"></script>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
<!--
	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_networking')" id="tab_networking_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_networking')"><?php echo language('IAX2');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_networking')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>
-->
	<div id="newline"></div>
	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_general')" id="tab_general_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_general')"><?php echo language('General Settings');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_general')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>
	<table id="tab_general"  width="98%" class="tedit" align="left">
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Bind Port');?>:
					<span class="showhelp">
					<?php echo language('Bindport and bindaddr may be specified.<br>
										 (Default: 4569)');?>
					</span>
				</div>
			</th>
			<td>
				<input type="text" name="iax_bind_port" value="<?php echo $iax_bind_port;?>" />
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Bind Address');?>:
					<span class="showhelp">
					<?php echo language('More than once to bind to multiple addresses,<br>
					but the first will be the default.');?>
					</span>
				</div>
			</th>
			<td>
				<input type="text" name="iax_bind_addr" value="<?php echo $iax_bind_addr;?>" />
			</td>
		</tr>
		
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Enable IAXCompat');?>:
					<span class="showhelp">
					<?php echo language('Set iaxcompat to yes if you plan to use layered <br>
										 switches or some other scenario which may cause <br>
										 some delay when doing a lookup in the dialplan. <br>
										 It incurs a small performance hit to enable it.  <br>
										 This option causes Asterisk to spawn a separate <br>
										 thread when it receives an IAX2 DPREQ (Dialplan  <br>
										 Request) instead of blocking while it waits for <br>
										 a response.');?>
					</span>
				</div>
			</th>
			<td >
				<select size=1 name="enable_iaxcompat" id="enable_iaxcompat" >
					<option value="no"  <?php echo $enable_iaxcompat['no'] ?> > <?php echo language('_No');?> </option>
					<option value="yes" <?php echo $enable_iaxcompat['yes'] ?> > <?php echo language('_Yes');?> </option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Enable Nochecksums');?>:
					<span class="showhelp">
					<?php echo language('Disable UDP checksums (if nochecksums is set, <br> 
										 then no checkums will be calculated/checked on <br>
										 systems supporting this feature).');?>
					</span>
				</div>
			</th>
			<td >
				<select size=1 name="enable_nochecksums" id="enable_nochecksums" >
					<option value="no"  <?php echo $enable_nochecksums['no'] ?> > <?php echo language('_No');?> </option>
					<option value="yes" <?php echo $enable_nochecksums['yes'] ?> > <?php echo language('_Yes');?> </option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Enable Delay Reject');?>:
					<span class="showhelp">
					<?php echo language('You may specify a global default AMA flag for<br>
										 iaxtel calls.  It must be one of \'default\', \'omit\',<br>   
										 \'billing\' or \'documentation\'.<br>  
										 These flags are used in the generation of call<br> 
										 detail records.');?>
					</span>
				</div>
			</th>
			<td >
				<select size=1 name="enable_delayreject" id="enable_delayreject" >
					<option value="no"<?php echo $enable_nochecksums['no'] ?> > <?php echo language('_No');?> </option>
					<option value="yes"<?php echo $enable_nochecksums['yes'] ?>> <?php echo language('_Yes');?> </option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('ADSI');?>:
					<span class="showhelp">
					<?php echo language('ADSI (Analog Display Services Interface) can be<br>  
										 enabled if you have (or may have) ADSI compatible<br>   
										 CPE equipment.');?>
					</span>
				</div>
			</th>
			<td >
				<select size=1 name="enable_adsi" id="enable_adsi" >
					<option value="no" <?php echo $enable_adsi['no'] ?>> <?php echo language('_No');?> </option>
					<option value="yes"<?php echo $enable_adsi['yes'] ?>> <?php echo language('_Yes');?> </option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('SRV Loopup');?>:
					<span class="showhelp">
					<?php echo language('Whether or not to perform an SRV lookup on outbound calls.');?>
					</span>
				</div>
			</th>
			<td >
				<select size=1 name="enable_srvloopup" id="enable_srvloopup" >
					<option value="no" <?php echo $enable_srvloopup['no'] ?>> <?php echo language('_No');?> </option>
					<option value="yes"<?php echo $enable_srvloopup['yes'] ?> > <?php echo language('_Yes');?> </option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('AMA Flags');?>:
					<span class="showhelp">
					<?php echo language('You may specify a global default AMA flag for iaxtel calls.<br>  
										 These flags are used in the generation of call detail records.');?>
					</span>
				</div>
			</th>
			<td >
				<select size=1 name="iax_amaflags" id="iax_amaflags" >
					<option value="default" <?php echo $iax_amaflags['default'] ?>> <?php echo language('default');?> </option>
					<option value="omit" <?php echo $iax_amaflags['omit'] ?>> <?php echo language('omit');?> </option>
					<option value="billing" <?php echo $iax_amaflags['billing'] ?>> <?php echo language('billing');?> </option>
					<option value="documentation" <?php echo $iax_amaflags['documentation'] ?>> <?php echo language('documentation');?> </option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Auto Kill');?>:
					<span class="showhelp">
					<?php echo language('If we don\'t get ACK to our NEW within 2000ms, and<br>   
										 autokill is set to yes,then we cancel the whole thing<br>   
										 (that\'s enough time for one retransmission only).<br>    
										 This is used to keep things from stalling for a long time<br>   
										 for a host that is not available, but would be ill advised<br>   
										 for bad connections. ');?>
					</span>
				</div>
			</th>
			<td >
				<select size=1 name="enable_autokill" id="enable_autokill" >
					<option value="no" <?php echo $enable_autokill['no'] ?>> <?php echo language('_No');?> </option>
					<option value="yes"<?php echo $enable_autokill['yes'] ?>> <?php echo language('_Yes');?> </option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Lauguage');?>:
					<span class="showhelp">
					<?php echo language('You may specify a global default language for users.<br>    
										 This can be specified also on a per-user basis.<br>    
										 If omitted, will fallback to English (en)');?>
					</span>
				</div>
			</th>
			<td >
				<select size=1 name="iax_language" id="iax_language" >
					<option value="en" > <?php echo language('English');?> </option>						
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Account Code');?>:						
					<span class="showhelp">
					<?php echo language('You may specify a default account for Call Detail Records 
										(CDRs) in addition to specifying on a per-user basis.');?>
					</span>
				</div>
			</th>
			<td >
				<input name="iax_accountcode" id="iax_accountcode" value=<?php echo $iax_accountcode;?>>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Call Token Optional');?>:						
				</div>
			</th>
			<td >
				<input name="iax_calltokenoptional" id="iax_calltokenoptional" value=<?php echo $iax_calltokenoptional;?>>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Description');?>:						
				</div>
			</th>
			<td >
				<input name="iax_description" id="iax_description" value=<?php echo $iax_description;?>>
			</td>
		</tr>
	</table>
	<div id="newline"></div>
	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_music_hold')" id="tab_music_hold_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_music_hold')"><?php echo language('Music On Hold');?></li>
		<li class="tb2_fold" onclick="lud(tab_music_hold)">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>
	<table id="tab_music_hold" width="98%" class="tedit" align="left">
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Mohsuggest');?>:
					<span class="showhelp">
					<?php echo language('The \'mohsuggest\' option specifies which music on hold<br>   
										 class to suggest to the peer channel when this channel<br>   
										 places the peer on hold. It may be specified globally or<br>   
										 on a per-user or per-peer basis.');?>
					</span>
				</div>
			</th>
			<td >
				<select size=1 name="iax_mohsuggest" id="mohsuggest" >
					<option value="default" > <?php echo language('default');?> </option>						
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Mohinterpret');?>:
					<span class="showhelp">
					<?php echo language('You may specify a global default language for users.<br>    
										 This can be specified also on a per-user basis.<br>    
										 If omitted, will fallback to English (en).');?>
					</span>
				</div>
			</th>
			<td >
				<select size=1 name="iax_mohinterpret" id="iax_mohinterpret" >
					<option value="" > <?php echo language('');?> </option>						
					<option value="default" <?php echo $iax_mohinterpret['default']?>> <?php echo language('default');?> </option>						
					<option value="passthrough" <?php echo $iax_mohinterpret['passthrough']?>> <?php echo language('passthrough');?> </option>						
				</select>
			</td>
		</tr>
		
	</table>	
	<div id="newline"></div>
	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_codec')" id="tab_codec_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_codec')"><?php echo language('Codec Settings');?></li>
		<li class="tb2_fold" onclick="lud(tab_codec)">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>
	
	<table id="tab_codec" width="98%" class="tedit" align="left">
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Band Width');?>:
					<span class="showhelp">
					<?php echo language('Specify bandwidth of low, medium, or high to <br>  
										 control which codecs are used in general.');?>
					</span>
				</div>
			</th>
			<td >
				<select size=1 name="iax_bandwidth" id="iax_bandwidth" >
					<option value="low" <?php if ($iax_bandwidth=="low") echo "Selected"?>> <?php echo language('low');?> </option>
					<option value="medium" <?php if ($iax_bandwidth=="medium") echo "Selected"?>> <?php echo language('medium');?> </option>
					<option value="high" <?php if ($iax_bandwidth=="high") echo "Selected"?>> <?php echo language('high');?> </option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Disallow');?>:						
					<span class="showhelp">
					<?php echo language('Fine tune codecs here using "allow" and <br>  
										 "disallow" clauses with specific codecs.');?>
					</span>
				</div>
			</th>
			<td >
				<select size=1 name="iax_disallow" id="iax_disallow" >
					<option value="all" > <?php echo language('all');?> </option>						
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Allow');?>:
					<span class="showhelp">
					<?php echo language('Fine tune codecs here using "allow" and <br>  
										 "disallow" clauses with specific codecs.');?>
					</span>
				</div>
			</th>
			<td >
			<?php
			
			$iax_allow_a=explode(",",$iax_allow);
			
			for($i=1;$i<=6;$i++) {
				//echo language('Codec Priority');echo " $i";
				$iax_codec_priority[$i]['notuse'] = '';
				$iax_codec_priority[$i]['ulaw'] = '';
				$iax_codec_priority[$i]['alaw'] = '';
				$iax_codec_priority[$i]['gsm'] = '';				
				$iax_codec_priority[$i]['g723'] = '';
				$iax_codec_priority[$i]['g722'] = '';
				$iax_codec_priority[$i]['g729'] = '';
			}

			if(isset($iax_allow)) {
				$allow = explode(',',$iax_allow);
				$i=1;
				foreach($allow as $each) {
					switch($each) {
					case 'ulaw':
						$iax_codec_priority[$i]['ulaw'] = 'selected';
						break;
					case 'alaw':
						$iax_codec_priority[$i]['alaw'] = 'selected';
						break;
					case 'gsm':
						$iax_codec_priority[$i]['gsm'] = 'selected';
						break;
					case 'g722':
						$iax_codec_priority[$i]['g722'] = 'selected';
						break;
					case 'g723':
						$iax_codec_priority[$i]['g723'] = 'selected';
						break;
					case 'g729':
						$iax_codec_priority[$i]['g729'] = 'selected';
						break;
					default:
						$iax_codec_priority[$i]['notuse'] = 'selected';
						break;
					}
					//if($i++>6)
					if ($i++>6)
						break;
				}
			} else { //Default must set codec priority
				$iax_codec_priority[1]['ulaw'] = 'selected';
				$iax_codec_priority[2]['alaw'] = 'selected';
				$iax_codec_priority[3]['gsm'] = 'selected';
				$iax_codec_priority[4]['g722'] = 'selected';
				$iax_codec_priority[5]['g723'] = 'selected';
				$iax_codec_priority[6]['g729'] = 'selected';					
			}
			
			
			?>
			<?php
				for($i=1;$i<=6;$i++) {
					echo "Priority $i";
			?>
			<select size=1 name="iax_codec_priority<?php echo $i?>" >
				<option value=notuse  <?php echo $iax_codec_priority[$i]['notuse']?> >Not Used</option>
				<option value=ulaw  <?php echo $iax_codec_priority[$i]['ulaw']?> >G.711 u-law</option>
				<option value=alaw  <?php echo $iax_codec_priority[$i]['alaw']?> >G.711 a-law</option>	
				<option value=g722  <?php echo $iax_codec_priority[$i]['g722']?> >G.722</option>					
				<?php 
				$product_type = get_product_type();
				if($product_type >= 4){
				?>
				<option value=g723  <?php echo $iax_codec_priority[$i]['g723']?> >G.723</option>
				<?php } ?>
				<option value=g729   <?php echo $iax_codec_priority[$i]['g729']?> >G.729</option>
				<option value=gsm   <?php echo $iax_codec_priority[$i]['gsm']?> >GSM</option>
				
			</select><br><br>			
			<?php
				}
			?>
			<?php //echo $iax_allow."<br>"?>					
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Codec Priority');?>:
					<span class="showhelp">
					<?php echo language('Codecpriority controls the codec negotiation of <br>  
										 an inbound IAX2 call. This option is inherited to <br>  
										 all user entities. It can also be defined in each <br>  
										 user entity separately which will override the <br>  
										 setting in general.');?>						</span>
				</div>
			</th>
			<td >
				<select size=1 name="iax_codecpriority" id="iax_codecpriority" >
					<option value="host" <?php if ($iax_codecpriority=="host") echo "selected";?>> <?php echo language('host');?> </option>
					<option value="caller" <?php if ($iax_codecpriority=="caller") echo "selected";?>> <?php echo language('caller');?> </option>
					<option value="disabled" <?php if ($iax_codecpriority=="disabled") echo "selected";?>> <?php echo language('disabled');?> </option>
					<option value="reqonly" <?php if ($iax_codecpriority=="reqonly") echo "selected";?>> <?php echo language('reqonly');?> </option>
				</select>
			</td>
		</tr>	
	</table>
	
	<div id="newline"></div>
	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_jitter_buffer')" id="tab_jitter_buffer_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_jitter_buffer')"><?php echo language('Jitter Buffer Settings');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_jitter_buffer')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>	
	<table id="tab_jitter_buffer"  width="98%" class="tedit" align="left">		
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Jitter Buffer');?>:
					<span class="showhelp">
					<?php echo language('Global default as to whether you want the jitter buffer at all.');?>
					</span>
				</div>
			</th>
			<td >
				<select size=1 name="enable_jitterbuffer" id="enable_jitterbuffer" >
					<option value="no" <?php echo $enable_jitterbuffer['no'] ?>> <?php echo language('_No');?> </option>
					<option value="yes" <?php echo $enable_jitterbuffer['yes'] ?>> <?php echo language('_Yes');?> </option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Force Jitter Buffer');?>:
					
									<span class="showhelp">
					<?php echo language('In the ideal world, when we bridge VoIP channels we <br>  
										 don\'t want to do jitterbuffering on the switch, since <br>  
										 the endpoints can each handle this.  However, some <br>  
										 endpoints may have poor jitterbuffers themselves, so <br>  
										 this option will force * to always jitterbuffer, even <br>  
										 in this case.');?>
					</span>
				</div>
			</th>
			<td >
				<select size=1 name="enable_forcejitterbuffer" id="enable_forcejitterbuffer" >
					<option value="no" <?php echo $enable_forcejitterbuffer['yes'] ?>> <?php echo language('_No');?> </option>
					<option value="yes"<?php echo $enable_forcejitterbuffer['yes'] ?>> <?php echo language('_Yes');?> </option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Max Jitter Buffers');?>:
					<span class="showhelp">
					<?php echo language('A maximum size for the jitter buffer.');?>
					</span>
				</div>
			</th>
			<td>
				<input type="text" name="iax_maxjitterbuffer" value="<?php echo $iax_maxjitterbuffer;?>" />
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Resyncthreshold');?>:
					<span class="showhelp">
					<?php echo language('When the jitterbuffer notices a significant change <br>  
										 in delay that continues over a few frames, it will <br>  
										resync, assuming that the change in delay was caused <br>  
										by a timestamping mix-up. The threshold for noticing <br>  
										a change in delay is measured as twice the measured <br>  
										jitter plus this resync threshold.');?>
					</span>
				</div>
			</th>
			<td>
				<input type="text" name="iax_resyncthreshold" value="<?php echo $iax_resyncthreshold;?>" /> Resyncing can be disabled by setting this parameter to -1.
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Max Jitter Interps');?>:
					<span class="showhelp">
					<?php echo language('The maximum number of interpolation frames the jitterbuffer<br>  
										 should return in a row. Since some clients do not send <br>  
										 CNG/DTX frames to indicate silence, the jitterbuffer will <br>  
										 assume silence has begun after returning this many <br>  
										 interpolations. This prevents interpolating throughout a <br>  
										 long silence.');?>
					</span>
				</div>
			</th>
			<td>
				<input type="text" name="iax_maxjitterinterps" value="<?php echo $iax_maxjitterinterps;?>" /> 
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Jitter Target Extra');?>:
					<span class="showhelp">
					<?php echo language('Number of milliseconds by which the new jitter <br>  
										 buffer will pad its size. the default is 40, so <br>  
										 without modification, the new jitter buffer will <br>  
										 set its size to the jitter value plus 40 milliseconds. <br>  
										 Increasing this value may help if your network normally <br>  
										 has low jitter, but occasionally has spikes.');?>
					</span>
				</div>
			</th>
			<td>
				<input type="text" name="iax_jittertargetextra" value="<?php echo $iax_jittertargetextra;?>" /> 
			</td>
		</tr>
		
	</table>
	<div id="newline"></div>
	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_misc')" id="tab_jitter_misc_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_misc')"><?php echo language('Misc Settings');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_misc')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>	
	<table id="tab_misc" width="98%" class="tedit" align="left">
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('IAX2 Thread Count');?>:
					<span class="showhelp">
					<?php echo language('Establishes the number of iax helper threads to handle I/O.');?>
					</span>
				</div>
			</th>
			<td>
				<input type="text" id="iax_iaxthreadcount" name="iax_iaxthreadcount" value="<?php echo $iax_iaxthreadcount;?>" /> 
			</td>
		</tr>
		
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('IAX2 Max Thread Count');?>:
					<span class="showhelp">
					<?php echo language('Establishes the number of extra dynamic threads 
										 that may be spawned to handle I/O.');?>
					</span>
				</div>
			</th>
			<td>
				<input type="text" name="iax_iaxmaxthreadcount" value="<?php echo $iax_iaxmaxthreadcount;?>" /> 
			</td>
		</tr>
		
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Max Call Number');?>:
					<span class="showhelp">
					<?php echo language('The \'maxcallnumbers\' option limits the amount of call <br>  
										 numbers allowed for each individual remote IP address. <br>   
										 Once an IP address reaches it\'s call number limit, no <br>  
										 more new connections are allowed until the previous ones <br>  
										 close. This option can be used in a peer definition as <br>  
										 well, but only takes effect for the IP of a dynamic peer <br>  
										 after it completes registration.');?>
					</span>
				</div>
			</th>
			<td>
				<input type="text" name="iax_maxcallnumbers" value="<?php echo $iax_maxcallnumbers;?>" />
			</td>
		</tr>
		
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('MaxCallNumbers_Nonvalidated');?>:
					<span class="showhelp">
					<?php echo language('The \'maxcallnumbers_nonvalidated\' is used to set the <br>  
										 combined number of call numbers that can be allocated <br>  
										 for connections where call token  validation has been <br>  
										 disabled. Unlike the \'maxcallnumbers\' option, this <br>  
										 limit is not separate for each individual IP address.  <br>  
										 Any connection resulting in a non-call token validated <br>  
										 call number being allocated contributes to this limit. <br>   
										 For use cases, see the call token user guide.  This <br>  
										 option\'s default value of 8192 should be sufficient in <br>  
										 most cases.');?>
					</span>
				</div>
			</th>
			<td>
				<input type="text" name="iax_maxcallnumbers_nonvalidated" value="<?php echo $iax_maxcallnumbers_nonvalidated;?>" />
			</td>
		</tr>
	</table>
	<div id="newline"></div>
	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_quality')" id="tab_quality_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_quality')"><?php echo language('Quality of Service');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_quality')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>	
	<table id= 'tab_quality' width="98%" class="tedit" align="left">
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('tos');?>:
					<span class="showhelp">
					<?php echo language('Type of Service');?>
					</span>
				</div>
			</th>
			<td>
			<select size=1 name="iax_tos" >
				<option value='0x00' <?php  echo $iax_tos['normalservice'];?>>Normal Service</option> 
				<option value='0x02'  <?php echo $iax_tos['mincost'];?> >Mincost</option>
				<option value='0x04'  <?php echo $iax_tos['highreliability'];?> >High Reliability</option>
				<option value='0x08'  <?php echo $iax_tos['highthroughput'];?> >High Throughput</option>
				<option value='0x10'   <?php echo $iax_tos['lowdelay'];?> >Low Delay</option>
			</select>
			</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('cos');?>:
					<span class="showhelp">
					<?php echo language('Class of Service');?>
					</span>
				</div>
			</th>
			<td>
				<input type="text" name="iax_cos" value="<?php echo $iax_cos;?>" /> 
			</td>
		</tr>
			
	</table>		
	<div id="newline"></div>
	<br>
	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" class="gen_short_btn float_btn"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';"/>
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td>
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';" />
			</td>
		</tr>
	</table>
<script type="text/javascript">
$(document).ready(function(){
	onload_func();
});

</script>
</form>
<?php require("/www/cgi-bin/inc/boot.inc");?>
<div id="float_btn1" class="float_btn1 sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()">
</div>

