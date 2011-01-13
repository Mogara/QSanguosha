local huangzhong_ai = SmartAI:newSubclass "huangzhong"

function huangzhong_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "liegong" then
		local effect = data:toSlashEffect()
		assert(effect.to)
		return not self:isFriend(effect.to)
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

local caoren_ai = SmartAI:newSubclass "caoren"

function caoren_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "jushou" then
		return true
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end
