local pkg=sgs.ai_standard
--the strategy that considers the cost of every card play: losing the card you are playing
sgs.newStrategy{
	match=function(self,move)
		local hand=self.player:getHandcards()
		hand=sgs.QList2Table(hand)
		local to_disc=self:askForDiscard(nil,hand,#hand-self.player:getHp())
		for _,card in ipairs(to_disc) do
			if move.card:getId()==card:getId() then return false end
		end
		return true
	end,
	evaluate=function(self,move)
		if move.card:inherits("Slash") then
			if self:getSlashNumber(self.player)<=1 then return -3 
			elseif self:getSlashNumber(self.player)>=3 then return 0
			else return -2 end
		elseif move.card:inherits("Jink") then
			if self:getJinkNumber(self.player)<=1 then return -4
			else return -1.5 end
		elseif move.card:inherits("Analeptic") then
			if self.player:getHp()<3 then return -4 end
		elseif move.card:inherits("Peach") then 
			
			local peaches=0
			local cards = self.player:getHandcards()
			cards=sgs.QList2Table(cards)
			for _,card in ipairs(cards) do
				if card:inherits("Peach") then peaches=peaches+1 end
			end
			
			for _, friend in ipairs(self.friends_noself) do
				if (self.player:getHp()-friend:getHp()>peaches) and (friend:getHp()<3) then return -7 end
			end	
			
			return -3
			
		end
		return -1
	end,
}

sgs.newStrategy{
	match=function()
		return true
	end,
	value=0.1
}

--the yield for a slash
sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("Slash")
	end,
	evaluate=function(self,move)
		
		local onEachHit=function(player)
			if not self:slashIsEffective(move.card,player) then return 0 end		
			if self:isEnemy(player) then 
				return 20/player:getHp() 
				--20 for 1HP 10 for 2HP 6 for 3HP
			elseif self:isFriend(player) then 
				return -20/player:getHp()
			end
			return 5/player:getHp() 
		end
		
		local onMissChance=function(player)			
			if not self:isFriend(player) then 
				return 2/(player:getHandcardNum()+2)-1
				--0 for no hand -0.3 for 1hand -0.5 for 2hand
			end
			return 0
		end
		
		local v=0
		for _,aplayer in sgs.qlist(move.to) do
			v=v+onEachHit(aplayer)*(1+onMissChance(aplayer))
			if self.player:hasFlag("drank") then v=v*1.5 end
		end
		
		return v
	end,
}

--dismantlement
pkg.basic_dismantlement_choice=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("Dismantlement")
	end,
	evaluate=function(self,move)
		
		local target=move.to:first()
		
		if target:containsTrick("lightning") and 
			self:hasWizard(self.enemies) and not self.has_wizard then
			return 8 
		elseif self:isFriend(target) then
			if (target:containsTrick("indulgence") or target:containsTrick("supply_shortage")) then
			return 7
			elseif target:hasArmorEffect("silver_lion") and target:getLostHp()>1 then
			return 6 end
		else 
			if target:getHp()<3 and target:getArmor() then return 5
			elseif target:getHandcardNum()<3 then return 6-target:getHandcardNum()
			elseif target:getHandcardNum()<move.from:getHandcardNum() then return 3
			end
		end
		
		return 0
	end,
}

--snatch, more yield on getting the target card
pkg.basic_snatch_choice=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("Snatch")
	end,
	evaluate=function(self,move)
	
		local target=move.to:first()
		
		if target:containsTrick("lightning") and 
			self:hasWizard(self.enemies) and not self.has_wizard then
			return 8 
		elseif self:isFriend(target) then
			if (target:containsTrick("indulgence") or target:containsTrick("supply_shortage")) then
			return 8
			elseif target:hasArmorEffect("silver_lion") and target:getLostHp()>1 then
			return 7 end
		else 
			if not self.player:getArmor() and target:getArmor() then return 6
			elseif target:getHp()<3 and target:getArmor() then return 5
			elseif target:getHandcardNum()<3 then return 7-target:getHandcardNum()
			else return 3 end
		end
		
		return 0
	end,
}

--analeptic
pkg.basic_anal_choice=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("Analeptic")
	end,
	evaluate=function(self,move)
		local slash_yield=0
		for _,amove in ipairs(self.allMoves) do
			if amove.card:inherits("Slash") and self:choiceEvaluate(amove)>slash_yield and not amove.to:first():hasArmorEffect("silver_lion") then 
				slash_yield=self:choiceEvaluate(amove)
			end
		end
		if slash_yield>10 then return 5
		elseif self.player:getHp()<3 and self:getPeachNum(move.from) then return -2
		elseif slash_yield>2 then return 4
		elseif self.player:getHandcardNum()-2>self.player:getHp() and slash_yield>0 then return 3
		else return 0
		end
	end
}

--peaches
pkg.basic_peach_choice=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("Peach")
	end,
	evaluate=function(self,move)
		local peaches=self:getPeachNum(move.from)
			for _, friend in ipairs(self.friends_noself) do
				if (self.player:getHp()-friend:getHp()>peaches) and (friend:getHp()<3) then return 0 end
			end	
		return 5
	end,
}

--fire_attack
pkg.basic_fire_attack_choice=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("FireAttack")
	end,
	evaluate=function(self,move)
		local suits={}
		local cards=self.player:getHandcards()
		for _,card in sgs.qlist(cards) do 
			suits[card:getSuit()]=(suits[card:getSuit()] or 0)+1
		end
		suits[move.card:getSuit()]=suits[move.card:getSuit()]-1
		local var=0
		for i=0,3 do
			if suits[i] and suits[i]>0  then var=var+1 end
		end
		
		local target=move.to:first()
		if self:isEnemy(target) then
			local multiplier= target:hasArmorEffect("vine")
			if multiplier then multiplier=1.5 else multiplier=1 end
			return (var-2)*5/target:getHp()*multiplier
		else return -1 end
		
	end
}

--duel
pkg.basic_duel_choice=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("Duel")
	end,
	evaluate=function(self,move)
		local target=move.to:first()
		local chance=self:getSlashNumber(self.player)*3-target:getHandcardNum()
		local value= 10*chance/target:getHp()
		if self:isEnemy(target) then return value
		else 
			if value>0 then return -value 
			else return value end
		end
	end
}

--equip
pkg.basic_equip_choice=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("EquipCard")
	end,
	evaluate=function(self,move)
	
		if self.player:getHp()>=self.player:getHandcardNum() then
			if (move.card:inherits("Armor") and self.player:getArmor())  or
				(move.card:inherits("Weapon") and self.player:getWeapon()) or
				(move.card:inherits("DefensiveHorse") and self.player:getDefensiveHorse())  or
				(move.card:inherits("OffensiveHorse") and self.player:getOffensiveHorse())  then
					return 0
			elseif move.card:inherits("Armor") then return 3 
			elseif move.card:inherits("Weapon") then
				if self:getSlash() and self:getSlash():isAvailable(self.player) then return 3 end
			end
	
		elseif move.card:inherits("Weapon") and self.player:getWeapon() then
			return self:evaluateEquip(move.card)-self:evaluateEquip(self.player:getWeapon())
		elseif move.card:inherits("Armor") and self.player:getArmor() then
			if self.player:hasArmorEffect("silver_lion") and self.player:getLostHp()>0 then 
				return 3
			elseif self.player:isChained() and self.player:hasArmorEffect("vine") then 
				return 3
			elseif move.card:objectName()=="eight_diagram" then
				return 3
			end
		else 
			return 3
		end
			
		return 0
	end,
}

--SupplyShortage
pkg.basic_supply_shortage_choice=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("SupplyShortage")
	end,
	evaluate=function(self,move)
		local target=move.to:first()
		local v=6-target:getHandcardNum()
		if v<3 then v=12/(7-v) end
		if self:isEnemy(target) then return v
		elseif self:isFriend(target) then return -v
		else return 1 end
	end
}

--Indulgence
pkg.basic_indulgence_choice=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("Indulgence")
	end,
	evaluate=function(self,move)
		local target=move.to:first()
		local v= target:getHandcardNum()-target:getHp()
		if v<1 then v=6/(7-v) end
		if self:isEnemy(target) then return v*2+6
		else return -v*2-6 end
	end
}

--collateral
pkg.basic_collateral_choice=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("Collateral")
	end,
	evaluate=function(self,move)
		local target=move.to:first()
		local victim=move.to:at(1)
		
		local v=0
		if not self.player:getWeapon() then 
			if self:getSlash() and self:getSlash():isAvailable(self.player) then v=v+6 end
		end
		if self:isEnemy(target) then v=v+target:getHandcardNum() end
		if self:isFriend(victim) then v=v-target:getHandcardNum()*2 end
		
		return v
	end
}

--iron chain
pkg.basic_iron_chain_choice=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("IronChain")
	end,
	evaluate=function(self,move)
		local v=0
		if move.to:length()==0 then return 4 end
		for _,target in sgs.qlist(move.to) do
			local temp=0
			if target:hasArmorEffect("vine") then temp=temp+2 end
			temp=temp+2
			if target:getHp()<2 then temp=temp+2 end
			
			if self:isEnemy(target) and not target:isChained() then v=v+temp
			elseif target:isChained() and self:isFriend(target) then v=v+temp
			else v=v-temp
			end
		end
		
		for _,card in sgs.qlist(self.player:getHandcards()) do
			if card:inherits("FireAttack") or
				card:inherits("FireSlash") or
				card:inherits("ThunderSlash") then
				v=v+2
				break
			end
		end
		
		return v
	end
}

--GodSalvation
pkg.basic_god_salvationchoice=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("GodSalvation")
	end,
	evaluate=function(self,move)
		local critical=0
		local yield=0
		for _,player in sgs.qlist(self.room:getAlivePlayers()) do
			if player:getLostHp()>0 then 
				if player:getHp()<2 then 
					if self:isEnemy(player) then critical=critical-8 
					else critical=critical+8
					end
				end
				if self:isEnemy(player) then yield=yield-20/(player:getHp()+1)
				else yield=yield+20/(player:getHp()+1)
				end
			end
		end
		
		return (yield+critical)/2
	end
}

--ExNihilo
pkg.basic_ExNihilo_choice=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("ExNihilo")
	end,
	value=10,
}

--AOE
pkg.basic_aoe_choice=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("SavageAssault") or move.card:inherits("ArcheryAttack")
	end,
	evaluate=function(self,move)
		local critical=0
		local yield=0
		for _,player in sgs.qlist(self.room:getAlivePlayers()) do
			if self:aoeIsEffective(move.card,player) then
				if player:getHp()<2 then 
					if self:isEnemy(player) then critical=critical+8 
					else critical=critical-8
					end
				end
				if self:isEnemy(player) then yield=yield+20/(player:getHp()+1)
				else yield=yield-20/(player:getHp()+1)
				end
			end
		end
		
		return (yield+critical)/2
	end
}

sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("Dismantlement") 
		and (move.to:first():hasSkill("kongcheng") or move.to:first():hasSkill("kongcheng"))
		and self:getEquipNumber(move.to:first())+move.to:first():getHandcardNum()==1
	end,
	value=0,
	override={pkg.basic_dismantlement_choice}
}

--old skillcard_use function compatibility
pkg.use_skill_card_compatibility=sgs.newStrategy{
	match=function(self,move)
		return move.card:inherits("SkillCard")
	end,
	value=10,
	override={pkg},
}

--give shuangxiong duel a bolder strategy


