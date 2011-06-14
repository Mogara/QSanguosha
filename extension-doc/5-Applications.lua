--技能讲解4：基本技能类型的组合与复杂技能的实现

--大多数技能都不是单独使用ViewAsSkill或者TriggerSkill就可以实现的。复杂的技能往往需要结合多种基本技能。

--一个的技能的描述，往往可以分成3部分：发动的时机、发动代价、效果。

--对于发动时机的控制，是只有触发技可以实现的。触发技可以在任何“有定义的时机”时产生作用。然而，对于需要根据消耗牌来决定效果的那些技能，用触发技往往无法实现，也很难符合游戏流程。

--ViewAsSkill可以非常完美地实现技能的发动流程。只有VIewAsSkill可以细致地操纵玩家发动技能所付出的代价。然而，ViewAsSkill本身并没有技能效果的实现，只能使用游戏牌作为技能效果，或者借助SkillCard。

--SkillCard本身并不是一个技能，但是只有他可以把技能发动用牌和技能效果联系起来。它对于ViewAsSkill而言是必需的。


--1.结合基本技能的方法：

--我们把技能效果定义在一个技能牌skill_card当中，技能牌的定义可以不从属于任何技能。然后，由于时机判断是整个技能的发动最先进行的一步，我们将ViewAsSkill定义在TriggerSkill当中。最后，我们把TriggerSkill的发动效果设为“询问使用skill_card”，而ViewAsSkill的效果设为“你可以将特定牌作为skill_card使用或打出”。如此，即实现了“xxx时，你可以使用xx牌发动，执行xxx效果。”

--以下为大乔流离技能的实现：

liuli_card=sgs.CreateSkillCard{

name="liuli_effect",

target_fixed=false,

will_throw=true,

filter=function(self,targets,to_select)

	if #targets>0 then return false end
	
	if to_select:hasFlag("slash_source") then return false end
	
	local card_id=self:getSubcards()[1]
	if sgs.Self:getWeapon() and sgs.Self:getWeapon():getId()==card_id then 
		return sgs.Self:distanceTo(to_select)<=1 
	end
	
	return sgs.Self:canSlash(to_select,true)
end,

on_effect=function(self,effect)
	effect.to:getRoom():setPlayerFlag(effect.to,"liuli_target")
end

}

liuli_viewAsSkill=sgs.CreateViewAsSkill{

name="liuli_viewAs",

n=1,

view_filter=function(self, selected, to_select)
	return true
end,

view_as=function(self, cards)
	if #cards==0 then return nil end
	local aliuli_card=liuli_card:clone()
	--使用之前创建的skillCard的clone方法来创建新的skillCard
	aliuli_card:addSubcard(cards[1])
	
	return aliuli_card
end,

enabled_at_play=function()
	return false
end,

enabled_at_response=function()
	return sgs.Self:getPattern()=="#liuli_effect"
	--仅在询问流离时可以发动此技能
end
}

liuli_triggerSkill=sgs.CreateTriggerSkill{

name="liuli_main",

view_as_skill=liuli_viewAsSkill,

events={sgs.CardEffected},

on_trigger=function(self,event,player,data)
	local room=player:getRoom()
	local players=room:getOtherPlayers(player)
	local effect=data:toCardEffect()
	
	
	if effect.card:inherits("Slash") and (not player:isNude()) and room:alivePlayerCount()>2 then

		local canInvoke
		
		for _,aplayer in sgs.qlist(players) do
			if player:canSlash(aplayer) then 
				canInvoke=true
			end
		end
		
		if not canInvoke then return end
		
		local prompt="#liuli_effect:"..effect.from:objectName()
		room:setPlayerFlag(effect.from,"slash_source")
		if room:askForUseCard(player,"#liuli_effect",prompt) then 
			room:output("ha?")
			for _,aplayer in sgs.qlist(players) do
				if aplayer:hasFlag("liuli_target") then 
					room:setPlayerFlag(effect.from,"-slash_source")
					room:setPlayerFlag(aplayer,"-liuli_target")
					effect.to=aplayer
					
					room:cardEffect(effect)
					return true
				end
			end
		end
			
		
	end
end,
}

sgs.LoadTranslationTable{
	["liuli_main"] = "lua流离",
	[":liuli_main"] = "和流离技能描述一摸一样",
	["#liuli_effect"] = "和流离的询问字串一摸一样",
}
--翻译代码

--技能实现说明：
--此技能的实现分3个部分
--第一部分是流离牌，效果为“将目标标记为流离对象”
--第二部分是流离ViewAsSkill，“每当你需要打出一张流离牌时，你可以将任意一张牌当作流离牌打出”
--第三部分是流离TriggerSkill，“触发技，每当你成为杀的目标时，你可以打出一张流离牌，然后让具有流离标记的对象代你成为杀的目标”