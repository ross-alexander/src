#!/usr/bin/env lua

-- ----------------------------------------------------------------------
--
-- rip using libdvdread
--
-- 2016-09-10: Ross Alexander
--
-- ----------------------------------------------------------------------

require("dvd");

-- Set device - should be commandline option

local device = "/dev/sr0";
local dest = ".";

-- Get disk info

info = dvd.dvd_info_t(device);

local disc_name = info.discinfo.disc_title;
if (not(arg[1] == nil))
then
   disc_name = arg[1];
end
print(disc_name)

-- Process commandline

local count = 0
if (not(arg[2] == nil))
then
   count = arg[2]
end

local sep = "-"
if (not(arg[3] == nil))
then
   sep = arg[3]
end

if (not(arg[4] == nil))
then
   dest = arg[4]
end

-- Loop over each title

for i = 1, info.title_count, 1 do
   local title = info:get_title(i)
   local len = title.general.length
   print(len)
   if (len > 600) and (len < 9000)
   then
      local idx = i
      if (count) then idx = count; end
      title:copy(string.format("%s/%s%s%02d.mpg", dest, disc_name, sep, idx), string.format("%s/%s%s%02d.ifo", dest, disc_name, sep, idx))
      count = count + 1; 
   end
end

dvd.eject(device)
