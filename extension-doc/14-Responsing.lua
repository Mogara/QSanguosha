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

关于上面提到的这些结构体的具体含义及其数据成员，已经超出本文档的范围。
请参见神大的讲座帖、roxiel 的教程和 src/server/structs.h

下面将给出 smart-ai.lua 中第三部分的一些函数及相关的表。
% skill_name：在下面的大多数函数中充当参数，表示技能名称
% self：下面大多数表的函数原型中的第一个参数，表示自身的 SmartAI 对象

下文中提到的大多数函数都与相应的表关联。所谓“默认行为”是指该表中相应的元素未定义的时候的行为。

! SmartAI:askForSuit()
该函数用于响应“选择花色”的请求。
% 返回值：Card::Suit 之一。即 sgs.Card_Spade, sgs.Card_Heart, sgs.Card_Club, sgs.Card_Diamond 之一
% 相关的表：暂无（以后会加入）
% 默认行为：按概率 2:2:2:3 的比例随机选择黑桃、红桃、草花、方片（此策略有待商榷）
目前与这一函数有关的技能只有周瑜的反间。

! SmartAI:askForSkillInvoke(skill_name, data)：响应 Room::askForSkillInvoke 的函数。
该函数用于响应“是否发动技能 skill_name”的请求。
在用户界面中表现为“你要发动技能 XX 吗？”的提示框（Window）。
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
该函数用于响应 “请选择” 的请求。
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
--% 例子 2：yitian-ai.lua 第 486 至 489 行关于义舍要牌的代码
sgs.ai_skill_choice.yisheask=function(self,choices)
	assert(sgs.yisheasksource) -- 关于 assert，见 15-Activate.lua
	if self:isFriend(sgs.yisheasksource) then return "allow" else return "disallow" end
	-- 如果义舍要牌的请求来自友方，则接受请求，否则拒绝请求。
end
--[[
! SmartAI:askForDiscard(reason, discard_num, optional, include_equip)：
响应 Room::askForDiscard 与 Room::askForExchange 的函数。
该函数用于响应 “请弃掉 X 张牌” 和 “请选择 X 张牌进行交换” 的请求。
在用户界面中表现为 “请弃掉 X 张牌” 和 “请选择 X 张牌进行交换” 的提示框。
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
% pattern：字符串，用于匹配的模式串
% prompt：字符串，表示提示信息
为后面描述的方便起见，这里需要介绍一下 prompt 的标准格式。
它与翻译文件里面以百分号开头的参数占位符一一对应。
最一般的 prompt 格式如下：
"%prompt_type:%src:%dest:%arg:%2arg"
%% %prompt_type：表示 prompt 的类型，决定了将会读取翻译文件中的哪一个条目。
%% %src：为一个 ServerPlayer 对象的对象名（"sgsX"），翻译文件中的 %src 将以相应的武将名代入
%% %dest：为一个 ServerPlayer 对象的对象名（"sgsX"），翻译文件中的 %dest 将以相应的武将名代入
%% %arg, %2arg：为自定义的额外参数，将直接翻译后代入。
更详细的说明属于技能编写范畴不属于 AI 范畴，故不再赘述。

% 返回值：字符串，将用于 Card::Parse 得到实际的卡牌，如果为 "."，表示不打出任何卡。
% 相关的表：sgs.ai_skill_cardask
% 默认行为：当 pattern 为 "slash" 或 "jink" 的时候，
在符合 pattern 的卡牌中随机挑选一张返回其 ID，如果没有任何卡牌满足要求则返回 "."
其余情况一律返回 "."（在目前版本的 AI 中，返回值可能为 nil，留待以后修正）。

* sgs.ai_skill_cardask：
% 元素名称：就是 %prompt_type
% 元素：函数，原型为 function(self, data, pattern, target, target2)
这是最完整的函数原型，实际上如果不需要用到后面的参数可以直接省略。
%% pattern，返回值：与 SmartAI.askForCard 含义相同
%% target：ServerPlayer*，表示与 %src 相应的 ServerPlayer 对象
%% target2：ServerPlayer*，表示与 %dest 相应的 ServerPlayer 对象
% 例子：standard-ai.lua 第 55 至 67 行关于鬼才的代码。]]
sgs.ai_skill_cardask["@guicai-card"]=function(self) -- 仅仅用到了第一个参数 self，后面的都可以省略
	local judge = self.player:getTag("Judge"):toJudge() -- 获得判定结构体

	if self:needRetrial(judge) then -- 若需要改判
		local cards = sgs.QList2Table(self.player:getHandcards()) -- 获得手牌的表
		local card_id = self:getRetrialCardId(cards, judge) -- 从所有手牌中寻找可供改判的牌
		if card_id ~= -1 then
			return "@GuicaiCard=" .. card_id -- 若找到则改判
		end
	end

	return "." -- 若不需要改判或没有可供改判的牌，则不打出任何牌
end
--[[
注意：这里的 AI 与旧版的不相同。
在旧版 AI 中，对应的第一行为 sgs.ai_skill_invoke["@guicai"] = function(self, prompt)。
已经仿照旧版 AI 编写改判技能 AI 的，请按照新的要求对相应的代码进行修改。

! SmartAI:askForUseCard(pattern, prompt)：响应 Room::askForUseCard 的函数
该函数用于响应“请使用一张牌”的请求。
在用户界面中表现为一个提示框，框内文字由 prompt 与翻译文件决定。
% pattern, prompt：与 SmartAI.askForCard 含义相同
% 返回值：字符串，将用于 CardUseStruct::parse 得到实际的卡牌使用结构体（CardUseStruct）
如果为 "."，表示不使用任何卡牌。
% 相关的表：sgs.ai_skill_use
% 默认行为：返回 "."。

此处必须引入对辅助函数的介绍：
void CardUseStruct::parse(const QString &str, Room *room)：根据字符串 str 设定卡牌使用结构体。
str 的格式如下："%card_str->%target_str"，注意其中没有多余的空格。
%card_str 是一个可以被 Card::Parse 解析的字符串，表示所使用的卡牌（可以是实体卡或虚拟卡）
此部分字符串决定了成员 card 的内容。
%target_str 是一个表示卡牌使用对象的字符串，它由一个或多个 ServerPlayer 对象的对象名（"sgsX"）用加号连接而成。
%target_str 也可以是 "."，表示卡牌没有使用对象（但是这种情况一般在编写技能时用 askForCard 就可以满足要求）。
此部分字符串决定了成员 to 的内容。

尽管有着与 sgs.Card_Parse 类似的函数 sgs.CardUseStruct_parse （注意 p 的大小写），
但是迄今为止在 AI 代码中还没有直接调用过这一函数，也不建议大家在编写 AI 时使用。

对于 AI 的编写来说，CardUseStruct::from 永远自动设定为 self.player，而成员 card 和 to 则需要由返回值字符串指定。

* sgs.ai_skill_use：
% 元素名称：pattern（注意与 sgs.ai_skill_cardask 的元素名称不同）
% 元素：函数，原型为 function(self, prompt)
%% prompt, 返回值：与 SmartAI.askForUseCard 含义相同
其中返回值可以为 nil，此时 SmartAI.askForUseCard 返回 "."。
% 例子：yitian-ai.lua 中第 243 至 253 行关于连理的代码。]]
sgs.ai_skill_use["@lianli"] = function(self, prompt)
	-- 元素名（即 pattern）为 "@lianli"，含有特殊字符 "@"，用方括号而不是点作索引
	-- 关于方括号和点的区别和联系，见 11-Fundamentals.lua
	self:sort(self.friends) -- 将自己的所有友方按嘲讽值排序，关于 SmartAI.sort，见 12-SmartAI.lua
	
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() then -- 找到嘲讽最高的友方男性角色
			return "@LianliCard=.->" .. friend:objectName() -- 对他使用连理技能卡
		end
	end
	
	return "."	-- 找不到的时候，不使用连理技能卡
end
--[[
! SmartAI:askForAG(card_ids, refusable, reason)：响应 Room::askForAG 的函数
该函数用于响应“请从展示的数张牌中选择一张”的请求，例如五谷、心战的第一阶段等。
在用户界面中表现为类似【五谷丰登】使用时的框。（CardContainer）
% card_ids：表，包含所有可供选择的卡牌的 ID
% refusable：布尔值，表示是否可以不选
% reason：字符串，表示请求选择的原因，对于因技能发起的请求一般为技能名。
% 返回值：数值，选择的牌的 ID。如果为 -1，表示随机选择一张。
% 相关的表：sgs.ai_skill_askforag
% 默认行为：即【五谷丰登】的选牌策略，调用 SmartAI.sortByCardNeed 对卡牌需要程度进行排序，选择最需要的卡牌。

* sgs.ai_skill_askforag：
% 元素名称：reason，其中所有的短横 "-" 要用下划线 "_" 取代。
% 元素：函数，原型为 function(self, card_ids)。
%% card_ids, 返回值：与 SmartAI.askForAG 含义相同。
% 例子：wind-ai.lua 第 258 行关于不屈吃桃的时候选择要去掉的不屈牌的代码。]]
sgs.ai_skill_askforag.buqu = function(self, card_ids)
	for i, card_id in ipairs(card_ids) do
		for j, card_id2 in ipairs(card_ids) do
			if i ~= j and sgs.Sanguosha:getCard(card_id):getNumber() == sgs.Sanguosha:getCard(card_id2):getNumber() then
			-- 若两张牌 ID 不等，但是点数相同
				return card_id -- 返回找到的第一组相同点数的卡的第一张的 ID
			end
		end
	end

	return card_ids[1] -- 返回第一张（相当于随机）
end
--[[
! SmartAI:askForCardShow(requestor, reason)：响应 Room::askForCardShow 的函数
该函数用于响应“展示一张卡牌”的请求。
在用户界面中表现为“XX 要求你展示一张手牌”的提示框。
% requestor：ServerPlayer*，表示请求的发出者
% reason：字符串，请求发出的原因，对于因技能发起的请求一般为技能名
% 返回值：Card*，表示展示的卡牌
% 相关的表：sgs.ai_cardshow
% 默认行为：返回随机一张手牌

* sgs.ai_cardshow：
% 元素名称：reason
% 元素：函数，原型为 function(self, requestor)
%% requestor, 返回值：与 SmartAI.askForCardShow 相同
% 例子：maneuverint-ai.lua 第 253 至 276 行关于对方使用火攻时展示牌的策略。]]
sgs.ai_cardshow.fire_attack = function(self, requestor)
	local priority  =
	{
	heart = 4,
	spade = 3,
	club = 2,
	diamond = 1
	} -- 优先级，红桃最优先展示，方片最次。
	local index = 0
	local result
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if priority[card:getSuitString()] > index then -- 若找到了优先级更高的卡牌
			result = card -- 则应展示优先级更高的卡牌
			index = priority[card:getSuitString()]
		end
	end
	if self.player:hasSkill("hongyan") and result:getSuit() == sgs.Card_Spade then -- 有技能 “红颜” 且卡牌为黑桃时
		result = sgs.Sanguosha:cloneCard(result:objectName(), sgs.Card_Heart, result:getNumber()) -- 转换为红桃，点数和种类不变
		result:setSkillName("hongyan") -- 展示的卡牌的技能名为 “红颜”
	end

	return result -- 返回结果
end
--[[
! SmartAI:askForYiji(cards)：用于响应 Room::askForYiji 的函数
该函数仅与“遗计”技能相关，故说明略去。

! SmartAI:askForPindian(requestor, reason)：用于响应 Room::askForPindian 的函数
该函数用于响应 “打出一张牌进行拼点” 的请求。
在用户界面表现为 “打出一张牌进行拼点” 的提示框。
% requestor：ServerPlayer*，请求的发出者
% reason：字符串，请求发出的原因，对于因技能发起的请求一般为技能名
% 返回值：Card*，用于拼点的卡牌
% 相关的表：暂无（以后会加入）
% 默认行为：返回点数最大的手牌

! SmartAI:askForPlayerChosen(targets, reason)：用于响应 Room::askForPlayerChosen 的函数
该函数用于响应 “在给定的范围内选择一名玩家” 的请求。
在用户界面表现为 “请选择一名玩家” 的提示框。
% targets：列表（QList），包含所有可供选择的玩家
% reason：字符串，请求发出的原因，对于因技能发起的请求一般为技能名
% 返回值：ServerPlayer*，选择的玩家
% 相关的表：sgs.ai_skill_playerchosen
% 默认行为：在 targets 中随机选择一名角色返回。

* sgs.ai_skill_playerchosen：
% 元素名称：reason，其中所有的短横 "-" 要用下划线 "_" 取代。
% 元素：函数，原型为 function(self,targets)
%% targets, 返回值：含义与 SmartAI.askForPlayerChosen 相同
返回值可以为 nil，此时执行默认行为。
% 例子：mountain-ai.lua 第 254 至 260 行关于放权的代码。]]
sgs.ai_skill_playerchosen.fangquan = function(self, targets)
	for _, target in sgs.qlist(targets) do
		if self:isFriend(target) then
			return target -- 返回第一个友方
		end
	end
end
--[[
为了提高代码的重用性，方便 AI 的编写，在 smart-ai.lua 和 standard-ai.lua 中定义了两个标准的策略。
* sgs.ai_skill_playerchosen.damage：选择一名角色对其造成 1 点伤害
* sgs.ai_skill_playerchosen.zero_card_as_slash：选择一名角色，视为对其使用了一张无花色无属性的【杀】。
下面的代码说明了这些 “标准策略” 的使用：]]
sgs.ai_skill_playerchosen.luanwu = sgs.ai_skill_playerchosen.zero_card_as_slash -- 乱武
sgs.ai_skill_playerchosen.quhu = sgs.ai_skill_playerchosen.damage -- 驱虎

-- 技能“旋风”是一个很好的例子：
sgs.ai_skill_playerchosen.xuanfeng_damage = sgs.ai_skill_playerchosen.damage
sgs.ai_skill_playerchosen.xuanfeng_slash = sgs.ai_skill_playerchosen.zero_card_as_slash
--[[
! SmartAI:askForSinglePeach(dying)：用于响应 Room::askForSinglePeach 的函数
该函数用于响应濒死求桃的请求。在用户界面上表现为 “XX 正在死亡线上挣扎” 的提示框。
% dying: ServerPlayer*，处于濒死状态的角色
% 返回值：字符串，经 Card::Parse 之后得到实际的卡牌。]]
