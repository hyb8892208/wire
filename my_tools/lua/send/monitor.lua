require'package_path'
local socket=require'socket'
local board_type
local slot_num

function sleep_linux(n)
        socket.select(nil,nil,n)
end

function get_file_content(path)
	local file = io.open(path, "r")
	if file then
		local content = file:read("*all")
		io.close(file)
		return content
	end
	return nil
end

function get_lua_process()
	local t= io.popen('ps -ef')
	local result = t:read("*all")
	return result
end

function check_master_lua(str)
	process_name="lua master.lua"
	if string.find(str, process_name) == nil then
		os.execute("cd /my_tools/lua/send && ".. process_name .. " &")
	end
end


function check_send_lua(str)
	for i = 1, board_type do
		process_name = "lua send.lua " .. slot_num .." "..i	
		if string.find(str, process_name) == nil then
			os.execute("cd /my_tools/lua/send &&" .. process_name .. " &")
		end
	end
end

function check_http_sms_out_lua(str)
	process_name="lua http_sms_out.lua"
	if string.find(str, process_name) == nil then
		os.execute("cd /my_tools/lua/send &&" .. process_name .. " >> /tmp/sms_out_log 2>&1 &")
	end
end

function check_sms_results_to_http_lua(str)
	process_name="lua sms_results_to_http.lua"
	if string.find(str, process_name) == nil then
		os.execute("cd /my_tools/lua/send &&" .. process_name .. " &")
	end
end

function check_event_lua(str)
	process_name="lua event.lua"
	if string.find(str, process_name) == nil then
		os.execute("cd /my_tools/lua/event/ && lua event.lua &")
	end
end

function check_sms_reports_to_http_lua(str)
	process_name="lua sms_reports_to_http.lua"
	if string.find(str, process_name) == nil then
		os.execute("cd /my_tools/lua/send/ && lua sms_reports_to_http.lua &")
	end
end

function check_insert_port_redis_lua(str)
	process_name="lua insert_port_redis.lua"
	if string.find(str, process_name) == nil then
		os.execute("cd /my_tools/lua/my_lua_tools && lua insert_port_redis.lua >/dev/null 2>&1 &")
	end
end

board_type = tonumber(get_file_content("/tmp/.boardtype"))

slot_num = tonumber(get_file_content("/tmp/.slotnum"))

while true do

	local lua_process=get_lua_process()

	check_master_lua(lua_process)

	check_send_lua(lua_process)
	
	check_http_sms_out_lua(lua_process)

	check_sms_results_to_http_lua(lua_process)

	check_event_lua(lua_process)

	check_sms_reports_to_http_lua(lua_process)
	
--	check_insert_port_redis_lua(lua_process)

	sleep_linux(5)
end
