<?php
function my_exit($link,$msg,$code) {
        mysql_query("unlock tables",$link);
        mysql_close($link);
        echo $msg;
        exit($code);
}
function query() {
				
        if(isset($_POST) && $_POST[submit] == "查询") {
                $link = mysql_connect("127.0.0.1","root","root") or die("Can not connect to DATABASE...".mysql_error());
                $db = mysql_select_db("openvox",$link);

                if(!$db) {
          		my_exit($link,"DATABASE ERROR:Can not select table!<br>".mysql_error(),1);
                }
                $o_macuuid = $_POST[macuuid];
                $macuuid = trim($o_macuuid);
		if(strlen($macuuid) == 12)
			$macuuidtable = "mac_".$macuuid;
		else
			$macuuidtable = "uuid_".$macuuid;
                $result = mysql_query("lock tables $macuuidtable write",$link); // Mutex : willbe unlock when exit
                if(!$result) {
                        my_exit($link,"DATABASE ERROR:Can not lock tables!<br>".mysql_error(),1);
                }
         	$result = mysql_query("select * from $macuuidtable",$link);
             	if(!$result) {
                	my_exit($link,"DATABASE ERROR:Can not find record from table!<br>".mysql_error(),1);
           	}
            	if(mysql_num_rows($result) == 0){
                	my_exit($link,"No record<br>",0);
                }else { 
                       return $result;
		}

        } else if(isset($_POST) && $_POST[submit] == "查询全部") {
                $link = mysql_connect("127.0.0.1","root","root") or die("Can not connect to DATABASE...".mysql_error());
                $db = mysql_select_db("openvox",$link);

                if(!$db) {
                        my_exit($link,"DATABASE ERROR:Can not select table!<br>".mysql_error(),1);
                }
                $result = mysql_query("lock tables macUuid write",$link); // Mutex : willbe unlock when exit
                if(!$result) {
                        my_exit($link,"DATABASE ERROR:Can not lock tables!<br>".mysql_error(),1);
                }
                $result = mysql_query("select * from macUuid",$link);
                if(!$result) {
                        my_exit($link,"DATABASE ERROR:Can not find record from table!<br>".mysql_error(),1);
                }
                if(mysql_num_rows($result) == 0){
                        my_exit($link,"No record<br>",0);
                }else {
                       return $result;
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
	table {
		color:#5897C6;
	}
	td,th{
	    width:250px;
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
		min-height:570px;
		height:auto;
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
	#result {
		margin: 0 auto;
                text-align:center;
	}
</style>
<script type="text/javascript" src="jquery.js"></script>
<script type="text/javascript">

$(document).ready(function(){

$("#macuuid").keyup(function() {
	$("#result").text("");	
});

$("#manform2").submit(function(){
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
			<li class="current"><a href="query.php" ><strong>查询</strong></a></li>
        		<li><a href="load.php"><strong>添加</strong></a></li>
        		<li><a href="delete.php"><strong>删除</strong></a></li>
				<li><a href="createuuid.php"><strong>生成UUID</strong></a></li>
        	</ul>
    	</div>
	<div class="content">
		<div >
		<form id="manform1"  action="/query.php" method="post" >
                        <p>
                                <input type="submit" name="submit" value="查询全部" />
                        </p>
        
		</form>
		<form id="manform2"  action="/query.php" method="post" >
			<p>
				MAC/MCUID: <input type="text" name="macuuid" id="macuuid" maxlength="24" size="32" value=<?php echo $_POST[macuuid]; ?> >
				&nbsp;&nbsp;<input type="submit" name="submit" value="查询" />
			</p>
	
		</form>
		</div>
		<div id="result">
			
			<center>
			<table  border="1" cellspacing="0" cellpadding="0" bordercolor="#5897C6">
				<?php 
					if(isset($_POST) && $_POST[submit] == "查询全部"){
				?>
                    		<caption>MAC-MCUID Table</caption>
                        	<th>MAC</th>
                        	<th>MCUID</th>
				<?php
					$result = query();
					while($rs=mysql_fetch_object($result)){
                                		$mac=$rs->MAC;
                                		$uuid=$rs->UUID;
		
				?>
				<tr align="center">
                    		<td><?php echo $mac; ?></td>
                    		<td><?php echo $uuid; ?></td>
                		</tr>
                		<?php
                    				}// end while
					}// end if
				?>

				<?php
					if (isset($_POST) && $_POST[submit] == "查询" && strlen($_POST[macuuid]) == 12) {
				
				?>
                                <caption>MAC-OPRT Table</caption>
                                <th>MAC</th>
                                <th>Time</th>
				<th>OPRT</th>
				<th>UUID</th>

                                <?php
                                        $result = query();
                                        while($rs=mysql_fetch_object($result)){
						
                                                $time = $rs->TIME;
                                                $oprt = $rs->OPRT;
						$uuid = $rs->UUID;

                                ?>
                                <tr align="center">
                                <td><?php echo $_POST[macuuid]; ?></td>
                                <td><?php echo $time; ?></td>
                                <td><?php echo $oprt; ?></td>
                                <td><?php echo $uuid; ?></td>
                                </tr>
                                <?php
                                        	}// end while
					}// end else if
                                ?>
				
                                <?php
                                        if (isset($_POST) && $_POST[submit] == "查询" && strlen($_POST[macuuid]) == 24) {

                                ?>
                                <caption>UUID-OPRT Table</caption>
                                <th>UUID</th>
                                <th>Time</th>
                                <th>OPRT</th>
                                <th>MAC</th>

                                <?php
                                        $result = query();
                                        while($rs=mysql_fetch_object($result)){

                                                $time = $rs->TIME;
                                                $oprt = $rs->OPRT;
                                                $mac = $rs->MAC;

                                ?>
                                <tr align="center">
                                <td><?php echo $_POST[macuuid]; ?></td>
                                <td><?php echo $time; ?></td>
                                <td><?php echo $oprt; ?></td>
                                <td><?php echo $mac; ?></td>
                                </tr>
                                <?php
                                                }// end while
                                        }// end else if
                                ?>
				
                	</table>
			</center>
		</div>
	</div>
	<div class="footer">
		<p>&nbsp;</p>
	</div>
</body>
</html>
