-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
	kingdoms = { "wei", "shu", "wu", "qun", "god" },
	package_names = {
		"StandardCard",
		"StandardExCard",
		"Maneuvering",
		"SPCard",
		"Nostalgia",
		"New3v3Card",

		"Standard",
		"Wind",
		"Fire",
		"Thicket",
		"Mountain",
		"God",
		"YJCM",
		"YJCM2012",
		"Assassins", 
		"Special3v3",
		"SP",
		"BGM",
		"Hegemony",
		"HegemonySP",
		"Ling",
		"NostalGeneral",
		"BGMDIY",
		"Yitian",
		"Wisdom",
		"Test"
	},

	color_wei = "#547998",
	color_shu = "#D0796C",
	color_wu = "#4DB873",
	color_qun = "#8A807A",
	color_god = "#96943D",

	easy_text = {
		"太慢了，做两个俯卧撑吧！",
		"快点吧，我等的花儿都谢了！",
		"高，实在是高！",
		"好手段，可真不一般啊！",
		"哦，太菜了。水平有待提高。",
		"你会不会玩啊？！",
		"嘿，一般人，我不使这招。",
		"呵，好牌就是这么打地！",
		"杀！神挡杀神！佛挡杀佛！",
		"你也忒坏了吧？！"
	},

	kof_ban = {
		"sunquan",
		"huatuo"
	},

	basara_ban = {
		"dongzhuo",
		"zuoci",
		"shenzhugeliang",
		"shenlvbu",
		"bgm_lvmeng",
		"zhanggongqi"
	},
	
	hegemony_ban = {
		"xiahoujuan"
	},

	pairs_ban = {
		"huatuo", "zhoutai", "zuoci", "bgm_pangtong", "shencaocao", "liaohua",
		"+luboyan",
		"simayi+zhenji", "simayi+dengai",
		"caoren+shenlvbu", "caoren+caozhi", "caoren+bgm_diaochan", "caoren+bgm_caoren", "caoren+neo_caoren",
		"guojia+dengai",
		"zhenji+zhangjiao", "zhenji+shensimayi", "zhenji+wangyi", "zhenji+zhugejin",
		"zhanghe+yuanshu",
		"dianwei+weiyan",
		"dengai+zhangjiao", "dengai+shensimayi", "dengai+zhugejin",
		"liubei+luxun", "liubei+zhangchunhua", "liubei+nos_zhangchunhua",
		"zhangfei+huanggai", "zhangfei+zhangchunhua", "zhangfei+nos_zhangchunhua",
		"zhugeliang+xushu", "zhugeliang+nos_xushu",
		"huangyueying+wolong", "huangyueying+ganning", "huangyueying+huanggai", "huangyueying+yuanshao", "huangyueying+yanliangwenchou",
		"huangzhong+xusheng",
		"wolong+luxun", "wolong+zhangchunhua", "wolong+nos_zhangchunhua",
		"sunquan+sunshangxiang",
		"lvmeng+yuanshu",
		"huanggai+sunshangxiang", "huanggai+yuanshao", "huanggai+yanliangwenchou", "huanggai+dongzhuo", "huanggai+wuguotai", "huanggai+guanxingzhangbao", "huanggai+huaxiong", "huanggai+neo_zhangfei",
		"luxun+yuji", "luxun+yanliangwenchou", "luxun+guanxingzhangbao", "luxun+heg_luxun",
		"sunshangxiang+shensimayi", "sunshangxiang+heg_luxun",
		"sunce+guanxingzhangbao",
		"yanliangwenchou+zhangchunhua", "yanliangwenchou+nos_zhangchunhua",
		"dongzhuo+shenzhaoyun", "dongzhuo+nos_zhangchunhua", "dongzhuo+diy_wangyuanji",
		"yuji+zhangchunhua", "yuji+nos_zhangchunhua",
		"shenlvbu+caozhi", "shenlvbu+liaohua", "shenlvbu+bgm_diaochan", "shenlvbu+bgm_caoren", "shenlvbu+neo_caoren",
		"shenzhaoyun+huaxiong",
		"caozhi+bgm_diaochan", "caozhi+bgm_caoren", "caozhi+neo_caoren",
		"gaoshun+zhangchunhua", "gaoshun+nos_zhangchunhua",
		"zhangchunhua+guanxingzhangbao", "zhangchunhua+heg_luxun", "zhangchunhua+neo_zhangfei",
		"guanxingzhangbao+bgm_zhangfei", "guanxingzhangbao+nos_zhangchunhua",
		"liaohua+bgm_diaochan",
		"bgm_diaochan+bgm_caoren",
		"bgm_caoren+neo_caoren",
		"nos_zhangchunhua+heg_luxun", "nos_zhangchunhua+neo_zhangfei",
		"caoren+dengshizai", "shenlvbu+dengshizai", "bgm_diaochan+dengshizai", "bgm_caoren+dengshizai", "neo_caoren+dengshizai",
		"jiangboyue+huangyueying", "jiangboyue+wolong", "jiangboyue+yuanshao",
		"jiangboyue+yanliangwenchou", "jiangboyue+ganning", "jiangboyue+luxun", "jiangboyue+zhanggongqi",
		"caochong+caocao", "yuanshu+caocao",
		"lukang+liubei", "lukang+wolong", "lukang+yuji", "jiangboyue+lukang", "lukang+yanliangwenchou", "lukang+guanxingzhangbao", "lukang+heg_luxun",
		"caoren+caizhaoji", "bgm_caoren+caizhaoji", "neo_caoren+caizhaoji", "dengshizai+caizhaoji",
		"zhugejin+caizhaoji", "simayi+caizhaoji", "shensimayi+caizhaoji", "caozhi+caizhaoji", "shenlvbu+caozhaoji", "bgm_diaochan+caizhaoji",
		"wis_jiangwei+zhanggongqi", "luxun+zhanggongqi"
	},
	
	couple_lord = "caocao",
	couple_couples = {
		"caopi|caozhi+zhenji",
		"simayi+zhangchunhua",
		"liubei+ganfuren|sp_sunshangxiang",
		"zhugeliang|wolong+huangyueying",
		"menghuo+zhurong",
		"zhouyu+xiaoqiao",
		"lvbu|dongzhuo+diaochan",
		"sunjian+wuguotai",
		"sunce+daqiao",
		"sunquan+bulianshi",
		"diy_simazhao+diy_wangyuanji"
	}
}
