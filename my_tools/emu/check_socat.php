<?php
include_once("/www/cgi-bin/inc/cluster.inc");

$ip = $argv[1];

$ret = check_socat($ip);
if (!$ret)
	start_socat($ip);

// socat正在运行，返回true，否则返回false
// $ip为空则是主板
function check_socat($ip="")
{
	if (strlen($ip) > 0)
	{
		$syscmd = "syscmd:ps -ef | grep socat | grep -v grep | grep -v check_socat | cut -c 0-5";
		$res = request_slave($ip, $syscmd, 5, true);
		printf("check_socat: slave\n", $ip);
	}
	else
	{
		$syscmd = "ps -ef | grep socat | grep -v grep | grep -v check_socat | cut -c 0-5";
		$res = system($syscmd);
		printf("check_socat: master\n", $ip);
	}
	if (strlen($res) > 1)
		return true;
	else
		return false;
}

function start_socat($ip="")
{
	//
	if (strlen($ip) > 0)
	{
		$syscmd = "syscmd:/my_tools/socat udp4-listen:5501,reuseaddr,fork file:/dev/ttyS1,nonblock,raw,echo=0,b115200,waitlock=/var/run/ttyS1.lock > /dev/nulll &";
		$res = request_slave($ip, $syscmd, 5, false);
		printf("start_socat: slave\n", $ip);
	}
	else
	{
		$syscmd = "/my_tools/socat udp4-listen:5501,reuseaddr,fork file:/dev/ttyS1,nonblock,raw,echo=0,b115200,waitlock=/var/run/ttyS1.lock > /dev/null &";
		$res = system($syscmd);
		printf("start_socat: master\n", $ip);
	}
	return true;
}



?>
