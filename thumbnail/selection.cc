#include <libcroco/libcroco.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

class workspace
{
public:
  xmlDoc *document;
  xmlXPathContext *xpath; 
  xmlXPathObject *result; 
  CRStyleSheet *stylesheet;
  CRCascade *cascade;
  CRSelEng *selector;
  enum CRStatus status;

  workspace(char **args);
  ~workspace();
  void print_properties();
};

void usage_and_exit (char *progname)
{
  fprintf (stderr, "Usage: %s <xml doc> <stylesheet> <xpath>\n", progname);
  exit(-1);
}


/* ----------------------------------------------------------------------
--
-- init
--
---------------------------------------------------------------------- */

workspace::workspace(char **args)
{
  document = 0;
  xpath = 0;
  result = 0;
  stylesheet = 0;
  cascade = 0;
  selector = 0;
  status = CR_ERROR;
  
  char* xml_document_path = args[0];
  char *css_sheet_path = args[1];
  char* xpath_expression = args[2];
  
  if (!(document = xmlParseFile(xml_document_path)))
    {
      fprintf(stderr, "could not parse the document %s", xml_document_path);
      return;
    }
  if (!(xpath = xmlXPathNewContext(document)))
    {
      fprintf(stderr, "Error: unable to create new XPath context\n");
      return;
    }
  if (!(result = xmlXPathEvalExpression((xmlChar*)xpath_expression, xpath)))
    {
      fprintf(stderr, "Error: unable to evaluate xpath expression\n");
      return;
    }
  if (result->type != XPATH_NODESET || !result->nodesetval)
    {
      fprintf(stderr, "Error: xpath does not evaluate to a node set\n");
      return;
    }
  
  status = cr_om_parser_simply_parse_file((const guchar*)css_sheet_path, CR_ASCII, &stylesheet);
  if (status != CR_OK || !stylesheet)
    {
      fprintf(stderr, "could not parse the stylesheet %s", css_sheet_path);
      return;
    }

  /* --------------------
     A cascade takes three sheetws, author, user & ua and combines them
     -------------------- */

  cascade = cr_cascade_new(stylesheet, 0, 0);
  selector = cr_sel_eng_new();
  status = CR_OK;
}

workspace::~workspace()
{
  if (selector) 
    {
      cr_sel_eng_destroy(selector);
      selector = NULL ;
    }
  if (cascade) 
    {
      cr_cascade_destroy(cascade);
      cascade = NULL ;
    }
  
  if (result) 
    {	  
      xmlXPathFreeObject(result);
      result = NULL ;
    }
  if (xpath)
    {
      xmlXPathFreeContext(xpath); 
      xpath = NULL ;
    }
  if (document) 
    {
      xmlFreeDoc(document);
      document = NULL ;
    }
  xmlCleanupParser ();
}

void  print_properties_real (CRPropList *proplist)
{
  for (CRPropList *cur_pair = proplist ; cur_pair; cur_pair= cr_prop_list_get_next (cur_pair))
    {
      gchar *str = NULL ;
      CRDeclaration *decl = NULL ;
    
      cr_prop_list_get_decl(cur_pair, &decl);
      if (decl)
	{
	  str = cr_declaration_to_string (decl, 0) ;
	  if (str)
	    {
	      printf ("%s\n", str) ;
	      g_free (str);
	      str = NULL ;
	    }
	}
    }
}

/* ----------------------------------------------------------------------
--
-- print_properties
--
---------------------------------------------------------------------- */

void workspace::print_properties()
{
  enum CRStatus status;
  CRPropList *prop_list = NULL;
  xmlNode *node = result->nodesetval->nodeTab[0];
  
  status = cr_sel_eng_get_matched_properties_from_cascade(selector, cascade, node, &prop_list);
  
  if (status != CR_OK)
    fprintf(stderr, "Error retrieving properties\n");
  else
    {
      xmlChar *prop = xmlGetNodePath(node) ;
      if (prop)
	{
	  printf("properties for node %s :\n", prop);
	  xmlFree (prop) ;
	  prop = NULL ;
	}
      print_properties_real (prop_list) ;
    }
  cr_prop_list_destroy (prop_list) ;
}

int main(int argc, char **argv)
{
  if (argc != 4) usage_and_exit(argv[0]);
  workspace ws(argv + 1);
  if (ws.status != CR_OK)
    return -1 ;
  
  if (ws.result->nodesetval->nodeNr == 0)
    printf("no matching nodes found\n");
  else
    ws.print_properties();

  return 0;
}
