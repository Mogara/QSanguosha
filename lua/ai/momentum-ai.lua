function sgs.ai_skill_invoke.wangxi(self, data)
	local target = data:toPlayer()
	if target and self:isFriend(target) then
		return not self:needKongcheng(target, true)
	else
		return self:needKongcheng(target, true)
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
		elseif player:getState() == "robot" then sgs.updateIntention(player, target, -60)
		end
	end
end

function sgs.ai_skill_invoke.hengjiang(self, data)
	local target = data:toPlayer()
	if self:isEnemy(target) then
		return true
	else
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
	return sgs.Card_Parse("@CunsiCard=.&cunsi")
end

sgs.ai_skill_use_func.CunsiCard = function(card, use, self)
	if sgs.turncount <= 2 and self.player:aliveCount() > 2 and #self.friends_noself == 0 then return end
	local to, manjuan
	for _, friend in ipairs(self.friends_noself) do
		to = friend
		break
	end
	if not to then to = self.player end
	if self.player:getMark("guixiu") >= 1 then
		use.card = sgs.Card_Parse("@GuixiuCard=.&guixiu")
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

sgs.ai_skill_choice.yingyang = function(self, choices, data)
	local pindian = data:toPindian()
	local reason = pindian.reason
	local from, to = pindian.from, pindian.to
	local f_num, t_num = pindian.from_number, pindian.to_number
	local amFrom = self.player:objectName() == from:objectName()

	if math.abs(f_num - t_num) > 3 then return "cancel" end

	local table_pindian_friends = { "tianyi", "shuangren", "qiaoshui" }
	if reason == "mizhao" then
		if amFrom then
			if self:isFriend(to) then
				if self:getCardsNum("Jink") > 0 then return "down"
				elseif getCardsNum("Jink", to, self.player) >= 1 then return "up"
				else return self.player:getHp() >= to:getHp() and "down" or "up"
				end
			else
				return "up"
			end
		else
			if self:isFriend(from) then
				if self:getCardsNum("Jink") > 0 then return "down"
				elseif getCardsNum("Jink", from, self.player) >= 1 then return "up"
				else return self.player:getHp() >= to:getHp() and "down" or "up"
				end
			else
				return "up"
			end
		end
	elseif reason == "quhu" then
		if amFrom and self.player:hasSkill("jieming") then
			if f_num > 8 then return "up"
			elseif self:getJiemingChaofeng(player) <= -6 then return "down"
			end
		end
		return "up"
	elseif reason == "xiechan" then
		return (not amFrom and self:getCardsNum("Slash") > getCardsNum("Slash", from, self.player)) and "down" or "up"
	elseif reason == "zhiba_pindian" or reason == "nosquanji" then
		return (amFrom and self:isFriend(to)) and "down" or "up"
	elseif table.contains(table_pindian_friends, reason) then
		return (not amFrom and self:isFriend(from)) and "down" or "up"
	else
		return "up"
	end
end

local duanxie_skill = {}
duanxie_skill.name = "duanxie"
table.insert(sgs.ai_skills, duanxie_skill)
duanxie_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("DuanxieCard") then return end
	return sgs.Card_Parse("@DuanxieCard=.&duanxie")
end

sgs.ai_skill_use_func.DuanxieCard = function(card, use, self)
	self:sort(self.enemies, "defense")
	local target
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isChained() and not self:getDamagedEffects(enemy) and not self:needToLoseHp(enemy) and sgs.isGoodTarget(enemy, self.enemies, self) then
			target = enemy
			break
		end
	end
	if not target then return end
	if not self:isWeak() or self.player:isChained() then
		use.card = card
		if use.to then use.to:append(target) end
	end
end

sgs.ai_card_intention.DuanxieCard = 60
sgs.ai_use_priority.DuanxieCard = 0

sgs.ai_skill_invoke.fenming = function(self)
	local value, count = 0, 0
	for _, player in sgs.qlist(self.room:getAllPlayers()) do
		if player:isChained() then
			count = count + 1
			if self:isFriend(player) then
				if self:needToThrowArmor(player) then
					value = value + 1
				elseif player:getHandcardNum() == 1 and self:needKongcheng(player) then
					value = value + 0.2
				elseif self.player:canDiscard(player, "he") then
					local dec = self:isWeak(player) and 1.2 or 0.8
					if player:objectName() == self.player:objectName() then dec = dec / 1.5 end
					if self:getOverflow(player) >= 0 then dec = dec / 1.5 end
					value = value - dec
				end
			elseif self:isEnemy(player) then
				if self.player:canDiscard(player, "he") then
					if self:doNotDiscard(player) then
						value = value - 0.8
					else
						local dec = self:isWeak(player) and 1.2 or 0.8
						if self:getValuableCard(player) or self:getDangerousCard(player) then dec = dec * 1.5 end
						value = value + dec
					end
				end
			end
		end
	end
	--self.room:writeToConsole(value / count)
	return value / count >= 0.2
end

sgs.ai_skill_invoke.hengzheng = function(self, data)
	local value = 0
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		value = value + self:getGuixinValue(player)
	end
	return value >= 1.3
end

sgs.ai_skill_invoke.chuanxin = function(self, data)
	local damage = data:toDamage()
	if not damage.to then return end
	local invoke
	local to = damage.to
	if to:getMark("chuanxin_" .. self.player:objectName()) == 0 and to:getVisibleSkillList():length() > 1 then
		local count = 0
		for _, skill in sgs.qlist(to:getVisibleSkillList()) do
			if not skill:isAttachedLordSkill() then count = count + 1 end
		end
		if count > 1 then
			for _, skill in ipairs(("benghuai|shiyong|yaowu|wumou|chanyuan|jinjiu|tongji"):split("|")) do
				if to:hasSkill(skill) then
					if self:isFriend(to) then return true
					elseif (skill == "benghuai" or skill == "shiyong") and to:getMaxHp() <= 3 then return true
					end
					return false
				end
			end
			invoke = true
		end
	end
	if to:getEquips():length() > 0 then
		if self:isFriend(to) then
			return self:needToLoseHp(to) and to:getEquips():length() == 1
					and (self:needToThrowArmor(to) or (to:hasSkills(sgs.lose_equip_skill)
														and (to:getOffensiveHorse() or (to:getWeapon() and self:evaluateWeapon(to:getWeapon(), to) < 4))))
		elseif to:getHp() >= 2 and to:hasSkills(sgs.lose_equip_skill) and (not to:getArmor() or self:needToThrowArmor(to)) then
			return false
		end
		invoke = true
	end
	return (invoke and self:isEnemy(to) and not self:hasHeavySlashDamage(self.player, damage.card, to))
			or (not invoke and self:isFriend(to))
end

sgs.ai_skill_choice.chuanxin = function(self, choices, data)
	if self.player:hasSkills("benghuai|shiyong}yaowu|wumou|chanyuan|jinjiu|tongji") then return "detach" end
	if self:needToLoseHp(self.player) or self:needToThrowArmor() or self.player:getEquips():length() <= 2 or self.player:hasSkills(sgs.lose_equip_skill) then
		return "throw"
	end
	return (not self:isWeak()) and "throw" or "detach"
end

sgs.ai_skill_choice.chuanxin_lose = function(self, choices, data)
	for _, skill in ipairs(("benghuai|shiyong|yaowu|wumou|chanyuan|jinjiu|tongji|huwei|pianyi|xiaoxi"):split("|")) do
		if self.player:hasSkill(skill) then return skill end
	end
	for _, skill in sgs.qlist(self.player:getVisibleSkillList()) do
		if (skill:getFrequency() == sgs.Skill_Wake and (self.player:getMark(skill:objectName()) > 0 or skill:objectName() == "baoling"))
			or (skill:getFrequency() == sgs.Skill_Limited and self.player:getMark(skill:getLimitMark()) == 0) then
			return skill:objectName()
		end
	end
	if self.player:hasSkill("cuorui") and self.player:getMark("CuoruiSkipJudge") > 0 then return "cuorui" end
	if self.player:hasSkills("paoxiao+huxiao") then return "huxiao" end
	if self.player:hasSkills("fankui+duodao") then return "duodao" end
	if self.player:hasSkills("jilve+wansha") then return "wansha" end
	if self.player:hasSkills("wuqian+wushuang") then return "wushuang" end
	if self.player:hasSkills("tianfu+kanpo") then return "kanpo" end
	if self.player:hasSkills("fuhun|nosfuhun") then
		if choices:matchOne("paoxiao") then return "paoxiao" end
		if choices:matchOne("wusheng") then return "wusheng" end
	end
	if self.player:hasSkill("luoyan") and self.player:getPile("xingwu"):length() > 0 then
		if choices:matchOne("tianxiang") then return "tianxiang" end
		if choices:matchOne("liuli") then return "liuli" end
	end
	if self.player:hasSkill("baobian") then
		for _, skill in ipairs(("paoxiao|shensu|tiaoxin"):split("|")) do
			if self.player:hasSkill(skill) then return skill end
		end
	end
	if self.player:hasSkill("mouduan") and (self.player:getMark("@wu") > 0 or self.player:getMark("@wen") > 0) then
		for _, skill in ipairs(("jiang|qianxun|yingzi|keji"):split("|")) do
			if self.player:hasSkill(skill) then return skill end
		end
	end
	if self.player:hasSkill("huashen") then
		local huashen_skill = self.player:getTag("HuashenSkill"):toString()
		if #huashen_skill > 0 and self.player:hasSkill(huashen_skill) then return huashen_skill end
	end
	if self.player:hasSkill("xiaode") then
		local xiaode_skill = self.player:getTag("XiaodeSkill"):toString()
		if #xiaode_skill > 0 and self.player:hasSkill(xiaode_skill) then return xiaode_skill end
	end
	for _, skill in sgs.qlist(self.player:getVisibleSkillList()) do
		if skill:isLordSkill() then return skill:objectName() end
	end
	for _, skill in ipairs(("guixiu|suishi|weidi|xinsheng|huoshou|lianpo|hongyan|mashu|jueqing|yicong|tannang|feiying"):split("|")) do
		if self.player:hasSkill(skill) then return skill end
	end
 end
 
 sgs.ai_skill_invoke.fengshi = function(self, data)
	local target = data:toPlayer()
	if not target then return false end
	if self:needToThrowArmor(target) then return self:isFriend(target) end
	if target:hasSkills(sgs.lose_equip_skill) then
		return self:isFriend(target) and (target:getOffensiveHorse() or (target:getWeapon() and self:evaluateWeapon(target:getWeapon(), target) < 4))
	end
	return self:isEnemy(target)
end

sgs.ai_choicemade_filter.skillInvoke.fengshi = function(self, player, promptlist)
	if promptlist[3] == "yes" then
		local fengshi_target
		for _, p in sgs.qlist(self.room:getAllPlayers()) do
			if p:hasFlag("FengshiTarget") then
				fengshi_target = p
				break
			end
		end
		if fengshi_target and not fengshi_target:hasSkills(sgs.lose_equip_skill) and not self:needToThrowArmor(fengshi_target) then
			sgs.updateIntention(player, fengshi_target, 60)
		end
	end
end

sgs.weapon_range.SixSwords = 2
sgs.weapon_range.DragonPhoenix = 2
sgs.weapon_range.Triblade = 3