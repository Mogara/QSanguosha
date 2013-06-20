sgs.ai_skill_playerchosen.koftuxi = function(self, targets)
	local cardstr = sgs.ai_skill_use["@@tuxi"](self, "@tuxi")
	if cardstr:match("->") then
		local targetstr = cardstr:split("->")[2]:split("+")
		if #targetstr > 0 then
			local target = findPlayerByObjectName(self.room, targetstr[1])
			return target
		end
	end
	return nil
end

sgs.ai_playerchosen_intention.koftuxi = function(self, from, to)
	local lord = self.room:getLord()
	if sgs.evaluatePlayerRole(from) == "neutral" and sgs.evaluatePlayerRole(to) == "neutral"
		and lord and not lord:isKongcheng()
		and not self:doNotDiscard(lord, "h", true) and from:aliveCount() >= 4 then
		sgs.updateIntention(from, lord, -35)
		return
	end
	if from:getState() == "online" then
		if (to:hasSkills("kongcheng|zhiji|lianying") and to:getHandcardNum() == 1) or to:hasSkills("tuntian+zaoxian") then
		else
			sgs.updateIntention(from, to, 80)
		end
	else
		local intention = from:hasFlag("tuxi_isfriend_" .. to:objectName()) and -5 or 80
		sgs.updateIntention(from, to, intention)
	end
end

sgs.ai_chaofeng.kof_zhangliao = 4

sgs.ai_view_as.kofqingguo = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place == sgs.Player_PlaceEquip then
		return ("jink:kofqingguo[%s:%s]=%d"):format(suit, number, card_id)
	end
end

function sgs.ai_cardneed.kofqingguo(to, card)
	if card:isKindOf("Weapon") then return not to:getWeapon()
	elseif card:isKindOf("Armor") then return not to:getArmor()
	elseif card:isKindOf("OffensiveHorse") then return not to:getOffensiveHorse()
	elseif card:isKindOf("DefensiveHorse") then return not to:getDefensiveHorse()
	end
	return false
end


sgs.ai_skill_invoke.kofliegong = sgs.ai_skill_invoke.liegong

function sgs.ai_cardneed.kofliegong(to, card)
	return isCard("Slash", card, to) and getKnownCard(to, "Slash", true) == 0
end

sgs.ai_skill_invoke.yinli = function(self)
	return not self:needKongcheng(self.player, true)
end

sgs.ai_skill_askforag.yinli = function(self, card_ids)
	if self:needKongcheng(self.player, true) then return card_ids[1] else return -1 end
end

sgs.ai_skill_choice.kofxiaoji = function(self, choices)
	if choices:match("recover") then return "recover" else return "draw" end
end

sgs.kofxiaoji_keep_value = sgs.xiaoji_keep_value

sgs.ai_skill_invoke.suzi = true
sgs.ai_skill_invoke.cangji = true

sgs.ai_skill_use["@@cangji"] = function(self, prompt)
	for i = 0, 3, 1 do
		local equip = self.player:getEquip(i)
		if not equip then continue end
		self:sort(self.friends_noself)
		if i == 0 then
			if equip:isKindOf("Crossbow") or equip:isKindOf("Blade") then
				for _, friend in ipairs(self.friends_noself) do
					if not self:getSameEquip(equip) and not self:hasCrossbowEffect(friend) and getCardsNum("Slash", friend) > 1 then
						return "@CangjiCard=" .. equip:getEffectiveId() .. "->" .. friend:objectName()
					end
				end
			elseif equip:isKindOf("Axe") then
				for _, friend in ipairs(self.friends_noself) do
					if not self:getSameEquip(equip)
						and (friend:getCardCount(true) >= 4
							or (friend:getCardCount(true) >= 2 and self:hasHeavySlashDamage(friend))) then
						return "@CangjiCard=" .. equip:getEffectiveId() .. "->" .. friend:objectName()
					end
				end
			end
		end
		for _, friend in ipairs(self.friends_noself) do
			if not self:getSameEquip(equip, friend) and not (i == 1 and (self:evaluateArmor(equip, friend) <= 0 or friend:hasSkills("bazhen|yizhong"))) then
				return "@CangjiCard=" .. equip:getEffectiveId() .. "->" .. friend:objectName()
			end
		end
		if equip:isKindOf("SilverLion") then
			for _, enemy in ipairs(self.enemies) do
				if not enemy:getArmor() and enemy:hasSkills("bazhen|yizhong") then
					return "@CangjiCard=" .. equip:getEffectiveId() .. "->" .. enemy:objectName()
				end
			end
		end
	end
	return "."
end

sgs.ai_card_intention.CangjiCard = function(self, card, from, tos)
	local to = tos[1]
	local equip = sgs.Sanguosha:getCard(card:getEffectiveId())
	if equip:isKindOf("SilverLion") and to:hasSkills("bazhen|yizhong") then
		sgs.updateIntention(from, to, 40)
	else
		sgs.updateIntention(from, to, -40)
	end
end