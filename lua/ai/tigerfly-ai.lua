--技能：奢靡
local shemi_skill = {}
shemi_skill.name = "shemi" --只保证能否发动
table.insert(sgs.ai_skills, shemi_skill)
shemi_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("ShemiAG") then return end
	local usenum = 2
	local usecard
	local hp = self.player:getHp()
	if hp <= 2 then usenum = 1 end
	local first_card, second_card 
	if self.player:getCards("he"):length() >= usenum then
		local flag = "he"
		if self:needToThrowArmor() and self.player:getArmor() then flag = "eh" 
			elseif self:getOverflow() > 0 then flag = "h" 
		end
		local cds = sgs.QList2Table(self.player:getCards(flag))
		local eid = self:getValuableCard(self.player) 
		self:sortByCardNeed(cds)
		for _, fcard in ipairs(cds) do
			if  not (fcard:isKindOf("Peach") or fcard:isKindOf("ExNihilo") or fcard:isKindOf("AmazingGrace")) and (fcard:getEffectiveId() ~= eid or not eid) then
				first_card = fcard
				if hp <= 2 then break end
				for _, scard in ipairs(cds) do
					if hp <= 2 then break end
					if first_card ~= scard and not (scard:isKindOf("Peach") or scard:isKindOf("ExNihilo") or scard:isKindOf("AmazingGrace")) and (scard:getEffectiveId() ~= eid or not eid) then
						second_card = scard
						local am = sgs.Sanguosha:cloneCard("amazing_grace", sgs.Card_NoSuit, 0)
						am:addSubcard(first_card)
						am:addSubcard(second_card)
						local dummy_use = {isDummy = true}
						self:useTrickCard(am, dummy_use)
						if not dummy_use.card then first_card=nil;second_card=nil break end	
					end
				end
				if first_card and second_card then break end
			end
		end
	end
	if usenum == 2 then
		if first_card and second_card then
		local first_id, second_id =  first_card:getId(), second_card:getId()
		local amazing = sgs.Card_Parse(("amazing_grace:shemi[to_be_decided:0]=%d+%d"):format(first_id, second_id))
		assert(amazing)
		return amazing
		end
	else	
		 usecard = first_card 
		 if usecard then
			local usecardid = usecard:getId()
			local amazing = sgs.Card_Parse(("amazing_grace:shemi[to_be_decided:0]=%d"):format(usecardid))
			assert(amazing)
			return amazing
		end
	end	
end
