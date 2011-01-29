local expand = require "expand"
local template = (io.open "lua/tools/result-template.html"):read("*a")


local args = { ... }
local data = {}
data.start_time = args[1]
data.players = {}
local record_file = ("records/%s.txt"):format(data.start_time)

require "sqlite3"

local db = sqlite3.open("users.db")
local stmt = db:prepare("SELECT * FROM results WHERE start_time = ?")
stmt:bind(data.start_time)
stmt:exec()

for row in stmt:rows() do
	local player = {}
	player.username = row.username
	player.general = sgs.Sanguosha:translate(row.general)
	player.role = sgs.Sanguosha:translate(row.role)
	player.alive = row.alive == "true"
	player.score = row.score
	
	local victims = ""
	if row.victims then
		victims = row.victims:split("+")
		for i=1, #victims do
			victims[i] = sgs.Sanguosha:translate(victims[i])
		end
		
		victims = table.concat(victims, ",")
	end
	
	player.victims = victims
	
	table.insert(data.players, player)
end

-- load the smtp support
local smtp = require("socket.smtp")
local ltn12 = require("ltn12")
local mime = require("mime")

local sender = sgs.GetConfig("Contest/Sender","")
local receivers = sgs.GetConfig("Contest/Receiver", ""):split(" ")
local from = ("<%s>"):format(sender)
local rcpt, tos = {}, {}
for _, receiver in ipairs(receivers) do
	table.insert(rcpt, ("<%s>"):format(receiver))
	local to = receiver:match("%w+")
	table.insert(tos, ("%s <%s>"):format(to, receiver))
end

local user = sender:match("%w+")

local server = sgs.GetConfig("Contest/SMTPServer", "")
local password = sgs.GetConfig("Contest/Password", "")

mesgt = {
  headers = {
    from = ("%s %s"):format(user, from),
    to = table.concat(tos, ","),
    subject = "QSanguosha",
  },
  body = {
	 [1] = {
		body = mime.eol(0, [[ Contest result]])
	 },
	 
     [2] = {
		headers = {
		   ["content-type"] = 'text/plain; name="result.html"',
		   ["content-disposition"] = 'attachment; filename="result.html"',
		},
		
		body = ltn12.source.chain(
		   ltn12.source.string(expand(template, data)),
		   mime.normalize("\r\n"))       
     },
	 
	 [3] = {
		headers = {
		   ["content-type"] = 'text/plain; name="record.txt"',
		   ["content-disposition"] = 'attachment; filename="record.txt"',
		},
		
		body = ltn12.source.chain(
		   ltn12.source.file(io.open(record_file, "r")),
		   mime.normalize("\r\n"))       
     },
	},	 
}


local r, e = smtp.send{
  from = from,
  rcpt = rcpt, 
  source = smtp.message(mesgt),
  
  user = user,
  password = password,
  server = server,
}

if r == nil then
	error(e)
end
