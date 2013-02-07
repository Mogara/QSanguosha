if sgs.GetConfig("GameMode", ""):match("zombie") then
	function sgs.ai_filterskill_filter.ganran(card, card_place)
		if card:getTypeId() == sgs.Card_Equip then
			local str = string.format("iron_chain:ganran[%s:%s]=%d", 
				card:getSuitString(),
				card:getNumberString(),
				card:getEffectiveId()
			)
			return str
		end
	end
	
	local ganran_skill = {}
	ganran_skill.name = "ganran"
	table.insert(sgs.ai_skills, ganran_skill)
	function ganran_skill.getTurnUseCard(self)
		local cards = self.player:getCards("he")
		local card = nil
		for _,cd in sgs.qlist(cards) do
			if cd:isKindOf("EquipCard") or cd:isKindOf("GanranEquip") then
				card = cd
				break
			end
		end
		--local card = self:getCard("EquipCard")
		if card then 
			local suit = card:getSuitString()
			local point = card:getNumberString()
			local id = card:getEffectiveId()
			local card_str = string.format("iron_chain:ganran[%s:%s]=%d", suit, point, id)
			local acard = sgs.Card_Parse(card_str)
			assert(acard)
			return acard
		end
	end

	local useTrickCard = SmartAI.useTrickCard
	function SmartAI:useTrickCard(card, use)
		if #self.enemies == 0 then
			if sgs.dynamic_value.damage_card[card:getClassName()] then return end
			if sgs.dynamic_value.control_card[card:getClassName()] then return end
			if sgs.dynamic_value.control_usecard[card:getClassName()] then return end
		end
		useTrickCard(self, card, use)
	end

	local peaching_skill = {name = "peaching"}
	table.insert(sgs.ai_skills, peaching_skill)
	function peaching_skill.getTurnUseCard(self)
		local peach = self:getCardId("Peach")
		if peach and type(peach) == "number" then return sgs.Card_Parse("@PeachingCard=" .. peach) end
	end
	
	function sgs.ai_skill_use_func.PeachingCard(card, use, self)
		self:sort(self.friends, "hp")
		for _, friend in ipairs(self.friends) do
			if friend:isWounded() and self.player:distanceTo(friend) <= 1 then
				use.card = card
				if use.to then use.to:append(friend) end
				return
			end
		end
	end
end
