<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/check.inc");

?>

<script type="text/javascript" src="/js/functions.js">
</script>

<script type="text/javascript" src="/js/check.js"></script>

<script type="text/javascript">
function check()
{
	var command = document.getElementById("command").value;

	if(trim(command) == '') {
		return false;
	}

	return true;
}
</script>

<?php
$command = '';
$result = '';
if($_POST && isset($_POST['send']) && $_POST['send'] == 'Execute') {
	if(isset($_POST['command'])) { 
		$command = trim($_POST['command']);
		if($command != '') {
			$result = execute_astcmd($command);
		}
	}
}
?>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Asterisk CLI');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	<table width="100%" class="tedit" >
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('Command');?>:
					<span class="showhelp">
					<?php echo language('Command help@adv-astcli',"
						Type your Asterisk CLI commands here to check or debug your gateway.<br/>
						e.g, type \"help\" or \"?\" you will get all help information.");
					?>
					</span>
				</div>
			</th>
			<td>
				<input id="command" type="text" name="command" style="width: 350px;" value='<?php echo $command; ?>' />&nbsp;&nbsp;
				<input type="hidden" name="send" id="send" value="" />
				<input type="submit" value="<?php echo language('Execute');?>" onclick="document.getElementById('send').value='Execute';return check();" />
			</td>
		</tr>
	</table>	
	</form>

<?php
if($result != '') {
	echo "<h5>";echo language("Output");echo ":</h5>";
	echo $result;
}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>
