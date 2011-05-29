-- pojun
sgs.ai_skill_invoke.pojun = function(self, data)
	local damage = data:toDamage()
	
	if not damage.to:faceUp() then
		return self:isFriend(damage.to)
	end		
	
	local good = damage.to:getHp() > 2	
	if self:isFriend(damage.to) then
		return good
	elseif self:isEnemy(damage.to) then
		return not good
	end
end

--jiushi
sgs.ai_skill_invoke.jiushi= true

--jiejiu
local jiejiu_skill={}
jiejiu_skill.name="jiejiu"
table.insert(sgs.ai_skills,jiejiu_skill)
jiejiu_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local anal_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:inherits("Analeptic") then 
			anal_card = card
			break
		end
	end

	if anal_card then		
		local suit = anal_card:getSuitString()
    	local number = anal_card:getNumberString()
		local card_id = anal_card:getEffectiveId()
		local card_str = ("slash:jiejiu[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
		
	--	assert(slash)
        
        return slash
	end
end
local gaoshun_ai = SmartAI:newSubclass "gaoshun"

function gaoshun_ai:askForCard(pattern,prompt)
	local card = super.askForCard(self, pattern, prompt)
	if card then return card end
	if pattern == "slash" then
		local cards = self.player:getCards("h")
		cards=sgs.QList2Table(cards)
		self:fillSkillCards(cards)
        self:sortByUseValue(cards,true)
		for _, card in ipairs(cards) do
			if card:inherits("Analeptic") then
				local suit = card:getSuitString()
				local number = card:getNumberString()
				local card_id = card:getEffectiveId()
				return ("slash:jiejiu[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	end
end

-- buyi
sgs.ai_skill_invoke.buyi = function(self, data)
	local dying = data:toDying()
	return self:isFriend(dying.who)
end

--xuanfeng
sgs.ai_skill_choice.xuanfeng = function(self, choices)
	self:sort(self.enemies, "defense")
	local slash = sgs.Card_Parse(("slash[%s:%s]"):format(sgs.Card_NoSuit, 0))
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy)<=1 then
			return "damage"
		elseif not self:slashProhibit(slash ,enemy) then
			return "slash"
		end
	end
	return "nothing"
end

sgs.ai_skill_playerchosen.xuanfeng_damage = function(self,targets)
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy)<=1 then return enemy end
	end

	return nil
end

sgs.ai_skill_playerchosen.xuanfeng_slash = function(self,targets)
	local slash = sgs.Card_Parse(("slash[%s:%s]"):format(sgs.Card_NoSuit, 0))
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if not self:slashProhibit(slash ,enemy) then return enemy end
	end
--	self:log("unfound")
	return self.enemys:at(math.random(0, targets:length() - 1))
end

-- local lingtong_ai = SmartAI:newSubclass "lingtong"
-- lingtong_ai:setOnceSkill("xuanfeng")
-- function lingtong_ai:activate(use)
	-- local cards = self.player:getHandcards()
	-- for _, card in sgs.qlist(cards) do
		-- if card:inherits("EquipCard") then
			-- use.card = card
			-- return
		-- end
	-- end
	-- super.activate(self, use)
-- end

--xuanhuo
local fazheng_ai = SmartAI:newSubclass "fazheng"
fazheng_ai:setOnceSkill("xuanhuo")
function fazheng_ai:activate(use)
	super.activate(self, use)
	if use:isValid() then
		return
	end
	
	local cards = self.player:getHandcards()
	if not self.xuanhuo_used then
		cards=sgs.QList2Table(cards)
		self:sortByUseValue(cards,true)
		
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isKongcheng() then
				for _, card in ipairs(cards)do
					if card:getSuit() == sgs.Card_Heart and not card:inherits("Peach")  and self.player:getHandcardNum() > 1 then
						use.card = sgs.Card_Parse("@XuanhuoCard=" .. card:getEffectiveId())
						use.to:append(enemy)
						self.xuanhuo_used = true
						return
					end	
				end		
			end
		end
		
	end
end
function fazheng_ai:askForPlayerChosen(players, reason)
	if reason == "xuanhuo" then
		for _, player in sgs.qlist(players) do
			if (player:getHandcardNum() <= 2 or player:getHp() < 2) and self:isFriend(player) then
				return player
			end
		end
	end
	for _, player in sgs.qlist(players) do
		if self:isFriend(player) then
			return player
		end
	end
	
	
	return super.askForPlayerChosen(self, players, reason)
end



--ganlu
local wuguotai_ai = SmartAI:newSubclass "wuguotai"
wuguotai_ai:setOnceSkill("ganlu")

function wuguotai_ai:activate(use)
	super.activate(self, use)
	if use:isValid() then
		return
	end
	
	local lost_hp = self.player:getLostHp()
	local enemy_equip = 0
	local target
	
	if not self.ganlu_used then
	
		local has_xiaoji = false
		local xiaoji_equip = 0
		local sunshangxiang
		for _, friend in ipairs(self.friends) do
			if friend:hasSkill("xiaoji") then 
				has_xiaoji = true
				xiaoji_equip = self:getEquipNumber(friend)
				sunshangxiang = friend
				break 
			end
		end
		if has_xiaoji then
			local max_equip, max_friend = 0
			local min_equip, min_friend = 5
		for _, friend in ipairs(self.friends) do
			if not friend:hasSkill("xiaoji") then
				if (self:getEquipNumber(friend) > max_equip) and (self:getEquipNumber(friend)-xiaoji_equip<=lost_hp) then 
				max_equip = self:getEquipNumber(friend) 
				max_friend = friend
				elseif (self:getEquipNumber(friend) < min_equip) and (xiaoji_equip-self:getEquipNumber(friend)<=lost_hp) then 
				min_equip = self:getEquipNumber(friend)
				min_friend = friend
				end
			end
		end
	
		local equips  = {}
		if sunshangxiang and (max_equip~=0 or min_equip~=5) then 
			use.card = sgs.Card_Parse("@GanluCard=.")
			use.to:append(sunshangxiang)
			if (max_equip ~= 0) and ((max_equip-self:getEquipNumber(sunshangxiang))>=0) then
				use.to:append(max_friend)
				self.ganlu_used = true
				return
			elseif(min_equip ~= 5) and ((self:getEquipNumber(sunshangxiang)-min_equip)>=0) then
				use.to:append(min_friend)
				self.ganlu_used = true
				return
			end
		end	
		
		for _, friend in ipairs(self.friends) do
				for _, enemy in ipairs(self.enemies) do
					if not enemy:hasSkill("xiaoji") then 
						if ((self:getEquipNumber(enemy)-self:getEquipNumber(friend))<= lost_hp) and 
							(self:getEquipNumber(enemy)>=self:getEquipNumber(friend))then
							use.card = sgs.Card_Parse("@GanluCard=.")
							use.to:append(friend)
							use.to:append(enemy)
							self.ganlu_used = true
							return
						end
					end
				end			
			end	
		end	
		
	end
	
	
end


--jujian
local xushu_ai = SmartAI:newSubclass "xushu"
xushu_ai:setOnceSkill("jujian")

function xushu_ai:activate(use)

	local abandon_handcard = {}
	local index = 0
	local hasPeach=false
	local find_peach = self.player:getCards("h")
	for _, ispeach in sgs.qlist(find_peach) do
		if ispeach:inherits("Peach") then hasPeach=true break end
	end
	
	local trick_num, basic_num, equip_num = 0, 0, 0
	if not hasPeach and self.player:isWounded() and self.player:getHandcardNum() >=3 and not self.jujian_used then 
		local cards = self.player:getHandcards()
		cards=sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		for _, card in ipairs(cards) do 
			if card:getTypeId() == sgs.Card_Trick and not card:inherits("ExNihilo") then trick_num = trick_num + 1
			elseif card:getTypeId() == sgs.Card_Basic then basic_num = basic_num + 1
			elseif card:getTypeId() == sgs.Card_Equip then equip_num = equip_num + 1
			end
		end
		local result_class
		if trick_num >= 3 then result_class = "TrickCard"
		elseif equip_num >= 3 then result_class = "EquipCard"
		elseif basic_num >= 3 then result_class = "BasicCard"
		end
		for _, friend in ipairs(self.friends_noself) do
			if (friend:getHandcardNum()<2) or (friend:getHandcardNum()<friend:getHp()+1) then
				for _, fcard in ipairs(cards) do 
					if fcard:inherits(result_class) and not fcard:inherits("ExNihilo") then
						table.insert(abandon_handcard, fcard:getId())
						index = index + 1
					end
					if index == 3 then break end
				end
			end
		end
		if index == 3 then 
			use.to:append(friend)
			use.card = sgs.Card_Parse("@JujianCard=" .. table.concat(abandon_handcard, "+"))
			self.jujian_used = true
			return 
		end	
	
	
	elseif not self.jujian_used then
		local cards = self.player:getHandcards()
		cards=sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		local slash_num = self:getSlashNumber(self.player)
		local jink_num = self:getJinkNumber(self.player)
		for _, friend in ipairs(self.friends_noself) do
			if (friend:getHandcardNum()<2) or (friend:getHandcardNum()<friend:getHp()+1) or self.player:isWounded() then
				for _, card in ipairs(cards) do
					if #abandon_handcard == 3 then break end
					if not card:inherits("Nullification") and not card:inherits("EquipCard") and 
						not card:inherits("Peach") and not card:inherits("Jink") and 
						not card:inherits("Indulgence") and not card:inherits("SupplyShortage") then
						table.insert(abandon_handcard, card:getId())
						index = 5
					elseif card:inherits("Slash") and slash_num > 1 then
						if (self.player:getWeapon() and not self.player:getWeapon():objectName()=="crossbow") or
							not self.player:getWeapon() then
							table.insert(abandon_handcard, card:getId())
							index = 5
							slash_num = slash_num - 1
						end
					elseif card:inherits("Jink") and jink_num > 1 then
						table.insert(abandon_handcard, card:getId())
						index = 5
						jink_num = jink_num - 1
					end
				end	
				if index == 5 then 
					use.card = sgs.Card_Parse("@JujianCard=" .. table.concat(abandon_handcard, "+"))
					use.to:append(friend)
					self.jujian_used = true
					return
				end
			end			
		end	
	
	end
	
	super.activate(self, use)
end


--mingce
mingce_skill={}
mingce_skill.name="mingce"
table.insert(sgs.ai_skills,mingce_skill)
mingce_skill.getTurnUseCard=function(self)
	local card
	
	local hcards = self.player:getCards("h")
	hcards = sgs.QList2Table(hcards)
	self:sortByUseValue(hcards, true)

		for _, hcard in ipairs(hcards) do
		if hcard:inherits("Slash") or hcard:inherits("EquipCard") then
			card = hcard
			break
		end
		end

	if card then
		card = sgs.Card_Parse("@MingceCard=" .. card:getEffectiveId()) 
		return card
	end

	return nil
	
end

sgs.ai_skill_use_func["MingceCard"]=function(card,use,self)
	if self.mingce_used then return end
	local target
	self:sort(self.friends_noself, "defense")
	local friends = self.friends_noself
	for _, friend in ipairs(friends) do
		if friend:getHp() <= 2 and friend:getHandcardNum() < 2 then
			target = friend
			break
		end
	end

	if not target then
		local maxAttackRange=0
		for _, friend in ipairs(friends) do
			if friend:getAttackRange() > maxAttackRange then
				maxAttackRange = friend:getAttackRange()
				target = friend
			end
		end
	end

--	if not target then target = self.friends_noself[1] end

	if target then
		use.card=card
		if use.to then
			use.to:append(target)
			self.mingce_used=true
		end
	end
end

sgs.ai_skill_choice.mingce = function(self , choices)
    if self.player:getHandcardNum()<=2 then return "draw" end
	if self.player:getHp()<=1 then return "draw" end
	for _,enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy) and not self:slashProhibit(slash ,enemy) then return "use" end
	end
    return "draw"
end

sgs.ai_skill_playerchosen.mingce = function(self,targets)
	local slash = sgs.Card_Parse(("slash[%s:%s]"):format(sgs.Card_NoSuit, 0))
	local targetlist=sgs.QList2Table(targets)

	self:sort(targetlist, "defense")
	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) and self.player:canSlash(target) and not self:slashProhibit(slash ,target) then
		--self:log("Find!")
		return target
		end
	end
	--self:log("unfound")
	return nil
end

-- local chengong_ai = SmartAI:newSubclass "chengong"

-- function chengong_ai:activate(use)
	-- super.activate(self, use)
	-- if use:isValid() and self:getSlashNumber(self.player) > 1 then
		-- return
	-- end
	
    -- if self.mingce_used then return nil end
		
	-- local card, target
		
	-- local hcards = self.player:getCards("h")
	-- hcards = sgs.QList2Table(hcards)
		-- for _, hcard in ipairs(hcards) do
		-- if hcard:inherits("Slash") or hcard:inherits("EquipCard") then
			-- card = hcard
			-- break
		-- end
	-- end
	
	-- if self:getEquipNumber(self.player) > 0 and not card then
		-- if self.player:getArmor() and self.player:getArmor():objectName() == "silver_lion" and self.player:isWounded() then
			-- card = self.player:getArmor()
		-- end
		-- local ecards = self.player:getCards("e")
		-- ecards = sgs.QList2Table(ecards)
		-- for _, ecard in ipairs(ecards) do
			-- if not (ecard:inherits("Armor") and card:inherits("DefensiveHorse")) then
				-- card = ecard
				-- break
			-- end
		-- end
	-- end	
	
	
	-- if card then 
		-- local friends = self.friends_noself
		-- for _, friend in ipairs(self.friends_noself) do
			-- if friend:getHp() <= 2 and friend:getHandcardNum() < 2 then
				-- target = friend
				-- break
			-- end
		-- end
		-- if not target then target = friends[1] end
	-- end
	-- if card and target then
		-- use.card = sgs.Card_Parse("@MingceCard=" .. card:getId()) 
		-- use.to:append(target)
		-- self.mingce_used=true
		-- return
	-- end
	
	-- return nil
-- end