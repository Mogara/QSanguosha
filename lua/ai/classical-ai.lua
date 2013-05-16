--[[
	文件：classical-ai.lua
	主题：经典战术
]]--
--[[
	PART 00：常用工具函数
]]--
	--判断卡牌的花色是否相符
	function MatchSuit(card, suit_table)
		if #suit_table > 0 then
			local cardsuit = card:getSuit()
			for _,suit in pairs(suit_table) do
				if cardsuit == suit then
					return true
				end
			end
		end
		return false
	end
	--判断卡牌的类型是否相符
	function MatchType(card, type_table)
		if type(suit_table) == "string" then
			type_table = type_table:split("|")
		end
		if #type_table > 0 then
			for _,cardtype in pairs(type_table) do
				if card:isKindOf(cardtype) then
					return true
				end
			end
		end
		return false
	end
--[[
	PART 01：3V3经典战术
	内容：黄金一波流、绝情阵
]]--
--黄金一波流--
	--相关信息
	sgs.GoldenWaveDetail = {
		KurouActor = {}, --苦肉执行者
		YijiActor = {}, --遗计执行者
		JijiuActor = {}, --急救执行者
		EruptSignal = {}, --起爆信号（五谷丰登中，急救执行者得到的红色牌）
	}
	--判断是否使用黄金一波流
	function GoldenWaveStart(self)
		local huanggai = self.player
		local room = huanggai:getRoom()
		sgs.GoldenWaveDetail.EruptSignal = {}
		if huanggai:hasSkill("kurou") then
			local guojia, huatuo
			if #self.friends_noself > 1 then
				for _,friend in pairs(self.friends_noself) do
					if friend:hasSkill("yiji") then
						guojia = friend
					elseif friend:hasSkill("jijiu") then
						huatuo = friend
					else
						room:setPlayerMark(friend, "GWF_Forbidden", 1)
					end
				end
			end
			if guojia and huatuo then
				sgs.GoldenWaveDetail.KurouActor = {huanggai:objectName()}
				sgs.GoldenWaveDetail.YijiActor = {guojia:objectName()}
				sgs.GoldenWaveDetail.JijiuActor = {huatuo:objectName()}
				room:setPlayerMark(huanggai, "GoldenWaveFlow", 1)
				room:setPlayerMark(guojia, "GoldenWaveFlow", 1)
				room:setPlayerMark(huatuo, "GoldenWaveFlow", 1)
				return true
			else
				sgs.GoldenWaveDetail.KurouActor = {}
				sgs.GoldenWaveDetail.YijiActor = {}
				sgs.GoldenWaveDetail.JijiuActor = {}
				room:setPlayerMark(huanggai, "GWF_Forbidden", 1)
				return false
			end
		end
		room:setPlayerMark(huanggai, "GWF_Forbidden", 1)
		return false
	end
	--黄金苦肉
	function GWFKurouTurnUse(self)
		local huanggai = self.player
		local released = sgs.GoldenWaveDetail.EruptSignal["Released"]
		if released then
			if self.getHp() > 1 then
				return sgs.Card_Parse("@KurouCard=.")
			end
		else
			return sgs.Card_Parse("@KurouCard=.")
		end
	end
	--黄金遗计
	function GWFYijiAsk(player, card_ids)
		local guojia = self.player
		local released = sgs.GoldenWaveDetail.EruptSignal["Released"]
		local huanggai = sgs.GoldenWaveDetail.KurouActor[1]
		local huatuo = sgs.GoldenWaveDetail.JijiuActor[1]
		if released then
			for _,id in ipairs(card_ids) do
				return huanggai, id
			end
		else
			for _,id in ipairs(card_ids) do
				local card = sgs.Sanguosha:getCard(id)
				if MatchType(card, "Crossbow|AOE|Duel") then
					return huanggai, id
				elseif card:isRed() and huatuo:isAlive() then
					return huatuo, id
				else
					return huanggai, id
				end
			end
		end
	end
	--黄金急救
	function GWFJijiuSignal(card, player, card_place)
		local huatuo = player
		if #EruptSignal > 0 then
			if card:getId() == EruptSignal[1] then
				local cards = player:getCards("he")
				for _,id in sgs.qlist(cards) do
					if id ~= EruptSignal[1] then
						local acard = sgs.Sanguosha:getCard(id)
						if acard:isRed() then
							return false
						end
					end
				end
				sgs.GoldenWaveDetail.EruptSignal["Released"] = card:getId()
			end
		end
		return true
	end
	--命苦的郭嘉（未完成）
--绝情阵
	--相关信息
	sgs.RuthlessDetail = {
		JianxiongActor = {}, --奸雄执行者
		TianxiangActor = {}, --天香执行者
		YijiActor = {}, --遗计执行者
	}
	--判断是否使用绝情阵
	function RuthlessStart(self)
		local caocao = self.player
		local room = caocao:getRoom()
		if caocao:hasSkill("jianxiong") then
			local xiaoqiao, guojia
			if #self.friends_noself > 1 then
				for _,friend in pairs(self.friends_noself) do
					if friend:hasSkill("yiji") then
						guojia = friend
					elseif friend:hasSkill("tianxiang") then
						xiaoqiao = friend
					else
						room:setPlayerMark(friend, "RL_Forbidden", 1)
					end
				end
			end
			if xiaoqiao and guojia then
				sgs.RuthlessDetail.JianxiongActor = {caocao:objectName()}
				sgs.RuthlessDetail.TianxiangActor = {xiaoqiao:objectName()}
				sgs.RuthlessDetail.YijiActor = {guojia:objectName()}
				room:setPlayerMark(caocao, "Ruthless", 1)
				room:setPlayerMark(guojia, "Ruthless", 1)
				room:setPlayerMark(xiaoqiao, "Ruthless", 1)
				return true
			else
				sgs.RuthlessDetail.JianxiongActor = {}
				sgs.RuthlessDetail.TianxiangActor = {}
				sgs.RuthlessDetail.YijiActor = {}
				room:setPlayerMark(caocao, "RL_Forbidden", 1)
				return false
			end
		end
		room:setPlayerMark(caocao, "RL_Forbidden", 1)
		return false
	end
	--绝情天香
	function RLTianxiangSkillUse(self, data)
		local xiaoqiao = self.player
		local caocao = sgs.RuthlessDetail.JianxiongActor[1]
		local damage = data
		if damage then
			local aoe = damage.card
			if aoe and aoe:isKindOf("AOE") then
				local handcards = self.player:getCards("h")
				handcards = sgs.QList2Table(handcards)
				self:sortByUseValue(handcards, true)
				for _, id in ipairs(handcards) do
					local suit = card:getSuit()
					if suit == sgs.Card_Heart or (xiaoqiao:hasSkill("hongyan") and suit == sgs.Card_Spade) then
						return "@TianxiangCard="..id.."->"..caocao:objectName()
					end
				end
			end
		end
	end
	--绝情遗计
	function RLYijiAsk(player, card_ids)
		local guojia = self.player
		local caocao = sgs.RuthlessDetail.JianxiongActor[1]
		local xiaoqiao = sgs.RuthlessDetail.TianxiangActor[1]
		for _,id in ipairs(card_ids) do
			local card = sgs.Sanguosha:getCard(id)
			if MatchType(card, "Crossbow|AOE|Duel|ExNihilo|Peach") then
				return caocao, id
			else
				local hearts = {sgs.Card_Heart, sgs.Card_Spade}
				if MatchSuit(card, hearts) and xiaoqiao:isAlive() then
					return xiaoqiao, id
				else
					return caocao, id
				end
			end
		end
	end
--[[
	PART 02：KOF经典战术
	内容：苦肉一波带、控底爆发
]]--
--苦肉一波带--
	--判断是否使用苦肉一波带
	function KOFKurouStart(self)
		local others = self.room:getOtherPlayers(self.player) 
		if others:length() == 1 then
			local enemy = others:first()
			if self:hasSkills("fankui|guixin|fenyong|zhichi|jilei", enemy) then
				self:speak("不行，这家伙不好对付，慢苦为妙。")
				self.room:setPlayerMark(self.player, "KKR_Forbidden", 1)
				return false
			end
			self:speak("看我大苦肉一波带走！")
			self.room:setPlayerMark(self.player, "KOFKurouRush", 1)
			return true
		end
		return false
	end
	--一波苦肉
	function KOFKurouTurnUse(self)
		local huanggai = self.player
		if huanggai:getHp() > 1 then
			return sgs.Card_Parse("@KurouCard=.")
		end
		if self:getCardsNum("Analeptic") + self:getCardsNum("Peach") > 0 then
			return sgs.Card_Parse("@KurouCard=.")
		end
	end
--控底爆发--
	--相关信息
	sgs.KOFControlType = {} --起爆卡牌的类型
	sgs.KOFControlSuit = {} --起爆卡牌的花色
	sgs.KOFControlResult = {} --控底结果
	sgs.KOFControlDetail = { --爆发详细信息
		EruptSkill = {}, --待爆发技能名
		MaxInterval = {}, --爆发时可容忍的两起爆卡牌间间隔
		ControlFinished = {} --爆发结束标志
	}
	--判断是否使用控底爆发战术
	function KOFControlStart(player)
		local room = player:getRoom()
		if player:hasSkill("guanxing") or player:hasSkill("super_guanxing") then
			local tag = player:getTag("1v1Arrange")
			if tag then
				local followList = tag:toStringList()
				if followList then
					if #followList > 0 then
						local follow = 1
						for _,name in ipairs(followList) do
							local general = sgs.Sanguosha:getGeneral(name)
							local flag = false
							if general:hasSkill("luoshen") then
								sgs.KOFControlSuit = {sgs.Card_Spade, sgs.Card_Club}
								sgs.KOFControlDetail.EruptSkill = {"luoshen"}
								sgs.KOFControlDetail.MaxInterval = {0}
								sgs.KOFControlDetail.ControlFinished = {false}
								flag = true
							elseif general:hasSkill("jizhi") then
								sgs.KOFControlType = {"TrickCard"}
								sgs.KOFControlDetail.EruptSkill = {"jizhi"}
								sgs.KOFControlDetail.MaxInterval = {1}
								sgs.KOFControlDetail.ControlFinished = {false}
							elseif general:hasSkill("xiaoji") then
								sgs.KOFControlType = {"EquipCard"}
								sgs.KOFControlDetail.EruptSkill = {"xiaoji"}
								sgs.KOFControlDetail.MaxInterval = {2}
								sgs.KOFControlDetail.ControlFinished = {false}
							elseif general:hasSkill("guhuo") then
								sgs.KOFControlSuit = {sgs.Card_Heart}
								sgs.KOFControlDetail.EruptSkill = {"guhuo"}
								sgs.KOFControlDetail.MaxInterval = {1}
								sgs.KOFControlDetail.ControlFinished = {false}
							elseif general:hasSkill("caizhaoji_hujia") then
								sgs.KOFControlSuit = {sgs.Card_Heart, sgs.Card_Diamond}
								sgs.KOFControlDetail.EruptSkill = {"caizhaoji_hujia"}
								sgs.KOFControlDetail.MaxInterval = {0}
								sgs.KOFControlDetail.ControlFinished = {false}
								flag = true
							end
							if #sgs.KOFControlType > 0 or #sgs.KOFControlSuit > 0 then
								room:setPlayerMark(player, "KOFControl", follow)
								if flag then
									room:setPlayerMark(player, "StrictControl", 1)
								end
								return true
							end
							follow = follow + 1
						end
					end
				end
			end
		end
		room:setPlayerMark(player, "KFC_Forbidden", 1)
		return false
	end
	--执行控底观星
	function KOFGuanxing(self, cards)
		local up = {}
		local bottom = {}
		local strict = self.player:getMark("StrictControl") > 0
		for _,id in pairs(cards) do
			local card = sgs.Sanguosha:getCard(id)
			if MatchSuit(card, sgs.KOFControlSuit) or MatchType(card, sgs.KOFControlType) then --相符
				if card:isKindOf("Peach") then --相符，但是桃子
					if self:isWeak() then --相符、桃子、虚弱
						table.insert(up, id)
					else --相符、桃子、不虚弱
						table.insert(bottom, id)
						table.insert(sgs.KOFControlResult, id)
						self.room:setPlayerMark(self.player, "KOFInterval", 0)
					end
				else --相符、不是桃子
					table.insert(bottom, id)
					table.insert(sgs.KOFControlResult, id)
					self.room:setPlayerMark(self.player, "KOFInterval", 0)
				end
			elseif strict then --不相符，严格
				table.insert(up, id)
			elseif card:isKindOf("Crossbow") then --不相符、不严格、诸葛连弩
				table.insert(bottom, id)
				table.insert(sgs.KOFControlResult, id)
				local marks = self.player:getMark("KOFInterval")
				self.room:setPlayerMark(self.player, "KOFInterval", marks+1)
			else --不相符、不严格、不为诸葛连弩
				local marks = self.player:getMark("KOFInterval")
				local maxInterval = sgs.KOFControlDetail.MaxInterval[1]
				if maxInterval and marks < maxInterval then --不相符、不严格、不为诸葛连弩、间隔较小
					local value = sgs.ai_use_value[card:objectName()]
					if value and value > 4 then --不相符、不严格、不为诸葛连弩、间隔较小、使用价值高
						table.insert(bottom, id)
						table.insert(sgs.KOFControlResult, id)
						self.room:setPlayerMark(self.player, "KOFInterval", marks+1)
					else --不相符、不严格、不为诸葛连弩、间隔较小、使用价值低
						table.insert(up, id)
					end
				else --不相符、不严格、不为诸葛连弩、间隔较大
					table.insert(up, id)
				end
			end
		end
		return up, bottom
	end
	--判断中间武将是否需要让路（待完善）
	function KOFNeedDeath(player)
		return false
	end
--[[
	PART X：控制总部
]]--
	sgs.classical_func = {}
	function Tactic(skillname, self, data)
		local func = sgs.classical_func[skillname]
		if func then
			return func(self, data)
		end
	end
--观星
	sgs.classical_func["guanxing"] = function(self, up_only)
		if not up_only then
			if self.player:getMark("KFC_Forbidden") == 0 then
				if self.player:getMark("KFC_Control") > 0 then
					return KOFGuanxing
				end
				if KOFControlStart(self.player) then
					return KOFGuanxing
				end
			end
		end
	end
--苦肉
	sgs.classical_func["kurou"] = function(self, data)
		local mode = string.lower(self.room:getMode()) 
		if mode == "02_1v1" then
			if self.player:getMark("KOFKurouRush") > 0 then
				return KOFKurouTurnUse
			elseif self.player:getMark("KKR_Forbidden") == 0 then
				if KOFKurouStart(self) then
					return KOFKurouTurnUse
				end
			end
		elseif mode == "06_3v3" then
			if self.player:getMark("GoldenWaveFlow") > 0 then
				return GWFKurouTurnUse
			elseif self.player:getMark("GWF_Forbidden") == 0 then
				if GoldenWaveStart(self) then
					return GWFKurouTurnUse
				end
			end
		end
	end
--遗计
	sgs.classical_func["yiji"] = function(self, data)
		local mode = string.lower(self.room:getMode())
		if mode == "06_3v3" then
			if self.player:getMark("GoldenWaveFlow") > 0 then
				return GWFYijiAsk
			elseif self.player:getMark("Ruthless") > 0 then
				return RLYijiAsk
			end
		end
	end
--急救
	sgs.classical_func["jijiu"] = function(self, data)
		local mode = string.lower(self.room:getMode())
		if mode == "06_3v3" then
			if self.player:getMark("GoldenWaveFlow") > 0 then
				return GWFJijiuSignal
			end
		end
	end
--天香
	sgs.classical_func["tianxiang"] = function(self, data)
		local mode = string.lower(self.room:getMode())
		if mode == "06_3v3" then
			if self.player:getMark("Ruthless") > 0 then
				return RLTianxiangSkillUse
			end
		end
	end