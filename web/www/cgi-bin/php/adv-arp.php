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
	$arp_arr = [];
	$product_type = get_product_type();
	if($product_type < 4){
		exec("busybox arp -a |grep -v \"incomplete\"| awk '{print $2 \" \" $4}'",$output);
	}else{
		exec("arp -a |grep -v \"incomplete\"| awk '{print $2 \" \" $4}'",$output);
	}
	
	for($i=0;$i<count($output);$i++){
		$item = explode(' ', $output[$i]);
		$t = rtrim($item[0],')');
		$t = ltrim($t,'(');
		$arp_arr[$t] = $item[1];
	}
?>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
		<input type="hidden" id="sel_arp_name" name="sel_arp_name" value="" />
	
		<table width="100%" id="tab_routings" class="tdrag">
			<tr>
				<th style="width:03%" class="nosort">
					<input type="checkbox" name="selall" onclick="selectAll(this.checked,'arp[]')" />
				</th>
				<th><?php echo language('IP');?></th>
				<th><?php echo language('MAC');?></th>
				<th width="07%"><?php echo language('Actions');?></th>
			</tr>
			
			<?php foreach($arp_arr as $key => $val){?>
			<tr bgcolor="#E8EFF7">
			
				<td width="03%"> 
					<input type="checkbox" name="arp[]" value="<?php echo $key;?>"/>
				</td>
				<td>
					<?php echo $key;?>
				</td>
				<td>
					<?php echo $val;?>
				</td>
				<td width="07%">
					<button type="button" value="Modify" style="width:32px;height:32px;margin-left:3px;cursor:pointer;" onclick="getPage('<?php echo $key ?>')">
						<img src="/images/edit.gif">
					</button>
					<button type="submit" value="Delete" style="width:32px;height:32px;cursor:pointer;margin-left:2px;" 
						onclick="document.getElementById('send').value='Delete';return delete_click('<?php echo $key ?>')" >
						<img src="/images/delete.gif">
					</button>
				</td>
				
			</tr>
			<?php }?>
		</table>
		<br>
		<input type="hidden" name="send" id="send" value="" />
		<input type="submit" value="<?php echo language('New ARP');?>" onclick="document.getElementById('send').value='New ARP';" />
		<input type="submit" value="<?php echo language('Delete');?>" onclick="document.getElementById('send').value='Delete';return batch_delete_click();" />
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
	$ip = $_GET['ip'];
	
	if(isset($ip)){
		$product_type = get_product_type();
		if($product_type < 4){
			exec("busybox arp -a |grep \"$ip\"| awk '{print $2 \" \" $4}'", $output);
		}else{
			exec("arp -a |grep \"$ip\"| awk '{print $2 \" \" $4}'", $output);
		}
		
		$temp = explode(') ', $output[0]);
		$mac = $temp[1];
	}else{
		$mac = '';
	}
?>

	<form enctype="multipart/form-data" action="<?php echo get_self(); ?>" method="post">

		<div id="tab">
			<li class="tb_unfold" onclick="lud(this,'tab_main')" id="tab_main_li">&nbsp;</li>
			<li class="tbg_fold" onclick="lud(this,'tab_main')"><?php echo language('ARP');?></li>
			<li class="tb2_fold" onclick="lud(this,'tab_main')">&nbsp;</li>
			<li class="tb_end">&nbsp;</li>
		</div>

		<div id="tab_main" style="display:block">
			<table width="100%" class="tedit" >
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('IP');?>:
							<span class="showhelp">
							<?php echo language('IP help',"");
							?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" name="ip" id="ip" value="<?php echo $ip;?>" />
						<span id="cip" style="color:red;"></span>
					</td>
				</tr>
				
				<tr>
					<th>
						<div class="helptooltips">
							<?php echo language('MAC');?>:
							<span class="showhelp">
							<?php echo language('MAC help',"");
							?>
							</span>
						</div>
					</th>
					<td >
						<input type="text" name="mac" id="mac" value="<?php echo $mac;?>" />
						<span id="cmac" style="color:red;"></span>
					</td>
				</tr>
			</table>
		</div>
		
		<br/>
		
		<input type="hidden" name="send" id="send" value="" />
		<input type="submit" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>
		<input type="button" value="<?php echo language('Cancel');?>" onclick="window.location.href='<?php echo get_self();?>'" />
	</form>
	
	<script>
	function check(){
		var ip = document.getElementById('ip').value;
		var mac = document.getElementById('mac').value;
		
		if(!check_ip(ip)){
			$("#cip").html("<?php echo language('js check ip','Please input a valid IP address');?>");
			return false;
		}
		
		if(!check_mac(mac)){
			$("#cmac").html("<?php echo language('js check mac','Please input a valid MAC address');?>");
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
	$mac = $_POST['mac'];
	
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
	
	if(isset($_POST['sel_arp_name']) && $_POST['sel_arp_name']){
		$ip = $_POST['sel_arp_name'];
		$aql->assign_delsection($ip);
		exec("arp -d $ip");
	}else{
		foreach($_POST['arp'] as $name){
			$aql->assign_delsection($name);
			exec("arp -d $name");
		}
	}
	
	$aql->save_config_file('arp.conf');
	unlock_file($hlock);
	wait_apply("exec", "");
}

if($_POST){
	if($_POST['send'] == 'New ARP'){
		show_arp();
	}else if($_POST['send'] == 'Save'){
		save_arp();
		show_arp_list();
	}else if($_POST['send'] == 'Delete'){
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