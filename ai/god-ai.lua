-- this script file contains the AI classes for gods

local shencaocao_ai = SmartAI:newSubclass "shencaocao"

function shencaocao_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "guixin" then
		return true
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

