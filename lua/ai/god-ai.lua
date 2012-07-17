wushen_skill={}
wushen_skill.name="wushen"
table.insert(sgs.ai_skills,wushen_skill)
wushen_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)

	local red_card

	self:sortByUseValue(cards,true)

	for _,card in ipairs(cards)  do
		if card:getSuitString()=="heart" then--and (self:getUseValue(card)<sgs.ai_use_value.Slash) then
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
	for _, player in ipairs(targetlist) do
		if self:isEnemy(player) and (not target or target:getHp() < player:getHp()) then
			target = player
		end
	end
	if target then return target end
	self:sort(targetlist, "hp")
	if self.player:getRole() == "loyalist" and targetlist[1]:isLord() then return targetlist[2] end
	return targetlist[1]
end

function sgs.ai_slash_prohibit.wuhun(self, to)
    if self:hasSkills("jueqing|qianxi") then return false end
	local maxfriendmark = 0
	local maxenemymark = 0
	for _, friend in ipairs(self.friends) do
		local friendmark = friend:getMark("@nightmare")
		if friendmark > maxfriendmark then maxfriendmark = friendmark end
	end
	for _, enemy in ipairs(self.enemies) do
		local enemymark = enemy:getMark("@nightmare")
		if enemymark > maxenemymark and enemy:objectName() ~= to:objectName() then maxenemymark = enemymark end
	end
	if self:isEnemy(to) and not (to:isLord() and self.player:getRole() == "rebel") then
		if (maxfriendmark+2 > maxenemymark) and not (#self.enemies==1 and #self.friends + #self.enemies == self.room:alivePlayerCount()) then 
			if not (self.player:getMark("@nightmare") == maxfriendmark and  self.role == "loyalist") then
				return true
			end
		end
	end
end

function SmartAI:cantbeHurt(player)
	if self.player:hasSkill("jueqing") then return false end
	local maxfriendmark = 0
	local maxenemymark = 0
	local dyingfriend = 0
	if player:hasSkill("wuhun") then
		for _, friend in ipairs(self.friends) do
			local friendmark = friend:getMark("@nightmare")
			if friendmark > maxfriendmark then maxfriendmark = friendmark end
		end
		for _, enemy in ipairs(self.enemies) do
			local enemymark = enemy:getMark("@nightmare")
			if enemymark > maxenemymark and enemy:objectName() ~= player:objectName() then maxenemymark = enemymark end
		end
		if self:isEnemy(player) and not (player:isLord() and self.player:getRole() == "rebel") then
			if (maxfriendmark+2 > maxenemymark) and not (#self.enemies==1 and #self.friends + #self.enemies == self.room:alivePlayerCount()) then 
				if not (self.player:getMark("@nightmare") == maxfriendmark and self.role == "loyalist") then
					return true
				end
			end
		elseif maxfriendmark+1 > maxenemymark then 
			return true
		end
	elseif player:hasSkill("duanchang") then
		if player:getHp() < 2 then
			if self:isFriend(player) then
				return true
			elseif #self.enemies > 2 then
				return true
			end
		end
	elseif player:hasSkill("tianxiang") then
		for _, friend in ipairs(self.friends) do
			if friend:getHp() < 2 and self:getCardsNum("Peach") == 0 then
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
	if player:hasSkill("wuhun") then
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
	if (player:hasSkill("niepan") and player:getMark("@nirvana") > 0 and player:getCards("e"):length() < 2) or 
		(player:hasSkill("fuli") and player:getMark("@laoji") > 0 and player:getCards("e"):length() < 2) then
		return true
	end
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
	if self.player:usedTimes("GongxinCard")>0 then return end
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

-- for some reason unknown, this doesn't work
--[[sgs.ai_skill_cardchosen.gongxin = function(self, who, flags)
global_room:writeToConsole("we are in !!!!!!!!!")
	local cards = self.player:getHandcards()	
	cards=sgs.QList2Table(cards)
    local has_jink
	local has_peach
	local has_null
    local heartnum = 0
	local hasindul = 0
	local nextplayer = self.player:getNextAlive()

	local judges = who:getJudgingArea()
	if not judges:isEmpty() then
		for _, judge in sgs.qlist(judges) do
			card = sgs.Sanguosha:getCard(judge:getEffectiveId())
			if card:isKindOf("Indulgence") then
				hasindul = 1
				break
			end
		end
	end
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Heart or (card:getSuit() == sgs.Card_Spade and who:hasSkill("hongyan")) then
	        has_null = card
            if card:isKindOf("Jink") then
                has_jink = card
				global_room:writeToConsole("has jink !!!!!!!!!")
            elseif card:isKindOf("Peach") then
                has_peach = card
				global_room:writeToConsole("has peach !!!!!!!!!")
			end
            heartnum = heartnum + 1
		end
	end	
	global_room:writeToConsole("heartnum = " .. heartnum)
	if self:isFriend(nextplayer) and has_peach then
		self.room:setPlayerFlag(self.player, "gongxin_put")
		global_room:writeToConsole("help friend  !!!!!!!!!")
		return has_peach:getEffectiveId()
	elseif who:objectName() == nextplayer:objectName() and has_jink and hasindul < 1 then
		self.room:setPlayerFlag(self.player, "gongxin_put")
		global_room:writeToConsole("jink cycle  !!!!!!!!!")
		return has_jink:getEffectiveId()
	elseif self:isFriend(nextplayer) and hasindul > 0 and has_null then
		self.room:setPlayerFlag(self.player, "gongxin_put")
		global_room:writeToConsole("indul ? OK  !!!!!!!!!")
		return has_null:getEffectiveId()
	elseif has_null then
		global_room:writeToConsole("Normal discard  !!!!!!!!!")
		return has_null:getEffectiveId()
	end
	return "."
end

sgs.ai_skill_choice.gongxin = function(self, choices)
	if self.player:hasFlag("gongxin_put") then
		self.room:setPlayerFlag(self.player, "-gongxin_put")
		return "put"
	end	
	return "discard"
end]]

sgs.ai_use_value.GongxinCard = 8.5
sgs.ai_use_priority.GongxinCard = 8.8
sgs.ai_card_intention.GongxinCard = 80

sgs.ai_skill_invoke.qinyin = function(self, data)
	for _,friend in ipairs(self.friends) do
		if friend:isWounded() then return true end
	end
	if sgs.ai_skill_choice.qinyin(self,"up+down")=="down" then return true end
	return false
end

sgs.ai_skill_choice.qinyin = function(self, choices)
	self:sort(self.friends, "hp")
	self:sort(self.enemies, "hp")
	if self.friends[1]:getHp() >= self.enemies[1]:getHp() and self:getAllPeachNum(self.player) > self:getAllPeachNum(self.enemies[1]) then
		return "down"
	else
		return "up"
	end
end

local yeyan_skill={}
yeyan_skill.name = "yeyan"
table.insert(sgs.ai_skills, yeyan_skill)
yeyan_skill.getTurnUseCard=function(self)
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
				if (enemy:getArmor() and enemy:getArmor():objectName() == "vine") or (enemy:isChained() and self:isGoodChainTarget(enemy)) then
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
		if ((self:isEquip("Vine", enemy) or enemy:getMark("@gale") > 0) or enemy:getHp() <= 1) 
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
		if not (enemy:getArmor() and enemy:getArmor():objectName() == "silver_lion") and
			not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) and
			self:objectiveLevel(enemy) > 3 and enemy:getMark("@fog") < 1 then
				if enemy:isChained() and self:isGoodChainTarget(enemy) then
					if enemy:getArmor() and enemy:getArmor():objectName() == "vine" then
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
		if not (enemy:getArmor() and enemy:getArmor():objectName() == "silver_lion") and
			not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) 
			and self:objectiveLevel(enemy) > 3 and enemy:getMark("@fog") < 1 then
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
		if not (enemy:getArmor() and enemy:getArmor():objectName() == "silver_lion") and
			not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) 
			and self:objectiveLevel(enemy) > 3 and enemy:getMark("@fog") < 1 then
				if not enemy:isChained() then
					if enemy:getArmor() and enemy:getArmor():objectName() == "vine" then
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
		if not (enemy:getArmor() and enemy:getArmor():objectName() == "silver_lion") and
			not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) 
			and self:objectiveLevel(enemy) > 3 and enemy:getMark("@fog") < 1 then
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
		if not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) and enemy:getMark("@fog") < 1 then
			if enemy:isChained() and self:isGoodChainTarget(enemy) then
				if enemy:getArmor() and enemy:getArmor():objectName() == "vine" then
					if use.to then use.to:append(enemy) end
					num = num + 1
					if num >=3 then break end
				end
			end
		end
	end
	if num < 3 then
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) and enemy:getMark("@fog") < 1 then
				if enemy:isChained() and self:isGoodChainTarget(enemy) and not (enemy:getArmor() and enemy:getArmor():objectName() == "vine") then
					if use.to then use.to:append(enemy) end
					num = num + 1
					if num >=3 then break end
				end
			end
		end
	end	
	if num < 3 then
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) and enemy:getMark("@fog") < 1 then
				if not enemy:isChained() then
					if enemy:getArmor() and enemy:getArmor():objectName() == "vine" then
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
			if not (enemy:hasSkill("tianxiang") and enemy:getHandcardNum() > 0) and enemy:getMark("@fog") < 1 then
				if not enemy:isChained() and not (enemy:getArmor() and enemy:getArmor():objectName() == "vine") then
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
	for _, friend in ipairs(self.friends) do
		if friend:hasSkill("huoji") or self:isEquip("Fan",friend) or (friend:hasSkill("smallyeyan") and friend:getMark("@flame")>0) then
			friendly_fire = true
			break
		end
	end

	local is_chained = 0
	local target = {}
	for _, enemy in ipairs(self.enemies) do
		if enemy:isChained() then
			is_chained = is_chained + 1
			table.insert(target, enemy)
		end
		if enemy:getArmor() and enemy:getArmor():objectName() == "vine" then
			table.insert(target, 1, enemy)
			break
		end
	end
	local usecard=false
	if friendly_fire and is_chained > 1 then usecard=true end
	self:sort(self.friends, "hp")
	if target[1] and not self:isWeak(self.friends[1]) then
		if target[1]:getArmor() and target[1]:getArmor():objectName() == "vine" and friendly_fire then usecard=true end
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
	if self:isFriend(lord) and not sgs.isLordHealthy() and not self.player:isLord() and not lord:hasSkill("buqu") then table.insert(targets, lord:objectName())
	else
		for _, friend in ipairs(self.friends_noself) do
			if self:isWeak(friend) and not friend:hasSkill("buqu") then table.insert(targets, friend:objectName()) break end
		end	
	end
	if self.player:getMark("@star") > #targets and self:isWeak() then table.insert(targets, self.player:objectName()) end
	if #targets > 0 then return "@DawuCard=.->" .. table.concat(targets, "+") end
	return "."
end

sgs.ai_card_intention.DawuCard = -70

sgs.ai_skill_invoke.guixin = function(self,data)
	return self.room:alivePlayerCount() > 2
end

sgs.ai_chaofeng.shencaocao = -6

sgs.ai_skill_choice.wumou = function(self, choices)
	if self.player:getHp() + self:getCardsNum("Peach") > 3 then return "losehp"
	else return "discard"
	end
end

local wuqian_skill={}
wuqian_skill.name = "wuqian"
table.insert(sgs.ai_skills, wuqian_skill)
wuqian_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("WuqianCard") or self.player:getMark("@wrath") < 2 then return end

	local card_str = ("@WuqianCard=.")
	self:sort(self.enemies, "hp")
	local has_enemy
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() <= 2 and self:getCardsNum("Jink", enemy) < 2 and self.player:distanceTo(enemy) <= self.player:getAttackRange() then 
			has_enemy = enemy break end
	end

	if has_enemy and self:getCardsNum("Slash") > 0 then
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if card:isKindOf("Slash") and self:slashIsEffective(card, has_enemy) and self.player:canSlash(has_enemy) and
				(self:getCardsNum("Analeptic") > 0 or has_enemy:getHp() <= 1) and card:isAvailable(self.player) then return sgs.Card_Parse(card_str)
			elseif card:isKindOf("Duel") then return sgs.Card_Parse(card_str)
			end
		end
	end
end

sgs.ai_skill_use_func.WuqianCard=function(card,use,self)
	self:sort(self.enemies,"hp")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() <= 2 and self:getCardsNum("Jink", enemy) < 2 and self.player:distanceTo(enemy) <= self.player:getAttackRange() then
			if use.to then
				use.to:append(enemy)
			end
			use.card = card
			return
		end
	end
end

sgs.ai_card_intention.WuqianCard = 80

local shenfen_skill={}
shenfen_skill.name = "shenfen"
table.insert(sgs.ai_skills, shenfen_skill)
shenfen_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("ShenfenCard") or self.player:getMark("@wrath") < 6 then return end
	return sgs.Card_Parse("@ShenfenCard=.")
end

sgs.ai_skill_use_func.ShenfenCard=function(card,use,self)
	if self:isFriend(self.room:getLord()) and self:isWeak(self.room:getLord()) and not self.player:isLord() then return end
	use.card = card
end

sgs.ai_use_value.ShenfenCard = 8
sgs.ai_use_priority.ShenfenCard = 2.3

sgs.dynamic_value.damage_card.ShenfenCard = true
sgs.dynamic_value.control_card.ShenfenCard = true

local longhun_skill={}
longhun_skill.name="longhun"
table.insert(sgs.ai_skills, longhun_skill)
longhun_skill.getTurnUseCard = function(self)
	if self.player:getHp()>1 then return end
	local cards=sgs.QList2Table(self.player:getCards("he"))
	self:sortByUseValue(cards,true)
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Diamond then
			return sgs.Card_Parse(("fire_slash:longhun[%s:%s]=%d"):format(card:getSuitString(),card:getNumberString(),card:getId()))
		end
	end
end

sgs.ai_view_as.longhun = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if player:getHp() > 1 then return end
	if card:getSuit() == sgs.Card_Diamond then
		return ("fire_slash:longhun[%s:%s]=%d"):format(suit, number, card_id)
	elseif card:getSuit() == sgs.Card_Club then
		return ("jink:longhun[%s:%s]=%d"):format(suit, number, card_id)
	elseif card:getSuit() == sgs.Card_Heart then
		return ("peach:longhun[%s:%s]=%d"):format(suit, number, card_id)
	elseif card:getSuit() == sgs.Card_Spade then
		return ("nullification:longhun[%s:%s]=%d"):format(suit, number, card_id)
	end
end

sgs.shenzhaoyun_suit_value = 
{
	heart = 6.7,
	spade = 5,
	club = 4.2,
	diamond = 3.9,
}

sgs.ai_skill_invoke.lianpo = true

function SmartAI:needBear(player)
    player = player or self.player
    return player:hasSkill("renjie") and not player:hasSkill("jilve") and player:getMark("@bear") < 4
end

sgs.ai_skill_invoke.jilve=function(self,data)
	local n=self.player:getMark("@bear")
	local use=(n>2 or self:getOverflow()>0)
	local event = self.player:getMark("JilveEvent")
	if event == sgs.AskForRetrial then
		local judge = data:toJudge()
		if not self:needRetrial(judge) then return false end
		return (use or judge.who == self.player or judge.reason == "lightning") and 
		        self:getRetrialCardId(sgs.QList2Table(self.player:getHandcards()), judge) ~= -1
	elseif event == sgs.Damaged then
		if #self.enemies == 0 then return false end
		return use and self:askForUseCard("@@fangzhu","@fangzhu")~="."
	elseif event == sgs.CardUsed or event == sgs.CardResponsed then
		local card = data:toCard()
		card = card or data:toCardUse().card
		return use or card:isKindOf("ExNihilo")
	else
		assert(false)
	end
end

local jilve_skill={}
jilve_skill.name="jilve"
table.insert(sgs.ai_skills,jilve_skill)
jilve_skill.getTurnUseCard=function(self)
    if self.player:getMark("@bear")<1 or self.player:usedTimes("JilveCard") > 2 then return end
	local wanshadone = self.player:getTag("JilveWansha"):toPlayer()
	if not wanshadone then
	    local cards=self.player:getHandcards()
	    cards=sgs.QList2Table(cards)
	    local slashes = self:getCards("Slash")
	    self:sort(self.enemies, "hp")
		local target
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("kongcheng") and enemy:isKongcheng()) and self:isWeak(enemy) and self:damageMinusHp(self, enemy, 1) > 0 then
				 sgs.ai_skill_choice.jilve="wansha" 
			     local wanshacard = sgs.Card_Parse("@JilveCard=.")
	             dummy_use={isDummy=true}
			     self:useSkillCard(wanshacard,dummy_use)
	             return sgs.Card_Parse("@JilveCard=.") 
			end
		end
	end
	if not self.player:hasUsed("ZhihengCard") and not wanshadone then
	   sgs.ai_skill_choice.jilve="zhiheng" 
	   local card=sgs.Card_Parse("@ZhihengCard=.")
	   local dummy_use={isDummy=true}
	   self:useSkillCard(card,dummy_use)
	   if dummy_use.card then return sgs.Card_Parse("@JilveCard=.") end
	elseif not wanshadone then
	    local cards=self.player:getHandcards()
	    cards=sgs.QList2Table(cards)
	    local slashes = self:getCards("Slash")
	    self:sort(self.enemies, "hp")
		local target
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("kongcheng") and enemy:isKongcheng()) and self:isWeak(enemy) and self:damageMinusHp(self, enemy, 1) > 0 then
				 sgs.ai_skill_choice.jilve="wansha" 
			     local wanshacard = sgs.Card_Parse("@JilveCard=.")
	             dummy_use={isDummy=true}
			     self:useSkillCard(wanshacard,dummy_use)
	             return sgs.Card_Parse("@JilveCard=.") 
			end
		end
	end
end

sgs.ai_skill_use_func.JilveCard=function(card,use,self)
	use.card = card
end

if sgs.ai_skill_choice.jilve == "zhiheng" or not sgs.ai_skill_choice.jilve == "wansha" then sgs.ai_use_priority.JilveCard = 3
else sgs.ai_use_priority.JilveCard = 6 end

sgs.ai_skill_use["@zhiheng"]=function(self,prompt)
	local card=sgs.Card_Parse("@ZhihengCard=.")
	local dummy_use={isDummy=true}
	self:useSkillCard(card,dummy_use)
	if dummy_use.card then return (dummy_use.card):toString() .. "->." end
	return "."
end
