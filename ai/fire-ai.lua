-- this scripts contains the AI classes for generals of fire package

local wolong_ai = class("WolongAI", SmartAI)

function wolong_ai:initialize(player)
	super.initialize(self, player)
end

function wolong_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "bazhen" then
		return true
	else
		return super.askForSkillInvoke(skill_name, data)
	end
end


