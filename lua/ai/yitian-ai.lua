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

sgs.ai_skill_invoke.lianli_slash = function(self, prompt)
	return self:getCardsNum("Slash")==0
end

-- tongxin
sgs.ai_skill_invoke.tongxin = true

-- wuling, choose a effect randomly
sgs.ai_skill_choice.wuling = function(self, choices)
	local choices_table = choices:split("+")
	return choices_table[math.random(1, #choices_table)]
end

-- caizhaoji_hujia
sgs.ai_skill_invoke.caizhaoji_hujia = true

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

sgs.ai_skill_playerchosen["yitian_lost"] = function(self, targets)
	self:sort(self.enemies, "hp")
	return self.enemies[1]
end

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

	if self.player:getPhase() == sgs.Player_Draw then
		-- weiyan1: Draw -> Play
		return handcard >= max_card
	elseif self.player:getPhase() == sgs.Player_Play then
		-- weiyan2: Play -> Draw
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
	for _, player in ipairs(self.friends_noself) do
		if player:hasSkill("yishe") and not player:getPile("rice"):isEmpty() then return sgs.Card_Parse("@YisheAskCard=.") end
	end
end

sgs.ai_skill_use_func["YisheAskCard"]=function(card,use,self)
	if self.player:usedTimes("YisheAskCard")>1 then return end
	local zhanglu
	local cards
	for _, player in ipairs(self.friends_noself) do
		if player:hasSkill("yishe") and not player:getPile("rice"):isEmpty() then zhanglu=player cards=player:getPile("rice") break end
	end	
	if not zhanglu then return end
	cards = sgs.QList2Table(cards)
	card_id = sgs.ai_skill_askforag.qixing(self, cards)
	if card_id > -1 then
		sgs.yisheasksource=self.player
		use.card = card
	end
end
