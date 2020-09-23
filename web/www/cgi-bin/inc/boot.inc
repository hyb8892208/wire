		</div>
	<div id="boot">
		<?php 
		if($data_info_arr['switch'] == 'on'){
			echo $data_info_arr['copyright'];
		}else{
			if($__system_type__ == 'openvox'){
		?>
				Copyright © <span id="copyright_time"></span> OpenVox All Rights Reserved.<br>TEL:+86-755-82535461 FAX:+86-755-83823074;
		<?php
			}
		}
		?>
	</div>
	<script>
		<?php if($data_info_arr['switch'] == 'on'){?>
			var footer_image = '<?php echo $data_info_arr['footer_image'];?>';
			
			if(footer_image != ''){
				$("#boot").css("background","url(/data/info/images/footer_image.png?v="+rand+") no-repeat");
			}
		<?php }
			if($__system_type__ == 'openvox'){
		?>
		var myDate = new Date();
		var copyright_year = myDate.getFullYear();
		$("#copyright_time").html(copyright_year);
		<?php } ?>
	</script>
	<?php
		function check_apply()
		{
			$wait_apply_file = "/tmp/web/wait_apply";
			if(file_exists($wait_apply_file) && filesize($wait_apply_file)>0)
				return true;

			return false;
		}

		function show_apply_infobar()
		{
	?>
	<script type="text/javascript">
		$(document).ready(function(){
			float_btn_location();
			$("#infobar").slideDown(300);
			$("#apply").click(function(){
				$("#infobar").slideUp(300);
				float_btn_pri_location();
				var url = "apply.php?apply=yes";
				$.ajax({
					url: url+"&random="+Math.random(),
					dataType: 'text',
					type: 'POST',
					timeout: 20000,
					error: function(data){
					},
					success: function(data){
					}
				});
			});
			$("#infobar_close").click(function(){
				$("#infobar").slideUp(300);
				float_btn_pri_location();
			});
		});
		function float_btn_pri_location()
		{
			$(".sec_float_btn1").animate({bottom:"0px"}, 300);
			$(".float_btn2").animate({bottom:"0px"}, 300);
			$(".float_close").animate({bottom:"27px"}, 300);
			$("#float_progress").animate({bottom:"-1px"}, 300);
		};
		function float_btn_location()
		{
			$(".sec_float_btn1").animate({bottom:"38px"}, 300);
			$(".float_btn2").animate({bottom:"38px"}, 300);
			$(".float_close").animate({bottom:"65px"}, 300);
			$("#float_progress").animate({bottom:"37px"}, 300);
		};
		
	</script>
	<?php
		}
		if(check_apply())
			show_apply_infobar();
	?>
	<div id="language_debug" class="language_debug" style="<?php if($language_debug == 'true') {echo 'display:block';}else{echo 'display:none';} ?>">
	    <span><?php echo language('LANGUAGE DEBUG MODE','LANGUAGE DEBUG MODE');?></span>
	</div>
	<div id="infobar" align="center" style="display:none">
		<table width="100%" height="100%"><tr><td valign="middle">
			<span><?php echo language('Apply Bar help','Settings have been changed. Calls may be terminated when you apply these changes. Do you want to apply now ?');?></span>
			<input type="button" name="apply" id="apply" valign="middle" style="top:-50%" value="<?php echo language('Apply');?>" />
			<span id="infobar_close" >x</span>
		</td></tr></table>
	</div>
</div>
</body>
</html>
