sgs.ai_skill_invoke.jueji=function(self,data)
	local target=data:toPlayer()
	if target:getHandcardNum()<1 or (target:getHandcardNum()==1 and self:hasSkills(sgs.need_kongcheng,target)) then
		return false
	end
	local handcard=target:getHandcardNum()
	local max_point=self:getMaxCard():getNumber()
	local poss=((max_point-1)/13)^handcard
	if math.random()<=poss then return true end

end

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

sgs.ai_skill_use["@@chengxiang"]=function(self,prompt)
	local prompts=prompt:split(":")
	assert(prompts[1]=="@chengxiang-card")
	local point=tonumber(prompts[4])
	local targets=self.friends
	if not targets then return end
	self:sort(targets,"hp")
	if not targets[1]:isWounded() then return end
	local cards=self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
	for _,card in ipairs(cards) do
		if card:getNumber()==point then return "@ChengxiangCard=" .. card:getId() .. "->" .. targets[1]:objectName() end
	end
	for _,card1 in ipairs(cards) do
		for __,card2 in ipairs(cards) do
			if card1:getId()==card2:getId() then
			elseif card1:getNumber()+card2:getNumber()==point then
				if targets[2] and targets[2]:isWounded() then
					return "@ChengxiangCard=" .. card1:getId() .. "+" .. card2:getId() .. "->" .. targets[1]:objectName() .. "+" .. targets[2]:objectName()
				elseif targets[1]:getHp()==1 or self:getUseValue(card1)+self:getUseValue(card2)<=6 then
					return "@ChengxiangCard=" .. card1:getId() .. "+" .. card2:getId() .. "->" .. targets[1]:objectName()
				end
			end
		end
	end
	return "."
end

-- lianli
sgs.ai_skill_use["@lianli"] = function(self, prompt)
	self:sort(self.friends)
	
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() then
			return "@LianliCard=.->" .. friend:objectName()
		end
	end
	
	return "."	
end

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

local lianli_slash_skill={name="lianli-slash"}
table.insert(sgs.ai_skills, lianli_slash_skill)
lianli_slash_skill.getTurnUseCard = function(self)
	if self.player:getMark("@tied")>0 then return sgs.Card_Parse("@LianliSlashCard=.") end
end

sgs.ai_skill_use_func["LianliSlashCard"] = function(card, use, self)
	if self.player:hasUsed("LianliSlashCard") and not sgs.lianlislash then return end
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	self:useBasicCard(slash, use)
	if use.card then use.card = card end
end

-- tongxin
sgs.ai_skill_invoke.tongxin = true

-- wuling, choose a effect randomly
sgs.ai_skill_choice.wuling = function(self, choices)
	local choices_table = choices:split("+")
	local available = {}
	for _, availchoice in ipairs(choices_table) do
		available[availchoice] = true
	end
	if available["water"] then
		self:sort(self.friends, "hp")
		if self:isWeak(self.friends[1]) then return "water" end
	end
	if available["earth"] then
		if #(self:getChainedFriends()) > #(self:getChainedEnemies()) and
			#(self:getChainedFriends()) + #(self:getChainedEnemies()) > 1 then return "earth" end
		if self:hasWizard(self.enemies, true) and not self:hasWizard(self.friends, true) then
			for _, player in sgs.qlist(self.room:getAlivePlayers()) do
				if player:containsTrick("lightning") then return "earth" end
			end
		end
	end
	if available["fire"] then
		for _,enemy in ipairs(self.enemies) do
			if self:isEquip("GaleShell", enemy) or self:isEquip("Vine", enemy) then return "fire" end
		end
		if #(self:getChainedFriends()) < #(self:getChainedEnemies()) and
			#(self:getChainedFriends()) + #(self:getChainedEnemies()) > 1 then return "fire" end
	end
	if available["wind"] then
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
	if available["thunder"] then
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
	return choices_table[math.random(1, #choices_table)]
end

-- caizhaoji_hujia
sgs.ai_skill_invoke.caizhaoji_hujia = function(self, data)
	local zhangjiao = self.room:findPlayerBySkillName("guidao")
	if zhangjiao and self:isEnemy(zhangjiao) and zhangjiao:getCards("he"):length()>2 then return false end
	return true
end

-- zhenggong, always invoke
sgs.ai_skill_invoke.zhenggong  = true

sgs.ai_skill_invoke.toudu = function(self, data)
	return #self.enemies>0
end

sgs.ai_skill_playerchosen.toudu = sgs.ai_skill_playerchosen.zero_card_as_slash

-- yitian-sword

-- hit enemy when yitian sword was lost
sgs.ai_skill_invoke["yitian-lost"] = function(self, data)
	if next(self.enemies) then
		return true
	else
		return false
	end
end

sgs.ai_skill_playerchosen["yitian_lost"] = sgs.ai_skill_playerchosen.damage

sgs.ai_skill_invoke["yitian_sword"] = function(self, targets)
	local slash=self:getCard("Slash")
	if not slash then return false end
	dummy_use={isDummy=true}
	self:useBasicCard(slash,dummy_use)
	if dummy_use.card then return true else return false end
end

-- zhenwei
sgs.ai_skill_invoke.zhenwei = true

sgs.ai_skill_invoke.yitian = function(self, data)
	local damage = data:toDamage()
	return self:isFriend(damage.to)
end

-- weiwudi (guixin2)
sgs.ai_skill_invoke.guixin2 = true

local function findPlayerForModifyKingdom(self, players)
	local lord = self.room:getLord()
	local isGood = self:isFriend(lord)

	for _, player in sgs.qlist(players) do
		if player:getRole() == "loyalist" then
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
	if to_modify:getRole() == "loyalist" or to_modify:getRole() == "renegade" then
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
	end

	return "wei"
end

sgs.ai_skill_choice.guixin2 = function(self, choices)
	if choices == "wei+shu+wu+qun" then
		local to_modify = self.room:getTag("Guixin2Modify"):toPlayer()
		return chooseKingdomForPlayer(self, to_modify)
	end

	if choices ~= "modify+obtain" then
		return choices:split("+")[1]
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

-- Lu Kang's Weiyan
sgs.ai_skill_invoke.lukang_weiyan = function(self, data)
	local handcard = self.player:getHandcardNum()
	local max_card = self.player:getMaxCards()

	prompt = data:toString()
	if prompt == "draw2play" then
		return handcard >= max_card and #(self:getTurnUse())>0
	elseif prompt == "play2draw" then
		return handcard < max_card
	end
end

local yishe_skill={name="yishe"}
table.insert(sgs.ai_skills,yishe_skill)
yishe_skill.getTurnUseCard = function(self)
	return sgs.Card_Parse("@YisheCard=.")
end

sgs.ai_skill_use_func["YisheCard"]=function(card,use,self)
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

sgs.ai_skill_use_func["YisheAskCard"]=function(card,use,self)
	if self.player:usedTimes("YisheAskCard")>1 then return end
	local zhanglu
	local cards
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasSkill("yishe") and not player:getPile("rice"):isEmpty() then zhanglu=player cards=player:getPile("rice") break end
	end	
	if not zhanglu then return end
	cards = sgs.QList2Table(cards)
	for _, pcard in ipairs(cards) do
		if not sgs.Sanguosha:getCard(pcard):inherits("Shit") then
			use.card = card
			return
		end
	end
end


sgs.ai_skill_invoke.gongmou = true

sgs.ai_skill_playerchosen.gongmou = function(self,choices)
	self:sort(self.enemies,"defense")
	return self.enemies[1]
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

sgs.ai_skill_use_func["LexueCard"] = function(card, use, self)
	if self.player:hasFlag("lexue") then
		local lexuesrc = sgs.Sanguosha:getCard(self.player:getMark("lexue"))
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByUseValue(cards, true)
		for _, hcard in ipairs(cards) do
			if hcard:getSuit() == lexuesrc:getSuit() then
				local lexue = sgs.Sanguosha:cloneCard(lexuesrc:objectName(), lexuesrc:getSuit(), lexuesrc:getNumber())
				lexue:addSubcard(hcard:getId())
				lexue:setSkillName("lexue")
				if self:getUseValue(lexuesrc) > self:getUseValue(hcard) then
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
