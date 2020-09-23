<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
<meta charset="utf-8">
<title>Openvox License Management</title>
<style type="text/css">
	body {
            margin:0;
            padding:0;
            background:white url('images/header-background.png') repeat-x;
            font:12px/1.6 arial;	
	}
        ul {
            margin:0;
            padding:0;
            list-style:none;
        }
        a{
            text-decoration:none;
            color:#3D81B4;
        }
	p{
	    font-size:120%;
	    color:#5897C6;
	}

        .header{
            position:relative;
            width:760px;
            height:138px;
            margin:0 auto;
            font:14px/1.6 arial;
        }
        .header h1{
	    color:#5897C6;
	    height:63px;
	    margin:20px 30px 0 0;
	    text-align:center;	
        }
        .header .mainNavigation{
            position:absolute;
            color:white;
            top:68px;
            left:185px;
        }
        .header .mainNavigation li{
            float:left;
            padding:5px;
        }
        .header .mainNavigation li a {
            display: block;
            line-height: 25px;
            padding: 0 0 0 14px;
            color: white;
            float: left;
        } 
        .header .mainNavigation a strong{
            display:block;
            padding:0 14px 0 0;
	    font-size:120%;
        }
        .header .mainNavigation .current a{
            background:transparent url('images/main-navi.gif') no-repeat;
        }
        .header .mainNavigation .current a strong{
            background:transparent url('images/main-navi.gif') no-repeat right;
        }
        .header .mainNavigation a:hover{
        		color:white;
            background:transparent url('images/main-navi-hover.gif') no-repeat;
        }
        .header .mainNavigation a:hover strong{
            background:transparent url('images/main-navi-hover.gif') no-repeat right;
        }

	.footer {
            clear:both;
            height:53px;
            margin:100px 0 0 0;
            background:transparent url('images/footer-background.png') repeat-x;
            text-indent: 0px;
            text-align:center;
	}
	.license {margin: 0 auto;width:80%}
	.gen1 {text-align:right;float:left;width:48%;margin-top:20px;margin-bottom:20px;padding-right:20px;border-right:#BDD6E8 1px solid}
	.gen2 {text-align:left;float:left;width:48%;margin-top:20px;padding-left:20px}
			
	.gen3 {
	     text-align:center;
             clear:both;
         }
	.th1 {padding-right:60px}
	.th2 {padding-left:80px}
	textarea {resize: none;}
	.content {
		height:570px;
		margin: 0 auto;
		text-align:center;
		width:80%
	}
</style>
<script type="text/javascript" src="jquery.js"></script>
<script type="text/javascript">
function check(){
	if($("#uuid").val() == ""){
		alert("UUID 不能为空");
		return false;
	}
		
	return true;
}
</script>
</head>
<body>
        <div class="header">
        	<h1><span>Openvox PBX 许可证管理系统</span></h1>
        	<ul class="mainNavigation">
        		<li><a href="index.html"><strong>生成</strong></a></li>
				<li><a href="analyze.php"><strong>解析</strong></a></li>
        		<li><a href="query.php" ><strong>查询</strong></a></li>
        		<li><a href="load.php"><strong>添加</strong></a></li>
        		<li><a href="delete.php"><strong>删除</strong></a></li>
				<li class="current"><a href="createuuid.php"><strong>生成UUID</strong></a></li>
        	</ul>
    	</div>
	<form id="manform"  action="/createuuid.php" method="post" onsubmit="return check();">
	<div class="content">
		<div class="gen3">
			<p>UUID:<p>
			<textarea cols="30" rows="30" name="uuid" id="uuid"></textarea>
		</div>
		<div class="gen3">	
			<input type="submit" name="submit" value="生成UUIDKEY" />
			&nbsp;
			<input type="reset"  name="reset" value="重置" />
		</div>
	</div>
	</form>
	<div class="footer">
		<p>&nbsp;</p>
	</div>
</body>
</html>

<?php 
function generate_uuid(){
	$uuid = $_POST['uuid'];
	$temp = explode("\n", $uuid);
	
	exec("rm -rf /tmp/uuid*");
	
	foreach($temp as $item){
		exec("cd /tmp/ && encSerial -s=$item");
	}
	
	$uuid_name = "uuidKey.tar.gz";
	exec("cd /tmp && tar -zcf /tmp/$uuid_name uuidKey*.bin");
	
	$uuid_path = "/tmp/$uuid_name";
	
	$file = fopen($uuid_path, "r");
	$size = filesize($uuid_path);
	
	header('Content-Encoding: none');
    header('Content-Type: application/force-download');
    header('Content-Type: application/octet-stream');
    header('Content-Type: application/download');
    header('Content-Description: File Transfer');
    header('Accept-Ranges: bytes');
    header( "Accept-Length: $size");
    header( 'Content-Transfer-Encoding: binary' );
    header( "Content-Disposition: attachment; filename=$uuid_name" );
    header('Pragma: no-cache');
    header('Expires: 0');
	
	ob_clean();
	flush();
	echo fread($file, $size);
	fclose($file);
	
	unlink(uuid_path);
}

if(isset($_POST["submit"])){
	generate_uuid();
}

?>