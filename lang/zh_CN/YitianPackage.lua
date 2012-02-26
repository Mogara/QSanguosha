-- translation for YitianPackage

return {
	["yitian"] = "倚天",
	["yitian_cards"] = "倚天卡牌包",

	["yitian_sword"] = "倚天剑",
	[":yitian_sword"] = "1. 你的回合外，你每受到一次伤害，在其结算完后你可以立即使用一张【杀】2. 当此剑从装备区失去时，你可用此剑指定一名其他角色受到你的1点无属性伤害",
	["yitian_sword:yes"] = "你可以使用指定任意一名角色受到你的一点无属性伤害",
	["yitian-lost"] = "倚天剑",
	["@yitian-sword"] = "您在回合外获得了一张【杀】，您可以此时将此【杀】打出",

	-- 神曹操内测第三版
	["shencc"] = "魏武帝",
	["guixin2"] = "归心",
	[":guixin2"] = "回合结束阶段，你可以做以下二选一：\
      1. 永久改变一名其他角色的势力\
      2. 永久获得一项未上场或已死亡角色的主公技。(获得后即使你不是主公仍然有效)",
	["guixin2:yes"] = "永久改变一名其他角色的势力或永久获得一项未上场或已死亡角色的主公技",
	["guixin2:modify"] = "永久改变一名其他角色的势力",
	["guixin2:obtain"] = "永久获得一项未上场或已死亡角色的主公技",

	["fanji"] = "反计",
	[":fanji"] = "若其他角色使用的一张以你为唯一目标的非延时锦囊牌在结算完之后进入弃牌堆，你可以立即获得它",

	["yx_sword"] = "杨修剑",
	[":yx_sword"] = "攻击范围：３<br/>当你的【杀】造成伤害时，可以指定攻击范围内的一名其他角色为伤害来源，杨修剑归该角色所有",

	-- 曹冲
	["caochong"] = "曹冲",
	["chengxiang"] = "称象",
	[":chengxiang"] = "每当你受到1次伤害，你可打出X张牌（X小于等于3），它们的点数之和与造成伤害的牌的点数相等，你可令X名角色各恢复1点体力（若其满体力则摸2张牌）",
	["@chengxiang-card"] = "请打出点数之和为 %arg 的卡牌以发动【称象】技能",
	["conghui"] = "聪慧",
	[":conghui"] = "<b>锁定技</b>，你将永远跳过你的弃牌阶段",
	["zaoyao"] = "早夭",
	[":zaoyao"] = "<b>锁定技</b>，回合结束阶段开始时，若你的手牌大于13张，则你必须弃掉所有手牌并流失1点体力",

	-- 张郃
	["zhangjunyi"] = "张儁乂",
	["jueji"] = "绝汲",
	[":jueji"] = "你可以跳过你的出牌阶段并和一名角色拼点。若你赢，你获得对方的拼点牌，并可立即再次与其拼点，如此反复，直到你没赢或不愿意继续拼点为止",
	["@jueji"] = "绝汲",
	["@jueji-pindian"] = "你可以发动【绝汲】与一名角色拼点",

	-- 陆抗
	["lukang"] = "陆抗",
	["lukang_weiyan"] = "围堰",
	[":lukang_weiyan"] = "你可以将你的摸牌阶段当作出牌阶段，出牌阶段当作摸牌阶段执行",
	["lukang_weiyan:normal"] = "正常顺序",
	["lukang_weiyan:choice1"] = "弃牌-摸牌-出牌",
	["lukang_weiyan:choice2"] = "出牌-弃牌-摸牌",
	["#WeiyanChoice1"] = "%from 选择的行动顺序是<b>弃牌-摸牌-出牌</b>",
	["#WeiyanChoice2"] = "%from 选择的行动顺序是<b>出牌-弃牌-摸牌</b>",
	["kegou"] = "克构",
	[":kegou"] = "<b>觉醒技</b>，回合开始阶段开始时，若你是除主公外唯一的吴势力角色，你须减少1点体力上限并获得技能“连营”",
	["#KegouWake"] = "%from 是场上唯一的吴势力角色，满足克构的觉醒条件",
	["lukang_weiyan:draw2play"] = "您是否想 <b>围堰</b> 将 <b>摸牌阶段</b> 当成 <b>出牌阶段</b> 来执行？",
	["lukang_weiyan:play2draw"] = "您是否想 <b>围堰</b> 将 <b>出牌阶段</b> 当成 <b>摸牌阶段</b> 来执行？",

	-- 夏侯涓
	["xiahoujuan"] = "夏侯涓",
	["lianli"] = "连理",
	["tongxin"] = "同心",
	["liqian"] = "离迁",
	["qiaocai"] = "樵采",
	[":lianli"] = "回合开始阶段开始时，你可以选择一名男性角色，你和其进入连理状态直到你的下回合开始：该角色可以帮你出闪，你可以帮其出杀",
	[":tongxin"] = "处于连理状态的两名角色，每受到一点伤害，你可以令你们两人各摸一张牌",
	[":liqian"] = "<b>锁定技</b>，当你处于连理状态时，势力为蜀；当你处于未连理状态时，势力为魏",
	[":qiaocai"] = "当你处于未连理状态时，出牌阶段，你可以获得一名角色判定区的所有牌，每阶段限一次",

	["$lianli"] = "连理并蒂，比翼不移",
	["$tongxin"] = "执子之手，与子偕老",
	["$qiaocai"] = "陟彼南山，言采其樵。未见君子，忧心惙惙",
	["~xiahoujuan"] = "行与子逝兮，归于其室",

	["lianli-slash"] = "连理（杀）",
	["lianlislash"] = "连理（杀）",
	["lianli-jink"] = "连理(闪)",
	["lianli-slash:slash"] = "你是否想你的连理角色替你出杀？",
	["lianli-jink:jink"] = "你是否想你的连理角色替你出闪？",
	["@lianli-slash"] = "请提供一张杀给你的连理对象",
	["@lianli-jink"] = "请提供一张闪给你的连理对象",
	[":lianli-slash"] = "与你处于连理状态的女性角色可以替你出杀",

	["@@lianli-card"] = "请选择一名要连理的对象",
	["#LianliConnection"] = "%from 与 %to 结为连理",
	["@tied"] = "连理",

	-- 神司马
	["jinxuandi"] = "晋宣帝",
	["wuling"] = "五灵",
	[":wuling"] = "回合开始阶段，你可选择一种五灵效果发动，该效果对场上所有角色生效\
	该效果直到你的下回合开始为止，你选择的五灵效果不可与上回合重复\
	[风]场上所有角色受到的火焰伤害+1\
	[雷]场上所有角色受到的雷电伤害+1\
	[水]场上所有角色使用桃时额外回复1点体力\
	[火]场上所有角色受到的伤害均视为火焰伤害\
	[土]场上所有角色每次受到的属性伤害至多为1",
	["#WulingWind"] = "%from 受到【五灵（风）】的影响，火焰伤害从 %arg 上升到 %arg2",
	["#WulingThunder"] = "%from 受到【五灵（雷）】的影响，雷电伤害从 %arg 上升到 %arg2",
	["#WulingFire"] = "%from 受到【五灵（火）】的影响，伤害属性变为火焰属性",
	["#WulingWater"] = "%from 受到【五灵（水）】的影响，吃桃后额外恢复一点体力",
	["#WulingEarth"] = "%from 受到【五灵（土）】的影响，属性伤害减少至 1 点",

	["wuling:wind"] = "[风]场上所有角色受到的火焰伤害+1",
	["wuling:thunder"] = "[雷]场上所有角色受到的雷电伤害+1",
	["wuling:water"] = "[水]场上所有角色使用桃时额外回复1点体力",
	["wuling:fire"] = "[火]场上所有角色受到的伤害均视为火焰伤害",
	["wuling:earth"] = "[土]场上所有角色每次受到的属性伤害至多为1",

	["$wuling1"] = "长虹贯日，火舞旋风",
	["$wuling2"] = "追云逐电，雷动九天",
	["$wuling3"] = "云销雨霁，彩彻区明",
	["$wuling4"] = "举火燎天，星沉地动",
	["$wuling5"] = "大地光华，承天载物",
	["~jinxuandi"] = "千年恩怨，一笔勾销，历史轮回，转身忘掉",

	["@wind"] = "五灵(风)",
	["@thunder"] = "五灵(雷)",
	["@fire"] = "五灵(火)",
	["@water"] = "五灵(水)",
	["@earth"] = "五灵(土)",

	-- 蔡琰
	["caizhaoji"] = "蔡昭姬",
	["guihan"] = "归汉",
	["caizhaoji_hujia"] = "胡笳",
	[":guihan"] = "出牌阶段，你可以主动弃置两张相同花色的红色手牌，和你指定的一名其他存活角色互换位置。每阶段限一次",
	[":caizhaoji_hujia"] = "回合结束阶段开始时，你可以进行判定：若为红色，立即获得此牌，如此往复，直到出现黑色为止，连续发动3次后武将翻面",
	["$guihan"] = "雁南征兮欲寄边心，雁北归兮为得汉音",
	["$caizhaoji_hujia"] = "北风厉兮肃泠泠。胡笳动兮边马鸣",
	["~caizhaoji"] = "人生几何时，怀忧终年岁",

	-- 陆逊
	["luboyan"] = "陆伯言",
	["luboyanf"] = "陆伯言(女)",
	["shenjun"] = "神君",
	[":shenjun"] = "<b>锁定技</b>，游戏开始时，你必须选择自己的性别。回合开始阶段开始时，你必须倒转性别，异性角色对你造成的非雷电属性伤害无效",
	["shaoying"] = "烧营",
	[":shaoying"] = "当你对一名不处于连环状态的角色造成一次火焰伤害时，你可进行一次判定：若判定结果为红色，则你对其下家造成一点火焰伤害",
	["zonghuo"] = "纵火",
	[":zonghuo"] = "<b>锁定技</b>，你的杀始终带有火焰属性",
	["shenjun:male"] = "男性",
	["shenjun:female"] = "女性",
	["male"] = "男性",
	["female"] = "女性",
	["#ShenjunChoose"] = "%from 选择了 %arg 作为初始性别",
	["#ShenjunProtect"] = "%to 的【神君】锁定技被触发，异性(%from)的非雷电属性伤害无效",
	["#ShenjunFlip"] = "%from 的【神君】锁定技被触发，性别倒置",
	["#Zonghuo"] = "%from 的锁定技【纵火】技能被触发，【杀】变为火焰属性",

	-- 钟会
	["zhongshiji"] = "钟士季",
	["gongmou"] = "共谋",
	["@conspiracy"] = "共谋",
	[":gongmou"] = "回合结束阶段开始时，可指定一名其他角色：其在摸牌阶段摸牌后，须给你X张手牌（X为你手牌数与对方手牌数的较小值），然后你须选择X张手牌交给对方",
	["#GongmouExchange"] = "%from 发动了【共谋】技能，与 %to 交换了 %arg 张手牌",

	-- 姜维
	["jiangboyue"] = "姜伯约",
	["lexue"] = "乐学",
	[":lexue"] = "出牌阶段，可令一名有手牌的其他角色展示一张手牌，若为基本牌或非延时锦囊，则你可将与该牌同花色的牌当作该牌使用或打出直到回合结束；若为其他牌，则立刻被你获得。每阶段限一次",
	["xunzhi"] = "殉志",
	[":xunzhi"] = "出牌阶段，你可以摸三张牌并变身为其他未上场或已阵亡的蜀势力角色，回合结束后你立即死亡",
	["$lexue1"] = "勤习出奇策,乐学生妙计",
	["$lexue2"] = "此乃五虎上将之勇",
	["$lexue3"] = "此乃诸葛武侯之智",
	["$xunzhi1"] = "丞相,计若不成,维亦无悔!",
	["$xunzhi2"] = "蜀汉英烈,忠魂佑我!",
	["~jiangboyue"] = "吾计不成,乃天命也!",

	-- 贾诩
	["jiawenhe"] = "贾文和",
	["dongcha"] = "洞察",
	[":dongcha"] = "回合开始阶段开始时，你可以指定一名其他角色：该角色的所有手牌对你处于可见状态，直到你的本回合结束。其他角色都不知道你对谁发动了洞察技能，包括被洞察的角色本身",
	["dushi"] = "毒士",
	[":dushi"] = "<b>锁定技</b>，杀死你的角色获得崩坏技能直到游戏结束",
	["@collapse"] = "崩坏",

	-- 古之恶来
	["guzhielai"] = "古之恶来",
	["sizhan"] = "死战",
	[":sizhan"] = "<b>锁定技</b>，当你受到伤害时，防止该伤害并获得与伤害点数等量的死战标记；你的回合结束阶段开始时，你须弃掉所有的X个死战标记并流失X点体力",
	["shenli"] = "神力",
	[":shenli"] = "<b>锁定技</b>，出牌阶段，你使用【杀】造成的<font color='red'>第一次伤害</font>+X，X为当前死战标记数且最大为3",
	["#SizhanPrevent"] = "%from 的锁定技【死战】被触发，防止了当前的 %arg 点伤害",
	["#SizhanLoseHP"] = "%from 的锁定技【死战】被触发，流失了 %arg 点体力",
	["#ShenliBuff"] = "%from 的锁定技【神力】被触发，【杀】的伤害增加了 %arg, 达到了 %arg2 点",
	["@struggle"] = "死战",

	-- 邓艾
	["dengshizai"] = "邓士载",
	["zhenggong"] = "争功",
	[":zhenggong"] = "其他角色的回合开始前，若你的武将牌正面向上，你可以将你的武将牌翻面并立即进入你的回合，你的回合结束后，进入该角色的回合",
	["toudu"] = "偷渡",
	[":toudu"] = "当你的武将牌背面向上时若受到伤害，你可以弃置一张手牌并将你的武将牌翻面，视为对一名其他角色使用了一张【杀】",
	["@zhenggong"] = "争功",
	["$zhenggong"] = "不肯屈人后,看某第一功",
	["$toudu"] = "攻其不意,掩其无备",
	["~dengshizai"] = "蹇利西南,不利东北；破蜀功高,难以北回……",

	-- 张鲁
	["zhanggongqi"] = "张公祺",
	["yishe"] = "义舍",
	[":yishe"] = "出牌阶段，你可将任意数量手牌正面朝上移出游戏称为“米”（至多存在五张）或收回；其他角色在其出牌阶段可选择一张“米”询问你，若你同意，该角色获得这张牌，每阶段限两次",
	["xiliang"] = "惜粮",
	[":xiliang"] = "你可将其他角色弃牌阶段弃置的红牌收为“米”或加入手牌",
	["rice"] = "米",
	["xiliang:put"] = "收为“米”",
	["xiliang:obtain"] = "加入手牌",
	["yisheask"] = "义舍要牌",
	["yisheask:allow"] = "同意",
	["yisheask:disallow"] = "不同意",

	-- 倚天剑
	["yitianjian"] = "倚天剑",
	["zhengfeng"] = "争锋",
	[":zhengfeng"] = " <b>锁定技</b>，当你的装备区没有武器时，你的攻击范围为X，X为你当前体力值。",
	["zhenwei"] = "镇威",
	[":zhenwei"] = "你的【杀】被手牌中的【闪】抵消时，可立即获得该【闪】。",
	["yitian"] = "倚天",
	[":yitian"] = "<b>联动技</b>，当你对曹操造成伤害时，可令该伤害-1",
	["#YitianSolace"] = "%from 发动了技能【倚天】，对 %to 的 %arg 点伤害减至 %arg2 点",

	-- 庞德
	["panglingming"] = "庞令明",
	["taichen"] = "抬榇",
	[":taichen"] = "出牌阶段，你可以自减1点体力或弃置一张武器牌，弃置你攻击范围内的一名角色区域的两张牌。每回合中，你可以多次使用抬榇",
	["$taichen"] = "良将不惧死以苟免，烈士不毁节以求生",
	["~panglingming"] = "吾宁死于刀下，岂降汝乎",

	["yanghu"] = "羊祜",
	["jinshen"] = "谨慎",
	[":jinshen"] = "锁定技，你不能成为【乐不思蜀】与【兵粮寸断】的目标。",

	-- CV
	["cv:shencc"] = "",
	["cv:caochong"] = "",
	["cv:zhangjunyi"] = "",
	["cv:lukang"] = "",
	["cv:jinxuandi"] = "宇文天启",
	["cv:xiahoujuan"] = "妙妙",
	["cv:caizhaoji"] = "妙妙",
	["cv:luboyan"] = "",
	["cv:luboyanf"] = "",
	["cv:zhongshiji"] = "",
	["cv:jiangboyue"] = "Jr. Wakaran",
	["cv:jiawenhe"] = "",
	["cv:guzhielai"] = "",
	["cv:dengshizai"] = "阿澈",
	["cv:zhanggongqi"] = "",
	["cv:yitianjian"] = "",
	["cv:panglingming"] = "Glory",

	["designer:shencc"] = "官方内测第三版",
	["designer:caochong"] = "太阳神上",
	["designer:zhangjunyi"] = "孔孟老庄胡",
	["designer:lukang"] = "太阳神上",
	["designer:jinxuandi"] = "title2009,塞克洛",
	["designer:xiahoujuan"] = "宇文天启，艾艾艾",
	["designer:caizhaoji"] = "冢冢的青藤",
	["designer:luboyan"] = "太阳神上、冢冢的青藤",
	["designer:luboyanf"] = "太阳神上、冢冢的青藤",
	["designer:zhongshiji"] = "Jr. Wakaran",
	["designer:jiangboyue"] = "Jr. Wakaran, 太阳神上",
	["designer:jiawenhe"] = "氢弹",
	["designer:guzhielai"] = "Jr. Wakaran, 太阳神上",
	["designer:dengshizai"] = "Bu懂",
	["designer:zhanggongqi"] = "背碗卤粉, Jr. Wakaran",
	["designer:yitianjian"] = "太阳神上",
	["designer:panglingming"] = "太阳神上",

	["illustrator:shencc"] = "三国志大战",
	["illustrator:caochong"] = "三国志大战",
	["illustrator:zhangjunyi"] = "火凤燎原",
	["illustrator:xiahoujuan"] = "三国志大战",
	["illustrator:lukang"] = "火神原画",
	["illustrator:jinxuandi"] = "梦三国",
	["illustrator:caizhaoji"] = "火星时代实训基地",
	["illustrator:luboyan"] = "真三国无双5",
	["illustrator:luboyanf"] = "阿摸",
	["illustrator:zhongshiji"] = "战国无双3",
	["illustrator:jiangboyue"] = "不详",
	["illustrator:jiawenhe"] = "不详",
	["illustrator:guzhielai"] = "火凤燎原",
	["illustrator:dengshizai"] = "豪杰传",
	["illustrator:zhanggongqi"] = "真三国友盟",
	["illustrator:yitianjian"] = "轩辕剑",
	["illustrator:panglingming"] = "三国志大战",

	["title:luboyanf"] = "玩火的少年",
	["title:zhongshiji"] = "狠毒的野心家",
	["title:jiangboyue"] = "赤胆的贤将",
	["title:jiawenhe"] = "明哲保身",
	["title:guzhielai"] = "不坠悍将",
	["title:dengshizai"] = "破蜀首功",
	["title:zhanggongqi"] = "五斗米道",
	["title:yitianjian"] = "跨海斩长鲸",
	["title:panglingming"] = "枱榇之悟",
}

