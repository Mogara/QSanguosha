-- pojun
sgs.ai_skill_invoke.pojun = function(self, data)
	local damage = data:toDamage()
	local good = damage.to:getHp() > 2
	
	
	if self:isFriend(damage.to) then
		return good
	elseif self:isEnemy(damage.to) then
		return not good
	end
end

--jiejiu
jiejiu_skill={}
jiejiu_skill.name="jiejiu"
table.insert(sgs.ai_skills,jiejiu_skill)
jiejiu_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("he")	
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
sgs.ai_skill_invoke.lingtong = function(self, data)
    local invoke = not self:isFriend(data:toPlayer())
    return invoke
end


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
		self:sortByUseValue(cards)
		
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


--ganlu
local wuguotai_ai = SmartAI:newSubclass "wuguotai"
wuguotai_ai:setOnceSkill("ganlu")

function wuguotai_ai:activate(use)
		super.activate(self, use)
	if use:isValid() then
		return
	end
	
	if not self.ganlu_used then
	
		local equips  = {}
		for _, friend in ipairs(self.friends) do
			if friend:hasSkill("xiaoji") and friend:getEquips() then
				for _, enemy in ipairs(self.enemies) do
						equips = enemy:getEquips()
						if equips then
							use.card=sgs.Card_Parse("@Ganlu=.")
							use.to:append(friend)
							use.to:append(enemy)
							self.ganlu_used = true
							return
						end	
				end	
			end	
		end
		
		for _, friend in ipairs(self.friends) do
			if friend:getEquips() then
				for _, enemy in ipairs(self.enemies) do
					if enemy:getEquips() and not enemy:hasSkill("xiaoji") then 
						use.card=sgs.Card_Parse("@Ganlu=.")
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


--jujian
local xushu_ai = SmartAI:newSubclass "xushu"

function xushu_ai:activate(use)
	super.activate(self, use)
	if use:isValid() then
		return
	end

	local abandon_handcard = {}
	local index = 0

--	if not self.jujian_used and self.player:isWounded() and self.player:getHandcardNum() > 2 then
	
--		local cards = self.player:getHandcards()
--		cards=sgs.QList2Table(cards)
--		local club, spade, diamond = true, true, true
--		self:sortByUseValue(cards, true)
--		for _, friend in ipairs(self.friends_noself) do
--			if (friend:getHandcardNum()<2) or (friend:getHandcardNum()<friend:getHp()+1) or self.player:isWounded() then
--				for _, card in ipairs(cards) do 
--					if card:getSuit() == sgs.Card_Club and club then 
--						table.insert(abandon_handcard, card:getEffectiveId())
--						index = index + 1
--						club = false
--					elseif card:getSuit() == sgs.Card_Spade and spade then
--						table.insert(abandon_handcard, card:getEffectiveId())
--						index = index + 1
--						spade = false
--					elseif card.getSuit() == sgs.Card_Diamond and not card:inherits("Peach") and diamond then
--						table.insert(abandon_handcard, card:getEffectiveId())
--						index = index + 1
--						diamond = false
--					end
--				end
--				if index > 0 then 
--					use.to:append(friend)
--					use.card = sgs.Card_Parse("@JujianCard=" .. table.concat(abandon_handcard, "+"))
--					self.jujian_used = true
--				end	
--			end
--		end
--	end
	
	
	if not self.player:hasUsed("JujianCard") then
		local cards = self.player:getHandcards()
		cards=sgs.QList2Table(cards)
		self:sortByUseValue(cards)
	--	local slash_num = self:getSlashNumber(self.player)
	--	local jink_num = self:getJinkNumber(self.player)
		for _, friend in ipairs(self.friends_noself) do
			if (friend:getHandcardNum()<2) or (friend:getHandcardNum()<friend:getHp()+1) or self.player:isWounded() then
				for _, card in ipairs(cards) do
					if not card:inherits("Nullification") and not card:inherits("EquipCard") and 
						not card:inherits("Peach") and not card:inherits("Jink") then
						table.insert(abandon_handcard, card:getEffectiveId())
						index = 5
		--			elseif card:inherits("Slash") or card:inherits("TunderSlash") or card:inherits("FireSlash") and slash_num > 1 then
		--				table.insert(abandon_handcard, card:getEffectiveId())
		--				index = 5
		--				slash_num = slash_num - 1
		--			elseif card:inherits("Jink") and jink_num > 1 then
		--				table.insert(abandon_handcard, card:getEffectiveId())
		--				index = 5
		--				jink_num = jink_num - 1
					end
				end	
				if index == 5 then 
					use.to:append(friend)
					use.card = sgs.Card_Parse("@JujianCard=" .. table.concat(abandon_handcard, "+"))
					return
				end
			end			
		end	
	
	end
end