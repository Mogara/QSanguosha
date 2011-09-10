#!/usr/bin/env lua

require "lfs"

sources = { "src/main.cpp" }
headers = {}
forms = {}

function extract(dir)
    for file in lfs.dir(dir) do
        file = dir .. "/" .. file
        if file:match("%.cpp$") then
            table.insert(sources, file)
        elseif file:match("%.h$") then
            table.insert(headers, file)
        elseif file:match("%.ui$") then
            table.insert(forms, file)
        end
    end
end

for dir in lfs.dir("src") do
    if not dir:match("^%.+$") and dir ~= "main.cpp" then
        extract("src/" .. dir)
    end
end

local format = "%s \\\n\t"

io.write("SOURCES += ")
for _, source in ipairs(sources) do
    io.write(format:format(source));
end

io.write('\n')

io.write("HEADERS += ");
for _, header in ipairs(headers) do
    io.write(format:format(header));
end

io.write('\n')

io.write("FORMS += ");
for _, form in ipairs(forms) do
    io.write(format:format(form))
end



