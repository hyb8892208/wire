<?php
require_once("/www/cgi-bin/inc/function.inc");
require_once("/www/cgi-bin/inc/wrcfg.inc");
global $flag_cdma;
$head_array = get_module_arr();

$filename = basename($_SERVER['REQUEST_URI']);

if(($pos=strpos($filename,'?'))) {
	$filename = substr($filename,0,$pos);
}	
global $enable_ast_logsettings;
$enable_ast_logsettings = get_oem_info_sepcs('enable_ast_logsettings');
if ($enable_ast_logsettings == 'on') {
	$log_array = array(
		array("log-settings.php","Log Settings"),
		array("log-sys.php","System"),
		array("log-ast.php","Asterisk"),
		array("log-sip.php","SIP"),
		array("log-iax.php","IAX2"),
		array("log-bsp.php","BSP"),
		array("log-rri.php","RRI"),
		array("log-debugat.php","AT Commands"),
		array("log-cdr.php","CDR"),
		array("log-statistics.php","Statistics"),
//		array("log-simemusvr.php","SimEmuSvr"),
		);
} else {
		$log_array = array(
		array("log-settings.php","Log Settings"),
		array("log-sys.php","System"),
//		array("log-ast.php","Asterisk"),
		array("log-sip.php","SIP"),
		array("log-iax.php","IAX2"),
//		array("log-bsp.php","BSP"),
//		array("log-rri.php","RRI"),
//		array("log-debugat.php","AT Commands"),
		array("log-cdr.php","CDR"),
		array("log-statistics.php","Statistics"),
//		array("log-simemusvr.php","SimEmuSvr"),
	);
}

$enable_emu_logsettings = get_oem_info_sepcs('enable_emu_logsettings');
if($enable_emu_logsettings == 'on'){
	array_push($log_array, array("log-simemusvr.php","SimEmuSvr"));
}

$nav_lists = array(
	"system"=>array(
		array("system-status.php","Status"),
		array("system-time.php","Time"),
		array("system-login.php","Login Settings"),
		array("system-general.php","General"),
		array("system-tools.php","Tools"),
		array("system-info.php","Information"),
		array("system-wizard.php", "Setting Wizard"),
	),
	
	"module"=>$head_array,

	"strategy"=>array(
		array("strategy-switch.php","Switch"),
		array("strategy-limit.php","Limit"),
		array("strategy-lock.php","Lock"),
		array("strategy-sms.php","SMS Limit"),
		array("strategy-callstats.php","Call Stats"),
		array("strategy-smsstats.php","SMS Stats"),
		array("strategy-pin.php","Pin Code"),
	),
	
	"voip"=>array(
		array("voip-endpoints.php","VoIP Endpoints"),
		array("sip-bindpoints.php", "Batch SIP Endpoints"),
		array("sip-adv-settings.php","Advanced SIP Settings"),
		array("iax-adv-settings.php","Advanced IAX2 Settings"),
		array("sip-security.php","Sip Account Security"),
		),

	"routing"=>array(
		array("routing-rules.php","Call Routing Rules"),
		array("routing-groups.php","Groups"),
		array("sip-sim-binding.php", "Batch Creating Rules"),
		array("routing-mnp.php","MNP Settings"),
		array("routing-blacklist.php","Routing Blacklist"),
		array("routing-advanced.php", "Advanced"),
		array("routing-auto.php", "Auto"),
		),

	"sms"=>array(
		array("sms-settings.php","SMS Settings"),
		array("sms-sender.php","SMS Sender"),
		array("sms-inbox.php","SMS Inbox"),
		array("sms-outbox.php","SMS Outbox"),
		array("sms-routing.php","SMS Forwarding"),		
		array("sms-ussd.php","USSD"),
		array("sms-mms.php","MMS Settings"),
	),

	"network"=>array(
		array("network-lan.php","LAN Settings"),
		array("network-wan.php","WAN Settings"),
		array("network-vpn.php","VPN Settings"),
		array("network-ddns.php","DDNS Settings"),
		array("network-toolkit.php","Toolkit"),
		array("network-switch.php","Security Settings"),
		array("network-rules.php","Security Rules"),
		array("network-sipcapture.php","SIP Capture"),
	),

	"advanced"=> array(
		array("adv-astapi.php","Asterisk API"),
		array("adv-astcli.php","Asterisk CLI"),
		array("adv-astfileeditor.php","Asterisk File Editor"),
		array("adv-internet.php","Internet"),
		array("adv-cloud.php", "Cloud"),
		array("adv-balance.php", "Balance"),
		array("adv-phonenumber.php", "PhoneNumber"),
		array("adv-arp.php", "ARP"),
	),

	"logs"=>$log_array
);

if($__system_type__ != 'openvox'){
	unset($nav_lists['advanced'][4]);//delete Cloud
}

$internet_flag = 0;
for($i=0;$i<=$__GSM_SUM__;$i++){
	$port_name = get_gsm_name_by_channel($i);
	if(strstr($port_name, 'lte')){
		$internet_flag = 1;
		break;
	}
}
if($internet_flag == 0){
	unset($nav_lists['advanced'][3]);//delete internet page
}

//delete internet page
$sys_type = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys sys_type");
$chan_count = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys total_chan_count");
$product_type = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys product_type");
$hw_ver = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys hw_ver");
$slot_num = get_slotnum();
if($product_type == '5' && $hw_ver == '1.0'){
	unset($nav_lists['network'][1]);
}else if($sys_type == 3){
	unset($nav_lists['network'][1]);
}else if($sys_type == 5 && $slot_num != 0){
	 unset($nav_lists['network'][1]);
}

//SWG-M201 delete SMS Forwarding
if($sys_type == 5 && $chan_count == 1){
	unset($nav_lists['sms'][4]);
}

$nav_lists_count = count($nav_lists);

function get_module_arr(){
	global $flag_cdma;
	$modules_all_type = array();
	$modules = get_module_type();
	$modules_all_type = $modules[1];
	$flag_cdma = false; 
	$modules_type_str = '';
	foreach($modules_all_type as $value){
		$modules_type_str .= $value;
	}
	if(!strstr($modules_type_str, 'lte') && !strstr($modules_type_str, 'gsm') && !strstr($modules_type_str, 'umts')){
		$flag_cdma = true;
	}
	$product_type = get_product_type();
	$sys_type = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys sys_type");
	$bcch_arr = array(
		array("gsm-settings.php","Module Settings"),
		//array("gsm-adv-settings.php","Advanced"),
		array("gsm-callforwarding.php","Call Forwarding"),
		array("gsm-callwaiting.php","Call Waiting"),
		array("simbank-settings.php","Simbank"),
		array("gsm-dtmf.php","DTMF"),
		array("gsm-bcch.php","BCCH"),
		array("gsm-toolkit.php","Toolkit"),
		array("module-update.php","Module Update"),
		array("gsm-calllimit.php","Call And SMS Limit"),
	);
	if(!strstr($modules_type_str, 'gsm')){
		 array_splice($bcch_arr, 5, 1);
	}
	if($sys_type != 1 || $product_type < 4){
		 array_splice($bcch_arr, 3, 1);
	}
	
	if($flag_cdma){
		$head_array = array(
			array("gsm-settings.php","Module Settings"),
			//array("gsm-adv-settings.php","Advanced"),
			//array("gsm-callforwarding.php","Call Forwarding"),
			//array("gsm-callwaiting.php","Call Waiting"),
			array("simbank-settings.php","Simbank"),
			array("gsm-dtmf.php","DTMF"),
			//array("gsm-bcch.php","BCCH"),
			array("gsm-toolkit.php","Toolkit"),
			array("module-update.php","Module Update"),
			array("gsm-calllimit.php","Call And SMS Limit"),
		);
		if($sys_type != 1 || $product_type < 4){
			 array_splice($head_array, 1, 1);
		}
		
		return $head_array;
	}else {
		return $bcch_arr;
	}
}

function get_data_info(){
	$aql = new aql();
	$setok = $aql->set('basedir','/www/data/info');
	if (!$setok) {
		exit(255);
	}

	$hlock=lock_file("/www/data/info/gw_info.conf");
	$general_conf = $aql->query("select * from gw_info.conf where section='general'");
	unlock_file($hlock);
	
	$arr = [];
	$arr['switch'] = $general_conf['general']['switch'];
	$arr['copyright'] = $general_conf['general']['copyright'];
	$arr['header_image'] = $general_conf['general']['header_image'];
	$arr['footer_image'] = $general_conf['general']['footer_image'];
	
	return $arr;
}
?>

<!-- 1 channel corresponds to 4 SIM slots -->
<?php 
	if($sys_type == 2){
		$calllimit_ind = count($nav_lists['module']);
		unset($nav_lists['module'][$calllimit_ind-1]);
?>
	<style>
	#menu ul{
		padding-left:300px;
	}
	.menu_line{
		width:30px;
	}
	#nav li a{
		padding-left:0px;
	}
	</style>
<?php 
	}else{
		unset($nav_lists['strategy']);
	}
?>

<div id="bg">
	
	<?php 
	$lang_temp = substr($_SERVER['HTTP_ACCEPT_LANGUAGE'], 0, 5);
	if($lang_temp == 'zh-CN'){
		$lang = 'chinese';
		$lang_content = '系统检测到您默认语言为中文，需要切换到中文吗？';
		$lang_confirm = '确定';
		$lang_cancel = '取消';
		$no_longer_reminding = "不再提醒";
	}else{
		$lang = 'english';
		$lang_content = 'The system detects that your default language is English. Do you need to switch to English?';
		$lang_confirm = 'Confirm';
		$lang_cancel = 'Cancel';
		$no_longer_reminding = "No Longer Reminding";
	}
	
	$conf_file = "/etc/asterisk/gw/web_language.conf";
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw/');
	if(!$aql->open_config_file($conf_file)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	
	$lang_res = $aql->query("select * from web_language.conf");
	if(isset($lang_res['general']['flag']) && $lang_res['general']['flag'] == 1){
		$sel_flag = 1;
	}else{
		$sel_flag = 0;
	}
	?>
	
	<div id="switch_language">
		<div id="lang_global">
			<img src="/images/lang_switch.png" />
		</div>
		
		<div id="country_icon">
			<div id="change_chinese" class="change_lang_cont">
				<img src="/images/chinese.png" />
				<input type="hidden" class="lang_val" value="chinese" />
				<span id="alt_chinese" class="change_language">中文</span>
			</div>
			<div id="change_english" class="change_lang_cont">
				<img src="/images/english.png" />
				<input type="hidden" class="lang_val" value="english" />
				<span id="alt_english" class="change_language">English</span>
			</div>
		</div>
	</div>
	
	<div id="lang_tip_float">
		<div id="lang_tip_sanjiao">
			<img src="/images/triangle.png" />
		</div>
		<div id="lang_tip_content">
			<span><?php echo $lang_content;?></span>
			<div id="lang_tip_confrim">
				<button id="lang_confrim_btn" style="margin-right:20px;"><?php echo $lang_confirm;?></button>
				<button id="lang_cancel_btn"><?php echo $lang_cancel;?></button>
			</div>
			<span id="lang_notip"><?php echo $no_longer_reminding;?></span>
			<div style="clear:both;"></div>
		</div>
	</div>
	
	<?php $data_info_arr = get_data_info(); ?>
	
	<script>
	$(function(){
		var cus_lang = '<?php echo $lang;?>';
		var sel_lang = '<?php echo $lang_res['general']['language'];?>';
		var sel_flag = '<?php echo $sel_flag;?>';
		
		if(sel_flag == '0'){
			if(sel_lang != cus_lang){
				$("#lang_tip_float").show();
			}
		}
	});
	
	<?php if($data_info_arr['switch'] == 'on'){?>
		var header_image = '<?php echo $data_info_arr['header_image'];?>';
		
		var rand = Math.random();
		if(header_image != ''){
			$("#bg").css("background","url(/data/info/images/header_image.png?v="+rand+") no-repeat");
		}
	<?php } ?>
	
	var flag = 1;
	$(document).click(function(){
		if(flag == 1){
			if($("#country_icon").css("display") != 'none'){
				$("#country_icon").hide();
			}
		}
		flag = 1;
	});
	
	$("#lang_global").click(function(){
		flag = 0;
		if($("#country_icon").css("display") == 'none'){
			$("#country_icon").show();
		}else{
			$("#country_icon").hide();
		}
	});
	
	$("#lang_notip").click(function(){
		cancel_sel_lang();
	});
	
	$("#lang_confrim_btn").click(function(){
		var cus_lang = '<?php echo $lang;?>';
		change_lang(cus_lang);
	});
	
	$("#lang_cancel_btn").click(function(){
		$("#lang_tip_float").hide();
	});
	
	$(".change_language").click(function(){
		var cus_lang = $(this).siblings('.lang_val').val();
		change_lang(cus_lang);
	});
	
	function change_lang(lang){
		$.ajax({
			type:"GET",
			url:"ajax_server.php?type=change_language&language_type="+lang,
			success:function(data){
				window.location.reload();
			},
			error:function(){
				alert("Switch Language failed!");
			}
		});
	}
	
	function cancel_sel_lang(){
		$.ajax({
			type:"GET",
			url:"ajax_server.php?type=cancel_language",
			success:function(data){
				$("#lang_tip_float").hide();
			},
			error:function(){
				$("#lang_tip_float").hide();
			}
		});
	}
	</script>
	

	<div id="menu">
		<UL id="nav">
<?php
	$k=0;
	$nana="";
	$nanh="";
	$crumb = 0;
	foreach($nav_lists as $key=>$value){
		echo "<li>";
		$n=0;
		foreach($value as $row) {
			if(strcmp($filename,$row[0]) == 0) {
				$nana = $key;
			}
			if($n==0) {
				$nanh=$row[0];
			}

			$n++;
		}

		if(strcmp($nana,$key) == 0) {
			$crumb = $k ;
			echo "<a class=\"nav_on\"";
		}else{
			echo "<a class=\"nav_off\"";
		}
		
		echo " id=mynav".$k." onmousedown=javascript:switch_over(" . $k.",". $nav_lists_count.") href=\"./../../cgi-bin/php/$nanh\" title='".language($key)."' style='max-width:106px;overflow:hidden;'>";
		echo language($key);
		echo "</a></li>\n";

		if($k < count($nav_lists)-1) {
			echo "<li class=\"menu_line\">|</li>\n";
		}
		$k++;
	}
?>
		</UL>

		<div id=menu_con>
			<span id="logo_font">
			<?php 
			$aql = new aql();
			$setok = $aql->set('basedir','/www/data/info');
			if (!$setok) {
				exit(255);
			}

			$hlock=lock_file("/www/data/info/gw_info.conf");
			$general_conf = $aql->query("select * from gw_info.conf where section='general'");
			unlock_file($hlock);
			
			$switch = $general_conf['general']['switch'];
			if($switch == 'on'){
				echo $general_conf['general']['product_name'];
			}else{
				echo get_model_name();
			}
			?>
			</span>
<?php
	$i=0;
	$nanc="";
	$network_style = "";
	foreach($nav_lists as $key=>$value) {
		if($key == 'network' || $key == 'module' || $key == 'advanced') $network_style = 'style=padding-left:230px;';
		if($nana === $key){
			echo "<div style=\"DISPLAY: block\" id=qh_con".$i." ><UL $network_style>\n";
		}else{
			echo "<div style=\"DISPLAY: none\" id=qh_con".$i." ><UL $network_style>\n";
		}
		$n=0;
		echo "<li class=\"bgl\"><img src=\"/images/bg_l.gif\" /></li>\n";
		foreach($value as $row) {
			if($n > 0) {
				echo "<li class=\"menu_con_line\">|</li>\n";
			}
			if($filename === $row[0]) {
				echo "<li class=\"bgbg\"><a href=\"./../../cgi-bin/php/$row[0]\" class=\"con_off\">";
				echo language($row[1]);
				echo "</a></li>\n";
				$nanc=$row[1];
			} else {
				echo "<li class=\"bgbg\"><a href=\"./../../cgi-bin/php/$row[0]\" class=\"con_on\">";
				echo language($row[1]);
				echo "</a></li>\n";
			}
			$n++;
		}

		echo "<li class=\"bgr\"><img src=\"/images/bg_r.gif\" /></li>\n";
		echo "</UL></div>\n";
		$network_style = "";
		$i++;
	}
?>
		</div>
	</div>
	<script type="text/javascript">
		$(document).ready(function(){
			var menu_confirm = 1;
	<?php for($key = 0 ; $key <= $k ;$key++){ ?>
			$("#mynav"+<?php echo $key; ?>).mouseover(function(){
				$(".nav_li_hover").removeClass("nav_li_hover");
				$("#mynav"+<?php echo $key; ?>).addClass("nav_li_hover");
				show_nav(<?php echo $key; ?>);
			});
	<?php } ?>
			function show_nav(cur_num){
				var nav_num = <?php echo $k; ?>;
				for(var num = 0 ; num <= nav_num ; num++){
					if(cur_num == num){
						$("#qh_con"+num).css("display","block");
					} else {
						$("#qh_con"+num).css("display","none");
					}
				}
			}
			$("#menu").mouseover(function(){
				menu_confirm = 1;
			});
			$("#menu").mouseleave(function(){
				menu_confirm = 0;
				setTimeout(function(){
					if(menu_confirm == 0){
						show_nav(<?php echo $crumb; ?>);
						$(".nav_li_hover").removeClass("nav_li_hover");
					}
				},1500);
			});
		});
	</script>
	<div id="aeu">
		<img src="/images/icons/<?PHP echo $nana ?>.gif" />
	</div>

	<div id="lps">
