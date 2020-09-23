<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/smsinboxdb.php");
?>

<?php
$smsinbox_file = '/data/log/smsinbox.db';
$line_counts = 10;

if(!file_exists($smsinbox_file)) {
	require("/www/cgi-bin/inc/boot.inc");
	exit(0);
}

$db = new SMSINBOXDB();
if(!$db) {
	require("/www/cgi-bin/inc/boot.inc");
	exit(0);
}

function show_msg($str)
{
	echo "<br/><br/><br/><br/><h2 style=\"text-align:center\">";
	echo $str;
	echo "</h2><br/><br/>";
}

/********************************************************************/

if($_POST) {
	if(isset($_POST['send'])) {
		if($_POST['send'] == 'Delete') {
			if(isset($_POST['sms'])) {
				del_sms($_POST['sms']);
			}
		} else if($_POST['send'] == 'Clean Up') {
			del_all_sms();
		} else if($_POST['send'] == 'Export') {
			$export_to_excel = true;
		}
	}
}

/********************************************************************/
$cur_page = 1;
if(isset($_GET['current_page']) && is_numeric($_GET['current_page']))
	$cur_page = $_GET['current_page'];
else if(isset($_POST['current_page']) && is_numeric($_POST['current_page']))
	$cur_page = $_POST['current_page'];
	
/* filter */

$filter_sql = '';

$port_filter = "all";
$port_check = "";
if(isset($_GET['port_filter']))
	$port_filter = $_GET['port_filter'];
else if(isset($_POST['port_filter']))	
	$port_filter = $_POST['port_filter'];
if($port_filter != 'all' && $port_filter != ''){
	$port_array = explode(",",$port_filter);
	foreach($port_array as $key=>$port){
		//if 3g module,chang form to "gsm"
		$port_check[$port] = "checked";
		if(strstr($port,"umts")) {
			$port = str_replace("umts","gsm",$port);
		} else if (strstr($port,"null")) {
			$port = str_replace("null","gsm",$port);
		} else if (strstr($port, "cdma")) {
			$port = str_replace("cdma", "gsm", $port);
		} else if( strstr($port, "lte")) {
			$port = str_replace("lte", "gsm", $port);
		}
		$bc = get_gsm_channel_by_name($port);
		$port = get_gsm_value_by_channel($bc['channel'],$bc['board']);
		if($key == 0){
			$filter_sql = "where (port='".$port."'";
			if(strstr($port, $__BRD_HEAD__.$bc['board'])){	//find Board-1-gsm-? record
				$filter_sql .= " OR port='".$port."'" . " OR port='" .$bc['channel'] . "'";
			}else if(strstr($port, $__BRD_HEAD__) < 0){	//find gsm-? record
				$port = $__BRD_HEAD__."1-".$__GSM_HEAD__.$port;
				$filter_sql .= " OR port='".$port."'" . " OR port='" .$bc['channel'] . "'";
			} else {
				$filter_sql .= " OR port='" .$bc['channel'] . "'";
			}
		}else{
			$filter_sql .= " OR port='".$port."'";
			if(strstr($port, $__BRD_HEAD__.$bs['board'])){	//find Board-1-gsm-? record
				$filter_sql .= " OR port='".$port."'" . " OR port='" .$bc['channel'] . "'";
			}else if(strstr($port, $__BRD_HEAD__) < 0){	//find gsm-? record
				$port = $__BRD_HEAD__."1-".$__GSM_HEAD__.$port;
				$filter_sql .= " OR port='".$port."'" . " OR port='" .$bc['channel'] . "'";
			} else {
				$filter_sql .= " OR port='" .$bc['channel'] . "'";
			}
		}
	}
	if($filter_sql != '')$filter_sql .= ")";
	
	
	}
/* for stand alone mode and slave mode */
$cluster_info = get_cluster_info();
if($cluster_info['mode'] != 'master' && $filter_sql == "") {
	$filter_sql = "where (port='1' OR port='".$__BRD_HEAD__."1-".$__GSM_HEAD__."1'";
	for($i=2;$i<=$__GSM_SUM__;$i++) {
		$filter_sql .= " OR port='".$i."'";
		$filter_sql .= " OR port='".$__BRD_HEAD__."1-".$__GSM_HEAD__."$i'";
	}
	$filter_sql .= ")";
}

$phone_number_filter = "";
if(isset($_GET['phone_number_filter']))
	$phone_number_filter =  $_GET['phone_number_filter'];
else if(isset($_POST['phone_number_filter']))
	$phone_number_filter =  $_POST['phone_number_filter'];

//echo "pre phonenumber filter_sql=".$phone_number_filter."<br>";
if($phone_number_filter != '' && preg_match("/^[-+*_@\d]*$/",$phone_number_filter)){
	$phone_number_filter_sql = str_replace('*', '%', $phone_number_filter);
	if($filter_sql == '')
		$filter_sql = "where phonenumber like '".$phone_number_filter_sql."'";
	else
		$filter_sql .= " and phonenumber like '".$phone_number_filter_sql."'";
}

//echo "phonenumber filter_sql=".$filter_sql."<br>";

$start_datetime_filter = "from";
if(isset($_GET['start_datetime_filter']))
	$start_datetime_filter = $_GET['start_datetime_filter'];
else if(isset($_POST['start_datetime_filter']))
	$start_datetime_filter = $_POST['start_datetime_filter'];
if($start_datetime_filter != 'from' && $start_datetime_filter != '') {
	if($filter_sql == '')
		$filter_sql = "where time >= '".$start_datetime_filter."'";
	else
		$filter_sql .= " and time >= '".$start_datetime_filter."'";
}

$end_datetime_filter = "to";
if(isset($_GET['end_datetime_filter']))
	$end_datetime_filter = $_GET['end_datetime_filter'];
else if(isset($_POST['end_datetime_filter']))
	$end_datetime_filter = $_POST['end_datetime_filter'];
if($end_datetime_filter != 'to' && $end_datetime_filter != ''){
	if($filter_sql == '')
		$filter_sql = "where time <= '".$end_datetime_filter."'";
	else
		$filter_sql .= " and time <= '".$end_datetime_filter."'";
}
$message_filter = "";
if(isset($_GET['message_filter']))
	$message_filter = $_GET['message_filter'];
else if(isset($_POST['message_filter']))
	$message_filter = $_POST['message_filter'];
if($message_filter != ''){
	$message_array = preg_split("/[^0-9a-zA-Z\x{4e00}-\x{9fa5}]+/u", $message_filter);
	foreach($message_array as $message){
		if($message != ""){
			if($filter_sql == '')
				$filter_sql = "where message like '%".$message."%'";
			else
				$filter_sql .= " and message like '%".$message."%'";
		}
	}
}

/* sort */
$sort = '';
if(isset($_GET['sort']))
	$sort = $_GET['sort'];
if(isset($_POST['sort']))
	$sort = $_POST['sort'];

$order = '';
if(isset($_GET['order']))
	$order = $_GET['order'];
if(isset($_POST['order']))
	$order = $_POST['order'];

$sort_sql = 'order by "id" desc';
$port_class='sort';
$phonenumber_class='sort';
$time_class='sort';

$port_order = 'des';
$phonenumber_order = 'des';
$time_order = 'des';
switch($sort) {
case 'port':
	if($order == 'des') {
		$port_order = 'asc';
		$port_class = 'asc';
		$sort_sql = 'order by "port" asc';
	} else {
		$port_class = 'des';
		$sort_sql = 'order by "port" desc';
	}
	break;
case 'phonenumber':
	if($order == 'des') {
		$phonenumber_order = 'asc';
		$phonenumber_class = 'asc';
		$sort_sql = 'order by "phonenumber" asc';
	} else {
		$phonenumber_class = 'des';
		$sort_sql = 'order by "phonenumber" desc';
	}
	break;
case 'time':
	if($order == 'des') {
		$time_order = 'asc';
		$time_class = 'asc';
		$sort_sql = 'order by "time" asc';
	} else {
		$time_class = 'des';
		$sort_sql = 'order by "time" desc';
	}
	break;
}

/* count */
//echo $filter_sql." ".$sort_sql;
$tmp = $db->try_query("select count(*) from sms $filter_sql $sort_sql");
if($tmp === false){
	$tmp = "";
	$counts = 0;
}else{
	$tmp = @$tmp->fetchArray();
	$counts = $tmp[0];
}
$page_count = ceil($counts / $line_counts);
if($cur_page > $page_count) {
	$cur_page = $page_count;
}
$start_record = ($cur_page-1)*$line_counts;



function del_sms($sms_array)
{
	global $smsinbox_file;
	global $db;
	if(!file_exists($smsinbox_file)) {
		return ;
	}
	
	$str="";
	foreach($sms_array as $id) {
		if(is_numeric($id))
			$str .= "id=$id or ";
	}

	if($str != "") {
		$str = rtrim($str,"or ");
		$hlock = lock_file($smsinbox_file);
		$db->try_query("delete from sms where $str");
		unlock_file($hlock);
	}
}

function del_all_sms()
{
	global $smsinbox_file;
	global $db;
	if(!file_exists($smsinbox_file)) {
		return ;
	}
	$hlock = lock_file($smsinbox_file);
	//$db->try_query("delete from sms");
	$db->drop_alldata();
	unlock_file($hlock);
}

function export_excel()
{
	global $smsinbox_file;
	global $db;
	global $__BRD_HEAD__;
	global $__GSM_HEAD__;
	global $filter_sql;
	global $sort_sql;	

	if(!file_exists($smsinbox_file)) {
		return ;
	}

	$output_name = 'smsinbox.xls';
	$all_time = trim(`date "+%Y:%m:%d:%H:%M:%S"`);
	$item = explode(':', $all_time, 6);
	if(isset($item[5])) {
		$year = $item[0];
		$month = $item[1];
		$date = $item[2];
		$hour = $item[3];
		$minute = $item[4];
		$second = $item[5];
		$output_name = "smsinbox-$year-$month-$date-$hour-$minute-$second.txt";
	}

	ob_clean();
	flush();
	header("Content-type: application/octet-stream"); 
	header("Accept-Ranges: bytes"); 
	//header("Content-type:application/vnd.ms-excel");
	header("Content-Disposition:attachment;filename=$output_name");

	//Add UTF-8 BOM
	$utf8_bom = pack("C*",0xEF,0xBB,0xBF);
	echo $utf8_bom;
	
	$hlock=lock_file($smsinbox_file);
	$results = $db->try_query("select * from sms $filter_sql $sort_sql");
	unlock_file($hlock);

	while($res = @$results->fetchArray()) {
		$res['message'] = str_replace("\n","<\\r>",$res['message']);
		$res['message'] = str_replace("\r","<\\n>",$res['message']);
		/*if(preg_match("/^[\d]{1}$/",$res['port']))
			$res['port'] = $__BRD_HEAD__."1-".$__GSM_HEAD__.$res['port'];*/
		$aliasname = get_gsm_aliasname($res['port'],1);
		if($aliasname == '') $aliasname = 'null';
		$res['port'] = get_gsm_name_by_channel($res['port'],1,false);
		echo $res['port']."\t".$aliasname."\t".$res['phonenumber']."\t".$res['time']."\t".$res['message']."\t".$res['smsc']."\n";
	}
	if($results === false){
		echo "Database is busy now. Please try later.";
	}
	exit(0);
}

if(isset($export_to_excel) && $export_to_excel) {
	export_excel();
}

?>

	<script type="text/javascript" src="/js/functions.js"></script>

	<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
	<link type="text/css" href="/css/jquery-ui-timepicker-addon.css" rel="stylesheet" media="all"/>

	<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>
	<script type="text/javascript" src="/js/jquery-ui-timepicker-addon.js"></script>
	<script type="text/javascript" src="/js/jquery-ui-sliderAccess.js"></script>
	<script type="text/javascript">
	$(function() {
		$( "#start_datetime_filter" ).datetimepicker({dateFormat: "yy/mm/dd", timeFormat: "HH:mm:ss"});
		$( "#end_datetime_filter" ).datetimepicker({dateFormat: "yy/mm/dd", timeFormat: "HH:mm:ss"});
		
		$( "#start_datetime_filter" ).datetimepicker();	
		$( "#end_datetime_filter" ).datetimepicker();
	});
	
	$(document).ready(function(){
		$("#port_table").css({"top":$("#port_filter").offset().bottom, "left":$("#port_filter").offset().left, "position":"absolute"});
		$("#port_filter").click(function(){
			if($("#port_table").css("display") == "none")
				$("#port_table").css("display", "block"); 
			else if($("#port_table").css("display") == "block")
				$("#port_table").css("display", "none");
		});

		$("#port_select").mouseleave(function(){
			$("#port_table").hide();
		});

		$("#check_all").click(function(){
			if($(this).attr("checked") == "checked"){ //check all
				$("#port_filter").attr("value", "all");
				$("input.port_check").each(function(){
					$(this).attr("checked",true);
				});
			}else{
				$("#port_filter").attr("value", "");
				$("input.port_check").each(function(){
					$(this).attr("checked",false);
				});
			}
		});

		$("#do_filter").click(function(){
			if(!check_filter()){
				return;
			}
			$("#port_filter_flag").attr("value", $("#port_filter").attr("value"));
			$("#phone_number_filter_flag").attr("value", $("#phone_number_filter").attr("value"));
			$("#start_datetime_filter_flag").attr("value", $("#start_datetime_filter").attr("value"));
			$("#end_datetime_filter_flag").attr("value", $("#end_datetime_filter").attr("value"));
			$("#message_filter_flag").attr("value", $("#message_filter").attr("value"));
			getpage(1);
		});

		$("#clean_filter").click(function(){
			$("#port_filter").attr("value", "all");
			$("#phone_number_filter").attr("value", "");
			$("#start_datetime_filter").attr("value", "from");
			$("#end_datetime_filter").attr("value", "to");
			$("#message_filter").attr("value", "");

			$("#current_page_flag").attr("value", 1);
			$("#port_filter_flag").attr("value", "all");
			$("#phone_number_filter_flag").attr("value", "");
			$("#start_datetime_filter_flag").attr("value", "from");
			$("#end_datetime_filter_flag").attr("value", "to");
			$("#message_filter_flag").attr("value", "");
			$("#sort_flag").attr("value", "");
			$("#order_flag").attr("value", "");

		});

		$("#input_page").keypress(function(e){
			if(window.event){ // IE
				var keynum = e.keyCode;
			}else if(e.which){ // Netscape/Firefox/Opera
				var keynum = e.which;
			}

			if(keynum == 13) {
				var page = $("#input_page").attr("value");
				$("#current_page_flag").attr("value", page);
				getpage(page);
			}
			
		});

		$("form").keypress(function(e){
			if(window.event){ // IE
				var keynum = e.keyCode;
			}else if(e.which){ // Netscape/Firefox/Opera
				var keynum = e.which;
			}

			if(keynum == 13) {
				return false;
			}
		});
	});

	function getpage(page)
	{
		var url = '<?php echo get_self();?>'+'?';
		if(page == 0)
			page = document.getElementById("current_page_flag").value;
		if(page != '')
			url += "current_page="+page+"&";

		var port_filter = document.getElementById("port_filter_flag").value;
		if(port_filter != 'all')
			url += "port_filter="+port_filter+"&";
		var phone_number_filter = document.getElementById("phone_number_filter_flag").value;
		if(phone_number_filter != '')
			url += "phone_number_filter="+encodeURIComponent(phone_number_filter)+"&";
		var start_datetime_filter = document.getElementById("start_datetime_filter_flag").value;
		if(start_datetime_filter != 'from')
			url += "start_datetime_filter="+encodeURIComponent(start_datetime_filter)+"&";
		var end_datetime_filter = document.getElementById("end_datetime_filter_flag").value;
		if(end_datetime_filter != 'to')
			url += "end_datetime_filter="+encodeURIComponent(end_datetime_filter)+"&";
		var message_filter = document.getElementById("message_filter_flag").value;
		if(message_filter != '')
			url += "message_filter="+encodeURIComponent(message_filter)+"&";


		var sort = document.getElementById("sort_flag").value;
		if(sort != '')
			url += "sort="+sort+"&";
		var order = document.getElementById("order_flag").value;
		if(order != '')
			url += "order="+order+"&";

		window.location.href = url;
	}

	function sort_click(obj, type)
	{
		document.getElementById("sort_flag").value=type;
		if(document.getElementById("order_flag").value == "des")
			document.getElementById("order_flag").value = "asc";
		else
			document.getElementById("order_flag").value = "des";
		getpage(0);
	}
	

	function set_check(check_obj,target_id)
	{
		//get this module type
		var port_name = check_obj.id;   //check_obj.id :umts-1.1...
		var num = port_name.indexOf("-");
		var type = port_name.substr(0,num);
		
		var target_obj = document.getElementById(target_id); 
		if(check_obj.checked == true){  
			if (type == "umts") {
				check_obj.value = check_obj.value.replace('gsm',type);
			}
			if (type == "null") {
				check_obj.value = check_obj.value.replace('gsm',type);
			} 
			if(target_obj.value == "all" || target_obj.value == ""){ 
				target_obj.value = check_obj.value; 
			}else{
				target_obj.value += "," + check_obj.value; 
			} 
		}else{
			if (type == "umts") {
				check_obj.value = check_obj.value.replace('gsm',type);
			}
			if (type == "null") {
				check_obj.value = check_obj.value.replace('gsm',type);
			}
			if(target_obj.value == "all"){
				var inputs = document.getElementsByTagName("input");     
				for(var i=0;i<inputs.length;i++){
					//alert("for "+i+" "+inputs[i].getAttribute("type")+" "+inputs[i].getAttribute("name"));
					if( (inputs[i].getAttribute("type")=="checkbox" || inputs[i].getAttribute("type")=="Checkbox") && inputs[i].getAttribute("name")=="checkbox_name[]" && inputs[i] != check_obj){
						inputs[i].checked = true;
						set_check(inputs[i],target_id);
					}
				}		
			}
			if(target_obj.value.indexOf("," + check_obj.value + ",") != -1){ 
				target_obj.value = target_obj.value.replace("," + check_obj.value,''); 
			}else if(target_obj.value.indexOf("," + check_obj.value) != -1){ 
				 target_obj.value = target_obj.value.replace("," + check_obj.value,''); 
			}else if(target_obj.value.indexOf(check_obj.value + ",") != -1){ 
				target_obj.value = target_obj.value.replace(check_obj.value + ",",''); 
			}else if(target_obj.value.indexOf(check_obj.value) != -1){ 
				target_obj.value = target_obj.value.replace(check_obj.value,''); 
			} 
		} 
	}

	function check_filter()
	{		
		
		//return true;
		var phone_number = document.getElementById("phone_number_filter").value;
		//var pattern = /^[-+*_0-9]*$/;
		var pattern = /^[-+*_@\d]*$/
		
		var result = pattern.exec(phone_number);
		if(result != null){
			return true;
		}else{
			alert("invalid phone number!");
			return false;
		}
	}

	</script>

	<!-- Filter table -->
<?php
	$filter_target = "where (port='1' OR port='".$__BRD_HEAD__."1-".$__GSM_HEAD__."1'";
	for ($i = 2; $i <= $__GSM_SUM__; $i++) {
		$filter_target .= " OR port='$i' OR port='".$__BRD_HEAD__."1-".$__GSM_HEAD__."$i'";
	}
	$filter_target .= ")";
	
	if($cluster_info['mode'] == 'master' && $filter_sql == '' && $counts <= 0 && $tmp != ""){
		show_msg(language('NO RECORD'));
	} else if($cluster_info['mode'] != 'master' 
		//&& $filter_sql == "where (port='1' OR port='2' OR port='3' OR port='4' OR port='Board-1-gsm-1' OR port='Board-1-gsm-2' OR port='Board-1-gsm-3' OR port='Board-1-gsm-4')" 
		&& $filter_sql == $filter_target
		&& $counts <= 0 && $tmp != ""){
		show_msg(language('NO RECORD'));
	}else{
?>
	<table width="100%" class="tsort" style="table-layout:fixed;" >
		<tr>
			<th width='05%' class="nosort"></th>
			<th width='15%' class="nosort"><?php echo language('Port');?></th>
			<th width='15%' class="nosort"><?php echo language('Delivery Address');?></th>
			<th width='25%' class="nosort"><?php echo language('Time');?></th>
			<th width='40%' class="nosort"><?php echo language('Message Keywords');?></th>
		</tr>
		<tr>
			<td>
			</td>
			<!-- port filter -->
			<td>
			<div id="port_select" style="zoom:1">
			<input type="text" id="port_filter" name="port_filter" style="width:95%;" value="<?php echo $port_filter;?>" readonly /> 
			<div> <!-- solve IE absolute position problem -->
			<table id="port_table" class="tsort" style="display:none;background:#FFFFFF;" > 
<?php
					$j = 1;
					for($i=1;$i<=$__GSM_SUM__;$i++){
						$port_name = get_gsm_name_by_channel($i);
						if(strstr($port_name, 'null')) continue;
						
						if($port_filter == "all")
							$port_check[$port_name] = "checked";
						else if(!isset($port_check[$port_name]))
							$port_check[$port_name] = "";
						
						if($j%4 == 1){echo "<tr>";}
						echo "<td><input type='checkbox' name='checkbox_name[]' class='port_check' value='$port_name' id='$port_name' onclick=\"set_check(this,'port_filter')\" $port_check[$port_name]>";
						echo $port_name;
						echo '</td>';
						if($j%4 == 0){echo "</tr>";}
						$j++;
					}
					
					//$cluster_info = get_cluster_info();
					if($__deal_cluster__){
						if($cluster_info['mode'] == 'master') {
							for($b=2; $b<=$__BRD_SUM__; $b++) {
								if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
									for ($line=0;$line<$__GSM_SUM__/8;$line++) {
										echo "<tr>";
										for($i=1+$line*8; $i<=(8+$line*8); $i++) {
											if ($i>$__GSM_SUM__) break;
											echo "<td>";
											//$name = $__BRD_HEAD__.$b."-".$__GSM_HEAD__.$i;
											$name_org = get_gsm_name_by_channel($i,$b);
											$name = trans_to_gsm($name_org);
											if($port_filter == "all")
												$port_check[$name] = "checked";
											else if(!isset($port_check[$name]))
												$port_check[$name] = "";
											echo "<input type=\"Checkbox\" class=\"port_check\" name=\"checkbox_name[]\" value=\"$name\" id=\"".$name_org."\" onclick=\"set_check(this,'port_filter')\" $port_check[$name]>";
											//echo $name;
											echo get_gsm_name_by_channel($i,$b);
											echo "</td>";
										}
										echo "</tr>";
									}
								}
							}
						}
					}
				?>
				<tr><td colspan=8><input type="Checkbox" id="check_all" <?php if($port_filter == "all") echo "checked";?>><?php echo language('All');?></td></tr>
			</table>   
			</div>
			</div>

			</td>
			<td>
			
				<input type="text" id="phone_number_filter" name="phone_number_filter"  style='width:95%' value="<?php echo $phone_number_filter;?>" oninput="this.value=this.value.replace(/[^-+*_@\d]*/g,'')">

			</td>
			<td>
				<input type="text" id="start_datetime_filter" name="start_datetime_filter" style='width:46%' value="<?php echo $start_datetime_filter;?>"/> 
				<input type="text" id="end_datetime_filter" name="end_datetime_filter" style='width:46%' value="<?php echo $end_datetime_filter;?>"/>
			</td>
			<td>
				<input type="text" id="message_filter" name="message_filter"  style='width:98%' value="<?php echo $message_filter; ?>">
			</td>

		</tr>
	</table>

	<br/>
	<input type="button" id="do_filter" value="<?php echo language('Filter');?>"/> 
	<input type="button" id="clean_filter" value="<?php echo language('Clean Filter');?>"/> 
	<br/>
	
	<div id="newline"></div>
<?php
	} /* if($filter_sql == '' && $counts <= 0) */
?>
	<!-- GSM SMS Inbox -->	
	<form id="mainform" enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
<?php
	$sql_requery="select * from sms $filter_sql $sort_sql limit $start_record, $line_counts";
	$results = $db->try_query($sql_requery);
	
	if($results === false || $tmp == ""){
		show_msg(language('DATABASE BUSY'));
	}else if($results !== false && $tmp != "" && $filter_sql != '' && $counts <= 0){
		show_msg(language('NO RECORD'));
	}
	
	//phone number switch
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$phonenum_res = $aql->query("select * from sim_query.conf");

	if($counts > 0) {
		echo "<span style=\"font-weight:bold\">";echo language('Total Records');echo ": $counts";echo "</span>";
		$flag = 0;
?>
	<table width="100%" class="tsort">
		<tr>
			<th width='03%' class="nosort">
				<input type="Checkbox" name="selall" onclick="selectAll(this.checked,'sms[]')" />
			</th>
			<th width='8%' class="<?php echo $port_class?>" onclick="sort_click(this,'port')"><?php echo language('Port');?></th>
			
			<?php if($phonenum_res['general']['phonenum_switch'] != 'on'){ ?>
			<th width='10%'><?php echo language('Phone Number');?></th>
			<?php } ?>
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			<th><?php echo language('Local Number');?></th>
			<?php } ?>
			
			<th width='12%' class="<?php echo $phonenumber_class?>" onclick="sort_click(this,'phonenumber')"><?php echo language('Delivery Address');?></th>
			<th width='12%' class="<?php echo $time_class?>" onclick="sort_click(this,'time')"><?php echo language('Time');?></th>
			<th width='35%' class="nosort"><?php echo language('Message');?></th>
			<th width='12%' class="nosort"><?php echo language('SMSC Global Title');?></th>
		</tr>
		<?php
		if($results){
			while($res = @$results->fetchArray()) {
				$flag ++;
				
				$port = $res['port'];
				$phonenum = '';
				if(($phonenum_res[$port]['query_type']&240) != 0){
					exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $port",$phone_output);
					$phonenum = $phone_output[0];
					$phone_output = '';
				}
		?>
		<tr>
			<td> 
				<input type="Checkbox" name="sms[]" value="<?php echo $res['id']?>" />					 
			</td>
			<td> 
				<?php 
					echo get_gsm_name_by_channel($port,1,false);
				?>
			</td>
			
			<?php if($phonenum_res['general']['phonenum_switch'] != 'on'){ ?>
			<td>
				<?php echo get_gsm_aliasname($port, 1);?>
			</td>
			<?php } ?>
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			<td><?php echo $phonenum;?></td>
			<?php } ?>
			
			<td> 
				<?php echo $res['phonenumber'];?>
			</td>
			<td> 
				<?php echo $res['time'];?>
			</td>
			<td> 
				<textarea rows="1" style="width:99%;" readonly ><?php echo $res['message'];?></textarea>
			</td>
			<td>
				<?php echo $res['smsc'];?>
			</td>
		</tr>
		<?php
				}
			}
		?>
	</table>
<?php
		if($flag == 0){
			show_msg(language('DATABASE BUSY'));
		}
	}

	/* page selector */

	if($page_count >= 2) {
		echo '<br/>';
		echo '<div class="pg">';

		if( $cur_page > 1 ) {
			$page = $cur_page - 1;
			echo "<a title=\"";echo language('Previous page');echo "\" style=\"cursor:pointer;\" class=\"prev\" onclick=\"getpage($page)\"></a>";
		} else {
			
		}
			
		if($cur_page-5 > 1) {
			$s = $cur_page-5;
		} else {
			$s = 1;
		}

		if($s + 10 < $page_count) {
			$e = $s + 10;
		} else {
			$e = $page_count;
		}

		for($i = $s; $i <= $e; $i++) {
			if($i != $cur_page) {
				echo "<a onclick=\"getpage($i)\" style=\"cursor:pointer;\">$i</a>";
			} else {
				echo "<strong>$cur_page</strong>";
			}
		}

		if( $cur_page < $page_count ) {
			$page = $cur_page + 1;
			echo "<a title=\"";echo language('Next page');echo "\" style=\"cursor:pointer;\" class=\"nxt\" onclick=\"getpage($page)\"></a>";
		} else {
			
		}

		echo "<label>";
		echo "<input type=\"text\" id=\"input_page\" name=\"page\" value=\"$cur_page\" size=\"2\" class=\"px\"  title=\"";
		echo language('input page help','Please input your page number, and press [Enter] to skip to.');echo "\" >";
		echo "<span title=\"";echo language('total pages');echo ": $page_count\"> / $page_count</span>";echo "</label>";
		echo "<a title=\"";echo language('goto input page');echo "\" style=\"cursor:pointer;\" onclick=\"getpage(document.getElementById('input_page').value)\">";
		echo language("go");echo "</a>";
		echo '</div>';
	}else if ($page_count == 1){
		echo '<br/>';
		echo '<div class="pg">';
		echo "<strong>1</strong>";
		echo '</div>';
	}

	if($counts > 0) {
?>
	<div id="newline"></div>
	<br/>

	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" value='<?php echo language('Delete');?>' 
		onclick="document.getElementById('send').value='Delete';return confirm('Are you sure to delete you selected ?')"/> 
	<input type="submit" value='<?php echo language('Clean Up');?>' 
		onclick="document.getElementById('send').value='Clean Up';return confirm('Are you sure to delete all smss ?')"/> 
	<input type="submit" value='<?php echo language('Export');?>' 
		onclick="document.getElementById('send').value='Export';"/>

<?php
	}
?>

	<input type="hidden" id="current_page_flag" name="current_page" value="<?php echo $cur_page;?>" />
	<input type="hidden" id="port_filter_flag" name="port_filter" value="<?php echo $port_filter;?>" />
	<input type="hidden" id="phone_number_filter_flag" name="phone_number_filter" value="<?php echo $phone_number_filter;?>" />
	<input type="hidden" id="start_datetime_filter_flag" name="start_datetime_filter" value="<?php echo $start_datetime_filter;?>" />
	<input type="hidden" id="end_datetime_filter_flag" name="end_datetime_filter" value="<?php echo $end_datetime_filter;?>" />
	<input type="hidden" id="message_filter_flag" name="message_filter" value="<?php echo $message_filter;?>" />
	<input type="hidden" id="sort_flag" name="sort" value="<?php echo $sort;?>" />
	<input type="hidden" id="order_flag" name="order" value="<?php echo $order;?>" />

	</form>

	<div id="newline"></div>

<script type="text/javascript">
$(document).ready(function(){
	$("textarea").each(function(){
		$(this).css("height", this.scrollHeight);
	});
});
</script>
<?php require("/www/cgi-bin/inc/boot.inc");?>

