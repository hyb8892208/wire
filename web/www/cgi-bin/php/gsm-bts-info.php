<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
?>

<?php
function show_cell_info()
{
	//$alldata = get_all_gsm_info();
	$cell_num = 7;
	global $__GSM_SUM__;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;
	$cluster_info = get_cluster_info();
	$board = get_slotnum();
	$js_board_str = '"'.$board.'"';
	for($c=1; $c<=$__GSM_SUM__; $c++) {
?>
<div id="<?php echo "cell_div_${board}_${c}"; ?>">
<div id="tab">
	<li class="tb1">&nbsp;</li>
	<li class="tbg"><?php echo get_gsm_name_by_channel($c,1); ?></li>
	<li class="tb2">&nbsp;</li>
</div>

<table width="100%" class="tshow" id="<?php echo "gsm_table_${board}_${c}"; ?>">
	<th width=""><?php echo language('cell');?></th>
	<th width=""> <?php echo language('arfcn');?></th>
	<th width=""> <?php echo language('rxl');?></th>
	<th width=""><?php echo language('rxq');?></th>
	<th width=""><?php echo language('mcc');?></th>
	<th width=""> <?php echo language('mnc');?></th>
	<th width=""> <?php echo language('bsic');?></th>
	<th width=""> <?php echo language('cellid');?></th>
	<th width=""><?php echo language('rla');?></th>
	<th width=""> <?php echo language('txp');?></th>
	<th width=""> <?php echo language('lac');?></th>
	<th width=""><?php echo language('TA');?></th>
	<th width=""><?php echo language('Count');?></th>
	<?php 
		for($i = 1 ; $i <= $cell_num ; $i++){
	?>
	<tr>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
	</tr>
	<?php 
		}
	?>
</table>
<br>
</div>
<?php
	}
?>

<?php
	if($cluster_info['mode'] == 'master') {
		for($b=2; $b<=$__BRD_SUM__; $b++) {
			if(isset($cluster_info[$__BRD_HEAD__.$b.'_ip']) && $cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
				$js_board_str .= ",\"$b\"";
				for($c=1; $c<=$__GSM_SUM__; $c++) {
?>
<div id="<?php echo "cell_div_${b}_${c}"; ?>">
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo get_gsm_name_by_channel($c,$b); ?></li>
		<li class="tb2">&nbsp;</li>
	</div>

	<table width="100%" class="tshow" id="<?php echo "gsm_table_${b}_${c}"?>">
		<th width=""><?php echo language('cell');?></th>
		<th width=""> <?php echo language('arfcn');?></th>
		<th width=""> <?php echo language('rxl');?></th>
		<th width=""><?php echo language('rxq');?></th>
		<th width=""><?php echo language('mcc');?></th>
		<th width=""> <?php echo language('mnc');?></th>
		<th width=""> <?php echo language('bsic');?></th>
		<th width=""> <?php echo language('cellid');?></th>
		<th width=""><?php echo language('rla');?></th>
		<th width=""> <?php echo language('txp');?></th>
		<th width=""> <?php echo language('lac');?></th>
		<th width=""><?php echo language('TA');?></th>	
		<th width=""><?php echo language('Count');?></th>	
	<?php 
		for($i = 1 ; $i <= $cell_num ; $i++){
	?>
	<tr>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
		<td></td>
	</tr>
	<?php 
		}
	?>
	</table>
	<br>
</div>
<?php
				}
			}
		}
	}
?>
	<script type="text/javascript">
	$(document).ready(function (){
		show_gsm_cell_info();
	}); 
	function show_gsm_cell_info()
	{
		var bs = new Array(<?php echo $js_board_str;?>);
		for(i in bs) {
			get_gsm_cell_info(bs[i]);
		}

		function get_gsm_cell_info(board){
			if(board == <?php echo get_slotnum(); ?>){
				gsm_url = "/service?action=get_cellsinfo&" + "random=" + Math.random();
			}else{
				gsm_url = "/" + board + "/service?action=get_cellsinfo&" + "random=" + Math.random();
			}
			$.ajax({
				url: gsm_url,
				type: 'GET',
				dataType: 'json',
				data: {},
				success: function(gsm_cell_info){
					show_gsm_table(gsm_cell_info,board);
				},
				complete: function(){
					setTimeout(function(){get_gsm_cell_info(board);},10000);
				}
			});
		};

		function show_gsm_table(gsm_cell_info,board) {
			for(var key1 in gsm_cell_info){
				for(var key2 in gsm_cell_info[key1]) {
					for(var key3 in gsm_cell_info[key1][key2]) {
						for(var key4 in gsm_cell_info[key1][key2][key3]) {
							var tr_num = key3;
							tr_num = ++tr_num;
							$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(0)").html(gsm_cell_info[key1][key2][key3][key4].cell);
							$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(1)").html(gsm_cell_info[key1][key2][key3][key4].arfcn);
							$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(2)").html(gsm_cell_info[key1][key2][key3][key4].rxl);
							$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(3)").html(gsm_cell_info[key1][key2][key3][key4].rxq);
							$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(4)").html(gsm_cell_info[key1][key2][key3][key4].mcc);
							$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(5)").html(gsm_cell_info[key1][key2][key3][key4].mnc);
							$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(6)").html(gsm_cell_info[key1][key2][key3][key4].bsic);
							$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(7)").html(gsm_cell_info[key1][key2][key3][key4].cellid);
							$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(8)").html(gsm_cell_info[key1][key2][key3][key4].rla);
							$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(9)").html(gsm_cell_info[key1][key2][key3][key4].txp);
							$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(10)").html(gsm_cell_info[key1][key2][key3][key4].lac);
							$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(11)").html(gsm_cell_info[key1][key2][key3][key4].TA);
							$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(12)").html(gsm_cell_info[key1][key2][key3][key4].cellcount);
							if(gsm_cell_info[key1][key2][key3][key4].active == 1){
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(0)").addClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(1)").addClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(2)").addClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(3)").addClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(4)").addClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(5)").addClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(6)").addClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(7)").addClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(8)").addClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(9)").addClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(10)").addClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(11)").addClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(12)").addClass("cell");
							}else{
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(0)").removeClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(1)").removeClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(2)").removeClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(3)").removeClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(4)").removeClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(5)").removeClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(6)").removeClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(7)").removeClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(8)").removeClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(9)").removeClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(10)").removeClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(11)").removeClass("cell");
								$("#gsm_table_"+board+"_"+key1+" tr:eq("+tr_num+") td:eq(12)").removeClass("cell");
							}
						}
					}
				}
			}
		};
		
		function DeleteOldRow(board,gsm_num) {
			var tab = document.getElementById("gsm_table_"+board+"_"+gsm_num) ;
		    var rows = tab.rows.length ;
		    if(rows <= 1)return false;
		    for (var i= 1; rows > i; i++) {
		       tab.deleteRow(1);
		    }
		};
		
		function AddNewRow(board,key1){
			var oTab=document.getElementById("gsm_table_"+board+"_"+key1);
			var rows = oTab.rows.length ;
			var oTr = oTab.insertRow(rows);
			var newTd0 = oTr.insertCell();
			var newTd1 = oTr.insertCell();
			var newTd2 = oTr.insertCell();
			var newTd3 = oTr.insertCell();
			var newTd4 = oTr.insertCell();
			var newTd5 = oTr.insertCell();
			var newTd6 = oTr.insertCell();
			var newTd7 = oTr.insertCell();
			var newTd8 = oTr.insertCell();
			var newTd9 = oTr.insertCell();
			var newTd10 = oTr.insertCell();
			var newTd11 = oTr.insertCell();	
		};
	};
	</script>

	<br>
<?php
}
?>
<?php
	show_cell_info();
?>

<?php require("/www/cgi-bin/inc/boot.inc");?>
