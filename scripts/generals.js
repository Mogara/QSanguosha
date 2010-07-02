var table = {
    // 魏国武将
    "caocao" : "曹操",
    "zhangliao" : "张辽",
    "guojia" : "郭嘉",
    "xiahoudun" : "夏侯惇",
    "simayi" : "司马懿",
    "xuchu" : "许褚",
    "zhenji" : "甄姬",

    //蜀国武将
    "liubei" : "刘备",
    "guanyu" : "关羽",
    "zhangfei" : "张飞",
    "zhaoyun" : "赵云",
    "machao" : "马超",
    "zhugeliang" : "诸葛亮",
    "huangyueying" : "黄月英",

    // 吴国武将
    "sunquan" : "孙权",
    "zhouyu" : "周瑜",
    "lumeng" : "吕蒙",
    "luxun" : "陆逊",
    "ganning" : "甘宁",
    "huanggai" : "黄盖",
    "daqiao" : "大乔",
    "sunshangxiang" : "孙尚香",

    // 群雄
    "lubu" : "吕布",
    "huatuo" : "华佗",
    "diaochan" : "貂蝉",

    // 势力
    "wei" : "魏",
    "shu" : "蜀",
    "wu" : "吴",
    "qun" : "群",

    // 技能
    "jianxiong" : "奸雄",
    "hujia" : "护驾",
    "tuxi" : "突袭",
    "tiandu" : "天妒",
    "yiji" : "遗计",
    "ganglie" : "刚烈",
    "fankui" : "反馈",
    "guicai" : "鬼才",
    "luoyi" :"裸衣",
    "luoshen" :"洛神",
    "qingguo" : "倾国",

    "rende" : "仁德",
    "jijiang" : "激将",
    "wusheng" : "武圣",
    "paoxiao" : "咆哮",
    "longdan" : "龙胆",
    "tieji" : "铁骑",
    "mashu" : "马术",
    "guanxing" : "观星",
    "kongcheng" : "空城",
    "jizhi" : "集智",
    "qicai" : "奇才",

    "zhiheng" : "制衡",
    "jiuyuan" : "救援",
    "yingzi" : "英姿",
    "fanjian" : "反间",
    "keji" : "克己",
    "qianxun" : "谦逊",
    "lianying" : "连营",
    "qixi" : "奇袭",
    "kurou" : "苦肉",
    "guose" : "国色",
    "liuli" : "流离",
    "lianyin" :"联姻",
    "xiaoji" : "枭姬",

    "wushuang" : "无双",
    "qingnang" : "青囊",
    "jijiu" : "急救",
    "lijian" : "离间",
    "biyue" : "闭月"
};

sgs.addTranslationTable(table);

sgs.pixmap_dir = "generals";

// general's skills
//var jianxiong = sgs.addSkill("jianxiong");
//var hujia = sgs.addSkill("hujia");

var caocao = sgs.addGeneral("caocao!", "wei");
//caocao.skills = [ jianxiong, hujia ];

var zhangliao = sgs.addGeneral("zhangliao", "wei");
var guojia = sgs.addGeneral("guojia", "wei", 3);
var xiahoudun = sgs.addGeneral("xiahoudun", "wei");
var simayi = sgs.addGeneral("simayi", "wei", 3);
var xuchu = sgs.addGeneral("xuchu", "wei");
var zhenji = sgs.addGeneral("zhenji", "wei", 3, false);

var liubei = sgs.addGeneral("liubei!", "shu");
var guanyu = sgs.addGeneral("guanyu", "shu");
var zhangfei = sgs.addGeneral("zhangfei", "shu");
var zhaoyun = sgs.addGeneral("zhaoyun", "shu");
var machao = sgs.addGeneral("machao", "shu");
var zhugeliang = sgs.addGeneral("zhugeliang", "shu", 3);
var huangyueying = sgs.addGeneral("huangyueying", "shu", 3, false);

var sunquan = sgs.addGeneral("sunquan!", "wu");
var zhouyu = sgs.addGeneral("zhouyu", "wu", 3);
var lumeng = sgs.addGeneral("lumeng", "wu");
var luxun = sgs.addGeneral("luxun", "wu", 3);
var huanggai = sgs.addGeneral("huanggai", "wu");
var ganning = sgs.addGeneral("ganning", "wu");
var daqiao = sgs.addGeneral("daqiao", "wu", 3, false);
var sunshangxiang = sgs.addGeneral("sunshangxiang", "wu", 3, false);

var lubu = sgs.addGeneral("lubu", "qun");
var huatuo = sgs.addGeneral("huatuo", "qun", 3);
var diaochan = sgs.addGeneral("diaochan", "qun", 3, false);

