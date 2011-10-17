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
		if card:inherits("Slash") then if self:getCardsNum("Slash") == 0 then return card:getEffectiveId() end
		elseif card:inherits("Jink") then if self:getCardsNum("Jink") == 0 then return card:getEffectiveId() end
		elseif card:inherits("Peach") then if self.player:isWounded() and self:getCardsNum("Peach") < self.player:getLostHp() then return card:getEffectiveId() end
		elseif card:inherits("Analeptic") then if self:getCardsNum("Analeptic") == 0 then return card:getEffectiveId() end
		elseif card:getTypeId() == sgs.Card_Trick then return card:getEffectiveId()
		else return -1 end
	end
	return -1 
end

--kuangfeng
sgs.ai_skill_use["@kuangfeng"]=function(self,prompt)
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
	local usecard=false
	if friendly_fire and is_chained > 1 then usecard=true end
	if target[1] then
		if target[1]:getArmor() and target[1]:getArmor():objectName() == "vine" then usecard=true end
	end
	if usecard then
		if not target[1] then table.insert(target,self.enemies[1]) end
		if target[1] then return "@KuangfengCard=.->" .. target[1]:objectName() else return "." end
	else
		return "."
	end
end
   
--dawu
sgs.ai_skill_use["@dawu"] = function(self, prompt)
	self:sort(self.friends, "hp")
	for _, friend in ipairs(self.friends) do
		if friend:getHp() == 1 and not friend:getArmor() and friend:getHandcardNum() <= friend:getHp() then
			return "@DawuCard=.->" .. friend:objectName()
		end
	end
	
	return "."
end

sgs.ai_skill_playerchosen.dawu = function(self, targets)
	for _, friend in sgs.qlist(targets) do
		if self:isFriend(friend) and friend:getHp() == 1 and not friend:getArmor() and friend:getHandcardNum() <= friend:getHp() then return friend end
	end
	
	return self.friends[1]
end

--wumou
sgs.ai_skill_choice.wumou = function(self, choices)
	if self.player:getHp() + self:getCardsNum("Peach") > 3 then return "losehp"
	else return "discard"
	end
end

--wuqian
local wuqian_skill={}
wuqian_skill.name = "wuqian"
table.insert(sgs.ai_skills, wuqian_skill)
wuqian_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("WuqianCard") or self.player:getMark("@wrath") < 2 then return end
	
	local card_str = ("@WuqianCard=.")
	self:sort(self.enemies, "hp")
	local has_enemy
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() <= 2 and self:getCardsNum("Jink", enemy) < 2 and self.player:inMyAttackRange(enemy) then has_enemy = enemy break end
	end
	
	if has_enemy and self:getCardsNum("Slash") > 0 then
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if card:inherits("Slash") and self:slashIsEffective(card, has_enemy) and 
				(self:getCardsNum("Analeptic") > 0 or has_enemy:getHp() <= 1) then return sgs.Card_Parse(card_str)
			elseif card:inherits("Duel") then return sgs.Card_Parse(card_str)
			end
		end
	end
end

sgs.ai_skill_use_func["WuqianCard"]=function(card,use,self)
    self:sort(self.enemies,"hp")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() <= 2 and self:getCardsNum("Jink", enemy) < 2 and self.player:inMyAttackRange(enemy) then 
			if use.to then 
				use.to:append(enemy)
			end
			use.card = card 
			return
		end
	end
end

--shenfen
local shenfen_skill={}
shenfen_skill.name = "shenfen"
table.insert(sgs.ai_skills, shenfen_skill)
shenfen_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("ShenfenCard") or self.player:getMark("@wrath") < 6 then return end
	return sgs.Card_Parse("@ShenfenCard=.")
end

sgs.ai_skill_use_func["ShenfenCard"]=function(card,use,self)
	use.card = card
end

--qinyin
sgs.ai_skill_invoke.qinyin = true

sgs.ai_skill_choice.qinyin = function(self, choices)
	self:sort(self.friends, "hp")
	self:sort(self.enemies, "hp")
	if self.friends[1]:getHp() >= self.enemies[1]:getHp() and self:getAllPeachNum(self.player) > self:getAllPeachNum(self.enemies[1]) then
		return "down"
	else
		return "up"
	end
end

--yeyan
local yeyan_skill={}
yeyan_skill.name = "yeyan"
table.insert(sgs.ai_skills, yeyan_skill)
yeyan_skill.getTurnUseCard=function(self)
    if self.player:getMark("@flame") == 0 then return end
	if self.player:getHandcardNum() >= 4 then
		local spade, club, heart, diamond
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if card:getSuit() == sgs.Card_Spade then spade = true
			elseif card:getSuit() == sgs.Card_Clue then club = true
			elseif card:getSuit() == sgs.Card_Heart then heart = true
			elseif card:getSuit() == sgs.Card_Diamond then diamond = true
			end
		end
		if spade and club and diamond and heart then
			self:sort(self.enemies, "hp")
			local target_num = 0
			for _, enemy in ipairs(self.enemies) do
				if (enemy:getArmor() and enemy:getArmor():objectName() == "vine") or enemy:isChained() then
					target_num = target_num + 1
				elseif enemy:getHp() <= 3 then
					target_num = target_num + 1
				end
			end
			
			if target_num == 1 then 
				return sgs.Card_Parse("@GreatYeyanCard=.")
			elseif target_num > 1 then
				return sgs.Card_Parse("@MediumYeyanCard=.")
			end
		end
	end
	
	if self.player:getHp() + self:getCardsNum("Peach") + self:getCardsNum("Analeptic") <= 1 then
		return sgs.Card_Parse("@SmallYeyanCard=.")
	end
end

sgs.ai_skill_use_func["SmallYeyanCard"]=function(card,use,self)
	local num = 0
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if use.to then use.to:append(enemy) end
		num = num + 1
		if num >= 3 then break end
	end
	use.card = card
end

sgs.ai_skill_use_func["MediumYeyanCard"]=function(card,use,self)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local need_cards = {}
	local spade, club, heart, diamond
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Spade and not spade then spade = true table.insert(need_cards, card)
		elseif card:getSuit() == sgs.Card_Clue and not club then club = true table.insert(need_cards, card)
		elseif card:getSuit() == sgs.Card_Heart and not heart then heart = true table.insert(need_cards, card)
		elseif card:getSuit() == sgs.Card_Diamond and not diamond then diamond = true table.insert(need_cards, card)
		end
	end
	if #need_cards < 4 then return end
	
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getArmor() and enemy:getArmor():objectName() == "vine" then
			if use.to then use.to:append(enemy) end
			break
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if enemy:isChained() then
			if use.to then use.to:append(enemy) end
			if use.to:length() == 2 then break end
		end
	end
	use.card = sgs.Card_Parse("@MediumYeyanCard=" .. table.concat(need_cards, "+"))
end

sgs.ai_skill_use_func["GreatYeyanCard"]=function(card,use,self)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local need_cards = {}
	local spade, club, heart, diamond
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Spade and not spade then spade = true table.insert(need_cards, card)
		elseif card:getSuit() == sgs.Card_Clue and not club then club = true table.insert(need_cards, card)
		elseif card:getSuit() == sgs.Card_Heart and not heart then heart = true table.insert(need_cards, card)
		elseif card:getSuit() == sgs.Card_Diamond and not diamond then diamond = true table.insert(need_cards, card)
		end
	end
	if #need_cards < 4 then return end
	
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if not (enemy:getArmor() and enemy:getArmor():objectName() == "silver_lion") then
			if use.to then use.to:append(enemy) end
			use.card = sgs.Card_Parse("@GreatYeyanCard=" .. table.concat(need_cards, "+"))
			return
		end
	end
end

sgs.ai_skill_invoke.lianpo = true

sgs.ai_skill_invoke.jilve=function(self,data)
	local struct
	local n=self.player:getMark("@bear")
	local use=(n>2 or self:getOverflow()>0)
	struct=data:toCard() 
	if not struct then
		struct=data:toCardUse()
		if struct then
			if not struct.card then struct=nil elseif struct.card:inherits("ExNihilo") then use=true end
		end
	end 
	if struct then return use end 
	struct=data:toDamage()
	if struct then if not struct.to==self.player then struct=nil end end
	if struct then return (use and self:askForUseCard("@@fangzhu","@fangzhu")~=".") end 
	struct=data:toJudge()
	if struct then if not struct.card then struct=nil end end
	if not struct then assert(false) end
	return (use and sgs.ai_skill_invoke["@guicai"](self,"dummyprompt",struct))
end

local jilve_skill={}
jilve_skill.name="jilve"
table.insert(sgs.ai_skills,jilve_skill)
jilve_skill.getTurnUseCard=function(self)
	if self.player:getMark("@bear")<1 then return end
	local zhiheng_skill
	for _, skill in ipairs(sgs.ai_skills) do
		if skill.name=="zhiheng" then zhiheng_skill=skill break end
	end
	local card=zhiheng_skill.getTurnUseCard(self)
	if card then return sgs.Card_Parse("@JilveCard=.") end
end

sgs.ai_skill_choice.jilve="zhiheng"

sgs.ai_skill_use_func["JilveCard"]=function(card,use,self)
	use.card = card
end

sgs.ai_skill_use["@zhiheng"]=function(self,prompt)
	for _, skill in ipairs(sgs.ai_skills) do
		if skill.name=="zhiheng" then zhiheng_skill=skill break end
	end
	local card=zhiheng_skill.getTurnUseCard(self)
	if card then return card:toString() .. "->." end
	return "."
end