-- translation for SP Package

return {
	["sp"] = "SP",
	
	--公孙瓒
	["gongsunzan"] = "公孙瓒", 
	["yicong"] = "义从", 
	[":yicong"] = "<b>锁定技</b>，只要你的体力值大于2点，你计算与其他角色的距离时，始终-1；只要你的体力值为2点或更低，其他角色计算与你的距离时，始终+1。",
	["#YicongAttackMode"] = "%from 的锁定技<b>【义从】</b>被触发，与其它角色计算距离时始终-1。",--，%arg
	["#YicongDefendMode"] = "%from 的锁定技<b>【义从】</b>被触发，其它角色与 %from 计算距离时始终+1。",--，%arg
	
	--袁术
	["yuanshu"] = "袁术",
	["yongsi"] = "庸肆",
	[":yongsi"] = "<b>锁定技</b>，摸牌阶段，你额外摸X张牌，X为场上现存势力数。弃牌阶段，你需要弃掉至少等同于场上现存势力数的牌（不足则全弃）。",
	["weidi"] = "伪帝",
	[":weidi"] = "<b>锁定技</b>，你视为拥有当前主公的主公技。",
	["#YongsiGood"] = "%from 的锁定技<b>【庸肆】</b>被触发，额外摸了 %arg 张牌。",
	["#YongsiBad"] = "%from 的锁定技<b>【庸肆】</b>被触发，必须弃掉至少 %arg 张牌。",
	["#YongsiWorst"] = "%from 的锁定技<b>【庸肆】</b>被触发，弃掉了所有的装备和手牌（共 %arg 张）。",	
	
	--SP孙尚香
	["sp_sunshangxiang"] = "SP孙尚香",
	
	--SP貂蝉
	["sp_diaochan"] = "SP貂蝉",
	["xuwei"] = "续尾",
	[":xuwei"] = "狗尾续貂，变身为原版貂蝉，每回合限一次。",
	
	--SP庞德
	["sp_pangde"] = "SP庞德",
	["taichen"] = "抬榇",
	[":taichen"] = "出牌阶段，你可以自减1点体力或弃一张武器牌，弃掉你攻击范围内的一名角色的两张牌（手牌、装备区、判定区），每回合中，你可以多次使用抬榇。",
	["$taichen"] = "抬榇",	
	
	--神·吕布
	["shenlvbu1"] = "神·吕布·最强神话",
	["shenlvbu2"] = "神·吕布·暴怒的战神",
	["xiuluo"] = "修罗",
	[":xiuluo"] = "回合开始阶段，你可以弃一张手牌来弃置你判定区里的一张延时类锦囊（必须花色相同）。",	
	["shenwei"] = "神威",
	[":shenwei"] = "<b>锁定技</b>，摸牌阶段，你额外摸两张牌；你的手牌上限+2。",	
	["shenji"] = "神戟",
	[":shenji"] = "没装备武器时，你使用的杀可指定至多3名角色为目标。",	
	["@xiuluo"] = "请弃掉一张与指定判定牌花色相同的手牌来将此判定牌弃置。",

	["#Reforming"] = "%from 进入重整状态。",
	["#ReformingRecover"] = "%from 在重整状态中回复了1点体力。",
	["#ReformingRevive"] = "%from 从重整状态中恢复！",
	["draw_1v3"] = "重整摸牌",
	["weapon_recast"] = "武器重铸",
	["hulaopass"] = "虎牢关模式",
	
	--SP关羽
	["sp_guanyu"] = "SP关羽",
	["danji"] = "单骑",
	[":danji"] = "<b>觉醒技</b>，回合开始阶段，若你的手牌数大于你当前的体力值，且本局主公为曹操时，你须减1点体力上限并永久获得技能“马术”。",
	["#DanjiWake"] = "%from 的手牌数（%arg）多于体力值（%arg2），且本局主公为曹操，觉醒技【单骑】被触发。",
	
	--武将台词
	["$yicong1"] = "冲啊！",
	["$yicong2"] = "众将听令，摆好阵势，御敌！",
	["~gongsunzan"] = "我军将败，我已无颜苟活于世。",

	["$yongsi1"] = "玉玺在手，天下我有！",
	["$yongsi2"] = "大汉天下已半入我手！",
	["$weidi1"] = "我才是皇帝！",
	["$weidi2"] = "你们都得听我的号令！",
	["~yuanshu"] = "可恶，就差一步了……",
	
	["~sp_sunshangxiang"] = "不，还不可以死……",
	
	["~sp_diaochan"] = "父亲大人，对不起……",
	
	["~sp_pangde"] = "四面都是水，我命休矣……",
	
	["$xiuluo"] = "神挡杀神，佛挡杀佛！",
	["$shenwei"] = "挡我者死！",
	["$shenji"] = "神挡杀神，佛挡杀佛！",
	["~shenlvbu1"] = "看我杀你们个片甲不留！",
	["~shenlvbu2"] = "呃，不可能！",
	
	["$danji"] = "",
	["~sp_guanyu"] = "",
	
	["designer:sp_pangde"] = "太阳神上",
	
	["cv:gongsunzan"] = "官方",
	["cv:yuanshu"] = "官方",
	["cv:sp_sunshangxiang"] = "官方",
	["cv:sp_diaochan"] = "官方",
	["cv:sp_pangde"] = "官方",
	["cv:shenlvbu1"] = "官方",
	["cv:shenlvbu2"] = "官方",
}
