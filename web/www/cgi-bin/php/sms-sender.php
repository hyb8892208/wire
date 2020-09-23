<?php
session_start();
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/define.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
include_once("/www/cgi-bin/inc/send_sms.php");

function get_newgsm_name_by_output_value($output){
	$result='';
	$result = "gsm1."."$output";

	return $result;
	
}

function get_newgsm_name_by_output_value2($output){
	global $__BRD_HEAD__;
	global $__GSM_HEAD__;	
	$db=explode('-',$output);
	
	if ($db[0]=='Board'){// master-slave mode example:Borad-1-gsm-1-out
		return "gsm-".$db[1].".".$db[3];
	}else{// stand alone mode; example:gsm-1-out
		return $output;		
	}	
}



function trace($context) {
	echo "$context";
	echo "<br>";
	ob_flush();
	flush();
}

function trace_sms($msg, $to, $span, $retry,$result, $flag = "tr") {
	if($flag == "tr"){		
		$msg = str_replace("''", "'", $msg);
		echo "<tr align='center' style='background-color: rgb(232, 239, 247);'>";
		echo "<td align='left' style='width:45%;word-break:break-all;'>$msg</td>";
		echo "<td style='width:15%'>$to</td>";
		echo "<td style='width:12%'>$span</td>";
		echo "<td style='width:12%'>$retry</td>";
		echo "<td style='width:8%'>$result</td>";
		echo "</tr>";
	}else if($flag == "table_start"){
		
		echo "<table style='width:100%;font-size:12px;border:1px solid rgb(59,112,162);'>";
		echo "<tr style='background-color:#D0E0EE;height:26px;'>";
		echo "<th style='width:45%;word-break:break-all;'>$msg</th>";
		echo "<th style='width:15%'>$to</th>";
		echo "<th style='width:12%'>$span</th>";
		echo "<th style='width:12%'>$retry</th>";
		echo "<th style='width:8%'>$result</th>";
		echo "</tr>";
	}else if($flag == "table_end"){
		echo "</table>";
	}
	ob_flush();
	flush();
}

function trace_sms_statistics($total, $success, $fail){
		$total   = $success + $fail;		
		$table   = "<b>Statistics Report</b>";
		$table  .= "<table style='width:100%;font-size:12px;border:1px solid rgb(59,112,162);'>";
		$table 	.=	"<tr style='background-color:#D0E0EE;height:26px;'>";
		$table 	.=		"<th style='width:30%'>Total</th>";
		$table 	.=		"<th style='width:30%'>Success</th>";
		$table 	.=		"<th style='width:30%'>Fail</th>";
		$table 	.=	"</tr>";
		$table 	.=	"<tr align='center' style='background-color: rgb(232, 239, 247);'>";
		$table 	.=		"<td style='width:30%'>$total</td>";
		$table 	.=		"<td style='width:30%'>$success</td>";
		$table 	.=		"<td style='width:30%'>$fail</td>";
		$table 	.=	"</tr>";
		$table 	.= "</table>";

		$script  = "<script type='text/javascript'>";
		$script .= 	"$(function(){\$('#report').after(\"$table\");});";
		$script .= "</script>";
		
		echo $script;
		

		ob_flush();
		flush();
	}

function trace_sms_error($error,$status) {
	$table = "<br>";
	$table .= "<b>Process Report</b>";
	$table .= "<table style='width:100%;font-size:12px;border:1px solid rgb(59,112,162);'>";
	$table .= "<tr style='background-color:#D0E0EE;height:26px;'>";
	$table .= 		"<th style='width:100%'>Error </th>";
	$table .= "</tr>";
	$table .= "<tr align='center' style='background-color: rgb(232, 239, 247);'>";
	$table .= 		"<td style='width:100%'>$status:$error</td>";
	$table .= "</tr>";
	$table .= "</table>";

	$script  = "<script type='text/javascript'>";
	$script .= 	"$(function(){\$('#report').after(\"$table\");});";
	$script .= "</script>";	
	
	echo $script;
	
	ob_flush();
	flush();
	
}

function get_module_state($channel){
	exec("grep -rn ^State /tmp/gsm/$channel",$temp);
	$module_select = explode(': ', $temp[0]);
	$module_state = $module_select[count($module_select)-1];
	
	return $module_state;
}
?>




<script type="text/javascript" src="/js/functions.js"></script>
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<script type="text/javascript">
function check_send()
{
	var inputs = document.getElementsByTagName("input");
	var check_sum = 0;
	var check_false_sum = 0;
	for(var i=0;i<inputs.length;i++){
		if(inputs[i].getAttribute("type") == "checkbox"){
			check_sum++;
			if(inputs[i].checked == false)
				check_false_sum++;
		}
	}
	if(check_sum == check_false_sum){
		alert("<?php echo language('select ports','Please select ports!');?>");
		return false;
	}

	var num = document.getElementById("dest_num").value;
	if(num == "") {
		alert("Please intput destination number!");
		return false;
	} 

	var msg = document.getElementById("msg").value; 
	//if(msg == "") {
	//	alert("Please intput message!");
	//	return false;
	//}

	return true;
}


function stop_loading()
{
	if (window.stop)
		window.stop();
	else
		document.execCommand("Stop");
}

function readfiles() {

	if(typeof window.ActiveXObject != 'undefined') {
/*		alert("en");
		var files = document.getElementById('number_book').files;
		var content = "";
		try {
			var fso = new ActiveXObject("Scripting.FileSystemObject"); 
			var name = $("#number_book").val();
			alert('"'+name+'"');
			var name = document.getElementById('number_book').value;
			alert(name);
			
			var reader = fso.openTextFile("F:\\3.项目\\24.sms_sender\\phone_number_book_not_valid.txt", 1);
			while(!reader.AtEndofStream) {
				content += reader.readline();
				content += "\n";
			}	
			alert(content);
			reader.close(); // close the reader
		} catch (e) {
			alert("Internet Explore read local file error: \n" + e);
			return;
		}
		document.getElementById("dest_num").value=content;
*/		return;
	}else{
		var files = document.getElementById('number_book').files;
		if (!files.length) {
			alert('Please select a file!');
			return;
		}
		var file = files[0];

		var reader = new FileReader();

		// If we use onloadend, we need to check the readyState.
		reader.onloadend = function(evt) {
			if (evt.target.readyState == FileReader.DONE) { // DONE == 2
				document.getElementById('byte_content').textContent = evt.target.result;
				//document.getElementById('byte_range').textContent = ['Read bytes: ', ' 0 ', ' - ', ' file end ',' of ', file.size, ' byte file'].join('');
			}
		};

		//read file;
		reader.readAsBinaryString(file);
		reader.onload=function(e){
			document.getElementById("dest_num").value=this.result;
		}
		return;
	}
}

function cookie_update() {
	if($("#flash_sms").attr("checked")=="checked"){
		setCookie("cookieFlashSms", "1");
	} else {
		setCookie("cookieFlashSms", "0");
	}
}
$(document).ready(function (){ 
	cookie_update();
	$("#flash_sms").iButton();
	$("#select_all").click(function(){ 
		var that = $(this);
		$(".port_sel_class").each(function(){
			if($(this).attr('disabled') != 'disabled'){
				$(this).attr("checked", $(that).is(':checked')); 
			}
		});
	}); 
    
	$(".port_sel_class").click(function(){ 
		if(!$(this).is(':checked')) {
			$("#select_all").attr("checked", false); 
	 	 }
	});

}); 
</script>

<?php
$_SEND=false;
$_STOP=false;
$flash_sms = 0;
if($_POST) {
	if(
		isset($_POST['spans']) &&
		is_array($_POST['spans']) &&
		isset($_POST['dest_num']) &&
		isset($_POST['msg'])
		) {

		$spans = $_POST['spans'];
		
		$sms = new SendSMS();
		$sms->spans_init();
		$deal_span_ret = $sms->dealSpans($spans);
		if($deal_span_ret != false) {
			
			$spans=$sms->a_spans; 
		
			$spans_list=array();
		
			foreach($spans as $key1=>$board) {
				foreach($board as $key2=>$span) {
					$span_checked[$key1][$key2] = true;				
					$spans_list[]=$spans[$key1][$key2];
				}
			}

			$spans=$spans_list;

			$dest_num = trim($_POST['dest_num']);
			//Allow send blank character
			//$send_msg = trim($_POST['msg']);
			$send_msg = $_POST['msg'];
			//$send_msg = str_replace("'", "''", $send_msg);
			if(isset($_POST['flash_sms'])){
				$flash_sms = 1;
			} else {
				$flash_sms = 0;
			}

		/*if($_FILES && isset($_FILES['number_book']['error']) && $_FILES['number_book']['error'] == 0) {  //Update successful
			$book_string = file_get_contents($_FILES['number_book']['tmp_name']);
			$dest_num .= " ".$book_string;
		}*/
		
			if(isset($_POST['send'])&& $_POST['send']=='Send'){
				$_SEND=true;
			}
			if(isset($_POST['send'])&& $_POST['send']=='Stop'){
				$_STOP=true;
			
			}
	}	}
}
?>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post" onsubmit="return check_send()"> 

	<table width="100%" class="tedit" >
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Port');?>:
					<span class="showhelp">
					<?php echo language('Port help@sms-sender','To select on which GSM channel(s) to send SMS and enter what message you want to send.');?>
					</span>
				</div>
			</th>
			<td>
				<table cellpadding="0" cellspacing="0" class="port_table">
<?php
				for($i=1;$i<=$__GSM_SUM__;$i++){
					$port_name = get_newgsm_name_by_output_value2(get_gsm_name_by_channel($i));
					if(strstr($port_name, 'null')) continue;
					
					$module_state = get_module_state($i);
					if(strstr($module_state, 'READY') || strstr($module_state, 'CALL PROGRESS') || strstr($module_state, 'CALL ACTIVE') || strstr($module_state, 'RING') || strstr($module_state, 'HANGUP')){
						$disabled = '';
						$color = '';
					}else{
						$disabled = 'disabled';
						$color = 'style="color:grey;"';
					}
					
					$checked =  isset($span_checked[1][$i]) ? 'checked' : '';
					echo "<td class='module_port'><input type='checkbox' name='spans[1][$i]' class='port_sel_class' value='".
						get_newgsm_name_by_output_value($i)."' $disabled $checked>";
					echo "<span $color>".$port_name."</span>";
					echo '</td>';
				}

				if($__deal_cluster__){
					$cluster_info = get_cluster_info();
					if($cluster_info['mode'] == 'master') {
						for($b=2; $b<=$__BRD_SUM__; $b++) {
							if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
								for ($line=0;$line<$__GSM_SUM__/4;$line++) {
									echo '<tr>';
									for($i=1+$line*4; $i<=(4+$line*4); $i++) {
										if ($i>$__GSM_SUM__) break;
										$checked =  isset($span_checked[$b][$i]) ? 'checked' : '';
										echo "<td><input type=\"checkbox\" class=\"port_sel_class\" name=\"spans[$b][$i]\" value=\"".get_newgsm_name_by_output_value(get_gsm_name_by_channel($i,$b))."\" $checked>";
										echo get_newgsm_name_by_output_value2(get_gsm_name_by_channel($i,$b));
										echo '&nbsp;&nbsp;</td>';
									}
									echo '</tr>';
								}
							}
						}
					}
				}
?>
				<tr style="border:none;">
					<td style="border:none;"><input type="checkbox" id="select_all"><?php echo language('All');?></td>
				</tr>
				</table>
			</td>
		</tr>
		<?php 
		if(!$flag_cdma) {
		?>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Flash SMS');?>:
					<span class="showhelp">
					<?php echo language('Flash SMS help');?>
					</span>
				</div>
			</th>
			<td >
				<input type="checkbox" id="flash_sms" name="flash_sms" style="" onchange="" <?php if($flash_sms == 1) echo "checked";?> />
			</td>
		</tr>
		<?php 
			}
		?>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Load numbers from text file');?>:
					<span class="showhelp">
					<?php echo language('Load numbers from text file help');?>
					</span>
				</div>
			</th>
			<td >
				<input type="file" id="number_book" name="number_book" style="" onchange="readfiles();" value="yes"/>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Destination Number');?>:
					<span class="showhelp">
					<?php echo language('Destination Number help',"
						The number will receive the message.<br/>
						You will be able to separate each number by these symbols: '\r', '\n', space character, semicolon, comma.<br/>
						If you have more than one destination numbers.");
					?>
					</span>
				</div>
			</th>
			<td >

<?php
			 if(isset($_POST['dest_num'])) {
				 $value = str_replace("\"","&quot;",$_POST['dest_num']);
			 } else {
				 $value = "";
			 }
			 echo "<textarea id=\"dest_num\" name=\"dest_num\" width=\"100%\" rows=\"5\" cols=\"80\" >$value</textarea>";
?>
			<br><h4><?php echo language('Destination Number help2',"\"; semicolon\" , \"| vertical Bar\" , \" , comma \" , \"   blank \" , \" : colon \" , \" . dot \" were treated as separators in Destination Number List");?></h4>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Message');?>:
					<span class="showhelp">
					<?php echo language('Message help','The SMS content to be sent out.');?>
					</span>
				</div>
			</th>
			<td >
				<textarea id="msg" name="msg" rows="18" cols="80" width="100%"><?php if(isset($send_msg)) echo str_replace("''", "'", $send_msg);?></textarea>
			</td>
		</tr>
		<tr>
			<th><?php echo language('Action');?>:</th>
			<td>
				<input type="hidden" name="send" id="send" value="" />
				<input type="submit" value="<?php echo language('Send');?>" onclick="cookie_update();document.getElementById('send').value='Send';"/>&nbsp;&nbsp;
				<input type="submit" value="<?php echo language('Stop');?>" onclick="document.getElementById('send').value='Stop';" />
			</td>
		</tr>						
	</table>

	</form>



<?php
	if ($_STOP){
		$redis_client = new Predis\Client();
		
		$redis_client->del("app.asterisk.php.feedbacklist");
		$redis_client->del("app.asterisk.php.sendlist");
		$redis_client->del("app.asterisk.sms.sendspans");
		for ($i=0;$i<count($spans);$i++){						
			$redis_client->del("app.asterisk.php.".$spans[$i]);
		}
		
	}
	if($_SEND) {

		$redis_client = new Predis\Client();
		
		$spans_count=count($spans);
		$redis_client->del("app.asterisk.php.sendlist");
		$redis_client->del("app.asterisk.sms.sendspans");
		if($spans_count > 16){
			$new_span_arr = array(); 
			$index  =0; 
			$i_max = 4; 
			$j_max = $spans_count/4; 
			for ($i = 0; $i < $i_max; $i++) { 
			   for($j = 0; $j < $j_max; $j++) { 
					$new_index = $j * 4 + $i;
					if(isset($spans[$new_index])) {
						$redis_client->del("app.asterisk.php.".$spans[$new_index]); 
					    $redis_client->rpush("app.asterisk.sms.sendspans",$spans[$new_index]); 
					    $new_span_arr[$index] = $spans[$new_index]; 
					    $index++; 
					}
			   } 
			}
		} else {
			for ($i=0;$i<count($spans);$i++){						
				if(isset($spans[$i])){
					$redis_client->del("app.asterisk.php.".$spans[$i]);
					$redis_client->rpush("app.asterisk.sms.sendspans",$spans[$i]);
				}	
			}
		}

		$redis_client->del("app.asterisk.php.feedbacklist");
		
		$dest_num=str_replace(' ',',',$dest_num);
		$dest_num=str_replace(';',',',$dest_num);
		$dest_num=str_replace('|',',',$dest_num);
		$dest_num=str_replace('.',',',$dest_num);
		$dest_num=str_replace(':',',',$dest_num);
		$dest_num=str_replace('&nbsp;',',',$dest_num);
		//$dest_num=str_replace('\n\r',',',$dest_num);
		//$dest_num=str_replace('\r',',',$dest_num);
		//$dest_num=str_replace('\n',',',$dest_num);
		//$dest_num=str_replace('\t',',',$dest_num);
		$dest_num=str_replace(PHP_EOL,',',$dest_num);
		$dest_num=explode(',',$dest_num);
		$count_number=count($dest_num);
		
		//get system uptime 
		$str = @file("/proc/uptime");
		$str = explode(" ", implode("", $str));
		$defualt_time = trim($str[0]);	
		$detect_time=(int)($defualt_time);
		
		$sum_success=0;
		$sum_failed=0;
		$sum_overtime=0;
		
		$aql = new aql();
		$aql->set('basedir','/etc/asterisk/gw');
		$res = $aql->query("select * from sms.conf");
		
		$send_attempt=trim($res['send']['attempt']);
		$send_repeat=trim($res['send']['repeat']);
			
		
		$send_uuid_list=array();

		$send_count=0;
		$reject_list="";
		
		$span_id=0;
		if($spans_count > 16){
			for ($i=0; $i<$count_number; $i++){
				if (trim($dest_num[$i])<>""){
					$tmp=preg_match ("/^[+]?[0-9]*$/", trim($dest_num[$i]));
					if ($tmp==0) {					
						$reject_list.=$dest_num[$i].",";
						continue;
					}
					$uuid=create_uuid();
					$send_uuid_list[]=$uuid;
					$push_array=array();
					$uuid=create_uuid();
					$push_array['uuid']=$uuid;
					$send_uuid_list[]=$uuid;
					$push_array['type']='sms';
					$push_array['port']=$new_span_arr[$span_id];
					$push_array['content']=$send_msg;
					$push_array['value']=trim($dest_num[$i]);
					$span_id++;
					if ($span_id == $spans_count){
						$span_id=0;
					}
					
					//echo $push_array['port']."===$i====<br>";
					$push_array['switch']=$flash_sms;
					$push_array['retry']=$send_attempt;
					$push_array['exten']=$send_repeat;
					$push_str=json_encode($push_array);
					
					
					$redis_client->rpush("app.asterisk.php.sendlist",$push_str);			
					$send_count++;
				}
			}
		} else {
			for ($i=0;$i<$count_number;$i++){
				if (trim($dest_num[$i])<>""){
					$tmp=preg_match ("/^[+]?[0-9]*$/", trim($dest_num[$i]));
					
					
					if ($tmp==0) {					
						$reject_list.=$dest_num[$i].",";
						continue;
					}
					$uuid=create_uuid();
					$send_uuid_list[]=$uuid;
					$push_array=array();
					$uuid=create_uuid();
					$push_array['uuid']=$uuid;
					$send_uuid_list[]=$uuid;
					$push_array['type']='sms';
					$push_array['port']=$spans[$span_id];
					$push_array['content']=$send_msg;
					$push_array['value']=trim($dest_num[$i]);
					$span_id++;
					if ($span_id==$spans_count){
						$span_id=0;
					}
					$push_array['switch']=$flash_sms;
					$push_array['retry']=$send_attempt;
					$push_array['exten']=$send_repeat;
					$push_str=json_encode($push_array);
					
					
					$redis_client->rpush("app.asterisk.php.sendlist",$push_str);			
					$send_count++;
				}
			}
		}
		
		if (trim($reject_list)<>""){
		echo "<font color='clred'> \"".rtrim($reject_list,",")."\" were rejected for unsupported format</font><br>";
		}
		
		$mark_time=time();
		$timeout_duration=120;
		
		$handle_task=$send_count;
		
		$timeup=false;
		$pop_str = "";
		
		trace("<br><div id=\"report\"></div>");
		
		trace("<b>Detail Report</b>");
		trace_sms("Message", "Destination Number","Port","Retry","Result", "table_start");
		while (($handle_task) && ($timeup==false)){
			for($i=0;$i<12;$i++) { 
				$blpop_str=$redis_client->blpop("app.asterisk.php.feedbacklist",10);//$timeout_duration);			
				if (isset($blpop_str)) {
					$pop_str=$blpop_str[1];
					break;
				}
			}
			
			//$pop_str=$redis_client->lpop("app.asterisk.php.feedbacklist");
			if ($pop_str<>""){
				$pop_array=json_decode($pop_str,true);
				$result_dest=$pop_array["value"];
				$result_sms=$pop_array["result"];
				
				
				if ($result_sms=='succeed'){
					$sum_success+=1;
				}else{
					$sum_failed+=1;
				}
				
				//js_setvalue($output,$pop_array["context"]);
				
				for ($i=0;$i<count($send_uuid_list);$i++){
					if ($send_uuid_list[$i]==$pop_array["uuid"]) {
						$handle_task-=1;
						
					}
				}
				
				
				
				$mark_time=time();
				
				//$tmpstr=str_replace('gsm','gsm-',$pop_array["port"]);
				//$tmpstr = change_port_form($tmpstr);
				$tmpstr = change_sms_routing_form_new($pop_array["port"]);
				trace_sms($pop_array["content"], $result_dest,$tmpstr,$pop_array["retry"],$result_sms, "tr");
				$pop_str = "";
			}
			
			$second=floor((time()-$mark_time));
			if ($second>$timeout_duration){
				$timeup=true;				
			}					
			/*
			if ($timeup){
			foreach($ary as $each) {
				//$each['output']
				echo "<script type=\"text/javascript\">";
				echo "if (document.getElementById('".$each['output']."').value=='Sending...'){";
				echo "document.getElementById('".$each['output']."').value='Over Time';}";
				echo "</script>";
		
			}
			}
			*/
			//ob_flush();
			//flush();
			//sleep(1);						
		}	
		
		
		trace_sms("Message", "Destination Number","Port","Retry","Result", "table_end");		
		
		if ($timeup==true){
			$errorstr =  "Pop sms response from redis <font color=ff00000>TIMEOUT</font>.<br> 
			Make sure SMS_SEND status(SIM card,network,asterisk,etc.) is fine.<br>";
			$status = "ERROR";
			$redis_client->del("app.asterisk.php.sendlist");
			for ($i=0;$i<count($spans);$i++){						
				$redis_client->del("app.asterisk.php.".$spans[$i]);
			}
			$redis_client->del("app.asterisk.php.feedbacklist");
				
			trace_sms_error($errorstr,$status);
		}
		trace_sms_statistics(123,$sum_success,$sum_failed);
		
		//echo "<pre>";
		//print_r($redis_client->lrange("app.asterisk.php.feedbacklistlist",0,-1));  //结果：Array ( [0] => 222 [1] => 111 );
		//echo "</pre>";
		
		//for ($numberid=0)
		
		/*
		foreach($spans as $each) {
		if(($each['type'] == 'master') or ($each['type'] == 'slave')) {			
			$uuid=create_uuid();
     		$push_str="{";
			$push_str.="\"uuid\":\"".$uuid."\",";
			
			//js_setvalue($each['output'],'Sending...');
			
			$push_str.="\"port\":\"".$each['output'])."\",";
			$push_str.="\"context\":\"".$each['invalue']."\",";
			$push_str.="\"time\":\"".$defualt_time."\",";
			$push_str.="\"retry\":\"".$default_retry."\"}";
			//echo $push_str."<br>";
			$task_count+=1;			
			$redis_client->rpush("app.asterisk.ussd.sendlist",$push_str);						
			
		}	
		*/
		
		
	}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>



