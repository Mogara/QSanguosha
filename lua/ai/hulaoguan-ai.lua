sgs.ai_skill_invoke["weapon_recast"] = function(self, data)
	if self:hasSkills(sgs.lose_equip_skill, self.player) then return false end
	if self.player:isLord() then 
		local card_use = data:toCardUse()
		if card_use.card:objectName() ~= "Crossbow" then return true else return false end 
	else
		if self.player:getWeapon() then return true else return false end
	end
end

sgs.ai_skill_invoke["draw_1v3"] = function(self, data)
	return not self:needKongcheng(self.player, true)
end

sgs.ai_skill_choice.Hulaopass = "recover"

sgs.ai_skill_cardask["@xiuluo"] = function(self, data, pattern)
	if self.player:containsTrick("YanxiaoCard") then return "." end
	if not self.player:containsTrick("indulgence") and not self.player:containsTrick("supply_shortage")
		and not (self.player:containsTrick("lightning") and not self:hasWizard(self.enemies)) then return "." end
	local indul_suit, ss_suit, lightning_suit = nil, nil, nil
	for _, card in sgs.qlist(self.player:getJudgingArea()) do
		if card:isKindOf("Indulgence") then indul_suit = card:getSuit() end
		if card:isKindOf("SupplyShortage") then ss_suit = card:getSuit() end
		if card:isKindOf("Lightning") then ss_suit = card:getSuit() end
	end
	if ss_suit then
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if card:getSuit() == ss_suit then return "$" .. card:getEffectiveId() end
		end
	elseif indul_suit then
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if card:getSuit() == indul_suit and not isCard("Peach", self.player, card) then return "$" .. card:getEffectiveId() end
		end
	elseif lightning_suit then
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if card:getSuit() == lightning_suit and not isCard("Peach", self.player, card) then return "$" .. card:getEffectiveId() end
		end
	end
	return "."
end

sgs.ai_skill_askforag.xiuluo = function(self, card_ids)
	for _, id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(id)
		if card:isKindOf("SupplyShortage") then return id end
	end
	for _, id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(id)
		if card:isKindOf("Indulgence") then return id end
	end
	if self:hasWizard(self.enemies) and self.player:containsTrick("lightning") then
		for _, id in ipairs(card_ids) do
			local card = sgs.Sanguosha:getCard(id)
			if card:isKindOf("Lightning") then return id end
		end
	end
	return card_ids[1]
end
