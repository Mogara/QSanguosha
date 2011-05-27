-- danlao
sgs.ai_skill_invoke.danlao = function(self, data)
	local effect = data:toCardEffect()
	if effect.card:inherits "GodSalvation" and self.player:isWounded() then
		return false
	else
		return true
	end
end

--tianxiang
sgs.ai_skill_use["@tianxiang"]=function(self, data)	
	local friend_lost_hp = 10
	local friend_hp = 0
	local card_id
	local target
	local cant_use_skill
	
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() <= 1 then 
			target = enemy
		end	
		if enemy:getHandcardNum() > 2 then 
			cant_use_skill = true
			break
		end
	end	
	
	if not cant_use_skill and target then
		local cards = self.player:getCards("h")
        cards=sgs.QList2Table(cards)
        for _,card in ipairs(cards) do
			if (card:getSuit() == sgs.Card_Spade or card:getSuit() == sgs.Card_Heart) then
				card_id = card:getId()
				break
			end	
		end
	end 
	
	if card_id then return "@TianxiangCard="..card_id.."->"..target:objectName() end
	
	
	for _, friend in ipairs(self.friends_noself) do
		if friend:getLostHp() < friend_lost_hp and friend:getHp() > 2 then	
				friend_lost_hp = friend:getLostHp()
				target = friend
		end
	end
	
	if target and friend_lost_hp < 2 then
		local cards = self.player:getCards("h")
        cards=sgs.QList2Table(cards)
        for _,card in ipairs(cards) do
			if (card:getSuit() == sgs.Card_Spade or card:getSuit() == sgs.Card_Heart) and not card:inherits("Peach") then
				card_id = card:getId()
				break
			end	
		end
		
	end
	
	if card_id then return "@TianxiangCard="..card_id.."->"..target:objectName() end
	
	local enemy_hp = 0
	target = nil
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() > enemy_hp then 
			target = enemy
			enemy_hp = enemy:getHp()
		end	
	end	
	if target then 
		local cards = self.player:getCards("h")
        cards=sgs.QList2Table(cards)
        for _,card in ipairs(cards) do
			if (card:getSuit() == sgs.Card_Spade or card:getSuit() == sgs.Card_Heart) and not card:inherits("Peach") then
				card_id = card:getId()
				break
			end	
		end
		
	end
	
	if card_id then return "@TianxiangCard="..card_id.."->"..target:objectName() end
	
	return "."
end	
