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
		    if card:inherits("Slash") and (self:getSlashNumber(self.player)<=1)then
		    elseif card:inherits("Jink") and (self:getJinkNumber(self.player)<=1)then
		    elseif card:inherits("Peach") and (self.player:getHp()<=2)then
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

local taichen_skill={}
taichen_skill.name="taichen"
table.insert(sgs.ai_skills,taichen_skill)
taichen_skill.getTurnUseCard=function(self)
		local card_str = ("@TaichenCard=.")
		local taichen_card = sgs.Card_Parse(card_str)
		assert(taichen_card)
        return taichen_card
end

sgs.ai_skill_use_func["TaichenCard"]=function(card,use,self)
    if self.player:usedTimes("TaichenCard")>0 then return end
    local lord=self.room:getLord()
    if self.player:getHp()>=lord:getHp() then
        if (self:getSlashNumber(self.player)+1)*2>self:getSlashNumber(lord) then
            use.card=card
        end
    end
end

local flood_skill={}
flood_skill.name="flood"
table.insert(sgs.ai_skills,flood_skill)
flood_skill.getTurnUseCard=function(self)
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
    if self.skill_flood_used then return end
    
    local eqs=0
    
    local players=self.room:getOtherPlayers(self.player)
    for _,player in sgs.qlist(players) do
        if player:getRole()=="rebel" then 
            eqs=eqs+self:getEquipNumber(player)
            if (player:getHandcardNum()<=2) or (player:getHp()<2) then
                eqs=eqs+2
            end
        end
    end
    
    if eqs>3 then 
        use.card=card 
        if not use.isDummy then self.skill_flood_used=true end
    end
end

sgs.ai_skill_use["dujiang"]=function(self)
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
    
    if rebel*2+3>(self:getEquipsNumber()*2+self.player:getHandcardNum()) then return true end
    if self:getHp()==1 and self:getHandcardNum()<=1 then return true end
    return false
end

local ganran_skill={}
ganran_skill.name="ganran"
table.insert(sgs.ai_skills,ganran_skill)
ganran_skill.getTurnUseCard=function(self)
	return nil
end