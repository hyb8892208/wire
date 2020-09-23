<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
require_once("/www/cgi-bin/inc/aql.php");
?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/float_btn.js"></script>
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />
<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
<link type="text/css" href="/css/jquery-ui-timepicker-addon.css" rel="stylesheet" media="all"/>
<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>
<script type="text/javascript" src="/js/jquery-ui-timepicker-addon.js"></script>

<?php
function show_auto(){
	global $__GSM_SUM__;
?>
<form enctype="multipart/form-data" action="<?php echo $_SERVER['PHP_SELF'] ?>" method="post">
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('General');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

<?php
$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw/');
$res = $aql->query("select * from call_monitor.conf");

$aql->set('basedir','/data/log/');
$res2 = $aql->query("select * from call_monitor_status.conf");

if(isset($res['general']['callmonitor_switch']) && $res['general']['callmonitor_switch'] == 'yes'){
	$callmonitor_switch_checked = 'checked';
}else{
	$callmonitor_switch_checked = '';
}

if(isset($res['general']['fixed_time_call_switch']) && $res['general']['fixed_time_call_switch'] == 'yes'){
	$fixed_time_call_switch_checked = 'checked';
}else{
	$fixed_time_call_switch_checked = '';
}

if(isset($res['general']['fixed_time_call_type'])){
	$fixed_time_call_type = $res['general']['fixed_time_call_type'];
}else{
	$fixed_time_call_type = 'day';
}

if(isset($res2['general']['next_fixed_time'])){
	$fixed_time_calltime = $res2['general']['next_fixed_time'];
}else if(isset($res['general']['fixed_time_calltime'])){
	$fixed_time_calltime = $res['general']['fixed_time_calltime'];
}else{
	$fixed_time_calltime = `date "+%Y-%m-%d %H:%M:%S"`;
}

$start_time_temp = explode(":", $res['general']['start_time']);
if(isset($res['general']['start_time'])){
	$start_hour = $start_time_temp[0];
	$start_min = $start_time_temp[1];
}else{
	$start_hour = 0;
	$start_min = 0;
}

if(isset($res['general']['end_time'])){
	$end_time_temp = explode(":", $res['general']['end_time']);
	$end_hour = $end_time_temp[0];
	$end_min = $end_time_temp[1];
}else{
	$end_hour = 0;
	$end_min = 0;
}

if(isset($res['general']['call_max_time'])){
	$call_max_time = $res['general']['call_max_time'];
}else{
	$call_max_time = '';
}

if(isset($res['general']['call_min_time'])){
	$call_min_time = $res['general']['call_min_time'];
}else{
	$call_min_time = '';
}

?>
	<table width="100%" class="tedit">
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Enable');?>:
					<span class="showhelp">
					<?php echo language('Enable help','Enable');?>
					</span>
				</div>
			</th>
			<td>
				<input type="checkbox" name="callmonitor_switch" id="callmonitor_switch" <?php echo $callmonitor_switch_checked?> />
			</td>
		</tr>
		
		<tbody class="callmonitor_switch_show">
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Fixed Time Call');?>:
					<span class="showhelp">
					<?php echo language('Fixed Time Call help','Fixed Time Call');?>
					</span>
				</div>
			</th>
			<td>
				<input type="checkbox" name="fixed_time_call_switch" id="fixed_time_call_switch" <?php echo $fixed_time_call_switch_checked;?> />
			</td>
		</tr>
		
		<tr class="fixed_time_call_show">
			<th>
				<div class="helptooltips">
					<?php echo language('Fixed Time Call Type');?>:
					<span class="showhelp">
					<?php echo language('Fixed Time Call Type help','Fixed Time Call Type');?>
					</span>
				</div>
			</th>
			<td>
				<select name="fixed_time_call_type" id="fixed_time_call_type">
					<option value="day" <?php if($fixed_time_call_type == 'day') echo 'selected';?>>1 <?php echo language('Day');?></option>
					<option value="2day" <?php if($fixed_time_call_type == '2day') echo 'selected';?>>2 <?php echo language('Day');?></option>
					<option value="3day" <?php if($fixed_time_call_type == '3day') echo 'selected';?>>3 <?php echo language('Day');?></option>
					<option value="4day" <?php if($fixed_time_call_type == '4day') echo 'selected';?>>4 <?php echo language('Day');?></option>
					<option value="5day" <?php if($fixed_time_call_type == '5day') echo 'selected';?>>5 <?php echo language('Day');?></option>
					<option value="week" <?php if($fixed_time_call_type == 'week') echo 'selected';?>>1 <?php echo language('Week');?></option>
					<option value="month" <?php if($fixed_time_call_type == 'month') echo 'selected';?>>1 <?php echo language('Month');?></option>
				</select>
			</td>
		</tr>
		
		<tr class="fixed_time_call_show">
			<th>
				<div class="helptooltips">
					<?php echo language('Next Call Time');?>:
					<span class="showhelp">
					<?php echo language('Next Call Time help','Defined next call date. <br>System will count start from that date and work as Reset Period setting.');?>
					</span>
				</div>
			</th>
			<td>
				<input type="text" name="fixed_time_calltime" id="fixed_time_calltime" value="<?php echo $fixed_time_calltime;?>" onchange="check_dl_datetime(this.id);" />
				<span id="cfixed_time_calltime"></span>
			</td>
		</tr>
		
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Setup Period');?>:
					<span class="showhelp">
					<?php echo language('Setup Period help','Setup Period'); ?>
					</span>
				</div>
			</th>
			<td>
				<select name="start_hour" id="start_hour">
					<?php
					for($i=0;$i<=23;$i++){
						$start_hour_sel = '';
						if($i == $start_hour){
							$start_hour_sel = 'selected';
						}
						echo "<option value='$i' $start_hour_sel>$i</option>";
					}
					?>
				</select>
				
				<select name="start_min" id="start_min">
					<?php
					for($i=0;$i<=59;$i++){
						$start_min_sel = '';
						if($i == $start_min){
							$start_min_sel = 'selected';
						}
						echo "<option value='$i' $start_min_sel>$i</option>";
					}
					?>
				</select>
				
				--
				
				<select name="end_hour" id="end_hour">
					<?php
					for($i=0;$i<=23;$i++){
						$end_hour_sel = '';
						if($i == $end_hour){
							$end_hour_sel = 'selected';
						}
						echo "<option value='$i' $end_hour_sel>$i</option>";
					}
					?>
				</select>
				
				<select name="end_min" id="end_min">
					<?php
					for($i=0;$i<=59;$i++){
						$end_min_sel = '';
						if($i == $end_min){
							$end_min_sel = 'selected';
						}
						echo "<option value='$i' $end_min_sel>$i</option>";
					}
					?>
				</select>
			</td>
		</tr>
		
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Call Max Time');?>:
					<span class="showhelp">
					<?php echo language('Call Max Time help','Call Max Time');?>
					</span>
				</div>
			</th>
			<td>
				<input type="text" name="call_max_time" id="call_max_time" value="<?php echo $call_max_time;?>" />
				<span id="ccall_max_time"></span>
			</td>
		</tr>
		
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Call Min Time');?>:
					<span class="showhelp">
					<?php echo language('Call Min Time help','Call Min Time');?>
					</span>
				</div>
			</th>
			<td>
				<input type="text" name="call_min_time" id="call_min_time" value="<?php echo $call_min_time;?>" />
				<span id="ccall_min_time"></span>
			</td>
		</tr>
		</tbody>
	</table>
	
	<br/>
	
	<table width="100%" class="tshow callmonitor_switch_show">
	
		<input type="hidden" id="sel_channel" name="sel_channel" value="" />
		<tr>
			<th style="width:03%" class="nosort">
				<input type="checkbox" name="selall" onclick="selectall(this.checked)" />
			</th>
			<th><?php echo language('ID');?></th>
			<th><?php echo language('Call Duration@auto','Call Duration');?></th>
			<th><?php echo language('Call Times');?></th>
			<th><?php echo language('Call Answers');?></th>
			<th><?php echo language('Online Time','Online Time(minute)');?></th>
			<th><?php echo language('Handle Type');?></th>
			<th width="30px"><?php echo language('Save');?></th>
		</tr>
		
<?php
	for($i=0;$i<=$__GSM_SUM__;$i++){
		$port_name = get_gsm_name_by_channel($i);
		if(strstr($port_name, 'null') && $i != 0) continue;
		
		$hcc = "style=background-color:#daedf4";
		
		if(isset($res[$i]['call_dur'])){
			$call_dur = $res[$i]['call_dur'];
		}else{
			$call_dur = '';
		}
		
		if(isset($res[$i]['call_times'])){
			$call_times = $res[$i]['call_times'];
		}else{
			$call_times = '';
		}
		
		if(isset($res[$i]['call_answers'])){
			$call_answers = $res[$i]['call_answers'];
		}else{
			$call_answers = '';
		}
		
		if(isset($res[$i]['handle_type'])){
			$handle_type = $res[$i]['handle_type'];
		}else{
			$handle_type = '';
		}
		
		if(isset($res[$i]['online_time'])){
			$online_time = $res[$i]['online_time'];
		}else{
			$online_time = '';
		}
?>
		<tr>
			<td <?php if($i==0) {echo $hcc;}?>>
				<input type="checkbox" name="<?php echo 'channel'.$i;?>" id="<?php echo 'channel'.$i;?>" />
				<input type="hidden" name="get_chan[]" class="get_chan" value="<?php echo $i;?>" />
			</td>
			
			<td <?php if($i==0) {echo $hcc;}?>>
				<?php if($i != 0){echo $port_name;}?>
			</td>
			
			<td <?php if($i==0) {echo $hcc;}?>>
				<input type="text" name="<?php echo "call_dur".$i;?>" id="<?php echo "call_dur".$i;?>" value="<?php echo $call_dur;?>" />
			</td>
			
			<td <?php if($i==0) {echo $hcc;}?>>
				<input type="text" name="<?php echo "call_times".$i;?>" id="<?php echo "call_times".$i;?>" value="<?php echo $call_times;?>" />
			</td>
			
			<td <?php if($i==0) {echo $hcc;}?>>
				<input type="text" name="<?php echo "call_answers".$i;?>" id="<?php echo "call_answers".$i;?>" value="<?php echo $call_answers;?>" />
			</td>
			
			<td <?php if($i==0) {echo $hcc;}?>>
				<input type="text" name="<?php echo "online_time".$i;?>" id="<?php echo "online_time".$i?>" value="<?php echo $online_time;?>" />
			</td>
			
			<td <?php if($i==0) {echo $hcc;}?>>
				<select name="<?php echo "handle_type".$i;?>" id="<?php echo "handle_type".$i;?>">
					<option value=""><?php echo language('_None','None');?></option>
					<option value="call_internal" <?php if($handle_type == 'call_internal') echo 'selected';?>><?php echo language('Call Internal');?></option>
				</select>
			</td>
			
			<td <?php if($i==0) {echo $hcc;}?>>
				<?php if($i != 0){?>
				<button type="submit" value="Modify" style="width:32px;height:32px;" onclick="document.getElementById('send').value='Save';document.getElementById('sel_channel').value=<?php echo $i;?>">
					<img src="/images/save.png">
				</button>
				<?php } ?>
			</td>
		</tr>
<?php
	}
?>
	</table>
	
	<div id="newline"></div>
	
	<table id="float_btn" class="float_btn">
		<tr id="float_btn_tr" class="float_btn_tr">
			<td>
				<input type="hidden" name="send" id="send" value="Save" />
				<input type="submit" id="sendlabel" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>
			</td>
			<td>
				<input type="button" value="<?php echo language('Cancel');?>" onclick="location.reload()" />
			</td>
			<td>
				<input type="button" value="<?php echo language('Batch');?>" onclick="check_batch();setValue();" />
			</td>
		</tr>
	</table>
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td style="width:51px;">
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>
			</td>
			<td style="width:51px;">
				<input type="button" id="float_button_2" class="float_short_button" value="<?php echo language('Cancel');?>" onclick="location.reload()" />
			</td>
			<td style="width:51px;">
				<input type="button" id="float_button_3" name="" class="float_short_button" value="<?php echo language('Batch');?>" onclick="check_batch();setValue();" />
			</td>
		</tr>
	</table>
</form>

<script>
	function selectall(checked){
		if(checked == true){
			$(":checkbox").attr("checked", true);
		}else{
			$(":checkbox").removeAttr("checked");
		}
	}
	
	function setValue(){
		var call_dur = document.getElementById('call_dur0').value;
		var call_times = document.getElementById('call_times0').value;
		var call_answers = document.getElementById('call_answers0').value;
		var online_time = document.getElementById('online_time0').value;
		var handle_type = document.getElementById('handle_type0').value;
		
		$(".get_chan").each(function(){
			var channel = $(this).val();
			if(document.getElementById('channel'+channel).checked == true){
				document.getElementById('call_dur'+channel).value = call_dur;
				document.getElementById('call_times'+channel).value = call_times;
				document.getElementById('call_answers'+channel).value = call_answers;
				document.getElementById('online_time'+channel).value = online_time;
				document.getElementById('handle_type'+channel).value = handle_type;
			}
		});
	}
	
	function check_batch(){
		var flag = 0;
		$(":checkbox").each(function(){
			if($(this).attr('checked') == 'checked'){
				flag = 1;
				return false;
			}
		});
		if(flag == 0){
			alert('Select port alert');
			return false;
		}
		return true;
	}
	
	//auto reset data
	$("#fixed_time_calltime" ).datetimepicker({
		dateFormat: "yy-mm-dd", 
		timeFormat: "HH:mm:ss", 
		beforeShow: function(){
			setTimeout(function(){
				$('#ui-datepicker-div').css("z-index",5);
			},0);
		}
	});
	$("#fixed_time_calltime" ).datetimepicker();
	
	function check_dl_datetime(id){
		var obj = document.getElementById(id);
		obj.value = obj.value.replace(/[^-: \d]*/g,'');

		val = obj.value;
		
		if(!check_datetime(val)) {
			$("#c"+id).html(con_str("<?php echo language('js check datetime','Please input right format date and time. eg: 2013-09-03 19:44:31');?>"));
			return false;
		} else {
			$("#c"+id).html("");
		}
		
		//compare time
		var current_time = Date.parse(new Date());
		var time_set_temp = document.getElementById('fixed_time_calltime').value;
		var time_set = Date.parse(time_set_temp);
		if(time_set < current_time){
			$("#c"+id).html(con_str("<?php echo language('Fixed Time Call Compare help','The time set cannot be less than the current time.');?>"));
			return false;
		}else{
			$("#c"+id).html("");
		}
		
		return true;
	}
	
	function check(){
		var callmonitor_switch = document.getElementById('callmonitor_switch').checked;
		var fixed_time_call_switch =document.getElementById('fixed_time_call_switch').checked;
		
		if(callmonitor_switch && fixed_time_call_switch){
			if(!check_dl_datetime('fixed_time_calltime')){
				document.getElementById('fixed_time_calltime').focus();
				return false;
			}
		}
		
		return true;
	}
	
	$("#fixed_time_call_switch").change(function(){
		if($(this).attr('checked') == 'checked'){
			$(".fixed_time_call_show").show();
		}else{
			$(".fixed_time_call_show").hide();
		}
	});
	
	$("#callmonitor_switch").change(function(){
		if($(this).attr('checked') == 'checked'){
			$(".callmonitor_switch_show").show();
		}else{
			$(".callmonitor_switch_show").hide();
		}
	});
	
	(function(){
		$("#callmonitor_switch").iButton();
		$("#fixed_time_call_switch").iButton();
		
		if($("#fixed_time_call_switch").attr('checked') == 'checked'){
			$(".fixed_time_call_show").show();
		}else{
			$(".fixed_time_call_show").hide();
		}
		
		if($("#callmonitor_switch").attr('checked') == 'checked'){
			$(".callmonitor_switch_show").show();
		}else{
			$(".callmonitor_switch_show").hide();
		}
	})();
</script>

<?php 
}

function save_auto(){
	global $__GSM_SUM__;
	$conf_path = '/etc/asterisk/gw/call_monitor.conf';
	$hlock = lock_file($conf_path);
	if(!file_exists($conf_path)) exec("touch /etc/asterisk/gw/call_monitor.conf");
	
	$aql = new aql();
	$aql->set('basedir','/ect/asterisk/gw/');
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	
	$res = $aql->query("select * from call_monitor.conf");
	
	if(!isset($res['general'])){
		$aql->assign_addsection('general','');
	}
	
	if(isset($_POST['callmonitor_switch'])){
		$callmonitor_switch = 'yes';
	}else{
		$callmonitor_switch = 'no';
	}
	if(isset($res['general']['callmonitor_switch'])){
		$aql->assign_editkey('general','callmonitor_switch',$callmonitor_switch);
	}else{
		$aql->assign_append('general','callmonitor_switch',$callmonitor_switch);
	}
	
	if(isset($_POST['fixed_time_call_switch'])){
		$fixed_time_call_switch = 'yes';
	}else{
		$fixed_time_call_switch = 'no';
	}
	if(isset($res['general']['fixed_time_call_switch'])){
		$aql->assign_editkey('general','fixed_time_call_switch',$fixed_time_call_switch);
	}else{
		$aql->assign_append('general','fixed_time_call_switch',$fixed_time_call_switch);
	}
	
	if(isset($_POST['fixed_time_call_type'])){
		$fixed_time_call_type = $_POST['fixed_time_call_type'];
	}else{
		$fixed_time_call_type = '';
	}
	if(isset($res['general']['fixed_time_call_type'])){
		$aql->assign_editkey('general','fixed_time_call_type',$fixed_time_call_type);
	}else{
		$aql->assign_append('general','fixed_time_call_type',$fixed_time_call_type);
	}
	
	if(isset($_POST['fixed_time_calltime'])){
		$fixed_time_calltime = $_POST['fixed_time_calltime'];
	}else{
		$fixed_time_calltime = '';
	}
	if(isset($res['general']['fixed_time_calltime'])){
		$aql->assign_editkey('general','fixed_time_calltime',$fixed_time_calltime);
	}else{
		$aql->assign_append('general','fixed_time_calltime',$fixed_time_calltime);
	}
	
	$start_hour = sprintf('%02s', $_POST['start_hour']);
	$start_min = sprintf('%02s', $_POST['start_min']);
	$start_time = $start_hour.':'.$start_min.':00';
	if(isset($res['general']['start_time'])){
		$aql->assign_editkey('general','start_time',$start_time);
	}else{
		$aql->assign_append('general','start_time',$start_time);
	}
	
	$end_hour = sprintf('%02s', $_POST['end_hour']);
	$end_min = sprintf('%02s', $_POST['end_min']);
	$end_time = $end_hour.':'.$end_min.':00';
	if(isset($res['general']['end_time'])){
		$aql->assign_editkey('general','end_time',$end_time);
	}else{
		$aql->assign_append('general','end_time',$end_time);
	}
	
	if(isset($res['general']['call_max_time'])){
		$aql->assign_editkey('general','call_max_time',$_POST['call_max_time']);
	}else{
		$aql->assign_append('general','call_max_time',$_POST['call_max_time']);
	}
	
	if(isset($res['general']['call_min_time'])){
		$aql->assign_editkey('general','call_min_time',$_POST['call_min_time']);
	}else{
		$aql->assign_append('general','call_min_time',$_POST['call_min_time']);
	}
	
	for($i=1;$i<count($_POST['get_chan']);$i++){
		$chan = $_POST['get_chan'][$i];
		if($chan != $_POST['sel_channel']){
			if($chan == 0) continue;
			if(!isset($_POST['channel'.$chan])) continue;
		}
		
		if(!isset($res[$chan])){
			$aql->assign_addsection($chan,'');
		}
		
		if(isset($res[$chan]['call_dur'])){
			$aql->assign_editkey($chan, 'call_dur', $_POST['call_dur'.$chan]);
		}else{
			$aql->assign_append($chan, 'call_dur', $_POST['call_dur'.$chan]);
		}
		
		if(isset($res[$chan]['call_times'])){
			$aql->assign_editkey($chan, 'call_times', $_POST['call_times'.$chan]);
		}else{
			$aql->assign_append($chan, 'call_times', $_POST['call_times'.$chan]);
		}
		
		if(isset($res[$chan]['call_answers'])){
			$aql->assign_editkey($chan, 'call_answers', $_POST['call_answers'.$chan]);
		}else{
			$aql->assign_append($chan, 'call_answers', $_POST['call_answers'.$chan]);
		}
		
		if(isset($res[$chan]['online_time'])){
			$aql->assign_editkey($chan, 'online_time', $_POST['online_time'.$chan]);
		}else{
			$aql->assign_append($chan, 'online_time', $_POST['online_time'.$chan]);
		}
		
		if(isset($res[$chan]['handle_type'])){
			$aql->assign_editkey($chan, 'handle_type', $_POST['handle_type'.$chan]);
		}else{
			$aql->assign_append($chan, 'handle_type', $_POST['handle_type'.$chan]);
		}
	}
	
	if(!$aql->save_config_file('call_monitor.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	
	//save to /data/log/call_monitor_status.conf
	$status_conf = '/data/log/call_monitor_status.conf';
	if(!file_exists($conf_path)) exec("touch /data/log/call_monitor_status.conf");
	
	$hlock = lock_file($status_conf);
	$aql->set('basedir','/data/log/');
	if(!$aql->open_config_file($status_conf)){
		echo $aql->get_error();
		unlock_file($hlock);
	}
	
	$status_res = $aql->query("select * from call_monitor_status.conf");
	
	if(!isset($status_res['general'])){
		$aql->assign_addsection('general','');
	}
	
	if(isset($status_res['general']['next_fixed_time'])){
		$aql->assign_editkey('general','next_fixed_time',$fixed_time_calltime);
	}else{
		$aql->assign_append('general','next_fixed_time',$fixed_time_calltime);
	}
	
	if(!$aql->save_config_file('call_monitor_status.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
	}
	unlock_file($hlock);
	
	save_routings_to_extensions();
	
	wait_apply("exec","/etc/init.d/callmonitor.sh reload");
	wait_apply("exec","asterisk -rx \"dialplan reload\"");
}

if($_POST){
	if(isset($_POST['send']) && $_POST['send'] == 'Save'){
		save_auto();
	}
	show_auto();
}else{
	show_auto();
}

require("/www/cgi-bin/inc/boot.inc");
?>

<div id="to_top"></div>
<div id="float_btn1" class="sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()" >
</div>