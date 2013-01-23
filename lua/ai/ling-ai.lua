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
		if card:isKindOf("EquipCard") and not (card:isKindOf("Weapon") and self:hasEquip(card))  then
			equipnum = equipnum + 1
		end
	end
	for _,card in ipairs(cards) do
		if card:isKindOf("Slash") then
			for _,enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy, card, true) and self:slashIsEffective(card, enemy) and self:objectiveLevel(enemy) > 3 then
					if getCardsNum("Jink", enemy) < 1 or (self:isEquip("Axe") and self.player:getCards("he"):length() > 4) then
						slashtarget = slashtarget + 1
					end
				end
			end
		end
		if card:isKindOf("Duel") then
			for _, enemy in ipairs(self.enemies) do
				if self:getCardsNum("Slash") >= getCardsNum("Slash", enemy) 
				and self:objectiveLevel(enemy) > 3 and not self:cantbeHurt(enemy) and self:damageIsEffective(enemy) and enemy:getMark("@late") == 0 then
					dueltarget = dueltarget + 1 
				end
			end
		end
	end		
	if (slashtarget+dueltarget) > 0 and equipnum > 0 then
		self:speak("luoyi")
		local luoyicard
		for _, card in sgs.qlist(self.player:getCards("he")) do
			if card:isKindOf("EquipCard") and not self.player:hasEquip(card) then 
				luoyicard = card
				break
			end
		end
		for _, card in sgs.qlist(self.player:getCards("he")) do
			if card:isKindOf("EquipCard") and not card:isKindOf("Weapon") then 
				luoyicard = card
				break
			end
		end
		return sgs.Card_Parse("@LuoyiCard=" .. luoyicard:getEffectiveId())
	end
end

sgs.ai_skill_use_func.LuoyiCard=function(card,use,self)
	use.card = card
end

sgs.ai_use_priority.LuoyiCard = 9.2

local neofanjian_skill={}
neofanjian_skill.name="neofanjian"
table.insert(sgs.ai_skills,neofanjian_skill)
neofanjian_skill.getTurnUseCard=function(self)
	if self.player:isKongcheng() then return nil end
	if self.player:usedTimes("NeoFanjianCard")>0 then return nil end

	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)

	local keep_value = self:getKeepValue(cards[1])
	if cards[1]:getSuit() == sgs.Card_Diamond then keep_value = keep_value + 1 end

	if keep_value < 6 then
		if cards[1]:isKindOf("Peach") or cards[1]:isKindOf("Analeptic") then return nil end
		local card_id = cards[1]:getEffectiveId()
		local card_str = "@NeoFanjianCard=" .. card_id
		local fanjianCard = sgs.Card_Parse(card_str)
		assert(fanjianCard)
		return fanjianCard
	end
end

sgs.ai_skill_use_func.NeoFanjianCard=function(card,use,self)
	self:sort(self.enemies, "hp")
			
	for _, enemy in ipairs(self.enemies) do		
		if self:objectiveLevel(enemy) <= 3 or self:cantbeHurt(enemy) or not self:damageIsEffective(enemy) then
		elseif (not enemy:hasSkill("qingnang")) or (enemy:getHp() == 1 and enemy:getHandcardNum() == 0 and not enemy:getEquips()) then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

sgs.ai_card_intention.NeoFanjianCard = 70

function sgs.ai_skill_suit.neofanjian()
	local map = {0, 0, 1, 2, 2, 3, 3, 3}
	return map[math.random(1,8)]
end
sgs.ai_skill_invoke.yishi = function(self, data)
	local damage = data:toDamage()
	local target = damage.to
	local judge_card = target:getCards("j")
	if self:isFriend(target) then
		if judge_card and judge_card:length() > 0 then return true end
		if not (target:getHp()>2 and target:hasSkill("yiji")) 
			and not (target:hasSkill("longhun") and target:getHp()>1 and target:getCards("he"):length()>2)
			and not (target:getHp()>2 and target:hasSkill("guixin") and self.room:alivePlayerCount() > 2)
				then return true
		end
	else
		if damage.card:hasFlag("drank") then return false end
		if self:isWeak(target) then return false end
		if target:getArmor() and self:evaluateArmor(target:getArmor(), target)>3 and not self:isEquip("Vine", target) then return true end
		if target:hasSkill("tuntian") then return false end
		if self:hasSkills(sgs.need_kongcheng, target) then return false end
		return false
	end 
end

sgs.ai_skill_invoke.zhulou = function(self, data)
	local weaponnum = 0
	for _, card in sgs.qlist(self.player:getCards("h")) do
		if card:isKindOf("Weapon") then
			weaponnum = weaponnum + 1
		end
	end

	if weaponnum > 0 then return true end
		
	if self.player:getHandcardNum() < 3 and self.player:getHp() > 2 then
		return true
	end

		if self.player:getHp() < 3 and self.player:getWeapon() then
			return true
	end

	return false
end

sgs.ai_skill_cardask["@zhulou-discard"] =  function(self, data)
	  for _, card in sgs.qlist(self.player:getCards("he")) do
		if card:isKindOf("Weapon") and not self.player:hasEquip(card) then
			return "$" .. card:getEffectiveId()
		end
	end
	for _, card in sgs.qlist(self.player:getCards("he")) do
		if card:isKindOf("Weapon") then
			return "$" .. card:getEffectiveId()
		end
	end
	return "."
end

sgs.ai_cardneed.zhulou = sgs.ai_cardneed.weapon

sgs.zhulou_keep_value = {
	Peach = 6,
	Jink = 5.1,
	Crossbow = 5,
	Blade = 5,
	Spear = 5,
	DoubleSword =5,
	QinggangSword=5,
	Axe=5,
	KylinBow=5,
	Halberd=5,
	IceSword=5,
	Fan=5,
	MoonSpear=5,
	GudingBlade=5
}

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
		if (self:hasSkills(sgs.masochism_skill,target) or self:getDamagedEffects(target,self.player)) and target:getHandcardNum()<=1 then return false end
		self.room:setPlayerFlag(target, "ganglie_target")
		return true
	else
		if self:getDamagedEffects(target,self.player) then 
			sgs.ai_ganglie_effect = string.format("%s_%s_%d",self.player:objectName(), target:objectName(),sgs.turncount) 
			return true 
		end
	end
	return false
end

sgs.ai_need_damaged.neoganglie = function (self, attacker)
	if self:getDamagedEffects(attacker,self.player) then return self:isFriend(attacker) end

	if self:isEnemy(attacker) and attacker:getHp() <= 2 and not attacker:hasSkill("buqu") and sgs.isGoodTarget(attacker,self.enemies) then
		return true
	end
	return false
end

sgs.ai_skill_choice.neoganglie = function(self, choices)
	local target
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasFlag("ganglie_target") then
			target = player
			self.room:setPlayerFlag(target, "-ganglie_target")
		end
	end
	if self:getDamagedEffects(target,self.player) and self:isFriend(target) then return "damage" end

	if (self:hasSkills(sgs.masochism_skill,target) or self:getDamagedEffects(target,self.player)) and target:getHandcardNum() > 1 then
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

sgs.ai_suit_priority.yishi= "club|spade|diamond|heart"