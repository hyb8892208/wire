<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");
include_once("/www/cgi-bin/inc/define.inc");
?>

<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/float_btn.js"></script>
<script type="text/javascript">
function onload_func(){
	$("#all-sw").iButton();
	advanced_ami_change();
}

function advanced_ami_change(){
	var sw = document.getElementById('all-sw').checked;
	
	if (sw) {
		set_visible('serialNumber_ctl',true);
		set_visible('emuPassword_ctl',true);
		set_visible('simbankIP_ctl',true);
		set_visible('serverPort_ctl',true);
		set_visible('heartbeatInt_ctl',true);
		set_visible('net_mode_tr',true);
		if(document.getElementById('net_mode').value == 'OPlink'){
			$(".switch_show").show();
		}
	
	} else {
		set_visible('serialNumber_ctl',false);
		set_visible('emuPassword_ctl',false);
		set_visible('simbankIP_ctl',false);
		set_visible('serverPort_ctl',false);
		set_visible('heartbeatInt_ctl',false);
		set_visible('net_mode_tr',false);
		$(".switch_show").hide();
	}	
}

</script>

<?php
if($_POST) {	
	if( isset($_POST['send']) && $_POST['send'] == 'Save') {	
		save_para_conf();
	}
}
show_simemu_settings();

function save_para_conf()
{
    $aql = new aql();
    $simemusvr_conf = "/etc/asterisk/simemusvr.conf";
	
    $hlock = lock_file($simemusvr_conf);
    if (!file_exists($simemusvr_conf)) {
        fclose(fopen($simemusvr_conf,"w"));
    }
    $aql->set('basedir','/etc/asterisk');
	
    if(!$aql->open_config_file($simemusvr_conf)){
        echo $aql->get_error();
        unlock_file($hlock);
        return ;
    }   
	
    $res = $aql->query("select * from simemusvr.conf");
    unlock_file($hlock);

    if(!$res){
        echo $aql->get_error();
        return;
    }

    if (!isset($res['SimEmuSvr'])) {
		$aql->assign_addsection('SimEmuSvr','');
    }
	
	if (isset($_POST['all-sw'])) {
		$simemusvr_switch = 'yes';
	} else {
		$simemusvr_switch = 'no';
	}
	if (isset($res['SimEmuSvr']['simemusvr_switch'])) {
		$aql->assign_editkey('SimEmuSvr','simemusvr_switch',$simemusvr_switch );
	}else{
		$aql->assign_append('SimEmuSvr','simemusvr_switch',$simemusvr_switch );
	}
	
	if (isset($res['SimEmuSvr']['seri'])) {
		$aql->assign_editkey('SimEmuSvr','seri',$_POST['serialNumber'] );
	}else{
		$aql->assign_append('SimEmuSvr','seri',$_POST['serialNumber'] );
	}
	
	if(isset($res['SimEmuSvr']['net_mode'])){
		$aql->assign_editkey('SimEmuSvr','net_mode',$_POST['net_mode']);
	}else{
		$aql->assign_append('SimEmuSvr','net_mode',$_POST['net_mode']);
	}
	
	if($_POST['net_mode'] == 'OPlink'){
		$server_ip = '10.150.210.1';
	}else{
		$server_ip = $_POST['simbankIP'];
	}
	if (isset($res['SimEmuSvr']['server_ip'])) {
		$aql->assign_editkey('SimEmuSvr','server_ip',$server_ip);
	}else{
		$aql->assign_append('SimEmuSvr','server_ip',$server_ip);
	}	

	if($_POST['net_mode'] == 'normal'){
		//$local_ip = exec("/my_tools/net_tool eth0 |sed -n '4P'");
		$local_ip = $_POST['net_vpn'];
	}else{
		$local_ip = '10.150.210.'.$_POST['node_id'];
	}
	if (isset($res['SimEmuSvr']['local_ip'])) {
		$aql->assign_editkey('SimEmuSvr','local_ip',$local_ip );
	}else{
		$aql->assign_append('SimEmuSvr','local_ip',$local_ip );
	}
	
	//OPlink
	 if (!isset($res['OPlink'])) {
		$aql->assign_addsection('OPlink','');
    }
	
	if($_POST['net_mode'] == 'OPlink'){
		$switch = 'on';
	}else{
		$switch = 'off';
	}
	if($simemusvr_switch == 'no'){
		$switch = 'off';
	}
	if(isset($res['OPlink']['switch'])){
		$aql->assign_editkey('OPlink','switch',$switch);
	}else{
		$aql->assign_append('OPlink','switch',$switch);
	}
	
	if(isset($res['OPlink']['node_id'])){
		$aql->assign_editkey('OPlink','node_id',$_POST['node_id']);
	}else{
		$aql->assign_append('OPlink','node_id',$_POST['node_id']);
	}
	
	if(isset($res['OPlink']['seri'])){
		$aql->assign_editkey('OPlink','seri',$_POST['simbank_serial_number']);
	}else{
		$aql->assign_append('OPlink','seri',$_POST['simbank_serial_number']);
	}
	
	if($_POST['node_server'] == 'china'){
		$node_server = 'oplink-cn01.openvox.cn:1024';
	}else if($_POST['node_server'] == 'america'){
		$node_server = 'oplink-us01.openvox.cn:1024';
	}else if($_POST['node_server'] == 'europe'){
		$node_server = 'oplink-eur01.openvox.cn:1024';
	}else{
		$customize_server = $_POST['customize_server'];
		if(strstr($customize_server, '://')){
			$temp = explode("://", $customize_server);
			$temp = explode('/', $temp[1]);
			$node_server = $temp[0];
		}else{
			$temp = explode('/', $customize_server);
			$node_server = $temp[0];
		}
		$node_server = trim($node_server);
	}
	if(isset($res['OPlink']['node_server'])){
		$aql->assign_editkey('OPlink','node_server',$node_server);
	}else{
		$aql->assign_append('OPlink','node_server',$node_server);
	}
	
	if (!$aql->save_config_file('simemusvr.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	
	clean_up_OPlink_status();

	if($simemusvr_switch == $res['SimEmuSvr']['simemusvr_switch']){
		$cmd = "sh /etc/init.d/simemu.sh restart > /dev/null 2>&1 &";
	}else{
		$cmd = "sh /etc/init.d/simemu.sh switch > /dev/null 2>&1 &";
	}
	wait_apply("exec", "sh /etc/init.d/OPlink restart");
	wait_apply("exec", $cmd);
}

function clean_up_OPlink_status(){
	if(file_exists('/tmp/OPlink.status')){
		file_put_contents('/tmp/OPlink.status', '');
	}
}

function show_simemu_settings()
{
	$aql = new aql();
    $simemusvr_conf = "/etc/asterisk/simemusvr.conf";
    $hlock = lock_file($simemusvr_conf);
    if (!file_exists($simemusvr_conf)) {
        fclose(fopen($simemusvr_conf,"w"));
    }
    //$aql->set('basedir','/etc/asterisk');
	$aql->set('basedir','/etc/asterisk');
    if(!$aql->open_config_file($simemusvr_conf)){
        echo $aql->get_error();
        unlock_file($hlock);
        return ;
    }   

    $res = $aql->query("select * from simemusvr.conf");
    unlock_file($hlock);

    if(!$res){
        echo $aql->get_error();
        return;
    }
	
	//simemusvr switch
	if(isset($res['SimEmuSvr']['simemusvr_switch'])) {
		$val = $res['SimEmuSvr']['simemusvr_switch'];
		if ($val == 'yes') {
			$Simemusvr_sw = 'checked';
		}
		else {
			$Simemusvr_sw = '';
		}
	} else {
		$Simemusvr_sw = '';
	}
	
	if(isset($res['SimEmuSvr']['seri'])) {
		$serial_number = $res['SimEmuSvr']['seri'];
	} else {
		$serial_number = '';
	}
	
	if(isset($res['SimEmuSvr']['net_mode'])){
		$net_mode = $res['SimEmuSvr']['net_mode'];
	}else{
		$net_mode = 'normal';
	}
	
	$server_ip_js = '';
	if(isset($res['SimEmuSvr']['server_ip'])) {
		$server_ip = $res['SimEmuSvr']['server_ip'];
		$server_ip_js = $res['SimEmuSvr']['server_ip'];
	} else {
		$server_ip = '';
	}
	if($net_mode == 'OPlink'){
		$server_ip = 1;
	}
	
	//OPlink
	if(isset($res['OPlink']['node_id'])){
		$node_id = $res['OPlink']['node_id'];
	}else{
		$node_id = '';
	}
	
	if(isset($res['OPlink']['seri'])){
		$simbank_serial_number = $res['OPlink']['seri'];
	}else{
		$simbank_serial_number = '';
	}
	
	if(isset($res['OPlink']['node_server'])){
		$node_server = $res['OPlink']['node_server'];
	}else{
		$node_server = '';
	}
	if($node_server == 'oplink-cn01.openvox.cn:1024'){
		$server_mode = 'china';
	}else if($node_server == 'oplink-us01.openvox.cn:1024'){
		$server_mode = 'america';
	}else if($node_server == 'oplink-eur01.openvox.cn:1024'){
		$server_mode = 'europe';
	}else{
		$server_mode = 'customize';
	}
	
	$local_ip = $res['SimEmuSvr']['local_ip'];
	
	$setok = $aql->set('basedir','/etc/asterisk/gw/network');
	$hlock = lock_file('/etc/asterisk/gw/network/lan.conf');
	$lan_res = $aql->query("select * from lan.conf");
	unlock_file($hlock);
	
?>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post" id="emuForm">
        <div id="tab">                                                                                           
                <li class="tb1">&nbsp;</li>                                                         
                <li class="tbg"><?php echo language('Simbank Options');?></li>                      
                <li class="tb2">&nbsp;</li>                                                         
        </div>  
	<table width="100%" class="tedit">
		<tr>
            <th>
                <div class="helptooltips">
                    <?php echo language('Simemusvr Switch');?>:
                    <span class="showhelp">
                        <?php echo language('Simemusvr Switch help','Control Simemusvr (open/close).Open:means change all to remote mode;Close:change all to local mode.');?>
                    </span>
                </div>
            </th>
			<td colspan=" 10">
				<input type="checkbox" id="all-sw" name="all-sw" <?php echo $Simemusvr_sw; ?>  onchange="advanced_ami_change()"  />
			</td>
		</tr>
		
		<tr id= "serialNumber_ctl" style = "display:none;">
			<th>
				<div class="helptooltips">                        
					<?php echo language('Serial Number');?>:  
					<span class="showhelp">                   
					<?php echo language('Serial Number help','Serial Number');?>
					</span>                                   
				</div> 
			</th>
			<td>
				<input type="text" name="serialNumber" id="serialNumber" maxlength="10" readonly style="background-color:#EBEBE4;border:1px solid #ccc;" value="<?php echo $serial_number;?>" />
			</td>
		</tr>
		
		<tr id="net_mode_tr" style="display:none;">
			<th border="0" style="border:none;">
				<div class="helptooltips">
					<?php echo language('Net Mode');?>:
					<span class="showhelp">
					<?php echo language('Net Mode help','Net Mode'); ?>
					</span>
				</div>
			</th>
			<td border="0" style="border:none;">
				<select id="net_mode" name="net_mode">
					<option value="normal" <?php if($net_mode == 'normal') echo 'selected';?>>Normal</option>
					<option value="OPlink" <?php if($net_mode == 'OPlink') echo 'selected';?>>OPlink</option>
				</select>
				
				<select id="net_vpn" name="net_vpn" style="display:none;">
					<?php
						unset($output);
						exec("/my_tools/net_tool eth0 2> /dev/null && echo ok",$output);
						if(isset($output[11]) && $output[11] == 'ok' && isset($output[1]) && $output[1] == 'Enable' && $output[3] != '') {
					?>
					<option value="<?php echo $output[3];?>" <?php if($local_ip == $output[3]) echo 'selected';?>>eth0</option>
					<?php } ?>
					
					<?php
						unset($output);
						exec("/my_tools/net_tool eth1 2> /dev/null && echo ok",$output);
						if(isset($output[11]) && $output[11] == 'ok' && isset($output[1]) && $output[1] == 'Enable' && $output[3] != '') {
					?>
					<option value="<?php echo $output[3];?>" <?php if($local_ip == $output[3]) echo 'selected';?>>eth1</option>
					<?php } ?>
					
					<?php
						unset($output);
						exec("/my_tools/net_tool ppp0 2> /dev/null && echo ok", $output);
						if(isset($output[11]) && $output[11] == 'ok' && isset($output[1]) && $output[1] == 'Enable' && $output[3] != '') {
					?>
					<option value="<?php echo $output[3];?>" <?php if($local_ip == $output[3]) echo 'selected';?>>ppp0</option>
					<?php } ?>
					
					<?php
						unset($output);
						exec("/my_tools/net_tool edge0 2> /dev/null && echo ok", $output);
						if(isset($output[11]) && $output[11] == 'ok' && isset($output[1]) && $output[1] == 'Enable' && $output[3] != ''){
					?>
					<option value="<?php echo $output[3];?>" <?php if($local_ip == $output[3]) echo 'selected';?>>N2N</option>
					<?php } ?>
					
					<?php
						unset($output);
						exec("/my_tools/net_tool tun0 2> /dev/null && echo ok", $output);
						if(isset($output[11]) && $output[11] == 'ok' && isset($output[1]) && $output[1] == 'Enable' && $output[3] != ''){
					?>
					<option value="<?php echo $output[3];?>" <?php if($local_ip == $output[3]) echo 'selected';?>>tun0</option>
					<?php } ?>
				</select>
			</td>
		</tr>
		
		<tr  id= "simbankIP_ctl" style = "display:none;">
			<th>
				<div class="helptooltips">                        
					<span id="simbank_server_ip"><?php echo language('Simbank Server IP');?>:</span>
					<span class="showhelp">
					<?php echo language('Simbank Server IP help','Simbank Server IP/Node');?>
					</span>                               
				</div> 
			</th>
			<td>
				<input type="text" name="simbankIP" id="simbankIP" value="<?php echo $server_ip;?>" />
			</td>
		</tr>
	</table>
	
	<br/>
	
	<div class="switch_show">
		<div id="tab">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('OPlink Settings');?></li>
			<li class="tb2">&nbsp;</li>
		</div>
		
		<table width="100%" class="tedit" >
			<tr id="field_lan_ipaddr" >
				<th>
					<div class="helptooltips">
						<?php echo language('Node ID');?>:
						<span class="showhelp">
						<?php echo language('Node ID help','Node ID');?>
						</span>
					</div>
				</th>
				<td >
					<select name="node_id" id="node_id">
						<?php 
						for($i=2;$i<=20;$i++){
						?>
						<option value="<?php echo $i;?>" <?php if($node_id == $i) echo 'selected';?>><?php echo $i;?></option>
						<?php } ?>
					</select>
				</td>
			</tr>
			
			<tr>
				<th><?php echo language('Simbank Serial Number');?></th>
				<td >
					<input type="text" name="simbank_serial_number" id="simbank_serial_number" value="<?php echo $simbank_serial_number;?>" /><span id="csimbank_serial_number"></span>
				</td>
			</tr>
			
			<tr>
				<th><?php echo language('OPlink Node Server');?></th>
				<td >
					<select name="node_server" id="node_server" onchange="server_change()">
						<option value="china" <?php if($server_mode == 'china') echo 'selected';?>><?php echo language('China');?></option>
						<option value="america" <?php if($server_mode == 'america') echo 'selected';?>><?php echo language('America');?></option>
						<option value="europe" <?php if($server_mode == 'europe') echo 'selected';?>><?php echo language('Europe');?></option>
						<option value="customize" <?php if($server_mode == 'customize') echo 'selected';?>><?php echo language('Customize');?></option>
					</select>
					<input style="margin-left:10px;" size="30px" type="text" name="customize_server" id="customize_server" value="<?php echo $node_server;?>" />
				</td>
			</tr>
		</table>
		
		<br/>
	
		<div class="switch_show">
			<div id="tab">
				<li class="tb1">&nbsp;</li>
				<li class="tbg">
					<div class="helptooltips">
						<?php echo language('Connection Status');?>
						<span class="showhelp">
						<?php echo language('Connection Status help','');?>  
						</span>
					</div>
				</li>
				<li class="tb2">&nbsp;</li>
			</div>
				
			<table width="100%" class="tedit" >
				<tr>
					<th><?php echo language('Connection Status');?>:</th>
					<td id="connection_status">
					
					</td>
				</tr>
			</table>
		</div>
	</div>
	
	<?php 
	$product_type = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys product_type");
	if($product_type == 5 || $product_type == 6){
	?>
	<br/>
	
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Simbank Version');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	
	<table width="100%" class="tedit" >
		<tr id="field_lan_ipaddr" >
			<th>
				<div class="helptooltips">
					<?php echo language("Version");?>:
					<span class="showhelp">
					<?php echo language("Version");?>
					</span>
				</div>
			</th>
			<td style="padding:5px 10px;">
				<?php
				$aql = new aql();
				$aql->set('basedir', '/tmp/');
				$res = $aql->query("select * from hw_info.cfg");
				
				$content = file_get_contents("/tmp/.emu_status");
				$temp = explode("\n",$content);
				
				$str = '';
				$n = 1;
				$p = 1;
				for($i=0;$i<count($temp);$i++){
					if($temp[$i] == "") continue;
					
					$tmp = explode("version:",$temp[$i]);
					$tmp1 = explode(" ",$tmp[1]);
					$tmp_str = $tmp1[1].' '.$tmp1[2].' '.$tmp1[3].' '.$tmp1[4];
					
					if(strstr($temp[$i],'emu') && strstr($temp[$i],'DETECTED') && !strstr($temp[$i],'UNDETECTED')){
						$str .= '<span style="line-height:18px;">emu-'.$n.' '.$tmp_str.'</span><br/>';
						$n++;
					}else if(!strstr($temp[$i],'emu')){
						if($p == 9) $p = 1;
						$str .= '<span style="line-height:16px;margin-left:10px;">port-'.$p.' '.$tmp_str.'</span><br/>';
						$p++;
					}else if(strstr($temp[$i],'emu') && strstr($temp[$i],'UNDETECTED')){
						$n++;
					}
				}
				
				echo $str;
				?>
			</td>
		</tr>
	</table>
	<?php } ?>
	
	<br/>
	
	<script>
	if(document.getElementById('net_mode').value == 'OPlink'){
		$(".switch_show").show();
		$("#simbank_server_ip").html("<?php echo language("Simbank Server Node");?>");
		$("#simbankIP").attr("readonly",'readonly');
		$("#simbankIP").css({'background-color':'#EBEBE4','border':'1px solid #ccc'});
		$("#simbankIP").val("1");
	}else{
		$(".switch_show").hide();
		$("#simbank_server_ip").html("<?php echo language("Simbank Server IP");?>");
		$("#simbankIP").removeAttr("readonly");
		$("#simbankIP").css({'background-color':'#fff','border':'1px solid #ccc'});
		$("#simbankIP").val("<?php echo $server_ip_js;?>");
	}
	
	onload_func();

	$("#net_mode").change(function(){
		if($(this).val() == 'OPlink'){
			$(".switch_show").show();
			$("#simbank_server_ip").html("<?php echo language("Simbank Server Node");?>");
			$("#simbankIP").attr("readonly",'readonly');
			$("#simbankIP").css({'background-color':'#EBEBE4','border':'1px solid #ccc'});
			$("#simbankIP").val("1");
		}else{
			$(".switch_show").hide();
			$("#simbank_server_ip").html("<?php echo language("Simbank Server IP");?>");
			$("#simbankIP").removeAttr("readonly");
			$("#simbankIP").css({'background-color':'#fff','border':'1px solid #ccc'});
			$("#simbankIP").val("<?php echo $server_ip_js;?>");
		}
	});
	
	var time = 0;
	function connection_status(){
		$.ajax({
			type:'GET',
			url: '/cgi-bin/php/ajax_server.php?type=connection_status',
			success: function(data){
				console.log(data);
				<?php if(isset($_POST['send']) && $_POST['send'] == 'Save'){?>
				if(data.indexOf('success') != -1){
					$("#connection_status").html("<span style='color:green'><?php echo language('Connected');?></span>");
				}else if(data == '' && time < 10){
					setTimeout('connection_status()',1000);
					time++;
					$("#connection_status").html('<img src="/images/loading.gif"/>');
				}else if(time >= 10){
					$("#connection_status").html("<span style='color:red'><?php echo language('Connect Timeout');?></span>");
				}else{
					$("#connection_status").html("<span style='color:red'><?php echo language('Connect Failed');?></span>");
				}
				<?php }else{?>
				if(data.indexOf('success') != -1){
					$("#connection_status").html("<span style='color:green'><?php echo language('Connected');?></span>");
				}else{
					$("#connection_status").html("<span style='color:red'><?php echo language('Connect Failed');?></span>");
				}
				<?php }?>
			}
		});
	}

	connection_status();
	
	function server_change(){
		var server_mode = $("#node_server").val();
		var old_server_mode = "<?php echo $server_mode; ?>";
		var node_server = "<?php echo $node_server; ?>";
		
		if(server_mode != 'customize'){
			$("#customize_server").hide();
		}else{
			if(old_server_mode == 'customize'){
				$("#customize_server").val(node_server);
			}else{
				$("#customize_server").val('');
			}
			$("#customize_server").show();
		}
	}
	server_change();
	
	function check(){
		var ipv4 = '<?php echo $lan_res['ipv4']['ipaddr']; ?>';
		var netmask = '<?php echo $lan_res['ipv4']['netmask']; ?>';
		
		var ipv4_temp = ipv4.split('.');
		var netmask_temp = netmask.split('.');
		
		var ipv4_netmask_str = (ipv4_temp[0]&netmask_temp[0])+'.'+(ipv4_temp[1]&netmask_temp[1])+'.'+(ipv4_temp[2]&netmask_temp[2]);
		var default_netmask_str = (10&netmask_temp[0])+'.'+(150&netmask_temp[1])+'.'+(210&netmask_temp[2]);
		
		if(document.getElementById('all-sw').checked){
			if(ipv4_netmask_str == default_netmask_str){
				alert("<?php echo language('Connect OPlink help','The current network environment does not support OPlink. Please modify the IP segment and subnet mask before using it.');?>");
				return false;
			}
		}
		
		return true;
	}
	
	$(function(){
		var net_mode = document.getElementById('net_mode').value;
		
		if(net_mode == 'normal'){
			$("#net_vpn").show();
		}
	});
	
	$("#net_mode").change(function(){
		if($(this).val() == 'normal'){
			$("#net_vpn").show();
		}else{
			$("#net_vpn").hide();
		}
	});
	</script>
<?php	
}
?>

	<input type="hidden" name="send" id="send" value="" />
	<table id="float_btn" class="float_btn">
		<tr id="float_btn_tr" class="float_btn_tr">
			<td>
				<input type="submit"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" />
			</td>
		</tr>
	</table>

	</form>

<?php require("/www/cgi-bin/inc/boot.inc");?>
<div id="float_btn1" class="float_btn1 sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()">
</div>
