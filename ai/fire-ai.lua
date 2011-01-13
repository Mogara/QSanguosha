-- this scripts contains the AI classes for generals of fire package

-- wolong

local wolong_ai = SmartAI:newSubclass "wolong"

function wolong_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "bazhen" then
		return true
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

-- pangtong

local pangtong_ai = SmartAI:newSubclass "pangtong"

function pangtong_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "niepan" then
		local dying = data:toDying()
		local peaches = dying.peaches

		local cards = self.player:getHandcards()
		local n = 0
		for i=0, cards:length()-1 do
			local card = cards:at(i)
			if card:inherits "Peach" or card:inherits "Analeptic" then
				n = n + 1
			end
		end

		return n < peaches
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

local xunyu_ai = SmartAI:newSubclass "xunyu"

function xunyu_ai:askForUseCard(pattern, prompt)
	if pattern == "@@jieming" then
		self:sort(self.friends) 

		local max_x = 0
		local target
		for _, friend in ipairs(self.friends) do
			local x = friend:getMaxHP() - friend:getHandcardNum()
			x = math.min(5, x)

			if x > max_x then
				max_x = x
				target = friend
			end
		end

		if target then
			return "@JiemingCard=.->" .. target:objectName()
		else
			return "."
		end
	else
		return super.askForUseCard(self, pattern, prompt)
	end
end

local pangde_ai = SmartAI:newSubclass "pangde"

function pangde_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "mengjin" then
		local effect = data:toSlashEffect()
		assert(effect.to)
		return not self:isFriend(effect.to)
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end