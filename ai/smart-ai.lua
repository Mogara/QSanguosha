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
	local skill = sgs.Sanguosha:getSkill(skill_name)
	if skill:getFrequency() == sgs.Skill_Frequent then
		return true
	end
end

function SmartAI:askForYiji(card_ids)
	return nil, 0
end

function SmartAI:askForUseCard(pattern, prompt)
	return "."
end

function SmartAI:slashIsEffective(slash, to)
	local weapon = self.player:getWeapon()
	if weapon and weapon:objectName() == "qinggang_sword" then
		return true
	end
	
	local armor = to:getArmor()
	if armor then
		if armor:objectName() == "renwang_shield" then
			return not slash:isBlack()
		elseif armor:inherits("Vine") then
			if slash:inherits("NatureSlash") then
				return true
			elseif weapon and weapon:inherits("Fan") then
				return true
			else
				return false
			end
		end		
	end
	
	return true
end

function SmartAI:slashHit(slash, to)
	
end

function SmartAI:useBasicCard(card, use)
	if card:inherits("Slash") then
		local enemies = self.lua_ai:getEnemies()
		enemies:sortByHp()
		for j=0, enemies:length()-1 do
			local enemy = enemies:at(j)			
			if self.player:canSlash(enemy, true) and
				self:slashIsEffective(card, enemy) then
				use.card = card
				local to = sgs.SPlayerList()
				to:append(enemy)
				use.to = to
				return
			end
		end
	elseif card:inherits("Peach") and self.player:isWounded() then
		use.card = card		
	end
end

function SmartAI:useTrickCard(card, use)
	if card:inherits("ExNihilo") then
		use.card = card
	elseif card:inherits("Collateral") then
	
	end
end

function SmartAI:useEquipCard(card, use)
	if self.lua_ai:useCard(card) then
		use.card = card
	end
end

function SmartAI:activate(use)
	local cards = self.player:getHandcards()
	for i=0, cards:length()-1 do
		local card = cards:at(i)
		local type = card:getTypeId()
		
		if type == sgs.Card_Basic then
			self:useBasicCard(card, use)
		elseif type == sgs.Card_Trick then
			self:useTrickCard(card, use)
		else
			self:useEquipCard(card, use)
		end
	end
end

dofile("ai/standard-ai.lua")