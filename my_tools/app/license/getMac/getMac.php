#!/usr/bin/php
<?php
	error_reporting(1);
	date_default_timezone_set("PRC");
	if($argc != 3){
		echo "Usage : $0 [UUID] [bind]\n";
        	echo "Example :\n";
        	echo "        $0 006c004a1247363032303536 1\n";
		echo "Please check the argument!\n";
		exit(1);
	}
	$UUID = $argv[1];
	$bind = $argv[2];
	$link = mysql_connect("172.16.0.168","root","root") or die("Can not connect to DATABASE...".mysql_error());
	$db = mysql_select_db("openvox",$link);
	
	if(!$db) {
		echo "DATABASE ERROR:Can not select table!\n";
		exit(1);
	}

	$result = mysql_query("lock tables macUuid write",$link); // Mutex : willbe unlock when exit
        if(!$result) {
                echo "DATABASE ERROR:Can not lock tables!\n";
                exit(1);
        }	
	$result = mysql_query("select MAC from macUuid where UUID='".$UUID."'",$link);		
	if(!$result) {
                echo "DATABASE ERROR:Can not select special MAC from table for UUID!\n";
                exit(1);
	}
			
	if(mysql_num_rows($result) > 1) {
		echo "DATABASE ERROR: Find multiple MACs that has been bound to UUID (".$UUID.")\n";
		exit(1);
	}else if(mysql_num_rows($result) == 1){
		$macobj = mysql_fetch_object($result);
		//$ret = mysql_query("update macUuid set UUID='".$UUID."' where MAC='".$macobj->MAC."'");
		//if(!$ret) {
		//	echo "DATABASE ERROR: Cannot bind UUID ".$UUID." to MAC ".$macobj->MAC."\n";
		//	exit(1);
		//}else {
			echo $macobj->MAC."\n";
			exit(0);
		//} 	
	}else if(mysql_num_rows($result) == 0) {
		$result1 = mysql_query("select MAC from macUuid where UUID is null limit 1",$link);
        	if(!$result1) {
                	echo "DATABASE ERROR:Can not select idle MAC from table for UUID!\n";
                	exit(1);
        	}
		if(mysql_num_rows($result1) == 0) {
			echo "DATABASE ERROR: There is no idle MAC for UUID (".$UUID.")\n";
			exit(1);
		} else if(mysql_num_rows($result1) == 1){
			$macobj = mysql_fetch_object($result1);
                        $ret = mysql_query("update macUuid set UUID='".$UUID."' where MAC='".$macobj->MAC."'");
                        if(!$ret) {
                        	echo "DATABASE ERROR: Cannot bind UUID ".$UUID." to MAC ".$macobj->MAC."\n";
				exit(1);
			}else {
				if($bind == 1) {

					$mac = $macobj->MAC;
					$uuid = $UUID;
					$mactable = "mac_".$mac;
					$uuidtable = "uuid_".$uuid;

                			mysql_query("unlock tables",$link);
                			mysql_query("lock tables $mactable write",$link); // Mutex : willbe unlock when exit
                			$time = date("Y-m-d H:i:s");
                			$cmd = "insert into $mactable(TIME,OPRT,UUID) values('$time','bind','$uuid')";
                			$result = mysql_query($cmd,$link);
                			if(!$result) {
                        			echo "DATABASE ERROR:Can not insert record to table!<br>".mysql_error();
						exit(1);
                			}
                        		mysql_query("unlock tables",$link);
                        		$cmd = "create table $uuidtable(TIME char(20), OPRT char(24), MAC char(12) , PRIMARY KEY (TIME))"; 
                        		$result = mysql_query($cmd,$link);
                        		if(!$result && mysql_errno() != 1050) {
                                		echo "DATABASE ERROR:Can not create mcuid table!<br>".mysql_error();
						exit(1);
                        		}
                         		mysql_query("lock tables $uuidtable write",$link); // Mutex : willbe unlock when exit
                        		$cmd = "insert into $uuidtable(TIME,OPRT,MAC) values('$time','bind','$mac')";
                        		$result = mysql_query($cmd,$link);
                        		if(!$result) {
                                		echo "DATABASE ERROR:Can not insert record to table!<br>".mysql_error();
						exit(1);
                        		}

                		}					
                        	echo $macobj->MAC."\n";
				exit(0);
			}
		}
	}

?>
