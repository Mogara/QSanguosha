--[[
	技能：归心
	描述：回合结束阶段，你可以做以下二选一：
		1. 永久改变一名其他角色的势力
		2. 永久获得一项未上场或已死亡角色的主公技。(获得后即使你不是主公仍然有效) 
]]--
sgs.ai_skill_invoke.weiwudi_guixin = true

local function findPlayerForModifyKingdom(self, players) --从目标列表中选择一名用于修改势力
	if players and not players:isEmpty() then
		local lord = self.room:getLord()
		local isGood = lord and self:isFriend(lord) --自己是否为忠方

		for _, player in sgs.qlist(players) do
			if not player:isLord() then
				if sgs.evaluatePlayerRole(player) == "loyalist" and not self:hasSkills("huashen|liqian",player) then
					local sameKingdom =lord and player:getKingdom() == lord:getKingdom() 
					if isGood ~= sameKingdom then
						return player
					end
				elseif lord and lord:hasLordSkill("xueyi") and not player:isLord() and not self:hasSkills("huashen|liqian",player) then
					local isQun = player:getKingdom() == "qun"
					if isGood ~= isQun then
						return player
					end
				end
			end
		end
	end
end

local function chooseKingdomForPlayer(self, to_modify) --选择合适的势力以修改目标势力
	local lord = self.room:getLord()
	local isGood = self:isFriend(lord)
	if  sgs.evaluatePlayerRole(to_modify) == "loyalist" or sgs.evaluatePlayerRole(to_modify) == "renegade" then
		if isGood then
			return lord and lord:getKingdom()
		else
			-- find a kingdom that is different from the lord
			local kingdoms = {"qun","wei", "shu", "wu"}
			for _, kingdom in ipairs(kingdoms) do
				if lord and lord:getKingdom() ~= kingdom then
					return kingdom
				end
			end
		end
	elseif lord and lord:hasLordSkill("xueyi") and not to_modify:isLord() then
		return isGood and "qun" or "wei"
	elseif self.player:hasLordSkill("xueyi") then
		return "qun"
	end

	return "qun"
end

sgs.ai_skill_choice.weiwudi_guixin = function(self, choices)
	if choices == "wei+shu+wu+qun" then --选择势力
		local to_modify = self.room:getTag("Guixin2Modify"):toPlayer()
		return chooseKingdomForPlayer(self, to_modify)
	end

	if choices ~= "modify+obtain" then --选择主公技
		if choices:match("xueyi") and not self.room:getLieges("qun", self.player):isEmpty() then return "xueyi" end
		if choices:match("weidai") and self:isWeak() then return "weidai" end
		if choices:match("ruoyu") then return "ruoyu" end
		local choice_table = choices:split("+")
		return choice_table[math.random(1,#choice_table)]
	end

	-- two choices: modify and obtain --选择技能项
	if self.player:getRole() == "renegade" or self.player:getRole() == "lord" then
		return "obtain"
	end
	
	local lord = self.room:getLord()
	if not lord then return "obtain" end

	local skills = lord:getVisibleSkillList()
	local hasLordSkill = false
	for _, skill in sgs.qlist(skills) do
		if skill:isLordSkill() then
			hasLordSkill = true
			break
		end
	end

	if not hasLordSkill then
		return "obtain"
	end

	local players = self.room:getOtherPlayers(self.player)
	players:removeOne(lord)
	if findPlayerForModifyKingdom(self, players) then
		return "modify"
	else
		return "obtain"
	end
end

sgs.ai_skill_playerchosen.weiwudi_guixin = function(self, players) --选择修改势力的目标
	if players and not players:isEmpty() then
		local player = findPlayerForModifyKingdom(self, players)
		return player or players:first()
	end
end
--[[
	技能：称象
	描述：每当你受到1次伤害，你可打出X张牌（X小于等于3），它们的点数之和与造成伤害的牌的点数相等，你可令X名角色各恢复1点体力（若其满体力则摸2张牌）
]]--
sgs.ai_skill_use["@@ytchengxiang"]=function(self,prompt)
	local prompts=prompt:split(":")
	-- assert(prompts[1]=="@ytchengxiang-card")
	local point=tonumber(prompts[4])
	local targets=self.friends
	if not targets then return end
	local compare_func = function(a, b)
		if a:isWounded() ~= b:isWounded() then
			return a:isWounded()
		elseif a:isWounded() then
			return a:getHp() < b:getHp()
		else
			return a:getHandcardNum() < b:getHandcardNum()
		end
	end
	table.sort(targets, compare_func)
	local cards=self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
	local opt1, opt2
	for _,card in ipairs(cards) do
		if card:getNumber()==point then opt1 = "@YTChengxiangCard=" .. card:getId() .. "->" .. targets[1]:objectName() break end
	end
	for _,card1 in ipairs(cards) do
		for __,card2 in ipairs(cards) do
			if card1:getId()==card2:getId() then
			elseif card1:getNumber()+card2:getNumber()==point then
				if #targets >= 2 and targets[2]:isWounded() then
					opt2 = "@YTChengxiangCard=" .. card1:getId() .. "+" .. card2:getId() .. "->" .. targets[1]:objectName() .. "+" .. targets[2]:objectName()
					break
				elseif targets[1]:getHp()==1 or self:getUseValue(card1)+self:getUseValue(card2)<=6 then
					opt2 = "@YTChengxiangCard=" .. card1:getId() .. "+" .. card2:getId() .. "->" .. targets[1]:objectName()
					break
				end
			end
		end
		if opt2 then break end
	end
	if opt1 and opt2 then
		if self.player:getHandcardNum() > 7 then return opt2 else return opt1 end
	end
	return opt2 or opt1 or "."
end

sgs.ai_card_intention.YTChengxiangCard = sgs.ai_card_intention.QingnangCard

function sgs.ai_cardneed.ytchengxiang(to, card, self)
	return card:getNumber()<8 and self:getUseValue(card)<6 and to:hasSkill("ytchengxiang") and to:getHandcardNum() < 12
end
--[[
	技能：绝汲
	描述：出牌阶段，你可以和一名角色拼点：若你赢，你获得对方的拼点牌，并可立即再次与其拼点，如此反复，直到你没赢或不愿意继续拼点为止。每阶段限一次。 
]]--
sgs.ai_skill_invoke.jueji = function(self, data)
	local target
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasFlag("jueji_target") then
			target = player
		end
	end
	if not target then return false end
	return not self:doNotDiscard(target, "h")
end

local jueji_skill = {}
jueji_skill.name = "jueji"
table.insert(sgs.ai_skills, jueji_skill)
jueji_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("JuejiCard") and not self.player:isKongcheng() then return sgs.Card_Parse("@JuejiCard=.") end
end

sgs.ai_skill_use_func.JuejiCard = function(card, use, self)
	if self.player:isKongcheng() then return end
	local zhugeliang = self.room:findPlayerBySkillName("kongcheng")
	if zhugeliang and self:isFriend(zhugeliang) and zhugeliang:getHandcardNum() == 1 and zhugeliang:objectName() ~= self.player:objectName()
	  and self:getEnemyNumBySeat(self.player, zhugeliang) > 0 and zhugeliang:getHp() <= 2 then
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByUseValue(cards, true)
		use.card = sgs.Card_Parse("@JuejiCard=" .. cards[1]:getId())
		zhugeliang:setFlags("jueji_target")
		if use.to then use.to:append(zhugeliang) end
		return
	end
	
	self:sort(self.enemies, "defense")
	local max_card = self:getMaxCard()
	local max_point = max_card:getNumber()
	
	if (self:needKongcheng() and self.player:getHandcardNum() == 1) or not self:hasLoseHandcardEffective() then
		for _, enemy in ipairs(self.enemies) do
			if not self:doNotDiscard(enemy, "h") then
				use.card = sgs.Card_Parse("@JuejiCard=" .. max_card:getId())
				enemy:setFlags("jueji_target")
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end
		
	for _, enemy in ipairs(self.enemies) do
		if not self:doNotDiscard(enemy, "h") then
			local enemy_max_card = self:getMaxCard(enemy)
			local allknown = 0
			if self:getKnownNum(enemy) == enemy:getHandcardNum() then
				allknown = allknown + 1
			end
			if (enemy_max_card and max_point > enemy_max_card:getNumber() and allknown > 0)
				or (enemy_max_card and max_point > enemy_max_card:getNumber() and allknown < 1 and max_point > 10) 
				or (not enemy_max_card and max_point > 10) then
				use.card = sgs.Card_Parse("@JuejiCard=" .. max_card:getId())
				enemy:setFlags("jueji_target")
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	if self:getOverflow() > 0 then
		for _, enemy in ipairs(self.enemies) do
			if not self:doNotDiscard(enemy, "h", true) then
				use.card = sgs.Card_Parse("@JuejiCard=" .. cards[1]:getId())
				enemy:setFlags("jueji_target")
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end
	return
end

sgs.ai_use_priority.JuejiCard = 3.4
sgs.ai_card_intention.JuejiCard = function(self, card, from, tos)
	local intention = 10
	local to = tos[1]
	if self:needKongcheng(to) and to:getHandcardNum() == 1 then
		intention = 0
	end
	sgs.updateIntention(from, tos[1], intention)
end
sgs.ai_cardneed.jueji = sgs.ai_cardneed.bignumber
sgs.dynamic_value.control_card.JuejiCard = true

function sgs.ai_skill_pindian.jueji(minusecard, self, requestor, maxcard)
	if self:isFriend(requestor) then return end
	if maxcard:getNumber() == 13 then return maxcard end
	if (maxcard:getNumber()/13)^requestor:getHandcardNum() <= 0.6 then return minusecard end
end
--[[
	技能：围堰
	描述：你可以将你的摸牌阶段当作出牌阶段，出牌阶段当作摸牌阶段执行 
]]--
sgs.ai_skill_invoke.lukang_weiyan = function(self, data)
	local handcard = self.player:getHandcardNum()
	local max_card = self.player:getMaxCards()
	local target = 0
	local slashnum = 0
	
	for _, slash in ipairs(self:getCards("Slash")) do
		for _,enemy in ipairs(self.enemies) do
			if self.player:canSlash(enemy, slash) and self:slashIsEffective(slash, enemy) and self:slashIsEffective(slash, enemy)
			  and not self:slashProhibit(slash, enemy) and sgs.isGoodTarget(enemy, self.enemies, self) then 
				slashnum = slashnum + 1
				target = target + 1
				break
			end 
		end
	end

	local prompt = data:toString()
	if prompt == "draw2play" then
		if self:needBear() then return false end
		if slashnum > 1 and target > 0 then return true end
		if self.player:isSkipped(sgs.Player_Play) and #(self:getTurnUse()) > 0 then return true end
		return false
	elseif prompt == "play2draw" then
		if self:needBear() then return true end
		if slashnum > 0 and target > 0 then return false end
		if #(self:getTurnUse()) == 0 then return true end
		return false
	end
end

function sgs.ai_cardneed.lukang_weiyan(to, card)
	return isCard("Slash", card, to) and getKnownCard(to, "Slash", true) < 2
end
--[[
	技能：五灵
	描述：回合开始阶段，你可选择一种五灵效果发动，该效果对场上所有角色生效
		该效果直到你的下回合开始为止，你选择的五灵效果不可与上回合重复
		[风]场上所有角色受到的火焰伤害+1
		[雷]场上所有角色受到的雷电伤害+1
		[水]场上所有角色使用桃时额外回复1点体力
		[火]场上所有角色受到的伤害均视为火焰伤害
		[土]场上所有角色每次受到的属性伤害至多为1 
]]--
sgs.ai_skill_choice.wuling = function(self, choices)
	if choices:match("water") then
		local weak_friend, weak_enemy = 0, 0
		for _, player in sgs.qlist(self.room:getAlivePlayers()) do
			if self:isWeak(player) then
				if self:isEnemy(player) then 
					weak_enemy = weak_enemy + 1
					if player:isLord() then weak_enemy = weak_enemy + 1 end
				elseif self:isFriend(player) then
					weak_friend = weak_friend + 1
					if player:isLord() then weak_friend = weak_friend + 1 end
				end
			end
		end
		if weak_friend > 0 and weak_friend >= weak_enemy then return "water" end
	end
	if choices:match("earth") then
		if #(self:getChainedFriends()) > #(self:getChainedEnemies()) and
			#(self:getChainedFriends()) + #(self:getChainedEnemies()) > 1 then return "earth" end
		if self:hasWizard(self.enemies, true) and not self:hasWizard(self.friends, true) then
			for _, player in sgs.qlist(self.room:getAlivePlayers()) do
				if player:containsTrick("lightning") then return "earth" end
			end
		end
	end
	if choices:match("fire") then
		for _,enemy in ipairs(self.enemies) do
			if enemy:hasArmorEffect("Vine") then return "fire" end
		end
		if #(self:getChainedFriends()) < #(self:getChainedEnemies()) and
			#(self:getChainedFriends()) + #(self:getChainedEnemies()) > 1 then return "fire" end
	end
	if choices:match("wind") then
		for _,enemy in ipairs(self.enemies) do
			if enemy:hasArmorEffect("Vine") then return "wind" end
		end
		for _,friend in ipairs(self.friends) do
			if friend:hasSkill("huoji") then return "wind" end
		end
		if #(self:getChainedFriends()) < #(self:getChainedEnemies()) and
			#(self:getChainedFriends()) + #(self:getChainedEnemies()) > 1 then return "wind" end
		for _,friend in ipairs(self.friends) do
			if self:isEquip("Fan", friend) then return "wind" end
		end
		if self:getCardId("FireSlash") or self:getCardId("FireAttack") then return "wind" end
	end
	if choices:match("thunder") then
		if self:hasWizard(self.friends,true) and not self:hasWizard(self.enemies,true) then
			for _, player in sgs.qlist(self.room:getAlivePlayers()) do
				if player:containsTrick("lightning") then return "thunder" end
			end
			for _, friend in ipairs(self.friends) do
				if friend:hasSkill("leiji") then return "thunder" end
			end
		end
		if self:getCardId("ThunderSlash") then return "thunder" end
	end
	local choices_table = choices:split("+")
	return choices_table[math.random(1, #choices_table)]
end
--[[
	技能：连理
	描述：回合开始阶段开始时，你可以选择一名男性角色，你和其进入连理状态直到你的下回合开始：该角色可以帮你出闪，你可以帮其出杀 
]]--
sgs.ai_skill_playerchosen.lianli = function(self, targets)
	local key = math.random(0, 2) == 1 and "defenseSlash" or "defense"
	self:sort(self.friends_noself, key)
	local AssistTarget = self:AssistTarget()
	if AssistTarget and AssistTarget:isMale() and not AssistTarget:hasSkill("manjuan") then return AssistTarget end
	for _, friend in ipairs(self.friends_noself) do --优先考虑与队友连理
		if friend:isMale() and not friend:hasSkill("manjuan") then
			return friend
		end
	end
	for _, friend in ipairs(self.friends_noself) do
		if friend:isMale() then
			return friend
		end
	end
	if self.player:isMale() then return self.player end
	if sgs.turncount <= 2 then
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if player:isMale() and not self:isEnemy(player) and not player:inMyAttackRange(self.player) then
				return player
			end
		end
	end	
	return nil
end
sgs.ai_playerchosen_intention.lianli = -77

table.insert(sgs.ai_global_flags, "lianlisource")
sgs.ai_skill_invoke.lianli_slash = function(self, data) --CardAsk
	return self:getCardsNum("Slash")==0
end

sgs.ai_skill_invoke.lianli_jink = function(self, data)
	local tied
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:getMark("@tied") > 0 then tied = player break end
	end
	if self:hasEightDiagramEffect(tied) then return true end
	return self:getCardsNum("Jink") == 0
end

sgs.ai_choicemade_filter.skillInvoke["lianli-jink"] = function(player, promptlist)
	if promptlist[#promptlist] == "yes" then
		sgs.lianlisource = player
	end
end

sgs.ai_choicemade_filter.cardResponded["@lianli-jink"] = function(player, promptlist)
	if promptlist[#promptlist] ~= "_nil_" then
		-- sgs.updateIntention(player, sgs.lianlisource, -80)
		local xiahoujuan = player:getRoom():findPlayerBySkillName("lianli")
		assert(xiahoujuan)
		sgs.updateIntention(player, xiahoujuan, -80)
		sgs.lianlisource = nil
	end
end

sgs.ai_skill_cardask["@lianli-jink"] = function(self)
	local players = self.room:getOtherPlayers(self.player)
	local target
	for _, p in sgs.qlist(players) do
		if p:getMark("@tied") > 0 then target = p break end
	end
	if not self:isFriend(target) then return "." end
	return self:getCardId("Jink") or "."
end

function sgs.ai_slash_prohibit.lianli(self, to, card, from)
	if self:isFriend(to) then return false end
	if self:canLiegong(to, from) then return false end
	local players = sgs.QList2Table(self.room:getOtherPlayers(to))
	for _, player in ipairs(players) do
		if player:getMark("@tied") > 0 and self:isFriend(player, to) then
			if player:hasSkill("tiandu") and sgs.ai_slash_prohibit.tiandu(self, player) then return true end
			if player:hasLordSkill("hujia") and sgs.ai_slash_prohibit.hujia(self, player) then return true end
			if player:hasSkill("leiji") and sgs.ai_slash_prohibit.leiji(self, player) then return true end
			if player:hasSkill("weidi") and sgs.ai_slash_prohibit.weidi(self, player) then return true end
		end
	end
	return false	
end

local lianli_slash_skill={name="lianli-slash"}
table.insert(sgs.ai_skills, lianli_slash_skill)
lianli_slash_skill.getTurnUseCard = function(self) --考虑主动使用连理杀
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	if self.player:getMark("@tied")>0 and slash:isAvailable(self.player) and not self.player:hasFlag("Global_LianliFailed") then 
		return sgs.Card_Parse("@LianliSlashCard=.") 
	end
end

sgs.ai_skill_use_func.LianliSlashCard = function(card, use, self)
	if self.player:hasUsed("LianliSlashCard") and not sgs.lianlislash then return end
	--local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	--self:useBasicCard(slash, use)
	if use.card then use.card = card end
end

local lianli_slash_filter = function(player, carduse)
	if carduse.card:isKindOf("LianliSlashCard") then
		sgs.lianlislash = false
	end
end

table.insert(sgs.ai_choicemade_filter.cardUsed, lianli_slash_filter)

sgs.ai_choicemade_filter.cardResponded["@lianli-slash"] = function(player, promptlist)
	if promptlist[#promptlist] ~= "_nil_" then
		sgs.lianlislash = true
	end
end

sgs.ai_skill_cardask["@lianli-slash"] = function(self)
	local players = self.room:getOtherPlayers(self.player)
	local target
	for _, p in sgs.qlist(players) do
		if p:getMark("@tied")>0 then target = p break end
	end
	if not self:isFriend(target) then return "." end
	return self:getCardId("Slash") or "."
end

--[[
	技能：同心
	描述：处于连理状态的两名角色，每受到一点伤害，你可以令你们两人各摸一张牌 
]]--
sgs.ai_skill_invoke.tongxin = true

--[[
	技能：归汉
	描述：出牌阶段，你可以主动弃置两张相同花色的红色手牌，和你指定的一名其他存活角色互换位置。每阶段限一次 
]]--
local guihan_skill = {name = "guihan"}
table.insert(sgs.ai_skills, guihan_skill)
function guihan_skill.getTurnUseCard(self)
	if self:getOverflow() <= 0 or self.player:hasUsed("GuihanCard") then return end
	if self.room:alivePlayerCount() == 2 or self.role == "renegade" then return end
	if #self.friends_noself == 0 then return end
	local rene = 0
	for _, aplayer in sgs.qlist(self.room:getAlivePlayers()) do
		if sgs.evaluatePlayerRole(aplayer) == "renegade" then rene = rene + 1 end
	end
	if #self.friends + #self.enemies + rene < self.room:alivePlayerCount() then return end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards)
	local red_cards = {}
	for index = #cards, 1, -1 do
		if self:getUseValue(cards[index]) >= 6 then break end
		if cards[index]:isRed() then
			if #red_cards == 0 or (#red_cards == 1 and cards[index]:getSuit() == sgs.Sanguosha:getCard(red_cards[1]):getSuit()) then
				table.insert(red_cards, cards[index]:getId())
				table.remove(cards, index)
			end
			if #red_cards >=2 then break end
		end
	end
	if #red_cards == 2 then return sgs.Card_Parse("@GuihanCard=" .. table.concat(red_cards, "+")) end
end

function sgs.ai_skill_use_func.GuihanCard(card, use, self)
	local values, range = {}, self.player:getAttackRange()
	local nplayer = self.player
	for i = 1, self.player:aliveCount() do
		local fediff, add, isfriend = 0, 0
		local np = nplayer
		for value = #self.friends_noself, 1, -1 do
			np = np:getNextAlive()
			if np:objectName() == self.player:objectName() then
				if self:isFriend(nplayer) then fediff = fediff + value
				else fediff = fediff - value
				end
			else
				if self:isFriend(np) then
					fediff = fediff + value
					if isfriend then add = add + 1
					else isfriend = true end
				elseif self:isEnemy(np) then
					fediff = fediff - value
					isfriend = false
				end
			end
		end
		values[nplayer:objectName()] = fediff + add
		nplayer = nplayer:getNextAlive()
	end
	local function get_value(a)
		local ret = 0
		for _, enemy in ipairs(self.enemies) do
			if a:objectName() ~= enemy:objectName() and a:distanceTo(enemy) <= range then ret = ret + 1 end
		end
		return ret
	end
	local function compare_func(a,b)
		if values[a:objectName()] ~= values[b:objectName()] then
			return values[a:objectName()] > values[b:objectName()]
		else
			return get_value(a) > get_value(b)
		end
	end
	local players = sgs.QList2Table(self.room:getAlivePlayers())
	table.sort(players, compare_func)
	if values[players[1]:objectName()] > 0 and players[1]:objectName() ~= self.player:objectName() then
		use.card = card
		if use.to then use.to:append(players[1]) end
	end
end

sgs.ai_use_priority.GuihanCard = 8
--[[
	技能：胡笳
	描述：回合结束阶段开始时，你可以进行判定：若为红色，立即获得此牌，如此往复，直到出现黑色为止，连续发动3次后武将翻面 
]]--
sgs.ai_skill_invoke.caizhaoji_hujia = function(self, data)
	local zhangjiao = self.room:findPlayerBySkillName("guidao")
	if zhangjiao and self:isEnemy(zhangjiao) and getKnownCard(zhangjiao, "black", false, "he") > 1 then return false end
	if not self.player:faceUp() then 
		return true 
	end
	local invokeNum = self.player:getMark("caizhaoji_hujia")
	if invokeNum < 2 then
		self.room:setPlayerMark(self.player, "caizhaoji_hujia", invokeNum + 1)
		return true
	else
		return false
	end
	--[[
	if invokeNum ~= 2 then 
		self.room:setPlayerMark(self.player, "caizhaoji_hujia", invokeNum + 1) 
		return true
	else
		if self:hasSkills("hongyan|noszhenlie|jiushi|toudu|guicai|huanshi", self.player) then
			self.room:setPlayerMark(self.player, "caizhaoji_hujia", invokeNum + 1) 
			return true
		end
		for _,p in pairs(self.friends_noself) do
			if self:hasSkills("fangzhu|jilve|guicai|huanshi", p) then
				self.room:setPlayerMark(self.player, "caizhaoji_hujia", invokeNum + 1) 
				return true 
			end
		end
		return false
	end
	]]--
end

sgs.ai_event_callback[sgs.EventPhaseEnd].caizhaoji_hujia = function(self, player, data)
	if player:getPhase() == sgs.Player_Finish then
		self.room:setPlayerMark(player, "caizhaoji_hujia", 0) 
	end
end
--[[
	技能：神君
	描述：游戏开始时，你必须选择自己的性别。回合开始阶段开始时，你必须倒转性别，异性角色对你造成的非雷电属性伤害无效 
]]--
function sgs.ai_skill_choice.shenjun(self, choices)
	local gender
	if sgs.isRolePredictable() then
		local male = 0
		self:updatePlayers()
		for _, enemy in ipairs(self.enemies) do
			if enemy:getGeneral():isMale() then male = male + 1 end
		end
		gender = (male < #self.enemies - male)
	else
		local males = 0
		for _, player in sgs.qlist(self.room:getAlivePlayers()) do
			if player:isMale() then males = males + 1 end
		end
		gender = (males <= self.player:aliveCount() - males)
	end
	if self.player:getSeat() < self.room:alivePlayerCount()/2 then gender = not gender end
	if gender then return "male" else return "female" end
end
--[[
	技能：烧营
	描述：当你对一名不处于连环状态的角色造成一次火焰伤害时，你可选择一名其距离为1的另外一名角色并进行一次判定：若判定结果为红色，则你对选择的角色造成一点火焰伤害 
]]--
function sgs.ai_skill_invoke.shaoying(self, data)
	local damage = data:toDamage()
	local enemynum = 0
	for _, p in sgs.qlist(self.room:getOtherPlayers(damage.to)) do
		if damage.to:distanceTo(p) <= 1 and self:isEnemy(p) then
			enemynum = enemynum + 1
		end
	end
	if enemynum < 1 then return false end
	local zhangjiao = self.room:findPlayerBySkillName("guidao")
	if zhangjiao and self:isEnemy(zhangjiao) and getKnownCard(zhangjiao, "black", false, "he") > 1 then return false end
	return true
end

sgs.ai_skill_playerchosen.shaoying = function(self, targets)
	local tos = {}
	for _, target in sgs.qlist(targets) do
		if self:isEnemy(target) then table.insert(tos, target) end
	end 
	
	if #tos > 0 then
		tos = self:SortByAtomDamageCount(tos, self.player, sgs.DamageStruct_Fire, nil)
		return tos[1]
	end
end

sgs.ai_playerchosen_intention.shaoying = function(self, from, to)
	sgs.shaoying_target = to
	sgs.updateIntention(from, to , 10)
end

--[[
	技能：共谋
	描述：回合结束阶段开始时，可指定一名其他角色：其在摸牌阶段摸牌后，须给你X张手牌（X为你手牌数与对方手牌数的较小值），然后你须选择X张手牌交给对方 
]]--
sgs.ai_skill_invoke.gongmou = function(self)
	sgs.gongmou_target = nil
	if self.player:hasSkill("manjuan") then return false end
	self:sort(self.friends_noself, "defense")
	for _, friend in ipairs(self, friends_noself) do
		if friend:hasSkill("enyuan") then
			sgs.gongmou_target = friend
		elseif friend:hasSkill("manjuan") then
			sgs.gongmou_target = friend
			return true
		end
	end
	if sgs.gongmou_target then return true end
	
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if not self:willSkipDrawPhase(enemy) and not (self:needKongcheng(enemy) and self.player:getHandcardNum() > enemy:getHandcardNum())
		  and not self:hasSkills("manjuan|qiaobian", enemy) then
			sgs.gongmou_target = enemy
			return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen.gongmou = function(self, targets)
	if sgs.gongmou_target then return sgs.gongmou_target end
	self:sort(self.enemies, "defense")
	return self.enemies[1]
end

sgs.ai_playerchosen_intention.gongmou = function(self, from, to)
	local intention = 60
	if to:hasSkill("manjuan") then intention = -intention
	elseif to:hasSkill("enyuan") then intention = 0
	end
	sgs.updateIntention(from, to, intention)
	sgs.gongmou_target = nil
end

sgs.ai_skill_discard.gongmou = function(self, discard_num, optional, include_equip)
	local cards = sgs.QList2Table(self.player:getHandcards())
	local to_discard = {}
	local compare_func = function(a, b)
		return self:getKeepValue(a) < self:getKeepValue(b)
	end
	table.sort(cards, compare_func)
	for _, card in ipairs(cards) do
		if #to_discard >= discard_num then break end
		table.insert(to_discard, card:getId())
	end
	
	return to_discard
end
--[[
	技能：乐学
	描述：出牌阶段，可令一名有手牌的其他角色展示一张手牌，若为基本牌或非延时锦囊，则你可将与该牌同花色的牌当作该牌使用或打出直到回合结束；若为其他牌，则立刻被你获得。每阶段限一次 
]]--
sgs.ai_cardshow.lexue = function(self, requestor)
	local cards = self.player:getHandcards()
	if self:isFriend(requestor) then
		for _, card in sgs.qlist(cards) do
			if card:isKindOf("Peach") and requestor:isWounded() then
				result = card
			elseif card:isNDTrick() then
				result = card
			elseif card:isKindOf("EquipCard") then
				result = card
			elseif card:isKindOf("Slash") then
				result = card
			end
			if result then return result end
		end
	else
		for _, card in sgs.qlist(cards) do
			if card:isKindOf("Jink") then
				result = card
				return result
			end
		end
	end
	return self.player:getRandomHandCard() 
end

local lexue_skill={name="lexue"}
table.insert(sgs.ai_skills,lexue_skill)
lexue_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("LexueCard") then return sgs.Card_Parse("@LexueCard=.") end
	if self.player:hasFlag("lexue") then return sgs.Card_Parse("@LexueCard=.") end
end

sgs.ai_skill_use_func.LexueCard = function(card, use, self)
	if self.player:hasFlag("lexue") then
		local lexuesrc = sgs.Sanguosha:getCard(self.player:getMark("lexue"))
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByUseValue(cards, true)
		for _, hcard in ipairs(cards) do
			if hcard:getSuit() == lexuesrc:getSuit() then
				local lexuestr = ("%s:lexue[%s:%s]=%d"):format(lexuesrc:objectName(), hcard:getSuitString(), hcard:getNumberString(), hcard:getId())
				local lexue = sgs.Card_Parse(lexuestr)
				if self:getUseValue(lexue) > self:getUseValue(hcard) then
					if lexuesrc:isKindOf("BasicCard") then
						self:useBasicCard(lexuesrc, use)
						if use.card then use.card = lexue return end
					else
						self:useTrickCard(lexuesrc, use)
						if use.card then use.card = lexue return end
					end
				end
			end
		end
	else
		if #self.enemies > 0 then
			local target
			self:sort(self.enemies, "hp")
			enemy = self.enemies[1]
			if self:isWeak(enemy) and not enemy:isKongcheng() then
				target = enemy
			else
				self:sort(self.friends_noself, "handcard")
				target = self.friends_noself[#self.friends_noself]
				if target and target:isKongcheng() then target = nil end
			end
			if not target then
				self:sort(self.enemies,"handcard")
				if self.enemies[1] and not self.enemies[1]:isKongcheng() then target = self.enemies[1] else return end
			end
			use.card = card
			if use.to then use.to:append(target) end
		end
	end
end

sgs.ai_use_priority.LexueCard = 10
--[[
	技能：殉志
	描述：出牌阶段，你可以摸三张牌并变身为其他未上场或已阵亡的蜀势力角色，回合结束后你立即死亡 
]]--
local xunzhi_skill = {name = "xunzhi"}
table.insert(sgs.ai_skills, xunzhi_skill)
function xunzhi_skill.getTurnUseCard(self)
	if self.player:hasUsed("XunzhiCard") then return end
	if self:needBear() then return end
	if not sgs.GetConfig("EnableHegemony", false) then
		if self.role == "renegade" or self.role == "lord" then return end
	end
	if (#self.friends > 1) or (#self.enemies == 1 and sgs.turncount > 1) then
		if self:getAllPeachNum() == 0 and self.player:getHp() == 1 then
			return sgs.Card_Parse("@XunzhiCard=.")
		end
		if self:isWeak() and self.role == "rebel" and self.player:inMyAttackRange(self.room:getLord()) and self:isEquip("Crossbow") then
			return sgs.Card_Parse("@XunzhiCard=.")
		end
	end
end

function sgs.ai_skill_use_func.XunzhiCard(card, use)
	use.card = card
end

function sgs.ai_skill_choice.xunzhi(self, choices)
	local xunzhi = choices:split("+")
	xunzhi:removeOne("weiyan")
	xunzhi:removeOne("zhugeliang")
	xunzhi:removeOne("pangtong")
	xunzhi:removeOne("menghuo")
	xunzhi:removeOne("liushan")
	xunzhi:removeOne("fazheng")
	xunzhi:removeOne("xushu")
	xunzhi:removeOne("liaohua")
	xunzhi:removeOne("madai")
	xunzhi:removeOne("jianyong")
	xunzhi:removeOne("guanping")
	xunzhi:removeOne("liufeng")
	xunzhi:removeOne("xiahoushi")
	xunzhi:removeOne("bgm_liubei")
	xunzhi:removeOne("ganfuren")
	xunzhi:removeOne("heg_jiangwei")
	xunzhi:removeOne("jiangwanfeiyi")
	xunzhi:removeOne("neo2013_masu")
	xunzhi:removeOne("neo2013_ganfuren")
	xunzhi:removeOne("neo2013_guanping")
	xunzhi:removeOne("nos_guanxingzhangbao")
	xunzhi:removeOne("wis_jiangwei")
	xunzhi:removeOne("shanfu")
	xunzhi:removeOne("zhoucang")
	
	
	--只是举个例子，不详细写了
	return xunzhi[1];
	
	
	--[[

+        QStringList xunzhizhangfei;
+        foreach(QString s, xunzhi)
+            if (s.contains("zhangfei"))
+                xunzhizhangfei << s;
+        
+        xunzhizhangfei.removeOne("bgm_zhangfei");
+        if (xunzhizhangfei.length() > 1)
+            xunzhizhangfei.removeOne("zhangfei");
+        if (xunzhi.contains("xiahouba") && player->getHp() <= 2)
+            xunzhizhangfei << "xiahouba";
+        
+        if (xunzhizhangfei.length() > 0){
+            int slashcount = 0;
+            foreach(const Card *c, player->getHandcards())
+                if (c->isKindOf("Slash"))
+                    slashcount ++;
+
+            if (slashcount > 2)
+                return xunzhizhangfei.at(qrand() % xunzhizhangfei.length());
+        }
+
+        QStringList xunzhihuangyueying;
+        foreach(QString s, xunzhi)
+            if (s.contains("huangyueying"))
+                xunzhihuangyueying << s;
+        if (xunzhihuangyueying.length() > 1)
+            xunzhihuangyueying.removeOne("huangyueying");
+
+        if (xunzhihuangyueying.length() > 0){
+            int trickcount = 0;
+            foreach(const Card *c, player->getHandcards())
+                if (c->isNDTrick())
+                    trickcount ++;
+
+            if (trickcount > 2)
+                return xunzhihuangyueying.first();
+        }
+
+        bool hascrossbow = false;
+        QList<const Card *> cards = player->getHandcards();
+        cards << player->getWeapon();
+        foreach(const Card *c, cards)
+            if (c->isKindOf("Crossbow") || c->isKindOf("VSCrossbow")){
+                hascrossbow = true;
+                break;
+            }
+        
+        if (hascrossbow){
+
+            QStringList xunzhiguanyu;
+            foreach(QString s, xunzhi)
+                if (s.contains("guanyu"))
+                    xunzhiguanyu << s;
+
+            if (xunzhiguanyu.length() > 1)
+                xunzhiguanyu.removeOne("guanyu");
+
+            if (xunzhiguanyu.length() > 0){
+                int redcount = 0;
+                foreach(const Card *c, player->getCards("he"))
+                    if (c->isRed() && (!c->isKindOf("Crossbow") || c->isKindOf("VSCrossbow")))
+                        redcount ++;
+
+                if (redcount > 1)
+                    return xunzhiguanyu.first();
+            }
+
+            QStringList xunzhizhaoyun;
+            foreach(QString s, xunzhi)
+                if (s.contains("zhaoyun"))
+                    xunzhizhaoyun << s;
+
+            if (xunzhizhaoyun.length() > 1)
+                xunzhizhaoyun.removeOne("zhaoyun");
+
+            if (xunzhizhaoyun.length() > 0){
+                int jinkcount = 0;
+                foreach(const Card *c, player->getHandcards())
+                    if (c->isKindOf("Jink"))
+                        jinkcount ++;
+
+                if (jinkcount > 1)
+                    return xunzhizhaoyun.first();
+            }
+        }
+
+        if (xunzhi.contains("huangzhong")){
+            bool haskylin = false;
+            bool hasslash = false;
+            QList<const Card *> cards = player->getHandcards();
+            cards << player->getWeapon();
+            foreach(const Card *c, cards)
+                if (c->isKindOf("Weapon") && ((const Weapon *)c)->getRange() >= 3)
+                    haskylin = true;
+                else if (c->isKindOf("Slash"))
+                    hasslash = true;
+
+            if (haskylin && hasslash)
+                return "huangzhong";
+
+        }
+
+        QStringList xunzhiothers;
+        if (xunzhi.contains("machao"))
+            xunzhiothers << "machao";
+        if (xunzhi.contains("nos_madai"))
+            xunzhiothers << "nos_madai";
+
+        if (xunzhiothers.length() > 0){
+            ServerPlayer *lastretrial;
+            foreach(ServerPlayer *s, getAlivePlayers())
+                if (s->hasSkills("guicai|guidao|huanshi|jilve"))
+                    lastretrial = s;
+
+            if (player->getAI()->GetRelation(player, lastretrial) == AI::Friend)
+                return xunzhiothers.last();
+        }
+
+        if (xunzhi.contains("wolong")){
+            int redcount = 0;
+            foreach(const Card *c, player->getHandcards())
+                if (c->isRed())
+                    redcount ++;
+
+            if (redcount > 2)
+                return "wolong";
+        }
+
+        if (xunzhi.contains("bgm_zhangfei") || xunzhi.contains("zhurong")){
+            int maxcard = 0;
+            bool red_slash = false;
+            bool slash = false;
+            foreach(const Card *c, player->getHandcards()){
+                if (c->getNumber() > maxcard)
+                    maxcard = c->getNumber();
+                if (c->isKindOf("Slash")){
+                    slash = true;
+                    if (c->isRed())
+                        red_slash = true;
+                }
+            }
+            if (maxcard >= 11)
+                if (red_slash && xunzhi.contains("bgm_zhangfei"))
+                    return "bgm_zhangfei";
+                else
+                    return "zhurong";
+        }
+
+        return xunzhi.at(qrand() % xunzhi.length());
+ 
	]]
end

--[[
	技能：毒士
	描述：杀死你的角色获得崩坏技能直到游戏结束 
]]--
function sgs.ai_slash_prohibit.dushi(self, to, card, from)	
	if from:hasSkill("jueqing") then return false end
	if from:hasFlag("nosjiefanUsed") then return false end
	return from:isLord() and #self.enemies > 1
end
--[[
	技能：争功
	描述：其他角色的回合开始前，若你的武将牌正面向上，你可以将你的武将牌翻面并立即进入你的回合，你的回合结束后，进入该角色的回合 
]]--
sgs.ai_skill_invoke.zhenggong = function(self, data)
	if sgs.turncount <= 1 and #self.enemies == 0 then return false end
	return true
end

--[[
	技能：偷渡
	描述：当你的武将牌背面向上时若受到伤害，你可以弃置一张手牌并将你的武将牌翻面，视为对一名其他角色使用了一张【杀】
]]--

sgs.ai_skill_use["@@toudu"] = function(self, prompt)
	local toudu_target = nil
	local targets = sgs.SPlayerList()
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self.player:canSlash(p, nil, false) then targets:append(p) end
	end
	if targets:length() == 0 then return "." end
	toudu_target = sgs.ai_skill_playerchosen.zero_card_as_slash(self, targets)
	if not toudu_target then return "." end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	local card_id = -1
	for _, card in ipairs(cards) do
		if not (isCard("Peach", card, self.player) and self:isFriend(toudu_target)) then
			card_id = card:getEffectiveId()
		end
	end
	if card_id == -1 then return "." end
	--self.player:speak(sgs.Sanguosha:getCard(card_id):getClassName() .. ":toudu:" .. toudu_target:getGeneralName())
	return "@TouduCard=" .. tostring(card_id) .. "->" .. toudu_target:objectName() --有时会把桃拿出来偷渡，不知什么原因
end
--[[
sgs.ai_skill_cardask['@toudu'] = function(self, data, pattern, target, target2)
	self.toudu_target = nil
	local targets = sgs.SPlayerList()
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self.player:canSlash(p, nil, false) then targets:append(p) end
	end
	if targets:length() == 0 then return "." end
	self.toudu_target = sgs.ai_skill_playerchosen.zero_card_as_slash(self, targets)
	if not self.toudu_target then return "." end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, card in ipairs(cards) do
		if not (isCard("Peach", card, self.player) and self:isFriend(self.toudu_target)) then return card:getEffectiveId() end
	end
	return "."
end

sgs.ai_skill_playerchosen.toudu = function(self, targets)
	if self.toudu_target then return self.toudu_target end
	
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
]]
sgs.ai_need_damaged.toudu = function(self, attacker, player)
	if not player:hasSkill("toudu") or player:faceUp() then return false end
	local peaches = getCardsNum("Peach", player)
	if peaches >= player:getLostHp() and peaches > 0 then return true end
	if self.player:objectName() == player:objectName() and player:getHp() > 1 then
		local slash = sgs.Sanguosha:cloneCard("Slash", sgs.Card_NoSuit, 0)
		for _, target in ipairs(self.enemies) do
			if self:isEnemy(target) and self:slashIsEffective(slash, target) and not self:getDamagedEffects(target, self.player, true)
				and getCardsNum("Jink", target) < 1 and (target:getHp() == 1 or self:hasHeavySlashDamage(player, nil, tareget) and target:getHp() == 2) then
				return true
			end
		end
	end
	return false
end
	
--[[
	技能：义舍
	描述：出牌阶段，你可将任意数量手牌正面朝上移出游戏称为“米”（至多存在五张）或收回；其他角色在其出牌阶段可选择一张“米”询问你，若你同意，该角色获得这张牌，每阶段限两次 
]]--
local yishe_skill = {name = "yishe"}
table.insert(sgs.ai_skills, yishe_skill)
yishe_skill.getTurnUseCard = function(self)
	if self:needBear() then return end
	return sgs.Card_Parse("@YisheCard=.")
end

sgs.ai_skill_use_func.YisheCard = function(card, use, self)
	sgs.ai_use_priority.YisheCard = 10
	if self.player:getPile("rice"):isEmpty() then
		sgs.ai_use_priority.YisheCard = 0
		local n = self.player:getHandcardNum()
		if n < 1 then return end
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		local usecards = {}
		local getOverflow = math.max(self:getOverflow(), 0)
		local discards = self:askForDiscard("dummyreason", math.min(getOverflow, 5), math.min(getOverflow, 5))
		if self:needKongcheng() and n < 6 then
			for _, card in ipairs(cards) do
				table.insert(usecards, card:getId())
			end
		else
			for _, card in ipairs(discards) do
				table.insert(usecards, card)
			end
		end
		if #usecards > 0 then
			use.card = sgs.Card_Parse("@YisheCard=" .. table.concat(usecards, "+"))
		end
	else
		if not self.player:hasUsed("YisheCard") then use.card = card return end
	end
end


sgs.ai_skill_choice.yisheask = function(self,choices)
	if self:isFriend(self.room:getCurrent()) then return "allow" else return "disallow" end
end

local yisheask_skill = {name = "yisheask"}
table.insert(sgs.ai_skills, yisheask_skill)
yisheask_skill.getTurnUseCard = function(self)
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasSkill("yishe") and not player:getPile("rice"):isEmpty() then return sgs.Card_Parse("@YisheAskCard=.") end
	end
end

sgs.ai_skill_use_func.YisheAskCard = function(card, use, self)
	if self.player:usedTimes("YisheAskCard")>1 then return end
	local zhanglu
	local cards
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasSkill("yishe") and not player:getPile("rice"):isEmpty() then zhanglu=player cards=player:getPile("rice") break end
	end	
	if not zhanglu or self:isEnemy(zhanglu) then return end
	cards = sgs.QList2Table(cards)
	for _, pcard in ipairs(cards) do
		use.card = card
		return
	end
end

sgs.ai_event_callback[sgs.ChoiceMade].yisheask = function(self, player, data)
	local datastr = data:toString()
	if datastr == "skillChoice:yisheask:allow" then
		sgs.updateIntention(self.player, self.room:getCurrent(), -70)
	end
end

--[[
	技能：惜粮
	描述：你可将其他角色弃牌阶段弃置的红牌收为“米”或加入手牌。 
]]--
sgs.ai_skill_invoke.xiliang = true

sgs.ai_skill_choice.xiliang = function(self, choices)	
	if self.player:hasSkill("manjuan") or self:needKongcheng(self.player) then return "put" end	
	if not self.player:hasSkill("yishe") then return "obtain" end
	if self:willSkipPlayPhase() and self.player:getHandcardNum() > 2 then return "put" end
	if self.player:getHandcardNum() < 3 or self:getCardsNum("Jink") < 1 then return "obtain" end
	if self:getOverflow() >= 0 then return "put" end
	return "obtain"
end

sgs.ai_chaofeng.zhanggongqi = 4
sgs.ai_use_priority.YisheAskCard = 9.1
--[[
	技能：镇威
	描述：你的【杀】被手牌中的【闪】抵消时，可立即获得该【闪】。 
]]--
sgs.ai_skill_invoke.ytzhenwei = function(self, data)
	return not self:needKongcheng(self.player) 
end
--[[
	技能：倚天
	描述：当你对曹操造成伤害时，可令该伤害-1 
]]--
sgs.ai_skill_invoke.yitian = function(self, data)
	local damage = data:toDamage()
	return self:isFriend(damage.to)
end
--[[
	技能：抬榇
	描述：出牌阶段，你可以自减1点体力或弃置一张武器牌，弃置你攻击范围内的一名角色区域的两张牌。每回合中，你可以多次使用抬榇 
]]--
local taichen_skill = {}
taichen_skill.name = "taichen"
table.insert(sgs.ai_skills, taichen_skill)
taichen_skill.getTurnUseCard = function(self)
	return sgs.Card_Parse("@TaichenCard=.")
end

sgs.ai_skill_use_func.TaichenCard = function(card, use, self)
	local target, card_str	
	local targets, friends, enemies = {}, {}, {}
	local weapon = self.player:getWeapon()	
	
	local hcards = self.player:getHandcards()
	local hand_weapon
	for _, hcard in sgs.qlist(hcards) do
		if hcard:isKindOf("Weapon") then 
			hand_weapon = true
			card_str = "@TaichenCard=" .. hcard:getId() 
		end
	end
	if hand_weapon or self.player:getHp() > 3 then
		if not card_str then card_str = "@TaichenCard=." end
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if self.player:canSlash(player) then 
				table.insert(targets, player)				
				
				if self:isFriend(player) then
					table.insert(friends, player)
				elseif self:isEnemy(player) and not self:doNotDiscard(player, "he", nil, 2) then 
					table.insert(enemies, player)
				end
			end
		end
	else
		if weapon then card_str = "@TaichenCard=" .. weapon:getId() end
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if self.player:distanceTo(player) <= 1 then 
				table.insert(targets, player)
				
				if self:isFriend(player) then
					table.insert(friends, player)
				elseif self:isEnemy(player) and not self:doNotDiscard(player, "he", nil, 2) then 
					table.insert(enemies, player)
				end
			end
		end
	end
	
	if #targets == 0 then return end
	for _, player in ipairs(targets) do
		if not player:containsTrick("YanxiaoCard") and player:containsTrick("lightning") and self:getFinalRetrial(player) == 2 then 
			target = player
			break
		end
	end
	if not target and #friends ~= 0 then
		for _, friend in ipairs(friends) do					
			if not friend:containsTrick("YanxiaoCard") and not (friend:hasSkill("qiaobian") and not friend:isKongcheng())
			  and (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then 
				target = friend 
				break 
			end
			if friend:getCards("e"):length() > 1 and self:hasSkills(sgs.lose_equip_skill, friend) then 
				target = friend 
				break 
			end
		end
	end	
	if not target and #enemies > 0 then
		self:sort(enemies, "defense")
		for _, enemy in ipairs(enemies) do
			if enemy:containsTrick("YanxiaoCard") and (enemy:containsTrick("indulgence") or enemy:containsTrick("supply_shortage")) then
				target = enemy 
				break
			end
			if self:getDangerousCard(enemy) then 
				target = enemy 
				break
			end
			if not (enemy:hasSkill("tuntian") and enemy:hasSkill("zaoxian")) then
				target = enemy 
				break
			end
		end
	end
	
	if not target then return end		
	if not card_str then
		if self:isFriend(target) and self.player:getHp() > 2 then card_str = "@TaichenCard=." end
	end
	
	if card_str then
		if use.to then
			if self:isFriend(target) then
				target:setFlags("TaichenOK")
			end
			use.to:append(target)
		end
		use.card = sgs.Card_Parse(card_str)
	end
end

sgs.ai_cardneed.taichen = sgs.ai_cardneed.weapon
sgs.taichen_keep_value = sgs.qiangxi_keep_value
sgs.ai_use_priority.TaichenCard = 3.6
sgs.ai_card_intention.TaichenCard = function(self,card, from, tos)
	if #tos > 0 then
		for _,to in ipairs(tos) do
			if to:hasFlag("TaichenOK") then
				to:setFlags("-TaichenOK")
				sgs.updateIntention(from, to, -30)
			else
				sgs.updateIntention(from, to, 30)
			end
		end
	end
	return 0
end
