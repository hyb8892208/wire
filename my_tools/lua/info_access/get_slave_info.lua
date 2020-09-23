--######################################
--provide functions for external calls
--a. in the head : some require & local define

--b. functions list & features
--	1. get_slave_info():get slave infomation 
--		input para:   slaveip   -- redis ip	
--					  keystr    -- get one value or more
--								-- key type string or table
--		return a table or string about info
--
--c. in the end : 
--   simple test codes
--++++++++++++++++++++++++++++++++

--#######################################
require'package_path'
require'logging.rolling_file' 
local redis = require'redis'
local socket = require'socket'
require'get_redis_slaveip'
--redis define
local redis_port =6379
--++++++++++++++++++++++++++++++
--####################################
function redis_port_check(redis_ip,redis_port,timeout)
	local logger = logging.rolling_file("/tmp/log/lua/redis_connect.log",1048576)
	local result 
	local logger_err_str 
	local logger_suc_str
	local starttime = os.time()

	local timeout = timeout
	--print("timeout = "..timeout)
	for i=0,timeout do
		--result = redis_port_check(redis_ip,redis_port)
		local sock,err = socket.tcp()
		if not sock then
			print("sock connect failed...")
		end
		sock:settimeout(3)
		local result,err = sock:connect(redis_ip,redis_port)
		if i >=2 then
			if err == "No route to host" or err == "timeout" then
				return "null"
			end
		end
		if not result then
			logger_err_str = "[ERROR]Could not connect to "..redis_ip..':'..redis_port..' ['..err..']'
			local nowtime = os.time()
			local timeheart = (nowtime - starttime)%2
			if timeheart == 1 then
				logger:error(logger_err_str)
			end
			starttime = nowtime
		else
			logger_suc_str = "[info]Connect to "..redis_ip..':'..redis_port..' success.'
			break
		end
		os.execute("sleep 1")
		if i == timeout  then
			print("Connect redis timeout")
			return "unknown"
		end
	end
	--print(logger_err_str)
	--print("logger_suc: "..logger_suc_str)

	--logger:info(logger_suc_str)
	
	local client = redis.connect(redis_ip,redis_port)
	return client
end
--####################################
	--get_slave_info
	--slaveip: get which board infomation
	--keystr: a string, get key  ==> value
	--		  a table , get allkey  ==> values
	---------------------
-- type ++
function get_slave_info(slaveip,keystr,timeout)
	local client = redis_port_check(slaveip,redis_port,timeout)
	if type(client) == "string" then
		return client
	end
	print("get redis client ok")
	if (type(keystr) == "string")  then
		local times = 5
		for i = 0,times do
			local info = client:get(keystr)
			if info ~=nil then
				return info
			end
			os.execute("sleep 1")
			if i == times then
				print("get redis data timeout")
				return "none" 
			end 
		end
	elseif (type(keystr) == "table") then
		local t = {}
		for k,v in pairs(keystr) do
			local times = 3
			for i=0,times do
				v = client:get(k) 
				if v ~= nil then
					t[k] = v
					break
				end
				os.execute("sleep 1")
				if i == times then
					print("get redis data timeout")
					t[k] = "none"
					break
				end
			end
		end
		return t
	end
end

--##################################
--[[
require'get_info_tools'
ip = "127.0.0.1"
--ip = "192.168.123.1"
local str = get_slave_info(ip,"local.product.module.ty",3)
print(str)
t = init_product_table()
local str = get_slave_info(ip,t,3)
table.foreach(str,print)

--]]
--[[
--]]
