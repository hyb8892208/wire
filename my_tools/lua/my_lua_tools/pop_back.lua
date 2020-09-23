require'package_path'
require'get_redis_client'
local redis = require'redis'
local client = get_redis_client("127.0.0.1","6379")

local t = client:lpop("app.asterisk.php.sendlist")
print(t)
