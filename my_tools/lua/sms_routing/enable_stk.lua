require'package_path'
require'get_board_type'
require'get_redis_client'
local board_type = get_board_type()

local socket = require'socket'

local redis_port = 6379
local redis_ip_local = "127.0.0.1"

local client = get_redis_client(redis_ip_local,redis_port)
local module_type = client:get("local.product.module.type")

ast_prefix = "asterisk -rx \""
is_reboot = 0

function sleep_linux(n)
	socket.select(nil, nil, n)
end

function reboot_asterisk()
	local reboot_asterisk = ast_prefix .. "core restart now" .. "\""
	print(reboot_asterisk)
	os.execute(reboot)
end

function set_stk_sim840()
	local command
	local port = 1
	for port=1,board_type do
		local command = "gsm send sync at " .. port .. " " .. "AT+STKPCIS? " .. "5000"
		local run = ast_prefix .. command .. "\""
		print(run)
		handle = io.popen(run)
		result = handle:read("*a")
		print(result)
		handle:close()
		if string.find(result, "+STKPCIS: 1") then
			print("stkpcis = 1")
			--[[
			command = "gsm send sync at " .. port .. " " .. "AT+STKTRS=10 " .. "5000"
			print(command)
			run = ast_prefix .. command .. "\""
			handle = io.popen(run)
			result = handle:read("*a")
			print(result)
			handle:close()
			--]]
		else if string.find(result, "+STKPCIS: 0") then
			--stkpcis: 0, need set stkpcis: 1
			is_reboot = 1
			local command = "gsm send sync at " .. port .. " " .. "AT+STKPCIS=1;&W " .. "5000"
			run = ast_prefix .. command .. "\""
			handle = io.popen(run)
			result = handle:read("*a")
			print(result)
			handle:close()
		end
		end
		port = port + 1
		sleep_linux(2)
	end

	if is_reboot == 1 then
		print("need reboot")
		reboot_asterisk();
	end
end

function set_stk_m35()
	local command
	local port = 1
	for port=1,board_type do
		local command = "gsm send sync at " .. port .. " " .. "AT+QSTKAUTORSP? " .. "5000"
		local run = ast_prefix .. command .. "\""
		print(run)
		handle = io.popen(run)
		result = handle:read("*a")
		print(result)
		handle:close()
		if string.find(result, "+QSTKAUTORSP: 0,1") then
			print("stkpcis = 1")
		else if string.find(result, "+QSTKAUTORSP: 0,30") then
			--stkpcis: 0, need set stkpcis: 1
			is_reboot = 1
			local command = "gsm send sync at " .. port .. " " .. "AT+QSTKAUTORSP=0,1;&W " .. "5000"
			run = ast_prefix .. command .. "\""
			handle = io.popen(run)
			result = handle:read("*a")
			print(result)
			handle:close()
		end
		end
		port = port + 1
		sleep_linux(2)
	end

	if is_reboot == 1 then
		print("need reboot")
		reboot_asterisk();
	end
end

if module_type ~=nil then
    if module_type == "sim840" then
		set_stk_sim840()
	else
		set_stk_m35()
	end
end
