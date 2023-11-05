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

   -- Validate to remove any unloadable images.  This uses
   -- gdk_pixbuf_get_file_info and does not load the image, just
   -- return its dimension and if scalable.
   
   tn:validate()
   
   -- Split into multiple thumbnail_t objects with num images each
   
   local split = tn:split(num)

   -- Loop over table (indexed 1 .. n)

   for i,v in ipairs(split) do
      result = compose(v, 6, size, 5)
      result:save(string.format("%s-%02d.jpg", dst, i))
   end
end

for i,v in pairs(options) do
   print(i, v)
end

if (options.src == nil)
then
   print("--src missing.")
   return -1
end

if (options.dst == nil)
then
   print("--dst missing.")
   return -1
end


-- example()

-- Use ternary to set size

thumbnail(options.src, options.dst, options.size == nil and 250 or options.size, 18)

