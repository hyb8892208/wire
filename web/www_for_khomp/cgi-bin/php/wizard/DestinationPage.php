<!--The page of Deistination -->
<?php 

function get_newgsm_name_by_output_value2($output){
	global $__BRD_HEAD__;
	global $__GSM_HEAD__;	
	$db=explode('-',$output);
	
	if ($db[0]=='Board'){// master-slave mode example:Borad-1-gsm-1-out
		return "gsm-".$db[1].".".$db[3];
	}else{// stand alone mode; example:gsm-1-out
		return $output;		
	}	
}
function dial_pattern_html($prepend,$prepend_c,$prefix,$prefix_c,$pattern,$pattern_c,$cid,$cid_c,$convert=true)
{
	if($prepend=='')	$prepend='prepend';
	if($prepend_c=='')	$prepend_c="style=\"color:#aaaaaa;\"";
	if($prefix=='')		$prefix='prefix';
	if($prefix_c=='')	$prefix_c="style=\"color:#aaaaaa;\"";
	if($pattern=='')	$pattern='match pattern';
	if($pattern_c=='')	$pattern_c="style=\"color:#aaaaaa;\"";
	if($cid=='')		$cid='CallerId';
	if($cid_c=='')		$cid_c="style=\"color:#aaaaaa;\"";
	if($convert) {
		$sign = '\\\'';
	} else {
		$sign = '\'';
	}

	echo "(<input title=\"prepend\" type=\"text\" size=\"8\" name=\"prepend[]\" onchange=\"_onchange(this)\" value=\"$prepend\" onfocus=\"_onfocus(this,${sign}prepend${sign})\" onblur=\"_onblur(this,${sign}prepend${sign})\" $prepend_c class=\"ant-input ant-input-lg prepend\"/>)";
	echo ' + ';
	echo "<input title=\"prefix\" type=\"text\" size=\"6\" name=\"prefix[]\" value=\"$prefix\" onchange=\"_onchange(this)\" onfocus=\"_onfocus(this,${sign}prefix${sign})\" onblur=\"_onblur(this,${sign}prefix${sign})\" $prefix_c class=\"ant-input ant-input-lg prefix\"/>";
	echo ' | ';
	echo "[<input title=\"match pattern\" type=\"text\" size=\"16\" name=\"pattern[]\" value=\"$pattern\" onfocus=\"_onfocus(this,${sign}match pattern${sign})\" onblur=\"_onblur(this,${sign}match pattern${sign})\" $pattern_c class=\"ant-input ant-input-lg pattern\"/>";
	echo ' / ';
	echo "<input title=\"CallerId\" type=\"text\" size=\"10\" name=\"cid[]\" value=\"$cid\" onfocus=\"_onfocus(this,${sign}CallerId${sign})\" onblur=\"_onblur(this,${sign}CallerId${sign})\" $cid_c class=\"ant-input ant-input-lg callerid\"/>]&nbsp;&nbsp;";
	
	echo '<img src="/images/delete.gif" style="float:none; margin-left:0px; margin-bottom:-3px;cursor:pointer;" alt="remove" title="';
	echo language('Click here to remove this pattern');
	echo '" onclick="javascript:this.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode);">';
}
function channel_select($label,$sel=NULL,$needanysip=false)
{
	global $__SIP_HEAD__;
	global $__IAX_HEAD__;
	global $__GSM_HEAD__;
	global $__GSM_SUM__;
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	global $__GRP_HEAD__;

	if($needanysip) {
		echo "<select class=\"ant-select-selection-selected-value\" name=\"$label\" id=\"$label\" >";
	} else {
		echo "<select class=\"ant-select-selection-selected-value\" name=\"$label\" id=\"$label\" >";
	}

	echo '<option value="none">';
	echo language('_None');
	echo '</option>';

	if($needanysip) {
		$select = $sel === 'anysip' ? 'selected' : '';
		echo "<option value=\"anysip\" $select>";
		echo language('Any SIP');
		echo '</option>';
	} else {
		$select = $sel === 'custom' ? 'selected' : '';
		echo "<option value='custom' $select>";
		echo language('Custom');
		echo '</option>';
	}

	$cluster_info = get_cluster_info();

	//GSM
	echo '<optgroup label="';echo language('Port');echo '">';
	for($i=1; $i<=$__GSM_SUM__; $i++) {
		$value = get_gsm_value_by_channel($i);
		$select = $sel === $value ? 'selected' : '';
		echo "<option value=\"$value\" $select>";
		echo get_gsm_name_by_channel($i);
		echo '</option>';
	}


	echo '</optgroup>';

	//SIP
	/* /etc/asterisk/sip_endpoints.conf */
	$all_sips = get_all_sips();
	echo '<optgroup class="sip_label" label="';echo language('SIP');echo '">';
	if($all_sips) {
		foreach($all_sips as $sip) {

			$endpoint_name = trim($sip['endpoint_name']);
			$value = get_sip_name_has_head($endpoint_name);
			$name = get_sip_name_no_head($endpoint_name);
			$select = $sel === $value ? 'selected' : '';
			echo "<option value=\"$value\" $select>";
			echo $name;
			echo '</option>';
		}
	}
	echo '</optgroup>';
	
	//IAX	
	//$aql = new aql();
	//$aql->set('basedir','/etc/asterisk');
	//$hlock = lock_file('/etc/asterisk/gw_iax_endpoints.conf');
	//$all_iaxs = $aql->query("select * from gw_iax_endpoints.conf");
	//unlock_file($hlock);
	$all_iaxs = get_all_iaxs();
	echo '<optgroup class="iax_label" label="';echo language('IAX2');echo '">';
	if($all_iaxs) {
		foreach($all_iaxs as $iax) {

			$endpoint_name = trim($iax['endpoint_name']);
			//if ($endpoint_name=="") continue;
			$value = get_iax_name_has_head($endpoint_name);
			$name = get_iax_name_no_head($endpoint_name);
			$select = $sel === $value ? 'selected' : '';
			echo "<option value=\"$value\" $select>";
			echo $name;
			echo '</option>';
		}
	}
	echo '</optgroup>';
	//Group
	/* /etc/asterisk/gw_group.conf */
	$all_groups = get_all_groups(true);
	echo '<optgroup class="group_label" label="';echo language('GROUP');echo '">';
	if($all_groups) {
		foreach($all_groups as $group) {

			$group_name = trim($group['group_name']);
			$value = get_grp_name_has_head($group_name);
			$name = get_grp_name_no_head($group_name);
			$select = $sel === $value ? 'selected' : '';
			echo "<option value=\"$value\" $select>";
			echo $name;
			echo '</option>';
		}
	}
	echo '</optgroup>';
	/* add to_channel type of conference
	/etc/asterisk/gw_conferences.conf 
	*/
	global $conference_flag;
	if ($conference_flag == "1"){
		$all_confereces = get_all_conference(true);
		echo '<optgroup class="conf_label" label="';echo language('Conference');echo '">';
		if($all_confereces) {
			foreach($all_confereces as $conference){
				$conference_number = trim($conference['conference_number']);
				$value = get_conference_name_has_head($conference_number);
				
				$select = $sel === $value ? 'selected' : '';
				echo "<option value=\"$value\" $select>";
				echo "conf-".$conference_number;
				echo '</option>';
			}
		}
		echo '</optgroup>';
	}

	
	echo '</select>';

	if($needanysip) {
		echo '<span id="warn_anysip"></span>';
	}
}
?>

<div class="ant-card-body" data-for="five-page" style="padding: 20px;display:none">
	<div class="ant-row">
		<div class="ant-col-24">
			<div class="section-title"><span><?php echo language("Routing Groups");?></span></div>
		</div>
	</div>
	<div class="ant-row ant-form-item">
		<div class="ant-col-4 ant-form-item-label">
			<label for="group_name" class="" title=""><span><?php echo language('Group Name');?>:</span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control has-success">
				<input type="text" value="" id="group_name" name="group_name" data-__meta="[object Object]" class="ant-input ant-input-lg">
			</div>
		</div>
	</div>
	<div class="ant-row ant-form-item">
		<div class="ant-col-4 ant-form-item-label">
			<label for="sip_endpoint_username" class="" title=""><span><?php echo language('Policy');?>:</span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control has-success">
				<select name="policy" id="policy" class="ant-select-selection-selected-value ">
					<option value="ascending"><?php echo language('Ascending');?></option>
					<option value="descending"><?php echo language('Descending');?></option>
					<option id="roundrobin" value="roundrobin" ><?php echo language('Roundrobin');?></option>
					<option id="reverseroundrobin" value="reverseroundrobin"><?php echo language('Reverse Roundrobin');?></option>
					<option id="fewestcalls" value="fewestcalls"><?php echo language('Fewest Calls');?>(*<?php echo language('experiment');?>)</option>
				</select>
			</div>
		</div>
	</div>
	<div class="ant-row ant-form-item">
		<div class="ant-col-4 ant-form-item-label">
			<label for="member" class="" title=""><span><?php echo language('Members');?></span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control has-success">
				<table cellpadding="0" cellspacing="0" class="port_table">
		<?php
				for ($line=0;$line<$__GSM_SUM__/4;$line++) {
					echo '<tr style="height:20px">';
					for($i=1+$line*4; $i<=(4+$line*4); $i++) {
						if ($i>$__GSM_SUM__) break;	
						$port_name = get_newgsm_name_by_output_value2(get_gsm_name_by_channel($i));
						if(strstr($port_name, 'null')) continue;			
						echo '<td><label class="ant-checkbox-wrapper">';
						echo '<span class="ant-checkbox" check_out="member">';
		                echo '<input type="checkbox" class="ant-checkbox-input members" name="gsm_members[]" value="'.get_gsm_value_by_channel($i).'"/> ';
		                echo '<span class="ant-checkbox-inner"></span>'; 
		                echo '</span></label>';
						echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
						echo "<lable class=\"port_name\" style=\"color: rgba(0,0,0,.65);\"><span>";
						echo $port_name;
						echo "</span></label>";
						echo '&nbsp;&nbsp;</td>';
					}
					echo '</tr>';
				}
		?>
					<tr style="border:none;">
						<td style="border:none;padding: 10px 0px;">
							<label class="ant-checkbox-wrapper"> 
			                  <span class="ant-checkbox"> 
			                     <input type="checkbox" class="ant-checkbox-input"  target-check="member" value="on" name="haha" id="select_all"/> 
			                     <span class="ant-checkbox-inner"></span> 
			                  </span> 
			             </label>

						<?php echo language('All');?></td>
					</tr>
				</table>
			</div>
		</div>
	</div>
	<div class="ant-row">
		<div class="ant-col-24">
			<div class="section-title"><span><?php echo language("Routing Rules");?></span></div>
		</div>
	</div>
	<div class="ant-routing-settings hidden">
		<div class="ant-row ant-form-item">
			<div class="ant-col-4 ant-form-item-label">
				<label for="routing_name" class="" title=""><span><?php echo language('Routing Name');?>:</span></label>
			</div>
			<div class="ant-col-6 ant-form-item-control-wrapper">
				<div class="ant-form-item-control ">
					<input type="text" value="" id="routing_name" name="routing_name" data-__meta="[object Object]" class="ant-input ant-input-lg">
				</div>
			</div>
		</div>
		<div class="ant-row ant-form-item">
			<div class="ant-col-4 ant-form-item-label">
				<label for="routing_name" class="" title=""><span><?php echo language('Call Comes in From');?>:</span></label>
			</div>
			<div class="ant-col-6 ant-form-item-control-wrapper">
				<div class="ant-form-item-control ">
					<?php channel_select('from_channel',NULL ,true) ?>
				</div>
			</div>
		</div>
		<div class="ant-row ant-form-item">
			<div class="ant-col-4 ant-form-item-label">
				<label for="transport" class="" title=""><span><?php echo language('Send Call Through');?>:</span></label>
			</div>
			<div class="ant-col-6 ant-form-item-control-wrapper">
				<div class="ant-form-item-control ">
					<?php channel_select('to_channel',NULL) ?>
				</div>
			</div>
		</div>
	</div>
	<div class="ant-row ant-form-item">
		<div class="ant-col-4 ant-form-item-label">
			<label for="lan2_dns1" class="" title=""><span><?php echo language('Dial Patterns that will use this Route');?>:</span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control ">
				<table width="98%" align="right" id="tbl_dialroute">
				<?php
					echo '<tr><td>';
					dial_pattern_html('','','','','','','','',false);
					echo '</td></tr>';
				?>
				</table>
			</div>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="top-button">
				<button type="button" class="ant-btn ant-btn-primary add-dial-pattern">
					<span>+ <?php echo language('Add More Dial Pattern Fields');?></span>
				</button>
			</div>
		</div>
	</div>
	<div class="ant-row ant-form-item">
		<div class="ant-col-4 ant-form-item-label">
			<label for="dhcp_enable" class="" title=""><span><?php echo language('Create Inbound Routes');?></span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control has-success">
				<label class="ant-checkbox-wrapper">
					<span class="ant-checkbox">
						<input type="checkbox" id="create_inbound_routes" class="ant-checkbox-input" name="create_inbound_routes" value="on"><span class="ant-checkbox-inner"></span>
					</span>
				</label>
			</div>
		</div>
	</div>
	<div class="ant-row ant-form-item forward-number hidden">
		<div class="ant-col-4 ant-form-item-label">
			<label for="forward_number" class="" title=""><span><?php echo language('Forward Number');?>:</span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control has-success">
				<input type="text" value="" id="forward_number" name="forward_number" data-__meta="[object Object]" class="ant-input ant-input-lg">
			</div>
		</div>
	</div>
</div>
