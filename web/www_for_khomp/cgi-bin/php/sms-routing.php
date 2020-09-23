<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
require_once("/www/cgi-bin/inc/aql.php");
?>
<script type="text/javascript" src="../../js/jquery.js"></script>
<?php 
	//在javascript 内存里创建sms-routing.conf里各规则的from_members字段的镜像表array_routings	
	echo "<script>var array_routings =new Array();</script>";
	$all_sms_routings = get_all_sms_routing(true);	
	if ($all_sms_routings!=""){
	foreach($all_sms_routings as $group){
		echo "<script>";
		echo "var member_string='".trim($group['from_member'])."';\n";		
		echo "array_routings['".$group['routing_name']."']=member_string.split(',')\n";
		echo "</script>";
	}
	}
	$sms_forward_trigger = check_oem_funs('sms_forward_trigger');
?>



<?php
function show_groups()
{
	global $__BRD_HEAD__;
	global $__GSM_HEAD__;
	global $only_view;

	$all_sms_routings = get_all_sms_routing(true);
		
	$last_order = 1;
	if($all_sms_routings) {
		$last = end($all_sms_routings); 
		if(isset($last['order']) && $last['order'] != '') {
			$last_order = $last['order'] + 1;
		}
	}
?>
	<script type="text/javascript">
	
	function getPage(value)
	{
		window.location.href = '<?php echo get_self();?>?sel_routing_name='+value;
	}

	function setValue(value1,value2)
	{
		document.getElementById('sel_routing_name').value = value1;
		document.getElementById('order').value = value2;
	}

	function delete_click(value1,value2)
	{
		ret = confirm("Are you sure to delete you selected ?");
		if(ret) {
			document.getElementById('sel_routing_name').value = value1;
			document.getElementById('order').value = value2;
			return true;
		}

		return false;
	}
	</script>
	
<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post" >
	<input type="hidden" id="sel_routing_name" name="sel_routing_name" value="" />
	<input type="hidden" id="order" name="order" value="" />
	
	<div class="content">
		<table class="table_show">
			<tr>
				<th width="120px"><?php echo language('Routing Name');?></th>
				<th width="50px"><?php echo language('Type');?></th>
				<th width="80px"><?php echo language('Policy');?></th>
				<th><?php echo language('From Members');?></th>
				<th><?php echo language('To Members');?></th>
				<th width="80px"><?php echo language('To Number');?></th>
				<th width="80px"><?php echo language('Actions');?></th>
			</tr>
			
			<?php
			if($all_sms_routings) {
				foreach($all_sms_routings as $group) {
					$routing_name = trim($group['routing_name']);
					$from_members = trim($group['from_member']);	
					$to_members = trim($group['to_member']);	
					$type = "module";
					$policy = trim($group['policy']);
					$to_number=trim($group['to_number']);
					$show_members = '';
					$from_members = rtrim($from_members,', ');
			?>

				<tr>
					<td>
						<?php echo $routing_name ?>
					</td>
					<td>
						<?php echo $type ?>
					</td>
					<td>
						<?php echo $policy ?>
					</td>
					<td>
						<?php echo change_sms_routing_form_new($from_members);?>
					</td>
					<td>
						<?php echo change_sms_routing_form_new($to_members);?>
					</td>
					<td>
						<?php echo $to_number ?>
					</td>
					<td>
						<button type="button" value="Modify" style="width:32px;height:32px;padding:0;" 
							onclick="getPage('<?php echo $routing_name ?>')">
							<img src="/images/edit.gif">
						</button>
						
						<?php if(!$only_view){ ?>
						<button type="submit" value="Delete" style="width:32px;height:32px;padding:0;" 
							onclick="document.getElementById('send').value='Delete';return delete_click('<?php echo $routing_name ?>', '')" >
							<img src="/images/delete.gif">
						</button>
						<?php } ?>
						
						<input type="hidden" name="routing_name[]" value="<?php echo $routing_name ?>" />
					</td>
				</tr>
		<?php
				}
			}
		?>
		</table>
	</div>
	
	<input type="hidden" name="send" id="send" value="" />
	
	<div id="button_save">
		<input type="submit" value="<?php echo language('New Routing');?>" onclick="document.getElementById('send').value='New Routing';setValue('', '<?php echo $last_order ?>')" />
	</div>
</form>
<?php
}
?>

<?php
function save_routings()
{
	// /etc/asterisk/sms_routing.conf
	// [routing_name]
	// order = 1,2,3,4,5....    //Must set
	// type = sip or gsm  //Must set
	// policy = 
	// from members = channel,channel......   //Must set
	// to members = channel,channel......   //Must set
    // to number = channel,channel......   //Must set
	
	global $sms_forward_trigger;
	$datachunk = '';

	
	
	
	//Routing Name already existed! 
	
	if( isset($_POST['routing_name']) ) {
		$routing_name = trim($_POST['routing_name']);
		if($routing_name == '') {
			echo "Must set Routing Name";
			return false;
		}
		$section = $routing_name;
	} else {
		echo "Must set Routing Name";
		return false;
	}

	$old_section = $section;
	if( isset($_POST['old_routing_name']) ) {
		$old_section = trim($_POST['old_routing_name']);
	}

	if( isset($_POST['order']) ) {
		$order = trim($_POST['order']);
		if($order == '') {
			echo "[$routing_name] ";
			echo language('does not exist');
			//echo "Must set order";
			return false;
		}
		$datachunk .= 'order='.$order."\n";
	} else {
		echo "[$routing_name] ";
		echo language('does not exist');
		//echo "Must set order"
		return false;
	}

	if( isset($_POST['type']) ) {
		$type = trim($_POST['type']);
		if($type == '') {
			echo "Must set type";
			return false;
		}
		$datachunk .= 'type='.$type."\n";
	} else {
		echo "Must set type";
		return false;
	}

	
	
	if( isset($_POST['policy']) ) {
		$policy = trim($_POST['policy']);
		if($policy == '') {
			echo "Must set policy";
			return false;
		}
		$datachunk .= 'policy='.$policy."\n";
	} else {
		echo "Must set policy";
		return false;
	}

	
	$from_members = '';
	
	if( isset($_POST['gsm_from_members']) && is_array($_POST['gsm_from_members']) ) {
		foreach($_POST['gsm_from_members'] as $value) {
			$value = trim($value);
			$pos = strpos($value, 'umts');
 			if ($pos !== false) {
				$value = str_replace('umts', 'gsm', $value);
			}
			$pos = strpos($value, 'cdma');
			if ($pos !== false) {
				$value = str_replace('cdma', 'gsm', $value);
			}
			$pos = strpos($value, 'lte');
			if ($pos !== false) {
				$value = str_replace('lte', 'gsm', $value);
			}
			if($value != '') {
				$from_members .= $value.',';
			}
		}
	}

	if($from_members == '') {
		echo "Must set from_members";
		return false;
	}
	
	$from_members = rtrim($from_members,',');
	$datachunk .= 'from_member='.$from_members."\n";

	$to_members = '';				
	if( isset($_POST['gsm_to_members']) && is_array($_POST['gsm_to_members']) ) {
		foreach($_POST['gsm_to_members'] as $value) {
			$value = trim($value);
			$pos = strpos($value, 'umts');
			if ($pos !== false) {
				$value = str_replace('umts','gsm',$value);
			}
			$pos = strpos($value, 'cdma');
			if ($pos !== false) {
				$value = str_replace('cdma', 'gsm', $value);
			}
			$pos = strpos($value, 'lte');
			if ($pos !== false) {
				$value = str_replace('lte', 'gsm', $value);
			}
			if($value != '') {
				$to_members .= $value.',';
			}
		}
	}

	if($to_members == '') {
		echo "Must set to_members";
		return false;
	}
	
	$to_members = rtrim($to_members,',');
	$datachunk .= 'to_member='.$to_members."\n";

	if( isset($_POST['to_number']) ) {
		$to_number = trim($_POST['to_number']);
		if($to_number == '') {
			echo "Must set to_number";
			return false;
		}
		$datachunk .= 'to_number='.$to_number."\n";
	} else {
		echo "Must set to_number";
		return false;
	}
	
	
	//custom trigger functions： save web configurations to sms_routing.conf
	//for example:
	//--trigger_start--
	//-TRIGGER-sorry=Call you back ("sorry" means trigger, "Call you back" means sms return)
	//--trigger_end--
	if ($sms_forward_trigger) {
		if (isset($_POST['trigger_clear'])) {
			$trigger_clear_str = trim($_POST['trigger_clear']);
			if ($trigger_clear_str != '') {
				$datachunk .= 'trigger_clear='.$trigger_clear_str."\n";
			} 
		}
		
		$i = 0;
		if(isset($_POST['trigger']) && is_array($_POST['trigger']) && $_POST['trigger'] != '') {
			foreach($_POST['trigger'] as $each) {
				if ($each != '') {
					$each = trim($each);
					$trigger_array[$i++]['trigger'] = $each;
				}
			}
		} 
		
		$i = 0;
		if(isset($_POST['context']) && is_array($_POST['context']) && $_POST['context'] != '') {
			foreach($_POST['context'] as $each) {
				if ($each != '') {
					$each = trim($each);
					$trigger_array[$i++]['context'] = $each;
				}
			}
		} 	
		
		$str = '';
		if(isset($trigger_array) && is_array($trigger_array)) {
			$str = '--trigger_start--'."\n";
			foreach($trigger_array as $each) {
				$trigger = '';
				if (isset($each['trigger'])) {
					$trigger = $each['trigger'];
				}
				$context = '';
				if (isset($each['context'])) {
					$context = $each['context'];
				}
				if (!($trigger=='' && $context=='')) {
					$str .= '-TRIGGER-'.$trigger."=".$context."\n";
				}
			}
			$str .= '--trigger_end--'."\n";
		}
		
		if ($str != '' ) {
			$datachunk .= $str;
		}
	} 

	//Save to sms_routing.conf
	///////////////////////////////////////////////////
	$sms_group_conf_path = "/etc/asterisk/sms_routing.conf";
	$hlock = lock_file($sms_group_conf_path);
	if (!file_exists($sms_group_conf_path)) fclose(fopen($sms_group_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($sms_group_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	
	$aql->assign_delsection($old_section);
	$aql->save_config_file('sms_routing.conf');
	$aql->assign_addsection($section,$datachunk);
	$aql->save_config_file('sms_routing.conf');
	
	
	unlock_file($hlock);
	///////////////////////////////////////////////////

	save_to_flash('/etc/asterisk','/etc/cfg');

	save_user_record("","SMS->SMS Forwarding:Save,section=".$section);

	return true;
}
?>

<?php
function del_group($routing_name)
{
	//Save to sms_routing.conf
	///////////////////////////////////////////////////
	// /etc/asterisk/sms_routing.conf
	$sms_group_conf_path = "/etc/asterisk/sms_routing.conf";
	$hlock = lock_file($sms_group_conf_path);
	if (!file_exists($sms_group_conf_path)) fclose(fopen($sms_group_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($sms_group_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	$aql->assign_delsection($routing_name);
	$aql->save_config_file('sms_routing.conf');
	unlock_file($hlock);
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	// /etc/cfg/sms_routing.conf
	$sms_group_conf_path = "/etc/cfg/sms_routing.conf";
	$hlock = lock_file($sms_group_conf_path);
	if (!file_exists($sms_group_conf_path)) fclose(fopen($sms_group_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/cfg');
	if(!$aql->open_config_file($sms_group_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	$aql->assign_delsection($routing_name);
	$aql->save_config_file('sms_routing.conf');
	unlock_file($hlock);
	///////////////////////////////////////////////////

	//save_routings_to_extensions();
	
	save_user_record("","SMS->SMS Forwarding:Delete,section=".$section);
}
?>


<?php
function gsm_member_show($members,$used_members,$type)
{
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	global $__GSM_SUM__;
	global $__GSM_HEAD__;
	global $__deal_cluster__;

	$cluster_info = get_cluster_info();
	if($cluster_info['mode'] == 'master') {
		$HEAD = $__BRD_HEAD__.'1-';
	} else {
		$HEAD = '';
	}

	$num = 1;

	echo "<table  cellpadding=\"0\" cellspacing=\"0\" style=\"border:none;\" >";
	
	echo "<tr style=\"border:none;\">";
	echo "<td style=\"border:none;\">";
	echo "NO.";
	echo "</td>";
	echo "<td style=\"border:none;\">";
	
	
	if ($type=='from'){
		echo "<span id='crouting_from'></span>";
	}else{
		echo "<span id='crouting_to'></span>";
	}
	
	echo "</td>";
	echo "</tr>";

	
	//GSM
	for($i=1; $i<=$__GSM_SUM__; $i++) {
		//	$value = get_gsm_value_by_channel($i);
		$value = "gsm1.".$i;
		$name = get_gsm_name_by_channel($i);
		if(strstr($name, 'null')){continue;}
		
		$checked = '';
		$display = '';
		if(is_array($members)){
			foreach($members as $each) {
				//$each=str_replace('gsm','gsm-',$each);
				$temp = explode('.',$each);
				$port = $temp[1];
				if($port == $i) {
					$checked = 'checked';
					break;
				}
			}
			
			foreach($used_members as $each){
				$temp = explode('.',$each);
				$port = $temp[1];
				if($port == $i){
					$display = 'display:none;';
					break;
				}
			}
		}
		
		if ($type=='from'){
			echo "<tr style=\"border:none; $display\" id=r_f_$name >";
		}else{
			echo "<tr style=\"border:none; $display\" id=r_t_$name >";
		}
		
	
		echo "<td style=\"border:none;\">";
		echo "$num";
		echo "</td>";
		echo "<td style=\"border:none;\">";
		//$v_name=str_replace('-','',$name);
		if ($type=='from'){
			echo "<input type=\"checkbox\" name=\"gsm_from_members[]\" id=\"f_$name\" value=\"$value\" $checked  onclick=\"dynamic_checkbox(this)\">";
		}else{
			echo "<input type=\"checkbox\" name=\"gsm_to_members[]\" id=\"t_$name\" value=\"$value\" $checked  onclick=\"dynamic_checkbox(this)\">";
		}
		
		echo $name;
		echo "</td>";
		echo "</tr>";
		$num++;
	}

	if($__deal_cluster__){
		if($cluster_info['mode'] == 'master') {
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				if(isset($cluster_info[$__BRD_HEAD__.$b.'_ip']) && $cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					for($i=1; $i<=$__GSM_SUM__; $i++) {
						$value = get_gsm_value_by_channel($i,$b);
						$name = get_gsm_name_by_channel($i,$b);

						$checked = '';
						foreach($members as $each) {
							//$each=str_replace('gsm','gsm-',$each);
							$each=change_sms_routing_form($each);
							if($each === $name) {
								$checked = 'checked';
								break;
							}
						}
						if ($type=='from'){
							echo "<tr style=\"border:none;\" id=r_f_$name>";
						}else{
							echo "<tr style=\"border:none;\" id=r_t_$name>";
						}
						echo "<td style=\"border:none;\">";
						echo "$num";
						echo "</td>";
						echo "<td style=\"border:none;\">";
						$v_name=str_replace('-','',$name);
						if ($type=='from'){
							echo "<input type=\"checkbox\" name=\"gsm_from_members[]\" id=\"f_$name\" value=\"$v_name\" $checked  onclick=\"dynamic_checkbox(this)\">";
						}else{
							echo "<input type=\"checkbox\" name=\"gsm_to_members[]\" id=\"t_$name\" value=\"$v_name\" $checked  onclick=\"dynamic_checkbox(this)\">";
						}
						//echo "<input type=\"checkbox\" name=\"gsm_members[]\" value=\"$value\" $checked>";
						echo $name;
						echo "</td>";
						echo "</tr>";
						$num++;
					}
				}
			}
		}
	}

	echo "</table>";
}

/******************************
custom trigger functions：draw "trigger settings" web configurations table
******************************/
function trigger_settings_table($trigger,$context)
{
	//if ($trigger == '') $trigger = 'trigger';
	//if ($context == '') $context = 'context';
	echo '<table class="tab_border_none" width="100%">';
	
	echo '<tr>';


	echo '<td class="td_talignr">';
	echo language('Trigger');
	echo ':</td>';
	echo '<td class="td_talignl">';

	echo "<input type=\"text\" size=\"45\" class=\"cls_trigger\" name=\"trigger[]\" value=\"$trigger\"/>";
	echo '</td>';
		
	echo '<td class="td_talignr">';
	echo language('Send Text');
	echo ':</td>';
	echo '<td class="td_talignl">';
	$send_text = '';
	echo "<input type=\"text\" size=\"45\" class=\"cls_text\" name=\"context[]\"  value=\"$context\" /> ";
	echo '</td>';


	
	echo '<td rowspan=2>';
	echo '<img src="/images/delete.gif" style="float:none; margin-left:0px; margin-bottom:-3px;cursor:pointer;" alt="remove"';
	echo 'title="';
	echo language('Click here to remove this pattern');
	echo '" onclick="javascript:this.parentNode.parentNode.parentNode.parentNode.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode.parentNode.parentNode.parentNode.parentNode);">';
	echo '</td>';
	
	echo '</tr></table>';
}

function add_group_page($routing_name,$order = '')
{
	global $__MODULE_HEAD_ARRAY__;
	global $sms_forward_trigger;
	global $only_view;
	
	$type_show = $__MODULE_HEAD_ARRAY__[1][1];
	$type_show = strtoupper(str_replace("-","",$type_show));
	if($routing_name) {
		echo "<h4 style='margin-top:30px;'>";echo language('Modify a Routing');echo "</h4>";
	} else {
		echo "<h4 style='margin-top:30px;'>";echo language('Create a Routing');echo "</h4>";
	}

	// /etc/asterisk/sms_routing.conf
	// [routing_name]
	// type = type (sip or gsm)
	// policy =
	// members = channel,channel......

	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk');
	if (!$setok) {
		return;
	}

	$section = $routing_name;

	if($section) {
		$hlock = lock_file('/etc/asterisk/sms_routing.conf');
		$res = $aql->query("select * from sms_routing.conf where section='$section'");
		unlock_file($hlock);
	}

	if(isset($res[$section]['order'])) {
		if($routing_name) {
			$order = trim($res[$section]['order']);
		}
	}

	if(isset($res[$section]['type'])) {
		$type = trim($res[$section]['type']);
	} else {
		$type = 'gsm';
	}

	if(isset($res[$section]['policy'])) {
		$policy = trim($res[$section]['policy']);
	} else {
		$policy = '';
	}

	if(isset($res[$section]['to_number'])) {
		$to_number = trim($res[$section]['to_number']);
	} else {
		$to_number = '';
	}

	if(isset($res[$section]['from_member'])) {
		$val = trim($res[$section]['from_member']);
		$from_members = explode(',',$val);
		if(!isset($from_members[0])) {
			$from_members = array();
		}
	} else {
		$from_members = array();
	}
	

	
	if(isset($res[$section]['to_member'])) {
		$val = trim($res[$section]['to_member']);
		$to_members = explode(',',$val);
		if(!isset($to_members[0])) {
			$to_members = array();
		}
	} else {
		$to_members = array();
	}


	
	//custom trigger functions： get trigger infomations from sms_routing.conf
	if ($sms_forward_trigger) {
		if (isset($res[$section]['trigger_clear'])) {
			$trigger_clear_str = trim($res[$section]['trigger_clear']);
		} else {
			$trigger_clear_str = '';
		}
		
		if (isset($res) && is_array($res)) {
			foreach ( $res as $routename =>$config ) {
				foreach ($config as $key => $value) {
					if (strpos($key,'TRIGGER-') == 1) {
						$trigger_str = substr($key,9);
						if (strstr($trigger_str,"\"")) {
							$trigger_str = str_replace("\"","&quot;",$trigger_str);
						}
						if (strstr($value,"\"")) {
							$value = str_replace("\"","&quot;",$value);
						}
						$trigger_array[$trigger_str] = $value;
					}
				}
			}
		}
	}
	
?>

	<script type="text/javascript" src="/js/check.js"></script>
	<script type="text/javascript" src="/js/functions.js"></script>

	<script type="text/javascript">
	function typechange()
	{
		var type = document.getElementById('type').value;

		if(type == 'sip') {
			set_visible('span_gsm_members', false);
			set_visible('span_sip_members', true);	
		} else {
			set_visible('span_sip_members', false);
			set_visible('span_gsm_members', true);	
		}
	}

	function onload_func()
	{
		typechange();
	}

	function check()
	{
		<?php if ($sms_forward_trigger) { ?>
		var clear_str = document.getElementById('trigger_clear').value;
		var obj_trigger = $(".cls_trigger");
		var obj_text = $(".cls_text");
		if ((clear_str.indexOf(";") != -1) || (clear_str.indexOf("=") != -1)) {
			document.getElementById('trigger_clear').style.color = "red";
			document.getElementById('ctrigger_clear').innerHTML = con_str('Invalid characters input.Characters contain \";=\" are not allowed.');
			for (var i = 0; i < obj_trigger.length;i++) {
				obj_trigger[i].style.color = "";
				obj_text[i].style.color = "";
			}
			document.getElementById('conf_list').innerHTML = '';
			return false;
		} else {
			document.getElementById('trigger_clear').style.color = "";
			document.getElementById('ctrigger_clear').innerHTML = "";
		}


		for (var i = 0; i < obj_trigger.length; i++) {
			var str_trigger = obj_trigger[i].value;
			if ((str_trigger.indexOf(";") != -1) || (str_trigger.indexOf("=") != -1)) {
					obj_trigger[i].style.color = "red";
					//$(obj_trigger[i]).next().html("valid trigger.");
					document.getElementById('conf_list').innerHTML = con_str("Invalid characters input.Characters contain \";=\" are not allowed.");
					return false;				
			} else {
				obj_trigger[i].style.color = "";
				document.getElementById('conf_list').innerHTML = '';
			}
			
			var str_text = obj_text[i].value;
			if ((str_text.indexOf(";") != -1) || (str_text.indexOf("=") != -1)) {
					obj_text[i].style.color = "red";
					//$(obj_text[i]).next().html("valid trigger.");
					document.getElementById('conf_list').innerHTML = con_str("Invalid characters input.Characters contain \";=\" are not allowed.");
					return false;				
			} else {
				obj_text[i].style.color = "";
				document.getElementById('conf_list').innerHTML = '';
			}
		}
		<?php } ?>
		
		return true;
	}
	
	function CheckForm(){
		var is_check = false;
		
		<?php
		$agroups = get_all_sms_routing(true);
		$name_ary = '';
		if($agroups) {
			foreach($agroups as $group) {		
				if(strcmp($group['routing_name'],$routing_name)==0)
					continue;
				$name_ary .=  '"'.$group['routing_name'].'"'.',';
			}
		}
		$name_ary = rtrim($name_ary,',');
		?>
		var name_ary = new Array(<?php echo $name_ary; ?>);
		var routing_name = document.getElementById('routing_name').value;
		
		document.getElementById('crouting_name').innerHTML = '';
		if(!check_routingname(routing_name)) {
			document.getElementById('routing_name').focus();
			document.getElementById("crouting_name").innerHTML = con_str('<?php echo htmlentities(language('js check groupname','Allowed character must be any of [0-9a-zA-Z`~!@#$%^*()_{}:|?-=.], 1-32 characters.'));?>');
			is_check = true;
		}
		
		for (var i in name_ary) 
		{
			if(name_ary[i] == routing_name) {
				document.getElementById('routing_name').focus();
				document.getElementById('crouting_name').innerHTML = con_str('Already exist.');
				is_check = true;
			}
		}
		
		document.getElementById('crouting_from').innerHTML="";
		document.getElementById('crouting_to').innerHTML="";
		var $check_boxes = $("input[name='gsm_from_members[]']:checked");
		if ($check_boxes.length<=0){
			document.getElementById('crouting_from').innerHTML = con_str('Please select member');
			$("input[name='gsm_from_members[]']").first().focus();
			is_check = true;
		}
		
		var $check_boxes = $("input[name='gsm_to_members[]']:checked");
		if ($check_boxes.length<=0){
			document.getElementById('crouting_to').innerHTML = con_str('Please select member');
			$("input[name='gsm_to_members[]']").first().focus();
			is_check = true;
		}
		
		//检测这次保存的from_member是否存在于其他routing的from_member
		//array_routings在此页面最上的代码里已经创建过，里面保存了说有的routing里的from_members内容
		//创建本次checked的from_member数组
		var $check_boxes = $("input[name='gsm_from_members[]']:checked");
		var select_checkbox=new Array();
		$check_boxes.each(function(){
            var array_push = new Array();
            select_checkbox.push($(this).val());            
        })
		
		for(var i=0;i<select_checkbox.length;i++){
			var old_routing_name=document.getElementById('old_routing_name').value;
			for (var key in array_routings){			
				if (old_routing_name!=key){
					if (array_routings[key].indexOf(select_checkbox[i])>-1){
						document.getElementById('crouting_from').innerHTML = con_str(select_checkbox[i]+' is aleady in \'From Members\' of Routing '+key+' ');					
						is_check = true;
					}
				}							
			}
		}		
		
		document.getElementById('crouting_number').innerHTML = '';
		if (document.modify_form.to_number.value==""){			
			document.getElementById('to_number').focus();
			document.getElementById('crouting_number').innerHTML = con_str('Please input number');
			is_check = true;
		}
		
		if(is_check){
			return false;
		}
		
		return true;
	}
	
	function dynamic_checkbox(object){
		var source_id=object.id;
		var tmpchar=source_id.charAt(0);
		if (tmpchar=="f"){
			target_id=trim('t'+source_id.substring(1,100));
			var target_row=trim('r_t'+source_id.substring(1,100));
		}else{
			target_id=trim('f'+source_id.substring(1,100));
			var target_row=trim('r_f'+source_id.substring(1,100));
		}
		
		if (modify_form[source_id].checked) {
			modify_form[target_id].checked=false;
			//modify_form[target_id].style.display="none";			
			document.getElementById(target_row).style.display="none";
		}else{
			modify_form[target_id].checked=false;
			//modify_form[target_id].style.display="";
			document.getElementById(target_row).style.display="";
		}
		return 0;
		
	}
	
	//custom trigger functions： add a new line
	function addTriggerRow() {
		var len = tbl_trigger_set.rows.length -1;
		//统计trigger的行数
		var count = len - 1;
		var max_line = 20;
		//添加一行
		if (count < max_line ) {
			var newTr = tbl_trigger_set.insertRow(len);
			var newTd = newTr.insertCell(0);
			newTd.innerHTML = '<?php trigger_settings_table('','')?>';
		} 	
	}
	
	</script>

<form name="modify_form" enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post" onsubmit="return CheckForm()";>
	<input type="hidden" id="order" name="order" value="<?php echo $order?>" />
	<input type="hidden" id="old_routing_name" name="old_routing_name" value="<?php echo $routing_name?>" />

	<div class="content">
		<span class="title">
			<?php echo language('Routing Groups');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Routing Name')){ ?>
							<b><?php echo language('Routing Name');?>:</b><br/>
							<?php echo language('Routing Name help',"The name of this route. Should be used to describe what types of calls this route matches(for example, 'SIP2GSM' or 'GSM2SIP').");?>
							
							<br/><br/>
						<?php } ?>
							
						<?php if(is_show_language_help('Type')){ ?>
							<b><?php echo language('Type');?>:</b><br/>
							<?php echo language('Type help','Type');?>

							<br/><br/>
						<?php } ?>

						<?php if(is_show_language_help('Policy')){ ?>
							<b><?php echo language('Policy');?>:</b><br/>
							<?php echo language('Policy help','Policy');?>

							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('From Members')){ ?>
							<b><?php echo language('From Members');?>:</b><br/>
							<?php echo language('From Members help','From Members');?>

							<br/><br/>
						<?php } ?>

						<?php if(is_show_language_help('To Members')){ ?>
							<b><?php echo language('To Members');?>:</b><br/>
							<?php echo language('To Members help','To Members');?>

							<br/><br/>
						<?php } ?>

						<?php if(is_show_language_help('To Number')){ ?>
							<b><?php echo language('To Number');?>:</b><br/>
							<?php echo language('To Number help','To Number');?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Routing Name');?>:
			</span>
			<div class="tab_item_right">
				<span id="crouting_name"></span>
				<input type="text" name="routing_name" id="routing_name" value="<?php echo $routing_name?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Type');?>:
			</span>
			<div class="tab_item_right">
				<select name="type" id="type" onchange="typechange()">
					<?php
					$show_type[0] = '';
					$show_type[1] = '';
					switch($type) {
						case 'gsm': $show_type[1] = 'selected'; break;
						default: 	$show_type[1] = 'selected'; break;
					}
					?>
					<option value="gsm" <?php echo $show_type[1];?> >MODULE</option>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Policy');?>:
			</span>
			<div class="tab_item_right">
				<select name="policy" id="policy">
					<?php
					$show_policy[0] = '';
					$show_policy[1] = '';
					$show_policy[2] = '';
					$show_policy[3] = '';
					$show_policy[4] = '';
					$show_policy[5] = '';
					$show_policy[6] = '';
					switch($policy) {
						case 'ascending': $show_policy[0] = 'selected'; break;
						case 'roundrobin': $show_policy[1] = 'selected'; break;
						default: $show_policy[0] = 'selected'; break;
					}
					?>
					<option value="ascending" <?php echo $show_policy[0];?> ><?php echo language('Ascending');?></option>					
					<option value="roundrobin" <?php echo $show_policy[1];?> ><?php echo language('Roundrobin');?></option>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('From Members');?>:
			</span>
			<div class="tab_item_right" style="width:300px;">
				<span id="span_gsm_members" >
				<?php gsm_member_show($from_members,$to_members,'from') ?>
				</span>
			</div>
			<div class="clear"></div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('To Members');?>:
			</span>
			<div class="tab_item_right" style="width:300px;">
				<span id="span_gsm_members">
				<?php gsm_member_show($to_members,$from_members,"to") ?>
				</span>
			</div>
			<div class="clear"></div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('To Number');?>:
			</span>
			<div class="tab_item_right">
				<span id="crouting_number"></span>
				<input type="text" name="to_number" id="to_number" value="<?php echo $to_number?>"
				oninput="this.value=this.value.replace(/[^\d]*/g,'')" onkeyup="this.value=this.value.replace(/[^\d]*/g,'')" />
			</div>
		</div>
	</div>
	

	<?php if ($sms_forward_trigger) { ?>
		
		<div class="content">
			<span class="title">
				<?php echo language('Advanced Settings');?>
				
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<?php if(is_show_language_help('Trigger Clear Code')){ ?>
								<b><?php echo language('Trigger Clear Code');?>:</b><br/>
								<?php echo language('Trigger Clear Code help','A specific trigger message code will be used for clearing the preconfigured list.');?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Trigger Configure List')){ ?>
								<b><?php echo language('Trigger Configure List');?>:</b><br/>
								<?php echo language('Trigger Configure List help','Configure "Trigger" and "Send Text".<br>For example: "From Members" receive a SMS text "Trigger" ,"To Members" will send "Send Text" to "To Number".' );?>
							<?php } ?>
						</div>
					</div>
				</div>
			</span>
			
			<div class="content">
				<span class="title"><?php echo language('Trigger Clear Configure');?></span>
				
				<div class="tab_item">
					<span>
						<?php echo language('Trigger Clear Code');?>:
					</span>
					<div class="tab_item_right">
						<span id="ctrigger_clear"></span>
						<input type="text" name="trigger_clear" id="trigger_clear" value="<?php echo $trigger_clear_str;?>" style="width: 300px;" />
					</div>
				</div>
			</div>
			
			<div class="content">
				<span class="title">
					<?php echo language('Trigger Configure List');?>
				</span>
				
				<div class="tab_item">
					<span>
						<?php echo language('Trigger Configure List');?>:
					</span>
					<div class="tab_item_right">
						<?php
						if (isset($trigger_array) && is_array($trigger_array)) {
							foreach($trigger_array as $trigger => $text) {
								echo '<tr><td>';
								trigger_settings_table($trigger,$text);
								echo '</td></tr>';								
							}
						} else {
							echo '<tr><td>';
							trigger_settings_table('','');
							echo '</td></tr>';					
						}

						?>
						
						<tr>
							<td colspan=5 style="text-align:left;">
								<input type="button" id='trigger_add_button' value="+ <?php echo language('Add More Trigger Settings Fields');?>" onclick="addTriggerRow()"/>
							</td>
						</tr>
					</div>
				</div>
			</div>
		</div>
	<?php } ?>
	
	<input type="hidden" name="send" id="send" value="" />
	
	<div id="button_save">
		
		<?php if(!$only_view){ ?>
		<button type="submit" class="float_btn gen_short_btn" onclick="document.getElementById('send').value='Save';return check();"><?php echo language('Save');?></button>
		<?php } ?>
		
		<button type="button" onclick="window.location.href='<?php echo get_self();?>'" ><?php echo language('Cancel');?></button>
	</div>
</form>

<script type="text/javascript">
$(document).ready(function(){
	onload_func();
});

</script>

<?php
}
?>


<?php
$check_float = 0;
	if($_POST) {
		$sms_routing_dir = "/my_tools/lua/sms_routing/";
		if( (isset($_POST['send']) && ($_POST['send'] == 'New Routing') ) ) {
			//Add new
			if( isset($_POST['sel_routing_name']) && isset($_POST['order']) && $_POST['order'] ) {
				$check_float = 1;
				add_group_page($_POST['sel_routing_name'],$_POST['order']);
							}
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Save') {
			if($only_view){
				return false;
			}
			
			save_routings();
			
			show_groups();
			
			exec("ps aux |grep sms_routing.lua|grep -v grep| awk '{print $1}'",$pid);					
			for ($i=0;$i<count($pid);$i++){			
				exec("kill -s 9 ".$pid[$i]);							
			}
			exec("cd "."$sms_routing_dir".";lua sms_routing.lua >/dev/null 2>&1 &");
	
			
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Apply') {
			if($only_view){
				return false;
			}
			
			save_routings();
			$check_float = 1;
			add_group_page($_POST['routing_name'],'');
	
			exec("ps aux |grep sms_routing.lua|grep -v grep| awk '{print $1}'",$pid);					
			for ($i=0;$i<count($pid);$i++){			
				exec("kill -s 9 ".$pid[$i]);							
			}
			exec("cd "."$sms_routing_dir".";lua sms_routing.lua >/dev/null 2>&1 &");
	
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Delete') {
			if($only_view){
				return false;
			}
			
			if(isset($_POST['sel_routing_name']) && $_POST['sel_routing_name']) {
				del_group($_POST['sel_routing_name']);
				show_groups();
				exec("ps aux |grep sms_routing.lua|grep -v grep| awk '{print $1}'",$pid);					
				for ($i=0;$i<count($pid);$i++){			
					exec("kill -s 9 ".$pid[$i]);							
				}
				exec("cd "."$sms_routing_dir".";lua sms_routing.lua >/dev/null 2>&1 &");
			}
		}
	} else if($_GET) {
		//Modify
		if( isset($_GET['sel_routing_name']) ) {
			$check_float = 1;
			add_group_page($_GET['sel_routing_name'],'');			
		}
	} else {
		show_groups();
	}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>