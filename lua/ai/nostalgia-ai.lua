sgs.weapon_range.MoonSpear = 3
sgs.ai_use_priority.MoonSpear = 2.635

local nosfanjian_skill = {}
nosfanjian_skill.name = "nosfanjian"
table.insert(sgs.ai_skills, nosfanjian_skill)
nosfanjian_skill.getTurnUseCard = function(self)
	if self.player:isKongcheng() then return nil end
	if self.player:usedTimes("NosFanjianCard") > 0 then return nil end

	local cards = self.player:getHandcards()
	
	for _, card in sgs.qlist(cards) do
		if card:getSuit() == sgs.Card_Diamond and self.player:getHandcardNum() == 1 then
			return nil
		elseif card:isKindOf("Peach") or card:isKindOf("Analeptic") then
			return nil
		end
	end
	
	local card_str = "@NosFanjianCard=."
	local fanjianCard = sgs.Card_Parse(card_str)
	assert(fanjianCard)

	return fanjianCard
end

sgs.ai_skill_use_func.NosFanjianCard = sgs.ai_skill_use_func.FanjianCard

sgs.ai_card_intention.NosFanjianCard = sgs.ai_card_intention.FanjianCard

sgs.dynamic_value.damage_card.NosFanjianCard = true

sgs.ai_chaofeng.noszhouyu = sgs.ai_chaofeng.zhouyu

nosjujian_skill = {}
nosjujian_skill.name = "nosjujian"
table.insert(sgs.ai_skills, nosjujian_skill)
nosjujian_skill.getTurnUseCard = function(self)
	if self:needBear() then return end
	if not self.player:hasUsed("NosJujianCard") then return sgs.Card_Parse("@NosJujianCard=.") end
end

sgs.ai_skill_use_func.NosJujianCard = function(card, use, self)
	local abandon_card = {}
	local index = 0
	local hasPeach = (self:getCardsNum("Peach") > 0)
	local to
	local AssistTarget = self:AssistTarget()

	local trick_num, basic_num, equip_num = 0, 0, 0
	if not hasPeach and self.player:isWounded() and self.player:getCards("he"):length() >=3 then
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		for _, card in ipairs(cards) do
			if card:getTypeId() == sgs.Card_Trick and not isCard("ExNihilo", card, self.player) then trick_num = trick_num + 1
			elseif card:getTypeId() == sgs.Card_Basic then basic_num = basic_num + 1
			elseif card:getTypeId() == sgs.Card_Equip then equip_num = equip_num + 1
			end
		end
		local result_class
		if trick_num >= 3 then result_class = "TrickCard"
		elseif equip_num >= 3 then result_class = "EquipCard"
		elseif basic_num >= 3 then result_class = "BasicCard"
		end

		for _, fcard in ipairs(cards) do
			if fcard:isKindOf(result_class) and not isCard("ExNihilo", fcard, self.player) then
				table.insert(abandon_card, fcard:getId())
				index = index + 1
				if index == 3 then break end
			end
		end

		if index == 3 then
			if AssistTarget and not AssistTarget:hasSkill("manjuan") then
				to = AssistTarget
			else
				to = self:findPlayerToDraw("noself", 3)
			end
			if not to then return end
			if use.to then use.to:append(to) end
			use.card = sgs.Card_Parse("@NosJujianCard=" .. table.concat(abandon_card, "+"))
			return
		end
	end
	
	abandon_card = {}
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local slash_num = self:getCardsNum("Slash")
	local jink_num = self:getCardsNum("Jink")
	index = 0
	for _, card in ipairs(cards) do
		if index >= 3 then break end
		if card:isKindOf("TrickCard") and not card:isKindOf("Nullification") then
			table.insert(abandon_card, card:getId())
			index = index + 1
		elseif card:isKindOf("EquipCard") then
			table.insert(abandon_card, card:getId())
			index = index + 1
		elseif card:isKindOf("Slash") then
			table.insert(abandon_card, card:getId())
			index = index + 1
			slash_num = slash_num - 1
		elseif card:isKindOf("Jink") and jink_num > 1 then
			table.insert(abandon_card, card:getId())
			index = index + 1
			jink_num = jink_num - 1
		end
	end
	
	if index == 3 then
		if AssistTarget and not AssistTarget:hasSkill("manjuan") then
			to = AssistTarget
		else
			to = self:findPlayerToDraw("noself", 3)
		end
		if not to then return end
		if use.to then use.to:append(to) end
		use.card = sgs.Card_Parse("@NosJujianCard=" .. table.concat(abandon_card, "+"))
		return
	end

	if self:getOverflow() > 0 then
		local getOverflow = math.max(self:getOverflow(), 0)
		local discard = self:askForDiscard("dummyreason", math.min(getOverflow, 3), nil, false, true)
		if AssistTarget and not AssistTarget:hasSkill("manjuan") and not self:needKongcheng(AssistTarget, true) then
			to = AssistTarget
		else
			to = self:findPlayerToDraw("noself", math.min(getOverflow, 3))
		end 
		if not to then return end
		use.card = sgs.Card_Parse("@NosJujianCard=" .. table.concat(discard, "+"))
		if use.to then use.to:append(to) end
		return
	end

	if index > 0 then
		if AssistTarget and not AssistTarget:hasSkill("manjuan") and not self:needKongcheng(AssistTarget, true) then
			to = AssistTarget
		else
			to = self:findPlayerToDraw("noself", index)
		end  
		if not to then return end
		use.card = sgs.Card_Parse("@NosJujianCard=" .. table.concat(abandon_card, "+"))
		if use.to then use.to:append(to) end
		return
	end
end

sgs.ai_use_priority.NosJujianCard = 0
sgs.ai_use_value.NosJujianCard = 6.7

sgs.ai_card_intention.NosJujianCard = -100

sgs.dynamic_value.benefit.NosJujianCard = true

sgs.ai_skill_cardask["@enyuanheart"] = function(self, data)
	local damage = data:toDamage()
	if self:needToLoseHp(self.player, damage.to, nil, true) and not self:hasSkills(sgs.masochism_skill) then return "." end
	if self:isFriend(damage.to) then return end
	if self:needToLoseHp() and not self:hasSkills(sgs.masochism_skill) then return "." end

	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getSuit() == sgs.Card_Heart and not isCard("Peach", card, self.player) and not isCard("ExNihilo", card, self.player) then
			return card:getEffectiveId()
		end
	end
	return "."
end

function sgs.ai_slash_prohibit.nosenyuan(self, to, card, from)
	if from:hasSkill("jueqing") then return false end
	if from:hasSkill("nosqianxi") and from:distanceTo(to) == 1 then return false end
	if from:hasFlag("nosjiefanUsed") then return false end
	if self:needToLoseHp(from) and not self:hasSkills(sgs.masochism_skill, from) then return false end
	if from:getHp() > 3 then return false end
	
	local n = 0
	local cards = from:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getSuit() == sgs.Card_Heart and not isCard("Peach", card, from) and not isCard("ExNihilo", card, from) then
			if not card:isKindOf("Slash") then return false end
			n = n + 1
		end
	end
	if n < 1 then return true end
	if n > 1 then return false end
	if n == 1 then return card:getSuit() == sgs.Card_Heart end
	return self:isWeak(from)
end

sgs.ai_need_damaged.nosenyuan = function (self, attacker, player)	
	if player:hasSkill("nosenyuan") and self:isEnemy(attacker, player) and self:isWeak(attacker)
		and not (self:needToLoseHp(attacker) and not self:hasSkills(sgs.masochism_skill, attacker)) then
			return true
	end
	return false
end

nosxuanhuo_skill = {}
nosxuanhuo_skill.name = "nosxuanhuo"
table.insert(sgs.ai_skills, nosxuanhuo_skill)
nosxuanhuo_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("NosXuanhuoCard") then
		return sgs.Card_Parse("@NosXuanhuoCard=.")
	end
end

sgs.ai_skill_use_func.NosXuanhuoCard = function(card, use, self)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)

	local target
	for _, friend in ipairs(self.friends_noself) do
		if self:hasSkills(sgs.lose_equip_skill, friend) and not friend:getEquips():isEmpty() and not friend:hasSkill("manjuan") then
			target = friend
			break	
		end
	end
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if self:getDangerousCard(enemy) then
				target = enemy
				break
			end
		end
	end
	if not target then
		for _, friend in ipairs(self.friends_noself) do
			if self:needToThrowArmor(friend) and not friend:hasSkill("manjuan") then
				target = friend
				break
			end
		end
	end
	if not target then
		self:sort(self.enemies, "handcard")
		for _, enemy in ipairs(self.enemies) do
			if self:getValuableCard(enemy) then
				target = enemy
				break
			end
			if target then break end

			local cards = sgs.QList2Table(enemy:getHandcards())
			local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), enemy:objectName())
			if not enemy:isKongcheng() and not (enemy:hasSkill("tuntian") and enemy:hasSkill("zaoxian")) then
				for _, cc in ipairs(cards) do
					if (cc:hasFlag("visible") or cc:hasFlag(flag)) and (cc:isKindOf("Peach") or cc:isKindOf("Analeptic")) then
						target = enemy
						break
					end
				end
			end
			if target then break end
		end
	end
	if not target then
		for _, friend in ipairs(self.friends_noself) do
			if friend:hasSkill("tuntian") and friend:hasSkill("zaoxian") and not friend:hasSkill("manjuan") then
				target = friend
				break
			end
		end
	end
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isNude() and enemy:hasSkill("manjuan") then
				target = enemy
				break
			end
		end
	end

	if target then
		target:setFlags("AI_NosXuanhuoTarget")
		if self:isFriend(target) then
			for _, card in ipairs(cards) do
				if card:getSuit() == sgs.Card_Heart then
					use.card = sgs.Card_Parse("@NosXuanhuoCard=" .. card:getEffectiveId())
					break
				end
			end
		else
			for _, card in ipairs(cards) do
				if card:getSuit() == sgs.Card_Heart and not isCard("Peach", card, target) and not isCard("Nullification", card, target) then
					use.card = sgs.Card_Parse("@NosXuanhuoCard=" .. card:getEffectiveId())
					break
				end
			end
		end

		if use.to then
			use.to:append(target)
		end
	end
end

sgs.ai_skill_playerchosen.nosxuanhuo = function(self, targets)
	if self:needBear() then return self.player end
	local to = self:findPlayerToDraw("nos_xuanhuo")
	if to then return to end
	return self.player
end

sgs.ai_playerchosen_intention.nosxuanhuo = -10

sgs.nosenyuan_suit_value = {
	heart = 3.9
}

sgs.ai_chaofeng.nosfazheng = -3

sgs.ai_skill_choice.nosxuanfeng = function(self, choices)
	self:sort(self.enemies, "defenseSlash")
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy)<=1 then
			return "damage"
		elseif not self:slashProhibit(slash, enemy) and self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self) then
			return "slash"
		end
	end
	return "nothing"
end

sgs.ai_skill_playerchosen.xuanfeng_damage = sgs.ai_skill_playerchosen.damage
sgs.ai_skill_playerchosen.nosxuanfeng_slash = sgs.ai_skill_playerchosen.zero_card_as_slash

sgs.ai_playerchosen_intention.xuanfeng_damage = 80
sgs.ai_playerchosen_intention.xuanfeng_slash = 80

sgs.nosxuanfeng_keep_value = sgs.xiaoji_keep_value

sgs.ai_skill_invoke.nosshangshi = sgs.ai_skill_invoke.shangshi

sgs.ai_view_as.nosgongqi = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:getTypeId() == sgs.Card_Equip then
		return ("slash:nosgongqi[%s:%s]=%d"):format(suit, number, card_id)
	end
end

local nosgongqi_skill = {}
nosgongqi_skill.name = "nosgongqi"
table.insert(sgs.ai_skills, nosgongqi_skill)
nosgongqi_skill.getTurnUseCard = function(self, inclusive)
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	
	local equip_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards) do
		if card:getTypeId() == sgs.Card_Equip and ((self:getUseValue(card) < sgs.ai_use_value.Slash) or inclusive) then
			equip_card = card
			break
		end
	end

	if equip_card then		
		local suit = equip_card:getSuitString()
		local number = equip_card:getNumberString()
		local card_id = equip_card:getEffectiveId()
		local card_str = ("slash:nosgongqi[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
		
		assert(slash)
		
		return slash
	end
end

sgs.ai_skill_invoke.nosjiefan = function(self, data)
	local dying = data:toDying()
	local slashnum = 0
	local who = dying.who
	local current = self.room:getCurrent()
	sgs.nosjiefancurrent = current
	for _, slash in ipairs(self:getCards("Slash")) do
		if self:slashIsEffective(slash, current) then 
			slashnum = slashnum + 1  
		end 
	end

	local has_slash_prohibit_skill = false
	for _, askill in sgs.qlist(current:getVisibleSkillList()) do
		local filter = sgs.ai_slash_prohibit[askill:objectName()]
		if filter and type(filter) == "function" then
			has_slash_prohibit_skill = true
			break
		end
	end
	if not current or current:isDead() or current:getPhase() == sgs.Player_NotActive
		or current:objectName() == self.player:objectName() or (current:hasSkill("wansha") and self.player:objectName() ~= dying:objectName())
		or (self:isEnemy(current) and self:needLeiji(current, self.player)) then
			return false
	end
	if self:isFriend(who) and not has_slash_prohibit_skill and slashnum > 0 then return true end
end

sgs.ai_skill_cardask["jiefan-slash"] = function(self, data, pattern, target)
	target = global_room:getCurrent()
	if self:isEnemy(target) and self:needLeiji(target, self.player) then return "." end
	for _, slash in ipairs(self:getCards("Slash")) do
		if self:slashIsEffective(slash, target) then 
			return slash:toString()
		end 
	end
	return "."
end


function sgs.ai_cardneed.nosqianxi(to, card)
	return isCard("Slash", card, to) and getKnownCard(to, "Slash", true) == 0
end

sgs.ai_skill_invoke.nosqianxi = function(self, data)
	local damage = data:toDamage()
	local target = damage.to
	if self:isFriend(target) then return false end
	if target:getLostHp() >= 2 and target:getHp() <= 1 then return false end
	if self:hasHeavySlashDamage(self.player, damage.card, target) and target:getHp() <= 1 then return false end
	if self:hasSkills(sgs.masochism_skill, target) or self:hasSkills(sgs.recover_skill, target) or self:hasSkills(sgs.exclusive_skill, target) then return true end
	if self:hasHeavySlashDamage(self.player, damage.card, target) then return false end
	return (target:getMaxHp() - target:getHp()) < 2 
end

sgs.ai_chaofeng.nos_madai = 3

sgs.ai_skill_invoke.nosfuhun = function(self, data)
	if self:needBear() then return false end
	local target = 0
	for _,enemy in ipairs(self.enemies) do
		if (self.player:distanceTo(enemy) <= self.player:getAttackRange())  then target = target + 1 end
	end
	return target > 0 and not self.player:isSkipped(sgs.Player_Play)
end

sgs.ai_chaofeng.nos_guanxingzhangbao = 2

sgs.ai_skill_invoke.noszhenlie = function(self, data)
	local judge = data:toJudge()
	if not judge:isGood() then 
	return true end
	return false
end

sgs.ai_skill_playerchosen.nosmiji = function(self, targets)
	if self:needBear() then return self.player end
	local n = self.player:getLostHp()
	if self.player:getPhase() == sgs.Player_Start then
		if self.player:getHandcardNum() - n < 2 and not self:needKongcheng() and not self:willSkipPlayPhase() then return self.player end
	elseif self.player:getPhase() == sgs.Player_Finish then
		if self.player:getHandcardNum() - n < 2 and not self:needKongcheng() then return self.player end
	end
	local to = self:findPlayerToDraw("all", n)
	if to then return to end
	return self.player
end

sgs.ai_playerchosen_intention.nosmiji = -80

sgs.ai_chaofeng.nos_wangyi = -2
