-- TestPackage

return {
-- danchuang
	["danchuang"] = "胆创包",

	["#v5zhonghui"] = "蛋疼的野心家",
	["v5zhonghui"] = "鐘會",
	["designer:v5zhonghui"] = "韓旭",
	["illustrator:v5zhonghui"] = "雪君S",
	["v5zhenggong"] = "爭功",
	[":v5zhenggong"] = "你每受到一次伤害，可以获得伤害来源装备区中的一张牌并立即放入你的装备区。",
	["v5quanji"] = "權計",
	[":v5quanji"] = "其他角色的回合开始时，你可以与该角色进行一次拼点。若你赢，该角色跳过回合开始阶段及判定阶段。",
	["@v5qj"] = "%src 的回合开始了，你可以发动【權計】和其拼点，若你赢，%src 跳过开始阶段和判定阶段",
	["v5baijiang"] = "拜將",
	[":v5baijiang"] = "觉醒技，回合开始阶段开始时，若你的装备区的装备牌为三张或更多时，你必须增加1点体力上限，失去技能【权计】和【争功】并获得技能“野心”（你每造成或受到一次伤害，可将牌堆顶的一张牌放置在武将牌上，称为“权”。出牌阶段，你可以用任意数量的手牌与等量的“权”交换，每阶段限一次）。",
	["#BaijiangWake"] = "%from 身上的装备达到 %arg 张，觉醒技【%arg2】触发",
	["v5yexin"] = "野心",
	[":v5yexin"] = "你每造成或受到一次伤害，可将牌堆顶的一张牌放置在武将牌上，称为“权”。出牌阶段，你可以用任意数量的手牌与等量的“权”交换，每阶段限一次。",
	["werpo"] = "權",
	["v5zili"] = "自立",
	[":v5zili"] = "觉醒技，回合开始阶段开始时，若你的“权”为四张或更多时，你必须减1点体力上限，并永久获得技能“排异”（回合结束阶段，将一张“权”移动到任何合理的区域，若不是你的区域，你可以摸一张牌）。",
	["#ZiLiWake"] = "%from 的权达到 %arg 张，觉醒技【%arg2】触发",
	["v5paiyi"] = "排異",
	[":v5paiyi"] = "回合结束阶段，你可以将一张“权”移动到任何合理的区域，若不是你的区域，你可以摸一张牌。",
	["v5paiyi:judge"] = "判定区",
	["v5paiyi:hand"] = "手牌区",
	["v5paiyi:equip"] = "装备区",
	["v5paiyi:werpo"] = "红灯区",

	["cv:v5zhonghui"] = "",
	["$v5zhenggong"] = "伐逆之任，何不予吾？",
	["$v5quanji1"] = "将军请留步。", -- 发动拼点
	["$v5quanji2"] = "将军劳苦功高，宜当暂歇。", -- 赢
	["$v5quanji3"] = "汝执意如此，莫要后悔。", -- 没赢
	["$v5baijiang"] = "请兵拜将，领军伐逆；破蜀之日，功成之时！",
	["$v5yexin1"] = "吾今功高，主必忌之！", -- 受到伤害
	["$v5yexin2"] = "主畏吾谋，不可不寻后路！", -- 交换牌
	["$v5zili"] = "大权在握，功高振军；自立蜀中，以图天下！",
	["$v5paiyi1"] = "若不成，退居蜀中，亦可为王！", -- 放进自己的区域
	["$v5paiyi2"] = "若事成，会师洛阳，天下可定！", -- 放进其他人的区域并摸牌
	["~v5zhonghui"] = "天之亡吾，争有何用……",

-- test
	["test"] = "测试",
	["undead"] = "不死之身",
	["goaway"] = "投降离开",

	["#zhibasunquan"] = "牛逼的贤君",
	["zhibasunquan"] = "制霸孙权",
	["super_zhiheng"] = "制霸",
	[":super_zhiheng"] = "出牌阶段，你可以弃置任意数量的牌，然后摸取等量的牌。每阶段可用X+1次，X为你已损失的体力值。",
	["$super_zhiheng1"] = "容我三思。",
	["$super_zhiheng2"] = "且慢！",
	["~zhibasunquan"] = "父亲…大哥…仲谋溃矣……",

	["#wuxingzhuge"] = "牛逼的丞相",
	["wuxingzhuge"] = "五星诸葛",
	["super_guanxing"] = "超级观星",
	[":super_guanxing"] = "回合开始阶段，你可以观看牌堆顶的5张牌，将其中任意数量的牌以任意顺序置于牌堆顶，其余则以任意顺序置于牌堆底。",
	["$super_guanxing1"] = "观今夜天象，知天下大事。",
	["$super_guanxing2"] = "知天易，逆天难。",
	["~wuxingzhuge"] = "将星陨落……天命难违……",

	["#dunkengcaoren"] = "大司马",
	["dunkengcaoren"] = "蹲坑曹仁",
	["super_jushou"] = "坚守",
	[":super_jushou"] = "回合结束阶段开始时，你可以摸五张牌，然后将你的武将牌翻面",

	["#sp_shenzhaoyun"] = "神威如龙",
	["sp_shenzhaoyun"] = "高达一号",
	["&sp_shenzhaoyun"] = "神赵云",
	["illustrator:sp_shenzhaoyun"] = "巴萨小马",
	["nosjuejing"] = "绝境",
	[":nosjuejing"] = "<b>锁定技</b>，摸牌阶段，你不摸牌。每当你的手牌数变化后，若你的手牌数不为4，你须将手牌补至或弃置至四张。",
	["noslonghun"] = "龙魂",
	[":noslonghun"] = "你可以将一张牌按以下规则使用或打出：<font color=\"red\">♥</font>当【桃】；<font color=\"red\">♦</font>当火【杀】；♠当【无懈可击】；♣当【闪】。回合开始阶段开始时，若其他角色的装备区内有【青釭剑】，你可以获得之。",
	["#noslonghun_duojian"] = "龙魂",
	[":#noslonghun_duojian"] = "回合开始阶段开始时，若其他角色的装备区内有【青釭剑】，你可以获得之。",

	["cv:sp_shenzhaoyun"] = "猎狐",
	["$nosjuejing"] = "龙战于野,其血玄黄",
	["$noslonghun1"] = "金甲映日,驱邪祛秽", -- spade
	["$noslonghun2"] = "腾龙行云,首尾不见", -- club
	["$noslonghun3"] = "潜龙于渊,涉灵愈伤", -- heart
	["$noslonghun4"] = "千里一怒,红莲灿世", -- diamond
	["~sp_shenzhaoyun"] = "血染鳞甲,龙坠九天",

	["#sujiang"] = "金童",
	["sujiang"] = "素将",
	["illustrator:sujiang"] = "火凤燎原",
	["#sujiangf"] = "玉女",
	["sujiangf"] = "素将(女)",
	["illustrator:sujiangf"] = "轩辕剑",
}