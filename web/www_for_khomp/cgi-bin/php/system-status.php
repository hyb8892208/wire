<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
include_once("/www/cgi-bin/inc/aql.php");

//locating to system-wizard.php
$setting_wizard = `/my_tools/set_config /etc/asterisk/gw.conf get option_value setup-wizard set_wizard`;
if($setting_wizard == '' || $setting_wizard != 'off'){
	header("location: http://" . $_SERVER['SERVER_NAME'] . '/cgi-bin/php/system-wizard.php');
}
?>


<?php

function show_sips()
{
	$all_sips = get_all_sips(true);
?>
	<div class="content">
		<span class="title"><?php echo language('SIP Information');?></span>
		
		<table class="table_show">
			<tr>
				<th><?php echo language('Endpoint Name');?></th>
				<th><?php echo language('User Name');?></th>
				<th><?php echo language('Host');?></th>
				<th><?php echo language('Registration');?></th>
				<th><?php echo language('SIP Status');?></th>
			</tr>
			
			<?php
			if($all_sips) {
				$sip_peers = get_sip_peers();
				$sip_registrys = get_sip_registrys();

				foreach($all_sips as $sip) {
					$host = '';
					$status = '';
					if($sip['registration'] == 'server') {
						foreach($sip_peers as $each) {
							if(strcmp($sip['username'],$each['username']) == 0 && $each['D']) {
								$host = $each['host'];
								$status = $each['status'];
								break;
							}
						}
					} else if( $sip['registration'] == 'none' ) {
						foreach($sip_peers as $each) {
							if(strcmp($sip['username'],$each['username']) == 0 && strcmp($sip['host'],$each['host']) == 0 && !$each['D']) {
								$host = $each['host'];
								$status = $each['status'];
								break;
							}
						}
					} else if($sip['registration'] == 'client') {
						if(isset($sip['host'])) {
							$host = trim($sip['host']);
							$sip_register_name = $sip['username'];
							if(isset($sip['outboundproxy']) && $sip['outboundproxy'] != ''){
								if(strstr($sip['outboundproxy'], ':')){
									list($host, $port) = explode(':', $sip['outboundproxy']);
								} else {
									$host = trim($sip['outboundproxy']);
								}
								$sip_register_name = $sip['username']."@".$sip['host'];
							}
							if($host != '') {
								foreach($sip_registrys as $each) {
									if(strstr($sip_register_name, $each['username']) && strcmp($host,$each['host']) == 0) {
										$host = $each['host'];
										$status = $each['status'];
										break;
									}
								}
							}
						}
					}
		?>
				<tr>
					<td>	
						<?php echo $sip['endpoint_name'] ?>
					</td>
					<td>	
						<?php echo $sip['username'] ?>						
					</td>
					<td>	
						<?php echo $host ?>							
					</td>
					<td>	
						<?php echo language($sip['registration']) ?>							
					</td>
					<td>
						<?php echo language($status);?>
					</td>
				</tr>
		<?php
				}
			}
		?>
			</table>
		</table>
	</div>

	<br>
<?php
}
?>

<?php
function show_iaxs()
{
	$all_iaxs = get_all_iaxs(true);
?>
	<div class="content">
		<span class="title"><?php echo language('IAX2 Information');?></span>
		
		<table class="table_show">
			<tr>
				<th><?php echo language('Endpoint Name');?></th>
				<th><?php echo language('User Name');?></th>
				<th><?php echo language('Host');?></th>
				<th><?php echo language('Registration');?></th>
				<th><?php echo language('IAX2 Status');?></th>
			</tr>
			
			<?php
			if($all_iaxs) {
				foreach($all_iaxs as $iax) {		
					$host = '';
					$status = '';		
			?>
				<tr>
					<td>	
						<?php echo $iax['endpoint_name'] ?>
					</td>
					<td>	
						<?php echo $iax['username'] ?>						
					</td>
					<td>	
						<?php 					
						if ($iax['username']=='anonymous'){
							echo $iax['host'];
						}else{		
							$host=get_iax_peers_new($iax['username'],'host');
							echo $host;
						}
						?>							
					</td>
					<td>	
						<?php echo language($iax['registration']) ?>							
					</td>
					<td>
						<?php $ret=get_iax_peers_new($iax['username'],'status');
						if (($ret=="Unmonitored") && ($iax['username']!="anonymous")){
							$ret1=get_sip_registrys_new($iax['username'],'status');						
							if ($ret1!=""){
								echo $ret1;
							}else{
								echo $ret;
							}
						}else{
							echo $ret;					
						}
						?>
					</td>
				</tr>
			<?php
				}
			}
			?>
		</table>
	</div>
	
	<br>
<?php
}
?>


<?php
function show_gsms()
{
	global $__GSM_SUM__;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $__deal_cluster__;
	if($__deal_cluster__){
		$cluster_info = get_cluster_info();
	}
	$board = get_slotnum();
	$js_board_str = '"'.$board.'"';
	
	//simbank tip
	$aql = new aql();
	$simbank_conf = "/etc/asterisk/simemusvr.conf";
	$aql->set('basedir','/etc/asterisk');
	
	$res = $aql->query("select * from simemusvr.conf");
	if(isset($res['SimEmuSvr']['simemusvr_switch']) && $res['SimEmuSvr']['simemusvr_switch'] == 'yes'){
		$sim_sw = true;
	}else{
		$sim_sw = false;
	}
	
	//phone number switch
	$aql->set('basedir','/etc/asterisk/gw');
	$phonenum_res = $aql->query("select * from sim_query.conf");
?>

	<div class="content">
		<span class="title"><?php echo language('Module Information');?></span>
		
		<table class="table_show">
			<tr>
				<th><?php echo language('Port');?></th>
				<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
				<th><?php echo language('Mobile Number');?></th>
				<?php } ?>
				
				<th width="4%"><?php echo language('Type');?></th>
				<th width="3%"> <?php echo language('Signal');?></th>
				<th width="5%"> <?php echo language('Band');?></th>
				<th width="4%"> <?php echo language('BER');?></th>
				<th width="10%"><?php echo language('Carrier');?></th>
				<th width="15%"><?php echo language('Registration Status');?></th>
				<th width="4%"> <?php echo language('PDD');?>(s)</th>
				<th width="4%"> <?php echo language('ACD');?>(s)</th>
				<th width="4%"> <?php echo language('ASR');?>(%)</th>
				<th width="25%"><?php echo language('Module Status');?></th>
				<th width="11%">              <?php echo language('Remain Time')." | ".language('Times');?></th>
			</tr>
			
			<?php
				$aql = new aql();
				$aql->set('basedir','/etc/asterisk/gw');
				$res = $aql->query("select * from sim_query.conf");
				
				for($c=1; $c<=$__GSM_SUM__; $c++) {
					$channel_type = get_gsm_type_by_channel($c,1);
					
					$phonenum = '';
					if(($res[$c]['query_type']&240) != 0){
						exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $c",$phone_output);
						$phonenum = $phone_output[0];
						$phone_output = '';
					}
			?>
			<tr id='<?php echo "gsm_${board}_${c}";?>' <?php if($channel_type == 'NULL') echo 'style="display:none;"';?>>
				<td>
					<div class="infotooltips">
						<span id='<?php echo "gsm_${board}_${c}-show";?>' ><span><?php echo change_gsms_port_for_showtype($c,1); ?></span></span>
						<span class="showdetail" id='<?php echo "gsm_${board}_${c}-showdetail";?>' style="line-height:20px;">
							<b><?php echo language('Model IMEI');?>:</b>&nbsp;			<span></span><br>
							<b><?php echo language('Network Name');?>:</b>&nbsp;			<span></span><br>
							<b><?php echo language('Network Status');?>:</b>&nbsp;			<span></span><br>
							<b><?php echo language('Signal Quality');?> (0,31):</b>&nbsp;		<span></span><br>
							<b><?php echo language('BER value');?> (0,7):</b>&nbsp; 		<span></span><br>
							<b><?php echo language('SIM IMSI');?>:</b>&nbsp;  			<span></span><br>
							<b><?php echo language('SIM SMS Center Number');?>:</b>&nbsp;		<span></span><br>
							<b style='display:none;'><?php echo language('Own Number');?>:</b><span></span>
							<b><?php echo language('Phone Number');?>:</b>&nbsp;		<span><?php echo $phonenum;?></span><br>
							<b><?php echo language('Remain Time');?>:</b>&nbsp;			<span></span><br>
							<b><?php echo language('PDD');?>(s):</b>&nbsp;       			<span></span><br>
							<b><?php echo language('ACD');?>(s):</b>&nbsp;       			<span></span><br>
							<b><?php echo language('ASR');?>(%):</b>&nbsp;       			<span></span><br>
							<b><?php echo language('State');?>:</b>&nbsp;				<span></span><br>
						</span>
					</div>
					
					<div class="infotooltips" style="margin-left:5px;display:none;border-bottom:none;">
						<span id='<?php echo "gsm_${board}_${c}-show";?>' ><img src="/images/triangle_limit.png" style="width:23px;height:23px;"></span>
						<span class="showdetail" id='<?php echo "gsm_${c}-showlimit"?>' style="line-height:20px;">
							<b><?php echo language('Call Limit');?>:</b>&nbsp;<span></span><br/>
							<b><?php echo language('Lock Status');?>:</b>&nbsp;<span></span><br/>
							<b><?php echo language('Mark Status');?>:</b>&nbsp;<span></span><br/>
							<b><?php echo language('SMS Status');?>:</b>&nbsp;<span></span><br/>
						</span>
					</div>
				</td>
				
				<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
				<td><?php echo $phonenum;?></td>
				<?php } ?>
				
				<td><?php echo $channel_type;?></td>
				<td></td>			
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
			</tr>
			<?php } ?>
		</table>
	</div>

	<script type="text/javascript">
	function dynamic_show_gsm()
	{
		var bs = new Array(<?php echo $js_board_str;?>);
		var board_sum = bs.length;
		var error_sum = 5;
		for(i in bs) {
			get_gsm_info(bs[i]);
		}
		if (board_sum > 1) {
			get_module_info(bs);
		}		
		function get_module_info(boards,error_sum) {
			var server_file = "./../../cgi-bin/php/ajax_server.php";
			var str = "";
			
			$.ajax({
				url: server_file+"?random="+Math.random()+"&type=system&module_type=module_type",	
				dataType: 'json',				
				type: 'GET',
				data:{},
				error: function(){				//request failed callback function;
					str = "";
					str += "<font color='black'>Can't get.</font><br>";
				},
				success: function(moduleinfo){			//request success callback function;
					for (i in boards) {
						var board = boards[i];
						for (var key1 in moduleinfo[board]) {
							head = moduleinfo[board][key1];
							$("#gsm_"+board+"_"+key1+"-show span:eq(0)").html(value);
						}
					}
				},
				complete: function(){
					setTimeout(function(){get_module_info(boards,error_sum);},4000);
				}

			});			
		}
		
		
		function get_gsm_info(board,error_sum){
			if(board == 1){
				gsm_url = "/service?action=get_gsminfo&" + "random=" + Math.random();
			}else{
				gsm_url = "/" + board + "/service?action=get_gsminfo&" + "random=" + Math.random();
			}
			$.ajax({
				url: gsm_url,
				type: 'GET',
				dataType: 'json',
				data: {},
				success: function(gsminfo){
					error_sum = 5;
					refresh_gsm_table(gsminfo,board);
				},
				error: function(){
					if(error_sum <= 0){
						clean_gsm_info(board);
						error_sum = 5;
					} else {
						error_sum -= 1;
					}
				},
				complete: function(){
					setTimeout(function(){get_gsm_info(board,error_sum);},4000);
				}
			});
		};

		function refresh_gsm_table(gsminfo,board) {
			<?php if($sim_sw){?>
				var sim_sw = 'on';
			<?php }else{?>
				var sim_sw = 'off';
			<?php }?>
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
				var n = 1;
			<?php }else{ ?>
				var n = 0;
			<?php }?>
			
			for(var key1 in gsminfo){
				for(var key2 in gsminfo[key1]) {
					if(gsminfo[key1][key2].show_status 	== "" &&
						gsminfo[key1][key2].ber 	== "" &&
						gsminfo[key1][key2].operator == "" &&
						gsminfo[key1][key2].register == "" &&
						gsminfo[key1][key2].pdd 	== "" &&
						gsminfo[key1][key2].acd 	== "" &&
						gsminfo[key1][key2].asr 	== "" &&
						gsminfo[key1][key2].state 	== ""
					) {
						continue;
					} else {
						if(sim_sw == 'on'){
							var temp = gsminfo[key1][key2].show_status;
							temp = temp.replace('nosim.gif','sim_wifi0.png');
							temp = temp.replace('wifi0.gif','sim_wifi0.png');
							temp = temp.replace('wifi1.gif','sim_wifi1.png');
							temp = temp.replace('wifi2.gif','sim_wifi2.png');
							temp = temp.replace('wifi3.gif','sim_wifi3.png');
							temp = temp.replace('wifi4.gif','sim_wifi4.png');
							temp = temp.replace('wifi5.gif','sim_wifi5.png');
							$("#gsm_"+board+"_"+key1+" td:eq("+(2+n)+")").html(temp);
							$("#gsm_"+board+"_"+key1+" td:eq("+(2+n)+") img").css({"width":"33px","height":"30px"});
						}else{
							$("#gsm_"+board+"_"+key1+" td:eq("+(2+n)+")").html(gsminfo[key1][key2].show_status);
						}
						$("#gsm_"+board+"_"+key1+" td:eq("+(3+n)+")").html(gsminfo[key1][key2].band);
						$("#gsm_"+board+"_"+key1+" td:eq("+(4+n)+")").html(gsminfo[key1][key2].ber);
						$("#gsm_"+board+"_"+key1+" td:eq("+(5+n)+")").html(gsminfo[key1][key2].operator);
						$("#gsm_"+board+"_"+key1+" td:eq("+(6+n)+")").html(gsminfo[key1][key2].register);
						$("#gsm_"+board+"_"+key1+" td:eq("+(7+n)+")").html(gsminfo[key1][key2].pdd);
						$("#gsm_"+board+"_"+key1+" td:eq("+(8+n)+")").html(gsminfo[key1][key2].acd);
						$("#gsm_"+board+"_"+key1+" td:eq("+(9+n)+")").html(gsminfo[key1][key2].asr);
						$("#gsm_"+board+"_"+key1+" td:eq("+(10+n)+")").html(gsminfo[key1][key2].state);
						$("#gsm_"+board+"_"+key1+" td:eq("+(11+n)+")").html(gsminfo[key1][key2].remain_time+' | '+gsminfo[key1][key2].day_remain_calls);
					}

					/* show gsm detail information */
					$("#gsm_"+board+"_"+key1+"-showdetail span:eq(00)").html(gsminfo[key1][key2].model_imei);
					$("#gsm_"+board+"_"+key1+"-showdetail span:eq(01)").html(gsminfo[key1][key2].operator);
					$("#gsm_"+board+"_"+key1+"-showdetail span:eq(02)").html(gsminfo[key1][key2].register);
					$("#gsm_"+board+"_"+key1+"-showdetail span:eq(03)").html(gsminfo[key1][key2].signal);
					$("#gsm_"+board+"_"+key1+"-showdetail span:eq(04)").html(gsminfo[key1][key2].ber);
					$("#gsm_"+board+"_"+key1+"-showdetail span:eq(05)").html(gsminfo[key1][key2].sim_imsi);
					$("#gsm_"+board+"_"+key1+"-showdetail span:eq(06)").html(gsminfo[key1][key2].sim_sms_center_number);
					$("#gsm_"+board+"_"+key1+"-showdetail span:eq(07)").html(gsminfo[key1][key2].own_number);
					$("#gsm_"+board+"_"+key1+"-showdetail span:eq(09)").html(gsminfo[key1][key2].remain_time);
					$("#gsm_"+board+"_"+key1+"-showdetail span:eq(10)").html(gsminfo[key1][key2].pdd);
					$("#gsm_"+board+"_"+key1+"-showdetail span:eq(11)").html(gsminfo[key1][key2].acd);
					$("#gsm_"+board+"_"+key1+"-showdetail span:eq(12)").html(gsminfo[key1][key2].asr);
					$("#gsm_"+board+"_"+key1+"-showdetail span:eq(13)").html(gsminfo[key1][key2].state);
					
					/* show port limit status */
					var limit = gsminfo[key1][key2].limit;
					if(limit != undefined && (limit.indexOf('Call') != -1 || limit.indexOf('Locked') != -1 || limit.indexOf('Locking') != -1 || limit.indexOf('Marked') != -1 || limit.indexOf('Sms') != -1)){
						$("#gsm_"+key1+"-showlimit").parent().show();
						
						if(limit.indexOf('Call') != -1){
							var call_limit = '<?php echo language('Limited');?>';
						}else{
							var call_limit = '<?php echo language('Unlimited');?>';
						}
						
						if(limit.indexOf('Locked') != -1){
							var lock_state = '<?php echo language('Locked');?>';
						}else if(limit.indexOf('Locking') != -1){
							var lock_state = '<?php echo language('Locking');?>';
						}else{
							var lock_state = '<?php echo language('Unlocked');?>';
						}
						
						if(limit.indexOf('Marked') != -1){
							var mark_state = '<?php echo language('Marked');?>';
						}else{
							var mark_state = '<?php echo language('Unmarked');?>';
						}
						
						if(limit.indexOf('Sms') != -1){
							var sms_state = '<?php echo language('Limited');?>';
						}else{
							var sms_state = '<?php echo language('Unlimited');?>';
						}
						$("#gsm_"+key1+"-showlimit span:eq(00)").html(call_limit);
						$("#gsm_"+key1+"-showlimit span:eq(01)").html(lock_state);
						$("#gsm_"+key1+"-showlimit span:eq(02)").html(mark_state);
						$("#gsm_"+key1+"-showlimit span:eq(03)").html(sms_state);
					}
				}
			}

		};
		
		function clean_gsm_info(board) {
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
				var n = 1;
			<?php }else{ ?>
				var n = 0;
			<?php }?>
			
			for(var gsm_num = 1 ;gsm_num <=<?php echo $__GSM_SUM__;?>; gsm_num++) {
				$("#gsm_"+board+"_"+gsm_num+" td:eq("+(2+n)+")").html('');
				$("#gsm_"+board+"_"+gsm_num+" td:eq("+(3+n)+")").html('');
				$("#gsm_"+board+"_"+gsm_num+" td:eq("+(4+n)+")").html('');
				$("#gsm_"+board+"_"+gsm_num+" td:eq("+(5+n)+")").html('');
				$("#gsm_"+board+"_"+gsm_num+" td:eq("+(6+n)+")").html('');
				$("#gsm_"+board+"_"+gsm_num+" td:eq("+(7+n)+")").html('');
				$("#gsm_"+board+"_"+gsm_num+" td:eq("+(8+n)+")").html('');
				$("#gsm_"+board+"_"+gsm_num+" td:eq("+(9+n)+")").html('');
				$("#gsm_"+board+"_"+gsm_num+" td:eq("+(10+n)+")").html('');
				$("#gsm_"+board+"_"+gsm_num+" td:eq("+(11+n)+")").html('');
				$("#gsm_"+board+"_"+gsm_num+"-showdetail span:eq(00)").html('');
				$("#gsm_"+board+"_"+gsm_num+"-showdetail span:eq(01)").html('');
				$("#gsm_"+board+"_"+gsm_num+"-showdetail span:eq(02)").html('');
				$("#gsm_"+board+"_"+gsm_num+"-showdetail span:eq(03)").html('');
				$("#gsm_"+board+"_"+gsm_num+"-showdetail span:eq(04)").html('');
				$("#gsm_"+board+"_"+gsm_num+"-showdetail span:eq(05)").html('');
				$("#gsm_"+board+"_"+gsm_num+"-showdetail span:eq(06)").html('');
				$("#gsm_"+board+"_"+gsm_num+"-showdetail span:eq(07)").html('');
				$("#gsm_"+board+"_"+gsm_num+"-showdetail span:eq(09)").html('');
				$("#gsm_"+board+"_"+gsm_num+"-showdetail span:eq(10)").html('');
				$("#gsm_"+board+"_"+gsm_num+"-showdetail span:eq(11)").html('');
				$("#gsm_"+board+"_"+gsm_num+"-showdetail span:eq(12)").html('');
				$("#gsm_"+board+"_"+gsm_num+"-showdetail span:eq(13)").html('');
				$("#gsm_"+gsm_num+"-showlimit span:eq(00)").html('');
				$("#gsm_"+gsm_num+"-showlimit span:eq(01)").html('');
				$("#gsm_"+gsm_num+"-showlimit span:eq(02)").html('');
				$("#gsm_"+gsm_num+"-showlimit span:eq(03)").html('');
			}

		};

	};

	</script>

	<br>
<?php
}
?>

<?php
function show_routings()
{
	$all_routings = get_all_routings(true);
?>
	<div class="content">
		<span class="title"><?php echo language('Routing Information');?></span>
		
		<table class="table_show">
			<tr>
				<th><?php echo language('Rule Name');?></th>
				<th><?php echo language('From');?></th>
				<th><?php echo language('To');?></th>
				<th><?php echo language('Rules');?></th>
			</tr>
			
			<?php
			if($all_routings) {
				foreach($all_routings as $routing) {
					$routing_name = trim($routing['routing_name']);
					$from_channel = trim($routing['from_channel']);	
					$to_channel = trim($routing['to_channel']);
					
					//if exist, get custom context
					$custom_context = '';
					if (isset($routing['custom_context'])) {
						$custom_context = trim($routing['custom_context']);
					} else {
						$custom_context = '';
					}
					
					if($from_channel == 'anysip') {
						$show_from_channel = language('Any SIP');
					} else {
						$show_from_channel = get_channel_name($from_channel);
					}
					
					$show_to_channel = '';
					if ($to_channel == 'custom') {
						$show_to_channel = language('custom-'.$custom_context);
					} else {
						$output = explode(',',$to_channel);
						foreach($output as $ch) {
							$show_to_channel .= get_channel_name($ch).' ';
						}
					}

					$show_rules='';
					$dp = parse_dial_pattern($routing['dial_pattern']);
					if($dp) {
						$show_rules .= 'Dial_pattern<br/>';
						foreach($dp as $each) {
							$t = '('.$each['prepend'].')+'.$each['prefix'].'|['.$each['pattern'].'/'.$each['cid'].']<br/>';
							$show_rules .= $t;
						}
					}
					$tp = parse_time_pattern($routing['time_pattern']);
					if($tp) {
						$show_rules .= 'Time_pattern<br/>';
						foreach($tp as $each) {
							$t = $each['stime'].'-'.$each['etime'].'&'.$each['sweek'].'-'.$each['eweek'].'&'.$each['sday'].'-'.$each['eday'].'&'.$each['smonth'].'-'.$each['emonth'].'<br/>'; 
							$show_rules .= $t;
						}
					}

			?>
				<tr>
					<td>
						<?php echo $routing_name ?>
					</td>
					<td>
						<?php echo $show_from_channel ?>
					</td>
					<td>
						<?php echo $show_to_channel ?>
					</td>
					<td>
						<?php echo $show_rules ?>
					</td>
				</tr>
			<?php
				}
			}
			?>
		</table>
	</div>
	
	<br>
<?php
}
?>


<?php
function show_networks()
{
?>
	<div class="content">
		<span class="title"><?php echo language('Network Information');?></span>
		
		<table class="table_show">
			<tr>
				<th><?php echo language('Name');?></th>
				<th><?php echo language('MAC Address');?></th>
				<th><?php echo language('IPV4 Address');?></th>
				<th><?php echo language('IPV6 Address');?></th>
				<th><?php echo language('Mask');?></th>
				<th><?php echo language('Gateway');?></th>
				<th><?php echo language('RX Packets');?></th>
				<th><?php echo language('TX Packets');?></th>
			</tr>
			
		<?php
			unset($output);
			exec("/my_tools/net_tool eth0 2> /dev/null && echo ok",$output);
			if(isset($output[11]) && $output[11] == 'ok' && isset($output[1]) && $output[1] == 'Enable') {
		?>
				<tr >
					<td>LAN</td>
					<td>
						<?php if(isset($output[2])) echo $output[2]; ?>								
					</td>
					<td>
						<?php if(isset($output[3])) echo $output[3]; ?>								
					</td>
					<td>
					<?php 
						exec("ifconfig eth0 |grep \"inet6.*Link\"|awk '{print $3}'",$ipv6_eth0_output);
						echo $ipv6_eth0_output[0];
					?>
					</td>
					<td>
						<?php if(isset($output[5])) echo $output[5]; ?>								
					</td> 
					<td>
						<?php if(isset($output[6])) echo $output[6]; ?>								
					</td>
					<td>
						<?php if(isset($output[8])) echo $output[8]; ?>								
					</td>
					<td>
						<?php if(isset($output[10])) echo $output[10]; ?>			
					</td>
				</tr>
		<?php
			}
		?>

		<?php
			unset($output);
			exec("/my_tools/net_tool eth1 2> /dev/null && echo ok",$output);
			if(isset($output[11]) && $output[11] == 'ok' && isset($output[1]) && $output[1] == 'Enable') {
		?>
				<tr>
					<td>WAN</td>
					<td>
						<?php if(isset($output[2])) echo $output[2]; ?>								
					</td>
					<td>
						<?php if(isset($output[3])) echo $output[3]; ?>								
					</td>
					<td>
					<?php 
						exec("ifconfig eth1 |grep \"inet6.*Link\"|awk '{print $3}'",$ipv6_eth1_output);
						echo $ipv6_eth1_output[0];
					?>
					</td>
					<td>
						<?php if(isset($output[5])) echo $output[5]; ?>								
					</td>
					<td>
						<?php if(isset($output[6])) echo $output[6]; ?>								
					</td>
					<td>
						<?php if(isset($output[8])) echo $output[8]; ?>								
					</td>
					<td>
						<?php if(isset($output[10])) echo $output[10]; ?>								
					</td>
				</tr>
		<?php
			}
		?>
		</table>
	</div>
	
	<br>
<?php
}
?>

<?php
function show_ppp30()
{
	unset($output);

	exec("/my_tools/net_tool ppp30 2> /dev/null && echo ok", $output);
	if(isset($output[11]) && $output[11] == 'ok' && isset($output[1]) && $output[1] == 'Enable') {
?>
	<div class="content">
		<span class="title"><?php echo language('VPN Information');?></span>
		
		<table class="table_show">
			<tr>
				<th><?php echo language('Name');?></th>
				<th><?php echo language('IP Address');?></th>
				<th><?php echo language('P-t-P IP Address');?></th>
				<th><?php echo language('Mask');?></th>
				<th><?php echo language('RX Packets');?></th>
				<th><?php echo language('TX Packets');?></th>
			</tr>
			<tr>
				<td>L2TP VPN</td>
				<td>
					<?php if(isset($output[2])) echo $output[3]; ?>								
				</td>
				<td>
					<?php 
						$p_t_P =`ifconfig ppp30 | awk '/inet/ {FS=" "; gsub("P-t-P:","" ,$3); print $3}'`; 
						echo $p_t_P;
					?>								
				</td>
				<td>
					<?php if(isset($output[6])) echo $output[5]; ?>								
				</td>
				<td>
					<?php if(isset($output[8])) echo $output[8]; ?>								
				</td>
				<td>
					<?php if(isset($output[10])) echo $output[10]; ?>								
				</td>
			</tr>
		</table>
	</div>

	<br>
<?php
	}
}

function show_pppoe()
{
	unset($output);

	exec("/my_tools/net_tool ppp0 2> /dev/null && echo ok", $output);
	if(isset($output[11]) && $output[11] == 'ok' && isset($output[1]) && $output[1] == 'Enable') {
?>
	<div class="content">
		<span class="title"><?php echo language('VPN Information');?></span>
		
		<table class="table_show">
			<tr>
				<th><?php echo language('Name');?></th>
				<th><?php echo language('IP Address');?></th>
				<th><?php echo language('P-t-P IP Address');?></th>
				<th><?php echo language('Mask');?></th>
				<th><?php echo language('RX Packets');?></th>
				<th><?php echo language('TX Packets');?></th>
			</tr>
			<tr>
				<td>PPTP</td>
				<td>
					<?php if(isset($output[2])) echo $output[3]; ?>								
				</td>
				<td>
					<?php 
						$p_t_P =`ifconfig ppp0 | awk '/inet/ {FS=" "; gsub("P-t-P:","" ,$3); print $3}'`; 
						echo $p_t_P;
					?>								
				</td>
				<td>
					<?php if(isset($output[6])) echo $output[5]; ?>								
				</td>
				<td>
					<?php if(isset($output[8])) echo $output[8]; ?>								
				</td>
				<td>
					<?php if(isset($output[10])) echo $output[10]; ?>								
				</td>
			</tr>
		</table>
	</div>

	<br>
<?php
	}
}

function show_tun0()
{
	unset($output);

	exec("/my_tools/net_tool tun0 2> /dev/null && echo ok", $output);
	if(isset($output[11]) && $output[11] == 'ok' && isset($output[1]) && $output[1] == 'Enable') {
?>
	<div class="content">
		<span class="title"><?php echo language('VPN Information');?></span>
		
		<table class="table_show">
			<tr>
				<th><?php echo language('Name');?></th>
				<th><?php echo language('IP Address');?></th>
				<th><?php echo language('P-t-P IP Address');?></th>
				<th><?php echo language('Mask');?></th>
				<th><?php echo language('RX Packets');?></th>
				<th><?php echo language('TX Packets');?></th>
			</tr>
			<tr>
				<td>OPENVPN</td>
				<td>
					<?php if(isset($output[2])) echo $output[3]; ?>								
				</td>
				<td>
					<?php 
						$p_t_P =`ifconfig tun0 | awk '/inet/ {FS=" "; gsub("P-t-P:","" ,$3); print $3}'`; 
						echo $p_t_P;
					?>								
				</td>
				<td>
					<?php if(isset($output[6])) echo $output[5]; ?>								
				</td>
				<td>
					<?php if(isset($output[8])) echo $output[8]; ?>								
				</td>
				<td>
					<?php if(isset($output[10])) echo $output[10]; ?>								
				</td>
			</tr>
		</table>
	</div>

	<br>
<?php
	}
}

function show_n2n()
{
	unset($output);

	exec("/my_tools/net_tool edge0 2> /dev/null && echo ok", $output);
	if(isset($output[11]) && $output[11] == 'ok' && isset($output[1]) && $output[1] == 'Enable') {
?>
	<div class="content">
		<span class="title"><?php echo language('VPN Information');?></span>
		
		<table class="table_show">
			<tr>
				<th><?php echo language('Name');?></th>
				<th><?php echo language('IP Address');?></th>
				<th><?php echo language('Mask');?></th>
				<th><?php echo language('RX Packets');?></th>
				<th><?php echo language('TX Packets');?></th>
			</tr>
			<tr>
				<td>N2N</td>
				<td><?php if(isset($output[2])) echo $output[3]; ?></td>
				<td><?php if(isset($output[6])) echo $output[5]; ?></td>
				<td><?php if(isset($output[8])) echo $output[8]; ?></td>
				<td><?php if(isset($output[10])) echo $output[10]; ?></td>
			</tr>
		</table>
	</div>

	<br>
<?php
	}
}

	session_start();

	//dynamic_show_warning();
	show_gsms();
	show_sips();
	
	if($_SESSION['id'] == 1){
		show_iaxs();
	}
	
	show_routings();
	show_networks();
	show_pppoe();
	show_ppp30();
	show_tun0();
	show_n2n();
?>

<script type="text/javascript"> 
$(document).ready(function (){ 
	dynamic_show_gsm();
}); 
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>
