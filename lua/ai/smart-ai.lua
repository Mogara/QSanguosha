 -- This is the Smart AI, and it should be loaded and run at the server side

-- "middleclass" is the Lua OOP library written by kikito
-- more information see: https://github.com/kikito/middleclass
require "middleclass"

-- initialize the random seed for later use
math.randomseed(os.time())

-- this table stores all specialized AI classes
sgs.ai_classes = {}

-- compare functions
sgs.ai_compare_funcs = {
	hp = function(a, b)
		return a:getHp() < b:getHp()
	end,

	handcard = function(a, b)
		return a:getHandcardNum() < b:getHandcardNum()
	end,

	value = function(a, b)
		local value1 = a:getHp() * 2 + a:getHandcardNum()
		local value2 = b:getHp() * 2 + b:getHandcardNum()

		return value1 < value2
	end,

	chaofeng = function(a, b)
		local c1 = sgs.ai_chaofeng[a:getGeneralName()]	or 0
		local c2 = sgs.ai_chaofeng[b:getGeneralName()] or 0

		if c1 == c2 then
			return sgs.ai_compare_funcs.value(a, b)
		else
			return c1 > c2
		end
	end,

	defense = function(a,b)
		local d1=a:getHp() * 2 + a:getHandcardNum()
		if(d1>a:getHp()*3) then d1=a:getHp()*3 end
		if a:getArmor() then d1=d1+2 end
		local d2=b:getHp() * 2 + b:getHandcardNum()
		if(d2>b:getHp()*3) then d2=b:getHp()*3 end
		if b:getArmor() then d2=d2+2 end
		
		local c1 = sgs.ai_chaofeng[a:getGeneralName()]	or 0
		local c2 = sgs.ai_chaofeng[b:getGeneralName()] or 0
		
		return d1<d2
	end,

	threat = function ( a, b)
		local players=sgs.QList2Table(a:getRoom():getOtherPlayers(a))
		local d1=a:getHandcardNum()
		for _, player in ipairs(players) do
			if a:canSlash(player,true) then
				d1=d1+10/(getDefense(player))
			end
		end
		players=sgs.QList2Table(b:getRoom():getOtherPlayers(b))
		local d2=b:getHandcardNum()
		for _, player in ipairs(players) do
			if b:canSlash(player,true) then
				d2=d2+10/(getDefense(player))
			end
		end
		local c1 = sgs.ai_chaofeng[a:getGeneralName()]	or 0
		local c2 = sgs.ai_chaofeng[b:getGeneralName()] or 0
		 return d1+c1/2>d2+c2/2
	end,

		
}



-- this table stores the chaofeng value for some generals
-- all other generals' chaofeng value should be 0
sgs.ai_chaofeng = {
	huatuo = 5,

	sunshangxiang = 4,
	huangyueying = 4,
	diaochan = 4,
	zhangjiao = 4,
	lusu = 4,

	zhangfei = 3,
	taishici = 3,
	xuchu = 3,

	zhangliao = 2,
	xuhuang = 2,
	ganning = 2,

	lubu = 1,
	huangzhong = 1,
	machao = 1,

	simayi = -1,
	caopi = -2,
	xiahoudun = -2,
	xunyu = -2,
	guojia = -3,

	shencaocao = -4,
	shenguanyu = -4,
}

-- this function is only function that exposed to the host program
-- and it clones an AI instance by general name
function CloneAI(player)
	local ai_class = sgs.ai_classes[player:getGeneralName()]
	if ai_class then
		return ai_class(player).lua_ai
	else
		return SmartAI(player).lua_ai
	end
end

sgs.ai_round={}
sgs.ai_knownSlash={}
sgs.ai_knownJink={}
sgs.ai_knownPeach={}
sgs.ai_character_parameters={}

function getCount(name)
	if sgs.ai_round[name] then 
                sgs.ai_round[name]=sgs.ai_round[name]+1
	else 
		sgs.ai_round[name]=1 
	end
        return sgs.ai_round[name]
end

--defense is defined as hp*2 + hand + (2)(if armor present)
function getDefense(player)
	local d=player:getHp() * 2 + player:getHandcardNum()
	if(d>player:getHp()*3) then d=player:getHp()*3 end
	if player:getArmor() then d=d+2 end
	return d
end

-- SmartAI is the base class for all other specialized AI classes
SmartAI = class "SmartAI"


-- the "initialize" function is just the "constructor"
function SmartAI:initialize(player)

	self.player = player
	self.room = player:getRoom()
	
	
	self.room.astring="yes"
	
	
	self.lua_ai = sgs.LuaAI(player)
	self.lua_ai.callback = function(method_name, ...)
		local method = self[method_name]
		if method then
			return method(self, ...)
		end
	end

	self:updatePlayers()
end

-- this function create 2 tables contains the friends and enemies, respectively
function SmartAI:updatePlayers()
	self.friends = sgs.QList2Table(self.lua_ai:getFriends())
	table.insert(self.friends, self.player)

	self.friends_noself = sgs.QList2Table(self.lua_ai:getFriends())

	self.enemies = sgs.QList2Table(self.lua_ai:getEnemies())

	self.has_wizard = self:hasWizard(self.friends) and not self:hasWizard(self.enemies)
end

function SmartAI:hasWizard(players)
	for _, player in ipairs(players) do
		if player:hasSkill("guicai") or player:hasSkill("guidao") then
			return true
		end
	end
end

function SmartAI:sort(players, key)
	key = key or "chaofeng" -- the default compare key is "chaofeng"

	local func= sgs.ai_compare_funcs[key]

	assert(func)

	table.sort(players, func)
end

function SmartAI:filterEvent(event, player, data)
	if event == sgs.Death then
		self:updatePlayers()
	end
end

function SmartAI:isFriend(other)
	return self.lua_ai:isFriend(other)
end

function SmartAI:isEnemy(other)
	return self.lua_ai:isEnemy(other)
end

function SmartAI:isNeutrality(other)
	return self.lua_ai:relationTo(other) == sgs.AI_Neutrality
end

-- get the card with the maximal card point
function SmartAI:getMaxCard(player)
	player = player or self.player

	if player:isKongcheng() then
		return nil
	end

	local cards = player:getHandcards()
	local max_card, max_point = nil, 0
	for _, card in sgs.qlist(cards) do
		local point = card:getNumber()
		if point > max_point then
			max_point = point
			max_card = card
		end
	end

	return max_card
end

-- the table that stores whether the skill should be invoked
-- used for SmartAI:askForSkillInvoke
sgs.ai_skill_invoke = {
	eight_diagram = true,
	double_sword = true,
	fan = true,
	
	kylin_bow = function(self, data)	
		local effect = data:toSlashEffect()
		
		if effect.to:hasSkill("xiaoji") then
			return false
		end
		
		return self:isEnemy(effect.to)
	end,
}

function SmartAI:askForSkillInvoke(skill_name, data)
	local invoke = sgs.ai_skill_invoke[skill_name]
	if type(invoke) == "boolean" then
		return invoke
	elseif type(invoke) == "function" then
		return invoke(self, data)
	else
		local skill = sgs.Sanguosha:getSkill(skill_name)
		return skill and skill:getFrequency() == sgs.Skill_Frequent
	end
end

function SmartAI:askForYiji(cards)

	return nil, 0
end

-- used for SmartAI:askForUseCard
sgs.ai_skill_use = {}

function SmartAI:askForUseCard(pattern, prompt)
	local use_func = sgs.ai_skill_use[pattern]
	if string.find(pattern,"@@liuli") then use_func = sgs.ai_skill_use["@@liuli"] end
	if use_func then
		return use_func(self, prompt) or "."
	else
		return "."
	end
end

function SmartAI:slashIsEffective(slash, to)
	if to:hasSkill("yizhong") and to:getArmor() and slash:isBlack() then
		return false
	end

	if self.player:hasWeapon("qinggang_sword") then
		return true
	end

	local armor = to:getArmor()
	if armor then
		if armor:objectName() == "renwang_shield" then
			return not slash:isBlack()
		elseif armor:inherits("Vine") then
			return slash:inherits("NatureSlash") or self.player:hasWeapon("fan")
		end
	end

	return true
end

function SmartAI:slashHit(slash, to)
	return self:getJinkNumber(to) == 0
end

function SmartAI:slashIsAvailable()
	if self.player:hasWeapon("crossbow") or self.player:hasSkill("paoxiao") then
		return true
	end

	if self.player:hasFlag("tianyi_success") then
		return self.player:getMark("SlashCount") < 2
	else
		return self.player:getMark("SlashCount") < 1
	end
end


	

function SmartAI:useBasicCard(card, use)
	if card:inherits("Slash") and self:slashIsAvailable() then
		self:sort(self.enemies, "defense")
		for _, enemy in ipairs(self.enemies) do
			if self.player:canSlash(enemy, true) and
				self:slashIsEffective(card, enemy) then
				if (getDefense(self.player)<getDefense(enemy) and self.player:getHandcardNum()<(1+self.player:getHp())) then return end
				-- fill the card use struct
				use.card = card
				use.to:append(enemy)

				return
			end
		end
	elseif card:inherits("Peach") and self.player:isWounded() then
		self:sort(self.friends, "defense")
		if getDefense(self.player)>6 and getDefense(self.friends[1])<4 then return end
		use.card = card
	elseif card:inherits("Analeptic") and self:slashIsAvailable() and self:getSlashNumber(self.player)>0 and not self.player:hasFlag("drank")then
		self:sort(self.enemies, "defense")		
		for _, enemy in ipairs(self.enemies) do 
			if self.player:canSlash(enemy, true) and
				self:slashIsEffective(card, enemy) then
				if (getDefense(self.player)-1)<getDefense(enemy) and self.player:getHandcardNum()<2+self.player:getHp()  then return end
					if enemy:getArmor() then if enemy:getArmor():objectName()=="eight_diagram" or enemy:getArmor():objectName()=="silver_lion" then return end end
					use.card = card
					return 
			end
		end
	end
end

function SmartAI:aoeIsEffective(card, to)
	-- the AOE starter is not effected by AOE
	if self.player == to then
		return false
	end

	-- the vine
	local armor = to:getArmor()
	if armor and armor:inherits("Vine") then
		return false
	end

	-- Jiaxu's weimu
	if self.room:isProhibited(self.player, to, card) then
		return false
	end

	-- Yangxiu's Danlao
	if to:hasSkill("danlao") then
		return false
	end

	-- Menghuo and Zhurong
	if card:inherits("SavageAssault") then
		if to:hasSkill("huoshou") or to:hasSkill("juxiang") then
			return false
		end
	end

	return true
end

function SmartAI:getDistanceLimit(card)
	if self.player:hasSkill "qicai" then
		return nil
	end

	if card:inherits "Snatch" then
		return 1
	elseif card:inherits "SupplyShortage" then
		if self.player:hasSkill "duanliang" then
			return 2
		else
			return 1
		end
	end
end

function SmartAI:exclude(players, card)
	local excluded = {}
	local limit = self:getDistanceLimit(card)
	for _, player in sgs.list(players) do
		if not self.room:isProhibited(self.player, player, card) then
			local should_insert = true
			if limit then
				should_insert = self.player:distanceTo(player) <= limit
			end

			if should_insert then
				table.insert(excluded, player)
			end
		end
	end

	return excluded
end

function SmartAI:useCardDismantlement(dismantlement, use)
	if (not self.has_wizard) and self:hasWizard(self.enemies) then
		-- find lightning
		local players = self.room:getOtherPlayers(self.player)
		players = self:exclude(players, dismantlement)
		for _, player in ipairs(players) do
			if player:containsTrick("lightning") then
				use.card = dismantlement
				use.to:append(player)
				return			
			end
		end
	end

	self:sort(self.friends_noself,"defense")
	local friends = self:exclude(self.friends_noself, dismantlement)
	for _, friend in ipairs(friends) do
		if friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage") then
			use.card = dismantlement
			use.to:append(friend)

			return
		end			
	end		
	
	self:sort(self.enemies,"threat")
	local enemies = self:exclude(self.enemies, dismantlement)
	for _, enemy in ipairs(enemies) do
		local equips = enemy:getEquips()
		if  not enemy:isNude() then
				use.card = dismantlement
				use.to:append(enemy)
				return 
		end
	end
end

-- very similar with SmartAI:useCardDismantlement
function SmartAI:useCardSnatch(snatch, use)
	if not self.has_wizard and self:hasWizard(self.enemies)  then
		-- find lightning
		local players = self.room:getOtherPlayers(self.player)
		players = self:exclude(players, snatch)
		for _, player in ipairs(players) do
			if player:containsTrick("lightning") then
				use.card = snatch
				use.to:append(player)
				
				return			
			end			
		end
	end

	self:sort(self.friends_noself,"defense")
	local friends = self:exclude(self.friends_noself, snatch)
	for _, friend in ipairs(friends) do
		if friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage") then
			use.card = snatch
			use.to:append(friend)

			return
		end			
	end		
	
	self:sort(self.enemies,"threat")
	local enemies = self:exclude(self.enemies, snatch)
	for _, enemy in ipairs(enemies) do
		if  not enemy:isNude() then
			
				use.card = snatch
				use.to:append(enemy)
	
				return
			
		end
	end
end

function SmartAI:useCardFireAttack(fire_attack, use)
	local lack = {
		spade = true,
		club = true,
		heart = true,
		diamond = true,
	}

	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getEffectiveId() ~= fire_attack:getEffectiveId() then
			lack[card:getSuitString()] = nil
		end
	end	

	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isKongcheng() then
			local cards = enemy:getHandcards()
			local success = true
			for _, card in sgs.qlist(cards) do
				if lack[card:getSuitString()] then
					success = false
					break
				end
			end

			if success then
				use.card = fire_attack
				use.to:append(enemy)
				return
			end
		end
	end
end

function SmartAI:useCardByClassName(card, use)
	local class_name = card:className()
	local use_func = self["useCard" .. class_name]
	
	if use_func then
		use_func(self, card, use)
	end
end

function SmartAI:getSlashNumber(player)
	local n = 0
	if player:hasSkill("wusheng") then
		local cards = player:getCards("he")
		for _, card in sgs.qlist(cards) do
			if card:isRed() or card:inherits("Slash") then
				n = n + 1
			end
		end
	elseif player:hasSkill("wushen") then
		local cards = player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:getSuit() == sgs.Card_Heart or card:inherits("Slash") then
				n = n + 1
			end
		end
	else
		local cards = player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:inherits("Slash") then
				n = n + 1
			end
		end

		local left = cards:length() - n
		if player:hasWeapon("spear") then
			n = n + math.floor(left/2)
		end
	end

	if player:isLord() and player:hasSkill("jijiang") then
		local lieges = self.room:getLieges("shu", player)
		for _, liege in sgs.qlist(lieges) do
			if liege == "loyalist" then
				n = n + self:getSlashNumber(liege)
			end
		end
	end

	if player:hasSkill("wushuang") then
		n = n * 2
	end

	return n
end

function SmartAI:getJinkNumber(player)
	local n = 0

	local cards = player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("Jink") then
			n = n + 1
		end
	end

	if player:hasSkill("longdan") then
		for _, card in sgs.qlist(cards) do
			if card:inherits("Slash") then
				n = n + 1
			end
		end
	elseif player:hasSkill("qingguo") then
		for _, card in sgs.qlist(cards) do
			if card:isBlack() then
				n = n + 1
			end
		end
	end

	local armor = player:getArmor()
	if armor and armor:objectName() == "eight_diagram" then
		local judge_card = self.room:peek()
		if judge_card:isRed() then
			n = n + 1
		end
	end

	if player:isLord() and player:hasSkill("hujia") then
		local lieges = self.room:getLieges(player, "wei")
		for _, liege in sgs.qlist(lieges) do
			if liege:getRole() == "loyalist" then
				n = n + self:getJinkNumber(liege)
			end
		end
	end

	return n
end

function SmartAI:useCardDuel(duel, use)
	self:sort(self.enemies,"defense")
	local enemies = self:exclude(self.enemies, duel)
	for _, enemy in ipairs(enemies) do
		local n1 = self:getSlashNumber(self.player)
		local n2 = self:getSlashNumber(enemy)

		if n1 >= n2 then
			use.card = duel
			use.to:append(enemy)

			return
		end
	end
end

local function handcard_subtract_hp(a, b)
	local diff1 = a:getHandcardNum() - a:getHp()
	local diff2 = b:getHandcardNum() - b:getHp()

	return diff1 < diff2
end

function SmartAI:useCardSupplyShortage(card, use)
	table.sort(self.enemies, handcard_subtract_hp)

	local enemies = self:exclude(self.enemies, card)
	for _, enemy in ipairs(enemies) do
		if not enemy:containsTrick("supply_shortage") then
			use.card = card
			use.to:append(enemy)

			return
		end
	end
end

local function hp_subtract_handcard(a,b)
	local diff1 = a:getHp() - a:getHandcardNum()
	local diff2 = b:getHp() - b:getHandcardNum()

	return diff1 < diff2
end

function SmartAI:useCardIndulgence(card, use)
	table.sort(self.enemies, hp_subtract_handcard)

	local enemies = self:exclude(self.enemies, card)
	for _, enemy in ipairs(enemies) do
		if not enemy:containsTrick("indulgence") and not enemy:hasSkill("keji") then			
			use.card = card
			use.to:append(enemy)

			return
		end
	end
end

function SmartAI:useCardCollateral(card, use)
	self:sort(self.enemies,"threat")

	for _, enemy in ipairs(self.enemies) do
		if not self.room:isProhibited(self.player, enemy, card)
			and enemy:getWeapon() then

			for _, enemy2 in ipairs(self.enemies) do
				if enemy:canSlash(enemy2) then
					use.card = card
					use.to:append(enemy)
					use.to:append(enemy2)

					return
				end
			end
		end
	end
end

function SmartAI:useCardIronChain(card, use)
	local targets = {}
	self:sort(self.friends,"defense")
	for _, friend in ipairs(self.friends) do
		if friend:isChained() then
			table.insert(targets, friend)
		end
	end

	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isChained() and not self.room:isProhibited(self.player, enemy, card) 
			and not enemy:hasSkill("danlao") then
			table.insert(targets, enemy)
		end
	end

	use.card = card

	if targets[1] then
		use.to:append(targets[1])
	end

	if targets[2] then
		use.to:append(targets[2])
	end
end

-- the ExNihilo is always used
function SmartAI:useCardExNihilo(card, use)
	use.card = card
end

-- when self has wizard (zhangjiao, simayi, use it)
function SmartAI:useCardLightning(card, use)
	if self.has_wizard and self.room:isProhibited(self.player, self.player, card) then
		use.card = card
	end
end

function SmartAI:useCardGodSalvation(card, use)
	local good, bad = 0, 0
	for _, friend in ipairs(self.friends) do if friend:isWounded() then
		
			good = good + 10/getDefense(friend)
		end
	end

	for _, enemy in ipairs(self.enemies) do if enemy:isWounded() then
		bad = bad + 10/getDefense(enemy)
		end
	end

	if good > bad then
		use.card = card
	end
end

function SmartAI:useCardAmazingGrace(card, use)
	if #self.friends >= #self.enemies then
		use.card = card
	end
end

function SmartAI:useTrickCard(card, use)
	if card:inherits("AOE") then
		local good, bad = 0, 0
		for _, friend in ipairs(self.friends) do
			if self:aoeIsEffective(card, friend) then
				bad = bad + 20/(getDefense(friend))
				
			end
		end

		for _, enemy in ipairs(self.enemies) do
			if self:aoeIsEffective(card, enemy) then
				good = good + 20/(getDefense(enemy))
			end
		end

		if good > bad then
			use.card = card
		end
	else
		self:useCardByClassName(card, use)
	end
end


sgs.weapon_range =
{	
	Crossbow = 1,
	Blade = 3,
	Spear = 3,
	DoubleSword =2,
	QinggangSword=2,
	Axe=3,
	KylinBow=5,
	Halberd=4,
	IceSword=2,
	Fan=4,
	GudingBlade=2,	
}

function SmartAI:evaluateEquip(card)
		if card==nil then
			return 0
		end		
		
		local deltaSelfThreat = 0
		local currentRange = sgs.weapon_range[card:className()] or 1	
		
		for _,enemy in ipairs(self.enemies) do
			if self.player:distanceTo(enemy) <= currentRange then
					deltaSelfThreat=deltaSelfThreat+6/getDefense(enemy)
			end
		end
		
		if card:inherits("Crossbow") and deltaSelfThreat~=0 then 
			deltaSelfThreat = deltaSelfThreat + self:getSlashNumber(self.player)*2-2
		elseif card:inherits("Blade") then 
			deltaSelfThreat = deltaSelfThreat + self:getSlashNumber(self.player)
		elseif card:inherits("Spear") and 
			self.player:getHandcardNum()/2 - self:getSlashNumber(self.player)>0 then 
				deltaSelfThreat = deltaSelfThreat + self.player:getHandcardNum()/2 - self:getSlashNumber(self.player)
		else
			for _,enemy in ipairs(self.enemies) do
				if self.player:distanceTo(enemy) <= currentRange then
					if card:inherits("DoubleSword") and 
						enemy:getGeneral():isMale() ~= self.player:getGeneral():isMale() then
							deltaSelfThreat=deltaSelfThreat+1
					elseif card:inherits("QinggangSword") and enemy:getArmor() then
						deltaSelfThreat=deltaSelfThreat+1
					elseif card:inherits("Axe") and enemy:getHp()<3 then
						deltaSelfThreat=deltaSelfThreat+3-enemy:getHp()
					elseif card:inherits("KylinBow") and (enemy:getDefensiveHorse() or enemy:getDefensiveHorse())then
						deltaSelfThreat=deltaSelfThreat+1
						break
					elseif card:inherits("GudingBlade") and enemy:getHandcardNum()<3 then
						deltaSelfThreat=deltaSelfThreat+1
					end
				end
			end
		end
		return deltaSelfThreat
end

function SmartAI:useEquipCard(card, use)
	
	if card:inherits("Weapon") then
		if self:evaluateEquip(card) > self:evaluateEquip(self.player:getWeapon()) then
		use.card = card		
		end
	elseif self.lua_ai:useCard(card) then
		use.card = card
	end
end

function SmartAI:activate(use)

        --self.room:output(getCount(self.player:objectName()))
	
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards)
	for _, card in ipairs(cards) do
	
			local type = card:getTypeId()

			if type == sgs.Card_Basic then
				self:useBasicCard(card, use)
			elseif type == sgs.Card_Trick then
				self:useTrickCard(card, use)
			else
				self:useEquipCard(card, use)
			end
			if use:isValid() then
				return
			end
		end
end

sgs.ai_keep_value = {
	Shit = 6,

	Peach = 5,

	Analeptic = 4.5,
	Jink = 4,

	Nullification = 3,

	Slash = 2,
	ThunderSlash = 2.5,
	FireSlash = 2.6,


	AmazingGrace=-1,
	lightning=-1,
	
}

sgs.ai_use_value = {
--priority of using an active card

	Peach = 5,

	Dismantlement=4.5,
	Snatch=4.3,
	ExNihilo=4.1,

	AmazingGrace=4.0,

	ArcheryAttack=3.5,
	SavageAssault=3.5,

	Collateral=3,
	Duel=2.9,
	IronChain = 2.8,
	
	Analeptic = 2.7,

	FireSlash = 2.6,
	ThunderSlash = 2.5,
	Slash = 2.4,

	FireAttack=2,
	
	
	
	--god_salvation
	--deluge
	--supply_shortage
	--earthquake
	--indulgence
	--mudslide
	--lightning
	--typhoon
}

function SmartAI:getKeepValue(card)
	local class_name = card:className()
	return sgs.ai_keep_value[class_name] or 0
end

function SmartAI:getUseValue(card)
	local class_name = card:className()
	if card:inherits("EquipCard") then return 6 end
	return sgs.ai_use_value[class_name] or 0
end

function SmartAI:sortByKeepValue(cards)
	local compare_func = function(a,b)
		local value1 = self:getKeepValue(a)
		local value2 = self:getKeepValue(b)

		if value1 ~= value2 then
			return value1 < value2
		else
			return a:getNumber() < b:getNumber()
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:sortByUseValue(cards)
	local compare_func = function(a,b)
		local value1 = self:getUseValue(a)
		local value2 = self:getUseValue(b)

		if value1 ~= value2 then
			return value1 > value2
		else
			return a:getNumber() > b:getNumber()
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:askForDiscard(reason, discard_num, optional, include_equip)
	if optional then
		return {}
	else
		local flags = "h"
		if include_equip then
			flags = flags .. "e"
		end

		local cards = self.player:getCards(flags)
		cards = sgs.QList2Table(cards)
		self:sortByKeepValue(cards)
		local to_discard = {}
		for i=1, discard_num do
			table.insert(to_discard, cards[i]:getEffectiveId())
		end

		return to_discard
	end
end

function SmartAI:askForPlayerChosen(targets, reason)
	local r = math.random(0, targets:length() - 1)
	return targets:at(r)
end

-- used for SmartAI:askForChoice
sgs.ai_skill_choice = {}

function SmartAI:askForChoice(skill_name, choices)
	local choice = sgs.ai_skill_choice[skill_name]
	if type(choice) == "string" then
		return choice
	elseif type(choice) == "function" then
		return choice(self, choices)
	else
		local skill = sgs.Sanguosha:getSkill(skill_name)
		if skill then
			return skill:getDefaultChoice(self.player)
		else
			local choices_table = choices:split("+")
			local r = math.random(1, #choices_table)
			return choices_table[r]
		end
	end		
end

function SmartAI:getCardRandomly(who, flags)
	local cards = who:getCards(flags)
	local r = math.random(0, cards:length()-1)
	local card = cards:at(r)
	return card:getEffectiveId()
end

function SmartAI:askForCardChosen(who, flags, reason)

	if self:isFriend(who) then
		if flags:match("j") then
			local tricks = who:getCards("j")

			local lightning, indulgence, supply_shortage
			for _, trick in sgs.qlist(tricks) do
				if trick:inherits "Lightning" then
					lightning = trick:getId()
				elseif trick:inherits "Indulgence" or trick:getSuit() == sgs.Card_Diamond then
					indulgence = trick:getId()
				else
					supply_shortage = trick:getId()
				end
			end

			if not self.has_wizard and lightning then
				return lightning
			end

			if indulgence and supply_shortage then
				if who:getHp() < who:getHandcardNum() then
					return indulgence
				else
					return supply_shortage
				end
			end

			if indulgence or supply_shortage then
				return indulgence or supply_shortage
			end
		elseif flags:match("e") and who:hasSkill("xiaoji") then
			local equips = who:getEquips()
			if not equips:isEmpty() then
				return equips:at(0):getId()
			end
		end
	else
		if who:getHandcardNum()<3 and not who:isKongcheng() then return -1
		
		elseif flags:match("e") and who:getEquips() then
		
			if who:getDefensiveHorse() then
				for _,friend in ipairs(self.friends) do
					if friend:distanceTo(who)==friend:getAttackRange()+1 then 
					 	return who:getDefensiveHorse():getId()
					end
				end
			end
			
		
			if who:getOffensiveHorse() then
				for _,friend in ipairs(self.friends) do
					if who:distanceTo(friend)==who:getAttackRange() and
					 who:getAttackRange()>1 then 
					 	return who:getOffensiveHorse():getId() 
					end
				end
			end
			
			if who:getArmor() then 
				if who:getArmor():objectName()~="silver_lion" then return who:getArmor():getId() end
			end
			
			if who:getWeapon() then 
				for _,friend in ipairs(self.friends) do
					if who:distanceTo(friend)<who:getAttackRange() and who:distanceTo(friend)>1 then 
					 	return who:getWeapon():getId()
					end
				end
			end
		elseif flags:match("h") and not who:isKongcheng() then
			return -1
		elseif flags:match("e") and who:getEquips() then
			return who:getEquips():at(0):getId()
		end
	end

	return self:getCardRandomly(who, flags)
end

function SmartAI:askForCard(pattern,prompt)
        if prompt then
            local description=string.sub(prompt,0,prompt:find(":")-1)
            local source=string.sub(prompt,prompt:find(":")+1)
            if description == "duel-slash" then return "." end
        end
        if pattern == "@guicai" then self.room:output(prompt) end
        if pattern == "@guidao" then self.room:output(prompt) end
	return nil
end

function SmartAI:askForNullification(trick_name, from, to)
	return nil
end

function SmartAI:askForAG(card_ids, refusable, reason)
	if refusable then
		return -1
	end
	
	local r = math.random(1, #card_ids)
	return card_ids[r]
end

function SmartAI:getOneFriend()
	for _, friend in ipairs(self.friends) do
		if friend ~= self.player then
			return friend
		end
	end
end

function SmartAI.newSubclass(theClass, name)
	local class_name = name:sub(1, 1):upper() .. name:sub(2) .. "AI"
	local new_class = class(class_name, theClass)

	function new_class:initialize(player)
		super.initialize(self, player)
	end

	sgs.ai_classes[name] = new_class

	return new_class
end

function SmartAI:setOnceSkill(name)
	function self:filterEvent(event, player, data)
		super.filterEvent(self, event, player, data)
		if event == sgs.PhaseChange and player:objectName() == self.player:objectName()
			and player:getPhase() == sgs.Player_Play then
			self[name .. "_used"] = false
		end
	end
end

-- load other ai scripts
dofile "lua/ai/standard-ai.lua"
dofile "lua/ai/wind-ai.lua"
dofile "lua/ai/fire-ai.lua"
dofile "lua/ai/thicket-ai.lua"
dofile "lua/ai/god-ai.lua"
dofile "lua/ai/yitian-ai.lua"
dofile "lua/ai/nostalgia-ai.lua"

-- dofile "lua/ai/general_config.lua"
