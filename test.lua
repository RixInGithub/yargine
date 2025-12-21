#!/usr/bin/env lua
-- i chose lua bcuz i think its just enough higher level than c to not make me lose my mind
-- in yargL, whitespace is optional unless there's no other way around it.
-- btw _ is whitespace cuz its barely blank (cry about it)
-- so instead of |+ 1 1| you can do |+_1_1| (fuck them snake case ppl)
inpIO = io.open("test.yrg")
inp = inpIO:read("*a")
inpIO:close()

function isWS(c)
	return c:find("[\x20\t\r\n_]")~=nil -- accurate c translation of doing this: |strrchar("...",c)!=NULL|
end

function slurpUntilWS(rule)
	out = ""
	while not isWS(inp:sub(1,1)) do
		ch = inp:sub(1,1)
		if rule == "id" and ch:find("[%a@#$]")==nil then -- sadly i'll have to expand the %l to the whole alphabet ig
			error("ye folk expected an identifier without a foul character in the name")
		end
		out = out .. ch -- if only it was that simple in c...
		inp = inp:sub(2) -- stare at this instead, this is easy in c. it's just |inp++;|. look at it, the beauty hides within.
	end
	if rule=="id" and out=="yar" or out=="yarg" or out=="f" then error("ye folk know the name from the legend of the keywords, is it really them? the keyword gods?") end -- populate with more keywords
	-- if rule=="id" and out == "_specialStackStart" then out = "_"..out end -- actually go shit yourselves
	-- anyways day 4 of saving overhead every day™ (trust)
	return out
end

function slurpWS()
	while isWS(inp:sub(1,1)) do inp=inp:sub(2) end
end

function slurpComment()
	inp = inp:sub(inp:find("\n")+1) -- expand to \r too in c reimplementation
	slurpWS()
end

-- these will be simple c**s in c:
yargTbl = {}
yarTbl = {}
fTbl = {}
parsed = {}
if inp:sub(1,2) == "#!" then slurpComment() end -- shebang (why not)
while #inp~=0 do
	slurpWS()
	while inp:sub(1,3) == "..." do -- day 3 of saving overhead every day™ (trust)
		slurpComment()
	end
	if #inp==0 then break end
	if inp:sub(1,4)=="yarg" or inp:sub(1,3)=="yar" then
		local isConst = inp:sub(1,4)=="yarg"
		local varTbl = isConst and yargTbl or yarTbl
		skipTkn = 4+(isConst and 1 or 0)
		inp = inp:sub(skipTkn)
		slurpWS()
		id = slurpUntilWS("id")
		for _,a in ipairs(yargTbl) do
			if a==id then
				error(string.format("ye folk haveth heard the yar of %s, so ye can't make a yarg for the same thing", a))
			end
		end
		slurpWS() -- yup, even before |:|. see, i told you it's optional! (incoming pun on the C reimplementation hehe)
		if inp:sub(1,1):find("[:=]")==nil then
			error(string.format("ye folk expected a symbol to yar%s", isConst and "g" or ""))
		end
		inp = inp:sub(2)
		-- expressions tommorow
	end
end
