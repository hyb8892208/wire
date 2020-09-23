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
	
	save_user_record("","VOIP->Advanced IAX2 Settings:Save");
}
?>

<?php
//Set Configs
//////////////////////////////////////////////////////////////////////////////////
if($_POST &&  isset($_POST['send']) &&  $_POST['send'] == 'Save') {
	if($only_view){
		return false;
	}
	
	// handle master	
	save_data();
	wait_apply("exec","asterisk -rx \"core restart now\" > /dev/null 2>&1 &");
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

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	
		<div class="content">
			<span class="title">
				<?php echo language('General Settings');?>
				
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<?php if(is_show_language_help('Bind Port')){ ?>
								<b><?php echo language('Bind Port');?>:</b><br/>
								<?php echo language('Bind Port help','Bindport and bindaddr may be specified.<br>(Default: 4569)');?>

								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Bind Address')){ ?>
								<b><?php echo language('Bind Address');?>:</b><br/>
								<?php echo language('Bind Address help','More than once to bind to multiple addresses,<br>but the first will be the default.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Enable IAX Compat')){ ?>
								<b><?php echo language('Enable IAX Compat');?>:</b><br/>
								<?php echo language('Enable IAX Compat help','Set iaxcompat to yes if you plan to use layered <br>
										 switches or some other scenario which may cause <br>
										 some delay when doing a lookup in the dialplan. <br>
										 It incurs a small performance hit to enable it.  <br>
										 This option causes Asterisk to spawn a separate <br>
										 thread when it receives an IAX2 DPREQ (Dialplan  <br>
										 Request) instead of blocking while it waits for <br>
										 a response.');?>
										 
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Enable Nochecksums')){ ?>
								<b><?php echo language('Enable Nochecksums');?>:</b><br/>
								<?php echo language('Enable Nochecksums help','Disable UDP checksums (if nochecksums is set, <br> 
										 then no checkums will be calculated/checked on <br>
										 systems supporting this feature).');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Enable Delay Reject')){ ?>
								<b><?php echo language('Enable Delay Reject');?>:</b><br/>
								<?php echo language('Enable Delay Reject help','You may specify a global default AMA flag for<br>
										 iaxtel calls.  It must be one of \'default\', \'omit\',<br>   
										 \'billing\' or \'documentation\'.<br>  
										 These flags are used in the generation of call<br> 
										 detail records.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('ADSI')){ ?>
								<b><?php echo language('ADSI');?>:</b><br/>
								<?php echo language('ADSI help','ADSI (Analog Display Services Interface) can be<br>  
										 enabled if you have (or may have) ADSI compatible<br>   
										 CPE equipment.');?>
										 
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('SRV Lookup')){ ?>
								<b><?php echo language('SRV Lookup');?>:</b><br/>
								<?php echo language('SRV Lookup help','Whether or not to perform an SRV lookup on outbound calls.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('AMA Flags')){ ?>
								<b><?php echo language('AMA Flags');?>:</b><br/>
								<?php echo language('AMA Flags help','You may specify a global default AMA flag for iaxtel calls.<br>  
										 These flags are used in the generation of call detail records.');?>
										 
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Auto Kill')){ ?>
								<b><?php echo language('Auto Kill');?>:</b><br/>
								<?php echo language('Auto Kill help','If we don\'t get ACK to our NEW within 2000ms, and<br>   
										 autokill is set to yes,then we cancel the whole thing<br>   
										 (that\'s enough time for one retransmission only).<br>    
										 This is used to keep things from stalling for a long time<br>   
										 for a host that is not available, but would be ill advised<br>   
										 for bad connections. ');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Language','Language')){ ?>
								<b><?php echo language('Language','Language');?>:</b><br/>
								<?php echo language('Language help','You may specify a global default language for users.<br>    
										 This can be specified also on a per-user basis.<br>    
										 If omitted, will fallback to English (en)');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Account Code')){ ?>
								<b><?php echo language('Account Code');?>:</b><br/>
								<?php echo language('Account Code help','You may specify a default account for Call Detail Records 
										(CDRs) in addition to specifying on a per-user basis.');?>
							<?php } ?>
						</div>
					</div>
				</div>
			</span>
			
			<div class="tab_item">
				<span>
					<?php echo language('Bind Port');?>:
				</span>
				<div class="tab_item_right">
					<input type="text" name="iax_bind_port" value="<?php echo $iax_bind_port;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Bind Address');?>:
				</span>
				<div class="tab_item_right">
					<input type="text" name="iax_bind_addr" value="<?php echo $iax_bind_addr;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Enable IAX Compat');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="enable_iaxcompat" id="enable_iaxcompat" >
						<option value="no"  <?php echo $enable_iaxcompat['no'] ?> > <?php echo language('_No');?> </option>
						<option value="yes" <?php echo $enable_iaxcompat['yes'] ?> > <?php echo language('_Yes');?> </option>
					</select>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Enable Nochecksums');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="enable_nochecksums" id="enable_nochecksums" >
						<option value="no"  <?php echo $enable_nochecksums['no'] ?> > <?php echo language('_No');?> </option>
						<option value="yes" <?php echo $enable_nochecksums['yes'] ?> > <?php echo language('_Yes');?> </option>
					</select>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Enable Delay Reject');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="enable_delayreject" id="enable_delayreject" >
						<option value="no"<?php echo $enable_nochecksums['no'] ?> > <?php echo language('_No');?> </option>
						<option value="yes"<?php echo $enable_nochecksums['yes'] ?>> <?php echo language('_Yes');?> </option>
					</select>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('ADSI');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="enable_adsi" id="enable_adsi" >
						<option value="no" <?php echo $enable_adsi['no'] ?>> <?php echo language('_No');?> </option>
						<option value="yes"<?php echo $enable_adsi['yes'] ?>> <?php echo language('_Yes');?> </option>
					</select>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('SRV Lookup');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="enable_srvloopup" id="enable_srvloopup" >
						<option value="no" <?php echo $enable_srvloopup['no'] ?>> <?php echo language('_No');?> </option>
						<option value="yes"<?php echo $enable_srvloopup['yes'] ?> > <?php echo language('_Yes');?> </option>
					</select>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('AMA Flags');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="iax_amaflags" id="iax_amaflags" >
						<option value="default" <?php echo $iax_amaflags['default'] ?>> <?php echo language('default');?> </option>
						<option value="omit" <?php echo $iax_amaflags['omit'] ?>> <?php echo language('omit');?> </option>
						<option value="billing" <?php echo $iax_amaflags['billing'] ?>> <?php echo language('billing');?> </option>
						<option value="documentation" <?php echo $iax_amaflags['documentation'] ?>> <?php echo language('documentation');?> </option>
					</select>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Auto Kill');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="enable_autokill" id="enable_autokill" >
						<option value="no" <?php echo $enable_autokill['no'] ?>> <?php echo language('_No');?> </option>
						<option value="yes"<?php echo $enable_autokill['yes'] ?>> <?php echo language('_Yes');?> </option>
					</select>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Language','Lauguage');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="iax_language" id="iax_language" >
						<option value="en" > <?php echo language('English');?> </option>						
					</select>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Account Code');?>:
				</span>
				<div class="tab_item_right">
					<input type="text" name="iax_accountcode" id="iax_accountcode" value=<?php echo $iax_accountcode;?>>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Call Token Optional');?>:
				</span>
				<div class="tab_item_right">
					<input type="text" name="iax_calltokenoptional" id="iax_calltokenoptional" value=<?php echo $iax_calltokenoptional;?>>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Description');?>:
				</span>
				<div class="tab_item_right">
					<input type="text" name="iax_description" id="iax_description" value=<?php echo $iax_description;?>>
				</div>
			</div>
		</div>
		
		<div class="content">
			<span class="title">
				<?php echo language('Music On Hold');?>
				
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<?php if(is_show_language_help('Mohsuggest')){ ?>
								<b><?php echo language('Mohsuggest');?>:</b><br/>
								<?php echo language('Mohsuggest help','The \'mohsuggest\' option specifies which music on hold<br>   
										 class to suggest to the peer channel when this channel<br>   
										 places the peer on hold. It may be specified globally or<br>   
										 on a per-user or per-peer basis.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Mohinterpret')){ ?>
								<b><?php echo language('Mohinterpret');?>:</b><br/>
								<?php echo language('Mohinterpret help','You may specify a global default language for users.<br>    
										 This can be specified also on a per-user basis.<br>    
										 If omitted, will fallback to English (en).');?>
							<?php } ?>
						</div>
					</div>
				</div>
			</span>
			
			<div class="tab_item">
				<span>
					<?php echo language('Mohsuggest');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="iax_mohsuggest" id="mohsuggest" >
						<option value="default" > <?php echo language('default');?> </option>						
					</select>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Mohinterpret');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="iax_mohinterpret" id="iax_mohinterpret" >
						<option value="" > <?php echo language('');?> </option>
						<option value="default" <?php echo $iax_mohinterpret['default']?>> <?php echo language('default');?> </option>
						<option value="passthrough" <?php echo $iax_mohinterpret['passthrough']?>> <?php echo language('passthrough');?> </option>
					</select>
				</div>
			</div>
		</div>
		
		<div class="content">
			<span class="title">
				<?php echo language('Codec Settings');?>
				
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<?php if(is_show_language_help('Band Width')){ ?>
								<b><?php echo language('Band Width');?>:</b><br/>
								<?php echo language('Band Width help','Specify bandwidth of low, medium, or high to <br>  
										 control which codecs are used in general.');?>

								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Disallow')){ ?>
								<b><?php echo language('Disallow');?>:</b><br/>
								<?php echo language('Disallow help','Fine tune codecs here using "allow" and <br>  
										 "disallow" clauses with specific codecs.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Allow')){ ?>
								<b><?php echo language('Allow');?>:</b><br/>
								<?php echo language('Allow help','Fine tune codecs here using "allow" and <br>  
										 "disallow" clauses with specific codecs.');?>
										 
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Codec Priority')){ ?>
								<b><?php echo language('Codec Priority');?>:</b><br/>
								<?php echo language('Codec Priority help','Codecpriority controls the codec negotiation of <br>  
										 an inbound IAX2 call. This option is inherited to <br>  
										 all user entities. It can also be defined in each <br>  
										 user entity separately which will override the <br>  
										 setting in general.');?>
							<?php } ?>
						</div>
					</div>
				</div>
			</span>
			
			<div class="tab_item">
				<span>
					<?php echo language('Band Width');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="iax_bandwidth" id="iax_bandwidth" >
						<option value="low" <?php if ($iax_bandwidth=="low") echo "Selected"?>> <?php echo language('low');?> </option>
						<option value="medium" <?php if ($iax_bandwidth=="medium") echo "Selected"?>> <?php echo language('medium');?> </option>
						<option value="high" <?php if ($iax_bandwidth=="high") echo "Selected"?>> <?php echo language('high');?> </option>
					</select>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Disallow');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="iax_disallow" id="iax_disallow" >
						<option value="all" > <?php echo language('all');?> </option>						
					</select>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Allow');?>:
				</span>
				<div class="tab_item_right">
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
							echo language('Priority')." $i:";
					?>
					<select size=1 name="iax_codec_priority<?php echo $i?>" >
						<option value=notuse  <?php echo $iax_codec_priority[$i]['notuse']?> ><?php echo language('Not Used');?></option>
						<option value=ulaw  <?php echo $iax_codec_priority[$i]['ulaw']?> ><?php echo language('G.711 u-law');?></option>
						<option value=alaw  <?php echo $iax_codec_priority[$i]['alaw']?> ><?php echo language('G.711 a-law');?></option>
						<option value=g722  <?php echo $iax_codec_priority[$i]['g722']?> ><?php echo language('G.722');?></option>
						<option value=g723  <?php echo $iax_codec_priority[$i]['g723']?> ><?php echo language('G.723');?></option>
						<option value=g729   <?php echo $iax_codec_priority[$i]['g729']?> ><?php echo language('G.729');?></option>
						<option value=gsm   <?php echo $iax_codec_priority[$i]['gsm']?> ><?php echo language('GSM');?></option>
						
					</select><br><br>			
					<?php
						}
					?>
				</div>
				<div class="clear"></div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Codec Priority');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="iax_codecpriority" id="iax_codecpriority" >
						<option value="host" <?php if ($iax_codecpriority=="host") echo "selected";?>> <?php echo language('host');?> </option>
						<option value="caller" <?php if ($iax_codecpriority=="caller") echo "selected";?>> <?php echo language('caller');?> </option>
						<option value="disabled" <?php if ($iax_codecpriority=="disabled") echo "selected";?>> <?php echo language('disabled');?> </option>
						<option value="reqonly" <?php if ($iax_codecpriority=="reqonly") echo "selected";?>> <?php echo language('reqonly');?> </option>
					</select>
				</div>
			</div>
		</div>
		
		<div class="content">
			<span class="title">
				<?php echo language('Jitter Buffer Settings');?>
				
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<?php if(is_show_language_help('Jitter Buffer')){ ?>
								<b><?php echo language('Jitter Buffer');?>:</b><br/>
								<?php echo language('Jitter Buffer help','Global default as to whether you want the jitter buffer at all.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Force Jitter Buffer')){ ?>
								<b><?php echo language('Force Jitter Buffer');?>:</b><br/>
								<?php echo language('Force Jitter Buffer help','In the ideal world, when we bridge VoIP channels we <br>  
										 don\'t want to do jitterbuffering on the switch, since <br>  
										 the endpoints can each handle this.  However, some <br>  
										 endpoints may have poor jitterbuffers themselves, so <br>  
										 this option will force * to always jitterbuffer, even <br>  
										 in this case.');?>
										 
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Max Jitter Buffers')){ ?>
								<b><?php echo language('Max Jitter Buffers');?>:</b><br/>
								<?php echo language('Max Jitter Buffers help','A maximum size for the jitter buffer.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Resyncthreshold')){ ?>
								<b><?php echo language('Resyncthreshold');?>:</b><br/>
								<?php echo language('Resyncthreshold help','When the jitterbuffer notices a significant change <br>  
										 in delay that continues over a few frames, it will <br>  
										resync, assuming that the change in delay was caused <br>  
										by a timestamping mix-up. The threshold for noticing <br>  
										a change in delay is measured as twice the measured <br>  
										jitter plus this resync threshold.');?>
										
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Max Jitter Interps')){ ?>
								<b><?php echo language('Max Jitter Interps');?>:</b><br/>
								<?php echo language('Max Jitter Interps help','The maximum number of interpolation frames the jitterbuffer<br>  
										 should return in a row. Since some clients do not send <br>  
										 CNG/DTX frames to indicate silence, the jitterbuffer will <br>  
										 assume silence has begun after returning this many <br>  
										 interpolations. This prevents interpolating throughout a <br>  
										 long silence.');?>
										 
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Jitter Target Extra')){ ?>
								<b><?php echo language('Jitter Target Extra');?>:</b><br/>
								<?php echo language('Jitter Target Extra help','Number of milliseconds by which the new jitter <br>  
										 buffer will pad its size. the default is 40, so <br>  
										 without modification, the new jitter buffer will <br>  
										 set its size to the jitter value plus 40 milliseconds. <br>  
										 Increasing this value may help if your network normally <br>  
										 has low jitter, but occasionally has spikes.');?>
							<?php } ?>
						</div>
					</div>
				</div>
			</span>
			
			<div class="tab_item">
				<span>
					<?php echo language('Jitter Buffer');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="enable_jitterbuffer" id="enable_jitterbuffer" >
						<option value="no" <?php echo $enable_jitterbuffer['no'] ?>> <?php echo language('_No');?> </option>
						<option value="yes" <?php echo $enable_jitterbuffer['yes'] ?>> <?php echo language('_Yes');?> </option>
					</select>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Force Jitter Buffer');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="enable_forcejitterbuffer" id="enable_forcejitterbuffer" >
						<option value="no" <?php echo $enable_forcejitterbuffer['yes'] ?>> <?php echo language('_No');?> </option>
						<option value="yes"<?php echo $enable_forcejitterbuffer['yes'] ?>> <?php echo language('_Yes');?> </option>
					</select>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Max Jitter Buffers');?>:
				</span>
				<div class="tab_item_right">
					<input type="text" name="iax_maxjitterbuffer" value="<?php echo $iax_maxjitterbuffer;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Resyncthreshold');?>:
				</span>
				<div class="tab_item_right">
					 <?php echo language('Resyncing can be disabled by setting this parameter to -1');?><input type="text" name="iax_resyncthreshold" value="<?php echo $iax_resyncthreshold;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Max Jitter Interps');?>:
				</span>
				<div class="tab_item_right">
					<input type="text" name="iax_maxjitterinterps" value="<?php echo $iax_maxjitterinterps;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Jitter Target Extra');?>:
				</span>
				<div class="tab_item_right">
					<input type="text" name="iax_jittertargetextra" value="<?php echo $iax_jittertargetextra;?>" />
				</div>
			</div>
		</div>
		
		<div class="content">
			<span class="title">
				<?php echo language('Misc Settings');?>
				
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<?php if(is_show_language_help('IAX2 Thread Count')){ ?>
								<b><?php echo language('IAX2 Thread Count');?>:</b><br/>
								<?php echo language('IAX2 Thread Count help','Establishes the number of iax helper threads to handle I/O.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('IAX2 Max Thread Count')){ ?>
								<b><?php echo language('IAX2 Max Thread Count');?>:</b><br/>
								<?php echo language('IAX2 Max Thread Count help','Establishes the number of extra dynamic threads 
										 that may be spawned to handle I/O.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Max Call Number')){ ?>
								<b><?php echo language('Max Call Number');?>:</b><br/>
								<?php echo language('Max Call Number help','The \'maxcallnumbers\' option limits the amount of call <br>  
										 numbers allowed for each individual remote IP address. <br>   
										 Once an IP address reaches it\'s call number limit, no <br>  
										 more new connections are allowed until the previous ones <br>  
										 close. This option can be used in a peer definition as <br>  
										 well, but only takes effect for the IP of a dynamic peer <br>  
										 after it completes registration.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('MaxCallNumbers_Nonvalidated')){ ?>
								<b><?php echo language('MaxCallNumbers_Nonvalidated');?>:</b><br/>
								<?php echo language('MaxCallNumbers_Nonvalidated help','The \'maxcallnumbers_nonvalidated\' is used to set the <br>  
										 combined number of call numbers that can be allocated <br>  
										 for connections where call token  validation has been <br>  
										 disabled. Unlike the \'maxcallnumbers\' option, this <br>  
										 limit is not separate for each individual IP address.  <br>  
										 Any connection resulting in a non-call token validated <br>  
										 call number being allocated contributes to this limit. <br>   
										 For use cases, see the call token user guide.  This <br>  
										 option\'s default value of 8192 should be sufficient in <br>  
										 most cases.');?>
							<?php } ?>
						</div>
					</div>
				</div>
			</span>
			
			<div class="tab_item">
				<span>
					<?php echo language('IAX2 Thread Count');?>:
				</span>
				<div class="tab_item_right">
					<input type="text" id="iax_iaxthreadcount" name="iax_iaxthreadcount" value="<?php echo $iax_iaxthreadcount;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('IAX2 Max Thread Count');?>:
				</span>
				<div class="tab_item_right">
					<input type="text" name="iax_iaxmaxthreadcount" value="<?php echo $iax_iaxmaxthreadcount;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Max Call Number');?>:
				</span>
				<div class="tab_item_right">
					<input type="text" name="iax_maxcallnumbers" value="<?php echo $iax_maxcallnumbers;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('MaxCallNumbers_Nonvalidated');?>:
				</span>
				<div class="tab_item_right">
					<input type="text" name="iax_maxcallnumbers_nonvalidated" value="<?php echo $iax_maxcallnumbers_nonvalidated;?>" />
				</div>
			</div>
		</div>
		
		<div class="content">
			<span class="title">
				<?php echo language('Quality of Service');?>
				
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<?php if(is_show_language_help('tos')){ ?>
								<b><?php echo language('tos');?>:</b><br/>
								<?php echo language('tos help','Type of Service');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('cos')){ ?>
								<b><?php echo language('cos');?>:</b><br/>
								<?php echo language('cos help','Class of Service');?>
							<?php } ?>
						</div>
					</div>
				</div>
			</span>
			
			<div class="tab_item">
				<span>
					<?php echo language('tos');?>:
				</span>
				<div class="tab_item_right">
					<select size=1 name="iax_tos" >
						<option value='0x00' <?php  echo $iax_tos['normalservice'];?>><?php echo language('Normal Service');?></option> 
						<option value='0x02'  <?php echo $iax_tos['mincost'];?> ><?php echo language('Mincost');?></option>
						<option value='0x04'  <?php echo $iax_tos['highreliability'];?> ><?php echo language('High Reliability');?></option>
						<option value='0x08'  <?php echo $iax_tos['highthroughput'];?> ><?php echo language('High Throughput');?></option>
						<option value='0x10'   <?php echo $iax_tos['lowdelay'];?> ><?php echo language('Low Delay');?></option>
					</select>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('cos');?>:
				</span>
				<div class="tab_item_right">
					<input type="text" name="iax_cos" value="<?php echo $iax_cos;?>" />
				</div>
			</div>
		</div>
		
		<input type="hidden" name="send" id="send" value="" />
		
		<div id="button_save">
		
			<?php if(!$only_view){ ?>
			<button type="submit" class="gen_short_btn float_btn"  onclick="document.getElementById('send').value='Save';"><?php echo language('Save');?></button>
			<?php } ?>
			
		</div>
	
<script type="text/javascript">
$(document).ready(function(){
	onload_func();
});

</script>
</form>
<?php require("/www/cgi-bin/inc/boot.inc");?>