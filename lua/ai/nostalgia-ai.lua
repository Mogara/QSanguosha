-- danlao
sgs.ai_skill_invoke.danlao = function(self, data)
	local effect = data:toCardEffect()
	if effect.card:inherits("GodSalvation") and self.player:isWounded() then
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
	if (self.jilei_source:hasSkill("paoxiao") or self:isEquip("Crossbow",self.jilei_source)) and self.jilei_source:inMyAttackRange(self.player) then
		return "basic"
	else
		return "trick"
	end
end
	
--tianxiang
sgs.ai_skill_use["@tianxiang"]=function(self, data)
	local friend_lost_hp = 10
	local friend_hp = 0
	local card_id
	local target
	local cant_use_skill
	local dmg

	if data=="@@tianxiang-card" then
		dmg = self.player:getTag("TianxiangDamage"):toDamage()
	else
		dmg = data
	end

	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
	for _,card in ipairs(cards) do
		if (card:getSuit() == sgs.Card_Spade or card:getSuit() == sgs.Card_Heart) and not card:inherits("Peach") then
			card_id = card:getId()
			break
		end
	end
	if not card_id then return "." end

	self:sort(self.enemies, "hp")

	for _, enemy in ipairs(self.enemies) do
		if (enemy:getHp() <= dmg.damage) then

		if (enemy:getHandcardNum() <= 2)
		or enemy:containsTrick("indulgence")
		or enemy:hasSkill("guose")
		or enemy:hasSkill("leiji")
		or enemy:hasSkill("ganglie")
		or enemy:hasSkill("enyuan")
		or enemy:hasSkill("qingguo")
		or enemy:hasSkill("wuyan")
		or enemy:hasSkill("kongcheng")
		then return "@TianxiangCard="..card_id.."->"..enemy:objectName() end
		end
	end

	for _, friend in ipairs(self.friends_noself) do
		if (friend:getLostHp() + dmg.damage>1) then
			if friend:isChained() and #self:getChainedFriends()>1 and dmg.nature>0 then
			elseif friend:getHp() >= 2 and dmg.damage<2 and
			(
			friend:hasSkill("yiji")
			or friend:hasSkill("jieming")
			or (friend:getHandcardNum()<3 and friend:hasSkill("rende"))
			or friend:hasSkill("buqu")
			or friend:hasSkill("shuangxiong")
			or friend:hasSkill("zaiqi")
			or friend:hasSkill("yinghun")
			or friend:hasSkill("jianxiong")
			or friend:hasSkill("fangzhu")
			)
			then return "@TianxiangCard="..card_id.."->"..friend:objectName()
			elseif friend:hasSkill("buqu") then return "@TianxiangCard="..card_id.."->"..friend:objectName() end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if (enemy:getLostHp() <= 1) or dmg.damage>1 then

		if (enemy:getHandcardNum() <= 2)
		or enemy:containsTrick("indulgence")
		or enemy:hasSkill("guose")
		or enemy:hasSkill("leiji")
		or enemy:hasSkill("ganglie")
		or enemy:hasSkill("enyuan")
		or enemy:hasSkill("qingguo")
		or enemy:hasSkill("wuyan")
		or enemy:hasSkill("kongcheng")
		then return "@TianxiangCard="..card_id.."->"..enemy:objectName() end
		end
	end

	for i = #self.enemies, 1, -1 do
		local enemy = self.enemies[i]
		if not enemy:isWounded() and not self:hasSkills(sgs.masochism_skill, enemy) then
			return "@TianxiangCard="..card_id.."->"..enemy:objectName()
		end
	end

	return "."
end

table.insert(sgs.ai_global_flags, "questioner")

sgs.ai_choicemade_filter.cardUsed.GuhuoCard = function(player, carduse)
	if carduse.card:inherits("GuhuoCard") then
		sgs.questioner = nil
		sgs.guhuotype = carduse.card:toString():split(":")[2]
	end
end

sgs.ai_skill_choice["guhuo"] = function(self, choices)
	if sgs.guhuotype and (sgs.guhuotype == "shit" or sgs.guhuotype == "amazing_grace") then return "noquestion" end
	local players = self.room:getOtherPlayers(self.player)
	players = sgs.QList2Table(players)
	local yuji
	if self.player:getHp()<2 then return "noquestion" end
	for _, other in ipairs(players) do
		if other:hasSkill("guhuo") then yuji = other break end
	end
	if self.lua_ai:isFriend(yuji) then return "noquestion"
	elseif sgs.questioner then return "noquestion"
	else
		self:sort(self.friends,"hp")
		if self.player:getHp()<self.friends[#self.friends]:getHp() then return "noquestion" end
	end
	local r=math.random(0,self.player:getHp()-1)
	if r==0 then return "noquestion" else return "question" end
end

sgs.ai_choicemade_filter.skillChoice.guhuo = function(self, promptlist)
	if promptlist[#promptlist] == "yes" then
		sgs.questioner = player
	end
end

local guhuo_skill={}
guhuo_skill.name="guhuo"
table.insert(sgs.ai_skills,guhuo_skill)
guhuo_skill.getTurnUseCard=function(self)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards)

	for _,card in ipairs(cards) do
		if card:isNDTrick() and card:getSuit() == sgs.Card_Heart then
			local dummyuse={}
			dummyuse.isDummy=true
			self:useTrickCard(card, dummyuse)
			if dummyuse.card then
				local parsed_card=sgs.Card_Parse("@GuhuoCard=" .. card:getId() .. ":" .. card:objectName())
				return parsed_card
			end
		end
	end

	local card_str = self:getGuhuoCard("Peach", self.player, true) or self:getGuhuoCard("Analeptic", self.player, true) or self:getGuhuoCard("Slash", self.player, true)
	if card_str then return sgs.Card_Parse(card_str) end

	local guhuo = "peach|ex_nihilo|snatch|amazing_grace|archery_attack|fire_attack"
	local guhuos = guhuo:split("|")
	for _, package in ipairs(sgs.Sanguosha:getBanPackages()) do
		if package == "maneuvering" then
			table.remove(guhuos, #guhuos)
			break
		end
	end

	for _,card in ipairs(cards) do
		if (card:inherits("Slash") and self:getCardsNum("Slash", self.player, "h")>=2 and not self:isEquip("Crossbow"))
		or (card:inherits("Jink") and self:getCardsNum("Jink", self.player, "h")>=3) then
			for i=1, 10 do
				local newguhuo = guhuos[math.random(1,#guhuos)]
				local guhuocard = sgs.Sanguosha:cloneCard(newguhuo, card:getSuit(), card:getNumber())
				local dummyuse = {isDummy = true}
				if newguhuo == "peach" then self:useBasicCard(guhuocard,dummyuse,false) else self:useTrickCard(guhuocard,dummyuse) end
				if dummyuse.card then
					local parsed_card=sgs.Card_Parse("@GuhuoCard=" .. card:getId() .. ":" .. newguhuo)
					return parsed_card
				end
			end
		end
	end
end

sgs.ai_skill_use_func["GuhuoCard"]=function(card,use,self)
	local userstring=card:toString()
	userstring=(userstring:split(":"))[2]
	local guhuocard=sgs.Sanguosha:cloneCard(userstring, card:getSuit(), card:getNumber())
	if guhuocard:getTypeId() == sgs.Card_Basic then self:useBasicCard(guhuocard,use,false) else assert(guhuocard) self:useTrickCard(guhuocard,use) end
	if not use.card then return end
	use.card=card
end