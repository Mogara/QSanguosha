function sgs.ai_skill_invoke.wangxi(self, data)
	local target = data:toPlayer()
	if self:isFriend(target) then
		return not self:needKongcheng(target, true) and not (hasManjuanEffect(self.player) and hasManjuanEffect(target))
	else
		if hasManjuanEffect(self.player) then return false end
		return self:needKongcheng(target, true) or hasManjuanEffect(target)
	end
end

sgs.ai_choicemade_filter.skillInvoke.wangxi = function(self, player, promptlist)
	local damage = self.room:getTag("CurrentDamageStruct"):toDamage()
	local target = nil
	if damage.from and damage.from:objectName() == player:objectName() then
		target = damage.to
	elseif damage.to and damage.to:objectName() == player:objectName() then
		target = damage.from
	end
	if target and promptlist[3] == "yes" then
		if self:needKongcheng(target, true) then sgs.updateIntention(player, target, 10)
		elseif not hasManjuanEffect(target) and player:getState() == "robot" then sgs.updateIntention(player, target, -60)
		end
	end
end

function sgs.ai_skill_invoke.hengjiang(self, data)
	local target = data:toPlayer()
	if self:isEnemy(target) then
		return true
	else
		if hasManjuanEffect(self.player) then return false end
		if target:getPhase() > sgs.Player_Discard then return true end
		if target:hasSkill("yongsi") then return false end
		if target:hasSkill("keji") and not target:hasFlag("KejiSlashInPlayPhase") then return true end
		return target:getHandcardNum() <= target:getMaxCards() - 2
	end
end

sgs.ai_choicemade_filter.skillInvoke.hengjiang = function(self, player, promptlist)
	if promptlist[3] == "yes" then
		local current = self.room:getCurrent()
		if current and current:getPhase() <= sgs.Player_Discard
			and not (current:hasSkill("keji") and not current:hasFlag("KejiSlashInPlayPhase")) and current:getHandcardNum() > current:getMaxCards() - 2 then
			sgs.updateIntention(player, current, 50)
		end
	end
end

sgs.ai_skill_invoke.guixiu = function(self, data)
	return self:isWeak() and not self:willSkipPlayPhase()
end

sgs.ai_skill_invoke.guixiu_rec = function()
	return true
end

local cunsi_skill = {}
cunsi_skill.name = "cunsi"
table.insert(sgs.ai_skills, cunsi_skill)
cunsi_skill.getTurnUseCard = function(self)
	return sgs.Card_Parse("@CunsiCard=.")
end

sgs.ai_skill_use_func.CunsiCard = function(card, use, self)
	if sgs.turncount <= 2 and self.player:aliveCount() > 2 and #self.friends_noself == 0 then return end
	local to, manjuan
	for _, friend in ipairs(self.friends_noself) do
		if not hasManjuanEffect(friend) then
			to = friend
			break
		else
			manjuan = friend
		end
	end
	if not to and manjuan then to = manjuan end
	if not to then to = self.player end
	if self.player:getMark("guixiu") >= 1 then
		use.card = sgs.Card_Parse("@GuixiuCard=.")
		return
	else
		use.card = card
		if use.to then use.to:append(to) end
	end
end

sgs.ai_skill_use_func.GuixiuCard = function(card, use, self)
	use.card = card
end

sgs.ai_skill_invoke.yongjue = function(self, data)
	local player = data:toPlayer()
	return player and self:isFriend(player) and not (self:needKongcheng(player, true) and not self:hasCrossbowEffect(player))
end

sgs.ai_use_value.CunsiCard = 10
sgs.ai_use_priority.CunsiCard = 10.1
sgs.ai_use_priority.GuixiuCard = sgs.ai_use_priority.CunsiCard