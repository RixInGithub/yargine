#!/usr/bin/env lua
-- i chose lua bcuz i think its just enough higher level than c to not make me lose my mind
-- in lyarg, whitespace is optional unless there's no other way around it.
-- btw _ is whitespace cuz its barely blank (cry about it)
-- so instead of |+ 1 1| you can do |+_1_1| (fuck them snake case ppl)
inpIO = io.open("test.yrg")
inp = inpIO:read("*a")
inpIO:close()
local __inp = inp:sub(1) -- copy of inp
local ops = "+-*/" -- c*... nomz..

function expressionParse()
	local out = ""
	if string.find(ops,__inp:sub(1,1))~=nil then
		if not isWS(__inp:sub(2,2)) then error("expected whitespace") end
		__inp=__inp:sub(3) -- day 5 of saving overhead every day™ (trust)
		slurpWS(inp)

		return
	end
	return out
end

function isWS(c)
	return string.find("\x20\t\r\n_",c)~=nil -- accurate c translation of doing this: |strrchar("...",c)!=NULL|
end

function slurpUntilWS(rule)
	local out = ""
	while not isWS(__inp:sub(1,1)) do
		ch = __inp:sub(1,1)
		if rule == "id" and string.find("%a@#$",ch)==nil then -- sadly i'll have to expand the %l to the whole alphabet ig
			error("ye folk expected an identifier without a foul character in the name")
		end
		out = out .. ch -- if only it was that simple in c...
		__inp = __inp:sub(2) -- stare at this instead, this is easy in c. it's just |__inp++;|. look at it, the beauty hides within.
	end
	if rule=="id" and out=="yar" or out=="yarg" or out=="f" then error("ye folk know the name from the legend of the keywords, is it really them? the keyword gods?") end -- populate with more keywords
	-- if rule=="id" and out == "_specialStackStart" then out = "_"..out end -- actually go shit yourselves
	-- anyways day 4 of saving overhead every day™ (trust)
	return out
end

function slurpWS()
	while isWS(__inp:sub(1,1)) do __inp=__inp:sub(2) end
end

function slurpComment()
	__inp = __inp:sub(__inp:find("\n")+1) -- expand to \r too in c reimplementation
	slurpWS()
end

-- these will be simple c**s in c:
local yargTbl = {}
local yarTbl = {}
local fTbl = {}
parsed = {}
if __inp:sub(1,2) == "#!" then slurpComment() end -- shebang (why not)
while #__inp~=0 do
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
		for _,a in ipairs(otrTbl) do
			if a==id then
				-- a => yar/yarg
				-- id => yarg/yar
				-- isConst => id => yarg, a => yar
				-- not isConst => id => yar, a => yarg
				error(string.format("can't yar%s a yar%s'd yariable", isConst and "g" or "", isConst and "" or "g"))
			end
		end
		slurpWS() -- yup, even before |:|. see, i told you it's optional! (incoming pun on the C reimplementation hehe)
		if string.find(":=",__inp:sub(1,1))==nil then
			error(string.format("ye folk expected a symbol to yar%s", isConst and "g" or ""))
		end
		__inp = __inp:sub(2)
		print("got variable", id)
		table.insert(varTbl,id)
		-- expressions tomorrow
	end
end
