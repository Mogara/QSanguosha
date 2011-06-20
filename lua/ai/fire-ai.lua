-- this scripts contains the AI classes for generals of fire package

-- wolong

-- bazhen
sgs.ai_skill_invoke.bazhen = true


-- pangtong

local pangtong_ai = SmartAI:newSubclass "pangtong"

-- niepan
sgs.ai_skill_invoke.niepan = function(self, data)
	local dying = data:toDying()
	local peaches = 1 - dying.who:getHp()

	local cards = self.player:getHandcards()
	local n = 0
	for _, card in sgs.qlist(cards) do
		if card:inherits "Peach" or card:inherits "Analeptic" then
			n = n + 1
		end
	end

	return n < peaches
end

-- lianhuan
function pangtong_ai:activate_dummy(use)
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getSuit() == sgs.Card_Club then
			local number = card:getNumber()
			local card_id = card:getEffectiveId()
			local card_str = ("iron_chain:lianhuan[club:%d]=%d"):format(number, card_id)
			local interlink = sgs.Card_Parse(card_str)
			self:useCardIronChain(interlink, use)
			return
		end
	end

	super.activate(self, use)
end

--luanji
local yuanshao_ai = SmartAI:newSubclass "yuanshao"
function yuanshao_ai:activate(use)

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
		self:useTrickCard(archeryattack, use)
		return
	end
	
	super.activate(self, use)
end

local xunyu_ai = SmartAI:newSubclass "xunyu"

function xunyu_ai:activate(use)
	if not self.player:hasUsed("QuhuCard") and not self.player:isKongcheng() then
		local max_card = self:getMaxCard()
		local max_point = max_card:getNumber()
		self:sort(self.enemies, "handcard")

		for _, enemy in ipairs(self.enemies) do
			if enemy:getHp() > self.player:getHp() then
				local enemy_max_card = self:getMaxCard(enemy)
				if enemy_max_card and max_point > enemy_max_card:getNumber() then
					for _, enemy2 in ipairs(self.enemies) do
						if (enemy:objectName() ~= enemy2:objectName()) and enemy:inMyAttackRange(enemy2) then
							local card_id = max_card:getEffectiveId()
							local card_str = "@QuhuCard=" .. card_id
							use.card = sgs.Card_Parse(card_str)
							use.to:append(enemy)						

							return
						end
					end
				end
			end
		end
	end

	super.activate(self, use)
end

function xunyu_ai:askForPlayerChosen(players, reason)
	if reason == "quhu" then
		for _, player in sgs.qlist(players) do
			if self:isEnemy(player) then
				return player
			end
		end
	end

	return super.askForPlayerChosen(self, players, reason)
end

sgs.ai_skill_use["@@jieming"] = function(self, prompt)
	self:sort(self.friends)

	local max_x = 0
	local target
	for _, friend in ipairs(self.friends) do
		local x = math.min(friend:getMaxHP(), 5) - friend:getHandcardNum()		

		if x > max_x then
			max_x = x
			target = friend
		end
	end

	if target then
		return "@JiemingCard=.->" .. target:objectName()
	else
		return "."
	end
end

-- mengjin
sgs.ai_skill_invoke.mengjin = function(self, data)
	local effect = data:toSlashEffect()
	return not self:isFriend(effect.to)
end

local dianwei_ai = SmartAI:newSubclass "dianwei"

function dianwei_ai:activate(use)
	super.activate(self, use)
	if use:isValid() then
		return
	end
	
	if not self.player:hasUsed("QiangxiCard") then
		local weapon = self.player:getWeapon()
		if weapon then
			local hand_weapon, cards
			cards = self.player:getHandcards()
			for _, card in sgs.qlist(cards) do
				if card:inherits("Weapon") then
					hand_weapon = card
					break
				end
			end

			self:sort(self.enemies)
			for _, enemy in ipairs(self.enemies) do
				if hand_weapon and self.player:inMyAttackRange(enemy) then
					use.card = sgs.Card_Parse("@QiangxiCard=" .. hand_weapon:getId())
					use.to:append(enemy)
					
					break
				end

				if self.player:distanceTo(enemy) <= 1 then
					use.card = sgs.Card_Parse("@QiangxiCard=" .. weapon:getId())
					use.to:append(enemy)

					return
				end
			end
		else
			self:sort(self.enemies, "hp")
			for _, enemy in ipairs(self.enemies) do
				if self.player:inMyAttackRange(enemy) and self.player:getHp() > enemy:getHp() and self.player:getHp() > 2 then
					use.card = sgs.Card_Parse("@QiangxiCard=.")
					use.to:append(enemy)

					return
				end
			end
		end
	end

end

--shuangxiong

sgs.ai_skill_invoke["shuangxiong"]=function(self,data)
    if self.player:isSkipped(sgs.Player_Play) or self.player:getHp() < 2 then
		return false
	end
    
    local cards=self.player:getCards("h")
    cards=sgs.QList2Table(cards)
    
    local handnum=0
    
    for _,card in ipairs(cards) do  
        if self:getUseValue(card)<8 then
			handnum=handnum+1
		end
    end
    
    handnum=handnum/2
    self:sort(self.enemies, "hp")
    for _, enemy in ipairs(self.enemies) do
        if (self:getSlashNumber(enemy)+enemy:getHp()<=handnum) and (self:getSlashNumber(self.player)>=self:getSlashNumber(enemy)) then return true end
    end
	
    return self.player:getHandcardNum()>=self.player:getHp()
end
