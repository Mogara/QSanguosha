sgs.ai_skill_invoke.moukui = function(self, data)
	local target = data:toPlayer()
    return not self:isFriend(target) 
end

sgs.ai_skill_choice.moukui = function(self, choices)
	return "discard"
end

sgs.ai_skill_invoke.tianming = function(self, data)
	if self:hasSkill("manjuan") and self.room:getCurrent() ~= self.player then return false end
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
	if handcardnum == 1 and trash and #self.enemies - count >= 2 and #self.friends_noself == 0 then
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
sgs.ai_card_intention.MizhaoCard = 20

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
	if self:isEquip("SilverLion", target) then return "." end
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
	if target_role == "renegade" or target_role == "unknown" then return false end
	local count1, count2 = 1, 0
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
	    if target_role == "loyalist" then count1 = count1 + 1 end
	    if target_role == "rebel" then count2 = count2 + 1 end
	end
	if self_role ~= "loyalist" and target_role == "loyalist" and count1 >= count2 then return true end
	if self_role ~= "rebel" and target_role == "rebel" and count1 < count2 then return true end
	return false
end

local mixin_skill={}
mixin_skill.name="mixin"
table.insert(sgs.ai_skills, mixin_skill)
mixin_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("MixinCard") or self.player:isKongcheng() then return end
	local cards = self.player:getHandcards()
	local allcard = {}
	cards = sgs.QList2Table(cards)
	
	local card
	
	self:sortByKeepValue(cards,true)
	
	card = cards[1]
	
	if not card then
		return nil
	end
	
	local card_id = card:getEffectiveId()
	local card_str = "@MixinCard="..card_id
	local skillcard = sgs.Card_Parse(card_str)
		
	assert(skillcard)
	return skillcard
end

sgs.ai_skill_use_func.MixinCard=function(card,use,self)
    for _, friend in ipairs(self.friends_noself) do
        if not friend:hasSkill("manjuan") then
			use.card = card
			if use.to then use.to:append(friend) end
			return
		end
    end
end

sgs.ai_skill_playerchosen.mixin = sgs.ai_skill_playerchosen.zero_card_as_slash

sgs.ai_use_priority.MixinCard = 0
sgs.ai_card_intention.MixinCard = 20

sgs.ai_skill_invoke.cangni = function(self, data)
	local target = self.room:getCurrent()
	if self.player:hasFlag("cangnilose") then return self:isEnemy(target) end
	if self.player:hasFlag("cangniget") then return self:isFriend(target) end
	local hand = self.player:getHandcardNum()
	local hp = self.player:getHp()
	return (hand + 2) <= hp or self.player:isWounded();
end

sgs.ai_skill_choice.cangni = function(self, choices)
	local hand = self.player:getHandcardNum()
	local hp = self.player:getHp()
	if (hand + 2) <= hp then
		return "draw"
	else
		return "recover"
	end
end

duyi_skill={}
duyi_skill.name="duyi"
table.insert(sgs.ai_skills, duyi_skill)
duyi_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("DuyiCard") then return end
	return sgs.Card_Parse("@DuyiCard=.")
end

sgs.ai_skill_use_func.DuyiCard=function(card,use,self)
	use.card = card
end

sgs.ai_skill_playerchosen.duyi=function(self, targets)
	local targetlist=sgs.QList2Table(targets)
	self:sort(targetlist,"hp")
	for _, target in sgs.qlist(targets) do
		if self:isFriend(target) and target:objectName() ~= self.player:objectName() then return target end
	end
	return targetlist[#targetlist]
end

sgs.ai_skill_invoke.duanzhi = function(self, data)
	local use = data:toCardUse()
	if self:isEnemy(use.from) and use.card:getSubtype() == "attack_card" and self.player:getHp() == 1 and not self:getCard("Peach") and not self:getCard("Analeptic") then
		return true
	end
	return self:isEnemy(use.from) and self.player:getHp() > 2
end

sgs.ai_skill_choice.duanzhi = function(self, choices)
	return "discard"
end

sgs.ai_skill_use["@@fengyin"] = function(self, data)
	local cards = self.player:getHandcards()
	local allcard = {}
	cards = sgs.QList2Table(cards)
	
	for _,acard in ipairs(cards)  do
		if acard:isKindOf("Slash") then
			card = acard
			break
		end
	end
	
	if not card then
		return "."
	end
	
	local card_id = card:getEffectiveId()
	
	local target = room:getCurrent()
	if self:isFriend(target) and target:containsTrick("indulgence") and target:getHandcardNum() + 2 > target:getHp() then
		return "@FengyinCard="..card_id
	end
	if self:isEnemy(target) and not target:containsTrick("indulgence") and target:getHandcardNum() >= target:getHp() then
		return "@FengyinCard="..card_id
	end
	return "."
end