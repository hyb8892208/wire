require'package_path'
require'logging.rolling_file'
local redis = require'redis'
local socket = require'socket'

function get_redis_client(redis_ip,redis_port)
	local logger = logging.rolling_file("/tmp/log/lua/redis_connect.log",1048576)
	local result 
	local logger_err_str  
	local logger_suc_str 
	local starttime = os.time()

	while true do
		--result = redis_port_check(redis_ip,redis_port)
		local sock,err = socket.tcp()
		if not sock then
			print("sock failed")
		end
		sock:settimeout(10)
		local result,err = sock:connect(redis_ip,redis_port)
		
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
	end
	if logger_err_str ~= nil then
		print(logger_err_str)
	end
	if logger_suc_str ~= nil then
		print(logger_suc_str)
	end

	--logger:info(logger_suc_str)
	local client = redis.connect(redis_ip,redis_port)
	return client
end
--[[
local redis_ip = "127.0.0.1" 
local redis_port = 6379 
local redis_timeout = 0 
local client = get_redis_client(redis_ip,redis_port)
local str = client:get("hello")
print(str)

print(os.time())
os.execute("sleep 1")
print(os.time())

print("end__________________")
--]]
