-- Menghuo's AI

local menghuo_ai = SmartAI:newSubclass "menghuo"

function menghuo_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "zaiqi" then
		return self.player:getLostHp() >= 2
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

-- Sunjian's AI

local sunjian_ai = SmartAI:newSubclass "sunjian"

function sunjian_ai:askForChoice(skill_name, choices)
	if skill_name == "yinghun" then
		if self:isFriend(self.yinghun) then
			return "dxt1"
		else
			return "d1tx"
		end
	else
		return super.askForChoice(skill_name, choices)
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

		return "@YinghunCard=.->" .. self.yinghun:objectName()
    end
end

-- Dong Zhuo's AI
local dongzhuo_ai = SmartAI:newSubclass "dongzhuo"

function dongzhuo_ai:askForChoice(skill_name, choice)
	if skill_name == "benghuai" then
		if self.player:getLostHp() >= 2 then
			return "maxhp"
		else
			return "hp"
		end
	else
		return super.askForChoice(self, skill_name, choice)
	end
end

local caopi_ai = SmartAI:newSubclass "caopi"

-- xingshang, allways invoke 
sgs.ai_skill_invoke.xingshang = true

function caopi_ai:askForUseCard(pattern, prompt)
	if pattern == "@@fangzhu" then
		self:sort(self.friends)

		local target
		for _, friend in ipairs(self.friends) do
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
	else
		return super.askForUseCard(self, pattern, prompt)
	end
end

local xuhuang_ai = SmartAI:newSubclass "xuhuang"

function xuhuang_ai:activate(use)
	-- find black basic or equip card
	local cards = self.player:getCards("he")
	local to_use
	for i=0, cards:length()-1 do
		local card = cards:at(i)
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

		self:useCardByClassName(card, use)
		if use:isValid() then
			return
		end
	end

	super.activate(self, use)
end

-- baonue
sgs.ai_skill_invoke.baonue = function(self, data)
	return self.player:getRole() == "loyalist"
end

-- haoshi
sgs.ai_skill_invoke.haoshi = function(self, data)
	if self.player:getHandcardNum() <= 1 then
		return true
	end

	local least = 1000
	local players = self.room:getOtherPlayers(self.player)
	for i=0, players:length()-1 do
		least = math.min(players:at(i):getHandcardNum(), least)		
	end

	self:sort(self.friends)
	for _, friend in ipairs(self.friends) do
		if friend:getHandcardNum() == least then
			return true
		end
	end

	return false
end