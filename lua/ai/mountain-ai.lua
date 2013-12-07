local function card_for_qiaobian(self, who, return_prompt)
	local card, target
	if self:isFriend(who) then
		local judges = who:getJudgingArea()
		if not judges:isEmpty() then
			for _, judge in sgs.qlist(judges) do
				card = sgs.Sanguosha:getCard(judge:getEffectiveId())
				if not judge:isKindOf("YanxiaoCard") then
					for _, enemy in ipairs(self.enemies) do
						if not enemy:containsTrick(judge:objectName()) and not enemy:containsTrick("YanxiaoCard")
							and not self.room:isProhibited(self.player, enemy, judge) then
							target = enemy
							break
						end
					end
					if target then break end
				end
			end
		end

		local equips = who:getCards("e")
		local weak
		if not target and not equips:isEmpty() and self:hasSkills(sgs.lose_equip_skill, who) then
			for _, equip in sgs.qlist(equips) do
				if equip:isKindOf("OffensiveHorse") then card = equip break
				elseif equip:isKindOf("Weapon") then card = equip break
				elseif equip:isKindOf("DefensiveHorse") and not self:isWeak(who) then
					card = equip
					break
				elseif equip:isKindOf("Armor") and (not self:isWeak(who) or self:needToThrowArmor(who)) then
					card = equip
					break
				end
			end

			if card then
				if card:isKindOf("Armor") or card:isKindOf("DefensiveHorse") then 
					self:sort(self.friends, "defense")
				else
					self:sort(self.friends, "handcard")
					self.friends = sgs.reverse(self.friends)
				end

				for _, friend in ipairs(self.friends) do
					if not self:getSameEquip(card, friend) and friend:objectName() ~= who:objectName() 
						and self:hasSkills(sgs.need_equip_skill .. "|" .. sgs.lose_equip_skill, friend) then
							target = friend
							break
					end
				end
				for _, friend in ipairs(self.friends) do
					if not self:getSameEquip(card, friend) and friend:objectName() ~= who:objectName() then
						target = friend
						break
					end
				end
			end
		end
	else
		local judges = who:getJudgingArea()
		if who:containsTrick("YanxiaoCard") then
			for _, judge in sgs.qlist(judges) do
				if judge:isKindOf("YanxiaoCard") then
					card = sgs.Sanguosha:getCard(judge:getEffectiveId())
					for _, friend in ipairs(self.friends) do
						if not friend:containsTrick(judge:objectName()) and not self.room:isProhibited(self.player, friend, judge) 
							and not friend:getJudgingArea():isEmpty() then
							target = friend
							break
						end
					end
					if target then break end
					for _, friend in ipairs(self.friends) do
						if not friend:containsTrick(judge:objectName()) and not self.room:isProhibited(self.player, friend, judge) then
							target = friend
							break
						end
					end
					if target then break end
				end
			end
		end
		if card==nil or target==nil then
			if not who:hasEquip() or self:hasSkills(sgs.lose_equip_skill, who) then return nil end
			local card_id = self:askForCardChosen(who, "e", "snatch")
			if card_id >= 0 and who:hasEquip(sgs.Sanguosha:getCard(card_id)) then card = sgs.Sanguosha:getCard(card_id) end

			if card then
				if card:isKindOf("Armor") or card:isKindOf("DefensiveHorse") then 
					self:sort(self.friends, "defense")
				else
					self:sort(self.friends, "handcard")
					self.friends = sgs.reverse(self.friends)
				end

				for _, friend in ipairs(self.friends) do
					if not self:getSameEquip(card, friend) and friend:objectName() ~= who:objectName() and self:hasSkills(sgs.lose_equip_skill .. "|shensu" , friend) then
						target = friend
						break
					end
				end
				for _, friend in ipairs(self.friends) do
					if not self:getSameEquip(card, friend) and friend:objectName() ~= who:objectName() then
						target = friend
						break
					end
				end
			end			
		end
	end

	if return_prompt == "card" then return card
	elseif return_prompt == "target" then return target
	else
		return (card and target)
	end
end

sgs.ai_skill_discard.qiaobian = function(self, discard_num, min_num, optional, include_equip)
	local to_discard = {}
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local stealer
	for _, ap in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if ap:hasSkill("tuxi") and self:isEnemy(ap) then stealer = ap end
	end
	local card	
	for i=1, #cards, 1 do
		local isPeach = cards[i]:isKindOf("Peach")
		if isPeach then
			if stealer and self:isEnemy(stealer) and self.player:getHandcardNum()<=2 and self.player:getHp() > 2 
			and (not stealer:containsTrick("supply_shortage") or stealer:containsTrick("YanxiaoCard")) then
				card = cards[i]
				break
			end
			local to_discard_peach = true
			for _,fd in ipairs(self.friends) do
				if fd:getHp()<=2 and not fd:hasSkill("niepan") then
					to_discard_peach = false
				end
			end
			if to_discard_peach then
				card = cards[i]
				break
			end
		else
			card = cards[i]
			break
		end
	end
	if card == nil then return {} end
	table.insert(to_discard, card:getEffectiveId())
	current_phase = self.player:getMark("qiaobianPhase")
	if current_phase == sgs.Player_Judge and not self.player:isSkipped(sgs.Player_Judge) then
		if not self.player:containsTrick("YanxiaoCard") then
			if (self.player:containsTrick("lightning") and not self:hasWizard(self.friends) and self:hasWizard(self.enemies))
				or (self.player:containsTrick("lightning") and #self.friends > #self.enemies) then
				return to_discard
			elseif self.player:containsTrick("supply_shortage") then
				if self.player:getHp() > self.player:getHandcardNum() then return to_discard end
				local cardstr = sgs.ai_skill_use["@@tuxi"](self, "@tuxi")
				if cardstr:match("->") then
					local targetstr = cardstr:split("->")[2]
					local targets = targetstr:split("+")
					if #targets == 2 then
						return to_discard
					end
				end
			elseif self.player:containsTrick("indulgence") then 
				if self.player:getHandcardNum() > 3 or self.player:getHandcardNum() > self.player:getHp() - 1 then return to_discard end
				for _, friend in ipairs(self.friends_noself) do
					if not friend:containsTrick("YanxiaoCard") and (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
						return to_discard
					end
				end
			end
		end
	elseif current_phase == sgs.Player_Draw and not self.player:isSkipped(sgs.Player_Draw) and not self.player:hasSkill("tuxi") then
		local cardstr = sgs.ai_skill_use["@@tuxi"](self, "@tuxi")
		if cardstr:match("->") then
			local targetstr = cardstr:split("->")[2]
			local targets = targetstr:split("+")
			if #targets == 2 then
				return to_discard
			end
		end
		return {}
	elseif current_phase == sgs.Player_Play and not self.player:isSkipped(sgs.Player_Play) then
		self:sortByKeepValue(cards)
		table.remove(to_discard)
		table.insert(to_discard, cards[1]:getEffectiveId())
		
		self:sort(self.enemies, "defense")
		self:sort(self.friends, "defense")
		self:sort(self.friends_noself, "defense")

		for _, friend in ipairs(self.friends) do
			if not friend:getCards("j"):isEmpty() and not friend:containsTrick("YanxiaoCard") and card_for_qiaobian(self, friend, ".") then
				-- return "@QiaobianCard=" .. card:getEffectiveId() .."->".. friend:objectName()
				return to_discard
			end
		end
		
		for _, enemy in ipairs(self.enemies) do
			if not enemy:getCards("j"):isEmpty() and enemy:containsTrick("YanxiaoCard") and card_for_qiaobian(self, enemy, ".") then
				return to_discard
			end
		end

		for _, friend in ipairs(self.friends_noself) do
			if not friend:getCards("e"):isEmpty() and self:hasSkills(sgs.lose_equip_skill, friend) and card_for_qiaobian(self, friend, ".") then
				return to_discard
			end
		end

		local top_value = 0
		for _, hcard in ipairs(cards) do
			if not hcard:isKindOf("Jink") then
				if self:getUseValue(hcard) > top_value then	top_value = self:getUseValue(hcard) end
			end
		end
		if top_value >= 3.7 and #(self:getTurnUse())>0 then return {} end

		local targets = {}
		for _, enemy in ipairs(self.enemies) do
			if not self:hasSkills(sgs.lose_equip_skill, enemy) and card_for_qiaobian(self, enemy, ".") then
				table.insert(targets, enemy)
			end
		end
		
		if #targets > 0 then
			self:sort(targets, "defense")
			-- return "@QiaobianCard=" .. card:getEffectiveId() .."->".. targets[#targets]:objectName()
			return to_discard
		end
	elseif current_phase == sgs.Player_Discard and not self.player:isSkipped(sgs.Player_Discard) then
		self:sortByKeepValue(cards)
		table.remove(to_discard)
		table.insert(to_discard, cards[1]:getEffectiveId())
		if self:needBear() then return {} end
		if self.player:getHandcardNum()-1 > self.player:getHp() then
			return to_discard
		end
	end

	return {}
end

sgs.ai_skill_cardchosen.qiaobian = function(self, who, flags)
	if flags == "ej" then
		return card_for_qiaobian(self, who, "card")
	end
end

sgs.ai_skill_playerchosen.qiaobian = function(self, targets)
	local who = self.room:getTag("QiaobianTarget"):toPlayer()
	if who then
		if not card_for_qiaobian(self, who, "target") then self.room:writeToConsole("NULL") end
		return card_for_qiaobian(self, who, "target")
	end
end

sgs.ai_skill_use["@@qiaobian"] = function(self, prompt)
	self:updatePlayers()
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local card = cards[1]

	if prompt == "@qiaobian-2" then
		local cardstr = sgs.ai_skill_use["@@tuxi"](self, "@tuxi")
		if cardstr:match("->") then
			local targetstr = cardstr:split("->")[2]
			-- return "@QiaobianCard=." .. card:getEffectiveId() .. "->" .. targetstr
			return "@QiaobianCard=.->" .. targetstr
		else
			return "."
		end
	end

	if prompt == "@qiaobian-3" then
		-- if self.player:getHandcardNum()-2 > self.player:getHp() then return "." end

		self:sort(self.enemies, "defense")
		for _, friend in ipairs(self.friends) do
			if not friend:getCards("j"):isEmpty() and not friend:containsTrick("YanxiaoCard") and card_for_qiaobian(self, friend, ".") then
				-- return "@QiaobianCard=" .. card:getEffectiveId() .."->".. friend:objectName()
				return "@QiaobianCard=.->".. friend:objectName()
			end
		end
		
		for _, enemy in ipairs(self.enemies) do
			if not enemy:getCards("j"):isEmpty() and enemy:containsTrick("YanxiaoCard") and card_for_qiaobian(self, enemy, ".") then
				-- return "@QiaobianCard=" .. card:getEffectiveId() .."->".. friend:objectName()
				return "@QiaobianCard=.->".. enemy:objectName()
			end
		end

		for _, friend in ipairs(self.friends_noself) do
			if not friend:getCards("e"):isEmpty() and self:hasSkills(sgs.lose_equip_skill, friend) and card_for_qiaobian(self, friend, ".") then
				return "@QiaobianCard=.->".. friend:objectName()
			end
		end

		local top_value = 0
		for _, hcard in ipairs(cards) do
			if not hcard:isKindOf("Jink") then
				if self:getUseValue(hcard) > top_value then	top_value = self:getUseValue(hcard) end
			end
		end
		if top_value >= 3.7 and #(self:getTurnUse())>0 then return "." end

		local targets = {}
		for _, enemy in ipairs(self.enemies) do
			if card_for_qiaobian(self, enemy, ".") then
				table.insert(targets, enemy)
			end
		end
		
		if #targets > 0 then
			self:sort(targets, "defense")
			-- return "@QiaobianCard=" .. card:getEffectiveId() .."->".. targets[#targets]:objectName()
			return "@QiaobianCard=.->".. targets[#targets]:objectName()
		end
	end

	return "."
end

sgs.ai_card_intention.QiaobianCard = function(self, card, from, tos)
	if from:getMark("qiaobianPhase") == 3 then return sgs.ai_card_intention.TuxiCard(self, card, from, tos) end
end

function sgs.ai_cardneed.qiaobian(to, card)
	return to:getCards("h"):length() <= 2
end

sgs.ai_skill_invoke.tuntian = function(self, data)
	if self.player:hasSkill("zaoxian") and #self.enemies == 1 and self.room:alivePlayerCount() == 2
		and self.player:getMark("zaoxian") == 0 and self:hasSkills("noswuyan|qianxun", self.enemies[1]) then
			return false
	end
	return true
end

sgs.ai_slash_prohibit.tuntian = function(self, from, to, card)
	if self:isFriend(to) then return false end
	if not to:hasSkill("zaoxian") then return false end
	if from:hasSkill("tieji") or self:canLiegong(to, from) then
		return false
	end
	local enemies = self:getEnemies(to)
	if #enemies == 1 and self.room:alivePlayerCount() == 2 and self:hasSkills("noswuyan|qianxun|weimu", enemies[1]) then return false end
	if getCardsNum("Jink", to) < 1 or sgs.card_lack[to:objectName()]["Jink"] == 1 or self:isWeak(to) then return false end
	if to:getHandcardNum() >= 3 and to:hasSkill("zaoxian") then return true end	
	return false	
end

local jixi_skill = {}
jixi_skill.name = "jixi"
table.insert(sgs.ai_skills, jixi_skill)
jixi_skill.getTurnUseCard = function(self)
	if self.player:getPile("field"):isEmpty()
		or (self.player:getHandcardNum() >= self.player:getHp() + 2
			and self.player:getPile("field"):length() <= self.room:getAlivePlayers():length() / 2 - 1) then
		return
	end
	local can_use = false
	for i = 0, self.player:getPile("field"):length() - 1, 1 do
		local snatch = sgs.Sanguosha:getCard(self.player:getPile("field"):at(i))
		local snatch_str = ("snatch:jixi[%s:%s]=%d"):format(snatch:getSuitString(), snatch:getNumberString(), self.player:getPile("field"):at(i))
		local jixisnatch = sgs.Card_Parse(snatch_str)

		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if (self.player:distanceTo(player, 1) <= 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, self.player, jixisnatch))
				and not self.room:isProhibited(self.player, player, jixisnatch) and self:hasTrickEffective(jixisnatch, player) then

				local suit = snatch:getSuitString()
				local number = snatch:getNumberString()
				local card_id = snatch:getEffectiveId()
				local card_str = ("snatch:jixi[%s:%s]=%d"):format(suit, number, card_id)
				local snatch = sgs.Card_Parse(card_str)
				assert(snatch)
				return snatch
			end
		end
	end
end

sgs.ai_view_as.jixi = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place == sgs.Player_PlaceSpecial and player:getPileName(card_id) == "field" then
		return ("snatch:jixi[%s:%s]=%d"):format(suit, number, card_id)
	end
end

sgs.ai_skill_cardask["@xiangle-discard"] = function(self, data)
	local target = data:toPlayer()
	if self:isFriend(target) and not self:findLeijiTarget(target, 50, self.player) then return "." end
	local has_peach, has_analeptic, has_slash, has_jink
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if card:isKindOf("Peach") then has_peach = card
		elseif card:isKindOf("Analeptic") then has_analeptic = card
		elseif card:isKindOf("Slash") then has_slash = card
		elseif card:isKindOf("Jink") then has_jink = card
		end
	end

	if has_slash then return "$" .. has_slash:getEffectiveId()
	elseif has_jink then return "$" .. has_jink:getEffectiveId()
	elseif has_analeptic or has_peach then
		if getCardsNum("Jink", target) == 0 and self.player:getMark("drank") > 0 and self:getAllPeachNum(target) == 0 then
			if has_analeptic then return "$" .. has_analeptic:getEffectiveId()
			else return "$" .. has_peach:getEffectiveId()
			end
		end
	else return "."
	end
end

function sgs.ai_slash_prohibit.xiangle(self, from, to)
	if self:isFriend(to, from) then return false end
	local slash_num, analeptic_num, jink_num
	if from:objectName() == self.player:objectName() then
		slash_num = self:getCardsNum("Slash")
		analeptic_num = self:getCardsNum("Analeptic")
		jink_num = self:getCardsNum("Jink")
	else
		slash_num = getCardsNum("Slash", from)
		analeptic_num = getCardsNum("Analpetic", from)
		jink_num = getCardsNum("Jink", from)
	end
	if self.player:getHandcardNum() == 2 then
		if self.player:hasSkill("beifa") then self.player:setFlags("stack_overflow_xiangle") end
		local needkongcheng = self:needKongcheng()
		self.player:setFlags("-stack_overflow_xiangle")
		if needkongcheng then return slash_num + analeptic_num + jink_num < 2 end
	end
	return slash_num + analeptic_num + math.max(jink_num - 1, 0) < 2
end

sgs.ai_skill_invoke.fangquan = function(self, data)
	if #self.friends == 1 then
		return false
	end

	-- First we'll judge whether it's worth skipping the Play Phase
	local cards = sgs.QList2Table(self.player:getHandcards())
	local shouldUse, range_fix = 0, 0
	local hasCrossbow, slashTo = false, false
	for _, card in ipairs(cards) do
		if card:isKindOf("TrickCard") and self:getUseValue(card) > 3.69 then
			local dummy_use = { isDummy = true }
			self:useTrickCard(card, dummy_use)
			if dummy_use.card then shouldUse = shouldUse + (card:isKindOf("ExNihilo") and 2 or 1) end
		end
		if card:isKindOf("Weapon") then
			local new_range = sgs.weapon_range[card:getClassName()]
			local current_range = self.player:getAttackRange()
			range_fix = math.min(current_range - new_range, 0)
		end
		if card:isKindOf("OffensiveHorse") and not self.player:getOffensiveHorse() then range_fix = range_fix - 1 end
		if card:isKindOf("DefensiveHorse") or card:isKindOf("Armor") and not self:getSameEquip(card) and (self:isWeak() or self:getCardsNum("Jink") == 0) then shouldUse = shouldUse + 1 end
		if card:isKindOf("Crossbow") or self:hasCrossbowEffect() then hasCrossbow = true end
	end

	local slashs = self:getCards("Slash")
	for _, enemy in ipairs(self.enemies) do
		for _, slash in ipairs(slashs) do
			if hasCrossbow and self:getCardsNum("Slash") > 1 and self:slashIsEffective(slash, enemy)
				and self.player:canSlash(enemy, slash, true, range_fix) then
				shouldUse = shouldUse + 2
				hasCrossbow = false
				break
			elseif not slashTo and self:slashIsAvailable() and self:slashIsEffective(slash, enemy)
				and self.player:canSlash(enemy, slash, true, range_fix) and getCardsNum("Jink", enemy) < 1 then
				shouldUse = shouldUse + 1
				slashTo = true
			end
		end
	end
	if shouldUse >= 2 then return end

	-- Then we need to find the card to be discarded
	local limit = self.player:getMaxCards()
	if self.player:isKongcheng() then return false end
	if self:getCardsNum("Peach") >= limit - 2 and self.player:isWounded() then return false end

	local to_discard = nil

	local index = 0
	local all_peaches = 0
	for _, card in ipairs(cards) do
		if isCard("Peach", card, self.player) then
			all_peaches = all_peaches + 1
		end
	end
	if all_peaches >= 2 and self:getOverflow() <= 0 then return false end
	self:sortByKeepValue(cards)
	cards = sgs.reverse(cards)

	for i = #cards, 1, -1 do
		local card = cards[i]
		if not isCard("Peach", card, self.player) and not self.player:isJilei(card) then
			to_discard = card:getEffectiveId()
			break
		end
	end
	return to_discard ~= nil
end

sgs.ai_skill_use["@@fangquan"] = function(self, prompt)
	local to_discard = nil
	local cards = sgs.QList2Table(self.player:getHandcards())
	local index = 0
	local all_peaches = 0
	for _, card in ipairs(cards) do
		if card:isKindOf("Peach") then
			all_peaches = all_peaches + 1
		end
	end
	if all_peaches >= 2 and self:getOverflow() <= 0 then return "." end
	self:sortByKeepValue(cards)
	cards = sgs.reverse(cards)

	for i = #cards, 1, -1 do
		local card = cards[i]
		if not card:isKindOf("Peach") and not self.player:isJilei(card) then
			to_discard = card:getEffectiveId()
			break
		end
	end
	if not to_discard then return "." end

	self:sort(self.friends_noself, "handcard")
	self.friends_noself = sgs.reverse(self.friends_noself)
	for _, target in ipairs(self.friends_noself) do
		if not target:hasSkill("dawu") and self:hasSkills("yongsi|zhiheng|" .. sgs.priority_skill .. "|shensu", target)
			and (not self:willSkipPlayPhase(target) or target:hasSkill("shensu")) then
			return "@FangquanCard=" .. to_discard .. "->" .. target:objectName()
		end
	end
	for _, target in ipairs(self.friends_noself) do
		if target:hasSkill("dawu") then
			local use = true
			for _, p in ipairs(self.friends_noself) do
				if p:getMark("@fog") > 0 then use = false break end
			end
			if use then return "@FangquanCard=" .. to_discard .. "->" .. target:objectName() end
		end
	end
	if #self.friends_noself > 0 then return "@FangquanCard=" .. to_discard .. "->" .. self.friends_noself[1]:objectName() end
	return "."
end

function SmartAI:isTiaoxinTarget(enemy)
	if not enemy then self.room:writeToConsole(debug.traceback()) return end
	if getCardsNum("Slash", enemy) < 1 and self.player:getHp() > 1 and not self:canHit(self.player, enemy)
		and not (enemy:hasWeapon("DoubleSword") and self.player:getGender() ~= enemy:getGender())
		then return true end
	if sgs.card_lack[enemy:objectName()]["Slash"] == 1
		or self:needLeiji(self.player, enemy) 
		or self:getDamagedEffects(self.player, enemy, true)
		or self:needToLoseHp(self.player, enemy, true)
		then return true end
	return false
end

local tiaoxin_skill = {}
tiaoxin_skill.name = "tiaoxin"
table.insert(sgs.ai_skills, tiaoxin_skill)
tiaoxin_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("TiaoxinCard") then return end
	return sgs.Card_Parse("@TiaoxinCard=.")
end

sgs.ai_skill_use_func.TiaoxinCard = function(card,use,self)
	local distance = use.DefHorse and 1 or 0
	local targets = {}
	for _, enemy in ipairs(self.enemies) do
		if enemy:distanceTo(self.player, distance) <= enemy:getAttackRange() and not self:doNotDiscard(enemy) and self:isTiaoxinTarget(enemy) then
			table.insert(targets, enemy)
		end
	end

	if #targets == 0 then return end
	
	sgs.ai_use_priority.TiaoxinCard = 8
	if not self.player:getArmor() and not self.player:isKongcheng() then
		for _, card in sgs.qlist(self.player:getCards("h")) do
			if card:isKindOf("Armor") and self:evaluateArmor(card) > 3 then
				sgs.ai_use_priority.TiaoxinCard = 5.9
				break
			end
		end
	end

	if use.to then
		self:sort(targets, "defenseSlash")
		use.to:append(targets[1])
	end
	use.card = sgs.Card_Parse("@TiaoxinCard=.")
end

sgs.ai_skill_cardask["@tiaoxin-slash"] = function(self, data, pattern, target)
	if target then
		for _, slash in ipairs(self:getCards("Slash")) do
			if self:isFriend(target) and self:slashIsEffective(slash, target) then
				if self:needLeiji(target, self.player) then return slash:toString() end
				if self:getDamagedEffects(target, self.player) then return slash:toString() end
				if self:needToLoseHp(target, self.player, nil, true) then return slash:toString() end
			end
			
			if not self:isFriend(target) and self:slashIsEffective(slash, target) 
				and not self:getDamagedEffects(target, self.player, true) and not self:needLeiji(target, self.player) then
					return slash:toString()
			end
		end
		for _, slash in ipairs(self:getCards("Slash")) do
			if not self:isFriend(target) then
				if not self:needLeiji(target, self.player) and not self:getDamagedEffects(target, self.player, true) then return slash:toString() end
				if not self:slashIsEffective(slash, target) then return slash:toString() end			
			end
		end
	end
	return "."
end


sgs.ai_card_intention.TiaoxinCard = 80

sgs.ai_skill_choice.zhiji = function(self, choice)
	if self.player:getHp() < self.player:getMaxHp()-1 then return "recover" end
	return "draw"
end

sgs.ai_cardneed.jiang = function(to, card, self)
	return isCard("Duel", card, to) or (isCard("Slash", card, to) and card:isRed())
end

local zhiba_pindian_skill = {}
zhiba_pindian_skill.name = "zhiba_pindian"
table.insert(sgs.ai_skills, zhiba_pindian_skill)
zhiba_pindian_skill.getTurnUseCard = function(self)
	if self.player:isKongcheng() or self:needBear() or self:getOverflow() <= 0 or self.player:getKingdom() ~= "wu"
		or self.player:hasFlag("ForbidZhiba") then return end
	return sgs.Card_Parse("@ZhibaCard=.")
end

sgs.ai_use_priority.ZhibaCard = 0

sgs.ai_skill_use_func.ZhibaCard = function(card, use, self)
	local lords = {}
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasLordSkill("zhiba") and not player:isKongcheng() and not player:hasFlag("ZhibaInvoked")
			and not (self:isEnemy(player) and player:getMark("hunzi") > 0) then
				table.insert(lords, player) 
		end
	end
	if #lords == 0 then return end
	self:sort(lords, "defense")
	for _, lord in ipairs(lords) do
		local zhiba_str
		local cards = self.player:getHandcards()

		local max_num = 0, max_card
		local min_num = 14, min_card
		for _, hcard in sgs.qlist(cards) do
			if hcard:getNumber() > max_num then
				max_num = hcard:getNumber()
				max_card = hcard
			end

			if hcard:getNumber() <= min_num then
				if hcard:getNumber() == min_num then
					if min_card and self:getKeepValue(hcard) > self:getKeepValue(min_card) then
						min_num = hcard:getNumber()
						min_card = hcard
					end
				else
					min_num = hcard:getNumber()
					min_card = hcard
				end
			end
		end

		local lord_max_num = 0, lord_max_card
		local lord_min_num = 14, lord_min_card
		local lord_cards = lord:getHandcards()
		local flag = string.format("%s_%s_%s","visible",global_room:getCurrent():objectName(),lord:objectName())
		for _, lcard in sgs.qlist(lord_cards) do			
			if (lcard:hasFlag("visible") or lcard:hasFlag(flag)) and lcard:getNumber() > lord_max_num then
				lord_max_card = lcard
				lord_max_num = lcard:getNumber()
			end
			if lcard:getNumber() < lord_min_num then
				lord_min_num = lcard:getNumber()
				lord_min_card = lcard
			end
		end

		if self:isEnemy(lord) and max_num > 10 and max_num > lord_max_num then
			if isCard("Jink", max_card, self.player) and self:getCardsNum("Jink") == 1 then return end
			if isCard("Peach", max_card, self.player) or isCard("Analeptic", max_card, self.player) then return end
			self.zhiba_pindian_card = max_card:getEffectiveId()
			zhiba_str = "@ZhibaCard=."
		end
		if self:isFriend(lord) and not lord:hasSkill("manjuan") and ((lord_max_num > 0 and min_num <= lord_max_num) or min_num < 7) then
			if isCard("Jink", min_card, self.player) and self:getCardsNum("Jink") == 1 then return end
			self.zhiba_pindian_card = min_card:getEffectiveId()
			zhiba_str = "@ZhibaCard=."
		end

		if zhiba_str then
			use.card = sgs.Card_Parse(zhiba_str)
			if use.to then use.to:append(lord) end
			return
		end
	end
end

sgs.ai_skill_choice.zhiba_pindian = function(self, choices)
	local who = self.room:getCurrent()
	local cards = self.player:getHandcards()
	local has_large_number, all_small_number = false, true
	for _, c in sgs.qlist(cards) do
		if c:getNumber() > 11 then
			has_large_number = true
			break
		end
	end
	for _, c in sgs.qlist(cards) do
		if c:getNumber() > 4 then
			all_small_number = false
			break
		end
	end
	if all_small_number or (self:isEnemy(who) and not has_large_number) then return "reject"
	else return "accept"
	end
end

function sgs.ai_skill_pindian.zhiba_pindian(minusecard, self, requestor, maxcard)
	local cards, maxcard = sgs.QList2Table(self.player:getHandcards())
	local function compare_func(a, b)
		return a:getNumber() > b:getNumber()
	end
	table.sort(cards, compare_func)
	for _, card in ipairs(cards) do
		if self:getUseValue(card) < 6 then maxcard = card break end
	end
	return maxcard or cards[1]
end

sgs.ai_card_intention.ZhibaCard = 0
sgs.ai_choicemade_filter.pindian.zhiba_pindian = function(self, from, promptlist)
	local number = sgs.Sanguosha:getCard(tonumber(promptlist[4])):getNumber()
	local lord = findPlayerByObjectName(self.room, promptlist[5])
	if not lord then return end
	if number < 6 then sgs.updateIntention(from, lord, -60)
	elseif number > 8 then sgs.updateIntention(from, lord, 60) end
end

sgs.ai_need_damaged.hunzi = function(self, attacker, player)
	if player:hasSkill("hunzi") and player:getMark("hunzi") == 0 and self:getEnemyNumBySeat(self.room:getCurrent(), player, player, true) < player:getHp()
		and (player:getHp() > 2 or player:getHp() == 2 and (player:faceUp() or player:hasSkill("guixin") or player:hasSkill("toudu") and not player:isKongcheng())) then
		return true
	end
	return false
end

local zhijian_skill = {}
zhijian_skill.name = "zhijian"
table.insert(sgs.ai_skills, zhijian_skill)
zhijian_skill.getTurnUseCard = function(self)
	local equips = {}
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if card:getTypeId() == sgs.Card_TypeEquip then
			table.insert(equips, card)
		end
	end
	if #equips == 0 then return end

	return sgs.Card_Parse("@ZhijianCard=.")
end

sgs.ai_skill_use_func.ZhijianCard = function(card, use, self)
	local equips = {}
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if card:isKindOf("Armor") or card:isKindOf("Weapon") then
			if not self:getSameEquip(card) then
			elseif card:isKindOf("GudingBlade") and self:getCardsNum("Slash") > 0 then
				local HeavyDamage
				local slash = self:getCard("Slash")
				for _, enemy in ipairs(self.enemies) do
					if self.player:canSlash(enemy, slash, true) and not self:slashProhibit(slash, enemy) and
						self:slashIsEffective(slash, enemy) and not self.player:hasSkill("jueqing") and enemy:isKongcheng() then
							HeavyDamage = true
							break
					end
				end
				if not HeavyDamage then table.insert(equips, card) end					
			else
				table.insert(equips, card)
			end
		elseif card:getTypeId() == sgs.Card_TypeEquip then
			table.insert(equips, card)
		end
	end

	if #equips == 0 then return end

	local select_equip, target
	for _, friend in ipairs(self.friends_noself) do
		for _, equip in ipairs(equips) do
			if not self:getSameEquip(equip, friend) and self:hasSkills(sgs.need_equip_skill .. "|" .. sgs.lose_equip_skill, friend) then
				target = friend
				select_equip = equip
				break
			end
		end
		if target then break end
		for _, equip in ipairs(equips) do
			if not self:getSameEquip(equip, friend) then
				target = friend
				select_equip = equip
				break
			end
		end
		if target then break end
	end

	if not target then return end
	if use.to then
		use.to:append(target)
	end
	local zhijian = sgs.Card_Parse("@ZhijianCard=" .. select_equip:getId())
	use.card = zhijian
end

sgs.ai_card_intention.ZhijianCard = -80
sgs.ai_use_priority.ZhijianCard = sgs.ai_use_priority.RendeCard + 0.1  -- 刘备二张双将的话，优先直谏
sgs.ai_cardneed.zhijian = sgs.ai_cardneed.equip

sgs.ai_skill_invoke.guzheng = function(self, data)
	if self:isLihunTarget(self.player, data:toInt() - 1) then return false end
	local player = self.room:getCurrent()
	local invoke = (self:isFriend(player) and not (player:hasSkill("kongcheng") and player:isKongcheng())) 
					or (data:toInt() >= 3 and not self.player:hasSkill("manjuan"))
					or (data:toInt() == 2 and not self:hasSkills(sgs.cardneed_skill, player) and not self.player:hasSkill("manjuan"))
					or (self:isEnemy(player) and player:hasSkill("kongcheng") and player:isKongcheng())
	return invoke
end

sgs.ai_skill_askforag.guzheng = function(self, card_ids)
	local who = self.room:getCurrent()
	
	local wulaotai = self.room:findPlayerBySkillName("buyi") 
	local Need_buyi = wulaotai and who:getHp() == 1 and self:isFriend(who, wulaotai)
	
	local cards, except_Equip, except_Key = {}, {}, {}
	for _, card_id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(card_id)
		if self.player:hasSkill("zhijian") and not card:isKindOf("EquipCard") then
			table.insert(except_Equip, card)
		end
		if not card:isKindOf("Peach") and not card:isKindOf("Jink") and not card:isKindOf("Analeptic") and
			not card:isKindOf("Nullification") and not (card:isKindOf("EquipCard") and self.player:hasSkill("zhijian")) then
			table.insert(except_Key, card)
		end		
		table.insert(cards, card)
	end
	
	if self:isFriend(who) then
		
		if Need_buyi then
			local buyicard1, buyicard2
			self:sortByKeepValue(cards)
			for _, card in ipairs(cards) do
				if card:isKindOf("TrickCard") and not buyicard1 then
					buyicard1 = card:getEffectiveId()
				end
				if not card:isKindOf("BasicCard") and not buyicard2 then
					buyicard2 = card:getEffectiveId()
				end
				if buyicard1 then break end
			end
			if buyicard1 or buyicard2 then 
				return buyicard1 or buyicard2
			end
		end

		local peach_num, peach, jink, analeptic, slash = 0
		for _, card in ipairs(cards) do
			if card:isKindOf("Peach") then peach = card:getEffectiveId() peach_num = peach_num + 1 end
			if card:isKindOf("Jink") then jink = card:getEffectiveId() end
			if card:isKindOf("Analeptic") then analeptic = card:getEffectiveId() end
			if card:isKindOf("Slash") then slash = card:getEffectiveId() end
		end
		if peach then
			if peach_num > 1 
				or (self:getCardsNum("Peach") >= self.player:getMaxCards())
				or (who:getHp() < getBestHp(who) and who:getHp() < self.player:getHp()) then
					return peach 
			end
		end
		if self:isWeak(who) and (jink or analeptic) then
			return jink or analeptic
		end
		
		for _, card in ipairs(cards) do
			if not card:isKindOf("EquipCard") then
				for _, askill in sgs.qlist(who:getVisibleSkillList()) do
					local callback = sgs.ai_cardneed[askill:objectName()]
					if type(callback)=="function" and callback(who, card, self) then
						return card:getEffectiveId()
					end
				end
			end
		end

		if jink or analeptic or slash then
			return jink or analeptic or slash
		end
		
		for _, card in ipairs(cards) do
			if not card:isKindOf("EquipCard") and not card:isKindOf("Peach") then
				return card:getEffectiveId()
			end
		end
		
	else
		
		if Need_buyi then
			for _, card in ipairs(cards) do
				if card:isKindOf("Slash") then
					return card:getEffectiveId() 
				end
			end 
		end

		for _, card in ipairs(cards) do
			if card:isKindOf("EquipCard") and self.player:hasSkill("zhijian") then
				local Cant_Zhijian = true
				for _, friend in ipairs(self.friends) do
					if not self:getSameEquip(card, friend) then
						Cant_Zhijian = false
					end
				end
				if Cant_Zhijian then 
					return card:getEffectiveId() 
				end
			end
		end
		
		local new_cards = (#except_Key > 0 and except_Key) or (#except_Equip > 0 and except_Equip) or cards
		
		self:sortByKeepValue(new_cards)
		local valueless, slash
		for _, card in ipairs (new_cards) do
			if card:isKindOf("Lightning") and not self:hasSkills(sgs.wizard_harm_skill, who) then
				return card:getEffectiveId()
			end
			
			if card:isKindOf("Slash") then slash = card:getEffectiveId() end
			
			if not valueless and not card:isKindOf("Peach") then
				for _, askill in sgs.qlist(who:getVisibleSkillList()) do
					local callback = sgs.ai_cardneed[askill:objectName()]
					if (type(callback)=="function" and not callback(who, card, self)) or not callback then
						valueless = card:getEffectiveId()
						break
					end
				end
			end
		end
		
		if slash or valueless then
			return slash or valueless 
		end

		return new_cards[1]:getEffectiveId()
	end

			
	return card_ids[1]
end

sgs.ai_chaofeng.erzhang = 5

sgs.ai_skill_cardask["@beige"] = function(self, data)
	local damage = data:toDamage()
	if not self:isFriend(damage.to) or self:isFriend(damage.from) then return "." end
	local to_discard = self:askForDiscard("beige", 1, 1, false, true)
	if #to_discard > 0 then return "$" .. to_discard[1] else return "." end
end

function sgs.ai_cardneed.beige(to, card)
	return to:getCardCount() <= 2
end

function sgs.ai_slash_prohibit.duanchang(self, from, to)
	if from:hasSkill("jueqing") or (from:hasSkill("nosqianxi") and from:distanceTo(to) == 1) then return false end
	if from:hasFlag("NosJiefanUsed") then return false end
	if to:getHp() > 1 or #(self:getEnemies(from)) == 1 then return false end
	if from:getMaxHp() == 3 and from:getArmor() and from:getDefensiveHorse() then return false end
	if from:getMaxHp() <= 3 or (from:isLord() and self:isWeak(from)) then return true end
	if from:getMaxHp() <= 3 or (self.room:getLord() and from:getRole() == "renegade") then return true end
	return false
end

sgs.ai_chaofeng.caiwenji = -5

sgs.ai_skill_invoke.huashen = function(self)
	return self.player:getHp() > 0
end

function sgs.ai_skill_choice.huashen(self, choices, data, xiaode_choice)
	local str = choices
	choices = str:split("+")
	if not xiaode_choice and self.player:getHp() < 1 and str:matchOne("nosbuqu") then return "nosbuqu" end
	if (xiaode_choice and xiaode_choice > 0) or self.player:getPhase() == sgs.Player_RoundStart then
		if not xiaode_choice and self.player:getHp() < 1 and str:matchOne("nosbuqu") then return "nosbuqu" end
		if (self.player:getHandcardNum() >= self.player:getHp() and self.player:getHandcardNum() < 10 and not self:isWeak()) or self.player:isSkipped(sgs.Player_Play) then
			if str:matchOne("keji") then return "keji" end
		end
		if self.player:getHandcardNum() > 4 then
			for _, askill in ipairs(("shuangxiong|nosfuhun|tianyi|xianzhen|paoxiao|luanji|huoji|qixi|duanliang|guose|" ..
			"luoyi|dangxian|neoluoyi|fuhun"):split("|")) do
				if str:matchOne(askill) then return askill end
			end
			if self:findFriendsByType(sgs.Friend_Draw) then
				for _, askill in ipairs(("nosrende|rende|lirang|longluo"):split("|")) do
					if str:matchOne(askill) then return askill end
				end
			end
		end

		if self.player:getLostHp() >= 2 then
			if str:matchOne("qingnang") then return "qingnang" end
			if str:matchOne("jieyin") and self:findFriendsByType(sgs.Friend_MaleWounded) then return "jieyin" end
			if str:matchOne("nosrende") and self:findFriendsByType(sgs.Friend_Draw) then return "nosrende" end
			if str:matchOne("rende") and self:findFriendsByType(sgs.Friend_Draw) then return "rende" end
			for _, askill in ipairs(("juejing|nosmiji|nosshangshi|shangshi|caizhaoji_hujia|kuiwei|" ..
			"neojushou|jushou|zaiqi|kuanggu"):split("|")) do
				if str:matchOne(askill) then return askill end
			end
			if str:matchOne("miji") and self:findFriendsByType(sgs.Friend_Draw) then return "miji" end
		end

		if self.player:getHandcardNum() < 2 then
			if str:matchOne("haoshi") then return "haoshi" end
		end

		if self.player:isWounded() then
			if str:matchOne("qingnang") then return "qingnang" end
			if str:matchOne("jieyin") and self:findFriendsByType(sgs.Friend_MaleWounded) then return "jieyin" end
			if str:matchOne("nosrende") and self:findFriendsByType(sgs.Friend_Draw) then return "nosrende" end
			if str:matchOne("rende") and self:findFriendsByType(sgs.Friend_Draw) then return "rende" end
			for _, askill in ipairs(("juejing|nosmiji"):split("|")) do
				if str:matchOne(askill) then return askill end
			end
			if self.player:getHp() < 2 and self.player:getHandcardNum() == 1 and self:getCardsNum("Peach") == 0 then
				if str:matchOne("shenzhi") then return "shenzhi" end
			end
		end

		if self.player:getCards("e"):length() > 1 then
			for _, askill in ipairs(("kofxiaoji|xiaoji|xuanfeng|nosxuanfeng|shensu|neoluoyi|gongqi"):split("|")) do
				if str:matchOne(askill) then return askill end
			end
			if self:findFriendsByType(sgs.Friend_All) then
				for _, askill in ipairs(("yuanhu|huyuan"):split("|")) do
					if str:matchOne(askill) then return askill end
				end
			end
		end
		
		if self.player:getWeapon() then
			for _, askill in ipairs(("qiangxi|zhulou"):split("|")) do
				if str:matchOne(askill) then return askill end
			end
		end

		for _, askill in ipairs(("manjuan|tuxi|dimeng|haoshi|guanxing|zhiheng|qiaobian|qice|noslijian|lijian|neofanjian|shuijian|shelie|luoshen|" ..
		"yongsi|shude|biyue|yingzi|qingnang|caizhaoji_hujia"):split("|")) do
			if str:matchOne(askill) then return askill end
		end
		
		if str:matchOne("lianli") and self:findFriendsByType(sgs.Friend_Male) then return "lianli" end
		if self:findFriendsByType(sgs.Friend_Draw) then
			for _, askill in ipairs(("nosrende|rende|anxu|mingce"):split("|")) do
				if str:matchOne(askill) then return askill end
			end
		end

		for _, askill in ipairs(("fanjian|duyi|mizhao|quhu|gongxin|duanliang|hongyuan|guose|" ..
		"baobian|ganlu|tiaoxin|zhaolie|moukui|liegong|mengjin|qianxi|tieji|wushuang|juejing|nosfuhun|nosqianxi|yanxiao|jueji|tanhu|huoshui|guhuo|xuanhuo|" ..
		"nosxuanhuo|qiangxi|fangquan|lirang|longluo|nosjujian|lieren|pojun|bawang|qixi|yinling|nosjizhi|jizhi|duoshi|zhaoxin|gongqi|neoluoyi|luoyi|wenjiu|jie|" ..
		"jiangchi|wusheng|longdan|jueqing|xueji|yinghun|longhun|jiuchi|qingcheng|shuangren|kuangfu|nosgongqi|wushen|paoxiao|lianhuan|chouliang|" ..
		"houyuan|jujian|shensu|jisu|luanji|chizhong|zhijian|shuangxiong|xinzhan|ytzhenwei|jieyuan|duanbing|fenxun|guidao|guicai|noszhenlie|wansha|" ..
		"bifa|lianpo|yicong|nosshangshi|shangshi|lianying|tianyi|xianzhen|zongshi|keji|kuiwei|yuanhu|juao|neojushou|jushou|huoji|roulin|fuhun|lihuo|xiaoji|" ..
		"mashu|zhengfeng|xuanfeng|nosxuanfeng|jiushi|dangxian|tannang|qicai|taichen|hongyan|kurou|lukang_weiyan|yicai|beifa|qinyin|zonghuo|" ..
		"shouye|shaoying|xingshang|suishi|yuwen|gongmou|weiwudi_guixin|wuling|shenfen"):split("|")) do
			if str:matchOne(askill) then return askill end
		end
	else
		if self.player:getHp() == 1 then
			if str:matchOne("wuhun") then return "wuhun" end
			if str:matchOne("buqu") and self.player:getPile("buqu"):length() <= 3 then return "buqu" end
			for _, askill in ipairs(("wuhun|duanchang|jijiu|longhun|jiushi|jiuchi|buyi|huilei|dushi|buqu|zhuiyi|jincui"):split("|")) do
				if str:matchOne(askill) then return askill end
			end
		end

		if str:matchOne("qiuyuan") and self.room:alivePlayerCount() > 2 then return "qiuyuan" end

		if (self:getAllPeachNum() > 0 or self.player:getHp() > 1 or not self:isWeak()) then
			if str:matchOne("guixin") and self.room:alivePlayerCount() > 3 then return "guixin" end
			if str:matchOne("yiji") then return "yiji" end
			if str:matchOne("yuce") and not self.player:isKongcheng() then return "yuce" end
			if self.player:getMark("@tied") > 0 and str:matchOne("tongxin") then return "tongxin" end
			for _, askill in ipairs(("fankui|jieming|chengxiang|vsganglie|neoganglie|ganglie|enyuan|fangzhu|nosenyuan|langgu"):split("|")) do
				if str:matchOne(askill) then return askill end
			end
		end
		
		if self.player:isKongcheng() then
			if str:matchOne("kongcheng") then return "kongcheng" end
		end
		
		for _, askill in ipairs(("yizhong|bazhen"):split("|")) do
			if str:matchOne(askill) and not self.player:getArmor() then return askill end
		end
		
		if not self.player:faceUp() then
			for _, askill in ipairs(("guixin|jiushi|cangni"):split("|")) do
				if str:matchOne(askill) then return askill end
			end
		end

		if self.player:getHandcardNum() > self.player:getHp() and self.player:getCards("e"):length() > 0 then
			if str:matchOne("yanzheng") then return "yanzheng" end
		end

		if self.player:getCards("e"):length() > 1 then
			for _, askill in ipairs(sgs.lose_equip_skill:split("|")) do
				if str:matchOne(askill) then return askill end
			end
		end

		for _, askill in ipairs(("noswuyan|weimu|wuyan|guzheng|luoying|xiliang|kanpo|liuli|beige|qingguo|gushou|mingzhe|xiangle|feiying|longdan"):split("|")) do
			if str:matchOne(askill) then return askill end
		end

		for _, askill in ipairs(("yiji|fankui|jieming|vsganglie|neoganglie|ganglie|enyuan|fangzhu|nosenyuan|langgu"):split("|")) do
			if str:matchOne(askill) then return askill end
		end

		for _, askill in ipairs(("huangen|mingshi|jianxiong|tanlan|qianxun|tianxiang|danlao|juxiang|huoshou|zhichi|yicong|wusheng|wushuang|" ..
		"leiji|guhuo|nosshangshi|shangshi|zhiyu|lirang|tianming|jieyuan|xiaoguo|jijiu|buyi|jiang|guidao|guicai|lianying|mingshi|shushen|shuiyong|" ..
		"tiandu|noszhenlie"):split("|")) do
			if str:matchOne(askill) then return askill end
		end

		if self.player:getCards("e"):length() > 0 then
			for _, askill in ipairs(sgs.lose_equip_skill:split("|")) do
				if str:matchOne(askill) then return askill end
			end
			if str:matchOne("qicai") then return "qicai" end
		end

		for _, askill in ipairs(("xingshang|weidi|chizhong|jilei|sijian|badao|nosjizhi|jizhi|anxian|wuhun|hongyan|buqu|dushi|zhuiyi|huilei"):split("|")) do
			if str:matchOne(askill) then return askill end
		end

		for _, askill in ipairs(("jincui|beifa|yanzheng|xiaoji|xuanfeng|nosxuanfeng|longhun|jiushi|jiuchi|nosjiefan|fuhun|zhenlie|kuanggu|lianpo"):split("|")) do
			if str:matchOne(askill) then return askill end
		end
		
		for _, askill in ipairs(("gongmou|weiwudi_guixin|wuling|kuangbao"):split("|")) do
			if str:matchOne(askill) then return askill end
		end
	end
	for index = #choices, 1, -1 do
		if ("renjie|benghuai|shenjun|dongcha|yishe|shiyong|wumou|yaowu"):match(choices[index]) then
			table.remove(choices,index)
		end
	end
	if not xiaode_choice and #choices > 0 then
		return choices[math.random(1, #choices)]
	end
end

sgs.ai_suit_priority.jiang = function(self, card) 
	return (card:isKindOf("Slash") or card:isKindOf("Duel")) and "diamond|heart|club|spade" or "club|spade|diamond|heart"
end

sgs.ai_cardneed.jiang = function(to, card, self)	
	return isCard("Duel", card, to) or (isCard("Slash", card, to) and card:isRed())	
end