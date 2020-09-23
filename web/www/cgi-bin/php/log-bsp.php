<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
?>

<?php
function echo_contents(){
?>

<script type="text/javascript">
	var url = "/tmp/log/bsp_server.log";
	$.ajax({
		url: url,
		type: 'GET',
		dataType: 'text',
		data: {},
		success: function(log_astinfo){
			document.getElementById("showlog").value = log_astinfo;
			document.getElementById("size").value = log_astinfo.length;
		},
	});
</script>

<?php
}
?>

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('BSP Logs');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	<center>
		<textarea id="showlog"  style="width:100%;height:450px" readonly></textarea>
		<input type="hidden" id="size" value="0" />
		<table>
			<tr>	
				<td><?php echo language('Refresh Rate');?>:</td>
				<td>
					<select id="interval" onchange="change_refresh_rate(this.value);">
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
				<td>
					<input type="button" value="<?php echo language('Clean Up');?>"  onclick="return CleanUp();"/>
				</td>
			</tr>
		</table>
	</center>
	

<script type="text/javascript" src="/js/functions.js"></script>

<script type="text/javascript">
function show_last(){
	var t = document.getElementById("showlog");
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
			'log_type':'bsp_log',
			'method':'clean'
		},
		error: function(data){
		},
		success: function(data){
			document.getElementById("showlog").value = '';
			show_last();
		}
	});
}

var updateStop = false;
function change_refresh_rate(value) {
	setCookie("cookieInterval", value);
	if(value != 0 && updateStop){
		updateStop = false;
		update_log();
	}
}

function refresh() {
	window.location.href="<?php echo get_self()?>";
}

function update_log() {
	var size  = $("#size").attr("value");	
	
	$.ajax({
		url: "/service?random="+Math.random(),
		type: 'GET',
		dataType: 'text',
		data: {
			'action':'process_log',
			'log_type':'bsp_log',
			'method':'update',
			'size':size
		},
		error: function(data){
		},
		success: function(data){
			var pos = data.indexOf('&');
			var size = data.substring(0,pos);
			var contents = data.substring(pos+1);
			if(size=='' || size<0) size = 0;
			$("#size").attr("value", size);
			if(size == 0){
				document.getElementById("showlog").value = '';
				show_last();
			} else {
				if(contents != ""){
					var t = document.getElementById("showlog");
					t.value += contents;
					t.scrollTop = t.scrollHeight;
				}
			}
		},
		complete: function(){
			var timeout = $("#interval").attr("value");
			if( timeout != 0){
				setTimeout(function(){update_log();}, timeout*1000);
			}else{
				updateStop = true;
			}
		}
	});
}

function cookie_update() {
	var cookieInterval = getCookie('cookieInterval');
	var nowInterval = document.getElementById("interval");

	if (cookieInterval == null) {
		setCookie("cookieInterval", nowInterval.value)
	} else {
		nowInterval.value = cookieInterval;
	}
}

$(document).ready(function(){
	cookie_update();
	show_last();
	var timeout = $("#interval").attr("value");
	if( timeout != 0) {
		update_log();
	}else{
		updateStop = true;
	}
});
</script>
<?php
echo_contents();
?>
<?php require("/www/cgi-bin/inc/boot.inc");?>
