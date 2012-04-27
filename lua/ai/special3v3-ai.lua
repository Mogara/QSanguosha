sgs.ai_skill_cardask["@huanshi-card"] = function(self, data)
	local judge = data:toJudge()

	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getCards("he"))
		local card_id = self:getRetrialCardId(cards, judge)
		local card = sgs.Sanguosha:getCard(card_id)
		if card_id ~= -1 then
			return "@HuanshiCard[" .. card:getSuitString() .. ":" .. card:getNumberString() .. "]=" .. card_id

	end

	return "."
end

sgs.ai_skill_invoke.hongyuan = function(self, data)
	return 	self.player:getHandcardNum() > 0
end

sgs.ai_skill_invoke.mingzhe = true

