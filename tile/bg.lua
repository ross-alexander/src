-- ----------------------------------------------------------------------
--
-- bg.lua
--
-- 2023-09-12
--
-- ----------------------------------------------------------------------

function background()

   -- Get all files under images (recursive)
   
   local images = scanner_t.new("/usr/share/backgrounds")
   images:rescan()

   -- Pick a random file, hopefully an image
   
   local px = tile_pixbuf_t.new_from_file(images:random())

   -- Get surface from xcb (root window)
   
   local s = tile_surface_t.new_from_xcb()

   -- Get ratio so it fits the screen
   
   local width_ratio, height_ratio = s.width / px.width, s.height / px.height
   local ratio = width_ratio > height_ratio and height_ratio or width_ratio
   local width, height = px.width * ratio, px.height * ratio

   -- Fill background to black
   
   s:fill(0, 0, 0)

   -- Create new scaled pixmap, which is then composed with the root window so that it it centered
   
   s:compose_pixbuf((s.width - width)/2, (s.height - height)/2, px:scale(width, height))
end
