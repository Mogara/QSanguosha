
-- jianxiong
sgs.ai_skill_invoke.jianxiong = function(self, data)
        return not sgs.Shit_HasShit(data:toCard())
end

sgs.ai_skill_invoke.jijiang = function(self, data)
        return self:getSlashNumber(self.player)<=0
end

sgs.ai_skill_choice.jijiang = function(self , choices)
    if self:isFriend(self.room:getLord()) then return "accept" end
    return "ignore"
end

sgs.ai_skill_choice.hujia = function(self , choices)
    if self:isFriend(self.room:getLord()) then return "accept" end
    return "ignore"
end

-- hujia
sgs.ai_skill_invoke.hujia = function(self, data)
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("Jink") then
			return false
		end
	end
	return true	
end

-- tuxi
sgs.ai_skill_use["@@tuxi"] = function(self, prompt)
	self:sort(self.enemies, "handcard")
	
	local first_index
	for i=1, #self.enemies-1 do																			------修改突袭
		if (self.enemies[i]:objectName() == "zhugeliang" and self.enemies[i]:getHandcardNum() == 1) or
		   (self.enemies[i]:objectName() == "luxun" and self.enemies[i]:getHandcardNum() == 1) then 
			first_index = nil
				
		elseif not self.enemies[i]:isKongcheng() then
			first_index = i
			break
		end
	end
	
	if not first_index then
		return "."
	end
	
	local first = self.enemies[first_index]:objectName()
	local second = self.enemies[first_index + 1]:objectName()
        --self:updateRoyalty(-0.8*sgs.ai_royalty[first],self.player:objectName())
        --self:updateRoyalty(-0.8*sgs.ai_royalty[second],self.player:objectName())
	return ("@TuxiCard=.->%s+%s"):format(first, second)
end

-- yiji (frequent)

-- tiandu, same as jianxiong
sgs.ai_skill_invoke.tiandu = sgs.ai_skill_invoke.jianxiong

-- ganglie
sgs.ai_skill_invoke.ganglie = function(self, data)
    local invoke=not self:isFriend(data:toPlayer())
    if invoke then
        --self:updateRoyalty(-0.8*sgs.ai_royalty[data:toPlayer():objectName()],self.player:objectName())
    end
    return invoke
end

-- fankui 
sgs.ai_skill_invoke.fankui = function(self, data) 
	local target = data:toPlayer()
	if self:isFriend(target) then
		return target:hasSkill("xiaoji") and not target:getEquips():isEmpty()	
	end
	if self:isEnemy(target) then				---不反馈1牌的诸葛和陆逊
			if (target:hasSkill("kongcheng") or target:hasSkill("lianying")) and target:getHandcardNum() == 1 then
				if not target:getEquips():isEmpty() then return true
				else return false 
				end
			end
	end
                --self:updateRoyalty(-0.8*sgs.ai_royalty[target:objectName()],self.player:objectName())
	return true
end

local zhenji_ai = SmartAI:newSubclass "zhenji"

function zhenji_ai:askForCard(pattern,prompt)
    local card = super.askForCard(self, pattern, prompt)	
    if card then return card end
	if pattern == "jink" then
		local card = super.askForCard(self, pattern , prompt)
		if card then return card end
		local cards = self.player:getHandcards()		
		for _, card in sgs.qlist(cards) do			
			if card:isBlack() then
				local suit = card:getSuitString()
				local number = card:getNumberString()
				local card_id = card:getEffectiveId()
				return ("jink:qingguo[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	end
	
	return nil
end

local guanyu_ai = SmartAI:newSubclass "guanyu"

function guanyu_ai:askForCard(pattern,prompt)
	local card = super.askForCard(self, pattern, prompt)
	if card then return card end
	if pattern == "slash" then
		local cards = self.player:getCards("he")
		cards=sgs.QList2Table(cards)
        self:sortByUseValue(cards,true)
		for _, card in ipairs(cards) do
			if card:isRed() then
			    if self:getUseValue(card)>9 or card:inherits("Peach") then return nil end			--桃不当杀
				local suit = card:getSuitString()
				local number = card:getNumberString()
				local card_id = card:getEffectiveId()
				return ("slash:wusheng[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	end
    
end

local zhaoyun_ai = SmartAI:newSubclass "zhaoyun"

function zhaoyun_ai:askForCard(pattern,prompt)
	if pattern == "jink" then
		local cards = self.player:getHandcards()		
		for _, card in sgs.qlist(cards) do			
			if card:inherits("Slash") then
				local suit = card:getSuitString()
				local number = card:getNumberString()
				local card_id = card:getEffectiveId()
				return ("jink:longdan[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	elseif pattern == "slash" then
		local cards = self.player:getHandcards()		
		for _, card in sgs.qlist(cards) do
			if card:inherits("Jink") then
				local suit = card:getSuitString()
				local number = card:getNumberString()
				local card_id = card:getEffectiveId()
				return ("slash:longdan[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	end
	
	return super.askForCard(self, pattern , prompt)	
end

-- tieji
sgs.ai_skill_invoke.tieji = function(self, data) 
	local effect = data:toSlashEffect()
	return not self:isFriend(effect.to) 
end

local zhouyu_ai = SmartAI:newSubclass "zhouyu"
zhouyu_ai:setOnceSkill "fanjian"

function zhouyu_ai:activate(use)
	super.activate(self, use)

	if not use:isValid() and not self.fanjian_used and not self.player:isKongcheng() and next(self.enemies) then
		local cards = self.player:getHandcards()
		local should_fanjian = true
		for _, card in sgs.qlist(cards) do
		--	if card:getSuit() == sgs.Card_Diamond or card:inherits("Peach") or card:inherits("Analeptic") then		--反间,增大发动几率
			if card:getSuit() == sgs.Card_Diamond and self.player:getHandcardNum() == 1 then
				should_fanjian = false
			elseif card:inherits("Peach") or card:inherits("Analeptic") then
				should_fanjian = false
			end
		end

		if should_fanjian then
		--	self:sort(self.enemies)
			self:sort(self.enemies, "hp")
			
			use.card = sgs.Card_Parse("@FanjianCard=.")
			for _, enemy in ipairs(self.enemies) do								------修改反间不对华佗使用
				if not enemy:hasSkill("qingnang") or (enemy:getHp() == 1 and enemy:getHandcardNum() == 0 and not enemy:getEquips()) then
					use.to:append(enemy)
					self.fanjian_used = true
					return
				end
			end

			return
		end
	end
end

local sunshangxiang_ai = SmartAI:newSubclass "sunshangxiang"
sunshangxiang_ai:setOnceSkill("jieyin")

function sunshangxiang_ai:activate(use)
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("EquipCard") then
			use.card = card
			return
		end
	end

	self:sort(self.friends, "hp")
	local target
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() and friend:isWounded() then
			target = friend
			break
		end
	end

	if not self.jieyin_used and target and self.player:getHandcardNum()>=2 then
		local cards = self.player:getHandcards()
		
		local first = cards:at(0):getEffectiveId()
		local second = cards:at(1):getEffectiveId()

		local card_str = ("@JieyinCard=%d+%d"):format(first, second)
		use.card = sgs.Card_Parse(card_str)
		use.to:append(target)

		self.jieyin_used = true

		return
	end

	super.activate(self, use)
end

local ganning_ai = SmartAI:newSubclass "ganning"

function ganning_ai:activate_dummy(use)
	local cards = self.player:getCards("he")	
	if self.player:getHandcardNum()<3 then 
		super.activate(self, use)	
		return
	end
	
	local black_card
	for _, card in sgs.qlist(cards) do
		if card:isBlack() then			
			black_card = card
			break
		end
	end

	if black_card then		
		local suit = black_card:getSuitString()
		local number = black_card:getNumberString()
		local card_id = black_card:getEffectiveId()
		local card_str = ("dismantlement:qixi[%s:%s]=%d"):format(suit, number, card_id)
		local dismantlement = sgs.Card_Parse(card_str)
		
		assert(dismantlement)

		self:useCardDismantlement(dismantlement, use)
		if use:isValid() then
			return
		end
	end

	super.activate(self, use)	
end

local huanggai_ai = SmartAI:newSubclass "huanggai"

function huanggai_ai:activate(use)
    if  (self.player:getHp() > 3 and self.player:getHandcardNum() > self.player:getHp()) or		--血量大于3且手牌大于血量
		(self.player:getHp() - self.player:getHandcardNum() >= 2) then
                use.card = sgs.Card_Parse("@KurouCard=.")
                return
        end


    super.activate(self, use)

    if use:isValid() then return end
    if self.player:getWeapon() and self.player:getWeapon():inherits("Crossbow") then
        for _, enemy in ipairs(self.enemies) do
            if self.player:canSlash(enemy,true) and self.player:getHp()>1 then
                use.card = sgs.Card_Parse("@KurouCard=.")
            return
            end
        end
    end
    super.activate(self, use)
end

local daqiao_ai = SmartAI:newSubclass "daqiao"

sgs.ai_skill_use["@@liuli"] = function(self, prompt)
	
	local others=self.room:getOtherPlayers(self.player)												-----source没有用?
	others=sgs.QList2Table(others)
	local source
	for _, enemy in ipairs(others) do 
		if enemy:objectName()==prompt then 
			 source=enemy
			 break
		end
	end
	
	

	for _, enemy in ipairs(self.enemies) do

		if self.player:canSlash(enemy,true) and not (prompt==("@liuli-card:"..enemy:getGeneralName())) then			----已经派出了source

                        local cards = self.player:getCards("he")
                        cards=sgs.QList2Table(cards)
                        for _,card in ipairs(cards) do
                            if card:inherits("Weapon") and self.player:distanceTo(enemy)>1 then local bullshit
                            elseif card:inherits("OffensiveHorse") and self.player:getAttackRange()==self.player:distanceTo(enemy)
                                and self.player:distanceTo(enemy)>1 then
                                local bullshit
                            else
                                --self:updateRoyalty(-0.8*sgs.ai_royalty[enemy:objectName()],self.player:objectName())
                                return "@LiuliCard="..card:getEffectiveId().."->"..enemy:objectName()
                            end
                        end
		end
	end
	return "."
end

function daqiao_ai:activate_dummy(use)
	super.activate(self, use)
	if use:isValid() then
		return
	end

	local cards = self.player:getCards("he")
	for _, card in sgs.qlist(cards) do
		if card:getSuit() == sgs.Card_Diamond then
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local card_str = ("indulgence:guose[diamond:%s]=%d"):format(number, card_id)
			
			local indulgence = sgs.Card_Parse(card_str)
			
			self:useCardIndulgence(indulgence, use)
			
			if use:isValid() then
				return
			end			
		end
	end


end

local huatuo_ai = SmartAI:newSubclass "huatuo"
huatuo_ai:setOnceSkill("qingnang")

local black_before_red = function(a, b)
	local color1 = a:isBlack() and 0 or 1
	local color2 = b:isBlack() and 0 or 1

	if color1 ~= color2 then
		return color1 < color2
	else
		return a:getNumber() < b:getNumber()
	end
end

function huatuo_ai:activate(use)

		super.activate(self, use)
		if use:isValid() then
			return
		end
		
		if not self.qingnang_used and not self.player:isKongcheng() then
			self:sort(self.friends, "hp")
			local most_misery = self.friends[1]

			if most_misery:isWounded() then
				local cards = self.player:getHandcards()
				cards = sgs.QList2Table(cards)
				table.sort(cards, black_before_red)
			--self:sortByUsePriority(cards)							--优先级
				local card_id = cards[1]:getEffectiveId()						--试改

				use.card = sgs.Card_Parse("@QingnangCard=" .. card_id)
				use.to:append(most_misery)
				self.qingnang_used = true

				return
			end
		end
end


local liubei_ai=SmartAI:newSubclass "liubei"
liubei_ai:setOnceSkill("rende")
--liubei_ai:setOnceSkill("rendesecond")

function liubei_ai:activate(use)
	
	
    if ((self.player:getHandcardNum()+2>=self.player:getHp()) or self.player:isWounded()) and not self.rendesecond_used then  --小修改
		if self.player:getHandcardNum()==0 then return end
		self:sort(self.friends_noself,"defense")
		for _, friend in ipairs(self.friends_noself) do
			if (friend:getHandcardNum()<2) or (friend:getHandcardNum()<friend:getHp()+1) or self.player:isWounded() then
				--local card_id = self:getCardRandomly(self.player, "h")
				
				local card_id = self:getCardRandomly(self.player, "h")
				use.card = sgs.Card_Parse("@RendeCard=" .. card_id)
				use.to:append(friend)
                                if self.rende_used then self.rendesecond_used=true break end			---..小错误
			 	self.rende_used=true
				return
			end
		end
	end
	
	if (not use:isValid()) and (self.player:getHandcardNum()>=self.player:getHp())then 
		for _, friend in ipairs(self.friends_noself) do
		    local card_id = self:getCardRandomly(self.player, "h")
		    use.card = sgs.Card_Parse("@RendeCard=" .. card_id)
            use.to:append(friend)
            return
        end
    end
	super.activate(self, use)
end

local sunquan_ai = SmartAI:newSubclass "sunquan"
sunquan_ai:setOnceSkill("zhiheng")

function sunquan_ai:activate(use)

	local unpreferedCards={}
	local cards=sgs.QList2Table(self.player:getHandcards())
	
	if not self.zhiheng_used then 
		if self.player:getHp() < 3 then
			local zcards = self.player:getCards("he")
			for _, zcard in sgs.qlist(zcards) do
				if not zcard:inherits("Peach") and not zcard:inherits("ExNihilo") then
					table.insert(unpreferedCards,zcard:getId())
					self.zhiheng_used = true
				end	
			end
		end
	end	
	
	if not self.zhiheng_used then 
		if self:getSlashNumber(self.player)>1 then 
			self:sortByKeepValue(cards)
			for _,card in ipairs(cards) do
				if card:inherits("Slash") then table.insert(unpreferedCards,card:getId()) end
			end
			table.remove(unpreferedCards,1)
		end
		
		local num=self:getJinkNumber(self.player)-1							---留一闪
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
                    if card:inherits("EquipCard") then
                        if card:inherits("Weapon") or--and self.player:getWeapon()) or
                        (card:inherits("DefensiveHorse") and self.player:getDefensiveHorse()) or
                        card:inherits("OffensiveHorse") or--and self.player:getOffensiveHorse()) or
                        (card:inherits("Armor") and self.player:getArmor()) or
                         card:inherits("AmazingGrace") or
                         card:inherits("Lightning") then
                            table.insert(unpreferedCards,card:getId())
                        end
                    end
                end
	
				if self.player:getWeapon() then														--不留武器和-1马
					table.insert(unpreferedCards, self.player:getWeapon():getId())
				end
				
				if self.player:getArmor() and self.player:getArmor():objectName() == "silver_lion" and self.player:isWounded() then
					table.insert(unpreferedCards, self.player:getArmor():getId())
				end	
				
				local equips=self.player:getEquips()
				for _,equip in sgs.qlist(equips) do
					if equip:inherits("OffensiveHorse") and self.player:getWeapon() then
						table.insert(unpreferedCards, equip:getId())
						break
					end
				end	
	end	
	
	if #unpreferedCards>0 then 
		use.card = sgs.Card_Parse("@ZhihengCard="..table.concat(unpreferedCards,"+")) 
		self.zhiheng_used=true
		return 
	end
	super.activate(self,use)
end

sgs.ai_skill_invoke["luoyi"]=function(self,data)
    local cards=self.player:getHandcards()
    cards=sgs.QList2Table(cards)

    for _,card in ipairs(cards) do
        if card:inherits("Slash") then

            for _,enemy in ipairs(self.enemies) do
                if self.player:canSlash(enemy, true) and
                self:slashIsEffective(card, enemy) and
                ( (not enemy:getArmor()) or (enemy:getArmor():objectName()=="renwang_shield") or (enemy:getArmor():objectName()=="vine") ) and
                enemy:getHandcardNum()< 2 then							--修改
                    if not self.player:containsTrick("indulgence") then
                        return true
                    end
                end
            end
        end
    end
    return false
end

sgs.ai_skill_invoke["@guicai"]=function(self,prompt)
    local judge = self.player:getTag("Judge"):toJudge()
	
	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getHandcards())		
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return "@GuicaiCard=" .. card_id
		end
	end
	
	return "."
end
