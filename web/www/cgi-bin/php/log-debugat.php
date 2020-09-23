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

if(isset($_POST['send']) && $_POST['send'] == 'Download') {
	download_all_at_log();
}

function echo_contents($board, $port)
{
	if($board != 1) {
		$head = "/$board";
	} else {
		$head = '';
	}

	//for 1-4 port
	$product_type = get_product_type();
	
	if($product_type < 4){
		$url = "'$head/tmp/log/asterisk/at/$port'";
	}else{
		$url = "'$head/tmp/var/log/asterisk/at/$port'";
	}
?>
<script type="text/javascript">
	$.ajax({
		url: <?php echo $url;?>,
		type: 'GET',
		dataType: 'text', 
		data: {},
		success: function(log_astinfo){
			document.getElementById("showlog").value = log_astinfo;
			document.getElementById("size").value = log_astinfo.length;
		},
	});
</script>

<?php
}

/*
function execute_at_command()
{
	global $__GSM_SUM__;
	global $__BRD_SUM__;
	global $__GSM_HEAD__;
	global $__BRD_HEAD__;
	$port = '';
	$board = '';
	$timeout = 2000;  //2s
	$execute_at = $_POST['execute_at'];
	$cmd = ast_cmdprocess($execute_at);
	$port = $_POST['port'];
	$board = $_POST['board'];
	if($board == 1){
		$astcmd = exec("asterisk -rx \"gsm send sync at $port \\\"$cmd\\\" $timeout \" 2> /dev/null");
	} else {
		$cluster_info = get_cluster_info();
		for($b=2; $b<=$__BRD_SUM__; $b++) {
			if($b == $board){
				if(isset($cluster_info[$__BRD_HEAD__.$b.'_ip']) && $cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					$data = "astcmd:gsm send sync at $port \\\"$cmd\\\" $timeout\n";
					$outvalue = request_slave($cluster_info[$__BRD_HEAD__.$b.'_ip'], $data, intval($timeout/1000)+1);
				}
			}
		}
	}
}*/
?>

<?php
function download_all_at_log()
{
	global $__GSM_SUM__;
	global $__BRD_SUM__;
	global $__GSM_HEAD__;
	global $__BRD_HEAD__;
	global $__deal_cluster__;

	$hlock = lock_file(get_self());
	
	//for 1-4 port
	$product_type = get_product_type();
	
	if($product_type < 4){
		$at_log_path = '/tmp/log/asterisk';
	}else{
		$at_log_path = '/tmp/var/log/asterisk';
	}
	
	$tar_dir = $at_log_path."/ATlog";
	
	if(file_exists($tar_dir) && is_dir($tar_dir))
		exec("rm -rf $tar_dir");
	if(file_exists($tar_dir) || is_file($tar_dir))
		unlink($tar_dir);
	if(!mkdir($tar_dir)){
		unlock_file($hlock);
		return false;
	}
	for($i=1;$i<=$__GSM_SUM__;$i++){
		$src = $at_log_path.'/at/'.$i;
		$des = $tar_dir.'/'.get_gsm_name_by_channel($i,1);;
		if(file_exists($at_log_path.'/at/'.$i)){
			copy($src, $des);
		}else{
			touch($des);
		}
	}

	if($__deal_cluster__){
		$cluster_info = get_cluster_info();
		if($cluster_info['mode'] == 'master') {
			for($board = 2; $board <= $__BRD_SUM__; $board++){
				if(isset($cluster_info[$__BRD_HEAD__.$board.'_ip']) && $cluster_info[$__BRD_HEAD__.$board.'_ip'] != '') {
					$slaveip = $cluster_info[$__BRD_HEAD__.$board.'_ip'];
					$log_type = 'at_log';
					$action_type = ''; 
					$size = 0;
					for($port = 1; $port <= $__GSM_SUM__; $port++){
						$slave_atlog = '';
						$contents = '';
						$slave_atlog = slave_process_log($slaveip, $log_type, $action_type, $size, $port);

						$log_array = explode('&', $slave_atlog, 2);
						if(file_exists($tar_dir)){
							$des = $tar_dir.'/'.get_gsm_name_by_channel($port,$board);
							if(isset($log_array[1]) && $log_array[1] != ''){
								file_put_contents($des, $log_array[1]);
							}else{
								touch($des);
							}
						}
					}
				}   
			}
		} 
	}

	$date = trim(`date "+%Y%m%d%H%M%S"`);
	$tar_name="ATlog-".$date.".tar.gz";
	$tar_path="/tmp/$tar_name";

	//pack file
	$pack_cmd="tar vcz -f $tar_path -C $at_log_path ./ATlog";
	//echo $pack_cmd;

	exec("$pack_cmd > /dev/null 2>&1 || echo $?",$output);
	if($output) {
		echo "</br>$pack_cmd";
		echo language("Packing was failed");echo "</br>";
		unlock_file($hlock);
		return;
	}
	exec("rm -rf $at_log_path/ATlog");

	if(!file_exists($tar_path)) {
		echo "</br>$tar_name";
		echo language("Can not find");
		unlock_file($hlock);
		return;
	}

	//打开文件  
	$file = fopen ($tar_path, "r" ); 
	$size = filesize($tar_path) ;

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

	unlink($tar_path);
	unlock_file($hlock);
}

?>
	<form enctype="multipart/form-data" action="" method="post" name="at_command">
		<b><?php echo language('Send AT Commands');?>&nbsp;:</b>
		<input type="text" style="width: 200px;" name="execute_at" id="execute_at" value=""/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		<input type="button" value="<?php echo language('Send');?>" onclick="send_at_command();"/>
	</form>
	
	<br />
	
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('AT Commands Logs');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	<center>
		<textarea id="showlog" wrap="on" style="width:100%;height:450px" readonly></textarea>
		<input type="hidden" id="size" value="" />
		<form id="manform" enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
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
					<input type="submit" value="<?php echo language('Download');?>"  onclick="document.getElementById('send').value='Download';return true;"/>
				</td>
			</tr>
		</table>
		</form>
	</center>
	

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
			'log_type':'at_log',
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
					'log_type':'at_log',
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
	/*board = value.substr(0,1);
	port = value.substr(1,1);
	*/
	str = value.split(".");
	board = str[0];
	port = str[1]; 
	window.location.href="<?php echo get_self()?>"+"?board="+board+"&port="+port;
}

function send_at_command() {
	var value = document.getElementById("channel").value;
	str = value.split(".");
	board = str[0];
	port = str[1]; 
	var at_command = '';
	at_command = document.getElementById("execute_at").value;
	if(board == 1){
		var server_file="/service";
	}else{
		var server_file="/"+board+"/service";
	}
	$.ajax({
		url: server_file+"?random="+Math.random(),      //request file;
		type: 'GET',                                    //request type: 'GET','POST';
		dataType: 'text',                               //return data type: 'text','xml','json','html','script','jsonp';
		data: {
			'action':'sendat',
			'cmd':at_command,
			'span':port,
			'timeout':'2000'
		},
		error: function(data){                          //request failed callback function;
			//alert("get data error");
		},
		success: function(data){                        //request success callback function;
			update_log();
		},
		complete: function(){
		}
	});
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
			'log_type':'at_log',
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
});
</script>
<?php
echo_contents($board,$port);
?>
<?php require("/www/cgi-bin/inc/boot.inc");?>
