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
	if effect.card:inherits("GodSalvation") and self.player:isWounded() then
		return false
	elseif effect.card:inherits("AmazingGrace") and
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
	if (self:isEquip("Crossbow",self.jilei_source) and self.jilei_source:inMyAttackRange(self.player)) or
		 self.jilei_source:isJilei(tmptrick) then
		return "basic"
	else
		return "trick"
	end
end

local function yuanhu_validate(self, equip_type, is_handcard)
	local is_SilverLion = false
	if equip_type == "SilverLion" then
		equip_type = "Armor"
		is_SilverLion = true
	end
	local targets
	if is_handcard then targets = self.friends else targets = self.friends_noself end
	if equip_type ~= "Weapon" then
		if equip_type == "DefensiveHorse" or equip_type == "OffensiveHorse" then self:sort(targets, "hp") end
		if equip_type == "Armor" then self:sort(targets, "handcard") end
		if is_SilverLion then
			for _, enemy in ipairs(self.enemies) do
				if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
					local seat_diff = enemy:getSeat() - self.player:getSeat()
					local alive_count = self.room:alivePlayerCount()
					if seat_diff < 0 then seat_diff = seat_diff + alive_count end
					if seat_diff > alive_count / 2.5 + 1 then return enemy	end
				end
			end
			for _, enemy in ipairs(self.enemies) do
				if enemy:hasSkill("bazhen") or enemy:hasSkill("yizhong") then
					return enemy
				end
			end
		end
		for _, friend in ipairs(targets) do
			if not self:isEquip(equip_type, friend) then
				if equip_type == "Armor" then
					if not self:needKongcheng(friend) and not self:hasSkills("bazhen|yizhong", friend) then return friend end
				else
					if friend:isWounded() and not friend:hasSkill("longhun") then return friend end
				end
			end
		end
	else
		for _, friend in ipairs(targets) do
			if not self:isEquip(equip_type, friend) then
				for _, aplayer in sgs.qlist(self.room:getAllPlayers()) do
					if friend:distanceTo(aplayer) == 1 then
						if self:isFriend(aplayer) and not aplayer:containsTrick("YanxiaoCard")
							and (aplayer:containsTrick("indulgence") or aplayer:containsTrick("supply_shortage")
								or (aplayer:containsTrick("lightning") and self:hasWizard(self.enemies))) then
							self.room:setPlayerFlag(aplayer, "YuanhuToChoose")
							return friend
						end
					end
				end
				self:sort(self.enemies, "defense")
				for _, enemy in ipairs(self.enemies) do
					if friend:distanceTo(enemy) == 1 and not enemy:isNude() then
						self.room:setPlayerFlag(enemy, "YuanhuToChoose")
						return friend
					end
				end
			end
		end
	end
	return nil
end

sgs.ai_skill_use["@@yuanhu"] = function(self, prompt)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	if self:isEquip("SilverLion") and yuanhu_validate(self, "SilverLion", false) then
		local player = yuanhu_validate(self, "SilverLion", false)
		local card_id = self.player:getArmor():getEffectiveId()
		return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
	end
	if self.player:getOffensiveHorse() and yuanhu_validate(self, "OffensiveHorse", false) then
		local player = yuanhu_validate(self, "OffensiveHorse", false)
		local card_id = self.player:getOffensiveHorse():getEffectiveId()
		return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
	end
	if self.player:getWeapon() and yuanhu_validate(self, "Weapon", false) then
		local player = yuanhu_validate(self, "Weapon", false)
		local card_id = self.player:getWeapon():getEffectiveId()
		return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
	end
	if self.player:getArmor() and self.player:getLostHp() <= 1 and self.player:getHandcardNum() >= 3
		and yuanhu_validate(self, "Armor", false) then
		local player = yuanhu_validate(self, "Armor", false)
		local card_id = self.player:getArmor():getEffectiveId()
		return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("DefensiveHorse") and yuanhu_validate(self, "DefensiveHorse", true) then
			local player = yuanhu_validate(self, "DefensiveHorse", true)
			local card_id = card:getEffectiveId()
			return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
		end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("OffensiveHorse") and yuanhu_validate(self, "OffensiveHorse", true) then
			local player = yuanhu_validate(self, "OffensiveHorse", true)
			local card_id = card:getEffectiveId()
			return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
		end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("Weapon") and yuanhu_validate(self, "Weapon", true) then
			local player = yuanhu_validate(self, "Weapon", true)
			local card_id = card:getEffectiveId()
			return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
		end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("SilverLion") and yuanhu_validate(self, "SilverLion", true) then
			local player = yuanhu_validate(self, "SilverLion", true)
			local card_id = card:getEffectiveId()
			return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
		end
		if card:isKindOf("Armor") and yuanhu_validate(self, "Armor", true) then
			local player = yuanhu_validate(self, "Armor", true)
			local card_id = card:getEffectiveId()
			return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
		end
	end
end

sgs.ai_skill_playerchosen.yuanhu = function(self, targets)
	targets = sgs.QList2Table(targets)
	for _, p in ipairs(targets) do
		if p:hasFlag("YuanhuToChoose") then 
			self.room:setPlayerFlag(p, "-YuanhuToChoose")
			return p 
		end 
	end
	for _, p in sgs.qlist(self.room:getAllPlayers()) do
		if p:hasFlag("YuanhuToChoose") then 
			self.room:setPlayerFlag(p, "-YuanhuToChoose")
		end 
	end
end

sgs.ai_card_intention.YuanhuCard = -30

sgs.ai_skill_use["@@bifa"] = function(self, prompt)
	if #self.enemies == 0 then return "." end
	self:sort(self.enemies)
	local cards = sgs.QList2Table(self.player:getCards("h"))
	self:sortByUseValue(cards, true)
	if self:getUseValue(cards[1]) < 5 then
		return "@BifaCard=" .. cards[1]:getEffectiveId() .. "->" .. self.enemies[1]:objectName()
	end
	return "."
end

sgs.ai_skill_cardask["@bifa-give"] = function(self, data, pattern, target)
	local pen = data:toCard()
	local typpe = pen:getType()
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)
	for _, c in ipairs(cards) do
		if c:getType() == typpe and c ~= pen then
		    return c:getId()
		end
	end
	return "."
end

sgs.ai_skill_invoke["#xiangxiangp"] = function(self, data)
	local lord = self.room:getLord()
	if lord:hasLordSkill("shichou") then
		return self:isFriend(lord)
	end
	return lord:getKingdom() == "shu"
end

sgs.ai_chaofeng.sp_sunshangxiang = sgs.ai_chaofeng.sunshangxiang

sgs.ai_skill_invoke["#ducaip"] = function(self, data)
	local lord = self.room:getLord()
	if lord:hasLordSkill("xueyi") then
		return not self:isFriend(lord)
	end
	return lord:getKingdom() == "wei"
end

sgs.ai_chaofeng.sp_caiwenji = sgs.ai_chaofeng.caiwenji

sgs.ai_skill_invoke["#guanyup"] = function(self, data)
	local lord = self.room:getLord()
	if lord:hasLordSkill("jijiang") then
		return not self:isFriend(lord)
	end
	return lord:getKingdom() == "wei"
end

sgs.ai_skill_invoke["#machaop"] = function(self, data)
	local lord = self.room:getLord()
	if lord:hasSkill("xueyi") then
		return self:isFriend(lord) 
	end
	if lord:hasLordSkill("shichou") then
		return not self:isFriend(lord)
	end
	
	return lord:getKingdom() == "qun"
end

sgs.ai_chaofeng.sp_machao = sgs.ai_chaofeng.machao

sgs.ai_skill_invoke["#diaochanp"] = function(self, data)
	return math.random(0, 2) == 0
end

sgs.ai_chaofeng.sp_diaochan = sgs.ai_chaofeng.diaochan

sgs.ai_skill_invoke["#pangdep"] = sgs.ai_skill_invoke["#ducaip"]

sgs.ai_skill_invoke["#jiaxup"] = sgs.ai_skill_invoke["#ducaip"]
