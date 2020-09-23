<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");
?>

<?php 
function show_balance(){
	global $__GSM_SUM__;
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$res = $aql->query("select * from sim_query.conf");
?>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="get">
		<table width="100%" class="tshow">
			<tr>
				<th><?php echo language('Port');?></th>
				<th width="100px"><?php echo language('Query Type');?></th>
				<th width="180px"><?php echo language('Destination Number@balance');?></th>
				<th width="180px"><?php echo language('Receive Number');?></th>
				<th width="250px"><?php echo language('Send Message');?></th>
				<th><?php echo language('Matching Key');?></th>
				<th><?php echo language('Balance@balance');?></th>
				<th width="50px"><?php echo language('Actions');?></th>
			</tr>
<?php
		for($c=1; $c<=$__GSM_SUM__; $c++){
			$channel_name = get_gsm_name_by_channel($c);
			if(strstr($channel_name,'null')) continue;
			
			$bal_query_type = '';
			if(isset($res[$c]['query_type'])){
				$temp = $res[$c]['query_type'];
				$bal_query_type = $temp&15;
			}
			$query_type = language('Inactive');
			if($bal_query_type==5){
				$query_type = language('SMS');
			}else if($bal_query_type==6){
				$query_type = language('Tel');
			}else if($bal_query_type==7){
				$query_type = language('USSD');
			}
			
			$bal_dst_num = '';
			if(isset($res[$c]['bal_dst_num'])){
				$bal_dst_num = $res[$c]['bal_dst_num'];
			}
			
			$bal_recv_num = '';
			if(isset($res[$c]['bal_recv_num'])){
				$bal_recv_num = $res[$c]['bal_recv_num'];
			}
			
			$bal_send_msg = '';
			if(isset($res[$c]['bal_send_msg'])){
				$bal_send_msg = $res[$c]['bal_send_msg'];
			}
			
			$bal_match_key = '';
			if(isset($res[$c]['bal_match_key'])){
				$bal_match_key = $res[$c]['bal_match_key'];
			}
			
			exec("/my_tools/redis-cli hget app.simquery.balance.channel $c",$output);
			$balance = $output[0];
			$output = '';
?>
			<tr>
				<td><?php echo $channel_name;?></td>
				<td><?php echo $query_type;?></td>
				<td><?php echo $bal_dst_num;?></td>
				<td><?php echo $bal_recv_num;?></td>
				<td><?php echo $bal_send_msg;?></td>
				<td><?php echo $bal_match_key;?></td>
				<td><?php echo $balance;?></td>
				<td style="text-align:center;">
					<button type="submit" value="Modify" style="width:32px;height:32px;" onclick="document.getElementById('send').value='Edit';document.getElementById('channel_sel').value='<?php echo $c;?>'">
						<img src="/images/edit.gif">
					</button>
				</td>
			</tr>
<?php
		}
?>
			<input type="hidden" name="send" id="send" value="" />
			<input type="hidden" name="channel_sel" id="channel_sel" value="" />
		</table>
	</form>
<?php
}
?>

<?php 
function edit_balance(){
	global $__GSM_SUM__;
	
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$channel = $_GET['channel_sel'];
	$res = $aql->query("select * from sim_query.conf where section ='$channel'");
	
	$lang_sync_title = language('Synchronization option');
	
	$query_type = 0;
	if(isset($res[$channel]['query_type'])){
		$temp = $res[$channel]['query_type'];
		$query_type = $temp&15;
	}
	
	$query_switch = 'checked';
	if($query_type == 0){
		$query_switch = '';
	}
	
	$bal_dst_num = '';
	if(isset($res[$channel]['bal_dst_num'])){
		$bal_dst_num = $res[$channel]['bal_dst_num'];
	}
	
	$bal_recv_num = '';
	if(isset($res[$channel]['bal_recv_num'])){
		$bal_recv_num = $res[$channel]['bal_recv_num'];
	}
	
	$bal_send_msg = '';
	if(isset($res[$channel]['bal_send_msg'])){
		$bal_send_msg = $res[$channel]['bal_send_msg'];
	}
	
	$bal_match_key = '';
	if(isset($res[$channel]['bal_match_key'])){
		$bal_match_key = $res[$channel]['bal_match_key'];
	}
	
	$interval = '0';
	if(isset($res[$channel]['interval'])){
		$interval = $res[$channel]['interval'];
	}
	
	$point_char = '.';
	if(isset($res[$channel]['point_char'])){
		$point_char = $res[$channel]['point_char'];
	}
	
	$thousandth_char = ',';
	if(isset($res[$channel]['thousandth_char'])){
		$thousandth_char = $res[$channel]['thousandth_char'];
	}
	
	$registered_query = '';
	if(isset($res[$channel]['registered_query']) && $res[$channel]['registered_query'] == 1){
		$registered_query = 'checked';
	}
	
	$call_counts_query = '0';
	if(isset($res[$channel]['call_counts_query'])){
		$call_counts_query = $res[$channel]['call_counts_query'];
	}
?>
	<script type="text/javascript" src="/js/check.js?v=1.1"></script>
	<script type="text/javascript" src="/js/functions.js"></script>
	<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />
	<script type="text/javascript" src="/js/jquery.ibutton.js"></script>
	
	<form id="manform" enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
		<input type="hidden" name="channel" value="<?php echo $channel;?>" />
		<div id="tab">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('Port');?> <?php echo get_gsm_name_by_channel($channel);?></li>
			<li class="tb2">&nbsp;</li>
		</div>

		<table width="100%" class="tedit" >
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Query Swith');?>:
						<span class="showhelp">
						<?php
							echo language('Query Swith help@balance','After opening, the balance can be queried.');
						?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" class="setting_sync" name="query_switch_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="checkbox" name="query_switch" id="query_switch" <?php echo $query_switch; ?> />
					<span id="cquery_switch"></span>
				</td>
			</tr>
		
			<tr class="sw_show">
				<th>
					<div class="helptooltips">
						<?php echo language('Query Type');?>:
						<span class="showhelp">
						<?php
							echo language('Query Type help','If the channel is in call state, the query will fail.');
						?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" class="setting_sync" name="query_type_sync" title="<?php echo $lang_sync_title;?>" />
					<select name="query_type" id="query_type">
						<option value="5" <?php if($query_type==5)echo 'selected';?>><?php echo language('SMS');?></option>
						<option value="6" <?php if($query_type==6)echo 'selected';?>><?php echo language('Tel');?></option>
						<option value="7" <?php if($query_type==7)echo 'selected';?>><?php echo language('USSD');?></option>
					</select>
					<span id="cquery_type"></span>
				</td>
			</tr>
			
			<tr id="show_distination" class="sw_show">
				<th>
					<div class="helptooltips">
						<?php echo language('Destination Number@balance');?>:
						<span class="showhelp">
						<?php
							echo language('Destination Number help@balance','Number used by operator to receive inquiry information.');
						?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" class="setting_sync" name="bal_dst_num_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="bal_dst_num" id="bal_dst_num" value="<?php echo $bal_dst_num; ?>" />
					<span id="cbal_dst_num"></span>
				</td>
			</tr>
			
			<tr class="sw_show">
				<th>
					<div class="helptooltips">
						<?php echo language('Receive Number');?>:
						<span class="showhelp">
						<?php
							echo language('Receive Number help','Number used by the operator to send the queried content.');
						?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" class="setting_sync" name="bal_recv_num_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="bal_recv_num" id="bal_recv_num" value="<?php echo $bal_recv_num; ?>" />
					<span id="cbal_recv_num"></span>
				</td>
			</tr>
			
			<tr id="show_send_message" class="sw_show">
				<th>
					<div class="helptooltips">
						<?php echo language('Send Message');?>:
						<span class="showhelp">
						<?php
							echo language('Send Message help','Short message content sent to operators for queries.');
						?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" class="setting_sync" name="bal_send_msg_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="bal_send_msg" id="bal_send_msg" value="<?php echo $bal_send_msg; ?>" />
					<span id="cbal_send_msg"></span>
				</td>
			</tr>
			
			<tr class="sw_show">
				<th>
					<div class="helptooltips">
						<?php echo language('Matching Key');?>:
						<span class="showhelp">
						<?php
							echo language('Matching Key help','The first 10 words in the contents of the content returned by the operator are not more than 30 words.');
						?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" class="setting_sync" name="bal_match_key_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="bal_match_key" id="bal_match_key" value="<?php echo $bal_match_key; ?>" />
					<span id="cbal_match_key"></span>
				</td>
			</tr>
			
			<tr class="sw_show">
				<th>
					<div class="helptooltips">
						<?php echo language('Registered Query');?>:
						<span class="showhelp">
						<?php
							echo language('Registered Query help','SIM Card Registration Successful Automatic Query Once.');
						?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" class="setting_sync" name="registered_query_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="checkbox" name="registered_query" id="registered_query" <?php echo $registered_query; ?> />
					<span id="cregistered_query"></span>
				</td>
			</tr>
			
			<tr class="sw_show">
				<th>
					<div class="helptooltips">
						<?php echo language('Interval@balance');?>:
						<span class="showhelp">
						<?php
							echo language('Interval help','Interval range:0-30000.When the value is 0, it is closed.');
						?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" class="setting_sync" name="interval_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="interval" id="interval" value="<?php echo $interval; ?>" /><?php echo language('Minute');?>
					<span id="cinterval"></span>
				</td>
			</tr>
			
			<tr class="sw_show">
				<th>
					<div class="helptooltips">
						<?php echo language('Call Count Query');?>:
						<span class="showhelp">
						<?php
							echo language('Call Count Query help','Call Count Query range:0-100.When the value is 0, it is closed.');
						?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" class="setting_sync" name="call_counts_query_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="call_counts_query" id="call_counts_query" value="<?php echo $call_counts_query; ?>" />
					<span id="ccall_counts_query"></span>
				</td>
			</tr>
			
			<tr class="sw_show">
				<th>
					<div class="helptooltips">
						<?php echo language('Decimal mark');?>:
						<span class="showhelp">
						<?php
							echo language('Decimal mark help','Operators reply to decimal point symbols of numerical content');
						?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" class="setting_sync" name="point_char_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="point_char" id="point_char" value="<?php echo $point_char; ?>" />
					<span id="cpoint_char"></span>
				</td>
			</tr>
			
			<tr class="sw_show">
				<th>
					<div class="helptooltips">
						<?php echo language('Kilo mark');?>:
						<span class="showhelp">
						<?php
							echo language('Kilo mark help','Operator responds to the microbit symbol of numeric content');
						?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" class="setting_sync" name="thousandth_char_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="thousandth_char" id="thousandth_char" value="<?php echo $thousandth_char; ?>" />
					<span id="cthousandth_char"></span>
				</td>
			</tr>
		</table>
		
		<br/>
		
		<div id="tab">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('The matching test');?></li>
			<li class="tb2">&nbsp;</li>
		</div>

		<table width="100%" class="tedit" >
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Message content');?>:
						<span class="showhelp">
						<?php
							echo language('Message content help', 'Message content');
						?>
						</span>
					</div>
				</th>
				<td>
					<textarea id="sms_content" style="height:120px;width:500px;"></textarea>
					<span id="csms_content"></span>
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Matching results');?>:
						<span class="showhelp">
						<?php
							echo language('Matching results help', 'Matching results');
						?>
						</span>
					</div>
				</th>
				<td>
					<button id="match_test" type="button"><?php echo language('Test');?></button>
					<span id="callback" style="margin-left:20px;"></span>
				</td>
			</tr>
		</table>
		
		<br>

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
							<?php echo language('PhoneNumber Save To Other Ports help');?>
							</span>
						</div>
					</th>
					<td>
						<table cellpadding="0" cellspacing="0" class="port_table">
<?php
							for($i=1;$i<=$__GSM_SUM__;$i++){
								$port_name = get_gsm_name_by_channel($i);
								if(strstr($port_name, 'null')) continue;
								if($i==$channel){
									$checked = 'checked';
									$disabled = 'disabled';
								}else{
									$checked = '';
									$disabled = '';
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
							<?php echo language('PhoneNumber Sync All Settings help');?>
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
					<input type="submit"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>
				</td>
				<td>
					<input type="button" value="<?php echo language('Cancel');?>" onclick="window.location.href='<?php echo get_self();?>'" />
				</td>
			</tr>
		</table>
	</form>
	
	<div id="sort_out" class="sort_out"></div>
	<div class="sort_gsm_cli" id="sort_gsm_cli">
		<div id="sort_info" class="sort_info" style="display:block">
		<?php 
		for($c=1; $c<=$__GSM_SUM__; $c++){
			$channel_name = get_gsm_name_by_channel($c);
			if(strstr($channel_name,'null')) continue;
		?>
			<li>
				<a style="<?php if($c==$channel){echo 'color:#CD3278;';}else{echo 'color:LemonChiffon4;';}?>" href="/cgi-bin/php/adv-balance.php?channel_sel=<?php echo $c; ?>&send=Edit" >
					<?php echo $channel_name;?>
				</a>
			</li>
		<?php 
		}
		?>
		</div>
	</div>
	
	<script>
	$(document).ready(function (){
		float_sort_hide();
		var sort_info_top = $("#lps").offset().top;
		$("#sort_gsm_cli").offset({top: sort_info_top });
		$("#sort_out").offset({top: sort_info_top });
		$("#sort_out").mouseover(function(){
			if($("#sort_out").offset().left <= 5){
		   		float_sort_on();
			}
		});
		$("#sort_gsm_cli").mouseleave(function(){
			float_sort_hide();
		});
		
		$(".port").click(function(){
			handle_port_sync();
		});

		$(".setting_sync").click(function(){
			handle_setting_sync();
		});
	});
	function float_sort_hide(){
		$("#sort_gsm_cli").stop().animate({left:"-198px"}, 300);
		$("#sort_out").stop().animate({left:"0px"}, 300);
	};
	function float_sort_on(){
		$("#sort_gsm_cli").animate({left:"0px"}, 300);
		$("#sort_out").animate({left:"198px"}, 300);
	};
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
	function handle_setting_sync(){
		if(isAllCheckboxChecked('class','setting_sync')){
			$("#all_settings_sync").attr({"checked":true});
		}else{
			$("#all_settings_sync").attr({"checked":false});
		}
	}
	function check(){
		var checked = document.getElementById('query_switch').checked;
		var bal_dst_num = document.getElementById('bal_dst_num').value;
		var bal_recv_num = document.getElementById('bal_recv_num').value;
		var bal_send_msg = document.getElementById('bal_send_msg').value;
		var bal_match_key = document.getElementById('bal_match_key').value;
		var interval = document.getElementById('interval').value;
		var call_counts_query = document.getElementById('call_counts_query').value;
		var query_type = document.getElementById('query_type').value;
		
		if(checked){
			if(query_type != '7'){
				if(bal_dst_num == ''){
					document.getElementById('cbal_dst_num').innerHTML = con_str("<?php echo language('js check null','can not be null!');?>");
					document.getElementById('bal_dst_num').focus();
					return false;
				}else if(!check_phonenum(bal_dst_num)){
					document.getElementById('cbal_dst_num').innerHTML = con_str("<?php echo language('js check phonenum','Please input a valid phone number!');?>");
					document.getElementById('bal_dst_num').focus();
					return false
				}else{
					document.getElementById('cbal_dst_num').innerHTML = '';
				}
			}
			
			document.getElementById('cbal_recv_num').innerHTML = '';
			if(query_type != '7'){
				if(bal_recv_num == ''){
					document.getElementById('cbal_recv_num').innerHTML = con_str("<?php echo language('js check null','can not be null!');?>");
					document.getElementById('bal_recv_num').focus();
					return false;
				}else if(!check_phonenum(bal_recv_num)){
					// document.getElementById('cbal_recv_num').innerHTML = con_str("<?php echo language('js check phonenum','Please input a valid phone number!');?>");
					// document.getElementById('bal_recv_num').focus();
					// return false;
				}else{
					document.getElementById('cbal_recv_num').innerHTML = '';
				}
			}
			
			if(query_type != '6'){
				if(bal_send_msg == ''){
					document.getElementById('cbal_send_msg').innerHTML = con_str("<?php echo language('js check null','can not be null!');?>");
					document.getElementById('bal_send_msg').focus();
					return false;
				}else if(!check_send_msg(bal_send_msg)){
					document.getElementById('cbal_send_msg').innerHTML = con_str("<?php echo language('js check send_msg', 'Allowed character must be 1-128 characters.');?>");
					document.getElementById('bal_send_msg').focus();
					return false;
				}else{
					document.getElementById('cbal_send_msg').innerHTML = '';
				}
			}
			
			if(bal_match_key == ''){
				document.getElementById('cbal_match_key').innerHTML = con_str("<?php echo language('js check null','can not be null!');?>");
				document.getElementById('bal_match_key').focus();
				return false;
			}else if(!check_match_key(bal_match_key)){
				document.getElementById('cbal_match_key').innerHTML = con_str("<?php echo language('js check match_key', 'Allowed character must be 1-32 characters.');?>");
				document.getElementById('bal_match_key').focus();
				return false;
			}else{
				document.getElementById('cbal_match_key').innerHTML = '';
			}
			
			if(interval == ''){
				document.getElementById('cinterval').innerHTML = con_str("<?php echo language('js check null','can not be null!');?>");
				document.getElementById('interval').focus();
				return false;
			}else if(!check_interval(interval)){
				document.getElementById('cinterval').innerHTML = con_str("<?php echo language('js interval', 'Interval range:0-30000');?>");
				document.getElementById('interval').focus();
				return false;
			}else{
				document.getElementById('cinterval').innerHTML = '';
			}
			
			if(call_counts_query == ''){
				document.getElementById('ccall_counts_query').innerHTML = con_str("<?php echo language('js check null','can not be null!');?>");
				document.getElementById('call_counts_query').focus();
				return false;
			}else if(!check_call_counts_query(call_counts_query)){
				document.getElementById('ccall_counts_query').innerHTML = con_str("<?php echo language('js call count query','Call Count Query range:0-100');?>");
				document.getElementById('call_counts_query').focus();
				return false;
			}else{
				document.getElementById('ccall_counts_query').innerHTML = '';
			}
		}
	}
	
	$("#query_type").change(function(){
		if($(this).val()=='5'){
			$("#show_distination").show();
			$("#show_send_message").show();
		}else if($(this).val()=='6'){
			$("#show_distination").show();
			$("#show_send_message").hide();
		}else if($(this).val()=='7'){
			$("#show_distination").hide();
			$("#show_send_message").show();
		}
	});
	$("#query_switch").change(function(){
		var sw = $(this).attr('checked');
		if(sw == 'checked'){
			$(".sw_show").show();
		}else{
			$(".sw_show").hide();
		}
	});

	$(function(){
		$("#registered_query").iButton();
		$("#query_switch").iButton();
		
		var sw = $("#query_switch").attr('checked');
		if(sw == 'checked'){
			$(".sw_show").show();
			
			if($("#query_type").val()=='5'){
				$("#show_distination").show();
				$("#show_send_message").show();
			}else if($("#query_type").val()=='6'){
				$("#show_distination").show();
				$("#show_send_message").hide();
			}else if($("#query_type").val()=='7'){
				$("#show_distination").hide();
				$("#show_send_message").show();
			}
		}else{
			$(".sw_show").hide();
		}
		
		
	});
	
	//match test
	$("#match_test").click(function(){
		var query_type = $("#query_type").val();
		var sms_content = $("#sms_content").val();
		var match_key_info = $("#bal_match_key").val();
		var point_char = $("#point_char").val();
		var thousandth_char = $("#thousandth_char").val();
		
		if(match_key_info == ''){
			document.getElementById('cbal_match_key').innerHTML = con_str("<?php echo language('js check null','can not be null!');?>");
			document.getElementById('bal_match_key').focus();
			return false;
		}else if(!check_match_key(match_key_info)){
			document.getElementById('cbal_match_key').innerHTML = con_str("<?php echo language('js check match_key', 'Allowed character must be 1-32 characters.');?>");
			document.getElementById('bal_match_key').focus();
			return false;
		}else{
			document.getElementById('cbal_match_key').innerHTML = '';
		}
		
		if(sms_content == ''){
			document.getElementById('csms_content').innerHTML = con_str("<?php echo language('js check null','can not be null!');?>");
			document.getElementById('sms_content').focus();
			return false;
		}else{
			document.getElementById('csms_content').innerHTML = '';
		}
		
		$.ajax({
			type:"POST",
			url: "ajax_server.php?type=match_test&balance=",
			data: {
				'query_type':query_type,
				'sms_content':sms_content,
				'match_key_info':match_key_info,
				'point_char':point_char,
				'thousandth_char':thousandth_char
			},
			success: function(res){
				if(res.length != 0){
					$("#callback").html(res);
				}else{
					$("#callback").html("Failed");
				}
			},
			error: function(res){
				$("#callback").html("Failed");
			}
		});
	});
	</script>
<?php 
}

function save_balance(){
	global $__GSM_SUM__;
	
	$channel = $_POST['channel'];
	
	$bal_query_type = 0;
	if(isset($_POST['query_switch']) && $_POST['query_switch'] == 'on'){
		if(isset($_POST['query_type'])){
			$bal_query_type = trim($_POST['query_type']);
		}
	}
	
	$bal_dst_num = '';
	if(isset($_POST['bal_dst_num'])){
		$bal_dst_num = trim($_POST['bal_dst_num']);
	}
	
	$bal_recv_num = '';
	if(isset($_POST['bal_recv_num'])){
		$bal_recv_num = trim($_POST['bal_recv_num']);
	}
	
	$bal_send_msg = '';
	if(isset($_POST['bal_send_msg'])){
		$bal_send_msg = trim($_POST['bal_send_msg']);
	}
	
	$bal_match_key = '';
	if(isset($_POST['bal_match_key'])){
		$bal_match_key = trim($_POST['bal_match_key']);
	}
	
	$interval = '';
	if(isset($_POST['interval'])){
		$interval = trim($_POST['interval']);
	}
	
	$point_char = '';
	if(isset($_POST['point_char'])){
		$point_char = trim($_POST['point_char']);
	}
	
	$thousandth_char = '';
	if(isset($_POST['thousandth_char'])){
		$thousandth_char = trim($_POST['thousandth_char']);
	}
	
	$registered_query = 0;
	if(isset($_POST['registered_query'])){
		$registered_query = 1;
	}
	
	$call_counts_query = '';
	if(isset($_POST['call_counts_query'])){
		$call_counts_query = trim($_POST['call_counts_query']);
	}
	
	$aql = new aql();
	$conf_path = '/etc/asterisk/gw/sim_query.conf';
	$hlock = lock_file($conf_path);
	if(!file_exists($conf_path)) {exec('touch /etc/asterisk/gw/sim_query.conf');}
	$aql->set('basedir','/etc/asterisk/gw');
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	$exist_array = $aql->query("select * from sim_query.conf");
	
	if(!isset($exist_array[$channel])){
		$aql->assign_addsection($channel,'');
	}
	
	if(isset($exist_array[$channel]['query_type'])){
		$num_query_type = $exist_array[$channel]['query_type']&240;
		$query_type = $bal_query_type|$num_query_type;
		$aql->assign_editkey($channel,'query_type',$query_type);
	}else{
		$aql->assign_append($channel,'query_type',$bal_query_type);
	}
	
	if(isset($exist_array[$channel]['bal_dst_num'])){
		$aql->assign_editkey($channel,'bal_dst_num',$bal_dst_num);
	}else{
		$aql->assign_append($channel,'bal_dst_num',$bal_dst_num);
	}
	
	if(isset($exist_array[$channel]['bal_recv_num'])){
		$aql->assign_editkey($channel,'bal_recv_num',$bal_recv_num);
	}else{
		$aql->assign_append($channel,'bal_recv_num',$bal_recv_num);
	}
	
	if(isset($exist_array[$channel]['bal_send_msg'])){
		$aql->assign_editkey($channel,'bal_send_msg',$bal_send_msg);
	}else{
		$aql->assign_append($channel,'bal_send_msg',$bal_send_msg);
	}
	
	if(isset($exist_array[$channel]['bal_match_key'])){
		$aql->assign_editkey($channel,'bal_match_key',$bal_match_key);
	}else{
		$aql->assign_append($channel,'bal_match_key',$bal_match_key);
	}
	
	if(isset($exist_array[$channel]['interval'])){
		$aql->assign_editkey($channel,'interval',$interval);
	}else{
		$aql->assign_append($channel,'interval',$interval);
	}
	
	if(isset($exist_array[$channel]['point_char'])){
		$aql->assign_editkey($channel,'point_char',$point_char);
	}else{
		$aql->assign_append($channel,'point_char',$point_char);
	}
	
	if(isset($exist_array[$channel]['thousandth_char'])){
		$aql->assign_editkey($channel,'thousandth_char',$thousandth_char);
	}else{
		$aql->assign_append($channel,'thousandth_char',$thousandth_char);
	}
	
	if(isset($exist_array[$channel]['registered_query'])){
		$aql->assign_editkey($channel,'registered_query',$registered_query);
	}else{
		$aql->assign_append($channel,'registered_query',$registered_query);
	}
	
	if(isset($exist_array[$channel]['call_counts_query'])){
		$aql->assign_editkey($channel,'call_counts_query',$call_counts_query);
	}else{
		$aql->assign_append($channel,'call_counts_query',$call_counts_query);
	}
	
	//sync
	$sync = false;
	if(isset($_POST['spans']) && is_array($_POST['spans'])){
		$sync = true;
		for($i=1;$i<=$__GSM_SUM__;$i++){
			if(isset($_POST['spans'][1][$i])){
				$sync_port[$i] = $_POST['spans'][1][$i];
			}
		}
	}
	
	if($sync){
		//registered_query
		if(isset($_POST['registered_query_sync'])){
			if($_POST['registered_query'] == 'on'){
				$_POST['registered_query'] = 1;
			}else{
				$_POST['registered_query'] = 0;
			}
		}
		
		foreach($sync_port as $port => $value){
			if(!isset($exist_array[$port])){
				$aql->assign_addsection($port,'');
			}
			
			if(isset($_POST['query_switch_sync'])){
				$_POST['query_type_sync'] = 'on';
				if(isset($_POST['query_switch']) && $_POST['query_switch'] == 'on'){
					$bal_query_type = trim($_POST['query_type']);
				}else{
					$bal_query_type = 0;
				}
			}
			
			//query_type
			if(isset($_POST['query_type_sync'])){
				if(isset($exist_array[$port]['query_type'])){
					$num_query_type_sync = $exist_array[$port]['query_type']&240;
					$_POST['query_type'] = $bal_query_type|$num_query_type_sync;
				}
			}
			
			foreach($_POST as $key => $value){
				if($key == 'query_switch') continue;
				if(isset($_POST[$key.'_sync'])){
					if(isset($exist_array[$port][$key])){
						$aql->assign_editkey($port,$key,$_POST[$key]);
					}else{
						$aql->assign_append($port,$key,$_POST[$key]);
					}
				}
			}
		}
	}
	
	if (!$aql->save_config_file('sim_query.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	
	wait_apply("exec","");
}

if(isset($_POST['send']) || isset($_GET['send'])){
	if($_GET['send']=='Edit'){
		edit_balance();
	}else if($_POST['send']=='Save'){
		save_balance();
		show_balance();
	}
}else{
	show_balance();
}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>