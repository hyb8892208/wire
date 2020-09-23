<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/cdrdb.php");
include_once("/www/cgi-bin/inc/smsinboxdb.php");
include_once("/www/cgi-bin/inc/smsoutboxdb.php");
include_once("/www/cgi-bin/inc/userdb.php");

date_default_timezone_set('UTC');
session_start();
?>

<?php
////////////////////////////////
//define  verions compare result
$__CMP_SAME__ = 0;                 //verison same
$__CMP_DIFF__ = 1;		   //version different
$__VERSION_MISS__ = 2;		   //version info can't receive

//define system update mode
$__UPDATE_ALL_BOARD__ = 1;
$__UPDATE_SINGLE_BOARD__ = 0;

$__UPDATE_LOG_FILE__ = "/data/log/update.txt";
$__UNPACK_FILE__ = "/my_tools/unpack.sh";

$cur_cfg_version = trim(@file_get_contents('/etc/asterisk/cfg_version'));
$cur_sys_version = trim(@file_get_contents('/version/version'));
//$newest_sys_version = trim(@file_get_contents('http://downloads.openvox.cn/pub/firmwares/GSM%20Gateway/current-version'));
$cluster_info = get_cluster_info();

/////////////////////////////////////////
// read slave info from redis
$all_version_info = read_redis_info();

////////////////////////////////
//according ip to figureout whether same of versions
function version_to_num($version_str)
{
	$version_str = str_replace('.',"",$version_str);
	$version_str = (int)$version_str;
	return $version_str;
}

function is_same_version()
{
	global $__CMP_SAME__;
	global $__CMP_DIFF__;
	global $__VERSION_MISS__;
	global $all_version_info;
	
	$select_board_info = '';

	$master_version = $all_version_info['master']['local.product.sw.version'];
	$master_version = version_to_num($master_version);
	$info_flag = $__CMP_SAME__;
	$info_flag_miss = 0;
	foreach($all_version_info as $key => $info) {
		if ( $key != 'master' ) {
			if ($info['error'] == '') {
				$slave_version = $info['local.product.sw.version'];
				$slave_version = version_to_num($slave_version);
				if ($master_version > 222) {
					if ($slave_version < 222) {
						$info_flag = $__CMP_DIFF__;
					} 					
				} else if ($master_version == 222) {
					if ($slave_version < 222) {
						$info_flag = $__CMP_DIFF__;
					}
				}					
			} else {
				if ($info['error'] == 'disconnected') {
					$info_flag_miss = $__VERSION_MISS__;
				} else {
					$info_flag = $__CMP_DIFF__;
				}
			}
		}
	}
	
	$info_flag = $info_flag + $info_flag_miss;
	return $info_flag;
	
}

function make_update_file_path()
{
	$file_path="/data/update_file_".date("YmdHim");
	$tmp = $file_path;

	$i=0;
	while(file_exists($file_path)) {
		$i++;
		$file_path = $tmp."$i";
	}

	return $file_path;
}

function del_old_updatefile()
{
	exec("rm -rf /data/update_file_*");
}

function update_system_filter($all_version_info){
	$board_connect_status = '';
	foreach ($all_version_info as $board => $version_info) {
		$ip = $version_info['ip'];
		if ($version_info['error'] != '') {
			if ($version_info['error'] == 'disconnected') {
				$status = 1;
			} else {
				$status = 0;
			}
		} else {
			$status = 0;
		}
		$board_connect_status[$ip] = $status;
	}
	return $board_connect_status;
}

function update_system_online(){
	$store_file = make_update_file_path();
	
	$url="http://downloads.openvox.cn/pub/firmwares/Wireless_Gateway/SWG-gen2/swg-current.bin";
	$remote_fh = @fopen ($url, "rb");
	$filesize = -1;
	$headers = @get_headers($url, 1);
	if(is_array($headers)){
		if ((!array_key_exists("Content-Length", $headers))) {
			$filesize = -1;
		}else{
			$filesize = $headers["Content-Length"][1];
		}
	}
?>
	<div class="modal fade in" id="modal-51" >
		<div class="modal-dialog">
			<div class="modal-content">
				
				<div class="modal-header">
					<h4 class="modal-title"><?php echo language('System Update');?></h4>
				</div>
				
				<div class="modal-body" style="min-height:200px;">
					<table width="75%" style="font-size:12px;" align="center">
						<tr>
							<td align="center"><?php echo language('Downloading');?></td>
							<td width="30%"></td>
							<td rowspan=2 width="60px" valign="bottom">
								<input type="button" value="<?php echo language('Cancel');?>" onclick="location=location" />
							<td>
						</tr>
						<tr>
							<td style="border:1px solid rgb(59, 112, 162);">
								<div id="progress_bar" style="float:left;text-align:center;width:1px;color:#000000;background-color:rgb(208, 224, 238);"></div>
							</td>
							<td width="30px"><div id="progress_per">0%</div></td>
						</tr>
					</table>
				</div>
					
			</div>
		</div>
	</div>
	<div class="modal-backdrop fade_in"></div>
	
	<script>
	var filesize = "<?php echo $filesize; ?>";
	if(filesize != -1){
		$.ajax({
			url:"ajax_server.php?random="+Math.random()+"&type=dowmload_system_file&store_file=<?php echo $store_file;?>",
			type:'GET',
			success:function(data){
			},
			error:function(data){
				$(".modal-body").html("<?php echo language('System Online Update Download error','Download system file failed. Please check the network connection!')?>");
				$(".modal-body").append("<div style='text-align:center;margin-top:30px;'><button id='button_cancel' type='button' style='margin-left:10px;'><?php echo language('Close');?></button></div>");
			}
		});
		
		get_system_update_progress();
	}else{
		$(".modal-body").html("<?php echo language('System Online Update Download error','Download system file failed. Please check the network connection!')?>");
		$(".modal-body").append("<div style='text-align:center;margin-top:30px;'><button id='button_cancel' type='button' style='margin-left:10px;'><?php echo language('Close');?></button></div>");
	}
	
	$("#button_cancel").click(function(){
		$("#modal-51").hide();
		$(".fade_in").hide();
	});
	
	function get_system_update_progress(){
		$.ajax({
			url:"ajax_server.php?random="+Math.random()+"&type=get_system_update_progress&store_file=<?php echo $store_file;?>",
			type:'GET',
			success:function(data){
				var percent = Math.round(data*100/filesize);
				document.getElementById("progress_bar").style.width = (percent+"%");
				if(percent > 0){
					document.getElementById('progress_bar').innerHTML = data+"/"+filesize;
					document.getElementById('progress_per').innerHTML = percent+"%";
				}else{
					document.getElementById('progress_per').innerHTML = percent+"%";
				}
				
				if(parseInt(data) < parseInt(filesize)){
					setTimeout("get_system_update_progress()", 1000);
				}else{
					$(".modal-body").append("<div style='text-align:center;margin-top:15px;'><?php echo language('System Download Succeeded', 'The system is being upgaded, please wait.');?><img src='/images/mini_loading.gif' /></div>");
					
					$.ajax({
						url:"ajax_server.php?random="+Math.random()+"&type=system_update&store_file=<?php echo $store_file;?>",
						type:'GET',
						success:function(data){
							$(".modal-body").html(data);
							
							var button_cancel_len = $(".modal-body").find("#button_cancel").length;
							if(button_cancel_len == 1){
								settime(10);
							}
						},
						error:function(data){
						}
					});
				}
			},
			error:function(data){
			}
		});
	}
	
	function settime(update_time){
		update_time--;
		$("#update_time").text(update_time);
		var f = setTimeout(
			function(){
				settime(update_time);
			}
		,1000);
		
		$("#button_cancel").click(function(){
			$("#modal-51").hide();
			$(".fade_in").hide();
			clearTimeout(f);
		});
		
		if(update_time <= 0){
			$(".modal-body").html("<?php echo language('System Reboot wait','System Rebooting...<br>Please wait for about 60s, system will be rebooting.');?><br/><img src='/images/loading.gif' />");
			$.ajax({
				url:"ajax_server.php?random="+Math.random()+"&type=system_reboot",
				type:'GET',
				success:function(data){
				},
				error:function(data){
				}
			});
			
			setTimeout("reboot_refresh()", 21000);
			clearTimeout(f);
		}
	}
	
	function reboot_refresh(){
		$.ajax({
			type: "GET",
			cache: false,
			url: "../../index.html",
			data: "",
			success:function(){
				window.location.href='../../index.html';
			},
			error:function(){
				setTimeout("reboot_refresh()", 1000);
			}
		});
	}
	</script>
<?php
}

function firmware_link_update_system_online(){
	$store_file = make_update_file_path();
	
	$url=$_POST['firmware_link'];
	$remote_fh = @fopen ($url, "rb");
	$filesize = -1;
	$headers = @get_headers($url, 1);
	
	if(is_array($headers)){
		if ((!array_key_exists("Content-Length", $headers))) {
			$filesize = -1;
		}else{
			if(is_array($headers['Content-Length'])){
				$filesize = $headers["Content-Length"][1];
			}else{
				$filesize = $headers["Content-Length"];
			}
		}
	}
?>
	<div class="modal fade in" id="modal-51" >
		<div class="modal-dialog">
			<div class="modal-content">
				
				<div class="modal-header">
					<h4 class="modal-title"><?php echo language('System Update');?></h4>
				</div>
				
				<div class="modal-body" style="min-height:200px;">
					<table width="100%" style="font-size:12px;" align="center">
						<tr>
							<td align="center"><?php echo language('Downloading');?></td>
							<td width="10%"></td>
							<td rowspan=2 width="60px" valign="bottom">
								<input type="button" value="<?php echo language('Cancel');?>" onclick="location=location" />
							<td>
						</tr>
						<tr>
							<td style="border:1px solid rgb(59, 112, 162);">
								<div id="progress_bar" style="float:left;text-align:center;width:1px;color:#000000;background-color:rgb(208, 224, 238);"></div>
							</td>
							<td width="30px"><div id="progress_per">0%</div></td>
						</tr>
					</table>
				</div>
					
			</div>
		</div>
	</div>
	<div class="modal-backdrop fade_in"></div>
	
	<script>
	var filesize = "<?php echo $filesize; ?>";
	if(filesize != -1){
		$.ajax({
			url:"ajax_server_noauth.php?random="+Math.random()+"&type=dowmload_link_file&store_file=<?php echo $store_file;?>",
			type:'POST',
			data:{
				url:'<?php echo $url;?>'
			},
			success:function(data){
			},
			error:function(data){
				$(".modal-body").html("<?php echo language('System Online Update Download error','Download system file failed. Please check the network connection!')?>");
				$(".modal-body").append("<div style='text-align:center;margin-top:30px;'><button id='button_cancel' type='button' style='margin-left:10px;'><?php echo language('Close');?></button></div>");
			}
		});
		
		get_system_update_progress();
	}else{
		$(".modal-body").html("<?php echo language('System Online Update Download error','Download system file failed. Please check the network connection!')?>");
		$(".modal-body").append("<div style='text-align:center;margin-top:30px;'><button id='button_cancel' type='button' style='margin-left:10px;'><?php echo language('Close');?></button></div>");
	}
	
	$(document).on('click','#button_cancel',function(){
		$("#modal-51").hide();
		$(".fade_in").hide();
	});
	
	function get_system_update_progress(){
		$.ajax({
			url:"ajax_server_noauth.php?random="+Math.random()+"&type=get_system_update_progress&store_file=<?php echo $store_file;?>",
			type:'GET',
			success:function(data){
				var percent = Math.round(data*100/filesize);
				document.getElementById("progress_bar").style.width = (percent+"%");
				if(percent > 0){
					document.getElementById('progress_bar').innerHTML = data+"/"+filesize;
					document.getElementById('progress_per').innerHTML = percent+"%";
				}else{
					document.getElementById('progress_per').innerHTML = percent+"%";
				}
				
				if(parseInt(data) < parseInt(filesize)){
					setTimeout("get_system_update_progress()", 1000);
				}else{
					$(".modal-body").append("<div style='text-align:center;margin-top:15px;'><?php echo language('System Download Succeeded', 'The system is being upgaded, please wait.');?><img src='/images/mini_loading.gif' /></div>");
					
					$.ajax({
						url:"ajax_server.php?random="+Math.random()+"&type=system_update&store_file=<?php echo $store_file;?>",
						type:'GET',
						success:function(data){
							$(".modal-body").html(data);
							
							var button_cancel_len = $(".modal-body").find("#button_cancel").length;
							if(button_cancel_len == 1){
								settime(10);
							}
						},
						error:function(data){
						}
					});
				}
			},
			error:function(data){
			}
		});
	}
	
	function settime(update_time){
		update_time--;
		$("#update_time").text(update_time);
		var f = setTimeout(
			function(){
				settime(update_time);
			}
		,1000);
		
		$("#button_cancel").click(function(){
			$("#modal-51").hide();
			$(".fade_in").hide();
			clearTimeout(f);
		});
		
		if(update_time <= 0){
			$(".modal-body").html("<?php echo language('System Reboot wait','System Rebooting...<br>Please wait for about 60s, system will be rebooting.');?><br/><img src='/images/loading.gif' />");
			$.ajax({
				url:"ajax_server.php?random="+Math.random()+"&type=system_reboot",
				type:'GET',
				success:function(data){
				},
				error:function(data){
				}
			});
			
			setTimeout("reboot_refresh()", 21000);
			clearTimeout(f);
		}
	}
	
	function reboot_refresh(){
		$.ajax({
			type: "GET",
			cache: false,
			url: "../../index.html",
			data: "",
			success:function(){
				window.location.href='../../index.html';
			},
			error:function(){
				setTimeout("reboot_refresh()", 1000);
			}
		});
	}
	</script>
<?php
}

function file_link_online_update(){
	$store_file = make_update_file_path();
	
	$url=$_POST['file_link'];
	$remote_fh = @fopen ($url, "rb");
	$filesize = -1;
	$headers = @get_headers($url, 1);
	if(is_array($headers)){
		if ((!array_key_exists("Content-Length", $headers))) {
			$filesize = -1;
		}else{
			if(is_array($headers['Content-Length'])){
				$filesize = $headers["Content-Length"][1];
			}else{
				$filesize = $headers["Content-Length"];
			}
		}
	}
?>
	<div class="modal fade in" id="modal-51" >
		<div class="modal-dialog">
			<div class="modal-content">
				
				<div class="modal-header">
					<h4 class="modal-title"><?php echo language('System Update');?></h4>
				</div>
				
				<div class="modal-body" style="min-height:200px;">
					<table width="100%" style="font-size:12px;" align="center">
						<tr>
							<td align="center"><?php echo language('Downloading');?></td>
							<td width="10%"></td>
							<td rowspan=2 width="60px" valign="bottom">
								<input type="button" value="<?php echo language('Cancel');?>" onclick="location=location" />
							<td>
						</tr>
						<tr>
							<td style="border:1px solid rgb(59, 112, 162);">
								<div id="progress_bar" style="float:left;text-align:center;width:1px;color:#000000;background-color:rgb(208, 224, 238);"></div>
							</td>
							<td width="30px"><div id="progress_per">0%</div></td>
						</tr>
					</table>
				</div>
					
			</div>
		</div>
	</div>
	<div class="modal-backdrop fade_in"></div>
	
	<script>
	var filesize = "<?php echo $filesize; ?>";
	if(filesize != -1){
		$.ajax({
			url:"ajax_server_noauth.php?random="+Math.random()+"&type=dowmload_link_file&store_file=<?php echo $store_file;?>",
			type:'POST',
			data:{
				url:'<?php echo $url;?>'
			},
			success:function(data){
			},
			error:function(data){
				$(".modal-body").html("<?php echo language('System Online Update Download error','Download system file failed. Please check the network connection!')?>");
				$(".modal-body").append("<div style='text-align:center;margin-top:30px;'><button id='button_cancel' type='button' style='margin-left:10px;'><?php echo language('Close');?></button></div>");
			}
		});
		
		get_system_update_progress();
	}else{
		$(".modal-body").html("<?php echo language('System Online Update Download error','Download system file failed. Please check the network connection!')?>");
		$(".modal-body").append("<div style='text-align:center;margin-top:30px;'><button id='button_cancel' type='button' style='margin-left:10px;'><?php echo language('Close');?></button></div>");
	}
	
	$(document).on('click','#button_cancel',function(){
		$("#modal-51").hide();
		$(".fade_in").hide();
	});
	
	function get_system_update_progress(){
		$.ajax({
			url:"ajax_server_noauth.php?random="+Math.random()+"&type=get_system_update_progress&store_file=<?php echo $store_file;?>",
			type:'POST',
			data:{
				url: "<?php echo $url; ?>"
			},
			success:function(data){
				var percent = Math.round(data*100/filesize);
				document.getElementById("progress_bar").style.width = (percent+"%");
				if(percent > 0){
					document.getElementById('progress_bar').innerHTML = data+"/"+filesize;
					document.getElementById('progress_per').innerHTML = percent+"%";
				}else{
					document.getElementById('progress_per').innerHTML = percent+"%";
				}
				
				if(parseInt(data) < parseInt(filesize)){
					setTimeout("get_system_update_progress()", 1000);
				}else{
					$.ajax({
						url:"ajax_server_noauth.php?random="+Math.random()+"&type=update_cfg_file&store_file=<?php echo $store_file;?>",
						type:'POST',
						data: {
							url: "<?php echo $url;?>"
						},
						success:function(data){
							if(data == 'success'){
								$(".modal-body").html("<?php echo language('Configuration Files Update Succeeded')?>");
							}else{
								$(".modal-body").html("<?php echo language('Configuration Files Update Failed')?>");
								$(".modal-body").append(data);
							}
							$(".modal-body").append("<div style='text-align:center;margin-top:30px;'><button id='button_cancel' type='button' style='margin-left:10px;'><?php echo language('Close');?></button></div>");
						},
						error:function(data){
						}
					});
				}
			},
			error:function(data){
			}
		});
	}
	</script>
<?php
}

function upload_cfg_file()
{
	if(! $_FILES) {
		return;
	}

	echo "<br>";
	$Report = language('Report');
	$Result = language('Result');
	$theme = language("Configuration Files Upload");
	trace_output_start("$Report", "$theme");
	trace_output_newline();
	if(isset($_FILES['upload_cfg_file']['error']) && $_FILES['upload_cfg_file']['error'] == 0) {  //Update successful
		if(!(isset($_FILES['upload_cfg_file']['size'])) || $_FILES['upload_cfg_file']['size'] > 80*1000*1000) { //Max file size 80Mbyte
			echo language('Configuration Files Upload Filesize error',"Your uploaded file was larger than 80M!<br>Uploading configuration files was failed.");
			return;
		}

		$store_file = make_update_file_path();

		if (!move_uploaded_file($_FILES['upload_cfg_file']['tmp_name'], $store_file)) {  
			echo language('Configuration Files Upload Move error',"Moving your updated file was failed!<br>Uploading configuration files was failed.");  
			return;
		}
		echo language("Configuration Files Uploading");echo " ......<br>";
		ob_flush();
		flush();

		exec("rm -rf /tmp/update_conf_file");
		exec("mkdir /tmp/update_conf_file");
		
		if(strstr($_FILES['upload_cfg_file']['name'], 'tar.gz') || strstr($_FILES['upload_cfg_file']['name'], '.bin')){
			$cmd = "tar vxz -f $store_file -C /tmp/update_conf_file"; 
		}else if(strstr($_FILES['upload_cfg_file']['name'], 'zip')){
			$cmd = "unzip $store_file -d /tmp/update_conf_file";
		}else{
			$cmd = "tar vxf $store_file -C /tmp/update_conf_file";
		}
		exec("$cmd > /dev/null 2>&1 || echo $?",$output);
		
		exec("find /dev/shm/update_conf_file/ -name cfg_version", $output_version);
		$cfg_version = $output_version[0];
		
		if(file_exists($cfg_version)){
			$version=file_get_contents($cfg_version);
			if($version == "") {
				$error_reports = language("Configuration Files Upload version error 2","</br>Unknown configuration version!");
				file_put_contents('/tmp/schedule_conf.log',$error_reports,FILE_APPEND);
				
				exec("rm -rf /tmp/update_conf_file");
				exec("rm $store_file -rf > /dev/null 2>&1");
				echo $error_reports;
				trace_output_end();
				return false;
			}

			$cmd = "( cat /version/cfg_ver_list 2> /dev/null | grep \"$version\" > /dev/null 2>&1 )";
			exec("$cmd || echo $?",$output);
			if($output) {
				$error_reports = language("Configuration Files Upload version error 3","</br>Your current system does not support this configuration version!</br>");
				file_put_contents('/tmp/schedule_conf.log',$error_reports,FILE_APPEND);
				
				exec("rm -rf /tmp/update_conf_file");
				exec("rm $store_file -rf > /dev/null 2>&1");
				echo $error_reports;
				trace_output_end();
				return false;
			}
		}else if(strstr($_FILES['upload_cfg_file']['name'],'tar') || strstr($_FILES['upload_cfg_file']['name'],'zip')){
			exec("find /dev/shm/update_conf_file/", $output_sourcefiles);
			exec("/etc/init.d/asterisk stop > /dev/null 2>&1");
			for($i=0;$i<count($output_sourcefiles);$i++){
				if(strstr($output_sourcefiles[$i],'.')){
					$temp = explode('/',$output_sourcefiles[$i]);
					
					$length = count($temp);
					$file_name = $temp[$length-1];
					
					unset($output_destination_file);
					exec("find /etc/asterisk/ -name $file_name",$output_destination_file);
					
					if($output_destination_file[0]){
						copy($output_sourcefiles[$i],$output_destination_file[0]);
					}
				}
			}
			exec("/etc/init.d/asterisk start > /dev/null 2>&1");
			
			exec("rm -rf /tmp/update_conf_file");
			exec("rm $store_file -rf > /dev/null 2>&1");
			trace_output_end();
			return false;
		}else{
			$error_reports = language("Configuration Files Upload version error 1","</br>Can not find configration file!</br>Can not find asterisk/cfg_version!<br>");
			file_put_contents('/tmp/schedule_conf.log',$error_reports,FILE_APPEND);
			
			exec("rm -rf /tmp/update_conf_file");
			exec("rm $store_file -rf > /dev/null 2>&1");
			echo $error_reports;
			trace_output_end();
			return false;
		}

		echo language('Stop Asterisk',"Stopping asterisk....<br>");
		ob_flush();
		flush();
		exec("/etc/init.d/asterisk stop > /dev/null 2>&1");
		
		echo language('Configuration Files Upload update',"Updating configuration files....<br>");
		ob_flush();
		flush();
		exec("rm -rf /etc/cfg/*");
		exec("rm -rf /etc/asterisk/*");
		
		exec("find /dev/shm/update_conf_file/ -name cfg",$output_cfg);
		$cfg_path = $output_cfg[0];
		if($cfg_path == ''){
			$cfg_path = '/dev/shm/update_conf_file/';
		}
		exec("cp $cfg_path/* /etc/cfg/ -a");
		exec("cp /etc/cfg/* /etc/asterisk/ -a");
		
		echo language('Start Asterisk',"Starting asterisk....<br>");
		ob_flush();
		flush();
		exec("/etc/init.d/asterisk start > /dev/null 2>&1");

		save_user_record("","System->File Upload Update Configuration");

		exec("rm -rf /tmp/update_conf_file");
		exec("rm $store_file -rf > /dev/null 2>&1");
		
		trace_output_newhead("$Result");
		echo language("Configuration Files Upload Succeeded");
		trace_output_end();
		
	} else {
		if(isset($_FILES['upload_cfg_file']['error'])) {
			switch($_FILES['upload_cfg_file']['error']) {
			case 1:    
				echo language('Configuration Files Upload error 1',"The file was larger than the server space 80M!");
				break;
			case 2:    
				echo language('Configuration Files Upload error 2',"The file was larger than the browser's limit!");
				break;
			case 3:
				echo language('Configuration Files Upload error 3',"The file was only partially uploaded!");
				break;
			case 4: 
				echo language('Configuration Files Upload error 4',"Can not find uploaded file!");
				break;
			case 5: 
				echo language('Configuration Files Upload error 5',"The server temporarily lost folder!");    
				break;
			case 6: 
				echo language('Configuration Files Upload error 6',"Failed to write to the temporary folder!");    
				break;    
			}
		}
		echo "<br>";
		trace_output_newhead("$Result");
		echo language("Configuration Files Upload Failed");
	}
	trace_output_end();
}

function backup_cfg_file()
{
	//Current config version
	global $cur_cfg_version;

	save_user_record("","SYSTEM->Tools:Download Backup Conf");

	//configuration file name
	$cfg_name="config-$cur_cfg_version.bin";

	//configuration file path
	$cfg_path="/tmp/$cfg_name";

	//pack config file
	$pack_cmd="tar vcz -f $cfg_path -C /etc ./cfg/";
	//echo $pack_cmd;

	exec("$pack_cmd > /dev/null 2>&1 || echo $?",$output);
	if($output) {
		echo "</br>$cfg_name ";
		echo language("Packing was failed");echo "</br>";
		echo language("Error code");echo ": ".$output[0];
		return;
	}

	if(!file_exists($cfg_path)) {
		echo "</br>$cfg_name";
		echo language("Can not find");
		return;
	}

	//���ļ�  
	$file = fopen ($cfg_path, "r" ); 
	$size = filesize($cfg_path) ;

	//�����ļ���ǩ 
	header('Content-Encoding: none');
	header('Content-Type: application/force-download');
	header('Content-Type: application/octet-stream');
	header('Content-Type: application/download');
	header('Content-Description: File Transfer');  
	header('Accept-Ranges: bytes');  
	header( "Accept-Length: $size");  
	header( 'Content-Transfer-Encoding: binary' );
	header( "Content-Disposition: attachment; filename=$cfg_name" ); 
	header('Pragma: no-cache');
	header('Expires: 0');
	//����ļ�����   
	//��ȡ�ļ����ݲ�ֱ������������
	ob_clean();
	flush();
	echo fread($file, $size);
	fclose ($file);

	unlink($cfg_path);
}

$g_restore_cfg_file = false;

function flush_cdr_sms()
{
	$cdr_db = new CDRDB();                    
	$sms_in_db = new SMSINBOXDB();
	$sms_out_db = new SMSOUTBOXDB();

    if(!$cdr_db || !$sms_in_db || !$sms_out_db) 
	{
        require("/www/cgi-bin/inc/boot.inc");
        exit(0);
    }   
	$cdr_log_file = '/data/log/cdr.db';
	$sms_in_file = '/data/log/smsinbox.db';
	$sms_out_file = '/data/log/smsoutbox.db';

	$hlock = lock_file($cdr_log_file);
	$cdr_db->try_query("delete from cdr");    
	$cdr_db->try_query("delete from grppolicy");
	unlock_file($hlock);

	$hlock = lock_file($sms_in_file);
	$sms_in_db->try_query("delete from sms");
	unlock_file($hlock);

	$hlock = lock_file($sms_out_file);
	$sms_out_db->try_query("delete from sms_out");
	unlock_file($hlock);
	
}


function res_def_cfg_file()
{
	global $cluster_info;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;

	flush_cdr_sms();   
	$default_cfg_Restore = language('Configuration Restore wait',"Default Configuration Files Restoring...<br>Please wait for about 60s, system will be rebooting.");
	js_reboot_progress("$default_cfg_Restore");

	echo "<br>";
	$Report = language('Report');
	$Configuration_Restore = language('Configuration Restore');
	trace_output_start("$Report","$Configuration_Restore");
	trace_output_newline();
	
	save_user_record("","SYSTEM->Tools:Factory Reset");
	$userdb = new Users();
	$userdb->factory_reset_userdb();
	
	exec("/my_tools/add_syslog \"Factory reset from Web.\"");
	exec("/my_tools/restore_cfg_file > /dev/null 2>&1 || echo $?",$output);
	echo language("Configuration Restoring");echo " ......<br/>";
	if($output) {
		echo language("Configuration Restore Failed");
		echo language("Error code");echo ": ".$output[0];
		flush();
		ob_flush();
	}
	trace_output_end();

	global $g_restore_cfg_file;
	$g_restore_cfg_file = true;
}

function system_reboot()
{
	global $cluster_info;
	global $__BRD_HEAD__;
	global $__BRD_SUM__;

	$System_Reboot_help = language('System Reboot wait','System Rebooting...<br>Please wait for about 60s, system will be rebooting.');
	js_reboot_progress("$System_Reboot_help");
	
	echo "<br>";
	$Report = language('Report');
	$System_Reboot = language('System Reboot');
	trace_output_start("$Report","$System_Reboot");
	trace_output_newline();
	
	if($cluster_info['mode'] == 'master') {
		
	}else{
		echo language("System Rebooting");echo "......";
	}
	trace_output_end();
	
	save_user_record("","SYSTEM->Tools:System Reboot");
	
	exec("sleep 3");
	exec("/sbin/hwclock -w > /dev/null 2>&1");
	exec("reboot -f > /dev/null 2>&1");
}

function system_switch(){
?>
	<div class="modal fade in" id="modal-51" >
		<div class="modal-dialog">
			<div class="modal-content">
			
				<div class="modal-header">
					<h4 class="modal-title"><?php echo language('System Switch');?></h4>
				</div>
				
				<div class="modal-body" style="min-height:200px;">
				<?php 
					echo language('System Switching').'......<br/>';
					exec("/my_tools/switch_sys.sh > /dev/null 2>&1 || echo $?",$output);
					$success_flag = 0;
					if(!$output){
						$success_flag = 1;
						
						save_user_record("","SYSTEM->Tools:System Switch Succeeded");
					}else{
						echo language("System Switch Failed");echo "($output[0])";
						echo '<div style="text-align:center;margin-top:30px;"><button id="button_cancel" type="button" style="margin-left:10px;">'.language('Close').'</button></div>';
						
						save_user_record("","SYSTEM->Tools:System Switch Failed");
					}
				?>
				</div>
			
			</div>
		</div>
	</div>
	<div class="modal-backdrop fade_in"></div>
	
	<script>
	$("#button_cancel").click(function(){
		$("#modal-51").hide();
		$(".fade_in").hide();
	});
	
	<?php 
	if($success_flag == 1){?>
		$(".modal-body").html("<?php echo language("System Switch Succeeded");?><br/>");
		$(".modal-body").append("<?php echo language('System Update Succeeded help',"You must reboot system to entry the newer system.")."<br/>";?>");
		$(".modal-body").append("<div style='text-align:center;margin-top:30px;'><?php echo language('System Count Dowm','The system will be restarted automatically after 10 seconds countdown');?>: &nbsp<span id='update_time'>10</span><?php echo language('Second');?><button id='button_cancel' type='button' style='margin-left:10px;'><?php echo language('Cancel');?></button></div>");
		settime(10);
		
	
		function settime(update_time){
			update_time--;
			$("#update_time").text(update_time);
			var f = setTimeout(
				function(){
					settime(update_time);
				}
			,1000);
			
			$("#button_cancel").click(function(){
				$("#modal-51").hide();
				$(".fade_in").hide();
				clearTimeout(f);
			});
			
			if(update_time <= 0){
				$(".modal-body").html("<?php echo language('System Reboot wait','System Rebooting...<br>Please wait for about 60s, system will be rebooting.');?><br/><img src='/images/loading.gif' />");
				$.ajax({
					url:"ajax_server.php?random="+Math.random()+"&type=system_reboot",
					type:'GET',
					success:function(data){
					},
					error:function(data){
					}
				});
				
				setTimeout("reboot_refresh()", 21000);
				clearTimeout(f);
			}
		}
		
		function reboot_refresh(){
			$.ajax({
				type: "GET",
				cache: false,
				url: "../../index.html",
				data: "",
				success:function(){
					window.location.href='../../index.html';
				},
				error:function(){
					setTimeout("reboot_refresh()", 1000);
				}
			});
		}
		<?php } ?>
	</script>
<?php
}

function voice_stop(){
	$port = $_POST['port'];
	exec("/my_tools/rri_cli chn_snd debug $port 0");
	
	$grounp_num = intval($port/2-0.5);
	
	$pcm_r = "$grounp_num-$port-r.pcm";
	$pcm_w = "$grounp_num-$port-w.pcm";
	if(!file_exists('/tmp/module_pipe/'.$pcm_r)){
		$pcm_r = "opvx_chan_unkown-$port-r.pcm";
	}
	if(!file_exists('/tmp/module_pipe/'.$pcm_w)){
		$pcm_w = "opvx_chan_unkown-$port-w.pcm";
	}
	
	exec("date +\"%Y-%m-%d-%H.%M.%S\"",$time_output);
	$time = $time_output[0];
	
	$download_filename = "channel$port-$time.tar.gz";
	$download_file="/tmp/module_pipe/channel$port-$time.tar.gz";
	
	$pack_cmd="tar -C /tmp/module_pipe -cvf $download_file $pcm_r $pcm_w";
	
	exec("$pack_cmd > /dev/null 2>&1 || echo $?",$output);
	if($output){
		echo "</br>$download_file ";
		echo language("Packing was failed");echo "</br>";
		echo language("Error code");echo ": ".$output[0];
		return;
	}

	if(!file_exists($download_file)) {
		echo "</br>$download_file";
		echo language("Can not find");
		return;
	}

	$file = fopen ($download_file, "r" );
	$size = filesize($download_file);

	header('Content-Encoding: none');
	header('Content-Type: application/force-download');
	header('Content-Type: application/octet-stream');
	header('Content-Type: application/download');
	header('Content-Description: File Transfer');
	header('Accept-Ranges: bytes');
	header("Accept-Length: $size");
	header('Content-Transfer-Encoding: binary');
	header("Content-Disposition: attachment; filename=$download_filename");
	header('Pragma: no-cache');
	header('Expires: 0');
	ob_clean();
	flush();
	echo fread($file, $size);
	fclose ($file);

	exec("> /tmp/module_pipe/$pcm_r");
	exec("> /tmp/module_pipe/$pcm_w");
	unlink($download_file);
}

function lisence_upload(){
	echo "<br/>";
	$Report = language('Report');
	$Result = language('Result');
	$Upload_Lisence = language('License Upload');
	trace_output_start($Report, $Upload_Lisence);
	trace_output_newline();
	if(isset($_FILES['upload_lisence_file']['error']) && $_FILES['upload_lisence_file']['error'] == 0){
		if(!(isset($_FILES['upload_lisence_file']['size'])) || $_FILES['upload_lisence_file']['size'] > 80*1000*1000){
			echo language('License Upload Filesize error',"Your uploaded file was larger than 80M!<br>Upload license was failed.");
			trace_output_end();
			return;
		}
		
		if(!move_uploaded_file($_FILES['upload_lisence_file']['tmp_name'], '/data/crt/'.$_FILES['upload_lisence_file']['name'])){
			echo language('License Upload Move error',"Moving your uploaded file was failed!<br/>");
			trace_output_end();
			return;
		}
		echo language("License Uploading");echo "......<br>\n";
		ob_flush();
		flush();
		
		exec("/etc/init.d/license.sh start >/dev/null 2>&1 &");
		
		$redis_client = new Predis\Client();
		$redis_client->lpush("app.license.flag","1");
		
		trace_output_newhead($Result);
		echo language("SUCCESS");
	}else{
		echo "<br/>";
		trace_output_newhead($Result);
		echo language("License Upload Failed");
	}
	trace_output_end();
}

function voice_start(){
	$port = $_POST['port'];
	exec("/my_tools/rri_cli chn_snd debug $port 1");
?>
<script>
	var hour=0,minute=0,second=0;
	var minutes = '00';
	var seconds = '00';
	var millisecond=0;
	var interval;
	function preview_dialog(){
		click();
		$("#preview_dg").dialog({
			resizable: false,
			height:400,
			width:500,
			modal: true,

			buttons: [
				{
					text:"<?php echo language('Stop Recording');?>",
					id:"close",
					style:"text-align: center",
					click:function(){
						document.getElementById('send').value = 'Voice Stop';
						$("#manform").submit();
						$(this).dialog( "close" );
					}
				}
			]		
		});
		$(".ui-button").click(function(){
			hour=minute=second=0;
			minutes = '00';
			seconds = '00';
			millisecond=0;
			window.clearInterval(interval);
			document.getElementById('send').value = 'Voice Stop';
			$("#manform").submit();
		});
	}
	
	function click(){
		minutes = '00', seconds = '00';
		var str = "<p style='text-align:center'><font id='time_run' color='#00ff33' size= '6px' style='font-weight:bold'>" + minutes +":"+ seconds + "</font></p>";
			str += "<br><font color='green' size='5px'><?php echo language('Voice Stop help');?></font>";
		$("#time").html(str);
		interval = setInterval(function() {
			millisecond=millisecond+1000;
			if(millisecond>=1000)
			{
				millisecond=0;
				second=second+1;
			}
			if(second>=60){
				second=0;
				minute=minute+1;
			}
			if(minute>=60){ 
				minute=0;
			}
			if(minute<10)minutes='0'+minute;
			else minutes=minute;
			if(second<10)seconds='0'+second;
			else seconds=second;
			if(minute>=30){
				clearInterval(interval);
			}
			$("#time_run").html(minutes +":"+ seconds);
			if(minute == 3){
				$("#preview_dg").dialog("close");
				window.clearInterval(interval);
				document.getElementById('send').value = 'Voice Stop';
				$("#manform").submit();
			}
		}, 1000);
	}
	
	preview_dialog();
</script>
<?php
}
?>

<script type="text/javascript">
var g_bAllowFile = false;

function checkFileChange(obj)
{
	var filesize = 0;  
	var  Sys = {};  

	if(navigator.userAgent.indexOf("MSIE")>0){
		Sys.ie=true;  
	} else
	//if(isFirefox=navigator.userAgent.indexOf("Firefox")>0)  
	{  
		Sys.firefox=true;  
	}
	   
	if(Sys.firefox){  
		//filesize = obj.files[0].fileSize;  
		filesize = obj.files[0].size;  
	} else if(Sys.ie){
		try {
			obj.select();
			var realpath = document.selection.createRange().text;
			//alert(obj.value);
			//alert(realpath);
			var fileobject = new ActiveXObject ("Scripting.FileSystemObject");//��ȡ�ϴ��ļ��Ķ���  
			//var file = fileobject.GetFile (obj.value);//��ȡ�ϴ����ļ�  
			var file = fileobject.GetFile (realpath);//��ȡ�ϴ����ļ�  
			var filesize = file.Size;//�ļ���С  
		} catch(e){
			alert("<?php echo language('System Update IE alert','Please allow ActiveX Scripting File System Object!');?>");
			return false;
		}
	}

	if(filesize > 1000*1000*80) {
		alert("<?php echo language('System Update filesize alert','Uploaded max file is 80M!');?>");
		g_bAllowFile = false;
		return false;
	}

	g_bAllowFile = true;
	return true;
} 

function checkLisenceFile(obj){
	if(confirm("<?php echo language("Upload License help", "Are you sure you want to upload license?");?>")){
		return true;
	}else{
		return false;
	}
}

function check_upload_lisence(){
	var file = document.getElementById('upload_lisence_file');
	var file_name = file.files[0].name;
	
	if(file_name.indexOf('.crt') == -1){
		alert("<?php echo language("Upload License file help", "Please upload the correct License file!");?>");
		$("#upload_lisence_file").val("");
		return false;
	}
}

function check_file_type(file_id)
{
	var x = document.getElementById(file_id).value;
	var result = x.match(/.+\.(\w+)/);
	return result[1];
}
function isAllowFile(file_id)
{
	var x = document.getElementById(file_id).value;
	if(x=="")
	{
		alert("<?php echo language('Select File alert','Please select your file first!');?>");
		return false;
	}
	//return true;

	if(g_bAllowFile)
		return true;

	alert("<?php echo language('System Update filesize alert','Uploaded max file is 80M!');?>");
	return false;
}

function update_system()
{		
<?php if($license_mode == 'on'){ ?>
	//check license version no less than 2.1.2
	var obj = document.getElementById('update_sys_file');
	var filename = obj.files[0].name;
	var version_temp = filename.split('-');
	if(version_temp.length-1 == 8){
		var version = version_temp[2];
	}else if(version_temp.length-1 == 7){
		var version = version_temp[1];
	}else{
		alert("<?php echo language("Firmware upload help","The format of the uploaded file is incorrect.");?>");
		return false;
	}
	var temp = version.split('.');
	var num0 = parseInt(temp[0]);
	var num1 = parseInt(temp[1]);
	var num2 = parseInt(temp[2]);
	if(isNaN(num0) || isNaN(num1) || isNaN(num2)){
		alert("<?php echo language("Firmware upload help","The format of the uploaded file is incorrect.");?>");
		return false;
	}
	if(num0 < 2 || num1 < 1 || num2 < 2){
		alert("<?php echo language("Firmware version help","Firmware version should not be less than 2.1.2"); ?>");
		return false;
	}
<?php } ?>
	
	//check select which board : all or single or board disconnected
	var sel = document.getElementById('select_update');
	if (sel != null) {			
		if ( sel.value == 'select_none' ) {       //not select board
			alert("Please select a board to update.");
			return false;
		} else if (sel.value == 'error') {		  //select board but it can not connect.
			alert("The board you selected can't update. Please make sure the board connection.");			
			return false;
		} else if (sel.value != 'select_all') {    //not select all ,rest is selected single board
			if(!isAllowFile('update_sys_file')) {
				return false;
			}

			if(!confirm("<?php echo language('System Update confirm','Are you sure to update system?\\nCaution: this might damage your configuration files.');?>")){
				return false;
			}
			return true;
		}	
	} else if (sel == null) {   // stand_alone or slave , can not do "select" operation
		if(!isAllowFile('update_sys_file')) {
			return false;
		}

		if(!confirm("<?php echo language('System Update confirm','Are you sure to update system?\\nCaution: this might damage your configuration files.');?>")){
			return false;
		}
		return true;
	}
	
	//select all board 
	var version_flag = 0;
	var version_table_obj = document.getElementById('version_table');

	<?php 
		$is_same = is_same_version();
	?>
	var version_flag = "<?php echo $is_same;?>";
	if (version_flag != 0 ) {
		version_status = version_flag % 2;
		if (version_status == 1 ) {
			if(!isAllowFile('update_sys_file')) {
				return false;
			}
			var file_type = check_file_type('update_sys_file');
			if (file_type == 'bin') {
				version_table_obj.style.display = "";
				if(!confirm("<?php echo language('Jump download confirm','Warning: version of slave boards is too low. Please make sure version of all boards is 2.2.2 or higher!!\nConfirm: jump to download page to get 2.2.2;\nCancel: back to original page.');?>")){
					return false;
				} else {
					//window.location.href="/cgi-bin/php/system-info.php";
					window.open("http://downloads.openvox.cn/pub/firmwares/GSM%20Gateway/wg400-mid.img");
					return false;
				}				
			} else {
				//use ".img" , back to low version
				version_table_obj.style.display = "";
				if (version_flag == version_status) {
					if(!confirm("<?php echo language('System Update confirm','Are you sure to update system?\\nCaution: this might damage your configuration files.');?>")){
						return false;
					} else {
						return true;
					}
				} else {
					if(!confirm("<?php echo language('Version confirm','Are you sure to update system?\\nCaution: this might damage your configuration files and disconnected boards could not be updated.');?>")){
						return false;
					} else {
						return true;
					}	
				}
			}
		} else if (version_status == 0) {
			version_table_obj.style.display = "";
			if(!isAllowFile('update_sys_file')) {
				return false;
			}			
			if (!confirm("<?php echo language('Version confirm','Are you sure to update system?\\nCaution: this might damage your configuration files and disconnected boards could not be updated.');?>")) {
				return false;
			} else {
				return true;
			}
		}
	}
	
	if(!isAllowFile('update_sys_file')) {
		return false;
	}

	if(!confirm("<?php echo language('System Update confirm','Are you sure to update system?\\nCaution: this might damage your configuration files.');?>")){
		return false;
	}

	return true;
}

function update_system_online_step1()
{
	//document.getElementById('showmsg').value = 'Getting information...';
	var str = "";
	$( "#update_online_dg" ).dialog({
		resizable: false,
		height:400,
		width:500,
		modal: true,

		buttons: [
			{
				text:"<?php echo language('Change Log');?>",
				id:"change_log",
				click:function(){
					update_system_online_step2();
				}
			},
			{
				text:"<?php echo language('Detailed');?>",
				id:"detailed",
				click:function(){
					update_system_online_step3();
				}
			},
			{
				text:"<?php echo language('Update Online Now');?>",
				id:"button_online_now",
				click:function(){
					$( this ).dialog( "close" );
					document.getElementById('send').value='System Online Update';
					document.getElementById('manform').submit();
				}
			},
			{
				text:"<?php echo language('Cancel');?>",
				id:"cancel",
				click:function(){
					$( this ).dialog( "close" );
				}
			}
		]
	});

	var server_file = "./../../cgi-bin/php/ajax_server.php";
	$.ajax({
		url: server_file+"?random="+Math.random()+"&type=system&system_type=newest_sys_version",	
		async: false,
		dataType: 'text',
		type: 'GET',
		timeout: 5000,
		error: function(data){				//request failed callback function;
			//document.getElementById('showmsg').value = "<?php echo language('System Online Update version error1','Get remote version failed. Please check the network connection!');?>";
			str = "";
			str += "<br><font color='black' > <?php echo language('System Online Update version error1','Get remote version failed. Please check the network connection!');?></font>";
			str += "<br><br><font color='red' size= '3px'><?php echo language('Error');?>: </font> <font color='green'><?php echo language('Get remote version failed', 'Get remote version failed.');?></font>";
			
		
			document.getElementById("redmsg").innerHTML = str;
			
		},
		success: function(data){			//request success callback function;
			var versionnum = data;
			if(versionnum == ''){
				//document.getElementById('showmsg').value = "<?php echo language('System Online Update version error1','Get remote version failed. Please check the network connection!');?>";
				str = "";
				str += "<br><font color='black' > <?php echo language('System Online Update version error1','Get remote version failed. Please check the network connection!');?></font>";
				str += "<br><br><font color='red' size= '3px'><?php echo language('Error');?>: </font> <font color='green'><?php echo language('Get remote version failed', 'Get remote version failed.');?></font>";
				document.getElementById("redmsg").innerHTML = str;
				return;
			} else {
				//document.getElementById('showmsg').value = "<?php echo language('System version help 1','Your current system version is :'); echo $cur_sys_version;?>"+"\n"+"<?php echo language('System version help 2','The latest system version is :'); ?>" + versionnum + ".\n<?php echo language('System Online Update confirm',"Be cautious, please:\\nThis might damage the structure of your original configuration files! \\nAre you sure to update your system?\\n");?>" + "<?php echo language('System version help 1','Your current system version is :'); echo $cur_sys_version;?>"+"\n\n"+"<?php echo language('System version help 3','Warning:\\nDO NOT leave this page in the process of updating; OTHERWISE system updating will fail!\\n'); ?>";
				
				//add version check
				<?php
				$is_same = is_same_version();
				?>
				var version_flag = "<?php echo $is_same;?>";
				var version_table_obj = document.getElementById('version_table');
				str = "";
				str += "<br><font color='black'><?php echo language('System version help 1','Your current system version is :'); ?></font>" + "<font color='green' ><b> <?php echo $cur_sys_version;?><b></font><br>";
				str += "<br><font color='black'> <?php echo language('System version help 2','The latest system version is :'); ?></font>" + versionnum + "<br>";
				str += "<br><font color='black'><?php echo language('Be cautious', 'Be cautious, please');?>:</font><br>";

				var is_slave_update = 0;
				if (versionnum != "2.2.2") {
					//version_flag = 10;
					if (version_flag != 0 ) {
						version_status = version_flag % 2;
						if (version_status == 1 ) {   // means : slave not update
							version_table_obj.style.display = "";
							str += "<br><font color='black'>Version of slave boards is too low.Please make sure slave of all boards is 2.2.2 or higher!!</font><br>";
							str += "<br><font color='black'>Click </font><a href='http://downloads.openvox.cn/pub/firmwares/GSM%20Gateway/wg400-mid.img'><font color='blue'>here</font></a><font color='black'> to get the compatible version 2.2.2</font><br>";
							is_slave_update = 1;
							$("#button_online_now").attr("disabled",true);

						} else if (version_status == 0) {    // means : slave is disconnected
							version_table_obj.style.display = "";
							str += "<br><font color='black'><?php echo language('System Online Update caution1', 'This might damage your configuration files and disconnected boards could not be updated.');?></font><br>";
						}
					} else {
						str += "<br><font color='black'><?php echo language('System Online Update caution3', 'This might damage the structure of your original configuration files!');?></font><br>";
					}					
				} else {
					if (version_flag != 0 ) {
						if (version_flag >= 2) {    // means : slave is disconnected
							version_table_obj.style.display = "";
							str += "<br><font color='black'><?php echo language('System Online Update caution1', 'This might damage your configuration files and disconnected boards could not be updated.');?></font><br>";
						}
					} else {
						str += "<br><font color='black'><?php echo language('System Online Update caution3', 'This might damage the structure of your original configuration files!');?></font><br>";
					}
				}
				if (is_slave_update == 0) {
					str += "<br><font color='black'><?php echo language('System Online Update caution2', 'Are you sure to update your system?');?></font><br>";
				}
				
				str += "<br><br><font color='red' size= '3px'><?php echo language('Warning');?>: </font><br>";
				str += "<br><font color='green'><?php echo language('System Online Update caution4', 'DO NOT leave this page in the process of updating; OTHERWISE system updating will fail!');?></font><br>";
				document.getElementById("redmsg").innerHTML =  str;
				
				return;
			}
		}
	});
}

function update_system_online_step2()
{
	var server_file = "./../../cgi-bin/php/ajax_server.php";
	var str = "";
	//document.getElementById('showmsg').value = 'Getting information...';

	$.ajax({
		url: server_file+"?random="+Math.random()+"&type=system&system_type=newest_sys_changelog",	
		async: false,
		dataType: 'text',				
		type: 'GET',					
		timeout: 5000,
		error: function(data){				//request failed callback function;
			//document.getElementById('showmsg').value = "Can't get change log.";
			str = "";
			str += "<font color='black'>Can't get change log.</font><br>";
			document.getElementById("redmsg").innerHTML =  str;
		},
		success: function(data){			//request success callback function;
			//document.getElementById('showmsg').value = data;

			document.getElementById("redmsg").innerHTML =  data;
		}
	});
}

function update_system_online_step3()
{
	var server_file = "./../../cgi-bin/php/ajax_server.php";
	var str = "";
	//document.getElementById('showmsg').value = 'Getting information...';

	$.ajax({
		url: server_file+"?random="+Math.random()+"&type=system&system_type=sys_changelog",	
		async: false,
		dataType: 'text',				
		type: 'GET',					
		timeout: 5000,
		error: function(data){				//request failed callback function;
			//document.getElementById('showmsg').value = "Can't get detial change log.";
			
			str = "";
			str += "<font color='black'>Can't get detial change log.</font><br>";
			document.getElementById("redmsg").innerHTML =  str;
		},
		success: function(data){			//request success callback function;
			//document.getElementById('showmsg').value = data;
			document.getElementById("redmsg").innerHTML =  data;
		}
	});
}


function upload_cfg_file2()
{
	if(!isAllowFile('upload_cfg_file')) {
		return false;
	}

	if( ! confirm("<?php echo language('File Upload confirm','Are you sure to upload configuration files?\nThis will damage the structure of your original configuration files.');?>") ) {
		return false;
	}

	return true;
}

function online_update_firmware(){
	var firmware_link = document.getElementById('firmware_link').value;
	if(firmware_link == ""){
		alert("<?php echo language("Firmware Link");?>  <?php echo language("js check null");?>");
		return false;
	}
	
	if(!confirm("<?php echo language('System Update confirm','Are you sure to update system?\\nCaution: this might damage your configuration files.');?>")){
		return false;
	}
	
	return true;
}

function online_update_cfg_file(){
	var file_link = document.getElementById('file_link').value;
	if(file_link == ""){
		alert("<?php echo language("File Link");?>  <?php echo language("js check null");?>");
		return false;
	}
	
	if( ! confirm("<?php echo language('File Upload confirm','Are you sure to upload configuration files?\nThis will damage the structure of your original configuration files.');?>") ) {
		return false;
	}
	
	return true;
}
</script>

	<form id="manform" enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<!-- system update -->
	<?php
	function update_system(){
		if(! $_FILES) {
			return;
		}
	?>
		<div class="modal fade in" id="modal-51" >
			<div class="modal-dialog">
				<div class="modal-content">
					
					<div class="modal-header">
						<h4 class="modal-title"><?php echo language('System Update');?></h4>
					</div>
					
					<div class="modal-body" style="min-height:200px;">
						<?php 
						$success_flag = 0;
						if(isset($_FILES['update_sys_file']['error']) && $_FILES['update_sys_file']['error'] == 0) {
							if(!(isset($_FILES['update_sys_file']['size'])) || $_FILES['update_sys_file']['size'] > 80*1000*1000) {
								echo '<span style="color:red">'.language('Error').': '.language('System Update Filesize error',"Your updated file was larger than 80M!<br>Updating system was failed.").'</span>';
								echo '<div style="text-align:center;margin-top:30px;"><button id="button_cancel" type="button" style="margin-left:10px;">'.language('Close').'</button></div>';
							}else{
								$store_file = make_update_file_path();

								if (!move_uploaded_file($_FILES['update_sys_file']['tmp_name'], $store_file)) {  
									echo '<span style="color:red">'.language('Error').': '.language('System Update Move error',"Moving your updated file was failed!<br>Updating system was failed.").'</span>';
									echo '<div style="text-align:center;margin-top:30px;"><button id="button_cancel" type="button" style="margin-left:10px;">'.language('Close').'</button></div>';
								}else{
									echo language("System Updating");echo " ......";
									echo "<img src='/images/mini_loading.gif' />";
									$success_flag = 1;
								}
							}
						} else {
							if(isset($_FILES['update_sys_file']['error'])) {
								switch($_FILES['update_sys_file']['error']) {
								case 1:
									echo '<span style="color:red">'.language('Error').': '.language('System Update error 1',"The file was larger than the server space 80M!").'</span>';
									break;
								case 2:
									echo '<span style="color:red">'.language('Error').': '.language('System Update error 2',"The file was larger than the browser's limit!").'</span>';
									break;
								case 3:
									echo '<span style="color:red">'.language('Error').': '.language('System Update error 3',"The file was only partially uploaded!").'</span>';
									break;
								case 4:
									echo '<span style="color:red">'.language('Error').': '.language('System Update error 4',"Can not find uploaded file!").'</span>';
									break;
								case 5:
									echo '<span style="color:red">'.language('Error').': '.language('System Update error 5',"The server temporarily lost folder!").'</span>';    
									break;
								case 6:
									echo '<span style="color:red">'.language('Error').': '.language('System Update error 6',"Failed to write to the temporary folder!").'</span>';    
									break;    
								}
							}
							echo language("System Update Failed");
							echo '<div style="text-align:center;margin-top:30px;"><button id="button_cancel" type="button" style="margin-left:10px;">'.language('Close').'</button></div>';
						}
						?>
					</div>
					
				</div>
			</div>
		</div>
		<div class="modal-backdrop fade_in"></div>
		
		<script>
		$(document).on('click','#button_cancel',function(){
			$("#modal-51").hide();
			$(".fade_in").hide();
		});
		
		<?php if($success_flag == 1){ ?>
		$.ajax({
			url:"ajax_server.php?random="+Math.random()+"&type=system_update&store_file=<?php echo $store_file;?>",
			type:'GET',
			success:function(data){
				$(".modal-body").html(data);
				
				var update_time_len = $(".modal-body").find("#update_time").length;
				if(update_time_len == 1){
					settime(10);
				}
			},
			error:function(data){
			}
		});
		
		function settime(update_time){
			update_time--;
			$("#update_time").text(update_time);
			var f = setTimeout(
				function(){
					settime(update_time);
				}
			,1000);
			
			$("#button_cancel").click(function(){
				$("#modal-51").hide();
				$(".fade_in").hide();
				clearTimeout(f);
			});
			
			if(update_time <= 0){
				$(".modal-body").html("<?php echo language('System Reboot wait','System Rebooting...<br>Please wait for about 60s, system will be rebooting.');?><br/><img src='/images/loading.gif' />");
				$.ajax({
					url:"ajax_server.php?random="+Math.random()+"&type=system_reboot",
					type:'GET',
					success:function(data){
					},
					error:function(data){
					}
				});
				
				setTimeout("reboot_refresh()", 21000);
				clearTimeout(f);
			}
		}
		
		function reboot_refresh(){
			$.ajax({
				type: "GET",
				cache: false,
				url: "../../index.html",
				data: "",
				success:function(){
					window.location.href='../../index.html';
				},
				error:function(){
					setTimeout("reboot_refresh()", 1000);
				}
			});
		}
		<?php } ?>
		</script>
	<?php
	}
	if($_POST) {
		if(isset($_POST['send'])) {
			if($_POST['send'] == 'System Update') {
				update_system();
			}else if($_POST['send'] == 'System Online Update') {
				update_system_online();
			}else if($_POST['send'] == 'System Switch'){
				system_switch();
			}else if($_POST['send'] == 'Firmware Link System Online Update'){
				firmware_link_update_system_online();
			}else if($_POST['send'] == 'File Link Online Update'){
				file_link_online_update();
			}
		}
	}
	?>
	<!-- system update -->
	<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
	<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>
	
	<div class="content">
		<span class="title"><?php echo language('Reboot Tools');?></span>
		
		<div class="tab_item">
			<span>
				<?php echo language('System Reboot help','Reboot the gateway and all the current calls will be dropped.');?>
			</span>
			<div class="tab_item_right">
			
				<?php if(!$only_view){ ?>
				<input type="submit" value="<?php echo language('System Reboot');?>" <?php if($__demo_enable__=='on'){echo 'disabled';}?> 
					onclick="document.getElementById('send').value='System Reboot';return confirm('<?php echo language('System Reboot confirm','Are you sure to reboot your gateway now?\nYou will lose all data in memory!');?>')"/>
				<?php } ?>
					
			</div>
		</div>
		
		<?php if($_SESSION['id'] == 1){ ?>
		<div class="tab_item">
			<span>
				<?php echo language('System Reboot help','Reboot the gateway and all the current calls will be dropped.');?>
			</span>
			<div class="tab_item_right">
			
				<?php if(!$only_view){ ?>
				<input type="submit" value="<?php echo language('System Switch');?>" <?php if($__demo_enable__=='on'){echo 'disabled';}?> 
					onclick="document.getElementById('send').value='System Switch';return confirm('<?php echo language('System Switch confirm','Are you sure to switch system now?After switching over, the system needs to be restarted to take effect.');?>');" />
				<?php } ?>
					
			</div>
		</div>
		<?php } ?>
	</div>
	
	<div class="content">
		<span class="title"><?php echo language('Update Firmware');?></span>
		
		<div class="tab_item">
			<span>
				<?php echo language('New system file');?>:<input type="file" name="update_sys_file" onchange="return checkFileChange(this)" id="update_sys_file" <?php if($__demo_enable__=='on'){echo 'disabled';}?> />
				<br/>
				<span style="font-weight:normal;"><?php echo language('Gateway Update Completed', 'Gateway needs to be restarted after firmware upgrade is completed.');?></span>
			</span>
			<div class="tab_item_right">
			
				<?php if(!$only_view){ ?>
				<input type="submit" style="margin-top:-20px;" value="<?php echo language('System Update');?>" <?php if($__demo_enable__=='on'){echo 'disabled';}?> 
					onclick="document.getElementById('send').value='System Update';return update_system()" />
				<?php } ?>
					
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Firmware Link');?>:
				<input type="text" name="firmware_link" id="firmware_link" style="padding:5px;width:500px;height:14px;"/>
			</span>
			<div class="tab_item_right">
				<?php if(!$only_view){ ?>
				<input type="submit" style="margin-top:-0px;" value="<?php echo language('Online Update');?>" <?php if($__demo_enable__=='on'){echo 'disabled';}?> 
					onclick="document.getElementById('send').value='Firmware Link System Online Update';return online_update_firmware();" />
				<?php } ?>
			</div>
		</div>
	</div>

	<div class="content">
		<span class="title"><?php echo language('Upload Configuration');?></span>
		
		<div class="tab_item">
			<span>
				<?php echo language('New configuration file');?>:<input type="file" name="upload_cfg_file" onchange="return checkFileChange(this)" id="upload_cfg_file" <?php if($__demo_enable__=='on'){echo 'disabled';}?> />
			</span>
			<div class="tab_item_right">
			
				<?php if(!$only_view){ ?>
				<input type="submit" value="<?php echo language('File Upload');?>" <?php if($__demo_enable__=='on'){echo 'disabled';}?> 
					onclick="document.getElementById('send').value='File Upload';return upload_cfg_file2()" />
				<?php } ?>
					
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('File Link');?>:
				<input type="text" name="file_link" id="file_link" style="padding:5px;width:500px;height:14px;"/>
			</span>
			<div class="tab_item_right">
				<?php if(!$only_view){ ?>
				<input type="submit" value="<?php echo language('Online Update');?>" <?php if($__demo_enable__=='on'){echo 'disabled';}?> 
					onclick="document.getElementById('send').value='File Link Online Update';return online_update_cfg_file();" />
				<?php } ?>
			</div>
		</div>
	</div>

	<?php if($license_mode == 'on'){ ?>
	
	<div class="content">
		<span class="title"><?php echo language('License');?></title>
		
		<div class="tab_item">
			<span>
				<div>
					<?php echo language("Upload License file");?>:
					<input type="file" name="upload_lisence_file" id="upload_lisence_file" onchange="return check_upload_lisence();"/>
					<?php 
					if($lisence_status != "unlimited" && $lisence_status != ""){ 
						echo language("Software service expiration time").":&nbsp;&nbsp;";
						echo $lisence_endtime;
					}
					?>
				</div>
				<div>
					<span><?php echo language('Board Number');?></span>:
					<?php 
					$sys_type = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys sys_type");
					if($sys_type != 1){
					?>
						<select name="mod_brd" id="mod_brd" >
							<?php 
							$aql = new aql();
							$aql->set('basedir', '/tmp');
							$res = $aql->query('select * from hw_info.cfg');
							
							foreach($res['mod_brd'] as $key => $val){
								if(strstr($key, 'mod_brd')){
									$temp = explode('mod_brd_', $key);
									$temp = $temp[1];
									echo "<option value='$temp'>board-$temp</option>";
								}
							}
							?>
						</select>
					<?php }else{ ?>
						<input type="hidden" name="mod_brd" id="mod_brd" value="1" />
					<?php } ?>
					<span style="margin-left:10px;">UUID:</span>&nbsp;&nbsp;<span id="uuid"></span>
				</div>
			</span>
			<div class="tab_item_right">
			
				<?php if(!$only_view){ ?>
				<input type="submit" value="<?php echo language('License Upload');?>" <?php if($__demo_enable__=='on'){echo 'disabled';}?> onclick="document.getElementById('send').value='Lisence Upload';return checkLisenceFile();" />
				<?php } ?>
				
				<br/><button type="button" style="margin-top:5px;" id="uuid_btn"><?php echo language('Get UUID');?></button>
			</div>
		</div>
	</div>
	
	<?php } ?>
	
	<script>
	$("#uuid_btn").click(function(){
		var server_file = "./../../cgi-bin/php/ajax_server.php";
		var mod_brd = document.getElementById('mod_brd').value;
		
		if(mod_brd != '' && mod_brd != undefined){
			$.ajax({
				url: server_file+"?random="+Math.random()+"&type=get_uuid&mod_brd="+mod_brd,
				dataType: 'text',
				type: 'GET',
				success: function(data){
					document.getElementById("uuid").innerHTML = data;
				},
				error: function(){
					alert("error");
				},
			});
		}else{
			alert('<?php echo language("Get UUID error");?>');
		}
	});
	</script>
	
	<div class="content">
		<span class="title"><?php echo language('Backup Configuration');?></span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Current configuration file version');echo ": $cur_cfg_version"; ?>
			</span>
			<div class="tab_item_right">
			
				<?php if(!$only_view){ ?>
				<input type="submit" value="<?php echo language('Download Backup');?>" <?php if($__demo_enable__=='on'){echo 'disabled';}?> 
					onclick="document.getElementById('send').value='Download Backup';"/>
				<?php } ?>
					
			</div>
		</div>
	</div>

	<?php //if($_SESSION['id'] == 1){ ?>
	<div class="content">
		<span class="title"><?php echo language('Voice Record');?></span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Voice Record help','Select a module to recode voice.');?>
				<select name="port" style="margin-left:10px;">
					<?php
					$port = $_POST['port'];
					for($i=1; $i<=$__GSM_SUM__; $i++) {?>
					<option value="<?php echo $i;?>" <?php if($port == $i) echo "selected";?>><?php echo get_gsm_name_by_channel($i);?></option>
					<?php }?>
				</select>
			</span>
			<div class="tab_item_right">
			
				<?php if(!$only_view){ ?>
				<input type="submit" value="<?php echo language('Start Recording');?>" onclick="document.getElementById('send').value='Voice Start';" />
				<?php } ?>
				
				<div id="preview_dg" title="<?php echo language('Voice Record')?>" style="display:none;width:470px;height:100px">
					<div>
						<div id="timemsg" style="display:block;width:470px;height:100px;margin:0" contenteditable="false">
							<div id="time" ></div>
							<div id="prompt_info"></div>
						</div>
					</div>
				</div>
			</div>
		</div>
	</div>
	<?php //} ?>
	
	<div class="content">
		<span class="title"><?php echo language('Restore Configuration');?></span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Factory Reset help','
					This will cause all the configuration files to back to default factory values! And reboot your gateway once it finishes.');
				?>
			</span>
			<div class="tab_item_right">
				
				<?php if(!$only_view){ ?>
				<input type="submit" value="<?php echo language('Factory Reset');?>" <?php if($__demo_enable__=='on'){echo 'disabled';}?> 
					onclick="document.getElementById('send').value='Factory Reset';return confirm('<?php echo language('Factory Reset confirm','Are you sure to restore configuration file now?');?>')"/>
				<?php } ?>
					
			</div>
		</div>
	</div>
	
	<input type="hidden" name="send" id="send" value="" />
</form>

<style>
.tab_item_right input[type=button], input[type=submit]{
	min-width:145px;
}
</style>

<?php

function show_loading($str)
{
	$id = rand(1,10000);
	$id = "L$id";

	echo <<<EOF
	<table align="center" id="$id">
		<tr>
			<td align="center">
				<img src="/images/loading.gif" align="middle"/>
			</td>
		</tr>
		<tr>
			<td align="center">
				System Updating......
			</td>
		</tr>
	</table>
EOF;
//EOF

	ob_flush();
	flush();

	return $id;
}

function hide_loading($id)
{
	echo <<<EOF
	<script type="text/javascript">
	document.getElementById("$id").style.display='none';
	</script>
EOF;
//EOF

	ob_flush();
	flush();
}



if($_POST) {
	if(isset($_POST['send'])) {
		if($_POST['send'] == 'File Upload') {
			upload_cfg_file();
			
			save_user_record("","SYSTEM->Tools:Upload Conf File");
		} else if($_POST['send'] == 'Factory Reset') {
			res_def_cfg_file();
		} else if($_POST['send'] == 'System Reboot') {
			system_reboot();
		}else if($_POST['send'] == 'Voice Start'){
			voice_start();
			
			save_user_record("","SYSTEM->Tools:Voice Start");
		}else if($_POST['send'] == 'Voice Stop'){
			voice_stop();
			
			save_user_record("","SYSTEM->Tools:Voice Stop");
		}else if($_POST['send'] == 'Lisence Upload'){
			lisence_upload();
			
			save_user_record("","SYSTEM->Tools:License Upload");
		}
	}
}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>

<?php
if($_POST) {
	if(isset($_POST['send'])) {
		if($_POST['send'] == 'Download Backup') {
			backup_cfg_file();
		}
	}
}
?>
