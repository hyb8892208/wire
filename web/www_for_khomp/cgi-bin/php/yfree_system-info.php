<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
?>

<?php
function printData()
{
	echo `df /data -h 2>/dev/null | awk 'NR==2{printf("%s/%s (%s)",$3,$2,$5)}' 2> /dev/null`;
}

function printMem()
{
	$res= `cat /proc/meminfo 2>/dev/null  | awk '{if(NR==1){A=$2}if(NR==2){B=$2}}END{print (A-B)/A*100}' 2>/dev/null`;
	echo "${res}%";
}

function memory_clean()
{
	exec("echo 3 > /proc/sys/vm/drop_caches;echo 0 > /proc/sys/vm/drop_caches");
}

if(isset($_GET['memory_clean']) && $_GET['memory_clean'] == 'yes') {
	memory_clean();
}

//////////////////////////////
$all_board_info = read_redis_info();

?>
	
	<table width="100%" class="tedit" >
		<tr>
			<th><?php echo language('Model Name');?>:</th>
			<td>
				<?php 
					//get_module_info('name',$all_board_info['master']['local.product.module.type']);
					echo "SWG-1016";
				?>
			</td>
	
		</tr>
		<tr>
			<th><?php echo language('Modem Description');?>:</th>
			<td>
			<?php 
				//get_module_info('modem',$all_board_info['master']['local.product.module.type']);
				if ($flag_cdma){
					echo "800MHz@CDMA 2000";
				} else {
					echo "850/900/1800/1900MHz@GSM";
				}
			?>
			</td>
		</tr>
		<tr>
			<th><?php echo language('Software Version');?>:</th>
			<td><?php if(file_exists('/version/version')) readfile('/version/version'); ?></td>
		</tr>
		<tr>
			<th><?php echo language('Hardware Version');?>:</th>
			<td><?php echo $all_board_info['master']['local.product.board.version']; ?></td>
		</tr>
		<tr>
			<th><?php echo language('Slot Number');?>:</th>
			<td><?php echo `cat /tmp/.slotnum 2> /dev/null` ?></td>
		</tr>
		<tr>
			<th><?php echo language('Storage Usage');?>:</th>
			<td><?php printData(); ?></td>
		</tr>
		<tr>
			<th><?php echo language('Memory Usage');?>:</th>
			<td>
				<table><tr>
					<td style="border:0px;padding:0px"><?php printMem(); ?></td>
					<td style="border:0px;"><a href="<?php echo get_self() ?>?memory_clean=yes"><?php echo language('Memory Clean');?></a></td>
				</tr></table>
			</td>
		</tr>
		<tr>
			<th><?php echo language('Build Time');?>:</th>
			<td><?php if(file_exists('/version/build_time')) readfile('/version/build_time'); ?></td>
		</tr>

<!--segcomdels--><!--teiqdels-->
		<tr>
			<th><?php echo language('Contact Address');?>:</th>
			<td>
			<?php echo language('Contact Address Contents','Rm.1012,No.1 Building,No.2875,YangGao Rd.,Shanghai');?>
			</td>
		</tr>
<!--segcomdele--><!--teiqdele-->

		<tr>
			<th><?php echo language('Tel');?>:</th>
			<td>+86-21-33191992-801</td>
		</tr>

<!--segcomdels--><!--teiqdels-->
		<tr>
			<th><?php echo language('Fax');?>:</th>
			<td>+86-21-33191992-860</td>
		</tr>
<!--segcomdele--><!--teiqdele-->

		<tr>
			<th><?php echo language('E-Mail');?>:</th>
			<td><a href="mailto:shenyj@h-shen.com">shenyj@h-shen.com</a></td>
		</tr>
		<tr>
			<th><?php echo language('Web Site');?>:</th>
			<td><a href="http://www.h-shen.com" target="_top">http://www.h-shen.com</a></td>
		</tr>
		<tr>
			<th><?php echo language('System Time');?>:</th>
			<td><span id="currenttime"></span></td>
		</tr>
		<tr>
			<th><?php echo language('System Uptime');?>:</th>
			<td><span id="uptime"></span></td>
		</tr>
	</table>

<?php

if(file_exists("/proc/uptime")) {
	$fh = fopen("/proc/uptime","r");
	$line = fgets($fh);
	fclose($fh);

	$start_time = substr($line,0,strpos($line,'.'));

	$all_time = trim(`date "+%Y:%m:%d:%H:%M:%S"`);
	$item = explode(':', $all_time, 6);
	if(isset($item[5])) {
		$year = $item[0];
		$month = $item[1];
		$date = $item[2];
		$hour = $item[3];
		$minute = $item[4];
		$second = $item[5];
	}
?>

<script type="text/javascript" language="javascript">
<!--
var c=0;
var Y=<?php echo $year?>;
var M=<?php echo $month?>;
var D=<?php echo $date?>;
var sec=<?php echo $hour*3600+$minute*60+$second?>;
function ctime() {
	sec++;
	H=Math.floor(sec/3600)%24;
	I=Math.floor(sec/60)%60;
	S=sec%60;
	if(S<10) S='0'+S;
	if(I<10) I='0'+I;
	if(H<10) H='0'+H;
	if (H=='00' & I=='00' & S=='00') D=D+1; //日进位
	if (M==2) { //判断是否为二月份******
		if (Y%4==0 && !Y%100==0 || Y%400==0) { //是闰年(二月有28天)
			if (D==30){
				M+=1;D=1;
			} //月份进位
		} else { //非闰年(二月有29天)
			if (D==29) {
				M+=1;D=1;
			} //月份进位
		}
	} else { //不是二月份的月份******
		if (M==4 || M==6 || M==9 || M==11) { //小月(30天)
			if (D==31) {
				M+=1;D=1;
			} //月份进位
		} else { //大月(31天)
			if (D==32) {
				M+=1;D=1;
			} //月份进位
		}
	}

	if (M==13) {
		Y+=1;M=1;
	} //年份进位

	//setInterval(ctime,1000);
	setTimeout("ctime()", 1000);
//	set_value('cur_time', 'ada');
//	document.system_time.cur_time.value = Y+'-'+M+'-'+D+' '+H+':'+I+':'+S;
	document.getElementById("currenttime").innerHTML = Y+'-'+M+'-'+D+' '+H+':'+I+':'+S;
}

var sec2=<?php echo $start_time?>;
function utime() {
	sec2++;
	day=Math.floor(sec2/3600/24);
	hour=Math.floor(sec2/3600)%24;
	minute=Math.floor(sec2/60)%60;
	second=sec2%60;
	if(hour<10) hour = '0' + hour;
	if(minute<10) minute = '0' + minute;
	if(second<10) second = '0' + second;

	//setInterval(utime,1000);
	document.getElementById("uptime").innerHTML = day+' days  '+hour+':'+minute+':'+second; //= hour+':'+minute+':'+second;
	setTimeout("utime()", 1000);
}

function onload_func()
{
	ctime();
	utime();
}

$(document).ready(function(){
	onload_func();
});
</script>

<?php
}
?>


<?php require("/www/cgi-bin/inc/boot.inc");?>
