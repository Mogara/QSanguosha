local fuluan_skill={}
fuluan_skill.name="fuluan"
table.insert(sgs.ai_skills,fuluan_skill)
fuluan_skill.getTurnUseCard=function(self,inclusive)
	if self.player:hasUsed("#fuluancard") then return nil end
	return sgs.Card_Parse("#fuluancard:.:")
end
sgs.ai_skill_use_func["#fuluancard"]=function(card,use,self)
	local first_found, second_found, third_found = false, false, false
	local first_card, second_card, third_card
	if self.player:getHandcardNum() >= 3 then
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		for _, fcard in ipairs(cards) do
			if not fcard:isKindOf("ExNihilo") then
				first_card = fcard
				first_found = true
				for _, scard in ipairs(cards) do
					if first_card ~= scard and scard:getSuitString() == first_card:getSuitString() and 
						not scard:isKindOf("ExNihilo") then
						second_card = scard
						second_found = true
						for _, tcard in ipairs(cards) do
							if first_card ~= tcard and second_card ~= tcard and tcard:getSuitString() == first_card:getSuitString() and 
								not tcard:isKindOf("ExNihilo") then
								third_card = tcard
								third_found = true
								break
							end
						end
						if third_card then break end
					end
				end
				if third_card then break end
			end
		end
	end
	if not third_found then return end
	self:sort(self.friends_noself, "defense")
	self:sort(self.enemies, "defense")
	local first_id,second_id,third_id = first_card:getId(),second_card:getId(),third_card:getId()
	for _,p in ipairs(self.friends_noself) do
		if self.player:inMyAttackRange(p) and not p:faceUp() then
			use.card = sgs.Card_Parse("#fuluancard:"..first_id.."+"..second_id.."+"..third_id..":")
			if use.to then use.to:append(p) end
			return
		end
	end
	for _,p in ipairs(self.enemies) do
		if self.player:inMyAttackRange(p) and p:faceUp() then
			use.card = sgs.Card_Parse("#fuluancard:"..first_id.."+"..second_id.."+"..third_id..":")
			if use.to then use.to:append(p) end
			return
		end
	end
end

sgs.ai_use_value.fuluancard = 5.8
sgs.ai_use_priority.fuluancard = 5.8

sgs.ai_chaofeng.wangyuanji = 2

sgs.ai_skill_invoke.zhaoxin = function(self, data)
	local target
	self:sort(self.enemies,"defense")
	for _,enemy in ipairs(self.enemies) do
		local amr=enemy:getArmor()
		if  not (amr and amr:isKindOf("Vine")) and self.player:inMyAttackRange(enemy) and not
			(enemy:hasSkill("kongcheng") and enemy:isKongcheng()) and not self:slashProhibit(nil, enemy) then
			target = enemy
			break
		end
	end
	if target then
		self.room:setPlayerFlag(target, "zhaoxin_target")
		return true
	end
	return false
end

sgs.ai_skill_playerchosen.zhaoxin = function(self, targets)
	local target
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:hasFlag("zhaoxin_target") then
			target = player
			self.room:setPlayerFlag(target, "-zhaoxin_target")
		end
	end
	return target
end

sgs.ai_skill_invoke.langgu = function(self, data)
	local damage = data:toDamage()
	if damage.from and self:isEnemy(damage.from) then
		return true
	end
	return false
end

sgs.ai_skill_askforag.langgu = function(self, card_ids)
	local cards = {}
	for _, card_id in ipairs(card_ids) do
		table.insert(cards, sgs.Sanguosha:getCard(card_id))
	end
	for _,acard in ipairs(cards) do
		local suit = acard:getSuitString()
		if self.player:hasFlag(suit) then
			return acard:getEffectiveId()
		end
	end
	return -1
end

sgs.ai_skill_cardask["@langgu"]=function(self, data)
	return "."
end

sgs.ai_skill_use["@@huangen"] = function(self, prompt)
	local targets = {}
	if self.player:hasFlag("god_salvation") then
		self:sort(self.enemies, "hp")
		for _, enemy in ipairs(self.enemies) do
			if #targets < self.player:getHp() and enemy:hasFlag("huangen") and enemy:isWounded() then 
				table.insert(targets, enemy)
			end
		end
	elseif self.player:hasFlag("amazing_grace") then
		local total = self.room:alivePlayerCount()
		local usefrom = self.room:getCurrent()
		if usefrom then
			local templist = {}
			local tempplayer = usefrom
			for i = 1,total,1 do
				table.insert(templist, tempplayer)
				tempplayer = tempplayer:getNextAlive()
			end
			for j=1,total,1 do
				if #targets < self.player:getHp() and templist[j]:hasFlag("huangen") then 
					if not self:isFriend(templist[j]) then
						table.insert(targets, templist[j])
					else
						break
					end
				end
			end
			for k=1,total,1 do
				if #targets < self.player:getHp() and templist[total-j+1]:hasFlag("huangen") then 
					if self:isFriend(templist[total-k+1]) then
						table.insert(targets, templist[total-k+1])
					else
						break
					end
				end
			end
		end
	else
		self:sort(self.friends, "defense")
		for _, friend in ipairs(self.friends) do
			if #targets < self.player:getHp() and friend:hasFlag("huangen") then 
				table.insert(targets, friend)
			end
		end
	end
	if #targets == 0 then
		return "."
	end
	local targetsname = {}
	for i=1,#targets,1 do
		table.insert(targetsname, targets[i]:objectName())
	end
	local str = "#huangencard:.:->"..table.concat(targetsname,"+")
	return str
end

--hantong discard from pile
sgs.ai_skill_askforag.hantong = function(self, card_ids)
	return card_ids[1]
end

--hantong jijiang viewasskill
local hantong_skill={}
hantong_skill.name="hantong"
table.insert(sgs.ai_skills,hantong_skill)
hantong_skill.getTurnUseCard=function(self,inclusive)
	local cannot = true
	for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if not p:hasFlag("sbleba") and p:getKingdom() == "shu" then
			cannot = false
		end
	end
	if cannot == true then return end
	if ((self.player:canSlashWithoutCrossbow()) or (self.player:getWeapon() and self.player:getWeapon():getClassName() == "Crossbow")) and (not self.player:getPile("hantongpile"):isEmpty() or self.player:hasFlag("hantongjijiang")) then
		return sgs.Card_Parse("#hantongcard:.:")
	end
end

sgs.ai_skill_use_func["#hantongcard"]=function(card,use,self)
	self:sort(self.enemies,"defense")
	for _,enemy in ipairs(self.enemies) do
		if self.player:inMyAttackRange(enemy) and not (enemy:hasSkill("kongcheng") and enemy:isKongcheng())
	 		and not self:slashProhibit(nil, enemy) then
			use.card = sgs.Card_Parse("#hantongcard:.:")
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

--hantong
sgs.ai_skill_invoke.hantong = function(self, data)
	return true
end
--hantong jijiang & hujia 
--I don't think we need it

-- hantong jiuyuan
sgs.ai_skill_invoke.jiuyuan = function(self, data)
	return true
end

--hantong xueyi
sgs.ai_skill_invoke.xueyi = function(self, data)
	local qunnum=0
	for _,p in sgs.qlist(self.room:getOtherPlayers(player)) do		
		if(p:getKingdom() == "qun") then
			qunnum=qunnum+1
		end
	end
	return self.player:getHandcardNum() > self.player:getHp() and qunnum~=0
end

sgs.ai_skill_use["@@diyyicong"] = function(self, prompt)
	local t,j,zero,one,two = 0,0,0,0,0
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)

	for _, c in ipairs(cards) do
		t = t+1
		if c:isKindOf("Jink") or c:isKindOf("Analeptic") or c:isKindOf("Peach") then
			j = j+1
		end
	end
	for _, p in ipairs(self.enemies) do
		if p:distanceTo(self.player) <= p:getAttackRange() then 
			zero = zero+1
		end
		if p:distanceTo(self.player)+1 <= p:getAttackRange() then 
			one = one+1
		end
		if p:distanceTo(self.player)+2 <= p:getAttackRange() then 
			two = two+1
		end
	end
	self:sortByKeepValue(cards)
	if t == 0 then
		return "."
	elseif t == 1 then
		if j == 0 and one < zero then
			return "#diyyicongcard:".. cards[1]:getId()..":->."
		end
	elseif t ==2 then
		if j == 0 and two == 0 and one > two then
			return "#diyyicongcard:".. cards[1]:getId().."+"..cards[2]:getId()..":->."
		end
		if one < zero then 
			return "#diyyicongcard:".. cards[1]:getId()..":->."
		end
	else
		if j==0 and two == 0 and one > two then
			return "#diyyicongcard:".. cards[1]:getId().."+"..cards[2]:getId()..":->."
		end
		return "#diyyicongcard:".. cards[1]:getId()..":->."
	end
	return "."
end

