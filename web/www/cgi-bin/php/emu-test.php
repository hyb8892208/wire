<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");

$aql = new aql();
$aql->set('basedir', '/tmp/');
$res = $aql->query("select * from hw_info.cfg");

?>

<form action="<?php echo get_self();?>" method="post">
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('EMU Test');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	
	<table width="100%" class="tedit">
		<tr>
			<th>
				<div class="helptooltips">
					<?php echo language('EMU Port');?>
					<span class="showhelp">
					<?php 
						echo language('EMU Port help','EMU Port');
					?>
					</span>
				</div>
			</th>
			<td>
				<select name="emu_port" id="emu_port">
					<?php 
					$sel = $_POST['emu_port'];
					for($i=0;$i<count($res['emu']);$i++){
						$val = $i+1;
						
						$select = '';
						if($sel == $val) $select = 'selected';
						echo "<option value='$val' $select>".$val."</option>";
					}
					?>
				</select>
			</td>
		</tr>
	</table>
	
	<br/>
	
	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" id="save" value="<?php echo language('Test');?>" onclick="document.getElementById('send').value='save'" />
</form>

<br/>

<?php 
function save_emu_test(){
	$emu_port = $_POST['emu_port'];
	
	$Report = language('Report');
	$Result = language('Result');
	$theme = language('EMU Test');
	trace_output_start($Report,$theme);
	trace_output_newline();
	echo language("EMU Testing");echo "......";
	ob_flush();
	flush();
	
	$a = exec("/my_tools/Emu_test ".$emu_port." ", $output, $return);
	
	if($return == 255){
		trace_output_newhead($Result);
		echo language("EMU Test Succeeded.");
	}else{
		trace_output_newhead($Result);
		for($j=0;$j<8;$j++){
			if(($return&(1 << $j)) == 0){
				echo "EMU port".$emu_port." ".($j+1)." failed<br/>";
			}
		}
		echo language("EMU Test Failed");
	}
	
	trace_output_newhead(language("Detial"));
	echo '<br/>';
	for($i=0;$i<count($output);$i++){
		echo $output[$i].'<br/>';
	}
	trace_output_end();
}

if(isset($_POST['send']) && $_POST['send'] == 'save'){
	save_emu_test();
}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>