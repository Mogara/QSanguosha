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
		local msg = string.format(fmt, ...)
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
					sgs.Sanguosha:translate(players[i]:getRole()),
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

function SmartAI:log(outString)
	self.room:output(outString)
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
			if p:getRole() == "rebel" and rebel_num > 0 then evaluate_rebel = evaluate_rebel + 1
			elseif p:getRole() == "loyalist" and loyalist_num > 0 then evaluate_loyalist = evaluate_loyalist + 1
			elseif p:getRole() == "renegade" and renegade_num > 0 then evaluate_renegade = evaluate_renegade + 1
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
	if string.len(str) > 3 and not string.find(str, "&") then
		global_room:writeToConsole("showskillname is empty! >> " .. str)
	end
	local card = cardparse(str)
	if not card then global_room:writeToConsole("Wrong!!sgs.Card_Parse >> " .. str) assert(false) end
	return card
end

function SmartAI:printAll(self, player, intention)
	local name = player:objectName()
	self.room:writeToConsole(self:getHegGeneralName(player) .. math.floor(intention * 10) / 10
							.. " R" .. math.floor((sgs.ai_loyalty["shu"][name] or 0) * 10) / 10
							.. " G" .. math.floor((sgs.ai_loyalty["wu"][name] or 0) * 10) / 10
							.. " B" .. math.floor((sgs.ai_loyalty["wei"][name] or 0) * 10) / 10
							.. " Q" .. math.floor((sgs.ai_loyalty["qun"][name] or 0) * 10) / 10
							.. " E" .. (sgs.ai_explicit[name] or "nil"))
end

function sgs.printFEList(player)
	global_room:writeToConsole("gameProcess :: " .. sgs.gameProcess())
	for _, p in sgs.qlist(global_room:getAlivePlayers()) do
		if player and p:objectName() ~= player:objectName() then continue end
		local name = p:getActualGeneral1Name() .. "/" .. p:getActualGeneral2Name()
		global_room:writeToConsole("----  " .. name .. "  kingdom::" .. p:getKingdom() .. "  ----")
		local sgsself = sgs.ais[p:objectName()]
		sgsself:updatePlayers()
		local msge = "enemies:"
		for _, enemy in ipairs(sgsself.enemies) do
			local name1 = enemy:getActualGeneral1Name() .. "/" .. enemy:getActualGeneral2Name()
			msge = msge .. name1 .. ", "
		end
		global_room:writeToConsole(msge)
		local msgf = "friends:"
		for _, friend in ipairs(sgsself.friends) do
			local name2 = friend:getActualGeneral1Name() .. "/" .. friend:getActualGeneral2Name()
			msgf = msgf .. name2 .. ", "
		end
		global_room:writeToConsole(msgf)
	end
end

function sgs.ShowPlayer(player)
	for _, p in sgs.qlist(global_room:getAlivePlayers()) do
		if player then
			if player:objectName() == p:objectName() then p:showGeneral() end
		else
			p:showGeneral()
		end
	end
end
