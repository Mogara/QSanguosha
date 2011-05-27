function speak(to,type)
	local i =math.random(1,#sgs.ai_chat[type])
	to:speak(sgs.ai_chat[type][i])
end

function speakTrigger(card,from,to,event)
	if (event=="death") and from:hasSkill("ganglie") then
		speak(from,"ganglie_death")
	end
	
	if not card then return end
	
	if card:inherits("Indulgence") and (to:getHandcardNum()>to:getHp()) then
		speak(to,"indulgence")
	elseif card:inherits("LeijiCard") then
		speak(from,"leiji_jink")
	elseif card:inherits("QuhuCard") then
		speak(from,"quhu")
	elseif card:inherits("Slash") and from:hasSkill("wusheng") and to:hasSkill("yizhong") then
		speak(from,"wusheng_yizhong")
	elseif card:inherits("Slash") and to:hasSkill("yiji") and (to:getHp()<=1) then
		speak(to,"guojia_weak")
	elseif card:inherits("SavageAssault") and (to:hasSkill("kongcheng") or to:hasSkill("huoji")) then
		speak(to,"daxiang")
	elseif card:inherits("FireAttack") and to:hasSkill("luanji") then
		speak(to,"yuanshao_fire")
	end
end


function SmartAI:speak(type, isFemale) 
    local i =math.random(1,#sgs.ai_chat[type])
	if isFemale then 
		type = type .. "_female" 
	end 
    self.player:speak(sgs.ai_chat[type][i])
end

sgs.ai_chat={}

sgs.ai_chat.hostile_female= 
{ 
"啧啧啧，来帮你解决点手牌吧", 
"叫你欺负人!" ,
"手牌什么的最讨厌了"
}

sgs.ai_chat.hostile={
"yoooo少年，不来一发么",
"果然还是看你不爽",
"我看你霸气外露，不可不防啊"
}

sgs.ai_chat.resbond_hostile={
"擦，小心菊花不保",
"内牛满面了", "哎哟我去" 
}

sgs.ai_chat.friendly=
{ "。。。" }

sgs.ai_chat.resbond_friendly= 
{ "谢了。。。" }

sgs.ai_chat.duel_female= 
{ 
"哼哼哼，怕了吧" 
}

sgs.ai_chat.duel=
{
"来吧！像男人一样决斗吧！"
}

sgs.ai_chat.lucky=
{
"哎哟运气好", 
"哈哈哈哈哈" 
}

sgs.ai_chat.collateral_female= 
{ 
"别以为这样就算赢了！" 
}

sgs.ai_chat.collateral= 
{ 
"你妹啊，我的刀！" 
}

sgs.ai_chat.jijiang_female= 
{ 
"别指望下次我会帮你哦" 
}

sgs.ai_chat.jijiang=
{
"主公，我来啦"
}

--huanggai
sgs.ai_chat.kurou=
{
"有桃么!有桃么？",
"教练，我想要摸桃",
"桃桃桃我的桃呢",
"求桃求连弩各种求"
}

--indulgence
sgs.ai_chat.indulgence=
{
"乐，乐你妹啊乐",
"擦，乐我",
"诶诶诶被乐了！"
}

--leiji
sgs.ai_chat.leiji_jink=
{
"我有闪我会到处乱说么？",
"你觉得我有木有闪啊",
"哈我有闪"
}

--quhu
sgs.ai_chat.quhu=
{
"出大的！",
"来来来拼点了",
"哟，拼点吧"
}

--wusheng to yizhong
sgs.ai_chat.wusheng_yizhong=
{
"诶你技能是啥来着？",
"在杀的颜色这个问题上咱是色盲",
"咦你的技能呢？"
}

--salvageassault
sgs.ai_chat.daxiang=
{
"好多大象啊！",
"擦，孟获你的宠物又调皮了",
"内牛满面啊敢不敢少来点AOE"
}

--xiahoudun
sgs.ai_chat.ganglie_death=
{
"菊花残，满地伤。。。"
}

sgs.ai_chat.guojia_weak=
{
"擦，再卖血会卖死的",
"不敢再卖了诶诶诶诶"
}

sgs.ai_chat.yuanshao_fire=
{
"谁去打119啊",
"别别别烧了别烧了。。。",
"又烧啊，饶了我吧。。。"
}				  

--xuchu
sgs.ai_chat.luoyi=
{
"不脱光衣服干不过你"
}