require'package_path'
require'func_string'
require'common_tools'
require'get_port_status'
require'emu_tools'
local socket = require'socket'

local emu_board = 4    ----emu控制的子板数量
local emu_mod_num = 8  ----emu控制的每块子板的最大通道数

function set_local_status(board)

	local cmd_head = ""
	local emu_mod_num = emu_mod_num
	local t_port_array = t_port_array
	local local_cmd_tail = "\103\000\000"

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
	
		-- send local command : e.g  00 67 00 00 (0--7)
		local_cmd = cmd_head..local_cmd_tail
		local_cmd_cmp = cmd_head.."\102\000"
		print("Handle EMU: "..board.." Module: "..key)
		print("Send Local Cmd:")
		local ret = send_cmd_to_serial(serial,local_cmd,local_cmd_cmp)
		print("--------------END LOCAL COMMAND--------------\n")

		----start the GSM module--------
		if ret ~= 0 then
			print("GSM power start failed! port value is error")
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
	set_local_status(i)   --遍历每个子板,发送本地指令,切换为本地SIM卡模式
end

print("GSM Power Start:")
os.execute("sleep 50")
start_gsm_cmd = "asterisk -rx \"gsm power on all\""
os.execute(start_gsm_cmd)

strtime = strtime..os.date()
print(strtime)
