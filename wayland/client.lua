-- client.lua

local lost = tile.tile_surface_t.new_from_file("lost.jpg")

function draw(surface)
   local c = surface:configuration()
   local w = c.width
   local h = c.height

   local p = tile.tile_pattern_t.create_linear(0, 0, w, h)
   p:add_color_stop_rgba(0, 1, 0, 0, 1.0)
   p:add_color_stop_rgba(1, 0, 1, 0, 1.0)

   local cr = surface:context_create()
   print(cr)

   cr:set_source(p)
   
--   cr:set_source_surface(lost, 0, 0)
   -- surface:set_source_rgb(1.0, 0.0, 0.0)
   cr:rectangle(0, 0, w, h)
   cr:fill_preserve()
   cr:set_line_width(2.0)
   cr:set_source_rgb(1, 1, 1)
   cr:stroke()
end
