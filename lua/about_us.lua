--[[********************************************************************
	Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

  This file is part of QSanguosha-Hegemony.

  This game is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3.0 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  See the LICENSE file for more details.

  QSanguosha-Hegemony Team
*********************************************************************]]

-- this script is used to generate pages about developers automatically

about_us = {}

programmers = {
	'啦啦SLG',
	'Fsu0413',
	'女王受·虫',
	'hmqgg',
	'takashiro'
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
	'洛神'
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
	content = HTMLTable.encloseEachElementInTags({...}, "tt", "pre")
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
	
	local part2_2 = '<link rel="stylesheet" type="text/css" href="developers/style.css" />'
	..'<script src="developers/script.js"></script>'
	
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
	table.insert(about_us.homepage, HTMLTable.getFileHead("QSanguosha-Hegemony-V2"))
	table.insert(about_us.homepage, HTMLTable.encloseInTagsOfDivId(HTMLTable.encloseInTag("QSanguosha-Hegemony-V2", "h1"), "header"))
	local sectionbody = {}
	table.insert(sectionbody, HTMLTable.createListingBlock("<br>简介", 
		"QSanguosha-Hegemony-V2是一个基于太阳神三国杀V2的开源项目，为了实现游卡官方国战而努力。"))
	table.insert(sectionbody, HTMLTable.createListingBlock("开发人员", "程序：" .. table.concat(programmers, " "),
		"AI：" .. table.concat(ai_designers, " "), "美工：" .. table.concat(art_designers, " ")))
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