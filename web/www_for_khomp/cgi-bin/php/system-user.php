<?php
require_once("/www/cgi-bin/inc/head.inc");
require_once("/www/cgi-bin/inc/menu.inc");
require_once("/www/cgi-bin/inc/userdb.php");
?>

<?php
$db = new Users();

function show_user_list(){
	global $db;
?>
	<script type="text/javascript" src="/js/functions.js"></script>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
		<div class="content">
			<table class="table_show">
				<tr>
					<th class="nosort">
						<input type="checkbox" name="selall" onclick="selectAll(this.checked,'id[]')" />
					</th>
					<th><?php echo language('Username');?></th>
					<th><?php echo language('IP Number');?></th>
					<th><?php echo language('Create Time');?></th>
					<th><?php echo language('Role');?></th>
					<th><?php echo language('Action');?></th>
				</tr>
				
				<?php 
				$results = $db->get_all_user_info();
				while($info = $results->fetchArray()){
				?>
					<tr>
						<td width="03%">
							<?php if($info['super'] != 1){ ?>
							<input type="checkbox" name="id[]" value="<?php echo $info['id'];?>" />
							<?php } ?>
						</td>
						<td><a href="/cgi-bin/php/record.php?user_id=<?php echo $info['id'];?>&username=<?php echo $info['username'];?>" style="color:blue;" target="_blank"><?php echo $info['username'];?></td>
						<td><?php echo $info['ip_num']!=0 ? $info['ip_num'] : 'No Limit';?></td>
						<td><?php system("date -d @".$info['create_time']." +\"%Y-%m-%d %H:%M:%S\"");?></td>
						<td><?php echo ($info['super'] == 1) ? language('Super') : language('General User');?></td>
						<td width="09%" style="text-align:center;">
							<button type="button" value="Modify" style="width:32px;height:32px;margin-left:3px;cursor:pointer;padding:0;" 
								onclick="document.getElementById('send').value='Edit User';getPage('<?php echo $info['id']; ?>')">
								<img src="/images/edit.gif">
							</button>
							<?php if($info['super'] != 1){ ?>
							<button type="submit" value="Delete" style="width:32px;height:32px;cursor:pointer;margin-left:2px;padding:0;" 
								onclick="document.getElementById('send').value='Delete';return delete_click('<?php echo $info['id']; ?>')" >
								<img src="/images/delete.gif">
							</button>
							<?php } ?>
						</td>
					</tr>
				<?php } ?>
			</table>
		
			<br/>
			<input type="hidden" name="send" id="send" value="" />
			<input type="hidden" id="del_id" name="del_id" value="" />
			<input type="submit" value="<?php echo language("New User"); ?>" onclick="document.getElementById('send').value='New User';"/>
			<input type="submit" value="<?php echo language("Delete"); ?>" onclick="document.getElementById('send').value='Delete';delete_batch_click();" />
		</div>
	</form>
	
	<script>
	function getPage(id){
		window.location.href = '<?php echo get_self();?>?id='+id;
	}
	
	function delete_click(id){
		var ret = confirm("<?php echo language('Delete confirm','Are you sure to delete you selected ?');?>");

		if(ret) {
			document.getElementById('del_id').value = id;
			return true;
		}

		return false;
	}
	
	function delete_batch_click(){
		var ret = confirm("<?php echo language('Delete confirm','Are you sure to delete you selected ?');?>");
		
		if(ret) {
			return true;
		}

		return false;
	}
	</script>
<?php
}

function edit_user(){
	global $nav_lists;
	global $db;
	
	if(isset($_GET['id'])){//Edit
		$results = $db->get_one_user_by_id($_GET['id']);
		$info = $results->fetchArray();
		
		if($info['ip_num'] != 1 && $info['ip_num'] != 3 && $info['ip_num'] != 10){
			$custom_num = $info['ip_num'];
		}else{
			$custom_num = 0;
		}
		
		$auth = unserialize($info['auth']);
		
		//Super Administrator
		if($info['super'] == 1){
			$disabled = 'disabled';
			$super_checked = 'checked';
		}
	}else{//New
		$info = [];
		$info['username'] = '';
		$info['ip_num'] = 1;
		$custom_num = 0;
	}
	
	$all_res = $db->get_all_user_info();
	$js_arr = '[';
	while($all_info = $all_res->fetchArray()){
		if($info['username'] == $all_info['username']) continue;
		
		$js_arr .= '"'.$all_info['username'].'",';
	}
	$js_arr = rtrim($js_arr,",");
	$js_arr .= ']';
?>

	<script type="text/javascript" src="/js/check.js"></script>
	<form enctype="multipart/form-data" action="<?php echo get_self(); ?>" method="post">
		<div class="content">
			<span class="title">
				<?php echo language('User');?>
				
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<?php if(is_show_language_help('Username')){ ?>
								<b><?php echo language('Username');?>:</b><br/>
								<?php echo language('Username help',"User name must be unique");?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Password')){ ?>
								<b><?php echo language('Password');?>:</b><br/>
								<?php echo language('Password help',"Modify user information, do not fill in the password, the original password will not change.<br/>Passwords must satisfy the following rules:<br/>1. At least 8 symbols<br/>2. Contains at least one numeric symbol<br/>3. Contains at least one lowercase letter<br/>4. Include at least one capital letter");?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Confirm Password')){ ?>
								<b><?php echo language('Confirm Password');?>:</b><br/>
								<?php echo language('Confirm Password help',"Please input the same password as 'Password' above.");?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Number of logged-in IPS')){ ?>
								<b><?php echo language('Number of logged-in IPS');?>:</b><br/>
								<?php echo language('Number of logged-in IPS help',"Number of simultaneous online IP");?>
								
								<br/><br/>
							<?php } ?>
							
							<?php if(is_show_language_help('Auth')){ ?>
								<b><?php echo language('Auth');?>:</b><br/>
								<?php echo language('Auth help',"Page permissions assigned to the current user");?>
							<?php } ?>
						</div>
					</div>
				</div>
			</span>
			
			<div class="tab_item">
				<span>
					<?php echo language('Username');?>:
				</span>
				<div class="tab_item_right">
					<span id="cusername"></span>
					<input type="text" name="username" id="username" value="<?php echo $info['username'];?>" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Password');?>:
				</span>
				<div class="tab_item_right">
					<span id="cpassword"></span>
					<input type="password" name="password" id="password" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Confirm Password');?>:
				</span>
				<div class="tab_item_right">
					<span id="cpw2"></span>
					<input id="pw2" type="password" name="pw2" />
				</div>
			</div>
			
			<div class="tab_item">
				<span>
					<?php echo language('Number of logged-in IPS');?>:
				</span>
				<div class="tab_item_right">
					<input type="text" style="margin-left:15px;width:84px;display:none;" name="cus_ip_num" id="cus_ip_num" value="<?php echo $custom_num;?>" />
					<select name="ip_num" id="ip_num">
						<option value="1" <?php if($info['ip_num'] == 1) echo 'selected';?>>1</option>
						<option value="3" <?php if($info['ip_num'] == 3) echo 'selected';?>>3</option>
						<option value="10" <?php if($info['ip_num'] == 10) echo 'selected';?>>10</option>
						<option value="custom" <?php if($custom_num != 0) echo 'selected';?>><?php echo language('Custom');?></option>
						<option value="0" <?php if($info['ip_num'] == 0) echo 'selected';?>><?php echo language('No Limited');?></option>
					</select>
				</div>
			</div>
			
			<?php if($info['super'] != 1){ ?>
			<div class="tab_item">
				<span>
					<?php echo language('Auth');?>:
				</span>
				<div class="tab_item_right" style="margin-left:30px;">
					<table cellpadding="0" cellspacing="0" class="port_table">
						<tr>
							<td><input type="checkbox" id="select_all" <?php echo $disabled.' '.$super_checked;?> /><?php echo language('All');?></td>
						</tr>
						
						<?php 
						foreach($nav_lists as $key=>$val){
							echo "<tr><td>";
							echo "<input type='checkbox' name='model' class='auth_each' onclick='module_sel(this,\"".$key."\")' $disabled $super_checked />";
							echo strtoupper(language($key));
							echo "<input style='margin-left:15px;' type='checkbox' name='model_ov' class='auth_each_ov' onclick='module_sel_ov(this,\"".$key."\")' $disabled $super_checked />";
							echo language("Only View");
							echo "</td></tr>";
							
							echo "<tr style='display:block;margin:5px 0 10px 30px;'>";
							foreach($val as $row){
								if($row[0] == 'system-user.php' ||
									$row[0] == 'system-ssh.php' ||
									$row[0] == 'gsm-toolkit.php' ||
									$row[0] == 'module-update.php' ||
									$row[0] == 'iax-adv-settings.php' ||
									$row[0] == 'adv-astapi.php' ||
									$row[0] == 'adv-astcli.php' ||
									$row[0] == 'adv-astfileeditor.php' ||
									$row[0] == 'adv-internet.php' ||
									$row[0] == 'adv-cloud.php' ||
									$row[0] == 'log-iax.php' ||
									//$row[0] == 'log-cdr.php' ||
									$row[0] == 'log-statistics.php'
								)
								{
									continue;
								}
								
								$checked = '';
								for($i=0;$i<count($auth[$key]);$i++){
									if($auth[$key][$i] == $row[2]){
										$checked = 'checked';
										break;
									}
								}
								
								$view_checked = '';
								for($i=0;$i<count($auth['view_'.$key]);$i++){
									if($auth['view_'.$key][$i] == $row[2]){
										$view_checked = 'checked';
										break;
									}
								}
								
								echo "<td class='sms_port' style='border-right:1px solid #999;width:300px;margin-left:10px;'>";
								echo "<input type='checkbox' name='page[]' class='auth_each ".$key."' value='".$key."-".$row[2]."' ".$checked." $disabled $super_checked/>";
								echo language($row[1]);
								echo "<span style='float:right;'>";
								echo "<input style='margin-left:15px;' type='checkbox' name='view[]' class='view_each view_".$key."' value='view_".$key."-".$row[2]."' ".$view_checked." $disabled />";
								echo language("Only View");
								echo "</span>";
								echo '</td>';
							}
							echo "</tr>";
						}
						?>
						
					</table>
				</div>
				<div style="clear:both;"></div>
			</div>
			<?php } ?>
		</div>
		
		<div id="button_save">
			<button type="submit" onclick="document.getElementById('send').value='Save';return check();" ><?php echo language('Save');?></button>
		</div>
	
		<br/>
		
		<input type="hidden" name="send" id="send" value="" />
		<input type="hidden" name="id" id="id" value="<?php echo $_GET['id'];?>" />
		<input type="hidden" name="old_username" id="old_username" value="<?php echo $info['username'];?>" />
	</form>
	
	<script>
	function check(){
		var is_check = false;
		
		var username = document.getElementById('username').value;
		var password = document.getElementById('password').value;
		var pw2 = document.getElementById('pw2').value;
		var all_username = <?php echo $js_arr; ?>;
		var regex=/^(?=.*?[A-Za-z]+)(?=.*?[0-9]+)(?=.*?[A-Z]).*$/;
		
		$("#cusername").html("");
		if(username == ''){
			$("#username").focus();
			$("#cusername").html(con_str("<?php echo language('js check null', 'can not be null!')?>"));
			is_check = true;
		}
		
		for(var i=0;i<all_username.length;i++){
			if(all_username[i] == username){
				$("#username").focus();
				$("#cusername").html(con_str("<?php echo language("User name has been used");?>"));
				is_check = true;
			}
		}
		
		$("#password").html("");
		<?php if(!isset($_GET['id'])){ ?>
		if(password == ''){
			$("#password").focus();
			$("#cpassword").html(con_str("<?php echo language('js check null', 'can not be null!')?>"));
			is_check = true;
		}
		<?php } ?>
		
		if(password != ''){
			if(password.length < 8){
				$("#password").focus();
				$("#cpassword").html(con_str("<?php echo language("Password len tip", "The password cannot be less than 8 characters.");?>"));
				is_check = true;
			}
			
			var t = regex.test(password);
			if((t==false)){
				$("#password").focus();
				$("#cpassword").html(con_str("<?php echo language("password rule tip","Does not comply with the password rules.");?>"));
				is_check = true;
			}
			
			if(password !== pw2){
				document.getElementById("cpw2").innerHTML = con_str('<?php echo language('Confirm Password warning web','This password must match the password above.');?>');
				is_check = true;
			}
		}
		
		if(is_check){
			return false;
		}
		
		return true;
	}
	
	function module_sel(that,module_name){
		if($(that).prop("checked")){
			$("."+module_name).prop("checked",true);
		}else{
			$("."+module_name).removeProp("checked");
		}
	}
	
	function module_sel_ov(that,module_name){
		if($(that).prop("checked")){
			$(".view_"+module_name).prop("checked",true);
		}else{
			$(".view_"+module_name).removeProp("checked");
		}
	}
	
	$("#select_all").change(function(){
		if($(this).prop("checked")){
			$(".auth_each").prop("checked",true);
		}else{
			$(".auth_each").removeProp("checked");
		}
	});
	
	$("#ip_num").change(function(){
		if($(this).val() == 'custom'){
			$("#cus_ip_num").show();
		}else{
			$("#cus_ip_num").hide();
		}
	});
	
	$(function(){
		if($("#ip_num").val() == 'custom'){
			$("#cus_ip_num").show();
		}else{
			$("#cus_ip_num").hide();
		}
	});
	</script>
<?php
}

function save_user(){
	global $nav_lists;
	global $db;
	
	$username = $_POST['username'];
	$old_username = $_POST['old_username'];
	$password = trim($_POST['password']);
	
	//Check whether the username is used
	if(!check_username_repeat($db,$old_username, $username)){
		echo language("User name has been used");
		return false;
	}
	
	$ip_num = $_POST['ip_num'];
	if($ip_num == 'custom'){
		$ip_num = $_POST['cus_ip_num'];
	}
	
	$auth_json = [];
	$page_arr = $_POST['page'];
	$view_page_arr = $_POST['view'];
	
	foreach($nav_lists as $key=>$val){
		
		$auth_json += [$key=>[]];//Associative array
		
		for($i=0;$i<count($page_arr);$i++){
			$temp = explode('-',$page_arr[$i]);
			$module = $temp[0];
			
			if($module == $key){
				array_push($auth_json[$key], $temp[1]);
			}
		}
		
		//only view
		$view_key = 'view_'.$key;
		$auth_json += [$view_key=>[]];
		
		for($i=0;$i<count($view_page_arr);$i++){
			$temp = explode('-',$view_page_arr[$i]);
			$module = $temp[0];
			
			if($module == $view_key){
				array_push($auth_json[$view_key], $temp[1]);
			}
		}
	}
	
	$auth = serialize($auth_json);
	
	$id = $_POST['id'];
	
	$real_password = md5($password.'-'.$username);
	if($id == ''){//Add
		$db->insert_user($username,$real_password,$ip_num,$auth);
		
		save_user_record($db,"SYSTEM->User:Create User ".$username);
	}else{//Update
		$results = $db->get_one_user_by_id($id);
		$info = $results->fetchArray();
		if($password == ''){
			$real_password = $info['password'];
		}
		$db->update_user($id,$username,$real_password,$ip_num,$auth);
		
		save_user_record($db,"SYSTEM->User:Update User ".$old_username.' to '.$username);
	}
}

function delete_user(){
	global $db;
	
	if($_POST['del_id'] != ''){
		$id = $_POST['del_id'];
		$db->delete_user($id);
		
		save_user_record($db,"SYSTEM->User:Delete User ID=".$id);
	}else{
		foreach($_POST['id'] as $id){
			$db->delete_user($id);
			
			save_user_record($db,"SYSTEM->User:Delete User ID=".$id);
		}
	}
}

if($_POST){
	if($_POST['send'] == 'New User'){
		edit_user();
	}else if($_POST['send'] == 'Save'){
		save_user();
		show_user_list();
	}else if($_POST['send'] == 'Delete'){
		delete_user();
		show_user_list();
	}
}else if($_GET){
	if(isset($_GET['id'])){
		edit_user();
	}
}else{
	show_user_list();
}
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>