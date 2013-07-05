-- huangyueying
sgs.ai_skill_invoke.new_jizhi = true
sgs.ai_skill_cardask["@newjizhi"] = function(self, data)
	if self.player:isKongcheng() then return "." end
	local peek = self.room:peek()
	if peek:inherits("Peach") or peek:inherits("Analeptic") then
		local use = data:toCardUse()
		local cards = sgs.QList2Table(self.player:getCards("h"))
		self:sortByUseValue(cards, true)
		for _, acard in ipairs(cards) do
			if acard ~= use.card and peek:objectName() ~= acard:objectName() and not acard:isKindOf("TrickCard") then
				return acard:getEffectiveId()
			end
		end
		for _, acard in ipairs(cards) do
			if acard ~= use.card and peek:objectName() ~= acard:objectName() then
				return acard:getEffectiveId()
			end
		end
		return "."
	end
end

-- liubei
new_rende_skill={}
new_rende_skill.name="new_rende"
table.insert(sgs.ai_skills, new_rende_skill)
new_rende_skill.getTurnUseCard=function(self)
	if self.player:isKongcheng() or self.player:hasUsed("NewRendeCard") then return end
	local invoke = false
	for _, player in ipairs(self.friends_noself) do
		if ((player:hasSkill("haoshi") and not player:containsTrick("supply_shortage"))
			or player:hasSkill("longluo") or (not player:containsTrick("indulgence") and  player:hasSkill("yishe"))
			and player:faceUp()) or player:hasSkill("jijiu") then
			invoke = true
			break
		end
	end
	if self:getOverflow() > 0 or self:getCard("Shit") or self.player:getLostHp() < 2 then
		invoke = true
	end
	local cards = sgs.QList2Table(self.player:getHandcards())
	local allcard = {}
	for _,card in ipairs(cards)  do
		table.insert(allcard, card:getId())
	end
	if invoke then
		return sgs.Card_Parse("@NewRendeCard=" .. table.concat(allcard, "+"))
	end
end
sgs.ai_skill_use_func.NewRendeCard = function(card,use,self)
	use.card = card
end
