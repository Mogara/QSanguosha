-- this scripts contains the AI classes for generals of fire package

-- wolong

-- bazhen
sgs.ai_skill_invoke.bazhen = true

-- pangtong

local pangtong_ai = SmartAI:newSubclass "pangtong"

-- niepan
sgs.ai_skill_invoke.niepan = function(self, data)
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
end

local xunyu_ai = SmartAI:newSubclass "xunyu"
xunyu_ai:setOnceSkill("quhu")

function xunyu_ai:activate(use)
	if not self.quhu_used and not self.player:isKongcheng() then
		local max_card = self:getMaxCard()
		local max_point = max_card:getNumber()
		self:sort(self.enemies, "handcard")

		for _, enemy in ipairs(self.enemies) do
			local enemy_max_card = self:getMaxCard(enemy)
			if enemy_max_card and max_point > enemy_max_card:getNumber() then
				for _, enemy2 in ipairs(self.enemies) do
					if enemy ~= enemy2 and enemy:inMyAttackRange(enemy2) then
						local card_id = max_card:getEffectiveId()
						local card_str = "@QuhuCard=" .. card_id
						use.card = sgs.Card_Parse(card_str)
						use.to:append(enemy)

						self.quhu_used = true

						return
					end
				end
			end
		end
	end

	super.activate(self, use)
end

function xunyu_ai:askForPlayerChosen(players, reason)
	if reason == "quhu" then
		for i=0, players:length()-1 do
			local player = players:at(i)
			if self:isEnemy(player) then
				return player
			end
		end
	end
	
	return super.askForPlayerChosen(self, players, reason)
end

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

-- mengjin
sgs.ai_skill_invoke.mengjin = sgs.ai_skill_invoke.tieji
