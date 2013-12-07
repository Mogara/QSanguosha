sgs.ai_skill_invoke.moukui = function(self, data)
	local target = data:toPlayer()
	sgs.moukui_target = target
	if self:isFriend(target) then return self:needToThrowArmor(target) else return true end 
end

sgs.ai_skill_choice.moukui = function(self, choices, data)
	local target = sgs.moukui_target
	if self:isEnemy(target) and self:doNotDiscard(target) then
		return "draw"
	end	
	return "discard"
end

sgs.ai_skill_invoke.tianming = function(self, data)
	self.tianming_discard = nil
	if hasManjuanEffect(self.player) then return false end
	if self.player:isNude() then return true end
	if not self:canHit() and self.player:getCards("he"):length() < 3 then return false end
	if self:canHit() then return true end
	local unpreferedCards = {}
	local cards = sgs.QList2Table(self.player:getHandcards())

	if self:getCardsNum("Slash") > 1 then
		self:sortByKeepValue(cards)
		for _, card in ipairs(cards) do
			if card:isKindOf("Slash") then table.insert(unpreferedCards, card:getId()) end
		end
		table.remove(unpreferedCards, 1)
	end

	local num = self:getCardsNum("Jink") - 1
	if self.player:getArmor() then num = num + 1 end
	if num > 0 then
		for _, card in ipairs(cards) do
			if card:isKindOf("Jink") and num > 0 then
				table.insert(unpreferedCards, card:getId())
				num = num - 1
			end
		end
	end

	for _, card in ipairs(cards) do
		if (card:isKindOf("Weapon") and self.player:getHandcardNum() < 3) or card:isKindOf("OffensiveHorse")
			or self:getSameEquip(card, self.player) or card:isKindOf("AmazingGrace") or card:isKindOf("Lightning") then
			table.insert(unpreferedCards, card:getId())
		end
	end

	if self.player:getWeapon() and self.player:getHandcardNum() < 3 then
		table.insert(unpreferedCards, self.player:getWeapon():getId())
	end

	if self:needToThrowArmor() then
		table.insert(unpreferedCards, self.player:getArmor():getId())
	end

	if self.player:getOffensiveHorse() and self.player:getWeapon() then
		table.insert(unpreferedCards, self.player:getOffensiveHorse():getId())
	end

	local use_cards = {}
	for index = #unpreferedCards, 1, -1 do
		if not self.player:isJilei(sgs.Sanguosha:getCard(unpreferedCards[index])) then table.insert(use_cards, unpreferedCards[index]) end
	end

	if #use_cards >= 2 or #use_cards == #cards then
		self.tianming_discard = use_cards
		return true
	end
end

sgs.ai_skill_discard.tianming = function(self, discard_num, min_num, optional, include_equip)
	local discard = self.tianming_discard
	if discard and #discard >= 2 then
		return { discard[1], discard[2] }
	else
		return self:askForDiscard("dummyreason", 2, 2, false, true)
	end
end

local mizhao_skill = {}
mizhao_skill.name = "mizhao"
table.insert(sgs.ai_skills, mizhao_skill)
mizhao_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("MizhaoCard") or self.player:isKongcheng() then return end
	if self:needBear() then return end
	local cards = self.player:getHandcards()
	local allcard = {}
	cards = sgs.QList2Table(cards)
	for _,card in ipairs(cards) do
		table.insert(allcard, card:getId()) 
	end
	local parsed_card = sgs.Card_Parse("@MizhaoCard=" .. table.concat(allcard,"+"))
	return parsed_card
end

sgs.ai_skill_use_func.MizhaoCard = function(card, use, self)
	local handcardnum = self.player:getHandcardNum()
	local trash = self:getCard("Disaster") or self:getCard("GodSalvation") or self:getCard("AmazingGrace") or self:getCard("Slash") or self:getCard("FireAttack")
	local count = 0
	local target
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isKongcheng() then count = count + 1 end
	end
	if handcardnum == 1 and trash and count >= 1 and #self.enemies > 1 then
		self:sort(self.enemies, "handcard")
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("manjuan") and enemy:isKongcheng()) and not enemy:hasSkills("tuntian+zaoxian") then
				target = enemy
				break
			end
		end
	end
	if not target then
		self:sort(self.friends_noself, "defense")
		self.friends_noself = sgs.reverse(self.friends_noself)
		if count < 1 then return end
		for _, friend in ipairs(self.friends_noself) do
			if friend:hasSkills("tuntian+zaoxian") and not friend:hasSkill("manjuan") and not self:isWeak(friend) then
				target = friend
				break
			end
		end
		if not target then
			for _, friend in ipairs(self.friends_noself) do
				if not friend:hasSkill("manjuan") then
					target = friend
					break
				end
			end
		end
	end
	if target then
		for _, acard in sgs.qlist(self.player:getHandcards()) do
			if isCard("Peach", acard, self.player) and self.player:getHandcardNum() > 1 and self.player:isWounded()
				and not self:needToLoseHp(self.player) then
					use.card = acard
					return
			end
		end
		use.card = card
		if use.to then
			target:setFlags("AI_MizhaoTarget")
			use.to:append(target)
		end
	end
end

sgs.ai_use_priority.MizhaoCard = 1.5
sgs.ai_card_intention.MizhaoCard = 0
sgs.ai_playerchosen_intention.mizhao = 10

sgs.ai_skill_playerchosen.mizhao = function(self, targets)
	self:sort(self.enemies, "defense")
	local slash = sgs.Sanguosha:cloneCard("slash")
	local from
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasFlag("AI_MizhaoTarget") then
			from = player
			from:setFlags("-AI_MizhaoTarget")
			break
		end
	end
	if from then
		for _, to in ipairs(self.enemies) do
			if targets:contains(to) and self:slashIsEffective(slash, to, from) and not self:getDamagedEffects(to, from, true)
				and not self:needToLoseHp(to, from, true) and not self:findLeijiTarget(to, 50, from) then
				return to
			end
		end
	end
	for _, to in ipairs(self.enemies) do
		if targets:contains(to) then
			return to
		end
	end
end

function sgs.ai_skill_pindian.mizhao(minusecard, self, requestor, maxcard)
	local req
	if self.player:objectName() == requestor:objectName() then
		for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if p:hasFlag("MizhaoPindianTarget") then
				req = p
				break
			end
		end
	else
		req = requestor
	end
	local cards, maxcard = sgs.QList2Table(self.player:getHandcards())
	local function compare_func1(a, b)
		return a:getNumber() > b:getNumber()
	end
	local function compare_func2(a, b)
		return a:getNumber() < b:getNumber()
	end
	if self:isFriend(req) and self.player:getHp() > req:getHp() then
		table.sort(cards, compare_func2)
	else
		table.sort(cards, compare_func1)
	end
	for _, card in ipairs(cards) do
		if self:getKeepValue(card) < 8 or card:isKindOf("EquipCard") then maxcard = card break end
	end
	return maxcard or cards[1]
end

sgs.ai_skill_cardask["@jieyuan-increase"] = function(self, data)
	local damage = data:toDamage()
	local target = damage.to
	if self:isFriend(target) then return "." end
	if target:hasArmorEffect("SilverLion") then return "." end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _,card in ipairs(cards) do
		if card:isBlack() then return "$" .. card:getEffectiveId() end
	end
	return "."
end

sgs.ai_skill_cardask["@jieyuan-decrease"] = function(self, data)
	local damage = data:toDamage()
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	if damage.card and damage.card:isKindOf("Slash") then		 
		if self:hasHeavySlashDamage(damage.from, damage.card, self.player) then
			for _,card in ipairs(cards) do
				if card:isRed() then return "$" .. card:getEffectiveId() end
			end
		end
	end
	if self:getDamagedEffects(self.player, damage.from) and damage.damage <= 1 then return "." end	
	if self:needToLoseHp(self.player, damage.from) and damage.damage <= 1 then return "." end	
	for _,card in ipairs(cards) do
		if card:isRed() then return "$" .. card:getEffectiveId() end
	end
	return "."
end

function sgs.ai_cardneed.jieyuan(to, card)
	return to:getHandcardNum() < 4 and (to:getHp() >= 3 and true or card:isRed())
end

sgs.ai_skill_invoke.fenxin = function(self, data)
	local target = data:toPlayer()
	local target_role = sgs.evaluatePlayerRole(target)
	local self_role = self.player:getRole()
	if target_role == "renegade" or target_role == "neutral" then return false end
	local process = sgs.gameProcess(self.room)
	return (target_role == "rebel" and self.role ~= "rebel" and process:match("rebel"))
			or (target_role == "loyalist" and self.role ~= "loyalist" and process:match("loyal"))
end

local mixin_skill = {}
mixin_skill.name = "mixin"
table.insert(sgs.ai_skills, mixin_skill)
mixin_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("MixinCard") or self.player:isKongcheng() then return end
	if self:needBear() then return end
	if #self.friends_noself == 0 then return end
	return sgs.Card_Parse("@MixinCard=.")
end

sgs.ai_skill_use_func.MixinCard = function(card, use, self)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	if #self.enemies < 1 then return end
	local slash	
	self:sortByKeepValue(cards)
	for _, acard in ipairs(cards) do
		if acard:isKindOf("Slash") then
			slash = acard
			break
		end
	end
	
	if slash then
		for _, friend in ipairs(self.friends_noself) do
			if friend:hasSkills("tuntian+zaoxian") and not friend:hasSkill("manjuan") then
				use.card = sgs.Card_Parse("@MixinCard="..slash:getEffectiveId())
				if use.to then use.to:append(friend) end
				return
			end
		end
		for _, friend in ipairs(self.friends_noself) do
			if not friend:hasSkill("manjuan") then
				use.card = sgs.Card_Parse("@MixinCard="..slash:getEffectiveId())
				if use.to then use.to:append(friend) end
				return
			end
		end
	else
		local compare_more_slash = function(a, b)
			return getCardsNum("Slash", a) > getCardsNum("Slash", b)
		end
		table.sort(self.friends_noself, compare_more_slash)
		for _, friend in ipairs(self.friends_noself) do
			if not friend:hasSkill("manjuan") and getCardsNum("Slash", friend) >= 1 then
				use.card = sgs.Card_Parse("@MixinCard="..cards[1]:getEffectiveId())
				if use.to then use.to:append(friend) end
				return
			end
		end
		for _, friend in ipairs(self.friends_noself) do
			if friend:hasSkills("tuntian+zaoxian") and not friend:hasSkill("manjuan") then
				use.card = sgs.Card_Parse("@MixinCard="..cards[1]:getEffectiveId())
				if use.to then use.to:append(friend) end
				return
			end
		end
	end
end

sgs.ai_skill_playerchosen.mixin = sgs.ai_skill_playerchosen.zero_card_as_slash

sgs.ai_skill_cardask["#mixin"] = function(self, data, pattern, target)
	if target then
		for _, slash in ipairs(self:getCards("Slash")) do
			if self:isFriend(target) and self:slashIsEffective(slash, target) then
				if self:needLeiji(target, self.player) then return slash:toString() end
				if self:getDamagedEffects(target, self.player) then return slash:toString() end
				if self:needToLoseHp(target, self.player, nil, true) then return slash:toString() end
			end
			
			if not self:isFriend(target) and self:slashIsEffective(slash, target) 
				and not self:getDamagedEffects(target, self.player, true) and not self:needLeiji(target, self.player) then
					return slash:toString()
			end
		end
		for _, slash in ipairs(self:getCards("Slash")) do
			if not self:isFriend(target) then
				if not self:needLeiji(target, self.player) then return slash:toString() end
				if not self:slashIsEffective(slash, target) then return slash:toString() end			
			end
		end
	end
	return "."
end

sgs.ai_use_priority.MixinCard = 0
sgs.ai_card_intention.MixinCard = -20

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

duyi_skill = {}
duyi_skill.name = "duyi"
table.insert(sgs.ai_skills, duyi_skill)
duyi_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("DuyiCard") then return end
	return sgs.Card_Parse("@DuyiCard=.")
end

sgs.ai_skill_use_func.DuyiCard = function(card,use,self)
	use.card = card
end

sgs.ai_skill_playerchosen.duyi = function(self, targets)
	if self:needBear() then return self.player end
	local to
	if self:getOverflow() < 0 then
		to = self:findPlayerToDraw(true)
	else
		to = self:findPlayerToDraw(false)
	end
	if to then return to
	else return self.player
	end
end

sgs.ai_skill_invoke.duanzhi = function(self, data)
	local use = data:toCardUse()
	if self:isEnemy(use.from) and use.card:getSubtype() == "attack_card" and self.player:getHp() == 1 and not self:getCard("Peach")
		and not self:getCard("Analeptic") and not isLord(self.player) and self:getAllPeachNum() == 0 then
		self.player:setFlags("AI_doNotSave")
		return true
	end
	return use.from and self:isEnemy(use.from) and not self:doNotDiscard(use.from, "he", true, 2) and self.player:getHp() > 2
end

sgs.ai_skill_choice.duanzhi = function(self, choices)
	return "discard"
end

sgs.ai_skill_use["@@fengyin"] = function(self, data)
	if self:needBear() then return "." end
	local cards = self.player:getHandcards()
	local card
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
	
	local target = self.room:getCurrent()
	if self:isFriend(target) and self:willSkipPlayPhase(target) and target:getHandcardNum() + 2 > target:getHp() and target:getHp() >= self.player:getHp() then
		return "@FengyinCard="..card_id
	end
	if self:isEnemy(target) and not self:willSkipPlayPhase(target) and target:getHandcardNum() >= target:getHp() and target:getHp() >= self.player:getHp() then
		return "@FengyinCard="..card_id
	end
	return "."
end

sgs.ai_skill_invoke.cv_caocao = function(self, data)
	if math.random(0, 6) == 0 then return true end
	return false
end

sgs.ai_skill_invoke.cv_lingju = function(self, data)
	if math.random(0, 2) == 0 then return true end
	return false
end