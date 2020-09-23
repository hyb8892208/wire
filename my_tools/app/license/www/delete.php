<?php
date_default_timezone_set("PRC");
function my_exit($link,$msg,$code) {
	mysql_query("unlock tables",$link);
	mysql_close($link);
	echo $msg;
	exit($code);
}
function delete() {
	if(isset($_POST) && $_POST[submit] == "删除") {
        	$link = mysql_connect("127.0.0.1","root","root") or die("Can not connect to DATABASE...".mysql_error());
        	$db = mysql_select_db("openvox",$link);

        	if(!$db) {
			my_exit($link,"DATABASE ERROR:Can not select table!<br>".mysql_error(),1);
        	}
        	$result = mysql_query("lock tables macUuid write",$link); // Mutex : willbe unlock when exit
        	if(!$result) {
                	my_exit($link,"DATABASE ERROR:Can not lock tables!<br>".mysql_error(),1);
        	}
		$o_macuuid = $_POST[macuuid];
		$macuuid = trim($o_macuuid);

		if(strlen($macuuid) == 12) {
        		$result = mysql_query("select * from macUuid where MAC='".$macuuid."'",$link);
        		if(!$result) {
				my_exit($link,"DATABASE ERROR:Can not find record from table!<br>".mysql_error(),1);
        		}
        		if(mysql_num_rows($result) == 0){
				my_exit($link,"No record<br>",0);
        		}else if(mysql_num_rows($result) == 1){
                		$macobj = mysql_fetch_object($result);
              			$mac = $macobj->MAC;
				$uuid = $macobj->UUID;
                		if(strlen($mac) != 12){
                			my_exit($link,"No record<br>",0);
				}
				$mactable = "mac_".$mac;
					
        			$result = mysql_query("delete from macUuid where MAC='".$mac."'",$link);
        			if(!$result) {
					my_exit($link,"DATABASE ERROR:Can not delete record from table!<br>".mysql_error(),1);
        			}
				mysql_query("unlock tables",$link);
				mysql_query("lock tables $mactable write",$link); // Mutex : willbe unlock when exit
				$time = date("Y-m-d H:i:s");
				$cmd = "insert into $mactable(TIME,OPRT,UUID) values('$time','delete','$uuid')";
				$result = mysql_query($cmd,$link);
                        	if(!$result) {
					my_exit($link,"DATABASE ERROR:Can not insert record to table!<br>".mysql_error(),1);
                        	}
				if (strlen($uuid) == 24) {
					$uuidtable = "uuid_".$uuid;
	                                mysql_query("unlock tables",$link);
        	                        mysql_query("lock tables $uuidtable write",$link); // Mutex : willbe unlock when exit
                	                $cmd = "insert into $uuidtable(TIME,OPRT,MAC) values('$time','delete','$mac')";
                        	        $result = mysql_query($cmd,$link);
                                	if(!$result) {
                                        	my_exit($link,"DATABASE ERROR:Can not insert record to table!<br>".mysql_error(),1);
                               		}
				}
				my_exit($link,"Delete Success\n",0);	
				
			} else {
				my_exit($link,"DATABASE ERROR:There are too many records for $macuuid in table!<br>".mysql_error(),1);    
			}
		}else if(strlen($macuuid) == 24) {
			$result = mysql_query("select * from macUuid where UUID='".$macuuid."'",$link);
                        if(!$result) {
				my_exit($link,"DATABASE ERROR:Can not find record from table!<br>".mysql_error(),1);
                        }
                        if(mysql_num_rows($result) == 0){
                        	 my_exit($link,"No record!\n",0);
			}else if(mysql_num_rows($result) == 1){
                                $macobj = mysql_fetch_object($result);
				$mac = $macobj->MAC;
                                $uuid = $macobj->UUID;
                                if(strlen($uuid) != 24){
                                         my_exit($link,"No record!\n",0);
                                }
				$mactable = "mac_".$mac;
				$uuidtable = "uuid_".$uuid;
				$result = mysql_query("delete from macUuid where UUID='".$macuuid."'",$link);
                        	if(!$result) {
                        		my_exit($link,"DATABASE ERROR:Can not delete record from table!<br>".mysql_error(),1);
				}
                                mysql_query("unlock tables",$link);
                                mysql_query("lock tables $mactable write",$link); // Mutex : willbe unlock when exit

				$time = date("Y-m-d H:i:s");
                                $cmd = "insert into $mactable(TIME,OPRT,UUID) values('$time','delete','$uuid')";
                                $result = mysql_query($cmd,$link);
                                if(!$result) {
                                	my_exit($link,"DATABASE ERROR:Can not insert record to table!<br>".mysql_error(),1);
				}
                                mysql_query("unlock tables",$link);
                                mysql_query("lock tables $uuidtable write",$link); // Mutex : willbe unlock when exit
                                $cmd = "insert into $uuidtable(TIME,OPRT,MAC) values('$time','delete','$mac')";
                                $result = mysql_query($cmd,$link);
                                if(!$result) {
                                        my_exit($link,"DATABASE ERROR:Can not insert record to table!<br>".mysql_error(),1);
                                }
				my_exit($link,"Delete Success\n",0);
			}else {
				my_exit($link,"DATABASE ERROR:There are too many records for $macuuid in table!<br>".mysql_error(),1);	
			}	
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

$("#macuuid").keyup(function() {
	$("#result").text("");	
});

$("#manform").submit(function(){
	if($("#macuuid").val() == "") {
		alert("MAC/MCUID should not be null");
		return false;
	}
	
	var o_macuuid = $("#macuuid").val();
	var macuuid = $.trim(o_macuuid);

	if((macuuid.length != 12) && macuuid.length != 24) {
		alert("The length of the MAC/MCUID should be 12 or 24!");
		return false;
	}
	if(macuuid.length == 12) {	
		var patt = new RegExp('[0123456789abcdefABCDEF]{12}')
		if(mac.match(patt)==null) {
			alert("The content of the MAC/MACUID is not right, shoule be 012345689abcdefABCDEF!")
                        return false;
		}
	}else {
                var patt = new RegExp('[0123456789abcdefABCDEF]{24}')
                if(mac.match(patt)==null) {
                        alert("The content of the MAC/MACUID is not right, shoule be 012345689abcdefABCDEF!")
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
        		<li><a href="load.php"><strong>添加</strong></a></li>
        		<li class="current"><a href="delete.php"><strong>删除</strong></a></li>
				<li><a href="createuuid.php"><strong>生成UUID</strong></a></li>
        	</ul>
    	</div>
	<form id="manform"  action="/delete.php" method="post" >
	<div class="content">
			<p>
				MAC/MCUID: <input type="text" name="macuuid" id="macuuid" maxlength="24" size="32" value=<?php echo $_POST[macuuid];?>>
				&nbsp;&nbsp;<input type="submit" name="submit" value="删除" />
			</p>
			<p id="result">
			<?php delete()  ?>
			</p>
	
	</div>
	</form>
	<div class="footer">
		<p>&nbsp;</p>
	</div>
</body>
</html>
