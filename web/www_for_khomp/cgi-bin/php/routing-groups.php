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
	global $only_view;
	
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
		ret = confirm("<?php echo language('Delete confirm','Are you sure to delete you selected ?');?>");

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

	<div class="content">
		<table class="table_show">
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

					$show_members = '';
					$output = explode(',',$members);
					foreach($output as $ch) {
						$name = get_channel_name($ch);
						$show_members .= $name;
						$show_members .= ', ';
					}
					$show_members = rtrim($show_members,', ');
			?>

				<tr>
					<td>
						<?php echo $group_name ?>
					</td>
					<td>
						<?php echo $type ?>
					</td>
					<td>
						<?php echo $policy ?>
					</td>
					<td>
						<?php echo $show_members ?>
					</td>
					<td>
						<button type="button" value="Modify" style="width:32px;height:32px;padding:0;" 
							onclick="getPage('<?php echo $group_name ?>')">
							<img src="/images/edit.gif">
						</button>
						
						<?php if(!$only_view){ ?>
						<button type="submit" value="Delete" style="width:32px;height:32px;padding:0;" 
							onclick="document.getElementById('send').value='Delete';return delete_click('<?php echo $group_name ?>', '')" >
							<img src="/images/delete.gif">
						</button>
						<?php } ?>
						
						<input type="hidden" name="group_name[]" value="<?php echo $group_name ?>" />
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
		<button type="submit" onclick="document.getElementById('send').value='New Group';setValue('', '<?php echo $last_order ?>')" ><?php echo language('New Group');?></button>
	</div>
	
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
	
	save_user_record("","ROUTING->Groups:Save,section=".$section);

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

	save_user_record("","ROUTING->Groups:Delete,section=".$group_name);
}
?>


<?php
function gsm_member_show($members)
{
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	global $__GSM_SUM__;
	global $__GSM_HEAD__;
	global $__deal_cluster__;
	
	if($__deal_cluster__){
		$cluster_info = get_cluster_info();
	}
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
	global $only_view;
	
	if($group_name) {
		echo "<h4 style='margin-top:30px;'>";echo language('Modify a Group');echo "</h4>";
	} else {
		echo "<h4 style='margin-top:30px;'>";echo language('Create a Group');echo "</h4>";
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
			
			$("#policy").val("ascending");
		} else if (type == 'gsm') {
			set_visible('span_sip_members', false);
			set_visible('span_gsm_members', true);	
			set_visible('span_iax_members',false);
			set_visible('roundrobin',true);
			set_visible('reverseroundrobin',true);
			set_visible('fewestcalls',true);
			set_visible('ringall',false);
			
			$("#policy").val("ascending");
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
		var is_check = false;
	
		var name_ary = new Array(<?php echo $name_ary; ?>);
		var group_name = document.getElementById('group_name').value;

		document.getElementById("cgroup_name").innerHTML = '';
		if(!check_routingname(group_name)) {
			console.log('hello');
			document.getElementById('group_name').focus();
			document.getElementById("cgroup_name").innerHTML = con_str('fdsfsdfsfsfsd<?php echo htmlentities(language('js check groupname','Allowed character must be any of [0-9a-zA-Z`~!@#$%^*()_{}:|?-=.], 1-32 characters.'));?>');
			is_check = true;
		}
		for (var i in name_ary) 
		{
			if(name_ary[i] == group_name) {
				document.getElementById('group_name').focus();
				document.getElementById('cgroup_name').innerHTML = con_str('Already exist.');
				is_check = true;
			}
		}
		
		if(is_check){
			return false;
		}

		return true;
	}
	</script>

<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<input type="hidden" id="order" name="order" value="<?php echo $order?>" />
	<input type="hidden" id="old_group_name" name="old_group_name" value="<?php echo $group_name?>" />

	<div class="content">
		<span class="title">
			<?php echo language('Routing Groups');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Group Name')){ ?>
							<b><?php echo language('Group Name');?>:</b><br/>
							<?php echo language('Group Name help',"The name of this route. Should be used to describe what types of calls this route matches(for example, 'SIP2GSM' or 'GSM2SIP').");?>
							
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
						
						<?php if(is_show_language_help('Members')){ ?>
							<b><?php echo language('Members');?>:</b><br/>
							<?php echo language('Members help','Members');?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Group Name');?>:
			</span>
			<div class="tab_item_right">
				<span id="cgroup_name"></span>
				<input type="text" name="group_name" id="group_name" value="<?php echo $group_name?>" />
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
					$show_type[2] = '';
					switch($type) {
						case 'sip': $show_type[0] = 'selected'; break;
						case 'gsm': $show_type[1] = 'selected'; break;
						//case 'iax': $show_type[2] = 'selected'; break;
						default: 	$show_type[1] = 'selected'; break;
					}
?>
					<?php if($_SESSION['id'] == 1){ ?>
					<option value="sip" <?php echo $show_type[0];?> ><?php echo language('SIP');?></option>
					<?php } ?>
					
					<option value="gsm" <?php echo $show_type[1];?> ><?php echo language('MODULE');?></option>
					<!--<option value="iax" <?php echo $show_type[2];?> >IAX2</option>-->
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
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Members');?>:
			</span>
			<div class="tab_item_right" style="width:300px;">
				<span id="span_gsm_members" style="display: none;" >
				<?php gsm_member_show($members) ?>
				</span>

				<span id="span_sip_members" style="display: none;" >
				<?php sip_member_show($members) ?>
				</span>
				<span id="span_iax_members" style="display: none;" >
				<?php iax_member_show($members) ?>
				</span>
			</div>
			<div class="clear"></div>
		</div>
	</div>


	<input type="hidden" name="send" id="send" value="" />
	
	<div id="button_save">
		
		<?php if(!$only_view){ ?>
		<button type="submit" class="float_btn gen_short_btn"  onclick="document.getElementById('send').value='Save';return check();"><?php echo language('Save');?></button>
		<button type="submit" class="float_btn gen_short_btn"  onclick="document.getElementById('send').value='Apply';return check();"><?php echo language('Apply');?></button>
		<?php } ?>
		
		<button type="button" onclick="window.location.href='<?php echo get_self();?>'" ><?php echo language('Cancel');?></button>
	</div>
	
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

<!--
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
-->
	
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
			if($only_view){
				return false;
			}
			
			if(save_groups()) {
				//ast_reload();
				wait_apply("exec","asterisk -rx \"core reload\" > /dev/null 2>&1 &");
			}
			show_groups();
			exec("cd /my_tools/lua/sms_routing; lua read_group.lua");
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Apply') {
			if($only_view){
				return false;
			}
			
			if(save_groups()) {
				//ast_reload();
				wait_apply("exec","asterisk -rx \"core reload\" > /dev/null 2>&1 &");
			}
			$check_float = 1;
			add_group_page($_POST['group_name'],'');
			exec("cd /my_tools/lua/sms_routing; lua read_group.lua");
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Delete') {
			if($only_view){
				return false;
			}
			
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