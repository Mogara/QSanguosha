lijian_skill={}
lijian_skill.name="lijian"
table.insert(sgs.ai_skills,lijian_skill)
lijian_skill.getTurnUseCard=function(self)
	if self.lijian_used then
		return nil
	end
	if not self.player:isNude() then
		local card
		local card_id
		if self.player:getArmor() and self.player:isWounded() and self.player:getArmor():objectName()=="silver_lion" then
			card = sgs.Card_Parse("@LijianCard=" .. self.player:getArmor():getId())
		elseif self.player:getHandcardNum() > self.player:getHp() then
			local cards = self.player:getHandcards()
			cards=sgs.QList2Table(cards)
			
			-- 引用AI[0511]patch中的代码
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

sgs.ai_skill_use_func["LijianCard"]=function(card,use,self)
	local findFriend_maxSlash=function(self,first)
		self:log("Looking for the friend!")
		local maxSlash = 0
		local friend_maxSlash
		for _, friend in ipairs(self.friends_noself) do
			if (self:getSlashNumber(friend)> maxSlash) and friend:getGeneral():isMale() then
				maxSlash=self:getSlashNumber(friend)
				friend_maxSlash = friend
			end
		end
		if friend_maxSlash then
			self:log("found!")
			local safe = false
			if (first:hasSkill("ganglie") or first:hasSkill("fankui") or first:hasSkill("enyuan")) then
				if (first:getHp()<=1 and first:getHandcardNum()==0) then safe=true end
			elseif (self:getSlashNumber(friend_maxSlash) >= first:getHandcardNum()) then safe=true end
			if safe then return friend_maxSlash end
		else self:log("unfound")
		end
		return nil
	end

	if not self.lijian_used --and not self.player:isNude()
	then
		self:sort(self.enemies, "hp")
		local males = {}
		local first, second
		local zhugeliang_kongcheng
		for _, enemy in ipairs(self.enemies) do
			if zhugeliang_kongcheng and #males==1 then table.insert(males, zhugeliang_kongcheng) end
			if enemy:getGeneral():isMale() and not enemy:hasSkill("wuyan") then
				if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then	zhugeliang_kongcheng=enemy
				else table.insert(males, enemy)	end
				if #males >= 2 then	break end
			end
		end
		--if (#males==0) then self:log("No real men!") end
		if (#males==1) and #self.friends_noself>0 then
			self:log("Only 1")
			first = males[1]
			if zhugeliang_kongcheng then table.insert(males, zhugeliang_kongcheng)
			else
				local friend_maxSlash = findFriend_maxSlash(self,first)
				if friend_maxSlash then table.insert(males, friend_maxSlash) end
			end
		end
		if (#males >= 2) then
			first = males[1]
			second = males[2]
			local lord = self.room:getLord()
			if (first:getHp()<=1) then
				if self.player:isLord() then
					self:log("Diaochan is the lord!")
					local friend_maxSlash = findFriend_maxSlash(self,first)
					if friend_maxSlash then second=friend_maxSlash end
				end
				if (lord:getGeneral():isMale()) and (not lord:hasSkill("wuyan")) then
					if (self.role=="rebel") and (not first:isLord()) then
						second = lord
					else
						if ((self.role=="loyalist" and not (first:hasSkill("ganglie") or first:hasSkill("enyuan")))
							or (self.role=="renegade"))
							and	(first:getHandcardNum()<2 or self:getSlashNumber(first)<1) then
							--the first male maybe have a "Slash" Card
							second = lord
						end
					end
				end
			end

			if first and second 
					then
				if use.to --and not (self.player:getHandcardNum()<=1 and first:getHp()>1)
					then 
		        use.to:append(first)
		        use.to:append(second)
		        self.lijian_used = true
				end
			end
            use.card=card
		end
	end
end
