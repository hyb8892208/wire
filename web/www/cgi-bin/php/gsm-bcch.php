<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
?>

<?php
function show_cell_info() {
	//$alldata = get_all_gsm_info();
	global $__GSM_SUM__;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $__GSM_HEAD__;
	global $__MODULE_HEAD_ARRAY__;
	global $__deal_cluster__;
	
	if($__deal_cluster__){
		$cluster_info = get_cluster_info();
	}
	
	function getModeInfo($channel,$slaveip,$board)
	{
		global $__GSM_HEAD__;
		global $__MODULE_HEAD_ARRAY__;
		if ($__MODULE_HEAD_ARRAY__[$board][$channel] != $__GSM_HEAD__) {
			$mode = '';
			return $mode;
		}
		$mode = 'default';
		$aql = new aql();
		if($slaveip == '') {
			$aql->set('basedir','/etc/asterisk');
			if(file_exists('/etc/asterisk/gw_gsm.conf')) {
				$res = $aql->query("select * from gw_gsm.conf where section='$channel'");
			}
		} else {
			if($__deal_cluster__){
				get_slave_file($slaveip,'/etc/asterisk/gw_gsm.conf');
				$aql->set('basedir','/tmp');
				if(file_exists("/tmp/$slaveip-gw_gsm.conf")) {
					$res = $aql->query("select * from $slaveip-gw_gsm.conf where section='$channel'");
				}
			}
		}
		if(isset($res[$channel]['bcch_type'])) $mode = $res[$channel]['bcch_type'];
		
		return $mode;
	}
	
	//phone number switch
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$phonenum_res = $aql->query("select * from sim_query.conf");
?>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<input type="hidden" name="send" id="send" value="" />
	<input type="hidden" name="sel_gsm" id="sel_gsm" value="" />
	<input type="hidden" name="sel_brd" id="sel_brd" value="" />
	<input type="hidden" name="sel_slaveip" id="sel_slaveip" value="" />

	<table width="100%" class="tshow" id="<?php echo "gsm_table"; ?>">
		<tr align="center">
			<td></td>
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			<td></td>
			<?php } ?>
			
			<td></td>
			<?php for($i = 0 ; $i <= 6 ; $i++) {?>
				<td colspan="3"><?php echo $i ?></td>
			<?php } ?>
			<td></td>
			<td></td>
		</tr>
	
		<th><?php echo language('Port');?></th>
		
		<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
		<th><?php echo language('Mobile Number');?></th>
		<?php } ?>
		
		<th><?php echo language('Mode');?></th>
		<?php for($i = 0 ; $i <= 6 ; $i++) {?>
			<th><?php echo language('LAC');?></th>
			<th><?php echo language('BCCH');?></th>
			<th><?php echo language('dbm');?></th>
		<?php } ?>
		<th><?php echo language('Status');?></th>
		<th><?php echo language('Detail');?></th>
	
<?php
		
		for ($c = 1 ; $c <= $__GSM_SUM__ ; $c++) {
			$channel_module_name = get_gsm_name_by_channel($c);
			$res  = explode('-', $channel_module_name, 2);
			$channel_module_type = $res[0];
			if($channel_module_type == 'lte' || $channel_module_type == 'null' || $channel_module_type == 'cdma' || $channel_module_type == 'umts') continue;
			
			$phonenum = '';
			if(($phonenum_res[$c]['query_type']&240) != 0){
				exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $c",$phone_output);
				$phonenum = $phone_output[0];
				$phone_output = '';
			}
?>
			<tr>
				<td><?php echo get_gsm_name_by_channel($c); ?></td>
				
				<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
				<td><?php echo $phonenum;?></td>
				<?php } ?>
				
				<td><?php echo getModeInfo($c,'',1); ?></td>
				<?php for($j = 0 ; $j <= 6 ; $j++)  { ?>
					<td></td>
					<td></td>
					<td></td>
				<?php } ?>
				<td></td>
				<td>
					<input type="hidden" name="<?php echo "data_1_{$c}" ?>" id="<?php echo "data_1_{$c}" ?>" value="" />
					<input type="submit" value="Detail" id="<?php echo "gsm_btn_1_{$c}" ?>" onclick="document.getElementById('send').value='Detail';setValue('<?php echo $c; ?>', '', '1'); return true;" />
				</td>
			</tr>
<?php 
		}
?>

<?php
	if($__deal_cluster__){
		if($cluster_info['mode'] == 'master') {
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				if(isset($cluster_info[$__BRD_HEAD__.$b.'_ip']) && $cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					$cluster_ip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
					for($c=1; $c<=$__GSM_SUM__; $c++) {
						$channel_module_name = get_gsm_name_by_channel($c, $b);
						$res  = explode('-', $channel_module_name, 2);
						$channel_module_type = $res[0];
						if($channel_module_type == 'lte' || $channel_module_type == 'null') continue;
						if($channel_module_type !== 'cdma' ) {
?>
						<tr>
							<td><?php echo get_gsm_name_by_channel($c,$b); ?><td>
							<td><?php echo getModeInfo($c,$cluster_ip,$b); ?></td>
							<?php for($j = 0 ; $j <= 6 ; $j++)  { ?>
								<td></td>
								<td></td>
								<td></td>
							<?php } ?>
							<td></td>
							<td>
								<input type="hidden" name="<?php echo "data_{$b}_{$c}" ?>" id="<?php echo "data_{$b}_{$c}" ?>" value="" />
								<input type="submit" value="Detail" id="<?php echo "gsm_btn_{$b}_{$c}" ?>" onclick="document.getElementById('send').value='Detail';setValue('<?php echo $c; ?>', '<?php echo $cluster_ip; ?>', '<?php echo $b; ?>'); return true;" />
							</td>
						</tr>
<?php
						}
					}
				}
			}
		}
	}
?>
	</table>
	<br />
	<table>
		<tr> 
			<td><button onclick="getAllCellInfo();" id="getCurrent"><?php echo language('Get Current State'); ?></button></td>
			<td><button onclick="searchAllCellInfo();" id="searchCell"><?php echo language('Search Cell'); ?></button></td>
			<td width="" align="center"><span id="get_status_span" style="display:inline-block;margin-left:50px;"></span></td>
		</tr>
	</table>
	</form>
	<script type="text/javascript">
	function setValue(channel,slaveip,board)
	{
		document.getElementById('sel_gsm').value = channel;
		document.getElementById('sel_slaveip').value = slaveip;
		document.getElementById('sel_brd').value = board;
	}

	function showCellInfo(data,board,port,reset)
	{
		<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			var n = 1;
		<?php }else{ ?>
			var n = 0;
		<?php }?>
		
		var tr_num = (parseInt(board) - 1) * <?php echo $__GSM_SUM__;?> + parseInt(port) + 1;
		var td_num = 2;
		var jsonlength = 0;
		for(var key1 in data){
			for(var key2 in data[key1]){
				
				$("#gsm_table tr:eq("+tr_num+") td:eq("+td_num+")").html(data[key1][key2].lac);
				td_num++;
				$("#gsm_table tr:eq("+tr_num+") td:eq("+td_num+")").html(data[key1][key2].arfcn);
				td_num++;
				$("#gsm_table tr:eq("+tr_num+") td:eq("+td_num+")").html(data[key1][key2].rxl);
				td_num++;
				jsonlength++;
			}
		}
		if (jsonlength > 0) {
			$("#gsm_table tr:eq("+tr_num+") td:eq("+(23+n)+")").html('SUCCESS');
			document.getElementById('data_' + board +'_' + port).value = JSON.stringify(data);
		}
		else {
			$("#gsm_table tr:eq("+tr_num+") td:eq("+(23+n)+")").html('UNKNOWN');
		}
	}

	function getAllCellInfo()
	{
		query_num = 0;
		check_num = 0;
		$("#getCurrent,#searchCell").attr("disabled","disabled");
<?php
		
		for($i=1; $i <= $__GSM_SUM__; $i++) { 
			if ($__MODULE_HEAD_ARRAY__[1][$i] == $__GSM_HEAD__) {
				echo "getCellInfo($i, 1);\n";
				echo "check_num++;\n";
			}
		}

		if($__deal_cluster__){
			if($cluster_info['mode'] == 'master') {
				for($b=2; $b<=$__BRD_SUM__; $b++) {
					if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
						for($i=1; $i <= $__GSM_SUM__; $i++) {
							if ($__MODULE_HEAD_ARRAY__[$b][$i] == $__GSM_HEAD__) {
								echo "getCellInfo($i, $b);\n";
								echo "check_num++;\n";
							}
						}
					}
				}
			}
		}
?>		
		function getCellInfo(port, board)
		{
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
				var n = 1;
			<?php }else{ ?>
				var n = 0;
			<?php }?>
			
			var tr_num = (parseInt(board) - 1) * <?php echo $__GSM_SUM__;?> + parseInt(port) + 1;
			var url = "";
			var times = <?php if(isset($_GET['timeout'])) echo $_GET['timeout'];else echo 1;?>;
			var timeout = 10000 * times;
			if( board == 1 ){
				url = "/service?action=get_curcellsinfo&span="+port+"&timeout="+timeout+"&random="+Math.random();
			}else{
				url = "/"+board+"/service?action=get_curcellsinfo&span="+port+"&timeout="+timeout+"&random="+Math.random();
			}
			$("#get_status_span").html("<img src='/images/mini_loading.gif' />");
			$("#gsm_table tr:eq("+tr_num+") td:eq("+(23+n)+")").html('QUERYING...');
			
			var a = $.ajax({
				"type"		:	"GET",
				"url"		:	url,
				"dataType"	:	"json",
				"success"	:	function(data){			
					if( data ){			
						showCellInfo(data,board,port);
					}
					else {
						$("#gsm_table tr:eq("+tr_num+") td:eq("+(23+n)+")").html('UNKNOWN');
					}
				},
				"complete"	:	function(com,data){
					data = $.trim(data);
					query_num++;
					
					if( data == "parsererror" ){
						$("#gsm_table tr:eq("+tr_num+") td:eq("+(23+n)+")").html('UNKNOWN');
						//$("#get_status_span").html("<span style='color:red;'>Get Current Status Failed.</span>");
					} else if( data == 'error' ){
						$("#gsm_table tr:eq("+tr_num+") td:eq("+(23+n)+")").html('UNKNOWN');
						//$("#get_status_span").html("<span style='color:red;'>Get Current Status Failed.</span>");
					}
					var responStr = $.trim(com.responseText);
					if (responStr == 'NOT READY') {
						
						$("#gsm_table tr:eq("+tr_num+") td:eq("+(23+n)+")").html('NOT READY');
					}
					
					if (query_num == check_num) {
						$("#getCurrent,#searchCell").removeAttr("disabled");
						$("#get_status_span").empty();
					}
				}
			});
		}
	}

	function searchAllCellInfo()
	{
		query_num = 0;
		check_num = 0;
		$("#getCurrent,#searchCell").attr("disabled","disabled");
<?php
		for($i=1; $i <= $__GSM_SUM__; $i++) {
			if($__MODULE_HEAD_ARRAY__[1][$i] == $__GSM_HEAD__){
				echo "searchCellInfo($i, 1);";
				echo "check_num++;\n";
			}
		}

		if($__deal_cluster__){
			if($cluster_info['mode'] == 'master') {
				for($b=2; $b<=$__BRD_SUM__; $b++) {
					if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
						for($i=1; $i <= $__GSM_SUM__; $i++) {
							if($__MODULE_HEAD_ARRAY__[$b][$i] == $__GSM_HEAD__){
								echo "searchCellInfo($i, $b);";
								echo "check_num++;\n";
							}
						}
					}
				}
			}
		}
?>	
		function searchCellInfo(port, board)
		{			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
				var n = 1;
			<?php }else{ ?>
				var n = 0;
			<?php }?>
		
			var tr_num = (parseInt(board) - 1) * <?php echo $__GSM_SUM__;?> + parseInt(port) + 1;	
			var url   = '';
			var times = <?php if(isset($_GET['timeout'])) echo $_GET['timeout'];else echo 1;?>;
			var timeout = 10000 * times;
			if( board == 1 ){
				url = "/service?action=get_curcellsinfo&span="+port+"&timeout="+timeout+"&reset=yes&random="+Math.random();
			}else{
				url = "/"+board+"/service?action=get_curcellsinfo&span="+port+"&timeout="+timeout+"&reset=yes&random="+Math.random();
			}
			
			$("#get_status_span").html("<img src='/images/mini_loading.gif' />");
			$("#gsm_table tr:eq("+tr_num+") td:eq("+(23+n)+")").html('QUERYING...');
			
			$.ajax({
				"type"		:	"GET",
				"url"		:	url,
				"dataType"	:	"json",
				"success"	:	function(data){			
					if( data ){			
						showCellInfo(data,board,port);
					}
					else {
						$("#gsm_table tr:eq("+tr_num+") td:eq("+(23+n)+")").html('UNKNOWN');
					}
				},
				"complete"	:	function(com,data){
					data = $.trim(data);
					query_num++;
					if( data == "parsererror" ){
						$("#gsm_table tr:eq("+tr_num+") td:eq("+(23+n)+")").html('UNKNOWN');
						//$("#get_status_span").html("<span style='color:red;'>Get Current Status Failed.</span>");
					} else if( data == 'error' ){
						$("#gsm_table tr:eq("+tr_num+") td:eq("+(23+n)+")").html('UNKNOWN');
						//$("#get_status_span").html("<span style='color:red;'>Get Current Status Failed.</span>");
					}
					
					var responStr = $.trim(com.responseText);
					if (responStr == 'NOT READY') {
						
						$("#gsm_table tr:eq("+tr_num+") td:eq("+(23+n)+")").html('NOT READY');
					}
										
					if (query_num == check_num) {
						$("#getCurrent,#searchCell").removeAttr("disabled");
						$("#get_status_span").empty();
					}
				}
			});
		}
	}
	
	$(document).ready(function (){
		 //getAllCellInfo();
	});
	</script>
<?php 
}
?>

<?php
function show_detail_info($channel, $board, $slaveip) {
	global $__GSM_SUM__;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $__GSM_HEAD__;
	global $__MODULE_HEAD_ARRAY__;
	global $__deal_cluster__;
	
	if($__deal_cluster__){
		$cluster_info = get_cluster_info();
	}
	
	$aql = new aql();
	if($slaveip == '') {
		$aql->set('basedir','/etc/asterisk');
		$res = $aql->query("select * from gw_gsm.conf where section='$channel'");
	} else {
		get_slave_file($slaveip,'/etc/asterisk/gw_gsm.conf');
		$aql->set('basedir','/tmp');
		$res = $aql->query("select * from $slaveip-gw_gsm.conf where section='$channel'");
	}
	
	$bcch_type = '';
	$bcch_range = '';
	$bcch_fixed = '';
	$bcch_calls = '';
	$bcch_faileds = '';
	$bcch_asr = '';
	$bcch_incalls = '';
	$bcch_minrxl = '';
	
	if(isset($res[$channel]['bcch_type'])) $bcch_type = $res[$channel]['bcch_type'];
	if(isset($res[$channel]['bcch_range'])) $bcch_range = $res[$channel]['bcch_range'];
	if(isset($res[$channel]['bcch_fixed'])) $bcch_fixed = $res[$channel]['bcch_fixed'];
	if(isset($res[$channel]['bcch_calls'])) $bcch_calls = $res[$channel]['bcch_calls'];
	if(isset($res[$channel]['bcch_faileds'])) $bcch_faileds = $res[$channel]['bcch_faileds'];
	if(isset($res[$channel]['bcch_asr'])) $bcch_asr = $res[$channel]['bcch_asr'];
	if(isset($res[$channel]['bcch_incalls'])) $bcch_incalls = $res[$channel]['bcch_incalls'];	
	if(isset($res[$channel]['bcch_minrxl'])) $bcch_minrxl = $res[$channel]['bcch_minrxl'];
?>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<input type="hidden" name="send" id="send" value="" />
	<input type="hidden" name="sel_gsm" id="sel_gsm" value="" />
	<input type="hidden" name="sel_brd" id="sel_brd" value="" />
	<input type="hidden" name="sel_fixed" id="sel_fixed" value="" />
	<input type="hidden" name="sel_slaveip" id="sel_slaveip" value="<?php if (isset($cluster_info[$__BRD_HEAD__.$board.'_ip'])) echo $cluster_info[$__BRD_HEAD__.$board.'_ip'];?>" />
	<div style="width:100%;height:30px">
		<div id="tab" >
			<li class="tb1">&nbsp;</li>
			<li class="tbg">
				<?php echo get_gsm_name_by_channel($channel,$board); ?>
			</li>
			<li class="tb2">&nbsp;</li>
		</div>
	</div>

	<div class="div_tab" id="tab_detail_div">
		<div class="div_tab_th">
			<div class="helptooltips">
				<div class="div_tab_text"><?php echo language('Port');?>:</div>
				<span class="showhelp">
				<?php
					$help = 'Port';
					echo language('Port',$help);
				?>
				</span>
			</div>
		</div>
		<div class="div_tab_td"> 
			<select size=1 name="port_select" id="port_select" onchange="onChangePort()">
<?php
				for($i=1; $i <= $__GSM_SUM__; $i++) {
					if($__MODULE_HEAD_ARRAY__[1][$i] == $__GSM_HEAD__){
						$selected = (1 == $board && $i == $channel) ? 'selected' : '';
						echo "<option value=\"1_{$i}\" $selected>";
						echo get_gsm_name_by_channel($i);
						echo "</option>\n";
					}
				}

				if($__deal_cluster__){
					if($cluster_info['mode'] == 'master') {
						for($b=2; $b<=$__BRD_SUM__; $b++) {
							if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
								for($i=1; $i <= $__GSM_SUM__; $i++) {
									if($__MODULE_HEAD_ARRAY__[$b][$i] == $__GSM_HEAD__){
										$selected = ($b == $board && $i == $channel) ? 'selected' : '';
										echo "<option value=\"{$b}_{$i}\" $selected>";
										echo get_gsm_name_by_channel($i,$b);
										echo "</option>\n";
									}
								}
							}
						}
					}
				}
?>
			</select>
		</div>

		<div class="div_tab_th">
		<div class="helptooltips">
			<div class="div_tab_text"><?php echo language('BCCH Mode');?>:</div>
			<span class="showhelp">
			<?php
				$help = "BCCH Mode";
				echo language('BCCH Mode help',$help);
			?>
			</span>
		</div>
		</div>

		<div class="div_tab_td">
		<select size=1 name="bcch_type" id="bcch_type" onchange="onChangeBcchMode();">		  
			<option id="BcchMode0" value="default">Default</option>
			<option id="BcchMode1" value="fixed">Fixed</option>
			<option id="BcchMode2" value="random">Random</option>
			<option id="BcchMode3" value="advanced">Advanced</option>
		</select>
		</div>

		<div class='div_tab_hide' id='bcch_minrxl_div'>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Minimum Signal Strength allow');?>:</div>
					<span class="showhelp">
					<?php
						$help = "Minimum Signal Strength allow";
						echo language('Minimum Signal Strength allow help',$help);
					?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input class="text" id="signal" maxlength="4" name="signal" size="18" value="<?php echo ($bcch_minrxl == '' ? '10' : $bcch_minrxl) ?>">
				<span id="csignal"></span>
			</div>
		</div>

		<div class='div_tab_hide' id='random_bcch_div'>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Auto Period');?>:</div>
					<span class="showhelp">
					<?php
						$help = "Auto Period";
						echo language('Auto Period help',$help);
					?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input class="text" id="timemin" maxlength="3" name="timemin" size="8" value="1">&nbsp; - &nbsp;
				<input class="text" id="timemax" maxlength="3" name="timemax" size="8" value="15">&nbsp;min
				<span id="auto_period"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Switch BCCH in Calling');?>:</div>
					<span class="showhelp">
					<?php
						$help = "Switch BCCH in Calling";
						echo language('Switch BCCH in Calling help',$help);
					?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
			   <input id="TalkEnable" name="TalkEnable" type="radio" value="0" <?php  if ($bcch_incalls == 'no' || $bcch_incalls == '') echo "checked=\"\""; ?> >No
			   <input id="TalkEnable" name="TalkEnable" type="radio" value="1" <?php  if ($bcch_incalls == 'yes') echo "checked=\"\""; ?> >Yes
			</div>
		</div>
	
		<div class='div_tab_hide' id='advance_bcch_div'>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Call times');?>:</div>
					<span class="showhelp">
					<?php
						$help = "Call times";
						echo language('Call times help',$help);
					?>
					</span>			
				</div>
			</div>
			<div class="div_tab_td">
				<input class="text" id="call_times" maxlength="2" name="call_times" size="8" value="<?php echo ($bcch_calls == '' ? 15 : $bcch_calls) ?>">
				<span id="ccall_times"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Minimun ASR');?>:</div>
					<span class="showhelp">
					<?php
						$help = "Minimun call ASR";
						echo language('Minimun call ASR',$help);
					?>
					</span>			
				</div>
			</div>
			<div class="div_tab_td">
				<input class="text" id="call_asr" maxlength="2" name="call_asr" size="8" value="<?php echo ($bcch_asr == '' ? 30 : $bcch_asr) ?>">&nbsp;%
				<span id="ccall_asr"></span>
			</div>		
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Call failed times');?>:</div>
					<span class="showhelp">
					<?php
						 $help = "Call failed times";
						 echo language('Call failed times',$help);
						 ?>
					</span>			
				</div>
			</div>
			<div class="div_tab_td">
				<input class="text" id="call_failed_times" maxlength="2" name="call_failed_times" size="8" value="<?php echo ($bcch_faileds == '' ? 6 : $bcch_faileds) ?>">
				<span id="ccall_failed_times"></span>
			</div>
		</div>
	
		<div class='div_tab_hide' id='apply_to_all_div'>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Apply To All Ports');?>:</div>
					<span class="showhelp">
					<?php
						$help = "Apply To All Ports";
						echo language('Apply To All Ports help',$help);
					?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
			   <input id="apply_to_all" name="apply_to_all" type="radio" value="0" checked="">No
			   <input id="apply_to_all" name="apply_to_all" type="radio" value="1" >Yes
			</div>
		</div>	
	</div>
	<br />
	<table width="100%" class="tshow" id="<?php echo "detail_table"; ?>">
		<tr>
			<th><?php echo language('Index');?></th>
			<th><?php echo language('MCC');?></th>
			<th><?php echo language('MNC');?></th>
			<th><?php echo language('LAC');?></th>
			<th><?php echo language('CID');?></th>
			<th><?php echo language('BCCH');?></th>
			<th><?php echo language('Receive Level');?></th>
			<th><?php echo language('Lock');?></th>
		</tr>
		<tr>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
		</tr>
		<tr>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
		</tr>
		<tr>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
		</tr>
		<tr>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
		</tr>
		<tr>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
		</tr>
		<tr>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
		</tr><tr>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
			<td></td>
		</tr>
	</table>
	<br />
	<table id="float_btn" class="float_btn">
		<tr id="float_btn_tr" class="float_btn_tr" style="padding-left: 15px;">
			<td>
				<input type=button id="getCurrent"  value="<?php echo language('Get Current State');?>" onclick="getCurrentCellInfo();" />
			</td>
			<td>
				<input type=button id="searchCell"  value="<?php echo language('Search Cell');?>" onclick="searchCellInfo();" />
			</td>
			<td>
				<input type="submit" id="Save" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>
			</td>	
			<td>
				<input type="submit" id="Apply" value="<?php echo language('Apply');?>" onclick="document.getElementById('send').value='Apply';return check();" />
			</td>
			<td>
				<input type=button id="Cancel"  value="<?php echo language('Cancel');?>" onclick="window.location.href='<?php echo get_self();?>'" />
			</td>
			<td width="" align="center"><span id="get_status_span" style="display:inline-block;margin-left:50px;"></span></td>
		</tr>
	</table>
	</form>
	<script type="text/javascript" src="/js/check.js"></script>
	<script type="text/javascript">
	var fix;
	function checkedLockForFixedMode() {

		if (document.getElementById('bcch_type').selectedIndex == 1) {
			showCheckbox();
			var count = 0;
			for (i = 1; i <= 7; i++) {
				var valbcch = parseInt($("#detail_table tr:eq("+i+") td:eq(5)").html());
				if ( valbcch == fix[0] || valbcch == fix[1] || valbcch == fix[2] ) {
					document.getElementById('bcch_lock_'+i).checked = true;
					count++;
				}
				if (count == 3) {
					break;
				}
			}
		}
	}

	function showDetailCellInfo(data,board,port,reset)
	{
		for(var key1 in data){
			for(var key2 in data[key1]){
				var tr_num = key1;
					tr_num = ++tr_num;
				$("#detail_table tr:eq("+tr_num+") td:eq(0)").html(data[key1][key2].cell);
				$("#detail_table tr:eq("+tr_num+") td:eq(1)").html(data[key1][key2].mcc);
				$("#detail_table tr:eq("+tr_num+") td:eq(2)").html(data[key1][key2].mnc);
				$("#detail_table tr:eq("+tr_num+") td:eq(3)").html(data[key1][key2].lac);
				$("#detail_table tr:eq("+tr_num+") td:eq(4)").html(data[key1][key2].cellid);
				$("#detail_table tr:eq("+tr_num+") td:eq(5)").html(data[key1][key2].arfcn);
				$("#detail_table tr:eq("+tr_num+") td:eq(6)").html(data[key1][key2].rxl);
			}
		}
		checkedLockForFixedMode();
	}

	function getCurrentCellInfo()
	{
		var val   = $("#port_select").val().split("_");  // board_port
		var board = val[0];
		var port  = val[1];
		var times = <?php if(isset($_GET['timeout'])) echo $_GET['timeout'];else echo 1;?>;
		var timeout = 10000 * times;
		$("#detail_table tr td").empty();
		var url = "";
		if( board == 1 ){
			url = "/service?action=get_curcellsinfo&span="+port+"&timeout="+timeout+"&random="+Math.random();
		}else{
			url = "/"+board+"/service?action=get_curcellsinfo&span="+port+"&timeout="+timeout+"&random="+Math.random();
		}
		
		$("#get_status_span").html("<img src='/images/mini_loading.gif' />");
		$("#getCurrent,#searchCell,#Save,#Apply,#Cancel").attr("disabled","disabled");

		$.ajax({
			"type"		:	"GET",
			"url"		:	url,
			"dataType"	:	"json",
			"success"	:	function(data){				
				if( data ){
					showDetailCellInfo(data,board,port);
					$("#get_status_span").empty();
					$("#gsm_table_"+board+"_"+port+" tr td").removeAttr("arfc");
				}else{
					$("#get_status_span").html("<span style='color:red;'>Get Current Status Failed.</span>");
				}
			},
			"complete"	:	function(com,data){
				data = $.trim(data);
				
				if( data == "parsererror" ){
					$("#get_status_span").html("<span style='color:red;'>Get Current Status Failed.</span>");
				}
				
				if( data == 'error' ){
					$("#get_status_span").html("<span style='color:red;'>Get Current Status Failed.</span>");
				}
				var responStr = $.trim(com.responseText);
				if (responStr == 'NOT READY') {
					
					$("#get_status_span").html('NOT READY');
				}
				
				$("#getCurrent,#searchCell,#Save,#Apply,#Cancel").removeAttr("disabled");
			}
		});
	}
	
	function searchCellInfo()
	{
		var val   = $("#port_select").val().split("_");  // board_port
		var board = val[0];
		var port  = val[1];
		var times = <?php if(isset($_GET['timeout'])) echo $_GET['timeout'];else echo 1;?>;
		var timeout = 10000 * times;
		$("#detail_table tr td").empty();
		var url = "";
		if( board == 1 ){
			url = "/service?action=get_curcellsinfo&span="+port+"&timeout="+timeout+"&reset=yes&random="+Math.random();
		}else{
			url = "/"+board+"/service?action=get_curcellsinfo&span="+port+"&timeout="+timeout+"&reset=yes&random="+Math.random();
		}
		
		$("#get_status_span").html("<img src='/images/mini_loading.gif' />");
		$("#getCurrent,#searchCell,#Save,#Apply,#Cancel").attr("disabled","disabled");

		$.ajax({
			"type"		:	"GET",
			"url"		:	url,
			"dataType"	:	"json",
			"success"	:	function(data){				
				if( data ){
					showDetailCellInfo(data,board,port);
					$("#get_status_span").empty();
					$("#gsm_table_"+board+"_"+port+" tr td").removeAttr("arfc");
				}else{
					$("#get_status_span").html("<span style='color:red;'>Get Current Status Failed.</span>");
				}
			},
			"complete"	:	function(com,data){
				data = $.trim(data);
				
				if( data == "parsererror" ){
					$("#get_status_span").html("<span style='color:red;'>Get Current Status Failed.</span>");
				}
				
				if( data == 'error' ){
					$("#get_status_span").html("<span style='color:red;'>Get Current Status Failed.</span>");
				}
				var responStr = $.trim(com.responseText);
				if (responStr == 'NOT READY') {
					
					$("#get_status_span").html('NOT READY');
				}
				
				$("#getCurrent,#searchCell,#Save,#Apply,#Cancel").removeAttr("disabled");
			}
		});
	}
	function onChangePort()
	{
		var gsm_port = $("#port_select").val().split('_');
		var board = gsm_port[0];
		var channel = gsm_port[1];

		document.getElementById('sel_gsm').value = channel;
		document.getElementById('sel_brd').value = board;
		document.getElementById('send').value = 'Detail';
		document.getElementById('sel_slaveip').value = "";
		
		document.forms[0].method = 'POST';
		document.forms[0].submit();
	}

	function hideAllHideClassDiv() 
	{
		$("#apply_to_all_div").hide();
		$("#random_bcch_div").hide();
		$("#advance_bcch_div").hide();
		$("#bcch_minrxl_div").hide();
	}

	function onCheckBcchLock(index)
	{
		var count = 0;
		var strfixed = "";
		var isempty = true;
		for (i = 1; i <= 7; i++) {
			if ($("#bcch_lock_"+i).attr("checked") == "checked") {
				if ($("#detail_table tr:eq("+i+") td:eq(5)").html() != "") {
					isempty = false;
				}
				if (count < 3) {			
					if (strfixed == '') {
						strfixed += parseInt($("#detail_table tr:eq("+i+") td:eq(5)").html());
					}
					else {
						strfixed += ',';
						strfixed += parseInt($("#detail_table tr:eq("+i+") td:eq(5)").html());
					}
				}
				count++;
			}
		}
		if (count > 0) {
			document.getElementById('sel_fixed').value = strfixed;
		}
		if (count > 3) {
			if (index > 0) {
				document.getElementById('bcch_lock_' + index).checked = false;
			}
		}
		if (isempty && count != 0) {
			return -1;
		}

		return count;
	}

	function showCheckbox()
	{
		var strhtml = "";
		for (i = 1; i <= 7; i++) {
			strhtml = "<input type=\"checkbox\" id=\"bcch_lock_"+i+"\" name=\"bcch_lock_"+i+"\" onclick=\"onCheckBcchLock("+i+");\" />";
			$("#detail_table tr:eq("+i+") td:eq(7)").html(strhtml);
		}
	}

	function hideCheckbox()
	{
		var strhtml = "";
		for (i = 1; i <= 7; i++) {
			$("#detail_table tr:eq("+i+") td:eq(7)").html(strhtml);
		}
	}

	function onChangeBcchMode()
	{
		hideAllHideClassDiv();
		hideCheckbox();
		if (document.getElementById('bcch_type').selectedIndex == 0) {
			$("#apply_to_all_div").show();
		}
		else if (document.getElementById('bcch_type').selectedIndex == 1) {
			$("#apply_to_all_div").show();
			checkedLockForFixedMode();
		}
		else if (document.getElementById('bcch_type').selectedIndex == 2) { 
			$("#bcch_minrxl_div").show();
			$("#random_bcch_div").show();
			$("#apply_to_all_div").show();
		}
		else if (document.getElementById('bcch_type').selectedIndex == 3) {
			$("#bcch_minrxl_div").show();
			$("#advance_bcch_div").show();
			$("#apply_to_all_div").show();
		}
	}

	$(document).ready(function (){ 
<?php 
			echo "var bcch_range=\"$bcch_range\";\n";
			echo "var bcch_fixed=\"$bcch_fixed\";\n";
			echo "var bcch_incalls=\"$bcch_incalls\";\n";
?>
			var range = bcch_range.split('-');
<?php
			if ($bcch_type == '')
				$bcch_type = 'default';
			if ($bcch_type == 'default') {
				echo "document.getElementById('bcch_type').selectedIndex = 0;\n";
			}
			elseif ($bcch_type == 'fixed') {
				echo "document.getElementById('bcch_type').selectedIndex = 1;\n";
			}
			elseif ($bcch_type == 'random') {
				echo "document.getElementById('bcch_type').selectedIndex = 2;\n";
			}
			elseif ($bcch_type == 'advanced') {
				echo "document.getElementById('bcch_type').selectedIndex = 3;\n";
			}
?>
		if(range[0]) {
			document.getElementById('timemin').value=range[0];
		}
		if(range[1]) {
			document.getElementById('timemax').value=range[1];		
		}

		fix = bcch_fixed.split(',');
		document.getElementById('sel_fixed').value = bcch_fixed;
		
		var gsm_port = $("#port_select").val().split('_');
		var board = gsm_port[0];
		var channel = gsm_port[1];
		document.getElementById('sel_gsm').value = channel;
		document.getElementById('sel_brd').value = board;	
<?php
		echo "board = " . $board . ";\n";
		echo "port = " . $channel . ";\n";
		$data_name = 'data_' .$board . '_' . $channel;
		if(isset($_POST[$data_name]) && trim($_POST[$data_name]) != '' ) {
			echo "data = '" . $_POST[$data_name] . "';";
			echo 'obj_data = JSON.parse(data);';
			echo 'showDetailCellInfo(obj_data,board,port);';
		}
?>
		onChangeBcchMode();	
	});
	
	function check_integer(ss)
	{
		var type="^[0-9]*[1-9][0-9]*$";
		var re = new RegExp(type);
	
		if(ss == 0){
			return true;
		}
	
		if(ss.match(re)==null){
			return false;
		}
	
		return true;
	}
	
	function check(){
		var signal = document.getElementById("signal").value;
		var timemin = document.getElementById("timemin").value;
		var timemax = document.getElementById("timemax").value;
		var call_times = document.getElementById("call_times").value;
		var call_asr = document.getElementById("call_asr").value;
		var call_failed_times = document.getElementById("call_failed_times").value;
		
		$("#get_status_span").html("");
		if (document.getElementById('bcch_type').selectedIndex == 1) {
			$("#get_status_span").html("");
			var ret = onCheckBcchLock(0);
			if (ret == 0) {
				$("#get_status_span").html("<?php echo "<span style='color:red;'>Please select at least one BCCH.</span>"; ?>");
				return false;		
			}
			else if (ret == -1){//table empty
				$("#get_status_span").html("<?php echo "<span style='color:red;'>Please click on the Get Current State button for the BCCH list.</span>"; ?>");
				return false;					
			}
		}
		else if (document.getElementById('bcch_type').selectedIndex == 2) {
			document.getElementById("auto_period").innerHTML = "";
			document.getElementById("csignal").innerHTML = "";
			if (!check_integer(signal) || parseInt(signal) < 1 || parseInt(signal) > 31) {
				document.getElementById("csignal").innerHTML = con_str("Allowed character must be an integer between 1 to 31");
				return false;
			}			
			if (!check_integer(timemin) || !check_integer(timemax)) {
				document.getElementById("auto_period").innerHTML = con_str("Allowed character must be an integer.");
				return false;
			}
		}
		else if (document.getElementById('bcch_type').selectedIndex == 3) {
			document.getElementById("ccall_times").innerHTML = "";
			document.getElementById("ccall_asr").innerHTML = "";
			document.getElementById("ccall_failed_times").innerHTML = "";
			document.getElementById("csignal").innerHTML = "";
			if (!check_integer(signal) || parseInt(signal) < 1 || parseInt(signal) > 31) {
				document.getElementById("csignal").innerHTML = con_str("Allowed character must be an integer between 1 to 31");
				return false;
			}				
			if (!check_integer(call_times))	 {
				document.getElementById("ccall_times").innerHTML = con_str("Allowed character must be an integer.");
				return false;			
			}
			if (!check_integer(call_asr))	 {
				document.getElementById("ccall_asr").innerHTML = con_str("Allowed character must be an integer.");
				return false;			
			}
			if (!check_integer(call_failed_times))	 {
				document.getElementById("ccall_failed_times").innerHTML = con_str("Allowed character must be an integer.");
				return false;			
			}
		}
		
		return true;
	}
	</script>

<?php 
}
?>

<?php
function save_specify_port($channel,$url,$post)
{
	$post['sel_gsm'] = $channel;
	if($url) {
		$buf = gw_send_POST($url, $post);
		if(strstr($buf,'GSM Save Successful!')){
			return true;
		}
	}
	
	return false;
}

function save_gsm($channel,$slaveip,$board)
{
/*
Location: /etc/asterisk/gw_gsm.conf
...
*/
	global $__GSM_SUM__;
	global $__BRD_SUM__;
	global $__GSM_HEAD__;
	global $__BRD_HEAD__;
	global $__deal_cluster__;
	
	$post = $_POST;
	$post['sel_slaveip'] = '';
	unset($post['spans']);

	/*
	 *
	 *	save to other ports
	 *
	 */
	if(isset($_POST['spans']) && is_array($_POST['spans'])) { //It's save to other ports.
		$post['sync'] = true;

		$s = 0;
		for($i=1; $i<=$__GSM_SUM__; $i++) {
			if(isset($_POST['spans'][1][$i])) {
				$save[$s]['url'] = $_SERVER['HTTP_REFERER'];
				$save[$s]['channel'] = $i;
				$save[$s]['board'] = 1;
				$s++;
			}
		}

		if($__deal_cluster__){
			$cluster_info = get_cluster_info();
			if($cluster_info['mode']=='master') {
				for($b=2; $b<=$__BRD_SUM__; $b++) {
					for($i=1; $i<=$__GSM_SUM__; $i++) {
						if(isset($_POST['spans'][$b][$i])) {
							if(isset($cluster_info[$__BRD_HEAD__.$b.'_ip'])) {
								$slave_ip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
								$save[$s]['url'] = "http://$slave_ip/cgi-bin/php/gsm-bcch.php";
							} else {
								$save[$s]['url'] = '';
							}
							$save[$s]['channel'] = $i;
							$save[$s]['board'] = $b;
							$s++;
						}
					}

				}
			}
		}
		
		ob_flush();
		flush();
		foreach($save as $each) {
			$pid = pcntl_fork();
			if($pid == 0) {
				ob_clean();
				ob_end_clean();
				save_specify_port($each['channel'],$each['url'],$post);
				exit(0);
			} else if($pid > 0) {
				$pids[] = $pid;
			} else {
			}
		}

		if(isset($pids) && is_array($pids)) {
			foreach($pids as $each) {
				pcntl_waitpid($each,$status);
			}
		}
	}

	/*
	 *
	 *	save to selected port(slave)
	 *
	 */
	 if($__deal_cluster__){
		if($slaveip != '') {
			if(isset($post['sync']))unset($post['sync']);
			$url = "http://$slaveip/cgi-bin/php/gsm-bcch.php";
			save_specify_port($channel,$url,$post);
			$data = "astcmd:"; //None pocess
			wait_apply("request_slave", $slaveip, $data);
			return true;
		}
	 }

	/*
	 *
	 *	save to selected port(master) or the port 'POST'
	 *
	 */
	$sync = false; //differentiate true submit and simulation submit
	if(isset($_POST['sync'])) {
		$sync = true;
	}
	//POST data preprocessing
	if(!isset($_POST['bcch_type'])) {
		$_POST['bcch_type'] = 'default';
	}

	if (isset($_POST['sel_fixed'])) {
		$_POST['bcch_fixed'] = $_POST['sel_fixed'];
	}
	if (isset($_POST['timemin']) && isset($_POST['timemax'])) {
		$_POST['bcch_range'] = $_POST['timemin']."-".$_POST['timemax'];
	}
	if(isset($_POST['call_times'])) {
		$_POST['bcch_calls'] = $_POST['call_times'];
	}
	if(isset($_POST['call_failed_times'])) {
		$_POST['bcch_faileds'] = $_POST['call_failed_times'];
	}	
	if(isset($_POST['call_asr'])) {
		$_POST['bcch_asr'] = $_POST['call_asr'];
	}
	if(isset($_POST['TalkEnable'])) {
		if ($_POST['TalkEnable'] == 1) {
			$_POST['bcch_incalls'] = 'yes';
		}
		else {
			$_POST['bcch_incalls'] = 'no';
		}
	}
	if(isset($_POST['signal'])) {
		$_POST['bcch_minrxl'] = $_POST['signal'];
	}	
	$default_array = array(
				'bcch_type'=>'default',
				'bcch_range'=>'',
				'bcch_fixed'=>'',
				'bcch_calls'=>'',
				'bcch_faileds'=>'',
				'bcch_asr'=>'',
				'bcch_incalls'=>'yes',
				'bcch_minrxl'=>'',
				);

	$conf_dir = '/etc/asterisk';
	$conf_file = 'gw_gsm.conf';
	$conf_path = $conf_dir."/".$conf_file;
	if(!file_exists($conf_path)) {
		fclose(fopen($conf_path,"w"));
	}

	$aql = new aql();
	$setok = $aql->set('basedir',$conf_dir);
	if (!$setok) {
		echo __LINE__.' '.$aql->get_error();
		return false;
	}
	$hlock=lock_file($conf_path);
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	$conf_array = $aql->query("select * from $conf_file");
	
	if(!isset($conf_array[$channel])) {
		$aql->assign_addsection($channel,'');
	}
	foreach($default_array as $key=>$value){
		if(isset($_POST[$key])){
			$val = trim($_POST[$key]);
		}else{
			$val = $value;
		}

		if($sync) {
			if(isset($_POST[$key.'_sync'])){
				if(isset($conf_array[$channel][$key])){
					$aql->assign_editkey($channel,$key,$val);
				}else{
					$aql->assign_append($channel,$key,$val);
				}
			}else{
				if(!isset($conf_array[$channel][$key])){
					$aql->assign_append($channel,$key,$value);
				}
			}
		}else{
			if(isset($conf_array[$channel][$key])){
				$aql->assign_editkey($channel,$key,$val);
			}else{
				$aql->assign_append($channel,$key,$val);
			}
		}
	}
	if (!$aql->save_config_file($conf_file)) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	
	save_gsm_to_extra_conf();

	wait_apply("exec", "asterisk -rx \"extra restart\" > /dev/null 2>&1 &");
	return true;
}

?>


<?php 
if (isset($_POST['send']) && ($_POST['send'] == 'Save' || $_POST['send'] == 'Apply')) {
	if (isset($_POST['apply_to_all']) && $_POST['apply_to_all'] == 1) {
		for ($gsmindex = 1; $gsmindex <= $__GSM_SUM__; $gsmindex++) {
			save_gsm($gsmindex,"",$_POST['sel_brd']);
		}
		
		if($__deal_cluster__){
			$cluster_info = get_cluster_info();		
			if($cluster_info['mode']=='master') {
				for($b=2; $b<=$__BRD_SUM__; $b++) {
					for($i=1; $i<=$__GSM_SUM__; $i++) {
						if(isset($cluster_info[$__BRD_HEAD__.$b.'_ip'])) {
							$slave_ip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
							save_gsm($i, $slave_ip, $b);
						} 
					}
				}
			}
		}
		
	} else {
		save_gsm($_POST['sel_gsm'],$_POST['sel_slaveip'],$_POST['sel_brd']);
	}
	show_cell_info(); 
} elseif (isset($_POST['send']) && $_POST['send'] == 'Detail') {
	if(isset($_POST['sel_gsm'])) $channel = $_POST['sel_gsm'];
	else $channel = 1;

	if(isset($_POST['sel_brd'])) $board = $_POST['sel_brd']; 
	else $board = 1;

	if(isset($_POST['sel_slaveip'])) $slaveip = $_POST['sel_slaveip'];
	else $slaveip = '';
	
	if($__deal_cluster__){
		global $__BRD_HEAD__;
		$cluster_info = get_cluster_info();
		if($cluster_info['mode'] == 'master') {
			if ($board != 1 && $slaveip == '') {
				if(isset($cluster_info[$__BRD_HEAD__.$board.'_ip']) && $cluster_info[$__BRD_HEAD__.$board.'_ip'] != '') {
					$slaveip = $cluster_info[$__BRD_HEAD__.$board.'_ip'];
				}
			}
		}
	}
	
	show_detail_info($channel, $board, $slaveip); 
} else {
	show_cell_info(); 
}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>
