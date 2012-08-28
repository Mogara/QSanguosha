-- sgs.ai_skill_invoke.qiaobian = true

sgs.ai_skill_discard.qiaobian = function(self, discard_num, min_num, optional, include_equip)
	local to_discard = {}
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local card = cards[1]
	table.insert(to_discard, card:getEffectiveId())
	if #to_discard < 1 then return {} end
	current_phase = self.player:getMark("qiaobianPhase")
	if current_phase == sgs.Player_Judge then
		if (self.player:containsTrick("supply_shortage") and self.player:getHp() > self.player:getHandcardNum()) or
			(self.player:containsTrick("indulgence") and self.player:getHandcardNum() > self.player:getHp()-1) or
			(self.player:containsTrick("lightning") and not self:hasWizard(self.friends) and self:hasWizard(self.enemies)) or
			(self.player:containsTrick("lightning") and #self.friends > #self.enemies) then
			return to_discard
		end
	elseif current_phase == sgs.Player_Draw then
		local cardstr = sgs.ai_skill_use["@@tuxi"](self, "@tuxi")
		if cardstr:match("->") then
			return to_discard
		end
	elseif current_phase == sgs.Player_Play then
		self:sortByKeepValue(cards)
		table.remove(to_discard)
		table.insert(to_discard, cards[1]:getEffectiveId())
		--[[self:sort(self.enemies, "hp")
		for _, friend in ipairs(self.friends) do
			-- if not friend:getCards("j"):isEmpty() and card_for_qiaobian(self, friend, "target") then
			if not friend:getCards("j"):isEmpty() then
				return to_discard
			end
		end

		for _, friend in ipairs(self.friends_noself) do
			-- if not friend:getCards("e"):isEmpty() and self:hasSkills(sgs.lose_equip_skill, friend) and card_for_qiaobian(self, friend, "target") then
			if not friend:getCards("e"):isEmpty() and self:hasSkills(sgs.lose_equip_skill, friend) then
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
			-- if card_for_qiaobian(self, enemy, "target") then
			if not enemy:getCards("e"):isEmpty() then
				return to_discard
			end
		end]]
		-- if self.player:getHandcardNum()-2 > self.player:getHp() then return "." end
		self:sort(self.enemies, "hp")
		for _, friend in ipairs(self.friends) do
			if not friend:getCards("j"):isEmpty() and card_for_qiaobian(self, friend, ".") then
				-- return "@QiaobianCard=" .. card:getEffectiveId() .."->".. friend:objectName()
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
			return to_discard
		end
	elseif current_phase == sgs.Player_Discard then
		self:sortByKeepValue(cards)
		table.remove(to_discard)
		table.insert(to_discard, cards[1]:getEffectiveId())
		if self.player:getHandcardNum()-1 > self.player:getHp() then
			return to_discard
		end
	end

	return {}
end

local function card_for_qiaobian(self, who, return_prompt)
	local card, target
	if self:isFriend(who) then
		local judges = who:getJudgingArea()
		if not judges:isEmpty() then
			for _, judge in sgs.qlist(judges) do
				card = sgs.Sanguosha:getCard(judge:getEffectiveId())
				for _, enemy in ipairs(self.enemies) do
					if not enemy:containsTrick(judge:objectName()) and not self.room:isProhibited(self.player, enemy, judge) then
						target = enemy
						break
					end
				end
				if target then break end
			end
		end

		local equips = who:getCards("e")
		if not target and not equips:isEmpty() and self:hasSkills(sgs.lose_equip_skill, who) then
			for _, equip in sgs.qlist(equips) do
				if equip:isKindOf("OffensiveHorse") then card = equip break
				elseif equip:isKindOf("DefensiveHorse") then card = equip break
				elseif equip:isKindOf("Weapon") then card = equip break
				elseif equip:isKindOf("Armor") then card = equip break
				end
			end

			if card then
				for _, friend in ipairs(self.friends) do
					if friend == who then
					elseif friend:getCards("e"):isEmpty() or not self:getSameEquip(card, friend) then
						target = friend
						break
					end
				end
			end
		end
	else
		if not who:hasEquip() or (who:getCards("e"):length() == 1 and who:getArmor() and who:getArmor():isKindOf("GaleShell")) then return nil end
		local card_id = self:askForCardChosen(who, "e", "snatch")
		if card_id >= 0 and who:hasEquip(sgs.Sanguosha:getCard(card_id)) then card = sgs.Sanguosha:getCard(card_id) end
		local targets = {}
		if card then
			for _, friend in ipairs(self.friends) do
				if friend:getCards("e"):isEmpty() or not self:getSameEquip(card, friend) then
					table.insert(targets, friend)
					break
				end
			end
		end
		
		if #targets > 0 then
			if card:isKindOf("Weapon") or card:isKindOf("OffensiveHorse") then
				self:sort(targets, "threat")
				target = targets[#targets]
			else
				self:sort(targets,"defense")
				target = targets[1]
			end
		end
	end

	if return_prompt == "card" then return card
	elseif return_prompt == "target" then return target
	else
		return (card and target)
	end
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

sgs.ai_skill_use["@qiaobian"] = function(self, prompt)
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

		self:sort(self.enemies, "hp")
		for _, friend in ipairs(self.friends) do
			if not friend:getCards("j"):isEmpty() and card_for_qiaobian(self, friend, ".") then
				-- return "@QiaobianCard=" .. card:getEffectiveId() .."->".. friend:objectName()
				return "@QiaobianCard=.->".. friend:objectName()
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

sgs.ai_card_intention.QiaobianCard = function(card, from, tos, source)
	if from:getPhase() == sgs.Player_Draw then
		for _, to in ipairs(tos) do
			sgs.updateIntention(from, to, sgs.ai_card_intention.TuxiCard)
		end
	end
	return 0
end

sgs.ai_skill_invoke.tuntian = true

local jixi_skill={}
jixi_skill.name="jixi"
table.insert(sgs.ai_skills, jixi_skill)
jixi_skill.getTurnUseCard = function(self)
	if self.player:getPile("field"):isEmpty()
		or (self.player:getHandcardNum()>=self.player:getHp() and
		self.player:getPile("field"):length()<= self.room:getAlivePlayers():length()/2) then
		return
	end
	local snatch=sgs.Sanguosha:getCard(self.player:getPile("field"):first())
	snatch=sgs.Sanguosha:cloneCard("snatch", snatch:getSuit(), snatch:getNumber())
	local use={isDummy=true}
	self:useCardSnatch(snatch,use)
	if use.card then return sgs.Card_Parse("@JixiCard=.") end
end

sgs.ai_skill_use_func.JixiCard = function(card, use, self)
	use.card = sgs.Card_Parse("@JixiCard=.")
end

sgs.ai_skill_askforag.jixi = function(self, card_ids)
	self.jixi=card_ids[math.random(1,#card_ids)]
	return self.jixi
end

sgs.ai_skill_playerchosen.jixi = function(self, targets)
	local snatch = sgs.Sanguosha:getCard(self.jixi)
	snatch = sgs.Sanguosha:cloneCard("snatch", snatch:getSuit(), snatch:getNumber())
	local choices = {}
	for _, target in sgs.qlist(targets) do
		if self:isEnemy(target) and not target:getCards("he"):isEmpty()
			and self:hasTrickEffective(snatch, target) then
			table.insert(choices, target)
		elseif self:isFriend(target) and not target:getCards("j"):isEmpty()
			and self:hasTrickEffective(snatch, target) then
			table.insert(choices, target)
		end
	end

	if #choices == 0 then return targets:at(0) end

	self:sort(choices, "hp")
	return choices[1]
end

sgs.ai_card_intention.JixiCard = sgs.ai_card_intention.Snatch

sgs.dynamic_value.control_card.JixiCard = true

sgs.ai_skill_cardask["@xiangle-discard"] = function(self, data)
	local target = data:toPlayer()
	if self:isFriend(target) and not
		(target:hasSkill("leiji") and (self:getCardsNum("Jink", target)>0 or (not self:isWeak(target) and self:isEquip("EightDiagram",target))))
		then return "." end
	local has_peach, has_anal, has_slash, has_jink
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if card:isKindOf("Peach") then has_peach = card
		elseif card:isKindOf("Analeptic") then has_anal = card
		elseif card:isKindOf("Slash") then has_slash = card
		elseif card:isKindOf("Jink") then has_jink = card
		end
	end

	if has_slash then return "$" .. has_slash:getEffectiveId()
	elseif has_jink then return "$" .. has_jink:getEffectiveId()
	elseif has_anal or has_peach then
		if self:getCardsNum("Jink", target) == 0 and self.player:hasFlag("drank") and self:getAllPeachNum(target) == 0 then
			if has_anal then return "$" .. has_anal:getEffectiveId()
			else return "$" .. has_peach:getEffectiveId()
			end
		end
	else return "."
	end
end

function sgs.ai_slash_prohibit.xiangle(self, to)
	if self:isFriend(to) then return false end
	return self:getCardsNum("Slash")+self:getCardsNum("Analpetic")+math.max(self:getCardsNum("Jink")-1,0) < 2
end

sgs.ai_skill_invoke.fangquan = function(self, data)
	if #self.friends == 1 then
		return false
	end

	local limit = self.player:getMaxCards()
	return self.player:getHandcardNum() <= limit and not self.player:isKongcheng()
end

sgs.ai_skill_discard.fangquan = function(self, discard_num, min_num, optional, include_equip)
	local to_discard = {}
	local cards = sgs.QList2Table(self.player:getHandcards())
	local index = 0
	local all_peaches = 0
	for _, card in ipairs(cards) do
		if card:isKindOf("Peach") then
			all_peaches = all_peaches + 1
		end
	end
	if all_peaches >= 2 and self:getOverflow() <= 0 then return {} end
	self:sortByKeepValue(cards)
	cards = sgs.reverse(cards)

	for i = #cards, 1, -1 do
		local card = cards[i]
		if not card:isKindOf("Peach") and not self.player:isJilei(card) then
			table.insert(to_discard, card:getEffectiveId())
			table.remove(cards, i)
			index = index + 1
			if index == 1 then break end
		end
	end	
	if #to_discard < 1 then return {} 
	else
		return to_discard
	end
end

sgs.ai_skill_playerchosen.fangquan = function(self, targets)
	for _, target in sgs.qlist(targets) do
		if self:isFriend(target) and not target:hasSkill("dawu") and
			self:hasSkills(sgs.priority_skill,target) and not target:containsTrick("indulgence") then
			return target
		end
	end
	for _, target in sgs.qlist(targets) do
		if self:isFriend(target) and not target:hasSkill("dawu") then
			return target
		end
	end
end

sgs.ai_playerchosen_intention.fangquan = -40

local tiaoxin_skill={}
tiaoxin_skill.name="tiaoxin"
table.insert(sgs.ai_skills, tiaoxin_skill)
tiaoxin_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("TiaoxinCard") then return end
	return sgs.Card_Parse("@TiaoxinCard=.")
end

sgs.slash_property = {}
sgs.ai_skill_use_func.TiaoxinCard = function(card,use,self)
	local targets = {}
	for _, enemy in ipairs(self.enemies) do
		sgs.slash_property =
		{
			is_black = false,
			is_red = false,
			is_normal = false,
			is_fire = false,
			is_thunder = false
		}

		local cards = enemy:getHandcards()
		cards = sgs.QList2Table(cards)

		for _, card in ipairs(cards) do
			if card:isKindOf("Slash") then
				if card:isBlack() then sgs.slash_property.is_black = true end
				if card:isRed() then sgs.slash_property.is_red = true end
				if card:isKindOf("FireSlash") then sgs.slash_property.is_fire = true
				elseif card:isKindOf("ThunderSlash") then sgs.slash_property.is_thunder = true
				else sgs.slash_property.is_normal = true
				end
			end
		end

		local slash_useless = false
		local has_armor = self.player:getArmor()
		if has_armor then
			if self.player:getArmor():objectName() == "Vine" then
				if not (sgs.slash_property.is_fire or sgs.slash_property.is_thunder) then
					slash_useless = true
				end
			elseif self.player:getArmor():objectName() == "RenwangShield" then
				if not sgs.slash_property.is_red then
					slash_useless = true
				end
			end
		end

		if enemy:distanceTo(self.player) <= enemy:getAttackRange() and
			(self:getCardsNum("Slash", enemy) == 0 or slash_useless or self:getCardsNum("Jink") > 0) and
			not enemy:isNude() then
			table.insert(targets, enemy)
		end
	end

	if #targets == 0 then return end

	if use.to then
		self:sort(targets, "hp")
		use.to:append(targets[1])
	end
	use.card = sgs.Card_Parse("@TiaoxinCard=.")
end

sgs.ai_skill_choice.tiaoxin = sgs.ai_skill_choice.collateral

sgs.ai_card_intention.TiaoxinCard = 80
sgs.ai_use_priority.TiaoxinCard = 8

sgs.ai_skill_choice.zhiji = function(self, choice)
	if self.player:getHp() < self.player:getMaxHp()-1 then return "recover" end
	return "draw"
end

local zhiba_pindian_skill={}
zhiba_pindian_skill.name="zhiba_pindian"
table.insert(sgs.ai_skills, zhiba_pindian_skill)
zhiba_pindian_skill.getTurnUseCard = function(self)
	if self.player:isKongcheng() or self.player:getHandcardNum() < self.player:getHp() or self.player:getKingdom() ~= "wu"
		or self.player:hasUsed("ZhibaCard") then return end
	return sgs.Card_Parse("@ZhibaCard=.")
end

sgs.ai_skill_use_func.ZhibaCard = function(card, use, self)
	local lords = {}
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasLordSkill("sunce_zhiba") and not player:isKongcheng() then table.insert(lords, player) end
	end
	if #lords == 0 then return end
	if self:needBear() then return end
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
		for _, lcard in sgs.qlist(lord_cards) do
			if lcard:hasFlag("visible") and lcard:getNumber() > lord_max_num then
				lord_max_card = lcard
				lord_max_num = lcard:getNumber()
			end
			if lcard:getNumber() < lord_min_num then
				lord_min_num = lcard:getNumber()
				lord_min_card = lcard
			end
		end

		if self:isEnemy(lord) and max_num > 9 and max_num > lord_max_num then
			zhiba_str = "@ZhibaCard=" .. max_card:getEffectiveId()
		end
		if self:isFriend(lord) and ((lord_max_num > 0 and min_num <= lord_max_num) or min_num < 8) then
			zhiba_str = "@ZhibaCard=" .. min_card:getEffectiveId()
		end

		if zhiba_str then
			use.card = sgs.Card_Parse(zhiba_str)
			if use.to then 
				use.to:append(lord) 
			end
			return
		end
	end
end

sgs.ai_skill_choice.zhiba_pindian = function(self, choices)
	local who = self.room:getCurrent()
	if self:isEnemy(who) then return "reject"
	else return "accept"
	end
end

sgs.ai_skill_choice.sunce_zhiba = function(self, choices)
	return "yes"
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

function sgs.ai_card_intention.ZhibaCard(card, from, tos, source)
	assert(#tos == 1)
	local number = sgs.Sanguosha:getCard(card:getSubcards():first()):getNumber()
	if number < 6 then sgs.updateIntention(from, tos[1], -60)
	elseif number > 8 then sgs.updateIntention(from, tos[1], 60) end
end 

local zhijian_skill={}
zhijian_skill.name="zhijian"
table.insert(sgs.ai_skills, zhijian_skill)
zhijian_skill.getTurnUseCard = function(self)
	local equips = {}
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if card:getTypeId() == sgs.Card_Equip then
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
			else
				table.insert(equips, card)
			end
		elseif card:getTypeId() == sgs.Card_Equip then
			table.insert(equips, card)
		end
	end

	if #equips == 0 then return end

	local select_equip, target
	for _, friend in ipairs(self.friends_noself) do
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

sgs.ai_cardneed.zhijian = sgs.ai_cardneed.equip

sgs.ai_skill_invoke.guzheng = function(self, data)
	local player = self.room:getCurrent()
	return (self:isFriend(player) and not self:hasSkills(sgs.need_kongcheng, player)) or data:toInt() >= 3
end

sgs.ai_skill_askforag.guzheng = function(self, card_ids)
	local who = self.room:getCurrent()
	local cards = {}
	for _, card_id in ipairs(card_ids) do
		table.insert(cards, sgs.Sanguosha:getCard(card_id))
	end

	if self:isFriend(who) then
		self:sortByKeepValue(cards,true)
	else
		self:sortByKeepValue(cards)
	end

	return cards[1]:getEffectiveId()
end

sgs.ai_chaofeng.erzhang = 5

sgs.ai_skill_invoke.beige = function(self, data)
	local damage = data:toDamage()
	return self:isFriend(damage.to) and not self:isFriend(damage.from)
end

function sgs.ai_slash_prohibit.duanchang(self, to)
	if self:isFriend(to) and self:isWeak(to) then return true end
	if self:hasSkills("jueqing|qianxi") then return false end
	return #self.enemies>1 and self:isWeak(to) and (self.player:isLord() or not self:isWeak())
end

sgs.ai_chaofeng.caiwenji = -5

sgs.ai_skill_invoke.huashen = function(self)
	return self.player:getHp() > 0
end

function sgs.ai_skill_choice.huashen(self, choices)
	local str = choices
	choices = str:split("+")
	if self.player:getHp() < 1 and str:matchOne("buqu") then return "buqu" end
	if str:matchOne("guixin2") then return "guixin2" end
	if self.player:getPhase() == sgs.Player_NotActive then
		if self.player:getHp() == 1 then
			if str:matchOne("wuhun") then return "wuhun" end
			for _, askill in ipairs(("wuhun|duanchang|jijiu|longhun|jiushi|jiuchi|buyi|huilei|dushi|juejing|zhuiyi|jiefan"):split("|")) do
				if str:matchOne(askill) then return askill end
			end
		end
		if str:matchOne("guixin") and (not self:isWeak() or self:getAllPeachNum() > 0) then return "guixin" end
		for _, askill in ipairs(sgs.masochism_skill:split("|")) do
			if str:matchOne(askill) and (self.player:getHp() > 1 or self:getAllPeachNum() > 0) then return askill end
		end

		if self.player:isKongcheng() then
			if str:matchOne("kongcheng") then return "kongcheng" end
		end
		for _, askill in ipairs(("yizhong|bazhen"):split("|")) do
				if str:matchOne(askill) and not self.player:getArmor() then return askill end
		end
		 
		for _, askill in ipairs(("wuyan|weimu|kanpo|liuli|qingguo|longdan|xiangle|jiang|" ..
		"danlao|qianxun|juxiang|huoshou|zhichi|jilei|feiying|yicong|wusheng|wushuang|tianxiang|leiji|" ..
		"xuanfeng|luoying|xiliang|guhuo|guidao|guicai|lianying|xiaoji|zhiyu|hongyan|tiandu|guzheng|xingshang|weidi|badao|gushou"):split("|")) do
			if str:matchOne(askill) then return askill end
		end
	else
		assert(self.player:getPhase() == sgs.Player_RoundStart)
		if self.player:getHp() < 1 and str:matchOne("buqu") then return "buqu" end
		if (self.player:getHandcardNum() < 20 and not self:isWeak()) or self.player:isSkipped(sgs.Player_Play) then
			if str:matchOne("keji") then return "keji" end
		end
		if self.player:getHandcardNum() > 5 then
			for _, askill in ipairs(("shuangxiong|fuhun|tianyi|xianzhen|paoxiao|huoji|luanji|qixi|duanliang|guose"):split("|")) do
				if str:matchOne(askill) then return askill end
			end
		end
		if self:isWeak() then
			for _, askill in ipairs(("qingnang|jieyin|zaiqi|longhun|kuanggu|kuiwei|miji|caizhaoji_hujia|jushou|jincui|yuwen"):split("|")) do
				if str:matchOne(askill) then return askill end
			end
		end
		for _, askill in ipairs(("tuxi|dimeng|haoshi|guanxing|manjuan|zhiheng|rende|qiaobian|fangquan|qice|" ..
		"lijian|quhu|fanjian|tieji|liegong|wushuang|shelie|luoshen|yongsi|yingzi|juejing|fuhun|qianxi|" ..
		"gongxin|duanliang|guose|mingce|ganlu|anxu|tiaoxin|xuanhuo|guhuo|roulin|qiangxi|mengjin|lieren|pojun|longluo" ..
		"jiushi|qixi|luoyi|wenjiu|jiuchi|longhun|wusheng|wushen|longdan|gongqi|lihuo|shensu|jiangchi|lianhuan|yinghun|houyuan|jujian|huoji|luanji|gongmou|" ..
		"jueji|zhijian|shuangxiong|xinzhan|chouliang|zhenwei|guidao|guicai|zhenlie|lianpo|mashu|zhengfeng|yicong|jizhi|lianying|xuanfeng|xiaoji|tianyi" ..
		"dangxian|qicai|xianzhen|wansha|zongshi|biyue|hongyan|lukang_weiyan|shipo|kurou|yicai|beifa|qinyin|zonghuo|shaoying|guihan|yishe|fuzuo|shouye"):split("|")) do
			if str:matchOne(askill) then return askill end
		end
	end
	for index = #choices, 1, -1 do
		if ("renjie|benghuai|wuling|liqian|lianli|tongxin|shenjun|xunzhi|dongcha" ..
		"juao")
		:match(choices[index]) then
			table.remove(choices,index)
		end
	end
	if #choices > 0 then
		return choices[math.random(1,#choices)]
	end
end
