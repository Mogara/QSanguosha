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
		dmg = self.player:getTag("TianxiangDamage"):toDamage()
	else
		dmg = data
	end
	
	local cards = self.player:getCards("h")
    cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
    for _,card in ipairs(cards) do
		if (card:getSuit() == sgs.Card_Spade or card:getSuit() == sgs.Card_Heart) and not card:inherits("Peach") then
			card_id = card:getId()
			break
		end
	end
	if not card_id then return "." end
	
	self:sort(self.enemies, "hp")
	
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
		then return "@TianxiangCard="..card_id.."->"..enemy:objectName() end
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
			then return "@TianxiangCard="..card_id.."->"..friend:objectName()
			elseif friend:hasSkill("buqu") then return "@TianxiangCard="..card_id.."->"..friend:objectName() end
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
		then return "@TianxiangCard="..card_id.."->"..enemy:objectName() end
		end
	end	
	
	for i = #self.enemies, 1, -1 do
		local enemy = self.enemies[i]
		if not enemy:isWounded() and not self:hasSkills(sgs.masochism_skill, enemy) then
			return "@TianxiangCard="..card_id.."->"..enemy:objectName()
		end	
	end
	
	return "."
end	

sgs.ai_skill_choice["guhuo"] = function(self, choices)
	local players = self.room:getOtherPlayers(self.player)
	players = sgs.QList2Table(players)
	local yuji
	for _, other in ipairs(players) do
		if other:hasSkill("guhuo") then yuji = other break end
	end
	if self.lua_ai:isFriend(yuji) then return "noquestion" 
	else
		if self.player:getHp() >= 2 then 
			local r = math.random(0, 1)
			if r == 0 then
				return "question"
			else
				return "noquestion"
			end
		else return "noquestion"
		end
	end
end