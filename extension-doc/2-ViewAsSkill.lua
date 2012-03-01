--技能详解1：ViewAsSkill（视为技，也叫视作技）

--在太阳神三国杀中，常用的基本技能是两种：触发技和视为技。我们使用这两种基本技能的复合来完成大多数复杂的技能。
--此外，我们还有距离技，禁止技等特殊类型的技能，它们用于实现某些某种程度上 “要求改变游戏系统” 的技能。

--触发技可以用来实现”在某个时机，满足发动条件时，执行某个效果（包括做出选择）这样的技能。
--触发技也可以用来改变游戏事件而不仅是单纯的产生效果。比如，放弃摸牌阶段并执行xx这样的技能也可以用触发技实现。
--事实上，一个触发技就是一条游戏规则。如果某名武将具有某个触发技，这个触发技就会被在服务端“注册”成为这局游戏的一条规则。
--被注册的触发技，通常会根据玩家“是否存活”以及玩家的武将“是否具有该技能”来决定是否被执行。

--视为技可以用来实现“可将某牌作为某牌打出”这样的技能。
--视为技的定义对于AI而言是无效的。为了让AI使用视为技，你基本上需要在AI中重新写一遍技能的定义。
--这是因为视为技是在客户端运行的，而AI在服务器端运行；触发技也是在服务端运行。
--如果某名玩家具有某个视作技，该技能通常就会被”注册“到该玩家的客户端。这个动作与其他玩家是不相干的。
--总之，视为技负责在客户端管理，有哪些牌可以选中，选中的牌又会被当成什么牌，这样的效果；
--但是视为技的运行本身不影响游戏进程；视为技做的事情是通过将特定的牌（甚至没有牌）“视为”特定的牌，
--来允许你使用那张本来不存在的牌。这也是大多数”主动技能“的实现方式。

--距离技和禁止技则分别用于实现“修改玩家间的距离”，以及“某人不能对某人使用某牌”这样的效果。
--与其他两种技能不同，距离技和禁止技不需要在某局游戏中注册：距离技是永远生效。
--也就是说，如果你写了一个距离技能叫”所有玩家间的距离-1“，那么无论场上有没有玩家具有这个技能，
--所有玩家间的距离都会被减一。禁止技也类似。
--这类技能的定义比较新，也比较清楚明了，可能你不需要看文档也能明白。

--正片开始



--首先讲解视为技。
--视为技在Lua中的创建使用了sgs.CreateViewAsSkill方法

-- **程序细节，以下可跳过
 
--sgs.CreateViewAsSkill方法可以在lua\sgs_ex.lua中找到。
--该方法会创建一个LuaViewAsSkill对象（定义见cpp源码），然后将你定义的Lua函数用于其成员函数。
--注意，本游戏cpp源码部分使用了类的继承重载来实现不同的技能，而在lua的DIY技能里面我们都使用固定的类
--我们通过将会被用作技能成员函数的lua function，直接赋给技能的实例来实现不同的技能。

-- **程序细节部分结束

--视为技在创建时，需要以下方法|变量的定义：

--name,	n, view_filter, view_as, enabled_at_play和enabled_at_response

--name：
--一个字符串即技能名称。
--该字段必须定义。（无默认值）

--n：
--整数值，每次发动技能所用牌数的最大值。绝大多数DIY用到的n可能都为1或2.
--默认值为0，即不需要选择自己的任何牌.

--view_filter：
--lua函数，返回一个布尔值，即某张卡是否可被选中以用作发动技能。
--发动技能时，将对所有手牌、装备进行遍历，并执行view_filter方法。返回了true的牌可以被选择用作技能发动。
--传入的参数为self(技能对象本身),selected(lua表，已经选择的所有牌), to_select(当前需判断是否可选中的牌)
--默认为"永远返回false"，即如果你没有定义，则这个技能的发动不允许你选择任何牌。

--view_as：
--lua函数，返回一个card对象，即被选中的牌应当被视为什么牌被打出或使用。
--这里的牌可以是游戏牌，也可以是技能牌。
--如果你的DIY主动技能的效果不是某张游戏牌的效果，那么你需要把该效果定义到一个技能牌当中
--然后在view_as方法中得到并返回一张你定义的技能牌。
--传入参数为self(技能对象本身),cards(lua表，已经选择的所有牌)
--默认为"返回空对象"，即如果你没有定义，那么这个技能的发动永远不允许你点确定。

--enabled_at_play
--lua函数，返回一个布尔值，即你在出牌阶段是否可以使用该技能。（该按钮是否可点）
--传入参数为self(技能对象本身)，player(玩家对象)
--默认为true，即如果你没有定义，你永远可以在出牌阶段使用本技能。

--enabled_at_response
--lua函数，返回一个布尔值，即你在需要用响应时，是否可以使用本技能进行响应。
--传入参数为self(技能对象本身)，player(玩家对象),pattern(要求响应的牌的匹配规则)
--默认为false,即如果你没有定义，你永远不可以在响应时使用本技能。

--enabled_at_nullification
--lua函数，返回一个布尔值，即你在询问无懈可击时是否可以使用该技能。（该按钮是否可点）
--传入参数为self(技能对象本身)，player(玩家对象)
--默认为false，即如果你没有定义，你永远不可以在出牌阶段使用本技能。

--** 实例

--以下为“任意一张草花牌”的view_filter方法：

n=1,

view_filter = function(self, selected, to_select)
	return to_select:getSuit()==sgs.Card_Club
end,

--getSuit()返回一张牌的花色的enum。
--如果to_select的花色为草花Club，则返回真（可被选择）。否则返回假（不可被选择）。

--以下为“任意两张同花色手牌“的view_filter方法：

n=2,

view_filter = function(self, selected, to_select)
	if #selected<1 then return not to_select:isEquipped() end
	return to_select:getSuit()==selected[1]:getSuit() and not to_select:isEquipped()
end,

--如果选中的牌数小于1，那么任何未被装备的牌（手牌）都可以被选择；
--否则，只有那些和已被选中的第一张牌花色相同的牌才能被选中。

--以下为”当成借刀杀人使用“的view_as方法部分：

n=1,

view_as = function(self, cards)

	
	if #cards<1 then return nil end 
	--如果没有牌被选中，那么返回空对象
	
	local suit,number
	
	for _,card in ipairs(cards) do
		if suit and (suit~=card:getSuit()) then suit=sgs.Card_NoSuit else suit=card:getSuit() end
		if number and (number~=card:getNumber()) then number=-1 else number=card:getNumber() end
	end
	
	--如果所有被选中的牌的花色一样，那么被视为的借刀杀人的花色也为该花色。否则，该借刀杀人无花色。
	--点数同理。
	
	local view_as_card= sgs.Sanguosha:cloneCard("collateral", suit, number)
	--生成一张借刀杀人
	
	for _,card in ipairs(cards) do
		view_as_card:addSubcard(card:getId())
	end
	--将被用作视为借刀杀人的牌都加入到借刀杀人的subcards里
	
	view_as_card:setSkillName(self:objectName())
	--标记该借刀杀人是由本技能生成的
	
	return view_as_card
end,

--将使用到的牌加入生成的借刀杀人的子牌列表中这一步，通常是必须的。
--因为可能存在对牌进行互动的技能，比如发动奸雄会得到发动乱击用的两张牌。

--将技能的名字即self:objectName赋予该卡的技能名属性，还是用于标记方便。

--需要其他游戏牌时，改动collateral这个名字即可。需要根据不同种类、数量的牌得到不同游戏牌时，改动if #cards<1 then 这句即可。
--当技能的效果不能简单地描述为”视为你打出了xx牌“时，你需要使用技能牌定义技能的效果。
--也就是说，将你的技能转述为”你可以将你的xx牌作为xx牌打出，其中xx牌的效果为blahblah“。然后，在技能牌的定义中实现技能的效果。
--技能的效果用”技能牌“实现，技能的发动约束用”视为技“实现，技能的发动时机控制用”触发技“实现。

--以下是貂蝉离间技能的view_as方法：

view_as = function(self, cards)

	if #cards<1 then return nil end
	
	local view_as_card=lijian_Card:clone()
	--lijian_Card的定义应该被包含在同一个module文件当中。我将在其他文档中讲解技能牌的定义。
	
	for _,card in ipairs(cards) do
		view_as_card:addSubcard(card:getId())
	end
	
	return view_as_card
end,

--以下是周瑜“反间”的enable_at_play方法：

enabled_at_play=function(self,player)
		return not player:hasFlag("fanjian_used")
		--反间只能每回合用一次。如果你已经使用过了，那么你不能再次使用。
end
		
--注意在lua中暂时不能使用player:hasUsed方法来判断是否用过你的某个LuaSkillCard
--因为LuaSkillCard并不使用在cpp里面用的类继承机制
--请暂时在你的LuaSkillCard中执行类似于room:setPlayerFlag(effect.from,"fanjian_used")
--这样的语句，然后在enabled_at_play里面使用player:hasFlag。

--以下是大乔”流离“的enabled_at_response方法：
enabled_at_response=function()
	return sgs.Self:getPattern()=="#liuli_effect"
	--仅在询问流离时可以发动此技能
	
	--这里的函数没有参数传入（我们不需要）
	--我们直接调用了sgs.Self，这个对象就是本地的ClientPlayer对象。
	--注意：只有在本地运行的代码（比如视作技）可以调用到sgs.Self。
end

--虽然看起来流离并不像是一个视作技
--但事实上我们把它分成了这样的三部分：
--1、你成为杀的目标时，你可以响应使用"流离牌"（技能牌）。用触发技实现。
--2、在且仅在你响应使用"流离牌"时，你可以将任意一张牌视为"流离牌"使用。用视为技实现。
--3、流离牌的效果为：流离牌的目标成为杀的目标，该杀跳过对你的结算。用技能牌实现。

--注意到正常情况下pattern永远不会自己就是"#liuli_effect"
--你需要在触发技当中使用room:askForUseCard(daqiao,"#liuli_target",prompt)
--这样，就可以创造出一个专门用于流离的响应来。