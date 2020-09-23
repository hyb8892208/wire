
function get_command_ret(board_type)
	local module_uc15 = "UC15"
	local module_sim840 = "SIM840"
	local module_m35 = "M35"
	local t = {}
	for i = 1,board_type do
		local str = "asterisk -rx \"gsm send sync at " .. i .. " AT+GMM 10000 \""
		table.insert(t,str)
	end
	table.foreach(t,print)
	
	while true do
		for i = 1, #t do
			local command_f = io.popen(t[i])
			local command_ret = command_f:read("*a")
			if string.find(command_ret,module_uc15) or string.find(command_ret,module_sim840) or string.find(command_ret,module_m35) then
				return command_ret
			end
		end
		os.execute("sleep 2")
	end
end

local command_ret = get_command_ret(4)
print(command_ret)
local module_name
local module_uc15 = "UC15"
local module_sim840 = "SIM840"
local module_m35 = "M35"

if string.find(command_ret,module_uc15) then
	module_name = "uc15"
elseif string.find(command_ret,module_sim840) then
	module_name = "sim840"
elseif string.find(command_ret,module_m35) then
	module_name = "m35"
end
print(module_name)

function save_module_type(file_name,module_name,line_n,board_type)
	local f = assert(io.open(file_name,"w"))
	local file_str = ""
	for i = 1,line_n do
		local str = module_name;
		for j = 1, board_type -1  do
			str =  str .. "," .. module_name
		end
		file_str = file_str .. str .. "\n"
	end
	file_str = string.sub(file_str,0,-2)
--	print(file_str)
	f:write(file_str)
	f:close()
end

save_module_type("/tmp/.module_type",module_name,11,4)
