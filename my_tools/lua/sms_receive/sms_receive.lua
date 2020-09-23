require'package_path'
require'get_redis_client'
require'logging.rolling_file'
require'get_redisip'
local redis_ip = get_redisip()

local logger = logging.rolling_file("/tmp/log/lua/sms_receive.log",1048576)

local port = arg[1]
local sms_src = arg[2]
local time = arg[3]
local sms_text = arg[4]
local smsc = arg[5]
sms_text = sms_text:gsub("\\","\\\\")
sms_text = sms_text:gsub("\"","\\\"")


--logger:info("\nport:%s\nsrc:%s\ntime:%s\ntext:%s\n\n",port,sms_src,time,sms_text)


function trim2(s)
        return s:match "^%s*(.-)%s*$"
end
function get_receive_sms_str()
	if smsc == nil then
		smsc = ""
	end

	local str = '{"port":' .. '"'.. port .. '",' ..
        	    '"src":' .. '"' .. sms_src .. '",' ..
		    '"time":' .. '"' .. time ..'",' ..
        	    '"text":' .. '"' .. sms_text ..'",' ..
				'"smsc":' .. '"' .. smsc .. '"' ..'}'  
	return str
end

local client = get_redis_client(redis_ip, 6379)                                                     
--[[
function connect_redis()
        local response = client:ping()
        if not response then
                client = redis.connect(redis_ip,6379)
        end
end
--]]
function main ()                                                   
        buf = get_receive_sms_str()
        print("buf is " .. buf)                                  
        
		--connect_redis()                                                          
        local llen = client:llen("app.asterisk.smsreceive.list")   
        if llen >= 0 and llen < 1000 then                          
                local ret = client:rpush("app.asterisk.smsreceive.list", buf)
                if ret > 0 then                                              
                        print("OK")                         
                else                                                         
                        print("insert wrong")                   
                end                                                          
        end                                                                  
end                                                                          
                                                                             
main()                             
