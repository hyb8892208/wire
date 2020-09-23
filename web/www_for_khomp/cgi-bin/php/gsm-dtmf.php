<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");
include_once("/www/cgi-bin/inc/config.inc");
?>


<?php
function save_to_dsp_conf()
{
/* 
/etc/asterisk/dsp.conf
[default] 
dtmf_detect_flag=
dtmf_normal_twist=
dtmf_reverse_twist=
relax_dtmf_normal_twist=
relax_dtmf_reverse_twist=
dtmf_relative_peak_row=
dtmf_relative_peak_col=
dtmf_hits_to_begin=
dtmf_misses_to_end=
*/
	$write = "[default]\n";	

	if(isset($_POST['dtmf_detect_flag']) && $_POST['dtmf_detect_flag'] == 'on'){
		$write .= "dtmf_detect_flag=1\n";
	}else{
		$write .= "dtmf_detect_flag=0\n";
	}
	if(isset($_POST['relax_dtmf_normal_twist'])) {
		$write .= "relax_dtmf_normal_twist=".trim($_POST['relax_dtmf_normal_twist'])."\n";
	}else{
		$write .= "relax_dtmf_normal_twist=\n";
	}
	if(isset($_POST['relax_dtmf_reverse_twist'])) {
		$write .= "relax_dtmf_reverse_twist=".trim($_POST['relax_dtmf_reverse_twist'])."\n";
	}else{
		$write .= "relax_dtmf_reverse_twist=\n";
	}
	if(isset($_POST['dtmf_relative_peak_row'])) {
		$write .= "dtmf_relative_peak_row=".trim($_POST['dtmf_relative_peak_row'])."\n";
	}else{
		$write .= "dtmf_relative_peak_row=\n";
	}
	if(isset($_POST['dtmf_relative_peak_col'])) {
		$write .= "dtmf_relative_peak_col=".trim($_POST['dtmf_relative_peak_col'])."\n";
	}else{
		$write .= "dtmf_relative_peak_col=\n";
	}
	if(isset($_POST['dtmf_hits_to_begin'])) {
		$write .= "dtmf_hits_to_begin=".trim($_POST['dtmf_hits_to_begin'])."\n";
	}else{
		$write .= "dtmf_hits_to_begin=\n";
	}
	if(isset($_POST['dtmf_misses_to_end'])) {
		$write .= "dtmf_misses_to_end=".trim($_POST['dtmf_misses_to_end'])."\n";
	}else{
		$write .= "dtmf_misses_to_end=\n";
	}

	$conf_file = "/etc/asterisk/dsp.conf";
	if(!file_exists($conf_file)) {
		fclose(fopen($conf_file,"w"));
	}

	$flock = lock_file($conf_file);
	$ret = file_put_contents($conf_file, $write);
	unlock_file($flock);

	if($ret == strlen($write)){
		return true;
	} else {
		return false;
	}
}

function save_to_chan_extra_conf()
{
/*
/etc/asterisk/chan_extra.conf
[channels]
usecallerid=yes
callwaiting=yes
usecallingpres=yes
callwaitingcallerid=yes
threewaycalling=yes
transfer=yes
canpark=yes
cancallforward=yes
callreturn=yes
echocancel=yes
echocancelwhenbridged=yes
group=1
callgroup=1
pickupgroup=1
relaxdtmf=yes			//this is the key
*/
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk');
	if (!$setok) {
		echo $aql->get_error();
		return;
	}

	$conf_path = '/etc/asterisk/extra-global.conf';
	$hlock = lock_file($conf_path);

	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}

	$config_array = $aql->query("select * from extra-global.conf");

	if(!isset($config_array['channels'])) {
		$aql->assign_addsection('channels','');
	}

	if(isset($config_array['channels']['relaxdtmf'])) {
		$aql->assign_editkey('channels','relaxdtmf','yes');
	} else {
		$aql->assign_append('channels','relaxdtmf','yes');
	}

	if (!$aql->save_config_file('extra-global.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	unlock_file($hlock);
	return true;
/*	$config_file = "/etc/asterisk/chan_extra.conf";
	$config_array = get_config($config_file);
	print_r($config_array);
	$config_array['channels']['relaxdtmf'] = 'yes';
	set_config($config_file, $config_array);
	return true;
*/
}

if($_POST && isset($_POST['send']) && $_POST['send'] == 'Save') {
	if($only_view){
		return false;
	}
	
	if(save_to_dsp_conf()){
		if(save_to_chan_extra_conf()){
			wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");
			
			save_user_record("","MODULE->DTMF:Save");
		}
	}
}
?>

<?php
/* get dsp.conf data */

$dtmf_detect_flag="";
$dtmf_normal_twist="";
$dtmf_reverse_twist="";
$relax_dtmf_normal_twist="";
$relax_dtmf_reverse_twist="";
$dtmf_relative_peak_row="";
$dtmf_relative_peak_col="";
$dtmf_hits_to_begin="";
$dtmf_misses_to_end="";

$config_file = "/etc/asterisk/dsp.conf";
$config_array_org = get_config($config_file, "default");
$config_array = $config_array_org['default'];
if(isset($config_array['dtmf_detect_flag']))$dtmf_detect_flag = $config_array['dtmf_detect_flag'];
if(isset($config_array['relax_dtmf_normal_twist']))$relax_dtmf_normal_twist = $config_array['relax_dtmf_normal_twist'];
if(isset($config_array['relax_dtmf_reverse_twist']))$relax_dtmf_reverse_twist = $config_array['relax_dtmf_reverse_twist'];
if(isset($config_array['dtmf_relative_peak_row']))$dtmf_relative_peak_row = $config_array['dtmf_relative_peak_row'];
if(isset($config_array['dtmf_relative_peak_col']))$dtmf_relative_peak_col = $config_array['dtmf_relative_peak_col'];
if(isset($config_array['dtmf_hits_to_begin']))$dtmf_hits_to_begin = $config_array['dtmf_hits_to_begin'];
if(isset($config_array['dtmf_misses_to_end']))$dtmf_misses_to_end = $config_array['dtmf_misses_to_end'];
	
?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript">

function check_dtmf_float(id)
{
	var obj = document.getElementById(id);
	obj.value = obj.value.replace(/[^.\d]*/g,'');

	val = $("#"+id).attr("value");
	
	if((!check_float(val)) || val==0) {
		$("#c"+id).html(con_str("<?php echo language('js check dtmf float 1','Please input floating number');?>"));
		return false;
	} else if(val > 100) {
		$("#c"+id).html(con_str("<?php echo language('js check dtmf float 2','The input should be less than 100.0');?>"));
		return false;
	} else {
		var log_val =10*(Math.log(val) * Math.LOG10E);
		log_val = log_val.toString().replace(/^(\d+\.\d{2})\d*$/,"$1");
		$("#c"+id).html(log_val+"dB");
	}

}

function check_dtmf_integer(id)
{
	var obj = document.getElementById(id);
	obj.value = obj.value.replace(/[^\d]*/g,'');

	val = $("#"+id).attr("value");
	
	if(!check_integer(val) || val==0) {
		$("#c"+id).html(con_str("<?php echo language('js check dtmf integer 1','Please input integer data');?>"));
		return false;
	} else if(val > 100) {
		$("#c"+id).html(con_str("<?php echo language('js check dtmf integer 2','The input should be less than 100');?>"));
		return false;
	} else {
		$("#c"+id).html("");
	}
}

function check_all()
{
	if(check_dtmf_float("relax_dtmf_normal_twist")==false)return false;
	if(check_dtmf_float("relax_dtmf_reverse_twist")==false)return false;
	if(check_dtmf_float("dtmf_relative_peak_row")==false)return false;
	if(check_dtmf_float("dtmf_relative_peak_col")==false)return false;
	if(check_dtmf_integer("dtmf_hits_to_begin")==false)return false;
	if(check_dtmf_integer("dtmf_misses_to_end")==false)return false;

	return true;
}

function dtmf_setting(value)
{
	if(value == "default") {
		var setting = new Array("6.31","3.98","6.3","6.3","2","3");
	} else if(value == "suggest1") {
		var setting = new Array("8.61","13.20","3.56","5.10","2","6");
	} else {
		var setting = new Array(<?php echo "\"$relax_dtmf_normal_twist\",\"$relax_dtmf_reverse_twist\",\"$dtmf_relative_peak_row\",\"$dtmf_relative_peak_col\",\"$dtmf_hits_to_begin\",\"$dtmf_misses_to_end\"";?>);
	}

	$("#relax_dtmf_normal_twist").attr("value", setting[0]);
	$("#relax_dtmf_reverse_twist").attr("value", setting[1]);
	$("#dtmf_relative_peak_row").attr("value", setting[2]);
	$("#dtmf_relative_peak_col").attr("value", setting[3]);
	$("#dtmf_hits_to_begin").attr("value", setting[4]);
	$("#dtmf_misses_to_end").attr("value", setting[5]);	

	check_all();

}

function dtmf_detect_flag_change(){
	var sw = document.getElementById('dtmf_detect_flag').checked;
	
	if(sw){
		$(".dtmf_flag").show();
	}else{
		$(".dtmf_flag").hide();
	}
}

</script>
<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<div class="content">
		<span class="title">
			<?php echo language('DTMF Detection Settings');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('DTMF Detect Flag')){ ?>
							<b><?php echo language('DTMF Detect Flag');?>:</b><br/>
							<?php echo language('DTMF Detect Flag help','DTMF Detect Flag');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Reference Value')){ ?>
							<b><?php echo language('Reference Value');?>:</b><br/>
							<?php echo language('Reference Value help','Reference Value');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Relax DTMF Normal Twist')){ ?>
							<b><?php echo language('Relax DTMF Normal Twist');?>:</b><br/>
							<?php echo language('Relax DTMF Normal Twist help','Relax DTMF Normal Twist');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Relax DTMF Reverse Twist')){ ?>
							<b><?php echo language('Relax DTMF Reverse Twist');?>:</b><br/>
							<?php echo language('Relax DTMF Reverse Twist help','Relax DTMF Reverse Twist');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('DTMF Relative Peak Row')){ ?>
							<b><?php echo language('DTMF Relative Peak Row');?>:</b><br/>
							<?php echo language('DTMF Relative Peak Row help','DTMF Relative Peak Row');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('DTMF Relative Peak Col')){ ?>
							<b><?php echo language('DTMF Relative Peak Col');?>:</b><br/>
							<?php echo language('DTMF Relative Peak Col help', 'DTMF Relative Peak Col');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('DTMF Hits Begin')){ ?>
							<b><?php echo language('DTMF Hits Begin');?>:</b><br/>
							<?php echo language('DTMF Hits Begin help','DTMF Hits Begin');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('DTMF Misses End')){ ?>
							<b><?php echo language('DTMF Misses End');?>:</b><br/>
							<?php echo language('DTMF Misses End help','DTMF Misses End');?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('DTMF Detect Flag');?>:
			</span>
			<div class="tab_item_right">
				<span><input type="checkbox" id="dtmf_detect_flag" name="dtmf_detect_flag" <?php if($dtmf_detect_flag==1)echo 'checked'; ?>  onchange="dtmf_detect_flag_change()"  /></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Reference Value');?>:
			</span>
			<div class="tab_item_right">
				<select onchange="dtmf_setting(this.value)">
					<option value="custom" selected><?php echo language('Custom');?></option>
					<option value="default" ><?php echo language('Default');?></option>
					<option value="suggest1"><?php echo language('Suggest');?> 1</option>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Relax DTMF Normal Twist');?>:
			</span>
			<div class="tab_item_right">
				<span id="crelax_dtmf_normal_twist"></span>
				<input type="text" id="relax_dtmf_normal_twist" name="relax_dtmf_normal_twist" value="<?php echo $relax_dtmf_normal_twist?>" oninput="check_dtmf_float(this.id);" onkeyup="check_dtmf_float(this.id);">
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Relax DTMF Reverse Twist');?>:
			</span>
			<div class="tab_item_right">
				<span id="crelax_dtmf_reverse_twist"></span>
				<input type="text" id="relax_dtmf_reverse_twist" name="relax_dtmf_reverse_twist" value="<?php echo $relax_dtmf_reverse_twist?>" oninput="check_dtmf_float(this.id);" onkeyup="check_dtmf_float(this.id);">
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('DTMF Relative Peak Row');?>:
			</span>
			<div class="tab_item_right">
				<span id="cdtmf_relative_peak_row"></span>
				<input type="text" id="dtmf_relative_peak_row" name="dtmf_relative_peak_row" value="<?php echo $dtmf_relative_peak_row?>" oninput="check_dtmf_float(this.id);" onkeyup="check_dtmf_float(this.id);">
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('DTMF Relative Peak Col');?>:
			</span>
			<div class="tab_item_right">
				<span id="cdtmf_relative_peak_col"></span>
				<input type="text" id="dtmf_relative_peak_col" name="dtmf_relative_peak_col" value="<?php echo $dtmf_relative_peak_col?>" oninput="check_dtmf_float(this.id);" onkeyup="check_dtmf_float(this.id);">
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('DTMF Hits Begin');?>:
			</span>
			<div class="tab_item_right">
				<span id="cdtmf_hits_to_begin"></span>
				<input type="text" id="dtmf_hits_to_begin" name="dtmf_hits_to_begin" value="<?php echo $dtmf_hits_to_begin?>" oninput="check_dtmf_integer(this.id);" onkeyup="check_dtmf_integer(this.id);">
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('DTMF Misses End');?>:
			</span>
			<div class="tab_item_right">
				<span id="cdtmf_misses_to_end"></span>
				<input type="text" id="dtmf_misses_to_end" name="dtmf_misses_to_end" value="<?php echo $dtmf_misses_to_end?>" oninput="check_dtmf_integer(this.id);" onkeyup="check_dtmf_integer(this.id);">
			</div>
		</div>
	</div>
	
	<input type="hidden" name="send" id="send" value="" />
	
	<div id="button_save">
		<?php if(!$only_view){ ?>
		<button type="submit" onclick="document.getElementById('send').value='Save';return check_all();" ><?php echo language('Save');?></button>
		<?php } ?>
	</div>
</form>

<script type="text/javascript">
	$(document).ready(function(){check_all();});
	dtmf_detect_flag_change();
</script>
<?php require("/www/cgi-bin/inc/boot.inc");?>
