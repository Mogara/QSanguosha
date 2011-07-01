function SmartAI:intentionFilter(event,player,data)
	if event==sgs.ChoiceMade then
	
		local decisionType
	
		local info=data:toCardUse()
		if info.from then 
			if not info.card then return end
			decisionType=decisionType or "cardUse"
		end
		
		if not info.from then 
			info=data:toString() 
			decisionType=decisionType or info:split(":")[1]
			--self:log(info)
		end
		
		
		for _,item in ipairs(sgs.ai_tracker) do
			if item.match(decisionType,player,data) then
				item.result(self,player,data)
			end
		end
		
		
	end
	
	if event==sgs.Death then
		sgs.ai_transMX(sgs.role_mx,player,player:getRole(),10)
	end
	
	-- if event==sgs.TurnStart then
		-- for _,item in ipairs(sgs.ai_tracker) do
			-- if item.match("turnStart",player,data) then
				-- item.result(self,player,data)
			-- end
		-- end
	-- end
end

function SmartAI:rememberToList(entry,player)
	local name
	if player then name=player:objectName()
	else name="general" end
	if not self.historian[name][entry.field] then 
		self.historian[name][entry.field]={}
	end
	table.insert(self.historian[name][entry.field],entry.content)
	if #self.historian[name][entry.field]>10 then 
		table.remove(self.historian[name][entry.field],1)
	end
end

function SmartAI:remember(entry,player)
	local name
	if player then name=player:objectName()
	else name="general" end
	self.historian[name][entry.field]=entry.content
end

function SmartAI:recall(field,player)
	local name
	if player then name=player:objectName()
	else name="general" end
	return self.historian[name][field]
end

 sgs.ai_tracker={}
sgs.historians={}
sgs.newHistorian=function(player)
	local historian={}
	for _,aplayer in sgs.qlist(player:getRoom():getAllPlayers()) do
	
		historian[aplayer:objectName()]={}

	end
	historian["general"]={}
	sgs.historians[player:objectName()]=historian
	return historian
end

function SmartAI:historianReport()
	for _,aplayer in sgs.qlist(self.room:getAllPlayers()) do
	for _,bplayer in sgs.qlist(self.room:getAllPlayers()) do
	
	if self.historian[aplayer:objectName()][bplayer:objectName()] then
		local astring=aplayer:objectName()..":"..bplayer:objectName().."="
		for _,value in ipairs(self.historian[aplayer:objectName()][bplayer:objectName()]) do
			astring=astring..","..value
		end
		self:log(astring)
	end

	end
	end
	
	if not sgs.role_mx then return end
	for _,aplayer in sgs.qlist(self.room:getAlivePlayers()) do
		
			self:log(
			math.ceil((sgs.role_mx.rebel[aplayer:objectName()] or 0)*100)/100 ..
			" ; "..
			math.ceil((sgs.role_mx.renegade[aplayer:objectName()] or 0)*100)/100 ..
			" ; "..
			math.ceil((sgs.role_mx.loyalist[aplayer:objectName()] or 0)*100)/100 ..
			" : "..
			aplayer:getGeneralName()
			)
		
	end
end

sgs.newGuessing=function(spec)
	local guessing={}
	guessing.match=spec.match
	guessing.result=spec.result
	table.insert(sgs.ai_tracker,guessing)
	return guessing
end

sgs.ai_intention_carduse_mapping=
{
Indulgence="Jeopardize",
SupplyShortage="Jeopardize",
LeijiCard="Attack",
RendeCard="Support",
QingnangCard="Save",
JieyinCard="Save",
HuangtianCard="Support",
JiemingCard="Support",
HaoshiCard="Support",
FanjianCard="Attack",
TianyiCard="Jeopardize",
QuhuCard="Jeopardize",
JujianCard="Support",
MingceCard="Support",
XianzhenCard="Jeopardize",
Slash="Attack",
FireSlash="Attack",
ThunderSlash="Attack",
Peach="Save",
Duel="Attack",
Collateral="Disable",
FireAttack="Attack",
TuxiCard="Jeopardize",
ShensuCard="Attack",
QiangxiCard="Attack",
QuhuCard="Jeopardize",
JujianCard="Support",
MingceCard="Support",
}

--cardUse
sgs.newGuessing{
	match=function(decisionType,player,data)
		return decisionType=="cardUse"
	end,
	result=function(self,player,data)
		local info=data:toCardUse()
		
		for _,target in sgs.qlist(info.to) do
			self:rememberToList({field=target:objectName(),content=info.card:className()},player)
		end
	end
}

--cardUse intention

function sgs.interpreteMove(player,data)
	local use=data:toCardUse()
	local intention=sgs.ai_intention_carduse_mapping[use.card:className()]
	return intention
end

sgs.newGuessing{
	match=function(decisionType,player,data)
		return decisionType=="cardUse"
	end,
	result=function(self,player,data)
		if self~=sgs.recorder then return end
		local use=data:toCardUse()
		local intention=sgs.interpreteMove(player,data)
		
		if not sgs.role_mx then 
			sgs.role_mx={}
			local mx=sgs.role_mx
			mx.rebel={}
			mx.renegade={}
			mx.loyalist={}
			
			local remains={}
			for _,aplayer in sgs.qlist(player:getRoom():getAllPlayers()) do
				if not aplayer:isLord() then 
					remains[aplayer:getRole()]=(remains[aplayer:getRole()] or 0)+1
				end
			end
			
			for _,aplayer in sgs.qlist(player:getRoom():getAllPlayers()) do
					
				if not aplayer:isLord() then 	
					mx.rebel[aplayer:objectName()]=remains.rebel
					mx.loyalist[aplayer:objectName()]=remains.loyalist
					mx.renegade[aplayer:objectName()]=remains.renegade
				end
			end
			
			
		end
		
		if player:isLord() then return end
			
		local function transfer(mx,from,fr,to,tr,value)
			mx[fr][from:objectName()]=mx[fr][from:objectName()]+value
			mx[tr][to:objectName()]  =mx[tr][to:objectName()]  +value
			mx[fr][to:objectName()]  =mx[fr][to:objectName()]  -value
			mx[tr][from:objectName()]=mx[tr][from:objectName()]-value
		end

		local function findRoom(mx,from,fr,to,tr,inc)
			local v
			local sum=self.player_sum
			if inc then
				v=sum-mx[tr][to:objectName()]
				if v>mx[tr][from:objectName()] then v=mx[tr][from:objectName()] end
				if v>mx[fr][to:objectName()] then v=mx[fr][to:objectName()] end
			else 
				v=sum-mx[tr][to:objectName()]
				if v<mx[tr][from:objectName()] then v=mx[tr][from:objectName()] end
				if v<mx[fr][to:objectName()] then v=mx[fr][to:objectName()] end
				v=sum-v
			end
			if v>0 then return v else return 0 end
		end	
			
		local function transMX(mx,player,role,value)
			local roles={"rebel","renegade","loyalist"}
			local sum=self.player_sum
			
			if mx[role][player:objectName()]+value>sum then value=sum-mx[role][player:objectName()] end
			if mx[role][player:objectName()]+value<0   then value=-mx[role][player:objectName()] end
			
			
				local room=0
				for _,arole in ipairs(roles) do
					for _,aplayer in sgs.qlist(player:getRoom():getAlivePlayers()) do
						if not aplayer:isLord() then
							if not (arole==role or aplayer:objectName()==player:objectName()) then
								room=room+findRoom(mx,player,role,aplayer,arole,value>0)
							end
							
						end
					end
				end
				
				if room<=0 then return end
				
				for _,arole in ipairs(roles) do
					for _,aplayer in sgs.qlist(player:getRoom():getAlivePlayers()) do
						if not aplayer:isLord() then
							if not (arole==role or aplayer:objectName()==player:objectName()) then
								local transValue=findRoom(mx,player,role,aplayer,arole,value>0)*value/room
								transfer(mx,player,role,aplayer,arole,transValue)
							end
							
						end
					end
				end
			
			
		end
		
		sgs.ai_transMX=transMX
		
		
		
		
		for _,target in sgs.qlist(use.to) do
			
			if intention=="Attack" or intention=="Jeopardize" then
				if target:isLord() then transMX(sgs.role_mx,player,"rebel",10) 
				else
				
				local sum=self.player_sum
				local v=sgs.role_mx.rebel[target:objectName()]-sum/2
				if v>3 then v=3
				elseif v<-1 then v=-1 end 
				transMX(sgs.role_mx,player,"rebel",-v)
				v=sgs.role_mx.loyalist[target:objectName()]-sum/2
				if v>3 then v=3
				elseif v<-1 then v=-1 end 
				transMX(sgs.role_mx,player,"loyalist",-v)
				
				end
			elseif intention=="Save" or intention=="Support" then 
				if target:isLord() then transMX(sgs.role_mx,player,"rebel",-10) 
				else
				
				local sum=self.player_sum
				local v=sgs.role_mx.rebel[target:objectName()]-sum/2
				if v>1 then v=1
				elseif v<-3 then v=-3 end 
				transMX(sgs.role_mx,player,"rebel",v)
				v=sgs.role_mx.loyalist[target:objectName()]-sum/2
				if v>1 then v=1
				elseif v<-3 then v=-3 end 
				transMX(sgs.role_mx,player,"loyalist",v)
				
				end
			end
			
			
		end
		
		local roles={"rebel","renegade","loyalist"}
		local mostRole="rebel"
		
		for _,arole in ipairs(roles) do
			if (not mostRole) or sgs.role_mx[mostRole][player:objectName()]< sgs.role_mx[arole][player:objectName()] then
			mostRole=arole
			end
		end
		
		local mx=sgs.role_exp or {}
		mx[player:objectName()]=mostRole
		sgs.role_exp=mx
	end,
}

--nullification
sgs.newGuessing{
	match=function(decisionType,player,data)
		return decisionType=="cardUse" and data:toCardUse().card:inherits("Nullification")
	end,
	result=function(self,player,data)
		local target=self.room:getTag("NullifyingTarget"):toPlayer()
		local source=self.room:getTag("NullifyingSource"):toPlayer()
		local card= self.room:getTag("NullifyingCard"):toCard()
		local num=self.room:getTag("NullifyingTimes"):toInt()
		
		local text="Nullification="..card:className()..":"
		if source then text=text..source:objectName().."->" end
		text=text..target:objectName()
		
		self:rememberToList({field=target:objectName(),content=text.."_"..num},player)
	end
}

--cardChosen
sgs.newGuessing{
	match=function(decisionType,player,data)
		return decisionType=="cardChosen"
	end,
	result=function(self,player,data)
		local info=data:toString():split(":")
		local target=self.room:getCardOwner(info[3])
		local place=self.room:getCardPlace(info[3])
		
			self:rememberToList({field=target:objectName(),content=info[2].."="..info[3]..":"..place},player)
		
	end
}

--peach
sgs.newGuessing{
	match=function(decisionType,player,data)
		return decisionType=="peach"
	end,
	result=function(self,player,data)
		local info=data:toString():split(":")
		local target=self:getPlayer(info[2])
		
			self:rememberToList({field=target:objectName(),content="peach"},player)
		
	end
}


-- sgs.ai_intention_cardeffect_mapping={

-- }

-- sgs.newGuessing{
	-- match=function(event,info,from,to)
		-- return event==sgs.CardEffect
	-- end,
	-- result=function(event,info,from,to)
		-- local name=info:className()
		-- return {field=to:objectName(),value=sgs.ai_intention_cardeffect_mapping[name]}
	-- end
-- }