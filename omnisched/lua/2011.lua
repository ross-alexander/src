-- ----------------------------------------------------------------------
--
-- 2026-03-24: Update example with getters / strftime only
--
-- ----------------------------------------------------------------------

require "date"

function cascade(a)
   if (a.wday == 6)
   then
      return a + 2;
   end
   if (a.wday == 0)
   then
      return a + 1;
   end
   return a
end

function christmas()
   local a = Date.new("december 25")
   for i = 0,4 do
      local wday = (a + i).wday
      if (wday == 6)
      then
	 a = a + 2
      elseif (wday == 0)
      then
	 a = a + 1
      end
      omni(a + i)
   end
end

function easter()
   local a = Date.new("'easter'")
   omni(a - 2)
   omni(a + 1)
end

function omni(a)
   print(a:strftime("%j%t%b %d\t\t# %a %d %b %Y"))
end

-- year = 2011
omni(Date.new("january 1"))
easter()
omni(Date.new("april 29"))
omni(Date.new("first monday of may"))
omni(Date.new("last monday of may"))
omni(Date.new("last monday of aug"))
christmas()
year = year + 1
print(omni(Date.new("january 1")))
