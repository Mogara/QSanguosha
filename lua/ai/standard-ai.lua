-- jianxiong
sgs.ai_skill_invoke.jianxiong = function(self, data)
        return not sgs.Shit_HasShit(data:toCard())
end

sgs.ai_skill_invoke.jijiang = function(self, data)
	if self:getCardsNum("Slash")<=0 then 
		return true
	end
	return false
end

sgs.ai_skill_choice.jijiang = function(self , choices)
	if not self.player:hasLordSkill("jijiang") then
		if self:getCardsNum("Slash") <= 0 then return "ignore" end
	end
	
	if self.player:isLord() then
		local target
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if player:hasSkill("weidi") then 
				target = player
				break
			end
		end
		if target and self:isEnemy(target) then return "ignore" end
    elseif self:isFriend(self.room:getLord()) then return "accept" end
    return "ignore"
end

sgs.ai_skill_choice.hujia = function(self , choices)
	if not self.player:hasLordSkill("hujia") then
		if self:getCardsNum("Jink") <= 0 then return "ignore" end
	end	
	if self.player:isLord() then
		local target
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if player:hasSkill("weidi") then 
				target = player
				break
			end
		end
		if target and self:isEnemy(target) then return "ignore" end
    elseif self:isFriend(self.room:getLord()) then return "accept" end
    return "ignore"
end

-- hujia
sgs.ai_skill_invoke.hujia = function(self, data)
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("Jink") then
			return false
		end
	end
	return true	
end

-- tuxi
sgs.ai_skill_use["@@tuxi"] = function(self, prompt)
	self:sort(self.enemies, "handcard")
	
	local first_index, second_index
	for i=1, #self.enemies-1 do																			
		if (self.enemies[i]:hasSkill("kongcheng") and self.enemies[i]:getHandcardNum() == 1) or
		   (self.enemies[i]:hasSkill("lianying") and self.enemies[i]:getHandcardNum() == 1) then 
		elseif not self.enemies[i]:isKongcheng() then
			if not first_index then 
				first_index = i 
			else 
				second_index = i 
			end
		end
		if second_index then break end
	end
	
	if first_index and not second_index then
		local others = self.room:getOtherPlayers(self.player)
		for _, other in sgs.qlist(others) do
			if (not self:isFriend(other) or (self:hasSkills(sgs.need_kongcheng, other) and other:getHandcardNum() == 1)) and 
				self.enemies[first_index]:objectName() ~= other:objectName() and not other:isKongcheng() then 
				return ("@TuxiCard=.->%s+%s"):format(self.enemies[first_index]:objectName(), other:objectName())
			end
		end
	end
	
	if not second_index then return "." end
	
	self:log(self.enemies[first_index]:getGeneralName() .. "+" .. self.enemies[second_index]:getGeneralName())
	local first = self.enemies[first_index]:objectName()
	local second = self.enemies[second_index]:objectName()
        --self:updateRoyalty(-0.8*sgs.ai_royalty[first],self.player:objectName())
        --self:updateRoyalty(-0.8*sgs.ai_royalty[second],self.player:objectName())
	return ("@TuxiCard=.->%s+%s"):format(first, second)
end

-- yiji (frequent)

-- tiandu, same as jianxiong
sgs.ai_skill_invoke.tiandu = sgs.ai_skill_invoke.jianxiong

-- ganglie
sgs.ai_skill_invoke.ganglie = function(self, data)
    local invoke=not self:isFriend(data:toPlayer())
    if invoke then
        --self:updateRoyalty(-0.8*sgs.ai_royalty[data:toPlayer():objectName()],self.player:objectName())
    end
    return invoke
end

-- fankui 
sgs.ai_skill_invoke.fankui = function(self, data) 
	local target = data:toPlayer()
	if self:isFriend(target) then
		return target:hasSkill("xiaoji") and not target:getEquips():isEmpty()	
	end
	if self:isEnemy(target) then				---fankui without zhugeliang and luxun
			if (target:hasSkill("kongcheng") or target:hasSkill("lianying")) and target:getHandcardNum() == 1 then
				if not target:getEquips():isEmpty() then return true
				else return false 
				end
			end
	end
                --self:updateRoyalty(-0.8*sgs.ai_royalty[target:objectName()],self.player:objectName())
	return true
end

-- tieji
sgs.ai_skill_invoke.tieji = function(self, data) 
	local effect = data:toSlashEffect()
	return not self:isFriend(effect.to) and (not effect.to:isKongcheng() or effect.to:getArmor())
end

sgs.ai_skill_use["@@liuli"] = function(self, prompt)
	
	local others=self.room:getOtherPlayers(self.player)												
	others=sgs.QList2Table(others)
	local source
	for _, player in ipairs(others) do 
		if player:hasFlag("slash_source") then
			source = player
			 break
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy,true) and not (source:objectName() == enemy:objectName()) then	
            local cards = self.player:getCards("he")
            cards=sgs.QList2Table(cards)
            for _,card in ipairs(cards) do
                if (self.player:getWeapon() and card:getId() == self.player:getWeapon():getId()) and self.player:distanceTo(enemy)>1 then local bullshit
                elseif card:inherits("OffensiveHorse") and self.player:getAttackRange()==self.player:distanceTo(enemy)
                    and self.player:distanceTo(enemy)>1 then
                    local bullshit
                else
                    return "@LiuliCard="..card:getEffectiveId().."->"..enemy:objectName()
                end
            end
		end
	end
	return "."
end

local liubei_ai=SmartAI:newSubclass "liubei"
function liubei_ai:activate(use)
    if self.player:usedTimes("RendeCard") < 2 then
		local cards = self.player:getHandcards()
		for _, friend in ipairs(self.friends_noself) do
			if friend:getHp() == 1 then
				for _, hcard in sgs.qlist(cards) do
					if hcard:inherits("Analeptic") or hcard:inherits("Peach") then 
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						use.to:append(friend)
						return
					end
				end
			end
			if friend:hasSkill("paoxiao") then
				for _, hcard in sgs.qlist(cards) do
					if hcard:inherits("Slash") then 
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						use.to:append(friend)
						return
					end
				end
			elseif friend:hasSkill("qingnang") and friend:getHp() < 2 and friend:getHandcardNum() < 1 then
				for _, hcard in sgs.qlist(cards) do
					if hcard:isRed() and not (hcard:inherits("ExNihilo") or hcard:inherits("Peach")) then 
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						use.to:append(friend)
						return
					end
				end
			elseif friend:hasSkill("jizhi") then
				for _, hcard in sgs.qlist(cards) do
					if hcard:getTypeId() == sgs.Card_Trick then 
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						use.to:append(friend)
						return
					end
				end
			elseif friend:hasSkill("guose") then
				for _, hcard in sgs.qlist(cards) do
					if hcard:getSuit() == sgs.Card_Diamond then 
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						use.to:append(friend)
						return
					end
				end
			elseif friend:hasSkill("leiji") then
				for _, hcard in sgs.qlist(cards) do
					if hcard:getSuit() == sgs.Card_Spade then 
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						use.to:append(friend)
						return
					end
				end
			elseif friend:hasSkill("xiaoji") then
				for _, hcard in sgs.qlist(cards) do
					if hcard:inherits("EquipCard") then 
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						use.to:append(friend)
						return
					end
				end
			end
			
		end
	end
	
	if (not use:isValid()) and (self.player:getHandcardNum()>=self.player:getHp()) then 
		if #self.friends_noself == 0 then return end
		
		self:sort(self.friends_noself, "handcard")
		local friend = self.friends_noself[1]
		local card_id = self:getCardRandomly(self.player, "h")
		if not sgs.Card_Parse(card_id):inherits("Shit") then
			use.card = sgs.Card_Parse("@RendeCard=" .. card_id)
			use.to:append(friend)
		end
		return
    end

	self:sort(self.enemies, "hp")
	
	if self:getCardId("Shit") and #self.enemies>0 then
		use.card=sgs.Card_Parse("@RendeCard=" .. self:getCardId("Shit"))
		use.to:append(self.enemies[1])
		return
	end
	super.activate(self, use)
end

local sunquan_ai = SmartAI:newSubclass "sunquan"
function sunquan_ai:activate(use)

	local unpreferedCards={}
	local cards=sgs.QList2Table(self.player:getHandcards())
	
	if not self.player:hasUsed("ZhihengCard") then 
		if self.player:getHp() < 3 then
			local zcards = self.player:getCards("he")
			for _, zcard in sgs.qlist(zcards) do
				if not zcard:inherits("Peach") and not zcard:inherits("ExNihilo") then
					table.insert(unpreferedCards,zcard:getId())
				end	
			end
		end
	end	
	
	if #unpreferedCards == 0 and not self.player:hasUsed("ZhihengCard") then 
		if self:getCardsNum("Slash")>1 then 
			self:sortByKeepValue(cards)
			for _,card in ipairs(cards) do
				if card:inherits("Slash") then table.insert(unpreferedCards,card:getId()) end
			end
			table.remove(unpreferedCards,1)
		end
		
		local num=self:getCardsNum("Jink")-1							
		if self.player:getArmor() then num=num+1 end
		if num>0 then
			for _,card in ipairs(cards) do
				if card:inherits("Jink") and num>0 then 
					table.insert(unpreferedCards,card:getId())
					num=num-1
				end
			end
		end
        for _,card in ipairs(cards) do
            if card:inherits("EquipCard") then
                if card:inherits("Weapon") or
                (card:inherits("DefensiveHorse") and self.player:getDefensiveHorse()) or
                card:inherits("OffensiveHorse") or
                (card:inherits("Armor") and self.player:getArmor()) or
                 card:inherits("AmazingGrace") or
                 card:inherits("Lightning") then
                    table.insert(unpreferedCards,card:getId())
                end
            end
        end
	
		if self.player:getWeapon() then														
			table.insert(unpreferedCards, self.player:getWeapon():getId())
		end
				
		if self.player:getArmor() and self.player:getArmor():objectName() == "silver_lion" and self.player:isWounded() then
			table.insert(unpreferedCards, self.player:getArmor():getId())
		end	
				
		local equips=self.player:getEquips()
		for _,equip in sgs.qlist(equips) do
			if equip:inherits("OffensiveHorse") and self.player:getWeapon() then
				table.insert(unpreferedCards, equip:getId())
				break
			end
		end	
	end	
	
	if #unpreferedCards>0 then 
		use.card = sgs.Card_Parse("@ZhihengCard="..table.concat(unpreferedCards,"+")) 
		return 
	end
	
	super.activate(self,use)
end

sgs.ai_skill_invoke["luoyi"]=function(self,data)
    local cards=self.player:getHandcards()
    cards=sgs.QList2Table(cards)

    for _,card in ipairs(cards) do
        if card:inherits("Slash") then

            for _,enemy in ipairs(self.enemies) do
                if self.player:canSlash(enemy, true) and
                self:slashIsEffective(card, enemy) and
                ( (not enemy:getArmor()) or (enemy:getArmor():objectName()=="renwang_shield") or (enemy:getArmor():objectName()=="vine") ) and
                enemy:getHandcardNum()< 2 then							
                    if not self.player:containsTrick("indulgence") then
						self:speak("luoyi")
                        return true
                    end
                end
            end
        end
    end
    return false
end


sgs.ai_skill_invoke["@guicai"]=function(self,prompt)
    local judge = self.player:getTag("Judge"):toJudge()
	
	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getHandcards())		
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return "@GuicaiCard=" .. card_id
		end
	end
	
	return "."
end
