
local qingnang_skill = {}
qingnang_skill.name = "qingnang"
table.insert(sgs.ai_skills, qingnang_skill)
qingnang_skill.getTurnUseCard = function(self)
	if self.player:getHandcardNum() < 1 then return nil end
	if self.player:usedTimes("QingnangCard") > 0 then return nil end
	
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)

	local compare_func = function(a, b)
		local v1 = self:getKeepValue(a) + ( a:isRed() and 50 or 0 ) + ( a:isKindOf("Peach") and 50 or 0 )
		local v2 = self:getKeepValue(b) + ( b:isRed() and 50 or 0 ) + ( b:isKindOf("Peach") and 50 or 0 )
		return v1 < v2
	end
	table.sort(cards, compare_func)

	local card_str = ("@QingnangCard=%d%s"):format(cards[1]:getId(), "&qingnang")
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.QingnangCard = function(card, use, self)
	local arr1, arr2 = self:getWoundedFriend()
	local target = nil

	if #arr1 > 0 and (self:isWeak(arr1[1]) or self:getOverflow() >= 1) then target = arr1[1] end
	if target then
		use.card = card
		if use.to then use.to:append(target) end 
		return
	end	
end

sgs.ai_use_priority.QingnangCard = 4.2
sgs.ai_card_intention.QingnangCard = -100

sgs.dynamic_value.benefit.QingnangCard = true

sgs.ai_view_as.jijiu = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place ~= sgs.Player_PlaceSpecial and card:isRed() and player:getPhase() == sgs.Player_NotActive
		and not player:hasFlag("Global_PreventPeach") then
		return ("peach:jijiu[%s:%s]=%d%s"):format(suit, number, card_id, "&qiangnang")
	end
end

sgs.jijiu_suit_value = {
	heart = 6,
	diamond = 6
}

sgs.ai_cardneed.jijiu = function(to, card)
	return card:isRed()
end

sgs.ai_suit_priority.jijiu= "club|spade|diamond|heart"

sgs.ai_skill_cardask["@wushuang-slash-1"] = function(self, data, pattern, target)
	if sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) then return "." end
	if not target:hasSkill("jueqing") and (self.player:hasSkill("wuyan") or target:hasSkill("wuyan")) then return "." end
	if self:getCardsNum("Slash") < 2 and not (self.player:getHandcardNum() == 1 and self.player:hasSkills(sgs.need_kongcheng)) then return "." end
end

sgs.ai_skill_cardask["@multi-jink-start"] = function(self, data, pattern, target, target2, arg)
	local rest_num = tonumber(arg)
	if rest_num == 1 then return sgs.ai_skill_cardask["slash-jink"](self, data, pattern, target) end
	if sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) then return "." end
	if sgs.ai_skill_cardask["slash-jink"](self, data, pattern, target) == "." then return "." end
	if self.player:hasSkill("kongcheng") then
		if self.player:getHandcardNum() == 1 and self:getCardsNum("Jink") == 1 and target:hasWeapon("GudingBlade") then return "." end
	else
		if self:getCardsNum("Jink") < rest_num and self:hasLoseHandcardEffective() then return "." end
	end
end

sgs.ai_skill_cardask["@multi-jink"] = sgs.ai_skill_cardask["@multi-jink-start"]


function SmartAI:getLijianCard()
	local card_id
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	local lightning = self:getCard("Lightning")

	if self:needToThrowArmor() then
		card_id = self.player:getArmor():getId()
	elseif self.player:getHandcardNum() > self.player:getHp() then			
		if lightning and not self:willUseLightning(lightning) then
			card_id = lightning:getEffectiveId()
		else	
			for _, acard in ipairs(cards) do
				if (acard:isKindOf("BasicCard") or acard:isKindOf("EquipCard") or acard:isKindOf("AmazingGrace"))
					and not acard:isKindOf("Peach") then 
					card_id = acard:getEffectiveId()
					break
				end
			end
		end
	elseif not self.player:getEquips():isEmpty() then
		local player = self.player
		if player:getWeapon() then card_id = player:getWeapon():getId()
		elseif player:getOffensiveHorse() then card_id = player:getOffensiveHorse():getId()
		elseif player:getDefensiveHorse() then card_id = player:getDefensiveHorse():getId()
		elseif player:getArmor() and player:getHandcardNum() <= 1 then card_id = player:getArmor():getId()
		end
	end
	if not card_id then
		if lightning and not self:willUseLightning(lightning) then
			card_id = lightning:getEffectiveId()
		else
			for _, acard in ipairs(cards) do
				if (acard:isKindOf("BasicCard") or acard:isKindOf("EquipCard") or acard:isKindOf("AmazingGrace"))
				  and not acard:isKindOf("Peach") then 
					card_id = acard:getEffectiveId()
					break
				end
			end
		end
	end
	return card_id
end

function SmartAI:findLijianTarget(card_name, use)
	local lord = self.room:getLord()
	local duel = sgs.Sanguosha:cloneCard("duel")

	local findFriend_maxSlash = function(self, first)
		self:log("Looking for the friend!")
		local maxSlash = 0
		local friend_maxSlash
		local nos_fazheng, fazheng
		for _, friend in ipairs(self.friends_noself) do
			if friend:isMale() and self:hasTrickEffective(duel, first, friend) then
				if friend:hasSkill("nosenyuan") and friend:getHp() > 1 then nos_fazheng = friend end
				if friend:hasSkill("enyuan") and friend:getHp() > 1 then fazheng = friend end
				if (getCardsNum("Slash", friend) > maxSlash) then
					maxSlash = getCardsNum("Slash", friend)
					friend_maxSlash = friend
				end
			end
		end

		if friend_maxSlash then
			local safe = false
			if first:hasSkills("fankui|enyuan|ganglie") and not first:hasSkills("wuyan|noswuyan") then
				if (first:getHp() <= 1 and first:getHandcardNum() == 0) then safe = true end
			elseif (getCardsNum("Slash", friend_maxSlash) >= getCardsNum("Slash", first)) then safe = true end
			if safe then return friend_maxSlash end
		else self:log("unfound")
		end
		if nos_fazheng or fazheng then	return nos_fazheng or fazheng end
		return nil
	end
	
	if self.role == "rebel" or (self.role == "renegade" and sgs.current_mode_players["loyalist"] + 1 > sgs.current_mode_players["rebel"]) then		
		
		if lord and lord:isMale() and not lord:isNude() and lord:objectName() ~= self.player:objectName() then
			self:sort(self.enemies, "handcard")
			local e_peaches = 0
			local loyalist
			
			for _, enemy in ipairs(self.enemies) do
				e_peaches = e_peaches + getCardsNum("Peach", enemy)
				if enemy:getHp() == 1 and self:hasTrickEffective(duel, enemy, lord) and enemy:objectName() ~= lord:objectName()
				and enemy:isMale() and not loyalist then
					loyalist = enemy
					break
				end
			end

			if loyalist and e_peaches < 1 then return loyalist, lord end
		end
		
		if #self.friends_noself >= 2 and self:getAllPeachNum() < 1 then
			local nextplayerIsEnemy
			local nextp = self.player:getNextAlive()
			for i = 1, self.room:alivePlayerCount() do
				if not self:willSkipPlayPhase(nextp) then
					if not self:isFriend(nextp) then nextplayerIsEnemy = true end
					break
				else
					nextp = nextp:getNextAlive()
				end
			end	
			if nextplayerIsEnemy then
				local round = 50
				local to_die, nextfriend
				self:sort(self.enemies, "hp")
				
				for _, a_friend in ipairs(self.friends_noself) do
					if a_friend:getHp() == 1 and a_friend:isKongcheng() and not a_friend:hasSkills("kongcheng|yuwen") and a_friend:isMale() then
						for _, b_friend in ipairs(self.friends_noself) do
							if b_friend:objectName() ~= a_friend:objectName() and b_friend:isMale() and self:playerGetRound(b_friend) < round
							and self:hasTrickEffective(duel, a_friend, b_friend) then
							
								round = self:playerGetRound(b_friend)
								to_die = a_friend
								nextfriend = b_friend
								
							end
						end
						if to_die and nextfriend then break end
					end
				end

				if to_die and nextfriend then return to_die, nextfriend end
			end
		end
	end
	
	if not self.player:hasUsed(card_name) then
		self:sort(self.enemies, "defense")
		local males, others = {}, {}
		local first, second
		local zhugeliang_kongcheng, xunyu

		for _, enemy in ipairs(self.enemies) do
			if enemy:isMale() and not enemy:hasSkills("wuyan|noswuyan") then
				if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then zhugeliang_kongcheng = enemy
				elseif enemy:hasSkill("jieming") then xunyu = enemy
				else
					for _, anotherenemy in ipairs(self.enemies) do
						if anotherenemy:isMale() and anotherenemy:objectName() ~= enemy:objectName() then
							if #males == 0 and self:hasTrickEffective(duel, enemy, anotherenemy) then
								if not (enemy:hasSkill("hunzi") and enemy:getMark("hunzi") < 1 and enemy:getHp() == 2) then
									table.insert(males, enemy)
								else
									table.insert(others, enemy)
								end
							end
							if #males == 1 and self:hasTrickEffective(duel, males[1], anotherenemy) then
								if not anotherenemy:hasSkills("nosjizhi|jizhi|jiang") then
									table.insert(males, anotherenemy)
								else
									table.insert(others, anotherenemy)
								end
								if #males >= 2 then break end
							end
						end
					end
				end
				if #males >= 2 then break end
			end
		end
		
		if #males >= 1 and males[1]:getHp() == 1 then
			if lord and self:isFriend(lord) and lord:isMale() and lord:objectName() ~= males[1]:objectName() and self:hasTrickEffective(duel, males[1], lord)
				and not lord:isLocked(duel) and lord:objectName() ~= self.player:objectName() and lord:isAlive()
				and (getCardsNum("Slash", males[1]) < 1
					or getCardsNum("Slash", males[1]) < getCardsNum("Slash", lord)
					or self:getKnownNum(males[1]) == males[1]:getHandcardNum() and getKnownCard(males[1], self.player, "Slash", true, "he") == 0)
				then
				return males[1], lord
			end
			
			local afriend = findFriend_maxSlash(self, males[1])
			if afriend and afriend:objectName() ~= males[1]:objectName() then
				return males[1], afriend
			end
		end
		
		-- @todo
		--[[
		if #males == 1 then
			if isLord(males[1]) and sgs.turncount <= 1 and self.role == "rebel" and self.player:aliveCount() >= 3 then
				local p_slash, max_p, max_pp = 0
				for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
					if p:isMale() and not self:isFriend(p) and p:objectName() ~= males[1]:objectName() and self:hasTrickEffective(duel, males[1], p) and not p:isLocked(duel)
						and p_slash < getCardsNum("Slash", p) then
						if p:getKingdom() == males[1]:getKingdom() then
							max_p = p
							break
						elseif not max_pp then
							max_pp = p
						end
					end
				end
				if max_p then table.insert(males, max_p) end
				if max_pp and #males == 1 then table.insert(males, max_pp) end
			end
		end
		]]
		
		if #males == 1 then
			if #others >= 1 and not others[1]:isLocked(duel) then
				table.insert(males, others[1])
			elseif xunyu and not xunyu:isLocked(duel) then
				if getCardsNum("Slash", males[1]) < 1 then
					table.insert(males, xunyu)
				else
					local drawcards = 0
					for _, enemy in ipairs(self.enemies) do
						local x = enemy:getMaxHp() > enemy:getHandcardNum() and math.min(5, enemy:getMaxHp() - enemy:getHandcardNum()) or 0
						if x > drawcards then drawcards = x end
					end
					if drawcards <= 2 then
						table.insert(males, xunyu)
					end
				end
			end
		end
		
		if #males == 1 and #self.friends_noself > 0 then
			self:log("Only 1")
			first = males[1]
			if zhugeliang_kongcheng and self:hasTrickEffective(duel, first, zhugeliang_kongcheng) then
				table.insert(males, zhugeliang_kongcheng)
			else
				local friend_maxSlash = findFriend_maxSlash(self, first)
				if friend_maxSlash then table.insert(males, friend_maxSlash) end
			end
		end
		
		if #males >= 2 then
			first = males[1]
			second = males[2]
			--[[
			if lord and first:getHp() <= 1 then
				if self.player:isLord() then 
					local friend_maxSlash = findFriend_maxSlash(self, first)
					if friend_maxSlash then second = friend_maxSlash end
				elseif lord:isMale() and not lord:hasSkills("wuyan|noswuyan") then
					if self.role=="rebel" and not first:isLord() and self:hasTrickEffective(duel, first, lord) then
						second = lord
					else
						if ( (self.role == "loyalist" or self.role == "renegade") and not first:hasSkills("ganglie|enyuan|neoganglie|nosenyuan") )
							and ( getCardsNum("Slash", first) <= getCardsNum("Slash", second) ) then
							second = lord
						end
					end
				end
			end
			]]
			if first and second and first:objectName() ~= second:objectName() and not second:isLocked(duel) then
				return first, second
			end
		end
	end
end

local lijian_skill = {}
lijian_skill.name = "lijian"
table.insert(sgs.ai_skills, lijian_skill)
lijian_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("LijianCard") or self.player:isNude() then
		return
	end
	local card_id = self:getLijianCard()
	if card_id then return sgs.Card_Parse("@LijianCard=" .. card_id .. "&lijian") end
end

sgs.ai_skill_use_func.LijianCard = function(card, use, self)
	local first, second = self:findLijianTarget("LijianCard", use)
	if first and second then
		use.card = card
		if use.to then
			use.to:append(first)
			use.to:append(second)
		end
	end
end

sgs.ai_use_value.LijianCard = 8.5
sgs.ai_use_priority.LijianCard = 4



sgs.dynamic_value.damage_card.LijianCard = true

sgs.ai_skill_invoke.biyue = function(self, data)
	return not self:needKongcheng(self.player, true)
end



local luanji_skill = {}
luanji_skill.name = "luanji"
table.insert(sgs.ai_skills, luanji_skill)
luanji_skill.getTurnUseCard = function(self)
	local archery = sgs.Sanguosha:cloneCard("archery_attack")

	local first_found, second_found = false, false
	local first_card, second_card
	if self.player:getHandcardNum() >= 2 then
		local cards = self.player:getHandcards()
		local same_suit = false
		cards = sgs.QList2Table(cards)
		for _, fcard in ipairs(cards) do
			if not (isCard("Peach", fcard, self.player) or isCard("ExNihilo", fcard, self.player) or isCard("AOE", fcard, self.player)) then
				first_card = fcard
				first_found = true
				for _, scard in ipairs(cards) do
					if first_card ~= scard and scard:getSuit() == first_card:getSuit()
						and not (isCard("Peach", scard, self.player) or isCard("ExNihilo", scard, self.player) or isCard("AOE", scard, self.player)) then

						local card_str = ("archery_attack:luanji[%s:%s]=%d+%d&luanji"):format("to_be_decided", 0, first_card:getId(), scard:getId())
						local archeryattack = sgs.Card_Parse(card_str)
						
						assert(archeryattack)
						
						local dummy_use = { isDummy = true }
						self:useTrickCard(archeryattack, dummy_use)
						if dummy_use.card then
							second_card = scard
							second_found = true
							break
						end
					end
				end
				if second_card then break end
			end
		end
	end

	if first_found and second_found then
		local luanji_card = {}
		local first_id = first_card:getId()
		local second_id = second_card:getId()
		local card_str = ("archery_attack:luanji[%s:%s]=%d+%d&luanji"):format("to_be_decided", 0, first_id, second_id)
		local archeryattack = sgs.Card_Parse(card_str)
		assert(archeryattack)
		return archeryattack
	end
end

sgs.ai_skill_invoke.shuangxiong = function(self, data)
	if self.player:isSkipped(sgs.Player_Play) or (self.player:getHp() < 2 and not (self:getCardsNum("Slash") > 1 and self.player:getHandcardNum() >= 3)) or #self.enemies == 0 then
		return false
	end
	local duel = sgs.Sanguosha:cloneCard("duel")

	local dummy_use = { isDummy = true }
	self:useTrickCard(duel, dummy_use)

	return self.player:getHandcardNum() >= 3 and dummy_use.card
end

sgs.ai_cardneed.shuangxiong = function(to, card, self)
	return not self:willSkipDrawPhase(to)
end

local shuangxiong_skill = {}
shuangxiong_skill.name = "shuangxiong"
table.insert(sgs.ai_skills, shuangxiong_skill)
shuangxiong_skill.getTurnUseCard = function(self)
	if self.player:getMark("shuangxiong") == 0 then return nil end
	local mark = self.player:getMark("shuangxiong")

	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)

	local card
	for _, acard in ipairs(cards) do
		if (acard:isRed() and mark == 2) or (acard:isBlack() and mark == 1) then
			card = acard
			break
		end
	end

	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("duel:shuangxiong[%s:%s]=%d&shuangxiong"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end


luanwu_skill = {}
luanwu_skill.name = "luanwu"
table.insert(sgs.ai_skills, luanwu_skill)
luanwu_skill.getTurnUseCard = function(self)
	if self.player:getMark("@chaos") <= 0 then return end
	--if self.room:getMode() == "_mini_13" then return sgs.Card_Parse("@LuanwuCard=.&luanwu") end
	local good, bad = 0, 0
	local lord = self.room:getLord()
	if lord and self.role ~= "rebel" and self:isWeak(lord) then return end
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self:isWeak(player) then
			if self:isFriend(player) then bad = bad + 1
			else good = good + 1
			end
		end
	end
	if good == 0 then return end

	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		local hp = math.max(player:getHp(), 1)
		if getCardsNum("Analeptic", player, self.player) > 0 then
			if self:isFriend(player) then good = good + 1.0 / hp
			else bad = bad + 1.0 / hp
			end
		end

		local has_slash = (getCardsNum("Slash", player, self.player) > 0)
		local can_slash = false
		if not can_slash then
			for _, p in sgs.qlist(self.room:getOtherPlayers(player)) do
				if player:distanceTo(p) <= player:getAttackRange() then can_slash = true break end
			end
		end
		if not has_slash or not can_slash then
			if self:isFriend(player) then good = good + math.max(getCardsNum("Peach", player, self.player), 1)
			else bad = bad + math.max(getCardsNum("Peach", player, self.player), 1)
			end
		end

		if getCardsNum("Jink", player) == 0 then
			local lost_value = 0
			if player:hasSkills(sgs.masochism_skill) then lost_value = player:getHp() / 2 end
			local hp = math.max(player:getHp(), 1)
			if self:isFriend(player) then bad = bad + (lost_value + 1) / hp
			else good = good + (lost_value + 1) / hp
			end
		end
	end

	if good > bad then return sgs.Card_Parse("@LuanwuCard=.&luanwu") end
end

sgs.ai_skill_use_func.LuanwuCard = function(card, use, self)
	use.card = card
end

sgs.dynamic_value.damage_card.LuanwuCard = true


sgs.ai_skill_invoke.mengjin = function(self, data)
	local effect = data:toSlashEffect()
	if self:isEnemy(effect.to) then
		if self:doNotDiscard(effect.to) then
			return false
		end
	end
	if self:isFriend(effect.to) then 
		return self:needToThrowArmor(effect.to) or self:doNotDiscard(effect.to)
	end
	return not self:isFriend(effect.to)
end



sgs.ai_skill_cardask["@guidao-card"]=function(self, data)
	local judge = data:toJudge()
	local all_cards = self.player:getCards("he")
	if all_cards:isEmpty() then return "." end
	
	local needTokeep = judge.card:getSuit() ~= sgs.Card_Spade and (not self:hasSkill("leiji") or judge.card:getSuit() ~= sgs.Card_Club)
						and sgs.ai_AOE_data and self:playerGetRound(judge.who) < self:playerGetRound(self.player) and self:findLeijiTarget(self.player, 50)
						and (self:getCardsNum("Jink") > 0 or self:hasEightDiagramEffect()) and self:getFinalRetrial() == 1

	local keptspade, keptblack = 0, 0
	if needTokeep then
		if self.player:hasSkill("nosleiji") then keptspade = 2 end
		if self.player:hasSkill("leiji") then keptblack = 2 end
	end
	local cards = {}
	for _, card in sgs.qlist(all_cards) do
		if card:isBlack() and not card:hasFlag("using") then
			if card:getSuit() == sgs.Card_Spade then keptspade = keptspade - 1 end
			keptblack = keptblack - 1
			table.insert(cards, card)
		end
	end

	if #cards == 0 then return "." end
	if keptblack == 1 then return "." end
	if keptspade == 1 and not self.player:hasSkill("leiji") then return "." end

	local card_id = self:getRetrialCardId(cards, judge)
	if card_id == -1 then
		if self:needRetrial(judge) and judge.reason ~= "beige" then
			if self:needToThrowArmor() then return "$" .. self.player:getArmor():getEffectiveId() end
			self:sortByUseValue(cards, true)
			if self:getUseValue(judge.card) > self:getUseValue(cards[1]) then
				return "$" .. cards[1]:getId()
			end
		end
	elseif self:needRetrial(judge) or self:getUseValue(judge.card) > self:getUseValue(sgs.Sanguosha:getCard(card_id)) then
		local card = sgs.Sanguosha:getCard(card_id)
		return "$" .. card_id
	end
	
	return "."
end

function sgs.ai_cardneed.guidao(to, card, self)
	for _, player in sgs.qlist(self.room:getAllPlayers()) do
		if player:containsTrick("lightning") and not player:containsTrick("YanxiaoCard") and self:getFinalRetrial(to, "lightning") == 1  then
			return card:getSuit() == sgs.Card_Spade and card:getNumber() >= 2 and card:getNumber() <= 9 and not self.player:hasSkills("hongyan|wuyan")
		end
		if self:isFriend(player) and self:willSkipDrawPhase(player) and self:getFinalRetrial(to, "supply_shortage") == 1then
			return card:getSuit() == sgs.Card_Club and self:hasSuit("club", true, to)
		end
	end
	if self:getFinalRetrial(to, "nosleiji") == 1 and to:hasSkill("nosleiji") then
		return card:getSuit() == sgs.Card_Spade
	end
	if to:hasSkill("leiji") and self:getFinalRetrial(to, "leiji")then
		return card:isBlack()
	end
end

function SmartAI:findLeijiTarget(player, leiji_value, slasher)
	if not player:hasSkill("leiji") then return end
	if slasher then
		if not self:slashIsEffective(sgs.Sanguosha:cloneCard("slash"), player, slasher, slasher:hasWeapon("QinggangSword")) then return nil end
		if slasher:hasSkill("liegong") and slasher:getPhase() == sgs.Player_Play and self:isEnemy(player, slasher)
			and (player:getHandcardNum() >= slasher:getHp() or player:getHandcardNum() <= slasher:getAttackRange()) then
			return nil
		end
		if slasher:hasSkill("kofliegong") and slasher:getPhase() == sgs.Player_Play
			and self:isEnemy(player, slasher) and player:getHandcardNum() >= slasher:getHp() then
			return nil
		end
		if not self:hasSuit("spade", true, player) and player:getHandcardNum() < 3 then return nil end
		local hasJink
		if getKnownCard(player, self.player, "Jink", true) > 0 then hasJink = true end
		if not hasJink and player:getHandcardNum() >= 3 and getCardsNum("Jink", player, self.player) >= 1 and sgs.card_lack[player:objectName()]["Jink"] ~= 1 then hasJink = true end
		if not hasJink and not self:isWeak(player) and self:hasEightDiagramEffect(player) and not slasher:hasWeapon("QinggangSword") then hasJink = true end
		if not hasJink then return end
	end
	local getCmpValue = function(enemy)
		local value = 0
		if not self:damageIsEffective(enemy, sgs.DamageStruct_Thunder, player) then return 99 end
		if enemy:hasSkill("hongyan") then return 99 end
		if self:cantbeHurt(enemy, player, 2) or self:objectiveLevel(enemy) < 3
			or (enemy:isChained() and not self:isGoodChainTarget(enemy, player, sgs.DamageStruct_Thunder, 2)) then return 100 end
		if not sgs.isGoodTarget(enemy, self.enemies, self) then value = value + 50 end
		if enemy:hasArmorEffect("SilverLion") then value = value + 20 end
		if enemy:hasSkills(sgs.exclusive_skill) then value = value + 10 end
		if enemy:hasSkills(sgs.masochism_skill) then value = value + 5 end
		if enemy:isChained() and self:isGoodChainTarget(enemy, player, sgs.DamageStruct_Thunder, 2) and #(self:getChainedEnemies(player)) > 1 then value = value - 25 end
		if enemy:isLord() then value = value - 5 end
		value = value + enemy:getHp() + sgs.getDefenseSlash(enemy, self) * 0.01
		return value
	end

	local cmp = function(a, b)
		return getCmpValue(a) < getCmpValue(b)
	end

	local enemies = self:getEnemies(player)
	table.sort(enemies, cmp)
	for _, enemy in ipairs(enemies) do
		if getCmpValue(enemy) < leiji_value then return enemy end
	end
	return nil
end

sgs.ai_skill_playerchosen.leiji = function(self, targets)

	local xq = self.room:findPlayerBySkillName("tianxiang")
	if xq and self:isEnemy(xq) and sgs.GetConfig("AIChat", true) and xq:getState() == "robot" then
		xq:speak(sgs.Sanguosha:translate(self.player:getGeneralName()) .. "有种来劈我！")
	end
	
	local mode = self.room:getMode()
	if mode:find("_mini_17") or mode:find("_mini_19") or mode:find("_mini_20") or mode:find("_mini_26") then
		local players = self.room:getAllPlayers();
		for _, aplayer in sgs.qlist(players) do
			if aplayer:getState() ~= "robot" then
				return aplayer
			end
		end
	end

	self:updatePlayers()
	return self:findLeijiTarget(self.player, 100, nil, 1)
end

function SmartAI:needLeiji(to, from)
	return self:findLeijiTarget(to, 50, from)
end

sgs.ai_playerchosen_intention.leiji = 80

function sgs.ai_slash_prohibit.leiji(self, from, to, card)
	if self:isFriend(to, from) then return false end
	if to:hasFlag("QianxiTarget") and (not self:hasEightDiagramEffect(to) or self.player:hasWeapon("QinggangSword")) then return false end
	local hcard = to:getHandcardNum()
	if from:hasSkill("liegong") and (hcard >= from:getHp() or hcard <= from:getAttackRange()) then return false end
	if (from:getHp() >= 4 and (getCardsNum("Peach", from, to) > 0 or from:hasSkills("ganglie|vsganglie"))) or from:hasSkill("hongyan") and #self.friends == 1 then
		return false end
	
	if sgs.card_lack[to:objectName()]["Jink"] == 2 then return true end
	if getKnownCard(to, global_room:getCurrent(), "Jink", true) >= 1 or (self:hasSuit("spade", true, to) and hcard >= 2) or hcard >= 4 then return true end
	if self:hasEightDiagramEffect(to) then return true end
end

sgs.guidao_suit_value = {
	spade = 3.9,
	club = 2.7
}

sgs.ai_suit_priority.guidao= "diamond|heart|club|spade"

sgs.ai_skill_discard.beige = function(self)
	local damage = self.player:getTag("beige_data"):toDamage()
	if not self:isFriend(damage.to) or self:isFriend(damage.from) then return {} end
	return self:askForDiscard("dummy_reason", 1, 1, false, true)
end

function sgs.ai_cardneed.beige(to, card)
	return to:getCardCount(true) <= 2
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


xiongyi_skill = {}
xiongyi_skill.name = "xiongyi"
table.insert(sgs.ai_skills, xiongyi_skill)
xiongyi_skill.getTurnUseCard = function(self)
	if self.player:getMark("@arise") < 1 then return end
	if (#self.friends <= #self.enemies and sgs.turncount > 2 and self.player:getLostHp() > 0) or (sgs.turncount > 1 and self:isWeak()) then
		return sgs.Card_Parse("@XiongyiCard=.&xiongyi")
	end
end

sgs.ai_skill_use_func.XiongyiCard = function(card, use, self)
	use.card = card
	for i = 1, #self.friends do
		if use.to then use.to:append(self.friends[i]) end
	end
end

sgs.ai_card_intention.XiongyiCard = -80
sgs.ai_use_priority.XiongyiCard = 9.31


sgs.ai_skill_askforyiji.lirang = function(self, card_ids)
	local Shenfen_user
	for _, player in sgs.qlist(self.room:getAllPlayers()) do
		if player:hasFlag("ShenfenUsing") then
			Shenfen_user = player
			break
		end
	end

	self:updatePlayers()
	local available_friends = {}
	for _, friend in ipairs(self.friends_noself) do
		local insert = true
		if insert and friend:hasFlag("DimengTarget") then
			local another
			for _, p in sgs.qlist(self.room:getOtherPlayers(friend)) do
				if p:hasFlag("DimengTarget") then
					another = p
					break
				end
			end
			if not another or not self:isFriend(another) then insert = false end
		end
		if insert and Shenfen_user and friend:objectName() ~= Shenfen_user:objectName() and friend:getHandcardNum() < 4 then insert = false end
		if insert then table.insert(available_friends, friend) end
	end

	local cards = {}
	for _, card_id in ipairs(card_ids) do
		table.insert(cards, sgs.Sanguosha:getCard(card_id))
	end
	local id = card_ids[1]

	local card, friend = self:getCardNeedPlayer(cards)
	if card and friend and table.contains(available_friends, friend) then return friend, card:getId() end
	if #available_friends > 0 then
		self:sort(available_friends, "handcard")
		if Shenfen_user and table.contains(available_friends, Shenfen_user) then
			return Shenfen_user, id
		end
		for _, afriend in ipairs(available_friends) do
			if not self:needKongcheng(afriend, true) then
				return afriend, id
			end
		end
		self:sort(available_friends, "defense")
		return available_friends[1], id
	end
	return nil, -1
end


sgs.ai_skill_use["@@shuangren"] = function(self, prompt)
	if self.player:isKongcheng() then return "." end
	self:sort(self.enemies, "handcard")
	local max_card = self:getMaxCard()
	local max_point = max_card:getNumber()

	local slash = sgs.Sanguosha:cloneCard("slash")
	local dummy_use = { isDummy = true }
	self.player:setFlags("slashNoDistanceLimit")
	self:useBasicCard(slash, dummy_use)
	self.player:setFlags("-slashNoDistanceLimit")

	if dummy_use.card then
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) and not enemy:isKongcheng() then
				local enemy_max_card = self:getMaxCard(enemy)
				local enemy_max_point = enemy_max_card and enemy_max_card:getNumber() or 100
				if max_point > enemy_max_point then
					self.shuangren_card = max_card:getEffectiveId()
					return "@ShuangrenCard=.->" .. enemy:objectName()
				end
			end
		end
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) and not enemy:isKongcheng() then
				if max_point >= 10 then
					self.shuangren_card = max_card:getEffectiveId()
					return "@ShuangrenCard=.->" .. enemy:objectName()
				end
			end
		end
		if #self.enemies < 1 then return end
		self:sort(self.friends_noself, "handcard")
		for index = #self.friends_noself, 1, -1 do
			local friend = self.friends_noself[index]
			if not friend:isKongcheng() then
				local friend_min_card = self:getMinCard(friend)
				local friend_min_point = friend_min_card and friend_min_card:getNumber() or 100
				if max_point > friend_min_point then
					self.shuangren_card = max_card:getEffectiveId()
					return "@ShuangrenCard=.->" .. friend:objectName()
				end
			end
		end

		local zhugeliang = self.room:findPlayerBySkillName("kongcheng")
		if zhugeliang and self:isFriend(zhugeliang) and zhugeliang:getHandcardNum() == 1 and zhugeliang:objectName() ~= self.player:objectName() then
			if max_point >= 7 then
				self.shuangren_card = max_card:getEffectiveId()
				return "@ShuangrenCard=.->" .. zhugeliang:objectName()
			end
		end

		for index = #self.friends_noself, 1, -1 do
			local friend = self.friends_noself[index]
			if not friend:isKongcheng() then
				if max_point >= 7 then
					self.shuangren_card = max_card:getEffectiveId()
					return "@ShuangrenCard=.->" .. friend:objectName()
				end
			end
		end
	end
	return "."
end

function sgs.ai_skill_pindian.shuangren(minusecard, self, requestor)
	local maxcard = self:getMaxCard()
	return self:isFriend(requestor) and self:getMinCard() or (maxcard:getNumber() < 6 and minusecard or maxcard)
end

sgs.ai_skill_playerchosen.shuangren = sgs.ai_skill_playerchosen.zero_card_as_slash
sgs.ai_card_intention.ShuangrenCard = sgs.ai_card_intention.TianyiCard
sgs.ai_cardneed.shuangren = sgs.ai_cardneed.bignumber


sgs.ai_skill_playerchosen.sijian = function(self, targets)
	return self:findPlayerToDiscard()
end

sgs.ai_playerchosen_intention.sijian = function(self, from, to)
	local intention = 80
	if (to:hasSkill("kongcheng") and to:getHandcardNum() == 1) or self:needToThrowArmor(to) then
		intention = 0
	end
	sgs.updateIntention(from, to, intention)
end

sgs.ai_skill_invoke.suishi = function(self, data)
	local promptlist = data:toString():split(":")
	local effect = promptlist[1]
	local tianfeng = findPlayerByObjectName(promptlist[2])
	if effect == "draw" then
		return tianfeng and self:isFriend(tianfeng)
	elseif effect == "losehp" then
		return tianfeng and self:isEnemy(tianfeng)
	end
	return false
end

sgs.ai_skill_invoke.kuangfu = function(self, data)
	local damage = data:toDamage()
	if damage.to:hasSkills(sgs.lose_equip_skill) then
		return self:isFriend(damage.to) and not self:isWeak(damage.to)
	end
	local benefit = (damage.to:getCards("e"):length() == 1 and damage.to:getArmor() and self:needToThrowArmor(damage.to))
	if self:isFriend(damage.to) then return benefit end
	return not benefit
end

sgs.ai_skill_choice.kuangfu_equip = function(self, choices, data)
	local who = data:toPlayer()
	if self:isFriend(who) then
		if choices:match("1") and self:needToThrowArmor(who) then return "1" end
		if choices:match("1") and self:evaluateArmor(who:getArmor(), who) < -5 then return "1" end
		if who:hasSkills(sgs.lose_equip_skill) and self:isWeak(who) then
			if choices:match("0") then return "0" end
			if choices:match("3") then return "3" end
		end
	else
		local dangerous = self:getDangerousCard(who)
		if dangerous then
			local card = sgs.Sanguosha:getCard(dangerous)
			if card:isKindOf("Weapon") and choices:match("0") then return "0"
			elseif card:isKindOf("Armor") and choices:match("1") then return "1"
			elseif card:isKindOf("DefensiveHorse") and choices:match("2") then return "2"
			elseif card:isKindOf("OffensiveHorse") and choices:match("3") then return "3"
			end
		end
		if choices:match("1") and who:hasArmorEffect("EightDiagram") and not self:needToThrowArmor(who) then return "1" end
		if who:hasSkills("jijiu|beige|mingce|weimu|qingcheng") and not self:doNotDiscard(who, "e", false, 1, reason) then
			if choices:match("2") then return "2" end
			if choices:match("1") and who:getArmor() and not self:needToThrowArmor(who) then return "1" end
			if choices:match("3") and (not who:hasSkill("jijiu") or who:getOffensiveHorse():isRed()) then return "3" end
			if choices:match("0") and (not who:hasSkill("jijiu") or who:getWeapon():isRed()) then return "0" end
		end
		local valuable = self:getValuableCard(who)
		if valuable then
			local card = sgs.Sanguosha:getCard(valuable)
			if card:isKindOf("Weapon") and choices:match("0") then return "0"
			elseif card:isKindOf("Armor") and choices:match("1") then return "1"
			elseif card:isKindOf("DefensiveHorse") and choices:match("2") then return "2"
			elseif card:isKindOf("OffensiveHorse") and choices:match("3") then return "3"
			end
		end
		if not self:doNotDiscard(who, "e") then
			if choices:match("3") then return "3" end
			if choices:match("1") then return "1" end
			if choices:match("2") then return "2" end
			if choices:match("0") then return "0" end
		end
	end
end

sgs.ai_skill_choice.kuangfu = function(self, choices)
	return "move"
end

local qingcheng_skill = {}
qingcheng_skill.name = "qingcheng"
table.insert(sgs.ai_skills, qingcheng_skill)
qingcheng_skill.getTurnUseCard = function(self, inclusive)
	local equipcard
	if self:needToThrowArmor() then
		equipcard = self.player:getArmor()
	else
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if card:isKindOf("EquipCard") then
				equipcard = card
				break
			end
		end
		if not equipcard then
			for _, card in sgs.qlist(self.player:getCards("he")) do
				if card:isKindOf("EquipCard") and not card:isKindOf("Armor") and not card:isKindOf("DefensiveHorse") then
					equipcard = card
				end
			end
		end
	end

	if equipcard then
		local card_id = equipcard:getEffectiveId()
		local card_str = ("@QingchengCard=" .. card_id .. "&qingcheng")
		local qc_card = sgs.Card_Parse(card_str)
		
		assert(qc_card)

		return qc_card
	end
end

sgs.ai_skill_use_func.QingchengCard = function(card, use, self)
	if true then return end
	local target
	if not target then return end
	use.card = card
	if use.to then
		use.to:append(target)
	end
	return
end

sgs.ai_skill_choice.qingcheng = function(self, choices, data)
	local target = data:toPlayer()
	if self:isFriend(target) then
		if target:hasSkill("shiyong", true) and target:getMark("Qingchengshiyong") == 0 then return "shiyong" end
	end
	if target:getHp() < 1 and target:hasSkill("buqu", true) and target:getMark("Qingchengbuqu") == 0 then return "buqu" end 
	if self:isWeak(target) then
		for _, askill in ipairs((sgs.exclusive_skill .. "|" .. sgs.save_skill):split("|")) do
			if target:hasSkill(askill, true) and target:getMark("Qingcheng" .. askill) == 0 then
				return askill
			end
		end
	end
	for _, askill in ipairs(("noswuyan|weimu|wuyan|guixin|fenyong|liuli|yiji|jieming|neoganglie|fankui|fangzhu|enyuan|nosenyuan|" ..
						"ganglie|vsganglie|langgu|qingguo|luoying|guzheng|jianxiong|longdan|xiangle|renwang|huangen|tianming|yizhong|bazhen|jijiu|" ..
						"beige|longhun|gushou|buyi|mingzhe|danlao|qianxun|jiang|yanzheng|juxiang|huoshou|anxian|zhichi|feiying|" ..
						"tianxiang|xiaoji|xuanfeng|nosxuanfeng|xiaoguo|guhuo|guidao|guicai|nosshangshi|lianying|sijian|mingshi|" ..
						"yicong|zhiyu|lirang|xingshang|shushen|shangshi|leiji|wusheng|wushuang|tuntian|quanji|kongcheng|jieyuan|" ..
						"jilve|wuhun|kuangbao|tongxin|shenjun|ytchengxiang|sizhan|toudu|xiliang|tanlan|shien"):split("|")) do
		if target:hasSkill(askill, true) and target:getMark("Qingcheng" .. askill) == 0 then
			return askill
		end
	end
end

sgs.ai_use_value.QingchengCard = 2
sgs.ai_use_priority.QingchengCard = 7.2
sgs.ai_card_intention.QingchengCard = 0

sgs.ai_choicemade_filter.skillChoice.qingcheng = function(self, player, promptlist)
	local choice = promptlist[#promptlist]
	local target = nil
	for _, p in sgs.qlist(self.room:getOtherPlayers(player)) do
		if p:hasSkill(choice, true) then
			target = p
			break
		end
	end
	if not target then return end
	if choice == "shiyong" then sgs.updateIntention(player, target, -10) else sgs.updateIntention(player, target, 10) end
end
