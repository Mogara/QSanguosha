-- caochong
sgs.ai_skill_invoke.chengxiang = function(self, data)
	self.chengx = 0
	return not (self.player:hasSkill("manjuan") and self.player:getPhase() == sgs.Player_NotActive)
end

sgs.ai_skill_askforag.chengxiang = function(self, card_ids)
	local cards = {}
	for _, card_id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(card_id)
		table.insert(cards, card)
		if card:inherits("Peach") then
			if self.chengx + card:getNumber() < 13 then
				self.chengx = self.chengx + card:getNumber()
				return card_id
			end
		end
	end
	self:sortByUseValue(cards)
	for _, card in ipairs(cards) do
		if self.chengx + card:getNumber() < 13 then
			self.chengx = self.chengx + card:getNumber()
			return card:getEffectiveId()
		end
	end
	return card_ids[1]
end

sgs.ai_skill_invoke.renxin = function(self, data)
	local dying = data:toDying()
	if not self:isFriend(dying.who) then return false end
	local pile = self:getCardsNum("Peach")
	if pile > 0 then return false end
	if self:isWeak() then return math.random(0, 1) == 0 end
	return true
end

-- manchong
local junxing_skill={}
junxing_skill.name = "junxing"
table.insert(sgs.ai_skills, junxing_skill)
junxing_skill.getTurnUseCard=function(self)
    if not self.player:hasUsed("JunxingCard") and self.player:getHandcardNum() > 2 then
		local cards = sgs.QList2Table(self.player:getCards("h"))
		self:sortByUseValue(cards, true)
		return sgs.Card_Parse("@JunxingCard=" .. cards[1]:getEffectiveId())
	end
end
sgs.ai_skill_use_func["JunxingCard"]=function(card,use,self)
	self:sort(self.enemies)
	if #self.enemies > 0 then
		if use.to then use.to:append(self.enemies[1]) end
		use.card=card
	end
end

sgs.ai_skill_invoke.yuce = true
sgs.ai_skill_cardask["@yuce"] = function(self, data)
	local da = data:toDamage()
	if self:isFriend(da.to) or self.player:isKongcheng() then
		return "."
	else
		local card = da.card
		local cards = sgs.QList2Table(self.player:getCards("h"))
		self:sortByUseValue(cards, true)
		for _, acard in ipairs(cards) do
			if acard:getType() ~= card:getType() then
				return acard:getEffectiveId()
			end
		end
		return "."
	end
end

-- guohuai
sgs.ai_skill_invoke.jingce = true

-- jianyong
sgs.ai_skill_invoke.z0ngshi = true
sgs.ai_skill_use["@@qiaoshui"] = function(self, prompt)
	if self.player:getHandcardNum() < 2 then return "." end
	local max_card = self:getMaxCard()
	self:sort(self.enemies, "handcard")
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isKongcheng() then
			return "@QiaoShuiCard=" .. max_card:getEffectiveId() .. "->" .. enemy:objectName()
		end
	end
	return "."
end

sgs.ai_skill_choice["qiaoshui"] = function(self, choices)
	local use = data:toCardUse()
	if use.card:isKindOf("AOE") then
		for _, friend in ipairs(self.friends_noself) do
			if not self.room:isProhibited(self.player, friend, use.card) then
				self.qiaoshuitarget = friend
				return "remove"
			end
		end
	elseif use.card:isKindOf("GlobalEffect") then
		for _, enemy in ipairs(self.enemies) do
			if not self.room:isProhibited(self.player, enemy, use.card) then
				if use.card:isKindOf("GodSalvation") then
					if enemy:isWounded() then
						self.qiaoshuitarget = enemy
						return "remove"
					end
				else
					self.qiaoshuitarget = enemy
					return "remove"
				end
			end
		end
	else
		local useto = sgs.QList2Table(use.to)
		if #use.to > 0 and self:isEnemy(useto[1]) then
			for _, enemy in ipairs(self.enemies) do
				if not table.contains(useto, enemy) then
					self.qiaoshuitarget = enemy
					return "add"
				end
			end
		end
	end
	return "cancel"
end

sgs.ai_skill_playerchosen.qiaoshui = function(self, targets)
	return self.qiaoshuitarget
end

-- guanping
sgs.ai_skill_cardask["@longyin"] = function(self, data)
	if self.player:isNude() then return "." end
	local use = data:toCardUse()
	if self:isEnemy(use.from) then
		return "."
	else
		if self:getCardsNum("Slash", use.from) <= 1 then return "." end
		local cards = sgs.QList2Table(self.player:getCards("he"))
		self:sortByUseValue(cards, true)
		for _, acard in ipairs(cards) do
			if not acard:isKindOf("Slash") then
				return acard:getEffectiveId()
			end
		end
		return "."
	end
end

-- liufeng
sgs.ai_skill_use["@@xiansi"] = function(self, prompt)
	if self.player:getPile("counter"):length() >= 2 and self:isWeak() then return "." end
	self:sort(self.enemies)
	local target1, target2
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isNude() then
			if not target1 then
				target1 = enemy
			elseif target1 ~= enemy then
				target2 = enemy
			end
		end
	end
	if not target1 and not target2 then return "."
	elseif not target2 then return "@XiansiCard=.->" .. target1:objectName()
	else
		return ("@XiansiCard=.->%s+%s"):format(target1:objectName(), target2:objectName())
	end
end

local xiansiv_skill={}
xiansiv_skill.name = "xiansiv"
table.insert(sgs.ai_skills, xiansiv_skill)
xiansiv_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("XiansiSlashCard") then return end
	local liufeng = self.room:findPlayerBySkillName("xiansi")
	if not liufeng or liufeng:getPile("counter"):length() < 2 then return end
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	if slash:isAvailable(self.player) and self:isEnemy(liufeng) and self.player:canSlash(liufeng) then
		return sgs.Card_Parse("@XiansiSlashCard=.")
	end
end
sgs.ai_skill_use_func["XiansiSlashCard"]=function(card,use,self)
	use.card=card
end

-- yufan
sgs.ai_skill_invoke.zongxuan = function(self, data)
	return math.random(0, 1) == 0
end

sgs.ai_skill_invoke.zhiyan = true
sgs.ai_skill_playerchosen.zhiyan = function(self, targets)
	targets = sgs.QList2Table(targets)
	self:sort(targets, "defense")
	for _, friend in ipairs(targets) do
		if self:isFriend(friend) and friend:isWounded() then
			return friend
		end
	end
	return self.player
end

-- pan&ma
sgs.ai_skill_cardask["@duodao"] = function(self, data)
	local damage = data:toDamage()
	if self:isFriend(damage.from) or self.player:isNude() then
		return "."
	else
		local cards = sgs.QList2Table(self.player:getCards("he"))
		self:sortByUseValue(cards, true)
		return cards[1]:getEffectiveId()
	end
end

-- zhuran
sgs.ai_skill_invoke.danshou = function(self, data)
	if self.player ~= self.room:getCurrent() then return true end
	return math.random(0, 1) == 0
end

-- liru
sgs.ai_skill_invoke.juece = function(self, data)
	local move = data:toCardMove()
	return self:isEnemy(move.from)
end

fencheng_skill={}
fencheng_skill.name="fencheng"
table.insert(sgs.ai_skills, fencheng_skill)
fencheng_skill.getTurnUseCard=function(self)
	if self.player:getMark("@conflagration") <= 0 then return end
	local good, bad = 0, 0
	local lord = self.room:getLord()
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		local equipnum = player:getEquips():length()
		if equipnum == 0 then good = good - 1 end
		local handcn = player:getHandcardNum()
		if self:isEnemy(player) then
			if handcn >= equipnum then
				bad = bad + 1
			else -- fire damage
				good = good + 1
				if self:isEquip("Vine", player) or player:getMark("@kuangfeng") > 0 or self:isEquip("GaleShell", player) then
					good = good + 1
				end
			end
		else
			if handcn < equipnum then -- fire damage
				bad = bad + 1
				if self:isEquip("Vine", player) or player:getMark("@kuangfeng") > 0 or self:isEquip("GaleShell", player) then
					bad = bad + 1
				end
			else
				good = good + 1
			end
		end
	end
	if good > 2 and good > bad then return sgs.Card_Parse("@FenchengCard=.") end
end
sgs.ai_skill_use_func.FenchengCard=function(card,use,self)
	use.card = card
end

-- fuhuanghou
sgs.ai_skill_invoke.zhuikong = function(self, data)
	local player = self.room:getCurrent()
	local max_card = self:getMaxCard()
	if max_card:getNumber() > 11 then
		return self:isEnemy(player)
	end
end

sgs.ai_skill_invoke.qiuyuan = true
sgs.ai_skill_playerchosen.qiuyuan = function(self, targets)
	targets = sgs.QList2Table(targets)
	self:sort(targets, "defense")
	for _, enemy in ipairs(targets) do
		if self:isEnemy(enemy) then
			return enemy
		end
	end
	return targets[1]
end
