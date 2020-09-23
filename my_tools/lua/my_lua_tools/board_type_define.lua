function get_board_type()
    local f = assert(io.open("/tmp/.boardtype","r"))
    local board_type = f:read("*all")
    board_type = tonumber(board_type)
    return board_type
end

function command_ret(command_str)
    local f = io.popen(command_str)
    local ret = f:read("*all")       
    return ret  
end

function read_board_type()
    local str = 'asterisk -rx "gsm show spans"' 
    local result = command_ret(str)
    print(result)
    _,num = string.gsub(result,"GSM span","GSM span")
	--print(num)
    return num
end

local board_type = read_board_type()
local f = assert(io.open("/tmp/.boardtype","wb+"))
f:write(board_type)
f:close()
