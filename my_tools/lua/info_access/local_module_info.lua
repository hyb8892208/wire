######################################
require'package_path'
require'get_redis_client'
local board_span = 8
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

function timeout_increase(times)
	local timeout = 1
	local i
	for i=1,times do
		timeout = timeout * 2
	end
	if timeout >= 20 then
		timeout = 20
	end
	return timeout
end

function module_split(command_ret)
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
		module_name = "other"
	end
	return module_name
end

function get_module_name(spans_num)
	local t = {}
	for i = 1,spans_num do
		local str = "asterisk -rx \"gsm send sync at " .. i .. " ati 5000 \""
		table.insert(t,str)
	end
	--table.foreach(t,print)
	
	local j = 0
	local ast_times = 1
	local module_type_name = "init"
	local success_times = 0
	local fail_times = 0
	while true do
		print("\n#######while start#########")
		local ast_status = os.execute("ps -ef | grep \"asterisk -g\" | grep -v grep")
		if ast_status == 0 then
			j = j + 1
			local module_type_tmp
			for i = 1, #t do
				local command_f = io.popen(t[i])
				local command_ret = command_f:read("*a")
				if string.find(command_ret,module_uc15) or string.find(command_ret,module_sim840) or string.find(command_ret,module_m35) then
					module_type_tmp = module_split(command_ret)
					break
				else 
					module_type_tmp = "OTHER"
				end
			end
			if module_type_tmp ~= "OTHER" then
				if module_type_name == "init" then
					module_type_name = module_type_tmp
				else
					if module_type_name == module_type_tmp then
						success_times = success_times + 1
						fail_times = 0
						local sleep_str = "sleep "..success_times
						os.execute(sleep_str)
					else
						success_times = 0
						fail_times = fail_times + 1
						module_type_name = module_type_tmp
						local sleep_str = "sleep "..timeout_increase(fail_times)
						os.execute(sleep_str)
					end
					if success_times >=3  then
						if module_type_name ~= "OTHER" then
							return module_type_name
						else
							success_times = 0
						end
					end
				end
			else
				local sleep_str = "sleep "..timeout_increase(j)
				print(sleep_str)
			end
				
				
			print(success_times.." success............")
			print(fail_times.." fail.............")
			print("module_tmp: "..module_type_tmp)
			print("module_type: "..module_type_name)
			print("..............................")
			--send cmd failed
			print("j = "..j)
			if j >= 10 then
				print("SPAN_BROKEN")
			end
		else
			--check asterisk fail
			local ast_timeout = timeout_increase(ast_times)
			local sleep_str = "sleep "..ast_timeout
			print(sleep_str)
			os.execute(sleep_str)
			ast_times = ast_times + 1
		end
		
	end
end

--####################################################################
starttime = os.date()

local module_type_name = get_module_name(board_span)
print(module_type_name)
local redis_times = 0
while true do
	local redis_status = os.execute("ps -ef | grep \"/my_tools/redis-server \" | grep -v grep")
	if redis_status == 0 then
		break
	else 
		--print("redis start failed")
		redis_times = redis_times + 1
		local sleep_str = "sleep "..timeout_increase(redis_times)
		--print(sleep_str)
		--print(redis_times)
		if redis_times >3 then
			--start redis
			redis_cmd = "/my_tools/redis-server /etc/asterisk/redis.conf > /dev/null 2>&1 &"
			os.execute(redis_cmd)
			break
		end
		os.execute(sleep_str)
	end
end
--print("^^^^^^^^^^^^^^")
local client = get_redis_client(redis_ip_local,redis_port)
client:set("local.product.module.type",module_type_name)

endtime = starttime .. "\n"..os.date()
print("########")
print(endtime)



