-- ----------------------------------------------------------------------
--
-- Old example where the monthly backups were the weekend of the
-- second Saturday
--
-- ----------------------------------------------------------------------

require("date")

function monthly(d)
   return(d:strftime("\t-day %d -month %b"))
end

local d = Date.new("jan 1")

print("*** Saturday schedule for " .. d:strftime("%Y") .. " ***")

for i = 1,12
do
   print(monthly(Date.new("second saturday of month "..i.." at 20:00")))
end
print "*** Sunday schedule ***"
for i = 1,12
do
   print(monthly(Date.new("second saturday of month "..i.." at 20:00") + 1))
end
