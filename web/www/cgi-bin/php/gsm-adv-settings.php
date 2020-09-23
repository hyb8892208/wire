<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");
include_once("/www/cgi-bin/inc/define.inc");
?>

<!---// load jQuery and the jQuery iButton Plug-in //---> 
<!--<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.3.2/jquery.min.js"></script> -->
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 

<!---// load the iButton CSS stylesheet //---> 
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />


<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/float_btn.js"></script>
<script type="text/javascript">
function enable_general(obj, showId)
{
	//$("."+showId+"_advance_sw").iButton("toggle", false);
	sw = document.getElementById('start_get_own_num').checked;
	tw = document.getElementById('own_num_from_ussd').checked;
	if(sw){
		$("#own_num_ussd").slideDown();
		if(tw){
			$("#own_num_ussd_advance").slideDown();
		} else {
			$("#own_num_ussd_advance").slideUp();
		}
	} else {
			$("#own_num_ussd").slideUp();
			$("#own_num_ussd_advance").slideUp();
	}
}

function enable_advance(obj, showId)
{
	$('#'+showId).slideToggle();
}

function cell_change()
{
	var sw = document.getElementById('start_get_cells').checked;
	var ew = document.getElementById('enable_start_timeout').checked;
	var tw = document.getElementById('enable_state_timeout').checked;
	document.getElementById('maxcells').disabled = !sw;
	document.getElementById('start_timeout').disabled = !ew;
	document.getElementById('state_timeout').disabled = !tw;
}

function start_cell_change()
{
	sw = document.getElementById('start_get_own_num').checked;
	tw = document.getElementById('own_num_from_ussd').checked;
	aw = document.getElementById('auto_check_block').checked;
	if(sw){
		$("#own_num_ussd").show();
		if(tw){
			$("#own_num_ussd_advance").show();
		} else {
			$("#own_num_ussd_advance").hide();
		}
	} else {
			$("#own_num_ussd").hide();
			$("#own_num_ussd_advance").hide();
	}
	if(aw){
		$("#auto_check_block_div").show();
	} else {
		$("#auto_check_block_div").hide();
	}
}

function enable_check_block()
{
	sw = document.getElementById('auto_check_block').checked;
	if(sw){
		$("#auto_check_block_div").slideDown();
	} else {
		$("#auto_check_block_div").slideUp();
	}
}

function check()
{
	var start_get_cells = document.getElementById("start_get_cells").checked;
	var enable_start_timeout = document.getElementById("enable_start_timeout").checked;
	var enable_state_timeout = document.getElementById("enable_state_timeout").checked;
	var auto_check_block = document.getElementById("auto_check_block").checked;
	var own_num_from_ussd = document.getElementById("own_num_from_ussd").checked;
	
	var nocarrier = document.getElementById("nocarrier").checked;
	var noanswer = document.getElementById("noanswer").checked;
	var nodialtone = document.getElementById("nodialtone").checked;
	var busy = document.getElementById("busy").checked;
	var error = document.getElementById("error").checked;
	var timeout = document.getElementById("timeout").checked;

	var maxcells = document.getElementById("maxcells").value;
	var start_timeout = document.getElementById("start_timeout").value;
	var state_timeout = document.getElementById("state_timeout").value;
	var at_timeout = document.getElementById("at_timeout").value;
	var at_counts = document.getElementById("at_counts").value;
	var detect_module_counts = document.getElementById("detect_module_counts").value;
	var dialtimeout = document.getElementById("dialtimeout").value;
	var get_own_num_ussd = document.getElementById("get_own_num_ussd").value;
	var own_num_len_min = document.getElementById("own_num_len_min").value;
	var own_num_len_max = document.getElementById("own_num_len_max").value;
	var own_num_allow = document.getElementById("own_num_allow").value;
	var own_num_strip_head = document.getElementById("own_num_strip_head").value;

	document.getElementById("cmaxcells").innerHTML = '';
	document.getElementById("cstart_timeout").innerHTML = '';
	document.getElementById("cstate_timeout").innerHTML = '';
	document.getElementById("cat_timeout").innerHTML = '';
	document.getElementById("cat_counts").innerHTML = '';
	document.getElementById("cdetect_module_counts").innerHTML = '';
	document.getElementById("cdialtimeout").innerHTML = '';
	document.getElementById("cown_num_len_min").innerHTML = '';
	document.getElementById("cown_num_len_max").innerHTML = '';
	document.getElementById("cown_num_allow").innerHTML = '';
	document.getElementById("cown_num_strip_head").innerHTML = '';
	document.getElementById("cget_own_num_ussd").innerHTML = '';
	document.getElementById("cblock_type").innerHTML = '';
	document.getElementById("cblock_sum").innerHTML = '';
	
	if(start_get_cells){
		if(!check_integer(maxcells)) {
			document.getElementById("cmaxcells").innerHTML = con_str("<?php echo language('Check Integer help','Allowed character must be an integer greater or equal to zero.');?>");
			return false;
		}
	}
	if(enable_state_timeout){
		if(!check_integer(state_timeout) || state_timeout < 5 || state_timeout > 3600) {
			document.getElementById("cstate_timeout").innerHTML = con_str("<?php echo language('Enable state timeout help','Allowed character must be an integer between 5 to 3600.');?>");
			return false;
		}
	}
	if(enable_start_timeout){
		if(!check_integer(start_timeout)) {
			document.getElementById("cstart_timeout").innerHTML = con_str("<?php echo language('Check Integer help','Allowed character must be an integer greater or equal to zero.');?>");
			return false;
		}
	}
	if(!check_integer(at_timeout)) {
		document.getElementById("cat_timeout").innerHTML = con_str("<?php echo language('Check Integer help','Allowed character must be an integer greater or equal to zero.');?>");
		return false;
	}
	if(!check_integer(at_counts)) {
		document.getElementById("cat_counts").innerHTML = con_str("<?php echo language('Check Integer help','Allowed character must be an integer greater or equal to zero.');?>");
		return false;
	}
	if(!check_integer(detect_module_counts)) {
		document.getElementById("cdetect_module_counts").innerHTML = con_str("<?php echo language('Check Integer help','Allowed character must be an integer greater or equal to zero.');?>");
		return false;
	}
	if(!check_integer(dialtimeout) || dialtimeout < 1 || dialtimeout > 3600) {
		document.getElementById("cdialtimeout").innerHTML = con_str("<?php echo language('Dial Timeout help','Allowed character must be an integer between 1 and 3600.');?>");
		return false;
	}
	if(own_num_from_ussd){
	   if (!check_own_num_ussd(get_own_num_ussd)){
			document.getElementById("cget_own_num_ussd").innerHTML = con_str("<?php echo language('Own Num USSD help','Please enter the correct instruction.');?>");
			return false;
		}
	   if (!check_integer(own_num_len_min)){
			document.getElementById("cown_num_len_min").innerHTML = con_str("<?php echo language('Check Integer help','Allowed character must be an integer greater or equal to zero.');?>");
			return false;
		}
	   if (!check_integer(own_num_len_max)){
			document.getElementById("cown_num_len_max").innerHTML = con_str("<?php echo language('Check Integer max help','Allowed character must be an integer greater or equal to min.');?>");
			return false;
		}
	   if (!check_own_num_allow(own_num_allow)){
			document.getElementById("cown_num_allow").innerHTML = con_str("<?php echo language('Cown Num allow help','Please enter the string like #*+.');?>");
			return false;
		}
	   if (!check_own_num_strip_head(own_num_strip_head)){
			document.getElementById("cown_num_strip_head").innerHTML = con_str("<?php echo language('Cown Num Strip Head help','Please enter the string like +*#0-9.');?>");
			return false;
		}
	}
	if(auto_check_block){
		var block_sum = document.getElementById("block_sum").value;
		if(!nocarrier && !noanswer && !nodialtone && !busy && !error && !timeout){
			document.getElementById("cblock_type").innerHTML = con_str("<?php echo language('Block Type help','Please select at least one.');?>");
			return false;
		}
		if(block_sum < 1 || block_sum > 50) {
			document.getElementById("cblock_sum").innerHTML = con_str("<?php echo language('Block Sum help','Allowed character must be an integer between 1 to 50.');?>");
			return false;
		}
		if(!check_integer(block_sum)) {
			document.getElementById("cblock_sum").innerHTML = con_str("<?php echo language('Block Sum help','Allowed character must be an integer between 1 to 50.');?>");
			return false;
		}
	}

	if(!check_scope($("[name='hangupdelaytime']")[0])){
		return false;
	}

	return true;
}

function check_integer(ss)
{
	var type="^[0-9]*[1-9][0-9]*$";
	var re = new RegExp(type);

	if(ss == 0){
		return true;
	}

	if(ss.match(re)==null){
		return false;
	}

	return true;
}

function check_own_num_ussd(str_ussd){
	var strRegex = '^[a-zA-z0-9@&\{\}/:\?#%!=\*()\-+"\',<>.\$]+$';
	var re=new RegExp(strRegex);
	return re.test(str_ussd);
}

function check_own_num_strip_head(str_ussd){
	var strRegex = '^[0-9#\*+]+$';
	var re=new RegExp(strRegex);
	return re.test(str_ussd);
}

function check_own_num_allow(str_ussd){
	var strRegex = '^[#\*+]+$';
	var re=new RegExp(strRegex);
	return re.test(str_ussd);
}

function check_scope(obj)
{
	var check = true;
	var help = $(obj).parent("div").find("span");
	var scope = obj.value.split("-", 2);
	var min = parseInt(scope[0]);
	var max = parseInt(scope[1]);

	var strRegex = '^[0-9-]+$';
	var ret =new RegExp(strRegex);
	check = ret.test(obj.value);

	if(scope.length == 1){
		if(isNaN(min)){
			check = false;
		}
		max = min;
	}else if(scope.length == 2){
		if(isNaN(min) || isNaN(max)){
			check = false;
		}
	}else{
		check = false;
	}

	if(obj.name == 'hangupdelaytime'){
		if(max < 0 || max > 3600 || min < 0 || min > 3600 || min > max || obj.value == ''){
			check = false;
		}
	}

	if(check == false){
		if(obj.name == 'hangupdelaytime'){
			help.html(con_str("<?php echo language('Hangup Delay Time warning','Format: 0 or 1-20, min: 0, max: 3600, min less than max');?>"));
		}
	}else{
		help.html("");
	}

	return check;
}

</script>

<?php
function save_to_extra_global_conf()
{
//extra-global.conf
/* 
[channels]
usecallerid=yes
callwaiting=yes
usecallingpres=yes
callwaitingcallerid=yes
threewaycalling=yes
transfer=yes
canpark=yes
cancallforward=yes
callreturn=yes
echocancel=yes
echocancelwhenbridged=yes
group=1
callgroup=1
pickupgroup=1
relaxdtmf=yes
 
processsms=yes
maxcells=7 
start_get_cells=no
enable_state_timeout=yes
state_timeout=60
enable_start_timeout=yes
start_timeout=100 
at_timeout=60 
at_counts=3 
detect_module_counts=10 
dialtimeout=100
fast_start=yes
start_get_own_num = '';
own_num_from_ussd = '';
get_own_num_ussd = '*140*2*4#';
own_num_len_min = '5';
own_num_len_max = '20';
own_num_allow = '+*#';
own_num_strip_head = '';
get_ussd_timeout = '10';
auto_check_block    yes | no 
block_type   no carrier | no answer | no dialtone | busy | error | timeout 
block_sum   1-50
hangupdelaytype=all|ring|answer
hangupdelaytime=0
*/	
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk');
	if (!$setok) {
		echo $aql->get_error();
		return;
	}
	$sms_conf_path = '/etc/asterisk/extra-global.conf';
	$hlock = lock_file($sms_conf_path);
	if(!$aql->open_config_file($sms_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	$exist_array = $aql->query("select * from extra-global.conf");
	
	if (isset($_POST['start_get_cells'])){
			$maxcells = trim($_POST['maxcells']);
			if(isset($exist_array['channels']['start_get_cells'])) {
				$aql->assign_editkey('channels','start_get_cells','yes');
			} else {
				$aql->assign_append('channels','start_get_cells','yes');
			}
			
			if(isset($exist_array['channels']['maxcells'])) {
				$aql->assign_editkey('channels','maxcells',$maxcells);
			} else {
				$aql->assign_append('channels','maxcells',$maxcells);
			}
	}else{
		if(isset($exist_array['channels']['start_get_cells'])) {
			$aql->assign_editkey('channels','start_get_cells','no');
		} else {
			$aql->assign_append('channels','start_get_cells','no');
		}
	}

	if (isset($_POST['enable_start_timeout'])){
		if(isset($exist_array['channels']['enable_start_timeout'])) {
			$aql->assign_editkey('channels','enable_start_timeout','yes');
		} else {
			$aql->assign_append('channels','enable_start_timeout','yes');
		}
		if(isset($_POST['start_timeout'])){
			$start_timeout = trim($_POST['start_timeout']);
			if(isset($exist_array['channels']['start_timeout'])) {
				$aql->assign_editkey('channels','start_timeout',$start_timeout);
			} else {
				$aql->assign_append('channels','start_timeout',$start_timeout);
			}
		}
	}else{
		if(isset($exist_array['channels']['enable_start_timeout'])) {
			$aql->assign_editkey('channels','enable_start_timeout','no');
		} else {
			$aql->assign_append('channels','enable_start_timeout','no');
		}
	}
	if (isset($_POST['enable_state_timeout'])){
		if(isset($exist_array['channels']['enable_state_timeout'])) {
			$aql->assign_editkey('channels','enable_state_timeout','yes');
		} else {
			$aql->assign_append('channels','enable_state_timeout','yes');
		}
		if(isset($_POST['state_timeout'])){
			$state_timeout = trim($_POST['state_timeout']);
			if(isset($exist_array['channels']['state_timeout'])) {
				$aql->assign_editkey('channels','state_timeout',$state_timeout);
			} else {
				$aql->assign_append('channels','state_timeout',$state_timeout);
			}
		}
	}else{
		if(isset($exist_array['channels']['enable_state_timeout'])) {
			$aql->assign_editkey('channels','enable_state_timeout','no');
		} else {
			$aql->assign_append('channels','enable_state_timeout','no');
		}
	}
	
	if(isset($_POST['at_timeout'])){
		$at_timeout = trim($_POST['at_timeout']);
		if(isset($exist_array['channels']['at_timeout'])) {
			$aql->assign_editkey('channels','at_timeout',$at_timeout);
		} else {
			$aql->assign_append('channels','at_timeout',$at_timeout);
		}
	}
	if(isset($_POST['at_counts'])){
		$at_counts = trim($_POST['at_counts']);
		if(isset($exist_array['channels']['at_counts'])) {
			$aql->assign_editkey('channels','at_counts',$at_counts);
		} else {
			$aql->assign_append('channels','at_counts',$at_counts);
		}
	}
	if(isset($_POST['detect_module_counts'])){
		$detect_module_counts = trim($_POST['detect_module_counts']);
		if(isset($exist_array['channels']['detect_module_counts'])) {
			$aql->assign_editkey('channels','detect_module_counts',$detect_module_counts);
		} else {
			$aql->assign_append('channels','detect_module_counts',$detect_module_counts);
		}
	}
	if(isset($_POST['dialtimeout'])){
		$dialtimeout = trim($_POST['dialtimeout']);
		if(isset($exist_array['channels']['dialtimeout'])) {
			$aql->assign_editkey('channels','dialtimeout',$dialtimeout);
		} else {
			$aql->assign_append('channels','dialtimeout',$dialtimeout);
		}
	}
	
	if (isset($_POST['fast_start'])){
		if(isset($exist_array['channels']['fast_start'])) {
			$aql->assign_editkey('channels','fast_start','yes');
		} else {
			$aql->assign_append('channels','fast_start','yes');
		}
	}else{
		if(isset($exist_array['channels']['fast_start'])) {
			$aql->assign_editkey('channels','fast_start','no');
		} else {
			$aql->assign_append('channels','fast_start','no');
		}
	}
	
	if (isset($_POST['start_get_own_num'])){
		if(isset($exist_array['channels']['start_get_own_num'])) {
			$aql->assign_editkey('channels','start_get_own_num','yes');
		} else {
			$aql->assign_append('channels','start_get_own_num','yes');
		}
		
		if (isset($_POST['own_num_from_ussd'])){
				if(isset($exist_array['channels']['own_num_from_ussd'])) {
					$aql->assign_editkey('channels','own_num_from_ussd','yes');
				} else {
					$aql->assign_append('channels','own_num_from_ussd','yes');
				}
				
				if(isset($_POST['get_own_num_ussd'])){
					$get_own_num_ussd = trim($_POST['get_own_num_ussd']);
					if(isset($exist_array['channels']['get_own_num_ussd'])) {
						$aql->assign_editkey('channels','get_own_num_ussd',$get_own_num_ussd);
					} else {
						$aql->assign_append('channels','get_own_num_ussd',$get_own_num_ussd);
					}
				}
				
				if(isset($_POST['own_num_len_min'])){
					$own_num_len_min = trim($_POST['own_num_len_min']);
					if(isset($exist_array['channels']['own_num_len_min'])) {
						$aql->assign_editkey('channels','own_num_len_min',$own_num_len_min);
					} else {
						$aql->assign_append('channels','own_num_len_min',$own_num_len_min);
					}
				}
				if(isset($_POST['own_num_len_max'])){
					$own_num_len_max = trim($_POST['own_num_len_max']);
					if(isset($exist_array['channels']['own_num_len_max'])) {
						$aql->assign_editkey('channels','own_num_len_max',$own_num_len_max);
					} else {
						$aql->assign_append('channels','own_num_len_max',$own_num_len_max);
					}
				}
				if(isset($_POST['own_num_allow'])){
					$own_num_allow = trim($_POST['own_num_allow']);
					if(isset($exist_array['channels']['own_num_allow'])) {
						$aql->assign_editkey('channels','own_num_allow',$own_num_allow);
					} else {
						$aql->assign_append('channels','own_num_allow',$own_num_allow);
					}
				}
				if(isset($_POST['own_num_strip_head'])){
					$own_num_strip_head = trim($_POST['own_num_strip_head']);
					if(isset($exist_array['channels']['own_num_strip_head'])) {
						$aql->assign_editkey('channels','own_num_strip_head',$own_num_strip_head);
					} else {
						$aql->assign_append('channels','own_num_strip_head',$own_num_strip_head);
					}
				}
				
				if(isset($_POST['get_ussd_timeout'])){
					$get_ussd_timeout = trim($_POST['get_ussd_timeout']);
					if(isset($exist_array['channels']['get_ussd_timeout'])) {
						$aql->assign_editkey('channels','get_ussd_timeout',$get_ussd_timeout);
					} else {
						$aql->assign_append('channels','get_ussd_timeout',$get_ussd_timeout);
					}
				}
		}else{
				if(isset($exist_array['channels']['own_num_from_ussd'])) {
					$aql->assign_editkey('channels','own_num_from_ussd','no');
				} else {
					$aql->assign_append('channels','own_num_from_ussd','no');
				}
		}
	}else{
		if(isset($exist_array['channels']['start_get_own_num'])) {
			$aql->assign_editkey('channels','start_get_own_num','no');
		} else {
			$aql->assign_append('channels','start_get_own_num','no');
		}
	}
	
	if (isset($_POST['auto_check_block'])){
			if(isset($exist_array['channels']['auto_check_block'])) {
				$aql->assign_editkey('channels','auto_check_block','yes');
			} else {
				$aql->assign_append('channels','auto_check_block','yes');
			}
			
			$block_type = '';
			if (isset($_POST['nocarrier'])){
				$block_type .= (','.$_POST['nocarrier']);
			}
			if (isset($_POST['noanswer'])){
				$block_type .= (','.$_POST['noanswer']);
			}
			if (isset($_POST['nodialtone'])){
				$block_type .= (','.$_POST['nodialtone']);
			}
			if (isset($_POST['busy'])){
				$block_type .= (','.$_POST['busy']);
			}
			if (isset($_POST['error'])){
				$block_type .= (','.$_POST['error']);
			}
			if (isset($_POST['timeout'])){
				$block_type .= (','.$_POST['timeout']);
			}
			if($block_type){
				$block_type = substr($block_type,1);
			}
			if(isset($exist_array['channels']['block_type'])) {
				$aql->assign_editkey('channels','block_type',$block_type);
			} else {
				$aql->assign_append('channels','block_type',$block_type);
			}
			
			$block_sum = trim($_POST['block_sum']);
			if(isset($exist_array['channels']['block_sum'])) {
				$aql->assign_editkey('channels','block_sum',$block_sum);
			} else {
				$aql->assign_append('channels','block_sum',$block_sum);
			}
	}else{
		if(isset($exist_array['channels']['auto_check_block'])) {
			$aql->assign_editkey('channels','auto_check_block','no');
		} else {
			$aql->assign_append('channels','auto_check_block','no');
		}
	}
	
	if(isset($_POST['hangupdelaytype'])){
		$hangupdelaytype = trim($_POST['hangupdelaytype']);
		if(isset($exist_array['channels']['hangupdelaytype'])) {
			$aql->assign_editkey('channels','hangupdelaytype',$hangupdelaytype);
		} else {
			$aql->assign_append('channels','hangupdelaytype',$hangupdelaytype);
		}
	}
	
	if(isset($_POST['hangupdelaytime'])){
		$hangupdelaytime = trim($_POST['hangupdelaytime']);
		if(isset($exist_array['channels']['hangupdelaytime'])) {
			$aql->assign_editkey('channels','hangupdelaytime',"$hangupdelaytime");
		} else {
			$aql->assign_append('channels','hangupdelaytime',"$hangupdelaytime");
		}
	}
	
	if (!$aql->save_config_file('extra-global.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	unlock_file($hlock);
	return true;
}

function set_default_to_extra_global_conf()
{
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk');
	if (!$setok) {
		echo $aql->get_error();
		return;
	}
	$sms_conf_path = '/etc/asterisk/extra-global.conf';
	$hlock = lock_file($sms_conf_path);
	if(!$aql->open_config_file($sms_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	$exist_array = $aql->query("select * from extra-global.conf");

	if(isset($exist_array['channels']['maxcells'])) {
		$aql->assign_editkey('channels','maxcells','7');
	} else {
		$aql->assign_append('channels','maxcells','7');
	}
	
	if(isset($exist_array['channels']['enable_start_timeout'])) {
		$aql->assign_editkey('channels','enable_start_timeout','yes');
	} else {
		$aql->assign_append('channels','enable_start_timeout','yes');
	}
	if(isset($exist_array['channels']['start_timeout'])) {
		$aql->assign_editkey('channels','start_timeout','100');
	} else {
		$aql->assign_append('channels','start_timeout','100');
	}
	if(isset($exist_array['channels']['enable_state_timeout'])) {
		$aql->assign_editkey('channels','enable_state_timeout','yes');
	} else {
		$aql->assign_append('channels','enable_state_timeout','yes');
	}
	if(isset($exist_array['channels']['state_timeout'])) {
		$aql->assign_editkey('channels','state_timeout','60');
	} else {
		$aql->assign_append('channels','state_timeout','60');
	}
	
	if(isset($exist_array['channels']['start_get_cells'])) {
		$aql->assign_editkey('channels','start_get_cells','no');
	} else {
		$aql->assign_append('channels','start_get_cells','no');
	}

	if(isset($exist_array['channels']['at_timeout'])) {
		$aql->assign_editkey('channels','at_timeout','60');
	} else {
		$aql->assign_append('channels','at_timeout','60');
	}
	if(isset($exist_array['channels']['at_counts'])) {
		$aql->assign_editkey('channels','at_counts','3');
	} else {
		$aql->assign_append('channels','at_counts','3');
	}
	if(isset($exist_array['channels']['detect_module_counts'])) {
		$aql->assign_editkey('channels','detect_module_counts','10');
	} else {
		$aql->assign_append('channels','detect_module_counts','10');
	}
	if(isset($exist_array['channels']['dialtimeout'])) {
		$aql->assign_editkey('channels','dialtimeout','100');
	} else {
		$aql->assign_append('channels','dialtimeout','100');
	}

	if(isset($exist_array['channels']['fast_start'])) {
		$aql->assign_editkey('channels','fast_start','yes');
	} else {
		$aql->assign_append('channels','fast_start','yes');
	}
	
	if(isset($exist_array['channels']['start_get_own_num'])) {
		$aql->assign_editkey('channels','start_get_own_num','no');
	} else {
		$aql->assign_append('channels','start_get_own_num','no');
	}
	
	if(isset($exist_array['channels']['own_num_from_ussd'])) {
		$aql->assign_editkey('channels','own_num_from_ussd','no');
	} else {
		$aql->assign_append('channels','own_num_from_ussd','no');
	}
	
	if(isset($exist_array['channels']['auto_check_block'])) {
		$aql->assign_editkey('channels','auto_check_block','no');
	} else {
		$aql->assign_append('channels','auto_check_block','no');
	}
	
	if(isset($exist_array['channels']['get_own_num_ussd'])) {
		$aql->assign_editkey('channels','get_own_num_ussd','*140*2*4#');
	} else {
		$aql->assign_append('channels','get_own_num_ussd','*140*2*4#');
	}
	
	if(isset($exist_array['channels']['own_num_len_min'])) {
		$aql->assign_editkey('channels','own_num_len_min','5');
	} else {
		$aql->assign_append('channels','own_num_len_min','5');
	}
	
	if(isset($exist_array['channels']['own_num_len_max'])) {
		$aql->assign_editkey('channels','own_num_len_max','20');
	} else {
		$aql->assign_append('channels','own_num_len_max','20');
	}
	
	if(isset($exist_array['channels']['own_num_allow'])) {
		$aql->assign_editkey('channels','own_num_allow','+*#');
	} else {
		$aql->assign_append('channels','own_num_allow','+*#');
	}
	
	if(isset($exist_array['channels']['own_num_strip_head'])) {
		$aql->assign_editkey('channels','own_num_strip_head','');
	} else {
		$aql->assign_append('channels','own_num_strip_head','');
	}
	
	if(isset($exist_array['channels']['get_ussd_timeout'])) {
		$aql->assign_editkey('channels','get_ussd_timeout','10');
	} else {
		$aql->assign_append('channels','get_ussd_timeout','10');
	}

	if(isset($exist_array['channels']['hangupdelaytype'])) {
		$aql->assign_editkey('channels','hangupdelaytype','all');
	} else {
		$aql->assign_append('channels','hangupdelaytype','all');
	}

	if(isset($exist_array['channels']['hangupdelaytime'])) {
		$aql->assign_editkey('channels','hangupdelaytime','0');
	} else {
		$aql->assign_append('channels','hangupdelaytime','0');
	}

	if (!$aql->save_config_file('extra-global.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	unlock_file($hlock);
	return true;
}

if($_POST && isset($_POST['send'])) {
	if($_POST['send'] == 'Save') {
		if(save_to_extra_global_conf()){
			system_operation();
		}
	} elseif ($_POST['send'] == 'set_default'){
		if(set_default_to_extra_global_conf()){
			system_operation();
		}
	}
}

function system_operation(){
	global $__deal_cluster__;
	
	wait_apply("exec", "asterisk -rx \"extra restart\" > /dev/null 2>&1 &");
	
	if($__deal_cluster__){
		global $__BRD_HEAD__; 
		global $__BRD_SUM__;
		$cluster_info = get_cluster_info();
		if($cluster_info['mode'] == 'master') {
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
					set_slave_file($slaveip,"/etc/asterisk/extra-global.conf","/etc/asterisk/extra-global.conf");
					wait_apply("request_slave", $slaveip, "syscmd:asterisk -rx \"extra restart\" > /dev/null 2>&1 &");
				}
			}
		}
	}
	
}
?>

<?php
$extra_aql = new aql();
$extra_aql->set('basedir','/etc/asterisk');
$general_res = $extra_aql->query("select * from extra-global.conf");

$start_get_cells = '';
$enable_start_timeout = 'checked';
$enable_state_timeout = 'checked';
$fast_start = 'checked';
$start_get_own_num = '';
$own_num_from_ussd = '';
$get_own_num_ussd = '*140*2*4#';
$own_num_len_min = 5; // 默认5 
$own_num_len_max =20;// 默认20 
$own_num_allow='+*#';   // 默认+ * #   
$own_num_strip_head = '';  // 默认空

$get_ussd_timeout = '10';
$auto_check_block = '';
$nocarrier = '';
$noanswer = '';
$nodialtone = '';
$busy = '';
$error = '';
$timeout = '';
$block_sum = '5';

$hangupdelaytype='all';
$hangupdelaytime=0;

if(isset($general_res['channels']['start_get_cells'])) {
	if(is_true(trim($general_res['channels']['start_get_cells']))){
		$start_get_cells = 'checked';
	}else{
		$start_get_cells = '';
	}
}

if(isset($general_res['channels']['fast_start'])) {
	if(is_true(trim($general_res['channels']['fast_start']))){
		$fast_start = 'checked';
	}else{
		$fast_start = '';
	}
}

if(isset($general_res['channels']['maxcells'])) {
	$maxcells = trim($general_res['channels']['maxcells']);
} else {
	$maxcells = '7';
}
if(isset($general_res['channels']['enable_start_timeout'])) {
	if(is_true(trim($general_res['channels']['enable_start_timeout']))) {
		$enable_start_timeout = 'checked';
	}else{
		$enable_start_timeout = '';
	}
}
if(isset($general_res['channels']['start_timeout'])) {
	$start_timeout = trim($general_res['channels']['start_timeout']);
} else {
	$start_timeout = '100';
}
if(isset($general_res['channels']['enable_state_timeout'])) {
	if(is_true(trim($general_res['channels']['enable_state_timeout']))) {
		$enable_state_timeout = 'checked';
	}else{
		$enable_state_timeout = '';
	}
}
if(isset($general_res['channels']['state_timeout'])) {
	$state_timeout = trim($general_res['channels']['state_timeout']);
} else {
	$state_timeout = '60';
}
if(isset($general_res['channels']['at_timeout'])) {
	$at_timeout = trim($general_res['channels']['at_timeout']);
} else {
	$at_timeout = '60';
}
if(isset($general_res['channels']['at_counts'])) {
	$at_counts = trim($general_res['channels']['at_counts']);
} else {
	$at_counts = '3';
}
if(isset($general_res['channels']['detect_module_counts'])) {
	$detect_module_counts = trim($general_res['channels']['detect_module_counts']);
} else {
	$detect_module_counts = '10';
}
if(isset($general_res['channels']['dialtimeout'])) {
	$dialtimeout = trim($general_res['channels']['dialtimeout']);
} else {
	$dialtimeout = '100';
}

if(isset($general_res['channels']['start_get_own_num'])) {
	if(is_true(trim($general_res['channels']['start_get_own_num']))){
		$start_get_own_num = 'checked';
	}else{
		$start_get_own_num = '';
	}
}

if(isset($general_res['channels']['own_num_from_ussd'])) {
	if(is_true(trim($general_res['channels']['own_num_from_ussd']))){
		$own_num_from_ussd = 'checked';
	}else{
		$own_num_from_ussd = '';
	}
}

if(isset($general_res['channels']['get_own_num_ussd'])) {
	$get_own_num_ussd = trim($general_res['channels']['get_own_num_ussd']);
}
if(isset($general_res['channels']['own_num_len_min'])) {
	$own_num_len_min = trim($general_res['channels']['own_num_len_min']);
}
if(isset($general_res['channels']['own_num_len_max'])) {
	$own_num_len_max = trim($general_res['channels']['own_num_len_max']);
}
if(isset($general_res['channels']['own_num_allow'])) {
	$own_num_allow = trim($general_res['channels']['own_num_allow']);
}
if(isset($general_res['channels']['own_num_strip_head'])) {
	$own_num_strip_head = trim($general_res['channels']['own_num_strip_head']);
}

if(isset($general_res['channels']['get_ussd_timeout'])) {
	$get_ussd_timeout = trim($general_res['channels']['get_ussd_timeout']);
}

if(isset($general_res['channels']['auto_check_block'])) {
	if(is_true(trim($general_res['channels']['auto_check_block']))){
		$auto_check_block = 'checked';
	}else{
		$auto_check_block = '';
	}
}

if(isset($general_res['channels']['block_type'])) {
	$block_type = explode(",",trim($general_res['channels']['block_type']));
	$block_type_counts = count($block_type);
	foreach($block_type as $value){
		if(trim($value) == 'nocarrier') $nocarrier ='checked';
		if(trim($value) == 'noanswer') $noanswer ='checked';
		if(trim($value) == 'nodialtone') $nodialtone ='checked';
		if(trim($value) == 'busy') $busy ='checked';
		if(trim($value) == 'error') $error ='checked';
		if(trim($value) == 'timeout') $timeout ='checked';
	}
}

if(isset($general_res['channels']['block_sum'])) {
	$block_sum = trim($general_res['channels']['block_sum']);
}

if(isset($general_res['channels']['hangupdelaytype'])) {
	$hangupdelaytype = trim($general_res['channels']['hangupdelaytype']);
}

if(isset($general_res['channels']['hangupdelaytime'])) {
	$hangupdelaytime = trim($general_res['channels']['hangupdelaytime']);
}

?>

	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<div style="width:100%;height:30px">
		<div id="tab" >
			<li class="tb1">&nbsp;</li>
			<li class="tbg">
				<?php echo language('General');?>
			</li>
			<li class="tb2">&nbsp;</li>
		</div>
		<div id="adv_warn">
			<li class="back_warn"></li>
			<li class="back_font">
				<?php echo language('Warning: Be cautions, advanced users only!');?>
			</li>
			<li>&nbsp;</li>
		</div>
	</div>

	<div class="div_tab" id="tab_internal_sms">
		<div class="divc_tab_show">
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Start Get Cells');?>:</div>
					<span class="showhelp">
					<?php echo language('Enable help', "ON(enabled),OFF(disabled)");?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" id="start_get_cells" name="start_get_cells" <?php echo $start_get_cells ?> onchange="cell_change()"/>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Max Cells');?>:</div>
					<span class="showhelp">
						<?php echo language('Max Cells help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<select id="maxcells" onchange="" name="maxcells">
					<?php
						$maxcells_max = 7;
						for($maxcells_num = 1; $maxcells_num <= $maxcells_max ; $maxcells_num++){
					?>
					<option value="<?php echo $maxcells_num;?>" <?php if($maxcells == $maxcells_num){echo 'selected';};?>><?php echo $maxcells_num;?></option>
					<?php
						}
					?>
				</select><span id="cmaxcells"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Start Timeout Enable');?>:</div>
					<span class="showhelp">
					<?php echo language('Enable help', "ON(enabled),OFF(disabled)");?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" id="enable_start_timeout" name="enable_start_timeout" <?php echo $enable_start_timeout ?> onchange="cell_change()" />
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Start Timeout');?>:</div>
					<span class="showhelp">
						<?php echo language('Start Timeout help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="start_timeout" type="text" name="start_timeout" style="" value="<?php echo $start_timeout; ?>" />&nbsp;(<?php echo language('second'); ?>)&nbsp;<span id="cstart_timeout"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('State Timeout Enable');?>:</div>
					<span class="showhelp">
					<?php echo language('Enable help', "ON(enabled),OFF(disabled)");?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" id="enable_state_timeout" name="enable_state_timeout" <?php echo $enable_state_timeout ?> onchange="cell_change()" />
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('State Timeout');?>:</div>
					<span class="showhelp">
						<?php echo language('State Timeout help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="state_timeout" type="text" name="state_timeout" style="" value="<?php echo $state_timeout; ?>" />&nbsp;(<?php echo language('second'); ?>)&nbsp;<span id="cstate_timeout"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('AT Timeout');?>:</div>
					<span class="showhelp">
						<?php echo language('AT Timeout help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="at_timeout" type="text" name="at_timeout" style="" value="<?php echo $at_timeout; ?>" />&nbsp;(<?php echo language('second'); ?>)&nbsp;<span id="cat_timeout"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('AT Counts');?>:</div>
					<span class="showhelp">
						<?php echo language('AT Counts help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="at_counts" type="text" name="at_counts" style="" value="<?php echo $at_counts; ?>" /><span id="cat_counts"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Detect Module Counts');?>:</div>
					<span class="showhelp">
						<?php echo language('Detect Module Counts help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="detect_module_counts" type="text" name="detect_module_counts" style="" value="<?php echo $detect_module_counts; ?>" /><span id="cdetect_module_counts"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Dial Timeout');?>:</div>
					<span class="showhelp">
						<?php echo language('Dial Timeout help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="dialtimeout" type="text" name="dialtimeout" style="" value="<?php echo $dialtimeout; ?>" />&nbsp;(<?php echo language('second'); ?>)&nbsp;<span id="cdialtimeout"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Fast Start');?>:</div>
					<span class="showhelp">
					<?php echo language('Enable help', "ON(enabled),OFF(disabled)");?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" id="fast_start" name="fast_start" <?php echo $fast_start ?> />
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Start Get Own Number');?>:</div>
					<span class="showhelp">
					<?php echo language('Enable help', "ON(enabled),OFF(disabled)");?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" id="start_get_own_num" name="start_get_own_num" <?php echo $start_get_own_num ?> onchange="enable_general(this, 'own_num_ussd')"/>
			</div>
		</div>
		
		<div class="div_tab_hide" id='own_num_ussd' style="height:">
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Own Num From USSD');?>:</div>
					<span class="showhelp">
					<?php echo language('Enable help', "ON(enabled),OFF(disabled)");?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" class="own_num_ussd_advance_sw" id="own_num_from_ussd" name="own_num_from_ussd" <?php echo $own_num_from_ussd ?>  onchange="enable_advance(this, 'own_num_ussd_advance')"/>
			</div>
		</div>

		<div class="div_tab_hide" id='own_num_ussd_advance' style="height:"><!-- internal_sms_advance start -->
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Get Own Num USSD');?>:</div>
					<span class="showhelp">
						<?php echo language('Get Own Num USSD help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="get_own_num_ussd" type="text" name="get_own_num_ussd" style="" value="<?php echo $get_own_num_ussd; ?>" /><span id="cget_own_num_ussd"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Own Num Length Min');?>:</div>
					<span class="showhelp">
						<?php echo language('Own Num Length Min help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="own_num_len_min" type="text" name="own_num_len_min" style="" value="<?php echo $own_num_len_min; ?>" /><span id="cown_num_len_min"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Own Num Length Max');?>:</div>
					<span class="showhelp">
						<?php echo language('Own Num Length Max help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="own_num_len_max" type="text" name="own_num_len_max" style="" value="<?php echo $own_num_len_max; ?>" /><span id="cown_num_len_max"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Own Num Allow');?>:</div>
					<span class="showhelp">
						<?php echo language('Own Num Allow help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="own_num_allow" type="text" name="own_num_allow" style="" value="<?php echo $own_num_allow; ?>" /><span id="cown_num_allow"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Own Num Strip Head');?>:</div>
					<span class="showhelp">
						<?php echo language('Own Num Strip Head help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="own_num_strip_head" type="text" name="own_num_strip_head" style="" value="<?php echo $own_num_strip_head; ?>" /><span id="cown_num_strip_head"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Get USSD Timeout');?>:</div>
					<span class="showhelp">
						<?php echo language('Get USSD Timeout help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<select id="get_ussd_timeout" onchange="" name="get_ussd_timeout">
					<?php
						$time_max = 20;
						for($time_num = 5; $time_num <= $time_max ; $time_num++){
					?>
				    <option value="<?php echo $time_num;?>" <?php if($get_ussd_timeout == $time_num){echo 'selected';};?>><?php echo $time_num;?></option>
				    <?php
				    	}
				    ?>
				</select>&nbsp;(<?php echo language('second'); ?>)&nbsp;<span id="cmaxcells"></span>
			</div>
		</div>
		
		<div class="divc_tab_show">
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Auto Check Block');?>:</div>
					<span class="showhelp">
					<?php echo language('Enable help', "ON(enabled),OFF(disabled)");?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" id="auto_check_block" name="auto_check_block" <?php echo $auto_check_block ?> onchange="enable_check_block()"/>
			</div>
		</div>

		<div class="div_tab_hide" id='auto_check_block_div' style="height:">
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('Block Type');?>:</div>
					<span class="showhelp">
						<?php echo language('Block Type help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input type="checkbox" name="nocarrier" value="nocarrier" id="nocarrier" <?php echo $nocarrier ?>/>NO CARRIER&nbsp;&nbsp;&nbsp;&nbsp;
				<input type="checkbox" name="noanswer" value="noanswer" id="noanswer" <?php echo $noanswer ?>/>NO ANSWER&nbsp;&nbsp;&nbsp;&nbsp;
				<input type="checkbox" name="nodialtone" value="nodialtone" id="nodialtone" <?php echo $nodialtone ?>/>NO DIALTONE&nbsp;&nbsp;&nbsp;&nbsp;
				<input type="checkbox" name="busy" value="busy" id="busy" <?php echo $busy ?>/>BUSY&nbsp;&nbsp;&nbsp;&nbsp;
				<input type="checkbox" name="error" value="error" id="error" <?php echo $error ?>/>ERROR&nbsp;&nbsp;&nbsp;&nbsp;
				<input type="checkbox" name="timeout" value="timeout" id="timeout" <?php echo $timeout ?>/>TIMEOUT<span id="cblock_type"></span>
			</div>
			<div class="div_tab_th">
				<div class="helptooltips">
					<div class="div_tab_text"><?php echo language('The Total NUM Of Blocks');?>:</div>
					<span class="showhelp">
						<?php echo language('The Total NUM Of Blocks help');?>
					</span>
				</div>
			</div>
			<div class="div_tab_td">
				<input id="block_sum" type="text" name="block_sum" style="" value="<?php echo $block_sum; ?>" /><span id="cblock_sum"></span>
			</div>
		</div>
		<div class="div_tab_th">
			<div class="helptooltips">
				<div class="div_tab_text"><?php echo language('Hangup Delay Type');?>:</div>
				<span class="showhelp">
				<?php echo language('Hangup Delay Type help', 'Hangup Delay Type');?>
				</span>
			</div>
		</div>
		<div class="div_tab_td">
			<select name="hangupdelaytype">
			    <option value="ring" <?php if($hangupdelaytype == 'ring'){echo 'selected';};?>><?php echo language('ring');?></option>
			    <option value="answer" <?php if($hangupdelaytype == 'answer'){echo 'selected';};?>><?php echo language('answer');?></option>
			    <option value="all" <?php if($hangupdelaytype == 'all'){echo 'selected';};?>><?php echo language('all');?></option>
			</select>
			<span></span>
		</div>
		<div class="div_tab_th">
			<div class="helptooltips">
				<div class="div_tab_text"><?php echo language('Hangup Delay Time');?>:</div>
				<span class="showhelp">
				<?php echo language('Hangup Delay Time help', 'Hangup Delay Time');?>
				</span>
			</div>
		</div>
		<div class="div_tab_td">
			<input type="text" name="hangupdelaytime" value="<?php echo $hangupdelaytime;?>" 
				oninput="check_scope(this);" onkeyup="check_scope(this);"/>
			<span></span>
		</div>
	</div>

	<br>
	
	<input type="hidden" name="send" id="send" value="" />
	<div id="float_btn" class="float_btn">
		<div id="float_btn_tr" class="float_btn_tr">
				<input type="submit"    value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>
				<input type="submit"   value="<?php echo language('Set Default');?>" onclick="document.getElementById('send').value='set_default';return set_default();"/>
		</div>
	</div>
	<table id="float_btn2" style="border:none;" class="float_btn2">
		<tr id="float_btn_tr2" class="float_btn_tr2">
			<td width="25px">
				<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();" />
			</td>
			<td style="padding-left: 10px;">
				<input type="submit" id="float_button_2" class="float_short_button" value="<?php echo language('Set Default');?>" onclick="document.getElementById('send').value='set_default';return set_default();"/>
			</td>
		</tr>
	</table>
	</form>

<script type="text/javascript">

function onload_func()
{
	cell_change();
	start_cell_change();
}

$(document).ready(function (){
	$("#enable_start_timeout").iButton();
	$("#enable_state_timeout").iButton();
	$("#start_get_cells").iButton();
	$("#fast_start").iButton();
	$("#start_get_own_num").iButton();
	$("#own_num_from_ussd").iButton();
	$("#auto_check_block").iButton();
	onload_func();
});

function set_default(){
	if(confirm("Are you sure to restore the default settings?")) {
		return true;
	}else{
		return false;
	}
}
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>
<div id="float_btn1" class="sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()">
</div>