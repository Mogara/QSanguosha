sgs.ai_skill_choice.RevealGeneral = function(self, choices)
	local event = self.player:getTag("event"):toInt()
	if event == sgs.Predamaged then
		local generals = self.player:getTag("roles"):toString():split("+")
		if #generals > 0 then
			for _, general in ipairs(generals) do
				local player = sgs.ServerPlayer(self.room)
				player:setGeneral(sgs.Sanguosha:getGeneral(general))
				if self:hasSkills(sgs.masochism_skill, player) then return "yes" end
			end
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