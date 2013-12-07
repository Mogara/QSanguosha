--[[
	技能：倨傲
	描述：出牌阶段，你可以选择两张手牌背面向上移出游戏，指定一名角色，被指定的角色到下个回合开始阶段时，跳过摸牌阶段，得到你所移出游戏的两张牌。每阶段限一次 
]]--
local juao_skill={}
juao_skill.name = "juao"
table.insert(sgs.ai_skills, juao_skill)
juao_skill.getTurnUseCard = function(self)
	if self:needBear() then return end
	if not self.player:hasUsed("JuaoCard") and self.player:getHandcardNum() > 1 then
		local card_id = self:getCardRandomly(self.player, "h")
		return sgs.Card_Parse("@JuaoCard=" .. card_id)
	end
end

sgs.ai_skill_use_func.JuaoCard = function(card, use, self)
	local givecard = {}
	local cards = self.player:getHandcards()
	for _, friend in ipairs(self.friends_noself) do
		if friend:getHp() == 1 then --队友快死了
			for _, hcard in sgs.qlist(cards) do
				if hcard:isKindOf("Analeptic") or hcard:isKindOf("Peach") then
					table.insert(givecard, hcard:getId())
				end
				if #givecard == 1 and givecard[1] ~= hcard:getId() then
					table.insert(givecard, hcard:getId())
				elseif #givecard == 2 then
					use.card = sgs.Card_Parse("@JuaoCard=" .. table.concat(givecard, "+"))
					if use.to then 
						use.to:append(friend) 
						self:speak("顶住，你的快递马上就到了。")
					end
					return
				end
			end
		end
		if friend:hasSkill("nosjizhi") then --队友有集智
			for _, hcard in sgs.qlist(cards) do
				if hcard:isKindOf("TrickCard") and not hcard:isKindOf("DelayedTrick") then
					table.insert(givecard, hcard:getId())
				end
				if #givecard == 1 and givecard[1] ~= hcard:getId() then
					table.insert(givecard, hcard:getId())
				elseif #givecard == 2 then
					use.card = sgs.Card_Parse("@JuaoCard=" .. table.concat(givecard, "+"))
					if use.to then use.to:append(friend) end
					return
				end
			end
		end
		if friend:hasSkill("jizhi") then --队友有集智
			for _, hcard in sgs.qlist(cards) do
				if hcard:isKindOf("TrickCard") then
					table.insert(givecard, hcard:getId())
				end
				if #givecard == 1 and givecard[1] ~= hcard:getId() then
					table.insert(givecard, hcard:getId())
				elseif #givecard == 2 then
					use.card = sgs.Card_Parse("@JuaoCard=" .. table.concat(givecard, "+"))
					if use.to then use.to:append(friend) end
					return
				end
			end
		end
		if friend:hasSkill("leiji") then --队友有雷击
			for _, hcard in sgs.qlist(cards) do
				if hcard:getSuit() == sgs.Card_Spade or hcard:isKindOf("Jink") then
					table.insert(givecard, hcard:getId())
				end
				if #givecard == 1 and givecard[1] ~= hcard:getId() then
					table.insert(givecard, hcard:getId())
				elseif #givecard == 2 then
					use.card = sgs.Card_Parse("@JuaoCard=" .. table.concat(givecard, "+"))
					if use.to then 
						use.to:append(friend) 
						self:speak("我知道你有什么牌，哼哼。")
					end
					return
				end
			end
		end
		if friend:hasSkill("xiaoji") or friend:hasSkill("xuanfeng") then --队友有枭姬（旋风）
			for _, hcard in sgs.qlist(cards) do
				if hcard:isKindOf("EquipCard") then
					table.insert(givecard, hcard:getId())
				end
				if #givecard == 1 and givecard[1] ~= hcard:getId() then
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
		if enemy:getHp() == 1 then --敌人快死了
			for _, hcard in sgs.qlist(cards) do
				if hcard:isKindOf("Disaster") then
					table.insert(givecard, hcard:getId())
				end
				if #givecard == 1 and givecard[1] ~= hcard:getId() and
					not hcard:isKindOf("Peach") and not hcard:isKindOf("TrickCard") then
					table.insert(givecard, hcard:getId())
					use.card = sgs.Card_Parse("@JuaoCard=" .. table.concat(givecard, "+"))
					if use.to then use.to:append(enemy) end
					return
				elseif #givecard == 2 then
					use.card = sgs.Card_Parse("@JuaoCard=" .. table.concat(givecard, "+"))
					if use.to then 
						use.to:append(enemy) 
						self:speak("咱最擅长落井下石了。")
					end
					return
				else
				end
			end
		end
		if enemy:hasSkill("yongsi") then --敌人有庸肆
			local players = self.room:getAlivePlayers()
			local extra = self:KingdomsCount(players) --额外摸牌的数目
			if enemy:getCardCount(true) <= extra then --如果敌人快裸奔了
				for _,hcard in sgs.qlist(cards) do
					if hcard:isKindOf("Disaster") then
						table.insert(givecard, hcard:getId())
					end
					if #givecard == 1 and givecard[1] ~= hcard:getId() then
						if not hcard:isKindOf("Peach") and not hcard:isKindOf("ExNihilo") then
							table.insert(givecard, hcard:getId())
							use.card = sgs.Card_Parse("@JuaoCard="..table.concat(givecard, "+"))
							if use.to then
								use.to:append(enemy)
							end
							return 
						end
					end
					if #givecard == 2 then
						use.card = sgs.Card_Parse("@JuaoCard="..table.concat(givecard, "+"))
						if use.to then
							use.to:append(enemy)
						end
						return 
					end
				end
			end
		end
	end
	if #givecard < 2 then
		for _, hcard in sgs.qlist(cards) do
			if hcard:isKindOf("Disaster") then
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
--[[
	技能：贪婪
	描述：每当你受到一次伤害，可与伤害来源进行拼点：若你赢，你获得两张拼点牌 
]]--
sgs.ai_skill_invoke.tanlan = function(self, data)
	local damage = data:toDamage()
	local from = damage.from
	local max_card = self:getMaxCard()
	if not max_card then return end
	if max_card:getNumber() > 10 and self:isFriend(from) then
		if from:getHandcardNum() == 1 and self:needKongcheng(from) then return true end
		if self:getOverflow(from) > 2 then return true end
		if not self:hasLoseHandcardEffective(from) then return true end
	end
	if self:isFriend(from) then return false end
	if max_card:getNumber() > 10 
		or (self.player:getHp() > 2 and self.player:getHandcardNum() > 2 and max_card:getNumber() > 4)
		or (self.player:getHp() > 1 and self.player:getHandcardNum() > 1 and max_card:getNumber() > 7)
		or (from:getHandcardNum() <= 2 and max_card:getNumber() > 2) 
		or (from:getHandcardNum() == 1 and self:hasLoseHandcardEffective(from) and not self:needKongcheng(from))
		or self:getOverflow() > 2 then
		return true
	end
end

sgs.ai_choicemade_filter.skillInvoke.tanlan = function(self, player, promptlist)
	local damage = self.room:getTag("CurrentDamageStruct"):toDamage()
	if damage.from and promptlist[3] == "yes" then
		local target = damage.from
		local intention = 10
		if target:getHandcardNum() == 1 and self:needKongcheng(target) then intention = 0 end
		if self:getOverflow(target) > 2 then intention = 0 end
		if not self:hasLoseHandcardEffective(target) then intention = 0 end
		sgs.updateIntention(player, target, intention)
	end
end

function sgs.ai_skill_pindian.tanlan(minusecard, self, requestor)
	local maxcard = self:getMaxCard()	
	return self:isFriend(requestor) and minusecard or ( maxcard:getNumber() < 6 and minusecard or maxcard )
end

sgs.ai_cardneed.tanlan = sgs.ai_cardneed.bignumber

--[[
	技能：异才
	描述：每当你使用一张非延时类锦囊时(在它结算之前)，可立即对攻击范围内的角色使用一张【杀】 
]]--
sgs.ai_skill_invoke.yicai = function(self, data)
	if self:needBear() then return false end
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy, nil, true) then
			if self:getCardsNum("Slash") > 0 then 
				return true 
			end
		end
	end
end
--[[
	技能：北伐（锁定技）
	描述：当你失去最后一张手牌时，视为对攻击范围内的一名角色使用了一张【杀】
]]--
sgs.ai_skill_playerchosen.beifa = function(self, targets)
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	local targetlist = {}
	for _,p in sgs.qlist(targets) do
		if not self:slashProhibit(slash, p) then
			table.insert(targetlist, p)
		end
	end
	self:sort(targetlist, "defenseSlash")
	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) then
			if self:slashIsEffective(slash, target) then
				if sgs.isGoodTarget(target, targetlist, self) then
					self:speak("嘿！没想到吧？")
					return target
				end
			end
		end
	end
	for i=#targetlist, 1, -1 do
		if sgs.isGoodTarget(targetlist[i], targetlist, self) then
			return targetlist[i]
		end
	end
	return targetlist[#targetlist]
end

sgs.ai_chaofeng.wis_jiangwei = 2
--[[
	技能：后援
	描述：出牌阶段，你可以弃置两张手牌，指定一名其他角色摸两张牌，每阶段限一次 
]]--
local houyuan_skill = {}
houyuan_skill.name = "houyuan"
table.insert(sgs.ai_skills, houyuan_skill)
houyuan_skill.getTurnUseCard = function(self)
	if self:needBear() then return end
	if not self.player:hasUsed("HouyuanCard") and self.player:getHandcardNum() > 1 then
		local givecard = {}
		local index = 0
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		self:sortByKeepValue(cards)
		for _, fcard in ipairs(cards) do
			if not fcard:isKindOf("Peach") then
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
	local AssistTarget = self:AssistTarget()
	if AssistTarget and not AssistTarget:hasSkill("manjuan") and not self:needKongcheng(AssistTarget, true) then
		target = AssistTarget
	else
		target = self:findPlayerToDraw(false, 2)
	end
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local usecards = {cards[1]:getId(), cards[2]:getId()}
	if not cards[1]:isKindOf("ExNihilo") then
		if use.to and target then
			use.to:append(target)
		end
		use.card = sgs.Card_Parse("@HouyuanCard=" .. table.concat(usecards, "+"))
		if use.to then
			self:speak("有你这样出远门不带粮食的么？接好了！")
		end
	end
	return 
end

sgs.ai_card_intention.HouyuanCard = -70

sgs.ai_chaofeng.wis_jiangwan = 6
--[[
	技能：霸王
	描述：当你使用的【杀】被【闪】响应时，你可以和对方拼点：若你赢，可以选择最多两个目标角色，视为对其分别使用了一张【杀】
]]--
sgs.ai_skill_invoke.bawang = function(self, data)
	local effect = data:toSlashEffect()
	local max_card = self:getMaxCard()
	if max_card and max_card:getNumber() > 10 then
		return self:isEnemy(effect.to)
	end
	if self:isEnemy(effect.to) then
		if self:getOverflow() > 0 then return true end
	end
end

function sgs.ai_skill_pindian.bawang(minusecard, self, requestor, maxcard)
	local cards = sgs.QList2Table(self.player:getHandcards())
	local function compare_func(a, b)
		return a:getNumber() > b:getNumber()
	end
	table.sort(cards, compare_func)
	for _, card in ipairs(cards) do
		if card:getNumber() > 10 then return card end
	end
	self:sortByKeepValue(cards)
	return cards[1]
end

sgs.ai_skill_use["@@bawang"] = function(self, prompt)
	local first_index, second_index
	self:sort(self.enemies, "defenseSlash")
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	for i=1, #self.enemies do
		if not (self.enemies[i]:hasSkill("kongcheng") and self.enemies[i]:isKongcheng()) and not self:slashProhibit(slash ,self.enemies[i]) then
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

sgs.ai_cardneed.bawang = sgs.ai_cardneed.bignumber
sgs.ai_card_intention.BawangCard = sgs.ai_card_intention.ShensuCard
--[[
	技能：危殆（主公技）
	描述：当你需要使用一张【酒】时，所有吴势力角色按行动顺序依次选择是否打出一张黑桃2~9的手牌，视为你使用了一张【酒】，直到有一名角色或没有任何角色决定如此做时为止 
]]--

function sgs.ai_cardsview.weidai(self, class_name, player)
	if class_name == "Analeptic" and player:hasLordSkill("weidai") and not player:hasFlag("Global_WeidaiFailed") then
		return "@WeidaiCard=.->."
	end
end



sgs.ai_skill_use_func.WeidaiCard = function(card, use, self)
	use.card = card
end

sgs.ai_card_intention.WeidaiCard = sgs.ai_card_intention.Peach

sgs.ai_skill_cardask["@weidai-analeptic"] = function(self, data)
	local who = data:toPlayer()
	if self:isEnemy(who) then return "." end
	if self:needBear() and who:getHp() > 0 then return "." end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	for _, fcard in ipairs(cards) do
		if fcard:getSuit() == sgs.Card_Spade and fcard:getNumber() > 1 and fcard:getNumber() < 10 then
			return fcard:getEffectiveId()
		end
	end
	return "."
end

sgs.ai_event_callback[sgs.ChoiceMade].weidai=function(self, player, data)
	local choices= data:toString():split(":")	
	if choices[1] == "cardResponded" and choices[3] == "@weidai-analeptic" then
		local target = findPlayerByObjectName(self.room, choices[4])
		local card = choices[#choices]
		if card ~= "_nil_" then
			sgs.updateIntention(player, target, -80)
		end
	end	
end

sgs.ai_chaofeng.wis_sunce = 1
--[[
	技能：笼络
	描述：回合结束阶段开始时，你可以选择一名其他角色摸取与你弃牌阶段弃牌数量相同的牌 
]]--
sgs.ai_skill_playerchosen.longluo = function(self, targets)
	if #self.friends <= 1 then return nil end
	local n = self.player:getMark("longluo")
	local to = self:findPlayerToDraw(false, n)
	if to then return to end
	return self.friends_noself[1]
end

sgs.ai_playerchosen_intention.longluo = -60

--[[
	技能：辅佐
	描述：当有角色拼点时，你可以打出一张点数小于8的手牌，让其中一名角色的拼点牌加上这张牌点数的二分之一（向下取整）
]]--
sgs.ai_skill_use["@@fuzuo"] = function(self, prompt, method)
	if self.player:isKongcheng() then return "." end
	
	local function find_a_card(number)
		local card
		number = math.abs(number)
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(cards)		
		for _, acard in ipairs(cards) do
			local anum = acard:getNumber()
			if math.ceil(anum/2) > number and anum < 8 then
				card = acard
			end
		end
		return card
	end
	
	local pindian = self.room:getTag("FuzuoPindianData"):toPindian()
	local from, to = pindian.from, pindian.to
	local from_num, to_num = pindian.from_number, pindian.to_number
	local reason = pindian.reason
	local PDcards = {}
	table.insert(PDcards, pindian.from_card)
	table.insert(PDcards, pindian.to_card)
	
	if math.abs(from_num - to_num) >= 3 then return "." end
	
	local card = find_a_card(from_num - to_num)
	if not card then return "." end
	
	local Valuable
	for _, acard in ipairs(PDcards) do
		if acard:isKindOf("ExNihilo") or acard:isKindOf("Peach") or acard:isKindOf("Snatch") or acard:isKindOf("Dismantlement") or acard:isKindOf("Duel") then
			Valuable = true
			break
		elseif acard:isKindOf("Slash") and self:hasCrossbowEffect(from) and reason ~= "zhiba_pindian" then
			Valuable = true
		end
	end
	
	local onlyone_Jink_Peach
	if isCard("Peach",card, self.player) and self:getCardsNum("Peach") <= 1 and self.player:isWounded() then
		onlyone_Jink_Peach = true
	elseif isCard("Jink",card, self.player) and self:getCardsNum("Jink") <= 1 then
		onlyone_Jink_Peach = true
	end
	
	if reason == "zhiba_pindian" then
		if Valuable or not onlyone_Jink_Peach or self:getOverflow() > 0 and self:willSkipPlayPhase() then
			if self:isFriend(to) and from_num > to_num then			
				return "@FuzuoCard="..card:getEffectiveId().."->"..to:objectName()
			elseif not self:isFriend(to) and to_num > from_num then
				return "@FuzuoCard="..card:getEffectiveId().."->"..from:objectName()
			end
		end		
	elseif reason == "dahe" or reason == "mizhao" or reason == "shuangren" then
		if self:isFriend(from) and from_num < to_num then
			return "@FuzuoCard="..card:getEffectiveId().."->"..from:objectName()
		elseif not self:isFriend(from) and from_num > to_num then
			return "@FuzuoCard="..card:getEffectiveId().."->"..to:objectName()
		end
		
	elseif reason == "lieren" or reason == "tanlan"  or reason == "jueji" then
		if Valuable or not onlyone_Jink_Peach or self:getOverflow() > 0 and self:willSkipPlayPhase() then
			if self:isFriend(from) and not self:isFriend(to) and from_num < to_num then
				return "@FuzuoCard="..card:getEffectiveId().."->"..from:objectName() 
			elseif self:isFriend(to) and not self:isFriend(from) and to_num < from_num then
				return "@FuzuoCard="..card:getEffectiveId().."->"..to:objectName()
			end		
		end
		
	elseif reason == "tianyi" or reason == "xianzhen" then		
		if self:isFriend(from) and from_num < to_num and getCardsNum("Slash", from) >= 1 then			
			return "@FuzuoCard="..card:getEffectiveId().."->"..from:objectName()
		elseif not self:isFriend(from) and self:isFriend(to) and from_num > to_num and getCardsNum("Slash", from) >= 1 then
			return "@FuzuoCard="..card:getEffectiveId().."->"..to:objectName()
		end
		
	elseif reason == "quhu"  then
		if not self:isFriend(from) and self:isFriend(to) and from_num > to_num then
			return "@FuzuoCard="..card:getEffectiveId().."->"..to:objectName()
		elseif self:isFriend(from) and from_num >= 10 and from_num < to_num then
			return "@FuzuoCard="..card:getEffectiveId().."->"..from:objectName()
		end
		
	else
		if self:isFriend(from) and self:isFriend(to) then return "." end
		if not onlyone_Jink_Peach or self:getOverflow() > 0 and self:willSkipPlayPhase() then
			if self:isFriend(from) and from_num < to_num then
				return "@FuzuoCard="..card:getEffectiveId().."->"..from:objectName()
			elseif not self:isFriend(to) and to_num < from_num then
				return "@FuzuoCard="..card:getEffectiveId().."->"..to:objectName()
			end
		end			
	end
	
	return "."
end

--[[
	技能：尽瘁
	描述：当你死亡时，可令一名角色摸取或者弃置三张牌 
]]--

sgs.ai_skill_playerchosen.jincui = function(self, targets)
	local AssistTarget = self:AssistTarget()
	if AssistTarget and not AssistTarget:hasSkill("manjuan") and not self:needKongcheng(AssistTarget, true) then return AssistTarget end
	local wf
	for _, friend in ipairs(self.friends_noself) do
		if self:isWeak(friend) then
			wf = true 
			break 
		end
	end

	if not wf then
		self:sort(self.enemies, "handcard")
		for _, enemy in ipairs(self.enemies) do
			if enemy:getCards("he"):length() == 3 
			  and not self:doNotDiscard(enemy, "he", true, 3, true) then
				sgs.jincui_discard = true
				return enemy
			end
		end
		for _, enemy in ipairs(self.enemies) do
			if enemy:getCards("he"):length() >= 3 
			  and not self:doNotDiscard(enemy, "he", true, 3, true)
			  and self:hasSkills(sgs.cardneed_skill, enemy) then
				sgs.jincui_discard = true
				return enemy
			end
		end
	end

	local to = self:findPlayerToDraw(false, 3)
	if to then return to end
	sgs.jincui_discard = true
	return self.enemies[1]
end


sgs.ai_skill_choice.jincui = function(self, choices)
	if sgs.jincui_discard then return "throw" else return "draw" end
end


--你使用黑色的【杀】造成的伤害+1，你无法闪避红色的【杀】

sgs.ai_slash_prohibit.wenjiu = function(self, from, to, card)
	local has_black_slash, has_red_slash
	local slashes = self:getCards("Slash")
	for _, slash in ipairs(slashes) do
		if slash:isBlack() and self:slashIsEffective(slash, to) then has_black_slash = true end
		if slash:isRed() and self:slashIsEffective(slash, to) then has_red_slash = true end
	end

	if self:isFriend(to) then
		return card:isRed() and (has_black_slash or self:isWeak(to))
	else		
		if has_red_slash and getCardsNum("Jink", to) > 0 then return not card:isRed() end
	end
end

--[[
	技能：霸刀
	描述：当你成为黑色的【杀】目标时，你可以对你攻击范围内的一名其他角色使用一张【杀】 
]]--

sgs.ai_skill_cardask["@askforslash"] = function(self, data)
	local slashes = self:getCards("Slash")	
	self:sort(self.enemies, "defenseSlash")
	
	for _, slash in ipairs(slashes) do
		local no_distance = sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, self.player, slash) > 50 or self.player:hasFlag("slashNoDistanceLimit")
		for _, enemy in ipairs(self.enemies) do
			if self.player:canSlash(enemy, slash, not no_distance) and not self:slashProhibit(slash, enemy) and slash:isBlack() and self:hasSkills("wenjiu")
				and self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self)
				and not (self.player:hasFlag("slashTargetFix") and not enemy:hasFlag("SlashAssignee")) then
				return ("%s->%s"):format(slash:toString(), enemy:objectName())
			end
		end
	end
	for _, slash in ipairs(slashes) do
		local no_distance = sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, self.player, slash) > 50 or self.player:hasFlag("slashNoDistanceLimit")
		for _, enemy in ipairs(self.enemies) do
			if self.player:canSlash(enemy, slash, not no_distance) and not self:slashProhibit(slash, enemy) and not slash:isBlack()
				and self:slashIsEffective(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self)
				and not (self.player:hasFlag("slashTargetFix") and not enemy:hasFlag("SlashAssignee")) then
				return ("%s->%s"):format(slash:toString(), enemy:objectName())
			end
		end
	end
	return "."
end


sgs.ai_slash_prohibit.badao = function(self, from, to, card)
	local has_black_slash, has_red_slash
	local slashes = self:getCards("Slash")
	for _, slash in ipairs(slashes) do
		if slash:isBlack() and self:slashIsEffective(slash, to) then has_black_slash = true end
		if slash:isRed() and self:slashIsEffective(slash, to) then has_red_slash = true end
	end
	if self:isFriend(to) then 
		return card:isRed() and (has_black_slash or self:isWeak(to)) 
	else
		if has_red_slash then return card:isBlack() end
		if getCardsNum("Slash", to) > 1 then
			local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
			for _, target in ipairs(self:getEnemies(to)) do
				if to:canSlash(target, slash) and not self:slashProhibit(slash, target)
					and self:slashIsEffective(slash, target) and not self:getDamagedEffects(target, to, true) 
					and not self:needToLoseHp(target, to, true, true) 
					and self:canHit(target, to) and self:isWeak(target) then
						return card:isBlack()
				end
			end
		end
	end
end

sgs.ai_cardneed.wenjiu = function(to, card)
	return card:isBlack() and isCard("Slash", card, to)
end
--[[
	技能：识破
	描述：任意角色判定阶段判定前，你可以弃置两张牌，获得该角色判定区里的所有牌 
]]--

sgs.ai_skill_discard.shipo = function(self)
	local target = self.room:getCurrent()
	if ((target:containsTrick("supply_shortage") and target:getHp() > target:getHandcardNum()) or
		(target:containsTrick("indulgence") and target:getHandcardNum() > target:getHp()-1)) then
		if self:isFriend(target) then
			return self:askForDiscard("dummyreason", 2, 2, false, true)
		end
	end
	return {}
end

sgs.ai_choicemade_filter.skillInvoke.shipo = function(self, player, promptlist)
	if promptlist[3] == "yes" then
		local cp = self.room:getCurrent()
		sgs.updateIntention(player, cp, -10)
	end
end

sgs.ai_cardneed.gushou = function(to, card)
	return to:getHandcardNum() < 3 and card:getTypeId() == sgs.Card_TypeBasic
end

sgs.ai_chaofeng.tianfeng = -1
--[[
	技能：授业
	描述：出牌阶段，你可以弃置一张红色手牌，指定最多两名其他角色各摸一张牌 
]]--
local shouye_skill = {}
shouye_skill.name = "shouye"
table.insert(sgs.ai_skills, shouye_skill)
shouye_skill.getTurnUseCard = function(self)
	if #self.friends_noself == 0 then return end
	if self.player:getHandcardNum() > 0 then
		if self.player:getMark("jiehuo") > 0 and self.player:hasUsed("ShouyeCard") then return end
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		self:sortByKeepValue(cards)
		for _, hcard in ipairs(cards) do
			if hcard:isRed() then
				return sgs.Card_Parse("@ShouyeCard=" .. hcard:getId())
			end
		end
	end
end

sgs.ai_skill_use_func.ShouyeCard = function(card, use, self)
	self:sort(self.friends_noself, "defense")
	local first
	local second
	local AssistTarget = self:AssistTarget()
	if AssistTarget and not AssistTarget:hasSkill("manjuan") and not self:needKongcheng(AssistTarget, true) then first = AssistTarget end

	for _, friend in ipairs(self.friends_noself) do
		if not friend:hasSkill("manjuan") and not self:needKongcheng(friend, true) then
			if not first then
				first = friend
			elseif first:objectName() ~= friend:objectName() then
				second = friend
			end
			if second then break end
		end
	end
	
	if self.player:hasSkill("jiehuo") and self.player:getMark("jiehuo") < 1 then
		sgs.ai_use_priority.ShouyeCard = 9.29
		if first and not second then
			for _, friend in ipairs(self.friends_noself) do
				if first:objectName() ~= friend:objectName() then
					second = friend
				end
				if second then break end
			end
			if not second then
				for _, enemy in ipairs(self.enemies) do
					if first:objectName() ~= enemy:objectName() and (enemy:hasSkill("manjuan") or self:needKongcheng(enemy, true)) then
						second = enemy
					end
					if second then break end
				end
			end
			if not second then sgs.ai_use_priority.ShouyeCard = 0 end
		end
	else
		sgs.ai_use_priority.ShouyeCard = 0
	end
	
	if not second and self:getOverflow() <= 0 then return end
	if first then
		if use.to then
			use.to:append(first)
			self:speak("好好学习，天天向上！")
		end
	end
	if second then
		if use.to then
			use.to:append(second)
		end
	end
	use.card = card
	return
end

sgs.ai_cardneed.shouye = function(to, card)
	return to:hasSkill("jiehuo") and to:getMark("jiehuo") < 1 and to:getHandcardNum() < 3 and card:isRed()
end

sgs.ai_card_intention.ShouyeCard = function(self, card, from, tos)
	local intention = -70
	for i=1, #tos do
		local to = tos[i]
		if to:hasSkill("manjuan") or self:needKongcheng(to, true) then
			intention = 0
		end
		sgs.updateIntention(from, tos[i], intention)
	end
end

--[[
	技能：师恩
	描述：其他角色使用非延时锦囊时，可以让你摸一张牌
]]--
sgs.ai_skill_invoke.shien = function(self, data)
	local target = data:toPlayer()
	if target and target:isAlive() then 
		return self:isFriend(target)
	end
	return false
end

sgs.ai_choicemade_filter.skillInvoke.shien = function(self, player, promptlist)
	local simahui = self.room:findPlayerBySkillName("shien")
	if simahui and promptlist[3] == "yes" then
		sgs.updateIntention(player, simahui, -10)
	end
end

sgs.ai_chaofeng.wis_shuijing = 5
