-- ----------------------------------------------------------------------
--
-- 2023-12-26: Various examples parsing dates
--
-- ----------------------------------------------------------------------

print("The year is " .. year)

function list_iter (t)
   local i = 0
   local n = #t
   return function ()
      i = i + 1
      if i <= n then return t[i] end
   end
end

for b in list_iter({"jan 1 2012", "may 12 2011"}) do
   print(Date.new(b):omni())
end

dates = {"third saturday of november",
	 "fourth saturday of november",
	 "fifth saturday of november",
	 "first saturday of december",
	 "second saturday of december",
}

for _, date in ipairs(dates) do
   print(Date.new(date):strftime("%A %d %B %Y"))
end


for m = 1,12 do
    local tuesday = string.format("second tuesday of month %d", m)
    print(Date.new(tuesday):strftime("%A %d %B %Y"))
  
end
