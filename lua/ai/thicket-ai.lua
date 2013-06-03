sgs.ai_skill_invoke.xingshang = true

function SmartAI:toTurnOver(player, n)
	if not player then global_room:writeToConsole(debug.traceback()) return end
	n = n or 0
	if self:isEnemy(player) then
		local manchong = self.room:findPlayerBySkillName("junxing")
		if manchong and self:isFriend(player, manchong) and self:playerGetRound(manchong) < self:playerGetRound(player)
			and manchong:faceUp() and not self:willSkipPlayPhase(manchong)
			and not (manchong:isKongcheng() and self:willSkipDrawPhase(manchong)) then
			return false
		end
	end
	if player:hasUsed("ShenfenCard") and player:faceUp() and player:getPhase() == sgs.Player_Play
		and (not player:hasUsed("ShenfenCard") and player:getMark("@wrath") >= 6 or player:hasFlag("ShenfenUsing")) then
		return false
	end
	if n > 1 and player:hasSkill("jijiu")
		and not (player:hasSkill("manjuan") and player:getPhase() == sgs.Player_NotActive) then
		return false
	end
	if not player:faceUp() and not player:hasFlag("ShenfenUsing") and not player:hasFlag("GuixinUsing") then
		return false
	end
	if (self:hasSkills("jushou|neojushou|kuiwei", player) and player:getPhase() <= sgs.Player_Finish)
		or (player:hasSkill("lihun") and not player:hasUsed("LihunCard") and player:faceUp() and player:getPhase() == sgs.Player_Play) then
		return false
	end
	return true
end

sgs.ai_skill_playerchosen.fangzhu = function(self, targets)
	self:updatePlayers()
	self:sort(self.friends_noself, "handcard")
	local target = nil
	local n = self.player:getLostHp()
	for _, friend in ipairs(self.friends_noself) do
		if not self:toTurnOver(friend, n) then
			target = friend
			break
		end
	end

	if not target then
		if n >= 3 then
			target = self:findPlayerToDraw(false, n)
			if not target then
				for _, enemy in ipairs(self.enemies) do
					if self:toTurnOver(enemy, n) and enemy:hasSkill("manjuan") and enemy:getPhase() == sgs.Player_NotActive then
						target = enemy
						break
					end
				end
			end	
		else
			self:sort(self.enemies, "chaofeng")
			for _, enemy in ipairs(self.enemies) do
				if self:toTurnOver(enemy, n) and enemy:hasSkill("manjuan") and enemy:getPhase() == sgs.Player_NotActive then
					target = enemy
					break
				end
			end
			if not target then
				for _, enemy in ipairs(self.enemies) do
					if self:toTurnOver(enemy, n) and self:hasSkills(sgs.priority_skill, enemy) then
						target = enemy
						break
					end
				end
			end
			if not target then
				for _, enemy in ipairs(self.enemies) do
					if self:toTurnOver(enemy, n) then
						target = enemy
						break
					end
				end
			end
		end
	end

	return target
end

sgs.ai_skill_playerchosen.songwei = function(self, targets)
	targets = sgs.QList2Table(targets)
	for _, target in ipairs(targets) do
		if self:isFriend(target) and target:isAlive() then
			return target
		end
	end
	return nil
end

sgs.ai_playerchosen_intention.songwei = -50

sgs.ai_playerchosen_intention.fangzhu = function(self, from, to)
	if to:hasSkill("manjuan") and to:getPhase() == sgs.Player_NotActive then sgs.updateIntention(from, to, 80) end
	local intention = 80 / math.max(from:getLostHp(), 1)
	if not self:toTurnOver(to, from:getLostHp()) then intention = -intention end
	if from:getLostHp() < 3 then
		sgs.updateIntention(from, to, intention)
	else
		sgs.updateIntention(from, to, math.min(intention, -30))
	end
end

sgs.ai_need_damaged.fangzhu = function (self, attacker, player)
	if not player:hasSkill("fangzhu") then return false end
	local enemies = self:getEnemies(player)
	if #enemies < 1 then return false end
	self:sort(enemies, "defense")
	for _, enemy in ipairs(enemies) do
		if player:getLostHp() < 1 and self:toTurnOver(enemy, player:getLostHp() + 1) then
			return true
		end
	end
	local friends = self:getFriendsNoself(player)
	self:sort(friends, "defense")
	for _, friend in ipairs(friends) do
		if not self:toTurnOver(friend, player:getLostHp() + 1) then return true end
	end
	return false
end

sgs.ai_chaofeng.caopi = -3

duanliang_skill={}
duanliang_skill.name="duanliang"
table.insert(sgs.ai_skills,duanliang_skill)
duanliang_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)

	local card

	self:sortByUseValue(cards,true)

	for _,acard in ipairs(cards)  do
		if (acard:isBlack()) and (acard:isKindOf("BasicCard") or acard:isKindOf("EquipCard")) and (self:getDynamicUsePriority(acard)<sgs.ai_use_value.SupplyShortage)then
			card = acard
			break
		end
	end

	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("supply_shortage:duanliang[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)

	assert(skillcard)

	return skillcard
end

sgs.ai_cardneed.duanliang = function(to, card)
	return card:isBlack() and card:getTypeId() ~= sgs.Card_TypeTrick and getKnownCard(to, "black", false) < 2
end

sgs.duanliang_suit_value = {
	spade = 3.9,
	club = 3.9
}

sgs.ai_chaofeng.xuhuang = 4

sgs.ai_skill_invoke.zaiqi = function(self, data)
	return self.player:getLostHp() >= 2
end

sgs.ai_cardneed.lieren = function(to, card)
	return isCard("Slash", card, to) and getKnownCard(to, "Slash", true) == 0
end

sgs.ai_skill_invoke.lieren = function(self, data)
	local damage = data:toDamage()
	if not self:isEnemy(damage.to) then return false end

	if self.player:getHandcardNum() == 1 then
		if (self:needKongcheng() or not self:hasLoseHandcardEffective()) and not self:isWeak() then return true end
		local card  = self.player:getHandcards():first()
		if card:isKindOf("Jink") or card:isKindOf("Peach") then return end
	end
	
	if (self.player:getHandcardNum() >= self.player:getHp() or self:getMaxCard():getNumber() > 10 
		or (self:needKongcheng() and self.player:getHandcardNum() == 1) or not self:hasLoseHandcardEffective()) 
		and not self:doNotDiscard(damage.to, "h", true) and not (self.player:getHandcardNum() == 1 and self:doNotDiscard(damage.to, "e", true)) then
			return true
	end
	if self:doNotDiscard(damage.to, "he", true, 2) then return false end
	return false
end

function sgs.ai_skill_pindian.lieren(minusecard, self, requestor)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	if requestor:objectName() == self.player:objectName() then
		return cards[1]:getId()
	end
	return self:getMaxCard(self.player):getId()
end

sgs.ai_skill_choice.yinghun = function(self, choices)
	return self.yinghunchoice
end

sgs.ai_skill_playerchosen.yinghun = function(self, targets)
	local x = self.player:getLostHp()
	local n = x - 1
	self:updatePlayers()
	if x == 1 and #self.friends == 1 then
		for _, enemy in ipairs(self.enemies) do
			if enemy:hasSkill("manjuan") then
				return enemy
			end
		end
		return nil
	end

	self.yinghun = nil
	local player = self:AssistTarget()

	if x == 1 then
		self:sort(self.friends_noself, "handcard")
		self.friends_noself = sgs.reverse(self.friends_noself)
		for _, friend in ipairs(self.friends_noself) do
			if self:hasSkills(sgs.lose_equip_skill, friend) and friend:getCards("e"):length() > 0
			  and not friend:hasSkill("manjuan") then
				self.yinghun = friend
				break
			end
		end
		if not self.yinghun then
			for _, friend in ipairs(self.friends_noself) do
				if (friend:hasSkill("tuntian") and friend:hasSkill("zaoxian")) and not friend:hasSkill("manjuan") then
					self.yinghun = friend
					break
				end
			end
		end
		if not self.yinghun then
			for _, friend in ipairs(self.friends_noself) do
				if self:needToThrowArmor(friend) and not friend:hasSkill("manjuan") then
					self.yinghun = friend
					break
				end
			end
		end
		if not self.yinghun then
			for _, enemy in ipairs(self.enemies) do
				if enemy:hasSkill("manjuan") then
					return enemy
				end
			end
		end

		if not self.yinghun and player and not player:hasSkill("manjuan") and player:getCardCount(true) > 0 and not self:needKongcheng(player, true) then
			self.yinghun = player
		end

		if not self.yinghun then
			for _, friend in ipairs(self.friends_noself) do
				if friend:getCards("he"):length() > 0 and not friend:hasSkill("manjuan") then
					self.yinghun = friend
					break
				end
			end
		end

		if not self.yinghun then
			for _, friend in ipairs(self.friends_noself) do
				if not friend:hasSkill("manjuan") then
					self.yinghun = friend
					break
				end
			end
		end
	elseif #self.friends > 1 then
		self:sort(self.friends_noself, "chaofeng")
		for _, friend in ipairs(self.friends_noself) do
			if self:hasSkills(sgs.lose_equip_skill, friend) and friend:getCards("e"):length() > 0
			  and not friend:hasSkill("manjuan") then
				self.yinghun = friend
				break
			end
		end
		if not self.yinghun then
			for _, friend in ipairs(self.friends_noself) do
				if (friend:hasSkill("tuntian") and friend:hasSkill("zaoxian")) and not friend:hasSkill("manjuan") then
					self.yinghun = friend
					break
				end
			end
		end
		if not self.yinghun then
			for _, friend in ipairs(self.friends_noself) do
				if self:needToThrowArmor(friend) and not friend:hasSkill("manjuan") then
					self.yinghun = friend
					break
				end
			end
		end
		if not self.yinghun and #self.enemies > 0 then
			local wf
			if self.player:isLord() then
				if self:isWeak() and (self.player:getHp() < 2 and self:getCardsNum("Peach") < 1) then
					wf = true
				end
			end
			if not wf then
				for _, friend in ipairs(self.friends_noself) do
					if self:isWeak(friend) then
						wf = true 
						break 
					end
				end
			end

			if not wf then
				self:sort(self.enemies, "chaofeng")
				for _, enemy in ipairs(self.enemies) do
					if enemy:getCards("he"):length() == n
					  and not self:doNotDiscard(enemy, "he", true, n, true) then
						self.yinghunchoice = "d1tx"
						return enemy
					end
				end
				for _, enemy in ipairs(self.enemies) do
					if enemy:getCards("he"):length() >= n
					  and not self:doNotDiscard(enemy, "he", true, n, true)
					  and self:hasSkills(sgs.cardneed_skill, enemy) then
						self.yinghunchoice = "d1tx"
						return enemy
					end
				end
			end
		end

		if not self.yinghun and player and not player:hasSkill("manjuan") and not self:needKongcheng(player, true) then
			self.yinghun = player
		end

		if not self.yinghun then
			self.yinghun = self:findPlayerToDraw(false, n)
		end
		if not self.yinghun then
			for _, friend in ipairs(self.friends_noself) do
				if not friend:hasSkill("manjuan") then
					self.yinghun = friend
					break
				end
			end
		end
		if self.yinghun then self.yinghunchoice = "dxt1" end
	end
	if not self.yinghun and x > 1 and #self.enemies > 0 then
		self:sort(self.enemies, "handcard")
		for _, enemy in ipairs(self.enemies) do
			if enemy:getCards("he"):length() >= n 
			  and not self:doNotDiscard(enemy, "he", true, n, true) then
				self.yinghunchoice = "d1tx"
				return enemy
			end
		end
		self.enemies = sgs.reverse(self.enemies)
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isNude()
			  and not (self:hasSkills(sgs.lose_equip_skill, enemy) and enemy:getCards("e"):length() > 0)
			  and not (enemy:hasArmorEffect("SilverLion") and enemy:isWounded() and self:isWeak(enemy))
			  and not (enemy:hasSkill("tuntian") and enemy:hasSkill("zaoxian")) then
				self.yinghunchoice = "d1tx"
				return enemy
			end
		end
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isNude()
			  and not (self:hasSkills(sgs.lose_equip_skill, enemy) and enemy:getCards("e"):length() > 0)
			  and not (enemy:hasArmorEffect("SilverLion") and enemy:isWounded() and self:isWeak(enemy))
			  and not (enemy:hasSkill("tuntian") and enemy:hasSkill("zaoxian") and x < 3 and enemy:getCards("he"):length() < 2) then
				self.yinghunchoice = "d1tx"
				return enemy
			end
		end
	end

	return self.yinghun
end

sgs.ai_skill_choice.yinghun = function(self, choices)
	return self.yinghunchoice
end

sgs.ai_playerchosen_intention.yinghun = function(self, from, to)
	if from:getLostHp() > 1 then return end
	local intention = -80
	if to:hasSkill("manjuan") then intention = -intention end
	sgs.updateIntention(from, to, intention)
end

sgs.ai_choicemade_filter.skillChoice.yinghun = function(player, promptlist, self)
	local to
	for _, p in sgs.qlist(self.room:getOtherPlayers(player)) do
		if p:hasFlag("YinghunTarget") then
			to = p
			break
		end
	end
	local choice = promptlist[#promptlist]
	local intention = (choice == "dxt1") and -80 or 80
	sgs.updateIntention(player, to, intention)
end

local function getLowerBoundOfHandcard(self)
	local least = math.huge
	local players = self.room:getOtherPlayers(self.player)
	for _, player in sgs.qlist(players) do
		least = math.min(player:getHandcardNum(), least)
	end

	return least
end

local function getBeggar(self)
	local least = getLowerBoundOfHandcard(self)

	self:sort(self.friends_noself)
	for _, friend in ipairs(self.friends_noself) do
		if friend:getHandcardNum() == least then
			return friend
		end
	end

	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:getHandcardNum() == least then
			return player
		end
	end
end

sgs.ai_skill_invoke.haoshi = function(self, data)
	local extra = 0
	if self.player:hasSkill("yongsi") then
		local kingdoms = {}
		for _,p in sgs.qlist(self.room:getAlivePlayers()) do
			kingdoms[p:getKingdom()] = true
		end
		extra=extra+#kingdoms
	end
	local sk = {["yingzi"]=1, ["zishou"]=self.player:getLostHp(), ["ayshuijian"]=1+self.player:getEquips():length(),
	["shenwei"]=2, ["juejing"]=self.player:getLostHp()}
	for s,n in ipairs(sk) do
		if self.player:hasSkill(s) then
			extra = extra+n
		end
	end
	if self.player:getHandcardNum()+extra <= 1 then
		return true
	end

	local beggar = getBeggar(self)
	return self:isFriend(beggar) and not beggar:hasSkill("manjuan")
end

sgs.ai_skill_use["@@haoshi!"] = function(self, prompt)
	local beggar = getBeggar(self)

	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
	local card_ids = {}
	for i=1, math.floor(#cards/2) do
		table.insert(card_ids, cards[i]:getEffectiveId())
	end

	return "@HaoshiCard=" .. table.concat(card_ids, "+") .. "->" .. beggar:objectName()
end

sgs.ai_card_intention.HaoshiCard = -80

function sgs.ai_cardneed.haoshi(to, card, self)
	return not self:willSkipDrawPhase(to)
end

dimeng_skill = {}
dimeng_skill.name = "dimeng"
table.insert(sgs.ai_skills, dimeng_skill)
dimeng_skill.getTurnUseCard = function(self)
	if self:needBear() then return end
	if self.player:hasUsed("DimengCard") then return end
	card = sgs.Card_Parse("@DimengCard=.")
	return card

end

--要求：mycards是经过sortByKeepValue排序的--
function DimengIsWorth(self, friend, enemy, mycards, myequips)
	local hand1 = enemy:getHandcardNum()
	local hand2 = friend:getHandcardNum()
	if hand1 < hand2 then
		return false
	elseif hand1 == hand2 and hand1 > 0 then
		return friend:hasSkill("tuntian") and friend:hasSkill("zaoxian")
	end
	local cardNum = #mycards
	local delt = hand1 - hand2 --assert: delt>0
	if delt > cardNum then
		return false
	end
	local equipNum = #myequips
	if equipNum > 0 then
		if self:hasSkills("xuanfeng|xiaoji|nosxuanfeng") then
			return true
		end
	end
	--now hand1>hand2 and delt<=cardNum
	local soKeep = 0
	local soUse = 0
	local marker = math.ceil(delt / 2)
	for i=1, delt, 1 do
		local card = mycards[i]
		local keepValue = self:getKeepValue(card)
		if keepValue > 4 then
			soKeep = soKeep + 1
		end
		local useValue = self:getUseValue(card)
		if useValue > 7 then
			soUse = soUse + 1
		end
	end
	if soKeep > marker then
		return false
	end
	if soUse > marker then
		return false
	end
	return true
end

--缔盟的弃牌策略--
local dimeng_discard = function(self, discard_num)
	local cards = self.player:getCards("he")
	local to_discard = {}
	cards = sgs.QList2Table(cards)
	
	local aux_func = function(card)
		local place = self.room:getCardPlace(card:getEffectiveId())
		if place == sgs.Player_PlaceEquip then
			if card:isKindOf("SilverLion") and self.player:isWounded() then return -2
			elseif card:isKindOf("OffensiveHorse") then return 1
			elseif card:isKindOf("Weapon") then return 2
			elseif card:isKindOf("DefensiveHorse") then return 3
			elseif card:isKindOf("Armor") then return 4
			end
		elseif self:getUseValue(card) > 7 then return 3 --使用价值高的牌，如顺手牵羊(9)
		elseif self:hasSkills(sgs.lose_equip_skill) then return 5
		else return 0
		end
		return 0
	end
	
	local compare_func = function(a, b)
		if aux_func(a) ~= aux_func(b) then 
			return aux_func(a) < aux_func(b) 
		end
		return self:getKeepValue(a) < self:getKeepValue(b)
	end

	table.sort(cards, compare_func)
	for _, card in ipairs(cards) do
		if not self.player:isJilei(card) then table.insert(to_discard, card:getId()) end
		if #to_discard >= discard_num then break end
	end
	if #to_discard ~= discard_num then return {} end
	return to_discard
end

sgs.ai_skill_use_func.DimengCard=function(card,use,self)
	local cardNum = 0
	local mycards = {}
	local myequips = {}
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if not self.player:isJilei(c) then 
			cardNum = cardNum + 1 
			table.insert(mycards, c)
		end
	end
	for _, c in sgs.qlist(self.player:getEquips()) do
		if not self.player:isJilei(c) then 
			cardNum = cardNum + 1 
			table.insert(mycards, c)
			table.insert(myequips, c)
		end
	end
	self:sortByKeepValue(mycards) --桃的keepValue是5，useValue是6；顺手牵羊的keepValue是1.9，useValue是9

	self:sort(self.enemies,"handcard")
	local friends={}
	for _,player in ipairs(self.friends_noself) do
		if not player:hasSkill("manjuan") then
			table.insert(friends, player)
		end
	end
	if #friends == 0 then return end
	self:sort(friends,"handcard")

	local lowest_friend=friends[1]

	self:sort(self.enemies,"defense")
	if lowest_friend then
		local hand2=lowest_friend:getHandcardNum()
		for _,enemy in ipairs(self.enemies) do
			local hand1=enemy:getHandcardNum()

			if enemy:hasSkill("manjuan") and (hand1 > hand2 - 1) and (hand1 - hand2) <= cardNum then
				if hand1 == hand2 then
					use.card = card
				else
					local discard_num = hand1 - hand2
					local discards = dimeng_discard(self, discard_num)
					if #discards > 0 then use.card = sgs.Card_Parse("@DimengCard=" .. table.concat(discards, "+")) end
				end
				if use.card and use.to then
					use.to:append(enemy)
					use.to:append(lowest_friend)
				end
				return
			end
		end
		for _, enemy in ipairs(self.enemies) do
			local hand1=enemy:getHandcardNum()
			if DimengIsWorth(self, lowest_friend, enemy, mycards, myequips) then
				if hand1 == hand2 then
					use.card = card
				else
					local discard_num = math.abs(hand1 - hand2)
					local discards = dimeng_discard(self, discard_num)
					if #discards > 0 then use.card = sgs.Card_Parse("@DimengCard=" .. table.concat(discards, "+")) end
				end
				if use.to then
					use.to:append(enemy)
					use.to:append(lowest_friend)
				end
				return
			end
		end
	end
end

sgs.ai_card_intention.DimengCard = function(self,card, from, to)
	local compare_func = function(a, b)
		return a:getHandcardNum() < b:getHandcardNum()
	end
	table.sort(to, compare_func)
	if to[1]:getHandcardNum() < to[2]:getHandcardNum() then
		sgs.updateIntention(from, to[1], -80)
	end
end

sgs.ai_use_value.DimengCard = 3.5
sgs.ai_use_priority.DimengCard = 2.8

sgs.dynamic_value.control_card.DimengCard = true

sgs.ai_chaofeng.lusu = 4

luanwu_skill = {}
luanwu_skill.name = "luanwu"
table.insert(sgs.ai_skills, luanwu_skill)
luanwu_skill.getTurnUseCard = function(self)
	if self.player:getMark("@chaos") <= 0 then return end
	local good, bad = 0, 0
	local lord = self.room:getLord()
	if lord and self.role ~= "rebel" and self:isWeak(lord) then return end
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self:isWeak(player) then
			if self:isFriend(player) then bad = bad + 1
			else good = good + 1
			end
		end
	end
	if good == 0 then return end

	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		local hp = math.max(player:getHp(), 1)
		if getCardsNum("Analeptic", player) > 0 then
			if self:isFriend(player) then good = good + 1.0 / hp
			else bad = bad + 1.0 / hp
			end
		end

		local has_slash = (getCardsNum("Slash", player) > 0)
		local can_slash = false
		if not can_slash then
			for _, p in sgs.qlist(self.room:getOtherPlayers(player)) do
				if player:distanceTo(p) <= player:getAttackRange() then can_slash = true break end
			end
		end
		if not has_slash or not can_slash then
			if self:isFriend(player) then good = good + math.max(getCardsNum("Peach", player), 1)
			else bad = bad + math.max(getCardsNum("Peach", player), 1)
			end
		end

		if getCardsNum("Jink", player) == 0 then
			local lost_value = 0
			if self:hasSkills(sgs.masochism_skill, player) then lost_value = player:getHp()/2 end
			local hp = math.max(player:getHp(), 1)
			if self:isFriend(player) then bad = bad + (lost_value + 1) / hp
			else good = good + (lost_value + 1) / hp
			end
		end
	end

	if good > bad then return sgs.Card_Parse("@LuanwuCard=.") end
end

sgs.ai_skill_use_func.LuanwuCard=function(card,use,self)
	use.card = card
end

sgs.ai_skill_playerchosen.luanwu = sgs.ai_skill_playerchosen.zero_card_as_slash

sgs.dynamic_value.damage_card.LuanwuCard = true

jiuchi_skill={}
jiuchi_skill.name="jiuchi"
table.insert(sgs.ai_skills,jiuchi_skill)
jiuchi_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)

	local card

	self:sortByUseValue(cards,true)

	for _,acard in ipairs(cards)  do
		if acard:getSuit() == sgs.Card_Spade then
			card = acard
			break
		end
	end

	if not card then return nil end
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("analeptic:jiuchi[spade:%s]=%d"):format(number, card_id)
	local analeptic = sgs.Card_Parse(card_str)

	if sgs.Analeptic_IsAvailable(self.player, analeptic) then
		assert(analeptic)
		return analeptic
	end
end

sgs.ai_view_as.jiuchi = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place == sgs.Player_PlaceHand then
		if card:getSuit() == sgs.Card_Spade then
			return ("analeptic:jiuchi[%s:%s]=%d"):format(suit, number, card_id)
		end
	end
end

function sgs.ai_cardneed.jiuchi(to, card, self)
	return card:getSuit() == sgs.Card_Spade and (getKnownCard(to, "club", false) + getKnownCard(to, "spade", false)) == 0
end

function sgs.ai_cardneed.roulin(to, card, self)
	for _, enemy in ipairs(self.enemies) do
		if card:isKindOf("Slash") and to:canSlash(enemy, nil, true) and self:slashIsEffective(card, enemy) 
				and not (enemy:hasSkill("kongcheng") and enemy:isKongcheng())
				and sgs.isGoodTarget(enemy, self.enemies, self) and not self:slashProhibit(card, enemy) and enemy:isFemale() then
			return getKnownCard(to, "Slash", true) == 0
		end
	end
end


sgs.ai_skill_cardask["@roulin1-jink-1"] = sgs.ai_skill_cardask["@wushuang-jink-1"]
sgs.ai_skill_cardask["@roulin2-jink-1"] = sgs.ai_skill_cardask["@wushuang-jink-1"]

sgs.ai_skill_choice.benghuai = function(self, choices, data)
	for _, friend in ipairs(self.friends) do
		if friend:hasSkill("tianxiang") and (self.player:getHp() >= 3 or (self:getCardsNum("Peach") + self:getCardsNum("Analeptic") > 0 and self.player:getHp() > 1)) then
			return "hp"
		end
	end
	if self.player:getMaxHp() >= self.player:getHp() + 2 then
		if self.player:getMaxHp() > 5 and (self.player:hasSkills("nosmiji|yinghun") or self.player:hasSkill("miji") and self:findPlayerToDraw(false)) then
			local enemy_num = 0
			for _, p in ipairs(self.enemies) do
				if p:inMyAttackRange(self.player) and not self:willSkipPlayPhase(p) then enemy_num = enemy_num + 1 end
			end
			local ls = sgs.fangquan_effect and self.room:findPlayerBySkillName("fangquan")
			if ls then
				sgs.fangquan_effect = false
				enemy_num = self:getEnemyNumBySeat(ls, self.player, self.player)
			end
			local least_hp = isLord(self.player) and math.max(2, enemy_num - 1) or 1
			if (self:getCardsNum("Peach") + self:getCardsNum("Analeptic") + self.player:getHp() > least_hp) then return "hp" end
		end
		return "maxhp"
	else
		return "hp"
	end
end

sgs.ai_skill_playerchosen.baonue = function(self, targets)
	targets = sgs.QList2Table(targets)
	for _, target in ipairs(targets) do
		if self:isFriend(target) and target:isAlive() then
			if target:isWounded() then
				return target
			end
			local zhangjiao = self.room:findPlayerBySkillName("guidao")
			if zhangjiao and self:isFriend(zhangjiao) then
				return target
			end
		end
	end
	return nil
end

sgs.ai_playerchosen_intention.baonue = -40

sgs.jiuchi_suit_value = {
	spade = 5,
}

sgs.ai_suit_priority.jiuchi= "diamond|heart|club|spade"
sgs.ai_suit_priority.duanliang= "club|spade|diamond|heart"
