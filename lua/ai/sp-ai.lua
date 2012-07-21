sgs.weapon_range.SPMoonSpear = 3

sgs.ai_skill_invoke.SPMoonSpear = function(self, data)
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	for _, target in ipairs(self.enemies) do
		if self.player:canSlash(target) and not self:slashProhibit(slash ,target) then
		return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen.SPMoonSpear = sgs.ai_skill_playerchosen.zero_card_as_slash
sgs.ai_playerchosen_intention.sp_moonspaer = 80

function sgs.ai_slash_prohibit.weidi(self, to, card)
	if to:isLord() then return false end
	local lord = self.room:getLord()
	for _, askill in sgs.qlist(lord:getVisibleSkillList()) do
		if askill:objectName() ~= "weidi" and askill:isLordSkill() then
			local filter = sgs.ai_slash_prohibit[askill:objectName()]
			if  type(filter) == "function" and filter(self, to, card) then return true end
		end
	end
end

sgs.ai_chaofeng.yuanshu = 3

sgs.ai_skill_invoke.danlao = function(self, data)
	local effect = data:toCardUse()
	local current = self.room:getCurrent()
	if effect.card:isKindOf("GodSalvation") and self.player:isWounded() then
		return false
	elseif effect.card:isKindOf("AmazingGrace") and
		(self.player:getSeat() - current:getSeat()) % (global_room:alivePlayerCount()) < global_room:alivePlayerCount()/2 then
		return false
	else
		return true
	end
end

sgs.ai_skill_invoke.jilei = function(self, data)
	local damage = data:toDamage()
	if not damage then return false end
	self.jilei_source = damage.from
	return self:isEnemy(damage.from)
end	

sgs.ai_skill_choice.jilei = function(self, choices)
	local tmptrick = sgs.Sanguosha:cloneCard("ex_nihilo", sgs.Card_NoSuit, 0)
	if (self:isEquip("CrossBow",self.jilei_source) and self.jilei_source:inMyAttackRange(self.player)) or
		 self.jilei_source:isJilei(tmptrick) then
		return "basic"
	else
		return "trick"
	end
end

sgs.ai_skill_invoke.chujia = function(self, data)
	return self.room:getLord():getKingdom() == "shu"
end

sgs.ai_chaofeng.sp_sunshangxiang = sgs.ai_chaofeng.sunshangxiang

sgs.ai_skill_invoke.guixiang = function(self, data)
	return self.room:getLord():getKingdom() == "wei"
end

sgs.ai_chaofeng.sp_caiwenji = sgs.ai_chaofeng.caiwenji

sgs.ai_skill_invoke.fanqun = function(self, data)
	local lord = self.room:getLord()
	return self:isFriend(lord) and lord:getKingdom() == "qun"
end

sgs.ai_chaofeng.sp_machao = sgs.ai_chaofeng.machao

sgs.ai_skill_invoke.tuoqiao = function(self, data)
	if math.random(0, 2) == 0 then return false
	elseif math.random(0, 2) == 0  then sgs.ai_skill_choice.tuoqiao="SP-Diaochan" return true
	else sgs.ai_skill_choice.tuoqiao="BGM-Diaochan" return true end
end

sgs.ai_chaofeng.sp_diaochan = sgs.ai_chaofeng.diaochan

sgs.ai_skill_invoke.guiwei = sgs.ai_skill_invoke.guixiang

sgs.ai_skill_invoke.pangde_guiwei = sgs.ai_skill_invoke.guixiang


