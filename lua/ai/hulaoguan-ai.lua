sgs.ai_skill_invoke["weapon_recast"] = function(self, data)
	if self.player:isLord() then 
		local card_use = data:toCardUse()
		if card_use.card:objectName() ~= "crossbow" then return true else return false end 
	else
		if self.player:getWeapon() then return true else return false end
	end
end

sgs.ai_skill_invoke["draw_1v3"] = function(self, data)
	return not (self.player:hasSkill("kongcheng") and self.player:isKongcheng())
end

sgs.ai_skill_cardask["@xiuluo"] = function(self)
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if card:getSuitString() == parsedPrompt[2] then return "$"..card:getEffectiveId() end
	end
	return "."
end
