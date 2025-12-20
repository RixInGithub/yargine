#!/usr/bin/env lua
inpIO = io.open("test.yrg")
inp = inpIO:read()
inpIO:close()

tkns = {}
while #inp do
	if inp:sub(1,3) == "..." then
		-- comment
		inp = inp:sub(inp:find("\n")+1)
	end
end
