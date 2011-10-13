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
		if (acard:isBlack()) and (acard:inherits("BasicCard") or acard:inherits("EquipCard")) and (self:getUseValue(acard)<(sgs.ai_use_value["SupplyShortage"] or 0)) then
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

dimeng_skill={}
dimeng_skill.name="dimeng"
table.insert(sgs.ai_skills,dimeng_skill)
dimeng_skill.getTurnUseCard=function(self)
    if self.dimeng_used then return nil end
    card=sgs.Card_Parse("@DimengCard=.")
    return card
    	
end

sgs.ai_skill_use_func["DimengCard"]=function(card,use,self)
    local cardNum=self.player:getHandcardNum()
	
	self:sort(self.enemies,"handcard")
	self:sort(self.friends_noself,"handcard")
	
	--local lowest_enemy=self.enemies[1]
	local lowest_friend=self.friends_noself[1]
	
	self:sort(self.enemies,"defense")
	if lowest_friend then
		for _,enemy in ipairs(self.enemies) do 
			local hand1=enemy:getHandcardNum()
			local hand2=lowest_friend:getHandcardNum()
			--local hand3=lowest_enemy:getHandcardNum()
	    
			if (hand1 > hand2) then 
				if (hand1-hand2)<=cardNum then 
					use.card=card
					if use.to then 
						use.to:append(enemy)
						use.to:append(lowest_friend)
						self.dimeng_used=true
						return 
					end
				end
			end
		end
	end
end

luanwu_skill={}
luanwu_skill.name="luanwu"
table.insert(sgs.ai_skills, luanwu_skill)
luanwu_skill.getTurnUseCard=function(self)
	if self.player:getMark("@chaos") <= 0 then return end
	local good, bad = 0, 0
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self:isWeak(player) then
			if self:isFriend(player) then bad = bad + 1
			else good = good + 1
			end
		end
	end
	if good == 0 then return end
	
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self:getAnalepticNum(player) > 0 then 
			if self:isFriend(player) then good = good + 1.0/player:getHp()
			else bad = bad + 1.0/player:getHp()
			end
		end
		
		local has_slash = self:getSlash(player)
		local can_slash = false
		if not can_slash then
			for _, p in sgs.qlist(self.room:getOtherPlayers(player)) do
				if player:inMyAttackRange(p) then can_slash = true break end
			end
		end
		if not has_slash or not can_slash then
			if self:isFriend(player) then good = good + math.max(self:getPeachNum(player), 1)
			else bad = bad + math.max(self:getPeachNum(player), 1)
			end
		end
		
		if self:getJinkNumber(player) == 0 then
			local lost_value = 0
			if self:hasSkills(sgs.masochism_skill, player) then lost_value = player:getHp()/2 end
			if self:isFriend(player) then bad = bad + (lost_value+1)/player:getHp()
			else good = good + (lost_value+1)/player:getHp()
			end
		end
	end
	
	if good > bad then return sgs.Card_Parse("@LuanwuCard=.") end
end

sgs.ai_skill_use_func["LuanwuCard"]=function(card,use,self)
	use.card = card
end
