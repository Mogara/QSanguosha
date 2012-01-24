function SmartAI:useCardShit(card, use)
	if (card:getSuit() == sgs.Card_Heart or card:getSuit() == sgs.Card_Club) and self.player:isChained() and
		#(self:getChainedFriends()) > #(self:getChainedEnemies()) then return end
	if self.player:getHp()>3 and self.player:hasSkill("shenfen") and self.player:hasSkill("kuangbao") then use.card = card return end
	if self.player:hasSkill("kuanggu") and card:getSuitString() ~= "spade" then use.card = card return end
	if card:getSuit() == sgs.Card_Heart and (self:isEquip("GaleShell") or self:isEquip("Vine")) then return end
	if not self.player:isWounded() then
		if self:hasSkills(sgs.need_kongcheng) and self.player:getHandcardNum() == 1 then
			use.card = card
			return
		end
		if sgs[self.player:getGeneralName() .. "_suit_value"] and
			(sgs[self.player:getGeneralName() .. "_suit_value"][card:getSuitString()] or 0) > 0 then return end
		local peach = self:getCard("Peach")
		if peach then
			self:sort(self.friends, "hp")
			if not self:isWeak(self.friends[1]) then
				use.card = card
				return
			end
		end
	end
end

sgs.ai_skill_invoke.grab_peach = function(self, data)
	local struct = data:toCardUse()
	return self:isEnemy(struct.from) and (struct.to:isEmpty() or self:isEnemy(struct.to:first()))
end

function SmartAI:useGaleShell(card, use)
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
		if who:getRole() == "rebel" then
			for _, player in sgs.qlist(targets) do
				if self:isFriend(player) then
					return player
				end
			end
		elseif who:getRole() == "loyalist" then
			if self:isEnemy(who) then return self.room:getLord() end
		end
	end
	
	return self.enemies[1]
end
