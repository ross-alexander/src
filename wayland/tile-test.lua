local tile = require('tile')

local lost = tile.tile_surface_t.new_from_file('lost.jpg')
print(lost)

local png = tile.tile_surface_t.new_from_file('cairo.png')
print(png)

local found = tile.tile_surface_t.new(1200, 800)
print(found)

local context = found:context_create()
print(context)

context:rectangle(20, 30, 400, 400)
context:set_source_surface(lost, 0, 0)
-- context:set_source_rgb(0, 1, 0)
context:fill_preserve()
context:set_line_width(5.0)
context:set_source_rgb(0, 0, 1)
context:stroke()

local p = tile.tile_pattern_t.create_linear(0, 300, 600, 300)

-- offset, r, g, b, a

p:add_color_stop_rgba(0, 0, 0, 1, 1.0)
p:add_color_stop_rgba(1, 0, 1, 0, 1.0)

context:rectangle(100, 400, 600, 600)
context:set_source(p)
context:fill_preserve()
context:set_source_rgb(1, 1, 1)
context:stroke()

png:write_to_png("tile-test-png.png")
lost:write_to_png("tile-test-lost.png")
if not found:write_to_png("tile-test-found.png") then
   print("found:write_to_png failed")
end

