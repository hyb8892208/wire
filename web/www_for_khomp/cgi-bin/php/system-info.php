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
//$all_board_info = read_redis_info();

function get_hw_ver(){
	$file_name = '/tmp/hw_info.cfg';
	$hw_ver = exec("/my_tools/set_config $file_name get option_value sys hw_ver");
	return $hw_ver;
}

$aql = new aql();
$setok = $aql->set('basedir','/www/data/info');
if (!$setok) {
	exit(255);
}

$hlock=lock_file("/www/data/info/gw_info.conf");
$general_conf = $aql->query("select * from gw_info.conf where section='general'");
unlock_file($hlock);

$switch = $general_conf['general']['switch'];

if($switch == 'on'){
	$product_name = $general_conf['general']['product_name'];
	$address = $general_conf['general']['address'];
	$tel = $general_conf['general']['tel'];
	$fax = $general_conf['general']['fax'];
	$email = $general_conf['general']['email'];
	$web_site = $general_conf['general']['web_site'];
}else{
	$product_name = get_model_name();
	$address = language('Contact Address Contents','Khomp - Ind��stria Brasileira');
	// $tel = '+86-755-82535461';
	// $fax = '+86-755-83823074';
	// $email = 'support@openvox.cn';
	$tel = '';
	$fax = '';
	$email = '';
	
	$language_type = get_language_type();
	if($language_type == "english"){
		//$web_site = "http://www.openvox.cn";
	} else{
		//$web_site = "http://openvox.com.cn";
	}
	$web_site = '';
}
$info_display = 'style="display:none;"';

function get_emu_version(){
	
	$aql = new aql();
	$aql->set('basedir', '/tmp/');
	$res = $aql->query("select * from hw_info.cfg");
	
	$content = file_get_contents("/tmp/.emu_status");
	$temp = explode("\n",$content);
	
	$con_arr = [];
	for($i=0;$i<count($temp);$i++){
		if(strstr($temp[$i],'emu')){
			array_push($con_arr,$temp[$i]);
		}
	}
	
	$str = '';
	$n = 1;
	for($i=0;$i<count($res['emu']);$i++){
		if(strstr($con_arr[$i],'DETECTED') && !strstr($con_arr[$i],'UNDETECTED')){
			$tmp = explode("version:",$con_arr[$i]);
			$tmp1 = explode(" ",$tmp[1]);
			$tmp_str = $tmp1[1].' '.$tmp1[2].' '.$tmp1[3].' '.$tmp1[4];
			
			$str .= '<span style="heigt:18px;">emu-'.$n.' '.$tmp_str.'</span><br/>';
		}
		
		if(strstr($con_arr[$i],'UNDETECTED')){
			$str .= '<span style="heigt:18px;">emu-'.$n.' ----</span><br/>';
		}
		$n++;
	}
	
	return $str;
}

?>

	<div class="content">
		<div class="tab_item" <?php if($product_name == '') echo $info_display;?>>
			<span>
				<?php echo language('Product Name');?>:
			</span>
			<div class="tab_item_right">
				<span><?php echo $product_name;?></span>
			</div>
		</div>
		
		<?php echo get_module_model_for_customized();?>

		<?php 
		$product_type = exec("/my_tools/set_config /tmp/hw_info.cfg get option_value sys product_type");
		if($product_type == 5 || $product_type == 6){
		?>
		<div class="tab_item">
			<span>EMU:</span>
			<div class="tab_item_right">
				<span><?php echo get_emu_version(); ?></span>
			</div>
		</div>
		<?php } ?>
		
				
		<div class="tab_item">
			<span>
				<?php echo language('Software Version');?>:
			</span>
			<div class="tab_item_right">
				<span><?php if(file_exists('/version/version')) readfile('/version/version'); ?></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Hardware Version');?>:
			</span>
			<div class="tab_item_right">
				<span><?php echo get_hw_ver(); ?></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Slot Number');?>:
			</span>
			<div class="tab_item_right">
				<span><?php echo `cat /tmp/.slotnum 2> /dev/null` ?></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Storage Usage');?>:
			</span>
			<div class="tab_item_right">
				<span><?php printData(); ?></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Memory Usage');?>:
			</span>
			<div class="tab_item_right">
				<span><?php printMem(); ?>
				<a href="<?php echo get_self() ?>?memory_clean=yes"><?php echo language('Memory Clean');?></a></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('Build Time');?>:
			</span>
			<div class="tab_item_right">
				<span>
				<?php 
					if(file_exists('/version/build_time')) {
						$build_time_val = file_get_contents('/version/build_time');
						echo date("d/m/Y H:i:s",strtotime($build_time_val));
					}
				?>
				</span>
			</div>
		</div>
		
		<?php if($__system_type__ == 'openvox'){ ?>
			<div class="tab_item" <?php if($address == '') echo $info_display;?>>
				<span>
					<?php echo language('Contact Address');?>:
				</span>
				<div class="tab_item_right">
					<span><?php echo $address;?></span>
				</div>
			</div>
			
			<div class="tab_item"  <?php if($tel == '') echo $info_display;?>>
				<span>
					<?php echo language('Tel');?>:
				</span>
				<div class="tab_item_right">
					<span><?php echo $tel;?></span>
				</div>
			</div>
			
			<div class="tab_item" <?php if($fax == '') echo $info_display;?>>
				<span>
					<?php echo language('Fax');?>:
				</span>
				<div class="tab_item_right">
					<span><?php echo $fax;?></span>
				</div>
			</div>
			
			<div class="tab_item" <?php if($email == '') echo $info_display;?>>
				<span>
					<?php echo language('E-Mail');?>:
				</span>
				<div class="tab_item_right">
					<span><a href="mailto:<?php echo $email;?>"><?php echo $email;?></a></span>
				</div>
			</div>
			
			<div class="tab_item" <?php if($web_site == '') echo $info_display;?>>
				<span>
					<?php echo language('Web Site');?>:
				</span>
				<div class="tab_item_right">
					<span><a href="<?php echo $web_site;?>" target="_top"><?php echo $web_site;?></a></span>
				</div>
			</div>
			
		<?php } ?>
		
		<div class="tab_item">
			<span>
				<?php echo language('Rebooting Counts');?>:
			</span>
			<div class="tab_item_right">
				<span><?php if(file_exists('/data/info/rtimes')) readfile('/data/info/rtimes'); ?></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('System Time');?>:
			</span>
			<div class="tab_item_right">
				<span id="currenttime"></span>
			</div>
		</div>
		
		<div class="tab_item">
			<span>
				<?php echo language('System Uptime');?>:
			</span>
			<div class="tab_item_right">
				<span id="uptime"></span>
			</div>
		</div>
	</div>
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
	if (H=='00' & I=='00' & S=='00') D=D+1; //�ս�λ
	if (M==2) { //�ж��Ƿ�Ϊ���·�******
		if (Y%4==0 && !Y%100==0 || Y%400==0) { //������(������28��)
			if (D==30){
				M+=1;D=1;
			} //�·ݽ�λ
		} else { //������(������29��)
			if (D==29) {
				M+=1;D=1;
			} //�·ݽ�λ
		}
	} else { //���Ƕ��·ݵ��·�******
		if (M==4 || M==6 || M==9 || M==11) { //С��(30��)
			if (D==31) {
				M+=1;D=1;
			} //�·ݽ�λ
		} else { //����(31��)
			if (D==32) {
				M+=1;D=1;
			} //�·ݽ�λ
		}
	}

	if (M==13) {
		Y+=1;M=1;
	} //��ݽ�λ

	//setInterval(ctime,1000);
	setTimeout("ctime()", 1000);
//	set_value('cur_time', 'ada');
//	document.system_time.cur_time.value = Y+'-'+M+'-'+D+' '+H+':'+I+':'+S;
	//document.getElementById("currenttime").innerHTML = Y+'-'+M+'-'+D+' '+H+':'+I+':'+S;
	document.getElementById("currenttime").innerHTML = D+'/'+M+'/'+Y+' '+H+':'+I+':'+S;
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
