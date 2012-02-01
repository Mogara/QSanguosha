--[[
smart-ai.lua 是整个 AI 中最先载入的脚本，也是 AI 系统的核心。
下面就让我们来看一下 smart-ai.lua 文件的组成。

++ 第一部分：初始化、取值、排序（第 1 至 572 行）
! 全局变量 version：当前 AI 的版本；
! SmartAI:initialize() ：对 AI 进行初始化。
这些元素一般在编写 AI 时都不建议直接使用。
在后续的文档中，以感叹号开头表示。
相应的，在编写 AI 时需要直接用到的元素则用星号开头的行表示。
可以直接使用但是较少用到的则以问号开头的行表示。
另外有一些元素尽管曾经经常直接使用，但是在新的 AI 中已经很少使用，这类元素用 *? 开头表示。

在后续文档中，如果没有特别说明，“玩家”一词指的是某个 ServerPlayer 对象。

? sgs.getValue(player)：获得一个玩家的手牌数 + 血量 * 2（下称为综合值）。
? sgs.getDefense(player)：获得一个玩家的防御值。
! SmartAI:assignKeep(num, start)：获得 AI 留在手上不使用的牌。
? SmartAI:getKeepValue(card, kept)：获得一张卡牌的保留值。
该函数将会载入表 sgs.ai_keep_value, sgs.[[武将名]]_keep_value 和 sgs.[[武将名]]_suit_value
* sgs.ai_keep_value：包含卡牌的保留值的表，元素名为卡牌的类名（className）。
例子可见 maneuvering-ai.lua 第 96 行。]]
sgs.ai_keep_value.Analeptic = 4.5
--[[更多例子可参考旧版 AI 的 general_config.lua 文件的前 20 行。

*? sgs.[[武将名]]_keep_value：包含武将的卡牌的保留值的表，目前主要在 standard-ai.lua 中使用。
该表的元素名为卡牌的类名，例子如 standard-ai.lua 里面 586 行附近张飞的表：]]
sgs.zhangfei_keep_value = 
{
	Peach = 6,
	Analeptic = 5.8,
	Jink = 5.7,
	FireSlash = 5.6,
	Slash = 5.4,
	ThunderSlash = 5.5,
	ExNihilo = 4.7
}
--[[更多例子可以参考旧版 AI 的 value_config.lua 文件的第 57 行至第 152 行。

* sgs.[[武将名]]_suit_value：与上面类似。
不过这里的元素名是卡牌的花色名称（全部小写）
god-ai.lua 第 423 行提供了一个例子：]]
sgs.shenzhaoyun_suit_value = 
{
	heart = 6.7, --红桃
	spade = 5, --黑桃
	club = 4.2, --草花
	diamond = 3.9, --方片
}
--[[
更多例子可以参考 value_config.lua 文件的第 154 行至第 230 行。

? SmartAI:getUseValue(card)：获得一张卡牌的使用价值。
该函数会载入表 sgs.ai_use_value。
* sgs.ai_use_value：包含卡牌的使用价值的表。例子如 standard_cards-ai.lua 第 609 行：]]
sgs.ai_use_value.ExNihilo = 10 --无中生有
--[[更多例子可以参考 general_config.lua 文件的第 22 行至第 78 行。

? SmartAI:getUsePriority(card)：获得一张卡牌的使用优先级。
该函数会载入表 sgs.ai_use_priority。
* sgs.ai_use_priority：一个包含卡牌的使用优先级的表。
元素名称为卡牌的类名，数值越大优先级越高。例子如 yjcm-ai.lua 第 114 行：]]
sgs.ai_use_priority.XinzhanCard = 9.2
--[[更多例子可以参考 general_config.lua 文件的第 80 行至第 167 行。

? SmartAI:getDynamicUsePriority(card)：获得一张卡牌的动态使用优先级。
用 getUsePriority 得到的优先级没有考虑到其它可能使用的卡牌。
事实上一张牌的优先级不是一成不变的。
手牌中拥有某些牌可能会使一张牌的使用优先级上升或者下降。
该函数会载入表 sgs.dynamic_value。

* sgs.dynamic_value：包含卡牌类型的表，这些类型用于调整其它卡牌的优先级。
该表包括以下几个子表，各子表的元素名称均为卡牌的类名，值则只取 true。
damage_card：能造成伤害的牌
control_usecard：类似乐不思蜀的延时类锦囊
control_card：不直接造成伤害但是能造成负面效果的牌，如过河拆桥
lucky_chance：类似闪电的延时类锦囊
benefit：能造成正面效果的牌，如桃。
例子可见 maneuvering-ai.lua 第 133 行：]]
sgs.dynamic_value.control_usecard.SupplyShortage = true
--[[更多例子可以参考 value_config.lua 前 55 行。

从上面的例子我们可以看到 AI 运作的基本模式是载入适当的表。
而扩展编写者的任务就是为这些表设置适当的值，而不用去干预 AI 载入这些表的具体过程。

? SmartAI:cardNeed(card) 获得一张卡牌的需要程度。
* sgs.ai_chaofeng：包括武将嘲讽值的表。AI 倾向于集火嘲讽值高的对象。
在目前的 AI 中，嘲讽值起到的作用有限，例如不能直接影响 AI 出杀的决策。
但是在很多情况下，嘲讽值还是能发挥其作用的。
该表的元素名为武将名。例子如 god-ai.lua 第 53 行。]]
sgs.ai_chaofeng.shenguanyu = -6
--[[嘲讽值的默认值为 0。更多的例子可以参考 general_config.lua 的 172 行至 236 行。

AI 的一个重要任务是寻找最佳决策，因此对玩家或卡牌进行排序显得尤为重要。
smart-ai.lua 中提供了一系列对对象进行排序的函数。这些函数在 AI 编写中是经常用到的。

! sgs.ai_compare_funcs：包含对两个玩家进行比较的一系列函数的表，该表被 SmartAI:sort 调用。
* SmartAI:sort(players, key, inverse) :对表 players 中的玩家按照关键字 key 进行排序。
inverse 参数若存在且为 true，表示排序之后要对整个表反序。
可用的关键字有：
* hp：按体力值从小到大排序，例如在 fire-ai.lua 第 138 行与典韦强袭有关的代码：]]
self:sort(self.enemies, "hp") -- 将所有敌方玩家按体力值从小到大排序
--[[这里，如果写成self:sort(self.enemies, "hp", true)，则表示将敌方玩家按体力值从大到小排序。
* handcard: 按手牌数从小到大排序
* value: 按综合值从小到大排序。
* chaofeng：按嘲讽值从大到小排序，嘲讽值相同的，则按综合值从小到大排序。
* defense：按防御值从小到大排序
* threat：按威胁值从大到小排序

下面是一些对卡牌排序的函数，其中 inverse 参数的含义与 SmartAI:sort 相同，不再赘述。
* SmartAI:sortByKeepValue(cards, inverse, kept)：
在玩家已经决定保留 kept 中的牌的前提下，将表 cards 中的牌按保留值从小到大排序。
在实际使用中，很少用到 kept 参数。
例子如 standard-ai.lua 第 1081 至 1086 行与华佗青囊有关的代码：]]
local cards = self.player:getHandcards() -- 获得所有手牌
cards=sgs.QList2Table(cards) -- 将列表转换为表
self:sortByKeepValue(cards) -- 按保留值排序
local card_str = ("@QingnangCard=%d"):format(cards[1]:getId())
-- 用排序后的第一张（保留值最小）的卡牌作为青囊的卡牌。

--[[
* SmartAI:sortByUseValue(cards, inverse)：将表 cards 中的牌按照使用价值从大到小排序。
* SmartAI:sortByUsePriority(cards, inverse)：将表 cards 中的牌按照使用优先级从大到小排序。
* SmartAI:sortByDynamicUsePriority(cards, inverse)：将表 cards 中的牌按照动态优先级从大到小排序。
? SmartAI:sortByCardNeed(cards)：将表 cards 中的牌按照需要程度从小到大排序。

++ 第二部分：身份、动机、仇恨值（第 565 至 1225 行）
这一部分将在 16-RoleJudgement.lua 中详细介绍。对于一般的 AI 编写，所需要知道的函数是下面这些：
* SmartAI:isFriend(other, another)：判断友善的关系。参数均为 ServerPlayer* 类型。
如果只有一个参数 other，判断 other 是否自己的友方，如果是则返回 true，否则返回 false。
如果有两个参数，判断 other 与 another 之间是否互为友方。
* SmartAI:isEnemy(other, another)：判断敌意的关系。与 SmartAI:isFriend 类似，不再赘述。
* SmartAI:getFriends(player)：返回一个包含 player 的所有友方玩家对象指针的表。
* SmartAI:getEnemies(player)：返回一个包含 player 的所有敌方玩家对象指针的表。
关于 SmartAI.isFriend 的例子，见 11-Fundamentals.lua 里面关于颂威代码的说明。

++ 第三部分：响应请求（第 1227 至 1910 行）
这一部分包括所有响应 Room::askFor*** 请求的函数。将在 14-Responsing.lua 详细介绍。
这一部分引入了一个重要的辅助函数：
? SmartAI:getCardRandomly(who, flags)：
在 ServerPlayer* 指针 who 指向的玩家（在下面中简称为玩家who）的由 flags 标志的牌中，随机选择一张牌，返回其 ID。
flags: 为字母 h，e，j 的任意组合。这三个字母分别表示手牌，装备区和判定区。
例如 self:getCardRandomly(sunshangxiang, "e") 表示在孙尚香的装备区里随便选择一张牌返回其 ID。
（当然，在这句代码之前需要先定义 sunshangxiang 这个变量，例如通过下面的代码。
在后续的例子中均假设这样的代码已经存在，不再重复。）]]
local sunshangxiang = self.room:findPlayerBySkillName("xiaoji")
--[[
++ 第四部分：主动出牌（第 1912 至 2802 行）
这一部分主要包括出牌阶段主动出牌的相关策略，将在 15-Activate.lua 详细介绍。
下面先对这一部分引入的众多辅助函数进行说明。
下面为简洁起见，用 [] 注出可以缺省的参数，没有特殊说明时，player 缺省值均为 self.player。
* SmartAI:getOverflow([player])：获得玩家 player 的手牌超额，即手牌数与手牌上限的差值。
返回值非负，返回值为 0 表示手牌没有超出手牌上限。

* SmartAI:isWeak([player])：判断玩家 player 是否处于虚弱状态。
目前虚弱状态定义为体力值不高于 1，或体力值不高于 2 且手牌数不高于 2。

? sgs.getSkillLists(player)：获得玩家 player 的所有 ViewAsSkill 和 FilterSkill 的表。
下面的代码：]]
vsnlist, fsnlist = sgs.getSkillLists(player)
--[=[执行之后，vsnlist 为包含 player 的所有 ViewAsSkill 和 FilterSkill 的技能名（字符串）的表，
fsnlist 为包含 player 的所有 FilterSkill 的技能名的表。

* SmartAI:hasWizard(players[, onlyharm])：判断 players 中是否有能从判定中得益的玩家。
若 onlyharm 为 true，则当 players 中有能改判的玩家时，返回 true，否则返回 false。
若 onlyharm 为 false 或省略，则当 players 中有能改判的玩家或有郭嘉时，返回 true，否则返回 false。

* SmartAI:needRetrial(judge)：判断由 judge （为 JudgeStruct* 类型）指定的判定结果是否需要改判。
如果需要改判，则返回 true，否则返回 false。

* SmartAI:getRetrialCardId(cards, judge)：
从表 cards 中选出适宜用于改判 judge 的判定结果的牌，返回其 ID。
若找不到可以改判的牌，则返回 -1。注意本函数并不检验是否需要改判。

* SmartAI:damageIsEffective([player[, nature[, source]]])：
判断玩家 source 对玩家 player 造成的 nature 属性的伤害是否有效。
nature 为 sgs.DamgeStruct_Normal, sgs.DamageStruct_Thunder, sgs.DamageStruct_Fire 之一，
分别表示无属性、雷属性和火属性。
三个参数均可缺省。缺省时 player 和 source 为 self.player， nature 为 sgs.DamageStruct_Normal。

* SmartAI:getMaxCard([player])：获得玩家 player 的手牌中点数最大的一张。

* SmartAI:getCardId(class_name[, player])：
在玩家 player 的手牌与装备区中获得一张类名为class_name的牌，返回其 ID。
* SmartAI:getCard(class_name[, player])：与 getCardId 一样，但是返回的是卡牌本身而不是其 ID。
* SmartAI:getCards(class_name[, player[, flag]])：与 getCard 一样，
但是获得的不是一张卡牌而是所有符合条件的卡牌的表。
其中 flags 的含义与 SmartAI.getCardRandomly 相同。
* SmartAI:getCardsNum(class_name[, player[, flag[, selfonly]]])：
与 getCards 一样，但不是返回表本身而是返回表长（即牌数）。
selfonly 表示是否需要考虑房间里的其它玩家，
当 selfonly 为 false 或缺省时，有两种情况会计入其它玩家的牌数：
. player 有激将技能且计算【杀】的张数时，会计入所有友方蜀将的【杀】；
. player 有护驾技能且计算【闪】的张数时，会计入所有友方魏将的【闪】；
* SmartAI:getAllPeachNum([player])：获得玩家 player 及其友方所有的【桃】数。
在这一组函数中，全部均已经考虑视为技，但是需要编写相关的代码来使 AI 会使用视为技，详见 13-ViewAs.lua。

* SmartAI:hasSuit(suit_strings[, include_equip[, player]])：
判断玩家 player 是否有由 suit_strings 指定的花色的手牌。
include_equip 为 true 时，同时计入装备区的牌。
suit_string 为 "spade", "heart", "club", "diamond" 的任意组合，
组合时以竖线分隔，例如 spade|club 表示黑色牌。
* SmartAI:getSuitNum(suit_strings[, include_equip[, player]])：
与 hasSuit 类似，但是返回的不是“有没有”而是“有多少张”。

* SmartAI:hasSkill(skill)：判断自己是否有名称为 skill 的技能。
* SmartAI:hasSkills(skill_names[, player])：判断玩家 player 是否有由字符串 skill_names 所指定的一系列技能。
skill_names 为技能名的任意组合，分隔符为竖线。smart-ai.lua 2030 至 2034 行有 skill_names 的例子。]=]
sgs.lose_equip_skill = "xiaoji|xuanfeng"
sgs.need_kongcheng = "lianying|kongcheng"
sgs.masochism_skill = "fankui|jieming|yiji|ganglie|enyuan|fangzhu"
sgs.wizard_skill = "guicai|guidao|tiandu"
sgs.wizard_harm_skill = "guicai|guidao"
--说到这里，顺便提一下这种字符串与 table 之间的转换，用下面一个例子来说明再合适不过了。若
s == "foo|bar|any", t == {"foo", "bar", "any"}
--则
table.concat(t, "|") == s -- 再插一句，对于 QList，相应的函数为 join。
s:split("|") == t
--[[
* SmartAI:exclude(players, card)：从玩家表 players 中剔除卡牌 card 不能用在其上的玩家。
例如]]
players == {jiaxu, sunshangxiang, wolong}
-- 且 card 为黑色过河拆桥，则在执行 SmartAI:exclude(players, card) 之后，
players == {sunshangxiang, wolong}
--[[
* SmartAI:trickProhibit(card, to)：判断卡牌 card 能否对 to 使用。
* SmartAI:hasTrickEffective(card, player)：判断卡牌 card 是否对玩家 player 有效。
这三个函数的区别是：前两个考虑的是是否存在相应的禁止技，后一个则是考虑实际上有没有效。
这一区别就如帷幕和智迟的区别。

* SmartAI:hasEquip(card)：判断装备区是否有卡牌 card。
* SmartAI:isEquip(equip_name, player)：判断 player 是否装备有类名为 equip_name 的卡牌。（已经考虑八阵）
* SmartAI:hasSameEquip(card, player)：判断 player 的装备区是否有与 card 卡牌属于同一种类的装备。
这里的“种类”是指武器、防具、防御马、进攻马之一。

++ 第五部分：载入扩展的 AI （第 2804 至 2830 行）
对于这一部分，需要知道的是，standard-ai.lua, standard_cards-ai.lua 和 maneuvering-ai.lua 总是在其它扩展之前载入，
因此在扩展的 AI 中，可以直接使用这三个文件中已经定义的元素。

除了 smart-ai.lua 之外，在 standard_cards-ai.lua 里头还有几个编写 AI 时经常用到的函数，介绍如下：

* SmartAI:slashProhibit(card, enemy)：从策略上判断是否不宜对玩家 enemy 使用【杀】card。
* SmartAI:slashIsEffective(card, enemy)：判断【杀】card 是否对 enemy 有效
（无效是指诸如智迟、大雾之类的技能）。
* SmartAI:slashIsAvailable([player])：判断玩家 player 本回合还能否继续出【杀】。]]
