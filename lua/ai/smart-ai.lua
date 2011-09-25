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
		return SmartAI.GetValue(a) < SmartAI.GetValue(b)
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
		return SmartAI.GetDefense(a) < SmartAI.GetDefense(b)
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

--- this function is only function that exposed to the host program
--- and it clones an AI instance by general name
-- @param player The ServerPlayer object that want to create the AI object
-- @return The AI object
function CloneAI(player)
	local ai_class = sgs.ai_classes[player:getGeneralName()]
	if ai_class then
		return ai_class(player).lua_ai
	else
		return SmartAI(player).lua_ai
	end
end

--- FIXME: ?
function getCount(name)
	if sgs.ai_round[name] then 
                sgs.ai_round[name]=sgs.ai_round[name]+1
	else 
		sgs.ai_round[name]=1 
	end
        return sgs.ai_round[name]
end

-- SmartAI is the base class for all other specialized AI classes
SmartAI = class "SmartAI"
super = SmartAI

--- Calculate the value for a player, 1 hp = 2 handcard
-- @param player The ServerPlayer object 
-- @return Its value
function SmartAI.GetValue(player)
	return player:getHp() * 2 + player:getHandcardNum()
end

--- defense is defined as min(value, hp*3) + 2(if armor is present)
function SmartAI.GetDefense(player)
	local defense = math.min(SmartAI.GetValue(player), player:getHp() * 3)
	if player:getArmor() then
		defense = defense + 2
	end
	
	return defense
end

getDefense = SmartAI.GetDefense

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
			local success, result1, result2 
			success, result1, result2 = pcall(method, self, ...) 
			if not success then 
				self.room:writeToConsole(result1) 
				self.room:writeToConsole(debug.traceback()) 
			else 
				return result1, result2 
			end  
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
    return useDefaultStrategy() or sgs.GetConfig("RolePredictable", true)	
end

function useDefaultStrategy()
	local mode=sgs.GetConfig("GameMode", "")
    if (mode=="06_3v3") or (not mode:find("0")) then return true end
    if (mode:find("02_1v1") or mode:find("03p")) then return true end
end

-- this function create 2 tables contains the friends and enemies, respectively
function SmartAI:updatePlayers(inclusive)
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
			self:updateRebelTarget(player)
			
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
        elseif (sgs.ai_chaofeng[player:getGeneralName()] or 0)<(sgs.ai_chaofeng[sgs.loyal_target:getGeneralName()] or 0) then sgs.loyal_target=player 
        elseif (sgs.loyal_target:getHp()>1) and (getDefense(player)<=3) then sgs.loyal_target=player 
        elseif (sgs.loyal_target:getHandcardNum()>0) and (player:getHandcardNum()==0) then sgs.loyal_target=player 
        elseif (sgs.loyal_target:getArmor()) and (not player:getArmor()) then sgs.loyal_target=player 
        end
    end
end

function SmartAI:updateRebelTarget(player)
	if self.role == "lord" or self.role == "loyalist" then return end
	if not sgs.rebel_target then sgs.rebel_target=player end
	if self.room:getLord():objectName() == player:objectName() then sgs.rebel_target=player
	elseif self:objectiveLevel(player)>=4 and self:objectiveLevel(player)<5 then
		if player:getHp() == 1 and sgs.rebel_target:getHp() > 2 then sgs.rebel_target=player
		elseif (sgs.rebel_target:getArmor()) and (not player:getArmor() and sgs.rebel_target:getHp()>player:getHp()) then sgs.rebel_target=player 
		elseif sgs.rebel_target:getHp()-player:getHp()>2 then sgs.rebel_target=player
        elseif (sgs.ai_chaofeng[player:getGeneralName()] or 0)<(sgs.ai_chaofeng[sgs.rebel_target:getGeneralName()] or 0) then sgs.rebel_target=player 
		end
	end
end

function SmartAI:printFEList()
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
	
	
    local hasRebel, hasLoyal, hasRenegade=false, false, false
    for _,oplayer in ipairs(players) do
        if oplayer:getRole()=="rebel" then hasRebel=true end
		if oplayer:getRole()=="loyalist" then hasLoyal=true end
        if oplayer:getRole()=="renegade" then hasRenegade=true end
		if hasRebel and hasLoyal and hasRenegade then break end
	end
	
	local rebel_num, loyalish_num, loyal_num = 0, 0, 0
    for _, aplayer in ipairs (players) do
        if aplayer:getRole()=="rebel" then 
            rebel_num = rebel_num + 1
		else
			loyalish_num = loyalish_num + 1
		end
    end
	if hasRenegade then loyal_num = loyalish_num-1 else loyal_num = loyalish_num end
	
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
		
		if not hasLoyal then 
			if #players == 2 then 
				if player:getRole() == "rebel" then return 5 else return 3.1 end
			end
			if player:getRole() == "renegade" then return -1 else return 5 end
		end
		
		if hasRenegade then 
			if rebel_num+1 == loyal_num then 
				if player:getRole() == "renegade" then return 4
				elseif player:getRole() == "rebel" then return 5
				else return -2
				end
			end
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
		
		if loyalish_num <= rebel_num then 
			if player:getRole() == "lord" then return -2
 			elseif player:getRole() == "renegade" then return -1 
			elseif player:getRole() == "rebel" then return 5
			end
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
			elseif player:getRole() == "renegade" and #players == 2 then
				if (self.player:getHp()+player:getHp() <= (self.room:getLord()):getHp()) then 
					return -1
				else 
					return 3.1
				end
			end
		elseif not hasLoyal then
			if player:getRole() == "lord" then return 5 
			elseif player:getRole() == "renegade" then return 3.1
			else return -2
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
			if player:isLord() then 
				if player:getHp() > 2 then return 3.1 else return -1 end
			else return 5 end
		end

		if rebel_num > loyalish_num then 
			if player:getRole() == "rebel" then return 5 else return -1 end
		elseif rebel_num < loyalish_num then
			if player:getRole() == "rebel" then return -1 
			elseif player:getRole() == "loyalist" and not player:isLord() then return 5
			elseif player:isLord() then 
				if player:getHp() > 2 then return 3.1 else return -1 end
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
				if player:getRole() == "rebel" then return 3.1
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
end

function SmartAI:sort(players, key)
	local func= sgs.ai_compare_funcs[key or "chaofeng"]
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
	
	if (event == sgs.PhaseChange) or (event == sgs.GameStart) then
		self:updatePlayers()
		for _,skill in ipairs(sgs.ai_skills) do
			if self:hasSkill(skill) then
			self[skill.name.."_used"]=false
			end
		end
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

		if sgs.ai_card_intention[card:className()] then
			local intention=sgs.ai_card_intention[card:className()](card,from,to,source)
			if to:isLord() and intention<0 then 
			sgs.ai_anti_lord[from:objectName()]=(sgs.ai_anti_lord[from:objectName()] or 0)+1
			end
			self:refreshRoyalty(from,intention)
		end
		
	elseif event == sgs.CardUsed then
		local struct= data:toCardUse()
		local card  = struct.card
		local to    = struct.to
			  to    = sgs.QList2Table(to)
		local from  = struct.from
		local source= self.room:getCurrent()                   

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
	elseif event == sgs.CardLost then
		local move=data:toCardMove()
		local from=move.from
		local to=  move.to
		local place=move.from_place
		if sgs.ai_snat_disma_effect then 
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

function SmartAI:getFriendsNoself(player)
	if self:isFriend(self.player, player) then
		return self.friends_noself
	elseif self:isEnemy(self.player, player) then
		friends = sgs.QList2Table(self.lua_ai:getEnemies())
		for i = #friends, 1, -1 do 
			if friends[i]:objectName() == player:objectName() then
			  table.remove(friends, i)
			end
		end
		return friends
	else 
		return {}
	end
end

function SmartAI:getFriends(player)
	if self:isFriend(self.player, player) then
		return self.friends
	elseif self:isEnemy(self.player, player) then
		return self.enemies
	else 
		return {player}
	end
end

function SmartAI:getEnemies(player)
	if self:isFriend(self.player, player) then
		return self.enemies
	elseif self:isEnemy(self.player, player) then
		return self.friends
	else 
		return {}
	end
end

function SmartAI:isFriend(other, another)
	if another then 
		if self.lua_ai:isFriend(other) and self.lua_ai:isFriend(another) then return true end
	end
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
		
		if effect.to:hasSkill("xiaoji") or effect.to:hasSkill("xuanfeng") then
			return self:isFriend(effect.to)
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

function SmartAI:askForYiji(cards)
    self:sort(self.friends_noself,"handcard")
		
	if self.player:getHandcardNum()>3 then
		if #self.friends_noself>0 then
			return self.friends_noself[1],cards[1]
		end
	end
	
--	if true then
--		return nil, -1
--	end
	
    if self.player:getHandcardNum()<2 then
		return nil, -1
	end
	
	for _, card_id in ipairs(cards) do
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
				
				if friend:hasSkill("jijiu") then 
					if card:isRed() then return friend, card_id end 
				end
				
				if friend:getHandcardNum() < friend:getHp() then
					return friend, card_id 
				end
			end
		end
	end
end

-- used for SmartAI:askForUseCard
sgs.ai_skill_use = {}

function SmartAI:askForUseCard(pattern, prompt)
	local use_func = sgs.ai_skill_use[pattern]
	if use_func then
		return use_func(self, prompt) or "."
	else
		return "."
	end
end

function SmartAI:slashIsEffective(slash, to)
	if to:hasSkill("yizhong") and not to:getArmor() then
		if slash:isBlack() then
			return false
		end		
	end
	
	if to:hasSkill("zhichi") and self.room:getTag("Zhichi"):toString() == to:objectName() then
		return false
	end

	if self.player:hasWeapon("qinggang_sword") then
		return true
	end

	local armor = to:getArmor()
	if armor then
		if armor:objectName() == "renwang_shield" then
			return not slash:isBlack()
		elseif armor:objectName() == "vine" then
			return slash:inherits("NatureSlash") or self.player:hasWeapon("fan")
		end
	end

	return true
end

function SmartAI:slashHit(slash, to)
	if self.player:hasSkill("pojun") and 
		(to:getHp() > 3 or (to:getHp() > 2 and not to:faceUp())) then
		return true 
	end
	
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

function SmartAI:getEquipNumber(player)
	return player:getEquips():length()
end

function SmartAI:searchForAnaleptic(use,enemy,slash)   
    if not self.toUse then return nil end
	
	for _,card in ipairs(self.toUse) do
		if card:getId()~=slash:getId() then return nil end
	end

    if not use.to then return nil end
    if self.anal_used then return nil end
    
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)
    self:fillSkillCards(cards)
	
	if (getDefense(self.player)<getDefense(enemy)) and 
		(self.player:getHandcardNum()<1+self.player:getHp()) or
		self.player:hasFlag("drank") then
			return
    end

    if enemy:getArmor() then 
        if ((enemy:getArmor():objectName())=="eight_diagram")
            or ((enemy:getArmor():objectName())=="silver_lion") then 
			if (self.player:getHandcardNum()<=1+self.player:getHp()) then
				return
			end
		end
	end
	
    for _, anal in ipairs(cards) do
        if (anal:className()=="Analeptic") and not (anal:getEffectiveId()==slash:getEffectiveId()) then
            self.anal_used=true
            
            return anal
        end
    end
end

--- Judge the player has the armor or not
-- @param player The player to judge
-- @param armor_name The armor's objectName
-- @return true if the player has the corresponding armor, otherwise false
function SmartAI:hasArmor(player, armor_name)
	if armor_name == "eight_diagram" then
		if player:getArmor() then
			return player:getArmor():objectName() == "eight_diagram"
		else
			return player:hasSkill("bazhen")
		end
	else
		return player:getArmor() and player:getArmor():objectName() == armor_name
	end
end

function SmartAI:slashProhibit(card,enemy)
	if card == nil then
		card = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	end

    if self:isFriend(enemy) then
        if card:inherits("FireSlash") or self.player:hasWeapon("fan") then
            if self:hasArmor(enemy, "vine") then return true end
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
        if self.player:hasSkill("tieji") or self.player:hasSkill("liegong") then return false end
        
        if enemy:getHandcardNum()>=2 then return true end
        if enemy:getArmor() and (enemy:getArmor():objectName()=="eight_diagram") then 
            local equips=enemy:getEquips()
            for _,equip in sgs.qlist(equips) do
                if equip:getSuitString()=="spade" then return true end
            end
        end
    end
    
    if enemy:hasSkill("tiandu") then 
        if self:hasArmor(enemy, "eight_diagram") then return true end
    end
    
    if enemy:hasSkill("ganglie") then
        if self.player:getHandcardNum()+self.player:getHp()<5 then return true end
    end
	
	if enemy:hasSkill("shenjun") and (enemy:getGeneral():isMale()~=self.player:getGeneral():isMale()) and not card:inherits("ThunderSlash") then
	    return true
	end
	
	return not self:slashIsEffective(card, enemy)
end


function SmartAI:useBasicCard(card, use,no_distance)
	if card:getSkillName()=="wushen" then no_distance=true end
	if (self.player:getHandcardNum()==1) and self.player:getWeapon() and self.player:getWeapon():inherits("Halberd")
	or self.player:hasSkill("shenji") and not self.player:getWeapon() then
		self.slash_targets=3
	end	
	
	if card:inherits("Slash") and self:slashIsAvailable() then
		local target_count=0
		for _, friend in ipairs(self.friends_noself) do						
			local slash_prohibit=false
			slash_prohibit=self:slashProhibit(card,friend)
			if (self.player:hasSkill("pojun") and friend:getHp() >3 and self:getJinkNumber(friend) == 0) 
			or (self.player:hasSkill("wushuang") and friend:hasSkill("leiji") and self:getJinkNumber(friend) > 1 and self.player:getHandcardNum() > 2 and #self.enemies > 0)
			or (friend:hasSkill("leiji") and self:getJinkNumber(friend) > 0)
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
--				break
			end
		end	
	
		self:sort(self.enemies, "defense")
		for _, enemy in ipairs(self.enemies) do
			local slash_prohibit=false
			slash_prohibit=self:slashProhibit(card,enemy)
			if not slash_prohibit then
				self.predictedRange = self.player:getAttackRange()
				if ((self.player:canSlash(enemy, not no_distance)) or 
				(use.isDummy and self.predictedRange and (self.player:distanceTo(enemy)<=self.predictedRange))) and 
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
		
		for _, friend in ipairs(self.friends_noself) do	
			if friend:hasSkill("yiji") and friend:getLostHp() < 1 and 
				not (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
				local slash_prohibit=false
				slash_prohibit=self:slashProhibit(card, friend)
				if not slash_prohibit then
					if ((self.player:canSlash(friend, not no_distance)) or 
						(use.isDummy and (self.player:distanceTo(friend)<=self.predictedRange))) and 
						self:slashIsEffective(card, friend) then
						use.card=card
						if use.to then 
							use.to:append(friend) 
							self:speak("yiji")
						end
						target_count=target_count+1
						if self.slash_targets<=target_count then return end
					end
				end	
				break
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
	if (to:hasSkill("zhichi") and self.room:getTag("Zhichi"):toString() == to:objectName()) then
		return false
	end
	
	--Zhangjiao's leiji
	if card:inherits("ArcheryAttack") then
		if (to:hasSkill("leiji") and self:getJinkNumber(to) > 0) or (to:getArmor() and to:getArmor():objectName() == "eight_diagram" and to:getHp() > 1) then
			return false
		end
	end

	return true
end

function SmartAI:getDistanceLimit(card)
	if self.player:hasSkill("qicai") then
		return nil
	end

	if card:inherits("Snatch") then
		return 1
	elseif card:inherits("SupplyShortage") then
		if self.player:hasSkill("duanliang") then
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
	local hasLion, target
	for _, friend in ipairs(friends) do
		if self:hasTrickEffective(dismantlement, friend) then
			if (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
				use.card = dismantlement
				if use.to then use.to:append(friend) end
				return
			end		
			if friend:getArmor() and friend:getArmor():objectName() == "silver_lion" and friend:isWounded() then
				hasLion = true
				target = friend
			end
		end
	end	
	
	if hasLion then 				
		use.card = dismantlement
		if use.to then use.to:append(target) end
		return
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
		
		    if  not enemy:isNude() and self:hasTrickEffective(dismantlement, enemy) and					---update
			   (not enemy:hasSkill("xiaoji") or enemy:getEquips():isEmpty()) then                   
				if enemy:getHandcardNum() == 1 then
					if enemy:hasSkill("kongcheng") or enemy:hasSkill("lianying") then return end
				end
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
	local hasLion, target
	for _, friend in ipairs(friends) do
		if self:hasTrickEffective(snatch, friend) then
			if (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
				use.card = snatch
				if use.to then use.to:append(friend) end
				return
			end		
			if friend:getArmor() and friend:getArmor():objectName() == "silver_lion" and friend:isWounded() then
				hasLion = true
				target = friend
			end
		end
	end	
	
	if hasLion then 				
		use.card = snatch
		if use.to then use.to:append(target) end
		return
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
		if  not enemy:isNude() and self:hasTrickEffective(snatch, enemy) and					---update
			(not enemy:hasSkill("xiaoji") or enemy:getEquips():isEmpty()) then                   
			if enemy:getHandcardNum() == 1 then
				if enemy:hasSkill("kongcheng") or enemy:hasSkill("lianying") then return end
			end       
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
	
	if self.player:hasSkill("hongyan") then
		lack["spade"] = true
	end
	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if (self:objectiveLevel(enemy)>3) and not enemy:isKongcheng()  and self:hasTrickEffective(fire_attack, enemy) then							----no xushu
			
			local cards = enemy:getHandcards()
			local success = true
			for _, card in sgs.qlist(cards) do
				if lack[card:getSuitString()] then
					success = false
					break
				end
			end

			if success or self.player:getHandcardNum()-1>=self.player:getHp() then
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
	player = player or self.player
	local n = 0
	local cards = player:getCards("h")
	for _, card in sgs.qlist(cards) do
		if self:canViewAs(card, "Slash", player) then		
			n = n + 1
		end
	end
	if player:hasWeapon("spear") then
		n = n + math.floor((cards:length() - n)/2)
	end	
	cards = player:getCards("e")
	for _, card in sgs.qlist(cards) do
		if self:getSkillViewCard(card, "Slash", true, player) then		
			n = n + 1
		end
	end
	
	if player:hasSkill("wushuang") then
		n = n * 2
	end
	
	if only_self then return n end

	if (player:isLord() or (player:hasSkill("weidi") and self.room:getLord():hasSkill("jijiang"))) and player:hasSkill("jijiang") then
		local lieges = self.room:getLieges("shu", player)
		for _, liege in sgs.qlist(lieges) do
			if self:isFriend(liege, player) then
				n = n + self:getSlashNumber(liege)
			end
		end
	end
	
	return n
end

function getJinkNumber(player, only_self)
	player = player or self.player
	local n = 0
	local cards = player:getCards("h")
	for _, card in sgs.qlist(cards) do
		if self:canViewAs(card, "Jink", player) then		
			n = n + 1
		end
	end	

	local armor = player:getArmor()
	if armor and armor:objectName() == "eight_diagram" then
		local judge_card = self.room:peek()
		if judge_card:isRed() then
			n = n + 1
		end
	end

  if only_self then return n end
  
	if (player:isLord() or (player:hasSkill("weidi") and self.room:getLord():hasSkill("hujia"))) and player:hasSkill("hujia") then
		local lieges = self.room:getLieges("wei", player)
		for _, liege in sgs.qlist(lieges) do
			if self:isFriend(liege, player) then
				n = n + self:getJinkNumber(liege)
			end
		end
	end
	return n
end

function SmartAI:getJinkNumber(player, only_self)
	return getJinkNumber(player, only_self)
end

function SmartAI:useCardDuel(duel, use)
	if self.player:hasSkill("wuyan") then return end
	self:sort(self.enemies,"defense")
	local enemies = self:exclude(self.enemies, duel)
	for _, enemy in ipairs(enemies) do
		if self:objectiveLevel(enemy)>3 then
			local n1 = self:getSlashNumber(self.player)
			local n2 = self:getSlashNumber(enemy)

			if n1 >= n2 and self:hasTrickEffective(duel, enemy) then													--no xushu
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
	
	for _, friend in ipairs(self.friends_noself) do												
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
			and self:hasTrickEffective(card, enemy)
			and not enemy:hasSkill("xiaoji")					
			and not enemy:hasSkill("weimu")						
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
		if not enemy:isChained() and not self.room:isProhibited(self.player, enemy, card) and not enemy:hasSkill("danlao") 
			and self:hasTrickEffective(card, enemy) and not (self:objectiveLevel(enemy)<=3) then
			table.insert(targets, enemy)
		end
	end

    use.card = card
	
	if targets[2] and not self.player:hasSkill("wuyan") then
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
	if self.player:containsTrick("lightning") then return end
	if self.player:hasSkill("weimu") and card:isBlack() then return end
	
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

function SmartAI:useCardGodSalvation(card, use)				
	local good, bad = 0, 0
	
	if self.player:hasSkill("wuyan") then 						
		use.card = card
		return 
	end	
	
	for _, friend in ipairs(self.friends) do 
		if friend:isWounded() then		
			good = good + 10/(friend:getHp())
			if friend:isLord() then good = good + 10/(friend:getHp()) end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if enemy:isWounded() then
			bad = bad + 10/(enemy:getHp())
			if enemy:isLord() then 
				bad = bad + 10/(enemy:getHp()) 
			end
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

function SmartAI:getPeachNum()
	local index = 0
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("Peach") then index = index + 1 end
	end
	if self.player:hasSkill("jijiu") then 
		cards = self.player:getCards("e")
		cards = sgs.QList2Table(cards)
		for _, card in ipairs(cards) do
			if card:isRed() then index = index + 1 end
		end
	end
	
	return index
end

function SmartAI:getAllPeachNum()
	local n = 0
	for _, friend in ipairs(self.friends) do
		n = n + self:getPeachNum(friend)
	end
	return n
end

function SmartAI:getAnalepticNum(player)
	player = player or self.player
	local n = 0
	local cards = player:getCards("h")
	for _, card in sgs.qlist(cards) do
		if self:canViewAs(card, "Analeptic", player) then		
			n = n + 1
		end
	end
	return n
end


function SmartAI:useTrickCard(card, use)
	if card:inherits("AOE") then
		if self.player:hasSkill("wuyan") then return end
		local good, bad = 0, 0
		for _, friend in ipairs(self.friends_noself) do
			if self:aoeIsEffective(card, friend) then
			    bad = bad + 20/(friend:getHp())+10
                if friend:isLord() and (friend:getHp()<3) then
					return 
				end
					
                if (friend:getHp()<2) and (self.player:isLord()) then
					return
				end
			end
			good = good + self:getPeachNum()
		end

		for _, enemy in ipairs(self.enemies) do
			if self:aoeIsEffective(card, enemy) then
			    good = good + 20/(enemy:getHp())+10
				
				if enemy:isLord() then
					good = good + 20/(enemy:getHp())
				end
			end
			bad = bad + self:getPeachNum()
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
    for _,card in ipairs(cards) do
        local dummy_use={}
        dummy_use.isDummy=true
        if (not self.player:hasSkill("kongcheng")) and (not self.player:hasSkill("lianying")) then
            if (i >= (self.player:getHandcardNum()-self.player:getHp()+self.retain)) and (self:getUseValue(card)<self.retain_thresh) then
                return turnUse
            end
        
            if (i >= (self.player:getHandcardNum()-self.player:getHp())) and (self:getUseValue(card)<8.5) and self.harsh_retain then
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
        end
    end

    return turnUse
end

function SmartAI:activate(use)
	self:updatePlayers()
	self:assignKeep(self.player:getHp(),true)
	self:printCards(self.kept)
	self.toUse =self:getTurnUse()
	self:printCards(self.toUse)

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
			self.toUse=nil
			return
		end
	end
	
	self.toUse=nil        
end

function SmartAI:hasEquip(card)
	return self.player:hasEquip(card)
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
		if self.player:hasSkill("xiaoji") or self.player:hasSkill("xuanfeng") then return 9 end
		if self.player:hasSkill("kurou") and card:inherits("Crossbow") then return 9 end
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
		elseif self.player:hasSkill("jujian") then
			if not self:hasTrickEffective(card) then v=0 
			else
				if card:inherits("TrickCard") then return 10 end
			end
		elseif self.player:hasSkill("jizhi") and card:inherits("TrickCard") then v=8.7
		else v = sgs.ai_use_value[class_name] or 0 end
		
	end

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
			
		elseif card:inherits("Collateral") or card:inherits("Dismantlement") or card:inherits("Snatch") or card:inherits("IronChain") then v = 0
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
		
		if self.player:getHandcardNum() == 3 then							
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
	
	local cards = self.player:getCards("h")
	if not cards then return {} end
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	
	local to_discard = {}
	for i=1, discard_num do
		table.insert(to_discard, cards[i]:getEffectiveId())
	end
	
	if include_equip then
		local equips = self.player:getCards("e")
		if equips then
			equips = sgs.QList2Table(equips)
			for i=#equips, 1, -1 do
				if equips[i]:inherits("OffensiveHorse") then
					table.insert(to_discard, equips[i]:getEffectiveId(), 1)
				elseif equips[i]:inherits("Weapon") then
					table.insert(to_discard, equips[i]:getEffectiveId(), 1)
				end
			end
		end
	end
	return to_discard	
end

--- Determine that the current judge is worthy retrial
-- @param judge The JudgeStruct that contains the judge information
-- @return True if it is needed to retrial
function SmartAI:needRetrial(judge)
	if self:isFriend(judge.who) then
		return not judge:isGood()
	elseif self:isEnemy(judge.who) then
		return judge:isGood()
	else
		return false
	end
end

--- Get the retrial cards with the lowest keep value
-- @param cards the table that contains all cards can use in retrial skill
-- @param judge the JudgeStruct that contains the judge information
-- @return the retrial card id or -1 if not found
function SmartAI:getRetrialCardId(cards, judge)
	local can_use = {}
	for _, card in ipairs(cards) do
		if self:isFriend(judge.who) and judge:isGood(card) then
			table.insert(can_use, card)
		elseif self:isEnemy(judge.who) and not judge:isGood(card) then
			table.insert(can_use, card)
		end
	end
	
	if next(can_use) then
		self:sortByKeepValue(can_use, true)
		return can_use[1]:getEffectiveId()
	else
		return -1
	end
end

sgs.ai_skill_playerchosen = {}

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
		if skill then
			return skill:getDefaultChoice(self.player) 
		else
			local choice_table = choices:split("+");
			local r = math.random(1, #choice_table)
			return choice_table[r]
		end
	end
end


function SmartAI:getCardRandomly(who, flags)
	local cards = who:getCards(flags)						
	local r = math.random(0, cards:length()-1)
	local card = cards:at(r)
	if who:getArmor() and who:getArmor():objectName() == "silver_lion" then
		if self:isEnemy(who) and who:isWounded() and card == who:getArmor() then
			if r ~= (cards:length()-1) then
				card = cards:at(r+1)
			else
				card = cards:at(r-1)
			end
		end
	end
	return card:getEffectiveId()
end

sgs.ai_skill_cardchosen = {}
function SmartAI:askForCardChosen(who, flags, reason)
	self.room:output(reason)

	local cardchosen = sgs.ai_skill_cardchosen[string.gsub(reason,"%-","_")]
	local card
	if type(cardchosen) == "function" then
		card = cardchosen(self, who)
	end
	if card then
		return card:getId()
	end
	
    if self:isFriend(who) then
		if flags:match("j") then
			local tricks = who:getCards("j")
			local lightning, indulgence, supply_shortage
			for _, trick in sgs.qlist(tricks) do
				if trick:inherits("Lightning") then
					lightning = trick:getId()
				elseif trick:inherits("Indulgence") or trick:getSuit() == sgs.Card_Diamond then
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
		end
		
		if flags:match("e") then
			if who:isWounded() and who:getArmor() and who:getArmor():objectName() == "silver_lion" then return who:getArmor():getId() end
			if who:hasSkill("xiaoji") then
				local equips = who:getEquips()
				if not equips:isEmpty() then
					return equips:at(0):getId()
				end
			end
		end
	else 
        if (who:getHandcardNum()<2) and (not who:isKongcheng()) and
			not (who:hasSkill("lianying") or who:hasSkill("kongcheng")) then return -1 
		end
		
		if reason == "ice_sword" then
			local hcards = who:getCards("h")
			hcards = sgs.QList2Table(hcards)
			for _, peach in ipairs(hcards) do
				if peach:inherits("Peach") or peach:inherits("Analeptic") then return peach:getId() end
			end
		end
		
		if flags:match("e") then		    
			if who:getDefensiveHorse() then
				for _,friend in ipairs(self.friends) do
					if friend:distanceTo(who)==friend:getAttackRange()+1 then 
					 	return who:getDefensiveHorse():getId()
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
		end
		if flags:match("h") then
			if not who:isKongcheng() then
				local cards = who:getHandcards()
				cards = sgs.QList2Table(cards)
				self:sortByUseValue(cards)
				return cards[1]:getId()
			end
		end
        
        if not who:isKongcheng() then
			return -1
		end
	end
	local new_flag=""
    if flags:match("h") then new_flag="h" end
    if flags:match("e") then new_flag=new_flag.."e" end
    return self:getCardRandomly(who, new_flag)							
end

function SmartAI:askForCard(pattern, prompt)
	self.room:output(prompt)
	
	if sgs.ai_skill_invoke[pattern] then return sgs.ai_skill_invoke[pattern](self, prompt) end
	local target, target2
	if not prompt then return end
	local parsedPrompt = prompt:split(":")
	if parsedPrompt[2] then
		local others = self.room:getOtherPlayers(self.player)
		others = sgs.QList2Table(others)
		for _, other in ipairs(others) do
			if other:getGeneralName() == parsedPrompt[2] or other:objectName() == parsedPrompt[2] then target = other break end
		end
		if parsedPrompt[3] then
			for _, other in ipairs(others) do
				if other:getGeneralName() == parsedPrompt[3] or other:objectName() == parsedPrompt[3] then target2 = other break end
			end
		end
	end
	
	if parsedPrompt[1] == "@xiuluo" then
		local hand_card = self.player:getHandcards()
		for _, card in sgs.qlist(hand_card) do
			if card:getSuitString() == parsedPrompt[2] then return "$"..card:getEffectiveId() end
		end
	end
	
	-- aoe
	if parsedPrompt[1] == "savage-assault-slash" or parsedPrompt[1] == "archery-attack-jink" then
		local objectName , aoe
		if parsedPrompt[1] == "savage-assault-slash" then 
			objectName = "savage_assault" 
		else
			objectName = "archery_attack" 
		end
		aoe = sgs.Sanguosha:cloneCard(objectName, sgs.Card_NoSuit , 0)	
		if (self.player:hasSkill("jianxiong") and self:getAoeValue(aoe) > -10) or (self.player:hasSkill("yiji")) and self.player:getHp() > 2 then return "." end
		if target and target:hasSkill("guagu") and self.player:isLord() then return "." end
		if self.player:hasSkill("jieming") and self:getJiemingChaofeng() <= -6 and self.player:getHp() >= 2 then return "." end
 	end
	
	if pattern == "slash" and self:getSlashNumber(self.player) > 0 then
		if parsedPrompt[1] == "@jijiang-slash" then
			if not target then target = self.room:getLord() end
			if not self:isFriend(target) then return "." end
		end	
		if parsedPrompt[1] == "@wushuang-slash-1" and self:getSlashNumber(self.player) < 2 and 
			not (self.player:getHandcardNum() == 1 and self:hasSkills(sgs.need_kongcheng)) then return "." end
		if parsedPrompt[1] == "collateral-slash" then 
			if target and (not self:isFriend(target2) or target2:getHp() > 2 or self:getJinkNumber(target2) > 0) and not self:hasSkills(sgs.lose_equip_skill) then 
				return self:getCardId("Slash")
			end
			self:speak("collateral", self.player:getGeneral():isFemale())
			return "."
		elseif (parsedPrompt[1] == "duel-slash") then
			if (not self:isFriend(target) or (target:getHp() > 2 and self.player:getHp() <= 1 and self:getPeachNum() == 0 and not self.player:hasSkill("buqu"))) then 
				return self:getCardId("Slash")
			else return "." end
		elseif (parsedPrompt[1] == "@jijiang-slash") then
			if target and self:isFriend(target) then 
				if (self.player:hasSkill("longdan") and self:getJinkNumber(self.player) > 1) then 
					self:speak("jijiang", self.player:getGeneral():isFemale()) 
					return self:getCardId("Slash")
				end 
			else return "." end	
		else
			return self:getCardId("Slash") or "."
		end
	elseif pattern == "jink" then
		if parsedPrompt[1] == "@hujia-jink" then
			if not target then target = self.room:getLord() end
			if not self:isFriend(target) then return "." end
		end
		if (parsedPrompt[1] == "@wushuang-jink-1" or parsedPrompt[1] == "@roulin1-jink-1" or parsedPrompt[1] == "@roulin2-jink-1") 
			and self:getJinkNumber(self.player) < 2 then return "." end
		if target then
			if self:isFriend(target) then
				if self.player:hasSkill("yiji") and (self.player:getLostHp()<2) then return "." end
				if self.player:hasSkill("jieming") and self:getJiemingChaofeng() <= -6 then return "." end
				if target:hasSkill("pojun") and not self.player:faceUp() then return "." end
				if target:hasSkill("guagu") and self.player:isLord() then return "." end
				if (target:hasSkill("jieyin") and (not self.player:isWounded()) and self.player:getGeneral():isMale()) and not self.player:hasSkill("leiji") then return "." end
			else
				if not target:hasFlag("drank") then
					if target:hasSkill("mengjin") and self.player:hasSkill("jijiu") then return "." end
				end
				if not self:hasSkills(sgs.need_kongcheng, player) then
					if self:isEquip("Axe", target) then
						if self:hasSkills(sgs.lose_equip_skill, target) and target:getEquips():length() > 1 then return "." end
						if target:getHandcardNum() - target:getHp() > 2 then return "." end
					elseif self:isEquip("Blade", target) then
						if self:getJinkNumber(self.player) <= self:getSlashNumber(target) then return "." end
					end
				end
								
			end
		end
		return self:getCardId("Jink") or "."
	end

	if parsedPrompt[1] == "double-sword-card" then 
		if target and self:isFriend(target) then return "." end					
		local cards = self.player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:inherits("Slash") or card:inherits("Shit") or card:inherits("Collateral") or card:inherits("GodSalvation")
			  or card:inherits("Lightning") or card:inherits("EquipCard") or card:inherits("AmazingGrace") then
				return "$"..card:getEffectiveId()
			end
		end	
		return "."
	elseif parsedPrompt[1] == "@axe" then
		if target and self:isFriend(target) then return "." end
		
		local allcards = self.player:getCards("he")
		allcards = sgs.QList2Table(allcards)
		if self.player:hasFlag("drank") or #allcards-2>= self.player:getHp() or (self.player:hasSkill("kuanggu") and self.player:isWounded()) then
			local cards = self.player:getCards("h")
			cards = sgs.QList2Table(cards)
			local index
			if self:hasSkills(sgs.need_kongcheng) then index = #cards end
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
			--self:log(#cards)
			if #cards>= 2 then
				self:sortByUseValue(cards, true)
				return "$"..cards[1]:getEffectiveId().."+"..cards[2]:getEffectiveId()
			end
		end
	elseif parsedPrompt[1] == "@enyuan" then
		local cards = self.player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:getSuit() == sgs.Card_Heart and not (card:inherits("Peach") or card:inherits("ExNihio")) then 
				return card:getEffectiveId()
			end
		end	
		return "."
	end
																	
	if self.player:hasSkill("tianxiang") then							
		local dmgStr = {damage = 1, nature = 0}
		local willTianxiang = sgs.ai_skill_use["@tianxiang"](self, dmgStr)
		if willTianxiang~= "." then return "." end
	end		
end

sgs.ai_skill_askforag = {}

function SmartAI:askForAG(card_ids, refusable, reason)	
	local cardchosen = sgs.ai_skill_askforag[string.gsub(reason, "%-", "_")]
	if type(cardchosen) == "function" then
		local card_id = cardchosen(self, card_ids)
		if card_id then return card_id end
	end
	
	if refusable and self:hasSkill("xinzhan") then
		local next_player = self.player:getNextAlive()
		if self:isFriend(next_player) and next_player:containsTrick("indulgence") then
			if #card_ids == 1 then return -1 end
		end
		return card_ids[1] 
	end
	local ids = card_ids
	local cards = {}
	for _, id in ipairs(ids) do
		table.insert(cards, sgs.Sanguosha:getCard(id))
	end
	self:sortByCardNeed(cards, true)
	return cards[#cards]:getEffectiveId()
end

function SmartAI:askForNullification(trick_name, from, to, positive)  							
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards)
	local null_card, hasNull
	for _, card in ipairs(cards) do
		if card:inherits("Nullification") then 
			null_card = card
			hasNull = true
			break
		end				
		if card:isBlack() and self.player:hasSkill("kanpo") then
			local number = card:getNumber()
			local card_id = card:getEffectiveId()
			local card_str = ("nullification:kanpo[%s:%d]=%d"):format(card:getSuitString(), number, card_id)
			null_card = sgs.Card_Parse(card_str)
			hasNull = true
			break
		end
	end
	
	if not hasNull then return nil end
  
	if positive then
		if from and self:isEnemy(from) then
			if trick_name:inherits("ExNihilo") and self:getOverflow(from) == 0 then return null_card end
			if trick_name:inherits("IronChain") and not self:isEquip("Vine", to) then return nil end
			if self:isFriend(to) then
				if trick_name:inherits("Dismantlement") then
					if to:getArmor() then return null_card end
				else
					if trick_name:inherits("Snatch") then return null_card end
					if self:isWeak(to) then
						if trick_name:inherits("Duel") then
							return null_card
						elseif trick_name:inherits("FireAttack") then
							if from:getHandcardNum() > 2 then return null_card end
						end
					end
				end
			elseif self:isEnemy(to) then
				if (trick_name:inherits("Snatch") or trick_name:inherits("Dismantlement")) and to:getCards("j"):length() > 0 then
					return null_card
				end
			end
		end
		
		if self:isFriend(to) then
			if trick_name:inherits("Indulgence") or trick_name:inherits("SupplyShortage") then
				return null_card
			end	
			if self:isWeak(to) then 
				if trick_name:inherits("ArcheryAttack") then
					if self:getJinkNumber(to) == 0 then return null_card end
				elseif trick_name:inherits("SavageAssault") then
					if self:getSlashNumber(to) == 0 then return null_card end
				end
			end
		end
		if from then
			if self:isEnemy(to) then
				if trick_name:inherits("GodSalvation") and self:isWeak(to) then
					return null_card
				end
			end	
		end
	else
		if from then
			if from:objectName() == to:objectName() then
				if self:isFriend(from) then return null_card
				else return nil end
			end
			if not (trick_name:inherits("AmazingGrace") or trick_name:inherits("GodSalvation") or trick_name:inherits("AOE")) then
				if self:isFriend(from) then return null_card end
			end
		else
			if self:isEnemy(to) then return null_card else return end
		end
	end
end

function SmartAI:askForSinglePeach(player, dying)										
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
					if self.player:hasSkill("jiushi") and self.player:faceUp() then return nil end
					return card
				elseif card:getSuit() == sgs.Card_Spade and self.player:hasSkill("jiuchi") then
					local suit = card:getSuitString()
					local number = card:getNumber()
					local card_id = card:getEffectiveId()
					local card_str = ("analeptic[%s:%d]=%d"):format(suit, number, card_id)
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
	local skill_name = skill 
	if type(skill) == "table" then
		skill_name = skill.name
	end
	
	local real_skill = sgs.Sanguosha:getSkill(skill_name)
	if real_skill and real_skill:isLordSkill() then
		return self.player:hasLordSkill(skill_name)
	else
		return self.player:hasSkill(skill_name)
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
            
			if skill.name == "ganran" then 				
				 for i=#cards,1,-1 do 
                    
                    if cards[i]:inherits("EquipCard") then
                        self:log("cant use "..cards[i]:className()..i)
                        table.remove(cards,i)
                    end
                end
			end	
			
			if skill.name == "jiejiu" then 				
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
        end
    end
end

function SmartAI:useSkillCard(card,use)
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

sgs.ai_cardshow = {}

function SmartAI:askForCardShow(requestor, reason)	

	local func = sgs.ai_cardshow[reason]
	if func then
		return func(self, requestor)
	else
		return self.player:getRandomHandCard()
	end
--	local card = self:getCardRandomly(self.player,"h")
--	card=sgs.Sanguosha:getCard(card)
--	if self.player:hasSkill("hongyan") and card:getSuit()==sgs.Card_Spade then
--		card:setSuit(sgs.Card_Heart)
--		card:setSkillName("hongyan")
--	end
--	return card
end

sgs.ai_cardshow.fire_attack = function(self, requestor)
	local priority =
	{
	heart = 4,
	spade = 3,
	club = 2,
	diamond = 1
	}
	local index = 0
	local result
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if priority[card:getSuitString()]>index then
			result = card
			index = priority[card:getSuitString()]
		end
	end
	if self.player:hasSkill("hongyan") and result:getTypeId() == sgs.Card_Spade then 
		result = sgs.Sanguosha:cloneCard(card:objectName(), sgs.Card_heart, card:getNumber())
		result:setSkillName("hongyan")
	end

	return result
end

function SmartAI:hasTrickEffective(card, player)
	if player then
		if (player:hasSkill("zhichi") and self.room:getTag("Zhichi"):toString() == player:objectName()) or player:hasSkill("wuyan") then
			if card and not (card:inherits("Indulgence") or card:inherits("SupplyShortage")) then return false end
		end
	else
		if self.player:hasSkill("wuyan") then 
			if card:inherits("TrickCard") and not 
				(card:inherits("DelayedTrick") or card:inherits("GodSalvation") or card:inherits("AmazingGrace")) then
			return false end
		end
	end
	return true
end
	
function SmartAI:hasSameEquip(card, player)
	player = player or self.player
	if player:getEquips():isEmpty() then return false end
	if card:inherits("Weapon") then 
		if player:getWeapon() then return true end
	elseif card:inherits("Armor") then
		if player:getArmor() then return true end
	elseif card:inherits("DefensiveHorse") then
		if player:getDefensiveHorse() then return true end
	elseif card:inherits("OffensiveHorse") then 
		if player:getOffensiveHorse() then return true end
	end
	return false
end

sgs.lose_equip_skill = "xiaoji|xuanfeng"
sgs.need_kongcheng = "lianying|kongcheng"

function SmartAI:hasSkills(skill_names, player)
	player = player or self.player
	for _, skill_name in ipairs(skill_names:split("|")) do
		if player:hasSkill(skill_name) then
			return true
		end
	end
end

function SmartAI:isEquip(equip_name, player)
	player = player or self.player
	local cards = player:getCards("e")
	for _, card in sgs.qlist(cards) do
		if card:inherits(equip_name) then return true end
	end
	if equip_name == "EightDiagram" and player:hasSkill("bazhen") and not player:getArmor() then return true end
	return false
end

function SmartAI:getCardId(class_name, player)
	player = player or self.player
	local cards = player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUsePriority(cards)
	for _, card in ipairs(cards) do
		if card:inherits(class_name) then
			return card:getEffectiveId()
		elseif self:isCompulsoryView(card, class_name, player) then 
			return self:isCompulsoryView(card, class_name, player)
		end
	end
	local skill_card
	for _, card in ipairs(cards) do
		skill_card = self:getSkillViewCard(card, class_name, false, player)
		if skill_card then return skill_card end
	end
	local cards = self.player:getCards("e")
	cards = sgs.QList2Table(cards)
	for _, card in ipairs(cards) do
		skill_card = self:getSkillViewCard(card, class_name, true, player)
		if skill_card then return skill_card end
	end
end

function SmartAI:canViewAs(card,class_name,player)
	player = player or self.player
	local r1 = card:inherits(class_name) or self:getSkillViewCard(card,class_name,false,player)
	local r2 = self:isCompulsoryView(card,"Slash",player)
	if class_name == "Slash" then
		return r1 or r2
	else
		return r1 and not r2
	end 
end

function SmartAI:isCompulsoryView(card,class_name,player)
	player = player or self.player
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if class_name == "Slash" then
	    if self.player:hasSkill("wushen") and card:getSuit() == sgs.Card_Heart then return ("slash:wushen[%s:%s]=%d"):format(suit, number, card_id) end
		if self.player:hasSkill("jiejiu") and card:inherits("Analeptic") then return ("slash:jiejiu[%s:%s]=%d"):format(suit, number, card_id) end
	end	
end

function SmartAI:getSkillViewCard(card,class_name,only_equip,player)
	player = player or self.player
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if class_name == "Slash" then
		if player:hasSkill("wusheng") then
			if card:isRed() and not card:inherits("Peach") then
				return ("slash:wusheng[%s:%s]=%d"):format(suit, number, card_id)
			end
		end	
		if not only_equip then
			if player:hasSkill("longdan") and card:inherits("Jink") then
				return ("slash:longdan[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	elseif class_name == "Jink" then
		if not only_equip then
			if player:hasSkill("longdan") and card:inherits("Slash") then
				return ("jink:longdan[%s:%s]=%d"):format(suit, number, card_id)
			elseif player:hasSkill("qingguo") and card:isBlack() then
				return ("jink:qingguo[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	elseif class_name == "Peach" then
		if player:hasSkill("jijiu") and card:isRed() then 
			return ("peach:jijiu[%s:%s]=%d"):format(suit, number, card_id)
		end
	elseif class_name == "Analeptic" then
		if not only_equip then
			if player:hasSkill("jiuchi") and card:getSuit() == sgs.Card_Spade then 
				return ("analeptic:jiuchi[%s:%s]=%d"):format(suit, number, card_id)
			end
		end	
	end
end

function SmartAI:getJiemingChaofeng(player)
	local max_x , chaofeng = 0 , 0
    for _, friend in ipairs(self:getFriends(player)) do
		local x = math.min(friend:getMaxHP(), 5) - friend:getHandcardNum()		
		if x > max_x then
			max_x = x
		end
    end
	if max_x < 2 then
		chaofeng = 5 - max_x * 2
	else
		chaofeng = (-max_x) * 2
	end    
    return chaofeng
end

function SmartAI:getOverflow(player)
	player = player or self.player
	return math.max(player:getHandcardNum() - player:getHp(), 0)
end

function SmartAI:hasSuit(suit_strings, include_equip, player)
	return self:getSuitNum(suit_strings, include_equip, player) > 0
end

function SmartAI:getSuitNum(suit_strings, include_equip, player)
	player = player or self.player
	local n = 0
	local flag = "h"
	if include_equip then flag = "he" end
	local allcards = player:getCards(flag)	
	for _, card in sgs.qlist(allcards) do
		for _, suit_string in ipairs(suit_strings:split("|")) do
			if card:getSuitString() == suit_string then
				n = n + 1
			end
		end	
	end
	return n
end

function SmartAI:isWeak(player)
	player = player or self.player
	return player:getHp() <= 2 and player:getHandcardNum() <= 1 and not player:hasSkill("buqu")
end

function SmartAI:getAoeValue(card, player)
	player = player or self.player 
	friends_noself = self:getFriendsNoself(player)
	enemies = self:getEnemies(player)
	local good, bad = 0, 0
	for _, friend in ipairs(friends_noself) do
		good = good + self:getAoeValueTo(card, friend, player)
	end

	for _, enemy in ipairs(enemies) do
		bad = bad + self:getAoeValueTo(card, enemy, player)
	end
	
	if player:hasSkill("jizhi") then
		good = good + 40	
	end
	return good - bad
end

function SmartAI:getAoeValueTo(card, to , from)
	if not from then from = self.player end 
	local value = 0
	local sj_num

	if to:hasSkill("buqu") then
		value = value + 10
	end
	
	if to:hasSkill("longdan") then
		value = value + 5
	end	
	
	if to:hasSkill("danlao") then
		value = value + 15
	end
	
	if card:inherits("SavageAssault") then
		sj_num = self:getSlashNumber(to)
		if to:hasSkill("juxiang") then
			value = value + 20
		end
	end
	if card:inherits("ArcheryAttack") then
		sj_num = self:getJinkNumber(to)
	end
	
	if self:aoeIsEffective(card, to) then
		if to:getHp() > 1 or (self:getPeachNum(to) + self:getAnalepticNum(to) > 0) then
			if to:hasSkill("yiji") or to:hasSkill("jianxiong") then
				value = value + 20
			end
			if to:hasSkill("jieming") then
				value = value - self:getJiemingChaofeng(to) * 3
			end
			if to:hasSkill("ganglie") or to:hasSkill("fankui") or to:hasSkill("enyuan") then
				if not self:isFriend(from, to) then
					value = value + 10
				else 
					value = value - 10
				end
			end
		end
		
		if card:inherits("ArcheryAttack") then
			sj_num = self:getJinkNumber(to)
			if (to:hasSkill("leiji") and self:getJinkNumber(to) > 0) or self:isEquip("EightDiagram", to) then
				value = value + 30
				if self:hasSuit("spade", true, to) then
					value = value + 20
				end
			end
			if to:hasSkill("qingguo") or self:isEquip("EightDiagram", to) then
				value = value + 10
			end	
		end	
		
		if to:getHp() ~= 0 then
			value = value - 24 / to:getHp() - 10
		end
		
		if self:isFriend(from, to) then
	    if (to:isLord() or from:isLord()) and (not to:hasSkill("buqu")) then
				if to:getHp() <= 1 and self:getPeachNum(from) == 0 and sj_num == 0 then
					if to:getRole() == "renegade" then
						value = value - 50
					else
						value = value - 150
					end
				end
			end
			value = value + self:getPeachNum(from) * 2
		elseif to:getRole() == "rebel" or (to:isLord() and from:getRole() == "rebel") then
			if to:getHp() <= 1 and self:getPeachNum(to) == 0 and sj_num == 0 then
				value = value - 50
			end
		end
	else
		value = value + 10
	end
	
	return value 
end

-- load other ai scripts
dofile "lua/ai/standard-ai.lua"
dofile "lua/ai/wind-ai.lua"
dofile "lua/ai/fire-ai.lua"
dofile "lua/ai/thicket-ai.lua"
dofile "lua/ai/mountain-ai.lua"
dofile "lua/ai/god-ai.lua"
dofile "lua/ai/yitian-ai.lua"
dofile "lua/ai/nostalgia-ai.lua"
dofile "lua/ai/yjcm-ai.lua"
dofile "lua/ai/sp-ai.lua"
dofile "lua/ai/wisdom-ai.lua"
dofile "lua/ai/joy-ai.lua"

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
dofile "lua/ai/hulaoguan-ai.lua"

dofile "lua/ai/guanxing-ai.lua"
