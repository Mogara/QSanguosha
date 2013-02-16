
-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
	version = "20130208",
	version_name = "金蛇版",
	mod_name = "official",
	kingdoms = { "wei", "shu", "wu", "qun", "god"},
	package_names = {
		"StandardCard",
		"StandardExCard",
		"Maneuvering",
		"SPCard",
		"Nostalgia",
		"New3v3Card",
		"HegemonyCard",
		"YitianCard",
		"Joy",
		"Disaster",
		"JoyEquip",

		"Standard",
		"Wind",
		"Fire",
		"Thicket",
		"Mountain",
		"God",
		"SP",
		"YJCM",
		"YJCM2012",
		"Special3v3",
		"BGM",
		"Yitian",
		"Wisdom",
--		"Ling",
		"Assassins",
		"Olympics",
		"Hegemony",
		"Paster",
		"SanDZhimeng",
		"NostalGeneral",
		"ChangbanSlope",
		"Danchuang",
		"Test",
	},

	scene_names = {
		"Guandu",
		"Fancheng",
		"Couple",
		"Zombie",
		"Impasse",
		"Custom",
	},

	ai_names = {
		"太阳神的三国杀",
		"启姐的小雏田",
		"KenKic的充气女仆",
		"米叔的种子",
		"海泡叉的乱码",
		"威廉古堡",
		"葱娘家的灵魂手办",
		"donle的最后之作",
		"天霜雪舞的烤萝莉",
		"逮捕麻麻的手铐",
		"冢冢的青藤",
		"安歧的小黑屋",
		"妙妙的思绪",
		"氢弹的狂风甲",
		"战栗的皇叔",
		"讨厌的核弹",
		"七爷的觉醒",
		"听雨的救赎",
		"科比挂的柯南",
		"导线的电阻",
		"QB的契约",
		"吉祥物小萨",
		"小二的肉饼",
		"Slob的杀虫剂",
		"克拉克的跑动投",
		"天下孟德的霸气",
		"早苗的假面",
		"墨韵的诅咒",
		"被和谐的XX生",
		"沾血的青苹果",
		"圆月的百合子",
		"海南的椰子",
		"喵一夏的幸福",
		"不踢足球的小翼",
		"爱上某的穹妹",
		"役满国士无双",
		"卖萌的豚纸",
		"江西安义的雷海",
		"中条老道的大头贴",
		"超级塞克洛",
		"姨妈家的小灰灰",
		"神配合的大A和小A",
		"盘踞神杀的三只猫",
		"爱上小衣的殃",
		"琉星的轨迹",
		"传说中的小雪",
		"龙凤呈祥",
		"总司令的拆迁队",
		"纠结的金田一君",
		"失落的三好学生",
		"肉酱茧",
	-- 感谢一路走来，为太阳神三国杀付出和支持过的朋友们！
	},

	color_wei = "#547998",
	color_shu = "#D0796C",
	color_wu = "#4DB873",
	color_qun = "#8A807A",
	color_god = "#96943D",

	mini_max = 44 -- 此处以Config.S_MINI_MAX_COUNT为准
}

scenario = { -- 非身份局模式下的一些设置
	savsa_packages = { -- 3v3模式：设置加载的扩展包
		"standard",
		"wind",
	},
	couple_lord = "caocao", -- 夫妻鞋展：设置主公
	couple_spouse = { -- 设置新的夫妻
		"simayi+guojia",
		"ganfuren+liubei",
		"hanxiandi+fushi",
	},
}
ban_list = { -- 初始禁表设置
	roles_ban = { -- 身份局单禁
		"bgm_pangtong",
		"liuxie"
	},
	kof_ban = { -- 1v1模式
		"huatuo",
		"lvmeng"
	},
	savsa_ban = { -- 3v3模式
		"yuji",
		"caizhaoji",
	},
	basara_ban = { -- 暗将模式
		"dongzhuo",
		"zuoci",
		"shenzhugeliang",
		"shenlvbu",
		"zhanggongqi",
		"huaxiong",
		"bgm_lvmeng"
	},
	hegemony_ban = { -- 国战模式（注意所有的神势力也会被禁用）
		"dongzhuo",
		"zuoci",
		"zhanggongqi",
		"huaxiong",
		"bgm_lvmeng",
		"xiahoujuan",
		"zhugejin"
	},
	pairs_ban = { -- 双将
	--	双将全禁
		"shencaocao",
		"dongzhuo", -- 董卓：体力值多
		"zuoci", -- 左慈：易产生BUG
		"zhoutai", -- 周泰：耐久太高
		"liaohua", -- 廖化：易产生BUG
		"bgm_pangtong", -- SB庞统：不解释，已经身份单禁了
	--	副将禁用
		"+luboyan", -- 陆伯言：变身BUG
	--	特定禁用
		"zhenji+zhangjiao", -- 无限刷牌
		"zhenji+simayi", -- 无限刷牌
		"huanggai+wuguotai", -- 无限回复
		"luxun+liubei", -- 无限刷牌
		"luxun+yuji", -- 无限刷牌
		"zhugejin+zhenji", -- 无限刷牌
		"huanggai+yuanshao", -- 超强爆发
		"luxun+wolong", -- 强力刷牌
		"luxun+daqiao", -- 超强防杀
		"huangyueying+wolong", -- 强力刷牌
		"huangyueying+yuanshao", -- 强力刷牌
		"huangyueying+ganning", -- 强力刷牌
		"huangyueying+yanliangwenchou", -- 强力刷牌
		"yanliangwenchou+huanggai", -- 超强爆发
		"yanliangwenchou+sunce", -- 决斗激昂
		"dengai+guojia", -- 屯田天妒
		"dengai+simayi", -- BUG禁配
		"dengai+zhangjiao", -- BUG禁配
--		"dengai+shenzhugeliang"
		"dengai+shensimayi", -- BUG禁配
		"zhugejin+dengai", -- BUG禁配
		"weiyan+huanggai", -- 吸血回复
		"sunquan+noslingtong", -- 制衡旋风
		"sunquan+sunshangxiang", -- 脱衣服睡觉
		"wuguotai+guojia", -- 卖血自补，高收益卖血将+高回复将
		"wuguotai+xunyu", -- 卖血自补，高收益卖血将+高回复将
		"huatuo+guojia", -- 几乎打不死，无限拖延时间
		"huatuo+xunyu", -- 几乎打不死，无限拖延时间
		"huatuo+xiahoujuan", -- 几乎打不死，无限拖延时间
		"yuanshu+zhanghe", -- 可避免庸肆负面效果
		"yuanshu+lvmeng", -- 可避免庸肆负面效果
		"caoren+shenlvbu", -- 缩短神愤延时
		"caozhi+shenlvbu", -- 缩短神愤延时
		"caoren+caozhi", -- 缩短据守延时
		"guanxingzhangbao+luxun", -- 辅助爆发
		"guanxingzhangbao+sunce", -- 辅助爆发
		"guanxingzhangbao+huanggai", -- 超强爆发
		"xushu+zhugeliang", -- 刀枪不入
		"nosxushu+zhugeliang", -- 刀枪不入
		"zhugejin+huatuo", -- 急救明哲
		"fazheng+xiahoudun", -- 双重肛裂
		"nosfazheng+xiahoudun", -- 双重肛裂

		"caochong+caocao",
		"caochong+yuanshu",
		"jiangboyue+huangyueying",
		"jiangboyue+wolong",
		"jiangboyue+yuanshao",
		"jiangboyue+yanliangwenchou",
		"jiangboyue+ganning",
		"jiangboyue+luxun",
		"jiangboyue+zhanggongqi",
		"jiangboyue+lukang",
		"lukang+liubei",
		"lukang+wolong",
		"lukang+yuji",
		"lukang+zhanggongqi",
		"zhanggongqi+luxun",
		"zhanggongqi+huatuo",
		"zhanggongqi+wisjiangwei",
		"caizhaoji+simayi",
		"caizhaoji+caoren",
		"caizhaoji+caozhi",
		"caizhaoji+zhugejin",
		"caizhaoji+shenlvbu",
		"caizhaoji+dengshizai",
		"dengshizai+caoren",
		"dengshizai+shenlvbu",
		"dengshizai+bgm_diaochan",
		"bgm_diaochan+caoren",
		"bgm_diaochan+shenlvbu",
		"bgm_diaochan+caizhaoji",
		"bgm_diaochan+bgm_caoren",
		"bgm_caoren+caoren",
		"bgm_caoren+caozhi",
		"bgm_caoren+shenlvbu",
		"bgm_caoren+dengshizai",
		"bgm_caoren+caizhaoji",
		"bgm_pangtong+huanggai",
		"bgm_zhangfei+guanyu",
		"bgm_liubei+zhugeliang"
	},
	forbid_packages = {
		"New3v3Card",
		"changban_slope",
		"test"
	},
}

mini_max = sgs.GetConfig("S_MINI_MAX_COUNT", config.mini_max)
for i=1, mini_max do
	local scene_name = ("MiniScene_%02d"):format(i)
	table.insert(config.scene_names, scene_name)
end

