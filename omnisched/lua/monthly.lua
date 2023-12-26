require("date")

local d = Date.new("jan 1")

print("*** Saturday schedule for " .. d:strftime("%Y") .. " ***")
local i
for i = 1,12
do
	print(Date.new("second saturday of month "..i.." at 20:00"):monthly())
end
print "*** Sunday schedule ***"
for i = 1,12
do
	print((Date.new("second saturday of month "..i.." at 20:00") + 1):monthly())
end
