neoluoyi_skill = {}
neoluoyi_skill.name = "neoluoyi"
table.insert(sgs.ai_skills, neoluoyi_skill)
neoluoyi_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("LuoyiCard") then return nil end
	if self:needBear() then return nil end
	local luoyicard
	if self:needToThrowArmor() then
		luoyicard = self.player:getArmor()
		return sgs.Card_Parse("@LuoyiCard=" .. luoyicard:getEffectiveId())
	end
	
	if not self:slashIsAvailable(self.player) then return nil end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	local slashtarget = 0
	local dueltarget = 0
	local equipnum = 0
	local offhorse = self.player:getOffensiveHorse()
	local noHorseTargets = 0
	self:sort(self.enemies,"hp")
	for _, card in sgs.qlist(self.player:getCards("he")) do
		if card:isKindOf("EquipCard") and not (card:isKindOf("Weapon") and self:hasEquip(card))  then
			equipnum = equipnum + 1
		end
	end
	for _,card in ipairs(cards) do
		if card:isKindOf("Slash") then
			for _,enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy, card) and self:slashIsEffective(card, enemy) and self:objectiveLevel(enemy) > 3 and sgs.isGoodTarget(enemy, self.enemies, self) then
					if getCardsNum("Jink", enemy) < 1 or (self:isEquip("Axe") and self.player:getCards("he"):length() > 4) then
						slashtarget = slashtarget + 1
						if offhorse and self.player:distanceTo(enemy, 1)<=self.player:getAttackRange() then
							noHorseTargets = noHorseTargets + 1
						end
					end
				end
			end
		end
		if card:isKindOf("Duel") then
			for _, enemy in ipairs(self.enemies) do
				if self:getCardsNum("Slash") >= getCardsNum("Slash", enemy) and sgs.isGoodTarget(enemy, self.enemies, self)
				and self:objectiveLevel(enemy) > 3 and not self:cantbeHurt(enemy, 2) and self:damageIsEffective(enemy) and enemy:getMark("@late") == 0 then
					dueltarget = dueltarget + 1 
				end
			end
		end
	end		
	if (slashtarget + dueltarget) > 0 and equipnum > 0 then
		self:speak("luoyi")
		if self:needToThrowArmor() then
			luoyicard = self.player:getArmor()
		end
		
		if not luoyicard then
			for _, card in sgs.qlist(self.player:getCards("he")) do
				if card:isKindOf("EquipCard") and not self.player:hasEquip(card) then 
					luoyicard = card
					break
				end
			end
		end
		if not luoyicard and offhorse then
			if noHorseTargets == 0 then
				for _, card in sgs.qlist(self.player:getCards("he")) do
					if card:isKindOf("EquipCard") and not card:isKindOf("OffensiveHorse") then 
						luoyicard = card
						break
					end
				end
				if not luoyicard and dueltarget == 0 then return nil end
			end
		end
		if not luoyicard then
			for _, card in sgs.qlist(self.player:getCards("he")) do
				if card:isKindOf("EquipCard") and not card:isKindOf("Weapon") then 
					luoyicard = card
					break
				end
			end
		end
		if not luoyicard then return nil end
		return sgs.Card_Parse("@LuoyiCard=" .. luoyicard:getEffectiveId())
	end
end

sgs.ai_skill_use_func.LuoyiCard=function(card,use,self)
	use.card = card
end

sgs.ai_use_priority.LuoyiCard = 9.2

local neofanjian_skill={}
neofanjian_skill.name = "neofanjian"
table.insert(sgs.ai_skills, neofanjian_skill)
neofanjian_skill.getTurnUseCard=function(self)
	if self.player:isKongcheng() then return nil end
	if self.player:usedTimes("NeoFanjianCard") > 0 then return nil end

	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	
	local hand_card
	local n = self.player:getHandcardNum()
		for i = 1,n do
		        if not (cards[i]:isKindOf("Analeptic") or cards[i]:isKindOf("Peach")) then
				hand_card = cards[i]
				break
			end
		end
	if not hand_card then return nil end

	local keep_value = self:getKeepValue(hand_card)
	if hand_card:getSuit() == sgs.Card_Diamond then keep_value = keep_value + 1 end

	if keep_value < 6 then
		local card_id = hand_card:getEffectiveId()
		local card_str = "@NeoFanjianCard=" .. card_id
		local fanjianCard = sgs.Card_Parse(card_str)
		assert(fanjianCard)
		return fanjianCard
	end
end

sgs.ai_skill_use_func.NeoFanjianCard = sgs.ai_skill_use_func.FanjianCard
sgs.ai_card_intention.NeoFanjianCard = sgs.ai_card_intention.FanjianCard
sgs.dynamic_value.damage_card.NeoFanjianCard = true

function sgs.ai_skill_suit.neofanjian(self)
	local map = {0, 0, 1, 2, 2, 3, 3, 3}
	local suit = map[math.random(1, 8)]
	if self.player:hasSkill("hongyan") and suit == sgs.Card_Spade then return sgs.Card_Heart else return suit end
end

sgs.ai_skill_invoke.yishi = function(self, data)
	local damage = data:toDamage()
	local target = damage.to
	if self:isFriend(target) then
		if damage.damage == 1 and self:getDamagedEffects(target, self.player)
			and (target:getJudgingArea():isEmpty() or target:containsTrick("YanxiaoCard")) then
			return false
		end
		return true
	else
		if self:hasHeavySlashDamage(self.player, damage.card, target) then return false end
		if self:isWeak(target) then return false end
		if self:doNotDiscard(target, "e", true) then
			return false
		end
		if self:getDamagedEffects(target, self.player, true) or (target:getArmor() and not target:getArmor():isKindOf("SilverLion")) then return true end
		if self:getDangerousCard(target) then return true end
		if target:getDefensiveHorse() then return true end
		return false
	end 
end

sgs.ai_skill_invoke.zhulou = function(self, data)
	local weaponnum = 0
	for _, card in sgs.qlist(self.player:getCards("h")) do
		if card:isKindOf("Weapon") then
			weaponnum = weaponnum + 1
		end
	end

	if weaponnum > 0 then return true end
		
	if self.player:getHandcardNum() < 3 and self.player:getHp() > 2 then
		return true
	end

	if self.player:getHp() < 3 and self.player:getWeapon() then
		return true
	end

	return false
end

sgs.ai_skill_cardask["@zhulou-discard"] =  function(self, data)
	if self.player:getWeapon() then
		return "$" .. self.player:getWeapon():getEffectiveId()
	end

	for _, card in sgs.qlist(self.player:getCards("he")) do
		if card:isKindOf("Weapon") then
			return "$" .. card:getEffectiveId()
		end
	end
	return "."
end

sgs.ai_cardneed.zhulou = sgs.ai_cardneed.weapon

sgs.zhulou_keep_value = sgs.qiangxi_keep_value

function sgs.ai_skill_invoke.neojushou(self, data)
	if not self.player:faceUp() then return true end
	for _, friend in ipairs(self.friends) do
		if self:hasSkills("fangzhu|jilve", friend) then return true end
	end
	return self:isWeak()
end

sgs.ai_skill_invoke.neoganglie = function(self, data)
	local who = data:toPlayer()
	if self:isFriend(who) and (self:getDamagedEffects(who, self.player) or self:needToLoseHp(who, self.player, nil, true)) then
		who:setFlags("ganglie_target")
		return true
	end
	if self:getDamagedEffects(who, self.player) and self:isEnemy(who) and who:getHandcardNum() < 2 then 
		return false
	end
	
	return not self:isFriend(who)
end

sgs.ai_choicemade_filter.skillInvoke.neoganglie = function(player, promptlist, self)
	if sgs.ganglie_target then
		local target = sgs.ganglie_target
		local intention = 10
		if promptlist[3] == "yes" then
			if self:getDamagedEffects(target, player) or self:needToLoseHp(target, player, nil, true) then
				intention = 0
			end
			sgs.updateIntention(player, target, intention)
		elseif self:canAttack(target) then
			sgs.updateIntention(player, target, -10)
		end
	end
	sgs.ganglie_target = nil
end

sgs.ai_need_damaged.neoganglie = function (self, attacker, player)
	if not player:hasSkill("neoganglie") then return false end
	if self:isEnemy(attacker, player) and attacker:getHp() <= 2 and not attacker:hasSkill("buqu") and sgs.isGoodTarget(attacker, self.enemies, self)
		and not self:getDamagedEffects(attacker, player) and not self:needToLoseHp(attacker, player) then
			return true
	end
	return false
end

sgs.ai_skill_choice.neoganglie = function(self, choices)
	local target
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasFlag("ganglie_target") then
			target = player
			target:setFlags("-ganglie_target")
		end
	end
	if (self:getDamagedEffects(target, self.player) or self:needToLoseHp(target, self.player)) and self:isFriend(target) then return "damage" end

	if (self:getDamagedEffects(target, self.player) or self:needToLoseHp(target, self.player)) and target:getHandcardNum() > 1 then
		return "throw"
	end
	return "damage"
end

sgs.ai_skill_discard.neoganglie = function(self, discard_num, min_num, optional, include_equip)
	local to_discard = {}
	local cards = sgs.QList2Table(self.player:getHandcards())
	local index = 0
	self:sortByKeepValue(cards)
	cards = sgs.reverse(cards)

	for i = #cards, 1, -1 do
		local card = cards[i]
		if not self.player:isJilei(card) then
			table.insert(to_discard, card:getEffectiveId())
			table.remove(cards, i)
			index = index + 1
			if index == 2 then break end
		end
	end	
	return to_discard
end

sgs.ai_suit_priority.yishi= "club|spade|diamond|heart"
