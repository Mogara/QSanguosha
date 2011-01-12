-- This is the Smart AI, and it should load at the server side

require "middleclass"

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

	chaofeng = function(a, b)
		local c1 = sgs.ai_chaofeng[a:getGeneralName()]	or 0
		local c2 = sgs.ai_chaofeng[b:getGeneralName()] or 0
		
		if c1 == c2 then
			return SmartAI.getWeak(a) < SmartAI.getWeak(b)
		else
			return c1 > c2
		end
	end
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
}

-- this function is exposed to the host program
-- and it clones an AI instance by general name
function CloneAI(player, specialized)	
	if specialized then
		local ai_class = sgs.ai_classes[player:getGeneralName()]
		if ai_class then
			return ai_class(player).lua_ai
		end
	end
	
	return SmartAI(player).lua_ai
end

-- SmartAI is the base class for all other specialized AI classes
SmartAI = class "SmartAI" 

function SmartAI.getWeak(player)
	return player:getHp() * 2 + player:getHandcardNum()
end

-- the "initialize" function is just the "constructor"
function SmartAI:initialize(player)
	self.player = player
	self.room = player:getRoom()
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
	local friends = self.lua_ai:getFriends()
	self.friends = {}
	for i=0, friends:length()-1 do		
		table.insert(self.friends, friends:at(i))		
	end
	table.insert(self.friends, self.player)

	local enemies = self.lua_ai:getEnemies()
	self.enemies = {}
	for i=0, enemies:length()-1 do
		table.insert(self.enemies, enemies:at(i))
	end

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
	local func = sgs.ai_compare_funcs[key]
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

sgs.ai_skill_table = {
	eight_diagram = true,
	double_sword = true,
}

function SmartAI:askForSkillInvoke(skill_name, data)
	local skill = sgs.Sanguosha:getSkill(skill_name)
	if skill then
		return skill:getFrequency() == sgs.Skill_Frequent
	else
		return sgs.ai_skill_table[skill_name]
	end
end

function SmartAI:askForYiji(card_ids)
	return nil, 0
end

function SmartAI:askForUseCard(pattern, prompt)
	return "."
end

function SmartAI:slashIsEffective(slash, to)
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
		self:sort(self.enemies, "chaofeng")
		for _, enemy in ipairs(self.enemies) do
			if self.player:canSlash(enemy, true) and
				self:slashIsEffective(card, enemy) then

				-- fill the card use struct
				use.card = card
				local to = sgs.SPlayerList()
				to:append(enemy)
				use.to = to

				return
			end
		end
	elseif card:inherits("Peach") and self.player:isWounded() then
		use.card = card		
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

-- tell the user that the snatch should be used on this target or not
function SmartAI:useCardSnatch(card, to)
	if not self.player:hasSkill("qicai") and self.player:distanceTo(to) > 1 then
		return false
	elseif self:isNeutrality(to) then
		return true
	else
		return self:useCardDismantlement(card, to)
	end
end

function SmartAI:useCardDismantlement(card, to)
	if self:isFriend(to) then
		if to:containsTrick("lightning") then
			return not self.has_wizard
		else
			return to:containsTrick("indulgence") or to:containsTrick("supply_shortage")
		end
	elseif self:isEnemy(to) then
		return true
	end	
end

function SmartAI:useCardByClassName(card, use)
	local class_name = card:className()
	local use_func = self["useCard" .. class_name]
	if not use_func then
		return
	end

	local players = self.room:getOtherPlayers(self.player)
	for i=0, players:length()-1 do
		local player = players:at(i)
		if not self.room:isProhibited(self.player, player, card)
			and use_func(self, card, player) then

			use.card = card
			local to = sgs.SPlayerList()
			to:append(player)
			use.to = to
			
			return
		end
	end
end

function SmartAI:getSlashNumber(player)
	local n = 0
	if player:hasSkill("wusheng") then
		local cards = player:getCards("he")
		for i=0, cards:length()-1 do
			if cards:at(i):isRed() then
				n = n + 1
			end
		end
	elseif player:hasSkill("wushen") then
		local cards = player:getHandcards()
		for i=0, cards:length()-1 do
			if cards:at(i):getSuit() == sgs.Card_Heart then
				n = n + 1
			end
		end
	else
		local cards = player:getHandcards()
		for i=0, cards:length()-1 do
			if cards:at(i):inherits("Slash") then
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
		for i=0, lieges:length()-1 do
			local liege = lieges:at(i)
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

function SmartAI:useCardDuel(card, to)
	if self:isEnemy(to) then
		local n1 = self:getSlashNumber(self.player)
		local n2 = self:getSlashNumber(to)

		return n1 >= n2
	end
end

function SmartAI:useCardSupplyShortage(card, to)
	if not self:isEnemy(to) then
		return false
	end

	if to:containsTrick(card:objectName()) then
		return false
	end

	if self.player:hasSkill("qicai") then
		return true
	else
		return self.player:distanceTo(to) <= 1
	end
end

function SmartAI:useCardIndulgence(card, to)
	if not self:isEnemy(to) then
		return false
	end

	return not to:containsTrick(card:objectName())
end

function SmartAI:useCollateral(card, use)
	self:sort(self.enemies)

	for _, enemy in ipairs(self.enemies) do
		if not self.room:isProhibited(self.player, enemy, card) 
			and enemy:getWeapon() then
			
			for _, enemy2 in ipairs(self.enemies) do
				if enemy:canSlash(enemy2) then
					use.card = card
					use.from = self.player
					use.to = sgs.SPlayerList()
					use.to:append(enemy)
					use.to:append(enemy2)

					return
				end
			end
		end
	end
end

function SmartAI:useIronChain(card, use)
	local targets = {}
	self:sort(self.friends)
	for _, friend in ipairs(self.friends) do
		if friend:isChained() then
			table.insert(targets, friend)
		end
	end

	self:sort(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isChained() and not self.room:isProhibited(self.player, enemy, card) then
			table.insert(targets, enemy)
		end
	end

	use.card = card
	use.to = sgs.SPlayerList()

	if #targets >= 2 then
		use.to:append(targets[1])
		use.to:append(targets[2])
	end
end

function SmartAI:useTrickCard(card, use)
	if card:inherits("ExNihilo") then
		use.card = card
	elseif card:inherits("Lightning") then
		if self.has_wizard and self.room:isProhibited(self.player, self.player, card) then
			use.card = card
		end
	elseif card:inherits("IronChain") then
		self:useIronChain(card, use)
	elseif card:inherits("Collateral") then
		self:useCollateral(card, use)
	elseif card:inherits("GodSalvation") then
		local good, bad = 0, 0
		for _, friend in ipairs(self.friends) do
			if friend:isWounded() then
				good = good + 1
			end
		end

		for _, enemy in ipairs(self.enemies) do
			if enemy:isWounded() then
				bad = bad + 1
			end
		end

		if good > bad then
			use.card = card
		end
	elseif card:inherits("AOE") then
		local good, bad = 0, 0
		for _, friend in ipairs(self.friends) do
			if self:aoeIsEffective(card, friend) then
				bad = bad + 1
			end
		end

		for _, enemy in ipairs(self.enemies) do
			if self:aoeIsEffective(card, enemy) then
				good = good + 1
			end
		end

		if good > bad then
			use.card = card
		end
	elseif card:inherits("AmazingGrace") then
		if #self.friends >= #self.enemies then
			use.card = card
		end
	else
		self:useCardByClassName(card, use)
	end
end

function SmartAI:useEquipCard(card, use)
	if self.lua_ai:useCard(card) then
		use.card = card
	end
end

function SmartAI:activate(use)
	local cards = self.player:getHandcards()
	for i=0, cards:length()-1 do
		local card = cards:at(i)
		local type = card:getTypeId()
		
		if type == sgs.Card_Basic then
			self:useBasicCard(card, use)
		elseif type == sgs.Card_Trick then
			self:useTrickCard(card, use)
		else
			self:useEquipCard(card, use)
		end
	end
end

function SmartAI:askForDiscard(reason, discard_num, optional, include_equip)
	if optional then
		return {}
	else
		return self.player:forceToDiscard(discard_num, include_equip)
	end
end

function SmartAI:askForChoice(skill_name, choices)
	local skill = sgs.Sanguosha:getSkill(skill_name)
	return skill:getDefaultChoice()
end

function SmartAI:getCardRandomly(who, flags)
	local cards = who:getCards(flags)
	local r = math.random(0, cards:length()-1)
	local card = cards:at(r)
	return card:getEffectiveId()	
end

function SmartAI:askForCardChosen(who, flags, reason)
	if flags:match("j") and self:isFriend(who) then
		local tricks = who:getCards("j")
		if tricks:isEmpty() then
			return self:getCardRandomly(who, flags)
		end
	end

	return self:getCardRandomly(who, flags)
end

function SmartAI:askForCard(pattern)
	return nil
end

function SmartAI:getOneFriend()
	for _, friend in ipairs(self.friends) do
		if friend ~= self.player then
			return friend
		end
	end
end

dofile "ai/standard-ai.lua"
dofile "ai/fire-ai.lua"
dofile "ai/thicket-ai.lua"
dofile "ai/god-ai.lua"