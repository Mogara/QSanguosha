sgs.weapon_range.SPMoonSpear = 3

sgs.ai_skill_invoke.sp_moonspear = function(self, data)
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	for _, target in ipairs(self.enemies) do
		if self.player:canSlash(target) and not self:slashProhibit(slash ,target) then
		return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen.sp_moonspear = sgs.ai_skill_playerchosen.zero_card_as_slash
sgs.ai_playerchosen_intention.sp_moonspaer = 80

function sgs.ai_slash_prohibit.weidi(self, to, card)
	if to:isLord() then return false end
	local lord = self.room:getLord()
	for _, askill in sgs.qlist(lord:getVisibleSkillList()) do
		if askill:objectName() ~= "weidi" and askill:isLordSkill() then
			local filter = sgs.ai_slash_prohibit[askill:objectName()]
			if  type(filter) == "function" and filter(self, to, card) then return true end
		end
	end
end

sgs.ai_chaofeng.yuanshu = 3

sgs.ai_skill_invoke.danlao = function(self, data)
	local effect = data:toCardUse()
	local current = self.room:getCurrent()
	if effect.card:inherits("GodSalvation") and self.player:isWounded() then
		return false
	elseif effect.card:inherits("AmazingGrace") and
		(self.player:getSeat() - current:getSeat()) % (global_room:alivePlayerCount()) < global_room:alivePlayerCount()/2 then
		return false
	else
		return true
	end
end

sgs.ai_skill_invoke.jilei = function(self, data)
	local damage = data:toDamage()
	if not damage then return false end
	self.jilei_source = damage.from
	return self:isEnemy(damage.from)
end

sgs.ai_skill_choice.jilei = function(self, choices)
	local tmptrick = sgs.Sanguosha:cloneCard("ex_nihilo", sgs.Card_NoSuit, 0)
	if (self:isEquip("Crossbow",self.jilei_source) and self.jilei_source:inMyAttackRange(self.player)) or
		 self.jilei_source:isJilei(tmptrick) then
		return "basic"
	else
		return "trick"
	end
end

local function yuanhu_validate(self, equip_type, is_handcard)
	local is_SilverLion = false
	if equip_type == "SilverLion" then
		equip_type = "Armor"
		is_SilverLion = true
	end
	local targets
	if is_handcard then targets = self.friends else targets = self.friends_noself end
	if equip_type ~= "Weapon" then
		if equip_type == "DefensiveHorse" or equip_type == "OffensiveHorse" then self:sort(targets, "hp") end
		if equip_type == "Armor" then self:sort(targets, "handcard") end
		if is_SilverLion then
			for _, enemy in ipairs(self.enemies) do
				if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
					local seat_diff = enemy:getSeat() - self.player:getSeat()
					local alive_count = self.room:alivePlayerCount()
					if seat_diff < 0 then seat_diff = seat_diff + alive_count end
					if seat_diff > alive_count / 2.5 + 1 then return enemy	end
				end
			end
			for _, enemy in ipairs(self.enemies) do
				if enemy:hasSkill("bazhen") or enemy:hasSkill("yizhong") then
					return enemy
				end
			end
		end
		for _, friend in ipairs(targets) do
			if not self:isEquip(equip_type, friend) then
				if equip_type == "Armor" then
					if not self:needKongcheng(friend) and not self:hasSkills("bazhen|yizhong", friend) then return friend end
				else
					if friend:isWounded() and not friend:hasSkill("longhun") then return friend end
				end
			end
		end
	else
		for _, friend in ipairs(targets) do
			if not self:isEquip(equip_type, friend) then
				for _, aplayer in sgs.qlist(self.room:getAllPlayers()) do
					if friend:distanceTo(aplayer) == 1 then
						if self:isFriend(aplayer) and not aplayer:containsTrick("YanxiaoCard")
							and (aplayer:containsTrick("indulgence") or aplayer:containsTrick("supply_shortage")
								or (aplayer:containsTrick("lightning") and self:hasWizard(self.enemies))) then
							self.room:setPlayerFlag(aplayer, "YuanhuToChoose")
							return friend
						end
					end
				end
				self:sort(self.enemies, "defense")
				for _, enemy in ipairs(self.enemies) do
					if friend:distanceTo(enemy) == 1 and not enemy:isNude() then
						self.room:setPlayerFlag(enemy, "YuanhuToChoose")
						return friend
					end
				end
			end
		end
	end
	return nil
end

sgs.ai_skill_use["@@yuanhu"] = function(self, prompt)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	if self:isEquip("SilverLion") and yuanhu_validate(self, "SilverLion", false) then
		local player = yuanhu_validate(self, "SilverLion", false)
		local card_id = self.player:getArmor():getEffectiveId()
		return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
	end
	if self.player:getOffensiveHorse() and yuanhu_validate(self, "OffensiveHorse", false) then
		local player = yuanhu_validate(self, "OffensiveHorse", false)
		local card_id = self.player:getOffensiveHorse():getEffectiveId()
		return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
	end
	if self.player:getWeapon() and yuanhu_validate(self, "Weapon", false) then
		local player = yuanhu_validate(self, "Weapon", false)
		local card_id = self.player:getWeapon():getEffectiveId()
		return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
	end
	if self.player:getArmor() and self.player:getLostHp() <= 1 and self.player:getHandcardNum() >= 3
		and yuanhu_validate(self, "Armor", false) then
		local player = yuanhu_validate(self, "Armor", false)
		local card_id = self.player:getArmor():getEffectiveId()
		return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("DefensiveHorse") and yuanhu_validate(self, "DefensiveHorse", true) then
			local player = yuanhu_validate(self, "DefensiveHorse", true)
			local card_id = card:getEffectiveId()
			return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
		end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("OffensiveHorse") and yuanhu_validate(self, "OffensiveHorse", true) then
			local player = yuanhu_validate(self, "OffensiveHorse", true)
			local card_id = card:getEffectiveId()
			return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
		end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("Weapon") and yuanhu_validate(self, "Weapon", true) then
			local player = yuanhu_validate(self, "Weapon", true)
			local card_id = card:getEffectiveId()
			return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
		end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("SilverLion") and yuanhu_validate(self, "SilverLion", true) then
			local player = yuanhu_validate(self, "SilverLion", true)
			local card_id = card:getEffectiveId()
			return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
		end
		if card:isKindOf("Armor") and yuanhu_validate(self, "Armor", true) then
			local player = yuanhu_validate(self, "Armor", true)
			local card_id = card:getEffectiveId()
			return "@YuanhuCard=" .. card_id .. "->" .. player:objectName()
		end
	end
end

sgs.ai_skill_playerchosen.yuanhu = function(self, targets)
	targets = sgs.QList2Table(targets)
	for _, p in ipairs(targets) do
		if p:hasFlag("YuanhuToChoose") then
			self.room:setPlayerFlag(p, "-YuanhuToChoose")
			return p
		end
	end
	for _, p in sgs.qlist(self.room:getAllPlayers()) do
		if p:hasFlag("YuanhuToChoose") then
			self.room:setPlayerFlag(p, "-YuanhuToChoose")
		end
	end
end

sgs.ai_card_intention.YuanhuCard = -30

sgs.ai_skill_use["@@bifa"] = function(self, prompt)
	if #self.enemies == 0 then return "." end
	self:sort(self.enemies)
	local target
	for _, p in ipairs(self.enemies) do
		if p:getPile("#pencil"):isEmpty() then
			target = p
			break
		end
	end
	local cards = sgs.QList2Table(self.player:getCards("h"))
	self:sortByUseValue(cards, true)
	if self:getUseValue(cards[1]) < 5 and target then
		return "@BifaCard=" .. cards[1]:getEffectiveId() .. "->" .. target:objectName()
	end
	return "."
end

sgs.ai_skill_cardask["@bifa-give"] = function(self, data, pattern, target)
	local pen = data:toCard()
	local typpe = pen:getType()
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)
	for _, c in ipairs(cards) do
		if c:getType() == typpe and c ~= pen then
		    return c:getId()
		end
	end
	return "."
end

local songci_skill = {}
songci_skill.name = "songci"
table.insert(sgs.ai_skills, songci_skill)
songci_skill.getTurnUseCard = function(self)
	return sgs.Card_Parse("@SongciCard=.")
end

sgs.ai_skill_use_func.SongciCard = function(card,use,self)
	self:sort(self.friends, "handcard")
	for _, friend in ipairs(self.friends) do
		if friend:getMark("@sonnet") == 0 and friend:getHandcardNum() < friend:getHp() and not (friend:hasSkill("manjuan") and self.room:getCurrent() ~= friend) then
			if not (friend:hasSkill("haoshi") and friend:getHandcardNum() <= 1 and friend:getHp() >= 3) then
				use.card = sgs.Card_Parse("@SongciCard=.")
				if use.to then use.to:append(friend) end
				return
			end
		end
	end
	
	self:sort(self.enemies, "handcard")
	self.enemies = sgs.reverse(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		if enemy:getMark("@sonnet") == 0 and enemy:getHandcardNum() > enemy:getHp() and not enemy:isNude()
		  and not self:doNotDiscard(enemy, "he", nil, 2, true) then
			use.card = sgs.Card_Parse("@SongciCard=.")
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

sgs.ai_use_value.SongciCard = 3
sgs.ai_use_priority.SongciCard = 3
sgs.ai_chaofeng.chenlin = 3

sgs.ai_card_intention.SongciCard = function(card, from, tos)	
	for _, to in ipairs(tos) do
		if to:getHandcardNum() > to:getHp() then
			sgs.updateIntention(from, to, 100)
		elseif to:getHandcardNum() < to:getHp() then
			sgs.updateIntention(from, to, -100)
		end
	end	
end

sgs.ai_skill_invoke.tianming = function(self, data)
	if self:hasSkill("manjuan") and self.room:getCurrent() ~= self.player then return false end
	if self:getCardsNum("Jink") == 0 then return true end
    local unpreferedCards={}
    local cards=sgs.QList2Table(self.player:getHandcards())

    local zcards = self.player:getCards("he")
    for _, zcard in sgs.qlist(zcards) do
        if not zcard:isKindOf("Peach") and not zcard:isKindOf("ExNihilo") then
            table.insert(unpreferedCards,zcard:getId())
        end
    end

    if #unpreferedCards == 0 then
        if self:getCardsNum("Slash")>1 then
            self:sortByKeepValue(cards)
            for _,card in ipairs(cards) do
                if card:isKindOf("Slash") then table.insert(unpreferedCards,card:getId()) end
            end
            table.remove(unpreferedCards, 1)
        end

        local num=self:getCardsNum("Jink") - 1
        if self.player:getArmor() then num=num+1 end
        if num>0 then
            for _,card in ipairs(cards) do
                if card:isKindOf("Jink") and num>0 then
                    table.insert(unpreferedCards,card:getId())
                    num=num-1
                end
            end
        end
        for _,card in ipairs(cards) do
            if (card:isKindOf("Weapon") and self.player:getHandcardNum() < 3) or card:isKindOf("OffensiveHorse") or
                self:getSameEquip(card, self.player) or	card:isKindOf("AmazingGrace") or card:isKindOf("Lightning") then
                table.insert(unpreferedCards,card:getId())
            end
        end

        if self.player:getWeapon() and self.player:getHandcardNum()<3 then
            table.insert(unpreferedCards, self.player:getWeapon():getId())
        end

	    if (self:isEquip("SilverLion") and self.player:isWounded()) then
            table.insert(unpreferedCards, self.player:getArmor():getId())
        end

        if self.player:getOffensiveHorse() and self.player:getWeapon() then
            table.insert(unpreferedCards, self.player:getOffensiveHorse():getId())
        end
    end

    for index = #unpreferedCards, 1, -1 do
        if self.player:isJilei(sgs.Sanguosha:getCard(unpreferedCards[index])) then table.remove(unpreferedCards, index) end
    end

    if #unpreferedCards >= 2 or #unpreferedCards == #cards then
        return true
    end
end

sgs.ai_skill_discard.tianming = function(self, discard_num, min_num, optional, include_equip)
    local unpreferedCards={}
    local cards=sgs.QList2Table(self.player:getHandcards())

    local zcards = self.player:getCards("he")
    for _, zcard in sgs.qlist(zcards) do
        if not zcard:isKindOf("Peach") and not zcard:isKindOf("ExNihilo") then
            table.insert(unpreferedCards,zcard:getId())
        end
    end

    if #unpreferedCards == 0 then
        if self:getCardsNum("Slash")>1 then
            self:sortByKeepValue(cards)
            for _,card in ipairs(cards) do
                if card:isKindOf("Slash") then table.insert(unpreferedCards,card:getId()) end
            end
            table.remove(unpreferedCards, 1)
        end

        local num=self:getCardsNum("Jink") - 1
        if self.player:getArmor() then num=num+1 end
        if num>0 then
            for _,card in ipairs(cards) do
                if card:isKindOf("Jink") and num>0 then
                    table.insert(unpreferedCards,card:getId())
                    num=num-1
                end
            end
        end
        for _,card in ipairs(cards) do
            if (card:isKindOf("Weapon") and self.player:getHandcardNum() < 3) or card:isKindOf("OffensiveHorse") or
                self:getSameEquip(card, self.player) or	card:isKindOf("AmazingGrace") or card:isKindOf("Lightning") then
                table.insert(unpreferedCards,card:getId())
            end
        end

        if self.player:getWeapon() and self.player:getHandcardNum()<3 then
            table.insert(unpreferedCards, self.player:getWeapon():getId())
        end

	    if (self:isEquip("SilverLion") and self.player:isWounded()) then
            table.insert(unpreferedCards, self.player:getArmor():getId())
        end

        if self.player:getOffensiveHorse() and self.player:getWeapon() then
            table.insert(unpreferedCards, self.player:getOffensiveHorse():getId())
        end
    end

    for index = #unpreferedCards, 1, -1 do
        if self.player:isJilei(sgs.Sanguosha:getCard(unpreferedCards[index])) then table.remove(unpreferedCards, index) end
    end

    local to_discard = {}
	local count = 0
	for index = #unpreferedCards, 1, -1 do
	    table.insert(to_discard, unpreferedCards[index])
		count = count + 1
		if count == 2 then return to_discard end
    end
	return to_discard
end

local mizhao_skill={}
mizhao_skill.name="mizhao"
table.insert(sgs.ai_skills, mizhao_skill)
mizhao_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("MizhaoCard") or self.player:isKongcheng() then return end
	local cards = self.player:getHandcards()
	local allcard = {}
	cards = sgs.QList2Table(cards)
	for _,card in ipairs(cards)  do
		table.insert(allcard,card:getId())
	end
	local parsed_card = sgs.Card_Parse("@MizhaoCard=" .. table.concat(allcard,"+"))
	return parsed_card
end

sgs.ai_skill_use_func.MizhaoCard=function(card,use,self)
	local handcardnum = self.player:getHandcardNum()
	local trash = self:getCard("Disaster") or self:getCard("GodSalvation") or self:getCard("AmazingGrace")
	local count = 0
	for _, enemy in ipairs(self.enemies) do
		if enemy:isKongcheng() then count = count + 1 end
	end
	if handcardnum == 1 and trash and #self.enemies - count >= 2 and #self.friends_noself == 0 then
		self:sort(self.enemies, "hp")
		use.card = card
        if use.to then use.to:append(self.enemies[1]) end
		return
	end
    self:sort(self.friends_noself, "hp")
    for _, friend in ipairs(self.friends_noself) do
        if not friend:hasSkill("manjuan") then
			use.card = card
			if use.to then use.to:append(friend) end
			return
		end
    end
end

sgs.ai_use_priority.MizhaoCard = 1.5
sgs.ai_card_intention.MizhaoCard = 20

sgs.ai_skill_playerchosen.mizhao = function(self, targets)
	self:sort(self.enemies, "hp")
    for _, enemy in ipairs(self.enemies) do
		if targets:contains(enemy) then return enemy end
	end
end

function sgs.ai_skill_pindian.mizhao(minusecard, self, requestor, maxcard)
	local cards, maxcard = sgs.QList2Table(self.player:getHandcards())
	local function compare_func1(a, b)
		return a:getNumber() > b:getNumber()
	end
	local function compare_func2(a, b)
		return a:getNumber() < b:getNumber()
	end
	if self:isFriend(requestor) and self.player:getHp() > requestor:getHp() then
		table.sort(cards, compare_func2)
	else
		table.sort(cards, compare_func1)
	end
	for _, card in ipairs(cards) do
		if self:getUseValue(card) < 6 then maxcard = card break end
	end
	return maxcard or cards[1]
end

sgs.ai_skill_cardask["@JieyuanIncrease"] = function(self, data)
	local damage = data:toDamage()
	local target = damage.to
	if self:isFriend(target) then return "." end
	if self:isEquip("SilverLion", target) then return "." end
	local cards=sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _,card in ipairs(cards) do
		if card:isBlack() then return "$" .. card:getEffectiveId() end
	end
	return "."
end

sgs.ai_skill_cardask["@JieyuanDecrease"] = function(self, data)
	local damage = data:toDamage()
	if self:hasSkills(sgs.masochism_skill) and damage.damage <= 1 and self.player:getHp() > 1 then return "." end
	local cards=sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	for _,card in ipairs(cards) do
		if card:isRed() then return "$" .. card:getEffectiveId() end
	end
	return "."
end

sgs.ai_skill_invoke.fenxin = function(self, data)
	local target = data:toPlayer()
    local target_role = sgs.evaluatePlayerRole(target)
	local self_role = self.player:getRole()
	if target_role == "renegade" or target_role == "unknown" then return false end
	local count1, count2 = 1, 0
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
	    if target_role == "loyalist" then count1 = count1 + 1 end
	    if target_role == "rebel" then count2 = count2 + 1 end
	end
	if self_role ~= "loyalist" and target_role == "loyalist" and count1 >= count2 then return true end
	if self_role ~= "rebel" and target_role == "rebel" and count1 < count2 then return true end
	return false
end

sgs.ai_skill_invoke.moukui = function(self, data)
	local target = data:toPlayer()
    return not self:isFriend(target) 
end

sgs.ai_skill_choice.moukui = function(self, choices)
	return "discard"
end

sgs.ai_skill_playerchosen.xingwu = sgs.ai_skill_playerchosen.xuanhuo

sgs.ai_skill_invoke["aocai"] = function(self, data)
	local target = data:toPlayer()
	if not target then return true end
	return self:isFriend(target)
end

local duwu_skill={}
duwu_skill.name = "duwu"
table.insert(sgs.ai_skills, duwu_skill)
duwu_skill.getTurnUseCard=function(self)
    if not self.player:hasUsed("DuwuCard") and not self.player:isNude() then
		local card_str
		self:sort(self.enemies)
		local cards = sgs.QList2Table(self.player:getCards("he"))
		self:sortByUseValue(cards, true)
		for _, enemy in ipairs(self.enemies) do
			if self.player:inMyAttackRange(enemy) and self.player ~= enemy then
				if enemy:getHp() == 1 and #cards > 0 then
					self.duwutarget = enemy
					card_str = cards[1]:getEffectiveId()
					break
				elseif enemy:getHp() == 2 and #cards > 1 then
					self.duwutarget = enemy
					card_str = cards[1]:getEffectiveId() .. "+" .. cards[2]:getEffectiveId()
					break
				end
			end
		end
		if self.duwutarget then
			return sgs.Card_Parse("@DuwuCard=" .. card_str)
		end
	end
end
sgs.ai_skill_use_func["DuwuCard"]=function(card,use,self)
	if use.to then use.to:append(self.duwutarget) end
	use.card=card
end

-- sp dusts
sgs.ai_skill_invoke["#xiangxiangp"] = function(self, data)
	local lord = self.room:getLord()
	if lord:hasLordSkill("shichou") then
		return self:isFriend(lord)
	end
	return lord:getKingdom() == "shu"
end

sgs.ai_chaofeng.sp_sunshangxiang = sgs.ai_chaofeng.sunshangxiang

sgs.ai_skill_invoke["#ducaip"] = function(self, data)
	local lord = self.room:getLord()
	if lord:hasLordSkill("xueyi") then
		return not self:isFriend(lord)
	end
	return lord:getKingdom() == "wei"
end

sgs.ai_chaofeng.sp_caiwenji = sgs.ai_chaofeng.caiwenji

sgs.ai_skill_invoke["#guanyup"] = function(self, data)
	local lord = self.room:getLord()
	if lord:hasLordSkill("jijiang") then
		return not self:isFriend(lord)
	end
	return lord:getKingdom() == "wei"
end

sgs.ai_skill_invoke["#machaop"] = function(self, data)
	local lord = self.room:getLord()
	if lord:hasSkill("xueyi") then
		return self:isFriend(lord)
	end
	if lord:hasLordSkill("shichou") then
		return not self:isFriend(lord)
	end

	return lord:getKingdom() == "qun"
end

sgs.ai_chaofeng.sp_machao = sgs.ai_chaofeng.machao

sgs.ai_skill_invoke["#diaochanp"] = function(self, data)
	return math.random(0, 2) == 0
end

sgs.ai_chaofeng.sp_diaochan = sgs.ai_chaofeng.diaochan

sgs.ai_skill_invoke["#pangdep"] = sgs.ai_skill_invoke["#ducaip"]

sgs.ai_skill_invoke["#jiaxup"] = sgs.ai_skill_invoke["#ducaip"]
