--技能详解4：其他技能

--距离技(sgs.CreateDistanceSkill)
--顾名思义，修改自己与其他玩家的距离用的，可以加也可以减。
--国战中只有马术和飞影用到了距离技
--注意距离技一经声明全场生效，即使不附加给玩家
--锁定与单一玩家的距离不用距离技，用setFixedDistance
--国战中同名的距离技也要区分开

--马术实现：

function sgs.CreateMashuSkill(name) --创建马术技能，在CreateDistanceSkill函数基础上建立的函数，啦啦版中的CreateFakeMoveSkill就是这样实现的
	local mashu_skill = {}
	mashu_skill.name = "LuaMashu_" .. name
	mashu_skill.correct_func = function(self, from, to)
		if from:hasSkill(self:objectName()) then
			return -1
		end
		return 0
	end
	return sgs.CreateDistanceSkill(mashu_skill)
end

LuaMashu_machao = sgs.CreateMashuSkill("machao") --附加给马超的技能为LuaMashu_machao，以此类推
LuaMashu_madai = sgs.CreateMashuSkill("madai")
LuaMashu_pangde = sgs.CreateMashuSkill("pangde")
LuaMashu_mateng = sgs.CreateMashuSkill("mateng")

--手牌上限技(sgs.CreateMaxCardsSkill)
--顾名思义，修改某人的手牌上限的，可以加也可以减。
--国战中只有横江和太平要术用到了手牌上限技
--注意手牌上限技一经声明全场生效，即使不附加给玩家
--国战中同名的手牌上限技也要区分开

--（你的手牌上限+2）实现：

LuaTestMaxCards = sgs.CreateMaxCardsSkill{
	name = "LuaTestMaxCards" ,
	extra_func = function(self, player)
		if (player:hasSkill(self:objectName())) then
			return 2
		end
		return 0
	end
}

--目标修改技(sgs.CreateTargetModSkill)
--顾名思义，修改某人使用牌的目标限制的
--有3种限制：
--次数限制：杀/酒
--目标数限制：杀/须指定目标的单目标锦囊（借刀除外）/铁索连环（借刀要用三将包的额外借刀技能卡，照抄灭计，桃/酒/无中生有使用PreCardUsed时机askForPlayerChosen）
--距离限制：杀/顺手牵羊/兵粮寸断
--除了杀的目标修改技之外，其他的目标修改技都已实现使用即亮将（虽然源码里使用的只有奇才和断粮）
--注意目标修改技一经声明全场生效，即使不附加给玩家

--奇才实现：

LuaQicai = sgs.CreateTargetModSkill{
	name = "LuaQicai" ,
	distance_limit_func = function(self, player, card)
	--这是距离限制的函数
	--目标数限制的函数名：extra_target_func
	--次数限制的函数名：residue_func
	--传入参数都是self, player, card
		if (player:hasSkill(self:objectName()) and card:isKindOf("TrickCard")) then
			return 1000 --返回1000，在我们的最多10人局里，基本就等于无限了
		end
		return 0
	end ,
}