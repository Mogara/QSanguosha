
-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
	version = "20120715",
	version_name = "涅槃版",
	mod_name = "official",
	kingdoms = { "wei", "shu", "wu", "qun", "god"},
	package_names = {
	"StandardCard",
        "StandardExCard",
        "Maneuvering",
        "SPCard",
        "Nostalgia",
	"New3v3Card",
	"YitianCard",
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
	"Ling",
        "BGM",
	"NostalGeneral",
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
	
	ai_names = {
		"Kenkic的机器女仆",
		"HyperX的基友",
		"天启的老公",
		"灵魂的⑨",
		"太阳神的苦工",
		"William大神",
		"海泡叉",
		"安岐家的男宠",
		"灵魂家的葱娘手办",
		"启姐家的好丈夫",
		"KK家的机器女仆",
		"神上的化身",
		"donle的最后之作",
		"全宇宙最强的天霜雪舞",
		"导线的电阻",
		"QB的契约",
		"小A喜欢用的外挂",
		"吉祥物小萨",
		"Slob的杀虫剂",
		"克拉克的跑动投",
		"讨厌的核弹",
	},
	
        color_wei = "#547998",
        color_shu = "#D0796C",
        color_wu = "#4DB873",
        color_qun = "#8A807A",
        color_god = "#96943D",
}

for i=1, 33 do
	local scene_name = ("MiniScene_%02d"):format(i)
	table.insert(config.scene_names, scene_name)
end

