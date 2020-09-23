<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
require_once("/www/cgi-bin/inc/aql.php");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/network_factory.inc");


function save_license_open()
{
	$conf_file = "/etc/asterisk/gw/system_type.conf";
	if(isset($_POST['license_mode'])){
		$__license_enable__ = 'on';
		$conf_array['license']['license_mode'] = $_POST['license_mode'];
		wait_apply("exec","");
		return modify_conf($conf_file, $conf_array);
	} else {
		$__license_enable__ = 'off';
	}
	
	$write = "[license]\n";
	$write .= "license_mode=$__license_enable__\n";
	$hlock=lock_file($conf_file);
	$fh = fopen($conf_file,"w");
	fwrite($fh,$write);
	fclose($fh);
	unlock_file($hlock);

	wait_apply("exec","");
}

if($_POST && isset($_POST['send'])) {
	if($_POST['send'] == 'Save') {
		save_license_open();
	}
}

$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$res = $aql->query('select * from system_type.conf');
if(isset($res['license']['license_mode'])) {
	$license_mode = trim($res['license']['license_mode']);
} else {
	$license_mode = 'off';
}
if($license_mode == 'on'){
	$license_check = 'checked';
}else{
	$license_check = '';
}

?>
	<form enctype="multipart/form-data" action="<?php echo $_SERVER['PHP_SELF'] ?>" method="post">
		<table width="100%" class="tedit" >
			<tr>		
				<th>
					<div class="helptooltips">
						<?php echo language('License Enable');?>:
						<span class="showhelp">
						<?php echo language('License Enable help','ON(enabled), OFF(disabled)');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="license_mode" <?php echo $license_check; ?> />
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