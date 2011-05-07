-- this scripts contains the AI classes for generals of fire package

-- wolong

-- bazhen
sgs.ai_skill_invoke.bazhen = true


-- pangtong

local pangtong_ai = SmartAI:newSubclass "pangtong"

-- niepan
sgs.ai_skill_invoke.niepan = function(self, data)
	local dying = data:toDying()
	local peaches = 1 - dying.who:getHp()

	local cards = self.player:getHandcards()
	local n = 0
	for _, card in sgs.qlist(cards) do
		if card:inherits "Peach" or card:inherits "Analeptic" then
			n = n + 1
		end
	end

	return n < peaches
end

-- lianhuan
function pangtong_ai:activate_dummy(use)
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getSuit() == sgs.Card_Club then
			local number = card:getNumber()
			local card_id = card:getEffectiveId()
			local card_str = ("iron_chain:lianhuan[club:%d]=%d"):format(number, card_id)
			local interlink = sgs.Card_Parse(card_str)
			self:useCardIronChain(interlink, use)
			return
		end
	end

	super.activate(self, use)
end

local xunyu_ai = SmartAI:newSubclass "xunyu"
xunyu_ai:setOnceSkill("quhu")

function xunyu_ai:activate(use)
	if not self.quhu_used and not self.player:isKongcheng() then
		local max_card = self:getMaxCard()
		local max_point = max_card:getNumber()
		self:sort(self.enemies, "handcard")

		for _, enemy in ipairs(self.enemies) do
			if enemy:getHp() > self.player:getHp() then
				local enemy_max_card = self:getMaxCard(enemy)
				if enemy_max_card and max_point > enemy_max_card:getNumber() then
					for _, enemy2 in ipairs(self.enemies) do
						if (enemy:objectName() ~= enemy2:objectName()) and enemy:inMyAttackRange(enemy2) then
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
	end

	super.activate(self, use)
end

function xunyu_ai:askForPlayerChosen(players, reason)
	if reason == "quhu" then
		for _, player in sgs.qlist(players) do
			if self:isEnemy(player) then
				return player
			end
		end
	end

	return super.askForPlayerChosen(self, players, reason)
end

sgs.ai_skill_use["@@jieming"] = function(self, prompt)
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
end

-- mengjin
sgs.ai_skill_invoke.mengjin = function(self, data)
	local effect = data:toSlashEffect()
	return not self:isFriend(effect.to)
end

local dianwei_ai = SmartAI:newSubclass "dianwei"
dianwei_ai:setOnceSkill "qiangxi"

function dianwei_ai:activate(use)
	if not self.qiangxi_used then
		local weapon = self.player:getWeapon()
		if weapon then
			local hand_weapon, cards
			cards = self.player:getHandcards()
			for _, card in sgs.qlist(cards) do
				if card:inherits("Weapon") then
					hand_weapon = card
					break
				end
			end

			self:sort(self.enemies)
			for _, enemy in ipairs(self.enemies) do
				if hand_weapon and self.player:inMyAttackRange(enemy) then
					use.card = sgs.Card_Parse("@QiangxiCard=" .. hand_weapon:getId())
					use.to:append(enemy)

					self.qiangxi_used = true
					break
				end

				if self.player:distanceTo(enemy) <= 1 then
					use.card = sgs.Card_Parse("@QiangxiCard=" .. weapon:getId())
					use.to:append(enemy)

					self.qiangxi_used = true

					return
				end
			end
		else
			self:sort(self.enemies, "hp")
			for _, enemy in ipairs(self.enemies) do
				if self.player:inMyAttackRange(enemy) and self.player:getHp() > enemy:getHp() then
					use.card = sgs.Card_Parse("@QiangxiCard=.")
					use.to:append(enemy)

					self.qiangxi_used = true

					return
				end
			end
		end
	end

	super.activate(self, use)
end

--shuangxiong

sgs.ai_skill_invoke["shuangxiong"]=function(self,data)
    local handnum=self.player:getHandcardNum()/2
    self:sort(self.enemies, "hp")
    for _, enemy in ipairs(self.enemies) do
        if (self:getSlashNumber(enemy)+enemy:getHp()<=handnum) and (self:getSlashNumber(self.player)>=self:getSlashNumber(enemy)) then return true end
    end
    return false
end