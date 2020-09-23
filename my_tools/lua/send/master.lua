require'package_path'
require'get_http_port'
require'get_redis_client'
local cjson = require'cjson'
local redis = require'redis'

local redis_ip = "127.0.0.1"
local redis_port = 6379
local send_list = "app.asterisk.php.sendlist"
local http_list = "app.asterisk.httpsms.list"
local http_back = "app.asterisk.httpsms.back"
local sendspans_list = "app.asterisk.sms.sendspans"
local board_list_pre = "app.asterisk.php."
local port_list = "httpsms.ports.list"
local gsm_prefix = "gsm1."
local pop_t = {}

local client = get_redis_client(redis_ip,redis_port)
local uuid = require'uuid'
local curr_port = 0
local curr_port_http_sms = 0

-- 单板的通道数
local span_num = tonumber(client:get("local.product.board.span"))

-- simbank.collect.sim.info table used to send SMS of collecting the SIM information
-- simquery.collect.balance.info table used to send SMS of collecting the SIM balance
-- simquery.collect.phonenum.info table used to send SMS of collecting the SIM number
local sms_list_table = {"simbank.collect.sim.info", "simquery.collect.balance.info", "simquery.collect.phonenum.info"}

print("span num = "..span_num)

function split(s, delimiter) 
	result = {}; 
	for match in (s..delimiter):gmatch("(.-)"..delimiter) do 
		table.insert(result, match); 
	end 
	return result;
end

function sleep_linux(n)
	socket.select(nil, nil, n)
end

-- 检测是否有simbank过来要转发的短信，有就分发出去
-- rpush app.asterisk.php.gsm1.19 "{\"type\":\"sms\",\"content\":\"hello world 555\",\"value\":\"13510161713\",\"switch\":0,\"retry\":\"0\",\"exten\":\"1\"}"
function distribute_simbank_sms()
	for i = 1, span_num do
		for j = 0, #sms_list_table do 
			if sms_list_table[j] ~= "" and sms_list_table[j] ~= nil then
				local map_key = i.."-send-stat"
				local stat = client:hget(sms_list_table[j], map_key)
				if tonumber(stat) == 1 then -- 1代表有短信需要发送 
					
					-- 取内容
					map_key = i.."-send-num"
					local send_num = client:hget(sms_list_table[j], map_key)
					if send_num == nil then
						send_num = " "
					end

					map_key = i.."-send-msg"
					local send_msg = client:hget(sms_list_table[j], map_key)
					if send_msg == nil then
						send_msg = " "
					end
					
					-- push进发送队列
					map_key = "app.asterisk.php.gsm1."..i
					local str = "{\"type\":\"sms\",\"content\":\""..send_msg.."\",\"value\":\""..send_num.."\",\"switch\":0,\"retry\":\"0\",\"exten\":\"1\"}"
					local ret = client:rpush(map_key, str)
					print("rpush "..map_key..str)
					print("ret = "..ret)
					
					-- 改状态
					map_key = i.."-send-stat"
					client:hset(sms_list_table[j], map_key, "2")
				end
			end 
		end
	end
end

-- 分发一条sms到一个元素个数小于5的span list
-- pengzhongwei,2016-01-21
function distribute_sms(pop_t)
	local list_span
	local count_retry = 0
	local count_spans = client:llen(sendspans_list) - 1
	
	while true do
		for i = 0, count_spans do
			if curr_port > count_spans then
				curr_port = 0
			end
			list_span = client:lindex(sendspans_list, curr_port)
			curr_port = curr_port + 1
			local board_list = board_list_pre .. list_span
			if client:llen(board_list) < 5 then
				client:rpush(board_list,pop_t[2])
				os.execute("usleep 100000")
				return
			end
		end
		
		count_retry = count_retry + 1
		if count_retry < 10 then
			sleep_linux(3)
		else
			return -- drop the sms
		end
	end
end

-- 使用httpsms方式发送短信时，返回一个空闲的发送端口或者nil
-- pengzhongwei, 2016-03-08
function get_httpsms_port_send()
	local port_num = 0
	local board_list = ""
	local port_name = ""

	print("port list :"..port_list)
	
	-- 1. 获取个数，别的进程会根据端口状态改写redis，改写过程中有可能为空，
	-- 尝试10次，每次sleep 1秒钟，如果都为空就返回
	for i=1,10 do
		port_num = client:llen(port_list)
		if port_num ~= 0 then
			break
		else
			sleep_linux(1)
		end
	end

	if port_num == 0 then
		return nil
	end

	-- 2. 查找一个空的队列(>=10为满)
	for i=1,port_num do
		if curr_port_http_sms >= port_num then 
			curr_port_http_sms = 0
		end
		port_name = client:lindex(port_list, curr_port_http_sms)
		curr_port_http_sms = curr_port_http_sms + 1
		
		if port_name ~= nil then
			port_name = string.gsub(port_name,"%-","")
			board_list = board_list_pre .. port_name
			if client:llen(board_list) < 10 then
				return port_name
			end
		else
			return nil
		end
	end

	-- 3. 所有端口都满了，在最后一个端口等30秒，超过就返回nil
	if board_list ~= nil then
		for i=1,30 do
			if client:llen(board_list) < 10 then
				return port_name
			else
				sleep_linux(1)
			end
		end
	end
	
	return nil
end

while true do
	while true do
		-- 发送simbank传过来的sms
		distribute_simbank_sms()
		
		pop_t = client:blpop(send_list,http_list,5)
		if pop_t == nil then
			break
		end

		if pop_t[1] == send_list then
			local tmp = cjson.decode(pop_t[2])
			
			if tmp["type"] == "sms" then
				distribute_sms(pop_t)
			else
				local	board_list = "app.asterisk.php." .. tmp["port"]
				client:rpush(board_list,pop_t[2])
			end
		end
		
		if pop_t[1] == http_list then
			local tmp = cjson.decode(pop_t[2])
--			print("----------")
--			table.foreach(tmp,print)
--			print("----------")
--			fcgi将URL中的%0A %0D分别转义为0x12 0x11,对应到10进制即为18 17
--			匹配这两个字符，转换为'\n' '\r'
			tmp.message = tmp.message:gsub(string.char(18),"\n")
			tmp.message = tmp.message:gsub(string.char(17),"\r")
			
			local to_t = {}
			local from_t = {}
			to_t = split(tmp.to,",")
			
			if tmp.from ~= "(null)" then
				from_t = split(tmp.from,",")
			end
			
--			table.foreach(to_t,print)
--			print("*******")
--			table.foreach(from_t,print)
--			print("*******")
			
			tmp.type = "http_sms"
			tmp.uuid = uuid.new()
			for i = 1, #to_t do
				tmp.to = to_t[i]
				tmp.queue = i
				tmp.sum = #to_t
				if from_t[i] and string.find(from_t[i],"umts[-]?%d+%.%d+") then
					from_t[i] = string.gsub(from_t[i],"umts","gsm")
				end
				if from_t[i] ~= nil and tonumber(from_t[i]) then
					tmp.from = gsm_prefix .. from_t[i]
				else
					tmp.from = get_httpsms_port_send()
				end
				if tmp.from == nil then
					local err_msg = string.format("{\"error\":\"No free port to send sms, please try later\"}")
					--print(err_msg)
					local http_back_pid = http_back..tmp.pidnum
					client:rpush(http_back_pid,err_msg)
				else
					local tmp_str = cjson.encode(tmp)
					local board_list = board_list_pre..tmp.from
					client:rpush(board_list,tmp_str)
				end
			end -- for
		end -- if
	end -- while
end -- while
