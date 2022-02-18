local t = { 13.3, 15.4}
local t2 = { 99 }
local a = 10.5
local b = 12.8
local c = a + b + t[1] + t[2]
local s = string.format("%04d", c)
print(s)
