require'package_path'
require'get_redis_client'
require'ping'
require'get_board_type'
require'get_redisip'
local board_type = get_board_type()
local command = require'command'
local redis_ip = get_redisip()

function trim2(s)
        return s:match "^%s*(.-)%s*$"
end 

function get_board_num()
	local handle = io.popen("cat /tmp/.slotnum")
        local board = handle:read("*a")
        handle:close()
        board = trim2(board)
	board = tonumber(board)
	return board
end

function get_redis_str(port)    --str:exmple:EXTRA/5-1          
        local board_num = get_board_num()                      
        num = (board_num - 1)*board_type + port
        local str = "app.asterisk.gsmstatus.channel" .. port
--      print(str)                                                    
        return str                                                    
end                                               

local ami_manager = command.get_ami_obj(_,_,event,event,logger)
local client = get_redis_client(redis_ip,"6379")

--[[
function connect_redis()                                           
        local response = client:ping()                    
        if not response then                              
                client = redis.connect(redis_ip,"6379")
        end                                                
end 
--]]                                                       
function connect_asterisk()                                
        local response = ping_asterisk(ami_manager)
        if  not response  then                             
                ami_manager = get_ami_obj(_,_,"event","event")
        end                                                 
end                                                  

function check_state()
	connect_asterisk()
	for port = 1,board_type do 
             local state = command.get_port_state(ami_manager,port,"127.0.0.1")
        --     print(state)
         	--local str = get_redis_str(port)
         	local str = "app.asterisk.gsmstatus.channel"
                if state == "READY" then
                	print("000000000000")
			local old = client:hget(str, port)
			if  old ~= "0" then 
        	                local response = client:hset(str, port, "0")                      
	                        if response ~= true then                                  
                  			return nil                                        
				end                                                       
			end
                else if state ~= nil then 
                	print("1111111111111")
			local old = client:hget(str, port)
			if  old ~= "1" then 
        	                local response = client:hset(str, port, "0")                      
	                        if response ~= true then                                  
                  			return nil                                        
				end                                                       
			end
		end
                end                                                               
        end                                  
end

while true do
	os.execute("sleep 5")
	check_state()
end
