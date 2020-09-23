require'package_path'
require'get_redis_client'
local redis = require'redis'
local client = get_redis_client("127.0.0.1",6379)

local file_path = "/etc/asterisk/gw/sms.conf"

function get_http_conf_str()
        local f = io.open(file_path,"r")
        local tmp = f:read("*a")
        local http_conf = string.match(tmp,"%[http_to_sms%](.-)%[")
        f:close()
        return http_conf
end









--client:lpush(list,str)
--[[
local tmp = client:keys("*")
print(tmp)
table.foreach(tmp,print)
	--]]
