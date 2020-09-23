function get_time()
	local time = os.execute("cat /proc/uptime > /tmp/time.txt")
	local f = io.open("/tmp/time.txt", "r")
	local str = f:read("*all")
	local strs = string.match(str,"(%S*)%.")
	return strs
end
--local time = get_time()
--print(time)

function get_custom_from_redis(redis_client,redis_type,type_name,type_key)
	local redis_client = redis_client
	if redis_type == "string" then
		return redis_client:get(type_name)
	elseif redis_type == "hash" then
		if type_name ~= nil and type_key ~= nil then
			return redis_client:hget(type_name,type_key)
		else 
			return nil
		end
	end
end

function get_conf_info(filepath)
	local f = io.open(filepath,"r")
	local t = {}
	if f == nil then
		print("Open file failed.")
		return nil
	end
	local label_name = ""
	local info_t = {}
	print(type(f))
	for line_info in f:lines() do
		if (string.find(line_info,"%[")) then
			label_name = string.match(line_info,"%[(.-)%]")
			info_t[label_name] = {}
		else
			for key,value in string.gmatch(line_info,"(.-)=(.+)") do
				if label_name ~= nil then
					info_t[label_name][key] = value
				end
			end
		end
	end
	f:close()
	return info_t
end
