require'package_path'
require "logging"

local logger = logging.new(function(self, level, message)
                             print(level, message)
                             return true
                           end)
                           
logger:setLevel (logging.INFO)
logger:log(logging.INFO, "sending email")

logger:info("trying to contact server")
logger:warn("server did not responded yet")
logger:error("server unreachable")

-- dump a table in a log message
local tab = { a = 1, b = 2 }
logger:debug(tab)

-- use string.format() style formatting
logger:info("val1='%s', val2=%d", "string value", 1234)

-- complex log formatting.
local function log_callback(val1, val2)
	-- Do some complex pre-processing of parameters, maybe dump a table to a string.
	return string.format("val1='%s', val2=%d", val1, val2)
end
-- function 'log_callback' will only be called if the current log level is "DEBUG"
logger:debug(log_callback, "string value", 1234)
