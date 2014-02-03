--大家好我是Fsu0413。
--从这个文件开始讲解DIY接口的用法。
--此文件为修改原作者hypercross的文件，用于神杀V2国战版，有很多的接口都和V1和V2有一些差别（尤其是V1）。

--首先，这个文件说明DIY需要的文件及其结构。

--DIY是以module的形式存在的。每个Module即是一个UTF8格式的Lua文件（建议用notepad++编辑），包含如下格式的代码：

module("extensions.moligaloo", package.seeall)  
-- 进入module。这里moligaloo这个词必须和文件名相同。

extension = sgs.Package("moligaloo", sgs.Package_GeneralPack)            
-- 创建扩展包对象。变量名必须为extension。
-- 第一个参数：指定扩展包的名字。这个词也必须和文件名相同。
-- 第二个参数：指定扩展包的类型：取值：
--[[
sgs.Package_GeneralPack -- 默认值，表示此扩展包为武将包。
sgs.Package_CardPack -- 表示此扩展包为卡牌包
]]

shiqian = sgs.General(extension, "shiqian", "qun") 
-- 创建武将对象,这里我们的武将是时迁。关于武将属性的详细说明见reference文档。

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
	["shentou"] = "神偷",
	[":shentou"] = "你可以将一张梅花手牌当做【顺手牵羊】使用。",

}
--此段为翻译，将技能名称与描述中文化，否则你将会看到拼音

shiqian:addSkill(shentou) 
--将神偷技能赋予时迁


--你可以将本文件保存至extension目录下的moligaloo.lua并启动游戏。此时扩展包即已经被添加至游戏。

--为了完善DIY扩展包，需要将音频、图片以及翻译代码放到指定目录。这一点将在其他文档中说明。