
-- liegong, same with tieji
sgs.ai_skill_invoke.liegong = sgs.ai_skill_invoke.tieji

-- jushou, allways invoke
sgs.ai_skill_invoke.jushou = true

--leiji
sgs.ai_skill_use["@@leiji"]=function(self,prompt)
	self:sort(self.enemies,"hp")
	for _,enemy in ipairs(self.enemies) do

		if (enemy) and (self:objectiveLevel(enemy)>3) then return "@LeijiCard=.->"..enemy:objectName() end
		return "."
	end
	return "."
end

--shensu

sgs.ai_skill_use["@@shensu1"]=function(self,prompt)
        self:updatePlayers(true)
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
                if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
                elseif def<6 and eff then return "@ShensuCard=.->"..enemy:objectName()
		
                elseif selfSub>=2 then return "."
                elseif selfDef<6 then return "." end
		
	end
	
	for _,enemy in ipairs(self.enemies) do
		local def=getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not 
				((amr:inherits("Vine") and not self.player:hasWeapon("fan"))
				or (amr:objectName()=="eight_diagram"))

                if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
                elseif eff and def<8 then return "@ShensuCard=.->"..enemy:objectName()
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
        self:updatePlayers(true)
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
		
                if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
                elseif def<6 and eff then return "@ShensuCard="..eCard:getEffectiveId().."->"..enemy:objectName() end
		
		if selfSub<0 then return "." end
	end
	
	for _,enemy in ipairs(self.enemies) do
		local def=getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not 
				((amr:inherits("Vine") and not self.player:hasWeapon("fan"))
				or (amr:objectName()=="eight_diagram"))
		
                if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
                elseif eff then return "@ShensuCard="..eCard:getEffectiveId().."->"..enemy:objectName();end
	end
	return "."
end

function fillCardSet(cardSet,suit,suit_val,number,number_val)
    if suit then
        cardSet[suit]={}
        for i=1,13 do
            cardSet[suit][i]=suit_val
        end
    end
    if number then
        cardSet.club[number]=number_val
        cardSet.spade[number]=number_val
        cardSet.heart[number]=number_val
        cardSet.diamond[number]=number_val
    end
end

function goodMatch(cardSet,card)
    local result=card:getSuitString()
    local number=card:getNumber()
    if cardSet[result][number] then return true
    else return false
    end
end

sgs.ai_skill_invoke["@guidao"]=function(self,prompt)
    local data=prompt:split(":")
    local target=data[2]
    local reason=data[4]
    local card=sgs.Sanguosha:getCard(string.match(data[5],"%d+"))
    local result=card:getSuitString()
    local number=card:getNumber()

    local players=self.room:getAllPlayers()

    players=sgs.QList2Table(players)

    for _, player in ipairs(players) do

        if player:objectName()==target then
            target=player
            break
        end
    end

    local cardSet={}
    
    cardSet.club={}
    cardSet.spade={}
    cardSet.heart={}
    cardSet.diamond={}
    
    if reason=="indulgence" then
        fillCardSet(cardSet,"heart",true)
    elseif reason=="supply_shortage" then
        fillCardSet(cardSet,"club",true)
    elseif reason=="eight_diagram" then
        fillCardSet(cardSet,"heart",true)
        fillCardSet(cardSet,"diamond",true)
    elseif reason=="luoshen" then
        fillCardSet(cardSet,"spade",true)
        fillCardSet(cardSet,"club",true)
    elseif reason=="lightning" then
        fillCardSet(cardSet,"heart",true)
        fillCardSet(cardSet,"club",true)
        fillCardSet(cardSet,"diamond",true)
        for i=10,13 do 
            fillCardSet(cardSet,nil,nil,i,true)
        end
    elseif reason=="tieji" then
        fillCardSet(cardSet,"heart",true)
        fillCardSet(cardSet,"diamond",true)
    elseif reason=="leiji" then
        fillCardSet(cardSet,"heart",true)
        fillCardSet(cardSet,"club",true)
        fillCardSet(cardSet,"diamond",true)
    else
        return "."
    end

    if self:isEnemy(target) and (goodMatch(cardSet,card)) then
        fillCardSet(cardSet,"heart",true)
        fillCardSet(cardSet,"diamond",true)
        return self:getRetrialCard("he",cardSet,true)
    end
    if self:isFriend(target) and (not goodMatch(cardSet,card)) then
        fillCardSet(cardSet,"heart",false)
        fillCardSet(cardSet,"diamond",false)
        return self:getRetrialCard("he",cardSet,false)
    end
    
    if card:inherits("EightDiagram") or (result=="spade") 
    or (card:inherits("Jink") and (self:getJinkNumber(self.player)<=0)) then
        if not goodMatch(cardSet,card) then
            fillCardSet(cardSet,"heart",true)
            fillCardSet(cardSet,"diamond",true)
            fillCardSet(cardSet,"spade",true)
            return self:getRetrialCard("he",cardSet,true)
        else 
            fillCardSet(cardSet,"heart",false)
            fillCardSet(cardSet,"diamond",false)
            fillCardSet(cardSet,"spade",false)
            return self:getRetrialCard("he",cardSet,false)
        end
    end

    return "."
end

huangtianv_skill={}
huangtianv_skill.name="huangtianv"
table.insert(sgs.ai_skills,huangtianv_skill)
huangtianv_skill.getTurnUseCard=function(self)
    if self.huangtianv_used then return nil end
	if self.player:isLord() then return nil end
	if self.player:getKingdom() ~= "qun" then return nil end

    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local card
	
	self:sortByUseValue(cards,true)
	
	for _,acard in ipairs(cards)  do
		if acard:inherits("Jink") then
			card = acard
			break
		end
	end
	
	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = "@HuangtianCard="..card_id
	local skillcard = sgs.Card_Parse(card_str)
	
	assert(skillcard)
	
	return skillcard
end

sgs.ai_skill_use_func["HuangtianCard"]=function(card,use,self)
    if not self:isFriend(self.room:getLord()) then return nil end
    
	use.card=card
	if use.to then
     use.to:append(self.room:getLord()) 
    self.huangtianv_used=true 
    end
	
end