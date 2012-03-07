-- This is the Smart AI, and it should be loaded and run at the server side

-- "middleclass" is the Lua OOP library written by kikito
-- more information see: https://github.com/kikito/middleclass
require "middleclass"

-- initialize the random seed for later use
math.randomseed(os.time())

-- SmartAI is the base class for all other specialized AI classes
SmartAI = class "SmartAI"

version = "QSanguosha AI 20120220 (V0.74 Stable)"
--- this function is only function that exposed to the host program
--- and it clones an AI instance by general name
-- @param player The ServerPlayer object that want to create the AI object
-- @return The AI object
function CloneAI(player)
	return SmartAI(player).lua_ai
end

sgs.ai_card_intention = 	{}
sgs.ai_playerchosen_intention = {}
sgs.role_evaluation = 		{}
sgs.ai_keep_value = 		{}
sgs.ai_use_value = 			{}
sgs.ai_use_priority = 		{}
sgs.ai_chaofeng = 			{}
sgs.ai_global_flags = 		{}
sgs.ai_skill_invoke = 		{}
sgs.ai_skill_suit = 		{}
sgs.ai_skill_cardask = 		{}
sgs.ai_skill_choice = 		{}
sgs.ai_skill_askforag = 	{}
sgs.ai_skill_pindian = 		{}
sgs.ai_filterskill_filter = {}
sgs.ai_skill_playerchosen = {}
sgs.ai_skill_discard = 		{}
sgs.ai_cardshow = 			{}
sgs.ai_skill_cardchosen = 	{}
sgs.ai_skill_use = 			{}
sgs.ai_cardneed = 			{}
sgs.ai_skill_use_func = 	{}
sgs.ai_skills = 			{}
sgs.ai_slash_weaponfilter = {}
sgs.ai_slash_prohibit = 	{}
sgs.ai_trick_prohibit =		{} -- obsolete
sgs.dynamic_value = 		{
	damage_card = 			{},
	control_usecard = 		{},
	control_card = 			{},
	lucky_chance = 			{},
	benefit = 				{}
}
sgs.ai_choicemade_filter = 	{
	cardUsed = 				{},
	cardResponsed = 		{},
	skillInvoke = 			{},
	skillChoice = 			{},
	Nullification = 		{},
	playerChosen =			{}
}

function setInitialTables()
	sgs.current_mode_players = 	{loyalist = 0, rebel = 0, renegade = 0}
	sgs.ai_type_name = 			{"Skill", "Basic", "Trick", "Equip"}
	sgs.target = 				{loyalist = nil, rebel = nil, renegade = nil } -- obsolete
	sgs.discard_pile =			global_room:getDiscardPile()
	sgs.draw_pile = 			global_room:getDrawPile()
	sgs.lose_equip_skill = 		"xiaoji|xuanfeng"
	sgs.need_kongcheng = 		"lianying|kongcheng"
	sgs.masochism_skill = 		"fankui|jieming|yiji|ganglie|enyuan|fangzhu|guixin|xinsheng"
	sgs.wizard_skill = 			"guicai|guidao|jilve|tiandu"
	sgs.wizard_harm_skill = 	"guicai|guidao|jilve"
	sgs.priority_skill = 		"dimeng|haoshi|qingnang|jijiu|jizhi|guzheng|qixi|xiaoji|jieyin|guose"
	sgs.exclusive_skill = 		"huilei|duanchang|enyuan|wuhun|leiji|buqu|jushou|yiji|ganglie|guixin"
	sgs.cardneed_skill =        "paoxiao|tianyi|xianzhen|shuangxiong|jizhi|guose|duanliang|qixi|qingnang|jieyin|renjie|zhiheng|rende|jujian|guicai|guidao|jilve|longhun|wusheng|longdan"
	
	for _, aplayer in sgs.qlist(global_room:getAllPlayers()) do
		table.insert(sgs.role_evaluation, aplayer:objectName())
		if aplayer:isLord() then
			sgs.role_evaluation[aplayer:objectName()] = {lord = 99999, rebel = 0, loyalist = 99999, renegade = 0}
		else
			sgs.role_evaluation[aplayer:objectName()] = {rebel = 30, loyalist = 30, renegade = 30}
		end
	end
	
end

function SmartAI:initialize(player)
	self.player = player
	self.room = player:getRoom()
	self.role  = player:getRole()
	self.lua_ai = sgs.LuaAI(player)
	self.lua_ai.callback = function(method_name, ...)
		local method = self[method_name]
		if method then
			local success, result1, result2
			success, result1, result2 = pcall(method, self, ...)
			if not success then
				self.room:writeToConsole(result1)
				self.room:writeToConsole(method_name)
				self.room:writeToConsole(debug.traceback())
				self.room:writeToConsole("Event stack:")
				self.room:outputEventStack()
				self.room:writeToConsole("End of Event Stack")
			else
				return result1, result2
			end
		end
	end

	self.retain = 2
	self.keepValue = {}
	self.kept = {}
	if not sgs.initialized then
		sgs.initialized = true
		sgs.turncount = 0
		global_room = self.room
		global_room:writeToConsole(version .. ", Powered by " .. _VERSION)
		
		setInitialTables()
		if sgs.isRolePredictable() then
			for _, aplayer in sgs.qlist(global_room:getOtherPlayers(global_room:getLord())) do
				sgs.role_evaluation[aplayer:objectName()][aplayer:getRole()] = 65535
			end
		end
	end

	if self.player:isLord() and not sgs.GetConfig("EnableHegemony", false) then
		if (sgs.ai_chaofeng[self.player:getGeneralName()] or 0) < 3 then
			sgs.ai_chaofeng[self.player:getGeneralName()] = 3
		end
	end
	
	self:updateAlivePlayerRoles()
	self:updatePlayers()
end

function sgs.getValue(player)
	if not player then global_room:writeToConsole(debug.traceback()) end
	return player:getHp() * 2 + player:getHandcardNum()
end

function sgs.getDefense(player)
	local defense = math.min(sgs.getValue(player), player:getHp() * 3)
	if player:getArmor() and not player:getArmor():inherits("GaleShell") then
		defense = defense + 2
	end
	if not player:getArmor() and player:hasSkill("bazhen") then
		defense = defense + 2
	end
	if not player:getArmor() and player:hasSkill("yizhong") then
		defense = defense + 2
	end
	local m = sgs.masochism_skill:split("|")
	for _, masochism in ipairs(m) do
		if player:hasSkill(masochism) then
			defense = defense + 1
		end
	end
	if player:getArmor() and player:getArmor():inherits("EightDiagram") and player:hasSkill("tiandu") then
		defense = defense + 0.3
	end
	if player:hasSkill("jieming") then
		defense = defense + 1
	end
	if player:getMark("@tied")>0 then
		defense = defense + 1
	end
	if player:hasSkill("qingguo") and player:getHandcardNum()>1 then
		defense = defense + 0.5
	end
	if player:hasSkill("longhun") and player:getHp() == 1 and player:getHandcardNum()>1 then
		defense = defense + 0.4
	end
	if player:hasSkill("longdan") and player:getHandcardNum()>2 then
		defense = defense + 0.3
	end
	return defense
end

function SmartAI:assignKeep(num,start)
	if num<=0 then return end
	if start then 
		self.keepValue={}
		self.kept={}
	end
	local cards=self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	self:sortByKeepValue(cards,true,self.kept)
	for _,card in ipairs(cards) do
		if not self.keepValue[card:getId()] then
			self.keepValue[card:getId()]=self:getKeepValue(card,self.kept)
			table.insert(self.kept,card)
			--self:log(card:className())
			self:assignKeep(num-1)
			break
		end
	end
end

function SmartAI:getKeepValue(card,kept)
	if not kept then return self.keepValue[card:getId()] or 0 end

	local class_name = card:className()
	local suit_string = card:getSuitString()
	local value, newvalue
	if sgs[self.player:getGeneralName().."_keep_value"] then
		value = sgs[self.player:getGeneralName().."_keep_value"][class_name]
		if value then return value end
	end
	if sgs[self.player:getGeneralName().."_suit_value"] then
		value = sgs[self.player:getGeneralName().."_suit_value"][suit_string]
	end
	newvalue = sgs.ai_keep_value[class_name] or 0
	for _,acard in ipairs(kept) do
		if acard:className() == card:className() then newvalue = newvalue - 1.2
		elseif acard:inherits("Slash") and card:inherits("Slash") then newvalue = newvalue - 1
		end
	end
	if not value or newvalue > value then value = newvalue end
	return value
end

function SmartAI:getUseValue(card)
	local class_name = card:className()
	local v = 0

	if card:inherits("GuhuoCard") then
		local userstring = card:toString()
		userstring = (userstring:split(":"))[2]
		local guhuocard = sgs.Sanguosha:cloneCard(userstring, card:getSuit(), card:getNumber())
		local usevalue = self:getUseValue(guhuocard,player) + #self.enemies*0.3
		if sgs.Sanguosha:getCard(card:getSubcards():first()):objectName() == userstring and card:getSuit() == sgs.Card_Heart then usevalue = usevalue + 3 end
		return usevalue
	end

	if card:getTypeId() == sgs.Card_Equip then
		if self:hasEquip(card) then
			if card:inherits("OffensiveHorse") and self.player:getAttackRange()>2 then return 5.5 end
			if card:inherits("DefensiveHorse") and self:isEquip("EightDiagram") then return 5.5 end
			return 9
		end
		if not self:hasSameEquip(card) then v = 6.7 end
		if self.weaponUsed and card:inherits("Weapon") then v = 2 end
		if self.player:hasSkill("qiangxi") and card:inherits("Weapon") then v = 2 end
		if self.player:hasSkill("kurou") and card:inherits("Crossbow") then return 9 end
		if self:hasSkill("bazhen") or self:hasSkill("yizhong") and card:inherits("Armor") then v = 2 end
		if self:hasSkills(sgs.lose_equip_skill) then return 10 end
	elseif card:getTypeId() == sgs.Card_Basic then
		if card:inherits("Slash") then
			if (self.player:hasFlag("drank") or self.player:hasFlag("tianyi_success") or self.player:hasFlag("luoyi")) then v = 8.7 end
			if self:isEquip("CrossBow") then v = v + 4 end
			v = v+self:getCardsNum("Slash")
		elseif card:inherits("Jink") then
			if self:getCardsNum("Jink") > 1 then v = v-6 end
		elseif card:inherits("Shit") and self.player:hasSkill("kuanggu") and card:getSuit()~= sgs.Card_Spade then
			v = 0.1
		end
	elseif card:getTypeId() == sgs.Card_Trick then
		if self.player:getWeapon() and not self:hasSkills(sgs.lose_equip_skill) and card:inherits("Collateral") then v = 2 end
		if self.player:getMark("shuangxiong") and card:inherits("Duel") then v = 8 end
		if self.player:hasSkill("jizhi") then v = 8.7 end
		if self.player:hasSkill("wumou") and card:isNDTrick() and not card:inherits("AOE") then
			if not (card:inherits("Duel") and self.player:hasUsed("WuqianCard")) then v = 1 end
		end
		if not self:hasTrickEffective(card) then v = 0 end
	end

	if self:hasSkills(sgs.need_kongcheng) then
		if self.player:getHandcardNum() == 1 then v = 10 end
	end
	if self:hasSkill({name = "halberd"}) and card:inherits("Slash") and self.player:getHandcardNum() == 1 then v = 10 end
	if card:getTypeId() == sgs.Card_Skill then
		if v == 0 then v = 10 end
	end

	if v == 0 then v = sgs.ai_use_value[class_name] or 0 end
	return v
end

function SmartAI:getUsePriority(card)
	local class_name = card:className()
	local v = 0
	if card:inherits("EquipCard") then
		if self:hasSkill(sgs.lose_equip_skill) then return 10 end
		if card:inherits("Armor") and not self.player:getArmor() then v = 6
		elseif card:inherits("Weapon") and not self.player:getWeapon() then v = 5.7
		elseif card:inherits("DefensiveHorse") and not self.player:getDefensiveHorse() then v = 5.8
		elseif card:inherits("OffensiveHorse") and not self.player:getOffensiveHorse() then v = 5.5
		end
		return v
	end

	if self.player:hasSkill("wuyan") then
		if card:inherits("Slash") then
			v = 4

		elseif card:inherits("Collateral") or card:inherits("Dismantlement") or card:inherits("Snatch") or card:inherits("IronChain") then v = 0
		end
		if v then return v else return sgs.ai_use_priority[class_name] end
	end
	if self.player:hasSkill("qingnang") then
		if card:inherits("Dismantlement") then v = 3.8
		elseif card:inherits("Collateral") then v = 3.9
		end
		if v then return v else return sgs.ai_use_priority[class_name] end
	end
	if self.player:hasSkill("rende") then
		if card:inherits("ExNihio") then v = 5.9 end
		return v or sgs.ai_use_priority[class_name]
	end

	v = sgs.ai_use_priority[class_name] or 0

	if card:inherits("Slash") and (card:getSuit() == sgs.Card_NoSuit) then v = v-0.1 end
	return v
end

function SmartAI:getDynamicUsePriority(card)
	if not card then return 0 end

	local type = card:getTypeId()
	local dummy_use = {}
	dummy_use.isDummy = true
	if type == sgs.Card_Trick then
		self:useTrickCard(card, dummy_use)
	elseif type == sgs.Card_Basic then
		self:useBasicCard(card, dummy_use)
	elseif type == sgs.Card_Equip then
		self:useEquipCard(card, dummy_use)
	else
		self:useSkillCard(card, dummy_use)
	end

	local good_null, bad_null = 0, 0
	for _, friend in ipairs(self.friends) do
		good_null = good_null + self:getCardsNum("Nullification", friend)
	end
	for _, enemy in ipairs(self.enemies) do
		bad_null = bad_null + self:getCardsNum("Nullification", enemy)
	end

	local value = self:getUsePriority(card)
	if dummy_use.card then
		local use_card = dummy_use.card
		local card_name = use_card:className()
		local dynamic_value

		if use_card:getTypeId() == sgs.Card_Equips then
			if self:hasSkills(sgs.lose_equip_skill) then value = value + 12 end
		end

		if use_card:getSkillName() == "wusheng" and
			sgs.Sanguosha:getCard(use_card:getEffectiveId()):inherits("GaleShell") and
			self:isEquip("GaleShell") then
			value = value + 10
		end

		if sgs.dynamic_value.benefit[class_name] then
			dynamic_value = 10
			if use_card:inherits("AmazingGrace") then
				for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
					dynamic_value = dynamic_value - 1
					if self:isEnemy(player) then dynamic_value = dynamic_value - ((player:getHandcardNum()+player:getHp())/player:getHp())*dynamic_value
					else dynamic_value = dynamic_value + ((player:getHandcardNum()+player:getHp())/player:getHp())*dynamic_value
					end
				end
			elseif use_card:inherits("GodSalvation") then
				local weak_mate, weak_enemy = 0, 0
				for _, player in sgs.qlist(self.room:getAllPlayers()) do
					if player:getHp() <= 1 and player:getHandcardNum() <= 1 then
						if self:isEnemy(player) then weak_enemy = weak_enemy + 1
						elseif self:isFriend(player) then weak_mate = weak_mate + 1
						end
					end
				end

				if weak_enemy > weak_mate then
					for _, card in sgs.qlist(self.player:getHandcards()) do
						if card:isAvailable(self.player) and sgs.dynamic_value.damage_card[card:className()] then
							if self:getDynamicUsePriority(card) - 0.5 > self:getUsePriority(card) then
								dynamic_value = -5
							end
						end
					end
				end
			elseif use_card:inherits("Peach") then
				dynamic_value = 7.85
			elseif use_card:inherits("QingnangCard") and self:getCardsNum("Snatch") > 0 and good_null >= bad_null then
				dynamic_value = 6.55
			elseif use_card:inherits("RendeCard") and self.player:usedTimes("RendeCard") < 2 then
				if not self.player:isWounded() then dynamic_value = 6.57
				elseif self:isWeak() then dynamic_value = 7.9
				else dynamic_value = 7.86
				end
			elseif use_card:inherits("JujianCard") then
				if not self.player:isWounded() then dynamic_value = 0
				else dynamic_value = 7.5
				end
			end
			value = value + dynamic_value
		elseif sgs.dynamic_value.damage_card[class_name] then
			local others
			if dummy_use.to then others = dummy_use.to else others = self.room:getOtherPlayers(self.player) end
			dummy_use.probably_hit = {}

			for _, enemy in sgs.qlist(others) do
				if self:isEnemy(enemy) and (enemy:getHp() <= 2 or enemy:isKongcheng())
					and self:getCardsNum("Analeptic", enemy) == 0 and self:getCardsNum("Peach", enemy) == 0 then
					table.insert(dummy_use.probably_hit, enemy)
					break
				end
			end

			if #dummy_use.probably_hit > 0 then
				self:sort(dummy_use.probably_hit, "defense")
				local probably_hit
				for _, hit in ipairs(dummy_use.probably_hit) do
					if not self:hasSkills(sgs.masochism_skill, hit) then
						probably_hit = hit
						break
					end
				end
				if not probably_hit then
					probably_hit = dummy_use.probably_hit[1]
					value = value + 12.5
				else
					value = value + 14
				end
				value = value - (probably_hit:getHp() - 1)/2.0

				if use_card:inherits("Slash") and self:getCardsNum("Jink", probably_hit) == 0 then
					value = value + 5
				elseif use_card:inherits("FireAttack") then
					value = value + 0.5 + self:getHandcardNum()
				elseif use_card:inherits("Duel") then
					value = value + 2 + (self:getHandcardNum() - self:getCardsNum("Slash", probably_hit))
				end
			end
		elseif sgs.dynamic_value.control_card[class_name] then
			if use_card:getTypeId() == sgs.Card_Trick then dynamic_value = 7 - bad_null/good_null else dynamic_value = 6.65 end
			value = value + dynamic_value
		elseif sgs.dynamic_value.control_usecard[class_name] then
			value = value + 6.6
		elseif sgs.dynamic_value.lucky_chance[class_name] then
			value = value + (#self.enemies - #self.friends)
		end
	end

	return value
end

function SmartAI:cardNeed(card)
	if not self.friends then self.room:writeToConsole(debug.traceback()) self.room:writeToConsole(sgs.turncount) return end
	local class_name = card:className()
	local suit_string = card:getSuitString()
	local value
	if card:inherits("Peach") then
		self:sort(self.friends,"hp")
		if self.friends[1]:getHp() < 2 then return 10 end
		if (self.player:getHp() < 3 or self.player:getLostHp() > 1 and not self:hasSkill("longhun")) or self:hasSkills("kurou|benghuai") then return 14 end
		return self:getUseValue(card)
	end
	if self:isWeak() and card:inherits("Jink") and self:getCardsNum("Jink") < self.player:getHp() then return 12 end
	if sgs[self.player:getGeneralName().."_keep_value"] then
		value = sgs[self.player:getGeneralName().."_keep_value"][class_name]
		if value then return value+4 end
	end
	if sgs[self.player:getGeneralName().."_suit_value"] then
		value = (sgs[self.player:getGeneralName().."_suit_value"][suit_string])
		if value then return value+4 end
	end

	if card:inherits("Jink") and self:getCardsNum("Jink") == 0 then return 5.9 end
	if card:inherits("Analeptic") then
		if self.player:getHp() < 2 then return 10 end
	end
	if card:inherits("Slash") and (self:getCardsNum("Slash") > 0) then return 4 end
	if card:inherits("Weapon") and (not self.player:getWeapon()) and (self:getCardsNum("Slash") > 1) then return 6 end
	if card:inherits("Nullification") and self:getCardsNum("Nullification") == 0 then
		if self.player:containsTrick("indulgence") or self.player:containsTrick("supply_shortage") then return 10 end
		for _,friend in ipairs(self.friends) do
			if friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage") then return 7 end
		end
		return 6
	end
	return self:getUseValue(card)
end

-- compare functions
sgs.ai_compare_funcs = {
	hp = function(a, b)
		return a:getHp() < b:getHp()
	end,

	handcard = function(a, b)
		return a:getHandcardNum() < b:getHandcardNum()
	end,

	value = function(a, b)
		return sgs.getValue(a) < sgs.getValue(b)
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
		return sgs.getDefense(a) < sgs.getDefense(b)
	end,

	threat = function (a, b)
		local players = sgs.QList2Table(a:getRoom():getOtherPlayers(a))
		local d1 = a:getHandcardNum()
		for _, player in ipairs(players) do
			if a:canSlash(player,true) then
				d1 = d1+10/(sgs.getDefense(player))
			end
		end
		players = sgs.QList2Table(b:getRoom():getOtherPlayers(b))
		local d2 = b:getHandcardNum()
		for _, player in ipairs(players) do
			if b:canSlash(player,true) then
				d2 = d2+10/(sgs.getDefense(player))
			end
		end

		local c1 = sgs.ai_chaofeng[a:getGeneralName()]	or 0
		local c2 = sgs.ai_chaofeng[b:getGeneralName()] or 0

		return d1+c1/2 > d2+c2/2
	end,
}

function SmartAI:sort(players, key, inverse)
	if not players then self.room:writeToConsole(debug.traceback()) end
	local func =  sgs.ai_compare_funcs[key or "chaofeng"]
	table.sort(players, func)
	if inverse then players = sgs.reverse(players) end
end

function SmartAI:sortByKeepValue(cards,inverse,kept)
	local compare_func = function(a,b)
		local value1 = self:getKeepValue(a,kept)
		local value2 = self:getKeepValue(b,kept)

		if value1 ~= value2 then
			if inverse then return value1 > value2 end
			return value1 < value2
		else
			return a:getNumber() < b:getNumber()
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:sortByUseValue(cards,inverse)
	local compare_func = function(a,b)
		local value1 = self:getUseValue(a)
		local value2 = self:getUseValue(b)

		if value1 ~= value2 then
				if not inverse then return value1 > value2
				else return value1 < value2
				end
		else
				return a:getNumber() > b:getNumber()
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:sortByUsePriority(cards)
	local compare_func = function(a,b)
		local value1 = self:getUsePriority(a)
		local value2 = self:getUsePriority(b)

		if value1 ~= value2 then
			return value1 > value2
		else
			return a:getNumber() > b:getNumber()
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:sortByDynamicUsePriority(cards)
	local compare_func = function(a,b)
		local value1 = self:getDynamicUsePriority(a)
		local value2 = self:getDynamicUsePriority(b)

		if value1 ~= value2 then
			return value1 > value2
		else
			return a and a:getTypeId() ~= sgs.Card_Skill and not (b and b:getTypeId() ~= sgs.Card_Skill)
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:sortByCardNeed(cards)
	local compare_func = function(a,b)
		local value1 = self:cardNeed(a)
		local value2 = self:cardNeed(b)

		if value1 ~= value2 then
			return value1 < value2
		else
			return a:getNumber() > b:getNumber()
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:getPriorTarget()
	local function inOneGroup(player)
		if sgs.isRolePredictable() then return self:isFriend(player) end
		if sgs.evaluatePlayerRole(player) == "unknown" then return true end
		return sgs.evaluatePlayerRole(player) == sgs.evaluatePlayerRole(self.player) and not sgs.evaluatePlayerRole(self.player) == "renegade"
	end
	if #self.enemies == 0 then return end
	local prior_targets = {}
	for _, enemy in ipairs(self.enemies) do	
		if not inOneGroup(enemy) and self:hasSkills(sgs.priority_skill, player) then
			table.insert(prior_targets, enemy)
		end
	end
	
	if #prior_targets > 0 then
		self:sort(prior_targets, "threat")
		return prior_targets[1]
	end
	
	self:sort(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		if not self:hasSkills(sgs.exclusive_skill, enemy) and not inOneGroup(enemy) then return enemy end
	end
	
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if self:isWeak(enemy) and not inOneGroup(enemy) then return enemy end
	end
	
	self:sort(self.enemies, "hp")
	return self.enemies[1]
end

function sgs.evaluatePlayerRole(player)
	if not player then global_room:writeToConsole("Player is empty in role's evaluation!") return end
	if player:isLord() then return "loyalist" end
	if sgs.current_mode_players["loyalist"] == 0 and sgs.current_mode_players["rebel"] == 0 then return "renegade" end
	if sgs.current_mode_players["loyalist"] == 0 and sgs.current_mode_players["renegade"] == 0 then return "rebel" end

	local max_value = math.max(sgs.role_evaluation[player:objectName()]["loyalist"], sgs.role_evaluation[player:objectName()]["renegade"])
	max_value = math.max(max_value, sgs.role_evaluation[player:objectName()]["rebel"])
	if max_value == sgs.role_evaluation[player:objectName()]["loyalist"] then
		if sgs.current_mode_players["loyalist"] == 0 and not sgs.current_mode_players["renegade"] == 0 then return "renegade" end 
	elseif max_value == sgs.role_evaluation[player:objectName()]["rebel"] then
		if sgs.current_mode_players["rebel"] == 0 and not sgs.current_mode_players["renegade"] == 0 then return "renegade" end
	end
	
	if sgs.role_evaluation[player:objectName()]["loyalist"] == sgs.role_evaluation[player:objectName()]["renegade"] and
		sgs.role_evaluation[player:objectName()]["rebel"] == sgs.role_evaluation[player:objectName()]["renegade"] then 
		return "unknown"
	end
	
	if sgs.role_evaluation[player:objectName()]["loyalist"] == sgs.role_evaluation[player:objectName()]["renegade"] then
		if sgs.role_evaluation[player:objectName()]["loyalist"] + 20 < sgs.role_evaluation[player:objectName()]["rebel"] then return "rebel"
		else return "unknown"
		end
	elseif sgs.role_evaluation[player:objectName()]["rebel"] == sgs.role_evaluation[player:objectName()]["renegade"] then
		if sgs.role_evaluation[player:objectName()]["rebel"] + 20 < sgs.role_evaluation[player:objectName()]["loyalist"] then return "loyalist"
		else return "unknown"
		end
	elseif sgs.role_evaluation[player:objectName()]["rebel"] == sgs.role_evaluation[player:objectName()]["loyalist"] then
		return "renegade"
	end
	
	if max_value == sgs.role_evaluation[player:objectName()]["loyalist"] then
		local rest = math.max(sgs.role_evaluation[player:objectName()]["rebel"], sgs.role_evaluation[player:objectName()]["renegade"])
		if sgs.role_evaluation[player:objectName()]["loyalist"] - rest <= 20 then return "unknown"
		else return "loyalist"
		end
	elseif max_value == sgs.role_evaluation[player:objectName()]["renegade"] then
		local rest = math.max(sgs.role_evaluation[player:objectName()]["rebel"], sgs.role_evaluation[player:objectName()]["loyalist"])
		if sgs.role_evaluation[player:objectName()]["renegade"] - rest <= 20 then return "unknown"
		else return "renegade"
		end
	elseif max_value == sgs.role_evaluation[player:objectName()]["rebel"] then
		local rest = math.max(sgs.role_evaluation[player:objectName()]["renegade"], sgs.role_evaluation[player:objectName()]["loyalist"])
		if sgs.role_evaluation[player:objectName()]["rebel"] - rest <= 20 then return "unknown"
		else return "rebel"
		end
	end
	
	return "unknown"
end

function sgs.backwardEvaluation(player)
	if player:isLord() then return "loyalist" end
	if sgs.evaluatePlayerRole(player) ~= "unknown" then return sgs.evaluatePlayerRole(player) end
	local players = global_room:getOtherPlayers(player)
	local rest_players = {}
	for _, arole in ipairs({"loyalist", "rebel", "renegade"}) do
		rest_players[arole] = sgs.current_mode_players[arole]
	end
	local unknowns = {}
	
	for _, p in sgs.qlist(players) do
		if not p:isLord() and sgs.evaluatePlayerRole(p) ~= "unknown" then 
			rest_players[sgs.evaluatePlayerRole(p)] = rest_players[sgs.evaluatePlayerRole(p)] - 1
		else
			table.insert(unknowns, p)
		end
	end
	if #unknowns == 0 then sgs.checkMisjudge() end
	
	if rest_players["rebel"] == 0 and rest_players["loyalist"] == 0 and rest_players["renegade"] == #unknowns then return "renegade"
	elseif rest_players["rebel"] == 0 and rest_players["renegade"] == 0 and rest_players["loyalist"] == #unknowns then return "loyalist"
	elseif rest_players["renegade"] == 0 and rest_players["loyalist"] == 0 and rest_players["rebel"] == #unknowns then return "rebel"
	end
	
	return "unknown"
end

function sgs.modifiedRoleEvaluation()
	local players = global_room:getOtherPlayers(global_room:getLord())
	for _, player in sgs.qlist(players) do
		local name = player:objectName()
		if sgs.evaluatePlayerRole(player) == "unknown" then
			local backward = sgs.backwardEvaluation(player)
			if backward == "loyalist" then 
				sgs.role_evaluation[name][backward] = math.max(sgs.role_evaluation[name]["rebel"], sgs.role_evaluation[name]["renegade"]) + 20
			elseif backward == "rebel" then 
				sgs.role_evaluation[name][backward] = math.max(sgs.role_evaluation[name]["loyalist"], sgs.role_evaluation[name]["renegade"]) + 20
			elseif backward == "renegade" then 
				sgs.role_evaluation[name][backward] = math.max(sgs.role_evaluation[name]["loyalist"], sgs.role_evaluation[name]["rebel"]) + 20
			end
		end
	end
end

function sgs.modifiedRoleTrends(role)
	local players = global_room:getOtherPlayers(global_room:getLord())
	local evaluated = {}
	for _, player in sgs.qlist(players) do
		if sgs.evaluatePlayerRole(player) == role then table.insert(evaluated, player) end
	end
	
	local sort_func = 
	{
		rebel = function(a, b)
			return sgs.role_evaluation[a:objectName()]["rebel"] < sgs.role_evaluation[b:objectName()]["rebel"]
		end,
		loyalist = function(a, b)
			return sgs.role_evaluation[a:objectName()]["loyalist"] < sgs.role_evaluation[b:objectName()]["loyalist"]
		end,
		renegade = function(a, b)
			return math.abs(sgs.role_evaluation[a:objectName()]["loyalist"] - sgs.role_evaluation[a:objectName()]["rebel"])
						> math.abs(sgs.role_evaluation[b:objectName()]["loyalist"] - sgs.role_evaluation[b:objectName()]["rebel"])
		end
	}
	local clearance = #evaluated - sgs.current_mode_players[role]
	local min_trends = {}
	if clearance <= 0 then global_room:writeToConsole("Modified Role Trends Failed!") return end
	
	table.sort(evaluated, sort_func[role])
	for _, p in ipairs(evaluated) do
		table.insert(min_trends, p)
		clearance = clearance - 1
		if clearance <= 0 then break end
	end
	
	for _, modifier in ipairs(min_trends) do
		local name = modifier:objectName()
		if role == "renegade" then
			sgs.role_evaluation[name][role] = math.max(sgs.role_evaluation[name]["rebel"], sgs.role_evaluation[name]["loyalist"]) - 15
		elseif role == "rebel" then
			sgs.role_evaluation[name][role] = math.max(sgs.role_evaluation[name]["renegade"], sgs.role_evaluation[name]["loyalist"]) - 15
		elseif role == "loyalist" then
			sgs.role_evaluation[name][role] = math.max(sgs.role_evaluation[name]["rebel"], sgs.role_evaluation[name]["renegade"]) - 15
		end
	end
	
	global_room:writeToConsole("Modified Role Trends Success!")
end

function sgs.evaluateRoleTrends(player)
	if player:isLord() then return "loyalist" end
	local max_value = math.max(sgs.role_evaluation[player:objectName()]["loyalist"], sgs.role_evaluation[player:objectName()]["renegade"])
	max_value = math.max(max_value, sgs.role_evaluation[player:objectName()]["rebel"])
	local rebel_value = sgs.role_evaluation[player:objectName()]["rebel"]
	local renegade_value = sgs.role_evaluation[player:objectName()]["renegade"]
	local loyalist_value = sgs.role_evaluation[player:objectName()]["loyalist"]
	
	if (rebel_value == renegade_value and max_value == rebel_value) or
		(rebel_value == loyalist_value and max_value == loyalist_value) or
		(renegade_value == loyalist_value and max_value == renegade_value) then return "neutral"
	elseif max_value == loyalist_value then return "loyalist"
	elseif max_value == renegade_value then return "renegade"
	elseif max_value == rebel_value then return "rebel"
	else return "neutral"
	end
end

function sgs.compareRoleEvaluation(player, first, second)
	if player:isLord() then return "loyalist" end
	local max_value = math.max(sgs.role_evaluation[player:objectName()][first], sgs.role_evaluation[player:objectName()][second])
	if sgs.role_evaluation[player:objectName()][first] == sgs.role_evaluation[player:objectName()][second] then return "unknown"
	elseif max_value == sgs.role_evaluation[player:objectName()][first] then return first
	elseif max_value == sgs.role_evaluation[player:objectName()][second] then return second
	else return "unknown"
	end
end

function sgs.isRolePredictable()
	if sgs.GetConfig("RolePredictable", true) then return true end
	local mode = global_room:getMode()
	if not mode:find("0") or mode:find("03p") or mode:find("02_1v1") or mode:find("04_1v3") or mode == "06_3v3" then return true end
	return false
end

function sgs.findIntersectionSkills(first, second)
	if type(first) == "string" then first = first:split("|") end
	if type(second) == "string" then second = second:split("|") end

	local findings = {}
	for _, skill in ipairs(first) do
		for _, compare_skill in ipairs(second) do
			if skill == compare_skill and not table.contains(findings, skill) then table.insert(findings, skill) end
		end
	end
	return findings
end

function sgs.findUnionSkills(first, second)
	if type(first) == "string" then first = first:split("|") end
	if type(second) == "string" then second = second:split("|") end
	
	local findings = table.copyFrom(first)
	for _, skill in ipairs(second) do
		if not table.contains(findings, skill) then table.insert(findings, skill) end
	end
	
	return findings
end
	

sgs.ai_card_intention.general=function(from,to,level)
	if sgs.isRolePredictable() then return end
	if not to then global_room:writeToConsole(debug.traceback()) return end
	if from:isLord() then return end
	--sgs.outputProcessValues(from:getRoom())
	sgs.outputRoleValues(from, level)
	
	if level > 0 then
		if to:isLord() and sgs.current_mode_players["rebel"] == 0 then
			sgs.role_evaluation[from:objectName()]["renegade"] = sgs.role_evaluation[from:objectName()]["renegade"] + level * 1.5
		elseif to:isLord() or sgs.evaluatePlayerRole(to) == "loyalist" then
			if sgs.evaluateRoleTrends(from) == "loyalist" then
				sgs.role_evaluation[from:objectName()]["renegade"] = sgs.role_evaluation[from:objectName()]["renegade"] + level
			elseif sgs.evaluatePlayerRole(from) == "rebel" then
				sgs.role_evaluation[from:objectName()]["rebel"] = sgs.role_evaluation[from:objectName()]["rebel"] + level
			elseif sgs.evaluatePlayerRole(from) == "renegade" then
				sgs.role_evaluation[from:objectName()]["rebel"] = sgs.role_evaluation[from:objectName()]["rebel"] + level/3.3
			elseif sgs.evaluateRoleTrends(from) == "neutral" then
				sgs.role_evaluation[from:objectName()]["rebel"] = sgs.role_evaluation[from:objectName()]["rebel"] + level/2.3
			elseif sgs.compareRoleEvaluation(from, "rebel", "loyalist") == "rebel" then
				sgs.role_evaluation[from:objectName()]["rebel"] = sgs.role_evaluation[from:objectName()]["rebel"] + level/2.5
			elseif sgs.compareRoleEvaluation(from, "rebel", "loyalist") == "loyalist" then
				sgs.role_evaluation[from:objectName()]["renegade"] = sgs.role_evaluation[from:objectName()]["renegade"] + level/3
			elseif sgs.evaluateRoleTrends(from) == "renegade" then
				level = level + (sgs.current_mode_players["loyalist"] - sgs.current_mode_players["rebel"])*15
				sgs.role_evaluation[from:objectName()]["renegade"] = sgs.role_evaluation[from:objectName()]["renegade"] + level/1.3
			end
		elseif sgs.evaluatePlayerRole(to) == "rebel" then 
			if sgs.evaluateRoleTrends(from) == "rebel" then
				sgs.role_evaluation[from:objectName()]["renegade"] = sgs.role_evaluation[from:objectName()]["renegade"] + level
			elseif sgs.evaluatePlayerRole(from) == "loyalist" then
				sgs.role_evaluation[from:objectName()]["loyalist"] = sgs.role_evaluation[from:objectName()]["loyalist"] + level
			elseif sgs.evaluatePlayerRole(from) == "renegade" then
				sgs.role_evaluation[from:objectName()]["loyalist"] = sgs.role_evaluation[from:objectName()]["loyalist"] + level/3.3
			elseif sgs.evaluateRoleTrends(from) == "neutral" then
				sgs.role_evaluation[from:objectName()]["loyalist"] = sgs.role_evaluation[from:objectName()]["loyalist"] + level/2.3
			elseif sgs.compareRoleEvaluation(from, "rebel", "loyalist") == "rebel" then
				sgs.role_evaluation[from:objectName()]["renegade"] = sgs.role_evaluation[from:objectName()]["renegade"] + level/2.5
			elseif sgs.compareRoleEvaluation(from, "rebel", "loyalist") == "loyalist" then
				sgs.role_evaluation[from:objectName()]["rebel"] = sgs.role_evaluation[from:objectName()]["rebel"] + level/3
			elseif sgs.evaluateRoleTrends(from) == "renegade" then
				level = level + (sgs.current_mode_players["rebel"] - sgs.current_mode_players["loyalist"])*15
				sgs.role_evaluation[from:objectName()]["renegade"] = sgs.role_evaluation[from:objectName()]["renegade"] + level/1.3
			end
		elseif sgs.evaluateRoleTrends(to) == "renegade" then
			if sgs.current_mode_players["rebel"] <= sgs.current_mode_players["loyalist"] then
				sgs.role_evaluation[from:objectName()]["loyalist"] = sgs.role_evaluation[from:objectName()]["loyalist"] + level/2.5
			else
				sgs.role_evaluation[from:objectName()]["rebel"] = sgs.role_evaluation[from:objectName()]["rebel"] + level/3
			end
		end
	elseif level < 0 then
		level = -level
		if to:isLord() or sgs.evaluatePlayerRole(to) == "loyalist" then
			if sgs.evaluateRoleTrends(from) == "rebel" then
				sgs.role_evaluation[from:objectName()]["renegade"] = sgs.role_evaluation[from:objectName()]["renegade"] + level
			elseif sgs.evaluatePlayerRole(from) == "loyalist" then
				sgs.role_evaluation[from:objectName()]["loyalist"] = sgs.role_evaluation[from:objectName()]["loyalist"] + level/1.1
			elseif sgs.evaluatePlayerRole(from) == "renegade" then
				sgs.role_evaluation[from:objectName()]["loyalist"] = sgs.role_evaluation[from:objectName()]["loyalist"] + level/1.9
			elseif sgs.evaluateRoleTrends(from) == "neutral" then
				sgs.role_evaluation[from:objectName()]["loyalist"] = sgs.role_evaluation[from:objectName()]["loyalist"] + level/3
			elseif sgs.compareRoleEvaluation(from, "rebel", "loyalist") == "rebel" then
				sgs.role_evaluation[from:objectName()]["renegade"] = sgs.role_evaluation[from:objectName()]["renegade"] + level/3.3
			elseif sgs.compareRoleEvaluation(from, "rebel", "loyalist") == "loyalist" then
				sgs.role_evaluation[from:objectName()]["loyalist"] = sgs.role_evaluation[from:objectName()]["loyalist"] + level/4.5
			elseif sgs.evaluateRoleTrends(from) == "renegade" then
				level = level + (sgs.current_mode_players["loyalist"] - sgs.current_mode_players["rebel"])*15
				sgs.role_evaluation[from:objectName()]["renegade"] = sgs.role_evaluation[from:objectName()]["renegade"] + level/1.3
			end
		elseif sgs.evaluatePlayerRole(to) == "rebel" then 
			if sgs.evaluateRoleTrends(from) == "loyalist" then
				sgs.role_evaluation[from:objectName()]["renegade"] = sgs.role_evaluation[from:objectName()]["renegade"] + level
			elseif sgs.evaluatePlayerRole(from) == "rebel" then
				sgs.role_evaluation[from:objectName()]["rebel"] = sgs.role_evaluation[from:objectName()]["rebel"] + level/1.1
			elseif sgs.evaluatePlayerRole(from) == "renegade" then 
				sgs.role_evaluation[from:objectName()]["rebel"] = sgs.role_evaluation[from:objectName()]["rebel"] + level/1.9
			elseif sgs.evaluateRoleTrends(from) == "neutral" then
				sgs.role_evaluation[from:objectName()]["rebel"] = sgs.role_evaluation[from:objectName()]["rebel"] + level/2
			elseif sgs.compareRoleEvaluation(from, "rebel", "loyalist") == "rebel" then
				sgs.role_evaluation[from:objectName()]["rebel"] = sgs.role_evaluation[from:objectName()]["rebel"] + level/3.3
			elseif sgs.compareRoleEvaluation(from, "rebel", "loyalist") == "loyalist" then
				sgs.role_evaluation[from:objectName()]["renegade"] = sgs.role_evaluation[from:objectName()]["renegade"] + level/3.5
			elseif sgs.evaluateRoleTrends(from) == "renegade" then
				level = level + (sgs.current_mode_players["rebel"] - sgs.current_mode_players["loyalist"])*15
				sgs.role_evaluation[from:objectName()]["renegade"] = sgs.role_evaluation[from:objectName()]["renegade"] + level/1.3
			end
		elseif sgs.evaluateRoleTrends(to) == "renegade" then
			if sgs.current_mode_players["rebel"] <= sgs.current_mode_players["loyalist"] then
				sgs.role_evaluation[from:objectName()]["rebel"] = sgs.role_evaluation[from:objectName()]["rebel"] + level/2.5
			else
				sgs.role_evaluation[from:objectName()]["loyalist"] = sgs.role_evaluation[from:objectName()]["loyalist"] + level/3
			end
		end
	end

	--sgs.outputProcessValues(from:getRoom())
	sgs.outputRoleValues(from, level)
	sgs.checkMisjudge()
end

function sgs.outputRoleValues(player, level)
	global_room:writeToConsole(player:getGeneralName() .. " " .. level .. " " .. sgs.evaluatePlayerRole(player) .." R" .. sgs.role_evaluation[player:objectName()]["rebel"] .. " L" ..
		sgs.role_evaluation[player:objectName()]["loyalist"] .. " r" .. sgs.role_evaluation[player:objectName()]["renegade"]
		.. " " .. sgs.gameProcess(player:getRoom()) .. " " .. sgs.current_mode_players["loyalist"] .. sgs.current_mode_players["rebel"]
		.. sgs.current_mode_players["renegade"]) 
end

function sgs.updateIntention(from, to, intention, card)
	if not to then global_room:writeToConsole(debug.traceback()) end
	if from:objectName() == to:objectName() then return end
	
	sgs.ai_card_intention.general(from, to, intention) 
	sgs.checkMisjudge(player)
end

function sgs.updateIntentions(from, tos, intention, card)
	for _, to in ipairs(tos) do
		sgs.updateIntention(from, to, intention, card)
	end
end

function sgs.outputProcessValues(room)
	local loyal = 0
	local rebel = 0
	local health = false
	local loyal_value, rebel_value = 0, 0, 0
	local currentplayer = room:getCurrent()
	for _, aplayer in sgs.qlist(room:getAlivePlayers()) do
		if not (aplayer:objectName() == currentplayer:objectName() and aplayer:getRole() == "renegade") then 
		if (not sgs.isRolePredictable() and sgs.evaluatePlayerRole(aplayer) == "rebel" )
			or (sgs.isRolePredictable() and aplayer:getRole() == "rebel") then
			local rebel_hp
			rebel = rebel+1
			if aplayer:hasSkill("benghuai") and aplayer:getHp() > 4 then rebel_hp = 4
			else rebel_hp = aplayer:getHp() end
			if aplayer:getMaxHP() == 3 then rebel_value = rebel_value + 0.5 end
			rebel_value = rebel_value + rebel_hp + math.max(sgs.getDefense(aplayer) - rebel_hp * 2, 0) * 0.7
			if aplayer:getWeapon() and aplayer:getWeapon():className() ~= "Weapon" then
				rebel_value = rebel_value + math.min(1.5, math.min(sgs.weapon_range[aplayer:getWeapon():className()],room:alivePlayerCount()/2)/2) * 0.4
			end
			global_room:writeToConsole("rebelnum is " .. rebel .. " value is " .. rebel_value)
		elseif (not sgs.isRolePredictable() and sgs.evaluatePlayerRole(aplayer) == "loyalist")
				or (sgs.isRolePredictable() and aplayer:getRole() == "loyalist") or aplayer:isLord() then
			local loyal_hp
			loyal = loyal+1
			if aplayer:hasSkill("benghuai") and aplayer:getHp() > 4 then loyal_hp = 4
			else loyal_hp = aplayer:getHp() end
			if aplayer:getMaxHP() == 3 then loyal_value = loyal_value + 0.5 end
			loyal_value = loyal_value + (loyal_hp + math.max(sgs.getDefense(aplayer) - loyal_hp * 2, 0) * 0.7)
			if aplayer:getWeapon() and aplayer:getWeapon():className() ~= "Weapon" then
				loyal_value = loyal_value + math.min(1.5, math.min(sgs.weapon_range[aplayer:getWeapon():className()],room:alivePlayerCount()/2)/2) * 0.4
			end
			global_room:writeToConsole("loyalnum is " .. loyal .. " value is " .. loyal_value)
		end
		end
	end
	local diff = loyal_value - rebel_value
	global_room:writeToConsole(diff)
	for _, aplayer in sgs.qlist(room:getAlivePlayers()) do
		if aplayer:isLord() then
		local lord_hp
		if aplayer:hasSkill("benghuai") and aplayer:getHp() > 4 then lord_hp = 4 
		else lord_hp = aplayer:getHp() end
		if lord_hp > 2 or (lord_hp == 2 and sgs.getDefense(aplayer) > 3) then health = true end
		end
	end
	if diff >= 0.6 then
	global_room:writeToConsole("diff >= 0.6")
	if health then global_room:writeToConsole("loyalist")
	else global_room:writeToConsole("dilemma") end
	elseif diff >= 0.3 then
	global_room:writeToConsole("diff >= 0.3") 
	if health then global_room:writeToConsole("loyalish")
	else global_room:writeToConsole("rebelish") end
	elseif diff <= -0.6 then global_room:writeToConsole("diff <= -0.6") global_room:writeToConsole("rebel")
	elseif diff <= -0.3 then 
		global_room:writeToConsole("diff <= -0.3")
		if health then global_room:writeToConsole("rebelish")
		else global_room:writeToConsole("rebel") end
	elseif not health then global_room:writeToConsole("rebelish")
	else global_room:writeToConsole("neutral") end


end

function sgs.gameProcess(room)
	local rebel_num = sgs.current_mode_players["rebel"]
	local loyal_num = sgs.current_mode_players["loyalist"]
	if sgs.turncount < 2 then return "neutral"
	elseif rebel_num == 0 then return "loyalist"
	elseif loyal_num == 0 and rebel_num > 1 then return "rebel" end
	local loyal_value, rebel_value = 0, 0, 0
	local health = false
	local currentplayer = room:getCurrent()
	for _, aplayer in sgs.qlist(room:getAlivePlayers()) do
		--if not (aplayer:objectName() == currentplayer:objectName() and aplayer:getRole() == "renegade") then 
		if not sgs.isRolePredictable() and sgs.evaluatePlayerRole(aplayer) == "rebel" then
			local rebel_hp
			if aplayer:hasSkill("benghuai") and aplayer:getHp() > 4 then rebel_hp = 4
			else rebel_hp = aplayer:getHp() end
			if aplayer:getMaxHP() == 3 then rebel_value = rebel_value + 0.5 end
			rebel_value = rebel_value + rebel_hp + math.max(sgs.getDefense(aplayer) - rebel_hp * 2, 0) * 0.7
			if aplayer:getWeapon() and aplayer:getWeapon():className() ~= "Weapon" then
				rebel_value = rebel_value + math.min(1.5, math.min(sgs.weapon_range[aplayer:getWeapon():className()],room:alivePlayerCount()/2)/2) * 0.4
			end
		elseif not sgs.isRolePredictable() and sgs.evaluatePlayerRole(aplayer) == "loyalist" then
			local loyal_hp
			if aplayer:hasSkill("benghuai") and aplayer:getHp() > 4 then loyal_hp = 4
			else loyal_hp = aplayer:getHp() end
			if aplayer:getMaxHP() == 3 then loyal_value = loyal_value + 0.5 end
			loyal_value = loyal_value + (loyal_hp + math.max(sgs.getDefense(aplayer) - loyal_hp * 2, 0) * 0.7)
			if aplayer:getWeapon() and aplayer:getWeapon():className() ~= "Weapon" then
				loyal_value = loyal_value + math.min(1.5, math.min(sgs.weapon_range[aplayer:getWeapon():className()],room:alivePlayerCount()/2)/2) * 0.4
			end
		end
		--end	
	end
	local diff = loyal_value - rebel_value
	for _, aplayer in sgs.qlist(room:getAlivePlayers()) do
		if aplayer:isLord() then
		local lord_hp
		if aplayer:hasSkill("benghuai") and aplayer:getHp() > 4 then lord_hp = 4 
		else lord_hp = aplayer:getHp() end
		if lord_hp > 3 or (lord_hp >= 2 and sgs.getDefense(aplayer) > 3) then health = true end
		end
	end

	if diff >= 0.6 then
	if health then return "loyalist"
	else return "dilemma" end
	elseif diff >= 0.3 then 
	if health then return "loyalish"
	else return "rebelish" end
	elseif diff <= -0.6 then return "rebel"
	elseif diff <= -0.3 then 
		if health then return "rebelish"
		else return "rebel" end
	elseif not health then return "rebelish"
	else return "neutral" end
end

function SmartAI:objectiveLevel(player)
	if player:objectName() == self.player:objectName() then return -2 end

	local players = self.room:getOtherPlayers(self.player)
	players = sgs.QList2Table(players)

	if #players == 1 then return 5 end
	
	if sgs.isRolePredictable() then
		if self.lua_ai:isFriend(player) then return -2
		elseif self.lua_ai:isEnemy(player) then return 5
		elseif self.lua_ai:relationTo(player) == sgs.AI_Neutrality then
			if self.lua_ai:getEnemies():isEmpty() then return 4 else return 0 end
		else return 0 end
	end

	local rebel_num = sgs.current_mode_players["rebel"]
	local loyal_num = sgs.current_mode_players["loyalist"]
	local renegade_num = sgs.current_mode_players["renegade"]
	local process = sgs.gameProcess(self.room)

	if self.role == "renegade" then
		if self:isWeak() and #self.enemies > 1 then 
		if #self.friends < 2 then return 5
		else return -1 end
		elseif process == "neutral" then return 0
		elseif process:match("rebel") then
			if sgs.evaluatePlayerRole(player) == "rebel" then 
				if process == "rebel" then return 5 else return 3 end
			elseif sgs.evaluateRoleTrends(player) == "rebel" then return 3
			else return -1 end
		elseif process:match("dilemma") then
			if sgs.evaluatePlayerRole(player) == "rebel" then return 5
			elseif sgs.evaluateRoleTrends(player) == "rebel" then return 3
			elseif player:isLord() then return -2
			else return 5 end
		else
			if sgs.evaluatePlayerRole(player) == "rebel" then return -2
			elseif sgs.evaluateRoleTrends(player) == "rebel" then return -1
			else
				if player:isLord() then
					if rebel_num > 0 then
						if loyal_num > 0 then
						if self:isWeak(player) then return -1
						elseif process == "loyalist" then return 3
						else return 0 end
						else 
							if self:isWeak(player) then return -1
							else return 3.5 end
						end    			        
					else
						if self:isWeak(player) then return 3
						elseif process == "loyalish" then return 3
						else return 4 end
					end
				else
					if process == "loyalist" then return 5 else return 3 end
				end
			end
		end
	end
	
	if self.player:isLord() or self.role == "loyalist" then
		if player:isLord() then return -2 end
		if rebel_num == 0 then
			if sgs.evaluatePlayerRole(player) == "renegade" then
				if self.player:isLord() and self:isWeak(player) then return 3
				elseif self.role == "loyalist" then return 5 
				else return 5
				end
			else
				if self.player:isLord() then return 0
				elseif loyal_num == 1 then return 5 end
			end
		end
		if loyal_num == 0 then
		if rebel_num > 2 then
			if sgs.evaluatePlayerRole(player) == "renegade" then return -1 end
		elseif rebel_num > 1 then
			if sgs.evaluatePlayerRole(player) == "renegade" then return 0 end
		elseif sgs.evaluatePlayerRole(player) == "renegade" then return 4 end
		end

		if sgs.evaluatePlayerRole(player) == "rebel" then return 5
		elseif sgs.evaluateRoleTrends(player) == "rebel" then return 3.5
		elseif sgs.evaluatePlayerRole(player) == "loyalist" then return -2
		elseif sgs.evaluateRoleTrends(player) == "loyalist" then return -1
		elseif sgs.backwardEvaluation(player) == "rebel" then return 5
		elseif sgs.backwardEvaluation(player) == "loyalist" then return -2
		elseif process:match("rebel") then return -1
		elseif sgs.compareRoleEvaluation(player, "rebel", "loyalist") == "rebel" then return 3
		else return 0 end
	elseif self.role == "rebel" then
		if player:isLord() then return 5
		elseif sgs.evaluatePlayerRole(player) == "loyalist" then return 5
		elseif sgs.evaluateRoleTrends(player) == "loyalist" then return 3.5
		elseif sgs.evaluatePlayerRole(player) == "rebel" then return -2
		elseif sgs.evaluateRoleTrends(player) == "rebel" then return -1
		elseif sgs.backwardEvaluation(player) == "rebel" then return -2
		elseif sgs.backwardEvaluation(player) == "loyalist" then return 4
		elseif process:match("loyalist") then return -1
		elseif sgs.compareRoleEvaluation(player, "renegade", "loyalist") == "loyalist" then return 3
		else return 0 end
	end
end

function SmartAI:isFriend(other, another)
	if not other then self.room:writeToConsole(debug.traceback()) return end
	if another then return self:isFriend(other)==self:isFriend(another) end
	if sgs.isRolePredictable() and self.lua_ai:relationTo(other) ~= sgs.AI_Neutrality then return self.lua_ai:isFriend(other) end
	if self.player:objectName() == other:objectName() then return true end
	if self:objectiveLevel(other) < 0 then return true end
	return false
end

function SmartAI:isEnemy(other, another)
	if not other then self.room:writeToConsole(debug.traceback()) return end
	if another then return self:isFriend(other)~=self:isFriend(another) end
	if sgs.isRolePredictable() and self.lua_ai:relationTo(other) ~= sgs.AI_Neutrality then return self.lua_ai:isEnemy(other) end
	if self.player:objectName() == other:objectName() then return false end
	if self:objectiveLevel(other) > 0 then return true end
	return false
end

function SmartAI:getFriendsNoself(player)
	if self:isFriend(self.player, player) then
		return self.friends_noself
	elseif self:isEnemy(self.player, player) then
		friends = sgs.QList2Table(self.lua_ai:getEnemies())
		for i = #friends, 1, -1 do
			if friends[i]:objectName() == player:objectName() then
				table.remove(friends, i)
			end
		end
		return friends
	else
		return {}
	end
end

function SmartAI:getFriends(player)
	if self:isFriend(self.player, player) then
		return self.friends
	elseif self:isEnemy(self.player, player) then
		return self.enemies
	else
		return {player}
	end
end

function SmartAI:getEnemies(player)
	if self:isFriend(self.player, player) then
		return self.enemies
	elseif self:isEnemy(self.player, player) then
		return self.friends
	else
		return {}
	end
end

function SmartAI:sortEnemies(players)
	local comp_func = function(a,b)
		local alevel = self:objectiveLevel(a)
		local blevel = self:objectiveLevel(b)

		if alevel~= blevel then return alevel > blevel end
		if alevel == 3 then return sgs.getDefense(a) >sgs.getDefense(b) end

		alevel = sgs.ai_chaofeng[a:getGeneralName()] or 0
		blevel = sgs.ai_chaofeng[b:getGeneralName()] or 0
		if alevel~= blevel then
			return alevel > blevel
		end

		alevel = sgs.getDefense(a)
		blevel = sgs.getDefense(b)
		if alevel~= blevel then
			return alevel < blevel
		end
	end
	table.sort(players,comp_func)
end

function SmartAI:updateAlivePlayerRoles()
	for _, arole in ipairs({"loyalist", "rebel", "renegade"}) do
		sgs.current_mode_players[arole] = 0
	end
	for _, aplayer in sgs.qlist(self.room:getOtherPlayers(self.room:getLord())) do
		sgs.current_mode_players[aplayer:getRole()] = sgs.current_mode_players[aplayer:getRole()] + 1
	end
	
	sgs.checkMisjudge()
end

function SmartAI:updateRoleTargets()
	for _, p in sgs.qlist(self.room:getAllPlayers()) do
		self:updateTarget(p)
	end
end

function SmartAI:updatePlayers()
	for _, aflag in ipairs(sgs.ai_global_flags) do
		sgs[aflag] = nil
	end

	sgs.discard_pile = global_room:getDiscardPile()
	sgs.draw_pile = global_room:getDrawPile()
	
	if sgs.isRolePredictable() then
		self.friends = sgs.QList2Table(self.lua_ai:getFriends())
		table.insert(self.friends, self.player)
		self.friends_noself = sgs.QList2Table(self.lua_ai:getFriends())
		self.enemies = sgs.QList2Table(self.lua_ai:getEnemies())
		
		self.retain = 2
		self.harsh_retain = false
		if #self.enemies == 0 then
			local neutrality = {}
			for _, aplayer in sgs.qlist(self.room:getOtherPlayers(self.player)) do
				if self.lua_ai:relationTo(aplayer) == sgs.AI_Neutrality then table.insert(neutrality, aplayer) end
			end
			local function compare_func(a,b)
				return self:objectiveLevel(a) > self:objectiveLevel(b)
			end
			table.sort(neutrality, compare_func)
			table.insert(self.enemies, neutrality[1])
			return
		end
	end

	local flist = {}
	local elist = {}
	self.enemies = elist
	self.friends = flist
	local lord = self.room:getLord()
	local role = self.role
	self.retain = 2
	self.harsh_retain = true

	local players = self.room:getOtherPlayers(self.player)
	players = sgs.QList2Table(players)

	for _,player in ipairs(players) do
		if #players == 1 then break end
		if self:objectiveLevel(player) < 0 then table.insert(flist,player) end
	end

	self.friends_noself = {}
	for _, player in ipairs(flist) do
		table.insert(self.friends_noself, player)
	end
	table.insert(self.friends,self.player)

	if self.role == "rebel" then self.retain = 2 end

	if self.player:getHp() < 2 then self.retain = 0 end
	self:sortEnemies(players)
	for _,player in ipairs(players) do
		if self:objectiveLevel(player) >= 4 then self.harsh_retain = false end
		if #elist == 0 then
			if self:objectiveLevel(player) < 4 then self.retain = 0 end
		else
			if self:objectiveLevel(player) <= 0 then return end
		end
		table.insert(elist,player)
	end
end

sgs.ai_choicemade_filter.Nullification.general = function(player, promptlist)
	if player:objectName() == promptlist[3] then return end
	local positive = true
	if promptlist[4] == "false" then positive = false end
	local className, to = promptlist[2]
	sgs.lastclass = className
	for _, aplayer in sgs.qlist(player:getRoom():getOtherPlayers(player)) do
		if aplayer:objectName() == promptlist[3] then to = aplayer break end
	end
	local intention = 0
	if type(sgs.ai_card_intention[className]) == "number" then intention = sgs.ai_card_intention[className]
	elseif sgs.dynamic_value.damage_card[className] then intention = 70
	elseif sgs.dynamic_value.benefit[className] then intention = -40
	elseif (className == "Snatch" or className == "Dismantlement") and
		(to:getCards("j"):isEmpty() and
		not (to:getArmor() and (to:getArmor():inherits("GaleShell") or to:getArmor():inherits("SilverLion")))) then
		intention = 80
	end
	if positive then intention = -intention end
	sgs.updateIntention(player, to, intention)
end

sgs.ai_choicemade_filter.playerChosen.general = function(from, promptlist)
	if from:objectName() == promptlist[3] then return end
	local reason = string.gsub(promptlist[2], "%-", "_")
	local to
	for _, p in sgs.qlist(from:getRoom():getAlivePlayers()) do
		if p:objectName() == promptlist[3] then to = p break end
	end
	local callback = sgs.ai_playerchosen_intention[reason]
	if callback then
		if type(callback) == "number" then
			sgs.updateIntention(from, to, sgs.ai_playerchosen_intention[reason])
		elseif type(callback) == "function" then
			callback(from, to)
		end
	end
end

function SmartAI:filterEvent(event, player, data)
	if not sgs.recorder then
		sgs.recorder = self
	end
	sgs.lastevent = event
	sgs.lasteventdata = eventdata
	if event == sgs.ChoiceMade and self == sgs.recorder then
		local carduse = data:toCardUse()
		if carduse and carduse:isValid() then
			for _, aflag in ipairs(sgs.ai_global_flags) do
				sgs[aflag] = nil
			end
			for _, callback in ipairs(sgs.ai_choicemade_filter.cardUsed) do
				if callback and type(callback) == "function" then
					callback(player, carduse)
				end
			end
		elseif data:toString() then
			promptlist = data:toString():split(":")
			local callbacktable = sgs.ai_choicemade_filter[promptlist[1]]
			if callbacktable and type(callbacktable) == "table" then
				local index = 2
				if promptlist[1] == "cardResponsed" then index = 3 end
				local callback = callbacktable[promptlist[index]] or callbacktable.general
				if callback and type(callback) == "function" then
					callback(player, promptlist)
				end
			end
		end
	elseif event == sgs.CardUsed or event == sgs.CardEffect or event == sgs.GameStart or event == sgs.Death or event == sgs.PhaseChange then
		self:updatePlayers()
	end
	
	if event == sgs.Death then
		if self == sgs.recorder then self:updateAlivePlayerRoles() end
	end
	if event == sgs.PhaseChange then
		if self.room:getCurrent():getPhase() == sgs.Player_NotActive then sgs.modifiedRoleEvaluation() end
	end

	if self ~= sgs.recorder then return end
	
	if event == sgs.CardEffect then
		local struct = data:toCardEffect()
		local card = struct.card
		local from = struct.from
		local to = struct.to
		if card:inherits("Collateral") then sgs.ai_collateral = true end
		if card:inherits("Dismantlement") or card:inherits("Snatch") or card:getSkillName() == "qixi" or card:getSkillName() == "jixi" then
			sgs.ai_snat_disma_effect = true
			sgs.ai_snat_dism_from = struct.from
			if to:getCards("j"):isEmpty() and
				not (to:getArmor() and (to:getArmor():inherits("GaleShell") or to:getArmor():inherits("SilverLion"))) then
				sgs.updateIntention(from, to, 80)
			end
		end
		if card:inherits("Slash") and to:hasSkill("leiji") and 
			(getCardsNum("Jink", to)>0 or (to:getArmor() and to:getArmor():objectName() == "eight_diagram"))
			and (to:getHandcardNum()>2 or from:getState() == "robot") then
			sgs.ai_leiji_effect = true
		end
	elseif event == sgs.Damaged then
		local damage = data:toDamage()
		local card = damage.card
		local from = damage.from
		local to   = damage.to
		local source = self.room:getCurrent()
		
		if not damage.card then
			local intention
			if sgs.ai_quhu_effect then
				sgs.quhu_effect = false
				local xunyu = self.room:findPlayerBySkillName("quhu")
				intention = 80
				from = xunyu
			else
				intention = 100 
			end
			
			if from then sgs.updateIntention(from, to, intention) end
		end
	elseif event == sgs.CardUsed then
		local struct = data:toCardUse()
		local card = struct.card
		local to = struct.to
		to = sgs.QList2Table(to)
		local from  = struct.from
		local source =  self.room:getCurrent()
		local str
		str = card:className() .. card:toString() .. ":"
		local toname = {}
		for _, ato in ipairs(to) do
			table.insert(toname, ato:getGeneralName())
		end
		if from then str = str .. from:getGeneralName() .. "->" .. table.concat(toname, "+") end
		if source then str = str .. "#" .. source:getGeneralName() end
		sgs.laststr = str
		--self.room:writeToConsole(str)

		local callback = sgs.ai_card_intention[card:className()]
		if callback then
			if type(callback) == "function" then
				callback(card, from, to)
			elseif type(callback) == "number" then
				sgs.updateIntentions(from, to, callback, card)
			end
		end
	elseif event == sgs.CardLost then
		local move = data:toCardMove()
		local from = move.from
		local place = move.from_place
		local card = sgs.Sanguosha:getCard(move.card_id)
		if sgs.ai_snat_disma_effect then
			sgs.ai_snat_disma_effect = false
			local intention = 70
			if place == sgs.Player_Judging then
				if not card:inherits("Disaster") then intention = -intention else intention = 0 end
			elseif place == sgs.Player_Equip then
				if player:getLostHp() > 1 and card:inherits("SilverLion") then intention = -intention end
				if self:hasSkills(sgs.lose_equip_skill, player) or card:inherits("GaleShell") then intention = 0 end
			end
			sgs.updateIntention(sgs.ai_snat_dism_from, from, intention)
		end
	elseif event == sgs.StartJudge then
		local judge = data:toJudge()
		local reason = judge.reason
		if reason == "beige" then
			local caiwenji = self.room:findPlayerBySkillName("beige")
			local intention = -60
			if player:objectName() == caiwenji:objectName() then intention = 0 end
			sgs.ai_card_intention.general(caiwenji, player, intention)
		end
	elseif event == sgs.PhaseChange and player:isLord() and player:getPhase()== sgs.Player_Finish then
		sgs.turncount = sgs.turncount + 1
		--self.room:writeToConsole(self.player:objectName() .. " " .. sgs.turncount)
	elseif event == sgs.GameStart then
		sgs.turncount = 0
	end
end

function SmartAI:askForSuit(reason)
	if not reason then return sgs.ai_skill_suit.fanjian() end -- this line is kept for back-compatibility
	local callback = sgs.ai_skill_suit[reason]
	if callback and type(callback) == "function" then
		if callback() then return callback(self) end
	end
	return math.random(0,3)
end

function SmartAI:askForSkillInvoke(skill_name, data)
	skill_name = string.gsub(skill_name, "%-", "_")
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

function SmartAI:askForChoice(skill_name, choices)
	local choice = sgs.ai_skill_choice[skill_name]
	if type(choice) == "string" then
		return choice
	elseif type(choice) == "function" then
		return choice(self, choices)
	else
		local skill = sgs.Sanguosha:getSkill(skill_name)
		if skill and choices:match(skill:getDefaultChoice(self.player)) then
			return skill:getDefaultChoice(self.player)
		else
			local choice_table = choices:split("+");
			for index, achoice in ipairs(choice_table) do
				if achoice == "benghuai" then table.remove(choice_table, index) break end
			end
			local r = math.random(1, #choice_table)
			return choice_table[r]
		end
	end
end

function SmartAI:askForDiscard(reason, discard_num, optional, include_equip)
	local callback = sgs.ai_skill_discard[reason]
	if callback and type(callback) == "function" then
		if callback(self, discard_num, optional, include_equip) then return callback(self, discard_num, optional, include_equip) end
	elseif optional then return {} end

	local flag = "h"
	if include_equip and (self.player:getEquips():isEmpty() or not self.player:isJilei(self.player:getEquips():first())) then flag = flag .. "e" end
	local cards = self.player:getCards(flag)
	local to_discard = {}
	cards = sgs.QList2Table(cards)
	local aux_func = function(card)
		local place = self.room:getCardPlace(card:getEffectiveId())
		if place == sgs.Player_Equip then
			if card:inherits("GaleShell") then return -2
			elseif card:inherits("SilverLion") and self.player:isWounded() then return -2
			elseif card:inherits("YitianSword") then return -1
			elseif card:inherits("OffensiveHorse") then return 1
			elseif card:inherits("Weapon") then return 2
			elseif card:inherits("DefensiveHorse") then return 3
			elseif card:inherits("Armor") then return 4 end
		elseif self:hasSkills(sgs.lose_equip_skill) then return 5
		else return 0 end
	end
	local compare_func = function(a, b)
		if aux_func(a) ~= aux_func(b) then return aux_func(a) < aux_func(b) end
		return self:getKeepValue(a) < self:getKeepValue(b)
	end

	table.sort(cards, compare_func)
	for _, card in ipairs(cards) do
		if #to_discard >= discard_num then break end
		if not self.player:isJilei(card) then table.insert(to_discard, card:getId()) end
	end
	
	return to_discard
end

function SmartAI:askForNullification(trick, from, to, positive)
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local null_card
	null_card = self:getCardId("Nullification")
	if null_card then null_card = sgs.Card_Parse(null_card) else return end

	if positive then
		if from and self:isEnemy(from) and (sgs.evaluateRoleTrends(from) ~= "neutral" or sgs.isRolePredictable()) then
			if trick:inherits("ExNihilo") and (self:isWeak(from) or self:hasSkills(sgs.cardneed_skill,from)) then return null_card end 
			if trick:inherits("IronChain") and not self:isEquip("Vine", to) then return nil end
			if self:isFriend(to) then
				if trick:inherits("Dismantlement") then 
					if self:getDangerCard(to) or self:getValuableCard(to) or (to:getHandcardNum() == 1 and not self:needKongcheng(to)) then return null_card end
				else
					if trick:inherits("Snatch") then return null_card end
					if self:isWeak(to) then
						if trick:inherits("Duel") then
							return null_card
						elseif trick:inherits("FireAttack") then
							if from:getHandcardNum() > 2 and not (from == to) then return null_card end
						end
					end
				end
			elseif self:isEnemy(to) then
				if (trick:inherits("Snatch") or trick:inherits("Dismantlement")) and to:getCards("j"):length() > 0 then
					return null_card
				end
			end
		end

		if self:isFriend(to) then
		    if not (to:hasSkill("guanxing") and global_room:alivePlayerCount() > 4) then 
				if (trick:inherits("Indulgence") and not to:hasSkill("tuxi")) or (trick:inherits("SupplyShortage") and not self:hasSkills("guidao|tiandu",to)) then
					return null_card
				end
			end
			if self:isWeak(to) then
				if trick:inherits("ArcheryAttack") then
					if self:getCardsNum("Jink", to) == 0 then return null_card end
				elseif trick:inherits("SavageAssault") then
					if self:getCardsNum("Slash", to) == 0 then return null_card end
				end
			end
		end
		if from then
			if self:isEnemy(to) then
				if trick:inherits("GodSalvation") and self:isWeak(to) then
					return null_card
				end
			end
		end
	else
		if from then
			if from:objectName() == to:objectName() then
				if self:isFriend(from) then return null_card else return end
			end
			if not (trick:inherits("AmazingGrace") or trick:inherits("GodSalvation") or trick:inherits("AOE")) then
				if self:isFriend(from) then
					if ("snatch|dismantlement"):match(trick:objectName()) and to:isNude() then
					elseif trick:inherits("FireAttack") and to:isKongcheng() then
					else return null_card end
				end
			end
		else
			if self:isEnemy(to) and (sgs.evaluateRoleTrends(to) ~= "neutral" or sgs.isRolePredictable()) then return null_card else return end
		end
	end
end

function SmartAI:getCardRandomly(who, flags)
	local cards = who:getCards(flags)
	if cards:isEmpty() then return end
	local r = math.random(0, cards:length()-1)
	local card = cards:at(r)
	if self:isEquip("SilverLion", who) then
		if self:isEnemy(who) and who:isWounded() and card == who:getArmor() then
			if r ~= (cards:length()-1) then
				card = cards:at(r+1)
			else
				card = cards:at(r-1)
			end
		end
	end
	if self:isEquip("GaleShell", who) then
		if self:isEnemy(who) and card == who:getArmor() then
			if r ~= (cards:length()-1) then
				card = cards:at(r+1)
			else
				card = cards:at(r-1)
			end
		end
	end
	return card:getEffectiveId()
end

function SmartAI:askForCardChosen(who, flags, reason)
	self.room:output(reason)
	local cardchosen = sgs.ai_skill_cardchosen[string.gsub(reason,"%-","_")]
	local card
	if type(cardchosen) == "function" then
		card = cardchosen(self, who, flags)
		if card then return card:getEffectiveId() end
	elseif type(cardchosen) == "number" then return cardchosen end

	if self:isFriend(who) then
		if flags:match("j") then
			local tricks = who:getCards("j")
			local lightning, indulgence, supply_shortage
			for _, trick in sgs.qlist(tricks) do
				if trick:inherits("Lightning") then
					lightning = trick:getId()
				elseif trick:inherits("Indulgence") or trick:getSuit() == sgs.Card_Diamond then
					indulgence = trick:getId()
				elseif not trick:inherits("Disaster") then
					supply_shortage = trick:getId()
				end
			end


			if self:hasWizard(self.enemies) and lightning then
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
		end

		if flags:match("e") then
			local zhangjiao = self.room:findPlayerBySkillName("leiji")
			if who:isWounded() and self:isEquip("SilverLion", who) and (not zhangjiao or self:isFriend(zhangjiao))
				and not self:hasSkills("qixi|duanliang", who) then return who:getArmor():getId() end
			if self:evaluateArmor(who:getArmor(), who)<-5 then return who:getArmor():getId() end
			if self:hasSkills(sgs.lose_equip_skill, who) then
				local equips = who:getEquips()
				if not equips:isEmpty() then
					return equips:at(0):getId()
				end
			end
		end
	else
		if flags:match("e") then
			if who:getWeapon() and who:getWeapon():inherits("Crossbow") then
				for _, friend in ipairs(self.friends) do
					if who:distanceTo(friend) <= 1 then return who:getWeapon():getId() end
				end
			end

			self:sort(self.friends, "hp")
			local friend = self.friends[1]
			if self:isWeak(friend) and who:inMyAttackRange(friend) then
				if who:getWeapon() and who:distanceTo(friend) > 1 then return who:getWeapon():getId() end
				if who:getOffensiveHorse() and who:distanceTo(friend) > 1 then return who:getOffensiveHorse():getId() end
			end

			if who:getDefensiveHorse() then
				for _,friend in ipairs(self.friends) do
					if friend:distanceTo(who) == friend:getAttackRange()+1 then
						return who:getDefensiveHorse():getId()
					end
				end
			end

			if who:getArmor() and self:evaluateArmor(who:getArmor(),who)>3 then
				return who:getArmor():getId()
			end

			if self:isEquip("Monkey", who) then
				return who:getOffensiveHorse():getId()
			end
		end

		if flags:match("j") then
			local tricks = who:getCards("j")
			local lightning
			for _, trick in sgs.qlist(tricks) do
				if trick:inherits("Lightning") then
					lightning = trick:getId()
				end
			end
			if self:hasWizard(self.enemies,true) and lightning then
				return lightning
			end
		end

		if flags:match("e") then
			if who:getArmor() and self:evaluateArmor(who:getArmor(), who)>0
				and not (who:getArmor():inherits("SilverLion") and self:isWeak(who)) then
				return who:getArmor():getId()
			end

			if who:getWeapon() then
				if not (who:hasSkill("xiaoji") and (who:getHandcardNum() >= who:getHp())) and not self:isEquip("YitianSword",who) then
					for _,friend in ipairs(self.friends) do
						if (who:distanceTo(friend) <= who:getAttackRange()) and (who:distanceTo(friend) > 1) then
							return who:getWeapon():getId()
						end
					end
				end
			end

			if who:getOffensiveHorse() then
				if who:hasSkill("xiaoji") and who:getHandcardNum() >= who:getHp() then
				else
					for _,friend in ipairs(self.friends) do
						if who:distanceTo(friend) == who:getAttackRange() and
						who:getAttackRange() > 1 then
							return who:getOffensiveHorse():getId()
						end
					end
				end
			end
		end
		if flags:match("h") then
			if not who:isKongcheng() then
				return -1
			end
		end
	end
	local new_flag = ""
	if flags:match("h") then new_flag = "h" end
	if flags:match("e") then new_flag = new_flag.."e" end
	return self:getCardRandomly(who, new_flag) or who:getCards(flags):first():getEffectiveId()
end

function sgs.ai_skill_cardask.nullfilter(self, data, pattern, target)
	if not self:damageIsEffective(nil, nil, target) then return "." end
	if self:getDamagedEffects(self) then return "." end
	if target and target:getWeapon() and target:getWeapon():inherits("IceSword") and self.player:getCards("he"):length() > 2 then return end
	if self:needBear() then return "." end
	if self.player:hasSkill("tianxiang") then
		local dmgStr = {damage = 1, nature = 0}
		local willTianxiang = sgs.ai_skill_use["@tianxiang"](self, dmgStr)
		if willTianxiang ~= "." then return "." end
	elseif self.player:hasSkill("longhun") and self.player:getHp() > 1 then
		return "."
	end
	if (self.player:hasSkill("yiji")) and self.player:getHp() > 2 then return "." end
	if target and target:hasSkill("guagu") and self.player:isLord() then return "." end
	if self.player:hasSkill("jieming") and self:getJiemingChaofeng() <= -6 and self.player:getHp() >= 2 then return "." end
	local sunshangxiang = self.room:findPlayerBySkillName("jieyin")
	if sunshangxiang and sunshangxiang:isWounded() and self:isFriend(sunshangxiang) and not self.player:isWounded() 
		and self.player:getGeneral():isMale() then
		self:sort(self.friends, "hp")
		for _, friend in ipairs(self.friends) do
			if friend:getGeneral():isMale() and friend:isWounded() then return end
		end
		return "."
	end
end

function SmartAI:askForCard(pattern, prompt, data)
	self.room:output(prompt)
	local target, target2
	local parsedPrompt = prompt:split(":")
	if parsedPrompt[2] then
		local others = self.room:getOtherPlayers(self.player)
		others = sgs.QList2Table(others)
		for _, other in ipairs(others) do
			if other:getGeneralName() == parsedPrompt[2] or other:objectName() == parsedPrompt[2] then target = other break end
		end
		if parsedPrompt[3] then
			for _, other in ipairs(others) do
				if other:getGeneralName() == parsedPrompt[3] or other:objectName() == parsedPrompt[3] then target2 = other break end
			end
		end
	end
	if self.player:hasSkill("hongyan") then
		local card
		if (pattern == ".S" or pattern == "..S") then return "."
		elseif pattern == "..H" then card = self.lua_ai:askForCard(".|spade,heart", prompt, data)
		elseif pattern == ".H" then card = self.lua_ai:askForCard(".|spade,heart|.|hand", prompt, data) end
		if card then return card:toString() end
	end
	local callback = sgs.ai_skill_cardask[parsedPrompt[1]]
	if callback and type(callback) == "function" then
		local ret = callback(self, data, pattern, target, target2)
		if ret then return ret end
	end
	
	if pattern == "slash" then
		return sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) or self:getCardId("Slash") or "."
	elseif pattern == "jink" then
		return sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) or self:getCardId("Jink") or "."
	end
end

function SmartAI:askForUseCard(pattern, prompt)
	local use_func = sgs.ai_skill_use[pattern]
	if use_func then
		return use_func(self, prompt) or "."
	else
		return "."
	end
end

function SmartAI:askForAG(card_ids, refusable, reason)
	local cardchosen = sgs.ai_skill_askforag[string.gsub(reason, "%-", "_")]
	if type(cardchosen) == "function" then
		local card_id = cardchosen(self, card_ids)
		if card_id then return card_id end
	end

	if refusable and self:hasSkill("xinzhan") then
		local next_player = self.player:getNextAlive()
		if self:isFriend(next_player) and next_player:containsTrick("indulgence") then
			if #card_ids == 1 then return -1 end
		end
		for _, card_id in ipairs(card_ids) do
			if not sgs.Sanguosha:getCard(card_id):inherits("Shit") then return card_id end
		end
		return -1
	end
	local ids = card_ids
	local cards = {}
	for _, id in ipairs(ids) do
		table.insert(cards, sgs.Sanguosha:getCard(id))
	end
	self:sortByCardNeed(cards)
	return cards[#cards]:getEffectiveId()
end

function SmartAI:askForCardShow(requestor, reason)
	local func = sgs.ai_cardshow[reason]
	if func then
		return func(self, requestor)
	else
		return self.player:getRandomHandCard()
	end
end

function sgs.ai_cardneed.bignumber(to, card, self)
	if not to:containsTrick("indulgence") and self:getUseValue(card) < 6 then
		return card:getNumber() > 10
	end
end

function sgs.ai_cardneed.equip(to, card, self)
	if not to:containsTrick("indulgence") then
		return card:getTypeId() == sgs.Card_Equip
	end
end

function SmartAI:needKongcheng(player)
	return (player:isKongcheng() and (player:hasSkill("kongcheng") or (player:hasSkill("zhiji") and not player:hasSkill("guanxing")))) or
			(not self:isWeak(player) and self:hasSkills(sgs.need_kongcheng,player))
end

function SmartAI:getCardNeedPlayer(cards)
	cards = cards or sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards,true)
	local name = self.player:objectName()
	if #self.friends > 1 then
		for _, hcard in ipairs(cards) do
			if not hcard:inherits("Shit") then
				if hcard:inherits("Analeptic") or hcard:inherits("Peach") then
					self:sort(self.friends_noself, "hp")
					if #self.friends>1 and self.friends_noself[1]:getHp() == 1 then
						return hcard, self.friends_noself[1]
					end
				end
				self:sort(self.friends_noself, "handcard")
				for _, friend in ipairs(self.friends_noself) do
					if not self:needKongcheng(friend) then
						for _, askill in sgs.qlist(friend:getVisibleSkillList()) do
							local callback = sgs.ai_cardneed[askill:objectName()]
							if callback and type(callback)=="function" and callback(friend, hcard, self) then
								return hcard, friend
							end
						end
					end
				end
				if self:getUseValue(hcard)<6 then
					for _, friend in ipairs(self.friends_noself) do
						if not self:needKongcheng(friend) then
							if sgs[friend:getGeneralName() .. "_suit_value"] and
								(sgs[friend:getGeneralName() .. "_suit_value"][hcard:getSuitString()] or 0)>=3.9 then
								return hcard, friend
							end
							if friend:getGeneral2Name()~="" then
								if sgs[friend:getGeneral2Name() .. "_suit_value"] and
									(sgs[friend:getGeneral2Name() .. "_suit_value"][hcard:getSuitString()] or 0)>=3.9 then
									return hcard, friend
								end
							end
						end
					end
				end
				local dummy_use = {isDummy = true}
				self:useSkillCard(sgs.Card_Parse("@ZhibaCard=."), dummy_use)
				if dummy_use.card then
					local subcard = sgs.Sanguosha:getCard(dummy_use.card:getEffectiveId())
					if self:getUseValue(subcard) < 6 and #self.friends > 1 then
						for _, player in ipairs(self.friends_noself) do
							if player:getKingdom() == "wu" and not self:needKongcheng(player) then
								return subcard, player
							end
						end
					end
				end
				if hcard:inherits("Armor") then
					self:sort(self.friends_noself, "defense")
					local v = 0
					local target
					for _, friend in ipairs(self.friends_noself) do
						if not friend:getArmor() and self:evaluateArmor(hcard, friend) > v and not friend:containsTrick("indulgence") 
							and not self:needKongcheng(friend) then
							v = self:evaluateArmor(hcard, friend)
							target = friend
						end
					end
					if target then
						return hcard, target
					end
				end
				if hcard:inherits("EquipCard") then
					self:sort(self.friends_noself)
					for _, friend in ipairs(self.friends_noself) do
						if (not self:hasSameEquip(hcard, friend)
							or (self:hasSkills(sgs.lose_equip_skill, friend) and not friend:containsTrick("indulgence"))) and
							not self:needKongcheng(friend) then
							return hcard, friend
						end
					end
				end
			end
		end
	end
	local shit
	shit = self:getCard("Shit")
	self:sort(self.enemies, "hp")
	if shit and #self.enemies  > 0 then
		for _,enemy in ipairs(self.enemies) do
			local v1 = 0
			if sgs[enemy:getGeneralName().."_suit_value"] then
				v1 = sgs[enemy:getGeneralName().."_suit_value"][shit:getSuitString()] or 0
			end
			if v1 <= 0 then
				return shit, enemy
			end
		end
	end
	local zhugeliang = self.room:findPlayerBySkillName("kongcheng")
	if zhugeliang and zhugeliang:objectName() ~= self.player:objectName() and self:isEnemy(zhugeliang) and zhugeliang:isKongcheng() then
		local shit = self:getCard("Shit") or self:getCard("Disaster") or self:getCard("GodSalvation") or self:getCard("AmazingGrace")
		if shit then
			return shit, zhugeliang
		end
		for _, card in ipairs(self:getCards("EquipCard")) do
			if self:hasSameEquip(card, zhugeliang) or (card:inherits("OffensiveHorse") and not card:inherits("Monkey")) then
				return card, zhugeliang
			end
		end
		if zhugeliang:getHp() < 2 then
			local slash = self:getCard("Slash")
			if slash then
				return slash, zhugeliang
			end
		end
	end
end

function SmartAI:askForYiji(card_ids)
	if self.player:getHandcardNum() <= 2 then
		return nil, -1
	end

	local cards = {}
	for _, card_id in ipairs(card_ids) do
		table.insert(cards, sgs.Sanguosha:getCard(card_id))
	end
	
	local card, friend = self:getCardNeedPlayer(cards)
	if card and friend then return friend, card:getId() end
	if #self.friends > 1 and self:getOverflow() > 0 then
		self:sort(self.friends_noself, "handcard")
		for _, afriend in ipairs(self.friends_noself) do
			if not self:needKongcheng(afriend) then
				for _, acard_id in ipairs(card_ids) do
					if not sgs.Sanguosha:getCard(acard_id):inherits("Shit") then return afriend, acard_id end
				end
			end
		end
	end
end

function SmartAI:askForPindian(requestor, reason)
	local cards = sgs.QList2Table(self.player:getHandcards())
	local compare_func = function(a, b)
		return a:getNumber() < b:getNumber()
	end
	table.sort(cards, compare_func)
	local maxcard, mincard, minusecard
	for _, card in ipairs(cards) do
		if self:getUseValue(card) < 6 then mincard = card break end
	end
	for _, card in ipairs(sgs.reverse(cards)) do
		if self:getUseValue(card) < 6 then maxcard = card break end
	end
	self:sortByUseValue(cards, true)
	minusecard = cards[1]
	maxcard = maxcard or minusecard
	mincard = mincard or minusecard
	local callback = sgs.ai_skill_pindian[reason]
	if callback and type(callback) == "function" then
		local ret = callback(minusecard, self, requestor, maxcard, mincard)
		if ret then return ret end
	end
	if self:isFriend(requestor) then return mincard else return maxcard end
end

sgs.ai_skill_playerchosen.damage = function(self, targets)
	local targetlist=sgs.QList2Table(targets)
	self:sort(targetlist,"hp")
	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) then return target end
	end
	return targetlist[#targetlist]
end

function SmartAI:askForPlayerChosen(targets, reason)
	self:log("askForPlayerChosen:"..reason)
	local playerchosen = sgs.ai_skill_playerchosen[string.gsub(reason,"%-","_")]
	local target
	if type(playerchosen) == "function" then
		target = playerchosen(self,targets)
	end
	if target then
		return target
	else
		local r = math.random(0, targets:length() - 1)
		return targets:at(r)
	end
end

function SmartAI:askForSinglePeach(dying)
	local card_str

	if self:isFriend(dying) then
		local buqu = dying:getPile("buqu")
		local weaklord = 0
		if not buqu:isEmpty() then
			local same = false
			for i, card_id in sgs.qlist(buqu) do
				for j, card_id2 in sgs.qlist(buqu) do
					if i ~= j and sgs.Sanguosha:getCard(card_id):getNumber() == sgs.Sanguosha:getCard(card_id2):getNumber() then
						same = true
						break
					end
				end
			end
			if not same then return "." end
		end
		if (self.player:objectName() == dying:objectName()) then
			card_str = self:getCardId("Analeptic") or self:getCardId("Peach")
		else
			for _, friend in ipairs(self:getFriends(player)) do
				if friend:getHp() == 1 and friend:isLord() and not friend:hasSkill("buqu") then weaklord =1 end
			end
			if weaklord ==0 or self:getAllPeachNum() > 1 then
			card_str = self:getCardId("Peach") 
			end
		end
	end

	return card_str or "."
end

function SmartAI:getTurnUse()
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)

	local turnUse = {}
	local slashAvail = 1
	self.predictedRange = self.player:getAttackRange()
	self.predictNewHorse = false
	self.retain_thresh = 5
	self.slash_targets = 1
	self.slash_distance_limit = false

	self.weaponUsed = false

	if self.player:isLord() then self.retain_thresh = 6 end
	if self.player:hasFlag("tianyi_success") then
		slashAvail = 2
		self.slash_targets = 2
		self.slash_distance_limit = true
	end

	self:fillSkillCards(cards)

	self:sortByUseValue(cards)

	if self:isEquip("Crossbow") then
		slashAvail = 100
	end


	local i = 0
	for _,card in ipairs(cards) do
		local dummy_use = {}
		dummy_use.isDummy = true
		if not self:hasSkills(sgs.need_kongcheng) then
			if (i >= (self.player:getHandcardNum()-self.player:getHp()+self.retain)) and (self:getUseValue(card) < self.retain_thresh) then
				return turnUse
			end

			if (i >= (self.player:getHandcardNum()-self.player:getHp())) and (self:getUseValue(card) < 8.5) and self.harsh_retain then
				return turnUse
			end
		end

		local type = card:getTypeId()
		self["use" .. sgs.ai_type_name[type + 1] .. "Card"](self, card, dummy_use)

		if dummy_use.card then
			if (card:inherits("Slash")) then
				if slashAvail > 0 then
					slashAvail = slashAvail-1
					table.insert(turnUse,card)
				end
			else
				if card:inherits("Weapon") then
					self.predictedRange = sgs.weapon_range[card:className()]
					self.weaponUsed = true
				end
				if card:inherits("OffensiveHorse") then self.predictNewHorse = true end
				if card:objectName() == "crossbow" then slashAvail = 100 end
				if card:inherits("Snatch") then i = i-1 end
				if card:inherits("Peach") then i = i+2 end
				if card:inherits("Collateral") then i = i-1 end
				if card:inherits("AmazingGrace") then i = i-1 end
				if card:inherits("ExNihilo") then i = i-2 end
				table.insert(turnUse,card)
			end
			i = i+1
		end
	end

	return turnUse
end

function SmartAI:activate(use)
	self:updatePlayers()
	self:assignKeep(self.player:getHp(),true)
	self.toUse  = self:getTurnUse()
	self:sortByDynamicUsePriority(self.toUse)
	for _, card in ipairs(self.toUse) do
		if not self.player:isJilei(card) then
			local type = card:getTypeId()
			self["use" .. sgs.ai_type_name[type + 1] .. "Card"](self, card, use)

			if use:isValid() then
				self.toUse = nil
				return
			end
		end
	end
	self.toUse = nil
end

function SmartAI:getOverflow(player)
	player = player or self.player
	return math.max(player:getHandcardNum() - player:getHp(), 0)
end

function SmartAI:isWeak(player)
	player = player or self.player
	local hcard = player:getHandcardNum()
	if player:hasSkill("longhun") then hcard = player:getCards("he"):length() end
	return ((player:getHp() <= 2 and hcard <= 2) or player:getHp() <= 1) and not player:hasSkill("buqu")
end

function SmartAI:useCardByClassName(card, use)
	local class_name = card:className()
	local use_func = self["useCard" .. class_name]

	if use_func then
		use_func(self, card, use)
	end
end

function sgs.getSkillLists(player)
	local slist = player:getVisibleSkillList()
	local vsnlist = {}
	local fsnlist = {}
	for _, askill in sgs.qlist(player:getVisibleSkillList()) do
		if askill:inherits("ViewAsSkill") then table.insert(vsnlist, askill:objectName()) end
		if askill:inherits("FilterSkill") then table.insert(fsnlist, askill:objectName()) end
	end
	return vsnlist, fsnlist
end

function SmartAI:hasWizard(players,onlyharm)
	local skill
	if onlyharm then skill = sgs.wizard_harm_skill else skill = sgs.wizard_skill end
	for _, player in ipairs(players) do
		if self:hasSkills(skill, player) then
			return true
		end
	end
end

--- Determine that the current judge is worthy retrial
-- @param judge The JudgeStruct that contains the judge information
-- @return True if it is needed to retrial
function SmartAI:needRetrial(judge)  
	local reason = judge.reason
	if reason == "typhoon" or reason == "earthquake" or reason == "volcano" or reason == "mudslide" then return false end
	if reason == "lightning" then  
		if self:isFriend(judge.who) then
			if who:isChained() and self:isGoodChainTarget(judge.who) then
				return false   
			end
		else
		    if who:isChained() and not self:isGoodChainTarget(judge.who) then 
				return judge:isGood()
			end
		end
	end
	if self:isFriend(judge.who) then
		if not self.player:hasSkill("guidao") and judge.reason == "luoshen" and self:getOverflow(judge.who) > 1 and self.player:getHandcardNum() < 3
			and not self:isEquip("Crossbow", judge.who) then return false end
		return not judge:isGood()
	elseif self:isEnemy(judge.who) then
		return judge:isGood()
	else
		return false
	end
end

function SmartAI:canRetrial(player) 
    player = player or self.player
	if player:hasSkill("guidao") then
	    local blackequipnum = 0
		for _,equip in sgs.qlist(player:getEquips()) do
			if equip:isBlack() then blackequipnum = blackequipnum+1 end
		end
		return (blackequipnum+player:getHandcardNum()) > 0
	elseif player:hasSkill("guicai") then
	    return player:getHandcardNum() > 0
	elseif player:hasSkill("jilve") then
	    return player:getHandcardNum() > 0 and player:getMark("@bear") > 0
	end		
end

function SmartAI:hasEnemyZhangjiao(player)   
    player = player or self.player
	for _, enemy in ipairs(self:getEnemies(player)) do
	    if enemy:hasSkill("guidao") then
			return true
		end
	end
	return false
end

function SmartAI:getFinalRetrial(player) 
	local maxfriendseat = -1
	local maxenemyseat = -1
	local tmpfriend
	local tmpenemy
	for _, aplayer in ipairs(self:getFriends(player)) do
		if self:hasSkills(sgs.wizard_harm_skill, aplayer) and self:canRetrial(aplayer) then
		    tmpfriend = (aplayer:getSeat() - player:getSeat()) % (global_room:alivePlayerCount())
			if tmpfriend > maxfriendseat then maxfriendseat = tmpfriend end
		end
	end
	for _, aplayer in ipairs(self:getEnemies(player)) do
		if self:hasSkills(sgs.wizard_harm_skill, aplayer) and self:canRetrial(aplayer) then
		    tmpenemy = (aplayer:getSeat() - player:getSeat()) % (global_room:alivePlayerCount())
			if tmpenemy > maxenemyseat then maxenemyseat = tmpenemy end
		end
	end
	if maxfriendseat == -1 and maxenemyseat == -1 then return 0
	elseif maxfriendseat>maxenemyseat then return 1
	else return 2 end
end

function SmartAI:hasDangerFriend(player) 
	player = player or self.player
	local hashy = false
	for _, aplayer in ipairs(self.enemies) do
		if aplayer:hasSkill("hongyan") then hashy = true break end
	end
	for _, aplayer in ipairs(self.enemies) do
		if aplayer:hasSkill("guanxing") or (aplayer:hasSkill("gongxin") and hashy) 
		   or aplayer:hasSkill("xinzhan") then 
		   if self:isFriend(aplayer:getNextAlive()) then return true end
		end
	end
	return false
end


--- Get the retrial cards with the lowest keep value
-- @param cards the table that contains all cards can use in retrial skill
-- @param judge the JudgeStruct that contains the judge information
-- @return the retrial card id or -1 if not found
function SmartAI:getRetrialCardId(cards, judge)
	local can_use = {}
	for _, card in ipairs(cards) do
		if self:isFriend(judge.who) and judge:isGood(card) then
			table.insert(can_use, card)
		elseif self:isEnemy(judge.who) and not judge:isGood(card) then
			table.insert(can_use, card)
		end
	end

	if next(can_use) then
		self:sortByKeepValue(can_use, true)
		return can_use[1]:getEffectiveId()
	else
		return -1
	end
end

function SmartAI:damageIsEffective(player, nature, source)
	player = player or self.player
	source = source or self.player
	nature = nature or sgs.DamageStruct_Normal
	if player:hasSkill("zhichi") and self.room:getTag("Zhichi"):toString() == player:objectName() then
		return false
	end

	if player:hasSkill("shenjun") and player:getGender() ~= source:getGender() and nature ~= sgs.DamageStruct_Thunder then
		return false
	end

	if player:getMark("@fog") > 0 and nature ~= sgs.DamageStruct_Thunder then
		return false
	end
	return true
end

function SmartAI:getDamagedEffects(self, player)
	player = player or self.player
	
	if (player:getHp() > 1 or player:hasSkill("buqu")) and self:hasSkills(sgs.masochism_skill, player) then
		local attacker = self.room:getCurrent()
		if self:isEnemy(attacker, player) and attacker:getHp() <= 1 then
			if self:hasSkills("ganglie|enyuan", player) then return true end
		end
		
		if player:hasSkill("jieming") then
			for _, friend in ipairs(self:getFriends(player)) do
				if math.min(friend:getMaxHP(), 5) - friend:getHandcardNum() >= 3 then return true end
			end
		elseif player:hasSkill("fangzhu") then
			if player:getLostHp() <= 1 then return true end
		end
	end
	
	return false
end

local function prohibitUseDirectly(card, player)
	local _, flist = sgs.getSkillLists(player)
	for _, askill in ipairs(flist) do
		local callback = sgs.ai_filterskill_filter[askill]
		if callback and type(callback) == "function" and callback(card) then return true end
	end
	return false
end

local function zeroCardView(class_name, player)
	if class_name == "Analeptic" then
		if player:hasSkill("jiushi") and player:faceUp() then
			return ("analeptic:jiushi[no_suit:0]=.")
		end
	end
end

local function isCompulsoryView(card, class_name, player, card_place)
	local _, flist = sgs.getSkillLists(player)
	for _, askill in ipairs(flist) do
		local callback = sgs.ai_filterskill_filter[askill]
		if callback and type(callback) == "function" and callback(card, card_place) and sgs.Card_Parse(callback(card)):inherits(class_name) then
			return callback(card, card_place)
		end
	end
end

sgs.ai_view_as = {}

local function getSkillViewCard(card, class_name, player, card_place)
	local vlist = sgs.getSkillLists(player)
	for _, askill in ipairs(vlist) do
		local callback = sgs.ai_view_as[askill]
		if callback and type(callback) == "function" and callback(card, player, card_place, class_name)
			and sgs.Card_Parse(callback(card, player, card_place, class_name)):inherits(class_name) then
			return callback(card, player, card_place, class_name)
		end
	end
end

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

function SmartAI:getCardId(class_name, player)
	player = player or self.player
	local cards = player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByUsePriority(cards)
	local card_str = self:getGuhuoCard(class_name, player) or zeroCardView(class_name, player)
	if card_str then return card_str end

	for _, card in ipairs(cards) do
		local card_place = self.room:getCardPlace(card:getEffectiveId())
		if card:inherits(class_name) and not prohibitUseDirectly(card, player) then
			return card:getEffectiveId()
		elseif isCompulsoryView(card, class_name, player, card_place) then
			return isCompulsoryView(card, class_name, player, card_place)
		end
	end
	for _, card in ipairs(cards) do
		local card_place = self.room:getCardPlace(card:getEffectiveId())
		card_str = getSkillViewCard(card, class_name, player, card_place)
		if card_str then return card_str end
	end
end

function SmartAI:getCard(class_name, player)
	player = player or self.player
	local card_id = self:getCardId(class_name, player)
	if card_id then return sgs.Card_Parse(card_id) end
end

function getCards(class_name, player, room, flag)
	flag = flag or "he"
	local cards = {}
	local card_place, card_str
	if not room then card_place = sgs.Player_Hand end

	for _, card in sgs.qlist(player:getCards(flag)) do
		card_place = card_place or room:getCardPlace(card:getEffectiveId())

		if class_name == "." then table.insert(cards, card)
		elseif isCompulsoryView(card, class_name, player, card_place) then
			card_str = isCompulsoryView(card, class_name, player, card_place)
			card_str = sgs.Card_Parse(card_str)
			table.insert(cards, card_str)
		elseif card:inherits(class_name) and not prohibitUseDirectly(card, player) then table.insert(cards, card)
		elseif getSkillViewCard(card, class_name, player, card_place) then
			card_str = getSkillViewCard(card, class_name, player, card_place)
			card_str = sgs.Card_Parse(card_str)
			table.insert(cards, card_str)
		end
	end
	return cards
end

function SmartAI:getCards(class_name, player, flag)
	player = player or self.player
	return getCards(class_name, player, self.room, flag)
end

function getCardsNum(class_name, player)
	return #getCards(class_name, player)
end

function SmartAI:getCardsNum(class_name, player, flag, selfonly)
	player = player or self.player
	local n = 0
	if type(class_name) == "table" then
		for _, each_class in ipairs(class_name) do
			n = n + #getCards(each_class, player, self.room, flag)
		end
		return n
	end
	n = #getCards(class_name, player, self.room, flag)

	if selfonly then return n end
	if class_name == "Jink" then
		if player:hasLordSkill("hujia") then
			local lieges = self.room:getLieges("wei", player)
			for _, liege in sgs.qlist(lieges) do
				if self:isFriend(liege, player) then
					n = n + self:getCardsNum("Jink", liege, nil, liege:hasLordSkill("hujia"))
				end
			end
		end
	elseif class_name == "Slash" then
		if player:hasSkill("wushuang") then
			n = n * 2
		end
		if player:hasLordSkill("jijiang") then
			local lieges = self.room:getLieges("shu", player)
			for _, liege in sgs.qlist(lieges) do
				if self:isFriend(liege, player) then
				n = n + self:getCardsNum("Slash", liege, nil, liege:hasLordSkill("jijiang"))
				end
			end
		end
	end
	return n
end

function SmartAI:getAllPeachNum(player)
	player = player or self.player
	local n = 0
	for _, friend in ipairs(self:getFriends(player)) do
		n = n + self:getCardsNum("Peach")
	end
	return n
end

function SmartAI:getCardsFromDiscardPile(class_name)
	sgs.discard_pile = self.room:getDiscardPile()
	local cards = {}
	for _, card_id in sgs.qlist(sgs.discard_pile) do
		local card = sgs.Sanguosha:getCard(card_id)
		if card:inherits(class_name) then table.insert(cards, card) end
	end
	
	return cards
end

function SmartAI:getCardsFromDrawPile(class_name)
	sgs.discard_pile = self.room:getDrawPile()
	local cards = {}
	for _, card_id in sgs.qlist(sgs.discard_pile) do
		local card = sgs.Sanguosha:getCard(card_id)
		if card:inherits(class_name) then table.insert(cards, card) end
	end
	
	return cards
end

function SmartAI:getCardsFromGame(class_name)
	local ban = sgs.GetConfig("BanPackages", "")
	local cards = {}
	for i=0, sgs.Sanguosha:getCardCount() do
		local card = sgs.Sanguosha:getCard(i)
		if card:inherits(class_name) and not ban:match(card:getPackage()) then table.insert(cards, card) end
	end
	
	return cards
end

function SmartAI:evaluatePlayerCardsNum(class_name, player)
	player = player or self.player
	local length = sgs.draw_pile:length()
	for _, p in sgs.qlist(self.room:getOtherPlayers(player)) do
		length = length + p:getHandcardNum()
	end
	
	local percentage = (#(self:getCardsFromGame(class_name)) - #(self:getCardsFromDiscardPile(class_name)))/length
	local modified = 1;
	if class_name == "Jink" then modified = 1.23
	elseif class_name == "Analeptic" then modified = 1.17
	elseif class_name == "Peach" then modified = 1.19
	elseif class_name == "Slash" then modified = 1.09
	end
	
	return player:getHandcardNum()*percentage*modified
end

function SmartAI:hasSuit(suit_strings, include_equip, player)
	return self:getSuitNum(suit_strings, include_equip, player) > 0
end

function SmartAI:getSuitNum(suit_strings, include_equip, player)
	player = player or self.player
	local n = 0
	local flag = "h"
	if include_equip then flag = "he" end
	local allcards = player:getCards(flag)
	for _, card in sgs.qlist(allcards) do
		for _, suit_string in ipairs(suit_strings:split("|")) do
			if card:getSuitString() == suit_string then
				n = n + 1
			end
		end
	end
	return n
end

function SmartAI:hasSkill(skill)
	local skill_name = skill
	if type(skill) == "table" then
		skill_name = skill.name
	end

	local real_skill = sgs.Sanguosha:getSkill(skill_name)
	if real_skill and real_skill:isLordSkill() then
		return self.player:hasLordSkill(skill_name)
	else
		return self.player:hasSkill(skill_name)
	end
end

function SmartAI:hasSkills(skill_names, player)
	player = player or self.player
	for _, skill_name in ipairs(skill_names:split("|")) do
		if player:hasSkill(skill_name) then
			return true
		end
	end
end

function SmartAI:fillSkillCards(cards)
	local i = 1
	while i <= #cards do
		if prohibitUseDirectly(cards[i], self.player) then
			table.remove(cards, i)
		else
			i = i + 1
		end
	end
	for _,skill in ipairs(sgs.ai_skills) do
		if self:hasSkill(skill) then
			local skill_card = skill.getTurnUseCard(self)
			if #cards == 0 then skill_card = skill.getTurnUseCard(self,true) end
			if skill_card then table.insert(cards, skill_card) end
		end
	end
end

function SmartAI:useSkillCard(card,use)
	local name
	if card:inherits("LuaSkillCard") then
		name = "#" .. card:objectName()
	else
		name = card:className()
	end
	sgs.ai_skill_use_func[name](card, use, self)
	if use.to then
		if not use.to:isEmpty() and sgs.dynamic_value.damage_card[name] then
			for _, target in sgs.qlist(use.to) do
				if self:damageIsEffective(target) then return end
			end
			use.card = nil
		end
	end
	if not use.card then return end
	local subcards = sgs.QList2Table(use.card:getSubcards())
	local shit = 0
	if #subcards > 0 then
		for _, card in ipairs(subcards) do
			if sgs.Sanguosha:getCard(card):inherits("Shit") then shit = shit + 1 end
		end
	end
	if shit - self.player:getHp() > self:getAllPeachNum() then use.card = nil end
end

function SmartAI:useBasicCard(card, use)
	if self.player:hasSkill("chengxiang") and self.player:getHandcardNum() < 8 and card:getNumber() < 7 then return end
	if not (card:inherits("Peach") and self.player:getLostHp() > 1) and self:needBear() then return end
	self:useCardByClassName(card, use)
end

function SmartAI:aoeIsEffective(card, to)
	if self.player == to then
		return false
	end
	local armor = to:getArmor()
	if armor and armor:inherits("Vine") then
		return false
	end
	if self.room:isProhibited(self.player, to, card) then
		return false
	end
	if self.player:hasSkill("wuyan") or self.player:hasSkill("danlao") then
		return false
	end
	if card:inherits("SavageAssault") then
		if to:hasSkill("huoshou") or to:hasSkill("juxiang") then
			return false
		end
	end
	if (to:hasSkill("zhichi") and self.room:getTag("Zhichi"):toString() == to:objectName()) then
		return false
	end
	if card:inherits("ArcheryAttack") then
		if (to:hasSkill("leiji") and self:getCardsNum("Jink", to) > 0) or (self:isEquip("EightDiagram", to) and to:getHp() > 1) then
			return false
		end
	end
	return true
end

function SmartAI:getDistanceLimit(card)
	if self.player:hasSkill("qicai") then
		return 100
	end

	if card:inherits("Snatch") then
		return 1
	elseif card:inherits("SupplyShortage") then
		if self.player:hasSkill("duanliang") then
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

function SmartAI:getJiemingChaofeng(player)
	local max_x , chaofeng = 0 , 0
	for _, friend in ipairs(self:getFriends(player)) do
		local x = math.min(friend:getMaxHP(), 5) - friend:getHandcardNum()
		if x > max_x then
			max_x = x
		end
	end
	if max_x < 2 then
		chaofeng = 5 - max_x * 2
	else
		chaofeng = (-max_x) * 2
	end
	return chaofeng
end

function SmartAI:getAoeValueTo(card, to , from)
	if not from then from = self.player end
	local value = 0
	local sj_num

	if to:hasSkill("buqu") then
		value = value + 10
	end

	if to:hasSkill("longdan") then
		value = value + 5
	end

	if to:hasSkill("danlao") then
		value = value + 15
	end

	if card:inherits("SavageAssault") then
		sj_num = self:getCardsNum("Slash", to)
		if to:hasSkill("juxiang") then
			value = value + 20
		end
	end
	if card:inherits("ArcheryAttack") then
		sj_num = self:getCardsNum("Jink", to)
	end

	if self:aoeIsEffective(card, to) then
		if to:getHp() > 1 or (self:getCardsNum("Peach", to) + self:getCardsNum("Analeptic", to) > 0) then
			if to:hasSkill("yiji") or to:hasSkill("jianxiong") then
				value = value + 20
			end
			if to:hasSkill("jieming") then
				value = value - self:getJiemingChaofeng(to) * 3
			end
			if to:hasSkill("ganglie") or to:hasSkill("fankui") or to:hasSkill("enyuan") then
				if not self:isFriend(from, to) then
					value = value + 10
				else
					value = value - 10
				end
			end
		end

		if card:inherits("ArcheryAttack") then
			sj_num = self:getCardsNum("Jink", to)
			if (to:hasSkill("leiji") and self:getCardsNum("Jink", to) > 0) or self:isEquip("EightDiagram", to) then
				value = value + 30
				if self:hasSuit("spade", true, to) then
					value = value + 20
				end
			end
			if to:hasSkill("qingguo") or self:isEquip("EightDiagram", to) then
				value = value + 10
			end
		end

		if to:getHp() > 0 then
			value = value - 24 / to:getHp() - 10
		end

		if self:isFriend(from, to) then
		if (to:isLord() or from:isLord()) and not (to:hasSkill("buqu") and to:getPile("buqu"):length() < 5) then
				if to:getHp() <= 1 and self:getCardsNum("Peach", from) == 0 and sj_num == 0 then
					if sgs.evaluatePlayerRole(to) == "renegade" then
						value = value - 50
					else
						value = value - 150
					end
				end
			end
			value = value + self:getCardsNum("Peach", from) * 2
		elseif sgs.evaluatePlayerRole(to) == "rebel" or (to:isLord() and sgs.evaluatePlayerRole(from) == "rebel") then
			if to:getHp() <= 1 and self:getCardsNum("Peach", to) == 0 and sj_num == 0 then
				value = value - 50
			end
		end
	else
		value = value + 10
	end

	return value
end

function SmartAI:getAoeValue(card, player)
	player = player or self.player
	friends_noself = self:getFriendsNoself(player)
	enemies = self:getEnemies(player)
	local good, bad = 0, 0
	for _, friend in ipairs(friends_noself) do
		good = good + self:getAoeValueTo(card, friend, player)
	end

	for _, enemy in ipairs(enemies) do
		bad = bad + self:getAoeValueTo(card, enemy, player)
	end

	if player:hasSkill("jizhi") then
		good = good + 40
	end
	return good - bad
end

function SmartAI:hasTrickEffective(card, player)
	if player then
		if self.room:isProhibited(self.player, player, card) then return false end
		if (player:hasSkill("zhichi") and self.room:getTag("Zhichi"):toString() == player:objectName()) or player:hasSkill("wuyan") then
			if card and not (card:inherits("Indulgence") or card:inherits("SupplyShortage")) then return false end
		end
		if (player:getMark("@fog") > 0 or (player:hasSkill("shenjun") and self.player:getGender() ~= player:getGender())) and
			sgs.dynamic_value.damage_card[card:className()] then return false end
	else
		if self.player:hasSkill("wuyan") then
			if card:inherits("TrickCard") and not
				(card:inherits("DelayedTrick") or card:inherits("GodSalvation") or card:inherits("AmazingGrace")) then
			return false end
		end
	end
	return true
end

function SmartAI:useTrickCard(card, use)
	if self.player:hasSkill("chengxiang") and self.player:getHandcardNum() < 8 and card:getNumber() < 7 then return end
	if self:needBear() and not ("amazing_grace|ex_nihilo|snatch|iron_chain"):match(card:objectName()) then return end
	if card:inherits("AOE") then
		if self.player:hasSkill("wuyan") then return end
		if self.role == "renegade" and not self:isWeak(self.room:getLord()) then use.card = card return end
		if self.role == "rebel" and sgs.turncount < 2 and card:inherits("ArcheryAttack") then return end
		local good, bad = 0, 0
		for _, friend in ipairs(self.friends_noself) do
			if self:aoeIsEffective(card, friend) then
				bad = bad + 20/(friend:getHp())+10
				if friend:isLord() and (friend:getHp() < 3) then
					return
				end

				if (friend:getHp() < 2) and (self.player:isLord()) then
					return
				end
			end
			good = good + self:getCardsNum("Peach")
		end

		for _, enemy in ipairs(self.enemies) do
			if self:aoeIsEffective(card, enemy) then
				good = good + 20/(enemy:getHp())+10

				if enemy:isLord() then
					good = good + 20/(enemy:getHp())
				end
			end
			bad = bad + self:getCardsNum("Peach")
		end

		if good > bad or (self:hasSkills(sgs.need_kongcheng) and self.player:getHandcardNum() == 1) then
			use.card = card
		end
	else
		self:useCardByClassName(card, use)
	end
	if use.to then
		if not use.to:isEmpty() and sgs.dynamic_value.damage_card[card:className()] then
			for _, target in sgs.qlist(use.to) do
				if self:damageIsEffective(target) then return end
			end
			use.card = nil
		end
	end
end

sgs.weapon_range = {}

function SmartAI:hasEquip(card)
	return self.player:hasEquip(card)
end

function SmartAI:isEquip(equip_name, player)
	player = player or self.player
	local cards = player:getCards("e")
	for _, card in sgs.qlist(cards) do
		if card:inherits(equip_name) then return true end
	end
	if equip_name == "EightDiagram" and player:hasSkill("bazhen") and not player:getArmor() then return true end
	if equip_name == "Crossbow" and player:hasSkill("paoxiao") then return true end
	return false
end

sgs.ai_weapon_value = {}

function SmartAI:evaluateWeapon(card)
	local deltaSelfThreat = 0
	local currentRange
	if not card then return -1
	else
		currentRange = sgs.weapon_range[card:className()] or 0
	end
	for _,enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy) <= currentRange then
				deltaSelfThreat = deltaSelfThreat+6/sgs.getDefense(enemy)
		end
	end

	if card:inherits("Crossbow") and deltaSelfThreat ~= 0 then
		if self.player:hasSkill("kurou") then deltaSelfThreat = deltaSelfThreat*3+10 end
		deltaSelfThreat = deltaSelfThreat + self:getCardsNum("Slash")*3-2
	end
	local callback = sgs.ai_weapon_value[card:objectName()]
	if callback and type(callback) == "function" then
		deltaSelfThreat = deltaSelfThreat + (callback(self) or 0)
		for _, enemy in ipairs(self.enemies) do
			if self.player:distanceTo(enemy) <= currentRange and callback then
				deltaSelfThreat = deltaSelfThreat + (callback(self, enemy) or 0)
			end
		end
	end

	return deltaSelfThreat
end

sgs.ai_armor_value = {}

function SmartAI:evaluateArmor(card, player)
	player = player or self.player
	local ecard = card or player:getArmor()
	for _, askill in sgs.qlist(player:getVisibleSkillList()) do
		local callback = sgs.ai_armor_value[askill:objectName()]
		if callback and type(callback) == "function" then
			return (callback(ecard, player, self) or 0)
		end
	end
	if not ecard then return 0 end
	local callback = sgs.ai_armor_value[ecard:objectName()]
	if callback and type(callback) == "function" then
		return (callback(player, self) or 0)
	end
	return 0.5
end

function SmartAI:hasSameEquip(card, player)
	player = player or self.player
	if player:getEquips():isEmpty() then return false end
	if card:inherits("Weapon") then
		if player:getWeapon() then return true end
	elseif card:inherits("Armor") then
		if player:getArmor() then return true end
	elseif card:inherits("DefensiveHorse") then
		if player:getDefensiveHorse() then return true end
	elseif card:inherits("OffensiveHorse") then
		if player:getOffensiveHorse() then return true end
	end
	return false
end

function SmartAI:useEquipCard(card, use)
	if self.player:hasSkill("chengxiang") and self.player:getHandcardNum() < 8 and card:getNumber() < 7 and self:hasSameEquip(card) then return end
	if self:hasSkills(sgs.lose_equip_skill) and self:evaluateArmor(card)>-5 then
		use.card = card
		return
	end
	if self.player:getHandcardNum() == 1 and self:hasSkills(sgs.need_kongcheng) and self:evaluateArmor(card)>-5 then
		use.card = card
		return
	end
	if self:hasSameEquip(card) and
		(self.player:hasSkill("rende") or self.player:hasSkill("qingnang")
		or (self.player:hasSkill("yongsi") and self:getOverflow() < 3)
		or (self.player:hasSkill("qixi") and card:isBlack())) then return end
	self:useCardByClassName(card, use)
	if use.card or use.broken then return end
	if card:inherits("Weapon") then
			if self:needBear() then return end
		if self.player:hasSkill("rende") then
			for _,friend in ipairs(self.friends_noself) do
				if not friend:getWeapon() then return end
			end
		end
		if self.player:getWeapon() and self.player:getWeapon():inherits("YitianSword") then use.card = card return end
		if self:evaluateWeapon(card) > self:evaluateWeapon(self.player:getWeapon()) then
			if (not use.to) and self.weaponUsed and (not self:hasSkills(sgs.lose_equip_skill)) then return end
			if self.player:getHandcardNum() <= self.player:getHp() then return end
			use.card = card
		end
	elseif card:inherits("Armor") then
			if self:needBear() and self.player:getLostHp() == 0 then return end
		local lion = self:getCard("SilverLion")
		if lion and self.player:isWounded() and not self:isEquip("SilverLion") and not card:inherits("SilverLion") and
			not (self:hasSkills("bazhen|yizhong") and not self.player:getArmor()) then
			use.card = lion
			return
		end
		if self.player:hasSkill("rende") and self:evaluateArmor(card)<4 then
			for _,friend in ipairs(self.friends_noself) do
				if not friend:getArmor() then return end
			end
		end
		if self:evaluateArmor(card) > self:evaluateArmor() then use.card = card end
		return
	elseif self:needBear() then return 
	elseif card:inherits("OffensiveHorse") and self.player:hasSkill("rende") then
		for _,friend in ipairs(self.friends_noself) do
			if not friend:getOffensiveHorse() then return end
		end
	elseif card:inherits("Monkey") or self.lua_ai:useCard(card) then
		use.card = card
	end
end

function SmartAI:damageMinusHp(self, enemy, type)
		local trick_effectivenum = 0
		local slash_damagenum = 0
		local analepticpowerup = 0
		local effectivefireattacknum = 0
		local basicnum = 0
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		for _, acard in ipairs(cards) do
			if acard:getTypeId() == sgs.Card_Basic and not acard:inherits("Peach") then basicnum = basicnum + 1 end
		end
		for _, acard in ipairs(cards) do
			if ((acard:inherits("Duel") or acard:inherits("SavageAssault") or acard:inherits("ArcheryAttack") or acard:inherits("FireAttack")) 
			and not self.room:isProhibited(self.player, enemy, acard))
			or ((acard:inherits("SavageAssault") or acard:inherits("ArcheryAttack")) and self:aoeIsEffective(acard, enemy)) then
				if acard:inherits("FireAttack") then
					if not enemy:isKongcheng() then 
					effectivefireattacknum = effectivefireattacknum + 1 
					else
					trick_effectivenum = trick_effectivenum -1
					end
				end
				trick_effectivenum = trick_effectivenum + 1
			elseif acard:inherits("Slash") and self:slashIsEffective(acard, enemy) and ( slash_damagenum == 0 or self:isEquip("Crossbow", self.player)) 
				and (self.player:distanceTo(enemy) <= self.player:getAttackRange()) then
				if not (enemy:hasSkill("xiangle") and basicnum < 2) then slash_damagenum = slash_damagenum + 1 end
				if self:getCardsNum("Analeptic") > 0 and analepticpowerup == 0 and 
					not ((self:isEquip("SilverLion", enemy) or self:isEquip("EightDiagram", enemy) or 
						(not enemy:getArmor() and enemy:hasSkill("bazhen"))) and not self:isEquip("QinggangSword", self.player)) then 
						slash_damagenum = slash_damagenum + 1 
						analepticpowerup = analepticpowerup + 1 
				end
				if self:isEquip("GudingBlade", self.player) and enemy:isKongcheng() and not self:isEquip("SilverLion", enemy) then
					slash_damagenum = slash_damagenum + 1 
				end
			end
		end
		if type == 0 then return (trick_effectivenum + slash_damagenum - effectivefireattacknum - enemy:getHp()) 
		else return  (trick_effectivenum + slash_damagenum - enemy:getHp()) end
	return -10
end

dofile "lua/ai/debug-ai.lua"
dofile "lua/ai/standard_cards-ai.lua"
dofile "lua/ai/maneuvering-ai.lua"
dofile "lua/ai/standard-ai.lua"
dofile "lua/ai/chat-ai.lua"
dofile "lua/ai/basara-ai.lua"
dofile "lua/ai/hegemony-ai.lua"
dofile "lua/ai/hulaoguan-ai.lua"

local loaded = "standard|standard_cards|maneuvering|sp"

local files = table.concat(sgs.GetFileNames("lua/ai"), " ")

for _, aextension in ipairs(sgs.Sanguosha:getExtensions()) do
	if not loaded:match(aextension) and files:match(string.lower(aextension)) then
		dofile("lua/ai/" .. string.lower(aextension) .. "-ai.lua")
	end
end

dofile "lua/ai/sp-ai.lua"

for _, ascenario in ipairs(sgs.Sanguosha:getScenarioNames()) do
	if not loaded:match(ascenario) and files:match(string.lower(ascenario)) then
		dofile("lua/ai/" .. string.lower(ascenario) .. "-ai.lua")
	end
end
