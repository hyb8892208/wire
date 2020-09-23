require'package_path'
require'common_tools'
local Serial = require'periphery'.Serial
local BAUD = 460800
local emu_dev_file = "/tmp/hw_info.cfg"

-- 函数名 get_emu_dev:      获取emu的tty设备号
-- 输入参数 board:          emu管理的子板编号,一个emu管理一个子板的8个端口，从顶到底board的编号为1-4
-- 返回值：                 返回emu的tty设备号
function get_emu_dev(board)

	local cmd = "/my_tools/set_config "..emu_dev_file.." get option_value emu emu_" .. board
	local file = io.popen(cmd)
	local dev = file:read("*all")
	
	print(type(dev))
	print("emu dev is "..dev)
	return dev
end

-- 函数名get_port_info:     获取实际端口编号与物理端口标号的对应关系
-- 输入参数：               无
-- 返回值： 				一个table，内容为实际端口编号对应的物理端口编号
function get_port_info()
	local file_path = "/tmp/hw_info.cfg"
	local t_port_array = {}
	local f = io.open(file_path, "r")
	assert(f)
	local pattern = "chan_(%d+)=(/%w+/%w+/%w+)"
	for str in f:lines() do
		for port, mapport, str in string.gmatch(str, pattern) do
			if port ~= nil and mapport ~= nil then
				t_port_array[port] = port
			end
		end
	end
	return t_port_array
end


-- 函数名connect:           连接串口，获取串口操作符
-- 输入参数board:           emu管理的子板编号,一个emu管理一个子板的8个端口，从顶到底board的编号为1-4
-- 返回值： 				串口操作符以及错误信息
function connect(board)
	local dev = get_emu_dev(board)
	if dev ~= "" then
		local serial = Serial(dev, BAUD)
		if not serial then
			return nil,"connect serial "..dev.."failed"
		end
		return serial,nil
	else
		return nil,"get dev error"
	end
end

-- 函数名send_cmd_to_serial:发送指令到串口，并进行串口返回数据校验，判断返回数据信息
-- 输入参数serial:          串口操作符
--         send:            待发送的指令
--         check1：         校验的指令1
--         check2:          校验的指令2
-- 返回值： 				返回0表示发送指令以及校验指令成功；返回-1表示发送指令或者校验指令失败
function send_cmd_to_serial(serial,send,check1,check2)
	local serial = serial
	local flag = 1 
	local resend = 0
	local head_len = 4
	while flag == 1 do
		local ret = serial:write(send)
		if ret==0 then print("    send data to serial error...") end
		
		print("    send["..#send.."] = ["..bytes2HexString(send).."]")
		
		local response = serial:read(#send, 200)
		if response and #response ~= 0 then
			print("    receive["..#response.."] = ["..bytes2HexString(response).."]")
			local byte4num = tonumber(string.byte(response, 4))
			local req_len = byte4num + head_len
			local curr_len = #response
			local retry = 5
			local rest_len = req_len - curr_len
			------如果没有接收到完整的UDP包，继续接收，并截取整合为完整的包
			while rest_len > 0 do
				local response_add = serial:read(rest_len, 200)
				retry = retry - 1
				if response_add then
						print("    receive["..#response_add.."] = ["..bytes2HexString(response_add).."]")
						response = response .. response_add
						print("    receive["..#response.."] = ["..bytes2HexString(response).."]")
				end
				if retry<=0 then break end
				rest_len = rest_len - #response
			end

			if (check ~= nil and #check > 0) then
				if check2 ~= nil and #check2 > 0 then
					if string.find(response,check2) then return 0 end
				end
				if string.find(response,check) then return 0 else flag=1 end
			else
				if (#response==0) then flag=1 else return 0 end
			end
		end
		print("    check not fine, resend command again...")
		os.execute("sleep 1")
		resend = resend + 1
		if resend >= 3 then flag = 0 end
	end
	return -1
end


