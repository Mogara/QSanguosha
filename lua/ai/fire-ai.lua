local quhu_skill={}
quhu_skill.name="quhu"
table.insert(sgs.ai_skills,quhu_skill)
quhu_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("QuhuCard") and not self.player:isKongcheng() then
		local max_card = self:getMaxCard()
		return sgs.Card_Parse("@QuhuCard=" .. max_card:getEffectiveId())
	end
end

sgs.ai_skill_use_func.QuhuCard = function(card, use, self)
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
						if use.to then
							use.to:append(enemy)
						end
						use.card = sgs.Card_Parse(card_str)
						return
					end
				end
			end
		end
	end
	if not self.player:isWounded() or (self.player:getHp() == 1 and self:getCardsNum("Analeptic") > 0) then
		local use_quhu
		for _, friend in ipairs(self.friends) do
			if math.min(5, friend:getMaxHP()) - friend:getHandcardNum() >= 2 then
				self:sort(self.enemies, "handcard")
				if self.enemies[#self.enemies]:getHandcardNum() > 0 then use_quhu = true break end
			end
		end
		if use_quhu then
			for _, enemy in ipairs(self.enemies) do
				if not enemy:isKongcheng() and self.player:getHp() < enemy:getHp() then
					local cards = self.player:getHandcards()
					cards = sgs.QList2Table(cards)
					self:sortByUseValue(cards, true)
					local card_id = cards[1]:getEffectiveId()
					local card_str = "@QuhuCard=" .. card_id
					if use.to then
						use.to:append(enemy)
					end
					use.card = sgs.Card_Parse(card_str)
					return
				end
			end
		end
	end
end

local quhu_filter = function(player, carduse)
	if carduse.card:inherits("QuhuCard") then
		sgs.ai_quhu_effect = true
	end
end

table.insert(sgs.ai_choicemade_filter.cardUsed, quhu_filter)

sgs.ai_cardneed.quhu = sgs.ai_cardneed.bignumber
sgs.ai_skill_playerchosen.quhu = sgs.ai_skill_playerchosen.damage
sgs.ai_playerchosen_intention.quhu = 80

sgs.ai_card_intention.QuhuCard = 30

sgs.dynamic_value.control_card.QuhuCard = true

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

sgs.ai_card_intention.JiemingCard =-80

sgs.ai_chaofeng.xunyu = 3

local qiangxi_skill={}
qiangxi_skill.name="qiangxi"
table.insert(sgs.ai_skills,qiangxi_skill)
qiangxi_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("QiangxiCard") then
		return sgs.Card_Parse("@QiangxiCard=.")
	end
end

sgs.ai_skill_use_func.QiangxiCard = function(card, use, self)
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
				if use.to then
					use.to:append(enemy)
				end
				break
			end
			if self.player:distanceTo(enemy) <= 1 then
				use.card = sgs.Card_Parse("@QiangxiCard=" .. weapon:getId())
				if use.to then
					use.to:append(enemy)
				end
				return
			end
		end
	else
		self:sort(self.enemies, "hp")
		for _, enemy in ipairs(self.enemies) do
			if self.player:inMyAttackRange(enemy) and self.player:getHp() > enemy:getHp() and self.player:getHp() > 2 then
				use.card = sgs.Card_Parse("@QiangxiCard=.")
				if use.to then
					use.to:append(enemy)
				end
				return
			end
		end
	end
end

sgs.ai_use_value.QiangxiCard = 2.5

sgs.ai_card_intention.QiangxiCard = 80

sgs.dynamic_value.damage_card.QiangxiCard = true

sgs.ai_chaofeng.dianwei = 2

local huoji_skill={}
huoji_skill.name="huoji"
table.insert(sgs.ai_skills,huoji_skill)
huoji_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)

	local card

	self:sortByUseValue(cards,true)

	for _,acard in ipairs(cards)  do
		if (acard:isRed()) and not acard:inherits("Peach") then--and (self:getUseValue(acard)<sgs.ai_use_value.FireAttack) then
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

sgs.ai_view_as.kanpo = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place ~= sgs.Player_Equip then
		if card:isBlack() then
			return ("nullification:kanpo[%s:%s]=%d"):format(suit, number, card_id)
		end
	end
end

sgs.ai_skill_invoke.bazhen = sgs.ai_skill_invoke.eight_diagram

function sgs.ai_armor_value.bazhen(card)
	if not card then return 4 end
end

sgs.wolong_suit_value = 
{
	spade = 3.9,
	club = 3.9
}

local lianhuan_skill={}
lianhuan_skill.name="lianhuan"
table.insert(sgs.ai_skills,lianhuan_skill)
lianhuan_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)

	local card

	self:sortByUseValue(cards,true)

	for _,acard in ipairs(cards)  do
		if (acard:getSuit() == sgs.Card_Club) then--and (self:getUseValue(acard)<sgs.ai_use_value.IronChain) then
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

sgs.ai_chaofeng.pangtong = -1

local tianyi_skill={}
tianyi_skill.name="tianyi"
table.insert(sgs.ai_skills,tianyi_skill)
tianyi_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("TianyiCard") and not self.player:isKongcheng() then return sgs.Card_Parse("@TianyiCard=.") end
end

sgs.ai_skill_use_func.TianyiCard=function(card,use,self)
	local zhugeliang = self.room:findPlayerBySkillName("kongcheng")
	if zhugeliang and self:isFriend(zhugeliang) and zhugeliang:getHandcardNum() == 1 and zhugeliang:objectName()~=self.player:objectName() then
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByUseValue(cards,true)
		use.card = sgs.Card_Parse("@TianyiCard=" .. cards[1]:getId())
		if use.to then use.to:append(zhugeliang) end
		return
	end
	
	self:sort(self.enemies, "handcard")
	local max_card = self:getMaxCard(self.player)
	local max_point = max_card:getNumber()
	local slashcount = self:getCardsNum("Slash")
	if max_card:inherits("Slash") then slashcount = slashcount - 1 end
	if self.player:hasSkill("kongcheng") and self.player:getHandcardNum()==1 then
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isKongcheng() then
				use.card = sgs.Card_Parse("@TianyiCard=" .. max_card:getId())
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end
	if slashcount > 1 or (slashcount == 1 and #self.enemies > 1) then
		local slash = self:getCard("Slash")
		assert(slash)
		local dummy_use = {isDummy = true}
		self:useBasicCard(slash, dummy_use)
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) and not enemy:isKongcheng() then
				local h = enemy:getHandcardNum()
				local poss = ((max_card:getNumber() - 1)/13)^h
				if math.random() < poss then
					use.card = sgs.Card_Parse("@TianyiCard=" .. max_card:getId())
					if use.to then use.to:append(enemy) end
					return
				end
			end
		end
		if dummy_use.card then
			self:sort(self.friends_noself,"handcard")
			for index = #self.friends_noself, 1, -1 do
				local friend = self.friends_noself[index]
				if not friend:isKongcheng() then
					local h = friend:getHandcardNum()
					local poss = ((14-max_card:getNumber())/13)^h
					if math.random() > poss then
						use.card = sgs.Card_Parse("@TianyiCard=" .. max_card:getId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			end
		end
	end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)
	if self:getUseValue(cards[1]) >= 6 then return end
	local shouldUse = (slashcount == 0)
	if slashcount > 0 then
		local slash = self:getCard("Slash")
		assert(slash)
		local dummyuse = {isDummy = true}
		self:useBasicCard(slash, dummyuse)
		if not dummyuse.card then shouldUse = true end
	end
	if shouldUse then
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) and not enemy:isKongcheng() and not enemy:hasSkill("tuntian") then
				use.card = sgs.Card_Parse("@TianyiCard=" .. cards[1]:getId())
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end
end

function sgs.ai_skill_pindian.tianyi(minusecard, self, requestor)
	if self:isFriend(requestor) then return end
	if requestor:getHandcardNum() <= 2 then return minusecard end
end

sgs.ai_cardneed.tianyi = sgs.ai_cardneed.bignumber

sgs.ai_card_intention.TianyiCard = 30

sgs.dynamic_value.control_card.TianyiCard = true

sgs.ai_use_value.TianyiCard = 8.5
sgs.ai_use_priority.TianyiCard = 4.2

sgs.ai_chaofeng.taishici = 3

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
			if not (fcard:inherits("Peach") or fcard:inherits("ExNihilo") or fcard:inherits("AOE")) then
				first_card = fcard
				first_found = true
				for _, scard in ipairs(cards) do
					if first_card ~= scard and scard:getSuitString() == first_card:getSuitString() and 
						not (scard:inherits("Peach") or scard:inherits("ExNihilo") or scard:inherits("AOE")) then
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

sgs.ai_chaofeng.yuanshao = 1

sgs.ai_skill_invoke.shuangxiong=function(self,data)
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
		if (self:getCardsNum("Slash", enemy)+enemy:getHp()<=handnum) and (self:getCardsNum("Slash")>=self:getCardsNum("Slash", enemy)) then return true end
	end

	return self.player:getHandcardNum()>=self.player:getHp()
end

local shuangxiong_skill={}
shuangxiong_skill.name="shuangxiong"
table.insert(sgs.ai_skills,shuangxiong_skill)
shuangxiong_skill.getTurnUseCard=function(self)

	if not self.player:getMark("shuangxiong") then return nil end
	local mark=self.player:getMark("shuangxiong")

	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
	
	local card
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

sgs.ai_chaofeng.shuangxiong = 1

sgs.ai_skill_invoke.mengjin = function(self, data)
	local effect = data:toSlashEffect()
	return not self:isFriend(effect.to)
end
