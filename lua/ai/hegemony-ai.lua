if sgs.GetConfig("EnableHegemony", false) then
	local init = SmartAI.initialize
	function SmartAI:initialize(player)
		if not sgs.initialized then
			for _, aplayer in sgs.qlist(player:getRoom():getAllPlayers()) do
				sgs.ai_explicit[aplayer:objectName()] = ""
			end
		end
		init(self, player)
	end
	sgs.ai_skill_choice.RevealGeneral = function(self, choices)
		local event = self.player:getTag("event"):toInt()
		local data = self.player:getTag("event_data")
		local generals = self.player:getTag("roles"):toString():split("+")
		local players = {}
		for _, general in ipairs(generals) do
			local player = sgs.ServerPlayer(self.room)
			player:setGeneral(sgs.Sanguosha:getGeneral(general))
			table.insert(players, player)
		end
		
		local anjiang = {}
		for _, player in sgs.qlist(self.room:getAllPlayers()) do
			if player:getGeneralName() == "anjiang" then table.insert(anjiang, player:getSeat()) end
		end

		if event == sgs.Predamaged then
			local damage = data:toDamage()
			for _, player in ipairs(players) do
				if self:hasSkills(sgs.masochism_skill, player) and self:isEnemy(damage.from) then return "yes" end
				if damage.damage > self.player:getHp() + self:getAllPeachNum() then return "yes" end
			end
		elseif event == sgs.CardEffected then
			local effect = data:toCardEffect()
			for _, player in ipairs(players) do
				if self.room:isProhibited(effect.from, player, effect.card) and self:isEnemy(effect.from) then return "yes" end
			end
		end
		
		if sgs.getValue(self.player) < 6 then return "no" end
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if self:isFriend(player) then return "yes" end
		end
		local vequips, defense = 0
		if self.player:getWeapon() or self:hasHegSkills("yitian", players) then vequips = vequips + 1 end
		if (self.player:getArmor() and self:evaluateArmor()>0) or self:hasHegSkills("bazhen|yizhong", players) then
			vequips = vequips + 2 defense = true end
		if self.player:getDefensiveHorse() or self:hasHegSkills("feiying", players) then vequips = vequips + 1.5 defense = true end
		if self.player:getOffensiveHorse() or self:hasHegSkills("mashu", players) then vequips = vequips + 0.5 end
		if vequips < 2.5 or not defense then return "no" end
		
		if sgs.ai_loyalty[self:getHegKingdom()][self.player:objectName()] == 160 then return "yes" end
		
		local anjiang = 0
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if player:getGeneralName() == "anjiang" then
				anjiang = anjiang + 1
			end
		end
		
		if math.random() > (anjiang + 1)/(self.room:alivePlayerCount() + 2) then
			return "yes"
		else
			return "no"
		end
	end
	
	sgs.isRolePredictable = function()
		return false
	end

	sgs.ai_loyalty = {
		wei = {},
		wu = {},
		shu = {},
		qun = {},
	}
	sgs.ai_explicit = {}
	
	SmartAI.hasHegSkills = function(self, skills, players)
		for _, player in ipairs(players) do
			if self:hasSkills(skills, player) then return true end
		end
		return false
	end
	
	SmartAI.getHegKingdom = function(self)
		local names = self.room:getTag(self.player:objectName()):toStringList()

		if #names == 0 then return self.player:getKingdom() end
		local kingdom = sgs.Sanguosha:getGeneral(names[1]):getKingdom()
		return kingdom
	end

	SmartAI.getHegGeneralName = function(self, player)
		player = player or self.player
		local names = self.room:getTag(player:objectName()):toStringList()
		if #names > 0 then return names[1] else return player:getGeneralName() end
	end
	
	SmartAI.objectiveLevel = function(self, player, recursive)
		if self.player:objectName() == player:objectName() then return -5 end
		local lieges = {}
		local liege_hp = 0
		for _, aplayer in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if self:getHegKingdom() == aplayer:getKingdom() then table.insert(lieges, aplayer) end
			liege_hp = liege_hp + aplayer:getHp()
		end

		local plieges = {}
		local modifier = 0
		for _, aplayer in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			local kingdom = aplayer:getKingdom()
			if kingdom == "god" then kingdom = sgs.ai_explicit[aplayer:objectName()] end
			if kingdom then plieges[kingdom] = (plieges[kingdom] or 0) + 1 end
		end
		local kingdoms = {"wei", "wu", "shu", "qun"}
		local max_kingdom = 0
		for _, akingdom in ipairs(kingdoms) do
			if (plieges[akingdom] or 0) > max_kingdom then max_kingdom = plieges[akingdom] end
		end

		if max_kingdom > 0 then
			local kingdom = player:getKingdom()
			if kingdom == "god" then kingdom = sgs.ai_explicit[player:objectName()] end
			if not kingdom or (plieges[kingdom] or 0) < max_kingdom then modifier = -2
			elseif (plieges[kingdom] or 0) > 2 then modifier = 2 end
		end

		if self:getHegKingdom() == player:getKingdom() then
			if recursive then return -3 end
			if self.player:getKingdom() == "god" and #lieges >= 2 then
				self:sort(lieges, "hp")
				if player:objectName() ~= lieges[1]:objectName() then return -3 end
				local enemy, enemy_hp = 0, 0
				for _, aplayer in sgs.qlist(self.room:getOtherPlayers(self.player)) do
					if self:objectiveLevel(aplayer, true) > 0 then enemy = enemy + 1 enemy_hp = enemy_hp + aplayer:getHp() end
				end
				local liege
				if enemy_hp - enemy >= liege_hp - #lieges then return -3 else return 4 end
			end
			return -3
		elseif player:getKingdom() ~= "god" then return 5 + modifier
		elseif sgs.ai_explicit[player:objectName()] == self:getHegKingdom() then 
			if self.player:getKingdom() ~= "god" and #lieges >= 1 and not recursive then
				for _, aplayer in sgs.qlist(self.room:getOtherPlayers(self.player)) do
					if self:objectiveLevel(aplayer, true) >= 0 then return -1 end
				end
				return 4
			end
			return -1
		elseif (sgs.ai_loyalty[self:getHegKingdom()][player:objectName()] or 0) == -160 then return 5 + modifier
		elseif (sgs.ai_loyalty[self:getHegKingdom()][player:objectName()] or 0) < -80 then return 4 + modifier
		end
		
		return 0
	end

	SmartAI.isFriend = function(self, player)
		return self:objectiveLevel(player) < 0
	end

	SmartAI.isEnemy = function(self, player)
		return self:objectiveLevel(player) >= 0
	end

	sgs.ai_card_intention["general"] = function(to, level)
		sgs.hegemony_to = to
		return -level
	end

	sgs.updateIntention = function(player, to, intention)
		intention = -intention
		local kingdoms = {"wei", "wu", "shu", "qun"}
		if player:getKingdom() ~= "god" then
			for _, akingdom in ipairs(kingdoms) do
				sgs.ai_loyalty[akingdom][player:objectName()] = -160
			end
			sgs.ai_loyalty[player:getKingdom()][player:objectName()] = 160
			sgs.ai_explicit[player:objectName()] = player:getKingdom()
			return
		end
		local kingdom = to:getKingdom()
		if kingdom ~= "god" then
			sgs.ai_loyalty[kingdom][player:objectName()] = (sgs.ai_loyalty[kingdom][player:objectName()] or 0) + intention
			if sgs.ai_loyalty[kingdom][player:objectName()] > 160 then sgs.ai_loyalty[kingdom][player:objectName()] = 160 end
			if sgs.ai_loyalty[kingdom][player:objectName()] < -160 then sgs.ai_loyalty[kingdom][player:objectName()] = -160 end
		elseif sgs.ai_explicit[player:objectName()] ~= "" then
			kingdom = sgs.ai_explicit[player:objectName()]
			sgs.ai_loyalty[kingdom][player:objectName()] = (sgs.ai_loyalty[kingdom][player:objectName()] or 0) + intention * 0.7
			if sgs.ai_loyalty[kingdom][player:objectName()] > 160 then sgs.ai_loyalty[kingdom][player:objectName()] = 160 end
			if sgs.ai_loyalty[kingdom][player:objectName()] < -160 then sgs.ai_loyalty[kingdom][player:objectName()] = -160 end
		else
			for _, aplayer in sgs.qlist(player:getRoom():getAlivePlayers()) do
				local kingdom = aplayer:getKingdom()
				if aplayer:objectName() ~= to:objectName() and kingdom ~= "god" and (sgs.ai_loyalty[kingdom][player:objectName()] or 0)>-80 then
					sgs.ai_loyalty[kingdom][player:objectName()] = (sgs.ai_loyalty[kingdom][player:objectName()] or 0) - intention * 0.2
					if sgs.ai_loyalty[kingdom][player:objectName()] > 160 then sgs.ai_loyalty[kingdom][player:objectName()] = 160 end
					if sgs.ai_loyalty[kingdom][player:objectName()] < -160 then sgs.ai_loyalty[kingdom][player:objectName()] = -160 end
				end
			end
		end
		local neg_loyalty_count, pos_loyalty_count, max_loyalty, max_kingdom = 0, 0
		for _, akingdom in ipairs(kingdoms) do
			local list = sgs.ai_loyalty[akingdom]
			if not max_loyalty then max_loyalty = (list[player:objectName()] or 0) max_kingdom = akingdom end
			if (list[player:objectName()] or 0)< 0 then
				neg_loyalty_count = neg_loyalty_count + 1
			elseif (list[player:objectName()] or 0)> 0 then
				pos_loyalty_count = pos_loyalty_count + 1
			end
			if max_loyalty < (list[player:objectName()] or 0) then
				max_loyalty = (list[player:objectName()] or 0)
				max_kingdom = akingdom
			end				
		end
		if neg_loyalty_count > 2 or pos_loyalty_count > 0 then
			sgs.ai_explicit[player:objectName()] = max_kingdom
		else
			sgs.ai_explicit[player:objectName()] = ""
		end
		-- self:printAll(player, intention)
	end

	SmartAI.updatePlayers = function(self, inclusive)
		local flist = {}
		local elist = {}
		self.friends = flist
		self.enemies = elist
		self.friends_noself = {}
		
		local players = sgs.QList2Table(self.room:getOtherPlayers(self.player))
		for _, aplayer in ipairs(players) do
			if self:isFriend(aplayer) then table.insert(flist, aplayer) end
		end
		for _, aplayer in ipairs(flist) do
			table.insert(self.friends_noself, aplayer)
		end
		table.insert(flist, self.player)
		for _, aplayer in ipairs(players) do
			if self:isEnemy(aplayer) then table.insert(elist, aplayer) end
		end
	end
	
	SmartAI.printAll = function(self, player, intention)
		local name = player:objectName()
		self.room:writeToConsole(self:getHegGeneralName(player) .. math.floor(intention*10)/10 ..
		" R" .. math.floor((sgs.ai_loyalty["shu"][name] or 0)*10)/10 ..
		" G" .. math.floor((sgs.ai_loyalty["wu"][name] or 0)*10)/10 ..
		" B" .. math.floor((sgs.ai_loyalty["wei"][name] or 0)*10)/10 ..
		" Q" .. math.floor((sgs.ai_loyalty["qun"][name] or 0)*10)/10 ..
		" E" .. (sgs.ai_explicit[name] or "nil"))
	end
	
	SmartAI.printFEList = function(self)
		for _, player in ipairs (self.enemies) do
			self.room:writeToConsole("enemy " .. self:getHegGeneralName(player))
		end

		for _, player in ipairs (self.friends_noself) do
			self.room:writeToConsole("friend " .. self:getHegGeneralName(player))
		end
		self.room:writeToConsole(self:getHegGeneralName().." list end")
	end
end
