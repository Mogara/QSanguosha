
sgs.ai_skill_choice.TriggerOrder = function(self, choices, data)
	if string.find(choices, "jieming") then return "jieming" end
	local choice = choices:split("+")[2]
	return choice
end

sgs.ai_skill_invoke["userdefine:halfmaxhp"] = true

sgs.ai_skill_choice.CompanionEffect = function(self, choice, data)
	if self:isWeak() and string.find(choice, "recover") then return "recover"
	else return "draw" end
end

sgs.ai_skill_choice.heg_nullification = function(self, choice, data)
	local effect = data:toCardEffect()
	if effect.card:isKindOf("AOE") then return "all" end
	return "single"
end