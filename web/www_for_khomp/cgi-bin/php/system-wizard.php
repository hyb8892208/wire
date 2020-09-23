<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/php/wizard/Functions.php");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
?>
<?php
$WizardPages = array( 
	"ChangePassword" => "ChangePasswordPage.php",
	"SelectTimezone" => "SelectTimezonePage.php",
	"NetworkSettings" => "NetworkSettingsPage.php",
	"SIPEndpoint" => "SIPEndpointPage.php",
	"Destination" => "DestinationPage.php",
	"Summary" => "SummaryPage.php",
);
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en" xml:lang="en">                                                       
	<head>                                                     
	<meta charset="utf-8">                                                                                            
	<meta name="viewport" content="width=device-width, initial-scale=1">                                              
	<title><?php echo language('Setting Wizard');?></title>                                                                     
	<link rel="icon" href="/images/logo.ico" type="image/x-icon">                                                     
	<link rel="shortcut icon" href="/images/logo.ico" type="image/x-icon">                                            
	<link rel="stylesheet"  href="/css/style.css" />                                                                  
	</head>                                                                                                           
	<body>                                                                                                            
	<script src="/js/js.js"></script>                                                                                 
	<script src="/js/jquery.js"></script> 
	<link rel="stylesheet" href="/css/wizard.css" />
	<script type="text/javascript" src="/js/jquery.js"></script>
	<script type="text/javascript" src="/js/scrollable.js"></script>
	<script type="text/javascript" src="/js/functions.js"></script>
	<script type="text/javascript" src="/js/check.js"></script>
	<script type="text/javascript">

function LanTypeChange()
{

	var lan_type = $("#lan_type").val();
    console.log(lan_type);
	if ( lan_type == "factory" ){
		//$("#lan_ipv4_setting").show();
		$("#lan_dns_setting").show();
		lan_ip_obj = $("#lan_ip_address");
		lan_ip_obj.attr('readOnly',true);
		lan_ip_obj.val("172.16.98.1");

		lan_netmask_obj = $("#lan_netmask");
		lan_netmask_obj.attr("readOnly", true);
		lan_netmask_obj.val("255.255.0.0");

		lan_gateway_obj = $("#lan_gateway");
		lan_gateway_obj.attr("readOnly", true);
		lan_gateway_obj.val("172.16.0.1");
	} else if( lan_type == "static" ){
		$("#lan_dns_setting").show();
		lan_ip_obj = $("#lan_ip_address");
		lan_ip_obj.attr("readOnly", false);
		lan_ip_obj.val("");

		lan_netmask_obj = $("#lan_netmask");
		lan_netmask_obj.attr("readOnly", false);
		lan_netmask_obj.val("");

		lan_gateway_obj = $("#lan_gateway");
		lan_gateway_obj.attr("readOnly", false);
		lan_gateway_obj.val("");
	} else if( lan_type == "dhcp" ){
		$("#lan_dns_setting").css("display", "none");
		lan_ip_obj = $("#lan_ip_address");
		lan_ip_obj.attr("readOnly", true);
		lan_ip_obj.val("");

		lan_netmask_obj = $("#lan_netmask");
		lan_netmask_obj.attr("readOnly", true);
		lan_netmask_obj.val("");

		lan_gateway_obj = $("#lan_gateway");
		lan_gateway_obj.attr("readOnly", true);
		lan_gateway_obj.val("");
	} 

}
function WanTypeChange()
{
	var wan_type = $("#wan_type").val();
	console.log(wan_type)
	if ( wan_type == "factory" ){
		$("#wan_ipv4_setting").show();
		wan_ip_obj = $("#wan_ip_address");
		wan_ip_obj.attr("readOnly", true);
		wan_ip_obj.val("172.16.98.1");

		wan_netmask_obj = $("#wan_netmask");
		wan_netmask_obj.attr("readOnly", true);
		wan_netmask_obj.val("255.255.0.0");

		wan_gateway_obj = $("#wan_gateway");
		wan_gateway_obj.attr("readOnly", true);
		wan_gateway_obj.val("172.16.0.1");
	} else if( wan_type == "static" ){
		$("#wan_ipv4_setting").show();
		wan_ip_obj = $("#wan_ip_address");
		wan_ip_obj.attr("readOnly", false);
		wan_ip_obj.val("");

		wan_netmask_obj = $("#wan_netmask");
		wan_netmask_obj.attr("readOnly", false);
		wan_netmask_obj.val("");

		wan_gateway_obj = $("#wan_gateway");
		wan_gateway_obj.attr("readOnly", false);
		wan_gateway_obj.val("");
	} else if( wan_type == "dhcp" ) {
		$("#wan_ipv4_setting").hide();
		wan_ip_obj = $("#wan_ip_address");
		wan_ip_obj.attr("readOnly", true);
		wan_ip_obj.val("");

		wan_netmask_obj = $("#wan_netmask");
		wan_netmask_obj.attr("readOnly", true);
		wan_netmask_obj.val("");

		wan_gateway_obj = $("#wan_gateway");
		wan_gateway_obj.attr("readOnly", true);
		wan_gateway_obj.val("");
	} else if( wan_type == "disable" ){

		$("#wan_ipv4_setting").hide();
		wan_ip_obj = $("#wan_ip_address");
		wan_ip_obj.attr("readOnly", true);
		wan_ip_obj.val("");

		wan_netmask_obj = $("#wan_netmask");
		wan_netmask_obj.attr("readOnly", true);
		wan_netmask_obj.val("");

		wan_gateway_obj = $("#wan_gateway");
		wan_gateway_obj.attr("readOnly", true);
		wan_gateway_obj.val("");
	}
}
</script>
<body style="background-color:  #F3F7FA">
<div id="app-content-main">
<div class="app-content-title" style="display:inline">
	<span class="content-name"><?php echo language('Setup Wizard');?></span>
	<div style="clear: both;"></div>
</div>
<div class="app-content-main-container" style="max-height: 900px; max-width: 1170px; margin: auto">
	<div class="mb12">
		<div class="ant-steps ant-steps-horizontal ant-steps-label-horizontal">
			<div class="ant-steps-item ant-steps-status-process" style="margin-right: 35px;">
				<div class="ant-steps-tail" style="padding-right: 21px;"></div>
				<div class="ant-steps-step">
					<div class="ant-steps-head">
						<div class="ant-steps-head-inner"><span class="ant-steps-icon ant-steps-icon-process">1</span></div>
					</div>
					<div class="ant-steps-main">
						<div class="ant-steps-title"><?php echo language('Change Password');?></div>
						<!-- react-text: 597 --><!-- /react-text -->
					</div>
				</div>
			</div>
			<div class="ant-steps-item ant-steps-status-wait" style="margin-right: 35px;">
				<div class="ant-steps-tail" style="padding-right: 21px;"><i></i></div>
				<div class="ant-steps-step">
					<div class="ant-steps-head">
						<div class="ant-steps-head-inner"><span class="ant-steps-icon">2</span></div>
					</div>
					<div class="ant-steps-main">
						<div class="ant-steps-title"><?php echo language('Select Time Zone');?></div>
						<!-- react-text: 607 --><!-- /react-text -->
					</div>
				</div>
			</div>
			<div class="ant-steps-item ant-steps-status-wait" style="margin-right: 35px;">
				<div class="ant-steps-tail" style="padding-right: 21px;"><i></i></div>
					<div class="ant-steps-step">
						<div class="ant-steps-head">
							<div class="ant-steps-head-inner"><span class="ant-steps-icon">3</span></div>
						</div>
					<div class="ant-steps-main">
						<div class="ant-steps-title"><?php echo language('Network Settings');?></div>
						<!-- react-text: 617 --><!-- /react-text -->
					</div>
				</div>
			</div>
			<div class="ant-steps-item ant-steps-status-wait" style="margin-right: 35px;">
				<div class="ant-steps-tail" style="padding-right: 21px;"><i></i></div>
				<div class="ant-steps-step">
					<div class="ant-steps-head">
						<div class="ant-steps-head-inner"><span class="ant-steps-icon">4</span></div>
					</div>
					<div class="ant-steps-main">
						<div class="ant-steps-title"><?php echo language('SIP Endpoints');?></div>
						<!-- react-text: 627 --><!-- /react-text -->
					</div>
				</div>
			</div>
			<div class="ant-steps-item ant-steps-status-wait" style="margin-right: 35px;">
				<div class="ant-steps-tail" style="padding-right: 0px;"><i></i></div>
				<div class="ant-steps-step">
					<div class="ant-steps-head">
						<div class="ant-steps-head-inner"><span class="ant-steps-icon">5</span></div>
					</div>
					<div class="ant-steps-main">
						<div class="ant-steps-title"><?php echo language('Destination');?></div>
						<!-- react-text: 637 --><!-- /react-text -->
					</div>
				</div>
			</div>
			<div class="ant-steps-item ant-steps-status-wait">
				<div class="ant-steps-tail" style="padding-right: 0px;"><i></i></div>
				<div class="ant-steps-step">
					<div class="ant-steps-head">
						<div class="ant-steps-head-inner"><span class="ant-steps-icon">6</span></div>
					</div>
					<div class="ant-steps-main">
						<div class="ant-steps-title"><?php echo language('Summary');?></div>
						<!-- react-text: 647 --><!-- /react-text -->
					</div>
				</div>
			</div>
		</div>
	</div>
	<div class="app-content-main-contents">
		<div class="ant-card ant-card-bordered" style="height: 500px; overflow: auto;">
			<div class="ant-card-head">
				<h3 class="ant-card-head-title"><?php echo language('Change Password');?></h3>
			</div>
			<form enctype="multipart/form-data" action="<?php echo $_SERVER['PHP_SELF'] ?>" method="post">
				<!--Display the Wizard Pages Contents -->
				<?php 
				foreach ($WizardPages as $Page => $PageFile) {
					if(isset($Page) && $Page != '' ){
						require("/www/cgi-bin/php/wizard/" . $PageFile);
					}
				}
				?>
				<input type="hidden" id="page_info" name="page_info" value="" />
			</form>
		</div>
		<!-- <form class="ant-form ant-form-horizontal display-block"> -->
		<br>
	
	</div>
<br />
	<div class="top-button">
		<button type="button" class="ant-btn prev" >
			<span><?php echo language('Previous');?></span>
		</button>
		<button type="button " class="ant-btn ant-btn-primary next">
			<span class="get-page" style="color:#fff;"><?php echo language('Next');?></span>
		</button>
		<?php if(!$only_view){ ?>
		<button type="button" class="ant-btn save" style="display:none;">
			<span><?php echo language('Save');?></span>
		</button>
		<?php } ?>
		<button type="button" class="ant-btn quit" >
			<span><?php echo language('Quit');?></span>
		</button>
	</div>
	
</div>
</div>
<div class="section-foot">	
	KVoLTE
</div>
</body>
<script type="text/javascript">
function quitSettingWizard(){
	$('.quit').click(function(){
		var quit_tips = '<?php echo language('quit setting wizard help', 'Are you sure you want to quit the setting wizard?');?>'
		if(confirm(quit_tips)){
			var server_file = "/cgi-bin/php/ajax_server.php"
			$.ajax({
				url: server_file+"?&type=system&action=set_wizard&value=off",
				async: false,
				dataType: 'text',				
				type: 'GET',
				data:{},
				error: function(){				//request failed callback function;
					var error_str = "Failed to quit Setting Wizard";
					alert(error_str);
					return false;
				},
				success: function(status){			//request success callback function;
					if(status == 'success'){
						window.location.href = 'system-status.php';
						return true;
					} else {
						return false;
					}
				},
				complete: function(status){
					
				}

			});
		}
	});
}

function modeChange()
{
	var tz_info = $('#system_timezone').val();
	if ((tz_info=='') || (tz_info==null)){
		$("#self_defined_time_zone").val(tz_info);
	}else {
		var tz_split = tz_info.split('@');
		$("#self_defined_time_zone").val(tz_split[1]);
	}
}
function checkChangePasswordPage()
{
	var new_username = $("#new_username").val();
	var new_password = $("#new_password").val();
	var confirm_password = $("#confirm_password").val();

	var rex = /^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)[\s\S]{8,32}$/;
	if( new_username == "" ){
		alert("please input username");
		return false;
	}
	if( new_password == ""){
		alert("please input password !");
		return false;
	} else {
		if(!rex.test(new_password)){
			alert('<?php echo htmlentities(language('js checking new_password', 'The password you enter must consist of lowercase letter, uppercase letter, and numbers, Length: 8-32 characters.'));?>');
		return false;
	}
	}
	if( new_password != confirm_password ){
		alert("confirm password is not right !");
		return false;
	}

	return true;
}
function checkNetworkSettingsPage()
{
	var lan_type = $("#lan_type").val();
	var wan_type = $("#wan_type").val();

	var lan_ip = $("#lan_ip_address").val();
	var lan_netmask = $("#lan_netmask").val();
	var lan_gateway = $("#lan_gateway").val();

	var wan_ip = $("#wan_ip_address").val();
	var wan_netmask = $("#wan_netmask").val();
	var wan_gateway = $("#wan_gateway").val();
	if(lan_type == 'static') {
		if(!check_ip(lan_ip)) {
			alert('<?php echo language('js check ip','Please input a valid IP address');?>');
			return false;
		} 

		if(!check_ip(lan_netmask)) {
			alert('<?php echo language('js check ip','Please input a valid IP address');?>');
			return false;
		} 

		if(!check_ip(lan_gateway)) {
			alert('<?php echo language('js check ip','Please input a valid IP address');?>');
			return false;
		} 
	}

	if(wan_type == 'static') {
		if(!check_ip(wan_ip)) {
			alert('<?php echo language('js check ip','Please input a valid IP address');?>');
			return false;
		} 

		if(!check_ip(wan_netmask)) {
			alert('<?php echo language('js check ip','Please input a valid IP address');?>');
			return false;
		} 

		if(!check_ip(wan_gateway)) {
			alert('<?php echo language('js check ip','Please input a valid IP address');?>');
			return false;
		} 
	}

	return true;
}
function checkSelectTimeZonePage()
{

}
function registrationChange(){
	var registration = $("#registration").val();
	if('server' == registration){
		$("#host").val('dynamic');
	} else {
		$("#host").val('');
	}
}
function checkSipEndpointsPage()
{
	var sip_endpoint_name = $("#sip_endpoint_name").val();
	var sip_endpoint_username = $("#sip_endpoint_name").val();
	var sip_endpoint_password = $("#sip_endpoint_password").val();
	var registration = $("#registration").val();
	var host = $("#host").val();
	if(!check_sipname(sip_endpoint_username)) {
		alert('<?php echo language('js check sipname','Allowed character must be any of [0-9a-zA-Z$*()-=_.], length: 1-32');?>');
		return false;
	}
	if(sip_endpoint_password != '' && !check_sippwd(sip_endpoint_password)) {
		alert('<?php echo htmlentities(language('js check sippwd','Allowed character must be any of [0-9a-zA-Z`~!#$%^&*()_+{}|<>-=,.], 0-32 characters.'));?>');
		return false;
	
	}
	if(registration != 'server') {
		if(!check_domain(host)) {
			alert('<?php echo language('js check domain','Invalid domain or IP address.');?>');
			return false;
		}
	}

	return true;
}
function checkDestinationPage()
{
	return true;
}
function displaySummaryNetworkPart()
{
	var sLanType = $("#lan_type").val();
	var sLanIP = $("#lan_ip_address").val();
	var sLanNetmask = $("#lan_netmask").val();
	var sLanGateway = $("#lan_gateway").val();
	var sLanDNS1 = $("#lan_dns1").val();
	var sLanDNS2 = $("#lan_dns2").val();
	var sLanDNS3 = $("#lan_dns3").val();

	var sWanType = $("#wan_type").val();
	var sWanIP = $("#wan_ip_address").val();
	var sWanNetmask = $("#wan_netmask").val();
	var sWanGateway = $("#wan_gateway").val();

	$("#s_lan_type").val(sLanType);
	$("#s_lan_ip_address").val(sLanIP);
	$("#s_lan_netmask").val(sLanNetmask);
	$("#s_lan_gateway").val(sLanGateway);
	$("#s_lan_dns1").val(sLanDNS1);
	$("#s_lan_dns2").val(sLanDNS2);
	$("#s_lan_dns3").val(sLanDNS3);

	$("#s_wan_type").val(sWanType);
	$("#s_wan_ip_address").val(sWanIP);
	$("#s_wan_netmask").val(sWanNetmask);
	$("#s_wan_gateway").val(sWanGateway);
	if(sLanType == 'dhcp'){
		$('#sdns').addClass('hidden')
	} else {
		$('#dns').removeClass('hidden')
	}
	if(sWanType == 'disable' || sWanType == 'dhcp'){
		$('#swan').addClass('hidden');
	} else {
		$('#swan').removeClass('hidden');
	}
}
function displaySummaryTimezonePart()
{
	var sSystemTimezone = $("#system_timezone").val();
	var sSelfDefinedTimezone = $("#self_defined_time_zone").val();
	var sLanguageType = $("#language_type").val();

	$("#s_system_timezone").val(sSystemTimezone);
	$("#s_self_defined_time_zone").val(sSelfDefinedTimezone);
	$("#s_language_type").val(sLanguageType);
}
function displaySummarySipendpointPart()
{
	var sSipUsername = $("#sip_endpoint_username").val();
	var sSipRegistration = $("#registration").val();
	var sSipHost = $("#host").val();

	$("#s_tab_sipendpoints td:eq(0)").html(sSipUsername);
	$("#s_tab_sipendpoints td:eq(1)").html(sSipRegistration);
	$("#s_tab_sipendpoints td:eq(2)").html(sSipHost);
}
function displaySummaryRoutingGroupPart()
{
	var sGroupName = $("#group_name").val();
	var sPolicy = $("#policy").val();
	var members = '';

	$('.port_name').each(function(i){
		let port_name = $(this).children('span').html();
		if(!port_name.match('null')){
			members += port_name + ',';
		}
	});
	$("#s_tab_groups td:eq(0)").html(sGroupName);
	$("#s_tab_groups td:eq(1)").html(sPolicy);
	$("#s_tab_groups td:eq(2)").html(members);

}
function displaySummaryRoutingRoutesPart()
{
	var sRouteFrom = $("#sip_endpoint_username").val();
	var routingName = $("#routing_name").val();
	var rules = '';
	if(routingName != ''){
		var prependArr = document.getElementsByName("prepend[]");
		var prefixArr = document.getElementsByName("prefix[]");
		var patternArr = document.getElementsByName("pattern[]");
		var calleridArr = document.getElementsByName("cid[]");

		var arrLength = prependArr.length;
		
		if(prependArr[0] != '' || prefixArr[0] != '' || patternArr[0] != '' || calleridArr[0] != ''){
			for(var i = 0; i < arrLength; i++){
				rules += '(' + prependArr[i].value + ')+' + prefixArr[i].value +'|[' + patternArr[i].value + '/' + calleridArr[i].value + ']<br>';
			}
		}
	}
	
	$("#s_tab_routings_out td:eq(0)").html("1");
	$("#s_tab_routings_out td:eq(1)").html("OUT");
	$("#s_tab_routings_out td:eq(2)").html(sRouteFrom);
	$("#s_tab_routings_out td:eq(3)").html("grp-ALL");
	$("#s_tab_routings_out td:eq(4)").html(rules);
	if($('#create_inbound_routes').attr('checked')){
		$('#s_tab_routings_in').removeClass('hidden');

		$("#s_tab_routings_in td:eq(0)").html("2");
		$("#s_tab_routings_in td:eq(1)").html("inbound");
		$("#s_tab_routings_in td:eq(2)").html("grp-ALL");
		$("#s_tab_routings_in td:eq(3)").html(sRouteFrom);
		$("#s_tab_routings_in td:eq(4)").html('');
	} else {
		$('#s_tab_routings_in').addClass('hidden');
	}
}
function addDPRow()
{
    var len = tbl_dialroute.rows.length-1;

    //Ìí¼ÓÒ»ÐÐ
    var newTr = tbl_dialroute.insertRow(len);
    var newTd = newTr.insertCell(0);
    newTd.innerHTML = '<?php dial_pattern_html('','','','','','','','');?>';
  
    //Ìí¼ÓÒ»ÁÐ
    //var newTd0 = newTr.insertCell(0);
}
function _onchange(obj)
{
    var save = -1;
    var pr=document.getElementsByName(obj.name);
    for(i=0;i<pr.length;i++){
        if(pr[i]==obj) {
            save = i;
            break;
        }
    }

    if(save != -1) {
        var pa=document.getElementsByName('pattern[]');
        if(pa[save] && pa[save].value=='match pattern') {
            pa[save].value='.';
        }
    }
}

function _onfocus(obj,str)
{
    if (obj.value == str) {
        obj.value =''
        obj.style.color = '#000000';
    }
}

function _onblur(obj,str)
{
    if (trim(obj.value) =='') {
        obj.value = str;
        obj.style.color = '#aaaaaa';
    }
}
function selectAllSpans()
{
	$("#select_all").click(function(){

        let spanClass = $(this).parent().attr('class');
		if($.trim(spanClass)=='ant-checkbox ant-checkbox-checked'){
			$('span[check_out="member"]').attr('class','ant-checkbox');
			$('span[check_out="member"]').find('input').attr('checked',false);
		}else{
			$('span[check_out="member"]').attr('class','ant-checkbox ant-checkbox-checked');
			$('span[check_out="member"]').find('input').attr('checked', 'checked');
		}	
	});
}
function createInboundRoute()
{
	$("#create_inbound_routes").click(function(){
		if($(this).attr('checked')){
			//$('.ant-routing-settings').removeClass("hidden");
			$('.forward-number').removeClass("hidden");
		} else {
		//	$('.ant-routing-settings').addClass("hidden");
			$('.forward-number').addClass('hidden');
		}
    });
}
function languageChange(){
	var language_type = document.getElementById('language_type');
	if(language_type.value == 'chinese'){
		var tz_info = 'Asia/Chongqing@CST-8';
		$("#system_timezone").val(tz_info);
		var tz_split = tz_info.split('@');
		$("#self_defined_time_zone").val(tz_split[1]);
	} else if(language_type.value == 'english'){
		var tz_info = 'America/Los_Angeles@PST8PDT,M3.2.0,M11.1.0';
		$("#system_timezone").val(tz_info);
		var tz_split = tz_info.split('@');
		$("#self_defined_time_zone").val(tz_split[1]);
	}else if(language_type.value == 'portuguese'){
		var tz_info = 'America/Sao_Paulo@BRT3BRST,M11.1.0/0,M2.5.0/0';
		$("#system_timezone").val(tz_info);
		var tz_split = tz_info.split('@');
		$("#self_defined_time_zone").val(tz_split[1]);
	}
	language_type.onchange = function(){
		if(language_type.value == 'chinese'){
			var tz_info = 'Asia/Chongqing@CST-8';
			$("#system_timezone").val(tz_info);
			var tz_split = tz_info.split('@');
			$("#self_defined_time_zone").val(tz_split[1]);
		} else if(language_type.value == 'english'){
			var tz_info = 'America/Los_Angeles@PST8PDT,M3.2.0,M11.1.0';
			$("#system_timezone").val(tz_info);
			var tz_split = tz_info.split('@');
			$("#self_defined_time_zone").val(tz_split[1]);
		}else if(language_type.value == 'portuguese'){
			var tz_info = 'America/Sao_Paulo@BRT3BRST,M11.1.0/0,M2.5.0/0';
			$("#system_timezone").val(tz_info);
			var tz_split = tz_info.split('@');
			$("#self_defined_time_zone").val(tz_split[1]);
		}
	}
}

$(function(){
	LanTypeChange();
	WanTypeChange();
	registrationChange();
	selectAllSpans();
	createInboundRoute();
	languageChange();
	$(".add-dial-pattern").click(function(){
        addDPRow();
    });
	
	$(".save").click(function(){
		//Members
		var group_members = '';
		$(".members").each(function(){
			if($(this).attr('checked') == 'checked'){
				var val = $(this).val();
				group_members += val + ',';
			}
		});
		
		//Prepend
		var prepend = '';
		$(".prepend").each(function(){
			var val = $(this).val();
			if(val == 'prepend') val = '';
			prepend += val + ',';
		});
		
		//Prefix
		var prefix = '';
		$(".prefix").each(function(){
			var val = $(this).val();
			if(val == 'prefix') val = '';
			prefix += val + ',';
		});
		
		//Pattern
		var pattern = '';
		$(".pattern").each(function(){
			var val = $(this).val();
			if(val == 'match pattern') val = '';
			pattern += val + ',';
		});
		
		//Callerid
		var callerid = '';
		$(".callerid").each(function(){
			var val = $(this).val();
			if(val == 'Callerid') val = '';
			callerid += val + ',';
		});
		
		//create_inbound_routes
		var create_inbound_routes = '';
		if($("#create_inbound_routes").attr('checked') == 'checked'){
			create_inbound_routes = 'on';
		}
		
		var all_data = {
			//ChangePassword
			'new_username' : get_id_val('new_username'),
			'new_password' : get_id_val('new_password'),
			'confirm_password' : get_id_val('confirm_password'),
			//SelectTimezone
			'system_timezone' : get_id_val('system_timezone'),
			'language_type' : get_id_val('language_type'),
			//Network
			'lan_type' : get_id_val('lan_type'),
			'lan_ip_address' : get_id_val('lan_ip_address'),
			'lan_netmask' : get_id_val('lan_netmask'),
			'lan_gateway' : get_id_val('lan_gateway'),
			'lan_dns1' : get_id_val('lan_dns1'),
			'lan_dns2' : get_id_val('lan_dns2'),
			'lan_dns3' : get_id_val('lan_dns3'),
			'wan_type' : get_id_val('wan_type'),
			'wan_ip_address' : get_id_val('wan_ip_address'),
			'wan_netmask' : get_id_val('wan_netmask'),
			'wan_gateway' : get_id_val('wan_gateway'),
			//SIP
			'sip_endpoint_username' : get_id_val('sip_endpoint_username'),
			'sip_endpoint_password' : get_id_val('sip_endpoint_password'),
			'registration' : get_id_val('registration'),
			'host' : get_id_val('host'),
			'transport' : get_id_val('transport'),
			'nat' : get_id_val('nat'),
			//Group
			'group_name' : get_id_val('group_name'),
			'policy' : get_id_val('policy'),
			'members' : group_members,
			//Routing
			'routing_name' : get_id_val('routing_name'),
			'from_channel' : get_id_val('from_channel'),
			'to_channel' : get_id_val('to_channel'),
			'prepend' : prepend,
			'prefix' : prefix,
			'pattern' : pattern,
			'callerid' : callerid,
			'create_inbound_routes' : create_inbound_routes,
			'forward_number' : get_id_val('forward_number')
		}
		
		var server_file = "/cgi-bin/php/ajax_server.php"
		$.ajax({
			url: server_file+"?&type=system&action=set_wizard&value=on",
			async: false,
			dataType: 'text',				
			type: 'POST',
			data:all_data,
			success: function(status){
				console.log(status);
			},
			error: function(){
			}
		});
		
		window.location.href="http://"+all_data['lan_ip_address'];
	}); 
	
	function get_id_val(id){
		return $("#"+id).val();
	}

    $(".ant-checkbox").click(function(){ 	
       let obj = $(this).attr("class"); 
       if('ant-checkbox' == obj){ 
          $(this).attr('class','ant-checkbox ant-checkbox-checked'); 
          $(this).find('.ant-checkbox-input').attr('checked', 'checked'); 
       }else{ 
          $(this).attr('class','ant-checkbox'); 
          $(this).find('.ant-checkbox-input').attr('checked',false); 
       } 
   });
    var current_step=$('.ant-card-body:visible').attr('data-for');
    if('first-page'==$.trim(current_step)){
    	$('.ant-card-head-title').text('<?php echo language('Change Password');?>');
        $(".prev").hide();
    }
    $('.prev').click(function(){
        let current_page = $('.ant-card-body:visible');
        let current_item = $('.ant-steps-status-process');
        let current_icon = $('.ant-steps-icon-process');

        let current_page_value = $(current_page).attr('data-for');
      
        $('.next').show();
        $(".prev").show();
        let current_page_index = $('.ant-steps-status-process .ant-steps-icon').text();
        $(current_page).hide();
        

        $(current_item).removeClass('ant-steps-status-process').addClass('ant-steps-status-wait');
        $(current_item).prev().removeClass('ant-steps-status-wait').addClass('ant-steps-status-process');

        $('.ant-steps-status-process .ant-steps-icon').removeClass('iconfont1 anticon-check1');
        $('.ant-steps-status-process .ant-steps-icon').text(parseInt(current_page_index)-1);

        $(current_page).prev().show();
      	switch($.trim(current_page_value)){
      		case 'first-page':
      			break;
      		case 'second-page':
      			$(".prev").hide();
      			$('.ant-card-head-title').text('<?php echo language('Change Password');?>');
      			break;
      		case 'third-page':
      			$('.ant-card-head-title').text('<?php echo language('Select Timezone');?>');
      			break;
      		case 'four-page':
      			$('.ant-card-head-title').text('<?php echo language('Network Settings');?>');
      			break;
      		case 'five-page':
      			$('.ant-card-head-title').text('<?php echo language('SIP Endpoints');?>');
      			break;
      		case 'six-page':
      			$('.ant-card-head-title').text('<?php echo language('Destination');?>');
      			$('.save').hide();
      			break;
      		default :
      			break;
      	}
       
    });
    $('.next').click(function(){
        let current_item = $('.ant-steps-status-process');
        let cur_page=$('.ant-card-body:visible');

        let current_page_value=$(cur_page).attr('data-for');
        switch($.trim(current_page_value)){
      		case 'first-page':
      			if($("#new_username").val() == '' && $("#new_password").val() == '' && $("#confirm_password").val() == ''){
      				var tips_info = '<?php echo language('skip change password help', 'Note: For security reasons, you should change the default password. Are you sure you want to go to the next step?');?>';
	      			if(!confirm(tips_info)){
	      				return false
	      			}
      			} else {
      				if(!checkChangePasswordPage()){
	      				return false;
	      			}
      			}
      			$('.ant-card-head-title').text('<?php echo language('Select Timezone');?>');
      			break;
      		case 'second-page':
      			$('.ant-card-head-title').text('<?php echo language('Network Settings');?>');	
      			break;
      		case 'third-page':
	      		if(!checkNetworkSettingsPage()){
      				return false;
      			}
      			$('.ant-card-head-title').text('<?php echo language('SIP Endpoints');?>');
      			break;
      		case 'four-page':
      			if(!checkSipEndpointsPage()){
      				return false;
      			}
      			$('.ant-card-head-title').text('<?php echo language('Destination');?>');
      			$('#group_name').val('ALL');
      			$('#policy').val('roundrobin');
      			//check all spans
      			$('span[check_out="member"]').attr('class','ant-checkbox ant-checkbox-checked');
				$('span[check_out="member"]').find('input').attr('checked', 'checked');
				// Creating the Outbound routing
				$('#routing_name').val('OUT');
				var SipUsername = $("#sip_endpoint_username").val();
				$(".sip_label").append('<option value="sip-'+ SipUsername +'">sip-'+ SipUsername + '</option>');
				$('#from_channel').val('sip-'+SipUsername);
				$(".group_label").append('<option value="grp-ALL">ALL</option>');
				$('#to_channel').val('grp-ALL');
      			break;
      		case 'five-page':
      			if(!checkDestinationPage()){
      				return false;
      			}
      			$('.ant-card-head-title').text('<?php echo language('Summary');?>');
      		    displaySummaryNetworkPart();
	        	displaySummaryTimezonePart();
	        	displaySummarySipendpointPart();
	        	displaySummaryRoutingRoutesPart();
	        	displaySummaryRoutingGroupPart();
	            $(".save").show();
	            $(".next").hide();
      			break;
      		case 'six-page': 
      			break;
      		default :
      			break;
      	}
        $(".prev").show();
        $(cur_page).hide();

        $('.ant-steps-status-process .ant-steps-icon').text('');
        $('.ant-steps-status-process .ant-steps-icon').addClass('iconfont1 anticon-check1');

        $(current_item).removeClass('ant-steps-status-process').addClass('ant-steps-status-wait');
        $(current_item).next().removeClass('ant-steps-status-wait').addClass('ant-steps-status-process');
    	
    	$(cur_page).next().show();
        
      	modeChange(); //used to timezone page 

    });
    quitSettingWizard();
});

</script>

<?php
/*
if(isset($_POST['post_data']) && $_POST['post_data'] == "Save"){
	saveChangePassword();
	saveSelectTimezone();
	saveSIPEndpoint();
	saveDestination();
	closeWizardSetting();
	saveNetworkSettings();
	//locating the page to system-status.php
	header("location: http://" . trim($_POST['lan_ip_address']) . '/cgi-bin/php/system-status.php');
}*/
?>
