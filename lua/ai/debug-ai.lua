function SmartAI:printStand()
	self.room:output(self.player:getRole())
	self.room:output("enemies:")
	for _, player in ipairs(self.enemies) do
		self.room:output(player:getGeneralName())
	end
	self.room:output("end of enemies")
	self.room:output("friends:")
	for _, player in ipairs(self.friends) do
		self.room:output(player:getGeneralName())
	end
	self.room:output("end of friends")
end

function SmartAI:printFEList()
	for _, player in ipairs (self.enemies) do
		self.room:writeToConsole("enemy "..player:getGeneralName()..(sgs.role_evaluation[player:objectName()][player:getRole()] or "") .. player:getRole())
	end
	for _, player in ipairs (self.friends_noself) do
		self.room:writeToConsole("friend "..player:getGeneralName()..(sgs.role_evaluation[player:objectName()][player:getRole()] or "") .. player:getRole())
	end
	self.room:writeToConsole(self.player:getGeneralName().." list end")
end

function SmartAI:log(outString)
	self.room:output(outString)
end

function outputPlayersEvaluation()
	for _, player in sgs.qlist(global_room:getOtherPlayers(global_room:getLord())) do
		local evaluate_role = sgs.evaluatePlayerRole(player)
		global_room:writeToConsole("Misjudge-------" .. player:getGeneralName())
		global_room:writeToConsole("[Role: " .. player:getRole() .. "      Evaluate role: " .. evaluate_role)
		global_room:writeToConsole("Rebel:" .. sgs.role_evaluation[player:objectName()]["rebel"] .. " Loyalist:"
									.. sgs.role_evaluation[player:objectName()]["loyalist"] .. " Renegade:"
									.. sgs.role_evaluation[player:objectName()]["renegade"] .. "]")
	end
end

function sgs.checkMisjudge(player)
	local room = global_room
	local mode = room:getMode()
	if player then
		if sgs.current_mode_players[player:getRole()] > sgs.mode_player[mode][player:getRole()] or sgs.current_mode_players[player:getRole()] < 0 then
			player:getRoom():writeToConsole("Misjudge--------> Role:" .. player:getRole() .. " Current players:" .. sgs.current_mode_players[player:getRole()]
											.. " Valid players:" .. sgs.mode_player[mode][player:getRole()])
		end
	else
		local rebel_num, loyalist_num, renegade_num = 0, 0, 0
		local evaluate_rebel, evaluate_loyalist, evaluate_renegade = 0, 0, 0
		for _, p in sgs.qlist(room:getOtherPlayers(room:getLord())) do
			local role = p:getRole()
			if role == "rebel" then rebel_num = rebel_num + 1
			elseif role == "loyalist" then loyalist_num = loyalist_num + 1
			elseif role == "renegade" then renegade_num = renegade_num + 1
			end
			
			if sgs.evaluatePlayerRole(p) == "rebel" then evaluate_rebel = evaluate_rebel + 1
			elseif sgs.evaluatePlayerRole(p) == "loyalist" then evaluate_loyalist = evaluate_loyalist + 1
			elseif sgs.evaluatePlayerRole(p) == "renegade" then evaluate_renegade = evaluate_renegade + 1
			end
		end
		
		if evaluate_renegade == 0 then 
			if evaluate_rebel > rebel_num+renegade_num or evaluate_loyalist > loyalist_num+renegade_num then
				outputPlayersEvaluation()
			end
		else
			if evaluate_rebel > rebel_num or evaluate_loyalist > loyalist_num or evaluate_renegade > renegade_num then
				outputPlayersEvaluation()
			end
		end
	end
end

local cardparse = sgs.Card_Parse
function sgs.Card_Parse(str)
	if not str then global_room:writeToConsole(debug.traceback()) end
	if type(str) ~= "string" and type(str) ~= "number" and str.toString then 
		global_room:writeToConsole(str:toString())
	end
	return cardparse(str)
end
