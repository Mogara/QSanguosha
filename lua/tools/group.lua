-- grouping

local input = "input.txt"
local output = "output.txt"

local qqs = {}

for line in io.lines(input) do
	local qq = tonumber(line)
	if qq then 
		table.insert(qqs, qq)
	end
end

local n = #qqs

math.randomseed(os.time())

-- shuffle
for i=1, n do
	local r = math.random(1, n)
	qqs[i], qqs[r] = qqs[r], qqs[i]
end

local output_file = io.open(output, "w")

for i=0, n/8 do
	output_file:write((i+1) .. ":\n")
	for j=0, 7 do
		local index = i*8+j+1
		local qq = qqs[index]
		if qq then
			output_file:write(qq .. "\n")
		end
	end

	output_file:write("\n\n---------------\n")	
end

output_file:close()

