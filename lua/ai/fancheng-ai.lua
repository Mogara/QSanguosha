local zhiyuan_skill={}
zhiyuan_skill.name="zhiyuan"
table.insert(sgs.ai_skills,zhiyuan_skill)
zhiyuan_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)

	local basic_card

	self:sortByUseValue(cards,true)

	for _,card in ipairs(cards)  do
		if card:getTypeId()==sgs.Card_Basic then
			if card:isKindOf("Slash") and (self:getCardsNum("Slash")<=1)then
			elseif card:isKindOf("Jink") and (self:getCardsNum("Jink")<=1)then
			elseif card:isKindOf("Peach") and (self.player:getHp()<=2)then
			else
				basic_card = card
				break
			end
		end
	end

	if basic_card then
		local card_id = basic_card:getEffectiveId()
		local card_str = ("@ZhiyuanCard="..card_id)
		local zhiyuan_card = sgs.Card_Parse(card_str)
		assert(zhiyuan_card)

		return zhiyuan_card
	end
	return nil
end

sgs.ai_skill_use_func["ZhiyuanCard"]=function(card,use,self)
	if self.player:usedTimes("ZhiyuanCard")>1 then return end
	self:sort(self.friends_noself, "handcard")
	for _, friend in ipairs(self.friends_noself) do
		if friend:getRole()=="rebel" then
			use.card=card
			if use.to then
				use.to:append(friend)
			end
			return
		end

	end
end

local taichen_fight_skill={}
taichen_fight_skill.name="taichen_fight"
table.insert(sgs.ai_skills,taichen_fight_skill)
taichen_fight_skill.getTurnUseCard=function(self)
		local card_str = ("@TaichenFightCard=.")
		local taichen_card = sgs.Card_Parse(card_str)
		assert(taichen_card)
		return taichen_card
end

sgs.ai_skill_use_func["TaichenFightCard"]=function(card,use,self)
	if self.player:usedTimes("TaichenFightCard")>0 then return end
	local lord=self.room:getLord()
	if self.player:getHp()>=lord:getHp() then
		if (self:getCardsNum("Slash")+1)*2>self:getCardsNum("Slash", lord) then
			use.card=card
		end
	end
end

local flood_skill={}
flood_skill.name="flood"
table.insert(sgs.ai_skills,flood_skill)
flood_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("FloodCard") then return end

	local cards=self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)

	local blacks={}

	for _,card in ipairs(cards) do
		if card:isBlack() then
			table.insert(blacks,card:getEffectiveId())
			if #blacks==3 then break end
		end
	end

	if #blacks<3 then return nil end

	local card_str = ("@FloodCard="..table.concat(blacks,"+"))
	local flood_card = sgs.Card_Parse(card_str)
	assert(flood_card)
	return flood_card
end

sgs.ai_skill_use_func["FloodCard"]=function(card,use,self)

	local eqs=0

	local players=self.room:getOtherPlayers(self.player)
	for _,player in sgs.qlist(players) do
		if player:getRole()=="rebel" then
			eqs=eqs+self.player:getEquips():length()
			if (player:getHandcardNum()<=2) or (player:getHp()<2) then
				eqs=eqs+2
			end
		end
	end

	if eqs>3 then
		use.card=card
	end
end

sgs.ai_skill_use["@dujiang-card"]=function(self)
	local equips=self.player:getEquips()
	equips=sgs.QList2Table(equips)

	if #equips<2 then return "." end

	return ("@DujiangCard="..equips[1]:getEffectiveId().."+"..equips[2]:getEffectiveId().."->"..".")
end

sgs.ai_skill_invoke.xiansheng=function(self)
	local players=self.room:getOtherPlayers(self.player)

	local rebel=0
	for _,player in sgs.qlist(players) do
		if self:objectiveLevel(player)>=4 then
			rebel=rebel+1
		end
	end

	if rebel*2+3>(self.player:getEquips():length()*2+self.player:getHandcardNum()) then return true end
	if self.player:getHp()==1 and self.player:getHandcardNum()<=1 then return true end
	return false
end

local ganran_skill={}
ganran_skill.name="ganran"
table.insert(sgs.ai_skills,ganran_skill)
ganran_skill.getTurnUseCard=function(self)
	local cards=self.player:getCards("h")
		cards=sgs.QList2Table(cards)

	for _,card in ipairs(cards) do
		if card:isKindOf("EquipCard") then
			local suit = card:getSuitString()
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local card_str = ("iron_chain:ganran[%s:%s]=%d"):format(suit, number, card_id)
			local thecard=sgs.Card_Parse(card_str)
			return thecard
		end
	end

	return nil
end
