--read group file
--edit by gy@2015.03.27

require'package_path'
require'get_redis_client'
function readfile(filename)
	local rfile=io.open(filename, "r")
	assert(rfile)
	for str in rfile:lines() do
		--string.find(str, "%[")
		if string.find(str, "%[") then
			--print("find group name ".. str)
			get_group_name(str)
			else
				--print("find group context".. str)
				get_group_context(str)
		end

		--print(str)
	end

	client:set("app.asterisk.group.number", group_nbr)

	--client:set("app.asterisk.grou.name", group_name)

	rfile:close()
end

--------------------------------------------

function writefile(filename, info)
	local wfile=io.open(filename, "w")
	assert(wfile)
	wfile:write(info)
	wfile:close()
end

redis = require "redis"
client = get_redis_client("127.0.0.1","6379")
local response = client:ping()

print(response)
--client:flushall()


function redis_set(key, value)

	client:set(key, value)
	local var = client:get(key)
	print(var)

end

--
function redis_get(key)
end

group_nbr = 0

function get_group_name(line)
	--print(line)
	--name = string.match(line, "[%w_@#%-!$%%%^&%*()~`|%;:'<>%+]+")
	local pos = string.find(line, "%[")
	name = string.sub(line, pos + 1, string.len(line) - 1)
	print(name)

	group_nbr = group_nbr + 1

	client:set("app.asterisk.group."..name..".rrmark", 0)
end

function get_group_context(line)
	--print(line)
	local model
	local context
	local prefix = "app.asterisk.group."

	local pos = string.find(line, "=")
	if pos then
		context = string.sub(line, pos + 1, string.len(line))
		--print(context)
		model = string.sub(line, 1, string.len(line) - string.len(context) - 1)
		--print(model)
	else
		return
	end

	if string.find(model, "order") then
		key = prefix..name..".order"
		--print(key)
	elseif string.find(model, "type") then
		key = prefix..name..".type"
	elseif string.find(model, "policy") then
		key = prefix..name..".policy"
	elseif string.find(model, "members") then
		key = prefix..name..".members"
	else
		print("blank line")
	end

	client:set(key, context)
end

--[[
function get_group_type(line)
	print(line)
	pos = string.find(line, "=")
	type = string.sub(line, pos + 1, string.len(line))
	print(type)
	end
--]]

readfile("/etc/asterisk/gw_group.conf")
--init_group_info("/etc/asterisk/gw_group.conf")

--writefile("test.txt", "wtf!!!!!\n")
