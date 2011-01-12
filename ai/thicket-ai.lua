-- Menghuo's AI

local menghuo_ai = class("MenghuoAI", SmartAI)

function menghuo_ai:initialize(player)
	super.initialize(self, player)
end

function menghuo_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "zaiqi" then
		return self.player:getLostHp() >= 2
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

-- Sunjian's AI

local sunjian_ai = class("SunjianAI", SmartAI)

function sunjian_ai:initialize(player)
	super.initialize(self, player)
end

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
local dongzhuo_ai = class("SunjianAI", SmartAI)

function dongzhuo_ai:initialize(player)
	super.initialize(self, player)
end

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

sgs.ai_classes["menghuo"] = menghuo_ai
sgs.ai_classes["sunjian"] = sunjian_ai
sgs.ai_classes["dongzhuo"] = dongzhuo_ai
