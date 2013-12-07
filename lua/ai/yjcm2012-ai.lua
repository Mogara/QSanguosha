sgs.ai_skill_invoke.qianxi = function(self, data)
	if self.player:getPile("incantation"):length() > 0 then
		local card = sgs.Sanguosha:getCard(self.player:getPile("incantation"):first())
		if not self.player:getJudgingArea():isEmpty() and not self.player:containsTrick("YanxiaoCard") and not self:hasWizard(self.enemies, true) then
			local trick = self.player:getJudgingArea():last()
			if trick:isKindOf("Indulgence") then
				if card:getSuit() == sgs.Card_Heart or (self.player:hasSkill("hongyan") and card:getSuit() == sgs.Card_Spade) then return false end
			elseif trick:isKindOf("SupplyShortage") then
				if card:getSuit() == sgs.Card_Club then return false end
			end
		end
		local zhangbao = self.room:findPlayerBySkillName("yingbing")
		if zhangbao and self:isEnemy(zhangbao) and not zhangbao:hasSkill("manjuan")
			and (card:isRed() or (self.player:hasSkill("hongyan") and card:getSuit() == sgs.Card_Spade)) then return false end
	end
 	for _, p in ipairs(self.enemies) do
		if self.player:distanceTo(p) == 1 and not p:isKongcheng() then
			return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen.qianxi = function(self, targets)
	local enemies = {}
	local slash = self:getCard("Slash") or sgs.Sanguosha:cloneCard("slash")
	local isRed = (self.player:getTag("qianxi"):toString() == "red")

	for _, target in sgs.qlist(targets) do
		if self:isEnemy(target) and not target:isKongcheng() then
			table.insert(enemies, target)
		end
	end

	if #enemies == 1 then
		return enemies[1]
	else
		self:sort(enemies, "defense")
		if not isRed then
			for _, enemy in ipairs(enemies) do
				if enemy:hasSkill("qingguo") and self:slashIsEffective(slash, enemy) then return enemy end
			end
			for _, enemy in ipairs(enemies) do
				if enemy:hasSkill("kanpo") then return enemy end
			end
		else
			for _, enemy in ipairs(enemies) do
				if getKnownCard(enemy, "Jink", false, "h") > 0 and self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self) then return enemy end
			end
			for _, enemy in ipairs(enemies) do
				if getKnownCard(enemy, "Peach", true, "h") > 0 or enemy:hasSkill("jijiu") then return enemy end
			end
			for _, enemy in ipairs(enemies) do
				if getKnownCard(enemy, "Jink", false, "h") > 0 and self:slashIsEffective(slash, enemy) then return enemy end
			end
		end
		for _, enemy in ipairs(enemies) do
			if enemy:hasSkill("longhun") then return enemy end
		end
		return enemies[1]
	end
	return targets:first()
end

sgs.ai_playerchosen_intention.qianxi = 80

function sgs.ai_cardneed.dangxian(to, card)
	return isCard("Slash", card, to) and getKnownCard(to, "Slash", true) == 0
end

sgs.ai_skill_invoke.zishou = function(self, data)
	if self:needBear() then return true end
	if self.player:isSkipped(sgs.Player_Play) then return true end

	local chance_value = 1
	local peach_num = self:getCardsNum("Peach")
	local can_save_card_num = self:getOverflow(self.player, true) - self.player:getHandcardNum()
	
	if self.player:getHp() <= 2 and self.player:getHp() < getBestHp(self.player) then chance_value = chance_value + 1 end
	if self.player:hasSkills("nosrende|rende") and self:findFriendsByType(sgs.Friend_Draw) then chance_value = chance_value - 1 end
	if self.player:hasSkill("qingnang") then
		for _, friend in ipairs(self.friends) do
			if friend:isWounded() then chance_value = chance_value - 1 break end
		end
	end
	if self.player:hasSkill("jieyin") then
		for _, friend in ipairs(self.friends) do
			if friend:isWounded() and friend:isMale() then chance_value = chance_value - 1 break end
		end
	end
	
	return self:ImitateResult_DrawNCards(self.player, self.player:getVisibleSkillList()) - can_save_card_num + peach_num  <= chance_value
end

sgs.ai_skill_invoke.fuli = true

function sgs.ai_cardsview.fuhun(self, class_name, player)
	if class_name == "Slash" then
		return cardsView_spear(self, player, "fuhun")
	end
end

local fuhun_skill = {}
fuhun_skill.name = "fuhun"
table.insert(sgs.ai_skills, fuhun_skill)
fuhun_skill.getTurnUseCard = function(self, inclusive)
	return turnUse_spear(self, inclusive, "fuhun")
end

function sgs.ai_skill_invoke.zhenlie(self, data)
	local use = data:toCardUse()
	if not use.from or use.from:isDead() then return false end
	if self.role == "rebel" and sgs.evaluatePlayerRole(use.from) == "rebel" and not use.from:hasSkill("jueqing")
		and self.player:getHp() == 1 and self:getAllPeachNum() < 1 then return false end

	if self:isEnemy(use.from) or (self:isFriend(use.from) and self.role == "loyalist" and not use.from:hasSkill("jueqing") and use.from:isLord() and self.player:getHp() == 1) then
		if use.card:isKindOf("Slash") then
			if not self:slashIsEffective(use.card, self.player, use.from) then return false end
			if self:hasHeavySlashDamage(use.from, use.card, self.player) then return true end

			local jink_num = self:getExpectedJinkNum(use)
			local hasHeart = false
			for _, card in ipairs(self:getCards("Jink")) do
				if card:getSuit() == sgs.Card_Heart then
					hasHeart = true
					break
				end
			end
			if self:getCardsNum("Jink") == 0
				or jink_num == 0
				or self:getCardsNum("Jink") < jink_num
				or (use.from:hasSkill("dahe") and self.player:hasFlag("dahe") and not hasHeart) then

				if use.card:isKindOf("NatureSlash") and self.player:isChained() and not self:isGoodChainTarget(self.player, nil, nil, nil, use.card) then return true end
				if use.from:hasSkill("nosqianxi") and use.from:distanceTo(self.player) == 1 then return true end
				if self:isFriend(use.from) and self.role == "loyalist" and not use.from:hasSkill("jueqing") and use.from:isLord() and self.player:getHp() == 1 then return true end
				if (not (self:hasSkills(sgs.masochism_skill) or (self.player:hasSkill("tianxiang") and getKnownCard(self.player, "heart") > 0)) or use.from:hasSkill("jueqing"))
					and not self:doNotDiscard(use.from) then
					return true
				end
			end
		elseif use.card:isKindOf("AOE") then
			local from = use.from
			if use.card:isKindOf("SavageAssault") then
				local menghuo = self.room:findPlayerBySkillName("huoshou")
				if menghuo then from = menghuo end
			end

			local friend_null = 0
			for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
				if self:isFriend(p) then friend_null = friend_null + getCardsNum("Nullification", p) end
				if self:isEnemy(p) then friend_null = friend_null - getCardsNum("Nullification", p) end
			end
			friend_null = friend_null + self:getCardsNum("Nullification")
			local sj_num = self:getCardsNum(use.card:isKindOf("SavageAssault") and "Slash" or "Jink")

			if not self:hasTrickEffective(use.card, self.player, from) then return false end
			if not self:damageIsEffective(self.player, sgs.DamageStruct_Normal, from) then return false end
			if use.from:hasSkill("drwushuang") and self.player:getCardCount() == 1 and self:hasLoseHandcardEffective() then return true end
			if sj_num == 0 and friend_null <= 0 then
				if self:isEnemy(from) and from:hasSkill("jueqing") then return not self:doNotDiscard(from) end
				if self:isFriend(from) and self.role == "loyalist" and from:isLord() and self.player:getHp() == 1 and not from:hasSkill("jueqing") then return true end
				if (not (self:hasSkills(sgs.masochism_skill) or (self.player:hasSkill("tianxiang") and getKnownCard(self.player, "heart") > 0)) or use.from:hasSkill("jueqing"))
					and not self:doNotDiscard(use.from) then
					return true
				end
			end
		elseif self:isEnemy(use.from) then
			if use.card:isKindOf("FireAttack") then
				if not self:hasTrickEffective(use.card, self.player) then return false end
				if not self:damageIsEffective(self.player, sgs.DamageStruct_Fire, use.from) then return false end
				if (self.player:hasArmorEffect("Vine") or self.player:getMark("@gale") > 0) and use.from:getHandcardNum() > 3
					and not (use.from:hasSkill("hongyan") and getKnownCard(self.player, "spade") > 0) then
					return not self:doNotDiscard(use.from)
				elseif self.player:isChained() and not self:isGoodChainTarget(self.player) then
					return not self:doNotDiscard(use.from)
				end
			elseif (use.card:isKindOf("Snatch") or use.card:isKindOf("Dismantlement"))
					and self:getCardsNum("Peach") == self.player:getHandcardNum() and not self.player:isKongcheng() then
				if not self:hasTrickEffective(use.card, self.player) then return false end
				return not self:doNotDiscard(use.from)
			elseif use.card:isKindOf("Duel") then
				if self:getCardsNum("Slash") == 0 or self:getCardsNum("Slash") < getCardsNum("Slash", use.from) then
					if not self:hasTrickEffective(use.card, self.player) then return false end
					if not self:damageIsEffective(self.player, sgs.DamageStruct_Normal, use.from) then return false end
					return not self:doNotDiscard(use.from)
				end
			elseif use.card:isKindOf("TrickCard") and not use.card:isKindOf("AmazingGrace") then
				if not self:doNotDiscard(use.from) and self:needToLoseHp(self.player) then
					return true
				end
			end
		end
	end
	return false
end

sgs.ai_skill_choice.miji_draw = function(self, choices)
	return "" .. self.player:getLostHp()
end

sgs.ai_skill_invoke.miji = function(self, data)
	if #self.friends_noself == 0 then return false end
	for _, friend in ipairs(self.friends_noself) do
		if not friend:hasSkill("manjuan") and not self:isLihunTarget(friend) then return true end
	end
	return false
end

sgs.ai_skill_askforyiji.miji = function(self, card_ids)
	local available_friends = {}
	for _, friend in ipairs(self.friends_noself) do
		if not friend:hasSkill("manjuan") and not self:isLihunTarget(friend) then table.insert(available_friends, friend) end
	end

	local toGive, allcards = {}, {}
	local keep
	for _, id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(id)
		if not keep and (isCard("Jink", card, self.player) or isCard("Analeptic", card, self.player)) then
			keep = true
		else
			table.insert(toGive, card)
		end
		table.insert(allcards, card)
	end

	local cards = #toGive > 0 and toGive or allcards
	self:sortByKeepValue(cards, true)
	local id = cards[1]:getId()

	local card, friend = self:getCardNeedPlayer(cards)
	if card and friend and table.contains(available_friends, friend) then return friend, card:getId() end

	if #available_friends > 0 then
		self:sort(available_friends, "handcard")
		for _, afriend in ipairs(available_friends) do
			if not self:needKongcheng(afriend, true) then
				return afriend, id
			end
		end
		self:sort(available_friends, "defense")
		return available_friends[1], id
	end
	return nil, -1
end

function sgs.ai_cardneed.jiangchi(to, card)
	return isCard("Slash", card, to) and getKnownCard(to, "Slash", true) < 2
end

sgs.ai_skill_choice.jiangchi = function(self, choices)
	local target = 0
	local goodtarget = 0
	local slashnum = 0
	local needburst = 0
	
	for _, slash in ipairs(self:getCards("Slash")) do
		for _,enemy in ipairs(self.enemies) do
			if self:slashIsEffective(slash, enemy) then 
				slashnum = slashnum + 1
				break
			end 
		end
	end

	for _,enemy in ipairs(self.enemies) do
		for _, slash in ipairs(self:getCards("Slash")) do
			if not self:slashProhibit(slash, enemy) and self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self) then
				goodtarget = goodtarget + 1
				break
			end
		end
	end

	for _,enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy) then
			target = target + 1
			break
		end
	end

	if slashnum > 1 or (slashnum > 0 and goodtarget > 0) then needburst = 1 end
	self:sort(self.enemies,"defenseSlash")
	local can_save_card_num = self.player:getMaxCards() - self.player:getHandcardNum()
	if target == 0 or can_save_card_num > 1 or self.player:isSkipped(sgs.Player_Play) then return "jiang" end
	if self:needBear() then return "jiang" end
	
	for _,enemy in ipairs(self.enemies) do
		local def = sgs.getDefense(enemy)
		local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
		local eff = self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self)

		if self:slashProhibit(slash, enemy) then
		elseif eff and def < 8 and needburst > 0 then return "chi"
		end
	end

	return "cancel"
end

local gongqi_skill={}
gongqi_skill.name = "gongqi"
table.insert(sgs.ai_skills, gongqi_skill)
gongqi_skill.getTurnUseCard = function(self,inclusive)
	if self.player:hasUsed("GongqiCard") then return end
	if self:needBear() then return end
	if #self.enemies == 0 then return end
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	if self:needToThrowArmor() then
		return sgs.Card_Parse("@GongqiCard=" .. self.player:getArmor():getEffectiveId())
	end
	
	for _, c in ipairs(cards) do
		if c:isKindOf("Weapon") then return sgs.Card_Parse("@GongqiCard=" .. c:getEffectiveId()) end
	end
	
	local handcards = self.player:getHandcards()
	handcards = sgs.QList2Table(handcards)
	local has_weapon, has_armor, has_def, has_off = false, false, false, false
	local weapon, armor
	for _, c in ipairs(handcards) do
		if c:isKindOf("Weapon") then
			has_weapon = true
			if not weapon or self:evaluateWeapon(weapon) < self:evaluateWeapon(c) then weapon = c end
		end
		if c:isKindOf("Armor") then
			has_armor = true
			if not armor or self:evaluateArmor(armor) < self:evaluateArmor(c) then armor = c end
		end
		if c:isKindOf("DefensiveHorse") then has_def = true end
		if c:isKindOf("OffensiveHorse") then has_off = true end
	end
	if has_off and self.player:getOffensiveHorse() then return sgs.Card_Parse("@GongqiCard=" .. self.player:getOffensiveHorse():getEffectiveId()) end
	if has_def and self.player:getDefensiveHorse() then return sgs.Card_Parse("@GongqiCard=" .. self.player:getDefensiveHorse():getEffectiveId()) end
	if has_weapon and self.player:getWeapon() and self:evaluateWeapon(self.player:getWeapon()) <= self:evaluateWeapon(weapon) then
		return sgs.Card_Parse("@GongqiCard=" .. self.player:getWeapon():getEffectiveId())
	end
	if has_armor and self.player:getArmor() and self:evaluateArmor(self.player:getArmor()) <= self:evaluateArmor(armor) then
		return sgs.Card_Parse("@GongqiCard=" .. self.player:getArmor():getEffectiveId())
	end
	
	if self:getOverflow() > 0 and self:getCardsNum("Slash") >= 1 then
		self:sortByKeepValue(handcards)
		self:sort(self.enemies, "defense")
		for _, c in ipairs(handcards) do
			if c:isKindOf("Snatch") or c:isKindOf("Dismantlement") then
				local use = { isDummy = true }
				self:useCardSnatch(c, use)
				if use.card then return end
			elseif isCard("Peach", c, self.player)
				or isCard("ExNihilo", c, self.player)
				or (isCard("Analeptic", c, self.player) and self.player:getHp() <= 2)
				or (isCard("Jink", c, self.player) and self:getCardsNum("Jink") < 2)
				or (isCard("Nullification", c, self.player) and self:getCardsNum("Nullification") < 2)
				or (isCard("Slash", c, self.player) and self:getCardsNum("Slash") == 1) then
				-- do nothing
			elseif not c:isKindOf("EquipCard") and #self.enemies > 0 and self.player:inMyAttackRange(self.enemies[1]) then
			else
				return sgs.Card_Parse("@GongqiCard=" .. c:getEffectiveId())
			end
		end
	end
end

sgs.ai_skill_use_func.GongqiCard = function(card, use, self)
	use.card = card
end

sgs.ai_skill_playerchosen.gongqi = function(self, targets)
	local player = self:findPlayerToDiscard()
	return player
end

sgs.ai_use_value.GongqiCard = 2
sgs.ai_use_priority.GongqiCard = 8

local jiefan_skill = {}
jiefan_skill.name = "jiefan"
table.insert(sgs.ai_skills, jiefan_skill)
jiefan_skill.getTurnUseCard = function(self, inclusive)
	if self.player:getMark("@rescue") == 0 then return end
	return sgs.Card_Parse("@JiefanCard=.")
end

sgs.ai_skill_use_func.JiefanCard = function(card, use, self)
	local target
	local use_value = 0
	local max_value = -10000
	for _, friend in ipairs(self.friends) do
		use_value = 0
		for _, p in sgs.qlist(self.room:getOtherPlayers(friend)) do
			if p:inMyAttackRange(friend) then
				if self:isFriend(p) then
					if not friend:hasSkill("manjuan") then use_value = use_value + 1 end
				else
					if p:getWeapon() then
						use_value = use_value + 1.2
					else
						if not friend:hasSkill("manjuan") then use_value = use_value + p:getHandcardNum() / 5 end
					end
				end
			end
		end
		use_value = use_value - friend:getHandcardNum() / 2
		if use_value > max_value then
			max_value = use_value
			target = friend
		end
	end

	if target and max_value >= self.player:aliveCount() / 2 then
		use.card = card
		if use.to then use.to:append(target) end
	end
end

sgs.ai_skill_cardask["@jiefan-discard"] = function(self, data)
	local player = data:toPlayer()
	if not player or not player:isAlive() or player:hasSkill("manjuan") or self:isFriend(player) then return "." end
	for _, card in sgs.qlist(self.player:getCards("he")) do
		if card:isKindOf("Weapon") and not self.player:hasEquip(card) then
			return "$" .. card:getEffectiveId()
		end
	end

	if not self.player:getWeapon() then return "." end
	local count = 0
	local range_fix = sgs.weapon_range[self.player:getWeapon():getClassName()] - self.player:getAttackRange(false)

	for _, p in sgs.qlist(self.room:getAllPlayers()) do
		if self:isEnemy(p) and self.player:distanceTo(p, range_fix) > self.player:getAttackRange() then count = count + 1 end
	end

	if count <= 2 then return "$" .. self.player:getWeapon():getEffectiveId() end
	return "."
end

sgs.ai_card_intention.JiefanCard = -80

function sgs.ai_cardneed.jiefan(to, card)
	return isCard("Slash", card, to) and getKnownCard(to, "Slash", true) == 0
end

anxu_skill = {}
anxu_skill.name = "anxu"
table.insert(sgs.ai_skills, anxu_skill)
anxu_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("AnxuCard") then return nil end
	card = sgs.Card_Parse("@AnxuCard=.")
	return card

end

sgs.ai_skill_use_func.AnxuCard = function(card,use,self)
	if #self.enemies == 0 then return end
	local intention = 50
	local friends = {}
	for _, friend in ipairs(self.friends_noself) do
		if friend:hasSkill("manjuan") then
			if self:needKongcheng(friend, true) then table.insert(friends, friend) end
		elseif not self:needKongcheng(friend, true) then
			table.insert(friends, friend)
		end
	end
	self:sort(friends, "handcard")

	local least_friend, most_friend
	if #friends > 0 then
		least_friend = friends[1]
		most_friend = friends[#friends]
	end

	local need_kongcheng_friend
	for _, friend in ipairs(friends) do
		if friend:getHandcardNum() == 1 and (friend:hasSkill("kongcheng") or (friend:hasSkill("zhiji") and friend:getMark("zhiji") == 0 and friend:getHp() >= 3)) then
			need_kongcheng_friend = friend
			break
		end
	end
	
	local enemies = {}
	for _, enemy in ipairs(self.enemies) do
		if not enemy:hasSkills("tuntian+zaoxian") and not (enemy:isKongcheng() or (enemy:getHandcardNum() <= 1 and self:needKongcheng(enemy))) then
			table.insert(enemies, enemy)
		end
	end

	self:sort(enemies, "handcard")
	enemies = sgs.reverse(enemies)
	local most_enemy
	if #enemies > 0 then most_enemy = enemies[1] end

	local prior_enemy, kongcheng_enemy, manjuan_enemy
	for _, enemy in ipairs(enemies) do
		if enemy:getHandcardNum() >= 2 and self:hasSkills(sgs.cardneed_skill, enemy) then
			if not prior_enemy then prior_enemy = enemy end
		end
		if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
			if not kongcheng_enemy then kongcheng_enemy = enemy end
		end
		if enemy:hasSkill("manjuan") then
			if not manjuan_enemy then manjuan_enemy = enemy end
		end
		if prior_enemy and kongcheng_enemy and manjuan_enemy then break end
	end
	
	-- Enemy -> Friend
	if least_friend then
		local tg_enemy 
		if not tg_enemy and prior_enemy and prior_enemy:getHandcardNum() > least_friend:getHandcardNum() then tg_enemy = prior_enemy end
		if not tg_enemy and most_enemy  and most_enemy:getHandcardNum() > least_friend:getHandcardNum() then tg_enemy = most_enemy end
		if tg_enemy  then
			use.card = card
			if use.to then
				use.to:append(tg_enemy)
				use.to:append(least_friend)
			end
			if not use.isDummy then
				sgs.updateIntention(self.player, tg_enemy, intention)
				sgs.updateIntention(self.player, least_friend, -intention)
			end
			return
		end

		if most_enemy and most_enemy:getHandcardNum() > least_friend:getHandcardNum() then
			use.card = card
			if use.to then
				use.to:append(most_enemy)
				use.to:append(least_friend)
			end
			if not use.isDummy then
				sgs.updateIntention(self.player, most_enemy, intention)
				sgs.updateIntention(self.player, least_friend, -intention)
			end
			return
		end

	end
	
	
	self:sort(enemies,"defense")
	if least_friend then
		for _,enemy in ipairs(enemies) do
			local hand1 = enemy:getHandcardNum()
			local hand2 = least_friend:getHandcardNum()

			if (hand1 > hand2) then
				use.card=card
				if use.to then
					use.to:append(enemy)
					use.to:append(least_friend)
					return
				end
			end
		end
	end
	


	self:sort(enemies, "handcard", true)
	-- Friend -> Friend
	if #friends >= 2 then
		if need_kongcheng_friend and least_friend:isKongcheng() then
			use.card = card
			if use.to then
				use.to:append(need_kongcheng_friend)
				use.to:append(least_friend)
			end
			if not use.isDummy then
				sgs.updateIntention(self.player, need_kongcheng_friend, -intention)
				sgs.updateIntention(self.player, least_friend, -intention)
			end
			return
		elseif most_friend:getHandcardNum() >= 4 and most_friend:getHandcardNum() > least_friend:getHandcardNum() then
			use.card = card
			if use.to then
				use.to:append(most_friend)
				use.to:append(least_friend)
			end
			if not use.isDummy then sgs.updateIntention(self.player, least_friend, -intention) end
			return
		end
	end
	
	-- Enemy -> Enemy
	if kongcheng_enemy and not kongcheng_enemy:hasSkill("manjuan") then
		local tg_enemy = prior_enemy or most_enemy
		if tg_enemy and not tg_enemy:isKongcheng() then
			use.card = card
			if use.to then
				use.to:append(tg_enemy)
				use.to:append(kongcheng_enemy)
			end
			if not use.isDummy then
				sgs.updateIntention(self.player, tg_enemy, intention)
				sgs.updateIntention(self.player, kongcheng_enemy, intention)
			end
			return
		elseif most_friend and most_friend:getHandcardNum() >= 4 then -- Friend -> Enemy for KongCheng
			use.card = card
			if use.to then
				use.to:append(most_friend)
				use.to:append(kongcheng_enemy)
			end
			if not use.isDummy then sgs.updateIntention(self.player, kongcheng_enemy, intention) end
			return
		end
	elseif manjuan_enemy then
		local tg_enemy = prior_enemy or most_enemy
		if tg_enemy and tg_enemy:getHandcardNum() > manjuan_enemy:getHandcardNum() then
			use.card = card
			if use.to then
				use.to:append(tg_enemy)
				use.to:append(manjuan_enemy)
			end
			if not use.isDummy then sgs.updateIntention(self.player, tg_enemy, intention) end
			return
		end
	elseif most_enemy then
		local tg_enemy, second_enemy
		if prior_enemy then
			for _, enemy in ipairs(enemies) do
				if enemy:getHandcardNum() < prior_enemy:getHandcardNum() then
					second_enemy = enemy
					tg_enemy = prior_enemy
					break
				end
			end
		end
		if not second_enemy then
			tg_enemy = most_enemy
			for _, enemy in ipairs(enemies) do
				if enemy:getHandcardNum() < most_enemy:getHandcardNum() then
					second_enemy = enemy
					break
				end
			end
		end
		if tg_enemy and second_enemy then
			use.card = card
			if use.to then
				use.to:append(tg_enemy)
				use.to:append(second_enemy)
			end
			if not use.isDummy then
				sgs.updateIntention(self.player, tg_enemy, intention)
				sgs.updateIntention(self.player, second_enemy, intention)
			end
			return
		end
	end
end

sgs.ai_card_intention.AnxuCard = 0
sgs.ai_use_priority.AnxuCard = 9.6
sgs.ai_chaofeng.bulianshi = 4

sgs.ai_skill_playerchosen.zhuiyi = function(self, targets)
	local first, second
	targets = sgs.QList2Table(targets)
	self:sort(targets,"defense")
	for _, friend in ipairs(targets) do
		if self:isFriend(friend) and friend:isAlive() and not (friend:hasSkill("manjuan") and friend:getPhase() == sgs.Player_NotActive and friend:getLostHp() == 0) then
			if isLord(friend) and self:isWeak(friend) then return friend end
			if not (friend:hasSkill("zhiji") and friend:getMark("zhiji") == 0 and not self:isWeak(friend) and friend:getPhase() == sgs.Player_NotActive) then
				if sgs.evaluatePlayerRole(friend) == "renegade" then second = friend
				elseif sgs.evaluatePlayerRole(friend) ~= "renegade" and not first then first = friend
				end
			end
		end
	end
	return first or second
end


function sgs.ai_cardneed.lihuo(to, card)
	local slash = card:isKindOf("Slash") and not (card:isKindOf("FireSlash") or card:isKindOf("ThunderSlash"))
	return (card:isKindOf("FireSlash") and getKnownCard(to, "FireSlash", false) == 0) or (slash and getKnownCard(to, "Slash", false) == 0)
end

sgs.ai_skill_invoke.lihuo = function(self, data)
	if not sgs.ai_skill_invoke.Fan(self, data) then return false end
	local use = data:toCardUse()
	for _, player in sgs.qlist(use.to) do
		if self:isEnemy(player) and self:damageIsEffective(player, sgs.DamageStruct_Fire) and sgs.isGoodTarget(player, self.enemies, self) then
			if player:isChained() then return self:isGoodChainTarget(player) end
			if player:hasArmorEffect("Vine") then return true end
		end
	end
	return false
end

sgs.ai_view_as.lihuo = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if sgs.Sanguosha:getCurrentCardUseReason() ~= sgs.CardUseStruct_CARD_USE_REASON_RESPONSE
		and card_place ~= sgs.Player_PlaceSpecial and card:objectName() == "slash" then
		return ("fire_slash:lihuo[%s:%s]=%d"):format(suit, number, card_id)
	end
end

local lihuo_skill={}
lihuo_skill.name="lihuo"
table.insert(sgs.ai_skills,lihuo_skill)
lihuo_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)
	local slash_card
	
	for _,card in ipairs(cards)  do
		if card:isKindOf("Slash") and not (card:isKindOf("FireSlash") or card:isKindOf("ThunderSlash")) then
			slash_card = card
			break
		end
	end

	if not slash_card then return nil end
	local dummy_use = { to = sgs.SPlayerList(), isDummy = true }
	self:useCardFireSlash(slash_card, dummy_use)
	if dummy_use.card and dummy_use.to:length() > 0 then
		local use = sgs.CardUseStruct()
		use.from = self.player
		use.to = dummy_use.to
		use.card = slash_card
		local data = sgs.QVariant()
		data:setValue(use)
		if not sgs.ai_skill_invoke.lihuo(self, data) then return nil end
	else return nil end

	local suit = slash_card:getSuitString()
	local number = slash_card:getNumberString()
	local card_id = slash_card:getEffectiveId()
	local card_str = ("fire_slash:lihuo[%s:%s]=%d"):format(suit, number, card_id)
	local fireslash = sgs.Card_Parse(card_str)
	assert(fireslash)
	
	return fireslash
		
end


function sgs.ai_cardneed.chunlao(to, card)
	return card:isKindOf("Slash") and to:getPile("wine"):isEmpty()
end

sgs.ai_skill_use["@@chunlao"] = function(self, prompt)
	local slashcards={}
	local chunlao = self.player:getPile("wine")
	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)
	for _,card in ipairs(cards)  do
		if card:isKindOf("Slash") then
			table.insert(slashcards,card:getId()) 
		end
	end
	if #slashcards > 0 and chunlao:isEmpty() then 
		return "@ChunlaoCard="..table.concat(slashcards,"+").."->".."." 
	end
	return "."
end

function sgs.ai_cardsview_valuable.chunlao(self, class_name, player)
	if class_name == "Peach" and player:getPile("wine"):length() > 0 then
		local dying = player:getRoom():getCurrentDyingPlayer()
		if dying then
			local analeptic = sgs.Sanguosha:cloneCard("analeptic")
			if dying:isLocked(analeptic) then return nil end
			return "@ChunlaoWineCard=."
		end
	end
end

sgs.ai_card_intention.ChunlaoWineCard = sgs.ai_card_intention.Peach

function sgs.ai_cardneed.chunlao(to, card)
	return to:getPile("wine"):isEmpty() and card:isKindOf("Slash")
end

sgs.chunlao_keep_value = {
	Peach = 6,
	Jink = 5.1,
	Slash = 5.5,
}

sgs.ai_skill_invoke.zhiyu = function(self, data)
	local manjuan = hasManjuanEffect(self.player)
	local damage = data:toDamage()
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	local first
	local difcolor = 0
	for _,card in ipairs(cards)  do
		if not first then first = card end
		if (first:isRed() and card:isBlack()) or (card:isRed() and first:isBlack()) then
			difcolor = 1
			break
		end
	end
	if difcolor == 0 and damage.from then
		if self:isFriend(damage.from) and (not damage.from:isKongcheng() or manjuan) then
			return false
		elseif self:isEnemy(damage.from) then
			if manjuan and self.player:isKongcheng() then return false end
			if self:doNotDiscard(damage.from, "h") and not damage.from:isKongcheng() then return false end
			return true
		end
	end
	if manjuan then return false end
	return true
end

local function get_handcard_suit(cards)
	if #cards == 0 then return sgs.Card_NoSuit end
	if #cards == 1 then return cards[1]:getSuit() end
	local black = false
	if cards[1]:isBlack() then black = true end
	for _, c in ipairs(cards) do
		if black ~= c:isBlack() then return sgs.Card_NoSuit end
	end
	return black and sgs.Card_NoSuitBlack or sgs.Card_NoSuitRed
end

local qice_skill = {}
qice_skill.name = "qice"
table.insert(sgs.ai_skills, qice_skill)
qice_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("QiceCard") or self.player:isKongcheng() then return end
	local cards = self.player:getHandcards()
	local allcard = {}
	cards = sgs.QList2Table(cards)
	local suit = get_handcard_suit(cards)
	local aoename = "savage_assault|archery_attack"
	local aoenames = aoename:split("|")
	local aoe
	local i
	local good, bad = 0, 0
	local caocao = self.room:findPlayerBySkillName("jianxiong")
	local qicetrick = "savage_assault|archery_attack|ex_nihilo|god_salvation"
	local qicetricks = qicetrick:split("|")
	local aoe_available, ge_available, ex_available = true, true, true
	for i = 1, #qicetricks do
		local forbiden = qicetricks[i]
		forbid = sgs.Sanguosha:cloneCard(forbiden, suit)
		if self.player:isCardLimited(forbid, sgs.Card_MethodUse, true) or not forbid:isAvailable(self.player) then
			if forbid:isKindOf("AOE") then aoe_available = false end
			if forbid:isKindOf("GlobalEffect") then ge_available = false end
			if forbid:isKindOf("ExNihilo") then ex_available = false end
		end
	end
	if self.player:hasUsed("QiceCard") then return end
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() then
			good = good + 10 / friend:getHp()
			if friend:isLord() then good = good + 10 / friend:getHp() end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if enemy:isWounded() then
			bad = bad + 10 / enemy:getHp()
			if enemy:isLord() then
				bad = bad + 10 / enemy:getHp()
			end
		end
	end

	for _, card in ipairs(cards) do
		table.insert(allcard, card:getId())
	end

	local godsalvation = sgs.Sanguosha:cloneCard("god_salvation", suit, 0)
	if self.player:getHandcardNum() < 3 then
		if aoe_available then
			for i = 1, #aoenames do
				local newqice = aoenames[i]
				aoe = sgs.Sanguosha:cloneCard(newqice)
				if self:getAoeValue(aoe) > 0 then
					local parsed_card = sgs.Card_Parse("@QiceCard=" .. table.concat(allcard, "+") .. ":" .. newqice)
					return parsed_card
				end
			end
		end
		if ge_available and self:willUseGodSalvation(godsalvation) then
			local parsed_card = sgs.Card_Parse("@QiceCard=" .. table.concat(allcard, "+") .. ":" .. "god_salvation")
			return parsed_card
		end
		if ex_available and self:getCardsNum("Jink") == 0 and self:getCardsNum("Peach") == 0 then
			local parsed_card = sgs.Card_Parse("@QiceCard=" .. table.concat(allcard, "+") .. ":" .. "ex_nihilo")
			return parsed_card
		end
	end

	if self.player:getHandcardNum() == 3 then
		if aoe_available then
			for i = 1, #aoenames do
				local newqice = aoenames[i]
				aoe = sgs.Sanguosha:cloneCard(newqice)
				if self:getAoeValue(aoe) > 0 then
					local parsed_card = sgs.Card_Parse("@QiceCard=" .. table.concat(allcard, "+") .. ":" .. newqice)
					return parsed_card
				end
			end
		end
		if ge_available and self:willUseGodSalvation(godsalvation) and self.player:isWounded() then
			local parsed_card = sgs.Card_Parse("@QiceCard=" .. table.concat(allcard, "+") .. ":" .. "god_salvation")
			return parsed_card
		end
		if ex_available and self:getCardsNum("Jink") == 0 and self:getCardsNum("Peach") == 0 and self:getCardsNum("Analeptic") == 0 and self:getCardsNum("Nullification") == 0 then
			local parsed_card = sgs.Card_Parse("@QiceCard=" .. table.concat(allcard, "+") .. ":" .. "ex_nihilo")
			return parsed_card
		end
	end
	if aoe_available then
		for i = 1, #aoenames do
			local newqice = aoenames[i]
			aoe = sgs.Sanguosha:cloneCard(newqice)
			if self:getAoeValue(aoe) > -5 and caocao and self:isFriend(caocao) and caocao:getHp() > 1 and not self:willSkipPlayPhase(caocao)
				and not self.player:hasSkill("jueqing") and self:aoeIsEffective(aoe, caocao, self.player) then
				local parsed_card = sgs.Card_Parse("@QiceCard=" .. table.concat(allcard, "+") .. ":" .. newqice)
				return parsed_card
			end
		end
	end
	if self:getCardsNum("Jink") == 0 and self:getCardsNum("Peach") == 0 and self:getCardsNum("Analeptic") == 0
		and self:getCardsNum("Nullification") == 0 and self.player:getHandcardNum() <= 3 then
		if ge_available and self:willUseGodSalvation(godsalvation) and self.player:isWounded() then
			local parsed_card = sgs.Card_Parse("@QiceCard=" .. table.concat(allcard, "+") .. ":" .. "god_salvation")
			return parsed_card
		end
		if ex_available then
			local parsed_card = sgs.Card_Parse("@QiceCard=" .. table.concat(allcard, "+") .. ":" .. "ex_nihilo")
			return parsed_card
		end
	end
end

sgs.ai_skill_use_func.QiceCard = function(card, use, self)
	local userstring = card:toString()
	userstring = (userstring:split(":"))[3]
	local qicecard = sgs.Sanguosha:cloneCard(userstring, card:getSuit(), card:getNumber())
	qicecard:setSkillName("qice")
	self:useTrickCard(qicecard, use)
	if use.card then
		for _, acard in sgs.qlist(self.player:getHandcards()) do
			if isCard("Peach", acard, self.player) and self.player:getHandcardNum() > 1 and self.player:isWounded()
				and not self:needToLoseHp(self.player) then
					use.card = acard
					return
			end
		end	
		use.card = card
	end
end

sgs.ai_use_priority.QiceCard = 1.5
sgs.ai_chaofeng.xunyou = 2