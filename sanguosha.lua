-- This is the start script of QSanguosha

package.path = package.path .. ";./lua/lib/?.lua"

dofile "lua/sgs_ex.lua"

function load_translation(file)
	local t = dofile(file)
	if type(t) ~= "table" then
	    error(("file %s is should return a table!"):format(file))
	end
	
	for key, value in pairs(t) do
		sgs.AddTranslationEntry(key, value)		
	end
end


function load_translations()
	local lang = sgs.GetConfig("Language", "zh_CN")
	local lang_dir = "lang/" .. lang

	local lang_files = sgs.GetFileNames(lang_dir)
	for _, file in ipairs(lang_files) do	
		load_translation(("%s/%s"):format(lang_dir, file))
	end
end

function load_extensions()
	local scripts = sgs.GetFileNames("extensions")
	
	for _, script in ipairs(scripts) do		
		local filename = ("extensions/%s"):format(script)
		local package = dofile(filename)
		sgs.Sanguosha:addPackage(package)
	end
end

load_translations()
load_extensions()
