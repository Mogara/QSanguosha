sgs.ai_skill_cardask["@huanshi-card"] = function(self, data)
	local judge = data:toJudge()

	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getCards("he"))
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return "@HuanshiCard=" .. card_id
		end
	end

	return "."
end

sgs.ai_skill_invoke.hongyuan = function(self, data)
	return 	self.player:getHandcardNum() > 0 
end

sgs.ai_skill_invoke.mingzhe = true

sgs.ai_skill_use["@@hongyuan"] = function(self, prompt)
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
	if not second_index then return "." end

	self:log(self.friends_noself[first_index]:getGeneralName() .. "+" .. self.friends_noself[second_index]:getGeneralName())
	local first = self.friends_noself[first_index]:objectName()
	local second = self.friends_noself[second_index]:objectName()
	return ("@HongyuanCard=.->%s+%s"):format(first, second)
end