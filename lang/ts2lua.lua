-- This is a script that extract translation informatio
-- from TS file to serveral translation files with lua format
-- The script requires 'lxp' which is the expat binding for Lua

require "lxp.lom"

content = io.open("sanguosha.ts"):read("*a")
content = lxp.lom.parse(content)

function parse_message(message)
	local source, translation
	for _, element in ipairs(message) do
		if element.tag == "source" then
			source = element[1]
		elseif element.tag == "translation" then
			translation = element[1]
		end
	end
	
	return { source, translation }
end

function parse_context(context)
	local name
	local messages = {}
	
	for _, element in ipairs(context) do		
		if element.tag == "name" then
			name = element[1]		
		elseif element.tag == "message" then
			table.insert(messages, parse_message(element))			
		end	
	end
	
	if name then
		local f = io.open(name .. ".lua", "w")
		
		f:write("-- translation for " .. name .. "\n\n")
		f:write("return {\n")
		for _, message in ipairs(messages) do	
			local source = message[1]
			local translation = message[2]
			if source and translation then
				f:write(("\t[%q] = %q, \n"):format(source, translation))
			end
		end
		f:write("}\n")
		
		f:close()
	end
end

for _, context in ipairs(content) do
	if context.tag == "context" then
		parse_context(context)
	end
end
