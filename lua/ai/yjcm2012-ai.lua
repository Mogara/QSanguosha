sgs.ai_skill_invoke.zishou = function(self, data)
	local chance_value = 1
	if (self.player:getHp() <= 2) then
		chance_value = chance_value + 1
	else
		local can_slash_card_num = 0
		for _, card in ipairs(sgs.QList2Table(self.player:getHandcards())) do
			if card:isKindOf("Slash") then
				for _,enemy in ipairs(self.enemies) do
					if self:slashIsEffective(card, enemy) 
						and self.player:distanceTo(enemy) <= self.player:getAttackRange() then 
						can_slash_card_num = can_slash_card_num + 1
						break
					end 
				end
			end
		end
		if(can_slash_card_num >= 1) then chance_value = chance_value - 1 end
	end

	local peach_num = self:getCardsNum("Peach")
	local can_save_card_num = self.player:getMaxCards() - self.player:getHandcardNum()

	return self.player:isSkipped(sgs.Player_Play)
			or ((self.player:getLostHp() + 2) - can_save_card_num + peach_num  <= chance_value)
end

sgs.ai_skill_invoke.qianxi = function(self, data)
	local damage = data:toDamage()
	local target = damage.to
	if self:isFriend(target) then return false end
	if target:getLostHp() >= 2 and target:getHp() <= 1 then return false end
	if self:hasSkills(sgs.masochism_skill,target) or self:hasSkills(sgs.recover_skill,target) or self:hasSkills("longhun|buqu",target) then return true end
	if damage.damage > 1 then return false end
	return (target:getMaxHp() - target:getHp()) < 2 
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

sgs.ai_skill_playerchosen.miji = function(self, targets)
	targets = sgs.QList2Table(targets)
	self:sort(targets, "defense")
	for _, target in ipairs(targets) do
		if self:isFriend(target) then return target end 
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
	if goodtarget == 0 or self.player:isSkipped(sgs.Player_Play) then return "jiang" end
		
	for _,enemy in ipairs(self.enemies) do
		local def=sgs.getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("QinggangSword") or not
			((amr:isKindOf("Vine") and not self.player:hasWeapon("Fan"))
			or (amr:objectName()=="EightDiagram"))
			
		if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
		elseif self:slashProhibit(nil, enemy) then
		elseif def<6 and eff and needburst > 0 then return "chi"
		end	
	end
	
	for _,enemy in ipairs(self.enemies) do
		local def=sgs.getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("QinggangSword") or not
			((amr:isKindOf("Vine") and not self.player:hasWeapon("Fan"))
			or (amr:objectName()=="EightDiagram"))

		if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
		elseif self:slashProhibit(nil, enemy) then
		elseif eff and def<8 and needburst > 0 then return "chi"
		end
	end

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
	target = target or global_room:getCurrent()
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
		for _,friend in ipairs(self.friends_noself) do
			local hand1=friend:getHandcardNum()
			local hand2=lowest_friend:getHandcardNum()
			if hand1 > hand2 and hand1 > 2 then
				use.card=card
				if use.to then
					use.to:append(friend)
					use.to:append(lowest_friend)
					return
				end
			end
		end
	end
	self:sort(self.enemies,"handcard",true)
	local much_enemy = self.enemies[1]
	for _,enemy in ipairs(self.enemies) do
		local hand1=enemy:getHandcardNum()
		local hand2=much_enemy:getHandcardNum()
		if hand1 < hand2 and hand1 > 1 then
			use.card=card
			if use.to then
				use.to:append(enemy)
				use.to:append(much_enemy)
				return
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
	local damage = data:toDamageStar()
	local exclude = self.player
	if damage and damage.from then exclude = damage.from end
	
	local friends = self:getFriendsNoself()
	table.removeOne(friends, exclude)
	return #friends > 0
end

sgs.ai_skill_playerchosen.zhuiyi = function(self, targets)	
	targets = sgs.QList2Table(targets)
	self:sort(targets,"defense")
	for _, friend in ipairs(targets) do
		if self:isFriend(friend) then
			return friend
		end
	end
end

sgs.ai_view_as.lihuo = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:isKindOf("Slash") and not (card:isKindOf("FireSlash") or card:isKindOf("ThunderSlash")) then
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
		if card:isKindOf("Slash") and not (card:isKindOf("FireSlash") or card:isKindOf("ThunderSlash")) then
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
		if card:isKindOf("Slash") then
			table.insert(slashcards,card:getId()) 
		end
	end
	if #slashcards > 0 and chunlao:isEmpty() then 
		return "@ChunlaoCard="..table.concat(slashcards,"+").."->".."." 
	end
	return "."
end

sgs.ai_skill_invoke.chunlao = function(self, data)
	local dying = data:toDying()
	return self:isFriend(dying.who) and self.player:getPile("wine"):length() > 0
end

sgs.chengpu_keep_value = 
{
	Peach = 6,
	Jink = 5.1,
	Slash = 5.5,
}

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
	userstring=(userstring:split(":"))[3]
	local qicecard=sgs.Sanguosha:cloneCard(userstring, card:getSuit(), card:getNumber())
	self:useTrickCard(qicecard,use) 
	if not use.card then return end
	use.card=card
end

sgs.ai_use_priority.QiceCard = 1.5