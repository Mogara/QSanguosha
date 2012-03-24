sgs.ai_skill_invoke.zishou = function(self, data)
	return self.player:getHandcardNum() < 2
end

sgs.ai_skill_invoke.qianxi = function(self, data)
	local damage = data:toDamage()
	local target = damage.to
	return not ((target:getHp() < 2 and target:getMaxHp() > 1) and not (target:hasSkill("longhun") or target:hasSkill("buqu")))
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
	return self:isFriend(friend) and not (self:isFriend(currentplayer) and self:isWeak(currentplayer)) and slashnum > 0
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
	self:sort(self.friends_noself,"handcard")

	local lowest_friend=self.friends_noself[1]

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

--[[local lihuo_skill={}
lihuo_skill.name="lihuo"
table.insert(sgs.ai_skills,lihuo_skill)
lihuo_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)
	local target = 0
	local slash_card
	for _,enemy in ipairs(self.enemies) do
		if (self.player:distanceTo(enemy) <= self.player:getAttackRange()) and enemy:getHandcardNum() < 3 then target = target + 1 end
	end
	self:sortByUseValue(cards,true)

	
	for _,card in ipairs(cards)  do
		if card:inherits("Slash") then
			slash_card = card
			break
		end
	end
	
	if not slash_card or target < 2 then return nil end
	local suit = slash_card:getSuitString()
	local number = slash_card:getNumberString()
	local card_id = slash_card:getEffectiveId()
	local card_str = ("slash:lihuo[%s:%s]=%d"):format(suit, number, card_id)
	local fireslash = sgs.Card_Parse(card_str)
	assert(fireslash)
	
	return fireslash
		
end

function sgs.ai_skill_invoke.chunlao(self, data)
	local slash_num
	local weak = 0
	local slash_card
	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)
	for _,card in ipairs(cards)  do
		if card:inherits("Slash") then
			slash_num = slash_num + 1
		end
	end
	for _,friend in ipairs(self.friends) do
		if self:isWeak(friend) then weak = weak + 1 end
	end
	self:sortByUseValue(cards,true)
	for _,card in ipairs(cards)  do
		if card:inherits("Slash") then
			slash_card = card
			break
		end
	end
	if slash_num > 1 or (slash_num > 0 and weak > 0) then return slash_card end
	return false
end

sgs.ai_skill_cardask.ChunlaoCard = function(self, data)
	local dying = data:toDying()
	return self:isFriend(dying.who)
end]]