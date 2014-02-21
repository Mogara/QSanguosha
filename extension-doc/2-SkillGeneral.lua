--1.技能

--要说三国杀武将DIY，不得不说的就是技能。
--神杀给技能留的接口，与平时我们所说的技能分类有点区别。

--基本上来讲，技能分为以下几种：

--视为技：
--龙胆、武圣等等
--特点：可以将某牌当作另一种牌使用

--触发技：
--奸雄、反馈等等
--特点：在某个时机，可以执行某某效果

--距离技：
--马术、飞影
--特点：不用说大家都明白

--手牌上限技
--横江（后续效果）
--特点：还能是什么
--克己不是手牌上限技，是触发技

--目标修改技
--咆哮、集智、天义（后续效果）
--特点：修改某张牌在出牌阶段内的使用次数、修改某张牌可以选择的目标人数、修改某张牌可以选择的距离限制

--技能共通：
--所有的技能都通过sgs.CreateXXXSkill这个函数来实现，比如触发技：
thetrigger = sgs.CreateTriggerSkill{
	name = "thetrigger" ,
	events = {sgs.EventPhaseStart} ,
	can_trigger = function(self, event, room, player, data)
		blablabla
	end ,
	on_cost = function(self, event, room, player, data)
		blablabla
	end ,
	on_effect = function(self, evnet, room, player, data)
		blablabla
	end ,
}

--所有技能共通的属性放在这里，在技能讲解时将不再仔细讲它们的作用。
--name：
--字符串类型，表示技能的名字
--没有默认值，不能为空。

--relate_to_place：
--字符串类型，但是除了默认值之外，只能取两个值。
--"head"：表示该技能是主将技。
--"deputy"：表示该技能是副将技。
--不写取值或者取值为nil，则为主副将皆有的技能。


--除了技能之外，还要讲一下的就是技能卡。
--类似仁德、制衡等等这种出牌阶段空闲时间的技能，没有固定的时机。
--他们的效果也不是普通的游戏牌里所拥有的。
--这时就需要技能卡来解决这个问题了
--技能卡其实是个虚拟的牌，用于解决这类技能问题，有些触发技也需要用到技能卡来实现技能。
--说白了就是技能卡就是被神杀当作牌，而这类的牌的效果就是技能的效果。
--技能卡需要使用视为技才能使用，也就是将某牌（甚至没有牌）当技能卡这种牌来使用。
--仁德、制衡、离间、驱虎等等，为出牌阶段使用的技能卡
--流离（TargetConfirming)、天香(DamageInflicted)、突袭(EventPhaseStart)、巧变(EventPhaseChanging)等等，为响应时使用的技能卡

--触发技的触发时机

TriggerEvent = {
    NonTrigger, --没有时机，不能触发技能

    GameStart, --游戏开始时：化身
    TurnStart, --回合开始前：争功（倚天）
    EventPhaseStart, --阶段开始时（包括回合开始时，各个阶段的开始时，回合结束时和结束后）：突袭等等，这类技能很多
    EventPhaseProceeding, --阶段进行时：无技能
    EventPhaseEnd, --阶段结束时：生息
    EventPhaseChanging, --阶段转换时（也就是两个阶段中间的时间点）：巧变、克己, data:toPhaseChange()
    EventPhaseSkipping, --阶段被跳过时：无技能

    DrawNCards, --摸排阶段摸牌时（英姿、好施前半部分）, data:toInt()
    AfterDrawNCards, --摸牌阶段摸牌后（好施后半部分）, data:toInt()

    PreHpRecover, --回复体力前：无技能, data:toRecover()
    HpRecover, --回复体力时：恩怨, data:toRecover()
    PreHpLost, --体力流失前：弘法, data:toInt()
    HpChanged, --体力变化时：无技能
    MaxHpChanged, --体力上限变化时：无技能
    PostHpReduced, --体力变化后：规则里最重要的濒死结算在此时机0优先级, data:toInt()（用于流失体力）/data:toDamage()（用于伤害）

    EventLoseSkill, --失去技能时：不屈, data:toString()
    EventAcquireSkill, --获得技能时：有技能，但是举不出来例子, data:toString()

    StartJudge, --判定开始时：咒缚, data:toJudge()
    AskForRetrial, --改判时：鬼才、鬼道, data:toJudge()
    FinishRetrial, --改判后：无技能, data:toJudge()
    FinishJudge, --判定生效时：天妒、屯田后续效果, data:toJudge()

    PindianVerifying, --拼点检查时：鹰扬, data:toPindian()
    Pindian, --拼点生效时：制霸收拼点牌等等, data:toPindian()

    TurnedOver, --武将牌被翻面时（对于国战来讲是叠置）：解围
    ChainStateChanged, --武将牌被横置或重置时：无技能

    ConfirmDamage, --计算伤害点数：酒, data:toDamage()
    Predamage, --造成伤害前：绝情, data:toDamage()
    DamageForseen, --受到伤害前：狂风大雾, data:toDamage()
    DamageCaused, --造成伤害时：装备寒冰剑、潜袭（旧）, data:toDamage()
    DamageInflicted, --受到伤害时：天香，装备藤甲伤害+1的效果, data:toDamage()
    PreDamageDone, --造成伤害扣减体力时：武魂加标记, data:toDamage()
    DamageDone, --造成伤害扣减体力后：无技能, data:toDamage()
    Damage, --造成伤害后：烈刃，胆守, data:toDamage()
    Damaged, --受到伤害后：奸雄反馈遗计等等，技能非常多, data:toDamage()
    DamageComplete, --铁索，天香誓仇摸牌, data:toDamage()

    Dying, --进入濒死状态时：补益, data:toDying()
    QuitDying, --濒死结算后：黩武失去体力的效果, data:toDying()
    AskForPeaches, --处于濒死状态时：涅槃, data:toDying()
    AskForPeachesDone, --处于濒死状态时（求桃结束）：不屈防止死亡的效果, data:toDying()
    Death, --死亡时：断肠挥泪, data:toDeath()
    BuryVictim, --死亡后（注意优先级为负）：飞龙夺凤复活, data:toDeath()
    BeforeGameOverJudge, --死亡前：焚心, data:toDeath()
    GameOverJudge, --判断是否游戏结束时：狱刎（智）, data:toDeath()
    GameFinished, --游戏结束时：无技能

    SlashEffected, --杀生效时（防止杀的效果专用）：享乐等等，技能非常多, data:toSlashEffect()
    SlashProceed, --杀求闪时：旧版烈弓、旧版铁骑, data:toSlashEffect()
    SlashHit, --杀击中时：无技能, data:toSlashEffect()
    SlashMissed, --杀被闪避时：武器青龙刀，虎啸, data:toSlashEffect()

    JinkEffect, --闪生效时（防止闪的效果专用）：大喝后续效果, data:toCard()

    CardAsked, --求响应牌时：护驾、激将、八阵, data:toStringList()
    CardResponded, --响应牌时：雷击, data:toCardResponse()
    BeforeCardsMove, --卡牌移动时（注意此时卡牌还未移动）：落英、礼让、巨象, data:toMoveOneTime()
    CardsMoveOneTime, --卡牌移动后：惜粮（倚天）, data:toMoveOneTime()

    PreCardUsed, --卡牌使用前（用于限次数的视为普通牌的技能）, data:toCardUse()
    CardUsed, --卡牌使用时（不包括使用闪）：集智, data:toCardUse()
    TargetConfirming, --指定/成为目标时：享乐、流离, data:toCardUse()
    TargetConfirmed, --指定/成为目标后：铁骑、烈弓, data:toCardUse()
    CardEffect, --卡牌生效前（不要触发技能！！！）, data:toCardEffect()
    CardEffected, --卡牌生效时（防止除了杀之外的其他牌效果使用，注意杀也会经历此时机，不要无效！！会导致残留qinggang tag造成防具一直无效问题！！！）, data:toCardEffect()
    PostCardEffected, --卡牌生效后：无技能, data:toCardEffect()
    CardFinished, --卡牌结算完毕, data:toCardEffect()
    TrickCardCanceling, --锦囊牌询问无懈可击时：探虎拼点之后的效果, data:toCardEffect()

    ChoiceMade, --做出选择后，用于战功包, data:toString()

	--这4个时机用于其他游戏模式，国战没用
    StageChange, --虎牢关专用，神吕布变身为第二形态，注意此时机是用来throw的而不是用来触发的
    FetchDrawPileCard, --小型场景用，摸牌堆牌
    ActionedReset, --武将牌重置时，用于3v3
    Debut, --登场时，用于1v1和血战到底

    TurnBroken, --结算终止回合结束，用于胆守和翩仪，注意此时机是用来throw的而不是用来触发的

	--这3个时机用于官方国战
    GeneralShown, --武将牌明置时：闺秀, data:toBool()
    GeneralHidden, --武将牌暗置时：不屈进入濒死状态的时机, data:toBool()
    GeneralRemoved, --武将牌被移除时：闺秀, data:toString()

    NumOfEvents --时机总数（不能触发技能）
}
