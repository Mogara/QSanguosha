local function hasExplicitRebel(room)
	for _, player in sgs.qlist(room:getAllPlayers()) do
		if sgs.isRolePredictable() and  sgs.evaluatePlayerRole(player) == "rebel" then return true end
		if sgs.compareRoleEvaluation(player, "rebel", "loyalist") == "rebel" then return true end
	end
	return false
end

function SmartAI:slashProhibit(card,enemy)
	card = card or sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	for _, askill in sgs.qlist(enemy:getVisibleSkillList()) do
		local filter = sgs.ai_slash_prohibit[askill:objectName()]
		if filter and type(filter) == "function" and filter(self, enemy, card) then return true end
	end

	if self:isFriend(enemy) then
		if card:inherits("FireSlash") or self.player:hasWeapon("fan") or self.player:hasSkill("zonghuo") then
			if self:isEquip("Vine", enemy) and not (enemy:isChained() and self:isGoodChainTarget(enemy)) then return true end
		end
		if enemy:isChained() and (card:inherits("NatureSlash") or self.player:hasSkill("zonghuo")) and not self:isGoodChainTarget(enemy) and
			self:slashIsEffective(card,enemy) then return true end
		if self:getCardsNum("Jink",enemy) == 0 and enemy:getHp() < 2 and self:slashIsEffective(card,enemy) then return true end
		if enemy:isLord() and self:isWeak(enemy) and self:slashIsEffective(card,enemy) then return true end
		if self:isEquip("GudingBlade") and enemy:isKongcheng() then return true end
	else
		if enemy:isChained() and not self:isGoodChainTarget(enemy) and self:slashIsEffective(card,enemy) 
			and (card:inherits("NatureSlash") or self.player:hasSkill("zonghuo")) then
			return true
		end
	end

	return not self:slashIsEffective(card, enemy)
end

function SmartAI:canLiuli(other, another)
	if not other:hasSkill("liuli") then return false end
	local n = other:getHandcardNum()
	if n > 0 and (other:distanceTo(another) <= other:getAttackRange()) then return true
	elseif other:getWeapon() and other:getOffensiveHorse() and (other:distanceTo(another) <= other:getAttackRange()) then return true
	elseif other:getWeapon() or other:getOffensiveHorse() then return other:distanceTo(another) <= 1
	else return false end
end

function SmartAI:slashIsEffective(slash, to)
    if to:hasSkill("zuixiang") and to:isLocked(slash) then return false end
	if to:hasSkill("yizhong") and not to:getArmor() then
		if slash:isBlack() then
			return false
		end
	end

	local natures = {
		Slash = sgs.DamageStruct_Normal,
		FireSlash = sgs.DamageStruct_Fire,
		ThunderSlash = sgs.DamageStruct_Thunder,
	}

	local nature = natures[slash:className()]
	if self.player:hasSkill("zonghuo") then nature = sgs.DamageStruct_Fire end
	if not self:damageIsEffective(to, nature) then return false end

	if self.player:hasWeapon("qinggang_sword") or (self.player:hasFlag("xianzhen_success") and self.room:getTag("XianzhenTarget"):toPlayer() == to) then
		return true
	end

	local armor = to:getArmor()
	if armor then
		if armor:objectName() == "renwang_shield" then
			return not slash:isBlack()
		elseif armor:objectName() == "vine" then
			return nature ~= sgs.DamageStruct_Normal or self.player:hasWeapon("fan")
		end
	end

	return true
end

function SmartAI:slashIsAvailable(player)
	player = player or self.player
	local slash = self:getCard("Slash", player) or sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	assert(slash)
	return slash:isAvailable(player)
end

function SmartAI:useCardSlash(card, use)
	if not self:slashIsAvailable() then return end
	local basicnum = 0
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	for _, acard in ipairs(cards) do
		if acard:getTypeId() == sgs.Card_Basic and not acard:inherits("Peach") then basicnum = basicnum + 1 end
	end
	local no_distance = self.slash_distance_limit
	if card:getSkillName() == "wushen" then no_distance = true end
	if card:getSkillName() == "gongqi" then no_distance = true end
	if self.player:hasSkill("lihuo") and card:inherits("FireSlash") then self.slash_targets = 2 end
	if (self.player:getHandcardNum() == 1
	and self.player:getHandcards():first():inherits("Slash")
	and self.player:getWeapon()
	and self.player:getWeapon():inherits("Halberd"))
	or (self.player:hasSkill("shenji") and not self.player:getWeapon()) then
		self.slash_targets = 3
	end

	self.predictedRange = self.player:getAttackRange()
	if self.player:hasSkill("qingnang") and self:isWeak() and self:getOverflow() == 0 then return end
	local huatuo = self.room:findPlayerBySkillName("jijiu")
	for _, friend in ipairs(self.friends_noself) do
		local slash_prohibit = false
		slash_prohibit = self:slashProhibit(card,friend)
		if (self.player:hasSkill("pojun") and friend:getHp() > 4 and self:getCardsNum("Jink", friend) == 0
			and friend:getHandcardNum() < 3)
		or (friend:hasSkill("leiji") 
		and (self:getCardsNum("Jink", friend) > 0 or (not self:isWeak(friend) and self:isEquip("EightDiagram",friend)))
		and (hasExplicitRebel(self.room) or not friend:isLord()))
		or (friend:isLord() and self.player:hasSkill("guagu") and friend:getLostHp() >= 1 and self:getCardsNum("Jink", friend) == 0)
		or (friend:hasSkill("jieming") and self.player:hasSkill("rende") and (huatuo and self:isFriend(huatuo)))
		then
			if not slash_prohibit then
				if ((self.player:canSlash(friend, not no_distance)) or
					(use.isDummy and (self.player:distanceTo(friend) <= self.predictedRange))) and
					self:slashIsEffective(card, friend) then
					use.card = card
					if use.to then
						use.to:append(friend)
						self:speak("hostile", self.player:getGeneral():isFemale())
						if self.slash_targets <= use.to:length() then return end
					end
				end
			end
		end
	end

	local targets = {}
	local ptarget = self:getPriorTarget()
	if ptarget and not self:slashProhibit(card, ptarget) then 
		table.insert(targets, ptarget)
	end
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		local slash_prohibit = false
		slash_prohibit = self:slashProhibit(card,enemy)
		if not slash_prohibit and enemy:objectName() ~= ptarget:objectName() then 
			table.insert(targets, enemy)
		end
	end
	
	for _, target in ipairs(targets) do
		local canliuli = false
		for _, friend in ipairs(self.friends_noself) do
			if self:canLiuli(target, friend) and self:slashIsEffective(card, friend) then canliuli = true end
		end
		if (self.player:canSlash(target, not no_distance) or
		(use.isDummy and self.predictedRange and (self.player:distanceTo(target) <= self.predictedRange))) and
		self:objectiveLevel(target) > 3
		and self:slashIsEffective(card, target) and
		not (target:hasSkill("xiangle") and basicnum < 2) and not canliuli and
		not (not self:isWeak(target) and #self.enemies > 1 and #self.friends > 1 and self.player:hasSkill("keji")
			and self:getOverflow() > 0 and not self:isEquip("Crossbow")) then
			-- fill the card use struct
			local usecard = card
			if not use.to or use.to:isEmpty() then
				local anal = self:searchForAnaleptic(use,target,card)
				if anal and not self:isEquip("SilverLion", target) and not self:isWeak() then
					if anal:getEffectiveId() ~= card:getEffectiveId() then use.card = anal return end
				end
				local equips = self:getCards("EquipCard", self.player, "h")
				for _, equip in ipairs(equips) do
					local callback = sgs.ai_slash_weaponfilter[equip:objectName()]
					if callback and type(callback) == "function" and callback(target, self) and
						self.player:distanceTo(target) <= (sgs.weapon_range[equip:className()] or 0) then
						self:useEquipCard(equip, use)
						if use.card then return end
					end
				end
				if target:isChained() and self:isGoodChainTarget(target) and not use.card then
					if self:isEquip("Crossbow") and card:inherits("NatureSlash") then
						local slashes = self:getCards("Slash")
						for _, slash in ipairs(slashes) do
							if not slash:inherits("NatureSlash") and self:slashIsEffective(slash, target)
								and not self:slashProhibit(slash, target) then
								usecard = slash
								break
							end
						end
					elseif not card:inherits("NatureSlash") then
						local slash = self:getCard("NatureSlash")
						if slash and self:slashIsEffective(slash, target) and not self:slashProhibit(slash, target) then usecard = slash end
					end
				end
			end
			use.card = use.card or usecard
			if use.to and not use.to:contains(target) then 
				use.to:append(target) 
				if self.slash_targets <= use.to:length() then return end
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
						if self.slash_targets <= use.to:length() then return end
					end
				end
			end
		end
	end
end

sgs.ai_skill_use.slash = function(self, prompt)
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

sgs.ai_card_intention.Slash = function(card,from,tos)
	if sgs.ai_liuli_effect then
		sgs.ai_liuli_effect=false
		return
	end
	for _, to in ipairs(tos) do
		local value = 80
		if sgs.ai_collateral then sgs.ai_collateral=false value = 0 end

		if sgs.ai_leiji_effect then
			if from and from:hasSkill("liegong") then return end
			sgs.ai_leiji_effect = false
			if sgs.ai_pojun_effect then
				value = value/1.5
			else
				--value = -value/1.5
				value = 0
			end
		end
		speakTrigger(card,from,to)
		if to:hasSkill("yiji") then
			-- value = value*(2-to:getHp())/1.1
			value = math.max(value*(2-to:getHp())/1.1, 0)
		end
		if from:hasSkill("pojun") and to:getHp() > 3 then value = 0 end
		sgs.updateIntention(from, to, value)
	end
end

sgs.ai_skill_cardask["slash-jink"] = function(self, data, pattern, target)
	local effect = data:toSlashEffect()
	if (not target or self:isFriend(target)) and effect.slash:hasFlag("jiefan-slash") then return "." end
	if sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) and not target:hasSkill("qianxi") then return "." end
	--if not target then self.room:writeToConsole(debug.traceback()) end
	if not target then return end
	if self:isFriend(target) then
		if target:hasSkill("rende") and self.player:hasSkill("jieming") then return "." end
		if target:hasSkill("pojun") and not self.player:faceUp() then return "." end
		if (target:hasSkill("jieyin") and (not self.player:isWounded()) and self.player:getGeneral():isMale()) and not self.player:hasSkill("leiji") then return "." end
		if self.player:isChained() and self:isGoodChainTarget(self.player) then return "." end
	else
		if not target:hasFlag("drank") then
			if target:hasSkill("mengjin") and self.player:hasSkill("jijiu") then return "." end
		end
		if not (self.player:getHandcardNum() == 1 and self:hasSkills(sgs.need_kongcheng)) and not target:hasSkill("qianxi") then
			if self:isEquip("Axe", target) then
				if self:hasSkills(sgs.lose_equip_skill, target) and target:getEquips():length() > 1 then return "." end
				if target:getHandcardNum() - target:getHp() > 2 then return "." end
			elseif self:isEquip("Blade", target) then
				if self:getCardsNum("Jink") <= self:getCardsNum("Slash", target) then return "." end
			end
		end
	end
end

sgs.dynamic_value.damage_card.Slash = true

sgs.ai_use_value.Slash = 4.6
sgs.ai_keep_value.Slash = 2
sgs.ai_use_priority.Slash = 2.4

function SmartAI:useCardPeach(card, use)
	local mustusepeach = false
	if not self.player:isWounded() then return end
	if self.player:hasSkill("longhun") and not self.player:isLord() and
		math.min(self.player:getMaxCards(), self.player:getHandcardNum()) + self.player:getCards("e"):length() > 3 then return end
	local peaches = 0
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	for _,card in ipairs(cards) do
		if card:inherits("Peach") then peaches = peaches+1 end
	end
	if self.player:isLord() and (self.player:hasSkill("hunzi") and not self.player:hasSkill("yingzi")) 
		and self.player:getHp() < 4 and self.player:getHp() > peaches then return end
	for _, friend in ipairs(self.enemies) do
		if (self:hasSkills(sgs.drawpeach_skill,enemy) and self.player:getHandcardNum() < 3) or (self.player:hasSkill("buqu") and self.player:getHp() < 1) then
			mustusepeach = true
		end
	end
	for _, friend in ipairs(self.friends_noself) do
		if not mustusepeach then
			if friend:isLord() and friend:getHp() == 1 and not friend:hasSkill("buqu") and peaches < 2 then return end
			if (self.player:getHp()-friend:getHp() > peaches) and (friend:getHp() < 3) and not friend:hasSkill("buqu") then return end
		end
	end

	if self.player:hasSkill("jieyin") and self:getOverflow() > 0 then
		self:sort(self.friends, "hp")
		for _, friend in ipairs(self.friends) do
			if friend:isWounded() and friend:getGeneral():isMale() then return end
		end
	end
		
	if self.player:hasSkill("ganlu") and not self.player:hasUsed("GanluCard") then
		local dummy_use = {isDummy = true}
		self:useSkillCard(sgs.Card_Parse("@GanluCard=."),dummy_use)
		if dummy_use.card then return end
	end

	use.card = card
end

sgs.ai_card_intention.Peach = -120

sgs.ai_use_value.Peach = 6
sgs.ai_keep_value.Peach = 5
sgs.ai_use_priority.Peach = 4.1

sgs.ai_use_value.Jink = 8.9
sgs.ai_keep_value.Jink = 4

sgs.dynamic_value.benefit.Peach = true

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

sgs.ai_skill_invoke.double_sword = true

function sgs.ai_slash_weaponfilter.double_sword(to, self)
	return self.player:getGender()~=to:getGender()
end

function sgs.ai_weapon_value.double_sword(self, enemy)
	if enemy and enemy:getGeneral():isMale() ~= self.player:getGeneral():isMale() then return 3 end
end

sgs.ai_skill_cardask["double-sword-card"] = function(self, data, pattern, target)
	if target and self:isFriend(target) then return "." end
	if self:needBear() then return "." end
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("Slash") or card:inherits("Shit") or card:inherits("Collateral") or card:inherits("GodSalvation")
		or card:inherits("Disaster") or card:inherits("EquipCard") or card:inherits("AmazingGrace") then
			return "$"..card:getEffectiveId()
		end
	end
	return "."
end

function sgs.ai_weapon_value.qinggang_sword(self, enemy)
	if enemy and enemy:getArmor() then return 3 end
end

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

function sgs.ai_slash_weaponfilter.guding_blade(to)
	return to:isKongcheng()
end

function sgs.ai_weapon_value.guding_blade(self, enemy)
	if not enemy then return end
	local value = 2
	if enemy:getHandcardNum() < 1 then value = 4 end
	return value
end

sgs.ai_skill_cardask["@axe"] = function(self, data, pattern, target)
	if target and self:isFriend(target) then return "." end

	local allcards = self.player:getCards("he")
	allcards = sgs.QList2Table(allcards)
	if self.player:hasFlag("drank") or #allcards-2 >= self.player:getHp() or (self.player:hasSkill("kuanggu") and self.player:isWounded()) then
		local cards = self.player:getCards("h")
		cards = sgs.QList2Table(cards)
		local index
		if self:hasSkills(sgs.need_kongcheng) then index = #cards end
		if self.player:getOffensiveHorse() then
			if index then
				if index < 2 then
					index = index + 1
					table.insert(cards, self.player:getOffensiveHorse())
				end
			end
			table.insert(cards, self.player:getOffensiveHorse())
		end
		if self.player:getArmor() then
			if index then
				if index < 2 then
					index = index + 1
					table.insert(cards, self.player:getArmor())
				end
			end
			table.insert(cards, self.player:getArmor())
		end
		if self.player:getDefensiveHorse() then
			if index then
				if index < 2 then
					index = index + 1
					table.insert(cards, self.player:getDefensiveHorse())
				end
			end
			table.insert(cards, self.player:getDefensiveHorse())
		end
		if #cards >= 2 then
			self:sortByUseValue(cards, true)
			return "$"..cards[1]:getEffectiveId().."+"..cards[2]:getEffectiveId()
		end
	end
end

function sgs.ai_slash_weaponfilter.axe(to, self)
	return self:getOverflow() > 0
end

function sgs.ai_weapon_value.axe(self, enemy)
	if enemy and enemy:getHp() < 3 then return 3 - enemy:getHp() end
end

sgs.ai_skill_cardask["blade-slash"] = function(self, data, pattern, target)
	if target and self:isFriend(target) and not (target:hasSkill("leiji") and self:getCardsNum("Jink", target, "h") > 0) then
		return "."
	end
	for _, slash in ipairs(self:getCards("Slash")) do
		if self:slashIsEffective(slash, target) then 
			return slash:toString()
		end 
	end
	return "."
end

function sgs.ai_weapon_value.blade(self, enemy)
	if not enemy then return self:getCardsNum("Slash") end
end

local spear_skill={}
spear_skill.name="spear"
table.insert(sgs.ai_skills,spear_skill)
spear_skill.getTurnUseCard=function(self,inclusive)
	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)

	if #cards<(self.player:getHp()+1) then return nil end
	if #cards<2 then return nil end

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

function sgs.ai_slash_weaponfilter.fan(to)
	local armor = to:getArmor()
	return armor and (armor:inherits("Vine") or armor:inherits("GaleShell"))
end

sgs.ai_skill_invoke.kylin_bow = function(self, data)
	local effect = data:toSlashEffect()

	if self:hasSkills(sgs.lose_equip_skill, effect.to) then
		return self:isFriend(effect.to)
	end

	return self:isEnemy(effect.to)
end

function sgs.ai_slash_weaponfilter.kylin_bow(to)
	if to:getDefensiveHorse() then return true else return false end
end

function sgs.ai_weapon_value.kylin_bow(self, target)
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if enemy:getOffensiveHorse() or enemy:getDefensiveHorse() then return 1 end
		end
	end
end

sgs.ai_skill_invoke.eight_diagram = function(self, data)
	local dying = 0
	local handang = self.room:findPlayerBySkillName("jiefan")
	for _, aplayer in sgs.qlist(self.room:getAlivePlayers()) do
		if aplayer:getHp() < 1 and not aplayer:hasSkill("buqu") then dying = 1 break end
	end
	if handang and self:isFriend(handang) and dying > 0 then return false end
	if sgs.hujiasource and not self:isFriend(sgs.hujiasource) then return false end
	if sgs.lianlisource and not self:isFriend(sgs.lianlisource) then return false end
	if self.player:hasSkill("tiandu") then return true end
	if self:hasSkills("leiji", self.enemies) and self:getFinalRetrial(sgs.hujiasource) == 2 then
		return false
	end	
	if self:getDamagedEffects(self) then return false end
	return true
end

function sgs.ai_armor_value.eight_diagram(player, self)
	local haszj = self:hasSkills("leiji", self:getEnemies(player))
	if haszj then 
		return 2
	end
	if player:hasSkill("tiandu") then 
		return 5
	end
	return 4 
end

function sgs.ai_armor_value.renwang_shield()
	return 4
end

function sgs.ai_armor_value.silver_lion(player, self)
	if self:hasWizard(self:getEnemies(player), true) then
		for _, player in sgs.qlist(self.room:getAlivePlayers()) do
			if player:containsTrick("lightning") then return 5 end
		end
	end
	return 1
end

sgs.ai_use_priority.OffensiveHorse = 2.69
sgs.ai_use_priority.Halberd = 2.685
sgs.ai_use_priority.KylinBow = 2.68
sgs.ai_use_priority.Blade = 2.675
sgs.ai_use_priority.GudingBlade = 2.67
sgs.ai_use_priority.DoubleSword =2.665
sgs.ai_use_priority.Spear = 2.66
sgs.ai_use_priority.IceSword = 2.65
sgs.ai_use_priority.QinggangSword = 2.645
sgs.ai_use_priority.Axe = 2.64
sgs.ai_use_priority.Crossbow = 2.63
sgs.ai_use_priority.SilverLion = 0.9
sgs.ai_use_priority.EightDiagram = 0.8
sgs.ai_use_priority.RenwangShield = 0.7
sgs.ai_use_priority.DefensiveHorse = 0

sgs.dynamic_value.damage_card.ArcheryAttack = true
sgs.dynamic_value.damage_card.SavageAssault = true

sgs.ai_use_value.ArcheryAttack = 3.8
sgs.ai_use_priority.ArcheryAttack = 3.5
sgs.ai_use_value.SavageAssault = 3.9
sgs.ai_use_priority.SavageAssault = 3.5

sgs.ai_skill_cardask.aoe = function(self, data, pattern, target, target2, name)
	if sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) then return "." end
	if not self:damageIsEffective(nil, nil, target) then return "." end
	local aoe = sgs.Sanguosha:cloneCard(name, sgs.Card_NoSuit, 0)
	if self.player:hasSkill("jianxiong") and self:getAoeValue(aoe) > -10 and
		(self.player:getHp()>1 or self:getAllPeachNum()>0) and not self.player:containsTrick("indulgence") then return "." end
end

sgs.ai_skill_cardask["savage-assault-slash"] = function(self, data, pattern, target, target2)
	return sgs.ai_skill_cardask.aoe(self, data, pattern, target, target2, "savage_assault")
end

sgs.ai_skill_cardask["archery-attack-jink"] = function(self, data, pattern, target)
	return sgs.ai_skill_cardask.aoe(self, data, pattern, target, target2, "archery_attack")
end

sgs.ai_keep_value.Nullification = 3
sgs.ai_use_value.Nullification = 8

function SmartAI:useCardAmazingGrace(card, use)
	if #self.friends >= #self.enemies or (self:hasSkills(sgs.need_kongcheng) and self.player:getHandcardNum() == 1)
		or self.player:hasSkill("jizhi") then
		use.card = card
	elseif self.player:hasSkill("wuyan") then
		use.card = card
	end
end

sgs.ai_use_value.AmazingGrace = 3
sgs.ai_keep_value.AmazingGrace = -1
sgs.ai_use_priority.AmazingGrace = 1

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

sgs.ai_use_priority.GodSalvation = 3.9
sgs.dynamic_value.benefit.GodSalvation = true

local function factorial(n)
	if n <= 0.1 then return 1 end
	return n*factorial(n-1)
end

function SmartAI:useCardDuel(duel, use)
	if self.player:hasSkill("wuyan") then return end
	self:sort(self.enemies,"handcard")
	local enemies = self:exclude(self.enemies, duel)
	local friends = self:exclude(self.friends_noself, duel)
	local target 
	local n1 = self:getCardsNum("Slash")
	if self.player:hasSkill("wushuang") then n1 = n1 * 2 end
	local huatuo = self.room:findPlayerBySkillName("jijiu")
	for _, friend in ipairs(friends) do
		if friend:hasSkill("jieming") and self.player:hasSkill("rende") and (huatuo and self:isFriend(huatuo))then
			use.card = duel
			if use.to then
				use.to:append(friend)
			end
			return
		end
	end
	local ptarget = self:getPriorTarget()
	if ptarget then
		local target = ptarget
		local n2 = target:getHandcardNum()
		if target:hasSkill("wushuang") then n2 = n2*2 end
		local useduel
		if target and self:objectiveLevel(target) > 3 and self:hasTrickEffective(duel, target) then
			if n1 >= n2 then
				useduel = true
			elseif n2 > n1*2 + 1 then
				useduel = false
			elseif n1 > 0 then
				local percard = 0.35
				if target:hasSkill("paoxiao") or target:hasWeapon("crossbow") then percard = 0.2 end
				local poss = percard ^ n1 * (factorial(n1)/factorial(n2)/factorial(n1-n2))
				if math.random() > poss then useduel = true end
			end
			if useduel then
				use.card = duel
				if use.to then
					use.to:append(target)
					self:speak("duel", self.player:getGeneral():isFemale())
				end
				return
			end
		end
	end
	local n2 
	for _, enemy in ipairs(enemies) do
		n2 = enemy:getHandcardNum()
		if self:objectiveLevel(enemy) > 3 then
			if enemy:hasSkill("wushuang") then n2 = n2*2 end
			target = enemy
			break
		end
	end
	
	local useduel
	if target and self:objectiveLevel(target) > 3 and self:hasTrickEffective(duel, target) then
		if n1 >= n2 then
			useduel = true
		elseif n2 > n1*2 + 1 then
			useduel = false
		elseif n1 > 0 then
			local percard = 0.35
			if target:hasSkill("paoxiao") or target:hasWeapon("crossbow") then percard = 0.2 end
			local poss = percard ^ n1 * (factorial(n1)/factorial(n2)/factorial(n1-n2))
			if math.random() > poss then useduel = true end
		end
		if useduel then
			use.card = duel
			if use.to then
				use.to:append(target)
				self:speak("duel", self.player:getGeneral():isFemale())
			end
			return
		end
	end
end

sgs.ai_card_intention.Duel=function(card,from,tos,source)
	if sgs.ai_lijian_effect then 
		sgs.ai_lijian_effect = false
		return
	end
	sgs.updateIntentions(from, tos, 80)
end

sgs.ai_use_value.Duel = 3.7
sgs.ai_use_priority.Duel = 2.9

sgs.dynamic_value.damage_card.Duel = true

sgs.ai_skill_cardask["duel-slash"] = function(self, data, pattern, target)
	if sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) then return "." end
	if self:isFriend(target) and target:hasSkill("rende") and self.player:hasSkill("jieming") then return "." end
	if (not self:isFriend(target) and self:getCardsNum("Slash")*2 >= target:getHandcardNum())
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

sgs.ai_keep_value.ExNihilo = 3.6
sgs.ai_use_value.ExNihilo = 10
sgs.ai_use_priority.ExNihilo = 6

sgs.dynamic_value.benefit.ExNihilo = true

function SmartAI:getDangerousCard(who)
	local weapon = who:getWeapon()
	if (weapon and weapon:inherits("Crossbow")) then return  weapon:getEffectiveId() end
	if (weapon and weapon:inherits("Spear") and who:hasSkill("paoxiao"))  then return  weapon:getEffectiveId() end
	if (weapon and weapon:inherits("Axe") and self:hasSkills("luoyi|pojun|jiushi|jiuchi", who)) then return weapon:getEffectiveId() end
	if (who:getArmor() and who:getArmor():inherits("EightDiagram") and who:getArmor():getSuit() == sgs.Card_Spade and who:hasSkill("leiji")) then return who:getArmor():getEffectiveId() end
	if (weapon and weapon:inherits("SPMoonSpear") and self:hasSkills("guidao|chongzhen|guicai|jilve", who)) then return weapon:getEffectiveId() end
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
	if friend and self:isWeak(friend) and who:inMyAttackRange(friend) then
		if weapon and who:distanceTo(friend) > 1 and not 
			(weapon and weapon:inherits("MoonSpear") and who:hasSkill("keji") and who:getHandcardNum() > 5) then return weapon:getEffectiveId() end
		if offhorse and who:distanceTo(friend) > 1 then return offhorse:getEffectiveId() end
	end

	if defhorse then
		for _,friend in ipairs(self.friends) do
			if friend:distanceTo(who) == friend:getAttackRange()+1 then
				return defhorse:getEffectiveId()
			end
		end
	end

	if armor and self:evaluateArmor(armor,who)>3 then
		return armor:getEffectiveId()
	end

	if self:isEquip("Monkey", who) then
		return offhorse:getEffectiveId()
	end


	local equips = sgs.QList2Table(who:getEquips())
	for _,equip in ipairs(equips) do
		if who:hasSkill("shensu") then return equip:getEffectiveId() end
		if who:hasSkill("longhun") and not equip:getSuit() == sgs.Card_Diamond then  return equip:getEffectiveId() end
		if who:hasSkill("qixi") and equip:isBlack() then  return equip:getEffectiveId() end
		if who:hasSkill("guidao") and equip:isBlack() then  return equip:getEffectiveId() end
		if who:hasSkill("guose") and equip:getSuit() == sgs.Card_Diamond then  return equip:getEffectiveId() end
		if who:hasSkill("jijiu") and equip:isRed() then  return equip:getEffectiveId() end
		if who:hasSkill("wusheng") and equip:isRed() then  return equip:getEffectiveId() end
		if who:hasSkill("duanliang") and equip:isBlack() then  return equip:getEffectiveId() end
	end

	if armor and self:evaluateArmor(armor, who)>0
		and not (armor:inherits("SilverLion") and who:isWounded()) then
		return armor:getEffectiveId()
	end

	if weapon then
		if not (who:hasSkill("xiaoji") and (who:getHandcardNum() >= who:getHp())) and not self:isEquip("YitianSword",who) then
			for _,friend in ipairs(self.friends) do
				if ((who:distanceTo(friend) <= who:getAttackRange()) and (who:distanceTo(friend) > 1)) or who:hasSkill("qiangxi") then
					return weapon:getEffectiveId()
				end
			end
		end
	end

	if offhorse then
		if who:hasSkill("xiaoji") and who:getHandcardNum() >= who:getHp() then
		else
			for _,friend in ipairs(self.friends) do
				if who:distanceTo(friend) == who:getAttackRange() and
				who:getAttackRange() > 1 then
					return offhorse:getEffectiveId()
				end
			end
		end
	end
end

function SmartAI:useCardSnatchOrDismantlement(card, use)
	local name = card:objectName()
	if self.player:hasSkill("wuyan") then return end
	local players = self.room:getOtherPlayers(self.player)
	local tricks
	players = self:exclude(players, card)
	for _, player in ipairs(players) do
		if player:containsTrick("lightning") and self:getFinalRetrial(player) ==2 and self:hasTrickEffective(card, player) then 
			use.card = card
			if use.to then 
				tricks = player:getCards("j")
				for _, trick in sgs.qlist(tricks) do
					if trick:inherits("Lightning") then
						sgs.ai_skill_cardchosen[name] = trick:getEffectiveId()
					end
				end
				use.to:append(player)
			end
			return
		end
	end

	self:sort(self.enemies,"defense")
	if sgs.getDefense(self.enemies[1]) >= 8 then self:sort(self.enemies, "threat") end
	local enemies = self:exclude(self.enemies, card)
	self:sort(self.friends_noself,"defense")
	local friends = self:exclude(self.friends_noself, card)
	local hasLion, target
	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(card, enemy) then
			if self:getDangerousCard(enemy) then
				use.card = card
				if use.to then
					sgs.ai_skill_cardchosen[name] = self:getDangerousCard(enemy)
					use.to:append(enemy)
					self:speak("hostile", self.player:getGeneral():isFemale())
				end
				return
			end
		end
	end

	for _, friend in ipairs(friends) do
		if (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) and self:hasTrickEffective(card, friend) then
			use.card = card
			if use.to then 
				tricks = friend:delayedTricks()
				for _, trick in sgs.qlist(tricks) do
					if trick:inherits("Indulgence") then
						if friend:getHp() < friend:getHandcardNum() then
							sgs.ai_skill_cardchosen[name] = trick:getEffectiveId() 
						end
					end
					if trick:inherits("SupplyShortage") then
						sgs.ai_skill_cardchosen[name] = trick:getEffectiveId() 
					end
					if trick:inherits("Indulgence") then
						sgs.ai_skill_cardchosen[name] = trick:getEffectiveId() 
					end
				end				
				use.to:append(friend) 
			end
			return
		end
		if self:isEquip("SilverLion", friend) and self:hasTrickEffective(card, friend) and 
		friend:isWounded() and (friend:hasSkill("benghuai") or (friend:getHp() < 4 and not friend:hasSkill("longhun"))) then
			hasLion = true
			target = friend
		end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(card, enemy) then
			if self:getValuableCard(enemy) then
				use.card = card
				if use.to then
					sgs.ai_skill_cardchosen[name] = self:getValuableCard(enemy)
					use.to:append(enemy)
					self:speak("hostile", self.player:getGeneral():isFemale())
				end
				return
			end
		end
	end

	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(card, enemy) and not self:needKongcheng(enemy) and not enemy:hasSkill("kongcheng") then
			if enemy:getHandcardNum() == 1 then 
				use.card = card
				if use.to then
					sgs.ai_skill_cardchosen[name] = self:getCardRandomly(enemy, "h")
					use.to:append(enemy)
					self:speak("hostile", self.player:getGeneral():isFemale())
				end
				return	
			end
		end
	end

	if hasLion then
		use.card = card
		if use.to then 
			sgs.ai_skill_cardchosen[name] = target:getArmor():getEffectiveId()
			use.to:append(target) 
		end
		return
	end

	if name == "snatch" or self:getOverflow() > 0 then
		for _, enemy in ipairs(enemies) do
			local equips = enemy:getEquips()
			if not enemy:isNude() and self:hasTrickEffective(card, enemy) and not enemy:hasSkill("tuntian") and
				not (self:hasSkills(sgs.lose_equip_skill, enemy) and enemy:getHandcardNum() == 0) and
				not (enemy:getCards("he"):length() == 1 and self:isEquip("GaleShell",enemy)) then
				if enemy:getHandcardNum() == 1 then
				if self:needKongcheng(enemy) or self:hasSkills("kongcheng|lianying", enemy) then return end
				end
				if self:hasSkills(sgs.cardneed_skill, enemy) then
					use.card = card
					if use.to then 
						sgs.ai_skill_cardchosen[name] = self:getCardRandomly(enemy, "he") 
						use.to:append(enemy)
						self:speak("hostile", self.player:getGeneral():isFemale())
					end
					return
				else
					use.card = card
					if use.to then
						if not equips:isEmpty() then
							sgs.ai_skill_cardchosen[name] = self:getCardRandomly(enemy, "e")
						else 
							sgs.ai_skill_cardchosen[name] = self:getCardRandomly(enemy, "h") end
						use.to:append(enemy)
						self:speak("hostile", self.player:getGeneral():isFemale())
					end
					return
				end
			end
		end
	end
end

SmartAI.useCardSnatch = SmartAI.useCardSnatchOrDismantlement

sgs.ai_use_value.Snatch = 9
sgs.ai_use_priority.Snatch = 4.3

sgs.dynamic_value.control_card.Snatch = true
function sgs.ai_card_intention.Snatch()
	sgs.ai_snat_disma_effect = false
end

SmartAI.useCardDismantlement = SmartAI.useCardSnatchOrDismantlement

sgs.ai_use_value.Dismantlement = 5.6
sgs.ai_use_priority.Dismantlement = 4.4
function sgs.ai_card_intention.Dismantlement()
	sgs.ai_snat_disma_effect = false
end

sgs.dynamic_value.control_card.Dismantlement = true

function SmartAI:useCardCollateral(card, use)
	if self.player:hasSkill("wuyan") then return end
	self:sort(self.enemies,"threat")

	for _, friend in ipairs(self.friends_noself) do
		if friend:getWeapon() and self:hasSkills(sgs.lose_equip_skill, friend) 
			and not self.room:isProhibited(self.player, friend, card) then

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
			and not self:hasSkills(sgs.lose_equip_skill, enemy)
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

sgs.ai_use_value.Collateral = 8.8
sgs.ai_use_priority.Collateral = 2.75

sgs.ai_card_intention.Collateral = function(card, from, tos)
	assert(#tos == 2)
	if tos[2]:objectName() == from:objectName() then
		sgs.updateIntention(from, tos[1], 80)
	elseif sgs.compareRoleEvaluation(tos[1], "rebel", "loyalist") == sgs.compareRoleEvaluation(tos[2], "rebel", "loyalist") then
		sgs.updateIntention(from, tos[2], 80)
	elseif from:getWeapon() and from:inMyAttackRange(tos[2]) then
		sgs.updateIntention(from, tos[1], 80)
	elseif tos[1]:isKongcheng() then
		sgs.updateIntention(from, tos[1], 80)
	end
	sgs.ai_collateral = false
end

sgs.dynamic_value.control_card.Collateral = true

sgs.ai_skill_cardask["collateral-slash"] = function(self, data, pattern, target, target2)
	if self:needBear() then return "." end
	if target and target2 and not self:hasSkills(sgs.lose_equip_skill) and self:isEnemy(target2) then
		for _, slash in ipairs(self:getCards("Slash")) do
			if self:slashIsEffective(slash, target) then 
				return slash:toString()
			end 
		end
	end
	if target and target2 and not self:hasSkills(sgs.lose_equip_skill) and self:isFriend(target2) then
		for _, slash in ipairs(self:getCards("Slash")) do
			if not self:slashIsEffective(slash, target2) then
				return slash:toString()
			end 
		end
		if (target2:getHp() > 2 or self:getCardsNum("Jink", target2) > 1) and not target2:getRole() == "lord" and self.player:getHandcardNum() > 1 then
			for _, slash in ipairs(self:getCards("Slash")) do
				return slash:toString()
			end 
		end
	end
	self:speak("collateral", self.player:getGeneral():isFemale())
	return "."
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
		if self:hasSkills("lijian|fanjian",enemy) and not enemy:containsTrick("indulgence") and not enemy:isKongcheng() and enemy:faceUp() and self:objectiveLevel(enemy) > 3 then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
	
	for _, enemy in ipairs(enemies) do
		if not enemy:containsTrick("indulgence") and not enemy:hasSkill("keji") and enemy:faceUp() and self:objectiveLevel(enemy) > 3 then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

sgs.ai_use_value.Indulgence = 8

sgs.ai_card_intention.Indulgence = 120

sgs.dynamic_value.control_usecard.Indulgence = true

function SmartAI:useCardLightning(card, use)
	if self.player:containsTrick("lightning") then return end
	--if self.player:hasSkill("weimu") and card:isBlack() then return end
	if self.room:isProhibited(self.player, self.player, card) then end

	--if not self:hasWizard(self.enemies) then--and self.room:isProhibited(self.player, self.player, card) then
	local function hasDangerousFriend() 
		local hashy = false
		for _, aplayer in ipairs(self.enemies) do
			if aplayer:hasSkill("hongyan") then hashy = true break end
		end
		for _, aplayer in ipairs(self.enemies) do
			if aplayer:hasSkill("guanxing") or (aplayer:hasSkill("gongxin") and hashy) 
			or aplayer:hasSkill("xinzhan") then 
				if self:isFriend(aplayer:getNextAlive()) then return true end
			end
		end
		return false
	end
	if self:getFinalRetrial(self.player) == 2 then 
	return
	elseif self:getFinalRetrial(self.player) == 1 then
		use.card = card
		return
	elseif not hasDangerousFriend() then
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

sgs.dynamic_value.lucky_chance.Lightning = true

sgs.ai_keep_value.Lightning = -1
