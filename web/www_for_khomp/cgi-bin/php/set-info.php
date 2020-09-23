<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
require_once("/www/cgi-bin/inc/aql.php");
?>

<script type="text/javascript" src="/js/functions.js"></script>
<link rel="stylesheet"  href="/css/zoomify.css" />
<script type="text/javascript" src="/js/zoomify.js"></script>

<?php 
function save_to_gw_info(){
	if(!file_exists('/www/data/info/gw_info.conf')){
		exec('touch /www/data/info/gw_info.conf');
	}
	
	$aql = new aql();
	$aql->set('basedir', '/www/data/info/');
	$hlock = lock_file('/www/data/info/gw_info.conf');
	$general_section = $aql->query("select * from gw_info.conf");
	if(!isset($general_section['general'])){
		$aql->assign_addsection('general','');
	}
	
	if(isset($_POST['switch'])){
		$switch = 'on';
	}else{
		$switch = 'off';
	}
	$product_name = $_POST['product_name'];
	$address = $_POST['address'];
	$tel = $_POST['tel'];
	$fax = $_POST['fax'];
	$email = $_POST['email'];
	$web_site = $_POST['web_site'];
	
	$copyright_val = $_POST['copyright'];
	$temp = explode("\n",$copyright_val);
	$copyright = '';
	for($i=0;$i<count($temp);$i++){
		if($i != (count($temp)-1)){
			$copyright .= $temp[$i].'<br/>';
		}else{
			$copyright .= $temp[$i];
		}
	}
	
	$img_dir = '/www/data/info/images/';
	if(!is_dir($img_dir)){
		mkdir($img_dir);
	}
	
	if(!(isset($_FILES['header_image']['size'])) || $_FILES['header_image']['size'] > 2*1024*1024) {//Max file size 2M
		echo language('Image Upload error', 'Your Image file was larger than 2M!<br>Uploading failed.');
		return;
	}
	if(!(isset($_FILES['footer_image']['size'])) || $_FILES['footer_image']['size'] > 2*1024*1024) {//Max file size 2M
		echo language('Image Upload Filesize error', 'Your Image file was larger than 2M!<br>Uploading failed.');
		return;
	}
	$header_path = $img_dir.'header_image.png';
	move_uploaded_file($_FILES['header_image']['tmp_name'], $header_path);
	$footer_path = $img_dir.'footer_image.png';
	move_uploaded_file($_FILES['footer_image']['tmp_name'], $footer_path);
	
	if(isset($general_section['general']['switch'])){
		$aql->assign_editkey('general','switch',$switch);
	}else{
		$aql->assign_append('general','switch',$switch);
	}
	
	if(isset($general_section['general']['product_name'])){
		$aql->assign_editkey('general','product_name',$product_name);
	}else{
		$aql->assign_append('general','product_name',$product_name);
	}
	
	if(isset($general_section['general']['address'])){
		$aql->assign_editkey('general','address',$address);
	}else{
		$aql->assign_append('general','address',$address);
	}
	
	if(isset($general_section['general']['tel'])){
		$aql->assign_editkey('general','tel',$tel);
	}else{
		$aql->assign_append('general','tel',$tel);
	}
	
	if(isset($general_section['general']['fax'])){
		$aql->assign_editkey('general','fax',$fax);
	}else{
		$aql->assign_append('general','fax',$fax);
	}
	
	if(isset($general_section['general']['email'])){
		$aql->assign_editkey('general','email',$email);
	}else{
		$aql->assign_append('general','email',$email);
	}
	
	if(isset($general_section['general']['web_site'])){
		$aql->assign_editkey('general','web_site',$web_site);
	}else{
		$aql->assign_append('general','web_site',$web_site);
	}
	
	if(isset($general_section['general']['copyright'])){
		$aql->assign_editkey('general','copyright',$copyright);
	}else{
		$aql->assign_append('general','copyright',$copyright);
	}
	
	if($_FILES['header_image']['tmp_name'] != ''){
		if(isset($general_section['general']['header_image'])){
			$aql->assign_editkey('general','header_image','/data/info/images/header_image.png');
		}else{
			$aql->assign_append('general','header_image','/data/info/images/header_image.png');
		}
	}
	
	if($_FILES['footer_image']['tmp_name'] != ''){
		if(isset($general_section['general']['footer_image'])){
			$aql->assign_editkey('general','footer_image','/data/info/images/footer_image.png');
		}else{
			$aql->assign_append('general','footer_image','/data/info/images/footer_image.png');
		}
	}
	
	$aql->save_config_file('gw_info.conf');
	unlock_file($hlock);
}

function download_sample_images(){
	$header_path = "bg.jpeg";
	$footer_path = "boot.jpeg";
	
	$download_file = "/www/images/Sample_Images.tar.gz";
	$pack_cmd = "tar -C /www/images -cvf $download_file $header_path $footer_path";
	exec("$pack_cmd > /dev/null 2>&1 || echo $?", $output);
	
	if($output){
		echo "</br>$download_file ";
		echo language("Packing was failed");echo "</br>";
		echo language("Error code");echo ": ".$output[0];
		return;
	}

	if(!file_exists($download_file)) {
		echo "</br>$download_file";
		echo language("Can not find");
		return;
	}
	
	$file = fopen ($download_file, "r" );
	$size = filesize($download_file);
	
	header('Content-Encoding: none');
	header('Content-Type: application/force-download');
	header('Content-Type: application/octet-stream');
	header('Content-Type: application/download');
	header('Content-Description: File Transfer');
	header('Accept-Ranges: bytes');
	header("Accept-Length: $size");
	header('Content-Transfer-Encoding: binary');
	header("Content-Disposition: attachment; filename=Sample_Images.tar.gz");
	header('Pragma: no-cache');
	header('Expires: 0');
	ob_clean();
	flush();
	echo fread($file, $size);
	fclose ($file);

	unlink($download_file);
}

if(isset($_POST['send']) && $_POST['send'] == 'Save'){
	save_to_gw_info();
	
	save_user_record("","SYSTEM->Set Info:Save");
}else if($_POST['send'] == 'Download'){
	download_sample_images();
}


$aql = new aql();
$setok = $aql->set('basedir','/www/data/info');
if (!$setok) {
	exit(255);
}

$hlock=lock_file("/www/data/info/gw_info.conf");
$general_conf = $aql->query("select * from gw_info.conf where section='general'");
unlock_file($hlock);

if($general_conf['general']['switch'] == 'on'){
	$switch = 'checked';
}else{
	$switch = '';
}
$product_name = $general_conf['general']['product_name'];
$address = $general_conf['general']['address'];
$tel = $general_conf['general']['tel'];
$fax = $general_conf['general']['fax'];
$email = $general_conf['general']['email'];
$web_site = $general_conf['general']['web_site'];
$copyright_val = $general_conf['general']['copyright'];
$copyright = str_replace("<br/>","",$copyright_val);

if($general_conf['general']['switch'] == 'on' && isset($general_conf['general']['header_image']) && $general_conf['general']['header_image'] != ''){
	$header_image = $general_conf['general']['header_image'];
}else{
	$header_image = '/images/bg.jpeg';
}

if($general_conf['general']['switch'] == 'on' && isset($general_conf['general']['footer_image']) && $general_conf['general']['footer_image'] != ''){
	$footer_image = $general_conf['general']['footer_image'];
}else{
	$footer_image = '/images/boot.jpeg';
}

?>
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">

	<div id="tab">
		<li class="tb_unfold" onclick="lud(this,'tab_networking')" id="tab_networking_li">&nbsp;</li>
		<li class="tbg_fold" onclick="lud(this,'tab_networking')"><?php echo language('Set Information');?></li>
		<li class="tb2_fold" onclick="lud(this,'tab_networking')">&nbsp;</li>
		<li class="tb_end">&nbsp;</li>
	</div>

	<div id="tab_networking" style="display:block">
		<table width="98%" class="tedit" align="right">
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Switch','Switch');?>:
						<span class="showhelp">
							<?php echo language('Switch help','If this switch is enabled, the following parameter information will be used.');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="switch" id="switch" <?php echo $switch;?> />
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Product Name');?>:
						<span class="showhelp">
							<?php echo language('Product Name help','Product Name');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="product_name" id="product_name" value="<?php echo $product_name;?>" />
				</td>
			</tr>
		
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Contact Address');?>:
						<span class="showhelp">
						<?php echo language('Contact Address help','Contact Address');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="address" value="<?php echo $address;?>" style="width:600px;"/>
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Tel');?>:
						<span class="showhelp">
						<?php echo language('Tel help','Tel');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="tel" value="<?php echo $tel;?>" />
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Fax');?>:
						<span class="showhelp">
						<?php echo language('Fax help','Fax');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" id="fax" name="fax" value="<?php echo $fax;?>" />
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('E-Mail');?>:
						<span class="showhelp">
						<?php echo language('E-Mail help','E-Mail');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" id="email" name="email" value="<?php echo $email;?>" />
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Web Site');?>:
						<span class="showhelp">
						<?php echo language('Web Site help','Web Site');?>
						</span>
					</div>
				</th>
				<td >
					<input type="text" id="web_site" name="web_site" value="<?php echo $web_site;?>" />
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Copyright');?>:
						<span class="showhelp">
						<?php echo language('Copyright help','Copyright');?>
						</span>
					</div>
				</th>
				<td>
					<textarea id="copyright" name="copyright" style="width:600px;height:100px;"><?php echo $copyright;?></textarea>
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Header Image');?>(1280x417):
						<span class="showhelp">
						<?php echo language('Header Image help','Header background picture, the size of the picture is 1280x417, can be seen in the following case picture.');?>
						</span>
					</div>
				</th>
				<td class="zoom_img" style="height:168px;">
					<input type="file" id="header_image" name="header_image" onchange="HeaderImg_change();"/>
					<img id="imgheader" src="<?php echo $header_image.'?v='.rand();?>" height="168" width="512" />
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Footer Image');?>(1280x105):
						<span class="showhelp">
						<?php echo language('Footer Image help','Footer background picture, the size of the picture is 1280x105, can be seen in the following case picture.');?>
						</span>
					</div>
				</th>
				<td class="zoom_img" style="height:53px;">
					<input type="file" id="footer_image" name="footer_image" onchange="FooterImg_change();" />
					<img id="imgfooter" src="<?php echo $footer_image.'?v='.rand();?>" height="53" width="640" />
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Download Sample Images');?>:
						<span class="showhelp">
						<?php echo language('Download Sample Images help','Download the case picture and modify it according to the size of the picture.');?>
						</span>
					</div>
				</th>
				<td style="height:53px;">
					<button onclick="document.getElementById('send').value='Download'"><?php echo language('Download Sample Images');?></button>
				</td>
			</tr>
		</table>
		
		<script>
		$(function(){
			$("#switch").iButton();
		});
		
		function HeaderImg_change(){
			var header_image = document.getElementById('header_image');
			var imgUrl = window.URL.createObjectURL(header_image.files[0]);
			var imgheader = document.getElementById('imgheader');
			imgheader.setAttribute('src',imgUrl);
		}
		function FooterImg_change(){
			var footer_image = document.getElementById('footer_image');
			var imgUrl = window.URL.createObjectURL(footer_image.files[0]);
			var imgfooter = document.getElementById('imgfooter');
			imgfooter.setAttribute('src',imgUrl);
		}
		
		$(".zoom_img img").zoomify();
		
		function check(){
			
		}
		</script>
		
		<div id="newline"></div>
		
		<br>

		<input type="hidden" name="send" id="send" value="" />
		<input type="submit" class="gen_short_btn float_btn"   value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';return check();"/>
	</div>
</form>

<?php require("/www/cgi-bin/inc/boot.inc");?>
