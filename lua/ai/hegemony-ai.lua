if sgs.GetConfig("EnableHegemony", false) then
	useDefaultStrategy = function()
		return true
	end

	SmartAI.getHegKingdom = function(self)
		local names = self.room:getTag(self.player:objectName()):toStringList()

		if #names<1 then return self.player:getKingdom() end
		local kingdom = sgs.Sanguosha:getGeneral(names[1]):getKingdom()
		--player:getRoom():output(blah)
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

		--if self is shown , then befriend the friends & hostile to all others

		if self.player:getKingdom() ~= "god" then
			if self.player:getKingdom() == player:getKingdom() then return -1 else return 5 end
		--if self is not shown, then hostile to all except for friend in later game
		else
			local party = 0
			for _, p in sgs.qlist(self.room:getAllPlayers()) do
				if p:getKingdom() == self:getHegKingdom() then party = party +1 end
			end

			self:log(party..' '..self.room:getAllPlayers():length())

			if (party < 3) and
			(2 * party + 2 >= self.room:getAllPlayers():length()) then
				if self:getHegKingdom() == player:getKingdom() then return -1 else return 5 end
			else
				return 4 end
		end

		-- below is not executed at all

		if self.player:getKingdom() ~= "god" then
			if self.player:getKingdom() == player:getKingdom() then return -1
			else
				if #anjiangs >= self.room:getAllPlayers():length() - #anjiangs then
					if player:getKingdom() ~= "god" then return -1 else return 4 end
				else
					if player:getKingdom() ~= "god" then
						if self_friends > player_friends then return 0 else return 5 end
					end
				end
			end
		else
			if player:getKingdom() ~= "god" then return 5 else return 4 end
		end

		return 0
	end
end
