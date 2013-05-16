function SmartAI:useCardThunderSlash(...)
	self:useCardSlash(...)
end

sgs.ai_card_intention.ThunderSlash = sgs.ai_card_intention.Slash

sgs.ai_use_value.ThunderSlash = 4.55
sgs.ai_keep_value.ThunderSlash = 2.5
sgs.ai_use_priority.ThunderSlash = 2.5

function SmartAI:useCardFireSlash(...)
	self:useCardSlash(...)
end

sgs.ai_card_intention.FireSlash = sgs.ai_card_intention.Slash

sgs.ai_use_value.FireSlash = 4.6
sgs.ai_keep_value.FireSlash = 2.6
sgs.ai_use_priority.FireSlash = 2.5

sgs.weapon_range.Fan = 4
sgs.ai_use_priority.Fan = 2.655
sgs.ai_use_priority.Vine = 0.95

sgs.ai_skill_invoke.Fan = function(self, data)
	local use = data:toCardUse()	
	local jinxuandi = self.room:findPlayerBySkillName("wuling")
	
	for _, target in sgs.qlist(use.to) do
		if self:isFriend(target) then
			if not self:damageIsEffective(target, sgs.DamageStruct_Fire) then return true end
			if target:isChained() and self:isGoodChainTarget(target) then return true end			
		else
			if not self:damageIsEffective(target, sgs.DamageStruct_Fire) then return false end
			if target:isChained() and not self:isGoodChainTarget(target) then return false end
			if target:hasArmorEffect("Vine") or target:getMark("@gale") > 0 or (jinxuandi and jinxuandi:getMark("@wind") > 0) then
				return true
			end
		end
	end
	return false
end

sgs.ai_skill_invoke.oldFan = function(self, data)
	local target = data:toSlashEffect().to	
	local jinxuandi = self.room:findPlayerBySkillName("wuling")	
	
	if self:isFriend(target) then
		if not self:damageIsEffective(target, sgs.DamageStruct_Fire) then return true end
		if target:isChained() and self:isGoodChainTarget(target) then return true end			
	else
		if not self:damageIsEffective(target, sgs.DamageStruct_Fire) then return false end
		if target:isChained() and not self:isGoodChainTarget(target) then return false end
		if target:hasArmorEffect("Vine") or target:getMark("@gale") > 0 or (jinxuandi and jinxuandi:getMark("@wind") > 0) then
			return true
		end
	end
	return false
end

if sgs.Sanguosha:getVersion() <= "20121221" then sgs.ai_skill_invoke.Fan = sgs.ai_skill_invoke.oldFan end


sgs.ai_view_as.Fan = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()

	local skill_name = card:getSkillName() or ""
	local can_convert = false
	if skill_name == "guhuo" then
		can_convert = true
	else
		local skill = sgs.Sanguosha:getSkill(skill_name)
		if not skill or skill:inherits("FilterSkill") then
			can_convert = true
		end
	end
	if can_convert and card:isKindOf("Slash") and not (card:isKindOf("FireSlash") or card:isKindOf("ThunderSlash")) then
		return ("fire_slash:Fan[%s:%s]=%d"):format(suit, number, card_id)
	end
end

local fan_skill={}
fan_skill.name="Fan"
table.insert(sgs.ai_skills,fan_skill)
fan_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)
	local slash_card
	
	for _,card in ipairs(cards)  do
		if card:isKindOf("Slash") and not (card:isKindOf("FireSlash") or card:isKindOf("ThunderSlash")) then
			slash_card = card
			break
		end
	end
	
	if not slash_card  then return nil end
	local suit = slash_card:getSuitString()
	local number = slash_card:getNumberString()
	local card_id = slash_card:getEffectiveId()
	local card_str = ("fire_slash:Fan[%s:%s]=%d"):format(suit, number, card_id)
	local fireslash = sgs.Card_Parse(card_str)
	assert(fireslash)
	
	return fireslash
		
end

function sgs.ai_weapon_value.Fan(self, enemy)
	if enemy and (self:isEquip("Vine", enemy) or enemy:getMark("@gale") > 0 or  self:isEquip("GaleShell", enemy)) then return 6 end
end

function sgs.ai_armor_value.Vine(player, self)
	if self:needKongcheng(player) and player:getHandcardNum() == 1 then 
		return player:hasSkill("kongcheng") and 5 or 3.8
	end
	if self:hasSkills(sgs.lose_equip_skill, player) then return 3.8 end
	if not self:damageIsEffective(player, sgs.DamageStruct_Fire) then return 6 end
	if self.player:hasSkill("sizhan") then return 4.9 end
	if player:hasSkill("jujian") and not player:getArmor() and #(self:getFriendsNoself(player)) > 0 and player:getPhase() == sgs.Player_Play then return 3 end
	if player:hasSkill("diyyicong") and not player:getArmor() and player:getPhase() == sgs.Player_Play then return 3 end

	for _, enemy in ipairs(self:getEnemies(player)) do
		if (enemy:canSlash(player) and self:isEquip("Fan",enemy)) or self:hasSkills("huoji|longhun|shaoying|zonghuo|wuling", enemy)
		  or (enemy:hasSkill("yeyan") and enemy:getMark("@flame") > 0) then return -2 end
		if getKnownCard(enemy, "FireSlash", true) >= 1 or getKnownCard(enemy, "FireAttack", true) >= 1 or
			getKnownCard(enemy, "Fan") >= 1 then return -2 end
	end

	if (#self.enemies < 3 and sgs.turncount > 2) or player:getHp() <= 2 then return 5 end
	return -1
end

function SmartAI:useCardAnaleptic(card, use)
	if not self.player:hasEquip(card) and not self:hasLoseHandcardEffective() and not self:isWeak()
		and sgs.Analeptic_IsAvailable(self.player, card) then
		use.card = card
	end
end

function SmartAI:searchForAnaleptic(use, enemy, slash)
	if not self.toUse then return nil end
	if not use.to then return nil end

	local anal = self:getCard("Analeptic")
	if not anal then return nil end

	local analAvail = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_Residue, self.player, anal)		
	local slashAvail = 0

	for _, card in ipairs(self.toUse) do
		if analAvail == 1 and card:getId() ~= slash:getId() then return nil end
		if card:isKindOf("Slash") then slashAvail = slashAvail + 1 end
	end
	
	if analAvail > 1 and analAvail < slashAvail then return nil end	
	if self.player:usedTimes("Analeptic") >= analAvail then return nil end

	if (slash and slash:hasFlag("drank")) or self.player:hasFlag("drank") then return nil end

	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:fillSkillCards(cards)
	local allcards = self.player:getCards("he")
	allcards = sgs.QList2Table(allcards)

	if self.player:getPhase() == sgs.Player_Play then
		if self.player:hasFlag("lexue") then
			local lexuesrc = sgs.Sanguosha:getCard(self.player:getMark("lexue"))
			if lexuesrc:isKindOf("Analeptic") then
				local cards = sgs.QList2Table(self.player:getHandcards())
				self:sortByUseValue(cards, true)
				for _, hcard in ipairs(cards) do
					if hcard:getSuit() == lexuesrc:getSuit() then
						local lexue = sgs.Sanguosha:cloneCard("analeptic", lexuesrc:getSuit(), lexuesrc:getNumber())
						lexue:addSubcard(hcard:getId())
						lexue:setSkillName("lexue")
						if self:getUseValue(lexuesrc) > self:getUseValue(hcard) then
							return lexue
						end
					end
				end
			end
		end

		if self.player:hasLordSkill("weidai") and not self.player:hasFlag("weidai_failed") then
			return sgs.Card_Parse("@WeidaiCard=.")
		end
	end

	local card_str = self:getCardId("Analeptic")
	if card_str then return sgs.Card_Parse(card_str) end
		
	for _, anal in ipairs(cards) do
		if (anal:getClassName() == "Analeptic") and not (anal:getEffectiveId() == slash:getEffectiveId()) then
			return anal
		end
	end
end

sgs.dynamic_value.benefit.Analeptic = true

sgs.ai_use_value.Analeptic = 5.98
sgs.ai_keep_value.Analeptic = 4.5
sgs.ai_use_priority.Analeptic = 2.7

local function handcard_subtract_hp(a, b)
	local diff1 = a:getHandcardNum() - a:getHp()
	local diff2 = b:getHandcardNum() - b:getHp()

	return diff1 < diff2
end

function SmartAI:useCardSupplyShortage(card, use)
	local enemies = self:exclude(self.enemies, card)

	local zhanghe = self.room:findPlayerBySkillName("qiaobian")
	local zhanghe_seat = zhanghe and zhanghe:faceUp() and not zhanghe:isKongcheng() and not self:isFriend(zhanghe) and zhanghe:getSeat() or 0
	
	local sb_daqiao = self.room:findPlayerBySkillName("yanxiao")
	local yanxiao = sb_daqiao and not self:isFriend(sb_daqiao) and sb_daqiao:faceUp() and
					(getKnownCard(sb_daqiao, "diamond", nil, "he") > 0
					or sb_daqiao:getHandcardNum() + self:ImitateResult_DrawNCards(sb_daqiao, sb_daqiao:getVisibleSkillList()) > 3
					or sb_daqiao:containsTrick("YanxiaoCard"))

	if #enemies == 0 then return end

	local getvalue = function(enemy)
		if enemy:containsTrick("supply_shortage") or enemy:containsTrick("YanxiaoCard") then return -100 end
		if enemy:getMark("juao") > 0 then return -100 end
		if enemy:hasSkill("qiaobian") and not enemy:containsTrick("supply_shortage") and not enemy:containsTrick("indulgence") then return -100 end
		if zhanghe_seat > 0 and (self:playerGetRound(zhanghe) <= self:playerGetRound(enemy) and self:enemiesContainsTrick() <= 1 or not enemy:faceUp()) then
			return - 100 end
		if yanxiao and (self:playerGetRound(sb_daqiao) <= self:playerGetRound(enemy) and self:enemiesContainsTrick(true) <= 1 or not enemy:faceUp()) then
			return -100 end

		local value = 0 - enemy:getHandcardNum()

		if self:hasSkills("yongsi|haoshi|tuxi|lijian|fanjian|neofanjian|dimeng|jijiu|jieyin|manjuan|beige",enemy)
		  or (enemy:hasSkill("zaiqi") and enemy:getLostHp() > 1)
			then value = value + 10 
		end
		if self:hasSkills(sgs.cardneed_skill,enemy) or self:hasSkills("zhaolie|tianxiang|qinyin|yanxiao|zhaoxin|toudu|renjie",enemy)
			then value = value + 5 
		end
		if self:hasSkills("yingzi|shelie|xuanhuo|buyi|jujian|jiangchi|mizhao|hongyuan|chongzhen|duoshi",enemy) then value = value + 1 end
		if enemy:hasSkill("zishou") then value = value + enemy:getLostHp() end
		if self:isWeak(enemy) then value = value + 5 end
		if enemy:isLord() then value = value + 3 end

		if self:objectiveLevel(enemy) < 3 then value = value - 10 end
		if not enemy:faceUp() then value = value - 10 end
		if self:hasSkills("keji|shensu|jisu", enemy) then value = value - enemy:getHandcardNum() end
		if self:hasSkills("guanxing|xiuluo|tiandu|guidao|noszhenlie", enemy) then value = value - 5 end
		if not sgs.isGoodTarget(enemy, self.enemies, self) then value = value - 1 end
		if self:needKongcheng(enemy) then value = value - 1 end
		if enemy:getMark("@kuiwei") > 0 then value = value - 2 end
		return value
	end

	local cmp = function(a,b)
		return getvalue(a) > getvalue(b)
	end

	table.sort(enemies, cmp)

	local target = enemies[1]
	if getvalue(target) > -100 then
		use.card = card
		if use.to then use.to:append(target) end
		return		
	end
end

sgs.ai_use_value.SupplyShortage = 7
sgs.ai_use_priority.SupplyShortage = 0.5
sgs.ai_card_intention.SupplyShortage = 120

sgs.dynamic_value.control_usecard.SupplyShortage = true

function SmartAI:getChainedFriends(player)
	player = player or self.player
	local chainedFriends = {}
	for _, friend in ipairs(self:getFriends(player)) do
		if friend:isChained() then
			table.insert(chainedFriends, friend)
		end
	end
	return chainedFriends
end

function SmartAI:getChainedEnemies(player)
	player = player or self.player
	local chainedEnemies = {}
	for _, enemy in ipairs(self:getEnemies(player)) do
		if enemy:isChained() then
			table.insert(chainedEnemies,enemy)
		end
	end
	return chainedEnemies
end

function SmartAI:isGoodChainPartner(player)  
	player = player or self.player
 	if player:hasSkill("buqu") or (self.player:hasSkill("niepan") and self.player:getMark("@nirvana") > 0) or self:needToLoseHp(player)
			or self:getDamagedEffects(player) or (player:hasSkill("fuli") and player:getMark("@laoji") > 0) then  
		return true
	end
	return false
end

function SmartAI:isGoodChainTarget(who, source)	
	if not who:isChained() then return false end
	source = source or self.player
	local good = #(self:getChainedEnemies(source))
	local bad = #(self:getChainedFriends(source))
	
	if not sgs.GetConfig("EnableHegemony", false) then	
		local lord = self.room:getLord()
		if lord and self:isWeak(lord) and lord:isChained() and not self:isEnemy(lord, source) then
			return false
		end
	end

	for _, friend in ipairs(self:getChainedFriends(source)) do
		if self:cantbeHurt(friend, nil, source) then
			return false
		end
		if self:isGoodChainPartner(friend) then 
			good = good + 1 
		elseif self:isWeak(friend) then 
			good = good - 1
		end
	end

	for _, enemy in ipairs(self:getChainedEnemies(source)) do
		if self:cantbeHurt(enemy, nil, source) then
			return false
		end
		if self:isGoodChainPartner(enemy) then 
			bad = bad + 1 
		elseif self:isWeak(enemy) then
			bad = bad - 1 
		end
	end
	return good >= bad
end


function SmartAI:useCardIronChain(card, use)	
	use.card = card
	if self.player:hasSkill("noswuyan") then return end	
	if #self.enemies == 1 and #(self:getChainedFriends()) <= 1 then return end
	if self:needBear() then return end
	if self.player:hasSkill("wumou") and self.player:getMark("@wrath") < 7 then return end
	local liuxie = self.room:findPlayerBySkillName("huangen") --ecup
	if liuxie and not self:isFriend(liuxie) then return end	--ecup
	local friendtargets = {}
	local enemytargets = {}
	local yangxiu = self.room:findPlayerBySkillName("danlao")
	self:sort(self.friends, "defense")
	for _, friend in ipairs(self.friends) do
		if friend:isChained() and not self:isGoodChainPartner(friend) and self:hasTrickEffective(card, friend) and not self:hasSkills("danlao|hongyan", friend) then
			table.insert(friendtargets, friend)
		end
	end
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isChained() and not self.room:isProhibited(self.player, enemy, card) 
			and not self:hasSkills("danlao|tianxiang", enemy)
			and self:hasTrickEffective(card, enemy) and self:objectiveLevel(enemy) >= 3
			and not self:getDamagedEffects(enemy) and not self:needToLoseHp(enemy, nil, nil, true) and sgs.isGoodTarget(enemy,self.enemies, self) then
			table.insert(enemytargets, enemy)
		end
	end

	local chainSelf =(self:needToLoseHp(self.player, nil, nil, true) or self:getDamagedEffects(self.player)) and not self.player:isChained()
					and not self.player:hasSkill("jueqing")
					and (self:getCardId("NatureSlash") or (self:getCardId("Slash") and (self.player:hasWeapon("fan") or self.player:hasSkill("lihuo"))) 
					or (self:getCardId("FireAttack") and self.player:getHandcardNum() > 2))
	
	local targets_num = 2 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, card)
	local targets = {}
	
	for i = 1, #friendtargets, 1 do table.insert(targets, friendtargets[i])	end			
	for i = 1, #enemytargets, 1 do table.insert(targets, enemytargets[i]) end			

	if chainSelf then table.insert(targets, self.player) end
	if yangxiu and self:isFriend(yangxiu) then 	table.insert(targets, yangxiu) end

	if use.to and (#targets >=2  or  (#targets == 1 and #(self:getChainedEnemies()) > 0 and (self:isEquip("Vine", targets[1]) or targets[1]:getMark("@gale")>0))) then
		for i = 1, #targets, 1 do
			use.to:append(targets[i])
			if use.to:length() == targets_num then return end
		end				
	end

end

sgs.ai_card_intention.IronChain=function(self, card, from, tos)
	local lord = getLord(from)
	local updated
	local contains_lord = lord and (#tos == 2 and (isLord(tos[1]) or isLord(tos[2])) or #tos == 1 and isLord(tos[1]))
	if not contains_lord and #self:exclude({lord}, card, from) > 0 and CanUpdateIntention(from) and sgs.turncount <= 1 then
		for _, to in ipairs(tos) do
			if not to:isChained() and sgs.ai_role[to:objectName()] == "neutral" and to:objectName() ~= from:objectName() then
				updated = true
				sgs.updateIntention(from, lord, -10)
			end
		end
	end
	if not updated then
		for _, to in ipairs(tos) do
			if to:isChained() then
				sgs.updateIntention(from, to, -10)
			else 
				sgs.updateIntention(from, to, 10)
			end
		end
	end
end

sgs.ai_use_value.IronChain = 5.4
sgs.ai_use_priority.IronChain = 8.5

sgs.dynamic_value.benefit.IronChain = true

sgs.ai_event_callback[sgs.ChoiceMade].fireattack=function(self,player,data)
	local datastr= data:toString()	
	if string.match(datastr,"cardResponded")  and  string.match(datastr,"@fire%-attack") and string.match(datastr,"_nil_") then
		self.room:setPlayerFlag(self.player, "FireAttackFailed_" .. self.room:getTag("LastFireAttack"):toString())
	end
end

sgs.ai_skill_cardask["@fire-attack"] = function(self, data, pattern, target)
	local cards = sgs.QList2Table(self.player:getHandcards())
	local convert = { [".S"] = "spade", [".D"] = "diamond", [".H"] = "heart", [".C"] = "club"} 
	local card

	self:sortByUseValue(cards, true)
	local lord = self.room:getLord()
	if sgs.GetConfig("EnableHegemony", false) then lord = nil end
	
	for _, acard in ipairs(cards) do		
		if acard:getSuitString() == convert[pattern] then
			if not isCard("Peach", acard, self.player) then
				card = acard
				break
			else
				local needKeepPeach = true
				if (self:isWeak(target) and not self:isWeak()) or target:getHp() == 1
						or self:isGoodChainTarget(target) or self:isEquip("Vine", target) or target:getMark("@gale") > 0 then 
					needKeepPeach = false 
				end
				if lord and not self:isEnemy(lord) and sgs.isLordInDanger() and self:getCardsNum("Peach") == 1 and self.player:aliveCount() > 2 then 
					needKeepPeach = true 
				end
				if not needKeepPeach then
					card = acard
					break
				end
			end
		end
	end

	if card then
		return card:getId()
	else
		self.room:setPlayerFlag(self.player, "FireAttackFailed_" .. self.room:getTag("LastFireAttack"):toString())
		return "."
	end
end

function SmartAI:useCardFireAttack(fire_attack, use)  
	if self.player:hasSkill("wuyan") and not self.player:hasSkill("jueqing") then return end
	if self.player:hasSkill("noswuyan") then return end

	local lack = {
		spade = true,
		club = true,
		heart = true,
		diamond = true,
	}

	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getEffectiveId() ~= fire_attack:getEffectiveId() then
			lack[card:getSuitString()] = false
		end
	end

	if self.player:hasSkill("hongyan") then
		lack.spade = true
	end

	local suitnum = 0
	for suit,islack in pairs(lack) do
		if not islack then suitnum = suitnum + 1  end
	end
		

	self:sort(self.enemies, "defense")

	local can_attack = function(enemy)
		if self.player:hasFlag("FireAttackFailed_" .. enemy:objectName()) then return false end
		if self:getOverflow() <= 0 and not self.player:hasSkill("jizhi") then return false end

		return self:objectiveLevel(enemy) > 3 and not enemy:isKongcheng() and not self.room:isProhibited(self.player, enemy, fire_attack) 
			and self:damageIsEffective(enemy, sgs.DamageStruct_Fire, self.player) and not self:cantbeHurt(enemy) 
			and self:hasTrickEffective(fire_attack, enemy)
			and sgs.isGoodTarget(enemy, self.enemies, self)
			and not (enemy:hasSkill("jianxiong") and not self:isWeak(enemy) and not self.player:hasSkill("jueqing"))
			and not (self:getDamagedEffects(enemy,self.player) and not self.player:hasSkill("jueqing"))
			and not (enemy:isChained() and not self:isGoodChainTarget(enemy) and not self.player:hasSkill("jueqing"))
	end
	
	local enemies, targets = {}, {}
	for _, enemy in ipairs(self.enemies) do
		if can_attack(enemy) then
			table.insert(enemies,enemy)
		end
	end
	
	local can_FireAttack_self
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if (not isCard("Peach", card, self.player) or self:getCardsNum("Peach") >= 3) and
			(not isCard("Analeptic", card, self.player) or self:getCardsNum("Analeptic") >= 2) then
			can_FireAttack_self = true
		end
	end
	if self.player:isChained() and self:isGoodChainTarget(self.player) and self.player:getHandcardNum() > 1 and not self.player:hasSkill("jueqing")
			and not self.room:isProhibited(self.player, self.player, fire_attack) and can_FireAttack_self
			and self:damageIsEffective(self.player, sgs.DamageStruct_Fire, self.player) and not self:cantbeHurt(self.player)
			and self:hasTrickEffective(fire_attack, self.player)
			and (self.player:getHp()>1 or self:getCardsNum("Peach")>=1 or self:getCardsNum("Analeptic")>=1 or self.player:hasSkill("buqu")
				or (self.player:hasSkill("niepan") and self.player:getMark("@nirvana") > 0)) then
		table.insert(targets, self.player)
	end

	for _, enemy in ipairs(enemies) do
		if enemy:getHandcardNum() ==1 then
			local handcards = sgs.QList2Table(enemy:getHandcards())
			local flag=string.format("%s_%s_%s","visible",self.player:objectName(),enemy:objectName())
			if handcards[1]:hasFlag("visible") or handcards[1]:hasFlag(flag) then
				local suitstring = handcards[1]:getSuitString()
				if not lack[suitstring] and not table.contains(targets, enemy) then
					table.insert(targets, enemy)
				end
			end
		end
	end
	
	if ((suitnum == 2 and lack.diamond==false) or suitnum<=1) and self:getOverflow()<=0 and #targets == 0 then return end

	for _, enemy in ipairs(enemies) do
		if self:isEquip("Vine", enemy) or enemy:getMark("@gale") > 0 then
			table.insert(targets, enemy)
		end
	end
	for _, enemy in ipairs(enemies) do
		table.insert(targets, enemy)
	end
	
	if #targets > 0 then
		local godsalvation = self:getCard("GodSalvation")
		if godsalvation and godsalvation:getId()~= fire_attack:getId() and self:willUseGodSalvation(godsalvation) then
			use.card = godsalvation 
			return
		end

		local targets_num = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, fire_attack)
		local setTag = false  -- 设置第一个火攻的目标为 LastFireAttack
		use.card = fire_attack
		for i = 1, #targets, 1 do
			if use.to then 
				use.to:append(targets[i])
				if not setTag and (#targets == 1 or (#targets >1 and targets[i]:objectName() ~= self.player:objectName())) then
					self.room:setTag("LastFireAttack",sgs.QVariant(targets[i]:objectName()))
					setTag = true
				end
				if use.to:length() == targets_num then return end
			end
		end
	end
end

sgs.ai_cardshow.fire_attack = function(self, requestor)
	local priority  =
	{
	heart = 4,
	spade = 3,
	club = 2,
	diamond = 1
	}
	if self.player:hasSkill("hongyan") then
		priority  =
		{
			heart = 4,
			spade = 0,
			club = 2,
			diamond = 1
		}
	end
	local index = 0
	local result
	local cards = sgs.QList2Table(self.player:getHandcards())
	if requestor:objectName() == self.player:objectName() then
		self:sortByUseValue(cards, true)
		return cards[1]
	end
		
	for _, card in ipairs(cards) do
		if priority[card:getSuitString()] > index then
			result = card
			index = priority[card:getSuitString()]
		end
	end
	if self.player:hasSkill("hongyan") and result:getSuit() == sgs.Card_Spade then
		result = sgs.Sanguosha:cloneCard(result:objectName(), sgs.Card_Heart, result:getNumber())
		result:setSkillName("hongyan")
	end

	return result
end

sgs.ai_use_value.FireAttack = 4.8
sgs.ai_use_priority.FireAttack = sgs.ai_use_priority.Dismantlement + 0.1

sgs.dynamic_value.damage_card.FireAttack = true

sgs.ai_card_intention.FireAttack = 80
