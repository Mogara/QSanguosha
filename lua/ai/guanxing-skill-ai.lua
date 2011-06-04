function judgecheck(target,reason,card)
	local cardSet={}

	cardSet.club={}
	cardSet.spade={}
	cardSet.heart={}
	cardSet.diamond={}

	
	if reason=="indulgence" then
		fillCardSet(cardSet,"heart",true)
	elseif reason=="supply_shortage" then
		fillCardSet(cardSet,"club",true)
	elseif reason=="eight_diagram" then
		fillCardSet(cardSet,"heart",true)
		fillCardSet(cardSet,"diamond",true)
	elseif reason=="luoshen" then
		fillCardSet(cardSet,"spade",true)
		fillCardSet(cardSet,"club",true)
	elseif reason=="lightning" then
		fillCardSet(cardSet,"heart",true)
		fillCardSet(cardSet,"club",true)
		fillCardSet(cardSet,"diamond",true)
		fillCardSet(cardSet,"spade",false)
		fillCardSet(cardSet,nil,nil,1,true)
		for i=10,13 do 
			fillCardSet(cardSet,nil,nil,i,true)
		end
	end

	if  target:hasSkill("tianxiang") and card:getSuitString()=="spade" then
		local changedcard = sgs.Card_Parse(("%s[%s:%s]"):format(card:objectName(),"heart", card:getNumber()))
		return goodMatch(cardSet,changedcard)
	end

	return goodMatch(cardSet,card)
end

function Card2Judge(card)
	local card_name=card:objectName()
	local card_suit=card:getSuitString()
	if card:inherits("DelayedTrick") then return card_name
	elseif (card_suit == "club") or (card_suit == "spade") then return "supply_shortage"
	elseif card_suit == "diamond" then return "indulgence" end
	
	return card_name
end

function solveValue(judge,player)
	if judge == "lightning" then return 4 end
	if judge == "supply_shortage" then
		--if player:getHp()*2 + player:getHandcardNum()<6 then
		if player:getHp() >= player:getHandcardNum() then
			return 2
		else
			return 1
		end
	end
	if judge == "indulgence" then
		--if player:getHp()*2 + player:getHandcardNum()<6 then
		if player:getHp() >= player:getHandcardNum() then
			return 1
		else
			return 2
		end
	end
	
	return 1
end

--This Guanxing strategy is too dirty to being rebuilt in the future...
function zhugeliang_guanxing(self,cards,up_only)
	local nextplayer=self.room:nextPlayer()
	local self_judge=sgs.QList2Table(self.player:getCards("j"))
	local next_judge=sgs.QList2Table(nextplayer:getCards("j"))
	local up,bottom={},{}

	local anticipation={}
	
	
	--Build up the anticipation matrix(n,3)
	self:log("Build up the anticipation matrix")
	local lightning_exist=false
	local self_need_judge, next_need_judge = 0,0
	local self_judge_left, next_judge_left = 0,0
	local self_hcard_num , next_hcard_num = 2,2 -- Generally, the man can gain 2 handcard

	--self_judge:insert
	while (#self_judge>0) do
		local judge_string = Card2Judge(table.remove(self_judge))
		if judge_string == "lightning" then lightning_exist=true end
		table.insert(anticipation,{"self",judge_string,nil})
		self_need_judge = self_need_judge + 1
	end
	--self_handcard:insert
	for i=1,self_hcard_num do
		table.insert(anticipation,{"self","handcard",nil})
	end
	--next_judge:insert
	while(#next_judge>0) do
		if lightning_exist then
			table.insert(anticipation,{"next","lightning",nil})
			lightning_exist=false
		else
			local judge_string = Card2Judge(table.remove(next_judge))
			table.insert(anticipation,{"next",judge_string,nil})
		end
		next_need_judge = next_need_judge + 1
	end
	--next_handcard:insert
	for i=1,next_hcard_num do
		table.insert(anticipation,{"next","handcard",nil})
	end
	table.insert(anticipation,{"other","handcard",nil})
	--Because of the special strategy for zhenji's luoshen, the reason of the table may be incorrect.
	--It will make some unknown errors. For example, the next player is Zhangliao or Zhouyu, hahaha...
	
	self:log("Before Fixed:" .. #anticipation)
	while(#anticipation>#cards) do table.remove(anticipation) end
	self:log("After Fixed:" .. #anticipation)
	self:log("Cards quatily:" .. #cards)
	
	for i=1,#anticipation do self:log(i..":"..anticipation[i][2]) end
	--card_id to card
	local remains={}
    for i,id in ipairs(cards) do
		local card = sgs.Sanguosha:getCard(id)
        table.insert(remains,card)
		self:log(i..":"..card:getSuitString())
    end

	self:sortByUseValue(remains,true)

	self:log("-----------------------------------------------------")
	--try to solve the own judgements
	local self_will_judge = self_need_judge
	if self_need_judge>0 then
		self:log("self_need_judge")
		local solve_success_value,solve_value_sum = 0,0
		local success_judge = 0
		local cardlist={}
		for i=1,self_need_judge do
			table.insert(cardlist,{anticipation[i][2]})
		end
		local remains_dummy
		
		local secondEmpty,lightningAtLast,supply_shortage_work = false,false,false
		local supply_shortage_empty
		local isBest=function()		--it may be added other conditions into in the future
			if solve_success_value * 2 <= solve_value_sum then
				return false
			end
			if self_will_judge>1 and lightningAtLast then
				return false
			end
			return true
		end
		--get the best solution with the largest value
		while( not isBest() and self_will_judge>0 ) do
			self:log("self_will_judge:"..self_will_judge)
			solve_success_value,solve_value_sum = 0,0
			success_judge = 0

			remains_dummy={}
			for i=1,#remains do
				table.insert(remains_dummy,remains[i])
			end
			self:log(#remains_dummy..","..#remains)

			for i=1,self_will_judge do
				self:log("self_judge")
				local solve_value = solveValue(cardlist[i][1],self.player)

				solve_value_sum = solve_value_sum + solve_value
				for j=1,#remains_dummy do
					if judgecheck(self.player,cardlist[i][1],remains_dummy[j]) then
						cardlist[i][2]=table.remove(remains_dummy,j)
						solve_success_value = solve_success_value + solve_value
						success_judge = success_judge + 1
						self:log("found")
						break
					end
				end
				if i==2 and cardlist[i][2]==nil then
					secondEmpty = true
				end
				if i==self_will_judge and secondEmpty and cardlist[i][1]=="lightning" then
					lightningAtLast = true
				end
			end
			if cardlist[1][1] == "supply_shortage" and cardlist[1][2] == nil then
				supply_shortage_work = true
			end

			self:log("solve_success_value:"..solve_success_value)
			self:log("solve_value_sum :"..solve_value_sum)
			if not isBest() then self_will_judge = self_will_judge-1 end
		end

		if isBest() then
			if supply_shortage_work then self_hcard_num = 0 end
			--fixed self_will_judge
			if self.player:getHp()>=2 then
				while(cardlist[self_will_judge][2]==nil) do self_will_judge = self_will_judge - 1 end
			end
			for i=1,math.min(self_will_judge,#cards) do
				anticipation[i][3] = cardlist[i][2]
			end
			self:log(#remains_dummy..","..#remains)
			remains={}
			for i=1,#remains_dummy do
				table.insert(remains,remains_dummy[i])
			end
			self:log(#remains_dummy..","..#remains)
		end
		self_judge_left = self_will_judge - success_judge
	end
	
	local next_will_judge = next_need_judge
	if self_will_judge==self_need_judge then	--It means most of own judgements can be solved , generally all own judgements can be solved.
		--self_handcard
		self:sortByUseValue(remains,false)
		-- friend's judge
	
		if self:isFriend(nextplayer) then	--when the next player is a friend
			self:log("Friend")
			local cardlist={}
			if nextplayer:hasSkill("luoshen") and self_need_judge + self_hcard_num <#cards  then --the special strategy for zhenji's luoshen , its other judgements won't be solved by zhugeliang.
				local remains_dummy ={}
				for i=1,#remains do
					table.insert(remains_dummy,remains[i])
				end

				for j,card in ipairs(remains_dummy) do
					if judgecheck(nextplayer,"luoshen",card) then
						self:log("luoshen:found")
						for n = self_need_judge + self_hcard_num + 1 , #anticipation do
							if not anticipation[n][3] then
								self:log("before remains_dummy:"..#remains_dummy)
								anticipation[n][3] = table.remove(remains_dummy,j)
								self:log("after remains_dummy:"..#remains_dummy)
								break
							end
						end
					end
				end
				remains = {}
				for i=1,#remains_dummy do
					table.insert(remains,remains_dummy[i])
				end
			elseif next_need_judge > 0 then --try to solve the friend's judgement if the friend have no "luoshen".
				local solve_success_value,solve_value_sum = 0,0
				local success_judge = 0
				local cardlist={}
				--#cardlist + self_need_judge + self_hcard_num
				--   =
				--math.min(self_need_judge + self_hcard_num + next_need_judge , #anticipation)
				for i=self_need_judge + self_hcard_num + 1, math.min(self_need_judge + self_hcard_num + next_need_judge , #anticipation) do
					table.insert(cardlist,{anticipation[i][2]})
				end
				local remains_dummy
		
				local secondEmpty,lightningAtLast,supply_shortage_work = false,false,false
				local supply_shortage_empty
				local isBest=function()		--it may be add other conditions into in the future
					if solve_success_value * 2 <= solve_value_sum then
						return false
					end
					if next_will_judge>1 and lightningAtLast then
						return false
					end
					return true
				end

				--get the best solution with the best value
				while( not isBest() and next_will_judge>0 ) do
					self:log("next_will_judge:"..next_will_judge)
					solve_success_value,solve_value_sum = 0,0
					success_judge = 0

					remains_dummy={}
					for i=1,#remains do
						table.insert(remains_dummy,remains[i])
					end
					self:log(#remains_dummy..","..#remains)

					next_nedd_judge = math.min(next_need_judge , #anticipation - (self_need_judge + self_hcard_num + enemy_luoshen))
					for i=1,next_will_judge do
						self:log("next_judge")
						local solve_value = solveValue(cardlist[i][1],nextplayer)

						solve_value_sum = solve_value_sum + solve_value
						for j=1,#remains_dummy do
							if judgecheck(nextplayer,cardlist[i][1],remains_dummy[j]) then
								cardlist[i][2]=table.remove(remains_dummy,j)
								solve_success_value = solve_success_value + solve_value
								success_judge = success_judge + 1
								self:log("found")
								break
							end
						end
						if i==2 and cardlist[i][2]==nil then
							secondEmpty = true
						end
						if i==next_will_judge and secondEmpty and cardlist[i][1]=="lightning" then
							lightningAtLast = true
						end
					end

					self:log("solve_success_value:"..solve_success_value)
					self:log("solve_value_sum :"..solve_value_sum)
					if not isBest() then next_will_judge = next_will_judge-1 end
				end
				if isBest() then
					--fixed self_will_judge
					if nextplayer:getHp()>=2 then
						while(cardlist[next_will_judge][2]==nil) do next_will_judge = next_will_judge - 1 end
					end
					for i=1,next_will_judge do
						anticipation[i + self_need_judge + self_hcard_num][3] = cardlist[i][2]
					end
					self:log(#remains_dummy..","..#remains)
					remains={}
					for i=1,#remains_dummy do
						table.insert(remains,remains_dummy[i])
					end
					self:log(#remains_dummy..","..#remains)
				end
				next_judge_left = next_will_judge - success_judge
			end
			self:log("self_hcard")
			for n = self_need_judge + 1,math.min(self_need_judge + self_hcard_num,#anticipation) do
				anticipation[n][3] = table.remove(remains)
				self:log("self_handcard:"..n)
			end
		else	--when the next player is not a friend.(emeny or neutrality)
			self:log("Not friend")
			--Get the card with the best value at first
			if self.player:getHp()<2 then
				self:sortByKeepValue(remains,false)
			else
				self:sortByUseValue(remains,false)
			end

			self:log("self_hcard")
			for n = self_need_judge + 1,math.min(self_need_judge + self_hcard_num,#anticipation) do
				anticipation[n][3] = table.remove(remains)
				self:log("self_handcard:"..n)
			end
			
			local enemy_luoshen = 0
			if nextplayer:hasSkill("luoshen") and self_need_judge + self_hcard_num <#anticipation then
				local remains_dummy ={}
				for i=1,#remains do
					table.insert(remains_dummy,remains[i])
				end

				for j,card in ipairs(remains_dummy) do
					if not judgecheck(nextplayer,"luoshen",card) then
						self:log("luoshen:found")
						for n = self_need_judge + self_hcard_num + 1 , #anticipation do
							if not anticipation[n][3] then
								self:log("before remains_dummy:"..#remains_dummy)
								anticipation[n][3] = table.remove(remains_dummy,j)
								self:log("after remains_dummy:"..#remains_dummy)
								break
							end
						end
						enemy_luoshen = 1
						break --One red card is enough.
					end
				end
				remains = {}
				for i=1,#remains_dummy do
					table.insert(remains,remains_dummy[i])
				end
			end
			if next_need_judge > 0 and self_need_judge + self_hcard_num + enemy_luoshen <#anticipation then
				next_nedd_judge = math.min(next_need_judge , #anticipation - (self_need_judge + self_hcard_num + enemy_luoshen))
				for i=1,next_will_judge do
					self:log("next_judge")
					--self_need_judge + self_hcard_num + enemy_luoshen
					local found = false
					for j=1,#remains do
						if not judgecheck(nextplayer,anticipation[i + self_need_judge + self_hcard_num][2],remains[j]) then
							anticipation[i + self_need_judge + self_hcard_num + enemy_luoshen][3] = table.remove(remains,j)
							self:log("found")
							found = true
							break
						end
					end
					if not found then break end
				end
			end
		end
	end


	--insert the empties
	self:sortByUseValue(remains,true)
	for i=1,self_judge_left + next_judge_left do
		for j=1,#anticipation do
			if not anticipation[j][3] then
				self:log(i..":anticipation["..j.."]")
				anticipation[j][3]=table.remove(remains)
				break
			end
		end
	end
	
	local k=1
	while(k <= #anticipation and anticipation[k][3]) do
		table.insert(up,anticipation[k][3]:getEffectiveId())
		self:log("insert into 'up':"..k)
		k=k+1
	end
	self:log("remains:"..#remains)
	k=1
	for _,card in ipairs(remains) do
		table.insert(bottom,card:getEffectiveId())
		self:log("insert into 'bottom':"..k)
		k=k+1
	end

	if #up + #bottom ~= #cards then
		self:log("Default Guanxing")
		up=cards
		bottom={}
	end
	self:log("Over")
	return up,bottom
end

function xinzhan_askForAG(self,card_ids,refusable,reason)
    local ids=card_ids
    local cards={}
    for i,id in ipairs(ids) do
		card=sgs.Sanguosha:getCard(id)
		self:log(i..":"..card:objectName())
        table.insert(cards,sgs.Sanguosha:getCard(id))
    end

	local nextplayer=self.room:nextPlayer()
	local next_judge=sgs.QList2Table(nextplayer:getCards("j"))

	local ExNihilo_num = 0

	local cardlist={
		spade={},
		heart={},
		club={},
		diamond={}
		}

	for i,card in ipairs(cards) do
		if card:inherit("ExNihilo") then ExNihilo_num = ExNihilo_num + 1 end
		if #cards - ExNihilo_num <= 1 then --当三张牌中有两张或以上的无中生有，将触发条件
			return card:getEffectiveId()
		end
		table.insert(cardlist[card:getSuitString()],cards[i])
	end

	if #cardlist["heart"]==0 then return nil end	--没有红桃牌，直接跳出


	--检查手牌有没有桃
	if self:getHp()<=1 then
		local has_pease = false
		for _, card in sgs.qlist(cards) do
			if card:inherits("Peach") then
				has_pease = true
				break
			end
		end
		if not has_pease then
			for i,heart in ipairs(cardlist["heart"]) do
				if heart:inherit("Peach") then
					return cardlist["heart"][i]:getEffectiveId()
				end
			end
		end
	end

	--预判
	local anticipation={}
	
	local luoshen_num,next_need_judge = 0
	
	if nextplayer:hasSkill("luoshen") then
		if self:isFriend(nextplayer) then
			for i=1,#cardlist["spade"]+#cardlist["club"] do
			table.insert(anticipation,{"next","luoshen",nil})
			luoshen_num = luoshen_num + 1
			end
		end
		table.insert(anticipation,{"next","luoshen",nil})
		luoshen_num = luoshen_num + 1
	end
	
	while(#next_judge>0) do
		local judge_string = Card2Judge(table.remove(next_judge))
		table.insert(anticipation,{"next",judge_string,nil})
		next_need_judge = next_need_judge + 1
	end
	
	while(#anticipation>#cards) do table.remove(anticipation) end
	
	--以下为至少有一张红桃牌的情况
	--说明：
	--当下家队友有洛神：luoshen_num>=1
	--当下家敌人有洛神:luoshen_num==1
	--#anticipation <= #cards <= luoshen_num + next_need_judge
	--目前next_need_judge和luoshen_num还没用到？
	
	if self:isFriend(nextplayer) then --队友

		local remains_dummy ={}

		for i=1,#cardlist["club"] do
			table.insert(remains_dummy,table.remove(cardlist["club"]))
		end
		for i=1,#cardlist["spade"] do
			table.insert(remains_dummy,table.remove(cardlist["spade"]))
		end
		for i=1,#cardlist["diamond"] do
			table.insert(remains_dummy,table.remove(cardlist["diamond"]))
		end
		for i=1,#cardlist["heart"] do
			table.insert(remains_dummy,table.remove(cardlist["heart"]))
		end

		for j,card in ipairs(remains_dummy) do
			for n = 1 , #anticipation do
				if judgecheck(nextplayer,anticipation[n][2],card) then
					anticipation[n][3] = table.remove(remains_dummy,j)
					break
				end
			end
		end
		
		--1、最后一张洛神无解，取牌碰运气
		--2、最后一张为兵粮寸断无解，取牌碰运气
		if not anticipation[#anticipation][3] then return table.remove(remains_dummy):getEffectiveId() end

		--1、最后一张闪电的判定是红桃
		if anticipation[#anticipation][2] == "lightning" and anticipation[#anticipation][3]:getSuitString()=="heart" then
			if not remains_dummy or judgecheck(nextplayer,"lightning",table.remove(remains_dummy)) then
				return nil
			end
			return anticipation[#anticipation][3]:getEffectiveId()
		end
	else --敌人
		local remains_dummy ={}


		for i=1,#cardlist["club"] do
			table.insert(remains_dummy,table.remove(cardlist["club"]))
		end
		for i=1,#cardlist["spade"] do
			table.insert(remains_dummy,table.remove(cardlist["spade"]))
		end
		for i=1,#cardlist["diamond"] do
			table.insert(remains_dummy,table.remove(cardlist["diamond"]))
		end
		for i=1,#cardlist["heart"] do
			table.insert(remains_dummy,table.remove(cardlist["heart"]))
		end

		for n = 1 , #anticipation do
			for j,card in ipairs(remains_dummy) do
				if not judgecheck(nextplayer,anticipation[n][2],card) then
					anticipation[n][3] = table.remove(remains_dummy,j)
					break
				end
			end
		end
		
		--1、有红桃牌的情况下，洛神必定判定失效
		--2、最后一张非洛神（其实也只有兵粮寸断了）判定为红桃时，取牌碰运气
		if anticipation[#anticipation][3]:getSuitString() == "heart" and anticipation[#anticipation][2] ~= "luoshen" then
			return anticipation[#anticipation][3]:getEffectiveId()
		end

		--1、最后一张闪电或者乐判定无牌填坑
		if (anticipation[#anticipation][2] == "lightning" or anticipation[#anticipation][2] == "indulgence")
			and anticipation[#anticipation][3] == nil then
			--如果待填入卡列表中有红桃牌（没被洛神或者兵用完）
			if remains_dummy or remains_dummy[#remains_dummy]:getSuitString() == "heart" then
				return remains_dummy[#remains_dummy]:getEffectiveId()
			end
		end
		
		if #anticipation >=2 then
			local card_miss = false	
			for i=1,#anticipation do
				--遇到闪电后面有判定的情况
				if not card_miss and anticipation[i][2] == "lightning" and anticipation[i][2] == nil then
					card_miss = true
				end
				if card_miss and anticipation[i][2] ~= "lightning" and anticipation[i][3] == nil then
				--若后面的判定命中，检查待填入卡列表
					-- if not remains_dummy or remains_dummy[#remains_dummy]:getSuitString() ~= "heart" then
						-- return nil
					-- end
					--如果待填入卡列表中有红桃牌（没被洛神或者兵用完）
					if remains_dummy or remains_dummy[#remains_dummy]:getSuitString() == "heart" then
						return remains_dummy[#remains_dummy]:getEffectiveId()
					end
				end
			end
		end
	end
    return nil
end

function masu_xinzhan(self,cards, up_only)
    local remains={}
	local cardlist={
		spade={},
		heart={},
		club={},
		diamond={}
		}
    for i,id in ipairs(cards) do
		card=sgs.Sanguosha:getCard(id)
		self:log(i..":"..card:objectName()..":"..card:getSuitString())
        table.insert(remains,card)
		table.insert(cardlist[card:getSuitString()],card)
    end

	self:log("1:#remains:"..#remains)
	
	local nextplayer=self.room:nextPlayer()
	local next_judge=sgs.QList2Table(nextplayer:getCards("j"))
	local anticipation={}
	
	local luoshen_num,next_need_judge = 0
	
	if nextplayer:hasSkill("luoshen") then
		if self:isFriend(nextplayer) then
			for i=1,#cardlist["spade"]+#cardlist["club"] do
				table.insert(anticipation,{"next","luoshen",nil})
				luoshen_num = luoshen_num + 1
			end
		end
		table.insert(anticipation,{"next","luoshen",nil})
		luoshen_num = luoshen_num + 1
	end
	
	while(#next_judge>0) do
		local judge_string = Card2Judge(table.remove(next_judge))
		table.insert(anticipation,{"next",judge_string,nil})
		--next_need_judge = next_need_judge + 1
	end
	
	while(#anticipation>#cards) do table.remove(anticipation) end

	if self:isFriend(nextplayer) then	--a friend
		self:log("friend")
		local remains_dummy ={}

		for i=1,#cardlist["club"] do
			table.insert(remains_dummy,table.remove(cardlist["club"]))
		end
		for i=1,#cardlist["spade"] do
			table.insert(remains_dummy,table.remove(cardlist["spade"]))
		end
		for i=1,#cardlist["diamond"] do
			table.insert(remains_dummy,table.remove(cardlist["diamond"]))
		end
		for i=1,#cardlist["heart"] do
			table.insert(remains_dummy,table.remove(cardlist["heart"]))
		end

		for n = 1 , #anticipation do
			for j,card in ipairs(remains_dummy) do
				if judgecheck(nextplayer,anticipation[n][2],card) then
					anticipation[n][3] = table.remove(remains_dummy,j)
					break
				end
			end
		end
		self:log("2:#remains:"..#remains)
		remains = {}
		for i=1,#remains_dummy do
			table.insert(remains,remains_dummy[i])
		end
		self:log("2:#remains:"..#remains)
	else --not a friend
		self:log("friend")
		local remains_dummy ={}
		for i=1,#cardlist["club"] do
			table.insert(remains_dummy,table.remove(cardlist["club"]))
		end
		for i=1,#cardlist["spade"] do
			table.insert(remains_dummy,table.remove(cardlist["spade"]))
		end
		for i=1,#cardlist["diamond"] do
			table.insert(remains_dummy,table.remove(cardlist["diamond"]))
		end
		for i=1,#cardlist["heart"] do
			table.insert(remains_dummy,table.remove(cardlist["heart"]))
		end

		for n = 1 , #anticipation do
			for j,card in ipairs(remains_dummy) do
				if not judgecheck(nextplayer,anticipation[n][2],card) then
					anticipation[n][3] = table.remove(remains_dummy,j)
					break
				end
			end
		end
		
		self:log("2:#remains:"..#remains)
		remains = {}
		for i=1,#remains_dummy do
			table.insert(remains,remains_dummy[i])
		end
		self:log("2:#remains:"..#remains)
	end

	for j=1,#anticipation do
		if not anticipation[j][3] then
			self:log(j..":anticipation["..j.."]")
			anticipation[j][3]=table.remove(remains,1)
			break
		end
	end

	local up={}
	k=1
	
	while(k <= #anticipation and anticipation[k][3]) do
		table.insert(up,anticipation[k][3]:getEffectiveId())
		self:log("insert into 'up':"..k)
		k=k+1
	end

	while (#remains>0) do table.insert(up,table.remove(remains):getEffectiveId()) end

	if #up < #cards then
		self:log("Default:Xinzhan")
		up = cards
	end

	return up,{}
end
