-- translation for SP Package

return {
	["sp"] = "SP", 
	["gongsunzan"] = "公孙瓒", 
	["yicong"] = "义从", 
	[":yicong"] = "锁定技，只要你的体力值大于2点，你计算与其他角色的距离时，始终-1；只要你的体力值为2点或更低，其他角色计算与你的距离时，始终+1。",
	
	["yuanshu"] = "袁术",
	["yongsi"] = "庸肆",
	[":yongsi"] = "锁定技，摸牌阶段，你额外摸X张牌，X为场上现存势力数。弃牌阶段，你至少弃掉等同于场上现存势力数的牌（不足则全弃）",
	["weidi"] = "伪帝",
	[":weidi"] = "锁定技，你视为拥有当前主公的主公技。",
	
	["tuoqiao"] = "脱壳",
	["xuwei"] = "续尾",
	[":tuoqiao"] = "金蝉脱壳，变身为SP貂蝉",
	[":xuwei"] = "狗尾续貂，变身为原版貂蝉",
	
	["#YongsiGood"] = "%from 的锁定技【庸肆】被触发，额外摸了 %arg 张牌",
	["#YongsiBad"] = "%from 的锁定技【庸肆】被触发，必须至少弃掉 %arg 张牌",
	["#YongsiWorst"] = "%from 的锁定技【庸肆】被触发，弃掉了所有的装备和手牌（共 %arg 张）",
	
	["taichen"] = "抬榇",
	[":taichen"] = "出牌阶段，你可以自减1点体力或弃一张武器牌，弃掉你攻击范围内的一名角色处（手牌、装备区、判定区）的两张牌，每回合中，你可以多次使用抬榇",
	
	["cv:gongsunzan"] = "",
	["cv:yuanshu"] = "名将三国",
	["cv:sp_sunshangxiang"] = "",
	["cv:sp_diaochan"] = "",
	["cv:sp_pangde"] = "",
	
	["designer:sp_pangde"] = "太阳神上",
	
	["$yongsi1"] = "嘿呀，还不错",
	["$yongsi2"] = "呐~哈哈哈哈哈",
	["$yongsi3"] = "呀呀呀呀呀呀",
	["$yongsi4"] = "嗙啪~呜哈哈哈哈哈",
	["~yuanshu"] = "呃呀~~~~~~~",
	
	["sp_sunshangxiang"] = "SP孙尚香",
	["sp_pangde"] = "SP庞德",
	["sp_diaochan"] = "SP貂蝉",

	["shenlvbu1"] = "神吕布(1)",
	["shenlvbu2"] = "神吕布(2)",
	["xiuluo"] = "修罗",
	[":xiuluo"] = "回合开始阶段，你可以弃一张手牌来弃置你判定区里的一张延时类锦囊(必须花色相同)。",
	["shenwei"] = "神威",
	[":shenwei"] = "<b>锁定技</b>，摸牌阶段，你额外摸两张牌，你的手牌上限+2。",	
	["shenji"] = "神戟",
	[":shenji"] = "没装备武器时，你使用的杀可指定至多3名角色为目标。",

	["#Reforming"] = "%from 进入重整状态",
	["#ReformingRecover"] = "%from 在重整状态中回复了1点体力",
	["#ReformingRevive"] = "%from 从重整状态中复活!",
	["draw_1v3"] = "重整摸牌",
	["weapon_recast"] = "武器重铸",
}
