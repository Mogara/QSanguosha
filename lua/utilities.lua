-- utilities, i.e: convert QList<const Card> to Lua's native table
function sgs.QList2Table(qlist)
	local t = {}
	for i = 0, qlist:length() - 1 do
		table.insert(t, qlist:at(i))
	end

	return t
end

-- the iterator of QList object
local qlist_iterator = function(list, n)
	if n < list:length() - 1 then
		return n + 1, list:at(n + 1) -- the next element of list
	end
end

function sgs.qlist(list)
	return qlist_iterator, list, -1
end

-- more general iterator
function sgs.list(list)
	if type(list) == "table" then
		return ipairs(list)
	else
		return sgs.qlist(list)
	end
end

function sgs.reverse(list)
	local new = {}
	for i = #list, 1, -1 do
		table.insert(new, list[i])
	end
	return new
end

-- copied from "Well House Consultants"
-- used to split string into a table, similar with php' explode function
function string:split(delimiter)
	local result = {}
	local from = 1
	local delim_from, delim_to = string.find(self, delimiter, from)
	while delim_from do
		table.insert(result, string.sub(self, from, delim_from - 1))
		from  = delim_to + 1
		delim_from, delim_to = string.find(self, delimiter, from)
	end
	table.insert(result, string.sub(self, from))
	return result
end

function table:contains(element)
	if #self == 0 or type(self[1]) ~= type(element) then return false end
	for _, e in ipairs(self) do
		if e == element then return true end
	end
end

function table:removeOne(element)
	if #self == 0 or type(self[1]) ~= type(element) then return false end

	for i = 1, #self do
		if self[i] == element then 
			table.remove(self, i)
			return true
		end
	end
	return false
end

function table:removeAll(element)
	if #self == 0 or type(self[1]) ~= type(element) then return 0 end
	local n = 0
	for i = 1, #self do
		if self[i] == element then 
			table.remove(self, i)
			n = n + 1
		end
	end
	return n
end

function table:insertTable(list)
	for _, e in ipairs(list) do
		table.insert(self, e)
	end
end

function table:removeTable(list)
	for _, e in ipairs(list) do
		table.removeAll(self, e)
	end
end

function table.copyFrom(list)
	local l = {}
	for _, e in ipairs(list) do
		table.insert(l, e)
	end
	return l
end

function table:indexOf(value, from)
	from = from or 1
	for i = from, #self do
		if self[i] == value then return i end
	end
	return -1
end

function string:matchOne(option)
	return self:match("^" .. option .. "%p") or self:match("%p" .. option .. "%p") or self:match("%p" .. option .. "$")
end

function string:startsWith(substr)
	local len = string.len(substr)
	if len == 0 or len > string.len(self) then return false end
	return string.sub(self, 1, len) == substr
end

function string:endsWith(substr)
	local len = string.len(substr)
	if len == 0 or len > string.len(self) then return false end
	return string.sub(self, -len, -1) == substr
end

function math:mod(num)
	return math.fmod(self, num)
end

sgs.CommandType = {
	"S_COMMAND_UNKNOWN",
	"S_COMMAND_CHOOSE_CARD",
	"S_COMMAND_PLAY_CARD",
	"S_COMMAND_RESPONSE_CARD",
	"S_COMMAND_SHOW_CARD",
	"S_COMMAND_SHOW_ALL_CARDS",
	"S_COMMAND_EXCHANGE_CARD",
	"S_COMMAND_DISCARD_CARD",
	"S_COMMAND_INVOKE_SKILL",
	"S_COMMAND_MOVE_FOCUS",
	"S_COMMAND_CHOOSE_GENERAL",
	"S_COMMAND_CHOOSE_KINGDOM",
	"S_COMMAND_CHOOSE_SUIT",
	"S_COMMAND_CHOOSE_ROLE",
	"S_COMMAND_CHOOSE_ROLE_3V3",
	"S_COMMAND_CHOOSE_DIRECTION",
	"S_COMMAND_CHOOSE_PLAYER",
	"S_COMMAND_CHOOSE_ORDER",
	"S_COMMAND_ASK_PEACH",
	"S_COMMAND_SET_MARK",
	"S_COMMAND_SET_FLAG",
	"S_COMMAND_CARD_FLAG",
	"S_COMMAND_NULLIFICATION",
	"S_COMMAND_MULTIPLE_CHOICE",
	"S_COMMAND_PINDIAN",
	"S_COMMAND_AMAZING_GRACE",
	"S_COMMAND_SKILL_YIJI",
	"S_COMMAND_SKILL_GUANXING",
	"S_COMMAND_SKILL_GONGXIN",
	"S_COMMAND_SET_PROPERTY",
	"S_COMMAND_CHANGE_HP",
	"S_COMMAND_CHANGE_MAXHP",
	"S_COMMAND_CHEAT",
	"S_COMMAND_SURRENDER",
	"S_COMMAND_ENABLE_SURRENDER",
	"S_COMMAND_GAME_OVER",
	"S_COMMAND_GAME_START",
	"S_COMMAND_MOVE_CARD",
	"S_COMMAND_GET_CARD",
	"S_COMMAND_LOSE_CARD",
	"S_COMMAND_LOG_EVENT",
	"S_COMMAND_LOG_SKILL",
	"S_COMMAND_UPDATE_CARD",
	"S_COMMAND_CARD_LIMITATION",
	"S_COMMAND_ADD_HISTORY",
	"S_COMMAND_SET_EMOTION",
	"S_COMMAND_FILL_AMAZING_GRACE",
	"S_COMMAND_CLEAR_AMAZING_GRACE",
	"S_COMMAND_TAKE_AMAZING_GRACE",
	"S_COMMAND_FIXED_DISTANCE",
	"S_COMMAND_KILL_PLAYER",
	"S_COMMAND_REVIVE_PLAYER",
	"S_COMMAND_ATTACH_SKILL",
	"S_COMMAND_NULLIFICATION_ASKED",
	"S_COMMAND_EXCHANGE_KNOWN_CARDS", -- For Dimeng only
	"S_COMMAND_SET_KNOWN_CARDS",
	"S_COMMAND_UPDATE_PILE",
	"S_COMMAND_RESET_PILE",
	"S_COMMAND_UPDATE_STATE_ITEM",
	"S_COMMAND_SPEAK",
	"S_COMMAND_ASK_GENERAL", -- the following 6 for 1v1 and 3v3
	"S_COMMAND_ARRANGE_GENERAL",
	"S_COMMAND_FILL_GENERAL",
	"S_COMMAND_TAKE_GENERAL",
	"S_COMMAND_RECOVER_GENERAL",
	"S_COMMAND_REVEAL_GENERAL",
	"S_COMMAND_AVAILABLE_CARDS",
	"S_COMMAND_ANIMATE",
	"S_COMMAND_LUCK_CARD",
	"S_COMMAND_VIEW_GENERALS"
}

local i = 0
for _, command in ipairs(sgs.CommandType) do
	sgs.CommandType[command] = i
	i = i + 1
end
