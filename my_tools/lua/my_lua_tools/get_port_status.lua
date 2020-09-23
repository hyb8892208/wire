require'package_path'
local command = require'command'
local board_sum

function lua_string_split(str, split_char)
	local sub_str_tab = {}
	while (true) do
		local pos = string.find(str, split_char)
		if (not pos) then
			sub_str_tab[#sub_str_tab + 1] = str
			break
		end
		local sub_str = string.sub(str, 1, pos - 1)
		sub_str_tab[#sub_str_tab + 1] = sub_str
		str = string.sub(str, pos + 1, #str)
	end
	return sub_str_tab
end

function get_board_sum()
	local f = io.open("/etc/asterisk/gw/cluster.conf","r")
	local cluster_str = f:read("*a")
	local board_sum = select(2,string.gsub(cluster_str,"Board%-%d_ip","Board%-%d_ip")) + 1
	return board_sum
end

function get_port_name(portname)
	local board,port = string.match(portname,"%-(%d+)%.(%d+)")
	board = tonumber(board)
	port = tonumber(port)
	local f = io.open("/tmp/.module_type","r")
	local f_str = f:read("*a")
	local module_table = lua_string_split(f_str,"\n")
	local module_str = lua_string_split(module_table[board],',')
	local module_type = module_str[port]
	
	if module_type == 'sim840' then
		module_head = 'gsm-'
	elseif module_type == 'uc15e' or module_type == 'uc15a' or module_type == 'uc15t' then
		module_head = 'umts-'
	else
		module_head = 'gsm-'
	end
	
	portname = module_head .. board .. '.' .. port
	return portname 
	
	
end

function get_slave_ip(board)
	if not board then
		return nil
	end
	
	local f = io.open("/etc/asterisk/gw/cluster.conf","r")
	local cluster_str = f:read("*a")
	
	pattern = "%s*Board%-" .. board .. "_ip%s*=%s*(%S*)"
	local ip = string.match(cluster_str,pattern)	
	
	
	return ip
end
function conf_split(s, delimiter) 
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

function get_manager_username()
	local f_path = "/etc/asterisk/manager.conf"
	local f = io.open(f_path,"r")
	local f_str = f:read("*a")
	f:close()
	f_str = string.match(f_str,"^%s*(.-)%s*$")
	f_str = string.gsub(f_str,"\n\n","\n")
	t = conf_split(f_str,"[")
	for i = 1,#t do
		tmp_t = conf_split(t[i],"\n")
		tmp_t[1] = string.match(tmp_t[1],"(.*)%]")
		if tmp_t[1] ~= "general" and tmp_t[1] ~= "send" and tmp_t[1] ~= "event" then
			local manager_username =  tmp_t[1]
			for j = 2,#tmp_t do
				local key,value = string.match(tmp_t[j],"(.*)=(.*)")
				if key == "secret" then
					local manager_secret = value
					return manager_username,manager_secret
				end
			end
		end
	end
	
	return nil,nil
end

------get_port_status ------------------------------------------
	--port like gsm1.1, gsmx.x
	--ami_manager : include all board (master & slaves) ami manager
function get_port_status(port,ami_manager)
	if not port then
		return nil
	end
	local board,n = string.match(port,"gsm[%-]?(%d*)%.(%d*)")
	board = tonumber(board)
	n = tonumber(n)
	if board == 1 then
		ip = "127.0.0.1"
	else
		ip = get_slave_ip(board)
	end
	print(ip)
	
	if not ami_manager then
--	local ami_manager = get_ami_obj(ip,ip,_,_,print)
		local username,secret = get_manager_username()
		if not username then
			ami_manager = get_ami_obj(ip,ip)
		else
			ami_manager = get_ami_obj(ip,ip,username,secret)
		end
	end
	
	--if type(ami_manager) == "table" then
		ami_manager = ami_manager[board]
	--end
	
	local status_t = {"CALL","RING","HANGUP","SEND","SENT","RECIEVE","ANSWER"}
	
	response,err = command.get_port_state(ami_manager,n,ip)
	if not response then
		return nil
	end

	local status = 0
	if response == "READY" then
		status = 1
	else
		for i = 1, #status_t do
			if response ~= nil then
				if string.find(response,status_t[i]) then
					status = 1
				end
			end
		end
	end

	return status
end
	
--local m = get_port_status("gsm2.1")
--print(m)
