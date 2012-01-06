if sgs.GetConfig("EnableHegemony", false) then
	useDefaultStrategy = function()
		return false
	end

	sgs.ai_loyalty = {
		wei = {},
		wu = {},
		shu = {},
		qun = {},
	}

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
	
	SmartAI.objectiveLevel = function(self, player)
		if self.player:objectName() == player:objectName() then return -5 end
		local anjiangs = {}
		for _, p in sgs.qlist(self.room:getAllPlayers()) do
			if p:getGeneralName() == "anjiang" then
				table.insert(anjiangs, p)
			end
		end

		if self:getHegKingdom() == player:getKingdom() and self:getHegKingdom() ~= "god" then return -3
		elseif player:getKingdom() ~= "god" then return 5
		elseif sgs.ai_explicit[player:objectName()] == self:getHegKingdom() then return -1
		elseif (sgs.ai_loyalty[self:getHegKingdom()][player:objectName()] or 0) < 0 then return 4
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

	SmartAI.refreshLoyalty = function(self, player, intention)
		local to = sgs.hegemony_to
		local kingdom = to:getKingdom()
		if kingdom ~= "god" then
			sgs.ai_loyalty[kingdom][player:objectName()] = (sgs.ai_loyalty[kingdom][player:objectName()] or 0) + intention
			if sgs.ai_loyalty[kingdom][player:objectName()] > 160 then sgs.ai_loyalty[kingdom][player:objectName()] = 160 end
			if sgs.ai_loyalty[kingdom][player:objectName()] < -160 then sgs.ai_loyalty[kingdom][player:objectName()] = -160 end
		elseif sgs.ai_explicit[player:objectName()] then
			kingdom = sgs.ai_explicit[player:objectName()]
			sgs.ai_loyalty[kingdom][player:objectName()] = (sgs.ai_loyalty[kingdom][player:objectName()] or 0) + intention * 0.7
			if sgs.ai_loyalty[kingdom][player:objectName()] > 160 then sgs.ai_loyalty[kingdom][player:objectName()] = 160 end
			if sgs.ai_loyalty[kingdom][player:objectName()] < -160 then sgs.ai_loyalty[kingdom][player:objectName()] = -160 end
		else
			for _, aplayer in sgs.qlist(player:getRoom():getOtherPlayers(self.player)) do
				local kingdom = aplayer:getKingdom()
				if aplayer:objectName() ~= to:objectName() and kingdom ~= "god" then
					sgs.ai_loyalty[kingdom][player:objectName()] = (sgs.ai_loyalty[kingdom][player:objectName()] or 0) - intention * 0.6
					if sgs.ai_loyalty[kingdom][player:objectName()] > 160 then sgs.ai_loyalty[kingdom][player:objectName()] = 160 end
					if sgs.ai_loyalty[kingdom][player:objectName()] < -160 then sgs.ai_loyalty[kingdom][player:objectName()] = -160 end
				end
			end
		end
		local neg_loyalty_count, pos_loyalty_count, max_loyalty, max_kingdom = 0, 0
		local kingdoms = {"wei", "wu", "shu", "qun"}
		for _, akingdom in ipairs(kingdoms) do
			local list = sgs.ai_loyalty[akingdom]
			if list[player:objectName()] then
				if not max_loyalty then max_loyalty = list[player:objectName()] max_kingdom = akingdom end
				if list[player:objectName()] < 0 then
					neg_loyalty_count = neg_loyalty_count + 1
				elseif list[player:objectName()] > 0 then
					pos_loyalty_count = pos_loyalty_count + 1
				end
				if max_loyalty < list[player:objectName()] then
					max_loyalty = list[player:objectName()]
					max_kingdom = akingdom
				end				
			end
		end
		if neg_loyalty_count > 2 or pos_loyalty_count > 0 then
			sgs.ai_explicit[player:objectName()] = max_kingdom
		end
		self:printAll(player, intention)
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
		table.insert(self.friends, self.player)
		self:sortEnemies(players)
		for _, aplayer in ipairs(players) do
			if #elist == 0 then 
				table.insert(elist, aplayer)
			elseif self:objectiveLevel(aplayer) <= 0 then
				return
			end
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
end
