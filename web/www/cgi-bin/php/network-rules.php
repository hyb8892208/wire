<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
require_once("/www/cgi-bin/inc/aql.php");
?>

<?php
function show_rules()
{
	$all_rules = get_all_firewall_rules(true);
	$last_order = 1;
	if($all_rules) {
		$last = end($all_rules); 
		if(isset($last['order']) && $last['order'] != '') {
			$last_order = $last['order'] + 1;
		}
	}
?>

	<script type="text/javascript">
	var sec = 60;
	var t;
	function getPage(value)
	{
		window.location.href = '<?php echo get_self();?>?sel_rule_name='+value;
	}

	function setValue(value1,value2)
	{
		document.getElementById('sel_rule_name').value = value1;
		document.getElementById('order').value = value2;
	}

	function delete_click(value1,value2)
	{
		ret = confirm("Are you sure to delete you selected ?");

		if(ret) {
			document.getElementById('sel_rule_name').value = value1;
			document.getElementById('order').value = value2;
			return true;
		}

		return false;
	}
	function preview_dialog() {
		sec = 60;
		$( "#preview_dg" ).dialog({
			resizable: false,
			height:400,
			width:500,
			modal: true,

			buttons: [
				{
					text:"Apply",
					id:"Apply",
					click:function(){
						remove_recover();
					}
				},
				{
					text:"Close",
					id:"close",
					click:function(){
						clearTimeout(t);
						$( this ).dialog( "close" );
					}
				}
			]		
		});
		if ( t != null ) {
			clearTimeout(t);
			sec = 60;
		}
		utime(sec);
		
		var server_file = "./../../cgi-bin/php/ajax_server.php";
		$.ajax({
			url: server_file+"?random="+Math.random()+"&type=system&firewall=firewall_preview",	
			async: false,
			dataType: 'text',
			type: 'GET',
			timeout: 5000,
			error: function(data){				//request failed callback function;
				str = "";
				str += "<br><font color='red' size= '3px' style='font-weight:bold'>ERROR: </font> <font color='green'>Configure firewall rules failed.</font>";
				document.getElementById("redmsg").innerHTML = str;
				
			},
			success: function(data){			//request success callback function;
				if(data == ''){
					str = "";
					str += "<br><font color='red' size= '3px' style='font-weight:bold'>ERROR: </font> <font color='green'>Configure firewall rules failed.</font>";
					document.getElementById("redmsg").innerHTML = str;
					return;
				} else {
					str = "";
					str += "<br><font color='red' size= '3px' style='font-weight:bold'>Warning: </font>";
					str += "<font color='green'><br>Please check your security rules carefully before apply!!! ";
					str += "<br>Wrong rules will cause abnormal behavior on gateway! </font>";
					str += "<br><br><font color='red' size= '3px' style='font-weight:bold'>Apply Tips: </font>";
					str += "<font color='green'><br>If your security rules will result in no response on web login, all rules will be deactivated.";
					str += "<br>You can login gateway and check the rules again after 1 minute. ";
					str += "<br>Otherwise, they will be applied successfully.<br><br></font>";

					document.getElementById("redmsg").innerHTML = str;
				}
			}
		});
	}
	
	
	function utime()
	{
		sec--;
		var str = "<br><font color='red' size= '3px' style='font-weight:bold'>Notice: </font>";
		str += "<br><font color='#00ff33' size= '3px' style='font-weight:bold'>" + sec + " </font><font color='green'>seconds later, all rules will be deactivated.";
		str += "<br>The dialog will close automitically, when the time runs out.";
		
		if (sec == 0) {
			$( "#preview_dg" ).dialog("close");
		}
		document.getElementById('timemsg').innerHTML = str;
		t = setTimeout("utime()", 1000);
	}
	
	function remove_recover()
	{
		var server_file = "./../../cgi-bin/php/ajax_server.php";
		$.ajax({
			url: server_file+"?random="+Math.random()+"&type=system&firewall=firewall_recover",	
			async: false,
			dataType: 'text',
			type: 'GET',
			timeout: 3000,
			error: function(data) {
				str = "";
				str += "<br><font color='red' size= '3px' style='font-weight:bold'>ERROR: </font> <font color='green' >Firewall configuration errors.</font>";
				str += "<br><br><font color='00ff33' style='font-weight:bold'>New security rules result in no response on web login.<br>Please check and modify them carefully!!</font>";	
				document.getElementById("redmsg").innerHTML = str;
			},
			success: function(data) {
				str = "";
				if (data.indexOf("off") >= 0) {
					str += "<br><font color='red' size= '3px' style='font-weight:bold'>Firewall Switch is off. </font>";
					str += "<br><font color='green'>You should open the firewall switch on the page of 'Security Settings'.<br><br></font>";
				} else {
					if ( (data.indexOf("ACCEPT ")) < 0 && (data.indexOf("DROP ")) < 0 ) {
						str += "<br><font color='red' size= '3px' style='font-weight:bold'>All rules removed. </font>";
						str += "<br><font color='green'>You should re-click the 'submit' button on the page to re-apply.<br><br></font>";
					} else {
						str += "<br><font color='red' size= '3px' style='font-weight:bold'>All rules are active now!</font>";
						str += "<br><br><font color='green' size= '2px' style='font-weight:bold' >Firewall rules list below:</font>";
						str += "<br><br>";
						str +=data;
					}
				}
				clearTimeout(t);
				document.getElementById("timemsg").innerHTML = '';
				document.getElementById('redmsg').innerHTML = str;
			}
		});
	}

	</script>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<input type="hidden" id="sel_rule_name" name="sel_rule_name" value="" />
	<input type="hidden" id="order" name="order" value="" />

	<table width="100%" id="tab_rules" class="tdrag">
		<tr>
			<th width="15%"><?php echo language('Rule Name');?></th>
			<th width="15%"><?php echo language('Type');?></th>
			<th width="15%"><?php echo language('Protocol');?></th>
			<th width="25%"><?php echo language('IP');?></th>
			<th width="20%"><?php echo language('Port');?></th>
			<th width="6%"><?php echo language('Actions');?></th>
		</tr>

<?php
	if($all_rules) {
		foreach($all_rules as $rule) {
			$rule_name = trim($rule['rule_name']);
			$protocol = trim($rule['protocol']);
			
			$port = trim($rule['port']);
			$ip = trim($rule['ip']);
			$actions = trim($rule['actions']);
			

?>

		<tr bgcolor="#E8EFF7">
			<td>
				<?php echo $rule_name ?>
			</td>
			<td>
				<?php echo $protocol ?>
			</td>
			<td>
				<?php echo $actions ?>
			</td>
			<td>
				<?php echo $ip ?>
			</td>
			<td>
				<?php echo $port ?>
			</td>
			<td>
				<button type="button" value="Modify" style="width:32px;height:32px;" 
					onclick="getPage('<?php echo $rule_name ?>')">
					<img src="/images/edit.gif">
				</button>
				<button type="submit" value="Delete" style="width:32px;height:32px;" 
					onclick="document.getElementById('send').value='Delete';return delete_click('<?php echo $rule_name ?>', '')" >
					<img src="/images/delete.gif">
				</button>
				<input type="hidden" name="rule_name[]" value="<?php echo $rule_name ?>" />
			</td>
		</tr>
<?php
		}
	}
?>
	</table>
	<br>
	<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
	<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>	
	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" value="<?php echo language('New Rule');?>" onclick="document.getElementById('send').value='New Rule';setValue('', '<?php echo $last_order ?>')" />

	<div id="preview_dg" title="Firewall Rules Apply" style="display:none">
		<div>
			<div id="redmsg" style="display:block;width:470px;margin:0 auto;"  contenteditable="false" ></div>
			<br>
			<div id="timemsg" style="display:block;width:470px;margin:0 auto" contenteditable = "false"></div>
		</div>
	</div>
		<input type="button" id="preview_btn" value="<?php echo language('Submit');?>" onclick="preview_dialog();" />
	<br />
	</form>
<?php
}
?>

<?php
function save_rules()
{
	// /etc/asterisk/firewall_rules.conf
	// [rule_name]
	// order = 1,2,3,4,5....    //Must set
	// protocol = tcp udp or icmp  //Must set
	// ip = ip/mask
	// port = port1-port2   //Must set

	$datachunk = '';

	//rule name already existed! 
	if( isset($_POST['rule_name']) ) {
		$rule_name = trim($_POST['rule_name']);
		if($rule_name == '') {
			echo "Must set rule name";
			return false;
		}
		$section = $rule_name;
	} else {
		echo "Must set rule name";
		return false;
	}

	$old_section = $section;
	if( isset($_POST['old_rule_name']) ) {
		$old_section = trim($_POST['old_rule_name']);
	}

	if( isset($_POST['order']) ) {
		$order = trim($_POST['order']);
		if($order == '') {
			echo "[$rule_name] ";
			echo language('does not exist');
			//echo "Must set order";
			return false;
		}
		$datachunk .= 'order='.$order."\n";
	} else {
		echo "[$rule_name] ";
		echo language('does not exist');
		//echo "Must set order"
		return false;
	}


	if( isset($_POST['protocol']) ) {
		$protocol = trim($_POST['protocol']);
		if($protocol == '') {
			echo "Must set protocol";
			return false;
		}
		$datachunk .= 'protocol='.$protocol."\n";
	} else {
		echo "Must set protocol";
		return false;
	}

	if( isset($_POST['actions']) ) {
		$actions = trim($_POST['actions']);
		if($actions == '') {
			echo "Must set actions";
			return false;
		}
		$datachunk .= 'actions='.$actions."\n";
	} else {
		echo "Must set actions";
		return false;
	}

	if (isset($_POST['allow_port1']) && isset($_POST['allow_port2'])) {
		$allow_port1 = trim($_POST['allow_port1']);
		$allow_port2 = trim($_POST['allow_port2']);
		if ($allow_port1 != null && $allow_port2 != null ) {
			$datachunk .= 'port='.$allow_port1.':'.$allow_port2."\n";
		} else {
			$datachunk .= 'port='.$allow_port1.$allow_port2."\n";
		}
	} else {
		if($protocol == 'ICMP'){
			$datachunk .= 'port=' . "\n";
		} else {
			echo "Must set port";
			return false;
		}
	}

	if (isset($_POST['allow_ip']) && isset($_POST['allow_mask'])) {
		$allow_ip = trim($_POST['allow_ip']);
		$allow_mask = trim($_POST['allow_mask']);
		if ($allow_ip != null && $allow_mask != null) {
			$datachunk .= 'ip='.$allow_ip.'/'.$allow_mask."\n";
		} else {
			$datachunk .= 'ip='.$allow_ip.$allow_mask."\n";
		}
	} else {
		echo "Must set ip";
		return false;
	}
	//Save to gw_rule.conf
	///////////////////////////////////////////////////
	$firewall_rules_conf_path = "/etc/asterisk/gw/firewall_rules.conf";
	$hlock = lock_file($firewall_rules_conf_path);
	if (!file_exists($firewall_rules_conf_path)) fclose(fopen($firewall_rules_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	if(!$aql->open_config_file($firewall_rules_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	$aql->assign_delsection($old_section);
	$aql->save_config_file('firewall_rules.conf');
	$aql->assign_addsection($section,$datachunk);
	$aql->save_config_file('firewall_rules.conf');
	unlock_file($hlock);
	///////////////////////////////////////////////////

	return true;
}
?>

<?php
function del_rule($rule_name)
{
	//Save to firewall_rules.conf
	///////////////////////////////////////////////////
	// /etc/asterisk/gw/firewall_rules.conf
	$firewall_rules_conf_path = "/etc/asterisk/gw/firewall_rules.conf";
	$hlock = lock_file($firewall_rules_conf_path);
	if (!file_exists($firewall_rules_conf_path)) fclose(fopen($firewall_rules_conf_path,"w"));
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	if(!$aql->open_config_file($firewall_rules_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	$aql->assign_delsection($rule_name);
	$aql->save_config_file('firewall_rules.conf');
	unlock_file($hlock);
	///////////////////////////////////////////////////

}
?>


<?php


function add_rule_page($rule_name,$order = '')
{
	global $__demo_enable__;
	
	if($rule_name) {
		echo "<h4>";echo language('Modify a Rule');echo "</h4>";
	} else {
		echo "<h4>";echo language('Create a Rule');echo "</h4>";
	}

	// /etc/asterisk/gw/firewall_rules.conf
	// [rule_name]
	// order = 1,2,3,4,5....    
	// protocol = tcp udp or icmp  
	// ip = ip/mask
	// port = port1-port2   

	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk/gw');
	if (!$setok) {
		return;
	}

	$section = $rule_name;
	

	if($section) {
		$hlock = lock_file('/etc/asterisk/gw/firewall_rules.conf');
		$res = $aql->query("select * from firewall_rules.conf where section='$section'");
		unlock_file($hlock);
	}

	if(isset($res[$section]['order'])) {
		if($rule_name) {
			$order = trim($res[$section]['order']);
		}
	}

	if(isset($res[$section]['type'])) {
		$type = trim($res[$section]['type']);
	} else {
		$type = 'gsm';
	}

	if(isset($res[$section]['protocol'])) {
		$protocol = trim($res[$section]['protocol']);
	} else {
		$protocol = '';
	}
	
	if(isset($res[$section]['port'])) {
		$ports = trim($res[$section]['port']);
		if (strpos($ports,":")) {
			$ports = explode(":",$ports);
			$allow_port1 = $ports[0];
			$allow_port2 = $ports[1];
		} else {
			$allow_port1 = trim($ports);
			$allow_port2 = "";
		}
	} else {
		$allow_port1 = "";
		$allow_port2 = "";
	}
	
	if(isset($res[$section]['ip'])) {
		$ips = trim($res[$section]['ip']);
		if (strpos($ips,"/")) {
			$ips = explode("/",$ips);
			$allow_ip = trim($ips[0]);
			$allow_mask = trim($ips[1]);
		} else {
			$allow_ip = trim($ips);
			$allow_mask = "";
		}
	} else {
		$allow_ip = '';
		$allow_mask = '';
	}
	
	if(isset($res[$section]['actions'])) {
		$actions = trim($res[$section]['actions']);
	} else {
		$actions = '';
	}
?>

	<script type="text/javascript" src="/js/check.js"></script>
	<script type="text/javascript" src="/js/functions.js"></script>
	<script type="text/javascript" src="/js/float_btn.js"></script>

	<script type="text/javascript">

	function onload_func()
	{
		 
	}

	function check_protocol()
	{
		var protocol = document.getElementById('protocol').value;
		//console.log('protocol = '+protocol);
		if(protocol == "ICMP"){
			document.getElementById('allow_port1').disabled = true;
			document.getElementById('allow_port2').disabled = true;
		} else {
			document.getElementById('allow_port1').disabled = false;
			document.getElementById('allow_port2').disabled = false;
		}
		return;
	}
	
	function check()
	{


	<?php
		$arules = get_all_firewall_rules();

		$name_ary = '';
		if($arules) {
			foreach($arules as $rule) {		
				if(strcmp($rule['rule_name'],$rule_name)==0)
					continue;
				$name_ary .=  '"'.$rule['rule_name'].'"'.',';
			}
		}
		$name_ary = rtrim($name_ary,',');
	?>
		var name_ary = new Array(<?php echo $name_ary; ?>);
		var rule_name = document.getElementById('rule_name').value;
		var port1 = parseInt(document.getElementById('allow_port1').value);
		var port2 = parseInt(document.getElementById('allow_port2').value);
		var ip = document.getElementById('allow_ip').value;
		var mask = document.getElementById('allow_mask').value;
		
		if(!check_routingname(rule_name)) {
			document.getElementById("crule_name").innerHTML = con_str('<?php echo htmlentities(language('js check rulename','Allowed character must be any of [0-9a-zA-Z`~!@#$%^*()_{}:|?-=.], 1-32 characters.'));?>');
			document.getElementById("cport").innerHTML = '';
			document.getElementById("cip").innerHTML = '';
			return false;
		}
		
		document.getElementById('crule_name').innerHTML = '';
		for (var i in name_ary) 
		{
			if(name_ary[i] == rule_name) {
				document.getElementById('crule_name').innerHTML = con_str('Already exist.');
				return false;
			}
		}
		
		document.getElementById('cport').innerHTML = '';

		if (port1>port2) {
			document.getElementById("cport").innerHTML = con_str('<?php echo 'The start port must smaller than the end port.';?>');
			document.getElementById("cip").innerHTML = '';
			return false;			
		} 
		if ( (port1 < 0) || (port1 > 65535) || (port2<0) || (port2>65535)) {
			document.getElementById('cport').innerHTML = con_str('<?php echo 'The range of port is 0~65535.Enter the port is invalid.';?>');
			document.getElementById("cip").innerHTML = '';
			return false;
		}
		
		document.getElementById('cip').innerHTML = '';
		if(!check_domain(ip) ) {
			if (ip != "") {
				document.getElementById("cip").innerHTML = con_str('<?php echo language('js check domain','Invalid domain or IP address.');?>');
				return false;
			} else {
				if (mask != "") {
					document.getElementById("cip").innerHTML = con_str('<?php echo "If IP is empty,mask should not be set.";?>');
					return false;
				}
			}
		}

		if (!check_domain(mask)) {
			if (mask != "") {
				mask = parseInt(mask);
				if (!isNaN(mask)) {
					if (mask <0 || mask >32) {
						document.getElementById("cip").innerHTML = con_str('<?php echo "The range of mask is 0~32,or be form of domain name.";?>');
						return false;
					}
				} else {
					document.getElementById("cip").innerHTML = con_str('<?php echo "The type of mask should be domain name or number.";?>');
					return false;
				}
			} 
		}

		return true;
	}
	</script>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<input type="hidden" id="order" name="order" value="<?php echo $order?>" />
	<input type="hidden" id="old_rule_name" name="old_rule_name" value="<?php echo $rule_name?>" />

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Security Rules');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tedit" >
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Rule Name');?>:
					<span class="showhelp">
					<?php echo language('Rule Name help',"The name of this rule.");?>
					</span>
				</div>
			</th>
			<td >
				<input type="text" name="rule_name" id="rule_name" value="<?php echo $rule_name?>" /><span id="crule_name"></span>
			</td>
		</tr>

		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Protocol');?>:
					<span class="showhelp">
					<?php echo language('Protocol help',"Choose the protocol.");?>
					</span>
				</div>
			</th>
			<td >
				<select name="protocol" id="protocol" onchange="check_protocol()">
<?php
					$show_protocol[0] = '';
					$show_protocol[1] = '';
					$show_protocol[2] = '';
					switch($protocol) {
						case 'TCP': $show_protocol[0] = 'selected'; break;
						case 'UDP': $show_protocol[1] = 'selected'; break;
						case 'ICMP': $show_protocol[2] = 'selected'; break;
						default: $show_protocol[0] = 'selected'; break;
					}
?>
					<option value="TCP" <?php echo $show_protocol[0];?> ><?php echo language('TCP');?></option>
					<option value="UDP" <?php echo $show_protocol[1];?> ><?php echo language('UDP');?></option>
					<option value="ICMP" <?php echo $show_protocol[2];?> ><?php echo language('ICMP');?></option>
				</select>
			</td>
		</tr>
		
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Port');?>:
					<span class="showhelp">
					<?php echo language('Port help',"Input the range of port.");?>
					</span>
				</div>
			</th>
			<td>
				<input type="text" id="allow_port1" name="allow_port1" width="20%" value="<?php echo $allow_port1;?>"
					oninput="this.value=this.value.replace(/[^\d]*/g,'')" onkeyup="this.value=this.value.replace(/[^\d]*/g,'')"/> :
				<input type="text" id="allow_port2" name="allow_port2" width="20%" value="<?php echo $allow_port2;?>"
					oninput="this.value=this.value.replace(/[^\d]*/g,'')" onkeyup="this.value=this.value.replace(/[^\d]*/g,'')"/>
				<span id="cport"></span>
			
			</td>
		</tr>
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('IP / MASK');?>:
					<span class="showhelp">
					<?php echo language('IP help',"The format of IP is IP/MASK.Confirm the range by IP and mask.");?>
					</span>
				</div>
			</th>
			<td>
				<input id="allow_ip" name="allow_ip" width="20%" value="<?php echo $allow_ip;?>"/> /
				<input id="allow_mask" name="allow_mask" width="20%" value="<?php echo $allow_mask;?>"/>
				<span id="cip"></span>
			
			</td>
		</tr>

		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Actions');?>:
					<span class="showhelp">
					<?php echo language('Actions help',"Choose the firewall action.");?>
					</span>
				</div>
			</th>
			<td >
				<select name="actions" id="actions">
<?php
					$show_actions[0] = '';
					$show_actions[1] = '';
					$show_actions[2] = '';
					switch($actions) {
						case 'ACCEPT': $show_actions[0] = 'selected'; break;
						case 'DROP': $show_actions[1] = 'selected'; break;
						default: $show_actions[0] = 'selected'; break;
					}
?>
					<option value="ACCEPT" <?php echo $show_actions[0];?> ><?php echo language('ACCEPT');?></option>
					<option value="DROP" <?php echo $show_actions[1];?> ><?php echo language('DROP');?></option>
				</select>
			</td>
		</tr>
	</table>
	</script>

	<br />

	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" class="float_btn gen_short_btn"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" <?php if($__demo_enable__=='on'){echo 'disabled';}?> />
	&nbsp;
	<input type=button  value="<?php echo language('Cancel');?>" onclick="window.location.href='<?php echo get_self();?>'" />
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td style="width:50px">
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" <?php if($__demo_enable__=='on'){echo 'disabled';}?> />
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
	$all_rules = get_all_firewall_rules(true);
	$rules_num = 0;
	if($all_rules) {
		foreach($all_rules as $rule) {
			$rules_num+=1;
			$rule['rule_name'] = trim($rule['rule_name']);
			if($rule['rule_name'] == $rule_name){
		?>
				<li><a style="color:#CD3278;" href="<?php echo get_self();?>?sel_rule_name=<?php echo $rule['rule_name']; ?>" ><?php echo $rule['rule_name']; ?></a></li>
		<?php
			}else{
		?>
				<li><a style="color:LemonChiffon4;" href="<?php echo get_self();?>?sel_rule_name=<?php echo $rule['rule_name']; ?>" ><?php echo $rule['rule_name']; ?></a></li>
		<?php
			}
		}
	}
//Control the left navigation hidden or height by '$rules_num'.
	if($rules_num==0){
?>
<script type="text/javascript">
	$("#sort_out").hide();
	$("#sort_info").hide();
</script>
	<?php
	}elseif($rules_num <= 5){
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
		if( (isset($_POST['send']) && ($_POST['send'] == 'New Rule') ) ) {
			//Add new
			if( isset($_POST['sel_rule_name']) && isset($_POST['order']) && $_POST['order'] ) {
				$check_float = 1;
				add_rule_page($_POST['sel_rule_name'],$_POST['order']);
			}
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Save') {
			save_rules();
			show_rules();
		} elseif (isset($_POST['send']) && $_POST['send'] == 'Delete') {
			if(isset($_POST['sel_rule_name']) && $_POST['sel_rule_name']) {
				del_rule($_POST['sel_rule_name']);
				show_rules();
			}
		}
	} else if($_GET) {
		//Modify
		if( isset($_GET['sel_rule_name']) ) {
			$check_float = 1;
			add_rule_page($_GET['sel_rule_name'],'');
		}
	} else {
		show_rules();
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
