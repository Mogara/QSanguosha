sgs.ai_skill_invoke.jianxiong = function(self, data)
	if self.jianxiong then self.jianxiong = nil return true end
	return not self:needKongcheng(self.player, true)
end

sgs.ai_skill_invoke.fankui = function(self, data)
	local target = data:toDamage().from
	if not target then return end
	if sgs.ai_need_damaged.fankui(self, target, self.player) then return true end

	if self:isFriend(target) then
		if self:getOverflow(target) > 2 then return true end
		if self:doNotDiscard(target) then return true end
		return (target:hasSkills(sgs.lose_equip_skill) and not target:getEquips():isEmpty())
		  or (self:needToThrowArmor(target) and target:getArmor()) or self:doNotDiscard(target)
	end
	if self:isEnemy(target) then
		if self:doNotDiscard(target) then return false end
		return true
	end
	return true
end

sgs.ai_choicemade_filter.cardChosen.fankui = function(self, player, promptlist)
	local damage = self.room:getTag("CurrentDamageStruct"):toDamage()
	if damage.from then
		local intention = 10
		local id = promptlist[3]
		local card = sgs.Sanguosha:getCard(id)
		local target = damage.from
		if self:needToThrowArmor(target) and self.room:getCardPlace(id) == sgs.Player_PlaceEquip and card:isKindOf("Armor") then
			intention = -intention
		elseif self:doNotDiscard(target) then intention = -intention
		elseif target:hasSkills(sgs.lose_equip_skill) and not target:getEquips():isEmpty() and
			self.room:getCardPlace(id) == sgs.Player_PlaceEquip and card:isKindOf("EquipCard") then
				intention = -intention
		elseif sgs.ai_need_damaged.fankui(self, target, player) then intention = 0
		elseif self:getOverflow(target) > 2 then intention = 0
		end
		sgs.updateIntention(player, target, intention)
	end
end

sgs.ai_skill_cardchosen.fankui = function(self, who, flags)
	local suit = sgs.ai_need_damaged.fankui(self, who, self.player)
	if not suit then return nil end

	local cards = sgs.QList2Table(who:getEquips())
	local handcards = sgs.QList2Table(who:getHandcards())
	if #handcards==1 and handcards[1]:hasFlag("visible") then table.insert(cards,handcards[1]) end

	for i=1,#cards,1 do
		if (cards[i]:getSuit() == suit and suit ~= sgs.Card_Spade) or
			(cards[i]:getSuit() == suit and suit == sgs.Card_Spade and cards[i]:getNumber() >= 2 and cards[i]:getNumber()<=9) then
			return cards[i]
		end
	end
	return nil
end


sgs.ai_need_damaged.fankui = function (self, attacker, player)
	if not player:hasSkill("guicai+fankui") then return false end
	if not attacker then return end
	local need_retrial = function(target)
		local alive_num = self.room:alivePlayerCount()
		return alive_num + target:getSeat() % alive_num > self.room:getCurrent():getSeat()
				and target:getSeat() < alive_num + player:getSeat() % alive_num
	end
	local retrial_card ={["spade"]=nil,["heart"]=nil,["club"]=nil}
	local attacker_card ={["spade"]=nil,["heart"]=nil,["club"]=nil}

	local handcards = sgs.QList2Table(player:getHandcards())
	for i=1,#handcards,1 do
		if handcards[i]:getSuit() == sgs.Card_Spade and handcards[i]:getNumber()>=2 and handcards[i]:getNumber()<=9 then
			retrial_card.spade = true
		end
		if handcards[i]:getSuit() == sgs.Card_Heart then
			retrial_card.heart = true
		end
		if handcards[i]:getSuit() == sgs.Card_Club then
			retrial_card.club = true
		end
	end

	local cards = sgs.QList2Table(attacker:getEquips())
	local handcards = sgs.QList2Table(attacker:getHandcards())
	if #handcards==1 and handcards[1]:hasFlag("visible") then table.insert(cards,handcards[1]) end

	for i=1,#cards,1 do
		if cards[i]:getSuit() == sgs.Card_Spade and cards[i]:getNumber()>=2 and cards[i]:getNumber()<=9 then
			attacker_card.spade = sgs.Card_Spade
		end
		if cards[i]:getSuit() == sgs.Card_Heart then
			attacker_card.heart = sgs.Card_Heart
		end
		if cards[i]:getSuit() == sgs.Card_Club then
			attacker_card.club = sgs.Card_Club
		end
	end

	local players = self.room:getOtherPlayers(player)
	for _, aplayer in sgs.qlist(players) do
		if aplayer:containsTrick("lightning") and self:getFinalRetrial(aplayer) ==1 and need_retrial(aplayer) then
			if not retrial_card.spade and attacker_card.spade then return attacker_card.spade end
		end

		if self:isFriend(aplayer, player) and not aplayer:hasSkill("qiaobian") then

			if aplayer:containsTrick("indulgence") and self:getFinalRetrial(aplayer) == 1 and need_retrial(aplayer) and aplayer:getHandcardNum() >= aplayer:getHp() then
				if not retrial_card.heart and attacker_card.heart then return attacker_card.heart end
			end
		end
	end
	return false
end


sgs.ai_skill_cardask["@guicai-card"]=function(self, data)
	local judge = data:toJudge()

	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getHandcards())
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return "$" .. card_id
		end
	end

	return "."
end

function sgs.ai_cardneed.guicai(to, card, self)
	for _, player in sgs.qlist(self.room:getAllPlayers()) do
		if self:getFinalRetrial(to) == 1 then
			if player:containsTrick("lightning") then
				return card:getSuit() == sgs.Card_Spade and card:getNumber() >= 2 and card:getNumber() <= 9 and not self.player:hasSkill("hongyan")
			end
			if self:isFriend(player) and self:willSkipDrawPhase(player) then
				return card:getSuit() == sgs.Card_Club
			end
			if self:isFriend(player) and self:willSkipPlayPhase(player) then
				return card:getSuit() == sgs.Card_Heart
			end
		end
	end
end

sgs.guicai_suit_value = {
	heart = 3.9,
	club = 3.9,
	spade = 3.5
}


sgs.ai_skill_invoke.ganglie = function(self, data)
	local mode = self.room:getMode()
	local damage = data:toDamage()
	if not damage.from then
		local zhangjiao = self.room:findPlayerBySkillName("guidao")
		return zhangjiao and self:isFriend(zhangjiao) and not zhangjiao:isNude()
	end
	if self:getDamagedEffects(damage.from, self.player) then
		if self:isFriend(damage.from) then
			return true
		end
		return false
	end
	return not self:isFriend(damage.from) and self:canAttack(damage.from)
end

sgs.ai_need_damaged.ganglie = function(self, attacker, player)
	if not attacker then return end
	if self:isEnemy(attacker) and attacker:getHp() + attacker:getHandcardNum() <= 3
		and not (attacker:hasSkills(sgs.need_kongcheng .. "|buqu") and attacker:getHandcardNum() > 1) and sgs.isGoodTarget(attacker, self:getEnemies(attacker), self) then
		return true
	end
	return false
end

function ganglie_discard(self, discard_num, min_num, optional, include_equip, skillName)
	local xiahou = self.room:findPlayerBySkillName(skillName)
	
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if isCard("Peach", card, self.player) then
			return {}
		end
	end
	
	if xiahou and (not self:damageIsEffective(self.player, sgs.DamageStruct_Normal, xiahou) or self:getDamagedEffects(self.player, xiahou)) then return {} end
	if xiahou and self:needToLoseHp(self.player, xiahou) then return {} end
end

sgs.ai_skill_discard.ganglie = function(self, discard_num, min_num, optional, include_equip)
	return ganglie_discard(self, discard_num, min_num, optional, include_equip, "ganglie")
end

function sgs.ai_slash_prohibit.ganglie(self, from, to)
	if self:isFriend(from, to) then return false end
	return from:getHandcardNum() + from:getHp() < 4
end

sgs.ai_choicemade_filter.skillInvoke.ganglie = function(self, player, promptlist)
	local damage = self.room:getTag("CurrentDamageStruct"):toDamage()
	if damage.from and damage.to then
		if promptlist[#promptlist] == "yes" then
			if not self:getDamagedEffects(damage.from, player) and not self:needToLoseHp(damage.from, player) then
				sgs.updateIntention(damage.to, damage.from, 40)
			end
		elseif self:canAttack(damage.from) then
			sgs.updateIntention(damage.to, damage.from, -40)
		end
	end
end

function SmartAI:findTuxiTarget()

	self:sort(self.enemies, "handcard_defense")
	local targets = {}

	local zhugeliang = self.room:findPlayerBySkillName("kongcheng")
	local luxun = self.room:findPlayerBySkillName("lianying")
	local dengai = self.room:findPlayerBySkillName("tuntian")

	local add_player = function (player, isfriend)
		if player:getHandcardNum() == 0 or player:objectName() == self.player:objectName() then return #targets end
		if #targets == 0 then
			table.insert(targets, player:objectName())
		elseif #targets == 1 then
			if player:objectName() ~= targets[1] then
				table.insert(targets, player:objectName())
			end
		end
		if isfriend and isfriend == 1 then
			self.player:setFlags("tuxi_isfriend_"..player:objectName())
		end
		return #targets
	end

	if zhugeliang and self:isFriend(zhugeliang) and sgs.ai_explicit[zhugeliang:objectName()] ~= "unknown" and zhugeliang:getHandcardNum() == 1
		and self:getEnemyNumBySeat(self.player,zhugeliang) > 0 then
		if zhugeliang:getHp() <= 2 then
			if add_player(zhugeliang, 1) == 2 then return targets end
		else
			local flag = string.format("%s_%s_%s","visible",self.player:objectName(),zhugeliang:objectName())
			local cards = sgs.QList2Table(zhugeliang:getHandcards())
			if #cards == 1 and (cards[1]:hasFlag("visible") or cards[1]:hasFlag(flag)) then
				if cards[1]:isKindOf("TrickCard") or cards[1]:isKindOf("Slash") or cards[1]:isKindOf("EquipCard") then
					if add_player(zhugeliang, 1) == 2 then return targets end
				end
			end
		end
	end

	if luxun and self:isFriend(luxun) and sgs.ai_explicit[luxun:objectName()] ~= "unknown" and luxun:getHandcardNum() == 1 and self:getEnemyNumBySeat(self.player,luxun) > 0 then
		local flag = string.format("%s_%s_%s","visible",self.player:objectName(),luxun:objectName())
		local cards = sgs.QList2Table(luxun:getHandcards())
		if #cards==1 and (cards[1]:hasFlag("visible") or cards[1]:hasFlag(flag)) then
			if cards[1]:isKindOf("TrickCard") or cards[1]:isKindOf("Slash") or cards[1]:isKindOf("EquipCard") then
				if add_player(luxun, 1) == 2  then return targets end
			end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		local cards = sgs.QList2Table(enemy:getHandcards())
		local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), enemy:objectName())
		for _, card in ipairs(cards) do
			if (card:hasFlag("visible") or card:hasFlag(flag)) and (card:isKindOf("Peach") or card:isKindOf("Nullification") or card:isKindOf("Analeptic") ) then
				if add_player(enemy) == 2  then return targets end
			end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if enemy:hasSkills("jijiu|qingnang|leiji|jieyin|beige|kanpo|liuli|qiaobian|zhiheng|guidao|tianxiang|lijian") then
			if add_player(enemy) == 2 then return targets end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		local x = enemy:getHandcardNum()
		local good_target = true
		if x == 1 and self:needKongcheng(enemy) then good_target = false end
		if x >= 2 and enemy:hasSkill("tuntian") then good_target = false end
		if good_target and add_player(enemy) == 2 then return targets end
	end

	if luxun and add_player(luxun, (self:isFriend(luxun) and 1 or nil)) == 2 then
		return targets
	end

	local others = self.room:getOtherPlayers(self.player)
	for _, other in sgs.qlist(others) do
		if self:objectiveLevel(other) >= 0 and not other:hasSkill("tuntian") and add_player(other) == 2 then
			return targets
		end
	end

	for _, other in sgs.qlist(others) do
		if self:objectiveLevel(other) >= 0 and not other:hasSkill("tuntian") and add_player(other) == 1 and math.random(0, 5) <= 1 and not self.player:hasSkill("qiaobian") then
			return targets
		end
	end
end

sgs.ai_skill_use["@@tuxi"] = function(self, prompt)
	local targets = self:findTuxiTarget()
	if type(targets) == "table" and #targets > 0 then
		return ("@TuxiCard=.&tuxi->" .. table.concat(targets, "+"))
	end
	return "."
end

sgs.ai_skill_invoke.luoyi = function(self,data)
	if self.player:isSkipped(sgs.Player_Play) then return false end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	local slashtarget = 0
	local dueltarget = 0
	self:sort(self.enemies,"hp")
	for _,card in ipairs(cards) do
		if card:isKindOf("Slash") then
			for _,enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy, card, true) and self:slashIsEffective(card, enemy) and self:objectiveLevel(enemy) > 3 and sgs.isGoodTarget(enemy, self.enemies, self) then
					if getCardsNum("Jink", enemy) < 1 or (self.player:hasWeapon("Axe") and self.player:getCards("he"):length() > 4) then
						slashtarget = slashtarget + 1
					end
				end
			end
		end
		if card:isKindOf("Duel") then
			for _, enemy in ipairs(self.enemies) do
				if self:getCardsNum("Slash") >= getCardsNum("Slash", enemy) and sgs.isGoodTarget(enemy, self.enemies, self)
				and self:objectiveLevel(enemy) > 3 and self:damageIsEffective(enemy) then
					dueltarget = dueltarget + 1
				end
			end
		end
	end
	if (slashtarget+dueltarget) > 0 then
		self:speak("luoyi")
		return true
	end
	return false
end

function sgs.ai_cardneed.luoyi(to, card, self)
	local slash_num = 0
	local target
	local slash = sgs.Sanguosha:cloneCard("slash")

	local cards = to:getHandcards()
	local need_slash = true
	for _, c in sgs.qlist(cards) do
		local flag = string.format("%s_%s_%s","visible",self.room:getCurrent():objectName(),to:objectName())
		if c:hasFlag("visible") or c:hasFlag(flag) then
			if isCard("Slash", c, to) then
				need_slash = false
				break
			end
		end
	end

	self:sort(self.enemies, "defenseSlash")
	for _, enemy in ipairs(self.enemies) do
		if to:canSlash(enemy) and not self:slashProhibit(slash ,enemy) and self:slashIsEffective(slash, enemy) and sgs.getDefenseSlash(enemy) <= 2 then
			target = enemy
			break
		end
	end

	if need_slash and target and isCard("Slash", card, to) then return true end
	return isCard("Duel",card, to)
end

sgs.luoyi_keep_value = {
	Peach 			= 6,
	Analeptic 		= 5.8,
	Jink 			= 5.2,
	Duel			= 5.5,
	FireSlash 		= 5.6,
	Slash 			= 5.4,
	ThunderSlash 	= 5.5,
	Axe				= 5,
	Blade 			= 4.9,
	Spear 			= 4.9,
	Fan				= 4.8,
	KylinBow		= 4.7,
	Halberd			= 4.6,
	MoonSpear		= 4.5,
	SPMoonSpear = 4.5,
	DefensiveHorse 	= 4
}


sgs.ai_skill_invoke.tiandu = function(self, data)
	local judge = data:toJudge()
	if judge.reason == "tuntian" then return false end
	return not (self:needKongcheng() and self.player:isKongcheng())
end 

function sgs.ai_slash_prohibit.tiandu(self, from, to)
	if self:canLiegong(to, from) then return false end
	if self:isEnemy(to) and self:hasEightDiagramEffect(to) and not IgnoreArmor(from, to) and #self.enemies > 1 then return true end
end

sgs.ai_skill_invoke.yiji = function(self)
	if self.player:getHandcardNum() < 2 then return true end
	for _, friend in ipairs(self.friends) do
		if not self:needKongcheng(friend, true) then return true end
	end
	return
end

sgs.ai_skill_askforyiji.yiji = function(self, card_ids)
	local cards = {}
	for _, card_id in ipairs(card_ids) do
		table.insert(cards, sgs.Sanguosha:getCard(card_id))
	end

	if self.player:getHandcardNum() <= 2 then
		return nil, -1
	end

	local new_friends = {}
	for _, friend in ipairs(self.friends) do
		if not self:needKongcheng(friend, true) then table.insert(new_friends, friend) end
	end

	if #new_friends > 0 then
		local card, target = self:getCardNeedPlayer(cards)
		if card and target then
			for _, friend in ipairs(new_friends) do
				if target:objectName() == friend:objectName() then
					return friend, card:getEffectiveId()
				end
			end
		end
		self:sort(new_friends, "defense")
		self:sortByKeepValue(cards, true)
		return new_friends[1], cards[1]:getEffectiveId()
	else
		return nil, -1
	end

end

sgs.ai_need_damaged.yiji = function (self, attacker, player)
	if not player:hasSkill("yiji") then return end
	local need_card = false
	local current = self.room:getCurrent()
	if self:hasCrossbowEffect(current) or current:hasSkill("paoxiao") or current:hasFlag("shuangxiong") then need_card = true end
	if current:hasSkills("jieyin|jijiu") and self:getOverflow(current) <= 0 then need_card = true end
	if self:isFriend(current, player) and need_card then return true end

	self:sort(self.friends, "hp")

	if #self.friends > 0 and self.friends[1]:objectName() == player:objectName() and self:isWeak(player) and getCardsNum("Peach", player, (attacker or self.player)) == 0 then return false end
	if #self.friends > 1 and self:isWeak(self.friends[2]) then return true end

	return player:getHp() > 2 and sgs.turncount > 2 and #self.friends > 1
end

sgs.ai_view_as.qingguo = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:isBlack() and card_place == sgs.Player_PlaceHand then
		return ("jink:qingguo[%s:%s]=%d&qingguo"):format(suit, number, card_id)
	end
end

function sgs.ai_cardneed.qingguo(to, card)
	return to:getCards("h"):length() < 2 and card:isBlack()
end

sgs.ai_skill_invoke.luoshen = function(self, data)
	if self:willSkipPlayPhase() then
		local erzhang = self.room:findPlayerBySkillName("guzheng")
		if erzhang and self:isEnemy(erzhang) then return false end
	end
	return true
end

sgs.qingguo_suit_value = {
	spade = 4.1,
	club = 4.2
}

sgs.ai_suit_priority.qingguo= "diamond|heart|club|spade"

sgs.ai_skill_use["@@shensu1"] = function(self, prompt)
	self:updatePlayers()
	self:sort(self.enemies, "defenseSlash")
	if self.player:containsTrick("lightning") and self.player:getCards("j"):length() == 1
		and self:hasWizard(self.friends) and not self:hasWizard(self.enemies, true) then
		return "."
	end

	local selfSub = self.player:getHp() - self.player:getHandcardNum()
	local selfDef = sgs.getDefense(self.player)

	for _, enemy in ipairs(self.enemies) do
		local def = sgs.getDefenseSlash(enemy, self)
		local slash = sgs.Sanguosha:cloneCard("slash")
		local eff = self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self)

		if not self.player:canSlash(enemy, slash, false) then
		elseif self:slashProhibit(nil, enemy) then
		elseif def < 5 and eff then return "@ShensuCard=.&shensu->" .. enemy:objectName()

		elseif selfSub >= 2 then return "."
		elseif selfDef < 6 then return "." end
	end

	for _, enemy in ipairs(self.enemies) do
		local def = sgs.getDefense(enemy)
		local slash = sgs.Sanguosha:cloneCard("slash")
		local eff = self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self)

		if not self.player:canSlash(enemy, slash, false) then
		elseif self:slashProhibit(nil, enemy) then
		elseif eff and def < 8 then return "@ShensuCard=.&shensu->" .. enemy:objectName()
		else return "." end
	end
	return "."
end

sgs.ai_get_cardType = function(card)
	if card:isKindOf("Weapon") then return 1 end
	if card:isKindOf("Armor") then return 2 end
	if card:isKindOf("DefensiveHorse") then return 3 end
	if card:isKindOf("OffensiveHorse") then return 4 end
end

sgs.ai_skill_use["@@shensu2"] = function(self, prompt, method)
	self:updatePlayers()
	self:sort(self.enemies, "defenseSlash")

	local selfSub = self.player:getHp() - self.player:getHandcardNum()

	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)

	local eCard
	local hasCard = { 0, 0, 0, 0 }

	if self:needToThrowArmor() and not self.player:isCardLimited(self.player:getArmor(), method) then
		eCard = self.player:getArmor()
	end

	if not eCard then
		for _, card in ipairs(cards) do
			if card:isKindOf("EquipCard") then
				hasCard[sgs.ai_get_cardType(card)] = hasCard[sgs.ai_get_cardType(card)] + 1
			end
		end

		for _, card in ipairs(cards) do
			if card:isKindOf("EquipCard") and hasCard[sgs.ai_get_cardType(card)] > 1 then
				eCard = card
				break
			end
		end

		if not eCard then
			for _, card in ipairs(cards) do
				if card:isKindOf("EquipCard") and sgs.ai_get_cardType(card) > 3 and not self.player:isCardLimited(card, method) then
					eCard = card
					break
				end
			end
		end
		if not eCard then
			for _, card in ipairs(cards) do
				if card:isKindOf("EquipCard") and not card:isKindOf("Armor") and not self.player:isCardLimited(card, method) then
					eCard = card
					break
				end
			end
		end
	end

	if not eCard then return "." end

	local effectslash, best_target, target, throw_weapon
	local defense = 6
	local weapon = self.player:getWeapon()
	if weapon and eCard:getId() == weapon:getId() and (eCard:isKindOf("Fan") or eCard:isKindOf("QinggangSword")) then throw_weapon = true end

	for _, enemy in ipairs(self.enemies) do
		local def = sgs.getDefense(enemy)
		local slash = sgs.Sanguosha:cloneCard("slash")
		local eff = self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self)

		if not self.player:canSlash(enemy, slash, false) then
		elseif throw_weapon and enemy:hasArmorEffect("Vine") then
		elseif self:slashProhibit(nil, enemy) then
		elseif eff then
			if enemy:getHp() == 1 and getCardsNum("Jink", enemy, self.player) == 0 then
				best_target = enemy
				break
			end
			if def < defense then
				best_target = enemy
				defense = def
			end
			target = enemy
		end
		if selfSub < 0 then return "." end
	end

	if best_target then return "@ShensuCard=" .. eCard:getEffectiveId() .. "&shensu->" .. best_target:objectName() end
	if target then return "@ShensuCard=" .. eCard:getEffectiveId() .. "&shensu->" .. target:objectName() end

	return "."
end

sgs.ai_cardneed.shensu = function(to, card, self)
	return card:getTypeId() == sgs.Card_TypeEquip and getKnownCard(to, self.player, "EquipCard", false) < 2
end

sgs.ai_card_intention.ShensuCard = sgs.ai_card_intention.Slash

sgs.shensu_keep_value = sgs.xiaoji_keep_value


local function card_for_qiaobian(self, who, return_prompt)
	local card, target
	if self:isFriend(who) then
		local judges = who:getJudgingArea()
		if not judges:isEmpty() then
			for _, judge in sgs.qlist(judges) do
				card = sgs.Sanguosha:getCard(judge:getEffectiveId())
				for _, enemy in ipairs(self.enemies) do
					if not enemy:containsTrick(judge:objectName()) and not self.room:isProhibited(self.player, enemy, judge) then
						target = enemy
						break
					end
					if target then break end
				end
			end
		end

		local equips = who:getCards("e")
		local weak = false
		if not target and not equips:isEmpty() and who:hasSkills(sgs.lose_equip_skill) then
			for _, equip in sgs.qlist(equips) do
				if equip:isKindOf("OffensiveHorse") then card = equip break
				elseif equip:isKindOf("Weapon") then card = equip break
				elseif equip:isKindOf("DefensiveHorse") and not self:isWeak(who) then
					card = equip
					break
				elseif equip:isKindOf("Armor") and (not self:isWeak(who) or self:needToThrowArmor(who)) then
					card = equip
					break
				end
			end

			if card then
				if card:isKindOf("Armor") or card:isKindOf("DefensiveHorse") then
					self:sort(self.friends, "defense")
				else
					self:sort(self.friends, "handcard")
					self.friends = sgs.reverse(self.friends)
				end
				for _, friend in ipairs(self.friends) do
					if not self:getSameEquip(card, friend) and friend:objectName() ~= who:objectName()
						and friend:hasSkills(sgs.need_equip_skill .. "|" .. sgs.lose_equip_skill) then
							target = friend
							break
					end
				end
				for _, friend in ipairs(self.friends) do
					if not self:getSameEquip(card, friend) and friend:objectName() ~= who:objectName() then
						target = friend
						break
					end
				end
			end
		end
	else
		local judges = who:getJudgingArea()

		if card == nil or target == nil then
			if not who:hasEquip() or who:hasSkills(sgs.lose_equip_skill) then return nil end
			local card_id = self:askForCardChosen(who, "e", "snatch")
			if card_id >= 0 and who:hasEquip(sgs.Sanguosha:getCard(card_id)) then card = sgs.Sanguosha:getCard(card_id) end
			if card then
				if card:isKindOf("Armor") or card:isKindOf("DefensiveHorse") then
					self:sort(self.friends, "defense")
				else
					self:sort(self.friends, "handcard")
					self.friends = sgs.reverse(self.friends)
				end
				for _, friend in ipairs(self.friends) do
					if not self:getSameEquip(card, friend) and friend:objectName() ~= who:objectName() and friend:hasSkills(sgs.lose_equip_skill .. "|shensu") then
						target = friend
						break
					end
				end
				for _, friend in ipairs(self.friends) do
					if not self:getSameEquip(card, friend) and friend:objectName() ~= who:objectName() then
						target = friend
						break
					end
				end
			end
		end
	end

	if return_prompt == "card" then return card
	elseif return_prompt == "target" then return target
	else
		return (card and target)
	end
end

sgs.ai_skill_cardchosen.qiaobian = function(self, who, flags)
	if flags == "ej" then
		return card_for_qiaobian(self, who, "card")
	end
end

sgs.ai_skill_playerchosen.qiaobian = function(self, targets)
	local who = self.room:getTag("QiaobianTarget"):toPlayer()
	if who then
		if not card_for_qiaobian(self, who, "target") then self.room:writeToConsole("NULL") end
		return card_for_qiaobian(self, who, "target")
	end
end

sgs.ai_skill_discard.qiaobian = function(self, discard_num, min_num, optional, include_equip)
	local current_phase = self.player:getMark("qiaobianPhase")
	local to_discard = {}
	self:updatePlayers()
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	local stealer
	for _, ap in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if ap:hasSkill("tuxi") and self:isEnemy(ap) then stealer = ap end
	end
	local card
	for i = 1, #cards, 1 do
		local isPeach = cards[i]:isKindOf("Peach")
		if isPeach then
			if stealer and self.player:getHandcardNum() <= 2 and self.player:getHp() > 2 and not stealer:containsTrick("supply_shortage") then
				card = cards[i]
				break
			end
			local to_discard_peach = true
			for _,fd in ipairs(self.friends) do
				if fd:getHp() <= 2 and not fd:hasSkill("niepan") then
					to_discard_peach = false
				end
			end
			if to_discard_peach then
				card = cards[i]
				break
			end
		else
			card = cards[i]
			break
		end
	end
	if not card then return {} end
	table.insert(to_discard, card:getEffectiveId())

	if current_phase == sgs.Player_Judge and not self.player:isSkipped(sgs.Player_Judge) then
		if (self.player:containsTrick("lightning") and not self:hasWizard(self.friends) and self:hasWizard(self.enemies))
			or (self.player:containsTrick("lightning") and #self.friends > #self.enemies) then
			return to_discard
		elseif self.player:containsTrick("supply_shortage") then
			if self.player:getHp() > self.player:getHandcardNum() then return to_discard end
			local cardstr = sgs.ai_skill_use["@@tuxi"](self, "@tuxi")
			if cardstr:match("->") then
				local targetstr = cardstr:split("->")[2]
				local targets = targetstr:split("+")
				if #targets == 2 then
					return to_discard
				end
			end
		elseif self.player:containsTrick("indulgence") then
			if self.player:getHandcardNum() > 3 or self.player:getHandcardNum() > self.player:getHp() - 1 then return to_discard end
			for _, friend in ipairs(self.friends_noself) do
				if (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
					return to_discard
				end
			end
		end
	elseif current_phase == sgs.Player_Draw and not self.player:isSkipped(sgs.Player_Draw) and not self.player:hasSkill("tuxi") then
		self.qiaobian_draw_targets = {}
		local cardstr = sgs.ai_skill_use["@@tuxi"](self, "@tuxi")
		if cardstr:match("->") then
			local targetstr = cardstr:split("->")[2]
			local targets = targetstr:split("+")
			if #targets == 2 then
				table.insert(self.qiaobian_draw_targets, targets[1])
				table.insert(self.qiaobian_draw_targets, targets[2])
				return to_discard
			end
		end
		return {}
	elseif current_phase == sgs.Player_Play and not self.player:isSkipped(sgs.Player_Play) then
		self:sortByKeepValue(cards)
		table.remove(to_discard)
		table.insert(to_discard, cards[1]:getEffectiveId())

		self:sort(self.enemies, "defense")
		self:sort(self.friends, "defense")
		self:sort(self.friends_noself, "defense")

		for _, friend in ipairs(self.friends) do
			if not friend:getCards("j"):isEmpty() and card_for_qiaobian(self, friend, ".") then
				return to_discard
			end
		end

		for _, friend in ipairs(self.friends_noself) do
			if not friend:getCards("e"):isEmpty() and friend:hasSkills(sgs.lose_equip_skill) and card_for_qiaobian(self, friend, ".") then
				return to_discard
			end
		end

		local top_value = 0
		for _, hcard in ipairs(cards) do
			if not hcard:isKindOf("Jink") then
				if self:getUseValue(hcard) > top_value then top_value = self:getUseValue(hcard) end
			end
		end
		if top_value >= 3.7 and #(self:getTurnUse()) > 0 then return {} end

		local targets = {}
		for _, enemy in ipairs(self.enemies) do
			if not enemy:hasSkills(sgs.lose_equip_skill) and card_for_qiaobian(self, enemy, ".") then
				table.insert(targets, enemy)
			end
		end

		if #targets > 0 then
			return to_discard
		end
	elseif current_phase == sgs.Player_Discard and not self.player:isSkipped(sgs.Player_Discard) then
		self.player:setFlags("AI_ConsideringQiaobianSkipDiscard")
		self:sortByKeepValue(cards)
		if self:getOverflow(false, true) > 1 then
			self.player:setFlags("-AI_ConsideringQiaobianSkipDiscard")
			return { cards[1]:getEffectiveId() }
		end
		self.player:setFlags("-AI_ConsideringQiaobianSkipDiscard")
	end

	return {}
end

sgs.ai_skill_use["@@qiaobian"] = function(self, prompt)
	self:updatePlayers()
	local QBCard = "@QiaobianCard=.&qiaobian->"
	if prompt == "@qiaobian-2" then
		if #self.qiaobian_draw_targets == 2 then
			return QBCard .. table.concat(self.qiaobian_draw_targets, "+")
		end
		return "."
	end

	if prompt == "@qiaobian-3" then
		-- if self.player:getHandcardNum()-2 > self.player:getHp() then return "." end

		self:sort(self.enemies, "defense")
		for _, friend in ipairs(self.friends) do
			if not friend:getCards("j"):isEmpty() and card_for_qiaobian(self, friend, ".") then
				return QBCard .. friend:objectName()
			end
		end

		for _, friend in ipairs(self.friends_noself) do
			if not friend:getCards("e"):isEmpty() and friend:hasSkills(sgs.lose_equip_skill) and card_for_qiaobian(self, friend, ".") then
				return QBCard .. friend:objectName()
			end
			if not friend:getArmor() then has_armor = false end
		end

		local cards = sgs.QList2Table(self.player:getHandcards())
		local top_value = 0
		for _, hcard in ipairs(cards) do
			if not hcard:isKindOf("Jink") then
				if self:getUseValue(hcard) > top_value then
					top_value = self:getUseValue(hcard)
				end
			end
		end
		if top_value >= 3.7 and #(self:getTurnUse()) > 0 then return "." end

		local targets = {}
		for _, enemy in ipairs(self.enemies) do
			if card_for_qiaobian(self, enemy, ".") then
				table.insert(targets, enemy)
			end
		end

		if #targets > 0 then
			self:sort(targets, "defense")
			return QBCard .. targets[#targets]:objectName()
		end
	end

	return "."
end

function sgs.ai_cardneed.qiaobian(to, card)
	return to:getHandcardNum() <= 2
end


duanliang_skill = {}
duanliang_skill.name = "duanliang"
table.insert(sgs.ai_skills, duanliang_skill)
duanliang_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)

	local card

	self:sortByUseValue(cards,true)

	for _,acard in ipairs(cards)  do
		if acard:isBlack() and (acard:isKindOf("BasicCard") or acard:isKindOf("EquipCard")) and (self:getDynamicUsePriority(acard) < sgs.ai_use_value.SupplyShortage) then
			card = acard
			break
		end
	end

	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("supply_shortage:duanliang[%s:%s]=%d%s"):format(suit, number, card_id, "&duanliang")
	local skillcard = sgs.Card_Parse(card_str)

	assert(skillcard)
	return skillcard
end

sgs.ai_cardneed.duanliang = function(to, card, self)
	return card:isBlack() and card:getTypeId() ~= sgs.Card_TypeTrick and getKnownCard(to, self.player, "black", false) < 2
end

sgs.duanliang_suit_value = {
	spade = 3.9,
	club = 3.9
}
sgs.ai_suit_priority.duanliang= "club|spade|diamond|heart"

function sgs.ai_skill_invoke.jushou(self, data)
	if not self.player:faceUp() then return true end
	for _, friend in ipairs(self.friends) do
		if friend:hasSkill("fangzhu") then return true end
	end
	return self:isWeak()
end


local qiangxi_skill = {}
qiangxi_skill.name = "qiangxi"
table.insert(sgs.ai_skills, qiangxi_skill)
qiangxi_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("QiangxiCard") then
		return sgs.Card_Parse("@QiangxiCard=.&qiangxi")
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
					use.card = sgs.Card_Parse("@QiangxiCard=" .. hand_weapon:getId() .. "&qiangxi")
					if use.to then
						use.to:append(enemy)
					end
					break
				end
				if self.player:distanceTo(enemy) <= 1 then
					use.card = sgs.Card_Parse("@QiangxiCard=" .. weapon:getId() .. "&qiangxi")
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
					use.card = sgs.Card_Parse("@QiangxiCard=.&qiangxi")
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


local quhu_skill = {}
quhu_skill.name = "quhu"
table.insert(sgs.ai_skills, quhu_skill)
quhu_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("QuhuCard") and not self.player:isKongcheng() then return sgs.Card_Parse("@QuhuCard=.&quhu") end
end

sgs.ai_skill_use_func.QuhuCard = function(QHCard, use, self)
	if #self.enemies == 0 then return end
	local max_card = self:getMaxCard()
	local max_point = max_card:getNumber()
	if self.player:hasSkill("yingyang") then max_point = math.min(max_point + 3, 13) end
	self:sort(self.enemies, "handcard")

	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() > self.player:getHp() and not enemy:isKongcheng() then
			local enemy_max_card = self:getMaxCard(enemy)
			local enemy_number = enemy_max_card and enemy_max_card:getNumber() or 0
			if enemy_max_card and enemy:hasSkill("yingyang") then enemy_number = math.min(enemy_number + 3, 13) end
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
						use.card = QHCard
						if use.to then use.to:append(enemy) end
						return
					end
				end
			end
		end
	end
	if not self.player:isWounded() or (self.player:getHp() == 1 and self:getCardsNum("Analeptic") > 0 and self.player:getHandcardNum() >= 2)
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
				if not enemy:isKongcheng() and self.player:getHp() < enemy:getHp() then
					local cards = self.player:getHandcards()
					cards = sgs.QList2Table(cards)
					self:sortByUseValue(cards, true)
					self.quhu_card = cards[1]:getEffectiveId()
					use.card = QHCard
					if use.to then use.to:append(enemy) end
					return
				end
			end
		end
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
	local selected_target = self.player:getTag("jieming_target"):toStringList()
	
	for _, player in ipairs(self.friends) do
		if player:isAlive() and not table.contains(selected_target, player:objectName()) then
			table.insert(friends, player)
		end
	end
	self:sort(friends)

	local max_x = 0
	local target

	local CP = self.room:getCurrent()
	local max_x = 0
	local AssistTarget = self:AssistTarget()
	for _, friend in ipairs(friends) do
		local x = math.min(friend:getMaxHp(), 5) - friend:getHandcardNum()
		if self:hasCrossbowEffect(CP) then x = x + 1 end
		if AssistTarget and friend:objectName() == AssistTarget:objectName() then x = x + 0.5 end

		if x > max_x and friend:isAlive() then
			max_x = x
			target = friend
		end
	end

	return target
end

sgs.ai_need_damaged.jieming = function(self, attacker, player)
	return player:hasSkill("jieming") and self:getJiemingChaofeng(player) <= -6
end

sgs.ai_playerchosen_intention.jieming = function(self, from, to)
	if to:getHandcardNum() < math.min(5, to:getMaxHp()) then
		sgs.updateIntention(from, to, -80)
	end
end

sgs.ai_skill_invoke.xingshang = true

function SmartAI:toTurnOver(player, n, reason) -- @todo: param of toTurnOver
	if not player then global_room:writeToConsole(debug.traceback()) return end
	n = n or 0
	if reason and reason == "fangzhu" and player:getHp() == 1 and sgs.ai_AOE_data then
		local use = sgs.ai_AOE_data:toCardUse()
		if use.to:contains(player) and self:aoeIsEffective(use.card, player)
			and self:playerGetRound(player) > self:playerGetRound(self.player)
			and player:isKongcheng() then
			return false
		end
	end
	if n > 1 and player:hasSkill("jijiu") then return false end
	if player:hasSkill("jushou") and player:getPhase() <= sgs.Player_Finish then return false end
	if not player:faceUp() then return false end
	return true
end

sgs.ai_skill_playerchosen.fangzhu = function(self, targets)
	self:updatePlayers()
	self:sort(self.friends_noself, "handcard")
	local target = nil
	local n = self.player:getLostHp()
	for _, friend in ipairs(self.friends_noself) do
		if not self:toTurnOver(friend, n, "fangzhu") then
			target = friend
			break
		end
	end

	if not target then
		if n >= 3 then
			target = self:findPlayerToDraw(false, n)
			if not target then
				for _, enemy in ipairs(self.enemies) do
					if self:toTurnOver(enemy, n, "fangzhu") then
						target = enemy
						break
					end
				end
			end
		else
			self:sort(self.enemies)
			for _, enemy in ipairs(self.enemies) do
				if self:toTurnOver(enemy, n, "fangzhu") then
					target = enemy
					break
				end
			end
			if not target then
				for _, enemy in ipairs(self.enemies) do
					if self:toTurnOver(enemy, n, "fangzhu") and enemy:hasSkills(sgs.priority_skill) then
						target = enemy
						break
					end
				end
			end
			if not target then
				for _, enemy in ipairs(self.enemies) do
					if self:toTurnOver(enemy, n, "fangzhu") then
						target = enemy
						break
					end
				end
			end
		end
	end
	return target
end

sgs.ai_playerchosen_intention.fangzhu = function(self, from, to)
	local intention = 80 / math.max(from:getLostHp(), 1)
	if not self:toTurnOver(to, from:getLostHp()) then intention = -intention end
	if from:getLostHp() < 3 then
		sgs.updateIntention(from, to, intention)
	else
		sgs.updateIntention(from, to, math.min(intention, -30))
	end
end

sgs.ai_need_damaged.fangzhu = function (self, attacker, player)
	if not player:hasSkill("fangzhu") then return end
	local enemies = self:getEnemies(player)
	if #enemies < 1 then return false end
	self:sort(enemies, "defense")
	for _, enemy in ipairs(enemies) do
		if player:getLostHp() < 1 and self:toTurnOver(enemy, player:getLostHp() + 1) then
			return true
		end
	end
	local friends = self:getFriendsNoself(player)
	self:sort(friends)
	for _, friend in ipairs(friends) do
		if not self:toTurnOver(friend, player:getLostHp() + 1) then return true end
	end
	return false
end

sgs.ai_skill_cardask["@xiaoguo"] = function(self, data)
	local currentplayer = self.room:getCurrent()

	local has_analeptic, has_slash, has_jink
	for _, acard in sgs.qlist(self.player:getHandcards()) do
		if acard:isKindOf("Analeptic") then has_analeptic = acard
		elseif acard:isKindOf("Slash") then has_slash = acard
		elseif acard:isKindOf("Jink") then has_jink = acard
		end
	end

	local card

	if has_slash then card = has_slash
	elseif has_jink then card = has_jink
	elseif has_analeptic then
		if (getCardsNum("EquipCard", currentplayer, self.player) == 0 and not self:isWeak()) or self:getCardsNum("Analeptic") > 1 then
			card = has_analeptic
		end
	end

	if not card then return "." end
	if self:isFriend(currentplayer) then
		if self:needToThrowArmor(currentplayer) then
			if card:isKindOf("Slash") or (card:isKindOf("Jink") and self:getCardsNum("Jink") > 1) then
				return "$" .. card:getEffectiveId()
			else return "."
			end
		end
	elseif self:isEnemy(currentplayer) then
		if not self:damageIsEffective(currentplayer) then return "." end
		if self:getDamagedEffects(currentplayer) or self:needToLoseHp(currentplayer, self.player) then return "." end
		if self:needToThrowArmor() then return "." end
		if currentplayer:hasSkills(sgs.lose_equip_skill) and currentplayer:getCards("e"):length() > 0 then return "." end
		return "$" .. card:getEffectiveId()
	end
	return "."
end

sgs.ai_choicemade_filter.cardResponded["@xiaoguo"] = function(self, player, promptlist)
	if promptlist[#promptlist] ~= "_nil_" then
		local current = self.room:getCurrent()
		if not current then return end
		local intention = 10
		if current:hasSkills(sgs.lose_equip_skill) and current:getCards("e"):length() > 0 then intention = 0 end
		if self:needToThrowArmor(current) then return end
		sgs.updateIntention(player, current, intention)
	end
end

sgs.ai_skill_cardask["@xiaoguo-discard"] = function(self, data)
	local yuejin = self.room:findPlayerBySkillName("xiaoguo")
	local player = self.player

	if self:needToThrowArmor() then
		return "$" .. player:getArmor():getEffectiveId()
	end

	if not self:damageIsEffective(player, sgs.DamageStruct_Normal, yuejin) then
		return "."
	end
	if self:getDamagedEffects(self.player, yuejin) then
		return "."
	end
	if self:needToLoseHp(player, yuejin) then
		return "."
	end

	local card_id
	if self.player:hasSkills(sgs.lose_equip_skill) then
		if player:getWeapon() then card_id = player:getWeapon():getId()
		elseif player:getOffensiveHorse() then card_id = player:getOffensiveHorse():getId()
		elseif player:getArmor() then card_id = player:getArmor():getId()
		elseif player:getDefensiveHorse() then card_id = player:getDefensiveHorse():getId()
		end
	end

	if not card_id then
		for _, card in sgs.qlist(player:getCards("h")) do
			if card:isKindOf("EquipCard") then
				card_id = card:getEffectiveId()
				break
			end
		end
	end

	if not card_id then
		if player:getWeapon() then card_id = player:getWeapon():getId()
		elseif player:getOffensiveHorse() then card_id = player:getOffensiveHorse():getId()
		elseif self:isWeak(player) and player:getArmor() then card_id = player:getArmor():getId()
		elseif self:isWeak(player) and player:getDefensiveHorse() then card_id = player:getDefensiveHorse():getId()
		end
	end

	if not card_id then return "." else return "$" .. card_id end
end

sgs.ai_cardneed.xiaoguo = function(to, card)
	return getKnownCard(to, global_room:getCurrent(), "BasicCard", true) == 0 and card:getTypeId() == sgs.Card_TypeBasic
end
