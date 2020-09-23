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
function show_strategy_sms(){
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
				<img src="/images/sms-failed.png" />
				<span><?php echo language('SMS Limit');?></span>
			</div>
		</div>
		
		<table width="100%" class="tshow">
			<tr>
				<th style="width:0.3%;"><input type="checkbox" name="selall_port" onclick="selectAll(this.checked,'port[]')" /></th>
				<th width="150px"><?php echo language('Port');?></th>
				<th><?php echo language('Card 1');?></th>
				<th><?php echo language('Card 2');?></th>
				<th><?php echo language('Card 3');?></th>
				<th><?php echo language('Card 4');?></th>
				<th width="38px"><?php echo language('Actions');?></th>
			</tr>
			
			<?php
			for($i=1;$i<=$__GSM_SUM__;$i++){
				$key = get_gsm_name_by_channel_for_showtype($i);
				$channel_type = get_gsm_type_by_channel($i,1);
				$val = $res_arr[$i];
			?>
			<tr <?php if($channel_type == 'NULL') echo 'style="display:none;"'; ?> >
				<td><input type="checkbox" class="sms_limit_checked" name="port[]" value="<?php echo $key;?>" /></td>
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
	
	//batch_sms_unlimit
	$("#batch_sms_unlimit").click(function(){
		var ret = confirm("<?php echo language('SMS Unlimit Tip','Are you sure you want to unlimit it?');?>");
		
		if(ret){
			if($(".sms_limit_checked:checked").length == 0){
				alert("<?php echo language("Please choose port");?>");
			}else if($(".sms_limit_checked").size() == $(".sms_limit_checked:checked").length){
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
				$(".sms_limit_checked").each(function(){
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

function edit_strategy_sms(){
	global $__GSM_SUM__;
	$lang_sync_title = language('Synchronization option');
	
	$channel = $_GET['sel_gsm'];
	
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw/call_limit/');
	$res = $aql->query("select * from calllimit_settings.conf");
	
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
	
	$day_sms_settings = $res[$channel]['day_sms_settings'];
	
	$mon_sms_settings = $res[$channel]['mon_sms_settings'];
	
	$sms_clean_date = $res[$channel]['sms_clean_date'];
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
			<li class="tbg"><?php echo language('SMS Limit');?></li>
			<li class="tb2">&nbsp;</li>
		</div>
		
		<table width="100%" class="tedit">
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('SMS Limit Switch');?>
						<span class="showhelp">
						<?php echo language('SMS Limit Switch help');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="sms_limit_switch_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="checkbox" name="sms_limit_switch" id="sms_limit_switch" <?php echo $sms_limit_switch;?> />
					<span id="csms_limit_switch"></span>
				</td>
			</tr>
			
			<tr class="sms_limit_tr">
				<th>
					<div class="helptooltips">
						<?php echo language('SMS Limit Success Flag');?>
						<span class="showhelp">
						<?php echo language('SMS Limit Success Flag help','When close, no matter whether the message is sent successfully or not, the number of messages is counted.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="sms_limit_success_flag_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="checkbox" name="sms_limit_success_flag" id="sms_limit_success_flag" <?php echo $sms_limit_success_flag;?> />
					<span id="csms_limit_success_flag"></span>
				</td>
			</tr>
			
			<tr class="sms_limit_tr">
				<th>
					<div class="helptooltips">
						<?php echo language('Day Limit SMS Count');?>
						<span class="showhelp">
						<?php echo language('Day Limit SMS Count help', 'Limit the number of SMS sent daily, the default value is 0, indicating unlimited.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="day_sms_settings_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="day_sms_settings" id="day_sms_settings" value="<?php echo $day_sms_settings;?>" />
					<span id="cday_sms_settings"></span>
				</td>
			</tr>
			
			<tr class="sms_limit_tr">
				<th>
					<div class="helptooltips">
						<?php echo language('Month Limit SMS Count');?>
						<span class="showhelp">
						<?php echo language('Month Limit SMS Count help', 'Limit the number of SMS sent per month, the default value is 0, indicating unlimited.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="mon_sms_settings_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="text" name="mon_sms_settings" id="mon_sms_settings" value="<?php echo $mon_sms_settings;?>" />
					<span id="cmon_sms_settings"></span>
				</td>
			</tr>
			
			<tr class="sms_limit_tr">
				<th>
					<div class="helptooltips">
						<?php echo language('SMS Clean Date');?>
						<span class="showhelp">
						<?php echo language('SMS Clean Date help', 'The number of messages sent monthly is automatically cleared at 0 points, 0 minutes and 0 seconds on the set date.');?>
						</span>
					</div>
				</th>
				<td>
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
					<span id="csms_clean_date"></span>
				</td>
			</tr>
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
				<li><a style="color:#CD3278;" href="/cgi-bin/php/strategy-sms.php?sel_gsm=<?php echo $cnum; ?>&send=Modify" ><?php echo get_gsm_name_by_channel($cnum,$bnum);?></a></li>
		<?php
					}else{
		?>
				<li><a style="color:LemonChiffon4;" href="/cgi-bin/php/strategy-sms.php?sel_gsm=<?php echo $cnum; ?>&send=Modify" ><?php echo get_gsm_name_by_channel($cnum,$bnum);?></a></li>
		<?php
					}
				}
			}
		?>
		</div>
	</div>

	<script>
	$(function(){
		$("#sms_limit_switch").iButton();
		$("#sms_limit_success_flag").iButton();
		
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
	});
	
	function check(){
		//sms limit
		if(document.getElementById('sms_limit_switch').checked){
			var day_sms_settings = document.getElementById('day_sms_settings').value;
			document.getElementById('cday_sms_settings').innerHTML = '';
			if(isNaN(day_sms_settings) || parseInt(day_sms_settings) < 0 || parseInt(day_sms_settings) > 65535){
				document.getElementById('day_sms_settings').focus();
				document.getElementById('cday_sms_settings').innerHTML = con_str("Must be Number.Range:0-65535");
				return false;
			}
			
			var mon_sms_settings = document.getElementById('mon_sms_settings').value;
			document.getElementById('cmon_sms_settings').innerHTML = '';
			if(isNaN(mon_sms_settings) || parseInt(mon_sms_settings) < 0 || parseInt(mon_sms_settings) > 65535){
				document.getElementById('mon_sms_settings').focus();
				document.getElementById('cmon_sms_settings').innerHTML = con_str("Must be Number.Range:0-65535");
				return false;
			}
			
			if(document.getElementById('mon_sms_settings').value != 0){
				var sms_clean_date = document.getElementById('sms_clean_date').value;
				document.getElementById('csms_clean_date').innerHTML = '';
				if(isNaN(sms_clean_date) || parseInt(sms_clean_date) < 1 || parseInt(sms_clean_date) > 31){
					document.getElementById('sms_clean_date').focus();
					document.getElementById('csms_clean_date').innerHTML = con_str("Must be Number.Range:1-31");
					return false;
				}
			}else if(document.getElementById('mon_sms_settings').value == 0){
				var sms_clean_date = document.getElementById('sms_clean_date').value;
				document.getElementById('csms_clean_date').innerHTML = '';
				if(isNaN(sms_clean_date) || parseInt(sms_clean_date) < 0 || parseInt(sms_clean_date) > 31){
					document.getElementById('sms_clean_date').focus();
					document.getElementById('csms_clean_date').innerHTML = con_str("Must be Number.Range:0-31");
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

function save_strategy_sms(){
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
				if($key == 'sms_limit_switch' || $key == 'sms_limit_success_flag'){
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
	
	if(!isset($_POST['sms_limit_switch'])){
		exec("/my_tools/calllimit_cli set chn monsmscnt $channel 0");
	}
	
	wait_apply("exec","/my_tools/set_calllimit.sh reload");
}

if($_POST){
	if(isset($_POST['send']) && $_POST['send'] == 'Save'){
		save_strategy_sms();
		show_strategy_sms();
	}
}else if($_GET){
	if(isset($_GET['send']) && $_GET['send'] == 'Modify'){
		edit_strategy_sms();
	}
}else{
	show_strategy_sms();
}

require("/www/cgi-bin/inc/boot.inc");
?>
<div id="float_btn1" class="sec_float_btn1"></div>
<div class="float_close" onclick="close_btn()" ></div>