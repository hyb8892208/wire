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
function show_strategy_lock(){
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
			<button id="batch_unlock" type="button"><?php echo language('Batch Unlock'); ?></button>
			<button id="batch_unmark" type="button"><?php echo language('Batch Unmark'); ?></button>
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
				<img src="/images/call-lock.png" />
				<span><?php echo language('Locked');?></span>
			</div>
			<div class="status_help_each" >
				<img src="/images/marker.png" />
				<span><?php echo language('Marker');?></span>
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
				<td><input type="checkbox" class="lock_checked" name="port[]" value="<?php echo $key;?>" /></td>
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
					
					//lock status
					if($val[$k]['call_fail_lock_status'] == 1){
						echo '<img src="/images/call-lock.png" style="cursor:pointer" class="lock_sta" alt="'.language("Lock").'" title="'.language("Lock").'" />';
					}else{
						//marker status
						if($val[$k]['call_fail_mark_status'] == 1){
							echo '<img src="/images/marker.png" style="cursor:pointer;" class="marker_sta" alt="'.language("Marker").'" title="'.language("Marker").'" />';
						}
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
	
	//batch_unlock
	$("#batch_unlock").click(function(){
		var ret = confirm("<?php echo language('Unlock Tip','Are you sure you want to unlock it?');?>");
		
		if(ret){
			if($(".lock_checked:checked").length == 0){
				alert("<?php echo language("Please choose port");?>");
			}else if($(".lock_checked").size() == $(".lock_checked:checked").length){
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
				$(".lock_checked").each(function(){
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
			if($(".lock_checked:checked").length == 0){
				alert("<?php echo language("Please choose port");?>");
			}else if($(".lock_checked").size() == $(".lock_checked:checked").length){
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
				$(".lock_checked").each(function(){
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
	</script>

<?php 
} 

function edit_strategy_lock(){
	global $__GSM_SUM__;
	$lang_sync_title = language('Synchronization option');
	
	$channel = $_GET['sel_gsm'];
	
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw/call_limit/');
	$res = $aql->query("select * from calllimit_settings.conf");
	
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
			<li class="tbg"><?php echo language('Lock Sim');?></li>
			<li class="tb2">&nbsp;</li>
		</div>
		
		<table width="100%" class="tedit">
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Lock Detect Switch');?>:
						<span class="showhelp">
						<?php echo language('Lock Detect Switch help');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="call_detect_flag_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="checkbox" name="call_detect_flag" id="call_detect_flag" <?php echo $call_detect_flag; ?> />
					<span id="ccall_detect_flag"></span>
				</td>
			</tr>
			
			<tbody class="lock_sim_tr">
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Mark Switch');?>:
							<span class="showhelp">
							<?php echo language('Mark Switch help');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="call_fail_mark_flag_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
						<input type="checkbox" name="call_fail_mark_flag" id="call_fail_mark_flag" <?php echo $call_fail_mark_flag;?> />
						<span id="ccall_fail_mark_flag"></span>
					</td>
				</tr>
				
				<tr class="mark_count_tr">
					<th>
						<div class="helptooltips">
							<?php echo language('Call Failed Mark Count');?>:
							<span class="showhelp">
							<?php echo language('Call Failed Mark Count help','The number of consecutive calls failed to reach the number of settings, marking the port.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="call_fail_mark_count_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
						<input type="text" name="call_fail_mark_count" id="call_fail_mark_count" value="<?php echo $call_fail_mark_count;?>" />
						<span id="ccall_fail_mark_count"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Call Failed Lock Switch');?>:
							<span class="showhelp">
							<?php echo language('Call Failed Lock Switch help');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="call_fail_lock_flag_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
						<input type="checkbox" name="call_fail_lock_flag" id="call_fail_lock_flag" <?php echo $call_fail_lock_flag;?> />
						<span id="ccall_fail_lock_flag"></span>
					</td>
				</tr>
				
				<tr class="lock_count_tr">
					<th>
						<div class="helptooltips">
							<?php echo language('Call Failed Lock Count');?>:
							<span class="showhelp">
							<?php echo language('Call Failed Lock Count help','Lock the port after the number of successive call failures reaches the set number, restrict the port expiration, and no longer select the port expiration in the group policy.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="call_fail_lock_count_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
						<input type="text" name="call_fail_lock_count" id="call_fail_lock_count" value="<?php echo $call_fail_lock_count;?>">
						<span id="ccall_fail_lock_count"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('SMS Send Detection Switch');?>:
							<span class="showhelp">
							<?php echo language('SMS Send Detection Switch help','After opening, when the number of successive call failures reaches the set value, send short message to check whether the port is available; if the short message is sent successfully, clear the number of successive call failures; if the short message fails, limit the port\'s outgoing.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="call_fail_lock_sms_flag_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
						<input type="checkbox" name="call_fail_lock_sms_flag" id="call_fail_lock_sms_flag" <?php echo $call_fail_lock_sms_flag;?> />
						<span id="ccall_fail_lock_sms_flag"></span>
					</td>
				</tr>
				
				<tr class="sms_switch_tr">
					<th>
						<div class="helptooltips">
							<?php echo language('SMS Send Detection Count');?>:
							<span class="showhelp">
							<?php echo language('SMS Send Detection Count help');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="call_fail_lock_sms_count_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
						<input type="text" name="call_fail_lock_sms_count" id="call_fail_lock_sms_count" value="<?php echo $call_fail_lock_sms_count;?>" />
						<span id="ccall_fail_lock_sms_count"></span>
					</td>
				</tr>
				
				<tr class="sms_switch_tr">
					<th>
						<div class="helptooltips">
							<?php echo language('Send Sms Number');?>:
							<span class="showhelp">
							<?php echo language('Send Sms Number');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="call_fail_lock_sms_callee_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
						<input type="text" name="call_fail_lock_sms_callee" id="call_fail_lock_sms_callee" value="<?php echo $call_fail_lock_sms_callee;?>" />
						<span id="ccall_fail_lock_sms_callee"></span>
					</td>
				</tr>
				
				<tr class="sms_switch_tr">
					<th>
						<div class="helptooltips">
							<?php echo language('Sms Message');?>:
							<span class="showhelp">
							<?php echo language('Sms Message help');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="call_fail_lock_sms_msg_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
						<input type="text" name="call_fail_lock_sms_msg" id="call_fail_lock_sms_msg" value="<?php echo $call_fail_lock_sms_msg;?>" />
						<span id="ccall_fail_lock_sms_msg"></span>
					</td>
				</tr>
				
				<tr class="sms_switch_tr">
					<th>
						<div class="helptooltips">
							<?php echo language('Testing SMS report');?>:
							<span class="showhelp">
							<?php echo language('Testing SMS report help', 'When closed, the successful sending of short message indicates that the port is available; when opened, the successful sending of short message and the receipt of short message report indicate that the port is available.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" name="call_fail_lock_sms_report_flag_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/>
						<input type="checkbox" name="call_fail_lock_sms_report_flag" id="call_fail_lock_sms_report_flag" <?php echo $call_fail_lock_sms_report_flag;?> />
						<span id="ccall_fail_lock_sms_report_flag"></span>
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
				<li><a style="color:#CD3278;" href="/cgi-bin/php/strategy-lock.php?sel_gsm=<?php echo $cnum; ?>&send=Modify" ><?php echo get_gsm_name_by_channel($cnum,$bnum);?></a></li>
		<?php
					}else{
		?>
				<li><a style="color:LemonChiffon4;" href="/cgi-bin/php/strategy-lock.php?sel_gsm=<?php echo $cnum; ?>&send=Modify" ><?php echo get_gsm_name_by_channel($cnum,$bnum);?></a></li>
		<?php
					}
				}
			}
		?>
		</div>
	</div>

	<script>
	$(function(){
		$("#call_detect_flag").iButton();
		$("#call_fail_mark_flag").iButton();
		$("#call_fail_lock_flag").iButton();
		$("#call_fail_lock_sms_flag").iButton();
		$("#call_fail_lock_sms_report_flag").iButton();
		
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
		
		//show and hide
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
	});
	
	function check(){
		//lock 
		if(document.getElementById('call_detect_flag').checked){
			if(document.getElementById('call_fail_mark_flag').checked){
				var call_fail_mark_count = document.getElementById('call_fail_mark_count').value;
				document.getElementById('ccall_fail_mark_count').innerHTML = '';
				if(isNaN(call_fail_mark_count) || parseInt(call_fail_mark_count) < 0){
					document.getElementById('call_fail_mark_count').focus();
					document.getElementById('ccall_fail_mark_count').innerHTML = con_str("Must be Number.Range:>=0");
					return false;
				}
			}
			
			if(document.getElementById('call_fail_lock_flag').checked){
				var call_fail_lock_count = document.getElementById('call_fail_lock_count').value;
				document.getElementById('ccall_fail_lock_count').innerHTML = '';
				if(isNaN(call_fail_lock_count) || parseInt(call_fail_lock_count) < 0){
					document.getElementById('call_fail_lock_count').focus();
					document.getElementById('ccall_fail_lock_count').innerHTML = con_str("Must be Number.Range:>=0");
					return false;
				}
			}
			
			if(document.getElementById('call_fail_lock_sms_flag').checked){
				var call_fail_lock_sms_count = document.getElementById('call_fail_lock_sms_count').value;
				document.getElementById('ccall_fail_lock_sms_count').innerHTML = '';
				if(isNaN(call_fail_lock_sms_count) || parseInt(call_fail_lock_sms_count) < 0){
					document.getElementById('call_fail_lock_sms_count').focus();
					document.getElementById('ccall_fail_lock_sms_count').innerHTML = con_str("Must be Number.Range:>=0");
					return false;
				}
			}
			
			if((document.getElementById('call_fail_mark_flag').checked) && (document.getElementById('call_fail_lock_flag').checked)){
				document.getElementById('ccall_fail_mark_count').innerHTML = '';
				if(parseInt(call_fail_mark_count) >= parseInt(call_fail_lock_count)){
					document.getElementById('call_fail_mark_count').focus();
					document.getElementById('ccall_fail_mark_count').innerHTML = con_str('<?php echo language("Mark Count and Lock Count","Call Failed Mark Count must be less than Call Failed Lock Count.");?>');
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
	</script>
<?php 
}

function save_strategy_lock(){
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
				if($key == 'call_detect_flag' || $key == 'call_fail_mark_flag' || $key == 'call_fail_lock_flag' || $key == 'call_fail_lock_sms_flag' || $key == 'call_fail_lock_sms_report_flag'){
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
		}
	}
	
	if(!$aql->save_config_file('calllimit_settings.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	
	if(!isset($_POST['call_detect_flag']) || !isset($_POST['call_fail_lock_flag'])){
		exec("/my_tools/set_calllimit.sh unlock $channel");
	}
	
	if(!isset($_POST['call_fail_mark_flag'])){
		exec("/my_tools/set_calllimit.sh unmark $channel");
	}
	
	wait_apply("exec","/my_tools/set_calllimit.sh reload");
}

if($_POST){
	if(isset($_POST['send']) && $_POST['send'] == 'Save'){
		save_strategy_lock();
		show_strategy_lock();
	}
}else if($_GET){
	if(isset($_GET['send']) && $_GET['send'] == 'Modify'){
		edit_strategy_lock();
	}
}else{
	show_strategy_lock();
}

require("/www/cgi-bin/inc/boot.inc");
?>
<div id="float_btn1" class="sec_float_btn1"></div>
<div class="float_close" onclick="close_btn()" ></div>