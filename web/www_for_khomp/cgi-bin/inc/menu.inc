<?php
require_once("/www/cgi-bin/inc/wrcfg.inc");
require_once("/www/cgi-bin/inc/function.inc");
global $flag_cdma;

$filename = basename($_SERVER['REQUEST_URI']);

if(($pos=strpos($filename,'?'))) {
	$filename = substr($filename,0,$pos);
}
	
?>

<div id="main">

	<div id="user_top">
		<div style="width:1280px;margin:auto;">
			<div id="khomp_lang">
				<div class="khomp_lang_img">
					<input type="hidden" class="lang_val" value="portuguese" />
					<span id="alt_portuguese" class="change_language"><img src="/images/brazil.png" /></span>
				</div>
				<div class="khomp_lang_img">
					<input type="hidden" class="lang_val" value="english" />
					<span id="alt_english" class="change_language"><img src="/images/english.png" /></span>
				</div>
			</div>
			
			<div id="user_show">
				<span>
					<img src="/images/user.png" />
				</span>
				
				<div id="user_option">
					<a href="/cgi-bin/php/system-login.php" class="user_tab"><?php session_start();echo $_SESSION['username'];?></a>
					<span class="user_tab" id="logout" style="float:none;"><?php echo language('Logout');?></span>
				</div>
			</div>
		</div>
	</div>
	
	<script>
	$("#logout").click(function(){
		$.ajax({
			url: "ajax_server.php?type=session_clean",
			type: "GET",
			success: function(data){
				window.location.href = "/cgi-bin/php/login.php";
			}
		});
	});
	</script>
	
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
	

	<div id="top">
		<a class="khomp_logo" href="<?php echo get_self();?>"><img style="width:240px;" src="/images/khomp_logo.png" /></a>
		<div id="top_menu">
			<ul>
			<?php
				$k=0;
				$nana="";
				$nanh="";
				$crumb = 0;
				foreach($nav_lists as $key=>$value){
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
						$top_selected = "class='top_selected'";
					}else{
						$top_selected = "";
					}
					
					echo "<li $top_selected data-value='$key'>";
					echo "<a id=mynav".$k." onmousedown=javascript:switch_over(" . $k.",". $nav_lists_count.") href=\"./../../cgi-bin/php/$nanh\" title='".language($key)."' style='max-width:106px;overflow:hidden;'>";
					echo language($key);
					echo "</a></li>";

					$k++;
				}
			?>
			<div class="clear"></div>
			</ul>
		</div>
		
<?php
	foreach($nav_lists as $key=>$value) {
		if(strcmp($nana,$key) == 0) {
			$show_sub_menu = "";
		}else{
			$show_sub_menu = "style='display:none;'";
		}
		echo "<div class='sub_menu' id='$key' $show_sub_menu><ul>";
		
		foreach($value as $row) {
			
			if($filename === $row[0]) {
				$sub_selected = "class='sub_selected'";
			} else {
				$sub_selected = "";
			}
			
			echo "<li $sub_selected><a href='/cgi-bin/php/$row[0]'>".language($row[1])."</a></li>";
		}

		echo "</ul>";
		// echo "<button class='apply'>Apply</button><button class='apply_none'>&nbsp;</button>";
		if(!strstr(get_self(),'system-status')){
			echo "<button class='apply'>".language("Apply")."</button>";
		}else{
			echo "<button class='refresh' onclick='window.location.reload();'>".language("Refresh")."</button>";
		}
		echo "<div class='clear'></div></div>";
	}
?>

	</div>
	
	<div id="apply_result">
		<?php echo language('Apply Success'); ?>
	</div>
	
	<div class="cancel_apply">
		<span id="cancel_apply_help" style="margin-left:20px;">
			<?php echo language('Cancel Apply help','Configuration settings can be cancelled before Apply.');?>
		</span>
		
		<span id="cancel_apply_point">
			<button type="button" style="background:#38AD6F;border:none;cursor:pointer;color:#fff;font-weight:bold;min-width:80px;height:30px;"><?php echo language('Cancel');?></button>
		</span>
	</div>