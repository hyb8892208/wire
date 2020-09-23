--######################################
--provide functions for external calls
--a. in the head : some require & local define

--b. functions list & features
--	1. init_product_table():define all the info about product
--	2. init_sys_table():define all the info about system
--	3. get_product_info(): get all the info about product 
-- 			by call function : get_product_XXX(),get each value need
--	4. get_sys_info(): get all the info about system
-- 			by call function : get_sys_XXX(),get each value need
--	5. insert_info_to_redis(): insert all the info into local redis
--
--c. in the end : 
--   run codes , save info to redis
--++++++++++++++++++++++++++++++++
--#######################################
require'package_path'
require'get_redis_client'
require'get_info_tools'
local t_product_info = {}
local t_sys_info = {}
local t_oem_info = {}
function get_product_board_span()
	local hw_info = '/tmp/hw_info.cfg'
	local readfile = io.open(hw_info, "r")
	local board_spans = ''
	assert(readfile)

	for str in readfile:lines() do
		if string.find(str, "total_chan_count=") then
			local value = string.gsub(str, "total_chan_count=", "")
			board_spans = value
			break
		end
	end
	readfile:close()
	return board_spans
end

function get_product_board_type()
	cmd_str = "cat /tmp/hw_info.cfg |grep ^sys_type |awk -F '=' '{printf $2}'"
	local value=run_linux_cmd(cmd_str)
	if value == nil then
		value = 0
	end
	return value
end

local board_span = get_product_board_span() 
--redis define
local redis_port =6379
local redis_ip_local = "127.0.0.1"
local redis = require'redis'

--+++++++++++++++++++++++++++++++
--module type define
local module_uc15 = "UC15"
local module_uc15a = "UC15A"
local module_uc15e = "UC15E"
local module_uc15t = "UC15T"
local module_sim840 = "SIM840"
local module_m35 = "M35"
--+++++++++++++++++++++++++++++++
--###########################################################
--
--board_type --name
function run_ati_cmd(spans_num)
	local t = {}
	for i = 1,spans_num do
		local str = "asterisk -rx \"gsm send sync at " .. i .. " ati 5000 \""
		table.insert(t,str)
	end
	table.foreach(t,print)
	
	local j = 0
	while true do
		local ast_status = os.execute("ps -ef | grep \"asterisk -g\" | grep -v grep")
		if ast_status == 0 then
			for i = 1, #t do
				local command_f = io.popen(t[i])
				local command_ret = command_f:read("*a")
				if string.find(command_ret,module_uc15) or string.find(command_ret,module_sim840) or string.find(command_ret,module_m35) then
					return command_ret
				end
			end
			os.execute("sleep 1")
		else
			j = j + 1
			os.execute("sleep 1")
			if j >= 15 then
				return"SIM840"
			end
		end
	end
end


--######################################################
--get product infomation

--read conf file
function get_conf_str(file_path,key)
	local f = io.open(file_path,"r")
	local str = f:read("*a")
	f:close()
	
	local pattern = key.."=(.-)\n"
	local value = string.match(str,pattern)
	return value	
end
function get_product_num()
	local product_num = 0
	return product_num	
end

function get_product_sw_version()
	local cmd_str = "cat /version/version"
	local res_str = run_linux_cmd(cmd_str)
	res_str = string_trim(res_str)
	return  res_str
end

function get_product_sw_buildtime()
	local cmd_str = "cat /version/build_time"
	local info_str = run_linux_cmd(cmd_str)
	info_str = string_trim(info_str)
	return info_str
end

function get_product_serialnumber()
	local file_path = "/etc/asterisk/gw/network/lan.conf"
	local info_str = get_conf_str(file_path,"mac")
	local serialnumber = string.gsub(info_str,":","")
	return serialnumber
end

function get_product_board_version()
	local cmd_str = "cat /version/hardware_version"
	local res_str = run_linux_cmd(cmd_str)
	res_str = string_trim(res_str)
	return  res_str
end

function get_product_module_type()
	local board_span = board_span
	local command_ret = run_ati_cmd(board_span)
	if string.find(command_ret,module_uc15a) then
		module_name = "uc15a"
	elseif string.find(command_ret,module_uc15e) then
		module_name = "uc15e"
	elseif string.find(command_ret,module_uc15t) then
		module_name = "uc15t"
	elseif string.find(command_ret,module_sim840) then
		module_name = "sim840"
	elseif string.find(command_ret,module_m35) then
		module_name = "m35"
	else
		module_name = "sim840"
	end
	print("*************")
	print(module_name)
	print("*************")
	return module_name
end
--product:means product 
function get_product_info()
	local info = {}
	--info["local.product.module.type"] = get_product_module_type()
	info["local.product.number"] = get_product_num()
	info["local.product.hw.version"] =  ""
	info["local.product.hw.type"] = ""
	info["local.product.sw.version"] = get_product_sw_version()
	info["local.product.sw.buildtime"] = get_product_sw_buildtime()
	info["local.product.sw.customid"] = ""
	info["local.product.serialnumber"] = get_product_serialnumber()
	info["local.product.board.type"] = get_product_board_type()
	info["local.product.board.span"] = get_product_board_span()
	info["local.product.board.version"] = get_product_board_version()
	info["local.product.sw.build"] = ""
	
	return info
end

--###################################################################
--get system info

function get_sys_cpu()
	local cmd_str = "cat /proc/cpuinfo "
	local res_str = run_linux_cmd(cmd_str)
	res_str = string_trim(res_str)
	return  res_str
end

--more
function get_sys_cpu_bit()
	--[[
	local cmd_str = " cat /proc/cpuinfo | grep flags | grep ' lm ' | wc -l"
	local res_str = run_linux_cmd(cmd_str)
	--print(type(res_str))
	res_str = tonumber(res_str)
	if res_str > 0 then
		return 64
	else 
		return 32
	end
	--]]
	return 0
end

function get_sys_slot_num()
	local cmd_str = "cat /tmp/.slotnum"
	local res_str = run_linux_cmd(cmd_str)
	--print(type(res_str))
	return tonumber(res_str)
end

function get_sys_memory()
	local cmd_str = " free | awk 'NR==4{printf(\"total:%d,free:%d\",$2/1024,$4/1024)}'";
	local res_str = run_linux_cmd(cmd_str)
	print(type(res_str))
	return res_str
end

function get_sys_kernel()
	local cmd_str = "uname -a"   -- or uname -r : get only version number
	local res_str = run_linux_cmd(cmd_str)
	res_str = string_trim(res_str)
	print(type(res_str))
	return res_str
end

function get_sys_kernel_buildtime()
	local cmd_str = " uname -a | awk 'NR==1{printf(\"%s %s %s %s %s %s\",$6,$7,$8,$9,$10,$11)}'"
	local res_str = run_linux_cmd(cmd_str)
	print(type(res_str))
	return res_str
end

function get_sys_net_iface()
	local cmd_str = "ifconfig | grep 'Link encap' | awk '{printf(\"%s \",$1)}'";
	local res_str = run_linux_cmd(cmd_str)
	print(type(res_str))
	return res_str
end

function get_sys_eth0_ip()
	local cmd_str = "ifconfig eth0 | awk 'NR==2{print $2}' | tr -d 'addr:'";
	local res_str = run_linux_cmd(cmd_str)
	res_str = string_trim(res_str)
	print(type(res_str))
	print(string.len(res_str))
	return res_str	
end

function get_sys_serialnumber()
	local cmd_str = "cat /data/info/device_info|grep serialnumber|awk -F '=' '{printf(\"%s\",$2)}'"
	local res_str = run_linux_cmd(cmd_str)
	if cmd_str == nil then
		res_str=""
	end
	return res_str
end

--get_sys_info: list most of all the infomation about the system
function get_sys_info()
	local t_sys_info = {}
	t_sys_info["local.system.cpu"] = get_sys_cpu()
	t_sys_info["local.system.cpu.bit"] = get_sys_cpu_bit()
	t_sys_info["local.system.slotnum"] = get_sys_slot_num()
	t_sys_info["local.system.memory"] = get_sys_memory()
	t_sys_info["local.system.kernel"] = get_sys_kernel()
	t_sys_info["local.system.kernel.build.time"] = get_sys_kernel_buildtime()
	t_sys_info["local.system.net.iface"] = get_sys_net_iface()
	t_sys_info["local.system.eth0.ip"] = get_sys_eth0_ip()
	t_sys_info["local.system.uboot.boot"] = ""
	t_sys_info["local.system.boot.check"] = ""
	t_sys_info["local.system.boot.error"] = ""
	t_sys_info["local.system.firmware.version"] = ""
	t_sys_info["local.system.firmware.build"] = ""
	t_sys_info["local.system.serialnumber"] = get_sys_serialnumber()
	
	return t_sys_info
	
end

--###################################################################
--get oem info
function get_oem_info()
	local t_oem_info = {}
	local t_oem_ver_ctl_conf = {}
	local file_path = "/etc/oem_ver_ctl.conf"
	local rfile = io.open(file_path,"r")
	assert(rfile)
	for str in rfile:lines() do
		table.insert(t_oem_ver_ctl_conf,str)
	end
	rfile:close()
	
	local pattern = "([%w_]+)=(.+)"
	for line_key,line_value in pairs(t_oem_ver_ctl_conf) do
		
		for conf_key, conf_value in string.gmatch(line_value,pattern) do
			if conf_key ~=nil and conf_value ~= nil then
				conf_value = string_trim(conf_value)
				t_oem_info[conf_key] = conf_value
			end
		end
		
	end
	table.foreach(t_oem_info,print)
	return t_oem_info
	
end
--####################################################################
--insert all the infomation into local redis 
--info : a table , contains all the info
--redis_type : mean redis support datatype,such as "string,list,hash,sets"
--type_name : now use for "hash name"
function insert_info_to_redis(info,client,redis_type,type_name)
	local client = client 
	local info = info
	table.foreach(info,print)
	print("-----start----")
	if redis_type == "string" then
		for name,value in pairs(info) do
			client:set(name,value)
		end
	elseif redis_type == "hash" then
		print(type_name)
		for name,value in pairs(info) do			
			client:hset(type_name,name,value)
		end
	end
end
--######################################

--run 
starttime = os.date()

--+++++++ get product & system infomation +++++++++
t_product_info = init_product_table()
t_product_info = get_product_info()
table.foreach(t_product_info,print)

t_sys_info = init_sys_table()
t_sys_info = get_sys_info()
table.foreach(t_sys_info,print)

--get oem infomations
t_oem_info = get_oem_info()
table.foreach(t_oem_info,print)
--+++++++ save infomation in redis   +++++++++
local client = get_redis_client(redis_ip_local,redis_port)
insert_info_to_redis(t_product_info,client,"string")
insert_info_to_redis(t_sys_info,client,"string")
--save oem infomations to redis
local oem_hash_name = "local.product.oem.ver.ctl"
insert_info_to_redis(t_oem_info,client,'hash',oem_hash_name)
endtime = starttime .. "\n"..os.date()
print("########")
print(endtime)



