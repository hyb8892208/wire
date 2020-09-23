function get_board_type()
	local f = assert(io.open("/tmp/.boardtype","r"))
        local board_type = f:read("*all")
        board_type = tonumber(board_type)
        return board_type
end
