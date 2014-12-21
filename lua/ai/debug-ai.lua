--[[********************************************************************
	Copyright (c) 2013-2014 - QSanguosha-Rara

  This file is part of QSanguosha-Hegemony.

  This game is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 3.0
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  See the LICENSE file for more details.

  QSanguosha-Rara
*********************************************************************]]

-- sgs.ai_debug_func[sgs.EventPhaseStart].debugfunc = function(self, player, data)
	-- if player:getPhase() == sgs.Player_Start then
		-- sgs.debugFunc(player)
	 -- end
-- end
--[[
sgs.ai_debug_func[sgs.CardUsed].debugfunc = function(self, player, data)
	local use = data:toCardUse()
	if not use.from or use.from:objectName() ~= player:objectName() then return end
	if use.card:isKindOf("Peach") or use.card:isKindOf("Nullification") then
		sgs.debugFunc(player, 3)
	elseif use.card:isKindOf("Slash") then
		sgs.debugFunc(player, 4)
	end
end]]

function sgs.debugFunc(player, debugType)
	local owner = global_room:getOwner()
	local choices = {"showVisiblecards", "showHandcards", "objectiveLevel", "getDefenseSlash"}
	-- local debugmsg = function(fmt, ...)
		-- if type(fmt) == "boolean" then fmt = fmt and "true" or "false" end
		-- local msg = string.format(fmt, ...)
		-- player:speak(msg)
		-- logmsg("ai.html", "<pre>" .. msg .. "</pre>")
	-- end

	local players = sgs.QList2Table(global_room:getAlivePlayers())

	local function showVisiblecards()
		global_room:writeToConsole(string.format("-=showVisiblecards; AI: %s/%s[%s]", player:getActualGeneral1Name(), player:getActualGeneral2Name(), player:getKingdom()))
		for i = 1, #players, 1 do
			local msg = string.format("%s/%s[Visiblecards]:", players[i]:getActualGeneral1Name(), players[i]:getActualGeneral2Name())
			local cards = sgs.QList2Table(players[i]:getHandcards())
			for _, card in ipairs(cards) do
				if sgs.cardIsVisible(c, players[i], player) then
					msg = msg .. card:getClassName() ..", "
				end
			end
			global_room:writeToConsole(msg)
		end
	end

	local function showHandcards()
		global_room:writeToConsole(string.format("-=showHandcards; AI: %s/%s[%s]", player:getActualGeneral1Name(), player:getActualGeneral2Name(), player:getKingdom()))
		for i = 1, #players, 1 do
			local msg = string.format("%s/%s[Handcards]:", players[i]:getActualGeneral1Name(), players[i]:getActualGeneral2Name())
			local cards = sgs.QList2Table(players[i]:getHandcards())
			for _, card in ipairs(cards) do
				msg = msg .. card:getClassName() ..", "
			end
			global_room:writeToConsole(msg2)
		end
	end

	local function objectiveLevel()
		global_room:writeToConsole("gameProcess :: " .. sgs.gameProcess())
		global_room:writeToConsole(string.format("-=objectiveLevel; AI: %s/%s[%s]", player:getActualGeneral1Name(), player:getActualGeneral2Name(), player:getKingdom()))
		local pSelf = sgs.ais[player:objectName()]
		for i = 1, #players, 1 do
			local level = pSelf:objectiveLevel(players[i])
			local rel = pSelf:evaluateKingdom(players[i])

			global_room:writeToConsole(string.format("%s/%s[%s]: %d %s", players[i]:getActualGeneral1Name(), players[i]:getActualGeneral2Name(),
															players[i]:getKingdom(), level, rel))
		end
	end

	local function getDefenseSlash()
		global_room:writeToConsole(string.format("-=getDefenseSlash; AI: %s/%s[%s]", player:getActualGeneral1Name(), player:getActualGeneral2Name(), player:getKingdom()))
		for i = 1, #players, 1 do
			global_room:writeToConsole(string.format("%s/%s:%.2f", players[i]:getActualGeneral1Name(), players[i]:getActualGeneral2Name(), sgs.getDefenseSlash(players[i], sgs.recorder)))
		end
	end

	if debugType then
		if debugType == 1 then showVisiblecards()
		elseif debugType == 2 then showHandcards()
		elseif debugType == 3 then objectiveLevel()
		elseif debugType == 4 then getDefenseSlash()
		elseif debugType == 5 then
			showVisiblecards()
			showHandcards()
			objectiveLevel()
			getDefenseSlash()
		end
		return
	end

	repeat
		local choice = global_room:askForChoice(owner, "aidebug", "cancel+"..table.concat(choices, "+"))
		if choice == "cancel" then break
		elseif choice == "showVisiblecards" then showVisiblecards()
		elseif choice == "showHandcards" then showHandcards()
		elseif choice == "objectiveLevel" then objectiveLevel()
		elseif choice == "getDefenseSlash" then getDefenseSlash()
		end
	until false
end


function logmsg(fname, fmt, ...)
	local fp = io.open(fname, "ab")
	if type(fmt) == "boolean" then fmt = fmt and "true" or "false" end
	fp:write(string.format(fmt, ...) .. "\r\n")
	fp:close()
end

function changeHero(player, new_general, isSecondaryHero)
	local room = player:getRoom()
	local flag = isSecondaryHero and "general2_showed" or "general1_showed"
	room:setPlayerProperty(player, flag, sgs.QVariant(true))

	if isSecondaryHero then
		room:changePlayerGeneral2(player, new_general)
	else
		room:changePlayerGeneral(player, new_general)
	end

	for _, skill in sgs.qlist(sgs.Sanguosha:getGeneral(new_general):getSkillList(true, not isSecondaryHero)) do
		player:addSkill(skill:objectName(), not isSecondaryHero)
		local args = {
			sgs.CommandType.S_GAME_EVENT_ADD_SKILL,
			player:objectName(),
			skill:objectName(),
			not isSecondaryHero,
		}
		room:doBroadcastNotify(sgs.CommandType.S_COMMAND_LOG_EVENT, json.encode(args))
	end

	local args = {
		11,
		player:objectName(),
		new_general,
		isSecondaryHero,
		true,
	}
	room:doBroadcastNotify(sgs.CommandType.S_COMMAND_LOG_EVENT, json.encode(args))

	player:setHp(5)
	player:setMaxHp(5)
	room:broadcastProperty(player, "hp")
	room:broadcastProperty(player, "maxhp")

	local void_data = sgs.QVariant()
	local gen = isSecondaryHero and player:getGeneral2() or player:getGeneral()
	if gen then
		local thread = room:getThread()
		for _, skill in sgs.qlist(gen:getSkillList(true, not isSecondaryHero)) do
			if skill:inherits("TriggerSkill") then
				local triggerskill = skill:toTriggerSkill()
				thread:addTriggerSkill(triggerskill)
			end
			if skill:getFrequency() == sgs.Skill_Limited and skill:getLimitMark() and skill:getLimitMark() ~= "" then
				room:setPlayerMark(player, skill:getLimitMark(), 1)
			end
		end
	end

	room:resetAI(player)
end

function endlessNiepan(self, player)
	if player:getHp() > 0 then return end
	local room = player:getRoom()
	room:setPlayerProperty(player, "Duanchang", sgs.QVariant(""))
	local getG2 = player:getGeneral2()

	local names = sgs.Sanguosha:getRandomGenerals(2)

	for _, skill in sgs.qlist(player:getSkillList()) do
		player:loseSkill(skill:objectName())
	end
	player:detachAllSkills()

	changeHero(player, names[1], false)
	if getG2 then
		changeHero(player, names[2], true)
	end
	player:bury()
	local flag = getG2 and "hd" or "h"
	player:setSkillsPreshowed(flag)
	room:setPlayerProperty(player, "actual_general1", sgs.QVariant(names[1]))
	room:setPlayerProperty(player, "actual_general2", sgs.QVariant(names[2]))
	room:setPlayerFlag(player, "Global_DFDebut")
	player:setChained(false)
	room:broadcastProperty(player, "chained")
	player:setFaceUp(true)
	room:broadcastProperty(player, "faceup")
	for _, p in sgs.qlist(room:getAlivePlayers()) do
		room:setPlayerProperty(p, "kingdom", sgs.QVariant(sgs.KingdomsTable[math.random(1, #sgs.KingdomsTable)]))
	end
	sgs.updateAlivePlayerRoles()
	for _, p in sgs.qlist(room:getAlivePlayers()) do
		self:updatePlayerKingdom(p)
	end

	room:setTag("SwapPile", sgs.QVariant(0))
	player:drawCards(5)

end

function SmartAI:log(outString)
	self.room:output(outString)
end

local cardparse = sgs.Card_Parse
function sgs.Card_Parse(str)
	if not str then global_room:writeToConsole(debug.traceback()) end
	if type(str) ~= "string" and type(str) ~= "number" and str.toString() then
		global_room:writeToConsole(str:toString())
	end
	local card = cardparse(str)
	if not card then global_room:writeToConsole("Wrong!!sgs.Card_Parse >> " .. str) assert(false) end
	return card
end

function SmartAI:printAll(self, player, intention)
	local name = player:objectName()
	self.room:writeToConsole(self:getHegGeneralName(player) .. math.floor(intention * 10) / 10
							.. " R" .. math.floor((sgs.ai_loyalty["shu"][name] or 0) * 10) / 10
							.. " G" .. math.floor((sgs.ai_loyalty["wu"][name] or 0) * 10) / 10
							.. " B" .. math.floor((sgs.ai_loyalty["wei"][name] or 0) * 10) / 10
							.. " Q" .. math.floor((sgs.ai_loyalty["qun"][name] or 0) * 10) / 10
							.. " E" .. (sgs.ai_explicit[name] or "nil"))
end

function sgs.printFEList(player)
	if not player then global_room:writeToConsole("---==== printFEList ====---") end
	global_room:writeToConsole("gameProcess :: " .. sgs.gameProcess())
	for _, p in sgs.qlist(global_room:getAlivePlayers()) do
		if player and p:objectName() ~= player:objectName() then continue end
		local name = p:getActualGeneral1Name() .. "/" .. p:getActualGeneral2Name()
		global_room:writeToConsole("----  " .. name .. "  kingdom::" .. p:getKingdom() .. "-" .. sgs.ais[global_room:getCurrent():objectName()]:evaluateKingdom(p) .. "  ----")
		local sgsself = sgs.ais[p:objectName()]
		sgsself:updatePlayers()
		local msge = "enemies:"
		for _, enemy in ipairs(sgsself.enemies) do
			local name1 = enemy:getActualGeneral1Name() .. "/" .. enemy:getActualGeneral2Name()
			msge = msge .. name1 .. ", "
		end
		global_room:writeToConsole(msge)
		local msgf = "friends:"
		for _, friend in ipairs(sgsself.friends) do
			local name2 = friend:getActualGeneral1Name() .. "/" .. friend:getActualGeneral2Name()
			msgf = msgf .. name2 .. ", "
		end
		global_room:writeToConsole(msgf)
	end
end

function sgs.ShowPlayer(player)
	for _, p in sgs.qlist(global_room:getAlivePlayers()) do
		if player then
			if player:objectName() == p:objectName() then p:showGeneral() end
		else
			p:showGeneral()
			p:showGeneral(false)
		end
	end
end
