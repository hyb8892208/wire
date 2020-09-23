<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/check.inc");
include_once("/www/cgi-bin/inc/define.inc");
?>

<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />

<script type="text/javascript" src="/js/functions.js">
</script>

<script type="text/javascript" src="/js/check.js">
</script>

<script type="text/javascript" src="/js/float_btn.js"></script>
<script type="text/javascript">

function mnp_change(sw)
{
	if(!sw){
		$("#mnp_url").val("");
		$("#mnp_timeout").val("");
	}
	document.getElementById('mnp_url').disabled = !sw;
	document.getElementById('mnp_timeout').disabled = !sw;
}

function check()
{
	var mnp_enable = document.getElementById("mnp_enable").checked;
	var mnp_url = document.getElementById("mnp_url").value;
	var mnp_timeout = document.getElementById("mnp_timeout").value;

	document.getElementById("cmnp_url").innerHTML = '';
	document.getElementById("cmnp_timeout").innerHTML = '';

	if(mnp_enable){
        if (!check_url(mnp_url)){
			document.getElementById("cmnp_url").innerHTML = con_str('<?php echo language('js check URL','Please check to enter the correct URL address.');?>');
			return false;
        }
		
		if(mnp_timeout <= 0) {
			document.getElementById("cmnp_timeout").innerHTML = con_str('<?php echo htmlentities(language('MNP timeout help','Allowed character must be  greater than zero.'));?>');
			return false;
		}
		
		return true;
	}
	return true;
}

function check_url(gen_url){
	/*gen_url = gen_url.replace (/\$\{.*\}/,'name');
	var strRegex = '^((https|http|ftp|rtsp|mms)?://)'
	+ '?(([0-9a-z_!~*\'().&=+$%-]+: )?[0-9a-z_!~*\'().&=+$%-]+@)?' //ftp??user@
	+ '(([0-9]{1,3}.){3}[0-9]{1,3}' // IP??????URL- 199.194.52.184
	+ '|' // ???¨ªIP??DOMAIN?¡§?¨°????
	+ '([0-9a-z_!~*\'()-]+.)*' // ?¨°??- www.
	+ '([0-9a-z][0-9a-z-]{0,61})?[0-9a-z].' // ?????¨°??
	+ '[a-z]{2,6})' // first level domain- .com or .museum
	+ '(:[0-9]{1,4})?' // ????- :80
	+ '((/?)|' // a slash isn't required if there is no file name
	+ '(/[0-9a-z_!~*\'().;?:@&=+$,%#-]+)+/?)$';*/
	var strRegex = '^[a-zA-z0-9@&\{\}/:\?#%!=\*()\\-+"\',<>.\$]+$';
	var re=new RegExp(strRegex);
	return re.test(gen_url);
}
</script>

<?php

function savemnpconf()
{
/*
#mnp.conf
mnp_enable=on
mnp_server=baidu.com
mnp_port=8080
mnp_user=newperson
mnp_password=1234 
mnp_timeout=5
mnp_app=/my_tools/mnp_get
*/
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk/gw');
	if (!$setok) {
		echo $aql->get_error();
		return false;
	}
	$conf_path = '/etc/asterisk/gw/mnp.conf';
	$hlock = lock_file($conf_path);
        
	if(!file_exists($conf_path)) {
		fclose(fopen($conf_path,"w"));
	}
	
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	
	$exist_array = $aql->query("select * from mnp.conf");

	if(!isset($exist_array['general'])) {
		$aql->assign_addsection('general','');
	}
	if (isset($_POST['mnp_enable']) && isset($_POST['mnp_url']) && isset($_POST['mnp_timeout']) ){
		$mnp_timeout = trim($_POST['mnp_timeout']);
		$mnp_url = trim($_POST['mnp_url']);
		$mnp_manipulation = trim($_POST['mnp_manipulation']);
		
		if(isset($exist_array['general']['mnp_enable'])) {
			$aql->assign_editkey('general','mnp_enable','on');
		} else {
			$aql->assign_append('general','mnp_enable','on');
		}
		
		if(isset($exist_array['general']['mnp_timeout'])) {
			$aql->assign_editkey('general','mnp_timeout',$mnp_timeout);
		} else {
			$aql->assign_append('general','mnp_timeout',$mnp_timeout);
		}
		
		if(isset($exist_array['general']['mnp_url'])) {
			$aql->assign_editkey('general','mnp_url',$mnp_url);
		} else {
			$aql->assign_append('general','mnp_url',$mnp_url);
		}

		if(isset($exist_array['general']['mnp_manipulation'])) {
			$aql->assign_editkey('general','mnp_manipulation',$mnp_manipulation);
		} else {
			$aql->assign_append('general','mnp_manipulation',$mnp_manipulation);
		}
	} else {
		if(isset($exist_array['general']['mnp_enable'])) {
			$aql->assign_editkey('general','mnp_enable','off');
		} else {
			$aql->assign_append('general','mnp_enable','off');
		}
		
		if(isset($exist_array['general']['mnp_timeout'])) {
			$aql->assign_editkey('general','mnp_timeout','');
		} else {
			$aql->assign_append('general','mnp_timeout','');
		}
		
		if(isset($exist_array['general']['mnp_url'])) {
			$aql->assign_editkey('general','mnp_url','');
		} else {
			$aql->assign_append('general','mnp_url','');
		}

		if(isset($exist_array['general']['mnp_manipulation'])) {
			$aql->assign_editkey('general','mnp_manipulation','');
		} else {
			$aql->assign_append('general','mnp_manipulation','');
		}
	}
	
	if (!$aql->save_config_file('mnp.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	return true;
}

if($_POST && isset($_POST['send']) && $_POST['send'] == 'Save') {
	if(savemnpconf()){
		save_endpoints_to_sips(); 
		save_routings_to_extensions();
		wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");
	}
}

$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$res = $aql->query("select * from mnp.conf");
$mnp_enable = '';

if(isset($res['general']['mnp_enable'])) {
	if(is_true(trim($res['general']['mnp_enable']))) {
		$mnp_enable = 'checked';
	}
}
	
if(isset($res['general']['mnp_timeout'])) {
	$mnp_timeout = trim($res['general']['mnp_timeout']);
} else {
	$mnp_timeout = "5";
}

if(isset($res['general']['mnp_url'])) {
	$mnp_url = trim($res['general']['mnp_url']);
} else {
	$mnp_url = '';
}

if(isset($res['general']['mnp_manipulation'])) {
	$mnp_manipulation = trim($res['general']['mnp_manipulation']);
} else {
	$mnp_manipulation = 'after';
}

if($mnp_manipulation == 'before') {
	$mnp_manipulation_checked['after'] = '';
	$mnp_manipulation_checked['before'] = 'checked';
} else {
	$mnp_manipulation_checked['after'] = 'checked';
	$mnp_manipulation_checked['before'] = '';
}

?>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('MNP Settings');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tedit" >
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('MNP Check Enable');?>:
					<span class="showhelp">
					<?php
						$help = "ON(enabled),OFF(disabled)";
						echo language('Enable help', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				<input type="checkbox" id="mnp_enable" name="mnp_enable" <?php  echo $mnp_enable ?> onchange="mnp_change(this.checked)" />
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('MNP URL');?>:
					<span class="showhelp">
					<?php
						$help = 'Please check to enter the correct URL address.';
						echo language('MNP url help', $help);
					?>
					</span>
				</div>
			</th>
			<td>
				<input id="mnp_url" type="password" name="mnp_url" style="width: 800px;" value="<?php echo $mnp_url; ?>" /><br><span id="cmnp_url"></span>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('MNP Timeout');?>:
					<span class="showhelp">
					<?php
						echo htmlentities(language('MNP timeout help',"Allowed character must be  greater than zero."));
					?>
					</span>
				</div>
			</th>
			<td>
				<input id="mnp_timeout" type="text" name="mnp_timeout" style="width: 250px;" value="<?php echo $mnp_timeout; ?>" /><span id="cmnp_timeout"></span>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Manipulation Choice');?>:
					<span class="showhelp">
					<?php
						echo htmlentities(language('Manipulation Choice help'));
					?>
					</span>
				</div>
			</th>
			<td>
				<input name="mnp_manipulation" id="mnp_manipulation" type="radio" value="after" <?php echo $mnp_manipulation_checked['after'] ?> > <?php echo language('Route calls after manipulation');?>
				&nbsp;&nbsp;&nbsp;
				<input name="mnp_manipulation" id="mnp_manipulation" type="radio" value="before" <?php echo $mnp_manipulation_checked['before'] ?> > <?php echo language('Route calls before manipulation');?>
			</td>
		</tr>
	</table>
	<br>

	<input type="hidden" name="send" id="send" value="" />
	<table id="float_btn" class="float_btn">
		<tr id="float_btn_tr" class="float_btn_tr">
			<td>
				<input type="submit"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" />
			</td>
		</tr>
	</table>
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td width="20px">
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" />
			</td>
		</tr>
	</table>
	</form>

<script type="text/javascript">
function onload_func()
{
<?php
	if($mnp_enable != '') {
		echo "mnp_change(true);\n";
	} else {
		echo "mnp_change(false);\n";
	}
?>
}

$(document).ready(function (){ 
	$("#mnp_enable").iButton();
	onload_func();
}); 
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>
<div id="float_btn1" class="float_btn1 sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()" >
</div>