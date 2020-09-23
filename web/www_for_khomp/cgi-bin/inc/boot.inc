
	<div id="bottom" class="content">
		<div id="bottom_top">
			<div id="bottom_top_content">
				<a href="https://www.khomp.com" target="_blank">Khomp</a>
				|
				<a href="https://www.khomp.com" target="_blank">Todos os direitos reservados.</a>
				|
				<a href="https://www.khomp.com" target="_blank">khomp.com</a>
			</div>
		</div>
		<div id="bottom_content">
			<span>KVoLTE</span>
		</div>
	</div>
	
	<script>
	<?php if(check_apply()){ ?>
		$(".cancel_apply").show();
		// $(".apply_none").hide();
		// $(".apply").show();
	<?php } ?>
	
	$(".apply").click(function(){
		// $(".apply").hide();
		// $(".apply_none").show();
		$(".cancel_apply").hide();
		
		var url = "apply.php?apply=yes";
		$.ajax({
			url: url+"&random="+Math.random(),
			dataType: 'text',
			type: 'POST',
			timeout: 20000,
			error: function(data){
			},
			success: function(data){
				$("#apply_result").fadeIn(200).delay(600).fadeOut(1000);
			}
		});
	});
	
	$("#cancel_apply_point").click(function(){
		var ret = confirm("<?php echo language("Apply Back help","Are you sure you want to cancel the configuration settings?");?>");
		if(ret){
			$.ajax({
				url: "/cgi-bin/php/ajax_server.php?type=conf_back",
				type: "GET",
				success: function(data){
					window.location.href = "<?php echo get_self();?>";
				}
			});
		}
	});
	</script>
</div>
</body>
</html>