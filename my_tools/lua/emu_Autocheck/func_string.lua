-- string functions

function bytes2HexString(bytes)
	local hexstr = "";
	for i = 1, string.len(bytes) do
		local charcode = tonumber(string.byte(bytes, i, i));
		hexstr = hexstr .. string.format("%02X", charcode) .. " ";
	end
	return hexstr;
end
