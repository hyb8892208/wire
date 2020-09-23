<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
?>
<?php
$board = get_slotnum();
$js_board_str = '"'.$board.'"';

//phone number switch
$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$phonenum_res = $aql->query("select * from sim_query.conf");
?>
	<script type="text/javascript" src="/js/functions.js"></script>
	<script type="text/javascript" src="/js/float_btn.js"></script>
	<table width="100%" class="tshow" >
		<tr>
			<th style="width: 6%;"><input type="checkbox" name="selall" onclick="selectAll(this.checked,'selinput')" /></th>
			<th style="width: 10%;"><?php echo language('Port');?></th>
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			<th><?php echo language('Mobile Number');?></th>
			<?php } ?>
			
			<th style="width: 30%;">
				<input type="radio" id="radio_sel_on" name="radio_sel"/><?php echo language('ON');?>
				<input type="radio" id="radio_sel_off" name="radio_sel"/><?php echo language('OFF');?>
				(<?php echo language('Call Waiting Function');?>)
			</th>
			<th style="width: 34%;padding-left:0;"><?php echo language('Status');?></th>
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
			<td><input type="checkbox" name="selinput" class="<?php if($display == '') echo 'show_input';?>" value="<?php echo $i; ?>" id="<?php echo $board."_".$i; ?>" /></td>
			<td><?php echo $channel_name; ?></td>				
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			<td><?php echo $phonenum;?></td>
			<?php } ?>
			
			<td style="padding-left:0;">
				<input name="<?php echo "callwaiting_".$board."_".$i; ?>" class="child_radio_on" id="<?php echo "on_".$board."_".$i; ?>" type="radio" value="on"><?php echo language('ON');?>
				<input name="<?php echo "callwaiting_".$board."_".$i; ?>" class="child_radio_off" id="<?php echo "off_".$board."_".$i; ?>" type="radio" value="off"><?php echo language('OFF');?>
			</td>
			<td id="<?php echo "status_".$board."_".$i; ?>"></td>
		</tr>
<?php
}

?>
	</table>
	<br/>
	<table id="float_btn" class="float_btn">
		<tr id="float_btn_tr" class="float_btn_tr">
			<td>
				<input type=button id="query_btn"  value="<?php echo language('Query');?>" onclick="query();" />
			</td>
			<td style="padding-left: 10px;">
				<input type=button id="setting_btn"  value="<?php echo language('Setting');?>" onclick="setting()"/>
			</td>
			<td style="padding-left: 20px;display:none;" id="progress" class="progress">
				<img src="/images/mini_loading.gif" align="middle" />
			</td>
		</tr>
	</table>

	<script type="text/javascript">
	$("#radio_sel_on").click(function(){
		$(".child_radio_on").attr("checked","checked");
	});
	$("#radio_sel_off").click(function(){
		$(".child_radio_off").attr("checked","checked");
	});
	
	function refresh_status_and_check_info(board,port)
	{
		$("#status_"+board+"_"+port).html('');
	}
	
	function query()
	{
		var bs = new Array(<?php echo $js_board_str;?>);
		var gsm_sum = <?php echo $__GSM_SUM__; ?>;
		var times = <?php if(isset($_GET['timeout'])) echo $_GET['timeout'];else echo 1;?>;
		var timeout = 40000 * times;
		var check_num = 0;
		var query_num = 0;
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
				$('.progress').show();
				return true;
			}
		}


		for(i in bs) {
			for(var c = 1; c <= gsm_sum; c++){
				if(document.getElementById(bs[i]+"_"+c).checked == true){
					refresh_all_info(bs[i],c);
					$("#status_"+bs[i]+"_"+c).html("<?php echo language('QUERYING','QUERYING');?>" + "...");
					query_all_info(bs[i],c);
				}
			}
		}

		function query_all_info(board, port) {
			if(board == 1){
				url = "/service?action=get_cwinfo&span=" + port + "&timeout=" + timeout+"random="+Math.random();
			}else{
				url = "/" + board + "/service?action=get_cwinfo&span=" + port + "&timeout=" + timeout+"random="+Math.random();
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
						$("#status_"+board+"_"+port).html("<?php echo language('TIMEOUT','TIMEOUT');?>");
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
						$('.progress').hide();
					}
				},
			});
		};

		function display_cf_info(atreport,board,port)
		{
			if(atreport.callwaiting == "ON"){
				$("#status_"+board+"_"+port).html("<?php echo language('SUCCESS','SUCCESS');?>");
				$("#on_"+board+"_"+port).attr({"checked":true});
			} else if(atreport.callwaiting == "OFF"){
				$("#status_"+board+"_"+port).html("<?php echo language('SUCCESS','SUCCESS');?>");
				$("#off_"+board+"_"+port).attr({"checked":true});
			} else if(atreport.callwaiting == "UNKNOWN"){
				$("#status_"+board+"_"+port).html("<?php echo language('UNKNOWN','UNKNOWN');?>");
				$("#on_"+board+"_"+port).attr({"checked":false});
				$("#off_"+board+"_"+port).attr({"checked":false});
			}else if(atreport.callwaiting == "TIMEOUT"){
				$("#status_"+board+"_"+port).html("<?php echo language('TIMEOUT','TIMEOUT');?>");
				$("#on_"+board+"_"+port).attr({"checked":false});
				$("#off_"+board+"_"+port).attr({"checked":false});
			}else if(atreport.callwaiting == "NOT READY"){
				$("#status_"+board+"_"+port).html("<?php echo language('NOT READY','NOT READY');?>");
				$("#on_"+board+"_"+port).attr({"checked":false});
				$("#off_"+board+"_"+port).attr({"checked":false});
			}else{
				$("#status_"+board+"_"+port).html("<?php echo language('FAILED','FAILED');?>");
				$("#on_"+board+"_"+port).attr({"checked":false});
				$("#off_"+board+"_"+port).attr({"checked":false});
			}
		}
		
		function refresh_all_info(board,port) {
			$("#status_"+board+"_"+port).html('');
			$("#on_"+board+"_"+port).attr({"checked":false});
			$("#off_"+board+"_"+port).attr({"checked":false});
		};
	}
	
	function setting()
	{
		var bs = new Array(<?php echo $js_board_str;?>);
		var gsm_sum = <?php echo $__GSM_SUM__; ?>;
		var show_gsm_sum = [];
		$(".show_input").each(function(){
			show_gsm_sum.push($(this).val());
		});
		var times = <?php if(isset($_GET['timeout'])) echo $_GET['timeout'];else echo 1;?>;
		var timeout = 40000 * times;
		var check_num = 0;
		var query_num = 0;
		$("#float_button_2").css({opacity:"0.5",filter:"alpha(opacity=50)"});
		if(!check_setting()) return;
		
		for(i in bs) {
			for(var c = 1; c <= gsm_sum; c++){
				if(document.getElementById(bs[i]+"_"+c).checked == true){
					if(document.getElementById("on_"+bs[i]+"_"+c).checked || document.getElementById("off_"+bs[i]+"_"+c).checked){
						$("#status_"+bs[i]+"_"+c).html("<?php echo language('SETTING','SETTING');?>"+"...");
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
					for(var c in show_gsm_sum){
						if(document.getElementById(bs[i]+"_"+show_gsm_sum[c]).checked == true){
							if(!(document.getElementById("on_"+bs[i]+"_"+show_gsm_sum[c]).checked || document.getElementById("off_"+bs[i]+"_"+show_gsm_sum[c]).checked)){
								confirm("<?php echo language('Please set the content','Please set the content!'); ?>")
								return false;
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
				$('.progress').show();
				return true;
			}
		}
		
		function replace_number(str)
		{
			return str.replace("+", "%2B");
		}
		
		function get_save_info(b,c){
			var url_info = '';

			if(document.getElementById("on_"+b+"_"+c).checked){
				url_info = "&sw=on";
			}
			if(document.getElementById("off_"+b+"_"+c).checked){
				url_info = "&sw=off";
			}
			save_info(b,c,url_info)
		}
			
		function save_info(board,port,url_info)
		{
			if(board == 1){
				url = "/service?action=set_cw&span=" + port + url_info + "&timeout=" + timeout+"&random="+Math.random();
			}else{
				url = "/" + board + "/service?action=set_cw&span=" + port + url_info + "&timeout=" + timeout+"&random="+Math.random();
			}
			$.ajax({
				url: url,
				type: 'GET',
				dataType: 'text',
				data: {},
				timeout:timeout*5,
				success: function(atreport){
					if(!atreport){
						$("#status_"+board+"_"+port).html("<?php echo language('TIMEOUT','TIMEOUT');?>");
					}else{
						display_set_info(atreport,board,port);
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
						$('.progress').hide();
					}
				},
			});
		}
		
		function display_set_info(atreport,board,port)
		{
			$("#status_"+board+"_"+port).html(atreport);
		}
	}
	</script>
<?php
require("/www/cgi-bin/inc/boot.inc");
?>
<div id="float_btn1" class="sec_float_btn1">
</div>
<table id="float_btn2" style="border:none;" class="float_btn2">
	<tr id="float_btn_tr2" class="float_btn_tr2">
		<td width="20px">
			<input type="button" id="float_button_1" class="float_short_button" value="<?php echo language('Query');?>" onclick="query();" />
		</td>
		<td style="padding-left: 10px;">
			<input type="button" id="float_button_2" class="float_short_button" value="<?php echo language('Setting');?>" onclick="setting()"/>
		</td>
	</tr>
</table>
<div  id="float_progress" style="display:none">
	<img src="/images/mini_loading.gif" align="middle" class="progress" style="display:none"/>
</div>
<div  class="float_close" onclick="close_btn()">
</div>