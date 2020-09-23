<?php
date_default_timezone_set("PRC");
function my_exit($link,$msg,$code) {
        mysql_query("unlock tables",$link);
        mysql_close($link);
        echo $msg;
        return $code;
}
function getSerial($mac) {
	$link = mysql_connect("127.0.0.1","root","root") or die("Can not connect to DATABASE...".mysql_error());
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
	$result = mysql_query("select UUID from macUuid where MAC='".$mac."'",$link);
        if(!$result) {
                echo "DATABASE ERROR:Can not select special UUID from table!\n";
                exit(1);
        }
	if(mysql_num_rows($result) == 0){
		echo "DATABASE ERROR: There is no record for MAC (".$mac.")\n";
        	exit(1);
	}else if(mysql_num_rows($result) == 1){
		$macobj = mysql_fetch_object($result);
		$uuid = $macobj->UUID;
		if(strlen($uuid) != 24){
			echo "DATABASE ERROR: There is no right UUID bound to MAC (".$mac.")\n";
                	exit(1);
		}	
		return $uuid;
	}else{
		echo "DATABASE ERROR: There is no UUID bound to  MAC (".$mac.")\n";
                exit(1);
	}
	
}
function updateRecord($mac,$serial,$limits){
	$link = mysql_connect("127.0.0.1","root","root") or die("Can not connect to DATABASE...".mysql_error());
	$db = mysql_select_db("openvox",$link);

	if(!$db) {
		my_exit($link,"DATABASE ERROR:Can not select table!<br>".mysql_error(),1);
	}
        $result = mysql_query("lock tables macUuid write",$link); // Mutex : willbe unlock when exit
        if(!$result) {
        	my_exit($link,"DATABASE ERROR:Can not lock tables!<br>".mysql_error(),1);
        }
        $mactable = "mac_".$mac;
        $uuidtable = "uuid_".$serial;
        if(strlen($uuid) == 0) 
        	$uuid = null;

     	mysql_query("unlock tables",$link);

       	mysql_query("lock tables $mactable write",$link); // Mutex : willbe unlock when exit
        
	$time = date("Y-m-d H:i:s");

	$cmd = "insert into $mactable(TIME,OPRT,UUID) values('$time','license','$limits')";
        $result = mysql_query($cmd,$link);
        if(!$result) {
        	my_exit($link,"DATABASE ERROR:Can not insert record to table!<br>".mysql_error(),1);
        }
        mysql_query("unlock tables",$link);
        mysql_query("lock tables $uuidtable write",$link); // Mutex : willbe unlock when exit
        $cmd = "insert into $uuidtable(TIME,OPRT,MAC) values('$time','create','$limits')";
        $result = mysql_query($cmd,$link);
        if(!$result) {
        	my_exit($link,"DATABASE ERROR:Can not insert record to table!<br>".mysql_error(),1);
        }

        my_exit($link,"Insert Success!",0);	
}
function generate_license() {
	$limits = array(
			'Sip' => 0,
			'Iax' => 0,
			'Cc' => 0,
			'T1' => 0,
			'T2' => 0,		
			'R1' => 0,
			'R2' => 0,
			'R3' => 0,
			'R4' => 0,
			'R5' => 0,
			'R6' => 0,
			'R7' => 0,
			'R8' => 0,
			);        

	if(isset($_POST['sipendpoint']))
		$limits['Sip'] = intval($_POST['sipendpoint']);

	if(isset($_POST['iaxendpoint']))
                $limits['Iax'] = intval($_POST['iaxendpoint']);

        if(isset($_POST['concurrent']))
                $limits['Cc'] = intval($_POST['concurrent']);

        if(isset($_POST['time1']))
                $limits['T1'] = intval($_POST['time1']);

        if(isset($_POST['time2']))
                $limits['T2'] = intval($_POST['time2']);

        if(isset($_POST['reserve1']))
                $limits['R1'] = $_POST['reserve1'];

        if(isset($_POST['reserve2']))
                $limits['R2'] = $_POST['reserve2'];

        if(isset($_POST['reserve3']))
                $limits['R3'] = $_POST['reserve3'];

        if(isset($_POST['reserve4']))
                $limits['R4'] = $_POST['reserve4'];

        if(isset($_POST['reserve5']))
                $limits['R5'] = $_POST['reserve5'];

        if(isset($_POST['reserve6']))
                $limits['R6'] = $_POST['reserve6'];	
	
        if(isset($_POST['reserve7']))
                $limits['R7'] = $_POST['reserve7'];

        if(isset($_POST['reserve8']))
                $limits['R8'] = $_POST['reserve8'];


        if(isset($_POST['macs']))
                $macs = $_POST['macs'];
	
	exec("rm -rf /license/res/*");
		
	$a_macs = explode("\r\n",$macs);
	$macs_num = count($a_macs);
	for($x=0; $x<$macs_num; $x++) {
		$mac = trim($a_macs[$x]);
		
		if(strlen($mac) == 0)
			continue;
		$serial = getSerial($mac);
		

		$limits['Serial'] = $serial;
		$licContent = json_encode($limits);
		$licFilename = "/license/license.txt";
		file_put_contents($licFilename, $licContent, LOCK_EX);

		exec("/bin/genlicense -c=/etc/license/license.conf",$output);

        	if($output) {
                	echo "</br>";
                	echo "Error:Can not generate license file!!!";
                	echo "</br>";
                	echo "Error code";
                	echo ": ".$output[0];
                	return;
        	}
		updateRecord($mac,$serial,$limits["Sip"]);
		exec("cp /license/res/license.crt /license/res/license-$mac.crt");
		exec("cp /license/license.txt /license/res/license-$mac.txt");
	}
	exec("cd /license/res/ && tar -zcvf /license/res/license.crt license-*.crt && cd /srv/www/htdocs/");
	exec("rm -rf /license/license.txt");


    $cfg_name="license.crt";                                                                                                                        
                                                                                                                                                                         
    $cfg_path="/license/res/license.crt";                                                                                                                                      
    //........                                                                                                             
    $file = fopen ($cfg_path, "r" );                                                                                       
    $size = filesize($cfg_path) ;                                                                                          
                                                                                                                               
    //............                                                                                                         
    header('Content-Encoding: none');                                                                                      
    header('Content-Type: application/force-download');                                
    header('Content-Type: application/octet-stream');                                  
    header('Content-Type: application/download');                                      
    header('Content-Description: File Transfer');                                      
    header('Accept-Ranges: bytes');                                                    
    header( "Accept-Length: $size");                                                   
    header( 'Content-Transfer-Encoding: binary' );                                     
    header( "Content-Disposition: attachment; filename=$cfg_name" );                   
    header('Pragma: no-cache');                                                        
    header('Expires: 0');                                                              
    //............                                                                     
    //..............................                                                   
        
	ob_clean();                                                                        
    flush();                                                                           
    echo fread($file, $size);                                                          
    fclose ($file);                                                                    
                                                                                           
    unlink($cfg_path);                                                                 
                                                                                          
                                                                                           
}
if(isset($_POST["submit"]))
        generate_license(); 
?>
