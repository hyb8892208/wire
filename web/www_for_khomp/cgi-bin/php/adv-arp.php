<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
require_once("/www/cgi-bin/inc/aql.php");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
?>
<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<?php 
function show_arp_list(){
	global $only_view;
	
	$arp_arr = [];
	exec("arp -a |grep -v \"incomplete\"| awk '{print $2 \" \" $4}'",$output);
	
	for($i=0;$i<count($output);$i++){
		$item = explode(' ', $output[$i]);
		$t = rtrim($item[0],')');
		$t = ltrim($t,'(');
		$arp_arr[$t] = $item[1];
	}
?>
<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<input type="hidden" id="sel_arp_name" name="sel_arp_name" value="" />
	
	<div class="content">
		<table class="table_show">
			<tr>
				<th style="width:03%" >
					<input type="checkbox" name="selall" onclick="selectAll(this.checked,'arp[]')" />
				</th>
				<th><?php echo language('IP');?></th>
				<th><?php echo language('MAC');?></th>
				<th width="07%"><?php echo language('Actions');?></th>
			</tr>
			
			<?php foreach($arp_arr as $key => $val){?>
			<tr>
			
				<td width="03%"> 
					<input type="checkbox" name="arp[]" value="<?php echo $key;?>"/>
				</td>
				<td>
					<?php echo $key;?>
				</td>
				<td>
					<?php echo $val;?>
				</td>
				<td width="09%">
					<button type="button" value="Modify" style="width:32px;height:32px;margin-left:3px;cursor:pointer;padding:0;" onclick="getPage('<?php echo $key ?>')">
						<img src="/images/edit.gif">
					</button>
					
					<?php if(!$only_view){ ?>
					<button type="submit" value="Delete" style="width:32px;height:32px;cursor:pointer;margin-left:2px;padding:0;" 
						onclick="document.getElementById('send').value='Delete';return delete_click('<?php echo $key ?>')" >
						<img src="/images/delete.gif">
					</button>
					<?php } ?>
				</td>
				
			</tr>
			<?php }?>
		</table>
	</div>
	
	<input type="hidden" name="send" id="send" value="" />
	
	<div id="button_save">
		<button type="submit" onclick="document.getElementById('send').value='New ARP';" ><?php echo language('New ARP');?></button>
		
		<?php if(!$only_view){ ?>
		<button type="submit" onclick="document.getElementById('send').value='Delete';return batch_delete_click();" ><?php echo language('Delete');?></button>
		<?php } ?>
		
	</div>
</form>

	<script>
	function getPage(value){
		window.location.href = '<?php echo get_self();?>?ip='+value;
	}
	
	function delete_click(value1){
		var ret = confirm("<?php echo language('Delete confirm','Are you sure to delete you selected ?');?>");

		if(ret) {
			document.getElementById('sel_arp_name').value = value1;
			return true;
		}

		return false;
	}
	
	function batch_delete_click(){
		var ret = confirm("<?php echo language('Delete confirm','Are you sure to delete you selected ?');?>");
		if(ret){
			return true;
		}
		return false;
	}
	</script>
	
<?php }?>

<?php 
function show_arp(){
	global $only_view;
	
	$ip = $_GET['ip'];
	
	if(isset($ip)){
		exec("arp -a |grep \"$ip\"| awk '{print $2 \" \" $4}'", $output);
		$temp = explode(') ', $output[0]);
		$mac = $temp[1];
		
		$mac_temp = explode(":", $mac);
		
		$mac1 = $mac_temp[0];
		$mac2 = $mac_temp[1];
		$mac3 = $mac_temp[2];
		$mac4 = $mac_temp[3];
		$mac5 = $mac_temp[4];
		$mac6 = $mac_temp[5];
	}else{
		$mac1 = '';
		$mac2 = '';
		$mac3 = '';
		$mac4 = '';
		$mac5 = '';
		$mac6 = '';
	}
?>

<form enctype="multipart/form-data" action="<?php echo get_self(); ?>" method="post">
	
	<div class="content">
		<span class="title">
			<?php echo language('ARP');?>
			
			<div class="tip_main">
				<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
				<div class="tip_help">
					<i class="top" ></i>
				
					<div class="tip_content">
						<?php if(is_show_language_help('IP')){ ?>
							<b><?php echo language('IP');?>:</b><br/>
							<?php echo language('IP help',"");?>
						
							<br/><br/>
						<?php } ?>
						
						<?php if(is_show_language_help('MAC')){ ?>
							<b><?php echo language('MAC');?>:</b><br/>
							<?php echo language('MAC help',"");?>
						<?php } ?>
					</div>
				</div>
			</div>
		</span>
		
		<div class="tab_item">
			<span>
				<?php echo language('IP');?>:
			</span>
			<div class="tab_item_right">
				<span id="cip" style="color:red;"></span>
				<input type="text" name="ip" id="ip" value="<?php echo $ip;?>" />
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('MAC');?>:
			</span>
			<div class="tab_item_right">
				<span id="cmac" style="color:red;"></span>
				<!-- <input type="text" name="mac" id="mac" value="<?php echo $mac;?>" /> -->
				<input type="text" name="mac1" id="mac1" value="<?php echo $mac1;?>" style="width:28px;" /> :
				<input type="text" name="mac2" id="mac2" value="<?php echo $mac2;?>" style="width:28px;" /> :
				<input type="text" name="mac3" id="mac3" value="<?php echo $mac3;?>" style="width:28px;" /> :
				<input type="text" name="mac4" id="mac4" value="<?php echo $mac4;?>" style="width:28px;" /> :
				<input type="text" name="mac5" id="mac5" value="<?php echo $mac5;?>" style="width:28px;" /> :
				<input type="text" name="mac6" id="mac6" value="<?php echo $mac6;?>" style="width:28px;" />
			</div>
		</div>
	</div>
		
	<input type="hidden" name="send" id="send" value="" />
	
	<div id="button_save">
	
		<?php if(!$only_view){ ?>
		<button type="submit" onclick="document.getElementById('send').value='Save';return check();"><?php echo language('Save');?></button>
		<?php } ?>
		
		<button type="button" onclick="window.location.href='<?php echo get_self();?>'" ><?php echo language('Cancel');?></button>
	</div>
</form>
	
	<script>
	function check(){
		var is_check = false;
		
		var ip = document.getElementById('ip').value;
		var mac1 = document.getElementById('mac1').value;
		var mac2 = document.getElementById('mac2').value;
		var mac3 = document.getElementById('mac3').value;
		var mac4 = document.getElementById('mac4').value;
		var mac5 = document.getElementById('mac5').value;
		var mac6 = document.getElementById('mac6').value;
		
		var mac = mac1 + ':' + mac2 + ':' + mac3 + ':' + mac4 + ':' + mac5 + ':' + mac6;
		
		if(!check_ip(ip)){
			$("#cip").html("<?php echo language('js check ip','Please input a valid IP address');?>");
			is_check = true;
		}
		
		if(!check_mac(mac)){
			$("#cmac").html("<?php echo language('js check mac','Please input a valid MAC address');?>");
			is_check = true;
		}
		
		if(is_check){
			return false;
		}
		
		return true;
	}
	</script>
<?php }

function save_arp(){
	$arp_file = "/etc/asterisk/gw/arp.conf";
	if(!file_exists($arp_file)){
		exec("touch $arp_file");
	}
	$hlock = lock_file($arp_file);
	$aql = new aql();
	$aql->set('basedir', '/etc/asterisk/gw');
	if(!$aql->open_config_file($arp_file)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	
	$ip = $_POST['ip'];
	$mac1 = $_POST['mac1'];
	$mac2 = $_POST['mac2'];
	$mac3 = $_POST['mac3'];
	$mac4 = $_POST['mac4'];
	$mac5 = $_POST['mac5'];
	$mac6 = $_POST['mac6'];
	
	$mac = $mac1.':'.$mac2.':'.$mac3.':'.$mac4.':'.$mac5.':'.$mac6;
	
	$exist_array = $aql->query("select * from arp.conf");
	if(!isset($exist_array[$ip])){
		$aql->assign_addsection($ip,'');
	}
	
	if(isset($exist_array[$ip]['ip'])){
		//$aql->assign_editkey($ip,'ip',$ip.' '.$mac);
		$aql->assign_editkey($ip,'ip',$ip);
	}else{
		//$aql->assign_append($ip,'ip',$ip.' '.$mac);
		$aql->assign_append($ip,'ip',$ip);
	}


	if(isset($exist_array[$ip]['mac'])){
		//$aql->assign_editkey($ip,'ip',$ip.' '.$mac);
		$aql->assign_editkey($ip,'mac',$mac);
	}else{
		//$aql->assign_append($ip,'ip',$ip.' '.$mac);
		$aql->assign_append($ip,'mac',$mac);
	}
	
	if(!$aql->save_config_file('arp.conf')){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	unlock_file($hlock);
	
	save_user_record("","ADVANCED->ARP->:Save,ip=".$ip);
	
	exec("arp -s $ip $mac");
	wait_apply("exec", "");
}

function delete_arp(){
	$arp_file = "/etc/asterisk/gw/arp.conf";
	if(!file_exists($arp_file)){
		exec("touch $arp_file");
	}
	$hlock = lock_file($arp_file);
	$aql = new aql();
	$aql->set('basedir', '/etc/asterisk/gw');
	if(!$aql->open_config_file($arp_file)){
		echo $aql->get_error();
		unlock_file($hlock);
		return;
	}
	
	$temp_ip = '';
	if(isset($_POST['sel_arp_name']) && $_POST['sel_arp_name']){
		$ip = $_POST['sel_arp_name'];
		$aql->assign_delsection($ip);
		exec("arp -d $ip");
		
		$temp_ip = $ip;
	}else{
		foreach($_POST['arp'] as $name){
			$aql->assign_delsection($name);
			exec("arp -d $name");
			
			$temp_ip .= $name.',';
		}
	}
	
	$aql->save_config_file('arp.conf');
	unlock_file($hlock);
	
	save_user_record("","ADVANCED->ARP->:Delete,ip=".$temp_ip);
	
	wait_apply("exec", "");
}

if($_POST){
	if($_POST['send'] == 'New ARP'){
		show_arp();
	}else if($_POST['send'] == 'Save'){
		if($only_view){
			return false;
		}
		
		save_arp();
		show_arp_list();
	}else if($_POST['send'] == 'Delete'){
		if($only_view){
			return false;
		}
		
		delete_arp();
		show_arp_list();
	}
}else if($_GET){
	if(isset($_GET['ip'])){
		show_arp();
	}
}else{
	show_arp_list();
}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>