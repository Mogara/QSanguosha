-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
	kingdoms = { "wei", "shu", "wu", "qun", "god" },
	kingdom_colors = {
		wei = "#547998",
		shu = "#D0796C",
		wu = "#4DB873",
		qun = "#8A807A",
		god = "#96943D",
	},
	
	package_names = {
		"StandardCard",
		"formation_equip",

		"Standard",
		"Formation",
		"Test"
	},

	hulao_packages = {
		"standard",
		"wind"
	},

	xmode_packages = {
		"standard",
		"wind",
		"fire",
		"nostal_standard",
		"nostal_wind",
	},

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
	
	robot_names = {
		"啦啦SLG",
		"Fsu0413",
		"小猪翼爱啦啦",
		"元嘉体",
		"豚紙愛啦啦",
		"女王受·虫",
		"doublebit",
		"爱上穹妹的某",
		"开不了车",
		"独孤安河",
		"百年东风",
		"Paracel_007",
		"haveatry823" ,
		"lrl026",
		"墨宣砚韵",
		"忧郁の月兔君",
		"来一口-水饺",
		"甄钰月儿",
		"卍冰の羽卍",
		"五毛羽君",
		"McDon",
		"陈家祺" ,
		"你妹大神"
	},

	roles_ban = {
		"vs_xiahoudun",
		"vs_guanyu",
		"vs_zhaoyun",
		"vs_lvbu",
		"kof_zhangliao",
		"kof_xuchu",
		"kof_zhenji",
		"kof_xiahouyuan",
		"kof_guanyu",
		"kof_machao",
		"kof_nos_huangyueying",
		"kof_huangzhong",
		"kof_jiangwei",
		"kof_menghuo",
		"kof_zhurong",
		"kof_sunshangxiang",
		"kof_nos_diaochan",
	},

	kof_ban = {
		"sunquan",
		"huatuo"
	},

	hulao_ban = {
		"yuji"
	},

	xmode_ban = {
		"huatuo",
		"zhangjiao",
		"caoren",
		"zhoutai",
		"yuji",
		"liubei",
		"diaochan",
		"huangyueying",
		"st_yuanshu",
		"st_huaxiong",
		"nos_zhangjiao",
		"nos_yuji",
	},

	basara_ban = {
	},
	
	hegemony_ban = {
	},

	pairs_ban = {
		--[["huatuo", "zhoutai", "zuoci", "bgm_pangtong", "shencaocao", "liaohua", "nos_zhoutai",
		"+luboyan",
		"simayi+zhenji", "simayi+dengai",
		"caoren+shenlvbu", "caoren+caozhi", "caoren+bgm_diaochan", "caoren+bgm_caoren", "caoren+neo_caoren", "caoren+nos_caoren",
		"nos_caoren+shenlvbu", "nos_caoren+caozhi", "nos_caoren+bgm_diaochan", "nos_caoren+bgm_caoren", "nos_caoren+neo_caoren",
		"guojia+dengai",
		"zhenji+zhangjiao", "zhenji+shensimayi", "zhenji+zhugejin", "zhenji+nos_zhangjiao", "zhenji+nos_wangyi",
		"zhanghe+yuanshu",
		"dianwei+weiyan",
		"dengai+zhangjiao", "dengai+shensimayi", "dengai+zhugejin", "dengai+nos_zhangjiao",
		"zhangfei+huanggai", "zhangfei+zhangchunhua", "zhangfei+nos_zhangchunhua",
		"zhugeliang+xushu", "zhugeliang+nos_xushu",
		"huangyueying+wolong", "huangyueying+ganning", "huangyueying+huanggai", "huangyueying+yuanshao", "huangyueying+yanliangwenchou",
		"huangzhong+xusheng",
		"wolong+luxun", "wolong+zhangchunhua", "wolong+nos_huangyueying", "wolong+nos_zhangchunhua",
		"sunquan+sunshangxiang",
		"ganning+nos_huangyueying",
		"lvmeng+yuanshu",
		"huanggai+sunshangxiang", "huanggai+yuanshao", "huanggai+yanliangwenchou", "huanggai+dongzhuo",
		    "huanggai+wuguotai", "huanggai+guanxingzhangbao", "huanggai+huaxiong", "huanggai+xiahouba",
		    "huanggai+nos_huangyueying", "huanggai+nos_guanxingzhangbao", "huanggai+neo_zhangfei",
		"luxun+yanliangwenchou", "luxun+guanxingzhangbao", "luxun+guanping", "luxun+heg_luxun",
		    "luxun+nos_liubei", "luxun+nos_yuji", "luxun+nos_guanxingzhangbao",
		"sunshangxiang+shensimayi", "sunshangxiang+heg_luxun",
		"sunce+guanxingzhangbao", "sunce+nos_guanxingzhangbao",
		"yuanshao+nos_huangyueying",
		"yanliangwenchou+zhangchunhua", "yanliangwenchou+nos_huangyueying", "yanliangwenchou+nos_zhangchunhua",
		"dongzhuo+shenzhaoyun", "dongzhuo+wangyi", "dongzhuo+diy_wangyuanji", "dongzhuo+nos_zhangchunhua", "dongzhuo+nos_wangyi",
		"shencaocao+caozhi",
		"shenlvbu+caozhi", "shenlvbu+liaohua", "shenlvbu+bgm_diaochan", "shenlvbu+bgm_caoren", "shenlvbu+neo_caoren",
		"shenzhaoyun+huaxiong",
		"caozhi+bgm_diaochan", "caozhi+bgm_caoren", "caozhi+neo_caoren",
		"gaoshun+zhangchunhua", "gaoshun+nos_zhangchunhua",
		"wuguotai+caochong",
		"zhangchunhua+guanxingzhangbao", "zhangchunhua+guanping", "zhangchunhua+xiahouba", "zhangchunhua+zhugeke",
		    "zhangchunhua+heg_luxun", "zhangchunhua+nos_liubei", "zhangchunhua+nos_yuji", "zhangchunhua+nos_guanxingzhangbao", "zhangchunhua+neo_zhangfei",
		"guanxingzhangbao+bgm_zhangfei", "guanxingzhangbao+nos_zhangchunhua",
		"liaohua+bgm_diaochan",
		"guanping+nos_zhangchunhua",
		"xiahouba+nos_zhangchunhua",
		"zhugeke+nos_zhangchunhua",
		"bgm_diaochan+bgm_caoren",
		"bgm_caoren+neo_caoren",
		"bgm_zhangfei+nos_guanxingzhangbao",
		"nos_liubei+nos_zhangchunhua",
		"nos_yuji+nos_zhangchunhua",
		"nos_zhangchunhua+heg_luxun", "nos_zhangchunhua+nos_guanxingzhangbao", "nos_zhangchunhua+neo_zhangfei",
		"caoren+dengshizai", "nos_caoren+dengshizai", "shenlvbu+dengshizai", "bgm_diaochan+dengshizai", "bgm_caoren+dengshizai", "neo_caoren+dengshizai",
		"jiangboyue+huangyueying", "jiangboyue+wolong", "jiangboyue+yuanshao",
		"jiangboyue+yanliangwenchou", "jiangboyue+ganning", "jiangboyue+luxun", "jiangboyue+zhanggongqi",
		"yt_caochong+caocao", "yuanshu+caocao",
		"lukang+nos_liubei", "lukang+wolong", "lukang+yuji", "jiangboyue+lukang", "lukang+yanliangwenchou", "lukang+guanxingzhangbao", "lukang+heg_luxun",
		"caoren+caizhaoji", "nos_caoren+caizhaoji", "bgm_caoren+caizhaoji", "neo_caoren+caizhaoji", "dengshizai+caizhaoji",
		"zhugejin+caizhaoji", "simayi+caizhaoji", "shensimayi+caizhaoji", "caozhi+caizhaoji", "shenlvbu+caizhaoji", "bgm_diaochan+caizhaoji",
		"wis_jiangwei+zhanggongqi", "luxun+zhanggongqi"]]
	},
	
	couple_lord = "caocao",
	couple_couples = {
		"caopi|caozhi+zhenji",
		"simayi+zhangchunhua",
		"liubei+ganfuren|sp_sunshangxiang",
		"zhangfei|bgm_zhangfei+xiahoushi|xiahoujuan",
		"zhugeliang|wolong+huangyueying",
		"menghuo+zhurong",
		"zhouyu+xiaoqiao",
		"lvbu|dongzhuo+diaochan",
		"sunjian+wuguotai",
		"sunce+daqiao",
		"sunquan+bulianshi",
		"diy_simazhao+diy_wangyuanji",
		"liuxie|diy_liuxie+fuhuanghou|as_fuhuanghou"
	}
}
