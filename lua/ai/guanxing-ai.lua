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

sgs.ai_judgestring =
{
	indulgence = "heart",
	diamond = "heart",
	supply_shortage = "club",
	spade = "club",
	club = "club",
	lightning = "spade",
}

local function getIdToCard(self, cards)
	local tocard = {}
	for _, card_id in ipairs(cards) do
		local card = sgs.Sanguosha:getCard(card_id)
		table.insert(tocard, card)
	end
	return tocard
end

local function getBackToId(self, cards)
	local cards_id = {}
	for _, card in ipairs(cards) do
		table.insert(cards_id, card:getEffectiveId())
	end
	return cards_id
end

--for test--
local function ShowGuanxingResult(self, up, bottom)
	self.room:writeToConsole("----GuanxingResult----")
	self.room:writeToConsole(string.format("up:%d", #up))
	if #up > 0 then
		for _,card in pairs(up) do
			self.room:writeToConsole(string.format("(%d)%s[%s%d]", card:getId(), card:getClassName(), card:getSuitString(), card:getNumber()))
		end
	end
	self.room:writeToConsole(string.format("down:%d", #bottom))
	if #bottom > 0 then
		for _,card in pairs(bottom) do
			self.room:writeToConsole(string.format("(%d)%s[%s%d]", card:getId(), card:getClassName(), card:getSuitString(), card:getNumber()))
		end
	end
	self.room:writeToConsole("----GuanxingEnd----")
end
--end--

local function GuanXing(self, cards)
	local up, bottom = {}, {}
	local has_lightning, self_has_judged
	local judged_list = {}
	local willSkipDrawPhase, willSkipPlayPhase

	bottom = getIdToCard(self, cards)
	self:sortByUseValue(bottom, true)

	local judge = sgs.QList2Table(self.player:getJudgingArea())
	judge = sgs.reverse(judge)

	if #judge > 0 then
		local lightning_index
		for judge_count, need_judge in ipairs(judge) do
			judged_list[judge_count] = 0
			if need_judge:isKindOf("Lightning") then
				lightning_index = judge_count
				has_lightning = need_judge
				continue
			elseif need_judge:isKindOf("Indulgence") then
				willSkipPlayPhase = true
				if self.player:isSkipped(sgs.Player_Play) then continue end
			elseif need_judge:isKindOf("SupplyShortage") then
				willSkipDrawPhase = true
				if self.player:isSkipped(sgs.Player_Draw) then continue end
			end
			local judge_str = sgs.ai_judgestring[need_judge:objectName()]
			if not judge_str then
				self.room:writeToConsole(debug.traceback())
				judge_str = sgs.ai_judgestring[need_judge:getSuitString()]
			end
			for index, for_judge in ipairs(bottom) do
				local suit = for_judge:getSuitString()
				if self.player:hasSkill("hongyan") and suit == "spade" then suit = "heart" end
				if judge_str == suit then
					table.insert(up, for_judge)
					table.remove(bottom, index)
					judged_list[judge_count] = 1
					self_has_judged = true
					if need_judge:isKindOf("SupplyShortage") then willSkipDrawPhase = false
					elseif need_judge:isKindOf("Indulgence") then willSkipPlayPhase = false
					end
					break
				end
			end
		end

		if lightning_index then
			for index, for_judge in ipairs(bottom) do
				local cardNumber = for_judge:getNumber()
				local cardSuit = for_judge:getSuitString()
				if self.player:hasSkill("hongyan") and cardSuit == "spade" then cardSuit = "heart" end
				if not (for_judge:getNumber() >= 2 and cardNumber <= 9 and cardSuit == "spade") then
					local i = lightning_index > #up and 1 or lightning_index
					table.insert(up, i , for_judge)
					table.remove(bottom, index)
					judged_list[lightning_index] = 1
					self_has_judged = true
					break
				end
			end
			if judged_list[lightning_index] == 0 then
				if #up >= lightning_index then
					for i = 1, #up - lightning_index + 1 do
						table.insert(bottom, table.remove(up))
					end
				end
				up = getBackToId(self, up)
				bottom = getBackToId(self, bottom)
				return up, bottom
			end
		end

		if not self_has_judged and #judge > 0 then
			return {}, cards
		end

		local index
		if willSkipDrawPhase then
			for i = #judged_list, 1, -1 do
				if judged_list[i] == 0 then index = i
				else break
				end
			end
		end

		for i = 1, #judged_list do
			if judged_list[i] == 0 then
				if i == index then
					up = getBackToId(self, up)
					bottom = getBackToId(self, bottom)
					return up, bottom
				end
				table.insert(up, i, table.remove(bottom, 1))
			end
		end

	end

	local drawCards = self:ImitateResult_DrawNCards(self.player, self.player:getVisibleSkillList(true))
	local drawCards_copy = drawCards
	if willSkipDrawPhase then drawCards = 0 end

	local pos = 1
	local luoshen_flag = false
	local next_judge = {}
	local next_player
	for _, p in sgs.qlist(global_room:getOtherPlayers(self.player)) do
		if p:faceUp() then next_player = p break end
	end
	next_player = next_player or self.player:faceUp() and self.player or self.player:getNextAlive()
	judge = sgs.QList2Table(next_player:getJudgingArea())
	judge = sgs.reverse(judge)
	if has_lightning and not next_player:containsTrick("lightning") then table.insert(judge, 1, has_lightning) end

	local nextplayer_has_judged = false
	judged_list = {}

	while (#bottom > drawCards) do
		if pos > #judge then break end
		local judge_str = sgs.ai_judgestring[judge[pos]:objectName()] or sgs.ai_judgestring[judge[pos]:getSuitString()]

		for index, for_judge in ipairs(bottom) do
			if judge[pos]:isKindOf("Lightning") then lightning_index = pos break end
			if self:isFriend(next_player) then
				if next_player:hasShownSkill("luoshen") then
					if for_judge:isBlack() then
						table.insert(next_judge, for_judge)
						table.remove(bottom, index)
						nextplayer_has_judged = true
						judged_list[pos] = 1
						break
					end
				else
					local suit = for_judge:getSuitString()
					local number = for_judge:getNumber()
					if next_player:hasShownSkill("hongyan") and suit == "spade" then suit = "heart" end
					if judge_str == suit then
						table.insert(next_judge, for_judge)
						table.remove(bottom, index)
						nextplayer_has_judged = true
						judged_list[pos] = 1
						break
					end
				end
			else
				if next_player:hasShownSkill("luoshen") and for_judge:isRed() and not luoshen_flag then
					table.insert(next_judge, for_judge)
					table.remove(bottom, index)
					nextplayer_has_judged = true
					judged_list[pos] = 1
					luoshen_flag = true
					break
				else
					local suit = for_judge:getSuitString()
					local number = for_judge:getNumber()
					if next_player:hasShownSkill("hongyan") and suit== "spade" then suit = "heart" end
					if judge_str ~= suit then
						table.insert(next_judge, for_judge)
						table.remove(bottom, index)
						nextplayer_has_judged = true
						judged_list[pos] = 1
						break
					end
				end
			end
		end
		if not judged_list[pos] then judged_list[pos] = 0 end
		pos = pos + 1
	end

	if lightning_index then
		for index, for_judge in ipairs(bottom) do
			local cardNumber = for_judge:getNumber()
			local cardSuit = for_judge:getSuitString()
			if next_player:hasShownSkill("hongyan") and cardSuit == "spade" then cardSuit = "heart" end
			if self:isFriend(next_player) and not (for_judge:getNumber() >= 2 and cardNumber <= 9 and cardSuit == "spade")
				or not self:isFriend(next_player) and for_judge:getNumber() >= 2 and cardNumber <= 9 and cardSuit == "spade" then
				local i = lightning_index > #next_judge and 1 or lightning_index
				table.insert(next_judge, i , for_judge)
				table.remove(bottom, index)
				judged_list[lightning_index] = 1
				nextplayer_has_judged = true
				break
			end
		end
	end

	local nextplayer_judge_failed
	if lightning_index and not nextplayer_has_judged then
		nextplayer_judge_failed = true
	elseif nextplayer_has_judged then
		local index
		for i = #judged_list, 1, -1 do
			if judged_list[i] == 0 then index = i
			else break
			end
		end
		for i = 1, #judged_list do
			if i == index then nextplayer_judge_failed = true break end
			if judged_list[i] == 0 then
				table.insert(next_judge, i, table.remove(bottom, 1))
			end
		end
	end

	self:sortByUseValue(bottom)
	local up_cards = {}
	if drawCards > 0 and #bottom > 0 then
		local has_slash = self:getCardsNum("Slash") > 0
		local shuangxiong, has_big
		local i = 0
		for index = 1, #bottom do
			local insert = false
			local gcard = bottom[index - i]
			if not willSkipPlayPhase and self.player:hasSkill("shuangxiong") and self.player:getHandcardNum() >= 3 then
				local rednum, blacknum = 0, 0
				local cards = sgs.QList2Table(self.player:getHandcards())
				for _, card in ipairs(cards) do
					if card:isRed() then rednum = rednum + 1 else blacknum = blacknum + 1 end
				end
				if not shuangxiong and ((rednum > blacknum and gcard:isBlack()) or (blacknum > rednum and gcard:isRed()))
					and (isCard("Slash", gcard, self.player) or isCard("Duel", gcard, self.player)) then
					insert = true
					shuangxiong = true
				end
				if not shuangxiong and ((rednum > blacknum and gcard:isBlack()) or (blacknum > rednum and gcard:isRed())) then
					insert = true
					shuangxiong = true
				end
			end
			if not insert and not willSkipPlayPhase and self.player:hasSkills("tianyi|quhu") then
				local maxcard = self:getMaxCard(self.player)
				has_big = maxcard and maxcard:getNumber() > 10
				if not has_big and gcard:getNumber() > 10 then
					insert = true
					has_big = true
				end
				if isCard("Slash", gcard, self.player) then
					insert = true
				end
			end
			if not insert and not willSkipPlayPhase and self.player:hasSkills("jizhi") and isCard("TrickCard", gcard, self.player) then
				insert = true
			end
			if not insert and not willSkipPlayPhase and self.player:hasSkill("yizhi") and isCard("Lightning", gcard, self.player) then
				insert = true
			end
			if not insert then
				if isCard("Peach", gcard, self.player) and (self.player:isWounded()  or self:getCardsNum("Peach") == 0) then
					insert = true
				elseif not willSkipPlayPhase and isCard("ExNihilo", gcard, self.player) then
					insert = true
				elseif not willSkipPlayPhase and isCard("BefriendAttacking", gcard, self.player) then
					insert = true
				elseif not has_slash and isCard("Slash", gcard, self.player) and not willSkipPlayPhase then
					insert = true
					has_slash = true
				end
			end

			if insert then
				drawCards = drawCards - 1
				table.insert(up_cards, table.remove(bottom, index - i))
				i = i + 1
				if isCard("ExNihilo", gcard, self.player) then
					drawCards = drawCards + 2
				elseif isCard("BefriendAttacking", gcard, self.player) then
					drawCards = drawCards + 4
				end
				if drawCards == 0 then break end
			end
		end

		if #bottom > 0 and drawCards > 0 then
			i = 0
			for index = 1, #bottom do
				local gcard = bottom[index - i]
				for _, skill in sgs.qlist(self.player:getVisibleSkillList(true)) do
					local callback = sgs.ai_cardneed[skill:objectName()]
					if type(callback) == "function" and sgs.ai_cardneed[skill:objectName()](self.player, gcard, self) then
						table.insert(up_cards, table.remove(bottom, index - i))
						drawCards = drawCards - 1
						i = i + 1
						if isCard("ExNihilo", gcard, self.player) then
							drawCards = drawCards + 2
						elseif isCard("BefriendAttacking", gcard, self.player) then
							drawCards = drawCards + 4
						end
						break
					end
				end
				if drawCards == 0 then break end
			end
		end
		if #bottom > 0 and drawCards > 0 then
			if willSkipPlayPhase then self.player:setFlags("willSkipPlayPhase") end
			for i = 1, #bottom do
				local c = self:getValuableCardForGuanxing(bottom, up_cards)
				if not c then break end
				for index, card in ipairs(bottom) do
					if card:getEffectiveId() == c:getEffectiveId() then
						table.insert(up_cards, table.remove(bottom, index))
						drawCards = drawCards - 1
						if isCard("ExNihilo", card, self.player) then
							drawCards = drawCards + 2
						elseif isCard("BefriendAttacking", card, self.player) then
							drawCards = drawCards + 4
						end
						break
					end
				end
				if drawCards == 0 then break end
			end
			self.player:setFlags("-willSkipPlayPhase")
		end
	end

	if #up_cards > 0 then
		table.insertTable(up, up_cards)
	end

	if #bottom > drawCards and #bottom > 0 and not nextplayer_judge_failed then
		local maxCount = math.min(#bottom - drawCards, self:ImitateResult_DrawNCards(next_player, next_player:getVisibleSkillList(true)))
		if self:isFriend(next_player) then
			local i = 0
			for index = 1, #bottom do
				if bottom[index - i]:isKindOf("Peach") and (next_player:isWounded() or getCardsNum("Peach", next_player, self.player) < 1)
					or isCard("ExNihilo", bottom[index - i], next_player) then
					table.insert(next_judge, table.remove(bottom, index - i))
					i = i + 1
					if maxCount == i then break end
				end
			end
			maxCount = maxCount - i
			if maxCount > 0 then
				i = 0
				for index = 1, #bottom do
					for _, skill in sgs.qlist(next_player:getVisibleSkillList(true)) do
						local callback = sgs.ai_cardneed[skill:objectName()]
						if type(callback) == "function" and sgs.ai_cardneed[skill:objectName()](next_player, bottom[index - i], self) then
							table.insert(next_judge, table.remove(bottom, index - i))
							i = i + 1
							break
						end
					end
					if maxCount == i then break end
				end
			end
		else
			local i = 0
			for index = 1, #bottom do
				if bottom[index - i]:isKindOf("Lightning") and not next_player:hasShownSkill("leiji") or bottom[index - i]:isKindOf("GlobalEffect") then
					table.insert(next_judge, table.remove(bottom, index - i))
					i = i + 1
					if maxCount == i then break end
				end
			end
		end
	end

	if #next_judge > 0 and drawCards > 0 then
		if #bottom >= drawCards then
			for i = 1, #bottom do
				table.insert(up, table.remove(bottom))
				drawCards = drawCards - 1
				if drawCards == 0 then break end
			end
		else
			table.insertTable(bottom, next_judge)
			next_judge = {}
		end
	end

	for _, gcard in ipairs(next_judge) do
		table.insert(up, gcard)
	end

	if not self_has_judged and not nextplayer_has_judged and #next_judge == 0 and #up >= drawCards_copy and drawCards_copy > 1 then
		for i = 1, drawCards_copy - 1 do
			if isCard("ExNihilo", up[i], self.player) or isCard("BefriendAttacking", up[i], self.player) then
				table.insert(up, drawCards_copy, table.remove(up, i))
			end
		end
	end

	up = getBackToId(self, up)
	bottom = getBackToId(self, bottom)
	return up, bottom
end

local function WuXin(self, cards)
	local up, bottom = {}, {}
	local judged_list = {}
	local hasJudge = false
	local next_player
	for _, p in sgs.qlist(global_room:getOtherPlayers(self.player)) do
		if p:faceUp() then next_player = p break end
	end
	next_player = next_player or self.player:faceUp() and self.player or self.player:getNextAlive()
	local judge = next_player:getCards("j")
	judge = sgs.QList2Table(judge)
	judge = sgs.reverse(judge)

	bottom = getIdToCard(self, cards)
	self:sortByUseValue(bottom, true)

	for judge_count, need_judge in ipairs(judge) do
		local index = 1
		local lightning_flag = false
		local judge_str = sgs.ai_judgestring[need_judge:objectName()] or sgs.ai_judgestring[need_judge:getSuitString()]

		for _, for_judge in ipairs(bottom) do
			if judge_str == "spade" and not lightning_flag then
				if for_judge:getNumber() >= 2 and for_judge:getNumber() <= 9 then lightning_flag = true end
			end
			if self:isFriend(next_player) then
				if judge_str == for_judge:getSuitString() then
					if not lightning_flag then
						table.insert(up, for_judge)
						table.remove(bottom, index)
						judged_list[judge_count] = 1
						has_judged = true
						break
					end
				end
			else
				if judge_str ~= for_judge:getSuitString() or
					(judge_str == for_judge:getSuitString() and judge_str == "spade" and lightning_flag) then
					table.insert(up, for_judge)
					table.remove(bottom, index)
					judged_list[judge_count] = 1
					has_judged = true
				end
			end
			index = index + 1
		end
		if not judged_list[judge_count] then judged_list[judge_count] = 0 end
	end

	if has_judged then
		for index = 1, #judged_list do
			if judged_list[index] == 0 then
				table.insert(up, index, table.remove(bottom))
			end
		end
	end

	self:sortByUseValue(bottom)

	while #bottom ~= 0 do
		table.insert(up, table.remove(bottom))
	end

	up = getBackToId(self, up)
	return up, {}
end

function SmartAI:askForGuanxing(cards, guanxing_type)
	if guanxing_type == sgs.Room_GuanxingBothSides then return GuanXing(self, cards)
	elseif guanxing_type == sgs.Room_GuanxingUpOnly then return WuXin(self, cards)
	elseif guanxing_type == sgs.Room_GuanxingDownOnly then return {}, cards
	end
	return cards, {}
end

function SmartAI:getValuableCardForGuanxing(cards, up_cards)
	local ignore_weapon, ignore_armor, ignore_DH, ignore_OH, ignore_jink
	for _, c in ipairs(up_cards) do
		if c:isKindOf("Weapon") or self:getCardsNum("Weapon", "he") > 0 then ignore_weapon = true
		elseif c:isKindOf("Armor") or self:getCardsNum("Armor", "he") > 0 then ignore_armor = true
		elseif c:isKindOf("DefensiveHorse") or self:getCardsNum("DefensiveHorse", "he") > 0 then ignore_DH = true
		elseif c:isKindOf("OffensiveHorse") or self:getCardsNum("OffensiveHorse", "he") > 0 then ignore_OH = true
		elseif c:isKindOf("Jink") then ignore_jink = true end
	end

	local peach, exnihilo, jink, analeptic, nullification, snatch, dismantlement, indulgence, befriendattacking
	for _, card in ipairs(cards) do
		if isCard("Peach", card, self.player) then
			peach = card
			if self.player:isWounded() or self:getCardsNum("Peach") == 0 then
				return peach
			end
		elseif isCard("ExNihilo", card, self.player) then
			exnihilo = card
		elseif isCard("Jink", card, self.player) and not ignore_jink then
			jink = card
		elseif isCard("Analeptic", card, self.player) then
			analeptic = card
		elseif isCard("Nullification", card, self.player) then
			nullification = card
		elseif isCard("Snatch", card, self.player) then
			snatch = card
		elseif isCard("Dismantlement", card, self.player) then
			dismantlement = card
		elseif isCard("Indulgence", card, self.player) then
			indulgence = card
		elseif isCard("BefriendAttacking", card, self.player) then
			befriendattacking = card
		end
	end

	for _, target in sgs.qlist(self.room:getAlivePlayers()) do
		if self:willSkipPlayPhase(target) or self:willSkipDrawPhase(target) then
			if nullification then return nullification
			elseif self:isFriend(target) and snatch and self:hasTrickEffective(snatch, target, self.player) and
				not self:willSkipPlayPhase() and self.player:distanceTo(target) == 1 then
				return snatch
			elseif self:isFriend(target) and dismantlement and self:hasTrickEffective(dismantlement, target, self.player) and
				not self:willSkipPlayPhase() and self.player:objectName() ~= target:objectName() then
				return dismantlement
			end
		end
	end

	if self.player:hasFlag("willSkipPlayPhase") then
		return peach or jink or analeptic
	end

	if exnihilo or peach then return exnihilo or peach end
	if befriendattacking then
		for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if p:hasShownOneGeneral() and not self.player:isFriendWith(p) then return befriendattacking end
		end
	end
	if (jink or analeptic) and (self:getCardsNum("Jink") == 0 or (self:isWeak() and self:getOverflow() <= 0)) then
		return jink or analeptic
	end
	if indulgence then return indulgence end

	if nullification and self:getCardsNum("Nullification") < 2 then return nullification end

	local eightdiagram, silverlion, vine, renwang, ironarmor, DefHorse, OffHorse
	local weapon, crossbow, halberd, double, qinggang, axe, gudingdao

	for _, card in ipairs(cards) do
		if card:isKindOf("Weapon") and ignore_weapon and not card:isKindOf("Crossbow") then continue
		elseif card:isKindOf("Armor") and ignore_armor then continue
		elseif card:isKindOf("DefensiveHorse") and ignore_DH then continue
		elseif card:isKindOf("OffensiveHorse") and ignore_OH then continue
		end

		if card:isKindOf("EightDiagram") then eightdiagram = card
		elseif card:isKindOf("SilverLion") then silverlion = card
		elseif card:isKindOf("Vine") then vine = card
		elseif card:isKindOf("RenwangShield") then renwang = card
		elseif card:isKindOf("IronArmor") then ironarmor = card

		elseif card:isKindOf("DefensiveHorse") and not self:getSameEquip(card) then DefHorse = card
		elseif card:isKindOf("OffensiveHorse") and not self:getSameEquip(card) then OffHorse = card

		elseif card:isKindOf("Crossbow") then crossbow = card
		elseif card:isKindOf("DoubleSword") then double = card
		elseif card:isKindOf("QinggangSword") then qinggang = card
		elseif card:isKindOf("Halberd") then halberd = card
		elseif card:isKindOf("GudingBlade") then gudingdao = card
		elseif card:isKindOf("Axe") then axe = card end

		if not weapon and card:isKindOf("Weapon") then weapon = card end
	end

	if eightdiagram then
		if not self.player:hasSkills("jgyizhong|bazhen") and self.player:hasSkills("tiandu|leiji|hongyan") then
			return eightdiagram
		end
		if sgs.ai_armor_value.EightDiagram(self.player, self) >= 5 then return eightdiagram end
	end

	if silverlion then
		local lightning, canRetrial
		for _, aplayer in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if aplayer:hasShownSkill("leiji") and self:isEnemy(aplayer) then
				return silverlion
			end
			if aplayer:containsTrick("lightning") then
				lightning = true
			end
			if aplayer:hasShownSkills("guicai|guidao") and self:isEnemy(aplayer) then
				canRetrial = true
			end
		end
		if lightning and canRetrial then return silverlion end
		if self.player:isChained() then
			for _, friend in ipairs(self.friends) do
				if friend:hasArmorEffect("Vine") and friend:isChained() then
					return silverlion
				end
			end
		end
		if self.player:isWounded() then return silverlion end
	end

	if vine then
		if sgs.ai_armor_value.Vine(self.player, self) > 0 and self.room:alivePlayerCount() <= 3 then
			return vine
		end
	end

	if renwang then
		if sgs.ai_armor_value.RenwangShield(self.player, self) > 0 and self:getCardsNum("Jink") == 0 then return renwang end
	end

	if ironarmor then
		for _, enemy in ipairs(self.enemies) do
			if enemy:hasShownSkill("huoji") then return ironarmor end
			if getCardsNum("FireAttack", enemy, self.player) > 0 then return ironarmor end
			if getCardsNum("FireSlash", enemy, self.player) > 0 then return ironarmor end
			if enemy:getFormation():contains(self.player) and getCardsNum("BurningCamps", enemy, self.player) > 0 then return ironarmor end
		end
	end

	if DefHorse and (not self.player:hasSkill("leiji") or self:getCardsNum("Jink") == 0) then
		local before_num, after_num = 0, 0
		for _, enemy in ipairs(self.enemies) do
			if enemy:canSlash(self.player, nil, true) then
				before_num = before_num + 1
			end
			if enemy:canSlash(self.player, nil, true, 1) then
				after_num = after_num + 1
			end
		end
		if before_num > after_num and (self:isWeak() or self:getCardsNum("Jink") == 0) then return DefHorse end
	end

	if analeptic then
		local slashs = self:getCards("Slash")
		for _, enemy in ipairs(self.enemies) do
			local hit_num = 0
			for _, slash in ipairs(slashs) do
				if self:slashIsEffective(slash, enemy) and self.player:canSlash(enemy, slash) and self:slashIsAvailable() then
					hit_num = hit_num + 1
					if getCardsNum("Jink", enemy, self.player) < 1
						or enemy:isKongcheng()
						or self:canLiegong(enemy, self.player)
						or self.player:hasShownSkills("tieji|wushuang")
						or (self.player:hasWeapon("Axe") or self:getCardsNum("Axe") > 0) and self.player:getCards("he"):length() > 4
						then
						return analeptic
					end
				end
			end
			if (self.player:hasWeapon("Blade") or self:getCardsNum("Blade") > 0) and getCardsNum("Jink", enemy, self.player) <= hit_num then return analeptic end
			if self:hasCrossbowEffect(self.player) and hit_num >= 2 then return analeptic end
		end
	end

	if weapon and self:getCardsNum("Slash") > 0 and self:slashIsAvailable() then

		local current_range = self.player:getAttackRange()
		local nosuit_slash = sgs.cloneCard("slash")
		local slash = self:getCard("Slash")

		self:sort(self.enemies, "defense")

		if crossbow then
			if self:getCardsNum("Slash") > 1 or self.player:hasSkills("kurou|keji") then
				return crossbow
			end
			if self.player:hasSkill("rende") then
				for _, friend in ipairs(self.friends_noself) do
					if getCardsNum("Slash", friend, self.player) > 1 then
						return crossbow
					end
				end
			end
		end

		if halberd then
--@todo
		end

		if gudingdao then
			local range_fix = current_range - 2
			for _, enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy, slash, true, range_fix) and not enemy:hasShownSkill("kongcheng")
					and (enemy:isKongcheng() or enemy:getHandcardNum() == 1 and	((self:getCardsNum("Dismantlement") > 0 or (self:getCardsNum("Snatch") > 0 and self.player:distanceTo(enemy) == 1)))) then
					return gudingdao
				end
			end
		end

		if double then
			local range_fix = current_range - 2
			for _, enemy in ipairs(self.enemies) do
				if self.player:getGender() ~= enemy:getGender() and self.player:canSlash(enemy, nil, true, range_fix) then
					return double
				end
			end
		end

		if axe then
			local range_fix = current_range - 3
			local FFFslash = self:getCard("FireSlash")
			for _, enemy in ipairs(self.enemies) do
				if enemy:hasArmorEffect("Vine") and FFFslash and self:slashIsEffective(FFFslash, enemy) and
					self.player:getCardCount(true) >= 3 and self.player:canSlash(enemy, FFFslash, true, range_fix) then
					return axe
				elseif self:getCardsNum("Analeptic") > 0 and self.player:getCardCount(true) >= 4 and
					self:slashIsEffective(slash, enemy) and self.player:canSlash(enemy, slash, true, range_fix) then
					return axe
				end
			end
		end

		if qinggang then
			local range_fix = current_range - 2
			for _, enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy, slash, true, range_fix) and self:slashIsEffective(slash, enemy, self.player, true) and enemy:getArmor() then
					return qinggang
				end
			end
		end

	end

	local classNames = { "Snatch", "Dismantlement", "Indulgence", "SupplyShortage", "Collateral", "Duel", "Drowning", "ArcheryAttack", "SavageAssault", "FireAttack",
							"GodSalvation", "Lightning" }
	local className2objectName = { Snatch = "snatch", Dismantlement = "dismantlement", Indulgence = "indulgence", SupplyShortage = "supply_shortage", Collateral = "collateral",
									Duel = "duel", Drowning = "drowning", ArcheryAttack = "archery_attack", SavageAssault = "savage_assault", FireAttack = "fire_attack",
									GodSalvation = "god_salvation", Lightning = "lightning" }
	local new_enemies = {}
	if #self.enemies > 0 then new_enemies = self.enemies
	else
		for _, aplayer in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if not string.find(self:evaluateKingdom(aplayer), self.player:getKingdom()) then
				table.insert(new_enemies, aplayer)
			end
		end
	end

	for _, className in ipairs(classNames) do
		for _, card in ipairs(cards) do
			if isCard(className, card, self.player) then
				local card_x = className ~= card:getClassName() and sgs.cloneCard(className2objectName[className], card:getSuit(), card:getNumber()) or card
				self.enemies = new_enemies
				local dummy_use = { isDummy = true }
				self:useTrickCard(card_x, dummy_use)
				if dummy_use.card then self:updatePlayers(false) return card end
			end
		end
	end

	if weapon and not self.player:getWeapon() and self:getCardsNum("Slash") > 0 and self:slashIsAvailable() then
		local inAttackRange
		for _, enemy in ipairs(self.enemies) do
			if self.player:inMyAttackRange(enemy) then
				inAttackRange = true
				break
			end
		end
		if not inAttackRange then return weapon end
	end

	if eightdiagram or silverlion or vine or renwang or ironarmor then
		return renwang or eightdiagram or ironarmor or silverlion or vine
	end

	return
end
