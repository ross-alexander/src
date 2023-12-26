#include <math.h>
#include <stdio.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "dl.h"

struct id id = {.desc = "New Cube"};

struct {
  double x;
  double y;
  double z;
} rotation = {0.0, 0.0, 0.0 };


void init()
{
  GLfloat light_diffuse_0[] = {1.0, 1.0, 1.0, 1.0};  /* Blue diffuse light RGBA. */
  GLfloat light_position_0[] = {0.0, 0.0, 5.0, 1.0};  /* Infinite light location. */

  GLfloat light_diffuse_1[] = {1.0, 1.0, 1.0, 1.0};  /* Red diffuse light. */
  GLfloat light_position_1[] = {0.0, 5.0, 0.0, 1.0};  /* Infinite light location. */

  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat diffuseMaterial[4] = { 0.5, 0.5, 0.5, 1.0 };

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_SMOOTH);
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_LIGHTING);
  glEnable(GL_CULL_FACE);

  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse_0);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position_0);
  glEnable(GL_LIGHT0);

  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse_1);
  glLightfv(GL_LIGHT1, GL_POSITION, light_position_1);
  //  glEnable(GL_LIGHT1);

  glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMaterial);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 25.0);

   glColorMaterial(GL_FRONT, GL_DIFFUSE);
   glEnable(GL_COLOR_MATERIAL);
}

void reshape(int w, int h)
{
  glViewport(0, 0, (GLsizei)w, (GLsizei)h); 
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
  glMatrixMode(GL_MODELVIEW);
}

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  /* viewing transformation  */
  /* eye - lookat - up */

  gluLookAt(0.0, 3.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

  glRotatef(rotation.y, 0.0, 1.0, 0.0);
  glRotatef(rotation.x, 1.0, 0.0, 0.0);
  glRotatef(rotation.z, 0.0, 0.0, 1.0);

  glPushMatrix();

  int parts = 32;

  for (int i = 0; i < parts; i+=2)
    {
      double a0 = (i-1) * (2*M_PI / parts);
      double x0 = sin(a0);
      double z0 = cos(a0);

      double a1 = (i+1) * (2*M_PI / parts);
      double x1 = sin(a1);
      double z1 = cos(a1);

      double a2 = (i+0) * (2*M_PI / parts);
      double x2 = sin(a2);
      double z2 = cos(a2);

      double w = 0.2;

      glColor3f(1.0, 0.0, 0.0);
      glBegin(GL_QUADS);
      //      glNormal3f(x2, 0.0, z2);
      glNormal3f(x0, 0, z0);

      glVertex3f(x0, +w, z0);
      glVertex3f(x0, -w, z0);

      glNormal3f(x1, 0, z1);

      glVertex3f(x1, -w, z1);
      glVertex3f(x1, +w, z1);
      glEnd();

      //      w += 0.1;

      if (i%4)
	glColor3f(0.0, 0.0, 1.0);
      else
	glColor3f(0.0, 1.0, 0.0);

      glBegin(GL_QUADS);
      glNormal3f(-x1, 0.0, -z1);
      glVertex3f(x1 * 1.0, +w, z1 * 1.0);
      glNormal3f(-x1, 0.0, -z1);
      glVertex3f(x1 * 1.0, -w, z1 * 1.0);
      glNormal3f(-x0, 0.0, -z0);
      glVertex3f(x0 * 1.0, -w, z0 * 1.0);
      glNormal3f(-x0, 0.0, -z0);
      glVertex3f(x0 * 1.0, +w, z0 * 1.0);
      glEnd();
    }
  glPopMatrix();

  //  glCullFace(GL_BACK);

  glFlush();
}

void mouse(int button, int state, int x, int y)
{
  printf("Button %d\n", button);
  switch (button)
    {
    case 1:
      rotation.y = fmod(rotation.y + 10.0, 360.0);
      break;
    case 2:
      rotation.z = fmod(rotation.z + 10.0, 360.0);
      break;
    case 3:
      rotation.x = fmod(rotation.x + 10.0, 360.0);
      break;
    }
  display();
}

void keyboard(int k)
{
  printf("Keyboard %d\n",k);
}
