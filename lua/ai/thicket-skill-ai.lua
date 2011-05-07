jiuchi_skill={}
jiuchi_skill.name="jiuchi"
table.insert(sgs.ai_skills,jiuchi_skill)
jiuchi_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local card
	
	self:sortByUseValue(cards,true)
	
	for _,acard in ipairs(cards)  do
		if (acard:getSuit() == sgs.Card_Spade) then --and (self:getUseValue(acard)<sgs.ai_use_value["Analeptic"]) then
			card = acard
			break
		end
	end
	
	    if not card then return nil end
		local number = card:getNumberString()
	    local card_id = card:getEffectiveId()
	    local card_str = ("analeptic:jiuchi[spade:%s]=%d"):format(number, card_id)
		local analeptic = sgs.Card_Parse(card_str)
		
	    assert(analeptic)
        
        return analeptic
		
end

duanliang_skill={}
duanliang_skill.name="duanliang"
table.insert(sgs.ai_skills,duanliang_skill)
duanliang_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("he")	
    cards=sgs.QList2Table(cards)
	
	local card
	
	self:sortByUseValue(cards,true)
	
	for _,acard in ipairs(cards)  do
		if (acard:isBlack()) and (acard:inherits("BasicCard") or acard:inherits("EquipCard")) then --and (self:getUseValue(acard)<(sgs.ai_use_value["SupplyShortage"] or 0)) then
			card = acard
			break
		end
	end
	
	    if not card then return nil end
	    local suit = card:getSuitString()
		local number = card:getNumberString()
	    local card_id = card:getEffectiveId()
	    local card_str = ("supply_shortage:duanliang[%s:%s]=%d"):format(suit, number, card_id)
		local skillcard = sgs.Card_Parse(card_str)
		
	    assert(skillcard)
        
        return skillcard
		
end


