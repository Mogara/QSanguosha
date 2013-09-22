-- This is the start script of QSanguosha

package.path = package.path .. ";./lua/lib/?.lua"

dofile "lua/utilities.lua"
dofile "lua/sgs_ex.lua"
--dofile "lua/sgs_ex2.lua"
dofile "lua/compatibility.lua"

function load_translation(file)
	local t = dofile(file)
	if type(t) ~= "table" then
		error(("file %s is should return a table!"):format(file))
	end

	sgs.LoadTranslationTable(t)
end

function load_translations()
	local lang = sgs.GetConfig("Language", "zh_CN")
	local subdir = { "", "Audio", "Package" }
	for _, dir in ipairs(subdir) do
		local lang_dir = "lang/" .. lang .. "/" .. dir
		local lang_files = sgs.GetFileNames(lang_dir)
		for _, file in ipairs(lang_files) do
			load_translation(("%s/%s"):format(lang_dir, file))
		end
	end
end

function load_extensions(just_require)
	local scripts = sgs.GetFileNames("extensions")
	local package_names = {}
	for _, script in ipairs(scripts) do
		if script:match(".+%.lua$") then
			local name = script:sub(script:find("%w+"))
			local module_name = "extensions." .. name
			local loaded = require(module_name)
			if not loaded.hidden then
				table.insert(package_names, name)
				sgs.Sanguosha:addPackage(loaded.extension)
			end
		end
	end
	local lua_packages = ""
	if #package_names > 0 then lua_packages = table.concat(package_names, "+") end
	sgs.SetConfig("LuaPackages", lua_packages)
end

if not sgs.GetConfig("DisableLua", false) then
	load_extensions()
end

local done_loading = sgs.Sanguosha:property("DoneLoading"):toBool()
if not done_loading then
	load_translations()
	done_loading = sgs.QVariant(true)
	sgs.Sanguosha:setProperty("DoneLoading", done_loading)
end
