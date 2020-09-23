<?php
date_default_timezone_set("PRC");
function my_exit($link,$msg,$code) {
        mysql_query("unlock tables",$link);
        mysql_close($link);
        echo $msg;
        exit($code);
}
function insert() {
	if(isset($_POST) && $_POST[submit] == "添加") {
                $link = mysql_connect("127.0.0.1","root","root") or die("Can not connect to DATABASE...".mysql_error());
                $db = mysql_select_db("openvox",$link);

                if(!$db) {
                        my_exit($link,"DATABASE ERROR:Can not select table!<br>".mysql_error(),1);
                }
                $result = mysql_query("lock tables macUuid write",$link); // Mutex : willbe unlock when exit
                if(!$result) {
                        my_exit($link,"DATABASE ERROR:Can not lock tables!<br>".mysql_error(),1);
                }
                $mac = trim($_POST[mac]);
                $uuid = trim($_POST[mcuid]);
		$mactable = "mac_".$mac;
		$uuidtable = "uuid_".$uuid;
		if(strlen($uuid) == 0) 
			$uuid = null;
	
		$cmd = "insert into macUuid(MAC,UUID) values('$mac','$uuid')";
                $result = mysql_query($cmd,$link);
                if(!$result) {
                	my_exit($link,"DATABASE ERROR:Can not insert record to table!<br>".mysql_error(),1);
               	}
		mysql_query("unlock tables",$link);

		$cmd = "create table $mactable(TIME char(20), OPRT char(24), UUID char(24) , PRIMARY KEY (TIME))";		
		$result = mysql_query($cmd,$link);
                if(!$result && (mysql_errno() != 1050)) {
                        my_exit($link,"DATABASE ERROR:Can not create mac table!<br>".mysql_error().":".mysql_errno(),1);
                }
                mysql_query("lock tables $mactable write",$link); // Mutex : willbe unlock when exit
                $time = date("Y-m-d H:i:s");

                $cmd = "insert into $mactable(TIME,OPRT,UUID) values('$time','insert','$uuid')";
                $result = mysql_query($cmd,$link);
                if(!$result) {
                	my_exit($link,"DATABASE ERROR:Can not insert record to table!<br>".mysql_error(),1);
                }
		if($uuid != null) {
			mysql_query("unlock tables",$link);
                	$cmd = "create table $uuidtable(TIME char(20), OPRT char(24), MAC char(12) , PRIMARY KEY (TIME))"; 
                	$result = mysql_query($cmd,$link);
                	if(!$result && mysql_errno() != 1050) {
                        	my_exit($link,"DATABASE ERROR:Can not create mcuid table!<br>".mysql_error(),1);
			}			
			 mysql_query("lock tables $uuidtable write",$link); // Mutex : willbe unlock when exit
			$cmd = "insert into $uuidtable(TIME,OPRT,MAC) values('$time','insert','$mac')";
                	$result = mysql_query($cmd,$link);
                	if(!$result) {
                        	my_exit($link,"DATABASE ERROR:Can not insert record to table!<br>".mysql_error(),1);
                	}
			
                }
		my_exit($link,"Insert Success!",0);
	}else if(isset($_POST) && $_POST[submit] == "上传"){
        	if(isset($_FILES['file']['error']) && $_FILES['file']['error'] == 0) {  //Update successful
                	$store_file="/srv/www/htdocs/macuuid.txt";
			if (!move_uploaded_file($_FILES['file']['tmp_name'], $store_file)) {
				echo "Moving your updated file was failed!<br>Uploading MAC/MCUID Table File was failed.";
				return;
			}
                	$link = mysql_connect("127.0.0.1","root","root") or die("Can not connect to DATABASE...".mysql_error());
                	$db = mysql_select_db("openvox",$link);

                	if(!$db) {
                        	my_exit($link,"DATABASE ERROR:Can not select table!<br>".mysql_error(),1);
                	}
                	$result = mysql_query("lock tables macUuid write",$link); // Mutex : willbe unlock when exit
                	if(!$result) {
                        	my_exit($link,"DATABASE ERROR:Can not lock tables!<br>".mysql_error(),1);
                	}
                	$cmd = "load data local infile '$store_file' into TABLE macUuid;";
                	$result = mysql_query($cmd,$link);
                	if(!$result) {
                        	my_exit($link,"DATABASE ERROR:Can not load record to table!<br>".mysql_error(),1);
                	}
						
                        $cmd = "select * from macUuid";
                        $macUuids = mysql_query($cmd,$link);
                        if(!$macUuids) {
                                my_exit($link,"DATABASE ERROR:Can not query record from table!<br>".mysql_error(),1);
                        }
			while($a_macuuid = mysql_fetch_array($macUuids)){
			
				$mac = $a_macuuid[MAC];
				$uuid = $a_macuuid[UUID];
				$mactable = "mac_".$mac;
				$uuidtable = "uuid_".$uuid;
			        if(strlen($uuid) == 0)
                        		$uuid = null;	
                		mysql_query("unlock tables",$link);

                		$cmd = "create table $mactable(TIME char(20), OPRT char(24), UUID char(24) , PRIMARY KEY (TIME))";
                		$result = mysql_query($cmd,$link);
                		if(!$result && (mysql_errno() != 1050)) {
                        		my_exit($link,"DATABASE ERROR:Can not create mac table!<br>".mysql_error().":".mysql_errno(),1);
                		}
                		mysql_query("lock tables $mactable write",$link); // Mutex : willbe unlock when exit
                		$time = date("Y-m-d H:i:s");

                		$cmd = "insert into $mactable(TIME,OPRT,UUID) values('$time','create','$uuid')";
                		$result = mysql_query($cmd,$link);
                		if(!$result) {
                        		my_exit($link,"DATABASE ERROR:Can not insert record to table!<br>".mysql_error(),1);
                		}
                		if($uuid != null) {
                        		mysql_query("unlock tables",$link);
                        		$cmd = "create table $uuidtable(TIME char(20), OPRT char(24), MAC char(12) , PRIMARY KEY (TIME))";
                        		$result = mysql_query($cmd,$link);
                        		if(!$result && mysql_errno() != 1050) {
                                		my_exit($link,"DATABASE ERROR:Can not create mcuid table!<br>".mysql_error(),1);
                        		}
                        		mysql_query("lock tables $uuidtable write",$link); // Mutex : willbe unlock when exit
                        		$cmd = "insert into $uuidtable(TIME,OPRT,MAC) values('$time','create','$mac')";
                        		$result = mysql_query($cmd,$link);
                        		if(!$result) {
                                		my_exit($link,"DATABASE ERROR:Can not insert record to table!<br>".mysql_error(),1);
                        		}
				}

                	}
			
			my_exit($link,"Load MAC/MCUID Table File Success!",0);			
			
		} else {
				echo "Uploading MAC/MCUID Table File was failed.";
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
	.con1 {
		text-align:right;
		float:left;
		width:48%;
		margin-top:20px;
		padding-right:20px;
	<!--	border-right:#BDD6E8 1px solid -->
	}
	.con2 { 
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

$("#mac").keyup(function() {
	$("#result").text("");	
});
$("#uuid").keyup(function() {
        $("#result").text("");
});
$("#file").change(function() {
        $("#result").text("");
});

$("#manform2").submit(function(){
	if($("#mac").val() == "") {
		alert("MAC should not be null");
		return false;
	}
	
	var o_mac = $("#mac").val();
	var mac = $.trim(o_mac);
        var o_mcuid = $("#mcuid").val();
        var mcuid = $.trim(o_mcuid);

	if(mac.length != 12) {
		alert("The length of the MAC should be 12!");
		return false;
	}
	if(mcuid.length !=0 && mcuid.length != 24) {
		alert("The length of the MCUID should be 24!");
                return false;
	}
	if(mac.length == 12) {	
		var patt = new RegExp('[0123456789abcdefABCDEF]{12}')
		if(mac.match(patt)==null) {
			alert("The content of the MAC is not right, shoule be 012345689abcdefABCDEF!")
                        return false;
		}
	}
	if(mcuid.length == 24){
                var patt = new RegExp('[0123456789abcdefABCDEF]{24}')
                if(mcuid.match(patt)==null) {
                        alert("The content of the MACUID is not right, shoule be 012345689abcdefABCDEF!")
                        return false;
                }
	
	}
	return true;
});

});
</script>
</head>
<body>
        <div class="header">
        	<h1><span>Openvox PBX 许可证管理系统</span></h1>
        	<ul class="mainNavigation">
        		<li><a href="index.html"><strong>生成</strong></a></li>
        		<li><a href="analyze.php"><strong>解析</strong></a></li>
			<li><a href="query.php" ><strong>查询</strong></a></li>
        		<li class="current"><a href="load.php"><strong>添加</strong></a></li>
        		<li><a href="delete.php"><strong>删除</strong></a></li>
				<li><a href="createuuid.php"><strong>生成UUID</strong></a></li>
        	</ul>
    	</div>
	<div class="content">
		<form id="manform1"  action="/load.php" method="post" enctype="multipart/form-data" >
                        <p>
                                上传MAC/MCUID表文件: <input type="file" name="file" id="file">&nbsp;&nbsp;
                                <input type="submit" name="submit" value="上传" />
                	</p>
                </form>
		<form id="manform2"  action="/load.php" method="post" >
			<p>
				MAC: <input type="text" name="mac" id="mac" maxlength="12" size="14" value=<?php echo $_POST[mac]; ?>>&nbsp;&nbsp;
				MCUID: <input type="text" name="mcuid" id="mcuid" maxlength="24" size="26" value=<?php echo $_POST[mcuid]; ?>>&nbsp;&nbsp;
				<input type="submit" name="submit" value="添加" />
			</p>
		</form>
		
			<p id="result">
			<?php insert()  ?>
			</p>
	
	</div>
	<div class="footer">
		<p>&nbsp;</p>
	</div>
</body>
</html>
