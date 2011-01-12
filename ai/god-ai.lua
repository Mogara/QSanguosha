local shencaocao_ai = class("ShencaocaoAI", SmartAI)

function shencaocao_ai:initialize(player)
	super.initialize(self, player)
end

function shencaocao_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "guixin" then
		return true
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

sgs.ai_classes["shencaocao"] = shencaocao_ai