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
	global $only_view;
	
	$cluster_info = get_cluster_info();
	
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

	<div class="content">
		<table class="table_show">
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
			
			<tr>
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
			</tr>
			
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
			<?php } ?>
		</table>
	</div>
	
	<div id="button_save">
		<?php if(!$only_view){ ?>
		<button onclick="getAllCellInfo();" id="getCurrent"><?php echo language('Get Current State'); ?></button>
		<button onclick="searchAllCellInfo();" id="searchCell"><?php echo language('Search Cell'); ?></button>
		<?php } ?>
		
		<span id="get_status_span" style="display:inline-block;margin-top:10px;float:right;"></span>
	</div>
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
	global $only_view;
	
	$cluster_info = get_cluster_info();
	
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
	
	<div class="content">
		<span class="title">
			<?php echo get_gsm_name_by_channel($channel,$board); ?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Port')){ ?>
							<b><?php echo language('Port');?>:</b><br/>
							<?php echo language('Port','Port'); ?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('BCCH Mode')){ ?>
							<b><?php echo language('BCCH Mode');?>:</b><br/>
							<?php echo language('BCCH Mode help',"BCCH Mode"); ?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Minimum Signal Strength allow')){ ?>
							<b><?php echo language('Minimum Signal Strength allow');?>:</b><br/>
							<?php echo language('Minimum Signal Strength allow help',"Minimum Signal Strength allow"); ?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Port');?>:
			</span>
			<div class="tab_item_right">
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
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('BCCH Mode');?>:
			</span>
			<div class="tab_item_right">
				<select size=1 name="bcch_type" id="bcch_type" onchange="onChangeBcchMode();">		  
					<option id="BcchMode0" value="default">Default</option>
					<option id="BcchMode1" value="fixed">Fixed</option>
					<option id="BcchMode2" value="random">Random</option>
					<option id="BcchMode3" value="advanced">Advanced</option>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Minimum Signal Strength allow');?>:
			</span>
			<div class="tab_item_right">
				<span id="csignal"></span>
				<input type="text" id="signal" maxlength="4" name="signal" size="18" value="<?php echo ($bcch_minrxl == '' ? '10' : $bcch_minrxl) ?>">
			</div>
		</div>
		
		<div class='div_tab_hide' id='random_bcch_div'>
			<div class="tab_item">
				<span>
					<?php echo language('Auto Period');?>:
				</span>
				<div class="tab_item_right">
					<span id="ctimemax"></span>
					<input class="text" id="timemin" maxlength="3" name="timemin" size="8" value="1">&nbsp; - &nbsp;
					<input class="text" id="timemax" maxlength="3" name="timemax" size="8" value="15">&nbsp;min
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Switch BCCH in Calling');?>:
				</span>
				<div class="tab_item_right">
					<span>
						<input id="TalkEnable" name="TalkEnable" type="radio" value="0" <?php  if ($bcch_incalls == 'no' || $bcch_incalls == '') echo "checked=\"\""; ?> >No
						<input id="TalkEnable" name="TalkEnable" type="radio" value="1" <?php  if ($bcch_incalls == 'yes') echo "checked=\"\""; ?> >Yes
					</span>
				</div>
			</div>
		</div>
		
		<div class='div_tab_hide' id='advance_bcch_div'>
			<div class="tab_item">
				<span>
					<?php echo language('Call times');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_times"></span>
					<input class="text" id="call_times" maxlength="2" name="call_times" size="8" value="<?php echo ($bcch_calls == '' ? 15 : $bcch_calls) ?>">
				</div>
			</div>
		
		
			<div class="tab_item">
				<span>
					<?php echo language('Minimun ASR');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_asr"></span>
					<input class="text" id="call_asr" maxlength="2" name="call_asr" size="8" value="<?php echo ($bcch_asr == '' ? 30 : $bcch_asr) ?>">&nbsp;%
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Call failed times');?>:
				</span>
				<div class="tab_item_right">
					<span id="ccall_failed_times"></span>
					<input class="text" id="call_failed_times" maxlength="2" name="call_failed_times" size="8" value="<?php echo ($bcch_faileds == '' ? 6 : $bcch_faileds) ?>">
				</div>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Apply To All Ports');?>:
			</span>
			<div class="tab_item_right">
				<span>
					<input id="apply_to_all" name="apply_to_all" type="radio" value="0" checked="">No
					<input id="apply_to_all" name="apply_to_all" type="radio" value="1" >Yes
				</span>
			</div>
		</div>
	</div>
	
	<div class="content">
		<table class="table_show">
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
			<tr style="height:32px">
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
			</tr>
			<tr style="height:32px">
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
			</tr>
			<tr style="height:32px">
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
			</tr>
			<tr style="height:32px">
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
			</tr>
			<tr style="height:32px">
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
			</tr>
			<tr style="height:32px">
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
				<td></td>
			</tr>
			<tr style="height:32px">
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
	</div>
	
	<div id="button_save">
		<button type="button" id="getCurrent" onclick="getCurrentCellInfo();" ><?php echo language('Get Current State');?></button>
		<button type="button" id="searchCell" onclick="searchCellInfo();" ><?php echo language('Search Cell');?></button>
		
		<?php if(!$only_view){ ?>
		<button type="submit" id="Save" onclick="document.getElementById('send').value='Save';return check();"><?php echo language('Save');?></button>
		<button type="submit" id="Apply" onclick="document.getElementById('send').value='Apply';return check();" ><?php echo language('Apply');?></button>
		<?php } ?>
		
		<button type="button" id="Cancel" onclick="window.location.href='<?php echo get_self();?>'" ><?php echo language('Cancel');?></button>
		<td width="" align="center"><span id="get_status_span" style="display:inline-block;margin-left:50px;"></span></td>
	</div>
		
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
		var is_check = false;
		
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
				is_check = true;
			}
			else if (ret == -1){//table empty
				$("#get_status_span").html("<?php echo "<span style='color:red;'>Please click on the Get Current State button for the BCCH list.</span>"; ?>");
				is_check = true;
			}
		}
		else if (document.getElementById('bcch_type').selectedIndex == 2) {
			document.getElementById("ctimemax").innerHTML = "";
			document.getElementById("csignal").innerHTML = "";
			if (!check_integer(signal) || parseInt(signal) < 1 || parseInt(signal) > 31) {
				document.getElementById('signal').focus();
				document.getElementById("csignal").innerHTML = con_str("Allowed character must be an integer between 1 to 31");
				is_check = true;
			}			
			if (!check_integer(timemin) || !check_integer(timemax)) {
				document.getElementById('timemin').focus();
				document.getElementById("ctimemax").innerHTML = con_str("Allowed character must be an integer.");
				is_check = true;
			}
		}
		else if (document.getElementById('bcch_type').selectedIndex == 3) {
			document.getElementById("ccall_times").innerHTML = "";
			document.getElementById("ccall_asr").innerHTML = "";
			document.getElementById("ccall_failed_times").innerHTML = "";
			document.getElementById("csignal").innerHTML = "";
			if (!check_integer(signal) || parseInt(signal) < 1 || parseInt(signal) > 31) {
				document.getElementById('signal').focus();
				document.getElementById("csignal").innerHTML = con_str("Allowed character must be an integer between 1 to 31");
				is_check = true;
			}				
			if (!check_integer(call_times))	 {
				document.getElementById('call_times').focus();
				document.getElementById("ccall_times").innerHTML = con_str("Allowed character must be an integer.");
				is_check = true;
			}
			if (!check_integer(call_asr))	 {
				document.getElementById('call_asr').focus();
				document.getElementById("ccall_asr").innerHTML = con_str("Allowed character must be an integer.");
				is_check = true;
			}
			if (!check_integer(call_failed_times))	 {
				document.getElementById('call_failed_times').focus();
				document.getElementById("ccall_failed_times").innerHTML = con_str("Allowed character must be an integer.");
				is_check = true;
			}
		}
		
		if(is_check){
			return false;
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
	if($only_view){
		return false;
	}
	
	if (isset($_POST['apply_to_all']) && $_POST['apply_to_all'] == 1) {
		for ($gsmindex = 1; $gsmindex <= $__GSM_SUM__; $gsmindex++) {
			save_gsm($gsmindex,"",$_POST['sel_brd']);
		}
	} else {
		save_gsm($_POST['sel_gsm'],$_POST['sel_slaveip'],$_POST['sel_brd']);
	}
	
	save_user_record("","MODULE->BCCH:Save,channel=".$_POST['sel_gsm']);
	show_cell_info(); 
} elseif (isset($_POST['send']) && $_POST['send'] == 'Detail') {
	if(isset($_POST['sel_gsm'])) $channel = $_POST['sel_gsm'];
	else $channel = 1;

	if(isset($_POST['sel_brd'])) $board = $_POST['sel_brd']; 
	else $board = 1;

	if(isset($_POST['sel_slaveip'])) $slaveip = $_POST['sel_slaveip'];
	else $slaveip = '';
	
	show_detail_info($channel, $board, $slaveip); 
} else {
	show_cell_info(); 
}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>
