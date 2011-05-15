-- zaiqi
sgs.ai_skill_invoke["zaiqi"] = function(self, data)
	return self.player:getLostHp() >= 2
end

-- Sunjian's AI

local sunjian_ai = SmartAI:newSubclass "sunjian"

sgs.ai_skill_choice.yinghun = function(self, choices)
	if self:isFriend(self.yinghun) then
		return "dxt1"
	else
		return "d1tx"
	end
end

function sunjian_ai:askForUseCard(pattern, prompt)
	if pattern == "@@yinghun" then        
		local x = self.player:getLostHp()
		if x == 1 and #self.friends == 1 then
			return "."
		end
	
        if #self.friends > 1 then
            self:sort(self.friends, "chaofeng")
            self.yinghun = self:getOneFriend()
        else
            self:sort(self.enemies, "chaofeng")
            self.yinghun = self.enemies[1]
        end
		
		if self.yinghun then
			return "@YinghunCard=.->" .. self.yinghun:objectName()
		else
			return "."
		end
    end
end

-- xingshang, allways invoke 
sgs.ai_skill_invoke.xingshang = true

-- fangzhu, fangzhu 
sgs.ai_skill_use["@@fangzhu"] = function(self, prompt)
	self:sort(self.friends_noself)
	local target
	for _, friend in ipairs(self.friends_noself) do
		if not friend:faceUp() then
			target = friend
			break
		end

		if friend:hasSkill("jushou") and friend:getPhase() == sgs.Player_Play then			
			target = friend
			break
		end
	end

	if not target then
		local x = self.player:getLostHp()
		if x >= 3 then
			target = self:getOneFriend()
		else
			self:sort(self.enemies)
			for _, enemy in ipairs(self.enemies) do
				if enemy:faceUp() then
					target = enemy
					break
				end
			end
		end
	end

	if target then
		return "@FangzhuCard=.->" .. target:objectName()
	else
		return "."
	end
end

local xuhuang_ai = SmartAI:newSubclass "xuhuang"

function xuhuang_ai:activate(use)
	-- find black basic or equip card
	local cards = self.player:getCards("he")
	local to_use
	for _, card in sgs.qlist(cards) do		
		if card:isBlack() and (card:inherits("BasicCard") or card:inherits("EquipCard")) then
			to_use = card
			break
		end
	end

	if to_use then
		local suit = to_use:getSuitString()
		local number = to_use:getNumberString()
		local card_id = to_use:getEffectiveId()
		local card_name = "supply_shortage"
		local skill_name = "duanliang"
		local card_str = ("%s:%s[%s:%s]=%d"):format(card_name, skill_name, suit, number, card_id)

		card = sgs.Card_Parse(card_str)

		self:useCardSupplyShortage(card, use)
		if use:isValid() then
			return
		end
	end

	super.activate(self, use)
end

sgs.ai_skill_invoke.songwei = function(self, data)
    return self:isFriend(self.room:getLord())
end

-- baonue
sgs.ai_skill_invoke.baonue = function(self, data)
	return self.player:getRole() == "loyalist"
end

function SmartAI:getBeggar()
	local least = math.huge
	local players = self.room:getOtherPlayers(self.player)
	for _, player in sgs.qlist(players) do
		least = math.min(player:getHandcardNum(), least)		
	end

	self:sort(self.friends_noself)
	for _, friend in ipairs(self.friends_noself) do
		if friend:getHandcardNum() == least then			
			return friend
		end
	end
end

-- haoshi
sgs.ai_skill_invoke.haoshi = function(self, data)
	if self.player:getHandcardNum() <= 1 then
		return true
	end

	if self:getBeggar() then
		return true
	else
		return false
	end
end

sgs.ai_skill_use["@@haoshi!"] = function(self, prompt)
	local beggar = self:getBeggar()
	
	local cards = self.player:getHandcards()
	local n = math.floor(self.player:getHandcardNum()/2)
	local card_ids = {}
	for i=1, n do
		table.insert(card_ids, cards:at(i-1):getEffectiveId())
	end
	
	return "@HaoshiCard=" .. table.concat(card_ids, "+") .. "->" .. beggar:objectName()
end

sgs.ai_skill_invoke.lieren = function(self, data)
    if self.player:getHandcardNum()>=self.player:getHp() then return true
    else return false
    end
end