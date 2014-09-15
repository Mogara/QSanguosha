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

sgs.ai_skill_invoke["userdefine:halfmaxhp"] = true

sgs.ai_skill_invoke["userdefine:changetolord"] = function(self)
	return math.random() < 0.8
end

sgs.ai_skill_choice.CompanionEffect = function(self, choice, data)
	if ( self:isWeak() or self:needKongcheng(self.player, true) ) and string.find(choice, "recover") then return "recover"
	else return "draw" end
end

sgs.ai_skill_invoke["userdefine:FirstShowReward"] = true


sgs.ai_skill_choice.heg_nullification = function(self, choice, data)
	local effect = data:toCardEffect()
	if effect.card:isKindOf("AOE") or effect.card:isKindOf("GlobalEffect") then
		if self:isFriendWith(effect.to) then return "all"
		elseif self:isFriend(effect.to) then return "single"
		elseif self:isEnemy(effect.to) then return "all"
		end
	end
	return "all"
end


sgs.ai_skill_choice["GameRule:TriggerOrder"] = function(self, choices, data)

	local canShowHead = string.find(choices, "GameRule_AskForGeneralShowHead")
	local canShowDeputy = string.find(choices, "GameRule_AskForGeneralShowDeputy")

	local firstShow = ("luanji|qianhuan"):split("|")
	local bothShow = ("luanji+shuangxiong|luanji+huoshui"):split("|")
	local needShowForAttack = ("chuanxin|suishi"):split("|")
	local needShowForHelp = ("sushen|cunsi|yicheng|qianhuan"):split("|")
	local needShowForLead = ("yicheng|qianhuan"):split("|")
	local woundedShow = ("zaiqi|yinghun|hunshang|hengzheng"):split("|")
	local followShow = ("qianhuan|duoshi|rende|jieyin|xiongyi|shouyue|hongfa"):split("|")

	local notshown, shown, allshown, f, e, eAtt = 0, 0, 0, 0, 0, 0
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
		if p:hasShownAllGenerals() then
			allshown = allshown + 1
		end
	end

	if self.player:inHeadSkills("baoling") then
		if (self:hasSkill("luanwu") and self.player:getMark("@chaos") ~= 0)
			or (self:hasSkill("xiongyi") and self.player:getMark("@arise") ~= 0) then
			canShowHead = false
		end
	end
	if self.player:inHeadSkills("baoling") then
		if (self.player:hasSkill("mingshi") and allshown >= (self.room:alivePlayerCount() - 1))
			or (self:hasSkill("luanwu") and self.player:getMark("@chaos") == 0)
			or (self:hasSkill("xiongyi") and self.player:getMark("@arise") == 0) then
			if canShowHead then
				return "GameRule_AskForGeneralShowHead"
			end
		end
	end

	if self.player:getMark("CompanionEffect") > 0 or (self.player:getMark("HalfMaxHpLeft") > 0 and self:willShowForDefence()) then
		if self:isWeak() then
			if canShowHead then
				return "GameRule_AskForGeneralShowHead"
			elseif canShowDeputy then
				return "GameRule_AskForGeneralShowDeputy"
			end
		end
	end

	for _, skill in ipairs(bothShow) do
		if self.player:hasSkills(skill) then
			if canShowHead then
				return "GameRule_AskForGeneralShowHead"
			elseif canShowDeputy then
				return "GameRule_AskForGeneralShowDeputy"
			end
		end
	end

	if shown == 0 and self.player:getJudgingArea():isEmpty() then
		for _, skill in ipairs(firstShow) do
			if self.player:hasSkill(skill) then
				if self.player:inHeadSkills(skill) and canShowHead then
					return "GameRule_AskForGeneralShowHead"
				elseif canShowDeputy then
					return "GameRule_AskForGeneralShowDeputy"
				end
			end
		end
	end

	if shown > 0 and eAtt > 0 and e - f < 3 and self.player:getJudgingArea():isEmpty() then
		for _, skill in ipairs(needShowForAttack) do
			if self.player:hasSkill(skill) then
				if self.player:inHeadSkills(skill) and canShowHead then
					return "GameRule_AskForGeneralShowHead"
				elseif canShowDeputy then
					return "GameRule_AskForGeneralShowDeputy"
				end
			end
		end
	end

	if shown > 1 and f > 0 and e > 0 then
		for _, skill in ipairs(needShowForHelp) do
			if self.player:hasSkill(skill) then
				if self.player:inHeadSkills(skill) and canShowHead then
					return "GameRule_AskForGeneralShowHead"
				elseif canShowDeputy then
					return "GameRule_AskForGeneralShowDeputy"
				end
			end
		end
	end

	if shown > 0 and notshown > self.room:alivePlayerCount()/2 then
		for _, skill in ipairs(needShowForLead) do
			if self.player:hasSkill(skill) then
				if self.player:inHeadSkills(skill) and canShowHead then
					return "GameRule_AskForGeneralShowHead"
				elseif canShowDeputy then
					return "GameRule_AskForGeneralShowDeputy"
				end
			end
		end
	end

	if self.player:getLostHp() >= 2 then
		for _, skill in ipairs(woundedShow) do
			if self.player:hasSkill(skill) then
				if self.player:inHeadSkills(skill) and canShowHead then
					return "GameRule_AskForGeneralShowHead"
				elseif canShowDeputy then
					return "GameRule_AskForGeneralShowDeputy"
				end
			end
		end
	end

	for _, skill in ipairs(followShow) do
		if self.player:hasSkill(skill) then
			for _, skill in ipairs(needShowForLead) do
				if self.player:hasSkill(skill) then
					if canShowHead then
						return "GameRule_AskForGeneralShowHead"
					elseif canShowDeputy then
						return "GameRule_AskForGeneralShowDeputy"
					end
				end
			end
			if (shown > 0 and e < notshown-1 ) or ( self.player:hasShownOneGeneral()) then
				if canShowHead then
					return "GameRule_AskForGeneralShowHead"
				elseif canShowDeputy then
					return "GameRule_AskForGeneralShowDeputy"
				end
			end
		end
		for _,p in sgs.qlist(self.room:getOtherPlayers(player)) do
			if p:hasShownSkill(skill) and p:getKingdom() == self.player:getKingdom() and not self.player:hasShownOneGeneral() then
				if canShowHead and canShowDeputy  then
					local cho = { "GameRule_AskForGeneralShowHead", "GameRule_AskForGeneralShowDeputy"}
					return cho[math.random(1, #cho)]
				elseif canShowHead then
					return "GameRule_AskForGeneralShowHead"
				elseif canShowDeputy then
					return "GameRule_AskForGeneralShowDeputy"
				end
			end
		end
	end

	if notshown == 1 and not self.player:hasShownOneGeneral() then
		if canShowHead and canShowDeputy  then
			local cho = { "GameRule_AskForGeneralShowHead", "GameRule_AskForGeneralShowDeputy"}
			return cho[math.random(1, #cho)]
		elseif canShowHead then
			return "GameRule_AskForGeneralShowHead"
		elseif canShowDeputy then
			return "GameRule_AskForGeneralShowDeputy"
		end
	end

	local skillTrigger = false
	local skillnames = choices:split("+")
	table.removeOne(skillnames, "GameRule_AskForGeneralShowHead")
	table.removeOne(skillnames, "GameRule_AskForGeneralShowDeputy")
	table.removeOne(skillnames, "cancel")
	if #skillnames ~= 0 then
		skillTrigger = true
	end

	if skillTrigger then
		if string.find(choices, "jieming") then return "jieming" end
		if string.find(choices, "fankui") and string.find(choices, "ganglie") then return "fankui" end
		if string.find(choices, "xunxun") and string.find(choices, "ganglie") then return "ganglie" end
		if string.find(choices, "luoshen") and string.find(choices, "guanxing") then return "luoshen" end

		local except = {}
		for _, skillname in ipairs(skillnames) do
			local invoke = self:askForSkillInvoke(skillname, data)
			if invoke == true then
				return skillname
			elseif invoke == false then
				table.insert(except, skillname)
			end
		end
		if string.find(choices, "cancel") and not canShowHead and not canShowDeputy and sgs.isAnjiang(self.player) then
			return "cancel"
		end
		table.removeTable(skillnames, except)

		if #skillnames > 0 then return skillnames[math.random(1, #skillnames)] end
	end

	return "cancel"
end

sgs.ai_skill_choice["GameRule:TurnStart"] = function(self, choices, data)
	local choice = sgs.ai_skill_choice["GameRule:TriggerOrder"](self, choices, data)
	if choice == "cancel" then
		local canShowHead = string.find(choices, "GameRule_AskForGeneralShowHead")
		local canShowDeputy = string.find(choices, "GameRule_AskForGeneralShowDeputy")
		if canShowHead then
			if self.player:isDuanchang() then return "GameRule_AskForGeneralShowHead" end
			for _, p in ipairs(self.enemies) do
				if p:hasShownSkill("mingshi") then return "GameRule_AskForGeneralShowHead" end
			end
		elseif canShowDeputy then
			if self.player:isDuanchang() then return "GameRule_AskForGeneralShowDeputy" end
			for _, p in ipairs(self.enemies) do
				if p:hasShownSkill("mingshi") then return "GameRule_AskForGeneralShowDeputy" end
			end
		end

		if not self.player:hasShownOneGeneral() then
			local gameProcess = sgs.gameProcess():split(">>")
			if self.player:getKingdom() == gameProcess[1] and not self:willBeCareerist() then
				if canShowHead then return "GameRule_AskForGeneralShowDeputy"
				elseif canShowDeputy then return "GameRule_AskForGeneralShowDeputy" end
			end
		end

	end
	return choice
end

sgs.ai_skill_invoke.GameRule_AskForArraySummon = function(self, data)
	return self:willShowForDefence() or self:willShowForAttack()
end

sgs.ai_skill_invoke.SiegeSummon = true

sgs.ai_skill_invoke.FormationSummon = true
