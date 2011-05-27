 -- This is the Smart AI, and it should be loaded and run at the server side

-- "middleclass" is the Lua OOP library written by kikito
-- more information see: https://github.com/kikito/middleclass
require "middleclass"

-- initialize the random seed for later use
math.randomseed(os.time())

-- this table stores all specialized AI classes
sgs.ai_classes = {}

-- compare functions
sgs.ai_compare_funcs = {
	hp = function(a, b)
		return a:getHp() < b:getHp()
	end,

	handcard = function(a, b)
		return a:getHandcardNum() < b:getHandcardNum()
	end,

	value = function(a, b)
		local value1 = a:getHp() * 2 + a:getHandcardNum()
		local value2 = b:getHp() * 2 + b:getHandcardNum()

		return value1 < value2
	end,

	chaofeng = function(a, b)
		local c1 = sgs.ai_chaofeng[a:getGeneralName()]	or 0
		local c2 = sgs.ai_chaofeng[b:getGeneralName()] or 0

		if c1 == c2 then
			return sgs.ai_compare_funcs.value(a, b)
		else
			return c1 > c2
		end
	end,

	defense = function(a,b)
		local d1=a:getHp() * 2 + a:getHandcardNum()
		if(d1>a:getHp()*3) then d1=a:getHp()*3 end
		if a:getArmor() then d1=d1+2 end
		local d2=b:getHp() * 2 + b:getHandcardNum()
		if(d2>b:getHp()*3) then d2=b:getHp()*3 end
		if b:getArmor() then d2=d2+2 end
		
		local c1 = sgs.ai_chaofeng[a:getGeneralName()]	or 0
		local c2 = sgs.ai_chaofeng[b:getGeneralName()] or 0
		
		if (a:getHandcardNum()<2) and (b:getHandcardNum()>=2) then return true end
		if (b:getHandcardNum()<2) and (a:getHandcardNum()>=2) then return false end
		
                if sgs.rebel_target:objectName()==a:objectName() then return true end
                if sgs.rebel_target:objectName()==b:objectName() then return false end
                
                if sgs.loyal_target then
                    if sgs.loyal_target:objectName()==a:objectName() then return true end
                    if sgs.loyal_target:objectName()==b:objectName() then return false end
                end

		return d1<d2
	end,

	threat = function ( a, b)
		local players=sgs.QList2Table(a:getRoom():getOtherPlayers(a))
		local d1=a:getHandcardNum()
		for _, player in ipairs(players) do
			if a:canSlash(player,true) then
				d1=d1+10/(getDefense(player))
			end
		end
		players=sgs.QList2Table(b:getRoom():getOtherPlayers(b))
		local d2=b:getHandcardNum()
		for _, player in ipairs(players) do
			if b:canSlash(player,true) then
				d2=d2+10/(getDefense(player))
			end
		end


		local c1 = sgs.ai_chaofeng[a:getGeneralName()]	or 0
		local c2 = sgs.ai_chaofeng[b:getGeneralName()] or 0


		 return d1+c1/2>d2+c2/2
        end,
}



-- this function is only function that exposed to the host program
-- and it clones an AI instance by general name
function CloneAI(player)
	local ai_class = sgs.ai_classes[player:getGeneralName()]
	if ai_class then
		return ai_class(player).lua_ai
	else
		return SmartAI(player).lua_ai
	end
end

function getCount(name)
	if sgs.ai_round[name] then 
                sgs.ai_round[name]=sgs.ai_round[name]+1
	else 
		sgs.ai_round[name]=1 
	end
        return sgs.ai_round[name]
end

--defense is defined as hp*2 + hand + (2)(if armor present)
function getDefense(player)
	local d=player:getHp() * 2 + player:getHandcardNum()
	if(d>player:getHp()*3) then d=player:getHp()*3 end
	if player:getArmor() then d=d+2 end
	return d
end

-- SmartAI is the base class for all other specialized AI classes
SmartAI = class "SmartAI"


-- the "initialize" function is just the "constructor"
function SmartAI:initialize(player)
	
	self.player = player
	self.room = player:getRoom()
	
        self.role =player:getRole()

        if sgs.ai_assumed[self.role] then sgs.ai_assumed[self.role] = sgs.ai_assumed[self.role] +1
        elseif self.role~="lord" then sgs.ai_assumed[self.role] =1
        end
	
	
	self.lua_ai = sgs.LuaAI(player)
	self.lua_ai.callback = function(method_name, ...)
		local method = self[method_name]
		if method then
			return method(self, ...)
		end
	end

        
        self.retain=2
        --self.harsh_retain=true
        if not sgs.ai_royalty[self.player:objectName()] then
            --self.room:output("initialized"..self.player:objectName()..self.role)
            sgs.ai_royalty[self.player:objectName()]=0
        end
        if self.player:isLord() then
            sgs.ai_royalty[self.player:objectName()]=160
            sgs.ai_explicit[self.player:objectName()]="loyalist"
            if (sgs.ai_chaofeng[self.player:getGeneralName()] or 0) < 3 then
                sgs.ai_chaofeng[self.player:getGeneralName()]=3
            end
        end
        
    self.keepValue={}
    self.kept={}


        --self:updatePlayers()
end

sgs.ai_assumed={}

function SmartAI:printStand()
    self.room:output(self.player:getRole())
    self.room:output("enemies:")
    for _, player in ipairs(self.enemies) do
        self.room:output(player:getGeneralName())
    end
    self.room:output("end of enemies")
    self.room:output("friends:")
    for _, player in ipairs(self.friends) do
        self.room:output(player:getGeneralName())
    end
    self.room:output("end of friends")
end

function isRolePredictable()
    if useDefaultStrategy() then return true end
    return sgs.GetConfig("RolePredictable", true)
end

function useDefaultStrategy()
	local mode=sgs.GetConfig("GameMode", "")
    if (mode=="06_3v3") or (not mode:find("0")) then return true end
    if (mode:find("02_1v1") or mode:find("03p")) then return true end
end

-- this function create 2 tables contains the friends and enemies, respectively
function SmartAI:updatePlayers(inclusive)
        --self:log("updated")
        self.friends = sgs.QList2Table(self.lua_ai:getFriends())
        table.insert(self.friends, self.player)

        self.friends_noself = sgs.QList2Table(self.lua_ai:getFriends())

        sgs.rebel_target=self.room:getLord()
        
        self.enemies = sgs.QList2Table(self.lua_ai:getEnemies())
        
        self.role =self.player:getRole()
		
        if isRolePredictable() then
            if (self.role=="lord") or (self.role=="loyalist") then self:refreshRoyalty(self.player,300)
            elseif (self.role=="rebel") then self:refreshRoyalty(self.player,-300)
			end
            
            self.retain=2
            self.harsh_retain=false
            --return
			if useDefaultStrategy() then return end
        end
        
        inclusive=inclusive or true
        
        local flist={}
        local elist={}
        self.enemies=elist
        self.friends=flist


        local lord=self.room:getLord()
        local role=self.role
        self.retain=2
        self.harsh_retain=true

        local players=self.room:getOtherPlayers(self.player)
        players=sgs.QList2Table(players)


        for _,player in ipairs(players) do
            if #players==1 then break end
            if self:objectiveLevel(player)<0 then table.insert(flist,player) end
        end

        self.friends_noself={}

        for _, player in ipairs (flist) do
            table.insert(self.friends_noself,player)
        end
        table.insert(self.friends,self.player)

        if self.role=="rebel" then
            sgs.rebel_target=self.room:getLord()
            self.retain=2
        end
--
        if self.player:getHp()<2 then self.retain=0 end
        self:sortEnemies(players)
        for _,player in ipairs(players) do
            if self:objectiveLevel(player)>=4 then self.harsh_retain=false end
            if #elist==0 then
                table.insert(elist,player)
                if self:objectiveLevel(player)<4 then self.retain=0 end
            else
                if self:objectiveLevel(player)<=0 then return end
                table.insert(elist,player)
                self:updateLoyalTarget(player)
                
                if self:objectiveLevel(player)>=4 then self.harsh_retain=false end
                --local use=self:getTurnUse()
                    --if (#use)>=(self.player:getHandcardNum()-self.player:getHp()+self.retain) then
                        --self.room:output(#    use.."cards can be used")
                        --if not inclusive then return end
                    --end
            end
        end




end

function SmartAI:updateLoyalTarget(player)
if self.role=="rebel" then return end
    if (self:objectiveLevel(player)>=4) then
        if not sgs.loyal_target then sgs.loyal_target=player 
		elseif self:getEquipNumber(sgs.loyal_target)>self:getEquipNumber(player) then sgs.loyal_target=player 
        elseif (sgs.loyal_target:getHp()>1) and (getDefense(player)<=3) then sgs.loyal_target=player 
        elseif (sgs.loyal_target:getHandcardNum()>0) and (player:getHandcardNum()==0) then sgs.loyal_target=player 
        elseif (sgs.ai_chaofeng[player:getGeneralName()] or 0)<(sgs.ai_chaofeng[sgs.loyal_target:getGeneralName()] or 0) then sgs.loyal_target=player 
        elseif (sgs.loyal_target:getArmor()) and (not player:getArmor()) then sgs.loyal_target=player 
        end
    end
end

function SmartAI:printFEList()
    self.room:output("-----------")
    self.room:output(self.player:getGeneralName().." list:")
    for _, player in ipairs (self.enemies) do
        self.room:output("enemy "..player:getGeneralName()..(sgs.ai_explicit[player:objectName()] or ""))
    end

    for _, player in ipairs (self.friends_noself) do
        self.room:output("friend "..player:getGeneralName()..(sgs.ai_explicit[player:objectName()] or ""))
    end
    self.room:output(self.player:getGeneralName().." list end")
end

function SmartAI:objectiveLevel(player)
    if useDefaultStrategy() then 
        if self.player:getRole()=="renegade" then
		elseif player:getRole()=="renegade" then
        elseif self:isFriend(player) then return -2
        elseif player:isLord() then return 5
        elseif player:getRole()=="renegade" then return 4.1
       else return 4.5 end
    end
    
    if player:objectName()==self.player:objectName() then return -2 end

    local modifier=0
    local rene=sgs.ai_renegade_suspect[player:objectName()] or 0
    if rene>1 then modifier=0.5 end
    
    local players=self.room:getOtherPlayers(self.player)
    players=sgs.QList2Table(players)
	
	
    local hasRebel=false
    for _,oplayer in ipairs(players) do
        if oplayer:getRole()=="rebel" then hasRebel=true break end
    end
        
    if self.role=="lord" then
    
        if #players == 1  then return 5 end 
		
        if not hasRebel then 
        
            local name=player:objectName()
			if player:getRole() == "renegade" then return 5 else return -2 end
			
			
--            self:sort(players,"defense")
--            if (players[#players]:objectName()==name) then modifier=-10
--            elseif players[1]:objectName()==name and ((sgs.ai_anti_lord[name] or 0)-2)<=(sgs.ai_lord_tolerance[name] or 0) then modifier=10
--            else modifier=2
        end
		
		local hasLoyal=false
		for _, oplayer in ipairs(players) do
			if oplayer:getRole() == "loyalist" then hasLoyal=true break end
		end
		
		if not hasLoyal then 
			if player:getRole() == "renegade" then return -1 else return 5 end
		end
		
        if sgs.ai_explicit[player:objectName()]=="rebel" then return 5-modifier
        elseif sgs.ai_explicit[player:objectName()]=="rebelish" then return 5-modifier
        elseif sgs.ai_explicit[player:objectName()]=="loyalist" then return -2
        elseif sgs.ai_explicit[player:objectName()]=="loyalish" then return -1
        elseif (self:singleRole())=="rebel" then return 4.6-modifier
        elseif (self:singleRole())=="loyalist" then return -1
        elseif (sgs.ai_royalty[player:objectName()]<=0) and 
            (sgs.ai_card_intention["general"](player,100)>0) 
            then return 3
        else return 0 end
    elseif self.role=="loyalist" then
		
		if not hasRebel then
			if player:getRole() == "renegade" then return 5 else return -2 end
		end
		
        if sgs.ai_explicit[player:objectName()]=="rebel" then return 5-modifier
        elseif sgs.ai_explicit[player:objectName()]=="rebelish" then return 5-modifier
        elseif player:isLord() then return -2
        elseif sgs.ai_explicit[player:objectName()]=="loyalist" then return -1
        elseif sgs.ai_explicit[player:objectName()]=="loyalish" then return -1
        elseif (self:singleRole())=="rebel" then return 4-modifier
        elseif (self:singleRole())=="loyalist" then return -1
        elseif (sgs.ai_royalty[player:objectName()]<=0) and 
            (sgs.ai_card_intention["general"](player,100)>0) 
            then return 3.1
        else return 0 end
    elseif self.role=="rebel" then
        if player:isLord() then return 5 end
		if not hasRebel then 
			if player:getRole() == "renegade" and #players>2 then return -1 
			elseif player:getRole() == "renegade" and #players == 2 then return 3
			end
        elseif sgs.ai_explicit[player:objectName()]=="loyalist" then return 5-modifier
        elseif sgs.ai_explicit[player:objectName()]=="loyalish" then return 5-modifier
        elseif sgs.ai_explicit[player:objectName()]=="rebel" then return -1
        elseif sgs.ai_explicit[player:objectName()]=="rebelish" then return -1
        elseif (self:singleRole())=="rebel" then return -1
        elseif (self:singleRole())=="loyalist" then return 4-modifier
        elseif (sgs.ai_royalty[player:objectName()]>0) and 
            (sgs.ai_card_intention["general"](player,100)<0) 
            then return 3
        else return 0 end
    elseif self.role=="renegade" then
        
        if #players==1 then return 5 end
        --if (#players==2) and player:isLord() then return 0 end
		
        if not hasRebel then 
			if player:isLord() then return 3 
			else return 5 end
		end
		
		local rebel_num = 0
		local loyalish_num = 0
        for _, aplayer in ipairs (players) do
            if aplayer:getRole()=="rebel" then 
                rebel_num = rebel_num + 1
			else
				loyalish_num = loyalish_num + 1
			end
        end
		if rebel_num > loyalish_num then 
			if player:getRole() == "rebel" then return 5 else return -1 end
		elseif rebel_num < loyalish_num then
			if player:getRole() == "rebel" then return -1 
			elseif player:getRole() == "loyalist" and not player:isLord() then return 5
			elseif player:isLord() then 
				if player:getHp() > 2 then return 3.5 else return -1 end
			end
		else
			local loyalish_count, rebel_count = 0, 0
			local loyalish_hp, rebel_hp = 0, 0
			for _, aplayer in ipairs(players) do
				if aplayer:getRole() == "rebel" then 
					rebel_hp = rebel_hp + aplayer:getHp()
					rebel_count = rebel_count + 1
				else
					loyalish_hp = loyalish_hp + aplayer:getHp()
					loyalish_count = loyalish_count + 1
				end
			end
			if (loyalish_hp-loyalish_count) <= (rebel_hp-rebel_count) then
				if player:getRole() == "rebel" then return 5
				else return -1 end
			else
				if player:getRole() == "rebel" then return 3
				else 
					if player:isLord() and loyalish_count == 1 then
						if loyalish_hp > rebel_hp then return 5 else return -2 end
					elseif loyalish_count > 1 then
						if player:isLord() then 
							if player:getHp() < 3 then return -1 else return 4 end
						else return 5 
						end
					end
				end
			end
		end
		
    end
    return 1
end

function SmartAI:sortEnemies(players)
    local comp_func=function(a,b)
        local alevel=self:objectiveLevel(a)
        local blevel=self:objectiveLevel(b)

        if alevel~=blevel then return alevel>blevel end
        if alevel==3 then return getDefense(a)>getDefense(b) end

        alevel=sgs.ai_chaofeng[a:getGeneralName()] or 0
        blevel=sgs.ai_chaofeng[b:getGeneralName()] or 0
        if alevel~=blevel then
            return alevel>blevel
        end

        alevel=getDefense(a)
        blevel=getDefense(b)
        if alevel~=blevel then
            return alevel<blevel
        end
    end
    table.sort(players,comp_func)
end

function SmartAI:hasWizard(players)
	for _, player in ipairs(players) do
		if player:hasSkill("guicai") or player:hasSkill("guidao") or player:hasSkill("tiandu") then
			return true
		end
	end
        return false
end

function SmartAI:sort(players, key)
	key = key or "chaofeng" -- the default compare key is "chaofeng"

	local func= sgs.ai_compare_funcs[key]

	assert(func)

	table.sort(players, func)
end

function SmartAI:filterEvent(event, player, data)


        if event == sgs.CardUsed then
            self:updatePlayers()
        elseif event == sgs.CardEffect then
            self:updatePlayers()
        elseif event == sgs.Death then
                self:updatePlayers()
				
                if self==sgs.recorder then
				speakTrigger(nil,player,nil,"death")
                	local selfexp=sgs.ai_explicit[player:objectName()]
                	if selfexp then
                	    if selfexp=="loyalish" then selfexp="loyalist"
                	    elseif selfexp=="rebelish" then selfexp="rebel"
                	    end
                    	sgs.ai_explicit[player:objectName()]=nil
                    	sgs.ai_assumed[selfexp]=sgs.ai_assumed[selfexp]+1
                    end
                    sgs.ai_assumed[player:getRole()]=sgs.ai_assumed[player:getRole()]-1 
                end
        end
        if (event == sgs.TurnStart) or (event == sgs.GameStart) then
                self:updatePlayers()
                --if (self.room:nextPlayer():objectName()==self.player:objectName()) then
                for _,skill in ipairs(sgs.ai_skills) do
                    if self:hasSkill(skill) then
                    self[skill.name.."_used"]=false
                    end
                end
                

                --end
                --self:updatePlayers()
                 --self:printRoyalty()
        end

        if not sgs.recorder then
            sgs.recorder=self
        end

        if self~=sgs.recorder then return end


        if event == sgs.TurnStart then
            self:updateRoyalty(self.room:getCurrent())
        end

        if event == sgs.CardEffect then

                local struct= data:toCardEffect()
                local card  = struct.card
                local to    = struct.to
                local from  = struct.from
                local source= self.room:getCurrent()
				
				
--                self.room:output(
  --                  card:className().." "..
    --                from:getGeneralName().." "..
      --              to:getGeneralName().." ".."effected")
                if sgs.ai_card_intention[card:className()] then
                    local intention=sgs.ai_card_intention[card:className()](card,from,to,source)
                    --self.room:output(intention..">")
                    if to:isLord() and intention<0 then 
                    sgs.ai_anti_lord[from:objectName()]=(sgs.ai_anti_lord[from:objectName()] or 0)+1
                    end
                    self:refreshRoyalty(from,intention)
                end
        elseif event == sgs.CardUsed then
                local struct= data:toCardUse()
                --self.room:output("struct")
                local card  = struct.card

--                self.room:output("Card")
                local to    = struct.to
                      to    = sgs.QList2Table(to)
--                self.room:output("to")
                local from  = struct.from
--                self.room:output("from")
                local source= self.room:getCurrent()

 --               self.room:output(
   --                 card:className().." "..
     --               from:getGeneralName().." ".."used"
       --             )
                   

                for _, eachTo in ipairs(to) do
                    if sgs.ai_carduse_intention[card:className()] then
                        local intention=sgs.ai_carduse_intention[card:className()](card,from,eachTo,source)
                        self:refreshRoyalty(from,intention)
                        
                        if eachTo:isLord() and intention<0 then 
                        sgs.ai_anti_lord[from:objectName()]=(sgs.ai_anti_lord[from:objectName()] or 0)+1
                        end
                        
                    end
                    self.room:output(eachTo:objectName())
                end
        elseif event == sgs.DrawNCards then
            --self.room:output(player:getGeneralName().." draws "..data:toInt())

        elseif event == sgs.CardDiscarded then
            local card = data:toCard()
            local cards= card:getSubcards()
            if type(cards)=="QList" then
                cards=sgs.QList2Table(cards)
                self.room:output(player:getGeneralName().." discards "..table.concat(cards,"+"))
            end

        elseif event == sgs.CardResponsed then
            local card = data:toCard()
            --self.room:output(player:getGeneralName().." responded with "..card:className())

        elseif event == sgs.CardLost then
            local move=data:toCardMove()
            local from=move.from
            local to=  move.to
            local place=move.from_place
            if sgs.ai_snat_disma_effect then 
                self.room:output(
                    "cardlostevent "..
                    from:getGeneralName().." "..
                    place
                    )
                sgs.ai_snat_disma_effect=false
                local intention=sgs.ai_card_intention.general(from,70)
                if place==2 then intention=-intention end
                
                if from:isLord() and intention<0 then 
                sgs.ai_anti_lord[sgs.ai_snat_dism_from:objectName()]=(sgs.ai_anti_lord[sgs.ai_snat_dism_from:objectName()] or 0)+1
                end
                
                self:refreshRoyalty(sgs.ai_snat_dism_from,intention)
            end
        end

end



function SmartAI:isFriend(other)
    if useDefaultStrategy() then return self.lua_ai:isFriend(other) end
    if (self.player:objectName())==(other:objectName()) then return true end 
	if self:objectiveLevel(other)<0 then return true end
    return false
end

function SmartAI:isEnemy(other)
    if useDefaultStrategy() then return self.lua_ai:isEnemy(other) end
    if (self.player:objectName())==(other:objectName()) then return false end 
	if self:objectiveLevel(other)>0 then return true end
	return false
    --local players=self.enemies
    --for _,player in ipairs(players) do
    --    if (player:objectName())==(other:objectName()) then return true end
    --end
        --return self.lua_ai:isEnemy(other)
    --    return false
end

function SmartAI:isNeutrality(other)
	return self.lua_ai:relationTo(other) == sgs.AI_Neutrality
end

-- get the card with the maximal card point
function SmartAI:getMaxCard(player)
	player = player or self.player

	if player:isKongcheng() then
		return nil
	end

	local cards = player:getHandcards()
	local max_card, max_point = nil, 0
	for _, card in sgs.qlist(cards) do
		local point = card:getNumber()
		if point > max_point then
			max_point = point
			max_card = card
		end
	end

	return max_card
end

-- the table that stores whether the skill should be invoked
-- used for SmartAI:askForSkillInvoke
sgs.ai_skill_invoke = {
	eight_diagram = true,
	double_sword = true,
	fan = true,
	
	kylin_bow = function(self, data)	
		local effect = data:toSlashEffect()
		
		if effect.to:hasSkill("xiaoji") then
			return false
		end
		
		return self:isEnemy(effect.to)
	end,
}

function SmartAI:askForSkillInvoke(skill_name, data)
	local invoke = sgs.ai_skill_invoke[skill_name]
	if type(invoke) == "boolean" then
		return invoke
	elseif type(invoke) == "function" then
		return invoke(self, data)
	else
		local skill = sgs.Sanguosha:getSkill(skill_name)
		return skill and skill:getFrequency() == sgs.Skill_Frequent
	end
end

function SmartAI:askForYiji(cards)									----function complete
    self:sort(self.friends_noself,"handcard")
		
	if self.player:getHandcardNum()>3 then
		if #self.friends_noself>0 then
			return self.friends_noself[1],cards[1]
		end
	end
    if true then return nil, 0 end
	
	--	cards = sgs.QList2Table(cards)
    if self.player:getHandcardNum()>=2 then
		for card_id in ipairs(cards) do
			local card = sgs.Sanguosha:getCard(card_id)
			for _, friend in ipairs(self.friends_noself) do
				if friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage") then
					if card:inherits("Nullification") then
						return friend, card_id 
					end
				else
					if card:inherits("Jink") then 
						if friend:getHp() < 2 and self:getJinkNumber(friend) < 1 then 
							return friend, card_id 
						end 
					end 
					if friend:hasSkill("jizhi") then 
						if card:getTypeId() == sgs.Card_Trick then 
							return friend, card_id 
						end 
					end
					if friend:hasSkill("paoxiao") or friend:hasSkill("tianyi") or self:getSlashNumber(friend) < 1 then 
						if card:inherits("Slash") then 
							return friend, card_id 
						end 
					end
					if friend:hasSkill("xiaoji") or friend:hasSkill("xuanfeng") then 
						if card:inherits("EquipCard") then return friend, card_id end 
					end
					if friend:hasSkill("guose") then 
						if card:getSuit() == sgs.Card_Diamond then return friend, card_id end 
					end
					if friend:hasSkill("tianxiang") then 
						if card:getSuit() == sgs.Card_Spade or
							card:getSuit() == sgs.Card_Heart then
							return friend, card_id 
						end 
					end
					if friend:hasSkill("leiji") then
						if self:getJinkNumber(friend) < 1 and card:inherits("Jink") then return friend, card_id
						elseif card:getSuit() == sgs.Card_Spade then return friend, card_id 
						end
					end
					if friend:hasSkill("xuanhuo") then 
						if (card:getSuit() == sgs.Card_Heart and 
						(card:inherits("Equipcard") or card:inherits("Jink") or card:inherits("FireSlash"))) then 
							return friend, card_id 
						end
					end
					
					if friend:hasSkill("qingguo") then 
						if card:isBlack() then return friend, card_id end 
					end
					if friend:hasSkill("qingnang") then 
						if card:isRed() then return friend, card_id end 
					end
					if friend:getHandcardNum() < friend:getHp() then return friend, card_id end
				end
			end
		end
	end
end

-- used for SmartAI:askForUseCard
sgs.ai_skill_use = {}

function SmartAI:askForUseCard(pattern, prompt)
	local use_func = sgs.ai_skill_use[pattern]
	if string.find(pattern,"@@liuli") then use_func = sgs.ai_skill_use["@@liuli"]
	elseif string.find(pattern,"dujiang") then use_func = sgs.ai_skill_use["dujiang"] end
	if use_func then
		return use_func(self, prompt) or "."
	else
		return "."
	end
end

function SmartAI:slashIsEffective(slash, to)
	if self.player:hasWeapon("qinggang_sword") then
		return true
	end

	local armor = to:getArmor()
	if armor then
		if armor:objectName() == "renwang_shield" then
		    if not slash then return true end
			return not slash:isBlack()
		elseif armor:inherits("Vine") then
		    if not slash then return false end
			return slash:inherits("NatureSlash") or self.player:hasWeapon("fan")
		end
	end

	return true
end

function SmartAI:slashHit(slash, to)
	
	return self:getJinkNumber(to) == 0
end

function SmartAI:slashIsAvailable()
	if self.player:hasWeapon("crossbow") or self.player:hasSkill("paoxiao") then
		return true
	end

	if self.player:hasFlag("tianyi_success") then
		return (self.player:usedTimes("Slash") + self.player:usedTimes("FireSlash") + self.player:usedTimes("ThunderSlash")) < 2
    else
        return (self.player:usedTimes("Slash") + self.player:usedTimes("FireSlash") + self.player:usedTimes("ThunderSlash")) < 1
	end
end


function SmartAI:getSlash()
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)
    
    self:sortByUsePriority(cards)
    
    for _, slash in ipairs(cards) do
        if slash:inherits("Slash") then return slash end
    end
    return nil
end

function SmartAI:getJink()																		---------add no use
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)
    
    self:sortByUsePriority(cards)
    
    for _, jink in ipairs(cards) do
        if jink:inherits("Jink") then return jink end
    end
    return nil
end

function SmartAI:getEquipNumber(player)
	local equips = player:getEquips()
	local index = 0
	if equips then
		for _, equip in sgs.qlist(equips) do
			index = index + 1
		end
	end
	return index
end

function SmartAI:searchForAnaleptic(use,enemy,slash)

    
    
    if not self.toUse then return nil end
	
	for _,card in ipairs(self.toUse) do
		if card:getId()~=slash:getId() then return nil end
	end
    --if #self.toUse<=1 then return nil end
    if not use.to then return nil end
    if self.anal_used then return nil end
    
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)
    self:fillSkillCards(cards)
    
    
   if (getDefense(self.player)<getDefense(enemy))and
   (self.player:getHandcardNum()<1+self.player:getHp()) or
     self.player:hasFlag("drank") then return end

    if enemy:getArmor() then 
        if ((enemy:getArmor():objectName())=="eight_diagram")
            or ((enemy:getArmor():objectName())=="silver_lion") then return nil end end
    for _, anal in ipairs(cards) do
        if (anal:className()=="Analeptic") and not (anal:getEffectiveId()==slash:getEffectiveId()) then
            self.anal_used=true
            
            return anal
        end
    end
return nil
end

function SmartAI:slashProhibit(card,enemy)
    if self:isFriend(enemy) then
        if card:inherits("FireSlash") or self.player:hasWeapon("fan") then
             if enemy:hasArmorEffect("vine") then return true end
        end
        if enemy:isChained() and not card:inherits("NatureSlash") then return true end
    end
    
    if enemy:hasSkill("liuli") then 
        if enemy:getHandcardNum()<1 then return false end
        for _, friend in ipairs(self.friends_noself) do
            if enemy:canSlash(friend,true) and self:slashIsEffective(card, friend) then return true end
        end
    end
    
    if enemy:hasSkill("leiji") then 
        if self.player:hasSkill("tieji") then return false end
        
        if enemy:getHandcardNum()>=2 then return true end
        if enemy:getArmor() and (enemy:getArmor():objectName()=="eight_diagram") then 
            local equips=enemy:getEquips()
            for _,equip in sgs.qlist(equips) do
                if equip:getSuitString()=="spade" then return true end
            end
        end
    end
    
    if enemy:hasSkill("tiandu") then 
        if enemy:getArmor() and (enemy:getArmor():objectName()=="eight_diagram") then return true end
    end
    
    if enemy:hasSkill("ganglie") then
        if self.player:getHandcardNum()+self.player:getHp()<5 then return true end
    end
	
	if enemy:hasSkill("yizhong") and not enemy:getArmor() and card:isBlack() then					--yujin's yizhong
		return true
	end	
	
	if enemy:hasSkill("shenjun") and (enemy:getGeneral():isMale()~=self.player:getGeneral():isMale()) and not card:inherits("ThunderSlash") then
	    return true
	end
	
	if enemy:getArmor() and (enemy:getArmor():objectName() == "renwang_shield") and ((not card) or card:isBlack()) then					--renwang shield
		if not self.player:getWeapon() or self.player:getWeapon():objectName() ~= "qinggang_sword"  then
			return true
		end	
	end	
	
	if enemy:getArmor() and (enemy:getArmor():objectName() == "vine") and ((not card) or card:inherits("NatureSlash")) then					--problem about equipping vine with nature slash
		return true
	end	
	
	if  (enemy:hasSkill("zhichi") and self.room:getTag("zhichi"):toString() == enemy:objectName()) then return true end			--chengong's zhichi
    
    return false
end

function SmartAI:useBasicCard(card, use,no_distance)
        if card:getSkillName()=="wushen" then no_distance=true end
	    if (self.player:getHandcardNum()==1) and self.player:getWeapon() and self.player:getWeapon():inherits("Halberd") then
            self.slash_targets=3
        end	
		
        if card:inherits("Slash") and self:slashIsAvailable() then
			local target_count=0
			for _, friend in ipairs(self.friends_noself) do						----guojia with is friend
                local slash_prohibit=false
                slash_prohibit=self:slashProhibit(card,friend)
				if (friend:hasSkill("yiji") and friend:getLostHp() <1) 
				or (friend:hasSkill("leiji") and self:getJinkNumber(friend) > 0)		--zhangjiao
                or (friend:isLord() and self.player:hasSkill("guagu") and friend:getLostHp()>=1 and self:getJinkNumber(friend)==0)
                then
					if not slash_prohibit then
                        if ((self.player:canSlash(friend, not no_distance)) or 
							(use.isDummy and (self.player:distanceTo(friend)<=self.predictedRange))) and 
							self:slashIsEffective(card, friend) then
							use.card=card
							if use.to then 
                                use.to:append(friend) 
                                self:speak("hostile", self.player:getGeneral():isFemale())
                            end
							target_count=target_count+1
							if self.slash_targets<=target_count then return end
						end
					end	
					break
				end
			end	
		
		    self:sort(self.enemies, "defense")
            for _, enemy in ipairs(self.enemies) do
                        local slash_prohibit=false
                        slash_prohibit=self:slashProhibit(card,enemy)
                        if not slash_prohibit then
                            if ((self.player:canSlash(enemy, not no_distance)) or 
                            (use.isDummy and (self.player:distanceTo(enemy)<=self.predictedRange))) and 
                            self:objectiveLevel(enemy)>3 and
                            self:slashIsEffective(card, enemy) then
                                -- fill the card use struct
                                local anal=self:searchForAnaleptic(use,enemy,card)
                                if anal then 
                                    use.card = anal
                                    return 
                                end
                                use.card=card
                                if use.to then use.to:append(enemy) end
                                target_count=target_count+1
                                if self.slash_targets<=target_count then return end
                            end
                        end
		    end
	    elseif card:inherits("Peach") and self.player:isWounded() then
				local peaches=0
				local cards = self.player:getHandcards()
    			cards=sgs.QList2Table(cards)
				for _,card in ipairs(cards) do
					if card:inherits("Peach") then peaches=peaches+1 end
				end
				
                	for _, friend in ipairs(self.friends_noself) do
                    	if (self.player:getHp()-friend:getHp()>peaches) and (friend:getHp()<3) and not friend:hasSkill("buqu") then return end
                	end	
                
			use.card = card
        
		
	end
end

function SmartAI:aoeIsEffective(card, to)
	-- the AOE starter is not effected by AOE
	if self.player == to then
		return false
	end

	-- the vine
	local armor = to:getArmor()
	if armor and armor:inherits("Vine") then
		return false
	end

	-- Jiaxu's weimu
	if self.room:isProhibited(self.player, to, card) then
		return false
	end

	-- Yangxiu and Xushu
	if self.player:hasSkill("wuyan") or self.player:hasSkill("danlao") then
		return false
	end
	
	-- Menghuo and Zhurong
	if card:inherits("SavageAssault") then
		if to:hasSkill("huoshou") or to:hasSkill("juxiang") then
			return false
		end
	end

	--Chengong's zhichi
	if (to:hasSkill("zhichi") and self.room:getTag("zhichi"):toString() == to:objectName()) then
		return false
	end

	return true
end

function SmartAI:getDistanceLimit(card)
	if self.player:hasSkill "qicai" then
		return nil
	end

	if card:inherits "Snatch" then
		return 1
	elseif card:inherits "SupplyShortage" then
		if self.player:hasSkill "duanliang" then
			return 2
		else
			return 1
		end
	end
end

function SmartAI:exclude(players, card)
	local excluded = {}
	local limit = self:getDistanceLimit(card)
	for _, player in sgs.list(players) do
		if not self.room:isProhibited(self.player, player, card) then
			local should_insert = true
			if limit then
				should_insert = self.player:distanceTo(player) <= limit
			end

			if should_insert then
				table.insert(excluded, player)
			end
		end
	end

	return excluded
end

function SmartAI:useCardDismantlement(dismantlement, use)
	if self.player:hasSkill("wuyan") then return end
	if (not self.has_wizard) and self:hasWizard(self.enemies) then
		-- find lightning
		local players = self.room:getOtherPlayers(self.player)
		players = self:exclude(players, dismantlement)
		for _, player in ipairs(players) do
			if player:containsTrick("lightning") and not player:hasSkill("wuyan") then
				use.card = dismantlement
                                if use.to then use.to:append(player) end
				return			
			end
		end
	end

	self:sort(self.friends_noself,"defense")
	local friends = self:exclude(self.friends_noself, dismantlement)
	for _, friend in ipairs(friends) do
		if friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage") 
			and not friend:hasSkill("wuyan") then								
			use.card = dismantlement
                        if use.to then use.to:append(friend) end

			return
		end			
	end		
	
	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if getDefense(enemy)<8 then break
		else self:sort(self.enemies,"threat")
		break
		end
	end	
	local enemies = self:exclude(self.enemies, dismantlement)
	for _, enemy in ipairs(enemies) do
		local equips = enemy:getEquips()
		
		--if equips or not (enemy:hasSkill("kongcheng") or enemy:hasSkill("lianying")) then			--not all conditions
		
		    if  not enemy:isNude() and not enemy:hasSkill("wuyan") and					---update
			   (not enemy:hasSkill("xiaoji") or enemy:getEquips():isEmpty()) and not							
			   (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) and not
			   (enemy:hasSkill("lianying") and enemy:getHandcardNum() == 1) and not 
			   (enemy:hasSkill("zhichi") and self.room:getTag("zhichi"):toString() == enemy:objectName()) then                   
				use.card = dismantlement
                if use.to then 
                    use.to:append(enemy) 
                    self:speak("hostile", self.player:getGeneral():isFemale())
                end
				return 
		    end
	--	end
	end
end

-- very similar with SmartAI:useCardDismantlement
function SmartAI:useCardSnatch(snatch, use)
	if self.player:hasSkill("wuyan") then return end
	
    if (not self.has_wizard) and self:hasWizard(self.enemies)  then
		-- find lightning
		local players = self.room:getOtherPlayers(self.player)
		players = self:exclude(players, snatch)
		for _, player in ipairs(players) do
			if player:containsTrick("lightning") and not player:hasSkill("wuyan") then
				use.card = snatch
                                if use.to then use.to:append(player) end
				
				return			
			end			
		end
	end

	self:sort(self.friends_noself,"defense")
	local friends = self:exclude(self.friends_noself, snatch)
	for _, friend in ipairs(friends) do
		if friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage") 
			and not friend:hasSkill("wuyan") then
			use.card = snatch
                        if use.to then use.to:append(friend) end

			return
		end			
	end		
	
	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if getDefense(enemy)<8 then break
		else self:sort(self.enemies,"threat")
		break
		end
	end	
	local enemies = self:exclude(self.enemies, snatch)
	for _, enemy in ipairs(enemies) do
		if  not enemy:isNude() and not enemy:hasSkill("wuyan") and
			   (not enemy:hasSkill("xiaoji") or enemy:getEquips():isEmpty()) and not						--update
			   (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) and not
			   (enemy:hasSkill("lianying") and enemy:getHandcardNum() == 1) and not
			   (enemy:hasSkill("zhichi") and self.room:getTag("zhichi"):toString() == enemy:objectName()) then                   
				use.card = snatch
                if use.to then 
                    use.to:append(enemy) 
                    self:speak("hostile", self.player:getGeneral():isFemale())
                end
				return
		end
	end
end

function SmartAI:useCardFireAttack(fire_attack, use)
	if self.player:hasSkill("wuyan") then return end
	local lack = {
		spade = true,
		club = true,
		heart = true,
		diamond = true,
	}

	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getEffectiveId() ~= fire_attack:getEffectiveId() and not card:inherits("Peach") then					--do not use peach answer the fireattack
			lack[card:getSuitString()] = nil
		end
	end	
	
	if self.player:objectName() == "xiaoqiao" then
		lack["spade"] = true
	end
	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if (self:objectiveLevel(enemy)>3) and not enemy:isKongcheng()  and not enemy:hasSkill("wuyan") then							----no xushu
			if (enemy:hasSkill("zhichi") and self.room:getTag("zhichi"):toString() == enemy:objectName()) then return end
			local cards = enemy:getHandcards()
			local success = true
			for _, card in sgs.qlist(cards) do
				if lack[card:getSuitString()] then
					success = false
					break
				end
			end

			if success then
				use.card = fire_attack
                                if use.to then use.to:append(enemy) end
				return
			end
		end
	end
end

function SmartAI:useCardByClassName(card, use)
	local class_name = card:className()
	local use_func = self["useCard" .. class_name]
	
	if use_func then
		use_func(self, card, use)
	end
end

function SmartAI:getSlashNumber(player)
	local n = 0
	if player:hasSkill("wusheng") then
		local cards = player:getCards("he")
		for _, card in sgs.qlist(cards) do
			if card:isRed() or card:inherits("Slash") and not card:inherits("Peach") then			--no peach
				n = n + 1
			end
		end
	elseif player:hasSkill("wushen") then
		local cards = player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:getSuit() == sgs.Card_Heart or card:inherits("Slash") then
				n = n + 1
			end
		end
	elseif player:hasSkill("jiejiu") then
		local cards = player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:inherits("Analeptic") or card:inherits("Slash") then
				n = n + 1
			end
		end	
	elseif player:hasSkill("longdan") then
		local cards = player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:inherits("Jink") or card:inherits("Slash") then
				n = n + 1
			end
		end
	else
		local cards = player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:inherits("Slash") then
				n = n + 1
			end
		end

		local left = cards:length() - n
		if player:hasWeapon("spear") then
			n = n + math.floor(left/2)
		end
	end

	if player:isLord() and player:hasSkill("jijiang") then
		local lieges = self.room:getLieges("shu", player)
		for _, liege in sgs.qlist(lieges) do
			if liege == "loyalist" then
				n = n + self:getSlashNumber(liege)
			end
		end
	end

	if player:hasSkill("wushuang") then
		n = n * 2
	end

	return n
end

function getJinkNumber(player,self)
    local n = 0

	local cards = player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("Jink") then
			n = n + 1
		end
	end

    if player:hasSkill("wushen") then
        for _, card in sgs.qlist(cards) do
			if card:inherits("Jink") and (card:getSuitString()=="heart") then
				n = n - 1
			end
		end
    end

	if player:hasSkill("longdan") then
		for _, card in sgs.qlist(cards) do
			if card:inherits("Slash") then
				n = n + 1
			end
		end
	elseif player:hasSkill("qingguo") then
		for _, card in sgs.qlist(cards) do
			if card:isBlack() then
				n = n + 1
			end
		end
	end

	local armor = player:getArmor()
	if armor and armor:objectName() == "eight_diagram" then
		local judge_card = self.room:peek()
		if judge_card:isRed() then
			n = n + 1
		end
	end

    if not self then return n end

	if player:isLord() and player:hasSkill("hujia") then
		local lieges = self.room:getLieges("wei",player)
		for _, liege in sgs.qlist(lieges) do
			if liege:getRole() == "loyalist" then
				n = n + self:getJinkNumber(liege)
			end
		end
	end

	return n
end

function SmartAI:getJinkNumber(player)
	return getJinkNumber(player,self)
end

function SmartAI:useCardDuel(duel, use)
	if self.player:hasSkill("wuyan") then return end
	self:sort(self.enemies,"defense")
	local enemies = self:exclude(self.enemies, duel)
	for _, enemy in ipairs(enemies) do
		if self:objectiveLevel(enemy)>3 then
		local n1 = self:getSlashNumber(self.player)
		local n2 = self:getSlashNumber(enemy)

		if n1 >= n2  and not enemy:hasSkill("wuyan") and not
			(enemy:hasSkill("zhichi") and self.room:getTag("zhichi"):toString() == enemy:objectName()) then													--no xushu
			use.card = duel
                if use.to then 
                    use.to:append(enemy) 
                    self:speak("duel", self.player:getGeneral():isFemale())
                end

			return
		end
		end
	end
end

local function handcard_subtract_hp(a, b)
	local diff1 = a:getHandcardNum() - a:getHp()
	local diff2 = b:getHandcardNum() - b:getHp()

	return diff1 < diff2
end

function SmartAI:useCardSupplyShortage(card, use)
	table.sort(self.enemies, handcard_subtract_hp)

	local enemies = self:exclude(self.enemies, card)
	for _, enemy in ipairs(enemies) do
		if ((#enemies==1) or not enemy:hasSkill("tiandu")) and not enemy:containsTrick("supply_shortage") then
			use.card = card
             if use.to then use.to:append(enemy) end

			return
		end
	end
end

local function hp_subtract_handcard(a,b)
	local diff1 = a:getHp() - a:getHandcardNum()
	local diff2 = b:getHp() - b:getHandcardNum()

	return diff1 < diff2
end

function SmartAI:useCardIndulgence(card, use)
	table.sort(self.enemies, hp_subtract_handcard)

	local enemies = self:exclude(self.enemies, card)
	for _, enemy in ipairs(enemies) do
		if not enemy:containsTrick("indulgence") and not enemy:hasSkill("keji") then			
			use.card = card
                        if use.to then use.to:append(enemy) end

			return
		end
	end
end

function SmartAI:useCardCollateral(card, use)
	if self.player:hasSkill("wuyan") then return end
	self:sort(self.enemies,"threat")

--	for _, enemy in ipairs(self.enemies) do
--		if not (self.room:isProhibited(self.player, enemy, card) or enemy:hasSkill("weimu"))
--			and enemy:getWeapon() then

--			for _, enemy2 in ipairs(self.enemies) do
--				if enemy:canSlash(enemy2) then
--					use.card = card
--                                       if use.to then use.to:append(enemy) end
--                                        if use.to then use.to:append(enemy2) end

--					return
--				end
--			end
--		end
--	end
	
	for _, friend in ipairs(self.friends_noself) do												---------------sunshangxiang first
		if friend:getWeapon() and friend:hasSkill("xiaoji") then
			
			for _, enemy in ipairs(self.enemies) do
				if friend:canSlash(enemy) then
					use.card = card
				end
				if use.to then use.to:append(friend) end
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end	

	local n = nil 
	local final_enemy = nil
	for _, enemy in ipairs(self.enemies) do
		if not self.room:isProhibited(self.player, enemy, card) 
			and not enemy:hasSkill("weimu")
			and not enemy:hasSkill("xiaoji")					--update
			and not enemy:hasSkill("wuyan")						
			and enemy:getWeapon() then
			
			for _, enemy2 in ipairs(self.enemies) do
				if enemy:canSlash(enemy2) then 					
					if enemy:getHandcardNum() == 0 then
						use.card = card
						if use.to then use.to:append(enemy) end
						if use.to then use.to:append(enemy2) end
						return
					else
						n = 1;
						final_enemy = enemy2
					end
				end
			end
			if n then use.card = card end
			if use.to then use.to:append(enemy) end
			if use.to then use.to:append(final_enemy) end
			return

		end
		n = nil
	end
end

function SmartAI:useCardIronChain(card, use)
	local targets = {}
	self:sort(self.friends,"defense")
	for _, friend in ipairs(self.friends) do
		if friend:isChained() then
			table.insert(targets, friend)
		end
	end

	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isChained() and not self.room:isProhibited(self.player, enemy, card)  and not enemy:hasSkill("wuyan")  				--same above
			and not enemy:hasSkill("danlao") and not (enemy:hasSkill("zhichi") and self.room:getTag("zhichi"):toString() == enemy:objectName()) 
			and not (self:objectiveLevel(enemy)<=3) then
			table.insert(targets, enemy)
		end
	end

        use.card = card
	
	if targets[2] then
                if use.to then use.to:append(targets[1]) end
                if use.to then use.to:append(targets[2]) end
	end
end

-- the ExNihilo is always used
function SmartAI:useCardExNihilo(card, use)
        use.card = card
        if not use.isDummy then 
                    self:speak("lucky")
        end
end

-- when self has wizard (zhangjiao, simayi, use it)
function SmartAI:useCardLightning(card, use)					
		if self.player:containsTrick("Lightning") then return nil end

        if not self:hasWizard(self.enemies) then--and self.room:isProhibited(self.player, self.player, card) then
            if self:hasWizard(self.friends) then
                use.card = card
                return
            end
            local players=self.room:getAllPlayers()
            players=sgs.QList2Table(players)
            
            local friends=0
            local enemies=0
            
            for _,player in ipairs(players) do
                if self:objectiveLevel(player)>=4 then
                    enemies=enemies+1
                elseif self:isFriend(player) then
                    friends=friends+1
                end
            end
            
            local ratio
            
            if friends==0 then ratio=999
            else ratio=enemies/friends							
            end
            
            if ratio>1.5 then
		        use.card = card
		        return
		    end
	    end
end

function SmartAI:useCardGodSalvation(card, use)				--need change
	local good, bad = 0, 0
	
	if self.player:hasSkill("wuyan") then 						--xushu use it directly
		use.card = card
		return 
	end	
	
	for _, friend in ipairs(self.friends) do if friend:isWounded() then
		
                        good = good + 10/(friend:getHp())
                        if friend:isLord() then good = good + 10/(friend:getHp()) end
		end
	end

	for _, enemy in ipairs(self.enemies) do if enemy:isWounded() then
                bad = bad + 10/(enemy:getHp())
                if enemy:isLord() then bad = bad + 10/(enemy:getHp()) end
		end
	end

	if good > bad then
		use.card = card
	end
end

function SmartAI:useCardAmazingGrace(card, use)
	if #self.friends >= #self.enemies then
		use.card = card
	elseif self.player:hasSkill("wuyan") then
		use.card = card
	end
end

function SmartAI:useTrickCard(card, use)
	if card:inherits("AOE") then
		if self.player:hasSkill("wuyan") then return end
		local good, bad = 0, 0
		for _, friend in ipairs(self.friends_noself) do
			if self:aoeIsEffective(card, friend) then
                                bad = bad + 20/(friend:getHp())+10
                                if friend:isLord() 
                                and (friend:getHp()<3) then return end
                                if (friend:getHp()<2)
                                and (self.player:isLord())
                                then return end
			end
		end

		for _, enemy in ipairs(self.enemies) do
			if self:aoeIsEffective(card, enemy) then
                                good = good + 20/(enemy:getHp())+10
                                if enemy:isLord() then good = good + 20/(enemy:getHp()) end
			end
		end

		if good > bad then
			use.card = card
		end
	else
		self:useCardByClassName(card, use)
	end
end


sgs.weapon_range =
{	
	Crossbow = 1,
	Blade = 3,
	Spear = 3,
	DoubleSword =2,
	QinggangSword=2,
	Axe=3,
	KylinBow=5,
	Halberd=4,
	IceSword=2,
	Fan=4,
	MoonSpear=3,
	GudingBlade=2,
	
}

function SmartAI:evaluateEquip(card)

		local deltaSelfThreat = 0
		local currentRange 
                if not card then return -1
                else
                currentRange = sgs.weapon_range[card:className()] or 0
		end
		for _,enemy in ipairs(self.enemies) do
			if self.player:distanceTo(enemy) <= currentRange then
					deltaSelfThreat=deltaSelfThreat+6/getDefense(enemy)
			end
		end
		
		if card:inherits("Crossbow") and deltaSelfThreat~=0 then 
		    if self.player:hasSkill("kurou") then deltaSelfThreat=deltaSelfThreat*3+10 end
			deltaSelfThreat = deltaSelfThreat + self:getSlashNumber(self.player)*3-2
		elseif card:inherits("Blade") then 
			deltaSelfThreat = deltaSelfThreat + self:getSlashNumber(self.player)
		elseif card:inherits("Spear") then--and 
			--self.player:getHandcardNum()/2 - self:getSlashNumber(self.player)>0 then 
				--deltaSelfThreat = deltaSelfThreat + self.player:getHandcardNum()/2 - self:getSlashNumber(self.player)
		else
			for _,enemy in ipairs(self.enemies) do
				if self.player:distanceTo(enemy) <= currentRange then
					if card:inherits("DoubleSword") and 
						enemy:getGeneral():isMale() ~= self.player:getGeneral():isMale() then
							deltaSelfThreat=deltaSelfThreat+3
					elseif card:inherits("QinggangSword") and enemy:getArmor() then
						deltaSelfThreat=deltaSelfThreat+3
					elseif card:inherits("Axe") and enemy:getHp()<3 then
						deltaSelfThreat=deltaSelfThreat+3-enemy:getHp()
					elseif card:inherits("KylinBow") and (enemy:getDefensiveHorse() or enemy:getDefensiveHorse())then
						deltaSelfThreat=deltaSelfThreat+1
						break
					elseif card:inherits("GudingBlade") and enemy:getHandcardNum()<3 then
						deltaSelfThreat=deltaSelfThreat+2
						if enemy:getHandcardNum()<1 then deltaSelfThreat=deltaSelfThreat+4 end
					end
				end
			end
		end
		return deltaSelfThreat
end

function SmartAI:useEquipCard(card, use)
	
	if card:inherits("Weapon") then
		if self:evaluateEquip(card) > (self:evaluateEquip(self.player:getWeapon())) then
		if use.isDummy and self.weaponUsed then return end
		if self.player:getHandcardNum()<=self.player:getHp() then return end
		use.card = card		
		end
	elseif card:inherits("Armor") then
	    if self.player:hasSkill("bazhen") then return end
	 	if not self.player:getArmor() then use.card=card
	 	elseif (self.player:getArmor():objectName())=="silver_lion" then use.card=card
	 	elseif self.player:isChained()  and (self.player:getArmor():inherits("vine")) and not (card:objectName()=="silver_lion") then use.card=card
	 	elseif self.player:hasSkill("leiji") or self.player:hasSkill("tiandu") then use.card=card
	 	end
	elseif self.lua_ai:useCard(card) then
		use.card = card
	end
end

function SmartAI:getTurnUse()				
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)
    
    
    
    local turnUse={}
    local slashAvail=1
    self.predictedRange=self.player:getAttackRange()
    self.predictNewHorse=false
    self.retain_thresh=5
    self.slash_targets=1
    self.slash_distance_limit=false
    
    self.weaponUsed=false
    
    if self.player:isLord() then self.retain_thresh=6 end
    if self.player:hasFlag("tianyi_success") then 
        slashAvail=2 
        self.slash_targets=2
        self.slash_distance_limit=true
    end
    
    
    self:fillSkillCards(cards)
    
    self:sortByUseValue(cards)
    
    if self.player:hasSkill("paoxiao") or 
        (
            self.player:getWeapon() and 
            (self.player:getWeapon():objectName()=="crossbow")
        ) then
        slashAvail=100     										
    end
    
            
    local i=0
    --self.room:output(#cards)
    for _,card in ipairs(cards) do
        local dummy_use={}
        dummy_use.isDummy=true
        if (not self.player:hasSkill("kongcheng")) and (not self.player:hasSkill("lianying")) then
            if (i >= (self.player:getHandcardNum()-self.player:getHp()+self.retain)) and (self:getUseValue(card)<self.retain_thresh) then
                --if self.room:getCurrent():objectName()==self.player:objectName() then self:log(card:className()..self:getUseValue(card)) end
                return turnUse
            end
        
            if (i >= (self.player:getHandcardNum()-self.player:getHp())) and (self:getUseValue(card)<8.5) and self.harsh_retain then
                --if self.room:getCurrent():objectName()==self.player:objectName() then self:log(card:className()..self:getUseValue(card)) end
                return turnUse
            end
        end
        
        local type = card:getTypeId()
        if type == sgs.Card_Basic then
            self:useBasicCard(card, dummy_use, self.slash_distance_limit)
        elseif type == sgs.Card_Trick then
            self:useTrickCard(card, dummy_use)
        elseif type == sgs.Card_Equip then
            self:useEquipCard(card, dummy_use)
        elseif type == sgs.Card_Skill then
            self:useSkillCard(card, dummy_use)
        end


        if dummy_use.card then
            if (card:inherits("Slash")) then 
                if slashAvail>0 then
                    slashAvail=slashAvail-1
                    table.insert(turnUse,card)
                    
                end
            else
                if card:inherits("Weapon") then 
                    self.predictedRange=sgs.weapon_range[card:className()] 
                    self.weaponUsed=true
                end
                if card:inherits("OffensiveHorse") then self.predictNewHorse=true end
                if card:objectName()=="crossbow" then slashAvail=100 end
                if card:inherits("Snatch") then i=i-1 end
                if card:inherits("Peach") then i=i+2 end
                if card:inherits("Collateral") then i=i-1 end
                if card:inherits("AmazingGrace") then i=i-1 end
                if card:inherits("ExNihilo") then i=i-2 end
                table.insert(turnUse,card)
            end
            i=i+1
        else
--            self.room:output(card:className().." unused")
        end
    end
--    self.room:output(self.player:getGeneralName()..i)
    return turnUse
end

function SmartAI:activate(use)
        --self.room:speakCommand(self.room,self.player,"yooo")
		--self:printAssume()
        --local moves=self:getMoves()
        --self:printMoves(moves)
        --if self.player:getHandcardNum()<self.player:getHp() then return end
        --self.room:output(getCount(self.player:objectName()))
        self:updatePlayers()
        self:assignKeep(self.player:getHp(),true)
        self:printCards(self.kept)
        self.toUse =self:getTurnUse()
        self:printCards(self.toUse)
        if self.harsh_retain then self:log("harsh_retaining") end
        --self:printFEList()
        --local cards = self.player:getHandcards()
        --cards=sgs.QList2Table(cards)
        --self:sortByUsePriority(cards)
        --self.room:output("usesize"..#self.toUse)

        self:sortByUsePriority(self.toUse)
        for _, card in ipairs(self.toUse) do

			local type = card:getTypeId()

			if type == sgs.Card_Basic then
				self:useBasicCard(card, use, self.slash_distance_limit)
			elseif type == sgs.Card_Trick then
				self:useTrickCard(card, use)
		    elseif type == sgs.Card_Skill then
                self:useSkillCard(card, use)
			else
				self:useEquipCard(card, use)
			end
                        if use:isValid() then
--                        self.room:output("card Used")
                self.toUse=nil
				return
                        else
                            self.room:output("invalidUseCard")
                        end
                end
        self.toUse=nil
        
end

function SmartAI:hasEquip(card)
    local equips=self.player:getEquips()
    if not equips then return false end
    for _,equip in sgs.qlist(equips) do
        if equip:getId()==card:getId() then return true end
    end
    return false
end

function SmartAI:getEquipsNumber()																	-----no use
	local equips = self.player:getEquips()
	if not equips then 
		return 0
	end
	local index = 0
	for _, equip in sgs.qlist(equips) do
		index = index + 1
	end
	return index
end	
	

function SmartAI:getKeepValue(card,kept)
    if not kept then return self.keepValue[card:getId()] or 0 end
    
	local class_name = card:className()		
	local suit_string = card:getSuitString()
    local value	
    if sgs[self.player:getGeneralName().."_keep_value"] then							
        value=sgs[self.player:getGeneralName().."_keep_value"][class_name]
	elseif sgs[self.player:getGeneralName().."_suit_value"] then
		value=sgs[self.player:getGeneralName().."_suit_value"][suit_string]
	end
	if not value then 
		value = sgs.ai_keep_value[class_name] or 0
		for _,acard in ipairs(kept) do
			if acard:className()==card:className() then value=value-1.2
			elseif acard:inherits("Slash") and card:inherits("Slash") then value=value-1 
			end
		end
	end
    return value
end

function SmartAI:getUseValue(card)
        local class_name = card:className()				
        local v=0
        
        if card:inherits("EquipCard") then 
            if self:hasEquip(card) then return 9 end
			if self:hasSkill("xiaoji") or self:hasSkill("xuanfeng") then return 9 end
			if self:hasSkill("kurou") and card:inherits("Crossbow") then return 9 end
            if card:inherits("Armor") and not self.player:getArmor() then v = 8.9
            elseif card:inherits("Weapon") and not self.player:getWeapon() then v = 6.2
            elseif card:inherits("DefensiveHorse") and not self.player:getDefensiveHorse() then v = 5.8
            elseif card:inherits("OffensiveHorse") and not self.player:getOffensiveHorse() then v = 5.5
            elseif self:hasSkill("bazhen") and card:inherits("Armor") then v=2
            else v = 2 end
            if self.weaponUsed and card:inherits("Weapon") then v=2 end
			if self.player:hasSkill("qiangxi") and card:inherits("Weapon") then v = 2 end
        else
            if card:inherits("Slash") and (self.player:hasFlag("drank") or self.player:hasFlag("tianyi_success") or self.player:hasFlag("luoyi")) then v = 8.7 --self:log("must slash")
            elseif self.player:getWeapon() and card:inherits("Collateral") then v=2
            elseif self.player:getMark("shuangxiong") and card:inherits("Duel") then v=8
			elseif self.player:hasSkill("jujian") and (card:inherits("AmazingGrace") or card:inherits("GodSalvation")) then v =10
			elseif self.player:hasSkill("jujian") and 
					(card:inherits("SavageAssault") or card:inherits("ArcheryAttack") or card:inherits("FireAttack") or card:inherits("Duel") or
					card:inherits("Dismantlement") or card:inherits("Snatch") or card:inherits("Collateral") or card:inherits("IronChain")) then
				v=4
			elseif self.player:hasSkill("jizhi") and card:getTypeId() == sgs.Card_Trick then
				v=8.7
            else v = sgs.ai_use_value[class_name] or 0 end
            
        end
        --if self.room:getCurrent():objectName()==self.player:objectName() then self:log(class_name..v) end
        if card:inherits("Slash") and (self:getSlashNumber(self.player)>1) then v=v+1 end
        if card:inherits("Jink") and (self:getJinkNumber(self.player)>1) then v=v-6 end
        
        if self.player:hasSkill("lianying") or self.player:hasSkill("kongcheng") then
            if self.player:getHandcardNum()==1 then v = 10 end
        end
        if self:hasSkill({name="halberd"}) and card:inherits("Slash") and self.player:getHandcardNum()==1 then v=10 end
        if card:getTypeId()==sgs.Card_Skill then 
            if v==0 then v=10 end
        end
        
        return v
end

function SmartAI:getUsePriority(card)
	local class_name = card:className()
	local v=0
	if card:inherits("EquipCard") then
		if self.player:hasSkill("xiaoji") then return 10 end
	    v=1
        if card:inherits("Armor") and not self.player:getArmor() then v = 6
        elseif card:inherits("Weapon") and not self.player:getWeapon() then v = 5.7
        elseif card:inherits("DefensiveHorse") and not self.player:getDefensiveHorse() then v = 5.8
        elseif card:inherits("OffensiveHorse") and not self.player:getOffensiveHorse() then v = 5.5
        end
        return v
    end
		if self.player:hasSkill("wuyan") then
			if card:inherits("Slash") then 
                v = 4
                
			elseif card:inherits("Collateral") or card:inherits("Dismantlement") or card:inherits("Snatch") or card:inherits("IronChain") then v = 1.0
			end
			if v then return v else return sgs.ai_use_priority[class_name] end
		end
		if self.player:hasSkill("qingnang") then
			if card:inherits("Dismantlement") then v = 3.8
			elseif card:inherits("Collateral") then v = 3.9
			end
			if v then return v else return sgs.ai_use_priority[class_name] end
		end	
		if self.player:hasSkill("rende") then
			if card:inherits("ExNihio") then v = 5.9 end
			return v or sgs.ai_use_priority[class_name]
		end
		
		
        v = sgs.ai_use_priority[class_name] or 0
        
        
        if card:inherits("Slash") and (card:getSuit()==sgs.Card_NoSuit) then v=v-0.1 end
        return v
end

function SmartAI:sortByKeepValue(cards,inverse,kept)
	local compare_func = function(a,b)
		local value1 = self:getKeepValue(a,kept)
		local value2 = self:getKeepValue(b,kept)

		if value1 ~= value2 then
		    if inverse then return value1>value2 end
			return value1 < value2
		else
			return a:getNumber() < b:getNumber()
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:sortByUseValue(cards,inverse)
        local compare_func = function(a,b)
                local value1 = self:getUseValue(a)
                local value2 = self:getUseValue(b)

                if value1 ~= value2 then
                        if not inverse then return value1 > value2
                        else return value1 < value2
                        end
                else
                        return a:getNumber() > b:getNumber()
                end
        end

        table.sort(cards, compare_func)
end

function SmartAI:sortByUsePriority(cards)
	local compare_func = function(a,b)
                local value1 = self:getUsePriority(a)
                local value2 = self:getUsePriority(b)

		if value1 ~= value2 then
			return value1 > value2
		else
			return a:getNumber() > b:getNumber()
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:sortByCardNeed(cards)
	local compare_func = function(a,b)
                local value1 = self:cardNeed(a)
                local value2 = self:cardNeed(b)

		if value1 ~= value2 then
			return value1 < value2
		else
			return a:getNumber() > b:getNumber()
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:askForDiscard(reason, discard_num, optional, include_equip)
    
    if reason=="ganglie" then
        if self.player:getHp()>self.player:getHandcardNum() then return {} end
		
		if self.player:getHandcardNum() == 3 then							--add the plan of ganglie's discard
			local to_discard = {}
			local cards = self.player:getHandcards()
			local index = 0
			local all_peaches = 0
			for _, card in sgs.qlist(cards) do
				if card:inherits("Peach") then 
					all_peaches = all_peaches + 1
				end
			end
			if all_peaches >= 2 then return {} end
			
			for _, card in sgs.qlist(cards) do
				if not card:inherits("Peach") then
					table.insert(to_discard, card:getEffectiveId())
					index = index + 1
					if index == 2 then break end
				end
			end
			return to_discard
		end
		
        if self.player:getHandcardNum()<2 then return {} end 
	elseif optional then
		return {}
	end
	
		local flags = "h"
		if include_equip then
			flags = flags .. "e"
		end

		local cards = self.player:getCards(flags)
		cards = sgs.QList2Table(cards)
		self:sortByKeepValue(cards)
		local to_discard = {}
		for i=1, discard_num do
			table.insert(to_discard, cards[i]:getEffectiveId())
		end

		return to_discard
	
end

function SmartAI:getRetrialCard(flags,cardSet,reversed)
    local cards=self.player:getCards(flags)
    cards=sgs.QList2Table(cards)
    self:sortByUseValue(cards,true)
    self.room:output("looking for card")

    for _, card in ipairs(cards) do
    
        local result=card:getSuitString()
        local number=card:getNumber()
        
        if (cardSet[result][number]) and not reversed then
            return card:getEffectiveId()
        end
        
        if (not cardSet[result][number]) and reversed then
            return card:getEffectiveId()
        end
    end
    self.room:output("unfound.")
    return "."
end


function SmartAI:askForPlayerChosen(targets, reason)
	self:log("askForPlayerChosen:"..reason)
	local playerchosen = sgs.ai_skill_playerchosen[string.gsub(reason,"%-","_")]
	local target
	if type(playerchosen) == "function" then
		target=playerchosen(self,targets)
	end
	if target then
		return target
	else
		local r = math.random(0, targets:length() - 1)
		return targets:at(r)
	end
end

-- used for SmartAI:askForChoice
sgs.ai_skill_choice = {}

function SmartAI:askForChoice(skill_name, choices)
	local choice = sgs.ai_skill_choice[skill_name]
	if type(choice) == "string" then
		return choice
	elseif type(choice) == "function" then
		return choice(self, choices)
	else
		local skill = sgs.Sanguosha:getSkill(skill_name)
		if skill then return skill:getDefaultChoice() end
	end		
	return nil
end

function SmartAI:getCardRandomly(who, flags)
	local cards = who:getCards(flags)
	local r = math.random(0, cards:length()-1)
	local card = cards:at(r)
	return card:getEffectiveId()
end

function SmartAI:askForCardChosen(who, flags, reason)

    if self:isFriend(who) then
		if flags:match("j") then
			local tricks = who:getCards("j")

			local lightning, indulgence, supply_shortage
			for _, trick in sgs.qlist(tricks) do
				if trick:inherits "Lightning" then
					lightning = trick:getId()
				elseif trick:inherits "Indulgence" or trick:getSuit() == sgs.Card_Diamond then
					indulgence = trick:getId()
				else
					supply_shortage = trick:getId()
				end
			end
			

			if self:hasWizard(self.enemies) and lightning then
				return lightning
			end

			if indulgence and supply_shortage then
				if who:getHp() < who:getHandcardNum() then
					return indulgence
				else
					return supply_shortage
				end
			end

			if indulgence or supply_shortage then
				return indulgence or supply_shortage
			end
		elseif flags:match("e") and who:hasSkill("xiaoji") then
			local equips = who:getEquips()
			if not equips:isEmpty() then
				return equips:at(0):getId()
			end
		end
	else
        if (who:getHandcardNum()<2) and (not who:isKongcheng()) and
         not (who:hasSkill("lianying") or who:hasSkill("kongcheng")) then return -1 
		
		elseif flags:match("e") then
		    
			if who:getDefensiveHorse() then
				for _,friend in ipairs(self.friends) do
					if friend:distanceTo(who)==friend:getAttackRange()+1 then 
					 	return who:getDefensiveHorse():getId()
					end
				end
			end
			
		
			if who:getOffensiveHorse() then
			    if who:hasSkill("xiaoji") and who:getHandcardNum()>=who:getHp() then
			    else
				    for _,friend in ipairs(self.friends) do
					    if who:distanceTo(friend)==who:getAttackRange() and
					    who:getAttackRange()>1 then 
					 	    return who:getOffensiveHorse():getId() 
					    end
				    end
				end
			end
			
			if who:getArmor() then 
			    local canFire=false
			        
			        if self.player:getWeapon() then 
			            if self.player:getWeapon():inherits("Fan") then canFire=true end
			        end
			    if self.toUse then
			        for _,card in ipairs(self.toUse) do 
			            if card:inherits("FireSlash") then canFire=true end
			            if card:inherits("FireAttack") then canFire=true end
			        end
			    end
			    if canFire and (who:getArmor():objectName()=="vine") then 
				elseif (who:getArmor():objectName()=="silver_lion") and who:isWounded() then 
                else return who:getArmor():getId() 
                end
			end
			
			if who:getWeapon() then 
			    if not (who:hasSkill("xiaoji") and (who:getHandcardNum()>=who:getHp())) then
				for _,friend in ipairs(self.friends) do
					if (who:distanceTo(friend) <= who:getAttackRange()) and (who:distanceTo(friend)>1) then 
					 	return who:getWeapon():getId()
					end
				end
				end
			end
		end
        
        if not who:isKongcheng() then
			return -1
		end
	end
    self:log("??????")
	local new_flag=""
    if flags:match("h") then new_flag="h" end
    if flags:match("e") then new_flag=new_flag.."e" end
    return self:getCardRandomly(who, new_flag)							---wtf? no return?
end

function SmartAI:askForCard(pattern,prompt)
        self.room:output(prompt)
        if sgs.ai_skill_invoke[pattern] then return sgs.ai_skill_invoke[pattern](self,prompt) end

        if not prompt then return end
        local parsedPrompt=prompt:split(":")

        if parsedPrompt[1]=="collateral-slash" then 
			local target 
			local others = self.room:getOtherPlayers(self.player)
			others = sgs.QList2Table(others)
			for _, other in ipairs(others) do
				if other:objectName() == parsedPrompt[3] then target = other break end
			end
			if target and (target:getHp() > 2 or self:getJinkNumber(target) > 0) and self:getSlashNumber(self.player) > 0 and not self.player:hasSkill("xiaoji") then 
				local cards = self.player:getHandcards()
				for _, card in sgs.qlist(cards) do
					if card:inherits("Slash") or card:inherits("FireSlash") or card:inherits("ThunderSlash") then 
						return card
					elseif self.player:hasSkill("wushen") then 
						if card:getTypeId() == sgs.Card_Heart then return card end
					elseif self.player:hasSkill("jiejiu") then 
						if card:inherits("Analeptic") then return card end 
					end
				end
			end
			self:speak("collateral", self.player:getGeneral():isFemale())
			return "."
        elseif (parsedPrompt[1]=="@jijiang-slash") then
            if self:isFriend(self.room:getLord()) then 
				if self:getSlashNumber(self.player) > 0 then self:speak("jijiang", self.player:getGeneral():isFemale()) end 
				return self:getSlash()
            else return "." end
        elseif parsedPrompt[1]=="double-sword-card" then 							--update the double sword
			local cards = self.player:getHandcards()
			for _, card in sgs.qlist(cards) do
				if card:inherits("Slash") or card:inherits("ThunderSlash") or card:inherits("FireSlash") or card:inherits("Shit") or
				   card:inherits("Lightning") or card:inherits("EquipCard") or card:inherits("AmazingGrace") then
					return card
				end
			end	
			return "."
	    elseif parsedPrompt[1]=="axe-card" then
			if self.player:hasFlag("drank") or self.player:hasSkill("lianying") or self.player:hasSkill("kongcheng")
				or (self.player:hasSkill("kuanggu") and self.player:isWounded()) then
				local cards = self.player:getCards("h")
	        cards=sgs.QList2Table(cards)
				local index
				if (self.player:hasSkill("lianying") or self.player:hasSkill("kongcheng")) then index = #cards end
				if self.player:getOffensiveHorse() then 
					if index then 
						if index < 2 then 
							index = index + 1 
							table.insert(cards, self.player:getOffensiveHorse()) 
						end 
					end
					table.insert(cards, self.player:getOffensiveHorse()) 
				end
				if self.player:getArmor() then 
					if index then 
						if index < 2 then 
							index = index + 1 
							table.insert(cards, self.player:getArmor()) 
						end 
					end
					table.insert(cards, self.player:getArmor()) 
				end
				if self.player:getDefensiveHorse() then 
					if index then 
						if index < 2 then 
							index = index + 1 
							table.insert(cards, self.player:getDefensiveHorse()) 
						end 
					end
					table.insert(cards, self.player:getDefensiveHorse()) 
				end
	        if #cards>=2 then
	            self:sortByUseValue(cards,true)
	            return "@AxeCard="..cards[1]:getEffectiveId().."+"..cards[2]:getEffectiveId()
	        end
			end
        elseif parsedPrompt[1]=="@wushuang-slash-1" and (self:getSlashNumber(self.player)<2)then
            return "."
        elseif (parsedPrompt[1]=="@wushuang-jink-1") and (self:getJinkNumber(self.player)<2) then return "." 
        elseif (parsedPrompt[1]=="@roulin1-jink-1") and (self:getJinkNumber(self.player)<2) then return "." 
        elseif (parsedPrompt[1]=="@roulin2-jink-1") and (self:getJinkNumber(self.player)<2) then return "." end
		
        if self.player:hasSkill("yiji") and (self.player:getLostHp()<1) then 
			if (parsedPrompt[1]=="slash-jink") then return "." end		
		end																	--guojia doesn't use jink when not wounded 
		if self.player:hasSkill("tianxiang") then							--tianxiang 
			local dmgStr={damage=1,nature=0}
			local willTianxiang=sgs.ai_skill_use["@tianxiang"](self, dmgStr)
			if willTianxiang~="." then return "." end
		end	
		

	return nil
end

function SmartAI:askForAG(card_ids,refusable)
    local ids=card_ids
    local cards={}
    for _,id in ipairs(ids) do
        table.insert(cards,sgs.Sanguosha:getCard(id))
    end
    self:sortByCardNeed(cards)
    return cards[#cards]:getEffectiveId()
end


function SmartAI:askForNullification(trick_name, from, to, positive)   							-----add nullification

	local cards = self.player:getCards("h")
	local null_card
		for _, card in sgs.qlist(cards) do
			if card:inherits("Nullification") then 
				null_card = card
				break
			end				
			if card:isBlack() and self.player:hasSkill("kanpo") then
			    local suit = card:getSuitString()
		        local number = card:getNumberString()
		        local card_id = card:getEffectiveId()
		        local card_str = ("nullification[%s:%s]=%d"):format(suit, number, card_id)
		        null_card = sgs.Card_Parse(card_str)				
				break
			end
		end
    if not null_card then return nil end
    
	if positive then
		
			if from and self:isEnemy(from) then


				if trick_name:inherits("ExNihilo") or trick_name:inherits("Collateral")
					or trick_name:inherits("Snatch") or trick_name:inherits("Dismantlement") 
					or trick_name:inherits("FireAttack") then 
						return null_card
				end
			end	
	
			if self:isEnemy(to) then
				if trick_name:inherits("GodSalvation") then
					return null_card
				end
			end	
	
			if self:isFriend(to) then
				if trick_name:inherits("Indulgence") or trick_name:inherits("SupplyShortage") then
					return null_card
				end
				if trick_name:inherits("Duel") and not(trick_name:getSuit() == sgs.Card_NoSuit) then
					return null_card
				end
				--if to:isLord() then return null_card end				--add codes
			end
		
	else
	    local reverse_null=self:askForNullification(trick_name, from, to, true)
		if null_card and not reverse_null then return null_card end
	end
	
	return nil
end

function SmartAI:askForSinglePeach(player, dying)										--add ask for peach
	local cards = self.player:getCards("he")
	
	if self:isFriend(dying) and dying:isLord() then
		for _, card in sgs.qlist(cards) do
			if card:inherits("Peach") then
				if not (card:getSuit() == sgs.Card_Heart and self.player:hasSkill("wushen")) then
					return card
				end
			elseif card:isRed() and self.player:hasSkill("jijiu") then
				return card
			end
		end
	end	
	
	if self:isFriend(dying) then
		for _, card in sgs.qlist(cards) do
			if (self.player:objectName() == dying:objectName()) then
				if card:inherits("Analeptic") and not self.player:hasSkill("jiejiu") then
					return card
				elseif card:getSuit() == sgs.Card_Spade and self.player:hasSkill("jiuchi") then
					local suit = card:getSuitString()
					local number = card:getNumberString()
					local card_id = card:getEffectiveId()
					local card_str = ("analeptic[%s:%s]=%d"):format(suit, number, card_id)
					local anal_card = sgs.Card_Parse(card_str)				
					return anal_card
				end
			else
				if card:inherits("Peach") then
					if not (card:getSuit() == sgs.Card_Heart and self.player:hasSkill("wushen")) then
						return card
					end
				elseif card:isRed() and self.player:hasSkill("jijiu") then
					return card
				end
			end
		end
	end
	
	return nil
end

function SmartAI:getOneFriend()
	for _, friend in ipairs(self.friends) do
		if friend ~= self.player then
			return friend
		end
	end
end

function SmartAI:getChainedFriends()
	local chainedFriends={}
	for _, friend in ipairs(self.friends) do
		if friend:isChained() then
			table.insert(chainedFriends,friend)
		end
	end
	return chainedFriends
end

function SmartAI:getChainedEnemies()
	local chainedEnemies={}
	for _, enemy in ipairs(self.enemies) do
		if enemy:isChained() then
			table.insert(chainedEnemies,enemy)
		end
	end
	return chainedEnemies
end

function SmartAI.newSubclass(theClass, name)
	local class_name = name:sub(1, 1):upper() .. name:sub(2) .. "AI"
	local new_class = class(class_name, theClass)

	function new_class:initialize(player)
		super.initialize(self, player)
	end

	sgs.ai_classes[name] = new_class

	return new_class
end

function SmartAI:setOnceSkill(name)
	function self:filterEvent(event, player, data)
		super.filterEvent(self, event, player, data)
		if not player then return end
		if event == sgs.PhaseChange and player:objectName() == self.player:objectName()
			and player:getPhase() == sgs.Player_Play then
			self[name .. "_used"] = false
                        self.toUse=nil
		end
	end
end

function SmartAI:hasSkill(skill)
    if (skill.name=="huangtianv") then
        return (self.player:getKingdom()=="qun") and (self.room:getLord():hasSkill("huangtian") and not self.player:isLord())			---????
    elseif (skill.name=="jijiang") then
        return (self.player:isLord() and self.player:hasSkill(skill.name))
    else
        return self.player:hasSkill(skill.name)
    end
end

function SmartAI:fillSkillCards(cards)
    for _,skill in ipairs(sgs.ai_skills) do

        if self:hasSkill(skill) then    
            
            if skill.name=="wushen" then 
                for i=#cards,1,-1 do 
                    
                    if cards[i]:getSuitString()=="heart" then
                        self:log("cant use "..cards[i]:className()..i)
                        table.remove(cards,i)
                    end
                end
            end
            
			if skill.name == "ganran" then 				---add
				 for i=#cards,1,-1 do 
                    
                    if cards[i]:inherits("EquipCard") then
                        self:log("cant use "..cards[i]:className()..i)
                        table.remove(cards,i)
                    end
                end
			end	
			
			if skill.name == "jiejiu" then 				---add
				 for i=#cards,1,-1 do 
                    
                    if cards[i]:inherits("Analeptic") then
                        self:log("cant use "..cards[i]:className()..i)
                        table.remove(cards,i)
                    end
                end
			end	
			
            local card=skill.getTurnUseCard(self)
            if #cards==0 then card=skill.getTurnUseCard(self,true) end
            if card then table.insert(cards,card) end
            --self:printCards(cards)
            
        end
    end
end

function SmartAI:useSkillCard(card,use)
    --self:log(card:className())
    sgs.ai_skill_use_func[card:className()](card,use,self)
end
sgs.ai_skill_use_func={}
sgs.ai_skills={}

function SmartAI:cardNeed(card)
	local class_name = card:className()
	local suit_string = card:getSuitString()
    local value	
    if sgs[self.player:getGeneralName().."_keep_value"] then							
        value=sgs[self.player:getGeneralName().."_keep_value"][class_name]
		if value then return value+4 end
	end
	if sgs[self.player:getGeneralName().."_suit_value"] then
		value=(sgs[self.player:getGeneralName().."_suit_value"][suit_string])
		if value then return value+4 end
	end
	
    if card:inherits("Jink") and (self:getJinkNumber(self.player)==0) then return 5.9 end
    if card:inherits("Peach") then
        self:sort(self.friends,"hp")
        if self.friends[1]:getHp()<2 then return 10 end
        return self:getUseValue(card)
    end
	
    if card:inherits("Analeptic") then
        if self.player:getHp()<2 then return 10 end
    end
	
	
	  
	
    if card:inherits("Slash") and (self:getSlashNumber(self.player)>0) then return 4 end
    if card:inherits("Weapon") and (not self.player:getWeapon()) and (self:getSlashNumber(self.player)>1) then return 6 end
    return self:getUseValue(card)
end

function SmartAI:cardMatch(condition)

end

-- load other ai scripts
dofile "lua/ai/standard-ai.lua"
dofile "lua/ai/wind-ai.lua"
dofile "lua/ai/fire-ai.lua"
dofile "lua/ai/thicket-ai.lua"
dofile "lua/ai/god-ai.lua"
dofile "lua/ai/yitian-ai.lua"
dofile "lua/ai/nostalgia-ai.lua"
dofile "lua/ai/yjcm-ai.lua"					

dofile "lua/ai/general_config.lua"
dofile "lua/ai/intention-ai.lua"
dofile "lua/ai/state-ai.lua"
dofile "lua/ai/playrule-ai.lua"
dofile "lua/ai/chat-ai.lua"
dofile "lua/ai/value_config.lua"

dofile "lua/ai/standard-skill-ai.lua"
dofile "lua/ai/thicket-skill-ai.lua"
dofile "lua/ai/fire-skill-ai.lua"
dofile "lua/ai/yjcm-skill-ai.lua"

dofile "lua/ai/fancheng-ai.lua"