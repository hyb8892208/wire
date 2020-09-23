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
function show_strategy_switch(){
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
			<button id="batch_unlock" type="button"><?php echo language('Batch Unlock'); ?></button>
			<button id="batch_unmark" type="button"><?php echo language('Batch Unmark'); ?></button>
			<button id="batch_sms_unlimit" type="button"><?php echo language('Batch SMS Unlimit'); ?></button>
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
			<div class="status_help_each" >
				<img src="/images/call-lock.png" />
				<span><?php echo language('Locked');?></span>
			</div>
			<div class="status_help_each" >
				<img src="/images/marker.png" />
				<span><?php echo language('Marker');?></span>
			</div>
			<div class="status_help_each" >
				<img src="/images/sms-failed.png" style="margin-top:3px;"/>
				<span><?php echo language('SMS Limit');?></span>
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
				<td><input type="checkbox" class="strategy_checked" name="port[]" value="<?php echo $key;?>" /></td>
				<td><?php echo $key;?></td>
				<?php for($j=1;$j<=4;$j++){ 
						$k = $j;
				?>
					
				<td>
					<?php 
					//sim status
					if($val[$k]['sim_sta'] == '1'){
						echo '<img src="/images/sim-idle.png" alt="'.language("Idle").'" class="idle" title="'.language("Idle").'('.language("Click Switch").')" style="cursor:pointer;"/>';
					}else if($val[$k]['sim_sta'] == '2'){
						echo '<img src="/images/sim-free.png" alt="'.language("Free").'" class="free" title="'.language("Free").'"/>';
					}else if($val[$k]['sim_sta'] == '3'){
						echo '<img src="/images/sim-busy.png" alt="'.language("Busy").'" class="busy" title="'.language("Busy").'"/>';
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
					
					//lock status
					if($val[$k]['call_fail_lock_status'] == 1){
						echo '<img src="/images/call-lock.png" style="cursor:pointer" class="lock_sta" alt="'.language("Locked").'" title="'.language("Lock").'" />';
					}else{
						//marker status
						if($val[$k]['call_fail_mark_status'] == 1){
							echo '<img src="/images/marker.png" style="cursor:pointer;" class="marker_sta" alt="'.language("Marker").'" title="'.language("Marker").'" />';
						}
					}
					
					//sms status
					if($val[$k]['sms_limit_sta'] == 1){
						echo '<img src="/images/sms-failed.png" style="cursor:pointer;" class="sms_failed_sta" alt="'.language("SMS Limit").'" title="'.language("SMS Limit").'" />';
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
	
	//switch sim card
	$(".idle").click(function(){
		var ret = confirm("<?php echo language('Switch Card help','Switching will clear all limit states.Are you sure you want to switch the card?');?>");
		
		if(ret){
			var busy_val = 0;
			$(this).parent().siblings().children('img').each(function(){
				var busy_sim = $(this).siblings('.sim_id');
				if($(this).hasClass("busy")){
					busy_val = $(busy_sim).val();
				}
			});
			
			if(busy_val == 0){
				var channel = $(this).siblings(".channel").val();
				var sim_id = $(this).siblings(".sim_id").val();
				
				$.ajax({
					type:"GET",
					url:"ajax_server.php?type=switch_sim_card&channel="+channel+"&sim_id="+sim_id,
					success:function(data){
						window.location.reload();
					},
					error:function(data){
						console.log(data);
					}
				});
			}else{
				alert("<?php echo language('SIM Card');?>" + busy_val + "<?php echo language('SIM Card Busy help','in the call, you can\'t switch the SIM card.');?>");
			}
		}
	});
	
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
	
	//unlock
	$(".lock_sta").click(function(){
		var ret = confirm("<?php echo language('Unlock Tip','Are you sure you want to unlock it?');?>");
		
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
				url:"ajax_server.php?type=set_sim_unlock",
				success:function(data){
					window.location.reload();
				},
				error:function(){
					alert("Unlock Failed!");
				}
			});
		}
	});
	
	//unmark
	$(".marker_sta").click(function(){
		var ret = confirm("<?php echo language('Unmark Tip','Are you sure you want to unmark it?');?>");
		
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
				url:"ajax_server.php?type=set_sim_unmark",
				success:function(data){
					window.location.reload();
				},
				error:function(){
					alert("Unmark Failed!");
				}
			});
		}
	});
	
	//sms unlimit
	$(".sms_failed_sta").click(function(){
		var ret = confirm("<?php echo language('SMS Unlimit Tip','Are you sure you want to unlimit it?');?>");
		
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
				url:"ajax_server.php?type=set_sms_limit",
				success:function(data){
					window.location.reload();
				},
				error:function(){
					alert("SMS Unlimit Failed!");
				}
			});
		}
	});
	
	//batch_unlimit
	$("#batch_unlimit").click(function(){
		var ret = confirm("<?php echo language('Unlimit Tip','This operation will unlimited the call times limit. Are you sure you want to do this?');?>");
		
		if(ret){
			if($(".strategy_checked:checked").length == 0){
				alert("<?php echo language("Please choose port");?>");
			}else if($(".strategy_checked").size() == $(".strategy_checked:checked").length){
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
				$(".strategy_checked").each(function(){
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
	
	//batch_unlock
	$("#batch_unlock").click(function(){
		var ret = confirm("<?php echo language('Unlock Tip','Are you sure you want to unlock it?');?>");
		
		if(ret){
			if($(".strategy_checked:checked").length == 0){
				alert("<?php echo language("Please choose port");?>");
			}else if($(".strategy_checked").size() == $(".strategy_checked:checked").length){
				$.ajax({
					type:"GET",
					url:"ajax_server.php?type=sim_unlock_all",
					success:function(data){
						window.location.reload();
					},
					error:function(data){
						console.log(data);
					}
				});
			}else{
				var arr = [];
				$(".strategy_checked").each(function(){
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
					url:"ajax_server.php?type=set_sim_unlock",
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
	
	//batch_unmark
	$("#batch_unmark").click(function(){
		var ret = confirm("<?php echo language('Unmark Tip','Are you sure you want to unmark it?');?>");
		
		if(ret){
			if($(".strategy_checked:checked").length == 0){
				alert("<?php echo language("Please choose port");?>");
			}else if($(".strategy_checked").size() == $(".strategy_checked:checked").length){
				$.ajax({
					type:"GET",
					url:"ajax_server.php?type=sim_unmark_all",
					success:function(data){
						window.location.reload();
					},
					error:function(data){
						console.log(data);
					}
				});
			}else{
				var arr = [];
				$(".strategy_checked").each(function(){
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
					url:"ajax_server.php?type=set_sim_unmark",
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
	
	//batch_sms_unlimit
	$("#batch_sms_unlimit").click(function(){
		var ret = confirm("<?php echo language('SMS Unlimit Tip','Are you sure you want to unlimit it?');?>");
		
		if(ret){
			if($(".strategy_checked:checked").length == 0){
				alert("<?php echo language("Please choose port");?>");
			}else if($(".strategy_checked").size() == $(".strategy_checked:checked").length){
				$.ajax({
					type:"GET",
					url:"ajax_server.php?type=sms_umlimit_all",
					success:function(data){
						window.location.reload();
					},
					error:function(data){
						console.log(data);
					}
				});
			}else{
				var arr = [];
				$(".strategy_checked").each(function(){
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
					url:"ajax_server.php?type=set_sms_limit",
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

function edit_strategy_switch(){
	global $__GSM_SUM__;
	$lang_sync_title = language('Synchronization option');
	
	$channel = $_GET['sel_gsm'];
	
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw/call_limit/');
	$res = $aql->query("select * from calllimit_settings.conf");
	
	if($res[$channel]['sim_switch_sw']){
		$sim_switch_sw = 'checked';
	}else{
		$sim_switch_sw = '';
	}
	
	if(isset($res[$channel]['sim_policy'])){
		$sim_policy = $res[$channel]['sim_policy'];
	}else{
		$sim_policy = 0;
	}
	
	if(isset($res[$channel]['sim_reg_timeout'])){
		$sim_reg_timeout = $res[$channel]['sim_reg_timeout'];
	}else{
		$sim_reg_timeout = 120;
	}
	
	if(isset($res[$channel]['total_using_time'])){
		$total_using_time = $res[$channel]['total_using_time'];
	}else{
		$total_using_time = 0;
	}
	
	if(isset($res[$channel]['total_callout_time'])){
		$total_callout_time = $res[$channel]['total_callout_time'];
	}else{
		$total_callout_time = 0;
	}
	
	if(isset($res[$channel]['total_callout_count'])){
		$total_callout_count = $res[$channel]['total_callout_count'];
	}else{
		$total_callout_count = 0;
	}
	
	if(isset($res[$channel]['total_sms_count'])){
		$total_sms_count = $res[$channel]['total_sms_count'];
	}else{
		$total_sms_count = 0;
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
			<li class="tbg"><?php echo language('strategy');?> (<?php echo get_gsm_name_by_channel($channel,1,false);?>)</li>
			<li class="tb2">&nbsp;</li>
		</div>
		
		<table width="100%" class="tedit">
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Switch');?>:
						<span class="showhelp">
						<?php echo language('Switch help', 'SIM card strategy switch');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="sim_switch_sw_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="checkbox" name="sim_switch_sw" id="sim_switch_sw" <?php echo $sim_switch_sw;?> />
					<span id="csim_switch_sw"></span>
				</td>
			</tr>
			
			<tbody id="strategy_content">
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Sim Policy');?>:
							<span class="showhelp">
							<?php echo language('Sim Policy help','Search for the next card in the order of SIM card policy, default to Asc representing incremental mode.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="sim_policy_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
						<select name="sim_policy">
							<option value="0" <?php if($sim_policy == '0') echo 'selected'; ?>>Asc</option>
							<option value="1" <?php if($sim_policy == '1') echo 'selected'; ?>>Desc</option>
						</select>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Registration Time');?>:
							<span class="showhelp">
							<?php echo language('Registration Time help','Maximum registration time of SIM card, after unsuccessful registration beyond the set value, the default is 120s.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="sim_reg_timeout_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="text" name="sim_reg_timeout" id="sim_reg_timeout" value="<?php echo $sim_reg_timeout;?>" />
						<?php echo language("Second");?>
						<span id="csim_reg_timeout"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Using Time');?>:
							<span class="showhelp">
							<?php echo language('Using Time help','The maximum usage time of SIM card is calculated from the beginning of switching to the card. <br/>After exceeding the set usage time, the card is cut off. The default 0 means shutdown.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="total_using_time_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
						<div style="float:left;margin-right:10px;">
							<input type="text" name="total_using_time" id="total_using_time" value="<?php echo $total_using_time;?>" />
							<?php echo language('Minute');?>
						</div>
						<span id="ctotal_using_time"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Callout Time');?>:
							<span class="showhelp">
							<?php echo language('Callout Time help','The maximum expiration time of SIM card is calculated after exceeding the set expiration time from switching to the card. <br/>The default 0 is to turn off.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="total_callout_time_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="text" name="total_callout_time" id="total_callout_time" value="<?php echo $total_callout_time;?>" />
						<?php echo language('Minute');?>
						<span id="ctotal_callout_time"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Callout Count');?>:
							<span class="showhelp">
							<?php echo language('Callout Count help','The maximum number of SIM card expiration, from switching to the card after calculating more than the set number of expiration, <br/>cut the card, default 0 means closed.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="total_callout_count_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="text" name="total_callout_count" id="total_callout_count" value="<?php echo $total_callout_count;?>" />
						<span id="ctotal_callout_count"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('SMS Count');?>:
							<span class="showhelp">
							<?php echo language('SMS Count help','The maximum number of SMS messages sent by sim card. From switching to the card,<br/> the number of SMS messages sent by SIM card exceeds the set number of times. The default 0 is to turn off.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="total_sms_count_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="text" name="total_sms_count" id="total_sms_count" value="<?php echo $total_sms_count;?>" />
						<span id="ctotal_sms_count"></span>
					</td>
				</tr>
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
			<tr id="float_btn_tr" class="float_btn_tr" >
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
				<li><a style="color:#CD3278;" href="/cgi-bin/php/strategy-switch.php?sel_gsm=<?php echo $cnum; ?>&send=Modify" ><?php echo get_gsm_name_by_channel($cnum,$bnum);?></a></li>
		<?php
					}else{
		?>
				<li><a style="color:LemonChiffon4;" href="/cgi-bin/php/strategy-switch.php?sel_gsm=<?php echo $cnum; ?>&send=Modify" ><?php echo get_gsm_name_by_channel($cnum,$bnum);?></a></li>
		<?php
					}
				}
			}
		?>
		</div>
	</div>

	<script>
	$(function(){
		$("#sim_switch_sw").iButton();
		
		$(".port").click(function(){
			handle_port_sync();
		});
		
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
		
		//sim_switch_sw
		var sim_switch_sw = document.getElementById('sim_switch_sw').checked;
		if(sim_switch_sw){
			$("#strategy_content").show();
		}else{
			$("#strategy_content").hide();
		}
	});
	
	$("#sim_switch_sw").change(function(){
		if($(this).attr('checked') == 'checked'){
			$("#strategy_content").show();
		}else{
			$("#strategy_content").hide();
		}
	});
	
	function check(){
		if(document.getElementById('sim_switch_sw').checked){
			var sim_reg_timeout = document.getElementById('sim_reg_timeout').value;
			document.getElementById('csim_reg_timeout').innerHTML = '';
			if(isNaN(sim_reg_timeout) || parseInt(sim_reg_timeout) < 120){
				document.getElementById('sim_reg_timeout').focus();
				document.getElementById('csim_reg_timeout').innerHTML = con_str("Must be Number.Range:>=120");
				return false;
			}
			
			var total_using_time = document.getElementById('total_using_time').value;
			document.getElementById('ctotal_using_time').innerHTML = '';
			if(isNaN(total_using_time) || parseInt(total_using_time) < 0){
				document.getElementById('total_using_time').focus();
				document.getElementById('ctotal_using_time').innerHTML = con_str("Must be Number.Range:>=0");
				return false;
			}
			
			var total_callout_time = document.getElementById('total_callout_time').value;
			document.getElementById('ctotal_callout_time').innerHTML = '';
			if(isNaN(total_callout_time) || parseInt(total_callout_time) < 0){
				document.getElementById('total_callout_time').focus();
				document.getElementById('ctotal_callout_time').innerHTML = con_str("Must be Number.Range:>=0");
				return false;
			}
			
			var total_callout_count = document.getElementById('total_callout_count').value;
			document.getElementById('ctotal_callout_count').innerHTML = '';
			if(isNaN(total_callout_count) || parseInt(total_callout_count) < 0){
				document.getElementById('total_callout_count').focus();
				document.getElementById('ctotal_callout_count').innerHTML = con_str("Must be Number.Range:>=0");
				return false;
			}
			
			var total_sms_count = document.getElementById('total_sms_count').value;
			document.getElementById('ctotal_sms_count').innerHTML = '';
			if(isNaN(total_sms_count) || parseInt(total_sms_count) < 0){
				document.getElementById('total_sms_count').focus();
				document.getElementById('ctotal_sms_count').innerHTML = con_str("Must be Number.Range:>=0");
				return false;
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
	</script>
<?php
}

function save_strategy_switch(){
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
	
	if(isset($_POST['sim_switch_sw'])){
		$sim_switch_sw = '1';
	}else{
		$sim_switch_sw = '0';
	}
	if(isset($res[$channel]['sim_switch_sw'])){
		$aql->assign_editkey($channel,'sim_switch_sw',$sim_switch_sw);
	}else{
		$aql->assign_append($channel,'sim_switch_sw',$sim_switch_sw);
	}
	
	$sim_policy = $_POST['sim_policy'];
	if(isset($res[$channel]['sim_policy'])){
		$aql->assign_editkey($channel,'sim_policy',$sim_policy);
	}else{
		$aql->assign_append($channel,'sim_policy',$sim_policy);
	}
	
	$sim_reg_timeout = $_POST['sim_reg_timeout'];
	if(isset($res[$channel]['sim_reg_timeout'])){
		$aql->assign_editkey($channel, 'sim_reg_timeout', $sim_reg_timeout);
	}else{
		$aql->assign_append($channel, 'sim_reg_timeout', $sim_reg_timeout);
	}
	
	$total_using_time = $_POST['total_using_time'];
	if(isset($res[$channel]['total_using_time'])){
		$aql->assign_editkey($channel, 'total_using_time', $total_using_time);
	}else{
		$aql->assign_append($channel, 'total_using_time', $total_using_time);
	}
	
	$total_callout_time = $_POST['total_callout_time'];
	if(isset($res[$channel]['total_callout_time'])){
		$aql->assign_editkey($channel, 'total_callout_time', $total_callout_time);
	}else{
		$aql->assign_append($channel, 'total_callout_time', $total_callout_time);
	}
	
	$total_callout_count = $_POST['total_callout_count'];
	if(isset($res[$channel]['total_callout_count'])){
		$aql->assign_editkey($channel, 'total_callout_count', $total_callout_count);
	}else{
		$aql->assign_append($channel, 'total_callout_count', $total_callout_count);
	}
	
	$total_sms_count = $_POST['total_sms_count'];
	if(isset($res[$channel]['total_sms_count'])){
		$aql->assign_editkey($channel, 'total_sms_count', $total_sms_count);
	}else{
		$aql->assign_append($channel, 'total_sms_count', $total_sms_count);
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
				if($key == 'call_limit_switch'){
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
			
			//sim_switch_sw
			if(isset($_POST['sim_switch_sw_sync'])){
				if(isset($sel_res[$port]['sim_switch_sw'])){
					$aql->assign_editkey($port,'sim_switch_sw',$sim_switch_sw);
				}else{
					$aql->assign_append($port,'sim_switch_sw',$sim_switch_sw);
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
	
	wait_apply("exec","/my_tools/set_calllimit.sh reload");
}
?>

<?php
if($_POST){
	if(isset($_POST['send']) && $_POST['send'] == 'Save'){
		save_strategy_switch();
		show_strategy_switch();
	}
}else if($_GET){
	if(isset($_GET['send']) && $_GET['send'] == 'Modify'){
		edit_strategy_switch();
	}
}else{
	show_strategy_switch();
}

require("/www/cgi-bin/inc/boot.inc");
?>
<div id="float_btn1" class="sec_float_btn1"></div>
<div class="float_close" onclick="close_btn()" ></div>