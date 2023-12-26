-- ----------------------------------------------------------------------
--
-- 2023-12-26: Simple example
--
-- ----------------------------------------------------------------------

-- Check if the year value is correct

print("The year is " .. year)

dates = {"third saturday of november",
	 "fourth saturday of november",
	 "fifth saturday of november",
	 "first saturday of december",
	 "second saturday of december",
}

for _, d in ipairs(dates) do
   print(Date.new(d):strftime("%A %d %B %Y"))
end
