--乐进
sgs.ai_skill_invoke.gzxiaoguo = function(self, data)
	local tar
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:getPhase() ~= sgs.Player_NotActive then
			tar = player
			break
		end
	end
	local x = self:getCardsNum("Jink", self.player) + self:getCardsNum("Slash", self.player) + self:getCardsNum("Analeptic", self.player)
	if tar and self:isEnemy(tar) and x>0 and not (self.player:getHandcardNum() == 1 and 
		x ==1 ) then
		return true
	end	
	return false
end

sgs.ai_skill_cardask["@gzxiaoguo"]=function(self, data)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if c:isKindOf("BasicCard") and not c:isKindOf("Peach") then
			return c:getId()
		end
	end
	return "."
end

sgs.ai_skill_cardask["@gzxiaoguoresponse"]=function(self, data)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	local equips = self.player:getEquips()
	if not self.player:getEquips():isEmpty() then
		for _, equip in sgs.qlist(equips) do
			if equip:isKindOf("SilverLion") then
				return equip:getId()
			end
		end
	end
	for _, c in ipairs(cards) do
		if c:isKindOf("EquipCard") or c:isKindOf("TrickCard") then
			return c:getId()
		end
	end
	if not self.player:getEquips():isEmpty() then
		for _, equip in sgs.qlist(equips) do
			return equip:getId()
		end
	end
	return "."
end

sgs.ai_skillInvoke_intention.gzxiaoguo = 80

--甘夫人
sgs.ai_skill_invoke.gzshushen = function(self, data)
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self:isFriend(player) then
			return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen.gzshushen1 = function(self, targets)
	local target
	self:sort(self.friends, "defense")
	for _, player in ipairs(self.friends) do
		target = player
		break
	end
	return target
end

sgs.ai_skill_playerchosen.gzshushen2 = function(self, targets)
	local target
	self:sort(self.friends_noself, "defense")
	for _, player in ipairs(self.friends_noself) do
		target = player
		break
	end
	return target
end

sgs.ai_playerchosen_intention.gzshushen = -60

sgs.ai_skill_invoke.gzshenzhi = function(self, data)
	return self.player:getHandcardNum() >= self.player:getHp() and self.player:getHandcardNum() <= self.player:getHp() + math.max(3, self.player:getHp())
			and self.player:getLostHp() > 0 and self:getCardsNum("Peach") == 0
end

sgs.ai_chaofeng.gzganfuren = 3

--陆逊
local gzduoshi_skill={}
gzduoshi_skill.name="gzduoshi"
table.insert(sgs.ai_skills,gzduoshi_skill)
gzduoshi_skill.getTurnUseCard=function(self,inclusive)
	if self.player:hasUsed("#gzduoshicard") then return nil end
	return sgs.Card_Parse("#gzduoshicard:.:")
end

sgs.ai_skill_use_func["#gzduoshicard"]=function(card,use,self)
	local cc
	local pp
	if self.player:getHandcardNum() >= 2 then
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		self:sortByKeepValue(cards)
		for _, fcard in ipairs(cards) do
			if fcard:isRed() and not fcard:isKindOf("ExNihilo") and not fcard:isKindOf("Peach") then
				cc = fcard
				break
			end
		end
	end
	for _,p in ipairs(self.friends_noself) do
		if p:getHandcardNum() >= 2 then
			pp = p
			break
		end
	end
	if cc and pp then
		use.card = sgs.Card_Parse("#gzduoshicard:"..cc:getId()..":")
		if use.to then use.to:append(pp) end
		return
	end
end

sgs.ai_use_value.gzduoshicard = 5
sgs.ai_use_priority.gzduoshicard = 2.61
sgs.ai_card_intention.gzduoshicard =  -81

--丁奉
sgs.ai_skill_invoke.gzduanbing = function(self, data)
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if not self:isFriend(player) and player:hasFlag("duanbingslash") then
			self.room:setPlayerFlag(player, "duanbingslash_target")
			return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen.gzduanbing = function(self, targets)
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasFlag("duanbingslash_target") then
			target = player
			self.room:setPlayerFlag(target, "-duanbingslash_target")
		end
	end
	return target
end

sgs.ai_playerchosen_intention.gzduanbing = 60

local gzfenxunvs_skill={}
gzfenxunvs_skill.name="gzfenxunvs"
table.insert(sgs.ai_skills,gzfenxunvs_skill)
gzfenxunvs_skill.getTurnUseCard=function(self,inclusive)
	if self.player:hasUsed("#gzfenxuncard") then return nil end
	return sgs.Card_Parse("#gzfenxuncard:.:")
end

sgs.ai_skill_use_func["#gzfenxuncard"]=function(card,use,self)
	local cc
	local pp
	if self.player:getHandcardNum() >= 3 then
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		self:sortByKeepValue(cards)
		for _, fcard in ipairs(cards) do
			if not fcard:isKindOf("ExNihilo") and not fcard:isKindOf("Peach") then
				cc = fcard
				break
			end
		end
	end
	self:sort(self.enemies, "defense")
	local x = 0
	for _,p in ipairs(self.enemies) do
		if not player:distanceTo(p)==1 then
			 x = x+1
		
		else
			pp = p
			break
		end
	end
	if cc and pp and x == 0 then
		use.card = sgs.Card_Parse("#gzfenxuncard:"..cc:getId()..":")
		if use.to then use.to:append(pp) end
		return
	end
end

sgs.ai_use_value.gzfenxuncard = 5
sgs.ai_use_priority.gzfenxuncard = 2.61
sgs.ai_card_intention.gzfenxuncard =  70
sgs.ai_chaofeng.gzdingfeng = 2

--孔融
sgs.ai_skill_choice.gzmingshi = function(self, choices)
	local splayer=self.room:findPlayerBySkillName("gzmingshi")
	if not self:isEnemy(splayer) then
		return "mingshicancel"
	end
	return "mingshishow"
end

sgs.ai_skillChoice_intention.gzmingshi = function(from, to, answer)
	local room = from:getRoom()
	local konglong = room:findPlayerBySkillName("gzmingshi")
	if answer == "mingshishow" and konglong then
		sgs.updateIntention(from, konglong, 80)
	elseif answer == "mingshicancel" and konglong then
		sgs.updateIntention(from, konglong, -40)
	end
end

sgs.ai_skill_invoke.gzlirang = function(self, data)
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self:isFriend(player) then
			return true
		end
	end
	return false
end

sgs.ai_skill_askforag.gzlirang = function(self, card_ids)
	return card_ids[1]
end

sgs.ai_skill_playerchosen.gzlirang = function(self, targets)
	local target
	self:sort(self.friends, "defense")
	for _, player in ipairs(self.friends_noself) do
		target = player
		break
	end
	return target
end

sgs.ai_playerchosen_intention.gzlirang = -60
sgs.ai_chaofeng.gzkongrong = 4

--丰田
sgs.ai_skill_invoke.gzsijian = function(self, data)
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if not self:isFriend(player) then
			return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen.gzsijian = function(self, targets)
	local target
	self:sort(self.enemies, "defense")
	for _, player in ipairs(self.enemies) do
		if not (player:isKongcheng() and player:getEquips():isEmpty()) then
			target = player
			break
		end
	end
	return target
end

sgs.ai_playerchosen_intention.gzsijian = 80

--纪灵
sgs.ai_skill_invoke.gzshuangren = function(self, data)
	local ph
	local mch
	local ch
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if not self:isFriend(player) then
			ph = true
		end
	end
	if self:getMaxCard():getNumber() >= 10 then
		mch = true
	end
	if self.player:getHandcardNum() - 1 <= self.player:getHp() then
		ch = true
	end
	if ch and ph then return true end
	if ph and mch then return true end
	return false
end

sgs.ai_skill_playerchosen.gzshuangren = function(self, targets)
	local target
	self:sort(self.enemies, "defense")
	for _, player in ipairs(self.enemies) do
		if not player:isKongcheng() and not (self:isEquip("Vine", player) 
		and not (self:isEquip("QinggangSword", self.player) or self:isEquip("Fan", self.player))) then
			target = player
			break
		end
	end
	return target
end

sgs.ai_playerchosen_intention.gzshuangren = 80

sgs.ai_chaofeng.gzjiling = 4

sgs.ai_skill_invoke.gzhuoshui = function(self, data)
	if not self.player:faceUp() or not self:isWeak() then
		self:sort(self.friends_noself, "defense")
		for _,friend in ipairs(self.friends_noself) do
			if not friend:faceUp() then
				self.gzhuoshui_target = friend
				break
			end
		end	
		if self.gzhuoshui_target then return true end
		
		self:sort(self.enemies)
		for _,enemy in ipairs(self.enemies) do
			if enemy:faceUp() then
				self.gzhuoshui_target = enemy
				break
			end
		end
		if self.gzhuoshui_target then return true end	
	end	
end

sgs.ai_skill_playerchosen.gzhuoshui = function(self, targets)	
	return self.gzhuoshui_target
end

sgs.ai_playerchosen_intention.gzhuoshui = function(from , to)
	local intention = to:faceUp() and 80 or - 80
	sgs.updateIntention(from , to , intention)
end

-- 老版邹氏的祸水
-- sgs.ai_skill_choice.gzhuoshui = function(self, choices)
	-- local str = choices
	-- choices = str:split("+")
	-- if self.player:getHp() < 1 and str:matchOne("buqu") then return "buqu" end
	-- local gskill = {}
	-- local dskill = {}
	-- for _, askill in ipairs(("tuxi|guose|qingnang|lijian|haoshi|dimeng"):split("|")) do
		-- if str:matchOne(askill) then 
			-- table.insert(gskill,askill)
		-- end
	-- end
	-- for _, askill in ipairs(("rende|jijiu|qingnang|buqu|duanchang|huilei|zhuiyi|yibu|jiushi"):split("|")) do
		-- if str:matchOne(askill) then 
			-- table.insert(dskill,askill)
		-- end
	-- end
	-- if self.player:getHp() >= 2 and #gskill ~= 0 then
		-- return gskill[math.random(1,#gskill)]
	-- elseif self.player:getHp() <= 1 and #dskill ~= 0 then
		-- return dskill[math.random(1,#dskill)]
	-- else
		-- return choices[math.random(1,#choices)]
	-- end
-- end

local gzqingcheng_skill={}
gzqingcheng_skill.name="gzqingcheng"
table.insert(sgs.ai_skills,gzqingcheng_skill)
gzqingcheng_skill.getTurnUseCard=function(self,inclusive)
	return sgs.Card_Parse("#gzqingchengcard:.:")
end

sgs.ai_skill_use_func["#gzqingchengcard"]=function(card,use,self)
	local cc
	local pp
	local lion
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)

	for _, fcard in ipairs(cards) do
		cc = fcard
		break
	end
	if self:isEquip("SilverLion", self.player) then
		for _, equip in ipairs(cards) do
			if equip:isKindOf("SilverLion") then
				cc = equip
				lion = true
				break
			end
		end
	end
	self:sort(self.enemies, "defense")
	local x = 0
	for _,p in ipairs(self.enemies) do
		pp = p
		break
	end
	if cc and pp and lion then
		use.card = sgs.Card_Parse("#gzqingchengcard:"..cc:getId()..":")
		if use.to then use.to:append(pp) end
		return
	end
	if cc and pp and sgs.getDefense(pp) < 6 and (self:hasSkills(sgs.masochism_skill,pp) or self:hasSkills(sgs.exclusive_skill,pp))then
		use.card = sgs.Card_Parse("#gzqingchengcard:"..cc:getId()..":")
		if use.to then use.to:append(pp) end
		return
	end
end

sgs.ai_chaofeng.gzzoushi = 2
--马腾
local gzxiongyi_skill={}
gzxiongyi_skill.name="gzxiongyi"
table.insert(sgs.ai_skills,gzxiongyi_skill)
gzxiongyi_skill.getTurnUseCard=function(self,inclusive)
	if self.player:getMark("@xiongyi") == 0 then return nil end
	return sgs.Card_Parse("#gzxiongyicard:.:")
end

sgs.ai_skill_use_func["#gzxiongyicard"]=function(card,use,self)
	if self.player:getHp() *2 + self.player:getHandcardNum() >7 then return end
	targets = {}
	local y = 0
	local x=self.player:aliveCount()
	local n=math.max(2,x%2+x/2)
	for _,p in ipairs(self.friends) do
		if y >= n then
			break
		end
		table.insert(targets, p)
		y = y+1
	end
	--local str =  "#gzxiongyicard:.:->"..table.concat(targetsname,"+")
	--return str
	use.card = sgs.Card_Parse("#gzxiongyicard:.:")
	if use.to then
		for _, pp in  ipairs(targets) do 
			use.to:append(pp) 
		end
	end
	return
end

sgs.ai_use_value.gzxiongyicard = 7
sgs.ai_use_priority.gzxiongyicard = 7
sgs.ai_card_intention.gzxiongyicard = -100

--上将潘凤
sgs.ai_skill_invoke.gzkuangfu = function(self, data)
	local damage = data:toDamage()
	if self:isEnemy(damage.to) then
		return true
	end
	if self:isFriend(damage.to) and self:isEquip("SilverLion", damage.to) then
		return true
	end
	
	return false
end

sgs.ai_skill_choice.gzkuangfu = function(self, choices)
	return "kuangfuget"
end

sgs.ai_cardChosen_intention.gzkuangfu = function(from, to, card_id)
	if not sgs.gzkuangfu_to then return end
	local room = from:getRoom()
	local isSilverLion = sgs.Sanguosha:getCard(card_id):isKindOf("SilverLion") 
	local intention = isSilverLion and 0 or 80
	sgs.updateIntention(from, sgs.gzkuangfu_to, intention)
	sgs.gzkuangfu_to = nil
end