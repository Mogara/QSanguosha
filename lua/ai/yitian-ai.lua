sgs.ai_skill_invoke.yitian_lost = function(self, data)
	if next(self.enemies) then
		return true
	else
		return false
	end
end

sgs.ai_skill_playerchosen.yitian_lost = sgs.ai_skill_playerchosen.damage

sgs.ai_skill_invoke.yitian_sword = function(self, targets)
	local slash=self:getCard("Slash")
	if not slash then return false end
	dummy_use={isDummy=true}
	self:useBasicCard(slash,dummy_use)
	if dummy_use.card then return true else return false end
end

sgs.weapon_range.YitianSword = 2

sgs.ai_skill_invoke.guixin2 = true

local function findPlayerForModifyKingdom(self, players)
	local lord = self.room:getLord()
	local isGood = self:isFriend(lord)

	for _, player in sgs.qlist(players) do
		if  sgs.evaluateRoleTrends(player) == "loyalist" then
			local sameKingdom = player:getKingdom() == lord:getKingdom()
			if isGood ~= sameKingdom then
				return player
			end
		elseif lord:hasLordSkill("xueyi") and not player:isLord() then
			local isQun = player:getKingdom() == "qun"
			if isGood ~= isQun then
				return player
			end
		end
	end
end

local function chooseKingdomForPlayer(self, to_modify)
	local lord = self.room:getLord()
	local isGood = self:isFriend(lord)
	if  sgs.evaluateRoleTrends(to_modify) == "loyalist" or sgs.evaluateRoleTrends(to_modify) == "renegade" then
		if isGood then
			return lord:getKingdom()
		else
			-- find a kingdom that is different from the lord
			local kingdoms = {"wei", "shu", "wu", "qun"}
			for _, kingdom in ipairs(kingdoms) do
				if lord:getKingdom() ~= kingdom then
					return kingdom
				end
			end
		end
	elseif lord:hasLordSkill("xueyi") and not to_modify:isLord() then
		return isGood and "qun" or "wei"
	elseif self.player:hasLordSkill("xueyi") then
		return "qun"
	end

	return "wei"
end

sgs.ai_skill_choice.guixin2 = function(self, choices)
	if choices == "wei+shu+wu+qun" then
		local to_modify = self.room:getTag("Guixin2Modify"):toPlayer()
		return chooseKingdomForPlayer(self, to_modify)
	end

	if choices ~= "modify+obtain" then
		if choices:match("xueyi") and not self.room:getLieges("qun", self.player):isEmpty() then return "xueyi" end
		if choices:match("weidai") and self:isWeak() then return "weidai" end
		if choices:match("ruoyu") then return "ruoyu" end
		local choice_table = choices:split("+")
		return choice_table[math.random(1,#choice_table)]
	end

	-- two choices: modify and obtain
	if self.player:getRole() == "renegade" or self.player:getRole() == "lord" then
		return "obtain"
	end
	
	local lord = self.room:getLord()
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

sgs.ai_skill_playerchosen.guixin2 = function(self, players)
	local player = findPlayerForModifyKingdom(self, players)
	return player or players:first()
end

sgs.ai_skill_use["@@chengxiang"]=function(self,prompt)
	local prompts=prompt:split(":")
	assert(prompts[1]=="@chengxiang-card")
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
		if card:getNumber()==point then opt1 = "@ChengxiangCard=" .. card:getId() .. "->" .. targets[1]:objectName() break end
	end
	for _,card1 in ipairs(cards) do
		for __,card2 in ipairs(cards) do
			if card1:getId()==card2:getId() then
			elseif card1:getNumber()+card2:getNumber()==point then
				if #targets >= 2 and targets[2]:isWounded() then
					opt2 = "@ChengxiangCard=" .. card1:getId() .. "+" .. card2:getId() .. "->" .. targets[1]:objectName() .. "+" .. targets[2]:objectName()
					break
				elseif targets[1]:getHp()==1 or self:getUseValue(card1)+self:getUseValue(card2)<=6 then
					opt2 = "@ChengxiangCard=" .. card1:getId() .. "+" .. card2:getId() .. "->" .. targets[1]:objectName()
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

sgs.ai_card_intention.ChengxiangCard = sgs.ai_card_intention.QingnangCard

function sgs.ai_cardneed.chengxiang(to, card, self)
	return card:getNumber()<8 and self:getUseValue(card)<6 and to:hasSkill("chengxiang") and to:getHandcardNum() < 12
end

sgs.ai_skill_invoke.jueji = true

sgs.ai_skill_use["@@jueji"]=function(self,prompt)
	local target
	local handcard
	if not self.enemies then return end
	for _, enemy in ipairs(self.enemies) do
		if not target or (enemy:getHandcardNum()<handcard and enemy:getHandcardNum()>0 and not (enemy:getHandcardNum()==1 and self:hasSkills(sgs.need_kongcheng,enemy))) then
			target=enemy
			handcard=enemy:getHandcardNum()
		end
	end
	if not target or handcard==0 or self.player:getHandcardNum()==0 then return "." end
	local max_point=self:getMaxCard():getNumber()
	local poss=((max_point-1)/13)^handcard
	if math.random()>poss then return "." end
	local cards=self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	local top_value=0
	for _, hcard in ipairs(cards) do
		if not hcard:inherits("Jink") then
			if self:getUseValue(hcard) > top_value then	top_value = self:getUseValue(hcard) end
		end
	end
	if top_value >= 4.7 then return "." else return "@JuejiCard=" .. self:getMaxCard():getEffectiveId() .. "->" .. target:objectName() end
end

sgs.ai_card_intention.JuejiCard = 30

function sgs.ai_skill_pindian.jueji(minusecard, self, requestor, maxcard)
	if self:isFriend(requestor) then return end
	if (maxcard:getNumber()/13)^requestor:getHandcardNum() <= 0.6 then return minusecard end
end

sgs.ai_skill_invoke.lukang_weiyan = function(self, data)
	local handcard = self.player:getHandcardNum()
	local max_card = self.player:getMaxCards()

	local prompt = data:toString()
	if prompt == "draw2play" then
		return handcard >= max_card and #(self:getTurnUse())>0
	elseif prompt == "play2draw" then
		return handcard < max_card or #(self:getTurnUse()) == 0
	end
end

sgs.ai_skill_choice.wuling = function(self, choices)
	if choices:match("water") then
		self:sort(self.friends, "hp")
		if self:isWeak(self.friends[1]) then return "water" end
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
			if self:isEquip("GaleShell", enemy) or self:isEquip("Vine", enemy) then return "fire" end
		end
		if #(self:getChainedFriends()) < #(self:getChainedEnemies()) and
			#(self:getChainedFriends()) + #(self:getChainedEnemies()) > 1 then return "fire" end
	end
	if choices:match("wind") then
		for _,enemy in ipairs(self.enemies) do
			if self:isEquip("GaleShell", enemy) or self:isEquip("Vine", enemy) then return "wind" end
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

sgs.ai_skill_use["@lianli"] = function(self, prompt)
	self:sort(self.friends)
	
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() then
			return "@LianliCard=.->" .. friend:objectName()
		end
	end
	
	return "."	
end

sgs.ai_card_intention.LianliCard = -80

table.insert(sgs.ai_global_flags, "lianlisource")
sgs.ai_skill_invoke.lianli_slash = function(self, data)
	return self:getCardsNum("Slash")==0
end

sgs.ai_skill_invoke.lianli_jink = function(self, data)
	local tied
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:getMark("@tied")>0 then tied = player break end
	end
	if self:isEquip("EightDiagram", tied) then return true end
	return self:getCardsNum("Jink")==0
end

sgs.ai_choicemade_filter.skillInvoke["lianli-jink"] = function(player, promptlist)
	if promptlist[#promptlist] == "yes" then
		sgs.lianlisource = player
	end
end

sgs.ai_choicemade_filter.cardResponsed["@lianli-jink"] = function(player, promptlist)
	if promptlist[#promptlist] ~= "_nil_" then
		sgs.updateIntention(player, sgs.lianlisource, -80)
		sgs.lianlisource = nil
	end
end

sgs.ai_skill_cardask["@lianli-jink"] = function(self)
	local players = self.room:getOtherPlayers(self.player)
	local target
	for _, p in sgs.qlist(players) do
		if p:getMark("@tied")>0 then target = p break end
	end
	if not self:isFriend(target) then return "." end
	return self:getCardId("Jink") or "."
end

local lianli_slash_skill={name="lianli-slash"}
table.insert(sgs.ai_skills, lianli_slash_skill)
lianli_slash_skill.getTurnUseCard = function(self)
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	if self.player:getMark("@tied")>0 and slash:isAvailable(self.player) then return sgs.Card_Parse("@LianliSlashCard=.") end
end

sgs.ai_skill_use_func.LianliSlashCard = function(card, use, self)
	if self.player:hasUsed("LianliSlashCard") and not sgs.lianlislash then return end
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	self:useBasicCard(slash, use)
	if use.card then use.card = card end
end

local lianli_slash_filter = function(player, carduse)
	if carduse.card:inherits("LianliSlashCard") then
		sgs.lianlislash = false
	end
end

table.insert(sgs.ai_choicemade_filter.cardUsed, lianli_slash_filter)

sgs.ai_choicemade_filter.cardResponsed["@lianli-slash"] = function(player, promptlist)
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

sgs.ai_skill_invoke.tongxin = true

local qiaocai_skill = {name = "qiaocai"}
table.insert(sgs.ai_skills, qiaocai_skill)
function qiaocai_skill.getTurnUseCard(self)
	if self.player:getMark("@tied")>0 or self.player:hasUsed("QiaocaiCard") then return end
	return sgs.Card_Parse("@QiaocaiCard=.")
end

function sgs.ai_skill_use_func.QiaocaiCard(card, use, self)
	local function compare_func(a,b)
		return a:getCards("j"):length() > b:getCards("j"):length()
	end
	table.sort(self.friends, compare_func)
	if not self.friends[1]:getCards("j"):isEmpty() then
		use.card = card
		if use.to then use.to:append(self.friends[1]) end
	end
end

sgs.ai_card_intention.QiaocaiCard = -70

local guihan_skill = {name = "guihan"}
table.insert(sgs.ai_skills, guihan_skill)
function guihan_skill.getTurnUseCard(self)
	if self:getOverflow() == 0 or self.player:hasUsed("GuihanCard") then return end
	if self.room:alivePlayerCount() == 2 or self.role == "renegade" then return end
	local rene = 0
	for _, aplayer in sgs.qlist(self.room:getAlivePlayers()) do
		if sgs.evaluateRoleTrends(aplayer) == "renegade" then rene = rene + 1 end
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
	local values, range, fediff = {}, self.player:getAttackRange(), 0
	local nplayer = self.player
	while nplayer:getNextAlive():objectName() ~= self.player:objectName() do
		nplayer = nplayer:getNextAlive()
		if self:isFriend(nplayer) then fediff = fediff - 1
		elseif self:isEnemy(nplayer) then fediff = fediff + 1 end
		if self:isFriend(nplayer:getNextAlive()) then values[nplayer:objectName()] = fediff else values[nplayer:objectName()] = -1 end
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
	local players = sgs.QList2Table(self.room:getOtherPlayers(self.player))
	table.sort(players, compare_func)
	if values[players[1]:objectName()] > 0 then
		use.card = card
		if use.to then use.to:append(players[1]) end
	end
end

sgs.ai_use_priority.GuihanCard = 8

sgs.ai_skill_invoke.caizhaoji_hujia = function(self, data)
	local zhangjiao = self.room:findPlayerBySkillName("guidao")
	if zhangjiao and self:isEnemy(zhangjiao) and zhangjiao:getCards("he"):length()>2 then return false end
	return true
end

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
		gender = (sgs.Sanguosha:getSkill("shenjun"):getDefaultChoice(self.player) == "male")	
	end
	if self.player:getSeat() < self.room:alivePlayerCount()/2 then gender = not gender end
	if gender then return "male" else return "female" end
end

function sgs.ai_skill_invoke.shaoying(self, data)
	local damage = data:toDamage()
	if not self:isEnemy(damage.to:getNextAlive()) then return false end
	local zhangjiao = self.room:findPlayerBySkillName("guidao")
	if not zhangjiao or self:isFriend(zhangjiao) then return true end
	if not zhangjiao:getCards("e"):isEmpty() then
		for _, card in sgs.qlist(zhangjiao:getCards("e")) do
			if card:isBlack() then return false end
		end
	end
	return zhangjiao:getHandcardNum() <= 1
end

sgs.ai_skill_invoke.gongmou = true

sgs.ai_skill_playerchosen.gongmou = function(self,choices)
	self:sort(self.enemies,"defense")
	return self.enemies[1]
end

sgs.ai_skill_discard.gongmou = function(self, discard_num, optional, include_equip)
	local cards = sgs.QList2Table(self.player:getHandcards())
	local to_discard = {}
	local compare_func = function(a, b)
		if a:inherits("Shit") ~= b:inherits("Shit") then return a:inherits("Shit") end
		return self:getKeepValue(a) < self:getKeepValue(b)
	end
	table.sort(cards, compare_func)
	for _, card in ipairs(cards) do
		if #to_discard >= discard_num then break end
		table.insert(to_discard, card:getId())
	end
	
	return to_discard
end

sgs.ai_cardshow.lexue = function(self, requestor)
	local cards = self.player:getHandcards()
	if self:isFriend(requestor) then
		for _, card in sgs.qlist(cards) do
			if card:inherits("Peach") and requestor:isWounded() then
				result = card
			elseif card:isNDTrick() then
				result = card
			elseif card:inherits("EquipCard") then
				result = card
			elseif card:inherits("Slash") then
				result = card
			end
			if result then return result end
		end
	else
		for _, card in sgs.qlist(cards) do
			if card:inherits("Jink") or card:inherits("Shit") then
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
					if lexuesrc:inherits("BasicCard") then
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
			if not self.enemies[1]:isKongcheng() then target = self.enemies[1] else return end
		end
		use.card = card
		if use.to then use.to:append(target) end
	end
end

sgs.ai_use_priority.LexueCard = 10

local xunzhi_skill = {name = "xunzhi"}
table.insert(sgs.ai_skills, xunzhi_skill)
function xunzhi_skill.getTurnUseCard(self)
	if self.player:hasUsed("XunzhiCard") then return end
	if (#self.friends > 1 and self.role ~= "renegade") or (#self.enemies == 1 and sgs.turncount > 1) then
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

function sgs.ai_slash_prohibit.dushi(self, to)
	if self:isFriend(to) and self:isWeak(to) then return true end
	return self.player:isLord() and self:isWeak(enemy)
end

sgs.ai_skill_invoke.zhenggong  = true

sgs.ai_skill_invoke.toudu = function(self, data)
	return #self.enemies>0
end

sgs.ai_skill_playerchosen.toudu = sgs.ai_skill_playerchosen.zero_card_as_slash

local yishe_skill={name="yishe"}
table.insert(sgs.ai_skills,yishe_skill)
yishe_skill.getTurnUseCard = function(self)
	return sgs.Card_Parse("@YisheCard=.")
end

sgs.ai_skill_use_func.YisheCard=function(card,use,self)
	if self.player:getPile("rice"):isEmpty() then
		local cards=self.player:getHandcards()
		cards=sgs.QList2Table(cards)
		local usecards={}
		for _,card in ipairs(cards) do
			if card:inherits("Shit") then table.insert(usecards,card:getId()) end
		end
		local discards = self:askForDiscard("gamerule", math.min(self:getOverflow(),5-#usecards))
		for _,card in ipairs(discards) do
			table.insert(usecards,card)
		end
		if #usecards>0 then
			use.card=sgs.Card_Parse("@YisheCard=" .. table.concat(usecards,"+"))
		end
	else
		if not self.player:hasUsed("YisheCard") then use.card=card return end
	end
end

table.insert(sgs.ai_global_flags, "yisheasksource")
local yisheask_filter = function(player, carduse)
	if carduse.card:inherits("YisheAskCard") then
		sgs.yisheasksource = player
	else
		sgs.yisheasksource = nil
	end
end

table.insert(sgs.ai_choicemade_filter.cardUsed, yisheask_filter)

sgs.ai_skill_choice.yisheask=function(self,choices)
	assert(sgs.yisheasksource)
	if self:isFriend(sgs.yisheasksource) then return "allow" else return "disallow" end
end

local yisheask_skill={name="yisheask"}
table.insert(sgs.ai_skills,yisheask_skill)
yisheask_skill.getTurnUseCard = function(self)
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasSkill("yishe") and not player:getPile("rice"):isEmpty() then return sgs.Card_Parse("@YisheAskCard=.") end
	end
end

sgs.ai_skill_use_func.YisheAskCard=function(card,use,self)
	if self.player:usedTimes("YisheAskCard")>1 then return end
	local zhanglu
	local cards
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasSkill("yishe") and not player:getPile("rice"):isEmpty() then zhanglu=player cards=player:getPile("rice") break end
	end	
	if not zhanglu or not self:isFriend(zhanglu) then return end
	cards = sgs.QList2Table(cards)
	for _, pcard in ipairs(cards) do
		if not sgs.Sanguosha:getCard(pcard):inherits("Shit") then
			use.card = card
			return
		end
	end
end

sgs.ai_skill_invoke.zhenwei = true

sgs.ai_skill_invoke.yitian = function(self, data)
	local damage = data:toDamage()
	return self:isFriend(damage.to)
end

local taichen_skill={}
taichen_skill.name="taichen"
table.insert(sgs.ai_skills,taichen_skill)
taichen_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("TaichenCard") then return end
	return sgs.Card_Parse("@TaichenCard=.")
end

sgs.ai_skill_use_func.TaichenCard=function(card,use,self)
	local target, card_str
	
	local targets, friends, enemies = {}, {}, {}
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self.player:canSlash(player) then 
			table.insert(targets, player) 
			
			if self:isFriend(player) then
				table.insert(friends, player)
			else 
				table.insert(enemies, player)
			end
		end
	end
	
	if #targets == 0 then return end
	
	if #friends ~= 0 then
		for _, friend in ipairs(friends) do
			local judge_card = friend:getCards("j")
			local equip_card = friend:getCards("e")
		
			if judge_card and judge_card:length() > 0 and not (judge_card:length() == 1 and judge_card:at(0):objectName() == "lightning") then 
				target = friend 
				break 
			end
			if equip_card and equip_card:length() > 1 and self:hasSkills(sgs.lose_equip_skill, friend) then 
				target = friend 
				break 
			end
		end
	end
	
	if not target and #enemies > 0 then
		self:sort(enemies, "defense")
		for _, enemy in ipairs(enemies) do
			if enemy:getCards("he") and enemy:getCards("he"):length()>=2 then 
				target = enemy 
				break
			end
		end
	end
	
	if not target then return end
	
	local weapon = self.player:getWeapon()
	local hcards = self.player:getHandcards()
	for _, hcard in sgs.qlist(hcards) do
		if hcard:inherits("Weapon") then 
			if weapon then card_str = "@TaichenCard=" .. hcard:getId() end
		end
	end
	
	if not card_str then
		if weapon and self.player:getOffensiveHorse() then
			card_str = "@TaichenCard=" .. weapon:getId() 
		else
			if self:isFriend(target) and self.player:getHp() > 2 then card_str = "@TaichenCard=." end
			if self:isEnemy(target) and self.player:getHp() > 3 then card_str = "@TaichenCard=." end
		end
	end
	
	if card_str then
		if use.to then
			use.to:append(target)
		end
		use.card = sgs.Card_Parse(card_str)
	end
end

sgs.ai_cardneed.taichen = sgs.ai_cardneed.equip
