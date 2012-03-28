sgs.ai_skill_invoke.zishou = function(self, data)
	return self.player:getHandcardNum() < 2 and self.player:isWounded()
end

sgs.ai_skill_invoke.qianxi = function(self, data)
	local damage = data:toDamage()
	local target = damage.to
	if self:isFriend(target) then return false end
	if self:hasSkills(sgs.masochism_skill,target) or self:hasSkills(sgs.recover_skill,target) then return true
	else
		return not ((target:getHp() < 2 and target:getMaxHp() > 2) and not (target:hasSkill("longhun") or target:hasSkill("buqu")))
	end
end

sgs.ai_skill_invoke.fuli = true

sgs.ai_skill_invoke.fuhun = function(self, data)
	local target = 0
	for _,enemy in ipairs(self.enemies) do
		if (self.player:distanceTo(enemy) <= self.player:getAttackRange())  then target = target + 1 end
	end
	return target > 0 and not self.player:isSkipped(sgs.Player_Play)
end

sgs.ai_skill_invoke.zhenlie = function(self, data)
	local judge = data:toJudge()
	if not judge:isGood() then 
	return true end
	return false
end

sgs.ai_skill_playerchosen.miji = function(self)
	self:sort(self.friends,"defense")
	for _, target in ipairs(self.friends) do
		return target 
	end
end

sgs.ai_skill_choice.jiangchi = function(self, choices)
	local target = 0
	local goodtarget = 0
	local slashnum = 0
	local needburst = 0
	
	for _, slash in ipairs(self:getCards("Slash")) do
		for _,enemy in ipairs(self.enemies) do
			if self:slashIsEffective(slash, enemy) then 
				slashnum = slashnum + 1 break
			end 
		end
	end

	for _,enemy in ipairs(self.enemies) do
		for _, slash in ipairs(self:getCards("Slash")) do
			if self:slashIsEffective(slash, enemy) and (self.player:distanceTo(enemy) <= self.player:getAttackRange())  then 
				goodtarget = goodtarget + 1  break
			end
		end
	end
	if slashnum > 1 or (slashnum > 0 and goodtarget == 0) then needburst = 1 end
	self:sort(self.enemies,"defense")
	
	for _,enemy in ipairs(self.enemies) do
		local def=sgs.getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not
			((amr:inherits("Vine") and not self.player:hasWeapon("fan"))
			or (amr:objectName()=="eight_diagram"))
			
		if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
		elseif self:slashProhibit(nil, enemy) then
		elseif def<6 and eff and needburst > 0 then return "chi"
		end	
	end
	
	for _,enemy in ipairs(self.enemies) do
		local def=sgs.getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not
			((amr:inherits("Vine") and not self.player:hasWeapon("fan"))
			or (amr:objectName()=="eight_diagram"))

		if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
		elseif self:slashProhibit(nil, enemy) then
		elseif eff and def<8 and needburst > 0 then return "chi"
		end
	end
	if goodtarget == 0 then return "jiang" end
	return "cancel"
end

sgs.ai_view_as.gongqi = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:getTypeId() == sgs.Card_Equip then
		return ("slash:gongqi[%s:%s]=%d"):format(suit, number, card_id)
	end
end

local gongqi_skill={}
gongqi_skill.name="gongqi"
table.insert(sgs.ai_skills,wusheng_skill)
gongqi_skill.getTurnUseCard=function(self,inclusive)
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	
	local equip_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards) do
		if card:getTypeId() == sgs.Card_Equip and ((self:getUseValue(card)<sgs.ai_use_value.Slash) or inclusive) then
			equip_card = card
			break
		end
	end

	if equip_card then		
		local suit = equip_card:getSuitString()
		local number = equip_card:getNumberString()
		local card_id = equip_card:getEffectiveId()
		local card_str = ("slash:gongqi[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
		
		assert(slash)
		
		return slash
	end
end

sgs.ai_skill_invoke.jiefan = function(self, data)
	local dying = data:toDying()
	local slashnum = 0
	local friend = dying.who
	local currentplayer = self.room:getCurrent()
	for _, slash in ipairs(self:getCards("Slash")) do
		if self:slashIsEffective(slash, currentplayer) then 
			slashnum = slashnum + 1  
		end 
	end
	return self:isFriend(friend) and not (self:isEnemy(currentplayer) and currentplayer:hasSkill("leiji") 
		and (currentplayer:getHandcardNum() > 2 or self:isEquip("EightDiagram", currentplayer))) and slashnum > 0
end

sgs.ai_skill_cardask["jiefan-slash"] = function(self, data, pattern, target)
	for _, slash in ipairs(self:getCards("Slash")) do
		if self:slashIsEffective(slash, target) then 
			return slash:toString()
		end 
	end
	return "."
end

anxu_skill={}
anxu_skill.name="anxu"
table.insert(sgs.ai_skills,anxu_skill)
anxu_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("AnxuCard") then return nil end
	card=sgs.Card_Parse("@AnxuCard=.")
	return card

end

sgs.ai_skill_use_func.AnxuCard=function(card,use,self)

	self:sort(self.enemies,"handcard")
	local friends={}
	for _,player in ipairs(self.friends_noself) do
		if not player:hasSkill("manjuan") then
			table.insert(friends, player)
		end
	end
	self:sort(friends,"handcard")

	local lowest_friend=friends[1]

	self:sort(self.enemies,"defense")
	if lowest_friend then
		for _,enemy in ipairs(self.enemies) do
			local hand1=enemy:getHandcardNum()
			local hand2=lowest_friend:getHandcardNum()

			if (hand1 > hand2) then
				use.card=card
				if use.to then
					use.to:append(enemy)
					use.to:append(lowest_friend)
					return
				end
			end
		end
	end
end

sgs.ai_card_intention.AnxuCard = function(card, from, to)
	local compare_func = function(a, b)
		return a:getHandcardNum() < b:getHandcardNum()
	end
	table.sort(to, compare_func)
	if to[1]:getHandcardNum() < to[2]:getHandcardNum() then
		sgs.updateIntention(from, to[1], (to[2]:getHandcardNum()-to[1]:getHandcardNum())*20+40)
	end
end

sgs.ai_skill_invoke.zhuiyi = function(self, data)
	local players = self.room:getOtherPlayers(self.player)
	players = sgs.QList2Table(players)
	local friendnum = 0
	for _,player in ipairs(players) do
		if self:isFriend(player) then friendnum = friendnum + 1 end
	end
	return friendnum > 0
end

sgs.ai_skill_playerchosen.zhuiyi = function(self)
	self:sort(self.friends_noself,"defense")
	return self.friends_noself[1]
end

sgs.ai_view_as.lihuo = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:inherits("Slash") and not (card:inherits("FireSlash") or card:inherits("ThunderSlash")) then
		return ("fire_slash:lihuo[%s:%s]=%d"):format(suit, number, card_id)
	end
end

local lihuo_skill={}
lihuo_skill.name="lihuo"
table.insert(sgs.ai_skills,lihuo_skill)
lihuo_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)
	local slash_card
	
	for _,card in ipairs(cards)  do
		if card:inherits("Slash") and not (card:inherits("FireSlash") or card:inherits("ThunderSlash")) then
			slash_card = card
			break
		end
	end
	
	if not slash_card  then return nil end
	local suit = slash_card:getSuitString()
	local number = slash_card:getNumberString()
	local card_id = slash_card:getEffectiveId()
	local card_str = ("fire_slash:lihuo[%s:%s]=%d"):format(suit, number, card_id)
	local fireslash = sgs.Card_Parse(card_str)
	assert(fireslash)
	
	return fireslash
		
end

sgs.ai_skill_use["@@chunlao"] = function(self, prompt)
	local slashcards={}
	local chunlao = self.player:getPile("wine")
	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)
	for _,card in ipairs(cards)  do
		if card:inherits("Slash") then
			table.insert(slashcards,card:getId()) 
		end
	end
	if #slashcards > 0 and chunlao:isEmpty() then 
		return "@ChunlaoCard="..table.concat(slashcards,"+").."->".."." 
	end
	return "."
end

sgs.ai_skill_invoke.chunlao = sgs.ai_skill_invoke.buyi

sgs.ai_skill_invoke.zhiyu = function(self)
	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)
	local first
	local difcolor = 0
	for _,card in ipairs(cards)  do
		if not first then first = card end
		if (first:isRed() and card:isBlack()) or (card:isRed() and first:isBlack()) then difcolor = 1 end
	end
	return difcolor == 0
end

local qice_skill={}
qice_skill.name="qice"
table.insert(sgs.ai_skills,qice_skill)
qice_skill.getTurnUseCard=function(self)
	local cards = self.player:getHandcards()
	local allcard = {}
	cards = sgs.QList2Table(cards)
	local aoename = "savage_assault|archery_attack"
	local aoenames = aoename:split("|")
	local aoe
	local i
	local good, bad = 0, 0
	local caocao = self.room:findPlayerBySkillName("jianxiong") 
	local qicetrick = "savage_assault|archery_attack|ex_nihilo|god_salvation"
	local qicetricks = qicetrick:split("|")
	for i=1, #qicetricks do
		local forbiden = qicetricks[i]
		forbid = sgs.Sanguosha:cloneCard(forbiden, sgs.Card_NoSuit, 0)
		if self.player:isLocked(forbid) then return end
	end
	if  self.player:hasUsed("QiceCard") then return end
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() then
			good = good + 10/(friend:getHp())
			if friend:isLord() then good = good + 10/(friend:getHp()) end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if enemy:isWounded() then
			bad = bad + 10/(enemy:getHp())
			if enemy:isLord() then
				bad = bad + 10/(enemy:getHp())
			end
		end
	end

	for _,card in ipairs(cards)  do
		table.insert(allcard,card:getId()) 
	end

	if self.player:getHandcardNum() < 3 then
		for i=1, #aoenames do
			local newqice = aoenames[i]
			aoe = sgs.Sanguosha:cloneCard(newqice, sgs.Card_NoSuit, 0)
			if self:getAoeValue(aoe) > -5 then
				local parsed_card=sgs.Card_Parse("@QiceCard=" .. table.concat(allcard,"+") .. ":" .. newqice)
				return parsed_card
			end
		end
		if good > bad then
			local parsed_card=sgs.Card_Parse("@QiceCard=" .. table.concat(allcard,"+") .. ":" .. "god_salvation")
			return parsed_card
		end
		if self:getCardsNum("Jink") == 0 and self:getCardsNum("Peach") == 0 then
			local parsed_card=sgs.Card_Parse("@QiceCard=" .. table.concat(allcard,"+") .. ":" .. "ex_nihilo")
			return parsed_card
		end
	end

	if self.player:getHandcardNum() == 3 then
		for i=1, #aoenames do
			local newqice = aoenames[i]
			aoe = sgs.Sanguosha:cloneCard(newqice, sgs.Card_NoSuit, 0)
			if self:getAoeValue(aoe) > 0 then
				local parsed_card=sgs.Card_Parse("@QiceCard=" .. table.concat(allcard,"+") .. ":" .. newqice)
				return parsed_card
			end
		end
		if good > bad and self.player:isWounded() then
			local parsed_card=sgs.Card_Parse("@QiceCard=" .. table.concat(allcard,"+") .. ":" .. "god_salvation")
			return parsed_card
		end
		if self:getCardsNum("Jink") == 0 and self:getCardsNum("Peach") == 0 and self:getCardsNum("Analeptic") == 0 and self:getCardsNum("Nullification") == 0 then
			local parsed_card=sgs.Card_Parse("@QiceCard=" .. table.concat(allcard,"+") .. ":" .. "ex_nihilo")
			return parsed_card
		end
	end
	for i=1, #aoenames do
		local newqice = aoenames[i]
		aoe = sgs.Sanguosha:cloneCard(newqice, sgs.Card_NoSuit, 0)
		if self:getAoeValue(aoe) > -5 and caocao and self:isFriend(caocao) and caocao:getHp()>1  and not caocao:containsTrick("indulgence") then
			local parsed_card=sgs.Card_Parse("@QiceCard=" .. table.concat(allcard,"+") .. ":" .. newqice)
			return parsed_card
		end
	end
	if self:getCardsNum("Jink") == 0 and self:getCardsNum("Peach") == 0 and self:getCardsNum("Analeptic") == 0 and self:getCardsNum("Nullification") == 0 then
		if good > bad and self.player:isWounded() then
			local parsed_card=sgs.Card_Parse("@QiceCard=" .. table.concat(allcard,"+") .. ":" .. "god_salvation")
			return parsed_card
		end
		local parsed_card=sgs.Card_Parse("@QiceCard=" .. table.concat(allcard,"+") .. ":" .. "ex_nihilo")
		return parsed_card
	end
end

sgs.ai_skill_use_func.QiceCard=function(card,use,self)
	local userstring=card:toString()
	userstring=(userstring:split(":"))[2]
	local qicecard=sgs.Sanguosha:cloneCard(userstring, card:getSuit(), card:getNumber())
	self:useTrickCard(qicecard,use) 
	if not use.card then return end
	use.card=card
end

sgs.ai_use_priority.QiceCard = 1.5