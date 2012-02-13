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
		self.room:writeToConsole("enemy "..player:getGeneralName()..(sgs.ai_explicit[player:objectName()] or "") .. player:getRole())
	end
	for _, player in ipairs (self.friends_noself) do
		self.room:writeToConsole("friend "..player:getGeneralName()..(sgs.ai_explicit[player:objectName()] or "") .. player:getRole())
	end
	self.room:writeToConsole(self.player:getGeneralName().." list end")
end

function SmartAI:log(outString)
	self.room:output(outString)
end

function sgs.checkMisjudge(player)
	local name = player:objectName()
	if ((sgs.ai_explicit[name] or ""):match("loyal") and player:getRole() == "rebel")
		or ((sgs.ai_explicit[name] or ""):match("rebel") and player:getRole() == "loyalist")
		or ((sgs.ai_renegade_suspect[name] or 0)> 2 and player:getRole() ~= "renegade")
		then
		player:getRoom():writeToConsole("++++++++" .. player:getGeneralName() .. " " .. 
			sgs.ai_explicit[name] .. " " .. player:getRole() .. " " .. player:getRoom():alivePlayerCount() .. (sgs.ai_renegade_suspect[name] or 0))
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
