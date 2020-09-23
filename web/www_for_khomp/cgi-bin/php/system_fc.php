<?php
include_once("/www/cgi-bin/inc/function.inc");
include_once("/www/cgi-bin/inc/cluster.inc");
include_once("/www/cgi-bin/inc/wrcfg.inc");
?>
<?php
$cur_cfg_version = trim(@file_get_contents('/etc/asterisk/cfg_version'));
$cur_sys_version = trim(@file_get_contents('/version/version'));
//$newest_sys_version = trim(@file_get_contents('http://downloads.openvox.cn/pub/firmwares/GSM%20Gateway/current-version'));
$cluster_info = get_cluster_info();
$g_restore_cfg_file = false;
function res_def_cfg_file()
{
	global $cluster_info;
	global $__BRD_SUM__;
	global $__BRD_HEAD__;

	for($b=2; $b<=$__BRD_SUM__; $b++) {
		if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
			trace_output_newline();
			$data = "syscmd:/my_tools/restore_cfg_file > /dev/null 2>&1\n";
			$ip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
			request_slave($ip, $data, 5, false);
		}
	}
	exec("/my_tools/restore_cfg_file > /dev/null 2>&1 || echo $?",$output);
	global $g_restore_cfg_file;
	$g_restore_cfg_file = true;
}
function system_reboot()
{
	global $cluster_info;
	global $__BRD_HEAD__;
	global $__BRD_SUM__;

	for($b=2; $b<=$__BRD_SUM__; $b++) {
		if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
			$data = "syscmd:reboot > /dev/null 2>&1\n";
			$ip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
			request_slave($ip, $data, 5, false);
		}
	}
	exec("reboot > /dev/null 2>&1");
}
function ast_reboot()
{
	global $cluster_info;
	global $__BRD_HEAD__;
	global $__BRD_SUM__;

	for($b=2; $b<=$__BRD_SUM__; $b++) {
		if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
			$data = "syscmd:/etc/init.d/asterisk restart > /dev/null 2>&1\n";
			$ip = $cluster_info[$__BRD_HEAD__.$b.'_ip'];
			request_slave($ip, $data, 5, false);
		}
	}
	exec("/etc/init.d/asterisk restart > /dev/null 2>&1 || echo $?",$output);
}
?>

