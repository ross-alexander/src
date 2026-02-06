local lgi = require 'lgi'
local Gtk = lgi.require('Gtk', '3.0')
   
function Background(file)
   local GdkPixbuf = lgi.GdkPixbuf
   local cairo = lgi.cairo
   local assert = lgi.assert

   local background = assert(GdkPixbuf.Pixbuf.new_from_file(file))

   local screen_width, screen_height, bg_width, bg_height = 1000, 800, background.width, background.height

   local width_ratio, height_ratio = screen_width / bg_width, screen_height / bg_height

   local ratio
   if (width_ratio > height_ratio) then
      ratio = height_ratio
   else
      ratio = width_ratio
   end
   local width, height = math.floor(bg_width * ratio), math.floor(bg_height * ratio)
   print(screen_width, screen_height, bg_width, bg_height, ratio, width, height)
   local dest = background:scale_simple(width, height, 'HYPER')

end

Background('/locker/images/awesome/people/586372290.jpg')
