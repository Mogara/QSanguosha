sgs.ai_skill_invoke.jianxiong = function(self, data)
	return not sgs.Shit_HasShit(data:toCard())
end

table.insert(sgs.ai_global_flags, "hujiasource")

sgs.ai_skill_invoke.hujia = function(self, data)
	local cards = self.player:getHandcards()
	if sgs.hujiasource then return false end
	for _, friend in ipairs(self.friends_noself) do
		if friend:getKingdom() == "wei" and self:isEquip("EightDiagram", friend) then return true end
	end
	for _, card in sgs.qlist(cards) do
		if card:inherits("Jink") then
			return false
		end
	end
	return true
end

sgs.ai_choicemade_filter.skillInvoke.hujia = function(player, promptlist)
	if promptlist[#promptlist] == "yes" then
		sgs.hujiasource = player
	end
end

function sgs.ai_slash_prohibit.hujia(self, to)
	if self:isFriend(to) then return false end
	local guojia = self.room:findPlayerBySkillName("tiandu")
	if guojia and guojia:getKingdom() == "wei" and self:isFriend(to, guojia) then return sgs.ai_slash_prohibit.tiandu(self, guojia) end
end

sgs.ai_choicemade_filter.cardResponsed["@hujia-jink"] = function(player, promptlist)
	if promptlist[#promptlist] ~= "_nil_" then
		sgs.updateIntention(player, sgs.hujiasource, -80)
		sgs.hujiasource = nil
	end
end

sgs.ai_skill_cardask["@hujia-jink"] = function(self)
	if not self:isFriend(sgs.hujiasource) then return "." end
	if self:needBear() then return "." end
	return self:getCardId("Jink") or "."
end

sgs.ai_skill_invoke.fankui = function(self, data)
	local target = data:toPlayer()
	if self:isFriend(target) then
		return (target:hasSkill("xiaoji") and not target:getEquips():isEmpty()) or (self:isEquip("SilverLion",target) and target:isWounded())
	end
	if self:isEnemy(target) then				---fankui without zhugeliang and luxun
		if target:hasSkill("tuntian") then return false end
		if (self:needKongcheng(target) or target:hasSkill("lianying")) and target:getHandcardNum() == 1 then
			if not target:getEquips():isEmpty() then return true
			else return false
			end
		end
	end
	--self:updateLoyalty(-0.8*sgs.ai_loyalty[target:objectName()],self.player:objectName())
	return true
end

sgs.ai_skill_cardask["@guicai-card"]=function(self, data)
	local judge = data:toJudge()

	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getHandcards())
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return "@GuicaiCard=" .. card_id
		end
	end

	return "."
end

sgs.simayi_suit_value = 
{
	heart = 3.9,
	club = 3.9,
	spade = 3.5
}

sgs.ai_chaofeng.simayi = -2

sgs.ai_skill_invoke.ganglie = function(self, data)
	return not self:isFriend(data:toPlayer())
end

sgs.ai_skill_discard.ganglie = function(self, discard_num, optional, include_equip)
	if self:getOverflow() > 0 then return end

	if self.player:getHandcardNum() == 3 then
		local to_discard = {}
		local cards = self.player:getHandcards()
		local index = 0
		local all_peaches = 0
		for _, card in sgs.qlist(cards) do
			if card:inherits("Peach") then
				all_peaches = all_peaches + 1
			end
		end
		if all_peaches >= 2 then return {} end
		self:sortByKeepValue(cards)
		cards = sgs.reverse(cards)

		for i = #cards, 1, -1 do
			local card = cards[i]
			if not card:inherits("Peach") and not self.player:isJilei(card) then
				table.insert(to_discard, card:getEffectiveId())
				table.remove(cards, i)
				index = index + 1
				if index == 2 then break end
			end
		end	
		if index < 2 then
			for i = #cards, 1, -1 do
				local card = cards[i]
				if not self.player:isJilei(card) then
					table.insert(to_discard, card:getEffectiveId())
					table.remove(cards, i)
					index = index + 1
					if index == 2 then break end
				end
			end	
		end
		return to_discard
	end

	if self.player:getHandcardNum() < 2 then return {} end
end

function sgs.ai_slash_prohibit.ganglie(self, to)
	if self:isWeak() or self:isFriend(to) then return true end
	return self.player:getHandcardNum()+self.player:getHp() < 5
end

sgs.ai_chaofeng.xiahoudun = -3

sgs.ai_skill_use["@@tuxi"] = function(self, prompt)
	self:sort(self.enemies, "handcard")

	local first_index, second_index
	for i=1, #self.enemies-1 do
		if self:hasSkills(sgs.need_kongcheng, self.enemies[i]) and self.enemies[i]:getHandcardNum() == 1 then
		elseif not self.enemies[i]:isKongcheng() then
			if not first_index then
				first_index = i
			else
				second_index = i
			end
		end
		if second_index then break end
	end

	if first_index and not second_index then
		local others = self.room:getOtherPlayers(self.player)
		for _, other in sgs.qlist(others) do
			if (not self:isFriend(other) or (self:hasSkills(sgs.need_kongcheng, other) and other:getHandcardNum() == 1)) and
				self.enemies[first_index]:objectName() ~= other:objectName() and not other:isKongcheng() then
				return ("@TuxiCard=.->%s+%s"):format(self.enemies[first_index]:objectName(), other:objectName())
			end
		end
	end

	if not second_index then return "." end

	self:log(self.enemies[first_index]:getGeneralName() .. "+" .. self.enemies[second_index]:getGeneralName())
	local first = self.enemies[first_index]:objectName()
	local second = self.enemies[second_index]:objectName()
	return ("@TuxiCard=.->%s+%s"):format(first, second)
end

sgs.ai_card_intention.TuxiCard = 80

sgs.ai_chaofeng.zhangliao = 4

sgs.ai_skill_invoke.luoyi=function(self,data)
	if self.player:isSkipped(sgs.Player_Play) then return false end
	local cards=self.player:getHandcards()
	cards=sgs.QList2Table(cards)

	for _,card in ipairs(cards) do
		if card:inherits("Slash") then

			for _,enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy, true) and
				self:slashIsEffective(card, enemy) and
				( (not enemy:getArmor()) or (enemy:getArmor():objectName()=="renwang_shield") or (enemy:getArmor():objectName()=="vine") ) and
				enemy:getHandcardNum()< 2 then
					if not self.player:containsTrick("indulgence") then
						self:speak("luoyi")
						return true
					end
				end
			end
		end
	end
	return false
end

sgs.xuchu_keep_value = 
{
	Peach 			= 6,
	Analeptic 		= 5.8,
	Jink 			= 5.7,
	FireSlash 		= 5.6,
	Slash 			= 5.4,
	ThunderSlash 	= 5.5,	
	Axe				= 5,
	Blade 			= 4.9,
	Spear 			= 4.9,
	Fan				= 4.8,
	KylinBow		= 4.7,
	Halberd			= 4.6,
	MoonSpear		= 4.5,
	DefensiveHorse 	= 4
}

sgs.ai_chaofeng.xuchu = 3

sgs.ai_skill_invoke.tiandu = sgs.ai_skill_invoke.jianxiong

function sgs.ai_slash_prohibit.tiandu(self, to)
	if self:isEnemy(to) and self:isEquip("EightDiagram", to) then return true end
end

sgs.ai_chaofeng.guojia = -4

sgs.ai_view_as.qingguo = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:isBlack() and card_place ~= sgs.Player_Equip then
		return ("jink:qingguo[%s:%s]=%d"):format(suit, number, card_id)
	end
end

sgs.zhenji_suit_value = 
{
	spade = 4.1,
	club = 4.2
}

local rende_skill={}
rende_skill.name="rende"
table.insert(sgs.ai_skills, rende_skill)
rende_skill.getTurnUseCard=function(self)
	if self.player:isKongcheng() then return end
	for _, player in ipairs(self.friends_noself) do
		if ((player:hasSkill("haoshi") and not player:containsTrick("supply_shortage"))
			or player:hasSkill("longluo")
			or (not player:containsTrick("indulgence") and (player:hasSkill("zhiheng") or player:hasSkill("yishe")))
			)
			and player:faceUp() then
			return sgs.Card_Parse("@RendeCard=.")
		end
	end
	if (self.player:usedTimes("RendeCard") < 2 or self:getOverflow() > 0 or self:getCard("Shit")) then
		return sgs.Card_Parse("@RendeCard=.")
	end
	if self.player:getLostHp() < 2 then
		return sgs.Card_Parse("@RendeCard=.")
	end
end

sgs.ai_skill_use_func.RendeCard = function(card, use, self)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards,true)
	local name = self.player:objectName()
	local card, friend = self:getCardNeedPlayer(cards)
	if card and friend then
		use.card = sgs.Card_Parse("@RendeCard=" .. card:getId())
		if use.to then use.to:append(friend) end
		return
	end
	
	if #self.friends == 1 then return end

	if (self:getOverflow()>0 or (self.player:isWounded() and self.player:usedTimes("RendeCard") < 2)) then
		self:sort(self.friends_noself, "handcard")
		local friend
		for _, player in ipairs(self.friends_noself) do
			if self:needKongcheng(player) then
			elseif not player:containsTrick("indulgence") then friend = player break end
		end
		if friend then
			local card_id = self:getCardRandomly(self.player, "h")
			use.card = sgs.Card_Parse("@RendeCard=" .. card_id)
			if use.to then use.to:append(friend) end
			return
		end
	end
	
	if self.player:getHandcardNum()==1 then
		for _, enemy in ipairs(self.enemies) do
			if self:isEquip("GudingBlade", enemy) and enemy:canSlash(self.player, true) then return end
		end
	end

	local special={}
	for _,player in ipairs(self.friends_noself) do
		if ((player:hasSkill("haoshi") and not player:containsTrick("supply_shortage"))
			or player:hasSkill("longluo")
			or (not player:containsTrick("indulgence") and (player:hasSkill("zhiheng") or player:hasSkill("yishe")))
			)
			and player:faceUp() then
			table.insert(special, player)
		end
	end
	
	if #special>0 then
		self:sort(special, "defense")
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if not card:inherits("Jink") or self:getCardsNum("Jink")>1 then
				use.card = sgs.Card_Parse("@RendeCard=" .. card:getId())
				if use.to then use.to:append(special[1]) end
				return
			end
		end
	end

	if self.player:getLostHp() < 2 then
		local card
		self:sortByUseValue(cards, true)
		for _, hcard in ipairs(cards) do
			if hcard:inherits("Jink") then if self:getCardsNum("Jink")>#self.enemies/2 then card = hcard end
			elseif hcard:inherits("Slash") then if self:getCardsNum("Slash") > 1 then card = hcard end
			elseif not hcard:inherits("Nullification") then card = hcard end
			if card then break end
		end
		if card then
			self:sort(self.friends_noself, "handcard")
			for _, friend in ipairs(self.friends_noself) do
				local draw = 2
				if friend:containsTrick("supply_shortage") then draw = 0 end
				if friend:getHandcardNum() + draw < friend:getMaxCards() and not friend:containsTrick("indulgence") and not
					((friend:isKongcheng() and (friend:hasSkill("kongcheng") or (friend:hasSkill("zhiji") and not friend:hasSkill("guanxing")))) or
					(not self:isWeak(friend) and self:hasSkills(sgs.need_kongcheng,friend))) then
					use.card = sgs.Card_Parse("@RendeCard=" .. card:getId())
					if use.to then use.to:append(friend) end
					return
				end
			end
			self:sort(self.friends_noself, "handcard")
			for _, friend in ipairs(self.friends_noself) do
				if not friend:containsTrick("indulgence") then
					use.card = sgs.Card_Parse("@RendeCard=" .. card:getId())
					if use.to then use.to:append(self.friends_noself[1]) end
					return
				end
			end
		end
	end
end

sgs.ai_use_value.RendeCard = 8.5
sgs.ai_use_priority.RendeCard = 5.8

sgs.ai_card_intention.RendeCard = -70

sgs.dynamic_value.benefit.RendeCard = true

table.insert(sgs.ai_global_flags, "jijiangsource")
local jijiang_filter = function(player, carduse)
	if carduse.card:inherits("JijiangCard") then
		sgs.jijiangsource = player
	else
		sgs.jijiangsource = nil
	end
end

table.insert(sgs.ai_choicemade_filter.cardUsed, jijiang_filter)

sgs.ai_skill_invoke.jijiang = function(self, data)
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("Slash") then
			return false
		end
	end
	if sgs.jijiangsource then return false else return true end
end

sgs.ai_choicemade_filter.skillInvoke.jijiang = function(player, promptlist)
	if promptlist[#promptlist] == "yes" then
		sgs.jijiangsource = player
	end
end

local jijiang_skill={}
jijiang_skill.name="jijiang"
table.insert(sgs.ai_skills,jijiang_skill)
jijiang_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("JijiangCard") or not self:slashIsAvailable() then return end
	local card_str = "@JijiangCard=."
	local slash = sgs.Card_Parse(card_str)
	assert(slash)

	return slash
end

sgs.ai_skill_use_func.JijiangCard=function(card,use,self)
	self:sort(self.enemies, "defense")
	local target_count=0
	for _, enemy in ipairs(self.enemies) do
		if (self.player:canSlash(enemy, not no_distance) or
			(use.isDummy and self.player:distanceTo(enemy)<=(self.predictedRange or self.player:getAttackRange())))
			and self:objectiveLevel(enemy)>3 and self:slashIsEffective(card, enemy) then
			use.card=card
			if use.to then
				use.to:append(enemy)
			end
			target_count=target_count+1
			if self.slash_targets<=target_count then return end
		end
	end	
end

sgs.ai_use_value.JijiangCard = 8.5
sgs.ai_use_priority.JijiangCard = 2.4

sgs.ai_choicemade_filter.cardResponsed["@jijiang-slash"] = function(player, promptlist)
	if promptlist[#promptlist] ~= "_nil_" then
		sgs.updateIntention(player, sgs.jijiangsource, -40)
		sgs.jijiangsource = nil
	end
end

sgs.ai_skill_cardask["@jijiang-slash"] = function(self)
	if not self:isFriend(sgs.jijiangsource) then return "." end
	if self:needBear() then return "." end
	return self:getCardId("Slash") or "."
end

sgs.ai_chaofeng.liubei = -2

sgs.ai_view_as.wusheng = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:isRed() and not card:inherits("Peach") then
		return ("slash:wusheng[%s:%s]=%d"):format(suit, number, card_id)
	end
end

local wusheng_skill={}
wusheng_skill.name="wusheng"
table.insert(sgs.ai_skills,wusheng_skill)
wusheng_skill.getTurnUseCard=function(self,inclusive)
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	
	local red_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards) do
		if card:isRed() and not card:inherits("Slash") and not card:inherits("Peach") 				--not peach
			and ((self:getUseValue(card)<sgs.ai_use_value.Slash) or inclusive) then
			red_card = card
			break
		end
	end

	if red_card then		
		local suit = red_card:getSuitString()
		local number = red_card:getNumberString()
		local card_id = red_card:getEffectiveId()
		local card_str = ("slash:wusheng[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
		
		assert(slash)
		
		return slash
	end
end

function sgs.ai_cardneed.paoxiao(to, card)
	if not to:containsTrick("indulgence") then
		return card:inherits("Slash")
	end
end

sgs.zhangfei_keep_value = 
{
	Peach = 6,
	Analeptic = 5.8,
	Jink = 5.7,
	FireSlash = 5.6,
	Slash = 5.4,
	ThunderSlash = 5.5,
	ExNihilo = 4.7
}

sgs.ai_chaofeng.zhangfei = 3

dofile "lua/ai/guanxing-ai.lua"

local longdan_skill={}
longdan_skill.name="longdan"
table.insert(sgs.ai_skills,longdan_skill)
longdan_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)
	
	local jink_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:inherits("Jink") then
			jink_card = card
			break
		end
	end
	
	if not jink_card then return nil end
	local suit = jink_card:getSuitString()
	local number = jink_card:getNumberString()
	local card_id = jink_card:getEffectiveId()
	local card_str = ("slash:longdan[%s:%s]=%d"):format(suit, number, card_id)
	local slash = sgs.Card_Parse(card_str)
	assert(slash)
	
	return slash
		
end

sgs.ai_view_as.longdan = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place ~= sgs.Player_Equip then
		if card:inherits("Jink") then
			return ("slash:longdan[%s:%s]=%d"):format(suit, number, card_id)
		elseif card:inherits("Slash") then
			return ("jink:longdan[%s:%s]=%d"):format(suit, number, card_id)
		end
	end
end

sgs.zhaoyun_keep_value = 
{
	Peach = 6,
	Analeptic = 5.8,
	Jink = 5.7,
	FireSlash = 5.7,
	Slash = 5.6,
	ThunderSlash = 5.5,
	ExNihilo = 4.7
}

sgs.ai_skill_invoke.tieji = function(self, data)
	local effect = data:toSlashEffect()
	local zj = self.room:findPlayerBySkillName("guidao")
	if self:hasEnemyZhangjiao(self.player) and self:canRetrial(zj) then
	    return false
	else
		return not self:isFriend(effect.to) 
	end 
	--return not self:isFriend(effect.to) and (not effect.to:isKongcheng() or effect.to:getArmor())
end

sgs.ai_chaofeng.machao = 1

function sgs.ai_cardneed.jizhi(to, card)
	if not to:containsTrick("indulgence") or card:inherits("Nullification") then
		return card:getTypeId() == sgs.Card_Trick
	end
end

sgs.huangyueying_keep_value = 
{
	Peach 		= 6,
	Analeptic 	= 5.9,
	Jink 		= 5.8,
	ExNihilo	= 5.7,
	Snatch 		= 5.7,
	Dismantlement = 5.6,
	IronChain 	= 5.5,
	SavageAssault=5.4,
	Duel 		= 5.3,
	ArcheryAttack = 5.2,
	AmazingGrace = 5.1,
	Collateral 	= 5,
	FireAttack	=4.9
}

sgs.ai_chaofeng.huangyueying = 4

local zhiheng_skill={}
zhiheng_skill.name="zhiheng"
table.insert(sgs.ai_skills, zhiheng_skill)
zhiheng_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("ZhihengCard") then 
		return sgs.Card_Parse("@ZhihengCard=.")
	end
end

sgs.ai_skill_use_func.ZhihengCard = function(card, use, self)
	local unpreferedCards={}
	local cards=sgs.QList2Table(self.player:getHandcards())
	
	if self.player:getHp() < 3 then
		local zcards = self.player:getCards("he")
		for _, zcard in sgs.qlist(zcards) do
			if not zcard:inherits("Peach") and not zcard:inherits("ExNihilo") then
				if self:getAllPeachNum()>0 or not zcard:inherits("Shit") then table.insert(unpreferedCards,zcard:getId()) end
			end	
		end
	end
	
	if #unpreferedCards == 0 then 
		if self:getCardsNum("Slash")>1 then 
			self:sortByKeepValue(cards)
			for _,card in ipairs(cards) do
				if card:inherits("Slash") then table.insert(unpreferedCards,card:getId()) end
			end
			table.remove(unpreferedCards,1)
		end
		
		local num=self:getCardsNum("Jink")-1							
		if self.player:getArmor() then num=num+1 end
		if num>0 then
			for _,card in ipairs(cards) do
				if card:inherits("Jink") and num>0 then 
					table.insert(unpreferedCards,card:getId())
					num=num-1
				end
			end
		end
		for _,card in ipairs(cards) do
			if (card:inherits("Weapon") and self.player:getHandcardNum() < 3) or card:inherits("OffensiveHorse") or
				self:getSameEquip(card, self.player) or	card:inherits("AmazingGrace") or card:inherits("Lightning") then
				table.insert(unpreferedCards,card:getId())
			end
		end
	
		if self.player:getWeapon() and self.player:getHandcardNum()<3 then
			table.insert(unpreferedCards, self.player:getWeapon():getId())
		end
				
		if (self:isEquip("SilverLion") and self.player:isWounded()) or self:isEquip("GaleShell") then
			table.insert(unpreferedCards, self.player:getArmor():getId())
		end	

		if self.player:getOffensiveHorse() and self.player:getWeapon() then
			table.insert(unpreferedCards, self.player:getOffensiveHorse():getId())
		end
	end	
	
	for index = #unpreferedCards, 1, -1 do
		if self.player:isJilei(sgs.Sanguosha:getCard(unpreferedCards[index])) then table.remove(unpreferedCards, index) end
	end
	
	if #unpreferedCards>0 then 
		use.card = sgs.Card_Parse("@ZhihengCard="..table.concat(unpreferedCards,"+")) 
		return 
	end
end

sgs.ai_use_value.ZhihengCard = 9

sgs.dynamic_value.benefit.ZhihengCard = true

local qixi_skill={}
qixi_skill.name="qixi"
table.insert(sgs.ai_skills,qixi_skill)
qixi_skill.getTurnUseCard=function(self,inclusive)
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	
	local black_card
	
	self:sortByUseValue(cards,true)
	
	local has_weapon=false
	
	for _,card in ipairs(cards)  do
		if card:inherits("Weapon") and card:isRed() then has_weapon=true end
	end
	
	for _,card in ipairs(cards)  do
		if card:isBlack()  and ((self:getUseValue(card)<sgs.ai_use_value.Dismantlement) or inclusive or self:getOverflow()>0) then
			local shouldUse=true

			if card:inherits("Armor") then
				if not self.player:getArmor() then shouldUse=false 
				elseif self:hasEquip(card) and not (card:inherits("SilverLion") and self.player:isWounded()) then shouldUse=false
				end
			end

			if card:inherits("Weapon") then
				if not self.player:getWeapon() then shouldUse=false
				elseif self:hasEquip(card) and not has_weapon and not card:inherits("YitianSword") then shouldUse=false
				end
			end
			
			if card:inherits("Slash") then
				local dummy_use = {isDummy = true}
				if self:getCardsNum("Slash") == 1 then
					self:useBasicCard(card, dummy_use)
					if dummy_use.card then shouldUse = false end
				end
			end

			if self:getUseValue(card) > sgs.ai_use_value.Dismantlement and card:inherits("TrickCard") then
				local dummy_use = {isDummy = true}
				self:useTrickCard(card, dummy_use)
				if dummy_use.card then shouldUse = false end
			end

			if shouldUse then
				black_card = card
				break
			end
			
		end
	end

	if black_card then
		local suit = black_card:getSuitString()
		local number = black_card:getNumberString()
		local card_id = black_card:getEffectiveId()
		local card_str = ("dismantlement:qixi[%s:%s]=%d"):format(suit, number, card_id)
		local dismantlement = sgs.Card_Parse(card_str)
		
		assert(dismantlement)

		return dismantlement
	end
end

sgs.ganning_suit_value = 
{
	spade = 3.9,
	club = 3.9
}

sgs.ai_chaofeng.ganning = 2

local kurou_skill={}
kurou_skill.name="kurou"
table.insert(sgs.ai_skills,kurou_skill)
kurou_skill.getTurnUseCard=function(self,inclusive)
	if  (self.player:getHp() > 3 and self.player:getHandcardNum() > self.player:getHp()) or		
		(self.player:getHp() - self.player:getHandcardNum() >= 2) then
		return sgs.Card_Parse("@KurouCard=.")
	end

	if self.player:getWeapon() and self.player:getWeapon():inherits("Crossbow") then
		for _, enemy in ipairs(self.enemies) do
			if self.player:canSlash(enemy,true) and self.player:getHp()>1 then
				return sgs.Card_Parse("@KurouCard=.")
			end
		end
	end
end

sgs.ai_skill_use_func.KurouCard=function(card,use,self)
	if not use.isDummy then self:speak("kurou") end
	use.card=card
end

sgs.ai_chaofeng.huanggai = 3

local fanjian_skill={}
fanjian_skill.name="fanjian"
table.insert(sgs.ai_skills,fanjian_skill)
fanjian_skill.getTurnUseCard=function(self)
	if self.player:isKongcheng() then return nil end
	if self.player:usedTimes("FanjianCard")>0 then return nil end

	local cards = self.player:getHandcards()

	for _, card in sgs.qlist(cards) do
		if card:getSuit() == sgs.Card_Diamond and self.player:getHandcardNum() == 1 then
			return nil
		elseif card:inherits("Peach") or card:inherits("Analeptic") then
			return nil
		end
	end

	local card_str = "@FanjianCard=."
	local fanjianCard = sgs.Card_Parse(card_str)
	assert(fanjianCard)

	return fanjianCard		
end

sgs.ai_skill_use_func.FanjianCard=function(card,use,self)
	self:sort(self.enemies, "hp")
			
	for _, enemy in ipairs(self.enemies) do								
		if (not enemy:hasSkill("qingnang")) or (enemy:getHp() == 1 and enemy:getHandcardNum() == 0 and not enemy:getEquips()) then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

sgs.ai_card_intention.FanjianCard = 70

function sgs.ai_skill_suit.fanjian()
	local map = {0, 0, 1, 2, 2, 3, 3, 3}
	return map[math.random(1,8)]
end

sgs.dynamic_value.damage_card.FanjianCard = true

sgs.ai_chaofeng.zhouyu = 3

local guose_skill={}
guose_skill.name="guose"
table.insert(sgs.ai_skills,guose_skill)
guose_skill.getTurnUseCard=function(self,inclusive)
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	
	local card
	
	self:sortByUseValue(cards,true)
	
	local has_weapon, has_armor = false, false
	
	for _,acard in ipairs(cards)  do
		if acard:inherits("Weapon") and not (acard:getSuit() == sgs.Card_Diamond) then has_weapon=true end
	end
	
	for _,acard in ipairs(cards)  do
		if acard:inherits("Armor") and not (acard:getSuit() == sgs.Card_Diamond) then has_armor=true end
	end
	
	for _,acard in ipairs(cards)  do
		if (acard:getSuit() == sgs.Card_Diamond) and ((self:getUseValue(acard)<sgs.ai_use_value.Indulgence) or inclusive) then
			local shouldUse=true
			
			if acard:inherits("Armor") then
				if not self.player:getArmor() then shouldUse=false 
				elseif self:hasEquip(acard) and not has_armor and self:evaluateArmor()>0 then shouldUse=false
				end
			end
			
			if acard:inherits("Weapon") then
				if not self.player:getWeapon() then shouldUse=false
				elseif self:hasEquip(acard) and not has_weapon then shouldUse=false
				end
			end
			
			if shouldUse then
				card = acard
				break
			end
		end
	end
	
	if not card then return nil end
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("indulgence:guose[diamond:%s]=%d"):format(number, card_id)	
	local indulgence = sgs.Card_Parse(card_str)
	assert(indulgence)
	return indulgence	
end

sgs.ai_skill_use["@@liuli"] = function(self, prompt)

	local others=self.room:getOtherPlayers(self.player)
	others=sgs.QList2Table(others)
	local source
	for _, player in ipairs(others) do
		if player:hasFlag("slash_source") then
			source = player
			break
		end
	end
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy,true) and not (source:objectName() == enemy:objectName()) then
			local cards = self.player:getCards("he")
			cards=sgs.QList2Table(cards)
			for _,card in ipairs(cards) do
				if (self.player:getWeapon() and card:getId() == self.player:getWeapon():getId()) and self.player:distanceTo(enemy)>1 then
				elseif card:inherits("OffensiveHorse") and self.player:getAttackRange()==self.player:distanceTo(enemy)
					and self.player:distanceTo(enemy)>1 then
				else
					return "@LiuliCard="..card:getEffectiveId().."->"..enemy:objectName()
				end
			end
		end
	end
	if self:isWeak() then
		for _, friend in ipairs(self.friends_noself) do
		if not self:isWeak(friend) then
			if self.player:canSlash(friend,true) and not (source:objectName() == friend:objectName()) then
					local cards = self.player:getCards("he")
					cards=sgs.QList2Table(cards)
					for _,card in ipairs(cards) do
						if (self.player:getWeapon() and card:getId() == self.player:getWeapon():getId()) and self.player:distanceTo(friend)>1 then
						elseif card:inherits("OffensiveHorse") and self.player:getAttackRange()==self.player:distanceTo(friend)
							and self.player:distanceTo(friend)>1 then
						else
							return "@LiuliCard="..card:getEffectiveId().."->".. friend:objectName()
						end
					end
				end
			end
		end
	end
	return "."
end

sgs.ai_card_intention.LiuliCard = function(card,from,to)
	sgs.ai_liuli_effect=true
end

function sgs.ai_slash_prohibit.liuli(self, to, card)
	if self:isFriend(to) then return false end
	for _, friend in ipairs(self.friends_noself) do
		if to:canSlash(friend,true) and self:slashIsEffective(card, friend) then return true end
	end
end

sgs.daqiao_suit_value = 
{
	diamond = 3.9
}

sgs.ai_chaofeng.daqiao = 2

sgs.ai_chaofeng.luxun = -1

local jieyin_skill={}
jieyin_skill.name="jieyin"
table.insert(sgs.ai_skills,jieyin_skill)
jieyin_skill.getTurnUseCard=function(self)
	if self.player:getHandcardNum()<2 then return nil end
	if self.player:hasUsed("JieyinCard") then return nil end
	
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	
	local first, second
	self:sortByUseValue(cards,true)
	for _, card in ipairs(cards) do
		if card:getTypeId() ~= sgs.Card_Equip and not (card:inherits("Shit") and self:isWeak() and self:getAllPeachNum()==0) then
			if not first then first  = cards[1]:getEffectiveId()
			else second = cards[2]:getEffectiveId()
			end
		end
		if second then break end
	end
	
	if not second then return end
	local card_str = ("@JieyinCard=%d+%d"):format(first, second)
	assert(card_str)
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.JieyinCard=function(card,use,self)
	self:sort(self.friends_noself, "hp")
	
	for _, friend in ipairs(self.friends_noself) do
		if friend:getGeneral():isMale() and friend:isWounded() then
			use.card=card
			if use.to then use.to:append(friend) end
			return
		end
	end
	if self.player:getGeneral():isMale() and self.player:isWounded() then
		use.card = card
		if use.to then use.to:append(self.player) end
		return
	end
end

sgs.ai_use_priority.JieyinCard = 2.5
sgs.ai_card_intention.JieyinCard = -80

sgs.dynamic_value.benefit.JieyinCard = true

sgs.sunshangxiang_keep_value = 
{
	Peach = 6,
	Jink = 5.1,
	Crossbow = 5,
	Blade = 5,
	Spear = 5,
	DoubleSword =5,
	QinggangSword=5,
	Axe=5,
	KylinBow=5,
	Halberd=5,
	IceSword=5,
	Fan=5,
	MoonSpear=5,
	GudingBlade=5,
	DefensiveHorse = 5,
	OffensiveHorse = 5
}

sgs.ai_chaofeng.sunshangxiang = 6

local qingnang_skill={}
qingnang_skill.name="qingnang"
table.insert(sgs.ai_skills,qingnang_skill)
qingnang_skill.getTurnUseCard=function(self)
	if self.player:getHandcardNum()<1 then return nil end
	if self.player:usedTimes("QingnangCard")>0 then return nil end
	
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	
	self:sortByKeepValue(cards)

	local card_str = ("@QingnangCard=%d"):format(cards[1]:getId())
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.QingnangCard=function(card,use,self)
	self:sort(self.friends, "defense")
	
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() and
			not (friend:hasSkill("longhun") and self:getAllPeachNum() > 0) and
			not (friend:hasSkill("hunzi") and friend:getMark("hunzi") == 0 and self:getAllPeachNum() > 1) then
			use.card=card
			if use.to then use.to:append(friend) end
			return
		end
	end
end

sgs.ai_use_priority.QingnangCard = 4.2
sgs.ai_card_intention.QingnangCard = -100

sgs.dynamic_value.benefit.QingnangCard = true

sgs.ai_view_as.jijiu = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:isRed() and player:getPhase()==sgs.Player_NotActive then
		return ("peach:jijiu[%s:%s]=%d"):format(suit, number, card_id)
	end
end

sgs.huatuo_suit_value = 
{
	heart = 6,
	diamond = 6
}

sgs.ai_chaofeng.huatuo = 6

sgs.ai_skill_cardask["@wushuang-slash-1"] = function(self, data, pattern, target)
	if sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) then return "." end
	if self:getCardsNum("Slash") < 2 and not (self.player:getHandcardNum() == 1 and self:hasSkills(sgs.need_kongcheng)) then return "." end
end

sgs.ai_skill_cardask["@wushuang-jink-1"] = function(self, data, pattern, target)
	if sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) then return "." end
	if self:getCardsNum("Jink") < 2 and not (self.player:getHandcardNum() == 1 and self:hasSkills(sgs.need_kongcheng)) then return "." end	
end

sgs.ai_chaofeng.lubu = 1

local lijian_skill={}
lijian_skill.name="lijian"
table.insert(sgs.ai_skills,lijian_skill)
lijian_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("LijianCard") then
		return 
	end
	if not self.player:isNude() then
		local card
		local card_id
		if self:isEquip("SilverLion") and self.player:isWounded() then
			card = sgs.Card_Parse("@LijianCard=" .. self.player:getArmor():getId())
		elseif self.player:getHandcardNum() > self.player:getHp() then
			local cards = self.player:getHandcards()
			cards=sgs.QList2Table(cards)
			
			for _, acard in ipairs(cards) do
				if (acard:inherits("BasicCard") or acard:inherits("EquipCard") or acard:inherits("AmazingGrace"))
					and not acard:inherits("Peach") and not acard:inherits("Shit") then 
					card_id = acard:getEffectiveId()
					break
				end
			end
		elseif not self.player:getEquips():isEmpty() then
			local player=self.player
			if player:getWeapon() then card_id=player:getWeapon():getId()
			elseif player:getOffensiveHorse() then card_id=player:getOffensiveHorse():getId()
			elseif player:getDefensiveHorse() then card_id=player:getDefensiveHorse():getId()
			elseif player:getArmor() and player:getHandcardNum()<=1 then card_id=player:getArmor():getId()
			end
		end
		if not card_id then
			cards=sgs.QList2Table(self.player:getHandcards())
			for _, acard in ipairs(cards) do
				if (acard:inherits("BasicCard") or acard:inherits("EquipCard") or acard:inherits("AmazingGrace"))
					and not acard:inherits("Peach") and not acard:inherits("Shit") then 
					card_id = acard:getEffectiveId()
					break
				end
			end
		end
		if not card_id then
			return nil
		else
			card = sgs.Card_Parse("@LijianCard=" .. card_id)
			return card
		end
	end
	return nil
end

sgs.ai_skill_use_func.LijianCard=function(card,use,self)
	local findFriend_maxSlash=function(self,first)
		self:log("Looking for the friend!")
		local maxSlash = 0
		local friend_maxSlash
		for _, friend in ipairs(self.friends_noself) do
			if (self:getCardsNum("Slash", friend)> maxSlash) and friend:getGeneral():isMale() then
				maxSlash=self:getCardsNum("Slash", friend)
				friend_maxSlash = friend
			end
		end
		if friend_maxSlash then
			local safe = false
			if (first:hasSkill("ganglie") or first:hasSkill("fankui") or first:hasSkill("enyuan")) then
				if (first:getHp()<=1 and first:getHandcardNum()==0) then safe=true end
			elseif (self:getCardsNum("Slash", friend_maxSlash) >= self:getCardsNum("Slash", first)) then safe=true end
			if safe then return friend_maxSlash end
		else self:log("unfound")
		end
		return nil
	end

	if not self.player:hasUsed("LijianCard") then
		self:sort(self.enemies, "hp")
		local males = {}
		local first, second
		local zhugeliang_kongcheng
		local duel = sgs.Sanguosha:cloneCard("duel", sgs.Card_NoSuit, 0)
		for _, enemy in ipairs(self.enemies) do
			--if zhugeliang_kongcheng and #males==1 and self:damageIsEffective(zhugeliang_kongcheng, sgs.DamageStruct_Normal, males[1]) 
				--then table.insert(males, zhugeliang_kongcheng) end
			if enemy:getGeneral():isMale() and not enemy:hasSkill("wuyan") then
				if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then	zhugeliang_kongcheng=enemy
				else
					if #males == 0 and self:hasTrickEffective(duel, enemy) then table.insert(males, enemy)
					elseif #males == 1 and self:damageIsEffective(enemy, sgs.DamageStruct_Normal, males[1]) then table.insert(males, enemy) end
				end
				if #males >= 2 then	break end
			end
		end
		if (#males==1) and #self.friends_noself>0 then
			self:log("Only 1")
			first = males[1]
			if zhugeliang_kongcheng and self:damageIsEffective(zhugeliang_kongcheng, sgs.DamageStruct_Normal, males[1]) then
				table.insert(males, zhugeliang_kongcheng)
			else
				local friend_maxSlash = findFriend_maxSlash(self,first)
				if friend_maxSlash and self:damageIsEffective(males[1], sgs.DamageStruct_Normal, enemy) then table.insert(males, friend_maxSlash) end
			end
		end
		if (#males >= 2) then
			first = males[1]
			second = males[2]
			local lord = self.room:getLord()
			if (first:getHp()<=1) then
				if self.player:isLord() or sgs.isRolePredictable() then 
					local friend_maxSlash = findFriend_maxSlash(self,first)
					if friend_maxSlash then second=friend_maxSlash end
				elseif (lord:getGeneral():isMale()) and (not lord:hasSkill("wuyan")) then 
					if (self.role=="rebel") and (not first:isLord()) and self:damageIsEffective(lord, sgs.DamageStruct_Normal, first) then
						second = lord
					else
						if ((self.role=="loyalist" or (self.role=="renegade") and not (first:hasSkill("ganglie") and first:hasSkill("enyuan"))))
							and	( self:getCardsNum("Slash", first)<=self:getCardsNum("Slash", second)) then
							second = lord
						end
					end
				end
			end

			if first and second then
				use.card = card
				if use.to then 
					use.to:append(first)
					use.to:append(second)
				end
			end
		end
	end
end

sgs.ai_use_value.LijianCard = 8.5
sgs.ai_use_priority.LijianCard = 4

lijian_filter = function(player, carduse)
	if carduse.card:inherits("LijianCard") then
		sgs.ai_lijian_effect = true
	end
end

table.insert(sgs.ai_choicemade_filter.cardUsed, lijian_filter)

sgs.ai_card_intention.LijianCard = function(card, from, to)
	if sgs.evaluateRoleTrends(to[1]) == sgs.evaluateRoleTrends(to[2]) then
		sgs.updateIntentions(from, to, 40)
	end
end

sgs.dynamic_value.damage_card.LijianCard = true

sgs.ai_chaofeng.diaochan = 4
