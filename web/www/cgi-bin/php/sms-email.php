<?php require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");
include_once("/www/cgi-bin/inc/define.inc");
?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />

<?php
function show_email(){
	global $__GSM_SUM__;
	
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$res = $aql->query("select * from email.conf");
	
?>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="get">
		<div id="tab">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('Email');?></li>
			<li class="tb2">&nbsp;</li>
		</div>
		
		<table width="100%" class="tshow">
			<tr>
				<th width=""><?php echo language('Port');?></th>
				<th><?php echo language('Enable');?></th>
				<th><?php echo language('SMTP Server');?></th>
				<th width="50px"><?php echo language('Actions');?></th>
			</tr>
			
<?php
			for($i=1;$i<=$__GSM_SUM__;$i++){
				$channel_name = get_gsm_name_by_channel($i);
				if(strstr($channel_name,'null')) continue;
				if(isset($res[$i]['sw']) && $res[$i]['sw'] != ''){
					$sw = $res[$i]['sw'];
				}else{
					$sw = 'off';
				}
				
?>
			<tr>
				<td><?php echo $channel_name;?></td>
				<td><?php echo strtoupper($sw);?></td>
				<td><?php echo strtoupper($res[$i]['smtpserversel']);?></td>
				<td style="text-align:center;">
					<button type="submit" value="Modify" style="width:32px;height:32px;" onclick="document.getElementById('send').value='Edit';document.getElementById('channel_sel').value='<?php echo $i;?>'">
						<img src="/images/edit.gif">
					</button>
				</td>
			</tr>
<?php 
			}
?>
		
			<input type="hidden" name="send" id="send" value="" />
			<input type="hidden" name="channel_sel" id="channel_sel" value="" />
		</table>
	</form>

<?php
}

function edit_email(){
	global $__GSM_SUM__;
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$channel = $_GET['channel_sel'];
	$res = $aql->query("select * from email.conf where section='$channel'");
	
	$lang_sync_title = language('Synchronization option');
	
	$mail_sw = '';
	if(isset($res[$channel]['sw'])) {
		if(is_true(trim($res[$channel]['sw']))){
			$mail_sw = 'checked';
		}
	}
	
	
	if(isset($res[$channel]['smtpserversel'])){
		$smtpserversel = $res[$channel]['smtpserversel'];
	}else{
		$smtpserversel = 'other';
	}
	
	if(isset($res[$channel]['sender'])) {
		$sender=trim($res[$channel]['sender']);
	} else {
		$sender="";
	}
	
	if(isset($res[$channel]['smtpserver'])) {
		$smtpserver=trim($res[$channel]['smtpserver']);
	} else {
		$smtpserver="";
	}
	
	if(isset($res[$channel]['smtpport'])) {
		$smtpport=trim($res[$channel]['smtpport']);
	} else {
		$smtpport="";
	}
	
	if(isset($res[$channel]['smtpuser'])) {
		$smtpuser=trim($res[$channel]['smtpuser']);
	} else {
		$smtpuser="";
	}
	
	if(isset($res[$channel]['smtppwd'])) {
		$smtppwd=trim($res[$channel]['smtppwd']);
	} else {
		$smtppwd="";
	}
	
	if(isset($res[$channel]['tls_enable']) && $res[$channel]['tls_enable']=='yes') {
		$tls_enable = 'checked';
	} else {
		$tls_enable = '';
	}
	
	if(isset($res[$channel]['smail1'])) {
		$smail1=trim($res[$channel]['smail1']);
	} else {
		$smail1="";
	}
	
	if(isset($res[$channel]['smail2'])) {
		$smail2=trim($res[$channel]['smail2']);
	} else {
		$smail2="";
	}
	
	if(isset($res[$channel]['smail3'])) {
		$smail3=trim($res[$channel]['smail3']);
	} else {
		$smail3="";
	}
	
	if(isset($res[$channel]['mail_title'])) {
		$mail_title=trim($res[$channel]['mail_title']);
	} else {
		$mail_title="";
	}
	
	if(isset($res[$channel]['mail_content'])) {
		$mail_content=trim($res[$channel]['mail_content']);
	} else {
		$mail_content="";
	}
?>
	<script type="text/javascript" src="/js/check.js?v=1.1"></script>
	<script type="text/javascript" src="/js/functions.js"></script>
	
	<form id="manform" enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
		<input type="hidden" name="channel" value="<?php echo $channel;?>" />
		<div id="tab">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('Port');?> <?php echo get_gsm_name_by_channel($channel);?></li>
			<li class="tb2">&nbsp;</li>
		</div>
		
		<table width="100%" class="tedit" >
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Enable');?>:
						<span class="showhelp">
						<?php echo language('Enable help','ON(enabled),OFF(disabled)');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" class="setting_sync" name="sw_sync" title="<?php echo $lang_sync_title;?>" />
					<input type="checkbox" id="mail_sw" name="sw" <?php echo $mail_sw; ?> />
				</td>
			</tr>
			
			<tbody id="smtp_content">
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('SMTP Server');?>:
							<span class="showhelp">
							<?php echo language('SMTP Server help');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="smtpserversel_sync" title="<?php echo $lang_sync_title;?>" />
						<select name="smtpserversel" id="smtpserversel" onchange="setSMTPServer(this.value)">
							<option value="other" <?php if($smtpserversel == 'other') echo 'selected';?>>OTHER</option>
							<option value="gmail" <?php if($smtpserversel == 'gmail') echo 'selected';?>>GMAIL</option>
							<option value="hotmail" <?php if($smtpserversel == 'hotmail') echo 'selected';?>>HOTMAIL</option>
							<option value="yahoo" <?php if($smtpserversel == 'yahoo') echo 'selected';?>>YAHOO</option>
						</select>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Email Address of Sender');?>:
							<span class="showhelp">
							<?php echo language('Email Address of Sender help','To set the email address of an available email account.<br/> For example, support@openvox.cn');?>
							</span>
						</div>
					</th>
					<td >
						<input type="checkbox" class="setting_sync" name="sender_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="text" name="sender" id="sender" style="width: 250px;" value="<?php echo $sender;?>" /><span id="csender"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Domain');?>:
							<span class="showhelp">
							<?php echo language('Domain help','To set outgoing mail server. <br/> e.g. mail.openvox.cn');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="smtpserver_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="text" name="smtpserver" id="smtpserver" style="width: 250px;" value="<?php echo $smtpserver;?>" /><span id="csmtpserver"></span>
				    </td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('SMTP Port');?>(<?php echo language('default');?> 25):
							<span class="showhelp">
							<?php echo language('SMTP Port help','To set port number of outgoing mail server. (Default is 25.)');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="smtpport_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="text" name="smtpport" id="smtpport" style="width: 250px;" value="<?php echo $smtpport;?>" /><span id="csmtpport"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('SMTP User Name');?>:
							<span class="showhelp">
							<?php
								$help = "The login name of your existing email account. <br/>"
									."This option might be different from your email address. <br/>"
									."Some email client doesn't need the email postfix.";
								echo language('SMTP User Name help',$help);
							?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="smtpuser_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="text" name="smtpuser" id="smtpuser" style="width: 250px;" value="<?php echo $smtpuser;?>" /><span id="csmtpuser"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('SMTP Password');?>:
							<span class="showhelp">
							<?php echo language('SMTP Password help','The password to login your existing email.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="smtppwd_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="password" name="smtppwd" id="smtppwd" style="width: 250px;" value="<?php echo $smtppwd;?>" />
						<span style="vertical-align:middle;display:inline-block;"><input type="checkbox" id="show_password" /></span>
						<span id="csmtppwd"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('TLS Enable');?>:
							<span class="showhelp">
							<?php echo language('TLS Enable help');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="tls_enable_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="checkbox" name="tls_enable" id="tls_enable" <?php echo $tls_enable;?> />
						<?php echo language('TLS Enable help','This option allows the authentication with certificates.');?>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Destination Email Address');?> 1:
							<span class="showhelp">
							<?php echo language('Destination Email Address 1 help','The first email address to receive the inbox message.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="smail1_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="text" name="smail1" id="smail1" style="width: 250px;" value="<?php echo $smail1;?>" /><span id="csmail1"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Destination Email Address');?> 2:
							<span class="showhelp">
							<?php echo language('Destination Email Address 2 help','The second email address to receive the inbox message.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="smail2_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="text" name="smail2" id="smail2"  style="width: 250px;" value="<?php echo $smail2;?>" /><span id="csmail2"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Destination Email Address');?> 3:
							<span class="showhelp">
							<?php echo language('Destination Email Address 3 help','The third email address to receive the inbox message.');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="smail3_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="text" name="smail3" id="smail3" style="width: 250px;" value="<?php echo $smail3;?>" /><span id="csmail3"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Title');?>:
							<span class="showhelp">
							<?php echo language('Title help','
								Available variable: <br/>
								$PHONENUMBER:SMS sender number. <br/>
								$PORT:SMS from which port.<br/>
								$TIME:SMS received time.<br/>
								$MESSAGE:SMS content.<br/>');
							?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="mail_title_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="text" name="mail_title" id="mail_title" style="width: 500px;" value="<?php echo $mail_title;?>" /><span id="cmail_title"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Content');?>:
							<span class="showhelp">
							<?php echo language('Content help','
								Available variable: <br/>
								$PHONENUMBER:SMS sender number. <br/>
								$PORT:SMS from which port.<br/>
								$TIME:SMS received time.<br/>
								$MESSAGE:SMS content.<br/>');
							?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" class="setting_sync" name="mail_content_sync" title="<?php echo $lang_sync_title;?>" />
						<input type="text" name="mail_content" id="mail_content" style="width: 500px;" value="<?php echo $mail_content;?>" /><span id="cmail_content"></span>
					</td>
				</tr>
			</tbody>
		</table>
		
		<br>

		<div id="tab" class="div_tab_title">
			<li class="tb_fold" onclick="lud(this,'save_to_other_ports')" id="save_to_other_ports_li">&nbsp;</li>
			<li class="tbg_fold" onclick="lud(this,'save_to_other_ports')"><?php echo language('Save To Other Ports');?></li>
			<li class="tb2_fold" onclick="lud(this,'save_to_other_ports')">&nbsp;</li>
			<li class="tb_end2">&nbsp;</li>
		</div>
		
		<div id="save_to_other_ports" style="display:none">
			<table width="100%" class="tedit" >
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('Save To Other Ports');?>:
							<span class="showhelp">
							<?php echo language('PhoneNumber Save To Other Ports help');?>
							</span>
						</div>
					</th>
					<td>
						<table cellpadding="0" cellspacing="0" class="port_table">
<?php
							for($i=1;$i<=$__GSM_SUM__;$i++){
								$port_name = get_gsm_name_by_channel($i);
								if(strstr($port_name, 'null')) continue;
								if($i==$channel){
									$checked = 'checked';
									$disabled = 'disabled';
								}else{
									$checked = '';
									$disabled = '';
								}
								echo "<td class='module_port'><input type='checkbox' name='spans[1][$i]' class='port' $checked $disabled>";
								echo $port_name;
								echo '</td>';
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
							<?php echo language('Sync All Settings');?>:
							<span class="showhelp">
							<?php echo language('PhoneNumber Sync All Settings help');?>
							</span>
						</div>
					</th>
					<td>
						<input type="checkbox" id="all_settings_sync" onclick="selectAllCheckbox(this.checked,'class','setting_sync');" checked disabled />
						<?php echo language('Select all settings');?>
					</td>
				</tr>
			</table>	
		</div>
		
		<br/>
		<input type="hidden" name="send" id="send" value="" />
		<table id="float_btn" class="float_btn">
			<tr id="float_btn_tr" class="float_btn_tr" style="padding-left: 15px;">
				<td>
					<input type="submit"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>
				</td>
				<td>
					<input type="button" value="<?php echo language('Cancel');?>" onclick="window.location.href='<?php echo get_self();?>'" />
				</td>
			</tr>
		</table>
	</form>

	<div id="sort_out" class="sort_out"></div>
	<div class="sort_gsm_cli" id="sort_gsm_cli">
		<div id="sort_info" class="sort_info" style="display:block">
		<?php 
		for($c=1; $c<=$__GSM_SUM__; $c++){
			$channel_name = get_gsm_name_by_channel($c);
			if(strstr($channel_name,'null')) continue;
		?>
			<li>
				<a style="<?php if($c==$channel){echo 'color:#CD3278;';}else{echo 'color:LemonChiffon4;';}?>" href="/cgi-bin/php/sms-email.php?channel_sel=<?php echo $c; ?>&send=Edit" >
					<?php echo $channel_name;?>
				</a>
			</li>
		<?php 
		}
		?>
		</div>
	</div>
	
	<script>
	$(function(){
		$("#mail_sw").iButton();
		
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
		
		if(document.getElementById('mail_sw').checked){
			$("#smtp_content").show();
		}else{
			$("#smtp_content").hide();
		}
		
		$(".port").click(function(){
			handle_port_sync();
		});
		
		$(".setting_sync").click(function(){
			handle_setting_sync();
		});
	});
	
	$("#mail_sw").change(function(){
		if($(this).prop("checked")){
			$("#smtp_content").show();
		}else{
			$("#smtp_content").hide();
		}
	});
	
	function handle_port_sync(){
		if(isAllCheckboxChecked('class','port')){
			$("#all_port").attr({"checked":true});
		}else{
			$("#all_port").attr({"checked":false});
		}
		if(isCheckboxChecked('class','port')){
			$(".setting_sync").show();
			$("#all_settings_sync").attr({"disabled":false,"checked":true});
			selectAllCheckbox(true,'class','setting_sync');
		}else{
			$(".setting_sync").hide();
			$("#all_settings_sync").attr({"disabled":true,"checked":true});
			selectAllCheckbox(false,'class','setting_sync');
		}
	}
	
	function handle_setting_sync(){
		if(isAllCheckboxChecked('class','setting_sync')){
			$("#all_settings_sync").attr({"checked":true});
		}else{
			$("#all_settings_sync").attr({"checked":false});
		}
	}
	
	function check(){
		var mail_sw = document.getElementById("mail_sw").checked;
		
		var sender = document.getElementById("sender").value;
		var smtpserver = document.getElementById("smtpserver").value;
		var smtpport = document.getElementById("smtpport").value;
		var smtpuser = document.getElementById("smtpuser").value;
		var smtppwd = document.getElementById("smtppwd").value;
		var smail1 = document.getElementById("smail1").value;
		var smail2 = document.getElementById("smail2").value;
		var smail3 = document.getElementById("smail3").value;
		
		if(mail_sw) {
			if(!check_email(sender)) {
				document.getElementById("csender").innerHTML = con_str('<?php echo language('js check email','Please input a valid email address');?>');
				return false;
			} else {
				document.getElementById("csender").innerHTML = '';
			}

			if(!check_domain(smtpserver)) {
				document.getElementById("csmtpserver").innerHTML = con_str('<?php echo language('js check domain','Invalid domain or IP address.');?>');
				return false;
			} else {
				document.getElementById("csmtpserver").innerHTML = '';
			}

			if(smtpport != '') { //Default 25
				if(!check_networkport(smtpport)) {
					document.getElementById("csmtpport").innerHTML = con_str('<?php echo language('js check networkport','Please input valid port number (1-65535)');?>');
					return false;
				} else {
					document.getElementById("csmtpport").innerHTML = '';
				}
			}

			if(!check_smtpuser(smtpuser)) {
				document.getElementById("csmtpuser").innerHTML = con_str('<?php echo language('js check smtpuser','Please input a valid STMP user name');?>');
				return false;
			} else {
				document.getElementById("csmtpuser").innerHTML = '';
			}

			if(!check_smtppwd(smtppwd)) {
				document.getElementById("csmtppwd").innerHTML = con_str('<?php echo language('js check smtppwd','Allowed character must be any of [0-9a-zA-Z`~!@#$%^*()_{}:|?-=.], 1-64 characters.');?>');
				return false;
			} else {
				document.getElementById("csmtppwd").innerHTML = '';
			}

			if(smail1 == '' && smail2 == '' && smail3 == '') {
				document.getElementById("csmail1").innerHTML = con_str('<?php echo language('js check smail','You must set a email address.');?>');
				return false;
			}

			if(smail1 != '') {
				if(!check_email(smail1)) {
					document.getElementById("csmail1").innerHTML = con_str('<?php echo language('js check email','Please input a valid email address');?>');
					return false;
				} else {
					document.getElementById("csmail1").innerHTML = '';
				}
			}

			if(smail2 != '') {
				if(!check_email(smail2)) {
					document.getElementById("csmail2").innerHTML = con_str('<?php echo language('js check email','Please input a valid email address');?>');
					return false;
				} else {
					document.getElementById("csmail2").innerHTML = '';
				}
			}

			if(smail3 != '') {
				if(!check_email(smail3)) {
					document.getElementById("csmail3").innerHTML = con_str('<?php echo language('js check email','Please input a valid email address');?>');
					return false;
				} else {
					document.getElementById("csmail3").innerHTML = '';
				}
			}
		}
		
		return true;
	}

	function float_sort_hide(){
		$("#sort_gsm_cli").stop().animate({left:"-198px"}, 300);
		$("#sort_out").stop().animate({left:"0px"}, 300);
	}
	
	function float_sort_on(){
		$("#sort_gsm_cli").animate({left:"0px"}, 300);
		$("#sort_out").animate({left:"198px"}, 300);
	}
	
	function setSMTPServer(value){
		switch(value) {
		case "other":
			document.getElementById("sender").value = "";
			document.getElementById('smtpserver').value = "";
			document.getElementById('smtpport').value = "";
			document.getElementById('smtpuser').value = "";
			document.getElementById('smtppwd').value = "";
			document.getElementById('tls_enable').checked = false;
			break;
		case "gmail":
			document.getElementById("sender").value = "";
			document.getElementById('smtpserver').value = "smtp.gmail.com";
			document.getElementById('smtpport').value = "587";
			document.getElementById('smtpuser').value = "";
			document.getElementById('smtppwd').value = "";
			document.getElementById('tls_enable').checked = true;
			break;
		case "hotmail":
			document.getElementById("sender").value = "";
			document.getElementById('smtpserver').value = "smtp.live.com";
			document.getElementById('smtpport').value = "587";
			document.getElementById('smtpuser').value = "";
			document.getElementById('smtppwd').value = "";
			document.getElementById('tls_enable').checked = true;
			break;
		case "yahoo":
			document.getElementById("sender").value = "";
			document.getElementById('smtpserver').value = "smtp.mail.yahoo.com";
			document.getElementById('smtpport').value = "587";
			document.getElementById('smtpuser').value = "";
			document.getElementById('smtppwd').value = "";
			document.getElementById('tls_enable').checked = false;
			break;
		}
	}
	
	$("#show_password").change(function(){
		if($(this).attr("checked") == 'checked'){
			$("#smtppwd").prop("type","text");
		}else{
			$("#smtppwd").prop("type","password");
		}
	});
	</script>

<?php
}

function save_email(){
	global $__GSM_SUM__;
	
	$aql = new aql();
	$conf_path = '/etc/asterisk/gw/email.conf';
	$hlock = lock_file($conf_path);
	if(!file_exists($conf_path)) {exec('touch /etc/asterisk/gw/email.conf');}
	$aql->set('basedir','/etc/asterisk/gw');
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	$exist_array = $aql->query("select * from email.conf");
	
	$channel = $_POST['channel'];
	
	if(!isset($exist_array[$channel])){
		$aql->assign_addsection($channel,'');
	}
	
	if(isset($_POST['sw'])) {
		$sw = 'on';
	} else {
		$sw = 'off';
	}
	if(isset($exist_array[$channel]['sw'])) {
		$aql->assign_editkey($channel,'sw',$sw);
	} else {
		$aql->assign_append($channel,'sw',$sw);
	}
	
	$smtpserversel = $_POST['smtpserversel'];
	if(isset($exist_array[$channel]['smtpserversel'])){
		$aql->assign_editkey($channel,'smtpserversel',$smtpserversel);
	}else{
		$aql->assign_append($channel,'smtpserversel',$smtpserversel);
	}
	
	if(isset($_POST['sender'])) {
		$val = trim($_POST['sender']);
		if(isset($exist_array[$channel]['sender'])) {
			$aql->assign_editkey($channel,'sender',$val);
		} else {
			$aql->assign_append($channel,'sender',$val);
		} 
	}
	
	if(isset($_POST['smtpserver'])) {
		$val = trim($_POST['smtpserver']);
		if(isset($exist_array[$channel]['smtpserver'])) {
			$aql->assign_editkey($channel,'smtpserver',$val);
		} else {
			$aql->assign_append($channel,'smtpserver',$val);
		} 
	}
	
	if(isset($_POST['smtpport'])) {
		$val = trim($_POST['smtpport']);
		if(isset($exist_array[$channel]['smtpport'])) {
			$aql->assign_editkey($channel,'smtpport',$val);
		} else {
			$aql->assign_append($channel,'smtpport',$val);
		} 
	}
	
	if(isset($_POST['smtpuser'])) {
		$val = trim($_POST['smtpuser']);
		if(isset($exist_array[$channel]['smtpuser'])) {
			$aql->assign_editkey($channel,'smtpuser',$val);
		} else {
			$aql->assign_append($channel,'smtpuser',$val);
		} 
	}
	
	if(isset($_POST['smtppwd'])) {
		$val = trim($_POST['smtppwd']);
		if(isset($exist_array[$channel]['smtppwd'])) {
			$aql->assign_editkey($channel,'smtppwd',$val);
		} else {
			$aql->assign_append($channel,'smtppwd',$val);
		} 
	}
	
	if(isset($_POST['tls_enable'])) {
		$tls_enable = 'yes';
	} else {
		$tls_enable = 'no';
	}
	if(isset($exist_array[$channel]['tls_enable'])) {
		$aql->assign_editkey($channel,'tls_enable',$tls_enable);
	} else {
		$aql->assign_append($channel,'tls_enable',$tls_enable);
	}
	
	if(isset($_POST['smail1'])) {
		$val = trim($_POST['smail1']);
		if(isset($exist_array[$channel]['smail1'])) {
			$aql->assign_editkey($channel,'smail1',$val);
		} else {
			$aql->assign_append($channel,'smail1',$val);
		} 
	}
	
	if(isset($_POST['smail2'])) {
		$val = trim($_POST['smail2']);
		if(isset($exist_array[$channel]['smail2'])) {
			$aql->assign_editkey($channel,'smail2',$val);
		} else {
			$aql->assign_append($channel,'smail2',$val);
		} 
	}
	
	if(isset($_POST['smail3'])) {
		$val = trim($_POST['smail3']);
		if(isset($exist_array[$channel]['smail3'])) {
			$aql->assign_editkey($channel,'smail3',$val);
		} else {
			$aql->assign_append($channel,'smail3',$val);
		} 
	}
	
	if(isset($_POST['mail_title'])) {
		$val = trim($_POST['mail_title']);
		if(isset($exist_array[$channel]['mail_title'])) {
			$aql->assign_editkey($channel,'mail_title',$val);
		} else {
			$aql->assign_append($channel,'mail_title',$val);
		} 
	}
	
	if(isset($_POST['mail_content'])) {
		$val = trim($_POST['mail_content']);
		if(isset($exist_array[$channel]['mail_content'])) {
			$aql->assign_editkey($channel,'mail_content',$val);
		} else {
			$aql->assign_append($channel,'mail_content',$val);
		} 
	}
	
	//sync
	$sync = false;
	if(isset($_POST['spans']) && is_array($_POST['spans'])){
		$sync = true;
		for($i=1;$i<=$__GSM_SUM__;$i++){
			if(isset($_POST['spans'][1][$i])){
				$sync_port[$i] = $_POST['spans'][1][$i];
			}
		}
	}
	
	if($sync){
		foreach($sync_port as $port => $value){
			if(!isset($exist_array[$port])){
				$aql->assign_addsection($port,'');
			}
			
			if(isset($_POST['sw_sync'])){
				$_POST['sw'] = $sw;
			}
			
			if(isset($_POST['tls_enable_sync'])){
				$_POST['tls_enable'] = $tls_enable;
			}
			
			foreach($_POST as $key => $value){
				if($key == 'query_switch') continue;
				if(isset($_POST[$key.'_sync'])){
					if(isset($exist_array[$port][$key])){
						$aql->assign_editkey($port,$key,$_POST[$key]);
					}else{
						$aql->assign_append($port,$key,$_POST[$key]);
					}
				}
			}
		}
	}
	
	if (!$aql->save_config_file('email.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	unlock_file($hlock);
	
	//reboot sms_recv
	exec("kill -9 `ps -w | grep sms_recv | grep -v grep | awk '{print $1}'` > /dev/null 2>&1");
	exec("php -r /my_tools/lua/sms_receive/sms_recv > /dev/null 2>&1");
	
	wait_apply("exec", "/etc/init.d/logfile_monitor restart > /dev/null 2>&1 &");
}

if(isset($_POST['send']) || isset($_GET['send'])){
	if($_GET['send']=='Edit'){
		edit_email();
	}else if($_POST['send']=='Save'){
		save_email();
		show_email();
	}
}else{
	show_email();
}

require("/www/cgi-bin/inc/boot.inc");
?>