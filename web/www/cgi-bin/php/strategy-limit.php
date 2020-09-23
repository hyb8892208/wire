<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
include_once("/www/cgi-bin/inc/aql.php");
?>

<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/functions.js"></script>
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />
<script type="text/javascript" src="/js/jquery.ibutton.js"></script>

<?php
function show_strategy_limit(){
	global $__GSM_SUM__;
	exec("/my_tools/calllimit_cli show allstatus",$cmd_res);

	$allstatus = json_decode($cmd_res[0],true);
	
	$res_arr = [];
	foreach($allstatus as $key => $val){
		$temp = explode("-", $key);
		$channel = $temp[0];
		$sim_id = $temp[1];
		$res_arr[$channel][$sim_id] = $val;
	}
?>

	<form enctype="multipart/form-data" action="<?php echo get_self();?>" method="get" >
		<input type="hidden" id="sel_gsm" name="sel_gsm" value="" />

		<div id="strategy_batch" style="float:left;">
			<button id="batch_unlimit" type="button"><?php echo language('Batch Unlimit'); ?></button>
		</div>
		
		<div id="status_help">
			<div class="status_help_each" >
				<img src="/images/sim-idle.png" />
				<span><?php echo language('Idle');?></span>
			</div>
			<div class="status_help_each" >
				<img src="/images/sim-free.png" />
				<span><?php echo language('Free');?></span>
			</div>
			<div class="status_help_each" >
				<img src="/images/sim-busy.png" />
				<span><?php echo language('Busy');?></span>
			</div>
			<div class="status_help_each" >
				<img src="/images/sim-error.png" />
				<span><?php echo language('Error');?></span>
			</div>
			<div class="status_help_each" >
				<img src="/images/sim-register.png" />
				<span><?php echo language('Register');?></span>
			</div>
			<div class="status_help_each" >
				<img src="/images/no-sim.png" />
				<span><?php echo language('No Sim');?></span>
			</div>
			<div class="status_help_each" >
				<img src="/images/call-limit.png" />
				<span><?php echo language('Limit');?></span>
			</div>
		</div>
		
		<table width="100%" class="tshow">
			<tr>
				<th style="width:0.3%;"><input type="checkbox" name="selall_port" onclick="selectAll(this.checked,'port[]')" /></th>
				<th width="150px"><?php echo language('Port');?></th>
				<th width="245px"><?php echo language('Card 1');?></th>
				<th width="245px"><?php echo language('Card 2');?></th>
				<th width="245px"><?php echo language('Card 3');?></th>
				<th width="245px"><?php echo language('Card 4');?></th>
				<th width="38px"><?php echo language('Actions');?></th>
			</tr>
			
			<?php
			for($i=1;$i<=$__GSM_SUM__;$i++){
				$key = get_gsm_name_by_channel_for_showtype($i);
				$channel_type = get_gsm_type_by_channel($i,1);
				$val = $res_arr[$i];
			?>
			<tr <?php if($channel_type == 'NULL') echo 'style="display:none;"'; ?> >
				<td><input type="checkbox" class="limit_checked" name="port[]" value="<?php echo $key;?>" /></td>
				<td><?php echo $key;?></td>
				<?php for($j=1;$j<=4;$j++){ 
						$k = $j;
				?>
				<td>
					<?php 
					//sim status
					if($val[$k]['sim_sta'] == '1'){
						echo '<img src="/images/sim-idle.png" alt="'.language("Idle").'" title="'.language("Idle").'"/>';
					}else if($val[$k]['sim_sta'] == '2'){
						echo '<img src="/images/sim-free.png" alt="'.language("Free").'" title="'.language("Free").'"/>';
					}else if($val[$k]['sim_sta'] == '3'){
						echo '<img src="/images/sim-busy.png" alt="'.language("Busy").'" title="'.language("Busy").'"/>';
					}else if($val[$k]['sim_sta'] == '4'){
						echo '<img src="/images/sim-error.png" alt="'.language("Error").'" title="'.language("Error").'"/>';
					}else if($val[$k]['sim_sta'] == '5'){
						echo '<img src="/images/sim-register.png" alt="'.language("Register").'" title="'.language("Register").'"/>';
					}else{
						echo '<img src="/images/no-sim.png" alt="'.language("No Sim").'" title="'.language("No Sim").'"/>';
					}
					
					//limit status
					if($val[$k]['limit_sta'] == 'LIMIT' || $val[$k]['call_time_limit_sta'] == 'LIMIT'){
						echo '<img src="/images/call-limit.png" style="cursor:pointer" class="limit_sta" alt="'.language("Limit").'" title="'.language("Limit").'" />';
					}
					?>
					<input type="hidden" class="channel" value="<?php echo $key;?>" />
					<input type="hidden" class="sim_id" value="<?php echo $k;?>" />
				</td>
				<?php } ?>
				<td>
					<button type="submit" value="Modify" style="width:32px;height:32px;"
						onclick="document.getElementById('send').value='Modify';setValue('<?php echo $key;?>', 1)">
						<img src="/images/edit.gif">
					</button>
				</td>
			</tr>
			<?php 
			}
			?>
			
		</table>
		
		<br/>
		
		<input type="hidden" name="send" id="send" value="" />
	</form>
	
	<script>
	function setValue(channel){
		document.getElementById('sel_gsm').value = channel;
	}
	
	//unlimit
	$(".limit_sta").click(function(){
		var ret = confirm("<?php echo language('Unlimit Tip','This operation will unlimited the call times limit. Are you sure you want to do this?')?>");
		
		var channel = $(this).siblings(".channel").val();
		var sim_id = $(this).siblings(".sim_id").val();
		if(ret){
			var arr = [{
				'channel':channel,
				'sim_id':sim_id
			}];
			
			$.ajax({
				type:"POST",
				data:{
					'arr':arr
				},
				url:"ajax_server.php?type=set_sim_unlimited",
				success:function(data){
					window.location.reload();
				},
				error:function(){
					alert("Unlimit Failed!");
				}
			});
		}
	});
	
	//batch_unlimit
	$("#batch_unlimit").click(function(){
		var ret = confirm("<?php echo language('Unlimit Tip','This operation will unlimited the call times limit. Are you sure you want to do this?');?>");
		
		if(ret){
			if($(".limit_checked:checked").length == 0){
				alert("<?php echo language("Please choose port");?>");
			}else if($(".limit_checked").size() == $(".limit_checked:checked").length){
				$.ajax({
					type:"GET",
					url:"ajax_server.php?type=sim_unlimit_all",
					success:function(data){
						window.location.reload();
					},
					error:function(){
						console.log(data);
					}
				});
			}else{
				var arr = [];
				$(".limit_checked").each(function(){
					var channel = $(this).val();
					
					if($(this).attr('checked') == 'checked'){
						for(var i=1;i<=4;i++){
							var obj = {
								'channel':channel,
								'sim_id':i
							}
							arr.push(obj);
						}
					}
				});
				
				$.ajax({
					type:"POST",
					data:{
						'arr':arr
					},
					url:"ajax_server.php?type=set_sim_unlimited",
					success:function(data){
						window.location.reload();
					},
					error:function(data){
						console.log(data);
					}
				});
			}
		}
	});
	</script>

<?php 
} 

function edit_strategy_limit(){
	global $__GSM_SUM__;
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
	
	$temp = exec("/my_tools/set_calllimit.sh status $channel | grep call_time_remain");
	$temp_arr = explode(':',$temp,2);
	$call_time_remain = $temp_arr[1];
	if($call_time_remain == ''){
		$call_time_remain = language('No Limit');
	}
?>
	<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
	<link type="text/css" href="/css/jquery-ui-timepicker-addon.css" rel="stylesheet" media="all"/>

	<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>
	<script type="text/javascript" src="/js/float_btn.js"></script>
	<script type="text/javascript" src="/js/jquery-ui-timepicker-addon.js"></script>
	<script type="text/javascript" src="/js/jquery-ui-sliderAccess.js"></script>
	<script type="text/javascript" src="/js/float_btn.js"></script>
	<form id="manform" enctype="multipart/form-data" action="<?php echo get_self();?>" method="post">
		<input type="hidden" name="channel" id="channel" value="<?php echo $channel;?>" />
		<div id="tab">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('Call Limit@time','Call Limit');?> (<?php echo get_gsm_name_by_channel($channel,1,false);?>)</li>
			<li class="tb2">&nbsp;</li>
		</div>
		
		<table width="100%" class="tedit">
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Call Limit Switch');?>:
						<span class="showhelp">
						<?php echo language('Call Limit Switch help');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="call_limit_switch_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="checkbox" name="call_limit_switch" id="call_limit_switch" <?php echo $call_limit_switch;?> />
					<span id="ccall_limit_switch"></span>
				</td>
			</tr>
			
			<tr class="call_limit_tr">
				<th>
					<div class="helptooltips">
						<?php echo language('Limit Daily Call Times');?>:
						<span class="showhelp">
						<?php echo language('Limit Daily Call Times help', 'Limit the number of calls per day (regardless of whether the person is connected), the default value is 0, indicating unlimited.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="day_calls_settings_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="text" name="day_calls_settings" id="day_calls_settings" value="<?php echo $day_calls_settings;?>" />
					<span id="cday_calls_settings"></span>
				</td>
			</tr>
			
			<tr class="call_limit_tr">
				<th>
					<div class="helptooltips">
						<?php echo language('Limit Daily Answer Times');?>:
						<span class="showhelp">
						<?php echo language('Limit Daily Answer Times help', 'Limit the number of calls per day and the number of answers. The default value is 0, indicating no limit.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="day_answer_setting_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="text" name="day_answer_setting" id="day_answer_setting" value="<?php echo $day_answer_setting;?>" />
					<span id="cday_answer_setting"></span>
				</td>
			</tr>
			
			<tr class="call_limit_tr">
				<th>
					<div class="helptooltips">
						<?php echo language('Limit Hour Call Times');?>:
						<span class="showhelp">
						<?php echo language('Limit Hour Call Times help', 'Limit the number of calls per hour (regardless of whether the person is connected), the default value is 0, indicating unlimited.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="hour_calls_settings_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="text" name="hour_calls_settings" id="hour_calls_settings" value="<?php echo $hour_calls_settings;?>" />
					<span id="chour_calls_settings"></span>
				</td>
			</tr>
		</table>
		
		<br/>
		
		<div id="tab">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('Call Limit Time');?></li>
			<li class="tb2">&nbsp;</li>
		</div>
		
		<table width="100%" class="tedit">
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Call Time Limit Switch');?>:
						<span class="showhelp">
						<?php echo language('Call Time Limit Switch help');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="call_time_switch_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="checkbox" name="call_time_switch" id="call_time_switch" <?php echo $call_time_switch; ?> />
				</td>
			</tr>
			
			<tbody id="call_time_switch_show">
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Step');?>:
							<span class="showhelp">
							<?php echo language('Call Duration Limit Step help','Step length value range is 1-999.<br>Step length multiplied by time of single call just said a single call duration time allowed.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="call_time_step_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="text" name="call_time_step" id="call_time_step" maxlength="3" value="<?php echo $call_time_step;?>" oninput="check_step(this.id);" onkeyup="check_step(this.id);"/>
						<span class="chelp"><?php echo language('Second');?></span>
						<span id="ccall_time_step"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Enable Single Call Duration Limit');?>:
							<span class="showhelp">
							<?php echo language('Enable Single Call Duration Limit help','Definite maximum call duration for single call. <br>Example: if Time of single call set to 10, the call will be disconnected after <br>talking 10*step seconds.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="call_time_single_switch_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="checkbox" name="call_time_single_switch" id="call_time_single_switch" <?php echo $call_time_single_switch;?> />
					</td>
				</tr>
				
				<tr id="call_time_settings_show">
					<th>
						<div class="helptooltips">
							<?php echo language('Single Call Duration Limitation');?>:
							<span class="showhelp">
							<?php echo language('Single Call Duration Limitation help','The value of limitation single call, this value range is 1-999999. <br>Step length multiplied by time of single call just said a single call duration time allowed.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="call_time_settings_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="text" name="call_time_settings" id="call_time_settings" maxlength="6" value="<?php echo $call_time_settings;?>" oninput="check_dl_integer(this.id);" onkeyup="check_dl_integer(this.id);" />
						<span id="ccall_time_settings"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Enable Call Duration Limitation');?>:
							<span class="showhelp">
							<?php echo language('Enable Call Duration Limitation help','This function is to limit the total call duration of GSM channel. <br>The max call duration is between 1 to 999999 steps.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="call_time_total_switch_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="checkbox" name="call_time_total_switch" id="call_time_total_switch" <?php echo $call_time_total_switch;?> />
					</td>
				</tr>
				
				<tbody id="call_time_total_switch_show">
					<tr>
						<th>
							<div class="helptooltips">
								<?php echo language('Call Duration Limitation');?>:
								<span class="showhelp">
								<?php echo language('Call Duration Limitation help','The value of total call limitation, this value range is 1-999999. <br>Step length multiplied by time of single call just said a single call duration time allowed.');?>
								</span>
							</div>
						</th>
						<td>
							<input type="checkbox" name="call_time_total_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
							<input type="text" name="call_time_total" id="call_time_total" maxlength="6" value="<?php echo $call_time_total;?>" oninput="check_dl_integer(this.id);" onkeyup="check_dl_integer(this.id);" />
							<span id="ccall_time_total"></span>
						</td>
					</tr>
					
					<tr>
						<th>
							<div class="helptooltips">
								<?php echo language('Minimum Charging Time');?>:
								<span class="showhelp">
								<?php echo language('Minimum Charging Time help','A single call over this time, <br>GSM side of the operators began to collect fees, unit for seconds.<br>The value must be less than Step length.');?>
								</span>
							</div>
						</th>
						<td>
							<input type="checkbox" name="call_time_free_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
							<input type="text" name="call_time_free" id="call_time_free" maxlength="3" value="<?php echo $call_time_free;?>" oninput="check_freetime(this.id);" onkeyup="check_freetime(this.id);" />
							<span class="chelp"><?php echo language('Second');?></span>
							<span id="ccall_time_free"></span>
						</td>
					</tr>
					
					<tr>
						<th>
							<div class="helptooltips">
								<?php echo language('Alarm Threshold');?>:
								<span class="showhelp">
								<?php echo language('Alarm Threshold help','Define a threshold value of call duration. <br>While the call steps equal to(or less than) this value, <br>the gateway will send alarm information to designated phone number via SMS(Send only once before the total call length is reset).<br>The value must be less than call duration limitation.');?>
								</span>
							</div>
						</th>
						<td>
							<input type="checkbox" name="call_time_warning_num_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
							<input type="text" name="call_time_warning_num" id="call_time_warning_num" maxlength="6" value="<?php echo $call_time_warning_num;?>" oninput="check_warning_time(this.id);" onkeyup="check_warning_time(this.id);" />
							<span id="ccall_time_warning_num"></span>
						</td>
					</tr>
					
					<tr>
						<th>
							<div class="helptooltips">
								<?php echo language('Alarm Phone Number');?>:
								<span class="showhelp">
								<?php echo language('Alarm Phone Number help','Receiving alarm phone number, user will received alarm message from gateway.');?>
								</span>
							</div>
						</th>
						<td>
							<input type="checkbox" name="call_time_warning_callee_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
							<input type="text" name="call_time_warning_callee" id="call_time_warning_callee" value="<?php echo $call_time_warning_callee;?>" oninput="check_dl_integer(this.id)" onkeyup="check_dl_integer(this.id);" />
							<span id="ccall_time_warning_callee"></span>
						</td>
					</tr>
					
					<tr>
						<th>
							<div class="helptooltips">
								<?php echo language('Alarm Description');?>:
								<span class="showhelp">
								<?php echo language('Alarm Description help','Alarm port information description, <br>which will be sent to user mobile phone with alarm information.');?>
								</span>
							</div>
						</th>
						<td>
							<input type="checkbox" name="call_time_warning_msg_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
							<input type="text" name="call_time_warning_msg" id="call_time_warning_msg" value="<?php echo $call_time_warning_msg;?>" size="50" oninput="check_warning_describe(this.id);" onkeyup="check_warning_describe(this.id);"/>
							<span id="ccall_time_warning_msg"></span>
						</td>
					</tr>
					
					<tr>
						<th>
							<div class="helptooltips">
								<?php echo language('Remain Time');?>:
								<span class="showhelp">
								<?php echo language('Remain Time help','This value is multiplied by to step length is a rest call time.');?>
								</span>
							</div>
						</th>
						<td>
							<input type="text" name="call_time_remain" id="call_time_remain" value="<?php echo $call_time_remain;?>" readonly disabled/>
							<input type="button" class="cbutton" value="<?php echo language('Reset');?>" onclick="remain_time_reset()">
						</td>
					</tr>
					
					<tr>
						<th>
							<div class="helptooltips">
								<?php echo language('Enable Auto Reset');?>:
								<span class="showhelp">
								<?php echo language('Enable Auto Reset help','Automatic restore remaining talk time, <br>that is, get total call minutes of GSM channel.');?>
								</span>
							</div>
						</th>
						<td>
							<input type="checkbox" name="call_time_clean_switch_sync" class="setting_sync" title="<?php echo lang_sync_title;?>" />
							<input type="checkbox" name="call_time_clean_switch" id="call_time_clean_switch" <?php echo $call_time_clean_switch;?> />
						</td>
					</tr>
					
					<tbody id="call_time_clean_switch_show">
						<tr>
							<th>
								<div class="helptooltips">
									<?php echo language('Auto Reset Type');?>:
									<span class="showhelp">
									<?php echo language('Auto Reset Type help','Reset call minutes by date, by week, by month.');?>
									</span>
								</div>
							</th>
							<td>
								<input type="checkbox" name="call_time_clean_type_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
								<select name="call_time_clean_type">
									<option value="1" <?php if($call_time_clean_type=='1') echo 'selected';?>><?php echo language('Day');?>(1<?php echo language('Day');?>)</option>
									<option value="2" <?php if($call_time_clean_type=='2') echo 'selected';?>><?php echo language('Week');?>(7<?php echo language('Day');?>)</option>
									<option value="3" <?php if($call_time_clean_type=='3') echo 'selected';?>><?php echo language('Month');?></option>
								</select>
							</td>
						</tr>
						
						<tr>
							<th>
								<div class="helptooltips">
									<?php echo language('Next Reset Time');?>:
									<span class="showhelp">
									<?php echo language('Next Reset Time help','Defined next reset date. <br>System will count start from that date and work as Reset Period setting.');?>
									</span>
								</div>
							</th>
							<td>
								<input type="checkbox" class="setting_sync" name="call_time_clean_date_sync" title="<?php echo $lang_sync_title;?>" />
								<input type="text" name="call_time_clean_date" id="call_time_clean_date" value="<?php echo $call_time_clean_date;?>" onchange="check_dl_datetime(this.id);" />
								<span id="ccall_time_clean_date"></span>
							</td>
						</tr>
					</tbody>
				</tbody>
			</tbody>
		</table>
		
		<br/>
		
		<div id="tab" class="div_tab_title">
			<li class="tb_fold" onclick="lud(this,'save_to_other_ports')" id="save_to_other_ports_li">&nbsp;</li>
			<li class="tbg_fold" onclick="lud(this,'save_to_other_ports')"><?php echo language('Save To Other Ports');?></li>
			<li class="tb2_fold" onclick="lud(this,'save_to_other_ports')">&nbsp;</li>
			<li class="tb_end2">&nbsp;</li>
		</div>
		
		<div id="save_to_other_ports" style="display:none">
			<table width="100%" class="tedit" >
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Save To Other Ports');?>:
							<span class="showhelp">
							<?php echo language('GSM Save To Other Ports help');?>
							</span>
						</div>
					</th>
					<td>
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
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Sync All Settings');?>:
							<span class="showhelp">
							<?php echo language('GSM Sync All Settings help');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" id="all_settings_sync" onclick="selectAllCheckbox(this.checked,'class','setting_sync');" checked disabled />
						<?php echo language('Select all settings');?>
					</td>
				</tr>
			</table>	
		</div>
		
		<br/>
	
		<input type="hidden" name="send" id="send" value="" />
		
		<table id="float_btn" class="float_btn">
			<tr id="float_btn_tr" class="float_btn_tr" style="padding-left: 15px;">
				<td>
					<input type="submit" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>
				</td>	
				<td>
					<input type="button" value="<?php echo language('Cancel');?>" onclick="window.location.href='<?php echo get_self();?>'" />
				</td>
			</tr>
		</table>
		<table id="float_btn2" style="border:none;" class="float_btn2">
			<tr id="float_btn_tr2" class="float_btn_tr2" style="padding-left: 15px;">
				<td style="width:50px">
					<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" />
				</td>
				<td>
					<input type="button" id="float_button_2" class="float_short_button" value="<?php echo language('Cancel');?>" onclick="window.location.href='<?php echo get_self();?>'"/>
				</td>
			</tr>
		</table>
	</form>

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
				<li><a style="color:#CD3278;" href="/cgi-bin/php/strategy-limit.php?sel_gsm=<?php echo $cnum; ?>&send=Modify" ><?php echo get_gsm_name_by_channel($cnum,$bnum);?></a></li>
		<?php
					}else{
		?>
				<li><a style="color:LemonChiffon4;" href="/cgi-bin/php/strategy-limit.php?sel_gsm=<?php echo $cnum; ?>&send=Modify" ><?php echo get_gsm_name_by_channel($cnum,$bnum);?></a></li>
		<?php
					}
				}
			}
		?>
		</div>
	</div>
	
	<script>
	$(function(){
		$("#call_limit_switch").iButton();
		$("#call_time_switch").iButton();
		$("#call_time_single_switch").iButton();
		$("#call_time_total_switch").iButton();
		$("#call_time_clean_switch").iButton();
		
		$(".port").click(function(){
			handle_port_sync();
		});
		
		//sidebar
		var sort_info_top = $("#lps").offset().top;
		$("#sort_gsm_cli").offset({top: sort_info_top });
		$("#sort_out").offset({top: sort_info_top });
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
		
		//auto reset data
		$("#call_time_clean_date" ).datetimepicker({
			dateFormat: "yy-mm-dd", 
			timeFormat: "HH:mm:ss", 
			beforeShow: function(){
				setTimeout(function(){
					$('#ui-datepicker-div').css("z-index",5);
				},0);
			}
		});
		$("#call_time_clean_date" ).datetimepicker();
		
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
		//call times limit
		if(document.getElementById('call_limit_switch').checked){
			var day_calls_settings = document.getElementById('day_calls_settings').value;
			document.getElementById('cday_calls_settings').innerHTML = '';
			if(isNaN(day_calls_settings) || parseInt(day_calls_settings) < 0){
				document.getElementById('day_calls_settings').focus();
				document.getElementById('cday_calls_settings').innerHTML = con_str("Must be Number.Range:>=0.");
				return false;
			}
			
			var day_answer_setting = document.getElementById('day_answer_setting').value;
			document.getElementById('cday_answer_setting').innerHTML = '';
			if(isNaN(day_answer_setting) || parseInt(day_answer_setting) < 0){
				document.getElementById('day_answer_setting').focus();
				document.getElementById('cday_answer_setting').innerHTML = con_str("Must be Number.Range:>=0");
				return false;
			}
			
			var hour_calls_settings = document.getElementById('hour_calls_settings').value;
			document.getElementById('chour_calls_settings').innerHTML = '';
			if(isNaN(hour_calls_settings) || parseInt(hour_calls_settings) < 0){
				document.getElementById('hour_calls_settings').focus();
				document.getElementById('chour_calls_settings').innerHTML = con_str("Must be Number.Range:>=0");
				return false;
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
					return false;
				}
			}
			
			if(document.getElementById('call_time_total_switch').checked){
				var call_time_total = document.getElementById('call_time_total').value;
				document.getElementById('ccall_time_total').innerHTML = '';
				if(isNaN(call_time_total) || parseInt(call_time_total) < 1){
					document.getElementById('call_time_total').focus();
					document.getElementById('ccall_time_total').innerHTML = con_str("Must be Number.Range:>=0");
					return false;
				}
			}*/
			
			if(document.getElementById('call_time_single_switch').checked && document.getElementById('call_time_total_switch').checked){
				var call_time_settings = document.getElementById('call_time_settings').value;
				var call_time_total = document.getElementById('call_time_total').value;
				document.getElementById('ccall_time_settings').innerHTML = '';
				if(parseInt(call_time_settings) > parseInt(call_time_total)){
					document.getElementById('call_time_settings').focus();
					document.getElementById('ccall_time_settings').innerHTML = con_str("<?php echo language('Single Limit Time Less Tip', 'Single Call Duration Limitation must be less than or equal to Call Duration Limitation.');?>");
					return false;
				}
			}
		}
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
		obj.value = obj.value.replace(/[^-: \d]*/g,'');

		val = obj.value;
		
		if(!check_datetime(val)) {
			$("#c"+id).html(con_str("<?php echo language('js check datetime','Please input right format date and time. eg: 2013-09-03 19:44:31');?>"));
			return false;
		} else {
			$("#c"+id).html("");
			return true;
		}
	}
	</script>

<?php 
}

function save_strategy_limit(){
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
				if($key == 'call_time_switch' || $key == 'call_time_single_switch' || $key == 'call_time_total_switch' || $key == 'call_time_clean_switch'){
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
	
	if(!isset($_POST['call_time_switch'])){
		exec("/my_tools/set_calllimit.sh resetcalltime $channel");
	}
	
	wait_apply("exec","/my_tools/set_calllimit.sh reload");
}

function Remain_time_reset(){
	$channel = $_POST['channel'];
	exec("/my_tools/set_calllimit.sh resetcalltime $channel");
	header("Location:".get_self()."?sel_gsm=".$channel.'&send=Modify');
}

if($_POST){
	if(isset($_POST['send']) && $_POST['send'] == 'Save'){
		save_strategy_limit();
		show_strategy_limit();
	}else if(isset($_POST['send']) && $_POST['send'] == 'Reset'){
		Remain_time_reset();
	}
}else if($_GET){
	if(isset($_GET['send']) && $_GET['send'] == 'Modify'){
		edit_strategy_limit();
	}
}else{
	show_strategy_limit();
}

require("/www/cgi-bin/inc/boot.inc");
?>
<div id="float_btn1" class="sec_float_btn1"></div>
<div class="float_close" onclick="close_btn()" ></div>