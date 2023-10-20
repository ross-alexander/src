#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libcroco/libcroco.h>

class css_t {
public:
  css_t() {};
};

static void start_document_cb (CRDocHandler *a_handler)
{
  fprintf(stdout, "Here\n");
  a_handler->app_data = new css_t;
}

static void start_selector_cb (CRDocHandler *a_handler, CRSelector *a_selector)
{
  css_t *context = (css_t*)a_handler->app_data;
  if (!context)
    return ;
  cr_selector_dump (a_selector, stdout) ;
  printf (" {\n") ; 
}

static void property_cb (CRDocHandler *a_handler, CRString *a_name, CRTerm *a_value, gboolean a_important)
{
  css_t *context = (css_t*)a_handler->app_data ;

  if (!context || !a_name)
    return ;

  printf ("%s : ", cr_string_peek_raw_str (a_name)) ;
  cr_term_dump (a_value, stdout) ;
  printf ("\n") ;
}

void display_usage (unsigned char *a_prog_name)
{
  unsigned char *prog_name = a_prog_name;

  if (!prog_name)
    {
      prog_name = (unsigned char*) "sac-example-1" ;
    }
  printf ("usage: %s [--help] | <css file name>\n", prog_name) ;	
}

int main (int argc, char **argv)
{	
  unsigned short i = 0 ;
  unsigned char * file_path = NULL ;
  CRParser * parser = NULL ;
  CRDocHandler *sac_handler = NULL ;
  
  if (argc <= 1)
    {
      display_usage ((unsigned char*)argv[0]) ;
      return -1 ;
    }
  for (i=1 ; i < argc ;i++)
    {
      if (*argv[i] != '-')
	break ;
      
      if (!strcmp (argv[i], "--help")
	  || !strcmp (argv[i], "-h"))
	{
	  display_usage ((unsigned char*)argv[0]) ;
	  return -1;
	}
      else
	{
	}
    }
  if (i > argc)
    {
      return -1;
    }
  
  file_path = (unsigned char*)argv[i];
  parser = cr_parser_new_from_file(file_path, CR_ASCII);
  if (!parser)
    {
      return -1 ;
    }

  sac_handler = cr_doc_handler_new();
  if (!sac_handler)
    {
      cr_parser_destroy (parser);
      return -1 ;
    }

  sac_handler->start_document = start_document_cb;
  sac_handler->start_selector = start_selector_cb;
  sac_handler->property = property_cb ;

  cr_parser_set_sac_handler(parser, sac_handler) ;

  cr_parser_parse(parser);
  cr_parser_destroy(parser);
  return 0;
}
