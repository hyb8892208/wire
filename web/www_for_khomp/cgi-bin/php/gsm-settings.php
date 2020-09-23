<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/cluster.inc");

//AQL
include_once("/www/cgi-bin/inc/aql.php");
?>


<script type="text/javascript">
function setValue(channel,board,slaveip)
{
	document.getElementById('sel_gsm').value = channel;
	document.getElementById('sel_brd').value = board;
	document.getElementById('sel_slaveip').value = slaveip;
}

function restartChannel(portNum,bnum)
{
	var gsm = "port-"+portNum;
	var value = "Are you sure to reset the "+gsm+"?";
	
	if(confirm(value)) {
		if(bnum == 1){
			restart_url = "/../../service?action=astcmd&cmd=gsm power reset "+portNum;
		}else{
			restart_url = "/" + bnum + "/service?action=astcmd&cmd=gsm power reset "+portNum;
		}
		$.ajax({
			url: restart_url,
			type: 'GET',
			dataType: 'text',
			data: {},
			success: function(){
				
			},
			complete: function(){
			}
		});
	
	}

}
</script>

<script type="text/javascript" src="/js/check.js?v=1.1"></script>
<script type="text/javascript" src="/js/functions.js"></script>

<?php
function show_gsms()
{
	//$alldata = get_all_gsm_info();
	$board = get_slotnum();
	$js_board_str = '"'.$board.'"';
	global $__GSM_SUM__;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $only_view;
	
	//phone number switch
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$phonenum_res = $aql->query("select * from sim_query.conf");
?>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="get">
		<div class="content">
			<table class="table_show">
				<tr>
					<th><?php echo language('Port');?></th>
					
					<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
					<th><?php echo language('Mobile Number');?></th>
					<?php } ?>
					
					<th width="150px"><?php echo language('Type');?></th>
					<th width="200px"><?php echo language('Carrier');?></th>
					<th width="250px"><?php echo language('Registration Status');?></th>
					<th><?php echo language('Module Status');?></th>
					<th width="80px"><?php echo language('Actions');?></th>
					<input type="hidden" id="sel_gsm" name="sel_gsm" value="" />
					<input type="hidden" id="sel_brd" name="sel_brd" value="" />
					<input type="hidden" id="sel_slaveip" name="sel_slaveip" value="" />
				</tr>
				
				<?php
				for($c=1; $c<=$__GSM_SUM__; $c++) {
					$channel_type = get_gsm_type_by_channel($c,1);
					
					$phonenum = '';
					if(($phonenum_res[$c]['query_type']&240) != 0){
						exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $c",$phone_output);
						$phonenum = $phone_output[0];
						$phone_output = '';
					}
				?>
					<tr id='<?php echo "gsm_${board}_${c}";?>' <?php if($channel_type == 'NULL') echo 'style="display:none";';?>>
						<td><?php echo get_gsm_name_by_channel_for_showtype($c); ?></td>
						
						<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
						<td><?php echo $phonenum;?></td>
						<?php } ?>
						
						<td><?php echo $channel_type;?></td>
						<td></td>
						<td></td>
						<td></td>
						<td>
							<button type="submit" value="Modify" style="width:32px;height:32px;padding:0;" 
								onclick="document.getElementById('send').value='Modify';setValue('<?php echo $c; ?>', '<?php echo $board; ?>', '')">
								<img src="/images/edit.gif">
							</button>
							
							<?php if(!$only_view){ ?>
							<button type="button" value="Reset" style="width:32px;height:32px;padding:0;"  
								onclick="restartChannel('<?php echo $c; ?>', '<?php echo $board; ?>')" >
								<img src="/images/restart.gif">
							</button>
							<?php } ?>
							
						</td>
					</tr>
				
				<?php } ?>
			</table>
		</div>
	
		<input type="hidden" name="send" id="send" value="" />
	</form>
	
	<script type="text/javascript">
	function dynamic_show_gsm()
	{
		var bs = new Array(<?php echo $js_board_str;?>);
		var error_sum = 5;
		for(i in bs) {
			get_gsm_info(bs[i]);
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
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
				var n = 1;
			<?php }else{ ?>
				var n = 0;
			<?php }?>
			
			for(var key1 in gsminfo){
				for(var key2 in gsminfo[key1]) {
					$("#gsm_"+board+"_"+key1+" td:eq("+(2+n)+")").html(gsminfo[key1][key2].operator);
					$("#gsm_"+board+"_"+key1+" td:eq("+(3+n)+")").html(gsminfo[key1][key2].register);
					$("#gsm_"+board+"_"+key1+" td:eq("+(4+n)+")").html(gsminfo[key1][key2].state);
				}
			}

		};
		
		function clean_gsm_info(board) {
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
				var n = 1;
			<?php }else{ ?>
				var n = 0;
			<?php }?>
			
			//for(var gsm_num = 1 ;gsm_num <=4; gsm_num++) {
			for(var gsm_num = 1 ;gsm_num <=<?php echo $__GSM_SUM__?>; gsm_num++) {
				$("#gsm_"+board+"_"+gsm_num+" td:eq("+(2+n)+")").html('');
				$("#gsm_"+board+"_"+gsm_num+" td:eq("+(3+n)+")").html('');
				$("#gsm_"+board+"_"+gsm_num+" td:eq("+(4+n)+")").html('');
			}

		};

	};

$(document).ready(function (){ 
	dynamic_show_gsm();
}); 
</script>
<?php
}
?>


<?php
function edit_gsm($channel,$slaveip,$board)
{
	exec("grep -rn Revision /tmp/gsm/$channel", $temp);
	$module_select = explode(": ", $temp[0]);
	$module_select = $module_select[1];
	$band_array_select = '';
	$bandgsm_array_select = '';
	$bandlte_array_select = '';
	$bandscdma_array_select = '';
	if (strstr($module_select, 'UC15A')) {
		$band_array_select['band_wcdma850'] = "WCDMA850";
		$band_array_select['band_wcdma1900'] = "WCDMA1900";
		
		$bandgsm_array_select['band_gsm850'] = 'GSM850';
		$bandgsm_array_select['band_gsm900'] = 'GSM900';
		$bandgsm_array_select['band_gsm1800'] = 'GSM1800';
		$bandgsm_array_select['band_gsm1900'] = 'GSM1900';
	} else if (strstr($module_select, 'UC15E')) {
		$band_array_select['band_wcdma900'] = "WCDMA900";
		$band_array_select['band_wcdma2100'] = "WCDMA2100";		
		
		$bandgsm_array_select['band_gsm900'] = 'GSM900';
		$bandgsm_array_select['band_gsm1800'] = 'GSM1800';
	} else if (strstr($module_select, 'UC15T')) {
		$band_array_select['band_wcdma850'] = "WCDMA850";
		$band_array_select['band_wcdma2100'] = "WCDMA2100";
		
		$bandgsm_array_select['band_gsm850'] = 'GSM850';
		$bandgsm_array_select['band_gsm900'] = 'GSM900';
		$bandgsm_array_select['band_gsm1800'] = 'GSM1800';
		$bandgsm_array_select['band_gsm1900'] = 'GSM1900';
	}else if (strstr($module_select, 'EC20CE')) {
		$band_array_select['band_wcdma900'] = 'WCDMA900';
		$band_array_select['band_wcdma2100'] = 'WCDMA2100';
		
		$bandgsm_array_select['band_gsm900'] = 'GSM900';
		$bandgsm_array_select['band_gsm1800'] = 'GSM1800';
		
		$bandlte_array_select['band_lte_b1'] = 'LTE-B1';
		$bandlte_array_select['band_lte_b3'] = 'LTE-B3';
		$bandlte_array_select['band_lte_b5'] = 'LTE-B5';
		$bandlte_array_select['band_lte_b8'] = 'LTE-B8';
		$bandlte_array_select['band_lte_b38'] = 'LTE-B38';
		$bandlte_array_select['band_lte_b39'] = 'LTE-B39';
		$bandlte_array_select['band_lte_b40'] = 'LTE-B40';
		$bandlte_array_select['band_lte_b41'] = 'LTE-B41';
		
		$bandscdma_array_select['band_scdma_b34'] = 'TD-SCDMA-B34';
		$bandscdma_array_select['band_scdma_b39'] = 'TD-SCDMA-B39';
	}else if (strstr($module_select, 'EC25E')){
		$band_array_select['band_wcdma850'] = 'WCDMA850';
		$band_array_select['band_wcdma900'] = 'WCDMA900';
		$band_array_select['band_wcdma2100'] = 'WCDMA2100';
		
		$bandgsm_array_select['band_gsm900'] = 'GSM900';
		$bandgsm_array_select['band_gsm1800'] = 'GSM1800';
		
		$bandlte_array_select['band_lte_b1'] = 'LTE-B1';
		$bandlte_array_select['band_lte_b3'] = 'LTE-B3';
		$bandlte_array_select['band_lte_b5'] = 'LTE-B5';
		$bandlte_array_select['band_lte_b7'] = 'LTE-B7';
		$bandlte_array_select['band_lte_b8'] = 'LTE-B8';
		$bandlte_array_select['band_lte_b20'] = 'LTE-B20';
		$bandlte_array_select['band_lte_b38'] = 'LTE-B38';
		$bandlte_array_select['band_lte_b40'] = 'LTE-B40';
		$bandlte_array_select['band_lte_b41'] = 'LTE-B41';
	}else if(strstr($module_select, 'EC25AUTL')){
		$bandlte_array_select['band_lte_b3'] = 'LTE-B3';
		$bandlte_array_select['band_lte_b7'] = 'LTE-B7';
		$bandlte_array_select['band_lte_b28'] = 'LTE-B28';
	}else if(strstr($module_select, 'EC25AUT')){
		$band_array_select['band_wcdma850'] = 'WCDMA850';
		$band_array_select['band_wcdma2100'] = 'WCDMA2100';
		
		$bandlte_array_select['band_lte_b1'] = 'LTE-B1';
		$bandlte_array_select['band_lte_b3'] = 'LTE-B3';
		$bandlte_array_select['band_lte_b5'] = 'LTE-B5';
		$bandlte_array_select['band_lte_b7'] = 'LTE-B7';
		$bandlte_array_select['band_lte_b28'] = 'LTE-B28';
	}else if(strstr($module_select, 'EC25AU')){
		$band_array_select['band_wcdma850'] = 'WCDMA850';
		$band_array_select['band_wcdma900'] = 'WCDMA900';
		$band_array_select['band_wcdma1900'] = 'WCDMA1900';
		$band_array_select['band_wcdma2100'] = 'WCDMA2100';
		
		$bandgsm_array_select['band_gsm850'] = 'GSM850';
		$bandgsm_array_select['band_gsm900'] = 'GSM900';
		$bandgsm_array_select['band_gsm1800'] = 'GSM1800';
		$bandgsm_array_select['band_gsm1900'] = 'GSM1900';
		
		$bandlte_array_select['band_lte_b1'] = 'LTE-B1';
		$bandlte_array_select['band_lte_b2'] = 'LTE-B2';
		$bandlte_array_select['band_lte_b3'] = 'LTE-B3';
		$bandlte_array_select['band_lte_b4'] = 'LTE-B4';
		$bandlte_array_select['band_lte_b5'] = 'LTE-B5';
		$bandlte_array_select['band_lte_b7'] = 'LTE-B7';
		$bandlte_array_select['band_lte_b8'] = 'LTE-B8';
		$bandlte_array_select['band_lte_b28'] = 'LTE-B28';
		$bandlte_array_select['band_lte_b40'] = 'LTE-B40';
	}else if(strstr($module_select, 'EC25A')){
		$band_array_select['band_wcdma850'] = 'WCDMA850';
		$band_array_select['band_wcdma1700'] = 'WCDMA1700';
		$band_array_select['band_wcdma1900'] = 'WCDMA1900';
		$band_array_select['band_wcdma2100'] = 'WCDMA2100';
		
		$bandlte_array_select['band_lte_b2'] = 'LTE-B2';
		$bandlte_array_select['band_lte_b4'] = 'LTE-B4';
		$bandlte_array_select['band_lte_b12'] = 'LTE-B12';
	}else if(strstr($module_select, 'EC25V')){
		$bandlte_array_select['band_lte_b4'] = 'LTE-B4';
		$bandlte_array_select['band_lte_b13'] = 'LTE-B13';
	}else if(strstr($module_select, 'EC25J')){
		$band_array_select['band_wcdma850_b6'] = 'WCDMA850-B6';
		$band_array_select['band_wcdma850_b19'] = 'WCDMA850-B19';
		$band_array_select['band_wcdma900'] = 'WCDMA900';
		$band_array_select['band_wcdma2100'] = 'WCDMA2100';
		
		$bandlte_array_select['band_lte_b1'] = 'LTE-B1';
		$bandlte_array_select['band_lte_b3'] = 'LTE-B3';
		$bandlte_array_select['band_lte_b8'] = 'LTE-B8';
		$bandlte_array_select['band_lte_b18'] = 'LTE-B18';
		$bandlte_array_select['band_lte_b19'] = 'LTE-B19';
		$bandlte_array_select['band_lte_b26'] = 'LTE-B26';
		$bandlte_array_select['band_lte_b41'] = 'LTE-B41';
	}
	
	$channel_module_name = get_gsm_name_by_channel($channel,$board,false);
	$res  = explode('-', $channel_module_name, 2);
	$channel_module_type = $res[0];
?>
	<script type="text/javascript">
	
	function modifySMSC(channel,old_smsc)
	{
		var value = window.prompt("<?php echo language('Modify SMSC prompt','Please input a new SMS Center Number');?>", old_smsc);

		if(value) {
			if(confirm("<?php echo language('Modify SMSC confirm','Do you confirm to change SIM SMS Center Number ? It\'s dangerous!');?>"
				+"\nOld SMSC: '"+old_smsc+"'\nNew SMSC: '"+value+"'\nSpan: <?php echo get_gsm_name_by_channel($channel,$board);?>")) {
				document.getElementById("new_smsc").value = value;
				document.getElementById('manform').submit();
			}
		}
	}

	function pinsw_click()
	{
		var needpin = document.getElementById("needpin").checked;

		if(needpin) {
			document.getElementById("pin").readOnly = false;
		} else {
			document.getElementById("pin").readOnly = true;
		}
	}
	
	function check()
	{
		var is_check = false;
		
		var name = document.getElementById("name").value;
		var vol = document.getElementById("vol").value;
		var mic = document.getElementById("mic").value;
		<?php if($channel_module_type == "lte" || $channel_module_type == 'umts'){ ?>
			var lte_txgain = document.getElementById("lte_txgain").value;
			var lte_txdgain = document.getElementById("lte_txdgain").value;
			var lte_rxgain = document.getElementById("lte_rxgain").value;
			
			document.getElementById("clte_txgain").innerHTML = '';
			if(!check_gsmgain(lte_txgain)) {
				document.getElementById('lte_txgain').focus();
				document.getElementById("clte_txgain").innerHTML = con_str('<?php echo language('js check gsmgain','Range: -1-65535.');?>');
				is_check = true;
			}

			document.getElementById("clte_txdgain").innerHTML = '';
			if(!check_gsmgain(lte_txdgain)) {
				document.getElementById('lte_txdgain').focus();
				document.getElementById("clte_txdgain").innerHTML = con_str('<?php echo language('js check gsmgain','Range: -1-65535.');?>');
				is_check = true;
			}
			
			document.getElementById("clte_rxgain").innerHTML = '';
			if(!check_gsmgain(lte_rxgain)) {
				document.getElementById('lte_rxgain').focus();
				document.getElementById("clte_rxgain").innerHTML = con_str('<?php echo language('js check gsmgain','Range: -1-65535.');?>');
				is_check = true;
			}
		<?php } ?>
		var dialprefix = document.getElementById("dialprefix").value;
		var pin = document.getElementById("pin").value;
		var needpin = document.getElementById("needpin").checked;

	//	var dacgain = document.getElementById("dacgain").value;
	//	var adcgain = document.getElementById("adcgain").value;


		document.getElementById("cname").innerHTML = '';
		if(name != '') {
			if(!check_diyname(name)) {
				document.getElementById('name').focus();
				document.getElementById("cname").innerHTML = con_str('<?php echo htmlentities(language('js check diyname','Allowed character must be any of [-_+.<>&0-9a-zA-Z],1 - 32 characters.'));?>');
				is_check = true;
			}
		}
		
		document.getElementById("cvol").innerHTML = '';
		if(!check_gsmvol(vol)) {
			document.getElementById('vol').focus();
			document.getElementById("cvol").innerHTML = con_str('<?php echo language('js check gsmvol',' Volume range: 0-100.');?>');
			is_check = true;
		}
		
		document.getElementById("cmic").innerHTML = '';
		if(!check_gsmmic(mic)) {
			document.getElementById('mic').focus();
			document.getElementById("cmic").innerHTML = con_str('<?php echo language('js check gsmmic',' Volume range: 0-15.');?>');
			is_check = true;
		}

/*
		if(!check_dacgain(dacgain)) {
			document.getElementById("cdacgain").innerHTML = con_str('<?php echo language('js check adcchipgain',' Gain range:  -42 - 20.');?>');
			return false;
		} else {
			document.getElementById("cdacgain").innerHTML = '';
		}

		if(!check_adcgain(adcgain)) {
			document.getElementById("cadcgain").innerHTML = con_str('<?php echo language('js check adcchipgain',' Gain range:  -42 - 20.');?>');
			return false;
		} else {
			document.getElementById("cadcgain").innerHTML = '';
		}
*/
		document.getElementById("cdialprefix").innerHTML = '';
		if(!check_dialprefix(dialprefix)) {
			document.getElementById('dialprefix').focus();
			document.getElementById("cdialprefix").innerHTML = con_str('<?php echo language('js check dialprefix','Must be 0-9 or \\\'+\\\',\\\'*\\\',\\\'#\\\' ');?>');
			is_check = true;
		}

		document.getElementById("cpin").innerHTML = '';
		if(needpin) {
			if(!check_gsmpin(pin)) {
				document.getElementById('pin').focus();
				document.getElementById("cpin").innerHTML = con_str('<?php echo language('js check gsmpin','Must be 4 - 12 digits');?>');
				is_check = true;
			}
		}
	
		// if(parseInt(csq_timeout) < 0 || parseInt(csq_timeout) > 300){
			// document.getElementById("ccsq_timeout").innerHTML = con_str('<?php echo language('js check csq timeout', 'the range of csq timeout is 1 ~ 300s')?>');
			// return false;
		// } else {
			// document.getElementById("ccsq_timeout").innerHTML = '';
		// }
		/* check dial limit settings */
		if($('#dl_single_sw').attr("checked")=="checked"){
			if(!check_step('dl_step')){
				document.getElementById('dl_step').focus();
				is_check = true;
			}
			
			if(!check_dl_integer('dl_single_limit')){
				document.getElementById('dl_single_limit').focus();
				is_check = true;
			}
		}

		if($('#dl_total_sw').attr("checked")=="checked"){
			if(!check_step('dl_step')){
				document.getElementById('dl_step').focus();
				is_check = true;
			}
			
			if(!check_dl_integer('dl_total_limit')){
				document.getElementById('dl_total_limit').focus();
				is_check = true;
			}
			
			if(!check_freetime('dl_free_time')){
				document.getElementById('dl_free_time').focus();
				is_check = true;
			}
			
			if(!check_warning_time('dl_warning_time')){
				document.getElementById('dl_warning_time').focus();
				is_check = true;
			}
			
			if(!check_warning_describe('dl_warning_describe')){
				document.getElementById('dl_warning_describe').focus();
				is_check = true;
			}

			if($('#dl_auto_reset_sw').attr("checked")=="checked"){
				if(!check_dl_datetime('dl_auto_reset_date')){
					document.getElementById('dl_auto_reset_date').focus();
					is_check = true;
				}
			}
		}
		
		if(is_check){
			return false;
		}
		
		return true;
	}
	</script>

	<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
	<link type="text/css" href="/css/jquery-ui-timepicker-addon.css" rel="stylesheet" media="all"/>
	<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>

	<script type="text/javascript">
	
	/* select_all_band : set all band options checked.*/
	var select_all_band_flag = false;
	function select_all_band(that)
	{
		$(".band_class").each(function(){
			if($(that).attr('checked') == 'checked'){
				if($(this).attr('disabled') != 'disabled'){
					$(this).attr('checked', true);
					$("#default").attr('checked',false);
				}
			}else{
				$(this).removeAttr('checked');
				$("#default").attr('checked',false);
			}
		});
			
	}

	function check_step(id)
	{
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

	function check_freetime(id)
	{
		var obj = document.getElementById(id);
		obj.value = obj.value.replace(/[^\d]*/g,'');

		var val_str = obj.value;
		var val = parseInt(val_str);
		var step_str = document.getElementById('dl_step').value;
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

	function check_warning_time(id)
	{
		var obj = document.getElementById(id);
		obj.value = obj.value.replace(/[^\d]*/g,'');

		var val_str = obj.value;
		var val = parseInt(val_str);
		var limit_str = document.getElementById('dl_total_limit').value;
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

	function check_warning_describe(id)
	{
		var obj = document.getElementById(id);
		obj.value = obj.value.replace(/[=;"]*/g,'');

		return true;
	}

	function check_dl_integer(id)
	{
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

	function check_phone_number(id)
	{
		var obj = document.getElementById(id);
		obj.value = obj.value.replace(/[^\d]*/g,'');

		val = obj.value;
		
		if(!check_number(val)) {
			$("#c"+id).html(con_str("<?php echo language('js check integer','Please input integer!');?>"));
			return false;
		} else {
			$("#c"+id).html("");
			return true;
		}

	}

	function check_dl_datetime(id)
	{
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

	function dl_reset()
	{
		if(confirm("<?php echo language('Dial Limit Reset confirm','Do you confirm to reset Remaining Time?');?>")){
			document.getElementById('send').value='dl_reset';
			document.getElementById('manform').submit();
		}
	}

	function enable_dl(id,showId)
	{
		$('#'+showId).slideToggle();
		if($('#dl_single_sw').attr("checked")!="checked" && $('#dl_total_sw').attr("checked")!="checked"){
			$("#diallimit_step").slideUp();
		}
		if($('#dl_single_sw').attr("checked")=="checked" || $('#dl_total_sw').attr("checked")=="checked"){
			$("#diallimit_step").slideDown();
		}
		handle_dl_sync();
	}
	
	function enable_STK_flag(){
		if($('#atd_stk_flag').attr('checked') == 'checked'){
			$('#atd_stk_at_main').show();
		}else{
			$('#atd_stk_at_main').hide();
		}
	}

	function select_checkbox(checked,containerID,className)
	{
		$('#'+containerID+' .'+className).attr({"checked":checked});
	}
	
	function handle_dl_sync()
	{
		$("#diallimit_step .setting_sync").hide();
		$("#single_diallimit .setting_sync").hide();
		$("#total_diallimit .setting_sync").hide();

		//check single diallimit
		if($('#dl_single_sw').attr("checked")=="checked" && $('#dl_single_sw_sync').attr("checked")=="checked"){
			$("#single_diallimit .setting_sync").attr({"checked":true});
		}else{
			$("#single_diallimit .setting_sync").attr({"checked":false});
		}

		//check total diallimit
		if($('#dl_total_sw').attr("checked")=="checked" && $('#dl_total_sw_sync').attr("checked")=="checked"){
			$("#total_diallimit .setting_sync").attr({"checked":true});
			$('#dl_auto_reset_sw_sync').show();

			//check auto reset diallimit
			if($('#dl_auto_reset_sw').attr("checked")=="checked" && $('#dl_auto_reset_sw_sync').attr("checked")=="checked"){
				$("#diallimit_auto_reset .setting_sync").attr({"checked":true});
			}else{
				$("#diallimit_auto_reset .setting_sync").attr({"checked":false});
			}
		}else{
			$("#total_diallimit .setting_sync").attr({"checked":false});
			$('#dl_auto_reset_sw_sync').hide();
		}

		//check step
		if(($('#dl_single_sw').attr("checked")=="checked" && $('#dl_single_sw_sync').attr("checked")=="checked")
			|| ($('#dl_total_sw').attr("checked")=="checked" && $('#dl_total_sw_sync').attr("checked")=="checked")){
			$("#diallimit_step .setting_sync").attr({"checked":true});
		}else{
			$("#diallimit_step .setting_sync").attr({"checked":false});
		}
	}

	function handle_port_sync()
	{
		if(isAllCheckboxChecked('class','port')){
			$("#all_port").attr({"checked":true});
		}else{
			$("#all_port").attr({"checked":false});
		}
		if(isCheckboxChecked('class','port')){
			$(".setting_sync").show();
			$("#all_settings_sync").attr({"disabled":false,"checked":true});
			selectAllCheckbox(true,'class','setting_sync');
			handle_dl_sync();
		}else{
			$(".setting_sync").hide();
			$("#all_settings_sync").attr({"disabled":true,"checked":true});
			selectAllCheckbox(false,'class','setting_sync');
		}
	}

	function handle_setting_sync()
	{
		if(isAllCheckboxChecked('class','setting_sync')){
			$("#all_settings_sync").attr({"checked":true});
		}else{
			$("#all_settings_sync").attr({"checked":false});
		}
	}
	
	function band_filter(that){
		var val = $(that).val();
		$(".band_class").removeAttr('checked');
		$(".band_class").attr('disabled',true);
		switch(parseInt(val)){
			case 0:
				$(".band_class").removeAttr('disabled');
				break;
			case 1:
				$(".gsm_band").removeAttr('disabled');
				$(".gsm_band").attr('checked',true);
				break;
			case 2:
				$(".wcdma_band").removeAttr('disabled');
				$(".wcdma_band").attr('checked',true);
				break;
			case 3:
				$(".lte_band").removeAttr('disabled');
				$(".lte_band").attr('checked',true);
				break;
			case 4:
				$(".scdma_band").removeAttr('disabled');
				$(".scdma_band").attr('checked',true);
				break;
			case 5:
				$(".wcdma_band").attr('checked',true);
				$(".wcdma_band").removeAttr('disabled');
				$(".scdma_band").removeAttr('disabled');
				$(".scdma_band").attr('checked',true);
				break;
			case 6:
				break;
			case 7:
				break;
			case 8:
				break;
		}
		$("#default").removeAttr('checked');
	}

	$(document).ready(function (){
		if($('#dl_single_sw').attr("checked")=="checked" || $('#dl_total_sw').attr("checked")=="checked"){
			$("#diallimit_step").show();
		}
		if($('#dl_single_sw').attr("checked")=="checked")$("#single_diallimit").show();
		if($('#dl_total_sw').attr("checked")=="checked")$("#total_diallimit").show();
		if($('#dl_auto_reset_sw').attr("checked")=="checked")$("#diallimit_auto_reset").show();
		if($('#atd_stk_flag').attr("checked")=="checked"){
			$("#atd_stk_at_main").show();
		}else{
			$("#atd_stk_at_main").hide();
		}

		$(".port").click(function(){
			handle_port_sync();
		});

		$(".setting_sync").click(function(){
			handle_setting_sync();
		});

		pinsw_click();
	});

	</script>

<?php
	global $__GSM_SUM__;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $__UMTS_HEAD__;
	global $__MODULE_HEAD_ARRAY__;
	global $__LTE_HEAD__;
	global $only_view;
	
	if($channel > $__GSM_SUM__ || $channel < 1)
		return;

	$aql = new aql();

	if($slaveip == '') {
		$aql->set('basedir','/etc/asterisk');
		$res = $aql->query("select * from gw_gsm.conf where section='$channel'");
	}

	//default settings
	$name = '';
	$vol='';
	$mic='';
	$lte_txgain='11172';
	$lte_txdgain='11172';
	$lte_rxgain='60000';
	$dacgain='';
	$adcgain='';
	$dialprefix='';
	$needpin = false;
	$pin='';
	$custom_start_at='';
	$gsm_ec = '';
	$clir = '';
	$band_mode='';
	$band='';
	$seloperator='';
	$atd_stk_flag = '';

	// gsm audio codec
	//$codec_selected = ''; // 当前选择的codec
	$codecs = array(); 	  // 当前模块可选的的codec类型
	$at_set_codec=''; 	  // 当前模块设置codec的at命令

	if(isset($res[$channel]['codec_selected'])) {
		$codec_selected = trim($res[$channel]['codec_selected']);
	}

	$gsm_info = get_gsm_info($slaveip);
	if (stripos($gsm_info[$channel]['model_name'], "SIM840") == FALSE && 
			stripos($gsm_info[$channel]['model_name'], "UC15") == FALSE &&
			stripos($gsm_info[$channel]['model_name'], "M35") == FALSE) {
		sleep(1);	
		$gsm_info = get_gsm_info($slaveip);
			}

	// 以下codec类型来自模块文档, key为at命令所需的参数, 与value对应
	// 类型17为default, 手册上说17是默认值且不能设定，但通过at命令设定17并查询可以成功
	if (stripos($gsm_info[$channel]['model_name'], "SIM840") !== FALSE) {
		$codecs = array(
			"0" => "FR",
			"1" => "EFR/FR",
			"2" => "FR/HR",
			//"3" ==> "FR/HR", // 与2相同
			"4" => "HR/EFR",
			"5" => "EFR/HR",
			"6" => "AMR-FR/EFR, AMR-HR",
			"7" => "AMR-FR/EFR, AMR-HR/HR",
			"8" => "AMR-HR/HR/AMR-FR/EFR",
			"9" => "AMR-HR/AMR-FR/EFR",
			"10" => "AMR-HR/AMR-FR/FR",
			"11" => "AMR-HR/HR/AMR-FR",
			"12" => "AMR-FR/AMR-HR",
			"13" => "AMR-FR/FR/AMR-HR",
			"14" => "AMR-FR/FR/AMR-HR/HR",
			"15" => "AMR-FR/EFR/FR/AMR-HR/HR",
			"16" => "AMR-HR/AMR-FR/EFR/FR/HR",
			"17" => "AMR-WEB/AMR-FR/EFR/FR/AMR-HR/HR" 
		);	

		// 默认值
		if (!isset($codec_selected)) {
			$codec_selected = "17";	
		}

		$at_set_codec = "AT+SVR=";

	} else if (stripos($gsm_info[$channel]['model_name'], "UC15") !== FALSE) {
		$codecs = array(
			"0" => "No AMR configuration",
			"1" => "GSM AMR NB",
			"2" => "GSM AMR WB",
			"4" => "GSM HR AMR",
			"3" => "GSM AMR NB&WB", // 3 = 1 + 2
			"5" => "GSM AMR NB&GSM HR AMR",
			"6" => "GSM AMR WB&GSM HR AMR",
			"7" => "GSM AMR NB&GSM AMR WB&GSM HR AMR",
			"8" => "WCDMA AMR WB",
			"15" => "All supported"
		);		

		if (!isset($codec_selected)) {
			$codec_selected = "0";
		}

		$at_set_codec = "AT+QCFG=\\\\\\\"amrcodec\\\\\\\",";
	} else if (stripos($gsm_info[$channel]['model_name'], "M35") !== FALSE || stripos($gsm_info[$channel]['model_name'], "M26") !== FALSE){
			$codecs = array(
				"0" => "auto",
				"1" => "FR",
				"2" => "HR",
				"3" => "EFR",
				"4" => "AMR_FR",
				"5" => "AMR_HR",
				"6" => "FR&EFR, FR",
				"7" => "EFR&FR, EFR",
				"8" => "EFR&HR, EFR",
				"9" => "EFR&AMR_FR, EFR",
				"10" => "AMR_FR&FR, AMR_FR",
				"11" => "AMR_FR&HR, AMR_FR",
				"12" => "AMR_FR&EFR, AMR_FR",
				"13" => "AMR_HR&FR, AMR_HR",
				"14" => "AMR_HR&HR, AMR_HR",
				"15" => "AMR_HR&EFR, AMR_HR"
			);

		if (!isset($codec_selected)) {
			$codec_selected = "0";
		}

		$at_set_codec = "AT+QSFR=";
	} else {
		$codecs = array("-1" => "ERROR");	
		$codec_selected = null;
		$at_set_codec = null;
	}

	if(isset($res[$channel]['name']))		$name = trim($res[$channel]['name']);
	if(isset($res[$channel]['vol']))		$vol =trim($res[$channel]['vol']);
	if(isset($res[$channel]['mic']))		$mic=trim($res[$channel]['mic']);
	if(isset($res[$channel]['lte_txgain']))		$lte_txgain =trim($res[$channel]['lte_txgain']);
	if(isset($res[$channel]['lte_txdgain']))		$lte_txdgain=trim($res[$channel]['lte_txdgain']);
	if(isset($res[$channel]['lte_rxgain']))		$lte_rxgain=trim($res[$channel]['lte_rxgain']);
	if(isset($res[$channel]['dacgain']))		$dacgain=trim($res[$channel]['dacgain']);
	if(isset($res[$channel]['adcgain']))		$adcgain=trim($res[$channel]['adcgain']);
	if(isset($res[$channel]['dialprefix']))		$dialprefix=trim($res[$channel]['dialprefix']);
	if(isset($res[$channel]['needpin'])) {
		if(trim($res[$channel]['needpin']) == "true")
			$needpin = true;
	}
	if(isset($res[$channel]['pin']))		$pin=trim($res[$channel]['pin']);
	if(isset($res[$channel]['custom_start_at']))	$custom_start_at=trim($res[$channel]['custom_start_at']);
	if(isset($res[$channel]['gsm_ec'])) {
		if(is_true(trim($res[$channel]['gsm_ec']))) {
			$gsm_ec = 'checked';
		}
	}
	if(isset($res[$channel]['atd_stk_flag'])){
		if(is_true(trim($res[$channel]['atd_stk_flag']))){
			$atd_stk_flag = 'checked';
		}
	}
	if(isset($res[$channel]['atd_stk_at']))		{
		$atd_stk_at=trim($res[$channel]['atd_stk_at']);
	}else{
		$atd_stk_at='AT+STKTR="810301250082028281830100"';
	}
	if(isset($res[$channel]['anonymouscall'])) {
		if(is_true(trim($res[$channel]['anonymouscall']))) {
			$clir = 'checked';
		}
	}
	if(isset($res[$channel]['band'])){
		$band = trim($res[$channel]['band']);
	}		
	if(isset($res[$channel]['band_mode'])){
		$band_mode = trim($res[$channel]['band_mode']);
	}
	//if(isset($res[$channel]['seloperator']))		$seloperator = trim($res[$channel]['seloperator']);
	if(isset($res[$channel]['operator_fullname']))  $seloperator = trim($res[$channel]['operator_fullname']);
	
	if(isset($res[$channel]['atd_clcc_flag']) && $res[$channel]['atd_clcc_flag'] == 'off'){
		$atd_clcc_flag = '';
	}else{
		$atd_clcc_flag = 'checked';
	}
	
	if(isset($res[$channel]['csq_flag'])){
		if($res[$channel]['csq_flag'] == 'on'){
			$csq_flag = 'checked';
		} else {
			$csq_flag = '';
		}
	} else {
		$csq_flag = 'checked';
	}
	if(isset($res[$channel]['at_timeout'])){
		$at_timeout = $res[$channel]['at_timeout'];
	}
	
	$calleridtype = 0;
	if(isset($res[$channel]['calleridtype'])){
		$calleridtype = $res[$channel]['calleridtype'];
	}

	if(isset($res[$channel]['csq_timeout'])){
		$csq_timeout = trim($res[$channel]['csq_timeout']);
	} else {
		$csq_timeout = '10';
	}
	//$gsm_info = get_gsm_info($slaveip);
	$gsm['operator'] = $gsm_info[$channel]['operator'];
	$gsm['signal'] = $gsm_info[$channel]['signal'];
	$gsm['ber'] = $gsm_info[$channel]['ber'];
	$gsm['simid'] = $gsm_info[$channel]['sim_imsi'];
	$gsm['sim_smsc'] = $gsm_info[$channel]['sim_sms_center_number'];
	$gsm['own_number'] = $gsm_info[$channel]['own_number'];
	$gsm['remain_time'] = $gsm_info[$channel]['remain_time'];
	$gsm['imei'] = $gsm_info[$channel]['model_imei'];
	$gsm['module_ver'] = $gsm_info[$channel]['revision'];
	$gsm['state'] = $gsm_info[$channel]['state'];

	$enable_modifysmsc = 'disabled';
	$enable_modifyimei = 'disabled';
	if(trim($gsm['state']) == 'READY') {
		$enable_modifysmsc = '';
	}
	if(trim($gsm['imei']) != '') {
		$enable_modifyimei = '';
	}
	/*
	 *     Diallimit Settings
	 *
	 *
	 */

	$dl_step = '60';
	$dl_single_sw = 'off';
	$dl_single_limit = '';
	$dl_total_sw = 'off';
	$dl_total_limit = '';
	$dl_free_time = '0';
	$dl_warning_time = '0';
	$dl_warning_num = '';
	$dl_warning_describe = '';
	$dl_remain_time = '';
	$dl_auto_reset_sw = 'off';
	$dl_auto_reset_type = 'day';
	$dl_auto_reset_date = '';

	if(isset($res[$channel]['dl_step']))			$dl_step = $res[$channel]['dl_step'];
	if(isset($res[$channel]['dl_single_sw']))		$dl_single_sw = $res[$channel]['dl_single_sw'];
	if(isset($res[$channel]['dl_single_limit']))		$dl_single_limit = $res[$channel]['dl_single_limit'];
	if(isset($res[$channel]['dl_total_sw']))		$dl_total_sw = $res[$channel]['dl_total_sw'];
	if(isset($res[$channel]['dl_total_limit']))		$dl_total_limit = $res[$channel]['dl_total_limit'];
	if(isset($res[$channel]['dl_free_time']))		$dl_free_time = $res[$channel]['dl_free_time'];
	if(isset($res[$channel]['dl_warning_time']))		$dl_warning_time = $res[$channel]['dl_warning_time'];
	if(isset($res[$channel]['dl_warning_num']))		$dl_warning_num = $res[$channel]['dl_warning_num'];
	if(isset($res[$channel]['dl_warning_describe']))	$dl_warning_describe = $res[$channel]['dl_warning_describe'];
	if(isset($res[$channel]['dl_auto_reset_sw']))		$dl_auto_reset_sw = $res[$channel]['dl_auto_reset_sw'];
	if(isset($res[$channel]['dl_auto_reset_type']))		$dl_auto_reset_type = $res[$channel]['dl_auto_reset_type'];

	$ret = get_remain_time_and_auto_reset_date($channel,$slaveip,$board);
	if(isset($ret['remain_time']))				$dl_remain_time = $ret['remain_time'];
	if(isset($ret['auto_reset_date']))			$dl_auto_reset_date = $ret['auto_reset_date'];

	/* date org format : 2013-09-03-19:21:43 */
	if($dl_auto_reset_date == ''){
		$dl_auto_reset_date = `date "+%Y-%m-%d-%H:%M:%S"`;
	}

	/* date show format : 2013-09-03 19:21:43 */
	$date_array = explode('-',$dl_auto_reset_date);
	if(isset($date_array[3])){
		$dl_auto_reset_date = $date_array[0].'-'.$date_array[1].'-'.$date_array[2].' '.$date_array[3];
	}

	if($dl_remain_time == ''){
		$dl_remain_time = language('No Limit');
	}

	$lang_sync_title = language('Synchronization option');

	if(!isset($at_timeout)){
		if($channel_module_type == 'lte'){
			$at_timeout = '100';
		}else{
			$at_timeout = '10';
		}
	}
	
	show_modify_imei_dialog();
?>

	<form id="manform" enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<input type="hidden" name="sel_gsm" value="<?php echo $channel?>" />
	<input type="hidden" name="sel_slaveip" value="<?php echo $slaveip?>" />
	<input type="hidden" name="sel_brd" value="<?php echo $board?>" />

	<input type="hidden" name="new_imei" id="new_imei" value=""/>
	<input type="hidden" name="new_smsc" id="new_smsc" value=""/>

	<div class="content">
		<span class="title">
			<?php echo language('Port');?> <?php echo get_gsm_name_by_channel($channel,$board,false);?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Name')){ ?>
							<b><?php echo language('Name','Name');?>:</b><br/>
							<?php echo language('Name help','The alias of the GSM port.<br/> Allowed characters "-_+.&lt;&gt;&amp;0-9a-zA-Z".Length: 1-32 characters.'); ?>
							
							<br/><br/>
						<?php } ?>
							
						<?php if(is_show_language_help('Speaker Volume')){ ?>
							<b><?php echo language('Speaker Volume');?>:</b><br/>
							<?php echo language('Speaker Volume help',"The speaker volume level, the range is 0-100.<br/> This will adjust the loud speaker volume level by an AT command.");?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Microphone Volume')){ ?>
							<b><?php echo language('Microphone Volume');?>:</b><br/>
							<?php echo language('Microphone Volume help',"The microphone volume, range is: 0-15. <br/> This will change the microphone gain level by an AT command."); ?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if($channel_module_type == "lte" || $channel_module_type == 'umts'){ ?>
							<?php if(is_show_language_help('Txgain')){ ?>
								<b><?php echo language('Txgain');?>:</b><br/>
								<?php
									$help = "Numeric type.Indicates uplink codec gain and the range is -1-65535.The default value is 11172.";
									echo language('Txgain help',$help);
								?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Txdgain')){ ?>
								<b><?php echo language('Txdgain');?>:</b><br/>
								<?php
									$help = "Numeric type.Indicates uplink digital gain and the range is -1-65535.The default value is 11172.";
									echo language('Txdgain help',$help);
								?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Rxgain')){ ?>
								<b><?php echo language('Rxgain');?>:</b><br/>
								<?php
									$help = "Numeric type.Indicates downlink digital gains.The range is -1-65535.The default value is 60000.";
									echo language('Rxgain help',$help);
								?>
								
								<br/><br/>
							<?php } ?>
						<?php } ?>
						
						<?php if(is_show_language_help('Dial Prefix')){ ?>
							<b><?php echo language('Dial Prefix');?>:</b><br/>
							<?php echo language('Dial Prefix help','The Prefix number of outgoing calls from this GSM channel.');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Pin Code')){ ?>
							<b><?php echo language('Pin Code');?>:</b><br/>
							<?php
								$help = "Personal indentification numbers of SIM card. <br/> PIN code can be modified to prevent SIM card from being stolen.";
								echo language('Pin Code help',$help);
							?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Custom AT commands when start')){ ?>
							<b><?php echo language('Custom AT commands when start');?>:</b><br/>
							<?php
								$help = "User custom AT commands when start system, use '|' to split AT command.";
								echo language('Custom AT commands when start help',$help);
							?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('STK flag')){ ?>
							<b><?php echo language('STK flag');?>:</b><br/>
							<?php echo language('STK flag help','STK flag'); ?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('STK AT commands when atd')){ ?>
							<b><?php echo language('STK AT commands when atd');?>:</b><br/>
							<?php echo language('STK AT commands when atd help','STK AT commands when atd'); ?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('CLIR')){ ?>
							<b><?php echo language('CLIR');?>:</b><br/>
							<?php
								$help = "Caller ID restriction,this function is used to hidden caller ID of SIM card number.<br/>The gateway will add '#31#' in front of mobile number. <br/>This function must support by Operator.";
								echo language('CLIR help',$help);
							?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if($channel_module_type !== "cdma" ) { ?>
							<?php if(is_show_language_help('SMS Center Number')){ ?>
								<b><?php echo language('SMS Center Number');?>:</b><br/>
								<?php echo language('SMS Center Number help','Your SMS center number of your local carrier.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if ($channel_module_type == 'lte') { ?>
								<?php if(is_show_language_help('Band Mode')){ ?>
									<b><?php echo language('Band Mode');?>:</b><br/>
									<?php echo language('Band Mode help','Band Mode');?>
								
									<br/><br/>
								<?php } ?>
							<?php } ?>
							
							<?php if(is_show_language_help('Band')){ ?>
								<b><?php echo language('Band');?>:</b><br/>
								<?php echo language('Band help','Band');?>
								
								<br/><br/>
							<?php } ?>
						<?php } ?>
						
						<?php if(is_show_language_help('SIM IMSI')){ ?>
							<b><?php echo language('SIM IMSI');?>:</b><br/>
							<?php echo language('SIM IMSI help','SIM IMSI');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Module IMEI')){ ?>
							<b><?php echo language('Module IMEI');?>:</b><br/>
							<?php echo language('Module IMEI help','Module IMEI');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Module Revision')){ ?>
							<b><?php echo language('Module Revision');?>:</b><br/>
							<?php echo language('Module Revision help','Module Revision');?>
							
							<br/><br/>
						<?php } ?>

						<?php if(is_show_language_help('Carrier')){ ?>
							<b><?php echo language('Carrier');?>:</b><br/>
							<?php echo language('Carrier help','Carrier');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if($channel_module_type !== "cdma" ) { ?>
							<?php if(is_show_language_help('Bind Carrier')){ ?>
								<b><?php echo language('Bind Carrier');?>:</b><br/>
								<?php echo language('Bind Carrier help','Bind Carrier');?>
								
								<br/><br/>
							<?php } ?>
						<?php } ?>
						
						<?php if(is_show_language_help('Signal')){ ?>
							<b><?php echo language('Signal');?>:</b><br/>
							<?php echo language('Signal help','Signal');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('BER')){ ?>
							<b><?php echo language('BER');?>:</b><br/>
							<?php echo language('BER help','BER');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Status')){ ?>
							<b><?php echo language('Status','Status');?>:</b><br/>
							<?php echo language('Status help','Status');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if($channel_module_type !== "cdma" && $channel_module_type !== "lte") { ?>
							<?php if(is_show_language_help('GSM Voice Codec')){ ?>
								<b><?php echo language('GSM Voice Codec');?>:</b><br/>
								<?php echo language('GSM Voice Codec help','GSM Voice Codec');?>
								
								<br/><br/>
							<?php } ?>
						<?php } ?>
						
						<?php if(is_show_language_help('CLCC')){ ?>
							<b><?php echo language('CLCC');?>:</b><br/>
							<?php echo language('CLCC help','If the command succeeds but no call is available and no response information is sent to TE, the current call list ME is returned. The default value is ON.');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if($__MODULE_HEAD_ARRAY__[$board][$channel] == $__LTE_HEAD__){ ?>
							<?php if(is_show_language_help('CSQ flag')){ ?>
								<b><?php echo language('CSQ flag');?>:</b><br/>
								<?php echo language('CSQ flag help','Open or close the CSQ flag function. The default value is ON.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('CSQ Timeout')){ ?>
								<b><?php echo language('CSQ Timeout');?>:</b><br/>
								<?php echo language('CSQ Timeout help','CSQ Timeout');?>
								
								<br/><br/>
							<?php } ?>
						<?php } ?>
						
						<?php if(is_show_language_help('AT Timeout')){ ?>
							<b><?php echo language('AT Timeout');?>:</b><br/>
							<?php echo language('AT Timeout help','AT Timeout. Unit: seconds');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('CallerID Type')){ ?>
							<b><?php echo language('CallerID Type');?>:</b><br/>
							<?php echo language('CallerID Type help','CallerID Type');?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Name','Name');?>:
			</span>
			<div class="tab_item_right">
				<span id="cname"></span>
				<input type=text name="name" id="name" value="<?php echo $name ?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Speaker Volume');?>:
			</span>
			<div class="tab_item_right">
				<span id="cvol"></span>
				<span><input type="checkbox" class="setting_sync" name="vol_sync" title="<?php echo $lang_sync_title;?>"/></span>
				<input type=text name="vol" id="vol" value="<?php echo $vol ?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Microphone Volume');?>:
			</span>
			<div class="tab_item_right">
				<span id="cmic"></span>
				<span><input type="checkbox" name="mic_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/></span>
				<input type=text name="mic" id="mic" value="<?php echo $mic ?>" />
			</div>
		</div>
		
		<?php if($channel_module_type == "lte" || $channel_module_type == 'umts'){ ?>
			<div class="tab_item">
				<span>
					<?php echo language('Txgain');?>:
				</span>
				<div class="tab_item_right">
					<span id="clte_txgain"></span>
					<span><input type="checkbox" class="setting_sync" name="lte_txgain_sync" title="<?php echo $lang_sync_title;?>"/></span>
					<input type=text name="lte_txgain" id="lte_txgain" value="<?php echo $lte_txgain ?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Txdgain');?>:
				</span>
				<div class="tab_item_right">
					<span id="clte_txdgain"></span>
					<span><input type="checkbox" name="lte_txdgain_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/></span>
					<input type=text name="lte_txdgain" id="lte_txdgain" value="<?php echo $lte_txdgain ?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Rxgain');?>:
				</span>
				<div class="tab_item_right">
					<span id="clte_rxgain"></span>
					<span><input type="checkbox" name="lte_rxgain_sync" class="setting_sync" title="<?php echo $lang_sync_title;?>"/></span>
					<input type=text name="lte_rxgain" id="lte_rxgain" value="<?php echo $lte_rxgain; ?>" />
				</div>
			</div>
		<?php } ?>
		
		<div class="tab_item">
			<span>
				<?php echo language('Dial Prefix');?>:
			</span>
			<div class="tab_item_right">
				<span id="cdialprefix"></span>
				<span><input type="checkbox" class="setting_sync" name="dialprefix_sync" title="<?php echo $lang_sync_title;?>"/></span>
				<input type=text name="dialprefix" id="dialprefix" value="<?php echo $dialprefix ?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Pin Code');?>:
			</span>
			<div class="tab_item_right">
				<span id="cpin"></span>
				<span><input type="checkbox" class="setting_sync" name="pin_sync" title="<?php echo $lang_sync_title;?>"/></span>
				<span><input type="checkbox" name="needpin" id="needpin" <?php if($needpin) echo "checked"?> onclick="pinsw_click()" /></span>
				<span style="margin-right:10px;"><?php echo language('_On');?></span>
				<input type=text name="pin" id="pin" value="<?php echo $pin ?>" readOnly/>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Custom AT commands when start');?>:
			</span>
			<div class="tab_item_right">
				<span><input type="checkbox" class="setting_sync" name="custom_start_at_sync" title="<?php echo $lang_sync_title;?>"/></span>
				<input type=text name="custom_start_at" id="custom_start_at" value='<?php echo $custom_start_at ?>' />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('STK flag');?>:
			</span>
			<div class="tab_item_right">
				<span><input type="checkbox" class="setting_sync" name="atd_stk_flag_sync" title="<?php echo $lang_sync_title;?>"/></span>
				<span><input type="checkbox" class="enable" name="atd_stk_flag" id="atd_stk_flag" <?php echo $atd_stk_flag ?> onchange="enable_STK_flag();"/></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('STK AT commands when atd');?>:
			</span>
			<div class="tab_item_right">
				<span><input type="checkbox" class="setting_sync" name="atd_stk_at_sync" title="<?php echo $lang_sync_title;?>"/></span>
				<input type=text name="atd_stk_at" id="atd_stk_at" value='<?php echo $atd_stk_at ?>' />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('CLIR');?>:
			</span>
			<div class="tab_item_right">
				<span><input type="checkbox" class="setting_sync" name="anonymouscall_sync" title="<?php echo $lang_sync_title;?>"/></span>
				<span><input type="checkbox" class="enable" name="clir" id="clir" <?php echo $clir ?>/></span>
			</div>
		</div>
		
		<?php if($channel_module_type !== "cdma" ) { ?>
			<div class="tab_item">
				<span>
					<?php echo language('SMS Center Number');?>:
				</span>
				<div class="tab_item_right">
					<?php echo $gsm['sim_smsc']; ?>
					&nbsp;&nbsp;<input type="button" value="<?php echo language('Modify');?>" <?php echo $enable_modifysmsc; ?> onclick="modifySMSC(<?php echo "$channel,"."'".$gsm['sim_smsc']."'" ?>)">
				</div>
			</div>
			
			<?php if ($channel_module_type == 'lte') { ?>
				<div class="tab_item">
					<span>
						<?php echo language('Band Mode');?>:
					</span>
					<div class="tab_item_right">
						<input type="checkbox" class="setting_sync" name="band_mode_sync" title="<?php echo $lang_sync_title;?>" />
						<select name="band_mode" id="band_mode" onchange="band_filter(this)"/>
							<option value="0" <?php if($band_mode == 0) echo 'selected';?>><?php echo language('AUTO'); ?></option>
							<option value="1" <?php if($band_mode == 1) echo 'selected';?>><?php echo language('GSM only'); ?></option>
							<option value="2" <?php if($band_mode == 2) echo 'selected';?>><?php echo language('WCDMA only'); ?></option>
							<option value="3" <?php if($band_mode == 3) echo 'selected';?>><?php echo language('LTE only'); ?></option>
							<option value="4" <?php if($band_mode == 4) echo 'selected';?>><?php echo language('TD-SCDMA only'); ?></option>
							<option value="5" <?php if($band_mode == 5) echo 'selected';?>><?php echo language('UMTS only'); ?></option>
							<option value="6" <?php if($band_mode == 6) echo 'selected';?>><?php echo language('CDMA only'); ?></option>
							<option value="7" <?php if($band_mode == 7) echo 'selected';?>><?php echo language('HDR only'); ?></option>
							<option value="8" <?php if($band_mode == 8) echo 'selected';?>><?php echo language('CDMA and HDR only'); ?></option>
						</select>
					</div>
				</div>
			<?php } ?>
			
			<div class="tab_item">
				<span>
					<?php echo language('Band');?>:
				</span>
				<div class="tab_item_right">
					<td>
						<input type="checkbox" class="setting_sync" name="band_sync" title="<?php echo $lang_sync_title;?>"/>
				<?php   
					if ($__MODULE_HEAD_ARRAY__[$board][$channel] == 'gsm-') {
				?>
						<select name="band" id="band">
							<?php
								$show_band[0] = '';
								$show_band[1] = '';
								$show_band[2] = '';
								$show_band[3] = '';
								$show_band[4] = '';
								$show_band[5] = '';
								switch($band) {
									case 'ALL_BAND':        $show_band[0] = 'selected'; break;
									case 'EGSM_MODE':       $show_band[1] = 'selected'; break;
									case 'DCS_MODE':        $show_band[2] = 'selected'; break;
									case 'PCS_MODE':        $show_band[3] = 'selected'; break;
									case 'EGSM_DCS_MODE':   $show_band[4] = 'selected'; break;
									case 'GSM850_PCS_MODE': $show_band[5] = 'selected'; break;
									default:                $show_band[0] = 'selected'; break;
								}
							?>
							<option value="ALL_BAND"        <?php echo $show_band[0];?> >All Band(850/900/1800/1900MHz)</option>
							<option value="EGSM_MODE"       <?php echo $show_band[1];?> >EGSM(850/900MHz)</option>
							<option value="DCS_MODE"        <?php echo $show_band[2];?>>DCS(1800MHz)</option>
							<option value="PCS_MODE"        <?php echo $show_band[3];?>>PCS(1900MHz)</option>
							<option value="EGSM_DCS_MODE"   <?php echo $show_band[4];?>>EGSM DCS(850/900/1800MHz)</option>
							<option value="GSM850_PCS_MODE" <?php echo $show_band[5];?>>GSM850 PCS(850/1900MHz)</option>
						</select>
					</td>
				<?php   
					} else if ($__MODULE_HEAD_ARRAY__[$board][$channel] == $__UMTS_HEAD__) {
							$band_array = explode(',',$band);
				?>
						<table id="checkbox_band" cellpadding="0" cellspacing="0" class="port_table"> 

						<tr>
						<?php
						foreach($bandgsm_array_select as $id=>$value) {
							$check_status = '';
							if ( strstr($band,$value)) {
								$check_status = 'checked';
							} else {
								$check_status = '';
							}
						?>
							<td class='module_port'> <input type="checkbox" id="<?php echo $id;?>" class="band_class" name="<?php echo $id;?>" value="<?php echo $value;?>" <?php echo $check_status;?>/> <?php echo $value;?></td>
						<?php
						}
						?>
						</tr>	
						<tr>
						<?php
						foreach($band_array_select as $id=>$value) {
							$check_status = '';
							if ( strstr($band,$value)) {
								$check_status = 'checked';
							} else {
								$check_status = '';
							}
						?>
							<td class='module_port'> <input type="checkbox" id="<?php echo $id;?>" class="band_class" name="<?php echo $id;?>" value="<?php echo $value;?>" <?php echo $check_status;?>/> <?php echo $value;?></td>
						<?php
						}
						?>
						</tr>
						
						<tr>
							<td class='module_port'>
								<input type="checkbox" id="default" name="default" value="default" <?php echo $check_status;?> > Default</td>
							</td>
						</tr>
						
						<tr style = "border:none;">
							<td style="border:none;"><input type="checkbox" onclick="select_all_band(this);"><?php echo language('ALL');?> </td>
						</tr>
						</table>
					</td>
				<?php
					} else if($__MODULE_HEAD_ARRAY__[$board][$channel] == $__LTE_HEAD__){
				?>
						<table id="checkbox_band" cellpadding="0" cellspacing="0" class="port_table"> 

						<tr>
						<?php
						foreach($bandgsm_array_select as $id=>$value) {
							$check_status = '';
							if ( strstr($band,$value)) {
								$check_status = 'checked';
							} else {
								$check_status = '';
							}
						?>
							<td class='module_port'> <input type="checkbox" class="band_class gsm_band" id="<?php echo $id;?>" name="<?php echo $id;?>" value="<?php echo $value;?>" <?php echo $check_status;?>/> <?php echo $value;?></td>
						<?php
						}
						?>
						</tr>
						
						<tr>
						<?php
						foreach($band_array_select as $id=>$value) {
							$check_status = '';
							if ( strstr($band,$value)) {
								$check_status = 'checked';
							} else {
								$check_status = '';
							}
						?>
							<td class='module_port'> <input type="checkbox" class="band_class wcdma_band" id="<?php echo $id;?>" name="<?php echo $id;?>" value="<?php echo $value;?>" <?php echo $check_status;?>/> <?php echo $value;?></td>
						<?php
						}
						?>
						</tr>
						
						<tr>
						<?php
						foreach($bandscdma_array_select as $id=>$value) {
							$check_status = '';
							if ( strstr($band,$value)) {
								$check_status = 'checked';
							} else {
								$check_status = '';
							}
						?>
							<td class='module_port'> <input type="checkbox" class="band_class scdma_band" id="<?php echo $id;?>" name="<?php echo $id;?>" value="<?php echo $value;?>" <?php echo $check_status;?>/> <?php echo $value;?></td>
						<?php
						}
						?>
						</tr>
						
						<tr>
						<?php
						foreach($bandlte_array_select as $id=>$value) {
							$check_status = '';
							if ( strstr($band,$value)) {
								$check_status = 'checked';
							} else {
								$check_status = '';
							}
						?>
							<td class='module_port'> <input type="checkbox" class="band_class lte_band" id="<?php echo $id;?>" name="<?php echo $id;?>" value="<?php echo $value;?>" <?php echo $check_status;?>/> <?php echo $value;?></td>
						<?php
						}
						?>
						</tr>
						
						<tr>
							<td class='module_port'>
								<input type="checkbox" id="default" name="default" value="default" <?php if(strstr($band,'NONE')) echo 'checked';?> > Default</td>
							</td>
						</tr>
						
						<tr style = "border:none;">
							<td style="border:none;">
								<input type="checkbox" onclick="select_all_band(this);">
								<span><?php echo language('ALL');?></span>
							</td>
						</tr>
						</table>
					</td>
				<?php
					} else {
				?>
					</td>
				<?php
					}
				 ?>
				</div>
				<div class="clear"></div>
			</div>
		
		<?php } ?>
		
		<div class="tab_item">
			<span>
				<?php echo language('SIM IMSI');?>:
			</span>
			<div class="tab_item_right">
				<span><?php echo $gsm['simid']; ?></span>
			</div>
		</div>
		
		<div class="tab_item" style="display:none;">
			<span>
				<?php echo language('Module IMEI');?>:
			</span>
			<div class="tab_item_right">
				<span><?php echo $gsm['imei']; ?>&nbsp;&nbsp;</span>
				<?php
					if($channel_module_type !== 'cdma' ) {
				?>
				<input type="button" value="<?php echo language('Modify');?>" <?php echo $enable_modifyimei;?> 
					onclick="modify_imei(<?php echo "$board, $channel,'".$gsm['imei']."'" ?>)">
				<?php 
					}
				?>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Module Revision');?>:
			</span>
			<div class="tab_item_right">
				<?php echo $gsm['module_ver']; ?>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Carrier');?>:
			</span>
			<div class="tab_item_right">
				<span><?php echo $gsm['operator']; ?></span>
			</div>
		</div>
	
		<?php if($channel_module_type !== "cdma" ) { ?>
		<div class="tab_item">
			<span>
				<?php echo language('Bind Carrier');?>:
			</span>
			<div class="tab_item_right">
				<input type="checkbox" class="setting_sync" name="seloperator_sync" title="<?php echo $lang_sync_title;?>" />
				<div style="float:right;" >
					<input type="button" name="list_carrier" value="<?php echo language("List Carrier"); ?>" onclick="get_carrier_list('<?php echo $channel; ?>', '<?php echo $board; ?>');" style="float: left;margin-right: 17px;"/>
					<img src="/images/mini_loading.gif" align="middle" id="progress" style="display:none;float: left;margin-right: 35px;margin-top: 5px;"/>
					<select name="seloperator" id="seloperator" style="float: left;"/>
						<?php if($seloperator != ''){ ?><option value='<?php echo $seloperator; ?>'><?php echo $seloperator; ?></option><?php } ?>
						<option value=""><?php echo 'Auto'; ?></option>
					</select>
				</div>
			</div>
		</div>
		<?php } ?>
		
		<div class="tab_item">
			<span>
				<?php echo language('Signal');?>:
			</span>
			<div class="tab_item_right">
				<span><?php echo $gsm['signal']; ?></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('BER');?>:
			</span>
			<div class="tab_item_right">
				<span><?php echo $gsm['ber']; ?></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Status','Status');?>:
			</span>
			<div class="tab_item_right">
				<span><?php echo $gsm['state']; ?></span>
			</div>
		</div>
		
		<?php if($channel_module_type !== "cdma" && $channel_module_type !== "lte") { ?>
			<div class="tab_item">
				<span>
					<?php echo language('GSM Voice Codec');?>:
				</span>
				<div class="tab_item_right">
					<input type="checkbox" class="setting_sync" name="codec_selected_sync" title="<?php echo $lang_sync_title;?>"/>
					<select name="codec_selected" id="codec_selected">
					<?php
						foreach($codecs as $key => $value ) {
							$option_selected = '';
							if ($codec_selected == $key) {
								$option_selected = 'selected';
							}
					?>
							<option value=<?php if(isset($at_set_codec)) {echo $at_set_codec.$key;}?>  <?php echo $option_selected;?>> <?php echo $value;?> </option>
					<?php } ?>
					</select>
				</div>
			</div>
		<?php } ?>
		
		<div class="tab_item">
			<span>
				<?php echo language('CLCC');?>:
			</span>
			<div class="tab_item_right">
				<span><input type="checkbox" class="setting_sync" name="atd_clcc_flag_sync" title="<?php echo $lang_sync_title;?>"/></span>
				<span><input type="checkbox" class="enable" name="atd_clcc_flag" id="atd_clcc_flag" <?php echo $atd_clcc_flag; ?>/></span>
			</div>
		</div>
		
		<?php if($__MODULE_HEAD_ARRAY__[$board][$channel] == $__LTE_HEAD__){ ?>
		<div class="tab_item">
			<span>
				<?php echo language('CSQ flag');?>:
			</span>
			<div class="tab_item_right">
				<span><input type="checkbox" class="setting_sync" name="csq_flag_sync" title="<?php echo $lang_sync_title;?>"/></span>
				<span><input type="checkbox" class="enable" name="csq_flag" id="csq_flag" <?php echo $csq_flag; ?>/></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('CSQ Timeout');?>:
			</span>
			<div class="tab_item_right">
				<span id="ccsq_timeout"></span>
				<input type="checkbox" class="setting_sync" name="csq_timeout_sync" title="<?php echo $lang_sync_title;?>"/>
				<input type="text" name="csq_timeout" id="csq_timeout" value="<?php echo $csq_timeout; ?>"
				oninput="this.value=this.value.replace(/[^\d]*/g,'')" onkeyup="this.value=this.value.replace(/[^\d]*/g,'')" />
			</div>
		</div>
		<?php }?>
		
		<div class="tab_item">
			<span>
				<?php echo language('AT Timeout');?>:
			</span>
			<div class="tab_item_right">
				<span id="cat_timeout"></span>
				<input type="checkbox" class="setting_sync" name="at_timeout_sync" title="<?php echo $lang_sync_title;?>"/>
				<input type="text" name="at_timeout" id="at_timeout" value="<?php echo $at_timeout; ?>"
				oninput="this.value=this.value.replace(/[^\d]*/g,'')" onkeyup="this.value=this.value.replace(/[^\d]*/g,'')" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('CallerID Type');?>:
			</span>
			<div class="tab_item_right">
				<input type="checkbox" class="setting_sync" name="calleridtype_sync" title="<?php echo $lang_sync_title;?>" />
				<select name="calleridtype" id="calleridtype">
					<option value="0" <?php if($calleridtype == 0) echo 'selected';?> >CLIP</option>
					<option value="1" <?php if($calleridtype == 1) echo 'selected';?> >CCINFO</option>
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
						<b><?php echo language('Save To Other Ports');?>:</b><br/>
						<?php echo language('Save To Other Ports help','Save To Other Ports');?>
						
						<br/><br/>
						
						<b><?php echo language('Sync All Settings');?>:</b><br/>
						<?php echo language('Sync All Settings help','Sync All Settings');?>
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
						if(!strstr($port_name, $channel_module_type)) continue;
						
						echo "<td class='module_port'><input type='checkbox' name='spans[1][$i]' class='port'>";
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
				<input type="checkbox" id="all_settings_sync" onclick="selectAllCheckbox(this.checked,'class','setting_sync');" checked disabled />
				<span><?php echo language('Select all settings');?></span>
			</div>
		</div>
	</div>
	
	<input type="hidden" name="send" id="send" value="" />
	
	<div id="button_save">
		
		<?php if(!$only_view){ ?>
		<button type="submit" onclick="document.getElementById('send').value='Save';return check();"><?php echo language('Save');?></button>
		<button type="submit" onclick="document.getElementById('send').value='Apply';return check();" ><?php echo language('Apply');?></button>
		<?php } ?>
		
		<button type="button" onclick="window.location.href='<?php echo get_self();?>'" ><?php echo language('Cancel');?></button>
	</div>
	
	</form>
	
<script type="text/javascript">
	$(document).ready(function (){
		$("#float_button_3").mouseover(function(){
		  $("#float_button_3").css({opacity:"1",filter:"alpha(opacity=100)"});
		});
		$("#float_button_3").mouseleave(function(){
		  $("#float_button_3").css({opacity:"0.5",filter:"alpha(opacity=50)"});
		});
		float_sort_hide();
		$("#sort_out").mouseover(function(){
			if($("#sort_out").offset().left <= 5){
		   		float_sort_on();
			}
		});
		$("#sort_gsm_cli").mouseleave(function(){
			float_sort_hide();
		});
	});
	function float_sort_hide()
	{
		$("#sort_gsm_cli").stop().animate({left:"-198px"}, 300);
		$("#sort_out").stop().animate({left:"0px"}, 300);
	};
	function float_sort_on()
	{
		$("#sort_gsm_cli").animate({left:"0px"}, 300);
		$("#sort_out").animate({left:"198px"}, 300);
	};
	function get_carrier_list(channel,board)
	{
		$("#progress").show();
		var operator = $("#seloperator").val();
		if(board == '1'){
			var carrier_url = "/../../service?action=astcmd&cmd=gsm show valid operator "+channel+" 45000";
		} else {
			var carrier_url = "/" + board + "/service?action=astcmd&cmd=gsm show valid operator "+channel+" 45000";
		}
		$.ajax({
			url: carrier_url,
			type: 'GET',
			dataType: 'text',
			data: {},
			error: function(data){
				$("#progress").hide();
				confirm("<?php echo language('Failed to get list of operators.') ?>")
			},
			success: function(carrier_list){
				if(carrier_list.match("FAILED") == 'FAILED' || carrier_list == ''){
					$("#progress").hide();
					alert("<?php echo language('Failed to get list of operators.') ?>");
				} else {
					show_carrier_list(carrier_list,operator);
				}
			},
			complete: function(){
				$("#progress").hide();
			}
		});
	};
    function removeAllOption(){
       var obj=document.getElementById('seloperator');
		obj.options.length=0;
    };
	function show_carrier_list(carrier_list,operator){
		removeAllOption();
		var carrier_arr = Array();
		carrier_arr = carrier_list.split('\n');
		document.getElementById('seloperator').options.add(new Option('Auto','',true,false));
		for(i in carrier_arr) {
			if(carrier_arr[i] != ''){
	           var obj=document.getElementById('seloperator');
				//obj.add(new Option(carrier_arr[i],carrier_arr[i]));    
	         	//obj.options.add(new Option(carrier_arr[i],carrier_arr[i])); 
				var str = change_form_carrier(carrier_arr[i]);
				obj.options.add(new Option(str,str)); 
	         	if(carrier_arr[i] == operator){
	         		obj.value = operator;
	         	}
        	}
		}
	};
	function change_form_carrier(carrier_string) {
		if (carrier_string.indexOf(',') == -1) {
			return carrier_string;
		}
		split_arr = carrier_string.split(',');
		var changed_carrier_str = '';
		for(i in split_arr) {
			if (i > 0) {
				if (i < 3) {
					changed_carrier_str += split_arr[i] + ",";
				} 
				if (i==3) {
					changed_carrier_str += split_arr[i];
				}
				if (i == 4) {
					switch(split_arr[i]) {
						case '0' : 
								changed_carrier_str += ',GSM';
								break;
						case '2' : 
								changed_carrier_str += ',UTRAN';
								break;
						case '3' : 
								changed_carrier_str += ',GSM W/EGPRS';
								break;
						case '4' : 
								changed_carrier_str += ',UTRAN W/HSDPA';
								break;												
					}
				}
			}
		}
		return changed_carrier_str;
	};
	
	$(".band_class").change(function(){
		if($(this).attr('checked') == 'checked'){
			$("#default").removeAttr('checked');
		}
	});
	$("#default").change(function(){
		if($(this).attr('checked') == 'checked'){
			var flag = 0;
			$(".band_class").each(function(){
				if($(this).attr('checked') == 'checked'){
					flag = 1;
				}
			});
			if(flag == 1){
				if(confirm('<?php echo language('Default Band Tip','You have selected some bands, default clears the selected bands.'); ?>')){
					$(".band_class").removeAttr('checked');
				}else{
					$("#default").removeAttr('checked');
				}
			}
			
		}
	});
</script>
<!--
<div id="sort_out" class="sort_out">
</div>
	<div class="sort_gsm_cli" id="sort_gsm_cli">
		<div id="sort_info" class="sort_info" style="display:block">
		<?php
			$alldata = get_all_gsm_info();
			foreach($alldata as $bnum => $chs ) {
				foreach($chs as $cnum => $ch) {
					if(strstr(get_gsm_name_by_channel($cnum,$bnum),'null'))continue;
					if($cnum == $channel && $bnum == $board){
		?>
				<li><a style="color:#CD3278;" href="/cgi-bin/php/gsm-settings.php?sel_gsm=<?php echo $cnum; ?>&sel_brd=<?php echo $bnum; ?>&sel_slaveip=<?php echo $ch['slaveip']; ?>&send=Modify" ><?php echo get_gsm_name_by_channel($cnum,$bnum);?></a></li>
		<?php
					}else{
		?>
				<li><a style="color:LemonChiffon4;" href="/cgi-bin/php/gsm-settings.php?sel_gsm=<?php echo $cnum; ?>&sel_brd=<?php echo $bnum; ?>&sel_slaveip=<?php echo $ch['slaveip']; ?>&send=Modify" ><?php echo get_gsm_name_by_channel($cnum,$bnum);?></a></li>
		<?php
					}
				}
			}
		?>
		</div>
	</div>
-->
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
[1]
name=
vol=70
mic=1
lte_txgain=-1
lte_txdgain=-1
lte_rxgain=-1
dacgain=-15
adcgain=-3
dialprefix=
pin= 
custom_start_at= 
gsm_ec=off 
anonymouscall=off
band=ALL_BAND
needpin=
dl_step=1		//>0
dl_single_sw=off	//on/off
dl_single_limit=0
dl_total_sw=off		//on/off
dl_total_limit=0
dl_free_time=0		//free call duration(<step)
dl_warning_time=0
dl_warning_num=
dl_warning_describe=
dl_auto_reset_sw=off	//on/off
dl_auto_reset_type=off	//day/week/month
...
*/
	global $__GSM_SUM__;
	global $__BRD_SUM__;
	global $__GSM_HEAD__;
	global $__BRD_HEAD__;
	global $__UMTS_HEAD__;
	global $__MODULE_HEAD_ARRAY__;

	$post = $_POST;
	$post['sel_slaveip'] = '';
	unset($post['spans']);
	/*
	 *	get the type of channel module;
	 */
	$channel_module_name = get_gsm_name_by_channel($channel,$board,false);
	$res  = explode('-', $channel_module_name, 2);
	$channel_module_type = $res[0];

	/*
	 *
	 *	save to other ports
	 *
	 */
	//////////////////////////////////////////////////////////////////////////
	$sync = false;
	if(isset($_POST['spans']) && is_array($_POST['spans'])) { //It's save to other ports.
		//$post['sync'] = true;
		$sync = true;
		//$s = 0;
		for($i=1; $i<=$__GSM_SUM__; $i++) {
			if(isset($_POST['spans'][1][$i])) {
				$sync_port[$i] = $_POST['spans'][1][$i];
			//	$save[$s]['url'] = $_SERVER['HTTP_REFERER'];
			//	$save[$s]['channel'] = $i;
			//	$save[$s]['board'] = 1;
			//	$s++;
			}
		}
	}
	//POST data preprocessing
	if(isset($_POST['gsm_ec'])) {
		$_POST['gsm_ec'] = 'on';
	} else {
		$_POST['gsm_ec'] = 'off';
	}
	
	if(isset($_POST['atd_stk_flag'])){
		$_POST['atd_stk_flag'] = 'on';
	}else{
		$_POST['atd_stk_flag'] = 'off';
	}

	if(isset($_POST['clir'])) {
		$_POST['anonymouscall'] = 'on';
	} else {
		$_POST['anonymouscall'] = 'off';
	}
	
	if(isset($_POST['atd_clcc_flag'])){
		$_POST['atd_clcc_flag'] = 'on';
	}else{
		$_POST['atd_clcc_flag'] = 'off';
	}

	if(isset($_POST['csq_flag'])){
		$_POST['csq_flag'] = 'on';
	} else {
		$_POST['csq_flag'] = 'off';
	}

	if(isset($_POST['pin_sync'])) {
		$_POST['needpin_sync'] = '';
	}
	if(isset($_POST['needpin']) && is_true(trim($_POST['needpin']))) {
		$_POST['needpin'] = 'true';
	} else {
		$_POST['needpin'] = '';
	}

	$band_flag = 0;
	$band_str = '';
	if($channel_module_type == 'gsm'){
		$band_str = $_POST['band'];
	}else if($channel_module_type != 'cdma' || $channel_module_type != 'gsm' ) {
		if($channel_module_type == 'umts'){
			$band_str .= '3G,';
		}else if($channel_module_type == 'lte'){
			$band_str .= '4G,';
		}
		if (isset($_POST['band_gsm850'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_gsm850'].',';
		}
		if (isset($_POST['band_gsm900'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_gsm900'].',';
		}
		if (isset($_POST['band_gsm1800'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_gsm1800'].',';
		}
		if (isset($_POST['band_gsm1900'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_gsm1900'].',';
		}
		
		if (isset($_POST['band_wcdma850'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_wcdma850'].',';
		}
		if (isset($_POST['band_wcdma850_b6'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_wcdma850_b6'].',';
		}
		if (isset($_POST['band_wcdma850_b19'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_wcdma850_b19'].',';
		}
		if (isset($_POST['band_wcdma900'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_wcdma900'].',';
		}
		if (isset($_POST['band_wcdma1700'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_wcdma1700'].',';
		}
		if (isset($_POST['band_wcdma1900'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_wcdma1900'].',';
		}
		if (isset($_POST['band_wcdma2100'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_wcdma2100'].',';
		}
		
		if (isset($_POST['band_scdma_b34'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_scdma_b34'].',';
		}
		if (isset($_POST['band_scdma_b39'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_scdma_b39'].',';
		}
		
		if (isset($_POST['band_lte_b1'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b1'].',';
		}
		if (isset($_POST['band_lte_b2'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b2'].',';
		}
		if (isset($_POST['band_lte_b3'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b3'].',';
		}
		if (isset($_POST['band_lte_b4'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b4'].',';
		}
		if (isset($_POST['band_lte_b5'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b5'].',';
		}
		if (isset($_POST['band_lte_b7'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b7'].',';
		}
		if (isset($_POST['band_lte_b8'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b8'].',';
		}
		if (isset($_POST['band_lte_b12'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b12'].',';
		}
		if (isset($_POST['band_lte_b13'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b13'].',';
		}
		if (isset($_POST['band_lte_b18'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b18'].',';
		}
		if (isset($_POST['band_lte_b19'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b19'].',';
		}
		if (isset($_POST['band_lte_b20'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b20'].',';
		}
		if (isset($_POST['band_lte_b26'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b26'].',';
		}
		if (isset($_POST['band_lte_b28'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b28'].',';
		}
		if (isset($_POST['band_lte_b38'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b38'].',';
		}
		if (isset($_POST['band_lte_b39'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b39'].',';
		}
		if (isset($_POST['band_lte_b40'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b40'].',';
		}
		if (isset($_POST['band_lte_b41'])) {
			$band_flag = 1;
			$band_str .= $_POST['band_lte_b41'].',';
		}
		
		$band_str = substr($band_str,0,-1);
		
		if(isset($_POST['default'])){
			$band_flag = 1;
			$band_str .= ',NONE';
		}
		
		if($band_flag == 0){
			$band_str = '';
		}
	}
	/* single diallimit switch 
	if(isset($_POST['dl_single_sw'])){
		$_POST['dl_single_sw'] = 'on';
	}else{
		$_POST['dl_single_sw'] = 'off';
	}	
	if(isset($_POST['dl_single_sw_sync']) && is_true($_POST['dl_single_sw'])){
		$_POST['dl_single_limit_sync'] = 'on';
	}else{
		if(isset($_POST['dl_single_limit_sync']))	unset($_POST['dl_total_limit_sync']);
	}	*/

	/* total diallimit switch 
	if(isset($_POST['dl_total_sw'])){
		$_POST['dl_total_sw'] = 'on';
	} else {
		$_POST['dl_total_sw']= 'off';
	}

	if(isset($_POST['dl_total_sw_sync']) && is_true($_POST['dl_total_sw'])) {
		
		$_POST['dl_total_limit_sync'] = 'on';
		$_POST['dl_free_time_sync'] = 'on';
		$_POST['dl_warning_time_sync'] = 'on';
		$_POST['dl_warning_num_sync'] = 'on';
		$_POST['dl_warning_describe_sync'] = 'on';
		//$_POST['dl_auto_reset_sw_sync'] = 'on';
	} else {
		if(isset($_POST['dl_total_limit_sync']))	unset($_POST['dl_total_limit_sync']);
		if(isset($_POST['dl_free_time_sync']))		unset($_POST['dl_free_time_sync']);
		if(isset($_POST['dl_warning_time_sync']))	unset($_POST['dl_warning_time_sync']);
		if(isset($_POST['dl_warning_num_sync']))	unset($_POST['dl_warning_num_sync']);
		if(isset($_POST['dl_warning_describe_sync']))	unset($_POST['dl_warning_describe_sync']);
		if(isset($_POST['dl_auto_reset_sw_sync']))	unset($_POST['dl_auto_reset_sw_sync']);
	}*/
	
	/* diallimit auto reset 
	if(isset($_POST['dl_auto_reset_sw'])) {
		$_POST['dl_auto_reset_sw'] = 'on';
	}else{
		$_POST['dl_auto_reset_sw'] = 'off';
	}
	if(isset($_POST['dl_auto_reset_sw_sync']) && is_true($_POST['dl_auto_reset_sw'])){
		$_POST['dl_auto_reset_type_sync'] = 'on';
		$_POST['dl_auto_reset_date_sync'] = 'on';
	}else{
		if(isset($_POST['dl_auto_reset_type_sync']))	unset($_POST['dl_auto_reset_type_sync']);
		if(isset($_POST['dl_auto_reset_date_sync']))	unset($_POST['dl_auto_reset_date_sync']);
	}

	if(isset($_POST['dl_auto_reset_date']) && $_POST['dl_auto_reset_date']!=''){
		$date_array = explode(' ',$_POST['dl_auto_reset_date']);
		if(isset($date_array[1]))
			$date = $date_array[0].'-'.$date_array[1];
		else
			$date = '';
		$_POST['dl_auto_reset_date'] = $date;
	} else {
		$_POST['dl_auto_reset_date'] = '';
	}*/

	$default_array = array( 'name'=>'', 
				'vol'=>'',
				'mic'=>'',
				'lte_txgain'=>'11172',
				'lte_txdgain'=>'11172',
				'lte_rxgain'=>'60000',
				'dacgain'=>'',
				'adcgain'=>'',
				'dialprefix'=>'',
				'pin'=>'',
				'needpin'=>'',
				'custom_start_at'=>'',
				'gsm_ec'=>'off',
				'anonymouscall'=>'off',
				'band_mode'=>'',
				'band'=>'',
				'seloperator'=>'',
				'operator_fullname'=>'',
				'brd_mcu_serial'=>'',
				'hw_port'=>"$channel",
				'module_usb_com'=>'',
				// 'dl_step'=>'60',
				// 'dl_single_sw'=>'off',
				// 'dl_single_limit'=>'',
				// 'dl_total_sw'=>'off',
				// 'dl_total_limit'=>'',
				// 'dl_free_time'=>'0',
				// 'dl_warning_time'=>'0',
				// 'dl_warning_num'=>'',
				// 'dl_warning_describe'=>'',
				// 'dl_auto_reset_sw'=>'off',
				// 'dl_auto_reset_type'=>'day',
				'codec_selected'=>'',
				'atd_clcc_flag'=>'off',
				'atd_stk_flag'=>'off',
				'atd_stk_at'=>'',
				'csq_flag' => 'on',
				'csq_timeout' => '10',
				'at_timeout'=>'',
				'calleridtype'=>'0'
				);
				
	$modem_array = array( 'GSM'=>'0',
				'UTRAN'=>'2',
				'GSM W/EGPRS'=>'3',
				'UTRAN W/HSDPA'=>'4',
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
	
	//save full operator name to operator_fullname
	$seloperator = '';
	$fullname = '';
	if (isset($_POST['seloperator'])) {
		$fullname = $_POST['seloperator'];
		if(!strstr($fullname,',')) {
			$fullname = '';
		}
	} else {
		$fullname = '';
	}
	foreach($default_array as $key=>$value){
		if ($key != 'band') {
			if(isset($_POST[$key])){
					$val = trim($_POST[$key]);
			}else{
				$val = $value;
			}
		} else {
			if($channel_module_type != 'cdma' ) {
				if (isset($_POST[$key])) {
					$val = trim($_POST[$key]);
				} else {
					$val = $band_str;
				}
			} else {
				$val = '';
			}
		}
		
		if ($key == 'operator_fullname') {
			$val = $fullname;
		} else if ($key == 'codec_selected' && isset($_POST['codec_selected'])) {
			$val = 	$_POST['codec_selected'];
			wait_apply("exec","asterisk -rx \"gsm send sync at $channel $val 45000\" > /dev/null 2>&1 ");
			if (strrpos($val, ",") !== FALSE) {
				$val =	substr($val, strrpos($val, ",") + 1);
			} else {
				$val =	substr($val, strrpos($val, "=") + 1);
			}
			$_POST['codec_selected'] = $val;
		} else if($key == 'brd_mcu_serial' || $key == 'module_usb_com') {
			$val = $conf_array[$channel][$key];
		}
		
		if ($key == 'seloperator') {
			if (isset($_POST[$key])) {
				$str = $_POST[$key];
				if (strstr($str,",")) {
					$carrier_str = explode(",",$str);
					$carrier_code = $carrier_str[2];
					if ( $carrier_code ) {
						$val = trim($carrier_code);
						foreach ($modem_array as $modem=>$num) {
							if (isset($carrier_str[3]) && $carrier_str[3] == $modem) {
								$val .= ','.$num;
							}
						}
					} else {
						$val = $value;
					}
				} else {
					$val =$value;
				}
			} else {
				$val = $value;
			}
			$seloperator = $val;
		}
	
		if(isset($conf_array[$channel][$key])){
			$aql->assign_editkey($channel,$key,$val);
		}else{
			$aql->assign_append($channel,$key,$val);
		}

	}
	/*
	 *	save parameters of current port to other ports synchonously
	 *
	 */
	 
	if($sync) {
		foreach($sync_port as $port => $value) {
 			$sel_channel_para = $aql->query("select * from gw_gsm.conf where section='$port'");
			if(isset($_POST['band_sync'])){
				$_POST['band'] = $band_str;
			}
			
 			foreach($_POST as $key => $value){
 				if(isset($_POST[$key.'_sync'])){
 					if(isset($sel_channel_para[$port][$key])){
						$aql->assign_editkey($port,$key,$_POST[$key]);
					}else{
						$aql->assign_append($port,$key,$_POST[$key]);
					}
 				}
				
				if($key.'_sync' == 'seloperator_sync'){
					$aql->assign_editkey($port,'seloperator',$seloperator);
					
					if(isset($sel_channel_para[$port]['operator_fullname'])){
						$aql->assign_editkey($port,'operator_fullname',$fullname);
					}else{
						$aql->assign_append($port,'operator_fullname',$fullname);
					}
				}
			}
			/*
			if (isset($_POST['dl_total_sw_sync']) && is_true($_POST['dl_total_sw'])){
				if(isset($_POST['dl_total_limit_sync'])){
					$remain_time = trim($_POST['dl_total_limit']);  
				} else {
					$remain_time = $sel_channel_para[$port]['dl_total_limit'];
				}
				if (isset($_POST['dl_auto_reset_sw_sync'])){
					$auto_reset_date = trim($_POST['dl_auto_reset_date']);
				} else {
					$auto_reset_date = $sel_channel_para[$port]['dl_auto_reset_date'];
				}
				if(isset($_POST['dl_total_limit_sync']) || isset($_POST['dl_auto_reset_date'])){
					set_remain_time_and_auto_reset_date($port, $remain_time, $auto_reset_date);
				}
			}*/
		}
	}
	if (!$aql->save_config_file($conf_file)) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	//diallimit reset
	///////////////////////////////////////////////////////////////////////////////////////
	/*
	$remain_time = '';
	$auto_reset_date = '';
	if(isset($_POST['dl_total_sw']) && is_true($_POST['dl_total_sw'])){	//total diallimit switched is on
		if(isset($_POST['dl_total_limit'])){
			if($sync == false || ($sync && isset($_POST['dl_total_limit_sync']))) {		//check selected port or POST port
				if(isset($conf_array[$channel]['dl_total_sw'])){
					$total_limit_sw_old = trim($conf_array[$channel]['dl_total_sw']);
				}else{
					$total_limit_sw_old = 'off';
				}
				if(isset($conf_array[$channel]['dl_total_limit'])){
					$total_limit_old = trim($conf_array[$channel]['dl_total_limit']);
				}else{
					$total_limit_old = '';
				}
				$total_limit = trim($_POST['dl_total_limit']);
				if($total_limit_sw_old == 'off' || $total_limit_old != $total_limit){
					$remain_time = $total_limit;
				}
			}
		}
		if(isset($_POST['dl_auto_reset_sw']) && is_true($_POST['dl_auto_reset_sw'])){	//auto reset switch is on
			if(isset($_POST['dl_auto_reset_date'])){
				if($sync == false || ($sync && isset($_POST['dl_auto_reset_date_sync']))) {	//check selected port or POST port
					$auto_reset_date = trim($_POST['dl_auto_reset_date']);
				}
			}
		} 
	}*/
	///////////////////////////////////////////////////////////////////////////////////////
	
	/*
	if($channel_module_type == 'lte'){
		if($conf_array[$channel]['lte_txgain'] != $_POST['lte_txgain'] || $conf_array[$channel]['lte_txdgain'] != $_POST['lte_txdgain'] || $conf_array[$channel]['lte_rxgain'] != $_POST['lte_rxgain']){
			if($conf_array[$channel]['lte_txgain'] != $_POST['lte_txgain'] || $conf_array[$channel]['lte_txdgain'] != $_POST['lte_txdgain']){
				($conf_array[$channel]['lte_txgain'] != $_POST['lte_txgain'])?$lte_txgain = $_POST['lte_txgain']:$lte_txgain = $conf_array[$channel]['lte_txgain'];
				($conf_array[$channel]['lte_txdgain'] != $_POST['lte_txdgain'])?$lte_txdgain = $_POST['lte_txdgain']:$lte_txdgain = $conf_array[$channel]['lte_txdgain'];
				wait_apply("exec","asterisk -rx \"gsm set txgain $channel $lte_txgain $lte_txdgain 45000\" ");
			}
			if($conf_array[$channel]['lte_rxgain'] != $_POST['lte_rxgain']){
				$lte_rxgain = $_POST['lte_rxgain'];
				wait_apply("exec","asterisk -rx \"gsm set rxgain $channel $lte_rxgain 45000\"");
			}
		}else{
			wait_apply("exec","sleep 6 > /dev/null 2>&1 ");
			wait_apply("exec", "asterisk -rx \"extra restart\" > /dev/null 2>&1 &");
		}
	}else{
		wait_apply("exec","sleep 6 > /dev/null 2>&1 ");
		wait_apply("exec", "asterisk -rx \"extra restart\" > /dev/null 2>&1 &");
	}*/
	
	save_gsm_to_extra_conf();
	//set_remain_time_and_auto_reset_date($channel,$remain_time,$auto_reset_date);
	
	wait_apply("exec","sleep 6 > /dev/null 2>&1 ");
	wait_apply("exec", "asterisk -rx \"extra restart\" > /dev/null 2>&1 &");
	
	save_user_record("","MODULE->Module Settings:Save,channel=".$channel);
	return true;
}

/*function IMEI_modify($channel,$imei,$slaveip = '')
{
	//modify_imei($channel, $imei, 'power_reset','no', $slaveip);
	if($slaveip == '') {
		//exec("asterisk -rx \"gsm send at $channel AT+EGMR=1,7,\\\\\\\"$imei\\\\\\\"\" 2> /dev/null");
		exec("asterisk -rx \"gsm set imei $channel $imei\" 2> /dev/null");
		exec("asterisk -rx \"gsm power reset $channel\"  2> /dev/null");
	} else {
		//$data = "astcmd:gsm send at $channel AT+EGMR=1,7,\\\\\\\"$imei\\\\\\\"\n";
		$data = "astcmd:gsm set imei $channel $imei\n";
		request_slave($slaveip, $data, 5, false);

		$data = "astcmd:gsm power reset $channel\n";
		request_slave($slaveip, $data, 5, false);
	}
}*/

function modify_SMSC($channel,$smsc,$slaveip = '')
{
	
	if($slaveip == '') {
		exec("asterisk -rx \"gsm send at $channel AT+CSCA=\\\\\\\"$smsc\\\\\\\"\" 2> /dev/null");
		exec("asterisk -rx \"gsm power reset $channel\" 2> /dev/null");
	}
}

function diallimit_reset($channel,$slaveip = '')
{
	
	if($slaveip == '') {
		exec("asterisk -rx \"gsm diallimit reset $channel\" 2>/dev/null");
	}
}

?>


<?php
$check_float = 0;
$cluster_info = get_cluster_info();

if($_POST || $_GET) {
	if (isset($_POST['send']) && $_POST['send'] == 'Save') {
		if($only_view){
			return false;
		}
		
		if(isset($_POST['sel_gsm']) && $_POST['sel_gsm'] != '') {
			if(isset($_POST['sel_slaveip'])) {
				if(isset($_POST['sel_brd'])) {
					if ($cluster_info['mode'] == 'slave') {
						$_POST['sel_brd'] = 1;
					}
					save_gsm($_POST['sel_gsm'],$_POST['sel_slaveip'],$_POST['sel_brd']);
				}
			}
		}
		show_gsms();
	} else if (isset($_POST['send']) && $_POST['send'] == 'Apply') {
		if($only_view){
			return false;
		}
		
		if(isset($_POST['sel_gsm']) && $_POST['sel_gsm'] != '') {
			if(isset($_POST['sel_slaveip'])) {
				if(isset($_POST['sel_brd'])) {
					if ($cluster_info['mode'] == 'slave') {
						$_POST['sel_brd'] = 1;
					}
					$check_float = 1;
					save_gsm($_POST['sel_gsm'],$_POST['sel_slaveip'],$_POST['sel_brd']);
					edit_gsm($_POST['sel_gsm'],$_POST['sel_slaveip'],$_POST['sel_brd']);
				}
			}
		}
	} else if (isset($_POST['send']) && $_POST['send'] == 'dl_reset') {//Discard
		if(isset($_POST['sel_gsm']) && $_POST['sel_gsm'] != '') {
			if(isset($_POST['sel_slaveip'])) {
				if(isset($_POST['sel_brd'])) {
					if ($cluster_info['mode'] == 'slave') {
						$_POST['sel_brd'] = 1;
					}
					diallimit_reset($_POST['sel_gsm'],$_POST['sel_slaveip']);
					$check_float = 1;
					edit_gsm($_POST['sel_gsm'],$_POST['sel_slaveip'],$_POST['sel_brd']);
				}
			}
		}
	} else if (isset($_GET['send']) && $_GET['send'] == 'Modify') {
		if(isset($_GET['sel_gsm']) && $_GET['sel_gsm'] != '') {
			if(isset($_GET['sel_slaveip'])) {
				if(isset($_GET['sel_brd'])) {
					if ($cluster_info['mode'] == 'slave') {
						$_GET['sel_brd'] = 1;
					}
					$check_float = 1;
					edit_gsm($_GET['sel_gsm'],$_GET['sel_slaveip'],$_GET['sel_brd']);
				}
			}
		}
	} else if(isset($_POST['new_imei']) && $_POST['new_imei'] != '') {//Discard
		$new_imei = trim($_POST['new_imei']);
		if($new_imei) {
			if(isset($_POST['sel_gsm']) && $_POST['sel_gsm'] != '') {
				if(isset($_POST['sel_slaveip'])) {
					IMEI_modify($_POST['sel_gsm'],$new_imei,$_POST['sel_slaveip']);
				}
				//edit_gsm($_POST['sel_gsm'],$_POST['sel_slaveip'],$_POST['sel_brd']);
			}
		}
		show_gsms();
	} else if(isset($_POST['new_smsc']) && $_POST['new_smsc'] != '') {
		$new_smsc = trim($_POST['new_smsc']);
		if($new_smsc) {
			if(isset($_POST['sel_gsm']) && $_POST['sel_gsm'] != '') {
				if(isset($_POST['sel_slaveip'])) {
					modify_SMSC($_POST['sel_gsm'],$new_smsc,$_POST['sel_slaveip']);
				}
				//edit_gsm($_POST['sel_gsm'],$_POST['sel_slaveip'],$_POST['sel_brd']);
			}
		}
		show_gsms();
	}

} else {
	show_gsms();
}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>