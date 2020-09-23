<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");
?>

<!---// load jQuery and the jQuery iButton Plug-in //---> 
<!--<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.3.2/jquery.min.js"></script> -->
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
 
<!---// load the iButton CSS stylesheet //---> 
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />


<?php
function is_valid_pwd($str)
{
	if(preg_match('/^[0-9\+]{4,16}$/',$str)) {
		return true;
	}

	return false;
}


function save_cluster_conf()
{
	global $__BRD_SUM__;
	global $__BRD_HEAD__;

	if(isset($_POST['mode'])) {
		$_mode = $_POST['mode'];
	} else {
		$_mode = 'stand_alone';
	}
	
	// add master board-numbers option
	if ($_mode == 'master') {
		if (isset($_POST['board_num'])) {
			$_board_num = $_POST['board_num'];
		} else {
			$_board_num = '';
		}
	} else {
		$_board_num = 1;
	}

	if(isset($_POST['master_password'])) {
		$_master_password = trim($_POST['master_password']);
	} else {
		$_master_password = '';
	}

	if(isset($_POST['master_ip'])) {
		$_master_ip = trim($_POST['master_ip']);
	} else {
		$_master_ip = '';
	}

	if(isset($_POST['slave_password'])) {
		$_slave_password = trim($_POST['slave_password']);
	} else {
		$_slave_password = '';
	}

	if(isset($_POST['slave_masterip']) && ($_mode == 'slave')) {
		$_slave_masterip = trim($_POST['slave_masterip']);
	} else {
		$_slave_masterip = '';
	}

	if(isset($_POST['slave_ip'])) {
		$_slave_ip = trim($_POST['slave_ip']);
	} else {
		$_slave_ip = '';
	}

	if(isset($_POST['remain_ori_ip'])) {
		$_remain_ori_ip = 1;
	} else {
		$_remain_ori_ip = 0;
	}

	switch($_mode) {
	case 'master':
		if(!is_valid_pwd($_master_password)) {
			echo language('Master Mode warning 1',"Please input valid password number 0-9 ,min:4 max:16");
			return false;
		}
		if(!is_ip($_master_ip)) {
			echo language('Master Mode warning 2',"Please input valid Master IP address.");
			return false;
		}
		break;
	case 'slave':
		if(!is_valid_pwd($_slave_password)) {
			echo language('Slave Mode warning 1',"Please input valid password number 0-9 ,min:4 max:16");
			return false;
		}
		if(!is_ip($_slave_ip)) {
			echo language('Slave Mode warning 2',"Please input valid Slave IP address.");
			return false;
		}
		if(!is_ip($_slave_masterip)) {
			echo language('Slave Mode warning 3',"Please input valid Master IP address.");
			return false;
		}
		break;
	}

	$_slave_list = '';
	$_slave_ori_list = '';
	if ($_mode != 'stand_alone') {
		for($b=2; $b<=$__BRD_SUM__; $b++) {
			if( isset($_POST[$__BRD_HEAD__.$b.'_ip']) && isset($_POST[$__BRD_HEAD__.$b.'_ori_ip']) && $b<=$_board_num ) {
				$_ip = trim($_POST[$__BRD_HEAD__.$b.'_ip']);
				$_ori_ip = trim($_POST[$__BRD_HEAD__.$b.'_ori_ip']);
				if($_ip != '' && $_ori_ip != '') {
					if(!is_ip($_ip)) {
						echo language('slave list warning 1',"Please input valid target IP address of board");
						echo $b;
						return false;
					}
					if(!is_ip($_ori_ip)) {
						echo language('slave list warning 2',"Please input valid original IP address of board");
						echo $b;
						return false;
					}

					$_slave_list .= $__BRD_HEAD__.$b.'_ip='.$_ip."\n";
					$_slave_ori_list .= $__BRD_HEAD__.$b.'_ori_ip='.$_ori_ip."\n";
				}
			}
		}
	}

	$write = <<<EOF
[general]
mode=$_mode
boardnum=$_board_num

[slave]
password=$_slave_password
ip=$_slave_ip
masterip=$_slave_masterip
remain_ori_ip=$_remain_ori_ip

[master]
password=$_master_password
ip=$_master_ip

[slavelist]
$_slave_list
$_slave_ori_list

EOF;
//EOF

	$cfg_file = "/etc/asterisk/gw/cluster.conf"; 
	$hlock = lock_file($cfg_file);
	$fh=fopen($cfg_file,"w");
	fwrite($fh,$write);
	fclose($fh);
	unlock_file($hlock);

	return true;
}

?>

<?php
$detail_checked = '';
$detail_show = '';
$cluster_save_mode = '';
if($_POST && isset($_POST['send'])) {
	if($_POST['send'] == 'manualcluster') {
		$detail_checked = 'checked';
		$detail_show = 'display:block;';
		if(save_cluster_conf()) {
			$cluster_save_mode = 'manualcluster';
		} else {
			echo "Save cluster configuration files failed!";
		}
	} else if($_POST['send'] == 'autocluster') {
		$cluster_save_mode = 'autocluster';
	}
}


$info = get_cluster_info();

$mode_select['master'] = '';
$mode_select['slave'] = '';
$mode_select['stand_alone'] = '';
switch($info['mode']) {
	case 'master': $mode_select['master'] = 'selected'; break;
	case 'slave': $mode_select['slave'] = 'selected'; break;
	default: $mode_select['stand_alone'] = 'selected'; break;
}

for ($b=2;$b<=$__BRD_SUM__;$b++) {
	$board_num_select[$b] = '';
}

if (isset($info['boardnum']) && $info['boardnum'] != '') {
	$b_sel = $info['boardnum'];
	$board_num_select[$b_sel] = 'selected';
} else {
	$board_num_select[11] = 'selected';
}

if($info['remain_ori_ip'] == '0') {
	$remain_ori_ip_check = '';
} else {
	$remain_ori_ip_check = 'checked';
}

?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript">
function check()
{
	return true;
}

function ClassicSettings()
{
	var type = document.getElementById('mode').value;

	var slotnum = <?php echo get_slotnum(); ?>

	if(type == 'slave') {
		document.getElementById('slave_password').value = '9999';
		document.getElementById('slave_ip').value = '192.168.9.' + slotnum;
		document.getElementById('slave_masterip').value = '192.168.9.1';
	} else if (type == 'master') {
		document.getElementById('master_password').value = '9999';
		document.getElementById('master_ip').value = '192.168.9.1';
<?php 
		$ipinfo = get_lanip_info();
		if($ipinfo['ip'] != '') {
			echo "localip = '".$ipinfo['ip']."';\n";
		} else {
			echo "localip = '172.16.99.1';\n";
		}

		echo "numip = ip2num(localip);\n";

		for($b=2; $b<=$__BRD_SUM__; $b++) {
			echo "document.getElementById('".$__BRD_HEAD__.$b.'_ip'."').value = '192.168.9.".$b."';\n";
			echo "document.getElementById('".$__BRD_HEAD__.$b.'_ori_ip'."').value = num2ip(numip+$b-1);\n";
		}
?>
	}
}

function modechange()
{
	var type = document.getElementById('mode').value;

	if(type == 'slave') {
		set_visible('tr_master_boards',false);
		set_visible('tr_master_password', false);
		set_visible('tr_slave_password', true);
		set_visible('tr_master_ip', false);
		set_visible('tr_slave_masterip', true);
		set_visible('tr_slave_ip', true);
		set_visible('tr_slaveip_list', false);
		set_visible('td_classic_settings',true);
		set_visible('tr_remain_ori_ip',true);
	} else if (type == 'master') {
		set_visible('tr_master_boards',true);
		set_visible('tr_master_password', true);
		set_visible('tr_slave_password', false);
		set_visible('tr_master_ip', true);
		set_visible('tr_slave_masterip', false);
		set_visible('tr_slave_ip', false);
		set_visible('tr_slaveip_list', true);
		set_visible('td_classic_settings',true);
		set_visible('tr_remain_ori_ip',true);
		board_num_change();
	} else {
		set_visible('tr_master_boards',false);
		set_visible('tr_master_password', false);
		set_visible('tr_slave_password', false);
		set_visible('tr_master_ip', false);
		set_visible('tr_slave_masterip', false);
		set_visible('tr_slave_ip', false);
		set_visible('tr_slaveip_list', false);
		set_visible('td_classic_settings',false);
		set_visible('tr_remain_ori_ip',false);
	}
}
function board_num_change()
{
	var total_board = "<?php echo $__BRD_SUM__;?>";
	var boardnum = document.getElementById('board_num').value;
	for (var i=2;i<=total_board;i++) {
		set_id = 'slave_list' + i;
		if (i <= boardnum) {
			set_visible(set_id,true);
		} else {
			set_visible(set_id,false);
		}
	}
}

function masterip_onchange()
{
	master_ip = document.getElementById('master_ip').value;
	numip = ip2num(master_ip);
<?php 
	for($b=2; $b<=$__BRD_SUM__; $b++) {
		echo "ip=numip+$b-1;\n";
		echo "document.getElementById('".$__BRD_HEAD__.$b.'_ip'."').value = num2ip(ip);\n";
	}
?>
}

function onload_func()
{
	modechange();
}

</script>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<input type="hidden" name="send" id="send" value="" />
	<div id="tab" style="height:30px;">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Working Mode');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<div width="100%" class="div_setting_c">
		<div class="divc_setting_v">
			<table width='100%' class="tedit" style="border:none">
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Action');?>:
						<span class="showhelp">
						<?php echo language('Action help');?>
						</span>
					</div>
				</th>
				<td>
					<input type="submit" value="<?php echo language('Automatic Cluster');?>" onclick="document.getElementById('send').value='autocluster';" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Detail');?>:
						<span class="showhelp">
						<?php echo language('Detail help');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" id="detail_enable" onchange="$('#detail').slideToggle();" <?php echo $detail_checked;?> />
				</td>
			</tr>
			</table>
		</div>
		<div id='detail' class='div_setting_d' style="position:relative;top:-2px;<?php echo $detail_show;?>">
			<table width='100%' class="tedit" style="border:none">
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Mode');?>:
							<span class="showhelp">
							<?php echo language('Mode help@system-cluster','
								Stand-alone Mode: <br>
								&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Run alone, total 4 ports<br>
								Master Mode: <br>
								&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Run as master with two different IP, controlling up to four slaves. <br>
								&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;The master can be accessed by the original IP. The target IP is used to communicate with the slaves. <br>
								Slave Mode: <br>
								&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Run as slave with two different IP, controlled by the master. <br>
								&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;If the original IP is forbidden, the slave can be accessed by the master with inward IP only. <br>');
							?>
							</span>
						</div>
					</th>
					<td >
						<table cellpadding="0" cellspacing="0">
							<tr>
								<td>
									<select id="mode" name="mode" onchange="modechange()">
										<option  value="stand_alone" <?php echo $mode_select['stand_alone']; ?> ><?php echo language('Stand-alone');?></option>
										<option  value="master" <?php echo $mode_select['master']; ?> ><?php echo language('Master');?></option>
										<option  value="slave" <?php echo $mode_select['slave']; ?> ><?php echo language('Slave');?></option>
									</select>
								</td>
								<td id="td_classic_settings" style="display: none;">
									<input type="button" name="classic_settings" id="classic_settings" value="<?php echo language('Set Default');?>" onclick="ClassicSettings()"/>
								</td>
							</tr>
						</table>
					</td>
				</tr>
				
				<tr id="tr_master_boards" style="display: none;">
					<th>
						<div class="helptooltips">
							<?php echo language('Cluster Number');?>:
							<span class="showhelp">
							<?php echo language('Cluster Number help','Set the number of cluster.');
							?>
							</span>
						</div>
					</th>
					<td >
						<table cellpadding="0" cellspacing="0">
							<tr>
								<td>
									<select id="board_num" name="board_num" onchange="board_num_change()">
										<?php 
										for ($b=2;$b<=$__BRD_SUM__;$b++) {
										?>
										<option value="<?php echo $b;?>" <?php echo $board_num_select[$b];?> ><?php echo $b;?></option>	
										<?php
										}
										?>
									</select>
								</td>
							</tr>
						</table>
					</td>
				</tr>
				
				<tr id="tr_master_password" style="display: none;">
					<th>
						<div class="helptooltips">
							<?php echo language('Password');?>:
							<span class="showhelp">
							<?php echo language('Password help@system-cluster master mode','The Master Mode password. <br>Must be 4~16bits digital 0-9.');?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" name="master_password" id="master_password" value="<?php echo $info['master_password'] ?>" />
					</td>
				</tr>
				<tr id="tr_slave_password" style="display: none;">
					<th>
						<div class="helptooltips">
							<?php echo language('Password');?>:
							<span class="showhelp">
							<?php echo language('Password elp@system-cluster slave mode',"The slave's Master Mode password. <br>Must be the same as the master.");?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" name="slave_password" id="slave_password" value="<?php echo $info['slave_password'] ?>" />
					</td>
				</tr>
				<tr id="tr_master_ip" style="display: none;">
					<th>
						<div class="helptooltips">
							<?php echo language('Master IP');?>(<?php echo language('Local IP');?>):
							<span class="showhelp">
							<?php echo language('Master IP help@system-cluster master mode',"
								The master's target IP. <br>
								Must be set in the subnet different from external subnet, <br>
								so that the external subnet couldn't access internal subnet.");
							?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" name="master_ip" id="master_ip" value="<?php echo $info['master_ip'] ?>" onchange="masterip_onchange()" />
					</td>
				</tr>
				<tr id="tr_slave_masterip" style="display: none;">
					<th>
						<div class="helptooltips">
							<?php echo language('Master IP');?>(<?php echo language('Target IP');?>):
							<span class="showhelp">
							<?php echo language('Master IP help@system-cluster slave mode',"
								The master's target IP. <br>
								You must set the master's target IP first.");
							?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" name="slave_masterip" id="slave_masterip" value="<?php echo $info['slave_masterip']?>" />
					</td>
				</tr>
				<tr id="tr_slave_ip" style="display: none;">
					<th>
						<div class="helptooltips">
							<?php echo language('Slave IP');?>(<?php echo language('Local IP');?>):
							<span class="showhelp">
							<?php echo language('Slave IP help@system-cluster slave mode',"
								The slave's target IP. <br>
								Must be set in the same subnet as the master.");
							?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" name="slave_ip" id="slave_ip" value="<?php echo $info['slave_ip']?>" />
					</td>
				</tr>
				<tr id="tr_slaveip_list" style="display: none;">
					<th>
						<div class="helptooltips">
							<?php echo language('Slaves IP List');?>:
							<span class="showhelp">
							<?php echo language('Slaves IP List help@system-cluster master mode',"
								Set the slaves's original and target IP. <br>
								The original IP is outward IP, and the target IP is inward IP. <br>
								Up to four slaves.");
							?>
							</span>
						</div>
					</th>
					<td>
						<table>
						<?php for($b=2; $b<=$__BRD_SUM__; $b++) { ?>
							<?php echo "<tr id=\"slave_list".$b."\" style=\"display: none;\" >";?>
							
								<td>
									<?php echo $__BRD_HEAD__.$b; ?>
								</td>
								<td>
									<?php echo language('Original IP');?>:
								</td>
								<td>
									<?php echo "<input type=\"text\" name=\"".$__BRD_HEAD__.$b.'_ori_ip'."\" id=\"".$__BRD_HEAD__.$b.'_ori_ip'."\" value=\"".$info[$__BRD_HEAD__.$b.'_ori_ip']."\"/>"; ?>
								</td>
								<td>
									<?php echo language('Target IP');?>:
								</td>
								<td>
									<?php echo "<input type=\"text\" name=\"".$__BRD_HEAD__.$b.'_ip'."\" id=\"".$__BRD_HEAD__.$b.'_ip'."\" value=\"".$info[$__BRD_HEAD__.$b.'_ip']."\"/>"; ?>
								</td>
							</tr>
						<?php } ?>
						</table>
					</td>
				</tr>
				<tr id="tr_remain_ori_ip" style="display: none;">
					<th>
						<div class="helptooltips">
							<?php echo language('Remain Original IP address');?>:
							<span class="showhelp">
							<?php echo language('Remain Original IP address help');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" id="remain_ori_ip" name="remain_ori_ip" <?php echo $remain_ori_ip_check ?> />
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Action');?>:
							<span class="showhelp">
							<?php echo language('Action help2');?>
							</span>
						</div>
					</th>
					<td>
						<input type="submit" value="<?php echo language('Manual Cluster');?>" onclick="document.getElementById('send').value='manualcluster';return check();" />
					</td>
				</tr>
			</table>
		</div>
	</div>
	</form>

<script type="text/javascript">
//document.body.onload = function(){onload_func();}
$("#detail_enable").iButton();
$("#remain_ori_ip").iButton();
onload_func();
</script>

<!--
<script type="text/javascript"> 
/*$(document).ready(function (){ 
  $("#remain_ori_ip").iButton();
}); */
</script>
-->

<?php
ob_flush();
flush();
function ping($ip)
{
	$socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	if(!$socket) return false;

	socket_set_option($socket,SOL_SOCKET,SO_RCVTIMEO,array("sec"=>1, "usec"=>0 ) );
	socket_set_option($socket,SOL_SOCKET,SO_SNDTIMEO,array("sec"=>1, "usec"=>0 ) );

	$connection = @socket_connect($socket, $ip, 8000);  
	if(!$connection) return false;  
		  
	@socket_write($socket, "ping\n");
	
	$all_buf = '';
	while ($buffer = @socket_read($socket, 1024, PHP_NORMAL_READ)) { 
		$all_buf .= $buffer;
	}

	@socket_close($socket);

	if(strcmp($all_buf,"ping\n") == 0) return true;

	return false;
}

function send_msg($ip,$msg)
{
	$socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	if(!$socket) return false;

	$connection = @socket_connect($socket, $ip, 8000);  
	if(!$connection) return false;  
		  
	@socket_write($socket, "$msg\n");
	@socket_close($socket);

	return true;
}

//Auto cluster
/////////////////////////////////////////////////////////////
function list_all_gws()
{
	$sflag = '$Server';
	$eflag = '------';
	$str = `/my_tools/gwping eth0 list 2> /dev/null`;

	while($str) {
		$find = strstr($str,$sflag);
		if(!$find) break;

		$pos = strpos($find,$eflag);
		if(!$pos) break;

		$section = substr($find,0,$pos);
		if(!$section) break;

		$allsection[] = $section;

		$str = substr($find,$pos);
	}

	if(!isset($allsection)) return false;

	$i=0;
	foreach( $allsection as $section ) {
		$line = explode("\n",$section);
		if(is_array($line)) {
			$mac = '';
			$productname = '';
			$slotnum = '';
			$ip = '';
			foreach($line as $each) {
				if(($pos = strpos($each,'$Server '))!==false) {
					$mac = trim(substr($each,$pos+strlen('$Server ')));
				} else if(($pos = strpos($each,'serverinfo:productname='))!==false) {
					$productname = trim(substr($each,$pos+strlen('serverinfo:productname=')));
				} else if(($pos = strpos($each,'serverinfo:slotnum='))!==false) {
					$slotnum = trim(substr($each,$pos+strlen('serverinfo:slotnum=')));
				} else if(($pos = strpos($each,'serverinfo:ip='))!==false) {
					$ip = trim(substr($each,$pos+strlen('serverinfo:ip=')));
				}
			}

			if( $mac && $productname && $slotnum && $ip ) {
				$allgws[$i]['mac'] = $mac;
				$allgws[$i]['productname'] = $productname;
				$allgws[$i]['slotnum'] = $slotnum;
				$allgws[$i]['ip'] = $ip;
				$slotnums[$i] = $slotnum;
				$i++;
			}
		}
	}

	if(isset($allgws)) {
		array_multisort($slotnums, SORT_ASC, $allgws);
		return $allgws;
	}

	return false;
}


function find_valid_gws($master_slotnum, $allgws)
{
	global $__BRD_SUM__;

	foreach($allgws as $gw) {
		if( $gw['slotnum'] >= 1 && $gw['slotnum'] <= $__BRD_SUM__ && $gw['productname'] == 'OpenVox 4 port GSM Gateway') {

			$slotnum = $gw['slotnum'];

			if($master_slotnum == $slotnum) { //Skip same master slot number gateways.
				continue;
			}

			if(isset($ret[$slotnum]['mac'])) {
				return "Please turn off the other gateways while scanning the network.";
				return false;
			}

			$ret[$slotnum]['mac'] = $gw['mac'];
			$ret[$slotnum]['productname'] = $gw['productname'];
			$ret[$slotnum]['slotnum'] = $gw['slotnum'];
			$ret[$slotnum]['ip'] = $gw['ip'];
		}
	}

	if(isset($ret)) return $ret;

	return "Not found gateway.";
}


function random_ip($how)
{
	$s3 = rand(10,254);
	$s4 = rand(1,254-$how);

	for($i=0; $i<$how; $i++) {
		$tmp = $s4 + $i;
		$ret[$i] = "192.168.$s3.$tmp";
	}

	return $ret;
}

function random_pwd()
{
	return rand(1000,999999);
}

function save_master_cfg($masterip, $pwd, $ips, $gws, $remain_ori_ip='1')
{
	global $__BRD_HEAD__;

	$_slave_list = '';
	$_slave_ori_list = '';
	$i = 1;
	foreach($gws as $gw) {
		$b = $gw['slotnum'];
		$ip = $ips[$i++];
		$ori_ip = $gw['ip'];
		$_slave_list .= $__BRD_HEAD__.$b.'_ip='.$ip."\n";
		$_slave_ori_list .= $__BRD_HEAD__.$b.'_ori_ip='.$ori_ip."\n";
	}

	$write = <<<EOF
[general]
mode=master

[master]
password=$pwd
ip=$masterip

[slave]
remain_ori_ip=$remain_ori_ip

[slavelist]
$_slave_list
$_slave_ori_list

EOF;
//EOF

	$cfg_file = "/etc/asterisk/gw/cluster.conf"; 
	//$hlock = lock_file($cfg_file);
	$fh=fopen($cfg_file,"w");
	fwrite($fh,$write);
	fclose($fh);
	//unlock_file($hlock);

	//Freedom 2014-02-14 22:16 Don't run in background.
	system("/my_tools/cluster_mode > /dev/null 2>&1"); 
}


function save_slave_cfg($mac, $slaveip, $pwd, $masterip, $remain_ori_ip=1)
{
	$cfg_content = "'[general]\\nmode=slave\\n[slave]\\npassword=$pwd\\nip=$slaveip\\nmasterip=$masterip\\nremain_ori_ip=$remain_ori_ip\\n'";
	$cmd = "\"echo -e $cfg_content > /etc/asterisk/gw/cluster.conf\"";
	system("/my_tools/gwping eth0 $cmd $mac");

	//Freedom 2014-02-14 22:16 Don't run in background.
	$cmd = "/my_tools/cluster_mode";
	system("/my_tools/gwping eth0 $cmd $mac > /dev/null 2>&1"); 
}
/////////////////////////////////////////////////////////////



function auto_cluster()
{
	echo "<br/>";
	echo "<b>".language("Automatic Cluster")."</b><hr/>";
	echo "<font color='red'>Automatically clustering!! Please don't leave this page till it finished...</font>";
	ob_flush();
	flush();

	$master_slotnum = get_slotnum();
	if(!is_numeric($master_slotnum)) {
		echo "<font color=ff0000>Get master slot number failed.<br/>Automatic cluster failed!!!</font>";
		return;
	}

	$master_ip_info = get_lanip_info();

	$allgws = list_all_gws();

	if(!$allgws) {
		echo "<font color=ff0000>List gateways failed.<br/>Automatic cluster failed!!!</font>";
		return;
	}

	$gws = find_valid_gws($master_slotnum,$allgws);
	if(!is_array($gws)) {
		echo "<table style='width:100%;font-size:12px;border:1px solid rgb(59,112,162);border-collapse:collapse;'>";
		echo "<tr style='background-color:#D0E0EE;border:1px solid rgb(59,112,162);height:26px;'>";
		echo "<th>Board</th>";
		echo "<th>MAC</th>";
		echo "<th>IP</th>";
		echo "</tr>";

		foreach($allgws as $each) {
			echo "<tr align='center' style='background-color:rgb(232, 239, 247);border:1px solid rgb(59,112,162);'>";

			echo "<td>";
			echo $each['slotnum'];
			echo "</td>";

			echo "<td>";
			echo $each['mac'];
			echo "</td>";

			echo "<td>";
			echo $each['ip'];
			echo "</td>";
			
			echo "</tr>";
		}
		echo "</table>";

		echo "<font color=ff0000>$gws</font><br/>";
		echo "<font color=ff0000>Automatic cluster failed!!!</font>";
		return;
	}

	$gw_count = count($gws) + 1;

	$ips = random_ip($gw_count);
	$pwd = random_pwd();

	$masterip = $ips[0];

	echo "<table style='width:100%;font-size:12px;border:1px solid rgb(59,112,162);border-collapse:collapse;'>";
	echo "<tr style='background-color:#D0E0EE;border:1px solid rgb(59,112,162);height:26px;'>";
	echo "<th >Name</th>";
	echo "<th >Board</th>";
	echo "<th >MAC</th>";
	echo "<th >Original IP</th>";
	echo "<th >Set IP</th>";
	echo "</tr>";

	echo "<tr align='center' style='background-color:rgb(232, 239, 247);border:1px solid rgb(59,112,162);'>";
	echo "<td>Master</td>";
	echo "<td>".$master_slotnum."</td>";
	echo "<td>".strtolower($master_ip_info['mac'])."</td>";
	echo "<td>".$master_ip_info['ip']."</td>";
	echo "<td>$masterip</td>";
	echo "</tr>";

	//Setting master
	///////////////////////////////////////////////
	save_master_cfg($masterip, $pwd, $ips, $gws, 0);
	//save_master_cfg($masterip, $pwd, $ips, $gws, 1);
	//restart lua program
	exec("cd /my_tools/lua/ && ./execute_lua.sh > /dev/null 2>&1");
	///////////////////////////////////////////////

	//Setting slave
	///////////////////////////////////////////////
	$i = 1;
	foreach($gws as $gw) {

		echo "<tr align='center' style='background-color:rgb(232, 239, 247);border:1px solid rgb(59,112,162);'>";
		echo "<td>Slave</td>";
		echo "<td>".$gw['slotnum']."</td>";
		echo "<td>".$gw['mac']."</td>";
		echo "<td>".$gw['ip']."</td>";
		echo "<td>".$ips[$i]."</td>";
		echo "</tr>";

		save_slave_cfg($gw['mac'],$ips[$i],$pwd,$masterip,0);
		//save_slave_cfg($gw['mac'],$ips[$i],$pwd,$masterip,1);
		//set_slave_file($ips[$i],"/etc/asterisk/gw/httpd.conf","/etc/asterisk/gw/httpd.conf");//for slave web authentication when system update.
		set_slave_file($ips[$i],"/etc/asterisk/gw/lighttpdpassword_digest","/etc/asterisk/gw/lighttpdpassword_digest");//for slave web authentication when system update.
		set_slave_file($ips[$i],"/etc/asterisk/gw/lighttpd_https.conf","/etc/asterisk/gw/lighttpd_https.conf");

		//Restart web server in here. Freedom 2014-02-14 22:16
		request_slave($ips[$i],"syscmd:/etc/init.d/lighttpd restart > /dev/null 2>&1 &",5,false);
		request_slave($ips[$i],"syscmd:/my_tools/lua/execute_lua.sh > /dev/null 2>&1 &",5,false);
		$i++;
	}
	///////////////////////////////////////////////

	echo "</table>";

	echo "<font color=008800>Automatic cluster successful.</font><br>";
	//
	exec("cd /my_tools/lua/info_access/ && lua save_module_type.lua > /dev/null 2>&1 ");
}

function manual_cluster()
{
	global $info;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;

	echo "<br/>";
	trace_output_start(language('Manual Cluster'),language('Report'),true);

	ob_flush();
	flush();

	exec("/my_tools/cluster_mode > /dev/null 2>&1");
	save_routings_to_extensions();
	ast_reload();
	exec("cd /my_tools/lua/ && ./execute_lua.sh > /dev/null 2>&1");
	if($info['mode'] == 'master') {
		$masterip = $info['master_ip'];
		$remain_ori_ip = $info['remain_ori_ip'];

		for($b=2; $b<=$__BRD_SUM__; $b++) {
			if(isset($info[$__BRD_HEAD__.$b.'_ip']) && $info[$__BRD_HEAD__.$b.'_ip'] != '') {
				trace_output_newline();
				//$ori_ip = "172.16.99.$b";   //Original IP address.
				$ori_ip = $info[$__BRD_HEAD__.$b.'_ori_ip'];   //Original IP address.
				$tar_ip = $info[$__BRD_HEAD__.$b.'_ip'];   //Target IP address.
				$tar_pwd = $info['master_password'];       //Target Password (SIP extension)

				if(ping($ori_ip)) {
					//echo "<br/>";
					echo "<font color='red'>Manual clustering!! Please don't leave this page till it finished...</font><br>";
					ob_flush();
					flush();
					echo "$ori_ip is alive...<br/>";
					echo "set $ori_ip to $tar_ip <br/>";
					$msg = "mode=slave;password=$tar_pwd;masterip=$masterip;slaveip=$tar_ip;remain_ori_ip=$remain_ori_ip;";
					send_msg($ori_ip,$msg);        //The Target board will be change IP address
					ob_flush();
					flush();
					for($c=1; $c<=20; $c++) {
						if(ping($tar_ip)) {
							echo "Set $tar_ip OK<br/>";
							//set_slave_file($tar_ip,"/etc/asterisk/gw/httpd.conf","/etc/asterisk/gw/httpd.conf");//for slave web authentication when system update.
							set_slave_file($tar_ip,"/etc/asterisk/gw/lighttpdpassword_digest","/etc/asterisk/gw/lighttpdpassword_digest");//for slave web authentication when system update.
							set_slave_file($tar_ip,"/etc/asterisk/gw/lighttpd_https.conf","/etc/asterisk/gw/lighttpd_https.conf");

							//Restart web server in here. Freedom 2014-02-14 22:16
							request_slave($tar_ip,"syscmd:/etc/init.d/lighttpd restart > /dev/null 2>&1 &",5,false);
							request_slave($tar_ip,"syscmd:/my_tools/lua/execute_lua.sh > /dev/null 2>&1 &",5,false);
							break;
						}
						usleep(200000);
					}
					if($c>20){
						echo "Set $tar_ip Failed<br/>";
					}
				} else {
					echo "$ori_ip is not found...<br/>";
				}
			}
		}
	} else {
		trace_output_newline();
		echo "Set to ".$info['mode']." OK<br/>";
	}

	trace_output_end();
	
	
	exec("cd /my_tools/lua/info_access/ && lua save_module_type.lua > /dev/null 2>&1 ");
}

function show_cluster_info()
{
	$info = get_cluster_info();
	$all_version_info = read_redis_info();
	global $info;
	exec("sleep 2");
	if($info['mode'] == 'master' || $info['mode'] == 'stand_alone') {
?>
	<br>
	
	<div id="tab" style="height:30px;">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Cluster Informations');?></li>
		<li class="tb2">&nbsp;</li>
	</div>	
			<table width="100%" class="tshow"> 
					<tr  id="title_cmp" height="40px"  >
						<td style="background-color:#D0E0EE" ><?php echo language('Board Name');?></td>
						<td style="background-color:#D0E0EE" ><?php echo language('Model Name');?></td>
						<td style="background-color:#D0E0EE" ><?php echo language('Modem Description');?></td>
						<td style="background-color:#D0E0EE" ><?php echo language('Software Version');?></td>
						<td style="background-color:#D0E0EE" ><?php echo language('Hardware Version');?></td>
						<td style="background-color:#D0E0EE" ><?php echo language('Build Time');?></th>
					</tr>
				<?php 
				//$all_version_info = read_redis_info();
				foreach ($all_version_info as $key=>$version_info){
					if ($version_info['error'] == '') {
				?>
						<tr id="<?php echo $version_info['name'];?>" height="25px">
							<td ><?php echo $version_info['name'];?></td>
							<td ><?php echo get_module_info('name',$version_info['local.product.module.type']);?></td>
							<td ><?php echo get_module_info('modem',$version_info['local.product.module.type']);?></td>
							<td ><?php echo $version_info['local.product.sw.version'];?></td>
							<td ><?php echo $version_info['local.product.board.version'];?></td>
							<td ><?php echo $version_info['local.product.sw.buildtime'];?></td>
						</tr>
				<?php
					} else {
						if ($version_info['error'] == 'disconnected') {
				?>
							<tr id="<?php echo $key;?>" height="25px"  >
								<td ><?php echo $key;?></td>
								<td colspan="5"  >Disconnected.</td>
							</tr>
				<?php						
						} else {							
				?>
							<tr id="<?php echo $key;?>" height="25px"  >
								<td ><?php echo $key;?></td>
								<td colspan="5"  >Incompatible version: Click <a href="http://downloads.openvox.cn/pub/firmwares/GSM%20Gateway/wg400-mid.img">here</a> to get the compatible version 2.2.2.</td>
							</tr>							
				<?php
						}
					}
				}	
				?>	
			</table> 
<?php	}
	
}


switch($cluster_save_mode) {
case 'manualcluster':
	manual_cluster();
	break;

case 'autocluster':
	auto_cluster();
	break;
}

if ($cluster_save_mode != 'autocluster') {
	show_cluster_info();
}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>
<?php
ob_flush();flush();
if($cluster_save_mode == 'manualcluster' || $cluster_save_mode == 'autocluster'){
	//webserver_restart();
	exec("/etc/init.d/lighttpd restart > /dev/null 2>&1 &");
}

?>
