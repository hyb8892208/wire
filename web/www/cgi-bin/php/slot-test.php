<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/aql.php");
?>

<table width="100%" class="tedit" >
	<tr>
		<th>
			<div class="helptooltips">
				<?php echo language('Slot Test');?>:
				<span class="showhelp">
				<?php echo language('Slot Test','Slot Test');?>:
				</span>
			</div>
		</th>
		<td>
			<button id="slot_test"><?php echo language('Test');?></button>
		</td>
	</tr>
</table>

<br/>
<table width="100%" class="tshow" >
	<tr>
		<th><?php echo language('Port');?></th>
		<th><?php echo language('Slot');?></th>
		<th><?php echo language('Result');?></th>
		<th><?php echo language('Retry');?></th>
	</tr>
</table>

<script>
$("#slot_test").click(function(){
	$(".tshow tr:first").nextAll().remove();
	$(".tshow").append("<tr><td colspan=4 style='text-align:center;'><img src='/images/loading.gif' /></td></tr>");
	
	$.ajax({
		url: '/cgi-bin/php/ajax_server.php?type=slot_test',
		type: 'GET',
		success: function(data){
		},
		error: function(data){
			console.log('slot test error');
		}
	});
	
	setTimeout(get_slot_test_log,10000);
});

var timeout = 0;
function get_slot_test_log(){
	$.ajax({
		url: '/cgi-bin/php/ajax_server.php?type=get_slot_test_log',
		type: 'GET',
		success: function(data){
			if(data != ''){
				$(".tshow tr").last().remove();
				
				render_table(data);
			}else{
				if(timeout == 360){
					$(".tshow tr").last().remove();
					$(".tshow").append("<tr><td colspan=4 style='color:red;text-align:center;'>Timeout,please Retry.</td></tr>");
				}
				
				timeout++;
				setTimeout(get_slot_test_log,1000);
			}
		},
		error: function(data){
			console.log('get slot test log error');
		}
	});
}

function render_table(data){
	var temp = data.split("\n");
	
	var arr = [];
	for(var i=0;i<temp.length;i++){
		if(temp[i] == '') continue;
		var tmp = temp[i].split(",");
		
		arr.push([tmp[0],tmp[1],tmp[2]]);
	}
	
	arr = arr.sort(sortNumber);
	
	var temp_arr = [];
	var second_length = arr.length/4;
	var n = 1;
	var second_arr = [];
	for(var i=0;i<arr.length;i++){
		second_arr.push(arr[i]);
		
		if(n==4){
			n=0;
			second_arr = second_arr.sort(sortNumber1);
			temp_arr = temp_arr.concat(second_arr);
			
			second_arr = [];
		}
		n++;
	}
	
	var html_str = '';
	for(var i=0;i<temp_arr.length;i++){
		if(temp_arr[i][2].indexOf("READY") != -1 && temp_arr[i][2].indexOf("OK") != -1){
			var style_str = 'style="color:green"';
		}else{
			var style_str = 'style="color:red"';
		}
		
		html_str = "<tr><td>"+temp_arr[i][0]+"</td><td>"+temp_arr[i][1]+"</td><td "+style_str+">"+temp_arr[i][2]+"</td><td><button type='button' onclick=\"Retry(this,'"+temp_arr[i][0]+"','"+temp_arr[i][1]+"');\"><?php echo language("Retry");?></button></td></tr>";
		$(".tshow").append(html_str);
	}
}

var retry_flag = [];
function Retry(that,channel,slot){
	if(retry_flag[channel] == 1){
		alert("<?php echo language("Channel");?>"+channel+"<?php echo language("is in operation");?>");
		return false;
	}
	retry_flag[channel] = 1;
	
	$(that).parent().prev().html('<img src="/images/mini_loading.gif" />');
	$.ajax({
		url: '/cgi-bin/php/ajax_server.php?type=slot_test_retry&channel='+channel+'&slot='+slot,
		type: 'GET',
		success: function(data){
			if(data.indexOf("READY") != -1 && data.indexOf("OK") != -1 ){
				$(that).parent().prev().css("color","green");
			}else{
				$(that).parent().prev().css("color","red");
			}
			
			retry_flag[channel] = 0;
			$(that).parent().prev().html(data);
		}
	});
}

function sortNumber(a,b){
	return a[0] - b[0];
}

function sortNumber1(a,b){
	return a[1] - b[1];
}
</script>

<?php
require("/www/cgi-bin/inc/boot.inc");
?>