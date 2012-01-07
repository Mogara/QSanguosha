--技能讲解2：技能牌SkillCard

--神杀中，技能的效果在很多时候都技能牌实现。即，把技能定义在一张没有实体的抽象“牌”当中，当你发动技能的时候，视为你使用了这张牌。

--对于指定对象发动的技能，对象的指定也算在牌的效果当中。
--很多游戏的技能发动都带有cost这个概念，即发动技能的代价。神杀中，cost只能是你的牌或装备；也就是说，
--cost只能靠ViewAsSkill来实现。如果想实现类似于“发动代价”这样的效果，请用“发动技能的负面效果”这样的概念来替换。

--由于技能牌的需要有多个实例存在（每次发动技能得到一个技能牌），
--我们在DIY module当中并不像ViewAsSkill和TriggerSkill当中使用构造函数来创建SkillCard。
--我们需要将SkillCard的参数在一个lua table当中定义好，然后在每次需要创建SkillCard的时候再调用sgs.CreateSkillCard获取SkillCard对象。
--或者，我们也可以先创建好一个SkillCard，然后在技能中复制它。

--sgs.CreateSkillCard需要以下参数定义：

--name,target_fixed,will_throw, filter,feasible,on_use,on_effect

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

--filter：
--lua函数，返回一个布尔值，类似于ViewAsSkill中的view_filter，但filter方法的对象是玩家目标。
--你在使用牌时只能指定玩家为对象，不能直接指定玩家的某张牌为对象；
--比如过河拆桥，在神杀中，“选择对方的一张牌并弃置”是过河拆桥的效果，但过河拆桥的对象只有对方玩家。
--如果你确实需要“作为对象的牌”，请还是在了解游戏机制后自行发明解决方法……
--传入参数为self,targets(已经选择的玩家),to_select(需要判断是否能选择的玩家)
--默认条件为“一名其他玩家”。

--feasible：
--lua函数，返回一个布尔值，相当于viewasSkill的view_as方法是否应该返回nil。
--在viewAsSkill中，我们可以无数次选中牌，直到返回了有意义的view_as再点确定，
--所以view_as返回了无意义的Nil也无所谓；然而在SkillCard当中，点确定的机会只有一次，
--因此我们规定用feasible来排除无效使用的情况。
--只有在feasible返回了true时，你才可以点确定。
--传入参数为self,targets(已经选择的玩家)
--默认条件为"target_fixed为true(不需要指定目标)，或者选择了至少一名玩家为目标"

--on_use:
--lua函数，无返回值，类似于on_trigger，执行使用效果。
--传入参数为self,room(游戏房间对象),source(使用者),targets(牌的使用目标)
--无默认。

--on_effect：
--lua函数，无返回值，同样用于执行使用效果，但只定义对于某一个目标的效果。
--通常情况下你只需要写on_effect或者on_use当中的一个。
--如果是没有目标或者是目标特定的技能，使用on_use；
--如果是有几个目标执行相同或类似的效果，使用on_effect。
--如果是玩家指定的目标，还是使用on_effect。

--以下为“离间牌”的 feasible 以及filter方法：

filter=function(self,targets,to_select)
	if (not to_select:getGeneral():isMale()) or #targets>1 then return false 
	elseif to_select:hasSkill("kongcheng") and to_select:isKongcheng() and #targets==0 then return false
	else return true end
	--当已经选择了多于1名玩家时，不能选择其他玩家。
	--空城诸葛不能选择。
end

feasible=function(self,targets)
	return #targets==2
	--只有选择了2名玩家时，才能使用牌
end

--on_use 和 on_effect 为牌的效果执行。他们的区别在于生效时机的不同。
--on_use在牌被使用时生效，而on_effect在牌在使用中对某一名目标进行结算时生效。
--因此，在不存在需要结算“一名玩家对另一名指定的玩家”的效果时，使用on_use实行效果即可；
--存在指定的目标时，则原则上应该使用on_effect。

--on_use和on_effect可以同时存在。牌效果进行结算时，先执行on_use，然后对每名目标执行on_effect

--以下为“雷击牌”的on_effect方法：

on_effect=function(self,effect)

	--effect 为一个CardEffectStruct，其from和to为player对象，代表谁使用的牌对谁结算
	local from=effect.from
	local to  =effect.to
	local room=to:getRoom()
	
	--sefEmotion在玩家的头像上显示表情
	room:setEmotion(to,"bad")
	
	--进行判定时，首先创建“判定”对象。
	--pattern为一个正则表达式，由冒号隔开的三段分别匹配牌名、花色和点数	
	--good的定义和之后的判定结果获取有关。
	--原则上，之前的pattern与判定牌匹配时，如果这种情况下执行的效果对于判定者来说是“好”的，
	--那么good应该是true。
	local judge=sgs.JudgeStruct()
	judge.pattern=sgs.QRegExp("(.*):(spade):(.*)")
	judge.good=false
	judge.reason="leiji"
	judge.who=to
	
	--然后，让room根据此判定对象进行判定。判定结果依然在judge里面。
	room:judge(judge)
	
	--如果判定结果是一个“坏”结果，那么造成伤害
	if judge.isBad() then
		--和判定一样，造成伤害时先创建伤害struct,然后交由room:damage执行
		local damage=sgs.DamageStruct()
        damage.card = nil
        damage.damage = 2
        damage.from = from
        damage.to = to
        damage.nature = sgs.DamageStruct_Thunder
		
		room:damage(damage)
	else
		room:setEmotion(from,"bad")
	end
	
end,

--以下为孙权“制衡”的on_use方法：

on_use=function(self,room,source,targets)
	--self代表技能牌本身。由于是将“任意张牌当成制衡牌打出”，
	--因此弃置制衡牌就等于弃置所有用来发动制衡的牌，也即被制衡掉的牌。
	room:throwCard(self)	

	--摸取相当于被用来发动制衡的牌的数目的牌。
	--可以用self:getSubcards()来获取这些牌的QList。
	room:drawCards(source,self:getSubcards():length())
end,