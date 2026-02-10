local lgo = require('LuaGObject')
local cairo = lgo.cairo

-- ----------------------------------------------------------------------
--
-- support functions
--
-- ----------------------------------------------------------------------

function square(x)
   return x*x
end

function deg2rad(theta)
   return theta * math.pi / 180
end

function rad2deg(theta)
   return theta * 180 / math.pi
end


function circle_line_intersection (x1, y1, x2, y2, x3, y3, r)

-- x1,y1  P1 coordinates (point of line)
-- x2,y2  P2 coordinates (point of line)
-- x3,y3, r  P3 coordinates and radius (sphere)
-- x,y   intersection coordinates

   local x, y, z, a, b, c, mu, i

  a =  square(x2 - x1) + square(y2 - y1);
  b =  2 * ((x2 - x1)*(x1 - x3) + (y2 - y1)*(y1 - y3));
  c =  square(x3) + square(y3) + square(x1) + square(y1) - 2* ( x3*x1 + y3*y1) - square(r) ;
  i =   b * b - 4 * a * c ;
  
  if (i < 0.0)
  then
	 -- no intersection
     return 0;
  end
  
  if (i == 0.0) then
     mu = -b/(2*a)
     local rx1 = x1 + mu*(x2-x1)
     local ry1 = y1 + mu*(y2-y1)
     return rx1, ry1
  end
  if (i > 0.0) then
      -- first intersection
      mu = (-b + math.sqrt( square(b) - 4*a*c )) / (2*a);
      local rx1 = x1 + mu*(x2-x1);
      local ry1 = y1 + mu*(y2-y1);
      -- second intersection
      mu = (-b - math.sqrt(square(b) - 4*a*c )) / (2*a);

      local rx2 = x1 + mu*(x2-x1);
      local ry2 = y1 + mu*(y2-y1);

      return rx1, ry1, rx2, ry2
  end
  return nil;
end


function circle_circle_intersection(x0, y0, r0, x1, y1, r1)
 
  local a, dx, dy, d, h, rx, ry;
  local x2, y2;

  dx = x1 - x0;
  dy = y1 - y0;

  d = math.sqrt((dy*dy) + (dx*dx));
  
  if (d > (r0 + r1)) then
    return 
  end

  if (d - math.abs(r0 - r1) < 0.0) then
    return
  end

  a = ((r0*r0) - (r1*r1) + (d*d)) / (2.0 * d)

  x2 = x0 + (dx * a/d);
  y2 = y0 + (dy * a/d);

  h = math.sqrt((r0*r0) - (a*a));

  rx = -dy * (h/d);
  ry = dx * (h/d);

  return x2 + rx, y2 + ry, x2 - rx, y2 - ry
end


-- ----------------------------------------------------------------------
--
-- Point
--
-- ----------------------------------------------------------------------

Point = {
   x, y = 0.0, 0.0, 
   __tostring = function(self)
      return string.format("Point(%6.2f, %6.2f)", self.x, self.y)
   end,
   rotate = function(self, theta)
      return Point:new{x = math.cos(theta) * self.x - math.sin(theta) * self.y, y = math.sin(theta) * self.x + math.cos(theta) * self.y}
   end,
   scale = function(self, s)
      return Point:new{x = self.x * s, y = self.y * s}
   end,
   translate = function(self, x, y)
      return Point:new{x = self.x + x, y = self.y + y}
   end,
   polar = function(self)
      local theta = math.atan(self.y / self.x)
      local phi

      if ((self.x < 0.0) and (self.y > 0.0)) then
	 phi = theta + math.pi;
      elseif ((self.x < 0.0) and ((self.y) < 0.0)) then
      phi = theta + math.pi;
      elseif ((self.x > 0.0) and (self.y < 0.0)) then
	 phi = theta + 2.0 * math.pi;
      else
	 phi = theta
      end
      return phi, math.sqrt(self.x * self.x + self.y * self.y)
   end,
   metric = function(self)
      return math.sqrt(self.x * self.x + self.y * self.y)
   end,
   fill = function(self, cr, radius)
      Circle:new {center = self, radius = radius}:fill(cr)
   end,
   text = function(self, cr, text)
      cr:save()
      cr:translate(self.x, self.y)
      cr:scale(1.0, -1.0)
      local extents = cr:text_extents(text)
      cr:move_to(-extents.width/2.0, extents.height/2.0)
      cr:show_text(text)
      cr:restore()
   end,
   normal = function(self)
      local l = math.sqrt(self.x * self.x + self.y * self.y)
      return Point:new {x = self.x/l, y = self.y/l}
   end,
   __sub = function(self, p)
      return Point:new {x = self.x - p.x, y = self.y - p.y}
   end
}

function Point:new(o)
   o = o or {x = 0.0, y = 0.0}
   setmetatable(o, self)
   self.__index = self
   return o
end

function Point:copy(o)
   if (getmetatable(o) == self)
   then
      o = {x=o.x, y=o.y}
      setmetatable(o, self)
      self.__index = self
      return o
   else
      return nil
   end
end


-- ----------------------------------------------------------------------
--
-- Circle
--
-- ----------------------------------------------------------------------

Circle = {
   center, radius = Point:new {0.0, 0.0}, 0.0,
   stroke = function(self, cr)
      cr:new_path()
      cr:arc(self.center.x, self.center.y, self.radius, 0, 2.0 * math.pi)
      cr:stroke()
      print(string.format("Circle:draw(%f %f %f)", self.center.x, self.center.y, self.radius))
   end,
   fill = function(self, cr)
      cr:new_path()
      cr:arc(self.center.x, self.center.y, self.radius, 0, 2.0 * math.pi)
      cr:fill()
      print(string.format("Circle:fill(%f %f %f)", self.center.x, self.center.y, self.radius))
   end,
   circle_intersect= function(self, c) 
      local x1, y1, x2, y2 = circle_circle_intersection(self.center.x,
							self.center.y,
							self.radius,
							c.center.x,
							c.center.y,
							c.radius)
      if not (x2 == nil) then
	 return Point:new {x = x1, y = y1}, Point:new {x = x2, y = y2}
      elseif not (x1 == nil) then
	 return Point:new {x = x1, y = y1}
      else
	 return
      end
   end
}

function Circle:new(o)
   o = o or {}
   setmetatable(o, self)
   self.__index = self
   return o
end

-- ----------------------------------------------------------------------
--
-- Line
--
-- ----------------------------------------------------------------------

Line = {
   a, b = Point:new {0.0, 0.0}, Point:new {0.0, 0.0},
   stroke = function(self, cr)
      cr:new_path()
      cr:move_to(self.a.x, self.a.y)
      cr:line_to(self.b.x, self.b.y)
      print(self.a, self.b)
      cr:stroke()
   end,
   circle_intersect = function(self, c)
      local ix1, iy1, ix2, iy2 = circle_line_intersection(self.a.x, self.a.y,
							  self.b.x, self.b.y,
							  c.center.x, c.center.y, c.radius)
      if not (ix2 == nil) then
	 return Point:new { x = ix1, y = iy1 }, Point:new { x = ix2, y = iy2 }
      elseif not (ix1 == nil) then
	 return Point:new { x = ix1, y = iy1 }
      else
	 return
      end
   end,
   scale = function(self, s)
      return Line:new {a = Point:new {x = self.a.x * s, y = self.a.y * s},
		       b = Point:new {x = self.b.x * s, y = self.b.y * s}}
   end,
   translate = function(self, s)
      local x = Point:new{x = self.b.x - self.a.x, y = self.b.y - self.a.y}:normal():rotate(deg2rad(90.0))
      print("--", x)
      return Line:new{a = Point:new {x = self.a.x + x.x*s, y = self.a.y + x.y*s},
		      b = Point:new {x = self.b.x + x.x*s, y = self.b.y + x.y*s}}
   end,
   __tostring = function(self)
      return string.format("Line((%6.2f, %6.2f) -- (%6.2f, %6.2f))", self.a.x, self.a.y, self.b.x, self.b.y)
   end
}
function Line:new(o)
   o = o or {}
   setmetatable(o, self)
   self.__index = self
   return o
end

function Line:copy(o)
   if (getmetatable(o) == self)
   then
      o = {a = self.a:copy(), b = self.b:copy()}
      setmetatable(o, self)
      self.__index = self
      return o
   else
      return nil
   end
end

-- ----------------------------------------------------------------------
--
-- trefoil
--
-- ----------------------------------------------------------------------

function trefoil(params, cr)
   local unit_point = Point:new{x = 1.0, y = 0.0}
   local unit_point_210 = unit_point:rotate(deg2rad(210.0))
   local unit_point_330 = unit_point:rotate(deg2rad(330.0))

   -- Do lower trefoil

   local left_outer = Circle:new{center = unit_point_210:scale(params.outer_offset), radius = params.outer_radius}
   local right_outer = Circle:new{center = unit_point_330:scale(params.outer_offset), radius = params.outer_radius}

   local left_inner = Circle:new{center = unit_point_210:scale(params.inner_offset), radius = params.inner_radius}
   local right_inner = Circle:new{center = unit_point_330:scale(params.inner_offset), radius = params.inner_radius}

   local center_circle = Circle:new{center = Point:new {x = 0.0, y = 0.0}, radius = params.center_radius}

   -- Make line double radius to get clean intersection

   local left_horn_line = Line:new{a = Point:new(), b = unit_point_210}:scale(params.outer_offset + 2.0 * params.outer_radius):translate(params.horn_gap/2.0)
   local right_horn_line = Line:new{a = Point:new(), b = unit_point_330}:scale(params.outer_offset + 2.0 * params.outer_radius):translate(-params.horn_gap/2.0)

   local left_center_line = Line:new{a = Point:new(), b = unit_point_210}:scale(params.outer_offset + 2.0 * params.outer_radius):translate(params.circle_gap/2.0)
   local right_center_line = Line:new{a = Point:new(), b = unit_point_330}:scale(params.outer_offset + 2.0 * params.outer_radius):translate(-params.circle_gap/2.0)

   local guides = false
   if guides then   
      left_inner:stroke(cr)
      right_inner:stroke(cr)
      left_outer:stroke(cr)
      right_outer:stroke(cr)
      center_circle:stroke(cr)
      left_horn_line:stroke(cr)
      right_horn_line:stroke(cr)
   end
   
   local left_horn_outer,_ = left_horn_line:circle_intersect(left_outer) -- A
   local _, outer_intersect = left_outer:circle_intersect(right_outer) -- B
   local right_horn_outer,_ = right_horn_line:circle_intersect(right_outer) -- C

   local right_horn_inner,_ = right_horn_line:circle_intersect(right_inner) -- D
   local _,right_inner_gap = right_center_line:circle_intersect(right_inner) -- E
   local right_circle_gap,_ = right_center_line:circle_intersect(center_circle) -- F
   local left_circle_gap,_ = left_center_line:circle_intersect(center_circle) -- G

   local _,left_inner_gap = left_center_line:circle_intersect(left_inner) -- H
   local left_horn_inner,_ = left_horn_line:circle_intersect(left_inner) -- I

   local ang_outer_l_start,_ = (left_horn_outer - left_outer.center):polar()
   local ang_outer_l_end,_ = (outer_intersect - left_outer.center):polar()

   local ang_outer_r_start,_ = (outer_intersect - right_outer.center):polar()
   local ang_outer_r_end,_ = (right_horn_outer - right_outer.center):polar()

   local ang_inner_r_start,_ = (right_horn_inner - right_inner.center):polar()
   local ang_inner_r_end,_ = (right_inner_gap - right_inner.center):polar()

   local ang_circle_start,_ = right_circle_gap:polar()
   local ang_circle_end,_ = left_circle_gap:polar()
   
   local ang_inner_l_start,_ = (left_inner_gap - left_inner.center):polar()
   local ang_inner_l_end,_ = (left_horn_inner - left_inner.center):polar()
   
   cr:new_path()
   cr:arc(left_outer.center.x, left_outer.center.y, left_outer.radius, ang_outer_l_start, ang_outer_l_end)   -- A--B
   cr:arc(right_outer.center.x, right_outer.center.y, right_outer.radius, ang_outer_r_start, ang_outer_r_end) -- B--C
   cr:line_to(right_horn_inner.x, right_horn_inner.y) -- C-D
   cr:arc_negative(right_inner.center.x, right_inner.center.y, right_inner.radius, ang_inner_r_start, ang_inner_r_end) -- D--E
   cr:line_to(right_inner_gap.x, right_inner_gap.y) -- E--F
   cr:arc_negative(0.0, 0.0, params.center_radius, ang_circle_start, ang_circle_end) -- F-G
   cr:line_to(left_inner_gap.x, left_inner_gap.y) -- G-H
   cr:arc_negative(left_inner.center.x, left_inner.center.y, left_inner.radius, ang_inner_l_start, ang_inner_l_end) -- H-I
   cr:close_path()

   local trefoil_path = cr:copy_path()

   -- --------------------
   -- Ring
   -- --------------------

   local ring_center = Circle:new {center = Point:new{x = 0.0, y = params.inner_offset}, radius = params.inner_radius - params.circle_gap}
   local ring_inner = Circle:new {center = Point:new{x = 0.0, y = 0.0}, radius = params.ring_inner_radius}
   local ring_outer = Circle:new {center = Point:new{x = 0.0, y = 0.0}, radius = params.ring_outer_radius}
--   ring_center:stroke(cr)
--   ring_inner:stroke(cr)
--   ring_outer:stroke(cr)

   local ring_inner_right, ring_inner_left = ring_center:circle_intersect(ring_inner) -- J & K
   local ring_outer_right, ring_outer_left = ring_center:circle_intersect(ring_outer) -- L & M
   local inner_center = Point:new {x = 0.0, y = params.inner_offset}

   cr:new_path()
   cr:move_to(ring_inner_left.x, ring_inner_left.y)
   cr:arc_negative(0.0, 0.0, params.ring_inner_radius, ring_inner_left:polar(), ring_inner_right:polar())
   cr:arc(0.0, params.inner_offset, params.inner_radius-params.circle_gap,
	  (ring_inner_right - inner_center):polar(),
	  (ring_outer_right - inner_center):polar())
   cr:arc(0.0, 0.0, params.ring_outer_radius, ring_outer_right:polar(), ring_outer_left:polar())
   cr:arc(0.0, params.inner_offset, params.inner_radius-params.circle_gap,
	  (ring_outer_left - inner_center):polar(),
	  (ring_inner_left - inner_center):polar())
   cr:close_path()
   ring_path = cr:copy_path()

   -- --------------------
   -- Display paths
   -- --------------------

   cr:set_line_width(0.5)

   cr:new_path()
   for i = 0,2,1 do
      cr:save()
      cr:rotate(deg2rad(120.0) * i)
      cr:append_path(trefoil_path)
      cr:append_path(ring_path)
      cr:restore()
   end
   cr:set_source_rgb(1.0, 0.0, 0.0)
   cr:fill_preserve()
   cr:set_source_rgb(0.0, 0.0, 0.0)
   cr:stroke()

   local old = false
   if old then   
      for i = 0,2,1 do
	 cr:save()
	 cr:rotate(deg2rad(120.0) * i)
	 cr:new_path()
	 cr:append_path(trefoil_path)
	 cr:set_source_rgb(1.0, 0.0, 0.0)
	 cr:fill_preserve()
	 cr:set_source_rgb(0.0, 0.0, 0.0)
	 cr:stroke()
	 cr:new_path()
	 cr:append_path(ring_path)
	 cr:set_source_rgb(1.0, 0.0, 0.0)
	 cr:fill_preserve()
	 cr:set_source_rgb(0.0, 0.0, 0.0)
	 cr:stroke()
	 cr:restore()
      end
   end
   

   -- Add dots

   local marking = false
   if marking then
      left_horn_outer:fill(cr, 4.0)
      right_horn_outer:fill(cr, 4.0)
      outer_intersect:fill(cr, 4.0)
      right_horn_inner:fill(cr, 4.0)
      right_inner_gap:fill(cr, 4.0)
      right_circle_gap:fill(cr, 4.0)
      left_circle_gap:fill(cr, 4.0)
      left_inner_gap:fill(cr, 4.0)
      left_horn_inner:fill(cr, 4.0)
      ring_inner_left:fill(cr, 4.0) -- J
      ring_inner_right:fill(cr, 4.0) -- K
      ring_outer_left:fill(cr, 4.0) -- L
      ring_outer_right:fill(cr, 4.0) -- M
      
      -- Add labels
      
      cr:set_source_rgb(0.0, 0.0, 0.0)
      left_horn_outer:text(cr, 'A')
      outer_intersect:text(cr, 'B')
      right_horn_outer:text(cr, 'C')
      right_horn_inner:text(cr, 'D')
      right_inner_gap:text(cr, 'E')
      right_circle_gap:text(cr, 'F')
      left_circle_gap:text(cr, 'G')
      left_inner_gap:text(cr, 'H')
      left_horn_inner:text(cr, 'I')
      ring_inner_left:text(cr, 'J')
      ring_inner_right:text(cr, 'K')
      ring_outer_right:text(cr, 'L')
      ring_outer_left:text(cr, 'M')
   end
end

-- ----------------------------------------------------------------------
--
-- main
--
-- ----------------------------------------------------------------------

local params = {
   center_radius = 30.0,
   outer_offset = 110.0,
   outer_radius = 150.0,
   inner_offset = 150.0,
   inner_radius = 105.0,
   ring_inner_radius = 100.0,
   ring_outer_radius = 135.0,
   horn_gap = 40.0,
   circle_gap = 10.0,
   width = 800.0,
   height = 800.0,
}

local surface = cairo.SvgSurface.create('point.svg', params.width, params.height)

local cr = cairo.Context.create(surface)
cr:translate(params.width/2.0, params.height/2.0)
local scale = 4.0 / 3.0;
cr:scale(scale, -scale)
cr:select_font_face('URW Gothic L', 'normal', 'bold');
cr:set_font_size(14);
trefoil(params, cr)

