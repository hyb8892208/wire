<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
?>

<?php
if(isset($_GET['board'])) {
	$board = $_GET['board'];
} else {
	$board = get_slotnum();
}

function echo_contents($board)
{
	if($board != 1) {
		$head = "/$board";
	} else {
		$head = '';
	}

	$url = "'$head/data/log/sys-log'";
?>
<script type="text/javascript">
	$.ajax({
		url: <?php echo $url;?>,
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
		<li class="tbg"><?php echo language('System Logs');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	<center>
		<textarea id="showlog" wrap="on" style="width:100%;height:450px" readonly></textarea>
		<input type="hidden" id="size" value="" />
		<table>
			<tr>	
			<?php
				if($__deal_cluster__){
					$cluster_info = get_cluster_info();
					if($cluster_info['mode'] == 'master') {
						echo '<td>';
						echo '<select size=1 name="channel" id="channel" onchange="change_channel(this.value);">';
						$selected = '';
						if($board == 1) $selected = 'selected';
						echo "<option value=\"1\" $selected>";
						echo $__BRD_HEAD__.'1';
						echo "</option>\n";
						for($b=2; $b<=$__BRD_SUM__; $b++) {
							if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
								$selected = '';
								if($b == $board) $selected = "selected";
								echo "<option value=\"$b\" $selected>";
								echo $__BRD_HEAD__.$b;
								echo "</option>\n";
							}
						}
						echo '</select>';
						echo '</td>';
					}
				}
			?>

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
	

<script type="text/javascript" src="/js/functions.js">
</script>

<script type="text/javascript">
function show_last()
{
	var t = document.getElementById("showlog");
	t.scrollTop = t.scrollHeight;
}

function CleanUp()
{
	if(!confirm("<?php echo language('Clean Up confirm','Are you sure to clean up this logs?');?>")) return false;
	var size  = $("#size").attr("value");
	
<?php
	if($board == 1) {
		echo "var server_file=\"/service\";\n";
	} else {
		echo "var server_file=\"/$board/service\";\n";
	}
?>

	$.ajax({
		url: server_file+"?random="+Math.random(),      //request file;
		type: 'GET',                                    //request type: 'GET','POST';
		dataType: 'text',                               //return data type: 'text','xml','json','html','script','jsonp';
		data: {
			'action':'process_log',
			'log_type':'sys_log',
			'method':'clean'
		},
		error: function(data){                          //request failed callback function;
			//alert("get data error");
		},
		success: function(data){                        //request success callback function;
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

function change_channel(value) {
	board = value;
	window.location.href="<?php echo get_self()?>"+"?board="+board;
}

function refresh() {
	window.location.href="<?php echo get_self()?>"+"?board="+<?php echo $board;?>;
}

function update_log() {
	var size  = $("#size").attr("value");
	
<?php
	if($board == 1) {
		echo "var server_file=\"/service\";\n";
	} else {
		echo "var server_file=\"/$board/service\";\n";
	}
?>	
	$.ajax({
		url: server_file+"?random="+Math.random(),      //request file;
		type: 'GET',                                    //request type: 'GET','POST';
		dataType: 'text',                               //return data type: 'text','xml','json','html','script','jsonp';
		data: {
			'action':'process_log',
			'log_type':'sys_log',
			'method':'update',
			'size':size
		},
		error: function(data){                          //request failed callback function;
			//alert("get data error");
		},
		success: function(data){                        //request success callback function;
			var pos = data.indexOf('&');
			var size = data.substring(0,pos);
			var contents = data.substring(pos+1);
			if(size=='' || size<0) size = 0;
			$("#size").attr("value", size);
			if(size == 0) {
				document.getElementById("showlog").value = '';
				show_last();
			} else {
				if(contents != "") {
					var t = document.getElementById("showlog");
					t.value += contents;
					t.scrollTop = t.scrollHeight;
					//show_last();
				}
			}
		},
		complete: function(){
			var timeout = $("#interval").attr("value");
			if( timeout != 0) {
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
echo_contents($board);
?>
<?php require("/www/cgi-bin/inc/boot.inc");?>
