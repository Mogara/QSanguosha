require "middleclass"

--- a SkillCard class
-- @example
--  HujiaCard = class("HujiaCard", SkillCard, {
--    initialize = function(self)
--      SkillCard.initialize(self)
--      self.name = "hujia"
--      self.target_fixed = true
--    end,
--    
--    on_use = function(self, card, room, source, targets)
--      ...
--    end
--  })
--  hujia_card = HujiaCard:create() -- need at file scope
SkillCard = class("SkillCard", {
  static = {
    create = function(self)
      return self:new():to_card()
    end,
  },

  initialize = function(self)
    self.name = nil
    self.target_fixed = false
    self.will_throw = true
  end,

  to_card = function(self)
    assert(type(self.name) == "string")

    local card = sgs.LuaSkillCard(self.name)
    card:setTargetFixed(self.target_fixed)
    card:setWillThrow(self.will_throw)
	
    local methods = {"filter", "feasible", "on_use", "on_effect"}
    for _,method in ipairs(methods) do
      if self[method] then
        card[method] = function(...)
          return self[method](self, ...)
        end
      end
    end

    return card
  end
})

Skill = class("Skill", {
  static = {
    create = function(self)
      return self:new():to_skill()
    end,
  },

  initialize = function(self)
    self.name = nil
    self.frequency = sgs.Skill_NotFrequent
  end,
})

--- a ViewAsSkill class
-- @example
--   Foo = class("Foo", ViewAsSkill, {
--     initialize = function(self)
--       ViewAsSkill.initialize(self)
--       self.name = "foo"
--       self.n = 1
--     end,
--
--     view_as = function(self, skill, cards)
--       ...
--     end
--   })
--   foo = Foo:create() -- need at file scope
--   caocao:addSkill(foo)
ViewAsSkill = class("ViewAsSkill", Skill, {
  initialize = function(self)
    Skill.initialize(self)
    self.n = 0
  end,

  to_skill = function(self)
    assert(type(self.name) == "string")

    local skill = sgs.LuaViewAsSkill(self.name)

    skill.view_as = function(this, cards)
      return self.view_as(self, this, cards)
    end

    skill.view_filter = function(this, selected, to_select)
      if #selected >= self.n then
        return false
      end

      return self.view_filter(self, this, selected, to_select)
    end

    local methods = {"enabled_at_play", "enabled_at_response", "enabled_at_nullification"}
    for _,method in ipairs(methods) do
      if self[method] then
        skill[method] = function(...)
          return self[method](self, ...)
        end
      end
    end

    return skill
  end
})


TriggerSkill = class("TriggerSkill", Skill, {
  initialize = function(self)
    Skill.initialize(self)
    self.events = {}
    self.priority = 1
  end,

  to_skill = function(self)
    assert(type(self.name) == "string")
    assert(type(self.on_trigger) == "function")

    local skill = sgs.LuaTriggerSkill(self.name, self.frequency)
    skill.priority = self.priority

    if type(self.events) == "number" then
      skill:addEvent(self.events)
    elseif type(self.events) == "table" then
      for _, event in ipairs(self.events) do
        skill:addEvent(event)
      end
    end

    if self.view_as_skill then
      skill:setViewAsSkill(self.view_as_skill)
    end

    local methods = {"on_trigger", "can_trigger"}
    for _,method in ipairs(methods) do
      if self[method] then
        skill[method] = function(...)
          return self[method](self, ...)
        end
      end
    end
    
    return skill
  end
})


ProhibitSkill = class("ProhibitSkill", Skill, {
  to_skill = function(self)
    assert(type(self.name) == "string")
    assert(type(self.is_prohibited) == "function")
    
    local skill = sgs.LuaProhibitSkill(self.name)	

    skill.is_prohibited = function(...)
      return self.is_prohibited(self, ...)
    end

    return skill
  end
})

DistanceSkill = class("DistanceSkill", Skill, {
  to_skill = function(self)
    assert(type(self.name) == "string")
    assert(type(self.correct_func) == "function")

    local skill = sgs.LuaDistanceSkill(self.name)

    skill.correct_func = function(...)
      return self.correct_func(self, ...)
    end

    return skill
  end
})

MaxCardsSkill = class("MaxCardsSkill", Skill, {
  to_skill = function(self)
    assert(type(self.name) == "string")
    assert(type(self.extra_func) == "function")

    local skill = sgs.LuaMaxCardsSkill(self.name)

    skill.extra_func = function(...)
      return self.extra_func(self, ...)
    end

    return skill
  end
})

GameStartSkill = class("GameStartSkill", TriggerSkill, {
  initialize = function(self)
    TriggerSkill.initialize(self)
    self.events = {sgs.GameStart}
  end,

  on_trigger = function(self, skill, event, player, data)
    self.on_gamestart(self, skill, player)
    return false
  end,

  to_skill = function(self)
    assert(type(self.on_gamestart) == "function")

    return TriggerSkill.to_skill(self) 
  end
})

MasochismSkill = class("MasochismSkill", TriggerSkill, {
  initialize = function(self)
    TriggerSkill.initialize(self)
    self.events = {sgs.Damaged}
    self.priority = -1
  end,

  on_trigger = function(self, skill, event, player, data)
    self.on_damaged(self, skill, player)
    return false
  end,

  to_skill = function(self)
    assert(type(self.on_damaged) == "function")
	
    return TriggerSkill.to_skill(self)
  end
})
