function get_time()
	local time = os.execute("cat /proc/uptime > /tmp/time.txt")
	local f = io.open("/tmp/time.txt", "r")
	local str = f:read("*all")
	local strs = string.match(str,"(%S*)%.")
	return strs
end
--local time = get_time()
--print(time)
