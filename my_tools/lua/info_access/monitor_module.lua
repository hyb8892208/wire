require'package_path'
local socket = require'socket'

function sleep_linux(n)
	socket.select(nil,nil,n)
end

while true do
	os.execute("lua save_module_type.lua")
	sleep_linux(60)
end

