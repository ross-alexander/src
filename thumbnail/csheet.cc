/* ----------------------------------------------------------------------
--
-- csheet
--
-- Thumbnail contact sheet generator using gegl using CSS style sheet using
-- horrible hack of creating an in-memory representation of the sheet as
-- XML to allow libcroco to apply CSS styling.
--
--
-- 2023-10-22: Ross Alexander
--   Copy gegl_t from image.cc as image_t.

-- 2021-08-22: Ross Alexander
--  image_t changed enough that doesn't work well with this so use
--  internal image_t class rather than image_t.h & image_t.cc
--
-- 2019-10-10: Ross Alexander
--   First cut
--
---------------------------------------------------------------------- */

#include <string>
#include <map>
#include <iostream>
#include <filesystem>
#include <gegl.h>
#include <fmt/printf.h>
#include <boost/program_options.hpp>
#include <libxml++/libxml++.h>
#include <libcroco/libcroco.h>

namespace fs = std::filesystem;

class image_t {
protected:
public:
  GeglRectangle bounds;
  int valid;
  GeglBuffer *buffer;
  fs::path path;
  image_t();
  image_t(int, int);
  image_t(fs::path);
  virtual int load();
  virtual int save();
  virtual int save(fs::path);
  virtual void fill(std::string);
  virtual image_t *scale(int);
  virtual image_t *scale(double);
  virtual void compose(image_t*, int, int);
  //  virtual bounds_t frame(double, double, double, double, double, const char*);
  //  virtual bounds_t text(double, double, double, int, const char*, const char*);
  //  virtual int get_bounds(bounds_t&);
  int width();
  int height();
  int is_valid();
  std::string description();
};

/* ----------------------------------------------------------------------
--
-- image_t
--
---------------------------------------------------------------------- */

image_t::image_t()
{
  valid = 0;
}

image_t::image_t(fs::path p)
{
  path = p;
  load();
}

image_t::image_t(int w, int h)
{
  GeglRectangle r;
  r.x = bounds.x = 0;
  r.y = bounds.y = 0;
  r.width = bounds.width = w;
  r.height = bounds.height = h;
  valid = 1;
}

int image_t::width()
{
  return bounds.width;
}

int image_t::height()
{
  return bounds.height;
}

int image_t::is_valid()
{
  return valid;
}

std::string image_t::description()
{
  std::string s;
  if (!is_valid())
    {
      return fmt::sprintf("%s: invalid", std::string(path));
    }
  return fmt::sprintf("[%s] %s: %d x %d %s", typeid(*this).name(), std::string(path), width(), height(), is_valid() ? "valid buffer" : "no buffer");
}

int image_t::load()
{
  if (path.empty())
    {
      valid = 0;
      return valid;
    }
  g_autoptr(GeglNode) graph = gegl_node_new();
  GeglNode* load = gegl_node_new_child(graph,
				       "operation", "gegl:load",
				       "path", path.c_str(),
				       nullptr);
  GeglNode* sink = gegl_node_new_child (graph,
					"operation", "gegl:buffer-sink",
					"buffer", &buffer,
					nullptr);


  gegl_node_link(load, sink);
  GeglRectangle r = gegl_node_get_bounding_box(sink);
  gegl_node_process(sink);
  if (buffer == nullptr)
    {
      valid = 0;
      return 0;
    }
  valid = 1;
  bounds.x = r.x;
  bounds.y = r.y;
  bounds.width = r.width;
  bounds.height = r.height;
  return valid;
}

int image_t::save()
{
  return save(path);
}

int image_t::save(fs::path path)
{
  if (!buffer)
    return 0;

  std::cout << "save " << path << "\n";
  
  GeglNode *graph = gegl_node_new();
  GeglNode *source = gegl_node_new_child(graph,
					 "operation", "gegl:buffer-source",
					 "buffer", buffer,
					 nullptr);
  GeglNode *save = gegl_node_new_child(graph,
				       "operation", "gegl:save",
				       "path", path.c_str(),
				       nullptr);
  
  gegl_node_link_many(source, save, nullptr);
  gegl_node_process(save);
  return 1;
}


void image_t::fill(std::string color)
{
}

image_t* image_t::scale(int)
{
  return new image_t();
}

image_t* image_t::scale(double)
{
  return new image_t();
}

void image_t::compose(image_t *i, int x, int y)
{
}



/* ----------------------------------------------------------------------
--
-- definitions
--
---------------------------------------------------------------------- */

class csheet_t {
private:
  std::map<std::string, int> dir_table;
  std::map<std::string, image_t*> image_table;
  int cols;
  int rows;
  int max;
  double size;
  double border;
  double hardness;
  double linewidth;
  double fontsize;
  std::string fg_color;
  std::string bg_color;
  CRStyleSheet *user_stylesheet;
  CRStyleSheet *base_stylesheet;
  CRCascade *cascade;
  xmlpp::Document *xml_doc;
  xmlpp::Element *xml_root;
  public:
  csheet_t();
  int dir_add(std::string);
  int dir_scan();
  int image_load();
  image_t* image_thumbnail(std::string);
  int cols_set(int);
  int max_set(int);
  void fg_color_set(std::string);
  void bg_color_set(std::string);
  void add_css(std::string);
};

/* ----------------------------------------------------------------------
--
-- globals
--
---------------------------------------------------------------------- */

static const char* base_sheet =
  "sheet {\n"
  "cols: 6;\n"
  "image-size: 200;\n"
  "background: white;\n"
  "}\n"
  "img {\n"
  "font-size: 14;\n"
  "font-family: \"Linux Libertine T\";\n"
  "}\n"
  ;

/* ----------------------------------------------------------------------
--
-- csheet
--
---------------------------------------------------------------------- */

csheet_t::csheet_t()
{
  max = 0;
  cols = 8;
  size = 200;
  border = 4;
  linewidth = 2.0;
  hardness = 1.0;
  fontsize = 10.0;
  fg_color = "rgba(1.0,0.0,0.0,1.0)";
  bg_color = "rgb(0.0, 1.0, 0.0)";
  const char *xml_ns = "";
  xml_doc = new xmlpp::Document("1.0");
  xml_root = xml_doc->create_root_node("sheet", "", xml_ns);
  assert(xml_root);
  assert(xml_root->cobj());

  user_stylesheet = 0;
  CRStatus status = cr_om_parser_simply_parse_buf((const guchar*)base_sheet, strlen(base_sheet), CR_ASCII, &base_stylesheet);
  assert (status == CR_OK);
}

int csheet_t::cols_set(int n)
{
  int old = cols;
  cols = n;
  return old;
}

int csheet_t::max_set(int n)
{
  int old = max;
  max = n;
  return max;
}

void csheet_t::bg_color_set(std::string color)
{
  bg_color = color;
}

int csheet_t::dir_add(std::string s)
{
  if(dir_table.contains(s))
    return 0;
  dir_table[s] = 1;
  return 1;
}


static GeglNode *create_text_node(GeglNode *graph, std::string string, std::string font, double size)
{
  GeglNode *text;
  GeglColor *color = gegl_color_new("black");

  printf("%s [ %s @ %4.2f ]\n", string.c_str(), font.c_str(), size);

  text = gegl_node_new_child(graph,
			     "operation", "gegl:text",
			     "color", color,
			     "font", font.c_str(),
			     "size", size,
			     "string", string.c_str(),
			     0);

  g_object_unref(color);
  return text;
}


/* ----------------------------------------------------------------------
--
-- dis_scan
--
---------------------------------------------------------------------- */

int csheet_t::dir_scan()
{
  int count = 0;

  // Loop over each entry in the directory table
  
  for (auto &i : dir_table)
    {
      // Use std::filesystem::directory_iterator to get list of entries
      for (auto &p : fs::directory_iterator(i.first))
	{
	  fs::file_status stat = fs::status(p);
	  // Check a regular file and if so load into GEGL buffer	  
	  if (fs::is_regular_file(stat))
	    {
	      image_t *image = new image_t(p);
	      image_table[std::string(p.path())] = image;
	      count++;
	    }
	}
    }
  return count;
}

/* ----------------------------------------------------------------------
--
-- image_load
--
---------------------------------------------------------------------- */

int csheet_t::image_load()
{
  int count = 0;
  for (auto &i : image_table)
    {
      image_t *image = i.second;
      g_autoptr(GeglNode) graph = gegl_node_new();
      GeglNode* load = gegl_node_new_child(graph,
					   "operation", "gegl:load",
					   "path", image->path.c_str(),
					   0);
      
      GeglNode* sink = gegl_node_new_child (graph,
					    "operation", "gegl:buffer-sink",
					    "buffer", &image->buffer,
					    0);

      gegl_node_link(load, sink);
      gegl_node_process(sink);
      image->valid = 1;
      count++;
      if (max && (count >= max)) break;
    }
  return count;
}

/* ----------------------------------------------------------------------
--
-- image_thumbnail
--
---------------------------------------------------------------------- */

std::map<std::string,std::string> print_properties_real(CRPropList *proplist)
{
  std::map<std::string,std::string> res;
  for (CRPropList *cur_pair = proplist ; cur_pair; cur_pair= cr_prop_list_get_next (cur_pair))
    {
      CRDeclaration *decl = NULL ;
      cr_prop_list_get_decl(cur_pair, &decl);
      if (decl)
	{
	  std::string k  = cr_string_peek_raw_str(decl->property);
	  std::string v = (char*)(cr_term_to_string(decl->value));
	  res[k] = v;
	  
 	  // printf("%s\n", cr_string_peek_raw_str(decl->property));
	  // int count = 0;
	  // for (CRTerm* term = decl->value ; term != 0; term = term->next)
	  //   {
	  //     if (guchar *str = cr_term_to_string(term))
	  // 	{
	  // 	  printf("++ %02d %s\n", count++, str) ;
	  // 	  g_free(str);
	  // 	}
	  //   }
	}
    }
  return res;
}

image_t* csheet_t::image_thumbnail(std::string file)
{
  /* --------------------
     Check if CSS is available
     -------------------- */

  cascade = cr_cascade_new(0, user_stylesheet, base_stylesheet);
  assert(cascade);
  
  CRSelEng *selector = cr_sel_eng_new();
  CRPropList *prop_list = NULL;
  assert(selector);
  assert(cascade);
  assert(xml_root);
  assert(xml_root->cobj());
  assert(&prop_list);
  
  CRStatus status = cr_sel_eng_get_matched_properties_from_cascade(selector, cascade, xml_root->cobj(), &prop_list);
  std::map<std::string,std::string> sel = print_properties_real(prop_list);
  if (sel.contains("cols")) cols = stod(sel["cols"], 0);
  if (sel.contains("image-size")) size = stoi(sel["image-size"], 0, 10);
  if (sel.contains("background")) bg_color = sel["background"];
  if (sel.contains("border-width")) linewidth = std::stod(sel["border-width"]);

  cr_prop_list_destroy(prop_list);
  cr_sel_eng_destroy(selector);

  int count = 0;
  xmlpp::Element *row = 0;

  /* --------------------
     Build table of images into xml
     -------------------- */

  for (auto &i : image_table)
    {
      if (i.second->valid)
	{
	  if (!row)
	    {
	      row = xml_root->add_child_element("row", "");
	      std::string r = fmt::sprintf("%d", (count / cols) + 1);
	      row->set_attribute("row", r);
	      if (((count/cols)+1)%2)
		row->set_attribute("class", "even");
	      else
		row->set_attribute("class", "odd");
	    }
	  count++;
	  assert(row);

	  xmlpp::Element *frame = row->add_child_element("frame", "");	  
	  xmlpp::Element *img = frame->add_child_element("img", "");
	  img->set_attribute("src", std::string(i.second->path).c_str());

	  if (row && ((count%cols) == 0))
	    {
	      row = 0;
	    }
	}
    }
  int rows = (count-1)/cols + 1;

  printf("images %d matrix (%d x %d)\n", count, cols, rows);

  /* --------------------
     Create destination buffer
     -------------------- */
  
  GeglRectangle dst_rect;
  dst_rect.x = 0;
  dst_rect.y = 0;
  dst_rect.width = cols * size + (cols + 1) * border;
  dst_rect.height = rows * (size + (2 * fontsize)) + (rows + 1) * border;

  GeglColor* bg = gegl_color_new(bg_color.c_str());
  GeglBuffer *dst = gegl_buffer_new(&dst_rect, babl_format("RGBA u8"));
  
  count = 0;

  /* --------------------
     Create background using gegl:rectangle
     -------------------- */
  
  GeglNode* graph = gegl_node_new();
  GeglNode* rectangle = gegl_node_new_child(graph,
					    "operation", "gegl:rectangle",
					    "color", bg,
					    "height", (double)dst_rect.height,
					    "width", (double)dst_rect.width,
					    "x", (double)0.0,
					    "y", (double)0.0,
					    NULL);


  GeglNode *src = rectangle;

  xmlpp::Node::NodeSet set = xml_root->find("/sheet/row/frame/img");
  for (unsigned int i = 0; i < set.size(); i++)
    {
      xmlpp::Element *node = dynamic_cast<xmlpp::Element*>(set[i]);
      image_t *image = image_table[node->get_attribute("src")->get_value()];

      /* --------------------
	 Calculate frame edge
	 -------------------- */
      
      double shift_x = (double)(count % cols) * (size + border) + border;
      double shift_y = (double)(count / cols) * (size + border + 2 * fontsize) + border;

      GeglColor* border_stroke = 0;
      GeglColor* frame_fill = 0;

      CRSelEng *selector = cr_sel_eng_new();
      CRPropList *prop_list = NULL;
      CRStatus status = cr_sel_eng_get_matched_properties_from_cascade(selector, cascade, node->cobj(), &prop_list);
      assert(status == CR_OK);
      assert(prop_list);

      std::map<std::string,std::string> sel = print_properties_real(prop_list);
      if (sel.contains("border-color")) border_stroke = gegl_color_new(sel["border-color"].c_str());
      if (sel.contains("background")) frame_fill = gegl_color_new(sel["background"].c_str());
      
      /* --------------------
	 Create frame path
	 -------------------- */
      
      GeglPath *path = gegl_path_new();

      gegl_path_append(path, 'M', shift_x, shift_y);
      gegl_path_append(path, 'L', shift_x + size, shift_y);
      gegl_path_append(path, 'L', shift_x + size, shift_y + size);
      gegl_path_append(path, 'L', shift_x, shift_y + size);
      gegl_path_append(path, 'L', shift_x, shift_y);
      gegl_path_append(path, 'z');

      /*
      gegl_path_append(path, 'M', shift_x, shift_y);
      gegl_path_append(path, 'L', shift_x + resize_w, shift_y);
      gegl_path_append(path, 'L', shift_x + resize_w, shift_y + resize_h);
      gegl_path_append(path, 'L', shift_x, shift_y + resize_h);
      gegl_path_append(path, 'L', shift_x, shift_y);
      gegl_path_append(path, 'z');
      */

      GeglNode *frame_over = gegl_node_new_child(graph, "operation", "gegl:over", 0);

      if (frame_fill)
	{      
	  GeglNode *frame = gegl_node_new_child(graph, "operation", "gegl:path",
						"d", path,
						"fill-opacity", 1.0,
						"fill", frame_fill,
						NULL);
	  gegl_node_connect(frame, "output", frame_over, "aux");
	}

      /* --------------------
	 Get image buffer
	 -------------------- */
      
      std::cout << count << " : " << image->path << '\n';
      GeglNode* image_source = gegl_node_new_child(graph,
						   "operation", "gegl:buffer-source",
						   "buffer", image->buffer,
						   NULL);
      
      double resize_w;
      double resize_h;
      
      const GeglRectangle *r = gegl_buffer_get_extent(image->buffer);

      /* --------------------
	 scale to fix box of size "size"
	 -------------------- */
      
      if (r->width > r->height)
	{
	  resize_w = size;
	  resize_h = (double)r->height / (double)r->width * size;
	}
      else
	{
	  resize_h = size;
	  resize_w = (double)r->width / (double)r->height * size;
	}

      GeglNode *scale = gegl_node_new_child(graph,
					    "operation", "gegl:scale-size",
					    "origin-x", 0.0,
					    "origin-y", 0.0,
					    "abyss-policy", GEGL_ABYSS_CLAMP,
					    "sampler", GEGL_SAMPLER_NOHALO,
					    "x", (double)resize_w,
					    "y", (double)resize_h,
					    NULL);

      /* --------------------
	 Center scaled image in box
	 -------------------- */

      double center_x = 0.0;
      double center_y = 0.0;
      if (resize_w > resize_h)
	{
	  center_y = (size - resize_h)/2.0;
	}
      else
	{
	  center_x = (size - resize_w)/2.0;
	}
      
      
      GeglNode *shift = gegl_node_new_child(graph,
					    "operation", "gegl:translate",
					    "x", shift_x + center_x,
					    "y", shift_y + center_y,
					    NULL);

      /* --------------------
	 Links nodes together then connect to an "over" node as aux
	 -------------------- */

      gegl_node_link_many(image_source, scale, shift, 0);
      GeglNode *image_over = gegl_node_new_child(graph, "operation", "gegl:over", NULL);
      gegl_node_connect(shift, "output", image_over, "aux");

      if (!border_stroke)
	border_stroke = gegl_color_new(fg_color.c_str());
      
      GeglNode *path_over = gegl_node_new_child(graph, "operation", "gegl:over", 0);
      GeglNode *stroke = gegl_node_new_child(graph, "operation", "gegl:path",
					     "d", path,
					     "fill-opacity", 0.0,
					     "stroke", border_stroke,
					     "stroke-width", linewidth,
					     "stroke-hardness", hardness,
					     NULL);
      gegl_node_connect(stroke, "output", path_over, "aux");

      GeglNode *text = create_text_node(graph, image->path.filename(), sel["font-family"], stod(sel["font-size"]));

      GeglRectangle bb = gegl_node_get_bounding_box(text);
      
      //      std::cout << sel["font-family"] << " at " << sel["font-size"] << " " << bb.width << "\n";
     
      GeglNode *text_translate = gegl_node_new_child(graph,
						     "operation", "gegl:translate",
						     "x", shift_x + (size - bb.width)/2,
						     "y", shift_y + size,
						     0);

      gegl_node_link(text, text_translate);
      g_object_unref(border_stroke);

      GeglNode *text_over = gegl_node_new_child(graph, "operation", "gegl:over", 0);
      gegl_node_connect(text_translate, "output", text_over, "aux");
      
      gegl_node_link_many(src, frame_over, image_over, path_over, text_over, 0);
      src = text_over;
      count++;

      if (prop_list)
	cr_prop_list_destroy(prop_list);
      cr_sel_eng_destroy(selector);
    }

  GeglNode *write_buffer = gegl_node_new_child (graph,
						"operation", "gegl:write-buffer",
						"buffer", dst,
						NULL);

  gegl_node_link(src, write_buffer);
  gegl_node_process(write_buffer);
  
  xml_doc->write_to_stream(std::cout);
  
  image_t *save_image = new image_t(file);
  save_image->buffer = dst;
  return save_image;
}


void csheet_t::add_css(std::string css_sheet_path)
{
  enum CRStatus status;

  status = cr_om_parser_simply_parse_file((const guchar*)css_sheet_path.c_str(), CR_ASCII, &user_stylesheet);
  assert (status == CR_OK);
}


/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char* argv[])
{
  gegl_init(&argc, &argv);

  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "describe arguments")
    ("maximum", po::value<int>(), "maximum")
    ("src", po::value<std::string>(), "src")
    ("dst", po::value<std::string>(), "dst")
    ("fg", po::value<std::string>(), "fg")
    ("bg", po::value<std::string>(), "bg")
    ("css", po::value<std::string>(), "css")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  csheet_t contact;

  contact.add_css("sheet.css");
  
  if (vm.count("maximum"))
    contact.max_set(vm["maximum"].as<int>());

  if (vm.count("bg"))
    contact.bg_color_set(vm["bg"].as<std::string>());
  
  if ((vm.count("src") == 0) || (vm.count("dst") == 0))
    {
      std::cerr << argv[0] << ": [--src=src directory] [--dst=dst file]\n";
      exit(1);
    }
  
  contact.dir_add(vm["src"].as<std::string>());
  contact.dir_scan();
  contact.image_load();
  image_t *image = contact.image_thumbnail(vm["dst"].as<std::string>());
  image->save();
}

