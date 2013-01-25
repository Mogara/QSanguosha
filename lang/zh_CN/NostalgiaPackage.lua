-- translation for NostalgiaPackage

return {
	["nostalgia"] = "怀旧卡牌包",

	["MoonSpear"] = "银月枪",
	[":MoonSpear"] = "装备牌·武器\
攻击范围：３\
武器特效：你的回合外，每当你使用或打出了一张黑色手牌（若为使用则在它结算之前），你可以立即对你攻击范围内的任意一名角色使用一张【杀】",
	["@moon-spear-slash"] = "银月枪的技能被触发，请打出一张【杀】以攻击",

	["nostal_general"] = "怀旧包",

	["#nos_fazheng"] = "蜀汉的辅翼",
	["nos_fazheng"] = "老法正",
	["nosenyuan"] = "恩怨",
	[":nosenyuan"] = "<b>锁定技</b>，其他角色每令你回复1点体力，该角色摸一张牌；其他角色每对你造成一次伤害后，需交给你一张红桃手牌，否则该角色失去1点体力。",
	["nosxuanhuo"] = "眩惑",
	[":nosxuanhuo"] = "出牌阶段，你可以将一张红桃手牌交给一名其他角色，然后你获得该角色的一张牌并交给除该角色外的其他角色。每阶段限一次。",
	["#EnyuanRecover"] = "%from 触发【%arg2】，令其恢复1点体力的角色 %to 摸 %arg 张牌",
	["@enyuanheart"] = "请展示一张红桃手牌并交给对方",

	["#nos_xushu"] = "忠孝的侠士",
	["nos_xushu"] = "老徐庶",
	["noswuyan"] = "无言",
	[":noswuyan"] = "<b>锁定技</b>，你使用的非延时类锦囊牌对其他角色无效；其他角色使用的非延时类锦囊牌对你无效。",
	["nosjujian"] = "举荐",
	[":nosjujian"] = "出牌阶段，你可以弃置至多三张牌，然后令一名其他角色摸等量的牌。若你以此法弃置三张同一类别的牌，你回复1点体力。每阶段限一次。",
	["#WuyanBaD"] = "%from 触发【%arg2】，对 %to 使用的锦囊【%arg】无效",
	["#WuyanGooD"] = "%from 触发【%arg2】， %to 使用的锦囊【%arg】对其无效",
	["#JujianRecover"] = "%from 弃置了三张 %arg ，恢复1点体力",

	["#nos_lingtong"] = "豪情烈胆",
	["nos_lingtong"] = "老凌统",
	["nosxuanfeng"] = "旋风",
	[":nosxuanfeng"] = "当你失去一次装备区里的牌时，你可以选择一项：1. 视为对一名其他角色使用一张【杀】；你以此法使用【杀】时无距离限制且不计入出牌阶段内的使用次数限制。2. 对距离为1的一名角色造成1点伤害。",
	["nosxuanfeng:nothing"] = "不发动",
	["nosxuanfeng:damage"] = "对距离1以内的其他角色造成伤害",
	["nosxuanfeng:slash"] = "对任意一名其他角色使用一张【杀】",
	
	["#nos_zhangchunhua"] = "冷血皇后",
	["nos_zhangchunhua"]="张春华",
	["nosshangshi"]="伤逝",
	[":nosshangshi"]="弃牌阶段外，当你的手牌数小于X时，你可以将手牌补至X张（X为你已损失的体力值）",
	
	["#nos_handang"] = "石城侯",
	["nos_handang"] = "老韩当",
	["designer:nos_handang"] = "ByArt",
	["illustrator:nos_handang"] = "DH",
	["cv:nos_handang"] = "风叹息",
	["nosgongqi"] = "弓骑",
	[":nosgongqi"] = "你可以将一张装备牌当【杀】使用或打出；你以此法使用【杀】时无距离限制。",
	["nosjiefan"] = "解烦",
	[":nosjiefan"] = "你的回合外，当一名角色处于濒死状态时，你可以对当前正进行回合的角色使用一张【杀】（无距离限制），此【杀】造成伤害时，你防止此伤害，视为对该濒死角色使用一张【桃】。\
◆你防止此伤害后，只有当之前处于濒死状态的角色依然处于濒死状态，才能执行视为对其使用一张【桃】的效果。\
◆此【杀】若未造成伤害，你不可以再次发动【解烦】，但你可以使用手牌里的【桃】进行响应；此【杀】若造成伤害，且执行视为对其使用一张【桃】的效果后该角色依然处于濒死状态，你可以再次发动【解烦】或使用手牌里的【桃】进行响应。",
	["nosjiefan-slash"] = "请对目标角色使用一张【杀】以响应技能",
	["#NosJiefanNull1"] = "%from 已经脱离了濒死状态，解烦效果二无法执行。",
	["#NosJiefanNull2"] = "%from 已经死透了，解烦效果二无法执行。",
	["#NosJiefanNull3"] = "因为当前为 %from 的回合， %to 不处于濒死状态，解烦效果二无法执行。",

}

