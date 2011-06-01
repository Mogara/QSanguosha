--技能详解1：ViewAsSkill

--在太阳神三国杀中，基本技能可分为三类：触发技，视为技和系统技。

--触发技可以用来实现”在某个阶段、满足发动条件时，执行某个效果（包括做出选择）这样的技能。
--触发技也可以用来改变游戏事件而不仅是单纯的产生效果。比如，放弃摸牌阶段并执行xx这样的技能也可以用触发技实现。

--视为技可以用来实现“可将某牌作为某牌打出”这样的技能。
--视为技的定义对于AI而言是无效的。为了让AI使用视为技，你基本上需要在AI中重新写一遍技能的定义。
--视为技体系的这一个缺陷是现在很多AI无视规则的根本原因。

--我们可以将触发技和视为技组合来制作技能。在触发技中定义一个视为技，可以让视为技在触发技描述的事件中发动。
--大多数的复杂技能都需要通过组合触发技和视为技来实现。

--系统技即任何无法用以上两种技能实现或者组合来描述的技能。比如，完杀、咆哮和马术即是这样的技能。
--这类技能通常改变了游戏基本规则，具有“玩家无法做出某种选择”、“玩家可以做出某种选择”的效果，
--或者使用了“计算距离时”这样的未定义时机。迄今为止，这样的技能都是靠直接包含到规则里面来实现的。

--因此，具有上述特点的技能，需要用系统技实现，是暂时无法DIY的。
--对于“玩家可以做出某种选择”这样的技能，比如咆哮，建议使用“你可以将一张杀作为一张杀打出”这样的实现。
--“玩家无法做出某种选择”这样的技能，请尽量避免。注意，无效化类的技能是完全可以的。

--
--正片开始



--首先讲解视为技。
--视为技在Lua中的创建使用了sgs.CreateViewAsSkill方法，该方法可以在lua\sgs_ex.lua中找到。
--视为技在创建时，需要以下方法|变量的定义：

--name,	n, view_filter, view_as以及可选的enabled_at_play和enabled_at_response

--name为一个字符串即技能名称。

--n为每次发动技能所用牌数的最大值（若无上限请设为998，只要998）。绝大多数DIY用到的n可能都为1或2.


--view_filter为某张卡是否可被选中以用作发动技能。发动技能时，将对所有手牌、装备进行遍历，并执行view_filter方法。返回了true的牌可以被选择用作技能发动。

--以下为“任意一张草花牌”的view_filter方法：

n=1,

view_filter = function(self, to_select, selected)
	local condition=(to_select:getSuit()==sgs.Card_Club)
	return condition
end,

--to_select为正在遍历的牌的对象引用。如果他的花色为草花Club，则返回真（可被选择）。否则返回假（不可被选择）。

--以下为“任意两张同花色手牌“的view_filter方法：

n=2,

view_filter = function(self, to_select, selected)
	if #selected<1 then return not to_select:isEquipped() end
	local condition=(to_select:getSuit()==selected[1]:getSuit())
	return condition and not to_select:isEquipped()
end,

--selected为一个lua table，包含已经选中的所有牌的引用。这里，如果选中的牌数小于1，那么任何牌都可以被选择；如果选中的牌数不小于1，那么只有那些和已被选中的第一张牌花色相同的牌才能被选中。


--view_as用作产生最终得到的，”被视为打出“的牌的对象。这里的牌可以是游戏牌，也可以是技能牌。如果你的DIY技能的效果不是某张游戏牌的效果，那么你需要把该效果定义到一个技能牌当中，然后在view_as方法中得到并返回一张你定义的技能牌。

--以下为”当成借刀杀人使用“的view_as方法部分：

n=1,

view_as = function(self, cards)

	local invalid_condition=(#cards<1)
	if invalid_condition then return nil end
	
	local suit,number
	
	for _,card in ipairs(cards) do
		if suit and (suit~=card:getSuit()) then suit=sgs.Card_NoSuit else suit=card:getSuit() end
		if number and (number~=card:getNumber()) then number=-1 else number=card:getNumber() end
	end
	
	local view_as_card= sgs.Sanguosha:cloneCard("collateral", suit, number)
	
	for _,card in ipairs(cards) do
		view_as_card:addSubcard(card:getId())
	end
	
	view_as_card:setSkillName(self:objectName())
	
	return view_as_card
end,

--若选中的牌数目小于1，则返回空对象表示没有效果牌可以产生。
--否则，记录牌的花色与数值。若被选中的牌当中花色或数值有不同，则为无花色或无数字。

--记录之后，生成借刀杀人牌。该牌的花色数字与之前记录的相同。
--将使用到的牌加入生成的借刀杀人的子牌列表中。这一步通常是必须的，因为可能存在对牌进行互动的技能（比如乱击与奸雄）
--将技能的名字即self:objectName赋予该卡的技能名属性。

--需要其他游戏牌时，改动collateral这个名字即可。需要根据不同种类、数量的牌得到不同游戏牌时，改动invalid_condition与后面的cloneCard即可。

--当技能的效果不能简单地描述为”视为你打出了xx牌“时，你需要使用技能牌定义技能的效果。也就是说，将你的技能转述为”你可以将你的xx牌作为xx牌打出，其中xx牌的效果为blahblah“。然后，在技能牌的定义中实现技能的效果。

--也就是说，技能的效果用”技能牌“实现，技能的发动约束用”视为技“实现，技能的发动时机控制用”触发技“实现。

--以下是貂蝉离间技能的view_as方法：

view_as = function(self, cards)

	local invalid_condition=(#cards<1)
	if invalid_condition then return nil end
	
	local view_as_card=sgs.CreateSkillCard( lijianCard )
	
	for _,card in ipairs(cards) do
		view_as_card:addSubcard(card:getId())
	end
	
	return view_as_card
end,

--其中lijianCard的定义应该被包含在同一个module文件当中。我将在其他文档中讲解技能牌的定义。