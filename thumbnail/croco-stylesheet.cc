#include <iostream>

#include <assert.h>
#include <libcroco/libcroco.h>
#include <libxml++/libxml++.h>

void  print_properties_real(CRPropList *proplist)
{
  for (CRPropList *cur_pair = proplist ; cur_pair; cur_pair= cr_prop_list_get_next (cur_pair))
    {
      CRDeclaration *decl = NULL ;
      cr_prop_list_get_decl(cur_pair, &decl);
      if (decl)
	{
	  printf("%s\n", cr_string_peek_raw_str(decl->property));
	  for (CRTerm* term = decl->value ; term != 0; term = term->next)
	    {
	      if (guchar *str = cr_term_one_to_string(term))
		{
		  printf ("++ %s\n", str) ;
		  g_free (str);
		}
	    }
	}
    }
}

int main()
{
  CRStyleSheet *stylesheet;
  CRCascade *cascade;
  CRSelEng *selector;
  enum CRStatus status;
  
  const char *css_sheet_path = "sheet.css";


  //  xmlpp::DomParser parser("sheet.xml");
  //  xmlpp::Element *xn_swamp = parser.get_document()->get_root_node();

  const char *xml_ns = "";
  xmlpp::Document doc("1.0");
  xmlpp::Element *xn_swamp = doc.create_root_node("sheet", "", xml_ns);
  //  doc.write_to_file_formatted("sheet.xml");

  xmlpp::Node::NodeSet set = xn_swamp->find("//sheet");

  status = cr_om_parser_simply_parse_file((const guchar*)css_sheet_path, CR_ASCII, &stylesheet);
  assert (status == CR_OK);
  cascade = cr_cascade_new(stylesheet, 0, 0);
  selector = cr_sel_eng_new();

  assert(cascade);
  assert(selector);
  
  
  if (status != CR_OK)
    {
      fprintf(stderr, "Error retrieving properties\n");
      return -1;
    }
  std::cout << "Set size = " << set.size() << "\n";
  
  for (unsigned int i = 0; i < set.size(); i++)
    {
      xmlpp::Node *node = dynamic_cast<xmlpp::Node*>(set[i]);

      xmlNode* node_cptr = node->cobj();
      assert(node_cptr);

      CRPropList *prop_list = NULL;
      status = cr_sel_eng_get_matched_properties_from_cascade(selector, cascade, node_cptr, &prop_list);
      if ((status == CR_OK) && prop_list)
	{
	  Glib::ustring prop = node->get_path();
	  std::cout << prop << "\n";
	  print_properties_real (prop_list) ;
	  cr_prop_list_destroy (prop_list) ;
	}
    }
}
