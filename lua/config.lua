--[[********************************************************************
	Copyright (c) 2013-2014 - QSanguosha-Rara

  This file is part of QSanguosha-Hegemony.

  This game is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 3.0
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  See the LICENSE file for more details.

  QSanguosha-Rara
*********************************************************************]]

-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
	kingdoms = { "wei", "qun", "shu", "wu", "god" },
	kingdom_colors = {
		wei = "#547998",
		shu = "#D0796C",
		wu = "#4DB873",
		qun = "#8A807A",
		god = "#96943D",
	},

	skill_colors = {
		compulsory = "#0000FF",
		once_per_turn = "#008000",
		limited = "#FF0000",
		head = "#00FF00",
		deputy = "#00FFFF",
		array = "#800080",
		lord = "#FFA500",
	},

	-- Sci-fi style background
	--dialog_background_color = "#49C9F0";
	--dialog_background_alpha = 75;
	dialog_background_color = "#D6E7DB";
	dialog_background_alpha = 255;

	package_names = {
		"StandardCard",
		"FormationEquip",
		"MomentumEquip" ,
		"StrategicAdvantage",
		--"StrategicAdvantageTest",

		"Standard",
		"Formation",
		"Momentum",
		"Test",
		"JiangeDefense"
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
		"啦啦失恋过", --啦啦SLG
		"Fsu0213", --Fsu0413
		"凌电信", --凌天翼
		"元嘉体",
		"萌豚紙",  --豚紙
		"女王神·Slob", --女王受·虫
		"Double_Bit！", --Double_Bit？
		"爱上穹妹的Jia", --爱上穹妹的某
		"没驾照开不了车", --开不了车
		"写书的独孤安河", --独孤安河
		"百年东南西北风", --百年东风
		"Paracel_00发", --Paracel_007
		"haveago823" , --haveatry823
		"离人泪026", --lrl026
		"墨宣砚韵", --a late developer
		"忧郁のlzxqqqq", --忧郁的月兔（lzxqqqq）
		--"来一口-水饺",
		--"甄钰月儿",
		"卍brianのvong卍", --卍冰の羽卍
		"五毛羽", --arrow羽
		"大同人陈家祺" , --陈家祺大同中心
		"fsu0415" , --你妹大神
		"麦当劳" , --果然萝卜斩
		"高调的富妮妮" , --低调的付尼玛
		"☆№Ｌ36×李Ｊ№★" , --☆№Ｌ糾×結Ｊ№★
		"ACG杀手", --hmqgg
		"Nagisa乐意", --Nagisa_Willing
		"0o叮咚咚叮o0", --0o叮咚Backup
		"医治曙光", --医治永恒（曙光）
		"甄姬真姬", --甄姬真妓（日月小辰）
		"tangjs我爱你", --tangjs520
		"帷幕之下问心云",
		"普肉", --Procellarum
		"大内总管KK", --KenKic
		"叶落孤舟",
		"晓月的微信", --晓月的泪痕
		"Xasy-Po-Love", --Easy-To-Love（XPL）
		"小修司V", --小休斯
		"清风不屈一对10", --清风弄错流年
		"非凡借刀教做人", --非凡神力
		"高城和二", --takashiro
		"tan∠ANY", --任意角的正切
		"刘恒飞翔", --恒星飞翔
		"寂镜Jnrio",
		"人啊环境", --rahj
		"良家大少", --祝家大少
		"禽受张", --老张
		"孝弯", --孝直
		"鱼纸酱" --鱼
	},
}
