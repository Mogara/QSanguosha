sgs.ai_skill_invoke.grab_peach = function(self, data)
	local struct = data:toCardUse()
	return self:isEnemy(struct.from) and (struct.to:isEmpty() or self:isEnemy(struct.to:first()))
end

function SmartAI:useCardGaleShell(card, use)
	use.broken = true
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy) <=1 and not self:hasSkills("jijiu|wusheng|longhun",enemy) then
			use.card = card
			if use.to then
				use.to:append(enemy)
			end
			return
		end
	end
end

sgs.ai_armor_value["gale-shell"] = function()
	return -10
end

sgs.ai_card_intention.GaleShell = 80
sgs.ai_use_priority.GaleShell = 0.9

sgs.dynamic_value.control_card.GaleShell = true

sgs.weapon_range.YxSword = 3

sgs.ai_skill_invoke.yx_sword = function(self, data)
	local damage= data:toDamage()
	local dmg = damage.damage
	if damage.to:getArmor() and damage.to:getArmor():objectName() == "vine" and damage.nature == sgs.DamageStruct_Fire then dmg = dmg + 1 end
	if damage.to:getArmor() and damage.to:getArmor():objectName() == "silver_lion" then dmg = 1 end

	if (damage.to:hasSkill("duanchang") and damage.to:getHp() - dmg < 1) or self:hasSkills("ganglie|fankui|enyuan", damage.to) then
		return true
	end
end

sgs.ai_skill_playerchosen.yx_sword = function(self, targets)
	local who = self.room:getTag("YxSwordVictim"):toPlayer()
	if who then
		if sgs.evaluateRoleTrends(who) == "rebel" then
			for _, player in sgs.qlist(targets) do
				if self:isFriend(player) then
					return player
				end
			end
		elseif sgs.evaluateRoleTrends(who) == "loyalist" then
			if self:isEnemy(who) then return self.room:getLord() end
		end
	end
	
	return self.enemies[1]
end
