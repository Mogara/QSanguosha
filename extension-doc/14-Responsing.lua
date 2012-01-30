--[[
通过前面三个 lua 文件的介绍，相信大家对 AI 的编写已经有了基本的认识。

这一文档将集中介绍与 smart-ai.lua 中第三部分“响应请求”的相关函数和类表。 
从这一文档介绍的内容开始，需要为 AI 编写作决策的代码了。
为了让 AI 作出正确的决策，必须给 AI 以足够的信息，其中一个方法就是通过 data 传递数据。
这在 11-Fundamentals.lua 中已经有所提及，下面深入地介绍一下 data 相关的一些内容。

++ data 是什么？
data 是一个 QVariant 对象，这一特殊的类型使得它可以传递任意类型的信息。
可以是一个整数、一张牌，一个使用牌的结构体（CardUseStruct），一个玩家（ServerPlayer*），等等。

++ 如何构造 data？
在 C++ 里，通过 QVariant data = QVariant::fromValue(object) 创建 data。
其中 object 可以是上面提到的任何一种对象。
在 Lua 里，通过下面的代码来构造 data。]]
local data = sgs.QVariant() -- 构造一个空的 QVariant 对象
data:setValue(object) -- 为 QVariant 对象设置值，相当于上面的 QVariant::fromValue(object)
--[[
++ 如何在 AI 里从 data 得到相应的值。
根据数据类型的不同，需要用不同的函数，列表如下。左边的是转换后得到的对象类型，右边是相应的转换函数。
这些内容实际上可以在 swig/qvariant.i 里面找到。注意所有的结构体都是以其指针的形式存在于 Lua 中的。
（更确切地说是一张特殊的表，更多细节可参见 SWIG 的文档）
number（数值类型）	data:toInt()							注意：Lua 里面没有 int 类型，只有 number 类型
string（字符串）	data:toString()
string 组成的表		data:toStringList()						注意：toStringList() 得到的是表（table）而不是列表（QList）
bool（布尔值）		data:toBool()
DamageStruct*		data:toDamage(), data:toDamageStar()	注意：两者没有实质上的区别
CardEffectStruct*	data:toCardEffect()
SlashEffectStruct*	data:toSlashEffect()
CardUseStruct*		data:toCardUse()
CardMoveStruct*		data:toCardMove()
Card*				data:toCard()
ServerPlayer*		data:toPlayer()
DyingStruct*		data:toDying()
RecoverStruct*		data:toRecover()
JudgeStruct*		data:toJudge()
PindianStruct*		data:toPindian()

关于上面提到的这些结构体的具体含义及其数据成员，已经超出本文档的范围，参见 src/server/structs.h

下面将给出 smart-ai.lua 中第三部分的一些函数及相关的表。
% skill_name：在下面的大多数函数中充当参数，表示技能名称
% self：下面大多数表的函数原型中的第一个参数，表示自身的 SmartAI 对象

! SmartAI:askForSuit()
该函数用于响应“选择花色”的请求。
% 返回值：Card::Suit 之一。即 sgs.Card_Spade, sgs.Card_Heart, sgs.Card_Club, sgs.Card_Diamond 之一
% 相关的表：暂无
% 默认行为：按概率 2:2:2:3 的比例随机选择黑桃、红桃、草花、方片

! SmartAI:askForSkillInvoke(skill_name, data)：响应 Room::askForSkillInvoke 的函数。
该函数用于响应“是否发动技能 skill_name”的请求。
% 返回值：布尔值，表示是否发动该技能
% 相关的表：sgs.ai_skill_invoke
% 默认行为：如果技能的发动频率（Frequency）为 sgs.Skill_Frequent，则发动，否则不发动

* sgs.ai_skill_invoke：
% 元素名称：技能名
% 元素：布尔值或函数
%% 布尔值一般为 true，表示该技能不管什么情况都发动
%% 函数，原型为：function(self, data)
%% 返回值：与 SmartAI.askForSkillInvoke （中的返回值，下略）含义相同

% 例子 1：mountain-ai.lua 第 168 行]]
sgs.ai_skill_invoke.tuntian = true -- 屯田总是发动
--[[% 例子 2：thicket-ai.lua 第 55 至 58 行，参见 11-Fundamentals.lua

! SmartAI:askForChoice(skill_name, choices)：响应 Room::askForChoice 的函数
该函数用于响应“请选择”的请求。
在用户界面中表现为一个以技能名为标题，对应于每一个选择有一个按钮的对话框。
% choices: 表，包含所有可用选择
% 返回值：字符串，是 choices 中的一项，表示作出的选择
% 相关的表：sgs.ai_skill_choice
% 默认行为：通过函数 Skill::getDefaultChoice 获得技能的默认选择（默认选择缺省为 "no"），
如果默认选择在 choices 中，则返回默认选择。否则随机返回 choices 中的一个元素。

* sgs.ai_skill_choice：
% 元素名称：技能名
% 元素：字符串值或函数
%% 字符串值：表明不论何种情况下都作出同一个给定的选择
%% 函数：原型为 function(self, choices)
%% choices, 返回值: 与 SmartAI.askForChoice 含义相同
% 例子 1：god-ai.lua 第 460 行]]
sgs.ai_skill_choice.jilve="zhiheng" -- 极略选择是制衡还是完杀时，总是选择制衡。
--% 例子 2：yitian-ai.lua 第 484 至 487 行
sgs.ai_skill_choice.yisheask=function(self,choices)
	assert(sgs.yisheasksource) -- 请暂且忽略这句话
	if self:isFriend(sgs.yisheasksource) then return "allow" else return "disallow" end
	-- 如果义舍要牌的请求来自友方，则接受请求，否则拒绝请求。
end
--[[
! SmartAI:askForDiscard(reason, discard_num, optional, include_equip)：
响应 Room::askForDiscard 与 Room::askForExchange 的函数
该函数用于响应 “请弃掉 X 张牌” 和 “请选择 X 张牌进行交换” 的请求
% reason：字符串，弃牌的原因，一般为技能名。如果是正常的弃牌阶段的弃牌，则为“gamerule”
% discard_num：数值，请求弃牌的张数
% optional：布尔值，是否可以选择不弃牌
% include_equip：布尔值，是否允许弃装备
% 返回值：表，包括所有要弃的牌的 ID
% 相关的表：sgs.ai_skill_discard
% 默认行为：若可以选择不弃牌，则不弃，否则按保留值从小到大依次弃牌。
如果允许弃装备且有 sgs.lose_equip_skill 中的技能，则优先弃装备。

* sgs.ai_skill_discard：
% 元素名称：弃牌原因（即 reason）
% 元素：函数，原型为 function(self, discard_num, optional, include_equip)
%% discard_num, optional, include_equip, 返回值：与 SmartAI.askForDiscard 含义相同
若返回 nil，则执行默认行为。
% 例子；standard-ai.lua 第 82 至 108 行关于刚烈的代码。]]
sgs.ai_skill_discard.ganglie = function(self, discard_num, optional, include_equip)
	if self.player:getHp() > self.player:getHandcardNum() then return {} end
	-- 若体力值比手牌数多，则不弃牌。此策略有待商榷。

	if self.player:getHandcardNum() == 3 then -- 手牌数为 3 时（临界情形）
		local to_discard = {} -- 初始化 to_discard 为空表
		-- 这一句不可省略，否则 table.insert(to_discard, ...) 会报错
		local cards = self.player:getHandcards() -- 获得所有手牌
		local index = 0
		local all_peaches = 0
		for _, card in sgs.qlist(cards) do
			if card:inherits("Peach") then
				all_peaches = all_peaches + 1 -- 计算出手牌中【桃】的总数。
			end
		end
		if all_peaches >= 2 then return {} end -- 若至少有 2 张【桃】，则不弃牌。

		for _, card in sgs.qlist(cards) do
			if not card:inherits("Peach") then
				table.insert(to_discard, card:getEffectiveId())
				-- 把不是【桃】的牌的 ID 加入到弃牌列表之中
				index = index + 1
				if index == 2 then break end -- 若弃牌列表中已经有两张牌的 ID，则中止循环
				-- 此处去除局部变量 index 而改用 #to_discard 会使代码更为简洁
			end
		end
		return to_discard -- 返回弃牌列表
	end

	if self.player:getHandcardNum() < 2 then return {} end -- 若手牌数不足 2 张，则无法弃牌。
end -- 其它情况，按照默认行为（弃牌阶段的策略）弃牌。
--[[
在这里插一句，从上面的注释可以看到现在 AI 的策略还有很多不完善的地方。
代码也还比较脏，这正是以后需要逐步努力改进的。

! SmartAI:askForNullification(trick_name, from, to, positive)：
响应 Room::askForNullification 的函数，该函数用于响应“是否使用【无懈可击】”的请求。
% trick_name：Card* 类型，表示对何张锦囊牌使用无懈可击
（本变量名易使人误以为是字符串类型，将在以后修改）
% from：ServerPlayer*，trick_name 的使用者（不是【无懈可击】的使用者）
% to: ServerPlayer*，trick_name 的使用对象
% positive：为 true 时，本【无懈可击】使 trick_name 失效，否则本【无懈可击】使 trick_name 生效
% 返回值：Card*，决定使用的【无懈可击】。如果为 nil，表示不使用【无懈可击】
% 相关的表：无
% 默认行为：较复杂，简单地说就是根据锦囊是否对己方有利决定是否使用。
有兴趣的可参见 smart-ai.lua 中的源代码。

! SmartAI:askForCardChosen(who, flags, reason)：响应 Room::askForCardChosen 的函数
该函数用于响应“请从给定的牌中选择一张”的请求。
在用户界面中表现为类似使用【顺手牵羊】的时候出现的对话框。
% who：ServerPlayer*，从何人的牌中选择
% flags：字符串，"h", "e", "j" 的任意组合，参见 12-SmartAI.lua
% reason：字符串，请求的原因，可能是技能名或者是卡牌的对象名（后者如 "snatch"）
% 返回值：数值，选择的实体卡的 ID。
% 相关的表：sgs.ai_skill_cardchosen
% 默认行为：即使用【过河拆桥】的时候选择牌的策略，较为复杂，有兴趣的可参见 smart-ai.lua 中的源代码。

? sgs.ai_skill_cardchosen：
% 元素名称：reason，其中所有的短横 "-" 要用下划线 "_" 取代。
% 元素：函数，原型为 cardchosen(self, who)
%% who, 返回值：与 SmartAI.askForCardChosen 含义相同
返回值为 nil 时，执行默认行为。
% 例子：只有一个，在 mountain-ai.lua 第 70 至 74 行。这是因为绝大部分情况下默认行为已经能满足要求。]]
sgs.ai_skill_cardchosen.qiaobian = function(self, who, flags)
	if flags == "ej" then
		return card_for_qiaobian(self, who, "card")
		-- 调用 mountain-ai.lua 第 1 行开始定义的辅助函数得到结果。
	end
end
--[[
! SmartAI:askForCard(pattern, prompt, data)：响应 Room::askForCard 的函数
该函数用于响应“请打出一张牌”的请求。
在用户界面中表现为一个提示框，框内文字由 prompt 与翻译文件决定。