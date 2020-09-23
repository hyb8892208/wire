require'package_path'
require'common_tools'
require'get_redis_slaveip'
require'get_info_tools'

function gen_iptables_cmd(cmd_info)
	local iptables_cmd_head = "xtables-multi iptables "
	local cmd_protocol = cmd_info["protocol"]
	local cmd_port = cmd_info["port"]
	local cmd_ip = cmd_info["ip"]
	local cmd_actions = cmd_info["actions"]
	
	local conf_cmd = ""
	local iptables_cmd = ""
	local iptables_drop_cmd = ""
	local actions_para = ""
	
	iptables_cmd = iptables_cmd_head
	iptables_drop_cmd = iptables_cmd_head
	
	if cmd_actions == "ACCEPT" then
		actions_para = "-A INPUT "
		iptables_drop_cmd = iptables_drop_cmd .."-A INPUT"
	else 
		actions_para = "-A INPUT"
	end
	iptables_cmd = iptables_cmd ..actions_para
	if cmd_protocol ~= nil then
		iptables_cmd = iptables_cmd .. " -p "..cmd_protocol
		iptables_drop_cmd = iptables_drop_cmd .. " -p "..cmd_protocol
	end
	
	if cmd_port ~= nil then
		iptables_cmd = iptables_cmd .. " --dport "..cmd_port
		iptables_drop_cmd = iptables_drop_cmd .. " --dport " .. cmd_port
	end
	
	if cmd_ip ~= nil then
		iptables_cmd = iptables_cmd .. " -s "..cmd_ip
		iptables_drop_cmd = iptables_drop_cmd .. " -s 0/0"
	end

	iptables_cmd = iptables_cmd .." -j "..cmd_actions.."\n"
	if cmd_actions == "ACCEPT" then
		iptables_drop_cmd = iptables_drop_cmd ..  " -j DROP \n"
		--print(conf_cmd)
		return iptables_cmd, iptables_drop_cmd
	else
		--print(iptables_cmd)
		return iptables_cmd, ""
	end

end

function gen_white_cmd(ip)
	local cmd_head = "xtables-multi iptables -A INPUT -s "
	local cmd_tail = " -j ACCEPT \n"
	local white_cmd = cmd_head .. ip .. cmd_tail
	return white_cmd
end
function gen_black_cmd(ip)
	local cmd_head = "xtables-multi iptables -A INPUT -s "
	local cmd_tail = " -j DROP \n"
	local white_cmd = cmd_head .. ip .. cmd_tail
	return white_cmd
end

function gen_custom_list_cmd(custom_ip,list_type)
	local custom_list_cmd = ""
	if custom_ip == nil then
		return ""
	end
	if string.match(custom_ip,",") then
		local ips = lua_string_split(custom_ip,",")
		for _,ip in pairs(ips) do			
			if ip ~= nil then
				if string.len(ip) ~= 0 then
					if list_type == "white_list" then
						local tmp_cmd = gen_white_cmd(ip)
						custom_list_cmd = custom_list_cmd ..tmp_cmd
					elseif list_type == "black_list"  then
						if ip == "0.0.0.0" then
							ip = "0.0.0.0/0"
							local tmp_cmd = gen_black_cmd(ip)
							custom_list_cmd = custom_list_cmd ..tmp_cmd
						else
							local tmp_cmd = gen_black_cmd(ip)
							custom_list_cmd = custom_list_cmd ..tmp_cmd
						end
					else
						return ""
					end
				end
			end
		end
	else 
		if list_type == "white_list" then
			local tmp_cmd = gen_white_cmd(custom_ip)
			custom_list_cmd = custom_list_cmd ..tmp_cmd
		elseif list_type == "black_list"  then
			if custom_ip == "0.0.0.0" then
				custom_ip = "0.0.0.0/0"
				local tmp_cmd = gen_black_cmd(custom_ip)
				custom_list_cmd = custom_list_cmd ..tmp_cmd
			else
				local tmp_cmd = gen_black_cmd(custom_ip)
				custom_list_cmd = custom_list_cmd ..tmp_cmd
			end
		else
			return ""
		end
	end
	
	return custom_list_cmd
end

function get_firewall_settings()
	local firewall_conf_path = "/etc/asterisk/gw/firewall.conf"
	local settings_info = get_conf_info(firewall_conf_path)
	if settings_info == nil then
		return "Read firewall.conf failed."
	else
		return settings_info
	end
end

function config_firewall()
	local cmd = ""
	local switch_conf_path = "/etc/asterisk/gw/firewall.conf"
	local rules_conf_path = "/etc/asterisk/gw/firewall_rules.conf"
	local conf_info = get_conf_info(rules_conf_path)
	local switch_info = get_conf_info(switch_conf_path)
	if  switch_info == nil then
		return "Firewall configurations failed."
	end

	if switch_info['general']['firewall'] == "off" then
		os.execute("xtables-multi iptables -F")
		return "Firewall switch is off. Firewall configurations cleared,ant don't start."
	else
		-- ping switch 
		local ping_sw = switch_info['general']['ping']
		if ping_sw == "off" then
			local ping_cmd = "xtables-multi iptables -A INPUT -i eth0 -p icmp --icmp-type echo-request -j DROP \n"
			cmd = cmd ..ping_cmd
		end
		-- WHITE LIST : include two parts : slave && custom config
		-- WHITE LIST SLAVE : open all slave authority ,add it to white_list
		local slave_ip = get_redis_slaveip()
		for key,ip in pairs(slave_ip) do
			slave_white_cmd = gen_white_cmd(ip)
			cmd = cmd ..slave_white_cmd
		end
		-- WHITE LIST CUSTOM : get ip from "/etc/asterisk/gw/firewall.conf"--white_list
		if switch_info['white_list']['enable'] == "on" then
			local custom_white_ip = switch_info['white_list']['ip']
			local custom_white_cmd = gen_custom_list_cmd(custom_white_ip,"white_list")
			cmd = cmd .. custom_white_cmd
		end	

		-- BLACK LIST CUSTOM : only custom config
		if switch_info['black_list']['enable'] == "on" then
			local custom_black_ip = switch_info['black_list']['ip']
			local custom_black_cmd = gen_custom_list_cmd(custom_black_ip,"black_list")
			cmd = cmd .. custom_black_cmd
		end
		
		--generate iptables rules command from firewall_rules.conf
		local drop_cmd = ""
		if conf_info ~= nil then
			local drop_all_flag = 0
			for label,info in pairs(conf_info) do
				local cmd_str = ""
				local drop_str = ""
				cmd_str, drop_str = gen_iptables_cmd(info)
				cmd = cmd ..cmd_str
				drop_cmd = drop_cmd .. drop_str
			end
		end
		cmd = cmd .. drop_cmd

		-- execute iptables commands
		os.execute("xtables-multi iptables -F")
		os.execute(cmd)
		return "Firewall configurations success."
	end
end

local result = ""
result = config_firewall()
print("result is \""..result.."\"")
