-- translation for ManeuveringPackage

return {
	["maneuvering"] = "军争篇",

	["fire_slash"] = "火杀",
	[":fire_slash"] = "基本牌<br />出牌时机：出牌阶段<br />使用目标：攻击范围内的一名其他角色<br />作用效果：对目标角色造成1点火焰伤害",

	["thunder_slash"] = "雷杀",
	[":thunder_slash"] = "基本牌<br />出牌时机：出牌阶段<br />使用目标：攻击范围内的一名其他角色<br />作用效果：对目标角色造成1点雷电伤害",

	["analeptic"] = "酒",
	[":analeptic"] = "基本牌<br />出牌时机：1、出牌阶段；2、当自己进入濒死阶段时<br />使用目标：1、包括你的一名角色 2、你<br />作用效果：1、令自己本阶段使用的下一张【杀】将要造成的伤害+1（每阶段限一次）；2、回复1点体力",
	["#UnsetDrankEndOfTurn"] = "%from 的出牌阶段结束，【<font color=\"yellow\"><b>酒</b></font>】的效果消失",

	["Fan"] = "朱雀羽扇",
	[":Fan"] = "装备牌·武器<br />攻击范围：４<br />武器特效：你可以将一张普通【杀】当火【杀】使用",

	["GudingBlade"] = "古锭刀",
	[":GudingBlade"] = "装备牌·武器<br />攻击范围：２<br />武器特效：<font color=\"blue\"><b>锁定技，</b></font>每当你使用【杀】对目标角色造成伤害时，若该角色没有手牌，此伤害+1",
	["#GudingBladeEffect"] = "%from 的【<font color=\"yellow\"><b>古锭刀</b></font>】效果被触发， %to 没有手牌，伤害从 %arg 增加至 %arg2",

	["Vine"] = "藤甲",
	[":Vine"] = "装备牌·防具<br />防具效果：<font color=\"blue\"><b>锁定技，</b></font>【南蛮入侵】、【万箭齐发】和普通【杀】对你无效。每当你受到火焰伤害时，此伤害+1",
	["#VineDamage"] = "%from 的防具【<font color=\"yellow\"><b>藤甲</b></font>】效果被触发，火焰伤害由 %arg 点增加至 %arg2 点",

	["SilverLion"] = "白银狮子",
	[":SilverLion"] = "装备牌·防具<br />防具效果：<font color=\"blue\"><b>锁定技，</b></font>每当你受到伤害时，若该伤害多于1点，你防止多余的伤害。每当你失去装备区里的【白银狮子】后，你回复1点体力",
	["#SilverLion"] = "%from 的防具【%arg2】防止了 %arg 点伤害，减至 <font color=\"yellow\"><b>1</b></font> 点",

	["fire_attack"] = "火攻",
	[":fire_attack"] = "锦囊牌<br />出牌时机：出牌阶段<br />使用目标：一名有手牌的角色。<br />作用效果：目标角色展示一张手牌。若你弃置一张与所展示牌相同花色的手牌，该角色受到1点火焰伤害",
	["fire-attack-card"] = "您可以弃置一张与 %dest 所展示卡牌相同花色(%arg)的牌对 %dest 造成1点火焰伤害",
	["@fire-attack"] = "%src 展示的牌的花色为 %arg，请弃置一张与其相同花色的手牌",

	["iron_chain"] = "铁索连环",
	[":iron_chain"] = "锦囊牌<br />出牌时机：出牌阶段。<br />使用目标：一至两名角色<br />作用效果：横置或重置目标角色的武将牌，被横置的角色处于“连环状态” <br />重铸：将此牌置入弃牌堆，然后摸一张牌",

	["supply_shortage"] = "兵粮寸断",
	[":supply_shortage"] = "延时锦囊牌<br />出牌时机：出牌阶段<br />使用目标：距离1的一名其他角色。<br />作用效果：将【兵粮寸断】置于目标角色判定区内。目标角色下个判定阶段进行一次判定；若判定结果不为♣，则跳过其摸牌阶段。然后将【兵粮寸断】置入弃牌堆。",

	["HuaLiu"] = "骅骝",
}
