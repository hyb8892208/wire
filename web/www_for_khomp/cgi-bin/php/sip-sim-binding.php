<?php
require_once("/www/cgi-bin/inc/define.inc");
require_once("/www/cgi-bin/inc/head.inc");
require_once("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
//AQL
require_once("/www/cgi-bin/inc/aql.php");
?>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript">

function setValue(value,slaveip)
{
	console.log("value="+value);
      // alert("value="+value+" slaveip="+slaveip);
	document.getElementById('send').value='Modify';
	console.log('value = '+value);
	document.getElementById('sel_chnl').value = value;
	console.log(document.getElementById('sel_chnl').value);	
	if(slaveip != '') {
		document.getElementById('sel_slaveip').value = slaveip;
	} else {
		document.getElementById('sel_slaveip').value = '';
	}

	var loadi = layer.load('loading......');
}

function siptrunk_change(channel)
{
	asschannel = document.getElementById('associated_chnnl').value;
	channel = 8000 + channel;
	
	if(asschannel == 'none') {
		document.getElementById('cid_number').value = channel;
		document.getElementById('fullname').value = "Channel " + channel;
	} else {
		ass = asschannel.split("-");
		document.getElementById('cid_number').value = ass[1];
		document.getElementById('fullname').value = ass[1];
	}
}

function siptrunk_change_editpage(obj, channel, bindedSipTrunk)
{
	var asschannel_old = $(obj).parent("td").find("input").attr("associated_chnnl_oldvalue");
	var asschannel = document.getElementById('associated_chnnl').value;
	var channel_new = ""+channel;
	var channel_new = 800 + channel_new.slice(-1);
	

	var i, sipTrunks;
	//alert("bindedSipTrunk="+bindedSipTrunk);
	sipTrunks = bindedSipTrunk.split("-",40);
	if (asschannel != 'none') {
		ass = asschannel.split("-");
		for (i = 1; i <= sipTrunks.length; i++) {
			if ((sipTrunks[i] != 'none') && (ass[1] == sipTrunks[i])) {
				alert(ass[1] + " has already used by Port " + i + "!");
				document.getElementById('associated_chnnl').value = asschannel_old;
				return;
			}
		}
	}
/*
	if(asschannel == 'none') {
		document.getElementById('cid_number').value = channel_new;
		document.getElementById('fullname').value = "Channel " + channel_new;		

		var channel_new = ""+channel;
		var boardid=parseInt(channel_new.slice(0,1));
		var portid=parseInt(channel_new.slice(-1));
		var ports = parseInt(<?php echo $__GSM_SUM__; ?>);
		//alert(channel+":"+boardid+":"+portid);
		document.getElementById('portname').value = 'board'+boardid+"-port"+(ports*(boardid-1)+portid);	
		//document.getElementById('portname').value = 'board-'+boardid+"-port-"+portid;				
	} else {
		ass = asschannel.split("-");
		document.getElementById('cid_number').value = ass[1];
		document.getElementById('fullname').value = ass[1];
		$(obj).parent("td").find("input").attr("associated_chnnl_oldvalue",asschannel);
	}
*/
}

function siptrunk_change_new(obj, channel, ports)
{
	var asschannel_old = $(obj).parent("td").find("input").attr("associated_chnnl_oldvalue");
	var asschannel = document.getElementById('associated_chnnl'+channel).value;
	var channel_new = ""+channel;
	var channel_new = 800 + channel_new.slice(-1);
	
	if (asschannel != 'none') {
<?php
		//added by wangxuechuan at 20131115
		$cluster_info = get_cluster_info();	
		if(($cluster_info['mode'] == 'master') || ($cluster_info['mode'] != 'master')) {
?>
			for (i = 1; i <= ports; i++) {
				if (channel != '1'+i) {
					otherchannel = document.getElementById('associated_chnnl1'+i).value;
					if (otherchannel == asschannel && otherchannel != 'none') {
						alert(asschannel + " has already used by Port " + i + "!");
						document.getElementById('associated_chnnl'+channel).value = asschannel_old;
						return;
					}
				}
			}
	<?php
		}
	?>
	}
/*
	if(asschannel == 'none') {
		document.getElementById('cid_number'+channel).value = channel_new;
		var channel_new = ""+channel;
		var boardid=parseInt(channel_new.slice(0,1));
		var portid=parseInt(channel_new.slice(-1));
		//alert(channel+":"+boardid+":"+portid+":"+(ports*(boardid-1)+portid));
		document.getElementById('portname'+channel).value = 'board'+boardid+"-port"+(ports*(boardid-1)+portid);
		//document.getElementById('portname'+channel).value = 'board-'+boardid+"-port-"+portid;
	} else {
		document.getElementById('cid_number'+channel).value = asschannel;
		$(obj).parent("td").find("input").attr("associated_chnnl_oldvalue",asschannel);
	}
*/
}

function check()
{
	var ports = <?php echo $__GSM_SUM__;?>;
	var select_siptrunk = false;
	<?php 
		$cluster_info = get_cluster_info();
		if(($cluster_info['mode'] == 'master') || ($cluster_info['mode'] != 'master')) {
	?>	
	for(var i = 1; i <= ports; i++){
		if(document.getElementById('associated_chnnl1'+i).value != 'none' ){
			select_siptrunk = true;
		}
	}
	<?php
	}
	
	if($__deal_cluster__){
		if($cluster_info['mode'] == 'master') {
		//update_cluster_board_num($cluster_info['devicemode']);
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
				if($slaveip != '') {
		?>
					for(var i = 1; i <= ports; i++){
						if(document.getElementById('<?php echo "associated_chnnl".$b;?>'+i).value != 'none' ){
							select_siptrunk = true;
						}
					}
		<?php
				}
			}
		}
	}
	?>
	if(!select_siptrunk){
		alert('all ports are not associated with the sipTrunk, please choosing a siptrunk for any ports!');
		return false;
	}
 	return true;
}


</script>
<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>

<?php
$all_sips = get_all_sips();
$fxs_use_hidecallerid = 0;
function channel_select($name, $sel=NULL, $channel, $bindedSipTrunk)
{
	global $__SIP_HEAD__;

	if('' == $sel) {
		echo "<input type=\"hidden\" associated_chnnl_oldvalue='None'>";
	} else {
		echo "<input type=\"hidden\" associated_chnnl_oldvalue=\"$sel\">";
	}
	
	echo "<select size=1 name=\"$name\" id=\"$name\" onchange=\"siptrunk_change_editpage(this, $channel,'".$bindedSipTrunk."')\">";
	echo '<option value="none">';echo language('_None');echo '</option>';

	global $all_sips;
	//$all_sips = get_all_sips();
	if($all_sips) {
		echo '<optgroup label="';echo language('SIP');echo '">';
			foreach($all_sips as $sip) {
			$username1 = trim($sip['username']);
			$value = $__SIP_HEAD__.$username1;

			if($sel == $value) {
        			echo "<option value=\"$value\" selected>";
			} else if (is_analog_use($value) || is_routing_use($value)) {
        			echo "<option value=\"$value\" disabled>";
			} else {
				echo "<option value=\"$value\">";
			}
			echo "$username1";
			echo '</option>';
		}
		echo '</optgroup>';
	}
	echo '</select>';
}

function channel_select_new($name, $sel=NULL, $channel, $ports)
{
	global $__SIP_HEAD__;
	$name_new = $name . $channel;

	if('' == $sel) {
		echo "<input type=\"hidden\" associated_chnnl_oldvalue='None'>";
	} else {
		echo "<input type=\"hidden\" associated_chnnl_oldvalue=\"$sel\">";
	}
		
	//echo "<select size=1 name=\"$name_new\" id=\"$name_new\" onchange=\"siptrunk_change_new(this, $channel, $ports)\">";
	echo "<select size=1 name=\"$name_new\" id=\"$name_new\" >";
	echo '<option value="none">None</option>';

	global $all_sips;
	//$all_sips = get_all_sips();
	if($all_sips) {
		echo '<optgroup label="SIP">';
		foreach($all_sips as $sip) {
			$endpoint_name = trim($sip['endpoint_name']);
			$name = get_sip_name_no_head($endpoint_name);
			if($sel == $endpoint_name) {
				echo "<option value=\"$endpoint_name\" selected>";
			} else {
				echo "<option value=\"$endpoint_name\">";
			}
			echo "$name";
			echo '</option>';
		}
		echo '</optgroup>';
	}
	echo '</select>';
}



function show_chnls()
{
	global $__GSM_SUM__;
	global $__SIP_HEAD__;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $only_view;
	
	$aql = new aql();
	$cluster_info = get_cluster_info();
	 
?>
<form enctype="multipart/form-data" action="<?php echo get_self();?>" method="post">
	
	<div class="content">
		<table class="table_show">
			<tr>
				<th width="10%"><?php echo language('Port');?></th>
				<th width="30%"><?php echo language('Sim Number');?></th>
				<th width="30%"><?php echo language('Sip Trunk');?></th>
				<th width="30%"><?php echo language('CallerID');?></th>
				<input type="hidden" id="sel_chnl" name="sel_chnl" value="" />
				<input type="hidden" id="sel_slaveip" name="sel_slaveip" value="" />			
			</tr>
			
			<?php

			if(($cluster_info['mode'] == 'master') || ($cluster_info['mode'] != 'master')) {
				$tablerows = $__GSM_SUM__; //4;
				for ($i = 1; $i <= $tablerows; $i++) {
					$port_name = change_gsms_port($i, 1);
					if(strstr($port_name, 'null-'))continue;
			?>
					<tr>
						<td>
							<?php 
									echo $port_name;
							?>
						</td>
						<td>
							<input type=text name="<?php echo 'sim_number1'.$i; ?>" id="<?php echo 'sim_number1'.$i; ?>" value="" title="<?php echo language('SIM number');?>" />

						</td>
						<td>
							<?php channel_select_new('associated_chnnl', '', '1'.$i, $__GSM_SUM__);?>
						</td>
						<td>
							<input type=text size="40px" name="<?php echo 'callerid1'.$i; ?>" id="<?php echo 'callerid1'.$i; ?>" value="" />
						</td>

					</tr>
			<?php
				}
			}
			?>
		</table>
		
		<div style="margin-top:20px;">
			<span style="color:red;font-size:14px;">
			<?php echo language('SIM number help', 'SIM Number: This number will be sent to target PBX or SIP Server as a DID number for inbound rule settings.If the SIM number is empty, no incoming routing is created.');?>
			</span>
		</div>
		
		<input type="hidden" name="send" id="send" value="" />
		
		<div id="button_save">
		
			<?php if(!$only_view){ ?>	
			<button type="submit" onclick="document.getElementById('send').value='Save';return check();"><?php echo language('Save');?></button>
			<?php } ?>
			
			<button type="button" onclick="location.reload()" ><?php echo language('Cancel');?></button>
		</div>
	</div>
	
</form>	
<?php
}
?>
<?php
function save_chnls()
{
	global $__GSM_SUM__;
	global $fxs_use_hidecallerid;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $__SIP_HEAD__;

	//GetEfficien();

	//$endpoint_allow = get_sip_allow_all();
	//$sip_username_and_ip_all = get_sip_username_and_ip_all();
	$aql = new aql();
	$cluster_info = get_cluster_info();	
	if(($cluster_info['mode'] == 'master') || ($cluster_info['mode'] != 'master')) {
		$gw_routing_conf = "/etc/asterisk/gw_routing.conf";
		$hlock = lock_file($gw_routing_conf);
		if (!file_exists($gw_routing_conf)) fclose(fopen($gw_routing_conf,"w"));

		$aql->set('basedir','/etc/asterisk');
		if(!$aql->open_config_file($gw_routing_conf)){
			echo $aql->get_error();
			unlock_file($hlock);
			return;
		}
		$order = 1;
		for ($i = 1; $i <= $__GSM_SUM__; $i++) {
			//this data chunk will store all the (changed, unchanged)keys
			//if(empty($_POST['sim_number1'.$i]) || $_POST['associated_chnnl1'.$i] == 'None') {
			if((isset($_POST['associated_chnnl1'.$i]) && $_POST['associated_chnnl1'.$i] == 'none') || !isset($_POST['associated_chnnl1' . $i])) {
				continue;
			}
			// $call_mode = "gsm2sip";
			// $section = $call_mode . "-1." . $i;
			$section = change_gsms_port($i, 1).'2sip';
			 
			$idname = 'sim_number1'.$i;
			if(isset($_POST[$idname])){
				$forward_number = trim($_POST[$idname]);
			} else {
				$forward_number = '';
			}
			$idname =  'associated_chnnl1' . $i;
			if(isset($_POST[$idname])) {
				$associated_sip = trim($_POST[$idname]);
			} else {
				$addociated_sip = '';
			}
			
			$idname = 'callerid1' . $i;
			if(isset($_POST[$idname])) {
				$callerid = trim($_POST[$idname]);
			} else {
				$callerid = '';
			}
			$dial_pattern = '';
			if(strstr($callerid, ',')){
				$callerid_arr = explode(',', $callerid);
				foreach ($callerid_arr as $callerid) {
					$dial_pattern .= '|||' . $callerid . ',';
				}
				$dial_pattern = substr($dial_pattern, 0, -1);
			} else {
				$dial_pattern = '|||' . $callerid;
			}
			
			if($_POST['sim_number1'.$i] != ''){
				$datachunk = array();
				/*
				 *	 FROM gsm ---->  TO sip
				 */
				$datachunk['gsm2sip'][$i] = '';
				$datachunk['gsm2sip'][$i] .= "order=$order\n"; 
				$datachunk['gsm2sip'][$i] .= "from_channel=gsm-$i\n";
				$datachunk['gsm2sip'][$i] .= "to_channel=sip-$associated_sip\n";

				$datachunk['gsm2sip'][$i] .= "dial_pattern=\n";
				$datachunk['gsm2sip'][$i] .= "time_pattern=\n";
				$datachunk['gsm2sip'][$i] .= "cid_name=\n";
				$datachunk['gsm2sip'][$i] .= "cid_number=\n";
				$datachunk['gsm2sip'][$i] .= "forward_number=$forward_number\n";
				$datachunk['gsm2sip'][$i] .= "DISA_sw=off\n";
				$datachunk['gsm2sip'][$i] .= "second_dial_sw=off\n";
				$datachunk['gsm2sip'][$i] .= "timeout=5\n";
				$datachunk['gsm2sip'][$i] .= "max_passwd_digits=10\n";
				$aql->assign_delsection($section);
				$aql->save_config_file('gw_routing.conf');
				$aql->assign_addsection($section, $datachunk['gsm2sip'][$i]);
				$aql->save_config_file('gw_routing.conf');
			}
			/*
			 * FROM sip ----> TO gsm
			 */
			// $call_mode = "sip2gsm";
			// $section = $call_mode . "-1." . $i;
			$section = 'sip2'.change_gsms_port($i, 1);
			$datachunk['sip2gsm'][$i] = '';
			$datachunk['sip2gsm'][$i] .= "order=$order\n";
			$datachunk['sip2gsm'][$i] .= "from_channel=sip-$associated_sip\n";
			$datachunk['sip2gsm'][$i] .= "to_channel=gsm-$i\n";
			if(!empty($callerid)) {
				$datachunk['sip2gsm'][$i] .= 'dial_pattern=' .$dial_pattern ."\n";
			} else {
				$datachunk['sip2gsm'][$i] .= "dial_pattern=\n";
			}
			$datachunk['sip2gsm'][$i] .= "time_pattern=\n";
			$datachunk['sip2gsm'][$i] .= "cid_name=\n";
			$datachunk['sip2gsm'][$i] .= "cid_number=\n";
			$datachunk['sip2gsm'][$i] .= "forward_number=\n";
			$datachunk['sip2gsm'][$i] .= "DISA_sw=off\n";
			$datachunk['sip2gsm'][$i] .= "second_dial_sw=off\n";
			$datachunk['sip2gsm'][$i] .= "timeout=5\n";
			$datachunk['sip2gsm'][$i] .= "max_passwd_digits=10\n";
			$aql->assign_delsection($section);
			$aql->save_config_file('gw_routing.conf');
			$aql->assign_addsection($section, $datachunk['sip2gsm'][$i]);
			$aql->save_config_file('gw_routing.conf');
		}
		unlock_file($hlock);
	}

	save_routings_to_extensions();
	
	echo "<h2>".language("Save Successful")."</h2>";
}

if($_POST) {
		if (isset($_POST['send']) && $_POST['send'] == "Save") {
				if($only_view){
					return false;
				}
			
				save_chnls();
				show_chnls();
				wait_apply("exec","asterisk -rx \"core reload\" > /dev/null 2>&1 &");
				
				save_user_record("","ROUTING->Batch Creating Rules:Save");
			}

	} else {
		show_chnls();
	}
?>

<?php require_once("/www/cgi-bin/inc/boot.inc");?>
<div id="to_top"></div>
<div id="float_btn1" class="sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()" >
</div>
