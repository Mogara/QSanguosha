--[[********************************************************************
	Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

  This file is part of QSanguosha-Hegemony.

  This game is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3.0 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  See the LICENSE file for more details.

  QSanguosha-Hegemony Team
*********************************************************************]]

function speak(to, type)
	if not sgs.GetConfig("AIChat", false) then return end
	if to:getState() ~= "robot" then return end

	local i =math.random(1, #sgs.ai_chat[type])
	to:speak(sgs.ai_chat[type][i])
end

function speakTrigger(card, from, to, event)
	if type(to) == "table" then
		for _, t in ipairs(to) do
			speakTrigger(card, from, t, event)
		end
		return
	end

	if (event == "death") and from:hasShownSkill("ganglie") then
		speak(from, "ganglie_death")
	end

	if not card then return end

	if card:isKindOf("Indulgence") and (to:getHandcardNum() > to:getHp()) then
		speak(to, "indulgence")
	elseif card:isKindOf("LeijiCard") then
		speak(from, "leiji_jink")
	elseif card:isKindOf("QuhuCard") then
		speak(from, "quhu")
	elseif card:isKindOf("Slash") and to:hasShownSkill("yiji") and (to:getHp() <= 1) then
		speak(to, "guojia_weak")
	elseif card:isKindOf("SavageAssault") and (to:hasShownSkill("kongcheng") or to:hasShownSkill("huoji")) then
		speak(to, "daxiang")
	elseif card:isKindOf("FireAttack") and to:hasShownSkill("luanji") then
		speak(to, "yuanshao_fire")
	end
end

sgs.ai_chat_func[sgs.SlashEffected].blindness = function(self, player, data)
	local effect = data:toSlashEffect()
	if not effect.from then return end

	local chat = {"队长，是我，别开枪，自己人.",
				"尼玛你杀我，你真是夏侯惇啊",
				"再杀我一下，老子和你拼命了"
				}

	if self:hasCrossbowEffect(effect.from) then
		table.insert(chat, "快闪，药家鑫来了。")
		table.insert(chat, "果然是连弩降智商呀。")
		table.insert(chat, "杀死我也没牌拿，真2")
	end

	if effect.from:getMark("drank") > 0 then
		table.insert(chat, "喝醉了吧，乱砍人？")
	end

	if sgs.isAnjiang(effect.to) then
		table.insert(chat, "尼玛眼瞎了，老子是和你一伙的")
		table.insert(chat, "老大别打我，我现在不方便暴露")
		table.insert(chat, "别再杀我，你会裸")
		table.insert(chat, "盲狙一时爽啊, 我泪奔啊")
		table.insert(chat, "我次奥，哥们，盲狙能不能轻点？")
		if not sgs.isAnjiang(effect.from) and effect.from:getRole() ~= "careerist" then
			table.insert(chat, "杀你妹啊，我也是" .. sgs.Sanguosha:translate(effect.from:getKingdom()))
		end
	end

	local index = 1 + (os.time() % #chat)

	if os.time() % 10 <= 3 and not effect.to:isLord() then
		effect.to:speak(chat[index])
	end
end

sgs.ai_chat_func[sgs.Death].stupid_lord = function(self, player, data)
	local damage = data:toDeath().damage
	local chat = {"2B了吧，老子和你是一伙的还杀我",
				-- "主要臣死，臣不得不死",
				"房主下盘T了这个2货，拉黑不解释",
				"还有更2的吗",
				"真的很无语",
				}
	if damage and damage.from and player:isFriendWith(damage.from) and damage.to:objectName() == player:objectName() then
		local index = 1 + (os.time() % #chat)
		damage.to:speak(chat[index])
	end
end
--[[
sgs.ai_chat_func[sgs.Dying].fuck_renegade = function(self, player, data)
	local dying = data:toDying()
	local chat = {"小内，你还不跳啊，要崩盘吧",
				"9啊，不9就输了",
				"999...999...",
				"小内，我死了，你也赢不了",
				"没戏了，小内不帮忙的话，我们全部托管吧",
				}
	if (self.role=="rebel" or self.role == "loyalist") and sgs.current_mode_players["renegade"] > 0 and dying.who:objectName() == player:objectName() then
		local index = 1 + (os.time() % #chat)
		player:speak(chat[index])
	end
end
]]
sgs.ai_chat_func[sgs.EventPhaseStart].ally = function(self, player, data)
	local gameProcess = sgs.gameProcess()
	if string.find(gameProcess, ">>>") then
		local kingdom = gameProcess:split(">")[1]
		kingdom = sgs.Sanguosha:translate(kingdom)
		local chat = {"现在" .. kingdom .. "国比较猖狂，我们应该联合起来搞死他们",
					}
		if player:getPhase() == sgs.Player_Player and os.time() % 10 < 4 then
			player:speak(chat[math.random(1, #chat)])
		end
	end
end

sgs.ai_chat_func[sgs.EventPhaseStart].comeon = function(self, player, data)
	local chat = {"有货，可以来搞一下",
				"我有X张【闪】",
				"没闪, 不要乱来",
				"不爽，来啊！砍我啊",
				"求杀求砍求蹂躏",
				}
	if player:getPhase() == sgs.Player_Finish and not player:isKongcheng() and player:hasShownSkill("leiji") and os.time() % 10 < 4 then
		local index = 1 + (os.time() % #chat)
		player:speak(chat[index])
	end
end

sgs.ai_chat_func[sgs.EventPhaseStart].beset = function(self, player, data)
	local chat = {
		"大家一起围观一下",
		"不要一下弄死了，慢慢来",
		"速度，一人一下，弄死",
		"你投降吧，免受皮肉之苦啊，投降给全尸",
	}
	if #self.enemies == 1 and player:getPhase() == sgs.Player_Start and self:getKingdomCount() == 2 and player:getPlayerNumWithSameKingdom("AI") >= 3
		and self.enemies[1]:getPlayerNumWithSameKingdom("AI") == 1 and os.time() % 10 < 4 then
		local index = 1 + (os.time() % #chat)
		player:speak(chat[index])
	end
end

function SmartAI:speak(type, isFemale)
	if not sgs.GetConfig("AIChat", false) then return end
	if self.player:getState() ~= "robot" then return end

	if sgs.ai_chat[type] then
		local i =math.random(1,#sgs.ai_chat[type])
		if isFemale then type = type .. "_female" end
		self.player:speak(sgs.ai_chat[type][i])
	else
		self.player:speak(type)
	end
end

sgs.ai_chat = {}

sgs.ai_chat.yiji =
{
"再用力一点",
"要死了啊!"
}

sgs.ai_chat.hostile_female =
{
"啧啧啧，来帮你解决点手牌吧",
"叫你欺负人!" ,
"手牌什么的最讨厌了"
}

sgs.ai_chat.hostile = {
"yoooo少年，不来一发么",
"果然还是看你不爽",
"我看你霸气外露，不可不防啊"
}

sgs.ai_chat.respond_hostile = {
"擦，小心菊花不保",
"内牛满面了", "哎哟我去"
}

sgs.ai_chat.friendly = {
"。。。"
}

sgs.ai_chat.respond_friendly = {
 "谢了。。。"
 }

sgs.ai_chat.duel_female = {
"哼哼哼，怕了吧"
}

sgs.ai_chat.duel = {
"来吧！像男人一样决斗吧！"
}

sgs.ai_chat.lucky = {
"哎哟运气好",
"哈哈哈哈哈"
}

sgs.ai_chat.collateral_female = {
"别以为这样就算赢了！"
}

sgs.ai_chat.collateral = {
"你妹啊，我的刀！"
}

sgs.ai_chat.jijiang_female = {
"别指望下次我会帮你哦"
}

sgs.ai_chat.jijiang = {
"主公，我来啦"
}

--huanggai
sgs.ai_chat.kurou = {
"有桃么!有桃么？",
"教练，我想要摸桃",
"桃桃桃我的桃呢",
"求桃求连弩各种求"
}

--indulgence
sgs.ai_chat.indulgence = {
"乐，乐你妹啊乐",
"擦，乐我",
"诶诶诶被乐了！"
}

--leiji
sgs.ai_chat.leiji_jink = {
"我有闪我会到处乱说么？",
"你觉得我有木有闪啊",
"哈我有闪"
}

--quhu
sgs.ai_chat.quhu = {
"出大的！",
"来来来拼点了",
"哟，拼点吧"
}

--salvageassault
sgs.ai_chat.daxiang = {
"好多大象啊！",
"擦，孟获你的宠物又调皮了",
"内牛满面啊敢不敢少来点AOE"
}

--xiahoudun
sgs.ai_chat.ganglie_death = {
"菊花残，满地伤。。。"
}

sgs.ai_chat.guojia_weak = {
"擦，再卖血会卖死的",
"不敢再卖了诶诶诶诶"
}

sgs.ai_chat.yuanshao_fire = {
"谁去打119啊",
"别别别烧了别烧了。。。",
"又烧啊，饶了我吧。。。"
}

--xuchu
sgs.ai_chat.luoyi = {
"不脱光衣服干不过你"
}