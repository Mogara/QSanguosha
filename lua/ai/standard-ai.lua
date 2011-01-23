
-- jianxiong
sgs.ai_skill_invoke.jianxiong = function(self, data)
	return not sgs.Shit_HasShit(data:toCard())
end

-- hujia
sgs.ai_skill_invoke.hujia = function(self, data)
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("Jink") then
			return false
		end
	end
	return true	
end

-- tuxi
sgs.ai_skill_use["@@tuxi"] = function(self, prompt)
	self:sort(self.enemies, "handcard")
	
	local first_index
	for i=1, #self.enemies-1 do
		if not self.enemies[i]:isKongcheng() then
			first_index = i
			break
		end
	end
	
	if not first_index then
		return "."
	end
	
	local first = self.enemies[first_index]:objectName()
	local second = self.enemies[first_index + 1]:objectName()
	return ("@TuxiCard=.->%s+%s"):format(first, second)
end

-- yiji (frequent)

-- tiandu, same as jianxiong
sgs.ai_skill_invoke.tiandu = sgs.ai_skill_invoke.jianxiong

-- ganglie
sgs.ai_skill_invoke.ganglie = function(self, data) return not self:isFriend(data:toPlayer()) end

-- fankui 
sgs.ai_skill_invoke.fankui = function(self, data) 
	local target = data:toPlayer()
	if self:isFriend(target) then
		return target:hasSkill("xiaoji") and not target:getEquips():isEmpty()
	else
		return true
	end
end

local zhenji_ai = SmartAI:newSubclass "zhenji"

function zhenji_ai:askForCard(pattern)
	if pattern == "jink" then
		local cards = self.player:getHandcards()		
		for _, card in sgs.qlist(cards) do			
			if card:isBlack() then
				local suit = card:getSuitString()
				local number = card:getNumberString()
				local card_id = card:getEffectiveId()
				return ("jink:qingguo[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	end
	
	return super.askForCard(self, pattern)	
end

local guanyu_ai = SmartAI:newSubclass "guanyu"

function guanyu_ai:askForCard(pattern)
	if pattern == "slash" then
		local cards = self.player:getCards("he")
		for _, card in sgs.qlist(cards) do
			if card:isRed() then
				local suit = card:getSuitString()
				local number = card:getNumberString()
				local card_id = card:getEffectiveId()
				return ("slash:wusheng[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	end

	return super.askForCard(self, pattern)
end

local zhaoyun_ai = SmartAI:newSubclass "zhaoyun"

function zhaoyun_ai:askForCard(pattern)
	if pattern == "jink" then
		local cards = self.player:getHandcards()		
		for _, card in sgs.qlist(cards) do			
			if card:inherits("Slash") then
				local suit = card:getSuitString()
				local number = card:getNumberString()
				local card_id = card:getEffectiveId()
				return ("jink:longdan[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	elseif pattern == "slash" then
		local cards = self.player:getHandcards()		
		for _, card in sgs.qlist(cards) do
			if card:inherits("Jink") then
				local suit = card:getSuitString()
				local number = card:getNumberString()
				local card_id = card:getEffectiveId()
				return ("slash:longdan[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	end
	
	return super.askForCard(self, pattern)	
end

-- tieji
sgs.ai_skill_invoke.tieji = function(self, data) 
	local effect = data:toSlashEffect()
	return not self:isFriend(effect.to) 
end

local zhouyu_ai = SmartAI:newSubclass "zhouyu"
zhouyu_ai:setOnceSkill "fanjian"

function zhouyu_ai:activate(use)
	super.activate(self, use)

	if not use:isValid() and not self.fanjian_used and not self.player:isKongcheng() and next(self.enemies) then
		local cards = self.player:getHandcards()
		local should_fanjian = true
		for _, card in sgs.qlist(cards) do
			if card:getSuit() == sgs.Card_Diamond or card:inherits("Peach") or card:inherits("Analeptic") then
				should_fanjian = false
			end
		end

		if should_fanjian then
			self:sort(self.enemies)
			
			use.card = sgs.Card_Parse("@FanjianCard=.")
			use.to:append(self.enemies[1])

			self.fanjian_used = true

			return		
		end
	end
end

local sunshangxiang_ai = SmartAI:newSubclass "sunshangxiang"
sunshangxiang_ai:setOnceSkill("jieyin")

function sunshangxiang_ai:activate(use)
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("EquipCard") then
			use.card = card
			return
		end
	end

	self:sort(self.friends, "hp")
	local target
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() and friend:isWounded() then
			target = friend
			break
		end
	end

	if not self.jieyin_used and target and self.player:getHandcardNum()>=2 then
		local cards = self.player:getHandcards()
		local first = cards:at(0):getEffectiveId()
		local second = cards:at(1):getEffectiveId()

		local card_str = ("@JieyinCard=%d+%d"):format(first, second)
		use.card = sgs.Card_Parse(card_str)
		use.to:append(target)

		self.jieyin_used = true

		return
	end

	super.activate(self, use)
end

local ganning_ai = SmartAI:newSubclass "ganning"

function ganning_ai:activate(use)
	local cards = self.player:getCards("he")	
	local black_card
	for _, card in sgs.qlist(cards) do
		if card:isBlack() then
			black_card = card
			break
		end
	end

	if black_card then		
		local suit = black_card:getSuitString()
		local number = black_card:getNumberString()
		local card_id = black_card:getEffectiveId()
		local card_str = ("dismantlement:qixi[%s:%s]=%d"):format(suit, number, card_id)
		local dismantlement = sgs.Card_Parse(card_str)
		
		assert(dismantlement)

		if not self.has_wizard then
			-- find lightning

			local players = self.room:getOtherPlayers(self.player)
			for _, player in sgs.qlist(players) do
				if player:containsTrick("lightning") then
					use.card = dismantlement
					use.to:append(player)
					return
				end
			end			
		end

		self:sort(self.friends_noself)
		for _, friend in ipairs(self.friends_noself) do
			if friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage") then
				use.card = dismantlement
				use.to:append(friend)

				return
			end			
		end		
		
		self:sort(self.enemies)
		for _, enemy in ipairs(self.enemies) do
			local equips = enemy:getEquips()
			if not equips:isEmpty() then
				use.card = dismantlement
				use.to:append(enemy)

				return
			end
		end		
	end

	super.activate(self, use)
end

local daqiao_ai = SmartAI:newSubclass "daqiao"

function daqiao_ai:activate(use)
	super.activate(self, use)
	if use:isValid() then
		return
	end

	local cards = self.player:getCards("he")
	for _, card in sgs.qlist(cards) do
		if card:getSuit() == sgs.Card_Diamond then
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local card_str = ("indulgence:guose[diamond:%s]=%d"):format(number, card_id)
			
			local indulgence = sgs.Card_Parse(card_str)
			
			self:useCardIndulgence(indulgence, use)
			
			if use:isValid() then
				return
			end			
		end
	end


end

local huatuo_ai = SmartAI:newSubclass "huatuo"
huatuo_ai:setOnceSkill("qingnang")

local black_before_red = function(a, b)
	local color1 = a:isBlack() and 0 or 1
	local color2 = b:isBlack() and 0 or 1

	if color1 ~= color2 then
		return color1 < color2
	else
		return a:getNumber() < b:getNumber()
	end
end

function huatuo_ai:activate(use)
	if not self.qingnang_used and not self.player:isKongcheng() then
		self:sort(self.friends, "hp")
		local most_misery = self.friends[1]

		if most_misery:isWounded() then
			local cards = self.player:getHandcards()
			cards = sgs.QList2Table(cards)
			table.sort(cards, black_before_red)
			local card_id = cards[1]:getEffectiveId()

			use.card = sgs.Card_Parse("@QingnangCard=" .. card_id)
			use.to:append(most_misery)
			self.qingnang_used = true

			return
		end
	end

	super.activate(self, use)
end

local diaochan_ai = SmartAI:newSubclass "diaochan"
diaochan_ai:setOnceSkill("lijian")

function diaochan_ai:activate(use)
	if not self.lijian_used and not self.player:isNude() then
		self:sort(self.enemies, "hp")
		local males = {}
		local first, second
		for _, enemy in ipairs(self.enemies) do
			if enemy:getGeneral():isMale() then
				table.insert(males, enemy)

				if #males == 2 then
					first = males[1]
					second = males[2]
					break
				end
			end
		end

		if first and second then
			local card_id = self:getCardRandomly(self.player, "he")
			use.card = sgs.Card_Parse("@LijianCard=" .. card_id)
			use.to:append(first)
			use.to:append(second)

			self.lijian_used = true
			return
		end
	end

	super.activate(self, use)
end

