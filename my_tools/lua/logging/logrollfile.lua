require'package_path'
require"logging.rolling_file"

local logger = logging.rolling_file("/log_dir/test2.log", 20)

logger:info("logging.file test")
logger:debug("debugging...")
logger:error("error!")
