<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");
include_once("/www/cgi-bin/inc/define.inc");
?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript">
function check()
{
	var dialtimeout = document.getElementById("dialtimeout").value;
	
	if(!check_integer(dialtimeout) || dialtimeout < 1 || dialtimeout > 3600) {
		document.getElementById("cdialtimeout").innerHTML = con_str("<?php echo language('Dial Timeout help','Allowed character must be an integer between 1 and 3600.');?>");
		return false;
	}
	
	return true;
}
</script>

<?php
function save_to_extra_global_conf()
{

	if(isset($_POST['dialtimeout'])){
		$dialtimeout = trim($_POST['dialtimeout']);
	}
	
	save_dialtimeout_to_extensions_macro_conf($dialtimeout);
	return true;
}

function set_default_to_extra_global_conf()
{
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk');
	if (!$setok) {
		echo $aql->get_error();
		return;
	}
	$sms_conf_path = '/etc/asterisk/extra-global.conf';
	$hlock = lock_file($sms_conf_path);
	if(!$aql->open_config_file($sms_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	$exist_array = $aql->query("select * from extra-global.conf");

	if(isset($exist_array['channels']['dialtimeout'])) {
		$aql->assign_editkey('channels','dialtimeout','100');
	} else {
		$aql->assign_append('channels','dialtimeout','100');
	}
	
	if (!$aql->save_config_file('extra-global.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	unlock_file($hlock);
	return true;
}
function save_dialtimeout_to_extensions_macro_conf($dialtimeout)
{
	$extensions_macro_conf = "/etc/asterisk/extensions_macro.conf";

	if(!file_exists($extensions_macro_conf))
		return;

	$hlock = lock_file($extensions_macro_conf);
	$handle = fopen($extensions_macro_conf,"r");
	
	$res = file_get_contents($extensions_macro_conf);
	
	if(strstr($res, "s,n,Set(DIALTIMEOUT=")){
		$write_str = '';
		while (!feof($handle)) {
			$line = fgets($handle);

			if(strstr($line, "Set(DIALTIMEOUT=")) {
				$line = "exten => s,n,Set(DIALTIMEOUT=$dialtimeout)\n";
			}

			$write_str .= $line;
		}
	}else{
		$write_str = '';
		while (!feof($handle)) {
			$line = fgets($handle);

			if(strstr($line, "s,n,Set(MAX=128)")) {
				$line = "exten => s,n,Set(MAX=128)\nexten => s,n,Set(DIALTIMEOUT=$dialtimeout)\n";
			}
			
			if(strstr($line, "s,n(dial),Dial(\${DIALSTR})")){
				$line = "exten => s,n(dial),Dial(\${DIALSTR},\${DIALTIMEOUT})\n";
			}

			$write_str .= $line;
		}
	}
	fclose($handle);
	
	//Write
	if($write_str != '') {
		$handle = fopen($extensions_macro_conf,"w");
		fwrite($handle,$write_str);
		fclose($handle);
	}
	unlock_file($hlock);
}

if($_POST && isset($_POST['send'])) {
	if($_POST['send'] == 'Save') {
		if($only_view){
			return false;
		}
		
		if(save_to_extra_global_conf()){
			wait_apply("exec", "asterisk -rx \"dialplan reload\" > /dev/null 2>&1 &");
			
			save_user_record("","ROUTING->Advanced:Save");
		}
	} elseif ($_POST['send'] == 'set_default'){
		set_default_to_extra_global_conf();
	}
}


$extensions_macro_conf = "/etc/asterisk/extensions_macro.conf";

if(!file_exists($extensions_macro_conf))
	return;

$hlock = lock_file($extensions_macro_conf);
$handle = fopen($extensions_macro_conf,"r");

$write_str = '';
$dialtimeout = '';
while (!feof($handle)) {
	$line = fgets($handle);

	if(strstr($line, "Set(DIALTIMEOUT=")) {
		$tmp_array = explode("=", $line);
		$dialtimeout = substr($tmp_array[2], 0, -2);
		break;
	}
}
fclose($handle);
?>

<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	
	<div class="content">
		<span class="title"><?php echo language('General');?></span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Dial Timeout');?>:
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<b><?php echo language('Dial Timeout');?>:</b><br/>
							<?php echo language('Dial Timeout help','Dial Timeout');?>
						</div>
					</div>
				</div>
			</span>
			<div class="tab_item_right">
				<span id="cdialtimeout"></span>
				<input id="dialtimeout" type="text" name="dialtimeout" style="" value="<?php echo $dialtimeout; ?>" />&nbsp;(<?php echo language('Second'); ?>)&nbsp;
			</div>
		</div>
	</div>

	<input type="hidden" name="send" id="send" value="" />
	
	<div id="button_save">
		<?php if(!$only_view){ ?>
		<button type="submit" onclick="document.getElementById('send').value='Save';return check();"><?php echo language('Save');?></button>
		<?php } ?>
	</div>
	
</form>

<script type="text/javascript">
function set_default(){
	if(confirm("Are you sure to restore the default settings?")) {
		return true;
	}else{
		return false;
	}
}
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>