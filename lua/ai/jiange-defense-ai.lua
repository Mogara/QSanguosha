--[[********************************************************************
	Copyright (c) 2013-2014 - QSanguosha-Rara

  This file is part of QSanguosha-Hegemony.

  This game is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 3.0
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  See the LICENSE file for more details.

  QSanguosha-Rara
*********************************************************************]]


sgs.ai_skill_invoke.jgjizhen = true

sgs.ai_skill_invoke.jglingfeng = true


sgs.ai_skill_playerchosen.jglingfeng = function(self, targets)
	self:updatePlayers()
	self:sort(self.enemies, "hp")
	local target = nil
	for _, enemy in ipairs(self.enemies) do
		if not self.player:isFriendWith(enemy) then
			target = enemy
			break
		end
	end
	return target
end

sgs.ai_playerchosen_intention.jglingfeng = 80

sgs.ai_skill_invoke.jgbiantian = true

sgs.ai_slash_prohibit.jgbiantian = function(self, from, enemy, card)
	if enemy:getMark("@fog") > 0 and not card:isKindOf("ThunderSlash") then return false end
	return true
end

sgs.ai_skill_playerchosen.jggongshen = function(self, targets)
	self:updatePlayers()
	self:sort(self.friends_noself)
	local target = nil
	local peacespell = false
	for _, enemy in ipairs(self.enemies) do
		if enemy:hasArmorEffect("PeaceSpell") then peacespell = true end
		if string.find(enemy:getGeneral():objectName(), "machine") and not self.player:isFriendWith(enemy) and not peacespell then
			if enemy:hasArmorEffect("Vine") or enemy:getMark("@gale") > 0 or enemy:getHp() == 1 then
				target = enemy
				break
			end
		end
	end
	if not target then
	for _, friend in ipairs(self.friends_noself) do
		if string.find(friend:getGeneral():objectName(), "machine")  and self.player:isFriendWith(friend) and friend:getLostHp() > 0 then
			if self:isWeak(friend) or peacespell then
				target = friend
				break
			end
		end
	end
	end
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if string.find(enemy:getGeneral():objectName(), "machine") and not self.player:isFriendWith(enemy) then
				target = enemy
				break
			end
		end
	end
	return target
end

sgs.ai_playerchosen_intention.jggongshen = 80

sgs.ai_skill_invoke.jgzhinang = true

--[[
sgs.ai_skill_choice.jgzhinang = function(self, choice, data)
	if self.player:getMark("zhinangEquip") > self.player:getMark("zhinangTrick") and string.find(choice, "EquipCard") then return "EquipCard" end
return "TrickCard"
end]]

sgs.ai_skill_playerchosen.jgzhinang = function(self, targets)
	for _, friend in ipairs(self.friends_noself) do
		if friend:faceUp() and not self:isWeak(friend) then
			if not friend:getWeapon() or friend:hasSkills("rende|jizhi") then
				return friend
			end
		end
	end
	return self.player
end

sgs.ai_playerchosen_intention.jgzhinang = function(self, from, to)
	if not self:needKongcheng(to, true) and self.player:isFriendWith(to) then sgs.updateIntention(from, to, -50) end
end

sgs.ai_skill_invoke.jgjingmiao = true

sgs.ai_skill_invoke.jgyuhuo_pangtong = true
sgs.ai_skill_invoke.jgyuhuo_zhuque = true

sgs.ai_slash_prohibit.jgyuhuo_pangtong = function(self, from, enemy, card)
	if enemy:hasShownSkill("jgyuhuo_pangtong") and card:isKindOf("FireSlash") then return false end
	return true
end

sgs.ai_slash_prohibit.jgyuhuo_zhuque = function(self, from, enemy, card)
	if enemy:hasShownSkill("jgyuhuo_zhuque") and card:isKindOf("FireSlash") then return false end
	return true
end

sgs.ai_skill_invoke.jgqiwu = true

sgs.ai_skill_playerchosen.jgqiwu = function(self, targets)
	local target = nil
	local chained = 0
	self:sort(self.friends, "hp")
	for _, friend in ipairs(self.friends) do
		if self.player:isFriendWith(friend) and friend:getLostHp() > 0 then
			target = friend
		end
	end
	return target
end

sgs.ai_skill_invoke.jgtianyu = true

sgs.ai_skill_invoke.jgjiguan_qinglong = true
sgs.ai_skill_invoke.jgjiguan_baihu = true
sgs.ai_skill_invoke.jgjiguan_zhuque = true
sgs.ai_skill_invoke.jgjiguan_xuanwu = true

sgs.ai_skill_invoke.jgjiguan_bian = true
sgs.ai_skill_invoke.jgjiguan_suanni = true
sgs.ai_skill_invoke.jgjiguan_chiwen = true
sgs.ai_skill_invoke.jgjiguan_yazi = true

sgs.ai_skill_invoke.jgmojian = true

sgs.ai_skill_invoke.jgbenlei = true

sgs.ai_skill_playerchosen.jgtianyun = function(self, targets)
	local target = nil
	local chained = 0
	self:sort(self.enemies, "hp")
	--[[for _, enemy in ipairs(self.enemies) do
		if not self.player:isFriendWith(enemy) and not enemy:hasArmorEffect("PeaceSpell") then
			if self.player:isChained() then
				chained = chained + 1
			end
		end
	end]]
	for _, enemy in ipairs(self.enemies) do
		if not self.player:isFriendWith(enemy) and not enemy:hasArmorEffect("PeaceSpell") then
			if enemy:hasArmorEffect("Vine") or enemy:getMark("@gale") > 0 or (enemy:getCards("e"):length() >= 2) or enemy:getHp() == 1 then
				target = enemy
				break
			end
		end
	end
	--[[if not target and chained > 1 and chained > ( 3 - self.player:getHp() )  then
		for _, enemy in ipairs(self.enemies) do
			if not self.player:isFriendWith(enemy) and not enemy:hasArmorEffect("PeaceSpell") then
				if enemy:isChained() then
					target = enemy
					break
				end
			end
		end
	end
	if not target and self.player:getHp() > 1 then
		for _, enemy in ipairs(self.enemies) do
			if not self.player:isFriendWith(enemy) and not enemy:hasArmorEffect("PeaceSpell") then
				if (enemy:getCards("e"):length() >= 1) then
					target = enemy
					break
				end
			end
		end
	end
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if not self.player:isFriendWith(enemy) and not enemy:hasArmorEffect("PeaceSpell") then
				target = enemy
				break
			end
		end
	end]]
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if self:isGoodChainTarget(enemy, self.player, sgs.DamageStruct_Fire, 2, nil) then
				target = enemy
				break
			end
		end
	end
	return target
end

sgs.ai_playerchosen_intention.jgtianyun = 80

sgs.ai_skill_invoke.jgyizhong = true

function sgs.ai_armor_value.jgyizhong(card)
	if not card then return 5 end
end

sgs.ai_slash_prohibit.jgyizhong = function(self, from, enemy, card)
	if not enemy:getArmor() and card:isBlack() and enemy:hasShownSkill("jgyizhong") then return false end
	return true
end

function sgs.ai_skill_invoke.jglingyu(self, data)
	local weak = 0
	for _, friend in ipairs(self.friends) do
		if friend:getLostHp() > 0 then
			weak = weak + 1
			if self:isWeak(friend) then
				weak = weak + 1
			end
		end
	end
	if not self.player:faceUp() then return true end
	for _, friend in ipairs(self.friends) do
		if friend:hasShownSkills("fangzhu") then return true end
	end
	return weak >= 2
end

sgs.ai_skill_invoke.jgchiying = true

sgs.ai_skill_playerchosen.jgleili = function(self, targets)
	local target = nil
	local chained = 0
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if not self.player:isFriendWith(enemy) and not enemy:hasArmorEffect("PeaceSpell") then
			if self.player:isChained() then
				chained = chained + 1
			end
		end
	end
	if chained > 1 then
		for _, enemy in ipairs(self.enemies) do
			if not self.player:isFriendWith(enemy) and not enemy:hasArmorEffect("PeaceSpell") then
				if enemy:isChained() then
					target = enemy
					break
				end
			end
		end
	end
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if not self.player:isFriendWith(enemy) and not enemy:hasArmorEffect("PeaceSpell") then
				target = enemy
				break
			end
		end
	end
	return target
end

sgs.ai_playerchosen_intention.jgleili = 80

sgs.ai_skill_playerchosen.jgchuanyun = function(self, targets)
	self:updatePlayers()
	local target = nil
	for _, enemy in ipairs(self.enemies) do
		if not self.player:isFriendWith(enemy) and enemy:getHp() > self.player:getHp() then
			target = enemy
			break
		end
	end
	return target
end

sgs.ai_playerchosen_intention.jgchuanyun = 80

sgs.ai_skill_playerchosen.jgfengxing =  sgs.ai_skill_playerchosen.zero_card_as_slash

sgs.ai_playerchosen_intention.jgfengxing = 80

sgs.ai_skill_invoke.jgkonghun = true

function sgs.ai_skill_invoke.jgfanshi(self, data)
	if not self.hasShownSkill("jgfanshi") then return false end
	return true
end

sgs.ai_skill_invoke.jgxuanlei = true

sgs.ai_skill_playerchosen.jghuodi = function(self, targets)
	self:sort(self.friends_noself, "handcard")
	local target = nil
	for _, enemy in ipairs(self.enemies) do
		if enemy:hasShownSkills("jgtianyu|jgtianyun") and not enemy:faceUp() and not self.player:isFriendWith(enemy) then
			target = enemy
			break
		end
	end
	if not target then
		self:sort(self.enemies)
		for _, enemy in ipairs(self.enemies) do
			if self:toTurnOver(enemy) and enemy:hasShownSkills(sgs.priority_skill) and not self.player:isFriendWith(enemy)
			and not (enemy:getMark("@fog") > 0 and enemy:hasShownSkill("jgbiantian")) then
				target = enemy
				break
			end
		end
		if not target then
			for _, enemy in ipairs(self.enemies) do
				if self:toTurnOver(enemy) and not self.player:isFriendWith(enemy)
				and not (enemy:getMark("@fog") > 0 and enemy:hasShownSkill("jgbiantian")) then
					target = enemy
					break
				end
			end
		end
	end
	return target
end

sgs.ai_playerchosen_intention.jghuodi = 80

sgs.ai_skill_invoke.jgjueji = true

sgs.ai_skill_playerchosen.jgdidong = function(self, targets)
	self:sort(self.friends_noself, "handcard")
	local target = nil
	for _, enemy in ipairs(self.enemies) do
		if enemy:hasShownSkills("jgtianyu|jgtianyun") and not enemy:faceUp() then
			target = enemy
			break
		end
	end
	for _, friend in ipairs(self.friends_noself) do
		if not self:toTurnOver(friend) then
			target = friend
			break
		end
	end
	if not target then
		self:sort(self.enemies)
		for _, enemy in ipairs(self.enemies) do
			if self:toTurnOver(enemy) and enemy:hasShownSkills(sgs.priority_skill) and not (enemy:getMark("@fog") > 0 and enemy:hasShownSkill("jgbiantian")) then
				target = enemy
				break
			end
		end
		if not target then
			for _, enemy in ipairs(self.enemies) do
				if self:toTurnOver(enemy) and not (enemy:getMark("@fog") > 0 and enemy:hasShownSkill("jgbiantian")) then
					target = enemy
					break
				end
			end
		end
	end
	return target
end

sgs.ai_playerchosen_intention.jgdidong = 80

sgs.ai_skill_invoke.jglianyu = true

function sgs.ai_skill_invoke.jgtanshi(self, data)
	if not self.hasShownSkill("jgtanshi") then return false end
	return true
end

sgs.ai_skill_invoke.jgtunshi = true

function sgs.ai_skill_invoke.jgdixian(self, data)
	local throw, e= 0, 0
	for _, enemy in ipairs(self.enemies) do
		if not self.player:isFriendWith(enemy) then
			e = enemy:getCards("e"):length()
			throw = throw + e
		end
	end
	if not self.player:faceUp() then return true end
	for _, friend in ipairs(self.friends) do
		if friend:hasShownSkills("fangzhu") then return true end
	end
	return throw >= 3
end

--[[
sgs.ai_trick_prohibit.jgjiguan = function(self, card, to, from)
	if to:hasShownSkills("jgjiguan_qinglong|jgjiguan_baihu|jgjiguan_zhuque|jgjiguan_xuanwu|jgjiguan_bian|jgjiguan_suanni|jgjiguan_chiwen|jgjiguan_yazi") then
		return true
	end
end
]]
