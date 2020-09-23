<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
include_once("/www/cgi-bin/inc/aql.php");
?>

<script type="text/javascript" src="/js/check.js?v=1.0"></script>
<script type="text/javascript" src="/js/functions.js"></script>

<?php
function show_gsm_calllimit(){
	global $__GSM_SUM__;
	global $only_view;

	$res_arr = [];
	for($i=1;$i<=$__GSM_SUM__;$i++){
		$port = get_gsm_name_by_channel_for_showtype($i);
		$channel_type = get_gsm_type_by_channel($i,1);
		
		$cmd_res = [];
		exec("/my_tools/set_calllimit.sh status $i | grep -E 'day_cur_sms|mon_cur_sms|limit_sta|call_fail_lock_status|call_fail_mark_status|sms_limit_sta|call_time_limit_sta|day_total_calls|hour_total_calls|day_total_answers|call_failed_count|call_time_count'",$cmd_res);
		for($j=0;$j<count($cmd_res);$j++){
			//general
			if(strstr($cmd_res[$j],'sms_limit_sta')){
				$sms_limit_sta = $cmd_res[$j];
			}else if(strstr($cmd_res[$j],'call_fail_lock_status')){
				$unlock_sta = $cmd_res[$j];
			}else if(strstr($cmd_res[$j],'call_fail_mark_status')){
				$unmark_sta = $cmd_res[$j];
			}else if(strstr($cmd_res[$j],'call_time_limit_sta')){
				$call_time_limit_sta = $cmd_res[$j];
			}else if(strstr($cmd_res[$j],'limit_sta')){
				$limit_sta = $cmd_res[$j];
			//call statistics
			}else if(strstr($cmd_res[$j],'day_total_calls')){
				$day_total_calls_num = $cmd_res[$j];
			}else if(strstr($cmd_res[$j],'hour_total_calls')){
				$hour_total_calls_num = $cmd_res[$j];
			}else if(strstr($cmd_res[$j],'day_total_answers')){
				$day_total_answers_num = $cmd_res[$j];
			}else if(strstr($cmd_res[$j],'call_failed_count')){
				$call_failed_count_num = $cmd_res[$j];
			}else if(strstr($cmd_res[$j],'call_time_count')){
				$call_time_count_num = $cmd_res[$j];
			//sms send statistics
			}else if(strstr($cmd_res[$j],'day_cur_sms')){
				$day_sms_num = $cmd_res[$j];
			}else if(strstr($cmd_res[$j],'mon_cur_sms')){
				$mon_sms_num = $cmd_res[$j];
			}
		}
		
		$limit_temp = explode(': ',$limit_sta);
		$call_time_limit_temp = explode(':',$call_time_limit_sta);
		$unlock_temp = explode(':',$unlock_sta);
		$unmake_temp = explode(':',$unmark_sta);
		$sms_limit_temp = explode(':',$sms_limit_sta);
		
		$hour_total_calls_temp = explode(': ',$hour_total_calls_num);
		$day_total_calls_temp = explode(': ',$day_total_calls_num);
		$day_total_answers_temp = explode(': ',$day_total_answers_num);
		$call_failed_count_temp = explode(': ',$call_failed_count_num);
		$call_time_count_temp = explode(':',$call_time_count_num);
		
		$day_sms_temp = explode(': ',$day_sms_num);
		$mon_sms_temp = explode(': ',$mon_sms_num);
		
		$res_arr[$i]['port'] = get_gsm_name_by_channel_for_showtype($i);
		$res_arr[$i]['channel_type'] = get_gsm_type_by_channel($i,1);
		
		$res_arr[$i]['limit_temp'] = $limit_temp[1];
		$res_arr[$i]['call_time_limit_temp'] = $call_time_limit_temp[1];
		$res_arr[$i]['unlock_temp'] = $unlock_temp[1];
		$res_arr[$i]['unmake_temp'] = $unmake_temp[1];
		$res_arr[$i]['sms_limit_temp'] = $sms_limit_temp[1];
		
		$res_arr[$i]['hour_total_calls_temp'] = $hour_total_calls_temp[1];
		$res_arr[$i]['day_total_calls_temp'] = $day_total_calls_temp[1];
		$res_arr[$i]['day_total_answers_temp'] = $day_total_answers_temp[1];
		$res_arr[$i]['call_failed_count_temp'] = $call_failed_count_temp[1];
		$res_arr[$i]['call_time_count_temp'] = $call_time_count_temp[1];
		
		$res_arr[$i]['day_sms_temp'] = $day_sms_temp[1];
		$res_arr[$i]['mon_sms_temp'] = $mon_sms_temp[1];
	}
	
	//phone number switch
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$phonenum_res = $aql->query("select * from sim_query.conf");
	
?>
	<form enctype="multipart/form-data" action="<?php echo get_self();?>" method="get" >
		<input type="hidden" id="sel_gsm" name="sel_gsm" value="" />

		<div class="content">
			<table class="table_show">
				<tr>
					<th width="150px"><?php echo language('Port');?></th>
					
					<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
					<th width="130px"><?php echo language('Mobile Number');?></th>
					<?php } ?>
					
					<th width="70px;"><?php echo language('Type');?></th>
					<th width="260px"><?php echo language('Call Status');?></th>
					<th width="260px"><?php echo language('Lock Status');?></th>
					<th width="130px"><?php echo language('Mark Status');?></th>
					<th width="130px"><?php echo language('SMS Status');?></th>
					<th width="38px"><?php echo language('Actions');?></th>
				</tr>
				
				<?php 
				for($i=1;$i<=$__GSM_SUM__;$i++){
					$port = $res_arr[$i]['port'];
					$channel_type = $res_arr[$i]['channel_type'];
					
					$phonenum = '';
					if(($phonenum_res[$i]['query_type']&240) != 0){
						exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $i",$phone_output);
						$phonenum = $phone_output[0];
						$phone_output = '';
					}
				?>
				<tr <?php if($channel_type == 'NULL') echo 'style="display:none;"'?>>
					<td><?php echo $port;?></td>
					
					<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
					<td><?php echo $phonenum;?></td>
					<?php } ?>
					
					<td><?php echo $channel_type;?></td>
					<td>
					<?php
						if($res_arr[$i]['limit_temp'] == 'LIMIT' || $res_arr[$i]['call_time_limit_temp'] == 'LIMIT'){
							echo language('Limited');
						}else{
							echo language('Unlimited');
						}
					?>
					<?php if(!$only_view){ ?>
						<button type="button" value="Reset" style="width:32px;height:32px;float:right;margin-right:30px;padding:0;"
							onclick="Unlimit('<?php echo $i; ?>','<?php echo $res_arr[$i]['limit_temp'];?>','<?php echo $res_arr[$i]['call_time_limit_temp'];?>')">
							<?php if($res_arr[$i]['limit_temp'] == 'LIMIT' || $res_arr[$i]['call_time_limit_temp'] == 'LIMIT'){?>
							<img src="/images/limit.png">
							<?php }else{?>
							<img src="/images/unlimit.png">
							<?php }?>
						</button>
					<?php } ?>
					</td>
					<td>
					<?php 
						if($res_arr[$i]['unlock_temp'] == 1){
							echo language('Locked');
						}else{
							echo language('Unlocked');
						}
					?>
					<?php if(!$only_view){ ?>
						<button type="button" value="Reset" style="width:32px;height:32px;float:right;margin-right:30px;padding:0;"
							onclick="Unlock('<?php echo $i; ?>','<?php echo $res_arr[$i]['unlock_temp'];?>')">
							<?php if($res_arr[$i]['unlock_temp'] == 1){?>
							<img src="/images/lock.png">
							<?php }else{?>
							<img src="/images/unlock.png?v=1">
							<?php }?>
						</button>
					<?php } ?>
					</td>
					<td>
					<?php
						if($res_arr[$i]['unmake_temp'] == 1){
							echo language('Marked');
						}else{
							echo language('Unmarked');
						}
					?>
					</td>
					<td>
					<?php 
						if($res_arr[$i]['sms_limit_temp'] == 1){
							echo language('Limited');
						}else{
							echo language('Unlimited');
						}
					?>
					</td>
					<td>
						<button type="submit" value="Modify" style="width:32px;height:32px;padding:0;"
							onclick="document.getElementById('send').value='Modify';setValue('<?php echo $i;?>', 1)">
							<img src="/images/edit.gif">
						</button>
					</td>
				</tr>
				<?php
				}
				?>
			</table>
		</div>
		
		<div class="content">
			<span class="title"><?php echo language('Call Statistics','Call Statistics');?></span>
			
			<table class="table_show">
				<tr>
					<th width="166px"><?php echo language('Port');?></th>
					
					<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
					<th width="126px"><?php echo language('Mobile Number');?></th>
					<?php } ?>
					
					<th width="78px"><?php echo language('Type');?></th>
					<th width="142px"><?php echo language('Hour Call Count');?></th>
					<th width="142px"><?php echo language('Daily Call Count');?></th>
					<th width="289px"><?php echo language('Daily Answer Count');?></th>
					<th width="145px"><?php echo language('Call Failed Count');?></th>
					<th><?php echo language('Call Duration');?></th>
				</tr>
				
				<?php 
				for($i=1;$i<=$__GSM_SUM__;$i++){
					$port = $res_arr[$i]['port'];
					$channel_type = $res_arr[$i]['channel_type'];
					
					$phonenum = '';
					if(($res[$i]['query_type']&240) != 0){
						exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $i",$phone_output);
						$phonenum = $phone_output[0];
						$phone_output = '';
					}
				?>
				<tr <?php if($channel_type == 'NULL') echo 'style="display:none;"'?>>
					<td><?php echo $port;?></td>
					
					<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
					<td><?php echo $phonenum;?></td>
					<?php } ?>
					
					<td><?php echo $channel_type;?></td>
					<td><?php echo $res_arr[$i]['hour_total_calls_temp'];?></td>
					<td><?php echo $res_arr[$i]['day_total_calls_temp'];?></td>
					<td><?php echo $res_arr[$i]['day_total_answers_temp'];?></td>
					<td><?php echo $res_arr[$i]['call_failed_count_temp'];?></td>
					<td><?php echo $res_arr[$i]['call_time_count_temp'];?></td>
				</tr>
				<?php }?>
			</table>
		</div>
		
		<div class="content">
			<span class="title"><?php echo language('SMS Sending Statistics');?></span>
			
			<table class="table_show">
				<tr>
					<th style="width:0.3%;"><input type="checkbox" name="selall_port" onclick="selectAll(this.checked,'port[]')" /></th>
					<th width="137px"><?php echo language('Port');?></th>
					
					<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
					<th width="127px"><?php echo language('Mobile Number');?></th>
					<?php } ?>
					
					<th width="80px"><?php echo language('Type');?></th>
					<th width="142px"><?php echo language('SMS count of the day');?></th>
					<th width="142px"><?php echo language('Daily limit');?></th>
					<th><?php echo language('SMS count of the month');?></th>
					<th width="144px"><?php echo language('Monthly limit');?></th>
					<th width="192px"><?php echo language('Monthly recovery date');?></th>
				</tr>
				
				<?php 
				$aql = new aql();
				$aql->set('basedir','/etc/asterisk/gw/call_limit/');
				$res = $aql->query("select * from calllimit_settings.conf");
				
				for($i=1;$i<=$__GSM_SUM__;$i++){
					$port = $res_arr[$i]['port'];
					$channel_type = $res_arr[$i]['channel_type'];
					
					if($res_arr[$i]['day_sms_temp'] != ''){
						$day_sms = $res_arr[$i]['day_sms_temp'];
					}else{
						$day_sms = '--';
					}
					
					if($res_arr[$i]['mon_sms_temp'] != ''){
						$mon_sms = $res_arr[$i]['mon_sms_temp'];
					}else{
						$mon_sms = '--';
					}
					
					$phonenum = '';
					if(($res[$i]['query_type']&240) != 0){
						exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $i",$phone_output);
						$phonenum = $phone_output[0];
						$phone_output = '';
					}
				?>
				<tr <?php if($channel_type == 'NULL') echo 'style="display:none;"'?>>
					<td><input type="checkbox" class="sms_limit_checked" name="port[]" value="<?php echo $i;?>" /></td>
					<td><?php echo $port;?></td>
					
					<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
					<td><?php echo $phonenum;?></td>
					<?php } ?>
					
					<td><?php echo $channel_type;?></td>
					<td><?php echo $day_sms;?></td>
					<td><?php echo $res[$i]['day_sms_settings']?></td>
					<td><?php echo $mon_sms;?></td>
					<td><?php echo $res[$i]['mon_sms_settings']?></td>
					<td><?php echo $res[$i]['sms_clean_date']?></td>
				</tr>
				<?php }?>
			
				<tr>
					<td></td>
					<td></td>
					<td></td>
					<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
					<td></td>
					<?php } ?>
					<td>
					<?php if(!$only_view){ ?>
						<input type="submit" value="<?php echo language('Clear zero');?>" onclick="document.getElementById('send').value='Clean Day';return check_sel_port();"/>
					<?php } ?>
					</td>
					<td></td>
					<td>
					<?php if(!$only_view){ ?>
						<input type="submit" value="<?php echo language('Clear zero');?>" onclick="document.getElementById('send').value='Clean Month';return check_sel_port();"/>
					<?php } ?>
					</td>
					<td></td>
					<td></td>
				</tr>
			</table>
		</div>
		
		<br/>
		
		<input type="hidden" name="send" id="send" value="" />
	</form>
	
	<script>	
	function setValue(channel){
		document.getElementById('sel_gsm').value = channel;
	}
	
	function Unlock(channel,sim_lock_status){
		if(sim_lock_status == 1){
			var ret = confirm("<?php echo language('Unlock Tip','Are you sure you want to unlock it?');?>")
			if(ret){
				$.ajax({
					type:"GET",
					url:"ajax_server.php?type=sim_unlock&channel="+channel,
					success:function(data){
						window.location.reload();
					},
					error:function(){
						alert("Unlock failed!");
					}
				});
			}
		}else if(sim_lock_status == 2){
			alert('Locking!');
			window.location.reload();
		}else{
			alert('<?php echo language('Sim Status Unlock','Sim Status:Unlock.');?>');
		}
	}
	
	function Unlimit(channel,sim_limit_status,call_time_limit_sta){
		if(sim_limit_status == 'LIMIT' && call_time_limit_sta == 'LIMIT'){
			var ret = confirm("<?php echo language('Unlimit Tip@both_time','This operation will unlimited the call times limit, and the call duration limit needs to be removed by using the reset button within the settings. Are you sure you want to do this?')?>");
			if(ret){
				$.ajax({
					type:"GET",
					url:"ajax_server.php?type=sim_unlimit&channel="+channel,
					success:function(data){
						window.location.reload();
					},
					error:function(){
						alert("Unlimit failed!");
					}
				});
			}
		}else if(sim_limit_status == 'LIMIT'){
			var ret = confirm("<?php echo language('Unlimit Tip','This operation will unlimited the call times limit. Are you sure you want to do this?')?>");
			if(ret){
				$.ajax({
					type:"GET",
					url:"ajax_server.php?type=sim_unlimit&channel="+channel,
					success:function(data){
						window.location.reload();
					},
					error:function(){
						alert("Unlimit failed!");
					}
				});
			}
		}else if(call_time_limit_sta == 'LIMIT'){
			alert('<?php echo language('Unlimit Tip@only_time','The call duration limit needs to be removed by using the reset button within the settings.');?>');
		}else{
			alert('<?php echo language('Sim Status Unlimit','Sim Status:Unlimit.');?>');
		}
	}
	
	$("#show_sms_limit").click(function(){
		if($("#sms_limit_table").css('display') == 'none'){
			$(this).siblings('.tb_fold').addClass('tb_unfold');
			$(this).siblings('.tb_fold').removeClass('tb_fold');
			$("#sms_limit_table").show();
		}else{
			$(this).siblings('.tb_unfold').addClass('tb_fold');
			$(this).siblings('.tb_unfold').removeClass('tb_unfold');
			$("#sms_limit_table").hide();
		}
	});
	
	$("#show_call_statistics").click(function(){
		if($("#call_statistics_table").css('display') == 'none'){
			$(this).siblings('.tb_fold').addClass('tb_unfold');
			$(this).siblings('.tb_fold').removeClass('tb_fold');
			$("#call_statistics_table").show();
		}else{
			$(this).siblings('.tb_unfold').addClass('tb_fold');
			$(this).siblings('.tb_unfold').removeClass('tb_unfold');
			$("#call_statistics_table").hide();
		}
	});
	
	function check_sel_port(){
		var flag = 0;
		$(".sms_limit_checked").each(function(){
			if($(this).attr('checked') == 'checked'){
				flag = 1;
			}
		});
		
		if(flag == 0){
			alert("<?php echo language('Select port alert');?>");
			return false;
		}
	}
	</script>
<?php
}

function edit_gsm_calllimit(){
	global $__GSM_SUM__;
	global $only_view;
	
	$lang_sync_title = language('Synchronization option');
	
	$channel = $_GET['sel_gsm'];
	
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw/call_limit/');
	$res = $aql->query("select * from calllimit_settings.conf");
	
	//call limit
	if($res[$channel]['call_limit_switch'] == '1'){
		$call_limit_switch = 'checked';
	}else{
		$call_limit_switch = '';
	}
	
	$day_calls_settings = $res[$channel]['day_calls_settings'];
	
	$day_answer_setting = $res[$channel]['day_answer_setting'];
	
	$hour_calls_settings = $res[$channel]['hour_calls_settings'];
	
	//call limit time
	if($res[$channel]['call_time_switch'] == 1){
		$call_time_switch = 'checked';
	}else{
		$call_time_switch = '';
	}
	
	if(isset($res[$channel]['call_time_step'])){
		$call_time_step = $res[$channel]['call_time_step'];
		if($call_time_step == '0'){
			$call_time_step = '60';
		}
	}else{
		$call_time_step = '60';
	}
	
	if($res[$channel]['call_time_single_switch'] == 1){
		$call_time_single_switch = 'checked';
	}else{
		$call_time_single_switch = '';
	}
	
	if(isset($res[$channel]['call_time_settings'])){
		$call_time_settings = $res[$channel]['call_time_settings'];
	}else{
		$call_time_settings = '';
	}
	
	if($res[$channel]['call_time_total_switch'] == 1){
		$call_time_total_switch = 'checked';
	}else{
		$call_time_total_switch = '';
	}
	
	if(isset($res[$channel]['call_time_total'])){
		$call_time_total = $res[$channel]['call_time_total'];
	}else{
		$call_time_total = '';
	}
	
	if(isset($res[$channel]['call_time_free'])){
		$call_time_free = $res[$channel]['call_time_free'];
	}else{
		$call_time_free = '0';
	}
	
	if(isset($res[$channel]['call_time_warning_num'])){
		$call_time_warning_num = $res[$channel]['call_time_warning_num'];
	}else{
		$call_time_warning_num = '0';
	}
	
	if(isset($res[$channel]['call_time_warning_callee'])){
		$call_time_warning_callee = $res[$channel]['call_time_warning_callee'];
	}else{
		$call_time_warning_callee = '';
	}
	
	if(isset($res[$channel]['call_time_warning_msg'])){
		$call_time_warning_msg = $res[$channel]['call_time_warning_msg'];
	}else{
		$call_time_warning_msg = '';
	}
	
	if(isset($res[$channel]['call_time_remain'])){
		$call_time_remain = $res[$channel]['call_time_remain'];
	}else{
		$call_time_remain = '';
	}
	
	if($res[$channel]['call_time_clean_switch'] == 1){
		$call_time_clean_switch = 'checked';
	}else{
		$call_time_clean_switch = '';
	}
	
	if(isset($res[$channel]['call_time_clean_type'])){
		$call_time_clean_type = $res[$channel]['call_time_clean_type'];
	}else{
		$call_time_clean_type = '1';
	}
	
	if(isset($res[$channel]['call_time_clean_date'])){
		$call_time_clean_date = $res[$channel]['call_time_clean_date'];
	}else{
		$call_time_clean_date = '';
	}
	
	$temp = exec("/my_tools/set_calllimit.sh status $channel | grep call_time_clean_date");
	$temp_arr = explode(':',$temp,2);
	$call_time_clean_date = $temp_arr[1];
	if($call_time_clean_date == ''){
		$call_time_clean_date = `date "+%Y-%m-%d %H:%M:%S"`;
	}
	$cur_time = trim(`date "+%Y-%m-%d %H:%M:%S"`);
	$call_time_clean_date = date("d/m/Y H:i:s",strtotime($call_time_clean_date));
	
	$temp = exec("/my_tools/set_calllimit.sh status $channel | grep call_time_remain");
	$temp_arr = explode(':',$temp,2);
	$call_time_remain = $temp_arr[1];
	if($call_time_remain == ''){
		$call_time_remain = language('No Limit');
	}
	
	//lock card
	if($res[$channel]['call_detect_flag'] == '1'){
		$call_detect_flag = 'checked';
	}else{
		$call_detect_flag = '';
	}
	
	if($res[$channel]['call_fail_mark_flag'] == '1'){
		$call_fail_mark_flag = 'checked';
	}else{
		$call_fail_mark_flag = '';
	}
	
	$call_fail_mark_count = $res[$channel]['call_fail_mark_count'];
	
	if($res[$channel]['call_fail_lock_flag'] == '1'){
		$call_fail_lock_flag = 'checked';
	}else{
		$call_fail_lock_flag = '';
	}
	
	$call_fail_lock_count = $res[$channel]['call_fail_lock_count'];
	
	if($res[$channel]['call_fail_lock_sms_flag'] == '1'){
		$call_fail_lock_sms_flag = 'checked';
	}else{
		$call_fail_lock_sms_flag = '';
	}
	
	$call_fail_lock_sms_count = $res[$channel]['call_fail_lock_sms_count'];
	
	$call_fail_lock_sms_callee = $res[$channel]['call_fail_lock_sms_callee'];
	
	$call_fail_lock_sms_msg = $res[$channel]['call_fail_lock_sms_msg'];
	
	if($res[$channel]['call_fail_lock_sms_report_flag'] == '1'){
		$call_fail_lock_sms_report_flag = 'checked';
	}else{
		$call_fail_lock_sms_report_flag = '';
	}
	
	//sms limit
	if($res[$channel]['sms_limit_switch'] == '1'){
		$sms_limit_switch = 'checked';
	}else{
		$sms_limit_switch = '';
	}
	
	if($res[$channel]['sms_limit_success_flag'] == '1'){
		$sms_limit_success_flag = 'checked';
	}else{
		$sms_limit_success_flag = '';
	}
	
	if($res[$channel]['time_sms_swtich'] == '1'){
		$time_sms_swtich = 'checked';
	}else{
		$time_sms_swtich = '';
	}
	
	if(isset($res[$channel]['time_sms_start'])){
		$temp = explode(':', $res[$channel]['time_sms_start']);
		$sms_start_hour = intval($temp[0]);
		$sms_start_minute = intval($temp[1]);
	}else{
		$sms_start_hour = 0;
		$sms_start_minute = 0;
	}
	
	if(isset($res[$channel]['time_sms_end'])){
		$temp = explode(':', $res[$channel]['time_sms_end']);
		$sms_end_hour = intval($temp[0]);
		$sms_end_minute = intval($temp[1]);
	}else{
		$sms_end_hour = 0;
		$sms_end_minute = 0;
	}
	
	$day_sms_settings = $res[$channel]['day_sms_settings'];
	
	$mon_sms_settings = $res[$channel]['mon_sms_settings'];
	
	$sms_clean_date = $res[$channel]['sms_clean_date'];
?>
	<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
	<link type="text/css" href="/css/jquery-ui-timepicker-addon.css" rel="stylesheet" media="all"/>

	<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>
	<script type="text/javascript" src="/js/jquery-ui-timepicker-addon.js"></script>
	<script type="text/javascript" src="/js/jquery-ui-sliderAccess.js"></script>
	<form id="manform" enctype="multipart/form-data" action="<?php echo get_self();?>" method="post">
		<input type="hidden" name="channel" id="channel" value="<?php echo $channel;?>" />
		
		<div class="content">
			<span class="title">
				<?php echo language('Call Limit','Call Limit');?> (<?php echo get_gsm_name_by_channel($channel,1,false);?>)
				
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<?php if(is_show_language_help('Call Limit Switch')){ ?>
								<b><?php echo language('Call Limit Switch');?>:</b><br/>
								<?php echo language('Call Limit Switch help','Call Limit Switch');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Limit Daily Call Times')){ ?>
								<b><?php echo language('Limit Daily Call Times');?>:</b><br/>
								<?php echo language('Limit Daily Call Times help', 'Limit the number of calls per day (regardless of whether the person is connected), the default value is 0, indicating unlimited.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Limit Daily Answer Times')){ ?>
								<b><?php echo language('Limit Daily Answer Times');?>:</b><br/>
								<?php echo language('Limit Daily Answer Times help', 'Limit the number of calls per day and the number of answers. The default value is 0, indicating no limit.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Limit Hour Call Times')){ ?>
								<b><?php echo language('Limit Hour Call Times');?>:</b><br/>
								<?php echo language('Limit Hour Call Times help', 'Limit the number of calls per hour (regardless of whether the person is connected), the default value is 0, indicating unlimited.');?>
							<?php } ?>
						</div>
					</div>
				</div>
			</span>
			
			<div class="tab_item">
				<span>
					<?php echo language('Call Limit Switch');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_limit_switch"></span>
					<input type="checkbox" name="call_limit_switch_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<span><input type="checkbox" name="call_limit_switch" id="call_limit_switch" <?php echo $call_limit_switch;?> /></span>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Limit Daily Call Times');?>:
				</span>
				<div class="tab_item_right">
					<span id="cday_calls_settings"></span>
					<input type="checkbox" name="day_calls_settings_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="text" name="day_calls_settings" id="day_calls_settings" value="<?php echo $day_calls_settings;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Limit Daily Answer Times');?>:
				</span>
				<div class="tab_item_right">
					<span id="cday_answer_setting"></span>
					<input type="checkbox" name="day_answer_setting_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="text" name="day_answer_setting" id="day_answer_setting" value="<?php echo $day_answer_setting;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Limit Hour Call Times');?>:
				</span>
				<div class="tab_item_right">
					<span id="chour_calls_settings"></span>
					<input type="checkbox" name="hour_calls_settings_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="text" name="hour_calls_settings" id="hour_calls_settings" value="<?php echo $hour_calls_settings;?>" />
				</div>
			</div>
		</div>
		
		<div class="content">
			<span class="title">
				<?php echo language('Call Limit Time');?>
				
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<?php if(is_show_language_help('Call Time Limit Switch')){ ?>
								<b><?php echo language('Call Time Limit Switch');?>:</b><br/>
								<?php echo language('Call Time Limit Switch help','Call Time Limit Switch');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Step')){ ?>
								<b><?php echo language('Step');?>:</b><br/>
								<?php echo language('Step help','Step length value range is 1-999.<br>Step length multiplied by time of single call just said a single call duration time allowed.<br/>Unit: seconds.');?>
							
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Enable Single Call Duration Limit')){ ?>
								<b><?php echo language('Enable Single Call Duration Limit');?>:</b><br/>
								<?php echo language('Enable Single Call Duration Limit help','Definite maximum call duration for single call. <br>Example: if Time of single call set to 10, the call will be disconnected after <br>talking 10*step seconds.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Single Call Duration Limitation')){ ?>
								<b><?php echo language('Single Call Duration Limitation');?>:</b><br/>
								<?php echo language('Single Call Duration Limitation help','The value of limitation single call, this value range is 1-999999. <br>Step length multiplied by time of single call just said a single call duration time allowed.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Enable Call Duration Limitation')){ ?>
								<b><?php echo language('Enable Call Duration Limitation');?>:</b><br/>
								<?php echo language('Enable Call Duration Limitation help','This function is to limit the total call duration of GSM channel. <br>The max call duration is between 1 to 999999 steps.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Call Duration Limitation')){ ?>
								<b><?php echo language('Call Duration Limitation');?>:</b><br/>
								<?php echo language('Call Duration Limitation help','The value of total call limitation, this value range is 1-999999. <br>Step length multiplied by time of single call just said a single call duration time allowed.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Minimum Charging Time')){ ?>
								<b><?php echo language('Minimum Charging Time');?>:</b><br/>
								<?php echo language('Minimum Charging Time help','A single call over this time, <br>GSM side of the operators began to collect fees, unit for seconds.<br>The value must be less than Step length.<br/>Unit: seconds.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Alarm Threshold')){ ?>
								<b><?php echo language('Alarm Threshold');?>:</b><br/>
								<?php echo language('Alarm Threshold help','Define a threshold value of call duration. <br>While the call steps equal to(or less than) this value, <br>the gateway will send alarm information to designated phone number via SMS(Send only once before the total call length is reset).<br>The value must be less than call duration limitation.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Alarm Phone Number')){ ?>
								<b><?php echo language('Alarm Phone Number');?>:</b><br/>
								<?php echo language('Alarm Phone Number help','Receiving alarm phone number, user will received alarm message from gateway.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Alarm Description')){ ?>
								<b><?php echo language('Alarm Description');?>:</b><br/>
								<?php echo language('Alarm Description help','Alarm port information description, <br>which will be sent to user mobile phone with alarm information.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Remain Time')){ ?>
								<b><?php echo language('Remain Time');?>:</b><br/>
								<?php echo language('Remain Time help','This value is multiplied by to step length is a rest call time.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Enable Auto Reset')){ ?>
								<b><?php echo language('Enable Auto Reset');?>:</b><br/>
								<?php echo language('Enable Auto Reset help','Automatic restore remaining talk time, <br>that is, get total call minutes of GSM channel.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Auto Reset Type')){ ?>
								<b><?php echo language('Auto Reset Type');?>:</b><br/>
								<?php echo language('Auto Reset Type help','Reset call minutes by date, by week, by month.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Next Reset Time')){ ?>
								<b><?php echo language('Next Reset Time');?>:</b><br/>
								<?php echo language('Next Reset Time help','Defined next reset date. <br>System will count start from that date and work as Reset Period setting.');?>
							<?php } ?>
						</div>
					</div>
				</div>
			</span>
			
			<div class="tab_item">
				<span>
					<?php echo language('Call Time Limit Switch');?>:
				</span>
				<div class="tab_item_right">
					<input type="checkbox" name="call_time_switch_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<span><input type="checkbox" name="call_time_switch" id="call_time_switch" <?php echo $call_time_switch; ?> /></span>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Step');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_time_step"></span>
					<input type="checkbox" name="call_time_step_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="call_time_step" id="call_time_step" maxlength="3" value="<?php echo $call_time_step;?>" oninput="check_step(this.id);" onkeyup="check_step(this.id);"/>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Enable Single Call Duration Limit');?>:
				</span>
				<div class="tab_item_right">
					<input type="checkbox" name="call_time_single_switch_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<span><input type="checkbox" name="call_time_single_switch" id="call_time_single_switch" <?php echo $call_time_single_switch;?> /></span>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Single Call Duration Limitation');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_time_settings"></span>
					<input type="checkbox" name="call_time_settings_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="call_time_settings" id="call_time_settings" maxlength="6" value="<?php echo $call_time_settings;?>" oninput="check_dl_integer(this.id);" onkeyup="check_dl_integer(this.id);" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Enable Call Duration Limitation');?>:
				</span>
				<div class="tab_item_right">
					<input type="checkbox" name="call_time_total_switch_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<span><input type="checkbox" name="call_time_total_switch" id="call_time_total_switch" <?php echo $call_time_total_switch;?> /></span>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Call Duration Limitation');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_time_total"></span>
					<input type="checkbox" name="call_time_total_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="call_time_total" id="call_time_total" maxlength="6" value="<?php echo $call_time_total;?>" oninput="check_dl_integer(this.id);" onkeyup="check_dl_integer(this.id);" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Minimum Charging Time');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_time_free"></span>
					<input type="checkbox" name="call_time_free_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="call_time_free" id="call_time_free" maxlength="3" value="<?php echo $call_time_free;?>" oninput="check_freetime(this.id);" onkeyup="check_freetime(this.id);" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Alarm Threshold');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_time_warning_num"></span>
					<input type="checkbox" name="call_time_warning_num_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="call_time_warning_num" id="call_time_warning_num" maxlength="6" value="<?php echo $call_time_warning_num;?>" oninput="check_warning_time(this.id);" onkeyup="check_warning_time(this.id);" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Alarm Phone Number');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_time_warning_callee"></span>
					<input type="checkbox" name="call_time_warning_callee_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="call_time_warning_callee" id="call_time_warning_callee" value="<?php echo $call_time_warning_callee;?>" oninput="check_dl_integer(this.id)" onkeyup="check_dl_integer(this.id);" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Alarm Description');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_time_warning_msg"></span>
					<input type="checkbox" name="call_time_warning_msg_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="call_time_warning_msg" id="call_time_warning_msg" value="<?php echo $call_time_warning_msg;?>" size="50" oninput="check_warning_describe(this.id);" onkeyup="check_warning_describe(this.id);"/>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Remain Time');?>:
				</span>
				<div class="tab_item_right">
					
					<?php if(!$only_view){ ?>
					<input type="button" class="cbutton" value="<?php echo language('Reset');?>" onclick="remain_time_reset()" style="margin-right:10px;">
					<?php } ?>
					
					<input type="text" name="call_time_remain" id="call_time_remain" value="<?php echo $call_time_remain;?>" readonly disabled/>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Enable Auto Reset');?>:
				</span>
				<div class="tab_item_right">
					<input type="checkbox" name="call_time_clean_switch_sync" class="setting_sync" title="<?php echo lang_sync_title;?>" />
					<span><input type="checkbox" name="call_time_clean_switch" id="call_time_clean_switch" <?php echo $call_time_clean_switch;?> /></span>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Auto Reset Type');?>:
				</span>
				<div class="tab_item_right">
					<input type="checkbox" name="call_time_clean_type_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<select name="call_time_clean_type">
						<option value="1" <?php if($call_time_clean_type=='1') echo 'selected';?>><?php echo language('Day');?>(1<?php echo language('Day');?>)</option>
						<option value="2" <?php if($call_time_clean_type=='2') echo 'selected';?>><?php echo language('Week');?>(7<?php echo language('Day');?>)</option>
						<option value="3" <?php if($call_time_clean_type=='3') echo 'selected';?>><?php echo language('Month');?></option>
					</select>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Next Reset Time');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_time_clean_date"></span>
					<input type="checkbox" class="setting_sync" name="call_time_clean_date_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="call_time_clean_date" id="call_time_clean_date" value="<?php echo $call_time_clean_date;?>" onchange="check_dl_datetime(this.id);" />
				</div>
			</div>
		</div>
		
		<div class="content">
			<span class="title">
				<?php echo language('Lock Sim');?>
				
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<?php if(is_show_language_help('Lock Detect Switch')){ ?>
								<b><?php echo language('Lock Detect Switch');?>:</b><br/>
								<?php echo language('Lock Detect Switch help','Lock Detect Switch');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Mark Switch')){ ?>
								<b><?php echo language('Mark Switch');?>:</b><br/>
								<?php echo language('Mark Switch help','Mark Switch');?>

								<br/><br/>
							<?php } ?>

							<?php if(is_show_language_help('Call Failed Mark Count')){ ?>
								<b><?php echo language('Call Failed Mark Count');?>:</b><br/>
								<?php echo language('Call Failed Mark Count help','The number of consecutive calls failed to reach the number of settings, marking the port.');?>

								<br/><br/>
							<?php } ?>
	
							<?php if(is_show_language_help('Call Failed Lock Switch')){ ?>
								<b><?php echo language('Call Failed Lock Switch');?>:</b><br/>
								<?php echo language('Call Failed Lock Switch help','Call Failed Lock Switch');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Call Failed Lock Count')){ ?>
								<b><?php echo language('Call Failed Lock Count');?>:</b><br/>
								<?php echo language('Call Failed Lock Count help','Lock the port after the number of successive call failures reaches the set number, restrict the port expiration, and no longer select the port expiration in the group policy.');?>
								
								<br/><br/>
							<?php } ?>

							<?php if(is_show_language_help('SMS Send Detection Switch')){ ?>
								<b><?php echo language('SMS Send Detection Switch');?>:</b><br/>
								<?php echo language('SMS Send Detection Switch help','After opening, when the number of successive call failures reaches the set value, send short message to check whether the port is available; if the short message is sent successfully, clear the number of successive call failures; if the short message fails, limit the port\'s outgoing.');?>
								
								<br/><br/>
							<?php } ?>

							<?php if(is_show_language_help('SMS Send Detection Count')){ ?>
								<b><?php echo language('SMS Send Detection Count');?>:</b><br/>
								<?php echo language('SMS Send Detection Count help', 'SMS Send Detection Count');?>
								
								<br/><br/>
							<?php } ?>

							<?php if(is_show_language_help('Send Sms Number')){ ?>
								<b><?php echo language('Send Sms Number');?>:</b><br/>
								<?php echo language('Send Sms Number');?>
								
								<br/><br/>
							<?php } ?>

							<?php if(is_show_language_help('Sms Message')){ ?>
								<b><?php echo language('Sms Message');?>:</b><br/>
								<?php echo language('Sms Message help','Sms Message');?>
								
								<br/><br/>
							<?php } ?>

							<?php if(is_show_language_help('Testing SMS report')){ ?>
								<b><?php echo language('Testing SMS report');?>:</b><br/>
								<?php echo language('Testing SMS report help', 'When closed, the successful sending of short message indicates that the port is available; when opened, the successful sending of short message and the receipt of short message report indicate that the port is available.');?>
							<?php } ?>
						</div>
					</div>
				</div>
			</span>
			
			<div class="tab_item">
				<span>
					<?php echo language('Lock Detect Switch');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_detect_flag"></span>
					<input type="checkbox" name="call_detect_flag_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<span><input type="checkbox" name="call_detect_flag" id="call_detect_flag" <?php echo $call_detect_flag; ?> /></span>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Mark Switch');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_fail_mark_flag"></span>
					<input type="checkbox" name="call_fail_mark_flag_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<span><input type="checkbox" name="call_fail_mark_flag" id="call_fail_mark_flag" <?php echo $call_fail_mark_flag;?> /></span>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Call Failed Mark Count');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_fail_mark_count"></span>
					<input type="checkbox" name="call_fail_mark_count_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="text" name="call_fail_mark_count" id="call_fail_mark_count" value="<?php echo $call_fail_mark_count;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Call Failed Lock Switch');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_fail_lock_flag"></span>
					<input type="checkbox" name="call_fail_lock_flag_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<span><input type="checkbox" name="call_fail_lock_flag" id="call_fail_lock_flag" <?php echo $call_fail_lock_flag;?> /></span>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Call Failed Lock Count');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_fail_lock_count"></span>
					<input type="checkbox" name="call_fail_lock_count_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="text" name="call_fail_lock_count" id="call_fail_lock_count" value="<?php echo $call_fail_lock_count;?>">
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('SMS Send Detection Switch');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_fail_lock_sms_flag"></span>
					<input type="checkbox" name="call_fail_lock_sms_flag_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<span><input type="checkbox" name="call_fail_lock_sms_flag" id="call_fail_lock_sms_flag" <?php echo $call_fail_lock_sms_flag;?> /></span>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('SMS Send Detection Count');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_fail_lock_sms_count"></span>
					<input type="checkbox" name="call_fail_lock_sms_count_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="text" name="call_fail_lock_sms_count" id="call_fail_lock_sms_count" value="<?php echo $call_fail_lock_sms_count;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Send Sms Number');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_fail_lock_sms_callee"></span>
					<input type="checkbox" name="call_fail_lock_sms_callee_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="text" name="call_fail_lock_sms_callee" id="call_fail_lock_sms_callee" value="<?php echo $call_fail_lock_sms_callee;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Sms Message');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_fail_lock_sms_msg"></span>
					<input type="checkbox" name="call_fail_lock_sms_msg_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="text" name="call_fail_lock_sms_msg" id="call_fail_lock_sms_msg" value="<?php echo $call_fail_lock_sms_msg;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Testing SMS report');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_fail_lock_sms_report_flag"></span>
					<input type="checkbox" name="call_fail_lock_sms_report_flag_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<span><input type="checkbox" name="call_fail_lock_sms_report_flag" id="call_fail_lock_sms_report_flag" <?php echo $call_fail_lock_sms_report_flag;?> /></span>
				</div>
			</div>
		</div>
		
		<div class="content">
			<span class="title">
				<?php echo language('SMS Limit');?>
				
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<?php if(is_show_language_help('SMS Limit Switch')){ ?>
								<b><?php echo language('SMS Limit Switch');?>:</b><br/>
								<?php echo language('SMS Limit Switch help','SMS Limit Switch');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('SMS Limit Success Flag')){ ?>
								<b><?php echo language('SMS Limit Success Flag');?>:</b><br/>
								<?php echo language('SMS Limit Success Flag help','When close, no matter whether the message is sent successfully or not, the number of messages is counted.');?>

								<br/><br/>
							<?php } ?>

							<?php if(is_show_language_help('SMS Time Interval Switch')){ ?>
								<b><?php echo language('SMS Time Interval Switch');?>:</b><br/>
								<?php echo language('SMS Time Interval Switch help', 'When this switch is enabled, SMS can only be sent within the start-stop time.');?>

								<br/><br/>
							<?php } ?>

							<?php if(is_show_language_help('SMS Time Start')){ ?>
								<b><?php echo language('SMS Time Start');?>:</b><br/>
								<?php echo language('SMS Time Start help', 'Starting time of sending SMS.');?>

								<br/><br/>
							<?php } ?>

							<?php if(is_show_language_help('SMS Time End')){ ?>
								<b><?php echo language('SMS Time End');?>:</b><br/>
								<?php echo language('SMS Time End help', 'End time of sending SMS.');?>

								<br/><br/>
							<?php } ?>

							<?php if(is_show_language_help('Day Limit SMS Count')){ ?>
								<b><?php echo language('Day Limit SMS Count');?>:</b><br/>
								<?php echo language('Day Limit SMS Count help', 'Limit the number of SMS sent daily, the default value is 0, indicating unlimited.');?>

								<br/><br/>
							<?php } ?>

							<?php if(is_show_language_help('Month Limit SMS Count')){ ?>
								<b><?php echo language('Month Limit SMS Count');?>:</b><br/>
								<?php echo language('Month Limit SMS Count help', 'Limit the number of SMS sent per month, the default value is 0, indicating unlimited.');?>

								<br/><br/>
							<?php } ?>

							<?php if(is_show_language_help('SMS Clean Date')){ ?>
								<b><?php echo language('SMS Clean Date');?>:</b><br/>
								<?php echo language('SMS Clean Date help', 'The number of messages sent monthly is automatically cleared at 0 points, 0 minutes and 0 seconds on the set date.');?>
							<?php } ?>
						</div>
					</div>
				</div>
			</span>
			
			<div class="tab_item">
				<span>
					<?php echo language('SMS Limit Switch');?>:
				</span>
				<div class="tab_item_right">
					<span id="csms_limit_switch"></span>
					<input type="checkbox" name="sms_limit_switch_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<span><input type="checkbox" name="sms_limit_switch" id="sms_limit_switch" <?php echo $sms_limit_switch;?> /></span>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('SMS Limit Success Flag');?>:
				</span>
				<div class="tab_item_right">
					<span id="csms_limit_success_flag"></span>
					<input type="checkbox" name="sms_limit_success_flag_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<span><input type="checkbox" name="sms_limit_success_flag" id="sms_limit_success_flag" <?php echo $sms_limit_success_flag;?> /></span>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('SMS Time Interval Switch');?>:
				</span>
				<div class="tab_item_right">
					<input type="checkbox" name="time_sms_swtich_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<span><input type="checkbox" name="time_sms_swtich" id="time_sms_swtich" <?php echo $time_sms_swtich;?> /></span>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('SMS Time Start');?>:
				</span>
				<div class="tab_item_right">
					<input type="checkbox" name="sms_start_time_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<select  name="sms_start_hour" id="sms_start_hour" style="width:50px;">
					<?php
						for($j=0; $j<=23; $j++) {
							($sms_start_hour == $j)?$hour_sel='selected':$hour_sel='';
							echo "<option  value='$j' $hour_sel>$j</option>";
						}
					?>
					</select>
					<?php echo language('Hour');?>
					<select name="sms_start_minute" id="sms_start_minute" style="width:50px;">
					<?php
						for($j=0; $j<=59; $j++) {
							($sms_start_minute == $j)?$minute_sel='selected':$minute_sel='';
							echo "<option  value='$j' $minute_sel>$j</option>";
						}
					?>
					</select>
					<?php echo language('Minute');?>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('SMS Time End');?>:
				</span>
				<div class="tab_item_right">
					<input type="checkbox" name="sms_end_time_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<select  name="sms_end_hour" id="sms_end_hour" style="width:50px;">
					<?php
						for($j=0; $j<=23; $j++) {
							($sms_end_hour == $j)?$hour_sel='selected':$hour_sel='';
							echo "<option  value='$j' $hour_sel>$j</option>";
						}
					?>
					</select>
					<?php echo language('Hour');?>
					<select name="sms_end_minute" id="sms_end_minute" style="width:50px;">
					<?php
						for($j=0; $j<=59; $j++) {
							($sms_end_minute == $j)?$minute_sel='selected':$minute_sel='';
							echo "<option  value='$j' $minute_sel>$j</option>";
						}
					?>
					</select>
					<?php echo language('Minute');?>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Day Limit SMS Count');?>:
				</span>
				<div class="tab_item_right">
					<span id="cday_sms_settings"></span>
					<input type="checkbox" name="day_sms_settings_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="day_sms_settings" id="day_sms_settings" value="<?php echo $day_sms_settings;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Month Limit SMS Count');?>:
				</span>
				<div class="tab_item_right">
					<span id="cmon_sms_settings"></span>
					<input type="checkbox" name="mon_sms_settings_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="mon_sms_settings" id="mon_sms_settings" value="<?php echo $mon_sms_settings;?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('SMS Clean Date');?>:
				</span>
				<div class="tab_item_right">
					<span id="csms_clean_date"></span>
					<input type="checkbox" name="sms_clean_date_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<select name="sms_clean_date" id="sms_clean_date">
					<?php 
					for($i=1;$i<=31;$i++){
						$selected = '';
						if($sms_clean_date == $i){
							$selected = 'selected';
						}
					?>
						<option value="<?php echo $i;?>" <?php echo $selected;?>><?php echo $i;?></option>
					<?php
					}
					?>
					</select>
				</div>
			</div>
		</div>
		
		<div class="content">
			<span class="title">
				<?php echo language('Save To Other Ports');?>
				
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<?php if(is_show_language_help('Save To Other Ports')){ ?>
								<b><?php echo language('Save To Other Ports');?>:</b><br/>
								<?php echo language('Save To Other Ports help','Save To Other Ports');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Sync All Settings')){ ?>
								<b><?php echo language('Sync All Settings');?>:</b><br/>
								<?php echo language('Sync All Settings help','Sync All Settings');?>
							<?php } ?>
						</div>
					</div>
				</div>
			</span>
			
			<div class="tab_item">
				<span>
					<?php echo language('Save To Other Ports');?>:
				</span>
				<div class="tab_item_right" style="float:left;">
					<table cellpadding="0" cellspacing="0" class="port_table">
					<?php
					for($i=1;$i<=$__GSM_SUM__;$i++){
						$port_name = get_gsm_name_by_channel($i);
						$channel_type = get_gsm_type_by_channel($i,1);
						if($channel_type == 'NULL') continue;
						$checked = '';
						$disabled = '';
						if($i == $channel) {
							$checked = 'checked';
							$disabled = 'disabled';
						}
						
						echo "<td class='module_port'><input type='checkbox' name='spans[1][$i]' class='port' $checked $disabled>";
						echo $port_name;
						echo '</td>';
					}
					?>
					<tr>
						<td colspan=4>
							<input type="checkbox" id="all_port" onclick="selectAllCheckbox(this.checked,'class','port');handle_port_sync();">
							<?php echo language('All');?>
							<span id="cports"></span>
						</td>
					</tr>
					</table>
				</div>
				<div class="clear"></div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Sync All Settings');?>:
				</span>
				<div class="tab_item_right">
					<span style="font-weight:normal;">
						<input type="checkbox" id="all_settings_sync" onclick="selectAllCheckbox(this.checked,'class','setting_sync');" checked disabled />
						<?php echo language('Select all settings');?>
					</span>
				</div>
			</div>
		</div>
		
	
		<input type="hidden" name="send" id="send" value="" />
	
		<div id="button_save">
		
			<?php if(!$only_view){ ?>
			<button type="submit" onclick="document.getElementById('send').value='Save';return check();"><?php echo language('Save');?></button>
			<?php } ?>
			
			<button type="button" onclick="window.location.href='<?php echo get_self();?>'" ><?php echo language('Cancel');?></button>
		</div>
	</form>
	
	<!--
	<div id="sort_out" class="sort_out"></div>
	<div class="sort_gsm_cli" id="sort_gsm_cli">
		<div id="sort_info" class="sort_info" style="display:block">
		<?php
			$alldata = get_all_gsm_info();
			foreach($alldata as $bnum => $chs ) {
				foreach($chs as $cnum => $ch) {
					if(strstr(get_gsm_name_by_channel($cnum,$bnum),'null'))continue;
					if($cnum == $channel){
		?>
				<li><a style="color:#CD3278;" href="/cgi-bin/php/gsm-calllimit.php?sel_gsm=<?php echo $cnum; ?>&send=Modify" ><?php echo get_gsm_name_by_channel($cnum,$bnum);?></a></li>
		<?php
					}else{
		?>
				<li><a style="color:LemonChiffon4;" href="/cgi-bin/php/gsm-calllimit.php?sel_gsm=<?php echo $cnum; ?>&send=Modify" ><?php echo get_gsm_name_by_channel($cnum,$bnum);?></a></li>
		<?php
					}
				}
			}
		?>
		</div>
	</div>
	-->
	
	<script>
	$(function(){
		$(".port").click(function(){
			handle_port_sync();
		});
		
		$("#sort_out").mouseover(function(){
			if($("#sort_out").offset().left <= 5){
				$("#sort_gsm_cli").animate({left:"0px"}, 300);
				$("#sort_out").animate({left:"198px"}, 300);
			}
		});
		$("#sort_gsm_cli").mouseleave(function(){
			$("#sort_gsm_cli").stop().animate({left:"-198px"}, 300);
			$("#sort_out").stop().animate({left:"0px"}, 300);
		});
		
		//show and hide
		var call_limit_switch = $("#call_limit_switch").attr('checked');
		if(call_limit_switch == 'checked'){
			$(".call_limit_tr").show();
		}else{
			$(".call_limit_tr").hide();
		}
		$("#call_limit_switch").change(function(){
			if($(this).attr('checked') == 'checked'){
				$(".call_limit_tr").show();
			}else{
				$(".call_limit_tr").hide();
			}
		});
		
		var call_detect_flag = $("#call_detect_flag").attr('checked');
		if(call_detect_flag == 'checked'){
			$(".lock_sim_tr").show();
		}else{
			$(".lock_sim_tr").hide();
		}
		$("#call_detect_flag").change(function(){
			if($(this).attr('checked') == 'checked'){
				$(".lock_sim_tr").show();
			}else{
				$(".lock_sim_tr").hide();
			}
		});
		
		var call_fail_mark_flag = $("#call_fail_mark_flag").attr('checked');
		if(call_fail_mark_flag == 'checked'){
			$(".mark_count_tr").show();
		}else{
			$(".mark_count_tr").hide();
		}
		$("#call_fail_mark_flag").change(function(){
			if($(this).attr('checked') == 'checked'){
				$(".mark_count_tr").show();
			}else{
				$(".mark_count_tr").hide();
			}
		});
		
		var call_fail_lock_flag = $("#call_fail_lock_flag").attr('checked');
		if(call_fail_lock_flag == 'checked'){
			$(".lock_count_tr").show();
		}else{
			$(".lock_count_tr").hide();
		}
		$("#call_fail_lock_flag").change(function(){
			if($(this).attr('checked') == 'checked'){
				$(".lock_count_tr").show();
			}else{
				$(".lock_count_tr").hide();
			}
		});
		
		var call_fail_lock_sms_flag = $("#call_fail_lock_sms_flag").attr('checked');
		if(call_fail_lock_sms_flag == 'checked'){
			$(".sms_switch_tr").show();
		}else{
			$(".sms_switch_tr").hide();
		}
		$("#call_fail_lock_sms_flag").change(function(){
			if($(this).attr('checked') == 'checked'){
				$(".sms_switch_tr").show();
			}else{
				$(".sms_switch_tr").hide();
			}
		});
		
		//sms_time_switch
		var sms_time_switch = document.getElementById('time_sms_swtich').checked;
		if(sms_time_switch){
			$(".sms_time_tr").show();
		}else{
			$(".sms_time_tr").hide();
		}
		$("#time_sms_swtich").change(function(){
			if($(this).attr('checked') == 'checked'){
				$(".sms_time_tr").show();
			}else{
				$(".sms_time_tr").hide();
			}
		});
		
		var sms_limit_switch = $("#sms_limit_switch").attr('checked');
		if(sms_limit_switch == 'checked'){
			$(".sms_limit_tr").show();
		}else{
			$(".sms_limit_tr").hide();
		}
		$("#sms_limit_switch").change(function(){
			if($(this).attr('checked') == 'checked'){
				$(".sms_limit_tr").show();
			}else{
				$(".sms_limit_tr").hide();
			}
		});
		
		//auto reset data
		$("#call_time_clean_date" ).datetimepicker({
			dateFormat: "dd/mm/yy", 
			timeFormat: "HH:mm:ss", 
			beforeShow: function(){
				setTimeout(function(){
					$('#ui-datepicker-div').css("z-index",5);
				},0);
			}
		});
		$("#call_time_clean_date" ).datetimepicker();
		
		//call_time_single_switch
		var call_time_single_switch = document.getElementById('call_time_single_switch').checked;
		if(call_time_single_switch){
			$("#call_time_settings_show").show();
		}else{
			$("#call_time_settings_show").hide();
		}
		$("#call_time_single_switch").change(function(){
			if($(this).attr('checked') == 'checked'){
				$("#call_time_settings_show").show();
			}else{
				$("#call_time_settings_show").hide();
			}
		});
		
		//call_time_switch ******
		//call_time_total_switch  ******
		//call_time_clean_switch  ******
		var call_time_switch = document.getElementById('call_time_switch').checked;
		var call_time_total_switch = document.getElementById('call_time_total_switch').checked;
		var call_time_clean_switch = document.getElementById('call_time_clean_switch').checked;
		if(call_time_switch){
			$("#call_time_switch_show").show();
			if(call_time_total_switch){
				$("#call_time_total_switch_show").show();
				if(call_time_clean_switch){
					$("#call_time_clean_switch_show").show();
				}else{
					$("#call_time_clean_switch_show").hide();
				}
			}else{
				$("#call_time_total_switch_show").hide();
				$("#call_time_clean_switch_show").hide();
			}
		}else{
			$("#call_time_switch_show").hide();
			$("#call_time_total_switch_show").hide();
			$("#call_time_clean_switch_show").hide();
		}
		
		$("#call_time_switch").change(function(){
			if($(this).attr('checked') == 'checked'){
				$("#call_time_switch_show").show();
				if($("#call_time_total_switch").attr('checked') == 'checked'){
					$("#call_time_total_switch_show").show();
					if($("#call_time_clean_switch").attr('checked') == 'checked'){
						$("#call_time_clean_switch_show").show();
					}else{
						$("#call_time_clean_switch_show").hide();
					}
				}else{
					$("#call_time_total_switch_show").hide();
					$("#call_time_clean_switch_show").hide();
				}
			}else{
				$("#call_time_switch_show").hide();
				$("#call_time_total_switch_show").hide();
				$("#call_time_clean_switch_show").hide();
			}
		});
		
		$("#call_time_total_switch").change(function(){
			if($(this).attr('checked') == 'checked'){
				$("#call_time_total_switch_show").show();
				if($("#call_time_clean_switch").attr('checked') == 'checked'){
					$("#call_time_clean_switch_show").show();
				}else{
					$("#call_time_clean_switch_show").hide();
				}
			}else{
				$("#call_time_total_switch_show").hide();
				$("#call_time_clean_switch_show").hide();
			}
		});
		
		$("#call_time_clean_switch").change(function(){
			if($(this).attr('checked') == 'checked'){
				$("#call_time_clean_switch_show").show();
			}else{
				$("#call_time_clean_switch_show").hide();
			}
		});
	});
	
	function check(){
		var is_check = false;
		
		//call times limit
		if(document.getElementById('call_limit_switch').checked){
			var day_calls_settings = document.getElementById('day_calls_settings').value;
			document.getElementById('cday_calls_settings').innerHTML = '';
			if(isNaN(day_calls_settings) || parseInt(day_calls_settings) < 0){
				document.getElementById('day_calls_settings').focus();
				document.getElementById('cday_calls_settings').innerHTML = con_str("Must be Number.Range:>=0.");
				
				is_check = true;
			}
			
			var day_answer_setting = document.getElementById('day_answer_setting').value;
			document.getElementById('cday_answer_setting').innerHTML = '';
			if(isNaN(day_answer_setting) || parseInt(day_answer_setting) < 0){
				document.getElementById('day_answer_setting').focus();
				document.getElementById('cday_answer_setting').innerHTML = con_str("Must be Number.Range:>=0");
				
				is_check = true;
			}
			
			var hour_calls_settings = document.getElementById('hour_calls_settings').value;
			document.getElementById('chour_calls_settings').innerHTML = '';
			if(isNaN(hour_calls_settings) || parseInt(hour_calls_settings) < 0){
				document.getElementById('hour_calls_settings').focus();
				document.getElementById('chour_calls_settings').innerHTML = con_str("Must be Number.Range:>=0");
				
				is_check = true;
			}
		}
		
		//call duration limit
		if(document.getElementById('call_time_switch').checked){
			/*
			if(document.getElementById('call_time_single_switch').checked){
				var call_time_settings = document.getElementById('call_time_settings').value;
				document.getElementById('ccall_time_settings').innerHTML = '';
				if(isNaN(call_time_settings) || parseInt(call_time_settings) < 1){
					document.getElementById('call_time_settings').focus();
					document.getElementById('ccall_time_settings').innerHTML = con_str("Must be Number.Range:>=0");
					
					is_check = true;
				}
			}
			
			if(document.getElementById('call_time_total_switch').checked){
				var call_time_total = document.getElementById('call_time_total').value;
				document.getElementById('ccall_time_total').innerHTML = '';
				if(isNaN(call_time_total) || parseInt(call_time_total) < 1){
					document.getElementById('call_time_total').focus();
					document.getElementById('ccall_time_total').innerHTML = con_str("Must be Number.Range:>=0");
					
					is_check = true;
				}
			}*/
			
			if(document.getElementById('call_time_single_switch').checked && document.getElementById('call_time_total_switch').checked){
				var call_time_settings = document.getElementById('call_time_settings').value;
				var call_time_total = document.getElementById('call_time_total').value;
				document.getElementById('ccall_time_settings').innerHTML = '';
				if(parseInt(call_time_settings) > parseInt(call_time_total)){
					document.getElementById('call_time_settings').focus();
					document.getElementById('ccall_time_settings').innerHTML = con_str("<?php echo language('Single Limit Time Less Tip', 'Single Call Duration Limitation must be less than or equal to Call Duration Limitation.');?>");
					
					is_check = true;
				}
			}
		}
		
		//lock 
		if(document.getElementById('call_detect_flag').checked){
			if(document.getElementById('call_fail_mark_flag').checked){
				var call_fail_mark_count = document.getElementById('call_fail_mark_count').value;
				document.getElementById('ccall_fail_mark_count').innerHTML = '';
				if(isNaN(call_fail_mark_count) || parseInt(call_fail_mark_count) < 0){
					document.getElementById('call_fail_mark_count').focus();
					document.getElementById('ccall_fail_mark_count').innerHTML = con_str("Must be Number.Range:>=0");
					
					is_check = true;
				}
			}
			
			if(document.getElementById('call_fail_lock_flag').checked){
				var call_fail_lock_count = document.getElementById('call_fail_lock_count').value;
				document.getElementById('ccall_fail_lock_count').innerHTML = '';
				if(isNaN(call_fail_lock_count) || parseInt(call_fail_lock_count) < 0){
					document.getElementById('call_fail_lock_count').focus();
					document.getElementById('ccall_fail_lock_count').innerHTML = con_str("Must be Number.Range:>=0");
					
					is_check = true;
				}
			}
			
			if(document.getElementById('call_fail_lock_sms_flag').checked){
				var call_fail_lock_sms_count = document.getElementById('call_fail_lock_sms_count').value;
				document.getElementById('ccall_fail_lock_sms_count').innerHTML = '';
				if(isNaN(call_fail_lock_sms_count) || parseInt(call_fail_lock_sms_count) < 0){
					document.getElementById('call_fail_lock_sms_count').focus();
					document.getElementById('ccall_fail_lock_sms_count').innerHTML = con_str("Must be Number.Range:>=0");
					
					is_check = true;
				}
			}
			
			if((document.getElementById('call_fail_mark_flag').checked) && (document.getElementById('call_fail_lock_flag').checked)){
				document.getElementById('ccall_fail_mark_count').innerHTML = '';
				if(parseInt(call_fail_mark_count) >= parseInt(call_fail_lock_count)){
					document.getElementById('call_fail_mark_count').focus();
					document.getElementById('ccall_fail_mark_count').innerHTML = con_str('<?php echo language("Mark Count and Lock Count","Call Failed Mark Count must be less than Call Failed Lock Count.");?>');
					
					is_check = true;
				}
			}
		}
		
		//sms limit
		if(document.getElementById('sms_limit_switch').checked){
			var day_sms_settings = document.getElementById('day_sms_settings').value;
			document.getElementById('cday_sms_settings').innerHTML = '';
			if(isNaN(day_sms_settings) || parseInt(day_sms_settings) < 0 || parseInt(day_sms_settings) > 65535){
				document.getElementById('day_sms_settings').focus();
				document.getElementById('cday_sms_settings').innerHTML = con_str("Must be Number.Range:0-65535");
				
				is_check = true;
			}
			
			var mon_sms_settings = document.getElementById('mon_sms_settings').value;
			document.getElementById('cmon_sms_settings').innerHTML = '';
			if(isNaN(mon_sms_settings) || parseInt(mon_sms_settings) < 0 || parseInt(mon_sms_settings) > 65535){
				document.getElementById('mon_sms_settings').focus();
				document.getElementById('cmon_sms_settings').innerHTML = con_str("Must be Number.Range:0-65535");
				
				is_check = true;
			}
			
			if(document.getElementById('mon_sms_settings').value != 0){
				var sms_clean_date = document.getElementById('sms_clean_date').value;
				document.getElementById('csms_clean_date').innerHTML = '';
				if(isNaN(sms_clean_date) || parseInt(sms_clean_date) < 1 || parseInt(sms_clean_date) > 31){
					document.getElementById('sms_clean_date').focus();
					document.getElementById('csms_clean_date').innerHTML = con_str("Must be Number.Range:1-31");
					
					is_check = true;
				}
			}else if(document.getElementById('mon_sms_settings').value == 0){
				var sms_clean_date = document.getElementById('sms_clean_date').value;
				document.getElementById('csms_clean_date').innerHTML = '';
				if(isNaN(sms_clean_date) || parseInt(sms_clean_date) < 0 || parseInt(sms_clean_date) > 31){
					document.getElementById('sms_clean_date').focus();
					document.getElementById('csms_clean_date').innerHTML = con_str("Must be Number.Range:0-31");
					
					is_check = true;
				}
			}
		}
		
		if(is_check){
			return false;
		}
		
		return true;
	}
	
	function handle_port_sync(){
		if(isAllCheckboxChecked('class','port')){
			$("#all_port").attr({"checked":true});
		}else{
			$("#all_port").attr({"checked":false});
		}
		if(isCheckboxChecked('class','port')){
			$(".setting_sync").show();
			$("#all_settings_sync").attr({"disabled":false,"checked":true});
			selectAllCheckbox(true,'class','setting_sync');
		}else{
			$(".setting_sync").hide();
			$("#all_settings_sync").attr({"disabled":true,"checked":true});
			selectAllCheckbox(false,'class','setting_sync');
		}
	}
	
	function remain_time_reset(){
		if(confirm("<?php echo language('Dial Limit Reset confirm','This operation will unlimite the call duration limit. Are you sure you want to do this?');?>")){
			document.getElementById('send').value='Reset';
			document.getElementById('manform').submit();
		}
	}
	
	function check_step(id){
		var obj = document.getElementById(id);
		obj.value = obj.value.replace(/[^\d]*/g,'');

		var val = obj.value;
		
		if(!check_int32(val) || val==0 || val=='') {
			$("#c"+id).html(con_str("<?php echo language('js check integer','Please input integer');echo '(1~999)';?>"));
			return false;
		} else {
			$("#c"+id).html("");
			return true;
		}
	}
	
	function check_dl_integer(id){
		var obj = document.getElementById(id);
		obj.value = obj.value.replace(/[^\d]*/g,'');

		val = obj.value;
		if(!check_number(val) || val==0 || val=='') {
			$("#c"+id).html(con_str("<?php echo language('js check integer','Please input integer!');?>"));
			return false;
		} else {
			$("#c"+id).html("");
			return true;
		}
	}
	
	function check_freetime(id){
		var obj = document.getElementById(id);
		obj.value = obj.value.replace(/[^\d]*/g,'');

		var val_str = obj.value;
		var val = parseInt(val_str);
		var step_str = document.getElementById('call_time_step').value;
		var step = parseInt(step_str);
		
		if(!check_integer(val_str)) {
			$("#c"+id).html(con_str("<?php echo language('js check integer','Please input integer');?>"));
			return false;
		} else if(val >= step) {
			$("#c"+id).html(con_str("<?php echo language('js check dl_step integer','The input should be less than duration step');?>"));
			return false;
		} else {
			$("#c"+id).html("");
			return true;
		}
	}
	
	function check_warning_time(id){
		var obj = document.getElementById(id);
		obj.value = obj.value.replace(/[^\d]*/g,'');

		var val_str = obj.value;
		var val = parseInt(val_str);
		var limit_str = document.getElementById('call_time_total').value;
		var limit = parseInt(limit_str);
		
		if(!check_integer(val_str)) {
			$("#c"+id).html(con_str("<?php echo language('js check integer','Please input integer');?>"));
			return false;
		} else if(val >= limit) {
			$("#c"+id).html(con_str("<?php echo language('js check warning time integer','The input should be less than call duration limitation.');?>"));
			return false;
		} else {
			$("#c"+id).html("");
			return true;
		}
	}
	
	function check_warning_describe(id){
		var obj = document.getElementById(id);
		obj.value = obj.value.replace(/[=;"]*/g,'');

		return true;
	}
	
	function check_dl_datetime(id){
		var obj = document.getElementById(id);
		obj.value = obj.value.replace(/[^-:/ \d]*/g,'');

		val = obj.value;
		
		if(!check_datetime(val)) {
			$("#c"+id).html(con_str("<?php echo language('js check datetime','Please input right format date and time. eg: 01/01/2019 19:44:31');?>"));
			return false;
		} else {
			$("#c"+id).html("");
		}
		
		var currentTime = "<?php echo $cur_time;?>";
		currentTime = new Date(Date.parse(currentTime));
		
		var call_time_clean_date = document.getElementById('call_time_clean_date').value;
		var day = call_time_clean_date.split('/')[0];
		var temp = call_time_clean_date.split(' ')[0];
		var year = temp.split('/')[2];
		call_time_clean_date = call_time_clean_date.replace(year,day);
		call_time_clean_date = call_time_clean_date.replace(day,year);
		
		call_time_clean_date = new Date(call_time_clean_date);
		if(call_time_clean_date < currentTime){
			$("#c"+id).html(con_str("<?php echo language('js check datetime2','The date must be greater than the current.');?>"));
			return false;
		}
		
		return true;
	}
	</script>

<?php 
}

function save_gsm_calllimit(){
	global $__GSM_SUM__;
	$channel = $_POST['channel'];
	
	$aql = new aql();
	
	$conf_path = '/etc/asterisk/gw/call_limit/calllimit_settings.conf';
	$hlock = lock_file($conf_path);
	if(!file_exists($conf_path)) {exec('touch /etc/asterisk/gw/call_limit/calllimit_settings.conf');}
	$aql->set('basedir','/etc/asterisk/gw/call_limit/');
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	
	$res = $aql->query("select * from calllimit_settings.conf");
	
	if(!isset($res[$channel])){
		$aql->assign_addsection($channel,'');
	}
	
	//call limit
	if(isset($_POST['call_limit_switch'])){
		$call_limit_switch = 1;
	}else{
		$call_limit_switch = 0;
	}
	if(isset($res[$channel]['call_limit_switch'])){
		$aql->assign_editkey($channel,'call_limit_switch',$call_limit_switch);
	}else{
		$aql->assign_append($channel,'call_limit_switch',$call_limit_switch);
	}
	
	$day_calls_settings = $_POST['day_calls_settings'];
	if(isset($res[$channel]['day_calls_settings'])){
		$aql->assign_editkey($channel,'day_calls_settings',$day_calls_settings);
	}else{
		$aql->assign_append($channel,'day_calls_settings',$day_calls_settings);
	}
	
	$day_answer_setting = $_POST['day_answer_setting'];
	if(isset($res[$channel]['day_answer_setting'])){
		$aql->assign_editkey($channel,'day_answer_setting',$day_answer_setting);
	}else{
		$aql->assign_append($channel,'day_answer_setting',$day_answer_setting);
	}
	
	$hour_calls_settings = $_POST['hour_calls_settings'];
	if(isset($res[$channel]['hour_calls_settings'])){
		$aql->assign_editkey($channel,'hour_calls_settings',$hour_calls_settings);
	}else{
		$aql->assign_append($channel,'hour_calls_settings',$hour_calls_settings);
	}
	
	
	//Call Limit Time
	if(isset($_POST['call_time_switch'])){
		$call_time_switch = 1;
	}else{
		$call_time_switch = 0;
	}
	
	$call_time_step = $_POST['call_time_step'];
	if(isset($res[$channel]['call_time_step'])){
		$aql->assign_editkey($channel,'call_time_step',$call_time_step);
	}else{
		$aql->assign_append($channel,'call_time_step',$call_time_step);
	}
	
	if(isset($_POST['call_time_single_switch'])){
		$call_time_single_switch = 1;
	}else{
		$call_time_single_switch = 0;
	}
	if(isset($res[$channel]['call_time_single_switch'])){
		$aql->assign_editkey($channel,'call_time_single_switch',$call_time_single_switch);
	}else{
		$aql->assign_append($channel,'call_time_single_switch',$call_time_single_switch);
	}
	
	$call_time_settings = $_POST['call_time_settings'];
	if(isset($res[$channel]['call_time_settings'])){
		$aql->assign_editkey($channel,'call_time_settings',$call_time_settings);
	}else{
		$aql->assign_append($channel,'call_time_settings',$call_time_settings);
	}
	
	if(isset($_POST['call_time_total_switch'])){
		$call_time_total_switch = 1;
	}else{
		$call_time_total_switch = 0;
	}
	if(isset($res[$channel]['call_time_total_switch'])){
		$aql->assign_editkey($channel,'call_time_total_switch',$call_time_total_switch);
	}else{
		$aql->assign_append($channel,'call_time_total_switch',$call_time_total_switch);
	}
	
	$call_time_total = $_POST['call_time_total'];
	if(isset($res[$channel]['call_time_total'])){
		$aql->assign_editkey($channel,'call_time_total',$call_time_total);
	}else{
		$aql->assign_append($channel,'call_time_total',$call_time_total);
	}
	
	if($call_time_settings == 0 && $call_time_total == 0){
		$call_time_switch = 0;
	}
	if(isset($res[$channel]['call_time_switch'])){
		$aql->assign_editkey($channel,'call_time_switch',$call_time_switch);
	}else{
		$aql->assign_append($channel,'call_time_switch',$call_time_switch);
	}
	
	$call_time_free = $_POST['call_time_free'];
	if(isset($res[$channel]['call_time_free'])){
		$aql->assign_editkey($channel,'call_time_free',$call_time_free);
	}else{
		$aql->assign_append($channel,'call_time_free',$call_time_free);
	}
	
	$call_time_warning_num = $_POST['call_time_warning_num'];
	if(isset($res[$channel]['call_time_warning_num'])){
		$aql->assign_editkey($channel,'call_time_warning_num',$call_time_warning_num);
	}else{
		$aql->assign_append($channel,'call_time_warning_num',$call_time_warning_num);
	}
	
	$call_time_warning_callee = $_POST['call_time_warning_callee'];
	if(isset($res[$channel]['call_time_warning_callee'])){
		$aql->assign_editkey($channel,'call_time_warning_callee',$call_time_warning_callee);
	}else{
		$aql->assign_append($channel,'call_time_warning_callee',$call_time_warning_callee);
	}
	
	$call_time_warning_msg = $_POST['call_time_warning_msg'];
	if(isset($res[$channel]['call_time_warning_msg'])){
		$aql->assign_editkey($channel,'call_time_warning_msg',$call_time_warning_msg);
	}else{
		$aql->assign_append($channel,'call_time_warning_msg',$call_time_warning_msg);
	}
	
	if(isset($_POST['call_time_clean_switch'])){
		$call_time_clean_switch = 1;
	}else{
		$call_time_clean_switch = 0;
	}
	if(isset($res[$channel]['call_time_clean_switch'])){
		$aql->assign_editkey($channel,'call_time_clean_switch',$call_time_clean_switch);
	}else{
		$aql->assign_append($channel,'call_time_clean_switch',$call_time_clean_switch);
	}
	
	$call_time_clean_type = $_POST['call_time_clean_type'];
	if(isset($res[$channel]['call_time_clean_type'])){
		$aql->assign_editkey($channel,'call_time_clean_type',$call_time_clean_type);
	}else{
		$aql->assign_append($channel,'call_time_clean_type',$call_time_clean_type);
	}
	
	$call_time_clean_date = $_POST['call_time_clean_date'];
	if(isset($res[$channel]['call_time_clean_date'])){
		$aql->assign_editkey($channel,'call_time_clean_date',$call_time_clean_date);
	}else{
		$aql->assign_append($channel,'call_time_clean_date',$call_time_clean_date);
	}
	
	//Lock Card
	if(isset($_POST['call_detect_flag'])){
		$call_detect_flag = 1;
	}else{
		$call_detect_flag = 0;
	}
	if(isset($res[$channel]['call_detect_flag'])){
		$aql->assign_editkey($channel,'call_detect_flag',$call_detect_flag);
	}else{
		$aql->assign_append($channel,'call_detect_flag',$call_detect_flag);
	}
	
	if(isset($_POST['call_fail_mark_flag'])){
		$call_fail_mark_flag = 1;
	}else{
		$call_fail_mark_flag = 0;
	}
	if(isset($res[$channel]['call_fail_mark_flag'])){
		$aql->assign_editkey($channel,'call_fail_mark_flag',$call_fail_mark_flag);
	}else{
		$aql->assign_append($channel,'call_fail_mark_flag',$call_fail_mark_flag);
	}
	
	$call_fail_mark_count = $_POST['call_fail_mark_count'];
	if(isset($res[$channel]['call_fail_mark_count'])){
		$aql->assign_editkey($channel,'call_fail_mark_count',$call_fail_mark_count);
	}else{
		$aql->assign_append($channel,'call_fail_mark_count',$call_fail_mark_count);
	}
	
	if(isset($_POST['call_fail_lock_flag'])){
		$call_fail_lock_flag = 1;
	}else{
		$call_fail_lock_flag = 0;
	}
	if(isset($res[$channel]['call_fail_lock_flag'])){
		$aql->assign_editkey($channel,'call_fail_lock_flag',$call_fail_lock_flag);
	}else{
		$aql->assign_append($channel,'call_fail_lock_flag',$call_fail_lock_flag);
	}
	
	$call_fail_lock_count = $_POST['call_fail_lock_count'];
	if(isset($res[$channel]['call_fail_lock_count'])){
		$aql->assign_editkey($channel,'call_fail_lock_count',$call_fail_lock_count);
	}else{
		$aql->assign_append($channel,'call_fail_lock_count',$call_fail_lock_count);
	}
	
	if(isset($_POST['call_fail_lock_sms_flag'])){
		$call_fail_lock_sms_flag = 1;
	}else{
		$call_fail_lock_sms_flag = 0;
	}
	if(isset($res[$channel]['call_fail_lock_sms_flag'])){
		$aql->assign_editkey($channel,'call_fail_lock_sms_flag',$call_fail_lock_sms_flag);
	}else{
		$aql->assign_append($channel,'call_fail_lock_sms_flag',$call_fail_lock_sms_flag);
	}
	
	$call_fail_lock_sms_count = $_POST['call_fail_lock_sms_count'];
	if(isset($res[$channel]['call_fail_lock_sms_count'])){
		$aql->assign_editkey($channel,'call_fail_lock_sms_count',$call_fail_lock_sms_count);
	}else{
		$aql->assign_append($channel,'call_fail_lock_sms_count',$call_fail_lock_sms_count);
	}
	
	$call_fail_lock_sms_callee = $_POST['call_fail_lock_sms_callee'];
	if(isset($res[$channel]['call_fail_lock_sms_callee'])){
		$aql->assign_editkey($channel,'call_fail_lock_sms_callee',$call_fail_lock_sms_callee);
	}else{
		$aql->assign_append($channel,'call_fail_lock_sms_callee',$call_fail_lock_sms_callee);
	}
	
	$call_fail_lock_sms_msg = $_POST['call_fail_lock_sms_msg'];
	if(isset($res[$channel]['call_fail_lock_sms_msg'])){
		$aql->assign_editkey($channel,'call_fail_lock_sms_msg',$call_fail_lock_sms_msg);
	}else{
		$aql->assign_append($channel,'call_fail_lock_sms_msg',$call_fail_lock_sms_msg);
	}
	
	if(isset($_POST['call_fail_lock_sms_report_flag'])){
		$call_fail_lock_sms_report_flag = 1;
	}else{
		$call_fail_lock_sms_report_flag = 0;
	}
	if(isset($res[$channel]['call_fail_lock_sms_report_flag'])){
		$aql->assign_editkey($channel,'call_fail_lock_sms_report_flag',$call_fail_lock_sms_report_flag);
	}else{
		$aql->assign_append($channel,'call_fail_lock_sms_report_flag',$call_fail_lock_sms_report_flag);
	}
	
	//sms limit
	if(isset($_POST['sms_limit_switch'])){
		$sms_limit_switch = 1;
	}else{
		$sms_limit_switch = 0;
	}
	if(isset($res[$channel]['sms_limit_switch'])){
		$aql->assign_editkey($channel,'sms_limit_switch',$sms_limit_switch);
	}else{
		$aql->assign_append($channel,'sms_limit_switch',$sms_limit_switch);
	}
	
	if(isset($_POST['sms_limit_success_flag'])){
		$sms_limit_success_flag = 1;
	}else{
		$sms_limit_success_flag = 0;
	}
	if(isset($res[$channel]['sms_limit_success_flag'])){
		$aql->assign_editkey($channel,'sms_limit_success_flag',$sms_limit_success_flag);
	}else{
		$aql->assign_append($channel,'sms_limit_success_flag',$sms_limit_success_flag);
	}
	
	if(isset($_POST['time_sms_swtich'])){
		$time_sms_swtich = 1;
	}else{
		$time_sms_swtich = 0;
	}
	if(isset($res[$channel]['time_sms_swtich'])){
		$aql->assign_editkey($channel,'time_sms_swtich',$time_sms_swtich);
	}else{
		$aql->assign_append($channel,'time_sms_swtich',$time_sms_swtich);
	}
	
	$sms_start_hour = $_POST['sms_start_hour'];
	$sms_start_hour = sprintf("%02d",$sms_start_hour);
	$sms_start_minute = $_POST['sms_start_minute'];
	$sms_start_minute = sprintf("%02d",$sms_start_minute);
	$sms_start_time = $sms_start_hour.':'.$sms_start_minute.':00';
	if(isset($res[$channel]['time_sms_start'])){
		$aql->assign_editkey($channel,'time_sms_start',$sms_start_time);
	}else{
		$aql->assign_append($channel,'time_sms_start',$sms_start_time);
	}
	
	$sms_end_hour = $_POST['sms_end_hour'];
	$sms_end_hour = sprintf("%02d",$sms_end_hour);
	$sms_end_minute = $_POST['sms_end_minute'];
	$sms_end_minute = sprintf("%02d",$sms_end_minute);
	$sms_end_time = $sms_end_hour.':'.$sms_end_minute.':00';
	if(isset($res[$channel]['time_sms_end'])){
		$aql->assign_editkey($channel,'time_sms_end',$sms_end_time);
	}else{
		$aql->assign_append($channel,'time_sms_end',$sms_end_time);
	}
	
	$day_sms_settings = $_POST['day_sms_settings'];
	if(isset($res[$channel]['day_sms_settings'])){
		$aql->assign_editkey($channel,'day_sms_settings',$day_sms_settings);
	}else{
		$aql->assign_append($channel,'day_sms_settings',$day_sms_settings);
	}
	
	$mon_sms_settings = $_POST['mon_sms_settings'];
	if(isset($res[$channel]['mon_sms_settings'])){
		$aql->assign_editkey($channel,'mon_sms_settings',$mon_sms_settings);
	}else{
		$aql->assign_append($channel,'mon_sms_settings',$mon_sms_settings);
	}
	
	$sms_clean_date = $_POST['sms_clean_date'];
	if(isset($res[$channel]['sms_clean_date'])){
		$aql->assign_editkey($channel,'sms_clean_date',$sms_clean_date);
	}else{
		$aql->assign_append($channel,'sms_clean_date',$sms_clean_date);
	}
	
	//sync
	$sync = false;
	if(isset($_POST['spans'])){
		$sync = true;
		for($i=1;$i<=$__GSM_SUM__;$i++){
			if(isset($_POST['spans'][1][$i])){
				$sync_port[$i] = $_POST['spans'][1][$i];
			}
		}
	}
	
	if($sync){
		foreach($sync_port as $port => $value){
			$sel_res = $aql->query("select * from calllimit_settings.conf where section='$port'");
			if(!isset($sel_res[$port])){
				$aql->assign_addsection($port,'');
			}
			foreach($_POST as $key => $val){
				if($key == 'call_limit_switch' || $key == 'call_time_switch' || $key == 'call_detect_flag' || $key == 'call_fail_mark_flag' || $key == 'call_fail_lock_flag' ||
					$key == 'call_fail_lock_sms_flag' || $key == 'call_fail_lock_sms_report_flag' || $key == 'sms_limit_switch' || $key == 'sms_limit_success_flag' || $key == 'time_sms_swtich'){
						continue;
					}
				
				if(isset($_POST[$key.'_sync'])){
					if(isset($sel_res[$port][$key])){
						$aql->assign_editkey($port,$key,$_POST[$key]);
					}else{
						$aql->assign_append($port,$key,$_POST[$key]);
					}
				}
			}
			
			//call_limit_switch
			if(isset($_POST['call_limit_switch_sync'])){
				if($call_limit_switch == 0){
					exec("/my_tools/set_calllimit.sh unlimited $port");
				}
				if(isset($sel_res[$port]['call_limit_switch'])){
					$aql->assign_editkey($port,'call_limit_switch',$call_limit_switch);
				}else{
					$aql->assign_append($port,'call_limit_switch',$call_limit_switch);
				}
			}
			
			//call_time_switch
			if(isset($_POST['call_time_switch_sync'])){
				if(isset($sel_res[$port]['call_time_switch'])){
					$aql->assign_editkey($port,'call_time_switch',$call_time_switch);
				}else{
					$aql->assign_append($port,'call_time_switch',$call_time_switch);
				}
			}
			
			//call_time_single_switch
			if(isset($_POST['call_time_single_switch_sync'])){
				if(isset($sel_res[$port]['call_time_single_switch'])){
					$aql->assign_editkey($port,'call_time_single_switch',$call_time_single_switch);
				}else{
					$aql->assign_append($port,'call_time_single_switch',$call_time_single_switch);
				}
			}
			
			//call_time_total_switch
			if(isset($_POST['call_time_total_switch_sync'])){
				if(isset($sel_res[$port]['call_time_total_switch'])){
					$aql->assign_editkey($port,'call_time_total_switch',$call_time_total_switch);
				}else{
					$aql->assign_append($port,'call_time_total_switch',$call_time_total_switch);
				}
			}
			
			//call_time_clean_switch
			if(isset($_POST['call_time_clean_switch_sync'])){
				if(isset($sel_res[$port]['call_time_clean_switch'])){
					$aql->assign_editkey($port,'call_time_clean_switch',$call_time_clean_switch);
				}else{
					$aql->assign_append($port,'call_time_clean_switch',$call_time_clean_switch);
				}
			}
			
			//call_detect_flag
			if(isset($_POST['call_detect_flag_sync'])){
				if($call_detect_flag == 0){
					exec("/my_tools/set_calllimit.sh unlock $port");
				}
				if(isset($sel_res[$port]['call_detect_flag'])){
					$aql->assign_editkey($port,'call_detect_flag',$call_detect_flag);
				}else{
					$aql->assign_append($port,'call_detect_flag',$call_detect_flag);
				}
			}
			
			//call_fail_mark_flag
			if(isset($_POST['call_fail_mark_flag_sync'])){
				if($call_fail_mark_flag == 0){
					exec("/my_tools/set_calllimit.sh unmark $port");
				}
				if(isset($sel_res[$port]['call_fail_mark_flag'])){
					$aql->assign_editkey($port,'call_fail_mark_flag',$call_fail_mark_flag);
				}else{
					$aql->assign_append($port,'call_fail_mark_flag',$call_fail_mark_flag);
				}
			}
			
			//call_fail_lock_flag
			if(isset($_POST['call_fail_lock_flag_sync'])){
				if($call_fail_lock_flag == 0){
					exec("/my_tools/set_calllimit.sh unlock $port");
				}
				if(isset($sel_res[$port]['call_fail_lock_flag'])){
					$aql->assign_editkey($port,'call_fail_lock_flag',$call_fail_lock_flag);
				}else{
					$aql->assign_append($port,'call_fail_lock_flag',$call_fail_lock_flag);
				}
			}
			
			//call_fail_lock_sms_flag
			if(isset($_POST['call_fail_lock_sms_flag_sync'])){
				if(isset($sel_res[$port]['call_fail_lock_sms_flag'])){
					$aql->assign_editkey($port,'call_fail_lock_sms_flag',$call_fail_lock_sms_flag);
				}else{
					$aql->assign_append($port,'call_fail_lock_sms_flag',$call_fail_lock_sms_flag);
				}
			}
			
			//call_fail_lock_sms_report_flag
			if(isset($_POST['call_fail_lock_sms_report_flag_sync'])){
				if(isset($sel_res[$port]['call_fail_lock_sms_report_flag'])){
					$aql->assign_editkey($port,'call_fail_lock_sms_report_flag',$call_fail_lock_sms_report_flag);
				}else{
					$aql->assign_append($port,'call_fail_lock_sms_report_flag',$call_fail_lock_sms_report_flag);
				}
			}
			
			//sms_limit_switch
			if(isset($_POST['sms_limit_switch_sync'])){
				if($sms_limit_switch == 0){
					exec("/my_tools/calllimit_cli set chn monsmscnt $port 0");
				}
				if(isset($sel_res[$port]['sms_limit_switch'])){
					$aql->assign_editkey($port,'sms_limit_switch',$sms_limit_switch);
				}else{
					$aql->assign_append($port,'sms_limit_switch',$sms_limit_switch);
				}
			}
			
			//time_sms_swtich
			if(isset($_POST['time_sms_swtich_sync'])){
				if(isset($sel_res[$port]['time_sms_swtich'])){
					$aql->assign_editkey($port,'time_sms_swtich',$time_sms_swtich);
				}else{
					$aql->assign_append($port,'time_sms_swtich',$time_sms_swtich);
				}
			}
			
			//sms_start_time
			if(isset($_POST['sms_start_time_sync'])){
				if(isset($sel_res[$port]['time_sms_start'])){
					$aql->assign_editkey($port,'time_sms_start',$sms_start_time);
				}else{
					$aql->assign_append($port,'time_sms_start',$sms_start_time);
				}
			}
			
			//sms_end_time
			if(isset($_POST['sms_end_time_sync'])){
				if(isset($sel_res[$port]['time_sms_end'])){
					$aql->assign_editkey($port,'time_sms_end',$sms_end_time);
				}else{
					$aql->assign_append($port,'time_sms_end',$sms_end_time);
				}
			}
			
			//sms_limit_success_flag
			if(isset($_POST['sms_limit_success_flag_sync'])){
				if(isset($sel_res[$port]['sms_limit_success_flag'])){
					$aql->assign_editkey($port,'sms_limit_success_flag',$sms_limit_success_flag);
				}else{
					$aql->assign_append($port,'sms_limit_success_flag',$sms_limit_success_flag);
				}
			}
		}
	}
	
	if(!$aql->save_config_file('calllimit_settings.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	
	if(!isset($_POST['call_limit_switch'])){
		exec("/my_tools/set_calllimit.sh unlimited $channel");
	}
	
	if(!isset($_POST['call_detect_flag']) || !isset($_POST['call_fail_lock_flag'])){
		exec("/my_tools/set_calllimit.sh unlock $channel");
	}
	
	if(!isset($_POST['call_fail_mark_flag'])){
		exec("/my_tools/set_calllimit.sh unmark $channel");
	}
	
	if(!isset($_POST['sms_limit_switch'])){
		exec("/my_tools/calllimit_cli set chn monsmscnt $channel 0");
	}

	if(!isset($_POST['call_time_switch'])){
		exec("/my_tools/set_calllimit.sh resetcalltime $channel");
	}
	
	wait_apply("exec","/my_tools/set_calllimit.sh reload");
	
	save_user_record("","MODULE->Call And SMS Limit:Save,channel=".$channel);
}

function clean_sms_day(){
	$port_arr = $_GET['port'];
	for($i=0;$i<count($port_arr);$i++){
		$port = $port_arr[$i];
		exec("/my_tools/calllimit_cli set chn daysmscnt $port 0");
	}
	
	save_user_record("","MODULE->Call And SMS Limit:Clean SMS Day");
}

function clean_sms_month(){
	$port_arr = $_GET['port'];
	for($i=0;$i<count($port_arr);$i++){
		$port = $port_arr[$i];
		exec("/my_tools/calllimit_cli set chn monsmscnt $port 0");
	}
	
	save_user_record("","MODULE->Call And SMS Limit:Clean SMS Month");
}

function Remain_time_reset(){
	$channel = $_POST['channel'];
	exec("/my_tools/set_calllimit.sh resetcalltime $channel");
	header("Location:".get_self()."?sel_gsm=".$channel.'&send=Modify');
	
	save_user_record("","MODULE->Call And SMS Limit:Remain Time Reset,channel=".$channel);
}

if($_POST){
	if(isset($_POST['send']) && $_POST['send'] == 'Save'){
		if($only_view){
			return false;
		}
		
		save_gsm_calllimit();
		show_gsm_calllimit();
	}else if(isset($_POST['send']) && $_POST['send'] == 'Reset'){
		if($only_view){
			return false;
		}
		
		Remain_time_reset();
	}
}else if($_GET){
	if(isset($_GET['send']) && $_GET['send'] == 'Modify'){
		edit_gsm_calllimit();
	}else if($_GET['send'] == 'Clean Day'){
		if($only_view){
			return false;
		}
		
		clean_sms_day();
		show_gsm_calllimit();
	}else if($_GET['send'] == 'Clean Month'){
		if($only_view){
			return false;
		}
		
		clean_sms_month();
		show_gsm_calllimit();
	}
}else{
	show_gsm_calllimit();
}

require("/www/cgi-bin/inc/boot.inc");
?>