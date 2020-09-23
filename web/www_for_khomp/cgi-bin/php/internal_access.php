<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
require_once("/www/cgi-bin/inc/aql.php");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/network_factory.inc");


function save_system_demo_conf()
{
	$conf_file = "/etc/asterisk/gw/system_type.conf";
	if(isset($_POST['demo_mode'])){
		$__demo_enable__ = 'on';
		$conf_array['demo']['demo_enable'] = $_POST['demo_mode'];
		wait_apply("exec","");
		return modify_conf($conf_file, $conf_array);
	} else {
		$__demo_enable__ = 'off';
	}
	
	$write = "[demo]\n";
	$write .= "demo_enable=$__demo_enable__\n";
	$hlock=lock_file($conf_file);
	$fh = fopen($conf_file,"w");
	fwrite($fh,$write);
	fclose($fh);
	unlock_file($hlock);

	wait_apply("exec","");
}

if($_POST && isset($_POST['send'])) {
	if($_POST['send'] == 'Save') {
		save_system_demo_conf();
		$apply_and_refresh = true;
	}
}

$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$res = $aql->query('select * from system_type.conf');
if(isset($res['demo']['demo_enable'])) {
	$demo_mode = trim($res['demo']['demo_enable']);
} else {
	$demo_mode = 'off';
}
if($demo_mode == 'on'){
	$demo_check = 'checked';
}
else{
	$demo_check = '';
}

?>
	<form enctype="multipart/form-data" action="<?php echo $_SERVER['PHP_SELF'] ?>" method="post">
		<table width="100%" class="tedit" >
			<tr>		
				<th>
					<div class="helptooltips">
						<?php echo language('Demo Enable');?>:
						<span class="showhelp">
						<?php echo language('Demo Enable help','ON(enabled), OFF(disabled)');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="demo_mode" <?php echo $demo_check ?> />
				</td>
			</tr>
		</table>
		
		<div id="newline"></div>
		
		<input type="hidden" name="send" id="send" value="" />
		<input type="submit" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';"/>
	</form>
<div>
<?php require("/www/cgi-bin/inc/boot.inc");?>
</div>