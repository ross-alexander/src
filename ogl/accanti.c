#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "jitter.h"
#include "dl.h"

/*  Initialize lighting and other values.
 */
void init(void)
{
  GLfloat mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat light_position[] = { 0.0, 0.0, 10.0, 1.0 };
  GLfloat lm_ambient[] = { 0.2, 0.2, 0.2, 1.0 };

  printf("init()\n");
  
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 50.0);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lm_ambient);
  
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glShadeModel (GL_FLAT);
  
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClearAccum(0.0, 0.0, 0.0, 0.0);
}

void displayObjects(void) 
{
    GLfloat torus_diffuse[] = { 0.7, 0.7, 0.0, 1.0 };
    GLfloat cube_diffuse[] = { 0.0, 0.7, 0.7, 1.0 };
    GLfloat sphere_diffuse[] = { 0.7, 0.0, 0.7, 1.0 };
    GLfloat octa_diffuse[] = { 0.7, 0.4, 0.4, 1.0 };


    glPushMatrix ();
    glRotatef (30.0, 1.0, 0.0, 0.0);

    glPushMatrix ();
    glTranslatef (-0.80, 0.35, 0.0); 
    glRotatef (100.0, 1.0, 0.0, 0.0);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, torus_diffuse);
    glutSolidTorus (0.275, 0.85, 16, 16);
    printf("displayObjects()\n");
    glPopMatrix ();

    glPushMatrix ();
    glTranslatef (-0.75, -0.50, 0.0); 
    glRotatef (45.0, 0.0, 0.0, 1.0);
    glRotatef (45.0, 1.0, 0.0, 0.0);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, cube_diffuse);
    glutSolidCube (1.5);
    glPopMatrix ();

    glPushMatrix ();
    glTranslatef (0.75, 0.60, 0.0); 
    glRotatef (30.0, 1.0, 0.0, 0.0);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, sphere_diffuse);
    glutSolidSphere (1.0, 16, 16);
    glPopMatrix ();

    glPushMatrix ();
    glTranslatef (0.70, -0.90, 0.25); 
    glMaterialfv(GL_FRONT, GL_DIFFUSE, octa_diffuse);
    glutSolidOctahedron ();
    glPopMatrix ();

    glPopMatrix ();
}

#define ACSIZE	8

void display(void)
{
  GLint viewport[4];
  int jitter;

  printf("display()\n");
  
  glGetIntegerv (GL_VIEWPORT, viewport);

  glClear(GL_ACCUM_BUFFER_BIT);
    for (jitter = 0; jitter < ACSIZE; jitter++) {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glPushMatrix ();
      /*	Note that 4.5 is the distance in world space between
       *	left and right and bottom and top.
       *	This formula converts fractional pixel movement to 
       *	world coordinates.
 */
      glTranslatef (j8[jitter].x*4.5/viewport[2],
		    j8[jitter].y*4.5/viewport[3], 0.0);
      displayObjects ();
      glPopMatrix ();
      glAccum(GL_ACCUM, 1.0/ACSIZE);
    }
    glAccum (GL_RETURN, 1.0);
    glFlush();
}

void reshape(int w, int h)
{
  printf("reshape()\n");
  
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (w <= h) 
    glOrtho (-2.25, 2.25, -2.25*h/w, 2.25*h/w, -10.0, 10.0);
  else 
    glOrtho (-2.25*w/h, 2.25*w/h, -2.25, 2.25, -10.0, 10.0);
  glMatrixMode(GL_MODELVIEW);
}

struct id id = {.desc = "Accanti"};
