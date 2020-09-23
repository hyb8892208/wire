<?php require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");
include_once("/www/cgi-bin/inc/define.inc");
?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/float_btn.js"></script>

<?php 
function show_mms(){
	global $__GSM_SUM__;
	
	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$res = $aql->query("select * from mms.conf");

	if(isset($res['general']['download_type'])){
		$download_type = $res['general']['download_type'];
	}else{
		$download_type = '';
	}

	if(isset($res['general']['download_time'])){
		$download_time = $res['general']['download_time'];
		$temp = explode(':',$download_time);
		$hour = intval($temp[0]);
		$minute = intval($temp[1]);
		$second = intval($temp[2]);
	}else{
		$hour = 0;
		$minute = 0;
		$second = 0;
	}

	//phone number switch
	$aql->set('basedir','/etc/asterisk/gw');
	$phonenum_res = $aql->query("select * from sim_query.conf");
	?>
	<form enctype="multipart/form-data" action="<?php echo $_SERVER['PHP_SELF'] ?>" method="post">
		<div id="tab">
			<li class="tb1">&nbsp;</li>
			<li class="tbg"><?php echo language('MMS Settings');?></li>
			<li class="tb2">&nbsp;</li>
		</div>
		
		<table width="100%" class="tedit">
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Download Type')?>:
						<span class="showhelp">
						<?php echo language('Download Type help','Download Type');?>
						</span>
					</div>
				</th>
				<td >
					<select name="download_type" id="download_type">
						<option value="off"><?php echo language('Close');?></option>
						<option value="imme" <?php if($download_type == 'imme') echo 'selected'; ?>><?php echo language('Immediately@mms','Immediately');?></option>
						<option value="ontime" <?php if($download_type == 'ontime') echo 'selected';?>><?php echo language('On Time');?></option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Download Time')?>:
						<span class="showhelp">
						<?php echo language('Download Time help','Download Time');?>
						</span>
					</div>
				</th>
				<td >
					<select  name="hour" id="hour">
					<?php
						for($j=0; $j<=23; $j++) {
							($hour == $j)?$hour_sel='selected':$hour_sel='';
							echo "<option  value='$j' $hour_sel>$j</option>";
						}
					?>
					</select>
					<?php echo language('Hour');?>
					<select name="minute" id="minute">
					<?php
						for($j=0; $j<=59; $j++) {
							($minute == $j)?$minute_sel='selected':$minute_sel='';
							echo "<option  value='$j' $minute_sel>$j</option>";
						}
					?>
					</select>
					<?php echo language('Minute');?>
					<select name="second" id="second">
					<?php
						for($j=0; $j<=59; $j++) {
							($second == $j)?$second_sel='selected':$second_sel='';
							echo "<option  value='$j' $second_sel>$j</option>";
						}
					?>
					</select>
					<span id="d"><?php echo language('Second');?></span>
				</td>
			</tr>
		</table>
		
		<br/>
		
		<table width="100%" class="tshow">
		
			<input type="hidden" id="sel_internet" name="sel_internet" value="" />
			<tr>
				<th style="width:03%" class="nosort">
					<input type="checkbox" name="selall" onclick="selectall(this.checked)" />
				</th>
				<th width=""><?php echo language('ID');?></th>
				
				<?php if($phonenum_res['general']['phonenum_switch'] == 'on'){ ?>
				<th><?php echo language('Mobile Number');?></th>
				<?php } ?>
				
				<th width=""><?php echo language('APN User Name');?></th>
				<th width=""><?php echo language('APN Password');?></th>
				<th width=""><?php echo language('APN');?></th>
				<th width="30px"><?php echo language('Save')?></th>
			</tr>
			
		<?php
		for ($i = 0; $i <= $__GSM_SUM__; $i++) {
			$port_name = get_gsm_name_by_channel($i);
			if(!strstr($port_name, 'lte') && $i != 0) continue;
			
			$hcc="style=background-color:#daedf4";
			
			if(isset($res[$i]['apn'])){
				$apn = $res[$i]['apn'];
			}else{
				$apn = '';
			}
			
			if(isset($res[$i]['username'])){
				$username = $res[$i]['username'];
			}else{
				$username = '';
			}
			
			if(isset($res[$i]['password'])){
				$password = $res[$i]['password'];
			}else{
				$password = '';
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
					<input type="text" name="<?php echo 'username'.$i; ?>" id="<?php echo 'username'.$i; ?>" value="<?php echo $username;?>" style="width: 100px;" />
				</td>
				<td <?php if ($i==0) {echo $hcc;}?>>
					<input type="text" name="<?php echo 'password'.$i; ?>" id="<?php echo 'password'.$i; ?>" value="<?php echo $password;?>" style="width: 100px;"/>
				</td>
				<td <?php if ($i==0) {echo $hcc;}?>>
					<input type="text" name="<?php echo 'apn'.$i; ?>" id="<?php echo 'apn'.$i; ?>" value="<?php echo $apn;?>" style="width: 60px;"/>
				</td>
				<td <?php if ($i==0) {echo $hcc;}?>>
					<?php if ($i != 0) {?>
					<button type="submit" value="Modify" style="width:32px;height:32px;" onclick="document.getElementById('send').value='Modify';document.getElementById('sel_internet').value=<?php echo $i;?>">
						<img src="/images/save.png">
					</button>
					<?php }?>
				</td>
			</tr>
		<?php } ?>
		</table>
		
		<div id="newline"></div>
		
		<table id="float_btn" class="float_btn">
			<tr id="float_btn_tr" class="float_btn_tr">
				<td>
					<input type="hidden" name="send" id="send" value="Save" />
					<input type="submit" id="sendlabel" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check_batch();"/>
				</td>
				<td>
					<input type="button" value="<?php echo language('Cancel');?>" onclick="location.reload()" />
				</td>
				<td>
					<input type="button" value="<?php echo language('Batch');?>" onclick="check_batch();setValue();" />
				</td>
			</tr>
		</table>
		<table id="float_btn2" style="border:none;" class="float_btn2">
			<tr id="float_btn_tr2" class="float_btn_tr2">
				<td style="width:51px;">
					<input type="submit" id="float_button_1" class="float_short_button" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check_batch();"/>
				</td>
				<td style="width:51px;">
					<input type="button" id="float_button_2" class="float_short_button" value="<?php echo language('Cancel');?>" onclick="location.reload()" />
				</td>
				<td style="width:51px;">
					<input type="button" id="float_button_3" name="" class="float_short_button" value="<?php echo language('Batch');?>" onclick="check_batch();setValue();" />
				</td>
			</tr>
		</table>
	</form>
		
	<script>
		function selectall(checked){
			if(checked == true){
				$(":checkbox").attr("checked", true);
			}else{
				$(":checkbox").removeAttr("checked");
			}
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
		
		function setValue(){
			var username_auto = document.getElementById('username0').value;
			var password_auto = document.getElementById('password0').value;
			var apn_auto = document.getElementById('apn0').value;
			
			$(".get_chan").each(function(){
				var channel = $(this).val();
				if(document.getElementById('channel'+channel).checked == true){
					document.getElementById('username'+channel).value = username_auto;
					document.getElementById('password'+channel).value = password_auto;
					document.getElementById('apn'+channel).value = apn_auto;
				}
			});
		}
	</script>
	
<?php
}

function save_all_mms(){
	global $__GSM_SUM__;
	
	$aql = new aql();
	$file_path = '/etc/asterisk/gw/mms.conf';
	$hlock = lock_file($file_path);
	$aql->set('basedir','/etc/asterisk/gw');
	if(!$aql->open_config_file($file_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	
	$res = $aql->query("select * from mms.conf");
	
	if(!isset($res['general'])){
		$aql->assign_addsection('general','');
	}
	
	$download_type = $_POST['download_type'];
	
	$hour = sprintf("%02d",$_POST['hour']);
	$minute = sprintf("%02d",$_POST['minute']);
	$second = sprintf("%02d",$_POST['second']);
	
	$download_time = $hour.':'.$minute.':'.$second;
	
	if(isset($res['general']['download_type'])){
		$aql->assign_editkey('general','download_type',$download_type);
	}else{
		$aql->assign_append('general','download_type',$download_type);
	}
	
	if(isset($res['general']['download_time'])){
		$aql->assign_editkey('general','download_time',$download_time);
	}else{
		$aql->assign_append('general','download_time',$download_time);
	}
	
	for($i=0;$i<count($_POST['get_chan']);$i++){
		$chan = $_POST['get_chan'][$i];
		if(!isset($_POST['channel'.$chan])) continue;
		
		$username = $_POST['username'.$chan];
		$password = $_POST['password'.$chan];
		$apn = $_POST['apn'.$chan];
		
		if(!isset($res[$chan])){
			$aql->assign_addsection($chan,'');
		}
		
		if(isset($res[$chan]['username'])){
			$aql->assign_editkey($chan,'username',$username);
		}else{
			$aql->assign_append($chan,'username',$username);
		}
		
		if(isset($res[$chan]['password'])){
			$aql->assign_editkey($chan,'password',$password);
		}else{
			$aql->assign_append($chan,'password',$password);
		}
		
		if(isset($res[$chan]['apn'])){
			$aql->assign_editkey($chan,'apn',$apn);
		}else{
			$aql->assign_append($chan,'apn',$apn);
		}
	}
	
	if(!$aql->save_config_file('mms.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	
	unlock_file($hlock);
}
	
	
function save_one_mms(){
	$aql = new aql();
	
	$file_path = '/etc/asterisk/gw/mms.conf';
	$hlock = lock_file($file_path);
	$aql->set('basedir','/etc/asterisk/gw');
	if(!$aql->open_config_file($file_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return -1;
	}
	
	$res = $aql->query("select * from mms.conf");
	
	if(!isset($res['general'])){
		$aql->assign_addsection('general','');
	}
	
	$download_type = $_POST['download_type'];
	
	$hour = sprintf("%02d",$_POST['hour']);
	$minute = sprintf("%02d",$_POST['minute']);
	$second = sprintf("%02d",$_POST['second']);
	
	$download_time = $hour.':'.$minute.':'.$second;
	
	if(isset($res['general']['download_type'])){
		$aql->assign_editkey('general','download_type',$download_type);
	}else{
		$aql->assign_append('general','download_type',$download_type);
	}
	
	if(isset($res['general']['download_time'])){
		$aql->assign_editkey('general','download_time',$download_time);
	}else{
		$aql->assign_append('general','download_time',$download_time);
	}
	
	if($_POST['sel_internet']!=''){
		$i = $_POST['sel_internet'];
		
		$username = $_POST['username'.$i];
		$password = $_POST['password'.$i];
		$apn = $_POST['apn'.$i];
		
		if(!isset($res[$i])){
			$aql->assign_addsection($i,'');
		}
		
		if(isset($res[$i]['username'])){
			$aql->assign_editkey($i,'username',$username);
		}else{
			$aql->assign_append($i,'username',$username);
		}
		
		if(isset($res[$i]['password'])){
			$aql->assign_editkey($i,'password',$password);
		}else{
			$aql->assign_append($i,'password',$password);
		}
		
		if(isset($res[$i]['apn'])){
			$aql->assign_editkey($i,'apn',$apn);
		}else{
			$aql->assign_append($i,'apn',$apn);
		}
	}
	
	if(!$aql->save_config_file('mms.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}
	
	unlock_file($hlock);
}

if($_POST){
	if(isset($_POST['send']) && $_POST['send'] == 'Save'){
		save_all_mms();
	}else if($_POST['send'] == 'Modify'){
		save_one_mms();
	}
	wait_apply("exec","/etc/init.d/handle_mms.sh reload");
	show_mms();
}else{
	show_mms();
}

require("/www/cgi-bin/inc/boot.inc");
?>

<div id="to_top"></div>
<div id="float_btn1" class="sec_float_btn1">
</div>
<div  class="float_close" onclick="close_btn()" >
</div>