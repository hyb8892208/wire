require'package_path'
require'get_redis_client'
require'get_port_status'
require'get_board_type'
local redis = require'redis'
local client = get_redis_client("127.0.0.1",6379)
local port_sum = get_board_type()
local file_path = "/etc/asterisk/gw/sms.conf"
board_sum = get_board_sum()

function get_http_conf_str()
	local f = io.open(file_path,"r")
	local tmp = f:read("*a")
	local http_conf = string.match(tmp,"%[http_to_sms%](.-)%[")
	f:close()
	return http_conf
end

local function get_value(name)
	if http_conf then
		name = string.match(http_conf,"\n%s*" .. name.. "%s*=(.-)\n")
		return name
	else
		return nil
	end
end

function mysplit(inputstr, sep)
        if sep == nil then
                sep = "%s"
        end
        if inputstr == nil then
        	return nil
        end
        local t={} ; i=1
        for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
                t[i] = str
                i = i + 1
        end
        return t
end


function get_http_conf_t()
	http_conf = get_http_conf_str()
	local t = {}
	t.port = {}
	t.port = mysplit(get_value("port"),",")
	t.report = get_value("report")
	t.timeout_total = get_value("timeout_total")
	t.debug = get_value("debug")
	t.timeout_wait = get_value("timeout_wait")
	t.timeout_gsm_send = get_value("timeout_gsm_send")
	
	return t
end



function insert_ports_redis()
	local username,secret = get_manager_username()
	local ami_manager_t = {}
	local port_list = "httpsms.ports.list"
	--client:del(port_list)
	local http_conf_t = get_http_conf_t()
	local port = {}
	port = http_conf_t.port
	if port and port[1] == "all" then
		local k = 1
		for i = 1,board_sum do
			for j = 1,port_sum do
				port[k]	= "gsm".. i .. "." .. j
				k = k+1
			end
		end
	end
	if #port > 0 then
		for i = 1, #port do
			local board,n = string.match(port[i],"gsm[%-]?(%d*)%.(%d*)")
			board = tonumber(board)
			n = tonumber(n)
			if board == 1 then
				ip = "127.0.0.1"
			else
				ip = get_slave_ip(board)
			end
			if not ami_manager_t[board] then
				if not username then
					ami_manager_t[board] = get_ami_obj(ip,ip)
				else
					ami_manager_t[board] = get_ami_obj(ip,ip,username,secret)
				end
			end
		end
	end
	local port_status_t = {}
	if #port > 0 then
		for i = 1, #port  do
			local status = get_port_status(port[i],ami_manager_t)
			if status == 1 then
				table.insert(port_status_t,port[i])
			end
		end
	end

	client:del(port_list)
	for key,value in pairs(port_status_t) do
		if value ~= nil then
			client:rpush(port_list,value)
		end
	end
	
end
insert_ports_redis()
