--[[
在 14-Responsing.lua 中集中介绍了 smart-ai.lua 中第三部分的内容。
这一文档将集中介绍与 smart-ai.lua 中第四部分“主动出牌”相关的内容。

首先介绍一个重要的函数：
! SmartAI:activate(use)：响应 Room::activate 的函数。
% use: CardUseStruct*
% 返回值：nil。
这个函数所要完成的任务，就是给 use.card 和 use.to 赋予恰当的值。
这两个值就决定了 AI 使用哪张牌，对谁使用。

在这里，有必要介绍一下程序在出牌阶段的流程。这部分流程的代码位于 gamerule.cpp 第 63 至 75 行。

case Player::Play: {
		player->clearHistory();//清除上一回合的历史记录

		while(player->isAlive()){ //只要玩家还活着，就一直循环
			CardUseStruct card_use;
			room->activate(player, card_use); //调用 Room::activate 以获得卡牌使用结构体
			if(card_use.isValid()){ //如果卡牌使用结构体有效
				room->useCard(card_use); //按照卡牌使用结构体来使用卡牌
			}else //如果卡牌使用结构体无效
				break; //结束出牌阶段
		}
		break;
	}

借助这段代码，首先介绍下面的概念：
+ 当玩家刚刚进入出牌阶段时以及出牌阶段刚刚结算完一张卡牌的使用时，都会调用 Room::activate
+ 每一次 Room::activate 被调用，都会使客户端进入等待用户出牌的状态，此时开始计算出牌阶段的时间。
+ 对于人类玩家，出牌超时或者点击“弃牌”按钮会返回无效的卡牌使用结构体，从而使出牌阶段结束
+ 对于 AI，只要不给 use.card 赋值，就是使得卡牌使用结构体无效，从而使出牌阶段结束

再来看一下 smart-ai.lua 第 1988 至 2005 行 SmartAI:activate 的代码。]]
function SmartAI:activate(use)
	self:updatePlayers() -- 见 16-RoleJudgement.lua
	self:assignKeep(self.player:getHp(),true) -- 获得需要留在手上的卡牌
	self.toUse  = self:getTurnUse() -- 获得准备使用的卡牌
	self:sortByDynamicUsePriority(self.toUse) -- 将准备使用的卡牌按使用优先级从高到低排序
	for _, card in ipairs(self.toUse) do
		if not self.player:isJilei(card) then -- 如果没有被“鸡肋”
			local type = card:getTypeId()
			self["use" .. sgs.ai_type_name[type + 1] .. "Card"](self, card, use) -- 按卡牌的类型使用卡牌
			-- 这一行代码会设置 use.card 和 use.to
			
			if use:isValid() then -- 若这张卡牌能有效使用（即 use.card 不为 nil）
				self.toUse = nil
				return -- 已经设置好 use.card 和 use.to，故返回
			end
		end
	end -- 循环结束，还没有找到能有效使用的卡牌。use.card 为 nil，结束出牌阶段。
	self.toUse = nil
end
--[[
从上面的代码出发，需要介绍下面一个重要的函数：
? SmartAI:getTurnUse()：获得准备使用的卡牌
% 返回值：表，包含所有准备使用的卡牌
这一函数的执行过程将在介绍完下面的若干相关内容后进行介绍。
目前只需要知道，这个函数会调用 SmartAI.fillSkillCards（该函数更多细节见 smart-ai.lua），
而 SmartAI.fillSkillCards 会载入表 sgs.ai_skills。

* sgs.ai_skills：表，包含所有出牌阶段的视为技
% 元素名称：无，用 table.insert(sgs.ai_skills, askill) 把 askill 加入到 sgs.ai_skills 中。
% 元素: 表，包含一个视为技的信息，包含下列的元素。
%% name：技能名称
%% getTurnUse：函数，原型为 function(self, inclusive)
%%% inclusive：布尔值，目前仅见用于 standard-ai.lua，不作介绍
%%% 返回值：Card*，表示 AI 出牌阶段需要考虑使用的牌
其含义将在介绍完 SmartAI.getTurnUse 的执行流程后变得明晰
如果为 nil，则表示出牌阶段不能使用这一视为技。
% 例子 1：mountain-ai.lua 中第 411 行至 424 行关于直谏的代码。]]
local zhijian_skill={} -- 初始化 zhijian_skill 为空表
-- 这是一个本地变量，变量名并不重要，取成 zhijian_skill 只是为了出于习惯
zhijian_skill.name="zhijian" -- 设置 name
table.insert(sgs.ai_skills, zhijian_skill) -- 把这个表加入到 sgs.ai_skills 中
zhijian_skill.getTurnUseCard = function(self)
-- 这个函数的作用仅仅是让 AI 在出牌阶段考虑使用直谏技能卡的可能性
-- 至于对谁使用，子卡是什么，这里先不用管
	local equips = {}
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if card:getTypeId() == sgs.Card_Equip then
			table.insert(equips, card) -- 如果找到了手牌中的武器牌，则加入到表 equips 中
		end
	end
	if #equips == 0 then return end -- 如果一张武器牌也没有找到，则直接返回（即直谏技能卡不可用）。

	return sgs.Card_Parse("@ZhijianCard=.") -- 返回一张直谏技能卡，它没有子卡。
end

--% 例子 2：fire-ai.lua 第 100 行至 107 行与强袭有关的代码。
-- 这是出牌阶段仅允许使用一次的技能卡的标准写法。
local qiangxi_skill={}
qiangxi_skill.name="qiangxi"
table.insert(sgs.ai_skills,qiangxi_skill)
qiangxi_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("QiangxiCard") then -- 如果出牌阶段还没有使用过强袭技能卡
		return sgs.Card_Parse("@QiangxiCard=.") -- 则告知 AI 出牌阶段应考虑使用强袭技能卡
		-- 同样，是扣减体力还是弃武器，对谁使用，都不是这里要关心的问题
	end
end
-- 对于 Lua 技能卡，上面的代码应该作相应修改。设有一 Lua 制衡技能卡，其对象名为 "luazhiheng"
-- 则 if 部分应修改为下面的样子：
if not self.player:hasUsed("#luazhiheng") then
	return sgs.Card_Parse("#luazhiheng:.")
end
--[[在这里顺便提一下，为了使日志记录和翻译文件正常工作，Lua 技能卡的对象名应该与相应的技能的对象名相同。
在这一例子中，Lua 制衡的视为技的对象名也应该取成 "luazhiheng"。
像 5-Application.lua 里面那样的写法不能正确显示日志，而会显示为 “XX 使用了技能 【】”。

% 例子 3：standard-ai.lua 第 554 至 584 行关于武圣的代码。
这个例子展示了当出牌阶段视为技视为的卡牌不是技能卡时应该怎样写 getTurnUse
与前面两个例子不同，这里需要明确指定卡牌的花色、点数、子卡。
可以将这一例子与 13-ViewAs.lua 作一对比。]]
local wusheng_skill={}
wusheng_skill.name="wusheng"
table.insert(sgs.ai_skills,wusheng_skill)
wusheng_skill.getTurnUseCard=function(self,inclusive) 
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards) -- 获得包含手牌与装备区的表
	
	local red_card
	
	self:sortByUseValue(cards,true) -- 按使用价值从小到大排列卡牌
	
	for _,card in ipairs(cards) do
		if card:isRed() and not card:inherits("Slash") and not card:inherits("Peach") 				--not peach
			and ((self:getUseValue(card)<sgs.ai_use_value.Slash) or inclusive) then
			-- 如果卡牌为红色，且不是【杀】和【桃】，且使用价值比【杀】要低或者没有手牌
			red_card = card
			break -- 记录这一张卡牌并跳出循环
		end
	end

	if red_card then -- 若找到了合适的卡牌作为子卡	
		local suit = red_card:getSuitString()
		local number = red_card:getNumberString()
		local card_id = red_card:getEffectiveId()
		local card_str = ("slash:wusheng[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str) -- 武圣【杀】
		
		assert(slash) -- 验证 slash 不为 nil
		
		return slash
	end
end
--[[
这里引入了一个函数 assert，这个函数在调试的时候是很有用的。
* assert(condition)：验证 condition 不为 false 且不为 nil，如果验证失败，则出错。
对于目前的 AI 来说，出错意味着会在服务器窗口出现 “Assertion Failed!” 的错误信息。
% condition：任意有效的表达式
% 返回值：nil

接下来要介绍一系列函数，它们会被 SmartAI.getTurnUse 和 SmartAI.activate 调用。
下面介绍的内容实际上是 self["use" .. sgs.ai_type_name[type + 1] .. "Card"](self, card, use) 这一行的具体化。
先介绍这些函数的原型。
? SmartAI:useSkillCard(card,use)：使用技能卡
? SmartAI:useBasicCard(card,use)：使用基本牌
? SmartAI:useTrickCard(card,use)：使用锦囊牌
? SmartAI:useEquipCard(card,use)：使用装备牌
它们的参数的含义都是相同的：
% card：Card*，表示使用的卡牌
% use：这是一个重要的也是较为难以理解的参数。
当这一函数被 SmartAI.getTurnUse 调用时，它是一个表，内容如下
%% card：nil
%% to：nil
%% isDummy：true
当这一函数被 SmartAI.activate 调用时，它的类型为 CardUseStruct*，元素如下：
%% card: nil
%% to: 一个空的 ServerPlayer* 列表
%% isDummy: nil
% 返回值：nil

只有理解清楚上面说的两种不同情况下 use 的不同内容，才能理解 AI 中为什么会常常出现 if use.to then 这样的代码。
对于 Lua 扩展的编写者来说，只要关心上面提到的第一个函数提供的接口就可以了。
而对于 C++ MOD 的编写者来说，还需要关心后面三个函数提供的接口。
对于第四个函数，很遗憾，目前的 AI 还没有提供可供扩展的接口。
要实现这部分的 AI 只能按照 11-Fundamentals.lua 里面的说法，在自己的 lua 文件中改写 smart-ai.lua 里的函数。

SmartAI:useSkillCard 会载入表 sgs.ai_skill_use_func。

* sgs.ai_skill_use_func：
% 元素名称：对于一般的技能卡，为类名，对于Lua技能卡，为对象名前面加上 "#"。
% 元素：函数，原型为 function((card, use, self)
%% card, use, nil：与 SmartAI.useSkillCard 含义相同
% 例子1：god-ai.lua 第 462 至 464 行关于极略的代码，这是最为简单的例子了。]]
sgs.ai_skill_use_func.JilveCard=function(card,use,self)
	use.card = card
end

-- % 例子2：wisdom-ai.lua 第 300 至 310 行关于授业的代码。
-- 这个例子展示了如何正确地设置 use.to。
sgs.ai_skill_use_func.ShouyeCard = function(card, use, self)
	self:sort(self.friends_noself, "handcard") -- 按手牌数从小到大排序
	if self.friends_noself[1] then
		if use.to then use.to:append(self.friends_noself[1]) end
		-- if use.to 不可省略，用于判断调用这一函数的是 getTurnUse 还是 activate
		-- 用 append 方法来添加使用对象到 use.to 中
	end
	if self.friends_noself[2] then
		if use.to then use.to:append(self.friends_noself[2]) end
	end
	use.card = card -- 不管是被 getTurnUse 还是 activate 调用，都会执行这句
	-- card 的子卡已经在 getTurnUseCard 里面设置好，此处直接令 use.card = card 即可。
	return
end

-- % 例子 3：fire-ai.lua 第 109 至 149 行关于强袭的代码，这个例子相对要复杂一些。
-- 这个例子展示了如果在 getTurnUseCard 中没有正确地设置子卡，该如何写 ai_skill_use_func
sgs.ai_skill_use_func.QiangxiCard = function(card, use, self) -- card 参数在下面没有用到
	local weapon = self.player:getWeapon() -- 获得装备的武器
	if weapon then -- 如果装备有武器
		local hand_weapon, cards
		cards = self.player:getHandcards()
		for _, card in sgs.qlist(cards) do -- 查找手牌中的武器牌
			if card:inherits("Weapon") then
				hand_weapon = card
				break
			end
		end
		self:sort(self.enemies) -- 按嘲讽值从大到小对敌方排序
		for _, enemy in ipairs(self.enemies) do
			if hand_weapon and self.player:inMyAttackRange(enemy) then
				-- 如果手牌中有武器牌且对方在自己的攻击范围内
				use.card = sgs.Card_Parse("@QiangxiCard=" .. hand_weapon:getId())
				-- 获得强袭技能卡，以手牌中的武器卡为子卡
				if use.to then
					use.to:append(enemy)
				end
				break -- 此处可以写成 return，效果相同
			end
			if self.player:distanceTo(enemy) <= 1 then -- 如果到对方的距离在 1 以内
				use.card = sgs.Card_Parse("@QiangxiCard=" .. weapon:getId())
				-- 以装备有的武器作为子卡的强袭技能卡
				if use.to then
					use.to:append(enemy)
				end
				return -- 在正确设置 use.card 和 use.to 后，务必 return
				-- 否则就会出现诸如甘露指定了三个目标的情况
			end
		end
	else
		self:sort(self.enemies, "hp") -- 按体力值从小到大排序
		for _, enemy in ipairs(self.enemies) do
			if self.player:inMyAttackRange(enemy) and self.player:getHp() > enemy:getHp() and self.player:getHp() > 2 then
				-- 如果对方在自己的攻击范围内，且自己的体力值超过 2，且自己的体力值大于对方的体力值
				use.card = sgs.Card_Parse("@QiangxiCard=.")
				-- 不包含任何子卡的强袭技能卡，即自减一点体力
				if use.to then
					use.to:append(enemy)
				end
				return
			end
		end
	end
end
--[[
关于 SmartAI.useSkillCard 就介绍到这里，下面再介绍一个重要的辅助函数。

! SmartAI:useCardByClassName(card, use)：按类名来使用卡牌。该函数源代码如下：]]
function SmartAI:useCardByClassName(card, use)
	local class_name = card:className()
	local use_func = self["useCard" .. class_name] -- 调用的函数名称为 useCard 后接类名。

	if use_func then
		use_func(self, card, use) -- 调用相应的函数
	end
end
--[[
这一函数为 SmartAI.useBasicCard 和 SmartAI.useTrickCard 提供了接口。
例如，要告诉 AI 如何使用【桃】，就需要写一个函数 SmartAI.useCardPeach
（该函数位于 standard_cards-ai.lua）
要告诉 AI 如何使用【兵粮寸断】，就需要写一个函数 SmartAI.useCardSupplyShortage
（该函数位于 maneuvering-ai.lua）
这些函数的编写基本方法与原则与 sgs.ai_skill_use_func 里面的函数的编写相同，不再赘述。

在这些内容都介绍完以后，让我们回过头来看看 SmartAI.getTurnUse 的执行流程。
下面给出 SmartAI.getTurnUse 的源代码，并给出部分注释。
没有给出注释的代码属于一般 AI 编写不用关心的部分。]]

function SmartAI:getTurnUse() -- 这个函数的目的就是得到计划使用的牌（下称“计划用牌”）
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards) -- 第一步：获得所有手牌作为可用牌 cards

	local turnUse = {} -- 这个表将作为这个函数的返回值，即计划用牌
	local slashAvail = 1
	self.predictedRange = self.player:getAttackRange()
	self.predictNewHorse = false
	self.retain_thresh = 5
	self.slash_targets = 1
	self.slash_distance_limit = false

	self.weaponUsed = false

	if self.player:isLord() then self.retain_thresh = 6 end
	if self.player:hasFlag("tianyi_success") then
		slashAvail = 2
		self.slash_targets = 2
		self.slash_distance_limit = true
	end

	self:fillSkillCards(cards) -- 第二步：从可用牌中剔除所有因 FilterSkill 而不能使用的卡牌
	-- （会载入 sgs.ai_filterskill_filter）
	-- 并把主动视为技相关的卡牌加入到可用牌中
	-- （会调用 XXX_skill.getTurnUseCard）

	self:sortByUseValue(cards) -- 第三步：将可用牌按使用价值从大到小排序

	if self.player:hasSkill("paoxiao") or (self.player:getWeapon() and self.player:getWeapon():objectName() == "crossbow") then
		slashAvail = 100
	end


	local i = 0
	for _,card in ipairs(cards) do -- 遍历所有的可用牌
		local dummy_use = {}
		dummy_use.isDummy = true
		if not self:hasSkills(sgs.need_kongcheng) then
		-- 第五步，判断卡牌的使用价值是否已经低到不值得使用的程度，如果是，表明已经得到所需要的计划用牌表
			if (i >= (self.player:getHandcardNum()-self.player:getHp()+self.retain)) and (self:getUseValue(card) < self.retain_thresh) then
				return turnUse -- 第六步，返回计划用牌表
			end

			if (i >= (self.player:getHandcardNum()-self.player:getHp())) and (self:getUseValue(card) < 8.5) and self.harsh_retain then
				return turnUse -- 第六步，返回计划用牌表
			end
		end

		local type = card:getTypeId()
		self["use" .. sgs.ai_type_name[type + 1] .. "Card"](self, card, dummy_use)
		-- 第四步，决定本张卡牌是否值得使用
		-- 这一步会调用 sgs.ai_skill_use_func 或 SmartAI.useCardByClassName

		if dummy_use.card then -- 如果这张卡牌应该使用
			if (card:inherits("Slash")) then
				if slashAvail > 0 then
					slashAvail = slashAvail-1
					table.insert(turnUse,card) -- 则把它加入到计划用牌中
				end
			else
				if card:inherits("Weapon") then
					self.predictedRange = sgs.weapon_range[card:className()]
					self.weaponUsed = true
				end
				if card:inherits("OffensiveHorse") then self.predictNewHorse = true end
				if card:objectName() == "crossbow" then slashAvail = 100 end
				if card:inherits("Snatch") then i = i-1 end
				if card:inherits("Peach") then i = i+2 end
				if card:inherits("Collateral") then i = i-1 end
				if card:inherits("AmazingGrace") then i = i-1 end
				if card:inherits("ExNihilo") then i = i-2 end
				table.insert(turnUse,card) -- 则把它加入到计划用牌中
			end
			i = i+1
		end
	end

	return turnUse -- 第六步，已经遍历了所有可用牌，返回计划用牌表
end
--[[
最后，介绍一个直接在技能代码里面使用 SmartAI.getTurnUse 的例子。
yitian-ai.lua 第 181 至 191 行关于陆抗围堰的代码。]]
sgs.ai_skill_invoke.lukang_weiyan = function(self, data)
	local handcard = self.player:getHandcardNum() -- 手牌数
	local max_card = self.player:getMaxCards() -- 手牌上限

	prompt = data:toString()
	if prompt == "draw2play" then -- 把摸牌阶段当作出牌阶段
		return handcard >= max_card and #(self:getTurnUse())>0
		-- 发动条件：手牌数不小于手牌上限，且有牌可出
	elseif prompt == "play2draw" then -- 把出牌阶段当作摸牌阶段
		return handcard < max_card or #(self:getTurnUse()) == 0
		-- 发动条件：手牌数小于手牌上限，或者无牌可出
	end
end
