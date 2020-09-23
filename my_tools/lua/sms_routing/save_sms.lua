require'package_path'

local port = arg[1]
local sms = arg[2]

redis = require 'redis'

function trim2(s)
	return s:match "^%s*(.-)%s*$"
end

function port2gsm(port)
	--board = os.execute("cat /tmp/.slotnum")
	local handle = io.popen("cat /tmp/.slotnum")
	local board = handle:read("*a")
	handle:close()
	board = trim2(board)
	--print("board is " .. board)
	buf = "gsm"..board.."."..port .. "::" .. sms
	print("buf is " .. buf)
	return buf
end

function get_master_ip()
	local rfile = io.open("/etc/asterisk/gw/cluster.conf", "r")
	if rfile then
		for str in rfile:lines() do
			if string.find(str, "masterip") then
				pos = string.find(str, "=")
				ip = string.sub(str, pos+1)
			end
		end
	end
	
	if ip == "" then
		ip = '127.0.0.1'
	end
	return ip
end

function main ()
	buf = port2gsm(port)
	--print("buf is " .. buf)
	masterip = get_master_ip()
	--print("master ip is " .. masterip)
	client = redis.connect(masterip, 6379)
	if client == nil then
		print("can't connect to redis")
		return
	end

	local llen = client:llen("app.asterisk.smsrouting.list")
	if llen >= 0 and llen < 1000 then
		local ret = client:lpush("app.asterisk.smsrouting.list", buf)
		if ret > 0 then
			print("OK")
		else
			print("insert wrong")
		end
	end
end

main()
