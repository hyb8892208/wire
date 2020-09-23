require'package_path'
require'get_redis_client'

local redis_port = 6379
local redis_ip_local = "127.0.0.1"
local redis = require'redis'

local FGSM = 0				--from GSM
local TGSM = 8				--to GSM
local TIMES = 3				--send command max times

local NOPULSE = ""			--opvxg4xx nopulse parameter
print "Start get module type..."

--[[
os.execute("echo \"0\" > /proc/gsm_module_power_key-0")
os.execute("echo \"1\" > /proc/gsm_module_power_key-0")
os.execute("sleep 1")

local i = 0
local module_type = "none"
for i = 0, TIMES do
	for j = 1, TGSM do
		run_cmd = "/my_tools/printmod.out -c " .. j .. " -a ATI"
		os.execute(run_cmd)
		os.execute("sleep 1")
		local result_cmd = "cat /sys/module/opvxg4xx/parameters/at_cmd" 
		local f = io.popen(result_cmd)
		--local f = io.popen("/mytools/printmod.out -c 0 -t 3")
		local str = f:read("*all")
		str = string.lower(str)
		--print(str)
		if string.find(str, "sim840")  then
			module_type = "sim840"
			break
		elseif string.find(str, "m35") then
			module_type = "m35"
			break
		elseif string.find(str, "uc15e") then
			module_type = "uc15e"
			break
		elseif string.find(str, "uc15a") then
			module_type = "uc15a"
			break
		elseif string.find(str, "uc15t") then
			module_type = "uc15t"
			break
		else
			module_type = "none"
		end
	end
	if module_type ~= "none" then 
		break
	end
end
--]]
--set default module type "sim840" -> "gsm"
local module_type = "sim840"

-- insert to redis
print("insert module type to redis......")
print("module type is "..module_type)
local client = get_redis_client(redis_ip_local,redis_port)
client:set("local.product.module.type",module_type)
