<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/cdrdb.php");
include_once("/www/cgi-bin/inc/smsinboxdb.php");
include_once("/www/cgi-bin/inc/smsoutboxdb.php");

date_default_timezone_set('UTC');
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

function update_system($ip)
{
	global $all_version_info;
	global $__UPDATE_ALL_BOARD__;
	global $__UPDATE_SINGLE_BOARD__;
	global $__UNPACK_FILE__;
	global $__UPDATE_LOG_FILE__;
	$update_mode_flag = $__UPDATE_SINGLE_BOARD__;
	if ($ip == "select_all") {
		$update_mode_flag = $__UPDATE_ALL_BOARD__;   //1: means all update
	} else if ($ip == 'select_none'){
		return ;
	} else {
		$update_mode_flag = $__UPDATE_SINGLE_BOARD__;	//0: means single update		
	}

	if(! $_FILES) {
		return;
	}

	echo "<br>";
	$Report = language('Report');
	$Result = language('Result');
	$System_Update = language('System Update');
	trace_output_start("$Report", "$System_Update");
	trace_output_newline();
	if(isset($_FILES['update_sys_file']['error']) && $_FILES['update_sys_file']['error'] == 0) {  //Update successful
		if(!(isset($_FILES['update_sys_file']['size'])) || $_FILES['update_sys_file']['size'] > 80*1000*1000) { //Max file size 80Mbyte
			echo language('System Update Filesize error',"Your updated file was larger than 80M!<br>Updating system was failed.");
			trace_output_end();
			return;
		}

		$store_file = make_update_file_path();

		if (!move_uploaded_file($_FILES['update_sys_file']['tmp_name'], $store_file)) {  
			echo language('System Update Move error',"Moving your updated file was failed!<br>Updating system was failed.");  
			trace_output_end();
			return;
		}
		echo language("System Updating");echo " ......<br>\n";
		ob_flush();
		flush();

		global $cluster_info;
		if($cluster_info['mode'] == 'master') {
			global $__BRD_SUM__;
			global $__BRD_HEAD__;
			$httpd_conf = '/etc/asterisk/gw/httpd.conf';
			$lighttpd_user_conf = '/etc/asterisk/gw/lighttpd_https.conf';
			$lighttpdpassword = '/etc/asterisk/gw/lighttpdpassword_digest';
			$user = get_web_user();
		
			$lighttpd_user_contents = 'server.port = 80\n';
			$lighttpd_user_contents .= '\$SERVER[\"socket\"] == \":443\" {\n';
			$lighttpd_user_contents .= 'ssl.engine = \"enable\"\n';
			$lighttpd_user_contents .= 'ssl.pemfile = \"/etc/ssl/server.pem\"\n';
			$lighttpd_user_contents .= '}';
			if(is_file($lighttpd_user_conf)){
				$lighttpd_user_contents = file_get_contents($lighttpd_user_conf);
				if($lighttpd_user_contents == ''){
					$lighttpd_user_contents = 'server.port = 80\n';
					$lighttpd_user_contents .= '\$SERVER[\"socket\"] == \":443\" {\n';
					$lighttpd_user_contents .= 'ssl.engine = \"enable\"\n';
					$lighttpd_user_contents .= 'ssl.pemfile = \"/etc/ssl/server.pem\"\n';
					$lighttpd_user_contents .= '}';
				}
			}
			$lighttpd_user_contents = str_replace('"','\"',$lighttpd_user_contents);
			$lighttpd_user_contents = str_replace('$','\$',$lighttpd_user_contents);
			$lighttpdpassword_contents = 'admin:Openvox-Wireless-Gateway:9fdb881065b05a05a9700f70e258498b';
			if(is_file($lighttpdpassword)){
				$lighttpdpassword_contents = file_get_contents($lighttpdpassword);
				if($lighttpdpassword_contents == ''){
					$lighttpdpassword_contents = 'admin:Openvox-Wireless-Gateway:9fdb881065b05a05a9700f70e258498b';
				}
			}
			set_time_limit(1800);  //Set execute php script 1800 seconds.
			//get board connect status
			$board_connect_status = update_system_filter($all_version_info);
			
			for($i=2; $i<=$__BRD_SUM__; $i++) {
				for($b=$i; $b<=$i+3 && $b<=$__BRD_SUM__; $b++) {
				//for($b=2; $b<=$__BRD_SUM__; $b++) {
					if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
						$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
						if ( $update_mode_flag == $__UPDATE_SINGLE_BOARD__ ) {
							if($slaveip != $ip ) {
								continue;
							}
						} 
						if (isset($board_connect_status[$slaveip]) && $board_connect_status[$slaveip] != '') {
							if ($board_connect_status[$slaveip] != 0) {
								echo language("Slave");echo ": ". $slaveip ." ";
								echo language('Updating ignored');echo " .....<br>";
								ob_flush();
								flush();
								continue;
							}
						}
						echo language("Slave");echo ": ". $slaveip ." ";
						echo language('System Updating');echo " .....<br>";
						ob_flush();
						flush();

						$pid = pcntl_fork();
						if ($pid == 0) {
							set_time_limit(1800);  //Set execute php script 1800 seconds.
							$data  = 'syscmd:sed -i "s/^\/:.*/\/:'.$user['name'].':'.$user['password'].'/g" '.$httpd_conf.';';
							$data .= '/etc/init.d/httpd restart;';
							$data .= 'sleep 2;';
							request_slave($slaveip,$data,5,true);

							$data  = 'syscmd:echo -e "'.$lighttpd_user_contents.'" > '.$lighttpd_user_conf.';';
							$data .= 'echo -e "'.$lighttpdpassword_contents.'" > '.$lighttpdpassword.';';
							$data .= '/etc/init.d/lighttpd restart;';
							$data .= 'sleep 2;';
							request_slave($slaveip,$data,5,true);

							$data = 'syscmd:if [ -e /etc/init.d/httpd ]; then echo httpd; else echo lighttpd; fi';
							$ret = request_slave($slaveip,$data,5,true);
							$port = '';
							if(strncmp($ret,'httpd',5)==0){
								$port = 80;
							}

							sleep(2);

							echo language("Slave");echo ": ". $slaveip ." ";
							for($c=1; $c<=3; $c++) {
								$ret = update_slave($slaveip,$store_file,$port);
								if($ret){
									echo language("System Update Succeeded");echo "<br>\n";
									break;
								}else{
									echo language("System Update Failed");echo "<br>\n";
									if($c<3) {
										echo "Retry angain.<br>\n";
									} else {
										echo "Please manual re-update!!!<br>\n";
									}
								}
							}
							ob_flush();
							flush();

							exit(0);
						} else if($pid > 0) {
							$pids[] = $pid;
						} else {
							$error_str = language('fork error');
							echo $slaveip." ";
							echo language('Update failed');
							echo " ($error_str).<br>";
						}
						ob_flush();
						flush();
					}
				}

				if(isset($pids) && is_array($pids)) {
					foreach($pids as $each) {
						pcntl_waitpid($each,$status);
					}
					unset($pids);
				}
				$i = $b-1;
			}
		}


		if ($update_mode_flag == 0) {
			if ($ip == $all_version_info['master']['ip'] ) {
				exec("$__UNPACK_FILE__ $store_file  > $__UPDATE_LOG_FILE__ || echo $?",$output);
			}
		} else {
			exec("$__UNPACK_FILE__ $store_file  > $__UPDATE_LOG_FILE__ || echo $?",$output);
		}

		if (isset($output)) {
			trace_output_newhead("$Result");
			if($output) {
				echo language("System Update Failed");echo "<br>\n";
				echo language("Error code");echo ": ".$output[0];
			} else {
				exec("/my_tools/add_syslog \"System Update\"");
				echo "<!-- Successfully update your system! -->";
				echo language("System Update Succeeded");echo "<br>\n";
				echo language('System Update Succeeded help',"You must reboot system to entry the newer system.");
			}
		}
		//unlink($store_file);
		del_old_updatefile();
	} else {
		del_old_updatefile();    //Need edit this code by another time. --Freedom--
		if(isset($_FILES['update_sys_file']['error'])) {
			switch($_FILES['update_sys_file']['error']) {
			case 1: // �ļ���С�����˷������Ŀռ��С    
				echo language('System Update error 1',"The file was larger than the server space 80M!");
				break;
			case 2: // Ҫ�ϴ����ļ���С�������������    
				echo language('System Update error 2',"The file was larger than the browser's limit!");
				break;
			case 3: // �ļ������ֱ��ϴ�
				echo language('System Update error 3',"The file was only partially uploaded!");
				break;
			case 4: // û���ҵ�Ҫ�ϴ����ļ�
				echo language('System Update error 4',"Can not find uploaded file!");
				break;
			case 5: // ��������ʱ�ļ��ж�ʧ
				echo language('System Update error 5',"The server temporarily lost folder!");    
				break;
			case 6: // �ļ�д�뵽��ʱ�ļ��г���
				echo language('System Update error 6',"Failed to write to the temporary folder!");    
				break;    
			}
		}
		echo "<br>";
		trace_output_newhead("$Result");
		echo language("System Update Failed");
	}
	trace_output_end();
}

function update_system_online()
{
	global $__UNPACK_FILE__;
	global $__UPDATE_LOG_FILE__;
	
	?>
	<br>
	<table width="75%" style="font-size:12px;" align="center">
		<tr>
			<td align="center"><?php echo language('Downloading');?></td>
			<td width="30px"></td>
			<td rowspan=2 width="60px" valign="bottom"><input type="button" value="Cancel" onclick="location=location" /></td>
		</tr>
		<tr>
			<td style="border: 1px solid rgb(59, 112, 162);">
				<div id="progress_bar" style="float:left;text-align:center;width:1px;color:#000000;background-color:rgb(208, 224, 238)"></div>
			</td>
			<td width="30px"><div id="progress_per">0%</div></td>
		</tr>
	</table>

	<script type="text/javascript">
		var filesize=0;

		function set_filesize(fsize)
		{
			filesize=fsize;
		}

		function set_downloaded(fsize)
		{
			if(filesize>0){
				var percent=Math.round(fsize*100/filesize);
				document.getElementById("progress_bar").style.width=(percent+"%");
				if(percent>0){
					document.getElementById("progress_bar").innerHTML = fsize+"/"+filesize;
					document.getElementById("progress_per").innerHTML = percent+"%";
				}else{
					document.getElementById("progress_per").innerHTML = percent+"%";
				}
			}
		}

	</script>

	<?php
	//if this is temp version, then use wg400-current.img, else use wg400-current.bin
	//$url="http://downloads.openvox.cn/pub/firmwares/GSM%20Gateway/wg400-current.img";
	//$url="http://downloads.openvox.cn/pub/firmwares/GSM%20Gateway/wg400-current.bin";
	$url="http://downloads.openvox.cn/pub/firmwares/Wireless_Gateway/SWG/swg-current.bin";
	
		
	$store_file = make_update_file_path();
	$remote_fh = @fopen ($url, "rb");
	if ($remote_fh){
		$filesize = -1;
		$headers = @get_headers($url, 1); 
		if(is_array($headers)){
			if ((!array_key_exists("Content-Length", $headers))) {
				$filesize=0;
			}
			$filesize = $headers["Content-Length"];
		}else{
			echo "<script>\n";
			echo "alert(\"";echo language('System Online Update Download error1','Download system file failed. Please check the network connection!');echo "\");\n";
			echo "window.location.href=\"".get_self()."\"\n";
			echo "</script>\n";
		}
		if($filesize != -1) 
			echo "<script>set_filesize($filesize);</script>";
		$store_fh = @fopen ($store_file, "wb");
		$downlen = 0;
		if ($store_fh){
			while(!feof($remote_fh)) {
				$data=fread($remote_fh, 1024 * 8 );
				if($data==false){
					echo "<script>\n";
					echo "alert(\"";echo language('System Online Update Download error1','Download system file failed. Please check the network connection!');echo "\");\n";
					echo "window.location.href=\"".get_self()."\"\n";
					echo "</script>\n";
					break;
				}else{
					$downlen += strlen($data);
					fwrite($store_fh, $data, 1024 * 8 );
					echo "<script>set_downloaded($downlen);</script>";
					ob_flush();
					flush();
				}
			}
			fclose($store_fh);
		}else{
			echo "<script>\n";
			echo "alert(\"";echo language('System Online Update fopen error','Save system file failed!');echo "\");\n";
			echo "window.location.href=\"".get_self()."\"\n";
			echo "</script>";
		}
		fclose($remote_fh);
	}else{
		echo "<script>\n";
		echo "alert(\"";echo language('System Online Update Download error1','Download system file failed. Please check the network connection!');echo "\");\n";
		echo "window.location.href=\"".get_self()."\"\n";
		echo "</script>\n";
	}

	if(!file_exists($store_file))
		return false;

	$Report = language('Report');
	$Result = language('Result');
	$System_Online_Update = language('System Online Update');
	trace_output_start("$Report", "$System_Online_Update");
	trace_output_newline();

	echo language("System Updating");echo "......<br>";
	ob_flush();
	flush();

	global $cluster_info;
	global $all_version_info;
	if($cluster_info['mode'] == 'master') {
		global $__BRD_SUM__;
		global $__BRD_HEAD__;
		$httpd_conf = '/etc/asterisk/gw/httpd.conf';
		$lighttpd_user_conf = '/etc/asterisk/gw/lighttpd_https.conf';
		$lighttpdpassword = '/etc/asterisk/gw/lighttpdpassword_digest';
		$user = get_web_user();

		$lighttpd_user_contents = 'server.port = 80\n';
		$lighttpd_user_contents .= '\$SERVER[\"socket\"] == \":443\" {\n';
		$lighttpd_user_contents .= '  ssl.engine = \"enable\"\n';
		$lighttpd_user_contents .= '  ssl.pemfile = \"/etc/ssl/server.pem\"\n';
		$lighttpd_user_contents .= '}';
		if(is_file($lighttpd_user_conf)){
			$lighttpd_user_contents = file_get_contents($lighttpd_user_conf);
			if($lighttpd_user_contents == ''){
				$lighttpd_user_contents = 'server.port = 80\n';
				$lighttpd_user_contents .= '\$SERVER[\"socket\"] == \":443\" {\n';
				$lighttpd_user_contents .= 'ssl.engine = \"enable\"\n';
				$lighttpd_user_contents .= 'ssl.pemfile = \"/etc/ssl/server.pem\"\n';
				$lighttpd_user_contents .= '}';
			}
		}
		$lighttpd_user_contents = str_replace('"','\"',$lighttpd_user_contents);
		$lighttpd_user_contents = str_replace('$','\$',$lighttpd_user_contents);

		$lighttpdpassword_contents = 'admin:Openvox-Wireless-Gateway:9fdb881065b05a05a9700f70e258498b';
		if(is_file($lighttpdpassword)){
			$lighttpdpassword_contents = file_get_contents($lighttpdpassword);
			if($lighttpdpassword_contents == ''){
				$lighttpdpassword_contents = 'admin:Openvox-Wireless-Gateway:9fdb881065b05a05a9700f70e258498b';
			}
		}

		set_time_limit(1800);  //Set execute php script 1800 seconds.
		//get board connect status
		$board_connect_status = update_system_filter($all_version_info);
		
		for($i=2; $i<=$__BRD_SUM__; $i++) {
			for($b=$i; $b<=$i+3 && $b<=$__BRD_SUM__; $b++) {
			//for($b=2; $b<=$__BRD_SUM__; $b++) {
				if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
					
					if (isset($board_connect_status[$slaveip]) && $board_connect_status[$slaveip] != '') {
						if ($board_connect_status[$slaveip] != 0) {
							echo language("Slave");echo ": ". $slaveip ." ";
							echo language('Updating ignored');echo " .....<br>";
							ob_flush();
							flush();
							continue;
						}
					}
					echo language("Slave");echo ": ". $slaveip ." ";
					echo language('System Updating');echo " .....<br>";
					ob_flush();
					flush();

					$pid = pcntl_fork();
					if ($pid == 0) {
						set_time_limit(1800);  //Set execute php script 1800 seconds.
						$data  = 'syscmd:sed -i "s/^\/:.*/\/:'.$user['name'].':'.$user['password'].'/g" '.$httpd_conf.';';
						$data .= '/etc/init.d/httpd restart;';
						$data .= 'sleep 2;';
						request_slave($slaveip,$data,5,true);

						$data  = 'syscmd:echo -e "'.$lighttpd_user_contents.'" > '.$lighttpd_user_conf.';';
						$data .= 'echo -e "'.$lighttpdpassword_contents.'" > '.$lighttpdpassword.';';
						$data .= '/etc/init.d/lighttpd restart;';
						$data .= 'sleep 2;';
						request_slave($slaveip,$data,5,true);

						$data = 'syscmd:if [ -e /etc/init.d/httpd ]; then echo httpd; else echo lighttpd; fi';
						$ret = request_slave($slaveip,$data,5,true);
						$port = '';
						if(strncmp($ret,'httpd',5)==0){
							$port = 80;
						}

						sleep(2);

						echo language("Slave");echo ": ". $slaveip ." ";
						for($c=1; $c<=3; $c++) {
							$ret = update_slave($slaveip,$store_file,$port);
							if($ret){
								echo language("System Update Succeeded");echo "<br>\n";
								break;
							}else{
								echo language("System Update Failed");echo "<br>\n";
								if($c<3) {
									echo "Retry angain.<br>\n";
								} else {
									echo "Please manual re-update!!!<br>\n";
								}
							}
						}
						ob_flush();
						flush();

						exit(0);
					} else if($pid > 0) {
						$pids[] = $pid;
					} else {
						$error_str = language('fork error');
						echo $slaveip." ";
						echo language('Update failed');
						echo " ($error_str).<br>";
					}
					ob_flush();
					flush();
				}
			}

			if(isset($pids) && is_array($pids)) {
				foreach($pids as $each) {
					pcntl_waitpid($each,$status);
				}
				unset($pids);
			}
			$i = $b-1;
		}
	}

	exec("$__UNPACK_FILE__ $store_file  > $__UPDATE_LOG_FILE__ || echo $?",$output);
	trace_output_newhead("$Result");
	if($output) {
		echo language('System Update Failed');echo "<br>\n";
		echo language("Error code");echo ": ".$output[0];
	} else {
		exec("/my_tools/add_syslog \"System Update\"");
		echo language('System Update Succeeded');echo "<br>\n";
		echo language('System Update Succeeded help',"You must reboot system to entry the newer system.");
	}
	del_old_updatefile();
	trace_output_end();
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

		//Get version
		$cmd = "tar vxz -f $store_file -C /tmp ./cfg/cfg_version"; 
		exec("$cmd > /dev/null 2>&1 || echo $?",$output);
		if($output || !file_exists("/tmp/cfg/cfg_version")) {
			echo language("Configuration Files Upload Failed");
			echo language('Configuration Files Upload version error 1',"</br>Can not find configration file!</br>Can not find asterisk/cfg_version!<br>");
			exec("rm /tmp/cfg/cfg_version -rf > /dev/null 2>&1");
			exec("rm $store_file -rf > /dev/null 2>&1");
			trace_output_end();
			return ;
		}

		echo language('Configuration Files Upload check',"Checking configuration version....<br>");
		ob_flush();
		flush();
		$version=trim(`cat /tmp/cfg/cfg_version`);

		if($version == "") {
			echo language("Configuration Files Upload Failed");
			echo language('Configuration Files Upload version error 2',"</br>Unknown configuration version!");
			exec("rm /tmp/cfg/cfg_version -rf > /dev/null 2>&1");
			exec("rm $store_file -rf > /dev/null 2>&1");
			trace_output_end();
			return;
		}

		$cmd = "( cat /version/cfg_ver_list 2> /dev/null | grep \"$version\" > /dev/null 2>&1 )";
		exec("$cmd || echo $?",$output);
		if($output) {
			echo language("Configuration Files Upload Failed");
			echo language('Configuration Files Upload version error 3',"<br>Your current system does not support this configuration version!</br>");
			exec("rm /tmp/cfg/cfg_version -rf > /dev/null 2>&1");
			exec("rm $store_file -rf > /dev/null 2>&1");
			trace_output_end();
			return;
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
		$cmd = "tar zxf $store_file -C /etc";
		exec($cmd);
		exec("cp /etc/cfg/* /etc/asterisk/ -a");

		echo language('Start Asterisk',"Starting asterisk....<br>");
		ob_flush();
		flush();
		exec("/etc/init.d/asterisk start > /dev/null 2>&1");

		unlink($store_file);

		trace_output_newhead("$Result");
		echo language("Configuration Files Upload Succeeded");
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

	//configuration file name
	$cfg_name="config-$cur_cfg_version.tar.gz";

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
	echo language("----------------------");
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
	if($cluster_info['mode'] == 'master') {
		for($b=2; $b<=$__BRD_SUM__; $b++) {
			if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
				trace_output_newline();
				$data = "syscmd:/my_tools/restore_cfg_file > /dev/null 2>&1\n";
				$ip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
				request_slave($ip, $data, 5, false);
				echo language("Slave");echo ": $ip ";
				echo language("Configuration Restoring");echo " ......<br/>";
				ob_flush();
				flush();
			}
		}
	}
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
		for($b=2; $b<=$__BRD_SUM__; $b++) {
			if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
				$data = "syscmd:reboot -f > /dev/null 2>&1\n";
				$ip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
				request_slave($ip, $data, 5, false);
				echo language("Slave");echo ":$ip ";echo language("System Rebooting");echo " ......<br/>";
				ob_flush();
				flush();
			}
		}
		echo language("System Rebooting");echo "......";
	}else{
		echo language("System Rebooting");echo "......";
	}
	trace_output_end();
	exec("sleep 3");
	exec("/sbin/hwclock -w > /dev/null 2>&1");
	exec("reboot -f > /dev/null 2>&1");
}

function ast_reboot()
{
	global $cluster_info;
	global $__BRD_HEAD__;
	global $__BRD_SUM__;

	echo "<br>";
	$Report = language('Report');
	$Asterisk_Reboot = language('Asterisk Reboot');
	trace_output_start("$Report","$Asterisk_Reboot");
	trace_output_newline();
	ob_flush();
	flush();

	if($cluster_info['mode'] == 'master') {
		for($b=2; $b<=$__BRD_SUM__; $b++) {
			if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
				$data = "syscmd:/etc/init.d/asterisk restart > /dev/null 2>&1\n";
				$ip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
				request_slave($ip, $data, 5, false);
				echo language("Slave");echo ":$ip ";echo language("Asterisk Rebooting");echo " ......<br/>";
				ob_flush();
				flush();
			}
		}
		echo language("Asterisk Rebooting");echo " ......<br/>";
	}else{
		echo language("Asterisk Rebooting");echo " ......<br/>";
	}
	exec("/etc/init.d/asterisk restart > /dev/null 2>&1 || echo $?",$output);

	$Result = language('Result');
	trace_output_newhead("$Result");
	if(!$output) {
		echo language("Asterisk Reboot Succeeded");
	} else {
		echo language("Asterisk Reboot Failed");echo "($output[0])";
	}

	trace_output_end();
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

			if(!confirm("<?php echo language('System Update confirm1','Are you sure to update system?\\nCaution: this might damage your configuration files.');?>")){
				return false;
			}
			return true;
		}	
	} else if (sel == null) {   // stand_alone or slave , can not do "select" operation
		if(!isAllowFile('update_sys_file')) {
			return false;
		}

		if(!confirm("<?php echo language('System Update confirm1','Are you sure to update system?\\nCaution: this might damage your configuration files.');?>")){
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
					//window.open("http://downloads.openvox.cn/pub/firmwares/GSM%20Gateway/wg400-mid.img");
					return false;
				}				
			} else {
				//use ".img" , back to low version
				version_table_obj.style.display = "";
				if (version_flag == version_status) {
					if(!confirm("<?php echo language('System Update confirm1','Are you sure to update system?\\nCaution: this might damage your configuration files.');?>")){
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

	if(!confirm("<?php echo language('System Update confirm1','Are you sure to update system?\\nCaution: this might damage your configuration files.');?>")){
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
				text:"Change Log",
				id:"change_log",
				click:function(){
					update_system_online_step2();
				}
			},
			{
				text:"Detailed",
				id:"detailed",
				click:function(){
					update_system_online_step3();
				}
			},
			{
				text:"Update Online Now",
				id:"button_online_now",
				click:function(){
					$( this ).dialog( "close" );
					document.getElementById('send').value='System Online Update';
					document.getElementById('manform').submit();
				}
			},
			{
				text:"Cancel",
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
			str += "<br><br><font color='red' size= '3px'>ERROR: </font> <font color='green'>Get remote version failed.</font>";
			
		
			document.getElementById("redmsg").innerHTML = str;
			
		},
		success: function(data){			//request success callback function;
			var versionnum = data;
			if(versionnum == ''){
				//document.getElementById('showmsg').value = "<?php echo language('System Online Update version error1','Get remote version failed. Please check the network connection!');?>";
				str = "";
				str += "<br><font color='black' > <?php echo language('System Online Update version error1','Get remote version failed. Please check the network connection!');?></font>";
				str += "<br><br><font color='red' size= '3px'>ERROR: </font> <font color='green'>Get remote version failed.</font>";
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
				str += "<br><font color='black'>Be cautious, please:</font><br>";

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
							str += "<br><font color='black'>This might damage your configuration files and disconnected boards could not be updated. </font><br>";
						}
					} else {
						str += "<br><font color='black'>This might damage the structure of your original configuration files! </font><br>";
					}					
				} else {
					if (version_flag != 0 ) {
						if (version_flag >= 2) {    // means : slave is disconnected
							version_table_obj.style.display = "";
							str += "<br><font color='black'>This might damage your configuration files and disconnected boards could not be updated. </font><br>";
						}
					} else {
						str += "<br><font color='black'>This might damage the structure of your original configuration files! </font><br>";
					}
				}
				if (is_slave_update == 0) {
					str += "<br><font color='black'>Are you sure to update your system?</font><br>";
				}
				
				str += "<br><br><font color='red' size= '3px'>Warning: </font><br>";
				str += "<br><font color='green'>DO NOT leave this page in the process of updating; OTHERWISE system updating will fail! </font><br>";
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
</script>

	<form id="manform" enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Reboot Tools');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tctl" >
		<tr>
			<th>
			<?php echo language('System Reboot help','Reboot the gateway and all the current calls will be dropped.');?>
			</th>
			<td>
				<input type="submit" value="<?php echo language('System Reboot');?>" 
					onclick="document.getElementById('send').value='System Reboot';return confirm('<?php echo language('System Reboot confirm','Are you sure to reboot your gateway now?\nYou will lose all data in memory!');?>')"/>
			</td>
		</tr>
	</table>

	<br/>

	<table width="100%" class="tctl" >
		<tr>
			<th>
			<?php echo language('Asterisk Reboot help','Reboot the asterisk and all the current calls will be dropped.');?>
			</th>
			<td>
				<input type="submit" value="<?php echo language('Asterisk Reboot');?>" 
					onclick="document.getElementById('send').value='Asterisk Reboot';return confirm('<?php echo language('Asterisk Reboot confirm','Are you sure to reboot Asterisk now?');?>')"/>
			</td>
		</tr>
	</table>

	<br/>
	
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Update Firmware');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tctl" >
		<tr>
			<th>
				<?php echo language('New system file');?>:<input type="file" name="update_sys_file" onchange="return checkFileChange(this)" id="update_sys_file"/>
				<?php 
					if($cluster_info['mode'] == 'master' ) {
				?>	
						<?php echo language("slave update select","Click the menu to select which updates");?>:
						<select name="select_update" id="select_update" >
							<option value="select_all" id="select_all"> all boards</option>
							
				<?php 
						foreach($all_version_info as $key=>$info) {
							if ($info['error'] == '') {
				?>				<option value="<?php echo $info['ip'];?>" > <?php echo $info['name'];?> </option>
				<?php		} else {
								if ($info['error'] == 'disconnected') {
				?>					<option value="error" > <?php echo $key; ?> </option>
				<?php			} else {
				?>
									<option value="<?php echo $info['ip'];?>" > <?php echo $key; ?> </option>
				<?php
								}
							}
						}
				?>
						</select>
				<?php
					}
				?>				
				
			</th>

			<td>
				<input type="submit" value="<?php echo language('System Update');?>" 
					onclick="document.getElementById('send').value='System Update';return update_system()" />
			</td>
		</tr>
	</table>


<?php 

	if($cluster_info['mode'] == 'master' || $cluster_info['mode'] == 'stand_alone') {	
?>
	<br>
	<table width="100%" class="tshow" style="display:none" id="version_table">  
<!--		<table width="100%" class="tshow"  id="version_table"> -->
			<tr  id="title_cmp">
				<th width="25%" ><?php echo language('Board Name');?></td>
				<th width="25%" ><?php echo language('IP');?></td>
				<th width="25%" ><?php echo language('Version');?></td>
				<th width="25%" ><?php echo language('Build Time');?></td>
			</tr>
		<?php 
		foreach ($all_version_info as $key=>$info){
			if ($info['error'] == '') {
		?>
				<tr id="<?php echo $info['name'];?>" >
					<td ><?php echo $info['name'];?></td>
					<td ><?php echo $info['ip'];?></td>
					<td ><?php echo $info['local.product.sw.version'];?></td>
					<td ><?php echo $info['local.product.sw.buildtime'];?></td>
				</tr>
		<?php
			} else {
				if ($info['error'] == 'disconnected') {
		?>
					<tr id="<?php echo $key;?>"  >
						<td ><?php echo $key;?></td>
						<td ><?php echo $info['ip'];?></td>
						<td colspan="2">disconnected</td>
					</tr>
		<?php	
				} else {
		?>
					<tr id="<?php echo $key;?>"  >
						<td ><?php echo $key;?></td>
						<td ><?php echo $info['ip'];?></td>
						<td colspan="2" >incompatible version</td>
					</tr>				
		<?php
				}
				
			}
		}	
		?>	
		<!--
		<tr>
			<td id='cmp_res' style="font-size:17px" colspan="4" >
				
			</td>
		</tr>				
		-->
	</table> 		
<?php 
	} 
?>	

	

	<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
	<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Upload Configuration');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tctl">
		<tr>
			<th>
				<?php echo language('New configuration file');?>:<input type="file" name="upload_cfg_file" onchange="return checkFileChange(this)" id="upload_cfg_file"/>
			</th>
			<td>
				<input type="submit" value="<?php echo language('File Upload');?>" 
					onclick="document.getElementById('send').value='File Upload';return upload_cfg_file2()" />
			</td>
		</tr>
	</table>

	<br/>

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Backup Configuration');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	<table width="100%" class="tctl">
		<tr>
			<th>
				<?php echo language('Current configuration file version');echo ": $cur_cfg_version"; ?>
			</th>
			<td>
				<input type="submit" value="<?php echo language('Download Backup');?>" 
					onclick="document.getElementById('send').value='Download Backup';"/>
			</td>
		</tr>
	</table>
	
	<br/>

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Restore Configuration');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tctl" >
		<tr>
			<th>
			<?php echo language('Factory Reset help','
				This will cause all the configuration files to back to default factory values! And reboot your gateway once it finishes.');
			?>
			</th>
			<td>
				<input type="submit" value="<?php echo language('Factory Reset');?>" 
					onclick="document.getElementById('send').value='Factory Reset';return confirm('<?php echo language('Factory Reset confirm','Are you sure to restore configuration file now?');?>')"/>
			</td>
		</tr>
	</table>
	<input type="hidden" name="send" id="send" value="" />
	</form>

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
		if($_POST['send'] == 'System Update') {
			if (isset($_POST['select_update'])) {
				$id=show_loading("System Updating......");
				$update_mode = $_POST['select_update'];		
				update_system($update_mode);
				hide_loading($id);
			} else {
				$id=show_loading("System Updating......");
				update_system(0);
				hide_loading($id);
			}
		} else if($_POST['send'] == 'System Online Update') {
			$id=show_loading("System Online Updating......");
			update_system_online();
			hide_loading($id);
		} else if($_POST['send'] == 'File Upload') {
			upload_cfg_file();
		} else if($_POST['send'] == 'Factory Reset') {
			res_def_cfg_file();
		} else if($_POST['send'] == 'System Reboot') {
			system_reboot();
		} else if($_POST['send'] == 'Asterisk Reboot') {
			ast_reboot();
		}
	}
}
?>
<script type="text/javascript">
function onload_func()
{
	version_check();
}

$(document).ready(function (){ 
	onload_func();
}); 
</script>


<?php require("/www/cgi-bin/inc/boot.inc");?>

<?php
/*
if($_POST && $g_restore_cfg_file) {
	exec("/my_tools/restore_cfg_file > /dev/null 2>&1 || echo $?",$output);
	if($output) {
		echo "Restoring default configruation file was failed!";
		echo "Error code:$output[0]";
		flush();
		ob_flush();
	}
}
*/

if($_POST) {
	if(isset($_POST['send'])) {
		if($_POST['send'] == 'Download Backup') {
			backup_cfg_file();
		}
	}
}
?>
