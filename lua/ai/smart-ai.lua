--[[********************************************************************
	Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

  This file is part of QSanguosha-Hegemony.

  This game is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3.0 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  See the LICENSE file for more details.

  QSanguosha-Hegemony Team
*********************************************************************]]

-- This is the Smart AI, and it should be loaded and run at the server side

-- "middleclass" is the Lua OOP library written by kikito
-- more information see: https://github.com/kikito/middleclass


-- initialize the random seed for later use
math.randomseed(os.time())

-- SmartAI is the base class for all other specialized AI classes
SmartAI = (require "middleclass").class("SmartAI")

version = "QSanguosha AI 20140730 (V0.25 Alpha)"

--- this function is only function that exposed to the host program
--- and it clones an AI instance by general name
-- @param player The ServerPlayer object that want to create the AI object
-- @return The AI object
function CloneAI(player)
	return SmartAI(player).lua_ai
end

sgs.ais = 					{}
sgs.ai_card_intention = 	{}
sgs.ai_playerchosen_intention = {}
sgs.ai_Yiji_intention = 	{}
sgs.ai_keep_value = 		{}
sgs.ai_use_value = 			{}
sgs.ai_use_priority = 		{}
sgs.ai_suit_priority = 		{}
sgs.ai_skill_invoke = 		{}
sgs.ai_skill_suit = 		{}
sgs.ai_skill_cardask = 		{}
sgs.ai_skill_choice = 		{}
sgs.ai_skill_askforag = 	{}
sgs.ai_skill_askforyiji = 	{}
sgs.ai_skill_pindian = 		{}
sgs.ai_skill_playerchosen = {}
sgs.ai_skill_discard = 		{}
sgs.ai_cardshow = 			{}
sgs.ai_nullification = 		{}
sgs.ai_skill_cardchosen = 	{}
sgs.ai_skill_use = 			{}
sgs.ai_cardneed = 			{}
sgs.ai_skill_use_func = 	{}
sgs.ai_skills = 			{}
sgs.ai_slash_weaponfilter = {}
sgs.ai_slash_prohibit = 	{}
sgs.ai_trick_prohibit = 	{}
sgs.ai_view_as = 			{}
sgs.ai_cardsview = 			{}
sgs.dynamic_value = 		{
	damage_card = 			{},
	control_usecard = 		{},
	control_card = 			{},
	lucky_chance = 			{},
	benefit = 				{}
}
sgs.ai_choicemade_filter = 	{
	cardUsed = 				{},
	cardResponded = 		{},
	skillInvoke = 			{},
	skillChoice = 			{},
	Nullification =			{},
	playerChosen =			{},
	cardChosen =			{},
	Yiji = 					{},
	viewCards = 			{},
	pindian = 				{}
}

sgs.card_lack =				{}
sgs.ai_need_damaged =		{}
sgs.ai_debug_func =			{}
sgs.ai_chat_func =			{}
sgs.ai_event_callback =		{}
sgs.ai_NeedPeach =			{}
sgs.shown_kingdom = 		{
	wei = 0,
	shu = 0,
	wu = 0,
	qun = 0
}
sgs.ai_damage_effect = 		{}
sgs.ai_explicit = 			{}
sgs.ai_loyalty = 			{
	wei = {},
	shu = {},
	wu = {},
	qun = {}
}
sgs.RolesTable =			{
	"lord",
	"loyalist",
	"renegade",
	"rebel",
	"careerist"
}
sgs.KingdomsTable = 		{
	"wei",
	"shu",
	"wu",
	"qun"
}
sgs.current_mode_players = {
	wei = 0,
	shu = 0,
	wu = 0,
	qun = 0
}
sgs.general_shown = {}
sgs.Slash_Natures = {
	Slash = sgs.DamageStruct_Normal,
	FireSlash = sgs.DamageStruct_Fire,
	ThunderSlash = sgs.DamageStruct_Thunder,
}

for i = sgs.NonTrigger, sgs.NumOfEvents, 1 do
	sgs.ai_debug_func[i] = {}
	sgs.ai_chat_func[i] = {}
	sgs.ai_event_callback[i] = {}
end

function setInitialTables()
	sgs.ai_type_name = 			{"Skill", "Basic", "Trick", "Equip"}
	sgs.lose_equip_skill = "xiaoji"
	sgs.lose_one_equip_skill = ""
	sgs.need_kongcheng = "kongcheng"
	sgs.masochism_skill = 		"yiji|fankui|jieming|ganglie|fangzhu|hengjiang|qianhuan"
	sgs.wizard_skill = 		"guicai|guidao|tiandu"
	sgs.wizard_harm_skill = 	"guicai|guidao"
	sgs.priority_skill = 		"dimeng|haoshi|qingnang|jizhi|guzheng|qixi|jieyin|guose|duanliang|fanjian|lijian|tuxi|qiaobian|zhiheng|luoshen|rende|wansha|qingcheng|shuangren"
	sgs.save_skill = 		"jijiu"
	sgs.exclusive_skill = 		"duanchang|buqu"
	sgs.cardneed_skill =		"paoxiao|tianyi|shuangxiong|jizhi|guose|duanliang|qixi|qingnang|luoyi|kanpo|" ..
								"jieyin|zhiheng|rende|guicai|guidao|luanji|qiaobian|beige|lirang|xiaoguo"
	sgs.drawpeach_skill =		"tuxi|qiaobian"
	sgs.recover_skill =		"rende|kuanggu|zaiqi|jieyin|qingnang|yinghun|hunzi|shenzhi|buqu"
	sgs.use_lion_skill =		 "duanliang|qixi|guidao|lijian|zhiheng|fenxun|qingcheng"
	sgs.need_equip_skill = 		"shensu|beige|huyuan|qingcheng"
	sgs.judge_reason =		"bazhen|EightDiagram|supply_shortage|tuntian|qianxi|indulgence|lightning|leiji|tieqi|luoshen|ganglie"

	sgs.Friend_All = 0
	sgs.Friend_Draw = 1
	sgs.Friend_Male = 2
	sgs.Friend_Female = 3
	sgs.Friend_Wounded = 4
	sgs.Friend_MaleWounded = 5
	sgs.Friend_FemaleWounded = 6

	for _, p in sgs.qlist(global_room:getAlivePlayers()) do
		local kingdom = p:getKingdom()
		if not table.contains(sgs.KingdomsTable, kingdom) and kingdom ~= "default" then
			table.insert(sgs.KingdomsTable, kingdom)
		end
		sgs.ai_loyalty[kingdom] = {}
		sgs.shown_kingdom[p:getKingdom()] = 0
		sgs.ai_explicit[p:objectName()] = "unknown"
		sgs.general_shown[p:objectName()] = {}
		if string.len(p:getRole()) == 0 then
			global_room:setPlayerProperty(p, "role", sgs.QVariant(p:getKingdom()))
		end
		if not table.contains(sgs.RolesTable, p:getRole()) then
			table.insert(sgs.RolesTable, kingdom)
		end
	end

	for _, p in sgs.qlist(global_room:getAlivePlayers()) do
		local kingdom = p:getKingdom()
		for kingdom, v in pairs(sgs.ai_loyalty) do
			sgs.ai_loyalty[kingdom][p:objectName()] = 0
		end
	end

end

function SmartAI:initialize(player)
	self.player = player
	self.room = player:getRoom()
	self.role = player:getRole()
	self.lua_ai = sgs.LuaAI(player)
	self.lua_ai.callback = function(full_method_name, ...)
		--The __FUNCTION__ macro is defined as CLASS_NAME::SUBCLASS_NAME::FUNCTION_NAME
		--in MSVC, while in gcc only FUNCTION_NAME is in place.
		local method_name_start = 1
		while true do
			local found = string.find(full_method_name, "::", method_name_start)
			if found ~= nil then
				method_name_start = found + 2
			else
				break
			end
		end
		local method_name = string.sub(full_method_name, method_name_start)
		local method = self[method_name]
		if method then
			local success, result1, result2
			success, result1, result2 = pcall(method, self, ...)
			if not success then
				self.room:writeToConsole(result1)
				self.room:writeToConsole(method_name)
				self.room:writeToConsole(debug.traceback())
				self.room:outputEventStack()
			else
				return result1, result2
			end
		end
	end

	self.retain = 2
	self.keepValue = {}
	self.kept = {}
	self.keepdata = {}
	self.predictedRange = 1
	self.slashAvail = 1

	if not sgs.initialized then
		sgs.initialized = true
		sgs.ais = {}
		sgs.turncount = 0
		sgs.debugmode = true
		global_room = self.room
		global_room:writeToConsole(version .. ", Powered by " .. _VERSION)

		setInitialTables()
	end

	sgs.ais[player:objectName()] = self

	sgs.card_lack[player:objectName()] = {}
	sgs.card_lack[player:objectName()]["Slash"] = 0
	sgs.card_lack[player:objectName()]["Jink"] = 0
	sgs.card_lack[player:objectName()]["Peach"] = 0
	sgs.ai_NeedPeach[player:objectName()] = 0

	sgs.updateAlivePlayerRoles()
	self:updatePlayers()
	self:assignKeep(true)
end

function sgs.cloneCard(name, suit, number)
	suit = suit or sgs.Card_SuitToBeDecided
	number = number or -1
	local card = sgs.Sanguosha:cloneCard(name, suit, number)
	if not card then global_room:writeToConsole(debug.traceback()) return end
	card:deleteLater()
	return card
end

function SmartAI:getTurnUse(priority)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)

	local turnUse = {}
	local slash = sgs.cloneCard("slash")
	local slashAvail = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_Residue, self.player, slash)
	self.slashAvail = slashAvail
	self.predictedRange = self.player:getAttackRange()
	self.slash_distance_limit = (1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, self.player, slash) > 50)

	self.weaponUsed = false
	self:fillSkillCards(cards, priority)
	self:sortByUseValue(cards)

	if self.player:hasWeapon("Crossbow") or #self.player:property("extra_slash_specific_assignee"):toString():split("+") > 1 then
		slashAvail = 100
		self.slashAvail = slashAvail
	end

	for _, card in ipairs(cards) do
		if not card:isAvailable(self.player) then continue end

		if priority and self:getDynamicUsePriority(card) < 6 then continue end

		local dummy_use = { isDummy = true }

		local type = card:getTypeId()
		self["use" .. sgs.ai_type_name[type + 1] .. "Card"](self, card, dummy_use)

		if dummy_use.card then
			if dummy_use.card:isKindOf("Slash") then
				if slashAvail > 0 then
					slashAvail = slashAvail - 1
					table.insert(turnUse, dummy_use.card)
				elseif dummy_use.card:hasFlag("AIGlobal_KillOff") then table.insert(turnUse, dummy_use.card) end
			else
				if self.player:hasFlag("InfinityAttackRange") or self.player:getMark("InfinityAttackRange") > 0 then
					self.predictedRange = 10000
				elseif dummy_use.card:isKindOf("Weapon") then
					if not sgs.weapon_range[card:getClassName()] then
						self.room:writeToConsole("weapon_range" .. card:getClassName())
					end
					self.predictedRange = sgs.weapon_range[card:getClassName()] or 1
					self.weaponUsed = true
				else
					self.predictedRange = 1
				end
				if dummy_use.card:objectName() == "Crossbow" then slashAvail = 100 self.slashAvail = slashAvail end
				table.insert(turnUse, dummy_use.card)
			end
		end
	end

	return turnUse
end

function SmartAI:activate(use)
	self:updatePlayers()
	self:assignKeep(true)
	if self.player:getHandcardNum() > 30 then
		self.toUse = self:getTurnUse(true)
		self:sortByDynamicUsePriority(self.toUse)
		for _, card in ipairs(self.toUse) do
			if not self.player:isCardLimited(card, card:getHandlingMethod())
				or (card:canRecast() and not self.player:isCardLimited(card, sgs.Card_MethodRecast)) then
				local type = card:getTypeId()

				self["use" .. sgs.ai_type_name[type + 1] .. "Card"](self, card, use)

				if use:isValid(nil) then
					self.toUse = nil
					return
				end
			end
		end
	end
	self.toUse = self:getTurnUse()
	self:sortByDynamicUsePriority(self.toUse)
	for _, card in ipairs(self.toUse) do
		if not self.player:isCardLimited(card, card:getHandlingMethod())
			or (card:canRecast() and not self.player:isCardLimited(card, sgs.Card_MethodRecast)) then
			local type = card:getTypeId()

			self["use" .. sgs.ai_type_name[type + 1] .. "Card"](self, card, use)

			if use:isValid(nil) then
				self.toUse = nil
				return
			end
		end
	end
	self.toUse = nil
end

function SmartAI:objectiveLevel(player)
	if not player then self.room:writeToConsole(debug.traceback()) return 0 end
	if self.player:objectName() == player:objectName() then return -2 end
	if self.room:getMode() == "jiange_defense" and self.player:getKingdom() == player:getKingdom() then return -2 end
	if self.player:isFriendWith(player) then return -2 end
	if self.room:alivePlayerCount() == 2 then return 5 end

	local self_kingdom = self.player:getKingdom()
	local player_kingdom_evaluate = self:evaluateKingdom(player)
	local player_kingdom_explicit = sgs.ai_explicit[player:objectName()]
	if player_kingdom_explicit == "unknown" then
		local mark = string.format("KnownBoth_%s_%s", self.player:objectName(), player:objectName())
		if player:getMark(mark) > 0 then
			player_kingdom_explicit = player:getRole() == "careerist" and "careerist" or player:getKingdom()
		end
	end

	local upperlimit = player:getLord() and 99 or self.room:getPlayers():length() / 2
	if (not sgs.isAnjiang(self.player) or sgs.shown_kingdom[self_kingdom] < upperlimit) and self.role ~= "careerist" and self_kingdom == player_kingdom_explicit then return -2 end
	if self:getKingdomCount() <= 2 then return 5 end

	local selfIsCareerist = self.role == "careerist" or sgs.shown_kingdom[self_kingdom] >= upperlimit and sgs.isAnjiang(self.player)

	local gameProcess = sgs.gameProcess()
	if gameProcess == "===" then
		if player:getMark("KnownBothEnemy" .. self.player:objectName()) > 0 then return 5 end
		if not selfIsCareerist and sgs.shown_kingdom[self_kingdom] < upperlimit then
			if sgs.isAnjiang(player) and player_kingdom_explicit == "unknown" then
				if player_kingdom_evaluate == self_kingdom then return -1
				elseif string.find(player_kingdom_evaluate, self_kingdom) then return 0
				elseif player_kingdom_evaluate == "unknown" and player:getHp() == 1 then return 0
				else
					return self:getOverflow() > 0 and 3.5 or 0
				end
			else
				return 5
			end
		else return self:getOverflow() > 0 and 4 or 0
		end
	elseif string.find(gameProcess, ">") then
		local isWeakPlayer = player:getHp() == 1
						and (player:isKongcheng() or sgs.card_lack[player:objectName()] == 1 and player:getHandcardNum() <= 1)
						and (sgs.getReward(player) >= 2 or self.player:aliveCount() <= 4) and self:isWeak(player)
		local kingdom = gameProcess:split(">")[1]
		if string.find(gameProcess, ">>>") then
			if self_kingdom == kingdom and not selfIsCareerist then
				if sgs.shown_kingdom[self_kingdom] < upperlimit and sgs.isAnjiang(player)
					and (player_kingdom_evaluate == self_kingdom or string.find(player_kingdom_evaluate, self_kingdom)) then return 0
				elseif player_kingdom_evaluate == "unknown" and sgs.turncount <= 2 then return 0
				else return 5
				end
			else
				if player_kingdom_explicit == kingdom or player_kingdom_evaluate == kingdom or isWeakPlayer then return 5
				elseif not string.find(player_kingdom_evaluate, kingdom) then return -1
				elseif player_kingdom_explicit == "careerist" then return -1
				elseif player_kingdom_evaluate == "unknown" then return 0
				else return 3
				end
			end
		elseif string.find(gameProcess, ">>") then
			if self_kingdom == kingdom and not selfIsCareerist then
				if sgs.shown_kingdom[self_kingdom] < upperlimit and sgs.isAnjiang(player) then
					if player_kingdom_evaluate == self_kingdom then return -1
					elseif string.find(player_kingdom_evaluate, self_kingdom) then return 0
					elseif player_kingdom_evaluate == "unknown" and sgs.turncount <= 2 then return 0
					end
				end
				return 5
			else
				if player_kingdom_explicit == kingdom or player_kingdom_evaluate == kingdom or isWeakPlayer then return 5
				elseif not string.find(player_kingdom_evaluate, kingdom) then return 0
				elseif player_kingdom_explicit == "careerist" then return 0
				elseif player_kingdom_evaluate == "unknown" then return 0
				else return 3
				end
			end
		else
			if self_kingdom == kingdom and not selfIsCareerist then
				if sgs.shown_kingdom[self_kingdom] < upperlimit and sgs.isAnjiang(player) then
					if player_kingdom_evaluate == self_kingdom then return -1
					elseif string.find(player_kingdom_evaluate, self_kingdom) then return 0
					elseif player_kingdom_evaluate == "unknown" and sgs.turncount <= 2 then return 0
					end
				end
				return 5
			else
				if player_kingdom_explicit == kingdom or isWeakPlayer then return 5
				elseif player_kingdom_evaluate == kingdom then return 3
				elseif player_kingdom_explicit == "careerist" then return 0
				elseif not string.find(player_kingdom_evaluate, kingdom) then return 0
				else return 1
				end
			end
		end
	end
end

function sgs.gameProcess(update)
	if not update and sgs.ai_gameProcess then return sgs.gameProcess end
	local value = {}
	local kingdoms = sgs.KingdomsTable
	for _, kingdom in ipairs(kingdoms) do
		value[kingdom] = 0
	end

	local anjiang = 0
	local players = global_room:getAlivePlayers()
	for _, ap in sgs.qlist(players) do
		if table.contains(kingdoms, sgs.ai_explicit[ap:objectName()]) then
			local v = 3 + sgs.getDefense(ap, true) / 2
			value[sgs.ai_explicit[ap:objectName()]] = value[sgs.ai_explicit[ap:objectName()]] + v
		else
			anjiang = anjiang + 1
		end
	end

	local cmp = function(a, b)
		return value[a] > value[b]
	end
	table.sort(kingdoms, cmp)
	local sum_value1, sum_value2, sum_value3 = 0, 0, 0
	for i = 2, #kingdoms do
		sum_value1 = sum_value1 + value[kingdoms[i]]
		if i < #kingdoms then sum_value2 = sum_value2 + value[kingdoms[i]] end
		if i > 2 then sum_value3 = sum_value3 + value[kingdoms[i]] end

	end

	local process = "==="
	if value[kingdoms[1]] >= sum_value1 and value[kingdoms[1]] > 0 then
		if anjiang <= 1 and players:length() > 4 then process = kingdoms[1] .. ">>>"
		elseif anjiang <= 3 then process = kingdoms[1] .. ">>"
		elseif anjiang <= 5 then process = kingdoms[1] .. ">" end
	elseif value[kingdoms[1]] >= sum_value2 and value[kingdoms[1]] > 0 then
		if anjiang == 0 then process = kingdoms[1] .. ">>"
		elseif anjiang <= 3 then process = kingdoms[1] .. ">" end
	elseif value[kingdoms[1]] >= sum_value3 and value[kingdoms[1]] > 0 then
		process = kingdoms[1] .. ">"
	end

	sgs.ai_process = process
	return process
end

function SmartAI:evaluateKingdom(player, other)
	if not player then self.room:writeToConsole(debug.traceback()) return "unknown" end
	other = other or self.player
	if self.room:getMode() == "custom_scenario" then
		return player:getRole() == "careerist" and "careerist" or player:getKingdom()
	end
	if self.room:getMode() == "jiange_defense" then return player:getKingdom() end
	if sgs.ai_explicit[player:objectName()] ~= "unknown" then return sgs.ai_explicit[player:objectName()] end
	if player:getMark(string.format("KnownBoth_%s_%s", other:objectName(), player:objectName())) > 0 then
		local upperlimit = player:getLord() and 99 or self.room:getPlayers():length() / 2
		return sgs.shown_kingdom[player:getKingdom()] < upperlimit and player:getKingdom() or "careerist"
	end

	local max_value, max_kingdom = 0, {}
	local KnownBothEnemy = player:getMark("KnownBothEnemy" .. other:objectName()) > 0
	for kingdom, v in pairs(sgs.ai_loyalty) do
		if KnownBothEnemy and kingdom == other:getKingdom() then continue end
		if sgs.ai_loyalty[kingdom][player:objectName()] > max_value then
			max_value = sgs.ai_loyalty[kingdom][player:objectName()]
		end
	end
	if max_value > 0 then
		for kingdom, v in pairs(sgs.ai_loyalty) do
			if sgs.ai_loyalty[kingdom][player:objectName()] == max_value then
				table.insert(max_kingdom, kingdom)
			end
		end
	end

	return #max_kingdom > 0 and table.concat(max_kingdom, "?") or "unknown"
end

function sgs.isAnjiang(player, another)
	if sgs.ai_explicit[player:objectName()] == "unknown" and not player:hasShownOneGeneral() then return true end
	return false
end

sgs.ai_card_intention["general"] = function(to, level)
end

function sgs.updateIntention(from, to, intention)
	if not from or not to then global_room:writeToConsole(debug.traceback()) end
	if not intention or type(intention) ~= "number" then global_room:writeToConsole(debug.traceback()) end
	if intention > 0 then intention = 10 end
	if intention < 0 then intention = -10 end
	local sendLog, output_to
	if sgs.recorder:evaluateKingdom(from) == "careerist" or sgs.recorder:evaluateKingdom(to, from) == "careerist" then
	elseif from:objectName() == to:objectName() then
	else
		local to_kingdom = sgs.recorder:evaluateKingdom(to, from)
		local kingdoms = sgs.KingdomsTable
		if sgs.isAnjiang(from) and to_kingdom ~= "unknown" then
			to_kingdom = to_kingdom:split("?")
			if intention > 0 then
				sendLog = true
				sgs.outputKingdomValues(from, intention)
				for _, kingdom in ipairs(kingdoms) do
					if table.contains(to_kingdom, kingdom) then
						sgs.ai_loyalty[kingdom][from:objectName()] = sgs.ai_loyalty[kingdom][from:objectName()] - intention
					else
						sgs.ai_loyalty[kingdom][from:objectName()] = sgs.ai_loyalty[kingdom][from:objectName()] + intention
					end
				end
			elseif intention < 0 then
				sendLog = true
				sgs.outputKingdomValues(from, intention)
				sgs.ai_loyalty[to:getKingdom()][from:objectName()] = sgs.ai_loyalty[to:getKingdom()][from:objectName()] - intention
			end
		elseif to:getMark(string.format("KnownBoth_%s_%s", from:objectName(), to:objectName())) > 0 and sgs.isAnjiang(to) then
			if sgs.isAnjiang(from) then
				from:setMark("KnownBothEnemy" .. to:objectName(), 1)
				to:setMark("KnownBothEnemy" .. from:objectName(), 1)
			else
				sendLog = true
				output_to = true
				sgs.outputKingdomValues(to, intention)
				for _, kingdom in ipairs(kingdoms) do
					if kingdom ~= from:getKingdom() then
						sgs.ai_loyalty[kingdom][to:objectName()] = sgs.ai_loyalty[kingdom][to:objectName()] + intention
					else
						sgs.ai_loyalty[kingdom][to:objectName()] = sgs.ai_loyalty[kingdom][to:objectName()] - intention
					end
				end
			end
		end
	end

	for _, p in sgs.qlist(global_room:getAllPlayers()) do
		sgs.ais[p:objectName()]:updatePlayers()
	end

	sgs.outputKingdomValues(output_to and to or from, sendLog and intention or 0, sendLog)
end

function sgs.outputKingdomValues(player, level, sendLog)
	local name = player:getGeneralName() .. "/" .. player:getGeneral2Name()
	if name == "anjiang/anjiang" then name = "SEAT" .. player:getSeat() end
	local msg = name .. " " .. level .. " "
	for _, kingdom in ipairs(sgs.KingdomsTable) do
		msg = msg .. " " .. kingdom .. math.ceil(sgs.ai_loyalty[kingdom][player:objectName()])
	end
	msg = msg .. " gP " .. sgs.gameProcess() .. " "
	for _, kingdom in ipairs(sgs.KingdomsTable) do
		msg = msg .. sgs.current_mode_players[kingdom]
	end
	global_room:writeToConsole(msg)
	if sendLog then
		local log = sgs.LogMessage()
		log.type = "#AI_evaluateKingdom"
		log.arg = sgs.recorder:evaluateKingdom(player)
		log.from = player
		global_room:sendLog(log)
	end
end

function SmartAI:updatePlayers(update)
	if update ~= false then update = true end

	self.friends = {}
	self.enemies = {}
	self.friends_noself = {}

	sgs.updateAlivePlayerRoles()
	self.role = self.player:getRole()

	if update then
		sgs.gameProcess(true)
	end

	if not sgs.isAnjiang(self.player) then
		local updateNewKingdom = self.player:getRole() == "careerist" and sgs.ai_explicit[self.player:objectName()] ~= "careerist"
									or self.player:getRole() ~= "careerist" and sgs.ai_explicit[self.player:objectName()] ~= self.player:getKingdom()
		if updateNewKingdom then self:updatePlayerKingdom(self.player) end
	end

	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		local level = self:objectiveLevel(player)
		if level < 0 then
			table.insert(self.friends_noself, player)
			table.insert(self.friends, player)
		elseif level > 0 then
			table.insert(self.enemies, player)
		end
	end
	table.insert(self.friends, self.player)
end

function SmartAI:updatePlayerKingdom(player, data)
	sgs.ai_explicit[player:objectName()] = player:getRole() == "careerist" and "careerist" or player:getKingdom()
	if data then
		local isHead = data:toBool()
		sgs.general_shown[player:objectName()][isHead and "head" or "deputy"] = true
	else
		sgs.general_shown[player:objectName()]["head"] = player:hasShownGeneral1()
		sgs.general_shown[player:objectName()]["deputy"] = player:hasShownGeneral2()
	end
	for _, k in ipairs(sgs.KingdomsTable) do
		if k == sgs.ai_explicit[player:objectName()] then sgs.ai_loyalty[k][player:objectName()] = 99
		else sgs.ai_loyalty[k][player:objectName()] = 0
		end
	end

	for k, v in pairs(sgs.shown_kingdom) do
		sgs.shown_kingdom[k] = 0
	end

	for _, p in sgs.qlist(self.room:getPlayers()) do
		if sgs.ai_explicit[p:objectName()] ~= "unknown" then
			sgs.ai_explicit[p:objectName()] = p:getRole() == "careerist" and "careerist" or p:getKingdom()
		end
		if sgs.ai_explicit[p:objectName()] == "careerist" or sgs.ai_explicit[p:objectName()] == "unknown" then continue end
		sgs.shown_kingdom[sgs.ai_explicit[p:objectName()]] = sgs.shown_kingdom[sgs.ai_explicit[p:objectName()]] + 1
	end

	if data then
		for _, p in sgs.qlist(global_room:getAllPlayers()) do
			sgs.ais[p:objectName()]:updatePlayers()
		end
	end
end

function sgs.getDefense(player)
	if not player then return 0 end
	local attacker = global_room:getCurrent()
	if not attacker then return sgs.getValue(player) end
	local hp = player:getHp()
	if player:hasShownSkill("benghuai") and player:getHp() > 4 then hp = 4 end
	local defense = math.min(hp * 2 + player:getHandcardNum(), hp * 3)
	local hasEightDiagram = false
	if player:hasArmorEffect("EightDiagram") or player:hasArmorEffect("bazhen") then
		hasEightDiagram = true
	end

	if player:getArmor() and player:hasArmorEffect(player:getArmor():objectName()) then defense = defense + 2 end
	if player:getDefensiveHorse() then defense = defense + 0.5 end

	if hasEightDiagram then
		if player:hasShownSkill("tiandu") then defense = defense + 1 end
		if player:hasShownSkill("leiji") then defense = defense + 1 end
		if player:hasShownSkill("hongyan") then defense = defense + 1 end
	end

	local m = sgs.masochism_skill:split("|")
	for _, masochism in ipairs(m) do
		if player:hasShownSkill(masochism) then
			local goodHp = player:getHp() > 1 or getCardsNum("Peach", player, global_room:getCurrent()) >= 1 or getCardsNum("Analeptic", player, global_room:getCurrent()) >= 1
							or hasBuquEffect(player) or (player:hasShownSkill("niepan") and player:getMark("@nirvana") > 0)
			if goodHp then defense = defense + 1 end
		end
	end

	if player:hasShownSkill("jieming") then defense = defense + 3 end
	if player:hasShownSkill("yiji") then defense = defense + 2 end
	if player:hasShownSkill("tuxi") then defense = defense + 0.5 end
	if player:hasShownSkill("luoshen") then defense = defense + 1 end

	if player:hasShownSkill("rende") and player:getHp() > 2 then defense = defense + 1 end
	if player:hasShownSkill("zaiqi") and player:getHp() > 1 then defense = defense + player:getLostHp() * 0.5 end
	if player:hasShownSkills("tieqi|liegong|kuanggu") then defense = defense + 0.5 end
	if player:hasShownSkill("xiangle") then defense = defense + 1 end
	if player:hasShownSkill("shushen") then defense = defense + 1 end
	if player:hasShownSkill("kongcheng") and player:isKongcheng() then defense = defense + 2 end
	if player:hasShownSkill("shouyue") then
		for _, p in sgs.qlist(global_room:getAlivePlayers()) do
			if p:getKingdom() == "shu" then
				if p:hasShownSkill("wusheng") then defense = defense + 1 end
				if p:hasShownSkill("paoxiao") then defense = defense + 1 end
				if p:hasShownSkill("longdan") then defense = defense + 1 end
				if p:hasShownSkill("liegong") then defense = defense + 1 end
				if p:hasShownSkill("tieqi") then defense = defense + 1 end
			end
		end
	end

	if player:hasShownSkill("yinghun") and player:getLostHp() > 0 then defense = defense + player:getLostHp() - 0.5 end
	if player:hasShownSkill("tianxiang") then defense = defense + player:getHandcardNum() * 0.5 end
	if player:hasShownSkill("buqu") then defense = defense + math.max(4 - player:getPile("buqu"):length(), 0) end
	if player:hasShownSkill("guzheng") then defense = defense + 1 end
	if player:hasShownSkill("dimeng") then defense = defense + 2 end
	if player:hasShownSkill("keji") then defense = defense + player:getHandcardNum() * 0.5 end
	if player:hasShownSkill("jieyin") and player:getHandcardNum() > 1 then defense = defense + 2 end

	if player:hasShownSkill("qianhuan") then defense = defense + 4 end
	if player:hasShownSkill("jijiu") then defense = defense + 2 end
	if player:hasShownSkill("lijian") then defense = defense + 0.5 end
	if player:hasLordSkill("hongfa") then
		for _, p in sgs.qlist(global_room:getAlivePlayers()) do
			if sgs.ai_explicit[p:objectName()] == "qun" then defense = defense + 1 end
		end
	end

	if not player:faceUp() then defense = defense - 0.5 end
	if player:containsTrick("indulgence") then defense = defense - 0.5 end
	if player:containsTrick("supply_shortage") then defense = defense - 0.5 end

	return defense
end

function sgs.getValue(player)
	if not player then global_room:writeToConsole(debug.traceback()) end
	return player:getHp() * 2 + player:getHandcardNum()
end

function SmartAI:assignKeep(start)
	local num = self.player:getHandcardNum()
	self.keepValue = {}
	self.kept = {}

	if start then
		--[[
			通常的保留顺序
			"peach-1" = 7,
			"peach-2" = 5.8, "jink-1" = 5.2,
			"peach-3" = 4.5, "analeptic-1" = 4.1,
			"jink-2" = 4.0, "ExNihilo-1" = 3.9, BefriendAttacking-1", "nullification-1" = 3.8, "thunderslash-1" = 3.66 "fireslash-1" = 3.63
			"slash-1" = 3.6 indulgence-1 = 3.5 SupplyShortage-1 = 3.48 snatch-1 = 3.46 Dismantlement-1 = 3.44 Duel-1 = 3.42 Drownning -3.40
				BurningCamps = 3.38, Collateral-1 = 3.36 ArcheryAttack-1 = 3.35 SavageAssault-1 = 3.34 KnownBoth = 3.33 IronChain = 3.32 GodSalvation-1 = 3.30,
				Fireattack-1 = 3.28 AllianceFeast = 3.26 FightTogether =3.24 LureTiger = 3.22 "peach-4" = 3.1
			"analeptic-2" = 2.9, "jink-3" = 2.7 ExNihilo-2 = 2.7 nullification-2 = 2.6 thunderslash-2 = 2.46 fireslash-2 = 2.43 slash-2 = 2.4
			...
			Weapon-1 = 2.08 Armor-1 = 2.06 DefensiveHorse-1 = 2.04 OffensiveHorse-1 = 2
			...
			AwaitExhausted = 1
			AmazingGrace-1 = -9 Lightning-1 = -10
		]]

		self.keepdata = {}
		for k, v in pairs(sgs.ai_keep_value) do
			self.keepdata[k] = v
		end

		for _, askill in sgs.qlist(self.player:getVisibleSkillList(true)) do
			local skilltable = sgs[askill:objectName() .. "_keep_value"]
			if skilltable then
				for k, v in pairs(skilltable) do
					self.keepdata[k] = v
				end
			end
		end
	end

	if sgs.turncount <= 1 and #self.enemies == 0 then
		self.keepdata.Jink = 4.2
	end

	if not self:isWeak() or num >= 4 then
		for _, friend in ipairs(self.friends_noself) do
			if self:willSkipDrawPhase(friend) or self:willSkipPlayPhase(friend) then
				self.keepdata.Nullification = 5.5
				break
			end
		end
	end

	if self:getOverflow(self.player, true) == 1 then
		self.keepdata.Analeptic = (self.keepdata.Jink or 5.2) + 0.1
		-- 特殊情况下还是要留闪，待补充...
	end

	if not self:isWeak() then
		local needDamaged = false
		if not needDamaged and not sgs.isGoodTarget(self.player, self.friends, self) then needDamaged = true end
		if not needDamaged then
			for _, skill in sgs.qlist(self.player:getVisibleSkillList(true)) do
				local callback = sgs.ai_need_damaged[skill:objectName()]
				if type(callback) == "function" and callback(self, nil, self.player) then
					needDamaged = true
					break
				end
			end
		end
		if needDamaged then
			self.keepdata.ThunderSlash = 5.2
			self.keepdata.FireSlash = 5.1
			self.keepdata.Slash = 5
			self.keepdata.Jink = 4.5
		end
	end

	if num == 0 then return end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards, true, self.kept, true)

	local resetCards = function(allcards)
		local result = {}
		for _, a in ipairs(allcards) do
			local found
			for _, b in ipairs(self.kept) do
				if a:getEffectiveId() == b:getEffectiveId() then
					found = true
					break
				end
			end
			if not found then table.insert(result, a) end
		end
		return result
	end

	for i = 1, num do
		for _, card in ipairs(cards) do
			self.keepValue[card:getId()] = self:getKeepValue(card, self.kept)
			table.insert(self.kept, card)
			break
		end
		cards = resetCards(cards)
	end

	if not self.player:getEquips():isEmpty() then
		for _, card in sgs.qlist(self.player:getEquips()) do
			self.keepValue[card:getId()] = self:getKeepValue(card, self.kept, true)
		end
	end
end

function SmartAI:getKeepValue(card, kept, writeMode)
	local owner = self.room:getCardOwner(card:getEffectiveId())
	if owner and owner:objectName() ~= self.player:objectName() then
		self.room:writeToConsole(debug.traceback())
		return
	end
	if not kept then
		return self.keepValue[card:getId()] or self.keepdata[card:getClassName()] or sgs.ai_keep_value[card:getClassName()] or 0
	end

	local maxvalue = self.keepdata[card:getClassName()] or sgs.ai_keep_value[card:getClassName()] or 0
	local mostvaluable_class = card:getClassName()
	for k, v in pairs(self.keepdata) do
		if isCard(k, card, self.player) and v > maxvalue then
			maxvalue = v
			mostvaluable_class = k
		end
	end

	local cardPlace = self.room:getCardPlace(card:getEffectiveId())
	if writeMode then
		if cardPlace == sgs.Player_PlaceEquip then
			if card:isKindOf("Armor") and self:needToThrowArmor() then return -10
			elseif self.player:hasSkills(sgs.lose_equip_skill) then
				if card:isKindOf("OffensiveHorse") then return -10
				elseif card:isKindOf("Weapon") then return -9.9
				elseif card:isKindOf("OffensiveHorse") then return -9.8
				else return -9.7
				end
			elseif self.player:hasSkills("bazhen|jgyizhong") and card:isKindOf("Armor") then return -8
			elseif self:needKongcheng() then return 5.0
			end
			local value = 0
			if card:isKindOf("Armor") then value = self:isWeak() and 5.2 or 3.2
			elseif card:isKindOf("DefensiveHorse") then value = self:isWeak() and 4.3 or 3.19
			elseif card:isKindOf("Weapon") then value = self.player:getPhase() == sgs.Player_Play and self:slashIsAvailable() and 3.39 or 3.2
			else value = 3.19
			end
			if mostvaluable_class ~= card:getClassName() then
				value = value + maxvalue
			end
			return value
		elseif cardPlace == sgs.Player_PlaceHand then
			local value_suit, value_number, newvalue = 0, 0, 0
			local suit_string = card:getSuitString()
			local number = card:getNumber()
			local i = 0

			for _, askill in sgs.qlist(self.player:getVisibleSkillList(true)) do
				if sgs[askill:objectName() .. "_suit_value"] then
					local v = sgs[askill:objectName() .. "_suit_value"][suit_string]
					if v then
						i = i + 1
						value_suit = value_suit + v
					end
				end
			end
			if i > 0 then value_suit = value_suit / i end

			i = 0
			for _, askill in sgs.qlist(self.player:getVisibleSkillList(true)) do
				if sgs[askill:objectName() .. "_number_value"] then
					local v = sgs[askill:objectName() .. "_number_value"][tostring(number)]
					if v then
						i = i + 1
						value_number = value_number + v
					end
				end
			end
			if i > 0 then value_number = value_number / i end

			newvalue = maxvalue + value_suit + value_number
			if mostvaluable_class ~= card:getClassName() then newvalue = newvalue + 0.1 end
			newvalue = self:adjustKeepValue(card, newvalue)

			return newvalue
		else
			return self.keepdata[card:getClassName()] or sgs.ai_keep_value[card:getClassName()] or 0
		end
	end

	local newvalue = self.keepValue[card:getId()] or self.keepdata[card:getClassName()] or sgs.ai_keep_value[card:getClassName()] or 0
	if cardPlace == sgs.Player_PlaceHand then
		local dec = 0
		for _, acard in ipairs(kept) do
			if isCard(mostvaluable_class, acard, self.player) then
				newvalue = newvalue - 1.2 - dec
				dec = dec + 0.1
			elseif acard:isKindOf("Slash") and card:isKindOf("Slash") then
				newvalue = newvalue - 1.2 - dec
				dec = dec + 0.1
			end
		end
	end
	return newvalue
end

function SmartAI:getUseValue(card)
	local class_name = card:getClassName()
	local v = sgs.ai_use_value[class_name] or 0
	if class_name == "LuaSkillCard" and card:isKindOf("LuaSkillCard") then
		v = sgs.ai_use_value[card:objectName()] or 0
	end

	if card:getTypeId() == sgs.Card_TypeEquip then
		if self.player:hasEquip(card) then
			if card:isKindOf("OffensiveHorse") and self.player:getAttackRange() > 2 then return 5.5 end
			if card:isKindOf("DefensiveHorse") and self:hasEightDiagramEffect() then return 5.5 end
			return 9
		end
		if not self:getSameEquip(card) then v = 6.7 end
		if self.weaponUsed and card:isKindOf("Weapon") then v = 2 end
		if self.player:hasSkills("qiangxi") and card:isKindOf("Weapon") then v = 2 end
		if self.player:hasSkill("kurou") and card:isKindOf("Crossbow") then return 9 end
		if self.player:hasSkills("bazhen|jgyizhong") and card:isKindOf("Armor") then v = 2 end

		if self.player:hasSkills(sgs.lose_equip_skill) then return 10 end
	elseif card:getTypeId() == sgs.Card_TypeBasic then
		if card:isKindOf("Slash") then
			v = sgs.ai_use_value[class_name] or 0
			if self.player:hasFlag("TianyiSuccess") or self:hasHeavySlashDamage(self.player, card) then v = 8.7 end
			if self.player:getPhase() == sgs.Player_Play and self:slashIsAvailable() and #self.enemies > 0 and self:getCardsNum("Slash") == 1 then v = v + 5 end
			if self:hasCrossbowEffect() then v = v + 4 end
			if card:getSkillName() == "Spear" then v = v - 1 end
		elseif card:isKindOf("Jink") then
			if self:getCardsNum("Jink") > 1 then v = v - 6 end
		elseif card:isKindOf("Peach") then
			if self.player:isWounded() then v = v + 6 end
		end
	elseif card:getTypeId() == sgs.Card_TypeTrick then
		if self.player:getWeapon() and not self.player:hasSkills(sgs.lose_equip_skill) and card:isKindOf("Collateral") then v = 2 end
		if card:getSkillName() == "shuangxiong" then v = 6 end
		if card:isKindOf("Duel") then v = v + self:getCardsNum("Slash") * 2 end
		if self.player:hasSkill("jizhi") then v = v + 4 end
	end

	if self.player:hasSkills(sgs.need_kongcheng) then
		if self.player:getHandcardNum() == 1 then v = 10 end
	end
	if self.player:hasWeapon("Halberd") and card:isKindOf("Slash") and self.player:isLastHandCard(card) then v = 10 end
	if self.player:getPhase() == sgs.Player_Play then v = self:adjustUsePriority(card, v) end
	return v
end

function SmartAI:getUsePriority(card)
	local class_name = card:getClassName()
	local v = 0
	if card:isKindOf("EquipCard") then
		if self.player:hasSkills(sgs.lose_equip_skill) then return 15 end
		if card:isKindOf("Armor") and not self.player:getArmor() then v = (sgs.ai_use_priority[class_name] or 0) + 5.2
		elseif card:isKindOf("Weapon") and not self.player:getWeapon() then v = (sgs.ai_use_priority[class_name] or 0) + 3
		elseif card:isKindOf("DefensiveHorse") and not self.player:getDefensiveHorse() then v = 5.8
		elseif card:isKindOf("OffensiveHorse") and not self.player:getOffensiveHorse() then v = 5.5
		end
		return v
	end

	v = sgs.ai_use_priority[class_name] or 0
	if class_name == "LuaSkillCard" and card:isKindOf("LuaSkillCard") then
		v = sgs.ai_use_priority[card:objectName()] or 0
	end
	return self:adjustUsePriority(card, v)
end

function SmartAI:adjustUsePriority(card, v)
	local suits = {"club", "spade", "diamond", "heart"}

	if card:getTypeId() == sgs.Card_TypeSkill then return v end
	if card:getTypeId() == sgs.Card_TypeEquip then return v end

	for _, askill in sgs.qlist(self.player:getVisibleSkillList(true)) do
		local callback = sgs.ai_suit_priority[askill:objectName()]
		if type(callback) == "function" then
			suits = callback(self, card):split("|")
			break
		elseif type(callback) == "string" then
			suits = callback:split("|")
			break
		end
	end

	table.insert(suits, "no_suit")
	if card:isKindOf("Slash") then
		if card:getSkillName() == "Spear" then v = v - 0.1 end
		if card:isRed() then
			v = v - 0.05
		end
		if card:isKindOf("NatureSlash") then
			if self.slashAvail == 1 then
				v = v + 0.05
				if card:isKindOf("FireSlash") then
					for _, enemy in ipairs(self.enemies) do
						if enemy:hasArmorEffect("Vine") or enemy:getMark("@gale") > 0 then v = v + 0.07 break end
					end
				elseif card:isKindOf("ThunderSlash") then
					for _, enemy in ipairs(self.enemies) do
						if enemy:getMark("@fog") > 0 then v = v + 0.06 break end
					end
				end
			else v = v - 0.05
			end
		end
		if self.player:hasSkill("jiang") and card:isRed() then v = v + 0.21 end

	end

	local suits_value = {}
	for index, suit in ipairs(suits) do
		suits_value[suit] = -index
	end
	v = v + (suits_value[card:getSuitString()] or 0) / 1000
	v = v + (13 - card:getNumber()) / 10000
	return v
end

function SmartAI:getDynamicUsePriority(card)
	if not card then return 0 end
	if card:hasFlag("AIGlobal_KillOff") then return 15 end

	if card:isKindOf("Slash") then
		for _, p in ipairs(self.friends) do
			if p:hasShownSkill("yongjue") and self.player:isFriendWith(p) then return 12 end
		end
	elseif card:isKindOf("AmazingGrace") then
		local zhugeliang = sgs.findPlayerByShownSkillName("kongcheng")
		if zhugeliang and self:isEnemy(zhugeliang) and zhugeliang:isKongcheng() then
			return math.max(sgs.ai_use_priority.Slash, sgs.ai_use_priority.Duel) + 0.1
		end
	elseif card:isKindOf("Peach") and self.player:hasSkill("kuanggu") then return 1.01
	elseif card:isKindOf("DelayedTrick") and #card:getSkillName() > 0 then
		return (sgs.ai_use_priority[card:getClassName()] or 0.01) - 0.01
	elseif card:isKindOf("Duel") then
		if self:hasCrossbowEffect()
			or self.player:canSlashWithoutCrossbow()
			or sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_Residue, self.player, sgs.cloneCard("slash")) > 0
			or self.player:hasUsed("FenxunCard") then
			return sgs.ai_use_priority.Slash - 0.1
		end
	elseif card:isKindOf("AwaitExhausted") and self.player:hasSkills("guose|duanliang") then
		return 0
	end

	local value = self:getUsePriority(card) or 0
	if card:getTypeId() == sgs.Card_TypeEquip then
		if self.player:hasSkills("xiaoji+qixi") and self:getSameEquip(card) then return 3 end
		if self.player:hasSkills(sgs.lose_equip_skill) then value = value + 12 end
		if card:isKindOf("Weapon") and self.player:getPhase() == sgs.Player_Play and #self.enemies > 0 then
			self:sort(self.enemies)
			local enemy = self.enemies[1]
			local v, inAttackRange = self:evaluateWeapon(card, self.player, enemy) / 20
			value = value + string.format("%3.2f", v)
			if inAttackRange then value = value + 0.5 end
		end
	end

	if card:isKindOf("AmazingGrace") then
		local dynamic_value = 10
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			dynamic_value = dynamic_value - 1
			if self:isEnemy(player) then dynamic_value = dynamic_value - ((player:getHandcardNum() + player:getHp()) / player:getHp()) * dynamic_value
			else dynamic_value = dynamic_value + ((player:getHandcardNum() + player:getHp()) / player:getHp()) * dynamic_value
			end
		end
		value = value + dynamic_value
	elseif card:isKindOf("ArcheryAttack") and self.player:hasSkill("luanji") then
		value = value + 5.5
	elseif card:isKindOf("Duel") and self.player:hasSkill("shuangxiong") then
		value = value + 6.2
	elseif card:isKindOf("WendaoCard") and self.player:hasShownSkills("wendao+hongfa") and not self.player:getPile("heavenly_army"):isEmpty()
		and self.player:getArmor() and self.player:getArmor():objectName() == "PeaceSpell" then
		value = value + 8
	end

	return value
end

function SmartAI:cardNeed(card)
	if not self.friends then self.room:writeToConsole(debug.traceback()) self.room:writeToConsole(sgs.turncount) return end
	local class_name = card:getClassName()
	local suit_string = card:getSuitString()
	local value
	if card:isKindOf("Peach") then
		self:sort(self.friends,"hp")
		if self.friends[1]:getHp() < 2 then return 10 end
		if (self.player:getHp() < 3 or self.player:getLostHp() > 1 and not self.player:hasSkill("buqu")) or self.player:hasSkills("kurou|benghuai") then return 14 end
		return self:getUseValue(card)
	end
	if self:isWeak() and card:isKindOf("Jink") and self:getCardsNum("Jink") < 1 then return 12 end

	local i = 0
	for _, askill in sgs.qlist(self.player:getVisibleSkillList(true)) do
		if sgs[askill:objectName() .. "_keep_value"] then
			local v = sgs[askill:objectName() .. "_keep_value"][class_name]
			if v then
				i = i + 1
				if value then value = value + v else value = v end
			end
		end
	end
	if value then return value / i + 4 end
	i = 0
	for _, askill in sgs.qlist(self.player:getVisibleSkillList(true)) do
		if sgs[askill:objectName() .. "_suit_value"] then
			local v = sgs[askill:objectName() .. "_suit_value"][suit_string]
			if v then
				i = i + 1
				if value then value = value + v else value = v end
			end
		end
	end
	if value then return value / i + 4 end

	if card:isKindOf("Slash") and self:getCardsNum("Slash") == 0 then return 5.9 end
	if card:isKindOf("Analeptic") then
		if self.player:getHp() < 2 then return 10 end
	end
	if card:isKindOf("Slash") and (self:getCardsNum("Slash") > 0) then return 4 end
	if card:isKindOf("Crossbow") and self.player:hasSkills("luoshen|kurou|keji|wusheng") then return 20 end
	if card:isKindOf("Axe") and self.player:hasSkill("luoyi") then return 15 end
	if card:isKindOf("Weapon") and (not self.player:getWeapon()) and (self:getCardsNum("Slash") > 1) then return 6 end
	if card:isKindOf("Nullification") and self:getCardsNum("Nullification") == 0 then
		if self:willSkipPlayPhase() or self:willSkipDrawPhase() then return 10 end
		for _, friend in ipairs(self.friends) do
			if self:willSkipPlayPhase(friend) or self:willSkipDrawPhase(friend) then return 9 end
		end
		return 6
	end
	return self:getUseValue(card)
end

-- compare functions
sgs.ai_compare_funcs = {
	hp = function(a, b)
		local c1 = a:getHp()
		local c2 = b:getHp()
		if c1 == c2 then
			return sgs.ai_compare_funcs.defense(a, b)
		else
			return c1 < c2
		end
	end,

	handcard = function(a, b)
		local c1 = a:getHandcardNum()
		local c2 = b:getHandcardNum()
		if c1 == c2 then
			return sgs.ai_compare_funcs.defense(a, b)
		else
			return c1 < c2
		end
	end,

	handcard_defense = function(a, b)
		local c1 = a:getHandcardNum()
		local c2 = b:getHandcardNum()
		if c1 == c2 then
			return  sgs.ai_compare_funcs.defense(a, b)
		else
			return c1 < c2
		end
	end,

	value = function(a, b)
		return sgs.getValue(a) < sgs.getValue(b)
	end,

	defense = function(a, b)
		local def_a, def_b = sgs.getDefenseSlash(a) , sgs.getDefenseSlash(b)
		if not def_a or not def_b then global_room:writeToConsole(debug.traceback()) return true end
		return def_a < def_b
	end,

}

function SmartAI:adjustKeepValue(card, v)
	local suits = {"club", "spade", "diamond", "heart"}
	for _, askill in sgs.qlist(self.player:getVisibleSkillList(true)) do
		local callback = sgs.ai_suit_priority[askill:objectName()]
		if type(callback) == "function" then
			suits = callback(self, card):split("|")
			break
		elseif type(callback) == "string" then
			suits = callback:split("|")
			break
		end
	end
	table.insert(suits, "no_suit")

	if card:isKindOf("Slash") then
		if card:isRed() then v = v + 0.02 end
		if card:isKindOf("NatureSlash") then v = v + 0.03 end
		if self.player:hasSkill("jiang") and card:isRed() then v = v + 0.04 end
	elseif card:isKindOf("HegNullification") then v = v + 0.02
	end

	local suits_value = {}
	for index,suit in ipairs(suits) do
		suits_value[suit] = index * 2
	end
	v = v + (suits_value[card:getSuitString()] or 0) / 100
	v = v + card:getNumber() / 500
	return v
end

function SmartAI:sortByKeepValue(cards, inverse, kept, writeMode)
	if writeMode then
		for _, card in ipairs(cards) do
			local value1 = self:getKeepValue(card, kept, true)
			self.keepValue[card:getId()] = value1
		end
	end

	local compare_func = function(a, b)
		local v1 = self:getKeepValue(a)
		local v2 = self:getKeepValue(b)

		if v1 ~= v2 then
			if inverse then return v1 > v2 end
			return v1 < v2
		else
			if not inverse then return a:getNumber() > b:getNumber() end
			return a:getNumber() < b:getNumber()
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:sortByUseValue(cards, inverse)
	local compare_func = function(a, b)
		local value1 = self:getUseValue(a)
		local value2 = self:getUseValue(b)

		if value1 ~= value2 then
			if not inverse then return value1 > value2 end
			return value1 < value2
		else
			if not inverse then return a:getNumber() > b:getNumber() end
			return a:getNumber() < b:getNumber()
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:sortByUsePriority(cards, player)
	local compare_func = function(a, b)
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
			return a and a:getTypeId() ~= sgs.Card_TypeSkill and not (b and b:getTypeId() ~= sgs.Card_TypeSkill)
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:sortByCardNeed(cards, inverse)
	local compare_func = function(a,b)
		local value1 = self:cardNeed(a)
		local value2 = self:cardNeed(b)

		if value1 ~= value2 then
			if inverse then return value1 > value2 end
			return value1 < value2
		else
			if not inverse then return a:getNumber() > b:getNumber() end
			return a:getNumber() < b:getNumber()
		end
	end

	table.sort(cards, compare_func)
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


function sgs.updateIntentions(from, tos, intention, card)
	for _, to in ipairs(tos) do
		sgs.updateIntention(from, to, intention, card)
	end
end

function SmartAI:isFriend(other, another)
	if not other then self.room:writeToConsole(debug.traceback()) return end
	if another then
		if sgs.gameProcess() == "===" and (sgs.isAnjiang(other) or sgs.isAnjiang(another))
			and other:getMark("KnownBothEnemy" .. another:objectName()) > 0 then
			return false
		end
		local of, af = self:isFriend(other), self:isFriend(another)
		return of ~= nil and af ~= nil and of == af
	end
	if self.player:objectName() == other:objectName() then return true end
	if self.player:isFriendWith(other) then return true end
	local level = self:objectiveLevel(other)
	if level < 0 then return true
	elseif level == 0 then return nil end
	return false
end

function SmartAI:isEnemy(other, another)
	if not other then self.room:writeToConsole(debug.traceback()) return end
	if another then
		if sgs.gameProcess() == "===" and (sgs.isAnjiang(other) or sgs.isAnjiang(another))
			and other:getMark("KnownBothEnemy" .. another:objectName()) > 0 then
			return true
		end
		local of, af = self:isEnemy(other), self:isEnemy(another)
		return of ~= nil and af ~= nil and of ~= af
	end
	if self.player:objectName() == other:objectName() then return false end
	local level = self:objectiveLevel(other)
	if level > 0 then return true
	elseif level == 0 then return nil end
	return false
end


function SmartAI:getFriendsNoself(player)
	player = player or self.player
	local friends_noself = {}
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if self:isFriend(p, player) and p:objectName() ~= player:objectName() then table.insert(friends_noself, p) end
	end
	return friends_noself
end

function SmartAI:getFriends(player)
	player = player or self.player
	local friends = {}
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if self:isFriend(p, player) then table.insert(friends, p) end
	end
	return friends
end

function SmartAI:getEnemies(player)
	local enemies = {}
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if self:isEnemy(p, player) then table.insert(enemies, p) end
	end
	return enemies
end

function SmartAI:sort(players, key)
	if not players then self.room:writeToConsole(debug.traceback()) end
	if #players == 0 then return end
	local func
	if not key or key == "defense" or key == "defenseSlash" then
		func = function(a, b)
			return sgs.getDefenseSlash(a, self) < sgs.getDefenseSlash(b, self)
		end
	elseif key == "hp" then
		func = function(a, b)
			local c1 = a:getHp()
			local c2 = b:getHp()
			if c1 == c2 then
				return sgs.getDefenseSlash(a, self) < sgs.getDefenseSlash(b, self)
			else
				return c1 < c2
			end
		end
	elseif key == "handcard" then
		func = function(a, b)
			local c1 = a:getHandcardNum()
			local c2 = b:getHandcardNum()
			if c1 == c2 then
				return sgs.getDefenseSlash(a, self) < sgs.getDefenseSlash(b, self)
			else
				return c1 < c2
			end
		end
	elseif key == "handcard_defense" then
		func = function(a, b, self)
			local c1 = a:getHandcardNum()
			local c2 = b:getHandcardNum()
			if c1 == c2 then
				return sgs.getDefenseSlash(a, self) < sgs.getDefenseSlash(b, self)
			else
				return c1 < c2
			end
		end
	elseif key == "equip_defense" then
		func = function(a, b, self)
			local c1 = a:getCards("e"):length()
			local c2 = b:getCards("e"):length()
			if c1 == c2 then
				return sgs.getDefenseSlash(a, self) < sgs.getDefenseSlash(b, self)
			else
				return c1 < c2
			end
		end
	else
		func = sgs.ai_compare_funcs[key]
	end

	if not func then self.room:writeToConsole(debug.traceback()) return end

	function _sort(players, key)
		table.sort(players, func)
	end
	if not pcall(_sort, players, key) then self.room:writeToConsole(debug.traceback()) end
end

function sgs.updateAlivePlayerRoles()
	for _, kingdom in ipairs(sgs.KingdomsTable) do
		sgs.current_mode_players[kingdom] = 0
	end
	for _, aplayer in sgs.qlist(global_room:getAllPlayers()) do
		if aplayer:getRole() == "careerist" then continue end
		local kingdom = aplayer:getKingdom()
		if not sgs.current_mode_players[kingdom] then sgs.current_mode_players[kingdom] = 0 end
		sgs.current_mode_players[kingdom] = sgs.current_mode_players[kingdom] + 1
	end
end

function findPlayerByObjectName(name, include_death, except)
	local players = nil
	if include_death then
		players = global_room:getPlayers()
	else
		players = global_room:getAllPlayers()
	end
	if except then
		players:removeOne(except)
	end
	for _,p in sgs.qlist(players) do
		if p:objectName() == name then
			return p
		end
	end
end

function getTrickIntention(trick_class, target)
	local intention = sgs.ai_card_intention[trick_class]
	if type(intention) == "number" then
		return intention
	elseif type(intention == "function") then
		if trick_class == "IronChain" then
			if target and target:isChained() then return -60 else return 60 end
		end
	end
	if trick_class == "Collateral" then return 0 end
	if trick_class == "AwaitExhausted" then return -10 end
	if trick_class == "BefriendAttacking" then return -10 end
	if sgs.dynamic_value.damage_card[trick_class] then
		return 70
	end
	if sgs.dynamic_value.benefit[trick_class] then
		return -40
	end
	if target then
		if trick_class == "Snatch" or trick_class == "Dismantlement" then
			local judgelist = target:getCards("j")
			if not judgelist or judgelist:isEmpty() then
				if not target:hasArmorEffect("SilverLion") or not target:isWounded() then
					return 80
				end
			end
		end
	end
	return 0
end


sgs.ai_choicemade_filter.Nullification.general = function(self, player, promptlist)
	local trick_class = promptlist[2]
	local target_objectName = promptlist[3]
	if string.find(trick_class, "Nullification") then
		if not sgs.nullification_source or not sgs.nullification_intention or type(sgs.nullification_intention) ~= "number" then
			self.room:writeToConsole(debug.traceback())
			return
		end
		sgs.nullification_level = sgs.nullification_level + 1
		if sgs.nullification_level % 2 == 0 then
			sgs.updateIntention(player, sgs.nullification_source, sgs.nullification_intention)
		elseif sgs.nullification_level % 2 == 1 then
			sgs.updateIntention(player, sgs.nullification_source, -sgs.nullification_intention)
		end
	else
		sgs.nullification_source = findPlayerByObjectName(target_objectName)
		sgs.nullification_level = 1
		sgs.nullification_intention = getTrickIntention(trick_class, sgs.nullification_source)
		if player:objectName() ~= target_objectName then
			sgs.updateIntention(player, sgs.nullification_source, -sgs.nullification_intention)
		end
	end
end

sgs.ai_choicemade_filter.playerChosen.general = function(self, from, promptlist)
	if from:objectName() == promptlist[3] then return end
	local reason = string.gsub(promptlist[2], "%-", "_")
	local to = findPlayerByObjectName(promptlist[3])
	local callback = sgs.ai_playerchosen_intention[reason]
	if callback then
		if type(callback) == "number" then
			sgs.updateIntention(from, to, sgs.ai_playerchosen_intention[reason])
		elseif type(callback) == "function" then
			callback(self, from, to)
		end
	end
end

sgs.ai_choicemade_filter.viewCards.general = function(self, from, promptlist)
	local to = findPlayerByObjectName(promptlist[#promptlist])
	if to and not to:isKongcheng() then
		local flag = string.format("%s_%s_%s", "visible", from:objectName(), to:objectName())
		for _, card in sgs.qlist(to:getHandcards()) do
			if not card:hasFlag("visible") then card:setFlags(flag) end
		end
	end
end

sgs.ai_choicemade_filter.Yiji.general = function(self, from, promptlist)
	local from = findPlayerByObjectName(promptlist[3])
	local to = findPlayerByObjectName(promptlist[4])
	local reason = promptlist[2]
	local cards = {}
	local card_ids = promptlist[5]:split("+")
	for _, id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(tonumber(id))
		table.insert(cards, card)
	end
	if from and to then
		local callback = sgs.ai_Yiji_intention[reason]
		if callback then
			if type(callback) == "number" and not (self:needKongcheng(to, true) and #cards == 1) then
				sgs.updateIntention(from, to, sgs.ai_Yiji_intention[reason])
			elseif type(callback) == "function" then
				callback(self, from, to, cards)
			end
		elseif not (self:needKongcheng(to, true) and #cards == 1) then
			sgs.updateIntention(from, to, -10)
		end
	end
end

function SmartAI:filterEvent(event, player, data)
	if not sgs.recorder then
		sgs.recorder = self
		self.player:speak(version)
	end
	if player:objectName() == self.player:objectName() then
		if sgs.debugmode and type(sgs.ai_debug_func[event]) == "table" then
			for _, callback in pairs(sgs.ai_debug_func[event]) do
				if type(callback) == "function" then callback(self, player, data) end
			end
		end
		if type(sgs.ai_chat_func[event]) == "table" and sgs.GetConfig("AIChat", false) and player:getState() == "robot" then
			for _, callback in pairs(sgs.ai_chat_func[event]) do
				if type(callback) == "function" then callback(self, player, data) end
			end
		end
		if type(sgs.ai_event_callback[event]) == "table" then
			for _, callback in pairs(sgs.ai_event_callback[event]) do
				if type(callback) == "function" then callback(self, player, data) end
			end
		end
	end

	if sgs.DebugMode_Niepan and event == sgs.AskForPeaches and self.room:getCurrentDyingPlayer():objectName() == self.player:objectName() then endlessNiepan(self, data:toDying().who) end

	sgs.lastevent = event
	sgs.lasteventdata = data
	if event == sgs.ChoiceMade and (self == sgs.recorder or self.player:objectName() == sgs.recorder.player:objectName()) then
		local carduse = data:toCardUse()
		if carduse and carduse.card ~= nil then
			for _, callback in ipairs(sgs.ai_choicemade_filter.cardUsed) do
				if type(callback) == "function" then
					callback(self, player, carduse)
				end
			end
		elseif data:toString() then
			promptlist = data:toString():split(":")
			local callbacktable = sgs.ai_choicemade_filter[promptlist[1]]
			if callbacktable and type(callbacktable) == "table" then
				local index = 2
				if promptlist[1] == "cardResponded" then

					if promptlist[2]:match("jink") then sgs.card_lack[player:objectName()]["Jink"] = promptlist[#promptlist] == "_nil_" and 1 or 0
					elseif promptlist[2]:match("slash") then sgs.card_lack[player:objectName()]["Slash"] = promptlist[#promptlist] == "_nil_" and 1 or 0
					elseif promptlist[2]:match("peach") then sgs.card_lack[player:objectName()]["Peach"] = promptlist[#promptlist] == "_nil_" and 1 or 0
					end

					index = 3
				end
				local callback = callbacktable[promptlist[index]] or callbacktable.general
				if type(callback) == "function" then
					callback(self, player, promptlist)
				end
			end

		end
	elseif event == sgs.CardFinished or event == sgs.GameStart or event == sgs.EventPhaseStart or event == sgs.RemoveStateChanged then
		self:updatePlayers(self == sgs.recorder)
	elseif event == sgs.BuryVictim or event == sgs.HpChanged or event == sgs.MaxHpChanged then
		self:updatePlayers(self == sgs.recorder)
	end

	if event == sgs.BuryVictim then
		if self == sgs.recorder then sgs.updateAlivePlayerRoles() sgs.debugFunc(player, 3) end
	end

	if self.player:objectName() == player:objectName() and event == sgs.AskForPeaches then
		local dying = data:toDying()
		if self:isFriend(dying.who) and dying.who:getHp() < 1 then
			sgs.card_lack[player:objectName()]["Peach"]=1
		end
	end
	if self.player:objectName() == player:objectName() and player:getPhase() ~= sgs.Player_Play and event == sgs.CardsMoveOneTime then
		local move = data:toMoveOneTime()
		if move.to and move.to:objectName() == player:objectName() and move.to_place == sgs.Player_PlaceHand and player:getHandcardNum() > 1 then
			self:assignKeep()
		end
	end

	if self ~= sgs.recorder then return end

	if event == sgs.GeneralShown then
		self:updatePlayerKingdom(player, data)
	elseif event == sgs.TargetConfirmed then
		local struct = data:toCardUse()
		local from = struct.from
		local card = struct.card
		local tos = sgs.QList2Table(struct.to)
		if from and from:objectName() == player:objectName() then
			local callback = sgs.ai_card_intention[card:getClassName()]
			if callback then
				if type(callback) == "function" then
					callback(self, card, from, tos)
				elseif type(callback) == "number" then
					sgs.updateIntentions(from, tos, callback, card)
				end
			end
			-- AI Chat
			speakTrigger(card, from, tos)
			if card:getClassName() == "LuaSkillCard" and card:isKindOf("LuaSkillCard") then
				local luacallback = sgs.ai_card_intention[card:objectName()]
				if luacallback then
					if type(luacallback) == "function" then
						luacallback(self, card, from, tos)
					elseif type(luacallback) == "number" then
						sgs.updateIntentions(from, tos, luacallback, card)
					end
				end
			end
		end

--[[
		if from and sgs.isAnjiang(from) and string.find("FireAttack|Dismantlement|Snatch|Slash|Duel", card:getClassName())
			and from:objectName() == player:objectName() then
			local unknown = true
			for _, to in ipairs(tos) do
				if self:evaluateKingdom(to, from) ~= "unknown" then
					unknown = false
					break
				end
			end
			if unknown then
				local targets = self:exclude(self.room:getOtherPlayers(from), card, from)
				for _, who in ipairs(targets) do
					if not sgs.isAnjiang(who)
						and (card:isKindOf("FireAttack")
							or ((card:isKindOf("Dismantlement") or card:isKindOf("Snatch"))
								and not self:needToThrowArmor(who) and not who:hasShownSkill("tuntian")
								and who:getCards("j"):isEmpty()
								and not (who:getCards("e"):length() > 0 and self:hasSkills(sgs.lose_equip_skill, who))
								and not (self:needKongcheng(who) and who:getHandcardNum() == 1))
							or (card:isKindOf("Slash") and not (self:getDamagedEffects(who, player, true) or self:needToLoseHp(who, player, true, true))
								and not ((who:hasShownSkill("leiji")) and getCardsNum("Jink", who, from) > 0))
							or (card:isKindOf("Duel") and card:getSkillName() ~= "lijian" and card:getSkillName() ~= "noslijian"
								and not (self:getDamagedEffects(who, player) or self:needToLoseHp(who, player, nil, true, true))))
						then
						sgs.updateIntention(from, who, -10)
					end
				end
			end
		end
]]

		if card:isKindOf("AOE") and self.player:objectName() == player:objectName() then
			for _, t in sgs.qlist(struct.to) do
				if t:hasShownSkill("fangzhu") then sgs.ai_AOE_data = data break end
				if t:hasShownSkill("guidao") and t:hasShownSkill("leiji") and card:isKindOf("ArcheryAttack") then sgs.ai_AOE_data = data break end
			end
		end

	elseif event == sgs.PreDamageDone then
		local damage = data:toDamage()
		local clear = true
		if clear and damage.to:isChained() then
			for _, p in sgs.qlist(self.room:getOtherPlayers(damage.to)) do
				if p:isChained() and damage.nature ~= sgs.DamageStruct_Normal then
					clear = false
					break
				end
			end
		end
		if not clear then
			if damage.nature ~= sgs.DamageStruct_Normal and not damage.chain then
				for _, p in sgs.qlist(self.room:getAlivePlayers()) do
					local added = 0
					if p:objectName() == damage.to:objectName() and p:isChained() and p:getHp() <= damage.damage then
						sgs.ai_NeedPeach[p:objectName()] = damage.damage + 1 - p:getHp()
					elseif p:objectName() ~= damage.to:objectName() and p:isChained() and self:damageIsEffective(p, damage.nature, damage.from) then
						if damage.nature == sgs.DamageStruct_Fire then
							added = p:hasArmorEffect("Vine") and added + 1 or added
							sgs.ai_NeedPeach[p:objectName()] = damage.damage + 1 + added - p:getHp()
						elseif damage.nature == sgs.DamageStruct_Thunder then
							sgs.ai_NeedPeach[p:objectName()] = damage.damage + 1 + added - p:getHp()
						end
					end
				end
			end
		else
			for _, p in sgs.qlist(self.room:getAlivePlayers()) do
				sgs.ai_NeedPeach[p:objectName()] = 0
			end
		end
	elseif event == sgs.CardUsed then
		local struct = data:toCardUse()
		local card = struct.card
		local who
		if not struct.to:isEmpty() then who = struct.to:first() end

		if card:isKindOf("Snatch") or card:isKindOf("Dismantlement") then
			for _, p in sgs.qlist(struct.to) do
				for _, c in sgs.qlist(p:getCards("hej")) do
					self.room:setCardFlag(c, "-AIGlobal_SDCardChosen_"..card:objectName())
				end
			end
		end

		if card:isKindOf("AOE") and sgs.ai_AOE_data then
			sgs.ai_AOE_data = nil
		end

		if card:isKindOf("Collateral") then sgs.ai_collateral = false end

	elseif event == sgs.CardsMoveOneTime then
		local move = data:toMoveOneTime()
		local from = nil   -- convert move.from from const Player * to ServerPlayer *
		local to = nil   -- convert move.to to const Player * to ServerPlayer *
		if move.from then from = findPlayerByObjectName(move.from:objectName(), true) end
		if move.to then to = findPlayerByObjectName(move.to:objectName(), true) end
		local reason = move.reason
		local from_places = sgs.QList2Table(move.from_places)

		for i = 0, move.card_ids:length()-1 do
			local place = move.from_places:at(i)
			local card_id = move.card_ids:at(i)
			local card = sgs.Sanguosha:getCard(card_id)

			if move.to_place == sgs.Player_PlaceHand and to and player:objectName() == to:objectName() then
				if card:hasFlag("visible") then
					if isCard("Slash",card, player) then sgs.card_lack[player:objectName()]["Slash"] = 0 end
					if isCard("Jink",card, player) then sgs.card_lack[player:objectName()]["Jink"] = 0 end
					if isCard("Peach",card, player) then sgs.card_lack[player:objectName()]["Peach"] = 0 end
				else
					sgs.card_lack[player:objectName()]["Slash"] = 0
					sgs.card_lack[player:objectName()]["Jink"] = 0
					sgs.card_lack[player:objectName()]["Peach"] = 0
				end
			end

			if move.to_place == sgs.Player_PlaceHand and to and place ~= sgs.Player_DrawPile then
				if from and player:objectName() == from:objectName()
					and from:objectName() ~= to:objectName() and place == sgs.Player_PlaceHand and not card:hasFlag("visible") then
					local flag = string.format("%s_%s_%s", "visible", from:objectName(), to:objectName())
					global_room:setCardFlag(card_id, flag, from)
				end
			end

			if player:hasFlag("AI_Playing") and player:hasShownSkill("leiji") and player:getPhase() == sgs.Player_Discard and isCard("Jink", card, player)
				and player:getHandcardNum() >= 2 and reason.m_reason == sgs.CardMoveReason_S_REASON_RULEDISCARD then sgs.card_lack[player:objectName()]["Jink"] = 2 end

			if player:hasFlag("AI_Playing") and sgs.turncount <= 3 and player:getPhase() == sgs.Player_Discard
				and reason.m_reason == sgs.CardMoveReason_S_REASON_RULEDISCARD and sgs.isAnjiang(player) then

				for _, target in sgs.qlist(self.room:getOtherPlayers(player)) do
					if isCard("Slash", card, player) then
						local has_slash_prohibit_skill = false
						for _, askill in sgs.qlist(target:getVisibleSkillList(true)) do
							local s_name = askill:objectName()
							local filter = sgs.ai_slash_prohibit[s_name]
							if filter and type(filter) == "function" and not (s_name == "tiandu" or s_name == "hujia" or s_name == "huilei" or s_name == "weidi") then
								if s_name == "xiangle" then
									local basic_num = 0
									for _, c_id in sgs.qlist(move.card_ids) do
										local c = sgs.Sanguosha:getCard(c_id)
										if c:isKindOf("BasicCard") then
											basic_num = basic_num + 1
										end
									end
									if basic_num < 2 then has_slash_prohibit_skill = true break end
								else
									has_slash_prohibit_skill = true
									break
								end
							end
						end

						if target:hasShownSkill("fangzhu") and target:getLostHp() < 2 then
							has_slash_prohibit_skill = true
						end

						if player:canSlash(target, card, true) and self:slashIsEffective(card, target) and not has_slash_prohibit_skill and sgs.isGoodTarget(target,self.enemies, self) then
							sgs.updateIntention(player, target, -35)
						end
					elseif isCard("Indulgence", card, player) then
						local zhanghe = sgs.findPlayerByShownSkillName("qiaobian")
						if not (zhanghe and sgs.ai_explicit[target:objectName()] == "wei" and self:playerGetRound(zhanghe) <= self:playerGetRound(target)) then
							for _, target in sgs.qlist(self.room:getOtherPlayers(player)) do
								if not target:containsTrick("indulgence") and not target:hasShownSkill("qiaobian") and not sgs.isAnjiang(target) then
									local aplayer = self:exclude( {target}, card, player)
									if #aplayer == 1 then
										sgs.updateIntention(player, target, -35)
									end
								end
							end
						end
					elseif isCard("SupplyShortage", card, player) then
						local zhanghe = sgs.findPlayerByShownSkillName("qiaobian")
						if not (zhanghe and sgs.ai_explicit[target:objectName()] == "wei" and self:playerGetRound(zhanghe) <= self:playerGetRound(target)) then
							for _, target in sgs.qlist(self.room:getOtherPlayers(player)) do
								if not target:containsTrick("supply_shortage") and not target:hasShownSkill("qiaobian") and not sgs.isAnjiang(target) then
									local aplayer = self:exclude( {target}, card, player)
									if #aplayer == 1 then
										sgs.updateIntention(player, target, -35)
									end
								end
							end
						end
					end
				end
			end
		end

	elseif event == sgs.EventPhaseEnd and player:getPhase() == sgs.Player_Player then
		player:setFlags("AI_Playing")
	elseif event == sgs.EventPhaseStart then
		if player:getPhase() == sgs.Player_RoundStart then
			if not sgs.ai_setSkillsPreshowed then
				self:setSkillsPreshowed()
				sgs.ai_setSkillsPreshowed = true
			end
			if player:getAI() then player:setSkillsPreshowed("hd", true) end
			-- sgs.printFEList(player)
			sgs.debugFunc(player, 3)
		elseif player:getPhase() == sgs.Player_NotActive then
			if sgs.recorder.player:objectName() == player:objectName() then sgs.turncount = sgs.turncount + 1 end
		end
	end
end

function SmartAI:askForSuit(reason)
	if not reason then return sgs.ai_skill_suit.fanjian(self) end -- this line is kept for back-compatibility
	local callback = sgs.ai_skill_suit[reason]
	if type(callback) == "function" then
		if callback(self) then return callback(self) end
	end
	return math.random(0, 3)
end

function SmartAI:askForSkillInvoke(skill_name, data)
	skill_name = string.gsub(skill_name, "%-", "_")
	local invoke = sgs.ai_skill_invoke[skill_name]
	if type(invoke) == "boolean" then
		return invoke
	elseif type(invoke) == "function" then
		if invoke(self, data) == true then
			return true
		else
			return false
		end
	else
		local skill = sgs.Sanguosha:getSkill(skill_name)
		if skill and skill:getFrequency() == sgs.Skill_Frequent then
			return true
		end
	end
	return nil
end

function SmartAI:askForChoice(skill_name, choices, data)
	local choice = sgs.ai_skill_choice[skill_name]
	if type(choice) == "string" then
		return choice
	elseif type(choice) == "function" then
		return choice(self, choices, data)
	else
		local choice_table = choices:split("+")
		for index, achoice in ipairs(choice_table) do
			if achoice == "benghuai" then table.remove(choice_table, index) break end
		end
		local r = math.random(1, #choice_table)
		return choice_table[r]
	end
end

function SmartAI:askForDiscard(reason, discard_num, min_num, optional, include_equip)
	min_num = min_num or discard_num
	local exchange = self.player:hasFlag("Global_AIDiscardExchanging")
	local callback = sgs.ai_skill_discard[reason]
	self:assignKeep(true)
	if type(callback) == "function" then
		local cb = callback(self, discard_num, min_num, optional, include_equip)
		if cb then
			if type(cb) == "number" and not self.player:isJilei(sgs.Sanguosha:getCard(cb)) then return { cb }
			elseif type(cb) == "table" then
				for _, card_id in ipairs(cb) do
					if not exchange and self.player:isJilei(sgs.Sanguosha:getCard(card_id)) then
						return {}
					end
				end
				return cb
			end
			return {}
		end
	elseif optional then
		return min_num == 1 and self:needToThrowArmor() and self.player:getArmor():getEffectiveId() or {}
	end

	local flag = "h"
	if include_equip and (self.player:getEquips():isEmpty() or not self.player:isJilei(self.player:getEquips():first())) then flag = flag .. "e" end
	local cards = self.player:getCards(flag)
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	local to_discard = {}

	local least = min_num
	if discard_num - min_num > 1 then
		least = discard_num - 1
	end
	local temp, discardEquip = {}
	for _, card in ipairs(cards) do
		if exchange or not self.player:isJilei(card) then
			place = self.room:getCardPlace(card:getEffectiveId())
			if discardEquip and place == sgs.Player_PlaceEquip then
				table.insert(temp, card:getEffectiveId())
			elseif self:getKeepValue(card) >= 4.1 then
				table.insert(temp, card:getEffectiveId())
			else
				table.insert(to_discard, card:getEffectiveId())
			end
			if self.player:hasSkills(sgs.lose_equip_skill) and place == sgs.Player_PlaceEquip then discardEquip = true end
		end
		if #to_discard >= discard_num then break end
	end
	if #to_discard < discard_num then
		for _, id in ipairs(temp) do
			table.insert(to_discard, id)
			if #to_discard >= discard_num then break end
		end
	end

	return to_discard
end

sgs.ai_skill_discard.gamerule = function(self, discard_num)

	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	local to_discard = {}
	for _, card in ipairs(cards) do
		if not self.player:isCardLimited(card, sgs.Card_MethodDiscard, true) then
			table.insert(to_discard, card:getId())
		end
		if #to_discard >= discard_num or self.player:isKongcheng() then break end
	end

	return to_discard
end


function SmartAI:askForNullification(trick, from, to, positive)
	if self.player:isDead() then return nil end
	local null_card = self:getCardId("Nullification")
	local null_num = self:getCardsNum("Nullification")
	if null_card then null_card = sgs.Card_Parse(null_card) else return nil end
	assert(null_card)
	if self.player:isLocked(null_card) then return nil end
	if (from and from:isDead()) or (to and to:isDead()) then return nil end

	if trick:isKindOf("FireAttack") then
		if to:isKongcheng() or from:isKongcheng() then return nil end
		if self.player:objectName() == from:objectName() and self.player:getHandcardNum() == 1 and self.player:handCards():first() == null_card:getId() then return nil end
	end

	if ("snatch|dismantlement"):match(trick:objectName()) and to:isAllNude() then return nil end

	if from then
		if (trick:isKindOf("Duel") or trick:isKindOf("FireAttack") or trick:isKindOf("AOE")) and self:getDamagedEffects(to, from) and self:isFriend(to) then
			return nil
		end
		if (trick:isKindOf("Duel") or trick:isKindOf("AOE")) and not self:damageIsEffective(to, sgs.DamageStruct_Normal) then return nil end
		if trick:isKindOf("FireAttack") and not self:damageIsEffective(to, sgs.DamageStruct_Fire) then return nil end
	end
	if (trick:isKindOf("Duel") or trick:isKindOf("FireAttack") or trick:isKindOf("AOE")) and self:needToLoseHp(to, from) and self:isFriend(to) then
		return nil
	end

	local callback = sgs.ai_nullification[trick:getClassName()]
	if type(callback) == "function" then
		local shouldUse = callback(self, trick, from, to, positive)
		if shouldUse then return null_card end
	end

	if positive then

		if from and (trick:isKindOf("FireAttack") or trick:isKindOf("Duel") or trick:isKindOf("AOE")) and self:cantbeHurt(to, from) then
			if self:isFriend(from) then return null_card end
			return
		end

		local isEnemyFrom = from and self:isEnemy(from)

		if isEnemyFrom and self.player:hasSkill("kongcheng") and self.player:getHandcardNum() == 1 and self.player:isLastHandCard(null_card) and trick:isKindOf("SingleTargetTrick") then
			return null_card
		elseif trick:isKindOf("ExNihilo") then
			if isEnemyFrom and self:evaluateKingdom(from) ~= "unknown" and (self:isWeak(from) or from:hasShownSkills(sgs.cardneed_skill)) then
				return null_card
			end
		elseif trick:isKindOf("Snatch") then
			if to:containsTrick("indulgence") or to:containsTrick("supply_shortage") and self:isFriend(to) and to:isNude() then return nil end
			if isEnemyFrom and self:isFriend(to, from) and to:getCards("j"):length() > 0 then
				return null_card
			elseif self:isFriend(to) then return null_card
			end
		elseif trick:isKindOf("Dismantlement") then
			if to:containsTrick("indulgence") or to:containsTrick("supply_shortage") and self:isFriend(to) and to:isNude() then return nil end
			if isEnemyFrom and self:isFriend(to, from) and to:getCards("j"):length() > 0 then
				return null_card
			end
			if self:isFriend(to) then
				if self:getDangerousCard(to) or self:getValuableCard(to) then return null_card end
				if to:getHandcardNum() == 1 and not self:needKongcheng(to) then
					if (getKnownCard(to, self.player, "TrickCard", false) == 1 or getKnownCard(to, self.player, "EquipCard", false) == 1 or getKnownCard(to, self.player, "Slash", false) == 1) then
						return nil
					end
					return null_card
				end
			end
		elseif trick:isKindOf("IronChain") then
			if isEnemyFrom and self:isFriend(to) then return to:hasArmorEffect("Vine") and null_card end
		elseif trick:isKindOf("Duel") then
			if trick:getSkillName() == "lijian" then
				if self:isFriend(to) and (self:isWeak(to) or null_num > 1 or self:getOverflow() or not self:isWeak()) then return null_card end
				return
			end
			if isEnemyFrom and self:isFriend(to) then
				if self:isWeak(to) then return null_card
				elseif self.player:objectName() == to:objectName() then
					if self:getCardsNum("Slash") > getCardsNum("Slash", from, self.player) then return
					elseif self.player:hasSkills(sgs.masochism_skill) and
						(self.player:getHp() > 1 or self:getCardsNum("Peach") > 0 or self:getCardsNum("Analeptic") > 0) then
						return nil
					elseif self:getCardsNum("Slash") == 0 then
						return null_card
					end
				end
			end
		elseif trick:isKindOf("FireAttack") then
			if to:isChained() then return not self:isGoodChainTarget(to, from, nil, nil, trick) and null_card end
			if isEnemyFrom and self:isFriend(to) then
				if from:getHandcardNum() > 2 or self:isWeak(to) or to:hasArmorEffect("Vine") or to:getMark("@gale") > 0 then
					return null_card
				end
			end
		elseif trick:isKindOf("Indulgence") then
			if self:isFriend(to) and not to:isSkipped(sgs.Player_Play) then
				if (to:hasShownSkill("guanxing") or to:hasShownSkill("yizhi") and to:inDeputySkills("yizhi"))
					and (global_room:alivePlayerCount() > 4 or to:hasShownSkill("yizhi")) then return end
				if to:getHp() - to:getHandcardNum() >= 2 then return nil end
				if to:hasShownSkill("tuxi") and to:getHp() > 2 then return nil end
				if to:hasShownSkill("qiaobian") and not to:isKongcheng() then return nil end
				return null_card
			end
		elseif trick:isKindOf("SupplyShortage") then
			if self:isFriend(to) and not to:isSkipped(sgs.Player_Draw) then
				if (to:hasShownSkill("guanxing") or to:hasShownSkill("yizhi") and to:inDeputySkills("yizhi"))
					and (global_room:alivePlayerCount() > 4 or to:hasShownSkill("yizhi")) then return end
				if to:hasShownSkills("guidao|tiandu") then return nil end
				if to:hasShownSkill("qiaobian") and not to:isKongcheng() then return nil end
				return null_card
			end

		elseif trick:isKindOf("ArcheryAttack") then
			if self:isFriend(to) then
				local heg_null_card = self:getCardId("HegNullification")
				if heg_null_card then
					for _, friend in ipairs(self.friends) do
						if self:playerGetRound(to) < self:playerGetRound(friend) and (self:aoeIsEffective(trick, to, from) or self:getDamagedEffects(to, from)) then
						else
							return heg_null_card
						end
					end
				end
				if not self:aoeIsEffective(trick, to, from) then return
				elseif self:getDamagedEffects(to, from) then return
				elseif to:objectName() == self.player:objectName() and self:canAvoidAOE(trick) then return
				elseif getKnownCard(to, self.player, "Jink", true, "he") >= 1 and to:getHp() > 1 then return
				elseif not self:isFriendWith(to) and self:playerGetRound(to) < self:playerGetRound(self.player) and self:isWeak() then return
				else
					return null_card
				end
			end
		elseif trick:isKindOf("SavageAssault") then
			if self:isFriend(to) then
				local menghuo
				for _, p in sgs.qlist(self.room:getAlivePlayers()) do
					if p:hasShownSkill("huoshou") then menghuo = p break end
				end
				local heg_null_card = self:getCardId("HegNullification")
				if heg_null_card then
					for _, friend in ipairs(self.friends) do
						if self:playerGetRound(to) < self:playerGetRound(friend)
							and (self:aoeIsEffective(trick, to, menghuo or from) or self:getDamagedEffects(to, menghuo or from)) then
						else
							return heg_null_card
						end
					end
				end
				if not self:aoeIsEffective(trick, to, menghuo or from) then return
				elseif self:getDamagedEffects(to, menghuo or from) then return
				elseif to:objectName() == self.player:objectName() and self:canAvoidAOE(trick) then return
				elseif getKnownCard(to, self.player, "Slash", true, "he") >= 1 and to:getHp() > 1 then return
				elseif not self:isFriendWith(to) and self:playerGetRound(to) < self:playerGetRound(self.player) and self:isWeak() then return
				else
					return null_card
				end
			end
		elseif trick:isKindOf("AmazingGrace") then
			if self:isEnemy(to) then
				local NP = self.room:nextPlayer(to)
				if self:isFriend(NP) then
					local ag_ids = self.room:getTag("AmazingGrace"):toStringList()
					local peach_num, exnihilo_num, snatch_num, analeptic_num, crossbow_num = 0, 0, 0, 0, 0
					for _, ag_id in ipairs(ag_ids) do
						local ag_card = sgs.Sanguosha:getCard(ag_id)
						if ag_card:isKindOf("Peach") then peach_num = peach_num + 1 end
						if ag_card:isKindOf("ExNihilo") then exnihilo_num = exnihilo_num + 1 end
						if ag_card:isKindOf("Snatch") then snatch_num = snatch_num + 1 end
						if ag_card:isKindOf("Analeptic") then analeptic_num = analeptic_num + 1 end
						if ag_card:isKindOf("Crossbow") then crossbow_num = crossbow_num + 1 end
					end
					if (peach_num == 1) or (peach_num > 0 and (self:isWeak(to) or self:getOverflow(NP) < 1)) then
						return null_card
					end
					if peach_num == 0 and not self:willSkipPlayPhase(NP) then
						if exnihilo_num > 0 then
							if NP:hasShownSkills("jizhi|rende|zhiheng") then return null_card end
						else
							for _, enemy in ipairs(self.enemies) do
								if snatch_num > 0 and to:distanceTo(enemy) == 1 and
									(self:willSkipPlayPhase(enemy, true) or self:willSkipDrawPhase(enemy, true)) then
									return null_card
								elseif analeptic_num > 0 and (enemy:hasWeapon("Axe") or getCardsNum("Axe", enemy, self.player) > 0) then
									return null_card
								elseif crossbow_num > 0 and getCardsNum("Slash", enemy, self.player) >= 3 then
									local slash = sgs.cloneCard("slash")
									for _, friend in ipairs(self.friends) do
										if enemy:distanceTo(friend) == 1 and self:slashIsEffective(slash, friend, enemy) then
											return null_card
										end
									end
								end
							end
						end
					end
				end
			end
		elseif trick:isKindOf("GodSalvation") then
			if self:isEnemy(to) and self:evaluateKingdom(to) ~= "unknown" and self:isWeak(to) then return null_card end
		end

	else

		if (trick:isKindOf("FireAttack") or trick:isKindOf("Duel") or trick:isKindOf("AOE")) and self:cantbeHurt(to, from) then
			if isEnemyFrom then return null_card end
		end
		if from and from:objectName() == to:objectName() then
			if self:isFriend(from) then return null_card else return end
		end

		if trick:isKindOf("Duel") then
			if trick:getSkillName() == "lijian" then
				if self:isEnemy(to) and (self:isWeak(to) or null_num > 1 or self:getOverflow() > 0 or not self:isWeak()) then return null_card end
				return
			end
			return from and self:isFriend(from) and not self:isFriend(to) and null_card
		elseif trick:isKindOf("GodSalvation") then
			if self:isFriend(to) and self:isWeak(to) then return null_card end
		elseif trick:isKindOf("AmazingGrace") then
			if self:isFriend(to) then return null_card end
		elseif not (trick:isKindOf("GlobalEffect") or trick:isKindOf("AOE")) then
			if from and self:isFriend(from) and not self:isFriend(to) then
				if ("snatch|dismantlement"):match(trick:objectName()) and to:isNude() then
				elseif trick:isKindOf("FireAttack") and to:isKongcheng() then
				else return null_card end
			end
		end
	end
	return
end

sgs.ai_skill_choice.heg_nullification = "all"

function SmartAI:getCardRandomly(who, flags)
	local cards = who:getCards(flags)
	if cards:isEmpty() then return end
	local r = math.random(0, cards:length() - 1)
	local card = cards:at(r)
	if who:hasArmorEffect("SilverLion") then
		if self:isEnemy(who) and who:isWounded() and card == who:getArmor() then
			if r ~= (cards:length() - 1) then
				card = cards:at(r + 1)
			elseif r > 0 then
				card = cards:at(r - 1)
			end
		end
	end
	return card:getEffectiveId()
end

function SmartAI:askForCardChosen(who, flags, reason, method)
	local isDiscard = (method == sgs.Card_MethodDiscard)
	local cardchosen = sgs.ai_skill_cardchosen[string.gsub(reason, "%-", "_")]
	local card
	if type(cardchosen) == "function" then
		card = cardchosen(self, who, flags, method)
		if type(card) == "number" then return card
		elseif card then return card:getEffectiveId() end
	elseif type(cardchosen) == "number" then
		sgs.ai_skill_cardchosen[string.gsub(reason, "%-", "_")] = nil
		for _, acard in sgs.qlist(who:getCards(flags)) do
			if acard:getEffectiveId() == cardchosen then return cardchosen end
		end
	end

	if ("snatch|dismantlement"):match(reason) then
		local flag = "AIGlobal_SDCardChosen_" .. reason
		local to_choose
		for _, card in sgs.qlist(who:getCards(flags)) do
			if card:hasFlag(flag) then
				card:setFlags("-" .. flag)
				to_choose = card:getId()
				break
			end
		end
		if to_choose then return to_choose end
	end

	if self:isFriend(who) then
		if flags:match("j") and not (who:hasShownSkill("qiaobian") and who:getHandcardNum() > 0) then
			local tricks = who:getCards("j")
			local lightning, indulgence, supply_shortage
			for _, trick in sgs.qlist(tricks) do
				if trick:isKindOf("Lightning") and (not isDiscard or self.player:canDiscard(who, trick:getId())) then
					lightning = trick:getId()
				elseif trick:isKindOf("Indulgence") and (not isDiscard or self.player:canDiscard(who, trick:getId()))  then
					indulgence = trick:getId()
				elseif not trick:isKindOf("Disaster") and (not isDiscard or self.player:canDiscard(who, trick:getId())) then
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
			if who:getArmor() and self:evaluateArmor(who:getArmor(), who) < -5 and (not isDiscard or self.player:canDiscard(who, who:getArmor():getEffectiveId())) then
				return who:getArmor():getEffectiveId()
			end
			if who:hasShownSkills(sgs.lose_equip_skill) and self:isWeak(who) then
				if who:getWeapon() and (not isDiscard or self.player:canDiscard(who, who:getWeapon():getEffectiveId())) then return who:getWeapon():getEffectiveId() end
				if who:getOffensiveHorse() and (not isDiscard or self.player:canDiscard(who, who:getOffensiveHorse():getEffectiveId())) then return who:getOffensiveHorse():getEffectiveId() end
			end
		end
	else
		local dangerous = self:getDangerousCard(who)
		if flags:match("e") and dangerous and (not isDiscard or self.player:canDiscard(who, dangerous)) then return dangerous end
		if flags:match("e") and who:hasArmorEffect("EightDiagram") and (not isDiscard or self.player:canDiscard(who, who:getArmor():getId())) then return who:getArmor():getId() end
		if flags:match("e") and who:hasShownSkills("jijiu|beige|weimu|qingcheng") and not self:doNotDiscard(who, "e", false, 1, reason) then
			if who:getDefensiveHorse() and (not isDiscard or self.player:canDiscard(who, who:getDefensiveHorse():getEffectiveId())) then return who:getDefensiveHorse():getEffectiveId() end
			if who:getArmor() and (not isDiscard or self.player:canDiscard(who, who:getArmor():getEffectiveId())) then return who:getArmor():getEffectiveId() end
			if who:getOffensiveHorse() and (not who:hasShownSkill("jijiu") or who:getOffensiveHorse():isRed()) and (not isDiscard or self.player:canDiscard(who, who:getOffensiveHorse():getEffectiveId())) then
				return who:getOffensiveHorse():getEffectiveId()
			end
			if who:getWeapon() and (not who:hasShownSkill("jijiu") or who:getWeapon():isRed()) and (not isDiscard or self.player:canDiscard(who, who:getWeapon():getEffectiveId())) then
				return who:getWeapon():getEffectiveId()
			end
		end
		if flags:match("e") then
			local valuable = self:getValuableCard(who)
			if valuable and (not isDiscard or self.player:canDiscard(who, valuable)) then
				return valuable
			end
		end
		if flags:match("h") and (not isDiscard or self.player:canDiscard(who, "h")) then
			if who:hasShownSkills("jijiu|qingnang|qiaobian|jieyin|beige")
				and not who:isKongcheng() and who:getHandcardNum() <= 2 and not self:doNotDiscard(who, "h", false, 1, reason) then
				return self:getCardRandomly(who, "h")
			end
			local cards = sgs.QList2Table(who:getHandcards())
			local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), who:objectName())
			if #cards <= 2 and not self:doNotDiscard(who, "h", false, 1, reason) then
				for _, cc in ipairs(cards) do
					if (cc:hasFlag("visible") or cc:hasFlag(flag)) and (cc:isKindOf("Peach") or cc:isKindOf("Analeptic")) then
						return self:getCardRandomly(who, "h")
					end
				end
			end
		end

		if flags:match("j") then
			local tricks = who:getCards("j")
			local lightning
			for _, trick in sgs.qlist(tricks) do
				if trick:isKindOf("Lightning") and (not isDiscard or self.player:canDiscard(who, trick:getId())) then
					lightning = trick:getId()
				end
			end
			if self:hasWizard(self.enemies, true) and lightning then
				return lightning
			end
		end

		if flags:match("h") and not self:doNotDiscard(who, "h") then
			if (who:getHandcardNum() == 1 and sgs.getDefenseSlash(who, self) < 3 and who:getHp() <= 2) or who:hasShownSkills(sgs.cardneed_skill) then
				return self:getCardRandomly(who, "h")
			end
		end

		if flags:match("e") and not self:doNotDiscard(who, "e") then
			if who:getDefensiveHorse() and (not isDiscard or self.player:canDiscard(who, who:getDefensiveHorse():getEffectiveId())) then return who:getDefensiveHorse():getEffectiveId() end
			if who:getArmor() and (not isDiscard or self.player:canDiscard(who, who:getArmor():getEffectiveId())) then return who:getArmor():getEffectiveId() end
			if who:getOffensiveHorse() and (not isDiscard or self.player:canDiscard(who, who:getOffensiveHorse():getEffectiveId())) then return who:getOffensiveHorse():getEffectiveId() end
			if who:getWeapon() and (not isDiscard or self.player:canDiscard(who, who:getWeapon():getEffectiveId())) then return who:getWeapon():getEffectiveId() end
		end

		if flags:match("h") then
			if (not who:isKongcheng() and who:getHandcardNum() <= 2) and not self:doNotDiscard(who, "h", false, 1, reason) then
				return self:getCardRandomly(who, "h")
			end
		end
	end
	return -1
end

function sgs.ai_skill_cardask.nullfilter(self, data, pattern, target)
	if self.player:isDead() then return "." end
	local damage_nature = sgs.DamageStruct_Normal
	local effect
	if type(data) == "userdata" then
		effect = data:toSlashEffect()
		if effect and effect.slash then
			damage_nature = effect.nature
		end
	end
	if effect and self:hasHeavySlashDamage(target, effect.slash, self.player) then return end
	if not self:damageIsEffective(nil, damage_nature, target) then return "." end
	if effect and target and target:hasWeapon("IceSword") and self.player:getCards("he"):length() > 1 then return end
	if self:getDamagedEffects(self.player, target) or self:needToLoseHp() then return "." end

	if self.player:hasSkill("tianxiang") then
		local dmgStr = {damage = 1, nature = damage_nature or sgs.DamageStruct_Normal}
		local willTianxiang = sgs.ai_skill_use["@@tianxiang"](self, dmgStr, sgs.Card_MethodDiscard)
		if willTianxiang ~= "." then return "." end
	end
end

function SmartAI:askForCard(pattern, prompt, data)
	local target, target2
	local parsedPrompt = prompt:split(":")
	local players
	if parsedPrompt[2] then
		local players = self.room:getPlayers()
		players = sgs.QList2Table(players)
		for _, player in ipairs(players) do
			if player:getGeneralName() == parsedPrompt[2] or player:objectName() == parsedPrompt[2] then target = player break end
		end
		if parsedPrompt[3] then
			for _, player in ipairs(players) do
				if player:getGeneralName() == parsedPrompt[3] or player:objectName() == parsedPrompt[3] then target2 = player break end
			end
		end
	end
	local arg, arg2 = parsedPrompt[4], parsedPrompt[5]
	local callback = sgs.ai_skill_cardask[parsedPrompt[1]]
	if type(callback) == "function" then
		local ret = callback(self, data, pattern, target, target2, arg, arg2)
		if ret then return ret end
	end

	if data and type(data) == "number" then return end
	local card
	if pattern == "slash" then
		card = sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) or self:getCardId("Slash") or "."
		if card == "." then sgs.card_lack[self.player:objectName()]["Slash"] = 1 end
	elseif pattern == "jink" then
		card = sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) or self:getCardId("Jink") or "."
		if card == "." then sgs.card_lack[self.player:objectName()]["Jink"] = 1 end
	end
	return card
end

function SmartAI:askForUseCard(pattern, prompt, method)
	local use_func = sgs.ai_skill_use[pattern]
	if use_func then
		return use_func(self, prompt, method) or "."
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

	local ids = card_ids
	local cards = {}
	for _, id in ipairs(ids) do
		table.insert(cards, sgs.Sanguosha:getCard(id))
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("Peach") then return card:getEffectiveId() end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("Indulgence") and not (self:isWeak() and self:getCardsNum("Jink") == 0) then return card:getEffectiveId() end
		if card:isKindOf("AOE") and not (self:isWeak() and self:getCardsNum("Jink") == 0) then return card:getEffectiveId() end
	end
	self:sortByCardNeed(cards, true)

	return cards[1]:getEffectiveId()
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
	if not self:willSkipPlayPhase(to) and self:getUseValue(card) < 6 then
		return card:getNumber() > 10
	end
end

function sgs.ai_cardneed.equip(to, card, self)
	if not self:willSkipPlayPhase(to) then
		return card:getTypeId() == sgs.Card_TypeEquip
	end
end

function sgs.ai_cardneed.weapon(to, card, self)
	if not self:willSkipPlayPhase(to) then
		return card:isKindOf("Weapon")
	end
end

function SmartAI:getEnemyNumBySeat(from, to, target, include_neutral)
	target = target or from
	local players = sgs.QList2Table(self.room:getAllPlayers())
	local to_seat = (to:getSeat() - from:getSeat()) % #players
	local enemynum = 0
	for _, p in ipairs(players) do
		if  (self:isEnemy(target, p) or (include_neutral and not self:isFriend(target, p))) and ((p:getSeat() - from:getSeat()) % #players) < to_seat then
			enemynum = enemynum + 1
		end
	end
	return enemynum
end

function SmartAI:getFriendNumBySeat(from, to)
	local players = sgs.QList2Table(self.room:getAllPlayers())
	local to_seat = (to:getSeat() - from:getSeat()) % #players
	local friendnum = 0
	for _, p in ipairs(players) do
		if self:isFriend(from, p) and ((p:getSeat() - from:getSeat()) % #players) < to_seat then
			friendnum = friendnum + 1
		end
	end
	return friendnum
end

function SmartAI:hasHeavySlashDamage(from, slash, to, getValue)
	from = from or self.room:getCurrent()
	slash = slash or self:getCard("Slash", from)
	to = to or self.player
	if not from or not to then self.room:writeToConsole(debug.traceback()) return false end
	if to:hasArmorEffect("SilverLion") and not IgnoreArmor(from, to) then
		if getValue then return 1
		else return false end
	end
	local dmg = 1
	local fireSlash = slash and (slash:isKindOf("FireSlash") or slash:objectName() == "slash" and from:hasWeapon("Fan"))
	local thunderSlash = slash and slash:isKindOf("ThunderSlash")

	if (slash and slash:hasFlag("drank")) then
		dmg = dmg + 1
	elseif from:getMark("drank") > 0 then
		dmg = dmg + from:getMark("drank")
	end
	if from:hasFlag("luoyi") then dmg = dmg + 1 end
	if from:hasWeapon("GudingBlade") and slash and to:isKongcheng() then dmg = dmg + 1 end
	if to:getMark("@gale") > 0 and fireSlash then dmg = dmg + 1 end
	local jiaren_zidan = sgs.findPlayerByShownSkillName("jgchiying")
	if jiaren_zidan and jiaren_zidan:isFriendWith(to) then
		dmg = 1
	end
	if to:hasArmorEffect("Vine") and not IgnoreArmor(from, to) and fireSlash then
		dmg = dmg + 1
	end

	if getValue then return dmg end
	return (dmg > 1)
end

function SmartAI:needKongcheng(player, keep)
	player = player or self.player
	if keep then return player:isKongcheng() and player:hasShownSkill("kongcheng") end
	if not self:hasLoseHandcardEffective(player) and not player:isKongcheng() then return true end
	return player:hasShownSkills(sgs.need_kongcheng)
end

function SmartAI:getLeastHandcardNum(player)
	player = player or self.player
	local least = 0
	local jwfy = sgs.findPlayerByShownSkillName("shoucheng")
	if least < 1 and jwfy and self:isFriend(jwfy, player) then least = 1 end
	return least
end

function SmartAI:hasLoseHandcardEffective(player)
	player = player or self.player
	return player:getHandcardNum() > self:getLeastHandcardNum(player)
end

function SmartAI:hasCrossbowEffect(player)
	player = player or self.player
	return player:hasWeapon("Crossbow") or player:hasShownSkill("paoxiao")
end

function SmartAI:getCardNeedPlayer(cards, friends_table, skillname)
	cards = cards or sgs.QList2Table(self.player:getHandcards())

	local cardtogivespecial = {}
	local specialnum = 0
	local keptslash = 0
	local friends = {}
	local cmpByAction = function(a,b)
		return a:getRoom():getFront(a, b):objectName() == a:objectName()
	end

	local cmpByNumber = function(a,b)
		return a:getNumber() > b:getNumber()
	end

	local friends_table = friends_table or self.friends_noself
	for _, player in ipairs(friends_table) do
		local exclude = self:needKongcheng(player) or self:willSkipPlayPhase(player)
		if player:hasShownSkills("keji|qiaobian|shensu") or player:getHp() - player:getHandcardNum() >= 3
			or (player:isLord() and self:isWeak(player) and self:getEnemyNumBySeat(self.player, player) >= 1) then
			exclude = false
		end
		if self:objectiveLevel(player) <= -2 and not exclude then
			table.insert(friends, player)
		end
	end

	local AssistTarget = self:AssistTarget()
	if AssistTarget and (self:needKongcheng(AssistTarget, true) or self:willSkipPlayPhase(AssistTarget)) then
		AssistTarget = nil
	end

	for _, player in ipairs(friends) do
		if player:hasShownSkill("jieming") or player:hasShownSkill("jijiu") then
			specialnum = specialnum + 1
		end
	end
	if specialnum > 1 and #cardtogivespecial == 0 and self.player:hasSkill("rende") and self.player:getPhase() == sgs.Player_Play then
		local xunyu = sgs.findPlayerByShownSkillName("jieming")
		local huatuo = sgs.findPlayerByShownSkillName("jijiu")
		local no_distance = self.slash_distance_limit
		local redcardnum = 0
		for _, acard in ipairs(cards) do
			if isCard("Slash", acard, self.player) then
				if self.player:canSlash(xunyu, nil, not no_distance) and self:slashIsEffective(acard, xunyu) then
					keptslash = keptslash + 1
				end
				if keptslash > 0 then
					table.insert(cardtogivespecial, acard)
				end
			elseif isCard("Duel", acard, self.player) then
				table.insert(cardtogivespecial, acard)
			end
		end
		for _, hcard in ipairs(cardtogivespecial) do
			if hcard:isRed() then redcardnum = redcardnum + 1 end
		end
		if self.player:getHandcardNum() > #cardtogivespecial and redcardnum > 0 then
			for _, hcard in ipairs(cardtogivespecial) do
				if hcard:isRed() then return hcard, huatuo end
				return hcard, xunyu
			end
		end
	end

	local cardtogive = {}
	local keptjink = 0
	for _, acard in ipairs(cards) do
		if isCard("Jink", acard, self.player) and keptjink < 1 and not self.player:hasSkill("kongcheng") then
			keptjink = keptjink + 1
		else
			table.insert(cardtogive, acard)
		end
	end

	self:sort(friends, "defense")
	for _, friend in ipairs(friends) do
		if self:isWeak(friend) and friend:getHandcardNum() < 3  then
			for _, hcard in ipairs(cards) do
				if isCard("Peach",hcard,friend) or (isCard("Jink",hcard,friend) and self:getEnemyNumBySeat(self.player,friend)>0) or isCard("Analeptic",hcard,friend) then
					return hcard, friend
				end
			end
		end
	end

	if (skillname == "rende" and self.player:hasSkill("rende") and self.player:isWounded() and self.player:getMark("rende") < 3) and not self.player:hasSkill("kongcheng") then
		if (self.player:getHandcardNum() < 3 and self.player:getMark("rende") == 0 and self:getOverflow() <= 0) then return end
	end

	for _, friend in ipairs(friends) do
		if friend:getHp() <= 2 and friend:faceUp() then
			for _, hcard in ipairs(cards) do
				if (hcard:isKindOf("Armor") and not friend:getArmor() and not friend:hasShownSkills("bazhen|jgyizhong"))
					or (hcard:isKindOf("DefensiveHorse") and not friend:getDefensiveHorse()) then
					return hcard, friend
				end
			end
		end
	end

	self:sortByUseValue(cards, true)
	for _, friend in ipairs(friends) do
		if friend:hasShownSkills("jijiu|jieyin") and friend:getHandcardNum() < 4 then
			for _, hcard in ipairs(cards) do
				if (hcard:isRed() and friend:hasShownSkill("jijiu")) or friend:hasShownSkill("jieyin") then
					return hcard, friend
				end
			end
		end
	end

	self:sortByUseValue(cards, true)
	for _, friend in ipairs(friends) do
		if friend:hasShownSkills("jizhi")  then
			for _, hcard in ipairs(cards) do
				if hcard:isKindOf("TrickCard") then
					return hcard, friend
				end
			end
		end
	end

	self:sortByUseValue(cards, true)
	for _, friend in ipairs(friends) do
		if friend:hasShownSkills("paoxiao")  then
			for _, hcard in ipairs(cards) do
				if hcard:isKindOf("Slash") then
					return hcard, friend
				end
			end
		end
	end

	--Crossbow
	for _, friend in ipairs(friends) do
		if friend:hasShownSkills("longdan|wusheng|keji") and not self:hasCrossbowEffect(friend) and friend:getHandcardNum() >= 2 then
			for _, hcard in ipairs(cards) do
				if hcard:isKindOf("Crossbow") then
					return hcard, friend
				end
			end
		end
	end

	for _, friend in ipairs(friends) do
		if getKnownCard(friend, self.player, "Crossbow") > 0 then
			for _, p in ipairs(self.enemies) do
				if sgs.isGoodTarget(p, self.enemies, self) and friend:distanceTo(p) <= 1 then
					for _, hcard in ipairs(cards) do
						if isCard("Slash", hcard, friend) then
							return hcard, friend
						end
					end
				end
			end
		end
	end

	table.sort(friends, cmpByAction)

	for _, friend in ipairs(friends) do
		if friend:faceUp() then
			local can_slash = false
			for _, p in sgs.qlist(self.room:getOtherPlayers(friend)) do
				if self:isEnemy(p) and sgs.isGoodTarget(p, self.enemies, self) and friend:distanceTo(p) <= friend:getAttackRange() then
					can_slash = true
					break
				end
			end
			local flag = string.format("weapon_done_%s_%s",self.player:objectName(),friend:objectName())
			if not can_slash then
				for _, p in sgs.qlist(self.room:getOtherPlayers(friend)) do
					if self:isEnemy(p) and sgs.isGoodTarget(p, self.enemies, self) and friend:distanceTo(p) > friend:getAttackRange() then
						for _, hcard in ipairs(cardtogive) do
							if hcard:isKindOf("Weapon") and friend:distanceTo(p) <= friend:getAttackRange() + (sgs.weapon_range[hcard:getClassName()] or 0)
									and not friend:getWeapon() and not friend:hasFlag(flag) then
								self.room:setPlayerFlag(friend, flag)
								return hcard, friend
							end
							if hcard:isKindOf("OffensiveHorse") and friend:distanceTo(p) <= friend:getAttackRange() + 1
									and not friend:getOffensiveHorse() and not friend:hasFlag(flag) then
								self.room:setPlayerFlag(friend, flag)
								return hcard, friend
							end
						end
					end
				end
			end

		end
	end

	table.sort(cardtogive, cmpByNumber)

	for _, friend in ipairs(friends) do
		if not self:needKongcheng(friend, true) and friend:faceUp() then
			for _, hcard in ipairs(cardtogive) do
				for _, askill in sgs.qlist(friend:getVisibleSkillList(true)) do
					local callback = sgs.ai_cardneed[askill:objectName()]
					if type(callback)=="function" and callback(friend, hcard, self) then
						return hcard, friend
					end
				end
			end
		end
	end

	self:sort(self.enemies, "defense")
	if #self.enemies > 0 and self.enemies[1]:isKongcheng() and self.enemies[1]:hasShownSkill("kongcheng") then
		for _, acard in ipairs(cardtogive) do
			if acard:isKindOf("Lightning") or acard:isKindOf("Collateral") or (acard:isKindOf("Slash") and self.player:getPhase() == sgs.Player_Play)
				or acard:isKindOf("OffensiveHorse") or acard:isKindOf("Weapon") or acard:isKindOf("AmazingGrace") then
				return acard, self.enemies[1]
			end
		end
	end

	if AssistTarget then
		for _, hcard in ipairs(cardtogive) do
			return hcard, AssistTarget
		end
	end

	self:sort(friends, "defense")
	for _, hcard in ipairs(cardtogive) do
		for _, friend in ipairs(friends) do
			if not self:needKongcheng(friend, true) and not self:willSkipPlayPhase(friend) and friend:hasShownSkills(sgs.priority_skill) then
				if (self:getOverflow() > 0 or self.player:getHandcardNum() > 3) and friend:getHandcardNum() <= 3 then
					return hcard, friend
				end
			end
		end
	end

	local shoulduse = skillname == "rende" and self.player:isWounded() and self.player:hasSkill("rende") and self.player:getMark("rende") < 3

	if #cardtogive == 0 and shoulduse then cardtogive = cards end

	self:sort(friends, "handcard")
	for _, hcard in ipairs(cardtogive) do
		for _, friend in ipairs(friends) do
			if not self:needKongcheng(friend, true) then
				if friend:getHandcardNum() <= 3 and (self:getOverflow() > 0 or self.player:getHandcardNum() > 3 or shoulduse) then
					return hcard, friend
				end
			end
		end
	end


	for _, hcard in ipairs(cardtogive) do
		for _, friend in ipairs(friends) do
			if not self:needKongcheng(friend, true) or #friends == 1 then
				if self:getOverflow() > 0 or self.player:getHandcardNum() > 3 or shoulduse then
					return hcard, friend
				end
			end
		end
	end

	for _, hcard in ipairs(cardtogive) do
		for _, friend in ipairs(friends_table) do
			if (not self:needKongcheng(friend, true) or #friends_table == 1) and (self:getOverflow() > 0 or self.player:getHandcardNum() > 3 or shoulduse) then
				return hcard, friend
			end
		end
	end

end

function SmartAI:askForYiji(card_ids, reason)
	if reason then
		local callback = sgs.ai_skill_askforyiji[string.gsub(reason,"%-","_")]
		if type(callback) == "function" then
			local target, cardid = callback(self, card_ids)
			if target and cardid then return target, cardid end
		end
	end
	return nil, -1
end

function SmartAI:askForPindian(requestor, reason)
	local passive = { "lieren" }
	if self.player:objectName() == requestor:objectName() and not table.contains(passive, reason) then
		if self[reason .. "_card"] then
			return sgs.Sanguosha:getCard(self[reason .. "_card"])
		else
			self.room:writeToConsole("Pindian card for " .. reason .. " not found!!")
			return self:getMaxCard(self.player):getId()
		end
	end
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

	local sameclass, c1 = true
	for _, c2 in ipairs(cards) do
		if not c1 then c1 = c2
		elseif c1:getClassName() ~= c2:getClassName() then sameclass = false end
	end
	if sameclass then
		if self:isFriend(requestor) then return self:getMinCard()
		else return self:getMaxCard() end
	end

	local callback = sgs.ai_skill_pindian[reason]
	if type(callback) == "function" then
		local ret = callback(minusecard, self, requestor, maxcard, mincard)
		if ret then return ret end
	end
	if self:isFriend(requestor) then return mincard else return maxcard end
end

sgs.ai_skill_playerchosen.damage = function(self, targets)
	local targetlist = sgs.QList2Table(targets)
	self:sort(targetlist, "hp")
	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) then return target end
	end
	return targetlist[#targetlist]
end

function SmartAI:askForPlayerChosen(targets, reason)
	local playerchosen = sgs.ai_skill_playerchosen[string.gsub(reason, "%-", "_")]
	local target = nil
	if type(playerchosen) == "function" then
		target = playerchosen(self, targets)
		return target
	end
	local r = math.random(0, targets:length() - 1)
	return targets:at(r)
end

function SmartAI:ableToSave(saver, dying)
	local current = self.room:getCurrent()
	if current and current:getPhase() ~= sgs.Player_NotActive and current:hasShownSkill("wansha")
		and current:objectName() ~= saver:objectName() and current:objectName() ~= dying:objectName() then
		return false
	end
	local peach = sgs.cloneCard("peach", sgs.Card_NoSuitRed, 0)
	if saver:isCardLimited(peach, sgs.Card_MethodUse, true) then return false end
	return true
end

function SmartAI:willUsePeachTo(dying)
	local card_str
	local forbid = sgs.cloneCard("peach")
	if self.player:isLocked(forbid) or dying:isLocked(forbid) then return "." end
	if self.player:objectName() == dying:objectName() then
		local analeptic = sgs.cloneCard("analeptic")
		if not self.player:isLocked(analeptic) and self:getCardId("Analeptic") then return self:getCardId("Analeptic") end
		if self:getCardId("Peach") then return self:getCardId("Peach") end
	end

	local damage = self.room:getTag("CurrentDamageStruct"):toDamage()
	if type(damage) == "userdata" and damage.to and damage.to:objectName() == dying:objectName() and damage.from
		and (damage.from:objectName() == self.player:objectName()
			or self.player:isFriendWith(damage.from)
			or self:evaluateKingdom(damage.from) == self.player:getKingdom())
		and (self.player:getKingdom() ~= sgs.ai_explicit[damage.to:objectName()] or self.role == "careerist") then
		return "."
	end

	if self:isFriend(dying) then
		if not self.player:isFriendWith(dying) and self:isWeak() then return "." end

		if self:getCardsNum("Peach") + self:getCardsNum("Analeptic") <= sgs.ai_NeedPeach[self.player:objectName()] then return "." end

		if math.ceil(self:getAllPeachNum()) < 1 - dying:getHp() then return "." end

		if dying:objectName() ~= self.player:objectName() then
			local possible_friend = 0
			for _, friend in ipairs(self.friends_noself) do
				if (self:getKnownNum(friend) == friend:getHandcardNum() and getCardsNum("Peach", friend, self.player) == 0)
					or (self:playerGetRound(friend) < self:playerGetRound(self.player)) then
				elseif sgs.card_lack[friend:objectName()]["Peach"] == 1 then
				elseif not self:ableToSave(friend, dying) then
				elseif friend:getHandcardNum() > 0 or getCardsNum("Peach", friend, self.player) > 0 then
					possible_friend = possible_friend + 1
				end
			end
			if possible_friend == 0 and self:getCardsNum("Peach") < 1 - dying:getHp() then
				return "."
			end
		end


		local buqu = dying:getPile("buqu")
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
		if dying:hasFlag("Kurou_toDie") and (not dying:getWeapon() or dying:getWeapon():objectName() ~= "Crossbow") then return "." end

		if (self.player:objectName() == dying:objectName()) then
			card_str = self:getCardId("Analeptic")
			if not card_str then card_str = self:getCardId("Peach") end
		elseif self:doNotSave(dying) then return "."
		else card_str = self:getCardId("Peach") end
	end
	if not card_str then return nil end
	return card_str
end

function SmartAI:askForSinglePeach(dying)
	local card_str = self:willUsePeachTo(dying)
	return card_str or "."
end

function SmartAI:getOverflow(player, getMaxCards)
	player = player or self.player
	local MaxCards = player:getMaxCards()
	if player:hasShownSkill("qiaobian") and not player:hasFlag("AI_ConsideringQiaobianSkipDiscard") then
		MaxCards = math.max(self.player:getHandcardNum() - 1, MaxCards)
		player:setFlags("-AI_ConsideringQiaobianSkipDiscard")
	end
	if player:hasShownSkill("keji") and not player:hasFlag("KejiSlashInPlayPhase") then MaxCards = self.player:getHandcardNum() end
	if getMaxCards then return MaxCards end
	return player:getHandcardNum() - MaxCards
end

function SmartAI:isWeak(player)
	player = player or self.player
	if hasBuquEffect(player) then return false end
	if player:hasShownSkill("niepan") and player:getMark("@nirvana") > 0 then return false end
	if player:hasShownSkill("kongcheng") and player:isKongcheng() and player:getHp() >= 2 then return false end
	if (player:getHp() <= 2 and player:getHandcardNum() <= 2) or player:getHp() <= 1 then return true end
	return false
end

function SmartAI:useCardByClassName(card, use)
	if not card then global_room:writeToConsole(debug.traceback()) return end
	local class_name = card:getClassName()
	local use_func = self["useCard" .. class_name]

	if use_func then
		use_func(self, card, use)
	end
end

function SmartAI:hasWizard(players, onlyharm)
	local skill
	if onlyharm then skill = sgs.wizard_harm_skill else skill = sgs.wizard_skill end
	for _, player in ipairs(players) do
		if player:hasShownSkills(skill) then
			return true
		end
	end
end

function SmartAI:canRetrial(player, to_retrial, reason)
	player = player or self.player
	to_retrial = to_retrial or self.player
	if player:hasShownSkill("guidao") then
		local blackequipnum = 0
		for _, equip in sgs.qlist(player:getEquips()) do
			if equip:isBlack() then blackequipnum = blackequipnum + 1 end
		end
		if blackequipnum + player:getHandcardNum() > 0 then return true end
	end
	if player:hasShownSkill("guicai") and player:getHandcardNum() > 0 then return true end
	return
end

function SmartAI:getFinalRetrial(player, reason)
	local maxfriendseat = -1
	local maxenemyseat = -1
	local tmpfriend
	local tmpenemy
	local wizardf, wizarde
	player = player or self.room:getCurrent()
	for _, aplayer in ipairs(self.friends) do
		if aplayer:hasShownSkills(sgs.wizard_harm_skill) and self:canRetrial(aplayer, player, reason) then
			tmpfriend = (aplayer:getSeat() - player:getSeat()) % (global_room:alivePlayerCount())
			if tmpfriend > maxfriendseat then
				maxfriendseat = tmpfriend
				wizardf = aplayer
			end
		end
	end
	for _, aplayer in ipairs(self.enemies) do
		if aplayer:hasShownSkills(sgs.wizard_harm_skill) and self:canRetrial(aplayer, player, reason) then
			tmpenemy = (aplayer:getSeat() - player:getSeat()) % (global_room:alivePlayerCount())
			if tmpenemy > maxenemyseat then
				maxenemyseat = tmpenemy
				wizarde = aplayer
			end
		end
	end
	if maxfriendseat == -1 and maxenemyseat == -1 then return 0, nil
	elseif maxfriendseat > maxenemyseat then return 1, wizardf
	else return 2, wizarde end
end

--- Determine that the current judge is worthy retrial
-- @param judge The JudgeStruct that contains the judge information
-- @return True if it is needed to retrial
function SmartAI:needRetrial(judge)
	local reason = judge.reason
	local who = judge.who
	if reason == "lightning" then
		if who:hasShownSkill("hongyan") then return false end

		if who:hasArmorEffect("SilverLion") and who:getHp() > 1 then return false end

		if who:hasArmorEffect("PeaceSpell") then return false end

		if self:isFriend(who) then
			if who:isChained() and self:isGoodChainTarget(who, self.player, sgs.DamageStruct_Thunder, 3) then return false end
		else
			if who:isChained() and not self:isGoodChainTarget(who, self.player, sgs.DamageStruct_Thunder, 3) then return judge:isGood() end
		end
	elseif reason == "indulgence" then
		if who:isSkipped(sgs.Player_Draw) and who:isKongcheng() then
			if who:hasShownSkill("kurou") and who:getHp() >= 3 then
				if self:isFriend(who) then
					return not judge:isGood()
				else
					return judge:isGood()
				end
			end
		end
		if self:isFriend(who) then
			local drawcardnum = self:ImitateResult_DrawNCards(who, who:getVisibleSkillList(true))
			if who:getHp() - who:getHandcardNum() >= drawcardnum and self:getOverflow() < 0 then return false end
			if who:hasShownSkill("tuxi") and who:getHp() > 2 and self:getOverflow() < 0 then return false end
			return not judge:isGood()
		else
			return judge:isGood()
		end
	elseif reason == "supply_shortage" then
		if self:isFriend(who) then
			if who:hasShownSkills("guidao|tiandu") then return false end
			return not judge:isGood()
		else
			return judge:isGood()
		end
	elseif reason == "luoshen" then
		if self:isFriend(who) then
			if who:getHandcardNum() > 30 then return false end
			if self:hasCrossbowEffect(who) or getKnownCard(who, self.player, "Crossbow", false) > 0 then return not judge:isGood() end
			if self:getOverflow(who) > 1 and self.player:getHandcardNum() < 3 then return false end
			return not judge:isGood()
		else
			return judge:isGood()
		end
	elseif reason == "tuntian" then
		if self:isFriend(who) then
			if self.player:objectName() == who:objectName() then
				return not self:isWeak()
			else
				return not judge:isGood()
			end
		else
			return judge:isGood()
		end
	elseif reason == "beige" then
		return true
	end

	if self:isFriend(who) then
		return not judge:isGood()
	elseif self:isEnemy(who) then
		return judge:isGood()
	else
		return false
	end
end

--- Get the retrial cards with the lowest keep value
-- @param cards the table that contains all cards can use in retrial skill
-- @param judge the JudgeStruct that contains the judge information
-- @return the retrial card id or -1 if not found
function SmartAI:getRetrialCardId(cards, judge, self_card)
	if self_card == nil then self_card = true end
	local can_use = {}
	local reason = judge.reason
	local who = judge.who

	local other_suit, hasSpade = {}
	for _, card in ipairs(cards) do
		local card_x = sgs.Sanguosha:getEngineCard(card:getEffectiveId())
		if who:hasShownSkill("hongyan") and card_x:getSuit() == sgs.Card_Spade then
			card_x = sgs.cloneCard(card_x:objectName(), sgs.Card_Heart, card:getNumber())
		end
		if reason == "beige" and not isCard("Peach", card_x, self.player) then
			local damage = self.room:getTag("CurrentDamageStruct"):toDamage()
			if damage.from then
				if self:isFriend(damage.from) then
					if not self:toTurnOver(damage.from, 0) and judge.card:getSuit() ~= sgs.Card_Spade and card_x:getSuit() == sgs.Card_Spade then
						table.insert(can_use, card)
						hasSpade = true
					elseif (not self_card or self:getOverflow() > 0) and judge.card:getSuit() ~= card_x:getSuit() then
						local retr = true
						if (judge.card:getSuit() == sgs.Card_Heart and who:isWounded() and self:isFriend(who))
							or (judge.card:getSuit() == sgs.Card_Club and self:needToThrowArmor(damage.from)) then
							retr = false
						end
						if retr
							and ((self:isFriend(who) and card_x:getSuit() == sgs.Card_Heart and who:isWounded())
								or (card_x:getSuit() == sgs.Card_Club and (self:needToThrowArmor(damage.from) or damage.from:isNude())))
								or (judge.card:getSuit() == sgs.Card_Spade and self:toTurnOver(damage.from, 0)) then
							table.insert(other_suit, card)
						end
					end
				else
					if not self:toTurnOver(damage.from, 0) and card_x:getSuit() ~= sgs.Card_Spade and judge.card:getSuit() == sgs.Card_Spade then
						table.insert(can_use, card)
					end
				end
			end
		elseif self:isFriend(who) and judge:isGood(card_x)
				and not (self_card and (self:getFinalRetrial() == 2 or self:dontRespondPeachInJudge(judge)) and isCard("Peach", card_x, self.player)) then
			table.insert(can_use, card)
		elseif self:isEnemy(who) and not judge:isGood(card_x)
				and not (self_card and (self:getFinalRetrial() == 2 or self:dontRespondPeachInJudge(judge)) and isCard("Peach", card_x, self.player)) then
			table.insert(can_use, card)
		end
	end
	if not hasSpade and #other_suit > 0 then table.insertTable(can_use, other_suit) end

	if next(can_use) then
		if self:needToThrowArmor() then
			for _, c in ipairs(can_use) do
				if c:getEffectiveId() == self.player:getArmor():getEffectiveId() then return c:getEffectiveId() end
			end
		end
		self:sortByKeepValue(can_use)
		return can_use[1]:getEffectiveId()
	else
		return -1
	end
end

function SmartAI:damageIsEffective(to, nature, from)
	local damageStruct = {}
	damageStruct.to = to or self.player
	damageStruct.from = from or self.room:getCurrent()
	damageStruct.nature = nature or sgs.DamageStruct_Normal
	return self:damageIsEffective_(damageStruct)
end

function SmartAI:damageIsEffective_(damageStruct)
	if type(damageStruct) ~= "table" and type(damageStruct) ~= "userdata" then self.room:writeToConsole(debug.traceback()) return end
	if not damageStruct.to then self.room:writeToConsole(debug.traceback()) return end
	local to = damageStruct.to
	local nature = damageStruct.nature or sgs.DamageStruct_Normal
	local damage = damageStruct.damage or 1
	local from = damageStruct.from

	if type(to) == "table" then self.room:writeToConsole(debug.traceback()) return end

	if to:hasShownSkill("mingshi") and from and not from:hasShownAllGenerals() then
		damage = damage - 1
		if damage < 1 then return false end
	end

	if to:hasArmorEffect("PeaceSpell") and nature ~= sgs.DamageStruct_Normal then return false end
	if to:hasShownSkills("jgyuhuo_pangtong|jgyuhuo_zhuque") and nature == sgs.DamageStruct_Fire then return false end
	if to:getMark("@fog") > 0 and nature ~= sgs.DamageStruct_Thunder then return false end

	for _, callback in pairs(sgs.ai_damage_effect) do
		if type(callback) == "function" then
			local is_effective = callback(self, damageStruct)
			if not is_effective then return false end
		end
	end

	return true
end

function SmartAI:getDamagedEffects(to, from, isSlash)
	from = from or self.room:getCurrent()
	to = to or self.player

	if isSlash then
		if from:hasWeapon("IceSword") and to:getCards("he"):length() > 1 and not self:isFriend(from, to) then
			return false
		end
	end

	if from:objectName() ~= to:objectName() and self:hasHeavySlashDamage(from, nil, to) then return false end

	if sgs.isGoodHp(to) then
		for _, askill in sgs.qlist(to:getVisibleSkillList(true)) do
			local callback = sgs.ai_need_damaged[askill:objectName()]
			if type(callback) == "function" and callback(self, from, to) then return true end
		end
	end
	return false
end

local function prohibitUseDirectly(card, player)
	if player:isCardLimited(card, card:getHandlingMethod()) then return true end
	if card:isKindOf("Peach") and player:hasFlag("Global_PreventPeach") then return true end
	return false
end

local function getPlayerSkillList(player)
	local skills = sgs.QList2Table(player:getVisibleSkillList(true))
	return skills
end

local function cardsView(self, class_name, player, cards)
	for _, skill in ipairs(getPlayerSkillList(player)) do
		local askill = skill:objectName()
		if player:hasShownSkill(askill) or player:hasLordSkill(askill) then
			local callback = sgs.ai_cardsview[askill]
			if type(callback) == "function" then
				local ret = callback(self, class_name, player, cards)
				if ret then return ret end
			end
		end
	end
	return {}
end

local function getSkillViewCard(card, class_name, player, card_place)
	for _, skill in ipairs(getPlayerSkillList(player)) do
		local askill = skill:objectName()
		if player:hasShownSkill(askill) or player:hasLordSkill(askill) then
			local callback = sgs.ai_view_as[askill]
			if type(callback) == "function" then
				local skill_card_str = callback(card, player, card_place, class_name)
				if skill_card_str then
					local skill_card = sgs.Card_Parse(skill_card_str)
					assert(skill_card)
					if skill_card:isKindOf(class_name) and not player:isCardLimited(skill_card, skill_card:getHandlingMethod()) then return skill_card_str end
				end
			end
		end
	end
end

function isCard(class_name, card, player)
	if not player or not card then global_room:writeToConsole(debug.traceback()) end
	if not card:isKindOf(class_name) then
		local place
		local id = card:getEffectiveId()
		if global_room:getCardOwner(id) == nil or global_room:getCardOwner(id):objectName() ~= player:objectName() then place = sgs.Player_PlaceHand
		else place = global_room:getCardPlace(id) end
		if getSkillViewCard(card, class_name, player, place) then return true end
	else
		if not prohibitUseDirectly(card, player) then return true end
	end
	return false
end

function SmartAI:getMaxCard(player, cards)
	player = player or self.player

	if player:isKongcheng() then
		return nil
	end

	cards = cards or player:getHandcards()
	local max_card, max_point = nil, 0
	for _, card in sgs.qlist(cards) do
		local flag = string.format("%s_%s_%s", "visible", global_room:getCurrent():objectName(), player:objectName())
		if (player:objectName() == self.player:objectName() and not self:isValuableCard(card)) or card:hasFlag("visible") or card:hasFlag(flag) then
			local point = card:getNumber()
			if point > max_point then
				max_point = point
				max_card = card
			end
		end
	end
	if player:objectName() == self.player:objectName() and not max_card then
		for _, card in sgs.qlist(cards) do
			local point = card:getNumber()
			if point > max_point then
				max_point = point
				max_card = card
			end
		end
	end

	if player:objectName() ~= self.player:objectName() then return max_card end

	if player:hasShownSkill("tianyi") and max_point > 0 then
		for _, card in sgs.qlist(cards) do
			if card:getNumber() == max_point and not isCard("Slash", card, player) then
				return card
			end
		end
	end

	return max_card
end

function SmartAI:getMinCard(player)
	player = player or self.player

	if player:isKongcheng() then
		return nil
	end

	local cards = player:getHandcards()
	local min_card, min_point = nil, 14
	for _, card in sgs.qlist(cards) do
		local flag = string.format("%s_%s_%s", "visible", global_room:getCurrent():objectName(), player:objectName())
		if player:objectName() == self.player:objectName() or card:hasFlag("visible") or card:hasFlag(flag) then
			local point = card:getNumber()
			if point < min_point then
				min_point = point
				min_card = card
			end
		end
	end

	return min_card
end

function SmartAI:getKnownNum(player)
	player = player or self.player
	if not player then
		return self.player:getHandcardNum()
	else
		local cards = player:getHandcards()
		local known = 0
		for _, card in sgs.qlist(cards) do
			local flag=string.format("%s_%s_%s","visible",global_room:getCurrent():objectName(),player:objectName())
			if card:hasFlag("visible") or card:hasFlag(flag) then
				known = known + 1
			end
		end
		return known
	end
end

function getKnownNum(player, anotherplayer)
	if not player then global_room:writeToConsole(debug.traceback()) return end
	local cards = player:getHandcards()
	local known = 0
	anotherplayer = anotherplayer or global_room:getCurrent()
	for _, card in sgs.qlist(cards) do
		local flag=string.format("%s_%s_%s", "visible", anotherplayer:objectName(), player:objectName())
		if card:hasFlag("visible") or card:hasFlag(flag) then
			known = known + 1
		end
	end
	return known
end

function getKnownCard(player, from, class_name, viewas, flags)
	if not player or (flags and type(flags) ~= "string") then global_room:writeToConsole(debug.traceback()) return 0 end
	flags = flags or "h"
	player = findPlayerByObjectName(player:objectName())
	from = from or global_room:getCurrent()
	local cards = player:getCards(flags)
	local known = 0
	local suits = {["club"] = 1, ["spade"] = 1, ["diamond"] = 1, ["heart"] = 1}
	for _, card in sgs.qlist(cards) do
		local flag = string.format("%s_%s_%s", "visible", from:objectName(), player:objectName())
		if card:hasFlag("visible") or card:hasFlag(flag) or player:objectName() == from:objectName() then
			if (viewas and isCard(class_name, card, player)) or card:isKindOf(class_name)
				or (suits[class_name] and card:getSuitString() == class_name)
				or (class_name == "red" and card:isRed()) or (class_name == "black" and card:isBlack()) then
				known = known + 1
			end
		end
	end
	return known
end

function SmartAI:getCardId(class_name, player, acard)
	player = player or self.player
	local cards
	if acard then cards = { acard }
	else
		cards = player:getCards("he")
		for _, key in sgs.list(player:getPileNames()) do
			for _, id in sgs.qlist(player:getPile(key)) do
				cards:append(sgs.Sanguosha:getCard(id))
			end
		end
		cards = sgs.QList2Table(cards)
	end
	self:sortByUsePriority(cards, player)

	local viewArr, cardArr = {}, {}

	for _, card in ipairs(cards) do
		local viewas, cardid
		local card_place = self.room:getCardPlace(card:getEffectiveId())
		viewas = getSkillViewCard(card, class_name, player, card_place)
		if viewas then table.insert(viewArr, viewas) end
		if card:isKindOf(class_name) and not prohibitUseDirectly(card, player) and card_place ~= sgs.Player_PlaceSpecial then
			table.insert(cardArr, card:getEffectiveId())
		end
	end
	if #viewArr > 0 or #cardArr > 0 then
		local viewas, cardid
		viewas = #viewArr > 0 and viewArr[1]
		cardid = #cardArr > 0 and cardArr[1]
		local viewCard
		if viewas then
			viewCard = sgs.Card_Parse(viewas)
			assert(viewCard)
		end
		if cardid or viewCard then return cardid or viewas end
	end
	local cardsView = cardsView(self, class_name, player)
	if #cardsView > 0 then return cardsView[1] end
	return
end

function SmartAI:getCard(class_name, player)
	player = player or self.player
	local card_id = self:getCardId(class_name, player)
	if card_id then return sgs.Card_Parse(card_id) end
end

function SmartAI:getCards(class_name, flag)
	local player = self.player
	local room = self.room
	if flag and type(flag) ~= "string" then room:writeToConsole(debug.traceback()) return {} end

	local private_pile
	if not flag then private_pile = true end
	flag = flag or "he"
	local all_cards = player:getCards(flag)
	if private_pile then
		for _, key in sgs.list(player:getPileNames()) do
			for _, id in sgs.qlist(player:getPile(key)) do
				all_cards:append(sgs.Sanguosha:getCard(id))
			end
		end
	end

	local cards, other = {}, {}
	local card_place, card_str

	for _, card in sgs.qlist(all_cards) do
		card_place = room:getCardPlace(card:getEffectiveId())

		if card:hasFlag("AI_Using") then
		elseif class_name == "." and card_place ~= sgs.Player_PlaceSpecial then table.insert(cards, card)
		elseif card:isKindOf(class_name) and not prohibitUseDirectly(card, player) and card_place ~= sgs.Player_PlaceSpecial then table.insert(cards, card)
		else
			card_str = getSkillViewCard(card, class_name, player, card_place)
			if card_str then
				card_str = sgs.Card_Parse(card_str)
				assert(card_str)
				table.insert(cards, card_str)
			else
				table.insert(other, card)
			end
		end
	end

	card_str = cardsView(self, class_name, player, other)
	if #card_str > 0 then
		for _, str in ipairs(card_str) do
			local c = sgs.Card_Parse(str)
			assert(c)
			table.insert(cards, c)
		end
	end

	return cards
end

function getCardsNum(class_name, player, from)
	if not player then
		global_room:writeToConsole(debug.traceback())
		return 0
	end

	if not from and global_room:getCurrent():objectName() == player:objectName() then
		global_room:writeToConsole("")
		global_room:writeToConsole("cheat???")
		global_room:writeToConsole(debug.traceback())
		return 0
	end

	local cards = sgs.QList2Table(player:getHandcards())
	local num = 0
	local shownum = 0
	local redpeach = 0
	local redslash = 0
	local blackcard = 0
	local blacknull = 0
	local equipnull = 0
	local equipcard = 0
	local heartslash = 0
	local heartpeach = 0
	local spadenull = 0
	local spadewine = 0
	local spadecard = 0
	local diamondcard = 0
	local clubcard = 0
	local slashjink = 0
	local other = {}
	from = from or global_room:getCurrent()

	for _, card in ipairs(cards) do
		local flag = string.format("%s_%s_%s", "visible", from:objectName(), player:objectName())
		if card:hasFlag("visible") or card:hasFlag(flag) or from:objectName() == player:objectName() then
			shownum = shownum + 1
			if isCard(class_name, card, player) then
				num = num + 1
			else
				table.insert(other, card)
			end
			if card:isKindOf("EquipCard") then
				equipcard = equipcard + 1
			end
			if card:isKindOf("Slash") or card:isKindOf("Jink") then
				slashjink = slashjink + 1
			end
			if card:isRed() then
				if not card:isKindOf("Slash") then
					redslash = redslash + 1
				end
				if not card:isKindOf("Peach") then
					redpeach = redpeach + 1
				end
			end
			if card:isBlack() then
				blackcard = blackcard + 1
				if not card:isKindOf("Nullification") then
					blacknull = blacknull + 1
				end
			end
			if card:getSuit() == sgs.Card_Heart then
				if not card:isKindOf("Slash") then
					heartslash = heartslash + 1
				end
				if not card:isKindOf("Peach") then
					heartpeach = heartpeach + 1
				end
			end
			if card:getSuit() == sgs.Card_Spade then
				if not card:isKindOf("Nullification") then
					spadenull = spadenull + 1
				end
				if not card:isKindOf("Analeptic") then
					spadewine = spadewine + 1
				end
			end
			if card:getSuit() == sgs.Card_Diamond and not card:isKindOf("Slash") then
				diamondcard = diamondcard + 1
			end
			if card:getSuit() == sgs.Card_Club then
				clubcard = clubcard + 1
			end
		end
	end
	num = num + #cardsView(sgs.ais[player:objectName()], class_name, player, other)

	local ecards = player:getCards("e")
	for _, card in sgs.qlist(ecards) do
		equipcard = equipcard + 1
		if player:getHandcardNum() > player:getHp() then
			equipnull = equipnull + 1
		end
		if card:isRed() then
			redpeach = redpeach + 1
			redslash = redslash + 1
		end
		if card:getSuit() == sgs.Card_Heart then
			heartpeach = heartpeach + 1
		end
		if card:getSuit() == sgs.Card_Spade then
			spadecard = spadecard + 1
		end
		if card:getSuit() == sgs.Card_Diamond  then
			diamondcard = diamondcard + 1
		end
		if card:getSuit() == sgs.Card_Club then
			clubcard = clubcard + 1
		end
	end

	if player:objectName() ~= from:objectName() then
		if class_name == "Slash" then
			local slashnum
			if player:hasShownSkill("wusheng") then
				slashnum = redslash + num + (player:getHandcardNum() - shownum) * 0.69
			elseif player:hasShownSkill("longdan") then
				slashnum = slashjink + (player:getHandcardNum() - shownum) * 0.72
			else
				slashnum = num+(player:getHandcardNum() - shownum) * 0.35
			end
			return slashnum
		elseif class_name == "Jink" then
			if player:hasShownSkill("qingguo") then
				return blackcard + num + (player:getHandcardNum() - shownum) * 0.85
			elseif player:hasShownSkill("longdan") then
				return slashjink + (player:getHandcardNum() - shownum) * 0.72
			else
				return num + (player:getHandcardNum() - shownum) * 0.6
			end
		elseif class_name == "Peach" then
			if player:hasShownSkill("jijiu") then
				return num + redpeach + (player:getHandcardNum() - shownum) * 0.6
			else
				return num
			end
		elseif class_name == "Nullification" then
			if player:hasShownSkill("kanpo") then
				return num + blacknull + (player:getHandcardNum() - shownum) * 0.5
			else
				return num
			end
		end
	end
	return num
end

function SmartAI:getCardsNum(class_name, flag)
	local player = self.player
	local n = 0
	if type(class_name) == "table" then
		for _, each_class in ipairs(class_name) do
			n = n + self:getCardsNum(each_class, flag)
		end
		return n
	end
	n = #self:getCards(class_name, flag)

	return n
end

function SmartAI:getAllPeachNum(player)
	player = player or self.player
	local n = 0
	for _, friend in ipairs(self:getFriends(player)) do
		local num = self.player:objectName() == friend:objectName() and self:getCardsNum("Peach") or getCardsNum("Peach", friend, self.player)
		n = n + num
	end
	return n
end
function SmartAI:getRestCardsNum(class_name, yuji)
	yuji = yuji or self.player
	local ban = sgs.Sanguosha:getBanPackages()
	ban = table.concat(ban, "|")
	sgs.discard_pile = self.room:getDiscardPile()
	local totalnum = 0
	local discardnum = 0
	local knownnum = 0
	local card
	for i = 1, sgs.Sanguosha:getCardCount() do
		card = sgs.Sanguosha:getEngineCard(i-1)
		-- if card:isKindOf(class_name) and not ban:match(card:getPackage()) then totalnum = totalnum+1 end
		if card:isKindOf(class_name) then totalnum = totalnum + 1 end
	end
	for _, card_id in sgs.qlist(sgs.discard_pile) do
		card = sgs.Sanguosha:getEngineCard(card_id)
		if card:isKindOf(class_name) then discardnum = discardnum + 1 end
	end
	for _, player in sgs.qlist(self.room:getOtherPlayers(yuji)) do
		knownnum = knownnum + getKnownCard(player, self.player, class_name)
	end
	return totalnum - discardnum - knownnum
end

function SmartAI:hasSuit(suit_strings, include_equip, player)
	return self:getSuitNum(suit_strings, include_equip, player) > 0
end

function SmartAI:getSuitNum(suit_strings, include_equip, player)
	player = player or self.player
	local n = 0
	local flag = include_equip and "he" or "h"
	local allcards
	if player:objectName() == self.player:objectName() then
		allcards = sgs.QList2Table(player:getCards(flag))
	else
		allcards = include_equip and sgs.QList2Table(player:getEquips()) or {}
		local handcards = sgs.QList2Table(player:getHandcards())
		local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), player:objectName())
		for i = 1, #handcards, 1 do
			if handcards[i]:hasFlag("visible") or handcards[i]:hasFlag(flag) then
				table.insert(allcards, handcards[i])
			end
		end
	end
	for _, card in ipairs(allcards) do
		for _, suit_string in ipairs(suit_strings:split("|")) do
			if card:getSuitString() == suit_string
				or (suit_string == "black" and card:isBlack()) or (suit_string == "red" and card:isRed()) then
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
	if type(player) == "table" then
		for _, p in ipairs(player) do
			if p:hasShownSkills(skill_names) then return true end
		end
		return false
	end
	if type(skill_names) == "string" then
		return player:hasShownSkills(skill_names)
	end
	return false
end

function SmartAI:fillSkillCards(cards, priority)
	local i = 1
	while i <= #cards do
		if prohibitUseDirectly(cards[i], self.player) then
			table.remove(cards, i)
		elseif priority and self:getDynamicUsePriority(cards[i]) < 6 then
			table.remove(cards, i)
		else
			i = i + 1
		end
	end
	for _, skill in ipairs(sgs.ai_skills) do
		if self:hasSkill(skill) or skill.name == "transfer" or (skill.name == "shuangxiong" and self.player:hasFlag("shuangxiong")) then
			local skill_card = skill.getTurnUseCard(self, #cards == 0)
			if skill_card then table.insert(cards, skill_card) end
		end
	end
end


function SmartAI:useSkillCard(card, use)
	local name
	if not card then self.room:writeToConsole(debug.traceback()) return end
	if card:isKindOf("LuaSkillCard") then
		name = "#" .. card:objectName()
	else
		name = card:getClassName()
	end
	if not use.isDummy and name ~= "TransferCard"
		and not self.player:hasSkill(card:getSkillName()) and not self.player:hasLordSkill(card:getSkillName()) then return end
	if sgs.ai_skill_use_func[name] then
		sgs.ai_skill_use_func[name](card, use, self)
		return
	end
end

function SmartAI:useBasicCard(card, use)
	if not card then global_room:writeToConsole(debug.traceback()) return end
	if self:needRende() then return end
	self:useCardByClassName(card, use)
end

function SmartAI:aoeIsEffective(card, to, source)
	local players = self.room:getAlivePlayers()
	players = sgs.QList2Table(players)
	source = source or self.room:getCurrent()

	if to:hasArmorEffect("Vine") then
		return false
	end

	if to:isLocked(card) then
		return false
	end

	if card:isKindOf("SavageAssault") then
		if to:hasShownSkills("huoshou|juxiang") then
			return false
		end
	end

	if to:hasShownSkill("weimu") and card:isBlack() then return false end

	if not self:hasTrickEffective(card, to, source) or not self:damageIsEffective(to, nil, source) then
		return false
	end
	return true
end

function SmartAI:canAvoidAOE(card)
	if not self:aoeIsEffective(card, self.player) then return true end
	if card:isKindOf("SavageAssault") then
		if self:getCardsNum("Slash") > 0 then
			return true
		end
	end
	if card:isKindOf("ArcheryAttack") then
		if self:getCardsNum("Jink") > 0 or (self:hasEightDiagramEffect() and self.player:getHp() > 1) then
			return true
		end
	end
	return false
end

function SmartAI:getDistanceLimit(card, from)
	from = from or self.player
	if card:isKindOf("Snatch") or card:isKindOf("SupplyShortage") or card:isKindOf("Slash") then
		return 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, from, card)
	elseif card:isKindOf("Indulgence") or card:isKindOf("FireAttack") then
		return 999
	end
end

function SmartAI:exclude(players, card, from)
	from = from or self.player
	local excluded = {}
	local limit = self:getDistanceLimit(card, from)
	local range_fix = 0
	if card:isKindOf("Snatch") and card:getSkillName() == "jixi" then
		range_fix = range_fix + 1
	end

	if type(players) ~= "table" then players = sgs.QList2Table(players) end

	if card:isVirtualCard() then
		for _, id in sgs.qlist(card:getSubcards()) do
			if from:getOffensiveHorse() and from:getOffensiveHorse():getEffectiveId() == id then range_fix = range_fix + 1 end
		end
	end

	for _, player in ipairs(players) do
		if --[[not sgs.Sanguosha:isProhibited(from, player, card) and]] (not card:isKindOf("TrickCard") or self:hasTrickEffective(card, player, from))
			and (not limit or from:distanceTo(player, range_fix) <= limit) then
			table.insert(excluded, player)
		end
	end
	return excluded
end


function SmartAI:getJiemingChaofeng(player)
	local max_x, chaofeng = 0, 0
	for _, friend in ipairs(self:getFriends(player)) do
		local x = math.min(friend:getMaxHp(), 5) - friend:getHandcardNum()
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

function SmartAI:getAoeValue(card)
	local attacker = self.player
	local good, bad, isEffective_F, isEffective_E = 0, 0, 0, 0

	local current = self.room:getCurrent()
	local wansha = current:hasShownSkill("wansha")
	local peach_num = self:getCardsNum("Peach")
	local null_num = self:getCardsNum("Nullification")
	local punish
	local enemies, kills = 0, 0
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if not self.player:isFriendWith(p) and self:evaluateKingdom(p) ~= self.player:getKingdom() then enemies = enemies + 1 end
		if self:isFriend(p) then
			if not wansha then peach_num = peach_num + getCardsNum("Peach", p, self.player) end
			null_num = null_num + getCardsNum("Nullification", p, self.player)
		else
			null_num = null_num - getCardsNum("Nullification", p, self.player)
		end
	end
	if card:isVirtualCard() and card:subcardsLength() > 0 then
		for _, subcardid in sgs.qlist(card:getSubcards()) do
			local subcard = sgs.Sanguosha:getCard(subcardid)
			if isCard("Peach", subcard, self.player) then peach_num = peach_num - 1 end
			if isCard("Nullification", subcard, self.player) then null_num = null_num - 1 end
		end
	end

	local function getAoeValueTo(to)
		local value, sj_num = 0, 0
		if card:isKindOf("ArcheryAttack") then sj_num = getCardsNum("Jink", to, self.player) end
		if card:isKindOf("SavageAssault") then sj_num = getCardsNum("Slash", to, self.player) end

		if self:aoeIsEffective(card, to, self.player) then
			local sameKingdom
			if self:isFriend(to) then
				isEffective_F = isEffective_F + 1
				if self.player:isFriendWith(to) or self:evaluateKingdom(to) == self.player:getKingdom() then sameKingdom = true end
			else
				isEffective_E = isEffective_E + 1
			end

			local jink = sgs.cloneCard("jink")
			local slash = sgs.cloneCard("slash")
			local isLimited
			if card:isKindOf("ArcheryAttack") and to:isCardLimited(jink, sgs.Card_MethodResponse) then isLimited = true
			elseif card:isKindOf("SavageAssault") and to:isCardLimited(slash, sgs.Card_MethodResponse) then isLimited = true end
			if card:isKindOf("SavageAssault") and sgs.card_lack[to:objectName()]["Slash"] == 1
				or card:isKindOf("ArcheryAttack") and sgs.card_lack[to:objectName()]["Jink"] == 1
				or sj_num < 1 or isLimited then
				value = -20
			else
				value = -10
			end
			-- value = value + math.min(50, to:getHp() * 10)

			if self:getDamagedEffects(to, self.player) then value = value + 30 end
			if self:needToLoseHp(to, self.player, nil, true) then value = value + 20 end

			if card:isKindOf("ArcheryAttack") then
				if to:hasShownSkills("leiji") and (sj_num >= 1 or self:hasEightDiagramEffect(to)) and self:findLeijiTarget(to, 50, self.player) then
					value = value + 20
					if self:hasSuit("spade", true, to) then value = value + 50
					else value = value + to:getHandcardNum() * 10
					end
				elseif self:hasEightDiagramEffect(to) then
					value = value + 5
					if self:getFinalRetrial(to) == 2 then
						value = value - 10
					elseif self:getFinalRetrial(to) == 1 then
						value = value + 10
					end
				end
			end

			if card:isKindOf("ArcheryAttack") and sj_num >= 1 then
				if to:hasShownSkill("xiaoguo") then value = value - 4 end
			elseif card:isKindOf("SavageAssault") and sj_num >= 1 then
				if to:hasShownSkill("xiaoguo") then value = value - 4 end
			end

			if to:getHp() == 1 then
				if sameKingdom then
					if null_num > 0 then null_num = null_num - 1
					elseif getCardsNum("Analeptic", to, self.player) > 0 then
					elseif not wansha and peach_num > 0 then peach_num = peach_num - 1
					elseif wansha and (getCardsNum("Peach", to, self.player) > 0 or self:isFriend(current) and getCardsNum("Peach", to, self.player) > 0) then
					else
						if not punish then
							punish = true
							value = value - self.player:getCardCount(true) * 10
						end
						value = value - to:getCardCount(true) * 10
					end
				else
					kills = kills + 1
					if wansha and (sgs.card_lack[to:objectName()]["Peach"] == 1 or getCardsNum("Peach", to, self.player) == 0) then
						value = value - sgs.getReward(to) * 10
					end
				end
			end

			if not sgs.isAnjiang(to) and to:isLord() then value = value - self.room:getLieges(to:getKingdom(), to):length() * 5 end

			if to:getHp() > 1 and to:hasShownSkill("jianxiong") then
				value = value + ((card:isVirtualCard() and card:subcardsLength() * 10) or 10)
			end

		else
			value = 0
			if to:hasShownSkill("juxiang") and not card:isVirtualCard() then value = value + 10 end
		end

		return value
	end

	if card:isKindOf("SavageAssault") then
		local menghuo = sgs.findPlayerByShownSkillName("huoshou")
		attacker = menghuo or attacker
	end

	for _, p in sgs.qlist(self.room:getAllPlayers()) do
		if p:objectName() == self.player:objectName() then continue end
		if self:isFriend(p) then
			good = good + getAoeValueTo(p)
		else
			bad = bad + getAoeValueTo(p)
		end
		if self:aoeIsEffective(card, p, self.player) and self:cantbeHurt(p, attacker) then bad = bad + 250 end
		if kills == enemies then return 998 end
	end

	if isEffective_F == 0 and isEffective_E == 0 then
		return attacker:hasShownSkill("jizhi") and 10 or -100
	elseif isEffective_E == 0 then
		return -100
	end

	if attacker:hasShownSkill("jizhi") then good = good + 10 end
	if attacker:hasShownSkill("luanji") then good = good + 5 * isEffective_E end

	return good - bad
end

function SmartAI:hasTrickEffective(card, to, from)
	from = from or self.room:getCurrent()
	to = to or self.player
	--if sgs.Sanguosha:isProhibited(from, to, card) then return false end
	if to:isRemoved() then return false end

	if not card:isKindOf("TrickCard") then return true end
	if to:hasShownSkill("hongyan") and card:isKindOf("Lightning") then return false end
	if to:hasShownSkill("qianxun") and card:isKindOf("Snatch") then return false end
	if to:hasShownSkill("qianxun") and card:isKindOf("Indulgence") then return false end
	if card:isKindOf("Indulgence") then
		if to:hasSkills("jgjiguan_qinglong|jgjiguan_baihu|jgjiguan_zhuque|jgjiguan_xuanwu") then return false end
		if to:hasSkills("jgjiguan_bian|jgjiguan_suanni|jgjiguan_chiwen|jgjiguan_yazi") then return false end
	end
	if to:hasShownSkill("weimu") and card:isBlack() then
		if from:objectName() == to:objectName() and card:isKindOf("DelayedTrick") then
		else
			return false
		end
	end
	if to:hasShownSkill("kongcheng") and to:isKongcheng() and card:isKindOf("Duel") then return false end

	local nature = sgs.DamageStruct_Normal
	if card:isKindOf("FireAttack") then nature = sgs.DamageStruct_Fire end

	if (card:isKindOf("Duel") or card:isKindOf("FireAttack") or card:isKindOf("ArcheryAttack") or card:isKindOf("SavageAssault"))
		and not self:damageIsEffective(to, nature, from) then return false end

	if to:hasArmorEffect("IronArmor") and (card:isKindOf("FireAttack") or card:isKindOf("BurningCamps")) then return false end

	for _, callback in pairs(sgs.ai_trick_prohibit) do
		if type(callback) == "function" then
			if callback(self, card, to, from) then return false end
		end
	end

	return true
end

function SmartAI:useTrickCard(card, use)
	if not card then global_room:writeToConsole(debug.traceback()) return end
	if self:needRende() and not card:isKindOf("ExNihilo") then return end
	self:useCardByClassName(card, use)
end

sgs.weapon_range = {}

function SmartAI:hasEightDiagramEffect(player)
	player = player or self.player
	return player:hasArmorEffect("EightDiagram") or player:hasArmorEffect("bazhen")
end

function SmartAI:hasCrossbowEffect(player)
	player = player or self.player
	return player:hasWeapon("Crossbow") or player:hasShownSkill("paoxiao")
end

sgs.ai_weapon_value = {}

function SmartAI:evaluateWeapon(card, player, target)
	player = player or self.player
	local deltaSelfThreat, inAttackRange = 0
	local currentRange
	local enemies = target and { target } or self:getEnemies(player)
	if not card then return -1
	else
		currentRange = sgs.weapon_range[card:getClassName()] or 0
	end
	for _, enemy in ipairs(enemies) do
		if player:distanceTo(enemy) <= currentRange then
			inAttackRange = true
			local def = sgs.getDefenseSlash(enemy, self) / 2
			if def < 0 then def = 6 - def
			elseif def <= 1 then def = 6
			else def = 6 / def
			end
			deltaSelfThreat = deltaSelfThreat + def
		end
	end

	local slash_num = player:objectName() == self.player:objectName() and self:getCardsNum("Slash") or getCardsNum("Slash", player, self.player)
	local analeptic_num = player:objectName() == self.player:objectName() and self:getCardsNum("Analeptic") or getCardsNum("Analeptic", player, self.player)
	local peach_num = player:objectName() == self.player:objectName() and self:getCardsNum("Peach") or getCardsNum("Peach", player, self.player)
	if card:isKindOf("Crossbow") and not player:hasShownSkill("paoxiao") and inAttackRange then
		deltaSelfThreat = deltaSelfThreat + slash_num * 3 - 2
		if player:hasShownSkill("kurou") then deltaSelfThreat = deltaSelfThreat + peach_num + analeptic_num + self.player:getHp() end
		if player:getWeapon() and not self:hasCrossbowEffect(player) and not player:canSlashWithoutCrossbow() and slash_num > 0 then
			for _, enemy in ipairs(enemies) do
				if player:distanceTo(enemy) <= currentRange
					and (sgs.card_lack[enemy:objectName()]["Jink"] == 1 or slash_num >= enemy:getHp()) then
					deltaSelfThreat = deltaSelfThreat + 10
				end
			end
		end
	end
	local callback = sgs.ai_weapon_value[card:objectName()]
	if type(callback) == "function" then
		deltaSelfThreat = deltaSelfThreat + (callback(self, nil, player) or 0)
		for _, enemy in ipairs(enemies) do
			if player:distanceTo(enemy) <= currentRange and callback then
				local added = sgs.ai_slash_weaponfilter[card:objectName()]
				if type(added) == "function" and added(self, enemy, player) then deltaSelfThreat = deltaSelfThreat + 1 end
				deltaSelfThreat = deltaSelfThreat + (callback(self, enemy, player) or 0)
			end
		end
	end

	if player:hasShownSkill("jijiu") and card:isRed() then deltaSelfThreat = deltaSelfThreat + 0.5 end
	if player:hasShownSkills("qixi|guidao") and card:isBlack() then deltaSelfThreat = deltaSelfThreat + 0.5 end

	return deltaSelfThreat, inAttackRange
end

sgs.ai_armor_value = {}

function SmartAI:evaluateArmor(card, player)
	player = player or self.player
	local ecard = card or player:getArmor()
	if not ecard then return 0 end

	local value = 0
	if player:hasShownSkill("jijiu") and ecard:isRed() then value = value + 0.5 end
	if player:hasShownSkills("qixi|guidao") and ecard:isBlack() then value = value + 0.5 end
	for _, askill in sgs.qlist(player:getVisibleSkillList(true)) do
		local callback = sgs.ai_armor_value[askill:objectName()]
		if type(callback) == "function" then
			return value + (callback(ecard, player, self) or 0)
		end
	end
	local callback = sgs.ai_armor_value[ecard:objectName()]
	if type(callback) == "function" then
		return value + (callback(player, self) or 0)
	end
	return value + 0.5
end

function SmartAI:getSameEquip(card, player)
	player = player or self.player
	if not card then return end
	if card:isKindOf("Weapon") then return player:getWeapon()
	elseif card:isKindOf("Armor") then return player:getArmor()
	elseif card:isKindOf("DefensiveHorse") then return player:getDefensiveHorse()
	elseif card:isKindOf("OffensiveHorse") then return player:getOffensiveHorse() end
end

function SmartAI:useEquipCard(card, use)
	if not card then global_room:writeToConsole(debug.traceback()) return end
	if self.player:hasSkill("xiaoji") and self:evaluateArmor(card) > -5 then
		local armor = self.player:getArmor()
		if armor and armor:objectName() == "PeaceSpell" and card:isKindOf("Armor") then
			if (self:getAllPeachNum() == 0 and self.player:getHp() < 3) and not (self.player:getHp() < 2 and self:getCardsNum("Analeptic") > 0) then
				return
			end
		end
		use.card = card
		return
	end
	if self.player:hasSkills(sgs.lose_equip_skill) and self:evaluateArmor(card) > -5 and #self.enemies > 1 then
		local armor = self.player:getArmor()
		if armor and armor:objectName() == "PeaceSpell" and card:isKindOf("Armor") then
			if (self:getAllPeachNum() == 0 and self.player:getHp() < 3) and not (self.player:getHp() < 2 and self:getCardsNum("Analeptic") > 0) then
				return
			end
		end
		use.card = card
		return
	end
	if self.player:getHandcardNum() == 1 and self:needKongcheng() and self:evaluateArmor(card) > -5 then
		local armor = self.player:getArmor()
		if armor and armor:objectName() == "PeaceSpell" and card:isKindOf("Armor") then
			if (self:getAllPeachNum() == 0 and self.player:getHp() < 3) and not (self.player:getHp() < 2 and self:getCardsNum("Analeptic") > 0) then
				return
			end
		end
		use.card = card
		return
	end
	if card:isKindOf("Armor") and card:objectName() == "PeaceSpell" then
		local lord_zhangjiao = sgs.findPlayerByShownSkillName("wendao") --有君张角在其他人（受伤/有防具）则不装备太平要术
		if lord_zhangjiao and lord_zhangjiao:isAlive() and not self:isWeak(lord_zhangjiao) then
			if self.player:objectName() ~= lord_zhangjiao:objectName() and (self.player:isWounded() or self.player:getArmor()) then
				return
			end
		end
	end
	if card:isKindOf("Weapon") and card:objectName() == "DragonPhoenix" then
		local lord_liubei = sgs.findPlayerByShownSkillName("zhangwu") --有君刘备在（其他势力/除他以外有武器）的人不装备龙凤剑
		if lord_liubei and lord_liubei:isAlive() then
			if not self.player:isFriendWith(lord_liubei) or (self.player:objectName() ~= lord_liubei:objectName() and self.player:getWeapon()) then
				return
			end
		end
	end
	local same = self:getSameEquip(card)
	local zzzh, isfriend_zzzh, isenemy_zzzh = sgs.findPlayerByShownSkillName("guzheng")
	if zzzh then
		if self:isFriend(zzzh) then isfriend_zzzh = true
		else isenemy_zzzh = true
		end
	end
	if same then
		if (self.player:hasSkill("rende") and self:findFriendsByType(sgs.Friend_Draw))
			or (self.player:hasSkills("qixi|duanliang") and (card:isBlack() or same:isBlack()))
			or (self.player:hasSkills("guose") and (card:getSuit() == sgs.Card_Diamond or same:getSuit() == sgs.Card_Diamond))
			or (self.player:hasSkill("jijiu") and (card:isRed() or same:isRed()))
			or (self.player:hasSkill("guidao") and same:isBlack() and card:isRed())
			or isfriend_zzzh
			then return end
	end
	local canUseSlash = self:getCardId("Slash") and self:slashIsAvailable(self.player)
	self:useCardByClassName(card, use)
	if use.card then return end
	if card:isKindOf("Weapon") then
		if same and self.player:hasSkill("qiangxi") and not self.player:hasUsed("QiangxiCard") then
			local dummy_use = { isDummy = true }
			self:useSkillCard(sgs.Card_Parse("@QiangxiCard=" .. same:getEffectiveId().. "&qiangxi"), dummy_use)
			if dummy_use.card and dummy_use.card:getSubcards():length() == 1 then return end
		end
		if self.player:hasSkill("rende") then
			for _, friend in ipairs(self.friends_noself) do
				if not friend:getWeapon() then return end
			end
		end
		if self.player:hasSkills("paoxiao") and card:isKindOf("Crossbow") then return end
		if not self:needKongcheng() and not self.player:hasSkills(sgs.lose_equip_skill) and self:getOverflow() <= 0 and not canUseSlash then return end
		--if (not use.to) and self.player:getWeapon() and not self.player:hasSkills(sgs.lose_equip_skill) then return end
		if self.player:hasSkill("zhiheng") and not self.player:hasUsed("ZhihengCard") and self.player:getWeapon() and not card:isKindOf("Crossbow") then return end
		if not self:needKongcheng() and self.player:getHandcardNum() <= self.player:getHp() - 2 then return end
		if not self.player:getWeapon() or self:evaluateWeapon(card) > self:evaluateWeapon(self.player:getWeapon()) then
			use.card = card
		end
	elseif card:isKindOf("Armor") then
		local lion = self:getCard("SilverLion")
		if lion and self.player:isWounded() and not self.player:hasArmorEffect("SilverLion") and not card:isKindOf("SilverLion")
			and not (self.player:hasSkills("bazhen|jgyizhong") and not self.player:getArmor()) then
			use.card = lion
			return
		end
		if self.player:hasSkill("rende") and self:evaluateArmor(card) < 4 then
			for _, friend in ipairs(self.friends_noself) do
				if not friend:getArmor() then return end
			end
		end
		if self:evaluateArmor(card) > self:evaluateArmor() or isenemy_zzzh and self:getOverflow() > 0 then use.card = card end
		return
	elseif card:isKindOf("OffensiveHorse") then
		if self.player:hasSkill("rende") then
			for _,friend in ipairs(self.friends_noself) do
				if not friend:getOffensiveHorse() then return end
			end
			use.card = card
			return
		else
			if not self.player:hasSkills(sgs.lose_equip_skill) and self:getOverflow() <= 0 and not (canUseSlash or self:getCardId("Snatch")) then
				return
			else
				if self.lua_ai:useCard(card) then
					use.card = card
					return
				end
			end
		end
	elseif card:isKindOf("DefensiveHorse") then
		local tiaoxin = true
		if self.player:hasSkill("tiaoxin") then
			local dummy_use = { isDummy = true, defHorse = true }
			self:useSkillCard(sgs.Card_Parse("@TiaoxinCard=.&tiaoxin"), dummy_use)
			if not dummy_use.card then tiaoxin = false end
		end
		if tiaoxin and self.lua_ai:useCard(card) then
			use.card = card
		end
	elseif self.lua_ai:useCard(card) then
		use.card = card
	end
end

function SmartAI:needRende()
	return self.player:getLostHp() > 1 and self:findFriendsByType(sgs.Friend_Draw) and (self.player:hasSkill("rende") and self.player:getMark("rende") < 3)
end

function SmartAI:needToLoseHp(to, from, isSlash, passive, recover)
	from = from or self.room:getCurrent()
	to = to or self.player
	if isSlash then
		if from:hasWeapon("IceSword") and to:getCards("he"):length() > 1 and not self:isFriend(from, to) then
			return false
		end
	end
	if self:hasHeavySlashDamage(from, nil, to) then return false end
	local n = to:getMaxHp()

	if not passive then
		if to:getMaxHp() > 2 then
			if to:hasShownSkill("rende") and not self:willSkipPlayPhase(to) and self:findFriendsByType(sgs.Friend_Draw, to) then n = math.min(n, to:getMaxHp() - 1) end
		end
	end

	local friends = self:getFriendsNoself(to)
	local xiangxiang = sgs.findPlayerByShownSkillName("jieyin")
	if xiangxiang and xiangxiang:isWounded() and self:isFriend(xiangxiang, to) and not to:isWounded() and to:isMale() then
		local need_jieyin = true
		self:sort(friends, "hp")
		for _, friend in ipairs(friends) do
			if friend:isMale() and friend:isWounded() then need_jieyin = false end
		end
		if need_jieyin then n = math.min(n, to:getMaxHp() - 1) end
	end
	if recover then return to:getHp() >= n end
	return to:getHp() > n
end

function IgnoreArmor(from, to)
	if not from or not to then global_room:writeToConsole(debug.traceback()) return end
	if not to:getArmor() then return true end
	if not to:hasArmorEffect(to:getArmor():objectName()) or from:hasWeapon("QinggangSword") then
		return true
	end
	return
end

function SmartAI:needToThrowArmor(player)
	player = player or self.player
	if not player:getArmor() or not player:hasArmorEffect(player:getArmor():objectName()) then return false end
	if player:hasShownSkill("bazhen") and not player:getArmor():isKindOf("EightDiagram") then return true end
	if self:evaluateArmor(player:getArmor(), player) <= -2 then return true end
	if player:hasArmorEffect("SilverLion") and player:isWounded() then
		if self:isFriend(player) then
			if player:objectName() == self.player:objectName() then
				return true
			else
				return self:isWeak(player) and not player:hasShownSkills(sgs.use_lion_skill)
			end
		else
			return true
		end
	end
	local damage = self.room:getTag("CurrentDamageStruct")
	if damage.damage and not damage.chain and not damage.prevented and damage.nature == sgs.DamageStruct_Fire
		and damage.to:isChained() and player:isChained() and player:hasArmorEffect("Vine") then
		return true
	end
	return false
end

function SmartAI:doNotDiscard(to, flags, conservative, n, cant_choose)
	if not to then global_room:writeToConsole(debug.traceback()) return end
	n = n or 1
	flags = flags or "he"
	if to:isNude() then return true end
	conservative = conservative or (sgs.turncount <= 2 and self.room:alivePlayerCount() > 2)
	local enemies = self:getEnemies(to)
	if #enemies == 1 and enemies[1]:hasShownSkills("qianxun|weimu") and self.room:alivePlayerCount() == 2 then conservative = false end

	if cant_choose then
		if self:needKongcheng(to) and to:getHandcardNum() <= n then return true end
		if self:getLeastHandcardNum(to) <= n then return true end
		if to:hasShownSkills(sgs.lose_equip_skill) and to:hasEquip() then return true end
		if self:needToThrowArmor(to) then return true end
	else
		if flags:match("e") then
			if to:hasShownSkills("jieyin+xiaoji") and to:getDefensiveHorse() then return false end
			if to:hasShownSkills("jieyin+xiaoji") and to:getArmor() and not to:getArmor():isKindOf("SilverLion") then return false end
		end
		if flags == "h" or (flags == "he" and not to:hasEquip()) then
			if to:isKongcheng() or not self.player:canDiscard(to, "h") then return true end
			if not self:hasLoseHandcardEffective(to) then return true end
			if to:getHandcardNum() == 1 and self:needKongcheng(to) then return true end
			if #self.friends > 1 and to:getHandcardNum() == 1 and to:hasShownSkill("sijian") then return false end
		elseif flags == "e" or (flags == "he" and to:isKongcheng()) then
			if not to:hasEquip() then return true end
			if to:hasShownSkills(sgs.lose_equip_skill) then return true end
			if to:getCardCount(true) == 1 and self:needToThrowArmor(to) then return true end
		end
		if flags == "he" and n == 2 then
			if not self.player:canDiscard(to, "e") then return true end
			if to:getCardCount(true) < 2 then return true end
			if not to:hasEquip() then
				if not self:hasLoseHandcardEffective(to) then return true end
				if to:getHandcardNum() <= 2 and self:needKongcheng(to) then return true end
			end
			if to:hasShownSkills(sgs.lose_equip_skill) and to:getHandcardNum() < 2 then return true end
			if to:getCardCount(true) <= 2 and self:needToThrowArmor(to) then return true end
		end
	end
	if flags == "he" and n > 2 then
		if not self.player:canDiscard(to, "e") then return true end
		if to:getCardCount(true) < n then return true end
	end
	return false
end

function SmartAI:findPlayerToDiscard(flags, include_self, isDiscard, players, return_table)
	local player_table = {}
	if isDiscard == nil then isDiscard = true end
	local friends, enemies = {}, {}
	if not players then
		friends = include_self and self.friends or self.friends_noself
		enemies = self.enemies
	else
		for _, player in sgs.qlist(players) do
			if self:isFriend(player) and (include_self or player:objectName() ~= self.player:objectName()) then table.insert(friends, player)
			elseif self:isEnemy(player) then table.insert(enemies, player) end
		end
	end
	flags = flags or "he"

	self:sort(enemies, "defense")
	if flags:match("e") then
		for _, enemy in ipairs(enemies) do
			if self.player:canDiscard(enemy, "e") then
				local dangerous = self:getDangerousCard(enemy)
				if dangerous and (not isDiscard or self.player:canDiscard(enemy, dangerous)) then
					table.insert(player_table, enemy)
				end
			end
		end
		for _, enemy in ipairs(enemies) do
			if enemy:hasArmorEffect("EightDiagram") and not self:needToThrowArmor(enemy) and self.player:canDiscard(enemy, enemy:getArmor():getEffectiveId()) then
				table.insert(player_table, enemy)
			end
		end
	end

	if flags:match("j") then
		for _, friend in ipairs(friends) do
			if ((friend:containsTrick("indulgence") and not friend:hasShownSkill("keji")) or friend:containsTrick("supply_shortage"))
				and not (friend:hasShownSkill("qiaobian") and not friend:isKongcheng())
				and (not isDiscard or self.player:canDiscard(friend, "j")) then
				table.insert(player_table, friend)
			end
		end
		for _, friend in ipairs(friends) do
			if friend:containsTrick("lightning") and self:hasWizard(enemies, true) and (not isDiscard or self.player:canDiscard(friend, "j")) then table.insert(player_table, friend) end
		end
		for _, enemy in ipairs(enemies) do
			if enemy:containsTrick("lightning") and self:hasWizard(enemies, true) and (not isDiscard or self.player:canDiscard(enemy, "j")) then table.insert(player_table, enemy) end
		end
	end

	if flags:match("e") then
		for _, friend in ipairs(friends) do
			if self:needToThrowArmor(friend) and (not isDiscard or self.player:canDiscard(friend, friend:getArmor():getEffectiveId())) then
				table.insert(player_table, friend)
			end
		end
		for _, enemy in ipairs(enemies) do
			if self.player:canDiscard(enemy, "e") then
				local valuable = self:getValuableCard(enemy)
				if valuable and (not isDiscard or self.player:canDiscard(enemy, valuable)) then
					table.insert(player_table, enemy)
				end
			end
		end
		for _, enemy in ipairs(enemies) do
			if enemy:hasShownSkills("jijiu|beige|weimu|qingcheng") and not self:doNotDiscard(enemy, "e") then
				if enemy:getDefensiveHorse() and (not isDiscard or self.player:canDiscard(enemy, enemy:getDefensiveHorse():getEffectiveId())) then table.insert(player_table, enemy) end
				if enemy:getArmor() and not self:needToThrowArmor(enemy) and (not isDiscard or self.player:canDiscard(enemy, enemy:getArmor():getEffectiveId())) then table.insert(player_table, enemy) end
				if enemy:getOffensiveHorse() and (not enemy:hasShownSkill("jijiu") or enemy:getOffensiveHorse():isRed()) and (not isDiscard or self.player:canDiscard(enemy, enemy:getOffensiveHorse():getEffectiveId())) then
					table.insert(player_table, enemy)
				end
				if enemy:getWeapon() and (not enemy:hasShownSkill("jijiu") or enemy:getWeapon():isRed()) and (not isDiscard or self.player:canDiscard(enemy, enemy:getWeapon():getEffectiveId())) then
					table.insert(player_table, enemy)
				end
			end
		end
	end

	if flags:match("h") then
		for _, enemy in ipairs(enemies) do
			local cards = sgs.QList2Table(enemy:getHandcards())
			local flag = string.format("%s_%s_%s","visible", self.player:objectName(), enemy:objectName())
			if #cards <= 2 and not enemy:isKongcheng() and not (enemy:hasShownSkill("tuntian") and enemy:getPhase() == sgs.Player_NotActive) then
				for _, cc in ipairs(cards) do
					if (cc:hasFlag("visible") or cc:hasFlag(flag)) and (cc:isKindOf("Peach") or cc:isKindOf("Analeptic")) and (not isDiscard or self.player:canDiscard(enemy, cc:getId())) then
						table.insert(player_table, enemy)
					end
				end
			end
		end
	end

	if flags:match("e") then
		for _, enemy in ipairs(enemies) do
			if enemy:hasEquip() and not self:doNotDiscard(enemy, "e") and (not isDiscard or self.player:canDiscard(enemy, "e")) then
				table.insert(player_table, enemy)
			end
		end
	end

	if flags:match("h") then
		self:sort(enemies, "handcard")
		for _, enemy in ipairs(enemies) do
			if (not isDiscard or self.player:canDiscard(enemy, "h")) and not self:doNotDiscard(enemy, "h") then
				table.insert(player_table, enemy)
			end
		end
	end

	if flags:match("h") then
		local zhugeliang = sgs.findPlayerByShownSkillName("kongcheng")
		if zhugeliang and self:isFriend(zhugeliang) and zhugeliang:getHandcardNum() == 1 and self:getEnemyNumBySeat(self.player, zhugeliang) > 0
			and zhugeliang:getHp() <= 2 and (not isDiscard or self.player:canDiscard(zhugeliang, "h")) then
			table.insert(player_table, zhugeliang)
		end
	end
	if return_table then return player_table
	else
		if #player_table == 0 then return nil else return player_table[1] end
	end
end

function SmartAI:findPlayerToDraw(include_self, drawnum)
	drawnum = drawnum or 1
	local players = sgs.QList2Table(include_self and self.room:getAllPlayers() or self.room:getOtherPlayers(self.player))
	local friends = {}
	for _, player in ipairs(players) do
		if self:isFriend(player) and not (player:hasShownSkill("kongcheng") and player:isKongcheng() and drawnum <= 2) then
			table.insert(friends, player)
		end
	end
	if #friends == 0 then return end

	self:sort(friends, "defense")
	for _, friend in ipairs(friends) do
		if friend:getHandcardNum() < 2 and not self:needKongcheng(friend) and not self:willSkipPlayPhase(friend) then
			return friend
		end
	end

	local AssistTarget = self:AssistTarget()
	if AssistTarget then
		for _, friend in ipairs(friends) do
			if friend:objectName() == AssistTarget:objectName() and not self:willSkipPlayPhase(friend) then
				return friend
			end
		end
	end

	for _, friend in ipairs(friends) do
		if friend:hasShownSkills(sgs.cardneed_skill) and not self:willSkipPlayPhase(friend) then
			return friend
		end
	end

	self:sort(friends, "handcard")
	for _, friend in ipairs(friends) do
		if not self:needKongcheng(friend) and not self:willSkipPlayPhase(friend) then
			return friend
		end
	end
	return nil
end

function SmartAI:dontRespondPeachInJudge(judge)
	if not judge or type(judge) ~= "userdata" then self.room:writeToConsole(debug.traceback()) return end
	local peach_num = self:getCardsNum("Peach")
	if peach_num == 0 then return false end
	if self:willSkipPlayPhase() and self:getCardsNum("Peach") > self:getOverflow(self.player, true) then return false end
	if judge.reason == "lightning" and self:isFriend(judge.who) then return false end

	local card = self:getCard("Peach")
	local dummy_use = { isDummy = true }
	self:useBasicCard(card, dummy_use)
	if dummy_use.card then return true end

	if peach_num <= self.player:getLostHp() then return true end

	if peach_num > self.player:getLostHp() then
		for _, friend in ipairs(self.friends) do
			if self:isWeak(friend) then return true end
		end
	end

	if (judge.reason == "EightDiagram" or judge.reason == "bazhen") and
		self:isFriend(judge.who) and not self:isWeak(judge.who) then return true
	elseif judge.reason == "tieqi" then return true
	elseif judge.reason == "qianxi" then return true
	elseif judge.reason == "beige" then return true
	end

	return false
end



function SmartAI:AssistTarget()
	if sgs.ai_AssistTarget_off then return end
	local human_count, player = 0
	if not sgs.ai_AssistTarget then
		for _, p in sgs.qlist(self.room:getAlivePlayers()) do
			if p:getState() ~= "robot" then
				human_count = human_count + 1
				player = p
			end
		end
		if human_count == 1 and player then
			sgs.ai_AssistTarget = player
		else
			sgs.ai_AssistTarget_off = true
		end
	end
	player = sgs.ai_AssistTarget
	if player and not player:getAI() and player:isAlive() and self:isFriend(player) and player:objectName() ~= self.player:objectName() then return player end
	return
end

function SmartAI:findFriendsByType(prompt, player)
	player = player or self.player
	local friends = self:getFriendsNoself(player)
	if #friends < 1 then return false end
	if prompt == sgs.Friend_Draw then
		for _, friend in ipairs(friends) do
			if not self:needKongcheng(friend, true) then return true end
		end
	elseif prompt == sgs.Friend_Male then
		for _, friend in ipairs(friends) do
			if friend:isMale() then return true end
		end
	elseif prompt == sgs.Friend_MaleWounded then
		for _, friend in ipairs(friends) do
			if friend:isMale() and friend:isWounded() then return true end
		end
	elseif prompt == sgs.Friend_All then
		return true
	else
		global_room:writeToConsole(debug.traceback())
		return
	end
	return false
end

function hasBuquEffect(player)
	return player:hasShownSkill("buqu") and player:getPile("buqu"):length() <= 4
end

function SmartAI:getKingdomCount()
	local count = 0
	local k = {}
	for _, ap in sgs.qlist(self.room:getAlivePlayers()) do
		if not k[ap:getKingdom()] then
			k[ap:getKingdom()] = true
			count = count + 1
		end
	end
	return count
end

function SmartAI:doNotSave(player)
	if (player:hasShownSkill("niepan") and player:getMark("@nirvana") > 0 and player:getCards("e"):length() < 2) then return true end
	if player:hasFlag("AI_doNotSave") then return true end
	return false
end

function SmartAI:ImitateResult_DrawNCards(player, skills)
	if not player then return 0 end
	if player:isSkipped(sgs.Player_Draw) then return 0 end
	if not skills or skills:length() == 0 then return 2 end
	local drawSkills = {}
	for _,skill in sgs.qlist(skills) do
		if player:hasShownSkill(skill:objectName()) then
			table.insert(drawSkills, skill:objectName())
		end
	end
	local count = 2
	if #drawSkills > 0 then
		local lost = player:getLostHp()
		for _,skillname in pairs(drawSkills) do
			if skillname == "tuxi" then return math.min(2, self.room:getOtherPlayers(player):length())
			elseif skillname == "shuangxiong" then return 1
			elseif skillname == "zaiqi" then return math.floor(lost * 3 / 4)
			elseif skillname == "luoyi" then count = count - 1
			elseif skillname == "yingzi" then count = count + 1
			elseif skillname == "haoshi" then count = count + 2 end
		end
	end
	return count
end

function SmartAI:willSkipPlayPhase(player, NotContains_Null)
	local player = player or self.player

	if player:isSkipped(sgs.Player_Play) then return true end
	if player:hasFlag("willSkipPlayPhase") then return true end

	local friend_null = 0
	local friend_snatch_dismantlement = 0
	local cp = self.room:getCurrent()
	if cp and self.player:objectName() == cp:objectName() and self.player:objectName() ~= player:objectName() and self:isFriend(player) then
		for _, hcard in sgs.qlist(self.player:getCards("he")) do
			if (isCard("Snatch", hcard, self.player) and self.player:distanceTo(player) == 1) or isCard("Dismantlement", hcard, self.player) then
				local trick = sgs.cloneCard(hcard:objectName(), hcard:getSuit(), hcard:getNumber())
				if self:hasTrickEffective(trick, player) then friend_snatch_dismantlement = friend_snatch_dismantlement + 1 end
			end
		end
	end
	if not NotContains_Null then
		for _, p in sgs.qlist(self.room:getAllPlayers()) do
			if self:isFriend(p, player) then friend_null = friend_null + getCardsNum("Nullification", p, self.player) end
			if self:isEnemy(p, player) then friend_null = friend_null - getCardsNum("Nullification", p, self.player) end
		end
	end
	if player:containsTrick("indulgence") then
		if self.player:hasSkill("keji") or (player:hasShownSkill("qiaobian") and not player:isKongcheng()) then return false end
		if friend_null + friend_snatch_dismantlement > 1 then return false end
		return true
	end
	return false
end

function SmartAI:willSkipDrawPhase(player, NotContains_Null)
	local player = player or self.player
	if player:isSkipped(sgs.Player_Draw) then return true end

	local friend_null = 0
	local friend_snatch_dismantlement = 0
	local cp = self.room:getCurrent()
	if not NotContains_Null then
		for _, p in sgs.qlist(self.room:getAllPlayers()) do
			if self:isFriend(p, player) then friend_null = friend_null + getCardsNum("Nullification", p, self.player) end
			if self:isEnemy(p, player) then friend_null = friend_null - getCardsNum("Nullification", p, self.player) end
		end
	end
	if cp and self.player:objectName() == cp:objectName() and self.player:objectName() ~= player:objectName() and self:isFriend(player) then
		for _, hcard in sgs.qlist(self.player:getCards("he")) do
			if (isCard("Snatch", hcard, self.player) and self.player:distanceTo(player) == 1) or isCard("Dismantlement", hcard, self.player) then
				local trick = sgs.cloneCard(hcard:objectName(), hcard:getSuit(), hcard:getNumber())
				if self:hasTrickEffective(trick, player) then friend_snatch_dismantlement = friend_snatch_dismantlement + 1 end
			end
		end
	end
	if player:containsTrick("supply_shortage") then
		if self.player:hasSkill("shensu") or (player:hasShownSkill("qiaobian") and not player:isKongcheng()) then return false end
		if friend_null + friend_snatch_dismantlement > 1 then return false end
		return true
	end
	return false
end

function SmartAI:resetCards(cards, except)
	local result = {}
	for _, c in ipairs(cards) do
		if c:getEffectiveId() == except:getEffectiveId() then continue
		else table.insert(result, c) end
	end
	return result
end

function SmartAI:isValuableCard(card, player)
	player = player or self.player
	if (isCard("Peach", card, player) and getCardsNum("Peach", player, self.player) <= 2)
		or (self:isWeak(player) and isCard("Analeptic", card, player))
		or (player:getPhase() ~= sgs.Player_Play
			and ((isCard("Nullification", card, player) and getCardsNum("Nullification", player, self.player) < 2 and player:hasShownSkill("jizhi"))
				or (isCard("Jink", card, player) and getCardsNum("Jink", player, self.player) < 2)))
		or (player:getPhase() == sgs.Player_Play and isCard("ExNihilo", card, player) and not player:isLocked(card)) then
		return true
	end
	local dangerous = self:getDangerousCard(player)
	if dangerous and card:getEffectiveId() == dangerous then return true end
	local valuable = self:getValuableCard(player)
	if valuable and card:getEffectiveId() == valuable then return true end
end

function SmartAI:cantbeHurt(player, from, damageNum)
	from = from or self.player
	damageNum = damageNum or 1
	if not player then self.room:writeToConsole(debug.traceback()) return end

	if player:hasShownSkill("duanchang") and not player:isLord() and #(self:getFriendsNoself(player)) > 0 and player:getHp() <= 1 then
		if not (from:getMaxHp() == 3 and from:getArmor() and from:getDefensiveHorse()) then
			if from:getMaxHp() <= 3 or (from:isLord() and self:isWeak(from)) then return true end
		end
	end
	if player:hasShownSkill("tianxiang") and getKnownCard(player, self.player, "diamond|club", false) < player:getHandcardNum() then
		local peach_num = self.player:objectName() == from:objectName() and self:getCardsNum("Peach") or getCardsNum("Peach", from, self.player)
		local dyingfriend = 0
		for _, friend in ipairs(self:getFriends(from)) do
			if friend:getHp() < 2 and peach_num then
				dyingfriend = dyingfriend + 1
			end
		end
		if dyingfriend > 0 and player:getHandcardNum() > 0 then
			return true
		end
	end
	if player:hasShownSkill("hengzheng") and player:getHandcardNum() ~= 0 and player:getHp() - damageNum == 1
		and from:getNextAlive():objectName() == player:objectName() then
		local hengzheng = 0
		for _, player in sgs.qlist(self.room:getOtherPlayers(from)) do
			if self:isEnemy(player, from) and player:getCardCount(true) > 0 then hengzheng = hengzheng + 1 end
		end
		if hengzheng > 2 then return true end
	end
	return false
end

function SmartAI:getGuixinValue(player)
	if player:isAllNude() then return 0 end
	local card_id = self:askForCardChosen(player, "hej", "dummy")
	if self:isEnemy(player) then
		for _, card in sgs.qlist(player:getJudgingArea()) do
			if card:getEffectiveId() == card_id then
				if card:isKindOf("Lightning") then
					if self:hasWizard(self.enemies, true) then return 0.8
					elseif self:hasWizard(self.friends, true) then return 0.4
					else return 0.5 * (#self.friends) / (#self.friends + #self.enemies) end
				else
					return -0.2
				end
			end
		end
		for i = 0, 3 do
			local card = player:getEquip(i)
			if card and card:getEffectiveId() == card_id then
				if card:isKindOf("Armor") and self:needToThrowArmor(player) then return 0 end
				local value = 0
				if self:getDangerousCard(player) == card_id then value = 1.5
				elseif self:getValuableCard(player) == card_id then value = 1.1
				elseif i == 1 then value = 1
				elseif i == 2 then value = 0.8
				elseif i == 0 then value = 0.7
				elseif i == 3 then value = 0.5
				end
				if player:hasShownSkills(sgs.lose_equip_skill) or self:doNotDiscard(player, "e", true) then value = value - 0.2 end
				return value
			end
		end
		if self:needKongcheng(player) and player:getHandcardNum() == 1 then return 0 end
		if not self:hasLoseHandcardEffective() then return 0.1
		else
			local index = player:hasShownSkills("jijiu|qingnang|leiji|jieyin|beige|kanpo|liuli|qiaobian|zhiheng|guidao|tianxiang|lijian") and 0.7 or 0.6
			local value = 0.2 + index / (player:getHandcardNum() + 1)
			if self:doNotDiscard(player, "h", true) then value = value - 0.1 end
			return value
		end
	elseif self:isFriend(player) then
		for _, card in sgs.qlist(player:getJudgingArea()) do
			if card:getEffectiveId() == card_id then
				if card:isKindOf("Lightning") then
					if self:hasWizard(self.enemies, true) then return 1
					elseif self:hasWizard(self.friends, true) then return 0.8
					else return 0.4 * (#self.enemies) / (#self.friends + #self.enemies) end
				else
					return 1.5
				end
			end
		end
		for i = 0, 3 do
			local card = player:getEquip(i)
			if card and card:getEffectiveId() == card_id then
				if card:isKindOf("Armor") and self:needToThrowArmor(player) then return 0.9 end
				local value = 0
				if i == 1 then value = 0.1
				elseif i == 2 then value = 0.2
				elseif i == 0 then value = 0.25
				elseif i == 3 then value = 0.25
				end
				if player:hasShownSkills(sgs.lose_equip_skill) then value = value + 0.1 end
				if player:hasShownSkill("tuntian") then value = value + 0.1 end
				return value
			end
		end
		if self:needKongcheng(player, true) and player:getHandcardNum() == 1 then return 0.5
		elseif self:needKongcheng(player) and player:getHandcardNum() == 1 then return 0.3 end
		if not self:hasLoseHandcardEffective() then return 0.2
		else
			local index = player:hasShownSkills("jijiu|qingnang|leiji|jieyin|beige|kanpo|liuli|qiaobian|zhiheng|guidao|tianxiang|lijian") and 0.5 or 0.4
			local value = 0.2 - index / (player:getHandcardNum() + 1)
			if player:hasShownSkill("tuntian") then value = value + 0.1 end
			return value
		end
	end
	return 0.3
end

function SmartAI:setSkillsPreshowed()
	for _, player in sgs.qlist(self.room:getAlivePlayers()) do
		if player:getAI() then
			player:setSkillsPreshowed("hd", true)
		end
	end
end

function SmartAI:willShowForAttack()
	if not sgs.isAnjiang(self.player) then return true end
	local notshown, shown, f, e, eAtt = 0, 0, 0, 0, 0
	for _,p in sgs.qlist(self.room:getAlivePlayers()) do
		if  not p:hasShownOneGeneral() then
			notshown = notshown + 1
		end
		if p:hasShownOneGeneral() then
			shown = shown + 1
			if self:evaluateKingdom(p) == self.player:getKingdom() then
				f = f + 1
			else
				e = e + 1
				if self:isWeak(p) and p:getHp() == 1 and self.player:distanceTo(p) <= self.player:getAttackRange() then eAtt= eAtt + 1 end
			end
		end
	end

	if self.room:alivePlayerCount() > 3 and shown <= math.max(self.room:alivePlayerCount()/2,3) and not self.player:hasShownOneGeneral() then
		if e < f or eAtt <= 0 then
			return false
		end
	end
	return true
end

function SmartAI:willShowForDefence()
	if not sgs.isAnjiang(self.player) then return true end
	local notshown, shown, f, e, eAtt = 0, 0, 0, 0, 0
	for _,p in sgs.qlist(self.room:getAlivePlayers()) do
		if  not p:hasShownOneGeneral() then
			notshown = notshown + 1
		end
		if p:hasShownOneGeneral() then
			shown = shown + 1
			if self:evaluateKingdom(p) == self.player:getKingdom() then
				f = f + 1
			else
				e = e + 1
				if self:isWeak(p) and p:getHp() == 1 and self.player:distanceTo(p) <= self.player:getAttackRange() then eAtt= eAtt + 1 end
			end
		end
	end

	if self.room:alivePlayerCount() > 3 and shown <= math.max(self.room:alivePlayerCount()/2, 3) and not self.player:hasShownOneGeneral() then
		if f < 2 or not self:isWeak() then
			return false
		end
	end
	return true
end

function sgs.getReward(player)
	local x = 1
	if not sgs.isAnjiang(player) and player:getRole() == "careerist" then return 1 end
	for _, p in sgs.qlist(player:getRoom():getOtherPlayers(player)) do
		if p:isFriendWith(player) then x = x + 1 end
	end
	return x
end

function sgs.hasNullSkill(skill_name, player)
	if sgs.general_shown[player:objectName()]["head"] and player:inHeadSkills(skill_name) and #player:disableShow(true) > 0
		and not player:hasShownGeneral1() then
		return true
	elseif sgs.general_shown[player:objectName()]["deputy"] and player:inDeputySkills(skill_name) and #player:disableShow(false) > 0
		and not player:hasShownGeneral2() then
		return true
	end
	return
end

function SmartAI:isFriendWith(player)
	if self.role == "careerist" then return false end
	if self.player:isFriendWith(player) then return true end
	local kingdom = self.player:getKingdom()
	local p_kingdom = self:evaluateKingdom(player)
	if kingdom == p_kingdom then
		local kingdom_num = self.player:getPlayerNumWithSameKingdom("AI")
		if not self.player:hasShownOneGeneral() then kingdom_num = kingdom_num + 1 end
		if not player:hasShownOneGeneral() then kingdom_num = kingdom_num + 1 end
		if self.player:aliveCount() / 2 > kingdom_num or player:getLord() then return true end
	end

	return false
end

function SmartAI:getBigAndSmallKingdoms()
	local jade_seal_owner
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if p:hasTreasure("JadeSeal") then
			jade_seal_owner = p
			break
		end
	end

	local kingdom_map = {}
	local kingdoms = sgs.Sanguosha:getKingdoms()
	table.insert(kingdoms, "careerist")
	for _, k in ipairs(kingdoms) do
		if k == "god" then continue end
		kingdom_map[k] = self.player:getPlayerNumWithSameKingdom("AI", k)
	end
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if not p:hasShownOneGeneral() then
			kingdom_map.anjiang = 1
			break
		end
	end
	local big, small = {}, {}

	local num = 0
	for kingdom, v in pairs(kingdom_map) do
		if v == 0 then continue end
		if #big == 0 then
			num = v
			table.insert(big, kingdom)
			continue
		end

		if v > num then
			num = v
			table.insertTable(small, big)
			big = {}
			table.insert(big, kingdom)
		elseif v == num then
			table.insert(big, kingdom)
		else
			table.insert(small, kingdom)
		end
	end

	if jade_seal_owner then
		if not jade_seal_owner:hasShownOneGeneral() or jade_seal_owner:getRole() == "careerist" then
			table.insertTable(small, big)
			big = {}
			table.insert(big, jade_seal_owner:objectName())
		else
			local kingdom = jade_seal_owner:getKingdom()
			table.insertTable(small, big)
			big = {}
			table.removeOne(small, kingdom)
			table.insert(big, kingdom)
		end
	end
	return big, small
end

function sgs.PlayerList2SPlayerList(playerList)
	local splist = sgs.SPlayerList()
	for _, p in sgs.qlist(global_room:getAlivePlayers()) do
		if playerList:contains(p) then splist:append(p) end
	end
	return splist
end

function sgs.findPlayerByShownSkillName(skill_name)
	for _, p in sgs.qlist(global_room:getAllPlayers()) do
		if p:hasShownSkill(skill_name) then return p end
	end
end


dofile "lua/ai/debug-ai.lua"
dofile "lua/ai/standard_cards-ai.lua"
dofile "lua/ai/maneuvering-ai.lua"
dofile "lua/ai/chat-ai.lua"
dofile "lua/ai/guanxing-ai.lua"
dofile "lua/ai/standard-wei-ai.lua"
dofile "lua/ai/standard-shu-ai.lua"
dofile "lua/ai/standard-wu-ai.lua"
dofile "lua/ai/standard-qun-ai.lua"
dofile "lua/ai/basara-ai.lua"
dofile "lua/ai/jiange-defense-ai.lua"

local loaded = "standard|standard_cards|maneuvering"

local files = table.concat(sgs.GetFileNames("lua/ai"), " ")
local LUAExtensions = string.split(string.lower(sgs.GetConfig("LuaPackages", "")), "+")
local LUAExtensionFiles = table.concat(sgs.GetFileNames("extensions/ai"), " ")

for _, aextension in ipairs(sgs.Sanguosha:getExtensions()) do
	if table.contains(LUAExtensions, string.lower(aextension)) then
		if LUAExtensionFiles:match(string.lower(aextension)) then
			dofile("extensions/ai/" .. string.lower(aextension) .. "-ai.lua")
		end
	elseif not loaded:match(aextension) and files:match(string.lower(aextension)) then
		dofile("lua/ai/" .. string.lower(aextension) .. "-ai.lua")
	end

end
