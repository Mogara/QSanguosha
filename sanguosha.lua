-- This is the start script of QSanguosha

function load_translation(file)
	local t = dofile(file)
	if type(t) ~= "table" then
	    error(("file %s is should return a table!"):format(file))
	end
	
	for key, value in pairs(t) do
		sgs.AddTranslationEntry(key, value)		
	end
end

function main()
	local lang = sgs.GetConfig("Language", "zh_CN")
	local lang_dir = "lang/" .. lang

	local lang_files = sgs.GetFileNames(lang_dir)
	for _, file in ipairs(lang_files) do	
		load_translation(("%s/%s"):format(lang_dir, file))
	end
end

local success, msg = pcall(main)
if not success then
	sgs.Alert(msg)
end