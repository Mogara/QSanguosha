-- this script file defines all functions written by Lua

-- trigger skills
function sgs.CreateTriggerSkill(spec)
	local frequency = spec.frequency or sgs.Skill_NotFrequent
	local skill = sgs.LuaTriggerSkill(spec.name, frequency)
	
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
	assert(spec.name)
	
	local card = sgs.LuaSkillCard(spec.name)
	
	card:setTargetFixed(spec.target_fixed)
	card:setWillThrow(spec.will_throw)	
	
	card.available = spec.available
	card.filter = spec.filter
	card.feasible = spec.feasible
	card.on_use = spec.on_use
	card.on_effect = spec.on_effect
	
	return card
end

function sgs.CreateViewAsSkill(spec)
	assert(spec.name)
	
	local skill = sgs.LuaViewAsSkill(spec.name)
	local n = spec.n
	
	if not n then
		skill.view_filter = spec.view_filter
		skill.view_as = spec.view_as
	elseif n == 0 then
		function skill:view_as()
			return spec.view_as(self)
		end
	elseif n == 1 then
		function skill:view_filter(selected, to_select)
			if next(selected) then
				return false
			end
			
			return spec.view_filter(self, to_select)
		end
		
		function skill:view_as(cards)
			if #cards == 1 then
				local card = cards[1]
				return spec.view_as(self, card)
			end
		end
	end	
	
	skill.enabled_at_play = spec.enabled_at_play
	skill.enabled_at_response = spec.enabled_at_response
	
	return skill
end

