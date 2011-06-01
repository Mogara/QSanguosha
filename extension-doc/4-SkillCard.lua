--技能讲解2：技能牌SkillCard

--神杀中，技能的效果在很多时候都技能牌实现。即，把技能定义在一张没有实体的抽象“牌”当中，当你发动技能的时候，视为你打出了这张牌。

--对于指定对象发动的技能，对象的指定也算在牌的效果当中。
--很多游戏的技能发动都带有cost这个概念，即发动技能的代价。神杀中，cost只能是你的牌或装备；也就是说，cost只能靠ViewAsSkill来实现。如果想实现类似于“发动代价”这样的效果，请用“发动技能的负面效果”这样的概念来替换。

--由于技能牌的需要有多个实例存在（每次发动技能得到一个技能牌），我们在DIY module当中并不像ViewAsSkill和TriggerSkill当中使用构造函数来创建SkillCard。我们需要将SkillCard的参数在一个lua table当中定义好，然后在每次需要创建SkillCard的时候再调用sgs.CreateSkillCard获取SkillCard对象。

--sgs.CreateSkillCard需要以下参数定义：

--name,target_fixed,will_throw, available,filter,feasible,on_use,on_effect

--target_fixed 和 will_throw为布尔量，代表技能是否需要指定对象发动，以及技能发动后该牌是否弃置。对于技能牌而言，即是说发动该技能所用的牌是否弃置。
--即使是以某名玩家为目标的技能，若总是没有选择（如陷阵杀），target_fixed也建议设为true。
--即使是大多数情况都没有选择的技能，若依然有可能有选择（如好施），target_fixed也建议设为false。

--对于拼点技能，请将will_throw设为false，否则对方将看到你的牌之后再选择拼点牌。

--也可以使用room:throwCard(card)这个方法来将牌移动到弃牌区。

--available方法返回是否可使用该牌。通常是受回合打出数目限制。如果返回false，那么使用该牌的“确定”按钮将是灰色的。若直接return true,那么该技能牌总是可以被打出。

--filter方法与ViewAsSkill中的view_filter相似，但filter方法的对象是玩家目标。对于还需选择目标玩家的牌（比如装备）的效果，请理解为“选择并获取某玩家的一张牌”是你的牌的后续效果，并不是牌本身的目标。

--feasible方法则相当于viewasSkill的view_as方法中invalid_condition的作用。在viewAsSkill中，我们可以无数次选中牌，直到返回了有意义的view_as再点确定，所以view_as返回了无意义的Nil也无所谓；然而在SkillCard当中，由于点确定的机会只有一次，我们必须要在实际的效果方法外排除无效的SkillCard的情况。


--以下为“离间牌”的available feasible 以及filter方法：

avaliable=function(self)
	return not sgs.Self:hasUsed("LijianCard")
	--sgs.Self为当前技能牌的拥有者玩家（或者说使用者）的player对象。
end,

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

--on_use 和 on_effect 为牌的效果执行。两者通常是只需要实现一个的，他们的区别在于生效时机的不同。on_use在牌被打出时即生效，而on_effect在牌对某一名目标进行结算时生效。因此，在不存在需要结算的“一名玩家对另一名玩家”的效果时，使用on_use实行效果即可；而存在指定的目标时，则原则上应该使用on_effect。

--on_use和on_effect可以同时存在。牌效果进行结算时，先执行on_use，然后对每名目标执行on_effect

--以下为“雷击牌”的on_effect方法：

on_effect=function(self,effect)

	--effect 为一个CardEffectStruct，其from和to为player对象，代表谁打出的牌对谁结算
	local from=effect.from
	local to  =effect.to
	local room=to:getRoom()
	
	--sefEmotion在玩家的头像上显示表情
	room:setEmotion(to,sgs.Room_Bad)
	
	--进行判定时，首先创建“判定”对象。
	--pattern为一个正则表达式，由冒号隔开的三段分别匹配牌名、花色和点数	--good的定义和之后的判定结果获取有关。原则上，之前的pattern与判定牌匹配时，如果这种情况下执行的效果对于判定者来说是“好”的，那么good应该是true。
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
		room:setEmotion(from,sgs.Room_Bad)
	end
	
end,

--以下为孙权“制衡”的on_use方法：

on_use=function(self,room,source,targets)
	room:throwCard(self)	--self代表技能牌本身。由于是将“任意张牌当成制衡牌打出”，因此弃置制衡牌就等于弃置所有用来发动制衡的牌，也即被制衡掉的牌。
	room:drawCards(source,self:getSubcards():length())
	--摸取相当于被用来发动制衡的牌的数目的牌。
	--可以用self:getSubcards()来获取这些牌的QList。
)
end,