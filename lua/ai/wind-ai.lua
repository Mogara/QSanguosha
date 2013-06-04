sgs.ai_skill_use["@@shensu1"]=function(self,prompt)
	self:updatePlayers()
	self:sort(self.enemies,"defense")
	if self.player:containsTrick("lightning") and self.player:getCards("j"):length()==1
	  and self:hasWizard(self.friends) and not self:hasWizard(self.enemies, true) then 
		return "."
	end
	
	if self:needBear() then return "." end

	local selfSub = self.player:getHp() - self.player:getHandcardNum()
	local selfDef = sgs.getDefense(self.player)
	
	for _,enemy in ipairs(self.enemies) do
		local def = sgs.getDefense(enemy)
		local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
		local eff = self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self)
			
		if not self.player:canSlash(enemy, slash, false) then
		elseif self:slashProhibit(nil, enemy) then
		elseif def < 6 and eff then return "@ShensuCard=.->"..enemy:objectName()

		elseif selfSub >= 2 then return "."
		elseif selfDef < 6 then return "." end	
	end
	
	for _,enemy in ipairs(self.enemies) do
		local def=sgs.getDefense(enemy)
		local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
		local eff = self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self)

		if not self.player:canSlash(enemy, slash, false) then
		elseif self:slashProhibit(nil, enemy) then
		elseif eff and def < 8 then return "@ShensuCard=.->"..enemy:objectName()
		else return "." end
	end
	return "."
end

sgs.ai_get_cardType = function(card)
	if card:isKindOf("Weapon") then return 1 end
	if card:isKindOf("Armor") then return 2 end
	if card:isKindOf("DefensiveHorse") then return 3 end
	if card:isKindOf("OffensiveHorse")then return 4 end
end

sgs.ai_skill_use["@@shensu2"] = function(self, prompt)
	self:updatePlayers()
	self:sort(self.enemies, "defenseSlash")
	
	local selfSub = self.player:getHp() - self.player:getHandcardNum()
	local selfDef = sgs.getDefense(self.player)
	
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	
	local eCard
	local hasCard = {0, 0, 0, 0}
	
	if self:needToThrowArmor() then
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
				if card:isKindOf("EquipCard") and sgs.ai_get_cardType(card) > 3 then
					eCard = card
					break
				end
			end
		end
		if not eCard then
			for _, card in ipairs(cards) do
				if card:isKindOf("EquipCard") and not card:isKindOf("Armor") then
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
		local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
		local eff = self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self)

		if not self.player:canSlash(enemy, slash, false) then
		elseif throw_weapon and enemy:hasArmorEffect("Vine") and not self.player:hasSkill("zonghuo") then
		elseif self:slashProhibit(nil, enemy) then
		elseif eff then
			if enemy:getHp() == 1 and getCardsNum("Jink", enemy) == 0 then best_target = enemy break end
			if def < defense then
				best_target = enemy
				defense = def
			end
			target = enemy
		end
		if selfSub < 0 then return "." end
	end
	
	if best_target then return "@ShensuCard="..eCard:getEffectiveId().."->"..best_target:objectName() end
	if target then return "@ShensuCard="..eCard:getEffectiveId().."->"..target:objectName() end
	
	return "."
end

sgs.ai_cardneed.shensu = function(to, card)
	return card:getTypeId() == sgs.Card_Equip and getKnownCard(to, "EquipCard", false) < 2
end

sgs.ai_card_intention.ShensuCard = 80

sgs.shensu_keep_value = sgs.xiaoji_keep_value

function sgs.ai_skill_invoke.jushou(self, data)
	if not self.player:faceUp() then return true end
	for _, friend in ipairs(self.friends) do
		if self:hasSkills("fangzhu|jilve", friend) then return true end
		if friend:hasSkill("junxing") and friend:faceUp() and not self:willSkipPlayPhase(friend)
			and not (friend:isKongcheng() and self:willSkipDrawPhase(friend)) then
			return true
		end
	end
	return self:isWeak()
end

function sgs.ai_cardneed.liegong(to, card)
	return (isCard("Slash", card, to) and getKnownCard(to, "Slash", true) == 0) or (card:isKindOf("Weapon") and not (to:getWeapon() or getKnownCard(to, "Weapon", false) > 0))
end

sgs.ai_skill_invoke.liegong = function(self, data)
	local target = data:toPlayer()
	return not self:isFriend(target)
end

function SmartAI:canLiegong(to, from)
	from = from or self.room:getCurrent()
	to = to or self.player
	if not from then return false end
	if from:hasSkill("liegong") and from:getPhase() == sgs.Player_Play and (to:getHandcardNum() >= from:getHp() or to:getHandcardNum() <= from:getAttackRange()) then return true end
	if from:hasSkill("kofliegong") and from:getPhase() == sgs.Player_Play and to:getHandcardNum() >= from:getHp() then return true end
	return false
end

sgs.ai_chaofeng.huangzhong = 1
sgs.ai_chaofeng.weiyan = -2

sgs.ai_skill_cardask["@guidao-card"]=function(self, data)
	local judge = data:toJudge()
	local all_cards = self.player:getCards("he")
	if all_cards:isEmpty() then return "." end
	local cards = {}
	for _, card in sgs.qlist(all_cards) do
		if card:isBlack() and not card:hasFlag("using") then
			table.insert(cards, card)
		end
	end

	if #cards == 0 then return "." end
	local card_id = self:getRetrialCardId(cards, judge)
	if card_id == -1 then
		if self:needRetrial(judge) and judge.reason ~= "beige" then
			self:sortByUseValue(cards, true)
			if self:getUseValue(judge.card) > self:getUseValue(cards[1]) then
				return "$" .. cards[1]:getId()
			end
		end
	elseif self:needRetrial(judge) or self:getUseValue(judge.card) > self:getUseValue(sgs.Sanguosha:getCard(card_id)) then
		local card = sgs.Sanguosha:getCard(card_id)
		return "$" .. card_id
	end
	
	return "."
end

function sgs.ai_cardneed.guidao(to, card, self)
	for _, player in sgs.qlist(self.room:getAllPlayers()) do
		if self:getFinalRetrial(to) == 1 then 
			if player:containsTrick("lightning") and not player:containsTrick("YanxiaoCard") then
				return card:getSuit() == sgs.Card_Spade and card:getNumber() >= 2 and card:getNumber() <= 9 and not self:hasSkills("hongyan|wuyan")
			end
			if self:isFriend(player) and self:willSkipDrawPhase(player) then
				return card:getSuit() == sgs.Card_Club and self:hasSuit("club", true, to)
			end
		end
	end
end

function sgs.ai_cardneed.leiji(to, card, self)
	return  ((isCard("Jink", card, to) and getKnownCard(to, "Jink", true) == 0)
			or (card:getSuit() == sgs.Card_Spade and not self:hasSuit("spade", true, to))
			or (card:isKindOf("EightDiagram") and not (self:isEquip("EightDiagram") or getKnownCard(to, "EightDiagram", false) >0)))
end

function SmartAI:findLeijiTarget(player, leiji_value)
	local getCmpValue = function(enemy)
		local value = 0
		if not self:damageIsEffective(enemy, sgs.DamageStruct_Thunder, player) then return 99 end
		if enemy:hasSkill("hongyan") then return 99 end
		if self:cantbeHurt(enemy, 2, player) or self:objectiveLevel(enemy) < 3 or (enemy:isChained() and not self:isGoodChainTarget(enemy, player)) then return 100 end
		if not sgs.isGoodTarget(enemy, self.enemies, self) then value = value + 50 end
		if enemy:hasArmorEffect("SilverLion") then value = value + 20 end
		if self:hasSkills(sgs.exclusive_skill, enemy) then value = value + 5 end
		if self:hasSkills(sgs.masochism_skill, enemy) then value = value + 3 end
		if self:hasSkills("tiandu|zhenlie", enemy) then value = value + 2 end
		if self:getDamagedEffects(enemy, player) or self:needToLoseHp(enemy, player) then value = value + 5 end
		if enemy:isChained() and self:isGoodChainTarget(enemy, player) and #(self:getChainedEnemies(player)) > 1 then value = value - 25 end
		if enemy:isLord() then value = value - 5 end
		value = value + enemy:getHp() * 2 + sgs.getDefenseSlash(enemy) * 0.01
		return value
	end

	local cmp = function(a, b)
		return getCmpValue(a) < getCmpValue(b)
	end

	local enemies = self:getEnemies(player)
	table.sort(enemies, cmp)
	for _,enemy in ipairs(enemies) do
		if getCmpValue(enemy) < leiji_value then return enemy end
	end
	return nil
end

sgs.ai_skill_playerchosen.leiji = function(self, targets)
	local mode = self.room:getMode()
	if mode:find("_mini_17") or mode:find("_mini_19") or mode:find("_mini_20") or mode:find("_mini_26") then
		local players = self.room:getAllPlayers();
		for _, aplayer in sgs.qlist(players) do
			if aplayer:getState() ~= "robot" then
				return aplayer
			end
		end
	end

	self:updatePlayers()
	return self:findLeijiTarget(self.player, 100)
end

function SmartAI:needLeiji(to, from)
	from = from or self.room:getCurrent()
	to = to or self.player
	if not to:hasSkill("leiji") then return false end
	if from and self:canLiegong(to, from) and not self:isFriend(to, from) then return false end
	if ( (to:hasSkill("guidao") and self:hasSuit("spade", true, to)) or (to:hasSkill("guicai") and self:hasSuit("spade", false, to))
		or (to:hasSkill("jilve") and self:hasSuit("spade", false, to) and to:getMark("@bear" ) > 0)
		or (getKnownCard(to, "Jink", true) > 1 and not self:isWeak(to)) )
		and (getKnownCard(to, "Jink", true) >= 1 or (not IgnoreArmor(from, to) and not self:isWeak(to) and self:isEquip("EightDiagram", to)))
		and self:findLeijiTarget(to, 50) and self:getFinalRetrial(to) == 1 then
			return true
	end
	return false
end

sgs.ai_card_intention.LeijiCard = 80

function sgs.ai_slash_prohibit.leiji(self, to, card, from)
	if self:isFriend(to) then return false end
	if to:hasFlag("qianxi_target") then return false end
	local hcard = to:getHandcardNum()
	if self:canLiegong(to, from) then return false end
	if self.role == "rebel" and to:isLord() then
		local other_rebel
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if sgs.evaluatePlayerRole(player) == "rebel" or sgs.compareRoleEvaluation(player, "rebel", "loyalist") == "rebel" then 
				other_rebel = player
				break
			end
		end		
		if not other_rebel and (self:hasSkills("hongyan") or self.player:getHp() >= 4) and (self:getCardsNum("Peach") > 0  or self:hasSkills("hongyan|ganglie|neoganglie")) then
			return false
		end
	end

	if getKnownCard(to, "Jink", true) >= 1 or (self:hasSuit("spade", true, to) and hcard >= 2) or hcard >= 4 then return true end
	if self:isEquip("EightDiagram", to) and not IgnoreArmor(from, to) then return true end
end

local huangtianv_skill = {}
huangtianv_skill.name = "huangtianv"
table.insert(sgs.ai_skills, huangtianv_skill)

huangtianv_skill.getTurnUseCard = function(self)
	if self.player:hasFlag("ForbidHuangtian") then return nil end
	if self.player:getKingdom() ~= "qun" then return nil end

	local cards = self.player:getCards("h")	
	cards = sgs.QList2Table(cards)	
	local card	
	self:sortByUseValue(cards,true)	
	for _,acard in ipairs(cards)  do
		if acard:isKindOf("Jink") then
			card = acard
			break
		end
	end	
	if not card then return nil end
	
	local card_id = card:getEffectiveId()
	local card_str = "@HuangtianCard="..card_id
	local skillcard = sgs.Card_Parse(card_str)
		
	assert(skillcard)
	return skillcard
end

sgs.ai_skill_use_func.HuangtianCard = function(card, use, self)
	if self:needBear() or self:getCardsNum("Jink", self.player, "h") <= 1 then
		return "."
	end
	local targets = {}
	for _,friend in ipairs(self.friends_noself) do
		if friend:hasLordSkill("huangtian") then
			if not friend:hasFlag("HuangtianInvoked") then
				if not friend:hasSkill("manjuan") then
					table.insert(targets, friend)
				end
			end
		end
	end
	if #targets > 0 then --黄天己方
		use.card = card
		self:sort(targets, "defense")
		if use.to then
			use.to:append(targets[1])
		end
	elseif self:getCardsNum("Slash", self.player, "he") >= 2 then --黄天对方
		for _,enemy in ipairs(self.enemies) do
			if enemy:hasLordSkill("huangtian") then
				if not enemy:hasFlag("HuangtianInvoked") then
					if not enemy:hasSkill("manjuan") then
						if enemy:isKongcheng() and not enemy:hasSkill("kongcheng") and not enemy:hasSkills("tuntian+zaoxian") then --必须保证对方空城，以保证天义/陷阵的拼点成功
							table.insert(targets, enemy)
						end
					end
				end
			end
		end
		if #targets > 0 then
			local flag = false
			if self.player:hasSkill("tianyi") and not self.player:hasUsed("TianyiCard") then
				flag = true
			elseif self.player:hasSkill("xianzhen") and not self.player:hasUsed("XianzhenCard") then
				flag = true
			end
			if flag then
				local maxCard = self:getMaxCard(self.player) --最大点数的手牌
				if maxCard:getNumber() > card:getNumber() then --可以保证拼点成功
					self:sort(targets, "defense", true) 
					for _,enemy in ipairs(targets) do
						if self.player:canSlash(enemy, nil, false, 0) then --可以发动天义或陷阵
								use.card = card
								enemy:setFlags("AI_HuangtianPindian")
								if use.to then
									use.to:append(enemy)
								end
								break
						end
					end
				end
			end
		end
	end
end

sgs.ai_card_intention.HuangtianCard = -80

sgs.ai_use_priority.HuangtianCard = 10
sgs.ai_use_value.HuangtianCard = 8.5

sgs.guidao_suit_value = {
	spade = 3.9,
	club = 2.7
}

sgs.ai_chaofeng.zhangjiao = 4

sgs.ai_skill_askforag.buqu = function(self, card_ids)
	for i, card_id in ipairs(card_ids) do
		for j, card_id2 in ipairs(card_ids) do
			if i ~= j and sgs.Sanguosha:getCard(card_id):getNumber() == sgs.Sanguosha:getCard(card_id2):getNumber() then
				return card_id
			end
		end
	end

	return card_ids[1]
end

function sgs.ai_skill_invoke.buqu(self, data)
	if #self.enemies == 1 and self.enemies[1]:hasSkill("guhuo") then
		return false
	else
		return true
	end
end

sgs.ai_chaofeng.zhoutai = -4

function sgs.ai_filterskill_filter.hongyan(card, card_place)
	if card:getSuit() == sgs.Card_Spade then
		return ("%s:hongyan[heart:%s]=%d"):format(card:objectName(), card:getNumberString(), card:getEffectiveId())
	end
end

sgs.ai_skill_use["@@tianxiang"] = function(self, data)
	local friend_lost_hp = 10
	local friend_hp = 0
	local card_id
	local target
	local cant_use_skill
	local dmg

	if data == "@tianxiang-card" then
		dmg = self.player:getTag("TianxiangDamage"):toDamage()
	else
		dmg = data
	end
	
	if not dmg then self.room:writeToConsole(debug.traceback()) return "." end
	
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
	for _,card in ipairs(cards) do
		if ((card:getSuit() == sgs.Card_Spade and self.player:hasSkill("hongyan")) or card:getSuit() == sgs.Card_Heart) and not card:isKindOf("Peach") then
			card_id = card:getId()
			break
		end
	end
	if not card_id then return "." end

	self:sort(self.enemies, "hp")

	for _, enemy in ipairs(self.enemies) do
		if (enemy:getHp() <= dmg.damage and enemy:isAlive()) then
			if ((enemy:getHandcardNum() <= 2) or self:hasSkills("guose|leiji|ganglie|enyuan|qingguo|wuyan|kongcheng", enemy)
				or enemy:containsTrick("indulgence")) and self:canAttack(enemy, (dmg.from or self.room:getCurrent()), dmg.nature) then
				return "@TianxiangCard="..card_id.."->"..enemy:objectName() 
			end
		end
	end

	for _, friend in ipairs(self.friends_noself) do
		if (friend:getLostHp() + dmg.damage > 1 and friend:isAlive()) then
			if friend:isChained() and #(self:getChainedFriends()) > 1 and dmg.nature ~= sgs.DamageStruct_Normal then
			elseif friend:getHp() >= 2 and dmg.damage < 2 and 
				(self:hasSkills("yiji|buqu|shuangxiong|zaiqi|yinghun|jianxiong|fangzhu", friend)
					or self:getDamagedEffects(friend, dmg.from or self.room:getCurrent())
					or self:needToLoseHp(friend, dmg.from or self.room:getCurrent(), nil, true)
					or (friend:getHandcardNum() < 3 and friend:hasSkill("rende")))
				then return "@TianxiangCard="..card_id.."->"..friend:objectName()
			elseif friend:hasSkill("buqu") then return "@TianxiangCard="..card_id.."->"..friend:objectName() end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if (enemy:getLostHp() <= 1 or dmg.damage > 1) and enemy:isAlive() then
			if ((enemy:getHandcardNum() <= 2)
				or enemy:containsTrick("indulgence") or self:hasSkills("guose|leiji|ganglie|enyuan|qingguo|wuyan|kongcheng", enemy))
				and self:canAttack(enemy, (dmg.from or self.room:getCurrent()), dmg.nature)
			then return "@TianxiangCard="..card_id.."->"..enemy:objectName() end
		end
	end

	for i = #self.enemies, 1, -1 do
		local enemy = self.enemies[i]
		if not enemy:isWounded() and not self:hasSkills(sgs.masochism_skill, enemy) and enemy:isAlive() and self:canAttack(enemy, (dmg.from or self.room:getCurrent()), dmg.nature) then
			return "@TianxiangCard="..card_id.."->"..enemy:objectName()
		end
	end

	return "."
end


sgs.ai_card_intention.TianxiangCard = function(self, card, from, tos)
	local to = tos[1]
	if self:getDamagedEffects(to) or self:needToLoseHp(to) then return end
	local intention = 10
	if (to:getHp() >= 2 and self:hasSkills("yiji|shuangxiong|zaiqi|yinghun|jianxiong|fangzhu", to))
		or (to:getHandcardNum() < 3 and (to:hasSkill("nosrende") or (to:hasSkill("rende") and not to:hasUsed("RendeCard"))))
		or to:hasSkill("buqu") then
		intention = -10
	end
	sgs.updateIntention(from, to, intention)
end

function sgs.ai_slash_prohibit.tianxiang(self, to, card, from)
	if from:hasSkill("jueqing") then return false end
	if from:hasSkill("nosqianxi") and from:distanceTo(to) == 1 then return false end
	if from:hasFlag("NosJiefanUsed") then return false end
	if self:isFriend(to, from) then return false end
	return self:cantbeHurt(to, 1, from)
end

sgs.tianxiang_suit_value = {
	heart = 4.9
}

function sgs.ai_cardneed.tianxiang(to, card)
	return (card:getSuit() == sgs.Card_Heart or (to:hasSkill("hongyan") and card:getSuit() == sgs.Card_Spade))
		and (getKnownCard(to, "heart", false) + getKnownCard(to, "spade", false)) < 2
end

table.insert(sgs.ai_global_flags, "questioner")

table.insert(sgs.ai_choicemade_filter.cardUsed, guhuo_filter)

sgs.ai_skill_choice.guhuo = function(self, choices)
	local yuji = self.room:findPlayerBySkillName("guhuo")
	local guhuoname = self.room:getTag("GuhuoType"):toString()
	if guhuoname == "peach+analeptic" then guhuoname = "peach" end
	if guhuoname == "normal_slash" then guhuoname = "slash" end
	local guhuocard = sgs.Sanguosha:cloneCard(guhuoname, sgs.Card_NoSuit, 0)
	local guhuotype = guhuocard:getClassName()
	if guhuotype and self:getRestCardsNum(guhuotype, yuji) == 0 and self.player:getHp() > 0 then return "question" end
	if guhuotype and (guhuotype == "AmazingGrace" or (guhuotype:match("Slash") and not self:isEquip("Crossbow",yuji))) then	return "noquestion" end
	if yuji:hasFlag("guhuo_failed") and math.random(1, 6) == 1 and self:isEnemy(yuji) and self.player:getHp() >= 3 and
		self.player:getHp() > self.player:getLostHp() then return "question" end
	local players = self.room:getOtherPlayers(self.player)
	players = sgs.QList2Table(players)
	local x = math.random(1, 5)

	self:sort(self.friends,"hp")

	if self.player:getHp() < 2 and self:getCardsNum("Peach") < 1 and self.room:alivePlayerCount() > 2 then return "noquestion" end
	if self:isFriend(yuji) then return "noquestion"
	elseif sgs.questioner then return "noquestion"
	else
		if self.player:getHp() < self.friends[#self.friends]:getHp() then return "noquestion" end
	end

	if self:needToLoseHp(self.player) and not self:hasSkills(sgs.masochism_skill, self.player) and x ~= 1 then return "question" end

	local questioner
	for _, friend in ipairs(self.friends) do
		if friend:getHp() == self.friends[#self.friends]:getHp() then
			if self:hasSkills("rende|kuanggu|zaiqi|buqu|yinghun|longhun|xueji|baobian") then
				questioner = friend
				break
			end
		end
	end
	if not questioner then questioner = self.friends[#self.friends] end
	return self.player:objectName() == questioner:objectName() and x ~= 1 and "question" or "noquestion"
end

sgs.ai_choicemade_filter.skillChoice.guhuo = function(player, promptlist)
	if promptlist[#promptlist] == "question" then
		sgs.questioner = player
	end
end

local guhuo_skill = {}
guhuo_skill.name = "guhuo"
table.insert(sgs.ai_skills, guhuo_skill)
guhuo_skill.getTurnUseCard = function(self)
	if self.player:isKongcheng() then return end
	local cards = sgs.QList2Table(self.player:getHandcards())
	local otherSuit_str, GuhuoCard_str = {}, {}
	for _,card in ipairs(cards) do
		if card:isNDTrick() then
			local dummyuse = { isDummy = true } 
			self:useTrickCard(card, dummyuse)
			if dummyuse.card then
				local cardstr = "@GuhuoCard=" .. card:getId() .. ":" .. card:objectName()
				if card:getSuit() == sgs.Card_Heart then
					table.insert(GuhuoCard_str, cardstr)
				else
					table.insert(otherSuit_str, cardstr)
				end
			end
		end
	end
	
	local other_suit, enemy_is_weak, zgl_kongcheng = true
	local can_fake_guhuo = sgs.turncount > 1
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() > 2 then
			other_suit = false
		end
		if enemy:getHp() > 1 then
			can_fake_guhuo = false
		end
		if self:isWeak(enemy) then
			enemy_is_weak = true
		end
		if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
			zgl_kongcheng = true
		end
	end
	
	if #otherSuit_str > 0 and other_suit then
		table.insertTable(GuhuoCard_str, otherSuit_str)
	end

	local peach_str = self:getGuhuoCard("Peach", self.player, true) 
	if peach_str then table.insert(GuhuoCard_str, peach_str) end
	
	local fakeCards = {}
	
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if (card:isKindOf("Slash") and self:getCardsNum("Slash", self.player, "h")>=2 and not self:isEquip("Crossbow"))
			or (card:isKindOf("Jink") and self:getCardsNum("Jink", self.player, "h")>=3)
			or (card:isKindOf("EquipCard") and self:getSameEquip(card))
			or card:isKindOf("Disaster") then
			table.insert(fakeCards, card)
		end
	end
	self:sortByUseValue(fakeCards, true)
	
	local function fake_guhuo(objectName, can_fake_guhuo)		
		if #fakeCards == 0 then return end
		
		local fakeCard
		local guhuo = "peach|ex_nihilo|snatch|dismantlement|amazing_grace|archery_attack|savage_assault|god_salvation|fire_attack"
		local guhuos = guhuo:split("|")
		for _, package in ipairs(sgs.Sanguosha:getBanPackages()) do
			if package == "maneuvering" then
				table.remove(guhuos, #guhuos)
				break
			end
		end
		for i=1, #guhuos do
			local forbiden = guhuos[i]
			forbid = sgs.Sanguosha:cloneCard(forbiden, sgs.Card_NoSuit, 0)
			if self.player:isLocked(forbid) then table.remove(forbiden, #guhuos) end
		end
		if can_fake_guhuo then
			for i=1, #guhuos do
				if guhuos[i] == "god_salvation" then table.remove(guhuos, i) break end
			end
		end
		for i=1, 10 do
			local card = fakeCards[math.random(1, #fakeCards)]
			local newguhuo = objectName or guhuos[math.random(1, #guhuos)]
			local guhuocard = sgs.Sanguosha:cloneCard(newguhuo, card:getSuit(), card:getNumber())
			if self:getRestCardsNum(guhuocard:getClassName()) > 0 then
				local dummyuse = {isDummy = true}
				if newguhuo == "peach" then self:useBasicCard(guhuocard, dummyuse) else self:useTrickCard(guhuocard, dummyuse) end
				if dummyuse.card then
					fakeCard = sgs.Card_Parse("@GuhuoCard=" .. card:getId() .. ":" .. newguhuo)
					break
				end
			end
		end
		return fakeCard
	end

	if #GuhuoCard_str > 0 then
	
		local guhuo_str = GuhuoCard_str[math.random(1, #GuhuoCard_str)]
		
		local str = guhuo_str:split("=")
		str = str[2]:split(":")
		local cardid, cardname = str[1], str[2]
		if sgs.Sanguosha:getCard(cardid):objectName() == cardname and cardname == "ex_nihilo" and math.random(1, 3) == 1 then
			local fake_exnihilo = fake_guhuo(cardname)
			if fake_exnihilo then return fake_exnihilo end
			
		elseif math.random(1, 5) == 1 then
			local fake_GuhuoCard = fake_guhuo()
			if fake_GuhuoCard then return fake_GuhuoCard end
		else
			return sgs.Card_Parse(guhuo_str)
		end
	
	elseif can_fake_guhuo and math.random(1, 4) ~= 1 then
		local fake_GuhuoCard = fake_guhuo(nil, can_fake_guhuo)
		if fake_GuhuoCard then return fake_GuhuoCard end
		
	elseif zgl_kongcheng and #fakeCards > 0 then
		return sgs.Card_Parse("@GuhuoCard="..fakeCards[1]:getEffectiveId()..":".."amazing_grace")
		
	else

		local lord = getLord(self.player)
		local drawcard = false
		if (lord and self:isFriend(lord) and lord:getHp() == 1 and self:isWeak(lord) and not isLord(self.player)) then
			drawcard = true
		elseif not enemy_is_weak then
			if sgs.current_mode_players["loyalist"] > sgs.current_mode_players["renegade"] + sgs.current_mode_players["rebel"] and
				self.role == "loyalist" and sgs.current_mode_players["rebel"] > 0 then
				drawcard = true
			elseif sgs.current_mode_players["rebel"] > sgs.current_mode_players["loyalist"] + sgs.current_mode_players["renegade"] + 2 and
				self.role == "rebel" then
				drawcard = true
			end
		end
		
		if drawcard and #fakeCards > 0 then
			local card_objectname
			local objectNames = {"ex_nihilo","snatch","dismantlement","amazing_grace","archery_attack","savage_assault","god_salvation","duel"}
			for _, objectName in ipairs(objectNames) do
				local acard = sgs.Sanguosha:cloneCard(objectName, sgs.Card_NoSuit, 0)
				if self:getRestCardsNum(acard:getClassName()) == 0 then
					card_objectname = objectName
					break
				end
			end
			if card_objectname then
				return sgs.Card_Parse("@GuhuoCard="..fakeCards[1]:getEffectiveId()..":"..card_objectname)
			end
		end
		
		local slash_str = self:getGuhuoCard("Slash", self.player, true)
		if slash_str and self:slashIsAvailable() then
			local card = sgs.Card_Parse(slash_str)
			local slash = sgs.Sanguosha:cloneCard("slash", card:getSuit(), card:getNumber())
			local dummy_use = { isDummy = true }
			self:useBasicCard(slash, dummy_use)
			if dummy_use.card then return card end
		end
	end
end

sgs.ai_skill_use_func.GuhuoCard=function(card,use,self)
	local userstring=card:toString()
	userstring=(userstring:split(":"))[3]
	local guhuocard=sgs.Sanguosha:cloneCard(userstring, card:getSuit(), card:getNumber())
	guhuocard:setSkillName("guhuo")
	if guhuocard:getTypeId() == sgs.Card_TypeBasic then self:useBasicCard(guhuocard, use) else assert(guhuocard) self:useTrickCard(guhuocard, use) end
	if not use.card then return end
	use.card=card
end

sgs.ai_use_priority.GuhuoCard = 10

function SmartAI:getGuhuoViewCard(class_name)
	local card_use = {}
	local ghly = (self.room:getMode() == "_mini_48")
	if ghly then
		card_use = sgs.QList2Table(self.player:getHandcards())
	else
		card_use = self:getCards(class_name, "h")
	end

	local classname2objectname = {
		["Slash"] = "slash", ["Jink"] = "jink",
		["Peach"] = "peach", ["Analeptic"] = "analeptic",
		["Nullification"] = "nullification",
	}

	if #card_use > 1 or (#card_use > 0 and (card_use[1]:getSuit() == sgs.Card_Heart or ghly)) then
		local index = 1
		if class_name == "Peach" or (class_name == "Analeptic" and not sgs.GetConfig("BanPackages", ""):match("maneuvering")) or class_name == "Jink" then
			index = #card_use
		end
		return "@GuhuoCard=" .. card_use[index]:getEffectiveId() .. ":" .. classname2objectname[class_name]
	end
end

function SmartAI:getGuhuoCard(class_name, at_play)
	local player = self.player
	if not player or not player:hasSkill("guhuo") then return end
	if at_play then
		if class_name == "Peach" and not player:isWounded() then return
		elseif class_name == "Analeptic" and player:hasUsed("Analeptic") then return
		elseif (class_name == "Slash" or class_name == "ThunderSlash" or class_name == "FireSlash") and not self:slashIsAvailable(player) then return
		elseif class_name == "Jink" or class_name == "Nullification" then return
		end
	else
		if class_name == "Peach" and self.player:hasFlag("Global_PreventPeach") then return end
	end
	return self:getGuhuoViewCard(class_name)
end

sgs.guhuo_suit_value = {
	heart = 5,
}

sgs.ai_skill_choice.guhuo_saveself = function(self, choices)
	if self:getCard("Peach") or not self:getCard("Analeptic") then return "peach" else return "analeptic" end
end

sgs.ai_suit_priority.guidao= "diamond|heart|club|spade"
sgs.ai_suit_priority.hongyan= "club|diamond|spade|heart"
sgs.ai_suit_priority.guhuo= "club|spade|diamond|heart"
sgs.ai_skill_choice.guhuo_slash = function(self, choices)
	return "slash"
end

function sgs.ai_cardneed.guhuo(to, card)
	return card:getSuit() == sgs.Card_Heart and (card:isKindOf("BasicCard") or card:isNDTrick())
end


function sgs.ai_cardneed.kuanggu(to, card)
	return card:isKindOf("OffensiveHorse") and not (to:getOffensiveHorse() or getKnownCard(to, "OffensiveHorse", false) > 0)
end