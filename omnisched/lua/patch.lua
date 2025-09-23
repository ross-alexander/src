-- ----------------------------------------------------------------------
--
-- 2025-02-12: Convert to array
--
-- ----------------------------------------------------------------------

require "date"

function omni(a)
   print(a:strftime("%j%t%b %d\t\t# %a %d %b %Y"))
--   print(a:omni())
end

keys = {'Date', 'Patch Tuesday', 'Phase 1', 'Phase 2', 'Phase 3', 'Phase 4'}
sep = {}
for k, v in pairs(keys)
do
   keys[k] = string.format("%-20s", v)
   sep[k] = string.format("%-20s", "---")
end

print(table.concat(keys))
print(table.concat(sep))

for year = 2025, 2026
do
   for month = 1, 12
   do
      tuesday = Date.new(string.format("second tuesday of %d, %d", month, year))
--      print(tuesday:strftime("Patching for %B %Y\n"))
      local phases = {
	 tuesday + 0,
	 tuesday + 4,
	 tuesday + 8,
	 tuesday + 9,
	 tuesday + 13
      }
      for k, v in pairs(phases)
      do
	 phases[k] = string.format("%-20s", v:strftime("%a %b %d %Y"))
      end
      print(string.format("%-20s%s", tuesday:strftime("%B %Y"), table.concat(phases)))
   end
end
