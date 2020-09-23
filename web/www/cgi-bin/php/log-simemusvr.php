<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
?>

<?php
if(isset($_GET['board'])) {
	$board = $_GET['board'];
} else {
	$board = get_slotnum();
}

if(isset($_GET['port'])) {
	$port = $_GET['port'];
} else {
	$port = 1;
}

function echo_contents($board, $port){
	if($board != 1) {
		$head = "/$board";
	} else {
		$head = '';
	}

	$url = "'$head/tmp/log/SimEmuSvr/$port'";
?>
<script type="text/javascript">
	$.ajax({
		url: <?php echo $url;?>,
		type: 'GET',
		dataType: 'text', 
		data: {},
		success: function(log_info){
			document.getElementById("showlog").value = log_info;
			document.getElementById("size").value = log_info.length;
		},
	});
</script>

<?php
}

function download_all_emu_log()
{
	global $__GSM_SUM__;

	$date = trim(`date "+%Y%m%d%H%M%S"`);
	$tar_name="Emulog-".$date.".tar.gz";
	if(!file_exists('/tmp/log/emu_log/')){
		mkdir('/tmp/log/emu_log');
	}
	$tar_path="/tmp/log/emu_log/$tar_name";
	
	//pack file
	$pack_cmd="tar vcz -f $tar_path /tmp/log/SimEmuSvr";

	exec("$pack_cmd > /dev/null 2>&1 || echo $?",$output);
	if($output) {
		echo "</br>$pack_cmd";
		echo language("Packing was failed");echo "</br>";
		return;
	}

	if(!file_exists($tar_path)) {
		echo "</br>$tar_name";
		echo language("Can not find");
		return;
	}

	//打开文件  
	$file = fopen ($tar_path, "r" ); 
	$size = filesize($tar_path);

	//输入文件标签 
	header('Content-Encoding: none');
	header('Content-Type: application/force-download');
	header('Content-Type: application/octet-stream');
	header('Content-Type: application/download');
	header('Content-Description: File Transfer');  
	header('Accept-Ranges: bytes');  
	header('Accept-Length: '.$size);  
	header('Content-Transfer-Encoding: binary' );
	header('Content-Disposition: attachment; filename='.$tar_name); 
	header('Pragma: no-cache');
	header('Expires: 0');
	//输出文件内容   
	//读取文件内容并直接输出到浏览器
	ob_clean();
	flush();
	echo fread($file, $size);
	fclose ($file);

	exec("rm -rf /tmp/log/emu_log/");
}

function Log_setting(){
	$aql = new aql();
	$log_logmonitor_path = "/etc/asterisk/gw/logfile_monitor.conf";
	$hlock = lock_file($log_logmonitor_path);
	if (!file_exists($log_logmonitor_path)) {
		fclose(fopen($log_logmonitor_path,"w"));
	}
	$aql->set('basedir','/etc/asterisk/gw/');
	if(!$aql->open_config_file($log_logmonitor_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	
	$res = $aql->query("select * from logfile_monitor.conf");
	
	if(isset($_POST['info'])){
		$info = 1;
	}else{
		$info = 0;
	}
	
	if(isset($_POST['warning'])){
		$warning = 2;
	}else{
		$warning = 0;
	}
	
	if(isset($_POST['error'])){
		$error = 4;
	}else{
		$error = 0;
	}
	$emu_log_class = $info + $warning + $error;
	
	if(isset($_POST['emu_usb_data_logs']) && $_POST['emu_usb_data_logs'] == 'on'){
		$emu_usb_data_logs = 1;
	}else{
		$emu_usb_data_logs = 0;
	}
	
	if(isset($_POST['autoclean']) && $_POST['autoclean'] == 'on'){
		$autoclean = 'on';
		$maxsize = $_POST['maxsize'];
	}else{
		$autoclean = 'off';
		$maxsize = '';
	}
	
	if(isset($_POST['emu_autoclean']) && $_POST['emu_autoclean'] == 'on'){
		$emu_autoclean = 'on';
		$emu_maxsize = $_POST['emu_maxsize'];
	}else{
		$emu_autoclean = 'off';
		$emu_maxsize = '';
	}
	
	if(!isset($res['simemu_logs'])){
		$aql->assign_addsection('simemu_logs','');
	}
	
	if(isset($res['simemu_logs']['emu_log_class'])){
		$aql->assign_editkey('simemu_logs', 'emu_log_class', $emu_log_class);
	}else{
		$aql->assign_append('simemu_logs', 'emu_log_class', $emu_log_class);
	}
	
	if(isset($res['simemu_logs']['emu_usb_data_logs'])){
		$aql->assign_editkey('simemu_logs', 'emu_usb_data_logs', $emu_usb_data_logs);
	}else{
		$aql->assign_append('simemu_logs', 'emu_usb_data_logs', $emu_usb_data_logs);
	}
	
	if(isset($res['simemu_logs']['autoclean_sw'])){
		$aql->assign_editkey('simemu_logs', 'autoclean_sw', $autoclean);
	}else{
		$aql->assign_append('simemu_logs', 'autoclean_sw', $autoclean);
	}
	
	if($maxsize != ''){
		if(isset($res['simemu_logs']['maxsize'])){
			$aql->assign_editkey('simemu_logs', 'maxsize', $maxsize);
		}else{
			$aql->assign_append('simemu_logs', 'maxsize', $maxsize);
		}
	}
	
	if(!isset($res['simemu_log'])){
		$aql->assign_addsection('simemu_log','');
	}
	
	if(isset($res['simemu_log']['autoclean_sw'])){
		$aql->assign_editkey('simemu_log', 'autoclean_sw', $emu_autoclean);
	}else{
		$aql->assign_append('simemu_log', 'autoclean_sw', $emu_autoclean);
	}
	
	if(isset($res['simemu_log']['maxsize'])){
		$aql->assign_editkey('simemu_log', 'maxsize', $emu_maxsize);
	}else{
		$aql->assign_append('simemu_log', 'maxsize', $emu_maxsize);
	}
	
	$aql->save_config_file('logfile_monitor.conf');
	unlock_file($hlock);
	wait_apply("exec", "/etc/init.d/logfile_monitor restart > /dev/null 2>&1 &");
	
	$client = new SoapClient("http://127.0.0.1:8868/?wsdl");
	$result = $client->__soapCall('EMULogSetting', array($emu_log_class, $emu_usb_data_logs), array('location' => 'http://127.0.0.1:8868', 'uri' => 'webproxy'));
}

if($_POST){
	if(isset($_POST['send']) && $_POST['send'] == 'Save'){
		Log_setting();
	}else if(isset($_POST['send']) && $_POST['send'] == 'Download') {
		download_all_emu_log();
	}
}

/** show settings **/
$aql = new aql();
$logfile_monitor_path = "/etc/asterisk/gw/logfile_monitor.conf";
$hlock = lock_file($logfile_monitor_path);
if(!file_exists($logfile_monitor_path)){
	fclose(fopen($logfile_monitor_path, "w"));
}
$aql->set('basedir','/etc/asterisk/gw/');
if(!$aql->open_config_file($logfile_monitor_path)){
	echo $aql->get_error();
	unlock_file($hlock);
	return -1;
}

$res = $aql->query("select * from logfile_monitor.conf");
$emu_log_class = $res['simemu_logs']['emu_log_class'];
$emu_usb_data_logs = $res['simemu_logs']['emu_usb_data_logs'];
$autoclean = $res['simemu_logs']['autoclean_sw'];
$maxsize = $res['simemu_logs']['maxsize'];
$emu_autoclean = $res['simemu_log']['autoclean_sw'];
$emu_maxsize = $res['simemu_log']['maxsize'];
unlock_file($hlock);

?>
	<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
	<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />

	<form enctype="multipart/form-data" action="<?php echo get_self(); ?>" method="post">
		<div id="tab">
			<li class="tb1">&nbsp;</li>
			<li class="tbg" style="width:auto; padding-right:10px;"><?php echo language('SimEmu Logs Settings');?></li>
			<li class="tb2">&nbsp;</li>
		</div>
		
		<table width="100%" class="tedit" >
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo 'SimEmu '.language('Auto clean');?>:
						<span class="showhelp">
						<?php echo language('Auto clean help','
							switch on : when the size of log file reaches the max size, <br> 
							&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; the system will cut a half of the file. New logs will be retained.<br>
							switch off : logs will remain, and the file size will increase gradually. <br>');
							echo language('Auto clean default@Asterisk Logs','default on, default size=100KB.');
						?>
						</span>
					</div>
				</th>
				<td >
					<table><tr>
						<td style="margin:0px;padding:0px;border:0">
							<input type=checkbox id="emu_autoclean" name="emu_autoclean" <?php if($emu_autoclean == 'on')echo 'checked';?> >
						</td>
						<td style="border:0">
							<?php echo language('maxsize');?> : 
							<select id="emu_maxsize" name="emu_maxsize" <?php if($emu_autoclean != "on")echo "disabled";?>>
								<?php
									$value_array = array("20KB","50KB","100KB","200KB","500KB","1MB","2MB");
									foreach($value_array as $value){
										$selected = "";
										if($emu_maxsize == $value)
											$selected = "selected";
										echo "<option value=\"$value\" $selected>$value</option>";
									}
								?>
							</select>
						</td>
					</tr></table>
				</td>
			</tr>
		
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Log_class');?>:
						<span class="showhelp">
						<?php echo language('Log_class help','Displays different log information.');?>
						</span>
					</div>
				</th>
				<td >
					<span style="float:left;margin:5px;"><?php echo language('Info');?>:</span>
					<div style="float:left;margin-right:30px;">
						<input type="checkbox" name="info" <?php if($emu_log_class & 1)echo 'checked';?>/>
					</div>
					<span style="float:left;margin:5px;"><?php echo language('Warning');?>:</span>
					<div style="float:left;margin-right:30px;">
						<input type="checkbox" name="warning" <?php if($emu_log_class & 2)echo 'checked';?>/>
					</div>
					<span style="float:left;margin:5px;"><?php echo language('Error');?>:</span>
					<div style="float:left;margin-right:30px;">
						<input type="checkbox" name="error" <?php if($emu_log_class & 4)echo 'checked';?>/>
					</div>
				</td>				
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('USB data Logs');?>:
						<span class="showhelp">
						<?php echo language('USB data Logs help','Displays different log information.');?>
						</span>
					</div>
				</th>
				<td >
					<input type=checkbox id="emu_usb_data_logs" name="emu_usb_data_logs" <?php if($emu_usb_data_logs == 1)echo 'checked';?>>
				</td>				
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Auto clean');?>:
						<span class="showhelp">
						<?php echo language('Auto clean help','
							switch on : when the size of log file reaches the max size, <br> 
							&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; the system will cut a half of the file. New logs will be retained.<br>
							switch off : logs will remain, and the file size will increase gradually. <br>');
							echo language('Auto clean default@Asterisk Logs','default on, default size=100KB.');
						?>
						</span>
					</div>
				</th>
				<td >
					<table><tr>
						<td style="margin:0px;padding:0px;border:0">
							<input type=checkbox id="autoclean" name="autoclean" <?php if($autoclean == 'on')echo 'checked';?> >
						</td>
						<td style="border:0">
							<?php echo language('maxsize');?> : 
							<select id="maxsize" name="maxsize" <?php if($autoclean != "on")echo "disabled";?>>
								<?php
									$value_array = array("20KB","50KB","100KB","200KB","500KB");
									foreach($value_array as $value){
										$selected = "";
										if($maxsize == $value)
											$selected = "selected";
										echo "<option value=\"$value\" $selected>$value</option>";
									}
								?>
							</select>
						</td>
					</tr></table>
				</td>
			</tr>
		</table>
		<br>
		<button onclick="document.getElementById('send').value='Save';"><?php echo language('Save');?></button>
	
		<br>
		<br>
	
		<div id="tab">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('SimEmu Logs');?></li>
			<li class="tb2">&nbsp;</li>
		</div>
		<center>
			<textarea id="showlog" wrap="on" style="width:100%;height:450px" readonly></textarea>
			<input type="hidden" id="size" value="" />
			<table>
				<tr>
					<td>
						<select size=1 name="channel" id="channel" onchange="change_channel(this.value);">
						<?php
						for($i=1; $i <= $__GSM_SUM__; $i++) {
							$selected = (1 == $board && $i == $port) ? 'selected' : '';
							echo "<option value=\"1.$i\" $selected>";
							echo get_gsm_name_by_channel($i);
							echo "</option>\n";
						}

						$board_array_str = "'1'";
						if($__deal_cluster__){
							$cluster_info = get_cluster_info();
							if($cluster_info['mode'] == 'master') {
								for($b=2; $b<=$__BRD_SUM__; $b++) {
									if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
										$board_array_str .= ",'$b'";
										for($i=1; $i <= $__GSM_SUM__; $i++) {
											$selected = ($b == $board && $i == $port) ? 'selected' : '';
											echo "<option value=\"$b.$i\" $selected>";
											echo get_gsm_name_by_channel($i,$b);
											echo "</option>\n";
										}
									}
								}
							}
						}
						
						$port_array_str = "'1'";
						for($i=2; $i <= $__GSM_SUM__; $i++) {
							$port_array_str .= ",'$i'";
						}
						?>
						</select>
					</td>

					<td><?php echo language('Refresh Rate');?>:</td>
					<td>
						<select id="interval" onchange="change_refresh_rate(this.value);">
							<option value="0" selected>Off</option>
							<option value="1">1s</option>
							<option value="2">2s</option>
							<option value="3">3s</option>
							<option value="4">4s</option>
							<option value="5">5s</option>
							<option value="6">6s</option>
							<option value="7">7s</option>
							<option value="8">8s</option>
							<option value="9">9s</option>
						</select>
					</td>
					<td>
						&nbsp;&nbsp;&nbsp;&nbsp;
					</td>
					<td>
						<input type="button" value="<?php echo language('Refresh');?>" onclick="refresh();"/>
					</td>
					<td>
						<input type="button" value="<?php echo language('Clean Up');?>"  onclick="return CleanUp();"/>
					</td>
					<td>
						<input type="button" value="<?php echo language('Clean All');?>"  onclick="return CleanAll();"/>
					</td>
					<td>
						<input type="hidden" name="send" id="send" value="" />
						<input type="submit" value="<?php echo language('Download');?>"  onclick="document.getElementById('send').value='Download';"/>
					</td>
				</tr>
			</table>
		</center>
	</form>

<script type="text/javascript" src="/js/functions.js">
</script>

<script type="text/javascript">
function show_last()
{
	var t = document.getElementById("showlog");
	t.scrollTop = t.scrollHeight;
}

function CleanUp()
{
	if(!confirm("<?php echo language('Clean Up confirm','Are you sure to clean up this logs?');?>")) return false;
	var port  = <?php echo $port; ?>;
	var size  = $("#size").attr("value");
	
<?php
	if($board == 1) {
		echo "var server_file=\"/service\";\n";
	} else {
		echo "var server_file=\"/$board/service\";\n";
	}
?>


	$.ajax({
		url: server_file+"?random="+Math.random(),      //request file;
		type: 'GET',                                    //request type: 'GET','POST';
		dataType: 'text',                               //return data type: 'text','xml','json','html','script','jsonp';
		data: {
			'port':port,
			'action':'process_log',
			'log_type':'emu_log',
			'method':'clean'
		},
		error: function(data){                          //request failed callback function;
			//alert("get data error");
		},
		success: function(data){                        //request success callback function;
			document.getElementById("showlog").value = '';
			show_last();
		}
	});

}

function CleanAll()
{
	if(!confirm("<?php echo language('Clean All confirm','Are you sure to clean up all logs?');?>")) return false;

	var board_array = [<?php echo $board_array_str;?>];
	var port_array = [<?php echo $port_array_str;?>];
	var size  = $("#size").attr("value");

	for(var i in board_array){
		if(board_array[i] == 1){
			var server_file = "/service";
		}else{
			var server_file = "/"+board_array[i]+"/service";
		}
		for(var j in port_array){
			$.ajax({
				url: server_file+"?random="+Math.random(),      //request file;
				type: 'GET',                                    //request type: 'GET','POST';
				dataType: 'text',                               //return data type: 'text','xml','json','html','script','jsonp';
				data: {
					'port':port_array[j],
					'action':'process_log',
					'log_type':'emu_log',
					'method':'clean'
				},
				error: function(data){                          //request failed callback function;
					//alert("get data error");
				},
				success: function(data){                        //request success callback function;
					document.getElementById("showlog").value = '';
					show_last();
				}
			});
		}
	}
}

var updateStop = false;
function change_refresh_rate(value) {
	setCookie("cookieInterval", value);
	if(value != 0 && updateStop){
		updateStop = false;
		update_log();
	}
}

function change_channel(value) {
	str = value.split(".");
	board = str[0];
	port = str[1]; 
	window.location.href="<?php echo get_self()?>"+"?board="+board+"&port="+port;
}

function refresh() {
	window.location.href="<?php echo get_self()?>"+"?board="+<?php echo $board;?>+"&port="+<?php echo $port;?>;
}

function update_log() {
	var port  = <?php echo $port; ?>;
	var size  = $("#size").attr("value");
	
<?php
	if($board == 1) {
		echo "var server_file=\"/service\";\n";
	} else {
		echo "var server_file=\"/$board/service\";\n";
	}
?>

	
	$.ajax({
		url: server_file+"?random="+Math.random(),      //request file;
		type: 'GET',                                    //request type: 'GET','POST';
		dataType: 'text',                               //return data type: 'text','xml','json','html','script','jsonp';
		data: {
			'port':port,
			'action':'process_log',
			'log_type':'emu_log',
			'method':'update',
			'size':size
		},
		error: function(data){                          //request failed callback function;
			//alert("get data error");
		},
		success: function(data){                        //request success callback function;
			var pos = data.indexOf('&');
			var size = data.substring(0,pos);
			var contents = data.substring(pos+1);
			if(size=='' || size<0) size = 0;
			$("#size").attr("value", size);
			if(size == 0) {
				document.getElementById("showlog").value = '';
				show_last();
			} else {
				if(contents != "") {
					var t = document.getElementById("showlog");
					t.value += contents;
					t.scrollTop = t.scrollHeight;
					show_last();
				}
			}
		},
		complete: function(){
			var timeout = $("#interval").attr("value");
			if( timeout != 0) {
				setTimeout(function(){update_log();}, timeout*1000);
			}else{
				updateStop = true;
			}
		}
	});
}

function cookie_update() {
	var cookieInterval = getCookie('cookieInterval');
	var nowInterval = document.getElementById("interval");

	if (cookieInterval == null) {
		setCookie("cookieInterval", nowInterval.value)
	} else {
		nowInterval.value = cookieInterval;
	}
}

$(document).ready(function(){
	cookie_update();
	show_last();
	var timeout = $("#interval").attr("value");
	if( timeout != 0) {
		update_log();
	}else{
		updateStop = true;
	}
	
	$(":checkbox").iButton();
	$("#autoclean").change(function(){$("#maxsize").attr("disabled", !$("#autoclean").attr("checked"));});
	$("#emu_autoclean").change(function(){$("#emu_maxsize").attr("disabled", !$("#emu_autoclean").attr("checked"));});
});
</script>
<?php
echo_contents($board,$port);
?>
<?php require("/www/cgi-bin/inc/boot.inc");?>
