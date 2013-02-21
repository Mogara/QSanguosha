--[[
	技能：缓释
	描述：（3V3局）在一名己方角色的判定牌生效前，你可以打出一张牌代替之。
		（身份局）在一名角色的判定牌生效前，你可以令其选择是否由你打出一张牌代替之。
]]--
sgs.ai_skill_cardask["@huanshi-card"] = function(self, data) --询问缓释改判卡牌
	local judge = data:toJudge()

	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getCards("he"))
		local card_id = self:getRetrialCardId(cards, judge)
		local card = sgs.Sanguosha:getCard(card_id)
		if card_id ~= -1 then
			return "@HuanshiCard[" .. card:getSuitString() .. ":" .. card:getNumberString() .. "]=" .. card_id
		end
	end

	return "."
end

sgs.ai_skill_invoke.huanshi = true

sgs.ai_skill_choice.huanshi = function(self, choices)
	local zhugejin = self.room:findPlayerBySkillName("huanshi")
	if self:objectiveLevel(zhugejin) > 2 then return "reject" end
	return "accept"
end

function sgs.ai_cardneed.huanshi(to, card, self)
	for _, player in ipairs(self.friends) do
		if self:getFinalRetrial(to) == 1 then 
			if self:willSkipDrawPhase(player) then
				return card:getSuit() == sgs.Card_Club and not self:hasSuit("club", true, to)
			end
			if self:willSkipPlayPhase(player) then
				return card:getSuit() == sgs.Card_Heart and not self:hasSuit("heart", true, to)
			end
		end
	end
end

function sgs.ai_cardneed.mingzhe(to, card, self)
	return card:isRed() and (getKnownCard(to, "heart", false) + getKnownCard(to, "diamond", false)) < 2
end

sgs.ai_skill_invoke.hongyuan = function(self, data)
	local count = 0
	for i = 1, #self.friends_noself do
		if self:needKongcheng(self.friends_noself[i]) and self.friends_noself[i]:getHandcardNum() == 0
			or self.friends_noself[i]:hasSkill("manjuan") then
		else
			count = count + 1
		end
		if count == 2 then return true end
	end
	return false
end

sgs.ai_skill_invoke.mingzhe = true

sgs.ai_suit_priority.mingzhe=function(self)	
	return self.player:getPhase()==sgs.Player_NotActive and "diamond|heart|club|spade" or "club|spade|diamond|heart"
end
--[[
	技能：弘援
	描述：（3V3局）摸牌阶段，你可以少摸一张牌，令其他己方角色各摸一张牌。
		（身份局）摸牌阶段，你可以少摸一张牌，令一至两名其他角色各摸一张牌。
]]--
sgs.ai_skill_use["@@hongyuan"] = function(self, prompt)
	if self:needBear() then return "." end
	self:sort(self.friends_noself, "handcard")
	local first_index, second_index
	for i=1, #self.friends_noself do
		if self:needKongcheng(self.friends_noself[i]) and self.friends_noself[i]:getHandcardNum() == 0 
			or self.friends_noself[i]:hasSkill("manjuan") then
		else
			if not first_index then
				first_index = i
			else
				second_index = i
			end
		end
		if second_index then break end
	end

	if first_index and not second_index then
		local others = self.room:getOtherPlayers(self.player)
		for _, other in sgs.qlist(others) do
			if (not self:isFriend(other) and (self:needKongcheng(other) and others:getHandcardNum() == 0 or other:hasSkill("manjuan"))) and
				self.friends_noself[first_index]:objectName() ~= other:objectName() then
				return ("@HongyuanCard=.->%s+%s"):format(self.friends_noself[first_index]:objectName(), other:objectName())
			end
		end
	end

	if not second_index then return "." end

	self:log(self.friends_noself[first_index]:getGeneralName() .. "+" .. self.friends_noself[second_index]:getGeneralName())
	local first = self.friends_noself[first_index]:objectName()
	local second = self.friends_noself[second_index]:objectName()
	return ("@HongyuanCard=.->%s+%s"):format(first, second)
end

sgs.ai_card_intention.HongyuanCard = function(card, from, tos, source)
	for _, to in ipairs(tos) do
		sgs.updateIntention(from, to, -80)
	end
end

sgs.ai_suit_priority.mingzhe=function(self)	
	return self.player:getPhase()==sgs.Player_NotActive and "diamond|heart|club|spade" or "club|spade|diamond|heart"
end

sgs.huanshi_suit_value = {
	heart = 3.9,
	diamond = 3.4,
	club = 3.9,
	spade = 3.5
}

sgs.mingzhe_suit_value = {
	heart = 4.0,
	diamond = 4.0
}