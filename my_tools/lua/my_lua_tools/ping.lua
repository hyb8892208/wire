require'package_path'
require'command'
function ping_asterisk(ami_manager)
	local response = ping(ami_manager,"127.0.0.1")
	return response
end
--[[
local ami_manager = get_ami_obj(_,_,send,send,print)
local response = ping_asterisk(ami_manager)
print(response)
--]]
