<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/define.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");
include_once("/www/cgi-bin/inc/language.inc");
?>

<?php
require_once("/www/cgi-bin/inc/language.inc");
$language = get_web_language_cache("/tmp/web/language.cache");

?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
	<head>
	<meta charset="utf-8">
	<meta name="viewport" content="width=device-width, initial-scale=1">
	<title>Automatic Modify GSM IMEI</title>
	<link rel="icon" href="/images/logo.ico" type="image/x-icon">
	<link rel="shortcut icon" href="/images/logo.ico" type="image/x-icon">
	<link rel="stylesheet"  href="/css/style.css" />
	</head>
	<script src="/js/js.js"></script>
	<script src="/js/jquery.js"></script>
	<body>

	<div id="bg_blank">
	<div>

<!--// load jQuery and the jQuery iButton Plug-in //--> 
<!--<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.3.2/jquery.min.js"></script> -->
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<!---// load the iButton CSS stylesheet //--> 
<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />
<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>
<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>

<?php

function save_to_autoimei_conf($board) {
/* 
Location: /etc/asterisk/gw/autoimei.conf
[general] 
sw=on
interval=60
channel=1,2,3,4
immediately=yes/no
force=yes/no

[1]
imei=35xxxx0xxxxxxx

[2]
imei=35xxxx0xxxxxxx

[3]
imei=35xxxx0xxxxxxx

[4]
imei=35xxxx0xxxxxxx

*/
	global $__GSM_SUM__;

	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk/gw');
	if (!$setok) {
		echo $aql->get_error();
		return false;
	}

	$autoimei_conf_path = '/etc/asterisk/gw/autoimei.conf';

	if(!file_exists($autoimei_conf_path)) {
		fclose(fopen($autoimei_conf_path,"w"));
	}

	$hlock = lock_file($autoimei_conf_path);

	if(!$aql->open_config_file($autoimei_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}

	$exist_array = $aql->query("select * from autoimei.conf");

	if(!isset($exist_array['general'])) {
		$aql->assign_addsection('general','');
	}

	/* autoimei_sw */
	if(isset($_POST['autoimei_sw'])) {
		$val = 'on';
	} else {
		$val = 'off';
	}
	if(isset($exist_array['general']['sw'])) {
		$aql->assign_editkey('general','sw',$val);
	} else {
		$aql->assign_append('general','sw',$val);
	}
	
	/* channel */
	$val = "";
	if(isset($_POST['spans']["$board"]) && is_array($_POST['spans']["$board"])){
		foreach($_POST['spans']["$board"] as $span){	//span
			$val .= "$span,";
		}
	}
	$val = rtrim($val,',');
	if(isset($exist_array['general']['channel'])) {
		$aql->assign_editkey('general','channel',$val);
	} else {
		$aql->assign_append('general','channel',$val);
	}

	/* interval */
	if(isset($_POST['interval'])) {
		if(!preg_match('/^[0-9]*$/', $_POST['interval']))
			return false;
		$val = trim($_POST['interval']);
		if(isset($exist_array['general']['interval'])) {
			$aql->assign_editkey('general','interval',$val);
		} else {
			$aql->assign_append('general','interval',$val);
		} 
	}

	/* immediately */
	if(isset($_POST['immediately'])) 
		$val = "yes";
	else
		$val = "no";

	if(isset($exist_array['general']['immediately'])) {
		$aql->assign_editkey('general','immediately',$val);
	} else {
		$aql->assign_append('general','immediately',$val);
	} 

	/* force */
	if(isset($_POST['force'])) 
		$val = "yes";
	else
		$val = "no";

	if(isset($exist_array['general']['force'])) {
		$aql->assign_editkey('general','force',$val);
	} else {
		$aql->assign_append('general','force',$val);
	} 

	/* imei */
	for($c=1;$c<=$__GSM_SUM__;$c++){
		$tag_tac = "${board}${c}_imei_tac";
		$tag_fac = "${board}${c}_imei_fac";
		$tag_snr = "${board}${c}_imei_snr";
		if(isset($_POST[$tag_tac])&&isset($_POST[$tag_fac])&&isset($_POST[$tag_snr])) {
			$imei_tac = $_POST[$tag_tac];
			$imei_fac = $_POST[$tag_fac];
			$imei_snr = $_POST[$tag_snr];
			if(!preg_match('/^[0-9xX]{6}$/',$imei_tac) || !preg_match('/^[0-9xX]{2}$/', $imei_fac) || !preg_match('/^[0-9xX]{6}$/', $imei_snr) )
				return false;
			$val = trim($imei_tac).trim($imei_fac).trim($imei_snr);
			if(!isset($exist_array[$c])) {
				$aql->assign_addsection($c,'');
			}
			if(isset($exist_array[$c]['imei'])) {
				$aql->assign_editkey($c,'imei',$val);
			} else {
				$aql->assign_append($c,'imei',$val);
			}   
		} 

	}

	/**********************************/
	if (!$aql->save_config_file('autoimei.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);

	return true;
}

if($_POST && isset($_POST['send']) && $_POST['send'] == 'Save') {
	
	if($__deal_cluster__){
		$cluster_info = get_cluster_info();
		if($cluster_info['mode'] == 'master') {
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
					if(save_to_autoimei_conf($b)){
						set_slave_file($slaveip,"/etc/asterisk/gw/autoimei.conf","/etc/asterisk/gw/autoimei.conf");
						wait_apply("request_slave", $slaveip, "syscmd:/etc/init.d/autoimei restart > /dev/null 2>&1 &");
					}
				}    
			}    
		}
	}
	
	
	$board = get_slotnum(); 
	if(save_to_autoimei_conf($board)){
		wait_apply("exec", "/etc/init.d/autoimei restart > /dev/null 2>&1 &");
	}
}
?>

<?php
$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$res = $aql->query("select * from autoimei.conf");

$autoimei_sw = 'off';
if(isset($res['general']['sw'])) {
	$autoimei_sw = trim($res['general']['sw']);
}

$interval = '3600';
if(isset($res['general']['interval'])) {
	$interval = trim($res['general']['interval']);
}

$immediately = 'yes';
if(isset($res['general']['immediately'])) {
	$immediately = trim($res['general']['immediately']);
}

$force = 'no';
if(isset($res['general']['force'])) {
	$force = trim($res['general']['force']);
}

$channel_conf = '';
if(isset($res['general']['channel'])) {
	$channel_conf = trim($res['general']['channel']);
}
$b= 1;
for($c=1; $c<=$__GSM_SUM__; $c++) {
	$span_checked[1][$c] = "";
	$imei = '';
	if(isset($res[$c]['imei'])) {
		$imei = trim($res[$c]['imei']);
	}
	${$b.$c."_imei_tac"} = substr($imei, 0, 6);
	${$b.$c."_imei_fac"} = substr($imei, 6, 2);
	${$b.$c."_imei_snr"} = substr($imei, 8, 6);
}
$channel_array = explode(",", "$channel_conf");
foreach($channel_array as $channel){
	if($channel>=1 && $channel<=$__GSM_SUM__){
		$span_checked[1][$channel] = "checked";
	}
}

/* get slave channel imei if possible */
if($__deal_cluster__){
	$aql->set('basedir','/tmp');
	$cluster_info = get_cluster_info();
	if($cluster_info['mode'] == 'master') {
		for($b=2; $b<=$__BRD_SUM__; $b++) {
			if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
				//get_slave_file();
				$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
				get_slave_file($slaveip,"/etc/asterisk/gw/autoimei.conf");
				$res = $aql->query("select * from $slaveip-autoimei.conf");
				$channel_conf = '';
				if(isset($res['general']['channel'])) {
					$channel_conf = trim($res['general']['channel']);
				}
				for($c=1; $c<=$__GSM_SUM__; $c++) {
					$span_checked[$b][$c] = "";
					$imei = '';
					if(isset($res[$c]['imei'])) {
						$imei = trim($res[$c]['imei']);
					}
					${$b.$c."_imei_tac"} = substr($imei, 0, 6);
					${$b.$c."_imei_fac"} = substr($imei, 6, 2);
					${$b.$c."_imei_snr"} = substr($imei, 8, 6);
				}
				$channel_array = explode(",", "$channel_conf");
				foreach($channel_array as $channel){
					if($channel>=1 && $channel<=$__GSM_SUM__){
						$span_checked[$b][$channel] = "checked";
					}
				}
			}
		}
	}
}

show_modify_imei_dialog();

?>
	<script type="text/javascript">
	var select_all_flag = false;
	function select_all()
	{
		var inputs = document.getElementsByTagName("input");     
		for(var i=0;i<inputs.length;i++){
			if(inputs[i].getAttribute("type") == "checkbox" && 
				inputs[i].getAttribute("class") == "port"){     
				if(select_all_flag == false)
					inputs[i].checked = true;
				else
					inputs[i].checked = false;
			}
		}
		if(select_all_flag == false)
			select_all_flag = true;
		else
			select_all_flag = false;
	}

	function set_to_all()
	{
		var imei_tac = $("#imei_tac").attr("value");
		var imei_fac = $("#imei_fac").attr("value");
		var imei_snr = $("#imei_snr").attr("value");

		for(var i=0;i<=<?php $b=1; echo $__GSM_SUM__;?>;i++){
			$("#<?php echo $b;?>"+i+"_imei_tac").attr("value", imei_tac);
			$("#<?php echo $b;?>"+i+"_imei_fac").attr("value", imei_fac);
			$("#<?php echo $b;?>"+i+"_imei_snr").attr("value", imei_snr);
		}
		<?php
		if($__deal_cluster__){
			if($cluster_info['mode'] == 'master') {
				for($b=2; $b<=$__BRD_SUM__; $b++) {
					if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
			?>
			for(var i=0;i<=<?php echo $__GSM_SUM__;?>;i++){
				$("#<?php echo $b;?>"+i+"_imei_tac").attr("value", imei_tac);
				$("#<?php echo $b;?>"+i+"_imei_fac").attr("value", imei_fac);
				$("#<?php echo $b;?>"+i+"_imei_snr").attr("value", imei_snr);
			}
			<?php
					}
				}
			}
		}
		?>
	}

	function check(id)
	{
		var obj = document.getElementById(id);
		obj.value = obj.value.replace(/[^xX\d]*/g,'');
	}

	function check_all()
	{
		/* check port */
		var span_flag = false;
		var inputs = document.getElementsByTagName("input");     
		for(var i=0;i<inputs.length;i++){
			if(inputs[i].getAttribute("type") == "checkbox" && inputs[i].getAttribute("class") == "port"){     
				if(inputs[i].checked == true){
					span_flag = true;
					break;
				}
			}
		}
		if(span_flag == false){
			document.getElementById('cport').innerHTML = con_str('<?php echo language('js select at least one port','You must select at least on port.');?>');
			return false;
		}else{
			document.getElementById('cport').innerHTML = "";
		}

		/* check interval */
		var interval = document.getElementById('interval');
		if(interval.value == "" || interval.value < 360 || interval.value > 31536000){
			document.getElementById('cinterval').innerHTML = con_str('<?php echo language('js check integer','Please input integer number');echo '(360~31536000).';?>');
			return false;
		}else{
			document.getElementById('cinterval').innerHTML = "";
		}

		/* check imei */
		var rex_6bit=/^[xX\d]{6}$/i;
		var rex_2bit=/^[xX\d]{2}$/i;
		var imei_tac = "";
		var imei_fac = "";
		var imei_snr = "";
		for(var i=1;i<=<?php $b=get_slotnum(); echo $__GSM_SUM__;?>;i++){
			var slotnum = <?php echo get_slotnum(); ?>;
			imei_tac = $("#<?php echo $b;?>"+i+"_imei_tac").attr("value");
			if(!rex_6bit.test(imei_tac)){
				$("#cimei_"+slotnum+"_"+i).html(con_str("<?php echo language('js imei tac help','The input TAC should be 6 bits 0~9 or x or X');?>"));
				return false;
			}else{
				$("#cimei_"+slotnum+"_"+i).html("");
			}
			imei_fac = $("#<?php echo $b;?>"+i+"_imei_fac").attr("value");
			if(!rex_2bit.test(imei_fac)){
				$("#cimei_"+slotnum+"_"+i).html(con_str("<?php echo language('js imei fac help','The input FAC should be 2 bits 0~9 or x or X');?>"));
				return false;
			}else{
				$("#cimei_"+slotnum+"_"+i).html("");
			}
			imei_snr = $("#<?php echo $b;?>"+i+"_imei_snr").attr("value");
			if(!rex_6bit.test(imei_snr)){
				$("#cimei_"+slotnum+"_"+i).html(con_str("<?php echo language('js imei snr help','The input SNR should be 6 bits 0~9 or x or X');?>"));
				return false;
			}else{
				$("#cimei_"+slotnum+"_"+i).html("");
			}
		}
		<?php
		if($__deal_cluster__){
			if($cluster_info['mode'] == 'master') {
				for($b=2; $b<=$__BRD_SUM__; $b++) {
					if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
			?>
			for(var i=1;i<=<?php echo $__GSM_SUM__;?>;i++){
				imei_tac = $("#<?php echo $b;?>"+i+"_imei_tac").attr("value");
				if(!rex_6bit.test(imei_tac)){
					$("#cimei_<?php echo $b?>"+"_"+i).html(con_str("<?php echo language('js imei tac help','The input TAC should be 6 bits 0~9 or x or X');?>"));
					return false;
				}else{
					$("#cimei_<?php echo $b?>"+"_"+i).html("");
				}
				imei_fac = $("#<?php echo $b;?>"+i+"_imei_fac").attr("value");
				if(!rex_2bit.test(imei_fac)){
					$("#cimei_<?php echo $b?>"+"_"+i).html(con_str("<?php echo language('js imei fac help','The input FAC should be 2 bits 0~9 or x or X');?>"));
					return false;
				}else{
					$("#cimei_<?php echo $b?>"+"_"+i).html("");
				}
				imei_snr = $("#<?php echo $b;?>"+i+"_imei_snr").attr("value");
				if(!rex_6bit.test(imei_snr)){
					$("#cimei_<?php echo $b?>"+"_"+i).html(con_str("<?php echo language('js imei snr help','The input SNR should be 6 bits 0~9 or x or X');?>"));
					return false;
				}else{
					$("#cimei_<?php echo $b?>"+"_"+i).html("");
				}
			}
			<?php
					}
				}
			}
		}
		?>

		return true;
	}

	$(document).ready(function()
	{
		$("#autoimei_sw").iButton();
		/**************************
		 *  IMEI update
		 *************************/
		var fresh_interval = 10000;  //millisecond
		var server_file = "ajax_server.php";
		
		data_update();
		function data_update(){
			$.ajax({
				url: server_file+"?random="+Math.random(),      //request file;
				type: 'GET',                                    //request type: 'GET','POST';
				dataType: 'json',                               //return data type: 'text','xml','json','html','script','jsonp';
				data: {
					"type":"gsm",
					"gsm_type":"imei",
				},
				error: function(data){},                        //request failed callback function;
				success: function(data, status){                        //request success callback function;
					var channel_id = "";
					var channel_val = "";
					for(key_board in data){   //board
						for(key_channel in data[key_board]){   //channel
							channel_id = "<?php echo $__BRD_HEAD__; ?>"+key_board+"-"+"<?php echo $__GSM_HEAD__; ?>"+key_channel;
							if($("#"+channel_id).html() != data[key_board][key_channel]['imei'] && data[key_board][key_channel]['imei'] != ""){
								$("#"+channel_id).html(data[key_board][key_channel]['imei']);
							}
							if(<?php if($force == "yes"){echo "true";}else{echo "false";}?>)
								$("#"+key_board+key_channel).attr("disabled", false);
							else
								$("#"+key_board+key_channel).attr("disabled", data[key_board][key_channel]['disable_modifyimei']);
						}
					}
				},
				complete: function(){
					if(<?php if(is_true($autoimei_sw))echo 'true';else echo 'false';?>){
						setTimeout(function(){data_update();}, fresh_interval);
					}
				}
			});
			
		};
	});
	</script>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	
	<br/>
	<br/>

	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_imei')" id="tab_imei_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_imei')">
			<div class="helptooltips">
				<?php echo language('Automatic Change IMEI');?>
				<span class="showhelp">
				<?php echo language('Automatic Change IMEI help','
					Allowing endpoints to send some specified KEY WORDS and corresponding PASSWORD to operate the gateway. <br/>
					Message is case-sensitive.');
				?>
				</span>
			</div>
		</li>
		<li class="tb2_fold" onclick="lud(this,'tab_imei')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>

	<div id="tab_imei" style="display:block">
	<table width="98%" class="tedit" align="right">
		
		<!-- channel ############################################### -->

		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Port');?>:
					<span class="showhelp">
					<?php echo language('Port help@gsm-autoimei','To select on which GSM channel(s) to send SMS and enter what message you want to send.');?>
					</span>
				</div>
			</th>
			<td>
				<table cellpadding="0" cellspacing="0" class="port_table">
<?php
				$j = 0;
				for($i=1;$i<=$__GSM_SUM__;$i++){
					$port_name = get_gsm_name_by_channel($i);
					$port_name_arr = explode("-", $port_name, 2);
					$port_type = $port_name_arr[0];
					$checked =  $span_checked[1][$i] ? 'checked' : '';
					if($port_type == "null" || $port_type == "cdma") continue;
					
					if($j==0) echo '<tr>';
					echo "<td><input type=\"checkbox\" name=\"spans[1][$i]\" value=\"$i\" $checked class=\"port\">";
					echo $port_name;
					echo '</td>';
					$j++;
					if($j==4){
						$j=0;
						echo '</tr>';
					}
				}

				if($__deal_cluster__){
					$cluster_info = get_cluster_info();
					if($cluster_info['mode'] == 'master') {
						for($b=2; $b<=$__BRD_SUM__; $b++) {
							if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {

								for($line=0; $line<=$__GSM_SUM__/4; $line++) {
									echo '<tr>';
									for($i=1+$line*4; $i<=(4+$line*4); $i++) {
										if($i>$__GSM_SUM__) break;
										$checked =  $span_checked[$b][$i] ? 'checked' : '';
										echo "<td><input type=\"checkbox\" name=\"spans[$b][$i]\" value=\"$i\" $checked class=\"port\">";
										echo get_gsm_name_by_channel($i,$b);
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
					<td style="border:none;"><input type="checkbox" onclick="select_all();"><?php echo language('All');?></td>
				</tr>
				</table>
			</td>
		</tr>
		
		<!--  sw ############################################### -->
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Enable');?>:
					<span class="showhelp">
					<?php echo language('Enable help','ON(enabled), OFF(disabled)');?>
					</span>
				</div>
			</th>
			<td>
				<?php 
					if($autoimei_sw == "on")
						$checked = "checked";
					else
						$checked = "";
				?> 
				<input type="checkbox" id="autoimei_sw" class="checkbox" name="autoimei_sw" <?php echo $checked; ?> onchange="" />
			</td>
		</tr>

		<!-- interval ############################################### -->
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Interval');?>:
				</div>
			</th>
			<td>
				<input type="text" id="interval" name="interval" value="<?php echo $interval ?>" oninput="this.value=this.value.replace(/[^\d]*/g,'')" onkeyup="check(this.id)"/> Second
				<span id="cinterval"></span>
			</td>
		</tr>

		<!-- immediately ############################################### -->
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Immediately');?>:
					<span class="showhelp">
					<?php echo language('Immediately help 1','To decide the modification take effect immediately or not.');?>
					</span>
				</div>
			</th>
			<td style="border:none;"> 
				<?php
						if($immediately == "yes")	
							$checked = "checked";
						else
							$checked = "";
				?>
				<input type="checkbox" name="immediately" <?php echo $checked ?> > <?php echo language('Immediately help 2','modify IMEI immediately');?>
			</td>
		</tr>

		<!-- force ############################################### -->
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Force');?>:
				</div>
			</th>
			<td style="border:none;"> 
				<?php
						if($force == "yes")	
							$checked = "checked";
						else
							$checked = "";
				?>
				<input type="checkbox" id="force" name="force" <?php echo $checked ?> > 
				<?php echo language('Force help','Modify IMEI no matter whether the channel state is ready or not. ');?>
			</td>
	</table>
	</div>

	<div id="newline"></div>

	<div id="tab">
		<li class="tb_fold" onclick="lud(this,'tab_imei_adv')" id="tab_imei_adv_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_imei_adv')">
			<div class="helptooltips">
				<?php echo language('Auto-IMEI Advanced');?>
				<span class="showhelp">
				<?php echo language('Auto-IMEI Advanced help','Advanced Settings for Automatic Modify IMEI.');?>
				</span>
			</div>
		</li>
		<li class="tb2_fold" onclick="lud(this,'tab_imei_adv')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>

	<div id="tab_imei_adv" style="display:none">
		<table width="98%" class="tedit" align="right">
			<tr style="border:none;">
				<th><?php echo language('IMEI Number Setting');?></th>
				<th style="width:496px">
					<table style="font-size:12px;">
						<tr>
							<th style="border:none;text-align:center;width:90px;">TAC(6 <?php echo language('digit');?>)</th>
							<th style="border:none;text-align:center;width:90px;">FAC(2 <?php echo language('digit');?>)</th>
							<th style="border:none;text-align:center;width:90px;">SNR(6 <?php echo language('digit');?>)</th>
							<th style="border:none;text-align:center;width:90px;">SP(1 <?php echo language('digit');?>)</th>
						</tr>
					</table>
				</th>
				<th style="width:125px;text-align:center">
					<?php echo language('Current IMEI');?>
				</th>
				<th style="width:125px;text-align:center">
					<?php echo language('Action');?>
				</th>
			</tr>
			<tr>
				<th><?php echo language('Set to All');?></th>
				<td>
					<table>
						<tr>
							<td style="border:none;"><input type="text" id="imei_tac" name="imei_tac" value="" style="width:80px;" maxlength=6></td>
							<td style="border:none;"><input type="text" id="imei_fac" name="imei_fac" value="" style="width:80px;" maxlength=2></td>
							<td style="border:none;"><input type="text" id="imei_snr" name="imei_snr" value="" style="width:80px;" maxlength=6></td>
							<td style="border:none;"><input type="text" id="imei_sp" name="imei_sp" value="Autogeneration" style="width:100px;" maxlength=1 readonly disabled></td>
						</tr>
					</table>
				</td>
				<td><?php echo language('_None');?></td>
				<td><input type="button" value="<?php echo language('Set to All');?>" style="width:80px" onclick="set_to_all();"></td>
			</tr>
			<?php
			for($i=1; $i<=$__GSM_SUM__; $i++) {
				$port_name = get_gsm_name_by_channel($i);
				$port_name_arr = explode("-", $port_name, 2);
				$port_type = $port_name_arr[0];
				 if($port_type != "null" && $port_type != "cdma") {
			?>
			<tr>
				<th>
				<?php
				echo $port_name;  
				?>
				</th>
				<td>
					<table>
						<tr>
						<?php $tag = "1${i}_imei_tac";?>
						<td style="border:none;"><input type="text" id="<?php echo $tag;?>" name="<?php echo $tag;?>" 
							value="<?php echo ${$tag};?>" style="width:80px;" maxlength=6 oninput="check(this.id)" onkeyup="check(this.id)">
						</td>
						<?php $tag = "1${i}_imei_fac"?>
						<td style="border:none;"><input type="text" id="<?php echo $tag;?>" name="<?php echo $tag;?>" 
							value="<?php echo ${$tag};?>" style="width:80px;" maxlength=2 oninput="check(this.id)" onkeyup="check(this.id)">
						</td>
						<?php $tag = "1${i}_imei_snr"?>
						<td style="border:none;"><input type="text" id="<?php echo $tag;?>" name="<?php echo $tag;?>" 
							value="<?php echo ${$tag};?>" style="width:80px;" maxlength=6 oninput="check(this.id)" onkeyup="check(this.id)">
						</td>
						<td style="border:none;"><input type="text" id="imei_sp" name="imei_sp" 
							value="Autogeneration" style="width:100px;" maxlength=1 readonly disabled>
						</td>
						</tr>
					</table>
					<span id="cimei_<?php echo get_slotnum(); ?>_<?php echo $i?>"></span>
				</td>
				<td id="<?php echo $__BRD_HEAD__."1-".$__GSM_HEAD__.$i; ?>" style="border:none;"></td>
				<td>
					<input type="button" id="<?php echo "1$i";?>" value="<?php echo language('Manual');?>" style="width:80px" 
						onclick="modify_imei(this.id.substr(0,1), this.id.substr(1,2), $('#<?php echo $__BRD_HEAD__."1-".$__GSM_HEAD__.$i; ?>').html())">
				</td>
			</tr>
			<?php
				}
			}   
			if($__deal_cluster__){
				if($cluster_info['mode'] == 'master') {
					for($b=2; $b<=$__BRD_SUM__; $b++) {
						if($cluster_info[$__BRD_HEAD__.$b.'_ip'] == '')
							continue; 
						for($i=1; $i<=$__GSM_SUM__; $i++) {
						?>
						<tr>
							<th>
							<?php
							echo get_gsm_name_by_channel($i,$b);   
							?>
							</th>
							<td>
								<table>
									<tr>
									<?php $tag = "$b${i}_imei_tac";?>
									<td style="border:none;"><input type="text" id="<?php echo $tag;?>" name="<?php echo $tag;?>" class="imei_tac" 
										value="<?php echo ${$tag};?>" style="width:80px" maxlength=6 oninput="check(this.id)" onkeyup="check(this.id)">
									</td>
									<?php $tag = "$b${i}_imei_fac"?>
									<td style="border:none;"><input type="text" id="<?php echo $tag;?>" name="<?php echo $tag;?>" class="imei_fac" 
										value="<?php echo ${$tag};?>" style="width:80px" maxlength=2 oninput="check(this.id)" onkeyup="check(this.id)">
									</td>
									<?php $tag = "$b${i}_imei_snr"?>
									<td style="border:none;"><input type="text" id="<?php echo $tag;?>" name="<?php echo $tag;?>" class="imei_snr" 
										value="<?php echo ${$tag};?>" style="width:80px" maxlength=6 oninput="check(this.id)" onkeyup="check(this.id)">
									</td>
									<td style="border:none;"><input type="text" id="imei_sp" name="imei_sp" 
										value="Autogeneration" style="width:100px" maxlength=1 readonly disabled>
									</td>
									</tr>
								</table>
								<span id="cimei_<?php echo $b."_".$i?>"></span>
							</td>
							<td id="<?php echo $__BRD_HEAD__.$b."-".$__GSM_HEAD__.$i; ?>" style="border:none;"></td>
							<td>
								<input type="button" id="<?php echo $b.$i;?>" value="<?php echo language('Manual');?>" style="width:80px" 
									onclick="modify_imei(this.id.substr(0,1), this.id.substr(1,2), $('#<?php echo $__BRD_HEAD__.$b."-".$__GSM_HEAD__.$i; ?>').html())">
							</td>
						</tr>
						<?php
						}   
					}   
				}   
			}
		?>
		</table>
	</div>
	<div id="newline"></div>
	<br>
	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" id="save" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check_all()" />
	<input type="button" id="" name="" value="<?php echo language('Back Home');?>" onclick="window.location.href='../../index.html'" />
	
	</form>

<?php require("/www/cgi-bin/inc/boot.inc");?>