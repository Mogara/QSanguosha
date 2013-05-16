sgs.ai_debug_func[sgs.EventPhaseStart].debugfunc=function(self, player, data)
	if player:getPhase()== sgs.Player_Start then
		debugFunc(self, self.room, player, data)
	 end
end

sgs.ai_debug_func[sgs.CardUsed].debugfunc=function(self, player, data)
	local card= data:toCardUse().card
	if card:isKindOf("Peach") or card:isKindOf("Nullification") then
		debugFunc(self, self.room, player, data)
	 end
end

function debugFunc(self, room, player, data)
	local owner =room:getOwner()
	local choices = {"showVisiblecards","showHandcards","objectiveLevel","getDefenseSlash"}
	local debugmsg =function(fmt,...)
		if type(fmt)=="boolean" then fmt = fmt and "true" or "false" end
		local msg=string.format(fmt, ...)
		player:speak(msg)
		logmsg("ai.html","<pre>"..msg.."</pre>")
	end
	local players = sgs.QList2Table(room:getAlivePlayers())
	repeat
		local choice=room:askForChoice(owner,"aidebug","cancel+"..table.concat(choices,"+"))
		if choice=="cancel" then break end
		if choice == "showVisiblecards" then
			debugmsg(" ")
			debugmsg("===================")
			debugmsg("查看已知牌; 当前AI是: %s[%s]",sgs.Sanguosha:translate(player:getGeneralName()),sgs.Sanguosha:translate(player:getRole()) )
			for i=1, #players, 1 do
				local msg=string.format("%s已知牌:",sgs.Sanguosha:translate(players[i]:getGeneralName()))
				local cards = sgs.QList2Table(players[i]:getHandcards())
				for _, card in ipairs(cards) do
					local flag=string.format("%s_%s_%s","visible",player:objectName(),players[i]:objectName())
					if card:hasFlag("visible") or card:hasFlag(flag) then
						msg = msg .. card:getLogName() ..", "
					end
				end
				debugmsg(msg)
			end
		end
		if choice == "showHandcards" then
			debugmsg(" ")
			debugmsg("===================")
			debugmsg("查看手牌; 当前AI是: %s[%s]",sgs.Sanguosha:translate(player:getGeneralName()),sgs.Sanguosha:translate(player:getRole()) )
			for i=1, #players, 1 do
				local msg=string.format("%s手牌:",sgs.Sanguosha:translate(players[i]:getGeneralName()))
				local cards = sgs.QList2Table(players[i]:getHandcards())
				for _, card in ipairs(cards) do
					msg = msg .. card:getLogName() ..", "
				end
				debugmsg(msg)
			end
		end
		if choice == "objectiveLevel" then
			debugmsg(" ")
			debugmsg("============%s(%.1f)", sgs.gameProcess(room), sgs.gameProcess(room,1))
			debugmsg("查看关系; 当前AI是: %s[%s]",sgs.Sanguosha:translate(player:getGeneralName()),sgs.Sanguosha:translate(player:getRole()) )
			for i=1, #players, 1 do
				local level=self:objectiveLevel(players[i])
				local rel =level>0 and "敌对" or (level<0 and "友好" or "中立")
				rel = rel .. " " .. level

				debugmsg("%s[%s]: %d:%d:%d %s",
					sgs.Sanguosha:translate(players[i]:getGeneralName()),					
					sgs.Sanguosha:translate(sgs.evaluatePlayerRole(players[i])),
					sgs.role_evaluation[players[i]:objectName()]["rebel"],
					sgs.role_evaluation[players[i]:objectName()]["loyalist"],
					sgs.role_evaluation[players[i]:objectName()]["renegade"],
					  rel)
			end
		end

		if choice == "getDefenseSlash" then
			debugmsg(" ")
			debugmsg("===================")
			debugmsg("查看对杀的防御; 当前AI是: %s[%s]",sgs.Sanguosha:translate(player:getGeneralName()),sgs.Sanguosha:translate(player:getRole()) )
			for i=1, #players, 1 do
				debugmsg("%s:%.2f",sgs.Sanguosha:translate(players[i]:getGeneralName()),sgs.getDefenseSlash(players[i]))				
			end
		end
		
	until false  
end


function logmsg(fname,fmt,...)
	local fp = io.open(fname,"ab")
	if type(fmt)=="boolean" then fmt = fmt and "true" or "false" end
	fp:write(string.format(fmt, ...).."\r\n")
	fp:close()
end

function endlessNiepan(who)
	local room = who:getRoom()	
	if who:getGeneral2() or who:getHp() > 0 then return end

	local rebel_value = sgs.role_evaluation[who:objectName()]["rebel"]
	local renegade_value = sgs.role_evaluation[who:objectName()]["renegade"]
	local loyalist_value = sgs.role_evaluation[who:objectName()]["loyalist"]
	
	for _,skill in sgs.qlist(who:getVisibleSkillList()) do
		if skill:getLocation()==sgs.Skill_Right then
			room:detachSkillFromPlayer(who, skill:objectName())
		end
	end
	local names = sgs.Sanguosha:getRandomGenerals(1)
	room:changeHero(who, names[1] , true, true, false, true)
	room:setPlayerProperty(who, "maxhp", sgs.QVariant(5))
	room:setPlayerProperty(who, "kingdom", sgs.QVariant(who:getGeneral():getKingdom()))
	who:setGender(who:getGeneral():getGender())
	room:setTag("SwapPile",sgs.QVariant(0))

	who:bury()
	who:drawCards(5)

	sgs.role_evaluation[who:objectName()]["rebel"] = rebel_value
	sgs.role_evaluation[who:objectName()]["renegade"] = renegade_value
	sgs.role_evaluation[who:objectName()]["loyalist"] = loyalist_value
end


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
	if not global_room:getLord() then return end
	global_room:writeToConsole("=========== MISJUDGE START ===========" )
	for _, player in sgs.qlist(global_room:getOtherPlayers(global_room:getLord())) do
		local evaluate_role = sgs.evaluatePlayerRole(player)
		global_room:writeToConsole("<------- " .. player:getGeneralName() .. " ------->")
		global_room:writeToConsole("Role: " .. player:getRole() .. "	  Evaluate role: " .. evaluate_role)
		global_room:writeToConsole("Rebel:" .. sgs.role_evaluation[player:objectName()]["rebel"] .. " Loyalist:"
									.. sgs.role_evaluation[player:objectName()]["loyalist"] .. " Renegade:"
									.. sgs.role_evaluation[player:objectName()]["renegade"])
	end
	global_room:writeToConsole("================ END ================" )
end

function sgs.checkMisjudge(player)
	if not global_room:getLord() then return end

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
		end
		for _, p in sgs.qlist(room:getOtherPlayers(room:getLord())) do
			if sgs.evaluatePlayerRole(p) == "rebel" and rebel_num > 0 then evaluate_rebel = evaluate_rebel + 1
			elseif sgs.evaluatePlayerRole(p) == "loyalist" and loyalist_num > 0 then evaluate_loyalist = evaluate_loyalist + 1
			elseif sgs.evaluatePlayerRole(p) == "renegade" and renegade_num > 0 then evaluate_renegade = evaluate_renegade + 1
			end
		end
		
		if evaluate_renegade < 1 then 
			if (evaluate_rebel >= rebel_num + renegade_num and evaluate_rebel > rebel_num)
				or (evaluate_loyalist >= loyalist_num + renegade_num and evaluate_loyalist > loyalist_num)
				or (evaluate_rebel == rebel_num + 1 and evaluate_loyalist == loyalist_num + 1) then
				outputPlayersEvaluation()
				if evaluate_rebel >= rebel_num + renegade_num and evaluate_rebel > rebel_num then
					sgs.modifiedRoleTrends("rebel")
				elseif evaluate_loyalist >= loyalist_num + renegade_num and evaluate_loyalist > loyalist_num and rebel_num > 0 then
					sgs.modifiedRoleTrends("loyalist")
				elseif  evaluate_rebel > rebel_num and evaluate_loyalist > loyalist_num then
					sgs.modifiedRoleTrends("rebel")
					sgs.modifiedRoleTrends("loyalist")
				end
			end
		else
			if evaluate_rebel > rebel_num or evaluate_loyalist > loyalist_num or evaluate_renegade > renegade_num then
				outputPlayersEvaluation()
				if evaluate_rebel > rebel_num then sgs.modifiedRoleTrends("rebel") end
				if evaluate_loyalist > loyalist_num then sgs.modifiedRoleTrends("loyalist") end
				if evaluate_renegade > renegade_num then sgs.modifiedRoleTrends("renegade") end
			end
		end
	end
end

local cardparse = sgs.Card_Parse
function sgs.Card_Parse(str)
	if not str then global_room:writeToConsole(debug.traceback()) end
	if type(str) ~= "string" and type(str) ~= "number" and str.toString() then
		global_room:writeToConsole(str:toString())
	end
	return cardparse(str)
end
