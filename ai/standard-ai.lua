-- the general AI in standard package

-- Cao Cao's AI
local caocao_ai = class("CaocaoAI", SmartAI)

function caocao_ai:initialize(player)
	super.initialize(self, player)
end

function caocao_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "jianxiong" then
		return not sgs.Shit_HasShit(data:toCard())
	elseif skill_name == "hujia" then
		local cards = self.player:getHandcards()
		for i=0, cards:length()-1 do
			if cards:at(i):inherits("Jink") then
				return false
			end
		end
		return true		
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

-- Zhang Liao's AI
local zhangliao_ai = class("ZhangliaoAI", SmartAI)

function zhangliao_ai:initialize(player)
	super.initialize(self, player)
end

function zhangliao_ai:askForUseCard(pattern, prompt)
	if pattern == "@@tuxi" then
		self:sort(self.enemies, "handcard")
		
		local first_index
		for i=1, #self.enemies-1 do
			if not self.enemies[i]:isKongcheng() then
				first_index = i
				break
			end
		end
		
		if not first_index then
			return "."
		end
		
		local first = self.enemies[first_index]:objectName()
		local second = self.enemies[first_index + 1]:objectName()
		return ("@TuxiCard=.->%s+%s"):format(first, second)
	else
		return super.askForUseCard(self, pattern, prompt)
	end
end

-- Guo Jia's AI
local guojia_ai = class("GuojiaAI", SmartAI)

function guojia_ai:initialize(player)
	super.initialize(self, player)	
end

function guojia_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "yiji" then
		return true
	elseif skill_name == "tiandu" then
		return not sgs.Shit_HashShit(data:toCard())
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

-- Xiahou Dun's AI

local xiahoudun_ai = class("XiahoudunAI", SmartAI)

function xiahoudun_ai:initialize(player)
	super.initialize(self, player)	
end

function xiahoudun_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "ganglie" then
		return not self:isFriend(data:toPlayer())
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

-- Sima Yi's AI
local simayi_ai = class("SimayiAI", SmartAI)

function simayi_ai:initialize(player)
	super.initialize(self, player)
end

function simayi_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "fankui" then
		return not self:isFriend(data:toPlayer())
	else
		return super.askForSkillInvoke(skill_name, data)
	end
end

local zhenji_ai = class("ZhenjiAI", SmartAI)

function zhenji_ai:initialize(player)
	super.initialize(self, player)
end

function zhenji_ai:askForCard(pattern)
	if pattern == "jink" then
		local cards = self.player:getHandcards()
		for i=0, cards:length()-1 do
			local card = cards:at(i)
			if card:isBlack() then
				local suit = card:getSuitString()
				local number = card:getNumberString()
				local card_id = card:getEffectiveId()
				return ("jink:qingguo[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	end
	
	return super.askForCard(self, pattern)	
end

sgs.ai_classes["caocao"] = caocao_ai
sgs.ai_classes["zhangliao"] = zhangliao_ai
sgs.ai_classes["guojia"] = guojia_ai
sgs.ai_classes["xiahoudun"] = xiahoudun_ai
sgs.ai_classes["simayi"] = simayi_ai
sgs.ai_classes["zhenji"] = zhenji_ai