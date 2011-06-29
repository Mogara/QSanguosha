function SmartAI:fillTarget(card,selected,moves)
		if card:targetsFeasible(selected,self.player) then
		
			local move=sgs.CardUseStruct()
			move.from=self.player
			move.card=card
			local prohibit=false
			for _,aplayer in sgs.qlist(selected) do
				move.to:append(self:getPlayer(aplayer:objectName()))
				if self.room:isProhibited(move.from,aplayer,move.card) then prohibit=true break end
			end
			if not prohibit then table.insert(moves,move) end
		end
			local players=self.room:getAlivePlayers()
			for _,aplayer in sgs.qlist(players) do
				if card:targetFilter(selected,aplayer,self.player) and not selected:contains(aplayer) then
					selected:append(aplayer)
					self:fillTarget(card,selected,moves)
					selected:removeAt(selected:length()-1)
				end
			end
		
	end

	
	
function SmartAI:fillSkillMove(hand)

	local function fillcards(sHand,selected,skill)
		if skill:viewAs(selected) then 
			end
		for _,eachHand in sgs.qlist(hand) do end
		
	end

	local skills=self.player:getVisibleSkillList()
	local sHand=sgs.CardList()
	
	for _,skill in sgs.qlist(skills) do
		fillcards(sHand,sgs.CardItemList(),skill)
	end
	for _,card in sgs.qlist(sHand) do
		hand:append(card)
	end
end


function SmartAI:moveGenerate()
	local moves={}
	self.allMoves=moves
	local hand=self.player:getCards("h")
	hand=sgs.QList2Table(hand)
	--self:fillSkillMove(hand)
	self:fillSkillCards(hand)
	
	
	
	local players=self.room:getAlivePlayers()
	
	
	for _,card in ipairs(hand) do
	
		if card:inherits("SkillCard") then 
			local use=sgs.CardUseStruct()
			use.from=self.player
			sgs.ai_skill_use_func[card:className()](card,use,self)
			if use.card then table.insert(moves,use) end
		elseif card:isAvailable(self.player) then
		
			if card:targetFixed() then 
				local use=sgs.CardUseStruct()
				use.from=self.player
				use.card=card
				table.insert(moves,use)
			else
				self:fillTarget(card,sgs.PlayerList(),moves)
			end
		end
	end
	
	return moves
end

function SmartAI:getMove()
	local moves=self:moveGenerate()
	local goodMoves={}
	
	self:log("*****************************")
	--self:printMoves(moves)
	--self:historianReport()
	self:log("--------------------------")
	
	for _,move in ipairs(moves) do
		local toRemove={}
		local isGood=self:choiceEvaluate(move)>0
		for i=1,#goodMoves do
			local amove=goodMoves[i]
			if not isGood then break end
			
			if self:isExclusiveMove(move,amove) then
				if self:choiceCompare(move,amove) then 
					table.insert(toRemove,i)
				else isGood=false break end
			end
		end
		if isGood then
			for i=#toRemove,1,-1 do table.remove(goodMoves,toRemove[i]) end
			table.insert(goodMoves,move)
		end
	end
	
	--self:printMoves(goodMoves)
	--self:log("-_-_-_-_-_-_-_-_-_-_-_-_-_-")
	self:moveSequenceSort(goodMoves)
	if #goodMoves<1 then return nil end
	return goodMoves[1]
end

function SmartAI:isExclusiveMove(a,b)
	return a.card:getId()==b.card:getId()
end

function SmartAI:choiceCompare(a,b)
	return self:choiceEvaluate(a)>self:choiceEvaluate(b)
end

function SmartAI:moveSequenceSort(moves)
	local comp=function (a,b)
		return self:sequenceEvaluate(a)>self:sequenceEvaluate(b)
	end
	table.sort(moves,comp)
end
	
function SmartAI:printMoves(moves)
	for _,move in ipairs(moves) do
		if type(move.from)~="string" then
			for _,to in sgs.qlist(move.to) do		
				self:log(move.card:objectName()..":"..move.from:getGeneralName().."->"..to:getGeneralName())
			end
			if move.to:length()==0 then self:log(move.card:objectName()..":"..move.from:getGeneralName()) end
		else
			for _,to in ipairs(move.to) do		
				self:log(move.card:objectName()..":"..move.from.."->"..to)
			end
		end
	end
end

function SmartAI:choiceEvaluate(move)
	local value=0
	local total_imp=0
	for _,guide in ipairs(sgs.ai_strategy_choice) do
		if guide.match(self,move) then
		
			if guide.evaluate then guide.value=guide.evaluate(self,move) end
			
			value=(value*total_imp+guide.value*guide.imp)/(total_imp+guide.imp)
			total_imp=total_imp+guide.imp
			
			guide.applied=true
			
			local function nullify(override)
				for _,overridden in ipairs(override) do
					if not overridden.value then
						nullify(overridden)
					elseif overridden.applied then
						if total_imp>overridden.imp then 
							value=(value*total_imp-overridden.value*overridden.imp)/(total_imp-overridden.imp)
						else
							value=0
						end
						total_imp=total_imp-overridden.imp
						
						overridden.applied=false
					end
				end
			end
			
			nullify(guide.override)
			
		end
	end
	return value
end

function SmartAI:sequenceEvaluate(move)
	local value=0
	local total_imp=0
	for _,guide in ipairs(sgs.ai_strategy_sequence) do
		guide.applied=false
		if guide.match(self,move) then
		
			if guide.evaluate then guide.value=guide.evaluate(self,move) end
			
			value=(value*total_imp+guide.value*guide.imp)/(total_imp+guide.imp)
			total_imp=total_imp+guide.imp
			
			guide.applied=true
			
			local function nullify(override)
				for _,overridden in ipairs(override) do
					if not overridden.value then
						nullify(overridden)
					elseif overridden.applied then
						if total_imp>overridden.imp then 
							value=(value*total_imp-overridden.value*overridden.imp)/(total_imp-overridden.imp)
						else
							value=0
						end
						total_imp=total_imp-overridden.imp
						
						overridden.applied=false
					end
				end
			end
			
			nullify(guide.override)
			
		end
	end
	return value
end

sgs.newStrategy=function(spec)
	local guide={}
	guide.match=spec.match
	guide.value=spec.value
	guide.evaluate=spec.evaluate
	guide.imp=spec.importance or 5
	guide.override=spec.override or {}
	if spec.isSequence then table.insert(sgs.ai_strategy_sequence,guide)
	else table.insert(sgs.ai_strategy_choice,guide) end
	return guide
end

sgs.ai_strategy_sequence={}
sgs.ai_strategy_choice={}

sgs.ai_standard={}



dofile "lua/ai/sequence-ai.lua"
dofile "lua/ai/choice-ai.lua"

dofile "lua/ai/intention-filter.lua"