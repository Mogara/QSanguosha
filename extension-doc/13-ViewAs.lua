--[[
下面介绍一下视为技的 AI 写法。事实上在这个文档里只会介绍视为技 AI 的一小部分。
对于与技能卡相关的视为技，以及出牌阶段的主动视为技，将在后续部分介绍。

之所以把这部分的视为技 AI 放到前面来讲，是因为这部分的 AI 比较简单。
基本上只是把技能换种方式重新写一遍，不涉及任何决策的问题。

需要把技能在 AI 里头重新写一遍，是因为视为技的代码在客户端执行，而 AI 则是在服务器端执行的。

这部分里介绍的内容与 SmartAI:getCard... 的一系列函数有关，具体包括哪些函数见 12-SmartAI.lua。

视为技是广义的说法，实际上包括所谓的“锁定视为技” FilterSkill，与一般视为技 ViewAsSkill。
下面举例说明一下一些相关的表与其元素。
* sgs.ai_filterskill_filter：与锁定视为技有关的表。例子如 god-ai.lua 第 32 至 37 行关于武神的代码：]]
sgs.ai_filterskill_filter.wushen = function(card, card_place) -- 武神技能的锁定视为技
	local suit = card:getSuitString() -- 获得卡牌花色
	local number = card:getNumberString() -- 获得卡牌点数
	local card_id = card:getEffectiveId() -- 获得卡牌的有效 ID
	if card:getSuit() == sgs.Card_Heart then return ("slash:wushen[%s:%s]=%d"):format(suit, number, card_id) end
	-- 如果卡牌的花色为红桃，则返回武神杀，花色与点数不变。
end
--[[
从上面的例子可以看到，我们需要对这个表进行的操作是给表里的一个以技能名命名的元素赋值。
所赋的值为一个函数，函数原型为 function(card, card_place)
其中 card 表示需要处理的卡牌，而 card_place 表示卡牌所处的位置。
card_place 为 sgs.Player_Hand 或 sgs.Player_Equip，分别表示手牌与装备区。
返回值则为一个将会传递给 sgs.Card_Parse 函数的字符串。该字符串经 sgs.Card_Parse 函数处理之后得到实际的卡牌。

因此，在这里有必要介绍两个重要的函数。
* sgs.Card_Parse，这个函数实际上就是源码里头的 Card::Parse，这一函数的原型如下：
static const Card* Card::Parse(const QString &str);
对于 AI 编写来说，只要知道这个函数的传入参数是一个字符串，而返回值是相应的卡牌就可以了。
在 AI 编写中遇到的传入字符串主要有以下几种类型：

. 一个整数 n，得到的卡牌是 ID 为 n 的卡牌。例如 sgs.Card_Parse("0") 和 sgs.Card_Parse(0) 都得到黑桃 7 的【杀】。
后一种情况下，0 被 Lua 自动转型为字符串。 

. 形如 "%object_name:%skill_name[%suit:%number]=%ids" 的字符串，注意整个字符串中没有多余的空格。
返回一张虚拟卡，其对象名（objectName）为 %object_name，技能名为 %skill_name，
花色由 %suit （如 "spade", "no_suit"）确定，点数由 %number 确定（如 "0", "A", "10", "K"）
%ids 是一串用加号连接起来的 ID，表示该虚拟卡的全部子卡的 ID，也可以为 "."，表示该卡没有子卡。
例如 ]]
sgs.Card_Parse("archery_attack:luanji[diamond:K]=29+28")
--[[将得到一张万箭齐发的虚拟卡。
该卡为方片 K，技能名为乱击，包含两张子卡，ID 分别为 28 （方片 10 的【杀】）与29（方片 K 的【杀】）。

. 形如 "@%class_name=%ids" 的字符串，同样在整个字符串中没有多余的空格。
返回一张技能卡，其类名为 %class_name，%ids 的含义与前面相同。
例如]]
sgs.Card_Parse("@RendeCard=0") --[[得到一张仁德技能卡，其子卡为黑桃 7 的【杀】。

. 对于 Lua 技能卡，相应的字符串应该替换为 "#%object_name:%ids"。
%object_name 为 Lua 技能卡的对象名，即 4-SkillCard.lua 第 16 行提到的 name 参数。
例如在已经定义了 5-Applications.lua 中提到的离间技能卡的前提下，]]
sgs.Card_Parse("#liuli_effect:0")
--[[将得到一张 Lua 流离技能卡，包括一张子卡，子卡为黑桃 7 的【杀】。

事实上，不仅仅是编写 AI，在编写 Lua 扩展时恰当运用 sgs.Card_Parse，能使代码更为精简。

* string:format(...)：字符串的格式化。
这个函数的功能是将给定的字符串中的参数占位符用相应的参数依次代入得到新的字符串。
所谓参数占位符，是指一些特殊的字符串，在 AI 中用到的主要有两种："%s" 和 "%d"。
"%s" 表示字符串类型的参数，"%d" 表示数值（number）类型的参数。
上面说的还是比较抽象，下面看几个例子：]]
("%d + %d = %d"):format(2, 5 ,7) == "2 + 5 = 7" --注意，写成"%d + %d = %d":format(2, 5, 7)会出错。
("%d + %s = %s"):format(2, "5", "7") == "2 + 5 = 7"
("archery_attack:luanji[%s:%s]=%d+%d"):format("diamond", "K", 29, 28) ==
"archery_attack:luanji[diamond:K]=29+28"

--[[介绍完这两个重要的函数之后，让我们回到与视为技有关的表。

* sgs.ai_view_as：与一般视为技有关的表。
这个表与 sgs.ai_filterskill_filter 十分相似。其元素名也是技能名，元素也是函数。
例子可见 standard-ai.lua 197 至 204 行关于倾国的代码。]]
sgs.ai_view_as.qingguo = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:isBlack() and card_place ~= sgs.Player_Equip then --如果是黑色牌且不在装备区
		return ("jink:qingguo[%s:%s]=%d"):format(suit, number, card_id)
	end
end
--[[与 ai_filterskill_filter 所不同的是，这里多出了一个参数 player，这是指拥有该技能的玩家。
这个参数的作用可以见 standard-ai.lua 中 1109 至 1116 行关于急救的代码。]]
sgs.ai_view_as.jijiu = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:isRed() and player:getPhase()==sgs.Player_NotActive then --是红色牌，且在华佗的回合外。
		return ("peach:jijiu[%s:%s]=%d"):format(suit, number, card_id)
	end
end
