function slashdamage(self,gong,shou)  --定义了一个函数，可以计算，一张杀在不被手牌中的闪闪避时，可以造成的伤害
    local pre = 1     --默认可造成一点伤害
	local amr=shou:getArmor()
	--以下是判断队友是否有技能鬼才鬼道极略
	local zj = self.room:findPlayerBySkillName("guidao")
	local sm = self.room:findPlayerBySkillName("guicai")
	local ssm = self.room:findPlayerBySkillName("jilve")
	local godlikefriend = false
	if (zj and self:isFriend(zj) and self:canRetrial(zj)) or
	    (sm and self:isFriend(sm) and sm:getHandcardNum() >= 2) or
		(ssm and self:isFriend(ssm) and ssm:getHandcardNum() >= 2 and ssm:getMark("@bear")) then
        godlikefriend = true
    end		
	
	--以下完成了这样的计算：
	--如果被杀的人有八卦，藤甲，那么伤害为0
	--如果我有青钢，朱雀，古锭刀并且满足触发条件，那么重新调整伤害值为1或2
	if ame then
	    if amr:objectName()=="eight_diagram" or shou:hasSkill("bazhen") or amr:isKindOf("Vine") then
		    pre = 0
		end
	    if (amr:objectName()=="eight_diagram" or shou:hasSkill("bazhen")) and (gong:hasWeapon("QinggangSword") or godlikefriend == true) then
		    pre = 1
		end
		if amr:isKindOf("Vine") and gong:hasWeapon("Fan") then
		    pre = 2
		end
		if amr:isKindOf("Vine") and gong:hasWeapon("QinggangSword") then
		    pre = 1
		end
		if not amr:objectName()=="silver_lion" and gong:hasWeapon("GudingBlade") and shou:isKongcheng() then
		    pre = 2
		end
	else
	    if gong:hasWeapon("GudingBlade") and shou:isKongcheng() then
		    pre = 2
		end
	end
	--以下目的是：如果被杀的角色拥有技能流离、雷击、刚烈、武魂、挥泪等等技能时，重新调整伤害值为0
	--可以避免对上述技能拥有者出杀
	if self:slashProhibit(nil, shou) then
	    pre = 0
	end
	--最后这一段，如果大猪哥空城了，那么重新调整伤害为-100
	--之所以没有调成0，是和后面的代码有关，如果此处调整为0，那么面对0血0牌的诸葛周泰双将，可能Ai会违反空城的规则
	if shou:hasSkill("kongcheng") and shou:isKongcheng() then
	    pre = -100
	end
	return pre
end
	
--一下为询问是否发动急速是Ai的处理
sgs.ai_skill_use["@@jisu"]=function(self,prompt)
	local besttarget --先整了两个空角色，最佳触发角色和触发角色
	local target
	self:sort(self.enemies,"hp") --这里讲敌人按照体力值排序，也可按防御值排序，我也不知道那个更合理些
	--以下，寻找最佳被杀的人选，如果我此技能一发动，你很有可能就濒死求桃了，那么就选你了
	for _,enemy in ipairs(self.enemies) do
	    if sgs.getDefense(enemy) < 6 and slashdamage(self,self.player,enemy) >= enemy:getHp() then
		    besttarget = enemy
			break
	    end
	end
	--以下，寻找一个可以的人选
	for _,enemy in ipairs(self.enemies) do
	    if sgs.getDefense(enemy) < 8 and slashdamage(self,self.player,enemy) > 0 then
		    target = enemy
			break
	    end
	end
	--以下，确定了要发动技能，标记了杀谁
	--如果你是最佳触发角色，那我是一定会发动的
	--如果你不是最佳触发者，那么还要考虑我自身的状态，这里没有考虑判定区和攻击范围内没人而打酱油的情况，这是因为我不知该如何考虑
	if besttarget then
		return "@JisuCard=.->" .. besttarget:objectName()
	elseif target and sgs.getDefense(self.player) > 8 then
	    return "@JisuCard=.->" .. target:objectName()
	end
	return "."
end

--这里是阻止他人对拥有技能 水泳 的人使用 火杀
--其他火焰伤害的情况，分别修改了 火攻，朱雀发动，亚炎 三处，需在相应位置查看修改痕迹
--没有考虑铁索 铁索收益计算 神诸葛狂风的问题，无力做的那么细致
function sgs.ai_slash_prohibit.shuiyong(self, to, card)
	if card:isKindOf("FireSlash") then return true end
	return false
end

--嘲讽值
sgs.ai_chaofeng.yeshiwen = 1

--以下是告诉AI，水箭技能拥有者需要装备牌，AI在发动仁德、遗计技能时回想起你的
sgs.ai_cardneed.shuijian = sgs.ai_cardneed.equip
--嘲讽值
sgs.ai_chaofeng.sunyang = 2
