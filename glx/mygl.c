#include <time.h>

#include <GL/gl.h>
#include <GL/glu.h>

time_t start;

void init()
{
  glClearColor(0,0,0,0);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  start = time(0);
}

/* ----------------------------------------------------------------------
--
-- reshape
--
---------------------------------------------------------------------- */

void reshape(int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   if (w <= h)
      gluOrtho2D (0.0, 30.0, 0.0, 30.0 * (GLfloat) h/(GLfloat) w);
   else
      gluOrtho2D (0.0, 30.0 * (GLfloat) w/(GLfloat) h, 0.0, 30.0);
   glMatrixMode(GL_MODELVIEW);
}

/* ----------------------------------------------------------------------
--
-- display
--
---------------------------------------------------------------------- */
void display()
{
  glLoadIdentity();
  glTranslatef(5, 5, 0);
  //  glRotatef(difftime(time(0), start), 0, 1.0f, 0.0f);
  //  glTranslatef(-5, -5, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glBegin (GL_TRIANGLES);
  glColor3f(1.0, 0.0, 0.0);
  glVertex2f(0.0, 0.0);
  glColor3f(0.0, 1.0, 0.0);
  glVertex2f(25.0, 0.0);
  glColor3f(0.0, 0.0, 1.0);
  glVertex2f(0.0, 25.0);
  glEnd();
}
