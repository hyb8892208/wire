<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
?>

<script type="text/javascript">
	var url_bts = "/../../service?action=process_log&log_type=bts_log&method=getcontents";
	var url_call = "/../../service?action=process_log&log_type=auto_intercall_log&method=getcontents";
	var url_sms = "/../../service?action=process_log&log_type=auto_intersms_log&method=getcontents";
	function get_log_content(){
		var show_kind = document.getElementById("interval").value;
		if(show_kind == 'bts'){
			$("#bts_toggle").show();
			var bts_board = document.getElementById("interval_bts").value;
			if(bts_board == 1){
				url_show = "/../../service?action=process_log&log_type=bts_log&method=getcontents";
			} else {
				url_show = "/../../"+bts_board+"/service?action=process_log&log_type=bts_log&method=getcontents";
			}
		} else if(show_kind == 'call'){
			$("#bts_toggle").hide();
			url_show = url_call;
		} else if(show_kind == 'sms'){
			$("#bts_toggle").hide();
			url_show = url_sms;
		} else {
			$("#bts_toggle").show();
			url_show = url_bts;
		}
		$.ajax({
			url: url_show,
			type: 'GET',
			dataType: 'text', 
			data: {},
			success: function(log_special_info){
				document.getElementById("showlog").value = log_special_info;
				document.getElementById("size").value = log_special_info.length;
			},
		});
	}
</script>

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Special Logs');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	<center>
		<textarea id="showlog" wrap="on" style="width:100%;height:450px" readonly></textarea>
		<input type="hidden" id="size" value="" />
		<table>
			<tr>	
				<td>
					<select id="interval" onchange="get_log_content();">
						<option value="bts" selected>BTS</option>
						<option value="call">Auto Internal Call</option>
						<option value="sms">Auto Internal SMS</option>
					</select>
				</td>
			<?php
				$board = get_slotnum();
				$cluster_info = get_cluster_info();
				echo '<td id="bts_toggle">';
				if($cluster_info['mode'] == 'master') {
					echo '<select name="interval_bts" id="interval_bts" onchange="get_log_content();">';
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
				}else{
					echo '<input type="text" style="display: none;" id="interval_bts" value="1"/>';
				}
				echo '</td>';
			?>
				<td>
					&nbsp;&nbsp;&nbsp;&nbsp;
				</td>
				<td>
					<input type="button" value="<?php echo language('Refresh');?>" onclick="get_log_content();"/>
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
$(document).ready(function(){
	get_log_content();
});
	
function show_last()
{
	var t = document.getElementById("showlog");
	t.scrollTop = t.scrollHeight;
}

function CleanUp()
{
	if(!confirm("<?php echo language('Clean Up confirm','Are you sure to clean up this logs?');?>")) return false;

	var size  = $("#size").attr("value");
	var show_kind = document.getElementById("interval").value;
	var log_type = '';
	if(show_kind == 'bts'){
		log_type = 'bts_log';
	} else if(show_kind == 'call'){
		log_type = 'auto_intercall_log';
	} else if(show_kind == 'sms'){
		log_type = 'auto_intersms_log';
	}
	$.ajax({
		url: "/../../service?random="+Math.random(),      //request file;
		type: 'GET',                                    //request type: 'GET','POST';
		dataType: 'text',                               //return data type: 'text','xml','json','html','script','jsonp';
		data: {
			'action':'process_log',
			'log_type':log_type,
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
</script>
<?php require("/www/cgi-bin/inc/boot.inc");?>
