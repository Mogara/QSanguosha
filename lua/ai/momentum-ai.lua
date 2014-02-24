function sgs.ai_skill_invoke.wangxi(self, data)
	local target = data:toPlayer()
	if target and (self.player:isFriendWith(target) or self:isFriend(target)) then
		return not self:needKongcheng(target, true)
	else
		return self:needKongcheng(target, true)
	end
end

sgs.ai_choicemade_filter.skillInvoke.wangxi = function(self, player, promptlist)
	local damage = self.room:getTag("CurrentDamageStruct"):toDamage()
	local target = nil
	if damage.from and damage.from:objectName() == player:objectName() then
		target = damage.to
	elseif damage.to and damage.to:objectName() == player:objectName() then
		target = damage.from
	end
	if target and promptlist[3] == "yes" then
		if self:needKongcheng(target, true) then sgs.updateIntention(player, target, 10)
		elseif player:getState() == "robot" then sgs.updateIntention(player, target, -60)
		end
	end
end

function sgs.ai_skill_invoke.hengjiang(self, data)
	local target = data:toPlayer()
	if not target then return end
	if self:isEnemy(target) then
		return true
	else
		if target:getPhase() > sgs.Player_Discard then return true end
		if target:hasSkill("keji") and not target:hasFlag("KejiSlashInPlayPhase") then return true end
		return target:getHandcardNum() <= target:getMaxCards() - 2
	end
end

sgs.ai_choicemade_filter.skillInvoke.hengjiang = function(self, player, promptlist)
	if promptlist[3] == "yes" then
		local current = self.room:getCurrent()
		if current and current:getPhase() <= sgs.Player_Discard
			and not (current:hasSkill("keji") and not current:hasFlag("KejiSlashInPlayPhase")) and current:getHandcardNum() > current:getMaxCards() - 2 then
			sgs.updateIntention(player, current, 50)
		end
	end
end

sgs.ai_skill_invoke.qianxi = function(self, data)
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
				if getKnownCard(enemy, self.player, "Jink", false, "h") > 0 and self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self) then return enemy end
			end
			for _, enemy in ipairs(enemies) do
				if getKnownCard(enemy, self.player, "Peach", true, "h") > 0 or enemy:hasSkill("jijiu") then return enemy end
			end
			for _, enemy in ipairs(enemies) do
				if getKnownCard(enemy, self.player, "Jink", false, "h") > 0 and self:slashIsEffective(slash, enemy) then return enemy end
			end
		end
		return enemies[1]
	end
	return targets:first()
end

sgs.ai_playerchosen_intention.qianxi = 80

sgs.ai_skill_invoke.guixiu = true

local cunsi_skill = {}
cunsi_skill.name = "cunsi"
table.insert(sgs.ai_skills, cunsi_skill)
cunsi_skill.getTurnUseCard = function(self)
	return sgs.Card_Parse("@CunsiCard=.&cunsi")
end

sgs.ai_skill_use_func.CunsiCard = function(card, use, self)
	if self.player:aliveCount() > 2 and #self.friends_noself == 0 then return end
	local to
	for _, friend in ipairs(self.friends_noself) do
		if self:evaluateKingdom(friend) == self.player:getKingdom() then
			to = friend
			break
		end
	end
	if not to then to = self.player end
	use.card = card
	if use.to then use.to:append(to) end
end

sgs.ai_skill_invoke.yongjue = true

sgs.ai_cardneed.jiang = function(to, card, self)
	return isCard("Duel", card, to) or (isCard("Slash", card, to) and card:isRed())
end

sgs.ai_suit_priority.jiang = function(self, card)
	return (card:isKindOf("Slash") or card:isKindOf("Duel")) and "diamond|heart|club|spade" or "club|spade|diamond|heart"
end

sgs.ai_skill_invoke.yingyang = function(self, data)
	local pindian = data:toPindian()
	local f_num, t_num = pindian.from_number, pindian.to_number
	if math.abs(f_num - t_num) <= 3 then return true end
	return false
end

sgs.ai_skill_choice.yingyang = function(self, choices, data)
	local pindian = data:toPindian()
	local reason = pindian.reason
	local from, to = pindian.from, pindian.to
	local f_num, t_num = pindian.from_number, pindian.to_number
	local amFrom = self.player:objectName() == from:objectName()

	local table_pindian_friends = { "tianyi", "shuangren" }
	if reason == "quhu" then
		if amFrom and self.player:hasSkill("jieming") then
			if f_num > 8 then return "jia3"
			elseif self:getJiemingChaofeng(player) <= -6 then return "jian3"
			end
		end
		return "jia3"
	elseif table.contains(table_pindian_friends, reason) then
		return (not amFrom and self:isFriend(from)) and "jian3" or "jia3"
	else
		return "jia3"
	end
end

sgs.ai_skill_invoke.hunshang = true

sgs.ai_skill_invoke.sunce_yingzi = sgs.ai_skill_invoke.yingzi
sgs.ai_skill_choice.sunce_yinghun = sgs.ai_skill_choice.yinghun
sgs.ai_skill_playerchosen.sunce_yinghun = sgs.ai_skill_playerchosen.yinghun
sgs.ai_playerchosen_intention.sunce_yinghun = sgs.ai_playerchosen_intention.yinghun
sgs.ai_choicemade_filter.skillChoice.sunce_yinghun = sgs.ai_choicemade_filter.skillChoice.yinghun

local duanxie_skill = {}
duanxie_skill.name = "duanxie"
table.insert(sgs.ai_skills, duanxie_skill)
duanxie_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("DuanxieCard") then return end
	return sgs.Card_Parse("@DuanxieCard=.&duanxie")
end

sgs.ai_skill_use_func.DuanxieCard = function(card, use, self)
	self:sort(self.enemies, "defense")
	local target
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isChained() and not self:getDamagedEffects(enemy) and not self:needToLoseHp(enemy) and sgs.isGoodTarget(enemy, self.enemies, self) then
			target = enemy
			break
		end
	end
	if not target then return end
	if not self:isWeak() or self.player:isChained() then
		use.card = card
		if use.to then use.to:append(target) end
	end
end

sgs.ai_card_intention.DuanxieCard = 60
sgs.ai_use_priority.DuanxieCard = 0

sgs.ai_skill_invoke.fenming = function(self)
	local value, count = 0, 0
	for _, player in sgs.qlist(self.room:getAllPlayers()) do
		if player:isChained() then
			count = count + 1
			if self:isFriend(player) then
				if self:needToThrowArmor(player) then
					value = value + 1
				elseif player:getHandcardNum() == 1 and self:needKongcheng(player) then
					value = value + 0.2
				elseif self.player:canDiscard(player, "he") then
					local dec = self:isWeak(player) and 1.2 or 0.8
					if player:objectName() == self.player:objectName() then dec = dec / 1.5 end
					if self:getOverflow(player) >= 0 then dec = dec / 1.5 end
					value = value - dec
				end
			elseif self:isEnemy(player) then
				if self.player:canDiscard(player, "he") then
					if self:doNotDiscard(player) then
						value = value - 0.8
					else
						local dec = self:isWeak(player) and 1.2 or 0.8
						if self:getValuableCard(player) or self:getDangerousCard(player) then dec = dec * 1.5 end
						value = value + dec
					end
				end
			end
		end
	end
	--self.room:writeToConsole(value / count)
	return value / count >= 0.2
end

sgs.ai_skill_invoke.hengzheng = function(self, data)
	local value = 0
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		value = value + self:getGuixinValue(player)
	end
	return value >= 1.3
end

sgs.ai_skill_choice.benghuai = function(self, choices, data)
	for _, friend in ipairs(self.friends) do
		if friend:hasSkill("tianxiang") and (self.player:getHp() >= 3 or (self:getCardsNum("Peach") + self:getCardsNum("Analeptic") > 0 and self.player:getHp() > 1)) then
			return "hp"
		end
	end
	if self.player:getMaxHp() >= self.player:getHp() + 2 then
		if self.player:getMaxHp() > 5 and (self.player:hasSkills("yinghun|zaiqi") and self:findPlayerToDraw(false)) then
			local enemy_num = 0
			for _, p in ipairs(self.enemies) do
				if p:inMyAttackRange(self.player) and not self:willSkipPlayPhase(p) then enemy_num = enemy_num + 1 end
			end
			local ls = sgs.fangquan_effect and self.room:findPlayerBySkillName("fangquan")
			if ls then
				sgs.fangquan_effect = false
				enemy_num = self:getEnemyNumBySeat(ls, self.player, self.player)
			end
			if (self:getCardsNum("Peach") + self:getCardsNum("Analeptic") + self.player:getHp() > 1) then return "hp" end
		end
		return "maxhp"
	else
		return "hp"
	end
end

sgs.ai_skill_invoke.chuanxin = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to) and not self:hasHeavySlashDamage(self.player, damage.card, damage.to)
end

sgs.ai_skill_choice.chuanxin = "discard"

sgs.ai_skill_invoke.wuxin = true

local wendao_skill = {}
wendao_skill.name = "wendao"
table.insert(sgs.ai_skills, wendao_skill)
wendao_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("WendaoCard") then
		local invoke = "no"
		local discardpile = self.room:getDiscardPile()
		local owner = nil
		for _, i in sgs.qlist(discardpile) do
			if sgs.Sanguosha:getCard(i):objectName() == "PeaceSpell" then
				invoke = "di"
				break
			end
		end
		if invoke == "no" then
			for _, p in sgs.qlist(self.room:getAlivePlayers()) do
				if p:getArmor() and p:getArmor():objectName() == "PeaceSpell" then
					invoke = "eq"
					owner = p
					break
				end
			end
		end
		if invoke ~= "no" then
			if invoke == "eq" then
				assert(owner)
				if owner:hasArmorEffect("PeaceSpell") then
					if (owner:objectName() == self.player:objectName()) then
						if (not self.player:hasSkill("hongfa")) or (self.player:getPile("heavenly_army"):isEmpty()) then
							if self.player:getHp() <= 1 then return nil end
						end
					else
						if (self.player:isFriendWith(owner)) then
							if not self:needToLoseHp(owner, self.player) then return nil end
							if owner:isChained() then return nil end
						else
							if self:needToLoseHp(owner, self.player) then return nil end
						end
					end
				end
			end
			local cards = sgs.QList2Table(self.player:getCards("he"))
			self:sortByKeepValue(cards)
			local cards_copy = {}
			for _, c in ipairs(cards) do
				table.insert(cards_copy, c)
			end
			for _, c in ipairs(cards_copy) do
				if c:objectName() == "PeaceSpell" then
					return sgs.Card_Parse("@WendaoCard=" .. c:getEffectiveId() .. "&wendao")
				end
				if (not c:isRed()) or isCard("Peach", c, self.player) then table.removeOne(cards, c) end
			end
			if #cards == 0 then return nil end
			return sgs.Card_Parse("@WendaoCard=" .. cards[1]:getEffectiveId() .. "&wendao")
		end
	end
	return nil
end

sgs.ai_skill_use_func.WendaoCard = function(card, use, self)
	use.card = card
end

sgs.ai_event_callback[sgs.EventPhaseStart].wendao = function(self, player, data)
	if player:hasSkills("wendao+hongfa") then
		if player:getArmor() and player:getArmor():objectName() == "PeaceSpell" and not player:getPile("heavenly_army"):isEmpty() then
			sgs.ai_use_priority.WendaoCard = 11
		else
			sgs.ai_use_priority.WendaoCard = sgs.ai_use_priority.ZhihengCard
		end
	end
end

sgs.ai_use_priority.WendaoCard = sgs.ai_use_priority.ZhihengCard

sgs.ai_skill_invoke.hongfa = true

sgs.ai_skill_invoke.hongfa_slash_resp = function(self, data)
	local asked = data:toStringList()
	local prompt = asked[2]
	if self:askForCard("slash", prompt, 1) == "." then return false end
	
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if isCard("Slash", card, self.player) then
			return false
		end
	end
	
	return true
end

local hongfa_slash_skill = {}
hongfa_slash_skill.name = "hongfa_slash"
table.insert(sgs.ai_skills, hongfa_slash_skill)
hongfa_slash_skill.getTurnUseCard = function(self)
	self.HongfaSlashCard_target = nil
	self.HongfaCard_id = nil
	if not self:slashIsAvailable() then return end
	if self.player:hasUsed("HongfaCard") then return end
	local lord = self.player:getLord()
	if lord and lord:hasLordSkill("hongfa") and not lord:getPile("heavenly_army"):isEmpty() then
		local hongfa_slash = sgs.Card_Parse("@HongfaCard=.&hongfa_slash")
		assert(hongfa_slash)
		return hongfa_slash
	end
	return
end

sgs.ai_skill_use_func.HongfaCard = function(card, use, self)
	local ids = self.player:getLord():getPile("heavenly_army")
	for _, id in sgs.qlist(ids) do
		local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_SuitToBeDecided, -1)
		slash:addSubcard(id)
		local dummy_use = { isDummy = true, to = sgs.SPlayerList() }
		self:useCardSlash(slash, dummy_use)
		if dummy_use.card and dummy_use.to:length() > 0 then
			use.card = card
			if not use.isDummy then
				local t = {}
				for _, to in sgs.qlist(dummy_use.to) do
					table.insert(t, to:objectName())
				end
				self.HongfaCard_target = table.concat(t, "+")
				self.HongfaCard_id = id
			end
		end
	end
end

sgs.ai_skill_use["@@hongfa_slash!"] = function(self, prompt)
	assert(self.HongfaCard_target)
	return "@HongfaSlashCard=.&hongfa_slash->" .. self.HongfaCard_target
end

sgs.ai_skill_askforag.hongfa = function(self, card_ids)
	if self.HongfaCard_id then
		return self.HongfaCard_id
	end
	return card_ids[1]
end

sgs.ai_use_priority.HongfaCard = sgs.ai_use_priority.Slash

sgs.ai_slash_prohibit.PeaceSpell = function(self, from, enemy, card)
	if enemy:hasArmorEffect("PeaceSpell") and card:isKindOf("NatureSlash") then return true end
	return
end
function sgs.ai_armor_value.PeaceSpell(player, self)
	if player:hasSkills("hongfa+wendao") then return 1000 end
	if getCardsNum("Peach", player, player) + getCardsNum("Analeptic", player, player) == 0 and player:getHp() == 1 then return 9 end
	return 3.5
end

PeaceSpell_damageeffect = function(self, to, nature, from)
	if to:hasArmorEffect("PeaceSpell") and nature ~= sgs.DamageStruct_Normal then return false end
	return true
end
table.insert(sgs.ai_damage_effect, PeaceSpell_damageeffect)

