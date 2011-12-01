	-- this script file contains the AI classes for gods

-- guixin, always invoke
sgs.ai_skill_invoke.guixin = true

-- shelie
sgs.ai_skill_invoke.shelie = true

local gongxin_skill={}
gongxin_skill.name="gongxin"
table.insert(sgs.ai_skills,gongxin_skill)
gongxin_skill.getTurnUseCard=function(self)
		local card_str = ("@GongxinCard=.")
		local gongxin_card = sgs.Card_Parse(card_str)
		assert(gongxin_card)
		return gongxin_card
end

sgs.ai_skill_use_func["GongxinCard"]=function(card,use,self)
	if self.player:usedTimes("GongxinCard")>0 then return end
	self:sort(self.enemies,"handcard")

	use.card = card
	if use.to then use.to:append(self.enemies[#self.enemies]) end
end

--function shenlubu_ai:useTrickCard(card, use)
--	if self.player:getMark("@wrath") > 0 then
		--return super.useTrickCard(self, card, use)
	--end
--end

sgs.ai_skill_choice.wumou = "discard"


--wushen
wushen_skill={}
wushen_skill.name="wushen"
table.insert(sgs.ai_skills,wushen_skill)
wushen_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)

	local red_card

	self:sortByUseValue(cards,true)

	for _,card in ipairs(cards)  do
		if card:getSuitString()=="heart" then--and (self:getUseValue(card)<sgs.ai_use_value["Slash"]) then
			red_card = card
			break
		end
	end

	if red_card then
		local suit = red_card:getSuitString()
		local number = red_card:getNumberString()
		local card_id = red_card:getEffectiveId()
		local card_str = ("slash:wushen[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)

		assert(slash)

		return slash
	end
end

--qixing
sgs.ai_skill_askforag.qixing = function(self, card_ids)
	local cards = {}
	for _, card_id in ipairs(card_ids) do
		table.insert(cards, sgs.Sanguosha:getCard(card_id))
	end
	for _, card in ipairs(cards) do
		if card:inherits("Slash") then if self:getCardsNum("Slash") == 0 then return card:getEffectiveId() end
		elseif card:inherits("Jink") then if self:getCardsNum("Jink") == 0 then return card:getEffectiveId() end
		elseif card:inherits("Peach") then if self.player:isWounded() and self:getCardsNum("Peach") < self.player:getLostHp() then return card:getEffectiveId() end
		elseif card:inherits("Analeptic") then if self:getCardsNum("Analeptic") == 0 then return card:getEffectiveId() end
		elseif card:getTypeId() == sgs.Card_Trick then return card:getEffectiveId()
		else return -1 end
	end
	return -1
end

--kuangfeng
sgs.ai_skill_use["@kuangfeng"]=function(self,prompt)
	local friendly_fire
	for _, friend in ipairs(self.friends) do
		local cards = friend:getHandcards()
		for _, card in sgs.qlist(cards) do
			if (card:inherits("FireAttack") and friend:getHandcardNum() >= 4) or card:inherits("FireSlash") then
				friendly_fire = true
				break
			end
		end
		if friendly_fire then break end
	end

	local is_chained = 0
	local target = {}
	for _, enemy in ipairs(self.enemies) do
		if enemy:isChained() then
			is_chained = is_chained + 1
			table.insert(target, enemy)
		end
		if enemy:getArmor() and enemy:getArmor():objectName() == "vine" then
			table.insert(target, 1, enemy)
			break
		end
	end
	local usecard=false
	if friendly_fire and is_chained > 1 then usecard=true end
	if target[1] then
		if target[1]:getArmor() and target[1]:getArmor():objectName() == "vine" then usecard=true end
	end
	if usecard then
		if not target[1] then table.insert(target,self.enemies[1]) end
		if target[1] then return "@KuangfengCard=.->" .. target[1]:objectName() else return "." end
	else
		return "."
	end
end

--dawu
sgs.ai_skill_use["@dawu"] = function(self, prompt)
	self:sort(self.friends, "hp")
	for _, friend in ipairs(self.friends) do
		if friend:getHp() == 1 and not friend:getArmor() and friend:getHandcardNum() <= friend:getHp() then
			return "@DawuCard=.->" .. friend:objectName()
		end
	end

	return "."
end

sgs.ai_skill_playerchosen.dawu = function(self, targets)
	for _, friend in sgs.qlist(targets) do
		if self:isFriend(friend) and friend:getHp() == 1 and not friend:getArmor() and friend:getHandcardNum() <= friend:getHp() then return friend end
	end

	return self.friends[1]
end

--wumou
sgs.ai_skill_choice.wumou = function(self, choices)
	if self.player:getHp() + self:getCardsNum("Peach") > 3 then return "losehp"
	else return "discard"
	end
end

--wuqian
local wuqian_skill={}
wuqian_skill.name = "wuqian"
table.insert(sgs.ai_skills, wuqian_skill)
wuqian_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("WuqianCard") or self.player:getMark("@wrath") < 2 then return end

	local card_str = ("@WuqianCard=.")
	self:sort(self.enemies, "hp")
	local has_enemy
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() <= 2 and self:getCardsNum("Jink", enemy) < 2 and self.player:inMyAttackRange(enemy) then has_enemy = enemy break end
	end

	if has_enemy and self:getCardsNum("Slash") > 0 then
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if card:inherits("Slash") and self:slashIsEffective(card, has_enemy) and self.player:canSlash(has_enemy) and
				(self:getCardsNum("Analeptic") > 0 or has_enemy:getHp() <= 1) and card:IsAvailable(self.player) then return sgs.Card_Parse(card_str)
			elseif card:inherits("Duel") then return sgs.Card_Parse(card_str)
			end
		end
	end
end

sgs.ai_skill_use_func["WuqianCard"]=function(card,use,self)
	self:sort(self.enemies,"hp")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() <= 2 and self:getCardsNum("Jink", enemy) < 2 and self.player:inMyAttackRange(enemy) then
			if use.to then
				use.to:append(enemy)
			end
			use.card = card
			return
		end
	end
end

--shenfen
local shenfen_skill={}
shenfen_skill.name = "shenfen"
table.insert(sgs.ai_skills, shenfen_skill)
shenfen_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("ShenfenCard") or self.player:getMark("@wrath") < 6 then return end
	return sgs.Card_Parse("@ShenfenCard=.")
end

sgs.ai_skill_use_func["ShenfenCard"]=function(card,use,self)
	use.card = card
end

--qinyin
sgs.ai_skill_invoke.qinyin = function(self, data)
	for _,friend in ipairs(self.friends) do
		if friend:isWounded() then return true end
	end
	if sgs.ai_skill_choice.qinyin(self,"up+down")=="down" then return true end
	return false
end

sgs.ai_skill_choice.qinyin = function(self, choices)
	self:sort(self.friends, "hp")
	self:sort(self.enemies, "hp")
	if self.friends[1]:getHp() >= self.enemies[1]:getHp() and self:getAllPeachNum(self.player) > self:getAllPeachNum(self.enemies[1]) then
		return "down"
	else
		return "up"
	end
end

--yeyan
local yeyan_skill={}
yeyan_skill.name = "smallyeyan"
table.insert(sgs.ai_skills, yeyan_skill)
yeyan_skill.getTurnUseCard=function(self)
	if self.player:getMark("@flame") == 0 then return end
	if self.player:getHandcardNum() >= 4 then
		local spade, club, heart, diamond
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if card:getSuit() == sgs.Card_Spade then spade = true
			elseif card:getSuit() == sgs.Card_Clue then club = true
			elseif card:getSuit() == sgs.Card_Heart then heart = true
			elseif card:getSuit() == sgs.Card_Diamond then diamond = true
			end
		end
		if spade and club and diamond and heart then
			self:sort(self.enemies, "hp")
			local target_num = 0
			for _, enemy in ipairs(self.enemies) do
				if (enemy:getArmor() and enemy:getArmor():objectName() == "vine") or enemy:isChained() then
					target_num = target_num + 1
				elseif enemy:getHp() <= 3 then
					target_num = target_num + 1
				end
			end

			if target_num == 1 then
				return sgs.Card_Parse("@GreatYeyanCard=.")
			elseif target_num > 1 then
				return sgs.Card_Parse("@MediumYeyanCard=.")
			end
		end
	end

	self.yeyanchained = false
	if self.player:getHp() + self:getCardsNum("Peach") + self:getCardsNum("Analeptic") <= 2 then
		return sgs.Card_Parse("@SmallYeyanCard=.")
	end
	local target_num = 0
	local chained = 0
	for _, enemy in ipairs(self.enemies) do
		if self:isEquip("Vine", enemy) or self:isEquip("GaleShell", enemy) or enemy:getHp()<=1 then 
			target_num = target_num + 1
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if enemy:isChained() then 
			if chained == 0 then target_num = target_num +1 end
			chained = chained + 1
		end
	end
	self.yeyanchained = (chained > 1)
	if target_num > 2 or (target_num > 1 and self.yeyanchained) or
	(#self.enemies + 1 == self.room:alivePlayerCount()) then return sgs.Card_Parse("@SmallYeyanCard=.") end
end

sgs.ai_skill_use_func["SmallYeyanCard"]=function(card,use,self)
	local num = 0
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if num >=3 then break end
		if self:isEquip("GaleShell", enemy) or self:isEquip("Vine", enemy) then
			if use.to then use.to:append(enemy) end
			num = num + 1
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if num >=3 then break end
		if self.yeyanchained and enemy:isChained() and not (self:isEquip("GaleShell", enemy) or self:isEquip("Vine", enemy)) then
			if use.to then use.to:append(enemy) end
			num = num + 1
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if num >= 3 then break end
		if not ((self.yeyanchained and enemy:isChained()) or (self:isEquip("GaleShell", enemy) or self:isEquip("Vine", enemy))) then
			if use.to then use.to:append(enemy) end
			num = num + 1
		end
	end
	use.card = card
end

sgs.ai_skill_use_func["MediumYeyanCard"]=function(card,use,self)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local need_cards = {}
	local spade, club, heart, diamond
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Spade and not spade then spade = true table.insert(need_cards, card)
		elseif card:getSuit() == sgs.Card_Clue and not club then club = true table.insert(need_cards, card)
		elseif card:getSuit() == sgs.Card_Heart and not heart then heart = true table.insert(need_cards, card)
		elseif card:getSuit() == sgs.Card_Diamond and not diamond then diamond = true table.insert(need_cards, card)
		end
	end
	if #need_cards < 4 then return end

	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getArmor() and enemy:getArmor():objectName() == "vine" then
			if use.to then use.to:append(enemy) end
			break
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if enemy:isChained() then
			if use.to then use.to:append(enemy) end
			if use.to:length() == 2 then break end
		end
	end
	use.card = sgs.Card_Parse("@MediumYeyanCard=" .. table.concat(need_cards, "+"))
end

sgs.ai_skill_use_func["GreatYeyanCard"]=function(card,use,self)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local need_cards = {}
	local spade, club, heart, diamond
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Spade and not spade then spade = true table.insert(need_cards, card)
		elseif card:getSuit() == sgs.Card_Clue and not club then club = true table.insert(need_cards, card)
		elseif card:getSuit() == sgs.Card_Heart and not heart then heart = true table.insert(need_cards, card)
		elseif card:getSuit() == sgs.Card_Diamond and not diamond then diamond = true table.insert(need_cards, card)
		end
	end
	if #need_cards < 4 then return end

	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if not (enemy:getArmor() and enemy:getArmor():objectName() == "silver_lion") then
			if use.to then use.to:append(enemy) end
			use.card = sgs.Card_Parse("@GreatYeyanCard=" .. table.concat(need_cards, "+"))
			return
		end
	end
end

sgs.ai_skill_invoke.lianpo = true

sgs.ai_skill_invoke.jilve=function(self,data)
	local n=self.player:getMark("@bear")
	local use=(n>2 or self:getOverflow()>0)
	if sgs.lastevent == sgs.AskForRetrial or sgs.lastevent == sgs.StartJudge then
		local judge = data:toJudge()
		return (use or judge.who == self.player) and self:needRetrial(judge)
	elseif sgs.lastevent == sgs.Damage then
		return use and self:askForUseCard("@@fangzhu","@fangzhu")~="."
	else
		local card = data:toCard()
		card = card or data:toCardUse().card
		return use or card:inherits("ExNihilo")
	end
end

local jilve_skill={}
jilve_skill.name="jilve"
table.insert(sgs.ai_skills,jilve_skill)
jilve_skill.getTurnUseCard=function(self)
	if self.player:getMark("@bear")<1 or self.player:hasUsed("ZhihengCard") then return end
	local card=sgs.Card_Parse("@ZhihengCard=.")
	local dummy_use={isDummy=true}
	self:useSkillCard(card,dummy_use)
	if dummy_use.card then return sgs.Card_Parse("@JilveCard=.") end
end

sgs.ai_skill_choice.jilve="zhiheng"

sgs.ai_skill_use_func["JilveCard"]=function(card,use,self)
	use.card = card
end

sgs.ai_skill_use["@zhiheng"]=function(self,prompt)
	local card=sgs.Card_Parse("@ZhihengCard=.")
	local dummy_use={isDummy=true}
	self:useSkillCard(card,dummy_use)
	if dummy_use.card then return (dummy_use.card):toString() .. "->." end
	return "."
end

sgs.ai_skill_playerchosen.wuhun = function(self, targets)
	local targetlist=sgs.QList2Table(targets)
	local target
	for _, player in ipairs(targetlist) do
		if self:isEnemy(player) and (not target or target:getHp() < player:getHp()) then
			target = player
		end
	end
	if target then return target end
	self:sort(targetlist, "hp")
	if self.player:getRole() == "loyalist" and targetlist[1]:isLord() then return targetlist[2] end
	return targetlist[1]
end

local longhun_skill={}
longhun_skill.name="longhun"
table.insert(sgs.ai_skills, longhun_skill)
longhun_skill.getTurnUseCard = function(self)
	if self.player:getHp()>1 then return end
	local cards=sgs.QList2Table(self.player:getCards("he"))
	self:sortByUseValue(cards,true)
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Diamond then
			return sgs.Card_Parse(("fire_slash:longhun[%s:%s]=%d"):format(card:getSuitString(),card:getNumberString(),card:getId()))
		end
	end
end
