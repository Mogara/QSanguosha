function sgs.ai_cardsview.jiushi(class_name, player)
	if class_name == "Analeptic" then
		if player:hasSkill("jiushi") and player:faceUp() then
			return ("analeptic:jiushi[no_suit:0]=.")
		end
	end
end

function sgs.ai_skill_invoke.jiushi(self, data)
	return not self.player:faceUp()
end


sgs.ai_skill_use["@@jujian"] = function(self, prompt)
	local needfriend = 0
	local nobasiccard = -1
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	if self:isEquip("SilverLion") and self.player:isWounded() then
			nobasiccard = self.player:getArmor():getId()
	else
		self:sortByKeepValue(cards)
		for _,card in ipairs(cards) do
			if card:getTypeId() ~= sgs.Card_Basic then nobasiccard = card:getEffectiveId() end
		end
	end
	for _, friend in ipairs(self.friends_noself) do
		if self:isWeak(friend) or friend:getHandcardNum() < 2 or not friend:faceUp() 
		or (friend:getArmor() and friend:getArmor():objectName() == "vine" and (friend:isChained() and not self:isGoodChainPartner(friend))) then
			needfriend = needfriend + 1
		end
	end
	if nobasiccard < 0 or needfriend < 1 then return "." end
	self:sort(self.friends_noself,"defense")
	for _, friend in ipairs(self.friends_noself) do
		if not friend:faceUp() then
			return "@JujianCard="..nobasiccard.."->"..friend:objectName()
		end
	end
	for _, friend in ipairs(self.friends_noself) do
		if friend:getArmor() and friend:getArmor():objectName() == "vine" and (friend:isChained() and not self:isGoodChainPartner(friend)) then
			return "@JujianCard="..nobasiccard.."->"..friend:objectName()
		end
	end
	for _, friend in ipairs(self.friends_noself) do
		if self:isWeak(friend) then
			return "@JujianCard="..nobasiccard.."->"..friend:objectName()
		end
	end
	return "@JujianCard="..nobasiccard.."->"..self.friends_noself[1]:objectName()
end

sgs.ai_skill_choice.jujian = function(self, choices)
	if not self.player:faceUp() then return "reset" end
	if self:isEquip("Vine") and self.player:isChained() and not self:isGoodChainPartner() then 
		return "reset"
	end
	if self:isWeak() and self.player:isWounded() then return "recover" end
	return "draw"
end

sgs.ai_card_intention.JujianCard = -100

sgs.xushu_keep_value = 
{
	Peach = 6,
	Jink = 5,
	Weapon = 5,
	Armor = 5,
	DefensiveHorse = 5,
	OffensiveHorse = 5,
	Duel = 5,
	FireAttack = 5,
	ArcheryAttack = 5,
	SavageAssault = 5
}

function sgs.ai_armor_value.yizhong(card)
	if not card then return 4 end
end

local xinzhan_skill={}
xinzhan_skill.name="xinzhan"
table.insert(sgs.ai_skills,xinzhan_skill)
xinzhan_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("XinzhanCard") and self.player:getHandcardNum() > self.player:getMaxHp() then
		return sgs.Card_Parse("@XinzhanCard=.")
	end
end

sgs.ai_skill_use_func.XinzhanCard=function(card,use,self)
	use.card = card
end

sgs.ai_use_value.XinzhanCard = 4.4
sgs.ai_use_priority.XinzhanCard = 9.2

function sgs.ai_slash_prohibit.huilei(self, to)
	if self.player:hasSkill("jueqing") then return false end
	if self:isFriend(to) and self:isWeak(to) then return true end
	return #self.enemies>1 and self:isWeak(to) and (self.player:getHandcardNum()>3 or self:getCardsNum("Shit")>0)
end

sgs.ai_chaofeng.masu = -4

sgs.ai_skill_invoke.enyuan = function(self, data)
	local damage = data:toDamage()
	local from = damage.from
	local source = self.room:getCurrent()
	local xuanhuotarget 
	if from then
		return not self:isFriend(from)
	else
		for _, player in sgs.qlist(self.room:getAlivePlayers()) do
			if player:hasFlag("xuanhuo_target") then
				xuanhuotarget = player
			end
		end
		if xuanhuotarget then
			return self:isFriend(xuanhuotarget)
		else
			return self:isFriend(source)
		end
	end
end

sgs.ai_skill_cardask["@enyuan"] = function(self)
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if  not (card:inherits("Peach") or card:inherits("ExNihilo")) then
			return card:getEffectiveId()
		end
	end
	return "."
end

function sgs.ai_slash_prohibit.enyuan(self)
	if self:isWeak() then return true end
end

sgs.ai_skill_use["@@xuanhuo"] = function(self, prompt)
	local lord = self.room:getLord()
	local killloyal = 0
	local robequip = 0
	if self:isEnemy(lord) then 
		for _, enemy in ipairs(self.enemies) do
			if lord:canSlash(enemy) and (enemy:getHp() < 2 and not enemy:hasSkill("buqu"))
			and sgs.getDefense(enemy) < 2 then
				killloyal = killloyal + 1
			end
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if enemy:getCards("e"):length() > 1 and self:getCardsNum("Slash", enemy) == 0 
		and not self:hasSkills(sgs.lose_equip_skill,enemy) then
			robequip = robequip + 1
		end
	end
	if #self.enemies < 2 and killloyal < 1 and robequip < 1 then return "." end
	if self:isEnemy(lord) and killloyal > 0 then
		self.room:setPlayerFlag(lord, "xuanhuo_target")
		return "@XuanhuoCard=.->"..lord:objectName()
	end
	for _, enemy in ipairs(self.enemies) do
		if enemy:getCards("e"):length() > 1 and self:getCardsNum("Slash", enemy) == 0 
		and not self:hasSkills(sgs.lose_equip_skill,enemy) then
			self.room:setPlayerFlag(enemy, "xuanhuo_target")
			return "@XuanhuoCard=.->"..enemy:objectName()
		end
	end
	self:sort(self.enemies,"defense")
	for _, friend in ipairs(self.friends_noself) do
		for _, enemy in ipairs(self.enemies) do
			if friend:canSlash(enemy) and (enemy:getHp() < 2 and not enemy:hasSkill("buqu"))
			and sgs.getDefense(enemy) < 2 then
				self.room:setPlayerFlag(friend, "xuanhuo_target")
				return "@XuanhuoCard=.->"..friend:objectName()
			end
		end
	end
	for _, friend in ipairs(self.friends_noself) do
		if self:hasSkills(sgs.lose_equip_skill, friend) and not friend:getEquips():isEmpty() then
			self.room:setPlayerFlag(friend, "xuanhuo_target")
			return "@XuanhuoCard=.->"..friend:objectName()
		end
	end
	self:sort(self.friends_noself,"defense")
		self.room:setPlayerFlag(self.friends_noself[1], "xuanhuo_target")
		return "@XuanhuoCard=.->"..self.friends_noself[1]:objectName()
end

sgs.ai_skill_choice.xuanhuo = function(self, choices)
	local fazheng = self.room:findPlayerBySkillName("xuanhuo")
	if fazheng and not self:isFriend(fazheng) then 
		for _, friend in ipairs(self.friends_noself) do
			if self.player:canSlash(friend) and self:isWeak(friend) then
				return "give"
			end
		end
		return "slash"
	end
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy) and (enemy:getHp() < 2 and not enemy:hasSkill("buqu"))
		and sgs.getDefense(enemy) < 2 then
			return "slash"
		end
	end
	return "give"
end

sgs.ai_skill_playerchosen.xuanhuo = function(self, targets)
	targets = sgs.QList2Table(targets)
	self:sort(targets,"defense")
	for _, target in ipairs(targets) do
		if self:isEnemy(target) and not target:hasSkill("buqu") then
			return target
		end
	end
end

sgs.ai_skill_cardask["xuanhuo-slash"] = function(self, data, pattern, target, target2)
	if target and target2  and self:isEnemy(target2) then
		for _, slash in ipairs(self:getCards("Slash")) do
			if self:slashIsEffective(slash, target2) then 
				return slash:toString()
			end 
		end
	end
	if target and target2 and self:isFriend(target2) then
		for _, slash in ipairs(self:getCards("Slash")) do
			if not self:slashIsEffective(slash, target2) then
				return slash:toString()
			end 
		end
		if (target2:getHp() > 2 or self:getCardsNum("Jink", target2) > 1) and not target2:getRole() == "lord" and self.player:getHandcardNum() > 1 then
			for _, slash in ipairs(self:getCards("Slash")) do
				return slash:toString()
			end 
		end
	end
	return "."
end

sgs.ai_playerchosen_intention.xuanhuo = 80
sgs.ai_card_intention.XuanhuoCard = -30

sgs.ai_chaofeng.fazheng = -3


sgs.ai_skill_choice.xuanfeng = function(self, choices)
	return "first"
end

sgs.ai_skill_playerchosen.xuanfeng = function(self, targets)	
	targets = sgs.QList2Table(targets)
	self:sort(targets,"defense")
	for _, enemy in ipairs(self.enemies) do
		if not self:needKongcheng(enemy) and not enemy:isNude() and
		not (enemy:hasSkill("guzheng") and self.room:getCurrent():getPhase() == sgs.Player_Discard) then
			return enemy
		end
	end
end



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

ganlu_skill={}
ganlu_skill.name="ganlu"
table.insert(sgs.ai_skills,ganlu_skill)
ganlu_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("GanluCard") then
		return sgs.Card_Parse("@GanluCard=.")
	end
end

sgs.ai_skill_use_func.GanluCard = function(card, use, self)
	local lost_hp = self.player:getLostHp()
	local enemy_equip = 0
	local target

	for _, friend in ipairs(self.friends) do
		for _, enemy in ipairs(self.enemies) do
			if not self:hasSkills(sgs.lose_equip_skill, enemy) then
				local ee = self:getCardsNum(".",enemy,"e")
				if self:isEquip("GaleShell", enemy) then ee = ee - 1 end
				local fe = self:getCardsNum(".",friend, "e")
				if self:isEquip("GaleShell", friend) then fe = fe - 1 end
				if self:hasSkills(sgs.lose_equip_skill, friend) then ee = ee + fe end
				local value = self:evaluateArmor(enemy:getArmor(),friend) - self:evaluateArmor(friend:getArmor(),enemy)
					- self:evaluateArmor(friend:getArmor(),friend) + self:evaluateArmor(enemy:getArmor(),enemy)
				if math.abs(self:getCardsNum(".", enemy, "e")-self:getCardsNum(".", friend, "e")) <= lost_hp and
					self:getCardsNum(".", enemy, "e")>0 and
					(ee > fe or (ee == fe and value>0)) then
					use.card = sgs.Card_Parse("@GanluCard=.")
					if use.to then
						use.to:append(friend)
						use.to:append(enemy)
					end
					return
				end
			end
		end
	end

	target = nil
	for _,friend in ipairs(self.friends) do
		if self:isEquip("YitianSword", friend) or (self:isEquip("SilverLion",friend) and friend:isWounded()) 
			or (self:hasSkills(sgs.lose_equip_skill, friend) and not friend:getEquips():isEmpty()) then target = friend break end
	end
	if not target then return end
	for _,friend in ipairs(self.friends) do
		if friend~=target and math.abs(self:getCardsNum(".", friend, "e")-self:getCardsNum(".", target, "e")) <= lost_hp then
			use.card = sgs.Card_Parse("@GanluCard=.")
			if use.to then
				use.to:append(friend)
				use.to:append(target)
			end
			return
		end
	end
end

sgs.dynamic_value.control_card.GanluCard = true

sgs.ai_skill_invoke.buyi = function(self, data)
	local dying = data:toDying()
	return self:isFriend(dying.who)
end

sgs.ai_cardshow.buyi = function(self, requestor)
	assert(self.player:objectName() == requestor:objectName())

	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getTypeId() ~= sgs.Card_Basic then
			return card
		end
	end

	return self.player:getRandomHandCard()
end

mingce_skill={}
mingce_skill.name="mingce"
table.insert(sgs.ai_skills,mingce_skill)
mingce_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("MingceCard") then return end

	local card
	if self.player:getArmor() and (self.player:getArmor():objectName() == "silver_lion" and self.player:isWounded()) then
		card = self.player:getArmor()
	end
	if not card then
		local hcards = self.player:getCards("h")
		hcards = sgs.QList2Table(hcards)
		self:sortByUseValue(hcards, true)

		for _, hcard in ipairs(hcards) do
			if hcard:inherits("Slash") or hcard:inherits("EquipCard") then
				card = hcard
				break
			end
		end
	end
	if card then
		card = sgs.Card_Parse("@MingceCard=" .. card:getEffectiveId())
		return card
	end

	return nil
end

sgs.ai_skill_use_func.MingceCard=function(card,use,self)
	local target
	self:sort(self.friends_noself, "defense")
	local friends = self.friends_noself
	for _, friend in ipairs(friends) do
		if friend:getHp() <= 2 and friend:getHandcardNum() < 2 and not (friend:hasSkill("kongcheng") and friend:isKongcheng()) then
			target = friend
			break
		end
	end

	if not target then
		local maxAttackRange=0
		for _, friend in ipairs(friends) do
			if friend:getAttackRange() > maxAttackRange and not (friend:hasSkill("kongcheng") and friend:isKongcheng()) then
				maxAttackRange = friend:getAttackRange()
				target = friend
			end
		end
	end

	if not target then
		local zhugeliang = self.room:findPlayerBySkillName("kongcheng")
		if zhugeliang and zhugeliang:isKongcheng() and zhugeliang:getHp() < 2 and zhugeliang:objectName() ~= self.player:objectName() then
			target = zhugeliang
		end
	end

	if target then
		use.card=card
		if use.to then
			use.to:append(target)
		end
	end
end

sgs.ai_skill_choice.mingce = function(self, choices)
	local chengong = self.room:findPlayerBySkillName("mingce")
	if chengong and not self:isFriend(chengong) then return "draw" end
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	if self.player:getHandcardNum()<=2 then return "draw" end
	if self.player:getHp()<=1 then return "draw" end
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy) and not self:slashProhibit(slash ,enemy) then return "use" end
	end
	return "draw"
end

sgs.ai_skill_playerchosen.mingce = sgs.ai_skill_playerchosen.zero_card_as_slash
sgs.ai_playerchosen_intention.mingce = 80

sgs.ai_use_value.MingceCard = 5.9
sgs.ai_use_priority.MingceCard = 4

sgs.ai_card_intention.MingceCard = -70

sgs.ai_cardneed.mingce = sgs.ai_cardneed.equip
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

		return slash
	end
end

sgs.ai_filterskill_filter.jiejiu = function(card, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:inherits("Analeptic") then return ("slash:jiejiu[%s:%s]=%d"):format(suit, number, card_id) end
end

local xianzhen_skill={}
xianzhen_skill.name="xianzhen"
table.insert(sgs.ai_skills,xianzhen_skill)
xianzhen_skill.getTurnUseCard=function(self)

	if self.player:hasUsed("XianzhenCard") then
		local card_str = "@XianzhenSlashCard=."
		local card = sgs.Card_Parse(card_str)
		return card
	end

	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)

	local max_card = self:getMaxCard()
	local slashNum=self:getCardsNum("Slash")
	if max_card then 
		local max_point = max_card:getNumber()
		if max_card:inherits("Slash") then slashNum=slashNum-1 end
	end

	self:sort(self.enemies, "hp")

	for _, enemy in ipairs(self.enemies) do
		local enemy_max_card = self:getMaxCard(enemy)

		if not enemy:isKongcheng() and enemy_max_card and max_point and max_point > enemy_max_card:getNumber() and slashNum > 0 then

			local slash=self:getCard("Slash")
			local dummy_use={}
			dummy_use.isDummy=true

			local no_distance=true
			self:useBasicCard(slash,dummy_use,no_distance)

			if dummy_use.card then
				local card_id = max_card:getEffectiveId()
				local card_str = "@XianzhenCard=" .. card_id
				local card = sgs.Card_Parse(card_str)
				return card
			end
		end
	end
	self:sortByUseValue(cards, true)
	if self:getOverflow()>0	then
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) and not enemy:isKongcheng() and not enemy:hasSkill("tuntian") then
				local card_id = cards[1]:getEffectiveId()
				local card_str = "@XianzhenCard=" .. card_id
				local card = sgs.Card_Parse(card_str)
				return card
			end
		end
	end
end

sgs.ai_skill_use_func.XianzhenSlashCard=function(card,use,self)
	local target = self.player:getTag("XianzhenTarget"):toPlayer()
	if self:askForCard("slash", "@xianzhen-slash") == "." then return end
	
	if self:getCard("Slash") and not (target:hasSkill("kongcheng") and target:isKongcheng()) and target:isAlive() then
		use.card=card
	end
end

sgs.ai_skill_use_func.XianzhenCard=function(card,use,self)

	self:sort(self.enemies, "handcard")
	local max_card = self:getMaxCard(self.player)
	local max_point = max_card:getNumber()

	for _, enemy in ipairs(self.enemies) do
		local enemy_max_card = self:getMaxCard(enemy)
		if not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1)
			and not enemy:isKongcheng()
			and (enemy_max_card and max_point > enemy_max_card:getNumber()) then
			if use.to then
				use.to:append(enemy)
			end
			use.card=card
			return
		end
	end
	if self:getOverflow()>0	then
		for _, enemy in ipairs(self.enemies) do
			if use.to and not enemy:isKongcheng() then
				use.to:append(enemy)
			end
			use.card=card
			break
		end
	end
end

sgs.ai_cardneed.xianzhen = sgs.ai_cardneed.bignumber
function sgs.ai_skill_pindian.xianzhen(minusecard, self, requestor)
	if self:isFriend(requestor) then return end
	if requestor:getHandcardNum() <= 2 then return minusecard end
end

sgs.ai_card_intention.XianzhenCard = 70

sgs.dynamic_value.control_card.XianzhenCard = true

sgs.ai_use_value.XianzhenCard = 9.2
sgs.ai_use_priority.XianzhenCard = 9.2

sgs.ai_skill_cardask["@xianzhen-slash"] = function(self)
	if self.player:hasSkill("tianxiang") then
		local dmgStr = {damage = 1, nature = 0}
		local willTianxiang = sgs.ai_skill_use["@tianxiang"](self, dmgStr)
		if willTianxiang ~= "." then return "." end
	elseif self.player:hasSkill("longhun") and self.player:getHp() > 1 then
		return "."
	end
	local target = self.player:getTag("XianzhenTarget"):toPlayer()
	local slashes = self:getCards("Slash")
	for _, slash in ipairs(slashes) do
		if self:slashIsEffective(slash, target) then return slash:toString() end
	end
	return "."
end

sgs.ai_use_value.XianzhenSlashCard = 9.2
sgs.ai_use_priority.XianzhenSlashCard = 2.6

sgs.ai_cardshow.quanji = function(self, requestor)
	local index = 0
	local result
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)

	return cards[1]
end

sgs.ai_skill_choice.zili = function(self, choice)
	if self.player:getHp() < self.player:getMaxHp()-1 then return "recover" end
	return "draw"
end

local paiyi_skill={}
paiyi_skill.name="paiyi"
table.insert(sgs.ai_skills, paiyi_skill)
paiyi_skill.getTurnUseCard = function(self)
	if not (self.player:getPile("power"):isEmpty()
		or self.player:hasUsed("PaiyiCard")) then
		return sgs.Card_Parse("@PaiyiCard=.")
	end
end

sgs.ai_skill_use_func.PaiyiCard = function(card, use, self)
	local target
	self:sort(self.friends_noself,"defense")
	for _, friend in ipairs(self.friends_noself) do
		if friend:getHandcardNum() < 2 and friend:getHandcardNum() + 1 < self.player:getHandcardNum() then
			target = friend
		end
		if target then break end
	end
	if not target then
		if self.player:getHandcardNum() < self.player:getHp() + self.player:getPile("power"):length() - 1 then
			target = self.player
		end
	end
	self:sort(self.friends_noself,"hp",true)
	if not target then
		for _, friend in ipairs(self.friends_noself) do
			if friend:getHandcardNum() > 1 and friend:getHandcardNum() + 2 > self.player:getHandcardNum() 
			and self:hasSkills("jieming|yiji|xinsheng|fangzhu|guixin",friend) then
				target = friend
			end
			if target then break end
		end
	end
	self:sort(self.enemies,"defense")
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if not self:hasSkills(sgs.masochism_skill,enemy) and not self:hasSkills("rende|jijiu",enemy)
			and enemy:getHandcardNum() + 2 > self.player:getHandcardNum() then
				target = enemy
			end
			if target then break end
		end
	end

	if target then
		use.card = sgs.Card_Parse("@PaiyiCard=.")
		if use.to then
			use.to:append(target)
		end
	end
end

sgs.ai_skill_askforag.paiyi = function(self, card_ids)
	self.paiyi=card_ids[math.random(1,#card_ids)]
	return self.paiyi
end