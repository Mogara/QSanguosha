--[[
	文件：imagine-ai.lua
	主题：场景模拟
	函数列表：
		SmartAI:AtomDamageCount(target, source, nature, card)
		SmartAI:DamageToCards(target, source, count)
		SmartAI:DamageToTurnOver(target, source, count)
		SmartAI:DamageResult(target, source, nature, card, chained)
		SmartAI:tdr(result, source, target)
]]--
--[[
	函数名：AtomDamageCount
	功能：计算一次单体伤害的点数
	参数表：
		target：伤害目标
		source：伤害来源，默认值为nil（无源伤害）,推荐值为self.player（当前角色）
		nature：伤害属性，取值为：
			一般伤害：sgs.DamageStruct_Normal（默认值）
			火焰伤害：sgs.DamageStruct_Fire
			雷电伤害：sgs.DamageStruct_Thunder
		card：伤害所用卡牌，默认值为nil（没有卡牌）
	返回值：一个整数，表示伤害点数
]]--
function SmartAI:AtomDamageCount(target, source, nature, card)
	if not target then --没有伤害目标直接返回0
		return 0
	end
	if not nature then --伤害属性的默认值处理
		nature = sgs.DamageStruct_Normal
	end
	if not self:damageIsEffective(target, nature, source) then --伤害无效直接返回0
		return 0
	end
	local count = 1 --初始伤害点数
	local isSlash = false --杀造成的伤害
	local isNDTrick = false --非延时性锦囊造成的伤害
	if card then --卡牌伤害
		isSlash = card:isKindOf("Slash")
		isNDTrick = card:isNDTrick()
	end
	if target:getMark("@late") > 0 then --如果目标智迟中
		if isSlash or isNDTrick then --如果卡牌为杀或非延时性锦囊
			return 0
		end
	end
	if target:hasSkill("wuyan") or target:hasSkill("noswuyan") then --如果目标有技能无言
		if isNDTrick then --如果卡牌为非延时性锦囊
			return 0
		end
	end
	if target:hasSkill("huoshou") or target:hasSkill("juxiang") then --如果目标有技能祸首、巨象
		if isNDTrick and card:isKindOf("SavageAssault") then --如果卡牌为南蛮入侵
			return 0
		end
	end
	if nature == sgs.DamageStruct_Fire then --如果是火焰伤害
		local armor = target:getArmor()
		if armor then 
			if armor:isKindOf("Vine") or armor:isKindOf("GaleShell") then --如果目标装备了藤甲或狂风甲
				count = count + 1
				self.room:writeToConsole("vine+1")
			end
		end
		if target:getMark("@gale") > 0 then --如果目标有狂风标记
			count = count + 1
			self.room:writeToConsole("gale+1")
		end
	end
	if source then --如果有伤害来源
		local weapon = source:getWeapon()
		if weapon then
			if weapon:isKindOf("GudingBlade") then --如果来源有古锭刀
				if isSlash and target:isKongcheng() then --如果卡牌为杀且目标空城
					count = count + 1
					self.room:writeToConsole("guding+1")
				end
			end
		end
		if source:hasFlag("luoyi") then --如果来源有裸衣标志
			if isSlash or card:isKindOf("Duel") then --如果卡牌为杀或决斗
				count = count + 1
				self.room:writeToConsole("luoyi+1")
			end
		end
		if isSlash then --如果卡牌为杀
			if card:hasFlag("drank") then --如果卡牌为酒杀
				count = count + 1
				self.room:writeToConsole("drank+1")
			end
			if source:hasSkill("jie") then --如果来源有技能疾恶
				if card:isRed() then --如果是红杀
					count = count + 1
					self.room:writeToConsole("jie+1")
				end
			end
		end
	end
	return count
end
--[[
	函数名：DamageToCards
	功能：模拟一次伤害带来的卡牌收入
	参数表：
		target：伤害目标
		source：伤害来源
		count：伤害点数，默认值为1
	返回值：table类型，表示卡牌收入。包含6个项目：
		第一项：对target方带来的收入的平均值
		第二项：对target方带来的最高收入
		第三项：对target方带来的最低收入
		第四项：对target的对方带来的收入的平均值
		第五项：对target的对方带来的最高收入
		第六项：对target的对方带来的最低收入
]]--
function SmartAI:DamageToCards(target, source, count)
	local result = {}
	local thisIncome = {0, 0, 0}
	local thatIncome = {0, 0, 0}
	local thisRangeMax = 0
	local thisRangeMin = 0
	local thatRangeMax = 0
	local thatRangeMin = 0
	--收入计算
	if target then --伤害目标必须存在
		local room = self.room
		local civil = false --是否自相残杀
		local enemies = {}
		local friends = {}
		local players = room:getAlivePlayers()
		local others = room:getOtherPlayers(target)
		if source then
			civil = self:isFriend(target, source)
		end
		for _,p in sgs.qlist(players) do --区分敌友
			if self:isFriend(p, target) then
				table.insert(friends, p)
			else
				table.insert(enemies, p)
			end
		end
		--遗计（按点数结算）
		if target:hasSkill("yiji") then
			thisRangeMax = thisRangeMax + 2*count
			thisRangeMin = thisRangeMin + 2*count
		end
		--节命（按点数结算，应取前count名补牌空间最大的同伴计总数）
		if target:hasSkill("jieming") then
			for i=1, count, 1 do 
				local minCount = 6
				local maxCount = -1
				local flag = false
				for _,p in pairs(friends) do
					local maxhp = math.min(p:getMaxHp(), 5)
					local handcount = p:getHandcardNum()
					local ct = maxhp - handcount
					if ct > maxCount then
						maxCount = ct
						flag = true
					end
					if ct < minCount then
						minCount = ct
						flag = true
					end
				end
				if flag then
					thisRangeMax = thisRangeMax + maxCount
					thisRangeMin = thisRangeMin + minCount
				end
			end
		end
		--反馈（按次数结算）
		if target:hasSkill("fankui") then
			if source and not civil then
				thisRangeMax = thisRangeMax + 1
				thisRangeMin = thisRangeMin + 1
				thatRangeMax = thatRangeMax - 1
				thatRangeMin = thatRangeMin - 1
			end
		end
		--恩怨（按点数结算）
		if target:hasSkill("enyuan") then
			if source and not civil then
				thisRangeMax = thisRangeMax + count
				thisRangeMin = thisRangeMin + count
				thatRangeMax = thatRangeMax - count
				thatRangeMin = thatRangeMin - count
			end
		end
		--刚烈（按次数结算）
		if target:hasSkill("ganglie") then
			if source and not civil then
				if source:getHandcardNum() >= 2 then
					thatRangeMax = thatRangeMax - 2 --选择弃牌
					thatRangeMin = thatRangeMin - 0 --选择掉血
				end
			end
		end
		--放逐（包括极略中的放逐，按次数结算，这部分先这么写了，其实应该根据放逐本身的AI确定卡牌收入）
		if target:hasSkill("fangzhu") or target:hasSkill("jilve") then
			local flag = true
			local lost = target:getLostHp()
			for _,p in pairs(friends) do
				if not p:faceUp() then --放逐target一方
					thisRangeMax = thisRangeMax + lost
					thisRangeMin = thisRangeMin + lost
					flag = false
					break
				end
			end
			if flag then --放逐target的对方
				thatRangeMax = thatRangeMax + lost
				thatRangeMin = thatRangeMin + lost
			end
		end
		--破军（按次数结算，被忽略）
		--归心（按点数结算）
		if target:hasSkill("guixin") then
			local maxlength = 0
			for _,p in sgs.qlist(players) do --统计场上
				local ct = p:getCardCount(true) + p:getJudgingArea():length()
				if ct > maxlength then
					maxlength = ct
				end
			end
			local times = math.min(maxlength, count) --归心的次数
			for i=1, times, 1 do 
				local k = 0
				for _,p in pairs(enemies) do --由于是统计收益，也就是差值，所以只看能收对方多少牌就行了。
					if not p:isAllNude() then
						k = k + 1
					end
				end
				thisRangeMax = thisRangeMax + k --发动归心
				thisRangeMin = thisRangeMin + 0 --不发动归心
				thatRangeMax = thatRangeMax - k --发动归心
				thatRangeMin = thatRangeMin - 0 --不发动归心
			end
		end
		--誓仇
		if target:hasLordSkill("shichou") then
			if target.tag then --这个总是nil，始终无效。
				local tag = target.tag:value("ShichouTarget")
				if tag then
					local victim = tag:toPlayer()
					if self:isFriend(victim, target) then --被誓仇将伤害弹到自己人身上
						thisRangeMax = thisRangeMax + count
						thisRangeMin = thisRangeMin + count
					else --将伤害弹到对方身上
						thatRangeMax = thatRangeMax + count 
						thatRangeMin = thatRangeMin + count 
					end
				end
			end
		end
		--天香
		if target:hasSkill("tianxiang") then
			if not target:isKongcheng() then
				local maxCount = -1
				local minCount = 999
				local flag = false
				for _,p in pairs(enemies) do --将伤害弹到target的对方
					local lost = p:getLostHp()
					if lost < minCount then
						minCount = lost
						flag = true
					end
					if lost > maxCount then
						maxCount = lost
						flag = true
					end
				end
				if flag then
					thatRangeMax = thatRangeMax + maxCount
					thatRangeMin = thatRangeMin + minCount
				end
				flag = false
				minCount = 999
				maxCount = -1
				for _,p in pairs(friends) do --将伤害弹到target一方
					local lost = p:getLostHp()
					if lost < minCount then
						minCount = lost
						flag = true
					end
					if lost > maxCount then
						maxCount = lost
						flag = true
					end
				end
				if flag then
					thisRangeMax = thisRangeMax + maxCount - 1 
					thisRangeMin = thisRangeMin + minCount - 1
				end
			end
		end
		--涅槃
		if target:hasSkill("niepan") then
			if target:getMark("@nirvana") > 0 then
				if target:getHp() <= count then
					local cardcount = target:getCardCount(true) --原有的卡牌数目
					thisRangeMax = thisRangeMax - cardcount + 3 --发动涅槃
					thisRangeMin = thisRangeMin + 0 --不发动涅槃（被前位同伴所救）
				end
			end
		end
		--智愚
		if target:hasSkill("zhiyu") then
			if source and not civil then
				thisRangeMax = thisRangeMax + 1
				thisRangeMin = thisRangeMin + 1
				thatRangeMax = thatRangeMax - 1
				if target:isKongcheng() then
					thatRangeMin = thatRangeMin - 1 --依然同色
				else
					thatRangeMin = thatRangeMin - 0 --不再同色
				end
			end
		end
		--连理
		if target:getMark("@tie") > 0 then
			for _,p in pairs(enemies) do 
				if p:getMark("@tie") > 0 then
					thatRangeMax = thatRangeMax + count
					thatRangeMin = thatRangeMin + count
				end
			end
			for _,p in pairs(friends) do
				if p:getMark("@tie") > 0 then
					thisRangeMax = thisRangeMax + count 
					thisRangeMin = thisRangeMin + count 
				end
			end
		end
	end
	--产生结果
	thisIncome[1] = math.floor( (thisRangeMax + thisRangeMin) / 2 ) --对己方收入向下取整
	thisIncome[2] = thisRangeMax
	thisIncome[3] = thisRangeMin
	thatIncome[1] = math.ceil( (thatRangeMax + thatRangeMax) / 2 ) --对对方收入向上取整
	thatIncome[2] = thatRangeMax
	thatIncome[3] = thatRangeMin
	table.insert(result, thisIncome[1])
	table.insert(result, thisIncome[2])
	table.insert(result, thisIncome[3])
	table.insert(result, thatIncome[1])
	table.insert(result, thatIncome[2])
	table.insert(result, thatIncome[3])
	return result
end
--[[
	函数名：DamageToTurnOver
	功能：模拟一次伤害造成的翻面影响
	参数表：
		target：伤害目标
		source：伤害来源
		count：伤害点数
	返回值：table类型，包含8个项目：
		第一项：对target一方带来的翻面人数
		第二项：对target一方带来的等价翻回人数
		第三项：伤害后target一方正面向上的人数
		第四项：伤害后target一方背面向上的人数
		第五项：对target的对方带来的翻面人数
		第六项：对target的对方带来的等价翻回人数
		第七项：伤害后target的对方正面向上的人数
		第八项：伤害后target的对方背面向上的人数
]]--
function SmartAI:DamageToTurnOver(target, source, count)
	local result = {}
	local targetFriendsResult = {0, 0, 0}
	local targetEnemiesResult = {0, 0, 0}
	local thisTurnOverCount = 0
	local thatTurnOverCount = 0
	local thisDownCount = 0
	local thatDownCount = 0
	local thisEqualNum = 0
	local thatEqualNum = 0
	if target then
		local room = self.room
		local civil = false --是否自相残杀
		local enemies = {}
		local friends = {}
		local players = room:getAlivePlayers()
		local others = room:getOtherPlayers(target)
		if source then
			civil = self:isFriend(target, source)
		end
		for _,p in sgs.qlist(players) do --区分敌友
			if self:isFriend(p, target) then
				table.insert(friends, p)
				if not p:faceUp() then
					thisDownCount = thisDownCount + 1
				end
			else
				table.insert(enemies, p)
				if not p:faceUp() then
					thatDownCount = thatDownCount + 1
				end
			end
		end
		--放逐（包括极略中的放逐，按次数结算）
		if target:hasSkill("fangzhu") or target:hasSkill("jilve") then
			if thisDownCount > 0 then
				thisDownCount = thisDownCount - 1
				thisTurnOverCount = thisTurnOverCount + 1
				thisEqualNum = thisEqualNum - 1
			elseif target:getLostHp() +count >= 3 then
				thisDownCount = thisDownCount + 1
				thisTurnOverCount = thisTurnOverCount + 1
				thisEqualNum = thisEqualNum + 1
			else
				thatDownCount = thatDownCount + 1
				thatTurnOverCount = thatTurnOverCount + 1
				thatEqualNum = thatEqualNum + 1
			end
		end
		--破军（按次数结算，被忽略）
		--[[if source and source:hasSkill("pojun") then
			if civil then 
				if target:faceUp() then
					if target:getHp() > 2 then
						thisDownCount = thisDownCount + 1
						thisTurnOverCount = thisTurnOverCount + 1
					end
				else
					thisDownCount = thisDownCount - 1
					thisTurnOverCount = thisTurnOverCount + 1
				end
			else
				if target:faceUp() then
					if target:getHp() <= 2 then
						thisDownCount = thisDownCount + 1
						thisTurnOverCount = thisTurnOverCount + 1
					end
				end
			end
		end]]--
		--归心（按点数结算）
		if target:hasSkill("guixin") then
			local flag = target:faceUp()
			for i=1, count, 1 do
				flag = not flag
				if flag then
					thisDownCount = thisDownCount - 1
				else
					thisDownCount = thisDownCount + 1
				end
			end
			if flag ~= target:faceUp() then
				thisTurnOverCount = thisTurnOverCount + 1
				if flag then --从背面翻回正面
					thisEqualNum = thisEqualNum - 1
				else --从正面翻到背面
					thisEqualNum = thisEqualNum + 1
				end
			end
		end
		--酒诗
		if target:hasSkill("jiushi") then
			if target:faceUp() then --正面濒死造酒
				if target:getHp() <= count then
					thisDownCount = thisDownCount + 1
					thisTurnOverCount = thisTurnOverCount + 1
					thisEqualNum = thisEqualNum + 1
				end
			else --背面受伤翻回
				thisDownCount = thisDownCount - 1
				thisTurnOverCount = thisTurnOverCount + 1
				thisEqualNum = thisEqualNum - 1
			end
		end
		--悲歌（被忽略）
		if target:getHp() <= count then --模拟濒死场景
			--伏枥
			if target:hasSkill("fuli") then
				if target:getMark("@laoji") > 0 then
					if target:faceUp() then
						thisDownCount = thisDownCount + 1
						thisEqualNum = thisEqualNum + 1
					else
						thisDownCount = thisDownCount - 1
						thisEqualNum = thisEqualNum - 1
					end
					thisTurnOverCount = thisTurnOverCount + 1
				end
			end
			--涅槃
			if target:hasSkill("niepan") then
				if target:getMark("@nirvana") > 0 then
					if not target:faceUp() then
						thisDownCount = thisDownCount - 1
						thisTurnOverCount = thisTurnOverCount + 1
						thisEqualNum = thisEqualNum - 1
					end
				end
			end
		end
		--产生结果
		targetFriendsResult[1] = thisTurnOverCount
		targetFriendsResult[2] = #friends - thisDownCount
		targetFriendsResult[3] = thisDownCount
		targetEnemiesResult[1] = thatTurnOverCount
		targetEnemiesResult[2] = #enemies - thatDownCount
		targetEnemiesResult[3] = thatDownCount
	end
	table.insert(result, targetFriendsResult[1])
	table.insert(result, thisEqualNum)
	table.insert(result, targetFriendsResult[2])
	table.insert(result, targetFriendsResult[3])
	table.insert(result, targetEnemiesResult[1])
	table.insert(result, thatEqualNum)
	table.insert(result, targetEnemiesResult[2])
	table.insert(result, targetEnemiesResult[3])
	return result
end
--[[
	函数名：DamageResult
	功能：模拟一次伤害产生的后果
	参数表：
		target：伤害目标
		source：伤害来源，默认值为self.player（当前角色）
		nature：伤害属性，取值为：
			一般伤害：sgs.DamageStruct_Normal（默认值）
			火焰伤害：sgs.DamageStruct_Fire
			雷电伤害：sgs.DamageStruct_Thunder
		card：伤害所用卡牌，默认值为nil（没有卡牌）
		chained：是否考虑铁索连环的影响，取值为：
			考虑铁索连环：true（默认值）
			忽略铁索连环：false
	返回值：table类型，表示伤害结果。包含7个项目：
		第一项：伤害收益价值
		第二项：对target一方造成的伤害点数
		第三项：对target一方造成的手牌收入
		第四项：对target一方造成的翻面人数
		第五项：对target的对方造成的伤害点数
		第六项：对target的对方造成的手牌收入
		第七项：对target的对方造成的翻面人数
]]--
function SmartAI:DamageResult(target, source, nature, card, chained)
	local result = {}
	local damageValue = {0}
	local enemyProfit = {0, 0, 0}
	local friendProfit = {0, 0, 0}
	if target then --伤害目标必须存在
		if not nature then
			nature = sgs.DamageStruct_Normal
		end
		local shrink = false --伤害缩水标志
		local armor = target:getArmor() 
		if armor and armor:isKindOf("SilverLion") then --目标装备有白银狮子
			shrink = true
		end
		--直接伤害
		local count = self:AtomDamageCount(target, source, nature, card)
		if count > 1 and shrink then
			count = 1
		end
		self.room:writeToConsole(string.format("Now:immediateDamage=%d", count))
		local cardsEffect = self:DamageToCards(target, source, count) --平均收入、最高收入、最低收入
		local turnOverEffect = self:DamageToTurnOver(target, source, count) --翻面人数、正面人数、背面人数
		friendProfit[1] = friendProfit[1] + count
		friendProfit[2] = friendProfit[2] + cardsEffect[3]
		friendProfit[3] = friendProfit[3] + turnOverEffect[2]
		damageValue[1] = damageValue[1] - 2*friendProfit[1] + friendProfit[2] - 2.5*friendProfit[3]
		self.room:writeToConsole(string.format("Now:damageValue=%d,fd=%d,fc=%d,ft=%d", damageValue[1], friendProfit[1], friendProfit[2], friendProfit[3]))
		enemyProfit[1] = enemyProfit[1] + 0
		enemyProfit[2] = enemyProfit[2] + cardsEffect[4]
		enemyProfit[3] = enemyProfit[3] + turnOverEffect[6]
		damageValue[1] = damageValue[1] + 2*enemyProfit[1] - enemyProfit[3] + 2.5*enemyProfit[3]
		self.room:writeToConsole(string.format("Now:damageValue=%d,ed=%d,ec=%d,et=%d", damageValue[1], enemyProfit[1], enemyProfit[2], enemyProfit[3]))
		--传导伤害
		if chained then --如果考虑铁索连环
			if nature ~= sgs.DamageStruct_Normal then --如果是属性伤害
				if target:isChained() then --如果目标被铁索连环
					local others = self.room:getOtherPlayers(target)
					for _,p in sgs.qlist(others) do
						if p:isChained() then
							self.room:writeToConsole("in chained.")
							local SubResult = self:DamageResult(p, source, nature, nil, false)
							if self:isFriend(p, target) then
								friendProfit[1] = friendProfit[1] + SubResult[2]
								friendProfit[2] = friendProfit[2] + SubResult[3]
								friendProfit[3] = friendProfit[3] + SubResult[4]
								damageValue[1] = damageValue[1] + SubResult[1]
							else
								enemyProfit[1] = enemyProfit[1] + SubResult[5]
								enemyProfit[2] = enemyProfit[2] + SubResult[6]
								enemyProfit[3] = enemyProfit[3] + SubResult[7]
								damageValue[1] = damageValue[1] - SubResult[1]
							end
						end
					end
				end
			end
		end
	end
	--产生最终结果
	table.insert(result, damageValue[1])
	table.insert(result, friendProfit[1])
	table.insert(result, friendProfit[2])
	table.insert(result, friendProfit[3])
	table.insert(result, enemyProfit[1])
	table.insert(result, enemyProfit[2])
	table.insert(result, enemyProfit[3])
	return result
end
--[[
	函数名：tdr
	功能：测试DamageResult函数的结果
	参数表：result（待测试的结果）
	返回值：无
]]--
function SmartAI:tdr(result, source, target)
	local room = self.room
	local from = "nobody"
	local to = "nobody"
	if source then
		from = source:getGeneralName()
	end
	if target then
		to = target:getGeneralName()
	end
	room:writeToConsole("--------------------")
	room:writeToConsole("*******START********")
	room:writeToConsole(string.format("damage from %s to %s:", from, to))
	room:writeToConsole(string.format("	damageValue: %d", result[1]))
	room:writeToConsole("target:")
	room:writeToConsole(string.format("	damageCount: %d", result[2]))
	room:writeToConsole(string.format("	drawCards: %d", result[3]))
	room:writeToConsole(string.format("	turnDelt: %d", result[4]))
	room:writeToConsole("another:")
	room:writeToConsole(string.format("	damageCount: %d", result[5]))
	room:writeToConsole(string.format("	drawCards: %d", result[6]))
	room:writeToConsole(string.format("	turnDelt: %d", result[7]))
	room:writeToConsole("********END*********")
	room:writeToConsole("--------------------")
end
--[[
	函数名：KingdomsCount
	功能：统计指定范围内出现的势力数目
	参数表：players（QList<Player*>类型，表示待统计的范围）
	返回值：一个整数，表示范围内出现的势力的数目
]]--
function SmartAI:KingdomsCount(players)
	local kingdoms = {}
	for _,p in sgs.qlist(players) do
		local kingdom = p:getKingdom()
		local flag = true
		for _,k in pairs(kingdoms) do
			if k == kingdom then
				flag = false
			end
		end
		if flag then
			table.insert(kingdoms, kingdom)
		end
	end
	return #kingdoms
end
--[[
	函数名：SortByAtomDamageCount
	功能：按单体伤害对一组目标进行排序
	参数表：
		targets：目标列表，table类型
		source：伤害来源
		nature：伤害属性，取值为：
			一般伤害：sgs.DamageStruct_Normal（默认值）
			火焰伤害：sgs.DamageStruct_Fire
			雷电伤害：sgs.DamageStruct_Thunder
		card：伤害所用卡牌，默认值为nil（没有卡牌）
		inverse：排序顺序，取值为：
			true：按从小到大的顺序排序
			false：按从大到小的顺序排序（默认值）
	返回值：table类型，表示排序结果。
]]--
function SmartAI:SortByAtomDamageCount(targets, source, nature, card, inverse)
	if not nature then
		nature = sgs.DamageStruct_Normal
	end
	local compare_func = function(a,b)
		local damageA = self:AtomDamageCount(a, source, nature, card)
		local damageB = self:AtomDamageCount(b, source, nature, card)
		if damageA == damageB then
			if inverse then
				return a:getHp() >= b:getHp()
			else
				return a:getHp() <= b:getHp()
			end
		else
			if inverse then
				return damageA < damageB
			else
				return damageA > damageB
			end
		end
	end
	table.sort(targets, compare_func)
	return targets
end
--[[
	函数名：qvt
	功能：测试QVariant中的数据类型
	参数表：data，QVariant类型，表示待测试的数据
	返回值：一个字符串，表示数据类型
]]--
function SmartAI:qvt(data)
	if not data then
		return "nil"
	elseif data:toDamage() and data:toDamage().to then
			return "Damage"
	elseif data:toCardEffect() and data:toCardEffect().card then
		return "CardEffect"
	elseif data:toSlashEffect() and data:toSlashEffect().slash then
		return "SlashEffect"
	elseif data:toCardUse() and data:toSlashEffect().card then
		return "CardUse"
	elseif data:toCard() and data:toCard():getSuit() then
		return "Card"
	elseif data:toPlayer() and data:toPlayer():getHp() then
		return "Player"
	elseif data:toDying() and data:toDying().who then
		return "Dying"
	elseif data:toDamageStar() and data:toDamageStar().to then
		return "DamageStart"
	elseif data:toRecover() and data:toRecover().recover then
		return "Recover"
	elseif data:toJudge() and data:toJudge().who then
		return "Judge"
	elseif data:toPindian() and data:toPindian().from then
		return "Pindian"
	elseif data:toPhaseChange() and data:toPhaseChange().to then
		return "PhaseChange"
	elseif data:toMoveOneTime() and data:toMoveOneTime().card_ids then
		return "MoveOneTime"
	elseif data:toResponsed() and data:toResponsed().m_who then
		return "Responsed"
	elseif data:toInt() then
		return "Int"
	elseif data:toString() then
		return "String"
	elseif data:toStringList() then
		return "StringList"
	elseif data:toBool() ~= nil then
		return "Bool"
	end
	return "Unknown"
end
--[[
	主题：集火
]]--
sgs.ai_event_callback[sgs.Damaged].general = function(self, player, data)
	local damage = data:toDamage()
	local count = damage.damage
	local marks = player:getMark("FocusDamageCount")
	self.room:setPlayerMark(player, "FocusDamageCount", marks + count)
end
sgs.ai_event_callback[sgs.HpRecover].general = function(self, player, data)
	local recover = data:toRecover()
	local count = recover.recover
	local marks = player:getMark("FocusRecoverCount")
	self.room:setPlayerMark(player, "FocusRecoverCount", marks + count)
end
function SmartAI:getDamagePoint(player)
	if player then
		return player:getMark("FocusDamageCount")
	end
	return 0
end
function SmartAI:getRecoverPoint(player)
	if player then
		return player:getMark("FocusRecoverCount")
	end
	return 0
end
function SmartAI:RecoverRate(player)
	if player then
		local room = player:getRoom()
		local damage = player:getMark("FocusDamageCount")
		local recover = player:getMark("FocusRecoverCount")
		local rate = (recover+1) / damage
		return (recover+1)*rate + damage*0.05
	end
	return 1000
end
function SmartAI:sortByRecoverRate(players, inverse)
	local compare_func = function(a,b)
		local rateA = self:RecoverRate(a)
		local rateB = self:RecoverRate(b)
		if rateA == rateB then
			return self:getDamagePoint(a) < self:getDamagePoint(b)
		else
			if inverse then
				return rateA > rateB
			else
				return rateA < rateB
			end
		end
	end
	table.sort(players, compare_func)
end
function SmartAI:getFocusTarget(players)
	if players and #players > 0 then
		self:sortByRecoverRate(players)
		return players[1]
	end
end
--[[
	主题：特定事件模拟
]]--
--[[
	函数名：ImitateResult_DrawNCards
	功能：模拟指定技能对摸牌数目的影响
	参数表：
		player：摸牌目标，ServerPlayer类型
		skills：技能列表，表示加以考虑的技能，QList<Skill*>类型
		overall：是否考虑所有指定的技能，取值为：
			true：考虑所有所给技能
			false：仅考虑所给技能中目标实际拥有的技能（默认值）
	返回值：一个数值，表示最终的摸牌数目
	注：神速、巧变、绝境（高达一号）等因属于跳过阶段的情形，这里不加以考虑
]]--
function SmartAI:ImitateResult_DrawNCards(player, skills, overall)
	if not player then
		return 0
	end
	if player:isSkipped(sgs.Player_Draw) then
		return 0
	end
	if not skills or skills:length() == 0 then
		return 2
	end
	local drawSkills = {}
	if overall then
		for _,skill in sgs.qlist(skills) do
			table.insert(drawSkills, skill:objectName())
		end
	else
		for _,skill in sgs.qlist(skills) do
			if player:hasSkill(skill:objectName()) then
				table.insert(drawSkills, skill:objectName())
			end
		end
	end
	local count = 2 --初始摸牌数目
	if #drawSkills > 0 then
		local others = self.room:getOtherPlayers(player) --其他角色
		local alives = self.room:getAlivePlayers() --存活角色
		local lost = player:getLostHp() --已损失体力值
		for _,skillname in pairs(drawSkills) do
			if skillname == "tuxi" then --突袭，放弃摸牌
				return math.min(2, others:length())
			elseif skillname == "shuangxiong" then --双雄，放弃摸牌
				return 1
			elseif skillname == "zaiqi" then --再起，放弃摸牌（摸牌数目不定，返回期望值）
				return math.floor(lost * 3 / 4) 
			elseif skillname == "shelie" then --涉猎，放弃摸牌（摸牌数目不定，返回期望值）
				return 3
			elseif skillname == "xuanhuo" then --眩惑，放弃摸牌（摸牌数目不定，返回期望值）
				return 1
			elseif skillname == "nosfuhun" then --老父魂，放弃摸牌
				return 2
			elseif skillname == "luoyi" then --裸衣，少摸一张牌
				count = count - 1
			elseif skillname == "yingzi" then --英姿，多摸一张牌
				count = count + 1
			elseif skillname == "haoshi" then --好施，多摸两张牌
				count = count + 1
			elseif skillname == "juejing" then --绝境，多摸已损失体力值数目的牌
				count = count + lost
			elseif skillname == "yongsi" then --庸肆，多摸现存势力数目的牌
				local kingdoms = self:KingdomsCount(alives)
				count = count + kingdoms
			elseif skillname == "shenwei" then --神威，多摸两张牌
				count = count + 2
			elseif skillname == "jiangchi" then --将驰，多摸一张牌或少摸一张牌
				local choices = "jiang+chi+cancel"
				local func = sgs.ai_skill_choice.jiangchi
				if func then
					local choice = func(self, choices) 
					if choice == "jiang" then
						count = count + 1
					elseif choice == "chi" then
						count = count - 1
					end
				end
			elseif skillname == "zishou" then --自守，多摸已损失体力值数目的牌
				count = count + lost
			elseif skillname == "hongyuan" then --弘援，少摸一张牌
				count = count - 1
			elseif skillname == "kuiwei" then --溃围，弃置场上武器数目的牌
				if player:getMark("@kuiwei") > 0 then
					for _,p in sgs.qlist(alives) do
						if p:getWeapon() then
							count = count - 1
						end
					end
				end
			elseif skillname == "zhaolie" then --昭烈，少摸一张牌
				count = count - 1
			elseif skillname == "ayshuijian" then --水箭，多摸(自身装备数目+1)的牌
				local equips = player:getCards("e")
				count = count + 1 + math.floor(equips:length() / 2)
			end
		end
	end
	return count
end