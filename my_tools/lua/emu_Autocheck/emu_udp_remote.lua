require'package_path'
require'func_string'
require'common_tools'
require'emu_tools'
--require'get_port_status'
local socket = require'socket'

local emu_board = 4         ----emu控制的子板数量
local emu_mod_num = 8		----emu控制的每块子板的最大通道数

--get command to serial
--
function set_remote_status(board)
	--local atr_cmd_tail = "\129\000\020\059\159\149\000\128\049\224\115\254\033\027\103\016\002\012\003\065\001\065\000"
	local cmd_head = ""
	local emu_mod_num = emu_mod_num
	local t_port_array = t_port_array
	local remote_cmd_tail = "\133\000\000"

	local serial,err = connect(board)
	if err ~= nil or serial == nil then
		print("EMU NUM:"..board.." serial connect error.Error info :"..err)
		return err
	end

	for key=1,emu_mod_num do
		if     key == 1 then cmd_head = "\000"
		elseif key == 2 then cmd_head = "\001"
		elseif key == 3 then cmd_head = "\002"
		elseif key == 4 then cmd_head = "\003"
		elseif key == 5 then cmd_head = "\004"
		elseif key == 6 then cmd_head = "\005"
		elseif key == 7 then cmd_head = "\006"
		elseif key == 8 then cmd_head = "\007"
		end

		local hw_port = (board-1)*emu_mod_num + key
		local port = 0
		for k,v in pairs(t_port_array) do
			if tonumber(v) ==  hw_port then
				port = k
				break
			end
		end

		-- send remote command : e.g  00 85 00 00 (0--7)
		remote_cmd = cmd_head..remote_cmd_tail
		print("Handle EMU: "..board.." Module: "..key)
		print("Send Remote Cmd:")
		local ret = send_cmd_to_serial(serial,remote_cmd,"")
		print("--------------END REMOTE COMMAND--------------\n")
		
		---- start the GSM module   --------
		if ret ~= 0 then
			print("Remote send FAILED! GSM not start...")
		end
		print("--------------END GSM RESTART--------------\n")
	end
	serial:close()
end

local strtime = os.date() .."\n"
t_port_array = {}
t_port_array = get_port_info()

print("GSM Power Off:")
start_gsm_cmd = "asterisk -rx \"gsm power off all\""
os.execute(start_gsm_cmd)

for i=1,emu_board do
	set_remote_status(i)   ----遍历每个子板,发送本地指令,切换为本地SIM卡模式
end

os.execute("sleep 50")
print("GSM Power Start:")
start_gsm_cmd = "asterisk -rx \"gsm power on all\""
os.execute(start_gsm_cmd)

strtime = strtime..os.date()
print(strtime)
--
