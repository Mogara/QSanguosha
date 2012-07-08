neoluoyi_skill={}
neoluoyi_skill.name="neoluoyi"
table.insert(sgs.ai_skills, neoluoyi_skill)
neoluoyi_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("LuoyiCard") then return nil end
	local cards=self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	local slashtarget = 0
	local dueltarget = 0
	local equipnum = 0
	self:sort(self.enemies,"hp")
	for _, card in sgs.qlist(self.player:getCards("he")) do
		if card:inherits("EquipCard") and not (card:inherits("Weapon") and self:hasEquip(card))  then
			equipnum = equipnum + 1
		end
	end
	for _,card in ipairs(cards) do
		if card:inherits("Slash") then
			for _,enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy, true) and self:slashIsEffective(card, enemy) and self:objectiveLevel(enemy) > 3 then
					if self:getCardsNum("Jink", enemy) < 1 or (self:isEquip("Axe") and self.player:getCards("he"):length() > 4) then
						slashtarget = slashtarget + 1
					end
				end
			end
		end
		if card:inherits("Duel") then
			for _, enemy in ipairs(self.enemies) do
				if self:getCardsNum("Slash") >= self:getCardsNum("Slash", enemy) 
				and self:objectiveLevel(enemy) > 3 and not self:cantbeHurt(enemy) and enemy:getMark("@fog") < 1 then 
					dueltarget = dueltarget + 1 
				end
			end
		end
	end		
	if (slashtarget+dueltarget) > 0 and equipnum > 0 then
		self:speak("luoyi")
		return sgs.Card_Parse("@LuoyiCard=.")
	end
end

sgs.ai_skill_use_func.LuoyiCard=function(card,use,self)
	use.card = card
end

sgs.ai_skill_cardask["@luoyi-discard"] = function(self, data)
	for _, card in sgs.qlist(self.player:getCards("he")) do
		if card:inherits("EquipCard") and not self.player:hasEquip(card) then 
			return "$" .. card:getEffectiveId()
		end
	end
	for _, card in sgs.qlist(self.player:getCards("he")) do
		if card:inherits("EquipCard") and not card:inherits("Weapon") then 
			return "$" .. card:getEffectiveId()
		end
	end
	return "."
end

sgs.ai_use_priority.LuoyiCard = 9.2

local neofanjian_skill={}
neofanjian_skill.name="neofanjian"
table.insert(sgs.ai_skills,neofanjian_skill)
neofanjian_skill.getTurnUseCard=function(self)
	if self.player:isKongcheng() then return nil end
	if self.player:usedTimes("NeoFanjianCard")>0 then return nil end

	local cards = self.player:getHandcards()

	local card_str = "@NeoFanjianCard=."
	local fanjianCard = sgs.Card_Parse(card_str)
	assert(fanjianCard)

	return fanjianCard		
end

sgs.ai_skill_use_func.NeoFanjianCard=function(card,use,self)
	self:sort(self.enemies, "hp")
			
	for _, enemy in ipairs(self.enemies) do		
		if self:objectiveLevel(enemy) <= 3 or self:cantbeHurt(enemy) or enemy:getMark("@fog") > 0 then						
		elseif (not enemy:hasSkill("qingnang")) or (enemy:getHp() == 1 and enemy:getHandcardNum() == 0 and not enemy:getEquips()) then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

sgs.ai_skill_cardchosen.neofanjian = function(self)
	local shit = self:getCard("Shit")
	if shit then return shit end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	return cards[1]
end

sgs.ai_card_intention.NeoFanjianCard = 70

function sgs.ai_skill_suit.neofanjian()
	local map = {0, 0, 1, 2, 2, 3, 3, 3}
	return map[math.random(1,8)]
end
sgs.ai_skill_invoke.zhongyi = function(self, data)
	local damage = data:toDamage()
	local target = damage.to
	if self:isFriend(target) then return false end
	return target:getCards("e"):length() > 0 
end

sgs.ai_skill_invoke.zhulou = function(self, data)
	if self.player:getHandcardNum() < 3 and self.player:getHp() > 2 then
		return true
	end
	return false
end

sgs.ai_skill_choice.zhulou = function(self, choices)
	local weaponnum = 0
	local weapon_card
	for _, card in sgs.qlist(self.player:getCards("he")) do
		if card:inherits("Weapon") then
			weapon_card = card
			weaponnum = weaponnum + 1
		end
	end
	if weaponnum > 0 then
		return "throw"
	else 
		return "losehp"
	end
end

sgs.ai_skill_cardask["@zhulou-discard"] = sgs.ai_skill_cardask["@xiaoguo-discard"]

function sgs.ai_skill_invoke.neojushou(self, data)
	if not self.player:faceUp() then return true end
	for _, friend in ipairs(self.friends) do
		if self:hasSkills("fangzhu|jilve", friend) then return true end
	end
	return self:isWeak()
end

sgs.ai_skill_invoke.neoganglie = function(self, data)
	local target = data:toPlayer()
	if not self:isFriend(target) then
		self.room:setPlayerFlag(target, "ganglie_target")
		return true
	end
	return false
end

sgs.ai_skill_choice.neoganglie = function(self, choices)
	local target
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasFlag("xuanhuo_target") then
			target = player
			self.room:setPlayerFlag(target, "-ganglie_target")
		end
	end
	if self:hasSkills(sgs.masochism_skill,target) and target:getHp() >= target:getHandcardNum() 
		and target:getHandcardNum() > 1 then
			return "throw"
	end
	return "damage"
end

sgs.ai_skill_discard.neoganglie = function(self, discard_num, min_num, optional, include_equip)
	local to_discard = {}
	local cards = sgs.QList2Table(self.player:getHandcards())
	local index = 0
	self:sortByKeepValue(cards)
	cards = sgs.reverse(cards)

	for i = #cards, 1, -1 do
		local card = cards[i]
		if not self.player:isJilei(card) then
			table.insert(to_discard, card:getEffectiveId())
			table.remove(cards, i)
			index = index + 1
			if index == 2 then break end
		end
	end	
	return to_discard
end