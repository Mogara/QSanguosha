function SmartAI:slashProhibit(card,enemy)
	if card == nil then
		card = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	end

	if self:isWeak() and self:hasSkills("enyuan|ganglie", enemy) then return true end
	if self:isFriend(enemy) then
		if card:inherits("FireSlash") or self.player:hasWeapon("fan") then
			if self:isEquip("Vine", enemy) then return true end
		end
		if enemy:isChained() and card:inherits("NatureSlash") and #(self:getChainedFriends())>1 and
			self:slashIsEffective(card,enemy) then return true end
		if self:getCardsNum("Jink",enemy) == 0 and enemy:getHp() < 2 and self:slashIsEffective(card,enemy) then return true end
		if enemy:isLord() and self:isWeak(enemy) and self:slashIsEffective(card,enemy) then return true end
		if self:hasSkills("duanchang|huilei|dushi", enemy) and self:isWeak(enemy) then return true end
		if self:isEquip("GudingBlade") and enemy:isKongcheng() then return true end
	else
		if enemy:hasSkill("liuli") then
			if enemy:getHandcardNum() < 1 then return false end
			for _, friend in ipairs(self.friends_noself) do
				if enemy:canSlash(friend,true) and self:slashIsEffective(card, friend) then return true end
			end
		end

		if enemy:hasSkill("leiji") then
			local hcard = enemy:getHandcardNum()
			if self.player:hasSkill("tieji") or
				(self.player:hasSkill("liegong") and (hcard>=self.player:getHp() or hcard<=self.player:getAttackRange())) then return false end

			if enemy:getHandcardNum() >= 2 then return true end
			if self:isEquip("EightDiagram", enemy) then
				local equips = enemy:getEquips()
				for _,equip in sgs.qlist(equips) do
					if equip:getSuitString() == "spade" then return true end
				end
			end
		end

		if enemy:hasSkill("tiandu") then
			if self:isEquip("EightDiagram", enemy) then return true end
		end

		if enemy:hasLordSkill("hujia") then
			for _, player in ipairs(self:getFriends(enemy)) do
				if player:hasSkill("tiandu") and self:isEquip("EightDiagram", player) and player:getKingdom() == "wei" then return true end
			end
		end

		if enemy:hasSkill("ganglie") then
			if self.player:getHandcardNum()+self.player:getHp() < 5 then return true end
		end

		if enemy:hasSkill("shenjun") and (enemy:getGeneral():isMale()~= self.player:getGeneral():isMale()) and not card:inherits("ThunderSlash") then
			return true
		end

		if enemy:hasSkill("xiangle") and self:getCardsNum("Slash")+self:getCardsNum("Analpetic")+math.max(self:getCardsNum("Jink")-1,0) < 2 then
			return true
		end

		if enemy:isChained() and #(self:getChainedFriends()) > #(self:getChainedEnemies()) and self:slashIsEffective(card,enemy) then
			return true
		end

		if enemy:hasSkill("wuhun") and self:isWeak(enemy) and not (enemy:isLord() and self.player:getRole() == "rebel") then
			local mark = 0
			local marks = {}
			for _, player in sgs.qlist(self.room:getAlivePlayers()) do
				local mymark = player:getMark("@nightmare")
				if player:objectName() == self.player:objectName() then
					mymark = mymark + 1
					if self.player:hasFlag("drank") then mymark = mymark + 1 end
				end
				if mymark > mark then mark = mymark end
				marks[player:objectName()] = mymark
			end
			if mark > 0 then
				for _,friend in ipairs(self.friends) do
					if marks[friend:objectName()] == mark and (not self:isWeak(friend) or friend:isLord()) and
						not (#self.enemies==1 and #self.friends + #self.enemies == self.room:alivePlayerCount()) then return true end
				end
				if self.player:getRole()~="rebel" and marks[self.room:getLord():objectName()] == mark and
					not (#self.enemies==1 and #self.friends + #self.enemies == self.room:alivePlayerCount()) then return true end
			end
		end

		if enemy:hasSkill("duanchang") and #self.enemies>1 and self:isWeak(enemy) and (self.player:isLord() or not self:isWeak()) then
			return true
		end

		if enemy:hasSkill("huilei") and #self.enemies>1 and self:isWeak(enemy) and
			(self.player:getHandcardNum()>3 or self:getCardsNum("Shit")>0) then
			return true
		end

		if enemy:hasSkill("dushi") and self.player:isLord() and self:isWeak(enemy) then return true end
	end

	return not self:slashIsEffective(card, enemy)
end

function SmartAI:slashIsEffective(slash, to)
	if to:hasSkill("yizhong") and not to:getArmor() then
		if slash:isBlack() then
			return false
		end
	end

	local nature = {
		Slash = sgs.DamageStruct_Normal,
		FireSlash = sgs.DamageStruct_Fire,
		ThunderSlash = sgs.DamageStruct_Thunder,
	}

	if not self:damageIsEffective(to, nature[slash:className()]) then return false end

	if self.player:hasWeapon("qinggang_sword") or (self.player:hasFlag("xianzhen_success") and self.room:getTag("XianzhenTarget"):toPlayer() == to) then
		return true
	end

	local armor = to:getArmor()
	if armor then
		if armor:objectName() == "renwang_shield" then
			return not slash:isBlack()
		elseif armor:objectName() == "vine" then
			return slash:inherits("NatureSlash") or self.player:hasWeapon("fan")
		end
	end

	return true
end

function SmartAI:slashIsAvailable(player)
	player = player or self.player
	if player:hasFlag("tianyi_failed") or player:hasFlag("xianzhen_failed") then return false end

	if player:hasWeapon("crossbow") or player:hasSkill("paoxiao") then
		return true
	end

	if player:hasFlag("tianyi_success") then
		return (player:usedTimes("Slash") + player:usedTimes("FireSlash") + player:usedTimes("ThunderSlash")) < 2
	else
		return (player:usedTimes("Slash") + player:usedTimes("FireSlash") + player:usedTimes("ThunderSlash")) < 1
	end
end

function SmartAI:useCardSlash(card, use)
	if not self:slashIsAvailable() then return end
	local no_distance = self.slash_distance_limit
	if card:getSkillName() == "wushen" then no_distance = true end
	if (self.player:getHandcardNum() == 1
	and self.player:getHandcards():first():inherits("Slash")
	and self.player:getWeapon()
	and self.player:getWeapon():inherits("Halberd"))
	or (self.player:hasSkill("shenji") and not self.player:getWeapon()) then
		self.slash_targets = 3
	end

	self.predictedRange = self.player:getAttackRange()
	local target_count = 0
	if self.player:hasSkill("qingnang") and self:isWeak() and self:getOverflow() == 0 then return end
	for _, friend in ipairs(self.friends_noself) do
		local slash_prohibit = false
		slash_prohibit = self:slashProhibit(card,friend)
		if (self.player:hasSkill("pojun") and friend:getHp() > 4 and self:getCardsNum("Jink", friend) == 0
			and friend:getHandcardNum() < 3)
		or (friend:hasSkill("leiji") 
		and (self:getCardsNum("Jink", friend) > 0 or (not self:isWeak(friend) and self:isEquip("EightDiagram",friend)))
		and (hasExplicitRebel(self.room) or not friend:isLord()))
		or (friend:isLord() and self.player:hasSkill("guagu") and friend:getLostHp() >= 1 and self:getCardsNum("Jink", friend) == 0)
		then
			if not slash_prohibit then
				if ((self.player:canSlash(friend, not no_distance)) or
					(use.isDummy and (self.player:distanceTo(friend) <= self.predictedRange))) and
					self:slashIsEffective(card, friend) then
					use.card = card
					if use.to then
						use.to:append(friend)
						self:speak("hostile", self.player:getGeneral():isFemale())
					end
					target_count = target_count+1
					if self.slash_targets <= target_count then return end
				end
			end
--				break
		end
	end

	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		local slash_prohibit = false
		slash_prohibit = self:slashProhibit(card,enemy)
		if not slash_prohibit then
			if (self.player:canSlash(enemy, not no_distance) or
			(use.isDummy and self.predictedRange and (self.player:distanceTo(enemy) <= self.predictedRange))) and
			self:objectiveLevel(enemy) > 3 and
			self:slashIsEffective(card, enemy) and
			not (not self:isWeak(enemy) and #self.enemies > 1 and #self.friends > 1 and self.player:hasSkill("keji")
				and self:getOverflow() > 0 and not self:isEquip("Crossbow")) then
				-- fill the card use struct
				local usecard = card
				if not use.to or use.to:isEmpty() then
					local anal = self:searchForAnaleptic(use,enemy,card)
					if anal and not self:isEquip("SilverLion", enemy) and not self:isWeak() then
						use.card = anal
						return
					end
					if self.player:getGender()~=enemy:getGender() and self:getCardsNum("DoubleSword",self.player,"h") > 0 then
						self:useEquipCard(self:getCard("DoubleSword"), use)
						if use.card then return end
					end
					if enemy:isKongcheng() and self:getCardsNum("GudingBlade", self.player, "h") > 0 then
						self:useEquipCard(self:getCard("GudingBlade"), use)
						if use.card then return end
					end
					if self:getOverflow()>0 and self:getCardsNum("Axe", self.player, "h") > 0 then
						self:useEquipCard(self:getCard("Axe"), use)
						if use.card then return end
					end
					if enemy:getArmor() and self:getCardsNum("Fan", self.player, "h") > 0 and
						(enemy:getArmor():inherits("Vine") or enemy:getArmor():inherits("GaleShell")) then
						self:useEquipCard(self:getCard("Fan"), use)
						if use.card then return end
					end
					if enemy:getDefensiveHorse() and self:getCardsNum("KylinBow", self.player, "h") > 0 then
						self:useEquipCard(self:getCard("KylinBow") ,use)
						if use.card then return end
					end
					if enemy:isChained() and #(self:getChainedFriends()) < #(self:getChainedEnemies()) and not use.card then
						if self:isEquip("Crossbow") and card:inherits("NatureSlash") then
							local slashes = self:getCards("Slash")
							for _, slash in ipairs(slashes) do
								if not slash:inherits("NatureSlash") and self:slashIsEffective(slash, enemy)
									and not self:slashProhibit(slash, enemy) then
									usecard = slash
									break
								end
							end
						elseif not card:inherits("NatureSlash") then
							local slash = self:getCard("NatureSlash")
							if slash and self:slashIsEffective(slash, enemy) and not self:slashProhibit(slash, enemy) then usecard = slash end
						end
					end
				end
				use.card = use.card or usecard
				if use.to then use.to:append(enemy) end
				target_count = target_count+1
				if self.slash_targets <= target_count then return end
			end
		end
	end

	for _, friend in ipairs(self.friends_noself) do
		if friend:hasSkill("yiji") and friend:getLostHp() < 1 and
			not (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
			local slash_prohibit = false
			slash_prohibit = self:slashProhibit(card, friend)
			if not slash_prohibit then
				if ((self.player:canSlash(friend, not no_distance)) or
					(use.isDummy and (self.player:distanceTo(friend) <= self.predictedRange))) and
					self:slashIsEffective(card, friend) then
					use.card = card
					if use.to then
						use.to:append(friend)
						self:speak("yiji")
					end
					target_count = target_count+1
					if self.slash_targets <= target_count then return end
				end
			end
			break
		end
	end
end

sgs.ai_skill_use["slash"] = function(self, prompt)
	if prompt ~= "@askforslash" and prompt ~= "@moon-spear-slash" then return "." end
	local slash = self:getCard("Slash")
	if not slash then return "." end
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy, true) and not self:slashProhibit(slash, enemy) and self:slashIsEffective(slash, enemy) then
			return ("%s->%s"):format(slash:toString(), enemy:objectName())
		end
	end
	return "."
end

sgs.ai_skill_playerchosen.zero_card_as_slash = function(self, targets)
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	local targetlist=sgs.QList2Table(targets)
	self:sort(targetlist, "defense")
	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) and not self:slashProhibit(slash ,target) and self:slashIsEffective(slash,target) then
			return target
		end
	end
	for i=#targetlist, 1, -1 do
		if not self:slashProhibit(slash, targetlist[i]) then
			return targetlist[i]
		end
	end
	return targets:first()
end

function SmartAI:useCardPeach(card, use)
	if not self.player:isWounded() then return end
	if self.player:hasSkill("longhun") and not self.player:isLord() and
	math.min(self.player:getMaxCards(), self.player:getHandcardNum()) + self.player:getCards("e"):length() > 3 then return end
	if not (self.player:hasSkill("rende") and self:getOverflow() > 1 and #self.friends_noself > 0) then
		local peaches = 0
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		for _,card in ipairs(cards) do
			if card:inherits("Peach") then peaches = peaches+1 end
		end

		for _, friend in ipairs(self.friends_noself) do
			if (self.player:getHp()-friend:getHp() > peaches) and (friend:getHp() < 3) and not friend:hasSkill("buqu") then return end
		end

		if self.player:hasSkill("jieyin") and self:getOverflow() > 0 then
			self:sort(self.friends, "hp")
			for _, friend in ipairs(self.friends) do
				if friend:isWounded() and friend:getGeneral():isMale() then return end
			end
		end

		use.card = card
	end
end

sgs.weapon_range.Crossbow = 1
sgs.weapon_range.DoubleSword = 2
sgs.weapon_range.QinggangSword = 2
sgs.weapon_range.IceSword = 2
sgs.weapon_range.GudingBlade = 2
sgs.weapon_range.Blade = 3
sgs.weapon_range.Spear = 3
sgs.weapon_range.Halberd = 4
sgs.weapon_range.KylinBow = 5

sgs.ai_skill_invoke.double_sword = true

sgs.ai_skill_invoke.ice_sword=function(self, data)
	if self.player:hasFlag("drank") then return false end
	local effect = data:toSlashEffect() 
	local target = effect.to
	if self:isFriend(target) then
		if self:isWeak(target) then return true
		elseif target:getLostHp()<1 then return false end
		return true
	else
		if self:isWeak(target) then return false end
		if target:getArmor() and self:evaluateArmor(target:getArmor(), target)>3 then return true end
		local num = target:getHandcardNum()
		if self.player:hasSkill("tieji") or (self.player:hasSkill("liegong")
			and (num >= self.player:getHp() or num <= self.player:getAttackRange())) then return false end
		if target:hasSkill("tuntian") then return false end
		if self:hasSkills(sgs.need_kongcheng, target) then return false end
		if target:getCards("he"):length()<4 and target:getCards("he"):length()>1 then return true end
		return false
	end
end

local spear_skill={}
spear_skill.name="spear"
table.insert(sgs.ai_skills,spear_skill)
spear_skill.getTurnUseCard=function(self,inclusive)
	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)

	if #cards<(self.player:getHp()+1) then return nil end
	if #cards<2 then return nil end
	if self:getCard("Slash") then return nil end

	self:sortByUseValue(cards,true)

	local suit1 = cards[1]:getSuitString()
	local card_id1 = cards[1]:getEffectiveId()
	
	local suit2 = cards[2]:getSuitString()
	local card_id2 = cards[2]:getEffectiveId()

	local suit="no_suit"
	if cards[1]:isBlack() == cards[2]:isBlack() then suit = suit1 end

	local card_str = ("slash:spear[%s:%s]=%d+%d"):format(suit, 0, card_id1, card_id2)

	local slash = sgs.Card_Parse(card_str)

	return slash	
end

sgs.ai_skill_invoke.kylin_bow = function(self, data)
	local effect = data:toSlashEffect()

	if self:hasSkills(sgs.lose_equip_skill, effect.to) then
		return self:isFriend(effect.to)
	end

	return self:isEnemy(effect.to)
end

sgs.ai_skill_invoke.eight_diagram = function(self, data)
	if sgs.hujiasource and not self:isFriend(sgs.hujiasource) then return false end
	if sgs.lianlisource and not self:isFriend(sgs.lianlisource) then return false end
	if self.player:hasSkill("tiandu") then return true end
	for _, enemy in ipairs(self.enemies) do
		if enemy:hasSkill("guidao") then
			if not enemy:getCards("e"):isEmpty() then
				for _, card in sgs.qlist(enemy:getCards("e")) do
					if card:isBlack() then return false end
				end
			end
			if enemy:getHandcardNum() > 1 then return false end
		end
	end
	
	if self:getDamagedEffects(self) then return false end
	return true
end

function SmartAI:useCardAmazingGrace(card, use)
	if #self.friends >= #self.enemies or (self:hasSkills(sgs.need_kongcheng) and self.player:getHandcardNum() == 1)
		or self.player:hasSkill("jizhi") then
		use.card = card
	elseif self.player:hasSkill("wuyan") then
		use.card = card
	end
end

function SmartAI:useCardGodSalvation(card, use)
	local good, bad = 0, 0

	if self.player:hasSkill("wuyan") and self.player:isWounded() then
		use.card = card
		return
	end

	for _, friend in ipairs(self.friends) do
		if friend:isWounded() then
			good = good + 10/(friend:getHp())
			if friend:isLord() then good = good + 10/(friend:getHp()) end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if enemy:isWounded() then
			bad = bad + 10/(enemy:getHp())
			if enemy:isLord() then
				bad = bad + 10/(enemy:getHp())
			end
		end
	end

	if good > bad then
		use.card = card
	end
end

local function factorial(n)
	if n <= 0.1 then return 1 end
	return n*factorial(n-1)
end

function SmartAI:useCardDuel(duel, use)
	if self.player:hasSkill("wuyan") then return end
	self:sort(self.enemies,"handcard")
	local enemies = self:exclude(self.enemies, duel)
	for _, enemy in ipairs(enemies) do
		if self:objectiveLevel(enemy) > 3 then
			local n1 = self:getCardsNum("Slash")
			local n2 = enemy:getHandcardNum()
			if enemy:hasSkill("wushuang") then n2 = n2*2 end
			if self.player:hasSkill("wushuang") then n1 = n1*2 end
			local useduel
			if self:hasTrickEffective(duel, enemy) then
				if n1 >= n2 then
					useduel = true
				elseif n2 > n1*2 + 1 then
					useduel = false
				elseif n1 > 0 then
					local percard = 0.35
					if enemy:hasSkill("paoxiao") or enemy:hasWeapon("crossbow") then percard = 0.2 end
					local poss = percard ^ n1 * (factorial(n1)/factorial(n2)/factorial(n1-n2))
					if math.random() > poss then useduel = true end
				end
				if useduel then
					use.card = duel
					if use.to then
						use.to:append(enemy)
						self:speak("duel", self.player:getGeneral():isFemale())
					end
					return
				end
			end
		end
	end
end

function SmartAI:useCardExNihilo(card, use)
	use.card = card
	if not use.isDummy then
		self:speak("lucky")
	end
end

function SmartAI:useCardSnatch(snatch, use)
	if self.player:hasSkill("wuyan") then return end

	if (not self.has_wizard) and self:hasWizard(self.enemies)  then
		-- find lightning
		local players = self.room:getOtherPlayers(self.player)
		players = self:exclude(players, snatch)
		for _, player in ipairs(players) do
			if player:containsTrick("lightning") and not player:hasSkill("wuyan") then
				use.card = snatch
				if use.to then use.to:append(player) end

				return
			end
		end
	end

	self:sort(self.friends_noself,"defense")
	local friends = self:exclude(self.friends_noself, snatch)
	local hasLion, target
	for _, friend in ipairs(friends) do
		if self:hasTrickEffective(snatch, friend) then
			if (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
				use.card = snatch
				if use.to then use.to:append(friend) end
				return
			end
			if self:isEquip("SilverLion", friend) and friend:isWounded() and (friend:hasSkill("benghuai") or friend:getHp() < 4) then
				hasLion = true
				target = friend
			end
		end
	end

	if hasLion then
		use.card = snatch
		if use.to then use.to:append(target) end
		return
	end

	self:sort(self.enemies,"defense")
	if sgs.getDefense(self.enemies[1]) >= 8 then self:sort(self.enemies, "threat") end

	local enemies = self:exclude(self.enemies, snatch)
	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(snatch, enemy) and
			not (self:hasSkills(sgs.lose_equip_skill, enemy) and enemy:getHandcardNum() == 0) and
			not (enemy:getCards("he"):length() == 1 and self:isEquip("GaleShell",enemy)) then
			if enemy:getHandcardNum() == 1 then
				if enemy:hasSkill("kongcheng") or enemy:hasSkill("lianying") then return end
			end
			use.card = snatch
			if use.to then
				use.to:append(enemy)
				self:speak("hostile", self.player:getGeneral():isFemale())
			end
			return
		end
	end
end

function SmartAI:useCardDismantlement(dismantlement, use)
	if self.player:hasSkill("wuyan") then return end
	if (not self.has_wizard) and self:hasWizard(self.enemies) then
		-- find lightning
		local players = self.room:getOtherPlayers(self.player)
		players = self:exclude(players, dismantlement)
		for _, player in ipairs(players) do
			if player:containsTrick("lightning") and not player:hasSkill("wuyan") then
				use.card = dismantlement
				if use.to then use.to:append(player) end
				return
			end
		end
	end

	self:sort(self.friends_noself,"defense")
	local friends = self:exclude(self.friends_noself, dismantlement)
	local hasLion, target
	for _, friend in ipairs(friends) do
		if self:hasTrickEffective(dismantlement, friend) then
			if (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
				use.card = dismantlement
				if use.to then use.to:append(friend) end
				return
			end
			if self:isEquip("SilverLion", friend) and friend:isWounded() and (friend:hasSkill("benghuai") or friend:getHp() < 4) then
				hasLion = true
				target = friend
			end
		end
	end

	if hasLion then
		use.card = dismantlement
		if use.to then use.to:append(target) end
		return
	end

	self:sort(self.enemies,"defense")
	if sgs.getDefense(self.enemies[1]) >= 8 then self:sort(self.enemies, "threat") end

	local enemies = self:exclude(self.enemies, dismantlement)
	for _, enemy in ipairs(enemies) do
		local equips = enemy:getEquips()
		if not enemy:isNude() and self:hasTrickEffective(dismantlement, enemy) and not enemy:hasSkill("tuntian") and
			not (self:hasSkills(sgs.lose_equip_skill, enemy) and enemy:getHandcardNum() == 0) and
			not (enemy:getCards("he"):length() == 1 and self:isEquip("GaleShell",enemy)) 
			and self:hasSkills("guidao|guicai|lijian|fanjian|qingnang|longhun", enemy) then
			if enemy:getHandcardNum() == 1 then
				if enemy:hasSkill("kongcheng") or enemy:hasSkill("lianying") then return end
			end
			use.card = dismantlement
			if use.to then
				use.to:append(enemy)
				self:speak("hostile", self.player:getGeneral():isFemale())
			end
			return
		end
	end	
	for _, enemy in ipairs(enemies) do
		local equips = enemy:getEquips()
		if not enemy:isNude() and self:hasTrickEffective(dismantlement, enemy) and not enemy:hasSkill("tuntian") and
			not (self:hasSkills(sgs.lose_equip_skill, enemy) and enemy:getHandcardNum() == 0) and
			not (enemy:getCards("he"):length() == 1 and self:isEquip("GaleShell",enemy)) then
			if enemy:getHandcardNum() == 1 then
				if enemy:hasSkill("kongcheng") or enemy:hasSkill("lianying") then return end
			end
			use.card = dismantlement
			if use.to then
				use.to:append(enemy)
				self:speak("hostile", self.player:getGeneral():isFemale())
			end
			return
		end
	end
end

function SmartAI:useCardCollateral(card, use)
	if self.player:hasSkill("wuyan") then return end
	self:sort(self.enemies,"threat")

	for _, friend in ipairs(self.friends_noself) do
		if friend:getWeapon() and self:hasSkills(sgs.lose_equip_skill, friend) then

			for _, enemy in ipairs(self.enemies) do
				if friend:canSlash(enemy) then
					use.card = card
				end
				if use.to then use.to:append(friend) end
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end

	local n = nil
	local final_enemy = nil
	for _, enemy in ipairs(self.enemies) do
		if not self.room:isProhibited(self.player, enemy, card)
			and self:hasTrickEffective(card, enemy)
			and not self:hasSkill(sgs.lose_equip_skill, enemy)
			and not enemy:hasSkill("weimu")
			and enemy:getWeapon() then

			for _, enemy2 in ipairs(self.enemies) do
				if enemy:canSlash(enemy2) then
					if enemy:getHandcardNum() == 0 then
						use.card = card
						if use.to then use.to:append(enemy) end
						if use.to then use.to:append(enemy2) end
						return
					else
						n = 1;
						final_enemy = enemy2
					end
				end
			end
			if n then use.card = card end
			if use.to then use.to:append(enemy) end
			if use.to then use.to:append(final_enemy) end
			return

		end
		n = nil
	end
end

local function hp_subtract_handcard(a,b)
	local diff1 = a:getHp() - a:getHandcardNum()
	local diff2 = b:getHp() - b:getHandcardNum()

	return diff1 < diff2
end

function SmartAI:useCardIndulgence(card, use)
	table.sort(self.enemies, hp_subtract_handcard)
	
	local enemies = self:exclude(self.enemies, card)
	for _, enemy in ipairs(enemies) do
		if self:hasSkills("lijian|fanjian") and not enemy:containsTrick("indulgence") and not enemy:isKongcheng() and enemy:faceUp() then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
	
	for _, enemy in ipairs(enemies) do
		if not enemy:containsTrick("indulgence") and not enemy:hasSkill("keji") and enemy:faceUp() then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

function SmartAI:useCardLightning(card, use)
	if self.player:containsTrick("lightning") then return end
	if self.player:hasSkill("weimu") and card:isBlack() then return end

	if not self:hasWizard(self.enemies) then--and self.room:isProhibited(self.player, self.player, card) then
		if self:hasWizard(self.friends) then
			use.card = card
			return
		end
		local players = self.room:getAllPlayers()
		players = sgs.QList2Table(players)

		local friends = 0
		local enemies = 0

		for _,player in ipairs(players) do
			if self:objectiveLevel(player) >= 4 then
				enemies = enemies + 1
			elseif self:isFriend(player) then
				friends = friends + 1
			end
		end

		local ratio

		if friends == 0 then ratio = 999
		else ratio = enemies/friends
		end

		if ratio > 1.5 then
			use.card = card
			return
		end
	end
end
