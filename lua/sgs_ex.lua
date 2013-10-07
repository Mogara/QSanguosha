-- this script file defines all functions written by Lua

-- trigger skills
function sgs.CreateTriggerSkill(spec)
	assert(type(spec.name) == "string")
	assert(type(spec.on_trigger) == "function")
	if spec.frequency then assert(type(spec.frequency) == "number") end
	if spec.limit_mark then assert(type(spec.limit_mark) == "string") end

	local frequency = spec.frequency or sgs.Skill_NotFrequent
	local limit_mark = spec.limit_mark or ""
	local skill = sgs.LuaTriggerSkill(spec.name, frequency, limit_mark)

	if type(spec.events) == "number" then
		skill:addEvent(spec.events)
	elseif type(spec.events) == "table" then
		for _, event in ipairs(spec.events) do
			skill:addEvent(event)
		end
	end
	
	if type(spec.global) == "boolean" then skill:setGlobal(spec.global) end

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

function sgs.CreateMaxCardsSkill(spec)
	assert(type(spec.name) == "string")
	assert(type(spec.extra_func) == "function")

	local skill = sgs.LuaMaxCardsSkill(spec.name)
	skill.extra_func = spec.extra_func

	return skill
end

function sgs.CreateTargetModSkill(spec)
	assert(type(spec.name) == "string")
	assert(type(spec.residue_func) == "function" or type(spec.distance_limit_func) == "function" or type(spec.extra_target_func) == "function")
	if spec.pattern then assert(type(spec.pattern) == "string") end

	local skill = sgs.LuaTargetModSkill(spec.name, spec.pattern or "Slash")
	if spec.residue_func then
		skill.residue_func = spec.residue_func
	end
	if spec.distance_limit_func then
		skill.distance_limit_func = spec.distance_limit_func
	end
	if spec.extra_target_func then
		skill.extra_target_func = spec.extra_target_func
	end

	return skill
end

function sgs.CreateMasochismSkill(spec)
	assert(type(spec.on_damaged) == "function")
	
	spec.events = sgs.Damaged
	
	function spec.on_trigger(skill, event, player, data)
		local damage = data:toDamage()
		spec.on_damaged(skill, player, damage)
		return false
	end
	
	return sgs.CreateTriggerSkill(spec)
end

function sgs.CreatePhaseChangeSkill(spec)
	assert(type(spec.on_phasechange) == "function")
	
	spec.events = sgs.EventPhaseStart
	
	function spec.on_trigger(skill, event, player, data)
		return spec.on_phasechange(skill, player)
	end
	
	return sgs.CreateTriggerSkill(spec)
end

function sgs.CreateDrawCardsSkill(spec)
	assert(type(spec.draw_num_func) == "function")
	
	spec.events = sgs.DrawNCards
	
	function spec.on_trigger(skill, event, player, data)
		local n = data:toInt()
		local nn = spec.draw_num_func(skill, player, n)
		data:setValue(nn)
		return false
	end
	
	return sgs.CreateTriggerSkill(spec)
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

--------------------------------------------

-- skill cards

function sgs.CreateSkillCard(spec)
	assert(spec.name)
	if spec.skill_name then assert(type(spec.skill_name) == "string") end

	local card = sgs.LuaSkillCard(spec.name, spec.skill_name)

	if type(spec.target_fixed) == "boolean" then
		card:setTargetFixed(spec.target_fixed)
	end

	if type(spec.will_throw) == "boolean" then
		card:setWillThrow(spec.will_throw)
	end

	if type(spec.can_recast) == "boolean" then
		card:setCanRecast(spec.can_recast)
	end
		
	if type(spec.handling_method) == "number" then
		card:setHandlingMethod(spec.handling_method)
	end

	card.filter = spec.filter
	card.feasible = spec.feasible
	card.about_to_use = spec.about_to_use
	card.on_use = spec.on_use
	card.on_effect = spec.on_effect
	card.on_validate = spec.on_validate
	card.on_validate_in_response = spec.on_validate_in_response

	return card
end

function sgs.CreateBasicCard(spec)
	assert(type(spec.name) == "string" or type(spec.class_name) == "string")
	if not spec.name then spec.name = spec.class_name
	elseif not spec.class_name then spec.class_name = spec.name end
	if spec.suit then assert(type(spec.suit) == "number") end
	if spec.number then assert(type(spec.number) == "number") end
	if spec.subtype then assert(type(spec.subtype) == "string") end
	local card = sgs.LuaBasicCard(spec.suit or sgs.Card_NoSuit, spec.number or 0, spec.name, spec.class_name, spec.subtype or "BasicCard")
	
	if type(spec.target_fixed) == "boolean" then
		card:setTargetFixed(spec.target_fixed)
	end

	if type(spec.can_recast) == "boolean" then
		card:setCanRecast(spec.can_recast)
	end

	card.filter = spec.filter
	card.feasible = spec.feasible
	card.available = spec.available
	card.about_to_use = spec.about_to_use
	card.on_use = spec.on_use
	card.on_effect = spec.on_effect

	return card
end

function isAvailable_AOE(self, player)
	local canUse = false;
	local players = player:getSiblings()
	for _, p in sgs.qlist(players) do
		if p:isDead() or player:isProhibited(p, self) then continue end
		canUse = true
		break
	end
	return canUse and self:cardIsAvailable(player);
end

function onUse_AOE(self, room, card_use)
	local source = card_use.from
	local targets = sgs.SPlayerList()
	local other_players = room:getOtherPlayers(source)
	for _, player in sgs.qlist(other_players) do
		local skill = room:isProhibited(source, player, self)
		if skill ~= nil then
			local log_message = sgs.LogMessage()
			log_message.type = "#SkillAvoid"
			log_message.from = player
			log_message.arg = skill:objectName()
			log_message.arg2 = self:objectName()
			room:broadcastSkillInvoke(skill:objectName())
		else
			targets:append(player)
		end
	end

	local use = card_use
	use.to = targets
	self:cardOnUse(room, use)
end

function isAvailable_GlobalEffect(self, player)
	local canUse = false;
	local players = player:getSiblings()
	players:append(player)
	for _, p in sgs.qlist(players) do
		if p:isDead() or player:isProhibited(p, self) then continue end
		canUse = true
		break
	end
	return canUse and self:cardIsAvailable(player);
end

function onUse_GlobalEffect(self, room, card_use)
	local source = card_use.from
	local targets = sgs.SPlayerList()
	local all_players = room:getAllPlayers()
	for _, player in sgs.qlist(all_players) do
		local skill = room:isProhibited(source, player, self)
		if skill ~= nil then
			local log_message = sgs.LogMessage()
			log_message.type = "#SkillAvoid"
			log_message.from = player
			log_message.arg = skill:objectName()
			log_message.arg2 = self:objectName()
			room:broadcastSkillInvoke(skill:objectName())
		else
			targets:append(player)
		end
	end

	local use = card_use
	use.to = targets
	self:cardOnUse(room, use)
end

function onUse_DelayedTrick(self, room, card_use)
	local use = card_use
	local wrapped = sgs.Sanguosha:getWrappedCard(self:getEffectiveId())
	use.card = wrapped

	local logm = sgs.LogMessage()
	logm.from = use.from
	logm.to = use.to
	logm.type = "#UseCard"
	logm.card_str = self:toString()
	room:sendLog(logm)

	local data = sgs.QVariant()
	data:setValue(use)
	local thread = room:getThread()
	thread:trigger(sgs.PreCardUsed, room, use.from, data)

	local reason = sgs.CardMoveReason(sgs.CardMoveReason_S_REASON_USE, use.from:objectName(), use.to:first():objectName(), self:getSkillName(), "")
	room:moveCardTo(self, use.from, use.to:first(), sgs.Player_PlaceDelayedTrick, reason, true)

	thread:trigger(sgs.CardUsed, room, use.from, data)
	thread:trigger(sgs.CardFinished, room, use.from, data)
end

function use_DelayedTrick(self, room, source, targets)
	if #targets == 0 then
		local reason = sgs.CardMoveReason(sgs.CardMoveReason_S_REASON_USE, source:objectName(), "", self:getSkillName(), "")
		room:moveCardTo(self, room:getCardOwner(self:getEffectiveId()), nil, sgs.Player_DiscardPile, reason, true)
	end
end

function sgs.CreateTrickCard(spec)
	assert(type(spec.name) == "string" or type(spec.class_name) == "string")
	if not spec.name then spec.name = spec.class_name
	elseif not spec.class_name then spec.class_name = spec.name end
	if spec.suit then assert(type(spec.suit) == "number") end
	if spec.number then assert(type(spec.number) == "number") end
	
	if spec.subtype then
		assert(type(spec.subtype) == "string")
	else
		local subtype_table = { "TrickCard", "single_target_trick", "delayed_trick", "aoe", "global_effect" }
		spec.subtype = subtype_table[(spec.subclass or 0) + 1]
	end
	
	local card = sgs.LuaTrickCard(spec.suit or sgs.Card_NoSuit, spec.number or 0, spec.name, spec.class_name, spec.subtype)

	if type(spec.target_fixed) == "boolean" then
		card:setTargetFixed(spec.target_fixed)
	end

	if type(spec.can_recast) == "boolean" then
		card:setCanRecast(spec.can_recast)
	end

	if type(spec.subclass) == "number" then
		card:setSubClass(spec.subclass)
	else
		card:setSubClass(sgs.LuaTrickCard_TypeNormal)
	end

	if spec.subclass then
		if spec.subclass == sgs.LuaTrickCard_TypeDelayedTrick then
			if not spec.about_to_use then spec.about_to_use = onUse_DelayedTrick end
			if not spec.on_use then spec.on_use = use_DelayedTrick end
		elseif spec.subclass == sgs.LuaTrickCard_TypeAOE then
			if not spec.available then spec.available = isAvailable_AOE end
			if not spec.about_to_use then spec.about_to_use = onUse_AOE end
			if not spec.target_fixed then card:setTargetFixed(true) end
		elseif spec.subclass == sgs.LuaTrickCard_TypeGlobalEffect then
			if not spec.available then spec.available = isAvailable_AOE end
			if not spec.about_to_use then spec.about_to_use = onUse_AOE end
			if not spec.target_fixed then card:setTargetFixed(true) end
		end
	end

	card.filter = spec.filter
	card.feasible = spec.feasible
	card.available = spec.available
	card.is_cancelable = spec.is_cancelable
	card.on_nullified = spec.on_nullified
	card.about_to_use = spec.about_to_use
	card.on_use = spec.on_use
	card.on_effect = spec.on_effect

	return card
end

function sgs.CreateViewAsSkill(spec)
	assert(type(spec.name) == "string")
	if spec.response_pattern then assert(type(spec.response_pattern) == "string") end
	local response_pattern = spec.response_pattern or ""

	local skill = sgs.LuaViewAsSkill(spec.name, response_pattern)
	local n = spec.n or 0

	function skill:view_as(cards)
		return spec.view_as(self, cards)
	end

	function skill:view_filter(selected, to_select)
		if #selected >= n then return false end
		return spec.view_filter(self, selected, to_select)
	end

	skill.enabled_at_play = spec.enabled_at_play
	skill.enabled_at_response = spec.enabled_at_response
	skill.enabled_at_nullification = spec.enabled_at_nullification

	return skill
end

function sgs.CreateOneCardViewAsSkill(spec)
	assert(type(spec.name) == "string")
	if spec.response_pattern then assert(type(spec.response_pattern) == "string") end
	local response_pattern = spec.response_pattern or ""
	if spec.filter_pattern then assert(type(spec.filter_pattern) == "string") end
	
	local skill = sgs.LuaViewAsSkill(spec.name, response_pattern)
	
	function skill:view_as(cards)
		if #cards ~= 1 then return nil end
		return spec.view_as(self, cards[1])
	end
	
	function skill:view_filter(selected, to_select)
		if #selected >= 1 or to_select:hasFlag("using") then return false end
		if spec.view_filter then return spec.view_filter(self, to_select) end
		if spec.filter_pattern then
			local pat = spec.filter_pattern
			if string.endsWith(pat, "!") then
				if sgs.Self:isJilei(to_select) then return false end
				pat = string.sub(pat, 1, -1)
			end
			return sgs.Sanguosha:matchExpPattern(pat, sgs.Self, to_select)
		end
	end
	
	skill.enabled_at_play = spec.enabled_at_play
	skill.enabled_at_response = spec.enabled_at_response
	skill.enabled_at_nullification = spec.enabled_at_nullification
	
	return skill
end

function sgs.CreateZeroCardViewAsSkill(spec)
	assert(type(spec.name) == "string")
	if spec.response_pattern then assert(type(spec.response_pattern) == "string") end
	local response_pattern = spec.response_pattern or ""
	
	local skill = sgs.LuaViewAsSkill(spec.name, response_pattern)
	
	function skill:view_as(cards)
		if #cards > 0 then return nil end
		return spec.view_as(self)
	end
	
	function skill:view_filter(selected, to_select)
		return false
	end
	
	skill.enabled_at_play = spec.enabled_at_play
	skill.enabled_at_response = spec.enabled_at_response
	skill.enabled_at_nullification = spec.enabled_at_nullification
	
	return skill
end

function sgs.CreateWeapon(spec)
	assert(type(spec.name) == "string" or type(spec.class_name) == "string")
	if not spec.name then spec.name = spec.class_name
	elseif not spec.class_name then spec.class_name = spec.name end
	if spec.suit then assert(type(spec.suit) == "number") end
	if spec.number then assert(type(spec.number) == "number") end
	assert(type(spec.range) == "number")
	local card = sgs.LuaWeapon(spec.suit or sgs.Card_NoSuit, spec.number or 0, spec.range, spec.name, spec.class_name)

	card.on_install = spec.on_install
	card.on_uninstall = spec.on_uninstall

	return card
end

function sgs.CreateArmor(spec)
	assert(type(spec.name) == "string" or type(spec.class_name) == "string")
	if not spec.name then spec.name = spec.class_name
	elseif not spec.class_name then spec.class_name = spec.name end
	if spec.suit then assert(type(spec.suit) == "number") end
	if spec.number then assert(type(spec.number) == "number") end
	local card = sgs.LuaArmor(spec.suit or sgs.Card_NoSuit, spec.number or 0, spec.name, spec.class_name)

	card.on_install = spec.on_install
	card.on_uninstall = spec.on_uninstall

	return card
end

function sgs.LoadTranslationTable(t)
	for key, value in pairs(t) do
		sgs.AddTranslationEntry(key, value)
	end
end