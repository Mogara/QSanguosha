
sgs.ai_skill_choice.TriggerOrder = function(self, choices, data)
	if string.find(choices, "jieming") then return "jieming" end
	local choice = choices:split("+")[2]
	return choice
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
	if string.find(sgs.gameProcess(), self.player:getKingdom() .. ">>") and sgs.isAnjiang(self.player) then
		local upperlimit = self.player:getLord() and 99 or self.room:getPlayers():length() / 2
		if sgs.shown_kingdom[self.player:getKingdom()] < upperlimit  then return choices:split("+")[1] end
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
	elseif sgs.turncount < 2 then return "cancel" end
	choices = choices:split("+")
	
	local playerscount = self.room:getPlayers():length()
	
	if (self:getKingdomCount() > 0) and ((self:getKingdomCount() + 1) * 2 <= playerscount) then return choices[1] end
	if self.player:aliveCount() <= 5 then return choices[1] end
	return choices[math.random(1, #choices)]
end