<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/define.inc");
require_once("/www/cgi-bin/inc/redis.inc");
?>

<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript">

//*/
function check()
{
	return true;
}

$(document).ready(function (){
   $("#select_all").click(function(){
      $(".port_sel_class").attr("checked", $(this).is(':checked')); 
	  
   });
   
   $("div#aeu img").attr("src","/images/icons/advanced.gif");
    
    $(".port_sel_class").click(function(){ 
      if(!$(this).is(':checked')) {
		  $("#select_all").attr("checked", false); 
	  }
    });
});

</script>

<?php
if($__deal_cluster__){
	$cluster_info = get_cluster_info();
}

if ($cluster_info['mode'] != "slave")  {
	show_simemu_settings();
} else {
	echo "This page does not take any effect. Please change to master page.";
}
if($_POST) {	
	if ($_POST['send'] && $_POST['send'] == 'autocheck') {
		check_port_status_change();		
	}
}

function js_autorows($name,$value)
{
	$class = str_replace("out","in",$name);
	//get line number for <textarea>
	$value_array = explode('\n', $value);
	$line_num = 0;
	if(is_array($value_array)){
		foreach($value_array as $line){
			if(strlen($line)<50){
				$line_num++;
			}else{
				$line_num = ceil($line/50);
			}
		}
	}
	if($line_num > 5)
		$line_num = 5;

$script = <<<EOF
	<script type="text/javascript">
	$("#${name}").attr("rows",$line_num);
	$(".${class}").attr("rows",$line_num);
	</script>
EOF;
//EOF
	echo $script;

}

function js_setvalue($name,$value)
{
	$value = trim($value);
	$value = str_replace("\r\n",'\\n',$value);
	$value = str_replace("\r",'\\n',$value);
	$value = str_replace("\n",'\\n',$value);
	$value = str_replace("\"",'\"',$value);

	echo "<script type=\"text/javascript\">";
	echo "$(\"#".$name."\").html(\"".$value."\");";
	echo "</script>";
	
	js_autorows($name,$value);

	ob_flush();
	flush();
}

////////////////////////////////////////////////////////
//timeout exception
function sig()
{
	throw new Exception;
}

////////////////////////////////////////////////////////
//check the ports selected status  
function check_port_status_change()
{
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $__GSM_SUM__;
	global $__deal_cluster__;
	//////////////////////////////////////////
	//get port number
	$check_array = array();
	$check_str = '';
	$port_numbers = 0;
	for($i=1; $i<=$__GSM_SUM__; $i++) {
		$id = 'sel-1-'.$i;
		if (isset($_POST[$id])) {
			$port_numbers++;
			if ($check_str == '') {
				$check_str = $i;
			} else {
				$check_str .= '-'.$i;
			}
		}
	}
	$check_array[1] = $check_str;
	
	if($__deal_cluster__){
		$cluster_info = get_cluster_info();
		if($cluster_info['mode'] == 'master') {		
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				$check_str = '';
				if(isset($cluster_info[$__BRD_HEAD__.$b.'_ip']) && $cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					for($i=1; $i<=$__GSM_SUM__; $i++) {
						$id = 'sel-'.$b.'-'.$i;
						if (isset($_POST[$id])) {
							$port_numbers++;
							if ($check_str == '') {
								$check_str = $i;
							} else {
								$check_str .= '-'.$i;
							}
						}
					}
				}
				$check_array[$b] = $check_str;
			}
		}
	}
	
	//get simemu switch status
	$simemu_sw_org = `/my_tools/set_config /etc/cfg/simemusvr.conf get option_value SimEmuSvr simemusvr_switch`;
	///////////////////////////////////////////////
	$count = 0;
	$timeout_duration = 10;
	$redis_client = new Predis\Client();
	if ($simemu_sw_org == 'yes') {
    	//modify conf simemusvr_switch 'yes' to 'no'
		$st = `/my_tools/set_config /etc/cfg/simemusvr.conf set option_value SimEmuSvr simemusvr_switch no`;
		exec("ps -ef | grep SimEmuSvr | grep -v grep | cut -c 0-5 | xargs kill -9 ");
	}
	
	//execute lua program
	$redis_client->del("app.asterisk.php.checklist");
	exec("ps -ef | grep emu_udp_check | grep -v grep | cut -c 0-5 | xargs kill -9");
	
	$log_file = "/tmp/.shresult".$i;
	$clear_cmd = "echo > ".$log_file;
	exec($clear_cmd);
	$cmd = "cd /my_tools/lua/emu_Autocheck/ && lua emu_udp_check.lua ".$check_array[1]."  > ".$log_file." 2>&1 &";
	//$cmd = "cd /my_tools/lua/emu_Autocheck/ && lua emu_udp_check.lua ".$i." ".$check_array[$i]."  > ".$log_file." 2>&1 &";
	exec($cmd,$res,$rc);
	
	sleep(1);

	//read data from redis
	//add get redis data timeout
	declare(ticks=1);
	try {
		pcntl_alarm(180);
		pcntl_signal(SIGALRM,"sig");

		while (($count >=0) && ($count <$port_numbers)) {
			$blpop_str=$redis_client->blpop("app.asterisk.php.checklist",$timeout_duration);
			if (isset($blpop_str)) {
				$pop_str=$blpop_str[1];
				if ($pop_str<>""){
					$pop_array = json_decode($pop_str,true);
					$pop_portname = $pop_array['portname'];
					$pop_result = $pop_array['result'];
					$portname_id = $pop_portname.'-out';
					js_setvalue($portname_id,$pop_result);
				}
				$count = $count + 1;
			}
			ob_flush();
			flush();
		}
		pcntl_alarm(0);
	} catch(Exception $e) {
		ob_flush();
		flush();
	}
	
	if ($simemu_sw_org == 'yes') {
		$st = `/my_tools/set_config /etc/cfg/simemusvr.conf set option_value SimEmuSvr simemusvr_switch yes`;
		exec("ps -ef | grep SimEmuSvr | grep -v grep | cut -c 0-5 | xargs kill -9 ");
	}
}

function show_simemu_settings()
{
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	global $__GSM_HEAD__;
	global $__GSM_SUM__;
	global $HEAD;
	global $__deal_cluster__;
	
	if($__deal_cluster__){
		$cluster_info = get_cluster_info();
	}
	
?>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post" id="emuForm">
       
	<div id="tab">                                                                                           
			<li class="tb1">&nbsp;</li>                                                         
			<li class="tbg"><?php echo language('SimBank Operation');?></li>                      
			<li class="tb2">&nbsp;</li>   
			<div id="adv_warn">
				<li class="back_warn"></li>
				<li class="back_font">
					<?php echo language('Warning: Only for factory check! Make sure simbank connection broken!');?>
				</li>
				<li>&nbsp;</li>
			</div>			
	</div>  
	<table width="100%" class="tedit"> 
		<tr>
			<th>
			    <div class="helptooltips">
                    <?php echo language('Check Operation'); ?>:
					<span class="showhelp">
						<?php echo language('Select the port blow to check the status.'); ?>
					</span>
                </div>
			</th>
			<td colspan = " 9">
				<input type="submit" value="<?php echo language('Check');?>" onclick="document.getElementById('send').value='autocheck';" />
			</td>
		</tr>
	</table>
	
	<br>
	
	<table width="100%" class="tshow">
		<tr>
			<th style="width: 20px;"><input type="checkbox" name="select_all" id="select_all" class="port_sel_class" /></th>
			<th style="width: 200px;"><?php echo language('Portname');?></th>
			<th><?php echo language('Result');?></th>
		</tr>	
	<?php
	for($i=1; $i<=$__GSM_SUM__; $i++) {
		$value = $HEAD.$__GSM_HEAD__.$i;

		$id = "sel-1-".$i;
		echo "<tr>";
		echo "<td>";
		echo "<input type=\"checkbox\" name=\"$id\"  id=\"$id\" class=\"port_sel_class\" />";
		echo "</td>";
		echo "<td>";
		echo get_gsm_name_by_channel($i);
		echo "</td>";
		echo "<td><textarea rows=\"1\" style=\"width:98%;\" name=\"$value-out\" id=\"$value-out\" readonly></textarea></td>";
		//echo "<td><textarea style=\"width:98%;overflow-y:auto\" name=\"$value-out\" id=\"$value-out\" readonly></textarea></td>";
		echo "</tr>";
	}
	
	if($__deal_cluster__){
		if($cluster_info['mode'] == 'master') {
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				if(isset($cluster_info[$__BRD_HEAD__.$b.'_ip']) && $cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					for($i=1; $i<=$__GSM_SUM__; $i++) {
						$value = $__BRD_HEAD__.$b.'-'.$__GSM_HEAD__.$i;
						$id = 'sel-'.$b.'-'.$i;
						echo "<tr>";
						echo "<td>";
						echo "<input type=\"checkbox\" name=\"$id\"  id=\"$id\" class=\"port_sel_class\" />";
						echo "</td>";
						echo "<td>";
						echo get_gsm_name_by_channel($i,$b);
						echo "</td>";
						echo "<td><textarea rows=\"1\" style=\"width:98%;\" name=\"$value-out\" id=\"$value-out\" value=\"$value-out\" readonly></textarea></td>";
						echo "</tr>";
					}
				}
			}
		}
	}
	?>		
	</table>
	
<?php	
}
?>

	<input type="hidden" name="send" id="send" value="" />

	</form>

<?php require("/www/cgi-bin/inc/boot.inc");?>
