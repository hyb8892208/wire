<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");
?>

<?php 
function show_phonenumber(){
	global $__GSM_SUM__;
	global $only_view;
	
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$res = $aql->query("select * from sim_query.conf");
	
	if($res['general']['phonenum_switch'] == 'on'){
		$phonenum_switch = 'checked';
	}else{
		$phonenum_switch = '';
	}
?>
<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="get">
	
	<div class="content">
		<span class="title"><?php echo language('General');?></span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Mobile Number Switch');?>:
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<b><?php echo language('Mobile Number Switch');?>:</b><br/>
							<?php echo language('Mobile Number Switch help', 'When this switch is turned on, the mobile number will be displayed on the pages.');?>
						</div>
					</div>
				</div>
			</span>
			<div class="tab_item_right">
				<div style="height:30px;width:50px;float:left;">
					<span><input type="checkbox" name="phonenum_switch" id="phonenum_switch" <?php echo $phonenum_switch;?> /></span>
				</div>
				
				<?php if(!$only_view){ ?>
				<input type="submit" value="<?php echo language('Save');?>" style="float:right;margin-right:10px;" onclick="document.getElementById('send').value='Save';" />
				<?php } ?>
			</div>
		</div>
	</div>

	<div class="content">
		<span class="title"><?php echo language('PhoneNumber').language('Setting');?></span>
		
		<table class="table_show">
			<tr>
				<th width=""><?php echo language('Port');?></th>
				<th width="100px"><?php echo language('Query Type');?></th>
				<th width="180px"><?php echo language('Destination Number');?></th>
				<th width="180px"><?php echo language('Receive Number');?></th>
				<th width="250px"><?php echo language('Send Message');?></th>
				<th><?php echo language('Matching Key');?></th>
				<th><?php echo language('PhoneNumber');?></th>
				<th width="50px"><?php echo language('Actions');?></th>
			</tr>
			
<?php
			for($c=1; $c<=$__GSM_SUM__; $c++){
				$channel_name = get_gsm_name_by_channel($c);
				if(strstr($channel_name,'null')) continue;
				
				$phonenum_query_type = '';
				if(isset($res[$c]['query_type'])){
					$temp = $res[$c]['query_type'];
					$phonenum_query_type = $temp&240;
				}
				$query_type = language('Inactive');
				if($phonenum_query_type==80){
					$query_type = language('SMS');
				}else if($phonenum_query_type==96){
					$query_type = language('Tel');
				}else if($phonenum_query_type==112){
					$query_type = language('USSD');
				}
				
				$phonenum_dst_num = '';
				if(isset($res[$c]['phonenum_dst_num'])){
					$phonenum_dst_num = $res[$c]['phonenum_dst_num'];
				}
				
				$phonenum_recv_num = '';
				if(isset($res[$c]['phonenum_recv_num'])){
					$phonenum_recv_num = $res[$c]['phonenum_recv_num'];
				}
				
				$phonenum_send_msg = '';
				if(isset($res[$c]['phonenum_send_msg'])){
					$phonenum_send_msg = $res[$c]['phonenum_send_msg'];
				}
				
				$phonenum_match_key = '';
				if(isset($res[$c]['phonenum_match_key'])){
					$phonenum_match_key = $res[$c]['phonenum_match_key'];
				}
				
				exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $c",$output);
				$phonenum = $output[0];
				$output = '';
?>
			<tr>
				<td><?php echo $channel_name;?></td>
				<td><?php echo $query_type;?></td>
				<td><?php echo $phonenum_dst_num;?></td>
				<td><?php echo $phonenum_recv_num;?></td>
				<td><?php echo $phonenum_send_msg;?></td>
				<td><?php echo $phonenum_match_key;?></td>
				<td><?php echo $phonenum;?></td>
				<td style="text-align:center;">
				
					<button type="submit" value="Modify" style="width:32px;height:32px;padding:0;" onclick="document.getElementById('send').value='Edit';document.getElementById('channel_sel').value='<?php echo $c;?>'">
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
	</div>
</form>
	
<?php
}
?>

<?php
function edit_phonenumber(){
	global $__GSM_SUM__;
	global $only_view;
	
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$channel = $_GET['channel_sel'];
	$res = $aql->query("select * from sim_query.conf where section='$channel'");
	
	$lang_sync_title = language('Synchronization option');
	
	$query_type = 0;
	if(isset($res[$channel]['query_type'])){
		$temp = $res[$channel]['query_type'];
		$query_type = $temp&240;
	}
	
	$query_switch = 'checked';
	if($query_type == 0){
		$query_switch = '';
	}
	
	$phonenum_dst_num = '';
	if(isset($res[$channel]['phonenum_dst_num'])){
		$phonenum_dst_num = $res[$channel]['phonenum_dst_num'];
	}
	
	$phonenum_recv_num = '';
	if(isset($res[$channel]['phonenum_recv_num'])){
		$phonenum_recv_num = $res[$channel]['phonenum_recv_num'];
	}
	
	$phonenum_send_msg = '';
	if(isset($res[$channel]['phonenum_send_msg'])){
		$phonenum_send_msg = $res[$channel]['phonenum_send_msg'];
	}
	
	$phonenum_match_key = '';
	if(isset($res[$channel]['phonenum_match_key'])){
		$phonenum_match_key = $res[$channel]['phonenum_match_key'];
	}

?>
<script type="text/javascript" src="/js/check.js?v=1.1"></script>
<script type="text/javascript" src="/js/functions.js"></script>
	
<form id="manform" enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
		<input type="hidden" name="channel" value="<?php echo $channel;?>" />
		
	<div class="content">
		<span class="title">
			<?php echo language('Port');?> <?php echo get_gsm_name_by_channel($channel);?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Query Swith')){ ?>
							<b><?php echo language('Query Swith');?>:</b><br/>
							<?php echo language('Query Swith help','Enquiry Number after Opening.'); ?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Query Type')){ ?>
							<b><?php echo language('Query Type');?>:</b><br/>
							<?php echo language('Query Type help','If the channel is in call state, the query will fail.');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Destination Number')){ ?>
							<b><?php echo language('Destination Number');?>:</b><br/>
							<?php echo language('Destination Number help','Number used by operator to receive inquiry information.'); ?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Receive Number')){ ?>
							<b><?php echo language('Receive Number');?>:</b><br/>
							<?php echo language('Receive Number help','Number used by the operator to send the queried content.'); ?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Send Message')){ ?>
							<b><?php echo language('Send Message');?>:</b><br/>
							<?php echo language('Send Message help','Short message content sent to operators for queries.'); ?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Matching Key')){ ?>
							<b><?php echo language('Matching Key');?>:</b><br/>
							<?php echo language('Matching Key help','Operators reply to the contents of the query before the number of the first 10 words, not more than 30 words.'); ?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Query Swith');?>:
			</span>
			<div class="tab_item_right">
				<span>
					<span id="cquery_switch"></span>
					<input type="checkbox" class="setting_sync" name="query_switch_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="checkbox" name="query_switch" id="query_switch" <?php echo $query_switch; ?> />
				</span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Query Type');?>:
			</span>
			<div class="tab_item_right">
				<span id="cquery_type"></span>
				<input type="checkbox" class="setting_sync" name="query_type_sync" title="<?php echo $lang_sync_title;?>" />
				<select name="query_type" id="query_type">
					<option value="80" <?php if($query_type==80)echo 'selected';?>><?php echo language('SMS');?></option>
					<option value="96" <?php if($query_type==96)echo 'selected';?>><?php echo language('Tel');?></option>
					<option value="112" <?php if($query_type==112)echo 'selected';?>><?php echo language('USSD');?></option>
				</select>
			</div>
		</div>
		
		<div class="tab_item" id="show_distination">
			<span>
				<?php echo language('Destination Number');?>:
			</span>
			<div class="tab_item_right">
				<span id="cphonenum_dst_num"></span>
				<input type="checkbox" class="setting_sync" name="phonenum_dst_num_sync" title="<?php echo $lang_sync_title;?>" />
				<input type="text" name="phonenum_dst_num" id="phonenum_dst_num" value="<?php echo $phonenum_dst_num; ?>" 
				oninput="this.value=this.value.replace(/[^\+\d]*/g,'')" onkeyup="this.value=this.value.replace(/[^\+\d]*/g,'')" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Receive Number');?>:
			</span>
			<div class="tab_item_right">
				<span id="cphonenum_recv_num"></span>
				<input type="checkbox" class="setting_sync" name="phonenum_recv_num_sync" title="<?php echo $lang_sync_title;?>" />
				<input type="text" name="phonenum_recv_num" id="phonenum_recv_num" value="<?php echo $phonenum_recv_num; ?>"
				oninput="this.value=this.value.replace(/[^\+\d]*/g,'')" onkeyup="this.value=this.value.replace(/[^\+\d]*/g,'')" />
			</div>
		</div>
		
		<div class="tab_item" id="show_send_message">
			<span>
				<?php echo language('Send Message');?>:
			</span>
			<div class="tab_item_right">
				<span id="cphonenum_send_msg"></span>
				<input type="checkbox" class="setting_sync" name="phonenum_send_msg_sync" title="<?php echo $lang_sync_title;?>" />
				<input type="text" name="phonenum_send_msg" id="phonenum_send_msg" value="<?php echo $phonenum_send_msg; ?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Matching Key');?>:
			</span>
			<div class="tab_item_right">
				<span id="cphonenum_match_key"></span>
				<input type="checkbox" class="setting_sync" name="phonenum_match_key_sync" title="<?php echo $lang_sync_title;?>" />
				<input type="text" name="phonenum_match_key" id="phonenum_match_key" value="<?php echo $phonenum_match_key; ?>" />
			</div>
		</div>
	</div>
	
	<div class="content">
		<span class="title">
			<?php echo language('The matching test');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Message content')){ ?>
							<b><?php echo language('Message content');?>:</b><br/>
							<?php echo language('Message content help', 'Message content'); ?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Matching results')){ ?>
							<b><?php echo language('Matching results');?>:</b><br/>
							<?php echo language('Matching results help', 'Matching results'); ?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Message content');?>:
			</span>
			<div class="tab_item_right">
				<span id="csms_content"></span>
				<textarea id="sms_content" style="height:120px;width:500px;"></textarea>
			</div>
			<div class="clear"></div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Matching results');?>:
			</span>
			<div class="tab_item_right">
				<span id="callback" style="margin-right:10px;"></span>
				<input id="match_test" type="button" value="<?php echo language('Test');?>" />
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
			<div class="tab_item_right">
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
			</div>
			<div class="clear"></div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Sync All Settings');?>:
			</span>
			<div class="tab_item_right">
				<span><input type="checkbox" id="all_settings_sync" onclick="selectAllCheckbox(this.checked,'class','setting_sync');" checked disabled />
				<?php echo language('Select all settings');?></span>
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
		for($c=1; $c<=$__GSM_SUM__; $c++){
			$channel_name = get_gsm_name_by_channel($c);
			if(strstr($channel_name,'null')) continue;
		?>
			<li>
				<a style="<?php if($c==$channel){echo 'color:#CD3278;';}else{echo 'color:LemonChiffon4;';}?>" href="/cgi-bin/php/adv-phonenumber.php?channel_sel=<?php echo $c; ?>&send=Edit" >
					<?php echo $channel_name;?>
				</a>
			</li>
		<?php 
		}
		?>
		</div>
	</div>
-->

	<script>
	$(document).ready(function (){
		float_sort_hide();
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
		var is_check = false;
		
		var checked = document.getElementById('query_switch').checked;
		var phonenum_dst_num = document.getElementById('phonenum_dst_num').value;
		var phonenum_recv_num = document.getElementById('phonenum_recv_num').value;
		var phonenum_send_msg = document.getElementById('phonenum_send_msg').value;
		var phonenum_match_key = document.getElementById('phonenum_match_key').value;
		var query_type = document.getElementById('query_type').value;
		
		if(checked){
			if(query_type != '112'){
				if(phonenum_dst_num == ''){
					document.getElementById('cphonenum_dst_num').innerHTML = con_str("<?php echo language('js check null', 'can not be null!')?>");
					document.getElementById('phonenum_dst_num').focus();
					is_check = true;
				}else if(!check_phonenum(phonenum_dst_num)){
					document.getElementById('cphonenum_dst_num').innerHTML = con_str("<?php echo language('js check phonenum','Please input a valid phone number!');?>");
					document.getElementById('phonenum_dst_num').focus();
					is_check = true;
				}else{
					document.getElementById('cphonenum_dst_num').innerHTML = '';
				}
			}
			
			document.getElementById('cphonenum_recv_num').innerHTML = '';
			if(query_type != '112'){
				if(phonenum_recv_num == ''){
					document.getElementById('cphonenum_recv_num').innerHTML = con_str("<?php echo language('js check null', 'can not be null!');?>");
					document.getElementById('phonenum_recv_num').focus();
					is_check = true;
				}else if(!check_phonenum(phonenum_recv_num)){
					document.getElementById('cphonenum_recv_num').innerHTML = con_str("<?php echo language('js check phonenum','Please input a valid phone number!');?>");
					document.getElementById('phonenum_recv_num').focus();
					is_check = true;
				}else{
					document.getElementById('cphonenum_recv_num').innerHTML = '';
				}
			}
			
			if(query_type != '96'){
				if(phonenum_send_msg == ''){
					document.getElementById('cphonenum_send_msg').innerHTML = con_str("<?php echo language('js check null','can not be null!');?>");
					document.getElementById('phonenum_send_msg').focus();
					is_check = true;
				}else if(!check_send_msg(phonenum_send_msg)){
					document.getElementById('cphonenum_send_msg').innerHTML = con_str("<?php echo language('js check send_msg', 'Allowed character must be 1-128 characters.');?>");
					document.getElementById('phonenum_send_msg').focus();
					is_check = true;
				}else{
					document.getElementById('cphonenum_send_msg').innerHTML = '';
				}
			}
			
			if(phonenum_match_key == ''){
				document.getElementById('cphonenum_match_key').innerHTML = con_str("<?php echo language('js check null','can not be null!');?>");
				document.getElementById('phonenum_match_key').focus();
				is_check = true;
			}else if(!check_match_key(phonenum_match_key)){
				document.getElementById('cphonenum_match_key').innerHTML = con_str("<?php echo language('js check match_key', 'Allowed character must be 1-32 characters.');?>");
				document.getElementById('phonenum_match_key').focus();
				is_check = true;
			}else{
				document.getElementById('cphonenum_match_key').innerHTML = '';
			}
		}
		
		if(is_check){
			return false;
		}
		
		return true;
	}

	$("#query_type").change(function(){
		if($(this).val()=='80'){
			$("#show_distination").show();
			$("#show_send_message").show();
		}else if($(this).val()=='96'){
			$("#show_distination").show();
			$("#show_send_message").hide();
		}else if($(this).val()=='112'){
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
		var sw = $("#query_switch").attr('checked');
		if(sw == 'checked'){
			$(".sw_show").show();
			
			if($("#query_type").val()=='80'){
				$("#show_distination").show();
				$("#show_send_message").show();
			}else if($("#query_type").val()=='96'){
				$("#show_distination").show();
				$("#show_send_message").hide();
			}else if($("#query_type").val()=='112'){
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
		var match_key_info = $("#phonenum_match_key").val();
		
		if(match_key_info == ''){
			document.getElementById('cphonenum_match_key').innerHTML = con_str("<?php echo language('js check null','can not be null!');?>");
			document.getElementById('phonenum_match_key').focus();
			return false;
		}else if(!check_match_key(match_key_info)){
			document.getElementById('cphonenum_match_key').innerHTML = con_str("<?php echo language('js check match_key', 'Allowed character must be 1-32 characters.');?>");
			document.getElementById('phonenum_match_key').focus();
			return false;
		}else{
			document.getElementById('cphonenum_match_key').innerHTML = '';
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
			url: "ajax_server.php?type=match_test&phonenum=",
			data: {
				'query_type':query_type,
				'sms_content':sms_content,
				'match_key_info':match_key_info,
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

function save_phonenumber(){
	global $__GSM_SUM__;
	
	$channel = $_POST['channel'];
	
	$phonenum_query_type = 0;
	if(isset($_POST['query_switch']) && $_POST['query_switch'] == 'on'){
		if(isset($_POST['query_type'])){
			$phonenum_query_type = trim($_POST['query_type']);
		}
	}
	
	$phonenum_dst_num = '';
	if(isset($_POST['phonenum_dst_num'])){
		$phonenum_dst_num = trim($_POST['phonenum_dst_num']);
	}
	
	$phonenum_recv_num = '';
	if(isset($_POST['phonenum_recv_num'])){
		$phonenum_recv_num = trim($_POST['phonenum_recv_num']);
	}
	
	$phonenum_send_msg = '';
	if(isset($_POST['phonenum_send_msg'])){
		$phonenum_send_msg = trim($_POST['phonenum_send_msg']);
	}
	
	$phonenum_match_key = '';
	if(isset($_POST['phonenum_match_key'])){
		$phonenum_match_key = trim($_POST['phonenum_match_key']);
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
		$bal_query_type = $exist_array[$channel]['query_type']&15;
		$query_type = $phonenum_query_type|$bal_query_type;
		$aql->assign_editkey($channel,'query_type',$query_type);
	}else{
		$aql->assign_append($channel,'query_type',$phonenum_query_type);
	}
	
	if(isset($exist_array[$channel]['phonenum_dst_num'])){
		$aql->assign_editkey($channel,'phonenum_dst_num',$phonenum_dst_num);
	}else{
		$aql->assign_append($channel,'phonenum_dst_num',$phonenum_dst_num);
	}
	
	if(isset($exist_array[$channel]['phonenum_recv_num'])){
		$aql->assign_editkey($channel,'phonenum_recv_num',$phonenum_recv_num);
	}else{
		$aql->assign_append($channel,'phonenum_recv_num',$phonenum_recv_num);
	}
	
	if(isset($exist_array[$channel]['phonenum_send_msg'])){
		$aql->assign_editkey($channel,'phonenum_send_msg',$phonenum_send_msg);
	}else{
		$aql->assign_append($channel,'phonenum_send_msg',$phonenum_send_msg);
	}
	
	if(isset($exist_array[$channel]['phonenum_match_key'])){
		$aql->assign_editkey($channel,'phonenum_match_key',$phonenum_match_key);
	}else{
		$aql->assign_append($channel,'phonenum_match_key',$phonenum_match_key);
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
					$bal_query_type_sync = $exist_array[$port]['query_type']&15;
					$_POST['query_type'] = $phonenum_query_type|$bal_query_type_sync;
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
	
	save_user_record("","ADVANCED->PhoneNumber:Save,channel=".$channel);
	
	wait_apply("exec","");
}

function save_general(){
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
	
	if(!isset($exist_array['general'])){
		$aql->assign_addsection('general','');
	}
	
	if(isset($_GET['phonenum_switch'])){
		$phonenum_switch = 'on';
	}else{
		$phonenum_switch = 'off';
	}
	
	if(isset($exist_array['general']['phonenum_switch'])){
		$aql->assign_editkey('general','phonenum_switch',$phonenum_switch);
	}else{
		$aql->assign_append('general','phonenum_switch',$phonenum_switch);
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
		edit_phonenumber();
	}else if($_GET['send']=='Save'){
		if($only_view){
			return false;
		}
		
		save_general();
		show_phonenumber();
	}else if($_POST['send']=='Save'){
		if($only_view){
			return false;
		}
		
		save_phonenumber();
		show_phonenumber();
	}
}else{
	show_phonenumber();
}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>