sgs.ai_skill_cardask["@xiaoguo"] = function(self, data)
	local currentplayer
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:getPhase() ~= sgs.Player_NotActive then
			currentplayer = player
			break
		end
	end
	
	local has_anal, has_slash, has_jink
	for _, acard in sgs.qlist(self.player:getHandcards()) do
		if acard:isKindOf("Analeptic") then has_anal = acard
		elseif acard:isKindOf("Slash") then has_slash = acard
		elseif acard:isKindOf("Jink") then has_jink = acard
		end
	end
	
	local card

	if has_slash then card = has_slash
	elseif has_jink then card = has_jink
	elseif has_anal then
		if (getCardsNum("EquipCard", currentplayer) == 0 and not self:isWeak()) or self:getCardsNum("Analeptic") > 1 then
			card = has_anal
		end
	end

	if not card then return "." end
	if self:isFriend(currentplayer) then
		if currentplayer:hasArmorEffect("SilverLion") and currentplayer:isWounded() and self:isWeak(currentplayer) then 
			if card:isKindOf("Slash") or (card:isKindOf("Jink") and self:getCardsNum("Jink") > 1) then
				return "$" .. card:getEffectiveId()
			else return "."
			end
		end
	elseif self:isEnemy(currentplayer) then
		if not self:damageIsEffective(currentplayer) then return "." end
		if currentplayer:hasArmorEffect("SilverLion") and currentplayer:isWounded() and self:isWeak(currentplayer) then return "." end
		if self:hasSkills(sgs.lose_equip_skill, currentplayer) and currentplayer:getCards("e"):length() > 0 then return "." end
		return "$" .. card:getEffectiveId()
	end
	return "."
end

sgs.ai_choicemade_filter.cardResponsed["@xiaoguo"] = function(player, promptlist)
	if promptlist[#promptlist] ~= "_nil_" then
		local current = player:getRoom():getCurrent()
		if not current then return end
		local intention = 50
		if current:hasArmorEffect("SilverLion") and current:isWounded() and self:isWeak(current) then intention = -30 end
		sgs.updateIntention(player, current, intention)
	end
end

sgs.ai_skill_cardask["@xiaoguo-discard"] = function(self, data)
	local yuejin = self.room:findPlayerBySkillName("xiaoguo")
	local player = self.player

	if player:hasArmorEffect("SilverLion") and player:isWounded() and self:isWeak() then
		return "$" .. player:getArmor():getEffectiveId()
	end

	if not self:damageIsEffective(player, sgs.DamageStruct_Normal, yuejin) then
		return "."
	end

	if self:getDamagedEffects(self.player) then
		return "."
	end

	if player:getHp() > getBestHp(player) then
		return "."
	end

	if player:hasArmorEffect("SilverLion") and player:isWounded() then
		return "$" .. player:getArmor():getEffectiveId()
	end
	
	local card_id
	if self:hasSkills(sgs.lose_equip_skill, player) then
		if player:getWeapon() then card_id = player:getWeapon():getId()
		elseif player:getOffensiveHorse() then card_id = player:getOffensiveHorse():getId()
		elseif player:getDefensiveHorse() then card_id = player:getDefensiveHorse():getId()
		elseif player:getArmor() then card_id = player:getArmor():getId()
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
		elseif player:getDefensiveHorse() then card_id = player:getDefensiveHorse():getId()
		elseif player:getHp() < 4 and player:getArmor() then card_id = player:getArmor():getId()
		end
	end

	if not card_id then
		return "."
	else
		return "$" .. card_id
	end
	return "."
end

sgs.ai_cardneed.xiaoguo = function(to, card)
	return getKnownCard(to, "BasicCard", true) == 0 and card:getTypeId() == sgs.Card_Basic
end

sgs.ai_chaofeng.yuejin = 2

sgs.ai_skill_use["@@shushen"] = function(self, prompt)
	if #self.friends_noself == 0 then return "." end
	local to = player_to_draw(self, "noself")
	if to then return ("@ShushenCard=.->%s"):format(to:objectName()) end
	return "."
end

sgs.ai_card_intention.ShushenCard = -80

sgs.ai_skill_invoke.shenzhi = function(self, data)
	return self.player:getHandcardNum() >= self.player:getHp() and self.player:getHandcardNum() <= self.player:getHp() + math.max(3, self.player:getHp())
			and self.player:getLostHp() > 0 and self:getCardsNum("Peach") == 0
end

function sgs.ai_cardneed.shenzhi(to, card)
	return to:getHandcardNum() < to:getHp()
end

local duoshi_skill = {}
duoshi_skill.name = "duoshi"
table.insert(sgs.ai_skills, duoshi_skill)
duoshi_skill.getTurnUseCard = function(self, inclusive)
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)

	local red_card
	if self.player:getCardCount(false) <= 2 then return end
	if self:needBear() then return end
	self:sortByUseValue(cards, true)

	for _, card in ipairs(cards) do
		if card:isRed() then
			local shouldUse = true
			if card:isKindOf("Slash") then
				local dummy_use = { isDummy = true }
				if self:getCardsNum("Slash") == 1 then
					self:useBasicCard(card, dummy_use)
					if dummy_use.card then shouldUse = false end
				end
			end

			if self:getUseValue(card) > sgs.ai_use_value.DuoshiCard and card:isKindOf("TrickCard") then
				local dummy_use = { isDummy = true }
				self:useTrickCard(card, dummy_use)
				if dummy_use.card then shouldUse = false end
			end

			if shouldUse and not card:isKindOf("Peach") then
				red_card = card
				break
			end

		end
	end

	if red_card then
		local card_id = red_card:getEffectiveId()
		local card_str = ("@DuoshiCard=" .. card_id)
		local await = sgs.Card_Parse(card_str)
		assert(await)
		return await
	end
end

sgs.ai_skill_use_func.DuoshiCard = function(card, use, self)
	use.card = card
	if use.to then use.to:append(self.player) end
	for _, player in ipairs(self.friends) do
		if use.to and not player:hasSkill("manjuan") and player:objectName() ~= self.player:objectName() then
			use.to:append(player)
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if use.to and enemy:hasSkill("manjuan") then
			use.to:append(enemy)
		end
	end
end

sgs.ai_use_value.DuoshiCard = 3
sgs.ai_use_priority.DuoshiCard = 2.2
sgs.ai_card_intention.DuoshiCard = function(card, from, tos, source)
	for _, to in ipairs(tos) do
		sgs.updateIntention(from, to, to:hasSkill("manjuan") and 50 or -50)
	end
end

local fenxun_skill = {}
fenxun_skill.name = "fenxun"
table.insert(sgs.ai_skills, fenxun_skill)
fenxun_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("FenxunCard") then return end
	if self:needBear() then return end
	if not self.player:isNude() then
		local card_id
		local slashcount = self:getCardsNum("Slash")
		local jinkcount = self:getCardsNum("Jink")
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		self:sortByKeepValue(cards)

		if (self.player:hasArmorEffect("SilverLion") and self.player:isWounded())
		  or (self:hasSkills("bazhen|yizhong") and self.player:getArmor()) then
			return sgs.Card_Parse("@FenxunCard=" .. self.player:getArmor():getId())
		elseif self.player:getHandcardNum() > 0 then
			local lightning = self:getCard("Lightning")
			if lightning and not self:willUseLightning(lightning) then
				card_id = lightning:getEffectiveId()
			else
				for _, acard in ipairs(cards) do
					if (acard:isKindOf("AmazingGrace") or acard:isKindOf("EquipCard")) then
						card_id = acard:getEffectiveId()
						break
					end
				end
			end
		elseif jinkcount > 1 then
			for _, acard in ipairs(cards) do
				if acard:isKindOf("Jink") then
					card_id = acard:getEffectiveId()
					break
				end
			end
		elseif slashcount > 1 then
			for _, acard in ipairs(cards) do
				if acard:isKindOf("Slash") then
					slashcount = slashcount - 1
					card_id = acard:getEffectiveId()
					break
				end
			end		
		elseif not self.player:getEquips():isEmpty() then
			local player = self.player
			if player:getWeapon() then card_id = player:getWeapon():getId() end
		end

		if not card_id then
			for _, acard in ipairs(cards) do
				if (acard:isKindOf("Disaster") or acard:isKindOf("AmazingGrace") or acard:isKindOf("EquipCard") or acard:isKindOf("BasicCard"))
					and not isCard("Peach", acard, self.player) and not isCard("Slash", acard, self.player) then
					card_id = acard:getEffectiveId()
					break
				end
			end
		end

		if slashcount > 0 and card_id then
			return sgs.Card_Parse("@FenxunCard=" .. card_id)
		end
	end
	return nil
end

sgs.ai_skill_use_func.FenxunCard = function(card, use, self)
	if not self.player:hasUsed("FenxunCard") then
		self:sort(self.enemies, "defense")
		local target
		for _, enemy in ipairs(self.enemies) do
			if self.player:distanceTo(enemy) > 1 and self.player:canSlash(enemy, nil, false) then
				target = enemy
				break
			end
		end
		if target and self:getCardsNum("Slash") > 0 then
			use.card = card
			if use.to then
				use.to:append(target)
			end
		end
	end
end

sgs.ai_use_value.FenxunCard = 5.5
sgs.ai_use_priority.FenxunCard = 4.1
sgs.ai_card_intention.FenxunCard = 50

sgs.ai_skill_choice.mingshi = function(self, choices, data)
	local damage = data:toDamage()
	return damage.to and self:isFriend(damage.to) and "no" or "yes"
end

sgs.ai_skill_invoke.lirang = function(self, data)
	return #self.friends_noself > 0
end

sgs.ai_skill_use["@@sijian"] = function(self, prompt)
	local to
	to = player_to_discard(self, "noself")
	if to then return ("@SijianCard=.->%s"):format(to:objectName()) end
	return "."
end

sgs.ai_card_intention.SijianCard = function(card, from, tos)
	local intention = 80
	local to = tos[1]
	if to:hasSkill("kongcheng") and to:getHandcardNum() == 1 and to:getHp() <= 2 then
		intention = -30
	end
	if to:hasArmorEffect("SilverLion") and to:isWounded() then
		  intention = -30
	end
	sgs.updateIntention(from, tos[1], intention)
end

sgs.ai_skill_choice.suishi1 = function(self, choices)
	local tianfeng = self.room:findPlayerBySkillName("suishi")
	if tianfeng and self:isFriend(tianfeng) then
		return "draw"
	end
	return "no"
end

sgs.ai_skill_choice.suishi2 = function(self, choices)
	local tianfeng = self.room:findPlayerBySkillName("suishi")
	if tianfeng and self:objectiveLevel(tianfeng) > 3 then
		return "damage"
	end
	return "no"
end

sgs.ai_skill_use["@@shuangren"] = function(self, prompt)
	local target
	self:sort(self.enemies, "handcard")
	local max_card = self:getMaxCard(self.player)
	local max_point = max_card:getNumber()
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isKongcheng() then
			local enemy_max_card = self:getMaxCard(enemy)
			local allknown = 0
			if self:getKnownNum(enemy) == enemy:getHandcardNum() then
				allknown = allknown + 1
			end
			if (enemy_max_card and max_point > enemy_max_card:getNumber() and allknown > 0)
				or (enemy_max_card and max_point > enemy_max_card:getNumber() and allknown < 1 and max_point > 10)
				or (not enemy_max_card and max_point > 10) then
					return "@ShuangrenCard=" .. max_card:getEffectiveId() .. "->" .. enemy:objectName()
			end
		end
	end
	return "."
end

function sgs.ai_skill_pindian.shuangren(minusecard, self, requestor, maxcard)
	local cards = sgs.QList2Table(self.player:getHandcards())
	local function compare_func(a, b)
		return a:getNumber() > b:getNumber()
	end
	table.sort(cards, compare_func)
	for _, card in ipairs(cards) do
		if card:getNumber()> 10 then return card end
	end
	self:sortByKeepValue(cards)
	return cards[1]
end

sgs.ai_chaofeng.jiling = 2
sgs.ai_skill_playerchosen.shuangren_slash = sgs.ai_skill_playerchosen.zero_card_as_slash
sgs.ai_card_intention.ShuangrenCard = 80
sgs.ai_cardneed.shuangren = sgs.ai_cardneed.bignumber

xiongyi_skill = {}
xiongyi_skill.name = "xiongyi"
table.insert(sgs.ai_skills, xiongyi_skill)
xiongyi_skill.getTurnUseCard = function(self)
	if self.player:getMark("@arise") < 1 then return end
	if #self.friends <= #self.enemies and self.player:getLostHp() > 1 then return sgs.Card_Parse("@XiongyiCard=.") end
end

sgs.ai_skill_use_func.XiongyiCard = function(card, use, self)
	use.card = card
	for i = 1, #self.friends - 1 do
		if use.to then use.to:append(self.friends[i]) end
	end
end

sgs.ai_card_intention.XiongyiCard = -80

sgs.ai_skill_invoke.kuangfu = function(self, data)
	local damage = data:toDamage()
	if self:isEnemy(damage.to) then
		if damage.to:getCards("e"):length() == 1 and damage.to:hasArmorEffect("SilverLion") and not IgnoreArmor(damage.from, damage.to)
		  and damage.to:isWounded() and self:isWeak(damage.to) then
			return false
		end
		return true
	end
	if damage.to:getCards("e"):length() == 1 and damage.to:hasArmorEffect("SilverLion") and not IgnoreArmor(damage.from, damage.to) 
	  and damage.to:isWounded() then
		return true
	end
	return false
end

sgs.ai_skill_choice.kuangfu = function(self, choices)
	return "move"
end

local qingcheng_skill = {}
qingcheng_skill.name = "qingcheng"
table.insert(sgs.ai_skills, qingcheng_skill)
qingcheng_skill.getTurnUseCard = function(self, inclusive)
	local equipcard
	if self:needBear() then return end
	if self.player:hasArmorEffect("SilverLion") and self.player:isWounded() then
		equipcard = self.player:getArmor()
	else
		for _, card in sgs.qlist(self.player:getCards("he")) do
			if card:isKindOf("EquipCard") and not self.player:hasEquip(card) then
				equipcard = card
				break
			end
		end
		if not equipcard then
			for _, card in sgs.qlist(self.player:getCards("he")) do
				if card:isKindOf("EquipCard") and not card:isKindOf("Armor") and not card:isKindOf("DefensiveHorse") then
					equipcard = card
				end
			end
		end
	end

	if equipcard then
		local card_id = equipcard:getEffectiveId()
		local card_str = ("@QingchengCard=" .. card_id)
		local qc_card = sgs.Card_Parse(card_str)

		return qc_card
	end
end

sgs.ai_skill_use_func.QingchengCard = function(card, use, self)
	if self.room:alivePlayerCount() == 2 then
		local only_enemy = self.room:getOtherPlayers(self.player):first()
		if only_enemy:getLostHp() < 3 then return end
	end
	for _, enemy in ipairs(self.enemies) do
		for _, askill in ipairs(("noswuyan|wuyan|weimu|kanpo|liuli|yiji|jieming|ganglie|neoganglie|fankui|jianxiong|enyuan|nosenyuan" ..
								"|qingguo|longdan|xiangle|jiang|yanzheng|tianming|" ..
								"huangen|danlao|qianxun|juxiang|huoshou|anxian|fenyong|zhichi|jilei|feiying|yicong|wusheng|wushuang|tianxiang|leiji|" ..
								"xuanfeng|nosxuanfeng|luoying|xiaoguo|guhuo|guidao|guicai|shangshi|lianying|sijian|xiaoji|mingshi|zhiyu|hongyan|tiandu|lirang|" .. 
								"guzheng|xingshang|shushen"):split("|")) do
			if enemy:hasSkill(askill, true) and enemy:getMark("Qingcheng" .. askill) == 0 then
				use.card = card
				if use.to then
					use.to:append(enemy)
				end
				return
			end
		end
	end
	for _, friend in ipairs(self.friends_noself) do
		if friend:hasSkill("shiyong", true) and friend:getMark("Qingchengshiyong") == 0 then
			use.card = card
			if use.to then
				use.to:append(friend)
			end
			return
		end
	end
	return
end

sgs.ai_skill_choice.qingcheng = function(self, choices, data)
	local target = data:toPlayer()
	for _, askill in ipairs(("noswuyan|wuyan|weimu|kanpo|liuli|qingguo|longdan|xiangle|jiang|yanzheng|tianming|" ..
							"huangen|danlao|qianxun|juxiang|huoshou|anxian|fenyong|zhichi|jilei|feiying|yicong|wusheng|wushuang|tianxiang|leiji|" ..
							"xuanfeng|nosxuanfeng|luoying|xiaoguo|guhuo|guidao|guicai|shangshi|lianying|sijian|xiaoji|mingshi|zhiyu|hongyan|tiandu|lirang|" .. 
							"guzheng|xingshang|shushen"):split("|")) do
		if target:hasSkill(askill, true) and target:getMark("Qingcheng" .. askill) == 0 then
			return askill
		end
	end
end

sgs.ai_chaofeng.zoushi = 3
sgs.ai_use_value.QingchengCard = 2
sgs.ai_use_priority.QingchengCard = 2.2
sgs.ai_card_intention.QingchengCard = 30

sgs.ai_skill_invoke.cv_caopi = function(self, data)
	if math.random(0, 2) == 0 then sgs.ai_skill_choice.cv_caopi = "heg_caopi" return true
	elseif math.random(0, 6) == 0 then sgs.ai_skill_choice.cv_caopi = "ass_caopi" return true
	end
	return false
end

sgs.ai_skill_invoke.cv_zhugeliang = function(self, data)
	if math.random(0, 2) == 0 then return true end
	return false
end

sgs.ai_skill_invoke.cv_huangyueying = sgs.ai_skill_invoke.cv_caopi