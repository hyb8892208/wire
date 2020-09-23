<?php
$equal = 1;
$a_lic = array();
$a_mac = array();
function analyze() {
	global $equal;
	global $a_lic;
	global $a_mac;
	if(isset($_POST) && $_POST[submit] == "解析") {
		if(isset($_FILES['file']['error']) && $_FILES['file']['error'] == 0) {  //Update successful
                	$store_file="/data/crt/license.crt";
                        if (!move_uploaded_file($_FILES['file']['tmp_name'], $store_file)) {
                                echo "Moving your license file was failed!<br>Uploading License File was failed.";
                                return;
                        }
		 	exec("cd /data/crt/ && tar -zxvf /data/crt/license.crt",$output,$return);
			if($return != 0){
                        	echo "Error:Can not analyze license file!!!";
                        	echo "</br>";
                        	echo "Error code - tar 1 error";
                        	echo ": ".$output[0];
                        	exec("rm -rf /data/crt/* && rm -rf /data/license/license* && rm -rf /data/license/serial*");
                        	return;
			}
			exec("rm -rf /data/crt/license.crt");
				
			$lics=scandir("/data/crt");
			if(count($lics) < 3){
                        	echo "Error:Can not analyze license file!!!";
                              	echo "</br>";
                               	echo "Error code - license is null";
                               	exec("rm -rf /data/crt/* && rm -rf /data/license/license* && rm -rf /data/license/serial*");
                               	return;
			}
				
			foreach($lics as $lic) {
                                if ($lic == "." || $lic == "..")
					continue;
				$cmd = "cd /data/crt/ && mv $lic /data/crt/license.crt && tar -zxvf /data/crt/license.crt -C /data/license";
				exec($cmd,$output,$return);
				if($return != 0){
                                	echo "Error:Can not analyze license file!!!";
                                	echo "</br>";
                                	echo "Error code - tar 2 error";
                                	echo ": ".$output[0];
                                	exec("rm -rf /data/crt/* && rm -rf /data/license/license* && rm -rf /data/license/serial*");
                                	return;
                        	}
				if(!file_exists("/bin/anlslicense")){
				{
                                        echo "Error:Can not analyze license file!!!";
                                        echo "</br>";
                                        echo "Error code - Can not find analyze script";
                                        exec("rm -rf /data/crt/* && rm -rf /data/license/license* && rm -rf /data/license/serial*");
                                        return;
                                }
				}
					
				exec("/bin/anlslicense",$output,$return);
                        	if($return == -1) {
                                	echo "</br>";
                                	echo "Error:Can not analyze license file!!!";
                                	echo "</br>";
                                	echo "Error code";
                                	echo ": ".$output[0];
					exec("rm -rf /data/crt/* && rm -rf /data/license/license* && rm -rf /data/license/serial*");
                                	return;
                        	}
				$licstr = file_get_contents("/data/license/license-dec");
				$licarr = json_decode($licstr, true);
				array_push($a_lic,$licarr);
				$mac = substr($lic,8,12);
				array_push($a_mac,$mac);
		
				exec("rm -rf /data/crt/license.crt");
			}
			for($i = 0; $i < (count($a_lic) - 1); $i++) {		
				if($a_lic[$i]["Sip"] != $a_lic[$i+1]["Sip"]){
					$equal = 0;
					break;
				}
                                if($a_lic[$i]["Iax"] != $a_lic[$i+1]["Iax"]){
                                        $equal = 0;
                                        break;
                                }
				if($a_lic[$i]["Cc"] != $a_lic[$i+1]["Cc"]){
                                        $equal = 0;
                                        break;
                                }
                                if($a_lic[$i]["T1"] != $a_lic[$i+1]["T1"]){
                                        $equal = 0;
                                        break;
                                }
                                if($a_lic[$i]["T2"] != $a_lic[$i+1]["T2"]){
                                        $equal = 0;
                                        break;
                                }
				if(strcmp($a_lic[$i]["R1"],$a_lic[$i+1]["R1"]) || strcmp($a_lic[$i]["R2"],$a_lic[$i+1]["R2"]) ||
				   strcmp($a_lic[$i]["R3"],$a_lic[$i+1]["R3"]) || strcmp($a_lic[$i]["R4"],$a_lic[$i+1]["R4"]) ||
                                   strcmp($a_lic[$i]["R5"],$a_lic[$i+1]["R5"]) || strcmp($a_lic[$i]["R6"],$a_lic[$i+1]["R6"]) ||
                                   strcmp($a_lic[$i]["R7"],$a_lic[$i+1]["R7"]) || strcmp($a_lic[$i]["R8"],$a_lic[$i+1]["R8"]) ) {
                                        $equal = 0;
                                        break;
				}
			}
			if($equal == 0) {
				echo "The limits of license files are different from each other!!!";
				return;
			}
		
			
			
		} else {
			echo "Uploading License File was failed.";
			return ;
		}
	}
}

?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
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
	#file{
	    border:1px solid #cdcdcd;
	}
	#result{
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
	.content {
		height:570px;
		margin: 0 auto;
		text-align:center;
		width:80%
	}
	.gen1 {
		text-align:right;
		float:left;
		width:48%;
		margin-top:20px;
		padding-right:20px;
		border-right:#BDD6E8 1px solid 
	}
	.gen2 { 
		text-align:left;
		float:left;
		width:48%;
		margin-top:35px;
		padding-left:20px
	}
			
	.con3 {
	     text-align:center;
             clear:both;
        }
	
	.th1 {
		padding-right:60px
	}
	
	.th2 {  
		padding-left:80px
	}
	
	textarea {
		resize: 
		none;
	}
</style>
<script type="text/javascript" src="jquery.js"></script>
<script type="text/javascript">
$(document).ready(function(){

$("#file").change(function() {
	$("#result").text(" ");	
});


});
</script>
</head>
<body>
        <div class="header">
        	<h1><span>Openvox PBX 许可证管理系统</span></h1>
        	<ul class="mainNavigation">
        		<li><a href="index.html"><strong>生成</strong></a></li>
        		<li class="current"><a href="analyze.php"><strong>解析</strong></a></li>
			<li><a href="query.php" ><strong>查询</strong></a></li>
        		<li><a href="load.php"><strong>添加</strong></a></li>
        		<li><a href="delete.php"><strong>删除</strong></a></li>
				<li><a href="createuuid.php"><strong>生成UUID</strong></a></li>
        	</ul>
    	</div>
	<div class="content">
		<div>
		<form id="manform"  action="/analyze.php" method="post" enctype="multipart/form-data" >
                        <p>
                                上传许可证文件: <input type="file" name="file" id="file">&nbsp;&nbsp;
                                <input type="submit" name="submit" value="解析" >
                	</p>
                </form>
		</div>
		<div id="result">
				<?php analyze()  ?>
				<?php if($equal && (count($a_lic)>=1)) {?>
                		<div class="gen1">
                        		<p class="th1">Limits:</p>
                        		<p>Sip Endpoint:<input type="number" name="sipendpoint" id="sipendpoint" min="0" value=<?php echo $a_lic[0]["Sip"];?>><p>
                        		<p>Iax Endpoint:<input type="number" name="iaxendpoint" id="iaxendpoint" min="0" value=<?php echo $a_lic[0]["Iax"];?>><p>
                        		<p>Concurrent:<input type="number" name="concurrent" id="concurrent" min="0" value=<?php echo $a_lic[0]["Cc"];?>><p>
                        		<p>Time1:<input type="number" name="time1" id="time1" min="0" value=<?php echo $a_lic[0]["T1"];?>><p>
                        		<p>Time2:<input type="number" name="time2" id="time2" min="0" value=<?php echo $a_lic[0]["T2"];?>><p>
                        		<p>Reserve1:<input type="text" name="reserve1" id="reserve1" value=<?php echo $a_lic[0]["R1"];?>><p>
                        		<p>Reserve2:<input type="text" name="reserve2" id="reserve2" value=<?php echo $a_lic[0]["R2"];?>><p>
                        		<p>Reserve3:<input type="text" name="reserve3" id="reserve3" value=<?php echo $a_lic[0]["R3"];?>><p>
                        		<p>Reserve4:<input type="text" name="reserve4" id="reserve4" value=<?php echo $a_lic[0]["R4"];?>><p>
                        		<p>Reserve5:<input type="text" name="reserve5" id="reserve5" value=<?php echo $a_lic[0]["R5"];?>><p>
                        		<p>Reserve6:<input type="text" name="reserve6" id="reserve6" value=<?php echo $a_lic[0]["R6"];?>><p>
                        		<p>Reserve7:<input type="text" name="reserve7" id="reserve7" value=<?php echo $a_lic[0]["R7"];?>><p>
                        		<p>Reserve8:<input type="text" name="reserve8" id="reserve8" value=<?php echo $a_lic[0]["R8"];?>><p>

                		</div>
                		<div class="gen2">
                        		<p class="th2">MACs:<p>
                        		<textarea cols="30" rows="30" name="macs" id="macs"><?php echo implode($a_mac,"\n");?></textarea>
                		</div>
				<?php } ?>
		</div>
	
	</div>
	<div class="footer">
		<p>&nbsp;</p>
	</div>
</body>
</html>
