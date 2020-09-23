<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
?>

<?php
if($__deal_cluster__){
	$cluster_info = get_cluster_info();
}
if($cluster_info['mode'] == 'master') {
	$HEAD = $__BRD_HEAD__.'1-';
} else {
	$HEAD = '';
}

function splitcmd($str)
{
	$timeout = substr(strrchr($str, "@"), 1);
	$timeout = trim($timeout);
	$pos = strrpos($str,"@");
	if($pos) {
		$cmd = substr($str,0,$pos);
	} else {
		$cmd = $str;
	}

	return array('cmd' => $cmd, 'timeout' => $timeout);
}

function get_value()
{
	global $cluster_info;
	global $__GSM_SUM__;
	global $__GSM_HEAD__;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $HEAD;

	$ret[]['type'] = '';

	for($i=1; $i<=$__GSM_SUM__; $i++) {
		$index = $HEAD.$__GSM_HEAD__.$i;
		$input = $HEAD.$__GSM_HEAD__.$i."-in";
		$output = $HEAD.$__GSM_HEAD__.$i."-out";
		if(isset($_POST[$input])) {
			$invalue = trim($_POST[$input]);
			if($invalue != '') {
				$res = splitcmd($invalue);
				$ret[$index]['cmd'] = $res['cmd'];
				$ret[$index]['timeout'] = $res['timeout'];
				$ret[$index]['span'] = $i;
				$ret[$index]['output'] = $output;
				$ret[$index]['invalue'] = $invalue;
				$ret[$index]['type'] = 'master';
			}
		}
	}

	return $ret;
}


$allinput = '';
$function['ussd'] = '';
$function['at'] = '';
$function['checknum'] = '';
if($_POST) {
	if(isset($_POST['function'])) {
		switch($_POST['function']) {
		case 'ussd': $function['ussd'] = 'selected'; break;
		case 'at': $function['at'] = 'selected'; break;
		case 'checknum': $function['checknum'] = 'selected'; break;
		default: $function['ussd'] = 'selected'; break;
		}
	}

	if(isset($_POST['allinput'])) {
		$allinput = trim($_POST['allinput']);
		if (strstr($allinput,"\"")) {
			$allinput = str_replace("\"","&quot;",$allinput);
		}
	}

	$allvalue = get_value();
}

//phone number switch
$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$phonenum_res = $aql->query("select * from sim_query.conf");
?>

	<script type="text/javascript" src="/js/functions.js">
	</script>
	<script type="text/javascript">
	function CopyToSel()
	{
		var value = document.getElementById("allinput").value;
		var el = document.getElementsByTagName('textarea');
		//var el2 = document.getElementsByTagName('selinput');
		var el2 = document.getElementsByName('selinput');
		var len = el.length;
		for(var i=0; i<len; i++) {
			if((el[i].id=="input")) {
				if(el2[i/2].checked) {
					el[i].value = value;
				}
			}
		}
	}

	function ClearAll()
	{
		var el = document.getElementsByTagName('textarea');
		var len = el.length;
		for(var i=0; i<len; i++) {
			el[i].value = '';
		}
	}
	</script>

<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	
	<div class="content">
		<div class="tab_item">
			<span>
				<?php echo language('Function');?>:
				<?php if(is_show_language_help('Function')){ ?>
					<div class="tip_main">
						<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
						<div class="tip_help">
							<i class="top" ></i>
						
							<div class="tip_content">
								<b><?php echo language('Function');?>:</b><br/>
								<?php echo language('Function help','
									Get USSD:<br/>
									Enter a specific USSD number<br/>
									(For example,*142# to check your SIM card\'s balance. This USSD number is might be different from different carriers) <br/>
									to get the USSD information.<br/>
									The gateway will try to get by AT commands.<br/></br/>
									Send AT Command:<br/>
									To perform some specific AT commands.<br/>
									This is useful when you have a debug of the GSM modem. e.g: perform [AT+CSQ] to check what signal quality it is.<br/><br/>
									Check Number:<br/>
									Enter a known number(like your mobile phone) to check what number it is of the SIM card.<br/>
									Click "Execute", then the gateway will dial to the number you already input.<br/>
									It only rings for one time and hangs up at once. <br/>
									Not generating telephone charge during this procedure.');
								?>
							</div>
						</div>
					</div>
				<?php } ?>
			</span>
			<div class="tab_item_right">
				<select size=1 name="function" id="function">
					<option value="ussd" <?php echo $function['ussd'];?> ><?php echo language('Get USSD');?></option>
					<option value="at" <?php echo $function['at'];?> ><?php echo language('Send AT Command');?></option>
					<option value="checknum" <?php echo $function['checknum'];?> ><?php echo language('Check Number');?></option>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Action');?>:
			</span>
			<div class="tab_item_right">
				<input type="text" style="width: 200px;" name="allinput" id="allinput" value="<?php echo $allinput ?>"/>
				<?php if(!$only_view){ ?>
				<input type="submit" value="<?php echo language('Execute');?>" onclick="document.getElementById('send').value='Execute';"/>
				<?php } ?>
				<input type="button" name="copy" value="<?php echo language('Copy to Selected');?>" onclick="CopyToSel()" />
				<input type="button" name="clearall" value="<?php echo language('Clear All');?>" onclick="ClearAll()" />
				
				<input type="hidden" name="send" id="send" value="" />
			</div>
		</div>
	</div>
	
	<div class="content">
		<table class="table_show">
			<tr>
				<th style="width: 20px;"><input type="checkbox" name="selall" onclick="selectAll(this.checked,'selinput')" /></th>
				<th style="width: 150px;"><?php echo language('Port');?></th>
				
				<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
				<th><?php echo language('Mobile Number');?></th>
				<?php } ?>
				
				<th style="width: 200px;"><?php echo language('Input');?></th>
				<th><?php echo language('Output');?></th>
			</tr>
			
			<?php
			for($i=1; $i<=$__GSM_SUM__; $i++) {
				$value = $HEAD.$__GSM_HEAD__.$i;

				if(isset($allvalue[$value]['invalue']))
					$invalue = $allvalue[$value]['invalue'];
				else 
					$invalue = '';

				$channel_name = get_gsm_name_by_channel($i);
				if(strstr($channel_name, 'null')){continue;}
				
				$phonenum = '';
				if(($phonenum_res[$i]['query_type']&240) != 0){
					exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $i",$phone_output);
					$phonenum = $phone_output[0];
					$phone_output = '';
				}
				
				echo "<tr>";
				echo "<td>";
				echo "<input type=\"checkbox\" name=\"selinput\" value=\"selinput\" />";
				echo "</td>";
				echo "<td>";
				echo $channel_name;
				echo "</td>";
				
				if($phonenum_res['general']['phonenum_switch'] == 'on'){
					echo "<td>".$phonenum."</td>";
				}
				
				echo "<td><textarea rows=\"1\" style=\"width:98%;\" name=\"$value-in\" id=\"input\" class=\"$value-in\">$invalue</textarea></td>";
				echo "<td><textarea rows=\"1\" style=\"width:98%;\" name=\"$value-out\" id=\"$value-out\" readonly></textarea></td>";
				echo "</tr>";
			}
			?>
		</table>
	</div>
</form>

<?php
ob_flush();
flush();
?>

<?php
function check_phone($via, $check_num)
{
	$cmd = "asterisk -rx \"gsm check phone stat $via $check_num 1\" 2> /dev/null";
	echo "Please wait...<br>";
	ob_flush();
	flush();
	$file = popen($cmd,"r");
	while(!feof($file)) {
		$line = fgets($file);
		echo $line."<br>";
		ob_flush();
		flush();
	}
	pclose($file);

	echo "<br>Check Phone Number Finish<br>";
}

function js_autorows($name,$value)
{
	$class = str_replace("out","in",$name);
	//get line number for <textarea>
	$value_array = explode('\n', $value);
	$line_num = 0;
	if(is_array($value_array)){
		foreach($value_array as $line){
			if(strlen($line)<50){
				$line_num++;
			}else{
				$line_num = ceil($line/50);
			}
		}
	}
	if($line_num > 5)
		$line_num = 5;

$script = <<<EOF
	<script type="text/javascript">
	$("#${name}").attr("rows",$line_num);
	$(".${class}").attr("rows",$line_num);
	</script>
EOF;
//EOF
	echo $script;

}


function js_setvalue($name,$value)
{
	$value = trim($value);
	$value = str_replace("\r\n",'\\n',$value);
	$value = str_replace("\r",'\\n',$value);
	$value = str_replace("\n",'\\n',$value);
	$value = str_replace("\"",'\"',$value);
	
//	echo "<script type=\"text/javascript\">";
//	echo "document.getElementById(\"$name\").innerText = \"$value\";";
//	echo "document.getElementById(\"$name\").innerHTML = \"$value\";";
//	echo "document.getElementById(\"$name\").value = \"$value\";";
//	echo "</script>";

	echo "<script type=\"text/javascript\">";
	echo "$(\"#".$name."\").html(\"".$value."\");";
	echo "</script>";
	
	js_autorows($name,$value);

	ob_flush();
	flush();
}


function parse_ussd($ussd_code)
{
	//Receive USSD format
	//1:Receive USSD on span 2,reponses:2,code:15
	//Text:ussd code

	if(isset($ussd_code[0]) && $ussd_code[0] == '1') {
		$valid_str = substr($ussd_code,strpos($ussd_code,"\n")+1);
		if(strlen($valid_str) > strlen("Text:") ) {
			return substr($valid_str,strlen("Text:"));
		}
	}

	return $ussd_code;
}


function is_receive_ussd($ussd_code)
{
	if(isset($ussd_code[0]) && $ussd_code[0] == '1') {
		return true;
	}

	return false;
}


function send($ary,$type)
{
	switch($type) {
	case 'ussd':
		$gsmcmd = 'send ussd';
		$defaulttimeout = 10000;  //10s
		$hangup = '';
		break;
	case 'at':
		$gsmcmd = 'send sync at';
		$defaulttimeout = 2000;  //2s
		$hangup = '';
		break;
	case 'checknum':
		$gsmcmd = 'check phone stat';
		$defaulttimeout = 20000;  //15s ==> 20s
		$hangup = '1';
		break;
	default:
		return;
	}
	
	//echo "<script type=\"text/javascript\">";
	//echo "$(document).ready(function(){";
	ob_flush();
	flush();
	foreach($ary as $each) {
		if($each['type'] == 'master') {
			$cmd = ast_cmdprocess($each['cmd']);
			$timeout = $each['timeout'];
			if($timeout == '') $timeout = $defaulttimeout;
			if($type == 'ussd' || $type == 'checknum' ) {
				$timeout = intval($timeout / 1000);    //time unit is second
				if($timeout <= 0) $timeout = intval($defaulttimeout / 1000);
			}
			$i = $each['span'];
			$output = $each['output'];
			$astcmd = "asterisk -rx \"gsm $gsmcmd $i \\\"$cmd\\\" $hangup $timeout \" 2> /dev/null";
			$pid = pcntl_fork();
			if ($pid == 0) {
				$outvalue = get_exec($astcmd);
				if($type == 'ussd') {
					for($a=0; $a<3; $a++) {//Max resend
						if(is_receive_ussd($outvalue)) {
							$outvalue = parse_ussd($outvalue);
							break;
						}
						$outvalue = get_exec($astcmd);
					}
				}
				js_setvalue($output,$outvalue);
				exit(0);
			}else if($pid > 0){
				$pids[]=$pid;
			}
		} else if($each['type'] == 'slave') {
			$cmd = ast_cmdprocess($each['cmd']);
			$timeout = $each['timeout'];
			if($timeout == '') $timeout = $defaulttimeout;
			if($type == 'ussd' || $type == 'checknum' ) {
				$timeout = intval($timeout / 1000);    //time unit is second
				if($timeout <= 0) $timeout = intval($defaulttimeout / 1000);
			}
			$i = $each['span'];
			$output = $each['output'];
			$data = "astcmd:gsm $gsmcmd $i \\\"$cmd\\\" $hangup $timeout\n";
			$ip = $each['ip'];
			$pid = pcntl_fork();
			if ($pid == 0) {
				$outvalue = request_slave($ip, $data, intval($timeout/1000)+1);
				if(!$outvalue) {
					$outvalue = "TIMEOUT";
				}
				if($type == 'ussd') {
					for($a=0; $a<3; $a++) {//Max resend
						if(is_receive_ussd($outvalue)) {
							$outvalue = parse_ussd($outvalue);
							break;
						}
						$outvalue = request_slave($ip, $data, intval($timeout/1000)+1);
						if(!$outvalue) {
							$outvalue = "TIMEOUT";
						}
					}
				}
				js_setvalue($output,$outvalue);
				exit(0);
			}else if($pid > 0){
				$pids[] = $pid;
			}
		}
	}
	//echo "});";
	//echo "</script>";
	if(isset($pids) && is_array($pids)) {
		foreach($pids as $pid){
			pcntl_waitpid(-1, $status);
		}
	}
}

function get_newgsm_name_by_oldgsmname($oldname){
	return str_replace('-','',$oldname);
}

function get_newgsm_name_by_output_value($output){
	$db=explode('-',$output);
	
    return "gsm1".".".$db[1];
	
}
	


function get_output_value_by_gsm_name($gsmname){
	global $__BRD_HEAD__;
	global $__GSM_HEAD__;
	global $HEAD;
	
	
	
	$pos=strpos($gsmname,'.',0);
	if ($HEAD==''){ //stand-alone mode
		$result=$__GSM_HEAD__.substr($gsmname,$pos+1,100000)."-out";
	}else{
		$result=$__BRD_HEAD__.substr($gsmname,3,$pos-3)."-".$__GSM_HEAD__.substr($gsmname,$pos+1,100000)."-out";
	}
	
	return $result;
}



function send_new($ary,$type){
	
	$redis_client = new Predis\Client();	
		
	$redis_client->del("app.asterisk.php.feedbacklist");
	$redis_client->del("app.asterisk.php.sendlist");
	
	
	//get system uptime 
	$str = @file("/proc/uptime");
	$str = explode(" ", implode("", $str));
	$defualt_time = trim($str[0]);	
	//defualt retry count
	$default_retry=1;
	$default_sendduration=60; //sec
	
	//task_count;
	$task_count=0;
	
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$res = $aql->query("select * from sms.conf");
	
	$send_attempt=trim($res['send']['attempt']);
	$send_repeat=trim($res['send']['repeat']);
	
	foreach($ary as $each) {
		if(($each['type'] == 'master') or ($each['type'] == 'slave')) {
			/*
			$uuid=create_uuid();
     		$push_str="{";
			$push_str.="\"uuid\":\"".$uuid."\",";
			$push_str.="\"type\":\"".$type."\",";
			js_setvalue($each['output'],'Sending...');
			$push_str.="\"port\":\"".get_newgsm_name_by_output_value($each['output'])."\",";
			$push_str.="\"content\":\"".$each['invalue']."\",";
			$push_str.="\"time\":\"".$defualt_time."\",";
			$push_str.="\"value\":\"".""."\",";
			$push_str.="\"switch\":\"".""."\",";
			$push_str.="\"retry\":\"".$send_attempt."\",";
			$push_str.="\"exten\":\"".$send_repeat."\"";
			$push_str.="}";
			*/
			$push_array=array();
			$uuid=create_uuid();
			$push_array['uuid']=$uuid;
			$push_array['type']=$type;
			js_setvalue($each['output'],'Sending...');
			$push_array['port']=get_newgsm_name_by_output_value($each['output']);
			$push_array['content']=$each['invalue'];
			$push_array['value']="";
			$push_array['switch']="";
			$push_array['time']=$defualt_time;
			$push_array['retry']=$send_attempt;
			$push_array['exten']=$send_repeat;
			$push_str=json_encode($push_array);
	
			
			//echo $push_str."<br>";
			$task_count+=1;			
			//echo $push_str."<br>";
			$redis_client->rpush("app.asterisk.php.sendlist",$push_str);						
		}
	}	
		
	$handle_task=$task_count;
	$timeout_duration=$default_sendduration*($default_retry);
	
	$mark_time=time();
	$timeup=false;

	//listen different list by type(at,ussd,modified)
	$feedbacklist = "app.asterisk.php.feedbacklist"; //modified
	if ($type == 'atcommand') {
		$feedbacklist = "app.asterisk.at.feedbacklist";
	} else if ($type == 'ussd') {
		$feedbacklist = "app.asterisk.ussd.feedbacklist";
	}

	while (($handle_task) && ($timeup==false)){
		$pop_str=$redis_client->lpop($feedbacklist);			
		if ($pop_str<>""){
			$pop_array=json_decode($pop_str,true);
			$output=get_output_value_by_gsm_name($pop_array["port"]);
			
			$pop_array["result"]=str_replace("\n-","",$pop_array["result"]);
			js_setvalue($output,$pop_array["result"]);
			$handle_task-=1;			
			
		}		
		$second=floor((time()-$mark_time));
		if ($second>$timeout_duration){
			$timeup=true;
		}		
	}
	
	if ($timeup){
		foreach($ary as $each) {
			//$each['output']
			echo "<script type=\"text/javascript\">";
			echo "if (document.getElementById('".$each['output']."').value=='Sending...'){";
			echo "document.getElementById('".$each['output']."').value='Over Time';}";
			echo "</script>";
		
		}
	}
}


if($_POST) {
	if(isset($_POST['send']) && isset($_POST['function'])) {
		if($_POST['send'] == 'Execute' && $_POST['function'] == "ussd") {
			//send($allvalue,'ussd');
			send_new($allvalue,'ussd');
		} else if($_POST['send'] == 'Execute' && $_POST['function'] == "at") {
			send_new($allvalue,'atcommand');
		} else if($_POST['send'] == 'Execute' && $_POST['function'] == "checknum") {
			send($allvalue,'checknum');
		}
	}
}
require("/www/cgi-bin/inc/boot.inc");
?>
