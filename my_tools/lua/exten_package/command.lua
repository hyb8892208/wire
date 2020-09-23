local utf8 = require'utf8_simple'
--require"logging.file"
--local logger = logging.file("/tmp/command_%s.log", "%Y-%m-%d")
function get_ami_obj(host,hostname,user_name,secret,logger)
	local make_ami_manager = require "ami".make_ami_manager

	local config =
	{
	  host = host or "127.0.0.1"; 
	  hostname = hostname or "127.0.0.1";
	  user_name = user_name or "admin";
	  secret = secret or "admin";
	  channel = "SIP/702";
	  context = "sip/1001";
	  priority = 1;
	  timeout = 30000;
	  secure = true;
	  logger = logger;
	}

	local ami_manager = make_ami_manager(config)
	if not ami_manager then
		return nil
	end
	ami_manager:get_connection()
	return ami_manager
end

function ping(ami_manager,ip)
	if not ami_manager then
		return nil
	end
	local field = "Ping"
	local response,err = ami_manager:command(ip,nil,field,"Ping",1)
	return response
end

function get_port_state(ami_manager,port,ip)

	local field = "State"
	local command_str = "gsm show span " .. port
	local response,err = ami_manager:command(ip,command_str,field)
	return response
end

function send_sms_prime(ami_manager,port,to_number,msg,ip,switch,id) 
	if not msg then
		msg = ""
	end
	local field = "SPAN"
	local command_str
	if switch ~= 1 then
		command_str = "gsm send sync sms " .. port .. " " .. to_number .. " " .. "\"" ..  msg .. "\"" .. " 90" .. " 0 " .. id
	else
		command_str = "gsm send sync sms " .. port .. " " .. to_number .. " " .. "\"" ..  msg .. "\"" .. " 90 "..switch .. " " .. id 
	end
	print(command_str)
	local response,err = ami_manager:command(ip,command_str,field,_,1)
	return response
end

function send_csms(ami_manager,port,to_number,msg,ip,flag,smscount,smssequence,id)
	local field = "SPAN"
	local command_str = "gsm send sync csms " .. port .. " " .. to_number .. " \"" ..  msg .. "\" " .. flag .. " " ..  smscount .. " " .. smssequence .. " 90" .. " 0 " .. id
	print(command_str)
	local response,err = ami_manager:command(ip,command_str,field,_,1)
	return response
end

function is_china_cdma(port)
        local result = false

        local command_str="cat /tmp/gsm/" .. port .. "|grep \"Network Name\" | awk -F: '{print $2}'"

        local cmd_handle=io.popen(command_str)

        local operator=cmd_handle:read("*all")

        local china_cdma1="CHINA TELECOM"

        local china_cdma2="CHN%-CT"

        if string.find(operator, china_cdma1) or string.find(operator, china_cdma2) then
                result = true
        end

        return result
end

function split_msg(msg,t,port)
	local len = #msg
	local utf8_len = utf8.len(msg)
	
	local my_string_sub = string.sub
	local max_len = 160
	local csms_len = max_len - 28
	-- all punctuation marks commonly used
	local split_char_set = ",. ，。；‘’！“”（）()《》'\"" 
	if len ~= utf8_len or is_china_cdma(port) == true then
		--logger:info("len=%d, utf8_len=%d,is cdma mode", len,utf8_len)
		my_string_sub = utf8.sub
		max_len = 70
		csms_len = max_len - 3
	end
	
	len = utf8.len(msg)
	local add = 0
	add = select(2, string.gsub(msg, "%[","["))
	add = add + select(2, string.gsub(msg, "%]","]"))
	add = add + select(2, string.gsub(msg, "{","{"))
	add = add + select(2, string.gsub(msg, "}","}"))
	len = len + add
	-- adding choose a length of sms split
	for i=0,csms_len do  -- SMS contents contains punctuation marks 
		split_char = my_string_sub(msg, csms_len-i, csms_len-i)
		if string.find(split_char_set, split_char) ~= nil then
			local prev_char = my_string_sub(msg, csms_len-i-1, csms_len-i-1)
			if prev_char and prev_char == "\\" then
				csms_len = csms_len - i - 1
			else
				csms_len = csms_len - i
			end
			break;
		end
	end
	-- SMS contents splited does not contains any punctuation marks
	if csms_len == 0 then 
		if max_len == 160 then
			csms_len = max_len - 28 - 3
		else
			csms_len = max_len - 3
		end
	end
	-- end adding

	if len > max_len then
		table.insert(t,my_string_sub(msg,1,csms_len-add))
		split_msg(my_string_sub(msg,csms_len+1-add,-1),t, port)
	else
		if len > csms_len and #t > 0 then
			table.insert(t,my_string_sub(msg,1,csms_len-add))
			split_msg(my_string_sub(msg,csms_len+1-add,-1),t, port)
		else
			table.insert(t,msg)
		end
	end
	return 0
end

function send_sms(ami_manager,port,to_number,msg,ip,switch,retry,id)
	if not ami_manager then
		return nil
	end
	
	local t = {}
	msg = msg:gsub("\\","\\\\\\\\")
	msg = msg:gsub("\r\n","\\\\n")
	msg = msg:gsub("\n","\\\\n")
	msg = msg:gsub("\"","\\\"")
	--msg = msg:gsub(" ","\\ ")
	msg = msg:gsub("%s","\\ ")

	split_msg(msg,t,port)
	-- the retry is nil when SMS forward calling, we need to assign 0 to retry
	if retry == nil then
		retry = 0
	end
	--logger:info("the Retry number is: %s", retry)
	if #t == 1 then
		for i=0, retry do
			local response = send_sms_prime(ami_manager,port,to_number,t[1],ip,switch,id)
			--logger:info("msg: %s ====response: %s", t[1], response)
			if not response or not string.find(response,"SUCCESSFULLY") then
				return response
			else
				break
			end
		end
	end
	
	if #t > 1 then 
		smscount = #t
		local smssequence = 1
		local flag = math.random(0,255)
		local position = 1
		local success_flag = false

		for j=0, retry do
			for i=position,#t do
				-- logger:info("The %s message: %s", i, t[i])
				local response = send_csms(ami_manager,port,to_number,t[i],ip,flag,smscount,smssequence,id)
				-- logger:info("the sending long sms status: %s", response)
				if not response or not string.find(response,"SUCCESSFULLY") then
					-- record the index of SMS that fail to be sent 
					position = i
					break
				end
				smssequence = smssequence + 1
				-- all sms splited is send successfully
				if i == #t then
					success_flag = true
				end
			end
			--all sms splited is sent successfully, jump out the loop of SMS Retry
			if success_flag == true then
				break
			end
		end
	end
	return "SUCCESSFULLY"
end

function send_ussd(ami_manager,port,ip,msg,timeout)
	if not ami_manager then
		return nil
	end
	if timeout then
	    timeout = timeout/1000
	else
	    timeout = 30
	end
	
	local command_str = "gsm send ussd " .. port .. " " .. msg .. " " .. timeout
	local response,err = ami_manager:command(ip,command_str,_)
	if response then
		if response["0"] then
			--return response["0"]
			return "Fail"
		end
		if response["1"] then
			local context = ""
			for i = 1,#response["text"] do
				context = context .. response["text"][i] .. "\n"
			end
			context = response["Text"] .. "\n" .. context
			context = string.match(context,"^(.*)\n")
			return context
		end
	end
	return "Fail",err
end



function send_at(ami_manager,port,ip,msg,timeout)
	if not ami_manager then
		return nil
	end
	
	if timeout then
	    timeout = tonumber(timeout)
	else
	    timeout = 10000
	end
    if string.match(msg,"\"") then
	msg = string.gsub(msg,"\"","\\\"")
    end
	
	local command_str = "gsm send sync at " .. port .. " " .. msg .. " " .. timeout
	local response,err = ami_manager:command(ip,command_str)
	
	if not response then
		return "Failed"
	end
	
--[[	
	local status = "Failed"
	for i = 1,#response["text"] do 
	    if response["text"][i] == "OK" then
		status = "OK" 
	    end
	end
]]--
	local status = "OK"
	
	local str = {}
	local free_str = "ActionID Response end Status text Event Privilege Board Channel Ping Timestamp \
					  Peer_Count ModuleLoadStatus ReloadReason Registry_Count ChannelType \
					  ModuleSelection ModuleCount"
	for k,v in pairs(response) do
	   if not string.find(free_str,k) and k ~= nil then
		 if type(response[k]) == "table" then
		     for i = 1,#response[k] do
			 str[#str+1] = k .. ": " .. response[k][i] .. "\n"
		     end
		 else
		     str[#str+1] = k .. ": " .. v .. "\n"
		 end
	   end
	end
	 
	local message = ""
	for i = 1,#str do
		message = message .. str[i]
	end
	
	if #message ~= 0 then
		message = string.match(message,"(.+)\n")
	end
	
	for i = 1,#response["text"] do 
	    if #response["text"][i] ~= 0 and response["text"][i] ~= "OK" then
	       message = message .. response["text"][i]
	    end
	end
	
	return message,status
end

local command = {
	get_ami_obj = get_ami_obj;
	get_port_state = get_port_state;
	send_ussd = send_ussd;
	send_sms = send_sms;
	send_at = sent_at;
}

return command
