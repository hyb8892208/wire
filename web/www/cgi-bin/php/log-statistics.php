<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/smsoutboxdb.php");
?>

<?php
//phone number switch
$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$phonenum_res = $aql->query("select * from sim_query.conf");

function show_statistics()
{
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	global $__GSM_SUM__;
	global $phonenum_res;
	
	if($phonenum_res['general']['phonenum_switch'] == 'on'){
		$outcolspan = 12;
		$incolspan = 9;
		$smscolspan = 6;
	}else{
		$outcolspan = 11;
		$incolspan = 8;
		$smscolspan = 5;
	}
?>

<script type="text/javascript" src="/js/float_btn.js"></script>
<form id="manform" enctype="multipart/form-data" action="<?php echo get_self();?>" method="post">
	<div id="tab" class="gsm_outbound" style="float:left;clear:none;width:auto;margin-right:10px;cursor:pointer;">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('GSM Outbound');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	
	<div id="tab" class="gsm_inbound" style="float:left;clear:none;width:auto;margin-right:10px;cursor:pointer;">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('GSM Inbound');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	
	<div id="tab" class="sms" style="float:left;clear:none;width:auto;cursor:pointer;">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('SMS');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	
	<!-- GSM Outbound -->
	<table width="100%" class="tshow gsm_outbound_table">
		<tr><th colspan="<?php echo $outcolspan;?>"><?php echo language('GSM Outbound');?></th></tr>
		<tr>
			<th width="100px"><?php echo language('Port');?></th>
			
			<th width="100px"><?php echo language('Phone Number');?></th>
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			<th width="100px;"><?php echo language('Mobile Number');?></th>
			<?php } ?>
			
			<th width="50px"><?php echo language('All Calls');?></th>
			<th width="50px"><?php echo language('All Durations');?></th>
			<th width="50px"><?php echo language('Answered');?></th>
			<th width="50px"><?php echo language('Canceled');?></th>
			<th width="50px"><?php echo language('Busy');?></th>
			<th width="50px"><?php echo language('No Answer');?></th>
			<th width="50px"><?php echo language('No Dialtone');?></th>
			<th width="50px"><?php echo language('No Carrier');?></th>
			<th width="50px"><?php echo language('Other');?></th>
		</tr>
<?php
	$a = 0;
	$res=`asterisk -rx "gsm show statistics outbound" 2> /dev/null`;
	if(!empty($res)) {

/* Format
Span | Count | All Duration | Answerd | Cancel | Busy | No Answer | No Dialtone | No Carrier
1    | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0
2    | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0
3    | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0
4    | 0     | 0            | 0       | 0      | 0    | 0         | 0           | 0
*/ 
		$total_count = 0;
		$total_duration = 0;
		$total_answer = 0;
		$total_cancel = 0;
		$total_busy = 0;
		$total_noanswer = 0;
		$total_nodialtone = 0;
		$total_nocarrier = 0;
		$total_other = 0;

		$line = explode("\n",$res);
		if(!empty($line)) {
			//Don't need first line.
			$line_len = count($line) - 1;
			for($i=1; $i < $line_len; $i++) {
				$item = explode("|",$line[$i],9);
				if(isset($item[8])) {
					if(strstr(get_gsm_name_by_channel(trim($item[0])), 'null')) continue;
					$show[$a]['port'] = get_gsm_name_by_channel(trim($item[0]),1,false);
					
					$chn = trim($item[0]);
					$show[$a]['aliasname'] = get_gsm_aliasname($chn,1);
					
					exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $chn",$phone_output);
					$show[$a]['phonenum'] = $phone_output[0];
					$phone_output = '';
					
					$show[$a]['count'] = trim($item[1]);
					$show[$a]['duration'] = trim($item[2]);
					$show[$a]['answer'] = trim($item[3]);
					$show[$a]['cancel'] = trim($item[4]);
					$show[$a]['busy'] = trim($item[5]);
					$show[$a]['noanswer'] = trim($item[6]);
					$show[$a]['nodialtone'] = trim($item[7]);
					$show[$a]['nocarrier'] = trim($item[8]);
					$other = $show[$a]['count'] - $show[$a]['answer'] - $show[$a]['cancel'] - $show[$a]['busy'] - $show[$a]['noanswer'] - $show[$a]['nodialtone'] - $show[$a]['nocarrier'];
					$show[$a]['other'] = $other > 0 ? $other : 0;
					
					$total_count += $show[$a]['count'];
					$total_duration += $show[$a]['duration'];
					$total_answer += $show[$a]['answer'];
					$total_cancel += $show[$a]['cancel'];
					$total_busy += $show[$a]['busy'];
					$total_noanswer += $show[$a]['noanswer'];
					$total_nodialtone += $show[$a]['nodialtone'];
					$total_nocarrier += $show[$a]['nocarrier'];
					$total_other += $show[$a]['other'];
					
					$a++;
				}
			}
		}
	}
	
	if(isset($show) && is_array($show)) {
		foreach($show as $each) {
?>

		<tr class="gsm_outbound_tr">
			<td><?php echo $each['port'];?></td>
			
			<td><?php echo $each['aliasname'];?></td>
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			<td><?php echo $each['phonenum'];?></td>
			<?php } ?>
			
			<td><?php echo $each['count'];?></td>
			<td><?php echo $each['duration'];?></td>
			<td><?php echo $each['answer'];?></td>
			<td><?php echo $each['cancel'];?></td>
			<td><?php echo $each['busy'];?></td>
			<td><?php echo $each['noanswer'];?></td>
			<td><?php echo $each['nodialtone'];?></td>
			<td><?php echo $each['nocarrier'];?></td>
			<td><?php echo $each['other'];?></td>
		</tr>
<?php
		}
	}
?>
		<tr class="total_tr">
			<td><?php echo language('Total');?></td>
			
			<td></td>
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			<td></td>
			<?php } ?>
			
			<td><?php echo $total_count;?></td>
			<td><?php echo $total_duration;?></td>
			<td><?php echo $total_answer;?></td>
			<td><?php echo $total_cancel;?></td>
			<td><?php echo $total_busy;?></td>
			<td><?php echo $total_noanswer;?></td>
			<td><?php echo $total_nodialtone;?></td>
			<td><?php echo $total_nocarrier;?></td>
			<td><?php echo $total_other;?></td>
		</tr>
	</table>
	
	<!-- GSM Inbound -->
	<table width="100%" class="tshow gsm_inbound_table" style="display:none;">
		<tr><th colspan="<?php echo $incolspan;?>"><?php echo language('GSM Inbound');?></th></tr>
		<tr>
			<th width="100px"><?php echo language('Port');?></th>
			
			<th width="150px"><?php echo language('Phone Number');?></th>
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			<th width="150px"><?php echo language('Mobile Number');?></th>
			<?php } ?>
			
			<th><?php echo language('All Calls');?></th>
			<th><?php echo language('All Durations');?></th>
			<th><?php echo language('Answered');?></th>
			<th><?php echo language('Canceled');?></th>
			<th><?php echo language('Busy');?></th>
			<th><?php echo language('Other');?></th>
		</tr>
<?php
	$a = 0;
	$res = `asterisk -rx "gsm show statistics inbound" 2> /dev/null`;
	if(!empty($res)){
		$total_count = 0;
		$total_duration = 0;
		$total_answer = 0;
		$total_cancel = 0;
		$total_busy = 0;
		$total_other = 0;
		
		$line = explode("\n",$res);
		if(!empty($line)){
			$line_len = count($line) - 1;
			for($i=1;$i<$line_len;$i++){
				$item = explode("|", $line[$i], 9);
				if(isset($item[5])){
					if(strstr(get_gsm_name_by_channel(trim($item[0])),'null')) continue;
					$show[$a]['port'] = get_gsm_name_by_channel(trim($item[0]),1,false);
					
					$chn = trim($item[0]);
					$show[$a]['aliasname'] = get_gsm_aliasname($chn,1);
					
					exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $chn",$phone_output);
					$show[$a]['phonenum'] = $phone_output[0];
					$phone_output = '';
					
					$show[$a]['count'] = trim($item[1]);
					$show[$a]['duration'] = trim($item[2]);
					$show[$a]['answer'] = trim($item[3]);
					$show[$a]['cancel'] = trim($item[4]);
					$show[$a]['busy'] = trim($item[5]);
					$other = $show[$a]['count'] - $show[$a]['answer'] - $show[$a]['cancel'] - $show[$a]['busy'];
					$show[$a]['other'] = $other > 0 ? $other : 0;
					
					$total_count += $show[$a]['count'];
					$total_duration += $show[$a]['duration'];
					$total_answer += $show[$a]['answer'];
					$total_cancel += $show[$a]['cancel'];
					$total_busy += $show[$a]['busy'];
					$total_other += $show[$a]['other'];
					
					$a++;
				}
			}
		}
	}
	
	if(isset($show) && is_array($show)){
		foreach($show as $each){
?>
		<tr class="gsm_inbound_tr">
			<td><?php echo $each['port'];?></td>
			
			<td><?php echo $each['aliasname'];?></td>
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			<td><?php echo $each['phonenum'];?></td>
			<?php } ?>
			
			<td><?php echo $each['count'];?></td>
			<td><?php echo $each['duration'];?></td>
			<td><?php echo $each['answer'];?></td>
			<td><?php echo $each['cancel'];?></td>
			<td><?php echo $each['busy'];?></td>
			<td><?php echo $each['other']?></td>
		</tr>
<?php 
		}
	}
?>
		<tr class="in_total_tr">
			<td><?php echo language('Total');?></td>
			
			<td></td>
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			<td></td>
			<?php } ?>
			
			<td><?php echo $total_count;?></td>
			<td><?php echo $total_duration;?></td>
			<td><?php echo $total_answer;?></td>
			<td><?php echo $total_cancel;?></td>
			<td><?php echo $total_busy;?></td>
			<td><?php echo $total_other;?></td>
		</tr>
	</table>
	
	<!-- SMS -->
	<?php 
	$db = new SMSOUTBOXDB();
	$sql = "select * from sms_out";
	$results = $db->try_query($sql);
	?>
	
	<table width="100%" class="tshow sms_table" style="display:none;">
		<tr><th colspan="<?php echo $smscolspan;?>"><?php echo language('SMS');?></th></tr>
		
		<tr>
			<th><?php echo language('Port');?></th>
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			<th width="150px"><?php echo language('Mobile Number');?></th>
			<?php } ?>
			
			<th><?php echo language('All SMS');?></th>
			<th><?php echo language('Deliverd');?></th>
			<th><?php echo language('Sent');?></th>
			<th><?php echo language('Failed');?></th>
		</tr>
		
		<?php
		$total_all_num = 0;
		$total_deliverd_num = 0;
		$total_sent_num = 0;
		$total_failed_num = 0;
		
		for($c=1; $c<=$__GSM_SUM__; $c++){
			$deliverd_num = 0;
			$failed_num = 0;
			$sent_num = 0;
			
			exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $c",$phone_output);
			$phonenum = $phone_output[0];
			$phone_output = '';
			
			while($res = @$results->fetchArray()){
				if($res['port'] == $c){
					if($res['status'] == 0){
						$failed_num++;
					}else if($res['status'] == 1){
						$sent_num++;
					}else if($res['status'] == 2){
						$deliverd_num++;
					}
				}
			}
			
			$all_num = $failed_num+$sent_num+$deliverd_num;
			
			$total_all_num += $all_num;
			$total_deliverd_num += $deliverd_num;
			$total_sent_num += $sent_num;
			$total_failed_num += $failed_num;
		?>
		<tr>
			<td><?php echo get_gsm_name_by_channel($c);?></td>
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			<td><?php echo $phonenum;?></td>
			<?php } ?>
			
			<td><?php echo $all_num;?></td>
			<td><?php echo $deliverd_num;?></td>
			<td><?php echo $sent_num;?></td>
			<td><?php echo $failed_num;?></td>
		</tr>
		<?php } ?>
		
		<tr>
			<td><?php echo language('Total');?></td>
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			<td></td>
			<?php } ?>
			
			<td><?php echo $total_all_num;?></td>
			<td><?php echo $total_deliverd_num;?></td>
			<td><?php echo $total_sent_num;?></td>
			<td><?php echo $total_failed_num;?></td>
		</tr>
	</table>
	
	<br/>
	<input type="hidden" name="send" id="send" value="GSM Outbound Export" />
	<table id="float_btn" class="float_btn">
		<tr id="float_btn_tr" class="float_btn_tr" style="padding-left: 15px;">
			<td>
				<input type="submit"  value="<?php echo language('Export');?>" />
			</td>
		</tr>
	</table>
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2" style="padding-left: 15px;">
			<td>
				<input type="submit" id="float_button_2" class="float_short_button" value="<?php echo language('Export');?>" />
			</td>
		</tr>
	</table>
</form>

<script type="text/javascript">
$(".gsm_outbound").click(function(){
	$("#send").val("GSM Outbound Export");
	$(".tshow").hide();
	$(".gsm_outbound_table").show();
});

$(".gsm_inbound").click(function(){
	$("#send").val("GSM Inbound Export");
	$(".tshow").hide();
	$(".gsm_inbound_table").show();
});

$(".sms").click(function(){
	$("#send").val("SMS Export");
	$(".tshow").hide();
	$(".sms_table").show();
});
</script>
<?php
}

function gsm_outbound_export(){
	global $phonenum_res;
	$file_name = 'gsm_outbound_stats.xls';
	
	$a = 0;
	$res=`asterisk -rx "gsm show statistics outbound" 2> /dev/null`;
	if(!empty($res)) {
		
		$total_count = 0;
		$total_duration = 0;
		$total_answer = 0;
		$total_cancel = 0;
		$total_busy = 0;
		$total_noanswer = 0;
		$total_nodialtone = 0;
		$total_nocarrier = 0;
		$total_other = 0;
		
		$line = explode("\n",$res);
		if(!empty($line)) {
			$line_len = count($line) - 1;
			for($i=1; $i < $line_len; $i++) {
				$item = explode("|",$line[$i],9);
				if(isset($item[8])) {
					if(strstr(get_gsm_name_by_channel(trim($item[0])), 'null')) continue;
					$show[$a]['port'] = get_gsm_name_by_channel(trim($item[0]));
					
					$chn = trim($item[0]);
					$show[$a]['aliasname'] = get_gsm_aliasname($chn,1);
					
					exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $chn",$phone_output);
					$show[$a]['phonenum'] = $phone_output[0];
					$phone_output = '';
					
					$show[$a]['count'] = trim($item[1]);
					$show[$a]['duration'] = trim($item[2]);
					$show[$a]['answer'] = trim($item[3]);
					$show[$a]['cancel'] = trim($item[4]);
					$show[$a]['busy'] = trim($item[5]);
					$show[$a]['noanswer'] = trim($item[6]);
					$show[$a]['nodialtone'] = trim($item[7]);
					$show[$a]['nocarrier'] = trim($item[8]);
					$other = $show[$a]['count'] - $show[$a]['answer'] - $show[$a]['cancel'] - $show[$a]['busy'] - $show[$a]['noanswer'] - $show[$a]['nodialtone'] - $show[$a]['nocarrier'];
					$show[$a]['other'] = $other > 0 ? $other : 0;
					
					$total_count += $show[$a]['count'];
					$total_duration += $show[$a]['duration'];
					$total_answer += $show[$a]['answer'];
					$total_cancel += $show[$a]['cancel'];
					$total_busy += $show[$a]['busy'];
					$total_noanswer += $show[$a]['noanswer'];
					$total_nodialtone += $show[$a]['nodialtone'];
					$total_nocarrier += $show[$a]['nocarrier'];
					$total_other += $show[$a]['other'];
					
					$a++;
				}
			}
		}
	}
	
	ob_clean();
	flush();
	header("Content-type:application/vnd.ms-excel");
	header("Content-Disposition:attachment;filename=$file_name");
	
	if($phonenum_res['general']['phonenum_switch'] == 'on'){
		echo "Port\tPhone Number\tMobile Number\tAll Calls\tAll Durations\tAnswered\tCanceled\tBusy\tNo Answer\tNo Dialtone\tNo Carrier\tOther\n";
		foreach($show as $each) {
			echo $each['port']."\t".$each['aliasname']."\t".$each['phonenum']."\t".$each['count']."\t".$each['duration']."\t".$each['answer']."\t".$each['cancel']."\t".$each['busy']."\t".$each['noanswer']."\t".$each['nodialtone']."\t".$each['nocarrier']."\t".$each['other']."\n";
		}
		echo "Total\t\t\t".$total_count."\t".$total_duration."\t".$total_answer."\t".$total_cancel."\t".$total_busy."\t".$total_noanswer."\t".$total_nodialtone."\t".$total_nocarrier."\t".$total_other."\n";
	}else{
		echo "Port\tPhone Number\nAll Calls\tAll Durations\tAnswered\tCanceled\tBusy\tNo Answer\tNo Dialtone\tNo Carrier\tOther\n";
		foreach($show as $each) {
			echo $each['port']."\t".$each['aliasname']."\t".$each['count']."\t".$each['duration']."\t".$each['answer']."\t".$each['cancel']."\t".$each['busy']."\t".$each['noanswer']."\t".$each['nodialtone']."\t".$each['nocarrier']."\t".$each['other']."\n";
		}
		echo "Total\t\t".$total_count."\t".$total_duration."\t".$total_answer."\t".$total_cancel."\t".$total_busy."\t".$total_noanswer."\t".$total_nodialtone."\t".$total_nocarrier."\t".$total_other."\n";
	}
	exit(0);
}

function gsm_inbound_export(){
	global $phonenum_res;
	$file_name = 'gsm_inbound_stats.xls';
	
	$a = 0;
	$res = `asterisk -rx "gsm show statistics inbound" 2> /dev/null`;
	
	if(!empty($res)){
		$total_count = 0;
		$total_duration = 0;
		$total_answer = 0;
		$total_cancel = 0;
		$total_busy = 0;
		$total_other = 0;
		
		$line = explode("\n",$res);
		if(!empty($line)){
			$line_len = count($line) - 1;
			for($i=1;$i<$line_len;$i++){
				$item = explode("|", $line[$i], 9);
				if(isset($item[5])){
					if(strstr(get_gsm_name_by_channel(trim($item[0])),'null')) continue;
					$show[$a]['port'] = get_gsm_name_by_channel(trim($item[0]));
					
					$chn = trim($item[0]);
					$show[$a]['aliasname'] = get_gsm_aliasname($chn,1);
					
					exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $chn",$phone_output);
					$show[$a]['phonenum'] = $phone_output[0];
					$phone_output = '';
					
					$show[$a]['count'] = trim($item[1]);
					$show[$a]['duration'] = trim($item[2]);
					$show[$a]['answer'] = trim($item[3]);
					$show[$a]['cancel'] = trim($item[4]);
					$show[$a]['busy'] = trim($item[5]);
					$other = $show[$a]['count'] - $show[$a]['answer'] - $show[$a]['cancel'] - $show[$a]['busy'];
					$show[$a]['other'] = $other > 0 ? $other : 0;
					
					$total_count += $show[$a]['count'];
					$total_duration += $show[$a]['duration'];
					$total_answer += $show[$a]['answer'];
					$total_cancel += $show[$a]['cancel'];
					$total_busy += $show[$a]['busy'];
					$total_other += $show[$a]['other'];
					
					$a++;
				}
			}
		}
	}
	
	ob_clean();
	flush();
	header("Content-type:application/vnd.ms-excel");
	header("Content-Disposition:attachment;filename=$file_name");

	if($phonenum_res['general']['phonenum_switch'] == 'on'){
		echo "Port\tPhone Number\tMobile Number\tAll Calls\tAll Durations\tAnswered\tMissed\tBusy\tOther\n";
		foreach($show as $each){
			echo $each['port']."\t".$each['aliasname']."\t".$each['phonenum']."\t".$each['count']."\t".$each['duration']."\t".$each['answer']."\t".$each['cancel']."\t".$each['busy']."\t".$each['other']."\n";
		}
		echo "Total\t\t\t".$total_count."\t".$total_duration."\t".$total_answer."\t".$total_cancel."\t".$total_busy."\t".$total_other."\n";
	}else{
		echo "Port\tPhone Number\tAll Calls\tAll Durations\tAnswered\tMissed\tBusy\tOther\n";
		foreach($show as $each){
			echo $each['port']."\t".$each['aliasname']."\t".$each['count']."\t".$each['duration']."\t".$each['answer']."\t".$each['cancel']."\t".$each['busy']."\t".$each['other']."\n";
		}
		echo "Total\t\t".$total_count."\t".$total_duration."\t".$total_answer."\t".$total_cancel."\t".$total_busy."\t".$total_other."\n";
	}
	exit(0);
}
function sms_export(){
	global $db;
	global $__GSM_SUM__;
	global $phonenum_res;
	$file_name = 'sms_stats.xls';
	
	$db = new SMSOUTBOXDB();
	$sql = "select * from sms_out";
	$results = $db->try_query($sql);
	
	ob_clean();
	flush();
	header("Content-type:application/vnd.ms-excel");
	header("Content-Disposition:attachment;filename=$file_name");
	
	$total_all_num = 0;
	$total_deliverd_num = 0;
	$total_sent_num = 0;
	$total_failed_num = 0;
	
	if($phonenum_res['general']['phonenum_switch'] == 'on'){
		echo "Port\tMobile Number\tAll SMS\tDeliverd\tSent\tFailed\n";
	}else{
		echo "Port\tAll SMS\tDeliverd\tSent\tFailed\n";
	}
	for($c=1; $c<=$__GSM_SUM__; $c++){
		$deliverd_num = 0;
		$failed_num = 0;
		$sent_num = 0;
		
		exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $c",$phone_output);
		$phonenum = $phone_output[0];
		$phone_output = '';
		
		while($res = @$results->fetchArray()){
			if($res['port'] == $c){
				if($res['status'] == 0){
					$failed_num++;
				}else if($res['status'] == 1){
					$sent_num++;
				}else if($res['status'] == 2){
					$deliverd_num++;
				}
			}
		}
		
		$all_num = $failed_num+$sent_num+$deliverd_num;
		
		$total_all_num += $all_num;
		$total_deliverd_num += $deliverd_num;
		$total_sent_num += $sent_num;
		$total_failed_num += $failed_num;
		
		$port = get_gsm_name_by_channel($c);
		
		if($phonenum_res['general']['phonenum_switch'] == 'on'){
			echo $port."\t".$phonenum."\t".$all_num."\t".$deliverd_num."\t".$sent_num."\t".$failed_num."\n";
		}else{
			echo $port."\t".$all_num."\t".$deliverd_num."\t".$sent_num."\t".$failed_num."\n";
		}
	}
	if($phonenum_res['general']['phonenum_switch'] == 'on'){
		echo "Total\t\t".$total_all_num."\t".$total_deliverd_num."\t".$total_sent_num."\t".$total_failed_num."\n";
	}else{
		echo "Total\t".$total_all_num."\t".$total_deliverd_num."\t".$total_sent_num."\t".$total_failed_num."\n";
	}
	
	exit(0);
}

if($_POST){
	if($_POST['send'] == 'GSM Outbound Export'){
		gsm_outbound_export();
	}else if($_POST['send'] == 'GSM Inbound Export'){
		gsm_inbound_export();
	}else if($_POST['send'] == 'SMS Export'){
		sms_export();
	}
}else{
	show_statistics();
}
?>
	
<?php require("/www/cgi-bin/inc/boot.inc");?>

<div id="float_btn1" class="sec_float_btn1"></div>
<div  class="float_close" onclick="close_btn()" ></div>