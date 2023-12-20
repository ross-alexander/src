#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <math.h>
#include <gts.h>

struct params {
  double alpha, beta, gamma;
};

double d = 0.4;
double light_switch = 1.0;
double eye_x = 0.0;
double eye_y = 0.0;
double eye_z = 5.0;

int size = 2;

struct params p;


void key(unsigned char k, int x, int y)
{
  switch (k) {
  case 27:  /* Escape */
    exit(0);
    break;
  case 'a':
    p.gamma *= 2.0;

    //    eye_z += 0.1;
    //    d += 0.1;
    break;
  case 's':
    p.gamma *= 0.5;
    //    eye_z -= 0.1;
    //    d -= 0.1;
    break;

  case 'i':
    p.alpha += 10.0;
    //    eye_y += 0.1;
    break;
  case 'j':
    p.alpha -= 10.0;
    //   eye_y -= 0.1;
    break;

  case 'k':
    p.beta += 10.0;
    //    eye_x -= 0.1;
    break;
  case 'l':
    p.beta -= 10.0;
    //    eye_x += 0.1;
    break;

  case 'z':
    light_switch = 1.0 - light_switch;
    break;
  default:
    printf("Unknown key %d\n", k);
    return;
  }
  glutPostRedisplay();
}

gint draw_triangle(GtsFace *f, void *d)
{
  //  double nx, ny, nz;
  double nt[3];
  GtsVertex *v[3];
  GtsTriangle *t = &f->triangle;

  gts_triangle_normal(t, &nt[0], &nt[1], &nt[2]);
  gts_triangle_vertices(t, &v[0], &v[1], &v[2]);

  //  gts_triangle_normal(t, &nx, &ny, &nz);
  //  glNormal3f(nx, ny, nz);

  /* --------------------
     Iterate over each vertex
     -------------------- */

  //  glNormal3f(0.0, 0.0, 1.0);

  //  printf("--------------------\n");

  for (int i = 0; i < 3; i++)
    {
      double nf[3] = {0.0, 0.0, 0.0};
      GSList *lt, *l;

      printf("%d : ", i);

      for (l = lt = gts_vertex_triangles(v[i], NULL); lt; lt = lt->next)
	{
	  double ntmp[3];
	  gts_triangle_normal((GtsTriangle*)lt->data, &ntmp[0], &ntmp[1], &ntmp[2]);
	  nf[0] += ntmp[0];
	  nf[1] += ntmp[1];
	  nf[2] += ntmp[2];
	  printf("x");
	}
      printf("\n");

      g_slist_free(l);

      double n = sqrt(nf[0] * nf[0] + nf[1] * nf[1] + nf[2] * nf[2]);

      //      if (i == 0)
      //	glNormal3f(0, 0, 1.0);
      //      else
	glNormal3f(nf[0]/n, nf[1]/n, nf[2]/n);

      //      printf("%d: %4.2f %4.2f %4.2f\n", i, v[i]->p.x, v[i]->p.y, v[i]->p.z);

      glVertex3f(v[i]->p.x, v[i]->p.y, v[i]->p.z);
    }
  return 1;
}

/* ----------------------------------------------------------------------
--
-- display
--
---------------------------------------------------------------------- */

void display()
{
  glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glLoadIdentity ();             // clear the matrix 
  // viewing transformation 

/* --------------------
   eye-x, eye-y, eye-z, lookat-x, lookat-y, lookat-z, up-x, up-y, up-z
   -------------------- */

//  gluLookAt (eye_x, eye_y, eye_z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

  gluLookAt (0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

  GLfloat light[] = { 0.0, 0.0, 8.0, light_switch };

  glLightfv (GL_LIGHT0, GL_POSITION, light);
  
  glRotatef(p.alpha, 0.0, 1.0, 0.0); // rotate around y-axis
  glRotatef(p.beta, 0.0, 0.0, 1.0); // rotate around z-axis
  glScalef(p.gamma, p.gamma, p.gamma);

  // Only show front of polygon

  glPolygonMode(GL_FRONT, GL_FILL);
  glPolygonMode(GL_BACK, GL_NONE);

  for (int x = -size; x <= size; x++)
    for (int y = -size; y <= size; y++)
      {
	glPushMatrix();
	glBegin(GL_TRIANGLES);
	
	if (x%2)
	  {
	    if (y%2)
	      glColor3f(0.0, 0.0, 0.5);
	    else
	      glColor3f(0.5, 0.5, 0.0);
	  }
	else
	  glColor3f(0.0, 0.5, 0.0);

	GtsSurface *s = gts_surface_new(gts_surface_class(),
					gts_face_class(),
					gts_edge_class(),
					gts_vertex_class());
	
	GtsVertex* v[7];
	double r = 1.0;
	double rs = sqrt(3) / 3 * r;
	double top = d;
	
	double cx = sqrt(3) * x;
	double cy = 2.0 * y + abs(x%2);
	
	v[0] = gts_vertex_new(gts_vertex_class(), 1 * -rs + cx,  -r + cy, 0.0);
	v[1] = gts_vertex_new(gts_vertex_class(), 1 * +rs + cx,  -r + cy, 0.0);
	v[2] = gts_vertex_new(gts_vertex_class(), 2 * +rs + cx, 0.0 + cy, 0.0);
	v[3] = gts_vertex_new(gts_vertex_class(), 1 * +rs + cx,  +r + cy, 0.0);
	v[4] = gts_vertex_new(gts_vertex_class(), 1 * -rs + cx,  +r + cy, 0.0);
	v[5] = gts_vertex_new(gts_vertex_class(), 2 * -rs + cx, 0.0 + cy, 0.0);
	v[6] = gts_vertex_new(gts_vertex_class(), cx, cy, top);
	
       	for (int i = 0; i < 6; i++)
	  {
	    double nx, ny, nz;
	    int j = (i+1)%6;
	    
	    GtsEdge *e1 = gts_edge_new(gts_edge_class(), v[6], v[i]);
	    GtsEdge *e2 = gts_edge_new(gts_edge_class(), v[i], v[j]);
	    GtsEdge *e3 = gts_edge_new(gts_edge_class(), v[j], v[6]);
	    GtsTriangle *t = gts_triangle_new(gts_triangle_class(), e1, e2, e3);
	    gts_triangle_normal(t, &nx, &ny, &nz);
	    gts_surface_add_face(s, (GtsFace*)t);
	  }
	gts_surface_foreach_face(s, (GtsFunc)draw_triangle, NULL);
	glEnd();
	glPopMatrix();
	gts_object_destroy((GtsObject*)s);
      }

  glColor3f (1.0, 0.0, 0.0);
  //  glutSolidSphere (1, 50, 50);
  glColor3f (0.0, 1.0, 0.0);

  glFrontFace(GL_CW);
  //  glutSolidTeapot(1.0);
  glFrontFace(GL_CCW);

  glFlush ();
}

void reshape(int w, int h)
{
  glViewport (0, 0, w, h);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

/* --------------------
   left, right, bottom, top, near, far
   -------------------- */

//  glFrustum (-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);

  gluPerspective(60.0, 1.0, 1.5, 20.0);

  glMatrixMode (GL_MODELVIEW);
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char* argv[])
{
  p.alpha = p.beta = 0.0;
  p.gamma = 1.0;

  glutInit(&argc, argv);

  int ch;
  while ((ch = getopt(argc, argv, "s:")) != EOF)
    {
      switch(ch)
	{
	case 's':
	  size = strtol(optarg, NULL, 10);
	  break;
	}
    }

  glutInitDisplayMode (GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
  glutInitWindowSize (500, 500);
  glutCreateWindow ("honeycomb");
  
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_SMOOTH);
  glFrontFace(GL_CCW);
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);
  
    //    GLfloat mat_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
/*   mat_specular and mat_shininess are NOT default values	*/
//    GLfloat mat_diffuse[] = { 0.4, 0.4, 0.4, 1.0 };
//    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
//    GLfloat mat_shininess[] = { 15.0 };

//    GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
//    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
//    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
//    GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1.0 };


  const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
  const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
  const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
  const GLfloat high_shininess[] = { 100.0f };
  
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

  const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
  const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
  const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  //  const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

  //  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
  
    glEnable(GL_LIGHT0);
  
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  
  glutKeyboardFunc(key);
  
  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}
