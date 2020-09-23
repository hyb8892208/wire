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
<script type="text/javascript" src="/js/float_btn.js"></script>

<?php
function show_strategy_smsstats(){
	exec("/my_tools/calllimit_cli show allstatus",$cmd_res);

	$allstatus = json_decode($cmd_res[0],true);
?>

	<form enctype="multipart/form-data" action="<?php echo get_self();?>" method="get" >
		<input type="hidden" id="sel_gsm" name="sel_gsm" value="" />

		<div id="tab">
			<li class="tb_unfold"></li>
			<li class="tbg" id="show_sms_limit" style="cursor:pointer;"><?php echo language('SMS Sending Statistics');?></li>
			<li class="tb2">&nbsp;</li>
		</div>
		
		<table width="100%" class="tshow" id="sms_limit_table" >
			<tr>
				<th style="width:0.3%;"><input type="checkbox" name="selall_port" onclick="selectAll(this.checked,'port[]')" /></th>
				<th width="150px"><?php echo language('Port');?>-SIM</th>
				<th><?php echo language('SMS count of the day');?></th>
				<th><?php echo language('Daily limit');?></th>
				<th><?php echo language('SMS count of the month');?></th>
				<th><?php echo language('Monthly limit');?></th>
				<th><?php echo language('Monthly recovery date');?></th>
			</tr>
			
			<?php 
			$aql = new aql();
			$aql->set('basedir','/etc/asterisk/gw/call_limit/');
			$res = $aql->query("select * from calllimit_settings.conf");
			
			foreach($allstatus as $key => $val){
				$temp = explode("-", $key);
				$channel = $temp[0];
				$channel_type = get_gsm_type_by_channel($channel,1);
			?>
			<tr <?php if($channel_type == 'NULL') echo 'style="display:none;"'; ?> >
				<td><input type="checkbox" class="sms_limit_checked" name="port[]" value="<?php echo $key;?>" /></td>
				<td><?php echo $key;?></td>
				<td><?php echo $val['day_cur_sms'];?></td>
				<td><?php echo $res[$channel]['day_sms_settings']?></td>
				<td><?php echo $val['mon_cur_sms'];?></td>
				<td><?php echo $res[$channel]['mon_sms_settings']?></td>
				<td><?php echo $res[$channel]['sms_clean_date']?></td>
			</tr>
			<?php }?>
			
			<tr>
				<td></td>
				<td></td>
				<td><input type="submit" value="<?php echo language('Clear zero');?>" onclick="document.getElementById('send').value='Clean Day';return check_sel_port();"/></td>
				<td></td>
				<td><input type="submit" value="<?php echo language('Clear zero');?>" onclick="document.getElementById('send').value='Clean Month';return check_sel_port();"/></td>
				<td></td>
				<td></td>
			</tr>
		</table>
		
		<table id="float_btn" class="float_btn">
		</table>
		
		<table id="float_btn2" style="border:none;" class="float_btn2" >
			<tr id="float_btn_tr2" class="float_btn_tr2">
				<td width="50px">
					<input type="submit" style="margin-left:200px;" value="<?php echo language('Clear zero');?>" onclick="document.getElementById('send').value='Clean Day';return check_sel_port();"/>
				</td>
				<td width="50px">
					<input type="submit" style="margin-left:330px;" value="<?php echo language('Clear zero');?>" onclick="document.getElementById('send').value='Clean Month';return check_sel_port();"/>
				</td>
			</tr>
		</table>
		
		<br/>
		
		<input type="hidden" name="send" id="send" value="" />
	</form>

	<script>
	$("#show_sms_limit").click(function(){
		if($("#sms_limit_table").css('display') == 'none'){
			$(this).siblings('.tb_fold').addClass('tb_unfold');
			$(this).siblings('.tb_fold').removeClass('tb_fold');
			$("#sms_limit_table").show();
		}else{
			$(this).siblings('.tb_unfold').addClass('tb_fold');
			$(this).siblings('.tb_unfold').removeClass('tb_unfold');
			$("#sms_limit_table").hide();
		}
	});
	
	function check_sel_port(){
		var flag = 0;
		$(".sms_limit_checked").each(function(){
			if($(this).attr('checked') == 'checked'){
				flag = 1;
			}
		});
		
		if(flag == 0){
			alert("<?php echo language('Select port alert');?>");
			return false;
		}
	}
	</script>
	
<?php 
}

function clean_sms_day(){
	$port_arr = $_GET['port'];
	for($i=0;$i<count($port_arr);$i++){
		$temp = explode('-',$port_arr[$i]);
		$channel = $temp[0];
		$sim_id = $temp[1];
		exec("/my_tools/set_calllimit.sh sim_setdaysmscnt $channel $sim_id 0");
	}
}

function clean_sms_month(){
	$port_arr = $_GET['port'];
	for($i=0;$i<count($port_arr);$i++){
		$temp = explode('-',$port_arr[$i]);
		$channel = $temp[0];
		$sim_id = $temp[1];
		exec("/my_tools/set_calllimit.sh sim_setmonsmscnt $channel $sim_id 0");
	}
}

if($_GET){
	if($_GET['send'] == 'Clean Day'){
		clean_sms_day();
		show_strategy_smsstats();
	}else if($_GET['send'] == 'Clean Month'){
		clean_sms_month();
		show_strategy_smsstats();
	}
}else{
	show_strategy_smsstats();
}
?>

<?php
require("/www/cgi-bin/inc/boot.inc");
?>
<div id="float_btn1" class="sec_float_btn1"></div>
<div class="float_close" onclick="close_btn()" ></div>