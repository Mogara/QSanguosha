-- This is the Smart AI, and it should load at the server side

require "middleclass"

sgs.ai_classes = {}

function CloneAI(player, specialized)	
	if specialized then
		local ai_class = sgs.ai_classes[player:getGeneralName()]
		if ai_class then
			return ai_class(player).lua_ai
		end
	end
	
	return SmartAI(player).lua_ai
end

SmartAI = class "SmartAI" 

function SmartAI:initialize(player)
	self.player = player
	self.room = player:getRoom()
	self.lua_ai = sgs.LuaAI(player)	
	self.lua_ai.callback = function(method_name, ...)
		local method = self[method_name]
		if method then
			return method(self, ...)
		end
	end	
end

function SmartAI:relationTo(other)
	local relation = self.lua_ai:relationTo(other)
	if relation == sgs.AI_Friend then
		return "friend"
	elseif relation == sgs.AI_Enemy then
		return "enemy"
	else
		return "neutrality"
	end
end

function SmartAI:isFriend(other)
	return self.lua_ai:isFriend(other)
end

function SmartAI:isEnemy(other)
	return self.lua_ai:isEnemy(other)
end

function SmartAI:isNeutrality(other)
	return self.lua_ai:relationTo(other) == sgs.AI_Neutrality
end

function SmartAI:askForSkillInvoke(skill_name, data)
	if skill_name == "jianxiong" then
		return true
	end
end

function SmartAI:activate(use)
	local cards = self.player:getHandcards()
	for i=0, cards:length()-1 do
		local card = cards:at(i)
		
		if card:inherits("Slash") then
			local enemies = self.lua_ai:getEnemies()
			for j=0, enemies:length()-1 do
				local enemy = enemies:at(j)
				if self.player:canSlash(enemy, true) then
					use.card = card
					use.from = self.player
					use.to:append(enemy)
					return
				end
			end
		end
	end
end