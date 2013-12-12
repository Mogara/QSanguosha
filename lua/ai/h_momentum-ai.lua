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