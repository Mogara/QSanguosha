-- translation for SanD-Zhimeng Package

return {
	["sand_zhimeng"] = "3D·织梦",

	["#diyliru"] = "魔王的智囊", -- qun,3HP
	["diyliru"] = "3D李儒",
	["designer:diyliru"] = "catcat44",
	["illustrator:diyliru"] = "一骑当千",
	["duji"] = "毒计",
	[":duji"] = "出牌阶段，若你的武将牌上没有牌，你可以将一张黑桃牌置于你的武将牌上。当一名其他角色在其出牌阶段使用一张【杀】指定目标后，你可将此牌置于其手上，并令此【杀】当有【酒】效果的【杀】结算，然后该角色须执行下列一项：将武将牌翻面或失去1点体力。",
	["duji:turn"] = "将自己翻面",
	["duji:lp"] = "失去1点体力",
	["du"] = "毒计",
	["sipo"] = "识破",
	[":sipo"] = "<b>锁定技</b>，你不能成为黑桃【杀】或黑桃锦囊的目标。",
	["cv:diyliru"] = "清水浊流",
	["$duji1"] = "此事我当先权宜之。", -- 放牌
	["$duji2"] = "不从者杀之，威权之行，正是此时。", -- 后续结算
	["$shipo"] = "皆在吾意料之中~", -- 成为黑色锦囊目标
	["~diyliru"] = "主公不查，吾等皆死于妇人之手矣！",

	["#diypanfeng"] = "无双上将", -- qun,4HP
	["diypanfeng"] = "3D潘凤",
	["designer:diypanfeng"] = "Why.",
	["illustrator:diypanfeng"] = "凌秋宏",
	["liefu"] = "烈斧",
	[":liefu"] = "当你使用的【杀】被目标角色的【闪】抵消时，你可以令此【杀】依然造成伤害，若如此做，你选择一项：弃置等同于目标角色已损失的体力值数量的牌，不足则全弃；令目标角色摸等同于其当前体力值数量的牌，最多为5张。",
	["#Liefu"] = "%from 发动了【%arg】技能，对 %to 强制命中",
	["liefu:pan"] = "自己弃牌",
	["liefu:feng"] = "对方摸牌",
	["cv:diypanfeng"] = "清水浊流",
	["$liefu1"] = "让你三招~", -- 自己弃牌
	["$liefu2"] = "就这点本事，也想和我斗~", -- 目标摸牌
	["~diypanfeng"] = "额……你竟然……",

	["#diychengyu"] = "世之奇士", -- wei,3HP
	["diychengyu"] = "3D程昱",
	["designer:diychengyu"] = "流云潇雪",
	["illustrator:diychengyu"] = "樱花闪乱",
	["pengri"] = "捧日",
	[":pengri"] = "出牌阶段，你可以获得一名其他角色的一张牌，视为该角色对你使用一张【杀】。每阶段限一次。",
	["gangli"] = "刚戾",
	[":gangli"] = "每当你受到其他角色造成的1点伤害后，你可以：选择除伤害来源外的另一名角色，视为伤害来源对该角色使用一张【杀】（此【杀】无距离限制且不计入出牌阶段使用次数限制）。",
	["cv:diychengyu"] = "清水浊流",
	["$pengri"] = "汝刚而无礼，匹夫之雄耳。",
	["$gangli"] = "拔亡为存，转祸为福。",
	["~diychengyu"] = "王业未毕，吾…怎可离公而去……",

	["#diyjiangwan"] = "安阳亭侯", -- shu,3HP
	["diyjiangwan"] = "3D蒋琬",
	["designer:diyjiangwan"] = "CoffeeNO加糖",
	["illustrator:diyjiangwan"] = "小仓",
	["yaliang"] = "雅量",
	[":yaliang"] = "当你成为其他角色所使用的非延时类锦囊的目标时，你可以摸一张牌，若如此做，该锦囊对你无效，且视为锦囊使用者对你使用了一张【杀】(该【杀】不计入回合使用限制)。",
	["#Yaliang"] = "受到 %from 的【%arg2】影响， %to 的锦囊【%arg】对其无效",
	["xungui"] = "循规",
	[":xungui"] = "出牌阶段，你可以将一张非延时类锦囊置于你的武将牌上，称为“规”。若存在“规”，则弃掉代替之，且你回复1点体力。每阶段限用一次。",
	["daoju"] = "蹈矩",
	[":daoju"] = "出牌阶段，你可以将两张相同颜色手牌当“规”所示锦囊使用。每阶段限用一次。",
	["gui"] = "规",
	["cv:diyjiangwan"] = "喵小林",
	["$yaliang"] = "君子和而不同，何必强求。",
	["$xungui1"] = "天有常道，地有常数。", -- 放置规
	["$xungui2"] = "君子亦有常体也. ", -- 弃置规回复体力
	["$daoju"] = "循规而作，依钜而行~",
	["~diyjiangwan"] = "无力扶汉，有负丞相重托啊~",

	["#diyyuejin"] = "胆识英烈", -- wei,4HP
	["diyyuejin"] = "3D乐进",
	["designer:diyyuejin"] = "R_Shanks",
	["illustrator:diyyuejin"] = "火神网",
	["xiandeng"] = "先登",
	[":xiandeng"] = "摸牌阶段，你可少摸一张牌，然后你无视一名其他角色的距离直到回合结束。",
	["xia0guo"] = "骁果",
	[":xia0guo"] = "出牌阶段，每当你使用非红桃【杀】被目标角色的【闪】抵消时，你可令该【闪】返回该角色手牌中，然后将此【杀】当一张延时类锦囊对该角色使用（黑色当【兵粮寸断】，方块当【乐不思蜀】）。",
	["cv:diyyuejin"] = "黄昏",
	["$xiandeng"] = "轻装便行，夺其先阵~",
	["$xia0guo"] = "当敌制决，计略周备，防不胜防。",
	["~diyyuejin"] = "魏王知遇之恩，今以死相报……",

	["#diychendao"] = "猛将之烈", -- shu,4HP
	["diychendao"] = "3D陈到",
	["designer:diychendao"] = "游神ViVA",
	["illustrator:diychendao"] = "楚汉风云",
	["jingrui"] = "精锐",
	[":jingrui"] = "若你的手牌数不小于你的体力值，你可以将一张手牌当做【杀】或【闪】使用或打出。",
	["cv:diychendao"] = "黄昏",
	["$jingrui1"] = "精锐尽出，尔等何当！", -- 当杀使用
	["$jingrui2"] = "白毦精兵在此，敌岂敢犯边~", -- 当闪使用
	["~diychendao"] = "精锐灭尽，大汉危矣……",

	["#diysimazhao"] = "权倾谋朝", -- wei,3HP
	["diysimazhao"] = "3D司马昭",
	["designer:diysimazhao"] = "殇の腥",
	["illustrator:diysimazhao"] = "火神网",
	["zha0xin"] = "昭心",
	[":zha0xin"] = "出牌阶段，若你的手牌数不小于你的体力值，你可以展示你全部手牌。若其均为不同花色，你令一名角色失去1点体力。若其均为同一种花色，你获得一名其他角色一张牌。每阶段限一次。",
	["huaiyi"] = "怀异",
	[":huaiyi"] = "每当你体力值发生一次变化后，你可以摸一张牌。",
	["cv:diysimazhao"] = "黄昏",
	["$zha0xin1"] = "汝既知吾意，留你不得~", -- 目标失去体力
	["$zha0xin2"] = "吾心中所想，汝等岂知？", -- 获得牌
	["$huaiyi1"] = "陛下，昭岂敢有异心啊！", -- 体力减少
	["$huaiyi2"] = "大权在握，无奈时机尚未成熟。", --恢复体力
	["~diysimazhao"] = "天命在吾司马氏，吾死而不亡……",

	["#diysunluban"] = "矫矜的毒刺", -- wu,3HP
	["diysunluban"] = "3D孙鲁班",
	["designer:diysunluban"] = "小掉线仙",
	["illustrator:diysunluban"] = "阎魔爱",
	["yinsi"] = "淫肆",
	[":yinsi"] = "你可以将一张装备牌当【酒】使用。",
	["chanxian"] = "谗陷",
	[":chanxian"] = "出牌阶段，你可以将一张方片牌交给一名其他角色，该角色进行二选一：1、对其攻击范围内的另一名由你指定的角色使用一张【杀】。2.令你选择获得其一张牌或对其造成一点伤害。每阶段限一次。 ",
	["chanxian:slash"] = "出杀",
	["chanxian:hsals"] = "摊手",
	["chanxian:get"] = "获得其一张牌",
	["chanxian:hit"] = "对其造成一点伤害",
	["@chanxian"] = "受到【谗陷】影响，你可以对 %src 使用一张【杀】",
	["cv:diysunluban"] = "神马芯",
	["$yinsi"] = "将军，饮了此杯，今晚留下陪奴家嘛~",
	["$chanxian1"] = "你还当他没有异心？", -- 效果1
	["$chanxian2"] = "奴家不依，奴家不依啦~", -- 效果2
	["~diysunluban"] = "我不甘心，我不甘心……",
	["`diysunluban"] = "!#$%^)*@……",

	["#diyzhugejin"] = "温厚的盟使", -- wu,4HP
	["diyzhugejin"] = "3D诸葛瑾",
	["designer:diyzhugejin"] = "catcat44",
	["illustrator:diyzhugejin"] = "战国风云",
	["yanhe"] = "言和",
	[":yanhe"] = "回合开始阶段开始时，若你已受伤，你可令一名其他角色装备区里的至多X张牌回到手牌（X为你已损失的体力值）。",
	["youqi"] = "忧戚",
	[":youqi"] = "觉醒技，回合开始阶段结束时，若你的体力为1，你须减1点体力上限，并永久获得技能“缔盟”和“空城”。 ",
	["#YouqiWake"] = "%from 的觉醒技【%arg】被触发",
	["cv:diyzhugejin"] = "喵小林",
	["$yanhe1"] = "永以为好，何操干戈。",
	["$yanhe2"] = "合则两利，分则皆败！",
	["$youqi"] = "吴蜀休憩相关，今虽有隙，然盟不可破。",
	["$dimeng3"] = "吾自为使，保全吴蜀之盟~",
	["~diyzhugejin"] = "吾有负子敬之托啊……",

	["#diyjushou"] = "忠贞义烈", -- qun,3HP
	["diyjushou"] = "3D沮授",
	["designer:diyjushou"] = "神·冥狐",
	["illustrator:diyjushou"] = "一骑当千",
	["quanjian"] = "劝谏",
	[":quanjian"] = "出牌阶段，你可以交给一名其他角色一张【闪】，展示其一张手牌：若为【闪】，则你与该角色各摸一张牌。每阶段限一次。",
	["sijie"] = "死节",
	[":sijie"] = "每当你受到1点伤害后，可弃置一名角色的X张牌（X为该角色已损失的体力值，且至少为1）。 ",
	["@sijie"] = "你可以发动【死节】，弃置一名角色的牌",
	["cv:diyjushou"] = "喵小林",
	["$quanjian"] = "胜负变化，不可不详。",
	["$sijie"] = "授不降也，为军所执耳！",
	["~diyjushou"] = "吾命随袁氏，若蒙公灵，速死为福……",

}
