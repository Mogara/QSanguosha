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

sgs.ai_skill_invoke["userdefine:halfmaxhp"] = true

sgs.ai_skill_choice.CompanionEffect = function(self, choice, data)
	if ( self:isWeak() or self:needKongcheng(self.player, true) ) and string.find(choice, "recover") then return "recover"
	else return "draw" end
end

sgs.ai_skill_invoke["userdefine:firstShowReward"] = true

--[[
sgs.ai_skill_choice.heg_nullification = function(self, choice, data)
	local effect = data:toCardEffect()
	if effect.card:isKindOf("AOE") then return "all" end
	return "single"
end
]]


sgs.ai_skill_choice.TriggerOrder = function(self, choices,data)

	local canShowHead = string.find(choices, "GameRule_AskForGeneralShowHead") 
	local canShowDeputy = string.find(choices, "GameRule_AskForGeneralShowDeputy")

	local skillTrigger = false
	local skillnames = choices:split("+")
	table.removeOne(skillnames, "GameRule_AskForGeneralShowHead") 
	table.removeOne(skillnames, "GameRule_AskForGeneralShowDeputy") 
	table.removeOne(skillnames, "cancel") 
	if skillnames ~= {} then 
		skillTrigger = true
	end

	if skillTrigger then
		if string.find(choices, "jieming") then return "jieming" end 
		if string.find(choices, "fankui") and string.find(choices, "ganglie") then return "fankui" end
		if string.find(choices, "xunxun") and string.find(choices, "ganglie") then return "ganglie" end

		
		for _, skillname in ipairs(skillnames) do
			if self:askForSkillInvoke(skillname, data) then
				return skillname
			end
		end
		if string.find(choices, "cancel") 
			and not canShowHead 
			and not canShowDeputy then
			return "cancel"
		end
		return skillnames[math.random(1, #skillnames)]
	end
	
	
	local firstShow = ("luanji"):split("|")	
	local bothShow = ("luanji+shuangxiong|luanji+huoshui|luoshen+fangzhu"):split("|")	
	local needShowForAttack = ("chuanxin|suishi"):split("|")	
	local needShowForHelp = ("sushen|cunsi|yicheng|qianhuan"):split("|")	
	local needShowForLead = ("yicheng|qianhuan"):split("|")
	local woundedShow = ("zaiqi|yinghun|hunshang|hengzheng"):split("|")
	local followShow = ("duoshi|rende|jieyin|xiongyi|shouyue|hongfa"):split("|")
	
	local notshown,shown,f,e = 0,0,0,0
	for _,p in sgs.qlist(self.room:getAlivePlayers()) do
		if  not p:hasShownOneGeneral() then
			notshown = notshown + 1
		end
		if p:hasShownOneGeneral() then
			shown = shown + 1
			if p:getKingdom() == self.player:getKingdom() then
				f = f + 1
			else
				e = e + 1
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
	
	if shown == 0 then 
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
		
	if shown > 0 and e > 0 and e - f < 3 then 
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
		end	
		for _,p in sgs.qlist(self.room:getOtherPlayers(player)) do 
			if p:hasShownSkill(skill) and p:getKingdom() == self.player:getKingdom() and not self.player:hasShownOneGeneral()  then 
				local cho = { "GameRule_AskForGeneralShowHead", "GameRule_AskForGeneralShowDeputy"}
				return cho[math.random(1, #cho)]
			end
		end
	end		

	if notshown == 1  and not self.player:hasShownOneGeneral()  then 
		local cho = { "GameRule_AskForGeneralShowHead", "GameRule_AskForGeneralShowDeputy"}
		return cho[math.random(1, #cho)]
	end
		
	return "cancel"
end















