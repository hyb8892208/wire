<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
require_once("/www/cgi-bin/inc/aql.php");
?>
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />

<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/float_btn.js"></script> 
<script type="text/javascript">
function _onfocus(obj,str)
{
	if (obj.value == str) {
		obj.value =''
		obj.style.color = '#000000';
	}
}
function _onchange(obj)
{
/*	var colDataRows = tbl_fctn.rows;
	for (var i = 0; i < colDataRows.length; i++)
    	alert(colDataRows[i].cells[0].childNodes[1].name);
 
	var tab = document.getElementById("tbl_fctn"); 
	var rows = tab.rows.length;
	for(var i = 1; i < rows; i++) {
		var cols = tab.rows[i].childNodes;
		for(var m = 0; m < cols.length; m++) {
			alert(cols[m].childNodes[0].length);
		}
	}
*/

	/*pattern[]*/
	var save = -1;
	var pr=document.getElementsByName(obj.name);
	for(i=0;i<pr.length;i++){
		if(pr[i]==obj) {
			save = i;
			break;
		}
	}

	if(save != -1) {
		var pa=document.getElementsByName('number[]');
		if(pa[save] && pa[save].value=='') {
			pa[save].value='.';
		}
	}
}
function _onblur(obj,str)
{
	if (trim(obj.value) =='') {
		obj.value = str;
		obj.style.color = '#aaaaaa';
	}
}
function addDPRow()
{
	var len = tbl_dialroute.rows.length - 1;
	
	var number = len - 1;
	var newTr = tbl_dialroute.insertRow(len);
	var newTd0 = newTr.insertCell(0);
	var newTh1 = newTr.insertCell(0);

	newTd0.innerHTML = '<td><?php dial_pattern_html('','');?></td>';
	newTh1.innerHTML = '<td style="background-color:#D0E0EE; text-align: right;font-weight: bold;"><?php echo language('Number');?>' + number + ':</td>';

}	
</script>
<?php
function dial_pattern_html($number,$convert=true)
{
	if($convert) {
		$sign = '\\\'';
	} else {
		$sign = '\'';
	}
	
	echo "<input title=\"number\" type=\"text\" size=\"8\" name=\"number[]\" onchange=\"_onchange(this)\" value=\"$number\" />";

	echo '<img src="/images/delete.gif" style="float:none; margin-left:20px; margin-bottom:-3px;cursor:pointer;" alt="remove" title="';
	echo language('Click here to remove this number list.');
	echo '" onclick="javascript:this.parentNode.parentNode.parentNode.removeChild(this.parentNode.parentNode);">';
}
?>

<?php
function show_page()
{

//Getting the page data from gw_routing_blacklist.conf

$gw_routing_blacklist_conf = '/etc/asterisk/gw_routing_blacklist.conf';

if(!file_exists($gw_routing_blacklist_conf)) fclose(fopen($gw_routing_blacklist_conf, "w"));
$blacklist_aql = new aql();
$blacklist_aql->set('basedir', '/etc/asterisk');

$hlock = lock_file($gw_routing_blacklist_conf);
$res = $blacklist_aql->query("select * from gw_routing_blacklist.conf");
unlock_file($hlock);

$enable_blacklist = $res['general']['enable_blacklist'];
$blacklist = array();
$blacklist = $res['list'];

?>
<form enctype="multipart/form-data" action="<?php echo get_self();?>" method="post">
<div id="tab">	
	<li class="tb_unfold" onclick="lud(this,'tab_main')" id="tab_main_li">&nbsp;</li>
	<li class="tbg_fold" onclick="lud(this,'tab_main')"><?php echo language('Routing Blacklist');?></li>
	<li class="tb2_fold" onclick="lud(this,'tab_main')">&nbsp;</li>
	<li class="tb_end">&nbsp;</li>
</div>

<?php js_float_show_time();?>
<table width="98%" class="tedit" align="right" id="tbl_dialroute">
	<tr>
		<th>
			<div class="helptooltips">
				<?php echo language('Enable Blacklist');?>:
				<span class="showhelp">
				<?php echo language('Enable Blacklist help');?>
				</span>
			</div>
		</th>
		<td >
			<input type="checkbox" id="enable_blacklist" name="enable_blacklist" style="" onchange="" <?php if($enable_blacklist == 'on') echo "checked";?> />
		</td>
	</tr>
	<tr class="ttitle">
		<td colspan = 2>
			<div style="font-size: 12px;height: 15px;" class="helptooltips">
				<?php echo language('Dial Patterns that will use this Routing Blacklist');?>
				<span class="showhelp">
				<?php echo language('Dial Patterns that will use this Route Blacklist help','AT the inbound mode, the rules will be routed to hangup(), if the CallerNumber is exist in the blacklist.');
				?>
				</span>
			</div>
		</td>
	</tr>
<?php
	if(isset($blacklist) && is_array($blacklist)) {
		$i = 0;
		foreach($blacklist as $number) {
			$i++;
			echo '<tr>';
			echo '<td>';echo language('Number');echo " $i: </td>";
			echo '<td>';
			dial_pattern_html($number,false);
			echo '</td></tr>';
		}
	} 
?>
	<tr id="add_new">
		<td colspan = 2>
			<input type="button" value="+ <?php echo language('Add a new number to Blacklist');?>" onclick="addDPRow()"/>
		</td>
	</tr>
</table>

<div id="newline"></div>
<input type="hidden" name="send" id="send" value="" />
<table id="" class="">
	<tr id="float_btn_tr" class="float_btn_tr" style="">
	<td>
	<input type="submit" class="float_btn gen_short_btn"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>
&nbsp;
	</td>
	<td>
	<!-- <input type="submit" class="float_btn gen_short_btn"  value="<?php echo language('Apply');?>" onclick="document.getElementById('send').value='Apply';return check();"/> -->
	</td>
	</tr>
</table>
</form>
<?php
} //end of show_page()
?>
<?php 
function save_routing_blacklist()
{
	$number_arr = $_POST['number'];
	$enable_blacklist = isset($_POST['enable_blacklist']) ? trim($_POST['enable_blacklist']) : 'off';
	$gw_routing_blacklist = '/etc/asterisk/gw_routing_blacklist.conf';
	$hlock = lock_file($gw_routing_blacklist);
	if(!file_exists($gw_routing_blacklist)) fclose(fopen($gw_routing_blacklist, "w"));

	$aql = new aql();
	$aql->set('basedir', '/etc/asterisk');
	if(!$aql->open_config_file($gw_routing_blacklist)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	
	$general_data = 'enable_blacklist=' . $enable_blacklist . "\n";
	
	$aql->assign_delsection('general');
	$aql->save_config_file('gw_routing_blacklist.conf');
	$aql->assign_addsection('general', $general_data);
	$aql->save_config_file('gw_routing_blacklist.conf');
	$data = '';
	foreach ($number_arr as $key => $value) {
		$data .= 'number' .$key . '=' . $value . "\n";
		
	}

	$aql->assign_delsection('list');
	$aql->save_config_file('gw_routing_blacklist.conf');
	$aql->assign_addsection('list', $data);
	$aql->save_config_file('gw_routing_blacklist.conf');

	unlock_file($hlock);
	if($enable_blacklist == 'on'){
		$cmd = '';
		foreach ($number_arr as $key => $value) {
			$cmd .= "asterisk -rx \"database put blacklist $value $key\" && ";
		}
		$cmd = substr($cmd, 0, -4);
		wait_apply("exec","asterisk -rx \"database deltree blacklist\" > /dev/null 2>&1 &");
		wait_apply("exec", $cmd);
	}
}

//Saving the page data to gw_routing_blacklist.conf
if(isset($_POST['send']) && trim($_POST['send'] == 'Save')){
	save_routing_blacklist();
	save_routings_to_extensions();
	wait_apply("exec", "asterisk -rx \"dialplan reload\"");

	show_page();
} else {
	show_page();
}

?>
<script type="text/javascript">
	$(document).ready(function(){
		$("#enable_blacklist").iButton();
	});
</script>
<?php require("/www/cgi-bin/inc/boot.inc");?>

<div id="float_btn1" class="float_btn1 sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()" >
</div>

