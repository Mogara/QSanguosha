-- pojun
sgs.ai_skill_invoke.pojun = function(self, data)
	local damage = data:toDamage()
	
	if not damage.to:faceUp() then
		return self:isFriend(damage.to)
	end		
	
	local good = damage.to:getHp() > 2	
	if self:isFriend(damage.to) then
		return good
	elseif self:isEnemy(damage.to) then
		return not good
	end
end

--jiushi
sgs.ai_skill_invoke.jiushi= true

--jiejiu
local jiejiu_skill={}
jiejiu_skill.name="jiejiu"
table.insert(sgs.ai_skills,jiejiu_skill)
jiejiu_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local anal_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:inherits("Analeptic") then 
			anal_card = card
			break
		end
	end

	if anal_card then		
		local suit = anal_card:getSuitString()
    	local number = anal_card:getNumberString()
		local card_id = anal_card:getEffectiveId()
		local card_str = ("slash:jiejiu[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
        
        return slash
	end
end

-- buyi
sgs.ai_skill_invoke.buyi = function(self, data)
	local dying = data:toDying()
	return self:isFriend(dying.who)
end

sgs.ai_cardshow.buyi = function(self, requestor)
	assert(self.player:objectName() == requestor:objectName())

	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getTypeId() ~= sgs.Card_Basic then
			return card
		end
	end

	return self.player:getRandomHandCard()
end

--xuanfeng
sgs.ai_skill_choice.xuanfeng = function(self, choices)
	self:sort(self.enemies, "defense")
	local slash = sgs.Card_Parse(("slash[%s:%s]"):format(sgs.Card_NoSuit, 0))
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy)<=1 then
			return "damage"
		elseif not self:slashProhibit(slash ,enemy) then
			return "slash"
		end
	end
	return "nothing"
end

sgs.ai_skill_playerchosen.xuanfeng_damage = function(self,targets)
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy)<=1 then return enemy end
	end

	return nil
end

sgs.ai_skill_playerchosen.xuanfeng_slash = function(self,targets)
	local slash = sgs.Card_Parse(("slash[%s:%s]"):format(sgs.Card_NoSuit, 0))
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if not (self:slashProhibit(slash ,enemy) or self:slashIsEffective(slash, enemy)) then return enemy end
	end
--	self:log("unfound")
	return self.enemies[1]
end

--xuanhuo
xuanhuo_skill={}
xuanhuo_skill.name="xuanhuo"
table.insert(sgs.ai_skills,xuanhuo_skill)
xuanhuo_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("XuanhuoCard") then
		return sgs.Card_Parse("@XuanhuoCard=.")
	end
end

sgs.ai_skill_use_func["XuanhuoCard"] = function(card, use, self)
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
		
	local target 
	for _, friend in ipairs(self.friends) do
		if self:hasSkills(sgs.lose_equip_skill, friend) then 
			for _, card in ipairs(cards) do
				if card:getSuit() == sgs.Card_Heart and self.player:getHandcardNum() > 1 then
					use.card = sgs.Card_Parse("@XuanhuoCard=" .. card:getEffectiveId())
					target = friend
					break
				end	
			end		
		end
		if target then break end
	end
	if not target then 
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isKongcheng() then
				for _, card in ipairs(cards)do
					if card:getSuit() == sgs.Card_Heart and not card:inherits("Peach")  and self.player:getHandcardNum() > 1 then
						use.card = sgs.Card_Parse("@XuanhuoCard=" .. card:getEffectiveId())
						target = enemy
						break
					end	
				end		
			end
			if target then break end
		end
	end
	
	if target then 
		self.room:setPlayerFlag(target, "xuanhuo_target")
		if use.to then
			use.to:append(target) 
		end
	end
end

sgs.ai_skill_playerchosen.xuanhuo = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if (player:getHandcardNum() <= 2 or player:getHp() < 2) and self:isFriend(player) and not player:hasFlag("xuanhuo_target") then
			return player
		end
	end
end

--ganlu
ganlu_skill={}
ganlu_skill.name="ganlu"
table.insert(sgs.ai_skills,ganlu_skill)
ganlu_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("GanluCard") then
		return sgs.Card_Parse("@GanluCard=.")
	end
end

sgs.ai_skill_use_func["GanluCard"] = function(card, use, self)
	local lost_hp = self.player:getLostHp()
	local enemy_equip = 0
	local target
	
	local has_xiaoji = false
	local xiaoji_equip = 0
	local sunshangxiang
	for _, friend in ipairs(self.friends) do
		if friend:hasSkill("xiaoji") then 
			has_xiaoji = true
			xiaoji_equip = self:getCardsNum(".", friend, "e")
			sunshangxiang = friend
			break 
		end
	end
	if has_xiaoji then
		local max_equip, max_friend = 0
		local min_equip, min_friend = 5
		for _, friend in ipairs(self.friends) do
			if not friend:hasSkill("xiaoji") then
				if (self:getCardsNum(".", friend, "e") > max_equip) and (self:getCardsNum(".", friend, "e")-xiaoji_equip<=lost_hp) then 
					max_equip = self:getCardsNum(".", friend, "e") 
					max_friend = friend
				elseif (self:getCardsNum(".", friend, "e") < min_equip) and (xiaoji_equip-self:getCardsNum(".", friend, "e")<=lost_hp) then 
					min_equip = self:getCardsNum(".", friend, "e")
					min_friend = friend
				end
			end
		end
	
		local equips  = {}
		if sunshangxiang and (max_equip~=0 or min_equip~=5) then 
			use.card = sgs.Card_Parse("@GanluCard=.")
			if use.to then
				use.to:append(sunshangxiang)
			end
			if (max_equip ~= 0) and ((max_equip-self:getCardsNum(".", sunshangxiang, "e"))>=0) then
				if use.to then use.to:append(max_friend) end
				return
			elseif(min_equip ~= 5) and ((self:getCardsNum(".", sunshangxiang, "e")-min_equip)>=0) then
				if use.to then use.to:append(min_friend) end
				return
			end
		end	
	end
	
	for _, friend in ipairs(self.friends) do
		for _, enemy in ipairs(self.enemies) do
			if not self:hasSkills(sgs.lose_equip_skill, enemy) then 
				if ((self:getCardsNum(".", enemy, "e")-self:getCardsNum(".", friend, "e"))<= lost_hp) and 
					(self:getCardsNum(".", enemy, "e")>=self:getCardsNum(".", friend, "e"))then
					use.card = sgs.Card_Parse("@GanluCard=.")
					if use.to then use.to:append(friend) end
					if use.to then use.to:append(enemy) end
					return
				end
			end
		end			
	end	
end


--jujian
jujian_skill={}
jujian_skill.name="jujian"
table.insert(sgs.ai_skills,jujian_skill)
jujian_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("JujianCard") then return sgs.Card_Parse("@JujianCard=.") end
end

sgs.ai_skill_use_func["JujianCard"] = function(card, use, self)
	local abandon_handcard = {}
	local index = 0
	local hasPeach=false
	local find_peach = self.player:getCards("h")
	for _, ispeach in sgs.qlist(find_peach) do
		if ispeach:inherits("Peach") then hasPeach=true break end
	end
	
	local trick_num, basic_num, equip_num = 0, 0, 0
	if not hasPeach and self.player:isWounded() and self.player:getHandcardNum() >=3 then 
		local cards = self.player:getHandcards()
		cards=sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		for _, card in ipairs(cards) do 
			if card:getTypeId() == sgs.Card_Trick and not card:inherits("ExNihilo") then trick_num = trick_num + 1
			elseif card:getTypeId() == sgs.Card_Basic then basic_num = basic_num + 1
			elseif card:getTypeId() == sgs.Card_Equip then equip_num = equip_num + 1
			end
		end
		local result_class
		if trick_num >= 3 then result_class = "TrickCard"
		elseif equip_num >= 3 then result_class = "EquipCard"
		elseif basic_num >= 3 then result_class = "BasicCard"
		end
		for _, friend in ipairs(self.friends_noself) do
			if (friend:getHandcardNum()<2) or (friend:getHandcardNum()<friend:getHp()+1) then
				for _, fcard in ipairs(cards) do 
					if fcard:inherits(result_class) and not fcard:inherits("ExNihilo") then
						table.insert(abandon_handcard, fcard:getId())
						index = index + 1
					end
					if index == 3 then break end
				end
			end
		end
		if index == 3 then 
			if use.to then use.to:append(friend) end
			use.card = sgs.Card_Parse("@JujianCard=" .. table.concat(abandon_handcard, "+"))
			return 
		end	
	else
		local cards = self.player:getHandcards()
		cards=sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		local slash_num = self:getCardsNum("Slash")
		local jink_num = self:getCardsNum("Jink")
		for _, friend in ipairs(self.friends_noself) do
			if (friend:getHandcardNum()<2) or (friend:getHandcardNum()<friend:getHp()+1) or self.player:isWounded() then
				for _, card in ipairs(cards) do
					if #abandon_handcard == 3 then break end
					if not card:inherits("Nullification") and not card:inherits("EquipCard") and 
						not card:inherits("Peach") and not card:inherits("Jink") and 
						not card:inherits("Indulgence") and not card:inherits("SupplyShortage") then
						table.insert(abandon_handcard, card:getId())
						index = 5
					elseif card:inherits("Slash") and slash_num > 1 then
						if (self.player:getWeapon() and not self.player:getWeapon():objectName()=="crossbow") or
							not self.player:getWeapon() then
							table.insert(abandon_handcard, card:getId())
							index = 5
							slash_num = slash_num - 1
						end
					elseif card:inherits("Jink") and jink_num > 1 then
						table.insert(abandon_handcard, card:getId())
						index = 5
						jink_num = jink_num - 1
					end
				end	
				if index == 5 then 
					use.card = sgs.Card_Parse("@JujianCard=" .. table.concat(abandon_handcard, "+"))
					if use.to then use.to:append(friend) end
					return
				end
			end			
		end	
	end
end


--mingce
mingce_skill={}
mingce_skill.name="mingce"
table.insert(sgs.ai_skills,mingce_skill)
mingce_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("MingceCard") then return end
	
	local card
	if self.player:getArmor() and (self.player:getArmor():objectName() == "silver_lion" and self.player:isWounded()) then 
		card = self.player:getArmor()
	end
	if not card then
		local hcards = self.player:getCards("h")
		hcards = sgs.QList2Table(hcards)
		self:sortByUseValue(hcards, true)

		for _, hcard in ipairs(hcards) do
			if hcard:inherits("Slash") or hcard:inherits("EquipCard") then
				card = hcard
				break
			end
		end
	end
	if card then
		card = sgs.Card_Parse("@MingceCard=" .. card:getEffectiveId()) 
		return card
	end

	return nil
end

sgs.ai_skill_use_func["MingceCard"]=function(card,use,self)
	local target
	self:sort(self.friends_noself, "defense")
	local friends = self.friends_noself
	for _, friend in ipairs(friends) do
		if friend:getHp() <= 2 and friend:getHandcardNum() < 2 then
			target = friend
			break
		end
	end

	if not target then
		local maxAttackRange=0
		for _, friend in ipairs(friends) do
			if friend:getAttackRange() > maxAttackRange then
				maxAttackRange = friend:getAttackRange()
				target = friend
			end
		end
	end

	if target then
		use.card=card
		if use.to then
			use.to:append(target)
		end
	end
end

sgs.ai_skill_choice.mingce = function(self, choices)
    if self.player:getHandcardNum()<=2 then return "draw" end
	if self.player:getHp()<=1 then return "draw" end
	for _,enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy) and not self:slashProhibit(slash ,enemy) then return "use" end
	end
    return "draw"
end

sgs.ai_skill_playerchosen.mingce = function(self, targets)
	local slash = sgs.Card_Parse(("slash[%s:%s]"):format(sgs.Card_NoSuit, 0))
	local targetlist=sgs.QList2Table(targets)

	self:sort(targetlist, "defense")
	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) and self.player:canSlash(target) and not self:slashProhibit(slash ,target) then
		return target
		end
	end
	return targets:first()
end