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

function zhangliao_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "tuxi" then
		local enemies = self.lua_ai:getEnemies()
		local n = 0
		for i=0, enemies:length()-1 do
			if enemies:at(i):getHandcardNum() > 0 then
				n = n + 1
			end
		end	
		
		return n >= 2
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end	
end

function zhangliao_ai:askForUseCard(pattern, prompt)
	if pattern == "@@tuxi" then
		local enemies = self.lua_ai:getEnemies()
		enemies:sortByHandcard()
		
		local first_index = 0
		for i=0, enemies:length()-1 do
			if enemies:at(i):getHandcardNum() > 0 then
				first_index = i
				break;
			end
		end
		
		local first = enemies:at(first_index)
		local second = enemies:at(first_index + 1)
		return "@TuxiCard=.->" .. first:objectName() .. "+" .. second:objectName()
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
		return not self.lua_ai:isFriend(data:toPlayer())
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

sgs.ai_classes["caocao"] = caocao_ai
sgs.ai_classes["zhangliao"] = zhangliao_ai
sgs.ai_classes["guojia"] = guojia_ai
sgs.ai_classes["xiahoudun"] = xiahoudun_ai