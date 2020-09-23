require'package_path'
require'get_redis_client'
require'get_info_tools'
require'get_redisip'
--redis define
local redis_port =6379
local redis = require'redis'
local str = arg[1]

local redis_type = "list"
local list_name = "app.asterisk.slavesms.httplist"
local master_ip = get_redisip()
--+++++++ save infomation in master redis   +++++++++
local client = get_redis_client(master_ip,redis_port)
insert_to_master_redis(client,str,redis_type,list_name)