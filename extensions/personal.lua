module("extensions.personal", package.seeall)
extension = sgs.Package("personal")

tianqi = sgs.General(extension, "tianqi", "god", 5, false)
tianyin = sgs.General(extension, "tianyin", "god", 3)
tianshuang = sgs.General(extension, "tianshuang", "god", 3)

eatdeath=sgs.CreateTriggerSkill{
	name="eatdeath",
	frequency = sgs.Skill_NotFrequent,
	events={sgs.Death},

	can_trigger = function(self, player)
		return true
	end,

	on_trigger = function(self,event,player,data)
		local room = player:getRoom()
		local tenkei = room:findPlayerBySkillName(self:objectName())
		if not tenkei then return false end

		local skillslist = tenkei:getTag("EatDeath"):toString()
		local eatdeath_skills = skillslist:split("+")
		if eatdeath_skills[1] == "" then table.remove(eatdeath_skills, 1) end

		if room:askForSkillInvoke(tenkei, self:objectName(), data) then
			if #eatdeath_skills > 0 and sgs.Sanguosha:getSkill(eatdeath_skills[1]) then
				local choice = room:askForChoice(tenkei, self:objectName(), table.concat(eatdeath_skills, "+"))
				room:detachSkillFromPlayer(tenkei, choice)
				for i = #eatdeath_skills, 1, -1 do
					if eatdeath_skills[i] == choice then
						table.remove(eatdeath_skills, i)
					end
				end
			end
			room:loseMaxHp(tenkei)
			local skills = player:getVisibleSkillList()
			for _, skill in sgs.qlist(skills) do
				if skill:getLocation() == sgs.Skill_Right then
					if skill:getFrequency() ~= sgs.Skill_Limited and
						skill:getFrequency() ~= sgs.Skill_Wake then
						local sk = skill:objectName()
						room:acquireSkill(tenkei, sk)
						table.insert(eatdeath_skills, sk)
					end
				end
			end
			tenkei:setTag("EatDeath", sgs.QVariant(table.concat(eatdeath_skills, "+")))
		end
		return false
	end
}

skydao=sgs.CreateTriggerSkill
{
	name="skydao",
	frequency = sgs.Skill_Compulsory,
	events={sgs.Damaged},

	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		if player:getPhase() == sgs.Player_NotActive then
			local log = sgs.LogMessage()
			log.type = "#SkydaoMAXHP"
			log.from = player
			log.arg = tonumber(player:getMaxHp())
			log.arg2 = self:objectName()
			room:setPlayerProperty(player, "maxhp", sgs.QVariant(player:getMaxHp() + 1))
			room:sendLog(log)
		end
	end
}

noqing=sgs.CreateTriggerSkill{
	name="noqing",
	frequency = sgs.Skill_Compulsory,
	events={sgs.Damaged},
	priority = -1,

	default_choice = function(player)
		if player:getMaxHp() >= player:getHp() + 2 then
			return "maxhp"
		else
			return "hp"
		end
		end,

	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		for _, tmp in sgs.qlist(room:getOtherPlayers(player)) do
			if tmp:getHp() < player:getHp() then
				return false
			end
		end
		for _, tmp in sgs.qlist(room:getAllPlayers()) do
			local choice = room:askForChoice(tmp, self:objectName(), "hp+max_hp")
			local log = sgs.LogMessage()
			log.from = player
			log.arg = self:objectName()
			log.to:append(tmp)
			if(choice == "hp") then
				log.type = "#NoqingLoseHp"
				room:sendLog(log)
				room:loseHp(tmp)
			else
				log.type = "#NoqingLoseMaxHp"
				room:sendLog(log)
				room:loseMaxHp(tmp)
			end
		end
		return false
	end
}

doubledao = sgs.CreateSlashSkill
{
	name = "doubledao",
-- 额外目标
	s_extra_func = function(self, from, to, slash) -- from：使用者；to：目标；slash：所用的杀。这三个参数除了from，其余都是可有可无的
		if from:hasSkill("doubledao") and slash and slash:getSuit() == sgs.Card_Club then --注意必须先判断from是否有这个技能，否则谁都会发动的
			return 1 -- 这张杀可以指定一个额外目标，注意加上原本的，一共两个目标
		end
	end,
-- 攻击范围
	s_range_func = function(self, from, to, slash)
		if from:hasSkill("doubledao") and slash and slash:getSuit() == sgs.Card_Heart then
			return -4 -- 注意这里因为是锁定攻击范围，所以前面要加个负号，如果不加，则累加攻击范围
		end
	end,
}

dragonfist = sgs.CreateSlashSkill
{
	name = "dragonfist",
-- 额外出杀（返回还能再使用的杀的数量）
	s_residue_func = function(self, from)
	    if from:hasSkill("dragonfist") then
                local init =  1 - from:getSlashCount() -- 还能再使用的杀的数量，若已经使用了1张杀，则init=1-1=0，不能使用杀了
                return init + from:getMark("Fist") -- 如果获得了1个Fist标记，则在可用杀的基础上+1，本例中未0+1=1，有多少Fist标记可再使用多少张杀
			-- 返回值为998，表示使用杀无次数限制（如连弩、咆哮）
			-- 返回值为-998，表示不能再使用杀（如天义拼点失败）
            else
                return 0
	    end
	end,
}

dragonfistt=sgs.CreateTriggerSkill{
	name="#dragonfist",
	events={sgs.Damage, sgs.PhaseChange},
	priority = -1,

	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		if event == sgs.PhaseChange then
			room:setPlayerMark(player, "Fist", 0) -- 阶段转换时清除标记
		else
			local damage = data:toDamage()
			if damage.card:isKindOf("Slash") then
				room:setPlayerMark(player, "Fist", player:getMark("Fist")+1)
			end
		end
		return false
	end
}

tianqi:addSkill(eatdeath)
tianyin:addSkill(skydao)
tianyin:addSkill(noqing)
tianshuang:addSkill(doubledao)
tianshuang:addSkill(dragonfist)
tianshuang:addSkill(dragonfistt)

sgs.LoadTranslationTable{
	["personal"] = "Pesonal",

	["#tianyin"] = "天道之化身",
	["tianyin"] = "天音",
	["designer:tianyin"] = "鎏铄天音",
	["cv:tianyin"] = "",
	["illustrator:tianyin"] = "帕秋丽同人",
	["skydao"] = "天道",
	[":skydao"] = "锁定技，你的回合外，你每受到一次伤害，增加1点体力上限",
	["#SkydaoMAXHP"] = "%from 的锁定技【%arg2】被触发，增加了一点体力上限，目前体力上限是 %arg",
	["noqing"] = "无情",
	[":noqing"] = "锁定技，你受到伤害时，若你的体力是全场最少或同时为最少，则所有人必须减少1点体力或1点体力上限",
	["noqing:hp"] = "体力",
	["noqing:max_hp"] = "体力上限",
	["#NoqingLoseHp"] = "受到 %from 【%arg】锁定技的影响，%to 流失了一点体力",
	["#NoqingLoseMaxHp"] = "受到 %from 【%arg】锁定技的影响，%to 流失了一点体力上限",

	["#tianqi"] = "食死徒",
	["tianqi"] = "天启",
	["designer:tianqi"] = "宇文天启",
	["cv:tianqi"] = "",
	["illustrator:tianqi"] = "火影忍者",
	["eatdeath"] = "拾尸",
	[":eatdeath"] = "当有角色死亡时，你可以失去一个因“拾尸”获得的技能(如果有的话)，然后失去一点体力上限并获得该角色当前的所有武将技(限定技、觉醒技除外)",

	["#tianshuang"] = "静流",
	["tianshuang"] = "天霜",
	["doubledao"] = "双刀",
	[":doubledao"] = "LUA演示：你的草花杀可额外指定一个目标；你使用红桃杀的攻击范围锁定为4.",
	["dragonfist"] = "龙拳",
	[":dragonfist"] = "LUA演示：出牌阶段，当你的【杀】造成伤害时，可额外出一次【杀】",
}
