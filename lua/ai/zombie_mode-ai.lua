if sgs.GetConfig("GameMode", ""):match("zombie") then
	function sgs.ai_filterskill_filter.ganran(card, card_place)
		if card:getTypeId() == sgs.Card_Equip then
			return ("iron_chain:ganran[%s:%s]=%d"):format(
				card:getSuitString(),
				card:getNumberString(),
				card:getEffectiveId()
			)
		end
	end

	local ganran_skill = {}
	table.insert(sgs.ai_skills, ganran_skill)
	ganran_skill.name = "ganran"
	function ganran_skill.getTurnUseCard(self)
		local card = self:getCard("EquipCard")
		if card then return sgs.Card_Parse(sgs.ai_filterskill_filter.ganran(card)) end
	end

	local useTrickCard = SmartAI.useTrickCard
	function SmartAI:useTrickCard(card, use)
		if #self.enemies == 0 then
			if sgs.dynamic_value.damage_card[card:className()] then return end
			if sgs.dynamic_value.control_card[card:className()] then return end
			if sgs.dynamic_value.control_usecard[card:className()] then return end
		end
		useTrickCard(self, card, use)
	end

	local peaching_skill = {name = "peaching"}
	table.insert(sgs.ai_skills, peaching_skill)
	function peaching_skill.getTurnUseCard(self)
		local peach = self:getCardId("Analeptic") or self:getCardId("Peach") or self:getCardId("Shit")
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

	sgs.ai_skill_invoke.harbourage = true
	sgs.ai_skill_playerchosen.harbourage = function(self, targets)
		self:sort(self.friends_noself, "defense")
		local target = self.friends_noself[1] or self.player
		for _, friend in ipairs(self.friends) do
			if self:isWeak(friend) and not friend:hasMark("@harb") and friend:getGeneral2Name() ~= "zombie" then
				target = friend
				break
			end
		end
		return target
	end
end
