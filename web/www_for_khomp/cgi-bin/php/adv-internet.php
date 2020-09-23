<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
require_once("/www/cgi-bin/inc/aql.php");
?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>

<?php 
function show_internet(){
	global $__GSM_SUM__;
	global $only_view;
?>
<form enctype="multipart/form-data" action="<?php echo $_SERVER['PHP_SELF'] ?>" method="post">
	
<?php
$aql = new aql();
$aql->set('basedir','/etc/asterisk/');
$res = $aql->query("select * from gw_internet.conf");

$reset_type = '';
if(isset($res['general']['reset_type'])){
	$reset_type = trim($res['general']['reset_type']);
}

$day = 1;
if(isset($res['general']['day'])){
	$day = trim($res['general']['day']);
}

$hour = 0;
if(isset($res['general']['hour'])){
	$hour = trim($res['general']['hour']);
}

$minute = 0;
if(isset($res['general']['minute'])){
	$minute = trim($res['general']['minute']);
}

//phone number switch
$aql->set('basedir','/etc/asterisk/gw');
$phonenum_res = $aql->query("select * from sim_query.conf");
?>

	<div class="content">
		<span class="title">
			<?php echo language('Internet');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('Timing internet')){ ?>
							<b><?php echo language('Timing internet')?>:</b><br/>
							<?php echo language('Timing internet help','Timing internet');?>
							
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('Timing time')){ ?>
							<b><?php echo language('Timing time')?>:</b><br/>
							<?php echo language('Timing time help','Timing time');?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Timing internet')?>:
			</span>
			<div class="tab_item_right">
				<select name="reset_type" id="reset_type">
					<option value=""><?php echo language('Close');?></option>
					<option value="by_month" <?php if($reset_type == 'by_month') echo 'selected'; ?>><?php echo language('By Month');?></option>
					<option value="by_day" <?php if($reset_type == 'by_day') echo 'selected';?>><?php echo language('By Day');?></option>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Timing time')?>:
			</span>
			<div class="tab_item_right">
				<span id="d"><?php echo language('Day');?></span>
				<select name="day" id="day" style="width:50px;">
				<?php
					for($j=1; $j<=31; $j++) {
						($day == $j)?$day_sel='selected':$day_sel='';
						echo "<option  value='$j' $day_sel>$j</option>";
					}
				?>
				</select>
				
				<span><?php echo language('Hour');?></span>
				<select  name="hour" id="hour" style="width:50px;">
				<?php
					for($j=0; $j<=23; $j++) {
						($hour == $j)?$hour_sel='selected':$hour_sel='';
						echo "<option  value='$j' $hour_sel>$j</option>";
					}
				?>
				</select>
				
				<span><?php echo language('Minute');?></span>
				<select name="minute" id="minute" style="width:50px;">
				<?php
					for($j=0; $j<=59; $j++) {
						($minute == $j)?$minute_sel='selected':$minute_sel='';
						echo "<option  value='$j' $minute_sel>$j</option>";
					}
				?>
				</select>
			</div>
		</div>
	</div>

	<div class="content">
		<table class="table_show">
			<input type="hidden" id="sel_internet" name="sel_internet" value="" />
			<tr>
				<th style="width:03%" class="nosort">
				<input type="checkbox" name="selall" onclick="selectall(this.checked)" />
				</th>		
				<th width=""><?php echo language('ID');?></th>
				
				<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
				<th><?php echo language('Mobile Number');?></th>
				<?php } ?>
				
				<th width=""><?php echo language('Open')?></th>
				<th width=""><?php echo language('APN User Name');?></th>
				<th width=""><?php echo language('APN Password');?></th>
				<th width=""><?php echo language('APN');?></th>
				<th width=""><?php echo language('URL')?></th>
				<th width=""><?php echo language('MAX')?>(MB)</th>
				<th width=""><?php echo language('USED')?></th>
				<th width=""><?php echo language('Time')?></th>
				<th width="30px"><?php echo language('Save')?></th>
			</tr>
		
<?php
	
	for ($i = 0; $i <= $__GSM_SUM__; $i++) {
		$port_name = get_gsm_name_by_channel($i);
		if(!strstr($port_name, 'lte') && $i != 0) continue;
		
		$hcc="style=background-color:#ECFFEF";
		
		$internet_sw = '0';
		$port = 'port'.$i;
		if(isset($res[$port]['internet_sw'])){
			$internet_sw = trim($res[$port]['internet_sw']);
		}
		
		$username = '';
		if(isset($res[$port]['username'])){
			$username = trim($res[$port]['username']);
		}
		
		$passwd = '';
		if(isset($res[$port]['passwd'])){
			$passwd = trim($res[$port]['passwd']);
		}
		
		$apn = '';
		if(isset($res[$port]['apn'])){
			$apn = trim($res[$port]['apn']);
		}
		
		$flow_total_size = '';
		if(isset($res[$port]['flow_total_size'])){
			$flow_total_size = trim($res[$port]['flow_total_size']);
		}
		
		$url = '';
		if(isset($res[$port]['url'])){
			$url = trim($res[$port]['url']);
		}
		
		$time = '';
		if(isset($res[$port]['time'])){
			$time = trim($res[$port]['time']);
		}
		
		$phonenum = '';
		if(($phonenum_res[$i]['query_type']&240) != 0){
			exec("/my_tools/redis-cli hget app.simquery.phonenum.channel $i",$phone_output);
			$phonenum = $phone_output[0];
			$phone_output = '';
		}
?>
		<tr>
			<td <?php if ($i==0) {echo $hcc;}?>>
				<?php if ($i != 0) {?>
				<input type="checkbox" name="<?php echo 'channel'.$i; ?>" id="<?php echo 'channel'.$i; ?>"/>
				<input type="hidden" name="get_chan[]" class="get_chan" value="<?php echo $i;?>" />
				<?php }?>
			</td>
			<td <?php if ($i==0) {echo $hcc;}?>>
				<?php if ($i != 0){ echo $port_name;}?>
			</td>
			
			<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
			<td <?php if ($i==0) {echo $hcc;}?>><?php echo $phonenum;?></td>
			<?php } ?>
			
			<td <?php if ($i==0) {echo $hcc;}?>>
				<select name="<?php echo 'internet_sw'.$i?>" class="internet_sw" id="<?php echo 'internet_sw'.$i?>">
					<option value="0" <?php if($internet_sw=='off') echo 'selected';?>><?php echo language("_No");?></option>
					<option value="1" <?php if($internet_sw=='on') echo 'selected';?>><?php echo language("_Yes");?></option>
				</select>
			</td>
			<td <?php if ($i==0) {echo $hcc;}?>>
				<input type="text" name="<?php echo 'username'.$i; ?>" id="<?php echo 'username'.$i; ?>" value="<?php echo $username;?>" style="width: 100px;" />
			</td>
			<td <?php if ($i==0) {echo $hcc;}?>>
				<input type="text" name="<?php echo 'passwd'.$i; ?>" id="<?php echo 'passwd'.$i; ?>" value="<?php echo $passwd;?>" style="width: 100px;"/>
			</td>
			<td <?php if ($i==0) {echo $hcc;}?>>
				<input type="text" name="<?php echo 'apn'.$i; ?>" id="<?php echo 'apn'.$i; ?>" value="<?php echo $apn;?>" style="width: 60px;"/>
			</td>
			<!--
			<td <?php if($i==0) {echo $hcc;}?>>
				<select name="<?php echo "reset_type".$i?>" class="reset_type" id="<?php echo "reset_type".$i;?>">
					<option><?php echo language('No Reset');?></option>
					<option value="by_month" <?php if($reset_type == 'by_month') echo 'selected'; ?>><?php echo language('By Month');?></option>
					<option value="by_day" <?php if($reset_type == 'by_day') echo 'selected';?>><?php echo language('By Day');?></option>
				</select>
			</td>
			<td <?php if($i==0) {echo $hcc;}?>>
				<select name="<?php echo "day".$i;?>" class="day" id="<?php echo "day".$i;?>">
				<?php
					for($j=1; $j<=31; $j++) {
						($day == $j)?$day_sel='selected':$day_sel='';
						echo "<option  value='$j' $day_sel>$j</option>";
					}
				?>
				</select>
				<?php echo language('d');?>
				<select  name="<?php echo "hour".$i;?>" class="hour" id="<?php echo "hour".$i;?>">
				<?php
					for($j=0; $j<=23; $j++) {
						($hour == $j)?$hour_sel='selected':$hour_sel='';
						echo "<option  value='$j' $hour_sel>$j</option>";
					}
				?>
				</select>
				<?php echo language('h');?>
				<select name="<?php echo "minute".$i;?>" class="minute" id="<?php echo "minute".$i;?>">
				<?php
					for($j=0; $j<=59; $j++) {
						($minute == $j)?$minute_sel='selected':$minute_sel='';
						echo "<option  value='$j' $minute_sel>$j</option>";
					}
				?>
				</select>
				<?php echo language('min');?>
			</td>
			-->
			<td <?php if ($i==0) {echo $hcc;}?>>
				<input type="text" name="<?php echo 'url'.$i; ?>" id="<?php echo 'url'.$i; ?>" value="<?php echo $url;?>" style="width: 150px;"/>
			</td>
			<td <?php if ($i==0) {echo $hcc;}?>>
				<input type="text" name="<?php echo 'flow_total_size'.$i; ?>" id="<?php echo 'flow_total_size'.$i; ?>" value="<?php echo $flow_total_size;?>" style="width: 50px;"/>
			</td>
			<td <?php if ($i==0) {echo $hcc;}?> id="used<?php echo $i;?>">
			</td>
			<td <?php if ($i==0) {echo $hcc;}?>>
				<?php echo $time;?>
			</td>
			<td <?php if ($i==0) {echo $hcc;}?>>
				<?php if ($i != 0) {?>
				
				<?php if(!$only_view){ ?>
				<button type="submit" value="Modify" style="width:32px;height:32px;padding:0;" onclick="document.getElementById('send').value='Modify';document.getElementById('sel_internet').value=<?php echo $i;?>">
					<img src="/images/save.png">
				</button>
				<?php } ?>
				
				<?php }?>
			</td>
		</tr>
<?php
	}
?>
		</table>
	</div>
	
	<input type="hidden" name="send" id="send" value="Save" />
	
	<div id="button_save">
		
		<?php if(!$only_view){ ?>
		<button type="submit" id="sendlabel" onclick="document.getElementById('send').value='Save';return check_batch();"><?php echo language('Save');?></button>
		<?php } ?>
		
		<button type="button" onclick="location.reload()" ><?php echo language('Cancel');?></button>
		<button type="button" onclick="check_batch();setValue();" ><?php echo language('Batch');?></button>
	</div>
</form>

<script>
	function selectall(checked){
		if(checked == true){
			$(":checkbox").attr("checked", true);
		}else{
			$(":checkbox").removeAttr("checked");
		}
	}
	
	function setValue(){
		var internet_sw_auto = document.getElementById('internet_sw0').value;
		var username_auto = document.getElementById('username0').value;
		var password_auto = document.getElementById('passwd0').value;
		var apn_auto = document.getElementById('apn0').value;
		// var reset_type_auto = document.getElementById('reset_type0').value;
		// var day_auto = document.getElementById('day0').value;
		// var hour_auto = document.getElementById('hour0').value;
		// var minute_auto = document.getElementById('minute0').value;
		var flow_total_size_auto = document.getElementById('flow_total_size0').value;
		var url_auto = document.getElementById('url0').value;
		
		$(".get_chan").each(function(){
			var channel = $(this).val();
			if(document.getElementById('channel'+channel).checked == true){
				document.getElementById('internet_sw'+channel).value = internet_sw_auto;
				document.getElementById('username'+channel).value = username_auto;
				document.getElementById('passwd'+channel).value = password_auto;
				document.getElementById('apn'+channel).value = apn_auto;
				// document.getElementById('reset_type'+channel).value = reset_type_auto;
				// document.getElementById('day'+channel).value = day_auto;
				// document.getElementById('hour'+channel).value = hour_auto;
				// document.getElementById('minute'+channel).value = minute_auto;
				document.getElementById('flow_total_size'+channel).value = flow_total_size_auto;
				document.getElementById('url'+channel).value = url_auto;
			}
		});
	}
	
	function check_batch(){
		var flag = 0;
		$(":checkbox").each(function(){
			if($(this).attr('checked') == 'checked'){
				flag = 1;
				return false;
			}
		});
		if(flag == 0){
			alert('Select port alert');
			return false;
		}
		return true;
	}
	
	var arr = [];
	$(".internet_sw").each(function(){
		if($(this).find("option:selected").val() == 1){
			var channel = $(this).parent().siblings().children('.get_chan').val();
			arr.push(channel);
		}
	});
	function get_used_val(){
		$.ajax({
			type: "POST",
			url: "ajax_server.php?type=internet",
			datatype: "json",
			data: {
				channel: JSON.stringify(arr)
			},
			success: function(data){
				var temp_arr = JSON.parse(data);
				for(var ind in temp_arr){
					var channel = ind;
					if(temp_arr[ind][0] == '') temp_arr[ind][0] = 0;
					var value = parseInt(temp_arr[ind][0]);
					var size = '';
					if( value < 0.1*1024){
						size = value.toFixed(2);
						size += 'B';
					}else if(value < 0.1*1024*1024){
						size = (value/1024).toFixed(2);
						size += 'KB';
					}else if(value < 0.1*1024*1024*1024){
						size = (value/(1024*1024)).toFixed(2);
						size += 'MB';
					}else{
						size = (value/(1024*1024*1024)).toFixed(2);
						size += 'GB';
					}
					$("#used"+channel).text(size);
				}
				setTimeout("get_used_val()", 2000);
			}
		});
	}
	if(arr.length != 0){
		get_used_val();
	}
	
	(function(){
		$("#reset_type").click(function(){
			if($(this).val()=='by_day'){
				$("#day").hide();
				$("#d").hide();
			}else if($(this).val()=='by_month'){
				$("#day").show();
				$("#d").show();
			}
		});
		
		if($("#reset_type").val()=='by_day'){
			$("#day").hide();
			$("#d").hide();
		}else if($("#reset_type").val()=='by_month'){
			$("#day").show();
			$("#d").show();
		}
	}());
</script>

<?php
}

function save_all_internet(){
	global $__GSM_SUM__;
	$aql = new aql();
	$gw_internet_conf_path = '/etc/asterisk/gw_internet.conf';
	$hlock = lock_file($gw_internet_conf_path);
	if(!file_exists($gw_internet_conf_path)) {exec('touch /etc/asterisk/gw_internet.conf');}
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($gw_internet_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	
	$exist_array = $aql->query("select * from gw_internet.conf");
	
	if(!isset($exist_array['general'])){
		$aql->assign_addsection('general','');
	}
	
	$reset_type = $_POST['reset_type'];
	$day = $_POST['day'];
	$hour = $_POST['hour'];
	$minute = $_POST['minute'];
	
	if(isset($exist_array['general']['reset_type'])){
		$aql->assign_editkey('general','reset_type',$reset_type);
	}else{
		$aql->assign_append('general','reset_type',$reset_type);
	}
	
	if(isset($exist_array['general']['day'])){
		$aql->assign_editkey('general','day',$day);
	}else{
		$aql->assign_append('general','day',$day);
	}
	
	if(isset($exist_array['general']['hour'])){
		$aql->assign_editkey('general','hour',$hour);
	}else{
		$aql->assign_append('general','hour',$hour);
	}
	
	if(isset($exist_array['general']['minute'])){
		$aql->assign_editkey('general','minute',$minute);
	}else{
		$aql->assign_append('general','minute',$minute);
	}
	
	exec("pidof opvx-ppp.sh", $output);//Get online or not
	
	for($i=0;$i<count($_POST['get_chan']);$i++){
		$chan = $_POST['get_chan'][$i];
		if(!isset($_POST['channel'.$chan])) continue;
		
		$internet_sw = $_POST['internet_sw'.$chan];
		($internet_sw == 1)?$internet_sw = 'on':$internet_sw = 'off';
		$username = $_POST['username'.$chan];
		$passwd = $_POST['passwd'.$chan];
		$apn = $_POST['apn'.$chan];
		// $reset_type = $_POST['reset_type'.$chan];
		// $day = $_POST['day'.$chan];
		// $hour = $_POST['hour'.$chan];
		// $minute = $_POST['minute'.$chan];
		$flow_total_size = $_POST['flow_total_size'.$chan];
		$url = $_POST['url'.$chan];
		
		exec("date +\"%Y-%m-%d %H:%M:%S\"",$time_output);
		$time = $time_output[0];
		
		$port = 'port'.$chan;
		if(!isset($exist_array[$port])){
			$aql->assign_addsection($port,'');
		}
		
		if(isset($exist_array[$port]['internet_sw'])){
			$aql->assign_editkey($port,'internet_sw',$internet_sw);
		}else{
			$aql->assign_append($port,'internet_sw',$internet_sw);
		}
		
		if(isset($exist_array[$port]['username'])){
			$aql->assign_editkey($port,'username',$username);
		}else{
			$aql->assign_append($port,'username',$username);
		}
		
		if(isset($exist_array[$port]['passwd'])){
			$aql->assign_editkey($port,'passwd',$passwd);
		}else{
			$aql->assign_append($port,'passwd',$passwd);
		}
		
		if(isset($exist_array[$port]['apn'])){
			$aql->assign_editkey($port,'apn',$apn);
		}else{
			$aql->assign_append($port,'apn',$apn);
		}
		
		/*
		if(isset($exist_array[$port]['reset_type'])){
			$aql->assign_editkey($port,'reset_type',$reset_type);
		}else{
			$aql->assign_append($port,'reset_type',$reset_type);
		}
		
		if(isset($exist_array[$port]['day'])){
			$aql->assign_editkey($port,'day',$day);
		}else{
			$aql->assign_append($port,'day',$day);
		}
		
		if(isset($exist_array[$port]['hour'])){
			$aql->assign_editkey($port,'hour',$hour);
		}else{
			$aql->assign_append($port,'hour',$hour);
		}
		
		if(isset($exist_array[$port]['minute'])){
			$aql->assign_editkey($port,'minute',$minute);
		}else{
			$aql->assign_append($port,'minute',$minute);
		}
		*/
		if(isset($exist_array[$port]['flow_total_size'])){
			$aql->assign_editkey($port,'flow_total_size',$flow_total_size);
		}else{
			$aql->assign_append($port,'flow_total_size',$flow_total_size);
		}
		
		if(isset($exist_array[$port]['url'])){
			$aql->assign_editkey($port,'url',$url);
		}else{
			$aql->assign_append($port,'url',$url);
		}
		
		if(strstr($url, '://')){
			$temp = explode("://", $url);
			$temp = explode('/' ,$temp[1]);
			$domain = $temp[0];
		}else{
			$temp = explode('/' ,$url);
			$domain = $temp[0];
		}
		$domain = trim($domain);
		if(isset($exist_array[$port]['domain'])){
			$aql->assign_editkey($port,'domain',$domain);
		}else{
			$aql->assign_append($port,'domain',$domain);
		}
		
		if(isset($exist_array[$port]['time'])){
			$aql->assign_editkey($port,'time',$time);
		}else{
			$aql->assign_append($port,'time',$time);
		}
		
		if($output[0] == ''){
			if(isset($exist_array[$port]['flow_use_size'])){
				$aql->assign_editkey($port,'flow_use_size','');
			}else{
				$aql->assign_append($port,'flow_use_size','');
			}
		}
	}
	if (!$aql->save_config_file('gw_internet.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	
	$write = '';
	$save_reset_tools = 'root /my_tools/opvx-ppp.sh';
	if($reset_type == 'by_month'){
		$write = "$minute $hour $day * * $save_reset_tools";
	}else if($reset_type == 'by_day'){
		$write = "$minute $hour * * * $save_reset_tools";
	}
	
	$file_path = "/etc/asterisk/gw/crontabs_root";
	$chlock = lock_file($file_path);
	exec("sed -i '/\/my_tools\/opvx-ppp.sh/d' \"$file_path\" 2> /dev/null");
	if($write != '') exec("echo \"$write\" >> $file_path");
	unlock_file($chlock);
	wait_apply("exec", "sh /etc/init.d/cron restart > /dev/null 2>&1 &");
	
	if($output[0] == '' && $reset_type == ''){
		exec("/my_tools/opvx-ppp.sh > /dev/null 2>&1 &");
	}
}

function save_one_internet(){
	$aql = new aql();
	$gw_internet_conf_path = '/etc/asterisk/gw_internet.conf';
	$hlock = lock_file($gw_internet_conf_path);
	if(!file_exists($gw_internet_conf_path)) {exec('touch /etc/asterisk/gw_internet.conf');}
	$aql->set('basedir','/etc/asterisk');
	if(!$aql->open_config_file($gw_internet_conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	$exist_array = $aql->query("select * from gw_internet.conf");
	
	if(!isset($exist_array['general'])){
		$aql->assign_addsection('general','');
	}
	
	$reset_type = $_POST['reset_type'];
	$day = $_POST['day'];
	$hour = $_POST['hour'];
	$minute = $_POST['minute'];
	
	if(isset($exist_array['general']['reset_type'])){
		$aql->assign_editkey('general','reset_type',$reset_type);
	}else{
		$aql->assign_append('general','reset_type',$reset_type);
	}
	
	if(isset($exist_array['general']['day'])){
		$aql->assign_editkey('general','day',$day);
	}else{
		$aql->assign_append('general','day',$day);
	}
	
	if(isset($exist_array['general']['hour'])){
		$aql->assign_editkey('general','hour',$hour);
	}else{
		$aql->assign_append('general','hour',$hour);
	}
	
	if(isset($exist_array['general']['minute'])){
		$aql->assign_editkey('general','minute',$minute);
	}else{
		$aql->assign_append('general','minute',$minute);
	}
	
	exec("pidof opvx-ppp.sh", $output);//Get online or not
	
	if($_POST['sel_internet']!=''){
		$i = $_POST['sel_internet'];
		$internet_sw = $_POST['internet_sw'.$i];
		($internet_sw == 1)?$internet_sw = 'on':$internet_sw = 'off';
		$username = $_POST['username'.$i];
		$passwd = $_POST['passwd'.$i];
		$apn = $_POST['apn'.$i];
		// $reset_type = $_POST['reset_type'.$i];
		// $day = $_POST['day'.$i];
		// $hour = $_POST['hour'.$i];
		// $minute = $_POST['minute'.$chan];
		$flow_total_size = $_POST['flow_total_size'.$i];
		$url = $_POST['url'.$i];
		
		exec("date +\"%Y-%m-%d %H:%M:%S\"",$time_output);
		$time = $time_output[0];
		
		$port = 'port'.$i;
		if(!isset($exist_array[$port])){
			$aql->assign_addsection($port,'');
		}
		
		if(isset($exist_array[$port]['internet_sw'])){
			$aql->assign_editkey($port,'internet_sw',$internet_sw);
		}else{
			$aql->assign_append($port,'internet_sw',$internet_sw);
		}
		
		if(isset($exist_array[$port]['username'])){
			$aql->assign_editkey($port,'username',$username);
		}else{
			$aql->assign_append($port,'username',$username);
		}
		
		if(isset($exist_array[$port]['passwd'])){
			$aql->assign_editkey($port,'passwd',$passwd);
		}else{
			$aql->assign_append($port,'passwd',$passwd);
		}
		
		if(isset($exist_array[$port]['apn'])){
			$aql->assign_editkey($port,'apn',$apn);
		}else{
			$aql->assign_append($port,'apn',$apn);
		}
		
		/*
		if(isset($exist_array[$port]['reset_type'])){
			$aql->assign_editkey($port,'reset_type',$reset_type);
		}else{
			$aql->assign_append($port,'reset_type',$reset_type);
		}
		
		if(isset($exist_array[$port]['day'])){
			$aql->assign_editkey($port,'day',$day);
		}else{
			$aql->assign_append($port,'day',$day);
		}
		
		if(isset($exist_array[$port]['hour'])){
			$aql->assign_editkey($port,'hour',$hour);
		}else{
			$aql->assign_append($port,'hour',$hour);
		}
		
		if(isset($exist_array[$port]['minute'])){
			$aql->assign_editkey($port,'minute',$minute);
		}else{
			$aql->assign_append($port,'minute',$minute);
		}
		*/
		if(isset($exist_array[$port]['flow_total_size'])){
			$aql->assign_editkey($port,'flow_total_size',$flow_total_size);
		}else{
			$aql->assign_append($port,'flow_total_size',$flow_total_size);
		}
		
		if(isset($exist_array[$port]['url'])){
			$aql->assign_editkey($port,'url',$url);
		}else{
			$aql->assign_append($port,'url',$url);
		}
		
		if(strstr($url, '://')){
			$temp = explode("://", $url);
			$temp = explode('/' ,$temp[1]);
			$domain = $temp[0];
		}else{
			$temp = explode('/' ,$url);
			$domain = $temp[0];
		}
		$domain = trim($domain);
		if(isset($exist_array[$port]['domain'])){
			$aql->assign_editkey($port,'domain',$domain);
		}else{
			$aql->assign_append($port,'domain',$domain);
		}
		
		if(isset($exist_array[$port]['time'])){
			$aql->assign_editkey($port,'time',$time);
		}else{
			$aql->assign_append($port,'time',$time);
		}
		
		if($output[0] == ''){
			if(isset($exist_array[$port]['flow_use_size'])){
				$aql->assign_editkey($port,'flow_use_size','');
			}else{
				$aql->assign_append($port,'flow_use_size','');
			}
		}
	}
	if (!$aql->save_config_file('gw_internet.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	unlock_file($hlock);
	
	//cron
	$write = '';
	$save_reset_tools = 'root /my_tools/opvx-ppp.sh';
	if($reset_type == 'by_month'){
		$write = "$minute $hour $day * * $save_reset_tools";
	}else if($reset_type == 'by_day'){
		$write = "$minute $hour * * * $save_reset_tools";
	}
	
	$file_path = "/etc/asterisk/gw/crontabs_root";
	$chlock = lock_file($file_path);
	exec("sed -i '/\/my_tools\/opvx-ppp.sh/d' \"$file_path\" 2> /dev/null");
	if($write != '') exec("echo \"$write\" >> $file_path");
	unlock_file($chlock);
	wait_apply("exec", "sh /etc/init.d/cron restart > /dev/null 2>&1 &");
	
	
	if($output[0] == '' && $reset_type == ''){
		exec("/my_tools/opvx-ppp.sh > /dev/null 2>&1 &");
	}
}

if($_POST){
	if(isset($_POST['send']) && $_POST['send'] == 'Save'){
		if($only_view){
			return false;
		}
		
		save_all_internet();
		
		save_user_record("","ADVANCED->Internet:Save All");
	}else if($_POST['send'] == 'Modify'){
		if($only_view){
			return false;
		}
		
		save_one_internet();
		
		save_user_record("","ADVANCED->Internet:Save");
	}
	show_internet();
}else{
	show_internet();
}

require("/www/cgi-bin/inc/boot.inc");
?>

<div id="to_top"></div>
<div id="float_btn1" class="sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()" >
</div>