
-- liegong, same with tieji
sgs.ai_skill_invoke.liegong = sgs.ai_skill_invoke.tieji

-- jushou, allways invoke
sgs.ai_skill_invoke.jushou = true

--leiji
sgs.ai_skill_use["@@leiji"]=function(self,prompt)
	self:sort(self.enemies,"hp")
	for _,enemy in ipairs(self.enemies) do
		self.room:output("@LeijiCard=.->"..enemy:objectName())
		if enemy then return "@LeijiCard=.->"..enemy:objectName() end
		return "."
	end
	return "."
end

--shensu

sgs.ai_skill_use["@@shensu1"]=function(self,prompt)
	self:sort(self.enemies,"defense")
	
	local selfSub = self.player:getHp()-self.player:getHandcardNum()
	local selfDef = getDefense(self.player)
	local hasJud = self.player:getJudgingArea()
	
	for _,enemy in ipairs(self.enemies) do
		local def=getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not 
				((amr:inherits("Vine") and not self.player:hasWeapon("fan"))
				or (amr:objectName()=="eight_diagram"))
		
		if def<6 and eff then return "@ShensuCard=.->"..enemy:objectName() end
		
		if selfSub>=2 then return "." end
		if selfDef<6 then return "." end
		
	end
	
	for _,enemy in ipairs(self.enemies) do
		local def=getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not 
				((amr:inherits("Vine") and not self.player:hasWeapon("fan"))
				or (amr:objectName()=="eight_diagram"))
		
		if eff and def<8 then return "@ShensuCard=.->"..enemy:objectName() 
		else return "." end 
	end
	return "."
end

sgs.ai_get_cardType=function(card)
if card:inherits("Weapon") then return 1 end
if card:inherits("Armor") then return 2 end 
if card:inherits("OffensiveHorse")then return 3 end 
if card:inherits("DefensiveHorse") then return 4 end 
end

sgs.ai_skill_use["@@shensu2"]=function(self,prompt)
	self:sort(self.enemies,"defense")
	
	local selfSub = self.player:getHp()-self.player:getHandcardNum()
	local selfDef = getDefense(self.player)
	
	local cards = self.player:getCards("he")
	
	cards=sgs.QList2Table(cards)
	
	local eCard
	local hasCard={}
	
	for _,card in ipairs(cards) do
		if card:inherits("EquipCard") then 
			if hasCard[sgs.ai_get_cardType(card)] then 
				hasCard[sgs.ai_get_cardType(card)]=hasCard[sgs.ai_get_cardType(card)]+1
			else
				hasCard[sgs.ai_get_cardType(card)]=1
			end
		end		
	end
	
	for _,card in ipairs(cards) do
		if card:inherits("EquipCard") then 
			if hasCard[sgs.ai_get_cardType(card)]>1 or sgs.ai_get_cardType(card)==3 then 
				eCard=card 
				break
			end
		end
	end
	
	if not eCard then return "." end
	

	for _,enemy in ipairs(self.enemies) do
		local def=getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not 
				((amr:inherits("Vine") and not self.player:hasWeapon("fan"))
				or (amr:objectName()=="eight_diagram"))
		
		if def<6 and eff then return "@ShensuCard="..eCard:getEffectiveId().."->"..enemy:objectName() end
		
		if selfSub<0 then return "." end
	end
	
	for _,enemy in ipairs(self.enemies) do
		local def=getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not 
				((amr:inherits("Vine") and not self.player:hasWeapon("fan"))
				or (amr:objectName()=="eight_diagram"))
		
		if eff then return "@ShensuCard="..eCard:getEffectiveId().."->"..enemy:objectName();end 
	end
	return "."
end