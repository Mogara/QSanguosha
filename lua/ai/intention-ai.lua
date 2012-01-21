sgs.ai_card_intention={}
sgs.ai_card_intention={}
sgs.ai_assumed["rebel"]=0
sgs.ai_assumed["loyalist"]=0
sgs.ai_assumed["renegade"]=0
sgs.ai_renegade_suspect={}
sgs.ai_anti_lord={}
sgs.ai_lord_tolerance={}

sgs.ai_card_intention["general"]=function(to,level)
	if not to then return 0 end
	local has_rebel = false
	for _, aplayer in sgs.qlist(to:getRoom():getAlivePlayers()) do
		if aplayer:getRole() == "rebel" then has_rebel = true break end
	end
	if not has_rebel then
		if to:isLord() then return -level*3 else return 0 end
	end
	if to:isLord() then
		return -level*2
	elseif sgs.ai_explicit[to:objectName()]=="loyalist" then
		return -level
	elseif sgs.ai_explicit[to:objectName()]=="loyalish" then
		return -level
	elseif sgs.ai_explicit[to:objectName()]=="rebel" then
		return level
	elseif sgs.ai_explicit[to:objectName()]=="rebelish" then
		return level
	else
		local nonloyals=sgs.ai_assumed["rebel"]--+sgs.ai_assumed["renegade"]
		local loyals=sgs.ai_assumed["loyalist"]
		if loyals+nonloyals<=1 then return 0 end

		local ratio
		if loyals<=0 then ratio=1
		elseif nonloyals<=0 then ratio =-1 

		else
			local ratio1=(-loyals+nonloyals-1)/(loyals+nonloyals)
			local ratio2=(-loyals+nonloyals+1)/(loyals+nonloyals)
			ratio=1-math.sqrt((1-ratio1)*(1-ratio2))
		end

		return level*ratio
	end
end

function refreshLoyalty(player,intention)
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
	for _, aplayer in sgs.qlist(self.room:getAlivePlayers()) do
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
			self:refreshLoyalty(player,0)
			sgs.ai_renegade_suspect[name]=(sgs.ai_renegade_suspect[name] or 0)+1
		end
	end
	
	if ((sgs.ai_anti_lord[name] or 0)-2)>(sgs.ai_lord_tolerance[name] or 0) then 
		if intention>0 then intention=intention/5 end
	end
	sgs.ai_loyalty[name]=sgs.ai_loyalty[name]+intention

	
	if sgs.ai_explicit[name]=="loyalish" then
		sgs.ai_assumed["loyalist"]=sgs.ai_assumed["loyalist"]+1
	elseif sgs.ai_explicit[name]=="loyalist" then
		sgs.ai_assumed["loyalist"]=sgs.ai_assumed["loyalist"]+1
	elseif sgs.ai_explicit[name]=="rebelish" then
		sgs.ai_assumed["rebel"]=sgs.ai_assumed["rebel"]+1
	elseif sgs.ai_explicit[name]=="rebel" then
		sgs.ai_assumed["rebel"]=sgs.ai_assumed["rebel"]+1
	end

	if (sgs.ai_renegade_suspect[name] or 0) > 1 then
		if intention >0 then sgs.ai_explicit[name] = "loyalish" sgs.ai_assumed["loyalist"] = sgs.ai_assumed["loyalist"] - 1
		elseif intention < 0 then sgs.ai_explicit[name] = "rebelish" sgs.ai_assumed["rebel"] = sgs.ai_assumed["rebel"] -1 end
		return
	end
	
	if sgs.ai_loyalty[name]<=-160 then
		sgs.ai_assumed["rebel"]=sgs.ai_assumed["rebel"]-1
		sgs.ai_explicit[name]="rebel"
		sgs.ai_loyalty[name]=-160
	elseif sgs.ai_loyalty[name]<=-70 then
		sgs.ai_assumed["rebel"]=sgs.ai_assumed["rebel"]-1
		sgs.ai_explicit[name]="rebelish"
	elseif sgs.ai_loyalty[name]>=160 then
		sgs.ai_assumed["loyalist"]=sgs.ai_assumed["loyalist"]-1
		sgs.ai_explicit[name]="loyalist"
		sgs.ai_loyalty[name]=160
	elseif sgs.ai_loyalty[name]>=70 then
		sgs.ai_assumed["loyalist"]=sgs.ai_assumed["loyalist"]-1
		sgs.ai_explicit[name]="loyalish"
	end
	--self:printAll(player, intention)
	--self:checkMisjudge(player)
end

function updateIntention(from, to, intention)
	updateLoyalty(from, sgs.ai_card_intention.general(to, intention))
	if to:isLord() then sgs.ai_anti_lord[from:objectName()] = sgs.ai_anti_lord[from:objectName()] + 1 end
end

function updateIntentions(from, tos, intention)
	for _, to in ipairs(tos) do
		if from:objectName() ~= to:objectName() then
			updateIntention(from, to, intention)
		end
	end
end

sgs.ai_card_intention.Indulgence = 120

sgs.ai_card_intention.SupplyShortage = 120

sgs.ai_card_intention["Slash"]=function(card,from,tos,source)
	for _, to in ipairs(tos) do
		if sgs.ai_liuli_effect then
			sgs.ai_liuli_effect=false
			return 0
		end
		local modifier=0
		if sgs.ai_collateral then sgs.ai_collateral=false modifier=-40 end
		local value=sgs.ai_card_intention.general(to,80+modifier)

		if sgs.ai_leiji_effect then
			if from and from:hasSkill("liegong") then return 0 end
			sgs.ai_leiji_effect = false
			return -value/1.5
		end
		if sgs.ai_pojun_effect then
			sgs.ai_pojun_effect=false
			if to:getHandcardNum()>5 then return -value end
		end
		speakTrigger(card,from,to)
		if to:hasSkill("yiji") then 
			return value*(2-to:getHp())/1.1
		end
		updateIntention(from, to, value)
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
	updateIntentions(from, tos, 80)
end

sgs.ai_card_intention.Collateral = 80

sgs.ai_card_intention.FireAttack = 80

sgs.ai_card_intention["IronChain"]=function(card,from,tos,source)
	for _, to in ipairs(tos) do
		if to:isChained() then
			updateIntention(from, to, 80)
		else 
			updateIntention(from, to, -80)
		end
	end
end

sgs.ai_card_intention["Dismantlement"]=function(card,from,tos,source)
	for _, to in ipairs(tos) do
		if to:getCards("j"):isEmpty() and
			not (to:getArmor() and (to:getArmor():inherits("GaleShell") or to:getArmor():inherits("SilverLion"))) then
			updateIntention(from, to, 80)
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

sgs.ai_card_intention["HuangtianCard"]=function(card,from,to,source)
	sgs.ai_lord_tolerance[from:objectName()]=(sgs.ai_lord_tolerance[from:objectName()] or 0)+1
	updateIntention(from, to, -80)
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

sgs.ai_card_intention["QiaobianCard"] = function(card, from, to, source)
	if from:getPhase() == sgs.Player_Draw then
		sgs.ai_card_intention["TuxiCard"](card, from, to, source)
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

--[[function SmartAI:printAll(player, intention)
	local name = player:objectName()
	self.room:writeToConsole(player:getGeneralName() .. math.floor(intention*10)/10 .. " Z" .. sgs.ai_assumed["loyalist"] .. " F" .. 
	sgs.ai_assumed["rebel"]	.. " N" .. sgs.ai_assumed["renegade"] .. " S" .. (self:singleRole() or "nil") .. " L" 
	.. (math.floor(sgs.ai_loyalty[name]*10)/10 or 0) .. " A" .. (sgs.ai_anti_lord[name] or 0) .. " RS" ..	(sgs.ai_renegade_suspect[name] or 0)
	.. " E" .. (sgs.ai_explicit[name] or "nil"))
end

function SmartAI:checkMisjudge(player)
	local name = player:objectName()
	if ((sgs.ai_explicit[name] == "loyalist" or sgs.ai_explicit[name] == "loyalish") and player:getRole() == "rebel")
		or ((sgs.ai_explicit[name] == "rebel" or sgs.ai_explicit[name] == "rebelish") and player:getRole() == "loyalist")
		or ((sgs.ai_renegade_suspect[name] or 0)> 2 and player:getRole() ~= "renegade")
		then
		self.room:writeToConsole("<font color='yellow'>MISJUDGE</font> <font color='white'>" .. player:getGeneralName() .. " " .. 
			sgs.ai_explicit[name] .. " " .. player:getRole() .. " " .. player:getRoom():alivePlayerCount() .. (sgs.ai_renegade_suspect[name] or 0)
			.. "</font>")
	end
end

function SmartAI:printAssume()
	self.room:output(sgs.ai_assumed["rebel"])
	self.room:output(sgs.ai_assumed["loyalist"])
	self.room:output("----")
end

function SmartAI:singleRole()
	local roles=0
	local theRole
	local selfexp=sgs.ai_explicit[self.player:objectName()]
	if selfexp=="loyalish" then selfexp="loyalist"
	elseif selfexp=="rebelish" then selfexp="rebel"
	end
	local selftru=self.role
	
	if (self.role~="lord") and (self.role~="renegade") then sgs.ai_assumed[self.role]=sgs.ai_assumed[self.role]-1 end
	if selfexp then sgs.ai_assumed[selfexp]=(sgs.ai_assumed[selfexp] or 0)+1 end
		
	
	if sgs.ai_assumed["rebel"]>0 then
		roles=roles+1
		theRole="rebel"
	end
	if sgs.ai_assumed["loyalist"]>-1 then
		roles=roles+1
		theRole="loyalist"
	end
	
	if (self.role~="lord") and (self.role~="renegade") then sgs.ai_assumed[self.role]=sgs.ai_assumed[self.role]+1 end
	if selfexp then sgs.ai_assumed[selfexp]=sgs.ai_assumed[selfexp]-1 end
	
	
	if roles==1 then
		if sgs.ai_assumed["loyalist"]==sgs.ai_assumed["rebel"] then return nil end
		return theRole
	end
	return nil
end

function SmartAI:printCards(cards)
	local string=""
	for _,card in ipairs(cards) do
		string=string.." "..card:className()
	end
	self.room:output(string)
end
]]