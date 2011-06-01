--技能详解3：TriggerSkill 触发技

--大部分技能都有”在xx时执行效果“这样的描述。任何非主动的技能也都可以用触发的方式来实现。
--也正因为如此，触发技种类繁多，也有很多不同的定义方式。
--基本的（也是万能的）触发技定义在Lua中是使用sgs.CreateViewAsSkill方法，该方法可以在lua\sgs_ex.lua中找到。

--CreateViewAsSkill需要以下参数：

--name, frequency（可选）, events, on_trigger, can_trigger(可选)

--其中name为技能名称字串

--frequency是技能的发动频率，默认为不经常发动，也就是说在满足发动条件时询问发动。锁定技需要在这里将frequency设为compulsory强制发动。

--events为发动时机。具体的时机列表我会提供参考文档。events可以是lua table也可以是一个单独的events enum(本质上就是int)

--on_trigger为触发效果函数。对于具有多个触发时机的技能(如庸肆)，需要使用条件语句，分时机执行效果。

--can_trigger为触发条件满足判断。对于满足发动条件的情况，此函数应该返回true。无定义时，默认为总是满足触发条件。

--以下是曹操奸雄的实现：

jianxiong=sgs.CreateTriggerSkill{
	
	frequency = sgs.Skill_Frequent,
	
	name      = "jianxiong",
	
	events={sgs.Damaged}, --或者events=sgs.Damaged
	
	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local card = data:toDamage().card
		if not room:obtainable(card,player) then return end
		if room:askForSkillInvoke(player,"jianxiong") then
			room:playSkillEffect("jianxiong")
			player:obtainCard(card)
		end
	end
}
--在on_trigger方法中，我们首先获取了room对象。

--对于影响整盘游戏的效果，我们必须需要获取room对象。大多数情况下，room对象都是必须获取的。

--on_trigger方法的data参数是一个QVariant，根据不同的事件我们需要用不同的方法得到它原本的数据类型。
--对于Damaged事件（你受到了伤害），data对象的类型是DamageStruct，我们使用toDamage()得到DamageStruct。

--询问技能发动时，需要使用room对象的askForSkillInvoke方法。
--playSkillEffect方法则可以播放技能的发动效果。（但是对技能发动效果本身没有影响）

--player:obtainCard(card) 即让player得到造成伤害的card。

--在”某个阶段可触发“的技能，或者”摸牌时改为xx“这样的技能，可以使用PhaseChange事件来触发，并对event对象进行判断进行触发控制。

--对于在复数个时机发动的触发技，我们需要在on_trigger中使用条件语句。

--以下是袁术”庸肆“技能的实现：

yongsi=sgs.CreateTriggerSkill{
	
	frequency = sgs.Skill_Compulsory, --锁定技
	
	name      = "yongsi",
	
	events={sgs.DrawNCards,sgs.PhaseChange}, --两个触发时机
	
	on_trigger=function(self,event,player,data)
	
		local room=player:getRoom()
		
		local getKingdoms=function() --可以在函数中定义函数
			local kingdoms={}
			local kingdom_number=0
			local players=room:getAlivePlayers()
			for _,aplayer in sgs.qlist(players) do
				if not kingdoms[aplayer:getKingdom()] then
					kingdoms[aplayer:getKingdom()]=true
					kingdom_number=kingdom_number+1
				end
			end
			return kingdom_number
		end
		
		if event==sgs.DrawNCards then 
			room:playSkillEffect("yongsi")
			data:setValue(data:toInt()+getKingdoms()) --DrawNCards事件的data是一个int类型的QVariant，改变该QVariant对象会改变摸牌数
		elseif (event==sgs.PhaseChange) and (player:getPhase()==sgs.Player_Discard) then
			--进入弃牌阶段时，先执行庸肆弃牌，然后再执行常规弃牌
			room:output("aaaaaa")
			local x = getKingdoms()
			local e = player:getEquips():length()+player:getHandcardNum()
			if e>x then room:askForDiscard(player,"yongsi",x,false,true) -- 最后两个参数为”是否强制“以及”是否包含装备“
			else 
				player:throwAllHandCards()
				player:throwAllEquips()
			end
		end
	end
}