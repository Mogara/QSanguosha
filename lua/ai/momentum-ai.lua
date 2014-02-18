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
	if not damage.to then return end
	local invoke
	local to = damage.to
	if to:getMark("chuanxin_" .. self.player:objectName()) == 0 and to:getVisibleSkillList():length() > 1 then
		local count = 0
		for _, skill in sgs.qlist(to:getVisibleSkillList()) do
			if not skill:isAttachedLordSkill() then count = count + 1 end
		end
		if count > 1 then
			for _, skill in ipairs(("benghuai|shiyong|yaowu|wumou|chanyuan|jinjiu|tongji"):split("|")) do
				if to:hasSkill(skill) then
					if self:isFriend(to) then return true
					elseif (skill == "benghuai" or skill == "shiyong") and to:getMaxHp() <= 3 then return true
					end
					return false
				end
			end
			invoke = true
		end
	end
	if to:getEquips():length() > 0 then
		if self:isFriend(to) then
			return self:needToLoseHp(to) and to:getEquips():length() == 1
					and (self:needToThrowArmor(to) or (to:hasSkills(sgs.lose_equip_skill)
														and (to:getOffensiveHorse() or (to:getWeapon() and self:evaluateWeapon(to:getWeapon(), to) < 4))))
		elseif to:getHp() >= 2 and to:hasSkills(sgs.lose_equip_skill) and (not to:getArmor() or self:needToThrowArmor(to)) then
			return false
		end
		invoke = true
	end
	return (invoke and self:isEnemy(to) and not self:hasHeavySlashDamage(self.player, damage.card, to))
			or (not invoke and self:isFriend(to))
end

sgs.ai_skill_choice.chuanxin = function(self, choices, data)
	if self.player:hasSkills("benghuai|shiyong}yaowu|wumou|chanyuan|jinjiu|tongji") then return "detach" end
	if self:needToLoseHp(self.player) or self:needToThrowArmor() or self.player:getEquips():length() <= 2 or self.player:hasSkills(sgs.lose_equip_skill) then
		return "throw"
	end
	return (not self:isWeak()) and "throw" or "detach"
end

sgs.ai_skill_choice.chuanxin_lose = function(self, choices, data)
	for _, skill in ipairs(("benghuai|shiyong|yaowu|wumou|chanyuan|jinjiu|tongji|huwei|pianyi|xiaoxi"):split("|")) do
		if self.player:hasSkill(skill) then return skill end
	end
	for _, skill in sgs.qlist(self.player:getVisibleSkillList()) do
		if (skill:getFrequency() == sgs.Skill_Wake and (self.player:getMark(skill:objectName()) > 0 or skill:objectName() == "baoling"))
			or (skill:getFrequency() == sgs.Skill_Limited and self.player:getMark(skill:getLimitMark()) == 0) then
			return skill:objectName()
		end
	end
	if self.player:hasSkill("cuorui") and self.player:getMark("CuoruiSkipJudge") > 0 then return "cuorui" end
	if self.player:hasSkills("paoxiao+huxiao") then return "huxiao" end
	if self.player:hasSkills("fankui+duodao") then return "duodao" end
	if self.player:hasSkills("jilve+wansha") then return "wansha" end
	if self.player:hasSkills("wuqian+wushuang") then return "wushuang" end
	if self.player:hasSkills("tianfu+kanpo") then return "kanpo" end
	if self.player:hasSkills("fuhun|nosfuhun") then
		if choices:matchOne("paoxiao") then return "paoxiao" end
		if choices:matchOne("wusheng") then return "wusheng" end
	end
	if self.player:hasSkill("luoyan") and self.player:getPile("xingwu"):length() > 0 then
		if choices:matchOne("tianxiang") then return "tianxiang" end
		if choices:matchOne("liuli") then return "liuli" end
	end
	if self.player:hasSkill("baobian") then
		for _, skill in ipairs(("paoxiao|shensu|tiaoxin"):split("|")) do
			if self.player:hasSkill(skill) then return skill end
		end
	end
	if self.player:hasSkill("mouduan") and (self.player:getMark("@wu") > 0 or self.player:getMark("@wen") > 0) then
		for _, skill in ipairs(("jiang|qianxun|yingzi|keji"):split("|")) do
			if self.player:hasSkill(skill) then return skill end
		end
	end
	if self.player:hasSkill("huashen") then
		local huashen_skill = self.player:getTag("HuashenSkill"):toString()
		if #huashen_skill > 0 and self.player:hasSkill(huashen_skill) then return huashen_skill end
	end
	if self.player:hasSkill("xiaode") then
		local xiaode_skill = self.player:getTag("XiaodeSkill"):toString()
		if #xiaode_skill > 0 and self.player:hasSkill(xiaode_skill) then return xiaode_skill end
	end
	for _, skill in sgs.qlist(self.player:getVisibleSkillList()) do
		if skill:isLordSkill() then return skill:objectName() end
	end
	for _, skill in ipairs(("guixiu|suishi|weidi|xinsheng|huoshou|lianpo|hongyan|mashu|jueqing|yicong|tannang|feiying"):split("|")) do
		if self.player:hasSkill(skill) then return skill end
	end
end

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
		if not invoke then
			for _, p in sgs.qlist(self.room:getAlivePlayers()) do
				if p:getWeapon() and p:getWeapon():objectName() == "PeaceSpell" then
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
						if not (self.player:hasSkill("qingnang") or (self.player:getHp() >= 3)) then
							return nil
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
			local to_discard
			local cards = sgs.QList2Table(self.player:getCards("he"))
			self:sortByKeepValue(cards)
			local cards_copy = {}
			for _, c in ipairs(cards) do
				table.insert(cards_copy, c)
			end
			for _, c in ipairs(cards_copy) do
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
	if getCardsNum("Peach", player, player) + getCardsNum("Analeptic", player, player) == 0 and player:getHp() == 1 then return 9 end
	return 3.5
end

PeaceSpell_damageeffect = function(self, to, nature, from)
	if to:hasArmorEffect("PeaceSpell") and nature ~= sgs.DamageStruct_Normal then return false end
	return true
end
table.insert(sgs.ai_damage_effect, PeaceSpell_damageeffect)

