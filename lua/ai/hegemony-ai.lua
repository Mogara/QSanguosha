if sgs.GetConfig("EnableHegemony", false) then
	useDefaultStrategy = function()
		return false
	end

	sgs.ai_loyalty = {
		wei = {name = "wei"},
		wu = {name = "wu"},
		shu = {name = "shu"},
		qun = {name = "qun"},
	}

	SmartAI.getHegKingdom = function(self)
		local names = self.room:getTag(self.player:objectName()):toStringList()

		if names:isEmpty() then return self.player:getKingdom() end
		local kingdom = sgs.Sanguosha:getGeneral(names[1]):getKingdom()
		return kingdom
	end

	SmartAI.objectiveLevel = function(self, player)
		local anjiangs = {}
		for _, p in sgs.qlist(self.room:getAllPlayers()) do
			if p:getGeneralName() == "anjiang" then
				table.insert(anjiangs, p)
			end
		end

		local player_friends, self_friends = 0, 0
		for _, p in sgs.qlist(self.room:getAllPlayers()) do
			if p:getKingdom() == self:getHegKingdom() then self_friends = self_friends + 1
			elseif p:getKingdom() == player:getKingdom()
				and not (p:getKingdom() == "god") then player_friends = player_friends + 1
			end
		end

		if self:getHegKingdom() == player:getKingdom() and self:getHegKingdom() ~= "god" then return -1
		elseif sgs.ai_explicit[player:objectName()] == self.player:getKingdom() then return -1
		elseif (sgs.ai_loyalty[self.player:getHegKingdom()][player:objectName()] or 0) > 0 then return 5
		else return 0 end
	end

	SmartAI.isFriend = function(self, player)
		return self:objectiveLevel(player) < 0
	end

	SmartAI.isEnemy = function(self, player)
		return self:objectiveLevel(player) >= 0
	end

	sgs.ai_intention["general"] = function(to, level)
		sgs.hegemony_to = to
		sgs.hegemony_level = level
		return level
	end

	SmartAI.refreshLoyalty = function(player, intention)
		if not sgs.recorder then sgs.recorder = self end
		if not sgs.hegemony_to or sgs.hegemony_level ~= intention or self ~= sgs.recorder then return end
		local to = sgs.hegemony_to
		local kingdom = to:getKingdom()
		if kingdom ~= "god" then
			sgs.ai_loyalty[kingdom][player:objectName()] = (sgs.ai_loyalty[kingdom][player:objectName()] or 0) + intention
		elseif sgs.ai_explicit[player:objectName()] then
			sgs.ai_loyalty[kingdom][player:objectName()] = (sgs.ai_loyalty[kingdom][player:objectName()] or 0) + intention * 0.7
		else
			for _, aplayer in sgs.qlist(player:getRoom():getOtherPlayers(self.player)) do
				local kingdom = aplayer:getKingdom()
				if aplayer:objectName() ~= to:objectName() and kingdom ~= "god" then
					sgs.ai_loyalty[kingdom][player:objectName()] = (sgs.ai_loyalty[kingdom][player:objectName()] or 0) - intention * 0.3
				end
			end
		end
		local intention_count, min_intention = 0
		local min_kingdom
		for _, list in ipairs(sgs.ai_loyalty) do
			if list[player:objectName()] then
				if not min_intention then min_intention = list[player:objectName()] min_kingdom = list.name end
				if list[player:objectName()] > 0 then
					intention_count = intention_count + 1
				elseif min_intention > list[player:objectName()] then
					min_intention = list[player:objectName()]
					min_kingdom = list.name
				end
			end
		end
		if intention_count < 3 then return end
		sgs.ai_explicit[player:objectName()] = min_kingdom
	end

	SmartAI.updatePlayers = function(self, inclusive)
		local flist = {}
		local elist = {}
		self.friends = flist
		self.enemy = elist
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
end
