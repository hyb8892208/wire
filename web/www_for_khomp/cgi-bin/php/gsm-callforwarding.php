<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
include_once("/www/cgi-bin/inc/aql.php");
?>

<?php
$board = get_slotnum();
$js_board_str = '"'.$board.'"';

//phone number switch
$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$phonenum_res = $aql->query("select * from sim_query.conf");

function cf_info($c,$b)
{
?>
	<table width="100%" cellpadding="0" cellspacing="0" class="tedit_tab">
		<tr>
			<td width="20%" align="center">
				<input name="<?php echo 'cf_type'.$b.'_'.$c ?>" id="<?php echo 'uncon_'.$b.'_'.$c ?>" type="radio" onClick="cf_change(<?php echo $b.','.$c; ?>);" >
			</td>
			<td width="28%" style="text-align:left;"><?php echo language('Call Forwarding Unconditional');?></td>
			<td width="42%" style="text-align:left;">
				<input type="text" id="<?php echo 'uncon_num_'.$b.'_'.$c ?>" maxlength="31" name="<?php echo 'uncon_num_'.$b.'_'.$c ?>" size="18" value=""><span id="<?php echo 'cuncon_num_'.$b.'_'.$c ?>" style="color:red"></span>
			</td>
			<td width="30%" id="<?php echo 'status_uncon_'.$b.'_'.$c;?>"></td>
		</tr>
		<tr>
			<td width="20%" rowspan="3" align="center">
				<input name="<?php echo 'cf_type'.$b.'_'.$c ?>" id="<?php echo 'con_'.$b.'_'.$c ?>" type="radio" onClick="cf_change(<?php echo $b.','.$c; ?>);" >
			</td>
			<td width="28%" style="text-align:left;">
				<input id="<?php echo 'noreply_box_'.$b.'_'.$c ?>" name="<?php echo 'noreply_box_'.$b.'_'.$c ?>" type="checkbox" onClick="cf_change(<?php echo $b.','.$c; ?>);"  ><?php echo language('Call Forwarding No Reply');?>
			</td>
			<td width="42%" style="text-align:left;">
				<input type="text" id="<?php echo 'noreply_num_'.$b.'_'.$c ?>" maxlength="31" name="<?php echo 'noreply_num_'.$b.'_'.$c ?>" size="18" value=""><span id="<?php echo 'cnoreply_num_'.$b.'_'.$c ?>" style="color:red"></span>
			</td>
			<td width="30%" id="<?php echo 'status_noreply_'.$b.'_'.$c;?>"></td>
		</tr>
		<tr>
			<td width="28%" style="text-align:left;">
				<input id="<?php echo 'busy_box_'.$b.'_'.$c ?>" name="<?php echo 'busy_box_'.$b.'_'.$c ?>" type="checkbox" onClick="cf_change(<?php echo $b.','.$c; ?>);"  ><?php echo language('Call Forwarding Busy');?>
			</td>
			<td width="42%" style="text-align:left;">
				<input type="text" id="<?php echo 'busy_num_'.$b.'_'.$c ?>" maxlength="31" name="<?php echo 'busy_num_'.$b.'_'.$c ?>" size="18" value=""><span id="<?php echo 'cbusy_num_'.$b.'_'.$c ?>" style="color:red"></span>
			</td>
			<td width="30%" id="<?php echo 'status_busy_'.$b.'_'.$c;?>"></td>
		</tr>    
		<tr>
			<td width="28%" style="text-align:left;">
				<input id="<?php echo 'noreg_box_'.$b.'_'.$c ?>" name="<?php echo 'noreg_box_'.$b.'_'.$c ?>" type="checkbox" onClick="cf_change(<?php echo $b.','.$c; ?>);" ><?php echo language('Call Forward on Not Reachable');?> 
			</td>
			<td width="42%" style="text-align:left;">
				<input type="text" id="<?php echo 'noreg_num_'.$b.'_'.$c ?>" maxlength="31" name="<?php echo 'noreg_num_'.$b.'_'.$c ?>" size="18" value=""><span id="<?php echo 'cnoreg_num_'.$b.'_'.$c ?>" style="color:red"></span>
			</td>
			<td width="30%" id="<?php echo 'status_noreg_'.$b.'_'.$c;?>"></td>
		</tr> 	   
		<tr>
			<td width="20%" align="center">
				<input name="<?php echo 'cf_type'.$b.'_'.$c ?>" id="<?php echo 'cancel_'.$b.'_'.$c ?>" type="radio" onClick="cf_change(<?php echo $b.','.$c; ?>);" >
			</td>
			<td colspan=2 width="50%" style="text-align:left;"><?php echo language('Cancel All');?></td>
			<td width="30%" id="<?php echo 'status_cancel_'.$b.'_'.$c;?>"></td>
		</tr>		  
	</table>
<?php
}
?>

	<script type="text/javascript" src="/js/functions.js"></script>
	
	<div class="content">
		<table class="table_show">
			<tr>
				<th style="width: 4%;"><input type="checkbox" name="selall" onclick="selectAll(this.checked,'selinput')" /></th>
				<th style="width: 10%;"><?php echo language('Port');?></th>
				
				<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
				<th><?php echo language('Mobile Number');?></th>
				<?php } ?>
				
				<th style="width: 70%;padding-left:0;">
					<table width="100%" cellpadding="0" cellspacing="0" style="border:none;">
						<tr>
							<th width="20%" align="center" style="border:none;border-right:1px solid #ccc;text-align:center;padding:0;"><?php echo language('Select');?></th>
							<th width="28%" style="border:none;border-right:1px solid #ccc;text-align:center;padding:0;"><?php echo language('Call Type');?></th>
							<th width="42%" style="border:none;border-right:1px solid #ccc;text-align:center;padding:0;"><?php echo language('Call Number');?></th>
							<th width="30%" style="border:none;border-right:1px solid #ccc;text-align:center;padding:0;"><?php echo language('Status');?></th>
						</tr>
					</table>
				</th>
			</tr>
			
			<?php
			for($i=1; $i<=$__GSM_SUM__; $i++) {
				$display = '';
				$channel_name = get_gsm_name_by_channel($i);
				if(strstr($channel_name, 'cdma') || strstr($channel_name, 'null')) $display = "style='display:none;'";
				
				$phonenum = '';
				if(($phonenum_res[$i]['query_type']&240) != 0){
					exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $i",$phone_output);
					$phonenum = $phone_output[0];
					$phone_output = '';
				}
			?>
					<tr <?php echo $display;?>>
						<td><input type="checkbox" name="selinput" value="" id="<?php echo $board."_".$i; ?>" /></td>
						<td><?php echo $channel_name; ?></td>				
						
						<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
						<td><?php echo $phonenum;?></td>
						<?php } ?>
						
						<td style="padding-left:0;">
							<?php cf_info($i,$board)?>
						</td>
					</tr>
			<?php } ?>
		</table>
	</div>
	
	<div id="button_save">
		<button type="button" id="query_btn" onclick="query();" ><?php echo language('Query');?></button>
		<button type="button" id="setting_btn" onclick="setting()"><?php echo language('Setting');?></button>
		<div id="progress" style="float:right;margin-top:10px;display:none;">
			<img src="/images/mini_loading.gif" align="middle" />
		</div>
	</div>

	<script type="text/javascript">
	
	$(document).ready(refresh__table);
	
	function refresh__table()
	{
		var bs = new Array(<?php echo $js_board_str;?>);
		var gsm_sum = <?php echo $__GSM_SUM__; ?>;
		for(i in bs) {
			for(var c = 1; c <= gsm_sum; c++){
				$("#status_noreg_"+bs[i]+"_"+c).html('');
				$("#status_noreply_"+bs[i]+"_"+c).html('');
				$("#status_busy_"+bs[i]+"_"+c).html('');
				$("#status_uncon_"+bs[i]+"_"+c).html('');
				$("#noreg_box_"+bs[i]+"_"+c).attr({"checked":false});
				$("#busy_box_"+bs[i]+"_"+c).attr({"checked":false});
				$("#noreply_box_"+bs[i]+"_"+c).attr({"checked":false});
				$("#noreg_num_"+bs[i]+"_"+c).val('');
				$("#noreply_num_"+bs[i]+"_"+c).val('');
				$("#busy_num_"+bs[i]+"_"+c).val('');
				$("#uncon_num_"+bs[i]+"_"+c).val('');
				$("#uncon_"+bs[i]+"_"+c).attr({"checked":false});
				$("#con_"+bs[i]+"_"+c).attr({"checked":false});
				$("#cancel_"+bs[i]+"_"+c).attr({"checked":false});
				document.getElementById("uncon_num_"+bs[i]+"_"+c).disabled = 'disabled';
				document.getElementById("busy_num_"+bs[i]+"_"+c).disabled = 'disabled';
				document.getElementById("noreply_num_"+bs[i]+"_"+c).disabled = 'disabled';	
				document.getElementById("noreg_num_"+bs[i]+"_"+c).disabled = 'disabled';
			}
		}
	}
	
	function check(b,c)
	{
		/* check call forward settings */
			if($("#uncon_"+b+"_"+c).attr("checked")=="checked"){
				if(!check_cfinfo_number("uncon_num_"+b+"_"+c))return false;
			}
			if($("#con_"+b+"_"+c).attr("checked")=="checked"){
				if($("#noreply_box_"+b+"_"+c).attr("checked")=="checked"){
					if(!check_cfinfo_number("noreply_num_"+b+"_"+c))return false;
				}
				if($("#busy_box_"+b+"_"+c).attr("checked")=="checked"){
					if(!check_cfinfo_number("busy_num_"+b+"_"+c))return false;
				}
				if($("#noreg_box_"+b+"_"+c).attr("checked")=="checked"){
					if(!check_cfinfo_number("noreg_num_"+b+"_"+c))return false;
				}
			}
		return true;
	}
		
	function check_cfinfo_number(id)
	{
		var obj = document.getElementById(id);
		val = obj.value;
		if(check_cf_number(val)) {
			$("#c"+id).html("<?php echo language('js check call forward integer','Please input the correct number!');?>");
			return false;
		} else {
			$("#c"+id).html("");
			return true;
		}

	}

	function check_cf_number(str)
	{
		var rex=/^(\+)?\d+$/i;
		if(rex.test(str)) {
			return false;
		}
	
		return true;
	}

	function cf_change(b,c)
	{
		if(document.getElementById("con_"+b+"_"+c).checked){
			document.getElementById("uncon_num_"+b+"_"+c).disabled = 'disabled';
			document.getElementById("busy_box_"+b+"_"+c).disabled = '';
			document.getElementById("noreply_box_"+b+"_"+c).disabled = '';
			document.getElementById("noreg_box_"+b+"_"+c).disabled = '';
			if(document.getElementById("busy_box_"+b+"_"+c).checked){
				document.getElementById("busy_num_"+b+"_"+c).disabled = '';
			}else{
				document.getElementById("busy_num_"+b+"_"+c).disabled = 'disabled';
			}
			if(document.getElementById("noreply_box_"+b+"_"+c).checked){
				document.getElementById("noreply_num_"+b+"_"+c).disabled = '';
			}else{
				document.getElementById("noreply_num_"+b+"_"+c).disabled = 'disabled';	
			}
			if(document.getElementById("noreg_box_"+b+"_"+c).checked){
				document.getElementById("noreg_num_"+b+"_"+c).disabled = '';
			}else{
				document.getElementById("noreg_num_"+b+"_"+c).disabled = 'disabled';
			}
		}
		if(document.getElementById("uncon_"+b+"_"+c).checked){
			document.getElementById("uncon_num_"+b+"_"+c).disabled = '';
			document.getElementById("noreply_num_"+b+"_"+c).disabled = 'disabled';
			document.getElementById("noreply_box_"+b+"_"+c).disabled = 'disabled';
			document.getElementById("busy_num_"+b+"_"+c).disabled = 'disabled';
			document.getElementById("busy_box_"+b+"_"+c).disabled = 'disabled';
			document.getElementById("noreg_num_"+b+"_"+c).disabled = 'disabled';
			document.getElementById("noreg_box_"+b+"_"+c).disabled = 'disabled';
		}
		if(document.getElementById("cancel_"+b+"_"+c).checked){
			document.getElementById("uncon_num_"+b+"_"+c).disabled = 'disabled';
			document.getElementById("noreply_num_"+b+"_"+c).disabled = 'disabled';
			document.getElementById("noreply_box_"+b+"_"+c).disabled = 'disabled';
			document.getElementById("busy_num_"+b+"_"+c).disabled = 'disabled';
			document.getElementById("busy_box_"+b+"_"+c).disabled = 'disabled';
			document.getElementById("noreg_num_"+b+"_"+c).disabled = 'disabled';
			document.getElementById("noreg_box_"+b+"_"+c).disabled = 'disabled';
		}
	}

	function refresh_status_and_check_info(board,port)
	{
		$("#status_noreg_"+board+"_"+port).html('');
		$("#status_noreply_"+board+"_"+port).html('');
		$("#status_busy_"+board+"_"+port).html('');
		$("#status_uncon_"+board+"_"+port).html('');
		$("#status_cancel_"+board+"_"+port).html('');
		$("#cnoreg_num_"+board+"_"+port).html('');
		$("#cnoreply_num_"+board+"_"+port).html('');
		$("#cbusy_num_"+board+"_"+port).html('');
		$("#cuncon_num_"+board+"_"+port).html('');
	}

	$(function(){
		var bs = new Array(<?php echo $js_board_str;?>);
		var gsm_sum = <?php echo $__GSM_SUM__; ?>;
		var check_num = 0;
		
		for(i in bs) {
			for(var c = 1; c <= gsm_sum; c++){
				check_num++;
				
				refresh_all_info(bs[i],c);
				$("#status_uncon_"+bs[i]+"_"+c).html("<?php echo language('QUERYING','QUERYING');?>" + "...");
				$("#status_busy_"+bs[i]+"_"+c).html("<?php echo language('QUERYING','QUERYING');?>" + "...");
				$("#status_noreply_"+bs[i]+"_"+c).html("<?php echo language('QUERYING','QUERYING');?>" + "...");
				$("#status_noreg_"+bs[i]+"_"+c).html("<?php echo language('QUERYING','QUERYING');?>" + "...");
				query_all_info(bs[i],c,check_num);
			}
		}
	});

	function query()
	{
		var bs = new Array(<?php echo $js_board_str;?>);
		var gsm_sum = <?php echo $__GSM_SUM__; ?>;
		var check_num = 0;
		$("#float_button_1").css({opacity:"0.5",filter:"alpha(opacity=50)"});
		if(!query_check()) return;
		function query_check()
		{
			for(i in bs) {
				for(var c = 1; c <= gsm_sum; c++){
					refresh_status_and_check_info(bs[i],c);
					if(document.getElementById(bs[i]+"_"+c).checked == true){
						check_num++;
					}
				}
			}

			if(check_num == 0){
				confirm("<?php echo language('Select least warning','Please select at least one port!'); ?>");
				return false;
			}else{
				if(!confirm("<?php echo language('Query Warning','Notice: This function need to request the data from GSM network,there will be some delay.');?>")) return;
				$('#query_btn').attr("disabled","true");
				$('#setting_btn').attr("disabled","true");
				$("#query_btn").removeClass('gen_short_btn').addClass('gen_short_cli_btn');
				$("#setting_btn").removeClass('gen_short_btn').addClass('gen_short_cli_btn');
				$('.gen_btn').addClass("click_gen_btn");
				$('.float_short_button').attr("disabled","true");
				$('#progress').show();
				return true;
			}
		}


		for(i in bs) {
			for(var c = 1; c <= gsm_sum; c++){
				if(document.getElementById(bs[i]+"_"+c).checked == true){
					refresh_all_info(bs[i],c);
					$("#status_uncon_"+bs[i]+"_"+c).html("<?php echo language('QUERYING','QUERYING');?>" + "...");
					$("#status_busy_"+bs[i]+"_"+c).html("<?php echo language('QUERYING','QUERYING');?>" + "...");
					$("#status_noreply_"+bs[i]+"_"+c).html("<?php echo language('QUERYING','QUERYING');?>" + "...");
					$("#status_noreg_"+bs[i]+"_"+c).html("<?php echo language('QUERYING','QUERYING');?>" + "...");
					query_all_info(bs[i],c,check_num);
				}
			}
		}
	}
	
	function query_all_info(board, port,check_num) {
		var times = <?php if(isset($_GET['timeout'])) echo $_GET['timeout'];else echo 1;?>;
		var timeout = 40000 * times;
		var query_num = 0;
		
		if(board == 1){
			url = "/service?action=get_cfinfo&span=" + port + "&timeout=" + timeout+"random="+Math.random();
		}else{
			url = "/" + board + "/service?action=get_cfinfo&span=" + port + "&timeout=" + timeout+"random="+Math.random();
		}
		$.ajax({
			url: url,
			type: 'GET',
			dataType: 'json',
			data: {},
			timeout:timeout*5,
			success: function(atreport){  //+CCFC:0,7 or +CCFC:1,1,"10086"
				if(atreport){
					display_cf_info(atreport,board,port);
				}else{
					$("#status_uncon_"+board+"_"+port).html("<?php echo language('TIMEOUT','TIMEOUT');?>");
					$("#status_busy_"+board+"_"+port).html("<?php echo language('TIMEOUT','TIMEOUT');?>");
					$("#status_noreply_"+board+"_"+port).html("<?php echo language('TIMEOUT','TIMEOUT');?>");
					$("#status_noreg_"+board+"_"+port).html("<?php echo language('TIMEOUT','TIMEOUT');?>");
				}
			},
			complete: function(){
				query_num++;
				if(query_num == check_num){
					$('#query_btn').removeAttr("disabled");
					$('#setting_btn').removeAttr("disabled");
					$("#query_btn").removeClass('gen_short_cli_btn').addClass('gen_short_btn');
					$("#setting_btn").removeClass('gen_short_cli_btn').addClass('gen_short_btn');
					$('.gen_btn').removeClass("click_gen_btn");
					$('.float_short_button').removeAttr("disabled");
					$('#progress').hide();
				}
			},
		});
	};

	function display_cf_info(atreport,board,port)
	{
		if(atreport.Unconditional == "OFF" || atreport.Unconditional == "TIMEOUT" 
		   || atreport.Unconditional == "UNKNOWN" || atreport.Unconditional == "NOT READY"){

			if(atreport.Unconditional == "OFF"){
				$("#status_uncon_"+board+"_"+port).html("<?php echo language('SUCCESS','SUCCESS');?>");
			} else {
				$("#status_uncon_"+board+"_"+port).html(atreport.Unconditional);
			}

			if(atreport.Busy == "OFF"){
				$("#status_busy_"+board+"_"+port).html("<?php echo language('SUCCESS','SUCCESS');?>");
			}else if(atreport.Busy == "UNKNOWN" || atreport.Busy == "TIMEOUT" || atreport.Busy == "NOT READY"){
				$("#status_busy_"+board+"_"+port).html(atreport.Busy);
			}else{
				document.getElementById("busy_num_"+board+"_"+port).disabled = '';
				document.getElementById("busy_box_"+board+"_"+port).checked = 'checked';
				$("#con_"+board+"_"+port).attr({"checked":true});
				$("#busy_num_"+board+"_"+port).val(atreport.Busy);
				$("#status_busy_"+board+"_"+port).html("<?php echo language('SUCCESS','SUCCESS');?>");
			}

			if(atreport.NoReply == "OFF"){
				$("#status_noreply_"+board+"_"+port).html("<?php echo language('SUCCESS','SUCCESS');?>");
			}else if(atreport.NoReply == "UNKNOWN" || atreport.NoReply == "TIMEOUT" || atreport.NoReply == "NOT READY"){
				$("#status_noreply_"+board+"_"+port).html(atreport.NoReply);
			}else{
				document.getElementById("noreply_num_"+board+"_"+port).disabled = '';
				document.getElementById("noreply_box_"+board+"_"+port).checked = 'checked';
				$("#con_"+board+"_"+port).attr({"checked":true});
				$("#noreply_num_"+board+"_"+port).val(atreport.NoReply);
				$("#status_noreply_"+board+"_"+port).html("<?php echo language('SUCCESS','SUCCESS');?>");
			}

			if(atreport.NotReachable == "OFF"){
				$("#status_noreg_"+board+"_"+port).html("<?php echo language('SUCCESS','SUCCESS');?>");
			}else if(atreport.NotReachable == "UNKNOWN" || atreport.NotReachable == "TIMEOUT" || atreport.NotReachable == "NOT READY" ){
				$("#status_noreg_"+board+"_"+port).html(atreport.NotReachable);
			}else{
				document.getElementById("noreg_num_"+board+"_"+port).disabled = '';
				document.getElementById("noreg_box_"+board+"_"+port).checked = 'checked';
				$("#con_"+board+"_"+port).attr({"checked":true});
				$("#noreg_num_"+board+"_"+port).val(atreport.NotReachable);
				$("#status_noreg_"+board+"_"+port).html("<?php echo language('SUCCESS','SUCCESS');?>");
			}
		}else{
			document.getElementById("uncon_num_"+board+"_"+port).disabled = '';
			$("#uncon_"+board+"_"+port).attr({"checked":true});
			$("#uncon_num_"+board+"_"+port).val(atreport.Unconditional);
			$("#status_uncon_"+board+"_"+port).html("<?php echo language('SUCCESS','SUCCESS');?>");
			$("#status_noreply_"+board+"_"+port).html("<?php echo language('SUCCESS','SUCCESS');?>");
			$("#status_busy_"+board+"_"+port).html("<?php echo language('SUCCESS','SUCCESS');?>");
			$("#status_noreg_"+board+"_"+port).html("<?php echo language('SUCCESS','SUCCESS');?>");
		}
	}
	
	function refresh_all_info(board,port) {
		$("#status_noreg_"+board+"_"+port).html('');
		$("#status_noreply_"+board+"_"+port).html('');
		$("#status_busy_"+board+"_"+port).html('');
		$("#status_uncon_"+board+"_"+port).html('');
		$("#status_cancel_"+board+"_"+port).html('');
		$("#noreg_box_"+board+"_"+port).attr({"checked":false});
		$("#busy_box_"+board+"_"+port).attr({"checked":false});
		$("#noreply_box_"+board+"_"+port).attr({"checked":false});
		$("#noreg_num_"+board+"_"+port).val('');
		$("#noreply_num_"+board+"_"+port).val('');
		$("#busy_num_"+board+"_"+port).val('');
		$("#uncon_num_"+board+"_"+port).val('');
		$("#cnoreg_num_"+board+"_"+port).html('');
		$("#cnoreply_num_"+board+"_"+port).html('');
		$("#cbusy_num_"+board+"_"+port).html('');
		$("#cuncon_num_"+board+"_"+port).html('');
		$("#uncon_"+board+"_"+port).attr({"checked":false});
		$("#con_"+board+"_"+port).attr({"checked":false});
		$("#cancel_"+board+"_"+port).attr({"checked":false});
		document.getElementById("uncon_num_"+board+"_"+port).disabled = 'disabled';
		document.getElementById("busy_num_"+board+"_"+port).disabled = 'disabled';
		document.getElementById("noreply_num_"+board+"_"+port).disabled = 'disabled';	
		document.getElementById("noreg_num_"+board+"_"+port).disabled = 'disabled';
	};


	function setting()
	{
		var bs = new Array(<?php echo $js_board_str;?>);
		var gsm_sum = <?php echo $__GSM_SUM__; ?>;
		var times = <?php if(isset($_GET['timeout'])) echo $_GET['timeout'];else echo 1;?>;
		var timeout = 40000 * times;
		var check_num = 0;
		var query_num = 0;
		$("#float_button_2").css({opacity:"0.5",filter:"alpha(opacity=50)"});
		if(!check_setting()) return;
		
		for(i in bs) {
			for(var c = 1; c <= gsm_sum; c++){
				if(document.getElementById(bs[i]+"_"+c).checked == true){
					if(document.getElementById("con_"+bs[i]+"_"+c).checked == true){
						if(document.getElementById("busy_box_"+bs[i]+"_"+c).checked == true){
							$("#status_busy_"+bs[i]+"_"+c).html("<?php echo language('SETTING','SETTING');?>"+"...");
						}
						if(document.getElementById("noreply_box_"+bs[i]+"_"+c).checked == true){
							$("#status_noreply_"+bs[i]+"_"+c).html("<?php echo language('SETTING','SETTING');?>"+"...");
						}
						if(document.getElementById("noreg_box_"+bs[i]+"_"+c).checked == true){
							$("#status_noreg_"+bs[i]+"_"+c).html("<?php echo language('SETTING','SETTING');?>"+"...");
						}
					} else if(document.getElementById("uncon_"+bs[i]+"_"+c).checked == true){
						$("#status_uncon_"+bs[i]+"_"+c).html("<?php echo language('SETTING','SETTING');?>"+"...");
						$("#status_noreg_"+bs[i]+"_"+c).html("");
						$("#status_noreply_"+bs[i]+"_"+c).html("");
						$("#status_busy_"+bs[i]+"_"+c).html("");
						$("#status_cancel_"+bs[i]+"_"+c).html("");
					} else if(document.getElementById("cancel_"+bs[i]+"_"+c).checked == true){
						$("#status_uncon_"+bs[i]+"_"+c).html("");
						$("#status_noreg_"+bs[i]+"_"+c).html("");
						$("#status_noreply_"+bs[i]+"_"+c).html("");
						$("#status_busy_"+bs[i]+"_"+c).html("");
						$("#status_cancel_"+bs[i]+"_"+c).html("<?php echo language('SETTING','SETTING');?>"+"...");
					}
					get_save_info(bs[i],c);
				}
			}
		}

		function check_setting()
		{
			for(i in bs) {
				for(var c = 1; c <= gsm_sum; c++){
					refresh_status_and_check_info(bs[i],c);
					if(document.getElementById(bs[i]+"_"+c).checked == true){
						check_num++;
					}
				}
			}
			
			if(check_num == 0){
				confirm("<?php echo language('Please choose port','Please choose the port you want to set!'); ?>");
				return false;
			}else{
				for(i in bs) {
					for(var c = 1; c <= gsm_sum; c++){
						if(document.getElementById(bs[i]+"_"+c).checked == true){
							if(!check(bs[i],c)) {
								return false;
							}
							if(!(document.getElementById("con_"+bs[i]+"_"+c).checked || document.getElementById("uncon_"+bs[i]+"_"+c).checked || document.getElementById("cancel_"+bs[i]+"_"+c).checked)){
								confirm("<?php echo language('Please set the content','Please set the content!'); ?>")
								return false;
							}
		
							if(document.getElementById("con_"+bs[i]+"_"+c).checked){
								if(!(document.getElementById("busy_box_"+bs[i]+"_"+c).checked || document.getElementById("noreply_box_"+bs[i]+"_"+c).checked ||document.getElementById("noreg_box_"+bs[i]+"_"+c).checked)){
									confirm("<?php echo language('Please set the content','Please set the content!'); ?>")
									return false;
								}
							}
						}
					}
				}
				if(!confirm("<?php echo language('Query Warning','Notice: This function need to request the data from GSM network,there will be some delay.');?>")) return false;
				$('#query_btn').attr("disabled","true");
				$('#setting_btn').attr("disabled","true");
				$("#query_btn").removeClass('gen_short_btn').addClass('gen_short_cli_btn');
				$("#setting_btn").removeClass('gen_short_btn').addClass('gen_short_cli_btn');
				$('.gen_btn').addClass("click_gen_btn");
				$('.float_short_button').attr("disabled","true");
				$('#progress').show();
				return true;
			}
		}
		
		function replace_number(str)
		{
			return str.replace("+", "%2B");
		}
		
		function get_save_info(b,c){
			var url_info = '';
			var id_info = '';
			if(document.getElementById("con_"+b+"_"+c).checked){
				url_info += "&type=cond";
				id_info = "cond";
				if(document.getElementById("busy_box_"+b+"_"+c).checked){
					url_info += "&busy="+replace_number(document.getElementById("busy_num_"+b+"_"+c).value);
				}
				if(document.getElementById("noreply_box_"+b+"_"+c).checked){
					url_info += "&noreply="+replace_number(document.getElementById("noreply_num_"+b+"_"+c).value);
				}
				if(document.getElementById("noreg_box_"+b+"_"+c).checked){
					url_info += "&notreach="+replace_number(document.getElementById("noreg_num_"+b+"_"+c).value);
				}
			}
			
			if(document.getElementById("uncon_"+b+"_"+c).checked){
				id_info = "uncond";
				url_info += "&type=uncond";
				url_info += "&number="+replace_number(document.getElementById("uncon_num_"+b+"_"+c).value);
			}
			if(document.getElementById("cancel_"+b+"_"+c).checked){
				id_info = "disable";
				url_info += "&type=disable";
			}
			if(id_info == ''){
				$('#query_btn').removeAttr("disabled");
				$('#setting_btn').removeAttr("disabled");
				$("#query_btn").removeClass('gen_short_cli_btn').addClass('gen_short_btn');
				$("#setting_btn").removeClass('gen_short_cli_btn').addClass('gen_short_btn');
				$('.gen_btn').removeClass("click_gen_btn");
				$('.float_short_button').removeAttr("disabled");
				$('#progress').hide();
				confirm("<?php echo language('Please set the content','Please set the content!'); ?>");
				return false;
			}
			save_info(b,c,url_info,id_info)
		}
			
		function save_info(board,port,url_info,id_info)
		{
			if(board == 1){
				url = "/service?action=set_cf&span=" + port + url_info + "&timeout=" + timeout+"random="+Math.random();
			}else{
				url = "/" + board + "/service?action=set_cf&span=" + port + url_info + "&timeout=" + timeout+"random="+Math.random();
			}
			$.ajax({
				url: url,
				type: 'GET',
				dataType: 'text',
				data: {},
				timeout:timeout*5,
				success: function(atreport){
					if(!atreport){
						$("#status_uncon_"+board+"_"+port).html("<?php echo language('TIMEOUT','TIMEOUT');?>");
						$("#status_busy_"+board+"_"+port).html("<?php echo language('TIMEOUT','TIMEOUT');?>");
						$("#status_noreply_"+board+"_"+port).html("<?php echo language('TIMEOUT','TIMEOUT');?>");
						$("#status_noreg_"+board+"_"+port).html("<?php echo language('TIMEOUT','TIMEOUT');?>");
					}else{
						display_set_info(atreport,id_info,board,port);
					}
				},
				complete: function(){
					query_num++;
					if(query_num == check_num){
						$('#query_btn').removeAttr("disabled");
						$('#setting_btn').removeAttr("disabled");
						$("#setting_btn").removeClass('gen_short_cli_btn').addClass('gen_short_btn');
						$("#query_btn").removeClass('gen_short_cli_btn').addClass('gen_short_btn');
						$('.gen_btn').removeClass("click_gen_btn");
						$('.float_short_button').removeAttr("disabled");
						$('#progress').hide();
					}
				},
			});
		}
		
		function display_set_info(atreport,id_info,board,port)
		{
			if(id_info=="disable"){
				$("#status_cancel_"+board+"_"+port).html(atreport);
			}else if(id_info=="cond"){
		         var lines = atreport.split(/\r\n|\r|\n/);
		         if(lines.length >=4) {
		         	if(lines[2] != "SKIP") $("#status_noreply_"+board+"_"+port).html(lines[2]);
					if(lines[1] != "SKIP") $("#status_busy_"+board+"_"+port).html(lines[1]);
					if(lines[3] != "SKIP") $("#status_noreg_"+board+"_"+port).html(lines[3]);
		         }
			}else{
				$("#status_uncon_"+board+"_"+port).html(atreport);
			}
		}
		
	}

	</script>
<?php require("/www/cgi-bin/inc/boot.inc"); ?>