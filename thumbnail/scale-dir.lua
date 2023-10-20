local tn = thumbnail_t.new()
tn:dir_add("/locker/images/personal/2021-09-London")
tn:dir_scan()

local images = tn.image_table

local tmp = path_t.new("/tmp/scaled")

for p,_ in pairs(images)
do
   local path = path_t.new(p)
--   print(tmp + path:filename())
   local i = image_t.new_from_file(tostring(path))
   if (i.valid) then
      local j = i:scale(0.25)
      local dst = tostring(tmp + path:filename())
      j:save(dst)
   end
--   local bb = image_t.get_file_info(path)
--   if (not (bb == nil)) then
--      print(bb.width, bb.height)
--   end
end
