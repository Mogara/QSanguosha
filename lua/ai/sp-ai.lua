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
sgs.ai_playerchosen_intention.SPMoonSpear = 80

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
		local player = yuanhu_validate(self, "Weapon", false)
		local card_id = self.player:getWeapon():getEffectiveId()
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

xueji_skill={}
xueji_skill.name="xueji"
table.insert(sgs.ai_skills,xueji_skill)
xueji_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("XuejiCard") then return end

	local card
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)

	for _, acard in ipairs(cards) do
		if acard:isRed() then
			card = acard
			break
		end
	end
	if card then
		card = sgs.Card_Parse("@XuejiCard=" .. card:getEffectiveId())
		return card
	end

	return nil
end

function can_be_selected_as_target(self, card, who)
	-- validation of rule
	if self.player:getWeapon() and self.player:getWeapon():getEffectiveId() == card:getEffectiveId() then
		if self.player:distanceTo(who) > 1 then return false end
	elseif self.player:getOffensiveHorse() and self.player:getOffensiveHorse():getEffectiveId() == card:getEffectiveId() then
		if self.player:distanceTo(who, 1) > self.player:getAttackRange() then return false end
	elseif self.player:distanceTo(who) > self.player:getAttackRange() then
		return false 
	end
	-- validation of strategy
	if self:cantbeHurt(who) or who:getMark("@fog") >= 1 or who:getMark("@fenyong") >= 1 then return false end
	if self:isEnemy(who) then
		if not self.player:hasSkill("jueqing") then
			if who:hasSkill("guixin") and (self.room:getAliveCount() >= 4 or not who:faceUp()) then return false end
			if who:hasSkill("jieming") then
				for _, enemy in ipairs(self.enemies) do
					if enemy:getHandcardNum() <= enemy:getMaxHp() - 2 and not enemy:hasSkill("manjuan") then return false end
				end
			end
			if who:hasSkill("fangzhu") then
				for _, enemy in ipairs(self.enemies) do
					if not enemy:faceUp() then return false end
				end
			end
		end
		return true
	elseif self:isFriend(who) then
		if who:hasSkill("yiji") and not self.player:hasSkill("jueqing") then
			local huatuo = self.room:findPlayerBySkillName("jijiu")
			if (huatuo and self:isFriend(huatuo) and huatuo:getHandcardNum() >= 3 and huatuo ~= self.player) 
				or (who:getLostHp() == 0 and who:getMaxHp() >= 3) then 
				return true 
			end 
		end
		if who:hasSkill("hunzi") and who == self.player:getNextAlive() and who:getHp() == 2 then return true end
		return false
	end
	return false
end

sgs.ai_skill_use_func.XuejiCard=function(card,use,self)
	if self.player:getLostHp() == 0 or self.player:hasUsed("XuejiCard") then return end
	self:sort(self.enemies)
	local to_use = false
	for _, enemy in ipairs(self.enemies) do
		if can_be_selected_as_target(self, card, enemy) then
			to_use = true
			break
		end
	end
	if not to_use then
		for _, friend in ipairs(self.friends_noself) do
			if can_be_selected_as_target(self, card, friend) then
				to_use = true
				break
			end
		end
	end
	if to_use then
		use.card = card
		if use.to then
			for _, enemy in ipairs(self.enemies) do
				if can_be_selected_as_target(self, card, enemy) then
					use.to:append(enemy)
					if use.to:length() == self.player:getLostHp() then return end
				end
			end
			for _, friend in ipairs(self.friends_noself) do
				if can_be_selected_as_target(self, card, friend) then
					use.to:append(friend)
					if use.to:length() == self.player:getLostHp() then return end
				end
			end
			assert(use.to:length() > 0)
		end
	end
end

sgs.ai_use_value.XuejiCard = 3
sgs.ai_use_priority.XuejiCard = 2.2

sgs.ai_skill_use["@@bifa"] = function(self, prompt)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	self:sort(self.enemies, "handcard")
	if #self.enemies > 0 then
		for _, c in ipairs(cards) do
			if c:isKindOf("EquipCard") then return "@BifaCard=" .. c:getEffectiveId() .. "->" .. self.enemies[1]:objectName() end
		end
		for _, c in ipairs(cards) do
			if c:isKindOf("TrickCard") and not (c:isKindOf("Nullification") and self:getCardsNum("Nullification") == 1) then 
				return "@BifaCard=" .. c:getEffectiveId() .. "->" .. self.enemies[1]:objectName() 
			end
		end
		for _, c in ipairs(cards) do
			if c:isKindOf("Slash") then 
				return "@BifaCard=" .. c:getEffectiveId() .. "->" .. self.enemies[1]:objectName() 
			end
		end
	end
end

sgs.ai_skill_cardask["@bifa-give"] = function(self, data)
	local card_type = data:toString()
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards)
	for _, c in ipairs(cards) do
		if c:isKindOf(card_type) and not c:isKindOf("Peach") and not c:isKindOf("ExNihilo") then
			return "$" .. c:getEffectiveId()
		end
	end
	return "."
end

local songci_skill={}
songci_skill.name="songci"
table.insert(sgs.ai_skills, songci_skill)
songci_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("SongciCard") then return end
	return sgs.Card_Parse("@SongciCard=.")
end

sgs.ai_skill_use_func.SongciCard = function(card,use,self)
	self:sort(self.friends, "handcard")
	for _, friend in ipairs(self.friends) do
		if friend:getMark("@songci") == 0 and friend:getHandcardNum() < friend:getHp() and not (friend:hasSkill("manjuan") and self.room:getCurrent() ~= friend) then
			if not (friend:hasSkill("haoshi") and friend:getHandcardNum() <= 1 and friend:getHp() >= 3) then
				use.card = sgs.Card_Parse("@SongciCard=.")
				if use.to then use.to:append(friend) end
				return
			end
		end
	end
	
	self:sort(self.enemies, "handcard", true)
	for _, enemy in ipairs(self.enemies) do
		if enemy:getMark("@songci") == 0 and enemy:getHandcardNum() > enemy:getHp() and not enemy:isNude() then
			if not ((self:hasSkills(sgs.lose_equip_skill, enemy) and enemy:getEquips():length() > 0) 
			        or (self:isEquip("SilverLion", enemy) and enemy:isWounded())) then
				use.card = sgs.Card_Parse("@SongciCard=.")
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end
end

sgs.ai_skill_invoke.chujia = function(self, data)
	local lord = self.room:getLord()
	if lord:hasLordSkill("shichou") then
		return self:isFriend(lord)
	end
	return lord:getKingdom() == "shu"
end

sgs.ai_chaofeng.sp_sunshangxiang = sgs.ai_chaofeng.sunshangxiang

sgs.ai_skill_invoke.guixiang = function(self, data)
	local lord = self.room:getLord()
	if lord:hasLordSkill("xueyi") then
		return not self:isFriend(lord)
	end
	return lord:getKingdom() == "wei"
end

sgs.ai_chaofeng.sp_caiwenji = sgs.ai_chaofeng.caiwenji

sgs.ai_skill_invoke.fanqun = function(self, data)
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

sgs.ai_skill_invoke.tuoqiao = function(self, data)
	if math.random(0, 2) == 0 then return false
	elseif math.random(0, 2) == 0  then sgs.ai_skill_choice.tuoqiao="SP-Diaochan" return true
	else sgs.ai_skill_choice.tuoqiao="BGM-Diaochan" return true end
end

sgs.ai_chaofeng.sp_diaochan = sgs.ai_chaofeng.diaochan

sgs.ai_skill_invoke.guiwei = sgs.ai_skill_invoke.guixiang

sgs.ai_skill_invoke.pangde_guiwei = sgs.ai_skill_invoke.guixiang


