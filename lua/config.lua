
-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
	version = "20120405",
	version_name = "踏青版",
	mod_name = "official",
	kingdoms = { "wei", "shu", "wu", "qun", "god"},
	package_names = {
		"StandardCard",
        "StandardExCard",
        "Maneuvering",
        "SPCard",
        "YitianCard",
        "Nostalgia",
        "Joy",
        "Disaster",
        "JoyEquip",

        "Standard",
        "Wind",
        "Fire",
        "Thicket",
        "Mountain",
        "God",
        "SP",
        "YJCM",
        "YJCM2012",
        "Special3v3",
        "BGM",
        "Yitian",
        "Wisdom",
        "Test";
	},

	scene_names = {
		"Guandu",
        "Fancheng",
        "Couple",
        "Zombie",
        "Impasse",
        "Custom",
	},
}

for i=1, 21 do
	local scene_name = ("MiniScene_%02d"):format(i)
	table.insert(config.scene_names, scene_name)
end

