-- danlao
sgs.ai_skill_invoke.danlao = function(self, data)
	local effect = data:toCardEffect()
	if effect.card:inherits("GodSalvation") and self.player:isWounded() then
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
	local dmg
	
	if data=="@@tianxiang-card" then
		dmg = self.room:getTag("TianxiangDamage"):toDamage()
	else
		dmg=data
	end
	
	self:sort(self.enemies,"hp")
	
	for _, enemy in ipairs(self.enemies) do
		if (enemy:getHp() <= dmg.damage) then 
			
		if (enemy:getHandcardNum() <= 2) 
		or enemy:containsTrick("indulgence")
		or enemy:hasSkill("guose") 
		or enemy:hasSkill("leiji") 
		or enemy:hasSkill("ganglie") 
		or enemy:hasSkill("enyuan") 
		or enemy:hasSkill("qingguo") 
		or enemy:hasSkill("wuyan") 
		or enemy:hasSkill("kongcheng") 
		then target = enemy break end
		
		end
	end	
	
	if target then
		local cards = self.player:getCards("h")
        cards=sgs.QList2Table(cards)
		self:sortByUseValue(cards,true)
        for _,card in ipairs(cards) do
			if (card:getSuit() == sgs.Card_Spade or card:getSuit() == sgs.Card_Heart) then
				card_id = card:getId()
				return "@TianxiangCard="..card_id.."->"..target:objectName()
			end	
		end
	end 
	
	for _, friend in ipairs(self.friends_noself) do
		if (friend:getLostHp() + dmg.damage>1) then	
				if friend:isChained() and #self:getChainedFriends()>1 and dmg.nature>0 then 
				
				elseif friend:getHp() >= 2 and dmg.damage<2 and 
				(
				friend:hasSkill("yiji") 
				or friend:hasSkill("jieming") 
				or (friend:getHandcardNum()<3 and friend:hasSkill("rende"))
				or friend:hasSkill("buqu")
				or friend:hasSkill("shuangxiong") 
				or friend:hasSkill("zaiqi") 
				or friend:hasSkill("yinghun") 
				or friend:hasSkill("jianxiong")
				or friend:hasSkill("fangzhu")
				)
				then target=friend break 
				
				elseif friend:hasSkill("buqu") then target=friend break end
		end
	end
	
	if target then
		local cards = self.player:getCards("h")
        cards=sgs.QList2Table(cards)
		self:sortByUseValue(cards,true)
        for _,card in ipairs(cards) do
			if (card:getSuit() == sgs.Card_Spade or card:getSuit() == sgs.Card_Heart) and not card:inherits("Peach") then
				card_id = card:getId()
				return "@TianxiangCard="..card_id.."->"..target:objectName()
			end	
		end
	end
	
	for _, enemy in ipairs(self.enemies) do
		if (enemy:getLostHp() <= 1) or dmg.damage>1 then 
			
		if (enemy:getHandcardNum() <= 2) 
		or enemy:containsTrick("indulgence")
		or enemy:hasSkill("guose") 
		or enemy:hasSkill("leiji") 
		or enemy:hasSkill("ganglie") 
		or enemy:hasSkill("enyuan") 
		or enemy:hasSkill("qingguo") 
		or enemy:hasSkill("wuyan") 
		or enemy:hasSkill("kongcheng") 
		then target = enemy break end
		
		end
	end	
	
	if target then
		local cards = self.player:getCards("h")
        cards=sgs.QList2Table(cards)
		self:sortByUseValue(cards,true)
        for _,card in ipairs(cards) do
			if (card:getSuit() == sgs.Card_Spade or card:getSuit() == sgs.Card_Heart) and not card:inherits("Peach") then
				card_id = card:getId()
				return "@TianxiangCard="..card_id.."->"..target:objectName()
			end	
		end
	end
	
	return "."
end	

sgs.ai_skill_choice["guhuo"] = function(self, choices)
	local r = math.random(0, 1)
	if r == 0 then
	return "question"
	else
	return "noquestion"
	end
end