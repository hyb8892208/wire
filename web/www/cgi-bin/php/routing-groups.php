<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
require_once("/www/cgi-bin/inc/aql.php");
?>

<?php
function show_groups()
{
	global $__BRD_HEAD__;
	global $__GSM_HEAD__;
	$all_groups = get_all_groups(true);
	$last_order = 1;
	if($all_groups) {
		$last = end($all_groups); 
		if(isset($last['order']) && $last['order'] != '') {
			$last_order = $last['order'] + 1;
		}
	}
?>

	<script type="text/javascript">
	function getPage(value)
	{
		window.location.href = '<?php echo get_self();?>?sel_group_name='+value;
	}

	function setValue(value1,value2)
	{
		document.getElementById('sel_group_name').value = value1;
		document.getElementById('order').value = value2;
	}

	function delete_click(value1,value2)
	{
		ret = confirm("Are you sure to delete you selected ?");

		if(ret) {
			document.getElementById('sel_group_name').value = value1;
			document.getElementById('order').value = value2;
			return true;
		}

		return false;
	}
	</script>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<input type="hidden" id="sel_group_name" name="sel_group_name" value="" />
	<input type="hidden" id="order" name="order" value="" />

	<table width="100%" id="tab_groups" class="tdrag">
		<tr>
			<th width="120px"><?php echo language('Group Name');?></th>
			<th width="50px"><?php echo language('Type');?></th>
			<th width="80px"><?php echo language('Policy');?></th>
			<th><?php echo language('Members');?></th>
			<th width="80px"><?php echo language('Actions');?></th>
		</tr>

<?php
	if($all_groups) {
		foreach($all_groups as $group) {
			$group_name = trim($group['group_name']);
			$members = trim($group['members']);	
			$type = trim($group['type']);
			if ($type == 'gsm' || $type == 'umts'){
				$type = 'module';
			}
			$policy = trim($group['policy']);
			switch($policy){
				case 'ascending':
					$policy_name = language('Ascending');
					break;
				case 'descending':
					$policy_name = language('Descending');
					break;
				case 'ringall':
					$policy_name = language('Ringall');
					break;
				case 'roundrobin':
					$policy_name = language('Roundrobin');
					break;
				case 'reverseroundrobin':
					$policy_name = language('Reverse Roundrobin');
					break;
			}

			$show_members = '';
			$output = explode(',',$members);
			foreach($output as $ch) {
				$name = get_channel_name($ch);
				$show_members .= $name;
				$show_members .= ', ';
			}
			$show_members = rtrim($show_members,', ');
?>

		<tr bgcolor="#E8EFF7">
			<td>
				<?php echo $group_name ?>
			</td>
			<td>
				<?php echo $type ?>
			</td>
			<td>
				<?php echo $policy_name; ?>
			</td>
			<td>
				<?php echo $show_members ?>
			</td>
			<td>
				<button type="button" value="Modify" style="width:32px;height:32px;" 
					onclick="getPage('<?php echo $group_name ?>')">
					<img src="/images/edit.gif">
				</button>
				<button type="submit" value="Delete" style="width:32px;height:32px;" 
					onclick="document.getElementById('send').value='Delete';return delete_click('<?php echo $group_name ?>', '')" >
					<img src="/images/delete.gif">
				</button>
				<input type="hidden" name="group_name[]" value="<?php echo $group_name ?>" />
			</td>
		</tr>
<?php
		}
	}
?>
	</table>
	<br>
	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" value="<?php echo language('New Group');?>" onclick="document.getElementById('send').value='New Group';setValue('', '<?php echo $last_order ?>')" />
	</form>
<?php
}
?>

<?php
function save_groups()
{
	// /etc/asterisk/gw_group.conf
	// [group_name]
	// order = 1,2,3,4,5....    //Must set
	// type = sip or gsm  //Must set
	// policy = 
	// members = channel,channel......   //Must set

	$datachunk = '';

	//group name already existed! 

	if( isset($_POST['group_name']) ) {
		$group_name = trim($_POST['group_name']);
		if($group_name == '') {
			echo "Must set group name";
			return false;
		}
		$section = $group_name;
	} else {
		echo "Must set group name";
		return false;
	}

	$old_section = $section;
	if( isset($_POST['old_group_name']) ) {
		$old_section = trim($_POST['old_group_name']);
	}

	if( isset($_POST['order']) ) {
		$order = trim($_POST['order']);
		if($order == '') {
			echo "[$group_name] ";
			echo language('does not exist');
			//echo "Must set order";
			return false;
		}
		$datachunk .= 'order='.$order."\n";
	} else {
		echo "[$group_name] ";
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

	if($type=='gsm') {
		$member_type = 'gsm_members';
	} else if($type=='sip') {
		$member_type = 'sip_members';
	} else if ($type=='iax'){
		$member_type = 'iax_members';
	} else {
		echo "Please input correct type.";
		return false;
	}

	$members = '';
	if( isset($_POST[$member_type]) && is_array($_POST[$member_type]) ) {
		foreach($_POST[$member_type] as $value) {
			$value = trim($value);
			if($value != '') {
				$members .= $value.',';
			}
		}
	}

	if($members == '') {
		echo "Must set members";
		return false;
	}
	
	$members = rtrim($members,',');
	$datachunk .= 'members='.$members."\n";

	//Save to gw_group.conf
	///////////////////////////////////////////////////
	$gw_group_conf_path = "/etc/asterisk/gw_group.conf";
	$hlock = lock_file($gw_group_conf_path);
	if (!file_exists($gw_group_conf_path)) fclose(fopen($gw_group_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($gw_group_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	$aql->assign_delsection($old_section);
	$aql->save_config_file('gw_group.conf');
	$aql->assign_addsection($section,$datachunk);
	$aql->save_config_file('gw_group.conf');
	unlock_file($hlock);
	///////////////////////////////////////////////////

	save_gsm_to_extra_conf();
	save_routings_to_extensions();

	return true;
}
?>

<?php
function del_group($group_name)
{
	//Save to gw_group.conf
	///////////////////////////////////////////////////
	// /etc/asterisk/gw_group.conf
	$gw_group_conf_path = "/etc/asterisk/gw_group.conf";
	$hlock = lock_file($gw_group_conf_path);
	if (!file_exists($gw_group_conf_path)) fclose(fopen($gw_group_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($gw_group_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	$aql->assign_delsection($group_name);
	$aql->save_config_file('gw_group.conf');
	unlock_file($hlock);
	///////////////////////////////////////////////////

	save_routings_to_extensions();

}
?>


<?php
function gsm_member_show($members)
{
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	global $__GSM_SUM__;
	global $__GSM_HEAD__;
	
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
	echo "<input type=\"checkbox\" name=\"selgsmall\" onclick=\"selectAll(this.checked,'gsm_members[]')\" />All";
	echo "</td>";
	echo "</tr>";

	//GSM
	for($i=1; $i<=$__GSM_SUM__; $i++) {
		$value = get_gsm_value_by_channel($i);
		$name = get_gsm_name_by_channel($i);
		if(strstr($name, 'null')){continue;}
		
		$checked = '';
		if(is_array($members)){
			foreach($members as $each) {
				if($each === $value) {
					$checked = 'checked';
					break;
				}
			}
		}
		echo "<tr style=\"border:none;\">";
		echo "<td style=\"border:none;\">";
		echo "$num";
		echo "</td>";
		echo "<td style=\"border:none;\">";
		echo "<input type=\"checkbox\" name=\"gsm_members[]\" value=\"$value\" $checked>";
		echo $name;
		echo "</td>";
		echo "</tr>";
		$num++;
	}

	echo "</table>";
}


function sip_member_show($members)
{
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	global $__SIP_HEAD__;

	//SIP
	/* /etc/asterisk/sip_endpoints.conf */
	$all_sips = get_all_sips();
	if($all_sips) {

		echo "<table  cellpadding=\"0\" cellspacing=\"0\" style=\"border:none;\" >";

		echo "<tr style=\"border:none;\">";
		echo "<td style=\"border:none;\">";
		echo "NO.";
		echo "</td>";
		echo "<td style=\"border:none;\">";
		echo "<input type=\"checkbox\" name=\"selsipall\" onclick=\"selectAll(this.checked,'sip_members[]')\" />All";
		echo "</td>";
		echo "</tr>";


		$num = 1;
		foreach($all_sips as $sip) {

			$endpoint_name = trim($sip['endpoint_name']);
			$value = get_sip_name_has_head($endpoint_name);

			$checked = '';
			foreach($members as $each) {
				if($each === $value) {
					$checked = 'checked';
					break;
				}
			}
			echo "<tr style=\"border:none;\">";
			echo "<td style=\"border:none;\">";
			echo "$num";
			echo "</td>";
			echo "<td style=\"border:none;\">";
			echo "<input type=\"checkbox\" name=\"sip_members[]\" value=\"$value\" $checked>";
			echo $value;
			echo "</td>";
			echo "</tr>";
			$num++;
		}

		echo "</table>";
	}
}

function iax_member_show($members)
{
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	global $__IAX_HEAD__;

	//SIP
	/* /etc/asterisk/sip_endpoints.conf */
	$all_iaxs = get_all_iaxs();
	if($all_iaxs) {

		echo "<table  cellpadding=\"0\" cellspacing=\"0\" style=\"border:none;\" >";

		echo "<tr style=\"border:none;\">";
		echo "<td style=\"border:none;\">";
		echo "NO.";
		echo "</td>";
		echo "<td style=\"border:none;\">";
		echo "<input type=\"checkbox\" name=\"selsipall\" onclick=\"selectAll(this.checked,'iax_members[]')\" />All";
		echo "</td>";
		echo "</tr>";


		$num = 1;
		foreach($all_iaxs as $iax) {

			$endpoint_name = trim($iax['endpoint_name']);
			$value = get_iax_name_has_head($endpoint_name);

			$checked = '';
			foreach($members as $each) {
				if($each === $value) {
					$checked = 'checked';
					break;
				}
			}
			echo "<tr style=\"border:none;\">";
			echo "<td style=\"border:none;\">";
			echo "$num";
			echo "</td>";
			echo "<td style=\"border:none;\">";
			echo "<input type=\"checkbox\" name=\"iax_members[]\" value=\"$value\" $checked>";
			echo $value;
			echo "</td>";
			echo "</tr>";
			$num++;
		}

		echo "</table>";
	}
}

function add_group_page($group_name,$order = '')
{
	if($group_name) {
		echo "<h4>";echo language('Modify a Group');echo "</h4>";
	} else {
		echo "<h4>";echo language('Create a Group');echo "</h4>";
	}

	// /etc/asterisk/gw_group.conf
	// [group_name]
	// type = type (sip or gsm)
	// policy =
	// members = channel,channel......

	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk');
	if (!$setok) {
		return;
	}

	$section = $group_name;

	if($section) {
		$hlock = lock_file('/etc/asterisk/gw_group.conf');
		$res = $aql->query("select * from gw_group.conf where section='$section'");
		unlock_file($hlock);
	}

	if(isset($res[$section]['order'])) {
		if($group_name) {
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

	if(isset($res[$section]['members'])) {
		$val = trim($res[$section]['members']);
		$members = explode(',',$val);
		if(!isset($members[0])) {
			$members = array();
		}
	} else {
		$members = array();
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
			set_visible('span_iax_members',false);
			set_visible('roundrobin',false);
			set_visible('reverseroundrobin',false);
			set_visible('fewestcalls',false);
			set_visible('ringall',true);
		} else if (type == 'iax') {
			set_visible('span_gsm_members', false);
			set_visible('span_sip_members', false);	
			set_visible('span_iax_members',true);
			set_visible('roundrobin',false);
			set_visible('reverseroundrobin',false);
			set_visible('fewestcalls',false);			
			set_visible('ringall',false);
		} else if (type == 'gsm') {
			set_visible('span_sip_members', false);
			set_visible('span_gsm_members', true);	
			set_visible('span_iax_members',false);
			set_visible('roundrobin',true);
			set_visible('reverseroundrobin',true);
			set_visible('fewestcalls',true);
			set_visible('ringall',false);
		}
	}
	
	function onload_func()
	{
		typechange();
	}

	function check()
	{
	<?php
		$agroups = get_all_groups();

		$name_ary = '';
		if($agroups) {
			foreach($agroups as $group) {		
				if(strcmp($group['group_name'],$group_name)==0)
					continue;
				$name_ary .=  '"'.$group['group_name'].'"'.',';
			}
		}
		$name_ary = rtrim($name_ary,',');
	?>
		var name_ary = new Array(<?php echo $name_ary; ?>);
		var group_name = document.getElementById('group_name').value;

		if(!check_routingname(group_name)) {
			document.getElementById("cgroup_name").innerHTML = con_str('<?php echo htmlentities(language('js check groupname','Allowed character must be any of [0-9a-zA-Z`~!@#$%^*()_{}:|?-=.], 1-32 characters.'));?>');
			return false;
		}
		
		document.getElementById('cgroup_name').innerHTML = '';
		for (var i in name_ary) 
		{
			if(name_ary[i] == group_name) {
				document.getElementById('cgroup_name').innerHTML = con_str('Already exist.');
				return false;
			}
		}

		return true;
	}
	</script>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<input type="hidden" id="order" name="order" value="<?php echo $order?>" />
	<input type="hidden" id="old_group_name" name="old_group_name" value="<?php echo $group_name?>" />

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Routing Groups');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tedit" >
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Group Name');?>:
					<span class="showhelp">
					<?php echo language('Group Name help',"The name of this route. Should be used to describe what types of calls this route matches(for example, 'SIP2GSM' or 'GSM2SIP').");?>
					</span>
				</div>
			</th>
			<td >
				<input type="text" name="group_name" id="group_name" value="<?php echo $group_name?>" /><span id="cgroup_name"></span>
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
				<select name="type" id="type">
<?php
					$show_type[0] = '';
					$show_type[1] = '';
					$show_type[2] = '';
					switch($type) {
						case 'sip': $show_type[0] = 'selected'; break;
						case 'gsm': $show_type[1] = 'selected'; break;
						//case 'iax': $show_type[2] = 'selected'; break;
						default: 	$show_type[1] = 'selected'; break;
					}
?>
					<option value="sip" <?php echo $show_type[0];?> >SIP</option>
					<option value="gsm" <?php echo $show_type[1];?> >MODULE</option>
					<!--<option value="iax" <?php echo $show_type[2];?> >IAX2</option>-->
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
					$show_policy[7] = '';
					switch($policy) {
						case 'ascending': $show_policy[0] = 'selected'; break;
						case 'descending': $show_policy[1] = 'selected'; break;
						case 'roundrobin': $show_policy[2] = 'selected'; break;
						case 'reverseroundrobin': $show_policy[3] = 'selected'; break;
						case 'leastrecent': $show_policy[4] = 'selected'; break;
						case 'fewestcalls': $show_policy[5] = 'selected'; break;
						case 'random': $show_policy[6] = 'selected'; break;
						case 'ringall': $show_policy[7] = 'selected'; break;
						default: $show_policy[0] = 'selected'; break;
					}
?>
					<option value="ascending" <?php echo $show_policy[0];?> ><?php echo language('Ascending');?></option>
					<option value="descending" <?php echo $show_policy[1];?> ><?php echo language('Descending');?></option>
					<option id="ringall" value="ringall" <?php echo $show_policy[7];?> ><?php echo language('Ringall');?></option>
					<option id="roundrobin" value="roundrobin" <?php echo $show_policy[2];?> ><?php echo language('Roundrobin');?></option>
					<option id="reverseroundrobin" value="reverseroundrobin" <?php echo $show_policy[3];?> ><?php echo language('Reverse Roundrobin');?></option>
					<!-- WGWUSB-466 <option id="fewestcalls" value="fewestcalls" <?php echo $show_policy[5];?> ><?php echo language('Fewest Calls');?>(*<?php echo language('experiment');?>)</option> -->
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Members');?>
					<span class="showhelp">
					<?php echo language('Members help');?>
					</span>
				</div>
			</th>
			<td >
				<span id="span_gsm_members" style="display: none;" >
				<?php gsm_member_show($members) ?>
				</span>

				<span id="span_sip_members" style="display: none;" >
				<?php sip_member_show($members) ?>
				</span>
				<span id="span_iax_members" style="display: none;" >
				<?php iax_member_show($members) ?>
				</span>
			</td>
		</tr>
	</table>

	<br />

	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" class="float_btn gen_short_btn"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>
	&nbsp;
	<input type="submit" class="float_btn gen_short_btn"  value="<?php echo language('Apply');?>" onclick="document.getElementById('send').value='Apply';return check();"/>
	&nbsp;
	<input type=button  value="<?php echo language('Cancel');?>" onclick="window.location.href='<?php echo get_self();?>'" />
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td style="width:50px">
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" />
			</td>
			<td style="width:50px">
				<input type="submit" id="float_button_3" class="float_short_button" value="<?php echo language('Apply');?>" onclick="document.getElementById('send').value='Apply';return check();" />
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
	
	$("#type").change(function(){
		typechange();
		
		var type = '<?php echo $type;?>';
		var cur_type = document.getElementById('type').value;
		if(type == cur_type){
			$("#policy").val('<?php echo $policy;?>');
		}else {
			$("#policy").val('ascending');
		}
	});
</script>
<div id="sort_out" class="sort_out">
</div>
	<div class="sort_gsm_cli" id="sort_gsm_cli">
		<div id="sort_info" class="sort_info" style="display:block">
		<?php
	$all_groups = get_all_groups(true);
	$groups_num = 0;
	if($all_groups) {
		foreach($all_groups as $group) {
			$groups_num+=1;
			$group['group_name'] = trim($group['group_name']);
			if($group['group_name'] == $group_name){
		?>
				<li><a style="color:#CD3278;" href="<?php echo get_self();?>?sel_group_name=<?php echo $group['group_name']; ?>" ><?php echo $group['group_name']; ?></a></li>
		<?php
			}else{
		?>
				<li><a style="color:LemonChiffon4;" href="<?php echo get_self();?>?sel_group_name=<?php echo $group['group_name']; ?>" ><?php echo $group['group_name']; ?></a></li>
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
	if($_POST) {
		if( (isset($_POST['send']) && ($_POST['send'] == 'New Group') ) ) {
			//Add new
			if( isset($_POST['sel_group_name']) && isset($_POST['order']) && $_POST['order'] ) {
				$check_float = 1;
				add_group_page($_POST['sel_group_name'],$_POST['order']);
	exec("cd /my_tools/lua/sms_routing; lua read_group.lua");
			}
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Save') {
			if(save_groups()) {
				//ast_reload();
				wait_apply("exec","asterisk -rx \"core reload\" > /dev/null 2>&1 &");
			}
			show_groups();
	exec("cd /my_tools/lua/sms_routing; lua read_group.lua");
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Apply') {
			if(save_groups()) {
				//ast_reload();
				wait_apply("exec","asterisk -rx \"core reload\" > /dev/null 2>&1 &");
			}
			$check_float = 1;
			add_group_page($_POST['group_name'],'');
	exec("cd /my_tools/lua/sms_routing; lua read_group.lua");
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Delete') {
			if(isset($_POST['sel_group_name']) && $_POST['sel_group_name']) {
				del_group($_POST['sel_group_name']);
				show_groups();
	exec("cd /my_tools/lua/sms_routing; lua read_group.lua");
				//ast_reload();
				wait_apply("exec","asterisk -rx \"core reload\" > /dev/null 2>&1 &");
			}
		}
	} else if($_GET) {
		//Modify
		if( isset($_GET['sel_group_name']) ) {
			$check_float = 1;
			add_group_page($_GET['sel_group_name'],'');
	exec("cd /my_tools/lua/sms_routing; lua read_group.lua");
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
