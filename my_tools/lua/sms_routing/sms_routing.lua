require'package_path'
require'get_redis_client'
require'get_port_status'
require'common_tools'
require'get_board_type'
local command = require'command'
require'ping'
local cjson = require'cjson'


client = get_redis_client('127.0.0.1', 6379)
local roundrobin_list = "sms.routing.roundrobin.port"
socket = require'socket'

--make_ami_manager = require "ami".make_ami_manager

function sleep_linux(n)
	socket.select(nil, nil, n)
end

function get_slave_info(slave_conf)
	local rfile=io.open(slave_conf, "r")
	assert(rfile)
	local t_slave = {}
	for str in rfile:lines() do
		if string.find(str, "Board[-]%d[_]ip=") then
			local ret = string.gsub(str, "Board[-]%d[_]ip=", "")
			table.insert(t_slave, ret)
		end
	end
	rfile:close()
	return t_slave
end

function get_ip_info()
   	local t = get_slave_info("/etc/asterisk/gw/cluster.conf")
	table.insert(t,1,"127.0.0.1")
	return t
end

local username,password = get_manager_username()
local ip_t = get_ip_info()
local ami_manager = {}
for i = 1, #ip_t do
	ami_manager[i] = get_ami_obj(tostring(ip_t[i]),tostring(ip_t[i]),username,password)
end

function connect_asterisk(board_n)
	local response = ping_asterisk(ami_manager[board_n])
	while not response do
		ami_manager[board_n] = get_ami_obj(ip_t[board_n], ip_t[board_n], username, password)
		response = ping_asterisk(ami_manager[board_n])
		sleep_linux(2)	
	end
end


function msg_decode(msg)
	local pos = string.find(msg, "::")
	return string.sub(msg, 1, pos-1)
end

function gsm2port(gsm)
	--gsm is gsm2.1,gsm10.4,etc
	--port is 1-4
	local t_gsm2port = {}
	for w in string.gmatch(gsm, "%d+") do
		table.insert(t_gsm2port, w)
	end
	return t_gsm2port
end

function read_sms_routing_conf()
	local rfile = io.open("/etc/asterisk/sms_routing.conf", "r")
	assert(rfile)
	
	local t_sms_routing = {}
	for str in rfile:lines() do
		table.insert(t_sms_routing, str)
	end
	
	rfile:close()
	
	return t_sms_routing	
end

function sms_group_policy(to_gsms, policy,from_gsm)
	local tmp	
	if policy == "ascending" then
		for index, tmp in pairs(to_gsms) do
			t = gsm2port(tmp)
			t[1] = tonumber(t[1])
			t[2] = tonumber(t[2])
			connect_asterisk(t[1])
			local str = "gsm" .. t[1] .. "." .. t[2]
			local response, err = get_port_status(str,ami_manager)
			if response == 1 then
				return tmp
			end
		end	
	elseif policy == "roundrobin" then
		local to_len = #to_gsms
		local org_pos = 0
		local org_to_gsm = client:hget(roundrobin_list,from_gsm)
		if org_to_gsm == nil then
			org_pos = 0
		else
			for index,tmp in pairs(to_gsms) do
				if tmp == org_to_gsm then
					org_pos = index
				end
			end
		end
		
		for index=org_pos+1,to_len do
			tmp = to_gsms[index]
			t = gsm2port(tmp)
			t[1] = tonumber(t[1])
			t[2] = tonumber(t[2])
			connect_asterisk(t[1])
			local str = "gsm" .. t[1] .. "." .. t[2]
			local response, err = get_port_status(str,ami_manager)
			if response == 1 then
				client:hset(roundrobin_list,from_gsm,tmp)
				break
			else
				tmp = nil
			end
		end
		
		if not tmp then
			for index = 1, org_pos do
				tmp = to_gsms[index]
				t = gsm2port(tmp)
				t[1] = tonumber(t[1])
				t[2] = tonumber(t[2])
				connect_asterisk(t[1])
				local str = "gsm" .. t[1] .. "." .. t[2]
				local response, err = get_port_status(str,ami_manager)
				if response == 1 then
					client:hset(roundrobin_list,from_gsm,tmp)
					break
				else
					tmp = nil
				end
			end
		end
		return tmp
	else
		return nil
	end
end

function get_to_and_number(sms_group_conf, port)
	local i = 1
	local value
	local tmp
	local pos_to_number
	local to_number
	local policy
	
	for _, value in pairs(sms_group_conf) do

		if string.find(value, "from_member") then
			if string.find(value, port) then
				j = i + 1 --goto to_membr_line
				local pos = string.find(sms_group_conf[j], "=")
				local to_members = string.sub(sms_group_conf[j], pos + 1)
				local members = {}
				for tmp in string.gmatch(to_members, "gsm[%d+].%d+") do
					table.insert(members, tmp)
				end
				j = j + 1 --goto to_number line
				pos_to_number = string.find(sms_group_conf[j], "=")
				to_number = string.sub(sms_group_conf[j], pos_to_number + 1)
				is_find = true
				j = i - 1  --goto policy line
				pos_policy = string.find(sms_group_conf[j],"=")
				policy = string.sub(sms_group_conf[j],pos_policy + 1)
				return members, to_number, policy
			end
		end
		i = i + 1
	end
	return nil, nil, nil
end

function rm_trigger_conf(sms_group_conf,port)
	local wfile = io.open("/etc/asterisk/sms_routing.conf","w")
	assert(wfile)
	local write_str = ""
	local i = 1
	local t_rm_conf = {}
	for key,value in pairs(sms_group_conf) do
		if string.find(value,"from_member") then
			if string.find(value,port) then
				j = i + 3
				local pos = string.find(sms_group_conf[j],"trigger_clear=")
				if pos then
					--sms_group_conf[j] = "null"
					j = j + 1
				end
				pos = string.find(sms_group_conf[j],"--trigger_start--")
				if pos then
					sms_group_conf[j] = "null"
					while 1 do
						local trigger_line = j + 1
						if string.find(sms_group_conf[trigger_line],"--trigger_end--") then
							sms_group_conf[trigger_line] = "null"
							break
						else
							sms_group_conf[trigger_line] = "null"
							j = j + 1
						end
					end
				end	
			end
		end
		
		if sms_group_conf[key] ~= "null"  then
			t_rm_conf[key] = sms_group_conf[key]
		end
		
		i = i + 1
	end
	for _,value in pairs(t_rm_conf) do
		write_str = write_str .. value .. "\n"
	end
	wfile:write(write_str)
	wfile:close()
end

function get_trigger_msg(sms_group_conf, port)
	local i = 1
	local value
	local trigger_msg_t = {}
	
	for _, value in pairs(sms_group_conf) do
		if string.find(value, "from_member") then
			if string.find(value, port) then
				j = i + 3 --goto "trigger start "line or "trigger_clear" line
				if not sms_group_conf[j] then
					return trigger_msg_t
				end
				local pos = string.find(sms_group_conf[j],"trigger_clear=")
				if pos then
					local pos_clear = string.find(sms_group_conf[j],"=")
					local trigger_clear_str = string.sub(sms_group_conf[j],pos_clear + 1)
					trigger_msg_t['trigger_clear'] = trigger_clear_str
					j = j + 1
				end
				local pos = string.find(sms_group_conf[j], "--trigger_start--")
				if pos then
					while 1 do
						local trigger_line = j + 1
						if string.find(sms_group_conf[trigger_line],"--trigger_end--") then
							break
						else
							local pos_trigger = string.find(sms_group_conf[trigger_line],"=")
							local trigger_str = string.sub(sms_group_conf[trigger_line],10,pos_trigger - 1 )
							local msg_str = string.sub(sms_group_conf[trigger_line],pos_trigger + 1 )
							
							trigger_msg_t[trigger_str] = msg_str
							
							j = j + 1
						end
					end
				else 
					-- trigger_msg_t is null table
					return trigger_msg_t
				end
				
			end
		end
		i = i + 1
	end
	return trigger_msg_t
end

function match_trigger_context(tbl_smsgroup_conf,trigger_msg_t,context,port)
	local buf = context
	local i,j = string.find(context, "\239\187\191")   --查找是否头是否是0xEF BB BF
	if i ~= nil then
		buf = string.sub(context, j + 1, #context)  -- 匹配成功,去掉前三个字符
	end

	if buf == trigger_msg_t['trigger_clear'] then
		rm_trigger_conf(tbl_smsgroup_conf,port)
		return nil
	end
	for trigger,msg in pairs(trigger_msg_t) do
		if buf == trigger then
			return msg
		end
	end
	return context
end

function insert_redis(msg)
	client:rpush("app.asterisk.smsrouting.list", msg)
	sleep_linux(1)
end

function send_sms()
	client:del("app.asterisk.smsrouting.list")
	client:del("sms.routing.roundrobin.port")
	tbl_smsgroup_conf = read_sms_routing_conf()
	if #tbl_smsgroup_conf == 0 then
		return 0
	end
	local count = 0
	
	while 1 do
		msg_t = client:blpop("app.asterisk.smsrouting.list",0)
		msg = msg_t[2]
		from_gsm = msg_decode(msg)
			
		local to_gsms, to_number, policy = get_to_and_number(tbl_smsgroup_conf, from_gsm)
		if to_gsms and to_number and policy then 
			print(msg)
			context = string.sub(msg, string.len(from_gsm)+3) --+3 for del "::"

			--add custom feature of "sms forward trigger"
			local company_hash_name = "local.product.oem.ver.ctl"
			local company_hash_key = "sms_forward_trigger"
			local trigger_switch = get_custom_from_redis(client,"hash",company_hash_name,company_hash_key)
			if trigger_switch ~= nil and trigger_switch == "1" then
				local trigger_msg_t = get_trigger_msg(tbl_smsgroup_conf, from_gsm)
				context = match_trigger_context(tbl_smsgroup_conf,trigger_msg_t,context,from_gsm)
				print(context)
			end

			--select a gsm port
			to_gsm = sms_group_policy(to_gsms,policy,from_gsm)
			if to_gsm then
				print(to_gsm)
				local m,n = string.match(to_gsm,"(%d*)%.(%d*)")
				m = tonumber(m)
				n = tonumber(n)
				to_gsm = "Board-" .. m .. "-gsm-" .. n                     
			
				connect_asterisk(m)
				if context ~= nil then
					-- adding para retry to send_sms()
					local response = command.send_sms(ami_manager[m],n, to_number, context,ip,0,0,0)
					if response and string.find(response, "SUCCESS") then
						--将转发内容存入发件箱
						local sms_out_one = {}
						sms_out_one.port = get_board_type() * (m - 1) + n
						sms_out_one.number = to_number
						sms_out_one.date = os.date("%Y-%m-%d %H:%M:%S")
						sms_out_one.content = context
						sms_out_one.status = "1"
						client:rpush("app.asterisk.smssend.list", cjson.encode(sms_out_one))

						-- 将转发的短信内容中的引号转义,解决收件箱中转发的短信显示错误
						context = string.gsub(context,"\\","\\\\")
						if string.find(context,"\"") then
							context = string.gsub(context,"\"","\\\"")
						end
						local time = os.date("%Y/%m/%d %X")
						local str = '{"port":"'.. to_gsm .. '","src":"'.. '@' .. to_number .. '","time":"' .. time .. '","text":"' .. context .. '"}'
						client:rpush("app.asterisk.smsreceive.list",str)
					else 
						insert_redis(msg)
					end
				end
			else 
				insert_redis(msg)
			end		
		end
	end
end

send_sms()
