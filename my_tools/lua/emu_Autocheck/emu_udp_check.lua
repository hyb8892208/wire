require'package_path'
require'func_string'
require'common_tools'
require'emu_tools'
require'get_redisip'

local redis = require'redis'
local cjson = require'cjson'

local port_str = arg[1]       ----传入的待处理的一个或多个端口,范围是1~32,多个端口用'-'连接，如1-3-5-6-7表示5个端口
local emu_mod_num = 8
local cmd_head = ""
local _GSM_HEAD = 'gsm-'
local remote_cmd_tail = "\133\000\000"
local atr_cmd_tail = "\129\000\020\059\159\149\000\128\049\224\115\254\033\027\103\016\002\012\003\065\001\065\000"
local t_port_array = {}
local redis_port = 6379
local redis_ip = get_redisip()
local send_list = "app.asterisk.php.checklist"
local client = redis.connect(redis_ip,redis_port)

function connect_redis()
	local response = client:ping()
	if not response then 
		client = redis.connect(redis_ip,redis_port)
	end
end

t_port_array = get_port_info()    ----获取实际端口和物理端口编号的对应信息
connect_redis()

for port in string.gmatch(port_str, "(%d+)") do
	local port_status = ''
	local emu_board = 0
	local emu_port = 0
	local hw_port = 0

	if t_port_array[port] then
		hw_port = tonumber(t_port_array[port])
		emu_port = hw_port % 8
		emu_board = (hw_port-emu_port) / 8 + 1

		local serial,err = connect(emu_board)
		if err ~= nil or serial == nil then
			print("serial connect error.Error info :"..err)
			os.exit(0)
		end

		if     emu_port == 1 then cmd_head = "\000"
		elseif emu_port == 2 then cmd_head = "\001"
		elseif emu_port == 3 then cmd_head = "\002"
		elseif emu_port == 4 then cmd_head = "\003"
		elseif emu_port == 5 then cmd_head = "\004"
		elseif emu_port == 6 then cmd_head = "\005"
		elseif emu_port == 7 then cmd_head = "\006"
		elseif emu_port == 8 then cmd_head = "\007"
		end

		-- send remote command : e.g  00 85 00 00 (0--7)
		remote_cmd = cmd_head..remote_cmd_tail
		remote_cmd_cmp = cmd_head.."\132\000"
		print("Handle EMU: "..emu_board.." Module: "..emu_port)
		print("Send Remote Cmd:")
		local ret = send_cmd_to_serial(serial,remote_cmd,remote_cmd_cmp)
		print("--------------END REMOTE COMMAND--------------\n")

		if ret == 0 then
			-- send ATR command 
			print("Send ATR Cmd:")
			local atr_cmd = cmd_head..atr_cmd_tail
			local atr_cmd_cmp = cmd_head.."\128\000"
			ret = send_cmd_to_serial(serial,atr_cmd,atr_cmd_cmp)
			
			if ret ~= 0 then
				print("    ATR not respond,send again!")
				ret = send_cmd_to_serial(serial,atr_cmd,atr_cmd_cmp)
			end
			print("--------------END ART COMMAND--------------\n")
			
			if ret == 0 then
				-- power reset gsm
				print("GSM Power Restart:")
				restart_gsm_cmd = "asterisk -rx \"gsm power reset "..port.."\""
				print(restart_gsm_cmd)
				os.execute(restart_gsm_cmd)
				print("--------------END GSM RESTART--------------\n")
				
				-- send check command : 80 97 00 00
				print("Send Check Cmd:")
				local recv_cmd = "\128\151\000\000"
				local check_cmd_cmp = cmd_head.."\16\000"
				local check_cmd_cmp2 = cmd_head .. "\000\000\5\160\164\000\000\2"
				ret = send_cmd_to_serial(serial,recv_cmd,check_cmd_cmp,check_cmd_cmp2)
				print("--------------END CHECK COMMAND--------------\n")
				
				if ret == 0 then port_status = "SUCCESSFUL" else port_status = "FAILED" end
			else
				port_status = "FAILED"
				print("Check Status: atr send failed.GSM not restart.")
			end
		else
			port_status = "FAILED"
		end
		print("Check Status: "..port_status)
		serial:close()
	else
		port_status = "FAILED"
	end
	
	-- insert result to redis
	tmp_t = {}
	tmp_t.portname = _GSM_HEAD..port
	tmp_t.result = port_status
	local result = cjson.encode(tmp_t)
	client:rpush(send_list,result)
end


