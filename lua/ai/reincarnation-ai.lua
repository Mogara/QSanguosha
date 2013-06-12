-- reincarnation
sgs.ai_card_intention.SacrificeCard = -40

local sacrifice_skill={}
sacrifice_skill.name = "sacrifice"
table.insert(sgs.ai_skills, sacrifice_skill)
sacrifice_skill.getTurnUseCard = function(self)
	if not sgs.GetConfig("EnableReincarnation", false) then return end
	if not self.player:hasUsed("SacrificeCard") and not self.player:isKongcheng() then
		local deathnote = {}
		for _, aplayer in sgs.qlist(self.room:getAllPlayers(true)) do
			if aplayer:isDead() then
				table.insert(deathnote, aplayer:objectName())
			end
		end
		if #deathnote == 0 then return end
		for _, name in ipairs(deathnote) do
			local target = self.room:findPlayer(name, true)
			if self:isFriend(target) then
				local renegade = true
				if not self.room:getScenario() and not self.room:getMode() == "06_3v3" and target:getRole() == "renegade" then
					renegade = false
				end
				if renegade then
					local cards = sgs.QList2Table(self.player:getCards("h"))
					self:sortByUseValue(cards, true)
					if self:getUseValue(cards[1]) < 3 then
						self.xjtarget = target:getGeneralName()
						self.xjcard = cards[1]
						return sgs.Card_Parse("@SacrificeCard=.")
					end
				end
			end
		end
	end
end
sgs.ai_skill_use_func["SacrificeCard"] = function(card,use,self)
	use.card=card
end
sgs.ai_skill_choice["sacrifice"] = function(self, choice)
	return self.xjtarget
end
sgs.ai_cardshow["sacrifice"] = function(self, requestor)
	return self.xjcard
end
