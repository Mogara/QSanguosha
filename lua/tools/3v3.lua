-- for 3v3 select generals

local priority_table = {
	-- S
	zhangliao = 7,
	guojia = 7,
	liubei = 7,
	zhugeliang = 7,
	xunyu = 7,
	xuhuang = 7,
	lusu = 7,
	
	-- A+
	simayi = 6,
	huangyueying = 6,
	sunshangxiang = 6,
	huatuo = 6,
	xiaoqiao = 6,
	yuanshao = 6,
	caopi = 6,
	dongzhuo = 6,
	
	-- A-
	zhenji = 5,
	zhangfei = 5,
	huanggai = 5,
	daqiao = 5,
	diaochan = 5,
	zhangjiao = 5,
	dianwei = 5,
	taishici = 5,
	shuangxiong = 5,
	sunjian = 5,
	
	-- B+
	caocao = 4,
	xuchu = 4,
	machao = 4,
	ganning = 4,
	lubu = 4,
	xiahouyuan = 4,
	huangzhong = 4,
	pangde = 4,
	zhurong = 4,
	
	-- B-
	sunquan = 3,
	zhouyu = 3,
	zhoutai = 3,
	jiaxu = 3,
	wolong = 3,
	pangtong = 3,
	
	-- C
	xiahoudun = 2,
	guanyu = 2,
	zhaoyun = 2,
	lumeng = 2,
	luxun = 2,
	weiyan = 2,
	menghuo = 2,
	
	-- D
	caoren = 1,
}

local function compare_by_priority(a, b)
	local p1 = priority_table[a] or 0
	local p2 = priority_table[b] or 0
	
	return p1 > p2
end

function SelectGeneral3v3(generals)
	table.sort(generals, compare_by_priority)
	return generals[1]
end