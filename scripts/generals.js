var table = {
    // Îº¹úÎä½«
    "caocao" : "²Ü²Ù",
    "zhangliao" : "ÕÅÁÉ",
    "guojia" : "¹ù¼Î",
    "xiahoudun" : "ÏÄºîª",
    "simayi" : "Ë¾ÂíÜ²",
    "xuchu" : "ĞíñÒ",
    "zhenji" : "Õç¼§",

    //Êñ¹úÎä½«
    "liubei" : "Áõ±¸",
    "guanyu" : "¹ØÓğ",
    "zhangfei" : "ÕÅ·É",
    "zhaoyun" : "ÕÔÔÆ",
    "machao" : "Âí³¬",
    "zhugeliang" : "Öî¸ğÁÁ",
    "huangyueying" : "»ÆÔÂÓ¢",

    // Îâ¹úÎä½«
    "sunquan" : "ËïÈ¨",
    "zhouyu" : "ÖÜè¤",
    "lumeng" : "ÂÀÃÉ",
    "luxun" : "Â½Ñ·",
    "ganning" : "¸ÊÄş",
    "huanggai" : "»Æ¸Ç",
    "daqiao" : "´óÇÇ",
    "sunshangxiang" : "ËïÉĞÏã",

    // ÈºĞÛ
    "lubu" : "ÂÀ²¼",
    "huatuo" : "»ªÙ¢",
    "diaochan" : "õõ²õ",

    // ÊÆÁ¦
    "wei" : "Îº",
    "shu" : "Êñ",
    "wu" : "Îâ",
    "qun" : "Èº",

    // ¼¼ÄÜ
    "jianxiong" : "¼éĞÛ",
    "hujia" : "»¤¼İ",
    "tuxi" : "Í»Ï®",
    "tiandu" : "Ìì¶Ê",
    "yiji" : "ÒÅ¼Æ",
    "ganglie" : "¸ÕÁÒ",

    "rende" : "ÈÊµÂ",
    "jijiang" : "¼¤½«",
    "wusheng" : "ÎäÊ¥",
    "paoxiao" : "ÅØÏø",
    "longdan" : "Áúµ¨",
    "tieji" : "ÌúÆï",
    "mashu" : "ÂíÊõ",
};

sgs.addTranslationTable(table);

// general's skills
//var jianxiong = sgs.addSkill("jianxiong");
//var hujia = sgs.addSkill("hujia");

var caocao = sgs.addGeneral("caocao", "wei");
//caocao.skills = [ jianxiong, hujia ];

var zhangliao = sgs.addGeneral("zhangliao", "wei");
var guojia = sgs.addGeneral("guojia", "wei", 3);
var xiahoudun = sgs.addGeneral("xiahoudun", "wei");
var simayi = sgs.addGeneral("simayi", "wei", 3);
var xuchu = sgs.addGeneral("xuchu", "wei");
var zhenji = sgs.addGeneral("zhenji", "wei", 3, false);

var liubei = sgs.addGeneral("liubei", "shu");
var guanyu = sgs.addGeneral("guanyu", "shu");
var zhangfei = sgs.addGeneral("zhangfei", "shu");
var zhaoyun = sgs.addGeneral("zhaoyun", "shu");
var machao = sgs.addGeneral("machao", "shu");
var zhugeliang = sgs.addGeneral("zhugeliang", "shu", 3);
var huangyueying = sgs.addGeneral("huangyueying", "shu", 3, false);

var sunquan = sgs.addGeneral("sunquan", "wu");
var zhouyu = sgs.addGeneral("zhouyu", "wu", 3);
var lumeng = sgs.addGeneral("lumeng", "wu");
var luxun = sgs.addGeneral("luxun", "wu", 3);
var huanggai = sgs.addGeneral("hunggai", "wu");
var ganning = sgs.addGeneral("ganning", "wu");
var daqiao = sgs.addGeneral("daqiao", "wu", 3, false);
var sunshangxiang = sgs.addGeneral("sunshangxiang", "wu", 3, false);


