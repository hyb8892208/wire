<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
include_once("/www/cgi-bin/inc/aql.php");
?>

<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript" src="/js/functions.js"></script>
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />
<script type="text/javascript" src="/js/jquery.ibutton.js"></script>
<script type="text/javascript" src="/js/float_btn.js"></script>

<?php
function show_pin_list(){
	global $__GSM_SUM__;
?>
	<form enctype="multipart/form-data" action="<?php echo get_self();?>" method="get" >
		<table width="100%" class="tshow">
			<tr>
				<th style="width:0.3%"><input type="checkbox" name="selall_port" onclick="selectAll(this.checked,'port[]')" /></th>
				<th width="150px"><?php echo language('Port');?>-SIM</th>
				<th><?php echo language('Pin Code');?></th>
				<th width="30px"><?php echo language('Action');?></th>
			</tr>
			
			<?php 
			$aql = new aql();
			$aql->set('basedir','/etc/asterisk/gw/call_limit/');
			$res = $aql->query("select * from sim_info.conf");
			
			for($i=1;$i<=$__GSM_SUM__;$i++){
				$port = get_gsm_name_by_channel_for_showtype($i);
				$channel_type = get_gsm_type_by_channel($i,1);
				for($j=1;$j<=4;$j++){
					$pin_code = $res[$i.'-'.$j]['pincode'];
			?>
			<tr <?php if($channel_type == 'NULL') echo 'style="display:none;"'; ?> >
				<td><input type="checkbox" class="pin_code_checked" name="port[]" value="<?php echo $i.'-'.$j;?>" /></td>
				<td><?php echo $i.'-'.$j;?></td>
				<td>
					<input type="text" name="pin_code" class="pin_code" style="width:500px;" value="<?php echo $pin_code;?>"/>
					<!--<input type="checkbox" name="pin_code_sw" class="pin_code_sw" <?php echo $pin_code_sw;?> />
					On-->
				</td>
				<td>
					<button class="saveone" type="button" style="width:32px;height:32px;" value="<?php echo $i.'-'.$j;?>">
						<img src="/images/save.png" />
					</button>
				</td>
			</tr>
			<?php
				}
			}
			?>
		</table>
		
		<div id="newline"></div>
		
		<table id="float_btn" class="float_btn">
			<tbody>
				<tr id="float_btn_tr" class="float_btn_tr" style="display: block;">
					<td>
						<input type="hidden" name="send" id="send" value="Save">
						<input type="button" class="saveall" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check_batch();">
					</td>
					<td>
						<input type="button" value="<?php echo language('Cancel');?>" onclick="location.reload()">
					</td>
				</tr>
			</tbody>
		</table>
		
		<table id="float_btn2" style="border: none; display: none; left: 349px;" class="float_btn2">
			<tbody>
				<tr id="float_btn_tr2" class="float_btn_tr2">
					<td style="width:51px;">
						<input type="button" id="float_button_1" class="float_short_button saveall" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';">
					</td>
					<td style="width:51px;">
						<input type="button" id="float_button_2" class="float_short_button" value="<?php echo language('Cancel');?>" onclick="location.reload()">
					</td>
				</tr>
			</tbody>
		</table>
	</form>

	<script>
	$(function(){
		// $(".pin_code_sw").each(function(){
			// if($(this).attr('checked') == 'checked'){
				// $(this).siblings(".pin_code").removeAttr("readonly");
			// }else{
				// $(this).siblings(".pin_code").attr("readonly","readonly");
			// }
		// });
	});
	
	$(".pin_code_sw").change(function(){
		if($(this).attr("checked") == "checked"){
			$(this).siblings(".pin_code").removeAttr("readonly");
		}else{
			$(this).siblings(".pin_code").attr("readonly","readonly");
		}
	});
	
	$(".saveone").click(function(){
		var port_sim = $(this).val();
		var temp = port_sim.split('-');
		var channel = temp[0];
		var sim_ind = temp[1];
		
		var pin_code = $(this).parent().siblings().children(".pin_code").val();
		if($(this).parent().siblings().children(".pin_code_sw").attr("checked") == "checked"){
			var need_pin = 1;
		}else{
			var need_pin = 0;
		}
		
		$.ajax({
			type:"POST",
			url:"ajax_server.php?type=save_pin_one",
			data:{
				"channel":channel,
				"sim_ind":sim_ind,
				"pin_code":pin_code,
				"need_pin":need_pin
			},
			success:function(data){
				window.location.reload();
			},
			error:function(data){
				console.log(data);
			}
		});
	});
	
	$(".saveall").click(function(){
		if($(".pin_code_checked:checked").length == 0){
			alert("<?php echo language("Please choose port");?>");
			return false;
		}
		
		var arr = [];
		$(".pin_code_checked").each(function(){
			if($(this).attr('checked') == 'checked'){
				var pin_code = $(this).parent().siblings().children('.pin_code').val();
				if($(this).parent().siblings().children('.pin_code_sw').attr('checked') == "checked"){
					var need_pin = 1;
				}else{
					var need_pin = 0;
				}
				
				var port_sim = $(this).val();
				var temp = port_sim.split('-');
				var channel = temp[0];
				var sim_ind = temp[1];
				
				var obj = {
					'channel':channel,
					'sim_ind':sim_ind,
					'pin_code':pin_code,
					'need_pin':need_pin
				}
				
				arr.push(obj);
			}
		});
		
		$.ajax({
			type:"POST",
			url:"ajax_server.php?type=save_pin_all",
			data:{
				"arr":arr
			},
			success:function(data){
				console.log(data);
				window.location.reload();
			},
			error:function(data){
				console.log(data);
			}
		});
		
	});
	</script>
<?php
}

show_pin_list();

require("/www/cgi-bin/inc/boot.inc");
?>
<div id="float_btn1" class="sec_float_btn1"></div>
<div class="float_close" onclick="close_btn()" ></div>