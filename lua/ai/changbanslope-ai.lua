--Çà¸Ö
sgs.ai_skill_invoke.cbqinggang = function(self, data)
	local damage = data:toDamage()
	return not self:isFriend(damage.to)
end

--ÁúÅ­
local cblongnu_skill = {}
cblongnu_skill.name = "cblongnu"
table.insert(sgs.ai_skills, cblongnu_skill)
cblongnu_skill.getTurnUseCard = function(self)
	local angers = self.player:getPile("Angers")
	local first_found, second_found = false, false
	local first_card, second_card
	if angers:length() >= 2 then
		--local same_suit = false
		for _, id1 in sgs.qlist(angers) do
			local cd1 = sgs.Sanguosha:getCard(id1)
			if cd1:isBlack() then
				first_card = cd1
				first_found = true
				for _, id2 in sgs.qlist(angers) do
					local cd2 = sgs.Sanguosha:getCard(id2)
					if first_card:getEffectiveId() ~= cd2:getEffectiveId() and cd2:getSuitString() == first_card:getSuitString() then
						second_card = cd2
						second_found = true
						break
					end
				end
				if second_card then break end
			end
		end
	end

	if first_found and second_found and (self:getCardsNum("Slash") > 0 or self:getCardsNum("Jink") > 0) and (not self.player:hasUsed("Slash")) and (not self.player:hasUsed("CBLongNuCard")) then
		local CBLongNuCard = {}
		local first_id = first_card:getEffectiveId()
		local second_id = second_card:getEffectiveId()
		local card_str = ("@CBLongNuCard=%d+%d"):format(first_id, second_id)
		return sgs.Card_Parse(card_str)
	end
end

sgs.ai_skill_use_func.CBLongNuCard = function(card, use, self)
	use.card = card
	if use.to then use.to:append(self.player) end
	return
end

sgs.ai_use_value.CBLongNuCard = 5.98
sgs.ai_use_priority.CBLongNuCard = 2.7

--Ô¡Ñª
local cbyuxue_skill = {}
cbyuxue_skill.name = "cbyuxue"
table.insert(sgs.ai_skills, cbyuxue_skill)
cbyuxue_skill.getTurnUseCard = function(self)
	local angers = self.player:getPile("Angers")
	local redangers = sgs.IntList()
	for _,id in sgs.qlist(angers) do
		local cd = sgs.Sanguosha:getCard(id)
		if cd:isRed() then
			redangers:append(id)
		end
	end
	if redangers:isEmpty() or (not self.player:isWounded()) then return nil end

	local card = sgs.Sanguosha:getCard(redangers:first())
	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("peach:cbyuxue[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

function sgs.ai_cardsview.cbyuxue(class_name, player)
	if class_name == "Peach" then
		if player:hasSkill("cbyuxue") and not player:getPile("Angers"):isEmpty() then
			return cbyuxue_skill.getTurnUseCard
		end
	end
end

sgs.ai_view_as.cbyuxue = function(card, player, card_place)
	local angers = player:getPile("Angers")
	local redangers = sgs.IntList()
	for _,id in sgs.qlist(angers) do
		local cd = sgs.Sanguosha:getCard(id)
		if cd:isRed() then
			redangers:append(id)
		end
	end
	if redangers:isEmpty() then return nil end
	
	local card = sgs.Sanguosha:getCard(redangers:first())
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	return ("peach:cbyuxue[%s:%s]=%d"):format(suit, number, card_id)
end

--ÁúÒ÷
sgs.ai_skill_invoke.cblongyin = true

sgs.ai_skill_askforag.cblongyin = function(self, card_ids)
	local angers = self.player:getPile("Angers")
	local redAngers, blackAngers = sgs.IntList(), sgs.IntList()
	for _, id in sgs.qlist(card_ids) do
		local cd = sgs.Sanguosha:getCard(id)
		if cd:isRed() then
			redAngers:append(id)
		else
			blackAngers:append(id)
		end
	end
	local has_redAngers, has_blackAngers = sgs.IntList(), sgs.IntList()
	for _, id in sgs.qlist(angers) do
		local cd = sgs.Sanguosha:getCard(id)
		if cd:isRed() and not (cd:inherits("Peach") and cd:inherits("ExNihilo")) then
			has_redAngers:append(id)
		elseif cd:isBlack() and not cd:inherits("Analeptic") then
			has_blackAngers:append(id)
		end
	end
	if has_redAngers:length() >= has_blackAngers:length() then
		self.cblongyin = redAngers:first()
	else
		self.cblongyin = blackAngers:first()
	end
	
	return self.cblongyin
end

--±¸Á¸
sgs.ai_skill_invoke.cbbeiliang = function(self, data)
	if self.player:getCards("h"):length() <= 2 then
		self:speak("cbbeiliang")
		return true
	end
	return false
end

--¾ÛÎä
local cbjuwu_skill = {}
cbjuwu_skill.name = "cbjuwu"
table.insert(sgs.ai_skills, cbjuwu_skill)
cbjuwu_skill.getTurnUseCard=function(self)
	if self.player:isKongcheng() then return end
	local cbzhaoyun = self.room:findPlayer("cbzhaoyun1")
	if not cbzhaoyun then cbzhaoyun = self.room:findPlayer("cbzhaoyun2") end
	if (not cbzhaoyun:containsTrick("supply_shortage") or not cbzhaoyun:containsTrick("indulgence")) and cbzhaoyun:faceUp() then
		return sgs.Card_Parse("@CBJuWuCard=.")
	end
	
	if (self.player:usedTimes("CBJuWuCard") < 1 or self:getOverflow() > 0 or self:getCard("Shit")) and self.player:getMark("cbjuwu") < self.player:getHp() then
		return sgs.Card_Parse("@CBJuWuCard=.")
	end	
end

sgs.ai_skill_use_func.CBJuWuCard = function(card, use, self)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards,true)
	local name = self.player:objectName()
	local cbzhaoyun = self.room:findPlayer("cbzhaoyun1")
	if not cbzhaoyun then cbzhaoyun = self.room:findPlayer("cbzhaoyun2") end
	local card = self:getCardNeedPlayer(cards)
	if card and cbzhaoyun then
		use.card = sgs.Card_Parse("@CBJuWuCard=" .. card:getId())
		if use.to then use.to:append(cbzhaoyun) end
		return
	end

	if (self:getOverflow()>0) then
		local card_id = self:getCardRandomly(self.player, "h")
		use.card = sgs.Card_Parse("@CBJuWuCard=" .. card_id)
		if use.to then use.to:append(cbzhaoyun) end
		return
	end
	
	if self.player:getHandcardNum()==1 then
		for _, enemy in ipairs(self.enemies) do
			if self:isEquip("GudingBlade", enemy) and enemy:canSlash(self.player, true) then return end
		end
	end
end

sgs.ai_use_value.CBJuWuCard = 8.5
sgs.ai_use_priority.CBJuWuCard = 5.8

sgs.ai_card_intention.CBJuWuCard = -70

sgs.dynamic_value.benefit.CBJuWuCard = true

--ß±Éñ
local cbshishen_skill = {}
cbshishen_skill.name = "cbshishen"
table.insert(sgs.ai_skills, cbshishen_skill)
cbshishen_skill.getTurnUseCard = function(self)
	local angers = self.player:getPile("Angers")
	local first_found, second_found = false, false
	local first_card, second_card
	if angers:length() >= 2 then
		local same_color = false
		for _, id1 in sgs.qlist(angers) do
			local cd1 = sgs.Sanguosha:getCard(id1)
			if cd1:isBlack() then
				first_card = cd1
				first_found = true
				for _, id2 in sgs.qlist(angers) do
					local cd2 = sgs.Sanguosha:getCard(id2)
					if first_card:getEffectiveId() ~= cd2:getEffectiveId() and first_card:sameColorWith(cd2) then
						second_card = cd2
						second_found = true
						break
					end
				end
				if second_card then break end
			end
		end
	end

	if first_found and second_found then
		local CBShiShenCard = {}
		local first_id = first_card:getEffectiveId()
		local second_id = second_card:getEffectiveId()
		local card_str = ("@CBShiShenCard=%d+%d"):format(first_id, second_id)
		return sgs.Card_Parse(card_str)
	end
end

sgs.ai_skill_use_func.CBShiShenCard = function(card, use, self)
	self:sort(self.enemies, "hp")
	
	for _, enemy in ipairs(self.enemies) do
		use.card = card
		if use.to then use.to:append(enemy) end
		return
	end
end

--²øÉß
local cbchanshe_skill = {}
cbchanshe_skill.name = "cbchanshe"
table.insert(sgs.ai_skills, cbchanshe_skill)
cbchanshe_skill.getTurnUseCard = function(self)
	local angers = self.player:getPile("Angers")
	local redangers = sgs.IntList()
	for _,id in sgs.qlist(angers) do
		local cd = sgs.Sanguosha:getCard(id)
		if cd:getSuit() == sgs.Card_Diamond then
			redangers:append(id)
		end
	end
	if redangers:isEmpty() then return nil end

	local card = sgs.Sanguosha:getCard(redangers:first())
	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("indulgence:cbchanshe[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

--AI RolePredictable
function sgs.isRolePredictable()
	if sgs.GetConfig("RolePredictable", true) then return true end
	local mode = string.lower(global_room:getMode())
	if not mode:find("0") or mode:find("03p") or mode:find("02_1v1") or mode:find("04_1v3") or mode == "06_3v3" or mode:find("mini") or mode:find("05_2v3") then return true end
	return false
end