sgs.ai_skill_invoke["weapon_recast"] = function(self, data)
	if self.player:isLord() then 
		local card_use = data:toCardUse()
		if card_use.card:objectName() ~= "CrossBow" then return true else return false end 
	else
		if self.player:getWeapon() then return true else return false end
	end
end

sgs.ai_skill_invoke["draw_1v3"] = function(self, data)
	return not (self.player:hasSkill("kongcheng") and self.player:isKongcheng())
end

sgs.ai_skill_invoke.xiuluo = function(self, data)
	local hand_card = self.player:getHandcards()
	local judge_list = self.player:getCards("j")
	for _, judge in sgs.qlist(judge_list) do
		for _, card in sgs.qlist(hand_card) do
			if card:getSuit() == judge:getSuit() then return true end
		end
	end
	
	return false
end

