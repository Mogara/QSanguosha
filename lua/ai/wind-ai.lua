sgs.ai_skill_use["@@shensu1"]=function(self,prompt)
	self:updatePlayers(true)
	self:sort(self.enemies,"defense")
	if self.player:containsTrick("lightning") and self.player:getCards("j"):length()==1
		and self:hasWizard(self.friends) and not self:hasWizard(self.enemies,true) then return false end
	
	local selfSub = self.player:getHp()-self.player:getHandcardNum()
	local selfDef = sgs.getDefense(self.player)
	local hasJud = self.player:getJudgingArea()
	
	for _,enemy in ipairs(self.enemies) do
		local def=sgs.getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not
			((amr:inherits("Vine") and not self.player:hasWeapon("fan"))
			or (amr:objectName()=="eight_diagram"))
			
		if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
		elseif self:slashProhibit(nil, enemy) then
		elseif def<6 and eff then return "@ShensuCard=.->"..enemy:objectName()

		elseif selfSub>=2 then return "."
		elseif selfDef<6 then return "." end
		
	end
	
	for _,enemy in ipairs(self.enemies) do
		local def=sgs.getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not
			((amr:inherits("Vine") and not self.player:hasWeapon("fan"))
			or (amr:objectName()=="eight_diagram"))

		if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
		elseif self:slashProhibit(nil, enemy) then
		elseif eff and def<8 then return "@ShensuCard=.->"..enemy:objectName()
		else return "." end
	end
	return "."
end

sgs.ai_get_cardType=function(card)
	if card:inherits("Weapon") then return 1 end
	if card:inherits("Armor") then return 2 end
	if card:inherits("OffensiveHorse")then return 3 end
	if card:inherits("DefensiveHorse") then return 4 end
end

sgs.ai_skill_use["@@shensu2"]=function(self,prompt)
	self:updatePlayers(true)
	self:sort(self.enemies,"defense")
	
	local selfSub = self.player:getHp()-self.player:getHandcardNum()
	local selfDef = sgs.getDefense(self.player)
	
	local cards = self.player:getCards("he")
	
	cards=sgs.QList2Table(cards)
	
	local eCard
	local hasCard={0, 0, 0, 0}
	
	for _,card in ipairs(cards) do
		if card:inherits("EquipCard") then
			hasCard[sgs.ai_get_cardType(card)] = hasCard[sgs.ai_get_cardType(card)]+1
		end		
	end
	
	for _,card in ipairs(cards) do
		if card:inherits("EquipCard") then
			if hasCard[sgs.ai_get_cardType(card)]>1 or sgs.ai_get_cardType(card)>3 then
				eCard = card
				break
			end
			if not eCard and (not card:inherits("Armor") or card:inherits("GaleShell")) then eCard = card end
		end
	end
	
	if not eCard then return "." end
	
	local effectslash, best_target, target
	local defense = 6
	for _,enemy in ipairs(self.enemies) do
		local def=sgs.getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not
			((amr:inherits("Vine") and not self.player:hasWeapon("fan"))
			or (amr:objectName()=="eight_diagram") or enemy:hasSkill("bazhen"))

		if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
		elseif self:slashProhibit(nil, enemy) then
		elseif eff then
			if enemy:getHp() == 1 and self:getCardsNum("Jink", enemy) == 0 then best_target = enemy break end
			if def < defense then
				best_target = enemy
				defense = def
			end
			target = enemy
		end
		if selfSub<0 then return "." end
	end
	
	if best_target then return "@ShensuCard="..eCard:getEffectiveId().."->"..best_target:objectName() end
	if target then return "@ShensuCard="..eCard:getEffectiveId().."->"..target:objectName() end
	
	return "."
end

sgs.ai_card_intention.ShensuCard = 80

sgs.xiahouyuan_keep_value = 
{
	Peach = 6,
	Jink = 5.1,
	Crossbow = 5,
	Blade = 5,
	Spear = 5,
	DoubleSword =5,
	QinggangSword=5,
	Axe=5,
	KylinBow=5,
	Halberd=5,
	IceSword=5,
	Fan=5,
	MoonSpear=5,
	GudingBlade=5,
	DefensiveHorse = 5,
	OffensiveHorse = 5
}

sgs.ai_skill_invoke.jushou = true

sgs.ai_skill_invoke.liegong = sgs.ai_skill_invoke.tieji

sgs.ai_chaofeng.huangzhong = 1
sgs.ai_chaofeng.weiyan = -2

sgs.ai_skill_cardask["@guidao-card"]=function(self,prompt)
	local judge = self.player:getTag("Judge"):toJudge()
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
				return "@GuidaoCard=" .. cards[1]:getId()
			end
		end
	elseif self:needRetrial(judge) or self:getUseValue(judge.card) > self:getUseValue(sgs.Sanguosha:getCard(card_id)) then
		return "@GuidaoCard=" .. card_id
	end
	
	return "."
end

sgs.ai_skill_use["@@leiji"]=function(self,prompt)
	local mode = sgs.GetConfig("GameMode", "")
	if mode:find("mini") or mode:find("custom_scenario") then 
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
		if not self:isEquip("SilverLion", enemy) and not enemy:hasSkill("hongyan") and
			self:objectiveLevel(enemy) > 3 then
			return "@LeijiCard=.->"..enemy:objectName()
		end
	end
	return "."
end

sgs.ai_card_intention.LeijiCard = 80

function sgs.ai_slash_prohibit.leiji(self, to, card)
	if self:isFriend(to) then return false end
	local hcard = to:getHandcardNum()
	if self.player:hasSkill("tieji") or
		(self.player:hasSkill("liegong") and (hcard>=self.player:getHp() or hcard<=self.player:getAttackRange())) then return false end

	if to:getHandcardNum() >= 2 then return true end
	if self:isEquip("EightDiagram", to) then
		local equips = to:getEquips()
		for _, equip in sgs.qlist(equips) do
			if equip:getSuitString() == "spade" then return true end
		end
	end
end

local huangtianv_skill={}
huangtianv_skill.name="huangtianv"
table.insert(sgs.ai_skills,huangtianv_skill)

huangtianv_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("HuangtianCard") then return nil end
	if self.player:getKingdom() ~= "qun" then return nil end

	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)
	
	local card
	
	self:sortByUseValue(cards,true)
	
	for _,acard in ipairs(cards)  do
		if acard:inherits("Jink") then
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
		if friend:hasLordSkill("huangtian") then
			table.insert(targets, friend)
		end
	end
	
	if #targets == 0 then return end

	use.card=card
	self:sort(targets, "defense")
	if use.to then
		use.to:append(targets[1])
	end
end

sgs.ai_card_intention.HuangtianCard = function(card,from,tos,source)
	for _, to in ipairs(tos) do
		sgs.updateIntention(from, to, -80)
		if to:isLord() then sgs.ai_lord_tolerance[from:objectName()]=(sgs.ai_lord_tolerance[from:objectName()] or 0)+1 end
	end
end

sgs.ai_use_priority.HuangtianCard = 10
sgs.ai_use_value.HuangtianCard = 8.5

sgs.zhangjiao_suit_value = 
{
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

sgs.ai_chaofeng.zhoutai = -4

sgs.ai_skill_use["@tianxiang"]=function(self, data)
	local friend_lost_hp = 10
	local friend_hp = 0
	local card_id
	local target
	local cant_use_skill
	local dmg

	if data=="@@tianxiang-card" then
		dmg = self.player:getTag("TianxiangDamage"):toDamage()
	else
		dmg = data
	end

	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
	for _,card in ipairs(cards) do
		if (card:getSuit() == sgs.Card_Spade or card:getSuit() == sgs.Card_Heart) and not card:inherits("Peach") then
			card_id = card:getId()
			break
		end
	end
	if not card_id then return "." end

	self:sort(self.enemies, "hp")

	for _, enemy in ipairs(self.enemies) do
		if (enemy:getHp() <= dmg.damage) then

		if (enemy:getHandcardNum() <= 2) or self:hasSkills("guose|leiji|ganglie|enyuan|qingguo|wuyan|kongcheng", enemy)
			or enemy:containsTrick("indulgence") then return "@TianxiangCard="..card_id.."->"..enemy:objectName() end
		end
	end

	for _, friend in ipairs(self.friends_noself) do
		if (friend:getLostHp() + dmg.damage>1) then
			if friend:isChained() and #self:getChainedFriends()>1 and dmg.nature>0 then
			elseif friend:getHp() >= 2 and dmg.damage<2 and 
				(self:hasSkills("yiji|buqu|shuangxiong|zaiqi|yinghun|jianxiong|fangzhu", friend) 
				or (friend:getHandcardNum()<3 and friend:hasSkill("rende"))
				)
				then return "@TianxiangCard="..card_id.."->"..friend:objectName()
			elseif friend:hasSkill("buqu") then return "@TianxiangCard="..card_id.."->"..friend:objectName() end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if (enemy:getLostHp() <= 1) or dmg.damage>1 then

		if (enemy:getHandcardNum() <= 2)
			or enemy:containsTrick("indulgence") or self:hasSkills("guose|leiji|ganglie|enyuan|qingguo|wuyan|kongcheng", enemy)
			then return "@TianxiangCard="..card_id.."->"..enemy:objectName() end
		end
	end

	for i = #self.enemies, 1, -1 do
		local enemy = self.enemies[i]
		if not enemy:isWounded() and not self:hasSkills(sgs.masochism_skill, enemy) then
			return "@TianxiangCard="..card_id.."->"..enemy:objectName()
		end
	end

	return "."
end

sgs.xiaoqiao_suit_value = 
{
	spade = 6,
	heart = 6
}

table.insert(sgs.ai_global_flags, "questioner")

local guhuo_filter = function(player, carduse)
	if carduse.card:inherits("GuhuoCard") then
		sgs.questioner = nil
		sgs.guhuotype = carduse.card:toString():split(":")[2]
	end
end

table.insert(sgs.ai_choicemade_filter.cardUsed, guhuo_filter)

sgs.ai_skill_choice.guhuo = function(self, choices)
	if sgs.guhuotype and (sgs.guhuotype == "shit" or sgs.guhuotype == "amazing_grace") then return "noquestion" end
	local players = self.room:getOtherPlayers(self.player)
	players = sgs.QList2Table(players)
	local yuji
	if self.player:getHp()<2 then return "noquestion" end
	for _, other in ipairs(players) do
		if other:hasSkill("guhuo") then yuji = other break end
	end
	if self.lua_ai:isFriend(yuji) then return "noquestion"
	elseif sgs.questioner then return "noquestion"
	else
		self:sort(self.friends,"hp")
		if self.player:getHp()<self.friends[#self.friends]:getHp() then return "noquestion" end
	end
	local r=math.random(0,self.player:getHp()-1)
	if r==0 then return "noquestion" else return "question" end
end

sgs.ai_choicemade_filter.skillChoice.guhuo = function(self, promptlist)
	if promptlist[#promptlist] == "yes" then
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
			local dummyuse={}
			dummyuse.isDummy=true
			self:useTrickCard(card, dummyuse)
			if dummyuse.card then
				local parsed_card=sgs.Card_Parse("@GuhuoCard=" .. card:getId() .. ":" .. card:objectName())
				return parsed_card
			end
		end
	end

	local card_str = self:getGuhuoCard("Peach", self.player, true) or self:getGuhuoCard("Analeptic", self.player, true) or self:getGuhuoCard("Slash", self.player, true)
	if card_str then return sgs.Card_Parse(card_str) end

	local guhuo = "peach|ex_nihilo|snatch|amazing_grace|archery_attack|fire_attack"
	local guhuos = guhuo:split("|")
	for _, package in ipairs(sgs.Sanguosha:getBanPackages()) do
		if package == "maneuvering" then
			table.remove(guhuos, #guhuos)
			break
		end
	end

	for _,card in ipairs(cards) do
		if (card:inherits("Slash") and self:getCardsNum("Slash", self.player, "h")>=2 and not self:isEquip("Crossbow"))
		or (card:inherits("Jink") and self:getCardsNum("Jink", self.player, "h")>=3) then
			for i=1, 10 do
				local newguhuo = guhuos[math.random(1,#guhuos)]
				local guhuocard = sgs.Sanguosha:cloneCard(newguhuo, card:getSuit(), card:getNumber())
				local dummyuse = {isDummy = true}
				if newguhuo == "peach" then self:useBasicCard(guhuocard,dummyuse,false) else self:useTrickCard(guhuocard,dummyuse) end
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
	userstring=(userstring:split(":"))[2]
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

sgs.yuji_suit_value =
{
	heart = 5
}
