require'package_path'
require'get_redis_client'
require'get_port_status'
local command = require'command'
require'ping'
require'get_redisip'
require'logging.rolling_file'
require'get_board_type'
local board_type = get_board_type()
local redis_ip = get_redisip()

local redis_port = 6379
local logger = logging.rolling_file("/tmp/log/lua/event.log",1048576)

local username,password = get_manager_username()
local ami_manager = get_ami_obj(_, _,username,password)
local client = get_redis_client(redis_ip,redis_port)

function sleep_linux(n)
        socket.select(nil,nil,n)
end

function get_board_num()
        local f = io.open("/tmp/.slotnum","r")
        local num = f:read("*all")
        num = tonumber(num)
        return num
end
local board_num = get_board_num()

--[[
function connect_redis()
        local response = client:ping()
        if not response then
                logger:warn("Redis down!")
                client = redis.connect(redis_ip,redis_port)
        end
end
--]]



function connect_asterisk()
        local response = ping_asterisk(ami_manager)
        if  not response  then
                logger:warn("Asterisk down!")
		local username,password = get_manager_username()
                ami_manager = get_ami_obj(_, _, username, password)
        end
end


function check_port()
	--connect_redis()
	connect_asterisk()
	for port = 1,board_type do
		local state = command.get_port_state(ami_manager,port,"127.0.0.1")
		print(state)
		local num = (board_num - 1)*board_type + port
		--local str = "app.asterisk.gsmstatus.channel" .. num
		local str = "app.asterisk.gsmstatus.channel"
		if state == "READY" then
			local old = client:hget(str, num)
			print(old)
			if old ~= "0" then
				local response = client:hset(str, num, "0")
				logger:info("checkport:change gsm%d.%d to 0",board_num,port)
				if response ~= true then
					return nil
				end
			end
		else
			local old = client:hget(str, num)
			print(old)
			if old ~= "1" then
				response = client:set(str, num, "1")
				logger:info("checkport:change gsm%d.%d to 1",board_num,port)
				if response ~= true then
					return nil
				end
			end
		end
	end

	return 1
end

while 1 do
	check_port()
	sleep_linux(2)
end

