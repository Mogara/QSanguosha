sgs.ai_judgestring =
{
	indulgence = "heart",
	diamond = "heart",
	supply_shortage = "club",
	spade = "club",
	club = "club",
	lightning = "spade",
}

local function getIdToCard(self, cards)
	local tocard = {}
	for _, card_id in ipairs(cards) do
		local card = sgs.Sanguosha:getCard(card_id)
		table.insert(tocard, card)
	end
	return tocard
end

local function getBackToId(self, cards)
	local cards_id = {}
	for _, card in ipairs(cards) do
		table.insert(cards_id, card:getEffectiveId())
	end
	return cards_id
end

local function getOwnCards(self, up, bottom, next_judge)
	self:sortByUseValue(bottom)
	local has_slash = self:getCardsNum("Slash") > 0
	local hasNext = false
	for index, gcard in ipairs(bottom) do
		if index == 3 then break end
		if #next_judge > 0 then
			table.insert(up, gcard)
			table.remove(bottom, index)
			hasNext = true
		else
			if has_slash then
				if not gcard:inherits("Slash") then
					table.insert(up, gcard)
					table.remove(bottom, index)
				end
			else
				if gcard:inherits("Slash") then
					table.insert(up, gcard)
					table.remove(bottom, index)
					has_slash = true
				end
			end
		end
	end

	if hasNext then
		for _, gcard in ipairs(next_judge) do
			table.insert(up, gcard)
		end
	end

	return up, bottom
end

local function GuanXing(self, cards)
	local up, bottom = {}, {}
	local has_lightning, has_judged
	local judged_list = {}

	bottom = getIdToCard(self, cards)
	self:sortByUseValue(bottom, true)

	local judge = self.player:getCards("j")
	judge = sgs.QList2Table(judge)
	judge = sgs.reverse(judge)

	for judge_count, need_judge in ipairs(judge) do
		local index = 1
		local lightning_flag = false
		local judge_str = sgs.ai_judgestring[need_judge:objectName()] or sgs.ai_judgestring[need_judge:getSuitString()]

		for _, for_judge in ipairs(bottom) do
			if judge_str == "spade" and not lightning_flag then
				has_lightning = need_judge
				if for_judge:getNumber() >= 2 and for_judge:getNumber() <= 9 then lightning_flag = true end
			end
			if (judge_str == for_judge:getSuitString() and not lightning_flag) or
				(lightning_flag and judge_str ~= for_judge:getSuitString()) then
				table.insert(up, for_judge)
				table.remove(bottom, index)
				judged_list[judge_count] = 1
				has_judged = true
				break
			end
			index = index + 1
		end
		if not judged_list[judge_count] then judged_list[judge_count] = 0 end
	end

	--if has_judged then
		for index=1, #judged_list do
			if judged_list[index] == 0 then
				table.insert(up, index, table.remove(bottom))
			end
		end
	--end

	local pos = 1
	local luoshen_flag = false
	local next_judge = {}
	local next_player = self.player:getNextAlive()
	judge = next_player:getCards("j")
	judge = sgs.QList2Table(judge)
	judge = sgs.reverse(judge)
	if has_lightning then table.insert(judge, 1, has_lightning) end

	has_judged = false
	judged_list = {}

	while(#bottom >= 3) do
		local index = 1
		local lightning_flag = false
		if pos > #judge then break end
		local judge_str = sgs.ai_judgestring[judge[pos]:objectName()] or sgs.ai_judgestring[judge[pos]:getSuitString()]

		for _, for_judge in ipairs(bottom) do
			if judge_str == "spade" and not lightning_flag then
				if for_judge:getNumber() >= 2 and for_judge:getNumber() <= 9 then lightning_flag = true end
			end

			if self:isFriend(next_player) then
				if next_player:hasSkill("luoshen") then
					if for_judge:isBlack() then
						table.insert(next_judge, for_judge)
						table.remove(bottom, index)
						has_judged = true
						judged_list[pos] = 1
						break
					end
				else
					if judge_str == for_judge:getSuitString() then
						if not lightning_flag then
							table.insert(next_judge, for_judge)
							table.remove(bottom, index)
							has_judged = true
							judged_list[pos] = 1
							break
						end
					end
				end
			else
				if next_player:hasSkill("luoshen") and for_judge:isRed() and not luoshen_flag then
					table.insert(next_judge, for_judge)
					table.remove(bottom, index)
					has_judged = true
					judged_list[pos] = 1
					luoshen_flag = true
					break
				else
					if (judge_str == for_judge:getSuitString() and judge_str == "spade" and lightning_flag)
						or judge_str ~= for_judge:getSuitString() then
						table.insert(next_judge, for_judge)
						table.remove(bottom, index)
						has_judged = true
						judged_list[pos] = 1
					end
				end
			end
			index = index + 1
		end
		if not judged_list[pos] then judged_list[pos] = 0 end
		pos = pos + 1
	end

	if has_judged then
		for index=1, #judged_list do
			if judged_list[index] == 0 then
				table.insert(next_judge, index, table.remove(bottom))
			end
		end
	end

	up, bottom = getOwnCards(self, up, bottom, next_judge)

	up = getBackToId(self, up)
	bottom = getBackToId(self, bottom)
	return up, bottom
end

local function XinZhan(self, cards)
	local up, bottom = {}, {}
	local judged_list = {}
	local hasJudge = false
	local next_player = self.player:getNextAlive()
	local judge = next_player:getCards("j")
	judge = sgs.QList2Table(judge)
	judge = sgs.reverse(judge)

	bottom = getIdToCard(self, cards)
	for judge_count, need_judge in ipairs(judge) do
		local index = 1
		local lightning_flag = false
		local judge_str = sgs.ai_judgestring[need_judge:objectName()] or sgs.ai_judgestring[need_judge:getSuitString()]
		self:log("------------------>"..judge_str ..":")

		for _, for_judge in ipairs(bottom) do
			if judge_str == "spade" and not lightning_flag then
				has_lightning = need_judge
				if for_judge:getNumber() >= 2 and for_judge:getNumber() <= 9 then lightning_flag = true end
			end
			if self:isFriend(next_player) then
				if judge_str == for_judge:getSuitString() then
					if not lightning_flag then
						table.insert(up, for_judge)
						table.remove(bottom, index)
						judged_list[judge_count] = 1
						has_judged = true
						break
					end
				end
			else
				if judge_str ~= for_judge:getSuitString() or
					(judge_str == for_judge:getSuitString() and judge_str == "spade" and lightning_flag) then
					table.insert(up, for_judge)
					table.remove(bottom, index)
					judged_list[judge_count] = 1
					has_judged = true
				end
			end
			index = index + 1
		end
		if not judged_list[judge_count] then judged_list[judge_count] = 0 end
	end

	if has_judged then
		for index=1, #judged_list do
			if judged_list[index] == 0 then
				table.insert(up, index, table.remove(bottom))
			end
		end
	end

	while #bottom ~= 0 do
		table.insert(up, table.remove(bottom))
	end

	up = getBackToId(self, up)
	return up, {}
end

function SmartAI:askForGuanxing(cards, up_only)
	if not up_only then return GuanXing(self,cards)
	else return XinZhan(self, cards)
	end
	return cards, {}
end