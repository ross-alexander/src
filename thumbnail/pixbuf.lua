-- ----------------------------------------------------------------------
--
-- 2023-10-20: Add comments
--
-- ----------------------------------------------------------------------

function example()
   local pixbuf = image_t.new_pixbuf_from_file("images/286.jpg")
   print(pixbuf)
   local scaled = pixbuf:scale_to(400)
   scaled:save("pixbuf.jpeg")
   
   local blank = image_t.new_pixbuf(2000, 2000)
   print(blank)
   blank:fill("orange")
   blank:compose(scaled, 100, 100)
   blank:save("blank.jpeg")
end

-- ----------------------------------------------------------------------
--
-- compose
--
-- ----------------------------------------------------------------------

function compose(tn, cols, size, border)
   -- Load the images into memory

   tn:load_images_pixbuf()

   -- Calculate number of rows

   local images = tn.image_table
   local rows = ((#images-1)//cols)+1
   local fontsize = 12

   -- Create result pixbuf
   
   local width = (size+border)*cols + border
   local height = (size+border+fontsize)*rows + border
   local dst = image_t.new_pixbuf(width, height)
   dst:fill("#FFFFFF")
   
   local count = 0
   for path, image in pairs(images) do
      if (image.valid) then
	 local x = border + (border+size)*(count%cols)
	 local y = border + (border+size+fontsize)*(count//cols)
	 
	 -- Scale image
      
	 local scale = image:scale_to(size)
	 
	 -- Create black background and compose onto sheet
	 
	 local bg = image_t.new_pixbuf(size, size)
	 bg:fill("#000000")
	 dst:compose(bg, x, y)
	 
	 -- Compose scaled image in middle of background
	 
	 local cx = x + (size - scale.width)/2
	 local cy = y + (size - scale.height)/2
	 dst:compose(scale, cx, cy)
	 
	 -- Add frame
	 
	 dst:frame(x, y, size, size, 2.0, "#FF0000")
	 
	 local name = image:name()
	 if (not (name == nil)) then
	    local bb = dst:add_text(x, y + size, size, fontsize, "Linux Biolinum O", name)
	 end      
	 count = count+1
      end
   end
   return dst
end   

-- ----------------------------------------------------------------------
--
-- thumbnail
--
-- ----------------------------------------------------------------------

function thumbnail(src, dst, size, num)
   local tn = thumbnail_t.new()

   tn:dir_add(src)

   -- Run recursive scan on directory
   
   tn:dir_scan()
   tn:load_images_pixbuf()

   for path, image in pairs(tn.image_table) do
      if (image.valid) then
	 print("++", path)
      else
	 print("--", path)
      end
   end

   tn:validate()
   
   local bounds = tn:get_bounds()
   for i,v in ipairs(bounds) do
      print(i, v)
   end
   
--   local images = tn.image_table
--   for i,v in pairs(images) do
--      print(i,v)
--   end

   -- Split into multiple thumbnail_t objects with a maximum of 32
   -- images each
   
   local split = tn:split(num)

   -- Loop over table (indexed 1 .. n)

   for i,v in ipairs(split) do
      -- Compose 8 images per row, 200 pixels image size, border 5 pixels wide
      result = compose(v, 6, size, 5)
      result:save(string.format("%s-%02d.jpg", dst, i))
   end
end


-- example()
thumbnail("/locker/images", "res", 250, 18)

