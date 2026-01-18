-- ----------------------------------------------------------------------
--
-- 2025-08-12: Fix scale issue and text paramter order problem
--
-- ----------------------------------------------------------------------

function compose(index, tn, cols, size, border)
   tn:load_images()
   local images = tn.image_table
   local rows = ((#images-1)//cols)+1
   local fontsize = 12
   local family = "Linux Biolinum O"
   
   local width = (size + border) * cols + border
   local height = (size + border + fontsize) * rows + border
   local dst = image_t.new(width, height)
   dst:fill("#FFFFFF")

   local bg = image_t.new(size, size)
   bg:fill("#000000")

   local count = 0
   for path, image in pairs(images) do
      print(string.format("%s [%f × %f]", path, image.width, image.height))
      local x = border + (border + size) * (count % cols)
      local y = border + (border + size + fontsize) * (count // cols)
      local scale = image:scale_to(size)
      
      dst:compose(bg, x, y)

--      dst:compose(scale, x, y)
      local cx = x + (size - scale.width)/2
      local cy = y + (size - scale.height)/2
      dst:compose(scale, cx, cy)
      dst:frame(x, y, size, size, 2.0, "#FF0000")
      local name = image:name()
      if (not (name == nil)) then
	 local bb = dst:add_text(x, y + size, size, fontsize, family, name)
      end
      count = count + 1
   end
   dst:save(string.format("202507-%02d.jpg", index))
end   

-- ----------------------------------------------------------------------
--
-- main
--
-- ----------------------------------------------------------------------

local tn = thumbnail_t.new()
tn:dir_add("/home/ralexand/src/lizards/cxx/svg")
tn:dir_scan()

-- for path, image in pairs(tn.image_table) do
--    print(path)
-- end

local rows = 4
local cols = 9
local size = 200

-- split into integer indexed table of thumbnails

local split = tn:split(rows * cols)
for i, v in ipairs(split) do
   compose(i, v, cols, size, 5)
end
