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

sgs.ai_skill_choice.TriggerOrder = function(self, choices, data)
	self.TurnStartShowGeneral_Choice = nil
	if string.find(choices, "jieming") then return "jieming" end
	local skillnames = choices:split("+")
	table.removeOne(skillnames, "trigger_none")
	for _, skillname in ipairs(skillnames) do
		if self:askForSkillInvoke(skillname, data) then
			return skillname
		end
	end
	if string.find(choices, "trigger_none") then
		return "trigger_none"
	end
	return skillnames[math.random(1, #skillnames)]
end

sgs.ai_skill_invoke["userdefine:halfmaxhp"] = true

sgs.ai_skill_choice.CompanionEffect = function(self, choice, data)
	if self:isWeak() and string.find(choice, "recover") then return "recover"
	else return "draw" end
end

sgs.ai_skill_invoke["userdefine:FirstShowReward"] = true

--[[
sgs.ai_skill_choice.heg_nullification = function(self, choice, data)
	local effect = data:toCardEffect()
	if effect.card:isKindOf("AOE") then return "all" end
	return "single"
end
]]



sgs.ai_skill_choice.TurnStartShowGeneral = function(self, choices)

	local firstshow = ("luanji|qianhuan"):split("|")	
	local showboth = ("luanji+shuangxiong|luanji+huoshui|luoshen+fangzhu"):split("|")	
	local needshow = ("yicheng|qianhuan|chuanxin|suishi"):split("|")	
	local woundedshow = ("zaiqi|yinghun|hunshang|hengzheng"):split("|")
	local followshow = ("duoshi|rende|jieyin|xiongyi|shouyue|hongfa"):split("|")
	
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

	if notshown == 1  and not self.player:hasShownOneGeneral()  then 
		local cho = { "show_head_general", "show_deputy_general", "show_both_generals"}
		return cho[math.random(1, #cho)]
	end
	
	for _, skill in ipairs(showboth) do
		if self.player:hasSkills(skill) then
			return "show_both_generals"
		end
	end
	
	if shown == 0 then 
		for _, skill in ipairs(firstshow) do
			if self.player:hasSkill(skill) then
				if self.player:inHeadSkills(skill) then 
					return "show_head_general"
				else	
					return "show_deputy_general"
				end
			end
		end
	end	
		
	if shown > 2 and (f > 2 or e - f < 2) then 
		for _, skill in ipairs(needshow) do
			if self.player:hasSkill(skill) then
				if self.player:inHeadSkills(skill) then 
					return "show_head_general"
				else	
					return "show_deputy_general"
				end
			end
		end
	end	
		
	if self.player:getLostHp() >= 2 then 
		for _, skill in ipairs(woundedshow) do
			if self.player:hasSkill(skill) then
				if self.player:inHeadSkills(skill) then 
					return "show_head_general"
				else	
					return "show_deputy_general"
				end
			end
		end
	end		
	
	for _, skill in ipairs(followshow) do
		for _,p in sgs.qlist(self.room:getOtherPlayers(player)) do 
			if p:hasShownSkill(skill) and p:getKingdom() == self.player:getKingdom() and not self.player:hasShownOneGeneral()  then 
				local cho = { "show_head_general", "show_deputy_general", "show_both_generals"}
				return cho[math.random(1, #cho)]
			end
		end
	end		

	return "cancel"

	
end















