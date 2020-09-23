<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");

//AQL
require_once("/www/cgi-bin/inc/aql.php");
//require_once("/www/cgi-bin/inc/initreadwrite.php");
?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>

<script type="text/javascript">
function selectall(checked, ports)
{
<?php
	global $__GSM_SUM__;

	$cluster_info = get_cluster_info();	
	
	if(($cluster_info['mode'] == 'master') || ($cluster_info['mode'] != 'master')) {
			for($i = 1; $i <= $__GSM_SUM__; $i++){
					echo "document.getElementById('channel1".$i."').checked = checked;\n";
			}
?>
		//for (i = 1; i <= ports; i++) {
		//	document.getElementById('channel1'+i).checked = checked;
		//}
<?php
	}
	global $__GSM_SUM__;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	//added by wangxuechuan at 20131115
	if($__deal_cluster__){
		if($cluster_info['mode'] == 'master') {
			//update_cluster_board_num($cluster_info['devicemode']);
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
				if($slaveip != '') {
					for ($i = 1 ;$i <= $__GSM_SUM__; $i++){
								//channel type is FXS
	?>
							document.getElementById('channel'+'<?php echo $b . $i?>').checked = checked;
	<?php
					}
				}
			}
		}
	}
	
?>
}

function setValue(ports)
{
<?php
	$name_ary = '';

	$asips = get_all_sips();

	if($asips) {
		foreach($asips as $sip) {		
			$name_ary .=  '"'.$sip['endpoint_name'].'"'.',';
		}
	}
	$name_ary = rtrim($name_ary,',');
?>
	var name_ary = new Array(<?php echo $name_ary; ?>);

	username = document.getElementById('username10').value;
	passwd = document.getElementById('passwd10').value;
	ipaddr = document.getElementById('ipaddr10').value;
	//bipaddr = document.getElementById('bipaddr10').value;
	//codec = document.getElementById('sip_codec_priority10').value;
	//codec_mode=document.getElementById('sip_codec_mode10').value;
	//vosenc = document.getElementById('vosenc10').value;
	port = document.getElementById('port10').value;
	register_mode = document.getElementById('register_mode10').value;
	sip_overwrite = "<?php echo language('sip overwrite confirm');?>";
	sips_overwrite = "<?php echo language('sips overwrite confirm');?>";
	password_auto = document.getElementById('password_auto').checked;
	var outboundproxy = document.getElementById('outboundproxy0').value;
	var outboundproxy_port = document.getElementById('outboundproxy_port0').value;

	if (username == '') {
		alert("<?php echo language('js check username 1', '\'User Name\' must not be null!');?>");
		document.getElementById('username10').focus();
		return false;
	} else {
		if(!check_sipname(username)) {
			alert("<?php echo language('js check sipname', 'Allowed character must be any of [0-9a-zA-Z$*-=_.], length: 1-32');?>");
			document.getElementById('username10').focus();
			return false;
		}
	}
	if (passwd != '') {
		if(!check_sippwd(passwd)) {
			alert("<?php echo language('js check sippwd', 'Allowed character must be any of [0-9a-zA-Z`~!#@$%^&*()_+{}|<>-=[],.],1 - 32 characters.');?>");
			document.getElementById('passwd10').focus();
			return false;
		}
	}
	if (ipaddr != '') {
		if(!check_domain(ipaddr)) {
			alert("<?php echo language('js check domain', 'Invalid hostname or ip address!');?>");
			document.getElementById('ipaddr10').focus();
			return false;
		}
	}
	
	var part = username.split('_');
	if (part.length == 0 || part.length == 1) {
		newuser = Number(username);
	} else if (part.length == 2) {
		newuser = Number(part[1]);
	} else {
		alert("<?php echo language('js check sipname', 'Allowed character must be any of [0-9a-zA-Z$*-=_.], length: 1-32');?>");
		document.getElementById('username10').focus();
		return false;
	}
	var newuser_as_string;
	var osips = new Array();

	validusers = 0;

<?php
	$cluster_info = get_cluster_info();	
	if(($cluster_info['mode'] == 'master') || ($cluster_info['mode'] != 'master')) {
?>
		for (i = 1; i <= ports; i++) {
			if (document.getElementById('channel1'+i).checked == true) {
				validusers++;
			}
		}
<?php
	}
	
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	if($__deal_cluster__){
		if($cluster_info['mode'] == 'master') {
			//update_cluster_board_num($cluster_info['devicemode']);
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
				if($slaveip != '') {
	?>				
					for (i = 1; i <= ports; i++) {
						if (document.getElementById('channel'+<?php echo $b?>+i).checked == true) {
							validusers++;
						}
					}
	<?php
				}
			}
		}
	}
?>	
	for (i = 1; i <= validusers; i++) {
		if (part.length == 0 || part.length == 1) {
			newuser_as_string = newuser + "";
		} else {
			newuser_as_string = part[0] + "_" + newuser + "";
		}
		for (var j in name_ary) {
			if(name_ary[j] == newuser_as_string) {
				osips.push(name_ary[j]);
			}
		}
		newuser = newuser + 1; 
	}
	
	if (osips.length == 1) {
		confirm_string = osips[0] + " " + sip_overwrite;
		if (!confirm(confirm_string)) {
			return false;
		}
	} else if (osips.length > 1) {
		confirm_string = "";
		for (i = 0; i < osips.length; i++) {
			confirm_string += osips[i] + " ";
		}
		confirm_string += sips_overwrite;
		if (!confirm(confirm_string)) {
			return false;
		}
	}
	
	/*var part = username.split('_');
	if (part.length == 0 || part.length == 1) {
		newuser = Number(username);
	} else {
		newuser = Number(part[1]);
	}*/

	var usernamearray = username.match(/\d+/g);
	//alert(usernamearray);
	if(usernamearray){
		   var usernamestr = usernamearray[usernamearray.length-1];
		   var newusername = Number(usernamestr);
	}else{
		alert("<?php echo language('js check busername', 'you need to set a number for username');?>");
		return ;
	}
	
	//newpasswd = Number(passwd);
	var numarray = passwd.match(/\d+/g);	
	if(numarray){	
		var numstr = numarray[numarray.length-1];
		var newpasswd = Number(numstr);
	}
	
<?php
	if(($cluster_info['mode'] == 'master') || ($cluster_info['mode'] != 'master')) {
?>			
		for (i = 1; i <= ports; i++) {
			if (document.getElementById('channel1'+i).checked == true) {
				/*if (part.length == 0 || part.length == 1) {
					document.getElementById('username1'+i).value = newuser;
				} else {
					document.getElementById('username1'+i).value = part[0] + "_" + newuser;
				}*/
				
				if(password_auto) {
					//document.getElementById('passwd1'+i).value = newpasswd;
					var tmppasswd = passwd;
					document.getElementById('passwd1'+i).value = tmppasswd.replace(numstr,newpasswd+'');

					var tmpusername = username;
					document.getElementById('username1'+i).value = tmpusername.replace(usernamestr,newusername+'');						
				} else {
					document.getElementById('passwd1'+i).value = passwd;

					var tmpusername = username;
					document.getElementById('username1'+i).value = tmpusername.replace(usernamestr,newusername+'');
				}
				document.getElementById('ipaddr1'+i).value = ipaddr;
			//	document.getElementById('bipaddr1'+i).value = bipaddr;
			//	document.getElementById('sip_codec_priority1'+i).value = codec;
			//	document.getElementById('sip_codec_mode1'+i).value = codec_mode;
				document.getElementById('port1'+i).value = port;
				document.getElementById('outboundproxy'+i).value = outboundproxy;
				document.getElementById('outboundproxy_port'+i).value = outboundproxy_port;
				document.getElementById('register_mode1'+i).value = register_mode;
			//	document.getElementById('vosenc1'+i).value = vosenc;
				//newuser = newuser + 1; 
				newpasswd = newpasswd + 1;
				newusername = newusername + 1;	
			} else {
				document.getElementById('username1'+i).value = '';
				document.getElementById('passwd1'+i).value = '';
				document.getElementById('ipaddr1'+i).value = '';
			//	document.getElementById('bipaddr1'+i).value = '';
			//	document.getElementById('sip_codec_priority1'+i).value = 'ulaw';
				document.getElementById('port1'+i).value = '';
				document.getElementById('outboundproxy'+i).value = '';
				document.getElementById('outboundproxy_port'+i).value = '';
				document.getElementById('register_mode1'+i).value = '';
			//	document.getElementById('vosenc1'+i).value = 'No';
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
					for (i = 1; i <= ports; i++) {
						if (document.getElementById('channel'+<?php echo $b?>+i).checked == true) {
							/*if (part.length == 0 || part.length == 1) {
								document.getElementById('username'+<?php echo $b?>+i).value = newuser;
							} else {
								document.getElementById('username'+<?php echo $b?>+i).value = part[0] + "_" + newuser;
							}*/
							if(password_auto) {
								//document.getElementById('passwd'+<?php echo $b?>+i).value = newpasswd;
								var tmppasswd = passwd;
								document.getElementById('passwd'+<?php echo $b?>+i).value = tmppasswd.replace(numstr,newpasswd+'');

								var tmpusername = username;
								document.getElementById('username'+<?php echo $b?>+i).value = tmpusername.replace(usernamestr,newusername+'');
							} else {
								document.getElementById('passwd'+<?php echo $b?>+i).value = passwd;
								
								var tmpusername = username;
								document.getElementById('username'+<?php echo $b?>+i).value = tmpusername.replace(usernamestr,newusername+'');
							
							}	
							document.getElementById('ipaddr'+<?php echo $b?>+i).value = ipaddr;
						//	document.getElementById('bipaddr'+<?php echo $b?>+i).value =bipaddr;
						//	document.getElementById('sip_codec_priority'+<?php echo $b?>+i).value = codec;
						//	document.getElementById('sip_codec_mode'+<?php echo $b?>+i).value = codec_mode;
							document.getElementById('port'+<?php echo $b?>+i).value = port;
							document.getElementById('register_mode'+<?php echo $b?>+i).value = register_mode;
						//	document.getElementById('vosenc'+<?php echo $b?>+i).value = vosenc;
							//newuser = newuser + 1; 
							newpasswd = newpasswd + 1;
							newusername = newusername + 1;
						} else {
							document.getElementById('username'+<?php echo $b?>+i).value = '';
							document.getElementById('passwd'+<?php echo $b?>+i).value = '';
							document.getElementById('ipaddr'+<?php echo $b?>+i).value = '';
						//	document.getElementById('bipaddr'+<?php echo $b?>+i).value = '';
						//	document.getElementById('sip_codec_priority'+<?php echo $b?>+i).value = 'ulaw';
							document.getElementById('port'+<?php echo $b?>+i).value = '';
							document.getElementById('register_mode'+<?php echo $b?>+i).value = '';
						//	document.getElementById('vosenc'+<?php echo $b?>+i).value = 'No';
						}
					}
	<?php	
				}
			}
		}
	}

?>	
}

function register_check(){
	var register_mode = document.getElementById('register_mode10').value;
	if(register_mode == 'server'){
		document.getElementById('ipaddr10').value = 'dynamic';
	} else {
		document.getElementById('ipaddr10').value = '';
	}
}

function check(ports)
{
<?php
	global $__BRD_SUM__;
	global $__GSM_SUM__;
	global $__BRD_HEAD__;

	//$bindings = get_analog_portbinging_info();
	$bindings = array();

	$bindings_ary = '';
	if ($bindings) {
		$cluster_info = get_cluster_info();	
		if(($cluster_info['mode'] == 'master') || ($cluster_info['mode'] != 'master')) {
			for ($i = 1; $i <= $__GSM_SUM__; $i++) {
				if (isset($bindings['1'.$i])) {
					if ($bindings['1'.$i] == 1)
						$bindings_ary .= '"' . '1-'.$i . '"' . ',';
				}
			}
		}
		
		//added by wangxuechuan at 20131115
		if($__deal_cluster__){
			if($cluster_info['mode'] == 'master') {
				//update_cluster_board_num($cluster_info['devicemode']);
				for($b=2; $b<=$__BRD_SUM__; $b++) {
					$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
					if($slaveip != '') {				
						for ($i = 1; $i <= $__GSM_SUM__; $i++) {
							if (isset($bindings[$b.$i])) {
								if ($bindings[$b.$i] == 1)
									$bindings_ary .= '"' . $b.'-'.$i . '"' . ',';
							}
						}				
					}
				}
			}	
		}
	}

	$bindings_ary = rtrim($bindings_ary,',');

?>
	//var bindings_ary = new Array(<?php echo $bindings_ary; ?>);
	port_overwrite = "<?php echo language('port binding overwrite confirm');?>";
	ports_overwrite = "<?php echo language('ports binding overwrite confirm');?>";
	var  checkbox_flag = false;
	var newbindings_ary = new Array();
	for (i = 0; i < newbindings_ary.length; i++) {
		checkornot = document.getElementById('channel'+bindings_ary[i]).checked;
		if (checkornot == true)
			newbindings_ary.push(bindings_ary[i]);
	}

<?php
	$cluster_info = get_cluster_info();	
	if(($cluster_info['mode'] == 'master') || ($cluster_info['mode'] != 'master')) {
?>
		for (i = 1; i <= ports; i++) {
			if (document.getElementById('channel1'+i).checked != true) {
				continue;
			} else {
				checkbox_flag = true;
			}
			username = document.getElementById('username1'+i).value;
			passwd = document.getElementById('passwd1'+i).value;
			ipaddr = document.getElementById('ipaddr1'+i).value;

			if (username == '') {
				alert("<?php echo language('js check username 1', '\'User Name\' must not be null!');?>");
				document.getElementById('username1'+i).focus();
				return false;
			} else {
				if(!check_sipname(username)) {
					alert("<?php echo language('js check sipname', 'Allowed character must be any of [0-9a-zA-Z$*-=_.], length: 1-32');?>");
					document.getElementById('username1'+i).focus();
					return false;
				}
			}
			if (passwd != '') {
				if(!check_sippwd(passwd)) {
					alert("<?php echo language('js check sippwd', 'Allowed character must be any of [0-9a-zA-Z`~!#@$%^&*()_+{}|<>-=[],.],1 - 32 characters.');?>");
					document.getElementById('passwd1'+i).focus();
					return false;
				}
			}
			if(!check_domain(ipaddr)) {
				alert("<?php echo language('js check domain', 'Invalid hostname or IP address!');?>");
				document.getElementById('ipaddr1'+i).focus();
				return false;
			}
		}		
<?php
	}

	//added by wangxuechuan at 20131115
	if($__deal_cluster__){
		if($cluster_info['mode'] == 'master') {
			//update_cluster_board_num($cluster_info['devicemode']);
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
				if($slaveip != '') {
	?>
					for (i = 1; i <= ports; i++) {
						if (document.getElementById('channel'+<?php echo $b?>+i).checked != true) {
							continue;
						} else {
							checkbox_flag = true;
						}
						username = document.getElementById('username'+<?php echo $b?>+i).value;
						passwd = document.getElementById('passwd'+<?php echo $b?>+i).value;
						ipaddr = document.getElementById('ipaddr'+<?php echo $b?>+i).value;
						if (username == '') {
							alert("<?php echo language('js check username 1', '\'User Name\' must not be null!');?>");
							document.getElementById('username'+<?php echo $b?>+i).focus();
							return false;
						} else {
							if(!check_sipname(username)) {
								alert("<?php echo language('js check sipname', 'Allowed character must be any of [0-9a-zA-Z$*-=_.], length: 1-32');?>");
								document.getElementById('username'+<?php echo $b?>+i).focus();
								return false;
							}
						}
						if (passwd != '') {
							if(!check_sippwd(passwd)) {
								alert("<?php echo language('js check sippwd', 'Allowed character must be any of [0-9a-zA-Z`~!#@$%^&*()_+{}|<>-=[],.],1 - 32 characters.');?>");
								document.getElementById('passwd'+<?php echo $b?>+i).focus();
								return false;
							}
						}
						if(!check_domain(ipaddr)) {
							alert("<?php echo language('js check domain', 'Invalid hostname or IP address!');?>");
							document.getElementById('ipaddr'+<?php echo $b?>+i).focus();
							return false;
						}
						
					}
	<?php
				}
			}
		}	
	}
?>
	if(!checkbox_flag){
		alert('At least 1 item must be checked!');
		return false;
	}
	if (newbindings_ary.length == 1) {
		confirm_string = newbindings_ary[0] + " " + port_overwrite;
		if (!confirm(confirm_string)) {
			return false;
		}
	} else if (newbindings_ary.length > 1) {
		confirm_string = "";
		for (i = 0; i < newbindings_ary.length; i++) {
			confirm_string += newbindings_ary[i] + " ";
		}
		confirm_string += ports_overwrite;
		if (!confirm(confirm_string)) {
			return false;
		}
	}

	return true;
}

function click_password_auto(obj,ports)
{
	username = document.getElementById('username10').value;
	passwd = document.getElementById('passwd10').value;
	if((passwd == '') && (username == '')){
		return;
	}
		
	if(obj.checked) {
		if(username == '') {
			return;
		}

		/*var newpass = passwd;
		var numstr1 = passwd.match(/\d+.*$/g).toString();
		var oldnumstr1 = numstr1;
		var numstr2 = numstr1.match(/\d+/g).toString();
		var newpasswd = Number(numstr2);
		alert(numstr1+"  "+ numstr2+" "+newpasswd);*/
		
		var numarray = passwd.match(/\d+/g);
		if(numarray){
			  var numstr = numarray[numarray.length-1];
			  var newpasswd = Number(numstr);
		}
		var usernamearray = username.match(/\d+/g);
		if(usernamearray){
			  var usernamestr = usernamearray[usernamearray.length-1];
			  var newusername = Number(usernamestr);
		}else{
			alert("<?php echo language('js check busername', 'you need to set a number for username');?>");
			return ;
		}
<?php
		$cluster_info = get_cluster_info();	
		if(($cluster_info['mode'] == 'master') || ($cluster_info['mode'] != 'master')) {
?>
			for (i = 1; i <= ports; i++) {
				if (document.getElementById('channel1'+i).checked != true) {
					continue;
				}

				var tmppasswd = passwd;
				var tmpusername = username;
				document.getElementById('passwd1'+i).value = tmppasswd.replace(numstr,newpasswd+'');
				if (document.getElementById('username1'+i).value == '') {
					document.getElementById('username1'+i).value = tmpusername.replace(usernamestr,newusername+'');
				}
				/*numstr1.replace(numstr2, newpasswd);
				newpass.replace(oldnumstr1,numstr1);
				document.getElementById('passwd1'+i).value = newpass;*/
				newpasswd = newpasswd + 1;
				newusername = newusername + 1;
			}
<?php
		}

		//added by wangxuechuan at 20131115
		global $__BRD_SUM__;
		global $__BRD_HEAD__;
		if($__deal_cluster__){
			if($cluster_info['mode'] == 'master') {
				//update_cluster_board_num($cluster_info['devicemode']);
				for($b=2; $b<=$__BRD_SUM__; $b++) {
					$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
					if($slaveip != '') {
	?>
						for (i = 1; i <= ports; i++) {
							if (document.getElementById('channel'+<?php echo $b?>+i).checked != true) {
								continue;
							}

							var tmppasswd = passwd;
							document.getElementById('passwd'+<?php echo $b?>+i).value = tmppasswd.replace(numstr,newpasswd+'');
							newpasswd = newpasswd + 1;

							var tmpusername = username;
							if (document.getElementById('username'+<?php echo $b?>+i).value == '') {
								document.getElementById('username'+<?php echo $b?>+i).value = tmpusername.replace(usernamestr,newusername+'');
							}
							newusername = newusername + 1;
						}
	<?php	
					}
				}
			}
		}
?>	
	} else {
		if(passwd == '') {
			passwd = username;
		}
<?php
		$cluster_info = get_cluster_info();	
		if(($cluster_info['mode'] == 'master') || ($cluster_info['mode'] != 'master')) {
?>
			for (i = 1; i <= ports; i++) {
				if (document.getElementById('channel1'+i).checked != true) {
					continue;
				}
				
				document.getElementById('passwd1'+i).value = passwd;
			}
<?php
		}

		//added by wangxuechuan at 20131115
		global $__BRD_SUM__;
		global $__BRD_HEAD__;
		if($__deal_cluster__){
			if($cluster_info['mode'] == 'master') {
				//update_cluster_board_num($cluster_info['devicemode']);
				for($b=2; $b<=$__BRD_SUM__; $b++) {
					$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
					if($slaveip != '') {
	?>
						for (i = 1; i <= ports; i++) {
							if (document.getElementById('channel'+<?php echo $b?>+i).checked != true) {
								continue;
							}
							
							document.getElementById('passwd'+<?php echo $b?>+i).value = passwd;
						}
	<?php	
					}
				}
			}
		}
?>	
	}
}
</script>

<?php

$fxs_use_hidecallerid = 0;
function show_sip_endpoints()
{
	global $__GSM_SUM__;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $__system_type__;
	global $__deal_cluster__;
	global $only_view;
	
?>
	<form enctype="multipart/form-data" action="<?php echo $_SERVER['PHP_SELF'] ?>" method="post">
	
		<div class="content">
			<table class="table_show">
				<tr>
					<th style="width:03%" >
						<input type="checkbox" name="selall" onclick="selectall(this.checked, <?php echo $__GSM_SUM__; ?>)" />
					</th>		
					<th width="7%"><?php echo language('ID');?></th>
					<th width="15%"><?php echo language('User Name');?></th>
					<th width="15%"><?php echo language('Password');?></th>
					<th width="15%"><?php echo language('Hostname or IP Address');?></th>
					<th width="15%"><?php echo language('Port')?></th>
					<th width="20%"><?php echo language('Outbound Proxy');?></th>
					<th width="15%"><?php echo language('Register Mode')?></th>
				</tr>
				
				<?php
					for ($i = 0; $i <= $__GSM_SUM__; $i++) {
					$hcc="style=background-color:#ECFFEF";
				?>
					<tr>
						<td <?php if($i==0){ echo $hcc;}?>> 
						<?php
						if ($i != 0) {
						?>
						<input type="checkbox" name="<?php echo 'channel1'.$i; ?>" id="<?php echo 'channel1'.$i; ?>"/>
						<?php
						}
						?>
						</td>		
						<td <?php if($i==0){ echo $hcc;}?>>
							<?php 
								if($i != 0) {
									echo $i; 
								}
							?>
						</td>
						<td <?php if($i==0){ echo $hcc;}?>>
							<input type=text name="<?php echo 'username1'.$i; ?>" id="<?php echo 'username1'.$i; ?>" value="" style="width: 100px;" />
						</td>
						<td <?php if($i==0){ echo $hcc;}?>>
							<input type=text name="<?php echo 'passwd1'.$i; ?>" id="<?php echo 'passwd1'.$i; ?>" value="" style="width: 100px;" />
						</td>
						<td <?php if($i==0){ echo $hcc;}?>>
							<input type=text name="<?php echo 'ipaddr1'.$i; ?>" id="<?php echo 'ipaddr1'.$i; ?>" value="" style="width: 110px;" />
						<!--	<input type=text name="<?php echo 'bipaddr1'.$i; ?>" id="<?php echo 'bipaddr1'.$i; ?>" value="" style="width: 110px;" /> -->
						</td>
						
						<td <?php if($i==0){ echo $hcc;}?>>
							<input type=text name="<?php echo 'port1'.$i; ?>" id="<?php echo 'port1'.$i; ?>" value="" style="width: 100px;" />
						</td>
						<td <?php if($i==0){ echo $hcc;}?>>
							<input type="text" name="<?php echo 'outboundproxy'.$i; ?>" id="<?php echo 'outboundproxy'.$i; ?>" value="" style="width:110px"/>:
							<input type="text" name="<?php echo 'outboundproxy_port'.$i; ?>" id="<?php echo 'outboundproxy_port'.$i; ?>" size="1" value="" >
						</td>
			
						<td <?php if($i==0){ echo $hcc;}?>>
							<select size=1 name="register_mode1<?php echo $i;?>" id="register_mode1<?php echo $i;?>" onchange="register_check()">
							<option value="client">client</option>
							
							<?php 
							session_start();
							if($_SESSION['id'] == 1){
							?>
							<option value="server">server</option>
							<?php } ?>
							
							<option value="none">none</option>
							</select>
						</td>
					</tr>
				<?php } ?>
			</table>
		</div>
		
		<input type="hidden" name="send" id="send" value="Save" />
		
		<div id="button_save">
		
			<?php if(!$only_view){ ?>
			<button type="submit" name="sendlabel" id="sendlabel" onclick="document.getElementById('send').value='Save';return check(<?php echo $__GSM_SUM__; ?>);"><?php echo language('Save');?></button>
			<?php } ?>
			
			<button type="button" onclick="location.reload()" ><?php echo language('Cancel');?></button>
			<button type="button" onclick="setValue(<?php echo $__GSM_SUM__; ?>)" ><?php echo language('Batch');?></button>
			<span style="float:right;margin-top:7px;">
				<input type="checkbox" id="password_auto" name="password_auto" onclick="click_password_auto(this,<?php echo $__GSM_SUM__; ?>)" checked=true/>
				<?php echo language('AutoPassword');?>
			</span>
		</div>
		
	</form>
	<script type="text/javascript">
		function lock_button(){
			if($("#float_button_5").attr("checked")=="checked"){
				$("#password_auto").attr({"checked":true});
			} else {
				$("#password_auto").attr({"checked":false});
			}
		}
		$(document).ready(function (){ 
				//check_chnl_type();
				$("#float_button_3").mouseover(function(){
				  $("#float_button_3").css({opacity:"1",filter:"alpha(opacity=100)"});
				});
				$("#float_button_3").mouseleave(function(){
				  $("#float_button_3").css({opacity:"0.5",filter:"alpha(opacity=50)"});
				});
				$("#float_button_4").mouseover(function(){
				  $("#float_button_4").css({opacity:"1",filter:"alpha(opacity=100)"});
				});
				$("#float_button_4").mouseleave(function(){
				  $("#float_button_4").css({opacity:"0.5",filter:"alpha(opacity=50)"});
				});
		}); 
	</script>
<?php
}
?>

<?php

function save_sip_endpoints()
{
	global $__GSM_SUM__;
	global $__BRD_SUM__;	
	global $__BRD_HEAD__;
	global $fxs_use_hidecallerid;
	global $__file_endpoints;
	global	$__config_memory;
	global $__system_type__;
	global $__deal_cluster__;

////////////////////////////////////////////////////////////////////////////////////////////////
	//read variable
//	GetEfficien();
////////////////////////////////////////////////////////////////////////////////////////////////

	$all_sips = get_all_sips(true);
	$last_order = 1;
	if($all_sips) {
		$last = end($all_sips); 
		if(isset($last['order']) && $last['order'] != '') {
			$last_order = $last['order'] + 1;
		}
	}

	$aql = new aql();
	//Save to gw_endpoints.conf
	$gw_endpoints_conf_path = '/etc/asterisk/gw_endpoints.conf';
	$hlock = lock_file($gw_endpoints_conf_path);
	if (!file_exists($gw_endpoints_conf_path)) fclose(fopen($gw_endpoints_conf_path,"w"));
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($gw_endpoints_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	//$res1 = $aql->query("select * from gw_endpoints.conf");
	//	if($__EFFICIEN__)
	//	{
	//		$filechunk = $aql->query("select * from gw_endpoints.conf");
	//	}
	$changeClusterPassword=false;
	$cluster_info = get_cluster_info();	

	if(($cluster_info['mode'] == 'master') || ($cluster_info['mode'] != 'master')) {
			

			for ($i = 1; $i <= $__GSM_SUM__; $i++) {
				$idname = 'channel1'.$i;
				if (!isset($_POST[$idname]))
					continue;

				//Save to gw_endpoints.conf
				$datachunk = '';
				$idname = 'username1'.$i;
				$username = trim($_POST[$idname]);
				$idname = 'register_mode1'.$i;
				$registration = trim($_POST[$idname]);
				
				
				for($m=1;$m<=8;$m++){
					if(($username==trim($cluster_info['master_password']).$m) && ($cluster_info['mode'] == 'master')) {
						$changeClusterPassword=true;
					}
				}


				$datachunk .= "username=$username\n";
				$datachunk .= "registration=$registration\n";
				$datachunk .= "allow_anonymous=no\n";
				$datachunk .= 'order='.$last_order."\n";

				$last_order += 1;
				$idname = 'passwd1'.$i;
				$passwd = trim($_POST[$idname]);
				$datachunk .= 'secret='.$passwd."\n";
					
				$idname = 'ipaddr1'.$i;					
				$ipaddr = trim($_POST[$idname]);

				$idname = 'port1'.$i;
                $port = trim($_POST[$idname]);
					
				$datachunk .= 'host='.$ipaddr."\n";
				$datachunk .= "transport=udp\n";
				$datachunk .= "port=$port\n";
				$datachunk .= "nat=yes\n";
				
				$outboundproxy = trim($_POST['outboundproxy'.$i]);
				$outboundproxy_port = trim($_POST['outboundproxy_port'.$i]);
				
				if($outboundproxy != '' && $outboundproxy_port != ''){
					$outboundproxy_val = $outboundproxy.":".$outboundproxy_port;
				}else if($outboundproxy != ''){
					$outboundproxy_val = $outboundproxy.":5060";
				}else{
					$outboundproxy_val = '';
				}
				$datachunk .= 'outboundproxy='.$outboundproxy_val."\n";
				
				if($outboundproxy_val != ''){
					$datachunk .= 'auth='.$username."\n";
					$datachunk .= 'contactuser='.$username."\n";
					$datachunk .= 'fromuser='.$username."\n";
					$datachunk .= 'fromdomain='.$ipaddr."\n";
					$datachunk .= "registery_enable=yes\n";
					$datachunk .= 'registery_string='.$username.'@'.$ipaddr.':'.$passwd.':'.$username.'@'.$outboundproxy_val.'/'.$username."\n";
					$datachunk .= "qualify=no\n";
					$datachunk .= "session-timers=refuse\n";
				}else{
					$datachunk .="fromdomain=\n";
					$datachunk .= "qualify=yes\n";
				}
				
				if($registration == 'client') {
					$datachunk .= 'register_extension='.$username."\n";
					$datachunk .= 'register_user='.$username."\n";
				}
				if (($__system_type__=='yfree') || ($__system_type__=='general')) {
					$datachunk .= "qualifyfreq=60\n";
				} else {
					$datachunk .= "qualifyfreq=15\n";
				}
				$datachunk .= "dtmfmode=rfc2833\n";
				$datachunk .= "trustrpid=no\n";
				$datachunk .= "sendrpid=no\n";
				$datachunk .= "callingpres=allowed_passed_screen\n";
				$datachunk .= "progressinband=never\n";
				$datachunk .= "usereqphone=no\n";
				$datachunk .= "use_q850_reason=no\n";
				$datachunk .= "directmedia=yes\n";
				$datachunk .= "ignorsdpversion=yes\n";
				$datachunk .= "allowtransfer=yes\n";
				$datachunk .= "promiscredir=no\n";
				$datachunk .= "max_forwards=70\n";
				$datachunk .= "registertrying=no\n";
				$datachunk .= "timert1=500\n";
				$datachunk .= "timerb=32000\n";
				$datachunk .= "session-timers=accept\n";
				$datachunk .= "session-minse=90\n";
				$datachunk .= "session-expires=1800\n";
				$datachunk .= "session-refresher=uas\n";

				if($registration == 'client') {
					if (!empty($port)) {
						$register = $username.":".$passwd."@".$ipaddr.":".$port."/".$username;
					} else {
						$register = $username.":".$passwd."@".$ipaddr.$port."/".$username;
					}
					
					if($outboundproxy_val != ''){
						$register = $username.'@'.$ipaddr.':'.$passwd.':'.$username.'@'.$outboundproxy_val.'/'.$username;
					}
					
					$datachunk .= 'register=>'.$register."\n";
				}

				$datachunk .= "insecure=port,invite\n";
				$datachunk .= "type=friend\n";
				$datachunk .= "context=$username\n";

				$aql->assign_delsection($username);
				$aql->save_config_file('gw_endpoints.conf');
				$aql->assign_addsection($username,$datachunk);
				$aql->save_config_file('gw_endpoints.conf');

			}		
		}

	//added by wangxuechuan at 20131115
	if($__deal_cluster__){
		if($cluster_info['mode'] == 'master') {
			//update_cluster_board_num($cluster_info['devicemode']);
			for($b=2; $b<=$__BRD_SUM__; $b++) {

				$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
				if($slaveip != '') {
					for ($i = 1; $i <= $__GSM_SUM__; $i++) {
						$idname = 'channel'.$b.$i;
						if (!isset($_POST[$idname]))
							continue;

						//Save to gw_endpoints.conf
						$datachunk = '';
						$idname = "username".$b.$i;
						$username = trim($_POST[$idname]);
						$idname = "register_mode".$b.$i;
						$registration = trim($_POST[$idname]);
						
						for($m=1;$m<=8;$m++){
							if($username==trim($cluster_info['master_password']).$m) {
								$changeClusterPassword=true;
							}
						}


						$datachunk .= "username=$username\n";
						$datachunk .= "registration=$registration\n";
						$datachunk .= "allow_anonymous=no\n";
						$datachunk .= 'order='.$last_order."\n";

						$last_order += 1;
						$idname = "passwd".$b.$i;
						$passwd = trim($_POST[$idname]);
						$datachunk .= 'secret='.$passwd."\n";
						
						$idname = 'ipaddr'. $b .$i;					
						$ipaddr = trim($_POST[$idname]);

						$idname = 'port'.$b.$i;
						$port = trim($_POST[$idname]);
						
						$datachunk .= 'host='.$ipaddr."\n";
						$datachunk .= "transport=udp\n";
						$datachunk .= "port=$port\n";
						$datachunk .= "nat=yes\n";
						if($registration == 'client') {
							$datachunk .= 'register_extension='.$username."\n";
							$datachunk .= 'register_user='.$username."\n";
						}
						$datachunk .="fromdomain=\n";
						$datachunk .= "qualify=yes\n";
						if (($__system_type__=='yfree') || ($__system_type__=='general')) {
							$datachunk .= "qualifyfreq=60\n";
						} else {
							$datachunk .= "qualifyfreq=15\n";
						}
						$datachunk .= "dtmfmode=rfc2833\n";
						$datachunk .= "trustrpid=no\n";
						$datachunk .= "sendrpid=no\n";
						$datachunk .= "callingpres=allowed_passed_screen\n";
						$datachunk .= "progressinband=never\n";
						$datachunk .= "usereqphone=no\n";
						$datachunk .= "use_q850_reason=no\n";
						$datachunk .= "directmedia=yes\n";
						$datachunk .= "ignorsdpversion=yes\n";
						$datachunk .= "allowtransfer=yes\n";
						$datachunk .= "promiscredir=no\n";
						$datachunk .= "max_forwards=70\n";
						$datachunk .= "registertrying=no\n";
						$datachunk .= "timert1=500\n";
						$datachunk .= "timerb=32000\n";
						$datachunk .= "session-timers=accept\n";
						$datachunk .= "session-minse=90\n";
						$datachunk .= "session-expires=1800\n";
						$datachunk .= "session-refresher=uas\n";

						if($registration == 'client'){
							if (!empty($port)) {
								$register = $username.":".$passwd."@".$ipaddr.":".$port."/".$username;
							} else {
								$register = $username.":".$passwd."@".$ipaddr.$port."/".$username;
							}
							$datachunk .= 'register=>'.$register."\n";
						}
						$datachunk .= "insecure=port,invite\n";
						$datachunk .= "type=friend\n";
						$datachunk .= "context=$username\n";
				

						$aql->assign_delsection($username);
						$aql->save_config_file('gw_endpoints.conf');
						$aql->assign_addsection($username,$datachunk);
						$aql->save_config_file('gw_endpoints.conf');

					}
				
					$gw_endpoints_file = '/etc/asterisk/gw_endpoints.conf';
				}
			}
		}
	}

	
	unlock_file($hlock);

	if($changeClusterPassword){
		changeClusterPassword('');
	}
	    
	save_endpoints_to_sips();
	save_routings_to_extensions();

	wait_apply("exec","/my_tools/cluster_mode update_sip_master");

	$__config_memory = false;
	$__file_endpoints = "";
	
	save_user_record("","VOIP->Batch SIP Bindpoints:Save");

	return 0;
}

if($_POST) {
	if (isset($_POST['send']) && $_POST['send'] == 'Save') {
		if($only_view){
			return false;
		}
		
		$ret = save_sip_endpoints();
		if ($ret == 0) {
			show_sip_endpoints();
			//ast_reload();
			wait_apply("exec", "asterisk -rx \"core reload\" > /dev/null 2>&1 &");
			wait_apply("exec", "asterisk -rx \"dahdi restart\" > /dev/null 2>&1 &");
		}
	}
} else {
	show_sip_endpoints();
}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>