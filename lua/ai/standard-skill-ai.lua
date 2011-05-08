
qixi_skill={}
qixi_skill.name="qixi"
table.insert(sgs.ai_skills,qixi_skill)
qixi_skill.getTurnUseCard=function(self,inclusive)
    local cards = self.player:getCards("he")	
    cards=sgs.QList2Table(cards)
	
	local black_card
	
	self:sortByUseValue(cards,true)
	
	local has_weapon=false
	
	for _,card in ipairs(cards)  do
	    if card:inherits("Weapon") and card:isRed() then has_weapon=true end
	end
	
	for _,card in ipairs(cards)  do
		if card:isBlack()  and ((self:getUseValue(card)<sgs.ai_use_value["Dismantlement"]) or inclusive) then
		    local shouldUse=true
		    
		    if card:inherits("Armor") then
                if not self.player:getArmor() then shouldUse=false 
                elseif self:hasEquip(card) then shouldUse=false
                end
            end
            
            if card:inherits("Weapon") then
                if not self.player:getWeapon() then shouldUse=false
                elseif self:hasEquip(card) and not has_weapon then shouldUse=false
                end
            end
		    
		    if shouldUse then
			    black_card = card
			    break
			end
			
		end
	end

	if black_card then
		local suit = black_card:getSuitString()
		local number = black_card:getNumberString()
		local card_id = black_card:getEffectiveId()
		local card_str = ("dismantlement:qixi[%s:%s]=%d"):format(suit, number, card_id)
		local dismantlement = sgs.Card_Parse(card_str)
		
		assert(dismantlement)
        
        return dismantlement
	end
end

wusheng_skill={}
wusheng_skill.name="wusheng"
table.insert(sgs.ai_skills,wusheng_skill)
wusheng_skill.getTurnUseCard=function(self,inclusive)
    local cards = self.player:getCards("he")	
    cards=sgs.QList2Table(cards)
	
	local red_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:isRed() and not card:inherits("Slash") and ((self:getUseValue(card)<sgs.ai_use_value["Slash"]) or inclusive) then
			red_card = card
			break
		end
	end

	if red_card then		
		local suit = red_card:getSuitString()
    	local number = red_card:getNumberString()
		local card_id = red_card:getEffectiveId()
		local card_str = ("slash:wusheng[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
		
		assert(slash)
        
        return slash
	end
end

longdan_skill={}
longdan_skill.name="longdan"
table.insert(sgs.ai_skills,longdan_skill)
longdan_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local jink_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:inherits("Jink") then--and (self:getUseValue(card)<sgs.ai_use_value["Slash"]) then
			jink_card = card
			break
		end
	end
	
	    if not jink_card then return nil end
		local suit = jink_card:getSuitString()
		local number = jink_card:getNumberString()
		local card_id = jink_card:getEffectiveId()
		local card_str = ("slash:longdan[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
	    assert(slash)
        
        return slash
		
end

jijiang_skill={}
jijiang_skill.name="jijiang"
table.insert(sgs.ai_skills,jijiang_skill)
jijiang_skill.getTurnUseCard=function(self)
        if not self:slashIsAvailable() then return nil end
        if self.jijiang_used then return nil end
		local card_str = "@JijiangCard=."
		local slash = sgs.Card_Parse(card_str)
	    assert(slash)
        
        return slash
		
end

sgs.ai_skill_use_func["JijiangCard"]=function(card,use,self)
    
	self:sort(self.enemies, "defense")
		local target_count=0
                for _, enemy in ipairs(self.enemies) do
                        if ((self.player:canSlash(enemy, not no_distance)) or 
                        (use.isDummy and (self.player:distanceTo(enemy)<=self.predictedRange)))
                                 and
                                self:objectiveLevel(enemy)>3 and
                                self:slashIsEffective(card, enemy) then

                                use.card=card
                                if use.to then 
                                    use.to:append(enemy) 
                                    self.jijiang_used=true
                                end
                                target_count=target_count+1
                                if self.slash_targets<=target_count then return end
                        end
               end
	
end

guose_skill={}
guose_skill.name="guose"
table.insert(sgs.ai_skills,guose_skill)
guose_skill.getTurnUseCard=function(self,inclusive)
    local cards = self.player:getCards("he")	
    cards=sgs.QList2Table(cards)
	
	local card
	
	self:sortByUseValue(cards,true)
	
	local has_weapon=false
	
	for _,acard in ipairs(cards)  do
	    if acard:inherits("Weapon") and not (acard:getSuit() == sgs.Card_Diamond) then has_weapon=true end
	end
	
	for _,acard in ipairs(cards)  do
		if (acard:getSuit() == sgs.Card_Diamond) and ((self:getUseValue(acard)<sgs.ai_use_value["Indulgence"]) or inclusive) then
		    local shouldUse=true
		    
		    if acard:inherits("Armor") then
                if not self.player:getArmor() then shouldUse=false 
                elseif self:hasEquip(acard) then shouldUse=false
                end
            end
            
            if acard:inherits("Weapon") then
                if not self.player:getWeapon() then shouldUse=false
                elseif self:hasEquip(acard) and not has_weapon then shouldUse=false
                end
            end
		    
		    if shouldUse then
			    card = acard
			    break
			end
		end
	end
	
	    if not card then return nil end
		local number = card:getNumberString()
	    local card_id = card:getEffectiveId()
		local card_str = ("indulgence:guose[diamond:%s]=%d"):format(number, card_id)	
		local indulgence = sgs.Card_Parse(card_str)
		
	    assert(indulgence)
        
        return indulgence
		
end

