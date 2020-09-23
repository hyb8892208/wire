<?php
require_once("/www/cgi-bin/inc/head.inc");
require_once("/www/cgi-bin/inc/menu.inc");
require_once("/www/cgi-bin/inc/userdb.php");

$db = new Users();
$line_counts = 15;//一个页面的条数
$cur_page = 1;//默认第1个页面

$user_id = $_GET['user_id'];
$username = $_GET['username'];

$count_res = $db->get_record_count_by_userid($user_id);
$count_tmp = $count_res->fetchArray();
$counts = $count_tmp[0];
$page_count = ceil($counts / $line_counts);

if(isset($_GET['current_page']) && is_numeric($_GET['current_page'])){
	$cur_page = $_GET['current_page'];
	
	if($cur_page > $page_count){
		$cur_page = $page_count;
	}
}
$start_record = ($cur_page-1)*$line_counts;

$record_res = $db->get_record_by_userid($user_id,$start_record,$line_counts);

?>


<div class="content">
	<table class="table_show">
		<tr>
			<th><?php echo language('Username');?></th>
			<th>IP</th>
			<th><?php echo language('Action');?></th>
			<th><?php echo language('Time');?></th>
		</tr>
		
		<?php while($record_info = $record_res->fetchArray()){
			$ip = $record_info['ip'];
			$action = $record_info['action'];
		?>
		
		<tr>
			<td><?php echo $username;?></td>
			<td><?php echo $ip;?></td>
			<td><?php echo $action;?></td>
			<td><?php system("date -d @".$record_info['time']." +\"%Y-%m-%d %H:%M:%S\"");?></td>
		</tr>
		
		<?php } ?>
	</table>
</div>

<?php 
	if($page_count >= 2) {
		echo '<br/>';
		echo '<div class="pg">';

		if( $cur_page > 1 ) {
			$page = $cur_page - 1;
			echo "<a title=\"";echo language('Previous page');echo "\" style=\"cursor:pointer;\" onclick=\"getpage($page);\"  class=\"prev\"></a>";
		} else {
			
		}
			
		if($cur_page-5 > 1) {
			$s = $cur_page-5;
		} else {
			$s = 1;
		}

		if($s + 10 < $page_count) {
			$e = $s + 10;
		} else {
			$e = $page_count;
		}

		for($i = $s; $i <= $e; $i++) {
			if($i != $cur_page) {
				echo "<a style=\"cursor:pointer;\" onclick=\"getpage($i);\" >$i</a>";
			} else {
				echo "<strong>$cur_page</strong>";
			}
		}

		if( $cur_page < $page_count ) {
			$page = $cur_page + 1;
			echo "<a title=\"";echo language('Next page');echo "\" style=\"cursor:pointer;\" onclick=\"getpage($page)\" class=\"nxt\" ></a>";
		} else {
			
		}

		echo "<label>";
		echo "<input type=\"text\" id=\"input_page\" name=\"page\" value=\"$cur_page\" size=\"2\" class=\"px\" title=\"";
		echo language('input page help','Please input your page number, and press [Enter] to skip to.');
		echo "\" onkeypress=\"keypress(event)\" >";
		echo "<span title=\"";echo language('total pages');echo ": $page_count\"> / $page_count</span>";
		echo "</label>";
		echo "<a title=\"";echo language('goto input page');echo "\" style=\"cursor:pointer;\" onclick=\"getpage(document.getElementById('input_page').value)\">";
		echo language('go');
		echo "</a>";
		echo '</div>';
	}else if($page_count == 1){
		echo '<br/>';
		echo '<div class="pg">';
		echo "<strong>1</strong>";
		echo '</div>';
	}
	
?>
	
	<br/>
	
	<script>
	function getpage(page){
		var url = '<?php echo get_self();?>?user_id=<?php echo $_GET['user_id'];?>&username=<?php echo $_GET['username'];?>&current_page='+page;
		
		window.location.href = url;
	}
	</script>

<?php require("/www/cgi-bin/inc/boot.inc");?>