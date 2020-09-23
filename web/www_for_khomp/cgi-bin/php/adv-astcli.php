<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/check.inc");

?>

<script type="text/javascript" src="/js/functions.js"></script>
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

	<div class="content">
		<span class="title"><?php echo language('Asterisk CLI');?></span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Command');?>:
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<b><?php echo language('Command');?>:</b><br/>
							<?php echo language('Command help',"
								Type your Asterisk CLI commands here to check or debug your gateway.<br/>
								e.g, type \"help\" or \"?\" you will get all help information.");
							?>
						</div>
					</div>
				</div>
			</span>
			<div class="tab_item_right">
				<input id="command" type="text" name="command" style="width: 350px;" value='<?php echo $command; ?>' />&nbsp;&nbsp;
				<input type="hidden" name="send" id="send" value="" />
				<input type="submit" value="<?php echo language('Execute');?>" onclick="document.getElementById('send').value='Execute';return check();" />
			</div>
		</div>
	</div>
	
</form>

<?php
if($result != '') {
	echo "<h5>";echo language("Output");echo ":</h5>";
	echo $result;
}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>