sgs.weapon_range.SPMoonSpear = 3

sgs.ai_skill_invoke.sp_moonspear = function(self, data)
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	for _, target in ipairs(self.enemies) do
		if self.player:canSlash(target) and not self:slashProhibit(slash ,target) then
		return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen.sp_moonspear = sgs.ai_skill_playerchosen.zero_card_as_slash

sgs.ai_chaofeng.yuanshu = 3

sgs.ai_skill_invoke.danlao = function(self, data)
	local effect = data:toCardEffect()
	if effect.card:inherits("GodSalvation") and self.player:isWounded() then
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
	if (self.jilei_source:hasSkill("paoxiao") or self:isEquip("Crossbow",self.jilei_source)) and self.jilei_source:inMyAttackRange(self.player) then
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
sgs.ai_chaofeng.sp_diaochan = sgs.ai_chaofeng.diaochan

sgs.ai_skill_invoke.guiwei = sgs.ai_skill_invoke.guixiang

sgs.ai_skill_invoke.pangde_guiwei = sgs.ai_skill_invoke.guixiang


