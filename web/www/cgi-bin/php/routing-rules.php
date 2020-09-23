<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
require_once("/www/cgi-bin/inc/aql.php");
?>

<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<script type="text/javascript" src="/js/functions.js"></script>

<?php




function show_routings()
{
	global $__BRD_HEAD__;
	global $__GSM_HEAD__;

	$all_routings = get_all_routings(true);
	$last_order = 1;
	if($all_routings) {
		$last = end($all_routings); 
		if(isset($last['order']) && $last['order'] != '') {
			$last_order = $last['order'] + 1;
		}
	}
?>

	<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>
	<script type="text/javascript">

	$(document).ready(function(){ 
		$(function() {
			$(".drag_sort").sortable({ 
				opacity: 0.6, cursor: 'url(/images/closehand.cur),auto', update: function() {
					var order = $(this).sortable("serialize") + '&action=updateRecordsListings';
				}
			});
		});
		$(".drag_sort li").mousedown(function(){
			$(this).css('cursor','url(/images/closehand.cur),auto');
		});
		$(".drag_sort li").mouseup(function(){
			$(this).css('cursor','url(/images/openhand.cur),auto');
		});
	});

	function setValue(value1,value2)
	{
		document.getElementById('sel_routing_name').value = value1;
		document.getElementById('order').value = value2;
	}

	function getPage(value)
	{
		window.location.href = '<?php echo get_self();?>?sel_routing_name='+escape(value);
	}

	function delete_click(value1,value2)
	{
		ret = confirm("<?php echo language('Delete confirm','Are you sure to delete you selected ?');?>");

		if(ret) {
			document.getElementById('sel_routing_name').value = value1;
			document.getElementById('order').value = value2;
			return true;
		}

		return false;
	}
function batch_delete_click()
{
	ret = confirm("<?php echo language('Delete confirm','Are you sure to delete you selected ?');?>");

	if(ret) {
		var loadi = layer.load('loading......');
		return true;
	}

	return false;
}

	
	</script>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<input type="hidden" id="sel_routing_name" name="sel_routing_name" value="" />
	<input type="hidden" id="order" name="order" value="" />

	<table width="100%" id="tab_routings" class="tdrag">
		<tr>
			<th style="width:03%" class="nosort">
				<input type="checkbox" name="selall" onclick="selectAll(this.checked,'rules[]')" />
			</th>
			<th width="05%"><?php echo language('Move');?></th>
			<th width="05%"><?php echo language('Order');?></th>
			<th width="15%"><?php echo language('Rule Name');?></th>
			<th width="15%"><?php echo language('From');?></th>
			<th width="20%"><?php echo language('To');?></th>
			<th width="30%"><?php echo language('Rules');?></th>
			<th width="07%"><?php echo language('Actions');?></th>
		</tr>

<?php
	if($all_routings) {
		$i = 1;
?>
		<tr>
			<td colspan=8 style="border:0;margin:0;padding:0">
			<ul class="drag_sort" id="movelist">
<?php
		foreach($all_routings as $routing) {
			$routing_name = trim($routing['routing_name']);
			$from_channel = trim($routing['from_channel']);	
			$to_channel = trim($routing['to_channel']);
			
			$custom_context = '';
			if (isset($routing['custom_context'])) {
				$custom_context = trim($routing['custom_context']);
			} else {
				$custom_context ='';
			}

			if($from_channel == 'anysip') {
				$show_from_channel = language('Any SIP');
			} else {
				$show_from_channel = get_channel_name($from_channel);
			}
			$show_to_channels = '';
			if ($to_channel == 'custom') {
				$show_to_channels = language('custom-'.$custom_context);
			} else {
				$output = explode(',',$to_channel);
				foreach($output as $ch) {
					$name = get_channel_name($ch);
					$show_to_channels .= $name;
					$show_to_channels .= ', ';  
				}
				$show_to_channels = rtrim($show_to_channels,', ');
			}
			$show_rules='';
			$callee_pattern = parse_dial_pattern($routing['callee_dial_pattern']);
			if($routing['callee_dial_pattern'] != '|||||' && $routing['callee_dial_pattern'] != '') {
				$show_rules .= 'Callee Dial Pattern: ';
				foreach($callee_pattern as $each) {
					$show_rules .= $each['prepend'].'+'.$each['prefix'].'|['.$each['match_pattern'].']|('.$each['sdfr'].'+'.$each['sta'].')|'.$each['rdfr'].'<br/>';
				}
			}
			$caller_pattern = parse_dial_pattern($routing['caller_dial_pattern']);
			if($routing['caller_dial_pattern'] != '|||||||' && $routing['caller_dial_pattern'] != ''){
				$show_rules .= 'Caller Dial Pattern: ';
				foreach($caller_pattern as $each){
					$show_rules .= $each['prepend'].'+'.$each['prefix'].'|['.$each['match_pattern'].']|('.$each['sdfr'].'+'.$each['sta'].')|'.$each['rdfr'].'|'.$each['callername'].' '.$each['trunk_sw'].'<br/>';
				}
			}
			
			$tp = parse_time_pattern($routing['time_pattern']);
			if($tp) {
				$show_rules .= 'Time Pattern: ';
				foreach($tp as $each) {
					$t = $each['stime'].'-'.$each['etime'].'&'.$each['sweek'].'-'.$each['eweek'].'&'.$each['sday'].'-'.$each['eday'].'&'.$each['smonth'].'-'.$each['emonth'].'<br/>'; 
					$show_rules .= $t;
				}
			}
?>
			<li>
				<table width="100%" class="tdrag" style="border:0;">
					<tr bgcolor="#E8EFF7">
						<td width="03%"> 
							<input type="checkbox" name="rules[]" value="<?php echo $routing_name; ?>" />
						</td>
						<td width="05%" class="drag">
						</td>
						<td width="05%" >
							<?php echo $i++ ?>
						</td>
						<td width="15%">
							<?php echo $routing_name ?>
						</td>
						<td width="15%">
							<?php echo $show_from_channel ?>
						</td>
						<td width="20%">
							<?php echo $show_to_channels;?>
						</td>
						<td width="30%">
							<?php echo $show_rules;?>
						</td>
						<td width="07%">
							<button type="button" value="Modify" style="width:32px;height:32px;cursor:pointer;" 
								onclick="getPage('<?php echo $routing_name ?>')">
								<img src="/images/edit.gif">
							</button>
							<button type="submit" value="Delete" style="width:32px;height:32px;cursor:pointer;" 
								onclick="document.getElementById('send').value='Delete';return delete_click('<?php echo $routing_name ?>', '')" >
								<img src="/images/delete.gif">
							</button>
							<input type="hidden" name="routing_name[]" value="<?php echo $routing_name ?>" />
						</td>
					</tr>
				</table>
			</li>
<?php
		}
?>
			</ul>
			</td>
		</tr>
<?php
	}
?>
	</table>
	<br>
	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" value="<?php echo language('New Call Routing Rule');?>" onclick="document.getElementById('send').value='New Call Routing Rule';setValue('', '<?php echo $last_order ?>')" />
	&nbsp;
	<input type="submit" value="<?php echo language('Delete');?>" onclick="document.getElementById('send').value='Delete';return batch_delete_click()" />
	&nbsp;
	<input type="submit" value="<?php echo language('Save Orders');?>"
		onclick="document.getElementById('send').value='Save Orders';"/>
	</form>
<?php
}
?>

<?php
function save_orders()
{
	if( isset($_POST['routing_name']) && is_array($_POST['routing_name']) && !empty($_POST['routing_name'])) {

		$aql = new aql();
		$setok = $aql->set('basedir','/etc/asterisk');
		if (!$setok) {
			return;
		}

		$aql->autocommit = false;
		$i = 1;
		$hlock = lock_file('/etc/asterisk/gw_routing.conf');
		foreach($_POST['routing_name'] as $routing_name) {
			$aql->query("update gw_routing.conf set order=\"$i\" where section='$routing_name'");
			$i++;	
		}
		$aql->commit('gw_routing.conf');
		unlock_file($hlock);

		save_routings_to_extensions();
	}
}

function match_dialpattern($str)
{
	if(preg_match('/^[*#+0-9XZN.]+$/',$str)) {
		return true;
	}
	return false;
}
function match_dialpattern_add($str)
{
	if(preg_match('/^[*#+0-9XZN.\-\[\]]+$/',$str)) {
		return true;
	}
	return false;
}
function match_dialpattern_digit($str)
{                   
	if(preg_match('/^[0-9]+$/',$str)) {
		return true;
	}
	return false;
}


function save_disa_password($name_head)
{	
	$disa_passwd_conf_name = $name_head.'_disa_passwd.conf';
	
	if (isset($_POST['recv_secret'])) {
		$recv_secret = trim($_POST['recv_secret']);
		if ($recv_secret) {
			$recv_secret = str_replace('#',"\n",$recv_secret);
			$disa_passwd_conf_path = '/etc/asterisk/pin/'.$disa_passwd_conf_name;
			
			$fp = fopen($disa_passwd_conf_path,"w");
			if ($fp) {
				fwrite($fp,$recv_secret);
			}
			fclose($fp);
		}
	}
}

function save_routings()
{
	// /etc/asterisk/gw_routing.conf
	// [routing_name]
	// order = 1,2,3,4,5....    //Must set
	// from_channel = channel (sip-sipname or gsm-gsmname  type: sip- is SIP, gsm- is GSM)  //Must set
	// to_channel = channel,channel......   //Must set
	// dial_pattern = prepend|prefix|match pattern|cid,prepend|prefix|match pattern|cid........
	// time_pattern = time|week|day|month,time|week|day|month........
	// cid_name
	// cid_number
	// forward_number
	
	$datachunk = '';

	//routing name already existed! 

	if( isset($_POST['routing_name']) ) {
		$routing_name = trim($_POST['routing_name']);
		if($routing_name == '') {
			echo "Must set routing name";
			return false;
		}
		$section = $routing_name;
	} else {
		echo "Must set routing name";
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
		//echo "Must set order";
		return false;
	}

	if( isset($_POST['from_channel']) ) {
		$from_channel = trim($_POST['from_channel']);
		if($from_channel == '' || $from_channel == 'none') {
			echo "Must set from channel";
			return false;
		}
		$datachunk .= 'from_channel='.$from_channel."\n";
		if($from_channel == 'anysip') {
			set_sip_general_allowguest();
		}
	} else {
		echo "Must set from channel";
		return false;
	}

	if( isset($_POST['to_channel']) ) {
		$to_channel = trim($_POST['to_channel']);
		if($to_channel == '' || $to_channel == 'none') {
			echo "Must set to channel";
			return false;
		}

		$to_channel .= ',';
		for($i=1; isset($_POST["fctn_channel$i"]); $i++) {
			$fctn_channel = trim($_POST["fctn_channel$i"]);
			if($fctn_channel == 'none' || $fctn_channel == '') continue;

			$to_channel .= $fctn_channel.',';
		}
		$to_channel = rtrim($to_channel,',');
		$datachunk .= 'to_channel='.$to_channel."\n";
	} else {
		echo "Must set to channel";
		return false;
	}
	
	//DISA pattern
	///////////////////////////////////////////////////////////////////////////////
	if (isset($_POST['disa_sw'])) {
		$disa_sw_val = trim($_POST['disa_sw']);
	} else {
		$disa_sw_val = 'off';
	}
	if (isset($_POST['second_dial_sw'])) {
		$second_dial_sw = trim($_POST['second_dial_sw']);
	} else {
		$second_dial_sw = 'off';
	}
	

	

	if (isset($_POST['disa_timeout'])) {
		$disa_timeout_val = trim($_POST['disa_timeout']);
	}
	if (isset($_POST['disa_password_digits'])) {
		$disa_password_digits_val = trim($_POST['disa_password_digits']);
	}
	
	//////////////////////////////////////////////////////////////////////
//	//add extensions_macro.conf
//	$section_macro_disa = 'macro-disa';
//	$data_macro_disa = '';
//	$data_macro_disa .= "exten => s,1,Set(TIMEOUT(digit)=5)"."\n";
//	$data_macro_disa .= "exten => s,n,Set(TIMEOUT(response)=\${ARG4})"."\n";
//	//$data_macro_disa .= "exten => s,n,Authenticate(/etc/asterisk/disa_passwd.conf,,5)"."\n";
//	$data_macro_disa .= "exten => s,n,Authenticate(\${ARG2},,\${ARG3})"."\n";
//	$data_macro_disa .= "exten => s,n,DISA(no-password,\${ARG1})"."\n";
	
	
//	$section_macro_auth = 'macro-Auth';
//	$data_macro_auth = '';
//	$data_macro_auth .= "exten => s,1,Set(TIMEOUT(digit)=5)"."\n";
//	$data_macro_auth .= "exten => s,n,Set(TIMEOUT(response)=\${ARG3})"."\n";
//	//$data_macro_auth .= "exten => s,n,Authenticate(/etc/asterisk/disa_passwd.conf,,5)"."\n";
//	$data_macro_auth .= "exten => s,n,Authenticate(\${ARG1},,\${ARG2})"."\n";
	
//	//echo $data_macro_disa;
	
	
//	$extensions_custom_conf_path = "/etc/asterisk/extensions_custom.conf";
//	$hlock_macro = lock_file($extensions_custom_conf_path);
//	if (!file_exists($extensions_custom_conf_path)) fclose(fopen($extensions_custom_conf_path,"w"));
//	$aql_macro = new aql();
//	$aql_macro->set('basedir','/etc/asterisk');
//	if(!$aql_macro->open_config_file($extensions_custom_conf_path)){
//		echo $aql_macro->get_error();
//		unlock_file($hlock_macro);
//		return false;
//	}
//	$aql_macro->assign_delsection($section_macro_disa);
//	$aql_macro->assign_delsection($section_macro_auth);
//	$aql_macro->save_config_file('extensions_custom.conf');
//	$aql_macro->assign_addsection($section_macro_disa,$data_macro_disa);
//	$aql_macro->assign_addsection($section_macro_auth,$data_macro_auth);
//	$aql_macro->save_config_file('extensions_custom.conf');
//	unlock_file($hlock_macro);

	//Dial pattern
	///////////////////////////////////////////////////////////////////////////////
	$i = 0;
	if(isset($_POST['prepend']) && is_array($_POST['prepend']) && $_POST['prepend'] != '') {
		foreach($_POST['prepend'] as $each) {
			$dp_ary[$i++]['prepend'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['prefix']) && is_array($_POST['prefix']) && $_POST['prefix'] != '') {
		foreach($_POST['prefix'] as $each) {
			$dp_ary[$i++]['prefix'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['pattern']) && is_array($_POST['pattern']) && $_POST['pattern'] != '') {
		foreach($_POST['pattern'] as $each) {
			$dp_ary[$i++]['pattern'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['cid']) && is_array($_POST['cid']) && $_POST['cid'] != '') {
		foreach($_POST['cid'] as $each) {
			$dp_ary[$i++]['cid'] = trim($each);
		}
	}

/*
	$dial_pattern = '';

	if(isset($dp_ary) && is_array($dp_ary)) {
		foreach($dp_ary as $each) {
			$prepend = '';
			if(isset($each['prepend'])) {
				$tmp = trim($each['prepend']);
				if(match_dialpattern($tmp)) {
					$prepend =  $tmp;
				}
			}
			$prefix = '';
			if(isset($each['prefix'])) {
				$tmp = trim($each['prefix']);
				if(match_dialpattern_add($tmp)) {
					$prefix =  $tmp;
				}
			}
			$pattern = '';
			if(isset($each['pattern'])) {
				$tmp = trim($each['pattern']);
				if(match_dialpattern_add($tmp)) {
					$pattern =  $tmp;
				}
			}
			$cid = '';
			if(isset($each['cid'])) {
				$tmp = trim($each['cid']);
				if(match_dialpattern($tmp)) {
					$cid =  $tmp;
				}
			}

			if(!($prepend=='' && $prefix=='' && $pattern=='' && $cid=='')) {
				$dp = "$prepend|$prefix|$pattern|$cid";
				$dial_pattern .= $dp.',';
			}
		}
		$dial_pattern = rtrim($dial_pattern,',');
	}

	$datachunk .= 'dial_pattern='.$dial_pattern."\n";
	///////////////////////////////////////////////////////////////////////////////
*/

	//callee Dial pattern
	///////////////////////////////////////////////////////////////////////////////
	$i = 0;
	if(isset($_POST['prepend_e']) && is_array($_POST['prepend_e']) && $_POST['prepend_e'] != '') {
		foreach($_POST['prepend_e'] as $each) {
			$e_dp_ary[$i++]['prepend_e'] = trim($each);
		}
	}

	
	$i = 0;
	if(isset($_POST['prefix_e']) && is_array($_POST['prefix_e']) && $_POST['prefix_e'] != '') {
		foreach($_POST['prefix_e'] as $each) {
			//echo strpos($_POST['from_channel'],'fxo');
			if (strpos($_POST['from_channel'],'fxo')!==false){
				$e_dp_ary[$i++]['prefix_e'] = "";
			}else{
				$e_dp_ary[$i++]['prefix_e'] = trim($each);
			}
		}
	}

	$i = 0;
	
	if(isset($_POST['match_pattern_e']) && is_array($_POST['match_pattern_e']) && $_POST['match_pattern_e'] != '') {
		foreach($_POST['match_pattern_e'] as $each) {
			
			if (strpos($_POST['from_channel'],'fxo')!==false){
				$e_dp_ary[$i++]['match_pattern_e']= "";
			}else{
				$e_dp_ary[$i++]['match_pattern_e'] = trim($each);
			}
			
		}
	}

	$i = 0;
	if(isset($_POST['sdfr_e']) && is_array($_POST['sdfr_e']) && $_POST['sdfr_e'] != '') {
		foreach($_POST['sdfr_e'] as $each) {
			$e_dp_ary[$i++]['sdfr_e'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['sta_e']) && is_array($_POST['sta_e']) && $_POST['sta_e'] != '') {
		foreach($_POST['sta_e'] as $each) {
			$e_dp_ary[$i++]['sta_e'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['rdfr_e']) && is_array($_POST['rdfr_e']) && $_POST['rdfr_e'] != '') {
		foreach($_POST['rdfr_e'] as $each) {
			$e_dp_ary[$i++]['rdfr_e'] = trim($each);
		}
	}
	$e_dial_pattern = '';

	if(isset($e_dp_ary) && is_array($e_dp_ary)) {
		foreach($e_dp_ary as $each) {
			$prepend_e = '';
			if(isset($each['prepend_e'])) {
				$e_tmp = trim($each['prepend_e']);
				if(match_dialpattern($e_tmp)) {
					$prepend_e =  $e_tmp;
				}
			}
			$prefix_e = '';
			if(isset($each['prefix_e'])) {
				$e_tmp = trim($each['prefix_e']);
				if(match_dialpattern_add($e_tmp)) {
					$prefix_e =  $e_tmp;
				}
			}
			$match_pattern_e = '';
			if(isset($each['match_pattern_e'])) {
				$e_tmp = trim($each['match_pattern_e']);
				if(match_dialpattern_add($e_tmp)) { 
					$match_pattern_e =  $e_tmp;
				}
			}
			$sdfr_e = '';
			if(isset($each['sdfr_e'])) {
				$e_tmp = trim($each['sdfr_e']);
				if(match_dialpattern_digit($e_tmp)) {
					$sdfr_e =  $e_tmp;
				}
			}
			$sta_e = '';
			if(isset($each['sta_e'])) {
				$e_tmp = trim($each['sta_e']);
				if(match_dialpattern($e_tmp)) {
					$sta_e =  $e_tmp;
				}
			}
			$rdfr_e = '';
			if(isset($each['rdfr_e'])) {
				$e_tmp = trim($each['rdfr_e']);
				if(match_dialpattern_digit($e_tmp)) {
					$rdfr_e =  $e_tmp;
				}
			}

			if(!($prepend_e=='' && $prefix_e=='' && $match_pattern_e=='' && $sdfr_e=='' && $sta_e=='' && $rdfr_e=='' )) {
				$e_dp = "$prepend_e|$prefix_e|$match_pattern_e|$sdfr_e|$sta_e|$rdfr_e";
				$e_dial_pattern .= $e_dp.',';
				
			}else{
				$dp = "|||||";
				$e_dial_pattern .= $dp.',';
			}
		}
		$e_dial_pattern = rtrim($e_dial_pattern,',');
		
	}

	$datachunk .= 'callee_dial_pattern='.$e_dial_pattern."\n";
    ///////////////////////////////////////////////////////////////////////////////
	
	//caller Dial pattern
	///////////////////////////////////////////////////////////////////////////////
	$i = 0;
	if(isset($_POST['prepend_r']) && is_array($_POST['prepend_r']) && $_POST['prepend_r'] != '') {
		foreach($_POST['prepend_r'] as $each) {
			$dp_ary[$i++]['prepend_r'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['prefix_r']) && is_array($_POST['prefix_r']) && $_POST['prefix_r'] != '') {
		foreach($_POST['prefix_r'] as $each) {
			$dp_ary[$i++]['prefix_r'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['match_pattern_r']) && is_array($_POST['match_pattern_r']) && $_POST['match_pattern_r'] != '') {
		foreach($_POST['match_pattern_r'] as $each) {
			$dp_ary[$i++]['match_pattern_r'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['sdfr_r']) && is_array($_POST['sdfr_r']) && $_POST['sdfr_r'] != '') {
		foreach($_POST['sdfr_r'] as $each) {
			$dp_ary[$i++]['sdfr_r'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['sta_r']) && is_array($_POST['sta_r']) && $_POST['sta_r'] != '') {
		foreach($_POST['sta_r'] as $each) {
			$dp_ary[$i++]['sta_r'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['rdfr_r']) && is_array($_POST['rdfr_r']) && $_POST['rdfr_r'] != '') {
		foreach($_POST['rdfr_r'] as $each) {
			$dp_ary[$i++]['rdfr_r'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['callername_r']) && is_array($_POST['callername_r']) && $_POST['callername_r'] != '') {
		foreach($_POST['callername_r'] as $each) {
			$dp_ary[$i++]['callername_r'] = trim($each);
		}
	}
	$i = 0;
	if(isset($_POST['trunk_sw']) && is_array($_POST['trunk_sw']) && $_POST['trunk_sw'] != '') {
		foreach ($_POST['trunk_sw'] as $each) {
			$dp_ary[$i++]['trunk_sw'] = trim($each);
		}
	}
	$dial_pattern = '';

	if(isset($dp_ary) && is_array($dp_ary)) {
		foreach($dp_ary as $each) {
			$prepend_r = '';
			if(isset($each['prepend_r'])) {
				$tmp = trim($each['prepend_r']);
				if(match_dialpattern($tmp)) {
					$prepend_r =  $tmp;
				}
			}
			$prefix_r = '';
			if(isset($each['prefix_r'])) {
				$tmp = trim($each['prefix_r']);
				if(match_dialpattern_add($tmp)) {
					$prefix_r =  $tmp;
				}
			}
			$match_pattern_r = '';
			if(isset($each['match_pattern_r'])) {
				$tmp = trim($each['match_pattern_r']);
				if(match_dialpattern_add($tmp)) {
					$match_pattern_r =  $tmp;
				}
			}
			$sdfr_r = '';
			if(isset($each['sdfr_r'])) {
				$tmp = trim($each['sdfr_r']);
				if(match_dialpattern_digit($tmp)) {
					$sdfr_r =  $tmp;
				}
			}
			$sta_r = '';
			if(isset($each['sta_r'])) {
				$tmp = trim($each['sta_r']);
				if(match_dialpattern($tmp)) {
					$sta_r =  $tmp;
				}
			}
			$rdfr_r = '';
			if(isset($each['rdfr_r'])) {
				$tmp = trim($each['rdfr_r']);
				if(match_dialpattern_digit($tmp)) {
					$rdfr_r =  $tmp;
				}
			}
			$callername_r = '';
			if(isset($each['callername_r'])) {
				$tmp = trim($each['callername_r']);
				
				//if(match_dialpattern($tmp)) {
				if ($tmp<>language("Caller Name")){
					$callername_r =  $tmp;
				}
			}
			$trunk_sw = '';
			if(isset($each['trunk_sw'])) {
				$tmp = trim($each['trunk_sw']);
				$trunk_sw = $tmp;
			}
			
			if(!($prepend_r=='' && $prefix_r=='' && $match_pattern_r=='' && $sdfr_r=='' && $sta_r=='' && $rdfr_r=='' && $callername_r=='')) {
				$dp = "$prepend_r|$prefix_r|$match_pattern_r|$sdfr_r|$sta_r|$rdfr_r|$callername_r|$trunk_sw";
				$dial_pattern .= $dp.',';
			}else{
				$dp = "|||||||";
				$dial_pattern .= $dp.',';
			}
			
		}
		$dial_pattern = rtrim($dial_pattern,',');
		
	}

	$datachunk .= 'caller_dial_pattern='.$dial_pattern."\n";

	// Time pattern
	///////////////////////////////////////////////////////////////////////////////
	$i = 0;
	if(isset($_POST['hstime']) && is_array($_POST['hstime']) && $_POST['hstime'] != '') {
		foreach($_POST['hstime'] as $each) {
			$tp_ary[$i++]['hstime'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['mstime']) && is_array($_POST['mstime']) && $_POST['mstime'] != '') {
		foreach($_POST['mstime'] as $each) {
			$tp_ary[$i++]['mstime'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['hetime']) && is_array($_POST['hetime']) && $_POST['hetime'] != '') {
		foreach($_POST['hetime'] as $each) {
			$tp_ary[$i++]['hetime'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['metime']) && is_array($_POST['metime']) && $_POST['metime'] != '') {
		foreach($_POST['metime'] as $each) {
			$tp_ary[$i++]['metime'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['sweek']) && is_array($_POST['sweek']) && $_POST['sweek'] != '') {
		foreach($_POST['sweek'] as $each) {
			$tp_ary[$i++]['sweek'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['eweek']) && is_array($_POST['eweek']) && $_POST['eweek'] != '') {
		foreach($_POST['eweek'] as $each) {
			$tp_ary[$i++]['eweek'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['sday']) && is_array($_POST['sday']) && $_POST['sday'] != '') {
		foreach($_POST['sday'] as $each) {
			$tp_ary[$i++]['sday'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['eday']) && is_array($_POST['eday']) && $_POST['eday'] != '') {
		foreach($_POST['eday'] as $each) {
			$tp_ary[$i++]['eday'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['smonth']) && is_array($_POST['smonth']) && $_POST['smonth'] != '') {
		foreach($_POST['smonth'] as $each) {
			$tp_ary[$i++]['smonth'] = trim($each);
		}
	}

	$i = 0;
	if(isset($_POST['emonth']) && is_array($_POST['emonth']) && $_POST['emonth'] != '') {
		foreach($_POST['emonth'] as $each) {
			$tp_ary[$i++]['emonth'] = trim($each);
		}
	}

	$time_pattern = '';

	if(isset($tp_ary) && is_array($tp_ary)) {
		foreach($tp_ary as $each) {

			if($each['hstime']||$each['hetime']||$each['mstime']||$each['metime']) {
				if(!$each['hstime']) {
					if($each['hetime']) $each['hstime'] = $each['hetime'];
					else $each['hstime'] = '00';
				}

				if(!$each['mstime']) {
					if($each['metime']) $each['mstime'] = $each['metime'];
					else $each['mstime'] = '00';
				}

				if(!$each['hetime']) {
					if($each['hstime']) $each['hetime'] = $each['hstime'];
					else $each['hetime'] = '00';
				}

				if(!$each['metime']) {
					if($each['mstime']) $each['metime'] = $each['mstime'];
					else $each['metime'] = '00';
				}
			}

			if($each['sweek']||$each['eweek']) {
				if(!$each['sweek']) {
					$each['sweek'] = $each['eweek'];
				}

				if(!$each['eweek']) {
					$each['eweek'] = $each['sweek'];
				}
			}

			if($each['sday']||$each['eday']) {
				if(!$each['sday']) {
					$each['sday'] = $each['eday'];
				}

				if(!$each['eday']) {
					$each['eday'] = $each['sday'];
				}
			}

			if($each['smonth']||$each['emonth']) {
				if(!$each['smonth']) {
					$each['smonth'] = $each['emonth'];
				}

				if(!$each['emonth']) {
					$each['emonth'] = $each['smonth'];
				}
			}

			$tp  = $each['hstime'] . ':' . $each['mstime']. '-' .$each['hetime']. ':'. $each['metime'];
			$tp .= '|' . $each['sweek'] . '-' . $each['eweek'];
			$tp .= '|' . $each['sday'] . '-' . $each['eday'];
			$tp .= '|' . $each['smonth'] . '-' . $each['emonth'];

			if( $each['hstime']||$each['hetime']||$each['mstime']||$each['metime']||
				$each['sweek']||$each['eweek']||
				$each['sday']||$each['eday']||
				$each['smonth']||$each['emonth'])
				$time_pattern .= $tp.',';
		}
		$time_pattern = rtrim($time_pattern,',');
	}

	$datachunk .= 'time_pattern='.$time_pattern."\n";
	///////////////////////////////////////////////////////////////////////////////


	if( isset($_POST['cid_name']) ) {
		$cid_name = trim($_POST['cid_name']);
		$datachunk .= 'cid_name='.$cid_name."\n";
	}

	if( isset($_POST['cid_number']) ) {
		$cid_number = trim($_POST['cid_number']);
		$datachunk .= 'cid_number='.$cid_number."\n";
	}

	if( isset($_POST['forward_number']) ) {
		$forward_number = trim($_POST['forward_number']);
		$datachunk .= 'forward_number='.$forward_number."\n";
	}
	
	if( ($to_channel == 'custom') && isset($_POST['custom_context'])) {
		$custom_context = trim($_POST['custom_context']);
		$datachunk .= 'custom_context='.$custom_context."\n";
	}

	// add delay setting
	if (isset($_POST['delay_sw'])) {
		$delay_setting = $_POST['delay_sw'] == 'on' ? '1' : '0';	
		if (isset($_POST['txt_delay_min']) && isset($_POST['txt_delay_max'])) {
			$delay_setting .= ','.$_POST['txt_delay_min'].','.$_POST['txt_delay_max'];
		}
		$datachunk .= 'transfer_delay='.$delay_setting."\n";
	}

	//add Disa information
	$datachunk .= 'DISA_sw='.$disa_sw_val."\n";
	$datachunk .= 'second_dial_sw='.$second_dial_sw."\n";
	$datachunk .= 'timeout='.$disa_timeout_val."\n";
	$datachunk .= 'max_passwd_digits='.$disa_password_digits_val;

	//Save to gw_routing.conf
	///////////////////////////////////////////////////
	$gw_routing_conf_path = "/etc/asterisk/gw_routing.conf";
	$hlock = lock_file($gw_routing_conf_path);
	if (!file_exists($gw_routing_conf_path)) fclose(fopen($gw_routing_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($gw_routing_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}

	$aql->assign_delsection($old_section);
	$aql->save_config_file('gw_routing.conf');
	$aql->assign_addsection($section,$datachunk);
	$aql->save_config_file('gw_routing.conf');
	unlock_file($hlock);
	///////////////////////////////////////////////////

	save_routings_to_extensions();

	return true;
}
?>

<?php
function del_routing($routing_name)
{
	//Save to gw_routing.conf
	///////////////////////////////////////////////////
	// /etc/asterisk/gw_routing.conf
	$gw_routing_conf_path = "/etc/asterisk/gw_routing.conf";
	$disa_passwd_conf_path = '/etc/asterisk/pin/'.$routing_name.'_disa_passwd.conf';
	if (file_exists($disa_passwd_conf_path)){
		unlink($disa_passwd_conf_path);
	}
	$hlock = lock_file($gw_routing_conf_path);
	if (!file_exists($gw_routing_conf_path)) fclose(fopen($gw_routing_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($gw_routing_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	$output = explode(',', $routing_name);
	foreach($output as $rule){
		$aql->assign_delsection($rule);	
	}
	$aql->save_config_file('gw_routing.conf');
	unlock_file($hlock);
	///////////////////////////////////////////////////

	save_routings_to_extensions();
}
?>

<?php
function channel_select($label,$sel=NULL,$needanysip=false)
{
	global $__SIP_HEAD__;
	global $__IAX_HEAD__;
	global $__GSM_HEAD__;
	global $__GSM_SUM__;
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	global $__GRP_HEAD__;
	global $__deal_cluster__;

	if($needanysip) {
		echo "<select size=1 name=\"$label\" id=\"$label\" onchange=\"fromchselchange()\">";
	} else {
		echo "<select size=1 name=\"$label\" id=\"$label\" onchange=\"tochselchange()\">";
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


	//GSM
	echo '<optgroup label="';echo language('Port');echo '">';
	for($i=1; $i<=$__GSM_SUM__; $i++) {
		$channel_name = get_gsm_name_by_channel($i);
		if(strstr($channel_name, 'null')){continue;}
		
		$value = get_gsm_value_by_channel($i);
		$select = $sel === $value ? 'selected' : '';
		echo "<option value=\"$value\" $select>";
		echo get_gsm_name_by_channel($i);
		echo '</option>';
	}

	if($__deal_cluster__){
		$cluster_info = get_cluster_info();
		if($cluster_info['mode'] == 'master') {
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				if(isset($cluster_info[$__BRD_HEAD__.$b.'_ip']) && $cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					for($i=1; $i<=$__GSM_SUM__; $i++) {
						$value = get_gsm_value_by_channel($i,$b);
						$select = $sel === $value ? 'selected' : '';
						echo "<option value=\"$value\" $select>";
						echo get_gsm_name_by_channel($i,$b);
						echo '</option>';
					}
				}
			}
		}
	}

	echo '</optgroup>';

	//SIP
	/* /etc/asterisk/sip_endpoints.conf */
	$all_sips = get_all_sips();
	if($all_sips) {
		echo '<optgroup label="';echo language('SIP');echo '">';
		foreach($all_sips as $sip) {

			$endpoint_name = trim($sip['endpoint_name']);
			$value = get_sip_name_has_head($endpoint_name);
			$name = get_sip_name_no_head($endpoint_name);
			$select = $sel === $value ? 'selected' : '';
			echo "<option value=\"$value\" $select>";
			echo $name;
			echo '</option>';
		}
		echo '</optgroup>';
	}
	
	//IAX	
	//$aql = new aql();
	//$aql->set('basedir','/etc/asterisk');
	//$hlock = lock_file('/etc/asterisk/gw_iax_endpoints.conf');
	//$all_iaxs = $aql->query("select * from gw_iax_endpoints.conf");
	//unlock_file($hlock);
	$all_iaxs = get_all_iaxs();
	if($all_iaxs) {
		echo '<optgroup label="';echo language('IAX2');echo '">';
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
		echo '</optgroup>';
	}
	//Group
	/* /etc/asterisk/gw_group.conf */
	$all_groups = get_all_groups(true);
	if($all_groups) {
		echo '<optgroup label="';echo language('GROUP');echo '">';
		foreach($all_groups as $group) {

			$group_name = trim($group['group_name']);
			$value = get_grp_name_has_head($group_name);
			$name = get_grp_name_no_head($group_name);
			$select = $sel === $value ? 'selected' : '';
			echo "<option value=\"$value\" $select>";
			echo $name;
			echo '</option>';
		}
		echo '</optgroup>';
	}
	
	echo '</select>';

	if($needanysip) {
		echo '<span id="warn_anysip"></span>';
	}
}
function failover_channel_select($name,$sel=NULL,$needanysip=false)
{
	global $__SIP_HEAD__;
	global $__IAX_HEAD__;
	global $__GSM_HEAD__;
	global $__GSM_SUM__;
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	global $__GRP_HEAD__;
	global $__deal_cluster__;

	if($needanysip) {
		echo "<select size=1 name=\"$name\" id=\"$name\" onchange=\"chselchange(this)\">";
	} else {
		echo "<select size=1 name=\"$name\" id=\"$name\">";
	}

	echo '<option value="none">';
	echo language('_None');
	echo '</option>';

	if($needanysip) {
		$select = $sel === 'anysip' ? 'selected' : '';
		echo "<option value=\"anysip\" $select>";
		echo language('Any SIP');
		echo '</option>';
	}


	//GSM
	echo '<optgroup label="';echo language('Port');echo '">';
	for($i=1; $i<=$__GSM_SUM__; $i++) {
		$channel_name = get_gsm_name_by_channel($i);
		if(strstr($channel_name, 'null')){continue;}
		
		$value = get_gsm_value_by_channel($i);
		$select = $sel === $value ? 'selected' : '';
		echo "<option value=\"$value\" $select>";
		echo get_gsm_name_by_channel($i);
		echo '</option>';
	}

	if($__deal_cluster__){
		$cluster_info = get_cluster_info();
		if($cluster_info['mode'] == 'master') {
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				if(isset($cluster_info[$__BRD_HEAD__.$b.'_ip']) && $cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					for($i=1; $i<=$__GSM_SUM__; $i++) {
						$value = get_gsm_value_by_channel($i,$b);
						$select = $sel === $value ? 'selected' : '';
						echo "<option value=\"$value\" $select>";
						echo get_gsm_name_by_channel($i,$b);
						echo '</option>';
					}
				}
			}
		}
	}

	echo '</optgroup>';

	//SIP
	/* /etc/asterisk/sip_endpoints.conf */
	$all_sips = get_all_sips();
	if($all_sips) {
		echo '<optgroup label="';echo language('SIP');echo '">';
		foreach($all_sips as $sip) {

			$endpoint_name = trim($sip['endpoint_name']);
			$value = get_sip_name_has_head($endpoint_name);
			$name = get_sip_name_no_head($endpoint_name);
			$select = $sel === $value ? 'selected' : '';
			echo "<option value=\"$value\" $select>";
			echo $name;
			echo '</option>';
		}
		echo '</optgroup>';
	}
	
	//IAX
	$all_iaxs = get_all_iaxs();
	if($all_iaxs) {
		echo '<optgroup label="';echo language('IAX2');echo '">';
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
		echo '</optgroup>';
	}	

	if($needanysip) {
		echo '<span id="warn_anysip"></span>';
	}
	
	
}
?>

<?php

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

	echo "(<input title=\"prepend\" type=\"text\" size=\"8\" name=\"prepend[]\" onchange=\"_onchange(this)\" value=\"$prepend\" onfocus=\"_onfocus(this,${sign}prepend${sign})\" onblur=\"_onblur(this,${sign}prepend${sign})\" $prepend_c />)";
	echo ' + ';
	echo "<input title=\"prefix\" type=\"text\" size=\"6\" name=\"prefix[]\" value=\"$prefix\" onchange=\"_onchange(this)\" onfocus=\"_onfocus(this,${sign}prefix${sign})\" onblur=\"_onblur(this,${sign}prefix${sign})\" $prefix_c />";
	echo ' | ';
	echo "[<input title=\"match pattern\" type=\"text\" size=\"16\" name=\"pattern[]\" value=\"$pattern\" onfocus=\"_onfocus(this,${sign}match pattern${sign})\" onblur=\"_onblur(this,${sign}match pattern${sign})\" $pattern_c />";
	echo ' / ';
	echo "<input title=\"CallerId\" type=\"text\" size=\"10\" name=\"cid[]\" value=\"$cid\" onfocus=\"_onfocus(this,${sign}CallerId${sign})\" onblur=\"_onblur(this,${sign}CallerId${sign})\" $cid_c />]&nbsp;&nbsp;";
	
	echo '<img src="/images/delete.gif" style="float:none; margin-left:0px; margin-bottom:-3px;cursor:pointer;" alt="remove" title="';
	echo language('Click here to remove this pattern');
	echo '" onclick="javascript:this.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode);">';
}

function initial_dial_pattern_para(){
	$para['calleeid']['prepend']='';
	$para['calleeid']['prepend_c']='';
	$para['calleeid']['prefix']='';
	$para['calleeid']['prefix_c']='';
	$para['calleeid']['match_pattern']='';
	$para['calleeid']['match_pattern_c']='';
	$para['calleeid']['sdfr']='';
	$para['calleeid']['sdfr_c']='';
	$para['calleeid']['sta']='';
	$para['calleeid']['sta_c']='';
	$para['calleeid']['rdfr']='';
	$para['calleeid']['rdfr_c']='';
	$para['calleeid']['calleename']='';
	$para['calleeid']['calleename_c']='';
	
	$para['callerid']['prepend']='';
	$para['callerid']['prepend_c']='';
	$para['callerid']['prefix']='';
	$para['callerid']['prefix_c']='';
	$para['callerid']['match_pattern']='';
	$para['callerid']['match_pattern_c']='';
	$para['callerid']['sdfr']='';
	$para['callerid']['sdfr_c']='';
	$para['callerid']['sta']='';
	$para['callerid']['sta_c']='';
	$para['callerid']['rdfr']='';
	$para['callerid']['rdfr_c']='';
	$para['callerid']['callername']='';
	$para['callerid']['callername_c']='';
	return $para;
}

function trunk_sw_select($trunk_sw)
{

	echo "<select title=\"Disabled Callernumber Vary\" name=\"trunk_sw[]\" onchange=\"trunk_sw_control(this)\">";
    //echo "<option value=\"off\">off";
	//echo "<option value=\"on\">on";
	
	if($trunk_sw == 'off'){
		echo "<option value=\"off\" selected >Modify_CallerID";
	}else {
		echo "<option value=\"off\">Modify_CallerID";
	}
	if($trunk_sw == 'on'){
		echo "<option value=\"on\" selected>Filter_CallerID";
	}else{
		echo "<option value=\"on\">Filter_CallerID";
	}
	echo "</select>";

}

function dial_pattern_html2($para,$convert=true)
{
	
	if ($para=='') {
		$para=initial_dial_pattern_para();
	}
	
	$para['calleeid']['match_pattern']=str_replace("{","[",$para['calleeid']['match_pattern']);
	$para['calleeid']['match_pattern']=str_replace("}","]",$para['calleeid']['match_pattern']);
	$para['callerid']['match_pattern']=str_replace("{","[",$para['callerid']['match_pattern']);
	$para['callerid']['match_pattern']=str_replace("}","]",$para['callerid']['match_pattern']);
	
	
	
	$prepend_e=$para['calleeid']['prepend'];
	$prepend_e_c=$para['calleeid']['prepend_c'];
	$prefix_e=$para['calleeid']['prefix'];
	$prefix_e_c=$para['calleeid']['prefix_c'];
	$match_pattern_e=$para['calleeid']['match_pattern'];
	$match_pattern_e_c=$para['calleeid']['match_pattern_c'];
	$sdfr_e=$para['calleeid']['sdfr'];
	$sdfr_e_c=$para['calleeid']['sdfr_c'];
	$sta_e=$para['calleeid']['sta'];
	$sta_e_c=$para['calleeid']['sta_c'];
	$rdfr_e=$para['calleeid']['rdfr'];
	$rdfr_e_c=$para['calleeid']['rdfr_c'];
	
	$prepend_r=$para['callerid']['prepend'];
	$prepend_r_c=$para['callerid']['prepend_c'];
	$prefix_r=$para['callerid']['prefix'];
	$prefix_r_c=$para['callerid']['prefix_c'];
	$match_pattern_r=$para['callerid']['match_pattern'];
	$match_pattern_r_c=$para['callerid']['match_pattern_c'];
	$sdfr_r=$para['callerid']['sdfr'];
	$sdfr_r_c=$para['callerid']['sdfr_c'];
	$sta_r=$para['callerid']['sta'];
	$sta_r_c=$para['callerid']['sta_c'];
	$rdfr_r=$para['callerid']['rdfr'];
	$rdfr_r_c=$para['callerid']['rdfr_c'];
	$callername_r=$para['callerid']['callername'];
	$callername_r_c=$para['callerid']['callername_c'];
	$trunk_sw = $para['callerid']['trunk_sw'];

	echo '<table class="tab_border_none" width="100%">';			
	if($convert) {
		$sign = '\\\'';
	} else {
		$sign = '\'';
	}
	
	
	//////////////////callee tr////////////////////////////////////////////////
	if($prepend_e=='')	$prepend_e=language('Prepend');
	if($prepend_e_c=='')	$prepend_e_c="style=\"color:#aaaaaa;\"";
	if($prefix_e=='')		$prefix_e=language('Prefix');
	if($prefix_e_c=='')	$prefix_e_c="style=\"color:#aaaaaa;\"";
	if($match_pattern_e=='')	$match_pattern_e=language('Match Pattern');
	if($match_pattern_e_c=='')	$match_pattern_e_c="style=\"color:#aaaaaa;\"";

	if($sdfr_e=='')		$sdfr_e=language('SDfR');
	if($sdfr_e_c=='')		$sdfr_e_c="style=\"color:#aaaaaa;\"";
	if($sta_e=='')	$sta_e=language('StA');
	if($sta_e_c=='')	$sta_e_c="style=\"color:#aaaaaa;\"";
	if($rdfr_e=='')		$rdfr_e=language('RdfR');
	if($rdfr_e_c=='')	$rdfr_e_c="style=\"color:#aaaaaa;\"";
	
	echo '<tr>';

	echo '<td class="td_talignr">';
	echo language('Callee_Dial_pattern');
	echo '</td>';
	
	echo '<td class="td_talignl">';
	echo "<input title=\"prepend\" type=\"text\" size=\"8\" name=\"prepend_e[]\" value=\"$prepend_e\" onfocus=\"_onfocus(this,${sign}".language('Prepend')."${sign})\" onblur=\"_onblur(this,${sign}".language('Prepend')."${sign})\" $prepend_e_c />";
	echo ' + ';
	echo "<input title=\"prefix\" type=\"text\" size=\"8\" name=\"prefix_e[]\" value=\"$prefix_e\" onfocus=\"_onfocus(this,${sign}".language('Prefix')."${sign})\" onblur=\"_onblur(this,${sign}".language('Prefix')."${sign})\" $prefix_e_c /> ";
	echo ' | ';
	echo "[ <input title=\"match pattern\" type=\"text\" size=\"16\" name=\"match_pattern_e[]\" value=\"$match_pattern_e\" onfocus=\"_onfocus(this,${sign}".language('Match Pattern')."${sign})\" onblur=\"_onblur(this,${sign}".language('Match Pattern')."${sign})\" $match_pattern_e_c />";
	echo ' ] | ';
	echo "( - <input title=\"sdfr\" type=\"text\" size=\"8\" name=\"sdfr_e[]\" value=\"$sdfr_e\" onfocus=\"_onfocus(this,${sign}".language('SDfR')."${sign})\" onblur=\"_onblur(this,${sign}".language('SDfR')."${sign})\" $sdfr_e_c />";
	echo ' + ';
	echo "<input title=\"sta\" type=\"text\" size=\"8\" name=\"sta_e[]\" value=\"$sta_e\" onfocus=\"_onfocus(this,${sign}".language('StA')."${sign})\" onblur=\"_onblur(this,${sign}".language('StA')."${sign})\" $sta_e_c /> ";
	echo ' ) | ';
	echo "<input title=\"rdfr\" type=\"text\" size=\"8\" name=\"rdfr_e[]\" value=\"$rdfr_e\" onfocus=\"_onfocus(this,${sign}".language('RdfR')."${sign})\" onblur=\"_onblur(this,${sign}".language('RdfR')."${sign})\" $rdfr_e_c /> &nbsp;&nbsp;&nbsp;";		
	echo '</td>';
	
	echo '<td rowspan=2>';
	echo '<img src="/images/delete.gif" style="float:none; margin-left:0px; margin-bottom:-3px;cursor:pointer;" alt="remove"';
	echo 'title="';
	echo language('Click here to remove this pattern');
	echo '" onclick="javascript:this.parentNode.parentNode.parentNode.parentNode.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode.parentNode.parentNode.parentNode.parentNode);">';
	echo '</td>';

	/////////////////caller tr/////////////////////////////////////////////////////

	if($prepend_r=='')	$prepend_r=language('Prepend');
	if($prepend_r_c=='')	$prepend_r_c="style=\"color:#aaaaaa;\"";
	if($prefix_r=='')		$prefix_r=language('Prefix');
	if($prefix_r_c=='')	$prefix_r_c="style=\"color:#aaaaaa;\"";
	if($match_pattern_r=='')	$match_pattern_r=language('Match Pattern');
	if($match_pattern_r_c=='')	$match_pattern_r_c="style=\"color:#aaaaaa;\"";
	if($sdfr_r=='')		$sdfr_r=language('SDfR');
	if($sdfr_r_c=='')		$sdfr_r_c="style=\"color:#aaaaaa;\"";
	if($sta_r=='')	$sta_r=language('StA');
	if($sta_r_c=='')	$sta_r_c="style=\"color:#aaaaaa;\"";
	if($rdfr_r=='')		$rdfr_r=language('RdfR');
	if($rdfr_r_c=='')	$rdfr_r_c="style=\"color:#aaaaaa;\"";	
	if($callername_r=='')		$callername_r=language('Caller Name');
	if($callername_r_c=='')		$callername_r_c="style=\"color:#aaaaaa;\"";
	
	
	echo '<tr>';

	echo '<td class="td_talignr">';
	echo language('Caller_Dial_pattern');
	echo '</td>';
	
	echo '<td class="td_talignl">';
	if($trunk_sw == 'on'){
		echo "<input title=\"prepend\" type=\"text\" size=\"8\" name=\"prepend_r[]\" value=\"$prepend_r\" disabled=\"disabled\" onfocus=\"_onfocus(this,${sign}".language('Prepend')."${sign})\" onblur=\"_onblur(this,${sign}".language('Prepend')."${sign})\" $prepend_r_c />";
		echo ' + ';
		echo "<input title=\"prefix\" type=\"text\" size=\"8\" name=\"prefix_r[]\" value=\"$prefix_r\" disabled=\"disabled\" onfocus=\"_onfocus(this,${sign}".language('Prefix')."${sign})\" onblur=\"_onblur(this,${sign}".language('Prefix')."${sign})\" $prefix_r_c /> ";
		echo ' | ';
	} else {
		echo "<input title=\"prepend\" type=\"text\" size=\"8\" name=\"prepend_r[]\" value=\"$prepend_r\" onfocus=\"_onfocus(this,${sign}".language('Prepend')."${sign})\" onblur=\"_onblur(this,${sign}".language('Prepend')."${sign})\" $prepend_r_c />";
		echo ' + ';
		echo "<input title=\"prefix\" type=\"text\" size=\"8\" name=\"prefix_r[]\" value=\"$prefix_r\" onfocus=\"_onfocus(this,${sign}".language('Prefix')."${sign})\" onblur=\"_onblur(this,${sign}".language('Prefix')."${sign})\" $prefix_r_c /> ";
		echo ' | ';
	}
	echo "[ <input title=\"match pattern\" type=\"text\" size=\"16\" name=\"match_pattern_r[]\" value=\"$match_pattern_r\" onfocus=\"_onfocus(this,${sign}".language('Match Pattern')."${sign})\" onblur=\"_onblur(this,${sign}".language('Match Pattern')."${sign})\" $match_pattern_r_c />";
	if($trunk_sw == 'on'){
		echo ' ] | ';
		echo "( - <input title=\"sdfr\" type=\"text\" size=\"8\" name=\"sdfr_r[]\" value=\"$sdfr_r\" disabled=\"disabled\" onfocus=\"_onfocus(this,${sign}".language('SDfR')."${sign})\" onblur=\"_onblur(this,${sign}".language('SDfR')."${sign})\" $sdfr_r_c />";
		echo ' + ';
		echo "<input title=\"sta\" type=\"text\" size=\"8\" name=\"sta_r[]\" value=\"$sta_r\" disabled=\"disabled\" onfocus=\"_onfocus(this,${sign}".language('StA')."${sign})\" onblur=\"_onblur(this,${sign}".language('StA')."${sign})\" $sta_r_c /> ";
		echo ' ) | ';
		echo "<input title=\"rdfr\" type=\"text\" size=\"8\" name=\"rdfr_r[]\" value=\"$rdfr_r\" disabled=\"disabled\" onfocus=\"_onfocus(this,${sign}".language('RdfR')."${sign})\" onblur=\"_onblur(this,${sign}".language('RdfR')."${sign})\" $rdfr_r_c /> ";		
		echo ' | ';
		echo "<input title=\"Caller Name\" type=\"text\" size=\"16\" name=\"callername_r[]\" value=\"$callername_r\" disabled=\"disabled\" onfocus=\"_onfocus(this,${sign}".language('Caller Name')."${sign})\" onblur=\"_onblur(this,${sign}".language('Caller Name')."${sign})\" $callername_r_c /> &nbsp;&nbsp;&nbsp;";
	} else {
		echo ' ] | ';
		echo "( - <input title=\"sdfr\" type=\"text\" size=\"8\" name=\"sdfr_r[]\" value=\"$sdfr_r\" onfocus=\"_onfocus(this,${sign}".language('SDfR')."${sign})\" onblur=\"_onblur(this,${sign}".language('SDfR')."${sign})\" $sdfr_r_c />";
		echo ' + ';
		echo "<input title=\"sta\" type=\"text\" size=\"8\" name=\"sta_r[]\" value=\"$sta_r\" onfocus=\"_onfocus(this,${sign}".language('StA')."${sign})\" onblur=\"_onblur(this,${sign}".language('StA')."${sign})\" $sta_r_c /> ";
		echo ' ) | ';
		echo "<input title=\"rdfr\" type=\"text\" size=\"8\" name=\"rdfr_r[]\" value=\"$rdfr_r\" onfocus=\"_onfocus(this,${sign}".language('RdfR')."${sign})\" onblur=\"_onblur(this,${sign}".language('RdfR')."${sign})\" $rdfr_r_c /> ";		
		echo ' | ';
		echo "<input title=\"Caller Name\" type=\"text\" size=\"16\" name=\"callername_r[]\" value=\"$callername_r\" onfocus=\"_onfocus(this,${sign}".language('Caller Name')."${sign})\" onblur=\"_onblur(this,${sign}".language('Caller Name')."${sign})\" $callername_r_c /> &nbsp;&nbsp;&nbsp;";
	}
	trunk_sw_select($trunk_sw);
	//echo "<input title=\"callername\" type=\"text\" size=\"16\" name=\"callername[]\" value=\"$callername\" onfocus=\"_onfocus(this,${sign}Caller Name${sign})\" onblur=\"_onblur(this,${sign}Caller Name${sign})\" $callername_c /> &nbsp;&nbsp;&nbsp;";
	echo '</td>';
	
	echo '<td>';
	echo '</td>';
	/*
	echo "<tr>";
	echo '<td class="td_talignr">';
	echo language('Caller Match Enable');
	echo "</td>";
	echo "<td>";
	trunk_sw_select($trunk_sw);
	echo "</td>";
	echo "</tr>";
	*/
	echo '</tr></table>';
	
}

function time_select($name, $time=NULL)
{
	//time format "00:00"
	if($time) $res = explode(':',$time,2);
	$h = '';
	$m = '';
	if(isset($res[0])) if(is_numeric($res[0])) $h = intval($res[0]);
	if(isset($res[1])) if(is_numeric($res[1])) $m = intval($res[1]);

	echo "<select name=\"h$name\">";
	echo '<option value="" >-';
	for($i=0; $i<24; $i++) {
		$val = sprintf("%02d",$i);
		if($h === $i) $sel = 'selected'; else $sel = '';
		echo "<option value=\"$val\" $sel > $val";
	}
	echo '</select>';

	echo ' : ';
	echo "<select name=\"m$name\">";
	echo '<option value="" >-';
	for($i=0; $i<60; $i++) {
		$val = sprintf("%02d",$i);
		if($m === $i) $sel = 'selected'; else $sel = '';
		echo "<option value=\"$val\" $sel > $val";
	}
	echo '</select>';
}

function week_select($name, $week=NULL)
{
	//week format "mon tue ....."

	$weeks = array(language('Monday'),language('Tuesday'),language('Wednesday'),language('Thursday'),language('Friday'),language('Saturday'),language('Sunday'));
	$values = array('mon','tue','wed','thu','fri','sat','sun');

	echo "<select name=\"$name\">";
	echo '<option value="" >-';
	for($i=0; $i<7; $i++) {
		if($week === $values[$i]) $sel = 'selected'; else $sel = '';
		echo "<option value=\"$values[$i]\" $sel > $weeks[$i]";
	}
	echo '</select>';
}

function day_select($name, $day=NULL)
{
	//day format "01"
	$d = '';
	if($day) $d = intval($day);

	echo "<select name=\"$name\">";
	echo '<option value="" >-';
	for($i=1; $i<=31; $i++) {
		$val = sprintf("%02d",$i);
		if($d === $i) $sel = 'selected'; else $sel = '';
		echo "<option value=\"$val\" $sel > $val";
	}
	echo '</select>';
}

function month_select($name, $month=NULL)
{
	//week format "jan feb ....."

	$months = array(language('January'),language('February'),language('March'),language('April'),language('May'),language('June'),language('July'),language('August'),language('September'),language('October'),language('November'),language('December'));
	$values = array('jan','feb','mar','apr','may','jun','jul','aug','sep','oct','nov','dec');

	echo "<select name=\"$name\">";
	echo '<option value="" >-';
	for($i=0; $i<12; $i++) {
		if($month === $values[$i]) $sel = 'selected'; else $sel = '';
		echo "<option value=\"$values[$i]\" $sel > $months[$i]";
	}
	echo '</select>';
}



function time_pattern_table($ts,$te,$ws,$we,$ds,$de,$ms,$me)
{
	echo '<table class="tab_border_none" width="100%">';
	//stat tr
	echo '<tr>';

	echo '<td class="td_talignr">';
	echo language('Time to start');
	echo ':</td>';
	echo '<td class="td_talignl">';
	time_select('stime[]',$ts);
	echo '</td>';

	echo '<td class="td_talignr">';
	echo language('Week Day start');
	echo ':</td>';
	echo '<td class="td_talignl">';
	week_select('sweek[]',$ws);
	echo '</td>';

	echo '<td class="td_talignr">';
	echo language('Month Day start');
	echo ':</td>';
	echo '<td class="td_talignl">';
	day_select('sday[]',$ds);
	echo '</td>';

	echo '<td class="td_talignr">';
	echo language('Month start');
	echo ':</td>';
	echo '<td class="td_talignl">';
	month_select('smonth[]',$ms);
	echo '</td>';

	echo '<td rowspan=2>';
	echo '<img src="/images/delete.gif" style="float:none; margin-left:0px; margin-bottom:-3px;cursor:pointer;" alt="remove"';
	echo 'title="';
	echo language('Click here to remove this pattern');
	echo '" onclick="javascript:this.parentNode.parentNode.parentNode.parentNode.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode.parentNode.parentNode.parentNode.parentNode);">';
	echo '</td>';

	//finish tr
	echo '<tr>';

	echo '<td class="td_talignr">';
	echo language('Time to finish');
	echo ':</td>';
	echo '<td class="td_talignl">';
	time_select('etime[]',$te);
	echo '</td>';

	echo '<td class="td_talignr">';
	echo language('Week Day finish');
	echo ':</td>';
	echo '<td class="td_talignl">';
	week_select('eweek[]',$we);
	echo '</td>';

	echo '<td class="td_talignr">';
	echo language('Month Day finish');
	echo ':</td>';
	echo '<td class="td_talignl">';
	day_select('eday[]',$de);
	echo '</td>';

	echo '<td class="td_talignr">';
	echo language('Month finish');
	echo ':</td>';
	echo '<td class="td_talignl">';
	month_select('emonth[]',$me);
	echo '</td>';

	echo '<td>';
	echo '</td>';

	echo '</tr></table>';
}


function add_routing_page($routing_name,$order = '')
{
	if($routing_name) {
		echo "<h4>";echo language('Modify a Call Routing Rule');echo "</h4>";
	} else {
		echo "<h4>";echo language('Create a Call Routing Rule');echo "</h4>";
	}

	$transfer_delay = check_oem_funs('call_forward_delay'); 

	$delay_enabled = 0; 	
	$delay_min = ''; 		// min
	$delay_max = ''; 		// max

	// /etc/asterisk/gw_routing.conf
	// [routing_name]
	// from_channel = channel (sip-sipname or gsm-gsmname  type: sip- is SIP, gsm- is GSM)
	// to_channel = channel,channel......
	// dial_pattern = prepend|prefix|match pattern|cid,prepend|prefix|match pattern|cid........
	// time_pattern = time|week|day|month,time|week|day|month........
	// cid_name
	// cid_number
	// forward_number
	
	//get the disa password
	$disa_passwd_str = '';
	if ($routing_name){
		$passwd_file = '/etc/asterisk/pin/'.$routing_name.'_disa_passwd.conf';
		if (file_exists($passwd_file)){ 	
			$fp = fopen($passwd_file,'r');						
			if ($fp) {
				for ($i=1;!feof($fp);$i++) {
					$disa_passwd_str .= fgets($fp);
				}
			}
		}
	}

	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk');
	if (!$setok) {
		return;
	}

	$section = $routing_name;

	if($section) {
		$hlock = lock_file('/etc/asterisk/gw_routing.conf');
		$res = $aql->query("select * from gw_routing.conf where section='$section'");
		unlock_file($hlock);
	}

	if (isset($res[$section]['transfer_delay'])) {
		$delay_setting = explode(',', $res[$section]['transfer_delay']);	
		if (isset($delay_setting[0])) $delay_enabled = $delay_setting[0];
		if (isset($delay_setting[1])) $delay_min = "$delay_setting[1]";
		if (isset($delay_setting[2])) $delay_max = "$delay_setting[2]";
	}

	if(isset($res[$section]['order'])) {
		if($routing_name) {
			$order = trim($res[$section]['order']);
		}
	}

	if(isset($res[$section]['from_channel'])) {
		$from_channel = trim($res[$section]['from_channel']);
	} else {
		$from_channel = '';
	}

	if(isset($res[$section]['to_channel'])) {
		$val = trim($res[$section]['to_channel']);
		$fctn_channel = explode(',',$val);
		if(isset($fctn_channel[0])) {
			$to_channel = $fctn_channel[0];
		} else {
			$to_channel = '';
		}
		array_shift($fctn_channel);   //Delete first
	} else {
		$to_channel = '';
	}

/*
	if(isset($res[$section]['dial_pattern'])) {
		$dial_pattern = parse_dial_pattern($res[$section]['dial_pattern']);
	}
*/
	if(isset($res[$section]['caller_dial_pattern'])) {
		$dial_pattern_caller = parse_dial_pattern($res[$section]['caller_dial_pattern']);	
	}

	if(isset($res[$section]['callee_dial_pattern'])) {
		$dial_pattern_callee = parse_dial_pattern($res[$section]['callee_dial_pattern']);
	}

	if(isset($res[$section]['time_pattern'])) {
		$time_pattern = parse_time_pattern($res[$section]['time_pattern']);
	}

	if(isset($res[$section]['cid_name'])) {
		$cid_name = trim($res[$section]['cid_name']);
	} else {
		$cid_name = '';
	}

	if(isset($res[$section]['cid_number'])) {
		$cid_number = trim($res[$section]['cid_number']);
	} else {
		$cid_number = '';
	}

	if(isset($res[$section]['forward_number'])) {
		$forward_number = trim($res[$section]['forward_number']);
	} else {
		$forward_number = '';
	}
	if(isset($res[$section]['custom_context'])) {
		$custom_context = trim($res[$section]['custom_context']);
	} else {
		$custom_context = '';
	}	
	$DISA_sw = '';
	if(isset($res[$section]['DISA_sw'])) {
		$val = trim($res[$section]['DISA_sw']);
		if ($val == 'on') {
			$DISA_sw = 'checked';
		}
	} 
	
	$second_dial_sw = '';
	if(isset($res[$section]['second_dial_sw'])) {
		$val = trim($res[$section]['second_dial_sw']);
		if ($val == 'on') {
			$second_dial_sw = 'checked';
		}
		
	} else {
		$second_dial_sw = '';
	}
	
		//get disa settings
	///////////////
	for ($i=1;$i<=10;$i++) {
		$disa_timeout[$i] = '';
		$disa_password_digits[$i] = '';
	}
	$disa_timeout[5] = 'selected';


	if (isset($res[$section]['timeout'])) {	
		$val = trim(($res[$section]['timeout']));
		$disa_timeout[$val] = 'selected';
	} else {
		$disa_timeout[5] = 'selected';		
	}	
	
	if (isset($res[$section]['max_passwd_digits'])) {
		$val = trim($res[$section]['max_passwd_digits']);
		$limit_length = $val;
		$disa_password_digits[$val] = 'selected';
	} else {
		$disa_password_digits[10] = 'selected';
		$limit_length = 10;
	}
	
	if(
		isset($dial_pattern) && is_array($dial_pattern) && count($dial_pattern) > 0 ||
		isset($time_pattern) && is_array($time_pattern) && count($time_pattern) > 0 ||
		isset($fctn_channel) && is_array($fctn_channel) && count($fctn_channel) > 0 ||
		$cid_name || 
		$cid_number ||
		$forward_number
	   ) {
		$show_adv = 'display:block';
	} else {
		$show_adv = 'display:none';
	}

?>
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/float_btn.js"></script>
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<script type="text/javascript">
function fromchselchange()
{
	var group_name, group_type;
	var groups = [];
	<?php 
		// $group_array = parse_ini_file("/etc/asterisk/gw_group.conf", true, INI_SCANNER_RAW);
		$group_array = get_all_groups(true);
		if(is_array($group_array)){
			foreach($group_array as $group_name => $group){
				foreach ($group as $key => $value) {
					if($key == 'type'){
		?>
					group_name = '<?php echo $group_name?>';
					group_type = '<?php echo $value?>';
					groups[group_name] = group_type;
		<?php
					}
				}
			}
		}
	?>
	document.getElementById('warn_anysip').innerHTML = '';
	var from_channel = document.getElementById('from_channel').value;
	
	if( from_channel == 'anysip') {
		document.getElementById('warn_anysip').innerHTML = con_str('<?php echo language('warn_anysip','Warning: It will set: allowguest=yes')?>');
	}
	
	var prepend = document.getElementsByName('prepend[]');
	var prefix = document.getElementsByName('prefix[]');
	var pattern = document.getElementsByName('pattern[]');
	var i;
	if(from_channel.indexOf("grp") == 0){
		var name = from_channel.substring(4);
		if(groups[name] == 'gsm'){
			
			for(i = 0; i < prepend.length; i++){
				
				prepend[i].disabled = true;
				prefix[i].disabled = true;
				pattern[i].disabled = true;
			}
		} else {
			for(i = 0; i < prepend.length; i++){
				prepend[i].disabled = false;
				prefix[i].disabled = false;
				pattern[i].disabled = false;
			}
		}
	} else if(from_channel.indexOf("gsm") == 0 || from_channel.indexOf("gsm") == 8){
		for(i = 0; i < prepend.length; i++){
				prepend[i].disabled = true;
				prefix[i].disabled = true;
				pattern[i].disabled = true;
			}
	} else {
		for(i = 0; i < prepend.length; i++){
			prepend[i].disabled = false;
			prefix[i].disabled = false;
			pattern[i].disabled = false;
		}
	}

}

function chselchange(obj)
{
	document.getElementById('warn_anysip').innerHTML = '';
	if(obj.options[obj.selectedIndex].value == 'anysip') {
		document.getElementById('warn_anysip').innerHTML = con_str('<?php echo language('warn_anysip','Warning: It will set: allowguest=yes')?>');
	}
}

function trunk_sw_control(obj)
{	
	var trunk_sw = trim(obj.value);
	var  obj_td;
	
	obj_td = obj.parentNode;
	var count = obj_td.childElementCount;
	td_childs = obj_td.childNodes;
	for(var i = 0; i < count; i++){
		if(obj_td == undefined || obj_td.children[i] == undefined){
			continue;
		}
		if(obj_td.children[i].nodeName == 'INPUT'){
			if(obj_td.children[i].getAttribute('title') != "match pattern"){
				if(trunk_sw == 'on'){
					obj_td.children[i].disabled = true;
				} else {
					obj_td.children[i].disabled = false;
				}
			}
		}
	}
	
}

function tochselchange()
{	
	var to_channel = document.getElementById('to_channel').value;
	var val = document.getElementById('custom_context');
	document.getElementById('cto_channel').innerHTML = '';
	document.getElementById('ccustom_context').innerHTML = '';
	if(to_channel == 'custom') {
		val.disabled = false;
		val.style.backgroundColor = "";
	} else {
		val.disabled = true;
		val.style.backgroundColor = "#d2d2d2";
	}
}

function _onchange(obj)
{
/*	var colDataRows = tbl_fctn.rows;
	for (var i = 0; i < colDataRows.length; i++)
    	alert(colDataRows[i].cells[0].childNodes[1].name);
 
	var tab = document.getElementById("tbl_fctn"); 
	var rows = tab.rows.length;
	for(var i = 1; i < rows; i++) {
		var cols = tab.rows[i].childNodes;
		for(var m = 0; m < cols.length; m++) {
			alert(cols[m].childNodes[0].length);
		}
	}
*/

	/*pattern[]*/
	var save = -1;
	var pr=document.getElementsByName(obj.name);
	for(i=0;i<pr.length;i++){
		if(pr[i]==obj) {
			save = i;
			break;
		}
	}

	if(save != -1) {
		var pa=document.getElementsByName('pattern[]');
		if(pa[save] && pa[save].value=='match pattern') {
			pa[save].value='.';
		}
	}
}

function _onfocus(obj,str)
{
	if (obj.value == str) {
		obj.value =''
		obj.style.color = '#000000';
	}
}

function _onblur(obj,str)
{
	if (trim(obj.value) =='') {
		obj.value = str;
		obj.style.color = '#aaaaaa';
	}
}

function addRow()
{
	var len = tbl_fctn.rows.length-1;
	var name = "fctn_channel"+len;

	//添加一行
	var newTr = tbl_fctn.insertRow(len);

	//添加两列
	var newTd0 = newTr.insertCell(0);
	var newTd1 = newTr.insertCell(0);

	//设置列内容和属性
	newTd0.innerHTML = '<td><?php failover_channel_select("'+name+'");?></td>';
	newTd1.innerHTML = '<th><?php echo language('Failover Call Through Number');?> ' + len + ':</th>'; 
}

function addDPRow()
{
	var len = tbl_dialroute.rows.length-1;

	//添加一行
	var newTr = tbl_dialroute.insertRow(len);
	var newTd = newTr.insertCell(0);
	newTd.innerHTML = '<?php dial_pattern_html('','','','','','','','');?>';

	//添加一列
	//var newTd0 = newTr.insertCell(0);
}

function addDPRow2()
{
	var len = tbl_callee_caller_manipulation.rows.length-1;

	var newTr = tbl_callee_caller_manipulation.insertRow(len);
	var newTd = newTr.insertCell(0);
	
	newTd.innerHTML = '<?php 
	$para='';
	dial_pattern_html2($para,true);?>';
	
	callee_pre_and_match_disabled();

	//var newTd0 = newTr.insertCell(0);
}

function addTPRow()
{
	var len = tbl_timeroute.rows.length-1;

	//添加一行
	var newTr = tbl_timeroute.insertRow(len);
	var newTd = newTr.insertCell(0);
	newTd.innerHTML = '<?php time_pattern_table('','','','','','','','')?>';
}

function disa_ami_change()
{
	$("#disa_sw").iButton();
	$("#second_dial_sw").iButton();
	$("#disa_secret_sw").iButton();
	var sw = document.getElementById('disa_sw').checked;
	if (sw) {
		set_visible('second_dial_ctl',true);
		set_visible('disa_timeout_ctl',true);
		set_visible('disa_password_digits_ctl',true);
		set_visible('disa_secret_ctl',true);
	} else {
		set_visible('second_dial_ctl',false);
		set_visible('disa_timeout_ctl',false);
		set_visible('disa_password_digits_ctl',false);
		set_visible('disa_secret_ctl',false);
	}
}
function delay_sw_change()
{
	$("#delay_sw").iButton();
	if(document.getElementById('delay_sw').checked) {
		$(".delay_edit_area").show();
	} else {
		$(".delay_edit_area").hide();
	}
}
function second_dial_change(){
	var sw = document.getElementById('second_dial_sw').checked;
	if (sw) {
		document.getElementById('forward_number').disabled = true;		
	} else {
		document.getElementById('forward_number').disabled = false;
	}
}

function select_length_change()
{
	var limit_length = document.getElementById('disa_password_digits').value;
	edit = $("#editsecret");
	
	var array_secret = edit.val().split(/[\r\n]+/g);
	//alert(array_secret.length);
	if ( array_secret.length >50 ) {
		alert("The lines of secret must be less than 50.\n\rPlease change password by clicking the 'Edit' Button.");
		return false;
	} else {
		for (var i=0;i<array_secret.length;i++){
			var secret = array_secret[i];
			var j = i + 1;
			if ( secret.length >limit_length ) {
				alert_str = "Sorry! The password digits does not match!!\n\r"+"Please re-select 'Max Password Digits' option,\n or change password by clicking the 'Edit' Button.";
				alert(alert_str);
				return false;
			} else {
				var regex = /\d*/i;
				var result = secret.match(regex);
				if (result != secret) {
					number_check = "Secret must be numbers(at:line "+ j + ").Please change password by clicking the 'Edit' Button.";
					alert(number_check);
					return false ;
				} else {
					//secret ok .....
				}							
			}					
		}
	}
	
}

function secret_edit()
{
	
	//document.getElementById('editsecret').value = 'hello';
	var limit_length = document.getElementById('disa_password_digits').value;
	var old_data = document.getElementById('editsecret').value;
	$("#DISA_secret_dg").dialog({
		resizeable: false,
		height:400,
		width:500,
		modal:true,
		buttons: {
			"Save Secret": function() {
				var save_flag = 0;
				edit = $("#editsecret");
				var array_secret = edit.val().split(/[\r\n]+/g);
				var secret_str = "";
				//alert(array_secret.length);
				if ( array_secret.length >50 ) {
					alert("The lines of secret must be less than 50.\n\rPlease delete some lines.");
					return ;
				} else {
					for (var i=0;i<array_secret.length;i++){
						var secret = array_secret[i];
						var j = i + 1;
						if ( secret.length >limit_length ) {
							alert_str = "Sorry! The password digits does not match!!\n\rPlease make sure the length of password(line "+j +") is less than "+limit_length+".";
							alert(alert_str);
							return ;
						} else {
							var regex = /\d*/i;
							var result = secret.match(regex);
							if (result != secret) {
								number_check = "Error! Secret must be numbers(line "+ j + ").\n\rPlease fix it.";
								alert(number_check);
								save_flag = 0;
								return ;
							} else {
								//secret ok .....
								save_flag = 1;
								secret_str = secret_str  + secret + "#";
							}							
						}					
					}
				}
				//alert("secret_str:" + secret_str);
				if (save_flag == 1) {
					//var res = document.getElementById('editsecret').value;
					//alert(document.getElementById('editsecret').value);
					alert("Save successfully.");
					$(this).dialog("close");
					document.getElementById('recv_secret').value = secret_str;
					//return document.getElementById('editsecret').value;
				}				
			},
			Cancel: function() {
				document.getElementById('editsecret').value = old_data;
				$(this).dialog( "close" );
			}
		}
	});
	return ;
}
function check()
{
<?php
	$aroutings = get_all_routings();

	$name_ary = '';
	if($aroutings) {
		foreach($aroutings as $routing) {		
			if(strcmp($routing['routing_name'],$routing_name)==0)
				continue;
			$name_ary .=  '"'.$routing['routing_name'].'"'.',';
		}
	}
	$name_ary = rtrim($name_ary,',');
?>
	var name_ary = new Array(<?php echo $name_ary; ?>);
	var routing_name = document.getElementById('routing_name').value;

	document.getElementById('crouting_name').innerHTML = '';
	for (var i in name_ary) 
	{
		if(name_ary[i] == routing_name) {
			document.getElementById('crouting_name').innerHTML = con_str('Already exist.');
			return false;
		}
	}

	var from_channel = document.getElementById('from_channel').value; 
	var to_channel = document.getElementById('to_channel').value; 

	if(!check_routingname(routing_name)) {
		document.getElementById("crouting_name").innerHTML = con_str('<?php echo htmlentities(language('js check routingname','Allowed character must be any of [0-9a-zA-Z`~!@#$%^*()_{}:|?-=.], 1-32 characters.'));?>');
		return false;
	}
	
	// check custom context settings
	var custom_context = document.getElementById('custom_context').value;
	if( to_channel == 'custom' && custom_context == '') {
		document.getElementById('cto_channel').innerHTML = con_str('Must set custom context.Check Advance Routing Rule.');
		document.getElementById("ccustom_context").innerHTML = con_str('Must set custom context.');		
		return false;
	} 
	

	// check delay setting
	// 2016-2-19
	// Lijun Huang
	$("#span_delay_min").html('');
	$("#span_delay_max").html('');
	if(<?PHP echo $transfer_delay?> && document.getElementById('delay_sw').checked) {
		var min = $.trim($("#txt_delay_min").val());
		var max = $.trim($("#txt_delay_max").val());
		// min value should be a legal number.
		if (min == "" || isNaN(min) || parseInt(min) < 0) {
			$("#span_delay_min").html(con_str('Pleas enter a legal value.'));
			return false;
		}
	
		// max value should be a legal number.
		if (max =="" || isNaN(max) || parseInt(max) < 0) {
			$("#span_delay_max").html(con_str('Pleas enter a legal value.'));
			return false;
		}
		// max value should greater than min value
		if (parseInt(max) < parseInt(min)) {
			$("#span_delay_min").html(con_str('min value should less than max value.'));
			$("#span_delay_max").html(con_str('max value should greater than min value.'));
			return false;	
		}

	}

	document.getElementById('cfrom_channel').innerHTML = '';
	if(from_channel == 'none') {
		document.getElementById('cfrom_channel').innerHTML = con_str('Must set.');
		return false;
	}

	document.getElementById('cto_channel').innerHTML = '';
	if(to_channel == 'none') {
		document.getElementById('cto_channel').innerHTML = con_str('Must set.');
		return false;
	}

	/*var divs = document.getElementsByNames('prepend[]');
	for(var i=0;i<divs.length;i++){
		alert(divs[i].value);
	}*/
	
	return select_length_change();
	
	
	return true;
}
</script>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<input type="hidden" id="order" name="order" value="<?php echo $order?>" />
	<input type="hidden" id="old_routing_name" name="old_routing_name" value="<?php echo htmlentities($routing_name);?>" />

	
	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_main')" id="tab_main_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_main')"><?php echo language('Call Routing Rule');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_main')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>

	<div id="tab_main" style="display:block">
		<table width="100%" class="tedit" >
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Routing Name');?>:
						<span class="showhelp">
						<?php echo language('Routing Name help',"
							The name of this route. Should be used to describe what types of calls this route matches(for example, 'SIP2GSM' or 'GSM2SIP').");
						?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" style="width:100px" name="routing_name" id="routing_name" value="<?php echo htmlentities($routing_name);?>" /><span id="crouting_name"></span>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Call Comes in From');?>:
						<span class="showhelp">
						<?php echo language('Call Comes in From help','The launching point of incoming calls.');?>
						</span>
					</div>
				</th>
				<td >
					<?php channel_select('from_channel',$from_channel,true) ?><span id="cfrom_channel"></span>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Send Call Through');?>:
						<span class="showhelp">
						<?php echo language('Send Call Through help','The destination to receive the incoming calls.');?>
						</span>
					</div>
				</th>
				<td >
					<?php channel_select('to_channel',$to_channel) ?><span id="cto_channel"></span>
				</td>
			</tr>
			</table>
		</div>	
		
		<div id="newline"></div>

		<div id="tab">
			<li class="tb_fold" onclick="lud(this,'tab_disa')" id="tab_disa_li">&nbsp;</li>
			<li class="tbg_fold" onclick="lud(this,'tab_disa')"><?php echo language('DISA Settings');?></li>
			<li class="tb2_fold" onclick="lud(this,'tab_disa')">&nbsp;</li>
			<li class="tb_end">&nbsp;</li>
		</div>
		
		<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
		<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>

		<div id="DISA_secret_dg" title="DISA Secret Edit" style="display:none">
			<center>
				<textarea id="editsecret" style="height:280px;width:460px;" rows="50" cols="10"><?php echo $disa_passwd_str;?></textarea>
			</center>
		</div>	

		<input type="hidden" name="recv_secret" id="recv_secret"  />
	
		<div id="tab_disa" style="display:block">
			<table width="100%" class="tedit" >
				<!-- ADD DISA ------- START      -------->
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Authentication');?>:
						<span class="showhelp">
						<?php echo language('Authentication help','Enable or disable password authentication.');?>
						</span>
					</div>
				</th>
				<td >
					<input type="checkbox" id="disa_sw" name="disa_sw" <?php echo $DISA_sw;?>  onchange="disa_ami_change()" />
				</td>
			</tr>
			
			<tr id= "second_dial_ctl" style = "display:none;">
				<th>
					<div class="helptooltips">
						<?php echo language('Secondary Dialing');?>:
						<span class="showhelp">
						<?php echo language('Secondary Dialing help','Enable or disable secondary dialing.');?>
						</span>
					</div>
				</th>
				<td >
					<input type="checkbox" id="second_dial_sw" name="second_dial_sw" <?php echo $second_dial_sw; ?> />
				</td>
			</tr>
			
			<tr id= "disa_timeout_ctl" style = "display:none;">
				<th>
					<div class="helptooltips">
						<?php echo language('DISA Timeout');?>:
						<span class="showhelp">
						<?php echo language('DISA Timeout help','Select the timemout range of 1 to 10 seconds.');?>
						</span>
					</div>
				</th>
				<td >
					<!--<?php channel_select('to_channel',$to_channel) ?><span id="cto_channel"></span>  
					 -->
				<select size=1 name="disa_timeout" id = "disa_timeout" style="width:80px" >
					<option value="1"  <?php echo $disa_timeout[1]?> >  1 s  </option>
					<option value="2"  <?php echo $disa_timeout[2]?> >  2 s  </option>
					<option value="3"  <?php echo $disa_timeout[3]?> >  3 s  </option>
					<option value="4"  <?php echo $disa_timeout[4]?> >  4 s  </option>
					<option value="5"  <?php echo $disa_timeout[5]?> >  5 s  </option>
					<option value="6"  <?php echo $disa_timeout[6]?> >  6 s  </option>
					<option value="7"  <?php echo $disa_timeout[7]?> >  7 s  </option>
					<option value="8"  <?php echo $disa_timeout[8]?> >  8 s  </option>
					<option value="9"  <?php echo $disa_timeout[9]?> >  9 s  </option>
					<option value="10"  <?php echo $disa_timeout[10]?> >  10 s </option>
				</select>
				</td>
			</tr>
			
			<tr id= "disa_password_digits_ctl" style = "display:none;">
				<th>
					<div class="helptooltips">
						<?php echo language('Max Password Digits');?>:
						<span class="showhelp">
						<?php echo language('Max Password Digits help','Restrict the max length of password.');?>
						</span>
					</div>
				</th>
				<td >
				<select size=1 name="disa_password_digits" id = "disa_password_digits" style="width:80px" onchange="select_length_change()">
					<option value="1"  <?php echo $disa_password_digits[1]?> >    1    </option>
					<option value="2"  <?php echo $disa_password_digits[2]?> >    2    </option>
					<option value="3"  <?php echo $disa_password_digits[3]?> >    3    </option>
					<option value="4"  <?php echo $disa_password_digits[4]?> >    4    </option>
					<option value="5"  <?php echo $disa_password_digits[5]?> >    5    </option>
					<option value="6"  <?php echo $disa_password_digits[6]?> >    6    </option>
					<option value="7"  <?php echo $disa_password_digits[7]?> >    7    </option>
					<option value="8"  <?php echo $disa_password_digits[8]?> >    8    </option>
					<option value="9"  <?php echo $disa_password_digits[9]?> >    9    </option>
					<option value="10"  <?php echo $disa_password_digits[10]?> >    10  </option>
				</select>
				</td>
			</tr>
			
			
			
			<tr id= "disa_secret_ctl" style = "display:none;">
				<th>
					<div class="helptooltips">
						<?php echo language('Password');?>:
						<span class="showhelp">
						<?php echo language('Password Authentication help','Click the button to edit the Authenticated Password.');?>
						</span>
					</div>
				</th>
				<td >
					<input type="button" style="width:80px"  value="<?php echo language('Edit');?>" onclick="secret_edit();"/>
				</td>
			</tr>
			<!-- ADD DISA ------- END     -------->
		</table>
	</div>

	<div id="newline"></div>

	<div id="tab">
		<li class="tb_fold" onclick="lud(this,'tab_advance')" id="tab_advance_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_advance')"><?php echo language('Advance Routing Rule');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_advance')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>

	<div id="tab_advance" style="<?php echo $show_adv;?>">
		<?php js_float_show_time();?>
		<table width="98%" class="tedit" align="right" id="tbl_callee_caller_manipulation" >
			<tr class="ttitle">
				<td>
					<div class="helptooltips">
						<?php echo language('CalleeID/callerID Manipulation');?>
						<span class="showhelp">
						<?php echo language('CalleeID Manipulation Help','
							
							A Dial Pattern is a unique set of digits that will select this route and send the call to the designated trunks.   If a dialed pattern matches this route, no subsequent routes will be tried.   If Time Groups are enabled, subsequent routes will be checked for matches outside of the designated time(s).
Rules:<br>
          <b>X</b> matches any digit from 0-9<br>
          <b>Z</b> matches any digit from 1-9<br>
          <b>N</b> matches any digit from 2-9<br>
          <b>[1237-9]</b>        matches any digit in the brackets (example: 1,2,3,7,8,9)<br>
           <b>.</b>&nbsp         wildcard, matches one or more dialed digits<br>
							<br><b> Prepend:</b> &nbsp;&nbsp;&nbsp;Digits to prepend to a successful match.  If the dialed number matches the patterns specified by the subsequent columns, then this will be prepended before sending to the trunks. <br/>
							<br><b>Prefix:</b> &nbsp;&nbsp;&nbsp;Prefix to remove on a successful match.  The dialed number is compared to this and the subsequent columns for a match. Upon a match, this prefix is removed from the dialed number before sending it to the trunks. <br/>
							<br><b>Match Pattern :</b>&nbsp;&nbsp;&nbsp;  The dialed number will be compared against the  prefix + this match pattern. Upon a match, the match pattern portion of the dialed number will be sent to the trunks.<br/>
							<br><b>SDfR(Stripped Digits from Right):</b> &nbsp;&nbsp;&nbsp;The amount of digits to be deleted from the right end of the number. If the value of this item exceeds the length of the current number, the whole number will be deleted. <br/>
							<br><b>RDfR(Reserved Digits from Right):</b> &nbsp;&nbsp;&nbsp;The amount of digits to be resevered from the right end of the number. If the value of this item under the length of the current number, the whole number will be reserverd. <br/>
							<br><b>StA(Suffix to Add) :</b> &nbsp;&nbsp;&nbsp;Designated information to be added to the right end of the current number.<br/>
							<br><b>Caller Name :</b> &nbsp;&nbsp;&nbsp;What caller name would you like to set before sending this call to the endpoint.<br/>
							<br><b>Disabled Caller Number Change :</b> &nbsp;&nbsp;Disable the caller number change, and fixed caller number match pattern.<br> 
						');
						?>
						</span>
					</div>
				</td>
			</tr>
<?php
		if (isset($dial_pattern_callee) && is_array($dial_pattern_callee) && isset($dial_pattern_caller) && is_array($dial_pattern_caller)) {
			$mylength=count($dial_pattern_callee);
			for ($i=0;$i<$mylength;$i++){
				
				//calleeid
				$para['calleeid']['prepend'] = language('Prepend');
				$para['calleeid']['prepend_c'] = 'style="color:#aaaaaa";';
				if(isset($dial_pattern_callee[$i]['prepend']) && $dial_pattern_callee[$i]['prepend'] != '' && $dial_pattern_callee[$i]['prepend'] != 'Prepend' ) {
					$para['calleeid']['prepend'] = $dial_pattern_callee[$i]['prepend'];
					$para['calleeid']['prepend_c'] = 'style="color:#000000";';
				}
				
				$para['calleeid']['prefix'] = language('Prefix');
				$para['calleeid']['prefix_c'] = 'style="color:#aaaaaa";';
				if(isset($dial_pattern_callee[$i]['prefix']) && $dial_pattern_callee[$i]['prefix'] != '' && $dial_pattern_callee[$i]['prefix'] != 'Prefix' ) {
					$para['calleeid']['prefix'] = $dial_pattern_callee[$i]['prefix'];
					$para['calleeid']['prefix_c'] = 'style="color:#000000";';
				}
				
				$para['calleeid']['match_pattern'] = language('Match Pattern');
				$para['calleeid']['match_pattern_c'] = 'style="color:#aaaaaa";';
				if(isset($dial_pattern_callee[$i]['match_pattern']) && $dial_pattern_callee[$i]['match_pattern'] != '' && $dial_pattern_callee[$i]['match_pattern'] != 'Match Pattern' ) {
					$para['calleeid']['match_pattern'] = $dial_pattern_callee[$i]['match_pattern'];
					$para['calleeid']['match_pattern_c'] = 'style="color:#000000";';
				}
				
				$para['calleeid']['sdfr'] = language('SDfR');
				$para['calleeid']['sdfr_c'] = 'style="color:#aaaaaa";';
				if(isset($dial_pattern_callee[$i]['sdfr']) && $dial_pattern_callee[$i]['sdfr'] != '' && $dial_pattern_callee[$i]['sdfr'] != 'SDfR' ) {
					$para['calleeid']['sdfr'] = $dial_pattern_callee[$i]['sdfr'];
					$para['calleeid']['sdfr_c'] = 'style="color:#000000";';
				}
				
				$para['calleeid']['sta'] = language('StA');
				$para['calleeid']['sta_c'] = 'style="color:#aaaaaa";';
				if(isset($dial_pattern_callee[$i]['sta']) && $dial_pattern_callee[$i]['sta'] != '' && $dial_pattern_callee[$i]['sta'] != 'StA' ) {
					$para['calleeid']['sta'] = $dial_pattern_callee[$i]['sta'];
					$para['calleeid']['sta_c'] = 'style="color:#000000";';
				}
				
				$para['calleeid']['rdfr'] = language('RdfR');
				$para['calleeid']['rdfr_c'] = 'style="color:#aaaaaa";';
				if(isset($dial_pattern_callee[$i]['rdfr']) && $dial_pattern_callee[$i]['rdfr'] != '' && $dial_pattern_callee[$i]['rdfr'] != 'RdfR' ) {
					$para['calleeid']['rdfr'] = $dial_pattern_callee[$i]['rdfr'];
					$para['calleeid']['rdfr_c'] = 'style="color:#000000";';
				}
				
				//callerID
				
				$para['callerid']['prepend'] = language('Prepend');
				$para['callerid']['prepend_c'] = 'style="color:#aaaaaa";';
				if(isset($dial_pattern_caller[$i]['prepend']) && $dial_pattern_caller[$i]['prepend'] != '' && $dial_pattern_caller[$i]['prepend'] != 'Prepend' ) {
					$para['callerid']['prepend'] = $dial_pattern_caller[$i]['prepend'];
					$para['callerid']['prepend_c'] = 'style="color:#000000";';
				}
				
				$para['callerid']['prefix'] = language('Prefix');
				$para['callerid']['prefix_c'] = 'style="color:#aaaaaa";';
				if(isset($dial_pattern_caller[$i]['prefix']) && $dial_pattern_caller[$i]['prefix'] != '' && $dial_pattern_caller[$i]['prefix'] != 'Prefix' ) {
					$para['callerid']['prefix'] = $dial_pattern_caller[$i]['prefix'];
					$para['callerid']['prefix_c'] = 'style="color:#000000";';
				}
				
				$para['callerid']['match_pattern'] = language('Match Pattern');
				$para['callerid']['match_pattern_c'] = 'style="color:#aaaaaa";';
				if(isset($dial_pattern_caller[$i]['match_pattern']) && $dial_pattern_caller[$i]['match_pattern'] != '' && $dial_pattern_caller[$i]['match_pattern'] != 'Match Pattern') {
					$para['callerid']['match_pattern'] = $dial_pattern_caller[$i]['match_pattern'];
					$para['callerid']['match_pattern_c'] = 'style="color:#000000";';
				}
				
				$para['callerid']['sdfr'] = language('SDfR');
				$para['callerid']['sdfr_c'] = 'style="color:#aaaaaa";';
				if(isset($dial_pattern_caller[$i]['sdfr']) && $dial_pattern_caller[$i]['sdfr'] != '' && $dial_pattern_caller[$i]['sdfr'] != 'SDfR' ) {
					$para['callerid']['sdfr'] = $dial_pattern_caller[$i]['sdfr'];
					$para['callerid']['sdfr_c'] = 'style="color:#000000";';
				}
				
				$para['callerid']['sta'] = language('StA');
				$para['callerid']['sta_c'] = 'style="color:#aaaaaa";';
				if(isset($dial_pattern_caller[$i]['sta']) && $dial_pattern_caller[$i]['sta'] != '' && $dial_pattern_caller[$i]['sta'] != 'StA' ) {
					$para['callerid']['sta'] = $dial_pattern_caller[$i]['sta'];
					$para['callerid']['sta_c'] = 'style="color:#000000";';
				}
				
				$para['callerid']['rdfr'] = language('RdfR');
				$para['callerid']['rdfr_c'] = 'style="color:#aaaaaa";';
				if(isset($dial_pattern_caller[$i]['rdfr']) && $dial_pattern_caller[$i]['rdfr'] != '' && $dial_pattern_caller[$i]['rdfr'] != 'RdfR' ) {
					$para['callerid']['rdfr'] = $dial_pattern_caller[$i]['rdfr'];
					$para['callerid']['rdfr_c'] = 'style="color:#000000";';
				}
				
				$para['callerid']['callername'] = language('Caller Name');
				$para['callerid']['callername_c'] = 'style="color:#aaaaaa";';
				if(isset($dial_pattern_caller[$i]['callername']) && $dial_pattern_caller[$i]['callername'] != '' && $dial_pattern_caller[$i]['callername'] != 'Caller Name' ) {
					$para['callerid']['callername'] = $dial_pattern_caller[$i]['callername'];
					$para['callerid']['callername_c'] = 'style="color:#000000";';
				}
				if(isset($dial_pattern_caller[$i]['trunk_sw']) && $dial_pattern_caller[$i]['trunk_sw'] != ''){
					$para['callerid']['trunk_sw'] = $dial_pattern_caller[$i]['trunk_sw'];
				}
				
				
				echo '<tr><td>';
					dial_pattern_html2($para,false);
				echo '</td></tr>';
				
		
			}				
				//$para['calleeid']['prepend_c']
		} else {
			echo '<tr><td>';
			$para=initial_dial_pattern_para();
			dial_pattern_html2($para,false);
			echo '</td></tr>';
		}
?>
			<tr>
				<td>
					<input type="button" value="+ <?php echo language('Add More Dial Pattern Fields');?>" onclick="addDPRow2();"/>
				</td>
			</tr>
		</table>


		<div id="newline"></div>
		<table width="98%" class="ttpedit" align="right" id="tbl_timeroute">
			<tr class="ttitle">
				<td colspan=5>
					<div class="helptooltips">
						<?php echo language('Time Patterns that will use this Route');?>
						<span class="showhelp">
					<?php echo language('Time Patterns that will use this Route help');?>
						</span>
					</div>
				</td>
			</tr>
			<?php
				if(isset($time_pattern) && is_array($time_pattern)) {
					foreach($time_pattern as $each) {
						echo '<tr><td>';
						time_pattern_table($each['stime'],$each['etime'],$each['sweek'],$each['eweek'],$each['sday'],$each['eday'],$each['smonth'],$each['emonth']);
						echo '</td></tr>';
					}
				} else {
					echo '<tr><td>';
					time_pattern_table('','','','','','','','');
					echo '</td></tr>';
				}
			?>
			<tr>
				<td colspan=5 style="text-align:left;">
					<input type="button" value="+ <?php echo language('Add More Time Pattern Fields');?>" onclick="addTPRow()"/>
				</td>
			</tr>
		</table>

		<div id="newline"></div>

		<table width="98%" class="tedit" align="right" >
			<tr class="ttitle">
				<td colspan = 2>
					<div class="helptooltips">
						<?php echo language('Change Rules');?>
					</div>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Set the Caller ID Name to');?>
						<span class="showhelp">
						<?php echo language('Set the Caller ID Name to help','What caller ID name would you like to set before sending this call to the endpoint.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="cid_name" value="<?php echo $cid_name?>" style="width: 300px;" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Set the Caller ID Number to');?>
						<span class="showhelp">
						<?php echo language('Set the Caller ID Number to help','What caller number would you like to set before sending this call to the endpoint.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="cid_number" value="<?php echo $cid_number?>" style="width: 300px;" />
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Forward Number');?>
						<span class="showhelp">
						<?php echo language('Forward Number help','
							This number will be sent to target PBX or SIP Server as a DID number for inbound rule settings.');
						?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="forward_number" id="forward_number" value="<?php echo $forward_number?>" style="width: 300px;" />
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Custom Context');?>
						<span class="showhelp">
						<?php echo language('Custom Context help','customing the context of dialing rules');
						?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="custom_context" id="custom_context" value="<?php echo $custom_context;?>" style="width: 300px;" /><span id="ccustom_context"></span>
				</td>
			</tr>
			<!-- delay setting area -->
			<?php if ($transfer_delay) {?>
			<tr>		
				<th> 
					<div class="helptooltips">
						<?php echo language('Random Delay');?>:
						<span class="showhelp">
						<?php echo language('Random Delay Setting help');?>
						</span>
					</div>
				</th>
				<td>
				<input type="checkbox" id="delay_sw" name="delay_sw" <?php if($delay_enabled) echo 'checked';?>  onchange="delay_sw_change()" />
				</td>
			</tr>
			<tr class="delay_edit_area">
				<th> 
					<div class="helptooltips">
						<?php echo language('Delay MIN');?>:
						<span class="showhelp">
						<?php echo language('Delay MIN help');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="txt_delay_min" id="txt_delay_min" value="<?php echo $delay_min?>" style="width:100px" /> <span id="span_delay_min"></span>
				</td>
			</tr>
			<tr class="delay_edit_area">
				<th> 
					<div class="helptooltips">
						<?php echo language('Delay MAX');?>:
						<span class="showhelp">
						<?php echo language('Delay MAX help');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="txt_delay_max" id="txt_delay_max" value="<?php echo $delay_max?>" style="width:100px"/> <span id="span_delay_max"></span>
				</td>
			</tr>
			<?php }?>			
		</table>

		<div id="newline"></div>

		<table width="98%" class="tedit" id="tbl_fctn" align="right">
			<tr class="ttitle">
				<td colspan = 2>
					<div class="helptooltips">
						<?php echo language('Failover Call Through Number');?>
						<span class="showhelp">
						<?php echo language('Failover Call Through Number help','The gateway will attempt to send the call out each of these in the order you specify');?>
						</span>
					</div>
				</td>
			</tr>
			<?php
				if(isset($fctn_channel)) {
					$i = 0;
					foreach($fctn_channel as $channel) {
						$i++;
						echo '<tr>';
						echo "<th>";echo language('Failover Call Through Number');echo " $i: </th>";
						echo '<td>';
						failover_channel_select("fctn_channel$i",$channel);
						echo '</td>';
						echo '</tr>';
					}
				}
			?>
			<tr>
				<td colspan = 2>
					<input type="button" name="add_failover_call" value="<?php echo language('Add a Failover Call Through Provider');?>" onclick="addRow();" />
				</td>
			</tr>
		</table>
	</div>

	<div id="newline"></div>

	<br>

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
		
		$("#disa_sw").iButton();
		disa_ami_change();

		//second_dial_change();

				
		$("#delay_sw").iButton();
		delay_sw_change();
		tochselchange();
		fromchselchange();
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
	
	function callee_pre_and_match_disabled(){
		var from_channel = document.getElementById('from_channel').value;
		var temp = from_channel.indexOf('gsm');
		if(temp != -1){
			$("input[name='prefix_e[]']").attr('disabled','disabled');
			$("input[name='match_pattern_e[]']").attr('disabled','disabled');
		}else{
			$("input[name='prefix_e[]']").removeAttr('disabled');
			$("input[name='match_pattern_e[]']").removeAttr('disabled');
		}
	}

	callee_pre_and_match_disabled();
	$("#from_channel").change(function(){
		callee_pre_and_match_disabled();
	});
</script>
<div id="sort_out" class="sort_out">
</div>
	<div class="sort_gsm_cli" id="sort_gsm_cli">
		<div id="sort_info" class="sort_info" style="display:block">
		<?php
	$all_routings = get_all_routings(true);
	$routings_num = 0;
	if($all_routings){
		foreach($all_routings as $routing) {
			$routings_num +=1;
			$routing['routing_name'] = trim($routing['routing_name']);
			if($routing['routing_name'] == $routing_name){
?>
		<li><a style="color:#CD3278;" href="<?php echo get_self();?>?sel_routing_name=<?php echo $routing['routing_name']; ?>" ><?php echo $routing['routing_name']; ?></a></li>
<?php
			}else{
?>
		<li><a style="color:LemonChiffon4;" href="<?php echo get_self();?>?sel_routing_name=<?php echo $routing['routing_name']; ?>" ><?php echo $routing['routing_name']; ?></a></li>
<?php
			}
		}
	}
//Control the left navigation hidden or height by '$routings_num'.
	if($routings_num==0){
?>
<script type="text/javascript">
	$("#sort_out").hide();
	$("#sort_info").hide();
</script>
	<?php
	}elseif($routings_num <= 5){
	?>
<script type="text/javascript">
$(document).ready(function(){
	$("#sort_info").css("height","120px");
	fromchselchange();
});

</script>
<?php
	}
		?>
		</div>
	</div>
<?php
}
?>

<?php
$check_float = 0;

	if($_POST) {
		if( (isset($_POST['send']) && ($_POST['send'] == 'New Call Routing Rule') ) ) {
			//Add new
			if( isset($_POST['sel_routing_name']) && isset($_POST['order']) && $_POST['order'] ) {
				$check_float = 1;
				add_routing_page($_POST['sel_routing_name'],$_POST['order']);
			}
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Save') {
			//save_disa_secret();
			//echo $_POST['routing_name'];
			$name_head = $_POST['routing_name'];
			save_disa_password($name_head);
			if(save_routings()) {
				//ast_reload();
				wait_apply("exec","asterisk -rx \"core reload\" > /dev/null 2>&1 &");
				wait_apply("exec","asterisk -rx \"dialplan reload\" > /dev/null 2>&1 &");
			}
			show_routings();
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Apply') {
			
			//save_disa_secret();
			$name_head = $_POST['routing_name'];
			save_disa_password($name_head);
			$check_float = 1;
			if(save_routings()) {
				//ast_reload();
				wait_apply("exec","asterisk -rx \"core reload\" > /dev/null 2>&1 &");
				wait_apply("exec","asterisk -rx \"dialplan reload\" > /dev/null 2>&1 &");
			}
			add_routing_page($_POST['routing_name'],'');
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Delete') {
			if(isset($_POST['sel_routing_name']) && $_POST['sel_routing_name']) {
				del_routing($_POST['sel_routing_name']);
				show_routings();
				//ast_reload();
				wait_apply("exec","asterisk -rx \"core reload\" > /dev/null 2>&1 &");
			} else {
				if(isset($_POST['rules'])){
					$routing_rules = "";
					foreach($_POST['rules'] as $rule) {
						$routing_rules .= "$rule,";
					}
					del_routing($routing_rules);
					show_routings();
					//ast_reload();
					wait_apply("exec","asterisk -rx \"core reload\" > /dev/null 2>&1 &");
				} else {
					show_routings();
				}
			}
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Save Orders') {
			save_orders();
			show_routings();
			//ast_reload();
			wait_apply("exec","asterisk -rx \"core reload\" > /dev/null 2>&1 &");
		}
	} else if($_GET) {
		//Modify
		if( isset($_GET['sel_routing_name']) ) {
			add_routing_page($_GET['sel_routing_name'],'');
			$check_float = 1;
		}
	} else {
		show_routings();
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
