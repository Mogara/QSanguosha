--[[********************************************************************
	Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

  This file is part of QSanguosha-Hegemony.

  This game is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3.0 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  See the LICENSE file for more details.

  QSanguosha-Hegemony Team
*********************************************************************]]

sgs.ai_skill_invoke.jgjizhen = true

sgs.ai_skill_invoke.jglingfeng = function(self, data)
	 return true 
end

sgs.ai_skill_playerchosen.jglingfeng = function(self, targets)
	self:updatePlayers()
	self:sort(self.enemies, "hp")
	local target = nil
	for _, enemy in ipairs(self.enemies) do
		if not self:isFriendWith(enemy) then
			target = enemy
			break
		end
	end
	return target
end

sgs.ai_playerchosen_intention.jglingfeng = 80

sgs.ai_skill_invoke.jgbiantian = true

sgs.ai_skill_playerchosen.jggongshen = function(self, targets)
	self:updatePlayers()
	self:sort(self.friends_noself)
	local target = nil
	for _, friend in ipairs(self.friends_noself) do
		if friend:getGeneral():objectName().contains("machine") and self:isWeak(friend) and self:isFriendWith(friend) then
			target = friend
			break
		end
	end
	if not target then
			if not target then
				for _, enemy in ipairs(self.enemies) do
					if enemy:getGeneral():objectName().contains("machine") and not self:isFriendWith(enemy) then
						target = enemy
						break
					end
				end
			end
		end
	return target
end

sgs.ai_playerchosen_intention.jggongshen = 80

sgs.ai_skill_invoke.jgzhinang = true

sgs.ai_skill_playerchosen.jgzhinang = function(self, targets)
	local id = self.player:getMark("jgzhinang")
	local card = sgs.Sanguosha:getCard(id)
	local cards = { card }
	local c, friend = self:getCardNeedPlayer(cards, self.friends)
	if friend and self:isFriendWith(friend) then return friend end

	self:sort(self.friends)
	for _, friend in ipairs(self.friends) do
		if self:isValuableCard(card, friend) and self:isFriendWith(friend) and not self:needKongcheng(friend, true) then return friend end
	end
	for _, friend in ipairs(self.friends) do
		if self:isWeak(friend) and self:isFriendWith(friend) and not self:needKongcheng(friend, true) then return friend end
	end
	for _, friend in ipairs(self.friends) do
		if self:isFriendWith(friend) and not self:needKongcheng(friend, true) then return friend end
	end
end

sgs.ai_playerchosen_intention.jgzhinang = function(self, from, to)
	if not self:needKongcheng(to, true) and self:isFriendWith(to) then sgs.updateIntention(from, to, -50) end
end

sgs.ai_skill_invoke.jgjingmiao  = true

sgs.ai_skill_invoke.jgyuhuo_pangtong = true
sgs.ai_skill_invoke.jgyuhuo_zhuque = true

sgs.ai_skill_invoke.jgqiwu  = true

sgs.ai_skill_invoke.jgtianyu  = true

sgs.ai_skill_invoke.jgjiguan_qinglong  = true
sgs.ai_skill_invoke.jgjiguan_baihu  = true
sgs.ai_skill_invoke.jgjiguan_zhuque  = true
sgs.ai_skill_invoke.jgjiguan_xuanwu  = true

sgs.ai_skill_invoke.jgjiguan_bian  = true
sgs.ai_skill_invoke.jgjiguan_suanni  = true
sgs.ai_skill_invoke.jgjiguan_taotie  = true
sgs.ai_skill_invoke.jgjiguan_yazi  = true

sgs.ai_skill_invoke.jgmojian  = true

sgs.ai_skill_invoke.jgbenlei  = true

sgs.ai_skill_playerchosen.jgtianyun = function(self, targets)
	local target = nil
	local chained = 0
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if not self:isFriendWith(enemy) and not enemy:hasArmorEffect("PeaceSpell") then 
			if self.player:isChained() then 
				chained = chained + 1
			end
		end	
	end
	for _, enemy in ipairs(self.enemies) do
		if not self:isFriendWith(enemy) and not enemy:hasArmorEffect("PeaceSpell") then 
			if ( enemy:getHp() <= 2 and enemy:hasArmorEffect("Vine") )
			or ((self:isWeak(enemy) and enemy:getCards("e"):length() >= 2) or enemy:getCards("e"):length() >= 3) 			
				then
				target = enemy
				break
			end
		end	
	end
	if not target and chained > 1 and chained > ( 3 - self.player:getHp() )  then 
			for _, enemy in ipairs(self.enemies) do
			if not self:isFriendWith(enemy) and not enemy:hasArmorEffect("PeaceSpell") then 
				if enemy:isChained() then
				target = enemy
				break
				end
			end	
		end
	if not target and self.player:getHp() > 1 then 
		for _, enemy in ipairs(self.enemies) do
			if not self:isFriendWith(enemy) and not enemy:hasArmorEffect("PeaceSpell") then 
				if self.player:getHp() > enemy:getHp() then
				target = enemy
				break
				end
			end	
		end	
	end
	return target
end

sgs.ai_playerchosen_intention.jgtianyun = 80

sgs.ai_skill_invoke.jgyizhong  = true
 
function sgs.ai_armor_value.jgyizhong(card)
	if not card then return 4 end
end

function sgs.ai_slash_prohibit.jgyizhong(self, from, to, card)
	return  (not to:getArmor() and card:isBlack() and to:hasShownSkill("jgyizhong"))
end

function sgs.ai_skill_invoke.jglingyu(self, data)
	local weak = 0
	for _, friend in ipairs(self.friends) do
		if friend:getLostHp() > 0 then 
			weak = weak + 1
			if friend:isWeak() then 
			weak = weak + 1
			end
		end	
	end
	if not self.player:faceUp() then return true end
	for _, friend in ipairs(self.friends) do
		if friend:hasShownSkills("fangzhu") then return true end
	end
	return weak > 4
end

sgs.ai_skill_invoke.jgchiying  = true

sgs.ai_skill_playerchosen.jgleili = function(self, targets)
	local target = nil
	local chained = 0
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if not self:isFriendWith(enemy) and not enemy:hasArmorEffect("PeaceSpell") then 
			if self.player:isChained() then 
				chained = chained + 1
			end
		end	
	end
	if chained > 1 then 
			for _, enemy in ipairs(self.enemies) do
			if not self:isFriendWith(enemy) and not enemy:hasArmorEffect("PeaceSpell") then 
				if enemy:isChained() then
				target = enemy
				break
				end
			end	
		end
	if not target then 
		for _, enemy in ipairs(self.enemies) do
			if not self:isFriendWith(enemy) and not enemy:hasArmorEffect("PeaceSpell") then 
				target = enemy
				break
			end	
		end	
	end
	return target
end

sgs.ai_playerchosen_intention.jgtianyun = 80

sgs.ai_skill_playerchosen.jgchuanyun = function(self, targets)
	self:updatePlayers()
	local target = nil
	for _, enemy in ipairs(self.enemies) do
		if not self:isFriendWith(enemy) and enemy:getHp() > self.player:getHp() then
			target = enemy
			break
		end
	end
	return target
end

sgs.ai_playerchosen_intention.jgchuanyun = 80

sgs.ai_skill_playerchosen.jgfengxing = function(self, targets)
	local slash = sgs.cloneCard("slash")
	local targetlist = sgs.QList2Table(targets)
	local arrBestHp, canAvoidSlash, forbidden = {}, {}, {}
	self:sort(targetlist, "defenseSlash")

	for _, target in ipairs(targetlist) do
		if not self:isFriendWith(target) and not self:slashProhibit(slash ,target) and sgs.isGoodTarget(target, targetlist, self) then
			if self:slashIsEffective(slash, target) then
				if self:getDamagedEffects(target, self.player, true) or self:needLeiji(target, self.player) then
					table.insert(forbidden, target)
				elseif self:needToLoseHp(target, self.player, true, true) then
					table.insert(arrBestHp, target)
				else
					return target
				end
			else
				table.insert(canAvoidSlash, target)
			end
		end
	end

	if #canAvoidSlash > 0 then return canAvoidSlash[1] end
	if #arrBestHp > 0 then return arrBestHp[1] end

	self:sort(targetlist, "defenseSlash")
	targetlist = sgs.reverse(targetlist)
	for _, target in ipairs(targetlist) do
		if target:objectName() ~= self.player:objectName() and not self:isFriendWith(target) and not table.contains(forbidden, target) then
			return target
		end
	end

	return targetlist[1]
end

sgs.ai_playerchosen_intention.jgfengxing = 80

sgs.ai_skill_invoke.jgkonghun  = true

function sgs.ai_skill_invoke.jgfanshi(self, data)
	if not self.hasShownSkill:("jgfanshi") then return false end
	return true
end

sgs.ai_skill_invoke.jgxuanlei  = true

sgs.ai_skill_playerchosen.jghuodi = function(self, targets)
	self:updatePlayers()
	local target = nil
	for _, enemy in ipairs(self.enemies) do
		if not self:isFriendWith(enemy) and enemy:faceUp() then
			target = enemy
			break
		end
	end
	return target
end

sgs.ai_playerchosen_intention.jghuodi = 80

sgs.ai_skill_invoke.jgjueji  = true

sgs.ai_skill_playerchosen.jgdidong = sgs.ai_skill_playerchosen.jghuodi

sgs.ai_skill_invoke.jglianyu  = true

function sgs.ai_skill_invoke.jgtanshi(self, data)
	if not self.hasShownSkill:("jgtanshi") then return false end
	return true
end

sgs.ai_skill_invoke.jgtunshi  = true

function sgs.ai_skill_invoke.jgdixian(self, data)
	local throw = 0
	for _, enemy in ipairs(self.enemies) do
		if not self:isFriendWith(enemy) then 
			local e = enemy:getCards("e"):length()  
			throw = throw + e
		end	
	end
	if not self.player:faceUp() then return true end
	for _, friend in ipairs(self.friends) do
		if friend:hasShownSkills("fangzhu") then return true end
	end
	return throw > 3
end










