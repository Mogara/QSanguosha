-- danlao
sgs.ai_skill_invoke.danlao = function(self, data)
	local effect = data:toCardEffect()
	if effect.card:inherits "GodSalvation" and self.player:isWounded() then
		return false
	else
		return true
	end
end