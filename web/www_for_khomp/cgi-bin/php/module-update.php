<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");

function get_update_module_type($channel){
	exec("grep -rn ^Revision /tmp/gsm/$channel", $temp);
	$module_select = explode(": ", $temp[0]);
	$module_name = $module_select[count($module_select)-1];
	
	$port_real_type = '';
	if(strstr($module_name, 'UC15') ||
		strstr($module_name, 'UC15A') ||
		strstr($module_name, 'UC15E') ||
		strstr($module_name, 'UC15T') ||
		strstr($module_name, 'Quectel_UC15')){
		$port_real_type = 'umts';
	}else if(strstr($module_name, 'Quectel_M35') || 
		strstr($module_name, 'M35') || 
		strstr($module_name, 'sim840')){
		$port_real_type = 'M35';
	}else if(strstr($module_name, 'Quectel_M26') ||
		strstr($module_name, 'M26')){
		$port_real_type = 'M26';
	}else if(strstr($module_name, 'SIMCOM_SIM6320C') || strstr($module_name, 'SIM6320C')){
		$port_real_type = 'cdma';
	}else if(strstr($module_name, 'EC20CE')){
		$port_real_type = 'EC20CE';
	}else if(strstr($module_name, 'EC25EF')){
		$port_real_type = 'EC25EF';
	}else if(strstr($module_name, 'EC25AUTL')){
		$port_real_type = 'EC25AUTL';
	}else if(strstr($module_name, 'EC25AUT')){
		$port_real_type = 'EC25AUT';
	}else if(strstr($module_name, 'EC25AU')){
		$port_real_type = 'EC25AU';
	}else if(strstr($module_name, 'EC25A')){
		$port_real_type = 'EC25A';
	}else if(strstr($module_name, 'EC25V')){
		$port_real_type = 'EC25V';
	}else if(strstr($module_name, 'EC25J')){
		$port_real_type = 'EC25J';
	}
	
	return $port_real_type;
}

function get_update_module_model($channel){
	exec("grep -rn ^Revision /tmp/gsm/$channel", $temp);
	$module_select = explode(": ", $temp[0]);
	
	$module_name = $module_select[count($module_select)-1];
	
	$module_temp = explode("R", $module_name);
	$module_model = trim($module_temp[0]);
	
	return $module_model;
}
?>

<!-- module update begin -->
<form id="manform" enctype="multipart/form-data" action="<?php echo get_self(); ?>" method="post">

	<div class="content">
		<span class="title"><?php echo language('Module Update');?></span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Port');?>:
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<b><?php echo language('Port');?>:</b><br/>
							<?php echo language('Port help','Select the module port that needs to be updated.');?>
						</div>
					</div>
				</div>
			</span>
			<div class="tab_item_right" style="float:left;">
				<table cellpadding="0" cellspacing="0" class="port_table">
					<?php 
					$j = 0;
					for($i=1;$i<=$__GSM_SUM__;$i++){
						$port_name = get_gsm_name_by_channel($i);
						$port_name_arr = explode("-", $port_name, 2);
						$port_type = $port_name_arr[0];
						if($port_type == "null") continue;
						
						$port_model = get_update_module_model($i);

						echo "<td class='module_port $port_model' ><input type='checkbox' name='spans[1][$i]' value='$i' class='port' data-model='$port_model'>";
						echo $port_name;
						echo '</td>';
						$j++;
					}
					?>
					<tr style="border:none;">
						<td style="border:none;">
							<input id="select_all" type="checkbox" >
							<?php echo language('All');?>
							<span style="float:right;margin-right:169px;"><input type='checkbox' id="show_all_ports" ><?php echo language('Show All');?></span>
						</td>
					</tr>
				</table>
			</div>
			<div class="clear"></div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Module Update file');?>:
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<b><?php echo language('Module Update file');?>:</b><br/>
							<?php echo language('Module Update file help', 'Module Update file');?>
						</div>
					</div>
				</div>
			</span>
			<div class="tab_item_right">
				<input type="file" name="update_module_file" id="update_module_file" onchange="return checkFileChange();"/>
				
				<?php if(!$only_view){ ?>
				<input type="button" value="<?php echo language('Update');?>" onclick="document.getElementById('send').value='Module Update';return submit_check();" style="float:right;margin-right:10px;"/>
				<?php } ?>
			</div>
		</div>
	</div>

	<div class="comfirm_box" id="module_confirm_box">
		<div class="comfirm_content">
			<p style="font-weight:bold;margin-bottom:0;"><?php echo language("Notice");?>:</p>
			<p style="color:red;font-size:14px;margin-top:0;">
				<?php echo language('Module Update alert', 'Please note the following points,if it is violated,it may cause the module upgrade to fail!');?>
			</p>
			<hr />
			<div>
				<p style="color:blue;"><?php echo language('Module Update note1', '1、Module upgrade will take a long time.');?></p>
				<p style="color:blue;"><?php echo language('Module Update note2', '2、System cannot be powered off during module update.');?></p>
				<p style="color:blue;"><?php echo language('Module Update note3', '3、Do not reboot system or asterisk during module update.');?></p>
				<p style="color:blue;"><?php echo language('Module Update note4', '4、Do not refresh this page during module update.');?></p>
				<p style="color:blue;"><?php echo language('Module Update note5', '5、Do not perform other operations on this system during module update.');?></p>
			</div>
			<div class="comfirm_button">
			
				<?php if(!$only_view){ ?>
				<button type="submit" id="notice_confirm"><?php echo language('Confirm');?></button>
				<?php } ?>
				
				<button type="button" style="margin-left:100px;" id="notice_cancel"><?php echo language('Cancel');?></button>
			</div>
		</div>
	</div>
	
	<input type="hidden" name="send" id="send" value="" />
<!-- module update end -->

<script>
function checkFileChange(){
	var file = document.getElementById('update_module_file');
	var name = file.files[0].name;
	var filesize = file.files[0].size;

	if(filesize > 1024*1024*80){
		alert("<?php echo language('System Update filesize alert','Uploaded max file is 80M!');?>");
		return false;
	}
	
	$(".module_port").hide();
	$(".port").each(function(){
		var module_model = $(this).attr("data-model");
		var file_temp = name.split("R");
		var file_model = file_temp[0];
		
		if(module_model == file_model){
			$(this).parent().show();
		}else{
			$(this).removeAttr("checked");
		}
	});
	
	return true;
}

function submit_check(){
	var file = document.getElementById('update_module_file');
	if(file.files[0] == undefined){
		alert("<?php echo language('Select File alert','Please select your file first!');?>");
		return false;
	}
	
	var name = file.files[0].name;
	
	if(name.indexOf('M35') != -1 || name.indexOf('M26') != -1 || name.indexOf('EC20') != -1 || name.indexOf('EC25') != -1 || name.indexOf('UC15') != -1 || name.indexOf('SIM6320C') != -1){
		if((name.indexOf('M35') != -1 || name.indexOf('M26') != -1) && name.indexOf('.bin') == -1){
			alert("<?php echo language('bin File help','Wrong file type. Please upload .bin file.');?>");
			return false;
		}else if(name.indexOf('EC20') != -1 && name.indexOf('.zip') == -1){
			alert("<?php echo language('zip File help','Wrong file type. Please upload .zip file.');?>");
			return false;
		}else if(name.indexOf('EC25') != -1 && name.indexOf('.zip') == -1){
			alert("<?php echo language('zip File help','Wrong file type. Please upload .zip file.');?>");
			return false;
		}else if(name.indexOf('UC15') != -1 && name.indexOf('.zip') == -1){
			alert("<?php echo language('zip File help','Wrong file type. Please upload .zip file.');?>");
			return false;
		}else if(name.indexOf('SIM6320C') != -1 && name.indexOf('.MLD') == -1){
			alert("<?php echo language('MLD File help','Wrong file type. Please upload .MLD file.');?>");
			return false;
		}
	}else{
		alert("<?php echo language("error File help","The file format is not correct. Please re select the file!");?>");
		return false;
	}
	
	var flag = 0;
	$(".port").each(function(){
		if($(this).attr('checked') == 'checked'){
			flag = 1;
		}
	});
	if(flag == 0){
		alert("<?php echo language('Select port alert');?>");
		return false;
	}
	
	$("#module_confirm_box").show();
	module_confirm();
}

function module_confirm(){
	$("#notice_confirm").click(function(){
		$("#module_confirm_box").hide();
	});
	
	$("#notice_cancel").click(function(){
		$("#module_confirm_box").hide();
	});
}

function select_all(){
	$("#select_all").click(function(){
		if($(this).attr("checked") == "checked"){
			$(".port").each(function(){
				if($(this).parent().css("display") != 'none'){
					$(this).attr("checked","checked");
				}
			});
		}else{
			$(".port").removeAttr("checked");
		}
	});
}

$(function(){
	select_all();
});

//show all ports
var select_port = [];
$("#show_all_ports").click(function(){
	if($(this).attr('checked') == 'checked'){
		$(".module_port").each(function(){
			if($(this).css('display') == 'none'){
				select_port.push($(this).attr('name'));
			}else{
				select_port.push('hide');
			}
		});
		$(".module_port").show();
	}else{
		var i = 0;
		$(".module_port").each(function(){
			if(select_port[i] != 'hide'){
				$(this).hide();
				$(this).children('.port').removeAttr('checked');
			}
			i++;
		});
		select_port = [];
	}
});
</script>

<?php 
function updating($module_sel){
?>
	<div class="modal fade in" id="modal-51" style="top:300px;">
		<div class="modal-dialog">
			<div class="modal-content">
			
				<div class="modal-header">
					<button type="button" class="close update_click_close">x</button>
					<h4 class="modal-title">
						<?php echo language('Module Updating');?><br>
					</h4>
				</div>
				
				<div class="modal-body" style="height:300px;overflow:auto;"></div>
				
			</div>
		</div>
	</div>
	<input type="hidden" id="filename" value="<?php if(isset($_FILES['update_module_file']['name'])) echo $_FILES['update_module_file']['name'];?>" />
	<div class="modal-backdrop fade_in"></div>
	
	<script>
	var para = {
		flag : '',
		req_time : [],//Every channel request times
		percent_time : [],//percent time no change in 5min
		old_percent_val : [],
		cdma_upload_ok : []
	};
	
	function request(chn){
		var percentage_timeout = 180;//3min
		
		var module_type = get_module_type();
		
		/**
		 * Update next one
		 */
		var update_next = function(){
			$(".err_res").each(function(){
				var err_flag = $(this).hasClass('err_flag');
				var suc_flag = $(this).hasClass('suc_flag');
				var upload_flag = $(this).hasClass('upload_flag');
				if(!err_flag && !suc_flag && !upload_flag){
					var channel = $(this).siblings('.progress').children('.chn_val').val();
					update(channel);
					return false;
				}
			});
		}
		
		/**
		 * Module updata error tip
		 */
		var module_error_tip = function(){
			$(".err_res_"+chn).addClass("err_flag");//add error flag
			if(module_type == 'gsm'){
				$(".err_res_"+chn).html("<span onclick='module_restart(this);' class='module_restart' >"+
									"<input type='hidden' value='"+chn+"' /><?php echo language("Restart update");?></span>"+
									"<?php echo language("Update error", "Failure to upgrade, an unknown error.");?>"
				);
			}else{
				$(".err_res_"+chn).html("<?php echo language("Update error", "Failure to upgrade, an unknown error.");?>");
				update_next();
			}
			monitor();
		}
		
		/**
		 * Percentage timeout
		 * The percentage time 5 minutes is not updated and time out.
		 */
		var cal_percent_time = function(data){
			if(para.percent_time[chn] == undefined) para.percent_time[chn] = 1;
			if(para.old_percent_val[chn] == undefined) para.old_percent_val[chn] = 0;
			if(para.old_percent_val[chn] == parseInt(data)){
				if(para.percent_time[chn] <= percentage_timeout){
					para.percent_time[chn]++;
				}else{
					module_error_tip();
					return false;
				}
			}else{
				para.percent_time[chn] = 0;
				para.old_percent_val[chn] = parseInt(data);
			}
			return true;
		}
		
		/**
		 * Display percentage progress.
		 */
		var show_percent_num = function(data){
			$(".module_"+chn).css("width", data+'%');
			$(".module_"+chn).text(data+'%');
			$(".err_res_"+chn).text("<?php echo language("Updating");?>.....");
			setTimeout(function(){
			  request(chn);
			}, 1000);
		}
		
		/**
		 * The next update of the CDMA module.
		 */
		var next_cdma_starts = function(){
			  $(".err_res_"+chn).addClass("upload_flag");//add upload success flag
			  
			  if(para.cdma_upload_ok[chn] == undefined) para.cdma_upload_ok[chn] = 1;
			  if(para.cdma_upload_ok[chn] == 1){
				  update_next();
				  para.cdma_upload_ok[chn] = 0;
			  }
			  return true;
		}
		
		/**
		 * CDMA upload ok timeout
		 * 5 minutes is not updated and timeout
		 */
		 var cal_cdma_upload_time = function(){
			 if(para.old_percent_val[chn] != 'upload ok'){
				  para.old_percent_val[chn] = 'upload ok';
				  para.percent_time[chn] = 1;
			  }
			  if(para.percent_time[chn] <= percentage_timeout){
				  para.percent_time[chn]++;
			  }else{
				  module_error_tip();
				  return false;
			  }
			  setTimeout(function(){
				  request(chn);
			  }, 1000);
			  return true;
		 }
		 
		 /**
		  * Update success
		  */
		 var update_success = function(){
			  $(".module_"+chn).css("width", '100%');
			  $(".module_"+chn).text('100%');
			  $(".module_"+chn).css("background-color", "#69DB4B");
			  $(".err_res_"+chn).text("<?php echo language("Update success");?>!");
			  $(".err_res_"+chn).addClass("suc_flag");//add success flag
			  
			  if(module_type == 'lte' || module_type == 'umts'){
				   update_next();
			  }
			  monitor();
			  return true;
		 }
		 
		 /**
		  * chn_x file not exist
		  * request 60s
		  */
		  var start_error = function(){
			  if(para.req_time[chn] == undefined) para.req_time[chn] = 1;
			  if(para.req_time[chn]<60){
				  setTimeout(function(){
					  request(chn);
					  para.req_time[chn]++;
				  }, 1000);
			  }else{
				  module_error_tip();
			  }
		  }
		
		$.ajax({
			  type: "GET",
			  url: "../../chn_"+chn,
			  datetype: "text",
			  success: function(data){
				  //updating
   				  if(data.indexOf('err') == -1 && data.indexOf('success') == -1 && data.indexOf('upload ok') == -1){
					  if(data == '') data = '0';
					  if(module_type == 'cdma'){
						  if(data!=0) --data;
					  }
					  
					  if(!cal_percent_time(data)){
						  return false;
					  }
					  
					  show_percent_num(data);
				  }
				  
				  //next one just for cdma
				  data = data + '';
				  if(data.indexOf('upload ok') != -1){
					  if(!next_cdma_starts()){
						  return false;
					  }
					  
					  if(!cal_cdma_upload_time()){
						  return false;
					  }
				  }
				  
				  //success
				  if(data.indexOf('success') != -1){
					  if(!update_success()){
						  return false;
					  }
				  }
				  
				  //error
				  if(data.indexOf('err') != -1){
					  module_error_tip();
				  }
			  },
			  error: function(data){
				  start_error();
			  }
		});
	}
	
	function update(chn){
		var filename = document.getElementById('filename').value;
		var module_type = get_module_type();
		para.req_time[chn] = 1;
		$.ajax({
			type: "POST",
			url: "ajax_server.php?type=module_update&update=",
			datetype: "text",
			data: {
				channel: chn,
				filename: filename,
				module_type: module_type
			},
			success: function(data){
			}
		});
		
		setTimeout(function(){
			request(chn);
		},3000);
	}
	
	function module_update_exit(){
		var module_sel = [];
		<?php for($i=0;$i<count($module_sel);$i++){?>
				module_sel.push(<?php echo $module_sel[$i];?>);
		<?php }?>
		
		$.ajax({
			type: "POST",
			url: "ajax_server.php?type=module_update",
			datetype: "json",
			data: {
				filename: JSON.stringify(module_sel)
			},
			success: function(data){
			}
		});
	}
	
	function update_complete(){
		var complete_flag = 1;
		$(".err_res").each(function(){
			var err_flag = $(this).hasClass('err_flag');
			var suc_flag = $(this).hasClass('suc_flag');
			var upload_flag = $(this).hasClass('upload_flag');
			if(!err_flag && !suc_flag && !upload_flag){
				complete_flag = 0;
				return false;
			}
		});
		return complete_flag;
	}
	
	function monitor(){
		para.flag = 1;
		$(".err_res").each(function(){
			var suc_flag = $(this).hasClass('suc_flag');
			if(!suc_flag){
				para.flag = 0;
			}
		});
		
		if(update_complete()){
			$(".err_res").each(function(){
				if($(this).hasClass('err_flag')){
					var chn = $(this).siblings('.progress').children('.chn_val').val();
					$(".err_res_"+chn).html("<span onclick='module_restart(this);' class='module_restart' >"+
										"<input type='hidden' value='"+chn+"' /><?php echo language("Restart update");?></span>"+
										"<?php echo language("Update error", "Failure to upgrade, an unknown error.");?>"
					);
				}
			});
		}
		
		if(para.flag){
			module_update_exit();
		}
	}
	
	function get_module_type(){
		var module_sel = [];
		var module_name = '';
		<?php for($i=0;$i<count($module_sel);$i++){
				$module_name = get_gsm_name_by_channel($module_sel[$i]);
		?>
				module_name = "<?php echo $module_name;?>";
				module_sel.push(<?php echo $module_sel[$i];?>);
		<?php }?>
		
		if(module_name.indexOf('gsm') != -1){
			return 'gsm';
		}else if(module_name.indexOf('lte') != -1){
			return 'lte';
		}else if(module_name.indexOf('cdma') != -1){
			return 'cdma';
		}else if(module_name.indexOf('umts') != -1){
			return 'umts';
		}
		
		return '';
	}
	
	function result(){
		var module_sel = [];
		var module_name = '';
		<?php for($i=0;$i<count($module_sel);$i++){
				$module_name = get_gsm_name_by_channel($module_sel[$i]);
		?>
				module_name = "<?php echo $module_name;?>";
				module_sel.push(<?php echo $module_sel[$i];?>);
		<?php }?>
		
		if(module_name.indexOf('gsm') != -1){
			for(var i=0;i<module_sel.length;i++){
				$(".modal-body").append("<div class='form-group' style='width:100%;'>module "+module_sel[i]+"<span class='err_res err_res_"+module_sel[i]+"'><?php echo language("Update waiting");?>.....</span><div class='progress'><div class='sub_progress module_"+module_sel[i]+"'></div></div></div>");
				request(module_sel[i]);
			}
		}else if(module_name.indexOf('lte') != -1){
			for(var i=0;i<module_sel.length;i++){
				$(".modal-body").append("<div class='form-group' style='width:100%;'>"+
					"module "+module_sel[i]+"<span class='err_res err_res_"+module_sel[i]+"'><?php echo language("Update waiting");?>.....</span>"+
					"<div class='progress'><div class='sub_progress module_"+module_sel[i]+"'></div>"+
						"<input type='hidden' class='chn_val' value='"+module_sel[i]+"' />"+
					"</div></div>"
				);
			}
			request(module_sel[0]);
		}else if(module_name.indexOf('cdma') != -1){
			for(var i=0;i<module_sel.length;i++){
				$(".modal-body").append("<div class='form-group' style='width:100%;'>"+
					"module "+module_sel[i]+"<span class='err_res err_res_"+module_sel[i]+"'><?php echo language("Update waiting");?>.....</span>"+
					"<div class='progress'><div class='sub_progress module_"+module_sel[i]+"'></div>"+
						"<input type='hidden' class='chn_val' value='"+module_sel[i]+"' />"+
					"</div></div>"
				);
			}
			request(module_sel[0]);
		}else if(module_name.indexOf('umts') != -1){
			for(var i=0;i<module_sel.length;i++){
				$(".modal-body").append("<div class='form-group' style='width:100%;'>"+
					"module "+module_sel[i]+"<span class='err_res err_res_"+module_sel[i]+"'><?php echo language("Update waiting");?>.....</span>"+
					"<div class='progress'><div class='sub_progress module_"+module_sel[i]+"'></div>"+
						"<input type='hidden' class='chn_val' value='"+module_sel[i]+"' />"+
					"</div></div>"
				);
			}
			request(module_sel[0]);
		}
		
		click_close();
	}
	
	function module_restart(that){
		var chn = that.children[0].value;
		$(".err_flag").each(function(){
			$(this).html("<?php echo language("Update waiting");?>.....");
		});
		
		if(update_complete()){
			$(".err_res").each(function(){
				if($(this).hasClass('err_flag')){
					$(this).removeClass('err_flag');
					$(this).removeClass('suc_flag');
					$(this).removeClass('upload_flag');
				}
			});
		}
		$(".err_res_"+chn).html("<?php echo language("Updating");?>.....");
		$(".module_"+chn).css("width", 0);
		$(".module_"+chn).text("");
		para.percent_time[chn] = 0;
		para.old_percent_val[chn] = 0;
		
		update(chn);
	}
	
	function click_close(){
		var msg = "<?php echo language("module update close tip", "The module is being updating, and the exit will cause the module update to fail. Are you sure you want to do this?");?>";
		$(".update_click_close").click(function(){
			if(!para.flag){
				if(confirm(msg)){
					$("#modal-51").css("display","none");
					$(".modal-backdrop").css("display","none");
					module_update_exit();
				}
			}else{
				$("#modal-51").css("display","none");
				$(".modal-backdrop").css("display","none");
			}
		});
	}
	
	result();
	</script>
	
<?php }

function module_update($module_sel){
	global $only_view;
	
	if(isset($_FILES['update_module_file']['error']) && $_FILES['update_module_file']['error'] == 0){
		if(!(isset($_FILES['update_module_file']['size'])) || $_FILES['update_module_file']['size'] > 80*1024*1024){ //Max file size 80Mbyte
			echo language('System Update filesize alert','Uploaded max file is 80M!');
			return false;
		}
		
		if(!file_exists('/data/module/')) exec('mkdir /data/module/');
		$store_file = "/data/module/".$_FILES['update_module_file']['name'];

		if (!move_uploaded_file($_FILES['update_module_file']['tmp_name'], $store_file)){
			echo "failed";
			return false;
		}
		
		$module_str = '';
		for($i=0;$i<count($module_sel);$i++){
			$module_str .= $module_sel[$i].',';
			if(file_exists("/www/chn_".$module_sel[$i])){
				unlink("/www/chn_".$module_sel[$i]);
			}
			if(file_exists("/www/upgrade_module_".$module_sel[$i].'.log')){
				unlink("/www/upgrade_module_".$module_sel[$i].'.log');
			}
		}
		$module_str = rtrim($module_str, ",");
		
		if(strstr($_FILES['update_module_file']['name'], 'M35')){
			exec("/my_tools/upgrade_module_m35.sh $module_str $store_file");
			
			save_user_record("","MODULE->Module Update:Update Module M35,channel=".$module_str);
		}else if(strstr($_FILES['update_module_file']['name'], 'M26')){
			exec("/my_tools/upgrade_module_m26.sh $module_str $store_file");
			
			save_user_record("","MODULE->Module Update:Update Module M26,channel=".$module_str);
		}else if(strstr($_FILES['update_module_file']['name'], 'EC20CE') || 
					strstr($_FILES['update_module_file']['name'], 'EC25EF') || 
					strstr($_FILES['update_module_file']['name'], 'EC25AUTL') || 
					strstr($_FILES['update_module_file']['name'], 'EC25AUT') || 
					strstr($_FILES['update_module_file']['name'], 'EC25AU') || 
					strstr($_FILES['update_module_file']['name'], 'EC25A') || 
					strstr($_FILES['update_module_file']['name'], 'EC25V') || 
					strstr($_FILES['update_module_file']['name'], 'EC25J')){
			exec("unzip $store_file -d /data/module");
			exec("/my_tools/upgrade_module_ec20.sh $module_sel[0] /data/module");
			
			save_user_record("","MODULE->Module Update:Update Module EC20CE,channel=".$module_sel[0]);
		}else if(strstr($_FILES['update_module_file']['name'], 'SIM6320C')){
			exec("/my_tools/upgrade_module_sim6320c.sh $module_sel[0] $store_file");
			
			save_user_record("","MODULE->Module Update:Update Module SIM6320C,channel=".$module_sel[0]);
		}else if(strstr($_FILES['update_module_file']['name'], 'UC15A')){
			exec("unzip $store_file -d /data/module");
			exec("/my_tools/upgrade_module_uc15.sh $module_sel[0] /data/module");
			
			save_user_record("","MODULE->Module Update:Update Module UC15A,channel=".$module_sel[0]);
		}
		
		return true;
	}
}
?>

<!--------------------------------------------------------------------------------------------------------------->

<!-- MCU update begin -->
	<div class="content">
		<span class="title"><?php echo language('MCU Update');?></span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Port');?>:
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<b><?php echo language('Port');?>:</b><br/>
							<?php echo language('Port help','Select the MCU port that needs to be updated.');?>
						</div>
					</div>
				</div>
			</span>
			<div class="tab_item_right" style="float:left;">
				<table cellpadding="0" cellspacing="0" class="port_table">
					<?php 
					for($i=1;$i<=$__GSM_SUM__;$i++){
						$port_name = get_gsm_name_by_channel($i);
						$port_name_arr = explode("-", $port_name, 2);
						$port_type = $port_name_arr[0];
						if($port_type == "null") continue;
						$mcu_port = $i/2;
						if(!is_int($mcu_port)) continue;
						
						$port_real_type = get_update_module_type($i);
						
						echo "<td class='mcu_module_port $port_real_type'><input type='checkbox' name='spans[1][$mcu_port]' value='$mcu_port' class='mcu_port'>";
						echo "$port_type-".($i-1).' & '."$port_type-".$i;
						echo '</td>';
					}
					?>
					<tr style="border:none;">
						<td style="border:none;">
							<input id="mcu_select_all" type="checkbox" >
							<?php echo language('All');?>
							<span style="float:right;margin-right:169px;"><input type='checkbox' id="mcu_show_all_ports" ><?php echo language('Show All');?></span>
						</td>
					</tr>
				</table>
			</div>
			<div class="clear"></div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('MCU Update file');?>:
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<b><?php echo language('MCU Update file');?>:</b><br/>
							<?php echo language('MCU Update file help','MCU Update file');?>
						</div>
					</div>
				</div>
			</span>
			<div class="tab_item_right">
				<input type="file" name="update_MCU_file" id="update_MCU_file" onchange="return module_checkFileChange();"/>
				
				<?php if(!$only_view){ ?>
				<input type="button" value="<?php echo language('Update');?>" onclick="document.getElementById('send').value='MCU Update';return MCU_submit_check()" style="float:right;margin-right:10px;" />
				<?php } ?>
				
			</div>
		</div>
	</div>

	<div class="comfirm_box" id="mcu_comfirm_box">
		<div class="comfirm_content">
			<p style="font-weight:bold;margin-bottom:0;"><?php echo language("Notice");?>:</p>
			<p style="color:red;font-size:14px;margin-top:0;">
				<?php echo language('MCU Update alert', 'Please note the following points,if it is violated,it may cause the MCU upgrade to fail!');?>
			</p>
			<hr />
			<div>
				<p style="color:blue;"><?php echo language('MCU Update note1', '1、MCU upgrade will take a long time.');?></p>
				<p style="color:blue;"><?php echo language('MCU Update note2', '2、System cannot be powered off during MCU update.');?></p>
				<p style="color:blue;"><?php echo language('MCU Update note3', '3、Do not reboot system or asterisk during MCU update.');?></p>
				<p style="color:blue;"><?php echo language('MCU Update note4', '4、Do not refresh this page during MCU update.');?></p>
				<p style="color:blue;"><?php echo language('MCU Update note5', '5、Do not perform other operations on this system during MCU update.');?></p>
			</div>
			<div class="comfirm_button">
			
				<?php if(!$only_view){ ?>
				<button type="submit" id="mcu_notice_confirm"><?php echo language('Confirm');?></button>
				<?php } ?>
				
				<button type="button" style="margin-left:100px;" id="mcu_notice_cancel"><?php echo language('Cancel');?></button>
			</div>
		</div>
	</div>
	
<!-- MCU update end -->

<script>
function module_checkFileChange(){
	var file = document.getElementById('update_MCU_file');
	var name = file.files[0].name;
	var filesize = file.files[0].size;

	if(filesize > 1024*1024*80){
		alert("<?php echo language('System Update filesize alert','Uploaded max file is 80M!');?>");
		return false;
	}
	
	if(name.indexOf('EC20') != -1){
		$(".mcu_module_port").hide();
		$(".EC20CE").show();
		$(".mcu_module_port").each(function(){
			if(!$(this).hasClass("EC20CE")){
				$(this).children(".mcu_port").removeAttr("checked");
			}
		});
	}else if(name.indexOf('EC25') != -1){
		$(".mcu_module_port").hide();
		$(".EC25EF").show();
		$(".EC25AUTL").show();
		$(".EC25AUT").show();
		$(".EC25AU").show();
		$(".EC25A").show();
		$(".EC25V").show();
		$(".EC25J").show();
		$(".mcu_module_port").each(function(){
			if(!$(this).hasClass("EC25EF") && !$(this).hasClass("EC25AUTL") && !$(this).hasClass("EC25AUT") && !$(this).hasClass("EC25AU") && !$(this).hasClass("EC25A") && !$(this).hasClass("EC25V") && !$(this).hasClass("EC25J")){
				$(this).children(".mcu_port").removeAttr("checked");
			}
		});
	}else if(name.indexOf('M35') != -1){
		$(".mcu_module_port").hide();
		$(".M35").show();
		$(".mcu_module_port").each(function(){
			if(!$(this).hasClass("M35")){
				$(this).children(".mcu_port").removeAttr("checked");
			}
		});
	}else if(name.indexOf('M26') != -1){
		$(".mcu_module_port").hide();
		$(".M26").show();
		$(".mcu_module_port").each(function(){
			if(!$(this).hasClass("M26")){
				$(this).children(".mcu_port").removeAttr("checked");
			}
		});
	}else if(name.indexOf('SIM6320') != -1){
		$(".mcu_module_port").hide();
		$(".cdma").show();
		$(".mcu_module_port").each(function(){
			if(!$(this).hasClass("cdma")){
				$(this).children(".mcu_port").removeAttr("checked");
			}
		});
	}else if(name.indexOf("UC15") != -1){
		$(".mcu_module_port").hide();
		$(".umts").show();
		$(".mcu_module_port").each(function(){
			if(!$(this).hasClass("umts")){
				$(this).children(".mcu_port").removeAttr("checked");
			}
		});
	}else{
		$(".mcu_module_port").hide();
		$(".mcu_port").removeAttr("checked");
		$("#mcu_select_all").removeAttr("checked");
	}
	
	return true;
}

function MCU_submit_check(){
	var file = document.getElementById('update_MCU_file');
	if(file.files[0] == undefined){
		alert("<?php echo language('Select File alert');?>");
		return false;
	}
	
	var flag = 0;
	$(".mcu_port").each(function(){
		if($(this).attr('checked') == 'checked'){
			flag = 1;
		}
	});
	if(flag == 0){
		alert("<?php echo language('Select port alert');?>");
		return false;
	}
	
	$("#mcu_comfirm_box").show();
	mcu_confirm();
}

function mcu_confirm(){
	$("#mcu_notice_confirm").click(function(){
		$("#mcu_comfirm_box").hide();
	});
	
	$("#mcu_notice_cancel").click(function(){
		$("#mcu_comfirm_box").hide();
	});
}

$("#mcu_select_all").click(function(){
	if($(this).attr("checked") == "checked"){
		$(".mcu_port").each(function(){
			if($(this).parent().css("display") != 'none'){
				$(this).attr("checked","checked");
			}
		});
	}else{
		$(".mcu_port").removeAttr("checked");
	}
});

//MCU show all ports
var mcu_select_port = [];
$("#mcu_show_all_ports").click(function(){
	if($(this).attr('checked') == 'checked'){
		$(".mcu_module_port").each(function(){
			if($(this).css('display') == 'none'){
				mcu_select_port.push($(this).attr('name'));
			}else{
				mcu_select_port.push('hide');
			}
		});
		$(".mcu_module_port").show();
	}else{
		var i = 0;
		$(".mcu_module_port").each(function(){
			if(mcu_select_port[i] != 'hide'){
				$(this).hide();
				$(this).children('.port').removeAttr('checked');
			}
			i++;
		});
		mcu_select_port = [];
	}
});
</script>

<?php 
function MCU_updating($module_sel){
?>
	<div class="modal fade in" id="modal-51" style="top:300px;">
		<div class="modal-dialog">
			<div class="modal-content">
			
				<div class="modal-header">
					<button type="button" class="close update_click_close">x</button>
					<h4 class="modal-title">
						<?php echo language('MCU Updating');?><br>
					</h4>
				</div>
				
				<div class="modal-body" style="height:300px;overflow:auto;"></div>
				
			</div>
		</div>
	</div>
	<input type="hidden" id="filename" value="<?php if(isset($_FILES['update_MCU_file']['name'])) echo $_FILES['update_MCU_file']['name'];?>" />
	<div class="modal-backdrop fade_in"></div>
	
	<script>
	var module_sel = [];
	var next = 0;
	function mcu_update(chn){
		$(".err_res_"+module_sel[next]).html("<?php echo language('Updating');?><img src='../../images/mini_loading.gif' /><input type='hidden' value='"+chn+"' />");
		
		var filename = document.getElementById('filename').value;
		var module_type = get_mcu_module_type();
		$.ajax({
			type: "POST",
			url: "ajax_server.php?type=mcu_update&update=",
			datetype: "text",
			data: {
				channel: chn,
				filename: filename,
				module_type: module_type
			},
			success: function(data){
				if(data.indexOf('success') != -1){
					$(".err_res_"+module_sel[next]).addClass('suc_flag');
					$(".err_res_"+module_sel[next]).html("<?php echo language('Update success')?>!");
				
					if(next<(module_sel.length-1)){
						next++;
						mcu_update(module_sel[next]);
						$(".err_res_"+module_sel[next]).html("<?php echo language('Updating');?><img src='../../images/mini_loading.gif' /><input type='hidden' value='"+chn+"' />");
					}
				}else{
					$(".err_res_"+module_sel[next]).addClass('err_flag');
					$(".err_res_"+module_sel[next]).html("<?php echo language("Update error", "Failure to upgrade, an unknown error.");?><input type='hidden' value='"+chn+"' />");
					if(next<(module_sel.length-1)){
						next++;
						mcu_update(module_sel[next]);
					}
				}
				mcu_monitor();
			},
			error: function(){
				$(".err_res_".module_sel[next]).addClass('err_flag');
				$(".err_res_"+module_sel[next]).html("<?php echo language('MCU Update failed','Failure to upgrade, Failure to connect to the server');?><input type='hidden' value='"+chn+"' />");
				if(next<(module_sel.length-1)){
					next++;
					mcu_update(module_sel[next]);
				}
				mcu_monitor();
			}
		});
	}
	
	var restart_sel = [];
	var re_next = 0;
	function mcu_restart(that, flag){
		if(flag == undefined){
			restart_sel = [];
			re_next = 0;
			
			var chn = $(that).siblings('input').val();
			restart_sel.push(chn);
			$(".err_flag").each(function(){
				var val = $(this).children('input').val();
				if(chn != val){
					restart_sel.push(val);
				}
				$(this).html("<?php echo language("Update waiting");?>.....");
				$(this).removeClass('err_flag');
			});
		}else{
			var chn = that;
		}
		
		$(".err_res_"+chn).html("<?php echo language('Updating');?><img src='../../images/mini_loading.gif' /><input type='hidden' value='"+chn+"' />");
		
		var filename = document.getElementById('filename').value;
		var module_type = get_mcu_module_type();
		$.ajax({
			type: "POST",
			url: "ajax_server.php?type=mcu_update&update=",
			datetype: "text",
			data: {
				channel: chn,
				filename: filename,
				module_type: module_type
			},
			success: function (data){
				if(data.indexOf('success') != -1){
					$(".err_res_"+restart_sel[re_next]).addClass('suc_flag');
					$(".err_res_"+restart_sel[re_next]).html("<?php echo language('Update success')?>!");
				
					if(re_next<(restart_sel.length-1)){
						re_next++;
						mcu_restart(restart_sel[re_next], 1);
						$(".err_res_"+restart_sel[re_next]).html("<?php echo language('Updating');?><img src='../../images/mini_loading.gif' /><input type='hidden' value='"+chn+"' />");
					}
				}else{
					$(".err_res_"+restart_sel[re_next]).addClass('err_flag');
					$(".err_res_"+restart_sel[re_next]).html("<?php echo language("Update error", "Failure to upgrade, an unknown error.");?><input type='hidden' value='"+chn+"' />");
					if(re_next<(restart_sel.length-1)){
						re_next++;
						mcu_restart(restart_sel[re_next], 1);
					}
				}
				mcu_monitor();
			},
			error: function(){
				$(".err_res_".restart_sel[re_next]).addClass('err_flag');
				$(".err_res_"+restart_sel[re_next]).html("<?php echo language('MCU Update failed','Failure to upgrade, Failure to connect to the server');?><input type='hidden' value='"+chn+"' />");
				if(re_next<(restart_sel.length-1)){
					re_next++;
					mcu_restart(restart_sel[re_next], 1);
				}
				mcu_monitor();
			}
		});
	}
	
	var monitor_flag = 0;
	function mcu_monitor(){
		monitor_flag = 1;
		$(".err_res").each(function(){
			if(!$(this).hasClass('suc_flag')){
				monitor_flag = 0;
			}
		});
		
		if(MCU_update_complete()){
			$(".err_res").each(function(){
				if($(this).hasClass('err_flag')){
					$(this).prepend("<span onclick='mcu_restart(this);' class='module_restart' >"+
										"<?php echo language("Restart update");?></span>");
				}
			});
		}
		
		if(monitor_flag){
			MCU_update_exit();
		}
	}
	
	function MCU_update_complete(){
		var complete_flag = 1;
		$(".err_res").each(function(){
			var err_flag = $(this).hasClass('err_flag');
			var suc_flag = $(this).hasClass('suc_flag');
			if(!err_flag && !suc_flag){
				complete_flag = 0;
				return false;
			}
		});
		return complete_flag;
	}
	
	function MCU_update_exit(){
		var module_sel = [];
		<?php for($i=0;$i<count($module_sel);$i++){?>
				module_sel.push(<?php echo $module_sel[$i];?>);
		<?php }?>
		
		$.ajax({
			type: "POST",
			url: "ajax_server.php?type=mcu_update",
			datetype: "json",
			data: {
				filename: JSON.stringify(module_sel)
			},
			success: function(data){
			}
		});
	}
	
	function get_mcu_module_type(){
		var module_name = '';
		<?php for($i=0;$i<count($module_sel);$i++){
				$module_name = get_gsm_name_by_channel($module_sel[$i]*2-1);
		?>
				module_name += "<?php echo $module_name;?>";
		<?php }?>
		if(module_name.indexOf('gsm') != -1){
			return 'gsm';
		}else if(module_name.indexOf('lte') != -1){
			return 'lte';
		}else if(module_name.indexOf('cdma') != -1){
			return 'cdma';
		}else if(module_name.indexOf('umts') != -1){
			return 'umts';
		}
		
		return '';
	}
	
	function mcu_click_close(){
		var msg = "<?php echo language("MCU update close tip", "The MCU is being updating, and the exit will cause the MCU update to fail. Are you sure you want to do this?");?>";
		$(".update_click_close").click(function(){
			if(!monitor_flag){
				if(confirm(msg)){
					$("#modal-51").css("display","none");
					$(".modal-backdrop").css("display","none");
					MCU_update_exit();
				}
			}else{
				$("#modal-51").css("display","none");
				$(".modal-backdrop").css("display","none");
			}
		});
	}
	
	function mcu_result(){
		var module_name = '';
		var module_type = '';
		<?php for($i=0;$i<count($module_sel);$i++){
				$module_name = get_gsm_name_by_channel($module_sel[$i]*2-1);
		?>
				module_name += "<?php echo $module_name;?>";
				module_sel.push(<?php echo $module_sel[$i];?>);
		<?php }?>
		
		if(module_name.indexOf('lte') != -1){
			module_type = 'lte';
		}else if(module_name.indexOf('gsm') != -1){
			module_type = 'gsm';
		}else if(module_name.indexOf('cdma') != -1){
			module_type = 'cdma';
		}else if(module_name.indexOf('umts') != -1){
			module_type = 'umts';
		}
		
		if(module_type != ''){
			for(var i=0;i<module_sel.length;i++){
				var mcu_name = module_type+'-'+(module_sel[i]*2-1)+' & '+module_type+'-'+(module_sel[i]*2);
				$(".modal-body").append("<div class='form-group style='width:100%;'>"+
						mcu_name+
						"<span class='err_res err_res_"+module_sel[i]+"'><?php echo language("Update waiting");?>......</span>"+
					"</div>"
				);
			}
			mcu_update(module_sel[0]);
		}else{
			$(".modal-body").append("<h3 style='color:red;'>Error!</h3>");
		}
		
		mcu_click_close();
	}
	
	mcu_result();
	</script>
	
<?php }?>
	
<?php
function MCU_update($module_sel){
	if(isset($_FILES['update_MCU_file']['error']) && $_FILES['update_MCU_file']['error'] == 0){
		if(!(isset($_FILES['update_MCU_file']['size'])) || $_FILES['update_MCU_file']['size'] > 80*1024*1024){ //Max file size 80Mbyte
			echo language('System Update filesize alert','Uploaded max file is 80M!');
			return false;
		}
		
		if(!file_exists('/data/module/')) exec('mkdir /data/module/');
		$store_file = "/data/module/".$_FILES['update_MCU_file']['name'];

		if(!move_uploaded_file($_FILES['update_MCU_file']['tmp_name'], $store_file)){
			echo "failed";
			return false;
		}
		
		return true;
	}
}

function get_module_select(){
	$module_sel = [];
	$i = 0;
	foreach($_POST['spans'][1] as $v){
		$module_sel[$i] = $v;
		$i++;
	}
	return $module_sel;
}

$enable_emu_logsettings = get_oem_info_sepcs('enable_emu_logsettings');
if($enable_emu_logsettings == 'on'){
?>

<!--------------------------------------------------------------------------------------------------------------->

<!-- EMU update begin -->
	<div class="content">
		<span class="title"><?php echo language('EMU Update');?></span>
		
		<div class="tab_item">
			<span>
				<?php echo language('Select a EMU to Update'); ?>:
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<b><?php echo language('Select a EMU to Update'); ?>:</b><br/>
							<?php echo language('Select a EMU to Update help','Select a EMU to Update');?>
						</div>
					</div>
				</div>
			</span>
			<div class="tab_item_right">
				<select id="emu" name="emu">
					<?php 
					for($i=1;$i<=4;$i++){
						$hw_ver = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value emu emu_".$i);
						if($hw_ver != ''){
							echo "<option value='$hw_ver' >$i</option>";
						}
					}
					?>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Baud Rate');?>:
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<b><?php echo language('Baud Rate');?>:</b><br/>
							<?php echo language('Baud Rate help','Baud Rate');?>
						</div>
					</div>
				</div>
			</span>
			<div class="tab_item_right">
				<select id="baud_rete" name="baud_rete">
					<option value="115200">115200</option>
					<option value="230400">230400</option>
					<option value="460800" selected>460800</option>
					<option value="921600">921600</option>
				</select>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('EMU Update file');?>:
				<div class="tip_main">
					<img class="tip_img" src="/images/tip.png" style="cursor:pointer;" />
					<div class="tip_help">
						<i class="top" ></i>
					
						<div class="tip_content">
							<b><?php echo language('EMU Update file');?>:</b><br/>
							<?php echo language('EMU Update File help','EMU Update File');?>
						</div>
					</div>
				</div>
			</span>
			<div class="tab_item_right">
				<input type="file" name="update_emu_file" id="update_emu_file" />
				
				<?php if(!$only_view){ ?>
				<input type="submit" value="<?php echo language('Update');?>" style="float:right;margin-right:10px;" onclick="document.getElementById('send').value='EMU Update';return EMU_submit_check();" />
				<?php } ?>
				
			</div>
		</div>
	</div>
	
<?php } ?>
	
</form>

<script>
function EMU_submit_check(){
	var file = document.getElementById('update_emu_file');
	if(file.files[0] == undefined){
		alert("<?php echo language('Select File alert');?>");
		return false;
	}
	
	if(!confirm('<?php echo language('EMU Update alert','Do not cut off the power supply during upgrading.Are you sure Update Emu?');?>')){
		return false;
	}
	return true;
}
</script>
<!-- EMU update end -->

	
<?php 
function emu_update(){
	if(! $_FILES){
		return;
	}
	
	echo "<br>";
	$Report = language('Report');
	$Result = language('Result');
	$theme = language("EMU Update");
	trace_output_start("$Report", "$theme");
	trace_output_newline();
	if(isset($_FILES['update_emu_file']['error']) && $_FILES['update_emu_file']['error'] == 0){
		if(!(isset($_FILES['update_emu_file']['size'])) || $_FILES['update_emu_file']['size'] > 80*1000*1000){
			echo language('Files Upload Filesize error',"Your uploaded file was larger than 80M!");
			trace_output_end();
			return;
		}
		
		$store_file = "/data/emu/";
		if(!is_dir($store_file)){
			mkdir($store_file);
		}
		
		$emu_file_path = $store_file.$_FILES['update_emu_file']['name'];
		
		if(!move_uploaded_file($_FILES['update_emu_file']['tmp_name'], $emu_file_path)){
			echo language('Files Upload Move error',"Moving your updated file was failed!");
			trace_output_end();
			return;
		}
		echo language("EMU Updating");echo "......";
		ob_flush();
		flush();
		
		$emu_dev = $_POST['emu'];
		$baud_rete = $_POST['baud_rete'];
		$cmd_emu_stop = "sh /etc/init.d/simemu.sh stop";
		$cmd = "/my_tools/emuUpdate $emu_file_path $emu_dev $baud_rete 1 1 > /tmp/log/emu_update.log 2>&1 ";
		exec($cmd_emu_stop);
		system($cmd,$output);
		if($output == 0){
			$file_name = $_FILES['update_emu_file']['name'];
			trace_output_newhead("$Result");
			echo language("EMU Update Succeeded.").'<br/>';
			echo "Update $file_name Succeeded";
			trace_output_end();
		}else{
			trace_output_newhead("$Result");
			echo language("EMU Update Failed");
			trace_output_end();
		}
		exec("rm -rf $store_file");
	}
	
	save_user_record("","MODULE->Module Update:Update EMU,emu channel=".$emu_dev);
}

if(isset($_POST['send'])){
	if($only_view){
		return false;
	}
	
	if($_POST['send'] == 'Module Update'){
		$module_sel = get_module_select();
		if(module_update($module_sel)){
			updating($module_sel);
		}else{
			echo "module update failed!";
		}
	}else if($_POST['send'] == 'MCU Update'){
		$module_sel = get_module_select();
		if(MCU_update($module_sel)){
			MCU_updating($module_sel);
		}else{
			echo "MCU update failed";
		}
	}else if($_POST['send'] == 'EMU Update'){
		
		emu_update();
	}
}

require("/www/cgi-bin/inc/boot.inc");?>