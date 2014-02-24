
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

sgs.ai_skill_choice.heg_nullification = function(self, choice, data)
	local effect = data:toCardEffect()
	if effect.card:isKindOf("AOE") then return "all" end
	return "single"
end

sgs.ai_skill_choice.TurnStartShowGeneral = function(self, choices)
	local function getReturnValue(head)
		return head and "show_head_general" or "show_deputy_general"
	end

	local must_show = ("shushen|shoucheng|yicheng|qianhuan|cunsi|chuanxin"):split("|")
	local must_not_show = ("baoling"):split("|")
	local not_show = {}
	for _, skill in ipairs(must_not_show) do
		if self.player:ownSkill(skill) then
			not_show[self.player:inHeadSkills(skill)] = true
		end
	end
	
	local show = {}
	for _, skill in ipairs(must_show) do
		if self.player:ownSkill(skill) and (not not_show[self.player:inHeadSkills(skill)]) then
			show[self.player:inHeadSkills(skill)] = true
		end
	end
	if (show[true] and show[false]) then
		return "show_both_generals"
	else
		for _, a in ipairs(show) do
			if show[a] then
				return getReturnValue(a)
			end
		end
	end
	
	local should_show = false
	local kingdom = self.player:getActualGeneral2():getKingdom()
	if self.room:getLord(kingdom) then
		should_show = true
	elseif sgs.shown_kingdom[kingdom] < self.room:getPlayers():length() / 2 then
		should_show = true
	end
	
	if should_show then
		if not_show[true] then
			return getReturnValue(true)
		else
			return getReturnValue(false)
		end
	end
	return "cancel"

--[[
	if string.find(sgs.gameProcess(), self.player:getKingdom() .. ">>") and sgs.isAnjiang(self.player) then
		local upperlimit = self.player:getLord() and 99 or self.room:getPlayers():length() / 2
		if sgs.shown_kingdom[self.player:getKingdom()] < upperlimit then return choices:split("+")[1] end
	end
	if self.TurnStartShowGeneral_Choice then
		local choice = self.TurnStartShowGeneral_Choice
		self.TurnStartShowGeneral_Choice = nil
		return choice
	end
	local skills_to_show = "zaiqi|buqu|kuanggu|guanxing|luoshen|tuxi|zhiheng|qiaobian|longdan"
							.. "|liuli|wushuang|niepan"
	local show_head_general, show_deputy_general
	for _, skname in ipairs(skills_to_show:split("|")) do
		if self.player:inHeadSkills(skname) then show_head_general = true
		elseif self.player:ownSkill(skname) then show_deputy_general = true end
		if show_head_general or show_deputy_general then
			if string.find(choices, "show_both_generals") then return "show_both_generals"
			else break end
		end
	end

	if show_head_general and string.find(choices, "show_head_general") then return "show_head_general"
	elseif show_deputy_general and string.find(choices, "show_deputy_general") then return "show_deputy_general"
	elseif sgs.turncount <= 2 then return "cancel" end
	choices = choices:split("+")

	local playerscount = self.room:getPlayers():length()

	if (self:getKingdomCount() > 0) and ((self:getKingdomCount() + 1) * 2 <= playerscount) then return choices[1] end
	if self.player:aliveCount() <= 5 then return choices[1] end
	return choices[math.random(1, #choices)]
	
]]
end