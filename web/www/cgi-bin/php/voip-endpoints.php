<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");

//AQL
require_once("/www/cgi-bin/inc/aql.php");
?>

<script type="text/javascript">

function onload_func_iax()
{
	registration_change(1);
	auth_change();
}
function getPage(value)
{
	window.location.href = '<?php echo get_self();?>?sel_endpoint_name='+escape(value)+'&type=sip';
}

function getPage_iax2(value)
{
	window.location.href = '<?php echo get_self();?>?sel_endpoint_name='+escape(value)+'&type=iax2';
}


function setValue(value1,value2)
{
	document.getElementById('sel_endpoint_name').value = value1;
	document.getElementById('order').value = value2;
}

function setValue_iax2(value1,value2)
{
	document.getElementById('sel_endpoint_name_iax2').value = value1;
	document.getElementById('order_iax2').value = value2;
}

function delete_click(value1,value2)
{
	ret = confirm("<?php echo language('Delete Selected confirm','Are you sure to delete you selected ?');?>");

	if(ret) {
		document.getElementById('sel_endpoint_name').value = value1;
		document.getElementById('order').value = value2;
		return true;
	}

	return false;
}

function delete_click_iax2(value1,value2)
{
	ret = confirm("<?php echo language('Delete Selected confirm','Are you sure to delete you selected ?');?>");

	if(ret) {
		document.getElementById('sel_endpoint_name_iax2').value = value1;
		document.getElementById('order_iax2').value = value2;
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

<script type="text/javascript" src="/js/functions.js"></script>


<?php
function show_sip_endpoints()
{
	$all_sips = get_all_sips(true);
	$last_order = 1;
	if($all_sips) {
		$last = end($all_sips); 
		if(isset($last['order']) && $last['order'] != '') {
			$last_order = $last['order'] + 1;
		}
	}
?>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<div id="tab" style="height:30px;">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('SIP Endpoint'); ?></li>
			<li class="tb2">&nbsp;</li>
		</div>
	<table width="100%" class="tshow">
		<tr>
			<th style="width:03%" class="nosort">
				<input type="checkbox" name="selall_sip" onclick="selectAll(this.checked,'log[]')" />
			</th>
			<th><?php echo language('Endpoint Name');?></th>
			<th><?php echo language('Registration');?></th>
			<th><?php echo language('Credentials');?></th>
			<th width="80px"><?php echo language('Actions');?></th>
			<input type="hidden" id="sel_endpoint_name" name="sel_endpoint_name" value="" />
			<input type="hidden" id="order" name="order" value="" />
		</tr>
<?php
	if($all_sips){
		foreach($all_sips as $sip) {
			$credentials = $sip['username'];
			if(isset($sip['host'])) {
				$host = trim($sip['host']);
				if($host != 'dynamic' && $host != '') {
					$credentials .= '@'.$host;
				}
			}
			$sip['endpoint_name'] = htmlentities($sip['endpoint_name']);
			$credentials = htmlentities($credentials);
?>
		<tr>
			<td> 
				<input type="checkbox" name="log[]" value="<?php echo $sip['endpoint_name']; ?>" />
			</td>
			<td>
				<?php echo $sip['endpoint_name']; ?>
			</td>
			<td>
				<?php echo $sip['registration']; ?>
			</td>
			<td>
				<?php echo $credentials; ?>
			</td>
			<td>
				<button type="button" value="Modify" style="width:32px;height:32px;" 
					onclick="getPage('<?php echo $sip['endpoint_name']; ?>')">
					<img src="/images/edit.gif">
				</button>
				<button type="submit" value="Delete" style="width:32px;height:32px;" 
					onclick="document.getElementById('send').value='Delete SIP';return delete_click('<?php echo $sip['endpoint_name']; ?>', '')" >
					<img src="/images/delete.gif">
				</button>
			</td>
		</tr>
<?php
		}
	}
?>
	</table>
	<div id="newline"></div>
	<input type="hidden" name="send" id="send" value="" />
	
	<input type="submit" value="<?php echo language('Add New SIP Endpoint');?>" onclick="document.getElementById('send').value='Add New SIP Endpoint';setValue('', '<?php echo $last_order ?>')" />
	&nbsp;
	<input type="submit" value="<?php echo language('Delete');?>" onclick="document.getElementById('send').value='Delete';return batch_delete_click()" />

	</form>
	<br>
<?php
}
?>

<?php
function show_iax2_endpoints()
{
    //$all_iax = IaxGetAll(true);	
	$all_iax = get_all_iaxs();
	$last_order = 1;
	if($all_iax) {
		$last = end($all_iax); 
		if(isset($last['order']) && $last['order'] != '') {
			$last_order = $last['order'] + 1;
		}
	}
?>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<div id="tab" style="height:30px;">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('IAX2 Endpoint'); ?></li>
			<li class="tb2">&nbsp;</li>
		</div>
	<table width="100%" class="tshow">
		<tr>
			<th style="width:03%" class="nosort">
				<input type="checkbox" name="selall_iax" onclick="selectAll(this.checked,'iax[]')" />
			</th>
			<th><?php echo language('Endpoint Name');?></th>
			<th><?php echo language('Registration');?></th>
			<th><?php echo language('Credentials');?></th>
			<th width="80px"><?php echo language('Actions');?></th>
			<input type="hidden" id="sel_endpoint_name_iax2" name="sel_endpoint_name" value="" />
			<input type="hidden" id="order_iax2" name="order" value="" />
		</tr>
<?php
	if($all_iax){
		foreach($all_iax as $iax) {
			$credentials = $iax['username'];
			if(isset($iax['host'])) {
				$host = trim($iax['host']);
				if($host != 'dynamic' && $host != '') {
					$credentials .= '@'.$host;
				}
			}
			$i['endpoint_name'] = htmlentities($iax['endpoint_name']);
			$credentials = htmlentities($credentials);
?>
		<tr>
			<td> 
				<input type="checkbox" name="iax[]" value="<?php echo $iax['endpoint_name']; ?>" />
			</td>
			<td>
				<?php echo $iax['endpoint_name']; ?>
			</td>
			<td>
				<?php echo $iax['registration']; ?>
			</td>
			<td>
				<?php echo $credentials; ?>
			</td>
			<td>
				<button type="button" value="Modify" style="width:32px;height:32px;" 
					onclick="getPage_iax2('<?php echo $iax['endpoint_name']; ?>')">
					<img src="/images/edit.gif">
				</button>
				<button type="submit" value="Delete" style="width:32px;height:32px;" 
					onclick="document.getElementById('send_iax').value='Delete IAX2';return delete_click_iax2('<?php echo $iax['endpoint_name']; ?>', '')" >
					<img src="/images/delete.gif">
				</button>
			</td>
		</tr>
<?php
		}
	}
?>
	</table>
	<div id="newline"></div>
	<input type="hidden" name="send" id="send_iax" value="" />
	<input type="submit" value="<?php echo language('Add New IAX2 Endpoint');?>" onclick="document.getElementById('send_iax').value='Add New IAX2 Endpoint';setValue_iax2('', '<?php echo $last_order ?>')" />
	&nbsp;
	<input type="submit" value="<?php echo language('Delete');?>" onclick="document.getElementById('send_iax').value='Delete IAX';return batch_delete_click()" />
	</form>
<?php
}
?>



<?php
function save_specify_sipendpoint($sip_username, $url, $post)
{
	//$_POST['sel_chnl']
	$post['endpoint_name'] = $sip_username;
	if($url) {
		$buf = gw_send_POST($url, $post);
		if(strstr($buf,'anglog')){
			return true;
		}
	}
	
	return false;
}
function save_sip_endpoints()
{
	if(isset($_POST['endpoint_name']) && $_POST['endpoint_name'] != '') {
		$_endpoint_name = trim($_POST['endpoint_name']);
		$section = $_endpoint_name;
	} else {
		echo "Please set Name";
		exit(255);
	}

	$old_section = $section;
	if( isset($_POST['old_endpoint_name']) ) {
		$old_section = trim($_POST['old_endpoint_name']);
	}

	if(isset($_POST['registration']) && $_POST['registration'] != '') {
		$_registration = $_POST['registration'];
	} else {
		echo "Please set registration";
		exit(255);
	}

	if(isset($_POST['anonymous'])) {
		if($_registration != 'none') {
			echo "Anonymous only use registration 'none'";
			exit(255);
		}
		$_anonymous = 'yes';
		$_POST['sip_username'] = 'anonymous';
	} else {
		$_anonymous = 'no';
	}

	if(isset($_POST['sip_username']) && $_POST['sip_username'] != '') {
		$_sip_username = $_POST['sip_username'];
	} else {
		echo "Please set Username";
		exit(255);
	}

	if(isset($_POST['old_sip_username']) && $_POST['sip_username'] != '') {
		$old_sip_username = trim($_POST['sip_username']);
	} else {
		$old_sip_username = $_sip_username;
	}

		$datachunk = '';
		$datachunk .= "username=$_sip_username\n";
		$datachunk .= "registration=$_registration\n";
		$datachunk .= "allow_anonymous=$_anonymous\n";

	if( isset($_POST['order']) ) {
		$order = trim($_POST['order']);
		if($order == '') {
			echo "[$_sip_username] ";
			echo language('does not exist');
			//echo "Must set order";
			return false;
		}
		$datachunk .= 'order='.$order."\n";
	} else {
		echo "[$_sip_username] ";
		echo language('does not exist');
		//echo "Must set order";
		return false;
	}

	if(isset($_POST['sip_password']) && $_POST['sip_password'] != '') {
		$_sip_password = $_POST['sip_password'];
		$datachunk .= 'secret='.$_sip_password."\n";
	} else {
		$_sip_password = '';
	}
	

		if(isset($_POST['host']) && $_POST['host'] != '') {
			$_sip_ip = $_POST['host'];
			$datachunk .= 'host='.$_sip_ip."\n";
		} else {
			$_sip_ip = '';
		}

		if( isset($_POST['old_sip_ip']) ) {
			$_old_sip_ip = trim($_POST['old_sip_ip']);
		}

		if(isset($_POST['transport']) && $_POST['transport'] != '') {
			$val = $_POST['transport'];
			$transport = trim($_POST['transport']);
			$datachunk .= 'transport='.$val."\n";
		}

		if(isset($_POST['nat']) && $_POST['nat'] != '') {
			$val = $_POST['nat'];
			$datachunk .= 'nat='.$val."\n";
		}

		if(isset($_POST['authentication_user']) && $_POST['authentication_user'] != '') {
			$_authentication_user = $_POST['authentication_user'];
			$datachunk .= 'auth='.$_authentication_user."\n";
		} else {
			$_authentication_user = '';
		}

		if(isset($_POST['register_extension']) && $_POST['register_extension'] != '') {
			$_register_extension = $_POST['register_extension'];
			$datachunk .= 'register_extension='.$_register_extension."\n";
		} else {
			$_register_extension = '';
		}

		if(isset($_POST['register_user']) && $_POST['register_user'] != '') {
			$_register_user = $_POST['register_user'];
			$datachunk .= 'register_user='.$_register_user."\n";
		} else {
			$_register_user = '';
		}

		if(isset($_POST['registery_enable']) && $_POST['register_user'] != '') {
			$_registery_enable = "yes";
			$datachunk .= 'registery_enable='.$_registery_enable."\n";
		} else {
			$_registery_enable = "no";
		}

		if ( strcmp($_registery_enable,"yes")==0 ){
			if(isset($_POST['registery_string']) && $_POST['registery_string'] != '') {
				$_registery_string = $_POST['registery_string'];
				$datachunk .= 'registery_string='.$_registery_string."\n";
			}
		}
		
		if(isset($_POST['contact_user']) && $_POST['contact_user'] != ''){
			$val = $_POST['contact_user'];
			$datachunk .= 'contactuser='.$val."\n";
		}

		if(isset($_POST['from_user']) && $_POST['from_user'] != '') {
			$val = $_POST['from_user'];
			$datachunk .= 'fromuser='.$val."\n";
		}

		if(isset($_POST['fromdomain']) && $_POST['fromdomain'] != '') {
			$val = $_POST['fromdomain'];
			$datachunk .= 'fromdomain='.$val."\n";
		}

	/*	if(isset($_POST['remote_secret']) && $_POST['remote_secret'] != '') {
			$_remote_secret = $_POST['remote_secret'];
			$datachunk .= 'remotesecret='.$_remote_secret."\n";
		} else {
			$_remote_secret = '';
		}
	*/
		if(isset($_POST['port']) && $_POST['port'] != '') {
			$_port = $_POST['port'];
			$datachunk .= 'port='.$_port."\n";
		} else {
			$_port = 5060;
			$datachunk .= 'port='.$_port."\n";
		}

		if(isset($_POST['qualify']) && $_POST['qualify'] != '') {
			$val = $_POST['qualify'];
			$datachunk .= 'qualify='.$val."\n";
		}

		if(isset($_POST['qualifyfreq']) && $_POST['qualifyfreq'] != '') {
			$val = $_POST['qualifyfreq'];
			$datachunk .= 'qualifyfreq='.$val."\n";
		}

	if(isset($_POST['dtmf_mode']) && $_POST['dtmf_mode'] != '') {
		$val = $_POST['dtmf_mode'];
		$datachunk .= 'dtmfmode='.$val."\n";
	}

	if(isset($_POST['trust_remote_party_id']) && $_POST['trust_remote_party_id'] != '') {
		$val = $_POST['trust_remote_party_id'];
		$datachunk .= 'trustrpid='.$val."\n";
	}

	if(isset($_POST['send_remote_party_id']) && $_POST['send_remote_party_id'] != '') {
		$val = $_POST['send_remote_party_id'];
		$datachunk .= 'sendrpid='.$val."\n";
	}

	/*if(isset($_POST['remote_party_id_format']) && $_POST['remote_party_id_format'] != '') {
		$val = $_POST['remote_party_id_format'];
		$datachunk .= 'rpid_format='.$val."\n";
	}*/

	if(isset($_POST['caller_id_presentation']) && $_POST['caller_id_presentation'] != '') {
		$val = $_POST['caller_id_presentation'];
		$datachunk .= 'callingpres='.$val."\n";
	}

	if(isset($_POST['progress_inband']) && $_POST['progress_inband'] != '') {
		$val = $_POST['progress_inband'];
		$datachunk .= 'progressinband='.$val."\n";
	}

	if(isset($_POST['append_user_phone_to_uri']) && $_POST['append_user_phone_to_uri'] != '') {
		$val = $_POST['append_user_phone_to_uri'];
		$datachunk .= 'usereqphone='.$val."\n";
	}
	
	if(isset($_POST['add_q850_reason_headers']) && $_POST['add_q850_reason_headers'] != '') {
		$val = $_POST['add_q850_reason_headers'];
		$datachunk .= 'use_q850_reason='.$val."\n";
	}

		if(isset($_POST['honor_sdp_version']) && $_POST['honor_sdp_version'] != '') {
			$val = $_POST['honor_sdp_version'];
			$datachunk .= 'ignoresdpversion='.$val."\n";
		}
		if(isset($_POST['directmedia']) && $_POST['directmedia'] != ''){
			$val = trim($_POST['directmedia']);
			$datachunk .= 'directmedia='.$val."\n";
		}
				
		if(isset($_POST['allow_transfers']) && $_POST['allow_transfers'] != '') {
			$val = $_POST['allow_transfers'];
			$datachunk .= 'allowtransfer='.$val."\n";
		}				

	if(isset($_POST['allow_promiscuous_redirects']) && $_POST['allow_promiscuous_redirects'] != '') {
		$val = $_POST['allow_promiscuous_redirects'];
		$datachunk .= 'promiscredir='.$val."\n";
	}

	if(isset($_POST['max_forwards']) && $_POST['max_forwards'] != '') {
		$val = $_POST['max_forwards'];
		$datachunk .= 'max_forwards='.$val."\n";
	}

	if(isset($_POST['send_trying_on_register']) && $_POST['send_trying_on_register'] != '') {
		$val = $_POST['send_trying_on_register'];
		$datachunk .= 'registertrying='.$val."\n";
	}

		if(isset($_POST['outboundproxy']) && $_POST['outboundproxy'] != '') {
			$outboundproxy = $_POST['outboundproxy'];
		} else {
			$outboundproxy = '';
		}
		if(isset($_POST['outboundproxy_port']) && $_POST['outboundproxy_port']) {
			$outboundproxy_port = trim($_POST['outboundproxy_port']);
		} else {
			$outboundproxy_port = '';
		}
		if($outboundproxy != '' && $outboundproxy_port != ''){
			$datachunk .= "outboundproxy=".$outboundproxy.":".$outboundproxy_port."\n";
		} else if($outboundproxy != ''){
			$datachunk .= "outboundproxy=".$outboundproxy.":5060\n";
		}else{
			$datachunk .= 'outboundproxy='."\n";
		}

		if (isset($_POST['enableoutboundtohost']) && $_POST['enableoutboundtohost'] != '') {
			$enableoutboundtohost = "yes";
			$datachunk .= "enableoutboundtohost=" . "$enableoutboundtohost\n";
		} else {
			$enableoutboundtohost = "no";
			$datachunk .= "enableoutboundtohost=" . "$enableoutboundtohost\n";
		}
		
	if(isset($_POST['transport']) && $_POST['transport'] == 'tls'){
		$datachunk .= "tlsenable=yes\n";
		if(isset($_POST['tlsverify']) && $_POST['tlsverify'] != '') {
			$val = trim($_POST['tlsverify']);
			$datachunk .= "tlsverify=$val\n";
		}
		if(isset($_POST['tlssetup']) && $_POST['tlssetup'] != '') {
			$val = trim($_POST['tlssetup']);
			$datachunk .= "tlssetup=$val\n";
		}
		if(isset($_POST['tlsprivatekey']) && $_POST['tlsprivatekey'] != '') {
			$val = trim($_POST['tlsprivatekey']);
			$datachunk .= "tlsprivatekey=/etc/asterisk/keys/$val\n";
		}
		if(isset($_POST['encryption']) && $_POST['encryption'] != '') {
			$val = trim($_POST['encryption']);
			$datachunk .= "encryption=$val\n";
		}
	}

	if(isset($_POST['default_t1_timer']) && $_POST['default_t1_timer'] != '') {
		$val = $_POST['default_t1_timer'];
		$datachunk .= 'timert1='.$val."\n";
	}

	if(isset($_POST['call_setup_timer']) && $_POST['call_setup_timer'] != '') {
		$val = $_POST['call_setup_timer'];
		$datachunk .= 'timerb='.$val."\n";
	}
	
	if(isset($_POST['session_timers']) && $_POST['session_timers'] != '') {
		$val = $_POST['session_timers'];
		$datachunk .= 'session-timers='.$val."\n";
	}

	if(isset($_POST['minimum_session_refresh_interval']) && $_POST['minimum_session_refresh_interval'] != '') {
		$val = $_POST['minimum_session_refresh_interval'];
		$datachunk .= 'session-minse='.$val."\n";
	}

	if(isset($_POST['maximum_session_refresh_interval']) && $_POST['maximum_session_refresh_interval'] != '') {
		$val = $_POST['maximum_session_refresh_interval'];
		$datachunk .= 'session-expires='.$val."\n";
	}

	if(isset($_POST['session_refresher']) && $_POST['session_refresher'] != '') {
		$val = $_POST['session_refresher'];
		$datachunk .= 'session-refresher='.$val."\n";
	}
	
	if(isset($_POST['call_limit']) && $_POST['call_limit'] != '') {
		$val = $_POST['call_limit'];
		$datachunk .= 'call-limit='.$val."\n";
	}
	
	

		//register final to sip_general.conf [general] register=>
		if($_registration == "client") {
	/*		if($_remote_secret != '') {
				$pwd = ":$_remote_secret";
			} else*/ if($_sip_password != '') {
				$pwd = ":$_sip_password";
			} else {
				$pwd = '';
			}

			if($_port != '') {
				$port = ":$_port";
			} else {
				$port = '';
			}

			if($_authentication_user != '') {
				$auser = ":$_authentication_user";
			} else {
				$auser = '';
			}

			if($_register_extension != '') {
				$reg_exten = "/$_register_extension";
			} else {
				$reg_exten = '';
			}

			if($_register_user != '') {
				$reg_user = "$_register_user";
			} else {
				$reg_user = "$_sip_username";
			}
			
			if(strstr($_sip_ip,':')){
				$_sip_ip = '['.$_sip_ip.']';
			}
			
			if(strstr($outboundproxy,':')){
				$outboundproxy = '['.$outboundproxy.']';
			}
			
			if($outboundproxy.$outboundproxy_port != ''){
				if($outboundproxy_port != ''){
					$host = $outboundproxy.":".$outboundproxy_port;
				} else {
					$host = $outboundproxy;
				}
				$reg_user = $reg_user."@".$_sip_ip;
			} else {
				$host = $_sip_ip.$port;
			}
			if($transport == 'tcp'){
				$_register = '>'.'tcp://'.$reg_user.$pwd.$auser.'@'.$host.$reg_exten;
			} else if($transport == 'tls'){
				$_register = '>'.'tls://'.$reg_user.$pwd.$auser.'@'.$host.$reg_exten;
			}else{
				$_register = '>'.$reg_user.$pwd.$auser.'@'.$host.$reg_exten;
			}
			if ( strcmp($_registery_enable,"yes")==0 && $_registery_string != ''){
				$datachunk .= "register=$_registery_string\n";
			} else {
				$datachunk .= "register=$_register\n";
			}
		}
		
		//Fixed value
		////////////////////////////////
		//if( $_registration != "server" ) {
			//Any sip add 'insecure=port,invite',if not do this,Winsip Call has a lot socket to create. Freedom 2013-11-27 11:23
		$datachunk .= "insecure=port,invite\n";
	//}
	$datachunk .= "type=friend\n";
	$datachunk .= "context=$_sip_username\n";
	
	
	
	////////////////////////////////

		//Save to gw_endpoints.conf
			// Start new Add by 06152017
	/*******************************
	 *
	 *	Saving to other sip_endpoint
	 *
	 ************************************/
	$sync = false;
	if(isset($_POST['spans'])){
		$sync = true;
		
		//$sip_counts = count(get_all_sips());
		$sel_sip_counts = count(get_all_sips());
		$sync_sips = array();
		for($i = 1 ; $i <= $sel_sip_counts; $i++){
			if(isset($_POST['spans'][$i])){
				$sync_sips[$i] = $_POST['spans'][$i];
			}
		}
	/*
		ob_flush();
		flush();
		foreach($save_post_info as $each) {
			$pid = pcntl_fork();
			if($pid == 0) {
				ob_clean();
				ob_end_clean();
				if(isset($each['sip_username'])){
					save_specify_sipendpoint($each['sip_username'], $each['url'], $post);
				}
				exit(0);
			} else if($pid > 0) {
				$pids[] = $pid;
			} else {
			}
		}

		if(isset($pids) && is_array($pids)) {
			foreach($pids as $each) {
				pcntl_waitpid($each,$status);
			}
		}
	*/
	 }
	
	//Save to gw_endpoints.conf
	///////////////////////////////////////////////////
	$gw_endpoints_conf_path = '/etc/asterisk/gw_endpoints.conf';
	$hlock = lock_file($gw_endpoints_conf_path);
	if (!file_exists($gw_endpoints_conf_path)) fclose(fopen($gw_endpoints_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($gw_endpoints_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	if($sync){
		$extensions_routing_contents = file_get_contents("/etc/asterisk/extensions_routing.conf");
		foreach($sync_sips as $sip){
			$sel_sip_para = $aql->query("select * from gw_endpoints.conf where section='$sip'");
		// new modify start
			foreach($_POST as $key => $value) {
				if(preg_match('/sync/i', $key, $matches)){
					$tmp_array = explode('_', $key);
					$para = $tmp_array[0];
					if($para == 'host'){
						if(strstr($_POST[$para],':')){
							$tmp_ip = '['.$_POST[$para].']';
						}else{
							$tmp_ip = $_POST[$para];
						}
					}
					if($para == 'port') {
						$tmp_port = $_POST[$para];
					}
					if($para == 'transport'){
						$tmp_transport = trim($_POST[$para]);
						if($tmp_transport == 'tls'){
							if(isset($sel_sip_para[$sip]['tlsenable'])){
								$aql->assign_editkey($sip, 'tlsenable', 'yes');
							}else{
								$aql->assign_append($sip, 'tlsenable', 'yes');
							}
						}
					}
					if($para == 'tlsprivatekey'){
						$_POST[$para] = '/etc/asterisk/keys/'.$_POST[$para];
					}
					if(!isset($_POST[$para])){
						$_POST[$para] = "no";
					} else if($_POST[$para] == 'on') {
						$_POST[$para] = "yes";
					}
					if($para == 'outboundproxy') {
						if(strstr($_POST[$para],':')){
							$_POST[$para] = '['.$_POST[$para]."]:".$_POST[$para.'_port'];
						}else{
							$_POST[$para] = $_POST[$para].":".$_POST[$para.'_port'];
						}
						
						$tmp_outboundproxy = $_POST[$para];
					}
					if(isset($sel_sip_para[$sip][$para])){
						$aql->assign_editkey($sip, $para, $_POST[$para]);
					}else{
						$aql->assign_append($sip, $para, $_POST[$para]);
					}
					if(isset($sel_sip_para[$sip]['register'])){
						if(!empty($tmp_ip)) {
							$register = str_replace($sel_sip_para[$sip]['host'], $tmp_ip, $sel_sip_para[$sip]['register']);
						} else {
							$register = $sel_sip_para[$sip]['register'];
						}
						if(!empty($tmp_port)){
							if(isset($register) && $register != ''){
								$register = str_replace($sel_sip_para[$sip]['port'], $tmp_port, $register);
							} else {
								$register = str_replace($sel_sip_para[$sip]['port'], $tmp_port, $sel_sip_para[$sip]['register']);
							}
							//$register2 = str_replace($sel_sip_para[$sip]['port'], $port, $sel_sip_para[$sip]['register']);
						}
						if(!empty($tmp_outboundproxy)){
							//$register = str_replace($sel_sip_para[$sip]['host'].":".$sel_sip_para[$sip]['port'], $tmp_outboundproxy, $sel_sip_para[$sip]['register']);
							//$register = str_replace($sel_sip_para[$sip]['register_user'], $sel_sip_para[$sip]['register_user']."@".$sel_sip_para[$sip]['host'], $register);
							if(isset($sel_sip_para[$sip]['auth'])){
								$register = $sel_sip_para[$sip]['register_user'].'@'.$sel_sip_para[$sip]['host'].':'.$sel_sip_para[$sip]['secret'].':'.$sel_sip_para[$sip]['auth'].'@'.$tmp_outboundproxy.'/'.$sel_sip_para[$sip]['register_extension'];
							} else {
								$register = $sel_sip_para[$sip]['register_user'].'@'.$sel_sip_para[$sip]['host'].':'.$sel_sip_para[$sip]['secret'].'@'.$tmp_outboundproxy.'/'.$sel_sip_para[$sip]['register_extension'];
							}
							//$register = '>'.$sel_sip_para[$sip]['register_user'].'@'.$sel_sip_para[$sip]['host'].':'.$sel_sip_para[]
						}
						if(!empty($tmp_transport)){
							if($tmp_transport == 'tcp'){
								if(strstr($register, 'tls')){
									$register = str_replace('tls', 'tcp', $register);
								}else if(!strstr($register, 'tcp')){
									$register = 'tcp://'.$register;	
								}
							}
							
							if($tmp_transport == 'tls'){
								if(strstr($register, 'tcp')){
									$register = str_replace('tcp', 'tls', $register);
								}else if(!strstr($register, 'tls')){
									$register = 'tls://'.$register;
								}
							}
							
							if($tmp_transport == 'udp'){
								if(strstr($register, 'tcp')){
									$register = str_replace('tcp://', '', $register);
								}else if(strstr($register, 'tls')){
									$register = str_replace('tls://', '', $register);
								}
							}
						}
						if(isset($register) && $register != ''){
							$register = '>'.$register;
							$aql->assign_editkey($sip, 'register', $register);
						}
					}
				}
			}
			
			if(isset($_POST['outboundproxy_sync']) || ($sel_sip_para[$sip]['outboundproxy'] != '')) {
				if (isset($_POST['enableoutboundtohost_sync'])) {
					$current_outboundproxy_post = explode(':', $_POST['outboundproxy'], 2);
					if(isset($sel_sip_para[$sip]['outboundproxy'])){
						$current_outboundproxy_conf = explode(':', $sel_sip_para[$sip]['outboundproxy'], 2);
						$replace_endpoint_outboundproxy = $sip . '-' . $current_outboundproxy_conf[0];
						$replace_endpoint_host = $sip . '-' . $sel_sip_para[$sip]['host'];
						$new_endpoint_outboundproxy = $sip . '-' .$current_outboundproxy_post[0]; 

						$flag_ip = preg_match("/$replace_endpoint_host/i", $extensions_routing_contents, $matches);
						$flag_outboundproxy = preg_match("/$replace_endpoint_outboundproxy/i", $extensions_routing_contents, $matches);
						if($flag_ip){
							$extensions_routing_contents = str_replace($replace_endpoint_host, $new_endpoint_outboundproxy, $extensions_routing_contents);
						} elseif ($flag_outboundproxy) {
							$extensions_routing_contents = str_replace($replace_endpoint_outboundproxy, $new_endpoint_outboundproxy, $extensions_routing_contents);
						}
					} 
					
				}
			}

		} /* end of foreach($sync_sips as $sip){ */
		file_put_contents('/etc/asterisk/extensions_routing.conf', $extensions_routing_contents);
	}
	// end new adding
	//Save to gw_endpoints.conf
	///////////////////////////////////////////////////
	$aql->assign_delsection($old_section);
	$aql->save_config_file('gw_endpoints.conf');
	$aql->assign_addsection($section,$datachunk);
	$aql->save_config_file('gw_endpoints.conf');
	unlock_file($hlock);

	$content = file_get_contents("/etc/asterisk/extensions_routing.conf");
	if(isset($_POST['outboundproxy']) && trim($_POST['outboundproxy']) != '' && isset($_POST['enableoutboundtohost']) && trim($_POST['enableoutboundtohost']) != ''){
		//sync_extension_rongting_from_sip_change()
		//save_routings_to_extensions();
		
		$old_host = $_POST['old_sip_ip'];
		$replace_host = $old_section . '-' . $old_host;
		$old_outboundproxy = $_POST['old_outboundproxy'];
		$replace_outboundproxy = $old_section . '-' . $old_outboundproxy;
		$new_outboundproxy = $old_section . '-' . $_POST['outboundproxy'] ; 
		$flag_ip = preg_match("/$replace_host/i", $content, $matches);
		$flag_outboundproxy = preg_match("/$replace_outboundproxy/i", $content, $matches);
		if($flag_ip){
			$content = str_replace($replace_host, $new_outboundproxy, $content);
		} elseif ($flag_outboundproxy) {
			$content = str_replace($replace_outboundproxy, $new_outboundproxy, $content);
		}
		file_put_contents('/etc/asterisk/extensions_routing.conf', $content);
	} else {
		$new_host = $section . '-' . $_POST['host'];
		$old_outboundproxy = $_POST['old_outboundproxy'];
		if($old_outboundproxy != '') {
			$replace_outboundproxy = $old_section . '-' . $old_outboundproxy;
			$flag_outboundproxy = preg_match("/$replace_outboundproxy/i", $content, $matches);
			if ($flag_outboundproxy) {
				if($new_host != '-'){
					$content = str_replace($replace_outboundproxy, $new_host, $content);
					file_put_contents('/etc/asterisk/extensions_routing.conf', $content);
				}
			}
		}	
	}
		///////////////////////////////////////////////////
	 // ending save gw_endpoints.conf
	
	save_endpoints_to_sips();
	if($old_sip_username != $_sip_username){
		update_sipname_from_routing_conf($old_sip_username, $_sip_username);
	}
	save_routings_to_extensions();
}
?>
<?php
function save_iax_endpoints()
{
	if(isset($_POST['endpoint_name']) && $_POST['endpoint_name'] != '') {
		$_endpoint_name = trim($_POST['endpoint_name']);
		$section = $_endpoint_name;
	} else {
		echo "Please set Name";
		exit(255);
	}

	$old_section = $section;
	if( isset($_POST['old_endpoint_name']) ) {
		$old_section = trim($_POST['old_endpoint_name']);
	}

	if(isset($_POST['iax_username']) && $_POST['iax_username'] != '') {
		$_iax_username = $_POST['iax_username'];
	} else {
		echo "Please set Username";
		exit(255);
	}

	if(isset($_POST['registration']) && $_POST['registration'] != '') {
		$_registration = $_POST['registration'];
	} else {
		echo "Please set registration";
		exit(255);
	}
	
	
	//Save to gw_iax_endpoints.conf
	///////////////////////////////////////////////////
	$datachunk = '';
	$datachunk .= "username=$_iax_username\n";
	$datachunk .= "callerid=$_iax_username\n";
	$datachunk .= "registration=$_registration\n";
	

	/*
	if( isset($_POST['order']) ) {
		$order = trim($_POST['order']);
		if($order == '') {
			echo "[$_iax_username] ";
			echo language('does not exist');
			//echo "Must set order";
			return false;
		}
		$datachunk .= 'order='.$order."\n";
	} else {
		echo "[$_iax_username] ";
		echo language('does not exist');
		//echo "Must set order";
		return false;
	}
	*/
	//echo "pasword=".$_POST['iax_password'];
	
	if(isset($_POST['iax_password']) && $_POST['iax_password'] != '') {
		$_iax_password = $_POST['iax_password'];
		$datachunk .= 'secret='.$_iax_password."\n";
	} else {
		$_iax_password = '';
	}
	
	$datachunk .= "type=friend\n";
	
	if(isset($_POST['iax_ip']) && $_POST['iax_ip'] != '') {
		$_iax_ip = $_POST['iax_ip'];
		$datachunk .= 'host='.$_iax_ip."\n";
	} else {
		$_iax_ip = '';
	}

	if(isset($_POST['auth']) && $_POST['auth'] != '') {
		$_auth = $_POST['auth'];
		$datachunk .= 'auth='.$_auth."\n";
	} else {
		$_auth = '';
	}
	if(isset($_POST['rsa_value']) && $_POST['rsa_value'] != '') {
		$_rsa_value = $_POST['rsa_value'];
		$datachunk .= 'inkeys='.$_rsa_value."\n";
	} else {
		$_inkeys = '';
	}
	
	
	
	/*
	if(isset($_POST['transport']) && $_POST['transport'] != '') {
		$val = $_POST['transport'];
		$datachunk .= 'transport='.$val."\n";
	}
	*/
	
	if(isset($_POST['transfer']) && $_POST['transfer'] != '') {
		$val = $_POST['transfer'];
		$datachunk .= 'transfer='.$val."\n";
	}
	
	if(isset($_POST['trunk']) && $_POST['trunk'] != '') {
		$val = $_POST['trunk'];
		$datachunk .= 'trunk='.$val."\n";
	}


    if(isset($_POST['qualify']) && $_POST['qualify'] != '') {
		$val = $_POST['qualify'];
		$datachunk .= 'qualify='.$val."\n";
	}

	if(isset($_POST['qualifysmoothing']) && $_POST['qualifysmoothing'] != '') {
		$val = $_POST['qualifysmoothing'];
		$datachunk .= 'qualifysmoothing='.$val."\n";
	}

	if(isset($_POST['qualifyfreqok']) && $_POST['qualifyfreqok'] != '') {
		$val = $_POST['qualifyfreqok'];
		$datachunk .= 'qualifyfreqok='.$val."\n";
	}

	if(isset($_POST['qualifyfreqnotok']) && $_POST['qualifyfreqnotok'] != '') {
		$val = $_POST['qualifyfreqnotok'];
		$datachunk .= 'qualifyfreqnotok='.$val."\n";
	}
	
	

	if(isset($_POST['port']) && $_POST['port'] != '') {
		$_port= $_POST['port'];
		$val = $_POST['port'];
		$datachunk .= 'port='.$val."\n";
	}
	
	if(isset($_POST['requirecalltoken']) && $_POST['requirecalltoken'] != '') {
		$val = $_POST['requirecalltoken'];
		$datachunk .= 'requirecalltoken='.$val."\n";
	}
	
	


	if(isset($_POST['encryption']) && $_POST['encryption'] != '') {
		$val = $_POST['encryption'];
		$datachunk .= 'encryption='.$val."\n";
	}

	if(isset($_POST['forceencryption']) && $_POST['forceencryption'] != '') {
		$val = $_POST['forceencryption'];
		$datachunk .= 'forceencryption='.$val."\n";
	}


	if(isset($_POST['trunkmaxsize']) && $_POST['trunkmaxsize'] != '') {		
		$val = $_POST['trunkmaxsize'];
		$datachunk .= 'trunkmaxsize='.$val."\n";
	}
	
	if(isset($_POST['trunkmtu']) && $_POST['trunkmtu'] != '') {
		$val = $_POST['trunkmtu'];
		$datachunk .= 'trunkmtu='.$val."\n";
	}

	if(isset($_POST['trunkfreq']) && $_POST['trunkfreq'] != '') {
		$val = $_POST['trunkfreq'];
		$datachunk .= 'trunkfreq='.$val."\n";
	}
			
	if(isset($_POST['trunktimestamps']) && $_POST['trunktimestamps'] != '') {
		$val = $_POST['trunktimestamps'];
		$datachunk .= 'trunktimestamps='.$val."\n";
	}				

	if(isset($_POST['minregexpire']) && $_POST['minregexpire'] != '') {
		$val = $_POST['minregexpire'];
		$datachunk .= 'minregexpire='.$val."\n";
	}
	
	if(isset($_POST['maxregexpire']) && $_POST['maxregexpire'] != '') {
		$val = $_POST['maxregexpire'];
		$datachunk .= 'maxregexpire='.$val."\n";
	}
	
	//register final to sip_general.conf [general] register=>
	if($_registration == "client") {
		
		if($_iax_password != '') {
			$pwd = ":$_iax_password";
		} else {
			$pwd = '';
		}

		if($_port != '') {
			$port = ":$_port";
		} else {
			$port = '';
		}

		$_register = '>'.$_iax_username.$pwd.'@'.$_iax_ip.$port;
		$datachunk .= "register=$_register\n";
	}
	
	//fix value
	
	$datachunk .= "context=iax-$_iax_username\n";
	
	//print_rr( $datachunk);
	//echo "old=".$old_section."<br>";
	//echo "new=".$section."<br>";
	
	
		//Save to gw_iax_endpoints.conf
	///////////////////////////////////////////////////
	$gw_iax_endpoints_conf_path = '/etc/asterisk/gw_iax_endpoints.conf';
	$hlock = lock_file($gw_iax_endpoints_conf_path);
	if (!file_exists($gw_iax_endpoints_conf_path)) fclose(fopen($gw_iax_endpoints_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($gw_iax_endpoints_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	$aql->assign_delsection($old_section);
	$aql->save_config_file('gw_iax_endpoints.conf');
	$aql->assign_addsection($section,$datachunk);
	$aql->save_config_file('gw_iax_endpoints.conf');
	unlock_file($hlock);
	///////////////////////////////////////////////////
	
	
	//IaxSaveEndpoint($old_section,$section,$datachunk);
//	save_endpoints_to_sips();
//	save_routings_to_extensions();
	save_endpoints_to_iaxs();
	save_routings_to_extensions();
}
?>
<?php
function del_sip_endpoint($endpoint_name)
{
	// /etc/asterisk/gw_endpoints.conf
	//Save to gw_endpoints.conf
	///////////////////////////////////////////////////
	$gw_endpoints_conf_path = "/etc/asterisk/gw_endpoints.conf";
	$hlock = lock_file($gw_endpoints_conf_path);
	if (!file_exists($gw_endpoints_conf_path)) fclose(fopen($gw_endpoints_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($gw_endpoints_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	$output = explode(',', $endpoint_name);
	foreach($output as $sip) {	    
		if ($sip != ''){
			$aql->assign_delsection($sip);
			update_sipname_from_routing_conf($sip, '');
		}

	}
	$aql->save_config_file('gw_endpoints.conf');
	unlock_file($hlock);
	///////////////////////////////////////////////////

	save_endpoints_to_sips();
	save_routings_to_extensions();
}
?>

<?php
function del_iax_endpoint($endpoint_name)
{
	// /etc/asterisk/gw_endpoints.conf
	//Save to gw_iax_endpoints.conf
	///////////////////////////////////////////////////
	$gw_iax_endpoints_conf_path = "/etc/asterisk/gw_iax_endpoints.conf";
	$hlock = lock_file($gw_iax_endpoints_conf_path);
	if (!file_exists($gw_iax_endpoints_conf_path)) fclose(fopen($gw_iax_endpoints_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($gw_iax_endpoints_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	$output = explode(',', $endpoint_name);
	foreach($output as $iax) {
		if(!empty($iax))
			$aql->assign_delsection($iax);
	}
	$aql->save_config_file('gw_iax_endpoints.conf');
	unlock_file($hlock);
	///////////////////////////////////////////////////
	//IaxDelEndpoint($endpoint_name);
	save_endpoints_to_iaxs();
	save_routings_to_extensions();
}
?>
<?php
function add_sip_endpoint_page($endpoint_name = NULL, $order = '')
{
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk');
	
	//Get Configs
	//////////////////////////////////////////////////////////////////////////////////
	if($endpoint_name) {   //Modify SIP not Add New SIP
		echo "<h4>";echo language('Edit SIP Endpoint');echo " \"$endpoint_name\" </h4>";
		$hlock = lock_file('/etc/asterisk/gw_endpoints.conf');
		//$res = $aql->query("select * from gw_endpoints.conf where section='$endpoint_name'");
		$res = $aql->query("select * from gw_endpoints.conf");
		unlock_file($hlock);
	} else {
		echo '<h4>';echo language('Add New SIP Endpoint');echo '</h4>';
	}

/* /etc/asterisk/gw_endpoints.conf */
// [endpoint_name]
// order = 
// username = 
// registration =  
// allow_anonymous = yes|no
// register =    like sip.conf [general]
// .......       like all sip.conf
	///////////////////////////////////////////
	$reg_port = '';
	if(isset($res[$endpoint_name]['port'])) {
		$reg_port = trim($res[$endpoint_name]['port']);
	}
	
	if(isset($res[$endpoint_name]['register'])) {
		$register = trim($res[$endpoint_name]['register']);
		$reg = parse_register($register);
		if(isset($reg['port']))
			$reg_port = $reg['port'];
	}
	///////////////////////////////////////////

	//Get Registration
	///////////////////////////////////////////
	$registration['none'] = '';
	$registration['client'] = '';
	$registration['server'] = '';
	if(isset($res[$endpoint_name]['registration'])) {
		$val = trim($res[$endpoint_name]['registration']);
		if(strcasecmp($val,'client') == 0) {
			$registration['client'] = 'selected';
		} else if(strcasecmp($val,'server') == 0) {
			$registration['server'] =  'selected';
		} else {
			$registration['none'] = 'selected';
		}
	} else {
		$registration['none'] = 'selected';
	}
	///////////////////////////////////////////

	if(isset($res[$endpoint_name]['order'])) {
		if($endpoint_name) {
			$order = trim($res[$endpoint_name]['order']);
		}
	}

	if(isset($res[$endpoint_name]['username'])) {
		$sip_username = trim($res[$endpoint_name]['username']);
	} else {
		$sip_username = '';
	}

	if(isset($res[$endpoint_name]['secret'])) {
		$sip_password = trim($res[$endpoint_name]['secret']);
		$sip_password = htmlentities($sip_password);
	} else {
		$sip_password = '';
	}

	if(isset($res[$endpoint_name]['call-limit'])) {
		$call_limit = trim($res[$endpoint_name]['call-limit']);
		$call_limit = htmlentities($call_limit);
	} else {
		$call_limit = '';
	}
	
	if(isset($res[$endpoint_name]['allow_anonymous']) && $res[$endpoint_name]['allow_anonymous'] == 'yes') {
		$anonymous_check = 'checked';
		$sip_username = '';
		$sip_password = '';
		$sip_username_disable = 'disabled';
		$sip_password_disable = 'disabled';
	} else {
		$anonymous_check = '';
		$sip_username_disable = '';
		$sip_password_disable = '';
	}

	if(isset($res[$endpoint_name]['host'])) {
		$sip_ip = trim($res[$endpoint_name]['host']);
	} else {
		$sip_ip = '';
	}

	$transport['tcp'] = '';
	$transport['udp'] = '';
	$transport['tls'] = '';
	if(isset($res[$endpoint_name]['transport'])) {
		if($res[$endpoint_name]['transport'] == 'tls'){
			$transport['tls'] = 'selected';
		}else if($res[$endpoint_name]['transport'] == 'tcp'){
			$transport['tcp'] = 'selected';
		}else{
			$transport['udp'] = 'selected';
		}
	} else {
		$transport['udp'] = 'selected';
	}

	$nat_traversal['no'] = '';
	$nat_traversal['force_rport'] = '';
	$nat_traversal['yes'] = '';
	$nat_traversal['comedia'] = '';
	if(isset($res[$endpoint_name]['nat'])) {
		$val = trim($res[$endpoint_name]['nat']);
		if(strcasecmp($val,'no') == 0) {
			$nat_traversal['no'] =  'selected';
		} else if(strcasecmp($val,'force_rport') == 0) {
			$nat_traversal['force_rport'] =  'selected';
		} else if(strcasecmp($val,'comedia') == 0) {
			$nat_traversal['comedia'] =  'selected';
		} else {
			$nat_traversal['yes'] = 'selected';
		}
	} else {
		$nat_traversal['yes'] = 'selected';
	}

	if(isset($res[$endpoint_name]['auth'])) {
		$authentication_user = trim($res[$endpoint_name]['auth']);
	} else {
		$authentication_user = '';
	}

	if(isset($res[$endpoint_name]['register_extension'])) {
		$register_extension = trim($res[$endpoint_name]['register_extension']);
	} else {
		$register_extension = '';
	}

	if(isset($res[$endpoint_name]['register_user'])) {
		$register_user = trim($res[$endpoint_name]['register_user']);
	} else {
		$register_user = '';
	}

	if(isset($res[$endpoint_name]['registery_enable'])) {
		$registery_enable = trim($res[$endpoint_name]['registery_enable']);
	} else {
		$registery_enable = 'no';
	}

	if(isset($res[$endpoint_name]['registery_string'])) {
		$registery_string = trim($res[$endpoint_name]['registery_string']);
	} else {
		$registery_string = '';
	}

	if(isset($res[$endpoint_name]['fromuser'])) {
		$from_user = trim($res[$endpoint_name]['fromuser']);
	} else {
		$from_user = '';
	}
	
	if(isset($res[$endpoint_name]['contactuser'])){
		$contact_user = trim($res[$endpoint_name]['contactuser']);
	}else{
		$contact_user = '';
	}

	if(isset($res[$endpoint_name]['fromdomain'])) {
		$from_domain = trim($res[$endpoint_name]['fromdomain']);
	} else {
		$from_domain = '';
	}

/*	if(isset($res[$endpoint_name]['remotesecret'])) {
		$remote_secret = trim($res[$endpoint_name]['remotesecret']);
	} else {
		$remote_secret = '';
	}
*/
	$qualify['yes'] = '';
	$qualify['no'] = '';
	if(isset($res[$endpoint_name]['qualify'])) {
		$val = trim($res[$endpoint_name]['qualify']);
		if(strcasecmp($val,'yes') == 0) {
			$qualify['yes'] = 'selected';
		} else {
			$qualify['no'] = 'selected';
		}
	} else {
		$qualify['no'] = 'selected';
	}

	if(isset($res[$endpoint_name]['qualifyfreq'])) {
		$qualify_frequency = trim($res[$endpoint_name]['qualifyfreq']);
	} else {
		$qualify_frequency = 60;
	}

	$dtmf_mode['inband'] = '';
	$dtmf_mode['info'] = '';
	$dtmf_mode['rfc2833'] = '';
	if(isset($res[$endpoint_name]['dtmfmode'])) {
		$val = trim($res[$endpoint_name]['dtmfmode']);
		if(strcasecmp($val,'inband') == 0) {
			$dtmf_mode['inband'] =  'selected';
		} else if(strcasecmp($val,'info') == 0) {
			$dtmf_mode['info'] =  'selected';
		} else {
			$dtmf_mode['rfc2833'] = 'selected';
		}
	} else {
		$dtmf_mode['rfc2833'] = 'selected';
	}

	$trust_remote_party_id['yes'] = '';
	$trust_remote_party_id['no'] = '';
	if(isset($res[$endpoint_name]['trustrpid'])) {
		$val = trim($res[$endpoint_name]['trustrpid']);
		if(strcasecmp($val,'yes') == 0) {
			$trust_remote_party_id['yes'] = 'selected';
		} else {
			$trust_remote_party_id['no'] = 'selected';
		}
	} else {
		$trust_remote_party_id['no'] = 'selected';
	}

	$send_remote_party_id['yes'] = '';
	$send_remote_party_id['no'] = '';
	$send_remote_party_id['rpid'] = '';
	$send_remote_party_id['pai'] = '';

	if(isset($res[$endpoint_name]['sendrpid'])) {
		$val = trim($res[$endpoint_name]['sendrpid']);
		if(strcasecmp($val,'yes') == 0) {
			$send_remote_party_id['yes'] = 'selected';
		} else if(strcasecmp($val,'rpid') == 0) {
			$send_remote_party_id['rpid'] = 'selected';
		} else if(strcasecmp($val,'pai') == 0) {
			$send_remote_party_id['pai'] = 'selected';
		} else {
			$send_remote_party_id['no'] = 'selected';
		}
	} else {
		$send_remote_party_id['no'] = 'selected';
	}

	/*$remote_party_id_format['pass'] = '';
	$remote_party_id_format['rpid'] = '';
	if(isset($res[$endpoint_name]['rpid_format'])) {
		$val = trim($res[$endpoint_name]['rpid_format']);
		if(strcasecmp($val,'rpid') == 0) {
			$remote_party_id_format['rpid'] = 'selected';
		} else {
			$remote_party_id_format['pass'] = 'selected';
		}
	} else {
		$remote_party_id_format['pass'] = 'selected';
	}*/

	$caller_id_presentation['allowed_passed_screen'] = '';
	$caller_id_presentation['allowed_not_screen'] = '';
	$caller_id_presentation['allowed_failed_screen'] = '';
	$caller_id_presentation['allowed'] = '';
	$caller_id_presentation['prohib_not_screen'] = '';
	$caller_id_presentation['prohib_passed_screen'] = '';
	$caller_id_presentation['prohib_failed_screen'] = '';
	$caller_id_presentation['prohib'] = '';
	$caller_id_presentation['unavailable'] = '';
	if(isset($res[$endpoint_name]['callingpres'])) {
		$val = trim($res[$endpoint_name]['callingpres']);
		if(strcasecmp($val,'allowed_not_screen') == 0) {
			$caller_id_presentation['allowed_not_screen'] = 'selected';
		} else if(strcasecmp($val,'allowed_failed_screen') == 0) {
			$caller_id_presentation['allowed_failed_screen'] = 'selected';
		} else if(strcasecmp($val,'allowed') == 0) {
			$caller_id_presentation['allowed'] = 'selected';
		} else if(strcasecmp($val,'prohib_not_screen') == 0) {
			$caller_id_presentation['prohib_not_screen'] = 'selected';
		} else if(strcasecmp($val,'prohib_passed_screen') == 0) {
			$caller_id_presentation['prohib_passed_screen'] = 'selected';
		} else if(strcasecmp($val,'prohib_failed_screen') == 0) {
			$caller_id_presentation['prohib_failed_screen'] = 'selected';
		} else if(strcasecmp($val,'prohib') == 0) {
			$caller_id_presentation['prohib'] = 'selected';
		} else if(strcasecmp($val,'unavailable') == 0) {
			$caller_id_presentation['unavailable'] = 'selected';
		} else {
			$caller_id_presentation['allowed_passed_screen'] = 'selected';
		}
	} else {
		$caller_id_presentation['allowed_passed_screen'] = 'selected';
	}

	$progress_inband['never'] = '';
	$progress_inband['yes'] = '';
	$progress_inband['no'] = '';
	if(isset($res[$endpoint_name]['progressinband'])) {
		$val = trim($res[$endpoint_name]['progressinband']);
		if(strcasecmp($val,'yes') == 0) {
			$progress_inband['yes'] =  'selected';
		} else if(strcasecmp($val,'no') == 0) {
			$progress_inband['no'] =  'selected';
		} else {
			$progress_inband['never'] = 'selected';
		}
	} else {
		$progress_inband['never'] = 'selected';
	}

	$append_user_phone_to_uri['yes'] = '';
	$append_user_phone_to_uri['no'] = '';
	if(isset($res[$endpoint_name]['usereqphone'])) {
		$val = trim($res[$endpoint_name]['usereqphone']);
		if(strcasecmp($val,'yes') == 0) {
			$append_user_phone_to_uri['yes'] = 'selected';
		} else {
			$append_user_phone_to_uri['no'] = 'selected';
		}
	} else {
		$append_user_phone_to_uri['no'] = 'selected';
	}

	$add_q850_reason_headers['yes'] = '';
	$add_q850_reason_headers['no'] = '';
	if(isset($res[$endpoint_name]['use_q850_reason'])) {
		$val = trim($res[$endpoint_name]['use_q850_reason']);
		if(strcasecmp($val,'yes') == 0) {
			$add_q850_reason_headers['yes'] = 'selected';
		} else {
			$add_q850_reason_headers['no'] = 'selected';
		}
	} else {
		$add_q850_reason_headers['no'] = 'selected';
	}

	$honor_sdp_version['yes'] = '';
	$honor_sdp_version['no'] = '';
	if(isset($res[$endpoint_name]['ignoresdpversion'])) {
		$val = trim($res[$endpoint_name]['ignoresdpversion']);
		if(strcasecmp($val,'yes') == 0) {
			$honor_sdp_version['yes'] = 'selected';
		} else {
			$honor_sdp_version['no'] = 'selected';
		}
	} else {
		$honor_sdp_version['yes'] = 'selected';
	}
	$directmedia['yes'] = '';
	$directmedia['no'] = '';
	$directmedia['nonat'] = '';
	$directmedia['update'] = '';
	$directmedia['outgoing'] = '';
	if(isset($res[$endpoint_name]['directmedia'])) {
		$val = trim($res[$endpoint_name]['directmedia']);
		if(strcasecmp($val, 'yes') == 0){
			$directmedia['yes'] = 'selected';
		} elseif (strcasecmp($val, 'nonat') == 0) {
			$directmedia['nonat'] = 'selected';
		} elseif (strcasecmp($val, 'update') == 0) {
			$directmedia['update'] = 'selected';
		} elseif (strcasecmp($val, 'outgoing') == 0) {
			$directmedia['outgoing'] = 'selected';
		} elseif (strcasecmp($val, 'no') == 0) {
			$directmedia['no'] = 'selected';
		}
	} else {
		$directmedia['yes'] = 'selected';
	}
	$allow_transfers['yes'] = '';
	$allow_transfers['no'] = '';
	if(isset($res[$endpoint_name]['allowtransfer'])) {
		$val = trim($res[$endpoint_name]['allowtransfer']);
		if(strcasecmp($val,'yes') == 0) {
			$allow_transfers['yes'] = 'selected';
		} else {
			$allow_transfers['no'] = 'selected';
		}
	} else {
		$allow_transfers['yes'] = 'selected';
	}

	$allow_promiscuous_redirects['yes'] = '';
	$allow_promiscuous_redirects['no'] = '';
	if(isset($res[$endpoint_name]['promiscredir'])) {
		$val = trim($res[$endpoint_name]['promiscredir']);
		if(strcasecmp($val,'yes') == 0) {
			$allow_promiscuous_redirects['yes'] = 'selected';
		} else {
			$allow_promiscuous_redirects['no'] = 'selected';
		}
	} else {
		$allow_promiscuous_redirects['no'] = 'selected';
	}

	if(isset($res[$endpoint_name]['max_forwards'])) {
		$max_forwards = trim($res[$endpoint_name]['max_forwards']);
	} else {
		$max_forwards = '70';
	}

	$send_trying_on_register['yes'] = '';
	$send_trying_on_register['no'] = '';
	if(isset($res[$endpoint_name]['registertrying'])) {
		$val = trim($res[$endpoint_name]['registertrying']);
		if(strcasecmp($val,'yes') == 0) {
			$send_trying_on_register['yes'] = 'selected';
		} else {
			$send_trying_on_register['no'] = 'selected';
		}
	} else {
		$send_trying_on_register['no'] = 'selected';
	}

	if(isset($res[$endpoint_name]['outboundproxy'])) {
		if(strstr($res[$endpoint_name]['outboundproxy'], ':')){
			list($outbound_proxy, $outbound_proxy_port) = explode(':', $res[$endpoint_name]['outboundproxy']);
		} else {
			$outbound_proxy = $res[$endpoint_name]['outboundproxy'];
			$outbound_proxy_port = '';
		}
	} else {
		$outbound_proxy = '';
		$outbound_proxy_port = '';
	}

	if (isset($res[$endpoint_name]['enableoutboundtohost']) && trim($res[$endpoint_name]['enableoutboundtohost']) == 'yes') {
		$enableoutboundtohost = 'yes';
	} else {
		$enableoutboundtohost = 'no';
	}
	
	if(isset($res[$endpoint_name]['tlsverify'])){
		$tlsverify = trim($res[$endpoint_name]['tlsverify']);
	}else{
		$tlsverify = 'yes';
	}
	
	if(isset($res[$endpoint_name]['tlssetup'])){
		$tlssetup = trim($res[$endpoint_name]['tlssetup']);
	}else{
		$tlssetup = 'incoming_and_outcoming';
	}
	
	if(isset($res[$endpoint_name]['tlsprivatekey'])){
		$val = trim($res[$endpoint_name]['tlsprivatekey']);
		$tmp_arr = explode('/', $val);
		$tlsprivatekey = $tmp_arr[4];
	}else{
		$tlsprivatekey = '';
	}
	
	if(isset($res[$endpoint_name]['encryption'])) {
		$encryption = trim($res[$endpoint_name]['encryption']);
	} else {
		$encryption = 'yes';
	}

	if(isset($res[$endpoint_name]['timert1'])) {
		$default_t1_timer = trim($res[$endpoint_name]['timert1']);
	} else {
		$default_t1_timer = '500';
	}

	if(isset($res[$endpoint_name]['timerb'])) {
		$call_setup_timer = trim($res[$endpoint_name]['timerb']);
	} else {
		$call_setup_timer = '32000';
	}

	$session_timers['accept'] = '';
	$session_timers['originate'] = '';
	$session_timers['refuse'] = '';
	if(isset($res[$endpoint_name]['session-timers'])) {
		$val = trim($res[$endpoint_name]['session-timers']);
		if(strcasecmp($val,'originate') == 0) {
			$session_timers['originate'] =  'selected';
		} else if(strcasecmp($val,'refuse') == 0) {
			$session_timers['refuse'] =  'selected';
		} else {
			$session_timers['accept'] = 'selected';
		}
	} else {
		$session_timers['accept'] = 'selected';
	}

	if(isset($res[$endpoint_name]['session-minse'])) {
		$minimum_session_refresh_interval = trim($res[$endpoint_name]['session-minse']);
	} else {
		$minimum_session_refresh_interval = '90';
	}

	if(isset($res[$endpoint_name]['session-expires'])) {
		$maximum_session_refresh_interval = trim($res[$endpoint_name]['session-expires']);
	} else {
		$maximum_session_refresh_interval = '1800';
	}

	$session_refresher['uas'] = '';
	$session_refresher['uac'] = '';
	if(isset($res[$endpoint_name]['session-refresher'])) {
		$val = trim($res[$endpoint_name]['session-refresher']);
		if(strcasecmp($val,'uac') == 0) {
			$session_refresher['uac'] = 'selected';
		} else {
			$session_refresher['uas'] = 'selected';
		}
	} else {
		$session_refresher['uas'] = 'selected';
	}
	//////////////////////////////////////////////////////////////////////////////////

?>

<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/float_btn.js"></script>
<script type="text/javascript" src="/js/jquery.ibutton.js"></script>
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />

<script type="text/javascript">
function anonymous_click(value)
{
	if(value) {
		document.getElementById('sip_username').value = '';
		document.getElementById('sip_password').value = '';

		document.getElementById('sip_username').disabled = true;
		document.getElementById('sip_password').disabled = true;
		document.getElementById('sip_username').readOnly = true;
		document.getElementById('sip_password').readOnly = true;
	} else {
		document.getElementById('sip_username').disabled = false;
		document.getElementById('sip_password').disabled = false;
		document.getElementById('sip_username').readOnly = false;
		document.getElementById('sip_password').readOnly = false;
	}
}


var sip_ip_clear = false;
function registration_change(load)
{
	value = document.getElementById('registration').value;
	
	if(value == 'client') {
		document.getElementById('authentication_user').disabled = false;
//		document.getElementById('remote_secret').disabled = false;
		document.getElementById('port').disabled = false;
		document.getElementById('register_extension').disabled = false;
		document.getElementById('register_user').disabled = false;

		document.getElementById('host').readOnly = false;
		if(sip_ip_clear)
			document.getElementById('host').value = '';

		document.getElementById('anonymous').checked = false;
		document.getElementById('anonymous').disabled = true;
		anonymous_click(false);
		if(load == 0)
			document.getElementById('qualify').value = 'no';
	} else {
		document.getElementById('authentication_user').disabled = true;
//		document.getElementById('remote_secret').disabled = true;
		document.getElementById('register_extension').disabled = true;
		document.getElementById('register_user').disabled = true;

		if(value == 'none') {
			document.getElementById('port').disabled = false;
			document.getElementById('host').readOnly = false;
			if(sip_ip_clear)
				document.getElementById('host').value = '';

			document.getElementById('anonymous').disabled = false;
			if(load == 0)
				document.getElementById('qualify').value = 'no';
		} else if(value == 'server') {
			document.getElementById('port').disabled = true;
			document.getElementById('host').value = 'dynamic';
			document.getElementById('host').readOnly = true;
			sip_ip_clear = true;

			document.getElementById('anonymous').checked = false;
			document.getElementById('anonymous').disabled = true;
			if(load == 0)
				document.getElementById('qualify').value = 'yes';
			anonymous_click(false);
		}
	}
}

function sip_username_change(obj)
{
	/*(if(!document.getElementById('from_user_readonly').checked) {
		document.getElementById('from_user').value = obj.value;
	}*/

	if(!document.getElementById('register_extension_readonly').checked) {
		document.getElementById('register_extension').value = obj.value;
	}
	if(!document.getElementById('register_user_readonly').checked) {
		document.getElementById('register_user').value = obj.value;
	}
}

function click_contact_user_readonly(obj)
{
	if(obj.checked){
		document.getElementById('contact_user').readOnly = false;
	}else{
		document.getElementById('contact_user').readOnly = true;
	}
}

function click_from_user_readonly(obj)
{
	if(obj.checked) {
		document.getElementById('from_user').readOnly = false;
	} else {
		document.getElementById('from_user').readOnly = true;
	}
}

function click_register_extension_readonly(obj)
{
	if(obj.checked) {
		document.getElementById('register_extension').readOnly = false;
	} else {
		document.getElementById('register_extension').readOnly = true;
	}
}

function click_register_user_readonly(obj)
{
	if(obj.checked) {
		document.getElementById('register_user').readOnly = false;
	} else {
		document.getElementById('register_user').readOnly = true;
	}
}

function registery_change()
{
	var sw = document.getElementById('registery_enable').checked;
	if (sw) {
		set_visible('tr_registery_string', true);
	} else {
		set_visible('tr_registery_string', false);
	}
}

function onload_func()
{
	registration_change(1);
	registery_change();
	check();
}
function handle_port_sync()
{
		if(isAllCheckboxChecked('class','port')){
			$("#all_port").attr({"checked":true});
		}else{
			$("#all_port").attr({"checked":false});

		}
		if(isCheckboxChecked('class','port')){
			$(".setting_sync").show();
			$("#all_settings_sync").attr({"disabled":false,"checked":false});
			//selectAllCheckbox(true,'class','setting_sync');
			selectAllCheckbox(false,'class','setting_sync'); //default unchecked
			//handle_dl_sync();
		}else{
			$(".setting_sync").hide();
			$("#all_settings_sync").attr({"disabled":true,"checked":true});
			selectAllCheckbox(false,'class','setting_sync');
		}
}

function handle_setting_sync()
{
	if(isAllCheckboxChecked('class','setting_sync')){
		$("#all_settings_sync").attr({"checked":true});
	}else{
		$("#all_settings_sync").attr({"checked":false});
	}
}
function check()
{
<?php
	//$name_ip = $sip_username .'@'. $sip_ip;

	$asips = get_all_sips();

	$name_ary = '';
	$ip_ary = '';
	$nameandip_ary = '';
	if($asips) {
		foreach($asips as $sip) {		
			if(strcmp($sip['endpoint_name'],$endpoint_name)==0)
				continue;
			$name_ary .=  '"'.$sip['endpoint_name'].'"'.',';

			$ip_ary .= '"'.$sip['host'].'"'.',';
			$nameandip_ary .= '"'.$sip['username'].'@'.$sip['host'].'"'.',';
		}
	}
	$name_ary = rtrim($name_ary,',');
	$ip_ary = rtrim($ip_ary,',');
	$nameandip_ary = rtrim($nameandip_ary,',');
?>
	var name_ary = new Array(<?php echo $name_ary; ?>);
	var ip_ary = new Array(<?php echo $ip_ary; ?>);
	var nameandip_ary = new Array(<?php echo $nameandip_ary; ?>);

	var endpoint_name = document.getElementById('endpoint_name').value;
	var sip_username = document.getElementById('sip_username').value;
	var sip_password = document.getElementById('sip_password').value;
	var sip_ip = document.getElementById('host').value;
	var anonymous = document.getElementById('anonymous').checked;

	document.getElementById('cendpoint_name').innerHTML = '';
	document.getElementById('csip_username').innerHTML = '';
	document.getElementById('csip_ip').innerHTML = '';
	document.getElementById('cport').innerHTML = '';

	/*
	if(!check_sipendp(endpoint_name)) {
		document.getElementById("cendpoint_name").innerHTML = con_str('<?php echo htmlentities(language('js check sipendp','Allowed character must be any of [0-9a-zA-Z`~!@#$%^*()_{}:|?-=.], 1-32 characters.'));?>');
		return false;
	}else{
		document.getElementById("cendpoint_name").innerHTML = '';
	}
	*/
	if(!anonymous) {
		console.log("sip_username="+sip_username);
		/*
		if(!check_sipname(sip_username)) {
			document.getElementById("csip_username").innerHTML = con_str('<?php echo language('js check sipname','Allowed character must be any of [0-9a-zA-Z$*()-=_.], length: 1-32');?>');
			return false;
		}else{
			document.getElementById("csip_username").innerHTML = '';
		}
		if(sip_password != '' && !check_sippwd(sip_password)) {
			document.getElementById("csip_password").innerHTML = con_str('<?php echo htmlentities(language('js check sippwd','Allowed character must be any of [0-9a-zA-Z`~!#$%^&*()_+{}|<>-=,.], 0-32 characters.'));?>');
			return false;
		}else{
			document.getElementById("csip_password").innerHTML = '';
		}
		*/
	} else {
		sip_username = 'anonymous';
	}

	registration = document.getElementById('registration').value;
	/*
	if(registration != 'server') {
		if(!check_domain(sip_ip)) {
			document.getElementById("csip_ip").innerHTML = con_str('<?php echo language('js check domain','Invalid domain or IP address.');?>');
			return false;
		}
	}
	*/
	for (var i in name_ary) 
	{
		if(name_ary[i] == endpoint_name) {
			document.getElementById('cendpoint_name').innerHTML = con_str('You already had a same username.');
			return false;
		}
	}
	

	for (var i in nameandip_ary) 
	{
		/*
		if(nameandip_ary[i] == (sip_username+'@'+sip_ip)) {
			document.getElementById('csip_username').innerHTML = con_str('You already had a same '+nameandip_ary[i]+'.');
			return false;
		}*/

		if(anonymous) {
			if(nameandip_ary[i] == ('anonymous@'+sip_ip)) {
				document.getElementById('csip_ip').innerHTML = con_str('You exist has '+sip_ip+' set to anonymous.');
				return false;
			}
		}
	}

/*
	if(anonymous) {
		for (var i in ip_ary) 
		{
			if(ip_ary[i] == sip_ip) {
				document.getElementById('csip_ip').innerHTML = con_str('You already had a same '+ip_ary[i]+'.');
				return false;
			}
		}
	}
*/
	if(registration != 'server') {
		var port = document.getElementById('port').value;
		if(port != '') {
			if(!check_networkport(port)) {
				document.getElementById("cport").innerHTML = con_str('<?php echo language('js check networkport','Please input valid port number (1-65535)');?>');
				return false;
			}
		}
	}

	return true;
}
$(document).ready(function(){ 
	onload_func(); 
	
	$(".port").click(function(){
		handle_port_sync();
	});

	$(".setting_sync").click(function(){
		handle_setting_sync();
	});
});


</script>

	<form name="add_sip_endpoint" id="add_sip_endpoint" enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<input type="hidden" id="order" name="order" value="<?php echo $order?>" />
	<input type="hidden" id="old_endpoint_name" name="old_endpoint_name" value="<?php echo htmlentities($endpoint_name);?>" />
	<input type="hidden" id="old_sip_ip" name="old_sip_ip" value="<?php echo $sip_ip?>" />
	<input type="hidden" id="old_sip_username" name="old_sip_username" value="<?php echo $sip_username?>" />
	<input type="hidden" id="old_outboundproxy" name="old_outboundproxy" value="<?php echo $outbound_proxy?>">

	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_main')" id="tab_main_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_main')"><?php echo language('Main Endpoint Settings');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_main')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>
	<div id="tab_main" style="display:block">
		<table width="98%" class="tedit" align="right">
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Name');?>:
						<span class="showhelp">
						<?php echo language('Name help@sip-endpoints',"
							A name which is able to read by human. <br/>
							And it's only used for user's referance.");
						?>
						</span>
					</div>
				</th>
				<td >
					<input  type="text" name="endpoint_name" id="endpoint_name" value="<?php echo htmlentities($endpoint_name);?>" /> <span id="cendpoint_name"></span>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('User Name');?>:
						<span class="showhelp">
						<?php echo language('User Name help@ip-endpoints','User Name the endpoint will use to authenticate with the gateway.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="sip_username" id="sip_username" value="<?php echo $sip_username?>" onchange="sip_username_change(this)" <?php echo $sip_username_disable;?> />
					<input type="checkbox" name="anonymous" id="anonymous" <?php echo $anonymous_check?> onclick="anonymous_click(this.checked)" /><?php echo language('Anonymous');?>
					<span id="csip_username"></span>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Password');?>:
						<span class="showhelp">
						<?php echo language('Password help@ip-endpoints','
							Password the endpoint will use to authenticate with the gateway.<br/>
							Allowed characters');
						?>
						</span>
					</div>
				</th>
				<td >
					<input  type="password" name="sip_password" id="sip_password" value="<?php echo $sip_password?>" <?php echo $sip_username_disable;?> />
					<span style="vertical-align:middle;display:inline-block;"><input type="checkbox" id="show_sip_password" /></span>
					<span id="csip_password"></span>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Registration');?>:
						<span class="showhelp">
						<?php echo language('Registration help@sip-endpoints','Whether this endpoint will register to this gateway or this gateway to the endpoint.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" class="setting_sync" name="registration_sync" title="<?php echo $lang_sync_title;?>"/>
					<select size=1 name="registration" id="registration" onchange="registration_change(0)">
						<option value="none"    <?php echo $registration['none']?>> <?php echo language('_None');?> </option>
						<option value= "server" <?php echo $registration['server']?>> <?php echo language('Server');?> </option>
						<option value= "client" <?php echo $registration['client']?>> <?php echo language('Client');?> </option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Hostname or IP Address');?>:
						<span class="showhelp">
						<?php echo language('Hostname or IP Address help',"
							IP address or hostname of the endpoint or 'dynamic' if the endpoint has a dynamic IP address.<br/>
							This will require registration.<br>
							Notice: If the input here is hostname and your DNS has changed, you must <span style=\"color:rgb(255,0,0)\">reboot asterisk</span>.");
						?>
						</span>
					</div>
				</th>
				<td >
					<input type="checkbox" class="setting_sync" name="host_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="text" id="host" name="host" value="<?php echo $sip_ip?>" /><span id="csip_ip"></span>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Port');?>:
						<span class="showhelp">
						<?php echo language('Port help@sip-endpoints','The port number the gateway will connect to at this endpoint.');?>
						</span>
					</div>
				</th>
				<td >
					<input type="checkbox" class="setting_sync" name="port_sync" title="<?php echo $lang_sync_title;?>"/>
					<input type="text" id="port" name="port" value="<?php echo $reg_port?>" /><span id="cport"></span>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Transport');?>:
						<span class="showhelp">
						<?php echo language('Transport help','
							This sets the possible transport types for outgoing. Order of usage, <br/>
							when the respective transport protocols are inabled, is UDP, TCP, TLS.<br/>
							The first enabled transport type is only used for outbound messages until a<br/> 
							Registration takes place. During the peer Registration the transport type <br/>
							may change to another supported type if the peer requests so.');
						?>
						</span>
					</div>
				</th>
				<td >
					<input type="checkbox" class="setting_sync" name="transport_sync" title="<?php echo $lang_sync_title;?>"/>
					<select size=1 name="transport" id="transport">
						<option value="udp"  <?php echo $transport['udp']?> > UDP </option>
						<option value= "tcp" <?php echo $transport['tcp']?> > TCP </option>
						<option value="tls" <?php echo $transport['tls'];?> > TLS </option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('NAT Traversal');?>:
						<span class="showhelp">
						<?php echo language('NAT Traversal help','Addresses NAT-related issues in incoming SIP or media sessions.');?>
						</span>
					</div>
				</th>
				<td >
				<input type="checkbox" class="setting_sync" name="nat_sync" title="<?php echo $lang_sync_title;?>"/>
					<select size=1 name="nat" id="nat">
						<option value= "no"          <?php echo $nat_traversal['no'];?> > <?php echo language('_No');?> </option>
						<option value= "force_rport" <?php echo $nat_traversal['force_rport'];?> > <?php echo language('Force report on');?> </option>
						<option value= "yes"         <?php echo $nat_traversal['yes'];?> > <?php echo language('_Yes');?> </option>
						<option value= "comedia"     <?php echo $nat_traversal['comedia'];?> > <?php echo language('Report if requested and comedia');?> </option>
					</select>
				</td>
			</tr>
		</table>
	
		<div id="newline"></div>

		<div id="tab">
			<li class="tb_indent">&nbsp;</li>
			<li class="tb_fold" onclick="lud(this,'tab_adv_reg')" id="tab_adv_reg_li">&nbsp;</li>
			<li class="tbg_fold" onclick="lud(this,'tab_adv_reg')"><?php echo language('Advanced:Registration Options');?></li>
			<li class="tb2_fold" onclick="lud(this,'tab_adv_reg')">&nbsp;</li>
			<li class="tb_end2">&nbsp;</li>
		</div>
		<div id="tab_adv_reg" style="display:none">
			<table width="96%" class="tedit" align="right" >
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Authentication User');?>:
							<span class="showhelp">
							<?php echo language('Authentication User help','A username to use only for registration.');?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" name="authentication_user" id="authentication_user" value="<?php echo $authentication_user?>" />
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Register Extension');?>:
							<span class="showhelp">
							<?php echo language('Register Extension help','
								When Gateway registers as a SIP user agent to a SIP proxy(provider), <br/>
								calls from this provider connect to this local extension.');
							?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" id="register_extension" name="register_extension" value="<?php echo $register_extension?>" readonly />
						<input type="checkbox" id="register_extension_readonly" name="register_extension_readonly" onclick="click_register_extension_readonly(this)"/>
						<?php echo language('Modify');?>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Register User');?>:
							<span class="showhelp">
							<?php echo language('Register User help','
								Register user name , <br/>
								it is the user of register => user[:secret[:authuser]]@host[:port][/extension] .');
							?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" id="register_user" name="register_user" value="<?php echo $register_user?>" readonly />
						<input type="checkbox" id="register_user_readonly" name="register_user_readonly" onclick="click_register_user_readonly(this)"/>
						<?php echo language('Modify');?>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Contact User');?>:
							<span class="showhelp">
							<?php echo language('Contact User help','eg: When the Contact User is 402<br/>Contact: &lt;sip:402@172.16.6.123:5060;transport=UDP&gt;');?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" id="contact_user" name="contact_user" value="<?php echo $contact_user?>" readonly />
						<input type="checkbox" id="contact_user_readonly" name="contact_user_readonly" onclick="click_contact_user_readonly(this)"/>
						<?php echo language('Modify');?>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('From User');?>:
							<span class="showhelp">
							<?php echo language('From User help','A username to identify the gateway to this endpoint.');?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" id="from_user" name="from_user" value="<?php echo $from_user?>" readonly />
						<input type="checkbox" id="from_user_readonly" name="from_user_readonly" onclick="click_from_user_readonly(this)"/>
						<?php echo language('Modify');?>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('From Domain');?>:
							<span class="showhelp">
							<?php echo language('From Domain help','A domain to identify the gateway to this endpoint.');?>
							</span>
						</div>
					</th>
					<td >
						<input type="checkbox" class="setting_sync" name="fromdomain_sync" title="<?php echo $lang_sync_title;?>"/>
						<input type="text" name="fromdomain" value="<?php echo $from_domain?>" />
					</td>
				</tr>
<!--				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Remote Secret');?>:
							<span class="showhelp">
							<?php echo language('Remote Secret help','A password which is only used if the gateway registers to the remote side.');?>
							</span>
						</div>
					</th>
					<td >
						<input type="password" id="remote_secret" name="remote_secret" value="<?php echo $remote_secret?>" />
					</td>
				</tr> -->
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Qualify');?>:
							<span class="showhelp">
							<?php echo language('Qualify help@sip-endpoints',"Whether or not to check the endpoint's connection status.");?>
							</span>
						</div>
					</th>
					<td >
						<input type="checkbox" class="setting_sync" name="qualify_sync" title="<?php echo $lang_sync_title;?>"/>
						<select size=1 name="qualify" id="qualify">
							<option value="no"  <?php echo $qualify['no']?> > <?php echo language('_No');?> </option>
							<option value="yes" <?php echo $qualify['yes']?> > <?php echo language('_Yes');?> </option>
						</select>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Qualify Frequency');?>:
							<span class="showhelp">
							<?php echo language('Qualify Frequency help',"How often, in seconds, to check the endpoint's connection status.");?>
							</span>
						</div>
					</th>
					<td >
						<input type="checkbox" class="setting_sync" name="qualifyfreq_sync" title="<?php echo $lang_sync_title;?>"/>
						<input type="text" name="qualifyfreq" value="<?php echo $qualify_frequency?>" />
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Outbound Proxy');?>:
							<span class="showhelp">
							<?php echo language('Outbound Proxy help','
								A proxy to which the gateway will send all outbound signaling instead of sending signaling directly to endpoints.');
							?>
							</span>
						</div>
					</th>
					<td >
						<input type="checkbox" class="setting_sync" name="outboundproxy_sync" title="<?php echo $lang_sync_title;?>"/>
						<input  type="text" name="outboundproxy" value="<?php echo $outbound_proxy?>" />:
						<input type="text" name="outboundproxy_port" size="1" value="<?php echo $outbound_proxy_port?>">
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Custom Registery');?>:
							<span class="showhelp">
							<?php echo language('Custom Registery help','
								Custom Registery On / Off.');
							?>
							</span>
						</div>
					</th>
					<td>
						<input type=checkbox name="registery_enable" id="registery_enable" 
							<?php 
							if ( strcmp($registery_enable,"yes")==0 ){
								echo "checked";
							}
							?>
							onchange="registery_change()"
						/><span id="registery_enable"></span>
					</td>
				</tr>
				<tr id="tr_registery_string">
					<th>
						<div class="helptooltips">
							<?php echo language('Registery String');?>:
							<span class="showhelp">
							<?php echo language('Registery String help','
								Context of registery. user[:secret[:authuser]]@host[:port][/extension] </br>
								eg: 1001@sip.com:pbx122@172.16.6.122/1001 </br>');
							?>
							</span>
						</div>
					</th>
					<td>
						<input type=text name="registery_string" id="registery_string" value="<?php echo $registery_string; ?>" style="width:500px;" /> <span id="cregistery_string"></span>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Enable Outboundproxy to Host');?>:
							<span class="showhelp">
							<?php echo language('Enable Outboundproxy to Host help','
								Outboundproxy to Host On / Off.');
							?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="enableoutboundtohost_sync" title="<?php echo $lang_sync_title;?>"/>
						<input type=checkbox name="enableoutboundtohost" id="enableoutboundtohost" 
							<?php 
							if ( strcmp($enableoutboundtohost, "yes")==0 ){
								echo "checked";
							}
							?>
							onchange=""
						/><span id="registery_enable"></span>
					</td>
				</tr>
				
				<tr id="tls_tlsverify" style="display:none;">
					<th>
						<div class="helptooltips">
							<?php echo language('Tlsverify');?>:
							<span class="showhelp">
							<?php echo language('Tlsverify help','enable TLS verify or disable.'); ?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="tlsverify_sync" title="<?php echo $lang_sync_title;?>" />
						<select size=1 id="tlsverify" name="tlsverify">
							<option value="yes" <?php if($tlsverify == 'yes') echo 'selected';?>>Yes</option>
							<option value="no" <?php if($tlsverify == 'no') echo 'selected';?>>No</option>
						</select>
					</td>
				<tr>
				
				<tr id="tls_tlssetup" style="display:none;">
					<th>
						<div class="helptooltips">
							<?php echo language('Tlssetup');?>:
							<span class="showhelp">
							<?php echo language('Tlssetup help','Whether we are willing to accept connections, connect to the other party, or both.'); ?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="tlssetup_sync" title="<?php echo $lang_sync_title;?>"/>
						 <select size=1 id="tlssetup" name="tlssetup" >
							<option value="incoming_and_outcoming" <?php if($tlssetup == "incoming_and_outcoming") echo "selected";?>>Incoming and Outcoming</option>
							<option value="incoming" <?php if($tlssetup == "incoming") echo "selected";?>>Incoming Only</option>
							<option value="outcoming" <?php if($tlssetup == "outcoming") echo "selected";?>>Outcoming Only</option>
						 </select>
					</td>
				</tr>
				
				<tr id="tls_tlsprivatekey" style="display:none">
					<th>
						<div class="helptooltips">
							<?php echo language('Tlsprivatekey');?>:
							<span class="showhelp">
							<?php echo language('Tlsprivatekey help','Private key file (*.pem format only) for TLS connections. If no tlsprivatekey is specified, tlscertfile is searched for both public and private key.'); ?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="tlsprivatekey_sync" title="<?php echo $lang_sync_title;?>"/>
						<input type="text" name="tlsprivatekey" id="tlsprivatekey" value="<?php echo $tlsprivatekey;?>" />
					</td>
				</tr>
				
				<tr id="tls_encryption" style="display:none">
					<th>
						<div class="helptooltips">
							<?php echo language('Encryption');?>:
							<span class="showhelp">
							<?php echo language('Encryption help','Whether to offer SRTP encrypted media (and only SRTP encrypted media) on outgoing calls to a peer. Calls will fail with HANGUPCAUSE=58 if the peer does not support SRTP. Defaults to no.'); ?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="encryption_sync" title="<?php echo $lang_sync_title;?>"/>
						<select size=1 id="encryption" name="encryption" >
							<option value="yes" <?php if($encryption == "yes") echo "selected"?>>Yes (SRTP only)</option>
							<option value="no" <?php if($encryption == "no") echo "selected"?>>No</option>
						</select>
					</td>
				</tr>
			</table>
		</div>
	</div>

	<div id="newline"></div>

	<div id="tab">
		<li class="tb_fold" onclick="lud(this,'tab_call_set')" id="tab_call_set_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_call_set')"><?php echo language('Call Settings');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_call_set')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>

	<div id="tab_call_set" style="display:none">
		<table width="98%" class="tedit" align="right">
			<tr>
				<td colspan = 2>
				<?php echo language('DTMF Settings');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('DTMF Mode');?>:
						<span class="showhelp">
						<?php echo language('DTMF Mode help',"
							Set default dtmfmode for sending DTMF. Default:rfc2833. <br/>
							Other options: 'info', SIP INFO message(application/dtmf-relay);<br/>
							'inband',Inband audio(require 64kbit codec -alaw,ulaw).");
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="dtmf_mode">
						<option value="rfc2833" <?php echo $dtmf_mode['rfc2833']?> > <?php echo language('RFC2833');?> </option>
						<option value="inband"  <?php echo $dtmf_mode['inband']?> > <?php echo language('Inband');?> </option>
						<option value="info"    <?php echo $dtmf_mode['info']?> > <?php echo language('Info');?> </option>
					</select>
				</td>
			</tr>
		</table>
		
		<div id="newline"></div>
		<table width="98%" class="tedit" align="right">
			<tr>
				<td colspan = 2>
				<?php echo language('Caller ID Settings');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Trust Remote-Party-ID');?>:
						<span class="showhelp">
						<?php echo language('Trust Remote-Party-ID help','Whether or not the Remote-Party-ID header should be trusted.');?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="trust_remote_party_id">
						<option value="yes" <?php echo $trust_remote_party_id['yes']?> > <?php echo language('_Yes');?> </option>
						<option value="no"  <?php echo $trust_remote_party_id['no']?> > <?php echo language('_No');?> </option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Send Remote-Party-ID');?>:
						<span class="showhelp">
						<?php echo language('Send Remote-Party-ID help','Whether or not to send the Remote-Party-ID header.');?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="send_remote_party_id" id="send_remote_party_id">
						<option value="yes" <?php echo $send_remote_party_id['yes']?> >Yes</option>
						<option value="no"  <?php echo $send_remote_party_id['no']?> >No</option>
						<option value="rpid"  <?php echo $send_remote_party_id['rpid']?> >Rpid</option>
						<option value="pai"  <?php echo $send_remote_party_id['pai']?> >Pai</option>
					</select>
				</td>
			</tr>
			<!--
			<tr>
				<th>
					<div class="helptooltips">
						<?php //echo language('Remote Party ID Format');?>:
						<span class="showhelp">
						<?php //echo language('Remote Party ID Format help','How to set the Remote-Party-ID header: from Remote-Party-ID or from P-Asserted-Identity.');?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="remote_party_id_format" id="remote_party_id_format">
						<option value="pass" <?php //echo $remote_party_id_format['pass']?> > <?php //echo language('P-Asserted-Identity Header');?> </option>
						<option value="rpid" <?php //echo $remote_party_id_format['rpid']?> > <?php //echo language('Remote-Party-ID Header');?> </option>
					</select>
				</td>
			</tr> 
			-->
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Caller ID Presentation');?>:
						<span class="showhelp">
						<?php echo language('Caller ID Presentation help','Whether or not to display Caller ID.');?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="caller_id_presentation" id="caller_id_presentation">
						<option value="allowed_not_screen"     <?php echo $caller_id_presentation['allowed_not_screen']?> > <?php echo language('Allowed,not screened');?> </option>
						<option value="allowed_passed_screen"  <?php echo $caller_id_presentation['allowed_passed_screen']?> > <?php echo language('Allowed,passed screen');?> </option>
						<option value="allowed_failed_screen"  <?php echo $caller_id_presentation['allowed_failed_screen']?> > <?php echo language('Allowed,failed screen');?> </option>
						<option value="allowed"                <?php echo $caller_id_presentation['allowed']?> > <?php echo language('Allowed');?> </option>
						<option value="prohib_not_screen"      <?php echo $caller_id_presentation['prohib_not_screen']?> > <?php echo language('Prohibited,not screened');?> </option>
						<option value="prohib_passed_screen"   <?php echo $caller_id_presentation['prohib_passed_screen']?> > <?php echo language('Prohibited,passed screen');?> </option>
						<option value="prohib_failed_screen"   <?php echo $caller_id_presentation['prohib_failed_screen']?> > <?php echo language('Prohibited,failed screen');?> </option>
						<option value="prohib"                 <?php echo $caller_id_presentation['prohib']?> > <?php echo language('Prohibited');?> </option>
						<option value="unavailable"            <?php echo $caller_id_presentation['unavailable']?> > <?php echo language('Unavailable');?> </option>
					</select>
				</td>
			</tr>
		</table>
		<div id="newline"></div>
		<table width="98%" class="tedit" align="right">
			<tr>
				<td colspan = 2>
				<?php echo language('Maximum Channels');?>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Call Limit');?>:
						
					</div>
				</th>
				<td >
					<input  type="text" name="call_limit" value="<?php echo $call_limit?>" />
				</td>
			</tr>
		</table>
		<div id="newline"></div>


		<div id="tab">
			<li class="tb_indent">&nbsp;</li>
			<li class="tb_fold" onclick="lud(this,'tab_adv_sig_set')" id="tab_adv_sig_set_li">&nbsp;</li>
			<li class="tbg_fold" onclick="lud(this,'tab_adv_sig_set')"><?php echo language('Advanced:Signaling Settings');?></li>
			<li class="tb2_fold" onclick="lud(this,'tab_adv_sig_set')">&nbsp;</li>
			<li class="tb_end2">&nbsp;</li>
		</div>
		<div id="tab_adv_sig_set" style="display:none">
			<table width="96%" class="tedit" align="right">
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Progress Inband');?>:
							<span class="showhelp">
							<?php echo language('Progress Inband help',"
								If we should generate in-band ringing. <br/>
								Always use 'never' to never use in-band signalling, even in cases where some buggy devices might not render it.<br/>
								Valid values: yes, no, never. <br/>
								Default: never.");
							?>
							</span>
						</div>
					</th>
					<td >
						<select size=1 name="progress_inband" id="progress_inband">
							<option value="never" <?php echo $progress_inband['never']?> > <?php echo language('Never');?> </option>
							<option value="yes"   <?php echo $progress_inband['yes']?> > <?php echo language('_Yes');?> </option>
							<option value="no"    <?php echo $progress_inband['no']?> > <?php echo language('_No');?> </option>
						</select>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Append user');?>=<?php echo language('phone to URI');?>:
							<span class="showhelp">
							<?php echo language('phone to URI help',"Whether or not to add ';user=phone' to URIs that contain a valid phone number.");?>
							</span>
						</div>
					</th>
					<td >
						<select size=1 name="append_user_phone_to_uri" id="append_user_phone_to_uri">
							<option value="no"  <?php echo $append_user_phone_to_uri['no']?> > <?php echo language('_No');?> </option>
							<option value="yes" <?php echo $append_user_phone_to_uri['yes']?> > <?php echo language('_Yes');?> </option>
						</select>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Add Q.850 Reason Headers');?>:
							<span class="showhelp">
							<?php echo language('Add Q.850 Reason Headers help','Whether or not to add Reason header and to use it if it is available.');?>
							</span>
						</div>
					</th>
					<td >
						<select size=1 name="add_q850_reason_headers" id="add_q850_reason_headers">
							<option value="no" <?php echo $add_q850_reason_headers['no']?> > <?php echo language('_No');?> </option>
							<option value="yes" <?php echo $add_q850_reason_headers['yes']?> > <?php echo language('_Yes');?> </option>
						</select>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Ignor SDP Version');?>:
							<span class="showhelp">
							<?php echo language('Ignor SDP Version help','
								By default, the gateway will honor the session version number in SDP packets and will only modify the SDP session if the version number changes. <br/>
								Turn this option off to force the gateway to ignore the SDP session version number and treat all SDP data as new data. <br/>
								This is required for devices that send non-standard SDP packets (observed with Microsoft OCS). <br/>
								By default this option is on.');
							?>
							</span>
						</div>
					</th>
					<td >
						<select size=1 name="honor_sdp_version" id="honor_sdp_version">
							<option value="no"  <?php echo $honor_sdp_version['no']?> > <?php echo language('_No');?> </option>
							<option value="yes" <?php echo $honor_sdp_version['yes']?> > <?php echo language('_Yes');?> </option>
						</select>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Directmedia');?>:
							<span class="showhelp">
							<?php echo language('Directmedia help',"
								the value of parameter directmedia is one of them (no , yes, nonat, update, outgoing), default value is yes. <br/>directmedia=yes  'allow RTP media direct'<br/>directmedia = no 'deny re-invites'<br/>directmedia = nonat 'allow reinvite when local, deny reinvite when NAT'<br/> directmedia = update 'use UPDATE instead of INVITE'<br/>directmedia = update,nonat 'use UPDATE when local, deny when NAT'
								");
							?>
							</span>
						</div>
					</th>
					<td >
						<select size=1 name="directmedia" id="directmedia">
							<option value="yes" <?php echo $directmedia['yes']?> > <?php echo language('Yes');?> </option>
							<option value="nonat" <?php echo $directmedia['nonat']?> > <?php echo language('Nonat');?> </option>
							<option value="update" <?php echo $directmedia['update']?> > <?php echo language('Update');?> </option>
							<option value="outgoing" <?php echo $directmedia['outgoing']?> > <?php echo language('Outgoing');?> </option>
							<option value="no" <?php echo $directmedia['no']?> > <?php echo language('no');?> </option>
						</select>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Allow Transfers');?>:
							<span class="showhelp">
							<?php echo language('Allow Transfers help',"
								Whether or not to globally enable transfers. <br/>
								Choosing 'no' will disable all transfers (unless enabled in peers or users). <br/>
								Default is enabled.");
							?>
							</span>
						</div>
					</th>
					<td >
						<select size=1 name="allow_transfers" id="allow_transfers">
							<option value="no"  <?php echo $allow_transfers['no']?> > <?php echo language('_No');?> </option>
							<option value="yes" <?php echo $allow_transfers['yes']?> > <?php echo language('_Yes');?> </option>
						</select>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Allow Promiscuous Redirects');?>:
							<span class="showhelp">
							<?php echo language('Allow Promiscuous Redirects help','
								Whether or not to allow 302 or REDIR to non-local SIP address. <br/>
								Note that promiscredir when redirects are made to the local system will cause loops since this gateway is incapable of performing a "hairpin" call.');
							?>
							</span>
						</div>
					</th>
					<td>
						<select size=1 name="allow_promiscuous_redirects">
							<option value="no"  <?php echo $allow_promiscuous_redirects['no']?> > <?php echo language('_No');?> </option>
							<option value="yes" <?php echo $allow_promiscuous_redirects['yes']?> > <?php echo language('_Yes');?> </option>
						</select>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Max Forwards');?>:
							<span class="showhelp">
							<?php echo language('Max Forwards help','Setting for the SIP Max-Forwards header (loop prevention).');?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" name="max_forwards" value="<?php echo $max_forwards?>" />
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Send TRYING on REGISTER');?>:
							<span class="showhelp">
							<?php echo language('Send TRYING on REGISTER help','Send a 100 Trying when the endpoint registers.');?>
							</span>
						</div>
					</th>
					<td >
						<select size=1 name="send_trying_on_register" id="send_trying_on_register">
							<option value="no"  <?php echo $send_trying_on_register['no']?> > <?php echo language('_No');?> </option>
							<option value="yes" <?php echo $send_trying_on_register['yes']?> > <?php echo language('_Yes');?> </option>
						</select>
					</td>
				</tr>
			</table>
		</div>

		<div id="newline"></div>

		<div id="tab">
			<li class="tb_indent">&nbsp;</li>
			<li class="tb_fold" onclick="lud(this,'tab_adv_timer_set')" id="tab_adv_timer_set_li">&nbsp;</li>
			<li class="tbg_fold" onclick="lud(this,'tab_adv_timer_set')"><?php echo language('Advanced:Timer Settings');?></li>
			<li class="tb2_fold" onclick="lud(this,'tab_adv_timer_set')">&nbsp;</li>
			<li class="tb_end2">&nbsp;</li>
		</div>
		<div id="tab_adv_timer_set" style="display:none">
			<table width="96%" class="tedit" align="right">
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Default T1 Timer');?>:
							<span class="showhelp">
							<?php echo language('Default T1 Timer help',"
								This timer is used primarily in INVITE transactions. <br/>
								The default for Timer T1 is 500 ms or the measured run-trip time between the gateway and the device if you have qualify=yes for the device.");
							?>
							</span>
						</div>
					</th>
					<td >
						<input  type="text" name="default_t1_timer" value="<?php echo $default_t1_timer?>" />
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Call Setup Timer');?>:
							<span class="showhelp">
							<?php echo language('Call Setup Timer help','
								If a provisional response is not received in this amount of time, the call will autocongest. <br/>
								Defaults to 64 times the default T1 timer.');
							?>
							</span>
						</div>
					</th>
					<td >
						<input  type="text" name="call_setup_timer" value="<?php echo $call_setup_timer?>" />
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Session Timers');?>:
							<span class="showhelp">
							<?php echo language('Session Timers help','
								Session-Timers feature operates in the following three modes: originate, Request and run session-timers always;<br/>
								accept, Run session-timers only when requested by other UA; refuse, <br/>
								Do not run session timers in any case.');
							?>
							</span>
						</div>
					</th>
					<td >
						<select size=1 name="session_timers" id="session_timers">
							<option value="accept"    <?php echo $session_timers['accept']?> > <?php echo language('Accept');?> </option>
							<option value="originate" <?php echo $session_timers['originate']?> > <?php echo language('Originate');?> </option>
							<option value="refuse"     <?php echo $session_timers['refuse']?> > <?php echo language('Refuse');?> </option>
						</select>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Minimum Session Refresh Interval');?>:
							<span class="showhelp">
							<?php echo language('Minimum Session Refresh Interval help','Minimum session refresh interval in seconds. Defualts to 90 secs.');?>
							</span>
						</div>
					</th>
					<td >
						<input  type="text" name="minimum_session_refresh_interval" value="<?php echo $minimum_session_refresh_interval ?>" />
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Maximum Session Refresh Interval');?>:
							<span class="showhelp">
							<?php echo language('Maximum Session Refresh Interval help','Maximum session refresh interval in seconds. Defaults to 1800s.');?>
							</span>
						</div>
					</th>
					<td >
						<input  type="text" name="maximum_session_refresh_interval" value="<?php echo $maximum_session_refresh_interval?>" />
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Session Refresher');?>:
							<span class="showhelp">
							<?php echo language('Session Refresher help','The session refresher, uac or uas. Defaults to uas.');?>
							</span>
						</div>
					</th>
					<td >
						<select size=1 name="session_refresher" id="session_refresher">
							<option value="uas" <?php echo $session_refresher['uas']?> > UAS </option>
							<option value="uac" <?php echo $session_refresher['uac']?> > UAC </option>
						</select>
					</td>
				</tr>
			</table>
		</div>
	</div>
	<div id="newline"></div>
	<div id="tab" >
		<li class="tb_fold" onclick="lud(this,'save_to_other_channels')" id="save_to_other_channels_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'save_to_other_channels')"><?php echo language('Save To Other Sips');?></li>
		<li class="tb2_fold" onclick="lud(this,'save_to_other_channels')">&nbsp;</li>
		<li class="tb_end2">&nbsp;</li>
	</div>
	<div id="save_to_other_channels" style="display:none">
	<table width="98%" class="tedit" align="right">
		<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Save To Other Sips', 'Save To Other Sips');?>:
						<span class="showhelp">
						<?php echo language('GSM Gateway Save To Other Sips help', 'Saving current sip param to other sips synchronously');?>
						</span>
					</div>
				</th>
				<td>
					<table cellpadding="0" cellspacing="0" class="port_table">
				<?php
					$all_sips = get_all_sips();
					$sip_counts = count($all_sips);
					if(is_array($all_sips)){
						$j = 1;
						foreach($all_sips as $sip){
							$sip_num[$j++] = trim($sip['username']);	
						}
						for($line = 0; $line <= $sip_counts / 4; $line++){
							echo "<tr>";
							for($i = 1 + $line * 4; $i <= (4 + $line*4); $i++){
								echo "<td>";
								if(isset($sip_num[$i])){
									if($endpoint_name == $sip_num[$i]){
										echo "<input type=\"checkbox\" checked disabled>";
									} else {
										echo "<input type=\"checkbox\" class=\"port\" name=\"spans[$i]\" value=\"$sip_num[$i]\">";
									}
									echo "Sip-" . $sip_num[$i];
									echo "&nbsp;&nbsp;</td>";
								}
							}
							echo "</tr>";
						}
					}
				?>
					<tr>
						<td colspan=4>
							<input type="checkbox" id="all_port" onclick="selectAllCheckbox(this.checked,'class','port');handle_port_sync();">
							<?php echo language('All');?>
							<span id="cports"></span>
						</td>
					</tr>
					</table>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Sync All Settings', 'Sync All Settings');?>:
						<span class="showhelp">
						<?php echo language('AG Sync All Settings help', 'AG Sync All settings');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" id="all_settings_sync" onclick="selectAllCheckbox(this.checked,'class','setting_sync');" disabled />
					<?php echo language('Select all settings', 'Select all settings');?>
			</td>
		</tr>
	</table>
	</div>
	<div id="newline"></div>

	<br>

	<table>
		<tr>
			<td>
			<input type="hidden" name="send" id="send" value="" />
			<input type="submit" class="float_btn gen_short_btn"  value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>
			&nbsp;
			<input type="submit" class="float_btn gen_short_btn"  value="<?php echo language('Apply');?>" onclick="document.getElementById('send').value='Apply';return check();"/>
			&nbsp;
			<input type=button   value="<?php echo language('Cancel');?>" onclick="window.location.href='<?php echo get_self() ?>'" />
			</td>
		</tr>
	</table>

	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td style="width:50px">
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" />
			</td>
			<td style="width:50px">
				<input type="submit" id="float_button_3" class="float_short_button" value="<?php echo language('Apply');?>" onclick="document.getElementById('send').value='Apply';return check();" />
			</td>
			<td>
				<input type="button" id="float_button_2" class="float_short_button" value="<?php echo language('Cancel');?>" onclick="window.location.href='<?php echo get_self() ?>'" />
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
	
	//tls
	if(document.getElementById('transport').value == 'tls'){
		$("#tls_tlsverify").show();
		$("#tls_tlssetup").show();
		$("#tls_tlsprivatekey").show();
		$("#tls_encryption").show();
	}
	$("#transport").change(function(){
		if($(this).val() == 'tls'){
			$("#tls_tlsverify").show();
			$("#tls_tlssetup").show();
			$("#tls_tlsprivatekey").show();
			$("#tls_encryption").show();
			$("#port").val('5061');
		}else{
			$("#tls_tlsverify").hide();
			$("#tls_tlssetup").hide();
			$("#tls_tlsprivatekey").hide();
			$("#tls_encryption").hide();
			$("#port").val('5060');
		}
	});
	
	$("#show_sip_password").change(function(){
		if($(this).attr("checked") == 'checked'){
			$("#sip_password").prop("type","text");
		}else{
			$("#sip_password").prop("type","password");
		}
	});
</script>
<div id="sort_out" class="sort_out">
</div>
	<div class="sort_gsm_cli" id="sort_gsm_cli">
		<div id="sort_info" class="sort_info" style="display:block">
		<?php
	$all_sips = get_all_sips(true);
	$sips_num = 0;
	if($all_sips){
		foreach($all_sips as $sip) {
			$sips_num +=1;
			$credentials = $sip['username'];
			$sip['endpoint_name'] = htmlentities($sip['endpoint_name']);
			if($sip['endpoint_name'] == $endpoint_name){
?>
		<li><a style="color:#CD3278;" href="<?php echo get_self();?>?sel_endpoint_name=<?php echo $sip['endpoint_name']; ?>&type=sip" ><?php echo $sip['endpoint_name']; ?></a></li>
<?php
			}else{
?>
		<li><a style="color:LemonChiffon4;" href="<?php echo get_self();?>?sel_endpoint_name=<?php echo $sip['endpoint_name']; ?>&type=sip" ><?php echo $sip['endpoint_name']; ?></a></li>
<?php
			}
		}
	}
//Control the left navigation hidden or height by '$sips_num'.
	if($sips_num==0){
?>
<script type="text/javascript">
	$("#sort_out").hide();
	$("#sort_info").hide();
</script>
	<?php
	}elseif($sips_num <= 5){
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
	$("#registery_enable").iButton();
	$("#enableoutboundtohost").iButton();
});

</script>

<?php
} //end of add_sip_endpoint_page

?>

<?php
function add_iax2_endpoint_page($endpoint_name = NULL, $order = '')
{
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk');
	
	//Get Configs
	//////////////////////////////////////////////////////////////////////////////////
	if($endpoint_name) {   //Modify SIP not Add New SIP
		echo "<h4>";echo language('Edit IAX2 Endpoint');echo " \"$endpoint_name\" </h4>";
		$hlock = lock_file('/etc/asterisk/gw_iax_endpoints.conf');
		//$res = $aql->query("select * from gw_endpoints.conf where section='$endpoint_name'");
		//$res = $aql->query("select * from gw_iax_endpoints.conf");
		$res = $aql->query("select * from gw_iax_endpoints.conf where section='$endpoint_name'");
		unlock_file($hlock);
	} else {
		echo '<h4>';echo language('Add New IAX2 Endpoint');echo '</h4>';
	}

/* /etc/asterisk/gw_endpoints.conf */
// [endpoint_name]
// order = 
// username = 
// registration =  
// allow_anonymous = yes|no
// register =    like sip.conf [general]
// .......       like all sip.conf
	
	
	
	///////////////////////////////////////////
	$reg_port = '';
	if(isset($res[$endpoint_name]['register'])) {
		$register = trim($res[$endpoint_name]['register']);
		$reg = parse_register($register);		
	}
	///////////////////////////////////////////

	//Get Registration
	///////////////////////////////////////////
	$registration['none'] = '';
	$registration['client'] = '';
	$registration['server'] = '';
	if(isset($res[$endpoint_name]['registration'])) {
		$val = trim($res[$endpoint_name]['registration']);
		if(strcasecmp($val,'client') == 0) {
			$registration['client'] = 'selected';
		} else if(strcasecmp($val,'server') == 0) {
			$registration['server'] =  'selected';
		} else {
			$registration['none'] = 'selected';
		}
	} else {
		$registration['none'] = 'selected';
	}
	
	$auth['md5']='';
	$auth['plaintext']='';
	$auth['rsa']='';
	if(isset($res[$endpoint_name]['auth'])) {
		$val = trim($res[$endpoint_name]['auth']);	
		if(strcasecmp($val,'md5') == 0) {
			$auth['md5'] = 'selected';
		} else if(strcasecmp($val,'plaintext') == 0) {
			$auth['plaintext'] = 'selected';
		} else {
			$auth['rsa'] =  'selected';			
		}
	}
	
	
	$transfer['yes']='';
	$transfer['no']='';
	
	if(isset($res[$endpoint_name]['transfer'])) {
		$val = trim($res[$endpoint_name]['transfer']);	
		if(strcasecmp($val,'Yes') == 0) {
			$transfer['yes'] ='selected';
		} else {
			$transfer['no'] = 'selected';			
		}
	}else{
		$transfer['no'] = 'selected';			
	}
	
	$trunk['yes']='';
	$trunk['no']='';	
	if(isset($res[$endpoint_name]['trunk'])) {
		$val = trim($res[$endpoint_name]['trunk']);	
		if(strcasecmp($val,'Yes') == 0) {
			$trunk['yes'] ='selected';
		} else {
			$trunk['no'] = 'selected';			
		}
	}else{
		$trunk['no'] = 'selected';			
	}
	
	$qualify['yes']='';
	$qualify['no']='';	
	if(isset($res[$endpoint_name]['qualify'])) {
		$val = trim($res[$endpoint_name]['qualify']);	
		if(strcasecmp($val,'Yes') == 0) {
			$qualify['yes'] ='selected';
		} else {
			$qualify['no'] = 'selected';			
		}
	}
	
	
	$requirecalltoken['yes']='';
	$requirecalltoken['no']='';	
	$requirecalltoken['auto']='';	
	
	if(isset($res[$endpoint_name]['requirecalltoken'])) {
		$val = trim($res[$endpoint_name]['requirecalltoken']);	
		if(strcasecmp($val,'Yes') == 0) {
			$requirecalltoken['yes'] ='selected';
		}else if(strcasecmp($val,'auto') == 0) {
			$requirecalltoken['auto']='selected';					
		} else {
			$requirecalltoken['no'] = 'selected';			
		}
	}else{
		$requirecalltoken['yes'] ='selected';
	}
	
	
	$qualifysmoothing['yes']='';
	$qualifysmoothing['no']='';	
	if(isset($res[$endpoint_name]['qualifysmoothing'])) {
		$val = trim($res[$endpoint_name]['qualifysmoothing']);	
		if(strcasecmp($val,'Yes') == 0) {
			$qualifysmoothing['yes'] ='selected';
		} else {
			$qualifysmoothing['no'] = 'selected';			
		}
	}
	
	
	$encryption['yes']='';
	$encryption['no']='';	
	if(isset($res[$endpoint_name]['encryption'])) {
		$val = trim($res[$endpoint_name]['encryption']);	
		if(strcasecmp($val,'Yes') == 0) {
			$encryption['yes'] ='selected';
		} else {
			$encryption['no'] = 'selected';			
		}
	}else{
		$encryption['no'] = 'selected';			
	}
	
	$forceencryption['yes']='';
	$forceencryption['no']='';	
	if(isset($res[$endpoint_name]['forceencryption'])) {
		$val = trim($res[$endpoint_name]['forceencryption']);	
		if(strcasecmp($val,'Yes') == 0) {
			$forceencryption['yes'] ='selected';
		} else {
			$forceencryption['no'] = 'selected';			
		}
	}else{
		$forceencryption['no'] = 'selected';			
	}
	
	$trunktimestamps['yes']='';
	$trunktimestamps['no']='';	
	if(isset($res[$endpoint_name]['trunktimestamps'])) {
		$val = trim($res[$endpoint_name]['trunktimestamps']);	
		if(strcasecmp($val,'Yes') == 0) {
			$trunktimestamps['yes'] ='selected';
		} else {
			$trunktimestamps['no'] = 'selected';			
		}
	}
	

	///////////////////////////////////////////
	
	if(isset($res[$endpoint_name]['inkeys'])) {
		if($endpoint_name) {
			$inkeys = trim($res[$endpoint_name]['inkeys']);
		}
	}
	
	if(isset($res[$endpoint_name]['port'])) {
		if($endpoint_name) {
			$reg_port = trim($res[$endpoint_name]['port']);
		}
	}else{
		$reg_port=4569;
	}
	
	if(isset($res[$endpoint_name]['order'])) {
		if($endpoint_name) {
			$order = trim($res[$endpoint_name]['order']);
		}
	}

	if(isset($res[$endpoint_name]['username'])) {
		$iax_username = trim($res[$endpoint_name]['username']);
	} else {
		$iax_username = '';
	}

	if(isset($res[$endpoint_name]['secret'])) {
		$iax_password = trim($res[$endpoint_name]['secret']);
		$iax_password = htmlentities($iax_password);
	} else {
		$iax_password = '';
	}

	if(isset($res[$endpoint_name]['host'])) {
		$iax_ip = trim($res[$endpoint_name]['host']);
	} else {
		$iax_ip = '';
	}
	
	if(isset($res[$endpoint_name]['auth'])) {
		$iax_auth = trim($res[$endpoint_name]['auth']);
	} else {
		$iax_auth = '';
	}

	
	if(isset($res[$endpoint_name]['qualifyfreqok'])) {
		$qualifyfreqok = trim($res[$endpoint_name]['qualifyfreqok']);
	} else {
		$qualifyfreqok = '6000';
	}

	if(isset($res[$endpoint_name]['qualifyfreqnotok'])) {
		$qualifyfreqnotok = trim($res[$endpoint_name]['qualifyfreqnotok']);
	} else {
		$qualifyfreqnotok = '6000';
	}

	if(isset($res[$endpoint_name]['fromuser'])) {
		$from_user = trim($res[$endpoint_name]['fromuser']);
	} else {
		$from_user = '';
	}

	if(isset($res[$endpoint_name]['port'])) {
		$port = trim($res[$endpoint_name]['port']);
	} else {
		$port = '4569';
	}
	
	if(isset($res[$endpoint_name]['trunkmaxsize'])) {
		$trunkmaxsize = trim($res[$endpoint_name]['trunkmaxsize']);
	} else {
		$trunkmaxsize = '128000';
	}
	
	if(isset($res[$endpoint_name]['trunkmtu'])) {
		$trunkmtu = trim($res[$endpoint_name]['trunkmtu']);
	} else {
		$trunkmtu = '0';
	}
	
	if(isset($res[$endpoint_name]['trunkfreq'])) {
		$trunkfreq = trim($res[$endpoint_name]['trunkfreq']);
	} else {
		$trunkfreq = '20';
	}
	

	
	if(isset($res[$endpoint_name]['minregexpire'])) {
		$minregexpire = trim($res[$endpoint_name]['minregexpire']);
	} else {
		$minregexpire = '60';
	}
	
	if(isset($res[$endpoint_name]['maxregexpire'])) {
		$maxregexpire = trim($res[$endpoint_name]['maxregexpire']);
	} else {
		$maxregexpire = '60';
	}
	
	

///////////////////////////////////////////////////////////////////////////////////	
?>

<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/float_btn.js"></script>

<script type="text/javascript">


var sip_ip_clear = false;

function onload_func_iax()
{
	registration_change(1);
	auth_change();
}


function auth_change(){
	value = document.getElementById('auth').value;
	if(value=='rsa'){
		
		document.getElementById('div_rsa').style.display="inline";
	}else{
		document.getElementById('rsa_value').value="";
		document.getElementById('div_rsa').style.display="none";
		
	}
	
}

function registration_change(load)
{
	value = document.getElementById('registration').value;
	
	if(value == 'client') {
		//document.getElementById('authentication_user').disabled = false;
		//document.getElementById('remote_secret').disabled = false;		
		//document.getElementById('register_extension').disabled = false;

		document.getElementById('port').disabled = false;
		document.getElementById('iax_ip').readOnly = false;
		if(sip_ip_clear)
			document.getElementById('iax_ip').value = '';

		//document.getElementById('anonymous').checked = false;
		//document.getElementById('anonymous').disabled = true;
		//anonymous_click(false);
		if(load == 0)
			document.getElementById('qualify').value = 'yes';
	} else {
		//document.getElementById('authentication_user').disabled = true;
		//document.getElementById('remote_secret').disabled = true;
		//document.getElementById('register_extension').disabled = true;

		if(value == 'none') {
			document.getElementById('port').disabled = false;
			document.getElementById('iax_ip').readOnly = false;
			if(sip_ip_clear)
				document.getElementById('iax_ip').value = '';

			//document.getElementById('anonymous').disabled = false;
			
			if(load == 0)
				document.getElementById('qualify').value = 'yes';
		} else if(value == 'server') {
			document.getElementById('port').disabled = true;
			document.getElementById('iax_ip').value = 'dynamic';
			document.getElementById('iax_ip').readOnly = true;
			sip_ip_clear = true;

			if(load == 0)
				document.getElementById('qualify').value = 'yes';
			//anonymous_click(false);
		}
	}
}

function iax_username_change(obj)
{
	/*(if(!document.getElementById('from_user_readonly').checked) {
		document.getElementById('from_user').value = obj.value;
	}*/

	//if(!document.getElementById('register_extension_readonly').checked) {
	//	document.getElementById('register_extension').value = obj.value;
	//}
}



function check_iax()
{
	//exit;
<?php
	//$name_ip = $sip_username .'@'. $sip_ip;
	$asips = get_all_iaxs();
	$name_ary = '';
	$ip_ary = '';
	$nameandip_ary = '';
	if($asips) {
		foreach($asips as $sip) {		
			if(strcmp($sip['endpoint_name'],$endpoint_name)==0)
				continue;
			$name_ary .=  '"'.$sip['endpoint_name'].'"'.',';
			$ip_ary .= '"'.$sip['host'].'"'.',';
			$nameandip_ary .= '"'.$sip['username'].'@'.$sip['host'].'"'.',';
		}
	}
	$name_ary = rtrim($name_ary,',');
	$ip_ary = rtrim($ip_ary,',');
	$nameandip_ary = rtrim($nameandip_ary,',');
?>
	var name_ary = new Array(<?php echo $name_ary; ?>);
	var ip_ary = new Array(<?php echo $ip_ary; ?>);
	var nameandip_ary = new Array(<?php echo $nameandip_ary; ?>);

	
	
	var endpoint_name = document.getElementById('endpoint_name').value;
	var iax_username = document.getElementById('iax_username').value;
	var iax_password = document.getElementById('iax_password').value;
	var iax_ip = document.getElementById('iax_ip').value;
	//var anonymous = document.getElementById('anonymous').checked;

	document.getElementById('cendpoint_name').innerHTML = '';
	document.getElementById('ciax_username').innerHTML = '';
	document.getElementById('ciax_ip').innerHTML = '';
	document.getElementById('cport').innerHTML = '';

	
	
	
	if(!check_sipendp(endpoint_name)) {
		document.getElementById("cendpoint_name").innerHTML = con_str('<?php echo htmlentities(language('js check sipendp','Allowed character must be any of [0-9a-zA-Z`~!@#$%^*()_{}:|?-=.], 1-32 characters.'));?>');
		return false;
	}else{
		document.getElementById("cendpoint_name").innerHTML = '';
	}
	
	if(!check_sipname(iax_username)) {
			document.getElementById("ciax_username").innerHTML = con_str('<?php echo language('js check sipname','Allowed character must be any of [0-9a-zA-Z$*()-=_.], length: 1-32');?>');
			return false;
		}else{
			document.getElementById("ciax_username").innerHTML = '';
		}
	if(iax_password != '' && !check_sippwd(iax_password)) {
		document.getElementById("ciax_password").innerHTML = con_str('<?php echo htmlentities(language('js check sippwd','Allowed character must be any of [0-9a-zA-Z`~!#$%^&*()_+{}|<>-=,.], 0-32 characters.'));?>');
		return false;
	}else{
		document.getElementById("ciax_password").innerHTML = '';
	}

	
	var registration = document.getElementById('registration').value;
	if(registration != 'server') {
		if(!check_domain(iax_ip)) {
			document.getElementById("ciax_ip").innerHTML = con_str('<?php echo language('js check domain','Invalid domain or IP address.');?>');			
			return false;
		}
	}

	for (var i in name_ary) 
	{
		if(name_ary[i] == endpoint_name) {
			document.getElementById('cendpoint_name').innerHTML = con_str('You already had a same username.');
			return false;
		}
	}

	for (var i in nameandip_ary) 
	{
		if(nameandip_ary[i] == (iax_username+'@'+iax_ip)) {
			document.getElementById('ciax_username').innerHTML = con_str('You already had a same '+nameandip_ary[i]+'.');
			return false;
		}

		if(nameandip_ary[i] == ('anonymous@'+iax_ip)) {
			document.getElementById('ciax_ip').innerHTML = con_str('You exist has '+iax_ip+' set to anonymous.');
			return false;
		}
	}


	
	if(registration != 'server') {
		var port = document.getElementById('port').value;
		if(port != '') {
			if(!check_networkport(port)) {
				document.getElementById("cport").innerHTML = con_str('<?php echo language('js check networkport','Please input valid port number (1-65535)');?>');
				return false;
			}
		}
	}

	return true;
}



</script>
    
	<form name="add_iax_endpoint" id="add_iax_endpoint" enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<input type="hidden" id="order" name="order" value="<?php echo $order?>" />
	<input type="hidden" id="old_endpoint_name" name="old_endpoint_name" value="<?php echo htmlentities($endpoint_name);?>" />

	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_main')" id="tab_main_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_main')"><?php echo language('Main Endpoint Settings');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_main')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>
	<div id="tab_main" style="display:block">
		<table width="98%" class="tedit" align="right">
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Name');?>:
						<span class="showhelp">
						<?php echo language('Name help@sip-endpoints',"
							A name which is able to read by human. <br/>
							And it's only used for user's referance.");
						?>
						</span>
					</div>
				</th>
				<td >
					<input  type="text" name="endpoint_name" id="endpoint_name" value="<?php echo htmlentities($endpoint_name);?>" /> <span id="cendpoint_name"></span>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('User Name');?>:
						<span class="showhelp">
						<?php echo language('User Name help@ip-endpoints','User Name the endpoint will use to authenticate with the gateway.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="iax_username" id="iax_username" value="<?php echo $iax_username?>" onchange="iax_username_change(this)" />				
					<span id="ciax_username"></span>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Password');?>:
						<span class="showhelp">
						<?php echo language('Password help@ip-endpoints','
							Password the endpoint will use to authenticate with the gateway.<br/>
							Allowed characters');
						?>
						</span>
					</div>
				</th>
				<td >
					<input  type="password" name="iax_password" id="iax_password" value="<?php echo $iax_password?>" />
					<span style="vertical-align:middle;display:inline-block;"><input type="checkbox" id="show_iax_password" /></span>
					<span id="ciax_password"></span>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Registration');?>:
						<span class="showhelp">
						<?php echo language('Registration help@sip-endpoints','Whether this endpoint will register to this gateway or this gateway to the endpoint.');?>
						</span>
					</div>
				</th>
				<td>
					<select size=1 name="registration" id="registration" onchange="registration_change(0)">					
						<option value= "none" <?php echo $registration['none']?>> <?php echo language('None');?> </option>
						<option value= "server" <?php echo $registration['server']?>> <?php echo language('Server');?> </option>
						<option value= "client" <?php echo $registration['client']?>> <?php echo language('Client');?> </option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Hostname or IP Address');?>:
						<span class="showhelp">
						<?php echo language('Hostname or IP Address help',"
							IP address or hostname of the endpoint or 'dynamic' if the endpoint has a dynamic IP address.<br/>
							This will require registration.<br>
							Notice: If the input here is hostname and your DNS has changed, you must <span style=\"color:rgb(255,0,0)\">reboot asterisk</span>.");
						?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" id="iax_ip" name="iax_ip" value="<?php echo $iax_ip?>" /><span id="ciax_ip"></span>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Auth');?>:
						<span class="showhelp">
						<?php echo language('Authentication method for connections');?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="auth" id="auth" onchange="auth_change();">
						<option value= "md5" <?php echo $auth['md5']?>> <?php echo language('md5');?> </option>
						<option value= "plaintext" <?php echo $auth['plaintext']?>> <?php echo language('plaintext');?> </option>
						<option value= "rsa" <?php echo $auth['rsa']?>> <?php echo language('rsa');?> </option>						
					</select>
					<div id="div_rsa" style="display:inline;" class="helptooltips"><?php echo language('inkeys');?>
					<span class="showhelp">
						<?php echo language('"inkeys" is a list of acceptable public keys on the  local system');
						?>
						</span>
						<input name="rsa_value" id="rsa_value" value="<?php echo $inkeys; ?>">
					</div>
				</td>
			</tr>
			<!--
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Transport');?>:
						<span class="showhelp">
						<?php echo language('Transport help','
							This sets the possible transport types for outgoing. Order of usage, <br/>
							when the respective transport protocols are inabled, is UDP, TCP, TLS.<br/>
							The first enabled transport type is only used for outbound messages until a<br/> 
							Registration takes place. During the peer Registration the transport type <br/>
							may change to another supported type if the peer requests so.');
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="transport">
						<option value="udp"  <?php echo $transport['udp']?> > UDP </option>						
					</select>
				</td>
			</tr>
			-->
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Transfer');?>:
						<span class="showhelp">
						<?php echo language('Disable or not IAX2 native transfer');
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="transfer">
						<option value="yes" <?php echo $transfer['yes']?>> Yes </option>						
						<option value="no" <?php echo $transfer['no']?>> No </option>						
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Trunk');?>:
						<span class="showhelp">
						<?php echo language('Use IAX2 trunking with this host');
						?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="trunk">
					<option value="yes" <?php echo $trunk['yes']?>> Yes </option>						
						<option value="no" <?php echo $trunk['no']?>> No </option>						
					</select>
				</td>
			</tr>
			
		</table>
		</div>
		<div id="newline"></div>
		<div id="tab">
			<!--<li class="tb_indent">&nbsp;</li>-->
			<li class="tb_fold" onclick="lud(this,'tab_adv_reg')" id="tab_adv_reg_li">&nbsp;</li>
			<li class="tbg_fold" onclick="lud(this,'tab_adv_reg')"><?php echo language('Advanced:Registration Options');?></li>
			<li class="tb2_fold" onclick="lud(this,'tab_adv_reg')">&nbsp;</li>
			<li class="tb_end2">&nbsp;</li>
		</div>
		<div id="tab_adv_reg" style="display:none">
			<table width="96%" class="tedit" align="right" >
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Qualify');?>:
							<span class="showhelp">
							<?php echo language('Qualify help@sip-endpoints',"Whether or not to check the endpoint's connection status.");?>
							</span>
						</div>
					</th>
					<td >
						<select size=1 name="qualify" id="qualify">
							<option value="yes" <?php echo $qualify['yes']?> > <?php echo language('_Yes');?> </option>
							<option value="no"  <?php echo $qualify['no']?> > <?php echo language('_No');?> </option>
							
						</select>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Qualify Smothing');?>:
							<span class="showhelp">
							<?php echo language('Use an average of the last two PONG results to reduce falsely detected LAGGED hosts. The default is no.');?>
							</span>
						</div>
					</th>
					<td >
						<select size=1 name="qualifysmoothing" id="qualifysmoothing">
							<option value="yes" <?php echo $qualifysmoothing['yes']?> > <?php echo language('_Yes');?> </option>
							<option value="no"  <?php echo $qualifysmoothing['no']?> > <?php echo language('_No');?> </option>							
						</select>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Qualify Freq Ok');?>:
							<span class="showhelp">
							<?php echo language('How frequently to ping the peer when everything seems to be OK, in milliseconds.');?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" name="qualifyfreqok" value="<?php echo $qualifyfreqok?>" />
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Qualify Freq Not Ok');?>:
							<span class="showhelp">
							<?php echo language('How frequently to ping the peer when it is either, LAGGED or UNAVAILABLE, in milliseconds.');?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" name="qualifyfreqnotok" value="<?php echo $qualifyfreqnotok?>" />
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Port');?>:
							<span class="showhelp">
							<?php echo language('The port number the gateway will connect to at this endpoint.');?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" id="port" name="port" value="<?php echo $port?>" /><span id="cport"></span>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Require Call Token');?>:
							
						</div>
					</th>
					<td >
						<select size=1 name="requirecalltoken" id="requirecalltoken">
							<option value="yes" <?php echo $requirecalltoken['yes']?> > <?php echo language('_Yes');?> </option>
							<option value="no"  <?php echo $requirecalltoken['no']?> > <?php echo language('_No');?> </option>
							<option value="auto"  <?php echo $requirecalltoken['auto']?> > <?php echo language('Auto');?> </option>
							
						</select>
					</td>
				</tr>
			
				
				
			</table>
			
			
		</div>
	

	<div id="newline"></div>
	
	<div id="tab">
			<!--<li class="tb_indent">&nbsp;</li>-->
			<li class="tb_fold" onclick="lud(this,'tab_Encryption')" id="tab_Encryption_li">&nbsp;</li>
			<li class="tbg_fold" onclick="lud(this,'tab_Encryption')"><?php echo language('IAX2 Encryption');?></li>
			<li class="tb2_fold" onclick="lud(this,'tab_Encryption')">&nbsp;</li>
			<li class="tb_end2">&nbsp;</li>
		</div>
		<div id="tab_Encryption" style="display:none">
		<table width="96%" class="tedit" align="right" >
		<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Encryption');?>:								<span class="showhelp">
							<?php echo language('Enable IAX2 encryption. The default is no.');?>
							</span>
						</div>
					</th>
					<td >
						<select size=1 name="encryption" id="encryption">
							<option value="yes" <?php echo $encryption['yes']?> > <?php echo language('_Yes');?> </option>
							<option value="no"  <?php echo $encryption['no']?> > <?php echo language('_No');?> </option>
							
						</select>
					</td>
				</tr>
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Force Encryption');?>:								<span class="showhelp">
							<?php echo language('Force encryption insures no connection is established unless both sides support encryption. By turning this option on, encryption is automatically turned on as well. The default is no.');?>
							</span>
						</div>
					</th>
					<td >
						<select size=1 name="forceencryption" id="forceencryption">
							<option value="yes" <?php echo $encryption['yes']?> > <?php echo language('_Yes');?> </option>
							<option value="no"  <?php echo $encryption['no']?> > <?php echo language('_No');?> </option>
							
						</select>
					</td>
				</tr>
				
		</table>
		
		
		</div>
	
	
	<div id="newline"></div>
	<div id="tab">
		<li class="tb_fold" onclick="lud(this,'tab_call_set')" id="tab_call_set_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_call_set')"><?php echo language('IAX2 Trunk settings');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_call_set')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>

	<div id="tab_call_set" style="display:none">
		<table width="98%" class="tedit" align="right">
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Trunk Max Size');?>:
						<span class="showhelp">
						<?php echo language('defaults to 128000 bytes, which supports up to 800 calls of ulaw at 20ms a frame.');?>
						</span>
					</div>
				</th>
				<td>					
					<input type="text" id="trunkmaxsize" name="trunkmaxsize" value="<?php echo $trunkmaxsize ?>" /><span id="ctrunkmaxsize"></span>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Trunk MTU');?>:
						<span class="showhelp">
						<?php echo language('sets the maximum transmission unit for IAX2 UDP trunking.');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" id="trunkmtu" name="trunkmtu" value="<?php echo $trunkmtu ?>" /><span id="ctrunkmtu "></span>
					
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Trunk Frequency');?>:
						<span class="showhelp">
						<?php echo language('How frequently to send trunk msgs in msec. This is 20ms by default.');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" id="trunkfreq" name="trunkfreq" value="<?php echo $trunkfreq ?>" /><span id="ctrunkfreq "></span>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Trunk Time Stamps');?>:	<span class="showhelp">
						<?php echo language('ensure that frame timestamps get sent end-to-end properly.');?>
						</span>
					</div>
				</th>
				<td >
					<select size=1 name="trunktimestamps" id="trunktimestamps">
						<option value="no"  <?php echo $trunktimestamps['no']?> > <?php echo language('_No');?> </option>
						<option value="yes" <?php echo $trunktimestamps['yes']?> > <?php echo language('_Yes');?> </option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Min. RegExpire');?>:
						<span class="showhelp">
						<?php echo language('Minimum amounts of time that IAX2 peers can request as a registration expiration interval in seconds.');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" id="minregexpire" name="minregexpire" value="<?php echo $minregexpire ?>" /><span id="cminregexpire "></span>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Max. RegExpire');?>:
						<span class="showhelp">
						<?php echo language('Maximum amounts of time that IAX2 peers can request as a registration expiration interval in seconds.');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" id="maxregexpire" name="maxregexpire" value="<?php echo $maxregexpire ?>" /><span id="cmaxregexpire "></span>
					
				</td>
			</tr>
		</table>
		<div id="newline"></div>
	</div>
	<br>
	.
	<table>
		<tr>
			<td>
			<input type="hidden" name="send" id="send" value="" />
			<!--<input type="button" value="<?php echo language('Check');?>" onclick="return check_iax();"/>-->
			<input type="submit" class="float_btn gen_short_btn"  value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save IAX2';return check_iax();"/>
			&nbsp;
			<input type="submit" class="float_btn gen_short_btn"  value="<?php echo language('Apply');?>" onclick="document.getElementById('send').value='Apply IAX2';return check_iax();"/>
			&nbsp;
			<input type=button   value="<?php echo language('Cancel');?>" onclick="window.location.href='<?php echo get_self() ?>'" />
			</td>
		</tr>
	</table>
	
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td style="width:50px">
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save IAX2';return check_iax();" />
			</td>
			<td style="width:50px">
				<input type="submit" id="float_button_3" class="float_short_button" value="<?php echo language('Apply');?>" onclick="document.getElementById('send').value='Apply IAX2';return check_iax();" />
			</td>
			<td>
				<input type="button" id="float_button_2" class="float_short_button" value="<?php echo language('Cancel');?>" onclick="window.location.href='<?php echo get_self() ?>'" />
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
	
	$("#show_iax_password").change(function(){
		if($(this).attr("checked") == 'checked'){
			$("#iax_password").prop("type","text");
		}else{
			$("#iax_password").prop("type","password");
		}
	});
</script>
<div id="sort_out" class="sort_out">
</div>
	<div class="sort_gsm_cli" id="sort_gsm_cli">
		<div id="sort_info" class="sort_info" style="display:block">
		<?php
	$all_sips = get_all_iaxs(true);
	$sips_num = 0;
	if($all_sips){
		foreach($all_sips as $sip) {
			$sips_num +=1;
			$credentials = $sip['username'];
			$sip['endpoint_name'] = htmlentities($sip['endpoint_name']);
			if($sip['endpoint_name'] == $endpoint_name){
?>
		<li><a style="color:#CD3278;" href="<?php echo get_self();?>?sel_endpoint_name=<?php echo $sip['endpoint_name']; ?>&type=iax2" ><?php echo $sip['endpoint_name']; ?></a></li>
<?php
			}else{
?>
		<li><a style="color:LemonChiffon4;" href="<?php echo get_self();?>?sel_endpoint_name=<?php echo $sip['endpoint_name']; ?>&type=iax2" ><?php echo $sip['endpoint_name']; ?></a></li>
<?php
			}
		}
	}
//Control the left navigation hidden or height by '$sips_num'.
	if($sips_num==0){
?>
<script type="text/javascript">
	$("#sort_out").hide();
	$("#sort_info").hide();
</script>
	<?php
	}elseif($sips_num <= 5){
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
	onload_func_iax();
});

</script>

<?php
} //end of add_iax2_endpoint_page
?>

<?php
$check_float = 0;
	if($_POST) {
		if( (isset($_POST['send']) && ($_POST['send'] == 'Add New SIP Endpoint') ) ) {
			//Add new sip endpoint
			if( isset($_POST['sel_endpoint_name']) && isset($_POST['order']) && $_POST['order'] ) {
				$check_float = 1;
				add_sip_endpoint_page($_POST['sel_endpoint_name'],$_POST['order']);
			}
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Add New IAX2 Endpoint') {	
			if( isset($_POST['sel_endpoint_name']) && isset($_POST['order']) && $_POST['order'] ) {				
				$check_float = 1;
				add_iax2_endpoint_page($_POST['sel_endpoint_name'],$_POST['order']);
			}
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Save') {
			save_sip_endpoints();
			show_sip_endpoints();
			show_iax2_endpoints();
			//ast_reload();
			wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Save IAX2') {
			save_iax_endpoints();
			show_sip_endpoints();
			show_iax2_endpoints();
			//ast_reload();
			wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Apply IAX2') {
			$check_float = 1;
			save_iax_endpoints();
			add_iax2_endpoint_page($_POST['endpoint_name'],'');
			//ast_reload();
			wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");
		
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Apply') {
			$check_float = 1;
			save_sip_endpoints();
			add_sip_endpoint_page($_POST['endpoint_name'],'');
			//ast_reload();
			wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");
		} elseif (isset($_POST['send']) && ($_POST['send'] == 'Delete SIP' || $_POST['send'] == 'Delete')) {
			if(isset($_POST['sel_endpoint_name']) && ($_POST['sel_endpoint_name'] != '')) {
				del_sip_endpoint($_POST['sel_endpoint_name']);
				show_sip_endpoints();
				show_iax2_endpoints();
				//ast_reload();
				wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");
			} else {
				if (isset($_POST['log'])) {
					
					$sipusers = "";
					foreach ($_POST['log'] as $id) {
						$sipusers .= "$id,";
					}
					if ($sipusers != "") {
						del_sip_endpoint($sipusers);
						show_sip_endpoints();
						show_iax2_endpoints();
						//ast_reload();
						wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");
					}
				} else {
					show_sip_endpoints();
					show_iax2_endpoints();
				}
			}
		} elseif (isset($_POST['send']) && ($_POST['send'] == 'Delete IAX2' || $_POST['send'] == 'Delete IAX')) {			
			if(isset($_POST['sel_endpoint_name']) && ($_POST['sel_endpoint_name'] != '')) {
				del_iax_endpoint($_POST['sel_endpoint_name']);
				show_sip_endpoints();
				show_iax2_endpoints();
				//ast_reload();
				wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");
			} else {
				if(isset($_POST['iax'])) {
					$iaxusers = "";
					foreach ($_POST['iax'] as $iax_id){
						$iaxusers .= "$iax_id,";
					}
					if(!empty($iaxusers)){
						del_iax_endpoint($iaxusers);
						show_sip_endpoints();
						show_iax2_endpoints();
						//ast_reload();
						wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");						
					}
				} else {
					show_sip_endpoints();
					show_iax2_endpoints();
				}
			}
		}
	} else if($_GET) {
		//Modify sip endpoint
		if (isset($_GET['sel_endpoint_name']) && ($_GET['type'] == "sip")) {
			$check_float = 1;
			add_sip_endpoint_page($_GET['sel_endpoint_name'],'');
		} else if (isset($_GET['sel_endpoint_name']) && ($_GET['type'] == "iax2")) {
			$check_float = 1;
			add_iax2_endpoint_page($_GET['sel_endpoint_name'],'');
		}
	} else {
		show_sip_endpoints();
		show_iax2_endpoints();
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
