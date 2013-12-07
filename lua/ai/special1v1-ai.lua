function SmartAI:useCardDrowning(card, use)
	if self.player:hasSkill("noswuyan") or (self.player:hasSkill("wuyan") and not self.player:hasSkill("jueqing")) then return end
	self:sort(self.enemies)
	local targets, equip_enemy = {}, {}
	for _, enemy in ipairs(self.enemies) do
		if (not use.current_targets or not table.contains(use.current_targets, enemy:objectName()))
			and self:hasTrickEffective(card, enemy) and self:damageIsEffective(enemy) and self:canAttack(enemy)
			and not self:getDamagedEffects(enemy, self.player) and not self:needToLoseHp(enemy, self.player) then
			if enemy:hasEquip() then table.insert(equip_enemy, enemy)
			else table.insert(targets, enemy)
			end
		end
	end
	if not (self.player:hasSkill("wumou") and self.player:getMark("@wrath") < 7) then
		if #equip_enemy > 0 then
			local function cmp(a, b)
				return a:getEquips():length() >= b:getEquips():length()
			end
			table.sort(equip_enemy, cmp)
			for _, enemy in ipairs(equip_enemy) do
				if not self:needToThrowArmor(enemy) then table.insert(targets, enemy) end
			end
		end
		for _, friend in ipairs(self.friends_noself) do
			if not (not use.current_targets or not table.contains(use.current_targets, friend:objectName())) and self:needToThrowArmor(friend) then
				table.insert(targets, friend)
			end
		end
	end
	if #targets > 0 then
		local targets_num = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, card)
		local lx = self.room:findPlayerBySkillName("huangen")
		use.card = card
		if use.to then
			for i = 1, targets_num, 1 do
				if not (use.to:length() > 0 and targets[i]:hasSkill("danlao"))
					and not (use.to:length() > 0 and lx and self:isFriend(lx, targets[i]) and self:isEnemy(lx) and lx:getHp() > targets_num / 2) then
					use.to:append(targets[i])
					if #targets == i then break end
				end
			end
		end
	end
end

sgs.ai_card_intention.Drowning = function(self, card, from, tos)
	for _, to in ipairs(tos) do
		if not self:hasTrickEffective(card, to, from) or not self:damageIsEffective(to, sgs.DamageStruct_Normal, from)
			or self:needToThrowArmor(to) then
		else
			sgs.updateIntention(from, to, 80)
		end
	end
end

sgs.ai_use_value.Drowning = 5
sgs.ai_use_priority.Drowning = 7

sgs.ai_skill_choice.drowning = function(self, choices, data)
	local effect = data:toCardEffect()
	if not self:damageIsEffective(self.player, sgs.DamageStruct_Normal, effect.from)
		or self:needToLoseHp(self.player, effect.from)
		or self:getDamagedEffects(self.player, effect.from) then return "damage" end
	if self:isWeak() and not self:needDeath() then return "throw" end

	local value = 0
	for _, equip in sgs.qlist(self.player:getEquips()) do
		if equip:isKindOf("Weapon") then value = value + self:evaluateWeapon(equip)
		elseif equip:isKindOf("Armor") then
			value = value + self:evaluateArmor(equip)
			if self:needToThrowArmor() then value = value - 5 end
		elseif equip:isKindOf("OffensiveHorse") then value = value + 2.5
		elseif equip:isKindOf("DefensiveHorse") then value = value + 5
		end
	end
	if value < 8 then return "throw" else return "damage" end
end

sgs.ai_skill_playerchosen.koftuxi = function(self, targets)
	local cardstr = sgs.ai_skill_use["@@tuxi"](self, "@tuxi")
	if cardstr:match("->") then
		local targetstr = cardstr:split("->")[2]:split("+")
		if #targetstr > 0 then
			local target = findPlayerByObjectName(self.room, targetstr[1])
			return target
		end
	end
	return nil
end

sgs.ai_playerchosen_intention.koftuxi = function(self, from, to)
	local lord = self.room:getLord()
	if sgs.evaluatePlayerRole(from) == "neutral" and sgs.evaluatePlayerRole(to) == "neutral"
		and lord and not lord:isKongcheng()
		and not self:doNotDiscard(lord, "h", true) and from:aliveCount() >= 4 then
		sgs.updateIntention(from, lord, -35)
		return
	end
	if from:getState() == "online" then
		if (to:hasSkills("kongcheng|zhiji|lianying") and to:getHandcardNum() == 1) or to:hasSkills("tuntian+zaoxian") then
		else
			sgs.updateIntention(from, to, 80)
		end
	else
		local intention = from:hasFlag("tuxi_isfriend_" .. to:objectName()) and -5 or 80
		sgs.updateIntention(from, to, intention)
	end
end

sgs.ai_chaofeng.kof_zhangliao = 4

xiechan_skill = {}
xiechan_skill.name = "xiechan"
table.insert(sgs.ai_skills, xiechan_skill)
xiechan_skill.getTurnUseCard = function(self)
	if self.player:getMark("@twine") <= 0 then return end
	self:sort(self.enemies, "handcard")
	if self.player:hasSkill("luoyi") and not self.player:hasFlag("luoyi") then return end
	return sgs.Card_Parse("@XiechanCard=.")
end

sgs.ai_skill_use_func.XiechanCard = function(card, use, self)
	self.player:setFlags("AI_XiechanUsing")
	local max_card = self:getMaxCard()
	self.player:setFlags("-AI_XiechanUsing")
	if max_card:isKindOf("Slash") and self:getCardsNum("Slash") <= 2 then return end
	local max_point = max_card:getNumber()

	local dummy_use = { isDummy = true, xiechan = true, to = sgs.SPlayerList() }
	local duel = sgs.Sanguosha:cloneCard("Duel")
	self:useCardDuel(duel, dummy_use)
	if not dummy_use.card or not dummy_use.card:isKindOf("Duel") then return end
	for _, enemy in sgs.qlist(dummy_use.to) do
		if not enemy:isKongcheng() and not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) then
			local enemy_max_card = self:getMaxCard(enemy)
			local enemy_max_point = enemy_max_card and enemy_max_card:getNumber() or 100
			if max_point > enemy_max_point then
				self.xiechan_card = max_card:getId()
				use.card = card
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end
end

sgs.ai_view_as.kofqingguo = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place == sgs.Player_PlaceEquip then
		return ("jink:kofqingguo[%s:%s]=%d"):format(suit, number, card_id)
	end
end

function sgs.ai_cardneed.kofqingguo(to, card)
	if card:isKindOf("Weapon") then return not to:getWeapon()
	elseif card:isKindOf("Armor") then return not to:getArmor()
	elseif card:isKindOf("OffensiveHorse") then return not to:getOffensiveHorse()
	elseif card:isKindOf("DefensiveHorse") then return not to:getDefensiveHorse()
	end
	return false
end


sgs.ai_skill_invoke.kofliegong = sgs.ai_skill_invoke.liegong

function sgs.ai_cardneed.kofliegong(to, card)
	return isCard("Slash", card, to) and getKnownCard(to, "Slash", true) == 0
end

sgs.ai_skill_invoke.yinli = function(self)
	return not self:needKongcheng(self.player, true)
end

sgs.ai_skill_askforag.yinli = function(self, card_ids)
	if self:needKongcheng(self.player, true) then return card_ids[1] else return -1 end
end

sgs.ai_skill_choice.kofxiaoji = function(self, choices)
	if choices:match("recover") then return "recover" else return "draw" end
end

sgs.kofxiaoji_keep_value = sgs.xiaoji_keep_value

sgs.ai_skill_invoke.suzi = true
sgs.ai_skill_invoke.cangji = true

sgs.ai_skill_use["@@cangji"] = function(self, prompt)
	for i = 0, 3, 1 do
		local equip = self.player:getEquip(i)
		if not equip then continue end
		self:sort(self.friends_noself)
		if i == 0 then
			if equip:isKindOf("Crossbow") or equip:isKindOf("Blade") then
				for _, friend in ipairs(self.friends_noself) do
					if not self:getSameEquip(equip) and not self:hasCrossbowEffect(friend) and getCardsNum("Slash", friend) > 1 then
						return "@CangjiCard=" .. equip:getEffectiveId() .. "->" .. friend:objectName()
					end
				end
			elseif equip:isKindOf("Axe") then
				for _, friend in ipairs(self.friends_noself) do
					if not self:getSameEquip(equip)
						and (friend:getCardCount() >= 4
							or (friend:getCardCount() >= 2 and self:hasHeavySlashDamage(friend))) then
						return "@CangjiCard=" .. equip:getEffectiveId() .. "->" .. friend:objectName()
					end
				end
			end
		end
		for _, friend in ipairs(self.friends_noself) do
			if not self:getSameEquip(equip, friend) and not (i == 1 and (self:evaluateArmor(equip, friend) <= 0 or friend:hasSkills("bazhen|yizhong"))) then
				return "@CangjiCard=" .. equip:getEffectiveId() .. "->" .. friend:objectName()
			end
		end
		if equip:isKindOf("SilverLion") then
			for _, enemy in ipairs(self.enemies) do
				if not enemy:getArmor() and enemy:hasSkills("bazhen|yizhong") then
					return "@CangjiCard=" .. equip:getEffectiveId() .. "->" .. enemy:objectName()
				end
			end
		end
	end
	return "."
end

sgs.ai_card_intention.CangjiCard = function(self, card, from, tos)
	local to = tos[1]
	local equip = sgs.Sanguosha:getCard(card:getEffectiveId())
	if equip:isKindOf("SilverLion") and to:hasSkills("bazhen|yizhong") then
		sgs.updateIntention(from, to, 40)
	else
		sgs.updateIntention(from, to, -40)
	end
end

sgs.ai_skill_invoke.huwei = function(self, data)
	local drowning = sgs.Sanguosha:cloneCard("drowning")
	local dummy_use = { isDummy = true }
	self:useTrickCard(drowning, dummy_use)
	return (dummy_use.card ~= nil)
end

sgs.ai_skill_invoke.xiaoxi = function(self, data)
	local slash = sgs.Sanguosha:cloneCard("slash")
	local dummy_use = { isDummy = true }
	self:useBasicCard(slash, dummy_use)
	return (dummy_use.card ~= nil)
end

sgs.ai_skill_invoke.manyi = function(self, data)
	local sa = sgs.Sanguosha:cloneCard("savage_assault")
	local dummy_use = { isDummy = true }
	self:useTrickCard(sa, dummy_use)
	return (dummy_use.card ~= nil)
end

local mouzhu_skill = {}
mouzhu_skill.name = "mouzhu"
table.insert(sgs.ai_skills, mouzhu_skill)
mouzhu_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("MouzhuCard") then return end
	return sgs.Card_Parse("@MouzhuCard=.")
end

sgs.ai_skill_use_func.MouzhuCard = function(card, use, self)

	local canleiji
	if self.player:hasSkill("leiji") and self:findLeijiTarget(self.player, 51) and self:hasSuit("spade", true) then
		canleiji = true
		self:sort(self.friends_noself, "handcard")
		sgs.reverse(self.friends_noself)
		for _, friend in ipairs(self.friends_noself) do
			if not friend:isKongcheng() and friend:getHandcardNum() < self.player:getHandcardNum() + 2
				and (self:getCardsNum("Jink") > 0 or not IgnoreArmor(friend, self.player) and not self:isWeak() and self:hasEightDiagramEffect()) then
				use.card = card
				if use.to then use.to:append(friend) end
				return
			end
		end
	end

	for _, enemy in ipairs(self.enemies) do	
		if enemy:getHandcardNum() > 0 and  (self:getDamagedEffects(self.player, enemy) or self:needToLoseHp(self.player, enemy)) then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
	
	local first, second, third, fourth
	local slash = self:getCard("Slash")
	local slash_nosuit = sgs.Sanguosha:cloneCard("slash")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHandcardNum() >= self.player:getHandcardNum() + 2 then
			first = enemy
			break
		elseif enemy:getHandcardNum() > 0 then
			if not self:slashIsEffective(slash_nosuit, self.player, nil, enemy) and self:getCardsNum("Slash") > getCardsNum("Slash", enemy) and not second then
				second = enemy
			elseif not enemy:hasSkills("wushuang|mengjin|tieji")
				and not ((enemy:hasSkill("roulin") or enemy:hasWeapon("DoubleSword")) and enemy:getGender() ~= self.player:getGender()) then
				
				if enemy:getHandcardNum() == 1 and slash and not third and self.player:inMyAttackRange(enemy)
					and (self:hasHeavySlashDamage(self.player, slash, enemy) or self.player:hasWeapon("GudingBlade") and not self:needKongcheng(enemy))
					and (not self:isWeak() or self:getCardsNum("Peach") + self:getCardsNum("Analeptic") > 0) then
					third = enemy
				elseif self:getCardsNum("Jink") > 0 and self:getCardsNum("Slash") > getCardsNum("Slash", enemy) and not fourth then
					fourth = enemy
				end
			end
		end
	end
	
	local target
	if canleiji then
		target = fourth and third or first or second
	else
		target = first or second or third or fourth
	end
	if target then
		use.card = card
		if use.to then use.to:append(target) end
		return
	end
	
end

sgs.ai_skill_cardask["@mouzhu-give"] = function(self, data)
	local target = self.room:getCurrent()
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	if self:isFriend(target) then
		if target:hasSkill("nosleiji") then
			local jink, spade
			for _, c in ipairs(cards) do
				if isCard("Jink", c, target) then jink = c:getEffectiveId() end
				if c:getSuit() == sgs.Card_Spade then spade = c:getEffectiveId() end
			end
			if self:hasSuit("spade", true, target) and jink then return jink
			elseif not self:hasEightDiagramEffect(target) and jink then return jink
			elseif spade or jink then return spade or jink
			end
		elseif target:hasSkill("leiji") then
			local jink, black
			for _, c in ipairs(cards) do
				if isCard("Jink", c, target) then jink = c:getEffectiveId() end
				if c:isBlack() then black = c:getEffectiveId() end
			end
			if self:hasSuit("black", true, target) and jink then return jink
			elseif not self:hasEightDiagramEffect(target) and jink then return jink
			elseif black or jink then return black or jink
			end
		end
	else
		if target:hasSkill("nosleiji") then
			for _, c in ipairs(cards) do
				if not c:isKindOf("Peach") and not c:isKindOf("Jink") and c:getSuit() ~= sgs.Card_Spade then
					return c:getEffectiveId()
				end
			end
		elseif target:hasSkill("leiji") then
			for _, c in ipairs(cards) do
				if not c:isKindOf("Peach") and not c:isKindOf("Jink") and not c:isBlack() then
					return c:getEffectiveId()
				end
			end
		end
		for _, c in ipairs(cards) do
			if not c:isKindOf("Peach") then return c:getEffectiveId() end
		end
	end
	
	return cards[1]:getEffectiveId()
end

sgs.ai_skill_choice.mouzhu = function(self, choices)
	local target = self.room:getCurrent()
	local slash = sgs.Sanguosha:cloneCard("slash")
	if target:hasSkills("leiji|nosleiji") then
		if self:isFriend(target) then
			if choices:match("slash") then return "slash" end
		else
			if choices:match("duel") then return "duel" end
		end
	end

	if self:isFriend(target) then
		if (target:hasSkills("leiji|nosleiji") or not self:slashIsEffective(slash, target)) and choices:match("slash") then return "slash" end
		if self:getDamagedEffects(self.player, target) and getCardsNum("Slash", target) >= 1 and choices:match("duel") then return "duel" end
	else
		if target:hasSkills("leiji|nosleiji") and choices:match("duel") then return "duel" end
		if self:getCardsNum("Slash") > getCardsNum("Slash", target) and choices:match("duel") then return "duel" end
	end
	
	if choices:match("slash") then return "slash" else return "duel" end
end

sgs.ai_use_priority.MouzhuCard = 5.5

sgs.ai_card_intention.MouzhuCard = function(self, card, from, tos)
	if not self.player:hasSkills("leiji|nosleiji") then sgs.updateIntention(from, tos[1], 30) end
end

sgs.ai_skill_invoke.yanhuo = function(self, data)
	local opponent = self.room:getOtherPlayers(self.player, true):first()
	return opponent:isAlive() and not self:doNotDiscard(opponent)
end

sgs.ai_skill_playerchosen.yanhuo = function(self, targets)
	local target = self:findPlayerToDiscard(nil, nil, true, targets)
	if target and target:objectName() ~= self.player:objectName() then return target end
end

sgs.ai_skill_invoke.kofkuanggu = function(self, data)
	local zhangbao = self.room:findPlayerBySkillName("yingbing")
	if zhangbao and self:isEnemy(zhangbao)
		and self.player:getPile("incantation"):length() > 0 and sgs.Sanguosha:getCard(self.player:getPile("incantation"):first()):isRed()
		and not self:hasWizard(self.friends) then return false end
	if self.player:isWounded() then return true end
	local zhangjiao = self.room:findPlayerBySkillName("guidao")
	return zhangjiao and self:isFriend(zhangjiao) and not zhangjiao:isNude()
end

local puji_skill = {}
puji_skill.name = "puji"
table.insert(sgs.ai_skills, puji_skill)
puji_skill.getTurnUseCard = function(self, inclusive)
	if not self.player:canDiscard(self.player, "he") or self.player:hasUsed("PujiCard") then return nil end

	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)

	if self:needToThrowArmor() then return sgs.Card_Parse("@PujiCard=" .. self.player:getArmor():getEffectiveId()) end
	for _, card in ipairs(cards) do
		if not self:isValuableCard(card) then
			if card:getSuit() == sgs.Card_Spade then return sgs.Card_Parse("@PujiCard=" .. card:getEffectiveId()) end
		end
	end
	for _, card in ipairs(cards) do
		if not self:isValuableCard(card) and not (self.player:hasSkill("jijiu") and card:isRed() and self:getOverflow() < 2) then
			if card:getSuit() == sgs.Card_Spade then return sgs.Card_Parse("@PujiCard=" .. card:getEffectiveId()) end
		end
	end
end

sgs.ai_skill_use_func.PujiCard = function(card, use, self)
	self.puji_id_choice = nil
	local players = self:findPlayerToDiscard("he", false, true, nil, true)
	for _, p in ipairs(players) do
		local id = self:askForCardChosen(p, "he", "dummyreason", sgs.Card_MethodDiscard)
		if id and (self:isFriend(p) or not p:hasEquip(sgs.Sanguosha:getCard(id)) or sgs.Sanguosha:getCard(id):getSuit() ~= sgs.Card_Spade) then
			self.puji_id_choice = id
			use.card = card
			if use.to then use.to:append(p) end
			return
		end
	end
end

sgs.ai_use_value.PujiCard = 5
sgs.ai_use_priority.PujiCard = 4.6

sgs.ai_card_intention.PujiCard = function(self, card, from, tos)
	if not self.puji_id_choice then return end
	local to = tos[1]
	local em_prompt = { "cardChosen", "puji", tostring(self.puji_id_choice), from:objectName(), to:objectName() }
	sgs.ai_choicemade_filter.cardChosen.snatch(self, nil, em_prompt)
end

sgs.ai_skill_use["@@niluan"] = function(self, prompt)
	return sgs.ai_skill_use.slash(self, prompt)
end

sgs.ai_skill_cardask["@niluan-slash"] = function(self, data, pattern, target, target2)
	if target and self:isFriend(target) and not self:findLeijiTarget(target, 50, self.player) then
		return "."
	end
	if not self.player:canSlash(target, false) then return "." end

	local black_card
	if not target:hasSkill("yizhong") and not target:hasArmorEffect("RenwangShield") and not target:hasArmorEffect("Vine") then
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(cards)
		local slash = sgs.Sanguosha:cloneCard("slash")
		for _, card in ipairs(cards) do
			if card:isBlack() and self:getKeepValue(card, cards) <= self:getKeepValue(slash, cards) then
				local black_slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_SuitToBeDecided, -1)
				black_slash:addSubcard(card)
				if self:slashIsEffective(black_slash, target) then
					black_card = card
					break
				end
			end
		end
		if not black_card then
			local offensive_horse = self.player:getOffensiveHorse()
			if offensive_horse and offensive_horse:isBlack() then
				local black_slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_SuitToBeDecided, -1)
				black_slash:addSubcard(offensive_horse)
				if self:slashIsEffective(black_slash, target) then
					black_card = offensive_horse
				end
			end
		end
	end
	if self:needToThrowArmor() and self.player:getArmor():isBlack() then black_card = self.player:getArmor()
	elseif self:needKongcheng(self.player, true) and self.player:getHandcardNum() == 1 and self.player:getHandcards():first():isBlack() then
		black_card = self.player:getHandcards():first()
	end
	if black_card then
		local black_slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_SuitToBeDecided, -1)
		black_slash:setSkillName("niluan")
		black_slash:addSubcard(black_card)
		return black_slash:toString()
	end
	return "."
end