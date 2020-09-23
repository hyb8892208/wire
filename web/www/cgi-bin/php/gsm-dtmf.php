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
	if(save_to_dsp_conf()){
		if(save_to_chan_extra_conf()){
			wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");
			
			if($__deal_cluster__){
				$cluster_info = get_cluster_info();
				if($cluster_info['mode'] == 'master') {
					for($b=2; $b<=$__BRD_SUM__; $b++) {
						if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
							$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
							set_slave_file($slaveip,"/etc/asterisk/dsp.conf","/etc/asterisk/dsp.conf");
							set_slave_file($slaveip,"/etc/asterisk/extra-global.conf","/etc/asterisk/extra-global.conf");
							$data = "syscmd:asterisk -rx \"core reload\" > /dev/null 2>&1";
							wait_apply("request_slave", $slaveip, $data);
						}
					}
				}
			}
			
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

<!--// load jQuery and the jQuery iButton Plug-in //--> 
<!--<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.3.2/jquery.min.js"></script> -->
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 

<!--// load the iButton CSS stylesheet //--> 
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/float_btn.js"></script>
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
	$(":checkbox").iButton();
	var sw = document.getElementById('dtmf_detect_flag').checked;
	
	if(sw){
		$(".dtmf_flag").show();
	}else{
		$(".dtmf_flag").hide();
	}
}

</script>
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg" style="width:auto;"><?php echo language('DTMF Detection Settings');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<table id="dtmf_tab" width="100%" class="tedit">
		<tr>
            <th>
                <div class="helptooltips">
                    <?php echo language('DTMF Detect Flag');?>:
                    <span class="showhelp">
                        <?php echo language('DTMF Detect Flag help');?>
                    </span>
                </div>
            </th>
			<td colspan=" 10">
				<input type="checkbox" id="dtmf_detect_flag" name="dtmf_detect_flag" <?php if($dtmf_detect_flag==1)echo 'checked'; ?>  onchange="dtmf_detect_flag_change()"  />
			</td>
		</tr>
		<tr class="dtmf_flag">
			<th>
				<div class="helptooltips">
					<?php echo language('Reference Value');?>:
					<span class="showhelp">
						<?php echo language('Reference Value help');?>
					</span>
				</div>
			</th>
			<td >
				<select onchange="dtmf_setting(this.value)">
					<option value="custom" selected><?php echo language('Custom');?></option>
					<option value="default" ><?php echo language('Default');?></option>
					<option value="suggest1"><?php echo language('Suggest');?> 1</option>
				</select>
			</td>
		</tr>
		<tr class="dtmf_flag">
			<th>
				<div class="helptooltips">
					<?php echo language('Relax DTMF Normal Twist');?>:
					<span class="showhelp">
						<?php echo language('Relax DTMF Normal Twist help');?>
					</span>
				</div>
			</th>
			<td >
				<input type="text" id="relax_dtmf_normal_twist" name="relax_dtmf_normal_twist" value="<?php echo $relax_dtmf_normal_twist?>" oninput="check_dtmf_float(this.id);" onkeyup="check_dtmf_float(this.id);">
				<span id="crelax_dtmf_normal_twist"></span>
			</td>
		</tr>
		<tr class="dtmf_flag">
			<th>
				<div class="helptooltips">
					<?php echo language('Relax DTMF Reverse Twist');?>:
					<span class="showhelp">
					<?php echo language('Relax DTMF Reverse Twist help');?>
					</span>
				</div>
			</th>
			<td >
				<input type="text" id="relax_dtmf_reverse_twist" name="relax_dtmf_reverse_twist" value="<?php echo $relax_dtmf_reverse_twist?>" oninput="check_dtmf_float(this.id);" onkeyup="check_dtmf_float(this.id);">
				<span id="crelax_dtmf_reverse_twist"></span>
			</td>
		</tr>
		<tr class="dtmf_flag">
			<th>
				<div class="helptooltips">
					<?php echo language('DTMF Relative Peak Row');?>:
					<span class="showhelp">
					<?php echo language('DTMF Relative Peak Row help');?>
					</span>
				</div>
			</th>
			<td >
				<input type="text" id="dtmf_relative_peak_row" name="dtmf_relative_peak_row" value="<?php echo $dtmf_relative_peak_row?>" oninput="check_dtmf_float(this.id);" onkeyup="check_dtmf_float(this.id);">
				<span id="cdtmf_relative_peak_row"></span>
			</td>
		</tr>
		<tr class="dtmf_flag">
			<th>
				<div class="helptooltips">
					<?php echo language('DTMF Relative Peak Col');?>:
					<span class="showhelp">
					<?php echo language('DTMF Relative Peak Col help');?>
					</span>
				</div>
			</th>
			<td >
				<input type="text" id="dtmf_relative_peak_col" name="dtmf_relative_peak_col" value="<?php echo $dtmf_relative_peak_col?>" oninput="check_dtmf_float(this.id);" onkeyup="check_dtmf_float(this.id);">
				<span id="cdtmf_relative_peak_col"></span>
			</td>
		</tr>
		<tr class="dtmf_flag">
			<th>
				<div class="helptooltips">
					<?php echo language('DTMF Hits Begin');?>:
					<span class="showhelp">
					<?php echo language('DTMF Hits Begin help');?>
					</span>
				</div>
			</th>
			<td >
				<input type="text" id="dtmf_hits_to_begin" name="dtmf_hits_to_begin" value="<?php echo $dtmf_hits_to_begin?>" oninput="check_dtmf_integer(this.id);" onkeyup="check_dtmf_integer(this.id);">
				<span id="cdtmf_hits_to_begin"></span>
			</td>
		</tr>
		<tr class="dtmf_flag">
			<th>
				<div class="helptooltips">
					<?php echo language('DTMF Misses End');?>:
					<span class="showhelp">
					<?php echo language('DTMF Misses End help');?>
					</span>
				</div>
			</th>
			<td >
				<input type="text" id="dtmf_misses_to_end" name="dtmf_misses_to_end" value="<?php echo $dtmf_misses_to_end?>" oninput="check_dtmf_integer(this.id);" onkeyup="check_dtmf_integer(this.id);">
				<span id="cdtmf_misses_to_end"></span>
			</td>
		</tr>
	</table>
	<br>

	<input type="hidden" name="send" id="send" value="" />
	<table id="float_btn" class="float_btn">
		<tr id="float_btn_tr" class="float_btn_tr">
			<td>
				<input type="submit"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check_all();" />
			</td>
		</tr>
	</table>
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td>
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check_all();" />
			</td>
		</tr>
	</table>
	</form>

<script type="text/javascript">
	$(document).ready(function(){check_all();});
	dtmf_detect_flag_change();
</script>
<?php require("/www/cgi-bin/inc/boot.inc");?>
<div id="float_btn1" class="sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()">
</div>
