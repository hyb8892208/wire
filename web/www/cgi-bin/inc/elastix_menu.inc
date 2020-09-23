<?php

$filename = basename($_SERVER['REQUEST_URI']);

if(($pos=strpos($filename,'?'))) {
	$filename = substr($filename,0,$pos);
}

	$head_array = array(
		array("gsm-settings.php","Module Settings"),
		//array("gsm-adv-settings.php","Advanced"),
		array("gsm-callforwarding.php","Call Forwarding"),
		array("gsm-callwaiting.php","Call Waiting"),
		array("gsm-dtmf.php","DTMF"),
		array("gsm-bcch.php","BCCH"),
		array("gsm-toolkit.php","Toolkit"),
	);

$nav_lists = array(
	"system"=>array(
		array("system-status.php","Status"),
		array("system-time.php","Time"),
		array("system-login.php","Login Settings"),
		array("system-general.php","General"),
		array("system-cluster.php","Cluster"),
		array("system-tools.php","Tools"),
		array("system-info.php","Information"),
	),

	//"gsm"=>array(
	//	array("gsm-settings.php","GSM Settings"),
	//	array("gsm-adv-settings.php","Advanced"),
	//	array("gsm-callforwarding.php","Call Forwarding"),
	//	array("gsm-callwaiting.php","Call Waiting"),
/*bengaldels*/
	//	array("gsm-bts-info.php","BTS Information"),
	//	array("gsm-special.php","Special"),
/*bengaldele*/
	//	array("gsm-dtmf.php","DTMF"),
	//	array("gsm-bcch.php","BCCH"),
	//	array("gsm-toolkit.php","Toolkit"),
	//),

	"module" => $head_array,

	"voip"=>array(
		array("voip-endpoints.php","VoIP Endpoints"),
		array("sip-adv-settings.php","Advanced SIP Settings"),
		array("iax-adv-settings.php","Advanced IAX2 Settings"),
		),

	"routing"=>array(
		array("routing-rules.php","Call Routing Rules"),
		array("routing-groups.php","Groups"),
		array("routing-mnp.php","MNP Settings"),
		),

	"sms"=>array(
		array("sms-settings.php","SMS Settings"),
		array("sms-sender.php","SMS Sender"),
		array("sms-inbox.php","SMS Inbox"),
		array("sms-outbox.php","SMS Outbox"),
		array("sms-routing.php","SMS Forwarding"),
	),

	"network"=>array(
		array("network-lan.php","LAN Settings"),
		//array("network-wan.php","WAN Settings"),
		//array("network-dns.php","DNS Settings"),
		array("network-ddns.php","DDNS Settings"),
		array("network-toolkit.php","Toolkit"),
		array("network-switch.php","Security Settings"),
		array("network-rules.php","Security Rules"),
	),

	"advanced"=>array(
		array("adv-astapi.php","Asterisk API"),
		array("adv-astcli.php","Asterisk CLI"),
		array("adv-astfileeditor.php","Asterisk File Editor"),
	),

	"logs"=>array(
		array("log-settings.php","Log Settings"),
		array("log-sys.php","System"),
		array("log-ast.php","Asterisk"),
		array("log-sip.php","SIP"),
		array("log-iax.php","IAX2"),
		array("log-debugat.php","AT Commands"),
/*bengaldels*/
		array("log-special.php","Special"),
/*bengaldele*/
		array("log-cdr.php","CDR"),
		array("log-statistics.php","Statistics"),
	)
);
$nav_lists_count = count($nav_lists);
?>

<div id="bg">
	<div id="ela_header"></div>
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
		
		echo " id=mynav".$k." onmousedown=javascript:switch_over(" . $k.",". $nav_lists_count.") href=\"./../../cgi-bin/php/$nanh\">";
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
<?php
	$i=0;
	$nanc="";
	foreach($nav_lists as $key=>$value) {
		if($nana === $key){
			echo "<div style=\"DISPLAY: block\" id=qh_con".$i." ><UL>\n";
		}else{
			echo "<div style=\"DISPLAY: none\" id=qh_con".$i." ><UL>\n";
		}
		$n=0;
		//echo "<li class=\"bgl\"><img src=\"/images/bg_l.gif\" /></li>\n";
		echo "<li class=\"bgl noborder\"><img src=\"/images/bg_l.gif\" height='26' /></li>\n";
		
		foreach($value as $row) {
			if($n > 0) {
				echo "<li class=\"menu_con_line\">|</li>\n";
			}

			if($filename === $row[0]) {
				echo "<li class='ela_block'></li><li class=\"bgbg\"><a href=\"./../../cgi-bin/php/$row[0]\" class=\"con_off\">";
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

		//echo "<li class=\"bgr\"><img src=\"/images/bg_r.gif\" /></li>\n";
		echo "<li class=\"bgr noborder\"><img src=\"/images/bg_r.gif\" height='26' /></li>\n";
		
		echo "</UL></div>\n";
		$i++;
	}
?>
		</div>
	</div>
	<script type="text/javascript">
		$(document).ready(function(){
			$("<li class='ela_block'></li>").insertAfter('#qh_con0 ul li:first');
			$("<li class='ela_block'></li>").insertAfter('#qh_con1 ul li:first');
			$("<li class='ela_block'></li>").insertAfter('#qh_con2 ul li:first');
			$("<li class='ela_block'></li>").insertAfter('#qh_con3 ul li:first');
			$("<li class='ela_block'></li>").insertAfter('#qh_con4 ul li:first');
			$("<li class='ela_block'></li>").insertAfter('#qh_con5 ul li:first');
			$("<li class='ela_block'></li>").insertAfter('#qh_con6 ul li:first');
			$("<li class='ela_block'></li>").insertAfter('#qh_con7 ul li:first');
			
			$("<li class='ela_block'></li>").insertBefore('#qh_con0 ul li:last');
			$("<li class='ela_block'></li>").insertBefore('#qh_con1 ul li:last');
			$("<li class='ela_block'></li>").insertBefore('#qh_con2 ul li:last');
			$("<li class='ela_block'></li>").insertBefore('#qh_con3 ul li:last');
			$("<li class='ela_block'></li>").insertBefore('#qh_con4 ul li:last');
			$("<li class='ela_block'></li>").insertBefore('#qh_con5 ul li:last');
			$("<li class='ela_block'></li>").insertBefore('#qh_con6 ul li:last');
			$("<li class='ela_block'></li>").insertBefore('#qh_con7 ul li:last');
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
	<!--
	<div id="aeu">
		<img src="/images/icons/<?PHP echo $nana ?>.gif" />
	</div>
	-->
	
	<div id="lps">
