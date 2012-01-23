sgs.ai_card_intention={}
sgs.ai_card_intention={}
sgs.ai_renegade_suspect={}
sgs.ai_anti_lord={}
sgs.ai_lord_tolerance={}

sgs.ai_card_intention["general"]=function(to,level)
	if not to then return 0 end
	local has_rebel = false
	-- if not to.getRoom then global_room:writeToConsole(debug.traceback()) end
	for _, aplayer in sgs.qlist(to:getRoom():getAlivePlayers()) do
		if aplayer:getRole() == "rebel" then has_rebel = true break end
	end
	if not has_rebel then
		if to:isLord() then return -level*3 else return 0 end
	end
	if to:isLord() then
		return -level*2
	elseif sgs.ai_explicit[to:objectName()]:match("loyal") or sgs.singleRole(to:getRoom()) == "loyalist" then
		return -level
	elseif sgs.ai_explicit[to:objectName()]:match("rebel") or sgs.singleRole(to:getRoom()) == "rebel" then
		return level 
	else
		return 0
	end
end

function sgs.refreshLoyalty(player,intention)
	if player:isLord() then return end
	local name=player:objectName()

	if sgs.isRolePredictable() then
		if player:getRole() == "loyalist" and intention > 0 then sgs.ai_explicit[name] = "loyalist"
		elseif player:getRole() == "rebel" and intention < 0 then sgs.ai_explicit[name] = "rebel"
		elseif player:getRole() == "renegade" and intention > 0 then sgs.ai_explicit[name] = "loyalist"
		elseif player:getRole() == "renegade" and intention < 0 then sgs.ai_explicit[name] = "rebel" end
		return
	end
	
	local has_rebel = false
	for _, aplayer in sgs.qlist(player:getRoom():getAlivePlayers()) do
		if aplayer:getRole() == "rebel" then has_rebel = true break end
	end
	if not has_rebel then
		if intention < -80 then
			sgs.ai_anti_lord[name] = (sgs.ai_anti_lord[name] or 0) + 1
			sgs.ai_renegade_suspect[name] = (sgs.ai_renegade_suspect[name] or 0) + 1
		end
	end
	if math.abs(intention)>70 and math.abs(sgs.ai_loyalty[name] or 0) > 70 then
		if sgs.ai_loyalty[name]*intention<0 then
			sgs.ai_loyalty[name]=sgs.ai_loyalty[name]/2
			sgs.refreshLoyalty(player,0)
			sgs.ai_renegade_suspect[name]=(sgs.ai_renegade_suspect[name] or 0)+1
		end
	end
	
	if ((sgs.ai_anti_lord[name] or 0)-2)>(sgs.ai_lord_tolerance[name] or 0) then 
		if intention>0 then intention=intention/5 end
	end
	sgs.ai_loyalty[name]=sgs.ai_loyalty[name]+intention


	if (sgs.ai_renegade_suspect[name] or 0) > 1 then
		if intention >0 then sgs.ai_explicit[name] = "loyalish"
		elseif intention < 0 then sgs.ai_explicit[name] = "rebelish" end
		return
	end
	
	if sgs.ai_loyalty[name]<=-160 then
		sgs.ai_loyalty[name]=-160
		sgs.ai_explicit[name] = "rebel"
	elseif sgs.ai_loyalty[name]<=-70 then
		sgs.ai_explicit[name] = "rebelish"
	elseif sgs.ai_loyalty[name]>=160 then
		sgs.ai_explicit[name] = "loyalist"
		sgs.ai_loyalty[name]=160
	elseif sgs.ai_loyalty[name]>=70 then
		sgs.ai_explicit[name] = "loyalish"
	end
	--self:printAll(player, intention)
	--self:checkMisjudge(player)
end

function sgs.updateIntention(from, to, intention, card)
	intention = sgs.ai_card_intention.general(to, intention)
	--[[if (from:getRole() == "loyalist" and intention < 0) or (from:getRole() == "rebel" and intention > 0) then
		local str = from:getGeneralName() .. "->" .. to:getGeneralName() .. ":" .. intention .. "@" .. from:getRoom():getCurrent():getGeneralName()
		if card then str = str .. "#" .. card:className() end
		from:getRoom():writeToConsole(str)
	end]]
	sgs.refreshLoyalty(from, intention)
	if to:isLord() and intention < 0 then sgs.ai_anti_lord[from:objectName()] = (sgs.ai_anti_lord[from:objectName()] or 0) + 1 end
end

function sgs.updateIntentions(from, tos, intention)
	for _, to in ipairs(tos) do
		if from:objectName() ~= to:objectName() then
			sgs.updateIntention(from, to, intention)
		end
	end
end

function sgs.singleRole(room, player)
	local players
	if player then players = room:getOtherPlayers(player) else players = room:getAlivePlayers() end
	local loyal_num, rebel_num = 1, 0
	for _, aplayer in sgs.qlist(players) do
		if aplayer:getRole() == "loyalist" then loyal_num = loyal_num + 1
		elseif aplayer:getRole() == "rebel" then rebel_num = rebel_num + 1 end
		if (sgs.ai_explicit[aplayer:objectName()] or ""):match("loyal") then loyal_num = loyal_num - 1
		elseif (sgs.ai_explicit[aplayer:objectName()] or ""):match("rebel") then rebel_num = rebel_num -1 end
	end
	if loyal_num <= 0 and rebel_num > 0 then return "rebel"
	elseif loyal_num > 0 and rebel_num <= 0 then return "loyalist"
	else return "neither" end
end

sgs.ai_card_intention.Indulgence = 120

sgs.ai_card_intention.SupplyShortage = 120

sgs.ai_card_intention["Slash"]=function(card,from,tos,source)
	for _, to in ipairs(tos) do
		if sgs.ai_liuli_effect then
			sgs.ai_liuli_effect=false
			return
		end
		local modifier=0
		if sgs.ai_collateral then sgs.ai_collateral=false modifier=-40 end
		local value=80+modifier

		if sgs.ai_leiji_effect then
			if from and from:hasSkill("liegong") then return end
			sgs.ai_leiji_effect = false
			if sgs.ai_pojun_effect then value = value/1.5 else value = -value/1.5 end
		end
		if sgs.ai_pojun_effect then
			sgs.ai_pojun_effect=false
			if to:getHandcardNum()>5 then value = -value end
		end
		speakTrigger(card,from,to)
		if to:hasSkill("yiji") then 
			value = value*(2-to:getHp())/1.1
		end
		sgs.updateIntention(from, to, value)
	end
end

sgs.ai_card_intention.FireSlash = sgs.ai_card_intention.Slash

sgs.ai_card_intention.ThunderSlash = sgs.ai_card_intention.Slash

sgs.ai_card_intention.Peach = -120

sgs.ai_card_intention["Duel"]=function(card,from,tos,source)
	if sgs.ai_lijian_effect then 
		sgs.ai_lijian_effect = false
		return
	end
	sgs.updateIntentions(from, tos, 80)
end

sgs.ai_card_intention.Collateral = 80

sgs.ai_card_intention.FireAttack = 80

sgs.ai_card_intention["IronChain"]=function(card,from,tos,source)
	for _, to in ipairs(tos) do
		if to:isChained() then
			sgs.updateIntention(from, to, 80)
		else 
			sgs.updateIntention(from, to, -80)
		end
	end
end

sgs.ai_card_intention["Dismantlement"]=function(card,from,tos,source)
	for _, to in ipairs(tos) do
		if to:getCards("j"):isEmpty() and
			not (to:getArmor() and (to:getArmor():inherits("GaleShell") or to:getArmor():inherits("SilverLion"))) then
			sgs.updateIntention(from, to, 80)
		end
	end
end

sgs.ai_card_intention["Snatch"]=sgs.ai_card_intention["Dismantlement"]

sgs.ai_card_intention["QixiCard"]=sgs.ai_card_intention["Dismantlement"]

sgs.ai_card_intention["JixiCard"]=sgs.ai_card_intention["Snatch"]

sgs.ai_card_intention["TuxiCard"]=80

sgs.ai_card_intention["LeijiCard"]=80

sgs.ai_card_intention["RendeCard"]=-70

sgs.ai_card_intention["QingnangCard"]=-100

sgs.ai_card_intention["ShensuCard"]=80

sgs.ai_card_intention["QiangxiCard"]=80

sgs.ai_card_intention["JieyinCard"]=-80

sgs.ai_card_intention["HuangtianCard"]=function(card,from,tos,source)
	for _, to in ipairs(tos) do
		sgs.updateIntention(from, to, -80)
		if to:isLord() then sgs.ai_lord_tolerance[from:objectName()]=(sgs.ai_lord_tolerance[from:objectName()] or 0)+1 end
	end
end

sgs.ai_card_intention["JiemingCard"]=-80

sgs.ai_card_intention["HaoshiCard"]=-80

sgs.ai_card_intention["FanjianCard"]=70

sgs.ai_card_intention["TianyiCard"]=30

sgs.ai_card_intention["QuhuCard"] = sgs.ai_card_intention["TianyiCard"]

sgs.ai_card_intention["JujianCard"]=-70

sgs.ai_card_intention["MingceCard"]=-70

sgs.ai_card_intention["XianzhenCard"]=70

sgs.ai_card_intention["LiuliCard"]=function(card,from,to,source)
	sgs.ai_liuli_effect=true
end

sgs.ai_card_intention["JujianCard"]=-80

sgs.ai_card_intention["TiaoxinCard"]=80

sgs.ai_card_intention["ZhijianCard"]=-80

sgs.ai_card_intention["QiaobianCard"] = function(card, from, tos, source)
	if from:getPhase() == sgs.Player_Draw then
		for _, to in ipairs(tos) do
			sgs.updateIntention(from, to, sgs.ai_card_intention["TuxiCard"])
		end
	end
	return 0
end

sgs.ai_card_intention["ChengxiangCard"]=sgs.ai_card_intention["QingnangCard"]

sgs.ai_card_intention["JuejiCard"]=sgs.ai_card_intention["TianyiCard"]

sgs.ai_card_intention["LianliCard"]=-80

sgs.ai_card_intention["QiaocaiCard"]=-70

sgs.ai_card_intention["ShouyeCard"]=sgs.ai_card_intention["JujianCard"]

sgs.ai_card_intention["HouyuanCard"]=sgs.ai_card_intention["JujianCard"]

sgs.ai_card_intention["BawangCard"]=sgs.ai_card_intention["ShensuCard"]

sgs.ai_card_intention["WeidaiCard"]=sgs.ai_card_intention["Peach"]

sgs.ai_card_intention["GongxinCard"]=sgs.ai_card_intention["TianyiCard"]

sgs.ai_card_intention["SmallYeyanCard"]=sgs.ai_card_intention["QiangxiCard"]

sgs.ai_card_intention["MediumYeyanCard"]=200

sgs.ai_card_intention["GreatYeyanCard"]=sgs.ai_card_intention["SmallYeyanCard"]

sgs.ai_card_intention["WuqianCard"]=sgs.ai_card_intention["XianzhenCard"]

sgs.ai_card_intention["KuangfengCard"]=80

sgs.ai_card_intention["DawuCard"]=-70

sgs.ai_card_intention["GaleShell"]=sgs.ai_card_intention["KuangfengCard"]

sgs.ai_explicit={}
sgs.ai_loyalty={}
