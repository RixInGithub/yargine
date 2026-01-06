#!/usr/bin/env lua
--[[ i chose lua bcuz i think its just enough higher level than c to not make me lose my mind
	 in lyarg, whitespace is optional unless there's no other way around it.
	 btw _ is whitespace cuz its barely blank (cry about it)
	 so instead of |+ 1 1| you can do |+_1_1| (fuck them snake case ppl) ]]
parsed = {}
local file = arg[1]or"./main.yrg"
inpIO = io.open(file)
inp = inpIO:read("*a")
inpIO:close()
local __inp = inp:sub(1) -- copy of inp
local ops = "+-*/" -- c*... nomz..
local line = 1

function kms(m)
	error(string.format("%s@%d: %s", file, line, m), 0)
end

function isNL(c)
	if #c~=1 then return false end
	return string.find("\r\n",c,1,true)~=nil -- accurate c translation of doing this: |strchr("...",c)!=NULL|
end

function isWS(c)
	if #c~=1 then return false end
	return string.find("\x20\t_",c,1,true)~=nil or isNL(c)
end

function expressionParse()
	local out = ""
	slurpWS() -- just in case
	if string.find(ops,__inp:sub(1,1),1,true)~=nil then
		op = __inp:sub(1,1)
		if not isWS(__inp:sub(2,2)) then kms("expected whitespace") end
		__inp=__inp:sub(3) -- day 4 of saving overhead every day™ (trust)
		slurpWS()
		local firstExp = expressionParse() -- parse again
		out = op .. " " .. firstExp
		if not isWS(__inp:sub(1,1)) then kms("expected whitespace") end
		__inp=__inp:sub(2)
		local sndExp = expressionParse() -- and againz!
		out = out .. " " .. sndExp
		if tonumber(firstExp) ~= nil and tonumber(sndExp) ~= nil then
			a = tonumber(firstExp)
			b = tonumber(sndExp)
			if op == "+" then
				return tostring(a+b)
			end
			if op == "-" then
				return tostring(a-b)
			end
			if op == "*" then
				return tostring(a*b)
			end
			if op == "/" then
				return tostring(a/b):gsub("%.%d+$","") -- simulated integer division
			end
		end
		return out
	end
	if string.find("0123456789",__inp:sub(1,1),1,true)~=nil then
		out = __inp:match("^%d+")
		__inp=__inp:sub(#out+1)
		return out
	end
	if __inp:sub(1,11)=="nonexistent" then
		out = "nonexistent"
		__inp=__inp:sub(#out+1)
		return out
	end
	local pInp = __inp:sub(1)
	local isId, id = pcall(slurpUntilWS, "id")
	if not isId then __inp = pInp end
	if isId then
		pInp1 = __inp:sub(1)
		slurpWS()
		if __inp:sub(1,1) ~= "(" then
			__inp = pInp1
			return id
		end
		__inp = pInp -- prepare for function calling handling
	end
	pInp = __inp:sub(1)
	isId, id = pcall(slurpUntilWS, "fname") -- reuse vars
	if not isId then __inp = pInp end
	if isId then
		out = "f_"..id..">" -- no returns, integrate with array logic below for dry
		slurpWS()
	end
	local open = isId and "(" or "["
	if __inp:sub(1,1) == open then
		local close = isId and ")" or "]" -- ill turn this to some arithmetic on a char in c
		out = out..open
		__inp = __inp:sub(2)
		while __inp:sub(1,1) ~= close do
			slurpWS()
			if __inp:sub(1,1) == "{" and not isId then
				__inp = __inp:sub(2)
				slurpWS()
				if __inp:sub(1,1) ~= "[" then kms("magic knife that splits arrays to nonarrays? howz?") end
				local firstLn = expressionParse()
				slurpWS()
				if __inp:sub(1,1) ~= "," then kms("blunt knife") end
				__inp = __inp:sub(2)
				slurpWS()
				if __inp:sub(1,1) ~= "[" then kms("magic knife that splits arrays to nonarrays? howz?") end
				local sndLn = expressionParse()
				slurpWS()
				if __inp:sub(1,1) ~= "}" then kms("expected }") end
				__inp = __inp:sub(2)
				slurpWS()
				if __inp:sub(1,1) ~= "]" then kms("expected ]") end
				__inp = __inp:sub(2)
				return "{" .. out:sub(1,-2) .. "]>" .. firstLn .. ">" .. sndLn .. "}"
			end
			out = out..expressionParse()
			slurpWS()
			if __inp:sub(1,1) ~= close then
				if __inp:sub(1,1) ~= "," then kms("expected comma") end
				__inp = __inp:sub(2)
				slurpWS()
				if __inp:sub(1,1) == close then kms("ew trailing comma") end
				out = out..","
			end
		end
		__inp = __inp:sub(2)
		return out .. close
	end
	if __inp:sub(1,1) == "\"" then
		__inp = __inp:sub(2)
		while __inp:sub(1,1)~="\"" do
			if #__inp==0 then kms("expected string end, got eof") end
			local ch = __inp:sub(1,1)
			local isEsc = ch == "\\"
			if isNL(ch) then kms("expected string end, got newline") end
			if isEsc then
				__inp = __inp:sub(2) -- skip
				if #__inp==0 then kms("expected char after \\, got eof") end
				local nuxt = __inp:sub(1,1) -- nasty
				if isNL(nuxt) then kms("expected char after \\, got newline") end
				local escs = {r = "\r", n = "\n", t = "\t", b = "\x07", ["\""] = "\""} -- meh, maybe i'll add a json parser to add all the other json-esque escapes.
				if escs[nuxt] == nil then kms("unknown escape \\"..ch) end
				ch = escs[nuxt]
			end
			out = out .. ch
			__inp = __inp:sub(2)
		end
		__inp=__inp:sub(2)
		return "s_"..out
	end
	kms("expected expressionoso")
end

function addToLatestParse(...)
	for _,t in ipairs({...}) do parsed[#parsed] = parsed[#parsed]..#t..">"..t end
end

function slurpUntilWS(rule)
	local out = ""
	while (not isWS(__inp:sub(1,1))) and #__inp>0 do
		local ch = __inp:sub(1,1)
		if (rule == "id" or rule == "fname" or rule == "farg") and ch:find("[%a@#$]")==nil then
			if (rule == "fname" and ch ~= "(") or (rule == "farg" and (ch ~= ")" and ch ~= ",")) or rule == "id" then kms("ye folk expected a name without a foul character inside") end
			break -- we still have to check for keywords and stuff
		end
		out = out .. ch -- if only it was that simple in c...
		__inp = __inp:sub(2) -- stare at this instead, this is easy in c. it's just |__inp++;|. look at it, the beauty hides within.
	end
	if rule=="id" and (out=="yar" or out=="yarg" or out=="f" or out=="nonexistent") then kms("ye folk know the name from the legend of the keywords, is it really them? the keyword gods?") end -- populate with more keywords
	return out
end

function slurpWS()
	while isWS(__inp:sub(1,1)) do
		if isNL(__inp:sub(1,1)) then
			if __inp:sub(1,2) == "\r\n" then __inp=__inp:sub(2) end
			line = line + 1
		end
		__inp=__inp:sub(2)
	end
end

function slurpComment()
	local firstNl = __inp:find("\n")
	local firstCr = __inp:find("\r")
	if firstNl==nil and firstCr==nil then return (function()__inp=""end)() end -- way to get around assignment not being an expression
	local won = -1
	if firstNl == nil then won = firstCr end
	if firstCr == nil then won = firstNl end
	if won==-1 then -- neither are nil, epique
		won = math.min(firstNl, firstCr) -- this'll be a macro in c.
	end
	__inp = __inp:sub(won+1)
	line = line + 1
	slurpWS()
end

local yargTbl = {}
local yarTbl = {}
local fTbl = {}
local fStk = {}
if __inp:sub(1,2) == "#!" then slurpComment() end -- shebang (why not)
while #__inp>0 do
	slurpWS()
	while __inp:sub(1,3) == "..." do -- day 3 of saving overhead every day™ (trust)
		slurpComment()
	end
	if #__inp==0 then break end
	if __inp:sub(1,4)=="yarg" or __inp:sub(1,3)=="yar" then
		local isConst = __inp:sub(1,4)=="yarg"
		local varTbl = isConst and yargTbl or yarTbl
		local otrTbl = isConst and yarTbl or yargTbl
		skipTkn = 4+(isConst and 1 or 0)
		__inp = __inp:sub(skipTkn)
		slurpWS()
		id = slurpUntilWS("id")
		table.insert(parsed,"")
		addToLatestParse("yar"..(isConst and "g" or ""), id)
		for _,a in ipairs(otrTbl) do
			if a==id then
				-- a => yar/yarg
				-- id => yarg/yar
				-- isConst => id => yarg, a => yar
				-- not isConst => id => yar, a => yarg
				kms(string.format("can't yar%s a yar%s'd yariable", isConst and "g" or "", isConst and "" or "g"))
			end
		end
		if isConst then
			for _,a in ipairs(varTbl) do
				if a==id then
					kms("can't yarg a yarg'd yariable")
				end
			end
		end
		for _,a in ipairs(fTbl) do
			if a==id then
				kms(string.format("can't yar%s a function", isConst and "g" or ""))
			end
		end
		slurpWS()
		if __inp:sub(1,1)~="=" then
			kms(string.format("ye folk expected an equal sign to yar%s", isConst and "g" or ""))
		end
		__inp = __inp:sub(2)
		slurpWS()
		table.insert(varTbl,id)
		local exp = expressionParse()
		addToLatestParse(exp)
		if isConst then
			slurpWS()
			if __inp:sub(1,19) == "... shut up yargine" then
				if isNL(__inp:sub(20,20)) then
					slurpComment()
					goto lyarg_continue0
				end
			end
			print(string.format("be warned, it's too late to yar %s now!",id))
		end
		goto lyarg_continue0
	end
	if __inp:sub(1,1)=="f" then
		if not isWS(__inp:sub(2,2)) then kms("expected whitespace") end
		__inp=__inp:sub(3)
		slurpWS()
		name = slurpUntilWS("fname")
		table.insert(parsed,"")
		addToLatestParse("fn", name)
		for _,a in ipairs(yarTbl) do
			if a==name then
				kms("can't make a function with same name as a yariable")
			end
		end
		for _,a in ipairs(yargTbl) do
			if a==name then
				kms("can't make a function with same name as a yariable")
			end
		end
		for _,a in ipairs(fTbl) do
			if a==name then
				kms("\"what do you mean the 'function mitosed'?\"")
			end
		end
		table.insert(fTbl,name)
		table.insert(fStk,name)
		table.insert(yarTbl,"_fnLocals")
		table.insert(yargTbl,"_fnLocals")
		slurpWS()
		if __inp:sub(1,1) ~= "(" then kms("expected opening parentheses before yarguments") end
		__inp=__inp:sub(2)
		local a = ""
		while __inp:sub(1,1) ~= ")" do
			slurpWS()
			a = a..slurpUntilWS("farg")
			slurpWS()
			if __inp:sub(1,1) ~= ")" then
				if __inp:sub(1,1) ~= "," then kms("expected comma") end
				__inp = __inp:sub(2)
				slurpWS()
				if __inp:sub(1,1) == close then kms("even ye folk didn't expect the dreaded trailing comma") end
				a = a..","
			end
		end
		addToLatestParse(a)
		__inp = __inp:sub(2)
		slurpWS()
		if __inp:sub(1,1) ~= "{" then kms("expected curly braces to open function") end
		__inp = __inp:sub(2)
		goto lyarg_continue0
	end
	if __inp:sub(1,1)=="r" then
		if not isWS(__inp:sub(2,2)) then kms("expected whitespace") end
		__inp=__inp:sub(3)
		slurpWS()
		table.insert(parsed,"")
		addToLatestParse("ret", expressionParse())
		goto lyarg_continue0
	end
	if __inp:sub(1,1)=="}" then
		if #fStk ~= 0 then
			__inp=__inp:sub(2)
			toClose = fStk[#fStk]
			table.remove(fStk, #fStk)
			table.insert(parsed,"")
			addToLatestParse("nf", toClose)
			local lastYarIdx = 0
			local lastYargIdx = 0
			for a,b in ipairs(yarTbl) do
				if b=="_fn" then
					lastYarIdx = a
				end
			end
			for a,b in ipairs(yargTbl) do
				if b=="_fn" then
					lastYargIdx = a
				end
			end
			yarTbl = {table.unpack(yarTbl, 0, lastYarIdx-1)}
			yargTbl = {table.unpack(yargTbl, 0, lastYargIdx-1)}
			goto lyarg_continue0 -- if #fStk == 0, fallthrough onto the kms catch all
		end
	end
	kms(string.format("unknown character 0x%x", __inp:byte(1)))
	::lyarg_continue0::
end
print(table.concat(parsed,"\n----\n"))