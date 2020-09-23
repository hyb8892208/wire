--######################################
--get module type info by redis ,save the info to a file
--a. in the head : some require & local define

--b. functions list & features
--	1. save_module_type():save module_type into file
--		input para:  file_name  --- the file name 
--					 module_type_t
--			if slave  get ip from file "/etc/asterisk/gw/cluster.conf"
--		return a table : for example 
--					1   127.0.0.127
--					2   192.168.xx.xx
--
--c. in the end : 
--   simple test codes
--++++++++++++++++++++++++++++++++
--#######################################

require'package_path'
require'get_redis_slaveip'
require'get_slave_info'

local redis_port = 6379
local redis = require'redis'

local board_type = 32
local board_sum = 1

local module_uc15 = "UC15"
local module_uc15a = "UC15A"
local module_uc15e = "UC15E"
local module_uc15t = "UC15T"
local module_sim840 = "SIM840"
local module_m35 = "M35"

local default_module_type = "sim840"

function save_module_type(file_name,module_type_t,line_n,board_type)
	local f = assert(io.open(file_name,"w"))
	local file_str = ""
	for i = 1,line_n do
		local module_name = ""
		if module_type_t[tostring(i)] then
			module_name = module_type_t[tostring(i)]
		else 
			module_name = default_module_type
		end
		local str = module_name;
		for j = 1, board_type -1  do
			str =  str .. "," .. module_name
		end
		file_str = file_str .. str .. "\n"
	end
--	file_str = string.sub(file_str,0,-2)
--	print(file_str)
	f:write(file_str)
	f:close()
end

--
--#########################################
local ip_table = get_redis_slaveip()
--table.foreach(ip_table,print)
local info_str = "local.product.module.type"
local module_type_t = {}
for num,ip in pairs(ip_table) do
	module_type_t[num] = get_slave_info(ip,info_str,5)
end
save_module_type("/tmp/.module_type",module_type_t,board_sum,board_type)



