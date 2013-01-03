-- yuejin
sgs.ai_skill_cardask["@xiaoguo"] = function(self, data, pattern, target)
	local target = data:toPlayer()
	if self:isFriend(target) then return "." end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _, c in ipairs(cards) do
		if c:isKindOf("BasicCard") and not c:isKindOf("Peach") then
		    return c:getId()
		end
	end
	return "."
end

sgs.ai_skill_cardask["@xiaoguoresponse"]=function(self, data)
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

sgs.ai_skill_invoke.shushen = function(self, data)
    for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
	    if self:isFriend(player) then
	        return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen.shushen1 = function(self, targets)
	local target
	self:sort(self.friends, "defense")
	for _, player in ipairs(self.friends) do
		target = player
		break
	end
	return target
end

sgs.ai_skill_playerchosen.shushen2 = function(self, targets)
	local target
	self:sort(self.friends_noself, "defense")
	for _, player in ipairs(self.friends_noself) do
		target = player
		break
	end
	return target
end

sgs.ai_skill_invoke.shenzhi = function(self, data)
	return self.player:getHandcardNum() > self.player:getLostHp() and self.player:isWounded()
end

sgs.ai_chaofeng.ganfuren = 3

local duoshi_skill={}
duoshi_skill.name="duoshi"
table.insert(sgs.ai_skills,duoshi_skill)
duoshi_skill.getTurnUseCard=function(self,inclusive)
	if self.player:hasUsed("DuoshiCard") then return nil end
	return sgs.Card_Parse("@DuoshiCard=.")
end
sgs.ai_skill_use_func["DuoshiCard"]=function(card,use,self)
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
	    use.card = sgs.Card_Parse("@DuoshiCard:"..cc:getId())
		if use.to then use.to:append(pp) end
		return
	end
end

sgs.ai_use_value.duoshi = 4
sgs.ai_use_priority.duoshi = 4

sgs.ai_skill_choice.mingshi = function(self, choices)
    local splayer=self.room:findPlayerBySkillName("mingshi")
    if not self:isEnemy(splayer) then
	    return "mingshicancel"
	end
	return "mingshishow"
end

sgs.ai_skill_invoke.duanbing = function(self, data)
	local use = data:toCardUse()
	local targets
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
        if self.player:distanceTo(p) == 1 and not use.to:contains(p) and self.player:canSlash(p, false) then
			if self:isEnemy(p) then
				self.duanbingtarget = p
				return true
			end
		end
	end
end

sgs.ai_skill_playerchosen.duanbing = function(self, targets)
	return self.duanbingtarget
end

local fenxun_skill={}
fenxun_skill.name="fenxun"
table.insert(sgs.ai_skills,fenxun_skill)
fenxun_skill.getTurnUseCard=function(self,inclusive)
	if self.player:hasUsed("FenxunCard") then return nil end
	return sgs.Card_Parse("@FenxunCard=.")
end
sgs.ai_skill_use_func["FenxunCard"]=function(card,use,self)
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
	    if not self.player:distanceTo(p)==1 then
		    x = x+1
	    else
		    pp = p
			break
		end
	end
	if cc and pp and x == 0 then
	    use.card = sgs.Card_Parse("@FenxunCard:"..cc:getId())
		if use.to then use.to:append(pp) end
		return
	end
end

sgs.ai_chaofeng.dingfeng = 2

sgs.ai_skill_invoke.lirang = function(self, data)
	return #self.friends_noself > 0
end

sgs.ai_skill_playerchosen.lirang = function(self, targets)
	self:sort(self.friends_noself, "defense")
	return self.friends_noself[1]
end

sgs.ai_skill_choice.mingshi = function(self, choices)
	return "mingshishow"
end

sgs.ai_chaofeng.kongrong = 4

sgs.ai_skill_invoke.sijian = true
sgs.ai_skill_choice.sijian = function(self, choices)
	return "mo"
end
sgs.ai_skill_playerchosen.sijian = function(self, targets)
	return self.player
end

sgs.ai_skill_use["@@shuangren"]=function(self,prompt)
	if self.player:isKongcheng() then return "." end
	local cards = sgs.QList2Table(self.player:getCards("h"))
	self:sortByUseValue(cards, true)
	if cards[1]:getNumber() > 9 then
		self:sort(self.enemies, "handcard")
		for _, player in ipairs(self.enemies) do
			if not player:isKongcheng() then
				return "@ShuangrenCard="..cards[1]:getEffectiveId().."->"..player:objectName()
			end
		end
	else
		self:sort(self.friends_noself, "handcard2")
		for _, player in ipairs(self.friends_noself) do
			if not player:isKongcheng() then
				return "@ShuangrenCard="..cards[1]:getEffectiveId().."->"..player:objectName()
			end
		end
	end
	return "."
end

sgs.ai_skill_playerchosen.shuangren = function(self, targets)
	self:sort(self.enemies, "defense")
	for _, player in ipairs(self.enemies) do
		if not player:isKongcheng() and not (self:isEquip("Vine", player) 
		and not (self:isEquip("QinggangSword", self.player) or self:isEquip("Fan", self.player))) then
			return player
		end
	end
end

sgs.ai_chaofeng.jiling = 4

sgs.ai_skill_invoke.huoshui = function(self, data)
	return false
end

local qingcheng_skill={}
qingcheng_skill.name="qingcheng"
table.insert(sgs.ai_skills,qingcheng_skill)
qingcheng_skill.getTurnUseCard=function(self,inclusive)
	return sgs.Card_Parse("@QingchengCard=.")
end

sgs.ai_skill_use_func["QingchengCard"]=function(card,use,self)
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
	    use.card = sgs.Card_Parse("@QingchengCard:"..cc:getId())
		if use.to then use.to:append(pp) end
		return
	end
	if cc and pp and sgs.getDefense(pp) < 6 and (self:hasSkills(sgs.masochism_skill,pp) or self:hasSkills(sgs.exclusive_skill,pp))then
	    use.card = sgs.Card_Parse("@QingchengCard:"..cc:getId())
		if use.to then use.to:append(pp) end
		return
	end
end

sgs.ai_chaofeng.zoushi = 2

local xiongyi_skill={}
xiongyi_skill.name="xiongyi"
table.insert(sgs.ai_skills,xiongyi_skill)
xiongyi_skill.getTurnUseCard=function(self,inclusive)
    if self.player:getMark("@xiongyi") == 0 then return nil end
	return sgs.Card_Parse("@XiongyiCard=.")
end

sgs.ai_skill_use_func["XiongyiCard"]=function(card,use,self)
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
	--local str =  "#xiongyicard:.:->"..table.concat(targetsname,"+")
	--return str
	use.card = sgs.Card_Parse("@XiongyiCard=.")
	if use.to then
    	for _, pp in ipairs(targets) do 
		    use.to:append(pp) 
		end
	end
	return
end

sgs.ai_skill_invoke.kuangfu = function(self, data)
    local damage = data:toDamage()
	if self:isEnemy(damage.to) then
		return true
	end
	if self:isFriend(damage.to) and self:isEquip("SilverLion", damage.to) then
	    return true
	end
	return false
end

sgs.ai_skill_choice.kuangfu = function(self, choices)
	return "kuangfuget"
end
