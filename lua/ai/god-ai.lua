wushen_skill={}
wushen_skill.name="wushen"
table.insert(sgs.ai_skills,wushen_skill)
wushen_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)

	local red_card
	self:sortByUseValue(cards,true)

	for _,card in ipairs(cards)  do
		if card:getSuit() == sgs.Card_Heart then
			red_card = card
			break
		end
	end

	if red_card then
		local suit = red_card:getSuitString()
		local number = red_card:getNumberString()
		local card_id = red_card:getEffectiveId()
		local card_str = ("slash:wushen[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)

		assert(slash)

		return slash
	end
end

sgs.ai_filterskill_filter.wushen = function(card, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:getSuit() == sgs.Card_Heart then return ("slash:wushen[%s:%s]=%d"):format(suit, number, card_id) end
end

sgs.ai_skill_playerchosen.wuhun = function(self, targets)
	local targetlist=sgs.QList2Table(targets)
	local target
	local lord
	for _, player in ipairs(targetlist) do
		if player:isLord() then lord = player end
		if self:isEnemy(player) and (not target or target:getHp() < player:getHp()) then
			target = player
		end
	end
	if self.role == "rebel" and lord then return lord end
	if target then return target end
	self:sort(targetlist, "hp")
	if self.player:getRole() == "loyalist" and targetlist[1]:isLord() then return targetlist[2] end
	return targetlist[1]
end

function SmartAI:getWuhunRevengeTargets()
	local targets = {}
	local maxcount = 0
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		local count = p:getMark("@nightmare")
		if count > maxcount then
			targets = { p }
			maxcount = count
		elseif count == maxcount then
			table.insert(targets, p)
		end
	end
	return targets
end

function sgs.ai_slash_prohibit.wuhun(self, to, card, from)
	if from:hasSkill("jueqing") then return false end
	if from:hasFlag("NosJiefanUsed") then return false end
	local damageNum = self:hasHeavySlashDamage(from, nil, to, true)

	local maxfriendmark = 0
	local maxenemymark = 0
	for _, friend in ipairs(self:getFriends(from)) do
		local friendmark = friend:getMark("@nightmare")
		if friendmark > maxfriendmark then maxfriendmark = friendmark end
	end
	for _, enemy in ipairs(self:getEnemies(from)) do
		local enemymark = enemy:getMark("@nightmare")
		if enemymark > maxenemymark and enemy:objectName() ~= to:objectName() then maxenemymark = enemymark end
	end
	if self:isEnemy(to, from) and not (to:isLord() and from:getRole() == "rebel") then
		if (maxfriendmark + damageNum >= maxenemymark) and not (#(self:getEnemies(from)) == 1 and #(self:getFriends(from)) + #(self:getEnemies(from)) == self.room:alivePlayerCount()) then
			if not (from:getMark("@nightmare") == maxfriendmark and from:getRole() == "loyalist") then
				return true
			end
		end
	end
end

function SmartAI:cantbeHurt(player, damageNum, from)
	from = from or self.player
	damageNum = damageNum or 1
	if from:hasSkill("jueqing") then return false end
	local maxfriendmark = 0
	local maxenemymark = 0
	local dyingfriend = 0
	if player:hasSkill("wuhun") and not player:isLord() and #(self:getFriendsNoself(player)) > 0 then
		for _, friend in ipairs(self:getFriends(from)) do
			local friendmark = friend:getMark("@nightmare")
			if friendmark > maxfriendmark then maxfriendmark = friendmark end
		end
		for _, enemy in ipairs(self:getEnemies(from)) do
			local enemymark = enemy:getMark("@nightmare")
			if enemymark > maxenemymark and enemy:objectName() ~= player:objectName() then maxenemymark = enemymark end
		end
		if self:isEnemy(player, from) then
			if maxfriendmark + damageNum >= maxenemymark and not (#(self:getEnemies(from)) == 1 and #(self:getFriends(from)) + #(self:getEnemies(from)) == self.room:alivePlayerCount())
				and not (from:getMark("@nightmare") == maxfriendmark and from:getRole() == "loyalist") then
				return true
			end
		elseif maxfriendmark + damageNum > maxenemymark then
			return true
		end
	end
	if player:hasSkill("duanchang") and not player:isLord() and #(self:getFriendsNoself(player)) > 0 and player:getHp() <= 1 then
		if not (from:getMaxHp() == 3 and from:getArmor() and from:getDefensiveHorse()) then
			if from:getMaxHp() <= 3 or (from:isLord() and self:isWeak(from)) then return true end
			if from:getMaxHp() <= 3 or (self.room:getLord() and from:getRole() == "renegade") then return true end
		end
	end
	if player:hasSkill("tianxiang") and getKnownCard(player, "diamond", false) + getKnownCard(player, "club", false) < player:getHandcardNum() then
		local peach_num = self.player:objectName() == from:objectName() and self:getCardsNum("Peach") or getCardsNum("Peach", from)
		for _, friend in ipairs(self:getFriends(from)) do
			if friend:getHp() < 2 and peach_num then
				dyingfriend = dyingfriend + 1
			end
		end
		if dyingfriend > 0 and player:getHandcardNum() > 0 then
			return true
		end
	end
	return false
end

function SmartAI:needDeath(player)
	local maxfriendmark = 0
	local maxenemymark = 0
	player = player or self.player
	if player:hasSkill("wuhun") and #(self:getFriendsNoself(player)) > 0 then
		for _, aplayer in sgs.qlist(self.room:getAlivePlayers()) do
			local mark = aplayer:getMark("@nightmare")
			if self:isFriend(player,aplayer) and player:objectName() ~= aplayer:objectName() then
				if mark > maxfriendmark then maxfriendmark = mark end
			end
			if self:isEnemy(player,aplayer) then
				if mark > maxenemymark then maxenemymark = mark end
			end
			if maxfriendmark > maxenemymark then return false
			elseif maxenemymark == 0 then return false
			else return true end
		end
	end
	return false
end

function SmartAI:doNotSave(player)
	if (player:hasSkill("niepan") and player:getMark("@nirvana") > 0 and player:getCards("e"):length() < 2)
		or (player:hasSkill("fuli") and player:getMark("@laoji") > 0 and player:getCards("e"):length() < 2) then
		return true
	end
	if player:hasFlag("AI_doNotSave") then return true end
	return false
end


sgs.ai_chaofeng.shenguanyu = -6

sgs.ai_skill_invoke.shelie = true

local gongxin_skill={}
gongxin_skill.name="gongxin"
table.insert(sgs.ai_skills,gongxin_skill)
gongxin_skill.getTurnUseCard=function(self)
		local card_str = ("@GongxinCard=.")
		local gongxin_card = sgs.Card_Parse(card_str)
		assert(gongxin_card)
		return gongxin_card
end

sgs.ai_skill_use_func.GongxinCard=function(card,use,self)
	if self.player:hasUsed("GongxinCard") then return end
	self:sort(self.enemies,"handcard")

	for index = #self.enemies, 1, -1 do
		if not self.enemies[index]:isKongcheng() and self:objectiveLevel(self.enemies[index]) > 0 then
			use.card = card
			if use.to then
				use.to:append(self.enemies[index])
			end
			return
		end
	end
end

sgs.ai_skill_askforag.gongxin = function(self, card_ids)
	local target = self.player:getTag("GongxinTarget"):toPlayer()
	if not target or self:isFriend(target) then return -1 end
	local nextAlive = self.player
	repeat
		nextAlive = nextAlive:getNextAlive()
	until nextAlive:faceUp()

	local peach, ex_nihilo, jink, nullification, slash
	local valuable
	for _, id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(id)
		if card:isKindOf("Peach") then peach = id end
		if card:isKindOf("ExNihilo") then ex_nihilo = id end
		if card:isKindOf("Jink") then jink = id end
		if card:isKindOf("Nullification") then nullification = id end
		if card:isKindOf("Slash") then slash = id end
	end
	valuable = peach or ex_nihilo or jink or nullification or slash or card_ids[1]

	local willUseExNihilo, willRecast
	if self:getCardsNum("ExNihilo") > 0 then
		local ex_nihilo = self:getCard("ExNihilo")
		if ex_nihilo then
			local dummy_use = { isDummy = true }
			self:useTrickCard(ex_nihilo, dummy_use)
			if dummy_use.card then willUseExNihilo = true end
		end
	elseif self:getCardsNum("IronChain") > 0 then
		local iron_chain = self:getCard("IronChain")
		if iron_chain then
			local dummy_use = { to = sgs.SPlayerList(), isDummy = true }
			self:useTrickCard(iron_chain, dummy_use)
			if dummy_use.card and dummy_use.to:isEmpty() then willRecast = true end
		end
	end
	if willUseExNihilo or willRecast then
		local card = sgs.Sanguosha:getCard(valuable)
		if card:isKindOf("Peach") then
			self.gongxinchoice = "put"
			return valuable
		end
		if card:isKindOf("TrickCard") or card:isKindOf("Indulgence") or card:isKindOf("SupplyShortage") then
			local dummy_use = { isDummy = true }
			self:useTrickCard(card, dummy_use)
			if dummy_use.card then
				self.gongxinchoice = "put"
				return valuable
			end
		end
		if card:isKindOf("Jink") and self:getCardsNum("Jink") == 0 then
			self.gongxinchoice = "put"
			return valuable
		end
		if card:isKindOf("Nullification") and self:getCardsNum("Nullification") == 0 then
			self.gongxinchoice = "put"
			return valuable
		end
		if card:isKindOf("Slash") and self:slashIsAvailable() then
			local dummy_use = { isDummy = true }
			self:useBasicCard(card, dummy_use)
			if dummy_use.card then
				self.gongxinchoice = "put"
				return valuable
			end
		end
		self.gongxinchoice = "discard"
		return valuable
	end

	local hasLightning, hasIndulgence, hasSupplyShortage
	local tricks = nextAlive:getJudgingArea()
	if not tricks:isEmpty() and not nextAlive:containsTrick("YanxiaoCard") then
		local trick = tricks:at(tricks:length() - 1)
		if self:hasTrickEffective(trick, nextAlive) then
			if trick:isKindOf("Lightning") then hasLightning = true
			elseif trick:isKindOf("Indulgence") then hasIndulgence = true
			elseif trick:isKindOf("SupplyShortage") then hasSupplyShortage = true
			end
		end
	end

	if self:isEnemy(nextAlive) and nextAlive:hasSkill("luoshen") and valuable then
		self.gongxinchoice = "put"
		return valuable
	end
	if nextAlive:hasSkill("yinghun") and nextAlive:isWounded() then
		self.gongxinchoice = self:isFriend(nextAlive) and "put" or "discard"
		return valuable
	end
	if target:hasSkill("hongyan") and hasLightning and self:isEnemy(nextAlive) and not (nextAlive:hasSkill("qiaobian") and nextAlive:getHandcardNum() > 0) then
		for _, id in ipairs(card_ids) do
			local card = sgs.Sanguosha:getEngineCard(id)
			if card:getSuit() == sgs.Card_Spade and card:getNumber() >= 2 and card:getNumber() <= 9 then
				self.gongxinchoice = "put"
				return id
			end
		end
	end
	if hasIndulgence and self:isFriend(nextAlive) then
		self.gongxinchoice = "put"
		return valuable
	end
	if hasSupplyShortage and self:isEnemy(nextAlive) and not (nextAlive:hasSkill("qiaobian") and nextAlive:getHandcardNum() > 0) then
		local enemy_null = 0
		for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if self:isFriend(p) then enemy_null = enemy_null - getCardsNum("Nullification", p) end
			if self:isEnemy(p) then enemy_null = enemy_null + getCardsNum("Nullification", p) end
		end
		enemy_null = enemy_null - self:getCardsNum("Nullification")
		if enemy_null < 0.8 then
			self.gongxinchoice = "put"
			return valuable
		end
	end

	if self:isFriend(nextAlive) and not self:willSkipDrawPhase() and not self:willSkipPlayPhase()
		and not nextAlive:hasSkill("luoshen")
		and not nextAlive:hasSkill("tuxi") and not (nextAlive:hasSkill("qiaobian") and nextAlive:getHandcardNum() > 0) then
		if (peach and valuable == peach) or (ex_nihilo and valuable == ex_nihilo) then
			self.gongxinchoice = "put"
			return valuable
		end
		if jink and valuable == jink and getCardsNum("Jink", nextAlive) < 1 then
			self.gongxinchoice = "put"
			return valuable
		end
		if nullification and valuable == nullification and getCardsNum("Nullification", nextAlive) < 1 then
			self.gongxinchoice = "put"
			return valuable
		end
		if slash and valuable == slash and self:hasCrossbowEffect(nextAlive) then
			self.gongxinchoice = "put"
			return valuable
		end
	end

	local card = sgs.Sanguosha:getCard(valuable)
	local keep = false
	if card:isKindOf("Slash") or card:isKindOf("Jink")
		or card:isKindOf("EquipCard")
		or card:isKindOf("Disaster") or card:isKindOf("GlobalEffect") or card:isKindOf("Nullification")
		or target:isLocked(card) then
		keep = true
	end
	self.gongxinchoice = (target:objectName() == nextAlive:objectName() and keep) and "put" or "discard"
	return valuable
end

sgs.ai_skill_choice.gongxin = function(self, choices)
	return self.gongxinchoice or "discard"
end

sgs.ai_use_value.GongxinCard = 8.5
sgs.ai_use_priority.GongxinCard = 9.5
sgs.ai_card_intention.GongxinCard = 80

sgs.ai_skill_invoke.qinyin = function(self, data)
	self:sort(self.friends, "hp")
	self:sort(self.enemies, "hp")
	local up = 0
	local down = 0
	
	for _, friend in ipairs(self.friends) do
		down = down - 10
		up = up + (friend:isWounded() and 10 or 0)
		if self:hasSkills(sgs.masochism_skill, friend) then
			down = down - 5
			if friend:isWounded() then up = up + 5 end
		end
		if self:needToLoseHp(friend, nil, nil, true) then down = down + 5 end
		if self:needToLoseHp(friend, nil, nil, true, true) and friend:isWounded() then up = up - 5 end
		
		if self:isWeak(friend) then
			if friend:isWounded() then up = up + 10 + (friend:isLord() and 20 or 0) end
			down = down - 10 - (friend:isLord() and 40 or 0)
			if friend:getHp() <= 1 and not friend:hasSkill("buqu") or friend:getPile("buqu"):length() > 4 then
				down = down - 20 - (friend:isLord() and 40 or 0)
			end
		end
	end
	
	for _, enemy in ipairs(self.enemies) do
		down = down + 10
		up = up - (enemy:isWounded() and 10 or 0)
		if self:hasSkills(sgs.masochism_skill, enemy) then 
			down = down + 10
			if enemy:isWounded() then up = up - 10 end
		end
		if self:needToLoseHp(enemy, nil, nil, true) then down = down - 5 end
		if self:needToLoseHp(enemy, nil, nil, true, true) and enemy:isWounded() then up = up - 5 end
		
		if self:isWeak(enemy) then
			if enemy:isWounded() then up = up - 10 end
			down = down + 10
			if enemy:getHp() <= 1 and not enemy:hasSkill("buqu") then
				down = down + 10 + ((enemy:isLord() and #self.enemies > 1) and 20 or 0)
			end
		end
	end

	if down > 0 then 
		sgs.ai_skill_choice.qinyin = "down"
		return true
	elseif up > 0 then
		sgs.ai_skill_choice.qinyin = "up"
		return true
	end
	return false
end

local yeyan_skill={}
yeyan_skill.name = "yeyan"
table.insert(sgs.ai_skills, yeyan_skill)
yeyan_skill.getTurnUseCard=function(self)
	if self.player:getRole() == "lord" and (#self.enemies > 1 or sgs.turncount <= 1) then return end
	if self.player:getMark("@flame") == 0 then return end
	if self.player:getHandcardNum() >= 4 then
		local spade, club, heart, diamond
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if card:getSuit() == sgs.Card_Spade then spade = true
			elseif card:getSuit() == sgs.Card_Club then club = true
			elseif card:getSuit() == sgs.Card_Heart then heart = true
			elseif card:getSuit() == sgs.Card_Diamond then diamond = true
			end
		end
		if spade and club and diamond and heart then
			self:sort(self.enemies, "hp")
			local target_num = 0
			for _, enemy in ipairs(self.enemies) do
				if enemy:hasArmorEffect("Vine") or (enemy:isChained() and self:isGoodChainTarget(enemy)) then
					target_num = target_num + 1
				elseif enemy:getHp() <= 3 then
					target_num = target_num + 1
				end
			end

			if target_num >= 1 then
				return sgs.Card_Parse("@GreatYeyanCard=.")
			end
		end
	end

	self.yeyanchained = false
	if self.player:getHp() + self:getCardsNum("Peach") + self:getCardsNum("Analeptic") <= 2 then
		return sgs.Card_Parse("@SmallYeyanCard=.")
	end
	local target_num = 0
	local chained = 0
	for _, enemy in ipairs(self.enemies) do
		if ((enemy:hasArmorEffect("Vine") or self:isEquip("GaleShell", enemy) or enemy:getMark("@gale") > 0) or enemy:getHp() <= 1) 
			and not (self.role == "renegade" and enemy:isLord()) then
			target_num = target_num + 1
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if enemy:isChained() and self:isGoodChainTarget(enemy) then 
			if chained == 0 then target_num = target_num +1 end
			chained = chained + 1
		end
	end
	self.yeyanchained = (chained > 1)
	if target_num > 2 or (target_num > 1 and self.yeyanchained) or
	(#self.enemies + 1 == self.room:alivePlayerCount() and self.room:alivePlayerCount() < sgs.Sanguosha:getPlayerCount(self.room:getMode())) then
		return sgs.Card_Parse("@SmallYeyanCard=.")
	end
end

sgs.ai_skill_use_func.GreatYeyanCard=function(card,use,self)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local need_cards = {}
	local spade, club, heart, diamond
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Spade and not spade then spade = true table.insert(need_cards, card:getId())
		elseif card:getSuit() == sgs.Card_Club and not club then club = true table.insert(need_cards, card:getId())
		elseif card:getSuit() == sgs.Card_Heart and not heart then heart = true table.insert(need_cards, card:getId())
		elseif card:getSuit() == sgs.Card_Diamond and not diamond then diamond = true table.insert(need_cards, card:getId())
		end
	end
	if #need_cards < 4 then return end
	local greatyeyan = sgs.Card_Parse("@GreatYeyanCard=" .. table.concat(need_cards, "+"))
	assert(greatyeyan)

	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if not enemy:hasArmorEffect("SilverLion") and
			not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) and
			self:objectiveLevel(enemy) > 3 and self:damageIsEffective(enemy, sgs.DamageStruct_Fire) then
				if enemy:isChained() and self:isGoodChainTarget(enemy) then
					if enemy:hasArmorEffect("Vine") then
						use.card = greatyeyan
						if use.to then 
							use.to:append(enemy)
							use.to:append(enemy)
							use.to:append(enemy)	
						end
						return
					end
				end
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if not enemy:hasArmorEffect("SilverLion") and
			not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) 
			and self:objectiveLevel(enemy) > 3 and self:damageIsEffective(enemy, sgs.DamageStruct_Fire) then
				if enemy:isChained() and self:isGoodChainTarget(enemy) then
					use.card = greatyeyan
					if use.to then 
						use.to:append(enemy)
						use.to:append(enemy)
						use.to:append(enemy)	
					end
					return
				end
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if not enemy:hasArmorEffect("SilverLion") and
			not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) 
			and self:objectiveLevel(enemy) > 3 and self:damageIsEffective(enemy, sgs.DamageStruct_Fire) then
				if not enemy:isChained() then
					if enemy:hasArmorEffect("Vine") then
						use.card = greatyeyan
						if use.to then 
							use.to:append(enemy)
							use.to:append(enemy)
							use.to:append(enemy)	
						end
						return
					end
				end
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if not enemy:hasArmorEffect("SilverLion") and
			not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) 
			and self:objectiveLevel(enemy) > 3 and self:damageIsEffective(enemy, sgs.DamageStruct_Fire) then
				if not enemy:isChained() then
					use.card = greatyeyan
					if use.to then 
						use.to:append(enemy)
						use.to:append(enemy)
						use.to:append(enemy)	
					end
					return
				end
		end
	end
end

sgs.ai_use_value.GreatYeyanCard = 8
sgs.ai_use_priority.GreatYeyanCard = 9

sgs.ai_card_intention.GreatYeyanCard = 200

sgs.ai_skill_use_func.SmallYeyanCard=function(card,use,self)
	local num = 0
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) and self:damageIsEffective(enemy, sgs.DamageStruct_Fire) then
			if enemy:isChained() and self:isGoodChainTarget(enemy) then
				if enemy:hasArmorEffect("Vine") then
					if use.to then use.to:append(enemy) end
					num = num + 1
					if num >=3 then break end
				end
			end
		end
	end
	if num < 3 then
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) and self:damageIsEffective(enemy, sgs.DamageStruct_Fire) then
				if enemy:isChained() and self:isGoodChainTarget(enemy) and not enemy:hasArmorEffect("Vine") then
					if use.to then use.to:append(enemy) end
					num = num + 1
					if num >=3 then break end
				end
			end
		end
	end	
	if num < 3 then
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) and self:damageIsEffective(enemy, sgs.DamageStruct_Fire) then
				if not enemy:isChained() then
					if enemy:hasArmorEffect("Vine") then
						if use.to then use.to:append(enemy) end
						num = num + 1
						if num >=3 then break end
					end
				end
			end
		end
	end
	if num < 3 then
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) and self:damageIsEffective(enemy, sgs.DamageStruct_Fire) then
				if not enemy:isChained() and not enemy:hasArmorEffect("Vine") then
					if use.to then use.to:append(enemy) end
					num = num + 1
					if num >=3 then break end
				end
			end
		end
	end
	if num > 0 then use.card = card end
end

sgs.ai_card_intention.SmallYeyanCard = 80
sgs.ai_use_priority.SmallYeyanCard = 2.3

sgs.ai_skill_askforag.qixing = function(self, card_ids)
	local cards = {}
	for _, card_id in ipairs(card_ids) do
		table.insert(cards, sgs.Sanguosha:getCard(card_id))
	end
	self:sortByCardNeed(cards)
	if self.player:getPhase() == sgs.Player_Draw then
		return cards[#cards]:getEffectiveId()
	end
	if self.player:getPhase() == sgs.Player_Finish then
		return cards[1]:getEffectiveId()
	end
	return -1
end

sgs.ai_skill_use["@@kuangfeng"] = function(self,prompt)
	local friendly_fire
	for _, friend in ipairs(self.friends_noself) do
		if friend:getMark("@gale") == 0 and self:damageIsEffective(friend, sgs.DamageStruct_Fire) and friend:faceUp() and not self:willSkipPlayPhase(friend)
			and (friend:hasSkill("huoji") or friend:hasWeapon("fan") or (friend:hasSkill("yeyan") and friend:getMark("@flame") > 0)) then
			friendly_fire = true
			break
		end
	end

	local is_chained = 0
	local target = {}
	for _, enemy in ipairs(self.enemies) do
		if enemy:getMark("@gale") == 0 and self:damageIsEffective(enemy, sgs.DamageStruct_Fire) then
			if enemy:isChained() then
				is_chained = is_chained + 1
				table.insert(target, enemy)
			elseif enemy:hasArmorEffect("Vine") then
				table.insert(target, 1, enemy)
				break
			end
		end
	end
	local usecard=false
	if friendly_fire and is_chained > 1 then usecard=true end
	self:sort(self.friends, "hp")
	if target[1] and not self:isWeak(self.friends[1]) then
		if target[1]:hasArmorEffect("Vine") and friendly_fire then usecard = true end
	end
	if usecard then
		if not target[1] then table.insert(target,self.enemies[1]) end
		if target[1] then return "@KuangfengCard=.->" .. target[1]:objectName() else return "." end
	else
		return "."
	end
end

sgs.ai_card_intention.KuangfengCard = 80

sgs.ai_skill_use["@@dawu"] = function(self, prompt)
	self:sort(self.friends_noself, "hp")
	local targets = {}
	local lord = self.room:getLord()
	self:sort(self.friends_noself,"defense")
	if lord and lord:getMark("@fog") == 0 and self:isFriend(lord) and not sgs.isLordHealthy() and not self.player:isLord() and not lord:hasSkill("buqu")
		and not (lord:hasSkill("hunzi") and lord:getMark("hunzi") == 0 and lord:getHp() > 1) then 
			table.insert(targets, lord:objectName())
	else
		for _, friend in ipairs(self.friends_noself) do
			if friend:getMark("@fog") == 0 and self:isWeak(friend) and not friend:hasSkill("buqu") 
				and not (friend:hasSkill("hunzi") and friend:getMark("hunzi") == 0 and friend:getHp() > 1) then
					table.insert(targets, friend:objectName())
					break 
			end
		end	
	end
	if self.player:getPile("stars"):length() > #targets and self:isWeak() then table.insert(targets, self.player:objectName()) end
	if #targets > 0 then return "@DawuCard=.->" .. table.concat(targets, "+") end
	return "."
end

sgs.ai_card_intention.DawuCard = -70

sgs.ai_skill_invoke.guixin = function(self, data)
	-- local damage = data:toDamage()
	if self.player:hasSkill("manjuan") and self.player:getPhase() == sgs.Player_NotActive then return false end
	local diaochan = self.room:findPlayerBySkillName("lihun")
	if diaochan and self:isEnemy(diaochan) and not diaochan:hasUsed("LihunCard") and self.player:isMale() and self.room:alivePlayerCount() > 5 then
		local CP = self.room:getCurrent()
		if (diaochan:objectName() == CP:objectName() or self:playerGetRound(diaochan) < self:playerGetRound(self.player)) then
			return false
		end
	end
	return self.room:alivePlayerCount() > 2 or not self.player:faceUp()
end

sgs.ai_need_damaged.guixin = function (self, attacker, player)	
	if not player:hasSkill("guixin") then return false end
	if self.room:alivePlayerCount() <=3 then return false end
	local drawcards = 0
	for _, aplayer in sgs.qlist(self.room:getOtherPlayers(player)) do
		if aplayer:getCards("hej"):length() > 0 then drawcards = drawcards + 1 end
	end
	return not self:IsLihunTarget(player, drawcards)
end

sgs.ai_chaofeng.shencaocao = -6

sgs.ai_skill_choice.wumou = function(self, choices)
	if self.player:getMark("@wrath") > 6 then return "discard" end
	if self.player:getHp() + self:getCardsNum("Peach") > 3 then return "losehp"
	else return "discard"
	end
end

local wuqian_skill={}
wuqian_skill.name = "wuqian"
table.insert(sgs.ai_skills, wuqian_skill)
wuqian_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("WuqianCard") or self.player:getMark("@wrath") < 2 then return end

	local card_str = ("@WuqianCard=.")
	self:sort(self.enemies, "hp")
	local has_enemy
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() <= 2 and getCardsNum("Jink", enemy) < 2 and enemy:getHandcardNum() > 0 and self.player:distanceTo(enemy) <= self.player:getAttackRange() then 
			has_enemy = enemy break end
	end

	if has_enemy and self:getCardsNum("Slash") > 0 then
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if card:isKindOf("Slash") and self:slashIsEffective(card, has_enemy) and self.player:canSlash(has_enemy, card)
				and (self:getCardsNum("Analeptic") > 0 or has_enemy:getHp() <= 1) and card:isAvailable(self.player) then
				return sgs.Card_Parse(card_str)
			elseif card:isKindOf("Duel") then
				return sgs.Card_Parse(card_str)
			end
		end
	end
end

sgs.ai_skill_use_func.WuqianCard=function(card,use,self)
	self:sort(self.enemies,"hp")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() <= 2 and getCardsNum("Jink", enemy) < 2 and self.player:inMyAttackRange(enemy) then
			if not enemy:hasArmorEffect("SilverLion") and getCardsNum("Jink", enemy) < 1 then
			else
				if use.to then
					use.to:append(enemy)
				end
				use.card = card
				return
			end
		end
	end
end

sgs.ai_use_value.WuqianCard = 5
sgs.ai_use_priority.WuqianCard = 2.5
sgs.ai_card_intention.WuqianCard = 80

function SmartAI:cansaveplayer(player)
	player = player or self.player
	local good = 0
	if  self:hasSkills("jijiu|nosjiefan",player)  and player:getHandcardNum() > 0  then
		good = good + 0.5 
	end
	if player:hasSkill("chunlao") and player:getPile("wine"):length() > 0 then
		good = good + 1 
	end
	if player:hasSkill("buyi") then 
		good = good +0.5
	end
	return good
end

function SmartAI:dangerousshenguanyu(player)
	if not player then self.room:writeToConsole("Player is empty in dangerousshenguanyu!") return end
	local good = 0
	if not isLord(player) and player:hasSkill("wuhun") and player:getHp() == 1 and (not self:isEnemy(player) or #self.enemies > 1 and sgs.turncount > 1) then
		local maxnightmare = 0
		local nightmareplayer = {}
		for _, ap in sgs.qlist(self.room:getAlivePlayers()) do
			maxnightmare = math.max(ap:getMark("@nightmare"),maxnightmare)
		end
		if maxnightmare == 0 then return good end
		for _, np in sgs.qlist(self.room:getAlivePlayers()) do
			if np:getMark("@nightmare") == maxnightmare then
				table.insert(nightmareplayer, np)
			end
		end
		if #nightmareplayer == 0 then return good end
		if self:isFriend(player) then
		for _, p in ipairs(nightmareplayer) do
				if self:isEnemy(p) and p:isLord() then good = good + 100 return good end
			end
			for _, p in ipairs(nightmareplayer) do
				if self:isEnemy(p)  then good = good + 1.5 return good end
			end
			good = good - 5
			return good
		else
			for _, p in ipairs(nightmareplayer) do
				if self:isFriend(p) and p:isLord() then good = good - 100 return good end
			end
			for _, p in ipairs(nightmareplayer) do
				if self:isFriend(p)  then	good = good - 1.5 return good end
			end
			good = good + 3
			return good
		end
	end
	return good
end

local shenfen_skill = {}
shenfen_skill.name = "shenfen"
table.insert(sgs.ai_skills, shenfen_skill)
shenfen_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("ShenfenCard") then return end
	if self.player:getMark("@wrath") < 6 then return end
	return sgs.Card_Parse("@ShenfenCard=.")
end

sgs.ai_skill_use_func.ShenfenCard = function(card,use,self)
	local friends_ZDL, enemies_ZDL = 0, 0
	local good = (#self.enemies - #self.friends_noself) * 1.5
	
	if self:isEnemy(self.player:getNextAlive()) and self.player:getHp() > 2 then good = good - 0.5 end
	if self.player:getRole() == "rebel" then good = good + 1 end	
	if self.player:getRole() == "renegade" then good = good + 0.5 end	
	if not self.player:faceUp() then good = good + 1 end
	if self:hasSkills("jushou|neojushou|lihun|kuiwei|jiushi") then good = good + 1 end
	if self.player:getWeapon() and self.player:getWeapon():isKindOf("Crossbow") and self:getCardsNum("Slash", self.player) > 1 then good = good + 1 end
	
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		good = good + self:dangerousshenguanyu(p)
		if p:hasSkill("dushi") and p:getHp() < 2 then good = good - 1 end
	end
	
	for _, friend in ipairs(self.friends_noself) do
		if (friend:hasSkill("fangzhu") and friend:getHp() > 1) or 
		(friend:hasSkill("jilve") and friend:getMark("@waked") > 0 and friend:getMark("@bear") > 0 and friend:getHp() > 1) then
			good = good + friend:getLostHp() * 0.25 + 0.5
			break
		end
	end
	
	for _, friend in ipairs(self.friends_noself) do
		if friend:hasSkill("jujian") then
			good = good + 0.5
			break
		end
	end	
	
	for _, friend in ipairs(self.friends_noself) do
		friends_ZDL = friends_ZDL + friend:getCardCount(true) + friend:getHp()
		if friend:getHandcardNum() > 4 then good = good + friend:getHandcardNum() * 0.25 end
		good = good + self:cansaveplayer(friend)
		if friend:hasArmorEffect("SilverLion") and friend:getHp() > 1 then good = good + 0.5 end
		if self:damageIsEffective(friend) then
			if friend:getHp() == 1 and self:getAllPeachNum() < 1 then
				if isLord(friend) then
					good = good - 100 
				elseif self.room:getMode() ~= "06_3v3" then
					if isLord(self.player) and sgs.evaluatePlayerRole(friend) == "loyalist" then
						good = good - 0.6 + (self.player:getCardCount(true)*0.3)
					end
				end
			else
				good = good - 1
			end
			if isLord(friend) then
				good = good - 0.5
			end
		elseif not self:damageIsEffective(friend) then
			good = good + 1
		end
		if friend:hasSkill("guixin") and friend:getHp()>1 then good = good + 1 end
	end	
	
	for _,enemy in ipairs(self.enemies) do
		enemies_ZDL = enemies_ZDL + enemy:getCardCount(true) + enemy:getHp()
		if enemy:getHandcardNum() > 4 then good = good - enemy:getHandcardNum()*0.25 end
		good = good - self:cansaveplayer(enemy)
		
		if self:damageIsEffective(enemy) then
			if isLord(enemy) and self.player:getRole() == "rebel" then
				good = good + 1
			end
			if enemy:getHp() == 1 then
				if isLord(enemy) and self.player:getRole() == "rebel" then
					good = good + 3
				elseif enemy:getRole() ~= "lord" then
					good = good + 1 
				end
			end
			
			if enemy:hasSkill("guixin") and enemy:getHp() > 1 then good = good - self.player:aliveCount()*0.2 end
			if enemy:hasSkill("ganglie") and enemy:getHp() > 1 then good = good - 1 end
			if enemy:hasSkill("xuehen") and enemy:getHp() > 1 then good = good - 1 end
			if enemy:hasArmorEffect("SilverLion") and enemy:getHp() > 1 then good = good - 0.5 end
		else    
			good = good - 1
		end
	end
	
	local Combat_Effectiveness = ((#self.friends_noself > 0 and friends_ZDL/#self.friends_noself or 0) - (#self.enemies > 0 and enemies_ZDL/#self.enemies or 0))/2
	-- self.room:writeToConsole("friendsZDL:"..friends_ZDL..", enemiesZDL:"..enemies_ZDL..", CE:"..Combat_Effectiveness)
	good = good - Combat_Effectiveness
	
	-- self.room:writeToConsole("UseShenfen:"..good)
	if good > 0 and self.player:getMark("@wrath") >5 then 
		use.card = card		
	end	
end

local shenfen_filter = function(player, carduse)
	if carduse.card:isKindOf("ShenfenCard") then
		sgs.shenfensource = player
	end
end
table.insert(sgs.ai_choicemade_filter.cardUsed, shenfen_filter)

sgs.ai_use_value.ShenfenCard = 8
sgs.ai_use_priority.ShenfenCard = 9.3
sgs.ai_card_intention.ShenfenCard = function(self, card, from, tos, source)
	 sgs.shenfensource = nil
end

sgs.dynamic_value.damage_card.ShenfenCard = true
sgs.dynamic_value.control_card.ShenfenCard = true

local longhun_skill={}
longhun_skill.name="longhun"
table.insert(sgs.ai_skills, longhun_skill)
longhun_skill.getTurnUseCard = function(self)
	if self.player:getHp()>1 then return end
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByUseValue(cards,true)
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Diamond and self:slashIsAvailable()
			and not (self.player:hasWeapon("Crossbow") and card:getEffectiveId() == self.player:getWeapon():getEffectiveId() and not self.player:canSlashWithoutCrossbow()) then
			return sgs.Card_Parse(("fire_slash:longhun[%s:%s]=%d"):format(card:getSuitString(),card:getNumberString(),card:getId()))
		end
	end
end

sgs.ai_view_as.longhun = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if player:getHp() > 1 or card_place == sgs.Player_PlaceSpecial then return end
	if card:getSuit() == sgs.Card_Diamond then
		return ("fire_slash:longhun[%s:%s]=%d"):format(suit, number, card_id)
	elseif card:getSuit() == sgs.Card_Club then
		return ("jink:longhun[%s:%s]=%d"):format(suit, number, card_id)
	elseif card:getSuit() == sgs.Card_Heart and not player:hasFlag("Global_PreventPeach") then
		return ("peach:longhun[%s:%s]=%d"):format(suit, number, card_id)
	elseif card:getSuit() == sgs.Card_Spade then
		return ("nullification:longhun[%s:%s]=%d"):format(suit, number, card_id)
	end
end

sgs.longhun_suit_value = {
	heart = 6.7,
	spade = 5,
	club = 4.2,
	diamond = 3.9,
}

function sgs.ai_cardneed.longhun(to, card, self)
	if to:getCards("he"):length() <= 2 then return true end
	return card:getSuit() == sgs.Card_Heart or card:getSuit() == sgs.Card_Spade
end

sgs.ai_skill_invoke.lianpo = true

function SmartAI:needBear(player)
	player = player or self.player
	return player:hasSkills("renjie+baiyin") and not player:hasSkill("jilve") and player:getMark("@bear") < 4
end

sgs.ai_skill_invoke.jilve_jizhi = function(self, data)
	local n = self.player:getMark("@bear")
	local use = (n > 2 or self:getOverflow() > 0)
	local card = data:toCardResponse().m_card
	card = card or data:toCardUse().card
	return use or card:isKindOf("ExNihilo")
end

sgs.ai_skill_invoke.jilve_guicai = function(self, data)
	local n = self.player:getMark("@bear")
	local use = (n > 2 or self:getOverflow() > 0)
	local judge = data:toJudge()
	if not self:needRetrial(judge) then return false end
	return (use or judge.who == self.player or judge.reason == "lightning")
			and self:getRetrialCardId(sgs.QList2Table(self.player:getHandcards()), judge) ~= -1
end

sgs.ai_skill_invoke.jilve_fangzhu = function(self, data)
	return (sgs.ai_skill_playerchosen.fangzhu(self, self.room:getOtherPlayers(self.player)) ~= nil)
end

local jilve_skill = {}
jilve_skill.name = "jilve"
table.insert(sgs.ai_skills, jilve_skill)
jilve_skill.getTurnUseCard = function(self)
	if self.player:getMark("@bear") < 1 or (self.player:hasFlag("JilveWansha") and self.player:hasFlag("JilveZhiheng")) then return end
	local wanshadone = self.player:hasFlag("JilveWansha")
	if not wanshadone then
		if self.player:getMark("bear") >= 5 then
			sgs.ai_skill_choice.jilve = "wansha"
			sgs.ai_use_priority.JilveCard = 8
			local wanshacard = sgs.Card_Parse("@JilveCard=.")
			dummy_use={isDummy=true}
			self:useSkillCard(wanshacard, dummy_use)
			return sgs.Card_Parse("@JilveCard=.")
		end
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		local slashes = self:getCards("Slash")
		self:sort(self.enemies, "hp")
		local target
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("kongcheng") and enemy:isKongcheng()) and self:isWeak(enemy) and self:damageMinusHp(self, enemy, 1) > 0
			  and #self.enemies > 1 then
				sgs.ai_skill_choice.jilve = "wansha"
				sgs.ai_use_priority.JilveCard = 8
				local wanshacard = sgs.Card_Parse("@JilveCard=.")
				dummy_use={isDummy=true}
				self:useSkillCard(wanshacard, dummy_use)
				return sgs.Card_Parse("@JilveCard=.") 
			end
		end
	end
	if not self.player:hasFlag("JilveZhiheng") then
		sgs.ai_skill_choice.jilve = "zhiheng"
		sgs.ai_use_priority.JilveCard = sgs.ai_use_priority.ZhihengCard
		local card = sgs.Card_Parse("@ZhihengCard=.")
		local dummy_use={isDummy=true}
		self:useSkillCard(card, dummy_use)
		if dummy_use.card then return sgs.Card_Parse("@JilveCard=.") end
	elseif not wanshadone then
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		local slashes = self:getCards("Slash")
		self:sort(self.enemies, "hp")
		local target
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("kongcheng") and enemy:isKongcheng()) and self:isWeak(enemy) and self:damageMinusHp(self, enemy, 1) > 0
			  and #self.enemies > 1 then
				sgs.ai_skill_choice.jilve = "wansha"
				sgs.ai_use_priority.JilveCard = 8
				local wanshacard = sgs.Card_Parse("@JilveCard=.")
				dummy_use = {isDummy=true}
				self:useSkillCard(wanshacard,dummy_use)
				return sgs.Card_Parse("@JilveCard=.") 
			end
		end
	end
end

sgs.ai_skill_use_func.JilveCard=function(card,use,self)
	use.card = card
end

sgs.ai_skill_use["@zhiheng"]=function(self,prompt)
	local card=sgs.Card_Parse("@ZhihengCard=.")
	local dummy_use={isDummy=true}
	self:useSkillCard(card,dummy_use)
	if dummy_use.card then return (dummy_use.card):toString() .. "->." end
	return "."
end

sgs.ai_suit_priority.wushen= "club|spade|diamond|heart"
