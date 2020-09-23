<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
?>



<div style="width:49%;display:inline-block;">
	<div class="content">
		<span class="title"><?php echo language('RRI Logs');?></span>
		
		<textarea id="showlog"  style="width:100%;height:450px;resize:none;" readonly></textarea>
		<input type="hidden" id="rri_size" value="0" />
		<table style="margin:auto;">
			<tr>	
				<td><?php echo language('Refresh Rate');?>:</td>
				<td>
					<select id="rri_refresh_rate" onchange="change_setting();">
						<option value="0" selected>Off</option>
						<option value="1">1s</option>
						<option value="2">2s</option>
						<option value="3">3s</option>
						<option value="4">4s</option>
						<option value="5">5s</option>
						<option value="6">6s</option>
						<option value="7">7s</option>
						<option value="8">8s</option>
						<option value="9">9s</option>
					</select>
				</td>
				<td>
					&nbsp;&nbsp;&nbsp;&nbsp;
				</td>
				<td>
					<input type="button" value="<?php echo language('Refresh');?>" onclick="refresh();"/>
				</td>
				<?php if(!$only_view){ ?>
				<td>
					<input type="button" value="<?php echo language('Clean Up');?>"  onclick="return CleanUp();"/>
				</td>
				<?php } ?>
			</tr>
		</table>
	</div>
</div>

<div style="width:49%;float:right;margin-right:3px;">
	
	<div class="content">
		<span class="title"><?php echo language('RRI Pipe Logs');?></span>
		
		<textarea id="showlog_pipe"  style="width:100%;height:450px;resize:none;" readonly></textarea>
		<input type="hidden" id="rri_pipe_size" value="0" />
		<table style="margin:auto;">
			<tr>
				<td>
					<select name="port" id="port" onchange="change_setting();">
						<?php for($i=1; $i<=$__GSM_SUM__; $i++) {?>
						<option value="<?php echo $i;?>"><?php echo get_gsm_name_by_channel($i);?></option>
						<?php }?>
					</select>
				</td>
				<td><?php echo language('Refresh Rate');?>:</td>
				<td>
					<select id="rri_pipe_refresh_rate" onchange="change_setting();">
						<option value="0" selected>Off</option>
						<option value="1">1s</option>
						<option value="2">2s</option>
						<option value="3">3s</option>
						<option value="4">4s</option>
						<option value="5">5s</option>
						<option value="6">6s</option>
						<option value="7">7s</option>
						<option value="8">8s</option>
						<option value="9">9s</option>
					</select>
				</td>
				<td>
					&nbsp;&nbsp;&nbsp;&nbsp;
				</td>
				<td>
					<input type="button" value="<?php echo language('Refresh');?>" onclick="refresh();"/>
				</td>
				<?php if(!$only_view){ ?>
				<td>
					<input type="button" value="<?php echo language('Clean Up');?>"  onclick="return CleanUp_pipe();"/>
				</td>
				<?php } ?>
			</tr>
		</table>
	</div>
</div>

<script type="text/javascript" src="/js/functions.js"></script>

<script type="text/javascript">
function show_last(id){
	var t = document.getElementById(id);
	t.scrollTop = t.scrollHeight;
}

function CleanUp(){
	if(!confirm("<?php echo language('Clean Up confirm','Are you sure to clean up this logs?');?>")) return false;
	
	$.ajax({
		url: "/service?random="+Math.random(),
		type: 'GET',
		dataType: 'text',
		data: {
			'action':'process_log',
			'log_type':'rri_log',
			'method':'clean'
		},
		error: function(data){
		},
		success: function(data){
			document.getElementById("showlog").value = '';
			show_last("showlog");
		}
	});
}

function CleanUp_pipe(){
	if(!confirm("<?php echo language('Clean Up confirm', 'Are you sure to clean up this logs');?>")) return false;
	var port = getCookie('port_cookie_val');
	
	$.ajax({
		url: "/service?random="+Math.random(),
		type: 'GET',
		dataType: 'text',
		data:{
			'action':'process_log',
			'log_type':'rri_pipe_log',
			'port' : port,
			'method':'clean'
		},
		error: function(data){
		},
		success: function(data){
			document.getElementById('showlog_pipe').value = '';
			show_last("showlog_pipe");
		}
	});
}

function refresh(){
	window.location.reload();
}

function rri_update_log(){
	var size = $("#rri_size").val();
	
	$.ajax({
		url: "/service?random="+Math.random(),
		type: 'GET',
		dataType: 'text',
		data: {
			'action':'process_log',
			'log_type':'rri_log',
			'method':'update',
			'size':size
		},
		success: function(data){
			var pos = data.indexOf('&');
			var size = data.substring(0,pos);
			var contents = data.substring(pos+1);
			if(size=='' || size<0) size = 0;
			$("#rri_size").val(size);
			if(size == 0){
				document.getElementById('showlog').value = '';
			}else{
				if(contents != ""){
					var t = document.getElementById("showlog");
					t.value += contents;
					t.scrollTop = t.scrollHeight;
				}
			}
			var timeout = document.getElementById('rri_refresh_rate').value;
			if(timeout != 0){
				setTimeout(function(){rri_update_log();}, timeout*1000);
			}
		},
	});
}

function rri_pipe_update_log(){
	var port = getCookie('port_cookie_val');
	var size = $("#rri_pipe_size").val();
	
	$.ajax({
		url: "/service?random="+Math.random(),
		type: 'GET',
		dataType: 'text',
		data: {
			'action':'process_log',
			'log_type':'rri_pipe_log',
			'port' : port,
			'method':'update',
			'size' : size
		},
		success: function(data){
			var pos = data.indexOf('&');
			var size = data.substring(0,pos);
			var contents = data.substring(pos+1);
			if(size=='' || size<0) size = 0;
			$("#rri_pipe_size").val(size);
			if(size == 0){
				document.getElementById('showlog_pipe').value = '';
			}else{
				if(contents != ""){
					var t = document.getElementById('showlog_pipe');
					t.value += contents;
					t.scrollTop = t.scrollHeight;
				}
			}
			var timeout = document.getElementById('rri_pipe_refresh_rate').value;
			if(timeout != 0){
				setTimeout(function(){rri_pipe_update_log();}, timeout*1000);
			}
		},
	});
}

function change_setting(){
	var rri_rate_val = document.getElementById('rri_refresh_rate').value;
	var rri_pipe_rate_val = document.getElementById('rri_pipe_refresh_rate').value;
	var port_val = document.getElementById('port').value;
	
	setCookie('rri_cookie_val', rri_rate_val);
	setCookie('rri_pipe_cookie_val', rri_pipe_rate_val);
	setCookie('port_cookie_val', port_val);
	
	rri_update_log();
	rri_pipe_update_log();
}

function cookie_update(){
	//rri refresh rate
	var rri_cookie_val = getCookie('rri_cookie_val');
	if (rri_cookie_val != null) {
		document.getElementById("rri_refresh_rate").value = rri_cookie_val;
	}

	//rri pipe refresh rate
	var rri_pipe_cookie_val = getCookie('rri_pipe_cookie_val');
	if(rri_pipe_cookie_val != null){
		document.getElementById('rri_pipe_refresh_rate').value = rri_pipe_cookie_val;
	}

	//port
	var port_cookie_val = getCookie('port_cookie_val');
	if(port_cookie_val != null){
		document.getElementById('port').value = port_cookie_val;
	}else{
		$("#port option:first").attr("selected", "selected");
	}
	
	rri_update_log();
	rri_pipe_update_log();
}

$(document).ready(function(){
	cookie_update();
});
</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>