sgs.weapon_range.SPMoonSpear = 3

sgs.ai_skill_playerchosen.SPMoonSpear = function(self, targets)
	targets = sgs.QList2Table(targets)
	self:sort(targets, "defense")
	for _, target in ipairs(targets) do
		if self:isEnemy(target) and self:damageIsEffective(target) and sgs.isGoodTarget(target, targets, self) then
			return target
		end
	end
	return nil
end

sgs.ai_playerchosen_intention.SPMoonSpear = 80

function sgs.ai_slash_prohibit.weidi(self, from, to, card)
	local lord = self.room:getLord()
	if not lord then return false end
	if to:isLord() then return false end	
	for _, askill in sgs.qlist(lord:getVisibleSkillList()) do
		if askill:objectName() ~= "weidi" and askill:isLordSkill() then
			local filter = sgs.ai_slash_prohibit[askill:objectName()]
			if type(filter) == "function" and filter(self, from, to, card) then return true end
		end
	end
end

sgs.ai_skill_use["@jijiang"] = function(self, prompt)
	if self.player:hasFlag("Global_JijiangFailed") then return "." end
	local card = sgs.Card_Parse("@JijiangCard=.")
	local dummy_use = { isDummy = true }
	self:useSkillCard(card, dummy_use)
	if dummy_use.card then
		local jijiang = {}
		if sgs.jijiangtarget then
			for _, p in ipairs(sgs.jijiangtarget) do
				table.insert(jijiang, p:objectName())
			end
			return "@JijiangCard=.->" .. table.concat(jijiang, "+")
		end
	end
	return "."
end

--[[
	技能：庸肆（弃牌部分）
	备注：为了解决场上有古锭刀时弃白银狮子的问题而重写此弃牌方案。
]]--
sgs.ai_skill_discard.yongsi = function(self, discard_num, min_num, optional, include_equip)
	self:assignKeep(self:assignKeepNum(), true)
	if optional then 
		return {} 
	end
	local flag = "h"
	local equips = self.player:getEquips()
	if include_equip and not (equips:isEmpty() or self.player:isJilei(equips:first())) then flag = flag .. "e" end
	local cards = self.player:getCards(flag)
	local to_discard = {}
	cards = sgs.QList2Table(cards)
	local aux_func = function(card)
		local place = self.room:getCardPlace(card:getEffectiveId())
		if place == sgs.Player_PlaceEquip then
			if card:isKindOf("SilverLion") then
				local players = self.room:getOtherPlayers(self.player) 
				for _,p in sgs.qlist(players) do
					local blade = p:getWeapon()
					if blade and blade:isKindOf("GudingBlade") then
						if p:inMyAttackRange(self.player) then
							if self:isEnemy(p, self.player) then
								return 6
							end
						else
							break --因为只有一把古锭刀，检测到有人装备了，其他人就不会再装备了，此时可跳出检测。
						end
					end
				end
				if self.player:isWounded() then 
					return -2
				end
			elseif card:isKindOf("Weapon") and self.player:getHandcardNum() < discard_num + 2 and not self:needKongcheng() then return 0
			elseif card:isKindOf("OffensiveHorse") and self.player:getHandcardNum() < discard_num + 2 and not self:needKongcheng() then return 0
			elseif card:isKindOf("OffensiveHorse") then return 1
			elseif card:isKindOf("Weapon") then return 2
			elseif card:isKindOf("DefensiveHorse") then return 3
			elseif self:hasSkills("bazhen|yizhong") and card:isKindOf("Armor") then return 0
			elseif card:isKindOf("Armor") then
				return 4
			end
		elseif self:hasSkills(sgs.lose_equip_skill) then 
			return 5
		else 
			return 0 
		end
		return 0
	end
	local compare_func = function(a, b)
		if aux_func(a) ~= aux_func(b) then return aux_func(a) < aux_func(b) end
		return self:getKeepValue(a) < self:getKeepValue(b)
	end

	table.sort(cards, compare_func)
	local least = min_num
	if discard_num - min_num > 1 then
		least = discard_num -1
	end
	for _, card in ipairs(cards) do
		if not self.player:isJilei(card) then 
			table.insert(to_discard, card:getId()) 
		end
		if (self.player:hasSkill("qinyin") and #to_discard >= least) or #to_discard >= discard_num then 
			break 
		end
	end
	return to_discard
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
	local tmptrick = sgs.Sanguosha:cloneCard("ex_nihilo")
	if (self:hasCrossbowEffect(self.jilei_source) and self.jilei_source:inMyAttackRange(self.player))
		or self.jilei_source:isCardLimited(tmptrick, sgs.Card_MethodUse, true) then
		return "BasicCard"
	else
		return "TrickCard"
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
				if self:hasSkills("bazhen|yizhong", enemy) then
					return enemy
				end
			end
		end
		for _, friend in ipairs(targets) do
			local has_equip = false
			for _, equip in sgs.qlist(friend:getEquips()) do
				if equip:isKindOf(equip_type) then
					has_equip = true
					break
				end
			end
			if not has_equip then
				if equip_type == "Armor" then
					if not self:needKongcheng(friend, true) and not self:hasSkills("bazhen|yizhong", friend) then return friend end
				else
					if friend:isWounded() and not (friend:hasSkill("longhun") and friend:getCardCount(true) >= 3) then return friend end
				end
			end
		end
	else
		for _, friend in ipairs(targets) do
			local has_equip = false
			for _, equip in sgs.qlist(friend:getEquips()) do
				if equip:isKindOf(equip_type) then
					has_equip = true
					break
				end
			end
			if not has_equip then
				for _, aplayer in sgs.qlist(self.room:getAllPlayers()) do
					if friend:distanceTo(aplayer) == 1 then
						if self:isFriend(aplayer) and not aplayer:containsTrick("YanxiaoCard")
							and (aplayer:containsTrick("indulgence") or aplayer:containsTrick("supply_shortage")
								or (aplayer:containsTrick("lightning") and self:hasWizard(self.enemies))) then
							aplayer:setFlags("AI_YuanhuToChoose")
							return friend
						end
					end
				end
				self:sort(self.enemies, "defense")
				for _, enemy in ipairs(self.enemies) do
					if friend:distanceTo(enemy) == 1 and self.player:canDiscard(enemy, "he") then
						enemy:setFlags("AI_YuanhuToChoose")
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
	if self.player:hasArmorEffect("SilverLion") then
		local player = yuanhu_validate(self, "SilverLion", false)
		if player then return "@YuanhuCard=" .. self.player:getArmor():getEffectiveId() .. "->" .. player:objectName() end
	end
	if self.player:getOffensiveHorse() then
		local player = yuanhu_validate(self, "OffensiveHorse", false)
		if player then return "@YuanhuCard=" .. self.player:getOffensiveHorse():getEffectiveId() .. "->" .. player:objectName() end
	end
	if self.player:getWeapon() then
		local player = yuanhu_validate(self, "Weapon", false)
		if player then return "@YuanhuCard=" .. self.player:getWeapon():getEffectiveId() .. "->" .. player:objectName() end
	end
	if self.player:getArmor() and self.player:getLostHp() <= 1 and self.player:getHandcardNum() >= 3 then
		local player = yuanhu_validate(self, "Armor", false)
		if player then return "@YuanhuCard=" .. self.player:getArmor():getEffectiveId() .. "->" .. player:objectName() end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("DefensiveHorse") then
			local player = yuanhu_validate(self, "DefensiveHorse", true)
			if player then return "@YuanhuCard=" .. card:getEffectiveId() .. "->" .. player:objectName() end
		end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("OffensiveHorse") then
			local player = yuanhu_validate(self, "OffensiveHorse", true)
			if player then return "@YuanhuCard=" .. card:getEffectiveId() .. "->" .. player:objectName() end
		end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("Weapon") then
			local player = yuanhu_validate(self, "Weapon", true)
			if player then return "@YuanhuCard=" .. card:getEffectiveId() .. "->" .. player:objectName() end
		end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("SilverLion") then
			local player = yuanhu_validate(self, "SilverLion", true)
			if player then return "@YuanhuCard=" .. card:getEffectiveId() .. "->" .. player:objectName() end
		end
		if card:isKindOf("Armor") and yuanhu_validate(self, "Armor", true) then
			local player = yuanhu_validate(self, "Armor", true)
			if player then return "@YuanhuCard=" .. card:getEffectiveId() .. "->" .. player:objectName() end
		end
	end
end

sgs.ai_skill_playerchosen.yuanhu = function(self, targets)
	targets = sgs.QList2Table(targets)
	for _, p in ipairs(targets) do
		if p:hasFlag("AI_YuanhuToChoose") then
			p:setFlags("-AI_YuanhuToChoose")
			return p
		end
	end
	return targets[1]
end

sgs.ai_card_intention.YuanhuCard = function(self, card, from, to)
	if to[1]:hasSkill("bazhen") or to[1]:hasSkill("yizhong") or (to[1]:hasSkill("kongcheng") and to[1]:isKongcheng()) then
		if sgs.Sanguosha:getCard(card:getEffectiveId()):isKindOf("SilverLion") then
			sgs.updateIntention(from, to[1], 10)
			return
		end
	end
	sgs.updateIntention(from, to[1], -50)
end

sgs.ai_cardneed.yuanhu = sgs.ai_cardneed.equip

sgs.yuanhu_keep_value = {
	Peach = 6,
	Jink = 5.1,
	Weapon = 4.7,
	Armor = 4.8,
	Horse = 4.9
}

sgs.ai_cardneed.xueji = function(to, card)
	return to:getHandcardNum() < 3 and card:isRed()
end

local xueji_skill = {}
xueji_skill.name = "xueji"
table.insert(sgs.ai_skills, xueji_skill)
xueji_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("XuejiCard") then return end
	if not self.player:isWounded() then return end
	
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

local function can_be_selected_as_target_xueji(self, card, who)
	-- validation of rule
	if self.player:getWeapon() and self.player:getWeapon():getEffectiveId() == card:getEffectiveId() then
		if self.player:distanceTo(who, sgs.weapon_range[self.player:getWeapon():getClassName()] - self.player:getAttackRange(false)) > self.player:getAttackRange() then return false end
	elseif self.player:getOffensiveHorse() and self.player:getOffensiveHorse():getEffectiveId() == card:getEffectiveId() then
		if self.player:distanceTo(who, 1) > self.player:getAttackRange() then return false end
	elseif self.player:distanceTo(who) > self.player:getAttackRange() then
		return false 
	end
	-- validation of strategy
	if self:isEnemy(who) and self:damageIsEffective(who) and not self:cantbeHurt(who) and not self:getDamagedEffects(who) and not self:needToLoseHp(who) then
		if not self.player:hasSkill("jueqing") then
			if who:hasSkill("guixin") and (self.room:getAliveCount() >= 4 or not who:faceUp()) and not who:hasSkill("manjuan") then return false end
			if (who:hasSkill("ganglie") or who:hasSkill("neoganglie")) and (self.player:getHp() == 1 and self.player:getHandcardNum() <= 2) then return false end
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
			if who:hasSkill("yiji") then
				local huatuo = self.room:findPlayerBySkillName("jijiu")
				if huatuo and self:isEnemy(huatuo) and huatuo:getHandcardNum() >= 3 then
					return false
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
		if who:hasSkill("hunzi") and who:getMark("hunzi") == 0
		  and who:objectName() == self.player:getNextAlive():objectName() and who:getHp() == 2 then
			return true 
		end
		if self:cantbeHurt(who) and not self:damageIsEffective(who) and not (who:hasSkill("manjuan") and who:getPhase() == sgs.Player_NotActive)
		  and not (who:hasSkill("kongcheng") and who:isKongcheng()) then
			return true
		end
		return false
	end
	return false
end

sgs.ai_skill_use_func.XuejiCard = function(card, use, self)
	if self.player:getLostHp() == 0 or self.player:hasUsed("XuejiCard") then return end
	self:sort(self.enemies)
	local to_use = false
	for _, enemy in ipairs(self.enemies) do
		if can_be_selected_as_target_xueji(self, card, enemy) then
			to_use = true
			break
		end
	end
	if not to_use then
		for _, friend in ipairs(self.friends_noself) do
			if can_be_selected_as_target_xueji(self, card, friend) then
				to_use = true
				break
			end
		end
	end
	if to_use then
		use.card = card
		if use.to then
			for _, enemy in ipairs(self.enemies) do
				if can_be_selected_as_target_xueji(self, card, enemy) then
					use.to:append(enemy)
					if use.to:length() == self.player:getLostHp() then return end
				end
			end
			for _, friend in ipairs(self.friends_noself) do
				if can_be_selected_as_target_xueji(self, card, friend) then
					use.to:append(friend)
					if use.to:length() == self.player:getLostHp() then return end
				end
			end
			assert(use.to:length() > 0)
		end
	end
end

sgs.ai_card_intention.XuejiCard = function(self, card, from, tos)
	local room = from:getRoom()
	local huatuo = room:findPlayerBySkillName("jijiu")
	for _,to in ipairs(tos) do
		local intention = 60
		if to:hasSkill("yiji") and not from:hasSkill("jueqing") then
			if (huatuo and self:isFriend(huatuo) and huatuo:getHandcardNum() >= 3 and huatuo:objectName() ~= from:objectName()) then
				intention = -30
			end
			if to:getLostHp() == 0 and to:getMaxHp() >= 3 then
				intention = -10
			end
		end
		if to:hasSkill("hunzi") and to:getMark("hunzi") == 0 then
			if to:objectName() == from:getNextAlive():objectName() and to:getHp() == 2 then 
				intention = -20 
			end
		end
		if self:cantbeHurt(to) and not self:damageIsEffective(to) then intention = -20 end
		sgs.updateIntention(from, to, intention)
	end
end

sgs.ai_use_value.XuejiCard = 3
sgs.ai_use_priority.XuejiCard = 2.35

sgs.ai_skill_use["@@bifa"] = function(self, prompt)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	self:sort(self.enemies, "hp")
	if #self.enemies < 0 then return "." end
	for _, enemy in ipairs(self.enemies) do
		if not (self:needToLoseHp(enemy) and not self:hasSkills(sgs.masochism_skill, enemy)) then
			for _, c in ipairs(cards) do
				if c:isKindOf("EquipCard") then return "@BifaCard=" .. c:getEffectiveId() .. "->" .. enemy:objectName() end
			end
			for _, c in ipairs(cards) do
				if c:isKindOf("TrickCard") and not (c:isKindOf("Nullification") and self:getCardsNum("Nullification") == 1) then 
					return "@BifaCard=" .. c:getEffectiveId() .. "->" .. enemy:objectName() 
				end
			end
			for _, c in ipairs(cards) do
				if c:isKindOf("Slash") then 
					return "@BifaCard=" .. c:getEffectiveId() .. "->" .. enemy:objectName() 
				end
			end
		end
	end
end

sgs.ai_skill_cardask["@bifa-give"] = function(self, data)
	local card_type = data:toString()
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	if self:needToLoseHp() and not self:hasSkills(sgs.masochism_skill) then return "." end
	self:sortByUseValue(cards)
	for _, c in ipairs(cards) do
		if c:isKindOf(card_type) and not isCard("Peach", c, self.player) and not isCard("ExNihilo", c, self.player) then
			return "$" .. c:getEffectiveId()
		end
	end
	return "."
end

sgs.ai_card_intention.BifaCard = 30

sgs.bifa_keep_value = {
	Peach = 6,
	Jink = 5.1,
	Nullification = 5,
	EquipCard = 4.9,
	TrickCard = 4.8
}

local songci_skill = {}
songci_skill.name = "songci"
table.insert(sgs.ai_skills, songci_skill)
songci_skill.getTurnUseCard = function(self)
	return sgs.Card_Parse("@SongciCard=.")
end

sgs.ai_skill_use_func.SongciCard = function(card,use,self)
	self:sort(self.friends, "handcard")
	for _, friend in ipairs(self.friends) do
		if friend:getMark("songci" .. self.player:objectName()) == 0 and friend:getHandcardNum() < friend:getHp() and not (friend:hasSkill("manjuan") and self.room:getCurrent() ~= friend) then
			if not (friend:hasSkill("kongcheng") and friend:isKongcheng()) then
				use.card = sgs.Card_Parse("@SongciCard=.")
				if use.to then use.to:append(friend) end
				return
			end
		end
	end
	
	self:sort(self.enemies, "handcard")
	self.enemies = sgs.reverse(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		if enemy:getMark("songci" .. self.player:objectName()) == 0 and enemy:getHandcardNum() > enemy:getHp() and not enemy:isNude()
			and not self:doNotDiscard(enemy, "nil", false, 2) then
			use.card = sgs.Card_Parse("@SongciCard=.")
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

sgs.ai_use_value.SongciCard = 3
sgs.ai_use_priority.SongciCard = 3

sgs.ai_card_intention.SongciCard = function(self, card, from, to)
	sgs.updateIntention(from, to[1], to[1]:getHandcardNum() > to[1]:getHp() and 80 or -80)
end

sgs.ai_skill_cardask["@xingwu"] = function(self, data)
	local cards = sgs.QList2Table(self.player:getHandcards())
	if #cards <= 1 and self.player:getPile("xingwu"):length() == 1 then return "." end

	local good_enemies = {}
	for _, enemy in ipairs(self.enemies) do
		if enemy:isMale() and ((self:damageIsEffective(enemy) and not self:cantbeHurt(enemy, self.player, 2))
								or (not self:damageIsEffective(enemy) and not enemy:getEquips():isEmpty()
									and not (enemy:getEquips():length() == 1 and enemy:getArmor() and self:needToThrowArmor(enemy)))) then
			table.insert(good_enemies, enemy)
		end
	end
	if #good_enemies == 0 and (not self.player:getPile("xingwu"):isEmpty() or not self.player:hasSkill("luoyan")) then return "." end

	local red_avail, black_avail
	local n = self.player:getMark("xingwu")
	if bit32.band(n, 2) == 0 then red_avail = true end
	if bit32.band(n, 1) == 0 then black_avail = true end

	self:sortByKeepValue(cards)
	local xwcard = nil
	local heart = 0
	local to_save = 0
	for _, card in ipairs(cards) do
		if self.player:hasSkill("tianxiang") and card:getSuit() == sgs.Card_Heart and heart < math.min(self.player:getHp(), 2) then
			heart = heart + 1
		elseif isCard("Jink", card, self.player) then
			if self.player:hasSkill("liuli") and self.room:alivePlayerCount() > 2 then
				for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
					if self:canLiuli(self.player, p) then
						xwcard = card
						break
					end
				end
			end
			if not xwcard and self:getCardsNum("Jink") >= 2 then
				xwcard = card
			end
		elseif to_save > self.player:getMaxCards()
				or (not isCard("Peach", card, self.player) and not (self:isWeak() and isCard("Analeptic", card, self.player))) then
			xwcard = card
		else
			to_save = to_save + 1
		end
		if xwcard then
			if (red_avail and xwcard:isRed()) or (black_avail and xwcard:isBlack()) then
				break
			else
				xwcard = nil
				to_save = to_save + 1
			end
		end
	end
	if xwcard then return "$" .. xwcard:getEffectiveId() else return "." end
end

sgs.ai_skill_playerchosen.xingwu = function(self, targets)
	local good_enemies = {}
	for _, enemy in ipairs(self.enemies) do
		if enemy:isMale() then
			table.insert(good_enemies, enemy)
		end
	end
	if #good_enemies == 0 then return targets:first() end

	local getCmpValue = function(enemy)
		local value = 0
		if self:damageIsEffective(enemy) then
			local dmg = enemy:hasArmorEffect("SilverLion") and 1 or 2
			if enemy:getHp() <= dmg then value = 5 else value = value + enemy:getHp() / (enemy:getHp() - dmg) end
			if not sgs.isGoodTarget(enemy, self.enemies, self) then value = value - 2 end
			if self:cantbeHurt(enemy, self.player, dmg) then value = value - 5 end
			if enemy:isLord() then value = value + 2 end
			if enemy:hasArmorEffect("SilverLion") then value = value - 1.5 end
			if self:hasSkills(sgs.exclusive_skill, enemy) then value = value - 1 end
			if self:hasSkills(sgs.masochism_skill, enemy) then value = value - 0.5 end
		end
		if not enemy:getEquips():isEmpty() then
			local len = enemy:getEquips():length()
			if enemy:hasSkills(sgs.lose_equip_skill) then value = value - 0.6 * len end
			if enemy:getArmor() and self:needToThrowArmor() then value = value - 1.5 end
			if enemy:hasArmorEffect("SilverLion") then value = value - 0.5 end

			if enemy:getWeapon() then value = value + 0.8 end
			if enemy:getArmor() then value = value + 1 end
			if enemy:getDefensiveHorse() then value = value + 0.9 end
			if enemy:getOffensiveHorse() then value = value + 0.7 end
			if self:getDangerousCard(enemy) then value = value + 0.3 end
			if self:getValuableCard(enemy) then value = value + 0.15 end
		end
		return value
	end

	local cmp = function(a, b)
		return getCmpValue(a) > getCmpValue(b)
	end
	table.sort(good_enemies, cmp)
	return good_enemies[1]
end

sgs.ai_playerchosen_intention.xingwu = 80

sgs.ai_skill_cardask["@yanyu-discard"] = function(self, data)
	if self.player:getHandcardNum() < 3 and self.player:getPhase() ~= sgs.Player_Play then
		if self:needToThrowArmor() then return "$" .. self.player:getArmor():getEffectiveId()
		elseif self:needKongcheng(self.player, true) and self.player:getHandcardNum() == 1 then return "$" .. self.player:handCards():first()
		else return "." end
	end
	local current = self.room:getCurrent()
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	if current:objectName() == self.player:objectName() then
		local ex_nihilo, savage_assault, archery_attack
		for _, card in ipairs(cards) do
			if card:isKindOf("ExNihilo") then ex_nihilo = card
			elseif card:isKindOf("SavageAssault") then savage_assault = card
			elseif card:isKindOf("ArcheryAttack") then archery_attack = card
			end
		end
		if savage_assault and self:getAoeValue(savage_assault) <= 0 then savage_assault = nil end
		if archery_attack and self:getAoeValue(archery_attack) <= 0 then archery_attack = nil end
		local aoe = archery_attack or savage_assault
		if ex_nihilo then
			for _, card in ipairs(cards) do
				if card:getTypeId() == sgs.Card_TypeTrick and not card:isKindOf("ExNihilo") and card:getEffectiveId() ~= ex_nihilo:getEffectiveId() then
					return "$" .. card:getEffectiveId()
				end
			end
		end
		if self.player:isWounded() then
			local peach
			for _, card in ipairs(cards) do
				if card:isKindOf("Peach") then
					peach = card
					break
				end
			end
			local dummy_use = { isDummy = true }
			self:useCardPeach(peach, dummy_use)
			if dummy_use.card and dummy_use.card:isKindOf("Peach") then
				for _, card in ipairs(cards) do
					if card:getTypeId() == sgs.Card_TypeBasic and card:getEffectiveId() ~= peach:getEffectiveId() then
						return "$" .. card:getEffectiveId()
					end
				end
			end
		end
		if aoe then
			for _, card in ipairs(cards) do
				if card:getTypeId() == sgs.Card_TypeTrick and card:getEffectiveId() ~= aoe:getEffectiveId() then
					return "$" .. card:getEffectiveId()
				end
			end
		end
		if self:getCardsNum("Slash") > 1 then
			for _, card in ipairs(cards) do
				if card:objectName() == "slash" then
					return "$" .. card:getEffectiveId()
				end
			end
		end
	else
		local throw_trick
		local aoe_type
		if getCardsNum("ArcheryAttack", current) >= 1 then aoe_type = "archery_attack" end
		if getCardsNum("SavageAssault", current) >= 1 then aoe_type = "savage_assault" end
		if aoe_type then
			local aoe = sgs.Sanguosha:cloneCard(aoe_type)
			if self:getAoeValue(aoe, current) > 0 then throw_trick = true end
		end
		if getCardsNum("ExNihilo", current) > 0 then throw_trick = true end
		if throw_trick then
			for _, card in ipairs(cards) do
				if card:getTypeId() == sgs.Card_TypeTrick and not isCard("ExNihilo", card, self.player) then
					return "$" .. card:getEffectiveId()
				end
			end
		end
		if self:getCardsNum("Slash") > 1 then
			for _, card in ipairs(cards) do
				if card:objectName() == "slash" then
					return "$" .. card:getEffectiveId()
				end
			end
		end
		if self:getCardsNum("Jink") > 1 then
			for _, card in ipairs(cards) do
				if card:isKindOf("Jink") then
					return "$" .. card:getEffectiveId()
				end
			end
		end
		if self.player:getHp() >= 3 and (self.player:getHandcardNum() > 3 or self:getCardsNum("Peach") > 0) then
			for _, card in ipairs(cards) do
				if card:isKindOf("Slash") then
					return "$" .. card:getEffectiveId()
				end
			end
		end
		if getCardsNum("TrickCard", current) - getCardsNum("Nullification", current) > 0 then
			for _, card in ipairs(cards) do
				if card:getTypeId() == sgs.Card_TypeTrick and not isCard("ExNihilo", card, self.player) then
					return "$" .. card:getEffectiveId()
				end
			end
		end
	end
	if self:needToThrowArmor() then return "$" .. self.player:getArmor():getEffectiveId() else return "." end
end

sgs.ai_skill_askforag.yanyu = function(self, card_ids)
	local cards = {}
	for _, id in ipairs(card_ids) do
		table.insert(cards, sgs.Sanguosha:getEngineCard(id))
	end
	self.yanyu_need_player = nil
	local card, player = self:getCardNeedPlayer(cards, true)
	if card and player then
		self.yanyu_need_player = player
		return card:getEffectiveId()
	end
	return -1
end

sgs.ai_skill_playerchosen.yanyu = function(self, targets)
	local only_id = self.player:getMark("YanyuOnlyId") - 1
	if only_id < 0 then
		assert(self.yanyu_need_player ~= nil)
		return self.yanyu_need_player
	else
		local card = sgs.Sanguosha:getEngineCard(only_id)
		if card:getTypeId() == sgs.Card_TypeTrick and not card:isKindOf("Nullification") then
			return self.player
		end
		local cards = { card }
		local c, player = self:getCardNeedPlayer(cards, true)
		return player
	end
end

sgs.ai_playerchosen_intention.yanyu = function(self, from, to)
	if hasManjuanEffect(to) then return end
	local intention = -60
	if self:needKongcheng(to, true) then intention = 10 end
	sgs.updateIntention(from, to, intention)
end

sgs.ai_skill_invoke.xiaode = function(self, data)
	local round = self:playerGetRound(self.player)
	local xiaode_skill = sgs.ai_skill_choice.huashen(self, table.concat(data:toStringList(), "+"), nil, math.random(1 - round, 7 - round))
	if xiaode_skill then
		sgs.xiaode_choice = xiaode_skill
		return true
	else
		sgs.xiaode_choice = nil
		return false
	end
end

sgs.ai_skill_choice.xiaode = function(self, choices)
	return sgs.xiaode_choice
end

function sgs.ai_cardsview_valuable.aocai(self, class_name, player)
	if player:hasFlag("Global_AocaiFailed") or player:getPhase() ~= sgs.Player_NotActive then return end
	if class_name == "Slash" and sgs.Sanguosha:getCurrentCardUseReason() == sgs.CardUseStruct_CARD_USE_REASON_RESPONSE_USE then
		return "@AocaiCard=.:slash"
	elseif (class_name == "Peach" and not player:hasFlag("Global_PreventPeach")) or class_name == "Analeptic" then
		local dying = self.room:getCurrentDyingPlayer()
		if dying and dying:objectName() == player:objectName() then
			local user_string = "peach+analeptic"
			if player:hasFlag("Global_PreventPeach") then user_string = "analeptic" end
			return "@AocaiCard=.:" .. user_string
		else
			local user_string
			if class_name == "Analeptic" then user_string = "analeptic" else user_string = "peach" end
			return "@AocaiCard=.:" .. user_string
		end
	end
end

sgs.ai_skill_invoke.aocai = function(self, data)
	local asked = data:toStringList()
	local pattern = asked[1]
	local prompt = asked[2]
	return self:askForCard(pattern, prompt, 1) ~= "."
end

sgs.ai_skill_askforag.aocai = function(self, card_ids)
	local card = sgs.Sanguosha:getCard(card_ids[1])
	if card:isKindOf("Jink") and self.player:hasFlag("dahe") then
		for _, id in ipairs(card_ids) do
			if sgs.Sanguosha:getCard(id):getSuit() == sgs.Card_Heart then return id end
		end
		return -1
	end
	return card_ids[1]
end

function SmartAI:getSaveNum(isFriend)
	local num = 0
	for _, player in sgs.qlist(self.room:getAllPlayers()) do
		if (isFriend and self:isFriend(player)) or (not isFriend and self:isEnemy(player)) then
			if not self.player:hasSkill("wansha") or player:objectName() == self.player:objectName() then
				if player:hasSkill("jijiu") then
					num = num + self:getSuitNum("heart", true, player)
					num = num + self:getSuitNum("diamond", true, player)
					num = num + player:getHandcardNum() * 0.4
				end
				if player:hasSkill("nosjiefan") and getCardsNum("Slash", player) > 0 then
					if self:isFriend(player) or self:getCardsNum("Jink") == 0 then num = num + getCardsNum("Slash", player) end
				end
				num = num + getCardsNum("Peach", player)
			end
			if player:hasSkill("buyi") and not player:isKongcheng() then num = num + 0.3 end
			if player:hasSkill("chunlao") and not player:getPile("wine"):isEmpty() then num = num + player:getPile("wine"):length() end
			if player:hasSkill("jiuzhu") and player:getHp() > 1 and not player:isNude() then
				num = num + 0.9 * math.max(0, math.min(player:getHp() - 1, player:getCardCount(true)))
			end
			if player:hasSkill("renxin") and player:objectName() ~= self.player:objectName() and not player:isKongcheng() then num = num + 1 end
		end
	end
	return num
end

local duwu_skill = {}
duwu_skill.name = "duwu"
table.insert(sgs.ai_skills, duwu_skill)
duwu_skill.getTurnUseCard = function(self, inclusive)
	if self.player:hasFlag("DuwuEnterDying") or #self.enemies == 0 then return end
	return sgs.Card_Parse("@DuwuCard=.")
end

sgs.ai_skill_use_func.DuwuCard = function(card, use, self)
	local cmp = function(a, b)
		if a:getHp() < b:getHp() then
			if a:getHp() == 1 and b:getHp() == 2 then return false else return true end
		end
		return false
	end
	local enemies = {}
	for _, enemy in ipairs(self.enemies) do
		if self:canAttack(enemy, self.player) and self.player:inMyAttackRange(enemy) then table.insert(enemies, enemy) end
	end
	if #enemies == 0 then return end
	table.sort(enemies, cmp)
	if enemies[1]:getHp() <= 0 then
		use.card = sgs.Card_Parse("@DuwuCard=.")
		if use.to then use.to:append(enemies[1]) end
		return
	end

	-- find cards
	local card_ids = {}
	if self:needToThrowArmor() then table.insert(card_ids, self.player:getArmor():getEffectiveId()) end

	local zcards = self.player:getHandcards()
	local use_slash, keep_jink, keep_analeptic = false, false, false
	for _, zcard in sgs.qlist(zcards) do
		if not isCard("Peach", zcard, self.player) and not isCard("ExNihilo", zcard, self.player) then
			local shouldUse = true
			if zcard:getTypeId() == sgs.Card_TypeTrick then
				local dummy_use = { isDummy = true }
				self:useTrickCard(zcard, dummy_use)
				if dummy_use.card then shouldUse = false end
			end
			if zcard:getTypeId() == sgs.Card_TypeEquip and not self.player:hasEquip(zcard) then
				local dummy_use = { isDummy = true }
				self:useEquipCard(zcard, dummy_use)
				if dummy_use.card then shouldUse = false end
			end
			if isCard("Jink", zcard, self.player) and not keep_jink then
				keep_jink = true
				shouldUse = false
			end
			if self.player:getHp() == 1 and isCard("Analeptic", zcard, self.player) and not keep_analeptic then
				keep_analeptic = true
				shouldUse = false
			end
			if shouldUse then table.insert(card_ids, zcard:getId()) end
		end
	end
	local hc_num = #card_ids
	local eq_num = 0
	if self.player:getOffensiveHorse() then
		table.insert(card_ids, self.player:getOffensiveHorse():getEffectiveId())
		eq_num = eq_num + 1
	end
	if self.player:getWeapon() and self:evaluateWeapon(self.player:getWeapon()) < 5 then
		table.insert(card_ids, self.player:getWeapon():getEffectiveId())
		eq_num = eq_num + 2
	end

	local function getRangefix(index)
		if index <= hc_num then return 0
		elseif index == hc_num + 1 then
			if eq_num == 2 then
				return sgs.weapon_range[self.player:getWeapon():getClassName()] - self.player:getAttackRange(false)
			else
				return 1
			end
		elseif index == hc_num + 2 then
			return sgs.weapon_range[self.player:getWeapon():getClassName()]
		end
	end

	for _, enemy in ipairs(enemies) do
		if enemy:getHp() > #card_ids then continue end
		if enemy:getHp() <= 0 then
			use.card = sgs.Card_Parse("@DuwuCard=.")
			if use.to then use.to:append(enemy) end
			return
		elseif enemy:getHp() > 1 then
			local hp_ids = {}
			if self.player:distanceTo(enemy, getRangefix(enemy:getHp())) <= self.player:getAttackRange() then
				for _, id in ipairs(card_ids) do
					table.insert(hp_ids, id)
					if #hp_ids == enemy:getHp() then break end
				end
				use.card = sgs.Card_Parse("@DuwuCard=" .. table.concat(hp_ids, "+"))
				if use.to then use.to:append(enemy) end
				return
			end
		else
			if not self:isWeak() or self:getSaveNum(true) >= 1 then
				if self.player:distanceTo(enemy, getRangefix(1)) <= self.player:getAttackRange() then
					use.card = sgs.Card_Parse("@DuwuCard=" .. card_ids[1])
					if use.to then use.to:append(enemy) end
					return
				end
			end
		end
	end
end

sgs.ai_use_priority.DuwuCard = 0.6
sgs.ai_use_value.DuwuCard = 2.45
sgs.dynamic_value.damage_card.DuwuCard = true
sgs.ai_card_intention.DuwuCard = 80

function getNextJudgeReason(self, player)
	if self:playerGetRound(player) > 2 then
		if player:hasSkills("ganglie|vsganglie") then return end
		local caiwenji = self.room:findPlayerBySkillName("beige")
		if caiwenji and caiwenji:canDiscard(caiwenji, "he") and self:isFriend(caiwenji, player) then return end
		if player:hasArmorEffect("EightDiagram") or player:hasSkill("bazhen") then
			if self:playerGetRound(player) > 3 and self:isEnemy(player) then return "EightDiagram"
			else return end
		end
	end
	if self:isFriend(player) and player:hasSkill("luoshen") then return "luoshen" end
	if not player:getJudgingArea():isEmpty() and not player:containsTrick("YanxiaoCard") then
		return player:getJudgingArea():last():objectName()
	end
	if player:hasSkill("qianxi") then return "qianxi" end
end

local zhoufu_skill = {}
zhoufu_skill.name = "zhoufu"
table.insert(sgs.ai_skills, zhoufu_skill)
zhoufu_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("ZhoufuCard") or self.player:isKongcheng() or self:getOverflow() <= 0 then return end
	return sgs.Card_Parse("@ZhoufuCard=.")
end

sgs.ai_skill_use_func.ZhoufuCard = function(card, use, self)
	local cards = {}
	for _, card in sgs.qlist(self.player:getHandcards()) do
		table.insert(cards, sgs.Sanguosha:getEngineCard(card:getEffectiveId()))
	end
	self:sortByKeepValue(cards)
	self:sort(self.friends_noself)
	local zhenji
	for _, friend in ipairs(self.friends_noself) do
		local reason = getNextJudgeReason(self, friend)
		if reason then
			if reason == "luoshen" then
				zhenji = friend
			elseif reason == "indulgence" then
				for _, card in ipairs(cards) do
					if card:getSuit() == sgs.Card_Heart or (friend:hasSkill("hongyan") and card:getSuit() == sgs.Card_Spade)
						and (friend:hasSkill("tiandu") or not self:isValuableCard(card)) then
						use.card = sgs.Card_Parse("@ZhoufuCard=" .. card:getEffectiveId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			elseif reason == "supply_shortage" then
				for _, card in ipairs(cards) do
					if card:getSuit() == sgs.Card_Club and (friend:hasSkill("tiandu") or not self:isValuableCard(card)) then
						use.card = sgs.Card_Parse("@ZhoufuCard=" .. card:getEffectiveId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			elseif reason == "lightning" and not friend:hasSkills("hongyan|wuyan") then
				for _, card in ipairs(cards) do
					if (card:getSuit() ~= sgs.Card_Spade or card:getNumber() == 1 or card:getNumber() > 9)
						and (friend:hasSkill("tiandu") or not self:isValuableCard(card)) then
						use.card = sgs.Card_Parse("@ZhoufuCard=" .. card:getEffectiveId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			end
		end
	end
	if zhenji then
		for _, card in ipairs(cards) do
			if card:isBlack() and not (zhenji:hasSkill("hongyan") and card:getSuit() == sgs.Card_Spade) then
				use.card = sgs.Card_Parse("@ZhoufuCard=" .. card:getEffectiveId())
				if use.to then use.to:append(zhenji) end
				return
			end
		end
	end
	self:sort(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		local reason = getNextJudgeReason(self, enemy)
		if not enemy:hasSkill("tiandu") and reason then
			if reason == "indulgence" then
				for _, card in ipairs(cards) do
					if not (card:getSuit() == sgs.Card_Heart or (enemy:hasSkill("hongyan") and card:getSuit() == sgs.Card_Spade))
						and not self:isValuableCard(card) then
						use.card = sgs.Card_Parse("@ZhoufuCard=" .. card:getEffectiveId())
						if use.to then use.to:append(enemy) end
						return
					end
				end
			elseif reason == "supply_shortage" then
				for _, card in ipairs(cards) do
					if not card:getSuit() == sgs.Card_Club and not self:isValuableCard(card) then
						use.card = sgs.Card_Parse("@ZhoufuCard=" .. card:getEffectiveId())
						if use.to then use.to:append(enemy) end
						return
					end
				end
			elseif reason == "lightning" and not enemy:hasSkills("hongyan|wuyan") then
				for _, card in ipairs(cards) do
					if card:getSuit() == sgs.Card_Spade and card:getNumber() >= 2 and card:getNumber() <= 9 then
						use.card = sgs.Card_Parse("@ZhoufuCard=" .. card:getEffectiveId())
						if use.to then use.to:append(enemy) end
						return
					end
				end
			end
		end
	end
end

sgs.ai_card_intention.ZhoufuCard = 0
sgs.ai_use_value.ZhoufuCard = 2
sgs.ai_use_priority.ZhoufuCard = 1.0

local function getKangkaiCard(self, target, data)
	local use = data:toCardUse()
	local weapon, armor, def_horse, off_horse = {}, {}, {}, {}
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if card:isKindOf("Weapon") then table.insert(weapon, card)
		elseif card:isKindOf("Armor") then table.insert(armor, card)
		elseif card:isKindOf("DefensiveHorse") then table.insert(def_horse, card)
		elseif card:isKindOf("OffensiveHorse") then table.insert(off_horse, card)
		end
	end
	if #armor > 0 then
		for _, card in ipairs(armor) do
			if ((not target:getArmor() and not target:hasSkills("bazhen|yizhong"))
				or (target:getArmor() and self:evaluateArmor(card, target) >= self:evaluateArmor(target:getArmor(), target)))
				and not (card:isKindOf("Vine") and use.card:isKindOf("FireSlash") and self:slashIsEffective(use.card, target, use.from)) then
				return card:getEffectiveId()
			end
		end
	end
	if self:needToThrowArmor()
		and ((not target:getArmor() and not target:hasSkills("bazhen|yizhong"))
			or (target:getArmor() and self:evaluateArmor(self.player:getArmor(), target) >= self:evaluateArmor(target:getArmor(), target)))
		and not (self.player:getArmor():isKindOf("Vine") and use.card:isKindOf("FireSlash") and self:slashIsEffective(use.card, target, use.from)) then
		return self.player:getArmor():getEffectiveId()
	end
	if #def_horse > 0 then return def_horse[1]:getEffectiveId() end
	if #weapon > 0 then
		for _, card in ipairs(weapon) do
			if not target:getWeapon()
				or (self:evaluateArmor(card, target) >= self:evaluateArmor(target:getWeapon(), target)) then
				return card:getEffectiveId()
			end
		end
	end
	if self.player:getWeapon() and self:evaluateWeapon(self.player:getWeapon()) < 5
		and (not target:getArmor()
			or (self:evaluateArmor(self.player:getWeapon(), target) >= self:evaluateArmor(target:getWeapon(), target))) then
		return self.player:getWeapon():getEffectiveId()
	end
	if #off_horse > 0 then return off_horse[1]:getEffectiveId() end
	if self.player:getOffensiveHorse()
		and ((self.player:getWeapon() and not self.player:getWeapon():isKindOf("Crossbow")) or self.player:hasSkills("mashu|tuntian")) then
		return self.player:getOffensiveHorse():getEffectiveId()
	end
end

sgs.ai_skill_invoke.kangkai = function(self, data)
	self.kangkai_give_id = nil
	if hasManjuanEffect(self.player) then return false end
	local target = data:toPlayer()
	if not target then return false end
	if target:objectName() == self.player:objectName() then
		return true
	elseif not self:isFriend(target) then
		return hasManjuanEffect(target)
	else
		local id = getKangkaiCard(self, target, self.player:getTag("KangkaiSlash"))
		if id then return true else return not self:needKongcheng(target, true) end
	end
end

sgs.ai_skill_cardask["@kangkai_give"] = function(self, data, pattern, target)
	if self:isFriend(target) then
		local id = getKangkaiCard(self, target, data)
		if id then return "$" .. id end
		if self:getCardsNum("Jink") > 1 then
			for _, card in sgs.qlist(self.player:getHandcards()) do
				if isCard("Jink", card, target) then return "$" .. card:getEffectiveId() end
			end
		end
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if not self:isValuableCard(card) then return "$" .. card:getEffectiveId() end
		end
	else
		local to_discard = self:askForDiscard("dummyreason", 1, 1, false, true)
		if #to_discard > 0 then return "$" .. to_discard[1] end
	end
end

sgs.ai_skill_invoke.kangkai_use = function(self, data)
	local use = self.player:getTag("KangkaiSlash"):toCardUse()
	local card = self.player:getTag("KangkaiCard"):toCard()
	if not use.card or not card then return false end
	if card:isKindOf("Vine") and use.card:isKindOf("FireSlash") and self:slashIsEffective(use.card, self.player, use.from) then return false end
	if ((card:isKindOf("DefensiveHorse") and self.player:getDefensiveHorse())
		or (card:isKindOf("OffensiveHorse") and (self.player:getOffensiveHorse() or (self.player:hasSkill("drmashu") and self.player:getDefensiveHorse()))))
		and not self.player:hasSkills(sgs.lose_equip_skill) then
		return false
	end
	if card:isKindOf("Armor")
		and ((self.player:hasSkills("bazhen|yizhong") and not self.player:getArmor())
			or (self.player:getArmor() and self:evaluateArmor(card) < self:evaluateArmor(self.player:getArmor()))) then return false end
	if card:isKindOf("Weanpon") and (self.player:getWeapon() and self:evaluateArmor(card) < self:evaluateArmor(self.player:getWeapon())) then return false end
	return true
end

sgs.ai_skill_use["@@qingyi"] = function(self, prompt)
	local card_str = sgs.ai_skill_use["@@shensu1"](self, "@shensu1")
	return string.gsub(card_str, "ShensuCard", "QingyiCard")
end

sgs.ai_card_intention.QingyiCard = sgs.ai_card_intention.Slash

sgs.ai_skill_invoke.cv_sunshangxiang = function(self, data)
	local lord = self.room:getLord()
	if lord and lord:hasLordSkill("shichou") then
		return self:isFriend(lord)
	end
	return lord:getKingdom() == "shu"
end

sgs.ai_chaofeng.sp_sunshangxiang = sgs.ai_chaofeng.sunshangxiang

sgs.ai_skill_invoke.cv_caiwenji = function(self, data)
	local lord = self.room:getLord()
	if lord and lord:hasLordSkill("xueyi") then
		return not self:isFriend(lord)
	end
	return lord:getKingdom() == "wei"
end

sgs.ai_chaofeng.sp_caiwenji = sgs.ai_chaofeng.caiwenji

sgs.ai_skill_invoke.cv_machao = function(self, data)
	local lord = self.room:getLord()
	if lord and lord:hasLordSkill("xueyi") and self:isFriend(lord) then
		sgs.ai_skill_choice.cv_machao = "sp_machao"
		return true
	end
	if lord and lord:hasLordSkill("shichou") and not self:isFriend(lord) then
		sgs.ai_skill_choice.cv_machao = "sp_machao"
		return true
	end
	if lord and lord:getKingdom() == "qun" and not lord:hasLordSkill("xueyi") then
		sgs.ai_skill_choice.cv_machao = "sp_machao"
		return true
	end
	if math.random(0, 2) == 0 then
		sgs.ai_skill_choice.cv_machao = "tw_machao"
		return true
	end
end

sgs.ai_chaofeng.sp_machao = sgs.ai_chaofeng.machao

sgs.ai_skill_invoke.cv_diaochan = function(self, data)
	if math.random(0, 2) == 0 then return false
	elseif math.random(0, 3) == 0 then sgs.ai_skill_choice.cv_diaochan = "tw_diaochan" return true
	elseif math.random(0, 3) == 0 then sgs.ai_skill_choice.cv_diaochan = "heg_diaochan" return true
	else sgs.ai_skill_choice.cv_diaochan = "sp_diaochan" return true end
end

sgs.ai_chaofeng.sp_diaochan = sgs.ai_chaofeng.diaochan

sgs.ai_skill_invoke.cv_pangde = sgs.ai_skill_invoke.cv_caiwenji
sgs.ai_skill_invoke.cv_jiaxu = sgs.ai_skill_invoke.cv_caiwenji

sgs.ai_skill_invoke.cv_yuanshu = function(self, data)
	return math.random(0, 2) == 0
end

sgs.ai_skill_invoke.cv_zhaoyun = sgs.ai_skill_invoke.cv_yuanshu
sgs.ai_skill_invoke.cv_ganning = sgs.ai_skill_invoke.cv_yuanshu
sgs.ai_skill_invoke.cv_shenlvbu = sgs.ai_skill_invoke.cv_yuanshu

sgs.ai_skill_invoke.cv_daqiao = function(self, data)
	if math.random(0, 3) >= 1 then return false
	elseif math.random(0, 4) == 0 then sgs.ai_skill_choice.cv_daqiao = "tw_daqiao" return true
	else sgs.ai_skill_choice.cv_daqiao = "wz_daqiao" return true end
end

sgs.ai_skill_invoke.cv_xiaoqiao = function(self, data)
	if math.random(0, 3) >= 1 then return false
	elseif math.random(0, 4) == 0 then sgs.ai_skill_choice.cv_xiaoqiao = "wz_xiaoqiao" return true
	else sgs.ai_skill_choice.cv_xiaoqiao = "heg_xiaoqiao" return true end
end

sgs.ai_skill_invoke.cv_zhouyu = function(self, data)
	if math.random(0, 3) >= 1 then return false
	elseif math.random(0, 4) == 0 then sgs.ai_skill_choice.cv_zhouyu = "heg_zhouyu" return true
	else sgs.ai_skill_choice.cv_zhouyu = "sp_heg_zhouyu" return true end
end

sgs.ai_skill_invoke.cv_zhenji = function(self, data)
	if math.random(0, 3) >= 2 then return false
	elseif math.random(0, 4) == 0 then sgs.ai_skill_choice.cv_zhenji = "sp_zhenji" return true
	elseif math.random(0, 4) == 0 then sgs.ai_skill_choice.cv_zhenji = "tw_zhenji" return true
	else sgs.ai_skill_choice.cv_zhenji = "heg_zhenji" return true end
end

sgs.ai_skill_invoke.cv_lvbu = function(self, data)
	if math.random(0, 3) >= 1 then return false
	elseif math.random(0, 4) == 0 then sgs.ai_skill_choice.cv_lvbu = "tw_lvbu" return true
	else sgs.ai_skill_choice.cv_lvbu = "heg_lvbu" return true end
end

sgs.ai_skill_invoke.cv_zhangliao = sgs.ai_skill_invoke.cv_yuanshu
sgs.ai_skill_invoke.cv_luxun = sgs.ai_skill_invoke.cv_yuanshu

sgs.ai_skill_invoke.cv_huanggai = function(self, data)
	return math.random(0, 4) == 0
end

sgs.ai_skill_invoke.cv_guojia = sgs.ai_skill_invoke.cv_huanggai
sgs.ai_skill_invoke.cv_zhugeke = sgs.ai_skill_invoke.cv_huanggai
sgs.ai_skill_invoke.cv_yuejin = sgs.ai_skill_invoke.cv_huanggai

sgs.ai_skill_invoke.cv_zhugejin = function(self, data)
	return math.random(0, 4) > 1
end