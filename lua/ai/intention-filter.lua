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
			self:log(info)
		end
		
		
		for _,item in ipairs(sgs.ai_tracker) do
			if item.match(decisionType,player,data) then
				item.result(self,player,data)
			end
		end
		
		
	end
end

function SmartAI:rememberToList(player,entry)
	local name=player:objectName()
	if not self.historian[name][entry.field] then 
		self.historian[name][entry.field]={}
	end
	table.insert(self.historian[name][entry.field],entry.content)
	if #self.historian[name][entry.field]>10 then 
		table.remove(self.historian[name][entry.field],1)
	end
end

function SmartAI:remember(player,entry)
	local name=player:objectName()
	self.historian[name][entry.field]=entry.content
end

function SmartAI:recall(player,field)
	return self.historian[player:objectName()][field]
end

 sgs.ai_tracker={}
sgs.historians={}
sgs.newHistorian=function(player)
	local historian={}
	for _,aplayer in sgs.qlist(player:getRoom():getAllPlayers()) do
	
		historian[aplayer:objectName()]={}

	end
	
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
Indulgence="Disable",
SupplyShortage="Disable",
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
}

sgs.newGuessing{
	match=function(decisionType,player,data)
		return decisionType=="cardUse"
	end,
	result=function(self,player,data)
		local info=data:toCardUse()
		
		for _,target in sgs.qlist(info.to) do
			self:rememberToList(player,{field=target:objectName(),content=info.card:className()})
		end
	end
}

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
		
		self:rememberToList(player,{field=target:objectName(),content=text.."_"..num})
	end
}

sgs.newGuessing{
	match=function(decisionType,player,data)
		return decisionType=="cardChosen"
	end,
	result=function(self,player,data)
		local info=data:toString():split(":")
		local target=self.room:getCardOwner(info[3])
		local place=self.room:getCardPlace(info[3])
		
			self:rememberToList(player,{field=target:objectName(),content=info[2].."="..info[3]..":"..place})
		
	end
}

sgs.newGuessing{
	match=function(decisionType,player,data)
		return decisionType=="peach"
	end,
	result=function(self,player,data)
		local info=data:toString():split(":")
		local target=self:getPlayer(info[2])
		
			self:rememberToList(player,{field=target:objectName(),content="peach"})
		
	end
}

-- sgs.ai_intention_cardeffect_mapping={
-- Slash="Attack",
-- FireSlash="Attack",
-- ThunderSlash="Attack",
-- Peach="Save",
-- Duel="Attack",
-- Collateral="Disable",
-- FireAttack="Attack",
-- IronChain="Jeopardize",
-- TuxiCard="Jeopardize",
-- ShensuCard="Attack",
-- QiangxiCard="Attack",
-- QuhuCard="Jeopardize",
-- JujianCard="Support",
-- MingceCard="Support",
-- XianzhenCard="Jeopardize",
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