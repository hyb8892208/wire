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
function show_strategy_callstats(){
	exec("/my_tools/calllimit_cli show allstatus",$cmd_res);

	$allstatus = json_decode($cmd_res[0],true);
?>
	<form enctype="multipart/form-data" action="<?php echo get_self();?>" method="get" >
		<input type="hidden" id="sel_gsm" name="sel_gsm" value="" />

		<div id="tab">
			<li class="tb_unfold"></li>
			<li class="tbg" id="show_call_statistics" style="cursor:pointer;"><?php echo language('Call Statistics@calllimit','Call Statistics');?></li>
			<li class="tb2">&nbsp;</li>
		</div>
		
		<table width="100%" class="tshow" id="call_statistics_table" >
			<tr>
				<th width="150px" rowspan=2><?php echo language('Port');?>-SIM</th>
				<th colspan=5 style="text-align:center;"><?php echo language('Call Limit');?></th>
				<th colspan=4 style="text-align:center;"><?php echo language('strategy');?></th>
			</tr>
			
			<tr>
				<th><?php echo language('Hour Call Count');?></th>
				<th><?php echo language('Daily Call Count');?></th>
				<th><?php echo language('Daily Answer Count');?></th>
				<th><?php echo language('Call Failed Count');?></th>
				<th><?php echo language('Call Duration');?></th>
				<th><?php echo language('Callout Count');?></th>
				<th><?php echo language('Callout Time');?></th>
				<th><?php echo language('SMS Count');?></th>
				<th><?php echo language('Using Time');?></th>
			</tr>
			
			<?php 
			foreach($allstatus as $key => $val){
				$temp = explode("-", $key);
				$channel = $temp[0];
				$channel_type = get_gsm_type_by_channel($channel,1);
			?>
			<tr <?php if($channel_type == 'NULL') echo 'style="display:none;"'; ?> >
				<td><?php echo $key;?></td>
				<td><?php echo $val['hour_total_calls'];?></td>
				<td><?php echo $val['day_total_calls'];?></td>
				<td><?php echo $val['day_total_answers'];?></td>
				<td><?php echo $val['call_failed_count'];?></td>
				<td><?php echo $val['call_time_count'];?></td>
				<td><?php echo $val['curr_callout_count'];?></td>
				<td><?php echo $val['curr_callout_time'];?></td>
				<td><?php echo $val['curr_sms_count'];?></td>
				<td><?php echo $val['curr_using_time'];?></td>
			</tr>
			<?php }?>
		</table>

		<br/>
		<input type="hidden" name="send" id="send" value="" />
	</form>
	
	<script>
	$("#show_call_statistics").click(function(){
		if($("#call_statistics_table").css('display') == 'none'){
			$(this).siblings('.tb_fold').addClass('tb_unfold');
			$(this).siblings('.tb_fold').removeClass('tb_fold');
			$("#call_statistics_table").show();
		}else{
			$(this).siblings('.tb_unfold').addClass('tb_fold');
			$(this).siblings('.tb_unfold').removeClass('tb_unfold');
			$("#call_statistics_table").hide();
		}
	});
	</script>
	
<?php
}

show_strategy_callstats();

require("/www/cgi-bin/inc/boot.inc");
?>
<div id="float_btn1" class="sec_float_btn1"></div>
<div class="float_close" onclick="close_btn()" ></div>