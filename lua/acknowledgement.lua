-- this script contains information for acknowledgement

module("acknowledgement", package.seeall)

effects_processors = {
	'秋色',
	'风叹息',
	'安岐大小姐'
}

art_designers = {
	'褪色',
	'冢冢的青藤',
	'背碗卤粉',
	'塞克洛',
	'爱拼才会赢',
	'shenglove82',
	'洛神',
	'Jr. Wakaran',
	'武士',
	'Guiglc',
	'艾艾艾',
	'离岛卷卷',
	'三好',
	'幻失落'
}

programmers = {
	'William915',
	'donle',
	'宇文天启',
	'HyperX',
	'clarkcyt',
	'Slob',
}

forum_supporters = {
	'白七',
	'DaoXian',
	'米叔',
}

local qun_owners = {
	{
		[0] = "主群",
		"50745533 游子",
		"10288204 小内",
		"71411495 KenKic",
		"116761946 哦了",
		"117088442 灵魂",
		"73354552 tyssgsd",
		"114041721 Grox",
		"39511833 Bazinga",
		"53422043 iblack",
		"42581190 Mr.Robin",
	},
	{
		[0] = '电信',
		"47744298 小虾米",
		"125632579 碧雪",
		"123943908 麒麟儿",
		"40858822 凌",
		"110528401 断念",
		"131966838 琉星",
		"199050337 涅槃",
		"103827743 梢欢",
		"89396738 小四郎",
		"170011920 忘却",
	},
	{
		[0] = "网通",
		"42149831 妹纸",
		"101489477 tianthw",
		"65016991 张教主",
		"58992519 7741",
		"102605100 小幺",
		"32675836 Jesee莫尘",
		"52244780 涅槃",
		"171692483 蓝",
		"165159520 月神",
		"211369199 YYH",
	},
	{
		[0] = "其他",
		{'DIY创意群', "74201710 KenKic"},
		{"3v3竞技群", "213577503 Paul"},
		{"Lua扩展群", "158859448 A小可"},
		{"新版测试群", "191935716 皮卡丘"},
	}
}

local cvset = {}

function addToCVSet(t)
	for k, v in pairs(t) do
		if k:match("^cv:") then
			cvset[v] = true
		end
	end	
end

require 'middleclass'

HTMLTable = class 'HTMLTable'

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

function HTMLTable.encloseInTags(t, inner, outer)
	local result = {}
	for _, e in ipairs(t) do
		table.insert(result, HTMLTable.encloseInTag(e, inner))
	end

	result = table.concat(result)
	return HTMLTable.encloseInTag(result, outer)
end

function HTMLTable:__tostring()
	local t = {}
	for _, row in ipairs(self.rows) do
		local rowstring = HTMLTable.encloseInTags(row, "td", "tr")
		table.insert(t, rowstring)
	end

	return HTMLTable.encloseInTags(t, nil, "table")
end

width = 900
height = 600
content = ""

function createContent()
	cvnames = {}
	for name in pairs(cvset) do
		table.insert(cvnames, name)
	end

	local qun_names = '一 二 三 四 五 六 七 八 九 十';
	qun_names = qun_names:split(' ')
	local getQunNameByNumber = function(i)
		return qun_names[i] .. '群'
	end

	qun_table = HTMLTable()
	for _, t in ipairs(qun_owners) do
		local category = t[0]
		local quns = HTMLTable()

		local step = 1
		if #t > 8 then
			step = 2
		end

		for i=1, #t, step do
			local row = {}
			for j=1, step do
				local index = i+j-1
				local e = t[index]
				if type(e) == 'string' then
					table.insert(row, getQunNameByNumber(index))
					table.insert(row, e)
				elseif type(e) == 'table' then
					for _, v in ipairs(e) do
						table.insert(row, v)
					end
				end
			end
			quns:addRow(unpack(row))
		end

		qun_table:addRow(category, tostring(quns))
	end

	content = [[
	<table>
	<tr>
	<td width='400'>
	#感谢为程序献过声的各位 CV#
	$cvnames
	#感谢配音后期处理人员#
	$effects_processors
	#感谢为程序提供过图片的各位美工#
	$art_designers
	#感谢程序维护人员#
	$programmers
	#感谢论坛赞助者#
	$forum_supporters
	<td>
	<td width='400'>
	#感谢提供超级群以便交流的各位群主#
	$qun_table
	</td>
	</tr>
	<tr><td colspan='2'><big>正是因为有了大家的支持，所以太阳神三国杀才越来越好!</big></td></tr>
	</table>
	]]

	content = content:gsub('#([^#]+)#', "<font color='blue'>%1</font> <br/>")
	content = content:gsub('%$([a-z_]+)', function(name)
		local value = _M[name]
		if instanceOf(HTMLTable, value) then
			return tostring(value)
		else
			return table.concat(value, ' ') .. '<br/><br/>'
		end
	end)
end

