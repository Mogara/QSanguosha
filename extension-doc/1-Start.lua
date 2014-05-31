--大家好我是Fsu0413。
--从这个文件开始讲解DIY接口的用法。
--此文件为修改原作者hypercross的文件，用于神杀V2国战版，有很多的接口都和V1和V2有一些差别（尤其是V1）。

--首先，这个文件说明DIY需要的文件及其结构。

--DIY是以LUA文件（建议使用Notepad++编辑）的形式存在的。每个文件里返回一个LUA表，表中的值全部为Package类型。

--创建一个Package对象，这里假设对象名为extension
extension = sgs.Package("moligaloo", sgs.Package_GeneralPack)
-- 第一个参数：指定扩展包的名字。
-- 第二个参数：指定扩展包的类型：取值：
--[[
sgs.Package_GeneralPack -- 默认值，表示此扩展包为武将包。
sgs.Package_CardPack -- 表示此扩展包为卡牌包
]]

shiqian = sgs.General(extension, "shiqian", "qun") 
-- 创建武将对象，这里我们的武将是时迁，是一个2血群雄武将。关于武将属性的详细说明见reference文档。

shentou = sgs.CreateOneCardViewAsSkill{ 
--创建技能，技能种类为OneCardViewAsSkill。这里的技能是“出牌阶段，你可以将任意一张梅花手牌当作顺手牵羊使用。” 
	name = "shentou",
	filter_pattern = ".|club|.|hand" ,
	view_as = function(self, card)
		local new_card = sgs.Sanguosha:cloneCard("snatch", sgs.Card_SuitToBeDecided, -1)
		new_card:addSubcard(card:getId())
		new_card:setSkillName(self:objectName())
		new_card:setShowSkill(self:objectName())
		return new_card
	end
}
--关于技能的说明将是几乎所有其他帮助文件的重点。此处省略。

sgs.LoadTranslationTable{
	["moligaloo"] = "太阳神上" ,
	
	["shentou"] = "神偷",
	[":shentou"] = "你可以将一张梅花手牌当做【顺手牵羊】使用。",

}
--此段为翻译，将包名、技能名称与描述中文化，否则你将会看到拼音

shiqian:addSkill(shentou) 
--将神偷技能赋予时迁

--最后返回包含这个Package对象的表

return {extension}

--你可以将本文件保存至extension目录下的moligaloo.lua并启动游戏。此时扩展包即已经被添加至游戏。

--为了完善DIY扩展包，需要将音频、图片以及翻译代码放到指定目录。这一点将在其他文档中说明。