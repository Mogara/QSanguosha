sgs.ai_skill_use["@@fengyin"] = function(self, data)
	local cards = self.player:getHandcards()
	local card
	cards = sgs.QList2Table(cards)

	for _,acard in ipairs(cards)  do
		if acard:isKindOf("Slash") then
			card = acard
			break
		end
	end

	if not card then
		return "."
	end
	local card_id = card:getEffectiveId()

	local target = self.room:getCurrent()
	if self:isFriend(target) and target:containsTrick("indulgence") and target:getHandcardNum() + 2 > target:getHp() then
		return "@FengyinCard="..card_id
	end
	if self:isEnemy(target) and not target:containsTrick("indulgence") and target:getHandcardNum() >= target:getHp() then
		return "@FengyinCard="..card_id
	end
	return "."
end

local mixin_skill={}
mixin_skill.name="mixin"
table.insert(sgs.ai_skills, mixin_skill)
mixin_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("MixinCard") or self.player:isKongcheng() then return end
	local cards = self.player:getHandcards()
	local allcard = {}
	cards = sgs.QList2Table(cards)
	
	local card
	
	self:sortByKeepValue(cards,true)
	
	card = cards[1]
	
	if not card then
		return nil
	end
	
	local card_id = card:getEffectiveId()
	local card_str = "@MixinCard="..card_id
	local skillcard = sgs.Card_Parse(card_str)
		
	assert(skillcard)
	return skillcard
end

sgs.ai_skill_use_func.MixinCard=function(card,use,self)
    for _, friend in ipairs(self.friends_noself) do
        if not friend:hasSkill("manjuan") then
			use.card = card
			if use.to then use.to:append(friend) end
			return
		end
    end
end

sgs.ai_skill_playerchosen.mixin = sgs.ai_skill_playerchosen.zero_card_as_slash

sgs.ai_use_priority.MixinCard = 0
sgs.ai_card_intention.MixinCard = 20

sgs.ai_skill_invoke.cangni = function(self, data)
	local target = self.room:getCurrent()
	local hint = data:toString()
	if hint == "cangni_lost" then
		return self:isEnemy(target)
	elseif hint == "cangni_got" then
		return self:isFriend(target)
	else
		local hand = self.player:getHandcardNum()
		local hp = self.player:getHp()
		return (hand + 2) <= hp or self.player:isWounded();
	end
end

sgs.ai_skill_choice.cangni = function(self, choices)
	local hand = self.player:getHandcardNum()
	local hp = self.player:getHp()
	if (hand + 2) <= hp then
		return "draw"
	else
		return "recover"
	end
end

duyi_skill={}
duyi_skill.name="duyi"
table.insert(sgs.ai_skills, duyi_skill)
duyi_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("DuyiCard") then return end
	return sgs.Card_Parse("@DuyiCard=.")
end

sgs.ai_skill_use_func.DuyiCard=function(card,use,self)
	use.card = card
end

sgs.ai_skill_playerchosen.duyi=function(self, targets)
	local targetlist=sgs.QList2Table(targets)
	self:sort(targetlist,"hp")
	for _, target in sgs.qlist(targets) do
		if self:isFriend(target) and target:objectName() ~= self.player:objectName() then return target end
	end
	return targetlist[#targetlist]
end

sgs.ai_skill_invoke.duanzhi = function(self, data)
	local use = data:toCardEffect()
	if self:isEnemy(use.from) and use.card:getSubtype() == "attack_card" and
		self.player:getHp() == 1 and not self:getCard("Peach") and not self:getCard("Analeptic") then
		return true
	end
	return self:isEnemy(use.from) and self.player:getHp() > 2
end

sgs.ai_skill_choice.duanzhi = function(self, choices)
	return "discard"
end
