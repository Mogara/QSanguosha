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
	if card:isKindOf("OffensiveHorse")then return 3 end
	if card:isKindOf("DefensiveHorse") then return 4 end
end

sgs.ai_skill_use["@@shensu2"]=function(self,prompt)
	self:updatePlayers()
	self:sort(self.enemies,"defenseSlash")
	
	local selfSub = self.player:getHp() - self.player:getHandcardNum()
	local selfDef = sgs.getDefense(self.player)
	
	local cards = self.player:getCards("he")
	
	cards = sgs.QList2Table(cards)
	
	local eCard
	local hasCard = {0, 0, 0, 0}
	
	for _,card in ipairs(cards) do
		if card:isKindOf("EquipCard") then
			hasCard[sgs.ai_get_cardType(card)] = hasCard[sgs.ai_get_cardType(card)]+1
		end		
	end
	
	for _,card in ipairs(cards) do
		if card:isKindOf("EquipCard") then
			if hasCard[sgs.ai_get_cardType(card)] > 1 or sgs.ai_get_cardType(card) > 3 then
				eCard = card
				break
			end
			if not eCard and not card:isKindOf("Armor") then eCard = card end
		end
	end
	
	if (self.player:hasArmorEffect("SilverLion") and self.player:isWounded())
	  or (self:hasSkills("bazhen|yizhong") and self.player:getArmor()) then
		eCard = self.player:getArmor()
	end

	if not eCard then return "." end
	
	local effectslash, best_target, target
	local defense = 6
	for _,enemy in ipairs(self.enemies) do
		local def=sgs.getDefense(enemy)
		local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
		local eff = self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self)

		if not self.player:canSlash(enemy, slash, false) then
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

sgs.ai_chaofeng.huangzhong = 1
sgs.ai_chaofeng.weiyan = -2

sgs.ai_skill_cardask["@guidao-card"]=function(self, data)
	local judge = data:toJudge()
	local all_cards = self.player:getCards("he")
	if all_cards:isEmpty() then return "." end
	local cards = {}
	for _, card in sgs.qlist(all_cards) do
		if card:isBlack() then
			table.insert(cards, card)
		end
	end

	if #cards == 0 then return "." end
	local card_id = self:getRetrialCardId(cards, judge)
	if card_id == -1 then
		if self:needRetrial(judge) then
			self:sortByUseValue(cards, true)
			if self:getUseValue(judge.card) > self:getUseValue(cards[1]) then
				return "@GuidaoCard[" .. cards[1]:getSuitString() .. ":" .. cards[1]:getNumberString() .."]=" .. cards[1]:getId()
			end
		end
	elseif self:needRetrial(judge) or self:getUseValue(judge.card) > self:getUseValue(sgs.Sanguosha:getCard(card_id)) then
		local card = sgs.Sanguosha:getCard(card_id)
		return "@GuidaoCard[" .. card:getSuitString() .. ":" .. card:getNumberString() .. "]=" .. card_id
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
			or (card:getSuit() == sgs.Card_Spade and self:hasSuit("spade", true, to))
			or (card:isKindOf("EightDiagram") and not (self:isEquip("EightDiagram") or getKnownCard(to, "EightDiagram", false) >0)))
end

sgs.ai_skill_use["@@leiji"]=function(self,prompt)
	local mode = self.room:getMode()
	if mode:find("_mini_17") or mode:find("_mini_19") or mode:find("_mini_20") or mode:find("_mini_26") then 
		local players = self.room:getAllPlayers();
		for _,aplayer in sgs.qlist(players) do
			if aplayer:getState() ~= "robot" then
				return "@LeijiCard=.->"..aplayer:objectName()
			end
		end
	end

	self:updatePlayers()
	self:sort(self.enemies,"hp")
	for _,enemy in ipairs(self.enemies) do
		if not enemy:hasArmorEffect("SilverLion") and not enemy:hasSkill("hongyan") and
			self:objectiveLevel(enemy) > 3 and not self:cantbeHurt(enemy) and not (enemy:isChained() and not self:isGoodChainTarget(enemy)) then
			return "@LeijiCard=.->"..enemy:objectName()
		end
	end
	
	for _,enemy in ipairs(self.enemies) do
		if not enemy:hasSkill("hongyan")
		 and not (enemy:isChained() and not self:isGoodChainTarget(enemy)) then
			return "@LeijiCard=.->"..enemy:objectName()
		end
	end
	
	return "."
end

sgs.ai_card_intention.LeijiCard = 80

function sgs.ai_slash_prohibit.leiji(self, to, card)
	if self:isFriend(to) then return false end
	local hcard = to:getHandcardNum()
	if self.player:hasSkill("liegong") and (hcard >= self.player:getHp() or hcard <= self.player:getAttackRange()) then return false end
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

	if getKnownCard(to,"Jink",true) >= 1 or (self:hasSuit("spade", true, to) and hcard >= 2) or hcard >= 4 then return true end
	if self:isEquip("EightDiagram", to) and not IgnoreArmor(self.player, to) then return true end
end


local huangtianv_skill={}
huangtianv_skill.name="huangtianv"
table.insert(sgs.ai_skills,huangtianv_skill)

huangtianv_skill.getTurnUseCard=function(self)
	if self.player:hasFlag("ForbidHuangtian") then return nil end
	if self.player:getKingdom() ~= "qun" then return nil end

	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)
	
	local card
	
	self:sortByUseValue(cards,true)
	
	for _,acard in ipairs(cards)  do
		if acard:isKindOf("Jink") then
			card = acard
			break
		end
	end
	
	if not card then
		return nil
	end
	
	local card_id = card:getEffectiveId()
	local card_str = "@HuangtianCard="..card_id
	local skillcard = sgs.Card_Parse(card_str)
		
	assert(skillcard)
	return skillcard
end

sgs.ai_skill_use_func.HuangtianCard=function(card,use,self)
	local targets = {}
	for _, friend in ipairs(self.friends_noself) do
		if friend:hasLordSkill("huangtian") and not friend:hasFlag("HuangtianInvoked") and not friend:hasSkill("manjuan") then
			table.insert(targets, friend)
		end
	end
	
	if #targets == 0 then return end
	if self:needBear() or self:getCardsNum("Jink",self.player,"h")<=1  then return "." end
	use.card=card
	self:sort(targets, "defense")
	if use.to then
		use.to:append(targets[1])
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
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
	for _,card in ipairs(cards) do
		if ((card:getSuit() == sgs.Card_Spade and self:hasSkill("hongyan")) or card:getSuit() == sgs.Card_Heart) and not card:isKindOf("Peach") then
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
					or friend:getHp() > getBestHp(friend)
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


sgs.ai_card_intention.TianxiangCard = function(card, from, tos)
	local to = tos[1]
	local intention = 10
	local friend = false
	for _, askill in ipairs(("yiji|shuangxiong|zaiqi|yinghun|jianxiong|fangzhu"):split("|")) do
		if to:hasSkill(askill) then
			friend = true
			break
		end
	end
	if (to:getHp() >= 2 and friend)
		or (to:getHandcardNum() < 3 and to:hasSkill("rende"))
		or to:hasSkill("buqu") then
		intention = -10
	end
	sgs.updateIntention(from, to, intention)
end


function sgs.ai_slash_prohibit.tianxiang(self, to)
	if self.player:hasSkill("jueqing") then return false end
	if self.player:hasSkill("qianxi") and self.player:distanceTo(self.player) == 1 then return false end
	if self.player:hasFlag("nosjiefanUsed") then return false end
	if self:isFriend(to) then return false end
	return self:cantbeHurt(to)
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
	local guhuocard = sgs.Sanguosha:cloneCard(guhuoname, sgs.Card_NoSuit, 0)
	local guhuotype = guhuocard:getClassName()
	--it seems that getBanPackages() cannot return a proper value
	--if guhuotype and self:getRestCardsNum(guhuotype) == 0 and self.player:getHp() > 0 then return "question" end
	if guhuotype and (guhuotype == "AmazingGrace" or (guhuotype:match("Slash") and not self:isEquip("Crossbow",yuji))) then return "noquestion" end
	local players = self.room:getOtherPlayers(self.player)
	players = sgs.QList2Table(players)
	local yuji

	self:sort(self.friends,"hp")

	if self.player:getHp()<2 and self.room:alivePlayerCount() > 2 then return "noquestion" end
	for _, other in ipairs(players) do
		if other:hasSkill("guhuo") then yuji = other break end
	end
	if self.lua_ai:isFriend(yuji) then return "noquestion"
	elseif sgs.questioner then return "noquestion"
	else		
		if self.player:getHp()<self.friends[#self.friends]:getHp() then return "noquestion" end
	end

	if self.player:getHp() > getBestHp(self.player) and not self:hasSkills(sgs.masochism_skill,self.player) then return "question" end

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
	return self.player:objectName() == questioner:objectName() and "question" or "noquestion"
end

sgs.ai_choicemade_filter.skillChoice.guhuo = function(player, promptlist)
	if promptlist[#promptlist] == "question" then
		sgs.questioner = player
	end
end

local guhuo_skill={}
guhuo_skill.name="guhuo"
table.insert(sgs.ai_skills,guhuo_skill)
guhuo_skill.getTurnUseCard=function(self)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards)

	for _,card in ipairs(cards) do
		if card:isNDTrick() and card:getSuit() == sgs.Card_Heart then
			local dummyuse = { isDummy = true } 
			self:useTrickCard(card, dummyuse)
			if dummyuse.card then
				local parsed_card=sgs.Card_Parse("@GuhuoCard=" .. card:getId() .. ":" .. card:objectName())
				return parsed_card
			end
		end
	end

	local card_str = self:getGuhuoCard("Peach", self.player, true) 
	if card_str then return sgs.Card_Parse(card_str) end

	local slash_str = self:getGuhuoCard("Slash", self.player, true) or self:getGuhuoCard("Analeptic", self.player, true)
	if slash_str and self:slashIsAvailable() then return sgs.Card_Parse(slash_str) end

	local guhuo = "peach|ex_nihilo|snatch|amazing_grace|archery_attack|fire_attack"
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

	self:sortByUseValue(cards, true)
	for _,card in ipairs(cards) do
		if (card:isKindOf("Slash") and self:getCardsNum("Slash", self.player, "h")>=2 and not self:isEquip("Crossbow"))
		or (card:isKindOf("Jink") and self:getCardsNum("Jink", self.player, "h")>=3)
		or (card:isKindOf("Weapon") and self.player:getWeapon())
		or card:isKindOf("Disaster") then
			for i=1, 10 do
				local newguhuo = guhuos[math.random(1,#guhuos)]
				local guhuocard = sgs.Sanguosha:cloneCard(newguhuo, card:getSuit(), card:getNumber())
				if self:getRestCardsNum(guhuocard:getClassName()) == 0 then return end
				local dummyuse = {isDummy = true}
				if newguhuo == "peach" then self:useBasicCard(guhuocard, dummyuse) else self:useTrickCard(guhuocard, dummyuse) end
				if dummyuse.card then
					local parsed_card=sgs.Card_Parse("@GuhuoCard=" .. card:getId() .. ":" .. newguhuo)
					return parsed_card
				end
			end
		end
	end
end

sgs.ai_skill_use_func.GuhuoCard=function(card,use,self)
	local userstring=card:toString()
	userstring=(userstring:split(":"))[3]
	local guhuocard=sgs.Sanguosha:cloneCard(userstring, card:getSuit(), card:getNumber())
	if guhuocard:getTypeId() == sgs.Card_Basic then self:useBasicCard(guhuocard,use,false) else assert(guhuocard) self:useTrickCard(guhuocard,use) end
	if not use.card then return end
	use.card=card
end

sgs.ai_use_priority.GuhuoCard = 10

local function getGuhuoViewCard(self, class_name, player)
	local card_use = {}
	card_use = self:getCards(class_name, player)

	if #card_use > 1 or (#card_use > 0 and card_use[1]:getSuit() == sgs.Card_Heart) then
		local index = 1
		if class_name == "Peach" or class_name == "Analeptic" or class_name == "Jink" then
			index = #card_use
		end
		return "@GuhuoCard=" .. card_use[index]:getEffectiveId() ..":".. card_use[index]:objectName()
	end
end

function SmartAI:getGuhuoCard(class_name, player, at_play)
	player = player or self.player
	if not player or not player:hasSkill("guhuo") then return end
	if at_play then
		if class_name == "Peach" and not player:isWounded() then return
		elseif class_name == "Analeptic" and player:hasUsed("Analeptic") then return
		elseif class_name == "Slash" and not self:slashIsAvailable(player) then return
		elseif class_name == "Jink" or class_name == "Nullification" then return
		end
	end
	return getGuhuoViewCard(self, class_name, player)
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