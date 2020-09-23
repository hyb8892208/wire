require'package_path'
require'get_redis_client'
require'logging.rolling_file'
local command = require'command'
local socket = require'socket'
require'ping'
require'get_redisip'
require'common_tools'
require'get_board_type'
require"logging.file"
local logger = logging.file("/tmp/send_%s.log", "%Y-%m-%d")

local redis_ip = get_redisip()
local redis_port = 6379

local cjson = require'cjson'
local send_list = "app.asterisk.php.gsm" .. arg[1] .. "." .. arg[2]
local curr_span = "gsm" .. arg[1] .. "." .. arg[2]
local send_port = arg[2]
local back_list
local php_back_list = "app.asterisk.php.feedbacklist"
local ussd_back_list = "app.asterisk.ussd.feedbacklist"
local at_back_list = "app.asterisk.at.feedbacklist"
local http_back_list = "app.asterisk.httpsms.back.in"
local sms_out_list = "app.asterisk.smssend.list"
local time
local list_t = {}
local response
--local ami_manager = get_ami_obj(_,_,"send","send",print)
local ami_manager = get_ami_obj(_,_,"send","send")
local client = get_redis_client(redis_ip,redis_port)

local logger = logging.rolling_file("/tmp/log/lua/send.log",1048)

function sleep_linux(n)
	socket.select(nil,nil,n)
end

function connect_asterisk()
	local response = ping_asterisk(ami_manager)
	if  not response  then
		logger:warn("Asterisk down!")
		ami_manager = get_ami_obj(_,_,"send","send")
	end
end
function get_sms_response(tmp_t)
	local send_content =  tmp_t.content
	local to_number = tmp_t.value
	local retry = tonumber(tmp_t.retry)
	local sms_response
	local cmd_outbox
	local send_status
	
	sms_response = send_sms(ami_manager,send_port,to_number,send_content,"127.0.0.1",tmp_t.switch,retry," ")
	if sms_response then
		if string.find(sms_response,"SUCCESSFULLY") then
			sms_response = "succeed"
		end
	end
	if sms_response ~= "succeed" then
		sms_response = "fail"
		send_status = "0";
	else
		send_status = "1";
	end
	if send_port and to_number and send_content then
		local sms_out_one = {}
	--	sms_out_one.port = get_board_type() * (arg[1] - 1) + send_port
		sms_out_one.port = send_port
		sms_out_one.number = to_number
		sms_out_one.date = os.date("%Y-%m-%d %H:%M:%S")
		sms_out_one.content = send_content
		sms_out_one.status = send_status
		client:rpush(sms_out_list, cjson.encode(sms_out_one))
	end
	return sms_response
end

while true do
	while true do
		send_list_t = client:lpop(send_list)
		if not send_list_t then
			sleep_linux(5)
			break
		end
		connect_asterisk()
		local tmp_t = cjson.decode(send_list_t)
		if not tmp_t then
			break
		end

		local send_content = tmp_t.content
		if tmp_t.type == "atcommand" then
			response,status = send_at(ami_manager,send_port,"127.0.0.1",send_content)
			if not response or #response == 0 then
				response = status
			else
				response = response .. "\n" .. status
			end
		elseif tmp_t.type == "sms" then
			local send_num = tonumber(tmp_t.exten)
			tmp_t.port = curr_span
			for i = 1,send_num do
				response = get_sms_response(tmp_t)
			end
			if response == "succeed" then
				sleep_linux(3)  -- sleep 3 秒， 防止gsm模块或者基站busy
			else
				response = "fail"
				sleep_linux(30) -- fail 就sleep，令队列拥挤，把分发权重分给其它队列
			end
		elseif tmp_t.type == "ussd" then
			local timeout = string.match(send_content,"@(%d+)")
			if timeout then
				send_content = string.match(send_content,"(%S+)@%d+")
			end
			response = command.send_ussd(ami_manager,send_port,"127.0.0.1",send_content)
		elseif tmp_t.type == "http_sms" then
			local send_status
		 	sms_response = send_sms(ami_manager,send_port,tmp_t.to,tmp_t.message,"127.0.0.1",0,0,tmp_t.id)			
		 	if sms_response and string.find(sms_response,"SUCCESS") then
		 		response = "success"
				send_status = "1"
		 	else
		 		response = "fail"
				send_status = "0"
		 	end
		
			if send_port and tmp_t.to and tmp_t.message then
				local sms_out_one = {}
			--	sms_out_one.port = get_board_type() * (arg[1] - 1) + send_port
				sms_out_one.port = send_port
				sms_out_one.number = tmp_t.to
				sms_out_one.date = os.date("%Y-%m-%d %H:%M:%S")
				sms_out_one.content = tmp_t.message
				sms_out_one.status = send_status
				client:rpush(sms_out_list, cjson.encode(sms_out_one))
			end
		end
		
		if not response then
			response = "fail(Connect to asterisk fail)"
		end
		
		tmp_t.result = response
		tmp_t.time = os.date("%Y-%m-%d %H:%M:%S")
		if tmp_t.type == "http_sms" then
		--	tmp_t.from = string.gsub(tmp_t.from,"gsm","gsm%-")
			tmp_t.from = string.sub(tmp_t.from, 6, -1)
			back_list = http_back_list
		elseif tmp_t.type == "ussd" then
			back_list = ussd_back_list
		elseif tmp_t.type == "atcommand" then
			back_list = at_back_list
		else
			back_list = php_back_list	
		end
		back_str = cjson.encode(tmp_t)
		client:rpush(back_list,back_str)
	end
end
