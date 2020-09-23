require'package_path'
require'get_board_type'
require'logging.rolling_file'
function command_ret(command_str)
	local f = io.popen(command_str)
	local ret = f:read("*all")
	return ret
end

function process_num(process_name)
	local command_str = "ps -ef | grep \"" .. process_name .. "$\" | grep -v grep | wc -l"
	local num = command_ret(command_str)
	num = tonumber(num)
	return num
end

local logger = logging.rolling_file("/tmp/lua_log/process.log",1024)
local get_slot_num_command = "cat /tmp/.slotnum 2> /dev/null"
local get_slot_type_command = "cat /tmp/.slot_type 2> /dev/null"
local board_type = get_board_type()
--print(board_type)
local slot_num = command_ret(get_slot_num_command)
local slot_type = command_ret(get_slot_type_command)
slot_num = tonumber(slot_num)
slot_type = tonumber(slot_type)
if slot_type == 1 then
	slot_num = 1
end

local process_name_t = {}
local http_process_t = {}
process_name_t[1] = "lua master.lua"
http_process_t[1] = "lua http_sms_out.lua"
--[[
process_name_t[2] = "lua send.lua " .. slot_num .." 1"
process_name_t[3] = "lua send.lua " .. slot_num .." 2"
process_name_t[4] = "lua send.lua " .. slot_num .." 3"
process_name_t[5] = "lua send.lua " .. slot_num .." 4"
--]]

for i = 1,board_type do
	process_name_t[i+1] = "lua send.lua " .. slot_num .." "..i
end

process_name_t[#process_name_t + 1] = "lua sms_results_to_http.lua"
process_name_t[#process_name_t + 1] = "lua event.lua"
-- process_name_t[#process_name_t + 1] = "lua listen_slave_http.lua"
process_name_t[#process_name_t + 1] = "lua sms_reports_to_http.lua"


if slot_num == 1 then
	if process_num(process_name_t[1]) == 0 then
		logger:warn("The process name %s down,it will be recovered soon",process_name_t[1])
		os.execute(process_name_t[1] .. " &") 
	end
end

if slot_num == 1 then
	if process_num(http_process_t[1]) == 0 then
		logger:warn("The process name %s down,it will be recovered soon",http_process_t[1])
		os.execute(http_process_t[1] .. " >> /tmp/sms_out_log 2>&1 &") 
	end
end

for i = 2, board_type+1 do
--	print(process_num(process_name_t[i]))
	if process_num(process_name_t[i]) == 0 then
		logger:warn("The process name %s down,it will be recovered soon",process_name_t[i])
		os.execute(process_name_t[i] .. " &")
	end
end

if slot_num == 1 then
	if process_num(process_name_t[#process_name_t-2]) == 0 then
		logger:warn("The process name %s down,it will be recovered soon",process_name_t[#process_name_t-2])
		os.execute("cd /my_tools/lua/send/ && lua sms_results_to_http.lua &")
	end
end
if process_num(process_name_t[#process_name_t-1]) == 0 then
	logger:warn("The process name %s down,it will be recovered soon",process_name_t[#process_name_t-1])
	os.execute("cd /my_tools/lua/event/ && lua event.lua &")
end
--[[
if slot_num == 1 then
	if process_num(process_name_t[#process_name_t-1]) == 0 then
		logger:warn("The process name %s down,it will be recovered soon",process_name_t[#process_name_t-1])
		os.execute("cd /my_tools/lua/send/ && lua listen_slave_http.lua &")
	end
end
]]--
if slot_num == 1 then
	if process_num(process_name_t[#process_name_t]) == 0 then
		logger:warn("The process name %s down, it will be recovered soon", process_name_t[#process_name_t])
		os.execute("cd /my_tools/lua/send/ && lua sms_reports_to_http.lua &")
	end
end
