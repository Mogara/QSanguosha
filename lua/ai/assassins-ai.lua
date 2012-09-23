sgs.ai_skill_invoke.moukui = function(self, data)
	local target = data:toPlayer()
    return not self:isFriend(target) 
end

sgs.ai_skill_choice.moukui = function(self, choices)
	return "discard"
end

sgs.ai_skill_invoke.tianming = function(self, data)
	if self:getCardsNum("Jink") == 0 then return true end
    local unpreferedCards={}
    local cards=sgs.QList2Table(self.player:getHandcards())
    
    local zcards = self.player:getCards("he")
    for _, zcard in sgs.qlist(zcards) do
        if not zcard:isKindOf("Peach") and not zcard:isKindOf("ExNihilo") then
            table.insert(unpreferedCards,zcard:getId())
        end	
    end
    
    if #unpreferedCards == 0 then 
        if self:getCardsNum("Slash")>1 then 
            self:sortByKeepValue(cards)
            for _,card in ipairs(cards) do
                if card:isKindOf("Slash") then table.insert(unpreferedCards,card:getId()) end
            end
            table.remove(unpreferedCards, 1)
        end
        
        local num=self:getCardsNum("Jink") - 1	
        if self.player:getArmor() then num=num+1 end
        if num>0 then
            for _,card in ipairs(cards) do
                if card:isKindOf("Jink") and num>0 then 
                    table.insert(unpreferedCards,card:getId())
                    num=num-1
                end
            end
        end
        for _,card in ipairs(cards) do
            if (card:isKindOf("Weapon") and self.player:getHandcardNum() < 3) or card:isKindOf("OffensiveHorse") or
                self:getSameEquip(card, self.player) or	card:isKindOf("AmazingGrace") or card:isKindOf("Lightning") then
                table.insert(unpreferedCards,card:getId())
            end
        end
    
        if self.player:getWeapon() and self.player:getHandcardNum()<3 then
            table.insert(unpreferedCards, self.player:getWeapon():getId())
        end
                
	    if (self:isEquip("SilverLion") and self.player:isWounded()) then
            table.insert(unpreferedCards, self.player:getArmor():getId())
        end	

        if self.player:getOffensiveHorse() and self.player:getWeapon() then
            table.insert(unpreferedCards, self.player:getOffensiveHorse():getId())
        end
    end	
    
    for index = #unpreferedCards, 1, -1 do
        if self.player:isJilei(sgs.Sanguosha:getCard(unpreferedCards[index])) then table.remove(unpreferedCards, index) end
    end
    
    if #unpreferedCards >= 2 or #unpreferedCards == #cards then 
        return true
    end
end

sgs.ai_skill_discard.tianming = function(self, discard_num, min_num, optional, include_equip)
    local unpreferedCards={}
    local cards=sgs.QList2Table(self.player:getHandcards())
    
    local zcards = self.player:getCards("he")
    for _, zcard in sgs.qlist(zcards) do
        if not zcard:isKindOf("Peach") and not zcard:isKindOf("ExNihilo") then
            table.insert(unpreferedCards,zcard:getId())
        end	
    end
    
    if #unpreferedCards == 0 then 
        if self:getCardsNum("Slash")>1 then 
            self:sortByKeepValue(cards)
            for _,card in ipairs(cards) do
                if card:isKindOf("Slash") then table.insert(unpreferedCards,card:getId()) end
            end
            table.remove(unpreferedCards, 1)
        end
        
        local num=self:getCardsNum("Jink") - 1	
        if self.player:getArmor() then num=num+1 end
        if num>0 then
            for _,card in ipairs(cards) do
                if card:isKindOf("Jink") and num>0 then 
                    table.insert(unpreferedCards,card:getId())
                    num=num-1
                end
            end
        end
        for _,card in ipairs(cards) do
            if (card:isKindOf("Weapon") and self.player:getHandcardNum() < 3) or card:isKindOf("OffensiveHorse") or
                self:getSameEquip(card, self.player) or	card:isKindOf("AmazingGrace") or card:isKindOf("Lightning") then
                table.insert(unpreferedCards,card:getId())
            end
        end
    
        if self.player:getWeapon() and self.player:getHandcardNum()<3 then
            table.insert(unpreferedCards, self.player:getWeapon():getId())
        end
                
	    if (self:isEquip("SilverLion") and self.player:isWounded()) then
            table.insert(unpreferedCards, self.player:getArmor():getId())
        end	

        if self.player:getOffensiveHorse() and self.player:getWeapon() then
            table.insert(unpreferedCards, self.player:getOffensiveHorse():getId())
        end
    end	
    
    for index = #unpreferedCards, 1, -1 do
        if self.player:isJilei(sgs.Sanguosha:getCard(unpreferedCards[index])) then table.remove(unpreferedCards, index) end
    end
    
    local to_discard = {}
	local count = 0
	for index = #unpreferedCards, 1, -1 do
	    table.insert(to_discard, unpreferedCards[index])
		count = count + 1
		if count == 2 then return to_discard end
    end
	return to_discard
end

local mizhao_skill={}
mizhao_skill.name="mizhao"
table.insert(sgs.ai_skills, mizhao_skill)
mizhao_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("MizhaoCard") or self.player:isKongcheng() then return end
	local cards = self.player:getHandcards()
	local allcard = {}
	cards = sgs.QList2Table(cards)
	for _,card in ipairs(cards)  do
		table.insert(allcard,card:getId()) 
	end
	local parsed_card = sgs.Card_Parse("@MizhaoCard=" .. table.concat(allcard,"+"))
	return parsed_card
end

sgs.ai_skill_use_func.MizhaoCard=function(card,use,self)
	local handcardnum = self.player:getHandcardNum()
	local trash = self:getCard("Disaster") or self:getCard("GodSalvation") or self:getCard("AmazingGrace")
	local count = 0
	for _, enemy in ipairs(self.enemies) do
		if enemy:isKongcheng() then count = count + 1 end
	end
	if handcardnum == 1 and trash and #self.enemies - count >= 2 then
		self:sort(self.enemies, "hp")
		use.card = card
        if use.to then use.to:append(self.enemies[1]) end
		return
	end
    self:sort(self.friends_noself, "hp")
    for _, friend in ipairs(self.friends_noself) do
        if not friend:hasSkill("manjuan") then
			use.card = card
			if use.to then use.to:append(friend) end
			return
		end
    end
end

sgs.ai_use_priority.MizhaoCard = 1.5

sgs.ai_skill_playerchosen.mizhao = function(self, targets)
	self:sort(self.enemies, "hp")
    for _, enemy in ipairs(self.enemies) do
		if targets:contains(enemy) then return enemy end
	end
end

function sgs.ai_skill_pindian.mizhao(minusecard, self, requestor, maxcard)
	local cards, maxcard = sgs.QList2Table(self.player:getHandcards())
	local function compare_func1(a, b)
		return a:getNumber() > b:getNumber()
	end
	local function compare_func2(a, b)
		return a:getNumber() < b:getNumber()
	end
	if self:isFriend(requestor) and self.player:getHp() > requestor:getHp() then
		table.sort(cards, compare_func2)
	else
		table.sort(cards, compare_func1)
	end
	for _, card in ipairs(cards) do
		if self:getUseValue(card) < 6 then maxcard = card break end
	end
	return maxcard or cards[1]
end

sgs.ai_skill_cardask["@JieyuanIncrease"] = function(self, data)
	local damage = data:toDamage()
	local target = damage.to
	if self:isFriend(target) then return "." end
	if target:getArmor() and target:getArmor():getClassName() == "SilverLion" then return "." end
	local cards=sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _,card in ipairs(cards) do
		if card:isBlack() then return "$" .. card:getEffectiveId() end
	end
	return "."
end

sgs.ai_skill_cardask["@JieyuanDecrease"] = function(self, data)
	local damage = data:toDamage()
	if self:hasSkills(sgs.masochism_skill) and damage.damage <= 1 and self.player:getHp() > 1 then return "." end
	local cards=sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _,card in ipairs(cards) do
		if card:isRed() then return "$" .. card:getEffectiveId() end
	end
	return "."
end

sgs.ai_skill_invoke.fenxin = function(self, data)
	local target = data:toPlayer()
    local target_role = sgs.evaluatePlayerRole(target)
	local self_role = self.player:getRole()
	if target_role == "renegade" then return false end
	local count1, count2 = 1, 0
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
	    if sgs.evaluatePlayerRole(target) == "loyalist" then count1 = count1 + 1 end
		if sgs.evaluatePlayerRole(target) == "rebel" then count2 = count2 + 1 end
	end
	if self_role ~= "loyalist" and count1 > count2 then return true end
	if self_role ~= "rebel" and count1 < count2 then return true end
	return false
end