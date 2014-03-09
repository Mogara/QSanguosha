--技能讲解2：技能牌SkillCard

--神杀中，技能的效果在很多时候都技能牌实现。即，把技能定义在一张没有实体的抽象“牌”当中，当你发动技能的时候，视为你使用了这张牌。

--对于指定对象发动的技能，对象的指定也算在牌的效果当中。
--很多游戏的技能发动都带有cost这个概念，即发动技能的代价。神杀中，cost只能是你的牌或装备；也就是说，
--cost只能靠ViewAsSkill来实现。如果想实现类似于“发动代价”这样的效果，请用“发动技能的负面效果”这样的概念来替换。

--由于技能牌的需要有多个实例存在（每次发动技能得到一个技能牌），
--我们在DIY module当中并不像ViewAsSkill和TriggerSkill当中使用构造函数来创建SkillCard。
--我们需要将SkillCard的参数在一个lua table当中定义好，然后在每次需要创建SkillCard的时候再调用sgs.CreateSkillCard获取SkillCard对象。
--或者，我们也可以先创建好一个SkillCard，然后在技能中复制它。（这是常用办法哦）

--sgs.CreateSkillCard需要以下参数定义：

--name, target_fixed, will_throw, handling_method, can_recast, filter, feasible, about_to_use, on_use, on_effect, on_validate, on_validate_in_response

--name:
--字符串，牌的名字。取个好听的名字~
--没有默认值。快去取名字……

--target_fixed：
--布尔值，使用该牌时是否需要玩家指定目标。
--默认为false，使用时你需要指定目标，然后点确定。

--will_throw:
--布尔值，该牌在使用后是否被弃置。还记得subCards吗？
--对于拼点技能，请将will_throw设为false，否则对方将看到你的牌之后再选择拼点牌。
--也可以将will_throw设为false,然后使用room:throwCard(card)这个方法来灵活地控制如何将牌移动到弃牌区。
--默认值为true。

--handling_method:
--Card::HandlingMethod枚举值，默认为：
sgs.Card_MethodUse --非will_throw
sgs.Card_MethodDiscard --will_throw
--表示这个技能卡的操作方式，不过一般情况下的非will_throw都要手动修改

--can_recast:
--布尔值，指定该牌能否被重铸。
--此值的修改仅仅影响被CardLimited之后还能不能选择这张牌，重铸的效果要写到about_to_use里

--filter：
--lua函数，返回一个布尔值，类似于ViewAsSkill中的view_filter，但filter方法的对象是玩家目标。
--你在使用牌时只能指定玩家为对象，不能直接指定玩家的某张牌为对象；
--比如过河拆桥，在神杀中，“选择对方的一张牌并弃置”是过河拆桥的效果，但过河拆桥的对象只有对方玩家。
--如果你确实需要“作为对象的牌”，请还是在了解游戏机制后自行发明解决方法……
--传入参数为self,targets(已经选择的玩家),to_select(需要判断是否能选择的玩家), Self(自身玩家)
--默认条件为“一名其他玩家”。

--feasible：
--lua函数，返回一个布尔值，相当于viewasSkill的view_as方法是否应该返回nil。
--在viewAsSkill中，我们可以无数次选中牌，直到返回了有意义的view_as再点确定，
--所以view_as返回了无意义的Nil也无所谓；然而在SkillCard当中，点确定的机会只有一次，
--因此我们规定用feasible来排除无效使用的情况。
--只有在feasible返回了true时，你才可以点确定。
--传入参数为self,targets(已经选择的玩家),Self(自身玩家)
--默认条件为"target_fixed为true(不需要指定目标)，或者选择了至少一名玩家为目标"

--about_to_use:
--lua函数，无返回值，执行使用结算当中的第一步
--除非有十足的把握重写这个函数不会影响技能卡结算，否则最好不要重写
--默认值比较复杂，暂时不介绍
--传入参数：self, room, cardUse(卡牌使用结构体)

--on_use:
--lua函数，无返回值，类似于on_trigger，执行使用效果中的一步。
--传入参数为self,room(游戏房间对象),source(使用者),targets(牌的使用目标)
--默认值为对每一名目标循环执行room:cardEffect()，此函数用于触发一系列事件最终执行on_effect。

--on_effect：
--lua函数，无返回值，执行使用效果的最后一步，定义对每名玩家的效果。
--传入参数self, effect(卡牌使用效果结构体)

--on_validate:
--lua函数，返回值为const Card *类型，用于在使用牌之前修改要使用的牌
--传入参数：self, cardUse
--默认值为返回自身
--使用此函数可以参考弘法

--on_validate_in_response
--lua函数，返回值为const Card *类型，用于在响应牌之前修改要使用的牌
--传入参数：self, user（打出牌的人）
--默认值为返回自身
--使用此函数可以参考弘法

--以下为“离间牌”的实现方法

LuaLijianCard = sgs.CreateSkillCard{
	name = "LuaLijianCard" ,
	filter = function(self, targets, to_select, Self)
		if not to_select:isMale() then
			return false
		end
		
		local duel = sgs.Sanguosha:cloneCard("Duel", sgs.Card_NoSuit, 0) --克隆一张决斗
		if (#targets == 0) and Self:isProhibited(to_select, duel) then --如果决斗目标不能被决斗，则返回false
			return false
		end
		if (#targets == 1) and to_select:isCardLimited(duel, sgs.Card_MethodUse) then --如果决斗的使用者不能决斗，则返回false
			return false
		end
		
		return (#targets < 2) and (to_select:objectName() ~= Self:objectName()) --离间牌的目标数要少于2才能选其他人，且不能为自己
	end ,
	feasible = function(self, targets, Self)
		return #targets == 2 --离间牌可以使用的前提只有目标数为2
	end ,
	about_to_use = function(self, room, cardUse) --重写的about_to_use，可以看出来只删掉了各种条件判断和排序目标的about_to_use也很麻烦，而且绝大多数情况不用重写
		local diaochan = cardUse.from
		
		local l = sgs.LogMessage()
		l.from = diaochan
		for _, p in sgs.qlist(cardUse.to) do
			l.to:append(p)
		end
		l.type = "#UseCard" ,
		l.card_str = self:toString()
		room:sendLog(l)
		
		local data = sgs.QVariant()
		data:setValue(cardUse)
		local thread = room:getThread()
		
		thread:trigger(sgs.PreCardUsed, room, diaochan, data)
		room:broadcastSkillInvoke("LuaLijian")
		
		local reason = sgs.CardMoveReason(sgs.CardMoveReason_S_REASON_THROW, diaochan:objectName(), "", "LuaLijian", "")
		room:moveCardTo(self, diaochan, nil, sgs.Player_DiscardPile, reason, true)
		
		thread:trigger(sgs.CardUsed, room, diaochan, data)
		thread:trigger(sgs.CardFinished, room, diaochan, data)
	end ,
	on_use = function(self, room, player, targets)
		--由于重写了about_to_use，删掉了排序目标，因此targets为界面上点击选择的顺序，正常的targets为按行动顺序排序的
		local to = targets[1] --决斗目标
		local from = targets[2] --决斗使用者
		
		local duel = sgs.Sanguosha:cloneCard("Duel", sgs.Card_NoSuit, 0) --真实克隆的决斗，这个才是真正要使用的
		duel:setSkillName("_" .. self:getSkillName()) --设置技能名
		
		if (not from:isCardLimited(duel, sgs.Card_MethodUse)) and (not from:isProhibited(to, duel)) then --如果满足了使用条件
			room:useCard(sgs.CardUseStruct(duel, from, to)) --使用决斗
		end
	end ,
}

--上例“离间”比较复杂，我们来看一个比较简单的技能卡：制衡。

LuaZhihengCard = sgs.CreateSkillCard{
	name = "LuaZhihengCard" ,
	target_fixed = true ,
	on_use = function(self, room, source, targets)
		if source:isAlive() then
			source:drawCards(self:getSubcards():length())
		end
	end ,
}

--制衡，就是这么简单。
--有人会问，弃牌哪里去了？怎么只有摸牌？不是要弃最多体力上限张么？这里怎么没限制？
--这张制衡技能卡的will_throw没有设置，所以为true，所以视为这张技能卡的牌，在执行on_use之前，就被弃置掉了。
--牌的限制在视为技里实现，而不是在这里。
