function get_redisip()
	local f = assert(io.open("/etc/asterisk/gw/cluster.conf","r"))
	local fstr = f:read("*all")
	f:close()

	local redis_ip

	if string.match(fstr,"%s*mode%s*=%s*slave%s*") then
		redis_ip = string.match(fstr,"%s+masterip=(%d+%.%d+%.%d+%.%d+)%s+")
	else
		redis_ip = "127.0.0.1"
	end
	return redis_ip
end


--===============================================
--[[
local ip = get_redisip()
print(ip)
--]]
