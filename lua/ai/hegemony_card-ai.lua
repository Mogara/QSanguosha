-- yuanjiaojingong
function SmartAI:useCardAllyFarAttackNear(far, use)
	local target
	self:sort(self.friends_noself, "defense")
	for _, friend in ipairs(self.friends_noself) do
		if friend:getKingdom() ~= self.player:getKingdom() and
			self:hasTrickEffective(far, friend) then
			target = friend
			break
		end
	end
	if target then
		use.card = far
		if use.to then use.to:append(target) end
	end
	return "."
end

-- yiyidailao
function SmartAI:useCardEaseVSFatigue(vs, use)
	local f = 1
	for _, friend in ipairs(self.friends) do
		if self:hasTrickEffective(vs, friend) and friend:getKingdom() == self.player:getKingdom() then
			f = f + 1
		end
	end
	local e = 0
	for _, enemy in ipairs(self.enemies) do
		if self:hasTrickEffective(vs, enemy) and enemy:getKingdom() == self.player:getKingdom() then
			e = e + 1
		end
	end
	if e > f then return "." end
	use.card = vs
end

-- zhijizhibi
function SmartAI:useCardKnowThyself(thy, use)
	use.card = thy
end

sgs.weapon_range.TriDouble = 3
sgs.ai_use_priority.TriDouble = sgs.ai_use_priority.Blade

sgs.ai_skill_cardask["@tri_double"] = function(self, data)
	if self.player:isKongcheng() then return "." end
	local damage = data:toDamage()
	local target
	for _, tmp in sgs.qlist(self.room:getOtherPlayers(damage.to)) do
		if damage.to:distanceTo(tmp) == 1 and self:isEnemy(tmp) then
			target = tmp
			break
		end
	end
	if not target then return "." end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)
	self.tritarget = target
	return cards[1]:getEffectiveId()
end

sgs.ai_skill_playerchosen.tri_double = function(self, targets)	
	return self.tritarget
end

sgs.weapon_range.WuLiuJian = 2
sgs.ai_use_priority.WuLiuJian = 2.6
