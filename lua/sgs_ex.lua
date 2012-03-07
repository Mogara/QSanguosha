-- this script file defines all functions written by Lua

-- trigger skills
function sgs.CreateTriggerSkill(spec)
	assert(type(spec.name) == "string")
	assert(type(spec.on_trigger) == "function")

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
	
	if spec.view_as_skill then
		skill:setViewAsSkill(spec.view_as_skill)
	end

	if type(spec.priority) == "number" then
		skill.priority = spec.priority
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

function sgs.CreateProhibitSkill(spec)
	assert(type(spec.name) == "string")
	assert(type(spec.is_prohibited) == "function")
	
	local skill = sgs.LuaProhibitSkill(spec.name)	
	skill.is_prohibited = spec.is_prohibited
	
	return skill
end

function sgs.CreateFilterSkill(spec)
	assert(type(spec.name) == "string")
	assert(type(spec.view_filter) == "function")
	assert(type(spec.view_as) == "function")

	local skill = sgs.LuaFilterSkill(spec.name)
	skill.view_filter = spec.view_filter
	skill.view_as = spec.view_as

	return skill
end

function sgs.CreateDistanceSkill(spec)
	assert(type(spec.name) == "string")
	assert(type(spec.correct_func) == "function")

	local skill = sgs.LuaDistanceSkill(spec.name)
	skill.correct_func = spec.correct_func

	return skill
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
	
	if type(spec.target_fixed) == "boolean" then
		card:setTargetFixed(spec.target_fixed)
	end

	if type(spec.will_throw) == "boolean" then
		card:setWillThrow(spec.will_throw)	
	end
	
	card.filter = spec.filter
	card.feasible = spec.feasible
	card.on_use = spec.on_use
	card.on_effect = spec.on_effect
	
	return card
end

function sgs.CreateViewAsSkill(spec)
	assert(spec.name)
	
	local skill = sgs.LuaViewAsSkill(spec.name)
	local n = spec.n or 0
	
	function skill:view_as(cards)
			return spec.view_as(self,cards)
	end
	
	function skill:view_filter(selected, to_select)
			if #selected>=n then
				return false
			end
			
			return spec.view_filter(self, selected, to_select)
	end
	
	skill.enabled_at_play = spec.enabled_at_play
	skill.enabled_at_response = spec.enabled_at_response
	skill.enabled_at_nullification = spec.enabled_at_nullification
	
	return skill
end

function sgs.LoadTranslationTable(t)
	for key, value in pairs(t) do
		sgs.AddTranslationEntry(key, value)		
	end
end

-- utilities, i.e: convert QList<const Card> to Lua's native table
function sgs.QList2Table(qlist)
	local t = {}
	for i=0, qlist:length()-1 do
		table.insert(t, qlist:at(i))
	end

	return t
end

-- the iterator of QList object
local qlist_iterator = function(list, n)
	if n < list:length()-1 then
		return n+1, list:at(n+1) -- the next element of list
	end
end

function sgs.qlist(list)
	return qlist_iterator, list, -1
end

-- more general iterator
function sgs.list(list)
	if type(list) == "table" then
		return ipairs(list)
	else
		return sgs.qlist(list)
	end
end	

function sgs.reverse(list)
	local new = {}
	for i=#list, 1, -1 do
		table.insert(new, list[i])
	end
	
	return new
end

-- copied from "Well House Consultants"
-- used to split string into a table, similar with php' explode function
function string:split(delimiter)
  local result = { }
  local from  = 1
  local delim_from, delim_to = string.find( self, delimiter, from  )
  while delim_from do
    table.insert( result, string.sub( self, from , delim_from-1 ) )
    from  = delim_to + 1
    delim_from, delim_to = string.find( self, delimiter, from  )
  end
  table.insert( result, string.sub( self, from  ) )
  return result
end

function table:contains(element)
	if #self == 0 or type(self[1]) ~= type(element) then return false
	end
	
	for _, e in ipairs(self) do
		if e == element then return true end
	end
end

function table:removeOne(element)
	if #self == 0 or type(self[1]) ~= type(element) then return false end
	
	for i=1, #self do
		if self[i] == element then 
			table.remove(self, i)
			return true
		end
	end
	return false
end

function table:removeAll(element)
	if #self == 0 or type(self[1]) ~= type(element) then return 0 end
	local n = 0
	for i=1, #self do
		if self[i] == element then 
			table.remove(self, i)
			n = n + 1
		end
	end
	return n
end

function table:insertTable(list)
	for _, e in ipairs(list) do
		table.insert(self, e)
	end
end

function table:removeTable(list)
	for _, e in ipairs(list) do
		table.removeAll(self,e)
	end
end

function table.copyFrom(list)
	local l = {}
	for _, e in ipairs(list) do
		table.insert(l, e)
	end
	return l
end
