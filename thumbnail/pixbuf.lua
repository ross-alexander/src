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

function compose(tn, settings)

   local cols = settings.columns
   local size = settings.size
   local padding = settings.padding
   local fontsize = settings.fontsize

   -- Load the images into memory

   tn:load_images_pixbuf()

   -- Calculate number of rows

   local images = tn.image_table
   local rows = ((#images-1)//cols)+1

   -- Create result pixbuf
   
   local width = (size+padding)*cols + padding
   local height = (size+padding+fontsize)*rows + padding
   local dst = image_t.new_pixbuf(width, height)
   dst:fill("#FFFFFF")
   
   local count = 0
   for path, image in pairs(images) do
      if (image.valid) then
	 local x = padding + (padding+size)*(count%cols)
	 local y = padding + (padding+size+fontsize)*(count//cols)
	 
	 -- Create black background and compose onto sheet
	 
	 local bg = image_t.new_pixbuf(size, size)
	 bg:fill("#000000")
	 dst:compose(bg, x, y)
	 
	 -- Scale image
      
	 local scale = image:scale_to(size)
	 
	 -- Compose scaled image in middle of background
	 
	 local cx = x + (size - scale.width)/2
	 local cy = y + (size - scale.height)/2
	 dst:compose(scale, cx, cy)
	 
	 -- Add frame
	 
	 dst:frame(x, y, size, size, 2.0, "#FF0000")
	 
	 local name = image:name()
	 if (not (name == nil)) then
	    local bb = dst:add_text(x, y + size, size, settings.fontsize, settings.fontname, name)
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

function thumbnail(src, dst, settings)
   local tn = thumbnail_t.new()

   tn:dir_add(src)

   -- Run recursive scan on directory
   
   tn:dir_scan()

   -- Validate to remove any unloadable images.  This uses
   -- gdk_pixbuf_get_file_info and does not load the image, just
   -- return its dimension and if scalable.
   
   tn:validate()
   
   -- Split into multiple thumbnail_t objects with num images each
   
   local split = tn:split(settings.rows * settings.columns)

   -- Loop over table (indexed 1 .. n)

   for i,v in ipairs(split) do
      result = compose(v, settings)
      result:save(string.format("%s-%02d.jpg", dst, i))
   end
end


-- ----------------------------------------------------------------------
--
-- M A I N
--
-- ----------------------------------------------------------------------

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

local settings = {
   fontname = "Linux Biolinum",
   fontsize = 10,
   columns = 6,
   rows = 3,
   padding = 5.0,
   border = {
      width = 1.0,
      color = "#00ff00"
   },
   size = options.size == nil and 250 or options.size
}

thumbnail(options.src, options.dst, settings)

