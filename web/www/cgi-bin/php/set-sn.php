<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");

$aql = new aql();
$aql->set('basedir','/data/info/');
$res = $aql->query("select * from device_info");
$old_sn = $res['[unsection]']['serialnumber'];

if($_POST && isset($_POST['send']) && $_POST['send'] == 'Save'){
	$predis_db = new Predis\Client($redis_server);
	
	$sn = $_POST['serialnumber'];
	$predis_db->set('local.system.serialnumber', $sn);
	
	if(isset($res['[unsection]']['serialnumber'])){
		$aql->assign_editkey('[unsection]','serialnumber',$sn);
	}else{
		$aql->assign_append('[unsection]','serialnumber',$sn);
	}
	
	if (!$aql->save_config_file('device_info')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
}
?>
<script type="text/javascript" src="/js/check.js"></script>
<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Serial Number Settings');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	
	<table width="100%" class="tedit" >
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Serial Number');?>:
					<span class="showhelp">
					<?php echo language('Serial Number help','Serial Number');?>
					</span>
				</div>
			</th>
			<td>
				<input id="serialnumber" type="text" name="serialnumber" value="<?php if($_POST['serialnumber'] == ''){echo $old_sn;}else{echo $_POST['serialnumber'];}?>" />
				<span id="cserialnumber"></span>
			</td>
		</tr>
	</table>
	<br/>
	<input type="hidden" name="send" id="send" value="" />
	<table id="float_btn" class="float_btn">
		<tr id="float_btn_tr" class="float_btn_tr">
			<td>
				<input type="submit"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" />
			</td>
		</tr>
	</table>
</form>

<script>
function check(){
	var serialnumber = document.getElementById("serialnumber").value;
	
	document.getElementById('cserialnumber').innerHTML = "";
	if(serialnumber == ""){
		document.getElementById('cserialnumber').innerHTML = con_str("<?php echo language('js check null');?>");
		return false;
	}
}
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>