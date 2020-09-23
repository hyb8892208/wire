<?php
include_once("/www/cgi-bin/inc/define.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");
include_once("/www/cgi-bin/inc/language.inc");
?>

<?php
require_once("/www/cgi-bin/inc/language.inc");
$language = get_web_language_cache("/tmp/web/language.cache");

?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">
	<head>
	<meta charset="utf-8">
	<meta name="viewport" content="width=device-width, initial-scale=1">
	<title>GSM Module COM For Gateway</title>
	<link rel="icon" href="/images/logo.ico" type="image/x-icon">
	<link rel="shortcut icon" href="/images/logo.ico" type="image/x-icon">
	<link rel="stylesheet"  href="/css/style.css" />
	</head>
	<script src="/js/js.js"></script>
	<script src="/js/jquery.js"></script>
	<body>

	<div id="bg_blank">
	<div>

<!---// load jQuery and the jQuery iButton Plug-in //---> 
<!--<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.3.2/jquery.min.js"></script> -->
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<!---// load the iButton CSS stylesheet //---> 
<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />
<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>
<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>

	<br/>
	<br/>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<?php
		$pulse_no_check = 'checked';
		$pulse_yes_check = '';

		if(isset($_POST['pulse']) && ($_POST['pulse'] == 1)) {
			$pulse_no_check = '';
			$pulse_yes_check = 'checked';
		}
	?>

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Remote COM Mode');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<input type="hidden" name="action" id="action" value="" />

	<table width="100%" class="tedit" >
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Power Down No Pulse');?>:
					<span class="showhelp">
					</span>
				</div>
			</th>
			<td>
				<input id="pulse" name="pulse" type="radio" value="0" <?php echo $pulse_no_check; ?> > No
				<input id="pulse" name="pulse" type="radio" value="1" <?php echo $pulse_yes_check; ?> >Yes
			</td>
		</tr>
		<tr>
			<th>
				<?php echo language('Entry Remote COM');?>:
			</th>
			<td>
				<input type="submit" value="<?php echo language('Entry Remote COM');?>" onclick="document.getElementById('action').value='entry';" />
			</td>
		</tr>
		<tr>
			<th>
				<?php echo language('Leave Remote COM');?>:
			</th>
			<td>
				<input type="submit" value="<?php echo language('Leave Remote COM');?>" onclick="document.getElementById('action').value='leave';" />
			</td>
		</tr>
		<tr>
			<th>
				<?php echo language('Power for Modules');?>:
			</th>
			<td>
				<input type="submit" value="<?php echo language('Power On');?>" onclick="document.getElementById('action').value='poweron';" />
				<input type="submit" value="<?php echo language('Power Off');?>" onclick="document.getElementById('action').value='poweroff';" />
			</td>
		</tr>
	</table>	
	</form>

<?php

	if(isset($_POST['action'])) {
		if($_POST['action'] == 'entry') {
			if(isset($_POST['pulse']) && ($_POST['pulse'] == 1)) {
				$pulse = 'nopulse';
			} else {
				$pulse = '';
			}
			exec("/my_tools/entry_remote_com_mode $pulse > /dev/null 2>&1");
		} else if($_POST['action'] == 'leave') {
			exec("/my_tools/leave_remote_com_mode > /dev/null 2>&1");
		} else if($_POST['action'] == 'poweron') {
			exec("echo 1 > /proc/gsm_module_power_key-0");
		} else if($_POST['action'] == 'poweroff') {
			exec("echo 0 > /proc/gsm_module_power_key-0");
		}
	}
	?>
	
	<?php require("/www/cgi-bin/inc/boot.inc");?>
}
