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
		if card then return sgs.ai_filterskill_filter.ganran(card) end
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
end