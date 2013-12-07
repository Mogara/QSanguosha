local quhu_skill = {}
quhu_skill.name = "quhu"
table.insert(sgs.ai_skills, quhu_skill)
quhu_skill.getTurnUseCard = function(self)
	if self:needBear() then return end
	if not self.player:hasUsed("QuhuCard") and not self.player:isKongcheng() then
		local max_card = self:getMaxCard()
		return sgs.Card_Parse("@QuhuCard=" .. max_card:getEffectiveId())
	end
end

sgs.ai_skill_use_func.QuhuCard = function(card, use, self)
	local max_card = self:getMaxCard()
	local max_point = max_card:getNumber()

	if #self.enemies == 0 then return end
	self:sort(self.enemies, "handcard")

	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() > self.player:getHp() and not enemy:isKongcheng() then
			local enemy_max_card = self:getMaxCard(enemy)
			local allknown = 0
			if self:getKnownNum(enemy) == enemy:getHandcardNum() then
				allknown = allknown + 1
			end
			if (enemy_max_card and max_point > enemy_max_card:getNumber() and allknown > 0)
				or (enemy_max_card and max_point > enemy_max_card:getNumber() and allknown < 1 and max_point > 10) 
				or (not enemy_max_card and max_point > 10) then
				for _, enemy2 in ipairs(self.enemies) do
					if (enemy:objectName() ~= enemy2:objectName())
						and enemy:distanceTo(enemy2) <= enemy:getAttackRange() then
						self.quhu_card = max_card:getEffectiveId()
						use.card = sgs.Card_Parse("@QuhuCard=.")
						if use.to then use.to:append(enemy) end
						return
					end
				end
			end
		end
	end
	if (not self.player:isWounded() or (self.player:getHp() == 1 and self:getCardsNum("Analeptic") > 0 and self.player:getHandcardNum() >= 2))
	  and self.player:hasSkill("jieming") then
		local use_quhu
		for _, friend in ipairs(self.friends) do
			if math.min(5, friend:getMaxHp()) - friend:getHandcardNum() >= 2 then
				self:sort(self.enemies, "handcard")
				if self.enemies[#self.enemies]:getHandcardNum() > 0 then use_quhu = true break end
			end
		end
		if use_quhu then
			for _, enemy in ipairs(self.enemies) do
				if not enemy:isKongcheng() and self.player:getHp() < enemy:getHp() and not enemy:hasSkill("jueqing") then
					local cards = self.player:getHandcards()
					cards = sgs.QList2Table(cards)
					self:sortByUseValue(cards, true)
					self.quhu_card = cards[1]:getEffectiveId()
					use.card = sgs.Card_Parse("@QuhuCard=.")
					if use.to then use.to:append(enemy) end
					return
				end
			end
		end
	end
end
local quhu_filter = function(self, player, carduse)
	if carduse.card:isKindOf("QuhuCard") then
		sgs.ai_quhu_effect = true
	end
end

table.insert(sgs.ai_choicemade_filter.cardUsed, quhu_filter)

sgs.ai_cardneed.quhu = sgs.ai_cardneed.bignumber
sgs.ai_skill_playerchosen.quhu = sgs.ai_skill_playerchosen.damage
sgs.ai_playerchosen_intention.quhu = 80

sgs.ai_card_intention.QuhuCard = 0
sgs.dynamic_value.control_card.QuhuCard = true

sgs.ai_skill_playerchosen.jieming = function(self, targets)
	local friends = {}
	for _, player in ipairs(self.friends) do
		if player:isAlive() and not hasManjuanEffect(player) then
			table.insert(friends, player)
		end
	end
	self:sort(friends)
	
	local max_x = 0
	local target
	local Shenfen_user
	for _, player in sgs.qlist(self.room:getAlivePlayers()) do
		if player:hasFlag("ShenfenUsing") then
			Shenfen_user = player
			break
		end
	end
	if Shenfen_user then
		local y, weak_friend = 3
		for _, friend in ipairs(friends) do
			local x = math.min(friend:getMaxHp(), 5) - friend:getHandcardNum()
			if friend:hasSkill("manjuan") and x > 0 then x = x + 1 end
			if friend:getMaxHp() >=5 and x > max_x and friend:isAlive() then
				max_x = x
				target = friend
			end
			
			if self:playerGetRound(friend, Shenfen_user) > self:playerGetRound(self.player, Shenfen_user) and x >= y
				and friend:getHp() == 1 and getCardsNum("Peach", friend) < 1 then
				y = x
				weak_friend = friend
			end
		end
		
		if weak_friend and ((getCardsNum("Peach", Shenfen_user) < 1) or (math.min(Shenfen_user:getMaxHp(), 5) - Shenfen_user:getHandcardNum() <= 1)) then
			return weak_friend
		end
		if self:isFriend(Shenfen_user) and math.min(Shenfen_user:getMaxHp(), 5) > Shenfen_user:getHandcardNum() then
			return Shenfen_user
		end
		if target then return target end
	end
	
	local CP = self.room:getCurrent()
	local max_x = 0
	local AssistTarget = self:AssistTarget()
	for _, friend in ipairs(friends) do
		local x = math.min(friend:getMaxHp(), 5) - friend:getHandcardNum()
		if friend:hasSkill("manjuan") then x = x + 1 end
		if self:hasCrossbowEffect(CP) then x = x + 1 end
		if AssistTarget and friend:objectName() == AssistTarget:objectName() then x = x + 0.5 end
		
		if x > max_x and friend:isAlive() then
			max_x = x
			target = friend
		end
	end

	return target
end

sgs.ai_need_damaged.jieming = function (self, attacker, player)
	return player:hasSkill("jieming") and self:getJiemingChaofeng(player) <= -6
end

sgs.ai_playerchosen_intention.jieming = function(self, from, to)
	if to:getHandcardNum() < math.min(5, to:getMaxHp()) then
		sgs.updateIntention(from, to, -80)
	end
end

sgs.ai_chaofeng.xunyu = 3

local qiangxi_skill = {}
qiangxi_skill.name= "qiangxi"
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
			if card:isKindOf("Weapon") then
				hand_weapon = card
				break
			end
		end
		self:sort(self.enemies)
		self.equipsToDec = hand_weapon and 0 or 1
		for _, enemy in ipairs(self.enemies) do
			if self:objectiveLevel(enemy) > 3 and not self:cantbeHurt(enemy) and self:damageIsEffective(enemy) then
				if hand_weapon and self.player:distanceTo(enemy) <= self.player:getAttackRange() then
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
		end
		self.equipsToDec = 0
	else
		self:sort(self.enemies, "hp")
		for _, enemy in ipairs(self.enemies) do
			if self:objectiveLevel(enemy) > 3 and not self:cantbeHurt(enemy) and self:damageIsEffective(enemy) then
				if self.player:distanceTo(enemy) <= self.player:getAttackRange() and self.player:getHp() > enemy:getHp() and self.player:getHp() > 1 then
					use.card = sgs.Card_Parse("@QiangxiCard=.")
					if use.to then
						use.to:append(enemy)
					end
					return
				end
			end
		end
	end
end

sgs.ai_use_value.QiangxiCard = 2.5
sgs.ai_card_intention.QiangxiCard = 80
sgs.dynamic_value.damage_card.QiangxiCard = true
sgs.ai_cardneed.qiangxi = sgs.ai_cardneed.weapon

sgs.qiangxi_keep_value = {
	Peach = 6,
	Jink = 5.1,
	Weapon = 5
}

sgs.ai_chaofeng.dianwei = 2

local huoji_skill={}
huoji_skill.name="huoji"
table.insert(sgs.ai_skills,huoji_skill)
huoji_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)

	local card

	self:sortByUseValue(cards,true)

	for _,acard in ipairs(cards) do
		if acard:isRed() and not acard:isKindOf("Peach") and (self:getDynamicUsePriority(acard) < sgs.ai_use_value.FireAttack or self:getOverflow() > 0) then
			if acard:isKindOf("Slash") and self:getCardsNum("Slash") == 1 then
				local keep
				local dummy_use = { isDummy = true , to = sgs.SPlayerList() }
				self:useBasicCard(acard, dummy_use)
				if dummy_use.card and dummy_use.to and dummy_use.to:length() > 0 then
					for _, p in sgs.qlist(dummy_use.to) do
						if p:getHp() <= 1 then keep = true break end
					end
					if dummy_use.to:length() > 1 then keep = true end
				end
				if keep then sgs.ai_use_priority.Slash = sgs.ai_use_priority.FireAttack + 0.1
				else
					sgs.ai_use_priority.Slash = 2.6
					card = acard
					break
				end
			else
				card = acard
				break
			end
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

sgs.ai_cardneed.huoji = function(to, card, self)
	return to:getHandcardNum() >= 2 and card:isRed()
end

sgs.ai_view_as.kanpo = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place == sgs.Player_PlaceHand then
		if card:isBlack() then
			return ("nullification:kanpo[%s:%s]=%d"):format(suit, number, card_id)
		end
	end
end

sgs.ai_cardneed.kanpo = function(to, card, self)
	return card:isBlack()
end

sgs.ai_skill_invoke.bazhen = sgs.ai_skill_invoke.EightDiagram

function sgs.ai_armor_value.bazhen(card)
	if not card then return 4 end
end

sgs.kanpo_suit_value = {
	spade = 3.9,
	club = 3.9
}

local lianhuan_skill={}
lianhuan_skill.name="lianhuan"
table.insert(sgs.ai_skills,lianhuan_skill)
lianhuan_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)

	local card
	self:sortByUseValue(cards, true)
	
	local slash = self:getCard("FireSlash") or self:getCard("ThunderSlash") or self:getCard("Slash")
	if slash then
		local dummy_use = { isDummy = true }
		self:useBasicCard(slash, dummy_use)
		if not dummy_use.card then slash = nil end
	end
	
	for _, acard in ipairs(cards) do
		if acard:getSuit() == sgs.Card_Club then
			local shouldUse = true
			if self:getUseValue(acard) > sgs.ai_use_value.IronChain and acard:getTypeId() == sgs.Card_TypeTrick then
				local dummy_use = { isDummy = true }
				self:useTrickCard(acard, dummy_use)
				if dummy_use.card then shouldUse = false end
			end
			if acard:getTypeId() == sgs.Card_TypeEquip then
				local dummy_use = { isDummy = true }
				self:useEquipCard(acard, dummy_use)
				if dummy_use.card then shouldUse = false end
			end
			if shouldUse and (not slash or slash:getEffectiveId() ~= acard:getEffectiveId()) then
				card = acard
				break
			end
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

sgs.ai_cardneed.lianhuan = function(to, card)
	return card:getSuit() == sgs.Card_Club and to:getHandcardNum() <= 2
end

sgs.ai_skill_invoke.niepan = function(self, data)
	local dying = data:toDying()
	local peaches = 1 - dying.who:getHp()

	return self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < peaches
end

sgs.ai_chaofeng.pangtong = -1

local tianyi_skill = {}
tianyi_skill.name = "tianyi"
table.insert(sgs.ai_skills, tianyi_skill)
tianyi_skill.getTurnUseCard = function(self)
	if self:needBear() then return end
	if not self.player:hasUsed("TianyiCard") and not self.player:isKongcheng() then return sgs.Card_Parse("@TianyiCard=.") end
end

sgs.ai_skill_use_func.TianyiCard = function(card,use,self)
	self:sort(self.enemies, "handcard")
	local max_card = self:getMaxCard()
	if not max_card then return end
	local max_point = max_card:getNumber()
	local slashcount = self:getCardsNum("Slash")
	if isCard("Slash", max_card, self.player) then slashcount = slashcount - 1 end
	if self.player:hasSkill("kongcheng") and self.player:getHandcardNum() == 1 then
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isKongcheng() and not self:doNotDiscard(enemy, "h") then
				sgs.ai_use_priority.TianyiCard = 1.2
				self.tianyi_card = max_card:getId()
				use.card = sgs.Card_Parse("@TianyiCard=.")
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if enemy:hasFlag("AI_HuangtianPindian") and enemy:getHandcardNum() == 1 then
			sgs.ai_use_priority.TianyiCard = 7.2
			self.tianyi_card = max_card:getId()
			use.card = sgs.Card_Parse("@TianyiCard=.")
			if use.to then
				use.to:append(enemy)
				enemy:setFlags("-AI_HuangtianPindian")
			end
			return
		end
	end
	local zhugeliang = self.room:findPlayerBySkillName("kongcheng")

	local slash = self:getCard("Slash")	
	local dummy_use = {isDummy = true}
	self.player:setFlags("slashNoDistanceLimit")
	if slash then self:useBasicCard(slash, dummy_use) end
	self.player:setFlags("-slashNoDistanceLimit")

	sgs.ai_use_priority.TianyiCard = (slashcount >= 1 and dummy_use.card) and 7.2 or 1.2
	if slashcount >= 1 and slash and dummy_use.card  then		
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) and not enemy:isKongcheng() then
				local enemy_max_card = self:getMaxCard(enemy)
				local enemy_max_point = enemy_max_card and enemy_max_card:getNumber() or 100
				if max_point > enemy_max_point then
					self.tianyi_card = max_card:getId()
					use.card = sgs.Card_Parse("@TianyiCard=.")
					if use.to then use.to:append(enemy) end
					return
				end
			end
		end
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) and not enemy:isKongcheng() then
				if max_point >= 10 then
					self.tianyi_card = max_card:getId()
					use.card = sgs.Card_Parse("@TianyiCard=.")
					if use.to then use.to:append(enemy) end
					return
				end
			end
		end
		if #self.enemies < 1 then return end
		self:sort(self.friends_noself, "handcard")
		for index = #self.friends_noself, 1, -1 do
			local friend = self.friends_noself[index]
			if not friend:isKongcheng() then
				local friend_min_card = self:getMinCard(friend)
				local friend_min_point = friend_min_card and friend_min_card:getNumber() or 100
				if max_point > friend_min_point then
					self.tianyi_card = max_card:getId()
					use.card = sgs.Card_Parse("@TianyiCard=.")
					if use.to then use.to:append(friend) end
					return
				end
			end
		end

		if zhugeliang and self:isFriend(zhugeliang) and zhugeliang:getHandcardNum() == 1 and zhugeliang:objectName() ~= self.player:objectName() then
			if max_point >= 7 then
				self.tianyi_card = max_card:getId()
				use.card = sgs.Card_Parse("@TianyiCard=.")
				if use.to then use.to:append(zhugeliang) end
				return
			end
		end

		for index = #self.friends_noself, 1, -1 do
			local friend = self.friends_noself[index]
			if not friend:isKongcheng() then
				if max_point >= 7 then
					self.tianyi_card = max_card:getId()
					use.card = sgs.Card_Parse("@TianyiCard=.")
					if use.to then use.to:append(friend) end
					return
				end
			end
		end
	end

	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)
	if zhugeliang and self:isFriend(zhugeliang) and zhugeliang:getHandcardNum() == 1
		and zhugeliang:objectName() ~= self.player:objectName() and self:getEnemyNumBySeat(self.player, zhugeliang) >= 1 then
		if isCard("Jink", cards[1], self.player) and self:getCardsNum("Jink") == 1 then return end
		self.tianyi_card = cards[1]:getId()
		use.card = sgs.Card_Parse("@TianyiCard=.")
		if use.to then use.to:append(zhugeliang) end
		return
	end

	if self:getOverflow() > 0 then
		for _, enemy in ipairs(self.enemies) do
			if not self:doNotDiscard(enemy, "h", true) and not enemy:isKongcheng() then
				self.tianyi_card = cards[1]:getId()
				use.card = sgs.Card_Parse("@TianyiCard=.")
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end
	return nil
end

function sgs.ai_skill_pindian.tianyi(minusecard, self, requestor)
	if requestor:getHandcardNum() == 1 then
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(cards)
		return cards[1]
	end
	local maxcard = self:getMaxCard()
	return self:isFriend(requestor) and self:getMinCard() or ( maxcard:getNumber() < 6 and  minusecard or maxcard )
end

sgs.ai_cardneed.tianyi = function(to, card, self)
	local cards = to:getHandcards()
	local has_big = false
	for _, c in sgs.qlist(cards) do
		local flag = string.format("%s_%s_%s", "visible", self.room:getCurrent():objectName(), to:objectName())
		if c:hasFlag("visible") or c:hasFlag(flag) then
			if c:getNumber() > 10 then
				has_big = true
				break
			end
		end
	end
	if not has_big then
		return card:getNumber() > 10
	else
		return card:isKindOf("Slash") or card:isKindOf("Analeptic")
	end
end

sgs.ai_card_intention.TianyiCard = 0
sgs.dynamic_value.control_card.TianyiCard = true

sgs.ai_use_value.TianyiCard = 8.5

sgs.ai_chaofeng.taishici = 3

local luanji_skill = {}
luanji_skill.name = "luanji"
table.insert(sgs.ai_skills, luanji_skill)
luanji_skill.getTurnUseCard = function(self)
	local archery = sgs.Sanguosha:cloneCard("archery_attack")

	local first_found, second_found = false, false
	local first_card, second_card
	if self.player:getHandcardNum() >= 2 then
		local cards = self.player:getHandcards()
		local same_suit = false
		cards = sgs.QList2Table(cards)
		for _, fcard in ipairs(cards) do
			if not (isCard("Peach", fcard, self.player) or isCard("ExNihilo", fcard, self.player) or isCard("AOE", fcard, self.player)) then
				first_card = fcard
				first_found = true
				for _, scard in ipairs(cards) do
					if first_card ~= scard and scard:getSuit() == first_card:getSuit()
						and not (isCard("Peach", scard, self.player) or isCard("ExNihilo", scard, self.player) or isCard("AOE", scard, self.player)) then

						local card_str = ("archery_attack:luanji[%s:%s]=%d+%d"):format("to_be_decided", 0, first_card:getId(), scard:getId())
						local archeryattack = sgs.Card_Parse(card_str)
						local dummy_use = { isDummy = true }
						self:useTrickCard(archeryattack, dummy_use)
						if dummy_use.card then
							second_card = scard
							second_found = true
							break
						end
					end
				end
				if second_card then break end
			end
		end
	end

	if first_found and second_found then
		local luanji_card = {}
		local first_id = first_card:getId()
		local second_id = second_card:getId()
		local card_str = ("archery_attack:luanji[%s:%s]=%d+%d"):format("to_be_decided", 0, first_id, second_id)
		local archeryattack = sgs.Card_Parse(card_str)
		assert(archeryattack)
			
		return archeryattack
	end
end

sgs.ai_chaofeng.yuanshao = 1

sgs.ai_skill_invoke.shuangxiong=function(self,data)
	if self:needBear() then return false end
	if self.player:isSkipped(sgs.Player_Play) or (self.player:getHp() < 2 and not (self:getCardsNum("Slash") > 1 and self.player:getHandcardNum() >= 3)) or #self.enemies == 0 then
		return false
	end
	local duel = sgs.Sanguosha:cloneCard("duel")

	local dummy_use = {isDummy = true}
	self:useTrickCard(duel, dummy_use)
	
	return self.player:getHandcardNum() >= 3 and dummy_use.card
end

sgs.ai_cardneed.shuangxiong=function(to, card, self)
	return not self:willSkipDrawPhase(to)
end

local shuangxiong_skill={}
shuangxiong_skill.name="shuangxiong"
table.insert(sgs.ai_skills,shuangxiong_skill)
shuangxiong_skill.getTurnUseCard=function(self)
	if self.player:getMark("shuangxiong") == 0 then return nil end
	local mark = self.player:getMark("shuangxiong")

	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
	
	local card
	for _,acard in ipairs(cards)  do
		if (acard:isRed() and mark == 2) or (acard:isBlack() and mark == 1) then
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

sgs.ai_chaofeng.yanliangwenchou = 1

sgs.ai_skill_invoke.mengjin = function(self, data)
	local effect = data:toSlashEffect()
	if self:isEnemy(effect.to) then
		if self:doNotDiscard(effect.to) then
			return false
		end
	end
	if self:isFriend(effect.to) then 
		return self:needToThrowArmor(effect.to) or self:doNotDiscard(effect.to)
	end
	return not self:isFriend(effect.to)
end

sgs.ai_suit_priority.lianhuan= "club|diamond|heart|spade"
sgs.ai_suit_priority.huoji= "club|spade|diamond|heart"
sgs.ai_suit_priority.kanpo= "diamond|heart|club|spade"
