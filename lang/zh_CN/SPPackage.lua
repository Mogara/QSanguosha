-- translation for SP Package

return {
	["sp"] = "SP",

	["#yangxiu"] = "恃才放旷",
	["yangxiu"] = "杨修",
	["illustrator:yangxiu"] = "张可",
	["jilei"] = "鸡肋",
	["danlao"] = "啖酪",
	[":jilei"] = "当你受到伤害时，说出一种牌的类别（基本牌、锦囊牌、装备牌），对你造成伤害的角色不能使用、打出或弃置该类别的手牌直到回合结束\
◆弃牌阶段，若仅剩不可弃置的手牌且超出手牌上限时，该角色需展示其手牌。例如，若指定的是装备牌，此时该角色2点体力，同时手里有两张【闪】和三张装备牌，弃牌阶段，该角色必须把两张【闪】弃置，然后该角色展示这三张装备牌，结束他的回合",
	[":danlao"] = "当一个锦囊指定了不止一个目标，且你也是其中之一时，你可以立即摸一张牌，则该锦囊跳过对你的结算",
	["$jilei"] = "食之无味，弃之可惜",
	["$danlao"] = "一人一口，分而食之",
	["~yangxiu"] = "恃才傲物，方有此命",
	["cv:yangxiu"] = "幻象迷宫",
	["#DanlaoAvoid"] = "%from 发动了技能【%arg2】，跳过了锦囊 %arg 对他的结算",
	["#Jilei"] = "%from 鸡肋了 %to 的 %arg",
	["#JileiClear"] = "%from 的鸡肋效果消失",

	["#gongsunzan"] = "白马将军",
	["gongsunzan"] = "公孙瓒",
	["illustrator:gongsunzan"] = "Vincent",
	["yicong"] = "义从",
	[":yicong"] = "<b>锁定技</b>，只要你的体力值大于2点，你计算与其他角色的距离时，始终-1；只要你的体力值为2点或更低，其他角色计算与你的距离时，始终+1",
	["$yicong1"] = "冲啊！",
	["$yicong2"] = "众将听令，摆好阵势，御敌！",
	["~gongsunzan"] = "我军将败，我已无颜苟活于世",

	["#yuanshu"] = "仲家帝",
	["yuanshu"] = "袁术",
	["illustrator:yuanshu"] = "吴昊",
	["yongsi"] = "庸肆",
	[":yongsi"] = "<b>锁定技</b>，摸牌阶段，你额外摸X张牌，X为场上现存势力数。弃牌阶段，你至少弃置等同于场上现存势力数的牌（不足则全弃）",
	["weidi"] = "伪帝",
	[":weidi"] = "<b>锁定技</b>，你视为拥有当前主公的主公技",
	["$yongsi1"] = "嘿呀，还不错",
	["$yongsi2"] = "呐~哈哈哈哈哈",
	["$yongsi3"] = "呀呀呀呀呀呀",
	["$yongsi4"] = "嗙啪~呜哈哈哈哈哈",
	["~yuanshu"] = "呃呀~~~~~~~",
	["cv:yuanshu"] = "名将三国",
	["#YongsiGood"] = "%from 的锁定技【%arg2】被触发，额外摸了 %arg 张牌",
	["#YongsiBad"] = "%from 的锁定技【%arg2】被触发，必须至少弃置 %arg 张牌",
	["#YongsiWorst"] = "%from 的锁定技【%arg2】被触发，弃置了所有的装备和手牌（共 %arg 张）",

	["#sp_diaochan"] = "绝世的舞姬",
	["sp_diaochan"] = "SP貂蝉",
	["illustrator:sp_diaochan"] = "巴萨小马",
	["tuoqiao"] = "脱壳",
	[":tuoqiao"] = "<b>限定技</b>，游戏开始时，你可以选择变身为SP貂蝉",
	["SP-Diaochan"] = "SP貂蝉",

	["#sp_sunshangxiang"] = "梦醉良缘",
	["sp_sunshangxiang"] = "SP孙尚香",
	["illustrator:sp_sunshangxiang"] = "木美人",
	["chujia"] = "出嫁",
	[":chujia"] = "<b>限定技</b>，游戏开始时，你可以选择变身为SP孙尚香，势力为蜀",
	["cv:sp_sunshangxiang"] = "官方，背碗卤粉",

	["#sp_guanyu"] = "汉寿亭侯",
	["sp_guanyu"] = "SP关羽",
	["illustrator:sp_guanyu"] = "LiuHeng",
	["cv:sp_guanyu"] = "喵小林，官方",
	["danji"] = "单骑",
	[":danji"] = "<b>觉醒技</b>，回合开始阶段，若你的手牌数大于你当前的体力值，且本局主公为曹操时，你须减1点体力上限并永久获得技能“马术”。",
	["#DanjiWake"] = "%from 的手牌数(%arg)多于体力值(%arg2)，且本局主公为曹操，达到【单骑】的觉醒条件",
	["$danji"] = "吾兄待我甚厚，誓以共死，今往投之，望曹公见谅。",

	["#sp_caiwenji"] = "金璧之才",
	["sp_caiwenji"] = "SP蔡文姬",
	["illustrator:sp_caiwenji"] = "木美人",
	["guixiang"] = "归乡",
	[":guixiang"] = "<b>限定技</b>，游戏开始时，你可以选择变身为SP蔡文姬，势力为魏",
	["cv:sp_caiwenji"] = "呼呼",
	
	["#sp_jiaxu"] = "算无遗策",
	["sp_jiaxu"] = "SP贾诩",
	["illustrator:sp_jiaxu"] = "雪君S",
	["guiwei"] = "归魏",
	[":guiwei"] = "<b>限定技</b>，游戏开始时，你可以选择变身为SP贾诩，势力为魏",
	["cv:sp_jiaxu"] = "落凤一箭",
	
	["#sp_pangde"] = "枱榇之悟",
	["sp_pangde"] = "SP庞德",
	["illustrator:sp_pangde"] = "天空之城",
	["pangde_guiwei"] = "归魏",
	[":pangde_guiwei"] = "<b>限定技</b>，游戏开始时，你可以选择变身为SP庞德，势力为魏",
	["cv:sp_pangde"] = "Glory",

	["#sp_machao"] = "西凉的猛狮",
	["sp_machao"] = "SP马超",
	["illustrator:sp_machao"] = "天空之城",
	["fanqun"] = "返群",
	[":fanqun"] = "<b>限定技</b>，游戏开始时，你可以选择变身为SP马超，势力为群",

	["#caohong"] = "福将",
	["caohong"] = "曹洪",
	["illustrator:caohong"] = "LiuHeng",
	["designer:caohong"] = "韩旭",
	["cv:caohong"] = "喵小林",
	["yuanhu"] = "援护",
	[":yuanhu"] = "回合结束阶段开始时，你可以将一张装备牌置于一名角色的装备区里，然后根据此装备牌的种类执行以下效果。\
武器牌：弃置与该角色距离为1的一名角色区域中的一张牌；\
防具牌：该角色摸一张牌；\
坐骑牌：该角色回复一点体力。",
	["@yuanhu-equip"] = "请使用技能【援护】",
	["~yuanhu"] = "选择一张装备牌→选择一名角色→点击确定",
	["$yuanhu1"] = "持吾兵戈，随我杀敌！", --武器
	["$yuanhu2"] = "汝今势微，吾当助汝。", --防具
	["$yuanhu3"] = "公急上马，洪敌贼军！", --坐骑
	["$yuanhu4"] = "天下可无洪，不可无公。", --对曹操
	["$yuanhu5"] = "持戈整兵，列阵御敌！", --对自己
	["~caohong"] = "主公已安，洪纵死亦何惜……", 
	
	["#guanyinping"] = "武姬",
	["guanyinping"] = "关银屏",
	["illustrator:guanyinping"] = "木美人",
	["designer:guanyinping"] = "韩旭",
	["cv:guanyinping"] = "蒲小猫",
	["xueji"] = "血祭",
	[":xueji"] = "出牌阶段，你可以弃置一张红色牌，对你攻击范围内的至多X名其他角色各造成1点伤害，然后这些角色各摸一张牌。X为你损失的体力值。每阶段限一次。",
	["huxiao"] = "虎啸",
	[":huxiao"] = "你于出牌阶段每使用一张【杀】被【闪】抵消，此阶段你可以额外使用一张【杀】。",
	["wuji"] = "武继",
	[":wuji"] = "<b>觉醒技</b>，回合结束阶段开始时，若本回合你已造成3点或更多伤害，你须加1点体力上限并回复1点体力，然后失去技能“虎啸”。",
	["$xueji1"] = "踏平南土，以消国恨",
	["$xueji2"] = "陷坚摧锋，以报家仇",
	["$huxiao1"] = "若无后手，何言为将？",
	["$huxiao2"] = "刀行如流水，杀你个措手不及！",
	["$huxiao3"] = "紫髯老贼！还吾父命来！",              -- for sunquan
	["$wuji"] = "武圣虽死，血脉尚存。先父佑我，再现武魂！",
	--["~guanyinping"] = "父亲……",
	
	["#chenlin"] = "破竹之咒",
	["chenlin"] = "陈琳",
	["illustrator:chenlin"] = "木美人",
	["designer:chenlin"] = "韩旭",
	["bifa"] = "笔伐",
	[":bifa"] = "回合结束阶段开始时，你可以将一张手牌背面朝下移出游戏并选择一名其他角色。该角色的回合开始时，其观看此牌并选择一项：1、交给你一张与此牌同类别的手牌，然后获得此牌。2、将此牌置入弃牌堆，然后失去1点体力。\
◆在目标角色执行【笔伐】的效果前，你不可以再次对其发动【笔伐】。\
◆目标角色在执行【笔伐】的效果前死亡，在弃置其区域里所有的牌的同时将此牌置入弃牌堆。",
	["@bifa-remove"] = "你可以发动【笔伐】",
	["~bifa"] = "选择一张手牌→选择一名其他角色→点击确定",
	["@bifa-give"] = "请交给目标角色一张类型相同的手牌",
	["songci"] = "颂词",
	[":songci"] = "出牌阶段，你可以选择一项：1、令一名手牌数小于其当前的体力值的角色摸两张牌。2、令一名手牌数大于其当前的体力值的角色弃置两张牌。每名角色每局游戏限一次。",
	["@songci"] = "颂词",
	["$bifa1"] = "文人岂无用，笔墨亦作兵。",
	["$bifa2"] = "将军可否直视此言？",                   --get
	["$bifa3"] = "行文如刀，笔墨诛心，",                 --throw
	["$songci1"] = "广宜恩信，班扬符赏。",               --drawCards
	["$songci2"] = "汝众违旅叛，当有此报！",             --discardCards
	--["~chenlin"] = "文未达意，贼不伏诛，吾之过也……",

--hulao mode
	["Hulaopass"] = "虎牢关模式",

	["#shenlvbu1"] = "最强神话",
	["shenlvbu1"] = "神吕布(1)",
	["illustrator:shenlvbu1"] = "LiuHeng",
	["#shenlvbu2"] = "暴怒的战神",
	["shenlvbu2"] = "神吕布(2)",
	["illustrator:shenlvbu2"] = "LiuHeng",
	["xiuluo"] = "修罗",
	[":xiuluo"] = "回合开始阶段，你可以弃置一张手牌来将你判定区里的一张延时类锦囊置入弃牌堆(必须花色相同)。",
	["@xiuluo"] = "请弃置一张花色相同的手牌",
	["shenwei"] = "神威",
	[":shenwei"] = "<b>锁定技</b>，摸牌阶段，你额外摸两张牌，你的手牌上限+2。",
	["shenji"] = "神戟",
	[":shenji"] = "没装备武器时，你使用的杀可指定至多3名角色为目标。",

	["#Reforming"] = "%from 进入重整状态",
	["#ReformingRecover"] = "%from 在重整状态中回复了1点体力",
	["#ReformingDraw"] = "%from 在重整状态中摸了1张牌",
	["#ReformingRevive"] = "%from 从重整状态中复活!",
	["draw_1v3"] = "重整摸牌",
	["weapon_recast"] = "武器重铸",
	["Hulaopass:recover"] = "恢复1点体力",
	["Hulaopass:draw"] = "摸1张牌",
	
	["cv:shenlvbu2"] = "风叹息",
	["$shenwei"] = "飞将之威，上天亦知！",
	["$shenji"] = "神戟在手，何人能及！",
	["$xiuluo"] = "哼！鬼蜮伎俩，休想阻我！",
	["~shenlvbu2"] = "什么？我败了？！",
	["$StageChange"] = "杂鱼们，好戏才刚刚开始！",
	
--sp_card
	["sp_cards"] = "SP卡牌包",
	["sp_moonspear"] = "SP银月枪",
	[":sp_moonspear"] = "你的回合外，每当你使用或打出一张黑色手牌时，你可以令你攻击范围内的一名其他角色打出一张【闪】，否则受到你对其造成的1点伤害",
	["@moon-spear-jink"] = "受到SP银月枪技能的影响，你必须打出一张【闪】",
}
