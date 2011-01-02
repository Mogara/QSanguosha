-- this script file defines all functions written by Lua

-- trigger skills
function sgs.CreateTriggerSkill(spec)
	local name = spec.name
	local frequency = spec.frequency or sgs.Skill_NotFrequent
	local skill = sgs.LuaTriggerSkill("moligaloo", frequency)
	
	if(type(spec.events) == "number") then
		skill:addEvent(spec.events)
	elseif(type(spec.events) == "table") then
		for _, event in ipairs(spec.events) do
			skill:addEvent(event)
		end
	end
	
	skill.on_trigger = spec.on_trigger
	
	if spec.can_trigger then
		skill.can_trigger = spec.can_trigger
	end

	return skill
end

function sgs.CreateGameStartSkill(spec)
	assert(type(spec.on_gamestart) == "function")
	
	spec.events = sgs.GameStart
	
	function spec.on_trigger(skill, event, player, data)
		spec.on_gamestart(skill, player)
		return false
	end
	
	return sgs.CreateTriggerSkill(spec)
end

function sgs.CreateMasochismSkill(spec)
	assert(type(spec.on_damaged) == "function")
	
	spec.events = sgs.Damaged
	
	function spec.on_trigger(skill, event, player, data)
		spec.on_damaged(skill, player)
		return false		
	end
	
	return sgs.CreateTriggerSkill(spec)
end

--------------------------------------------

-- skill cards

function sgs.CreateSkillCard(spec)
	
end
