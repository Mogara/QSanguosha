sgs.ai_skill_choice.RevealGeneral = function(self, choices)
	local event = self.player:getTag("event"):toInt()
	local data = self.player:getTag("event_data")
	local generals = self.player:getTag("roles"):toString():split("+")
	local players = {}
	for _, general in ipairs(generals) do
		local player = sgs.ServerPlayer(self.room)
		player:setGeneral(sgs.Sanguosha:getGeneral(general))
		table.insert(players, player)
	end
	
	if event == sgs.DamageInflicted then
		local damage = data:toDamage()
		for _, player in ipairs(players) do
			if self:hasSkills(sgs.masochism_skill, player) and self:isEnemy(damage.from, damage.to) then return "yes" end
		end
	elseif event == sgs.CardEffected then
		local effect = data:toCardEffect()
		for _, player in ipairs(players) do
			if self.room:isProhibited(effect.from, player, effect.card) and self:isEnemy(effect.from, effect.to) then return "yes" end
		end
	end

	local anjiang = 0
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:getGeneralName() == "anjiang" then
			anjiang = anjiang + 1
		end
	end
	if math.random() > (anjiang + 1)/(self.room:alivePlayerCount() + 1) then
		return "yes"
	else
		return "no"
	end
end