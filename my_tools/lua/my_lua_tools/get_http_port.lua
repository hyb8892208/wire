require'package_path'
require'get_redis_client'
local redis = require'redis'
local client = get_redis_client("127.0.0.1",6379)
local port_list = "httpsms.ports.list"

function get_sms_port()
	local port = client:lpop(port_list)
	if not port then
		return nil
	end
	client:rpush(port_list,port)
	return port
end

--local port = get_sms_port()

--print(port)
