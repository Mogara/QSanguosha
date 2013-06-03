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