/* ----------------------------------------------------------------------
--
-- thumbnail
--
-- Thumbnail contact sheet generator using gegl or cairo (& gdk-pixbuf)
-- and lua.
--
-- 2019-12-31: Ross Alexander
--   First cut
--
---------------------------------------------------------------------- */

#include <string>
#include <map>
#include <iostream>
#include <filesystem>
#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <fmt/printf.h>
#include <boost/program_options.hpp>
#include <libxml++/libxml++.h>
#include <libcroco/libcroco.h>

#include "image.h"
#include "thumbnail.h"

namespace fs = std::filesystem;


thumbnail_t::thumbnail_t()
{
};

void thumbnail_t::dst_set(std::string d)
{
  dst_dir = d;
}
/* ----------------------------------------------------------------------
--
-- dir_add
--
---------------------------------------------------------------------- */

int thumbnail_t::dir_add(std::string s)
{
  if(dir_table.contains(s))
    return 0;
  dir_table[s] = 1;
  return 1;
}

/* ----------------------------------------------------------------------
--
-- dis_scan
--
---------------------------------------------------------------------- */

int thumbnail_t::dir_scan()
{
  int count = 0;

  // Loop over each entry in the directory table
    for (auto &i : dir_table)
    {
      // Use std::filesystem::directory_iterator to get list of entries
      for (auto &p : fs::directory_iterator(i.first))
	{
	  fs::file_status stat = fs::status(p);
	  if (fs::is_regular_file(stat))
	    {
	      image_table[p.path()] = nullptr;
	      count++;
	    }
	}
    }
  return count;
}


void thumbnail_t::scale(int dimen)
{
  for (auto &i : image_table)
    {
      image_t *image = i.second;
      image->load();
      image_t *scaled = image->scale(100);

      fs::path dst = image->path.filename();
      fs::path base = "/tmp/thumbs/";

      scaled->save(base / dst);
    }
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

extern void thumbnail_lua(std::string);
extern void thumbnail_lua_options(boost::program_options::variables_map);

int main(int argc, char* argv[])
{
  gegl_init(&argc, &argv);

  // Use boost's program_options to parse arguments
  
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "describe arguments")
    ("maximum", po::value<int>(), "maximum")
    ("src", po::value<std::string>(), "src")
    ("dst", po::value<std::string>(), "dst")
    ("fg", po::value<std::string>(), "fg")
    ("bg", po::value<std::string>(), "bg")
    ("lua", po::value<std::string>(), "lua")
    ("size", po::value<int>(), "size")
    ;

  po::variables_map options;
  po::store(po::parse_command_line(argc, argv, desc), options);
  po::notify(options);

  if (options.count("lua"))
    {
      thumbnail_lua_options(options);
      exit(0);
    }
  
  thumbnail_t thumbnail;

  if (options.count("dst"))
    thumbnail.dst_set(options["dst"].as<std::string>());
  if (options.count("src"))
    thumbnail.dir_add(options["src"].as<std::string>());
  thumbnail.dir_scan();
  thumbnail.scale(100);
  
}
