-- translation for WisdomPackage

local t = {
	["wisdom"] = "智包",
	["designer:wisdoms"] = "太阳神三国杀创意小组",

	["#wis_xuyou"] = "恃才傲物",
	["wis_xuyou"] = "智许攸",
	["&wis_xuyou"] = "许攸",
	["juao"] = "倨傲",
	[":juao"] = "<font color=\"green\"><b>出牌阶段限一次，</b></font>你可以将两张手牌背面向上移出游戏并选择一名角色，该角色的下个回合开始阶段开始时，须获得你移出游戏的两张牌并跳过摸牌阶段。",
	["hautain"] = "倨傲牌",
	["#JuaoObtain"] = "%from 受到了“%arg”的影响",
	["tanlan"] = "贪婪",
	[":tanlan"] = "每当你受到其他角色造成的一次伤害后，可与伤害来源拼点：若你赢，你获得双方的拼点牌。",
	["shicai"] = "恃才",
	[":shicai"] = "<b>锁定技</b>，若你向其他角色发起拼点且你拼点赢时，或其他角色向你发起拼点且拼点没赢时，你摸一张牌",

	["#wis_jiangwei"] = "天水麒麟",
	["wis_jiangwei"] = "智姜维",
	["&wis_jiangwei"] = "姜维",
	["yicai"] = "异才",
	[":yicai"] = "每当你使用一张非延时类锦囊时，你可以使用一张【杀】。",
	["beifa"] = "北伐",
	[":beifa"] = "<b>锁定技</b>，当你失去最后的手牌时，视为你对一名其他角色使用了一张【杀】，若不能如此做，则视为你对自己使用了一张【杀】。",

	["#wis_jiangwan"] = "武侯后继",
	["wis_jiangwan"] = "智蒋琬",
	["&wis_jiangwan"] = "蒋琬",
	["houyuan"] = "后援",
	[":houyuan"] = "<font color=\"green\"><b>出牌阶段限一次，</b></font>你可以弃置两张手牌并令一名其他角色摸两张牌。",
	["chouliang"] = "筹粮",
	[":chouliang"] = "回合结束阶段开始时，若你手牌少于三张，你可以从牌堆顶亮出4-X张牌（X为你的手牌数），你获得其中的基本牌，将其余的牌置入弃牌堆。",

	["#wis_sunce"] = "江东的小霸王",
	["wis_sunce"] = "智孙策",
	["&wis_sunce"] = "孙策",
	["bawang"] = "霸王",
	[":bawang"] = "每当你使用的【杀】被【闪】抵消时，你可以与目标角色拼点：若你赢，可以视为你对至多两名角色各使用了一张【杀】（此杀不计入每阶段的使用限制）。",
	["@bawang"] = "你可以发动技能“霸王”",
	["~bawang"] = "选择 1-2 名角色→点击确定",
	["weidai"] = "危殆",
	[":weidai"] = "<b>主公技</b>，当你需要使用一张【酒】时，你可以令其他吴势力角色将一张黑桃2~9的手牌置入弃牌堆，视为你将该牌当【酒】使用。",
--	["analeptic:accept"] = "打出一张黑桃2~9手牌响应【危殆】",
--	["analeptic:ignore"] = "我是打酱油的~",
	["@weidai-analeptic"] = "%src 现在需要【酒】<br/>你只要提供一张 黑桃２~９的手牌<br/>就能为 %src 提供一张【酒】",

	["#wis_zhangzhao"] = "东吴重臣",
	["wis_zhangzhao"] = "智张昭",
	["&wis_zhangzhao"] = "张昭",
	["longluo"] = "笼络",
	[":longluo"] = "回合结束阶段开始时，你可以令一名其他角色摸你于此回合弃牌阶段弃置的牌等量的牌。",
	["fuzuo"] = "辅佐",
	[":fuzuo"] = "每当其他角色拼点时，你可以弃置一张点数小于8的手牌，让其中一名角色的拼点牌的点数加上这张牌点数的二分之一（向下取整）",
	["$Fuzuo"] = "%from 的拼点牌点数视为 %arg",
	["jincui"] = "尽瘁",
	[":jincui"] = "当你死亡时，可选择一名角色，令该角色摸三张牌或者弃置三张牌。",
	["jincui:draw"] = "摸三张牌",
	["jincui:throw"] = "弃置三张牌",
	["@fuzuo-pindian"] = "你可以发动技能“辅佐”",
	["~fuzuo"] = "选择一张牌→选择一名进行拼点的角色→点击确定",

	["#wis_huaxiong"] = "心高命薄",
	["wis_huaxiong"] = "智华雄",
	["&wis_huaxiong"] = "华雄",
	["badao"] = "霸刀",
	[":badao"] = "当你成为黑色的【杀】的目标后，你可以使用一张【杀】。",
	["wenjiu"] = "温酒",
	[":wenjiu"] = "<b>锁定技</b>，你使用黑色的【杀】造成的伤害+1，你无法闪避红色的【杀】",
	["#Wenjiu1"] = "%to 受到【温酒】技能的影响，%from 对其使用的红色杀不可闪避",
	["#Wenjiu2"] = "%from 的【温酒】技能被触发，伤害从 %arg 点上升至 %arg2 点",

	["#wis_tianfeng"] = "甘冒虎口",
	["wis_tianfeng"] = "智田丰",
	["&wis_tianfeng"] = "田丰",
	["shipo"] = "识破",
	[":shipo"] = "一名角色的判定阶段开始时，你可以弃置两张牌并获得该角色判定区内的所有牌。",
	["gushou"] = "固守",
	[":gushou"] = "回合外，当你使用或打出一张基本牌时，你可以摸一张牌。",
	["yuwen"] = "狱刎",
	[":yuwen"] = "<b>锁定技</b>，当你死亡时，伤害来源为自己。",

	["#wis_shuijing"] = "水镜先生",
	["wis_shuijing"] = "智司马徽",
	["&wis_shuijing"] = "司马徽",
	["shouye"] = "授业",
	[":shouye"] = "出牌阶段，你可以弃置一张红色手牌，令至多两名其他角色各摸一张牌。“解惑”发动后，每阶段限一次。",
	["@shouye"] = "授业",
	["jiehuo"] = "解惑",
	[":jiehuo"] = "<font color=\"purple\"><b>觉醒技，</b></font>当你发动“授业”不少于7人次时，须减1点体力上限，并获得技能“师恩”（其他角色使用非延时锦囊时，可以让你摸一张牌）。",
	["#JiehuoWake"] = "%from 的觉醒技“%arg”被触发，以后你每阶段只能发动一次“%arg2”",
	["shien"] = "师恩",
	[":shien"] = "其他角色使用非延时锦囊时，可以让你摸一张牌",
	["forbid_shien"] = "是否关闭“师恩”提示",
	["forbid_shien:yes"] = "是，永久关闭（不可逆操作）",
	["forbid_shien:no"] = "不，谢谢",
	["forbid_shien:maybe"] = "是，暂时关闭（直到我下回合结束）",
	["$JiehuoAnimate"] = "image=image/animate/jiehuo.png",

	["cv:wis_xuyou"] = "庞小鸡",
	["$juao1"] = "用吾之计，敌克轻取~",
	["$juao2"] = "阿瞒，卿不得我，不得冀州也。", -- 对曹操
	["$tanlan1"] = "汝等小计，何足道哉", -- 发动拼点
	["$tanlan2"] = "匹夫尔敢如此", -- 拼点失败;拼点成功触发恃才
	["$shicai1"] = "真是不自量力！",
	["$shicai2"] = "莽夫，无可救药啊。", -- 贪婪触发
	["~wis_xuyou"] = "汝等果然是无可救药~啊~",

	["cv:wis_jiangwei"] = "Jr. Wakaran",
	["$yicai1"] = "系从尚父出，术奉武侯来。",
	["$yicai2"] = "天水麒麟儿，异才敌千军。",
	["$beifa1"] = "北伐兴蜀汉，继志越祁山",
	["$beifa2"]= "哀侯悲愤填心胸，九伐中原亦无悔。",
	["~wis_jiangwei"] = "终究还是回天乏术吗？",

	["cv:wis_jiangwan"] = "喵小林",
	["$houyuan"] = "汝等力战，吾定当稳固后方。",
	["$chouliang"] = "息民筹粮，伺机反攻。",
	["~wis_jiangwan"] = "蜀中疲敝，无力辅政矣",

	["cv:wis_sunce"] = "裤衩",
	["$bawang1"] = "匹夫，可敢与我一较高下？", -- 提出拼点
	["$bawang2"] = "虎踞鹰扬，霸王之武。", -- 胜利
	["$bawang3"] = "来日再战，吾必胜汝。", -- 失败
	["$weidai1"] = "吾之将士，借我一臂之力！", -- 回合内发动
	["$weidai2"] = "我江东子弟何在？", -- 濒死时发动
	["~wis_sunce"] = "百战之躯，竟遭暗算……",

	["cv:wis_zhangzhao"] = "喵小林",
	["$longluo1"] = "吾当助汝，共筑功勋。",
	["$longluo2"] = "江东多才俊，吾助主揽之。", -- 对孙权+孙策
	["$fuzuo1"] = "尽诚匡弼，成君之业。",
	["$fuzuo2"] = "此言，望吾主慎之重之。", -- 对孙权+孙策
	["$jincui1"] = "从吾之谏，功业可成。", -- 摸牌
	["$jincui2"] = "此人贼心昭著，当趋逐之。", -- 弃牌
	["~wis_zhangzhao"] = "尽力辅佐，吾主为何……",

	["cv:wis_huaxiong"] = "极光星逝",
	["$badao"] = "三合之内，吾必斩汝！",
	["$wenjiu1"] = "有末将在，何需温侯出马？", -- 触发黑色杀
	["$wenjiu2"] = "好快……", -- 触发红色杀（凄惨点）
	["~wis_huaxiong"] = "来将何人……啊……",

	["cv:wis_tianfeng"] = "喵小林",
	["$gushou1"] = "外结英雄，内修农战。", -- 失去闪 桃 酒
	["$gushou2"] = "奇兵迭出，扰敌疲兵。", -- 失去杀
	["$shipo1"] = "此中有诈，吾当出计破之。",
	["$shipo2"] = "休要蒙蔽我主！", -- 对主公袁绍
	["$yuwen"] = "吾当自死，不劳明公动手。",
	["~wis_tianfeng"] = "不识其主，虽死何惜。",

	["cv:wis_shuijing"] = "喵小林",
	["$shouye1"] = "授汝等之策，自可平定天下。", -- 用红桃
	["$shouye2"] = "为天下太平，还望汝等尽力。", -- 用方片
	["$jiehuo"] = "桃李满天下，吾可归隐矣。",
	["$shien1"] = "吾师教诲，终身不忘。",
	["$shien2"] = "龙凤之才，全赖吾师。", -- 龙凤发动师恩
	["~wis_shuijing"] = "儒生俗士，终究难平天下吗？",
	
	
	["illustrator:wis_xuyou"] = "三国志大战",
	["illustrator:wis_jiangwei"] = "巴萨小马",
	["illustrator:wis_jiangwan"] = "Zero",
	["illustrator:wis_sunce"] = "永恒之轮",
	["illustrator:wis_zhangzhao"] = "三国志大战",
	["illustrator:wis_huaxiong"] = "三国志大战",
	["illustrator:wis_tianfeng"] = "小矮米",
	["illustrator:wis_shuijing"] = "小仓",
}

local generals = {"wis_xuyou", "wis_jiangwei", "wis_jiangwan", "wis_sunce", "wis_zhangzhao", "wis_huaxiong", "wis_tianfeng", "wis_shuijing"}

for _, general in ipairs(generals) do
	t["designer:" .. general] = t["designer:wisdoms"]
end

return t
