function init_product_table()
	local t_product_info = {}
	t_product_info["local.product.number"] = ""
	t_product_info["local.product.hw.version"] = ""
	t_product_info["local.product.hw.type"] = ""
	t_product_info["local.product.sw.version"] = ""
	t_product_info["local.product.sw.buildtime"] = ""
	t_product_info["local.product.sw.customid"] = ""
	t_product_info["local.product.serialnumber"] = ""
	t_product_info["local.product.board.type"] = ""
	t_product_info["local.product.board.span"] = ""
	t_product_info["local.product.board.version"] = ""
	--t_product_info["local.product.module.type"] = ""
	t_product_info["local.product.sw.build"] = ""
	
	return t_product_info
end

function init_sys_table()
	local t_sys_info = {}
	t_sys_info["local.system.cpu"] = ""
	t_sys_info["local.system.cpu.bit"] = ""
	t_sys_info["local.system.slotnum"] = "1"
	t_sys_info["local.system.memory"] = ""
	t_sys_info["local.system.kernel"] = ""
	t_sys_info["local.system.kernel.build.time"] = ""
	t_sys_info["local.system.net.iface"] = ""
	t_sys_info["local.system.eth0.ip"] = ""
	t_sys_info["local.system.uboot.boot"] = ""
	t_sys_info["local.system.boot.check"] = ""
	t_sys_info["local.system.boot.error"] = ""
	t_sys_info["local.system.firmware.version"] = ""
	t_sys_info["local.system.firmware.build"] = ""
	
	return t_sys_info 
end

--lua run cmd
function run_linux_cmd(cmd)
	local cmd = cmd
	local cmd_f = io.popen(cmd)
	local res_str = cmd_f:read("*a")
	return res_str
end

function lua_string_split(str, split_char)
    local sub_str_tab = {};
    while (true) do
        local pos = string.find(str, split_char);
        if (not pos) then
            sub_str_tab[#sub_str_tab + 1] = str;
            break;
        end
        local sub_str = string.sub(str, 1, pos - 1);
        sub_str_tab[#sub_str_tab + 1] = sub_str;
        str = string.sub(str, pos + 1, #str);
    end

    return sub_str_tab;
end

function string_trim(str)
	return (string.gsub(str,"^%s*(.-)%s*$","%1"))
end

function insert_to_master_redis(client,info,redis_type,type_name)
	local client = client 
	local info = info
	if redis_type == "string" then
		for name,value in pairs(info) do
			client:set(name,value)
		end
	elseif redis_type == "hash" then
		for name,value in pairs(info) do			
			client:hset(type_name,name,value)
		end
	elseif redis_type == "list" then
		client:rpush(type_name,info)
	end
end
