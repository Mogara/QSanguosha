    -- this script file contains the AI classes for gods

-- guixin, always invoke
sgs.ai_skill_invoke.guixin = true

-- shelie
sgs.ai_skill_invoke.shelie = true

local gongxin_skill={}
gongxin_skill.name="gongxin"
table.insert(sgs.ai_skills,gongxin_skill)
gongxin_skill.getTurnUseCard=function(self)
		local card_str = ("@GongxinCard=.")
		local gongxin_card = sgs.Card_Parse(card_str)
		assert(gongxin_card)
        return gongxin_card
end

sgs.ai_skill_use_func["GongxinCard"]=function(card,use,self)
    if self.player:usedTimes("GongxinCard")>0 then return end
    self:sort(self.enemies,"handcard")
    
    for _,enemy in ipairs(self.enemies) do  
        local cards = enemy:getHandcards()
			for _, acard in sgs.qlist(cards) do				
				if acard:getSuit() == sgs.Card_Heart and not acard:inherits("Shit") then
					use.card = card
					if use.to then use.to:append(enemy) end
					return
				end
			end
    end
end

local shenlubu_ai = SmartAI:newSubclass "shenlubu"

function shenlubu_ai:useTrickCard(card, use)
	if self.player:getMark("@wrath") > 0 then
		return super.useTrickCard(self, card, use)
	end
end

sgs.ai_skill_choice.wumou = "discard"


--wushen
wushen_skill={}
wushen_skill.name="wushen"
table.insert(sgs.ai_skills,wushen_skill)
wushen_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("he")	
    cards=sgs.QList2Table(cards)
	
	local red_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:getSuitString()=="heart" then--and (self:getUseValue(card)<sgs.ai_use_value["Slash"]) then
			red_card = card
			break
		end
	end

	if red_card then		
		local suit = red_card:getSuitString()
    	local number = red_card:getNumberString()
		local card_id = red_card:getEffectiveId()
		local card_str = ("slash:wushen[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
		
		assert(slash)
        
        return slash
	end
end

local shenguanyu_ai = SmartAI:newSubclass "shenguanyu"
function shenguanyu_ai:askForCard(pattern,prompt)
	local card = super.askForCard(self, pattern, prompt)
	if card then return card end
	if pattern == "slash" then
		local cards = self.player:getCards("h")
		cards=sgs.QList2Table(cards)
		self:fillSkillCards(cards)
        self:sortByUseValue(cards,true)
		for _, card in ipairs(cards) do
			if card:getSuit() == sgs.Card_Heart then
				local suit = card:getSuitString()
				local number = card:getNumberString()
				local card_id = card:getEffectiveId()
				return ("slash:wushen[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	end    
end

--qixing
sgs.ai_skill_askforag.qixing = function(self, card_ids)
	local cards = {}
	for _, card_id in ipairs(card_ids) do
		table.insert(cards, sgs.Sanguosha:getCard(card_id))
	end
	for _, card in ipairs(cards) do
		if card:inherits("Slash") then if self:getSlashNumber(self.player) == 0 then return card:getEffectiveId() end
		elseif card:inherits("Jink") then if self:getJinkNumber(self.player) == 0 then return card:getEffectiveId() end
		elseif card:inherits("Peach") then if self.player:isWounded() and self:getPeachNum() < self.player:getLostHp() then return card:getEffectiveId() end
		elseif card:inherits("Analeptic") then if self:getAnalepticNum(self.player) == 0 then return card:getEffectiveId() end
		elseif card:getTypeId() == sgs.Card_Trick then return card:getEffectiveId()
		else return -1 end
	end
	return -1 
end

--kuangfeng
sgs.ai_skill_invoke.kuangfeng = function(self, data)
	local friendly_fire
	for _, friend in ipairs(self.friends) do
		local cards = friend:getHandcards()
		for _, card in sgs.qlist(cards) do
			if (card:inherits("FireAttack") and friend:getHandcardNum() >= 4) or card:inherits("FireSlash") then
				friendly_fire = true
				break
			end
		end
		if friendly_fire then break end
	end
	
	local is_chained = 0
	local target = {}
	for _, enemy in ipairs(self.enemies) do	
		if enemy:isChained() then 
			is_chained = is_chained + 1 
			table.insert(target, enemy)
		end
		if enemy:getArmor() and enemy:getArmor():objectName() == "vine" then
			table.insert(target, 1, enemy)
			break
		end
	end
	if is_chained > 1 or (target[1]:getArmor() and target[1]:getArmor():objectName() == "vine") then return true 
	else return false
	end
end

sgs.ai_skill_playerchosen.kuangfeng = function(self, targets)
	for _, target in sgs.qlist(targets) do
		if self:isEnemy(target) and (target:isChained() or (target:getArmor() and target:getArmor():objectName() == "vine")) then
			return enemy
		end
	end
	
	return self.enemies[1]
end

--dawu
sgs.ai_skill_invoke.dawu = function(self, data)
	self:sort(self.friends, "hp")
	for _, friend in ipairs(self.friends) do
		if friend:getHp() == 1 and not friend:getArmor() and friend:getHandcardNum() <= friend:getHp() then return true end
	end
	
	return false
end

sgs.ai_skill_playerchosen.dawu = function(self, targets)
	for _, friend in sgs.qlist(targets) do
		if self:isFriend(friend) and friend:getHp() == 1 and not friend:getArmor() and friend:getHandcardNum() <= friend:getHp() then return friend end
	end
	
	return self.friends[1]
end
	
sgs.ai_skill_invoke.lianpo = true
