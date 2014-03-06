function SmartAI:canAttack(enemy, attacker, nature)
	attacker = attacker or self.player
	nature = nature or sgs.DamageStruct_Normal
	local damage = 1
	if nature == sgs.DamageStruct_Fire and not enemy:hasArmorEffect("SilverLion") then
		if enemy:hasArmorEffect("Vine") then damage = damage + 1 end
		if enemy:getMark("@gale") > 0 then damage = damage + 1 end
	end
	if #self.enemies == 1 then return true end
	if self:getDamagedEffects(enemy, attacker) or (self:needToLoseHp(enemy, attacker, false, true) and #self.enemies > 1) or not sgs.isGoodTarget(enemy, self.enemies, self) then return false end
	if self:objectiveLevel(enemy) <= 2 or self:cantbeHurt(enemy, self.player, damage) or not self:damageIsEffective(enemy, nature, attacker) then return false end
	if nature ~= sgs.DamageStruct_Normal and enemy:isChained() and not self:isGoodChainTarget(enemy, self.player, nature) then return false end
	return true
end


function sgs.isGoodHp(player)
	local goodHp = player:getHp() > 1 or getCardsNum("Peach", player, global_room:getCurrent()) >= 1 or getCardsNum("Analeptic", player, global_room:getCurrent()) >= 1
					or hasBuquEffect(player) or (player:hasSkill("niepan") and player:getMark("@nirvana") > 0)
	if goodHp then
		return goodHp
	elseif sgs.ais[player:objectName()]:getAllPeachNum(player) >= 1 and not global_room:getCurrent():hasSkill("wansha") then
		return true
	end
	return false
end

function sgs.isGoodTarget(player, targets, self, isSlash)
	-- self = self or sgs.ais[player:objectName()]
	local arr = { "jieming", "yiji", "fangzhu" }
	local m_skill = false
	local attacker = global_room:getCurrent()

	if targets and type(targets)=="table" then
		if #targets == 1 then return true end
		local foundtarget = false
		for i = 1, #targets, 1 do
			if sgs.isGoodTarget(targets[i]) and not self:cantbeHurt(targets[i]) then
				foundtarget = true
				break
			end
		end
		if not foundtarget then return true end
	end

	for _, masochism in ipairs(arr) do
		if player:hasSkill(masochism) then
			if masochism == "jieming" and self and self:getJiemingChaofeng(player) > -4 then m_skill = false
			elseif masochism == "yiji" and self and not self:findFriendsByType(sgs.Friend_Draw, player) then m_skill = false
			else
				m_skill = true
				break
			end
		end
	end



	if isSlash and self and (self:hasCrossbowEffect() or self:getCardsNum("Crossbow") > 0) and self:getCardsNum("Slash") > player:getHp() then
		return true
	end

	if m_skill and sgs.isGoodHp(player) then
		return false
	else
		return true
	end
end

function sgs.getDefenseSlash(player, self)
	if not player then return 0 end
	local attacker = global_room:getCurrent()
	local defense = getCardsNum("Jink", player, attacker)

	local knownJink = getKnownCard(player, attacker, "Jink", true)

	if sgs.card_lack[player:objectName()]["Jink"] == 1 and knownJink == 0 then defense = 0 end

	defense = defense + knownJink * 1.2

	local hasEightDiagram = false

	if (player:hasArmorEffect("EightDiagram") or (player:hasSkill("bazhen") and not player:getArmor()))
	  and not IgnoreArmor(attacker, player) then
		hasEightDiagram = true
	end

	if hasEightDiagram then
		defense = defense + 1.3
		if player:hasSkill("tiandu") then defense = defense + 0.6 end
		if player:hasSkill("leiji") then defense = defense + 0.4 end
		if player:hasSkill("hongyan") then defense = defense + 0.2 end
	end

	if player:hasSkill("tuntian") and player:hasSkill("jixi") and getCardsNum("Jink", player) >= 1 then
		defense = defense + 1.5
	end

	if attacker:canSlashWithoutCrossbow() and attacker:getPhase() == sgs.Player_Play then
		local hcard = player:getHandcardNum()
		if attacker:hasSkill("liegong") and (hcard >= attacker:getHp() or hcard <= attacker:getAttackRange()) then defense = 0 end
	end

	local niaoxiang_BA = false
	local jiangqin = global_room:findPlayerBySkillName("niaoxiang")
	if jiangqin then
		if jiangqin:inSiegeRelation(jiangqin, player) then
			niaoxiang_BA = true
		end
	end
	local need_double_jink = attacker:hasSkill("wushuang") or niaoxiang_BA
	if need_double_jink and getKnownCard(player, attacker, "Jink", true, "he") < 2
		and getCardsNum("Jink", player, global_room:getCurrent()) < 1.5 then
		defense = 0
	end

	local jink = sgs.Sanguosha:cloneCard("jink")
	if player:isCardLimited(jink, sgs.Card_MethodUse) then defense = 0 end

	if player:hasFlag("QianxiTarget") then
		local red = player:getMark("@qianxi_red") > 0
		local black = player:getMark("@qianxi_black") > 0
		if red then
			if player:hasSkill("qingguo") then
				defense = defense - 1
			else
				defense = 0
			end
		elseif black then
			if player:hasSkill("qingguo") then
				defense = defense - 1
			end
		end
	end

	defense = defense + math.min(player:getHp() * 0.45, 10)

	if attacker then
		local m = sgs.masochism_skill:split("|")
		for _, masochism in ipairs(m) do
			if player:hasSkill(masochism) and sgs.isGoodHp(player) then
				defense = defense + 1
			end
		end
		if player:hasSkill("jieming") then defense = defense + 4 end
		if player:hasSkill("yiji") then defense = defense + 4 end
	end

	if not sgs.isGoodTarget(player) then defense = defense + 10 end

	if player:hasSkill("rende") and player:getHp() > 2 then defense = defense + 1 end
	if player:hasSkill("kuanggu") and player:getHp() > 1 then defense = defense + 0.2 end
	if player:hasSkill("zaiqi") and player:getHp() > 1 then defense = defense + 0.35 end

	if player:getHp() <= 2 then defense = defense - 0.4 end

	local playernum = global_room:alivePlayerCount()
	if (player:getSeat()-attacker:getSeat()) % playernum >= playernum-2 and playernum>3 and player:getHandcardNum()<=2 and player:getHp()<=2 then
		defense = defense - 0.4
	end

	if player:hasSkill("tianxiang") then defense = defense + player:getHandcardNum() * 0.5 end

	if player:getHandcardNum() == 0 and not player:hasSkill("kongcheng") then
		if player:getHp() <= 1 then defense = defense - 2.5 end
		if player:getHp() == 2 then defense = defense - 1.5 end
		if not hasEightDiagram then defense = defense - 2 end
		if attacker:hasWeapon("GudingBlade") and player:getHandcardNum() == 0
		  and not (player:hasArmorEffect("SilverLion") and not IgnoreArmor(attacker, player)) then
			defense = defense - 2
		end
	end

	local has_fire_slash
	local cards = sgs.QList2Table(attacker:getHandcards())
	for i = 1, #cards, 1 do
		if (attacker:hasWeapon("Fan") and cards[i]:objectName() == "slash" and not cards[i]:isKindOf("ThunderSlash")) or cards[i]:isKindOf("FireSlash")  then
			has_fire_slash = true
			break
		end
	end

	if player:hasArmorEffect("Vine") and not IgnoreArmor(attacker, player) and has_fire_slash then
		defense = defense - 0.6
	end

	if not player:faceUp() then defense = defense - 0.35 end

	if player:containsTrick("indulgence") and not player:containsTrick("YanxiaoCard") then defense = defense - 0.15 end
	if player:containsTrick("supply_shortage") and not player:containsTrick("YanxiaoCard") then defense = defense - 0.15 end

	if not hasEightDiagram then
		if player:hasSkill("jijiu") then defense = defense - 3 end
		if player:hasSkill("dimeng") then defense = defense - 2.5 end
		if player:hasSkill("guzheng") and knownJink == 0 then defense = defense - 2.5 end
		if player:hasSkill("qiaobian") then defense = defense - 2.4 end
		if player:hasSkill("jieyin") then defense = defense - 2.3 end
		if player:hasSkill("lijian") then defense = defense - 2.2 end
	end
	return defense
end

sgs.ai_compare_funcs["defenseSlash"] = function(a,b)
	return sgs.getDefenseSlash(a) < sgs.getDefenseSlash(b)
end

function SmartAI:slashProhibit(card, enemy, from)
	local mode = self.room:getMode()
	card = card or sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	from = from or self.player
	local nature = card:isKindOf("FireSlash") and sgs.DamageStruct_Fire
					or card:isKindOf("ThunderSlash") and sgs.DamageStruct_Thunder
	for _, askill in sgs.qlist(enemy:getVisibleSkillList()) do
		local filter = sgs.ai_slash_prohibit[askill:objectName()]
		if filter and type(filter) == "function" and filter(self, from, enemy, card) then return true end
	end

	if self:isFriend(enemy, from) then
		if (card:isKindOf("FireSlash") or from:hasWeapon("Fan")) and enemy:hasArmorEffect("Vine")
			and not (enemy:isChained() and self:isGoodChainTarget(enemy, from, nil, nil, card)) then return true end
		if enemy:isChained() and card:isKindOf("NatureSlash") and self:slashIsEffective(card, enemy, from)
			and not self:isGoodChainTarget(enemy, from, nature, nil, card) then return true end
		if getCardsNum("Jink",enemy, from) == 0 and enemy:getHp() < 2 and self:slashIsEffective(card, enemy, from) then return true end
		if enemy:isLord() and self:isWeak(enemy) and self:slashIsEffective(card, enemy, from) then return true end
		if from:hasWeapon("GudingBlade") and enemy:isKongcheng() then return true end
	else
		if card:isKindOf("NatureSlash") and enemy:isChained() and not self:isGoodChainTarget(enemy, from, nature, nil, card) and self:slashIsEffective(card, enemy, from) then
			return true
		end
	end

	return self.room:isProhibited(from, enemy, card) or not self:slashIsEffective(card, enemy, from) -- @todo: param of slashIsEffective
end

function SmartAI:canLiuli(other, another)
	if not other:hasSkill("liuli") then return false end
	if type(another) == "table" then
		if #another == 0 then return false end
		for _, target in ipairs(another) do
			if target:getHp() < 3 and self:canLiuli(other, target) then return true end
		end
		return false
	end

	if not self:needToLoseHp(another, self.player, true) or not self:getDamagedEffects(another, self.player, true) then return false end
	local n = other:getHandcardNum()
	if n > 0 and (other:distanceTo(another) <= other:getAttackRange()) then return true
	elseif other:getWeapon() and other:getOffensiveHorse() and (other:distanceTo(another) <= other:getAttackRange()) then return true
	elseif other:getWeapon() or other:getOffensiveHorse() then return other:distanceTo(another) <= 1
	else return false end
end

function SmartAI:slashIsEffective(slash, to, from, ignore_armor)
	if not slash or not to then self.room:writeToConsole(debug.traceback()) return end
	from = from or self.player
	if to:hasSkill("kongcheng") and to:isKongcheng() then return false end

	local natures = {
		Slash = sgs.DamageStruct_Normal,
		FireSlash = sgs.DamageStruct_Fire,
		ThunderSlash = sgs.DamageStruct_Thunder,
	}


	local nature = natures[slash:getClassName()]
	self.equipsToDec = sgs.getCardNumAtCertainPlace(slash, from, sgs.Player_PlaceEquip)
	local eff = self:damageIsEffective(to, nature, from)
	self.equipsToDec = 0
	if not eff then return false end

	if to:hasArmorEffect("Vine") and self:getCardId("FireSlash") and slash:isKindOf("ThunderSlash") and self:objectiveLevel(to) >= 3 then
		 return false
	end

	if IgnoreArmor(from, to) or ignore_armor then
		return true
	end

	local armor = to:getArmor()
	if armor then
		if armor:objectName() == "RenwangShield" then
			return not slash:isBlack()
		elseif armor:objectName() == "Vine" then
			local skill_name = slash:getSkillName() or ""
			local can_convert = false
			local skill = sgs.Sanguosha:getSkill(skill_name)
			if not skill or skill:inherits("FilterSkill") then
				can_convert = true
			end
			return nature ~= sgs.DamageStruct_Normal or can_convert and from:hasWeapon("Fan")
		end
	end

	return true
end

function SmartAI:slashIsAvailable(player, slash) -- @todo: param of slashIsAvailable
	player = player or self.player
	slash = slash or self:getCard("Slash", player)
	if not slash or not slash:isKindOf("Slash") then slash = sgs.Sanguosha:cloneCard("slash") end
	assert(slash)
	return slash:isAvailable(player)
end

function SmartAI:findWeaponToUse(enemy)
	local weaponvalue = {}
	local hasweapon
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if c:isKindOf("Weapon") then
			local dummy_use = { isDummy == true, to = sgs.SPlayerList() }
			self:useEquipCard(c, dummy_use)
			if dummy_use.card then
				weaponvalue[c] = self:evaluateWeapon(c, self.player, enemy)
				hasweapon = true
			end
		end
	end
	if not hasweapon then return end
	if self.player:getWeapon() then weaponvalue[self.player:getWeapon()] = self:evaluateWeapon(self.player:getWeapon(), self.player, enemy) end
	local max_value, max_card = -1000
	for c, v in pairs(weaponvalue) do
		if v > max_value then max_card = c max_value = v end
	end
	if self.player:getWeapon() and self.player:getWeapon():getEffectiveId() == max_card:getEffectiveId() then return false end
	return max_card
end

function SmartAI:isPriorFriendOfSlash(friend, card, source)
	source = source or self.player
	local huatuo = self.room:findPlayerBySkillName("jijiu")
	if not self:hasHeavySlashDamage(source, card, friend)
		and (self:findLeijiTarget(friend, 50, source) or (friend:hasSkill("jieming") and source:hasSkill("rende") and huatuo and self:isFriend(huatuo, source))) then
		return true
	end
	if card:isKindOf("NatureSlash") and friend:isChained() and self:isGoodChainTarget(friend, source, nil, nil, card) then return true end
	return
end

function SmartAI:useCardSlash(card, use)
	if not use.isDummy and not self:slashIsAvailable(self.player, card) then return end

	local basicnum = 0
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	for _, acard in ipairs(cards) do
		if acard:getTypeId() == sgs.Card_TypeBasic and not acard:isKindOf("Peach") then basicnum = basicnum + 1 end
	end
	local no_distance = sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, self.player, card) > 50
						or self.player:hasFlag("slashNoDistanceLimit")
	self.slash_targets = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, card)
	if self.player:hasSkill("duanbing") then self.slash_targets = self.slash_targets + 1 end

	local rangefix = 0
	if card:isVirtualCard() then
		if self.player:getWeapon() and card:getSubcards():contains(self.player:getWeapon():getEffectiveId()) then
			if self.player:getWeapon():getClassName() ~= "Weapon" then
				rangefix = sgs.weapon_range[self.player:getWeapon():getClassName()] - self.player:getAttackRange(false)
			end
		end
		if self.player:getOffensiveHorse() and card:getSubcards():contains(self.player:getOffensiveHorse():getEffectiveId()) then
			rangefix = rangefix + 1
		end
	end

	local function canAppendTarget(target)
		if use.to:contains(target) then return false end
		local targets = sgs.PlayerList()
		for _, to in sgs.qlist(use.to) do
			targets:append(to)
		end
		return card:targetFilter(targets, target, self.player)
	end

	if not use.isDummy and self.player:hasSkill("qingnang") and self:isWeak() and self:getOverflow() == 0 then return end
	for _, friend in ipairs(self.friends_noself) do
		local slash_prohibit = false
		slash_prohibit = self:slashProhibit(card, friend)
		if self:isPriorFriendOfSlash(friend, card) then
			if not slash_prohibit then
				if (not use.current_targets or not table.contains(use.current_targets, friend:objectName()))
					and (self.player:canSlash(friend, card, not no_distance, rangefix)
						or (use.isDummy and (self.player:distanceTo(friend, rangefix) <= self.predictedRange)))
					and self:slashIsEffective(card, friend) then
					use.card = card
					if use.to and canAppendTarget(friend) then
						use.to:append(friend)
					end
					if not use.to or self.slash_targets <= use.to:length() then return end
				end
			end
		end
	end


	local targets = {}
	local forbidden = {}
	self:sort(self.enemies, "defenseSlash")
	for _, enemy in ipairs(self.enemies) do
		if not self:slashProhibit(card, enemy) and sgs.isGoodTarget(enemy, self.enemies, self, true) then
			if not self:getDamagedEffects(enemy, self.player, true) then table.insert(targets, enemy) else table.insert(forbidden, enemy) end
		end
	end
	if #targets == 0 and #forbidden > 0 then targets = forbidden end

	for _, target in ipairs(targets) do
		local canliuli = false
		for _, friend in ipairs(self.friends_noself) do
			if self:canLiuli(target, friend) and self:slashIsEffective(card, friend) and #targets > 1 and friend:getHp() < 3 then canliuli = true end
		end
		if (not use.current_targets or not table.contains(use.current_targets, target:objectName()))
			and (self.player:canSlash(target, card, not no_distance, rangefix)
				or (use.isDummy and self.predictedRange and self.player:distanceTo(target, rangefix) <= self.predictedRange))
			and self:objectiveLevel(target) > 3
			and self:slashIsEffective(card, target, self.player, shoulduse_wuqian)
			and not (target:hasSkill("xiangle") and basicnum < 2) and not canliuli
			and not (not self:isWeak(target) and #self.enemies > 1 and #self.friends > 1 and self.player:hasSkill("keji")
				and self:getOverflow() > 0 and not self:hasCrossbowEffect()) then

			if target:getHp() > 1 and target:hasSkill("jianxiong") and self.player:hasWeapon("Spear") and card:getSkillName() == "Spear" then
				local ids, isGood = card:getSubcards(), true
				for _, id in sgs.qlist(ids) do
					local c = sgs.Sanguosha:getCard(id)
					if isCard("Peach", c, target) or isCard("Analeptic", c, target) then isGood = false break end
				end
				if not isGood then continue end
			end

			-- fill the card use struct
			local usecard = card
			if not use.to or use.to:isEmpty() then
				if self.player:hasWeapon("Spear") and card:getSkillName() == "Spear" then
				elseif self.player:hasWeapon("Crossbow") and self:getCardsNum("Slash") > 1 then
				elseif not use.isDummy then
					local card = self:findWeaponToUse(target)
					if card then
						use.card = card
						return
					end
				end

				if target:isChained() and self:isGoodChainTarget(target, nil, nil, nil, card) and not use.card then
					if self:hasCrossbowEffect() and card:isKindOf("NatureSlash") then
						local slashes = self:getCards("Slash")
						for _, slash in ipairs(slashes) do
							if not slash:isKindOf("NatureSlash") and self:slashIsEffective(slash, target)
								and not self:slashProhibit(slash, target) then
								usecard = slash
								break
							end
						end
					elseif not card:isKindOf("NatureSlash") then
						local slash = self:getCard("NatureSlash")
						if slash and self:slashIsEffective(slash, target) and not self:slashProhibit(slash, target) then usecard = slash end
					end
				end
				local godsalvation = self:getCard("GodSalvation")
				if not use.isDummy and godsalvation and godsalvation:getId() ~= card:getId() and self:willUseGodSalvation(godsalvation) and
					(not target:isWounded() or not self:hasTrickEffective(godsalvation, target, self.player)) then
					use.card = godsalvation
					return
				end
			end
			use.card = use.card or usecard
			if use.to and not use.to:contains(target) and canAppendTarget(target) then
				use.to:append(target)
			end
			if not use.isDummy then
				local analeptic = self:searchForAnaleptic(use, target, use.card)
				if analeptic and self:shouldUseAnaleptic(target, use.card) and analeptic:getEffectiveId() ~= card:getEffectiveId() then
					use.card = analeptic
					if use.to then use.to = sgs.SPlayerList() end
					return
				end
			end
			if not use.to or self.slash_targets <= use.to:length() then return end
		end
	end

	for _, friend in ipairs(self.friends_noself) do
		local slash_prohibit = self:slashProhibit(card, friend)
		if (not use.current_targets or not table.contains(use.current_targets, friend:objectName()))
			and not self:hasHeavySlashDamage(self.player, card, friend) and (not use.to or not use.to:contains(friend))
			and (self:getDamagedEffects(friend, self.player) and not (friend:isLord() and #self.enemies < 1) or self:needToLoseHp(friend, self.player, true, true)) then

			if not slash_prohibit then
				if ((self.player:canSlash(friend, card, not no_distance, rangefix))
					or (use.isDummy and self.predictedRange and self.player:distanceTo(friend, rangefix) <= self.predictedRange))
					and self:slashIsEffective(card, friend) then
					use.card = card
					if use.to and canAppendTarget(friend) then
						use.to:append(friend)
					end
					if not use.to or self.slash_targets <= use.to:length() then return end
				end
			end
		end
	end
end

sgs.ai_skill_use.slash = function(self, prompt)
	local parsedPrompt = prompt:split(":")
	local callback = sgs.ai_skill_cardask[parsedPrompt[1]] -- for askForUseSlashTo
	if self.player:hasFlag("slashTargetFixToOne") and type(callback) == "function" then
		local slash
		local target
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if player:hasFlag("SlashAssignee") then target = player break end
		end
		local target2 = nil
		if #parsedPrompt >= 3 then target2 = findPlayerByObjectName(parsedPrompt[3]) end
		if not target then return "." end
		local ret = callback(self, nil, nil, target, target2, prompt)
		if ret == nil or ret == "." then return "." end
		slash = sgs.Card_Parse(ret)
		assert(slash)
		local no_distance = sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, self.player, slash) > 50 or self.player:hasFlag("slashNoDistanceLimit")
		local targets = {}
		local use = { to = sgs.SPlayerList() }
		if self.player:canSlash(target, slash, not no_distance) then use.to:append(target) else return "." end

		self:useCardSlash(slash, use)
		for _, p in sgs.qlist(use.to) do table.insert(targets, p:objectName()) end
		if table.contains(targets, target:objectName()) then return ret .. "->" .. table.concat(targets, "+") end
		return "."
	end
	local useslash, target
	local slashes = self:getCards("Slash")
	self:sort(self.enemies, "defenseSlash")
	for _, slash in ipairs(slashes) do
		local no_distance = sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, self.player, slash) > 50 or self.player:hasFlag("slashNoDistanceLimit")
		for _, friend in ipairs(self.friends_noself) do
			local slash_prohibit = false
			slash_prohibit = self:slashProhibit(card, friend)
			if not self:hasHeavySlashDamage(self.player, card, friend)
				and self.player:canSlash(friend, slash, not no_distance) and not self:slashProhibit(slash, friend)
				and self:slashIsEffective(slash, friend)
				and (self:findLeijiTarget(friend, 50, self.player) or (friend:hasSkill("jieming") and self.player:hasSkill("rende") and (huatuo and self:isFriend(huatuo))))
				and not (self.player:hasFlag("slashTargetFix") and not friend:hasFlag("SlashAssignee")) then

				useslash = slash
				target = friend
				break
			end
		end
	end
	if not useslash then
		for _, slash in ipairs(slashes) do
			local no_distance = sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, self.player, slash) > 50 or self.player:hasFlag("slashNoDistanceLimit")
			for _, enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy, slash, not no_distance) and not self:slashProhibit(slash, enemy)
					and self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self)
					and not (self.player:hasFlag("slashTargetFix") and not enemy:hasFlag("SlashAssignee")) then

					useslash = slash
					target = enemy
					break
				end
			end
		end
	end
	if useslash and target then
		local targets = {}
		local use = { to = sgs.SPlayerList() }
		use.to:append(target)

		self:useCardSlash(useslash, use)
		for _, p in sgs.qlist(use.to) do table.insert(targets, p:objectName()) end
		if table.contains(targets, target:objectName()) then return useslash:toString() .. "->" .. table.concat(targets, "+") end
	end
	return "."
end

sgs.ai_skill_playerchosen.slash_extra_targets = function(self, targets)
	local slash = sgs.Sanguosha:cloneCard("slash")
	targets = sgs.QList2Table(targets)
	self:sort(targets, "defenseSlash")
	for _, target in ipairs(targets) do
		if self:isEnemy(target) and not self:slashProhibit(slash, target) and sgs.isGoodTarget(target, targetlist, self) and self:slashIsEffective(slash, target) then
			return target
		end
	end
	return nil
end

sgs.ai_skill_playerchosen.zero_card_as_slash = function(self, targets)
	local slash = sgs.Sanguosha:cloneCard("slash")
	local targetlist = sgs.QList2Table(targets)
	local arrBestHp, canAvoidSlash, forbidden = {}, {}, {}
	self:sort(targetlist, "defenseSlash")

	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) and not self:slashProhibit(slash ,target) and sgs.isGoodTarget(target, targetlist, self) then
			if self:slashIsEffective(slash, target) then
				if self:getDamagedEffects(target, self.player, true) or self:needLeiji(target, self.player) then
					table.insert(forbidden, target)
				elseif self:needToLoseHp(target, self.player, true, true) then
					table.insert(arrBestHp, target)
				else
					return target
				end
			else
				table.insert(canAvoidSlash, target)
			end
		end
	end
	for i=#targetlist, 1, -1 do
		local target = targetlist[i]
		if not self:slashProhibit(slash, target) then
			if self:slashIsEffective(slash, target) then
				if self:isFriend(target) and (self:needToLoseHp(target, self.player, true, true)
					or self:getDamagedEffects(target, self.player, true) or self:needLeiji(target, self.player)) then
						return target
				end
			else
				table.insert(canAvoidSlash, target)
			end
		end
	end

	if #canAvoidSlash > 0 then return canAvoidSlash[1] end
	if #arrBestHp > 0 then return arrBestHp[1] end

	self:sort(targetlist, "defenseSlash")
	targetlist = sgs.reverse(targetlist)
	for _, target in ipairs(targetlist) do
		if target:objectName() ~= self.player:objectName() and not self:isFriend(target) and not table.contains(forbidden, target) then
			return target
		end
	end

	return targetlist[1]
end

sgs.ai_card_intention.Slash = function(self, card, from, tos)
	for _, to in ipairs(tos) do
		local value = 80
		speakTrigger(card, from, to)
		sgs.updateIntention(from, to, value)
	end
end

sgs.ai_skill_cardask["slash-jink"] = function(self, data, pattern, target)
	local isdummy = type(data) == "number"
	local function getJink()
		return self:getCardId("Jink") or not isdummy and "."
	end

	local slash
	if type(data) == "userdata" then
		local effect = data:toSlashEffect()
		slash = effect.slash
	else
		slash = sgs.Sanguosha:cloneCard("slash")
	end
	local cards = sgs.QList2Table(self.player:getHandcards())
	if sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) then return "." end
	if not target then return getJink() end
	if not self:hasHeavySlashDamage(target, slash, self.player) and self:getDamagedEffects(self.player, target, slash) then return "." end
	if slash:isKindOf("NatureSlash") and self.player:isChained() and self:isGoodChainTarget(self.player, target, nil, nil, slash) then return "." end
	if self:isFriend(target) then
		if self:findLeijiTarget(self.player, 50, target) then return getJink() end
		if target:hasSkill("jieyin") and not self.player:isWounded() and self.player:isMale() and not self.player:hasSkill("leiji") then return "." end
		if target:hasSkill("rende") and self.player:hasSkill("jieming") then return "." end
	else
		if self:hasHeavySlashDamage(target, slash) then return getJink() end

		if self.player:getHandcardNum() == 1 and self:needKongcheng() then return getJink() end
		if not self:hasLoseHandcardEffective() and not self.player:isKongcheng() then return getJink() end
		if target:hasSkill("mengjin") then
			if self:doNotDiscard(self.player, "he", true) then return getJink() end
			if self.player:getCards("he"):length() == 1 and not self.player:getArmor() then return getJink() end
			if self.player:hasSkills("jijiu|qingnang") and self.player:getCards("he"):length() > 1 then return "." end
			if (self:getCardsNum("Peach") > 0 or (self:getCardsNum("Analeptic") > 0 and self:isWeak()))
				and not self.player:hasSkill("tuntian") and not self:willSkipPlayPhase() then
				return "."
			end
		end
		if target:hasWeapon("Axe") then
			if target:hasSkills(sgs.lose_equip_skill) and target:getEquips():length() > 1 and target:getCards("he"):length() > 2 then return not isdummy and "." end
			if target:getHandcardNum() - target:getHp() > 2 and not self:isWeak() and not self:getOverflow() then return not isdummy and "." end
		elseif target:hasWeapon("Blade") then
			if slash:isKindOf("NatureSlash") and self.player:hasArmorEffect("Vine")
				or self.player:hasArmorEffect("RenwangShield")
				or self:hasEightDiagramEffect()
				or self:hasHeavySlashDamage(target, slash)
				or (self.player:getHp() == 1 and #self.friends_noself == 0) then
			elseif (self:getCardsNum("Jink") <= getCardsNum("Slash", target, self.player) or self.player:hasSkill("qingnang")) and self.player:getHp() > 1
					or self.player:hasSkill("jijiu") and getKnownCard(self.player, self.player, "red") > 0
				then
				return not isdummy and "."
			end
		end
	end
	return getJink()
end

sgs.dynamic_value.damage_card.Slash = true

sgs.ai_use_value.Slash = 4.5
sgs.ai_keep_value.Slash = 3.6
sgs.ai_use_priority.Slash = 2.6

function SmartAI:canHit(to, from, conservative)
	from = from or self.room:getCurrent()
	to = to or self.player
	local jink = sgs.Sanguosha:cloneCard("jink")
	if to:isCardLimited(jink, sgs.Card_MethodUse) then return true end
	if self:canLiegong(to, from) then return true end
	if not self:isFriend(to, from) then
		if from:hasWeapon("Axe") and from:getCards("he"):length() > 2 then return true end
		if from:hasWeapon("Blade") and getCardsNum("Jink", to, from) <= getCardsNum("Slash", from, from) then return true end
		if from:hasSkill("mengjin") and not self:hasHeavySlashDamage(from, nil, to) and not self:needLeiji(to, from) then
			if self:doNotDiscard(to, "he", true) then
			elseif to:getCards("he"):length() == 1 and not to:getArmor() then
			elseif self:willSkipPlayPhase() then
			elseif (getCardsNum("Peach", to, from) > 0 or getCardsNum("Analeptic", to, from) > 0) then return true
			elseif not self:isWeak(to) and to:getArmor() and not self:needToThrowArmor() then return true
			elseif not self:isWeak(to) and to:getDefensiveHorse() then return true
			end
		end
	end

	local hasHeart, hasRed, hasBlack
	for _, card in ipairs(self:getCards("Jink"), to) do
		if card:getSuit() == sgs.Card_Heart then hasHeart = true end
		if card:isRed() then hasRed = true end
		if card:isBlack() then hasBlack = true end
	end
	if to:getMark("@qianxi_red") > 0 and not hasBlack then return true end
	if to:getMark("@qianxi_black") > 0 and not hasRed then return true end
	if not conservative and self:hasHeavySlashDamage(from, nil, to) then conservative = true end
	if not conservative and self:hasEightDiagramEffect(to) and not IgnoreArmor(from, to) then return false end
	local need_double_jink = from and from:hasSkill("wushuang")
	if to:objectName() == self.player:objectName() then
		if getCardsNum("Jink", to, from) == 0 then return true end
		if need_double_jink and getCardsNum("Jink", to, from) < 2 then return true end
	end
	if getCardsNum("Jink", to, from) == 0 then return true end
	if need_double_jink and getCardsNum("Jink", to, from) < 2 then return true end
	return false
end

function SmartAI:useCardPeach(card, use)
	local mustusepeach = false
	if not self.player:isWounded() then return end

	local peaches = 0
	local cards = self.player:getHandcards()

	cards = sgs.QList2Table(cards)
	for _,card in ipairs(cards) do
		if isCard("Peach", card, self.player) then peaches = peaches + 1 end
	end

	if self.player:isLord() and (self.player:hasSkill("hunzi") and self.player:getMark("hunzi") == 0)
		and self.player:getHp() < 4 and self.player:getHp() > peaches then return end

	if self.player:hasSkill("rende") and self:findFriendsByType(sgs.Friend_Draw) then return end

	if self.player:hasArmorEffect("SilverLion") then
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if card:isKindOf("Armor") and self:evaluateArmor(card) > 0 then
				use.card = card
				return
			end
		end
	end

	local SilverLion, OtherArmor
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if card:isKindOf("SilverLion") then
			SilverLion = card
		elseif card:isKindOf("Armor") and not card:isKindOf("SilverLion") and self:evaluateArmor(card) > 0 then
			OtherArmor = true
		end
	end
	if SilverLion and OtherArmor then
		use.card = SilverLion
		return
	end

	for _, enemy in ipairs(self.enemies) do
		if self.player:getHandcardNum() < 3 and
				(enemy:hasSkills(sgs.drawpeach_skill) or getCardsNum("Dismantlement", enemy, self.player) >= 1
					or enemy:hasSkill("jixi") and enemy:getPile("field"):length() >0 and enemy:distanceTo(self.player) == 1
					or enemy:hasSkill("qixi") and getKnownCard(enemy, self.player, "black", nil, "he") >= 1
					or getCardsNum("Snatch", enemy, self.player) >= 1 and enemy:distanceTo(self.player) == 1
					or (enemy:hasSkill("tiaoxin") and self.player:inMyAttackRange(enemy) and self:getCardsNum("Slash") < 1 or not self.player:canSlash(enemy)))
				then
			mustusepeach = true
			break
		end
	end


	if self.player:getHp() == 1 then
		mustusepeach = true
	end

	if mustusepeach or (self.player:hasSkill("buqu") and self.player:getHp() < 1 and self.player:getMaxCards() == 0) or peaches > self.player:getHp() then
		use.card = card
		return
	end

	if self:getOverflow() <= 0 and #self.friends_noself > 0 then
		return
	end

	if self.player:hasSkill("kuanggu") and self.player:getLostHp()==1 and self.player:getOffensiveHorse() then
		return
	end

	if self:needToLoseHp(self.player, nil, nil, nil, true) then return end

	if lord and self:isFriend(lord) and lord:getHp() <= 2 and self:isWeak(lord) then
		if self.player:isLord() then use.card = card end
		if self:getCardsNum("Peach") > 1 and self:getCardsNum("Peach") + self:getCardsNum("Jink") > self.player:getMaxCards() then use.card = card end
		return
	end

	self:sort(self.friends, "hp")
	if self.friends[1]:objectName()==self.player:objectName() or self.player:getHp()<2 then
		use.card = card
		return
	end

	if #self.friends > 1 and ((not hasBuquEffect(self.friends[2]) and self.friends[2]:getHp() < 3 and self:getOverflow() < 2)
								or (not hasBuquEffect(self.friends[1]) and self.friends[1]:getHp() < 2 and peaches <= 1 and self:getOverflow() < 3)) then
		return
	end

	if self.player:hasSkill("jieyin") and self:getOverflow() > 0 then
		self:sort(self.friends, "hp")
		for _, friend in ipairs(self.friends) do
			if friend:isWounded() and friend:isMale() then return end
		end
	end

	use.card = card
end

sgs.ai_card_intention.Peach = function(self, card, from, tos)
	for _, to in ipairs(tos) do
		sgs.updateIntention(from, to, -120)
	end
end

sgs.ai_use_value.Peach = 6
sgs.ai_keep_value.Peach = 7
sgs.ai_use_priority.Peach = 0.9

sgs.ai_use_value.Jink = 8.9
sgs.ai_keep_value.Jink = 5.2

sgs.dynamic_value.benefit.Peach = true

sgs.ai_keep_value.Weapon = 2.08
sgs.ai_keep_value.Armor = 2.06
sgs.ai_keep_value.Horse = 2.04

sgs.weapon_range.Weapon = 1
sgs.weapon_range.Crossbow = 1
sgs.weapon_range.DoubleSword = 2
sgs.weapon_range.QinggangSword = 2
sgs.weapon_range.IceSword = 2
sgs.weapon_range.GudingBlade = 2
sgs.weapon_range.Axe = 3
sgs.weapon_range.Blade = 3
sgs.weapon_range.Spear = 3
sgs.weapon_range.Halberd = 4
sgs.weapon_range.KylinBow = 5
sgs.weapon_range.SixSwords = 2
sgs.weapon_range.DragonPhoenix = 2
sgs.weapon_range.Triblade = 3

sgs.ai_skill_invoke.DoubleSword = function(self, data)
	return not self:needKongcheng(self.player, true)
end

function sgs.ai_slash_weaponfilter.DoubleSword(self, to, player)
	return player:distanceTo(to) <= math.max(sgs.weapon_range.DoubleSword, player:getAttackRange()) and player:getGender() ~= to:getGender()
end

function sgs.ai_weapon_value.DoubleSword(self, enemy, player)
	if enemy and enemy:isMale() ~= player:isMale() then return 4 end
end

function SmartAI:getExpectedJinkNum(use)
	local jink_list = use.from:getTag("Jink_" .. use.card:toString()):toStringList()
	local index, jink_num = 1, 1
	for _, p in sgs.qlist(use.to) do
		if p:objectName() == self.player:objectName() then
			local n = tonumber(jink_list[index])
			if n == 0 then return 0
			elseif n > jink_num then jink_num = n end
		end
		index = index + 1
	end
	return jink_num
end

sgs.ai_skill_cardask["double-sword-card"] = function(self, data, pattern, target)
	if self.player:isKongcheng() then return "." end
	local use = data:toCardUse()
	local jink_num = self:getExpectedJinkNum(use)
	if jink_num > 1 and self:getCardsNum("Jink") == jink_num then return "." end

	if self:needKongcheng(self.player, true) and self.player:getHandcardNum() <= 2 then
		if self.player:getHandcardNum() == 1 then
			local card = self.player:getHandcards():first()
			return (jink_num > 0 and isCard("Jink", card, self.player)) and "." or ("$" .. card:getEffectiveId())
		end
		if self.player:getHandcardNum() == 2 then
			local first = self.player:getHandcards():first()
			local last = self.player:getHandcards():last()
			local jink = isCard("Jink", first, self.player) and first or (isCard("Jink", last, self.player) and last)
			if jink then
				return first:getEffectiveId() == jink:getEffectiveId() and ("$"..last:getEffectiveId()) or ("$"..first:getEffectiveId())
			end
		end
	end
	if target and self:isFriend(target) then return "." end
	if target and self:needKongcheng(target, true) then return "." end
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if (card:isKindOf("Slash") and self:getCardsNum("Slash") > 1)
			or (card:isKindOf("Jink") and self:getCardsNum("Jink") > 2)
			or card:isKindOf("Disaster")
			or (card:isKindOf("EquipCard") and not self.player:hasSkills(sgs.lose_equip_skill))
			or (not self.player:hasSkill("jizhi") and (card:isKindOf("Collateral") or card:isKindOf("GodSalvation")
															or card:isKindOf("FireAttack") or card:isKindOf("IronChain") or card:isKindOf("AmazingGrace"))) then
			return "$" .. card:getEffectiveId()
		end
	end
	return "."
end

function sgs.ai_weapon_value.QinggangSword(self, enemy)
	if enemy and enemy:getArmor() and enemy:hasArmorEffect(enemy:getArmor():objectName()) then return 3 end
end

function sgs.ai_slash_weaponfilter.QinggangSword(self, enemy, player)
	if player:distanceTo(enemy) > math.max(sgs.weapon_range.QinggangSword, player:getAttackRange()) then return end
	if enemy:getArmor() and enemy:hasArmorEffect(enemy:getArmor():objectName())
		and (sgs.card_lack[enemy:objectName()] == 1 or getCardsNum("Jink", enemy, self.player) < 1) then
		return true
	end
end

sgs.ai_skill_invoke.IceSword = function(self, data)
	local damage = data:toDamage()
	local target = damage.to
	if self:isFriend(target) then
		if self:getDamagedEffects(target, self.players, true) or self:needToLoseHp(target, self.player, true) then return false
		elseif target:isChained() and self:isGoodChainTarget(target, self.player, nil, nil, damage.card) then return false
		elseif self:isWeak(target) or damage.damage > 1 then return true
		elseif target:getLostHp() < 1 then return false end
		return true
	else
		if self:isWeak(target) then return false end
		if damage.damage > 1 or self:hasHeavySlashDamage(self.player, damage.card, target) then return false end
		if target:hasSkill("lirang") and #self:getFriendsNoself(target) > 0 then return false end
		if target:getArmor() and self:evaluateArmor(target:getArmor(), target) > 3 and not (target:hasArmorEffect("SilverLion") and target:isWounded()) then return true end
		local num = target:getHandcardNum()
		if self.player:hasSkill("tieji") or self:canLiegong(target, self.player) then return false end
		if target:hasSkill("tuntian") and target:getPhase() == sgs.Player_NotActive then return false end
		if target:hasSkills(sgs.need_kongcheng) then return false end
		if target:getCards("he"):length()<4 and target:getCards("he"):length()>1 then return true end
		return false
	end
end

function sgs.ai_slash_weaponfilter.GudingBlade(self, to)
	return to:isKongcheng() and not to:hasArmorEffect("SilverLion")
end

function sgs.ai_weapon_value.GudingBlade(self, enemy)
	if not enemy then return end
	local value = 2
	if enemy:getHandcardNum() < 1 and not enemy:hasArmorEffect("SilverLion") then value = 4 end
	return value
end

function SmartAI:needToThrowAll(player)
	player = player or self.player
	if player:getPhase() == sgs.Player_NotActive or player:getPhase() == sgs.Player_Finish then return false end
	local erzhang = self.room:findPlayerBySkillName("guzheng")
	if erzhang and not zhanglu and self:isFriend(erzhang, player) then return false end

	self.yongsi_discard = nil
	local index = 0

	local kingdom_num = 0
	local kingdoms = {}
	for _, ap in sgs.qlist(self.room:getAlivePlayers()) do
		if not kingdoms[ap:getKingdom()] then
			kingdoms[ap:getKingdom()] = true
			kingdom_num = kingdom_num + 1
		end
	end

	local cards = self.player:getCards("he")
	local Discards = {}
	for _, card in sgs.qlist(cards) do
		local shouldDiscard = true
		if card:isKindOf("Axe") then shouldDiscard = false end
		if isCard("Peach", card, player) or isCard("Slash", card, player) then
			local dummy_use = { isDummy = true }
			self:useBasicCard(card, dummy_use)
			if dummy_use.card then shouldDiscard = false end
		end
		if card:getTypeId() == sgs.Card_TypeTrick then
			local dummy_use = { isDummy = true }
			self:useTrickCard(card, dummy_use)
			if dummy_use.card then shouldDiscard = false end
		end
		if shouldDiscard then
			if #Discards < 2 then table.insert(Discards, card:getId()) end
			index = index + 1
		end
	end

	if #Discards == 2 and index < kingdom_num then
		self.yongsi_discard = Discards
		return true
	end
	return false
end

sgs.ai_skill_cardask["@Axe"] = function(self, data, pattern, target)
	if target and self:isFriend(target) then return "." end
	local effect = data:toSlashEffect()
	local allcards = self.player:getCards("he")
	allcards = sgs.QList2Table(allcards)
	if self:hasHeavySlashDamage(self.player, effect.slash, target)
	  or (#allcards - 3 >= self.player:getHp())
	  or (self.player:hasSkill("kuanggu") and self.player:isWounded() and self.player:distanceTo(effect.to) == 1)
	  or (effect.to:getHp() == 1 and not effect.to:hasSkill("buqu"))
	  or (self:needKongcheng() and self.player:getHandcardNum() > 0)
	  or (self.player:hasSkills(sgs.lose_equip_skill) and self.player:getEquips():length() > 1 and self.player:getHandcardNum() < 2)
	  or self:needToThrowAll() then
		local discard = self.yongsi_discard
		if discard then return "$"..table.concat(discard, "+") end

		local hcards = {}
		for _, c in sgs.qlist(self.player:getHandcards()) do
			if not (isCard("Slash", c, self.player) and self:hasCrossbowEffect()) then table.insert(hcards, c) end
		end
		self:sortByKeepValue(hcards)
		local cards = {}
		local hand, armor, def, off = 0, 0, 0, 0
		if self:needToThrowArmor() then
			table.insert(cards, self.player:getArmor():getEffectiveId())
			armor = 1
		end
		if (self.player:hasSkills(sgs.need_kongcheng) or not self:hasLoseHandcardEffective()) and self.player:getHandcardNum() > 0 then
			hand = 1
			for _, card in ipairs(hcards) do
				table.insert(cards, card:getEffectiveId())
				if #cards == 2 then break end
			end
		end
		if #cards < 2 and self.player:hasSkills(sgs.lose_equip_skill) then
			if #cards < 2 and self.player:getOffensiveHorse() then
				off = 1
				table.insert(cards, self.player:getOffensiveHorse():getEffectiveId())
			end
			if #cards < 2 and self.player:getArmor() then
				armor = 1
				table.insert(cards, self.player:getArmor():getEffectiveId())
			end
			if #cards < 2 and self.player:getDefensiveHorse() then
				def = 1
				table.insert(cards, self.player:getDefensiveHorse():getEffectiveId())
			end
		end

		if #cards < 2 and hand < 1 and self.player:getHandcardNum() > 2 then
			hand = 1
			for _, card in ipairs(hcards) do
				table.insert(cards, card:getEffectiveId())
				if #cards == 2 then break end
			end
		end

		if #cards < 2 and off < 1 and self.player:getOffensiveHorse() then
			off = 1
			table.insert(cards, self.player:getOffensiveHorse():getEffectiveId())
		end
		if #cards < 2 and hand < 1 and self.player:getHandcardNum() > 0 then
			hand = 1
			for _, card in ipairs(hcards) do
				table.insert(cards, card:getEffectiveId())
				if #cards == 2 then break end
			end
		end
		if #cards < 2 and armor < 1 and self.player:getArmor() then
			armor = 1
			table.insert(cards, self.player:getArmor():getEffectiveId())
		end
		if #cards < 2 and def < 1 and self.player:getDefensiveHorse() then
			def = 1
			table.insert(cards, self.player:getDefensiveHorse():getEffectiveId())
		end

		if #cards == 2 then
			local num = 0
			for _, id in ipairs(cards) do
				if self.player:hasEquip(sgs.Sanguosha:getCard(id)) then num = num + 1 end
			end
			self.equipsToDec = num
			local eff = self:damageIsEffective(effect.to, effect.nature, self.player)
			self.equipsToDec = 0
			if not eff then return "." end
			return "$" .. table.concat(cards, "+")
		end
	end
end


function sgs.ai_slash_weaponfilter.Axe(self, to, player)
	return player:distanceTo(to) <= math.max(sgs.weapon_range.Axe, player:getAttackRange()) and self:getOverflow(player) > 0
end

function sgs.ai_weapon_value.Axe(self, enemy, player)
	if player:hasSkill("luoyi") then return 6 end
	if enemy and self:getOverflow() > 0 then return 2 end
	if enemy and enemy:getHp() < 3 then return 3 - enemy:getHp() end
end

sgs.ai_skill_cardask["blade-slash"] = function(self, data, pattern, target)
	if target and self:isFriend(target) and not self:findLeijiTarget(target, 50, self.player) then
		return "."
	end
	for _, slash in ipairs(self:getCards("Slash")) do
		if self:slashIsEffective(slash, target) and (self:isWeak(target) or self:getOverflow() > 0) then
			return slash:toString()
		end
	end
	return "."
end

function sgs.ai_weapon_value.Blade(self, enemy)
	if not enemy then return math.min(self:getCardsNum("Slash"), 3) end
end

function cardsView_spear(self, player, skill_name)
	local cards = player:getCards("he")
	cards = sgs.QList2Table(cards)
	if skill_name ~= "fuhun" or player:hasSkill("wusheng") then
		for _, acard in ipairs(cards) do
			if isCard("Slash", acard, player) then return end
		end
	end
	local cards = player:getCards("h")
	cards = sgs.QList2Table(cards)
	local newcards = {}
	for _, card in ipairs(cards) do
		if not isCard("Slash", card, player) and not isCard("Peach", card, player) and not (isCard("ExNihilo", card, player) and player:getPhase() == sgs.Player_Play) then table.insert(newcards, card) end
	end
	if #newcards < 2 then return end
	self:sortByKeepValue(newcards)

	local card_id1 = newcards[1]:getEffectiveId()
	local card_id2 = newcards[2]:getEffectiveId()

	local card_str = ("slash:%s[%s:%s]=%d+%d&%s"):format(skill_name, "to_be_decided", 0, card_id1, card_id2, skill_name)
	return card_str
end

function sgs.ai_cardsview.Spear(self, class_name, player)
	if class_name == "Slash" then
		return cardsView_spear(self, player, "Spear")
	end
end

function turnUse_spear(self, inclusive, skill_name)
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	if skill_name ~= "fuhun" or self.player:hasSkill("wusheng") then
		for _, acard in ipairs(cards) do
			if isCard("Slash", acard, self.player) then return end
		end
	end

	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards)
	local newcards = {}
	for _, card in ipairs(cards) do
		if not isCard("Slash", card, self.player) and not isCard("Peach", card, self.player) and not (isCard("ExNihilo", card, self.player) and self.player:getPhase() == sgs.Player_Play) then table.insert(newcards, card) end
	end
	if #cards <= self.player:getHp() - 1 and self.player:getHp() <= 4 and not self:hasHeavySlashDamage(self.player)
		and not self.player:hasSkills("kongcheng|lianying|paoxiao") then return end
	if #newcards < 2 then return end

	local card_id1 = newcards[1]:getEffectiveId()
	local card_id2 = newcards[2]:getEffectiveId()

	if newcards[1]:isBlack() and newcards[2]:isBlack() then
		local black_slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuitBlack)
		local nosuit_slash = sgs.Sanguosha:cloneCard("slash")

		self:sort(self.enemies, "defenseSlash")
		for _, enemy in ipairs(self.enemies) do
			if self.player:canSlash(enemy) and not self:slashProhibit(nosuit_slash, enemy) and self:slashIsEffective(nosuit_slash, enemy)
				and self:canAttack(enemy) and self:slashProhibit(black_slash, enemy) and self:isWeak(enemy) then
				local redcards, blackcards = {}, {}
				for _, acard in ipairs(newcards) do
					if acard:isBlack() then table.insert(blackcards, acard) else table.insert(redcards, acard) end
				end
				if #redcards == 0 then break end

				local redcard, othercard

				self:sortByUseValue(blackcards, true)
				self:sortByUseValue(redcards, true)
				redcard = redcards[1]

				othercard = #blackcards > 0 and blackcards[1] or redcards[2]
				if redcard and othercard then
					card_id1 = redcard:getEffectiveId()
					card_id2 = othercard:getEffectiveId()
					break
				end
			end
		end
	end

	local card_str = ("slash:%s[%s:%s]=%d+%d&%s"):format(skill_name, "to_be_decided", 0, card_id1, card_id2, skill_name)
	local slash = sgs.Card_Parse(card_str)
	assert(slash)
	return slash
end

local Spear_skill = {}
Spear_skill.name = "Spear"
table.insert(sgs.ai_skills, Spear_skill)
Spear_skill.getTurnUseCard = function(self, inclusive)
	return turnUse_spear(self, inclusive, "Spear")
end

function sgs.ai_weapon_value.Spear(self, enemy, player)
	if enemy and getCardsNum("Slash", player, self.player) == 0 then
		if self:getOverflow(player) > 0 then return 2
		elseif player:getHandcardNum() > 2 then return 1
		end
	end
	return 0
end

function sgs.ai_slash_weaponfilter.Fan(self, to, player)
	return player:distanceTo(to) <= math.max(sgs.weapon_range.Fan, player:getAttackRange())
		and to:hasArmorEffect("Vine")
end

sgs.ai_skill_invoke.KylinBow = function(self, data)
	local damage = data:toDamage()
	if damage.from:hasSkill("kuangfu") and damage.to:getCards("e"):length() == 1 then return false end
	if damage.to:hasSkills(sgs.lose_equip_skill) then
		return self:isFriend(damage.to)
	end
	return self:isEnemy(damage.to)
end

function sgs.ai_slash_weaponfilter.KylinBow(self, to, player)
	return player:distanceTo(to) <= math.max(sgs.weapon_range.KylinBow, player:getAttackRange())
		and (to:getDefensiveHorse() or to:getOffensiveHorse())
end

function sgs.ai_weapon_value.KylinBow(self, enemy)
	if enemy and (enemy:getOffensiveHorse() or enemy:getDefensiveHorse()) then return 1 end
end

sgs.ai_skill_invoke.EightDiagram = function(self, data)
	local dying = 0
	for _, aplayer in sgs.qlist(self.room:getAlivePlayers()) do
		if aplayer:getHp() < 1 and not aplayer:hasSkill("buqu") then dying = 1 break end
	end

	local heart_jink = false
	for _, card in sgs.qlist(self.player:getCards("he")) do
		if card:getSuit() == sgs.Card_Heart and isCard("Jink", card, self.player) then
			heart_jink = true
			break
		end
	end

	if self:getDamagedEffects(self.player, nil, true) or self:needToLoseHp(self.player, nil, true, true) then return false end
	if self:getCardsNum("Jink") == 0 then return true end
	local zhangjiao = self.room:findPlayerBySkillName("guidao")
	if zhangjiao and self:isEnemy(zhangjiao) then
		if getKnownCard(zhangjiao, self.player, "black", false, "he") > 1 then return false end
		if self:getCardsNum("Jink") > 1 and getKnownCard(zhangjiao, self.player, "black", false, "he") > 0 then return false end
	end
	return true
end

function sgs.ai_armor_value.EightDiagram(player, self)
	local haszj = self:hasSkills("guidao", self:getEnemies(player))
	if haszj then
		return 2
	end
	if player:hasSkills("tiandu|leiji") then
		return 6
	end

	return 4
end

function sgs.ai_armor_value.RenwangShield(player, self)
	if player:hasSkill("bazhen") then return 0 end
	if player:hasSkill("leiji") and getKnownCard(player, self.player, "Jink", true) > 1 and player:hasSkill("guidao")
		and getKnownCard(player, self.player, "black", false, "he") > 0 then
			return 0
	end
	return 4.5
end

function sgs.ai_armor_value.SilverLion(player, self)
	if self:hasWizard(self:getEnemies(player), true) then
		for _, player in sgs.qlist(self.room:getAlivePlayers()) do
			if player:containsTrick("lightning") then return 5 end
		end
	end
	if self.player:isWounded() and not self.player:getArmor() then return 9 end
	if self.player:isWounded() and self:getCardsNum("Armor", "h") >= 2 and not self.player:hasArmorEffect("SilverLion") then return 8 end
	return 1
end

sgs.ai_use_priority.OffensiveHorse = 2.69

sgs.ai_use_priority.Axe = 2.688
sgs.ai_use_priority.Halberd = 2.685
sgs.ai_use_priority.KylinBow = 2.68
sgs.ai_use_priority.Blade = 2.675
sgs.ai_use_priority.GudingBlade = 2.67
sgs.ai_use_priority.DoubleSword =2.665
sgs.ai_use_priority.Spear = 2.66
sgs.ai_use_priority.IceSword = 2.65
-- sgs.ai_use_priority.Fan = 2.655
sgs.ai_use_priority.QinggangSword = 2.645
sgs.ai_use_priority.Crossbow = 2.63

sgs.ai_use_priority.SilverLion = 1.0
-- sgs.ai_use_priority.Vine = 0.95
sgs.ai_use_priority.EightDiagram = 0.8
sgs.ai_use_priority.RenwangShield = 0.85
sgs.ai_use_priority.DefensiveHorse = 2.75

sgs.dynamic_value.damage_card.ArcheryAttack = true
sgs.dynamic_value.damage_card.SavageAssault = true

sgs.ai_use_value.ArcheryAttack = 3.8
sgs.ai_use_priority.ArcheryAttack = 3.5
sgs.ai_keep_value.ArcheryAttack = 3.38
sgs.ai_use_value.SavageAssault = 3.9
sgs.ai_use_priority.SavageAssault = 3.5
sgs.ai_keep_value.SavageAssault = 3.36

sgs.ai_skill_cardask.aoe = function(self, data, pattern, target, name)
	if sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) then return "." end

	local aoe
	if type(data) == "userdata" then aoe = data:toCardEffect().card else aoe = sgs.Sanguosha:cloneCard(name) end
	assert(aoe ~= nil)
	local menghuo = self.room:findPlayerBySkillName("huoshou")
	local attacker = target
	if menghuo and aoe:isKindOf("SavageAssault") then attacker = menghuo end

	if not self:damageIsEffective(nil, nil, attacker) then return "." end
	if self:getDamagedEffects(self.player, attacker) or self:needToLoseHp(self.player, attacker) then return "." end

	if self.player:hasSkill("jianxiong") and (self.player:getHp() > 1 or self:getAllPeachNum() > 0)
		and not self:willSkipPlayPhase() then
		if not self:needKongcheng(self.player, true) and self:getAoeValue(aoe) > -10 then return "." end
	end

end

sgs.ai_skill_cardask["savage-assault-slash"] = function(self, data, pattern, target)
	return sgs.ai_skill_cardask.aoe(self, data, pattern, target, "savage_assault")
end

sgs.ai_skill_cardask["archery-attack-jink"] = function(self, data, pattern, target)
	return sgs.ai_skill_cardask.aoe(self, data, pattern, target, "archery_attack")
end

sgs.ai_keep_value.Nullification = 3.8
sgs.ai_use_value.Nullification = 8

function SmartAI:useCardAmazingGrace(card, use)
	-- if (self.role == "lord" or self.role == "loyalist") and sgs.turncount <= 2 and self.player:getSeat() <= 3 and self.player:aliveCount() > 5 then return end
	local value = 1
	local suf, coeff = 0.8, 0.8
	if self:needKongcheng() and self.player:getHandcardNum() == 1 or self.player:hasSkills("nosjizhi|jizhi") then
		suf = 0.6
		coeff = 0.6
	end
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		local index = 0
		if self:hasTrickEffective(card, player, self.player) then
			if self:isFriend(player) then index = 1 elseif self:isEnemy(player) then index = -1 end
		end
		value = value + index * suf
		if value < 0 then return end
		suf = suf * coeff
	end
	use.card = card
end

sgs.ai_use_value.AmazingGrace = 3
sgs.ai_keep_value.AmazingGrace = -1
sgs.ai_use_priority.AmazingGrace = 1.2
sgs.dynamic_value.benefit.AmazingGrace = true

function SmartAI:willUseGodSalvation(card)
	if not card then self.room:writeToConsole(debug.traceback()) return false end
	local good, bad = 0, 0
	local wounded_friend = 0
	local wounded_enemy = 0


	if self.player:hasSkill("jizhi") then good = good + 6 end
	if (self.player:hasSkill("kongcheng") and self.player:getHandcardNum() == 1) or not self:hasLoseHandcardEffective() then good = good + 5 end

	for _, friend in ipairs(self.friends) do
		good = good + 10 * getCardsNum("Nullification", friend, self.player)
		if self:hasTrickEffective(card, friend, self.player) then
			if friend:isWounded() then
				wounded_friend = wounded_friend + 1
				good = good + 10
				if friend:isLord() then good = good + 10 / math.max(friend:getHp(), 1) end
				if friend:hasSkills(sgs.masochism_skill) then
					good = good + 5
				end
				if friend:getHp() <= 1 and self:isWeak(friend) then
					good = good + 5
					if friend:isLord() then good = good + 10 end
				else
					if friend:isLord() then good = good + 5 end
				end
				if self:needToLoseHp(friend, nil, nil, true, true) then good = good - 3 end
			end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		bad = bad + 10 * getCardsNum("Nullification", enemy, self.player)
		if self:hasTrickEffective(card, enemy, self.player) then
			if enemy:isWounded() then
				wounded_enemy = wounded_enemy + 1
				bad = bad + 10
				if enemy:isLord() then
					bad = bad + 10 / math.max(enemy:getHp(), 1)
				end
				if enemy:hasSkills(sgs.masochism_skill) then
					bad = bad + 5
				end
				if enemy:getHp() <= 1 and self:isWeak(enemy) then
					bad = bad + 5
					if enemy:isLord() then bad = bad + 10 end
				else
					if enemy:isLord() then bad = bad + 5 end
				end
				if self:needToLoseHp(enemy, nil, nil, true, true) then bad = bad - 3 end
			end
		end
	end
	return good - bad > 5 and wounded_friend > 0
end

function SmartAI:useCardGodSalvation(card, use)
	if self:willUseGodSalvation(card) then
		use.card = card
	end
end

sgs.ai_use_priority.GodSalvation = 1.1
sgs.ai_keep_value.GodSalvation = 3.32
sgs.dynamic_value.benefit.GodSalvation = true
sgs.ai_card_intention.GodSalvation = function(self, card, from, tos)
	local can, first
	for _, to in ipairs(tos) do
		if to:isWounded() and not first then
			first = to
			can = true
		elseif first and to:isWounded() and not self:isFriend(first, to) then
			can = false
			break
		end
	end
	if can then
		sgs.updateIntention(from, first, -10)
	end
end

function SmartAI:useCardDuel(duel, use)

	local enemies = self:exclude(self.enemies, duel)
	local friends = self:exclude(self.friends_noself, duel)
	local n1 = self:getCardsNum("Slash")
	if self.player:hasSkill("wushuang") or use.isWuqian then
		n1 = n1 * 2
	end
	local huatuo = self.room:findPlayerBySkillName("jijiu")
	local targets = {}

	local canUseDuelTo=function(target)
		return self:hasTrickEffective(duel, target) and self:damageIsEffective(target,sgs.DamageStruct_Normal) and not self.room:isProhibited(self.player, target, duel)
	end

	for _, friend in ipairs(friends) do
		if (not use.current_targets or not table.contains(use.current_targets, friend:objectName()))
			and friend:hasSkill("jieming") and canUseDuelTo(friend) and self.player:hasSkill("rende") and (huatuo and self:isFriend(huatuo)) then
			table.insert(targets, friend)
		end
	end

	for _, enemy in ipairs(enemies) do
		if (not use.current_targets or not table.contains(use.current_targets, enemy:objectName()))
			and self.player:hasFlag("duelTo_" .. enemy:objectName()) and canUseDuelTo(enemy) then
			table.insert(targets, enemy)
		end
	end

	local cmp = function(a, b)
		local v1 = getCardsNum("Slash", a) + a:getHp()
		local v2 = getCardsNum("Slash", b) + b:getHp()

		if self:getDamagedEffects(a, self.player) then v1 = v1 + 20 end
		if self:getDamagedEffects(b, self.player) then v2 = v2 + 20 end

		if not self:isWeak(a) and a:hasSkill("jianxiong") then v1 = v1 + 10 end
		if not self:isWeak(b) and b:hasSkill("jianxiong") then v2 = v2 + 10 end

		if self:needToLoseHp(a) then v1 = v1 + 5 end
		if self:needToLoseHp(b) then v2 = v2 + 5 end

		if a:hasSkills(sgs.masochism_skill) then v1 = v1 + 5 end
		if b:hasSkills(sgs.masochism_skill) then v2 = v2 + 5 end

		if not self:isWeak(a) and a:hasSkill("jiang") then v1 = v1 + 5 end
		if not self:isWeak(b) and b:hasSkill("jiang") then v2 = v2 + 5 end

		if v1 == v2 then return sgs.getDefenseSlash(a, self) < sgs.getDefenseSlash(b, self) end

		return v1 < v2
	end

	table.sort(enemies, cmp)

	for _, enemy in ipairs(enemies) do
		local useduel
		local n2 = getCardsNum("Slash", enemy)
		if enemy:hasSkill("wushuang") then n2 = n2 * 2 end
		if sgs.card_lack[enemy:objectName()]["Slash"] == 1 then n2 = 0 end
		useduel = n1 >= n2 or self:needToLoseHp(self.player, nil, nil, true)
					or self:getDamagedEffects(self.player, enemy) or (n2 < 1 and sgs.isGoodHp(self.player))
					or ((self:hasSkill("jianxiong") or self.player:getMark("shuangxiong") > 0) and sgs.isGoodHp(self.player))

		if (not use.current_targets or not table.contains(use.current_targets, enemy:objectName()))
			and self:objectiveLevel(enemy) > 3 and canUseDuelTo(enemy) and not self:cantbeHurt(enemy) and useduel and sgs.isGoodTarget(enemy, enemies, self) then
			if not table.contains(targets, enemy) then table.insert(targets, enemy) end
		end
	end

	if #targets > 0 then

		local godsalvation = self:getCard("GodSalvation")
		if godsalvation and godsalvation:getId() ~= duel:getId() and self:willUseGodSalvation(godsalvation) then
			local use_gs = true
			for _, p in ipairs(targets) do
				if not p:isWounded() or not self:hasTrickEffective(godsalvation, p, self.player) then break end
				use_gs = false
			end
			if use_gs then
				use.card = godsalvation
				return
			end
		end

		local targets_num = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, duel)
		if use.isDummy and use.xiechan then targets_num = 100 end
		local enemySlash = 0
		local setFlag = false

		use.card = duel

		for i = 1, #targets, 1 do
			local n2 = getCardsNum("Slash", targets[i])
			if sgs.card_lack[targets[i]:objectName()]["Slash"] == 1 then n2 = 0 end
			if self:isEnemy(targets[i]) then enemySlash = enemySlash + n2 end

			if use.to then
				if i == 1 and not use.current_targets then
					use.to:append(targets[i])
					if not use.isDummy then self:speak("duel", self.player:isFemale()) end
				end
				if not setFlag and self.player:getPhase() == sgs.Player_Play and self:isEnemy(targets[i]) then
					self.player:setFlags("duelTo" .. targets[i]:objectName())
					setFlag = true
				end
				if use.to:length() == targets_num then return end
			end
		end
	end

end

sgs.ai_card_intention.Duel = function(self, card, from, tos)
	if string.find(card:getSkillName(), "lijian") then return end
	sgs.updateIntentions(from, tos, 80)
end

sgs.ai_use_value.Duel = 3.7
sgs.ai_use_priority.Duel = 2.9
sgs.ai_keep_value.Duel = 3.42

sgs.dynamic_value.damage_card.Duel = true

sgs.ai_skill_cardask["duel-slash"] = function(self, data, pattern, target)
	if self.player:getPhase()==sgs.Player_Play then return self:getCardId("Slash") end

	if sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) then return "." end

	if self:cantbeHurt(target) then return "." end

	if self:isFriend(target) and target:hasSkill("rende") and self.player:hasSkill("jieming") then return "." end
	if self:isEnemy(target) and not self:isWeak() and self:getDamagedEffects(self.player, target) then return "." end

	if self:isFriend(target) then
		if self:getDamagedEffects(self.player, target) or self:needToLoseHp(self.player, target) then return "." end
		if self:getDamagedEffects(target, self.player) or self:needToLoseHp(target, self.player) then
			return self:getCardId("Slash")
		else
			return "."
		end
	end

	if (not self:isFriend(target) and self:getCardsNum("Slash") >= getCardsNum("Slash", target, self.player))
		or (target:getHp() > 2 and self.player:getHp() <= 1 and self:getCardsNum("Peach") == 0 and not self.player:hasSkill("buqu")) then
		return self:getCardId("Slash")
	else return "." end

end

function SmartAI:useCardExNihilo(card, use)
	use.card = card
	if not use.isDummy then
		self:speak("lucky")
	end
end

sgs.ai_card_intention.ExNihilo = -80

sgs.ai_keep_value.ExNihilo = 3.9
sgs.ai_use_value.ExNihilo = 10
sgs.ai_use_priority.ExNihilo = 9.3

sgs.dynamic_value.benefit.ExNihilo = true

function SmartAI:getDangerousCard(who)
	local weapon = who:getWeapon()
	local armor = who:getArmor()
	if weapon and (weapon:isKindOf("Crossbow") or weapon:isKindOf("GudingBlade")) then
		for _, friend in ipairs(self.friends) do
			if weapon:isKindOf("Crossbow") and who:distanceTo(friend) <= 1 and getCardsNum("Slash", who, self.player) > 0 then
				return weapon:getEffectiveId()
			end
			if weapon:isKindOf("GudingBlade") and who:inMyAttackRange(friend) and friend:isKongcheng() and not friend:hasSkill("kongcheng") and getCardsNum("Slash", who) > 0 then
				return weapon:getEffectiveId()
			end
		end
	end
	if (weapon and weapon:isKindOf("Spear") and who:hasSkill("paoxiao") and who:getHandcardNum() >=1 ) then return weapon:getEffectiveId() end
	if weapon and weapon:isKindOf("Axe") and who:hasSkill("luoyi") then
		return weapon:getEffectiveId()
	end
	if armor and armor:isKindOf("EightDiagram") and who:hasSkill("leiji") then return armor:getEffectiveId() end

	if (weapon and who:hasSkill("liegong")) then return weapon:getEffectiveId() end
end

function SmartAI:getValuableCard(who)
	local weapon = who:getWeapon()
	local armor = who:getArmor()
	local offhorse = who:getOffensiveHorse()
	local defhorse = who:getDefensiveHorse()
	self:sort(self.friends, "hp")
	local friend
	if #self.friends > 0 then friend = self.friends[1] end
	if friend and self:isWeak(friend) and who:distanceTo(friend) <= who:getAttackRange() and not self:doNotDiscard(who, "e", true) then
		if weapon and (who:distanceTo(friend) > 1) then
			return weapon:getEffectiveId()
		end
		if offhorse and who:distanceTo(friend) > 1 then
			return offhorse:getEffectiveId()
		end
	end

	if defhorse and not self:doNotDiscard(who, "e")
		and not (self.player:hasWeapon("KylinBow") and self.player:canSlash(who) and self:slashIsEffective(sgs.Sanguosha:cloneCard("slash"), who, self.player)
				and (getCardsNum("Jink", who, self.player) < 1 or sgs.card_lack[who:objectName()].Jink == 1 )) then
		return defhorse:getEffectiveId()
	end

	if armor and self:evaluateArmor(armor, who) > 3
	  and not self:needToThrowArmor(who)
	  and not self:doNotDiscard(who, "e") then
		return armor:getEffectiveId()
	end

	if offhorse then
		if who:hasSkills("kuanggu|duanbing|qianxi") then
			return offhorse:getEffectiveId()
		end
	end

	local equips = sgs.QList2Table(who:getEquips())
	for _,equip in ipairs(equips) do
		if who:hasSkills("guose") and equip:getSuit() == sgs.Card_Diamond then  return equip:getEffectiveId() end
		if who:hasSkills("qixi|duanliang|guidao") and equip:isBlack() then  return equip:getEffectiveId() end
		if who:hasSkills("wusheng|jijiu") and equip:isRed() then  return equip:getEffectiveId() end
		if who:hasSkills(sgs.need_equip_skill) and not who:hasSkills(sgs.lose_equip_skill) then return equip:getEffectiveId() end
	end

	if armor and not self:needToThrowArmor(who) and not self:doNotDiscard(who, "e") then
		return armor:getEffectiveId()
	end

	if offhorse and who:getHandcardNum() > 1 then
		if not self:doNotDiscard(who, "e", true) then
			for _,friend in ipairs(self.friends) do
				if who:distanceTo(friend) == who:getAttackRange() and who:getAttackRange() > 1 then
					return offhorse:getEffectiveId()
				end
			end
		end
	end

	if weapon and who:getHandcardNum() > 1 then
		if not self:doNotDiscard(who, "e", true) then
			for _,friend in ipairs(self.friends) do
				if (who:distanceTo(friend) <= who:getAttackRange()) and (who:distanceTo(friend) > 1) then
					return weapon:getEffectiveId()
				end
			end
		end
	end
end

function SmartAI:useCardSnatchOrDismantlement(card, use)
	local isJixi = card:getSkillName() == "jixi"
	local isDiscard = (not card:isKindOf("Snatch"))
	local name = card:objectName()
	local players = self.room:getOtherPlayers(self.player)
	local tricks
	local usecard = false

	local targets = {}
	local targets_num = (1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, card))

	local addTarget = function(player, cardid)
		if not table.contains(targets, player:objectName())
			and (not use.current_targets or not table.contains(use.current_targets, player:objectName())) then
			if not usecard then
				use.card = card
				usecard = true
			end
			table.insert(targets, player:objectName())
			if usecard and use.to and use.to:length() < targets_num then
				use.to:append(player)
				if not use.isDummy then
					sgs.Sanguosha:getCard(cardid):setFlags("AIGlobal_SDCardChosen_" .. name)
					if use.to:length() == 1 then self:speak("hostile", self.player:isFemale()) end
				end
			end
			if #targets == targets_num then return true end
		end
	end

	players = self:exclude(players, card)
	for _, player in ipairs(players) do
		if not player:getJudgingArea():isEmpty() and self:hasTrickEffective(card, player)
			and ((player:containsTrick("lightning") and self:getFinalRetrial(player) == 2) or #self.enemies == 0) then
			tricks = player:getCards("j")
			for _, trick in sgs.qlist(tricks) do
				if trick:isKindOf("Lightning") and (not isDiscard or self.player:canDiscard(player, trick:getId())) then
					if addTarget(player, trick:getEffectiveId()) then return end
				end
			end
		end
	end

	local enemies = {}
	if #self.enemies == 0 and self:getOverflow() > 0 then
		enemies = self:exclude(enemies, card)
		self:sort(enemies, "defense")
		enemies = sgs.reverse(enemies)
	else
		enemies = self:exclude(self.enemies, card)
		self:sort(enemies, "defense")
	end

	for _, enemy in ipairs(enemies) do
		if self:slashIsAvailable() then
			for _, slash in ipairs(self:getCards("Slash")) do
				if not self:slashProhibit(slash, enemy) and enemy:getHandcardNum() == 1 and enemy:getHp() == 1 and self:hasLoseHandcardEffective(enemy)
					and self:objectiveLevel(enemy) > 3 and not self:cantbeHurt(enemy) and not enemy:hasSkill("kongcheng")
					and self.player:canSlash(enemy, slash) and self:hasTrickEffective(card, enemy)
					and (not enemy:isChained() or self:isGoodChainTarget(enemy, nil, nil, nil, slash))
					and (not self:hasEightDiagramEffect(enemy) or IgnoreArmor(self.player, enemy)) then
					if addTarget(enemy, enemy:getHandcards():first():getEffectiveId()) then return end
				end
			end
		end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(card, enemy) then
			local dangerous = self:getDangerousCard(enemy)
			if dangerous and (not isDiscard or self.player:canDiscard(enemy, dangerous)) then
				if addTarget(enemy, dangerous) then return end
			end
		end
	end

	self:sort(self.friends_noself, "defense")
	local friends = self:exclude(self.friends_noself, card)
	for _, friend in ipairs(friends) do
		if (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) and self:hasTrickEffective(card, friend) then
			local cardchosen
			tricks = friend:getJudgingArea()
			for _, trick in sgs.qlist(tricks) do
				if trick:isKindOf("Indulgence") and (not isDiscard or self.player:canDiscard(friend, trick:getId())) then
					if friend:getHp() <= friend:getHandcardNum() or friend:isLord() or name == "snatch" then
						cardchosen = trick:getEffectiveId()
						break
					end
				end
				if trick:isKindOf("SupplyShortage") and (not isDiscard or self.player:canDiscard(friend, trick:getId())) then
					cardchosen = trick:getEffectiveId()
					break
				end
				if trick:isKindOf("Indulgence") and (not isDiscard or self.player:canDiscard(friend, trick:getId())) then
					cardchosen = trick:getEffectiveId()
					break
				end
			end
			if cardchosen then
				if addTarget(friend, cardchosen) then return end
			end
		end
	end

	local hasLion, target
	for _, friend in ipairs(friends) do
		if self:hasTrickEffective(card, friend) and self:needToThrowArmor(friend) and (not isDiscard or self.player:canDiscard(friend, friend:getArmor():getEffectiveId())) then
			hasLion = true
			target = friend
		end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(card, enemy) then
			local valuable = self:getValuableCard(enemy)
			if valuable and (not isDiscard or self.player:canDiscard(enemy, valuable)) then
				if addTarget(enemy, valuable) then return end
			end
		end
	end

	for _, enemy in ipairs(enemies) do
		local cards = sgs.QList2Table(enemy:getHandcards())
		local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), enemy:objectName())
		if #cards <= 2 and self:hasTrickEffective(card, enemy) and not enemy:isKongcheng() and not self:doNotDiscard(enemy, "h", true) then
			for _, cc in ipairs(cards) do
				if (cc:hasFlag("visible") or cc:hasFlag(flag)) and (cc:isKindOf("Peach") or cc:isKindOf("Analeptic")) then
					if addTarget(enemy, self:getCardRandomly(enemy, "h")) then return end
				end
			end
		end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(card, enemy) then
			if enemy:hasSkills("jijiu|qingnang|jieyin") then
				local cardchosen
				local equips = { enemy:getDefensiveHorse(), enemy:getArmor(), enemy:getOffensiveHorse(), enemy:getWeapon() }
				for _, equip in ipairs(equips) do
					if equip and (not enemy:hasSkill("jijiu") or equip:isRed()) and (not isDiscard or self.player:canDiscard(enemy, equip:getEffectiveId())) then
						cardchosen = equip:getEffectiveId()
						break
					end
				end

				if not cardchosen and enemy:getDefensiveHorse() and (not isDiscard or self.player:canDiscard(enemy, enemy:getDefensiveHorse():getEffectiveId())) then cardchosen = enemy:getDefensiveHorse():getEffectiveId() end
				if not cardchosen and enemy:getArmor() and not self:needToThrowArmor(enemy) and (not isDiscard or self.player:canDiscard(enemy, enemy:getArmor():getEffectiveId())) then
					cardchosen = enemy:getArmor():getEffectiveId()
				end
				if not cardchosen and not enemy:isKongcheng() and enemy:getHandcardNum() <= 3 and (not isDiscard or self.player:canDiscard(enemy, "h")) then
					cardchosen = self:getCardRandomly(enemy, "h")
				end

				if cardchosen then
					if addTarget(enemy, cardchosen) then return end
				end
			end
		end
	end

	for _, enemy in ipairs(enemies) do
		if self:hasTrickEffective(card, enemy) and enemy:hasArmorEffect("EightDiagram")
			and not self:needToThrowArmor(enemy) and (not isDiscard or self.player:canDiscard(enemy, enemy:getArmor():getEffectiveId())) then
			addTarget(enemy, enemy:getArmor():getEffectiveId())
		end
	end

	for i = 1, 2 + (isJixi and 3 or 0), 1 do
		for _, enemy in ipairs(enemies) do
			if not enemy:isNude() and self:hasTrickEffective(card, enemy)
				and not (self:needKongcheng(enemy) and i <= 2) and not self:doNotDiscard(enemy) then
				if (enemy:getHandcardNum() == i and sgs.getDefenseSlash(enemy) < 6 + (isJixi and 6 or 0) and enemy:getHp() <= 3 + (isJixi and 2 or 0)) then
					local cardchosen
					if self.player:distanceTo(enemy) == self.player:getAttackRange() + 1 and enemy:getDefensiveHorse() and not self:doNotDiscard(enemy, "e")
						and (not isDiscard or self.player:canDiscard(enemy, enemy:getDefensiveHorse():getEffectiveId()))then
						cardchosen = enemy:getDefensiveHorse():getEffectiveId()
					elseif enemy:getArmor() and not self:needToThrowArmor(enemy) and not self:doNotDiscard(enemy, "e")
						and (not isDiscard or self.player:canDiscard(enemy, enemy:getArmor():getEffectiveId()))then
						cardchosen = enemy:getArmor():getEffectiveId()
					elseif not isDiscard or self.player:canDiscard(enemy, "h") then
						cardchosen = self:getCardRandomly(enemy, "h")
					end
					if cardchosen then
						if addTarget(enemy, cardchosen) then return end
					end
				end
			end
		end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(card, enemy) then
			local valuable = self:getValuableCard(enemy)
			if valuable and (not isDiscard or self.player:canDiscard(enemy, valuable)) then
				if addTarget(enemy, valuable) then return end
			end
		end
	end

	if hasLion and (not isDiscard or self.player:canDiscard(target, target:getArmor():getEffectiveId())) then
		if addTarget(target, target:getArmor():getEffectiveId()) then return end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isKongcheng() and not self:doNotDiscard(enemy, "h") and self:hasTrickEffective(card, enemy)
			and enemy:hasSkills(sgs.cardneed_skill) and (not isDiscard or self.player:canDiscard(enemy, "h")) then
			if addTarget(enemy, self:getCardRandomly(enemy, "h")) then return end
		end
	end

	for _, enemy in ipairs(enemies) do
		if enemy:hasEquip() and not self:doNotDiscard(enemy, "e") and self:hasTrickEffective(card, enemy) then
			local cardchosen
			if enemy:getDefensiveHorse() and (not isDiscard or self.player:canDiscard(enemy, enemy:getDefensiveHorse():getEffectiveId())) then
				cardchosen = enemy:getDefensiveHorse():getEffectiveId()
			elseif enemy:getArmor() and not self:needToThrowArmor(enemy) and (not isDiscard or self.player:canDiscard(enemy, enemy:getArmor():getEffectiveId())) then
				cardchosen = enemy:getArmor():getEffectiveId()
			elseif enemy:getOffensiveHorse() and (not isDiscard or self.player:canDiscard(enemy, enemy:getOffensiveHorse():getEffectiveId())) then
				cardchosen = enemy:getOffensiveHorse():getEffectiveId()
			elseif enemy:getWeapon() and (not isDiscard or self.player:canDiscard(enemy, enemy:getWeapon():getEffectiveId())) then
				cardchosen = enemy:getWeapon():getEffectiveId()
			end
			if cardchosen then
				if addTarget(enemy, cardchosen) then return end
			end
		end
	end

	if name == "snatch" or self:getOverflow() > 0 then
		for _, enemy in ipairs(enemies) do
			local equips = enemy:getEquips()
			if not enemy:isNude() and self:hasTrickEffective(card, enemy) and not self:doNotDiscard(enemy, "he") then
				local cardchosen
				if not equips:isEmpty() and not self:doNotDiscard(enemy, "e") then
					cardchosen = self:getCardRandomly(enemy, "e")
				else
					cardchosen = self:getCardRandomly(enemy, "h") end
				if cardchosen then
					if addTarget(enemy, cardchosen) then return end
				end
			end
		end
	end
end

SmartAI.useCardSnatch = SmartAI.useCardSnatchOrDismantlement

sgs.ai_use_value.Snatch = 9
sgs.ai_use_priority.Snatch = 4.3
sgs.ai_keep_value.Snatch = 3.46

sgs.dynamic_value.control_card.Snatch = true

SmartAI.useCardDismantlement = SmartAI.useCardSnatchOrDismantlement

sgs.ai_use_value.Dismantlement = 5.6
sgs.ai_use_priority.Dismantlement = 4.4
sgs.ai_keep_value.Dismantlement = 3.44

sgs.dynamic_value.control_card.Dismantlement = true

sgs.ai_choicemade_filter.cardChosen.snatch = function(self, player, promptlist)
	local from = findPlayerByObjectName(promptlist[4])
	local to = findPlayerByObjectName(promptlist[5])
	if from and to then
		local id = tonumber(promptlist[3])
		local place = self.room:getCardPlace(id)
		local card = sgs.Sanguosha:getCard(id)
		local intention = 70
		if place == sgs.Player_PlaceDelayedTrick then
			if not card:isKindOf("Disaster") then intention = -intention else intention = 0 end
		elseif place == sgs.Player_PlaceEquip then
			if card:isKindOf("Armor") and self:evaluateArmor(card, to) <= -2 then intention = 0 end
			if card:isKindOf("SilverLion") then
				if to:getLostHp() > 1 then
					if to:hasSkills(sgs.use_lion_skill) then
						intention = self:willSkipPlayPhase(to) and -intention or 0
					else
						intention = self:isWeak(to) and -intention or 0
					end
				else
					intention = 0
				end
			elseif to:hasSkills(sgs.lose_equip_skill) then
				if self:isWeak(to) and (card:isKindOf("DefensiveHorse") or card:isKindOf("Armor")) then
					intention = math.abs(intention)
				else
					intention = 0
				end
			end
		elseif place == sgs.Player_PlaceHand then
			if self:needKongcheng(to, true) and to:getHandcardNum() == 1 then
				intention = 0
			end
		end
		sgs.updateIntention(from, to, intention)
	end
end

sgs.ai_choicemade_filter.cardChosen.dismantlement = sgs.ai_choicemade_filter.cardChosen.snatch

function SmartAI:useCardCollateral(card, use)
	local fromList = sgs.QList2Table(self.room:getOtherPlayers(self.player))
	local toList   = sgs.QList2Table(self.room:getAlivePlayers())

	local cmp = function(a, b)
		local alevel = self:objectiveLevel(a)
		local blevel = self:objectiveLevel(b)

		if alevel ~= blevel then return alevel > blevel end

		local anum = getCardsNum("Slash", a)
		local bnum = getCardsNum("Slash", b)

		if anum ~= bnum then return anum < bnum end
		return a:getHandcardNum() < b:getHandcardNum()
	end

	table.sort(fromList, cmp)
	self:sort(toList, "defense")

	local needCrossbow = false
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy) and self:objectiveLevel(enemy) > 3
			and sgs.isGoodTarget(enemy, self.enemies, self) and not self:slashProhibit(nil, enemy) then
			needCrossbow = true
			break
		end
	end

	needCrossbow = needCrossbow and self:getCardsNum("Slash") > 2 and not self.player:hasSkill("paoxiao")

	if needCrossbow then
		for i = #fromList, 1, -1 do
			local friend = fromList[i]
			if (not use.current_targets or not table.contains(use.current_targets, friend:objectName()))
				and friend:getWeapon() and friend:getWeapon():isKindOf("Crossbow") and self:hasTrickEffective(card, friend) then
				for _, enemy in ipairs(toList) do
					if friend:canSlash(enemy, nil) and friend:objectName() ~= enemy:objectName() then
						self.room:setPlayerFlag(self.player, "needCrossbow")
						use.card = card
						if use.to then use.to:append(friend) end
						if use.to then use.to:append(enemy) end
						return
					end
				end
			end
		end
	end

	local n = nil
	local final_enemy = nil
	for _, enemy in ipairs(fromList) do
		if (not use.current_targets or not table.contains(use.current_targets, enemy:objectName()))
			and self:hasTrickEffective(card, enemy)
			and not enemy:hasSkills(sgs.lose_equip_skill)
			and not (enemy:hasSkill("weimu") and card:isBlack())
			and not enemy:hasSkill("tuntian")
			and self:objectiveLevel(enemy) >= 0
			and enemy:getWeapon() then

			for _, enemy2 in ipairs(toList) do
				if enemy:canSlash(enemy2) and self:objectiveLevel(enemy2) > 3 and enemy:objectName() ~= enemy2:objectName() then
					n = 1
					final_enemy = enemy2
					break
				end
			end

			if not n then
				for _, enemy2 in ipairs(toList) do
					if enemy:canSlash(enemy2) and self:objectiveLevel(enemy2) <=3 and self:objectiveLevel(enemy2) >=0 and enemy:objectName() ~= enemy2:objectName() then
						n = 1
						final_enemy = enemy2
						break
					end
				end
			end

			if not n then
				for _, friend in ipairs(toList) do
					if enemy:canSlash(friend) and self:objectiveLevel(friend) < 0 and enemy:objectName() ~= friend:objectName()
							and (self:needToLoseHp(friend, enemy, true, true) or self:getDamagedEffects(friend, enemy, true)) then
						n = 1
						final_enemy = friend
						break
					end
				end
			end

			if not n then
				for _, friend in ipairs(toList) do
					if enemy:canSlash(friend) and self:objectiveLevel(friend) < 0 and enemy:objectName() ~= friend:objectName()
							and (getKnownCard(friend, self.player, "Jink", true, "he") >= 2 or getCardsNum("Slash", enemy) < 1) then
						n = 1
						final_enemy = friend
						break
					end
				end
			end

			if n then
				use.card = card
				if use.to then use.to:append(enemy) end
				if use.to then use.to:append(final_enemy) end
				return
			end
		end
		n = nil
	end

	for _, friend in ipairs(fromList) do
		if (not use.current_targets or not table.contains(use.current_targets, friend:objectName()))
			and friend:getWeapon() and (getKnownCard(friend, self.player, "Slash", true, "he") > 0 or getCardsNum("Slash", friend) > 1 and friend:getHandcardNum() >= 4)
			and self:hasTrickEffective(card, friend)
			and self:objectiveLevel(friend) < 0
			and not self.room:isProhibited(self.player, friend, card) then

			for _, enemy in ipairs(toList) do
				if friend:canSlash(enemy, nil) and self:objectiveLevel(enemy) > 3 and friend:objectName() ~= enemy:objectName()
						and sgs.isGoodTarget(enemy, self.enemies, self) and not self:slashProhibit(nil, enemy) then
					use.card = card
					if use.to then use.to:append(friend) end
					if use.to then use.to:append(enemy) end
					return
				end
			end
		end
	end

	self:sortEnemies(toList)

	for _, friend in ipairs(fromList) do
		if (not use.current_targets or not table.contains(use.current_targets, friend:objectName()))
			and friend:getWeapon() and friend:hasSkills(sgs.lose_equip_skill)
			and self:hasTrickEffective(card, friend)
			and self:objectiveLevel(friend) < 0
			and not (friend:getWeapon():isKindOf("Crossbow") and getCardsNum("Slash", friend) > 1)
			and not self.room:isProhibited(self.player, friend, card) then

			for _, enemy in ipairs(toList) do
				if friend:canSlash(enemy, nil) and friend:objectName() ~= enemy:objectName() then
					use.card = card
					if use.to then use.to:append(friend) end
					if use.to then use.to:append(enemy) end
					return
				end
			end
		end
	end
end

sgs.ai_use_value.Collateral = 5.8
sgs.ai_use_priority.Collateral = 2.75
sgs.ai_keep_value.Collateral = 3.40

sgs.ai_card_intention.Collateral = function(self,card, from, tos)
	assert(#tos == 1)
	sgs.ai_collateral = true
end

sgs.dynamic_value.control_card.Collateral = true

sgs.ai_skill_cardask["collateral-slash"] = function(self, data, pattern, target2, target, prompt)
	-- self.player = killer
	-- target = user
	-- target2 = victim
	if true then return "." end
	local current = self.room:getCurrent()
	if self:isFriend(current) and (current:hasFlag("needCrossbow") or
			(getCardsNum("Slash", current, self.player) >= 2 and self.player:getWeapon():isKindOf("Crossbow"))) then
		if current:hasFlag("needCrossbow") then self.room:setPlayerFlag(current, "-needCrossbow") end
		return "."
	end

	if self:isFriend(target2) and self:needLeiji(target2, self.player) then
		for _, slash in ipairs(self:getCards("Slash")) do
			if self:slashIsEffective(slash, target2) then
				return slash:toString()
			end
		end
	end

	if target2 and (self:getDamagedEffects(target2, self.player, true) or self:needToLoseHp(target2, self.player, true)) then
		for _, slash in ipairs(self:getCards("Slash")) do
			if self:slashIsEffective(slash, target2) and self:isFriend(target2) then
				return slash:toString()
			end
			if not self:slashIsEffective(slash, target2, self.player, true) and self:isEnemy(target2) then
				return slash:toString()
			end
		end
		for _, slash in ipairs(self:getCards("Slash")) do
			if not self:getDamagedEffects(target2, self.player, true) and self:isEnemy(target2) then
				return slash:toString()
			end
		end
	end

	if target2 and not self.player:hasSkills(sgs.lose_equip_skill) and self:isEnemy(target2) then
		for _, slash in ipairs(self:getCards("Slash")) do
			if self:slashIsEffective(slash, target2) then
				return slash:toString()
			end
		end
	end
	if target2 and not self.player:hasSkills(sgs.lose_equip_skill) and self:isFriend(target2) then
		for _, slash in ipairs(self:getCards("Slash")) do
			if not self:slashIsEffective(slash, target2) then
				return slash:toString()
			end
		end
		for _, slash in ipairs(self:getCards("Slash")) do
			if (target2:getHp() > 3 or not self:canHit(target2, self.player, self:hasHeavySlashDamage(self.player, slash, target2)))
				and self.player:getHandcardNum() > 1 then
					return slash:toString()
			end
			if self:needToLoseHp(target2, self.player) then return slash:toString() end
		end
	end
	self:speak("collateral", self.player:isFemale())
	return "."
end

local function hp_subtract_handcard(a,b)
	local diff1 = a:getHp() - a:getHandcardNum()
	local diff2 = b:getHp() - b:getHandcardNum()

	return diff1 < diff2
end

function SmartAI:enemiesContainsTrick(EnemyCount)
	local trick_all, possible_indul_enemy, possible_ss_enemy = 0, 0, 0
	local indul_num = self:getCardsNum("Indulgence")
	local ss_num = self:getCardsNum("SupplyShortage")
	local enemy_num, temp_enemy = 0

	local zhanghe = self.room:findPlayerBySkillName("qiaobian")
	if zhanghe and (not self:isEnemy(zhanghe) or zhanghe:isKongcheng() or not zhanghe:faceUp()) then zhanghe = nil end

	if self.player:hasSkill("guose") then
		for _, acard in sgs.qlist(self.player:getCards("he")) do
			if acard:getSuit() == sgs.Card_Diamond then indul_num = indul_num + 1 end
		end
	end

	if self.player:hasSkill("duanliang") then
		for _, acard in sgs.qlist(self.player:getCards("he")) do
			if acard:isBlack() then ss_num = ss_num + 1 end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if enemy:containsTrick("indulgence") then
			if not enemy:hasSkill("keji") and	(not zhanghe or self:playerGetRound(enemy) >= self:playerGetRound(zhanghe)) then
				trick_all = trick_all + 1
				if not temp_enemy or temp_enemy:objectName() ~= enemy:objectName() then
					enemy_num = enemy_num + 1
					temp_enemy = enemy
				end
			end
		else
			possible_indul_enemy = possible_indul_enemy + 1
		end
		if self.player:distanceTo(enemy) == 1 or self.player:hasSkill("duanliang") and self.player:distanceTo(enemy) <= 2 then
			if enemy:containsTrick("supply_shortage") then
				if not enemy:hasSkill("shensu") and (not zhanghe or self:playerGetRound(enemy) >= self:playerGetRound(zhanghe)) then
					trick_all = trick_all + 1
					if not temp_enemy or temp_enemy:objectName() ~= enemy:objectName() then
						enemy_num = enemy_num + 1
						temp_enemy = enemy
					end
				end
			else
				possible_ss_enemy  = possible_ss_enemy + 1
			end
		end
	end
	indul_num = math.min(possible_indul_enemy, indul_num)
	ss_num = math.min(possible_ss_enemy, ss_num)
	if not EnemyCount then
		return trick_all + indul_num + ss_num
	else
		return enemy_num + indul_num + ss_num
	end
end

function SmartAI:playerGetRound(player, source)
	if not player then return self.room:writeToConsole(debug.traceback()) end
	source = source or self.room:getCurrent()
	if player:objectName() == source:objectName() then return 0 end
	local players_num = self.room:alivePlayerCount()
	local round = (player:getSeat() - source:getSeat()) % players_num
	return round
end

function SmartAI:useCardIndulgence(card, use)
	local enemies = {}

	if #self.enemies == 0 then
		if sgs.turncount <= 1 and sgs.isAnjiang(self.player:getNextAlive()) then
			enemies = self:exclude({self.player:getNextAlive()}, card)
		end
	else
		enemies = self:exclude(self.enemies, card)
	end

	local zhanghe = self.room:findPlayerBySkillName("qiaobian")
	local zhanghe_seat = zhanghe and zhanghe:faceUp() and not zhanghe:isKongcheng() and not self:isFriend(zhanghe) and zhanghe:getSeat() or 0

	if #enemies == 0 then return end

	local getvalue = function(enemy)
		if enemy:hasSkill("qianxun") then return -101 end
		if enemy:hasSkill("weimu") and card:isBlack() then return -101 end
		if enemy:containsTrick("indulgence") then return -101 end
		if enemy:hasSkill("qiaobian") and not enemy:containsTrick("supply_shortage") and not enemy:containsTrick("indulgence") then return -101 end
		if zhanghe_seat > 0 and (self:playerGetRound(zhanghe) <= self:playerGetRound(enemy) and self:enemiesContainsTrick() <= 1 or not enemy:faceUp()) then
			return -101 end

		local value = enemy:getHandcardNum() - enemy:getHp()

		if enemy:hasSkills("lijian|fanjian|dimeng|jijiu|jieyin|zhiheng|rende") then value = value + 10 end
		if enemy:hasSkills("qixi|guose|duanliang|luoshen|jizhi|wansha") then value = value + 5 end
		if enemy:hasSkills("guzheng|duoshi") then value = value + 3 end
		if self:isWeak(enemy) then value = value + 3 end
		if enemy:isLord() then value = value + 3 end

		if self:objectiveLevel(enemy) < 3 then value = value - 10 end
		if not enemy:faceUp() then value = value - 10 end
		if enemy:hasSkills("keji|shensu") then value = value - enemy:getHandcardNum() end
		if enemy:hasSkills("lirang|guanxing") then value = value - 5 end
		if enemy:hasSkills("tuxi|tiandu") then value = value - 3 end
		if not sgs.isGoodTarget(enemy, self.enemies, self) then value = value - 1 end
		return value
	end

	local cmp = function(a,b)
		return getvalue(a) > getvalue(b)
	end

	table.sort(enemies, cmp)

	local target = enemies[1]
	if getvalue(target) > -100 then
		use.card = card
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_use_value.Indulgence = 8
sgs.ai_use_priority.Indulgence = 0.5
sgs.ai_card_intention.Indulgence = 120
sgs.ai_keep_value.Indulgence = 3.5

sgs.dynamic_value.control_usecard.Indulgence = true

function SmartAI:willUseLightning(card)
	if not card then self.room:writeToConsole(debug.traceback()) return false end
	if self.player:containsTrick("lightning") then return end
	if self.player:hasSkill("weimu") and card:isBlack() then return end
	if self.room:isProhibited(self.player, self.player, card) then return end

	local function hasDangerousFriend()
		local hashy = false
		for _, aplayer in ipairs(self.enemies) do
			if aplayer:hasSkill("hongyan") then hashy = true break end
		end
		for _, aplayer in ipairs(self.enemies) do
			if aplayer:hasSkill("guanxing") and self:isFriend(aplayer:getNextAlive()) then return true end
		end
		return false
	end

	if self:getFinalRetrial(self.player) == 2 then
	return
	elseif self:getFinalRetrial(self.player) == 1 then
		return true
	elseif not hasDangerousFriend() then
		local players = self.room:getAllPlayers()
		players = sgs.QList2Table(players)

		local friends = 0
		local enemies = 0

		for _,player in ipairs(players) do
			if self:objectiveLevel(player) >= 4 and not player:hasSkill("hongyan") and not (player:hasSkill("weimu") and card:isBlack()) then
				enemies = enemies + 1
			elseif self:isFriend(player) and not player:hasSkill("hongyan") and not (player:hasSkill("weimu") and card:isBlack()) then
				friends = friends + 1
			end
		end

		local ratio

		if friends == 0 then ratio = 999
		else ratio = enemies/friends
		end

		if ratio > 1.5 then
			return true
		end
	end
end

function SmartAI:useCardLightning(card, use)
	if self:willUseLightning(card) then
		use.card = card
	end
end

sgs.ai_use_priority.Lightning = 0
sgs.dynamic_value.lucky_chance.Lightning = true

sgs.ai_keep_value.Lightning = -1

sgs.ai_skill_askforag.amazing_grace = function(self, card_ids)

	local NextPlayerCanUse, NextPlayerisEnemy
	local NextPlayer = self.player:getNextAlive()
	if sgs.turncount > 1 and not self:willSkipPlayPhase(NextPlayer) then
		if self:isFriend(NextPlayer) then
			NextPlayerCanUse = true
		else
			NextPlayerisEnemy = true
		end
	end

	local cards = {}
	local trickcard = {}
	for _, card_id in ipairs(card_ids) do
		local acard = sgs.Sanguosha:getCard(card_id)
		table.insert(cards, acard)
		if acard:isKindOf("TrickCard") then
			table.insert(trickcard , acard)
		end
	end

	local nextfriend_num = 0
	local aplayer = self.player:getNextAlive()
	for i =1, self.player:aliveCount() do
		if self:isFriend(aplayer) then
			aplayer = aplayer:getNextAlive()
			nextfriend_num = nextfriend_num + 1
		else
			break
		end
	end

	local SelfisCurrent
	if self.room:getCurrent():objectName() == self.player:objectName() then SelfisCurrent = true end

---------------

	local friendneedpeach, peach
	local peachnum, jinknum = 0, 0
	if NextPlayerCanUse then
		if (not self.player:isWounded() and NextPlayer:isWounded()) or
			(self.player:getLostHp() < self:getCardsNum("Peach")) or
			(not SelfisCurrent and self:willSkipPlayPhase() and self.player:getHandcardNum() + 2 > self.player:getMaxCards()) then
			friendneedpeach = true
		end
	end
	for _, card in ipairs(cards) do
		if isCard("Peach", card, self.player) then
			peach = card:getEffectiveId()
			peachnum = peachnum + 1
		end
		if card:isKindOf("Jink") then jinknum = jinknum + 1 end
	end
	if (not friendneedpeach and peach) or peachnum > 1 then return peach end

	local exnihilo, jink, analeptic, nullification, snatch, dismantlement
	for _, card in ipairs(cards) do
		if isCard("ExNihilo", card, self.player) then
			if not NextPlayerCanUse or (not self:willSkipPlayPhase() and (self.player:hasSkills("jizhi|zhiheng|rende") or not NextPlayer:hasSkills("jizhi|zhiheng|rende"))) then
				exnihilo = card:getEffectiveId()
			end
		elseif isCard("Jink", card, self.player) then
			jink = card:getEffectiveId()
		elseif isCard("Analeptic", card, self.player) then
			analeptic = card:getEffectiveId()
		elseif isCard("Nullification", card, self.player) then
			nullification = card:getEffectiveId()
		elseif isCard("Snatch", card, self.player) then
			snatch = card
		elseif isCard("Dismantlement", card, self.player) then
			dismantlement = card
		end

	end

	for _, target in sgs.qlist(self.room:getAlivePlayers()) do
		if self:willSkipPlayPhase(target) or self:willSkipDrawPhase(target) then
			if nullification then return nullification
			elseif self:isFriend(target) and snatch and self:hasTrickEffective(snatch, target, self.player) and
				not self:willSkipPlayPhase() and self.player:distanceTo(target) == 1 then
				return snatch:getEffectiveId()
			elseif self:isFriend(target) and dismantlement and self:hasTrickEffective(dismantlement, target, self.player) and
				not self:willSkipPlayPhase() and self.player:objectName() ~= target:objectName() then
				return dismantlement:getEffectiveId()
			end
		end
	end

	if SelfisCurrent then
		if exnihilo then return exnihilo end
		if (jink or analeptic) and (self:getCardsNum("Jink") == 0 or (self:isWeak() and self:getOverflow() <= 0)) then
			return jink or analeptic
		end
	else
		local CP = self.room:getCurrent()
		local possible_attack = 0
		for _, enemy in ipairs(self.enemies) do
			if enemy:inMyAttackRange(self.player) and self:playerGetRound(CP, enemy) < self:playerGetRound(CP, self.player) then
				possible_attack = possible_attack + 1
			end
		end
		if possible_attack > self:getCardsNum("Jink") and self:getCardsNum("Jink") <= 2 and sgs.getDefenseSlash(self.player) <= 2 then
			if jink or analeptic or exnihilo then return jink or analeptic or exnihilo end
		else
			if exnihilo then return exnihilo end
		end
	end

	if nullification and (self:getCardsNum("Nullification") < 2 or not NextPlayerCanUse) then
		return nullification
	end

	if jinknum == 1 and jink and self:isEnemy(NextPlayer) and (NextPlayer:isKongcheng() or sgs.card_lack[NextPlayer:objectName()]["Jink"] == 1) then
		return jink
	end

	self:sortByUseValue(cards)
	for _, card in ipairs(cards) do
		for _, skill in sgs.qlist(self.player:getVisibleSkillList()) do
			local callback = sgs.ai_cardneed[skill:objectName()]
			if type(callback) == "function" and callback(self.player, card, self) then
				return card:getEffectiveId()
			end
		end
	end

	local eightdiagram, silverlion, vine, renwang, DefHorse, OffHorse
	local weapon, crossbow, halberd, double, qinggang, axe, gudingdao
	for _, card in ipairs(cards) do
		if card:isKindOf("EightDiagram") then eightdiagram = card:getEffectiveId()
		elseif card:isKindOf("SilverLion") then silverlion = card:getEffectiveId()
		elseif card:isKindOf("Vine") then vine = card:getEffectiveId()
		elseif card:isKindOf("RenwangShield") then renwang = card:getEffectiveId()
		elseif card:isKindOf("DefensiveHorse") and not self:getSameEquip(card) then DefHorse = card:getEffectiveId()
		elseif card:isKindOf("OffensiveHorse") and not self:getSameEquip(card) then OffHorse = card:getEffectiveId()
		elseif card:isKindOf("Crossbow") then crossbow = card
		elseif card:isKindOf("DoubleSword") then double = card:getEffectiveId()
		elseif card:isKindOf("QinggangSword") then qinggang = card:getEffectiveId()
		elseif card:isKindOf("Axe") then axe = card:getEffectiveId()
		elseif card:isKindOf("GudingBlade") then gudingdao = card:getEffectiveId()
		elseif card:isKindOf("Halberd") then halberd = card:getEffectiveId()
		elseif card:isKindOf("Weapon") then weapon = card:getEffectiveId() end
	end

	if eightdiagram then
		if not self.player:hasSkill("bazhen") and self.player:hasSkills("tiandu|leiji|hongyan") and not self:getSameEquip(card) then
			return eightdiagram
		end
		if NextPlayerisEnemy and NextPlayer:hasSkills("tiandu|leiji|hongyan") and not self:getSameEquip(card, NextPlayer) then
			return eightdiagram
		end
	end

	if silverlion then
		local lightning, canRetrial
		for _, aplayer in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if aplayer:hasSkill("leiji") and self:isEnemy(aplayer) then
				return silverlion
			end
			if aplayer:containsTrick("lightning") then
				lightning = true
			end
			if aplayer:hasSkills("guicai|guidao") and self:isEnemy(aplayer) then
				canRetrial = true
			end
		end
		if lightning and canRetrial then return silverlion end
		if self.player:isChained() then
			for _, friend in ipairs(self.friends) do
				if friend:hasArmorEffect("Vine") and friend:isChained() then
					return silverlion
				end
			end
		end
		if self.player:isWounded() then return silverlion end
	end

	if vine then
		if sgs.ai_armor_value.Vine(self.player, self) > 0 and self.room:alivePlayerCount() <= 3 then
			return vine
		end
	end

	if renwang then
		if sgs.ai_armor_value.RenwangShield(self.player, self) > 0 and self:getCardsNum("Jink") == 0 then return renwang end
	end

	if DefHorse and (not self.player:hasSkill("leiji") or self:getCardsNum("Jink") == 0) then
		local before_num, after_num = 0, 0
		for _, enemy in ipairs(self.enemies) do
			if enemy:canSlash(self.player, nil, true) then
				before_num = before_num + 1
			end
			if enemy:canSlash(self.player, nil, true, 1) then
				after_num = after_num + 1
			end
		end
		if before_num > after_num and (self:isWeak() or self:getCardsNum("Jink") == 0) then return DefHorse end
	end

	if analeptic then
		local slashs = self:getCards("Slash")
		for _, enemy in ipairs(self.enemies) do
			local hit_num = 0
			for _, slash in ipairs(slashs) do
				if self:slashIsEffective(slash, enemy) and self.player:canSlash(enemy, slash) and self:slashIsAvailable() then
					hit_num = hit_num + 1
					if getCardsNum("Jink", enemy) < 1
						or enemy:isKongcheng()
						or self:canLiegong(enemy, self.player)
						or self.player:hasSkills("tieji|wushuang|qianxi")
						or (self.player:hasWeapon("Axe") or self:getCardsNum("Axe") > 0) and self.player:getCards("he"):length() > 4
						then
						return analeptic
					end
				end
			end
			if (self.player:hasWeapon("Blade") or self:getCardsNum("Blade") > 0) and getCardsNum("Jink", enemy) <= hit_num then return analeptic end
			if self:hasCrossbowEffect(self.player) and hit_num >= 2 then return analeptic end
		end
	end

	if weapon and (self:getCardsNum("Slash") > 0 and self:slashIsAvailable() or not SelfisCurrent) then
		local current_range = (self.player:getWeapon() and sgs.weapon_range[self.player:getWeapon():getClassName()]) or 1
		local nosuit_slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
		local slash = SelfisCurrent and self:getCard("Slash") or nosuit_slash

		self:sort(self.enemies, "defense")

		if crossbow then
			if #self:getCards("Slash") > 1 or self.player:hasSkills("kurou|keji")
				or (self.player:hasSkills("luoshen|guzheng") and not SelfisCurrent and self.room:alivePlayerCount() >= 4) then
				return crossbow:getEffectiveId()
			end
			if self.player:hasSkill("rende") then
				for _, friend in ipairs(self.friends_noself) do
					if getCardsNum("Slash", friend) > 1 then
						return crossbow:getEffectiveId()
					end
				end
			end
			if self:isEnemy(NextPlayer) then
				local CanSave, huanggai, zhenji
				for _, enemy in ipairs(self.enemies) do
					if enemy:hasSkill("jijiu") and getKnownCard(enemy, self.player, "red", nil, "he") > 1 then CanSave = true end
					if enemy:hasSkill("kurou") then huanggai = enemy end
					if enemy:hasSkill("keji") then return crossbow:getEffectiveId() end
					if enemy:hasSkills("luoshen|guzheng") then return crossbow:getEffectiveId() end
				end
				if huanggai then
					if huanggai:getHp() > 2 then return crossbow:getEffectiveId() end
					if CanSave then return crossbow:getEffectiveId() end
				end
				if getCardsNum("Slash", NextPlayer) >= 3 and NextPlayerisEnemy then return crossbow:getEffectiveId() end
			end
		end

		if halberd then
			if self.player:hasSkills("rende") and self:findFriendsByType(sgs.Friend_Draw) then return halberd end
			if SelfisCurrent and self:getCardsNum("Slash") == 1 and self.player:getHandcardNum() == 1 then return halberd end
		end

		if gudingdao then
			local range_fix = current_range - 2
			for _, enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy, slash, true, range_fix) and enemy:isKongcheng() and
					(not SelfisCurrent or (self:getCardsNum("Dismantlement") > 0 or (self:getCardsNum("Snatch") > 0 and self.player:distanceTo(enemy) == 1))) then
					return gudingdao
				end
			end
		end

		if axe then
			local range_fix = current_range - 3
			local FFFslash = self:getCard("FireSlash")
			for _, enemy in ipairs(self.enemies) do
				if enemy:hasArmorEffect("Vine") and FFFslash and self:slashIsEffective(FFFslash, enemy) and
					self.player:getCardCount(true) >= 3 and self.player:canSlash(enemy, FFFslash, true, range_fix) then
					return axe
				elseif self:getCardsNum("Analeptic") > 0 and self.player:getCardCount(true) >= 4 and
					self:slashIsEffective(slash, enemy) and self.player:canSlash(enemy, slash, true, range_fix) then
					return axe
				end
			end
		end

		if double then
			local range_fix = current_range - 2
			for _, enemy in ipairs(self.enemies) do
				if self.player:getGender() ~= enemy:getGender() and self.player:canSlash(enemy, nil, true, range_fix) then
					return double
				end
			end
		end

		if qinggang then
			local range_fix = current_range - 2
			for _, enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy, slash, true, range_fix) and self:slashIsEffective(slash, enemy, self.player, true) then
					return qinggang
				end
			end
		end

	end

	local snatch, dismantlement, indulgence, supplyshortage, collateral, duel, aoe, godsalvation, fireattack, lightning
	local new_enemies = {}
	if #self.enemies > 0 then new_enemies = self.enemies
	else
		for _, aplayer in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if aplayer:getGeneralName() == "neutral" then
				table.insert(new_enemies, aplayer)
			end
		end
	end
	for _, card in ipairs(cards) do
		for _, enemy in ipairs(new_enemies) do
			if card:isKindOf("Snatch") and self:hasTrickEffective(card, enemy, self.player) and self.player:distanceTo(enemy) == 1 and not enemy:isNude() then
				snatch = card:getEffectiveId()
			elseif not enemy:isNude() and card:isKindOf("Dismantlement") and self:hasTrickEffective(card, enemy, self.player) then
				dismantlement = card:getEffectiveId()
			elseif card:isKindOf("Indulgence") and self:hasTrickEffective(card, enemy, self.player) and not enemy:containsTrick("indulgence") then
				indulgence = card:getEffectiveId()
			elseif card:isKindOf("SupplyShortage")	and self:hasTrickEffective(card, enemy, self.player) and not enemy:containsTrick("supply_shortage") then
				supplyshortage = card:getEffectiveId()
			elseif card:isKindOf("Collateral") and self:hasTrickEffective(card, enemy, self.player) and enemy:getWeapon() then
				collateral = card:getEffectiveId()
			elseif card:isKindOf("Duel") and self:hasTrickEffective(card, enemy, self.player) and
					(self:getCardsNum("Slash") >= getCardsNum("Slash", enemy, self.player) or self.player:getHandcardNum() > 4) then
				duel = card:getEffectiveId()
			elseif card:isKindOf("AOE") then
				local dummy_use = {isDummy = true}
				self:useTrickCard(card, dummy_use)
				if dummy_use.card then
					aoe = card:getEffectiveId()
				end
			elseif card:isKindOf("FireAttack") and self:hasTrickEffective(card, enemy, self.player) then
				local FFF
				if enemy:getHp() == 1 or enemy:hasArmorEffect("Vine") then FFF = true end
				if FFF then
					local suits= {}
					local suitnum = 0
					for _, hcard in sgs.qlist(self.player:getHandcards()) do
						if hcard:getSuit() == sgs.Card_Spade then
							suits.spade = true
						elseif hcard:getSuit() == sgs.Card_Heart then
							suits.heart = true
						elseif hcard:getSuit() == sgs.Card_Club then
							suits.club = true
						elseif hcard:getSuit() == sgs.Card_Diamond then
							suits.diamond = true
						end
					end
					for k, hassuit in pairs(suits) do
						if hassuit then suitnum = suitnum + 1 end
					end
					if suitnum >= 3 or (suitnum >= 2 and enemy:getHandcardNum() == 1 ) then
						fireattack = card:getEffectiveId()
					end
				end
			elseif card:isKindOf("GodSalvation") and self:willUseGodSalvation(card) then
				godsalvation = card:getEffectiveId()
			elseif card:isKindOf("Lightning") and self:getFinalRetrial() == 1 then
				lightning = card:getEffectiveId()
			end
		end

		for _, friend in ipairs(self.friends_noself) do
			if (self:hasTrickEffective(card, friend) and (self:willSkipPlayPhase(friend, true) or self:willSkipDrawPhase(friend, true))) or
				self:needToThrowArmor(friend) then
				if isCard("Snatch", card, self.player) and self.player:distanceTo(friend) == 1 then
					snatch = card:getEffectiveId()
				elseif isCard("Dismantlement", card, self.player) then
					dismantlement = card:getEffectiveId()
				end
			end
		end
	end

	if snatch or dismantlement or indulgence or supplyshortage or collateral or duel or aoe or godsalvation or fireattack or lightning then
		if not self:willSkipPlayPhase() or not NextPlayerCanUse then
			return snatch or dismantlement or indulgence or supplyshortage or collateral or duel or aoe or godsalvation or fireattack or lightning
		end
		if #trickcard > nextfriend_num + 1 and NextPlayerCanUse then
			return lightning or fireattack or godsalvation or aoe or duel or collateral or supplyshortage or indulgence or dismantlement or snatch
		end
	end

	if weapon and not self.player:getWeapon() and self:getCardsNum("Slash") > 0 and (self:slashIsAvailable() or not SelfisCurrent) then
		local inAttackRange
		for _, enemy in ipairs(self.enemies) do
			if self.player:inMyAttackRange(enemy) then
				inAttackRange = true
				break
			end
		end
		if not inAttackRange then return weapon end
	end

	self:sortByCardNeed(cards, true)
	for _, card in ipairs(cards) do
		if not card:isKindOf("TrickCard") and not card:isKindOf("Peach") then
			return card:getEffectiveId()
		end
	end

	return cards[1]:getEffectiveId()
end



function SmartAI:useCardAwaitExhausted(AwaitExhausted, use)
	use.card = AwaitExhausted
	return
end
sgs.ai_use_priority.AwaitExhausted = 2.8
sgs.ai_use_value.AwaitExhausted = 5
sgs.ai_keep_value.AwaitExhausted = 1
sgs.ai_card_intention.AwaitExhausted = function(self, card, from, tos)
	for _, to in ipairs(tos) do
		sgs.updateIntention(from, to, -50)
	end
end
sgs.ai_nullification.AwaitExhausted = function(self, card, from, to, positive)
	if positive then
		if self:isEnemy(to) then
			if self:getOverflow() > 0 or self:getCardsNum("Nullification") > 1 then return true end
		end
	else
		if self:isFriend(to) and (self:getOverflow() > 0 or self:getCardsNum("Nullification") > 1) then return true end
	end
	return
end

function SmartAI:useCardBefriendAttacking(BefriendAttacking, use)
	if not BefriendAttacking:isAvailable(self.player) then return end
	local targets = sgs.PlayerList()
	local players = sgs.QList2Table(self.room:getOtherPlayers(self.player))
	self:sort(players)
	for _, to_select in ipairs(players) do
		if self:isFriend(to_select) and BefriendAttacking:targetFilter(targets, to_select, self.player) and not targets:contains(to_select) then
			targets:append(to_select)
			if use.to then use.to:append(to_select) end
		end
	end
	if targets:isEmpty() then
		for _, to_select in ipairs(players) do
			if BefriendAttacking:targetFilter(targets, to_select, self.player) and not targets:contains(to_select) then
				targets:append(to_select)
				if use.to then use.to:append(to_select) end
			end
		end
	end
	if not targets:isEmpty() then
		use.card = BefriendAttacking
		return
	end
end
sgs.ai_use_priority.BefriendAttacking = 9.28
sgs.ai_use_value.BefriendAttacking = 9
sgs.ai_keep_value.BefriendAttacking = 3.88

sgs.ai_nullification.BefriendAttacking = function(self, card, from, to, positive)
	if positive then
		if not self:isFriend(to) and self:isEnemy(from) and self:isWeak(from) then return true end
	else
		if self:isFriend(from) then return true end
	end
	return
end

function SmartAI:useCardKnownBoth(KnownBoth, use)
	self.knownboth_choice = {}
	local targets = sgs.PlayerList()
	local total_num = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, KnownBoth)
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if KnownBoth:targetFilter(targets, player, self.player) and sgs.isAnjiang(player) and not targets:contains(player)
			and player:getMark(("KnownBoth_%s_%s"):format(self.player:objectName(), player:objectName())) == 0 then
			use.card = KnownBoth
			targets:append(player)
			if use.to then use.to:append(player) end
			self.knownboth_choice[player:objectName()] = "head_general"
		end
	end

	if total_num > targets:length() then
		self:sort(self.enemies, "handcard")
		sgs.reverse(self.enemies)
		for _, enemy in ipairs(self.enemies) do
			if KnownBoth:targetFilter(targets, enemy, self.player) and enemy:getHandcardNum() - self:getKnownNum(enemy) > 3 and not targets:contains(enemy) then
				use.card = KnownBoth
				targets:append(enemy)
				if use.to then use.to:append(enemy) end
				self.knownboth_choice[enemy:objectName()] = "handcards"
			end
		end
	end
	use.card = KnownBoth
	if use.to then use.to = sgs.SPlayerList() end
end
sgs.ai_skill_choice.known_both = function(self, choices, data)
	local target = data:toPlayer()
	if target and self.knownboth_choice and self.knownboth_choice[target:objectName()] then return self.knownboth_choice[target:objectName()] end
	return "handcards"
end
sgs.ai_use_priority.KnownBoth = 9.1
sgs.ai_use_value.KnownBoth = 5.5
sgs.ai_keep_value.KnownBoth = 3.33
sgs.ai_nullification.KnownBoth = function(self, card, from, to, positive)
end

sgs.ai_choicemade_filter.skillChoice.known_both = function(self, from, promptlist)
	local choice = promptlist[#promptlist]
	if choice ~= "handcards" then
		for _, to in sgs.qlist(self.room:getOtherPlayers(from)) do
			if to:hasFlag("KnownBothTarget") then
				to:setMark(("KnownBoth_%s_%s"):format(from:objectName(), to:objectName()), 1)
				break
			end
		end
	end
end



sgs.ai_skill_use["@@Triblade"] = function(self, prompt)
	local damage = self.room:getTag("CurrentDamageStruct"):toDamage()
	if type(damage) ~= "userdata" then return "." end
	local targets = sgs.SPlayerList()
	for _, p in sgs.qlist(self.room:getOtherPlayers(damage.to)) do
		if damage.to:distanceTo(p) == 1 then targets:append(p) end
	end
	if targets:isEmpty() then return "." end
	local id
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if not self.player:isCardLimited(c, sgs.Card_MethodDiscard) then id = c:getEffectiveId() break end
	end
	if not id then return "." end
	for _, target in sgs.qlist(targets) do
		if self:isEnemy(target) and self:damageIsEffective(target, nil, self.player) and not self:getDamagedEffects(target, self.player)
			and not self:needToLoseHp(target, self.player) then
			return "@TribladeSkillCard=" .. id .. "&tribladeskill->" .. target:objectName()
		end
	end
	for _, target in sgs.qlist(targets) do
		if self:isFriend(target) and self:damageIsEffective(target, nil, self.player)
			and (self:getDamagedEffects(target, self.player) or self:needToLoseHp(target, self.player, nil, true)) then
			return "@TribladeSkillCard=" .. id .. "&tribladeskill->" .. target:objectName()
		end
	end
	return "."
end
function sgs.ai_slash_weaponfilter.Triblade(self, to, player)
	if player:distanceTo(to) > math.max(sgs.weapon_range.Triblade, player:getAttackRange()) then return end
	return sgs.card_lack[to:objectName()]["Jink"] == 1 or getCardsNum("Jink", to, self.player) == 0
end
function sgs.ai_weapon_value.Triblade(self, enemy, player)
	if not enemy then return 1 end
	if enemy and player:getHandcardNum() > 2 then return math.min(3.8, player:getHandcardNum() - 1) end
end

sgs.ai_use_priority.Triblade = 2.673

