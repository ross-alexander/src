function compose(index, tn, cols, size, border)
   tn:load_images()
   local images = tn.image_table
   local rows = ((#images-1)//cols)+1
   local fontsize = 16
   print("rows ", rows)

   local width = (size+border)*cols + border
   local height = (size+border+fontsize)*rows + border
   local dst = image_t.new(width, height)
   dst:fill("#FFFFFF")
   
   local count = 0
   for path, image in pairs(images) do
      print("**", path, image)
      local x = border + (border+size)*(count%cols)
      local y = border + (border+size+fontsize)*(count//cols)
      local scale = image:scale(size)
      local bg = image_t.new(size, size)
      bg:fill("#000000")
      dst:compose(bg, x, y)
      --   dst:compose(scale, x, y)
      local cx = x + (size - scale.width)/2
      local cy = y + (size - scale.height)/2
      dst:compose(scale, cx, cy)
      dst:frame(x, y, size, size, 2.0, "#FF0000")
      local name = image:name()
      if (not (name == nil)) then
	 local bb = dst:add_text(x, y + size, fontsize, size, "Linux Libertine O", name)
      end
      count = count+1
   end
   dst:save(string.format("foo-%02d.jpg", index))
end   

local tn = thumbnail_t.new()
tn:dir_add("/locker/images/hold")
tn:dir_scan()
local split = tn:split(32)

for i, v in ipairs(split) do
   compose(i, v, 8, 200, 5)
end
