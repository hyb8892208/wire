<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/check.inc");
include_once("/www/cgi-bin/inc/define.inc");
?>

<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/float_btn.js"></script>

<?php /* Defualt Settings */

$auto_lock_cell_default = array(
	'general'=>array(
		'enable'		=>'no',
		'select_type'		=>'ascend',
		'when_type'		=>'random_time',
		'signal_threshold'	=>'10',
		'max_lock'		=>'3'
	),
	'when'=>array(
		'random_time'		=>'1800-3600',
		'random_time_min'	=>'30',
		'random_time_max'	=>'60'
	),
	'debug'=>array(
		'call_ast_type'		=>'socket',
		'get_cells_from_at'	=>'no',
		'cell_mnc_check'	=>'yes',
		'cell_mcc_check'	=>'yes',
		'immediate'		=>'no',
		'at_counts'		=>'3',
		'at_timeout'		=>'10',
		'reconnect_ami_time'	=>'30'
	),
);

$internal_call_default = array(
	'general' => array(
		'enable'		=> 'no',
		'ranking'		=> 'yes',
		'maxchannel'		=> '2',
		'outcalls'		=> '3-6',
		'incalls'		=> '1-1',
		'incall_duration'	=> '60-120',
		'outcalls_min'		=> '3',
		'outcalls_max'		=> '6',
		'incalls_min'		=> '1',
		'incalls_max'		=> '1',
		'incall_duration_min'	=> '30',
		'incall_duration_max'	=> '60'
	),
	'advance' => array(
		'call_ast_type'		=> 'socket',
		'get_channel_delay'	=> '3',
		'call_fail_max'		=> '3',
		'call_fail_delay'	=> '3',
		'call_wait_answer'	=> '30',
		'call_check_delay'	=> '3',
		'call_next_delay'	=> '3',
		'ami_timeout'		=> '10',
		'ami_reconnect_delay'	=> '30',
		'enforce_incall_sw'	=> 'yes'
	)
);

$internal_sms_default = array(
	'general' => array(
		'enable'			=> 'no',
		'sender_policy'			=> 'random',
		'reply_sw'			=> 'yes',
		'send_counts_min'		=> '1',
		'send_counts_max'		=> '3',
		'receiver_counts_min'		=> '1',
		'receiver_counts_max'		=> '3',
		'send_delay_min'		=> '60',
		'send_delay_max'		=> '120',
	),
	'advance' => array(
		'call_ast_type'			=> 'socket',
		'contents_char_list'		=> 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890',
		'contents_len_min'		=> '10',
		'contents_len_max'		=> '50',
		'at_timeout'			=> '10',
		'ami_timeout'			=> '10',
		'ami_reconnect_delay'		=> '30',
	),
);
?>

<?php

function save_auto_lock_cell_conf($status)
{
/*
[general] 
enable=yes|no 
select_type=random|ascend|descend 
when_type=random_time|random_calls 
signal_threshold=10 

[when] 
random_time=30-50 
random_calls=3-5 

[debug] 
at_counts=3 
at_timeout=5 
get_cells_from_at=no|yes 
call_ast_type=pipe|socket 
cell_mcc_check=yes|no 
cell_mnc_check=yes|no 
reconnect_ami_time=30

*/

	global $auto_lock_cell_default;
	$auto_lock_cell = $auto_lock_cell_default;

	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk/gw');
	if (!$setok) {
		echo $aql->get_error();
		return false;
	}
	$conf_path = '/etc/asterisk/gw/auto_lock_cell.conf';
	$hlock = lock_file($conf_path);
        
	if(!file_exists($conf_path)) {
		fclose(fopen($conf_path,"w"));
	}
	
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	
	$exist_array = $aql->query("select * from auto_lock_cell.conf");

	if(!isset($exist_array['general'])) {
		$aql->assign_addsection('general','');
	}
	if(!isset($exist_array['when'])) {
		$aql->assign_addsection('when','');
	}
	if(!isset($exist_array['debug'])) {
		$aql->assign_addsection('debug','');
	}

	foreach($auto_lock_cell as $section => $arr){
		foreach($arr as $key => $value){
			/* $_POST => $auto_lock_cell */
			switch($key){
			case 'enable':
				if($status == 'open'){
					$auto_lock_cell[$section][$key] = 'yes';
				}elseif($status == 'close') {
						$auto_lock_cell[$section][$key] = 'no';
				} else {
					if(isset($_POST['auto_lock_cell'][$section][$key])){
						$auto_lock_cell[$section][$key] = 'yes';
					}else{
						$auto_lock_cell[$section][$key] = 'no';
					}
				}
				break;
			case 'get_cells_from_at':
			case 'cell_mnc_check':
			case 'cell_mcc_check':
			case 'immediate':
				if(isset($_POST['auto_lock_cell'][$section][$key])){
					$auto_lock_cell[$section][$key] = 'yes';
				}
				break;
			case 'random_time':
				if(isset($_POST['auto_lock_cell'][$section]['random_time_min']) && isset($_POST['auto_lock_cell'][$section]['random_time_max'])){
					$min = trim($_POST['auto_lock_cell'][$section]['random_time_min'])*60;
					$max = trim($_POST['auto_lock_cell'][$section]['random_time_max'])*60;
					if($max >= $min && $min > 0){
						$auto_lock_cell[$section][$key] = "$min-$max";
					}
				}
				break;
			case 'random_time_min':
			case 'random_time_max':
				continue 2;
			default:
				if(isset($_POST['auto_lock_cell'][$section][$key])){
					$auto_lock_cell[$section][$key] = trim($_POST['auto_lock_cell'][$section][$key]);
				}
			}

			/* $auto_lock_cell => exist_array */
			if(isset($exist_array[$section][$key])) {
				$aql->assign_editkey($section,$key,$auto_lock_cell[$section][$key]);
			} else {
				$aql->assign_append($section,$key,$auto_lock_cell[$section][$key]);
			}
		}
	}

	if (!$aql->save_config_file('auto_lock_cell.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	return true;
}

function save_internal_call_conf($status)
{
/*
-------------
[general]
enable=no|yes
maxchannel=2
outcalls=6|1-2|5-9 
incalls=1|1-2|5-9 
incall_duration=60-120 //second 

[debug] 
call_ast_type=pipe|socket
get_channel_delay=5 //second
call_fail_max=5
call_fail_delay=5 // second
call_wait_answer=30 //second
call_check_delay=5 // second
call_next_delay=10 //second
ami_timeout=10 //second
ami_reconnect_delay=30 //second
enforce_incall_sw=yes|no
--------------
*/

	global $internal_call_default;
	$internal_call = $internal_call_default;

	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk/gw');
	if (!$setok) {
		echo $aql->get_error();
		return false;
	}
	$conf_path = '/etc/asterisk/gw/auto_intercall.conf';
	$hlock = lock_file($conf_path);
        
	if(!file_exists($conf_path)) {
		fclose(fopen($conf_path,"w"));
	}
	
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	
	$exist_array = $aql->query("select * from auto_intercall.conf");

	if(!isset($exist_array['general'])) {
		$aql->assign_addsection('general','');
	}
	if(!isset($exist_array['advance'])) {
		$aql->assign_addsection('advance','');
	}

	foreach($internal_call as $section => $arr){
		foreach($arr as $key => $value){
			/* $_POST => $internal_call */
			switch($key){
			case 'enable':
				if($status == 'open'){
					$internal_call[$section][$key] = 'yes';
					break;
				} elseif($status == 'close') {
					$internal_call[$section][$key] = 'no';
					break;
				}
			case 'ranking':
			case 'enforce_incall_sw':
				if(isset($_POST['internal_call'][$section][$key])){
					$internal_call[$section][$key] = 'yes';
				}else{
					$internal_call[$section][$key] = 'no';
				}
				break;
			case 'outcalls':
			case 'incalls':
			case 'incall_duration':
				if(isset($_POST['internal_call'][$section][$key.'_min']) && isset($_POST['internal_call'][$section][$key.'_max'])){
					$min = trim($_POST['internal_call'][$section][$key.'_min']);
					$max = trim($_POST['internal_call'][$section][$key.'_max']);
					if($max >= $min && $min > 0){
						$internal_call[$section][$key] = "$min-$max";
					}
				}
				break;
			case 'outcalls_min':
			case 'outcalls_max':
			case 'incalls_min':
			case 'incalls_max':
			case 'incall_duration_min':
			case 'incall_duration_max':
				continue 2;
			default:
				if(isset($_POST['internal_call'][$section][$key])){
					$internal_call[$section][$key] = trim($_POST['internal_call'][$section][$key]);
				}
			}

			/* $internal_call => exist_array */
			if(isset($exist_array[$section][$key])) {
				$aql->assign_editkey($section,$key,$internal_call[$section][$key]);
			} else {
				$aql->assign_append($section,$key,$internal_call[$section][$key]);
			}
		}
	}

	if (!$aql->save_config_file('auto_intercall.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	return true;
}

function save_internal_sms_conf($status)
{
/*
------------------
[general]
enable=yes|no
sender_policy=random|ascend|descend
reply_sw=yes|no
send_counts_min=1
send_counts_max=5
send_delay_min=60
send_delay_max=120

[advance]
call_ast_type=pipe|socket
contents_char_list=abc
ami_timeout=10		//second
ami_reconnect_delay=30	//second
------------------
*/

	global $internal_sms_default;
	$internal_sms = $internal_sms_default;

	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk/gw');
	if (!$setok) {
		echo $aql->get_error();
		return false;
	}
	$conf_path = '/etc/asterisk/gw/auto_intersms.conf';
	$hlock = lock_file($conf_path);
        
	if(!file_exists($conf_path)) {
		fclose(fopen($conf_path,"w"));
	}
	
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	
	$exist_array = $aql->query("select * from auto_intersms.conf");

	if(!isset($exist_array['general'])) {
		$aql->assign_addsection('general','');
	}
	if(!isset($exist_array['advance'])) {
		$aql->assign_addsection('advance','');
	}

	foreach($internal_sms as $section => $arr){
		foreach($arr as $key => $value){
			/* $_POST => $internal_sms */
			switch($key){
			case 'enable':
				if($status == 'open'){
					$internal_sms[$section][$key] = 'yes';
					break;
				} elseif($status == 'close') {
					$internal_sms[$section][$key] = 'no';
					break;
				}
			case 'reply_sw':
				if(isset($_POST['internal_sms'][$section][$key])){
					$internal_sms[$section][$key] = 'yes';
				}else{
					$internal_sms[$section][$key] = 'no';
				}
				break;
			default:
				if(isset($_POST['internal_sms'][$section][$key])){
					$internal_sms[$section][$key] = trim($_POST['internal_sms'][$section][$key]);
				}
			}
			/* $internal_sms => exist_array */
			if(isset($exist_array[$section][$key])) {
				$aql->assign_editkey($section,$key,$internal_sms[$section][$key]);
			} else {
				$aql->assign_append($section,$key,$internal_sms[$section][$key]);
			}
		}
	}

	if (!$aql->save_config_file('auto_intersms.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	return true;
}
?>

<?php
function show_auto_lock_cell()
{
	global $auto_lock_cell_default;
	$auto_lock_cell = $auto_lock_cell_default;

	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$res = $aql->query("select * from auto_lock_cell.conf");

	echo "\n<script type=\"text/javascript\">\n";
	echo "function set_auto_lock_cell_default()\n";
	echo "{\n";
	foreach($auto_lock_cell as $section => $arr){
		foreach($arr as $key => $value){
			/* set js default value */
			switch($key){
			case 'enable':
				break;
			case 'get_cells_from_at':
			case 'cell_mnc_check':
			case 'cell_mcc_check':
			case 'immediate':
				$sw = is_true($value)?'true':'false';
				echo "$(\"[name='auto_lock_cell[$section][$key]']\").iButton(\"toggle\", $sw);\n";
				break;
			default:
				echo "$(\"[name='auto_lock_cell[$section][$key]']\").attr(\"value\", \"$value\");\n";
			}

			/* get config file  value */
			if(isset($res[$section][$key])){
				$val = trim($res[$section][$key]);
				if($val != ''){
					$auto_lock_cell[$section][$key] = $val;
				}
			}
		}	
	}
	echo "}\n";
	echo "</script>\n";

	if(sscanf($auto_lock_cell['when']['random_time'],"%d-%d", $min, $max) == 2 
		&& $max >= $min && $min > 0
	){
		$auto_lock_cell['when']['random_time_min'] = ceil($min/60);
		$auto_lock_cell['when']['random_time_max'] = ceil($max/60);
	}else if($auto_lock_cell['when']['random_time'] > 0){
		$auto_lock_cell['when']['random_time_min'] = ceil($auto_lock_cell['when']['random_time']/60);
		$auto_lock_cell['when']['random_time_max'] = ceil($auto_lock_cell['when']['random_time']/60);
	}

?>

	<script type="text/javascript">
	function check_auto_lock_cell()
	{
		var ret = true;
		$("#auto_lock_cell").find(":text").each(function(){
			if(!check_minmax(this)){
				ret = false;
			}
		});

		$("#auto_lock_cell_advance").find(":text").each(function(){
			if(!check_uint(this)){
				ret = false
			}
		});

		return ret;
	}
	</script>

	<div id="tab" class="div_tab_title">
		<li class="tb_unfold" onclick="lud(this,'tab_auto_lock_cell')" id="tab_auto_lock_cell_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_auto_lock_cell')"><?php echo language('BTS Setting');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_auto_lock_cell')">&nbsp;</li>
		<li class="tb_end2">&nbsp;</li>
	</div>

	<div class="div_tab" id="tab_auto_lock_cell"><!-- tab_auto_lock_cell end -->
		<div class="divc_tab_show">
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Auto Enable');?>:</div>
					<span class="showhelp">
					<?php echo language('Auto Enable 1 help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="button"  value="<?php echo language('One Key To Enable');?>" onclick="window.location.href='<?php echo get_self().'?&key_to_start=1'?>'"/>&nbsp;&nbsp;&nbsp;
				<input type="button"  value="<?php echo language('One Key To Disable');?>" onclick="window.location.href='<?php echo get_self().'?&key_to_close=1'?>'"/>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Enable');?>:</div>
					<span class="showhelp">
					<?php echo language('Enable help', "ON(enabled),OFF(disabled)");?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" class="enable" id="auto_lock_cell_sw" name="auto_lock_cell[general][enable]" 
					<?php if(is_true($auto_lock_cell['general']['enable']))echo 'checked';?>
					onchange="enable_general(this, 'auto_lock_cell');" />
			</div>
		</div>
		<div class="div_tab_hide" id='auto_lock_cell' style="height:"><!-- auto_lock_cell start -->
			<!--<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Select Cell Type');?>:</div>
					<span class="showhelp">
					<?php echo language('cell type help', 'Please select the cell type.');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<select id="select_type" onchange="" name="auto_lock_cell[general][select_type]">
					<option value="random" <?php if($auto_lock_cell['general']['select_type'] == 'random'){echo 'selected';}; ?>><?php echo language('random');?></option>
					<option value="ascend" <?php if($auto_lock_cell['general']['select_type'] == 'ascend'){echo 'selected';}; ?>><?php echo language('ascend');?></option>
					<option value="descend " <?php if($auto_lock_cell['general']['select_type'] == 'descend'){echo 'selected';}; ?>><?php echo language('descend');?></option>
				</select>
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('When Type');?>:</div>
					<span class="showhelp">
					<?php
						echo language('When type help',"Please select the when type.");
					?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<select id="when_type" onchange="" name="auto_lock_cell[general][when_type]">
					<option value="random_time" <?php if($auto_lock_cell['general']['when_type'] == 'random_time'){echo 'selected';};?>><?php echo language('random time');?></option>
				</select>
				<span></span>
			</div>-->
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Max Lock');?>:</div>
					<span class="showhelp">
					<?php echo language('Max Lock help', 'The max number of cell sum when locking.');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<select name="auto_lock_cell[general][max_lock]">
					<?php
						for($i = 1; $i <= 6; $i++){
							if($auto_lock_cell['general']['max_lock'] == $i){
								$selected = 'selected';
							}else{
								$selected = '';
							}
							echo "<option value=\"$i\" $selected>$i</option>\n";
						}
					?>
				</select>
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Random Time');?>:</div>
					<span class="showhelp">
					<?php
						$help = 'Please input the random time between 1 to 1440.For example:30-50.';
						echo language('random time help', $help);
					?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="random_time_fir" type="text" name="auto_lock_cell[when][random_time_min]" style="width:50px;" value="<?php echo $auto_lock_cell['when']['random_time_min'];?>" 
					onchange="check_minmax(this);"  />
				&nbsp;-&nbsp;
				<input id="random_time_sec" type="text" name="auto_lock_cell[when][random_time_max]" style="width:50px;" value="<?php echo $auto_lock_cell['when']['random_time_max'];?>" 
					 onchange="check_minmax(this);" />
				&nbsp;<?php echo language('minute');?>&nbsp;&nbsp;
				<span id="crandom_time"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Signal Threshold');?>:</div>
					<span class="showhelp">
					<?php
						echo language('Signal Threshold help2');
					?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<select id="signal_threshold" onchange="" name="auto_lock_cell[general][signal_threshold]">
					<?php
						for($i = 1; $i <= 31 ; $i++){
							if($auto_lock_cell['general']['signal_threshold'] == $i){
								$selected = 'selected';
							}else{
								$selected = '';
							}
							echo "<option value=\"$i\" $selected>$i</option>\n";
						}
					?>
				</select>
				<span id="csignal_threshold"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Advance');?>:</div>
					<span class="showhelp">
					<?php echo language('Enable help', "ON(enabled),OFF(disabled)");?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" class="enable" id="auto_lock_cell_advance_sw" name="auto_lock_cell_advance_sw" 
					onchange="enable_advance(this, 'auto_lock_cell_advance')" />
			</div>
		</div><!-- auto_lock_cell end -->

		<div class="div_tab_hide" id='auto_lock_cell_advance' style="height:"><!-- auto_lock_cell_advance start -->
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Call AST Type');?>:</div>
					<span class="showhelp"><?php echo language('Call AST Type help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<select id="call_ast_type" onchange="" name="auto_lock_cell[debug][call_ast_type]">
					<option value="socket" <?php if($auto_lock_cell['debug']['call_ast_type'] == 'socket'){echo 'selected';}?>>
						<?php echo 'socket';?>
					</option>
					<option value="pipe" <?php if($auto_lock_cell['debug']['call_ast_type'] == 'pipe'){echo 'selected';}?>>
						<?php echo 'pipe';?>
					</option>
				</select>
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Get Cells From AT');?>:</div>
					<span class="showhelp">
					<?php echo language('Get Cells From AT help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" class="enable" name="auto_lock_cell[debug][get_cells_from_at]" 
					<?php if(is_true($auto_lock_cell['debug']['get_cells_from_at']))echo 'checked';?>/>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Cell MCC Check');?>:</div>
					<span class="showhelp">
					<?php echo language('Cell MCC Check help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" class="enable" id="cell_mcc_check" name="auto_lock_cell[debug][cell_mcc_check]" 
					<?php if(is_true($auto_lock_cell['debug']['cell_mcc_check']))echo 'checked';?>/> 
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Cell MNC Check');?>:</div>
					<span class="showhelp">
					<?php echo language('Cell MNC Check help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" class="enable" id="cell_mnc_check" name="auto_lock_cell[debug][cell_mnc_check]" 
					<?php if(is_true($auto_lock_cell['debug']['cell_mnc_check']))echo 'checked';?>/> 
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Immediate');?>:</div>
					<span class="showhelp">
					<?php echo language('Lock Cell Immediate help', 'Lock Cell Immediately. This option may affect call quality, so recommend turning off');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" class="enable" name="auto_lock_cell[debug][immediate]" 
					<?php if(is_true($auto_lock_cell['debug']['immediate']))echo 'checked';?>/> 
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('AT Counts');?>:</div>
					<span class="showhelp">
					<?php echo language('AT Counts help2');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="at_counts" type="text" name="auto_lock_cell[debug][at_counts]" value="<?php echo $auto_lock_cell['debug']['at_counts'];?>"
					 onchange="check_uint(this);"/>
				<span id="cat_counts"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('AT Timeout');?>:</div>
					<span class="showhelp">
					<?php echo language('AT Timeout help2');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="at_timeout" type="text" name="auto_lock_cell[debug][at_timeout]" style="" value="<?php echo $auto_lock_cell['debug']['at_timeout'];?>"
					 onchange="check_uint(this);"/>
				&nbsp;<?php echo language('second');?>&nbsp;&nbsp;
				<span id="cat_timeout"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('AMI Reconnect Delay');?>:</div>
					<span class="showhelp">
					<?php echo language('AMI Reconnect Delay help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="reconnect_ami_time" type="text" name="auto_lock_cell[debug][reconnect_ami_time]" value="<?php echo $auto_lock_cell['debug']['reconnect_ami_time'];?>"
					 onchange="check_uint(this);"/>
				&nbsp;second&nbsp;&nbsp;
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Set Default');?>:</div>
					<span class="showhelp">
					<?php echo language('Set default value help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="button" value="Default" onclick="set_auto_lock_cell_default();"/>
			</div>
		</div><!-- auto_lock_cell_advance end -->
	</div><!-- tab_auto_lock_cell end -->

<?php
}
?>

<?php
function show_internal_call()
{
	global $__GSM_SUM__;
	global $__BRD_SUM__;
	global $internal_call_default;
	$internal_call = $internal_call_default;
	
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$res = $aql->query("select * from auto_intercall.conf");

	echo "\n<script type=\"text/javascript\">\n";
	echo "function set_internal_call_default()\n";
	echo "{\n";
	foreach($internal_call as $section => $arr){
		foreach($arr as $key => $value){
			switch($key){
			case 'enable':
				break;
			case 'ranking':
			case 'enforce_incall_sw':
				$sw = is_true($value)?'true':'false';
				echo "$(\"[name='internal_call[$section][$key]']\").iButton(\"toggle\", $sw);\n";
				break;
			default:
				echo "$(\"[name='internal_call[$section][$key]']\").attr(\"value\", \"$value\");\n";
			}
			if(isset($res[$section][$key])){
				$val = trim($res[$section][$key]);
				if($val != ''){
					$internal_call[$section][$key] = $val;
				}
			}
		}	
	}
	echo "check_internal_call();\n";
	echo "}\n";
	echo "</script>\n";

	if(sscanf($internal_call['general']['outcalls'],"%d-%d", $min, $max) == 2 
		&& $max >= $min && $min > 0
	){
		$internal_call['general']['outcalls_min'] = $min;
		$internal_call['general']['outcalls_max'] = $max;
	}else if($internal_call['general']['outcalls'] > 0){
		$internal_call['general']['outcalls_min'] = $internal_call['general']['outcalls'];
		$internal_call['general']['outcalls_max'] = $internal_call['general']['outcalls'];
	}

	if(sscanf($internal_call['general']['incalls'],"%d-%d", $min, $max) == 2 
		&& $max >= $min && $min > 0
	){
		$internal_call['general']['incalls_min'] = $min;
		$internal_call['general']['incalls_max'] = $max;
	}else if($internal_call['general']['incalls'] > 0){
		$internal_call['general']['incalls_min'] = $internal_call['general']['incalls'];
		$internal_call['general']['incalls_max'] = $internal_call['general']['incalls'];
	}
	
	if(sscanf($internal_call['general']['incall_duration'],"%d-%d", $min, $max) == 2 
		&& $max >= $min && $min > 0
	){
		$internal_call['general']['incalls_duration_min'] = $min;
		$internal_call['general']['incalls_duration_max'] = $max;
	}else if($internal_call['general']['incall_duration'] > 0){
		$internal_call['general']['incalls_duration_min'] = $internal_call['general']['incall_duration'];
		$internal_call['general']['incalls_duration_max'] = $internal_call['general']['incall_duration'];
	}

?>
	<script type="text/javascript">
	function check_internal_call()
	{
		var ret = true;
		$("#internal_call").find(":text").each(function(){
			if(!check_minmax(this)){
				ret = false;
			}
		});

		$("#internal_call_advance").find(":text").each(function(){
			if(!check_uint(this)){
				ret = false
			}
		});

		return ret;
	}

	$(document).ready(function (){ 
		$("#internal_call_advance").find(":text").keyup(function(){
			check_uint(this);
		});
	});
	</script>
	
	<div id="tab" class="div_tab_title">
		<li class="tb_unfold" onclick="lud(this,'tab_internal_call')" id="tab_internal_call_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_internal_call')"><?php echo language('Auto Internal Call');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_internal_call')">&nbsp;</li>
		<li class="tb_end2">&nbsp;</li>
	</div>

	<div class="div_tab" id="tab_internal_call"><!-- tab_internal_call start -->
		<div class="divc_tab_show">
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Auto Enable');?>:</div>
					<span class="showhelp">
					<?php echo language('Auto Enable 2 help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="button" value="<?php echo language('One Key To Enable');?>" onclick="window.location.href='<?php echo get_self().'?&key_to_start=2'?>'"/>&nbsp;&nbsp;&nbsp;
				<input type="button" value="<?php echo language('One Key To Disable');?>" onclick="window.location.href='<?php echo get_self().'?&key_to_close=2'?>'"/>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Enable');?>:</div>
					<span class="showhelp">
					<?php echo language('Enable help3');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" class="enable" id="internal_call_sw" name="internal_call[general][enable]" 
					<?php
						if($internal_call['general']['enable'] == 'on' || $internal_call['general']['enable'] == 'yes'){
							echo 'checked';
						}
					?> 
					onchange="enable_general(this, 'internal_call');" />
			</div>
		</div>
		<div class="div_tab_hide" id='internal_call' style="height:180"><!-- internal_call start -->
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Ranking');?>:</div>
					<span class="showhelp">
					<?php echo language('Ranking help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" class="enable" name="internal_call[general][ranking]" <?php if(is_true($internal_call['general']['ranking'])){echo 'checked';}?> />
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('MAX Channels');?>:</div>
					<span class="showhelp">
					<?php echo language('MAX Channels help', 'Please select the max channels for internal call');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<select id="select_type" onchange="" name="internal_call[general][maxchannel]">
					<?php 
						for($i = 1; $i <= ($__GSM_SUM__*$__BRD_SUM__/2); $i++){ 
							$j = 2*$i;
							if($j == $internal_call['general']['maxchannel']){
								$selected = 'selected';
							}else{
								$selected = '';
							}
							echo "<option value=\"$j\" $selected>$j</option>";
						} 
					?>
				</select><span id="cselect_type"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('External Calls');?>:</div>
					<span class="showhelp">
					<?php
						$help = 'Please input the range of external calls counts for every gsm span every cycle';
						echo language('External Calls help', $help);
					?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_call[general][outcalls_min]" style="width:50px;" value="<?php echo $internal_call['general']['outcalls_min']; ?>" 
					onchange="check_minmax(this);"/>
				&nbsp;-&nbsp;
				<input type="text" name="internal_call[general][outcalls_max]" style="width:50px;" value="<?php echo $internal_call['general']['outcalls_max']; ?>" 
					 onchange="check_minmax(this);"/>
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Internal Calls');?>:</div>
					<span class="showhelp">
					<?php
						$help = 'Please input the range of internal calls counts for every gsm span every cycle';
						echo language('Internal Calls help', $help);
					?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_call[general][incalls_min]" style="width:50px;" value="<?php echo $internal_call['general']['incalls_min']; ?>" 
					onchange="check_minmax(this);"/>
				&nbsp;-&nbsp;
				<input type="text" name="internal_call[general][incalls_max]" style="width:50px;" value="<?php echo $internal_call['general']['incalls_max']; ?>" 
					 onchange="check_minmax(this);"/>
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Internal Call Duration');?>:</div>
					<span class="showhelp">
					<?php
						$help = 'Please input the range of duration for every internal call';
						echo language('Internal Calls Duration help', $help);
					?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_call[general][incall_duration_min]" style="width:50px;" value="<?php echo $internal_call['general']['incalls_duration_min']; ?>" 
					 onchange="check_minmax(this);"/>
				&nbsp;-&nbsp;
				<input type="text" name="internal_call[general][incall_duration_max]" style="width:50px;" value="<?php echo $internal_call['general']['incalls_duration_max']; ?>" 
					 onchange="check_minmax(this);"/>
				&nbsp;<?php echo language('second');?>&nbsp;&nbsp;
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Advance');?>:</div>
					<span class="showhelp">
					<?php echo language('Advance help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" class="enable" id="internal_call_advance_sw" name="internal_call_advance_sw" 
					onchange="enable_advance(this, 'internal_call_advance')" />
			</div>
		</div><!-- internal_call end -->

		<div class="div_tab_hide" id='internal_call_advance' style="height:252"><!-- internal_call_advance start -->
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Call AST Type');?>:</div>
					<span class="showhelp">
					<?php echo language('Call AST Type help','The communicate type with asterisk');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<select onchange="" name="internal_call[advance][call_ast_type]">
					<option value="socket" <?php if($internal_call['advance']['call_ast_type'] == 'socket'){echo 'selected';}; ?>>
						<?php echo 'socket'; ?>
					</option>
					<option value="pipe" <?php if($internal_call['advance']['call_ast_type'] == 'pipe'){echo 'selected';}; ?>>
						<?php echo 'pipe'; ?>
					</option>
				</select>
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Get Channel Delay');?>:</div>
					<span class="showhelp">
					<?php echo language('Get Channel Delay help','The delay after get new channel fail for internal call');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_call[advance][get_channel_delay]" value="<?php echo $internal_call['advance']['get_channel_delay'];?>"/>
				&nbsp;<?php echo language('second');?>&nbsp;&nbsp;
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Call Fail MAX');?>:</div>
					<span class="showhelp">
					<?php echo language('Call Fail MAX help','The max counts for failure of internal call every cycle');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_call[advance][call_fail_max]" value="<?php echo $internal_call['advance']['call_fail_max'];?>"/>
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Call Fail Delay');?>:</div>
					<span class="showhelp">
					<?php echo language('Call Fail Delay help','The delay between the failure and recall of one internal call');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_call[advance][call_fail_delay]" value="<?php echo $internal_call['advance']['call_fail_delay'];?>"/>
				&nbsp;<?php echo language('second');?>&nbsp;&nbsp;
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Call Wait Answer');?>:</div>
					<span class="showhelp">
					<?php echo language('Call Wait Answer help','The timewait before callee answer');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_call[advance][call_wait_answer]" value="<?php echo $internal_call['advance']['call_wait_answer'];?>"/>
				&nbsp;<?php echo language('second');?>&nbsp;&nbsp;
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Call Check Delay');?>:</div>
					<span class="showhelp">
					<?php echo language('Call Check Delay help','The delay for every check of call connection');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_call[advance][call_check_delay]" value="<?php echo $internal_call['advance']['call_check_delay'];?>"/>
				&nbsp;<?php echo language('second');?>&nbsp;&nbsp;
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Call Next Delay');?>:</div>
					<span class="showhelp">
					<?php echo language('Call Next Delay help','The delay between one succuss internal call and the next');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_call[advance][call_next_delay]" value="<?php echo $internal_call['advance']['call_next_delay'];?>"/>
				&nbsp;<?php echo language('second');?>&nbsp;&nbsp;
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('AMI Timeout');?>:</div>
					<span class="showhelp">
					<?php echo language('AMI Timeout help','When you select socket of call ast type, this option controls the socket connection timeout');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_call[advance][ami_timeout]" value="<?php echo $internal_call['advance']['ami_timeout'];?>"/>
				&nbsp;<?php echo language('second');?>&nbsp;&nbsp;
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('AMI Reconnect Delay');?>:</div>
					<span class="showhelp">
					<?php echo language('AMI Reconnect Delay help','When you select socket of call ast type, this option controls the delay before socket reconnection ');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_call[advance][ami_reconnect_delay]" value="<?php echo $internal_call['advance']['ami_reconnect_delay'];?>"/>
				&nbsp;<?php echo language('second');?>&nbsp;&nbsp;
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Enforce');?>:</div>
					<span class="showhelp">
					<?php echo language('Enforce Internal Call help','This option will enforce hangup the caller and callee before internal call begin.');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" class="enable"  name="internal_call[advance][enforce_incall_sw]" 
					<?php
						if($internal_call['advance']['enforce_incall_sw'] == 'on' || $internal_call['advance']['enforce_incall_sw'] == 'yes'){
							echo 'checked';
						}
					?>
				/>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Set Default2');?>:</div>
					<span class="showhelp">
					<?php echo language('Set default value help2');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="button" value="Default" onclick="set_internal_call_default();"/>
			</div>
		</div><!-- internal_call_advance end -->
	</div><!-- tab_internal_call end -->
<?php
}
?>

<?php
function show_internal_sms()
{
	global $internal_sms_default;
	$internal_sms = $internal_sms_default;

	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$res = $aql->query("select * from auto_intersms.conf");

	echo "\n<script type=\"text/javascript\">\n";
	echo "function set_internal_sms_default()\n";
	echo "{\n";
	foreach($internal_sms as $section => $arr){
		foreach($arr as $key => $value){
			switch($key){
			case 'enable':
				break;
			case 'reply_sw':
				$sw = is_true($value)?'true':'false';
				echo "$(\"[name='internal_sms[$section][$key]']\").iButton(\"toggle\", $sw);\n";
				break;
			default:
				echo "$(\"[name='internal_sms[$section][$key]']\").attr(\"value\", \"$value\");\n";
			}
			if(isset($res[$section][$key])){
				$val = trim($res[$section][$key]);
				if($val != ''){
					$internal_sms[$section][$key] = $val;
				}
			}
		}	
	}
	echo "}\n";
	echo "</script>\n";
?>

	<script type="text/javascript">
	function check_sms_contents(obj)
	{
		obj.value = obj.value.replace(/[^\w]*/g,'');
	}

	function check_internal_sms()
	{
		var ret = true;
		$("#internal_sms").find(":text").each(function(){
			if(!check_minmax(this)){
				ret = false;
			}
		});

		$("#internal_sms_advance").find(":text").each(function(){
			if(this.name != 'internal_sms[advance][contents_char_list]'){
				if(!check_uint(this)){
					ret = false;
				}
			}
		});

		if(check_sms_contents($("[name='internal_sms[advance][contents_char_list]']")[0])){
			return false;
		}

		return ret;
	}

	</script>

	<div id="tab" class="div_tab_title">
		<li class="tb_unfold" onclick="lud(this,'tab_internal_sms')" id="tab_internal_sms_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_internal_sms')"><?php echo language('Auto Internal SMS');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_internal_sms')">&nbsp;</li>
		<li class="tb_end2">&nbsp;</li>
	</div>

	<div class="div_tab" id="tab_internal_sms"><!-- tab_internal_sms start -->
		<div class="divc_tab_show">
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Auto Enable');?>:</div>
					<span class="showhelp">
					<?php echo language('Auto Enable 3 help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="button"  value="<?php echo language('One Key To Enable');?>" onclick="window.location.href='<?php echo get_self().'?&key_to_start=3'?>'"/>&nbsp;&nbsp;&nbsp;
				<input type="button"  value="<?php echo language('One Key To Disable');?>" onclick="window.location.href='<?php echo get_self().'?&key_to_close=3'?>'"/>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Enable');?>:</div>
					<span class="showhelp">
					<?php echo language('Enable help', "ON(enabled),OFF(disabled)");?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" class="enable" id="internal_sms_sw" name="internal_sms[general][enable]" 
					<?php if(is_true($internal_sms['general']['enable']))echo 'checked';?> 
					onchange="enable_general(this, 'internal_sms');" />
			</div>
		</div>
		<div class="div_tab_hide" id='internal_sms' style="height:"><!-- internal_sms start -->
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Reply');?>:</div>
					<span class="showhelp">
					<?php echo language('Reply help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" class="enable" name="internal_sms[general][reply_sw]" 
					<?php if(is_true($internal_sms['general']['reply_sw']))echo 'checked';?>/>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Sender Policy');?>:</div>
					<span class="showhelp">
					<?php echo language('Internal SMS Sender Policy help', 'The different orders of GSM spans sending SMS');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<select name="internal_sms[general][sender_policy]">
					<option value="random" <?php if($internal_sms['general']['sender_policy'] == 'randome'){echo 'selected';};?>>
						<?php echo language('random');?>
					</option>
					<option value="ascend" <?php if($internal_sms['general']['sender_policy'] == 'ascend'){echo 'selected';};?>>
						<?php echo language('ascend');?>
					</option>
					<option value="descend" <?php if($internal_sms['general']['sender_policy'] == 'descend'){echo 'selected';};?>>
						<?php echo language('descend');?>
					</option>
				</select>
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Receiver Number');?>:</div>
					<span class="showhelp">
					<?php
						$help = 'The SMS receiver number from one sender';
						echo language('Internal SMS Receiver Number help', $help);
					?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_sms[general][receiver_counts_min]" style="width:50px;" 
					value="<?php echo $internal_sms['general']['receiver_counts_min']; ?>" 
					onchange="check_minmax(this);"/>
				&nbsp;-&nbsp;
				<input type="text" name="internal_sms[general][receiver_counts_max]" style="width:50px;" 
					value="<?php echo $internal_sms['general']['receiver_counts_max']; ?>" 
					onchange="check_minmax(this);"/>
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Send Counts');?>:</div>
					<span class="showhelp">
					<?php
						$help = 'The SMS counts range of one sender sending SMS to one receiver';
						echo language('Internal SMS Send Counts help', $help);
					?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_sms[general][send_counts_min]" style="width:50px;" 
					value="<?php echo $internal_sms['general']['send_counts_min']; ?>" 
					 onchange="check_minmax(this);"/>
				&nbsp;-&nbsp;
				<input type="text" name="internal_sms[general][send_counts_max]" style="width:50px;" 
					value="<?php echo $internal_sms['general']['send_counts_max']; ?>" 
					onchange="check_minmax(this);"/>
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Send Delay');?>:</div>
					<span class="showhelp">
					<?php
						$help = 'The delay range after sending one SMS and before another sending.';
						echo language('Internal SMS Send Delay help', $help);
					?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_sms[general][send_delay_min]" style="width:50px;" 
					value="<?php echo $internal_sms['general']['send_delay_min']; ?>" 
					 onchange="check_minmax(this);"/>
				&nbsp;-&nbsp;
				<input type="text" name="internal_sms[general][send_delay_max]" style="width:50px;" 
					value="<?php echo $internal_sms['general']['send_delay_max']; ?>" 
					 onchange="check_minmax(this);"/>
				&nbsp;<?php echo language('second');?>&nbsp;&nbsp;
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Advance');?>:</div>
					<span class="showhelp">
					<?php echo language('Enable help', "ON(enabled),OFF(disabled)");?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" class="enable" id="internal_sms_advance_sw" name="internal_sms_advance_sw" 
					onchange="enable_advance(this, 'internal_sms_advance')" />
			</div>
		</div><!-- internal_sms end -->

		<div class="div_tab_hide" id='internal_sms_advance' style="height:"><!-- internal_sms_advance start -->
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Call AST Type');?>:</div>
					<span class="showhelp">
					<?php echo language('Call AST Type help','The communicate type with asterisk');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<select onchange="" name="internal_sms[advance][call_ast_type]">
					<option value="socket" <?php if($internal_sms['advance']['call_ast_type'] == 'socket'){echo 'selected';}; ?>>
						<?php echo 'socket'; ?>
					</option>
					<option value="pipe" <?php if($internal_sms['advance']['call_ast_type'] == 'pipe'){echo 'selected';}; ?>>
						<?php echo 'pipe'; ?>
					</option>
				</select>
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('SMS Contents Chars');?>:</div>
					<span class="showhelp">
					<?php echo language('SMS Contents Chars help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" size="64" name="internal_sms[advance][contents_char_list]" value="<?php echo $internal_sms['advance']['contents_char_list'];?>"
					 onchange="check_sms_contents(this);"/>
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('SMS Contents Length');?>:</div>
					<span class="showhelp">
					<?php echo language('SMS Contents Length help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_sms[advance][contents_len_min]" style="width:50px;" 
					value="<?php echo $internal_sms['advance']['contents_len_min']; ?>" 
					 onchange="check_minmax(this);"/>
				&nbsp;-&nbsp;
				<input type="text" name="internal_sms[advance][contents_len_max]" style="width:50px;" 
					value="<?php echo $internal_sms['advance']['contents_len_max']; ?>" 
					 onchange="check_minmax(this);"/>
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('AT Timeout');?>:</div>
					<span class="showhelp">
					<?php echo language('AT Timeout help 1');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_sms[advance][at_timeout]" value="<?php echo $internal_sms['advance']['at_timeout'];?>"
					onchange="check_uint(this);" />
				&nbsp;<?php echo language('second');?>&nbsp;&nbsp;
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('AMI Timeout');?>:</div>
					<span class="showhelp">
					<?php echo language('AMI Timeout help','When you select socket of call ast type, this option controls the socket connection timeout');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_sms[advance][ami_timeout]" value="<?php echo $internal_sms['advance']['ami_timeout'];?>"
					 onchange="check_uint(this);"/>
				&nbsp;<?php echo language('second');?>&nbsp;&nbsp;
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('AMI Reconnect Delay');?>:</div>
					<span class="showhelp">
					<?php echo language('AMI Reconnect Delay help','
						When you select socket of call ast type, this option controls the delay before socket reconnection ');
					?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="text" name="internal_sms[advance][ami_reconnect_delay]" value="<?php echo $internal_sms['advance']['ami_reconnect_delay'];?>"
					 onchange="check_uint(this);"/>
				&nbsp;<?php echo language('second');?>&nbsp;&nbsp;
				<span></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Set Default');?>:</div>
					<span class="showhelp">
					<?php echo language('Set default value help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="button" value="Default" onclick="set_internal_sms_default();"/>
			</div>
		</div><!-- internal_sms_advance end -->
	</div><!-- tab_internal_sms end -->
<?php
}
?>

<?php

if($_POST && isset($_POST['send']) && $_POST['send'] == 'Save') {
	$cluster_info = get_cluster_info();
	if(save_auto_lock_cell_conf('') && save_internal_call_conf('') && save_internal_sms_conf('')){
		wait_apply("exec", "/etc/init.d/gsm_special_funs restart > /dev/null 2>&1 &");
		wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");
		if($cluster_info['mode'] == 'master') {
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
					set_slave_file($slaveip,"/etc/asterisk/gw/auto_lock_cell.conf","/etc/asterisk/gw/auto_lock_cell.conf");
					//set_slave_file($slaveip,"/etc/asterisk/gw/auto_intercall.conf","/etc/asterisk/gw/auto_intercall.conf");
					//set_slave_file($slaveip,"/etc/asterisk/gw/auto_intersms.conf","/etc/asterisk/gw/auto_intersms.conf");
					wait_apply("request_slave", $slaveip, "syscmd:/etc/init.d/gsm_special_funs restart > /dev/null 2>&1 &");
					wait_apply("request_slave", $slaveip, "astcmd:core reload");
				}
			}
		}
	}
} elseif($_GET && isset($_GET['key_to_start'])) {
	if($_GET['key_to_start'] == '1'){
		auto_lock_cell_advance('open');
		save_auto_lock_cell_conf('open');
		system_operation();
	} elseif($_GET['key_to_start'] == '2') {
		auto_internal_call_advance('open');
		save_internal_call_conf('open');
		system_operation();
	} elseif($_GET['key_to_start'] == '3') {
		save_internal_sms_conf('open');
		auto_internal_call_advance('open');
		system_operation();
	}
} elseif($_GET && isset($_GET['key_to_close'])) {
	if($_GET['key_to_close'] == '1'){
		auto_lock_cell_advance('close');
		save_auto_lock_cell_conf('close');
		system_operation();
	} elseif($_GET['key_to_close'] == '2') {
		auto_internal_call_advance('close');
		save_internal_call_conf('close');
		system_operation();
	} elseif($_GET['key_to_close'] == '3') {
		save_internal_sms_conf('close');
		auto_internal_call_advance('close');
		system_operation();
	}
}

function auto_lock_cell_advance($cell_status)
{
	if(!isset($cell_status)) return;
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
	if(isset($exist_array['channels']['start_get_cells'])) {
		if($cell_status == 'open'){
			$aql->assign_editkey('channels','start_get_cells','yes');
		} elseif($cell_status == 'close'){
			$aql->assign_editkey('channels','start_get_cells','no');
		}
	} else {
		if($cell_status == 'open'){
			$aql->assign_append('channels','start_get_cells','yes');
		} elseif($cell_status == 'close'){
			$aql->assign_append('channels','start_get_cells','no');
		}
	}

	if(isset($exist_array['channels']['fast_start'])) {
		if($cell_status == 'open'){
			$aql->assign_editkey('channels','fast_start','yes');
		} elseif($cell_status == 'close'){
			$aql->assign_editkey('channels','fast_start','no');
		}
	} else {
		if($cell_status == 'open'){
			$aql->assign_append('channels','fast_start','yes');
		} elseif($cell_status == 'close'){
			$aql->assign_append('channels','fast_start','no');
		}
	}

	if (!$aql->save_config_file('extra-global.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	unlock_file($hlock);
	return true;
}

function auto_internal_call_advance($cell_status)
{
	if(!isset($cell_status)) return;
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
	
	if(isset($exist_array['channels']['start_get_own_num'])) {
		if($cell_status == 'open'){
			$aql->assign_editkey('channels','start_get_own_num','yes');
		} elseif($cell_status == 'close'){
			$aql->assign_editkey('channels','start_get_own_num','no');
		}
	} else {
		if($cell_status == 'open'){
			$aql->assign_append('channels','start_get_own_num','yes');
		} elseif($cell_status == 'close'){
			$aql->assign_append('channels','start_get_own_num','no');
		}
	}
	
	if (!$aql->save_config_file('extra-global.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	unlock_file($hlock);
	return true;
}

function system_operation(){
	global $__BRD_HEAD__; 
	global $__BRD_SUM__;

	wait_apply("exec", "asterisk -rx \"extra restart\" > /dev/null 2>&1 &");
	wait_apply("exec", "/etc/init.d/gsm_special_funs restart > /dev/null 2>&1 &");
	wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");
	$cluster_info = get_cluster_info();
	if($cluster_info['mode'] == 'master') {
		for($b=2; $b<=$__BRD_SUM__; $b++) {
			if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
				$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
				set_slave_file($slaveip,"/etc/asterisk/extra-global.conf","/etc/asterisk/extra-global.conf");
				set_slave_file($slaveip,"/etc/asterisk/gw/auto_lock_cell.conf","/etc/asterisk/gw/auto_lock_cell.conf");
				//set_slave_file($slaveip,"/etc/asterisk/gw/auto_intercall.conf","/etc/asterisk/gw/auto_intercall.conf");
				//set_slave_file($slaveip,"/etc/asterisk/gw/auto_intersms.conf","/etc/asterisk/gw/auto_intersms.conf");
				wait_apply("request_slave", $slaveip, "syscmd:asterisk -rx \"extra restart\" > /dev/null 2>&1 &");
				wait_apply("request_slave", $slaveip, "syscmd:/etc/init.d/gsm_special_funs restart > /dev/null 2>&1 &");
				wait_apply("request_slave", $slaveip, "astcmd:core reload");
			}
		}
	}
}

?>

<script type="text/javascript">
function enable_general(obj, showId)
{
	$("#"+showId+"_advance_sw").iButton("toggle", false);
	$("#"+showId+"_advance").hide();
	$("#"+showId).slideToggle();
}

function enable_advance(obj, showId)
{
	$('#'+showId).slideToggle();
}

function digital(obj)
{
	obj.value = obj.value.replace(/[^\d]*/g,'');
}

function check_minmax(obj)
{
	var ret = true;
	digital(obj);
	
	var min_obj = $(obj).parent("div").find("input:eq(0)")[0];
	var max_obj = $(obj).parent("div").find("input:eq(1)")[0];
	var min = parseInt(min_obj.value);
	var max = parseInt(max_obj.value);
	$(obj).parent("div").find("span").html("");
	if(min <= 0 || min > max || min_obj.value == '' || max_obj.value == ''){
		$(obj).parent("div").find("span").html(con_str('<?php echo language('MIN MAX help', 'MAX should not less than MIN and greater than 0');?>'));
		return false;
	}

	if(obj.name == 'internal_sms[general][receiver_counts_min]' || obj.name == 'internal_sms[general][receiver_counts_max]'){
		if(max > <?php echo $__GSM_SUM__*$__BRD_SUM__;?>){
			$(obj).parent("div").find("span").html(con_str('<?php echo language('MIN MAX receiver_counts help', 'MAX should less than ');echo $__GSM_SUM__*$__BRD_SUM__;?>'));
			return false;
		}
	}

	if(obj.name == 'internal_sms[advance][contents_len_min]' || obj.name == 'internal_sms[advance][contents_len_max]'){
		if(max > 50){
			$(obj).parent("div").find("span").html(con_str('<?php echo language('MIN MAX contents_len help', 'MAX should less than 50');?>'));
			return false;
		}
	}

	return ret;
}

function check_uint(obj)
{
	digital(obj);
	
	var val = parseInt(obj.value);
	if(val <= 0 || obj.value == ''){
		$(obj).parent("div").find("span").html(con_str('<?php echo language('Input digital value help', 'Please input digital value (>0)');?>'));
		return false;
	}else{
		$(obj).parent("div").find("span").html("");
		return true;
	}
}

function check_all()
{
	if(!check_auto_lock_cell()
		|| !check_internal_call()
		|| !check_internal_sms()
	){
		return false;
	}

	return true;
}

</script>

<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<?php show_auto_lock_cell();?><br>
	<?php show_internal_call();?><br>
	<?php show_internal_sms();?><br>
	<input type="hidden" name="send" id="send" value="" />
	<div id="float_btn" class="float_btn">
		<div id="float_btn_tr" class="float_btn_tr">
			<input type="submit" value="<?php echo language('Save');?>" 
				onclick="document.getElementById('send').value='Save';return check_all();"/>
		</div>
	</div>
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td width="25px">
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" 
					onclick="document.getElementById('send').value='Save';return check_all();" />
			</td>
		</tr>
	</table>
</form>

<script type="text/javascript">
$(document).ready(function (){ 
	$(".enable").iButton();
	if($('#auto_lock_cell_sw').attr("checked")=="checked")$("#auto_lock_cell").show();
	if($('#internal_call_sw').attr("checked")=="checked")$("#internal_call").show();
	if($('#internal_sms_sw').attr("checked")=="checked")$("#internal_sms").show();
}); 
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>
<div id="float_btn1" class="sec_float_btn1"></div>
<div class="float_close" onclick="close_btn()"></div>
