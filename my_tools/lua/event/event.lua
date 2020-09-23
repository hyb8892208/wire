require'package_path'
require'get_redis_client'
require'logging.rolling_file'
require'get_board_type'
require'get_port_status'
local socket = require'socket'                                                                 
local ltn12 = require'ltn12'
local str_gsmstatus = "app.asterisk.gsmstatus.channel"
local str_simstatus = "app.asterisk.simstatus.channel"  --for emu
local str_calldial = "app.asterisk.extradial.channel"
local str_callconnect = "app.asterisk.extraconnect.channel"
local str_callhangup = "app.asterisk.extrahangup.channel"
local smsreports_list = "app.sms.reports.list"
local smsreportstohttp_list = "app.sms.reportstohttp.list"
local smsresulttohttp_list = "app.sms.resulttohttp.list"
local board_type = get_board_type()

local logger = logging.rolling_file("/tmp/log/event.log",1048576)

local parse = function(line)
	local k,v = line:match("^(.-):%s*(.+)$")
	if not k then
		return nil, "parse error, malformed line"
	end
	return k,v
end

function sleep_linux(n)
        socket.select(nil,nil,n)
end

function connect()
	local sock,err = socket.tcp()
	if not sock then                                                                               
		return nil,err
	end                                                                                            
	sock:settimeout(30)                                                                         
	local result,err = sock:connect("127.0.0.1",5038)
	if not result then
		return nil,err
	end

	local login = "Action:Login\nUsername:event\nSecret:event\n\n"

	ltn12.pump.all(
	ltn12.source.string(login),
	socket.sink("keep-open",sock)
	)
	return sock
end

local sock = connect()

local flag = {}
function connect_asterisk()
	--logger:error("Asterisk down!")
	flag = {}
	sock = connect()
end

require'get_redisip'
local redis_ip = get_redisip()
local client = get_redis_client(redis_ip,"6379")

function get_board_num()
        local f = io.open("/tmp/.slotnum","r")
        local slot_num = f:read("*all")
		f:close()
        slot_num = tonumber(slot_num)

        local fs = io.open("/tmp/.slot_type","r")
        local slot_type = fs:read("*all")
		fs:close()
        slot_type = tonumber(slot_type)

		if slot_type == 1 then
			slot_num = 1
		end
		
        return slot_num
end

function get_redis_str(str)    --str:exmple:EXTRA/5-1
	local num = string.match(str,"(%d+)%-")
	local board_num = get_board_num()
	num = (tonumber(num) + 1) / 2
	local gsm_str = "gsm" .. board_num .. "." .. num
	num = (board_num - 1)*board_type + num
	--local str = "app.asterisk.gsmstatus.channel" .. num
	return num,gsm_str
end

function init_redis()
--	local logger = print
	local board_num = get_board_num()
	local command = require'command'
	local username,password = get_manager_username()
	sleep_linux(2)
	local ami_manager = command.get_ami_obj(_,_,username,password)
	
	for port = 1,board_type do
		local state = command.get_port_state(ami_manager,port,"127.0.0.1")
		local num = (board_num - 1)*board_type + port
		--local str = "app.asterisk.gsmstatus.channel" .. num
		if state == "READY" then
			client:hset(str_simstatus,num,"0")
			local response = client:hset(str_gsmstatus, num, "0")
			logger:info("Init gsm%d.%d to 0",board_num,port)
			if response ~= true then
				return nil
			end
		else
			client:hset(str_simstatus,num,"1") 
			response = client:hset(str_gsmstatus, num, "1")
			logger:info("Init gsm%d.%d to 1",board_num,port)
			if response ~= true then
				return nil
			end
		end
		
		response = client:hset(str_calldial, num, "0")
		logger:info("Init %s%d to 0",str_calldial,num)
		if response ~= true then
			return nil
		end
		
		response = client:hset(str_callconnect, num, "0")
		logger:info("Init %s%d to 0",str_callconnect,num)
		if response ~= true then
			return nil
		end
		
		response = client:hset(str_callhangup, num, "0")
		logger:info("Init %s%d to 0",str_callhangup,num)
		if response ~= true then
			return nil
		end
	end

	return 1
end


local init_result = init_redis()
if not init_result then
	logger:error("Initialize event redis failed")
end



function  set_status(t)
	assert(type(t) == "table")
		if t.Event == "ExtraDown" then
			local board_num = get_board_num()
			local num = (board_num - 1)*board_type + t.Channel
			--local str = "app.asterisk.gsmstatus.channel" .. num
			client:hset(str_gsmstatus, num, "1")
			client:hset(str_simstatus, num, "1")	
			-- logger:warn("The channel gsm%d.%d down set the value to 1",board_num,t.Channel)
			flag[t.Channel] = 1
			return 0
		end
	
		if t.Event == "ExtraLock" then
			local board_num = get_board_num()
			local num = (board_num - 1)*board_type + t.Channel
			client:hset(str_gsmstatus, num, "1")
			flag[t.Channel] = 1
			return 0
		end

		if t.Event == "ExtraUnlock" then
			local board_num = get_board_num()
			local num = (board_num - 1)*board_type + t.Channel
			client:hset(str_gsmstatus, num, "0")
			flag[t.Channel] = 0
			return 0
		end

		-- if t.Event == "ExtraUp" then
		if t.Event == "ExtraReady" then
			--if flag[t.Channel] == 1 or not flag[t.Channel] then
				local board_num = get_board_num()
				local num = (board_num - 1)*board_type + t.Channel
				--local str = "app.asterisk.gsmstatus.channel" .. num
				client:hset(str_gsmstatus, num, "0")
				client:hset(str_simstatus, num, "0")
			--	logger:info("The channel gsm%d.%d recover change the value to 0",board_num,t.Channel)
				flag[t.Channel] = 0
				return 0
			--end
		end
	
		if t.Event == "Dial" and t.Channel and t.Destination then
			if string.find(t.Destination,"EXTRA") then
				str_ext = t.Destination
			elseif string.find(t.Channel,"EXTRA") then
				str_ext = t.Channel
			else
				return 0
			end
			local str,gsm_str = get_redis_str(str_ext)
			client:hset(str_gsmstatus, str, "1")
			logger:info("The channel %s is dialing ,change the value to 1",gsm_str)
			return 0
		end

		if t.Event == "SMSSendStatus" then
			print('status' .. t.Status)
			if t.Status == "SUCCESS" then
				status = "Sent"
			else
				status = "Fail"
			end
			time = string.gsub(os.date("%Y-%m-%d %H:%M:%S"), " ", "%%20")
			sms_resTohttp = '{"port":' .. '"' .. t.Channel .. '",' ..
				'"phonenumber":' .. '"' .. t.Destination .. '",' ..
				'"message":' .. '"",' ..
				'"id":' .. '"' .. t.ID .. '",' ..
				'"imsi":' .. '"' .. t.IMSI .. '",' ..
				'"time":' .. '"' .. time .. '",' ..
				'"status":' .. '"' .. status .. '",' ..
				'"type":' .. '"SMS-Status-Result"}'
			client:rpush(smsresulttohttp_list, sms_resTohttp)
			-- logger:info(" %s ",sms_resTohttp)
		end

		if t.Event == "SMSStatusReport" then
            if t.Status == "SUCCESS" then
                    message = "Message%20received%20on%20handset"
            else
                    message = "Message%20not%20received%20on%20handset"
            end
            time = string.gsub(t.Time_scts, " ", "%%20")
            sms_reports = '{"port":' .. '"' .. t.Channel .. '",' ..
	                        '"board":' .. '"' .. t.Board .. '",' ..
	                        '"phonenumber":' .. '"' .. t.Sender .. '",' ..
	                        '"time":' .. '"' .. t.Time_scts .. '",' ..
	                        '"message":' .. '"",' ..
	                        '"status":' .. '"' .. t.Status .. '",' ..
                                '"cause":' .. '"' .. t.Cause .. '"}'
	        smsreports_tohttp = '{"port":' .. '"' .. t.Channel .. '",' ..
	                        '"board":' .. '"' .. t.Board .. '",' ..
	                        '"phonenumber":' .. '"' .. t.Sender .. '",' ..
	                        '"imsi":' .. '"' .. t.IMSI .. '",' ..
	                        '"time":' .. '"' .. time .. '",' ..
	                        '"message":' .. '"",' ..
	                        '"status":' .. '"' .. message .. '",' ..
							'"cause":' .. '"' .. t.Cause .. '",' ..
	                        '"type":' .. '"SMS-Status-Report"}'
            client:rpush(smsreports_list, sms_reports)
            client:rpush(smsreportstohttp_list, smsreports_tohttp)
            return 0
        end
		
		if t.Event == "Hangup" and t.Channel and string.find(t.Channel,"EXTRA") then
			local str,gsm_str = get_redis_str(t.Channel)
			client:hset(str_gsmstatus, str,"0")
			logger:info("The channel %s has hangup ,change the value to 0",gsm_str)
			return 0
		end
		
		if t.Event == "Newchannel" and t.ChannelState and t.Channel and string.find(t.Channel,"EXTRA") then
			local str,gsm_str = get_redis_str(t.Channel)
			if tonumber(t.ChannelState) == 1 then
				client:hset(str_gsmstatus, str,"0")
				client:hset(str_simstatus, num, "0")
				logger:info("The channel %s receive the Newchannel event and the ChannelState is 1,change the value to 0",gsm_str)
				return 0
			else
				client:hset(str_gsmstatus, str,"1")
				logger:info("The channel %s receive the Newchannel event and the ChannelState is %d,change the value to 1",gsm_str,t.ChannelState)
				return 0
			end
		end
		
		if t.Event == "Bridge" and t.Channel1 and t.Channel2 then
			if string.find(t.Channel1,"EXTRA") then
				str_ext = t.Channel1
			elseif string.find(t.Channel2,"EXTRA") then
				str_ext = t.Channel2
			else
				return 0
			end
			local str,gsm_str = get_redis_str(str_ext)
			client:hset(str_gsmstatus, str,"1")
			logger:info("The channel %s is dialing ,change the value to 1",gsm_str)
			return 0
		end

		if t.Event == "ExtraDial" or t.Event == "ExtraOutDial" then
			client:hset(str_calldial,t.Channel,"1")
			logger:info("hset %s %d to 1(ExtraDial)",str_calldial,t.Channel)
			return 0
		end

		if t.Event == "ExtraConnect" then
			client:hset(str_callconnect, t.Channel,"1")
			logger:info("hset %s to 1(ExtraConnect)",str_callconnect,t.Channel)
			return 0
		end

		if t.Event == "ExtraHangup" then
			client:hset(str_callhangup, t.Channel,"1")
			logger:info("hset %s %d to 1(ExtraHangup)",str_callhangup,t.Channel)
			return 0
		end
		
	return nil
end


local parse_line = function(t,line)
	local k,v = parse(line)
	if k then
		if t[k] then
			if type(t[k]) ~= "table" then
				t[k] = {t[k]}
			end
			t[k][#t[k] + 1] = v
		else
			t[k] = v
		end
	else
		local err = v
		return nil , err
	end
end


while true do
	local t = {}
	while true do
		if sock then
			local recvt,sentdt,status = socket.select({sock},nil,1)
			if #recvt > 0 then
				local receive,receive_status = sock:receive()
				if receive_status ~= "closed" then
					if receive then
						if #receive == 0 or string.find(receive,"END COMMAND") then
								break
						else
							parse_line(t,receive)
						end
					else
						break
					end
				else
					sock = nil
					break
				end
			end
		else
			connect_asterisk()
			sleep_linux(2)
			if sock then
				init_redis()
			end
			break
		end
	end
	set_status(t)
end
