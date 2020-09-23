<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
require_once("/www/cgi-bin/inc/aql.php");

function show_security(){
	$sip_general_tls_conf = "/etc/asterisk/sip_general_tls.conf";
	$tls_para_array = parse_ini_file($sip_general_tls_conf, true, INI_SCANNER_RAW);
	if(isset($tls_para_array['tlsbindaddr'])){
		$tlsbindaddr = explode(':', $tls_para_array['tlsbindaddr'], 2);
		$port = $tlsbindaddr[1];
	}
	
	if(!isset($port) && $port == ''){
		$port = '5061';
	}
	
	if(isset($tls_para_array['tlsenable'])){
		$tls_enable = $tls_para_array['tlsenable'];
	}else{
		$tls_enable = 'no';
	}
	
	if(isset($tls_para_array['tlsdontverifyserver'])){
		$tls_verify_server = $tls_para_array['tlsdontverifyserver'];
	}else{
		$tls_verify_server = 'no';
	}
	
	if(isset($tls_para_array['tlsclientmethod'])){
		$tls_client_method = $tls_para_array['tlsclientmethod'];
	}else{
		$tls_client_method = 'tlsv1';
	}
	
	$keys_conf = "/etc/asterisk/tls/keys.conf";
	$keys_conf_array = parse_ini_file($keys_conf, true, INI_SCANNER_RAW);
	
	if(isset($_GET['page']) && is_numeric($_GET['page']) && $_GET['page'] > 1){
		$cur_page = $_GET['page'];
	}else{
		$cur_page = 1;
	}
	
	$dir = "/etc/asterisk/keys";
	$i = 0;
	exec("/bin/ls $dir",$list_files);
	foreach($list_files as $file){
		if($file != '.' && $file != '..'){
			$filepath = $dir."/".$file;
			if(is_file($filepath)){
				$all_files[$i] = $file;
				$i++;
			}
		}
	}
	$page_count = ceil($i/5);
	if($cur_page > $page_count){
		$cur_page = $page_count;
	}
	
?>
<script type="text/javascript" src="/js/jquery.ibutton.js"></script>
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />
<script type="text/javascript" src="/js/float_btn.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/functions.js"></script>
<form enctype="multipart/form-data" action="<?php echo $_SERVER['PHP_SELF']?>" method="post">
	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_tls_settings')" id="tab_tls_settings_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_tls_settings')"><?php echo language('TLS Setting');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_tls_settings')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>
	
	<div id="tab_tls_settings">
		<table width="98%" class="tedit" align="right">
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('TLS Enable');?>:
						<span class="showhelp">
						<?php echo language('TLS Enable help','Enable or disable DTLS-SRTP support.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="tls_enable" id="tls_enable" <?php 
					if(strcmp($tls_enable,"yes")==0){
						echo "checked";
					}
					?> />
					<span id="ctls_enable"></span>
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('TLS Verify Server');?>:
						<span class="showhelp">
						<?php echo language('Enable TLS Verify Server help','Enable or disable tls verify server(default is no).');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="tls_verify_server" id="tls_verify_server" <?php
					if(strcmp($tls_verify_server,"no")==0){
						echo "checked";
					}
					?> />
					<span id="ctls_verify_server"></span>
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Port');?>:
						<span class="showhelp">
						<?php echo language('Port help','Specify the port for remote connection.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" id="port" name="port" value="<?php echo $port;?>"/>
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('TLS Client Method');?>:
						<span class="showhelp">
						<?php echo language('TLS Client Method help','values include tlsv1, sslv3, sslv2, Specify protocol for outbound client connections, default is sslv2.');?>
						</span>
					</div>
				</th>
				<td>
					<select size=1 name="tls_client_method" id="tls_client_method">
						<option value="tlsv1" <?php if(strcmp($tls_client_method, 'tlsv1') == 0) echo "selected"; ?>> <?php echo language('tlsv1');?> </option>
						<option value="sslv2" <?php if(strcmp($tls_client_method, 'sslv2') == 0) echo "selected"; ?>> <?php echo language('sslv2');?> </option>
						<option value="sslv3" <?php if(strcmp($tls_client_method, 'sslv3') == 0) echo "selected"; ?>> <?php echo language('sslv3');?> </option>
					</select>
				</td>
			</tr>
		</table>
	</div>
	
	<div id="newline"></div>
	
	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_keys')" id="tab_keys_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_keys')"><?php echo language('TLS keys');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_keys')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>
	
	<div id="tab_keys">
		<table width="98%" class="tshow" align="right">
			<thead>
				<tr>
					<th><?php echo language('Type');?></th>
					<th><?php echo language('Key Name');?></th>
					<th><?php echo language('IP Address');?></th>
					<th><?php echo language('Organization');?></th>
					<th><?php echo language('Password');?></th>
					<th><?php echo language('Operation');?></th>
				</tr>
			</thead>
			
			<tbody>
				<tr>	
					<td>
						<select id="type" name="type" onchange="change_type();">
							<option value="client"><?php echo language('client')?></option>
							<option value="server"><?php echo language('server')?></option>
						</select>
					</td>
					<td>
						<input type="text" id="keyname" name="keyname" value="" align="left"/>
						<span id="ckeyname"></span>
					</td>
					<td>
						<input type="text" id="ip_address" name="ip_address" value="" align="left" />
						<span id="cip_address"></span>
					</td>
					<td>
						<input type="text" id="organization" name="organization" value="" align="left" />
						<span id="corganization"></span>
					</td>
					<td>
						<input type="text" id="password" name="password" value="" align="left" />
						<span id="cpassword"></span>
					</td>
					<td>
						<input class="button" type="submit" value="<?php echo language('Create');?>" align="left" onclick="document.getElementById('send').value='Add';return addcheck();" />
						<span id="ccreate"></span>
					</td>
				</tr>
				
				<?php foreach($keys_conf_array as $key => $key_value) { ?>
				<tr>
					<?php	
					foreach($key_value as $para => $value){
						if($para == "keyname"){
							$key_num = $value;
						}
						echo "<td>$value</td>";
					}
					?> 
					<td>
						<input class="button" type="submit" value="<?php echo language('Delete');?>" onclick="document.getElementById('send').value='Delete';return delete_key('<?php echo $key;?>');" />
						<input class="button" type="submit" value="<?php echo language('Download');?>" onclick="document.getElementById('send').value='Download'; return download_key('<?php echo $key_num;?>');" />
					</td>
				</tr>
				<?php } ?>
			</tbody>
		</table>
	</div>
	
	<div id="newline"></div>
	
	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_show_files')" id="tab_show_files_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_show_files')"><?php echo language('Key Files');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_show_files')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>
	
	<div id="tab_show_files" style="display:block;clear:both;height:180px">
		<table width="98%" class="tctl" align="right">
			<tr>
				<th>
					<?php echo language('Upload the pem file');?>:
					<input type="file" name="key_file[]" id="key_file[] "onchange=" check_pem_file_format(this)">
					<br/>
					<?php echo language('Upload the crt file');?>:
					<input type="file" name="key_file[]" id="key_file[] "onchange="check_crt_file_format(this)">
				</th>
				<td>
					<input type="submit" value="<?php echo language('File Upload');?>" onclick="document.getElementById('send').value='Upload'; return check_file_format();" />
				</td>
			</tr>
		</table>
		
		<br />
		
		<table width="98%" class="tshow" align="right" >
			<tr>
				<th>
					<?php echo language('File Name');?>
				</th>
				<th>
					<?php echo language('File Size');?>
				</th>
				<th width="2%">
					<?php echo language('Operation');?>
				</th>
			</tr>
			
			<?php
				for($i=($cur_page-1)*5; $i<($cur_page-1)*5+5; $i++) {
					if(!isset($all_files[$i])) {
						break;
					}

					echo "<tr>";
					echo "<td>".$all_files[$i]."</td>";
					echo "<td>".filesize($dir."/".$all_files[$i])."</td>";
			?>
						<td>
							<button type="submit" value="Delete" style="width:32px;height:32px;" 
								onclick="document.getElementById('send').value='Delete_File';return delete_current_file('<?php echo $all_files[$i]; ?>')" >
								<img src="/images/delete.gif">
							</button>
						</td>
			<?php
					echo "</tr>";
				}
			?>
		</table>
		
		<?php
		if($page_count >= 2) {
			echo '<br/>';
			echo '<div class="pg" width="98%">';

			if( $cur_page > 1 ) {
				$page = $cur_page - 1;
				echo "<a title=\"";echo language('Previous page');echo "\" href=\"".$_SERVER['PHP_SELF']."?page=$page\" class=\"prev\"></a>";
			} else {
				
			}
				
			if($cur_page-3 > 1) {
				$s = $cur_page-4;
			} else {
				$s = 1;
			}
			if($s + 5 < $page_count) {
				$e = $s + 5; // the number of page flag = 5 + 1
			} else {
				$e = $page_count;
			}

			for($i = $s; $i <= $e; $i++) {
				if($i != $cur_page) {
					echo "<a href=\"".$_SERVER['PHP_SELF']."?page=$i\" >$i</a>";
				} else {
					echo "<strong>$cur_page</strong>";
				}
			}

			if( $cur_page < $page_count ) {
				$page = $cur_page + 1;
				echo "<a title=\"";echo language('Next page');echo "\" href=\"".$_SERVER['PHP_SELF']."?&page=$page\" class=\"nxt\" ></a>"; //redirective 
			} else {
				
			}

			echo "<label>";
			echo "<input type=\"text\" id=\"page\" name=\"page\" value=\"$cur_page\" size=\"2\" class=\"px\" title=\"";
			echo language('input page help','Please input your page number, and press [Enter] to skip to.');
			echo "\" onkeypress=\"gotopage(this.value)\" >";
			echo "<span title=\"";echo language('total pages');echo ": $page_count\"> / $page_count</span>";
			echo "</label>";
		?>
				<a title="<?php echo language('goto input page');?>" style="cursor:pointer;" onclick="goto_inputpage()"><?php echo language('go');?></a>
			</div>
		<?php
			}
		?>	
	</div>
	
	<div id="newline"></div>
	
	<input type="hidden" name="send" id="send" value="" />
	<input type="hidden" name="del_keyname" id="del_keyname" value="" />
	<input type="hidden" name="download_keyname" id="download_keyname" value="" />
	<input type="hidden" name="del_file_name" id="del_file_name" value="" />
	<table id="float_btn" class="float_btn">
		<tr id="float_btn_tr" class="float_btn_tr" style="">
			<td>
				<input type="submit" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';"/>
			</td>
		</tr>
	</table>
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2" style="">
			<td style="width:51px;">
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';" />
			</td>
		</tr>
	</table>
	
	<script>
		$(function(){
			$("#tls_enable").iButton();
			$("#tls_verify_server").iButton();
		});
		
		function gotopage(page){
			if(event.keyCode == 13) {
				event.keyCode = 0;  //Must set
				event.which = 0;
				window.location.href="<?php echo $_SERVER['PHP_SELF']."?&page="?>"+page;
			}
		}
		
		function goto_inputpage(){
			window.location.href="<?php echo $_SERVER['PHP_SELF']."?page="?>"+document.getElementById('page').value;
		}
		
		function addcheck(){
			var keyname = document.getElementById('keyname').value;
			var ip_address = document.getElementById('ip_address').value;
			var organization = document.getElementById('organization').value;
			var password = document.getElementById('password').value;
			
			document.getElementById('ccreate').innerHTML='';
			if(!check_diyname(keyname)) {
				document.getElementById('ccreate').innerHTML = con_str('<?php echo language('js check keyname', 'keyname you input is not valid');?>');
				return false;
			}
			if(!check_domain(ip_address)) {
				document.getElementById('ccreate').innerHTML = con_str('<?php echo language('js check ip_address', 'ip address you input is not valid');?>');
				return false;
			}
			if(!check_diyname(organization)){
				document.getElementById('ccreate').innerHTML = con_str('<?php echo language('js check organization', 'organization you input is not right');?>');
				return false;
			}
			if(!check_diypwd(password)) {
				document.getElementById('ccreate').innerHTML = con_str('<?php echo language('js check password', 'passwork you input is not valid');?>');
				return false;
			}
			return true;
		}
		
		function delete_key(key){
			var ret = confirm("<?php echo language('Delete confirm','Are you sure to delete you selected ?');?>");
			if(ret) {
				if(key)	{
					document.getElementById('del_keyname').value = key;
					return;
				} else {
					alert("failed to delete key: keyname is not right !");
					return false;
				}
			} else {
				return false;
			}
		}
		
		function delete_current_file(file_name){
			var ret = confirm("<?php echo language('Delete confirm','Are you sure to delete you selected ?');?>");
			if(ret) {
				if(file_name) {
					document.getElementById('del_file_name').value = file_name;
					return;
				} else {
					alert("Failed to delete the file: no found the file !");
					return false;
				}
			} else {
				return false;
			}
		}
		
		function download_key(key_num){
			if(key_num){
				document.getElementById('download_keyname').value = key_num;
				return true;
			} else {
				alert('failed to download the key: keyname is emtpy !');
				return false;
			}
		}
		
		var pem_flag = false;
		function check_pem_file_format(obj){
			var pos = obj.value.lastIndexOf('.');
			var file_suffix = obj.value.substring(pos+1);
			console.log('file_suffix ='+file_suffix);
			if(file_suffix == 'pem') {
				pem_flag = true;
				
			}
		}
		var crt_flag = false;
		function check_crt_file_format(obj){
			var pos = obj.value.lastIndexOf('.');
			var file_suffix = obj.value.substring(pos+1);
			if(file_suffix == 'crt') {
				crt_flag = true;
			}

		}
		function check_file_format() {
			if(!pem_flag){
				alert('The file you upload is not pem format, please upload file again.');
				return false;
			}
			if(!crt_flag){
				alert('The file you upload is not crt format, please upload file again.');
				return false;
			}
			return true; 
		}
	</script>
</form>

<?php }

function save_tlssetting(){
	if(trim($_POST['tls_enable']) == 'on'){
		$tlsenable = 'yes';
	} else {
		$tlsenable = 'no';
	}
	
	if(trim($_POST['port']) != '') {
		$port = trim($_POST['port']);
	} else {
		$port = '5061';
	}
	$tlsbindaddr = "0.0.0.0:".$port;
	
	if(trim($_POST['tls_verify_server']) == 'on'){
		$tls_verify_server = 'no';
	} else {
		$tls_verify_server = 'yes';
	}
	
	$tls_client_method = trim($_POST['tls_client_method']);
	
	$datachunk_tlssetting = '';
	$datachunk_tlssetting .= "tlsbindaddr=$tlsbindaddr\n";
	$datachunk_tlssetting .= "tlscertfile=/etc/asterisk/keys/asterisk.pem\n";
	$datachunk_tlssetting .= "tlscafile=/etc/asterisk/keys/ca.crt\n";
	$datachunk_tlssetting .= "tlscipher=ALL\n";
	$datachunk_tlssetting .= "tlsenable=$tlsenable\n";
	$datachunk_tlssetting .= "tlsdontverifyserver=$tls_verify_server\n";
	$datachunk_tlssetting .= "tlsclientmethod=$tls_client_method\n";

	$sip_general_tls_conf = "/etc/asterisk/sip_general_tls.conf";
	$flock = lock_file($sip_general_tls_conf);
	$fd = fopen($sip_general_tls_conf, "w");
	flock($fd, LOCK_EX | LOCK_NB);
	fwrite($fd, $datachunk_tlssetting);
	fclose($fd);
	
	wait_apply("exec","asterisk -rx \"sip reload\"");
}

function create_key(){
	$type = trim($_POST['type']);
	$keyname = trim($_POST['keyname']);
	$ip_address = trim($_POST['ip_address']);
	$organization = trim($_POST['organization']);
	$password = trim($_POST['password']);
	$section = $type."-".$keyname;
	
	$datachunk_key = '';
	$datachunk_key .= "type=$type\n";
	$datachunk_key .= "keyname=$keyname\n";
	$datachunk_key .= "ipaddress=$ip_address\n";
	$datachunk_key .= "organization=$organization\n";
	$datachunk_key .= "password=$password\n";

	if(!is_dir("/etc/asterisk/tls/")){
		mkdir("/etc/asterisk/tls",0777);
	}
	
	$keys_conf = "/etc/asterisk/tls/keys.conf";
	if(!file_exists($keys_conf)){
		touch($keys_conf);
	}
	$hlock = lock_file($keys_conf);
	
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/tls');
	if(!$aql->open_config_file($keys_conf)){
		echo $aql->get_error();
		return;
	}
	$aql->assign_addsection($section, $datachunk_key);
	if($type == "server"){
		exec("rm -rf /etc/asterisk/tls/ca.*");
		exec("rm -rf /etc/asterisk/tls/asterisk.*");	
		exec("rm -rf /etc/asterisk/tls/tmp.*");
		$cmd = "/my_tools/addkey_server.sh $ip_address $password $organization";
		exec($cmd, $output);
		if(strpos(end($output),"asterisk.pem")){
			if(!is_dir("/etc/asterisk/keys")){
				mkdir("/etc/asterisk/keys",0777);
			}
			$aql->save_config_file('keys.conf');
			unlock_file($hlock);
			$cmd = "cd /etc/asterisk/tls \n cp asterisk.pem ca.crt /etc/asterisk/keys";
			exec($cmd);
		} else {  // failed to create key
			echo "<script type=\"text/javascript\">alert('Please checking the server keys is exist or not and client password must be same with Server!!!')</script>";
		}
	} else {
		$cmd = "/my_tools/addkey_client.sh $ip_address $password $organization $keyname"; 
		exec($cmd, $output);
		if(strpos(end($output),$keyname.".pem")){
			$aql->save_config_file('keys.conf');
			unlock_file($hlock);
		} else {  // failed to create key
			echo "<script type=\"text/javascript\">alert('Please checking the server keys is exist or not and client password must be same with Server!!!')</script>";
		}
	}
}

function delete_key(){
	if(isset($_POST['del_keyname'])){
		$section = trim($_POST['del_keyname']);
	} else {
		$section = '';
	}
	if($section != ''){
		$section = trim($_POST['del_keyname']);
		$keys_conf = "/etc/asterisk/tls/keys.conf";
		$hlock = lock_file($keys_conf);
	
		$aql = new aql();
		$aql->set('basedir','/etc/asterisk/tls');
		if(!$aql->open_config_file($keys_conf)){
			echo $aql->get_error();
			return;
		}
		$aql->assign_delsection($section);
		$aql->save_config_file('keys.conf');
		
		$res = explode('-', $section, 2);
		$file_name = $res[1];
		if($file_name != ''){
			exec("rm -rf /etc/asterisk/tls/$file_name*");
		}
		if($res[0] == 'server') {
			exec("rm -rf /etc/asterisk/tls/ca.*");
			exec("rm -rf /etc/asterisk/tls/asterisk.*");		
			exec("rm -rf /etc/asterisk/tls/tmp.*");
		}
	}
}

function delete_file(){
	if(isset($_POST['del_file_name'])) {
		$file = trim($_POST['del_file_name']);
	} else {
		echo "<script type=\"text/javascript\">alert('failed to delete the file: No found the file!');</script>";
	}
	$del_cmd = "rm -rf /etc/asterisk/keys/$file";
	exec($del_cmd);
}

function download_key(){
	if(isset($_POST['download_keyname'])) {
		$file_name = trim($_POST['download_keyname']);
	} else {
		echo "<script type=\"text/javascript\">alert('failed to download the file: No found the file');</script>";
	}
	$key_file_name = $file_name.".tar.gz";
	$key_file_path = '/etc/asterisk/tls/'.$key_file_name;
	$tar_cmd = "cd /etc/asterisk/tls \n tar vcz -f ".$key_file_name." ".$file_name.".pem ca.crt";
	
	exec("$tar_cmd > /dev/null 2>&1 || echo $?",$output);
	
	// if($output) {
		// echo "</br>$key_file_name ";
		// echo language("Packing was failed");echo "</br>";
		// echo language("Error code");echo ": ".$output[0];
		// return;
	// }
	
	if(!file_exists($key_file_path)){
		echo "<br> $key_file_name :";
		echo language('can not find');
		return;
	}
	$fd = fopen($key_file_path, "r");
	$size = filesize($key_file_path);
	
	header('Content-Encoding: none');
	header('Content-Type: application/force-download');
	header('Content-Type: application/octet-stream');
	header('Content-Type: application/download');
	header('Content-Description: File Transfer');
	header('Accept-Ranges: bytes');
	header( "Accept-Length: $size");
	header( 'Content-Transfer-Encoding: binary' );
	header( "Content-Disposition: attachment; filename=$key_file_name" );
	header('Pragma: no-cache');
	header('Expires: 0');
	
	ob_clean();
	flush();
	echo fread($fd, $size);
	fclose ($fd);
	unlink($key_file_name);
}

function upload_keys(){
	$dest_path = "/etc/asterisk/keys/";
	
	if(!dir($dest_path)){
		mkdir("/etc/asterisk/keys",0777);
	}
	
	$count = count($_FILES['key_file']['name']);
    for ($i = 0; $i < $count; $i++) {
		$tmpfile = $_FILES['key_file']['tmp_name'][$i];
		$dest_file = $dest_path.$_FILES['key_file']['name'][$i];
		if(!move_uploaded_file($tmpfile, $dest_file)){
			echo "<script type=\"text/javascript\">alert('Failed to upload the $tmpfile<br>');</script>";
		}
	}
}

if($_POST) {
	if (isset($_POST['send']) && $_POST['send'] == "Save") {
		save_tlssetting();
		show_security();
	} else if(isset($_POST['send']) && $_POST['send'] == "Add"){
		create_key();
		show_security();
	} else if(isset($_POST['send']) && $_POST['send'] == "Delete"){
		delete_key();
		show_security();
	} else if(isset($_POST['send']) && $_POST['send'] == "Download"){
		download_key();
		show_security();
	} else if(isset($_POST['send']) && $_POST['send'] == "Upload"){
		upload_keys();
		show_security();
	} else if(isset($_POST['send']) && $_POST['send'] == "Delete_File"){
		delete_file();
		show_security();
	} else {
		show_security();
	}
		
} else {
	show_security();
}

require("/www/cgi-bin/inc/boot.inc");
?>

<div id="to_top"></div>
<div id="float_btn1" class="sec_float_btn1"></div>
<div class="float_close" onclick="close_btn()" ></div>