sgs.ai_skill_invoke.ice_sword=function(self, data)
	if self.player:hasFlag("drank") then return false end
	local effect = data:toSlashEffect() 
	local target = effect.to
	if self:isFriend(target) then return false end
	local hasPeach
	local cards = target:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("Peach") or card:inherits("Analeptic") then hasPeach = true break end
	end
	if hasPeach then return true end
	if (target:getHandcardNum() > 1 or target:getArmor()) and target:getHp() > 1 then
		return true
	end
	return false
end

sgs.ai_skill_cardchosen.ice_sword = function(self, who)
	local hcards = who:getCards("h")
	hcards = sgs.QList2Table(hcards)
	for _, peach in ipairs(hcards) do
		if peach:inherits("Peach") or peach:inherits("Analeptic") then return peach:getId() end
	end
end

local spear_skill={}
spear_skill.name="spear"
table.insert(sgs.ai_skills,spear_skill)
spear_skill.getTurnUseCard=function(self,inclusive)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
    
    if #cards<(self.player:getHp()+1) then return nil end
    if #cards<2 then return nil end
    if self:getCard("Slash") then return nil end
    
    self:sortByUseValue(cards,true)
    
    local suit1 = cards[1]:getSuitString()
	local card_id1 = cards[1]:getEffectiveId()
	
	local suit2 = cards[2]:getSuitString()
	local card_id2 = cards[2]:getEffectiveId()
	
	local suit="no_suit"
	if cards[1]:isBlack() == cards[2]:isBlack() then suit = suit1 end
	
	local card_str = ("slash:spear[%s:%s]=%d+%d"):format(suit, 0, card_id1, card_id2)
	
    local slash = sgs.Card_Parse(card_str)
    
    return slash
    
end

local qixi_skill={}
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

local wusheng_skill={}
wusheng_skill.name="wusheng"
table.insert(sgs.ai_skills,wusheng_skill)
wusheng_skill.getTurnUseCard=function(self,inclusive)
    local cards = self.player:getCards("he")	
    cards=sgs.QList2Table(cards)
	
	local red_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:isRed() and not card:inherits("Slash") and not card:inherits("Peach") 				--not peach
			and ((self:getUseValue(card)<sgs.ai_use_value["Slash"]) or inclusive) then
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

local longdan_skill={}
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


local fanjian_skill={}
fanjian_skill.name="fanjian"
table.insert(sgs.ai_skills,fanjian_skill)
fanjian_skill.getTurnUseCard=function(self)
        if self.player:isKongcheng() then return nil end
        if self.player:usedTimes("FanjianCard")>0 then return nil end
		
		local cards = self.player:getHandcards()
		
		for _, card in sgs.qlist(cards) do
		--	if card:getSuit() == sgs.Card_Diamond or card:inherits("Peach") or card:inherits("Analeptic") then		
			if card:getSuit() == sgs.Card_Diamond and self.player:getHandcardNum() == 1 then
				return nil
			elseif card:inherits("Peach") or card:inherits("Analeptic") then
				return nil
			end
		end
		
		
		local card_str = "@FanjianCard=."
		local fanjianCard = sgs.Card_Parse(card_str)
	    assert(fanjianCard)
        
        return fanjianCard
		
end

sgs.ai_skill_use_func["FanjianCard"]=function(card,use,self)
	self:sort(self.enemies, "hp")
			
			for _, enemy in ipairs(self.enemies) do								
				if (not enemy:hasSkill("qingnang")) or (enemy:getHp() == 1 and enemy:getHandcardNum() == 0 and not enemy:getEquips()) then
					use.card = card
					if use.to then use.to:append(enemy) end
					
					return
				end
			end
end

local jieyin_skill={}
jieyin_skill.name="jieyin"
table.insert(sgs.ai_skills,jieyin_skill)
jieyin_skill.getTurnUseCard=function(self)
        if self.player:getHandcardNum()<2 then return nil end
        if self.player:usedTimes("JieyinCard")>0 then return nil end
		
		local cards = self.player:getHandcards()
		cards=sgs.QList2Table(cards)
		
		self:sortByUseValue(cards,true)
		
		local first  = cards[1]:getEffectiveId()
		local second = cards[2]:getEffectiveId()

		local card_str = ("@JieyinCard=%d+%d"):format(first, second)
		return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func["JieyinCard"]=function(card,use,self)
	self:sort(self.friends, "hp")
	
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() and friend:isWounded() then
			use.card=card
			if use.to then use.to:append(friend) end
			return
		end
	end
end

local qingnang_skill={}
qingnang_skill.name="qingnang"
table.insert(sgs.ai_skills,qingnang_skill)
qingnang_skill.getTurnUseCard=function(self)
        if self.player:getHandcardNum()<1 then return nil end
        if self.player:usedTimes("QingnangCard")>0 then return nil end
		
		local cards = self.player:getHandcards()
		cards=sgs.QList2Table(cards)
		
		self:sortByKeepValue(cards)

		local card_str = ("@QingnangCard=%d"):format(cards[1]:getId())
		return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func["QingnangCard"]=function(card,use,self)
	self:sort(self.friends, "defense")
	
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() then
			use.card=card
			if use.to then use.to:append(friend) end
			return
		end
	end
end

local kurou_skill={}
kurou_skill.name="kurou"
table.insert(sgs.ai_skills,kurou_skill)
kurou_skill.getTurnUseCard=function(self,inclusive)
        if  (self.player:getHp() > 3 and self.player:getHandcardNum() > self.player:getHp()) or		
		(self.player:getHp() - self.player:getHandcardNum() >= 2) then
                return sgs.Card_Parse("@KurouCard=.")
        end
		
		--if not inclusive then return nil end
		
	if self.player:getWeapon() and self.player:getWeapon():inherits("Crossbow") then
        for _, enemy in ipairs(self.enemies) do
            if self.player:canSlash(enemy,true) and self.player:getHp()>1 then
                return sgs.Card_Parse("@KurouCard=.")
            end
        end
    end
end

sgs.ai_skill_use_func["KurouCard"]=function(card,use,self)
	
	if not use.isDummy then self:speak("kurou") end
	
	use.card=card
end

local jijiang_skill={}
jijiang_skill.name="jijiang"
table.insert(sgs.ai_skills,jijiang_skill)
jijiang_skill.getTurnUseCard=function(self)
        if self.player:hasUsed("JijiangCard") or not self:slashIsAvailable() then return end
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
                                end
                                target_count=target_count+1
                                if self.slash_targets<=target_count then return end
                        end
               end
	
end

local rende_skill={}
rende_skill.name="jijiang"
table.insert(sgs.ai_skills,rende_skill)
rende_skill.getTurnUseCard=function(self)
        local cards = self.player:getHandcards()	
		cards=sgs.QList2Table(cards)
		
		for _,acard in ipairs(cards)  do
			
		end
end

sgs.ai_skill_use_func["RendeCard"]=function(card,use,self)
    
	
end

local guose_skill={}
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


local lijian_skill={}
lijian_skill.name="lijian"
table.insert(sgs.ai_skills,lijian_skill)
lijian_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("LijianCard") then
		return 
	end
	if not self.player:isNude() then
		local card
		local card_id
		if self.player:getArmor() and self.player:isWounded() and self.player:getArmor():objectName()=="silver_lion" then
			card = sgs.Card_Parse("@LijianCard=" .. self.player:getArmor():getId())
		elseif self.player:getHandcardNum() > self.player:getHp() then
			local cards = self.player:getHandcards()
			cards=sgs.QList2Table(cards)
			
			for _, acard in ipairs(cards) do
				if (acard:inherits("BasicCard") or acard:inherits("EquipCard") or acard:inherits("AmazingGrace"))
					and not acard:inherits("Peach") and not acard:inherits("Shit") then 
					card_id = acard:getEffectiveId()
					break
				end
			end
		elseif not self.player:getEquips():isEmpty() then
			local player=self.player
			if player:getWeapon() then card_id=player:getWeapon():getId()
			elseif player:getOffensiveHorse() then card_id=player:getOffensiveHorse():getId()
			elseif player:getDefensiveHorse() then card_id=player:getDefensiveHorse():getId()
			elseif player:getArmor() and player:getHandcardNum()<=1 then card_id=player:getArmor():getId()
			end
		end
		if not card_id then
			cards=sgs.QList2Table(self.player:getHandcards())
			for _, acard in ipairs(cards) do
				if (acard:inherits("BasicCard") or acard:inherits("EquipCard") or acard:inherits("AmazingGrace"))
					and not acard:inherits("Peach") and not acard:inherits("Shit") then 
					card_id = acard:getEffectiveId()
					break
				end
			end
		end
		if not card_id then
			return nil
		else
			card = sgs.Card_Parse("@LijianCard=" .. card_id)
			return card
		end
	end
	return nil
end

sgs.ai_skill_use_func["LijianCard"]=function(card,use,self)
	local findFriend_maxSlash=function(self,first)
		self:log("Looking for the friend!")
		local maxSlash = 0
		local friend_maxSlash
		for _, friend in ipairs(self.friends_noself) do
			if (self:getCardsNum("Slash", friend)> maxSlash) and friend:getGeneral():isMale() then
				maxSlash=self:getCardsNum("Slash", friend)
				friend_maxSlash = friend
			end
		end
		if friend_maxSlash then
			local safe = false
			if (first:hasSkill("ganglie") or first:hasSkill("fankui") or first:hasSkill("enyuan")) then
				if (first:getHp()<=1 and first:getHandcardNum()==0) then safe=true end
			elseif (self:getCardsNum("Slash", friend_maxSlash) >= first:getHandcardNum()) then safe=true end
			if safe then return friend_maxSlash end
		else self:log("unfound")
		end
		return nil
	end

	if not self.player:hasUsed("LijianCard") then
		self:sort(self.enemies, "hp")
		local males = {}
		local first, second
		local zhugeliang_kongcheng
		for _, enemy in ipairs(self.enemies) do
			if zhugeliang_kongcheng and #males==1 then table.insert(males, zhugeliang_kongcheng) end
			if enemy:getGeneral():isMale() and not enemy:hasSkill("wuyan") then
				if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then	zhugeliang_kongcheng=enemy
				else table.insert(males, enemy)	end
				if #males >= 2 then	break end
			end
		end
		if (#males==1) and #self.friends_noself>0 then
			self:log("Only 1")
			first = males[1]
			if zhugeliang_kongcheng then table.insert(males, zhugeliang_kongcheng)
			else
				local friend_maxSlash = findFriend_maxSlash(self,first)
				if friend_maxSlash then table.insert(males, friend_maxSlash) end
			end
		end
		if (#males >= 2) then
			first = males[1]
			second = males[2]
			local lord = self.room:getLord()
			if (first:getHp()<=1) then
				if self.player:isLord() or isRolePredictable() then 
					local friend_maxSlash = findFriend_maxSlash(self,first)
					if friend_maxSlash then second=friend_maxSlash end
				elseif (lord:getGeneral():isMale()) and (not lord:hasSkill("wuyan")) then 
					if (self.role=="rebel") and (not first:isLord()) then
						second = lord
					else
						if ((self.role=="loyalist" or (self.role=="renegade") and not (first:hasSkill("ganglie") and first:hasSkill("enyuan"))))
							and	( self:getCardsNum("Slash", first)<=self:getCardsNum("Slash", second)) then
							second = lord
						end
					end
				end
			end

			if first and second 
					then
				if use.to
					then 
		        use.to:append(first)
		        use.to:append(second)
				end
			end
            use.card=card
		end
	end
end

