-- translation for StandardPackage

local t = {
	["standard_cards"] = "标准版",

	["slash"] = "杀",
	[":slash"] = "基本牌<br />出牌时机：出牌阶段<br />使用目标：攻击范围内的一名其他角色<br />作用效果：对目标角色造成1点伤害",
	["slash-jink"] = "%src 使用了【杀】，请使用一张【闪】",
	["@multi-jink-start"] = "%src 使用了【杀】，你须连续使用 %arg 张【闪】",
	["@multi-jink"] = "%src 使用了【杀】，你须再使用 %arg 张【闪】",
	["@slash_extra_targets"] = "请选择此【杀】的额外目标",

	["jink"] = "闪",
	[":jink"] = "基本牌<br />出牌时机：以你为目标的【杀】结算时<br />使用目标：以你为目标的【杀】<br />作用效果：抵消目标【杀】的效果",
	["#NoJink"] = "%from 不能使用【<font color=\"yellow\"><b>闪</b></font>】响应此【<font color=\"yellow\"><b>杀</b></font>】",

	["peach"] = "桃",
	[":peach"] = "基本牌<br />出牌时机：1、出牌阶段；2、有角色处于濒死状态时<br />使用目标：1、包括你的一名已受伤角色；2、处于濒死状态的一名角色<br />作用效果：目标角色回复1点体力",

	["Crossbow"] = "诸葛连弩",
	[":Crossbow"] = "装备牌·武器<br />攻击范围：１<br />武器特效：<font color=\"blue\"><b>锁定技，</b></font>你于出牌阶段内使用【杀】无数量限制。",

	["DoubleSword"] = "雌雄双股剑",
	[":DoubleSword"] = "装备牌·武器<br />攻击范围：２<br />武器特效：每当你使用【杀】指定一名异性角色为目标后，你可以令其选择一项：弃置一张手牌，或令你摸一张牌。",
	["double-sword-card"] = "%src 发动了【雌雄双股剑】效果，你须弃置一张手牌，或令 %src 摸一张牌",

	["QinggangSword"] = "青釭剑",
	[":QinggangSword"] = "装备牌·武器<br />攻击范围：２<br />武器特效：<font color=\"blue\"><b>锁定技，</b></font>每当你使用【杀】指定一名角色为目标后，你无视其防具，直到此【杀】被【闪】抵消后或造成伤害扣减体力前。",

	["Blade"] = "青龙偃月刀",
	[":Blade"] = "装备牌·武器<br />攻击范围：３<br />武器特效：每当你使用的【杀】被【闪】抵消后，你可以对该角色再使用一张【杀】。",
	["blade-slash"] = "你可以发动【青龙偃月刀】再对 %src 使用一张【杀】",
	["#BladeUse"] = "%from 对 %to 发动了【<font color=\"yellow\"><b>青龙偃月刀</b></font>】效果",

	["Spear"] = "丈八蛇矛",
	[":Spear"] = "装备牌·武器<br />攻击范围：３<br />武器特效：你可以将两张手牌当【杀】使用或打出",

	["Axe"] = "贯石斧",
	[":Axe"] = "装备牌·武器<br />攻击范围：３<br />武器特效：每当你使用的【杀】被【闪】抵消后，你可以弃置两张牌，则此【杀】继续造成伤害。",
	["@Axe"] = "你可以弃置两张牌令此【杀】继续造成伤害",
	["~Axe"] = "选择两张牌→点击确定",

	["Halberd"] = "方天画戟",
	[":Halberd"] = "装备牌·武器<br />攻击范围：４<br />武器特效：<font color=\"blue\"><b>锁定技，</b></font>你使用最后的手牌【杀】的目标数上限+2。",

	["KylinBow"] = "麒麟弓",
	[":KylinBow"] = "装备牌·武器<br />攻击范围：５<br />武器特效：每当你使用【杀】对目标角色造成伤害时，你可以弃置其装备区内的一张坐骑牌。",
	["KylinBow:dhorse"] = "+1坐骑",
	["KylinBow:ohorse"] = "-1坐骑",

	["EightDiagram"] = "八卦阵",
	[":EightDiagram"] = "装备牌·防具<br />防具效果：每当你需要使用或打出一张【闪】时，你可以进行一次判定：若判定结果为红色，视为你使用或打出了一张【闪】。",

	["standard_ex_cards"] = "标准版EX",

	["RenwangShield"] = "仁王盾",
	[":RenwangShield"] = "装备牌·防具<br />防具效果：<font color=\"blue\"><b>锁定技，</b></font>黑色【杀】对你无效",

	["IceSword"] = "寒冰剑",
	[":IceSword"] = "装备牌·武器<br />攻击范围：２<br />武器特效：每当你使用【杀】对目标角色造成伤害时，若该角色有牌，你可以防止此伤害，然后依次弃置其两张牌。",

	["Horse"] = "坐骑",
	[":+1 horse"] = "装备牌·坐骑<br />坐骑效果：其他角色与你的距离+1。",
	["JueYing"] = "绝影",
	["DiLu"] = "的卢",
	["ZhuaHuangFeiDian"] = "爪黄飞电",
	[":-1 horse"] = "装备牌·坐骑<br />坐骑效果：你与其他角色的距离-1。",
	["ChiTu"] = "赤兔",
	["DaYuan"] = "大宛",
	["ZiXing"] = "紫骍",

	["amazing_grace"] = "五谷丰登",
	[":amazing_grace"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：所有角色。<br />作用效果：你展示牌堆顶等于现存角色数量的牌，每名目标角色获得其中一张牌，然后将其余的牌置入弃牌堆。",

	["god_salvation"] = "桃园结义",
	[":god_salvation"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：所有角色。<br />作用效果：每名目标角色回复1点体力",

	["savage_assault"] = "南蛮入侵",
	[":savage_assault"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：所有其他角色。<br />作用效果：每名目标角色须打出一张【杀】，否则受到1点伤害",
	["savage-assault-slash"] = "%src 使用了【南蛮入侵】，请打出一张【杀】来响应",

	["archery_attack"] = "万箭齐发",
	[":archery_attack"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：所有其他角色。<br />作用效果：每名目标角色须打出一张【闪】，否则受到1点伤害",
	["archery-attack-jink"] = "%src 使用了【万箭齐发】，请打出一张【闪】以响应",

	["collateral"] = "借刀杀人",
	[":collateral"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：装备区内有武器牌的且攻击范围内有另一名角色的一名其他角色A。（你需要选择另一名在A攻击范围内的角色B）。<br />作用效果：A须对B使用一张【杀】，否则你获得A装备区内的武器牌。",
	["collateral-slash"] = "%dest 使用了【借刀杀人】，请对 %src 使用一张【杀】",
	["#CollateralSlash"] = "%from 选择了此【<font color=\"yellow\"><b>杀</b></font>】的目标 %to",

	["duel"] = "决斗",
	[":duel"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：一名其他角色<br />作用效果：由目标角色开始，你与其轮流打出一张【杀】，首先不打出【杀】的一方受到对方造成的1点伤害。",
	["duel-slash"] = "%src 对你【决斗】，你需要打出一张【杀】",

	["ex_nihilo"] = "无中生有",
	[":ex_nihilo"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：包括你的一名角色<br />作用效果：摸两张牌。",

	["snatch"] = "顺手牵羊",
	[":snatch"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：距离1的一名角色<br />作用效果：你获得其区域里的一张牌",

	["dismantlement"] = "过河拆桥",
	[":dismantlement"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：一名其他角色。<br />作用效果：你弃置其区域里的一张牌",

	["nullification"] = "无懈可击",
	[":nullification"] = "锦囊牌<br />出牌时机：目标锦囊对目标角色生效前<br />使用目标：目标锦囊。<br />作用效果：抵消目标锦囊牌对一名角色产生的效果，或抵消另一张【无懈可击】产生的效果。",

	["indulgence"] = "乐不思蜀",
	[":indulgence"] = "延时锦囊牌<br />出牌时机：出牌阶段<br />使用目标：一名其他角色。<br />作用效果：将【乐不思蜀】置于目标角色判定区内。目标角色下个判定阶段进行一次判定；若判定结果不为<font color=\"red\">♥</font>，则跳过其出牌阶段。然后将【乐不思蜀】置入弃牌堆。",

	["lightning"] = "闪电",
	[":lightning"] = "延时锦囊牌<br />出牌时机：出牌阶段<br />使用目标：你<br />作用效果：将【闪电】置于你判定区内，目标角色下个判定阶段进行一次判定；若判定结果为♠2-9，则该角色受到3点雷电伤害，将闪电置入弃牌堆，否则将【闪电】移动到当前目标角色下家判定区内。",
}

local ohorses = { "ChiTu", "DaYuan", "ZiXing" }
local dhorses = { "ZhuaHuangFeiDian", "DiLu", "JueYing", "HuaLiu" }

for _, horse in ipairs(ohorses) do
	t[":" .. horse] = t[":-1 horse"]
end

for _, horse in ipairs(dhorses) do
	t[":" .. horse] = t[":+1 horse"]
end

return t