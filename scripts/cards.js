var table = {
    "basic" : "基本牌",
    "equip" : "装备牌",
    "trick" : "锦囊牌",

    "spade" : "黑桃",
    "club" : "梅花",
    "heart" : "红桃",
    "diamond" : "方块",

    "weapon" : "武器",
    "armor" : "防具",
    "defensive_horse" : "防御马",
    "offensive_horse" : "进攻马",

    "delayed_trick" : "延时锦囊",
    "aoe" : "范围效果",

    "slash" : "杀",
    "jink" : "闪",
    "peach" : "桃",

    "crossbow" : "诸葛连弩",
    "double_sword" : "雌雄双股剑",
    "qinggang_sword" : "青釭剑",
    "blade" : "青龙偃月刀",
    "spear" : "丈八蛇矛",
    "axe" : "贯石斧",
    "halberd" : "方天画戟",
    "kylin_bow" : "麒麟弓",

    "eight_diagram" : "八卦",
    "jueying" : "绝影",
    "dilu" : "的卢",
    "zhuahuangfeidian" : "爪黄飞电",
    "chitu" : "赤兔",
    "dawan" : "大宛",
    "zixing" : "紫骍",

    "amazing_grace" : "五谷丰登",
    "god_salvation" : "桃园结义",
    "savage_assault" : "南蛮入侵",
    "archery_attack" : "万箭齐发",
    "collateral" : "借刀杀人",
    "ex_nihilo" : "无中生有",
    "duel" : "决斗",
    "nullification" : "无懈可击",
    "snatch" : "顺手牵羊",
    "dismantlement" : "过河拆桥",
    "lightning" : "闪电",
    "indulgence" : "乐不思蜀",
};

sgs.addTranslationTable(table);

sgs.pixmap_dir = "cards";

// basic cards
var slash = sgs.addCardClass("slash", "basic");
var jink = sgs.addCardClass("jink", "basic");
var peach = sgs.addCardClass("peach", "basic");

// weapon cards, added by attack range
var crossbow = sgs.addCardClass("crossbow", "equip", "weapon");
var double_sword = sgs.addCardClass("double_sword", "equip", "weapon");
var qinggang_sword = sgs.addCardClass("qinggang_sword", "equip", "weapon");
var blade = sgs.addCardClass("blade", "equip", "weapon");
var spear = sgs.addCardClass("spear", "equip", "weapon");
var axe = sgs.addCardClass("axe", "equip", "weapon");
var halberd = sgs.addCardClass("halberd", "equip", "weapon");
var kylin_bow = sgs.addCardClass("kylin_bow", "equip", "weapon");

crossbow.range = 1;
double_sword.range = qinggang_sword.range = 2;
blade.range = spear.range = axe.range = 3;
halberd.range = 4;
kylin_bow.range = 5;

// shield card, only one
var eight_diagram = sgs.addCardClass("eight_diagram", "equip", "armor");

// horses, +1 horses and -1 horses
var jueying = sgs.addCardClass("jueying", "equip", "defensive_horse");
var dilu = sgs.addCardClass("dilu", "equip", "defensive_horse");
var zhuahuangfeidian = sgs.addCardClass("zhuahuangfeidian", "equip", "defensive_horse");

var chitu = sgs.addCardClass("chitu", "equip", "offensive_horse");
var zixing = sgs.addCardClass("zixing", "equip", "offensive_horse");
var dawan = sgs.addCardClass("dawan","equip", "offensive_horse");

// trick cards, added by target number
// target all
var amazing_grace = sgs.addCardClass("amazing_grace", "trick");
var god_salvation = sgs.addCardClass("god_salvation", "trick");
// target all except source (AOE)
var savage_assault = sgs.addCardClass("savage_assault", "trick", "aoe");
var archery_attack = sgs.addCardClass("archery_attack", "trick", "aoe");
// target two
var collateral = sgs.addCardClass("collateral", "trick");
// target one
var ex_nihilo = sgs.addCardClass("ex_nihilo", "trick");
var duel = sgs.addCardClass("duel", "trick");
var nullification = sgs.addCardClass("nullification", "trick");
var snatch = sgs.addCardClass("snatch", "trick");
var dismantlement = sgs.addCardClass("dismantlement", "trick");
// two delayed trick cards
var lightning = sgs.addCardClass("lightning", "trick", "delayed_trick");
var indulgence = sgs.addCardClass("indulgence", "trick", "delayed_trick");

var card_names = [
     // spade suit
    "duel", "lightning", // A
    "double_sword", "eight_diagram", // 2
    "snatch", "dismantlement", // 3
    "snatch", "dismantlement", // 4
    "jueying", "blade", // 5
    "qinggang_sword", "indulgence", // 6
    "slash", "savage_assault", // 7
    "slash", "slash", // 8
    "slash", "slash", // 9
    "slash", "slash", // 10
    "snatch", "nullification", // J
    "dismantlement", "spear", // Q
    "savage_assault", "dawan", // K

    // club
    "duel", "crossbow", // A
    "slash", "eight_diagram", // 2
    "slash", "dismantlement", // 3
    "slash", "dismantlement", // 4
    "slash", "dilu", // 5
    "slash", "indulgence", // 6
    "slash", "savage_assault", // 7
    "slash", "slash", // 8
    "slash", "slash", // 9
    "slash", "slash", // 10
    "slash", "slash", // J
    "collateral", "nullification", // Q
    "collateral", "nullification", // K

    // heart suit
    "god_salvation", "archery_attack", // A
    "jink", "jink", // 2
    "peach", "amazing_grace", // 3
    "peach", "amazing_grace", // 4
    "kylin_bow", "chitu", // 5
    "peach", "indulgence", // 6
    "peach", "ex_nihilo", // 7
    "peach", "ex_nihilo", // 8
    "peach", "ex_nihilo", // 9
    "slash", "slash", // 10
    "slash", "ex_nihilo", // J
    "dismantlement", "peach", // Q
    "zhuahuangfeidian", "jink", // K

    // diamond
    "duel", "crossbow", // A
    "jink", "jink", // 2
    "jink", "snatch", // 3
    "jink", "snatch", // 4
    "jink", "axe", // 5
    "slash", "jink", // 6
    "slash", "jink", // 7
    "slash", "jink", // 8
    "slash", "jink", // 9
    "slash", "jink", // 10
    "jink", "jink", // J
    "peach", "halberd", // Q
    "slash", "zixing", // K
];

for(i=0;i<card_names.length;i++){
    var name = card_names[i];
    var suit = Math.floor(i / (card_names.length /4));
    var number = (i % (card_names.length / 4))/2 + 1;
    sgs.addCard(name, suit, number);
}


