local lianhuan_skill={}
lianhuan_skill.name="lianhuan"
table.insert(sgs.ai_skills,lianhuan_skill)
lianhuan_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local card
	
	self:sortByUseValue(cards,true)
	
	for _,acard in ipairs(cards)  do
		if (acard:getSuit() == sgs.Card_Club) then--and (self:getUseValue(acard)<sgs.ai_use_value["IronChain"]) then
			card = acard
			break
		end
	end
	
	    if not card then return nil end
		local number = card:getNumberString()
	    local card_id = card:getEffectiveId()
	    local card_str = ("iron_chain:lianhuan[club:%s]=%d"):format(number, card_id)
		local skillcard = sgs.Card_Parse(card_str)
		
	    assert(skillcard)
        
        return skillcard
		
end

local huoji_skill={}
huoji_skill.name="huoji"
table.insert(sgs.ai_skills,huoji_skill)
huoji_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local card
	
	self:sortByUseValue(cards,true)
	
	for _,acard in ipairs(cards)  do
		if (acard:isRed()) and not acard:inherits("Peach") then--and (self:getUseValue(acard)<sgs.ai_use_value["FireAttack"]) then
			card = acard
			break
		end
	end
	
	    if not card then return nil end
	    local suit = card:getSuitString()
		local number = card:getNumberString()
	    local card_id = card:getEffectiveId()
	    local card_str = ("fire_attack:huoji[%s:%s]=%d"):format(suit, number, card_id)
		local skillcard = sgs.Card_Parse(card_str)
		
	    assert(skillcard)
        
        return skillcard
		
end

local shuangxiong_skill={}
shuangxiong_skill.name="shuangxiong"
table.insert(sgs.ai_skills,shuangxiong_skill)
shuangxiong_skill.getTurnUseCard=function(self)

    if not self.player:getMark("shuangxiong") then return nil end
    local mark=self.player:getMark("shuangxiong")
    
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local card
	
	self:sortByUseValue(cards,true)
	
	
	
	for _,acard in ipairs(cards)  do
		if (acard:isRed() and (mark==2)) or (acard:isBlack() and (mark==1)) then
			card = acard
			break
		end
	end
	
	    if not card then return nil end
	    local suit = card:getSuitString()
		local number = card:getNumberString()
	    local card_id = card:getEffectiveId()
	    local card_str = ("duel:shuangxiong[%s:%s]=%d"):format(suit, number, card_id)
		local skillcard = sgs.Card_Parse(card_str)
		
	    assert(skillcard)
        
        return skillcard
		
end

local tianyi_skill={}
tianyi_skill.name="tianyi"
table.insert(sgs.ai_skills,tianyi_skill)
tianyi_skill.getTurnUseCard=function(self)
    
    if self.tianyi_used then return nil end
    
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
    
    local max_card = self:getMaxCard()
    if not max_card then return nil end
	local max_point = max_card:getNumber()
	
	local slashNum=self:getSlashNumber(self.player)
	if max_card:inherits("Slash") then slashNum=slashNum-1 end
	
	if slashNum<2 then return nil end
	
	self:sort(self.enemies, "handcard")
	
	for _, enemy in ipairs(self.enemies) do
	
	    local enemy_max_card = self:getMaxCard(enemy)
		if enemy_max_card and max_point > enemy_max_card:getNumber() then
		    
		    local slash=self:getSlash()
		    local dummy_use={}
            dummy_use.isDummy=true
            
            local no_distance=true
		    self:useBasicCard(slash,dummy_use,no_distance)
		    
		    if not dummy_use.card then 
		        return nil
		    end
		    
		    local card_id = max_card:getEffectiveId()
			local card_str = "@TianyiCard=" .. card_id
			local card = sgs.Card_Parse(card_str)

		    return card
		end
	end
	
	self:sortByUseValue(cards, true)
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isKongcheng() then
		    local card_id = cards[1]:getEffectiveId()
			local card_str = "@TianyiCard=" .. card_id
			local card = sgs.Card_Parse(card_str)
			return card
		end
	end
end

sgs.ai_skill_use_func["TianyiCard"]=function(card,use,self)

	self:sort(self.enemies, "handcard")
	local max_card = self:getMaxCard(self.player)
	local max_point = max_card:getNumber()
	for _, enemy in ipairs(self.enemies) do
	    local enemy_max_card = self:getMaxCard(enemy)
		if not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) 
			and (enemy_max_card and max_point > enemy_max_card:getNumber()) then
		    
		    if use.to then 
		        self.tianyi_used = true
		        use.to:append(enemy)
		        
            end
            use.card=card
            break
		end
	end
end


--luanji
local luanji_skill={}
luanji_skill.name="luanji"
table.insert(sgs.ai_skills,luanji_skill)
luanji_skill.getTurnUseCard=function(self)
	local first_found, second_found = false, false
	local first_card, second_card
	if self.player:getHandcardNum() >= 2 then
		local cards = self.player:getHandcards()
		local same_suit=false
		cards = sgs.QList2Table(cards)
		for _, fcard in ipairs(cards) do
			if not (fcard:inherits("Peach") or fcard:inherits("ExNihilo")) then
				first_card = fcard
				first_found = true
				for _, scard in ipairs(cards) do
					if first_card ~= scard and scard:getSuitString() == first_card:getSuitString() and not (scard:inherits("Peach") or scard:inherits("ExNihilo")) then
						second_card = scard
						second_found = true
						break
					end
				end
				if second_card then break end
			end
		end
	end
	
	if first_found and second_found then
		local luanji_card = {}
		local first_suit, first_number, first_id = first_card:getSuitString(), first_card:getNumberString(), first_card:getId()
		local second_suit, second_number, second_id = second_card:getSuitString(), second_card:getNumberString(), second_card:getId()
		local card_str = ("archery_attack:luanji[%s:%s]=%d+%d"):format(first_suit, first_number, first_id, second_id)
		local archeryattack = sgs.Card_Parse(card_str)
		assert(archeryattack)
		return archeryattack
	end
end