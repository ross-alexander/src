require "date"

function cascade(a)
   if (a:wday() == 6)
   then
      return a + 2;
   end
   if (a:wday() == 0)
   then
      return a + 1;
   end
   return a
end

function christmas(year)
   local a = Date.new("december 25 " .. year)
   for i = 0,4 do
      local wday = (a + i):wday()
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

function newyear(year)
  local a = Date.new("january 1 " .. year)
  local c = 1
  local i = 0
  while i < c do
    omni(a + i)
    local wday = (a + i):wday()
    if (wday == 6)
    then
      c = c + 1
    elseif (wday == 0)
    then
      c = c + 1
    end
    i = i + 1
  end
end


function easter(year)
   local a = Date.new("'easter'")
   omni(a - 2)
   omni(a + 1)
end

function omni(a)
   print(a:omni())
end

if (#arg < 1) then
   print "Must pass year on command line"
else
   local year = arg[1]

   christmas(year - 1)
   newyear(year)
   easter(year)
   omni(Date.new("first monday of may"))
   omni(Date.new("first monday of jun"))
   omni(Date.new("first tuesday of jun"))
   omni(Date.new("last monday of aug"))
   christmas(year)
   newyear(year + 1)
end
