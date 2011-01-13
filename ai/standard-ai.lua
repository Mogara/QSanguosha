-- the general AI in standard package

-- Cao Cao's AI
local caocao_ai = SmartAI:newSubclass "caocao"

function caocao_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "jianxiong" then
		return not sgs.Shit_HasShit(data:toCard())
	elseif skill_name == "hujia" then
		local cards = self.player:getHandcards()
		for i=0, cards:length()-1 do
			if cards:at(i):inherits("Jink") then
				return false
			end
		end
		return true		
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

-- Zhang Liao's AI
local zhangliao_ai = SmartAI:newSubclass "zhangliao"

function zhangliao_ai:askForUseCard(pattern, prompt)
	if pattern == "@@tuxi" then
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
	else
		return super.askForUseCard(self, pattern, prompt)
	end
end

-- Guo Jia's AI
local guojia_ai = SmartAI:newSubclass "guojia"

function guojia_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "yiji" then
		return true
	elseif skill_name == "tiandu" then
		return not sgs.Shit_HasShit(data:toCard())
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

-- Xiahou Dun's AI

local xiahoudun_ai = SmartAI:newSubclass "xiahoudun"

function xiahoudun_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "ganglie" then
		return not self:isFriend(data:toPlayer())
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

-- Sima Yi's AI
local simayi_ai = SmartAI:newSubclass "simayi"

function simayi_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "fankui" then
		return not self:isFriend(data:toPlayer())
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

local zhenji_ai = SmartAI:newSubclass "zhenji"

function zhenji_ai:askForCard(pattern)
	if pattern == "jink" then
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		for _, card in ipairs(cards) do			
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
		cards = sgs.QList2Table(cards)
		for _, card in ipairs(cards) do
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
		cards = sgs.QList2Table(cards)
		for _, card in ipairs(cards) do			
			if card:inherits("Slash") then
				local suit = card:getSuitString()
				local number = card:getNumberString()
				local card_id = card:getEffectiveId()
				return ("jink:longdan[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	elseif pattern == "slash" then
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		for _, card in ipairs(cards) do
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

local machao_ai = SmartAI:newSubclass "machao"

function machao_ai:askForSkillInvoke(skill_name, data)
	if skill_name == "tieji" then
		local effect = data:toSlashEffect()
		assert(effect.to)
		return not self:isFriend(effect.to)
	else
		return super.askForSkillInvoke(self, skill_name, data)
	end
end

local sunshangxiang_ai = SmartAI:newSubclass "sunshangxiang"

function sunshangxiang_ai:activate(use)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)

	for _, card in ipairs(cards) do
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

	if target and self.player:getHandcardNum()>=2 then
		local cards = self.player:getHandcards()
		local first = cards:at(0):getEffectiveId()
		local second = cards:at(1):getEffectiveId()

		local card_str = ("@JieyinCard=%d+%d"):format(first, second)
		use.card = sgs.Card_Parse(card_str)
		use.to:append(target)

		return
	end

	super.activate(self, use)
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
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		table.sort(cards, black_before_red)
		local card_id = cards[1]:getEffectiveId()

		use.card = sgs.Card_Parse("@QingnangCard=" .. card_id)
		use.to:append(most_misery)
		self.qingnang_used = true

		return
	end

	super.activate(self, use)
end

local diaochan_ai = SmartAI:newSubclass "diaochan"
diaochan_ai:setOnceSkill("lijian")

function diaochan_ai:activate(use)
	if not self.lijian_used and not self.player:isKongcheng() then
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
			local card_id = self.player:getHandcards():at(0):getEffectiveId()
			use.card = sgs.Card_Parse("@LijianCard=" .. card_id)
			use.to:append(first)
			use.to:append(second)

			self.lijian_used = true
			return
		end
	end

	super.activate(self, use)
end
