-- ----------------------------------------------------------------------
--
-- 2023-11-26: Fix ordering of add_text
--
-- ----------------------------------------------------------------------



-- ----------------------------------------------------------------------
--
-- compose
--
-- ----------------------------------------------------------------------

function compose(index, tn, cols, size, border)
   local fontsize = 16
   local family = "Linux Libertine O"

   tn:load_images()

   local images = tn.image_table
   local rows = ((#images-1) // cols) + 1

   local width = (size + border) * cols + border
   local height = (size + border + fontsize) * rows + border
   local dst = image_t.new(width, height)
   dst:fill("#ffffc0")
   
   local count = 0
   for path, image in pairs(images) do
      print(image)
      local x = border + (border + size) * (count % cols)
      local y = border + (border + size + fontsize) * (count // cols)

      -- Scale image
      
      local scale = image:scale_to(size)

      -- Create black background and compose onto sheet

      local bg = image_t.new(size, size)
      bg:fill("#000000")
      dst:compose(bg, x, y)

      -- Compose scaled image in middle of background

      local cx = x + (size - scale.width)/2
      local cy = y + (size - scale.height)/2
      dst:compose(scale, cx, cy)

      dst:frame(x, y, size, size, 2.0, "blue")
      
      local name = image:name()
      if (not (name == nil)) then
	 local bb = dst:add_text(x, y + size, size, fontsize, family, name)
      end
      count = count + 1
   end
   return dst
end   

-- ----------------------------------------------------------------------
--
-- main
--
-- ----------------------------------------------------------------------

local tn = thumbnail_t.new()
local src = "test"
if (not (options.src == nil)) then
   src = options.src
end
tn:dir_add(src)
tn:dir_scan()

local split = tn:split(32)

local size = 100
if (not (options.size == nil)) then
   size = options.size
end

local dst = "thumbnails.jpg"
if (not (options.dst == nil)) then dst = options.dst end


for index, tn in ipairs(split) do
   local res = compose(1, tn, 8, size, 5)
   res:save(string.format(dst, index))
end
