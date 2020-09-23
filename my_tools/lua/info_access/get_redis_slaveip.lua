--######################################
--provide functions for external calls
--a. in the head : some require & local define

--b. functions list & features
--	1. get_redis_slaveip():get redis_ip 
--			if master ip = "127.0.0.1"
--			if slave  get ip from file "/etc/asterisk/gw/cluster.conf"
--		return a table : for example 
--					1   127.0.0.127
--					2   192.168.xx.xx
--
--c. in the end : 
--   simple test codes
--++++++++++++++++++++++++++++++++
--#######################################

function get_redis_slaveip()
	local f = assert(io.open("/etc/asterisk/gw/cluster.conf","r"))
	local fstr = f:read("*all")
	f:close()
	local slave_ip
	local t_ip = {}

	
	
	if string.match(fstr,"%s*mode%s*=%s*master%s*") then
		t_ip["1"] = "127.0.0.1"
		for num,ip in string.gmatch(fstr,"Board%-(%d+)_ip=(%d+%.%d+%.%d+%.%d+)") do
			t_ip[num] = ip
		end
	else
		t_ip["1"] = "127.0.0.1"
	end
	return t_ip
end

--===============================================
--[[
local ip = get_redis_slaveip()
table.foreach(ip,print)
--]]
