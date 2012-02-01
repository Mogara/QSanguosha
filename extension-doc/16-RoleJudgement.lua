--[[
在前面五个文档中，您已经学习了在开启身份预知的情况下，让 AI 按照您的要求去工作的所有基本知识了。
为了真正熟悉 AI 的编写，接下来您需要做的只是不断地模仿和练习。
下面你可以根据自己的情况作出选择：
+ 如果您对身份判断不感兴趣，或者认为您添加的技能和卡牌对身份判断影响很小。
您可以直接跳到 17-Example.lua，并且仅仅阅读其中与身份判断无关的部分。
+ 如果您希望进一步了解身份判断部分的 AI，欢迎继续阅读本文档。

本文档将集中介绍 smart-ai.lua 中第二部分“身份、动机、仇恨值”的内容。
首先需要树立一个概念，在一局游戏中，所有的 AI 共享同一套身份判断有关的数据。
而并不是像一些人想象的那样，每个 AI 有自己的身份判断数据。
另外，下面介绍的内容均以 8 人身份局为例。
对于某些很特殊的情况（例如国战），此处不作介绍。

++ 与 AI 身份判断有关的表
（注意，它们都是 sgs 而不是 SmartAI 的元素，这就印证了上面的说法）
下面这些表的元素名称，如果没有特别说明，都是 ServerPlayer 对象的对象名，即形如 "sgsX" 的字符串。
* sgs.ai_loyalty：表，包含忠诚度
% 元素：数值，为元素名称所对应的玩家的忠诚度，取值范围为 [-160, 160]。
数值越大越忠诚，初始值为 0，主公的忠诚度永远为 160。
? sgs.ai_anti_lord：表
% 元素：数值，为元素名称对应的玩家的明确攻击主公的次数，取值范围为非负数。
初始值为 nil
! sgs.ai_renegade_suspect：表，包含内奸的可疑程度
% 元素：数值，为元素名称对应的玩家的内奸可疑程度，取值范围为非负数。
数值越大越像内奸，初始值为 nil
? sgs.ai_explicit：表，包含玩家目前表现出的身份
% 元素：字符串，为以下四个值之一：
%% loyalist：忠臣（sgs.ai_loyalty 达到 160）
%% loyalish：跳忠，但没有到可以判为忠臣的程度（sgs.ai_loyalty 达到 80，但没达到 160）
%% rebel：反贼（sgs.ai_loyalty 达到 -160）
%% rebelish：跳反，但没有到可以判为反贼的程度（sgs.ai_loyalty 小于 -80，但没达到 -160）
初始值为 nil，主公的取值永远为 "loyalist"。
在目前版本的 AI 中，对忠臣与跳忠，反贼与跳反之间的区别并不明晰。
（表现为很多时候对待忠臣与对待跳忠玩家的策略是完全相同的，反贼也是）
这有待以后逐步完善。

还有一些其它的表，因为还不完善，在此不作介绍。

++ AI 如何运用与身份判断有关的表里面的数据
将上面几个表整合起来得到敌友关系的是如下的重要函数：
* SmartAI:objectiveLevel(player)：获得对玩家 player 的动机水平。
% player：ServerPlayer*
% 返回值：数值，表示自身对 player 的动机水平。
动机水平为负，表示 player 是自己的友方。
动机水平介乎 0 到 3 之间（含 0 与 3），表示 player 是自己的敌方，但是不会对 player 使用【杀】。
动机水平大于 3，表示 player 是自己的敌方，且会对 player 使用【杀】。
总的来说，动机水平越负，表示友善程度越高。
目前的 AI 中，动机水平取值范围为 [-3, 6]。
此函数是 AI 中被调用最为频繁的函数之一。
想进一步了解 SmartAI.objectiveLevel 是怎么具体运用上面几个表里面的数据而最终得到动机水平的，
可以参见 smart-ai.lua 中的相关源码，此处不作介绍。

在 12-SmartAI.lua 中介绍的一系列与敌友关系有关的函数，都是基于 SmartAI.objectiveLevel 的。
当然，如果开启了身份预知，那么这些与敌友关系有关的函数将直接调用源代码中 AI 部分的相应代码。

在此顺带介绍一下与身份预知关系最密切的一个函数。
! sgs.isRolePredictable()：是否按照身份预知下的策略去运行 AI
% 返回值：布尔值，含义同上。
注意 “是否按照身份预知下的策略去运行 AI” 与 “是否在服务器窗口中勾选了身份预知” 是两个不同的概念。
更多细节请参看 sgs.isRolePredictable() 的源代码。

此外，还需要知道下面这一重要函数：
* SmartAI:updatePlayers(inclusive)：更新与敌友关系有关的列表。
% inclusive：布尔值，此处不作介绍。
% 返回值：nil。
% 相关表格：sgs.ai_global_flags
此函数有以下几个作用：
+ 将 sgs.ai_global_flags 指明的所有元素重置为 nil（详见下面的说明）
+ 生成表 self.friends, self.friends_noself, self.enemies。

此函数也是 AI 中被调用最为频繁的函数之一。
如果你有任何技能的 AI 代码觉得在开始之前更新一下这几个列表会比较好，可以直接在技能代码中调用]]
self:updatePlayers()
--[[
* sgs.ai_global_flags：表，包括表 sgs 中所有需要重置为 nil 的元素名称。
% 元素名称：无，用 table.insert 把元素加入本表
% 元素：字符串，要重置的元素名称。
用下面一个例子来说明，设原来]]
sgs.ai_global_flags == {"abc", "def-ghi", "@foo"}
sgs.abc == 3
sgs["def-ghi"] == true
sgs["@foo"] == function(a, b)
	return a + b/2 + 3
end
-- 则在执行 SmartAI.updatePlayers 之后
sgs.ai_global_flags == {"abc", "def-ghi", "@foo"}
sgs.abc == nil
sgs["def-ghi"] == nil
sgs["@foo"] == nil
--[[
++ AI 如何设置与身份判断有关的表里面的数据
前面两个部分的代码都是需要了解的，但是在实际编写的过程中并不需要直接用到。
与各个扩展最为紧密的使得 AI 的身份判断部分能够正确工作的代码，都落在这一部分。

在 AI 中，我们通过调用下面两个函数来更新与身份判断有关的表里面的数据。
? sgs.refreshLoyalty(player, intention)：更新玩家 player 的忠诚度
% player：ServerPlayer*，要更新忠诚度的玩家
% intention：数值，需要更新的忠诚度。
% 返回值：nil
这一函数的作用是更新表 sgs.ai_loyalty 的值，
使得 sgs.ai_loyalty 表中元素名称为 player 的对象名的元素增加 intention。
并相应更新 sgs.ai_renegade_suspect 和 sgs.ai_explicit。
% 例子：]]
sgs.refreshLoyalty("sgs5", 80) -- 对象名为 sgs5 的玩家的忠诚度加上 80
sgs.refreshLoyalty("sgs7", -100) --[[ 对象名为 sgs7 的玩家的忠诚度减去 100
这一函数实际上较少直接使用，仅用于少数与主公技有关的身份判断情形。

下面这一函数更经常使用，用起来也更为方便：
* sgs.updateIntention(from, to, intention, card)：根据仇恨值更新忠诚度
% from：ServerPlayer*，行为来源
% to：ServerPlayer*，行为对象
% intention：数值，行为的仇恨值
% card：Card*，与行为有关的卡牌（可以是实体卡或者虚拟卡），如果行为不涉及卡牌，则为 nil。
所谓仇恨值，是指 from 对 to 执行这一行为表明 from 对 to 有多大的敌意。
对于目前的 AI，仇恨值与忠诚度的关系如下：
如果 to 是主公，则 from 的忠诚度减去仇恨值的两倍。
如果 to 跳忠或可被判为忠臣，则 from 的忠诚度减去仇恨值。
如果 to 跳反或可被判为反贼，则 from 的忠诚度增加仇恨值。
% 返回值：nil
% 例子：]]
sgs.updateIntention(huatuo, sunquan, -80) --[[华佗对孙权执行了一个友善的行为，仇恨值为 -80

为了简化诸如多目标锦囊的 sgs.updateIntention 的调用流程，另设了一个辅助函数：]]
function sgs.updateIntentions(from, tos, intention, card)
-- % from, intention, card, 返回值：含义同 sgs.updateIntention
-- % tos：表，包含所有行为对象
	for _, to in ipairs(tos) do -- 遍历 tos 中的玩家
		if from:objectName() ~= to:objectName() then -- 如果行为来源与行为对象不是同一个人
			sgs.updateIntention(from, to, intention，card)
		end
	end
end
--[[
介绍完这些重要的工具函数之后，可以开始讲述 AI 如何从具体的游戏过程中设置与身份判断有关的表的数据了。

这一部分内容略为难以理解，但却是未来 AI 身份判断进一步完善的突破口。
如果没有兴趣了解更多细节，可以直接跳到本文档后面关于 sgs.ai_card_intention 的描述。
不阅读这一部分对于实际 AI 的编写影响很小。

---------------------选读部分开始---------------------
在这一部分，AI 的角色变了，AI 不再是决策者，而成为了游戏的观察者和记录者。
这部分 AI 的主要任务，就是处理游戏中发生的各种事件，得出这些事件背后意味着的敌友关系。

而很容易理解，对于一局游戏来说，只要有一个记录者就够了。
! sgs.recorder：一个特殊的 SmartAI 对象，它是游戏的记录者

! SmartAI:filterEvent(event, player, data)：记录发生的事件。
% event：TriggerEvent 类型，表示事件的种类，详情请参见神主的讲座帖和 src/server/structs.h。
% player：ServerPlayer*，事件发生的对象
% data：QVariant*，与事件有关的数据
% 返回值：nil
% 相关的表：很多，详见下述。
这一函数当 self 不是 sgs.recorder 的时候将不会执行任何与身份判断有关的处理。
当 self 是 sgs.recorder 的时候，会根据事件不同作出不同的处理。
这一函数会在以下事件发生时调用 SmartAI.updatePlayer：
sgs.CardUsed, sgs.CardEffect, sgs.Death, sgs.PhaseChange, sgs.GameStart

接下来要介绍一个在编写扩展的时候从来不需要用到，但是对 AI 却很重要的事件：sgs.ChoiceMade。
这个事件表示 player 作出了某种选择。
这个事件之所以如此重要，是因为它具有超前性，例如，“作出使用某张牌的选择” 明显比 sgs.CardUsed 时间要超前得多。
后者发生在卡牌使用的结算完成之后。
这个事件在解决循环激将的问题上起到了关键作用。下面会具体说明。

接下来介绍与 sgs.ChoiceMade 有关的表：
* sgs.ai_choicemade_filter：表，包含与 sgs.ChoiceMade 有关的身份判断的 AI 代码。
% 元素名称与元素：目前版本的 AI 中共 5 个。

%% cardUsed：表，包含“决定使用某张卡牌”相关的事件响应函数
%%% 元素名称：无，用 table.insert 把元素加入本表
%%% 元素：函数，原型为 function(player, carduse)
%%%% player，返回值：与 SmartAI.filterEvent 含义相同
%%%% carduse：CardUseStruct*，卡牌使用结构体，用于描述作出的决策。
不难理解，大部分与 AI 出牌有关的身份判断代码应该放在这一部分。
但是，目前由于历史原因，大部分使用卡牌的身份判断还是交给了 sgs.CardUsed 事件去处理，
即在卡牌结算结束后处理。

%% cardResponsed：表，包含“决定打出某张卡牌或不打出任何卡牌”相关的事件响应函数
%%% 元素名称：Room::askForCard 当中的 prompt
%%% 元素：函数，原型为 function(player, promptlist)
%%%% player，返回值：与 SmartAI.filterEvent 含义相同
%%%% promptlist：表
%%%%% promptlist[2]：Room::askForCard 或者 Room::askForUseCard 当中的 pattern
%%%%% promptlist[#promptlist]：字符串，若为 "_nil_"，表示决定不打出卡牌，否则表示打出了卡牌。

%% skillInvoke：表，包含“决定发动或不发动某项技能”相关的事件响应函数
%%% 元素名称：技能名
%%% 元素，原型为 function(player, promptlist)
%%%% player，返回值：与 SmartAI.filterEvent 含义相同
%%%% promptlist：表
%%%% promptlist[3]：字符串，若为 "yes"，表示选择发动此技能，否则选择不发动。

%% skillChoice：表，包含“决定选择某一项”相关的事件响应函数
%%% 元素名称：技能名
%%% 元素，原型为 function(player, promptlist)
%%%% player，返回值：与 SmartAI.filterEvent 含义相同
%%%% promptlist：表
%%%% promptlist[3]：字符串，为所作的选择。

%% Nullification：表，包括“决定使用无懈可击”相关的事件响应函数
这个表目前很少用到，不作介绍。

% 例子：standard-ai.lua 第 472 至 497，531 至 536 行关于激将的代码。]]
-- sgs.jijiangsource 用于记录激将的来源
table.insert(sgs.ai_global_flags, "jijiangsource")
-- 每当执行 SmartAI.updatePlayers 时，清除激将的来源
local jijiang_filter = function(player, carduse)
	if carduse.card:inherits("JijiangCard") then -- 如果有人使用了激将技能卡
		sgs.jijiangsource = player -- 记录激将来源
	else -- 如果有人使用了其它卡（注意是使用不是打出）
		sgs.jijiangsource = nil -- 清除激将来源
	end
end

table.insert(sgs.ai_choicemade_filter.cardUsed, jijiang_filter)
-- 把上面的函数注册到 sgs.ai_choicemade_filter 里面

sgs.ai_skill_invoke.jijiang = function(self, data)
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("Slash") then
			return false
		end
	end
	if sgs.jijiangsource then return false else return true end
	-- 如果已经有人在激将，则不发动激将
end

sgs.ai_choicemade_filter.skillInvoke.jijiang = function(player, promptlist)
	if promptlist[#promptlist] == "yes" then-- 如果有人在要求打出【杀】询问是否发动激将时，选择了发动激将
		sgs.jijiangsource = player -- 记录下激将的来源
	end
end

-- 中间数行代码略去

sgs.ai_choicemade_filter.cardResponsed["@jijiang-slash"] = function(player, promptlist)
	if promptlist[#promptlist] ~= "_nil_" then -- 如果有人响应了激将
		sgs.updateIntention(player, sgs.jijiangsource, -40) -- 响应激将者对激将来源的仇恨值为 -40
		sgs.jijiangsource = nil -- 当有人响应时，激将的结算结束，故清除激将来源
	end
end
--[[
这个例子完整地展示了 sgs.ai_choicemade_filter 的应用。
如果您能够读懂这部分的代码，并且理解这部分的代码是如何防止循环激将的。您对 AI 的了解已经很不错了。
当然，读不懂也没关系，因为像激将这样的技能毕竟还是少数。

SmartAI.filterEvent 除了会处理 sgs.ChoiceMade，以及如前所述在一系列事件发生时调用 SmartAI.updatePlayers 之外，
还会处理下面的事件：sgs.CardEffect, sgs.Damaged, sgs.CardUsed, sgs.CardLost, sgs.StartJudge。
很遗憾，在目前版本的 AI 中，大部分的事件都没有提供可供扩展使用的接口。唯一提供了接口的是 sgs.CardUsed。
与这个事件相应的表是下面这个：
---------------------选读部分结束---------------------

* sgs.ai_card_intention：表，包括与卡牌使用相关的仇恨值
% 元素名称：卡牌的类名
% 元素：数值或函数
%% 函数：原型为 (card, from, tos, source)
%%% card：Card*，所使用的卡牌
%%% from：ServerPlayer*，卡牌的使用者
%%% tos：表，包括卡牌的所有使用对象
%%% source：ServerPlayer*，在卡牌使用时处于出牌阶段的玩家
在这个函数中，需要手动调用 sgs.updateIntention 来指明仇恨值。
%% 数值：表明卡牌使用者对卡牌使用对象的仇恨值。
实际上会自动调用函数 sgs.updateIntentions
对于绝大部分的卡牌，用数值已经可以满足要求。

% 例子 1：standard-ai.lua 第 1105 行关于青囊的代码。]]
sgs.ai_card_intention.QingnangCard = -100 -- 青囊的使用者对使用对象的仇恨值为 -100

-- 例子 2：maneuvering-ai.lua 第 180 至 188 行关于铁索连环的代码。
sgs.ai_card_intention.IronChain=function(card,from,tos,source)
	for _, to in ipairs(tos) do -- tos 是一个表
		if to:isChained() then -- 若使用对象处于连环状态
		-- 注意这里指的是铁索连环使用完之后的状态
			sgs.updateIntention(from, to, 80) -- 使用者对使用对象的仇恨值为 80
		else
			sgs.updateIntention(from, to, -80)
		end
	end
end
