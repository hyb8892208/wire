local p = "../exten_package/"  
local q = "../my_lua_tools/"
local m_package_path = package.path  
package.path = string.format("%s;%s?.lua;%s?/init.lua",  
		    m_package_path, p, p)  
package.cpath = string.format("%s;%s?.so;%s?/loadall.so",  
		    m_package_path, p, p)  
local n_package_path = package.path  
package.path = string.format("%s;%s?.lua;%s?/init.lua",  
		    n_package_path, q, q)  


