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

-- this script is used to generate pages about developers automatically

about_us = {}

programmers = {
	'啦啦SLG',
	'Fsu0413',
	'女王受·虫',
	'hmqgg',
	'takashiro',
	'lrl026',
	'Para',
	'tangjs520'
}

ai_designers = {
	'lrl026',
	'lzxqqqq',
	'任意角的正切',
	'元嘉体',
	'peterli2012y'
}

art_designers = {
	'爱上穹妹的某',
	'36李',
	'低调的付尼玛',
	'张拯明寰',
	'82',
	'XXX',
	'KenKic',
	'洛神',
	'祝家大少'
}

assistants = {
	'Dear J'
}

audio_workers = {
	'豚紙',
	'<font color=red><b>墨宣砚韵</b></font>',
	'极光星逝',
	'両仪弑',
	'饮魂之殇',
	'孝直',
	'鱼梓酱',
	'doublebit',
	'尖子班的学生',
	'琴音'
}

about_us.width = 900
about_us.height = 600

HTMLTable = (require 'middleclass').class('HTMLTable')

function HTMLTable:initialize()
	self.rows = {}
end

function HTMLTable:addRow(...)
	table.insert(self.rows, {...})
end

function HTMLTable.encloseInTag(element, tag)
	if tag then
		return ("<%s>%s</%s>"):format(tag, element, tag)
	else
		return element
	end
end

function HTMLTable.encloseInTagWithProperties(element, tag, ...)
	return ("<%s %s>%s</%s>"):format(tag, table.concat({...}, " "), element, tag)
end

function HTMLTable.encloseInTagsOfDivId(element, ...)
	local result = element
	for _, v in ipairs({...}) do
		result = HTMLTable.encloseInTagWithProperties(result, "div", ('id="%s"'):format(v))
	end
	return result
end

function HTMLTable.encloseInTagsOfDivClass(element, ...)
	local result = element
	for _, v in ipairs({...}) do
		result = HTMLTable.encloseInTagWithProperties(result, "div", ('class="%s"'):format(v))
	end
	return result
end

function HTMLTable.encloseEachElementInTags(t, inner, outer)
	if type(t) ~= "table" then
		t = {t}
	end
	local result = {}
	for _, e in ipairs(t) do
		table.insert(result, HTMLTable.encloseInTag(HTMLTable.encloseInTag(e, inner), outer))
	end

	return table.concat(result)
end

function HTMLTable.encloseInTags(t, inner, outer)
	if type(t) ~= "table" then
		t = {t}
	end
	local result = {}
	for _, e in ipairs(t) do
		table.insert(result, HTMLTable.encloseInTag(e, inner))
	end

	result = table.concat(result)
	return HTMLTable.encloseInTag(result, outer)
end

function HTMLTable.createListingBlock(title, ...)
	title = HTMLTable.encloseInTagsOfDivClass(title, "title")
	local contents = {...}
	--if type(arg[1]) == "table" then contents = arg[1] end
	content = HTMLTable.encloseEachElementInTags(contents, "tt", "pre")
	content = HTMLTable.encloseInTagsOfDivClass(content, "content")
	return HTMLTable.encloseInTagsOfDivClass(title .. content, "listingblock")
end

function HTMLTable:__tostring()
	local t = {}
	for _, row in ipairs(self.rows) do
		local rowstring = HTMLTable.encloseInTags(row, "td", "tr")
		table.insert(t, rowstring)
	end

	return HTMLTable.encloseInTags(t, nil, "table")
end

function HTMLTable.getFileHead(title)
	local part1 = '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"'
		..'"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">'
		..'<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">'

	local part2_1 = HTMLTable.encloseInTag(title, "table")

	local part2_2 = '<link rel="stylesheet" type="text/css" href="style-sheet/page.css" />'
		..'<script src="ui-script/page.js"></script>'

	local part2 = HTMLTable.encloseInTag(part2_1 .. part2_2, "head")

	return part1 .. part2 .. '<body class="article">'
end

function HTMLTable.getFileFoot()
	return "</body></html>"
end

function HTMLTable.getHref(content, href)
	return ('<a target="_blank" href="%s">%s</a>'):format(href, content)
end

function HTMLTable.getImg(src, href, title)
	local img
	if title then
		img = ('<img border="0" src="%s" alt="%s" title="%s">'):format(src, title, title)
	else
		img = ('<img border="0" src="%s">'):format(src)
	end
	if href then
		img = HTMLTable.getHref(img, href)
	end
	return img
end

function getQunInfo(name, owner, idkey, imagekey)
	local result = {}
	table.insert(result, HTMLTable.encloseEachElementInTags(("%s %s"):format(name, owner), "tt", "pre"))
	table.insert(result, HTMLTable.encloseInTag("&nbsp;", "pre"))
	table.insert(result, HTMLTable.getImg("./image/system/developers/group.png",
		"http://shang.qq.com/wpa/qunwpa?idkey=" .. idkey, name))
	table.insert(result, HTMLTable.encloseEachElementInTags(
		HTMLTable.getImg(("./image/system/developers/qrcode-%s.png"):format(imagekey)), "tt", "pre"))
	table.insert(result, HTMLTable.encloseEachElementInTags("", "tt", "pre"))
	return table.concat(result)
end

function getTextWithOptionalLink(text, address)
	local result = text
	if address then
		result = result .. " " .. HTMLTable.getHref(address, address)
	end
	return HTMLTable.encloseEachElementInTags(result, "tt", "pre")
end

about_us.homepage = ""

function createHomePage()
	about_us.homepage = {}
	table.insert(about_us.homepage, HTMLTable.getFileHead("QSanguosha-Hegemony"))
	table.insert(about_us.homepage, HTMLTable.encloseInTagsOfDivId(HTMLTable.encloseInTag("QSanguosha-Hegemony", "h1"), "header"))
	local sectionbody = {}
	table.insert(sectionbody, HTMLTable.createListingBlock("<br>简介",
		"QSanguosha-Hegemony是一个基于太阳神三国杀V2的开源项目，为了实现游卡官方国战而努力。"))
	table.insert(sectionbody, HTMLTable.createListingBlock("开发人员", "程序：" .. table.concat(programmers, " "),
		"AI：" .. table.concat(ai_designers, " "), "美工：" .. table.concat(art_designers, " "), "助理: " .. table.concat(assistants, " "),
		"配音工作人员:" .. table.concat(audio_workers, " ")))
	table.insert(sectionbody, HTMLTable.createListingBlock("特别鸣谢", getQunInfo("太阳神三国杀国战联机群",
		"低调的付尼玛", "01112a97ee4545654e1a098850184a84a9eadd3d6c7dc570fdd883e461babfd4", "nini"),
		getQunInfo("豚豚神杀游戏群", "洛神", "ffb10d7ef73fbdef7cf7da3f6a64b95b889c465fdc4e1662979434583357638b", "tuntun"),
		getTextWithOptionalLink("结算顾问 凌天翼 问心云", "http://dadao.net/sgs/guo.html"),
		getTextWithOptionalLink("AI战术顾问 叶落孤舟"),
		getTextWithOptionalLink("太阳神三国杀论坛", "http://qsanguosha.org")))
	sectionbody = HTMLTable.encloseInTagsOfDivClass(table.concat(sectionbody), "sectionbody")
	table.insert(about_us.homepage, HTMLTable.encloseInTagsOfDivId(sectionbody, "preamble", "content"))
	table.insert(about_us.homepage, HTMLTable.getFileFoot())
	about_us.homepage = table.concat(about_us.homepage)
end

local pages = {
	{
		[0] = "Yanguam",
		"程序员基因到我已经传了三代，爷爷爸爸和叔叔们都是程序员。好吧，我在技术上对不起这优良的血统。",
		"暑期在一家投行旗下的火腿肠生产企业附属的房地产公司工作，大部分代码在上班时间完成。",
		"为了发扬自由软件的文化而加入神杀。坚信程序可能因时间而没落，而传承的技术与思想永不消亡。"
	},

	{
		[0] = "BeginnerSlob",
		"三流程序员，大学狗，一心想要做出自己的东西而加入神杀。",
		"简单的说就是不拘小节（这根本就是散漫吧啊喂！"
	},

	{
		[0] = "Fsu0413",
		"喜欢你妹大神的Fsu0413，没事闲的时候愿意写点代码，偶然间发现神杀可以写LUA扩展包而加入神杀LUA组，从0开始学习神杀接口编程。",
		"目前已经找到工作，驾照也考下来了，可是……不知到时候工作会不会忙……我会尽量抽空为神杀国战更新代码的。"
	},

	{
		[0] = "takashiro",
		"弃神杀许久后突然又被国战勾起了开杀的欲望的高城君被神杀国战的代码和界面所感染，由Fs君带入Rara团队。",
		"是一只内心时常骚动但又总是要憋很久才有明着骚的勇气的即将进入研一的新生，希望未来能够成为一名优秀的码农。",
		"开发群每天都可以很开心地潜水..."
	},

	{
		[0] = "hmqgg",
		"药不能停的高三党。。一直仰慕各位大神",
		"ps P站号37492141"
	},

	{
		[0] = "36li",
		"别拦我，我已经放弃治疗了",
		"渣美工一枚，没事别找我",
		"容我打个广告",
		HTMLTable.getHref("百度36李吧欢迎你", "http://tieba.baidu.com/f?ie=utf8&kw=36李&fr=itb_favo&fp=favo")
	},

	{
		[0] = "lzxqqqq",
		"一只忧郁的月兔，来自月球的新奇物种，目前在用它拙劣的技术尝试和AI互动。",
		"性格非常的温顺，只要你喂它口感适宜新鲜美味的萌妹纸就会非常的听话~",
		"最近沉迷于Dota2和DiabloIII不可自拔……"
	},

	{
		[0] = "zyun7799",
		"程序员编制里的非程序员，为了愉快地与电脑玩耍而愉快地学写代码。",
		"“或许我永远不会把这当作正业，但是我可以很认真地不务正业。”——路过的正切君"
	}
}

sgs.LoadTranslationTable {

	["homepage"] = "太阳神三国杀·国战",

	["Yanguam"] = "啦啦SLG",
	["BeginnerSlob"] = "女王受·虫",
	["36li"] = "36李",
	["zyun7799"] = "任意角的正切"
}


about_us.developers = {"homepage"}

function createDeveloperPages()
	for _, t in ipairs(pages) do
		local page = {}
		local owner = t[0]
		table.insert(about_us.developers, owner)
		table.insert(page, HTMLTable.getFileHead(owner))
		table.insert(page, HTMLTable.encloseInTagsOfDivId(HTMLTable.encloseInTag(sgs.Sanguosha:translate(owner), "h1"), "header"))
		local listingblock = HTMLTable.createListingBlock("<br><br><br>" .. HTMLTable.getImg(("./image/system/developers/%s.jpg"):format(owner), nil, owner) .. "<br>简介", table.unpack(t))
		local sectionbody = HTMLTable.encloseInTagsOfDivClass(listingblock, "sectionbody")
		table.insert(page, HTMLTable.encloseInTagsOfDivId(sectionbody, "preamble", "content"))
		table.insert(page, HTMLTable.getFileFoot())
		about_us[owner] = table.concat(page)
	end
end

function createAboutUsPages()
	createHomePage()
	createDeveloperPages()
end
