<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
require_once("/www/cgi-bin/inc/aql.php");

date_default_timezone_set('UTC');
?>

<!---// load jQuery and the jQuery iButton Plug-in //--> 
<!--<script type="text/javascript" src="http://ajax.googleapis.com/ajax/libs/jquery/1.3.2/jquery.min.js"></script> -->
<script type="text/javascript" src="/js/jquery.ibutton.js"></script> 
 
<!---// load the iButton CSS stylesheet //--> 
<link type="text/css" href="/css/jquery.ibutton.css" rel="stylesheet" media="all" />

<?php
function save_pptpvpn_operate_server($pptpvpn)
{

	$vpn_conf = "/etc/asterisk/gw/pptp/vpn.conf"; 
	$options_conf='/etc/asterisk/gw/pptp/options.pptp';
	if($pptpvpn=='1'){
		if(!file_exists($options_conf)){
			$write_str = <<<EOF
lock
noauth
#refuse-pap
#refuse-eap
#refuse-chap
#refuse-mschap
nobsdcomp
nodeflate
idle 0
defaultroute
maxfail 0
EOF;
//EOF			
			$hlock = lock_file($options_conf);
			$handle = fopen($options_conf,"w");
			fwrite($handle,$write_str);
			fclose($handle);
			unlock_file($hlock);
		}
		
		if (isset($_POST['pptp_server'])) {
			$server=trim($_POST['pptp_server']);
		} else {
			$server='0.0.0.0';
		}
		
		if (isset($_POST['pptp_account'])) {
			$account=trim($_POST['pptp_account']);
		} else {
			$account='test';
		}
		if (isset($_POST['pptp_password'])) {
			$password=trim($_POST['pptp_password']);
		} else {
			$password='test';
		}
		if (isset($_POST['pptp_domain'])) {
			$domain=trim($_POST['pptp_domain']);
		} else {
			$domain='';
		}
		if (isset($_POST['pptp_mppe'])) {
			$mppe=true;
		} else {
			$mppe=false;
		}
		
		$product_type = get_product_type();
		if($product_type < 4){
			$mppe_str = "mppe required,stateless";
		}else{
			$mppe_str = "require-mppe";
		}
		
		$hlock = lock_file($vpn_conf);
		if(file_exists($vpn_conf)){
			$handle = fopen($vpn_conf,"r");

		       $write_str = '';
			while (!feof($handle)) {
				$line = fgets($handle);	

				if(strstr($line, 'pty')) {
				    $line="pty \"pptp ".$server." --nolaunchpppd\"\n";
				}
				
				if(strstr($line, 'name') && !strstr($line, 'remotename')){
					$line="name ".$account."\n";
				}
				
				if(strstr($line, 'require-mppe') || strstr($line, 'mppe required,stateless')) {
					if($mppe) {
					    $line=$mppe_str."\n";
					}else{
					    $line="#".$mppe_str."\n";
					}
				}
				$write_str .= $line;
			}
			
			fclose($handle);
		} else {
			if ($mppe) {
			    $mppestr=$mppe_str."\n";
			}else{
			    $mppestr="#".$mppe_str."\n";
			}
			
			$write_str = <<<EOF
pty "pptp $server --nolaunchpppd"
debug
lock
noauth
nobsdcomp
nodeflate
name test
remotename vpn
ipparam vpn
#+chap
#+mschap-v2
$mppestr
#mppe stateless
file /etc/ppp/options.pptp
EOF;
//EOF
		}
		
		if($write_str != '') {
			$handle = fopen($vpn_conf,"w");
			fwrite($handle,$write_str);
			fclose($handle);
		}
		unlock_file($hlock);

		   
		$chap_conf = '/etc/ppp/chap-secrets';
		$hlock = lock_file($chap_conf);
		$handle = fopen($chap_conf,"w");
		$write_str =$account.' vpn '.$password.' *';
		fwrite($handle,$write_str);
		fclose($handle);
		unlock_file($hlock);
	}
}

function save_vpn_cfg($vpntype)
{
	$conf_path = '/etc/asterisk/gw/vpn.conf';
	if(!file_exists($conf_path)) {
		fclose(fopen($conf_path,"w"));
	}

	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$hlock = lock_file($conf_path);
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}

	$exist_array = $aql->query("select * from vpn.conf");

	if(!isset($exist_array['general'])) {
		$aql->assign_addsection('general','');
	}

	if(isset($exist_array['general']['vpntype'])) {
		$aql->assign_editkey('general','vpntype', $vpntype);
	} else {
		$aql->assign_append('general','vpntype', $vpntype);
	}
	
	if(!isset($exist_array['MonitorVPN'])) {
		$aql->assign_addsection('MonitorVPN','');
	}
	
	$minute = $_POST['minute'];
	if(isset($exist_array['MonitorVPN']['time'])){
		$aql->assign_editkey('MonitorVPN','time',$minute);
	}else{
		$aql->assign_append('MonitorVPN','time',$minute);
	}
	
	$ip = $_POST['monitorip'];
	if(isset($exist_array['MonitorVPN']['ip'])){
		$aql->assign_editkey('MonitorVPN','ip',$ip);
	}else{
		$aql->assign_append('MonitorVPN','ip',$ip);
	}

	if (!$aql->save_config_file('vpn.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false; 
	}
	unlock_file($hlock);
	
	//restart cron
	if($vpntype == 'openvpn' && $ip != ""){
		if($minute != 0){
			$write = "*/$minute * * * * root /my_tools/monitor_vpn.sh $ip";
			
			$file_path = "/etc/asterisk/gw/crontabs_root";
			$chlock = lock_file($file_path);
			exec("sed -i '/\/my_tools\/monitor_vpn.sh/d' \"$file_path\" 2> /dev/null");
			if($write != '') exec("echo \"$write\" >> $file_path");
			unlock_file($chlock);
		}else{
			$file_path = "/etc/asterisk/gw/crontabs_root";
			exec("sed -i '/\/my_tools\/monitor_vpn.sh/d' \"$file_path\" 2> /dev/null");
		}
		wait_apply("exec", "sh /etc/init.d/cron restart > /dev/null 2>&1 &");
	}
	
	return true;
}

function save_openvpn_operate_server($openvpn)
{
	/*if (isset($_POST['openvpn'])) {
		$openvpn = '1';
	} else {
		$openvpn = '0';
	}*/

	$conf_path = '/etc/asterisk/gw/openvpn.conf';
	if(!file_exists($conf_path)) {
		fclose(fopen($conf_path,"w"));
	}

	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw');
	$hlock = lock_file($conf_path);
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}

	$exist_array = $aql->query("select * from openvpn.conf");

	if(!isset($exist_array['general'])) {
		$aql->assign_addsection('general','');
	}

	if(isset($exist_array['general']['openvpn'])) {
		$aql->assign_editkey('general','openvpn', $openvpn);
	} else {
		$aql->assign_append('general','openvpn', $openvpn);
	}

	if (!$aql->save_config_file('openvpn.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false; 
	}
	unlock_file($hlock);

	return true;
}

function make_update_file_path($isdir)
{
	$file_path="/data/update_file_".date("YmdHim");
	$tmp = $file_path;

	$i=0;
	while(file_exists($file_path)) {
		$i++;
		$file_path = $tmp."$i";
	}

	if($isdir)
		system("mkdir $file_path");
	return $file_path;
}

function del_old_updatefile()
{
	exec("rm -rf /data/update_file_*");
}

function upload_cfg_file()
{
	if(! $_FILES) {
		return;
	}

	echo "<br>";
	$Report = language('Report');
	$Result = language('Result');
	$theme = language("OpenVPN Configuration Files Upload");
	trace_output_start("$Report", "$theme");
	trace_output_newline();
	if(isset($_FILES['upload_cfg_file']['error']) && $_FILES['upload_cfg_file']['error'] == 0) {  //Update successful
		if(!(isset($_FILES['upload_cfg_file']['size'])) || $_FILES['upload_cfg_file']['size'] > 40*1000*1000) { //Max file size 40Mbyte
			echo language('Configuration Files Upload Filesize error',"Your uploaded file was larger than 40M!<br>Uploading configuration files was failed.");
			return;
		}

		$store_file = make_update_file_path(0);

		if (!move_uploaded_file($_FILES['upload_cfg_file']['tmp_name'], $store_file)) {  
			echo language('Configuration Files Upload Move error',"Moving your updated file was failed!<br>Uploading configuration files was failed.");  
			return;
		}
		echo language("Configuration Files Uploading");echo " ......<br>";
		ob_flush();
		flush();

		echo language('Configuration Files Upload check',"Checking configuration version....<br>");
		ob_flush();
		flush();

		echo language('Configuration Files Upload update',"Updating configuration files....<br>");
		ob_flush();
		flush();
		$untar_file = make_update_file_path(1);
		$cmd = "tar zxf $store_file -C $untar_file || echo $?";
		exec($cmd,$output);
		if($output) {
			echo language('Configuration Files Upload Failed');echo "<br>\n";
			echo language("Configuration Files format error", "Configuration File format must be tar.gz!<br>");
			del_old_updatefile();
			return;
		}

		echo language('Stop OpenVPN',"Stoping OpenVPN....<br>");
		ob_flush();
		flush();

		exec("/etc/init.d/vpn stop > /dev/null 2>&1");

		exec("mkdir -p /etc/asterisk/openvpn");
		exec("rm -rf /etc/asterisk/openvpn/*");
		exec("rm -rf /etc/cfg/openvpn/*");
		exec("mv $untar_file/* /etc/asterisk/openvpn/");

		echo language('Start OpenVPN',"Starting OpenVPN....<br>");
		ob_flush();
		flush();
		exec("/etc/init.d/vpn start > /dev/null 2>&1");

		del_old_updatefile();

		trace_output_newhead("$Result");
		echo language("Configuration Files Upload Succeeded");
	} else {
		if(isset($_FILES['upload_cfg_file']['error'])) {
			switch($_FILES['upload_cfg_file']['error']) {
			case 1:    
				echo language('Configuration Files Upload error 1',"The file was larger than the server space 40M!");
				break;
			case 2:    
				echo language('Configuration Files Upload error 2',"The file was larger than the browser's limit!");
				break;
			case 3:
				echo language('Configuration Files Upload error 3',"The file was only partially uploaded!");
				break;
			case 4: 
				echo language('Configuration Files Upload error 4',"Can not find uploaded file!");
				break;
			case 5: 
				echo language('Configuration Files Upload error 5',"The server temporarily lost folder!");    
				break;
			case 6: 
				echo language('Configuration Files Upload error 6',"Failed to write to the temporary folder!");    
				break;    
			}
		}
		echo "<br>";
		trace_output_newhead("$Result");
		echo language("Configuration Files Upload Failed");
	}
	trace_output_end();
}

function upload_ovpn_file(){
	if(! $_FILES) {
		return;
	}
	
	echo "<br>";
	$Report = language('Report');
	$Result = language('Result');
	$theme = language("OpenVPN Configuration Files Upload");
	trace_output_start("$Report", "$theme");
	trace_output_newline();
	
	if(!strstr($_FILES['upload_cfg_file']['name'],'ovpn')){
		echo language("Firmware upload help","The format of the uploaded file is incorrect.")."<br/>";
		trace_output_end();
		return;
	}
	
	if(isset($_FILES['upload_cfg_file']['error']) && $_FILES['upload_cfg_file']['error'] == 0) {  //Update successful
		if(!(isset($_FILES['upload_cfg_file']['size'])) || $_FILES['upload_cfg_file']['size'] > 40*1000*1000) { //Max file size 40Mbyte
			echo language('Configuration Files Upload Filesize error',"Your uploaded file was larger than 40M!<br>Uploading configuration files was failed.");
			return;
		}

		exec("mkdir -p /etc/asterisk/openvpn");
		exec("rm -rf /etc/asterisk/openvpn/*");
		exec("rm -rf /etc/cfg/openvpn/*");
		
		if (!move_uploaded_file($_FILES['upload_cfg_file']['tmp_name'], '/etc/asterisk/openvpn/openvpn-conn.ovpn')) {
			echo language('Configuration Files Upload Move error',"Moving your updated file was failed!<br>Uploading configuration files was failed.");  
			return;
		}
		echo language("Configuration Files Uploading");echo " ......<br>";
		ob_flush();
		flush();

		echo language('Configuration Files Upload check',"Checking configuration version....<br>");
		ob_flush();
		flush();

		echo language('Configuration Files Upload update',"Updating configuration files....<br>");
		ob_flush();
		flush();

		echo language('Stop OpenVPN',"Stoping OpenVPN....<br>");
		ob_flush();
		flush();

		exec("/etc/init.d/vpn stop > /dev/null 2>&1");

		echo language('Start OpenVPN',"Starting OpenVPN....<br>");
		ob_flush();
		flush();
		exec("/etc/init.d/vpn start > /dev/null 2>&1");

		del_old_updatefile();

		trace_output_newhead("$Result");
		echo language("Configuration Files Upload Succeeded");
	} else {
		if(isset($_FILES['upload_cfg_file']['error'])) {
			switch($_FILES['upload_cfg_file']['error']) {
			case 1:    
				echo language('Configuration Files Upload error 1',"The file was larger than the server space 40M!");
				break;
			case 2:    
				echo language('Configuration Files Upload error 2',"The file was larger than the browser's limit!");
				break;
			case 3:
				echo language('Configuration Files Upload error 3',"The file was only partially uploaded!");
				break;
			case 4: 
				echo language('Configuration Files Upload error 4',"Can not find uploaded file!");
				break;
			case 5: 
				echo language('Configuration Files Upload error 5',"The server temporarily lost folder!");    
				break;
			case 6: 
				echo language('Configuration Files Upload error 6',"Failed to write to the temporary folder!");    
				break;    
			}
		}
		echo "<br>";
		trace_output_newhead("$Result");
		echo language("Configuration Files Upload Failed");
	}
	trace_output_end();
}

function save_n2nvpn(){
	$conf_path = '/etc/asterisk/gw/n2n/n2n.conf';
	if(!file_exists($conf_path)) {
		fclose(fopen($conf_path,"w"));
	}

	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw/n2n/');
	$hlock = lock_file($conf_path);
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}

	$exist_array = $aql->query("select * from n2n.conf");

	if(!isset($exist_array['n2n'])) {
		$aql->assign_addsection('n2n','');
	}

	if(isset($_POST['n2n_enable'])){
		$n2n_enable = 'on';
	}else{
		$n2n_enable = 'off';
	}
	if(isset($exist_array['n2n']['enable'])) {
		$aql->assign_editkey('n2n','enable', $n2n_enable);
	} else {
		$aql->assign_append('n2n','enable', $n2n_enable);
	}
	
	$n2n_server_addr = $_POST['n2n_server_addr'];
	if(isset($exist_array['n2n']['server_addr'])){
		$aql->assign_editkey('n2n','server_addr',$n2n_server_addr);
	}else{
		$aql->assign_append('n2n','server_addr',$n2n_server_addr);
	}
	
	$n2n_port = $_POST['n2n_port'];
	if(isset($exist_array['n2n']['port'])){
		$aql->assign_editkey('n2n','port',$n2n_port);
	}else{
		$aql->assign_append('n2n','port',$n2n_port);
	}
	
	$n2n_local_ip = $_POST['n2n_local_ip'];
	if(isset($exist_array['n2n']['local_ip'])){
		$aql->assign_editkey('n2n','local_ip',$n2n_local_ip);
	}else{
		$aql->assign_append('n2n','local_ip',$n2n_local_ip);
	}
	
	$n2n_mask = $_POST['n2n_mask'];
	if(isset($exist_array['n2n']['subnet_mask'])){
		$aql->assign_editkey('n2n','subnet_mask',$n2n_mask);
	}else{
		$aql->assign_append('n2n','subnet_mask',$n2n_mask);
	}
	
	/*
	$n2n_local_port = $_POST['n2n_local_port'];
	if(isset($exist_array['n2n']['local_port'])){
		$aql->assign_editkey('n2n','local_port',$n2n_local_port);
	}else{
		$aql->assign_append('n2n','local_port',$n2n_local_port);
	}*/
	
	$n2n_username = $_POST['n2n_username'];
	if(isset($exist_array['n2n']['user_name'])){
		$aql->assign_editkey('n2n','user_name',$n2n_username);
	}else{
		$aql->assign_append('n2n','user_name',$n2n_username);
	}
	
	$n2n_password = $_POST['n2n_password'];
	if(isset($exist_array['n2n']['password'])){
		$aql->assign_editkey('n2n','password',$n2n_password);
	}else{
		$aql->assign_append('n2n','password',$n2n_password);
	}

	if (!$aql->save_config_file('n2n.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false; 
	}
	unlock_file($hlock);

	wait_apply("exec","/etc/init.d/vpn");
}

function save_l2tpvpn(){
	$conf_path = '/etc/asterisk/gw/xl2tpd/xl2tp_vpn.conf';
	
	if(!is_dir('/etc/asterisk/gw/xl2tpd/')){
		mkdir('/etc/asterisk/gw/xl2tpd/');
	}
	
	if(!file_exists($conf_path)) {
		touch($conf_path);
		fclose(fopen($conf_path,"w"));
	}

	$aql = new aql();
	$aql->set('basedir','/etc/asterisk/gw/xl2tpd/');
	$hlock = lock_file($conf_path);
	if(!$aql->open_config_file($conf_path)){
		echo $aql->get_error();
		unlock_file($hlock);
		return false;
	}

	$exist_array = $aql->query("select * from xl2tp_vpn.conf");

	if(!isset($exist_array['l2tp'])) {
		$aql->assign_addsection('l2tp','');
	}

	if(isset($_POST['l2tp_enable'])){
		$l2tp_enable = 'on';
	}else{
		$l2tp_enable = 'off';
	}
	if(isset($exist_array['l2tp']['enable'])) {
		$aql->assign_editkey('l2tp','enable', $l2tp_enable);
	} else {
		$aql->assign_append('l2tp','enable', $l2tp_enable);
	}
	
	$l2tp_server = $_POST['l2tp_server'];
	if(isset($exist_array['l2tp']['server'])) {
		$aql->assign_editkey('l2tp','server', $l2tp_server);
	} else {
		$aql->assign_append('l2tp','server', $l2tp_server);
	}
	
	$l2tp_username = $_POST['l2tp_username'];
	if(isset($exist_array['l2tp']['username'])) {
		$aql->assign_editkey('l2tp','username', $l2tp_username);
	} else {
		$aql->assign_append('l2tp','username', $l2tp_username);
	}
	
	$l2tp_password = $_POST['l2tp_password'];
	if(isset($exist_array['l2tp']['password'])) {
		$aql->assign_editkey('l2tp','password', $l2tp_password);
	} else {
		$aql->assign_append('l2tp','password', $l2tp_password);
	}
	
	if(isset($_POST['IPSec'])){
		$IPSec = 'on';
	}else{
		$IPSec = 'off';
	}
	if(isset($exist_array['l2tp']['ipsec'])) {
		$aql->assign_editkey('l2tp','ipsec', $IPSec);
	} else {
		$aql->assign_append('l2tp','ipsec', $IPSec);
	}
	
	/*
	$IPSec_IP = $_POST['IPSec_IP'];
	if(isset($exist_array['l2tp']['ipsec_ip'])) {
		$aql->assign_editkey('l2tp','ipsec_ip', $IPSec_IP);
	} else {
		$aql->assign_append('l2tp','ipsec_ip', $IPSec_IP);
	}
	*/
	
	$IPSec_password = $_POST['IPSec_password'];
	if(isset($exist_array['l2tp']['ipsec_password'])) {
		$aql->assign_editkey('l2tp','ipsec_password', $IPSec_password);
	} else {
		$aql->assign_append('l2tp','ipsec_password', $IPSec_password);
	}
	
	/*
	$IPSec_remote_IP = $_POST['IPSec_remote_IP'];
	if(isset($exist_array['l2tp']['ipsec_remote_ip'])){
		$aql->assign_editkey('l2tp','ipsec_remote_ip', $IPSec_remote_IP);
	}else{
		$aql->assign_append('l2tp','ipsec_remote_ip', $IPSec_remote_IP);
	}
	
	if(isset($_POST['default_gateway'])){
		$default_gateway = 'on';
	}else{
		$default_gateway = 'off';
	}
	if(isset($exist_array['l2tp']['default_gateway'])) {
		$aql->assign_editkey('l2tp','default_gateway', $default_gateway);
	} else {
		$aql->assign_append('l2tp','default_gateway', $default_gateway);
	}*/
	
	if (!$aql->save_config_file('xl2tp_vpn.conf')) {
		echo $aql->get_error();
		unlock_file($hlock);
		return false; 
	}
	unlock_file($hlock);
	
	wait_apply("exec","");
}

function l2tp_save_to_other_conf(){
	//xl2tpd.conf
	$l2tp_server = $_POST['l2tp_server'];
	
	$xl2tpd_path = '/etc/asterisk/gw/xl2tpd/xl2tpd.conf';
	if(!file_exists($xl2tpd_path)){
		touch($xl2tpd_path);
	}
		
	$xl2tpd_str = "[lac testvpn]\n";
	$xl2tpd_str .= "require chap=yes\n";
	$xl2tpd_str .= "require pap=yes\n";
	$xl2tpd_str .= "require authentication=yes\n";
	$xl2tpd_str .= "ppp debug=yes\n";
	$xl2tpd_str .= "pppoptfile=/etc/xl2tpd/dialvpn.lac\n";
	$xl2tpd_str .= "length bit=yes\n";
	$xl2tpd_str .= "redial=yes\n";
	$xl2tpd_str .= "redial timeout=30\n";
	$xl2tpd_str .= "max redials=5\n";
	$xl2tpd_str .= "lns=$l2tp_server\n";
	
	file_put_contents($xl2tpd_path, $xl2tpd_str);
	
	//dialvpn.lac
	$l2tp_username = $_POST['l2tp_username'];
	$l2tp_password = $_POST['l2tp_password'];
	
	$dialvpn_path = '/etc/asterisk/gw/xl2tpd/dialvpn.lac';
	if(!file_exists($dialvpn_path)){
		touch($dialvpn_path);
	}
		
	$dialvpn_str = "ipcp-accept-local\n";
	$dialvpn_str .= "unit 30\n";
	$dialvpn_str .= "ipcp-accept-remote\n";
	$dialvpn_str .= "refuse-eap\n";
	$dialvpn_str .= "require-mschap-v2\n";
	$dialvpn_str .= "noccp\n";
	$dialvpn_str .= "noauth\n";
	$dialvpn_str .= "idle 1800\n";
	$dialvpn_str .= "mtu 1410\n";
	$dialvpn_str .= "mru 1410\n";
	$dialvpn_str .= "defaultroute\n";
	$dialvpn_str .= "usepeerdns\n";
	$dialvpn_str .= "debug\n";
	$dialvpn_str .= "connect-delay 5000\n";
	$dialvpn_str .= "name $l2tp_username\n";
	$dialvpn_str .= "password $l2tp_password\n";
	
	file_put_contents($dialvpn_path, $dialvpn_str);
}

function l2tp_save_to_ipsec(){
	$aql = new aql();
	$setok = $aql->set('basedir','/etc/asterisk/gw/network');
	$res=$aql->query("select * from lan.conf");
	$local_ip = $res['ipv4']['ipaddr'];
	
	//ipsec.conf
	$server_ip = $_POST['l2tp_server'];
	
	$ipsec_conf_path = '/etc/asterisk/gw/ipsec/ipsec.conf';
	if(!file_exists($ipsec_conf_path)){
		touch($ipsec_conf_path);
	}
	
	$ipsec_conf_str = "config setup\n";
	$ipsec_conf_str .= "\t\t nat_traversal=yes\n";
	$ipsec_conf_str .= "\t\t protostack=netkey\n";
	
	$ipsec_conf_str .= "conn testvpn\n";
	$ipsec_conf_str .= "\t\t authby=secret\n";
	$ipsec_conf_str .= "\t\t pfs=no\n";
	$ipsec_conf_str .= "\t\t auto=add\n";
	$ipsec_conf_str .= "\t\t keyingtries=3\n";
	$ipsec_conf_str .= "\t\t dpddelay=30\n";
	$ipsec_conf_str .= "\t\t dpdtimeout=120\n";
	$ipsec_conf_str .= "\t\t dpdaction=clear\n";
	$ipsec_conf_str .= "\t\t rekey=yes\n";
	$ipsec_conf_str .= "\t\t ikelifetime=8h\n";
	$ipsec_conf_str .= "\t\t keylife=1h\n";
	$ipsec_conf_str .= "\t\t type=transport\n";
	$ipsec_conf_str .= "\t\t left=$local_ip\n";
	$ipsec_conf_str .= "\t\t leftprotoport=17/1701\n";
	$ipsec_conf_str .= "\t\t right=$server_ip\n";
	$ipsec_conf_str .= "\t\t rightprotoport=17/1701\n";
	
	file_put_contents($ipsec_conf_path, $ipsec_conf_str);
	
	//ipsec.secrets
	$IPSec_password = $_POST['IPSec_password'];
	
	$ipsec_secrets_path = '/etc/asterisk/gw/ipsec/ipsec.secrets';
	if(!file_exists($ipsec_secrets_path)){
		touch($ipsec_secrets_path);
	}
	
	$ipsec_secret_str = "$local_ip %any: PSK '$IPSec_password'\n";
	
	file_put_contents($ipsec_secrets_path, $ipsec_secret_str);
}

//Read data 
$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$res = $aql->query('select * from vpn.conf');

$type='';
if(isset($res['general']['vpntype'])) {
	$type = trim($res['general']['vpntype']);
} else {
	$type = 'nonevpn';
}

if(isset($_POST['vpn_type'])){
	$type = $_POST['vpn_type'];
}

$vpntype['openvpn'] = '';
$vpntype['pptpvpn'] = '';
$vpntype['n2nvpn'] = '';
$vpntype['nonevpn'] = '';
switch($type) {
	case 'openvpn': $vpntype['openvpn'] = 'selected'; break;
	case 'pptpvpn': $vpntype['pptpvpn'] = 'selected'; break;
	case 'n2nvpn': $vpntype['n2nvpn'] = 'selected'; break;
	default: $vpntype['nonevpn'] = 'selected'; break;
}

if(isset($_POST['minute'])){
	$minute = $_POST['minute'];
}else{
	$minute = $res['MonitorVPN']['time'];
}

if(isset($_POST['monitorip'])){
	$monitorip = $_POST['monitorip'];
}else{
	$monitorip = $res['MonitorVPN']['ip'];
}

if($_POST){ 
	if(isset($_POST['send'])) {
		if($_POST['send'] == 'Save') {			
			if (isset($_POST['vpn_type'])) {
				$type=trim($_POST['vpn_type']);
				save_vpn_cfg($type);
				switch($type) {
					case 'openvpn': 
						if($vpntype['pptpvpn']=='selected'){
							save_pptpvpn_operate_server('0');
						}
						save_openvpn_operate_server('1');
						break;
					
					case 'pptpvpn': 
						if($vpntype['openvpn']=='selected'){
							save_openvpn_operate_server('0');
						}
						save_pptpvpn_operate_server('1');
						break;
						
					case 'n2nvpn':
						save_n2nvpn();
						break;
						
					case 'l2tpvpn':
						save_l2tpvpn();
						l2tp_save_to_other_conf();
						l2tp_save_to_ipsec();
						break;
						
					case 'nonevpn': 
						if($vpntype['openvpn']=='selected'){
							save_openvpn_operate_server('0');
						}
						
						if($vpntype['pptpvpn']=='selected'){
							save_pptpvpn_operate_server('0');
						}
						break;
						
					default: 
						break;
				}
				wait_apply("exec", "/etc/init.d/vpn restart >/dev/null 2>&1");
			}
		}
	}
}

$vpntype['openvpn'] = '';
$vpntype['pptpvpn'] = '';
$vpntype['n2nvpn'] = '';
$vpntype['l2tpvpn'] = '';
$vpntype['nonevpn'] = '';
switch($type) {
	case 'openvpn': $vpntype['openvpn'] = 'selected'; break;
	case 'pptpvpn': $vpntype['pptpvpn'] = 'selected'; break;
	case 'n2nvpn': $vpntype['n2nvpn'] = 'selected'; break;
	case 'l2tpvpn': $vpntype['l2tpvpn'] = 'selected'; break;
	default: $vpntype['nonevpn'] = 'selected'; break;
}

//Read data 
//$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw');
$res = $aql->query('select * from openvpn.conf');

if(isset($res['general']['openvpn'])) {
	$openvpn = trim($res['general']['openvpn']);
} else {
	$openvpn = '0';
}

$mppe=false;
//$serverip='0.0.0.0';
$serverip = '';
$domain='';
$vpn_conf = '/etc/ppp/peers/vpn';
if(file_exists($vpn_conf)){
	$hlock = lock_file($vpn_conf);
	$handle = fopen($vpn_conf,"r");

	$product_type = get_product_type();
	
	while (!feof($handle)) {
		$line = fgets($handle);	
		if(strstr($line, 'pty')) {
		       $start=strpos($line,"pptp")+5;
			$line=substr($line,$start);
		       $end=strpos($line,"--")-1;
	 	       $serverip=trim(substr($line,0,$end));
		}

		if($product_type < 4){
			if($line=="mppe required,stateless\n") {
			   $mppe=true;
			}
		}else{
			if($line=="require-mppe\n") {
			   $mppe=true;
			}
		}
		
		/*if (strstr($line, '#mppe required')) {
		       $mppe=false;
		} else {
			if (strstr($line, 'mppe required')) {
			       $mppe=true;
			}
		}*/		
	}
	
	fclose($handle);
	unlock_file($hlock);
}

$account='test';
$password='test';
$chap_conf = '/etc/ppp/chap-secrets';
if(file_exists($chap_conf)){
	$hlock = lock_file($chap_conf);
	$handle = fopen($chap_conf,"r");
	$line = fgets($handle);	
	
	$data=explode(" ",$line);
	if(isset($data)){
		if(isset($data[0])){
			$account=$data[0];
		} else {
			$account='';
		}
		if(isset($data[2])){
			$password=$data[2];
		}else{
			$password = '';
		}
	}
	
	fclose($handle);
	unlock_file($hlock);
}

//n2nvpn
$conf_path = '/etc/asterisk/gw/n2n/n2n.conf';
$aql = new aql();
$aql->set('basedir','/etc/asterisk/gw/n2n/');

$exist_array = $aql->query("select * from n2n.conf");

if($exist_array['n2n']['enable'] == 'on'){
	$n2n_enable = 'checked';
}else{
	$n2n_enable = '';
}

$n2n_server_addr = $exist_array['n2n']['server_addr'];

$n2n_port = $exist_array['n2n']['port'];

$n2n_local_ip = $exist_array['n2n']['local_ip'];

$n2n_mask = $exist_array['n2n']['subnet_mask'];

//$n2n_local_port = $exist_array['n2n']['local_port'];

$n2n_username = $exist_array['n2n']['user_name'];

$n2n_password = $exist_array['n2n']['password'];

//l2tpvpn
$conf_path = '/etc/asterisk/gw/xl2tpd/xl2tp_vpn.conf';
$aql->set('basedir','/etc/asterisk/gw/xl2tpd/');

$l2tp_array = $aql->query("select * from xl2tp_vpn.conf");

if($l2tp_array['l2tp']['enable'] == 'on'){
	$l2tp_enable = 'checked';
}else{
	$l2tp_enable = '';
}

if(isset($l2tp_array['l2tp']['server'])){
	$l2tp_server = $l2tp_array['l2tp']['server'];
}else{
	$l2tp_server = '';
}

if(isset($l2tp_array['l2tp']['username'])){
	$l2tp_username = $l2tp_array['l2tp']['username'];
}else{
	$l2tp_username = '';
}

if(isset($l2tp_array['l2tp']['password'])){
	$l2tp_password = $l2tp_array['l2tp']['password'];
}else{
	$l2tp_password = '';
}

if($l2tp_array['l2tp']['ipsec'] == 'on'){
	$IPSec = 'checked';
}else{
	$IPSec = '';
}

// if(isset($l2tp_array['l2tp']['ipsec_ip'])){
	// $IPSec_IP = $l2tp_array['l2tp']['ipsec_ip'];
// }else{
	// $IPSec_IP = '';
// }

if(isset($l2tp_array['l2tp']['ipsec_password'])){
	$IPSec_password = $l2tp_array['l2tp']['ipsec_password'];
}else{
	$IPSec_password = '';
}

// if(isset($l2tp_array['l2tp']['ipsec_remote_ip'])){
	// $IPSec_remote_IP = $l2tp_array['l2tp']['ipsec_remote_ip'];
// }else{
	// $IPSec_remote_IP = '';
// }

// if($l2tp_array['l2tp']['default_gateway'] == 'on'){
	// $default_gateway = 'checked';
// }else{
	// $default_gateway = '';
// }
	
?>

<script type="text/javascript" src="/js/functions.js"></script>
<script type="text/javascript" src="/js/check.js"></script>
<script type="text/javascript">
<!--
function trim(str)
{
	return str.replace(/(^\s*)|(\s*$)/g, "");
}

function isAllowFile(file_id)
{
	var x = document.getElementById(file_id).value;
	if(x=="")
	{
		alert("<?php echo language('Select File alert','Please select your file first!');?>");
		return false;
	}
	//return true;

	if(g_bAllowFile)
		return true;

	alert("Uploaded max file is 40M!");
	return false;
}

function upload_cfg_file2()
{
	if(!isAllowFile('upload_cfg_file')) {
		return false;
	}

	if( ! confirm("<?php echo language('File Upload confirm','Are you sure to upload configuration files?\nThis will damage the structure of your original configuration files.');?>") ) {
		return false;
	}

	return true;
}

-->
</script>

	<form enctype="multipart/form-data" action="<?php echo get_self(); ?>" method="post">

	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('VPN Settings');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	
	<table width="100%" class="tedit" >
		<tr  id="tr_vpn_type">
			<th>
				<div class="helptooltips">
					<?php echo language('VPNType');?>:
					<span class="showhelp">
					<?php echo language('Type help@network-openvpn','
						The type of VPN.<br/>
						OpenVPN: openvpn.<br/>
						PPTP VPN: pptp vpn.<br/>
						None: None.');
					?>
					</span>
				</div>
			</th>
			<td>
				<select id="vpn_type" name="vpn_type" onchange="vpntypechange()">
					<option  value="openvpn" <?php echo $vpntype['openvpn'];?> ><?php echo language('OpenVPN');?></option>
					<option  value="pptpvpn" <?php echo $vpntype['pptpvpn'];?> ><?php echo language('PPTP VPN');?></option>
					<option  value="n2nvpn" <?php echo $vpntype['n2nvpn'];?> ><?php echo language('N2N VPN');?></option>
					<option  value="l2tpvpn" <?php echo $vpntype['l2tpvpn'];?> ><?php echo language('L2TP VPN');?></option>
					<option  value="nonevpn" <?php echo $vpntype['nonevpn'];?> ><?php echo language('None');?></option>
				</select>
			</td>
		</tr>
		
		<tr class="div_openvpn">
			<th>
				<div class="helptooltips">
					<?php echo language('VPN detection');?>:
					<span class="showhelp">
					<?php echo language('VPN detection help','Automatic detection of VPN, if the set time is 2 minutes, then every 2 minutes to detect the connection status with IP (e.g. 10.8.0.1) devices, if not, automatically restart VPN.Time set 0 means no detection.');?>
					</span>
				</div>
			</th>
			<td>
				<select name="minute" id="minute">
				<?php
					for($j=0; $j<=59; $j++) {
						($minute == $j)?$minute_sel='selected':$minute_sel='';
						echo "<option  value='$j' $minute_sel>$j</option>";
					}
				?>
				</select>
				<?php echo language('Minute');?>
				<span style="margin-left:20px;">IP:<input type="text" id="monitorip" name="monitorip" value="<?php echo $monitorip;?>"/></span>
			</td>
		</tr>
	</table>
<br>

	<div class="div_openvpn" style="display:none" >
		<h4><?php echo language('OpenVPN Settings');?></h4>
		<table width="100%" class="tctl">
			<tr id="tr_upload_conf">
				<th>
					<?php echo language('Upload Configuration');?>:<input type="file" name="upload_cfg_file" onchange="return checkFileChange(this)" id="upload_cfg_file"/>
				</th>
				<td>
					<input type="submit" value="<?php echo language('File Upload');?>" 
						onclick="document.getElementById('send').value='File Upload';return upload_cfg_file2()" />
				</td>
			</tr>
		</table>
		<table width="100%" class="tctl">
			<tr>
				<th>
					<textarea name="helpinfo" id="helpinfo" style="width:100%;font-size:12px;Line-height:1.5;background:#E8EFF7;border:none" clos="20" rows="5" readonly>
						   <?php echo language('OpenVPN Configuration Files help'); ?>
					</textarea>
				</th>
				<td width="1px">
				</td>
			</tr>
		</table>
		<table width="100%" class="tctl">
			<tr id="tr_download_samples">
				<th>
					<div class="helptooltips">
					<?php echo language('Sample Configuration');?>
					<span class="showhelp">
					<?php echo language('OpenVPN Sample Configuration help',"It's just a sample configuration which help you to refer to the format. <br/>");?>
					</span> 
					</div>
				</th>
				<td>
					<input type="submit" value="<?php echo language('Download Samples');?>" 
						onclick="document.getElementById('send').value='Download Samples';"/>
				</td>
			</tr>
		</table>
		
		<br/>
		
		<table class="tedit" width="100%">
			<tr>
				<th>
					<div class="helptooltips">
						<span style="color:red">* &nbsp;</span><?php echo language('Connection Status');?>:
					</div>
				</th>
				<td class="openvpn_connect">
				
				</td>
			</tr>
		</table>
		
	</div>

	<div id="div_n2nvpn" style="display:none" >
		<h4><?php echo language('N2N VPN Settings');?></h4>
		
		<table width="100%" class="tedit" >
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Enable');?>:
						<span class="showhelp">
						<?php echo language('Enable help');?>
						</span>
					</div>
				</th>
				<td>
					<input type="checkbox" name="n2n_enable" id="n2n_enable" <?php echo $n2n_enable?> />
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Server Address');?>:
						<span class="showhelp">
						<?php echo language('Server Address help');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="n2n_server_addr" id="n2n_server_addr" value="<?php echo $n2n_server_addr;?>" />
					<span id="cn2n_server_addr"></span>
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Port');?>:
						<span class="showhelp">
						<?php echo language('Port help');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="n2n_port" id="n2n_port" value="<?php echo $n2n_port;?>" />
					<span id="cn2n_port"></span>
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Local IP');?>:
						<span class="showhelp">
						<?php echo language('Local IP help');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="n2n_local_ip" id="n2n_local_ip" value="<?php echo $n2n_local_ip;?>" />
					<span id="cn2n_local_ip"></span>
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Subnet Mask');?>:
						<span class="showhelp">
						<?php echo language('Subnet Mask help');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="n2n_mask" id="n2n_mask" value="<?php echo $n2n_mask;?>" />
					<span id="cn2n_mask"></span>
				</td>
			</tr>
			<!--
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Local Port');?>:
						<span class="showhelp">
						<?php echo language('Local Port help')?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="n2n_local_port" id="n2n_local_port" value="<?php echo $n2n_local_port;?>" />
					<span id="cn2n_local_port"></span>
				</td>
			</tr>-->
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('User Name');?>:
						<span class="showhelp">
						<?php echo language('User Name help');?>
						</span>
					</div>
				</th>
				<td>
					<input type="text" name="n2n_username" id="n2n_username" value="<?php echo $n2n_username;?>" />
					<span id="cn2n_username"></span>
				</td>
			</tr>
			
			<tr>
				<th>
					<div class="helptooltips">
						<?php echo language('Password');?>:
						<span class="showhelp">
						<?php echo language('Password help');?>
						</span>
					</div>
				</th>
				<td>
					<input type="password" name="n2n_password" id="n2n_password" value="<?php echo $n2n_password;?>" />
					<span style="vertical-align:middle;display:inline-block;"><input type="checkbox" id="show_n2n_password" /></span>
					<span id="cn2n_password"></span>
				</td>
			</tr>
			
			<tr id="n2n_show_status" <?php if($exist_array['n2n']['enable'] != 'on'){ echo 'style="display:none;"';}?>>
				<th>
					<div class="helptooltips">
						<span style="color:red">* &nbsp;</span><?php echo language('Connection Status');?>:
						<span class="showhelp">
							<?php echo language('N2N connection status',"N2N server connection status.");?>
						</span>
					</div>
				</th>
				<td class="n2n_connect">
				
				</td>
			</tr>
		</table>
	</div>

	<div id="div_pptpvpn_tab" style="display:none">
	<h4><?php echo language('PPTP VPN Settings');?></h4>
	
	<table width="100%" class="tedit" id="tr_pptp" >
		<tr id="tr_pptp_server" >
			<th>
				<div class="helptooltips">
					<?php echo language('Server');?>:
					<span class="showhelp">
					<?php echo language('PPTP VPN Configuration help',"The pptp's server ip address.");?>
					</span>
				</div>
			</th>
			<td >
				<input type="text" name="pptp_server" id="pptp_server" value="<?php echo $serverip; ?>" />
			</td>
		</tr>
		<tr id="tr_pptp_account" >
			<th>
				<div class="helptooltips">
					<?php echo language('Account');?>:
					<span class="showhelp">
					<?php echo language('PPTP VPN Account help',"pptp vpn account");?>
					</span>
				</div>
			</th>
			<td >
				<input type="text" name="pptp_account" id="pptp_account" value="<?php echo $account; ?>" />
			</td>
		</tr>
		<tr id="tr_pptp_password" >
			<th>
				<div class="helptooltips">
					<?php echo language('Password');?>:
					<span class="showhelp">
					<?php echo language('PPTP VPN Password help',"pptp vpn password");?>
					</span>
				</div>
			</th>
			<td >
				<input type="password" name="pptp_password" id="pptp_password" onpaste="return false" oncopy="return false" value="<?php echo $password; ?>" />
				<span style="vertical-align:middle;display:inline-block;"><input type="checkbox" id="show_pptp_password" /></span>
			</td>
		</tr>
		<!--		
		<tr id="tr_pptp_domain" >
			<th>
				<div class="helptooltips">
					<?php echo language('Domain');?>:
					<span class="showhelp">
					<?php echo language('PPTP VPN Domain help',"pptp server domain");?>
					</span>
				</div>
			</th>
			<td >
				<input type="text" name="pptp_domain" id="pptp_domain" value="<?php echo $domain;?>" />
			</td>
		</tr>
		-->
		<tr id="tr_pptp_mppe" >
			<th>
				<div class="helptooltips">
					<?php echo language('Use MPPE', "Use MPPE");?>:
					<span class="showhelp">
					<?php echo language('PPTP VPN MPPE help',"Use MPPE or not");?>
					</span>
				</div>
			</th>
			<td >
				<input type="checkbox" id="pptp_mppe" name="pptp_mppe" <?php if($mppe){echo 'checked';}?>/>
			</td>
		</tr>
		<tr id="connect_status">
			<th>
				<div class="helptooltips">
					<span style="color:red">* &nbsp;</span><?php echo language('Connection Status');?>:
					<span class="showhelp">
					<?php echo language('pptp-vpn connection status',"pptp-vpn server connection status.");
					?>
					</span>
				</div>
			</th>
			<td class="connect">		
					
			</td>
		</tr>			
	</table>	
	</div>
	
	<div id="div_l2tpvpn_tab" style="display:none">
	<h4><?php echo language('L2TP VPN Settings');?></h4>
	
	<table width="100%" class="tedit" >
		<tr id="tr_pptp_server" >
			<th>
				<div class="helptooltips">
					<?php echo language('Enable');?>:
					<span class="showhelp">
					<?php echo language('Enable help',"Enable");?>
					</span>
				</div>
			</th>
			<td >
				<input type="checkbox" name="l2tp_enable" id="l2tp_enable" <?php echo $l2tp_enable; ?>/>
			</td>
		</tr>
		
		<tr id="tr_pptp_server" >
			<th>
				<div class="helptooltips">
					<?php echo language('Server');?>:
					<span class="showhelp">
					<?php echo language('Server help',"Server");?>
					</span>
				</div>
			</th>
			<td >
				<input type="text" name="l2tp_server" id="l2tp_server" value="<?php echo $l2tp_server; ?>" />
			</td>
		</tr>
		
		<tr id="tr_pptp_server" >
			<th>
				<div class="helptooltips">
					<?php echo language('Username');?>:
					<span class="showhelp">
					<?php echo language('Username help',"Username");?>
					</span>
				</div>
			</th>
			<td >
				<input type="text" name="l2tp_username" id="l2tp_username" value="<?php echo $l2tp_username;?>" />
			</td>
		</tr>
		
		<tr id="tr_pptp_server" >
			<th>
				<div class="helptooltips">
					<?php echo language('Password');?>:
					<span class="showhelp">
					<?php echo language('Password help',"Password");?>
					</span>
				</div>
			</th>
			<td >
				<input type="password" name="l2tp_password" id="l2tp_password" value="<?php echo $l2tp_password; ?>" />
			</td>
		</tr>
		
		<tr id="tr_pptp_server" >
			<th>
				<div class="helptooltips">
					<?php echo language('IPSec');?>:
					<span class="showhelp">
					<?php echo language('IPSec help',"IPSec");?>
					</span>
				</div>
			</th>
			<td >
				<input type="checkbox" name="IPSec" id="IPSec" <?php echo $IPSec; ?> />
			</td>
		</tr>
		
		<tr id="tr_pptp_server" >
			<th>
				<div class="helptooltips">
					<?php echo language('IPSec Password');?>:
					<span class="showhelp">
					<?php echo language('IPSec Password help',"IPSec Password");?>
					</span>
				</div>
			</th>
			<td >
				<input type="password" name="IPSec_password" id="IPSec_password" value="<?php echo $IPSec_password; ?>" />
			</td>
		</tr>
		
		<tr id="connect_status">
			<th>
				<div class="helptooltips">
					<span style="color:red">* &nbsp;</span><?php echo language('Connection Status');?>:
					<span class="showhelp">
					<?php echo language('L2TP connection status',"L2TP connection status.");
					?>
					</span>
				</div>
			</th>
			<td class="l2tp_connect">		
			</td>
		</tr>
	</table>
	</div>
	
	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" value="<?php echo language('Save');?>" style="margin-top: 10px;" onclick="document.getElementById('send').value='Save';" <?php if($__demo_enable__=='on'){echo 'disabled';}?> />
	</form>
	<br />

<script type="text/javascript"> 
function vpntypechange()
{
	var type = document.getElementById('vpn_type').value;

	if(type == 'openvpn') {
		$("#div_pptpvpn_tab").hide();
		$("#div_n2nvpn").hide();
		$(".div_openvpn").show();
		$("#div_l2tpvpn_tab").hide();
	} else if (type == 'pptpvpn') {
		$("#div_n2nvpn").hide();
		$(".div_openvpn").hide();
		$("#div_pptpvpn_tab").show();
		$("#div_l2tpvpn_tab").hide();
	}else if(type == 'n2nvpn'){
		$("#div_n2nvpn").show();
		$("#div_pptpvpn_tab").hide();
		$(".div_openvpn").hide();
		$("#div_l2tpvpn_tab").hide();
	}else if(type == 'l2tpvpn'){
		$("#div_n2nvpn").hide();
		$("#div_pptpvpn_tab").hide();
		$(".div_openvpn").hide();
		$("#div_l2tpvpn_tab").show();
	}else{
		$("#div_n2nvpn").hide();
		$("#div_pptpvpn_tab").hide();
		$(".div_openvpn").hide();
		$("#div_l2tpvpn_tab").hide();
	}
}

var g_bAllowFile = false;

function checkFileChange(obj)
{
	var filesize = 0;  
	var  Sys = {};  

	if(navigator.userAgent.indexOf("MSIE")>0){
		Sys.ie=true;  
	} else
	//if(isFirefox=navigator.userAgent.indexOf("Firefox")>0)  
	{  
		Sys.firefox=true;  
	}
	   
	if(Sys.firefox){  
		filesize = obj.files[0].fileSize;  
	} else if(Sys.ie){
		try {
			obj.select();
			var realpath = document.selection.createRange().text;
			//alert(obj.value);
			//alert(realpath);
			var fileobject = new ActiveXObject ("Scripting.FileSystemObject");//获取上传文件的对象  
			//var file = fileobject.GetFile (obj.value);//获取上传的文件  
			var file = fileobject.GetFile (realpath);//获取上传的文件  
			var filesize = file.Size;//文件大小  
		} catch(e){
			alert("Please allow ActiveX Scripting File System Object!");
			return false;
		}
	}

	if(filesize > 1000*1000*40) {
		alert("Uploaded max file is 40M!");
		g_bAllowFile = false;
		return false;
	}

	g_bAllowFile = true;
	return true;
}
var connect ="<?php echo language('connected');?>";
var connect_ok="<span style='color:green'>"+connect+"</span>";

function get_status()//pptp
{
	$.ajax({
		type:'GET',
		url:'/cgi-bin/php/ajax_server.php?type=pptpvpn_status',
		success: function(data) {
			var data = $.trim(data);
			if(data == 'OK'){
				$('.connect').html(connect_ok);
			} else if(data == ''){
				$('.connect').html('<img src="/images/loading.gif"/>');
			} else{
				var connect_failed = "<span style='color:red'><?php echo language('Failed to connect');?></span>";
				$('.connect').html(connect_failed);
			}
		}
	})
}

function get_n2n_status(){//N2N
	$.ajax({
		type:'GET',
		url:'/cgi-bin/php/ajax_server.php?type=n2n_status',
		success:function(data){
			var data = $.trim(data);
			if(data == 'OK'){
				$('.n2n_connect').html(connect_ok);
			} else if(data == ''){
				$('.n2n_connect').html('<img src="/images/loading.gif"/>');
			} else{
				var connect_failed = "<span style='color:red'><?php echo language('Failed to connect');?></span>";
				$('.n2n_connect').html(connect_failed);
			}
		}
	});
}

function get_l2tp_status(){
	var vpn_type = document.getElementById('vpn_type').value;
	
	if(document.getElementById('l2tp_enable').checked && vpn_type == 'l2tpvpn'){
		$.ajax({
			type:'GET',
			url:'/cgi-bin/php/ajax_server.php?type=l2tpvpn_status',
			success:function(data){
				var data = $.trim(data);
				if(data == 'OK'){
					$('.l2tp_connect').html(connect_ok);
				}else if(data == '' && time < 30){
					$('.l2tp_connect').html('<img src="/images/loading.gif"/>');
					setTimeout("get_l2tp_status()",1000);
					time++;
				}else if(time == 30){
					var connect_failed = "<span style='color:red'><?php echo language('Failed to connect');?></span>";
					$('.l2tp_connect').html(connect_failed);
				}
			}
		});
	}else{
		var connect_failed = "<span style='color:red'><?php echo language('Failed to connect');?></span>";
		$('.l2tp_connect').html(connect_failed);
	}
}

var time = 0;
function get_openvpn_status_after_file_upload(){
	$.ajax({
		type:'GET',
		url:'/cgi-bin/php/ajax_server.php?type=openvpn_status',
		success:function(data){
			var data = $.trim(data);
			
			<?php if($_POST && $_POST['vpn_type'] != 'openvpn'){ ?>
			$('.openvpn_connect').html("");
			return false;
			<?php } ?>
			
			if(data == 'OK'){
				$('.openvpn_connect').html(connect_ok);
			}else {
				time++;
				if(time < 20){
					$('.openvpn_connect').html('<img src="/images/loading.gif"/>');
					setTimeout("get_openvpn_status_after_file_upload()",1000);
				}else{
					var connect_failed = "<span style='color:red'><?php echo language('Failed to connect');?></span>";
					$('.openvpn_connect').html(connect_failed);
				}
			}
		}
	});
}

function get_openvpn_status(){
	$.ajax({
		type:'GET',
		url:'/cgi-bin/php/ajax_server.php?type=openvpn_status',
		success:function(data){
			var data = $.trim(data);
			if(data == 'OK'){
				$('.openvpn_connect').html(connect_ok);
			}else {
				var connect_failed = "<span style='color:red'><?php echo language('Failed to connect');?></span>";
				$('.openvpn_connect').html(connect_failed);
			}
		}
	});
}

function onload_func()
{
	vpntypechange();
}
	
$(document).ready(function (){ 
	$("#n2n_enable").iButton();
	$("#l2tp_enable").iButton();
	//$(":checkbox").iButton(); 
	onload_func();
	get_status();
	
	get_n2n_status();
	
	get_l2tp_status();
	
	<?php if($_FILES){ ?>
	get_openvpn_status_after_file_upload();
	<?php }else{ ?>
	get_openvpn_status();
	<?php }?>
	
	$("#apply").click(function(){
		$('.connect').html('<img src="/images/loading.gif"/>');
		setTimeout("get_status()", 30000);
		
		$('.n2n_connect').html('<img src="/images/loading.gif"/>');
		setTimeout("get_n2n_status()",10000);
		
		$('.l2tp_connect').html('<img src="/images/loading.gif"/>');
		setTimeout("get_l2tp_status()",100);
		
		get_openvpn_status_after_file_upload();
	});
	
	$("#n2n_enable").change(function(){
		if($(this).attr('checked') == 'checked'){
			$("#n2n_show_status").show();
		}else{
			$("#n2n_show_status").hide();
		}
	});
}); 

$("#show_n2n_password").change(function(){
	if($(this).attr("checked") == 'checked'){
		$("#n2n_password").prop("type","text");
	}else{
		$("#n2n_password").prop("type","password");
	}
});

$("#show_pptp_password").change(function(){
	if($(this).attr("checked") == 'checked'){
		$("#pptp_password").prop("type","text");
	}else{
		$("#pptp_password").prop("type","password");
	}
});

</script>


<?php 

function download_sample_file()
{
	$filename = "openvpn.sample_configs.tar.gz";
	$cfg_name = "/version/" . $filename;

	if(!file_exists($cfg_name)) {
		echo "</br>$cfg_name";
		echo language("Can not find");
		return;
	}

	//打开文件  
	$file = fopen($cfg_name, "r"); 
	$size = filesize($cfg_name);

	//输入文件标签 
	header('Content-Encoding: none');
	header('Content-Type: application/force-download');
	header('Content-Type: application/octet-stream');
	header('Content-Type: application/download');
	header('Content-Description: File Transfer');  
	header('Accept-Ranges: bytes');  
	header("Accept-Length: $size");  
	header('Content-Transfer-Encoding: binary');
	header("Content-Disposition: attachment; filename=$filename"); 
	header('Pragma: no-cache');
	header('Expires: 0');
	//输出文件内容   
	//读取文件内容并直接输出到浏览器
	ob_clean();
	flush();
	echo fread($file, $size);
	fclose ($file);
}

if($_POST){ 
	if(isset($_POST['send'])) {
		if($_POST['send'] == 'File Upload') {
			
			if(strstr($_FILES['upload_cfg_file']['name'], 'tar.gz')){
				upload_cfg_file();
			}else{
				upload_ovpn_file();
			}
			save_vpn_cfg('openvpn');
			save_openvpn_operate_server('1');
			save_to_flash('/etc/asterisk','/etc/cfg');
		} else if($_POST['send'] == 'Download Samples') {
			download_sample_file();
		}
	}
}

require("/www/cgi-bin/inc/boot.inc");
?>
