-- Bold and innovative use of existing elements, should be encouraged!
-- v5zhonghui

-- zhenggong
sgs.ai_skill_invoke.v5zhenggong = sgs.ai_skill_invoke.ganglie

-- quanji
sgs.ai_skill_use["@@v5qj"] = function(self, prompt)
	local target = self.room:getCurrent()
	if not target then return "." end
	local m = self:getMaxCard()
	local n = self:getMinCard()
	local card_id = -1
	if self:isFriend(target) then
		if (target:containsTrick("indulgence") or target:containsTrick("supply_shortage")) and m then
			card_id = m:getEffectiveId()
		end
		if target:hasSkill("zhiji") and target:getMark("zhiji") == 0 and target:getHandcardNum()== 1 and n then
			card_id = n:getEffectiveId()
		end
	else
		if not m then return "." end
		if self:hasSkills("luoshen|guanxing|yinghun|lianli", target) then
			card_id = m:getEffectiveId()
		end
		if target:hasSkill("miji") and target:isWounded() then
			card_id = m:getEffectiveId()
		end
		if (target:hasSkill("hunzi") and target:getMark("hunzi") == 0 and target:getHp()== 1) or
			(target:hasSkill("zili") and target:getMark("zili") == 0 and target:getPile("power"):length() >= 3) or
			(target:hasSkill("zaoxian") and target:getMark("zaoxian") == 0 and target:getPile("field"):length() >= 3) or
			(target:hasSkill("v5baijiang") and target:getMark("v5baijiang") == 0 and target:getEquips():length() >= 3) then
			card_id = m:getEffectiveId()
		end
	end
	if card_id > -1 then
		return "@V5QuanjiCard=" .. card_id .. "->."
	else
		return "."
	end
end

-- baijiang wake

-- yexin
sgs.ai_skill_invoke.v5yexin = true

-- zili wake

-- paiyi
sgs.ai_skill_invoke.v5paiyi = true
sgs.ai_skill_askforag.v5paiyi = function(self, card_ids)
	local card
	if not self.v5py then self.v5py = self.friends[1] end
	return card_ids[1]
end

sgs.ai_skill_playerchosen.v5paiyi = function(self, targets)
	local target = self.v5py
	for _, friend in ipairs(self.friends_noself) do
		if friend ~= target then
			self.v5py = friend
			break
		end
	end
	if target and target:isAlive() then
		return target
	else
		return self.player
	end
end

sgs.ai_skill_choice.v5paiyi = function(self, choices)
	local choicess = sgs.QList2Table(choices:split("+"))
	if choicess.contains("equip") then
		return "equip"
	else
		return "hand"
	end
end

