require'package_path'
require'get_redis_client'
require'get_port_status'
local redis = require'redis'
local cjson = require'cjson'

local client = get_redis_client("127.0.0.1","6379")

local back_list_in = "app.asterisk.httpsms.back.in"
local back_list_out = "app.asterisk.httpsms.back"

--clear list
client:del(back_list_in)
client:del(back_list_out)
--clear cmd
local clear_cmd = "/my_tools/redis-cli  KEYS \""..back_list_out.."*\" | xargs /my_tools/redis-cli DEL"
os.execute(clear_cmd)

while true do
	local back_t = {}
	local blpop_t = {}
	local tmp_t = {}
	blpop_t = client:blpop(back_list_in,0)
	tmp_t= cjson.decode(blpop_t[2])
	table.insert(back_t,tmp_t)
	local message = tmp_t.message
	local uuid = tmp_t.uuid
	local report = tmp_t.report
	local pidnum = tmp_t.pidnum
	--print("pidnum is: "..pidnum)
	if report == "(null)" then
		report = client:hget("/etc/asterisk/gw/sms.conf","http_to_sms.report")
	end
	local n = tonumber(tmp_t.sum) - 1
	
	--send num > 1 
	while true do
		if n <= 0 then
			break
		end
		local m = client:blpop(back_list_in,0)
		local m_t = cjson.decode(m[2])
		if m_t.uuid == uuid then
			table.insert(back_t,m_t)
			n = n - 1
		else 
			client:rpush(back_list_in,m[2])
		end
	end
	
	function sort_by_queue(a,b)
		return a.queue < b.queue
	end
	table.sort(back_t,sort_by_queue)
	
	local str_json_t = {}
	local str_noreport_t = {}
	local str_str_t = {}
	for i = 1, #back_t do
		--print("pidnum is(n): "..pidnum)
	--	back_t[i].from = get_port_name(back_t[i].from)
		local str_json = string.format("\t\t\t\"%d\":\[{\
				\"port\":\"%s\",\
				\"phonenumber\":\"%s\",\
				\"time\":\"%s\",\
				\"result\":\"%s\"\n\t\t\t}]",
		i,back_t[i].from,back_t[i].to,
		back_t[i].time,
		back_t[i].result)
		
		local str_noreport = string.format("\t--record %d start--\
	result:%s\
	--record %d end--",i,back_t[i].result,i)
	
		local str_str = string.format("\t--record %d start--\
	port:%s\
	phonenumber:%s\
	time:%s\
	result:%s\
	--record %d end--",
		i,back_t[i].from,
		back_t[i].to,
		back_t[i].time,
		back_t[i].result,i)
	
		table.insert(str_json_t,str_json)
		table.insert(str_noreport_t,str_noreport)
		table.insert(str_str_t,str_str)
		
	end
	local tmp_json = table.concat(str_json_t,",\n")
	local tmp_noreport = table.concat(str_noreport_t,"\n\n")
	local tmp_str = table.concat(str_str_t,"\n\n")
	local out_json = "\t{\n\t\t\"message\":" .. "\"" .. message .. 
			"\",\n\t\t\"report\":\[{\n" .. tmp_json ..
			"\n\t\t}\]\n\t}"
--	print(out_json)
	local out_noreport = "\tmessage:" .. message .. "\n\n" .. tmp_noreport .. "\n"
--	print(out_noreport)
	local out_str = "\tmessage:" .. message .. "\n\n" .. tmp_str .. "\n"
--	print(out_str)
	local back_list_outpid = back_list_out .. pidnum
	--print(back_list_outpid)
	report = string.lower(report)
	if report == "string" then
--		client:rpush(back_list_out,out_str)
		client:rpush(back_list_outpid,out_str)
	elseif report == "json" then
--		client:rpush(back_list_out,out_json)
		client:rpush(back_list_outpid,out_json)
	else
		client:rpush(back_list_outpid,out_noreport)
	end
end
