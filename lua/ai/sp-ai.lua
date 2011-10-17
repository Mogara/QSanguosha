--sp pangde
local taichen_skill={}
taichen_skill.name="taichen"
table.insert(sgs.ai_skills,taichen_skill)
taichen_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("TaichenCard") then return end
	return sgs.Card_Parse("@TaichenCard=.")
end

sgs.ai_skill_use_func["TaichenCard"]=function(card,use,self)
	local target, card_str
	
	local targets, friends, enemies = {}, {}, {}
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if self.player:canSlash(player) then 
			table.insert(targets, player) 
			
			if self:isFriend(player) then
				table.insert(friends, player)
			else 
				table.insert(enemies, player)
			end
		end
	end
	
	if #targets == 0 then return end
	
	if #friends ~= 0 then
		for _, friend in ipairs(friends) do
			local judge_card = friend:getCards("j")
			local equip_card = friend:getCards("e")
		
			if judge_card and judge_card:length() > 0 and not (judge_card:length() == 1 and judge_card:at(0):objectName() == "lightning") then 
				target = friend 
				break 
			end
			if equip_card and equip_card:length() > 1 and self:hasSkills(sgs.lose_equip_skill, friend) then 
				target = friend 
				break 
			end
		end
	end
	
	if not target and #enemies > 0 then
		self:sort(enemies, "defense")
		for _, enemy in ipairs(enemies) do
			if enemy:getCards("he") and enemy:getCards("he"):length()>=2 then 
				target = enemy 
				break
			end
		end
	end
	
	if not target then return end
	
	local weapon = self.player:getWeapon()
	local hcards = self.player:getHandcards()
	for _, hcard in sgs.qlist(hcards) do
		if hcard:inherits("Weapon") then 
			if weapon then card_str = "@TaichenCard=" .. hcard:getId() end
		end
	end
	
	if not card_str then
		if weapon and self.player:getOffensiveHorse() then
			card_str = "@TaichenCard=" .. weapon:getId() 
		else
			if self:isFriend(target) and self.player:getHp() > 2 then card_str = "@TaichenCard=." end
			if self:isEnemy(target) and self.player:getHp() > 3 then card_str = "@TaichenCard=." end
		end
	end
	
	if card_str then
		if use.to then
			use.to:append(target)
		end
		use.card = sgs.Card_Parse(card_str)
	end
end

--shenlvbu2
sgs.ai_skill_invoke.xiuluo = function(self, data)
	local hand_card = self.player:getHandcards()
	local judge_list = self.player:getCards("j")
	for _, judge in sgs.qlist(judge_list) do
		for _, card in sgs.qlist(hand_card) do
			if card:getSuit() == judge:getSuit() then return true end
		end
	end
	
	return false
end

-- chujia
sgs.ai_skill_invoke.chujia = function(self, data)
	return self.room:getLord():getKingdom() == "shu"
end

-- guixiang
sgs.ai_skill_invoke.guixiang = function(self, data)
	return self.room:getLord():getKingdom() == "wei"
end

sgs.ai_skill_playerchosen.sp_moonspear=function(self,targets)
	local targetlist=sgs.QList2Table(targets)
	self:sort(targetlist,"defense")
	for _,enemy in ipairs(targetlist) do
		if self:isEnemy(enemy) then return enemy end
	end
end
