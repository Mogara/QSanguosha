local function Reverse(self, cardlist)
	local size = #cardlist
	local Reverse_list = {}
	while(size > 0) do
		table.insert(Reverse_list, cardlist[size])
		size = size - 1
	end
	return Reverse_list
end

sgs.JudgeString = 
{
indulgence = "heart",
diamond = "heart",
supply_shortage = "club",
spade = "club",
club = "club",
lightning = "spade",
}

function getIdToCard(self, cards)
	local tocard = {}
	for _, card_id in ipairs(cards) do
		local card = sgs.Sanguosha:getCard(card_id)
		table.insert(tocard, card)
		self:log(card:objectName()..":"..card:getSuitString())
	end
	return tocard
end

function getBackToId(self, cards)
	local cards_id = {}
	for _, card in ipairs(cards) do
		self:log(card:objectName()..":"..card:getSuitString())
		table.insert(cards_id, card:getEffectiveId())
	end
	return cards_id
end

function getOwnCards(self, up, buttom, next_judge)
	self:sortByUseValue(buttom)
	local hasSlash = self:getSlash()
	local hasNext = false
	for index, gcard in ipairs(buttom) do
		if index == 3 then break end
		if #next_judge > 0 then
			table.insert(up, gcard) 
			table.remove(buttom, index)
			self:log(gcard:objectName() .. "!!!!!")
			hasNext = true
		else
			if hasSlash then 
				if not (gcard:inherits("Slash") and gcard:inherits("Nullification")) then 
					table.insert(up, gcard) 
					table.remove(buttom, index)
				end
			else
				if gcard:inherits("Slash") then 
					table.insert(up, gcard) 
					table.remove(buttom, index)
					hasSlash = true 
				end
			end
		end
	end
	
	if hasNext then
		for _, gcard in ipairs(next_judge) do
			table.insert(up, gcard) 
		end
	end
	
	return up, buttom
end

function GuanXing(self, cards)
	local up, buttom = {}, {}
	local hasLightning, hasJudged
	local is_judged = {}
	
	buttom = getIdToCard(self, cards)
	
	local judge = self.player:getCards("j")
	judge = sgs.QList2Table(judge)
	judge = Reverse(self, judge)
	
	for judge_count, need_judge in ipairs(judge) do
		local index = 1
		local lightning_flag = false
		local judge_str = sgs.JudgeString[need_judge:objectName()] or sgs.JudgeString[need_judge:getSuitString()]
		self:log("------------------>"..judge_str ..":")
		
		for _, for_judge in ipairs(buttom) do
			if judge_str == "spade" and not lightning_flag then
				hasLightning = need_judge
				if for_judge:getNumber() >= 2 and for_judge:getNumber() <= 9 then lightning_flag = true end
			end
			if judge_str == for_judge:getSuitString() then
				if not lightning_flag then
					table.insert(up, for_judge)
					table.remove(buttom, index)
					is_judged[judge_count] = 1
					hasJudged = true
					break
				end
			end
			index = index + 1
		end
		if not is_judged[judge_count] then is_judged[judge_count] = 0 end
	end
	
	if hasJudged then
		for index=1, #is_judged do
			if is_judged[index] == 0 then
				table.insert(up, index, table.remove(buttom))
			end
		end
	end
	
	local pos = 1
	local luoshen_flag = false
	local next_judge = {}
	local next_player = self.player:getNextAlive()
	judge = next_player:getCards("j")
	judge = sgs.QList2Table(judge)
	judge = Reverse(self, judge)
	if hasLightning then table.insert(judge, 1, hasLightning) end
	
	while(#buttom >= 3) do
		local index = 1
		local lightning_flag = false
		if pos > #judge then break end
		local judge_str = sgs.JudgeString[judge[pos]:objectName()] or sgs.JudgeString[judge[pos]:getSuitString()]
		self:log("------------------>"..judge_str ..":")
	
		for _, for_judge in ipairs(buttom) do
			if judge_str == "spade" and not lightning_flag then
				hasLightning = need_judge
				if for_judge:getNumber() >= 2 and for_judge:getNumber() <= 9 then lightning_flag = true end
			end
			
			if self:isFriend(next_player) then 
				self:log("luoshen:"..for_judge:getSuitString())
				if next_player:hasSkill("luoshen") then
					if for_judge:isBlack() then
						table.insert(next_judge, for_judge)
						table.remove(buttom, index)	
						break
					end
				else
					if judge_str == for_judge:getSuitString() then
						if not lightning_flag then
							table.insert(next_judge, for_judge)
							table.remove(buttom, index)
							break
						end
					end
				end
			else
				if next_player:hasSkill("luoshen") and not luoshen_flag then
					self:log("luoshen:"..for_judge:getSuitString())
					if for_judge:isRed() and not luoshen_flag then
						table.insert(next_judge, for_judge)
						table.remove(buttom, index)	
						luoshen_flag = true
						break
					end
				else
					if judge_str ~= for_judge:getSuitString() or 
						(judge_str == for_judge:getSuitString() and judge_str == "spade" and lightning_flag) then
						table.insert(up, for_judge)
						table.remove(buttom, index)
					end
				end
			end
			index = index + 1
		end
		
		pos = pos + 1
	end
	
	up, buttom = getOwnCards(self, up, buttom, next_judge) 
	
	self:log("-------------After Change--------------")
	up = getBackToId(self, up)
	buttom = getBackToId(self, buttom)
	return up, buttom
end

function XinZhan(self, cards)
	local up, buttom = {}, {}
	local is_judged = {}
	local hasJudge = false
	local next_player = self.player:getNextAlive()
	local judge = next_player:getCards("j")
	judge = sgs.QList2Table(judge)
	judge = Reverse(self, judge)
	
	buttom = getIdToCard(self, cards)
	for judge_count, need_judge in ipairs(judge) do
		local index = 1
		local lightning_flag = false
		local judge_str = sgs.JudgeString[need_judge:objectName()] or sgs.JudgeString[need_judge:getSuitString()]
		self:log("------------------>"..judge_str ..":")
		
		for _, for_judge in ipairs(cards) do
			if judge_str == "spade" and not lightning_flag then
				hasLightning = need_judge
				if for_judge:getNumber() >= 2 and for_judge:getNumber() <= 9 then lightning_flag = true end
			end
			if self:isFriend(next_player) then
				if judge_str == for_judge:getSuitString() then
					if not lightning_flag then
						table.insert(up, for_judge)
						table.remove(buttom, index)
						is_judged[judge_count] = 1
						hasJudged = true
						break
					end
				end
			else
				if judge_str ~= for_judge:getSuitString() or 
					(judge_str == for_judge:getSuitString() and judge_str == "spade" and lightning_flag) then
					table.insert(up, for_judge)
					table.remove(buttom, index)
					is_judged[judge_count] = 1
					hasJudged = true
				end
			end
			index = index + 1
		end
		if not is_judged[judge_count] then is_judged[judge_count] = 0 end
	end
	
	if hasJudged then
		for index=1, #is_judged do
			if is_judged[index] == 0 then
				table.insert(up, index, table.remove(buttom))
			end
		end
	end
	
	while #buttom ~= 0 do
		table.insert(up, table.remove(buttom))
	end
	
	return up, {}
end