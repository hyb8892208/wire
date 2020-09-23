require'package_path'
require'get_redis_client'
local redis = require'redis'

local redis_ip = "127.0.0.1"
local redis_port = 6379
local client = get_redis_client(redis_ip,redis_port)

function split(s, delimiter) 
	result = {}
	s = s .. delimiter
	if string.find(delimiter,"%[") then
		delimiter = string.gsub(delimiter,"%[","%%%[")
	end
	for match in s:gmatch("(.-)"..delimiter) do
		if #match > 0 then
			table.insert(result, match)
		end
	end 
	return result
end

function my_trim(s)
	s = string.match(s,"^%s*(.-)%s*$")
	s = string.gsub(s,"\n\n","\n")
	return s
end

function conf_redis(filename)
	local f = io.open(filename,"r")
	local str = f:read("*a")

	str = my_trim(str)

	local t = {}
	local tmp_t = {}
	t = split(str,"[")

	for i = 1,#t do
		tmp_t = split(t[i],"\n")
		tmp_t[1] = string.match(tmp_t[1],"(.*)%]")
		for j = 2,#tmp_t do
--			print(tmp_t[1].."." ..string.match(tmp_t[j],"(.*)=.*"))
--			print(string.match(tmp_t[j],".*=(.*)"))
			client:hset(filename,tmp_t[1].."." ..string.match(tmp_t[j],"(.*)=.*"),string.match(tmp_t[j],".*=(.*)"))
		end
	end
end

function init()
	conf_redis("/etc/asterisk/gw/sms.conf")
end

if arg[1] == "init" then
	init()
else 
	conf_redis(arg[1])
end

--[[
local file = "/etc/asterisk/gw/sms.conf"
conf_redis(file)
local m = client:hget(file,"http_to_sms.report")
print(m)
--]]
