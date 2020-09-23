<?php
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/cluster.inc");

function do_apply()
{
	global $__deal_cluster__;
	$wait_apply_file = "/tmp/web/wait_apply";
	
	if(!file_exists($wait_apply_file) || filesize($wait_apply_file) <=0)
		return false;

	$fh = lock_file($wait_apply_file);
	$wait_apply = file_get_contents($wait_apply_file);
	file_put_contents($wait_apply_file,''); //Clear file immediately. Freedom 2014-08-12 11:11.
	unlock_file($fh);

	$apply_array = explode("\n", $wait_apply);

	if(!is_array($apply_array)){
		return false;
	}

	foreach($apply_array as $line) {
		$apply = explode("@", $line);
		if(!isset($apply[0])) continue;
		switch($apply[0]) {
			case "exec":
				exec($apply[1]);
				break;
			case "request_slave":
				request_slave($apply[1], $apply[2]."\n", 10, false);
				break;
		}
	}

	//Freedom delete 2014-08-12 11:11
	/*$fh = lock_file($wait_apply_file);
	$handle = fopen($wait_apply_file,"w");
	//ftruncate($handle, 0);
	fclose($handle);
	unlock_file($fh);*/

	
	if($__deal_cluster__){
		global $__BRD_SUM__;
		global $__BRD_HEAD__;
		$cluster_info = get_cluster_info();
		if($cluster_info['mode'] == 'master') {
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					$slaveip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
					//Freedom Add don't use web server, use php CLI, not need web server password. 2014-08-12 11:11
					//$url = 'http://'.$slaveip.'/cgi-bin/php/apply.php?apply=yes';
					//gw_send_GET($url);
					request_slave($slaveip, "syscmd:php /www/cgi-bin/php/apply.php apply > /dev/null 2>&1", 5, false);
				}
			}   
		}   
	}
	
	return true;
}

if(isset($_GET['apply'])){
	do_apply();
	save_to_flash('/etc/asterisk','/etc/cfg');
}

if(isset($argv[1]) && !strcmp($argv[1],'apply')) {
	do_apply();
	save_to_flash('/etc/asterisk','/etc/cfg');
}


?>
