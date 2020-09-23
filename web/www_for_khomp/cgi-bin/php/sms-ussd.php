<?php require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");
include_once("/www/cgi-bin/inc/define.inc");
?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>

<?php
if($_POST && isset($_POST['send']) && $_POST['send'] == 'Save'){
	if($only_view){
		return false;
	}
	
	save_to_ussd_conf();
	
	wait_apply("exec", "/my_tools/cluster_mode > /dev/null 2>&1");
	wait_apply("exec", "/etc/init.d/lighttpd restart > /dev/null 2>&1 &");//Load conf to webservice to handle HTTP message sending.
	wait_apply("exec", "/etc/init.d/ussdresults restart > /dev/null 2>&1 &");
	//wait_apply("exec", "/etc/init.d/smsreports stop > /dev/null 2>&1 &");
	
	save_user_record("","SMS->USSD:Save");
}

$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$res = $aql->query("select * from ussd.conf");

$http_sw = '';
$http_use_default_user= 'checked';
if(isset($res['http_to_ussd']['enable'])) {
	if(is_true(trim($res['http_to_ussd']['enable']))){
		$http_sw = 'checked';
	}
}

$cors_sw = '';
if(isset($res['http_to_ussd']['cors_enable'])) {
	if(is_true(trim($res['http_to_ussd']['cors_enable']))){
		$cors_sw = 'checked';
	}
}

$allow_access_origin_url = "*";
if(isset($res['http_to_ussd']['allow_access_origin_url'])){
	if(strstr($res['http_to_ussd']['allow_access_origin_url'], 'http')){
		$tmp_val = explode('//', $res['http_to_ussd']['allow_access_origin_url']);
		$allow_access_origin_url = $tmp_val[1];
	} else {
		$allow_access_origin_url = trim($res['http_to_ussd']['allow_access_origin_url']);
	}
}

$http_url = "http://".$_SERVER['SERVER_NAME'].":".$_SERVER['SERVER_PORT']."/sendussd?username=xxx&password=xxx&message=xxx&[port=xxx&amp;][timeout=xxx&][id=xxx]";

if(isset($res['http_to_ussd']['username'])) {
	$http_username = trim($res['http_to_ussd']['username']);
}else{
	$http_username = "ussduser";
}

if(isset($res['http_to_ussd']['use_default_user'])) {
	if(is_true(trim($res['http_to_ussd']['use_default_user']))){
		$http_use_default_user = 'checked';
	} else {
		$http_use_default_user = '';
	}
}

if(isset($res['http_to_ussd']['password'])) {
	$http_password = trim($res['http_to_ussd']['password']);
}else{
	$http_password = "ussdpwd";
}

if(isset($res['http_to_ussd']['port'])) {
	$http_port = trim($res['http_to_ussd']['port']);
} else {
	$http_port = "all";
}

if(isset($res['http_to_ussd']['report'])) {
	$http_report = trim($res['http_to_ussd']['report']);
} else {
	$http_report = "json";
}

$sms_sw = '';
if(isset($res['ussd_to_http']['ussd_to_http_enable'])) {
	if(is_true(trim($res['ussd_to_http']['ussd_to_http_enable']))){
		$sms_sw = 'checked';
	}
}

if(isset($res['ussd_to_http']['url_http'])){
	$url_http = trim($res['ussd_to_http']['url_http']);
}else{
	$url_http = 'http';
}

if(isset($res['ussd_to_http']['url_host'])) {
	$url_host=trim($res['ussd_to_http']['url_host']);
} else {
	$url_host="";
}

if(isset($res['ussd_to_http']['url_port'])) {
	$url_port=trim($res['ussd_to_http']['url_port']);
} else {
	$url_port="";
}

if(isset($res['ussd_to_http']['url_path'])) {
	$url_path=trim($res['ussd_to_http']['url_path'], "\t\n\r\0/");
} else {
	$url_path="";
}

if(isset($res['ussd_to_http']['url_to_num'])) {
	$url_to_num=trim($res['ussd_to_http']['url_to_num']);
} else {
	$url_to_num="";
}

if(isset($res['ussd_to_http']['url_message'])) {
	$url_message=trim($res['ussd_to_http']['url_message']);
} else {
	$url_message="";
}

if(isset($res['ussd_to_http']['url_time'])) {
	$url_time=trim($res['ussd_to_http']['url_time']);
} else {
	$url_time="";
}

if(isset($res['ussd_to_http']['url_status'])) {
	$url_status=trim($res['ussd_to_http']['url_status']);
} else {
	$url_status="";
}

if(isset($res['ussd_to_http']['url_code'])) {
	$url_code=trim($res['ussd_to_http']['url_code']);
} else {
	$url_code="";
}

if(isset($res['ussd_to_http']['url_id'])){
	$url_id = trim($res['ussd_to_http']['url_id']);
}else{
	$url_id = "id";
}

if(isset($res['ussd_to_http']['url_user_defined'])) {
	$url_user_defined=trim($res['ussd_to_http']['url_user_defined']);
} else {
	$url_user_defined="";
}

?>
<link type="text/css" href="/css/jquery-ui-1.10.2.custom.all.css" rel="stylesheet" media="all"/>
<script type="text/javascript" src="/js/jquery-ui-1.10.2.custom.all.min.js"></script>
<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<div class="content">
		<span class="title">
			<?php echo language('HTTP to USSD');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Enable')){ ?>
							<b><?php echo language('Enable');?>:</b><br/>
							<?php echo language('Enable help', "ON(enabled),OFF(disabled)");?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Enable CORS')){ ?>
							<b><?php echo language('Enable CORS');?>:</b><br/>
							<?php echo language('Enable CORS help', "ON(enabled),OFF(disabled)");?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Allow Access Origin Domain')){ ?>
							<b><?php echo language('Allow Access Origin Domain');?>:</b><br/>
							<?php echo language('Allow Access Origin Domain help','Allow Access Origin Domain'); ?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('URL')){ ?>
							<b><?php echo language('URL');?>:</b><br/>
							<?php 
								echo language('URL help','
									The URL for send ussd. <br>
									username: the login username for send ussd.<br>
									password: the login password for send ussd.<br>
									phonenumber: the destination telephone number. <br>
									message: the USSD contents. <br>
									port: the gsm port for send ussd. eg gsm-1.1,gsm-1.2. <br>
									report: the sending result report format. <br>
									timeout: how long to wait. <br>');
							?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('User Name')){ ?>
							<b><?php echo language('User Name');?>:</b><br/>
							<?php echo language('User Name help','UserName'); ?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Password')){ ?>
							<b><?php echo language('Password');?>:</b><br/>
							<?php echo language('Password help','Password'); ?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Port')){ ?>
							<b><?php echo language('Port');?>:</b><br/>
							<?php echo language('Port help', 'port'); ?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Report')){ ?>
							<b><?php echo language('Report');?>:</b><br/>
							<?php echo language('Report help', 'USSD send result Report');?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Enable');?>:
			</span>
			<div class="tab_item_right">
				<span><input type="checkbox" id="http_sw" name="http_sw" <?php echo $http_sw; ?> onchange="http_change_gen(this,'http_api_gen')" /></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Enable CORS');?>:
			</span>
			<div class="tab_item_right">
				<span><input type="checkbox" id="cors_sw" name="cors_sw" <?php echo $cors_sw; ?> onchange="cors_change_gen(this,'cors_api_gen')" /></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Allow Access Origin Domain');?>:
			</span>
			<div class="tab_item_right">
				<span id="chttp_username"></span>
				<input id="allow_access_origin_url" type="text" name="allow_access_origin_url" value="<?php echo $allow_access_origin_url;?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('URL');?>:
			</span>
			<div class="tab_item_right">
				<span><font size="2px" color="#000" ><?php echo language($http_url);?> </font></span>
				<button class="button_show_help" type="button" id="http_to_sms_help" title="Usage of HTTP to USSD">
					<img src="/images/help.png">
				</button>
			</div>
			<div class="clear"></div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('User Name');?>:
			</span>
			<div class="tab_item_right">
				<span id="chttp_username"></span>
				<input type="checkbox" id="http_use_default_user" name="http_use_default_user" <?php echo $http_use_default_user; ?> onchange="check_server_user();" />
				<?php echo language("Use default user and password");?>
				<input id="http_username" type="text" name="http_username" value="<?php echo $http_username;?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Password');?>:
			</span>
			<div class="tab_item_right">
				<span id="chttp_password"></span>
				<input id="http_password" type="password" name="http_password" value="<?php echo $http_password;?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Port');?>:
			</span>
			<div class="tab_item_right">
				<table cellpadding="0" cellspacing="0" class="port_table" id="port_table" style="width: 100%;">
					<?php
					$j = 0;
					for($i=1;$i<=$__GSM_SUM__;$i++){
						$port_name = get_gsm_name_by_channel($i);
						$port_name_arr = explode("-", $port_name, 2);
						$port_type = $port_name_arr[0];
						if($port_type == "null") continue;
						
						$checked = '';
						$each_port = 'gsm-1.'.$i;
						if($http_port == 'all'){
							$checked = 'checked';
						}else{
							$temp = explode(",",$http_port);
							for($k=0;$k<count($temp);$k++){
								if($temp[$k] == $each_port){
									$checked = 'checked';
								}
							}
						}
						
						echo "<td class='module_port' ><input type='checkbox' id='gsm_1_$i' name='checkbox_name[]' value='gsm-1.$i' $checked onclick='set_check_all()' >";
						echo $port_name;
						echo '</td>';
						$j++;
					}
					?>
					<tr style="border:none;">
						<td style="border:none;">
							<input type="text" id="http_port_select" name="http_port_select" style="display:none" value="<?php echo $http_port; ?>" /> 
							<input type="checkbox" id="checkbox_all" onclick="select_all();"><?php echo language('All');?>
						</td>
					</tr>
				</table>
			</div>
			<div class="clear"></div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Report');?>:
			</span>
			<div class="tab_item_right">
				<select id="http_report" name="http_report" >
					<option  value="json" <?php if($http_report == "json") echo "selected" ?> ><?php echo language('JSON');?></option>
					<option  value="string" <?php if($http_report == "string") echo "selected" ?> ><?php echo language('String');?></option>
					<option  value="no" <?php if($http_report == "no") echo "selected" ?> ><?php echo language('No Report');?></option>
				</select>
			</div>
		</div>
	</div>
	
	<div class="content">
		<span class="title">
			<?php echo language('USSD Result');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Enable')){ ?>
							<b><?php echo language('Enable');?>:</b><br/>
							<?php echo language('Enable help', "ON(enabled),OFF(disabled)");?>
						
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('URL')){ ?>
							<b><?php echo language('URL');?>:</b><br/>
							<?php echo language('URL help', 'The USSD receive HTTP URL'); ?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Enable');?>:
			</span>
			<div class="tab_item_right">
				<span><input type="checkbox" id="sms_sw" name="sms_sw" <?php echo $sms_sw; ?> onchange="sms_to_http_change('div_tab_sms_hide')" /></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('URL');?>:
			</span>
			<div class="tab_item_right">
				<span id="csms_url"></span>
				<select name="url_http" id="url_http" style="width:70px;">
					<option value="http" <?php if($url_http == 'http') echo 'selected';?>>http</option>
					<option value="https" <?php if($url_http == 'https') echo 'selected';?>>https</option>
				</select>
					://&nbsp;
				<input id="url_host" type="text" name="url_host" value="<?php echo $url_host; ?>" style="width:78px;" 
					onfocus="_onfocus(this,'host')" onblur="_onblur(this,'host')" />:
				<input id="url_port" type="text" name="url_port" value="<?php echo $url_port; ?>" style="width:28px;" 
					onfocus="_onfocus(this,'port')" onblur="_onblur(this,'port')" />/
				<input id="url_path" type="text" name="url_path" value="<?php echo $url_path; ?>" style="width:78px;" 
					onfocus="_onfocus(this,'path')" onblur="_onblur(this,'path')" />?
				<input id="url_to_num" type="text" name="url_to_num" value="<?php echo $url_to_num; ?>" style="width:28px;" 
					onfocus="_onfocus(this,'key')" onblur="_onblur(this,'key')" />
					=port&amp;
				<input id="url_message" type="text" name="url_message" value="<?php echo $url_message; ?>" style="width:38px;" 
					onfocus="_onfocus(this,'key')" onblur="_onblur(this,'key')" />
					=message&amp;
				<input id="url_time" type="text" name="url_time" value="<?php echo $url_time; ?>" style="width:25px;" 
					onfocus="_onfocus(this,'key')" onblur="_onblur(this,'key')" />
					=time&amp;
				<input id="url_status" type="text" name="url_status" value="<?php echo $url_status; ?>" style="width:28px;" 
					onfocus="_onfocus(this,'key')" onblur="_onblur(this,'key')" />
					=status&amp;
				<input id="url_code" type="text" name="url_code" value="<?php echo $url_code; ?>" style="width:28px;" 
					onfocus="_onfocus(this,'key')" onblur="_onblur(this,'key')" />
					=code&amp;
				<input id="url_id" type="text" name="url_id" value="<?php echo $url_id; ?>" style="width:20px;"
					onfocus="_onfocus(this,'key')" onblur="_onblur(this,'key')" />
					=id&amp;
				<input id="url_user_defined" type="text" name="url_user_defined" value="<?php echo $url_user_defined; ?>" style="width:28px;" 
					onfocus="_onfocus(this,'User Defined')" onblur="_onblur(this,'User Defined')" />
				<span id="csms_url"></span>
				<button class="button_show_help" type="button" id="sms_to_http_help" title="Usage of USSD to HTTP">
					<img src="/images/help.png">
				</button>
			</div>
			<div class="clear"></div>
		</div>
	</div>

	<input type="hidden" name="send" id="send" value="" />
	
	<div id="button_save">
	
		<?php if(!$only_view){ ?>
		<button type="submit" class="float_btn gen_short_btn" onclick="document.getElementById('send').value='Save';return check();" ><?php echo language('Save');?></button>
		<?php } ?>
		
	</div>
	
	<div id="preview_dg" title="<?php echo language('USSD to HTTP Usage')?>" style="display:none;width:470px;height:100px">
		<div>
			<div id="timemsg" style="display:block;width:500px;height:100px;margin:0" contenteditable = "false">
				<p id="display_info"></p>
				<div id="prompt_info"></div>
			</div>
		</div>
	</div>
</form>

<script>
function http_change_gen(obj, showId){
	$("#"+showId).slideToggle();
}

function cors_change_gen(obj, showId){
	$("#cors_api_adv").hide();
	$("#"+showId).slideToggle();

}

function check_server_user(){
	var server_user_sw = document.getElementById('http_use_default_user').checked;
	if(!server_user_sw){
		$('#http_username').removeAttr("disabled");
		$('#http_password').removeAttr("disabled");
	} else {
		$('#http_username').attr("disabled","true");
		$('#http_password').attr("disabled","true");
	}
}

var select_all_flag = false;
function select_all(){
	var inputs = document.getElementsByTagName("input");     
	for(var i=0;i<inputs.length;i++){
		if(inputs[i].getAttribute("name") == "checkbox_name[]"){     
			if(select_all_flag == false)
				inputs[i].checked = true;
			else
				inputs[i].checked = false;
		}
	}
	if(select_all_flag == false)
		select_all_flag = true;
	else
		select_all_flag = false;	
	set_check_all();
}

function set_check_all(){
	var target_obj = document.getElementById("http_port_select"); 
	target_obj.value = '';
	var inputs = document.getElementsByTagName("input");
	for(var i=0;i<inputs.length;i++){
		if( (inputs[i].getAttribute("type")=="checkbox" || inputs[i].getAttribute("type")=="Checkbox") && inputs[i].getAttribute("name")=="checkbox_name[]"){
			if(inputs[i].checked){
				set_check_all_value(inputs[i]);
			} else {
				remove_check_all_value(inputs[i]);
			}
		}
	}
}

function set_check_all_value(check_obj){
	var target_obj = document.getElementById("http_port_select"); 
	if(target_obj.value == ''){
		target_obj.value = check_obj.value;
	} else {
		target_obj.value += "," + check_obj.value; 
	}
}

function remove_check_all_value(check_obj){
	var target_obj = document.getElementById("http_port_select"); 
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

function http_change_adv(obj, showId){
	$('#'+showId).slideToggle();
}

function sms_to_http_change(showClass){
	var sms_sw = document.getElementById('sms_sw').checked;
	if(sms_sw){
		$("."+showClass).show();
	} else {
		$("."+showClass).hide();
	}
}

function _onfocus(obj,str){
	if (obj.value == str) {
		obj.value =''
		obj.style.color = '#000000';
	}
}

function _onblur(obj,str){
	if (trim(obj.value) =='') {
		obj.value = str;
		obj.style.color = '#aaaaaa';
	}
}

function http_sms(){
	if($('#http_sw').attr("checked")=="checked")$("#http_api_gen").show();
	if($('#cors_sw').attr("checked") == "checked"){
		$("#cors_api_gen").show();
	} else {
		$("#cors_api_gen").hide();
	}
	if($('#sms_sw').attr("checked")== "checked") {
		$("#sms_api_gen").show();
		$("#sms_reports_gen").show();
		$("#sms_results_gen").show();
	} else {
		$("#sms_api_gen").hide();
		$("#sms_reports_gen").hide();
		$("#sms_results_gen").hide();
	}
	//if($('#sms_reports_sw').attr("checked")=="checked")$("#sms_report_api").show();
}

function check_server_user(){
	var server_user_sw = document.getElementById('http_use_default_user').checked;
	if(!server_user_sw){
		$('#http_username').removeAttr("disabled");
		$('#http_password').removeAttr("disabled");
	} else {
		$('#http_username').attr("disabled","true");
		$('#http_password').attr("disabled","true");
	}
}

function check_url_help(){
	var url_host = document.getElementById("url_host");
	var url_port = document.getElementById("url_port");
	var url_path = document.getElementById("url_path");
	var url_to_num = document.getElementById("url_to_num");
	var url_message = document.getElementById("url_message");
	var url_time = document.getElementById("url_time");
	var url_code = document.getElementById("url_code");
	var url_id = document.getElementById("url_id");
	var url_user_defined = document.getElementById("url_user_defined");
	
	if(trim(url_host.value) == ''){
		url_host.value = 'host';
		url_host.style.color = '#aaaaaa';
	}
	if(trim(url_port.value) == ''){
		url_port.value = 'port';
		url_port.style.color = '#aaaaaa';
	}
	if(trim(url_path.value) == ''){
		url_path.value = 'path';
		url_path.style.color = '#aaaaaa';
	}
	if(trim(url_to_num.value) == ''){
		url_to_num.value = 'key';
		url_to_num.style.color = '#aaaaaa';
	}
	if(trim(url_message.value) == ''){
		url_message.value = 'key';
		url_message.style.color = '#aaaaaa';
	}
	if(trim(url_time.value) == ''){
		url_time.value = 'key';
		url_time.style.color = '#aaaaaa';
	}
	if(trim(url_code.value) == ''){
		url_code.value = 'key';
		url_code.style.color = '#aaaaaa';
	}
	if(trim(url_id.value) == ''){
		url_id.value = 'key';
		url_id.style.color = '#aaaaaa';
	}
	if(trim(url_user_defined.value) == ''){
		url_user_defined.value = 'User Defined';
		url_user_defined.style.color = '#aaaaaa';
	}
}

function preview_dialog(info) {
	$("#display_info").html(info);
	$("#preview_dg").dialog({
		resizable: false,
		height:600,
		width:800,
		modal: true,

		buttons: [
			{
				text:"<?php echo language('Close');?>",
				id:"close",
				style:"text-align: center",
				click:function(){
					
				    $(this).dialog( "close" );
				    send_dump_capture_request();
				}
			}
		]		
	});
}

$("#show_password").change(function(){
	if($(this).attr("checked") == 'checked'){
		$("#http_password").prop("type","text");
	}else{
		$("#http_password").prop("type","password");
	}
});

$(document).ready(function(){
	http_sms();
	check_server_user();
	check_url_help();
	
	$('#sms_to_http_help').click(function(){
		var serverFile = '/cgi-bin/php/ajax_server.php';
		$.ajax({
			url: serverFile + "?random=" + Math.random() + "&type=system&action=get_http_api_info&filename=usage_ussdTohttp",
			type: 'GET',
			dataType: "text",
			data: {},
			success: function(info){
				preview_dialog(info); 
			},
			error: function(){
			},
		});
		
	});
	$("#http_to_sms_help").click(function(){
		var serverFile = '/cgi-bin/php/ajax_server.php';
		$.ajax({
			url: serverFile + "?random=" + Math.random() + "&type=system&action=get_http_api_info&filename=usage_sendussd",
			type: 'GET',
			dataType: "text",
			data: {},
			success: function(info){
				preview_dialog(info); 
			},
			error: function(){
			},
		});
	});
});
</script>

<?php 
function save_to_ussd_conf(){
	$sms_conf_path = '/etc/asterisk/gw/ussd.conf';
	
	if(!file_exists($sms_conf_path)){
		touch($sms_conf_path);
	}
	
	$aql = new aql();
	$setok = $aql->set('basedir', '/etc/asterisk/gw');
	if (!$setok) {
		echo $aql->get_error();
		return;
	}
	
	$hlock = lock_file($sms_conf_path);

	if(!$aql->open_config_file($sms_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	
	$exist_array = $aql->query("select * from ussd.conf");
	
	//http_to_ussd
	if(!isset($exist_array['http_to_ussd'])){
		$aql->assign_addsection('http_to_ussd', '');
	}
	
	if(isset($_POST['http_sw'])) {
		$http_sw = 'on';
	} else {
		$http_sw = 'off';
	}
	if(isset($exist_array['http_to_ussd']['enable'])) {
		$aql->assign_editkey('http_to_ussd','enable',$http_sw);
	} else {
		$aql->assign_append('http_to_ussd','enable',$http_sw);
	}
	
	if(isset($_POST['cors_sw'])) {
		$cors_sw = 'on';
	} else {
		$cors_sw = 'off';
	}
	if(isset($exist_array['http_to_ussd']['cors_enable'])) {
		$aql->assign_editkey('http_to_ussd','cors_enable',$cors_sw);
	} else {
		$aql->assign_append('http_to_ussd','cors_enable',$cors_sw);
	}
	
	if(isset($_POST['allow_access_origin_url'])) {
		$val = trim($_POST['allow_access_origin_url']);
		if($val != '' && $val != "*"){
			if(!strstr($val, "http")){
				$val = $_POST['url_http']."://".$val;
			}
		}
		if(isset($exist_array['http_to_ussd']['allow_access_origin_url'])){
			$aql->assign_editkey('http_to_ussd', 'allow_access_origin_url', $val);
		} else {
			$aql->assign_append('http_to_ussd', 'allow_access_origin_url', $val);
		}
	}
	
	if(isset($_POST['http_use_default_user'])) {
		$val = 'on';
	} else {
		$val = 'off';
	}
	if(isset($exist_array['http_to_ussd']['use_default_user'])) {
		$aql->assign_editkey('http_to_ussd','use_default_user',$val);
	} else {
		$aql->assign_append('http_to_ussd','use_default_user',$val);
	}
	
	if(isset($_POST['http_use_default_user'])){
		$http_username = "ussduser";
		$http_password = "ussdpwd";
	}else{
		$http_username = trim($_POST['http_username']);
		$http_password = trim($_POST['http_password']);
	}
	if(isset($exist_array['http_to_ussd']['username'])) {
		$aql->assign_editkey('http_to_ussd','username',$http_username);
	} else {
		$aql->assign_append('http_to_ussd','username',$http_username);
	} 
	if(isset($exist_array['http_to_ussd']['password'])) {
		$aql->assign_editkey('http_to_ussd','password',$http_password);
	} else {
		$aql->assign_append('http_to_ussd','password',$http_password);
	} 
	
	if(isset($_POST['http_port_select'])) {
		$val = trim($_POST['http_port_select']);
		if(isset($exist_array['http_to_ussd']['port'])) {
			$aql->assign_editkey('http_to_ussd','port',$val);
		} else {
			$aql->assign_append('http_to_ussd','port',$val);
		} 
	}
	
	if(isset($_POST['http_report'])) {
		$val = trim($_POST['http_report']);
		if(isset($exist_array['http_to_ussd']['report'])) {
			$aql->assign_editkey('http_to_ussd','report',$val);
		} else {
			$aql->assign_append('http_to_ussd','report',$val);
		} 
	}
	
	//ussd_to_http
	if(!isset($exist_array['ussd_to_http'])){
		$aql->assign_addsection('ussd_to_http', '');
	}
	
	if(isset($_POST['sms_sw'])) {
		$val = 'on';
	} else {
		$val = 'off';
	}
	if(isset($exist_array['ussd_to_http']['ussd_to_http_enable'])) {
		$aql->assign_editkey('ussd_to_http','ussd_to_http_enable',$val);
	} else {
		$aql->assign_append('ussd_to_http','ussd_to_http_enable',$val);
	}
	
	$url_http = $_POST['url_http'];
	if(isset($exist_array['ussd_to_http']['url_http'])){
		$aql->assign_editkey('ussd_to_http','url_http',$url_http);
	}else{
		$aql->assign_append('ussd_to_http','url_http',$url_http);
	}
	
	if(isset($_POST['url_host'])) {
		$val = trim($_POST['url_host']);
		if(isset($exist_array['ussd_to_http']['url_host'])) {
			$aql->assign_editkey('ussd_to_http','url_host',$val);
		} else {
			$aql->assign_append('ussd_to_http','url_host',$val);
		}
	}
	
	if(isset($_POST['url_port'])) {
		$val = trim($_POST['url_port']);
		if(isset($exist_array['ussd_to_http']['url_port'])) {
			$aql->assign_editkey('ussd_to_http','url_port',$val);
		} else {
			$aql->assign_append('ussd_to_http','url_port',$val);
		}
	}
	
	if(isset($_POST['url_path'])) {
		$val = trim($_POST['url_path']);
		if(isset($exist_array['ussd_to_http']['url_path'])) {
			$aql->assign_editkey('ussd_to_http','url_path','/'.$val);
		} else {
			$aql->assign_append('ussd_to_http','url_path','/'.$val);
		}
	}
	
	if(isset($_POST['url_to_num'])) {
		$val = trim($_POST['url_to_num']);
		if(isset($exist_array['ussd_to_http']['url_to_num'])) {
			$aql->assign_editkey('ussd_to_http','url_to_num',$val);
		} else {
			$aql->assign_append('ussd_to_http','url_to_num',$val);
		}
	}
	
	if(isset($_POST['url_message'])) {
		$val = trim($_POST['url_message']);
		if(isset($exist_array['ussd_to_http']['url_message'])) {
			$aql->assign_editkey('ussd_to_http','url_message',$val);
		} else {
			$aql->assign_append('ussd_to_http','url_message',$val);
		}
	}
	
	if(isset($_POST['url_time'])) {
		$val = trim($_POST['url_time']);
		if(isset($exist_array['ussd_to_http']['url_time'])) {
			$aql->assign_editkey('ussd_to_http','url_time',$val);
		} else {
			$aql->assign_append('ussd_to_http','url_time',$val);
		}
	}
	
	if(isset($_POST['url_status'])) {
		$val = trim($_POST['url_status']);
		if(isset($exist_array['ussd_to_http']['url_status'])) {
			$aql->assign_editkey('ussd_to_http','url_status',$val);
		} else {
			$aql->assign_append('ussd_to_http','url_status',$val);
		}
	}
	
	if(isset($_POST['url_code'])) {
		$val = trim($_POST['url_code']);
		if(isset($exist_array['ussd_to_http']['url_code'])) {
			$aql->assign_editkey('ussd_to_http','url_code',$val);
		} else {
			$aql->assign_append('ussd_to_http','url_code',$val);
		}
	}
	
	if(isset($_POST['url_id'])){
		$val = trim($_POST['url_id']);
		if(isset($exist_array['ussd_to_http']['url_id'])){
			$aql->assign_editkey('ussd_to_http','url_id',$val);
		}else{
			$aql->assign_append('ussd_to_http','url_id',$val);
		}
	}
	
	if(isset($_POST['url_user_defined'])) {
		if(trim($_POST['url_user_defined']) == 'User Defined'){
			$_POST['url_user_defined'] = '';
			$val = '';
		} else {
			$val = trim($_POST['url_user_defined']);
		}
		if(isset($exist_array['ussd_to_http']['url_user_defined'])) {
			$aql->assign_editkey('ussd_to_http','url_user_defined',$val);
		} else {
			$aql->assign_append('ussd_to_http','url_user_defined',$val);
		}
	}
	
	if(isset($_POST['url_host']) && isset($_POST['url_port'])) {
		$val = "$url_http://".trim($_POST['url_host']).":".trim($_POST['url_port']).'/'.trim($_POST['url_path'])."?"
			.trim($_POST['url_to_num'])."=\${port}&"
			.trim($_POST['url_message'])."=\${message}&"
			.trim($_POST['url_time'])."=\${time}&"
			.trim($_POST['url_code'])."=\${code}&"
			.trim($_POST['url_id'])."=\${id}&"
			.trim($_POST['url_user_defined']);
		if(isset($exist_array['ussd_to_http']['url'])) {
			$aql->assign_editkey('ussd_to_http','url',$val);
		} else {
			$aql->assign_append('ussd_to_http','url',$val);
		}
	}
	
	if (!$aql->save_config_file('ussd.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	unlock_file($hlock);
}

?>

<?php require("/www/cgi-bin/inc/boot.inc");?>