<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/cdrdb.php");
include_once("/www/cgi-bin/inc/aql.php");
?>

<?php
$cdr_log_file = '/data/log/cdr.db';
$line_counts = 10;
if(!file_exists($cdr_log_file)) {
	require("/www/cgi-bin/inc/boot.inc");
	exit(0);
}

$db = new CDRDB();
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

/****************************************************************/
if($_POST) {
	if(isset($_POST['send'])) {
		if($_POST['send'] == 'Delete') {
			if(isset($_POST['log'])) {
				del_log($_POST['log']);
			}
		} else if($_POST['send'] == 'Clean Up') {
			del_all_log();
		}
	}
}
/****************************************************************/

function export_excel()
{
	global $cdr_log_file;
	global $db;
	global $__GSM_HEAD__;
	global $__BRD_HEAD__;
	global $filter_sql;
	global $sort_sql;

	if(!file_exists($cdr_log_file)) {
		return ;
	}

	$output_name = 'cdr.csv';
	$all_time = trim(`date "+%Y:%m:%d:%H:%M:%S"`);
	$item = explode(':', $all_time, 6);
	if(isset($item[5])) {
		$year = $item[0];
		$month = $item[1];
		$date = $item[2];
		$hour = $item[3];
		$minute = $item[4];
		$second = $item[5];
		$output_name = "cdr-$year-$month-$date-$hour-$minute-$second.csv";
	}

	ob_clean();
	flush();
	header("Content-type:application/vnd.ms-excel");
	header("Content-Disposition:attachment;filename=$output_name");

	$hlock=lock_file($cdr_log_file);
	$results = $db->try_query("select * from cdr $filter_sql $sort_sql");
	unlock_file($hlock);
	
	$aql = new aql();
	$simemusvr_conf = "/etc/asterisk/simemusvr.conf";
	$aql->set('basedir','/etc/asterisk');
	$simemu_res = $aql->query("select * from simemusvr.conf");

	$sys_type = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys sys_type");
	if($simemu_res['SimEmuSvr']['simemusvr_switch'] == 'yes'){
		echo "Caller ID,Callee ID,From,To,Sim Number,Start Time,Duration,Result,Bank,Slot\n";
	}else{
		echo "Caller ID,Callee ID,From,To,Sim Number,Start Time,Duration,Result\n";
	}
	while($res = @$results->fetchArray()) {
		if(get_channel_type($res['from']) == 'gsm') {
		//	$res['from'] = change_port_form($res['from']);
			$res['from'] = change_sms_routing_form_new_for_cdr_excel($res['from'],$sys_type);
		} 
		if(get_channel_type($res['to']) == 'gsm') {
		//	$res['to'] = change_port_form($res['to']);
			$res['to'] = change_sms_routing_form_new_for_cdr_excel($res['to'],$sys_type);
		}
		if($simemu_res['SimEmuSvr']['simemusvr_switch'] == 'yes'){
			echo $res['callerid']."\t,".$res['calleeid']."\t,".$res['from']."\t,".$res['to']."\t,".$res['simnum']."\t,".$res['starttime']."\t,".format_time($res['duration'])."\t,".$res['result']."\t,".$res['bank']."\t,".$res['slot']."\n";
		}else{
			echo $res['callerid']."\t,".$res['calleeid']."\t,".$res['from']."\t,".$res['to']."\t,".$res['simnum']."\t,".$res['starttime']."\t,".format_time($res['duration'])."\t,".$res['result']."\n";
		}
	}
	if($results === false){
		echo "Database is busy now. Please try later.";
	}
	exit(0);
}

/* page */
$cur_page = 1;
if(isset($_GET['current_page']) && is_numeric($_GET['current_page']))
	$cur_page = $_GET['current_page'];


/* filter */
$filter_sql = '';

$caller_from_filter = "";
if(isset($_GET['caller_from_filter']))
	$caller_from_filter = $_GET['caller_from_filter'];
else if(isset($_POST['caller_from_filter']))
	$caller_from_filter = $_POST['caller_from_filter'];
if($caller_from_filter != '' && preg_match("/^[-+*@.\w]*$/", $caller_from_filter)){
	$caller_from_filter_sql = str_replace('*', '%', $caller_from_filter);
	$filter_sql = "where callerid like '".$caller_from_filter_sql."'";
}
$caller_to_filter = "";
if(isset($_GET['caller_to_filter']))
	$caller_to_filter = $_GET['caller_to_filter'];
else if (isset($_POST['caller_to_filter']))
	$caller_to_filter = $_POST['caller_to_filter'];
if($caller_to_filter != '' && preg_match("/^[-+*@.\w]*$/",$caller_to_filter)){
	$caller_to_filter_sql = str_replace('*', '%', $caller_to_filter);
	if($filter_sql == '')
		$filter_sql = "where calleeid like '".$caller_to_filter_sql."'";
	else
		$filter_sql .= " and calleeid like '".$caller_to_filter_sql."'";
}
// Getting the sys_type from /tmp/hw_info.conf
$sys_type = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys sys_type");

$channel_from_filter = "";
if(isset($_GET['channel_from_filter']))
	$channel_from_filter = trim($_GET['channel_from_filter']);
else if(isset($_POST['channel_from_filter']))
	$channel_from_filter = trim($_POST['channel_from_filter']);
if($channel_from_filter != '' && preg_match("/^[-+*@.\s\w]*$/",$channel_from_filter)){
	if(preg_match('/'.$__GSM_HEAD__.'/i',$channel_from_filter,$match) 
		|| preg_match('/'.$__UMTS_HEAD__.'/i',$channel_from_filter,$match)
		|| preg_match('/'.$__CDMA_HEAD__.'/i', $channel_from_filter, $match)
		|| preg_match('/'.$__LTE_HEAD__.'/i', $channel_from_filter, $match)){
		$match_board = explode('-',$match[0]);
	
		if($sys_type == '1'){
			$channel_from_filter_arr = explode('-', $channel_from_filter, 2);
			$channel_from_filter_current = $channel_from_filter_arr[0] . '-1.' . $channel_from_filter_arr[1];
		} else if($sys_type == '3'){
			$channel_from_filter_arr = explode('-',	$channel_from_filter, 2);
			$port_arr = explode('.', $channel_from_filter_arr[1], 2);
			$channel_from_filter_current = $channel_from_filter_arr[0] . '-1.' . $port_arr[1];
		} else {
			$channel_from_filter_current = $channel_from_filter;
		}
		if (isset($match_board[0]) && isset($match_board[1])) {
			$channel_from_filter_sql = $channel_from_filter_current;
			if(strstr($channel_from_filter_current, "umts")){
				$channel_from_filter_sql = str_replace('umts','gsm',$channel_from_filter_sql);
			}
			if(strstr($channel_from_filter_current, "cdma")){
				$channel_from_filter_sql = str_replace('cdma','gsm',$channel_from_filter_sql);
			}
			if(strstr($channel_from_filter_current, "lte")){
				$channel_from_filter_sql = str_replace('lte','gsm',$channel_from_filter_sql);
			}
			$channel_from_filter_sql = str_replace('*', '%', $channel_from_filter_sql);
			$channel_from_filter_sql .= '%';
		} else {
			$channel_from_filter_sql = '';
		}
		if($filter_sql == '')
			$filter_sql = "where \"from\" like '".$channel_from_filter_sql."'";
		else
			$filter_sql .= " and \"from\" like '".$channel_from_filter_sql."'";	
	}else if(preg_match('/['.$__GSM_HEAD__.']+/',$channel_from_filter)){
		$channel_from_filter_sql = '';
		$from_flag = 0;
		for ($b=1;$b<=$__BRD_SUM__;$b++) {
			if ( strstr($__MODULE_HEAD_ARRAY__[$b][1],$channel_from_filter) ) {
				$gsm_head = strstr($__MODULE_HEAD_ARRAY__[$b][1],$channel_from_filter);
				if ($from_flag == 0) {
					$filter_sql_type = " and ";
				} else {
					$filter_sql_type = " or ";
				}

				$channel_from_filter_sql = str_replace('umts','gsm',$gsm_head);
				$channel_from_filter_sql = str_replace('cdma','gsm',$gsm_head);
				$channel_from_filter_sql .= $b;
				$channel_from_filter_sql = str_replace('*', '%', $channel_from_filter_sql);
				$channel_from_filter_sql = '%'.$channel_from_filter_sql.'%';
				$channel_from_filter_sql = $channel_from_filter_sql;
				$from_flag = 1;
			}
			if ($channel_from_filter_sql != '') {
				if ($filter_sql == ''){
					$filter_sql =  "where \"from\" like '".$channel_from_filter_sql."'";
				} else {
					$filter_sql .=  $filter_sql_type."\"from\" like '".$channel_from_filter_sql."'";
				}
			}
			$channel_from_filter_sql = '';
		}
		if ($from_flag == 0) {
			$channel_from_filter_sql = str_replace('*', '%', $channel_from_filter);
			if($filter_sql == '')
				$filter_sql = "where \"from\" like '".$channel_from_filter_sql."'";
			else
				$filter_sql .= " and \"from\" like '".$channel_from_filter_sql."'";
		}
	}else{
		$channel_from_filter_sql = str_replace('*', '%', $channel_from_filter);
		if($filter_sql == '')
			$filter_sql = "where \"from\" like '".$channel_from_filter_sql."'";
		else
			$filter_sql .= " and \"from\" like '".$channel_from_filter_sql."'";
	}

}

$channel_to_filter = "";
if(isset($_GET['channel_to_filter']))
	$channel_to_filter = trim($_GET['channel_to_filter']);
else if(isset($_POST['channel_to_filter']))
	$channel_to_filter = trim($_POST['channel_to_filter']);	
if($channel_to_filter != '' && preg_match("/^[-+*@.\s\w]*$/",$channel_to_filter)){
	if(preg_match('/'.$__GSM_HEAD__.'/i',$channel_to_filter,$match) 
		|| preg_match('/'.$__UMTS_HEAD__.'/i',$channel_to_filter,$match)
		|| preg_match('/'.$__CDMA_HEAD__.'/i', $channel_to_filter, $match)
		|| preg_match('/'.$__LTE_HEAD__.'/i', $channel_to_filter, $match)){
		$match_board = explode('-',$match[0]);
		if($sys_type == '1'){
			$channel_to_filter_arr = explode('-', $channel_to_filter, 2);
			$channel_to_filter_current = $channel_to_filter_arr[0] . '-1.' . $channel_to_filter_arr[1];
		} else if($sys_type == '3'){
			$channel_to_filter_arr = explode('-',	$channel_to_filter, 2);
			$port_arr = explode('.', $channel_to_filter_arr[1], 2);
			$channel_to_filter_current = $channel_to_filter_arr[0] . '-1.' . $port_arr[1];
		} else {
			$channel_to_filter_current = $channel_to_filter;
		}
		if (isset($match_board[0]) && isset($match_board[1])) {
			$channel_to_filter_sql = $channel_to_filter_current;
			if(strstr($channel_to_filter_current, "umts")){
				$channel_to_filter_sql = str_replace('umts','gsm',$channel_to_filter_sql);
			}
			if(strstr($channel_to_filter_current, "cdma")){
				$channel_to_filter_sql = str_replace('cdma','gsm',$channel_to_filter_sql);
			}
			if(strstr($channel_to_filter_current, "lte")){
				$channel_to_filter_sql = str_replace('lte','gsm',$channel_to_filter_sql);
			}
			$channel_to_filter_sql = str_replace('*', '%', $channel_to_filter_sql);
			$channel_to_filter_sql .= '%';			
		} else {
			$channel_to_filter_sql = '';
		}
		if($filter_sql == '')
			$filter_sql = "where \"to\" like '".$channel_to_filter_sql."'";
		else
			$filter_sql .= " and \"to\" like '".$channel_to_filter_sql."'";
	}else if(preg_match('/['.$__GSM_HEAD__.']+/',$channel_to_filter)){
		$channel_to_filter_sql = '';
		$to_flag = 0;
		for ($b=1;$b<=$__BRD_SUM__;$b++) {
			if ( strstr($__MODULE_HEAD_ARRAY__[$b][1],$channel_to_filter) ) {
				$gsm_head = strstr($__MODULE_HEAD_ARRAY__[$b][1],$channel_to_filter);
				if ($to_flag == 0) {
					$filter_sql_type = " and ";
				} else {
					$filter_sql_type = " or ";
				}

				$channel_to_filter_sql = str_replace('umts','gsm',$gsm_head);
				$channel_to_filter_sql = str_replace('cdma','gsm',$gsm_head);
				$channel_to_filter_sql .= $b;
				$channel_to_filter_sql = str_replace('*', '%', $channel_to_filter_sql);
				$channel_to_filter_sql = '%'.$channel_to_filter_sql.'%';
				$channel_to_filter_sql = $channel_to_filter_sql;
				$to_flag = 1;
			}
			if ($channel_to_filter_sql != '') {
				if ($filter_sql == ''){
					$filter_sql =  "where \"to\" like '".$channel_to_filter_sql."'";
				} else {
					$filter_sql .=  $filter_sql_type."\"to\" like '".$channel_to_filter_sql."'";
				}
			}
			$channel_to_filter_sql = '';
		}
		if ($to_flag == 0) {
			$channel_to_filter_sql = str_replace('*', '%', $channel_to_filter);
			if($filter_sql == '')
				$filter_sql = "where \"to\" like '".$channel_to_filter_sql."'";
			else
				$filter_sql .= " and \"to\" like '".$channel_to_filter_sql."'";
		}
	}else{
		$channel_to_filter_sql = str_replace('*', '%', $channel_to_filter);
		if($filter_sql == '')
			$filter_sql = "where \"to\" like '".$channel_to_filter_sql."'";
		else
			$filter_sql .= " and \"to\" like '".$channel_to_filter_sql."'";
	}
}

$starttime_from_filter = "from";
if(isset($_GET['starttime_from_filter']))
	$starttime_from_filter = $_GET['starttime_from_filter'];
else if(isset($_POST['starttime_from_filter']))
	$starttime_from_filter = $_POST['starttime_from_filter'];
if($starttime_from_filter != '' && $starttime_from_filter != 'from') {
	if($filter_sql == '')
		$filter_sql = "where starttime >= '".$starttime_from_filter."'";
	else
		$filter_sql .= " and starttime >= '".$starttime_from_filter."'";
}

$starttime_to_filter = "to";
if(isset($_GET['starttime_to_filter']))
$starttime_to_filter = $_GET['starttime_to_filter'];
else if(isset($_POST['starttime_to_filter']))
	$starttime_to_filter = $_POST['starttime_to_filter'];
if($starttime_to_filter != '' && $starttime_to_filter != 'to') {
	if($filter_sql == '')
		$filter_sql = "where starttime <= '".$starttime_to_filter."'";
	else
		$filter_sql .= " and starttime <= '".$starttime_to_filter."'";
}

$duration_from_filter = "from";
if(isset($_GET['duration_from_filter']))
	$duration_from_filter = $_GET['duration_from_filter'];
elseif(isset($_POST['duration_from_filter']))
	$duration_from_filter = $_POST['duration_from_filter'];
if($duration_from_filter != '' && $duration_from_filter != 'from'){
	$digital_duration_from_filter = time_to_digital($duration_from_filter);
	if($filter_sql == '')
		$filter_sql = "where duration >= '".$digital_duration_from_filter."'";
	else
		$filter_sql .= " and duration >= '".$digital_duration_from_filter."'";
}

$duration_to_filter = "to";
if(isset($_GET['duration_to_filter']))
	$duration_to_filter = $_GET['duration_to_filter'];
elseif(isset($_POST['duration_to_filter']))
	$duration_to_filter = $_POST['duration_to_filter'];
if($duration_to_filter != '' && $duration_to_filter != 'to'){
	$digital_duration_to_filter = time_to_digital($duration_to_filter);
	if($filter_sql == '')
		$filter_sql = "where duration <= '".$digital_duration_to_filter."'";
	else
		$filter_sql .= " and duration <= '".$digital_duration_to_filter."'";
}

$result_filter = "all";
if(isset($_GET['result_filter']))
	$result_filter = $_GET['result_filter'];
elseif(isset($_POST['result_filter']))
	$result_filter = $_POST['result_filter'];
if($result_filter != '' && $result_filter != 'all'){
	if($filter_sql == '')
		$filter_sql = "where result='".$result_filter."'";
	else
		$filter_sql .= " and result='".$result_filter."'";
}

/* sort */
$sort = '';
if(isset($_GET['sort'])){
	$sort = $_GET['sort'];
}else if(isset($_POST['sort'])){
	$sort = $_POST['sort'];
}

$order = '';
if(isset($_GET['order']))
	$order = $_GET['order'];

$sort_sql = 'order by "id" desc';
$callerid_class='sort';
$calleeid_class='sort';
$from_class='sort';
$to_class='sort';
$starttime_class='sort';
$duration_class='sort';
$result_class='sort';
$bank_class='sort';
$sim_class='sort';

$callerid_order = 'des';
$calleeid_order = 'des';
$from_order = 'des';
$to_order = 'des';
$starttime_order = 'des';
$duration_order = 'des';
$result_order = 'des';
switch($sort) {
case 'callerid':
	if($order == 'des') {
		$callerid_order = 'asc';
		$callerid_class = 'asc';
		$sort_sql = 'order by "callerid" asc';
	} else {
		$callerid_class = 'des';
		$sort_sql = 'order by "callerid" desc';
	}
	break;
case 'calleeid':
	if($order == 'des') {
		$calleeid_order = 'asc';
		$calleeid_class = 'asc';
		$sort_sql = 'order by "calleeid" asc';
	} else {
		$calleeid_class = 'des';
		$sort_sql = 'order by "calleeid" desc';
	}
	break;
case 'from':
	if($order == 'des') {
		$from_order = 'asc';
		$from_class = 'asc';
		$sort_sql = 'order by "from" asc';
	} else {
		$from_class = 'des';
		$sort_sql = 'order by "from" desc';
	}
	break;
case 'to':
	if($order == 'des') {
		$to_order = 'asc';
		$to_class = 'asc';
		$sort_sql = 'order by "to" asc';
	} else {
		$to_class = 'des';
		$sort_sql = 'order by "to" desc';
	}
	break;
case 'starttime':
	if($order == 'des') {
		$starttime_order = 'asc';
		$starttime_class = 'asc';
		$sort_sql = 'order by "starttime" asc';
	} else {
		$starttime_class = 'des';
		$sort_sql = 'order by "starttime" desc';
	}
	break;
case 'duration':
	if($order == 'des') {
		$duration_order = 'asc';
		$duration_class = 'asc';
		$sort_sql = 'order by "duration" asc';
	} else {
		$duration_class = 'des';
		$sort_sql = 'order by "duration" desc';
	}
	break;
case 'result':
	if($order == 'des') {
		$result_order = 'asc';
		$result_class = 'asc';
		$sort_sql = 'order by "result" asc';
	} else {
		$result_class = 'des';
		$sort_sql = 'order by "result" desc';
	}
	break;
case 'bank':
	if($order == 'des') {
		$bank_order = 'asc';
		$bank_class = 'asc';
		$sort_sql = 'order by "bank" asc';
	} else {
		$bank_class = 'des';
		$sort_sql = 'order by "bank" desc';
	}
	break;
case 'slot':
	if($order == 'des') {
		$sim_order = 'asc';
		$sim_class = 'asc';
		$sort_sql = 'order by "slot" asc';
	} else {
		$sim_class = 'des';
		$sort_sql = 'order by "slot" desc';
	}
	break;
}

/* count */
//echo "filter_sql=$filter_sql sort_sql=$sort_sql";
$tmp = $db->try_query("select count(*) from cdr $filter_sql $sort_sql");
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

function del_log($log_array)
{
	global $cdr_log_file;
	global $db;
	if(!file_exists($cdr_log_file)) {
		return ;
	}
	
	$str="";
	foreach($log_array as $id) {
		if(is_numeric($id))
			$str .= "id=$id or ";
	}

	if($str != "") {
		$str = rtrim($str,"or ");
		$hlock = lock_file($cdr_log_file);
		$db->try_query("delete from cdr where $str");
		unlock_file($hlock);
	}
}

function del_all_log()
{
	global $cdr_log_file;
	global $db;
	if(!file_exists($cdr_log_file)) {
		return ;
	}
	$hlock = lock_file($cdr_log_file);
	$db->drop_alldata();
	unlock_file($hlock);
}

function time_to_digital($time)
{
	$array = explode(":", $time);
	$hour = $array[0];
	$minute = $array[1];
	$second = $array[2];
	
	$second_total = $hour*60*60 + $minute*60 + $second;
	
	return $second_total;
}
 
if($_POST['send'] == 'Export') {
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
		$( "#starttime_from_filter" ).datetimepicker({dateFormat: "yy-mm-dd", timeFormat: "HH:mm:ss"});
		$( "#starttime_to_filter" ).datetimepicker({dateFormat: "yy-mm-dd", timeFormat: "HH:mm:ss"});
		$( "#duration_from_filter" ).timepicker({timeFormat: "HH:mm:ss"});
		$( "#duration_to_filter" ).timepicker({timeFormat: "HH:mm:ss"});

		$( "#starttime_from_filter" ).datetimepicker();
		$( "#starttime_tofilter" ).datetimepicker();
		$( "#duration_from_filter" ).timepicker();
		$( "#duration_to_filter" ).timepicker();
		
		var from_flag = "<?php echo $_GET['from_flag'];?>";
		if(from_flag == 1){
			$(".gsm_inbound_table").show();
			$(".gsm_outbound_table").hide();
		}
	});

	$(document).ready(function(){
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
	
	function getpage(page,to_statis_page,from_statis_page,from_flag)
	{
		if(from_flag == 1){
			var url = '<?php echo get_self();?>'+'?from_flag=1&';
		}else{
			var url = '<?php echo get_self();?>'+'?';
		}
		
	
		if(page == 0)
			page = document.getElementById("current_page_flag").value;
		
		if(to_statis_page == 0 || to_statis_page == undefined)
			to_statis_page = document.getElementById("to_statis_cur_page").value;
		
		if(from_statis_page == 0 || from_statis_page == undefined)
			from_statis_page = document.getElementById("from_statis_cur_page").value;
		
		if(page != '')
			url += "current_page="+page+"&to_statis_cur_page="+to_statis_page+"&from_statis_cur_page="+from_statis_page+"&";

		var caller_from_filter = document.getElementById("caller_from_filter_flag").value;
		if(caller_from_filter != '')
			url += "caller_from_filter="+encodeURIComponent(caller_from_filter)+"&";
		var caller_to_filter = document.getElementById("caller_to_filter_flag").value;
		if(caller_to_filter != '')
			url += "caller_to_filter="+encodeURIComponent(caller_to_filter)+"&";
		var channel_from_filter = document.getElementById("channel_from_filter_flag").value;
		if(channel_from_filter != 'all')
			url += "channel_from_filter="+encodeURIComponent(channel_from_filter)+"&";
		var channel_to_filter = document.getElementById("channel_to_filter_flag").value;
		if(channel_to_filter != 'all')
			url += "channel_to_filter="+encodeURIComponent(channel_to_filter)+"&";
		var starttime_from_filter = document.getElementById("starttime_from_filter_flag").value;
		if(starttime_from_filter != 'from')
			url += "starttime_from_filter="+encodeURIComponent(starttime_from_filter)+"&";
		var starttime_to_filter = document.getElementById("starttime_to_filter_flag").value;
		if(starttime_to_filter != 'to')
			url += "starttime_to_filter="+encodeURIComponent(starttime_to_filter)+"&";
		var duration_from_filter = document.getElementById("duration_from_filter_flag").value;
		if(duration_from_filter != 'from')
			url += "duration_from_filter="+encodeURIComponent(duration_from_filter)+"&";
		var duration_to_filter = document.getElementById("duration_to_filter_flag").value;
		if(duration_to_filter != 'to')
			url += "duration_to_filter="+encodeURIComponent(duration_to_filter)+"&";
		var result_filter = document.getElementById("result_filter_flag").value;
		if(result_filter != 'all')
			url += "result_filter="+encodeURIComponent(result_filter)+"&";

		var sort = document.getElementById("sort_flag").value;
		if(sort != '')
			url += "sort="+sort+"&";
		var order = document.getElementById("order_flag").value;
		if(order != '')
			url += "order="+order+"&";

		//alert("get page: "+url);
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

	function check_filter()
	{
		var pattern = /^[-+*_0-9]*$/;
		var caller_from = document.getElementById("caller_from_filter").value;
		var caller_to = document.getElementById("caller_to_filter").value;

		var result = pattern.exec(caller_from);
		if(result != null){
			return true;
		}else{
			alert("invalid Caller ID!");
			return false;
		}

		var result = pattern.exec(caller_to);
		if(result != null){
			return true;
		}else{
			alert("invalid Callee ID!");
			return false;
		}
	}
	function filter()
	{
		if(!check_filter()){
			return;
		}

		document.getElementById("caller_from_filter_flag").value = document.getElementById("caller_from_filter").value;
		document.getElementById("caller_to_filter_flag").value = document.getElementById("caller_to_filter").value;
		document.getElementById("channel_from_filter_flag").value = document.getElementById("channel_from_filter").value;
		document.getElementById("channel_to_filter_flag").value = document.getElementById("channel_to_filter").value;
		document.getElementById("starttime_from_filter_flag").value = document.getElementById("starttime_from_filter").value;
		document.getElementById("starttime_to_filter_flag").value = document.getElementById("starttime_to_filter").value;
		document.getElementById("duration_from_filter_flag").value = document.getElementById("duration_from_filter").value;
		document.getElementById("duration_to_filter_flag").value = document.getElementById("duration_to_filter").value;
		document.getElementById("result_filter_flag").value = document.getElementById("result_filter").value;

		getpage(1);
	}

	function clear_filter()
	{
		document.getElementById("current_page_flag").value = 1;

		document.getElementById("caller_from_filter").value = "";
		document.getElementById("caller_to_filter").value = "";
		document.getElementById("channel_from_filter").value = "";
		document.getElementById("channel_to_filter").value = "";
		document.getElementById("starttime_from_filter").value = "from";
		document.getElementById("starttime_to_filter").value = "to";
		document.getElementById("duration_from_filter").value = "from";
		document.getElementById("duration_to_filter").value = "to";
		document.getElementById("result_filter").value = "all";

		document.getElementById("caller_from_filter_flag").value = "";
		document.getElementById("caller_to_filter_flag").value = "";
		document.getElementById("channel_from_filter_flag").value = "";
		document.getElementById("channel_to_filter_flag").value = "";
		document.getElementById("starttime_from_filter_flag").value = "from";
		document.getElementById("starttime_to_filter_flag").value = "to";
		document.getElementById("duration_from_filter_flag").value = "from";
		document.getElementById("duration_to_filter_flag").value = "to";
		document.getElementById("result_filter_flag").value = "all";

		document.getElementById("sort_flag").value = "";
		document.getElementById("order_flag").value = "";
	}

	function keypress(e)
	{
		if(window.event){ // IE
			keynum = e.keyCode;
		}else if(e.which){ // Netscape/Firefox/Opera
			keynum = e.which;
		}
		//alert("he "+keynum);

		if(keynum == 13) {
			getpage(document.getElementById("input_page").value);
		}
	}

	</script>

	<!-- filter table -->

<?php
	if($filter_sql == '' && $counts <= 0 && $tmp != ""){
		show_msg(language('NO RECORD'));
	}else{
?>
	<table width="100%" class="tsort">
		<tr>
			<th style="width:03%" class="nosort"></th>
			<th style="width:12%" class="nosort"><?php echo language('Caller ID');?></th>
			<th style="width:12%" class="nosort"><?php echo language('Callee ID');?></th>
			<th style="width:14%" class="nosort"><?php echo language('From');?></th>
			<th style="width:14%" class="nosort"><?php echo language('To');?></th>
			<th style="width:20%" class="nosort"><?php echo language('Start Time');?></th>
			<th style="width:14%" class="nosort"><?php echo language('Duration');?></th>
			<th style="width:12%" class="nosort"><?php echo language('Result');?></th>
		</tr>
		<tr>
			<td>
			</td>
			<td>
				<input type="text" style="width:95%" id="caller_from_filter" name="caller_from_filter" value="<?php echo $caller_from_filter; ?>" oninput="this.value=this.value.replace(/[^-+*@.\w]*/g,'')">
			</td>
			<td>
				<input type="text" style="width:95%" id="caller_to_filter" name="caller_to_filter" value="<?php echo $caller_to_filter; ?>" oninput="this.value=this.value.replace(/[^-+*@.\w]*/g,'')">
			</td>
			<td>
				<input type="text" style="width:95%" id="channel_from_filter" name="channel_from_filter" value="<?php echo $channel_from_filter; ?>" oninput="this.value=this.value.replace(/[^-+*@.\s\w]*/g,'')">
			</td>
			<td>
				<input type="text" style="width:95%" id="channel_to_filter" name="channel_to_filter" value="<?php echo $channel_to_filter; ?>" oninput="this.value=this.value.replace(/[^-+*@.\s\w]*/g,'')">
			</td>
			<td>
				<input type="text" style="width:45%" id="starttime_from_filter" name="starttime_from_filter" value="<?php echo $starttime_from_filter; ?>" >
				<input type="text" style="width:45%" id="starttime_to_filter" name="starttime_to_filter" value="<?php echo $starttime_to_filter; ?>">
			</td>
			<td>
				<input type="text" style="width:40%" id="duration_from_filter" name="duration_from_filter" value="<?php echo $duration_from_filter; ?>" >
				<input type="text" style="width:40%" id="duration_to_filter" name="duration_to_filter" value="<?php echo $duration_to_filter; ?>" >
			</td>
			<td>
				<select style="width:98%" name="result_filter" id="result_filter" >
					<option value="" selected><?php echo language('All');?></option>
					<option value="ANSWERED" <?php if($result_filter == "ANSWERED")echo "selected"?>><?php echo language('ANSWERED');?></option>
					<option value="BUSY" <?php if($result_filter == "BUSY")echo "selected"?>><?php echo language('BUSY');?></option>
					<option value="FAILED" <?php if($result_filter == "FAILED")echo "selected"?>><?php echo language('FAILED');?></option>
					<option value="NO ANSWER" <?php if($result_filter == "NO ANSWER")echo "selected"?>><?php echo language('NO ANSWER');?></option>
					<option value="CANCEL" <?php if($result_filter == "CANCEL")echo "selected"?>><?php echo language('CANCEL');?></option>
					<option value="NO CARRIER" <?php if($result_filter == "NO CARRIER")echo "selected"?>><?php echo language('NO CARRIER');?></option>
					<option value="NO DIALTONE" <?php if($result_filter == "NO DIALTONE")echo "selected"?>><?php echo language('NO DIALTONE');?></option>
				</select>
			</td>
		</tr>
	</table>
	<br/>
	<input type="button" onclick='filter();' value="<?php echo language('Filter');?>" />
	<input type="button" onclick='clear_filter();' value="<?php echo language('Clean Filter');?>"/>
	<br/>
	<br/>
<?php
	} /* if($filter_sql == '' && $counts <= 0) */

	$results = $db->try_query("select * from cdr $filter_sql $sort_sql limit $start_record,$line_counts");
	if($results === false || $tmp == ""){
		show_msg(language('DATABASE BUSY'));
	}else if($results !== false && $tmp !="" && $filter_sql != '' && $counts <= 0){
		show_msg(language('NO RECORD'));
	}
?>
	<!-- CDR list -->

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
<?php
	if($counts > 0) {
		echo "<span style=\"font-weight:bold\">";echo language('Total Records');echo ": $counts";echo "</span>\n";
		$flag = 0;
		
		$aql = new aql();
		$simemusvr_conf = "/etc/asterisk/simemusvr.conf";
		$aql->set('basedir','/etc/asterisk');
		$simemu_res = $aql->query("select * from simemusvr.conf");
?>
	<table width="100%" class="tsort">
		<tr>
			<th style="width:03%" class="nosort">
				<input type="checkbox" name="selall" onclick="selectAll(this.checked,'log[]')" />
			</th>
			<th style="width:12%" class="<?php echo $callerid_class?>" onclick="sort_click(this,'callerid')"><?php echo language('Caller ID');?></th>
			<th style="width:12%" class="<?php echo $calleeid_class?>" onclick="sort_click(this,'calleeid')"><?php echo language('Callee ID');?></th>
			<th style="width:14%" class="<?php echo $from_class?>" onclick="sort_click(this,'from')"><?php echo language('From');?></th>
			<th style="width:14%" class="<?php echo $to_class?>" onclick="sort_click(this,'to')"><?php echo language('To');?></th>
			<th style="width:14%" ><?php echo language('Sim Number');?></th>
			<th style="width:20%" class="<?php echo $starttime_class?>" onclick="sort_click(this,'starttime')"><?php echo language('Start Time');?></th>
			<th style="width:14%" class="<?php echo $duration_class?>" onclick="sort_click(this,'duration')"><?php echo language('Duration');?></th>
			<th style="width:12%" class="<?php echo $result_class?>" onclick="sort_click(this,'result')"><?php echo language('Result');?></th>
			
			<?php if($simemu_res['SimEmuSvr']['simemusvr_switch'] == 'yes'){ ?>
				<th style="width:10%" class="<?php echo $bank_class?>" onclick="sort_click(this,'bank')"><?php echo language('Bank');?></th>
				<th style="width:10%" class="<?php echo $sim_class?>" onclick="sort_click(this,'slot')"><?php echo language('Slot');?></th>
			<?php } ?>
		</tr>

		<?php
		while($res = @$results->fetchArray()) {
			$flag++;
		?>

		<tr>
			<td> 
				<input type="checkbox" name="log[]" value="<?php echo $res['id']?>" />					 
			</td>
			<td> 
				<?php echo $res['callerid'];?>
			</td>
			<td> 
				<?php echo $res['calleeid'];?>
			</td>
			<td style="word-break: break-all;"> 
				<?php 
					if(get_channel_type($res['from']) == 'gsm') {
						//echo get_gsm_name_by_channel($res['from'],1,true);
						//echo change_port_form($res['from']);
						echo change_sms_routing_form_new($res['from']);
					} else {
						echo $res['from'];
					}
				?>
			</td>
			<td style="word-break: break-all;"> 
				<?php 
					if(get_channel_type($res['to']) == 'gsm') {
						//echo get_gsm_name_by_channel($res['to'],1,true);
						//echo change_port_form($res['to']);
						echo change_sms_routing_form_new($res['to']);
					} else {
						echo $res['to'];
					}
				?>
			</td>
			<td>
				<?php echo $res['simnum'];?>
			</td>
			<td> 
				<?php echo $res['starttime'];?>
			</td>
			<td> 
				<?php echo format_time($res['duration']); ?>
			</td>
			<td> 
				<?php echo $res['result'];?>
			</td>
			
			<?php if($simemu_res['SimEmuSvr']['simemusvr_switch'] == 'yes'){ ?>
			<td>
				<?php echo $res['bank'];?>
			</td>
			<td>
				<?php echo $res['slot'];?>
			</td>
			<?php } ?>
			
		</tr>
		<?php
			}
		?>
	</table>
<?php
		if($flag == 0){
			show_msg(language('DATABASE BUSY'));
		}
	}

	if($page_count >= 2) {
		echo '<br/>';
		echo '<div class="pg">';

		if( $cur_page > 1 ) {
			$page = $cur_page - 1;
			echo "<a title=\"";echo language('Previous page');echo "\" style=\"cursor:pointer;\" onclick=\"getpage($page);\"  class=\"prev\"></a>";
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
				echo "<a style=\"cursor:pointer;\" onclick=\"getpage($i);\" >$i</a>";
			} else {
				echo "<strong>$cur_page</strong>";
			}
		}

		if( $cur_page < $page_count ) {
			$page = $cur_page + 1;
			echo "<a title=\"";echo language('Next page');echo "\" style=\"cursor:pointer;\" onclick=\"getpage($page)\" class=\"nxt\" ></a>";
		} else {
			
		}

		echo "<label>";
		echo "<input type=\"text\" id=\"input_page\" name=\"page\" value=\"$cur_page\" size=\"2\" class=\"px\" title=\"";
		echo language('input page help','Please input your page number, and press [Enter] to skip to.');
		echo "\" onkeypress=\"keypress(event)\" >";
		echo "<span title=\"";echo language('total pages');echo ": $page_count\"> / $page_count</span>";
		echo "</label>";
		echo "<a title=\"";echo language('goto input page');echo "\" style=\"cursor:pointer;\" onclick=\"getpage(document.getElementById('input_page').value)\">";
		echo language('go');
		echo "</a>";
		echo '</div>';
	}else if($page_count == 1){
		echo '<br/>';
		echo '<div class="pg">';
		echo "<strong>1</strong>";
		echo '</div>';
	}

	if($page_count >= 1){
?>

	<div id="newline"></div>
	<br>
	
	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" value='<?php echo language('Delete');?>' 
		onclick="document.getElementById('send').value='Delete';return confirm('<?php echo language('Delete confirm','Are you sure to delete you selected ?');?>')"/> 
	<input type="submit" value='<?php echo language('Clean Up');?>' 
		onclick="document.getElementById('send').value='Clean Up';return confirm('<?php echo language('Clean Up confirm','Are you sure to clean up this logs?');?>')"/> 
	<input type="submit" value='<?php echo language('Export');?>' onclick="document.getElementById('send').value='Export';"/>

<?php
	}
?>
	
	<div id="newline"></div>
	<br/>
	
	<?php if($counts > 0) { ?>
	
	<div id="tab" class="gsm_outbound" style="float:left;clear:none;width:auto;margin-right:10px;cursor:pointer;">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('GSM Outbound');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	
	<div id="tab" class="gsm_inbound" style="float:left;clear:none;width:auto;margin-right:10px;cursor:pointer;">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('GSM Inbound');?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<?php 
		if($channel_from_filter_sql != ''){
			$from_filter_sql = " and `from` like '".$channel_from_filter_sql."'";//from
		}else{
			$from_filter_sql = "";
		}
		
		if($channel_to_filter_sql != ''){
			$to_filter_sql = " and `to` like '".$channel_to_filter_sql."'";//to
		}else{
			$to_filter_sql = "";
		}
		
		if($starttime_from_filter != 'from'){
			$starttime_from_sql = " and starttime >= '".$starttime_from_filter."'";//starttime from
		}else{
			$starttime_from_sql = "";
		}
		
		if($starttime_to_filter != 'to'){
			$starttime_to_sql = " and starttime <= '".$starttime_to_filter."'";//starttime to
		}else{
			$starttime_to_sql = "";
		}
	?>

	<!-- Outbound -->
	<div class="statis_show gsm_outbound_table">
	
	<table width="100%" class="tsort">
		<tr><th colspan="12"><?php echo language('GSM Outbound');?></th></tr>
		<tr>
			<th><?php echo language('Port');?></th>
			<th><?php echo language('Sim Number');?></th>
			<th><?php echo language('All Calls');?></th>
			<th><?php echo language('All Durations');?></th>
			<th><?php echo language('Answered');?></th>
			<th><?php echo language('No Answer');?></th>
			<th><?php echo language('Busy');?></th>
			<th><?php echo language('Failed');?></th>
			<th><?php echo language('CANCEL');?></th>
			<th><?php echo language('NO CARRIER');?></th>
			<th><?php echo language('NO DIALTONE');?></th>
			<th><?php echo language('Other');?></th>
		</tr>
		<?php 
		//paging
		$to_statis_cur_page = 1;
		if($_GET['to_statis_cur_page']){
			$to_statis_cur_page = $_GET['to_statis_cur_page'];
		}
		
		$count_sql = "select count(*) from cdr where `to` like '%-%' $from_filter_sql $to_filter_sql $starttime_from_sql $starttime_to_sql group by `to`,simnum order by id desc";
		$count_res = $db->try_query($count_sql);
		
		$n = 0;
		while($count_res->fetchArray()){
			$n++;
		}
		$statis_count = $n;
		
		$statis_page_count = ceil($statis_count / $line_counts);
		if($to_statis_cur_page > $statis_page_count) {
			$statis_page_count = $statis_page_count;
		}
		$statis_start_record = ($to_statis_cur_page-1)*$statis_page_count;
		
		$to_sql = "select `to`,
						simnum,
						count(id) as all_calls,
						sum(duration) as duration,
						sum(case when result = 'ANSWERED' then 1 else 0 end) as answered,
						sum(case when result = 'NO ANSWER' then 1 else 0 end) as no_answer,
						sum(case when result = 'BUSY' then 1 else 0 end) as busy,
						sum(case when result = 'FAILED' then 1 else 0 end) as failed,
						sum(case when result = 'CANCEL' then 1 else 0 end) as cancel,
						sum(case when result = 'NO CARRIER' then 1 else 0 end) as no_carrier,
						sum(case when result = 'NO DIALTONE' then 1 else 0 end) as no_dialtone,
						sum(case when result <> 'ANSWERED' and result <> 'NO ANSWER' and result <> 'BUSY' and result <> 'FAILED' and result <> 'CANCEL' and result <> 'NO CARRIER' and result <> 'NO DIALTONE' then 1 else 0 end) as other
				from cdr where `to` like '%-%' $from_filter_sql $to_filter_sql $starttime_from_sql $starttime_to_sql group by `to`,simnum order by id desc limit $statis_start_record,$line_counts";
		$to_res = $db->try_query($to_sql);
		
		$to_temp = [];
		while($to_info = $to_res->fetchArray()){
			array_push($to_temp,$to_info);
		}
		
		$res_arr = [];
		for($i=0;$i<count($to_temp);$i++){
			array_push($res_arr,$to_temp[$i]);
		}
		
		for($i=0;$i<count($res_arr);$i++){
		?>
		<tr>
			<td>
			<?php 
				if(get_channel_type($res_arr[$i]['from']) == 'gsm') {
					echo change_sms_routing_form_new($res_arr[$i]['to']);
				} else {
					echo $res_arr[$i]['to'];
				}
			?>
			</td>
			<td><?php echo $res_arr[$i]['simnum'];?></td>
			<td><?php echo $res_arr[$i]['all_calls'];?></td>
			<td><?php echo $res_arr[$i]['duration'];?></td>
			<td><?php echo $res_arr[$i]['answered'];?></td>
			<td><?php echo $res_arr[$i]['no_answer'];?></td>
			<td><?php echo $res_arr[$i]['busy'];?></td>
			<td><?php echo $res_arr[$i]['failed'];?></td>
			<td><?php echo $res_arr[$i]['cancel'];?></td>
			<td><?php echo $res_arr[$i]['no_carrier'];?></td>
			<td><?php echo $res_arr[$i]['no_dialtone'];?></td>
			<td><?php echo $res_arr[$i]['other'];?></td>
		</tr>
		<?php } ?>
	</table>
	
	<?php
	if($statis_page_count >= 2) {
		echo '<br/>';
		echo '<div class="pg">';

		if( $to_statis_cur_page > 1 ) {
			$page = $to_statis_cur_page - 1;
			echo "<a title=\"";echo language('Previous page');echo "\" style=\"cursor:pointer;\" onclick=\"getpage(0,$page);\"  class=\"prev\"></a>";
		}
			
		if($to_statis_cur_page-5 > 1) {
			$s = $to_statis_cur_page-5;
		} else {
			$s = 1;
		}

		if($s + 10 < $statis_page_count) {
			$e = $s + 10;
		} else {
			$e = $statis_page_count;
		}

		for($i = $s; $i <= $e; $i++) {
			if($i != $to_statis_cur_page) {
				echo "<a style=\"cursor:pointer;\" onclick=\"getpage(0,$i);\" >$i</a>";
			} else {
				echo "<strong>$to_statis_cur_page</strong>";
			}
		}

		if( $to_statis_cur_page < $statis_page_count ) {
			$page = $to_statis_cur_page + 1;
			echo "<a title=\"";echo language('Next page');echo "\" style=\"cursor:pointer;\" onclick=\"getpage(0,$page)\" class=\"nxt\" ></a>";
		}

		echo "<label>";
		echo "<input type=\"text\" id=\"input_page\" name=\"page\" value=\"$to_statis_cur_page\" size=\"2\" class=\"px\" title=\"";
		echo language('input page help','Please input your page number, and press [Enter] to skip to.');
		echo "\" onkeypress=\"keypress(event)\" >";
		echo "<span title=\"";echo language('total pages');echo ": $statis_page_count\"> / $statis_page_count</span>";
		echo "</label>";
		echo "<a title=\"";echo language('goto input page');echo "\" style=\"cursor:pointer;\" onclick=\"getpage(0,document.getElementById('input_page').value)\">";
		echo language('go');
		echo "</a>";
		echo '</div>';
	}else if($statis_page_count == 1){
		echo '<br/>';
		echo '<div class="pg">';
		echo "<strong>1</strong>";
		echo '</div>';
	}
	?>
	</div>

	<!-- Inbound -->
	<div class="statis_show gsm_inbound_table" style="display:none;">
	
	<table width="100%" class="tsort">
		<tr><th colspan="12"><?php echo language('GSM Inbound');?></th></tr>
		<tr>
			<th><?php echo language('Port');?></th>
			<th><?php echo language('Sim Number');?></th>
			<th><?php echo language('All Calls');?></th>
			<th><?php echo language('All Durations');?></th>
			<th><?php echo language('Answered');?></th>
			<th><?php echo language('No Answer');?></th>
			<th><?php echo language('Busy');?></th>
			<th><?php echo language('Failed');?></th>
			<th><?php echo language('CANCEL');?></th>
			<th><?php echo language('NO CARRIER');?></th>
			<th><?php echo language('NO DIALTONE');?></th>
			<th><?php echo language('Other');?></th>
		</tr>
		
		<?php
		//paging
		$from_statis_cur_page = 1;
		if($_GET['from_statis_cur_page']){
			$from_statis_cur_page = $_GET['from_statis_cur_page'];
		}
		
		$count_sql = "select count(*) from cdr where `from` like '%-%' $from_filter_sql $to_filter_sql $starttime_from_sql $starttime_to_sql group by `from`,simnum order by id desc";
		$count_res = $db->try_query($count_sql);
		
		$n = 0;
		while($count_res->fetchArray()){
			$n++;
		}
		$statis_count = $n;
		
		$statis_page_count = ceil($statis_count / $line_counts);
		if($from_statis_cur_page > $statis_page_count) {
			$statis_page_count = $statis_page_count;
		}
		$statis_start_record = ($from_statis_cur_page-1)*$statis_page_count;
		
		$from_sql = "select `from`,
						simnum,
						count(id) as all_calls,
						sum(duration) as duration,
						sum(case when result = 'ANSWERED' then 1 else 0 end) as answered,
						sum(case when result = 'NO ANSWER' then 1 else 0 end) as no_answer,
						sum(case when result = 'BUSY' then 1 else 0 end) as busy,
						sum(case when result = 'FAILED' then 1 else 0 end) as failed,
						sum(case when result = 'CANCEL' then 1 else 0 end) as cancel,
						sum(case when result = 'NO CARRIER' then 1 else 0 end) as no_carrier,
						sum(case when result = 'NO DIALTONE' then 1 else 0 end) as no_dialtone,
						sum(case when result <> 'ANSWERED' and result <> 'NO ANSWER' and result <> 'BUSY' and result <> 'FAILED' and result <> 'CANCEL' and result <> 'NO CARRIER' and result <> 'NO DIALTONE' then 1 else 0 end) as other
				from cdr where `from` like '%-%' $from_filter_sql $to_filter_sql $starttime_from_sql $starttime_to_sql group by `from`,simnum order by id desc limit $statis_start_record,$line_counts";
		$from_res = $db->try_query($from_sql);
		
		$from_temp = [];
		while($from_info = $from_res->fetchArray()){
			array_push($from_temp,$from_info);
		}
				
		$res_arr = [];
		for($i=0;$i<count($from_temp);$i++){
			array_push($res_arr,$from_temp[$i]);
		}
		
		for($i=0;$i<count($res_arr);$i++){
		?>
		
		<tr>
			<td>
			<?php 
				if(get_channel_type($res_arr[$i]['from']) == 'gsm') {
					echo change_sms_routing_form_new($res_arr[$i]['from']);
				} else {
					echo $res_arr[$i]['from'];
				}
			?>
			</td>
			<td><?php echo $res_arr[$i]['simnum'];?></td>
			<td><?php echo $res_arr[$i]['all_calls'];?></td>
			<td><?php echo $res_arr[$i]['duration'];?></td>
			<td><?php echo $res_arr[$i]['answered'];?></td>
			<td><?php echo $res_arr[$i]['no_answer'];?></td>
			<td><?php echo $res_arr[$i]['busy'];?></td>
			<td><?php echo $res_arr[$i]['failed'];?></td>
			<td><?php echo $res_arr[$i]['cancel'];?></td>
			<td><?php echo $res_arr[$i]['no_carrier'];?></td>
			<td><?php echo $res_arr[$i]['no_dialtone'];?></td>
			<td><?php echo $res_arr[$i]['other'];?></td>
		</tr>
		<?php } ?>
	</table>
	
	<?php
	if($statis_page_count >= 2) {
		echo '<br/>';
		echo '<div class="pg">';

		if( $from_statis_cur_page > 1 ) {
			$page = $from_statis_cur_page - 1;
			echo "<a title=\"";echo language('Previous page');echo "\" style=\"cursor:pointer;\" onclick=\"getpage(0,0,$page,1);\"  class=\"prev\"></a>";
		}
			
		if($from_statis_cur_page-5 > 1) {
			$s = $from_statis_cur_page-5;
		} else {
			$s = 1;
		}

		if($s + 10 < $statis_page_count) {
			$e = $s + 10;
		} else {
			$e = $statis_page_count;
		}

		for($i = $s; $i <= $e; $i++) {
			if($i != $from_statis_cur_page) {
				echo "<a style=\"cursor:pointer;\" onclick=\"getpage(0,0,$i,1);\" >$i</a>";
			} else {
				echo "<strong>$from_statis_cur_page</strong>";
			}
		}

		if( $from_statis_cur_page < $statis_page_count ) {
			$page = $from_statis_cur_page + 1;
			echo "<a title=\"";echo language('Next page');echo "\" style=\"cursor:pointer;\" onclick=\"getpage(0,0,$page,1)\" class=\"nxt\" ></a>";
		}

		echo "<label>";
		echo "<input type=\"text\" id=\"input_page\" name=\"page\" value=\"$from_statis_cur_page\" size=\"2\" class=\"px\" title=\"";
		echo language('input page help','Please input your page number, and press [Enter] to skip to.');
		echo "\" onkeypress=\"keypress(event)\" >";
		echo "<span title=\"";echo language('total pages');echo ": $statis_page_count\"> / $statis_page_count</span>";
		echo "</label>";
		echo "<a title=\"";echo language('goto input page');echo "\" style=\"cursor:pointer;\" onclick=\"getpage(0,0,document.getElementById('input_page').value,1)\">";
		echo language('go');
		echo "</a>";
		echo '</div>';
	}else if($statis_page_count == 1){
		echo '<br/>';
		echo '<div class="pg">';
		echo "<strong>1</strong>";
		echo '</div>';
	}
	?>
	</div>
	
	<?php } ?>

	<div id="newline"></div>
	<br>
	
	<script>
	$(".gsm_outbound").click(function(){
		$(".statis_show").hide();
		$(".gsm_outbound_table").show();
	});

	$(".gsm_inbound").click(function(){
		$(".statis_show").hide();
		$(".gsm_inbound_table").show();
	});
	
	</script>

	<input type="hidden" id="to_statis_cur_page" name="to_statis_cur_page" value="<?php echo $to_statis_cur_page;?>" />
	<input type="hidden" id="from_statis_cur_page" name="from_statis_cur_page" value="<?php echo $from_statis_cur_page;?>" />
	<input type="hidden" id="current_page_flag" name="current_page" value="<?php echo $cur_page;?>" />
	<input type="hidden" id="sort_flag" name="sort" value="<?php echo $sort;?>" />
	<input type="hidden" id="order_flag" name="order" value="<?php echo $order;?>" />
	<input type="hidden" id="caller_from_filter_flag" name="caller_from_filter" value="<?php echo $caller_from_filter;?>" />
	<input type="hidden" id="caller_to_filter_flag" name="caller_to_filter" value="<?php echo $caller_to_filter;?>" />
	<input type="hidden" id="channel_from_filter_flag" name="channel_from_filter" value="<?php echo $channel_from_filter;?>" />
	<input type="hidden" id="channel_to_filter_flag" name="channel_to_filter" value="<?php echo $channel_to_filter;?>" />
	<input type="hidden" id="starttime_from_filter_flag" name="starttime_from_filter" value="<?php echo $starttime_from_filter;?>" />
	<input type="hidden" id="starttime_to_filter_flag" name="starttime_to_filter" value="<?php echo $starttime_to_filter;?>" />
	<input type="hidden" id="duration_from_filter_flag" name="duration_from_filter" value="<?php echo $duration_from_filter;?>" />
	<input type="hidden" id="duration_to_filter_flag" name="duration_to_filter" value="<?php echo $duration_to_filter;?>" />
	<input type="hidden" id="result_filter_flag" name="result_filter" value="<?php echo $result_filter;?>" />
	<input type="hidden" id="sort_sql" name="sort_sql" value="<?php echo $sort_sql;?>" />

	<div id="newline"></div>

	</form>

<?php require("/www/cgi-bin/inc/boot.inc");?>