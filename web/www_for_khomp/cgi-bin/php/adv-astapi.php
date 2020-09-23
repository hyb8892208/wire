<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/check.inc");
?>

<?php
function save_data()
{
	//get org send & event infomation
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk');
	$hlock=lock_file("/etc/asterisk/manager.conf");
	$manager_conf = $aql->query("select * from manager.conf");
	unlock_file($hlock);
	
	$send_str = '';
	if (isset($manager_conf['send'])) {
		$send_str = "[send]\n";
		if(isset($manager_conf['send']['secret'])) {
			$_val = $manager_conf['send']['secret'];
			$send_str .= "secret=$_val\n";
		} 
		if(isset($manager_conf['send']['read'])) {
			$_val = $manager_conf['send']['read'];
			$send_str .= "read=$_val\n";
		} 
		if(isset($manager_conf['send']['write'])) {
			$_val = $manager_conf['send']['write'];
			$send_str .= "write=$_val\n";
		} 
	} else {
		$send_str = '';
	}
	
	$event_str = '';
	if (isset($manager_conf['event'])) {
		$event_str = "[event]\n";
		if(isset($manager_conf['event']['secret'])) {
			$_val = $manager_conf['event']['secret'];
			$event_str .= "secret=$_val\n";
		} 
		if(isset($manager_conf['event']['read'])) {
			$_val = $manager_conf['event']['read'];
			$event_str .= "read=$_val\n";
		} 
		if(isset($manager_conf['event']['write'])) {
			$_val = $manager_conf['event']['write'];
			$event_str .= "event=$_val\n";
		} 
	} else {
		$event_str = '';
	}
	
	
	if(!isset($_POST['enable_ami'])) {
		$aql = new aql();
		$setok = $aql->set('basedir','/etc/asterisk');
		if (!$setok) {
			echo $aql->get_error();
			return false;
		}

		$manager_conf_path='/etc/asterisk/manager.conf';
		$hlock = lock_file($manager_conf_path);

		if(!$aql->open_config_file($manager_conf_path)){
			echo $aql->get_error();
			unlock_file($hlock);
			return false;
		}

		$exist_array = $aql->query("select * from manager.conf where section='general'");

		if(!isset($exist_array['general'])) {
			$aql->assign_addsection('general','');
		}

		if(isset($exist_array['general']['enabled'])) {
			$aql->assign_editkey('general','enabled','no');
		} else {
			$aql->assign_append('general','enabled','no');
		} 

		if (!$aql->save_config_file('manager.conf')) {
			echo $aql->get_error();
			unlock_file($hlock);
			return false;
		}
		unlock_file($hlock);
		return true;
	}

	$write_str = "[general]\n";
	$write_str .= "bindaddr=0.0.0.0\n";
	$write_str .= "enabled=yes\n";
	
	if(isset($_POST['port']) && $_POST['port'] != '') {
		$_port = trim($_POST['port']);
		$write_str .= "port=$_port\n";
	}

	if(isset($_POST['name']) && $_POST['name'] != '') {
		$_name = trim($_POST['name']);
		$write_str .= "[$_name]\n";

		if(isset($_POST['secret']) && $_POST['secret'] != '') {
			$_secret = trim($_POST['secret']);
			$write_str .= "secret=$_secret\n";
		}

		if(isset($_POST['deny']) && $_POST['deny'] != '') {
			$tmp = trim($_POST['deny']);
			$tmps = explode('&',$tmp);
			if($tmps) {
				foreach($tmps as $each) {
					$_deny = trim($each);
					$write_str .= "deny=$_deny\n";
				}
			}
		}

		if(isset($_POST['permit']) && $_POST['permit'] != '') {
			$tmp = trim($_POST['permit']);
			$tmps = explode('&',$tmp);
			if($tmps) {
				foreach($tmps as $each) {
					$_permit = trim($each);
					$write_str .= "permit=$_permit\n";
				}
			}
		}

		$_read = '';
		if(isset($_POST['system_r'])) {
			$_read .= 'system,';
		}
		if(isset($_POST['call_r'])) {
			$_read .= 'call,';
		}
		if(isset($_POST['log_r'])) {
			$_read .= 'log,';
		}
		if(isset($_POST['verbose_r'])) {
			$_read .= 'verbose,';
		}
		if(isset($_POST['agent_r'])) {
			$_read .= 'agent,';
		}
		if(isset($_POST['user_r'])) {
			$_read .= 'user,';
		}
		if(isset($_POST['config_r'])) {
			$_read .= 'config,';
		}
		if(isset($_POST['dtmf_r'])) {
			$_read .= 'dtmf,';
		}
		if(isset($_POST['reporting_r'])) {
			$_read .= 'reporting,';
		}
		if(isset($_POST['cdr_r'])) {
			$_read .= 'cdr,';
		}
		if(isset($_POST['dialplan_r'])) {
			$_read .= 'dialplan,';
		}
		$_read = rtrim($_read,',');
		$write_str .= "read=$_read\n";

		$_write = '';
		if(isset($_POST['system_w'])) {
			$_write .= 'system,';
		}
		if(isset($_POST['call_w'])) {
			$_write .= 'call,';
		}
		if(isset($_POST['log_w'])) {
			$_write .= 'log,';
		}
		if(isset($_POST['verbose_w'])) {
			$_write .= 'verbose,';
		}
		if(isset($_POST['command_w'])) {
			$_write .= 'command,';
		}
		if(isset($_POST['agent_w'])) {
			$_write .= 'agent,';
		}
		if(isset($_POST['user_w'])) {
			$_write .= 'user,';
		}
		if(isset($_POST['config_w'])) {
			$_write .= 'config,';
		}
		if(isset($_POST['reporting_w'])) {
			$_write .= 'reporting,';
		}
		if(isset($_POST['originate_w'])) {
			$_write .= 'originate,';
		}
		$_write = rtrim($_write,',');
		$write_str .= "write=$_write\n";
		
		if ($_name == 'send') {
			$write_str .= $event_str;
		} else if ($_name == 'event') {
			$write_str .= $send_str;
		} else {
			$write_str .= $send_str;
			$write_str .= $event_str;
		}
	}

	$file_path="/etc/asterisk/manager.conf";
	$hlock=lock_file($file_path);
	$fh = fopen($file_path,"w");
	fwrite($fh,$write_str);
	fclose($fh);
	unlock_file($hlock);

	return true;
}

function ast_reboot()
{
	global $cluster_info;
	global $__BRD_HEAD__;
	global $__BRD_SUM__;
	global $__deal_cluster__;

	echo "<br>";
	$Report = language('Report');
	$Asterisk_Reboot = language('Asterisk Reboot');
	trace_output_start("$Report","$Asterisk_Reboot");
	trace_output_newline();
	ob_flush();
	flush();

	if($cluster_info['mode'] == 'master') {
		if($__deal_cluster__){
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					$data = "syscmd:/etc/init.d/asterisk restart > /dev/null 2>&1\n";
					$ip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
					request_slave($ip, $data, 5, false);
					echo language("Slave");echo ":$ip ";echo language("Asterisk Rebooting");echo " ......<br/>";
					ob_flush();
					flush();
				}
			}
			echo language("Asterisk Rebooting");echo " ......<br/>";
		}
	}else{
		echo language("Asterisk Rebooting");echo " ......<br/>";
	}
	exec("/etc/init.d/asterisk restart > /dev/null 2>&1 || echo $?",$output);

	$Result = language('Result');
	trace_output_newhead("$Result");
	if(!$output) {
		echo language("Asterisk Reboot Succeeded");
	} else {
		echo language("Asterisk Reboot Failed");echo "($output[0])";
	}

	trace_output_end();
}

if($_POST && isset($_POST['send'])) {
	if($_POST['send'] == 'Save'){
		if(save_data()) {
			//manager_reload();
			wait_apply("exec", "asterisk -rx \"manager reload\" > /dev/null 2>&1 &");
		}
	}else if($_POST['send'] == 'Asterisk Reboot') {
		ast_reboot();
		
		save_user_record("","SYSTEM->Tools:Asterisk Reboot");
	}
}


//Get Configs
//////////////////////////////////////////////////////////////////////////////////
$aql = new aql();
$setok = $aql->set('basedir','/etc/asterisk');
$hlock=lock_file("/etc/asterisk/manager.conf");
$manager_conf = $aql->query("select * from manager.conf");
unlock_file($hlock);

$enable_ami = '';
if(isset($manager_conf['general']['enabled'])) {
	$val = trim($manager_conf['general']['enabled']);
	if(strcasecmp($val,"yes") == 0) {
		$enable_ami = 'checked';
	}
}

if(isset($manager_conf['general']['port'])) {
	$port = trim($manager_conf['general']['port']);
} else {
	$port = '';
}

$name = '';
$secret = '';
$deny = '';
$permit = '';

$read['system'] = '';
$read['call'] = '';
$read['log'] = '';
$read['verbose'] = '';
$read['command'] = '';
$read['agent'] = '';
$read['user'] = '';
$read['config'] = '';
$read['dtmf'] = '';
$read['reporting'] = '';
$read['cdr'] = '';
$read['dialplan'] = '';
$read['originate'] = '';
$read['all'] = '';

$write['system'] = '';
$write['call'] = '';
$write['log'] = '';
$write['verbose'] = '';
$write['command'] = '';
$write['agent'] = '';
$write['user'] = '';
$write['config'] = '';
$write['dtmf'] = '';
$write['reporting'] = '';
$write['cdr'] = '';
$write['dialplan'] = '';
$write['originate'] = '';
$write['all'] = '';

if(is_array($manager_conf)) {
	foreach($manager_conf as $section => $value) {
		if(is_array($value) && count($value) && $section != 'general') {
			$name = $section;
			if(isset($value['secret'])) {
				$secret = trim($value['secret']);
			}

			$deny = '';
			if(isset($value['deny'])) {
				$deny = trim($value['deny']);
			}
			for($i=1;isset($value["deny[$i]"]);$i++) {
				$deny .= '&'.trim($value["deny[$i]"]);
			}

			$permit = '';
			if(isset($value['permit'])) {
				$permit = trim($value['permit']);
			}
			for($i=1;isset($value["permit[$i]"]);$i++) {
				$permit .= '&'.trim($value["permit[$i]"]);
			}

			if(isset($value['read'])) {
				$tmp = trim($value['read']);
				$array = explode(',', $tmp);
				foreach($array as $each) {
					$val = trim($each);
					if($val != '') {
						$read[$val] = 'checked';
					}
				}
			}
			if( $read['system'] == 'checked' &&
				$read['call'] == 'checked' &&
				$read['log'] == 'checked' &&
				$read['verbose'] == 'checked' &&
				$read['agent'] == 'checked' &&
				$read['user'] == 'checked' &&
				$read['config'] == 'checked' &&
				$read['dtmf'] == 'checked' &&
				$read['reporting'] == 'checked' &&
				$read['cdr'] == 'checked' &&
				$read['dialplan'] == 'checked'
			) {
				$read['all'] = 'checked';
			}

			if(isset($value['write'])) {
				$tmp = trim($value['write']);
				$array = explode(',', $tmp);
				foreach($array as $each) {
					$val = trim($each);
					if($val != '') {
						$write[$val] = 'checked';
					}
				}
			}
			if( $write['system'] == 'checked' &&
				$write['call'] == 'checked' &&
				$write['log'] == 'checked' &&
				$write['verbose'] == 'checked' &&
				$write['command'] == 'checked' &&
				$write['agent'] == 'checked' &&
				$write['user'] == 'checked' &&
				$write['config'] == 'checked' &&
				$write['reporting'] == 'checked' &&
				$write['originate'] == 'checked'
			) {
				$write['all'] = 'checked';
			}

			break;
		}
	}
}
?>


<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>

<script type="text/javascript">

function selAllRead(value)
{
	document.getElementById("system_r").checked = value;
	document.getElementById("call_r").checked = value;
	document.getElementById("log_r").checked = value;
	document.getElementById("verbose_r").checked = value;
	//document.getElementById("command_r").checked = value;
	document.getElementById("agent_r").checked = value;
	document.getElementById("user_r").checked = value;
	document.getElementById("config_r").checked = value;
	document.getElementById("dtmf_r").checked = value;
	document.getElementById("reporting_r").checked = value;
	document.getElementById("cdr_r").checked = value;
	document.getElementById("dialplan_r").checked = value;
	//document.getElementById("originate_r").checked = value;
}

function selAllWrite(value)
{
	document.getElementById("system_w").checked = value;
	document.getElementById("call_w").checked = value;
	document.getElementById("log_w").checked = value;
	document.getElementById("verbose_w").checked = value;
	document.getElementById("command_w").checked = value;
	document.getElementById("agent_w").checked = value;
	document.getElementById("user_w").checked = value;
	document.getElementById("config_w").checked = value;
	//document.getElementById("dtmf_w").checked = value;
	document.getElementById("reporting_w").checked = value;
	//document.getElementById("cdr_w").checked = value;
	//document.getElementById("dialplan_w").checked = value;
	document.getElementById("originate_w").checked = value;
}

function check()
{
	var is_check = false;
	
	if(!document.getElementById("enable_ami").checked){
		return true;
	}

	//var port = document.getElementById("port").value;
	var name = document.getElementById("name").value;
	var secret = document.getElementById("secret").value;
	var deny = document.getElementById("deny").value;
	var permit = document.getElementById("permit").value;

	/*if(!check_networkport(port)) {
		document.getElementById("cport").innerHTML = con_str('<?php echo language('js check networkport','Please input valid port number (1-65535)');?>');
		return false;
	} else {
		document.getElementById("cport").innerHTML = '';
	}
	*/
	var rex = /^(?![a-zA-z]+$)(?!\d+$)(?![!@#$%^&*]+$)(?![a-zA-z\d]+$)(?![a-zA-z!@#$%^&*]+$)(?![\d!@#$%^&*]+$)[a-zA-Z\d!@#$%^&*]{8,32}$/;

	document.getElementById("cname").innerHTML = '';
	if(!check_diyname(name)) {
		document.getElementById('name').focus();
		document.getElementById("cname").innerHTML = con_str('<?php echo htmlentities(language('js check diyname','Allowed character must be any of [-_+.<>&0-9a-zA-Z],1 - 32 characters.'));?>');
		is_check = true;
	} else {
		if(name != 'send' && name != 'event' && name != 'general'){
			document.getElementById("cname").innerHTML = '';
		} else {
			document.getElementById('name').focus();
			document.getElementById("cname").innerHTML = con_str('<?php echo htmlentities(language('js limit aminame', 'limit the input string to "send", "event", "general". '))?>')
			is_check = true;
		}
	}
	
	if(!rex.test(secret)){
		document.getElementById('secret').focus();
		document.getElementById("csecret").innerHTML = con_str('<?php echo htmlentities(language('js check the password complexity','The password you enter must consist of letters, numbers, and special characters, \"0-9a-zA-Z!@#$%^&*\".Length: 8-32 characters.'));?>');
		is_check = true;
	} else {
		document.getElementById("csecret").innerHTML = '';
	}
	/*
	if(!check_diypwd(secret)) {
		document.getElementById("csecret").innerHTML = con_str('<?php echo htmlentities(language('js check diypwd','Allowed character must be any of [0-9a-zA-Z`~!@$%^&*()_+{}|<>?-=[],./],4 - 32 characters.'));?>');
		return false;
	} else {
		document.getElementById("csecret").innerHTML = '';
	}
	*/
	
	if(is_check){
		return false;
	}
	
	return true;
}

function enable_ami_change()
{
	var sw = document.getElementById('enable_ami').checked;

	if(sw) {
		set_visible('field_manager', true);
		set_visible('field_rights', true);
		set_visible('field_port', true);
		set_visible('field_warning',false);

		//obj = document.getElementById('port');
		//obj.disabled = false;
	} else {
		set_visible('field_warning',true);
		set_visible('field_manager', false);
		set_visible('field_rights', false);
		set_visible('field_port', false);

		//obj = document.getElementById('port');
		//obj.disabled = true;
	}
}

function onload_func()
{
	enable_ami_change();
}
</script>

<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<div class="content">
		<span class="title"><?php echo language('Asterisk Reboot');?></span>

		<div class="tab_item">
			<span>
				<?php echo language('Asterisk Reboot help','Reboot the asterisk and all the current calls will be dropped.');?>
			</span>
			<div class="tab_item_right">
				<input type="submit" value="<?php echo language('Asterisk Reboot');?>" <?php if($__demo_enable__=='on'){echo 'disabled';}?> 
					onclick="document.getElementById('send').value='Asterisk Reboot';return confirm('<?php echo language('Asterisk Reboot confirm','Are you sure to reboot Asterisk now?');?>')"/>
			</div>
		</div>
	</div>

	<div class="content">
		<span class="title">
			<?php echo language('General');?>
		</span>
		
		<div class="tab_item">
			<span><?php echo language('Enable');?>:</span>
			<div class="tab_item_right">
				<span><input type="checkbox" name="enable_ami" id="enable_ami" onchange="enable_ami_change()" <?php echo $enable_ami?> ></span>
			</div>
		</div>
		
		<div class="tab_item" style="display: none;">
			<span>
				<?php echo language('Warning');?>:
			</span>
			<div class="tab_item_right">
				<input id="warning" type="hidden" name="warning" value=""> <font color=ff0000> 
				<?php echo language('Warning help','*Asterisk API is closed.Please note that this operation may lead to serious consequences on calling and sending sms.');?></font>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Port');?>:
			</span>
			<div class="tab_item_right">
				<input id="port" type="hidden" name="port" value="5038"> <span>5038</span>
			</div>
		</div>
	</div>

	<div class="content">
		<span class="title">
			<?php echo language('Manager');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Manager Name')){ ?>
							<b><?php echo language('Manager Name');?>:</b><br/>
							<?php echo language('Manager Name help','Name of the manager without space.');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Manager secret')){ ?>
							<b><?php echo language('Manager secret');?>:</b><br/>
							<?php echo language('Manager secret help',"
								Password for the manager.<br/>
								The password you enter must consist of letters, numbers, and special characters \"0-9a-zA-Z!@#$%^&*\".Length: 8-32 characters.");
							?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Deny')){ ?>
							<b><?php echo language('Deny');?>:</b><br/>
							<?php echo language('Deny help','
								If you want to deny many hosts or networks, use char & as separator.<br/>
								Example: 0.0.0.0/0.0.0.0 or 192.168.1.0/255.255.255.0&10.0.0.0/255.0.0.0');
							?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Permit')){ ?>
							<b><?php echo language('Permit');?>:</b><br/>
							<?php echo language('Permit help','
								If you want to permit many hosts or networks, use char & as separator.<br/>
								Example: 0.0.0.0/0.0.0.0 or 192.168.1.0/255.255.255.0&10.0.0.0/255.0.0.0');
							?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Manager Name');?>:
			</span>
			<div class="tab_item_right">
				<span id="cname"></span>
				<input id="name" type="text" name="name" value="<?php echo $name?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Manager secret');?>:
			</span>
			<div class="tab_item_right">
				<span id="csecret"></span>
				<input id="secret" type="text" name="secret" value="<?php echo $secret?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Deny');?>:
			</span>
			<div class="tab_item_right">
				<input id="deny" type="text" name="deny" value="<?php echo $deny?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Permit');?>:
			</span>
			<div class="tab_item_right">
				<input id="permit" type="text" name="permit" value="<?php echo $permit?>" />
			</div>
		</div>
	</div>

	<div class="content">
		<span class="title">
			<?php echo language('Rights');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('System')){ ?>
							<b><?php echo language('System');?>:</b><br/>
							<?php echo language('System help','
								General information about the system and ability to run system management commands, <br/>
								such as Shutdown, Restart, and Reload.');
							?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Call')){ ?>
							<b><?php echo language('Call');?>:</b><br/>
							<?php echo language('Call help','Information about channels and ability to set information in a running channel.');?>

							<br/><br/>
						<?php } ?>

						<?php if(is_show_language_help('Log')){ ?>
							<b><?php echo language('Log');?>:</b><br/>
							<?php echo language('Log help','Logging information.  Read-only. (Defined but not yet used.)');?>

							<br/><br/>
						<?php } ?>

						<?php if(is_show_language_help('Verbose')){ ?>
							<b><?php echo language('Verbose');?>:</b><br/>
							<?php echo language('Verbose help','Verbose information.  Read-only. (Defined but not yet used.)');?>

							<br/><br/>
						<?php } ?>

						<?php if(is_show_language_help('Command')){ ?>
							<b><?php echo language('Command');?>:</b><br/>
							<?php echo language('Command help','Permission to run CLI commands.  Write-only.');?>

							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Agent')){ ?>
							<b><?php echo language('Agent');?>:</b><br/>
							<?php echo language('Agent help','Information about queues and agents and ability to add queue members to a queue.');?>

							<br/><br/>
						<?php } ?>

						<?php if(is_show_language_help('User')){ ?>
							<b><?php echo language('User');?>:</b><br/>
							<?php echo language('User help','Permission to send and receive UserEvent.');?>

							<br/><br/>
						<?php } ?>
							
						<?php if(is_show_language_help('Config')){ ?>
							<b><?php echo language('Config');?>:</b><br/>
							<?php echo language('Config help','Ability to read and write configuration files.');?>

							<br/><br/>
						<?php } ?>

						<?php if(is_show_language_help('DTMF')){ ?>
							<b><?php echo language('DTMF');?>:</b><br/>
							<?php echo language('DTMF help','Receive DTMF events.  Read-only.');?>

							<br/><br/>
						<?php } ?>

						<?php if(is_show_language_help('Reporting')){ ?>
							<b><?php echo language('Reporting');?>:</b><br/>
							<?php echo language('Reporting help','Ability to get information about the system.');?>

							<br/><br/>
						<?php } ?>

						<?php if(is_show_language_help('CDR')){ ?>
							<b><?php echo language('CDR');?>:</b><br/>
							<?php echo language('CDR help','Output of cdr_manager, if loaded.  Read-only.');?>

							<br/><br/>
						<?php } ?>

						<?php if(is_show_language_help('Dialplan')){ ?>
							<b><?php echo language('Dialplan');?>:</b><br/>
							<?php echo language('Dialplan help','Receive NewExten and VarSet events.  Read-only.');?>

							<br/><br/>
						<?php } ?>

						<?php if(is_show_language_help('Originate')){ ?>
							<b><?php echo language('Originate');?>:</b><br/>
							<?php echo language('Originate help','Permission to originate new calls.  Write-only.');?>

							<br/><br/>
						<?php } ?>

						<?php if(is_show_language_help('All')){ ?>
							<b><?php echo language('All');?>:</b><br/>
							<?php echo language('All help','Select all or deselect all.');?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('System');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('read');?>:
				<input type=checkbox name="system_r" id="system_r" <?php echo $read['system']?> >
				&nbsp;
				&nbsp;
				&nbsp;
				&nbsp;
				<?php echo language('write');?>:
				<input type=checkbox name="system_w" id="system_w" <?php echo $write['system']?> >
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Call');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('read');?>:
				<input type=checkbox name="call_r" id="call_r" <?php echo $read['call']?> >
				&nbsp;
				&nbsp;
				&nbsp;
				&nbsp;
				<?php echo language('write');?>:
				<input type=checkbox name="call_w" id="call_w" <?php echo $write['call']?> >
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Log');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('read');?>:
				<input type=checkbox name="log_r" id="log_r" <?php echo $read['log']?>>
				&nbsp;
				&nbsp;
				&nbsp;
				&nbsp;
				<?php echo language('write');?>:
				<input type=checkbox name="log_w" id="log_w" <?php echo $write['log']?> >
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Verbose');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('read');?>:
				<input type=checkbox name="verbose_r" id="verbose_r" <?php echo $read['verbose']?>>
				&nbsp;
				&nbsp;
				&nbsp;
				&nbsp;
				<?php echo language('write');?>:
				<input type=checkbox name="verbose_w" id="verbose_w" <?php echo $write['verbose']?> >
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Command');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('read');?>:
				<input type=checkbox name="command_r" id="command_r" disabled>
				&nbsp;
				&nbsp;
				&nbsp;
				&nbsp;
				<?php echo language('write');?>:
				<input type=checkbox name="command_w" id="command_w" <?php echo $write['command']?> >
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Agent');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('read');?>:
				<input type=checkbox name="agent_r" id="agent_r" <?php echo $read['agent']?> >
				&nbsp;
				&nbsp;
				&nbsp;
				&nbsp;
				<?php echo language('write');?>:
				<input type=checkbox name="agent_w" id="agent_w" <?php echo $write['agent']?> >
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('User');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('read');?>:
				<input type=checkbox name="user_r" id="user_r" <?php echo $read['user']?> >
				&nbsp;
				&nbsp;
				&nbsp;
				&nbsp;
				<?php echo language('write');?>:
				<input type=checkbox name="user_w" id="user_w" <?php echo $write['user']?> >
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Config');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('read');?>:
				<input type=checkbox name="config_r" id="config_r" <?php echo $read['config']?> >
				&nbsp;
				&nbsp;
				&nbsp;
				&nbsp;
				<?php echo language('write');?>:
				<input type=checkbox name="config_w" id="config_w" <?php echo $write['config']?> >
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('DTMF');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('read');?>:
				<input type=checkbox name="dtmf_r" id="dtmf_r" <?php echo $read['dtmf']?> >
				&nbsp;
				&nbsp;
				&nbsp;
				&nbsp;
				<?php echo language('write');?>:
				<input type=checkbox name="dtmf_w" id="dtmf_w" disabled>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Reporting');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('read');?>:
				<input type=checkbox name="reporting_r" id="reporting_r" <?php echo $read['reporting']?> >
				&nbsp;
				&nbsp;
				&nbsp;
				&nbsp;
				<?php echo language('write');?>:
				<input type=checkbox name="reporting_w" id="reporting_w" <?php echo $write['reporting']?> >
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('CDR');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('read');?>:
				<input type=checkbox name="cdr_r" id="cdr_r" <?php echo $read['cdr']?> >
				&nbsp;
				&nbsp;
				&nbsp;
				&nbsp;
				<?php echo language('write');?>:
				<input type=checkbox name="cdr_w" id="cdr_w" disabled>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Dialplan');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('read');?>:
				<input type=checkbox name="dialplan_r" id="dialplan_r" <?php echo $read['dialplan']?> >
				&nbsp;
				&nbsp;
				&nbsp;
				&nbsp;
				<?php echo language('write');?>:
				<input type=checkbox name="dialplan_w" id="dialplan_w" disabled>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Originate');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('read');?>:
				<input type=checkbox name="originate_r" id="originate_r" disabled>
				&nbsp;
				&nbsp;
				&nbsp;
				&nbsp;
				<?php echo language('write');?>:
				<input type=checkbox name="originate_w" id="originate_w" <?php echo $write['originate']?> >
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('All');?>:
			</span>
			<div class="tab_item_right">
				<?php echo language('read');?>:
				<input type=checkbox name="all_r" id="all_r" <?php echo $read['all']?> onclick="selAllRead(this.checked)" >
				&nbsp;
				&nbsp;
				&nbsp;
				&nbsp;
				<?php echo language('write');?>:
				<input type=checkbox name="all_w" id="all_w" <?php echo $write['all']?> onclick="selAllWrite(this.checked)" >
			</div>
		</div>
	</div>
	
	<input type="hidden" name="send" id="send" value="" />
	
	<div id="button_save">
		<button type="submit" onclick="document.getElementById('send').value='Save';return check();" ><?php echo language('Save');?></button>
	</div>
</form>



<script type="text/javascript"> 
$("#show_password").change(function(){
	if($(this).attr("checked") == 'checked'){
		$("#secret").prop("type","text");
	}else{
		$("#secret").prop("type","password");
	}
});

$(document).ready(function (){ 
	onload_func();
}); 
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>