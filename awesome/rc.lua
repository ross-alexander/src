-- ----------------------------------------------------------------------
--
-- Special background function
--
-- ----------------------------------------------------------------------

local lgi = require 'lgi'
local Gtk = lgi.require('Gtk', '3.0')

local scanner = require('Scanner').new()
scanner:add('/locker/images/awesome')
scanner:rescan()

-- ----------------------------------------------------------------------
--
-- Set background from images
--
-- ----------------------------------------------------------------------

function Background(scanner, beautiful, gears)
   local GdkPixbuf = lgi.GdkPixbuf
   local cairo = lgi.cairo
   local assert = lgi.assert
   local file

   if (scanner == nil) then
       file = beautiful.wallpaper
   else
     file = scanner:random()
     if (file == nil) then
        file = beautiful.wallpaper
     end
   end

   local background = assert(GdkPixbuf.Pixbuf.new_from_file(file))

   print(file, background)
   
   if background then
      for s = 1, screen.count() do
	 local ls = screen[s]
	 local screen_width, screen_height, bg_width, bg_height = ls.geometry.width, ls.geometry.height, background.width, background.height
	 local width_ratio, height_ratio = screen_width / bg_width, screen_height / bg_height
	 local ratio
	 if (width_ratio > height_ratio) then
	    ratio = height_ratio
	 else
	    ratio = width_ratio
	 end
	 local width, height = math.floor(bg_width * ratio), math.floor(bg_height * ratio)

	 local dest = background:scale_simple(width, height, 'HYPER')

	 local surface = cairo.ImageSurface.create('ARGB32', screen_width, screen_height)
	 local cr = cairo.Context.create(surface)
	 cr:set_source_rgb(0, 0, 0)
	 cr:paint()
	 cr:set_source_pixbuf(dest, (screen_width - width)/2, (screen_height - height)/2)
	 cr:paint()

	 gears.wallpaper.centered(surface, s)
--	 gears.wallpaper.maximized(surface, s, true)
      end
   end
end

-- ----------------------------------------------------------------------
--
-- Load modules
--
-- ----------------------------------------------------------------------

local awful = require("awful")
awful.rules = require("awful.rules")
local beautiful = require("beautiful")
local gears = require("gears")
local wibox = require("wibox")

-- Set various variables

terminal = "xterm"
editor = os.getenv("EDITOR") or "emacs"
editor_cmd = terminal .. " -e " .. editor

-- Load theme

beautiful.init("/home/ralexand/.config/awesome/themes/hepazulian/theme.lua")

-- Set background

Background(scanner, beautiful, gears)

-- Only have floating layout

local layouts =
{
    awful.layout.suit.floating,
}

-- Set single tag per screen

tags = {}
for s = 1, screen.count() do
    -- Each screen has its own tag table.
    tags[s] = awful.tag({ 1 }, s, layouts[1])
end

-- Set modkey to windows (Mod4) or alt (Mod1)

modkey = "Mod4"

-- Set client move and resize

clientbuttons = awful.util.table.join(
    awful.button({ }, 1, function (c) client.focus = c; c:raise() end),
    awful.button({ modkey }, 1, awful.mouse.client.move),
    awful.button({ modkey }, 3, awful.mouse.client.resize))

-- Set minimal global keys

globalkeys = awful.util.table.join(
   awful.key({ modkey, "Control" }, "b", function () Background(scanner, beautiful, gears)    end),
   awful.key({ modkey, "Shift"   }, "j", function () awful.client.swap.byidx(  1)    end),
   awful.key({ modkey, "Shift"   }, "k", function () awful.client.swap.byidx( -1)    end),
   awful.key({ modkey, "Control" }, "j", function () awful.screen.focus_relative( 1) end),
   awful.key({ modkey, "Control" }, "k", function () awful.screen.focus_relative(-1) end),
   awful.key({ modkey,           }, "Return", function () awful.util.spawn(terminal) end),
   awful.key({ modkey, "Control" }, "r", awesome.restart),
   awful.key({ modkey, "Shift"   }, "q", awesome.quit),
   awful.key({ modkey,           }, "Tab",
      function ()
	 -- awful.client.focus.history.previous()
	 awful.client.focus.byidx(-1)
	 if client.focus then
            client.focus:raise()
	 end
   end),
   awful.key({ modkey, "Shift"   }, "Tab",
      function ()
	 -- awful.client.focus.history.previous()
	 awful.client.focus.byidx(1)
	 if client.focus then
            client.focus:raise()
	 end
   end)
)


root.keys(globalkeys)

-- Mouse  bindings

root.buttons(awful.util.table.join(
    awful.button({ }, 1, function () mymainmenu:toggle() end),
    awful.button({ }, 4, awful.tag.viewnext),
    awful.button({ }, 5, awful.tag.viewprev)
))

-- Set simple rules

awful.rules.rules = {
    -- All clients will match this rule.
    { rule = { },
      properties = {  border_width = beautiful.border_width,
		      border_color = beautiful.border_normal,
		      focus = awful.client.focus.filter,
		      raise = true,
		      buttons = clientbuttons }
    },
}

function client_manage(c, startup)
   local titlebars_enabled = true
   
   if titlebars_enabled and (c.type == "normal" or c.type == "dialog") then
      -- buttons for the titlebar
      local buttons = awful.util.table.join(
	 awful.button({ }, 1, function()
				 client.focus = c
				 c:raise()
				 awful.mouse.client.move(c)
			   end),
	 awful.button({ }, 3, function()
				 client.focus = c
				 c:raise()
				 awful.mouse.client.resize(c)
			   end)
      )
      
      -- Widgets that are aligned to the left

      local left_layout = wibox.layout.fixed.horizontal()
      left_layout:add(awful.titlebar.widget.iconwidget(c))
      left_layout:buttons(buttons)
      
      -- Widgets that are aligned to the right

      local right_layout = wibox.layout.fixed.horizontal()
      right_layout:add(awful.titlebar.widget.floatingbutton(c))
      right_layout:add(awful.titlebar.widget.maximizedbutton(c))
      right_layout:add(awful.titlebar.widget.stickybutton(c))
      right_layout:add(awful.titlebar.widget.ontopbutton(c))
      right_layout:add(awful.titlebar.widget.closebutton(c))
      
      -- The title goes in the middle

      local middle_layout = wibox.layout.flex.horizontal()
      local title = awful.titlebar.widget.titlewidget(c)
      title:set_align("center")
      middle_layout:add(title)
      middle_layout:buttons(buttons)
      
      -- Now bring it all together

      local layout = wibox.layout.align.horizontal()
      layout:set_left(left_layout)
      layout:set_right(right_layout)
      layout:set_middle(middle_layout)
      awful.titlebar(c):set_widget(layout)
   end
end

client.connect_signal("manage", client_manage)
client.connect_signal("focus", function(c) c.border_color = beautiful.border_focus end)
client.connect_signal("unfocus", function(c) c.border_color = beautiful.border_normal end)

-- Create the main menu

myawesomemenu = {
   { "edit config", editor_cmd .. " " .. awesome.conffile },
   { "restart", awesome.restart },
   { "quit", function () awesome.quit() end}
}

mymainmenu = awful.menu(
   {
      items = {
	 { "awesome", myawesomemenu, beautiful.awesome_icon },
	 { "open terminal", terminal },
	 { "Gnome Terminal", "gnome-terminal" },
	 { "termit", "termit" },
	 { "emacs", "emacs" },
	 { "firefox", "/opt/firefox/firefox" },
	 { "Freecell", "sol" },
	 { "Sudoku", "gnome-sudoku" }
      }
   }
)

-- mylauncher = awful.widget.launcher({ image = beautiful.awesome_icon, menu = mymainmenu })

-- wibox

mytextclock = wibox.widget.textclock()

mytaglist = {}
mytaglist.buttons = awful.util.table.join(
   awful.button({ }, 1, awful.tag.viewonly),
   awful.button({ modkey }, 1, awful.client.movetotag),
   awful.button({ }, 3, awful.tag.viewtoggle),
   awful.button({ modkey }, 3, awful.client.toggletag),
   awful.button({ }, 4, function(t) awful.tag.viewnext(awful.tag.getscreen(t)) end),
   awful.button({ }, 5, function(t) awful.tag.viewprev(awful.tag.getscreen(t)) end)
)


mytasklist = {}
mytasklist.buttons = awful.util.table.join(
   awful.button({ }, 1,
		function (c)
		   if c == client.focus then
		      c.minimized = true
		   else
		      -- Without this, the following
		      -- :isvisible() makes no sense
		      c.minimized = false
		      if not c:isvisible() then
			 awful.tag.viewonly(c:tags()[1])
		      end
		      -- This will also un-minimize
		      -- the client, if needed
		      client.focus = c
		      c:raise()
		   end
	     end),
   awful.button({ }, 3,
		function ()
		   if instance then
		      instance:hide()
		      instance = nil
		   else
		      instance = awful.menu.clients({theme = { width = 250 } })
		   end
	     end),
   awful.button({ }, 4,
		function ()
		   awful.client.focus.byidx(1)
		   if client.focus then client.focus:raise() end
	     end),
   awful.button({ }, 5,
		function ()
		   awful.client.focus.byidx(-1)
		   if client.focus then client.focus:raise() end
	  end))

-- add menu launcher

mylauncher = awful.widget.launcher({ image = beautiful.awesome_icon, menu = mymainmenu })

mywibox = {}

for s = 1, screen.count() do
   mywibox[s] = awful.wibar({ position = "bottom", screen = s })

   -- Create a taglist widget

   mytaglist[s] = awful.widget.taglist(s, awful.widget.taglist.filter.all, mytaglist.buttons)
   
   -- Create a tasklist widget

   mytasklist[s] = awful.widget.tasklist(s, awful.widget.tasklist.filter.currenttags, mytasklist.buttons)

    -- Widgets that are aligned to the left

   local left_layout = wibox.layout.fixed.horizontal()
   left_layout:add(mylauncher)
   left_layout:add(mytaglist[s])
   --    left_layout:add(mypromptbox[s])
   
   -- Widgets that are aligned to the right

   local right_layout = wibox.layout.fixed.horizontal()
   right_layout:add(mytextclock)
   
   -- Now bring it all together (with the tasklist in the middle)
   
   local layout = wibox.layout.align.horizontal()
   layout:set_left(left_layout)
   layout:set_middle(mytasklist[s])
   layout:set_right(right_layout)
   mywibox[s]:set_widget(layout)
end
 
-- Apply a random wallpaper every changeTime seconds.
local changeTime = 10
local wallpaperTimer = gears.timer { timeout = changeTime }
wallpaperTimer:connect_signal("timeout", function()
					    Background(scanner, beautiful, gears)
				      end)

-- initial start when rc.lua is first run
-- wallpaperTimer:start()
