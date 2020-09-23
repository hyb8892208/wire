require'package_path'
require'get_redis_client'
require'get_redisip'

local port = arg[1]
local sms = arg[2]

function trim2(s)
	return s:match "^%s*(.-)%s*$"
end

function port2gsm(port)
	--board = os.execute("cat /tmp/.slotnum")
	local handle = io.popen("cat /tmp/.slotnum")
	local board = handle:read("*a")
	handle:close()
	board = trim2(board)
	board = tonumber(board)

	local f = io.popen("cat /tmp/.slot_type")
	local slot_type = f:read("*a")
	f:close()
	slot_type = trim2(slot_type)
	slot_type = tonumber(slot_type)

	if slot_type == 1 then
		board = 1
	end

	--print("board is " .. board)
	buf = "gsm"..board.."."..port .. "::" .. sms
	print("buf is " .. buf)
	return buf
end

local redis_ip = get_redisip()
local client = get_redis_client(redis_ip, 6379)

function main ()
	buf = port2gsm(port)
	--print("buf is " .. buf)
	local llen = client:llen("app.asterisk.smsrouting.list")
	if llen >= 0 and llen < 1000 then
		local ret = client:rpush("app.asterisk.smsrouting.list", buf)
		if ret > 0 then
			print("OK")
		else
			print("insert wrong")
		end
	end
end

--local f = io.popen("ps -ef | grep sms_routing.lua | grep -v grep | wc -l")
--local n = tonumber(f:read("*all"))
--if n > 0 then
        main()
--elseif client:llen("app.asterisk.smsrouting.list") > 0 then
--end
