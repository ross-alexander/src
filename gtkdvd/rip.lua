#!/usr/bin/env lua-5.4

-- ----------------------------------------------------------------------
--
-- rip using libdvdread
--
-- 2023-02-01:
--   Improve text output
--
-- 2016-09-10: Ross Alexander
--
-- ----------------------------------------------------------------------

require("dvd")
require("json")


-- Set device - should be commandline option

local device = "/dev/sr0"
local dest = "."

-- Get disk info

local info = dvd.dvd_info_t(device)
local disc_name = info.discinfo.disc_title

if (not(arg[1] == nil))
then
   disc_name = arg[1]
end

-- Process commandline

local count = 0
if (not(arg[2] == nil))
then
   count = arg[2]
end

local sep = "."
if (not(arg[3] == nil))
then
   sep = arg[3]
end

if (not(arg[4] == nil))
then
   dest = arg[4]
end

local info_table = dvd.dvd_info_table(info)

-- print(json.encode(info_table))

print(disc_name)

-- Loop over each title

for i = 1, info.title_count, 1 do
   local title = info:get_title(i)
   local len = title.general.length
   print(title:to_string())
   print(string.format("%02d : length %f", i, len))
   if (len > 600) and (len < 9000)
   then
      local idx = i
      if (count) then idx = count; end
      local media_file = string.format("%s/%s%s%02d.mpg", dest, disc_name, sep, idx)
      local info_file = string.format("%s/%s%s%02d.ifo", dest, disc_name, sep, idx)
      print(string.format("%02d : %s %s", idx, media_file, info_file))

      title:copy(media_file, info_file)
      count = count + 1; 
   end
end

-- dvd.eject(device)
