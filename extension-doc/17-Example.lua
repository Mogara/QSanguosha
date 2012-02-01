-- 下面以倚天包中义舍的技能代码为例，说明如何综合运用前面介绍的内容来编写完整的技能 AI
-- 这段代码位于 yitian-ai.lua 第 449 至 514 行
local yishe_skill={name="yishe"}
table.insert(sgs.ai_skills,yishe_skill) -- 注册义舍技能
yishe_skill.getTurnUseCard = function(self) -- getTurnUseCard 见 15-Activate.lua
	return sgs.Card_Parse("@YisheCard=.") -- 义舍技能卡，未指定子卡
end

sgs.ai_skill_use_func.YisheCard=function(card,use,self) -- 义舍技能卡的使用函数
	-- ai_skill_use_func 见 15-Activate.lua
	-- 这同时包括两种情况：把手牌加入到“米”中与把“米”中所有牌收回到手牌，故需分情况讨论
	if self.player:getPile("rice"):isEmpty() then -- 第一种情况
		local cards=self.player:getHandcards()
		cards=sgs.QList2Table(cards) -- sgs.QList2Table 见 11-Fundamentals.lua
		local usecards={}
		for _,card in ipairs(cards) do -- 先加入所有的【屎】牌
			if card:inherits("Shit") then table.insert(usecards,card:getId()) end
		end
		local discards = self:askForDiscard("gamerule", math.min(self:getOverflow(),5-#usecards))
		-- 按照弃牌阶段弃牌的策略来选择追加到“米”里面的牌
		-- 5-#usecards 表示最多只能义舍 5 张牌
		-- 需追加到义舍牌中的牌应该是手牌的溢出部分，且保证追加后“米”数不超过 5
		-- SmartAI.getOverflow 见 12-SmartAI.lua
		-- SmartAI.askForDiscard 见 14-Responsing.lua
		for _,card in ipairs(discards) do -- 把追加的牌插入到“米”中
			table.insert(usecards,card)
		end
		if #usecards>0 then -- 若有牌可以义舍
			use.card=sgs.Card_Parse("@YisheCard=" .. table.concat(usecards,"+"))
			-- sgs.Card_Parse 见 13-ViewAs.lua
		end
	else -- 第二种情况
		if not self.player:hasUsed("YisheCard") then use.card=card return end
		-- 仅当回合内没有使用过义舍，即“米”不是刚刚放进去时，才发动义舍把“米”回收到手牌
		-- ServerPlayer::hasUsed 的用法见 15-Activate.lua
	end
end

table.insert(sgs.ai_global_flags, "yisheasksource") -- sgs.ai_global_flags 见 16-RoleJudgement.lua
local yisheask_filter = function(player, carduse)
	if carduse.card:inherits("YisheAskCard") then
		sgs.yisheasksource = player -- 记录下义舍要牌的请求来源
	else
		sgs.yisheasksource = nil
	end
end

table.insert(sgs.ai_choicemade_filter.cardUsed, yisheask_filter) -- 注册到 sgs.ai_choicemade_filter 中
-- sgs.ai_choicemade_filter 见 16-RoleJudgement.lua

sgs.ai_skill_choice.yisheask=function(self,choices) -- sgs.ai_skill_choice 见 14-Responsing.lua
-- 决定是否接纳义舍要牌的请求
	assert(sgs.yisheasksource) -- 验证义舍要牌的来源已经正确设置，避免出错
	-- assert 见 15-Activate.lua
	if self:isFriend(sgs.yisheasksource) then return "allow" else return "disallow" end
	-- 如果是友方，则同意请求，否则拒绝请求
end

-- 对于已经详细阅读过 16-RoleJudgement.lua 的朋友，不妨思考一下
-- 如果想告诉 AI，接纳义舍要牌的请求属于友善的行为，该写什么代码

local yisheask_skill={name="yisheask"}
table.insert(sgs.ai_skills,yisheask_skill) -- 注册义舍要牌技能
yisheask_skill.getTurnUseCard = function(self)
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasSkill("yishe") and not player:getPile("rice"):isEmpty() then return sgs.Card_Parse("@YisheAskCard=.") end
		-- 如果有人有义舍技能且其“米”牌堆不为空，则考虑使用义舍要牌技能卡的可能性
		-- 此处可以用 self.room:findPlayerBySkillName 简化代码
	end
end

sgs.ai_skill_use_func.YisheAskCard=function(card,use,self) -- 义舍要牌技能卡的使用函数
	if self.player:usedTimes("YisheAskCard")>1 then return end -- 每回合最多义舍要牌两次
	-- 这里说明了对于限制回合内的最大使用次数的牌，其 AI 该怎样书写
	local zhanglu
	local cards
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasSkill("yishe") and not player:getPile("rice"):isEmpty() then zhanglu=player cards=player:getPile("rice") break end
	end	
	-- 同样，可以用 self.room:findPlayerBySkillName 简化代码
	if not zhanglu or not self:isFriend(zhanglu) then return end -- 如果不是友方，则不发出请求
	cards = sgs.QList2Table(cards)
	for _, pcard in ipairs(cards) do
		if not sgs.Sanguosha:getCard(pcard):inherits("Shit") then -- 若“米”牌中至少有一张不是【屎】牌，则发出请求
			use.card = card
			return -- 已经正确设置 use.card，直接返回
		end
	end
end