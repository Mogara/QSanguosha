local juao_skill={}
juao_skill.name = "juao"
table.insert(sgs.ai_skills, juao_skill)
juao_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("JuaoCard") and self.player:getHandcardNum() > 1 then
		local card_id = self:getCardRandomly(self.player, "h")
		return sgs.Card_Parse("@JuaoCard=" .. card_id)
	end
end

sgs.ai_skill_use_func.JuaoCard = function(card, use, self)
	local givecard = {}
	local cards = self.player:getHandcards()
	for _, friend in ipairs(self.friends_noself) do
		if friend:getHp() == 1 then
			for _, hcard in sgs.qlist(cards) do
				if hcard:inherits("Analeptic") or hcard:inherits("Peach") then
					table.insert(givecard, hcard:getId())
				end
				if #givecard == 1 and givecard[1] ~= hcard:getId() and not hcard:inherits("Shit") then
					table.insert(givecard, hcard:getId())
				elseif #givecard == 2 then
					use.card = sgs.Card_Parse("@JuaoCard=" .. table.concat(givecard, "+"))
					if use.to then use.to:append(friend) end
					return
				end
			end
		end
		if friend:hasSkill("jizhi") then
			for _, hcard in sgs.qlist(cards) do
				if hcard:inherits("TrickCard") and not hcard:inherits("DelayedTrick") then
					table.insert(givecard, hcard:getId())
				end
				if #givecard == 1 and givecard[1] ~= hcard:getId() and not hcard:inherits("Shit") then
					table.insert(givecard, hcard:getId())
				elseif #givecard == 2 then
					use.card = sgs.Card_Parse("@JuaoCard=" .. table.concat(givecard, "+"))
					if use.to then use.to:append(friend) end
					return
				end
			end
		end
		if friend:hasSkill("leiji") then
			for _, hcard in sgs.qlist(cards) do
				if hcard:getSuit() == sgs.Card_Spade or hcard:inherits("Jink") then
					table.insert(givecard, hcard:getId())
				end
				if #givecard == 1 and givecard[1] ~= hcard:getId() and not hcard:inherits("Shit") then
					table.insert(givecard, hcard:getId())
				elseif #givecard == 2 then
					use.card = sgs.Card_Parse("@JuaoCard=" .. table.concat(givecard, "+"))
					if use.to then use.to:append(friend) end
					return
				end
			end
		end
		if friend:hasSkill("xiaoji") then
			for _, hcard in sgs.qlist(cards) do
				if hcard:inherits("EquipCard") then
					table.insert(givecard, hcard:getId())
				end
				if #givecard == 1 and givecard[1] ~= hcard:getId() and not hcard:inherits("Shit") then
					table.insert(givecard, hcard:getId())
				elseif #givecard == 2 then
					use.card = sgs.Card_Parse("@JuaoCard=" .. table.concat(givecard, "+"))
					if use.to then use.to:append(friend) end
					return
				end
			end
		end
	end
	givecard = {}
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() == 1 then
			for _, hcard in sgs.qlist(cards) do
				if hcard:inherits("Shit") or hcard:inherits("Disaster") then
					table.insert(givecard, hcard:getId())
				end
				if #givecard == 1 and givecard[1] ~= hcard:getId() and
					not hcard:inherits("Peach") and not hcard:inherits("TrickCard") then
					table.insert(givecard, hcard:getId())
					use.card = sgs.Card_Parse("@JuaoCard=" .. table.concat(givecard, "+"))
					if use.to then use.to:append(enemy) end
					return
				elseif #givecard == 2 then
					use.card = sgs.Card_Parse("@JuaoCard=" .. table.concat(givecard, "+"))
					if use.to then use.to:append(enemy) end
					return
				else
				end
			end
		end
	end
	if #givecard < 2 then
		for _, hcard in sgs.qlist(cards) do
			if hcard:inherits("Shit") or hcard:inherits("Disaster") then
				table.insert(givecard, hcard:getId())
			end
			if #givecard == 2 then
				use.card = sgs.Card_Parse("@JuaoCard=" .. table.concat(givecard, "+"))
				if use.to then use.to:append(self.enemies[1]) end
				return
			end
		end
	end
end

sgs.ai_skill_invoke.tanlan = function(self, data)
	local damage = data:toDamage()
	local max_card = self:getMaxCard()
	if not max_card or self:isFriend(damage.from) then return end
	if max_card:getNumber() > 10 or
		(self.player:getHp() > 2 and self.player:getHandcardNum() > 2 and max_card:getNumber() > 4) or
		(self.player:getHp() > 1 and self.player:getHandcardNum() > 1 and max_card:getNumber() > 7) or
		(damage.from:getHandcardNum() <= 2 and max_card:getNumber() > 2) then
		return true
	end
end

sgs.ai_skill_invoke.yicai = function(self, data)
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy, true) and self:getCardsNum("Slash") > 0 then return true end
	end
end

sgs.ai_skill_playerchosen.beifa = sgs.ai_skill_playerchosen.zero_card_as_slash

sgs.ai_chaofeng.wisjiangwei = 2

local houyuan_skill={}
houyuan_skill.name="houyuan"
table.insert(sgs.ai_skills,houyuan_skill)
houyuan_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("HouyuanCard") and self.player:getHandcardNum() > 1 then
		local givecard = {}
		local index = 0
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		for _, fcard in ipairs(cards) do
			if not fcard:inherits("Shit") then
				table.insert(givecard, fcard:getId())
				index = index + 1
			end
			if index == 2 then break end
		end
		if index < 2 then return end
		return sgs.Card_Parse("@HouyuanCard=" .. table.concat(givecard, "+"))
	end
end

sgs.ai_skill_use_func.HouyuanCard = function(card, use, self)
	if #self.friends == 1 then return end
	local target
	local max_x = 20
	for _, friend in ipairs(self.friends_noself) do
		local x = friend:getHandcardNum()
		if x < max_x then
			max_x = x
			target = friend
		end
	end
	if use.to then use.to:append(target) end
	use.card = card
	return
end

sgs.ai_card_intention.HouyuanCard = -70

sgs.ai_chaofeng.wisjiangwan = 6

sgs.ai_skill_invoke.bawang = function(self, data)
	local effect = data:toSlashEffect()
	local max_card = self:getMaxCard()
	if max_card and max_card:getNumber() > 10 then
		return self:isEnemy(effect.to)
	end
end

sgs.ai_skill_use["@@bawang"] = function(self, prompt)
	local first_index, second_index
	for i=1, #self.enemies do
		if not (self.enemies[i]:hasSkill("kongcheng") and self.enemies[i]:isKongcheng()) then
			if not first_index then
				first_index = i
			else
				second_index = i
			end
		end
		if second_index then break end
	end
	if not first_index then return "." end
	local first = self.enemies[first_index]:objectName()
	if not second_index then
		return ("@BawangCard=.->%s"):format(first)
	else
		local second = self.enemies[second_index]:objectName()
		return ("@BawangCard=.->%s+%s"):format(first, second)
	end
end

sgs.ai_card_intention.BawangCard = sgs.ai_card_intention.ShensuCard

sgs.ai_skill_use["@@weidai"] = function(self, prompt)
	return "@WeidaiCard=.->."
end

sgs.ai_skill_use_func.WeidaiCard = function(card, use, self)
	use.card = card
end

sgs.ai_card_intention.WeidaiCard = sgs.ai_card_intention.Peach

sgs.ai_skill_cardask["@weidai-analeptic"] = function(self, data)
	local who = data:toPlayer()
	if self:isEnemy(who) then return "." end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	for _, fcard in ipairs(cards) do
		if fcard:getSuit() == sgs.Card_Spade and fcard:getNumber() > 1 and fcard:getNumber() < 10 then
			return fcard:getEffectiveId()
		end
	end
	return "."
end

sgs.ai_chaofeng.wissunce = 1

sgs.ai_skill_playerchosen.longluo = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isFriend(player) and player:getHp() > player:getHandcardNum() then
			return player
		end
	end
	return self.friends_noself[1]
end

sgs.ai_skill_invoke.longluo = function(self, data)
	return #self.friends > 1
end

sgs.ai_skill_choice.fuzuo = function(self , choices)
	return "cancel"
end

sgs.ai_skill_invoke.jincui = function(self, data)
	return true
end

sgs.ai_skill_playerchosen.jincui = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isFriend(player) and player:getHp() - player:getHandcardNum() > 1 then
			return player
		end
	end
	if #self.friends > 1 then return self.friends_noself[1] end
	sgs.jincui_discard = true
	return self.enemies[1]
end

sgs.ai_skill_choice.jincui = function(self, choices)
	if sgs.jincui_discard then return "throw" else return "draw" end
end

sgs.ai_chaofeng.wiszhangzhao = -1

sgs.ai_skill_invoke.badao = function(self, data)
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy, true) and self:getCardsNum("Slash") > 0 then return true end
	end
end

sgs.ai_skill_invoke.shipo = function(self, data)
	local target = data:toPlayer()
	if ((target:containsTrick("supply_shortage") and target:getHp() > target:getHandcardNum()) or
		(target:containsTrick("indulgence") and target:getHandcardNum() > target:getHp()-1)) then
		return self:isFriend(target)
	end
end

sgs.ai_chaofeng.tianfeng = -3

local shouye_skill={}
shouye_skill.name = "shouye"
table.insert(sgs.ai_skills, shouye_skill)
shouye_skill.getTurnUseCard=function(self)
	if #self.friends_noself == 0 then return end
	if self.player:getHandcardNum() > 0 then
		local n = self.player:getMark("shouyeonce")
		if n > 0 and self.player:hasUsed("ShouyeCard") then return end
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		for _, hcard in ipairs(cards) do
			if hcard:isRed() then
				return sgs.Card_Parse("@ShouyeCard=" .. hcard:getId())
			end
		end
	end
end

sgs.ai_skill_use_func.ShouyeCard = function(card, use, self)
	self:sort(self.friends_noself, "handcard")
	if self.friends_noself[1] then
		if use.to then use.to:append(self.friends_noself[1]) end
	end
	if self.friends_noself[2] then
		if use.to then use.to:append(self.friends_noself[2]) end
	end
	use.card = card
	return
end

sgs.ai_card_intention.ShouyeCard = -70

sgs.ai_skill_invoke.shien = function(self, data)
	return self:isFriend(data:toPlayer())
end

sgs.ai_chaofeng.wisshuijing = 5
