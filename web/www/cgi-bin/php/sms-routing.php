<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
require_once("/www/cgi-bin/inc/aql.php");
?>
<script type="text/javascript" src="../../js/jquery.js"></script>
<?php 
	//��javascript �ڴ��ﴴ��sms-routing.conf��������from_members�ֶεľ����array_routings	
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

	<table width="100%" id="tab_groups" class="tdrag">
		<tr>
			<th width="120px"><?php echo language('Routing Name');?></th>
			<th width="50px"><?php echo language('Type');?></th>
			<th width="80px"><?php echo language('Policy');?></th>
			<th><?php echo language('From_Members');?></th>
			<th><?php echo language('To_Members');?></th>
			<th width="80px"><?php echo language('To Number');?></th>
			<th width="80px"><?php echo language('Actions');?></th>
		</tr>

<?php
	if($all_sms_routings) {
		foreach($all_sms_routings as $group) {
			$routing_name = trim($group['routing_name']);
			$from_members = trim($group['from_member']);	
			$to_members = trim($group['to_member']);	
			//$type = trim($group['type']);
			$type = "module";
			$policy = trim($group['policy']);
            $to_number=trim($group['to_number']);
			$show_members = '';
			$from_members = rtrim($from_members,', ');
?>

		<tr bgcolor="#E8EFF7">
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
				<?php 
					echo change_sms_routing_form_new($from_members);
					//echo change_sms_routing_form($from_members);
				    //echo str_replace('gsm','gsm-',$from_members) ?>
			</td>
			<td>
				<?php 
					echo change_sms_routing_form_new($to_members);
					//echo change_sms_routing_form($to_members);
					//echo str_replace('gsm','gsm-',$to_members) 
				?>
			</td>
			<td>
				<?php echo $to_number ?>
			</td>
			<td>
				<button type="button" value="Modify" style="width:32px;height:32px;" 
					onclick="getPage('<?php echo $routing_name ?>')">
					<img src="/images/edit.gif">
				</button>
				<button type="submit" value="Delete" style="width:32px;height:32px;" 
					onclick="document.getElementById('send').value='Delete';return delete_click('<?php echo $routing_name ?>', '')" >
					<img src="/images/delete.gif">
				</button>
				<input type="hidden" name="routing_name[]" value="<?php echo $routing_name ?>" />
			</td>
		</tr>
<?php
		}
	}
?>
	</table>
	<br>
	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" value="<?php echo language('New Routing');?>" onclick="document.getElementById('send').value='New Routing';setValue('', '<?php echo $last_order ?>')" />
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
	
	
	//custom trigger functions�� save web configurations to sms_routing.conf
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
custom trigger functions��draw "trigger settings" web configurations table
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
	$type_show = $__MODULE_HEAD_ARRAY__[1][1];
	$type_show = strtoupper(str_replace("-","",$type_show));
	if($routing_name) {
		echo "<h4>";echo language('Modify a Routing');echo "</h4>";
	} else {
		echo "<h4>";echo language('Create a Routing');echo "</h4>";
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


	
	//custom trigger functions�� get trigger infomations from sms_routing.conf
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
	<script type="text/javascript" src="/js/float_btn.js"></script>
	

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
		

		if(!check_routingname(routing_name)) {
			document.getElementById("crouting_name").innerHTML = con_str('<?php echo htmlentities(language('js check groupname','Allowed character must be any of [0-9a-zA-Z`~!@#$%^*()_{}:|?-=.], 1-32 characters.'));?>');
			return false;
		}
		
		
		document.getElementById('crouting_name').innerHTML = '';
		for (var i in name_ary) 
		{
			if(name_ary[i] == routing_name) {
				document.getElementById('crouting_name').innerHTML = con_str('Already exist.');
				return false;
			}
		}
		
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
		
		return true;
	}
	
	function CheckForm(){
		
		document.getElementById('crouting_from').innerHTML="";
		document.getElementById('crouting_to').innerHTML="";
		var $check_boxes = $("input[name='gsm_from_members[]']:checked");
		if ($check_boxes.length<=0){
			document.getElementById('crouting_from').innerHTML = con_str('Please select member');
			return false;
		}
		
		var $check_boxes = $("input[name='gsm_to_members[]']:checked");
		if ($check_boxes.length<=0){
			document.getElementById('crouting_to').innerHTML = con_str('Please select member');
			return false;
		}
		
		//�����α����from_member�Ƿ����������routing��from_member
		//array_routings�ڴ�ҳ�����ϵĴ������Ѿ������������汣����˵�е�routing���from_members����
		//��������checked��from_member����
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
						return false;
					}					
				}							
			}
		}		
		
		
		
		if (document.modify_form.to_number.value==""){			
			$("to_number").focus();
			document.getElementById('crouting_number').innerHTML = con_str('Please input number');
			return false
		}	
        
				
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
	
	//custom trigger functions�� add a new line
	function addTriggerRow() {
		var len = tbl_trigger_set.rows.length -1;
		//ͳ��trigger������
		var count = len - 1;
		var max_line = 20;
		//���һ��
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

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Routing Groups');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tedit" >
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Routing Name');?>:
					<span class="showhelp">
					<?php echo language('Routing Name help',"The name of this route. Should be used to describe what types of calls this route matches(for example, 'SIP2GSM' or 'GSM2SIP').");?>
					</span>
				</div>
			</th>
			<td >
				<input type="text" name="routing_name" id="routing_name" value="<?php echo $routing_name?>" /><span id="crouting_name"></span>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Type');?>:
					<span class="showhelp">
					<?php echo language('Type help');?>
					</span>
				</div>
			</th>
			<td >
				<select name="type" id="type" onchange="typechange()">
<?php
					$show_type[0] = '';
					$show_type[1] = '';
					switch($type) {
						//case 'sip': $show_type[0] = 'selected'; break;
						case 'gsm': $show_type[1] = 'selected'; break;
						default: 	$show_type[1] = 'selected'; break;
					}
?>					<!--
					<option value="sip" <?php echo $show_type[0];?> >SIP</option>
					<option value="gsm" <?php echo $show_type[1];?> ><?php echo $type_show;?></option>
					-->
					<option value="gsm" <?php echo $show_type[1];?> >MODULE</option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Policy');?>:
					<span class="showhelp">
					<?php echo language('Policy help');?>
					</span>
				</div>
			</th>
			<td >
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
						//case 'descending': $show_policy[1] = 'selected'; break;
						case 'roundrobin': $show_policy[1] = 'selected'; break;
						//case 'reverseroundrobin': $show_policy[3] = 'selected'; break;
						//case 'leastrecent': $show_policy[4] = 'selected'; break;
						//case 'fewestcalls': $show_policy[5] = 'selected'; break;
						//case 'random': $show_policy[6] = 'selected'; break;
						default: $show_policy[0] = 'selected'; break;
					}
?>
					<option value="ascending" <?php echo $show_policy[0];?> ><?php echo language('Ascending');?></option>					
					
					<option value="roundrobin" <?php echo $show_policy[1];?> ><?php echo language('Roundrobin');?></option>
					<!--
					<option value="descending" <?php echo $show_policy[1];?> ><?php echo language('Descending');?></option>
					<option value="reverseroundrobin" <?php echo $show_policy[3];?> ><?php echo language('Reverse Roundrobin');?></option>
					<option value="leastrecent" <?php echo $show_policy[4];?> ><?php echo language('Least Recent');?>(*<?php echo language('experiment');?>)</option>
					<option value="fewestcalls" <?php echo $show_policy[5];?> ><?php echo language('Fewest Calls');?>(*<?php echo language('experiment');?>)</option>
					<option value="random" <?php echo $show_policy[6];?> ><?php echo language('Random');?></option>
					-->
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('From Members');?>
					<span class="showhelp">
					<?php echo language('From Members help');?>
					</span>
				</div>
			</th>
			<td >
				<span id="span_gsm_members" >
				<?php gsm_member_show($from_members,$to_members,'from') ?>
				</span>


			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('To Members');?>
					<span class="showhelp">
					<?php echo language('To Members help');?>
					</span>
				</div>
			</th>
			<td >
				<span id="span_gsm_members">
				<?php gsm_member_show($to_members,$from_members,"to") ?>
				</span>

			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('To Number');?>:
					<span class="showhelp">
					<?php echo language('To Number help');?>
					</span>
				</div>
			</th>
			<td >
				<input type="text" name="to_number" id="to_number" value="<?php echo $to_number?>" /><span id="crouting_number"></span>
			</td>
		</tr>
	</table>

	<br>
	<?php if ($sms_forward_trigger) { ?>
	
	<div id="tab">
		<li class="tb_fold" onclick="lud(this,'tab_advance')" id="tab_advance_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_advance')"><?php echo language('Advanced Settings');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_advance')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>

	<div id="tab_advance" style="<?php echo $show_adv;?>">

		<table width="98%" class="tedit" align="right" >
			<tr class="ttitle">
				<td colspan = 2>
					<?php echo language('Trigger Clear Configure');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Trigger Clear Code');?>
						<span class="showhelp">
						<?php echo language('Trigger Clear Code help','A specific trigger message code will be used for clearing the preconfigured list.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="trigger_clear" id="trigger_clear" value="<?php echo $trigger_clear_str;?>" style="width: 300px;" /><span id="ctrigger_clear"></span>
				</td>
			</tr>
		</table>
		
		<div id="newline"></div>

		<table width="98%" class="tedit" align="right" id="tbl_trigger_set" >
			<tr class="ttitle">
				<td>
					<div class="helptooltips">
						<?php echo language('Trigger Configure List');?>
						<span class="showhelp">
						<?php echo language('Trigger Configure List help','Configure "Trigger" and "Send Text".<br>For example: "From Members" receive a SMS text "Trigger" ,"To Members" will send "Send Text" to "To Number".' );
						?>
						</span>
						<span id="conf_list"></span>
					</div>
				</td>
			</tr>
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
		</table>
	</div>

	<div id="newline"></div>

	<br>
	<?php } ?>
	
	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" class="float_btn gen_short_btn"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>
	<input type=button  value="<?php echo language('Cancel');?>" onclick="window.location.href='<?php echo get_self();?>'" />
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td style="width:50px">
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" />
			</td>
			<td>
				<input type="button" id="float_button_2" class="float_short_button" value="<?php echo language('Cancel');?>" onclick="window.location.href='<?php echo get_self();?>'" />
			</td>
		</tr>
	</table>
	</form>

<script type="text/javascript">




	$(document).ready(function (){
		$("#float_button_3").mouseover(function(){
		  $("#float_button_3").css({opacity:"1",filter:"alpha(opacity=100)"});
		});
		$("#float_button_3").mouseleave(function(){
		  $("#float_button_3").css({opacity:"0.5",filter:"alpha(opacity=50)"});
		});
		float_sort_hide();
		var sort_info_top = $("#lps").offset().top;
		$("#sort_gsm_cli").offset({top: sort_info_top });
		$("#sort_out").offset({top: sort_info_top });
		$("#sort_out").mouseover(function(){
			if($("#sort_out").offset().left <= 5){
		   		float_sort_on();
			}
		});
		$("#sort_gsm_cli").mouseleave(function(){
			float_sort_hide();
		});
	});
	function float_sort_hide()
	{
		$("#sort_gsm_cli").stop().animate({left:"-198px"}, 300);
		$("#sort_out").stop().animate({left:"0px"}, 300);
	};
	function float_sort_on()
	{
		$("#sort_gsm_cli").animate({left:"0px"}, 300);
		$("#sort_out").animate({left:"198px"}, 300);
	};
</script>
<div id="sort_out" class="sort_out">
</div>
	<div class="sort_gsm_cli" id="sort_gsm_cli">
		<div id="sort_info" class="sort_info" style="display:block">
		<?php
	$all_groups = get_all_sms_routing(true);
	$groups_num = 0;
	if($all_groups) {
		foreach($all_groups as $group) {
			$groups_num+=1;
			$group['routing_name'] = trim($group['routing_name']);
			if($group['routing_name'] == $routing_name){
		?>
				<li><a style="color:#CD3278;" href="<?php echo get_self();?>?sel_routing_name=<?php echo $group['routing_name']; ?>" ><?php echo $group['routing_name']; ?></a></li>
		<?php
			}else{
		?>
				<li><a style="color:LemonChiffon4;" href="<?php echo get_self();?>?sel_routing_name=<?php echo $group['routing_name']; ?>" ><?php echo $group['routing_name']; ?></a></li>
		<?php
			}
		}
	}
//Control the left navigation hidden or height by '$groups_num'.
	if($groups_num==0){
?>
<script type="text/javascript">
	$("#sort_out").hide();
	$("#sort_info").hide();
</script>
	<?php
	}elseif($groups_num <= 5){
	?>
<script type="text/javascript">
$(document).ready(function(){
	$("#sort_info").css("height","120px");
});

</script>
<?php
	}
		?>
		</div>
	</div>
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
$product_type = get_product_type();
	if($_POST) {
		$sms_routing_dir = "/my_tools/lua/sms_routing/";
		if( (isset($_POST['send']) && ($_POST['send'] == 'New Routing') ) ) {
			//Add new
			if( isset($_POST['sel_routing_name']) && isset($_POST['order']) && $_POST['order'] ) {
				$check_float = 1;
				add_group_page($_POST['sel_routing_name'],$_POST['order']);
							}
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Save') {
			
			save_routings();
			
			show_groups();
			
			if($product_type < 4){
				exec("ps -w |grep sms_routing.lua|grep -v grep| awk '{print $1}'",$pid);
			}else{
				exec("ps aux |grep sms_routing.lua|grep -v grep| awk '{print $1}'",$pid);
			}
			for ($i=0;$i<count($pid);$i++){			
				exec("kill -s 9 ".$pid[$i]);							
			}
			exec("cd "."$sms_routing_dir".";lua sms_routing.lua >/dev/null 2>&1 &");
	
			
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Apply') {
			save_routings();
			$check_float = 1;
			add_group_page($_POST['routing_name'],'');
	
			if($product_type < 4){
				exec("ps -w |grep sms_routing.lua|grep -v grep| awk '{print $1}'",$pid);
			}else{
				exec("ps aux |grep sms_routing.lua|grep -v grep| awk '{print $1}'",$pid);
			}
			for ($i=0;$i<count($pid);$i++){			
				exec("kill -s 9 ".$pid[$i]);							
			}
			exec("cd "."$sms_routing_dir".";lua sms_routing.lua >/dev/null 2>&1 &");
	
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Delete') {
			if(isset($_POST['sel_routing_name']) && $_POST['sel_routing_name']) {
				del_group($_POST['sel_routing_name']);
				show_groups();
				
				if($product_type < 4){
					exec("ps -w |grep sms_routing.lua|grep -v grep| awk '{print $1}'",$pid);
				}else{
					exec("ps aux |grep sms_routing.lua|grep -v grep| awk '{print $1}'",$pid);
				}
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
<?php
	if($check_float == 1){
?>
	<div id="float_btn1" class="float_btn1 sec_float_btn1">
	</div>
	<div  class="float_close" onclick="close_btn()" >
	</div>
<?php	
	}
?>

