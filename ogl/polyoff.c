#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>

#include "dl.h"

GLuint list;
GLint spinx = 0;
GLint spiny = 0;
GLfloat tdist = 0.0;
GLfloat polyfactor = 1.0;
GLfloat polyunits = 1.0;

/*  display() draws two spheres, one with a gray, diffuse material,
 *  the other sphere with a magenta material with a specular highlight.
 */
void display (void)
{
    GLfloat gray[] = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat black[] = { 0.0, 0.0, 0.0, 1.0 };

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix ();
    glTranslatef (0.0, 0.0, tdist);
    glRotatef ((GLfloat) spinx, 1.0, 0.0, 0.0);
    glRotatef ((GLfloat) spiny, 0.0, 1.0, 0.0);

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gray);
    glMaterialfv(GL_FRONT, GL_SPECULAR, black);
    glMaterialf(GL_FRONT, GL_SHININESS, 0.0);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(polyfactor, polyunits);
    glCallList (list);
    glDisable(GL_POLYGON_OFFSET_FILL);

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glColor3f (1.0, 1.0, 1.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glCallList (list);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glPopMatrix ();
    glFlush ();
}

/*  specify initial properties
 *  create display list with sphere  
 *  initialize lighting and depth buffer
 */
void init(void)
{
    GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };

    GLfloat global_ambient[] = { 0.2, 0.2, 0.2, 1.0 };

    glClearColor (0.0, 0.0, 0.0, 1.0);

    list = glGenLists(1);
    glNewList (list, GL_COMPILE);
       glutSolidSphere(1.0, 20, 12);
    glEndList ();

    glEnable(GL_DEPTH_TEST);

    glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv (GL_LIGHT0, GL_POSITION, light_position);
    glLightModelfv (GL_LIGHT_MODEL_AMBIENT, global_ambient);
}

/*  call when window is resized  */
void reshape(int width, int height)
{
    glViewport (0, 0, width, height);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluPerspective(45.0, (GLdouble)width/(GLdouble)height,
	    1.0, 10.0);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    gluLookAt (0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

/*  call when mouse button is pressed  */
/* ARGSUSED2 */
void mouse(int button, int state, int x, int y)
{
    switch (button) {
	case GLUT_LEFT_BUTTON:
	    switch (state) {
		case GLUT_DOWN:
		    spinx = (spinx + 5) % 360; 
                    glutPostRedisplay();
		    break;
		default:
		    break;
            }
            break;
	case GLUT_MIDDLE_BUTTON:
	    switch (state) {
		case GLUT_DOWN:
		    spiny = (spiny + 5) % 360; 
                    glutPostRedisplay();
		    break;
		default:
		    break;
            }
            break;
	case GLUT_RIGHT_BUTTON:
	    switch (state) {
		case GLUT_UP:
		    exit(0);
		    break;
		default:
		    break;
            }
            break;
        default:
            break;
    }
}

/* ARGSUSED1 */
void keyboard (unsigned char key, int x, int y)
{
   switch (key) {
      case 't':
         if (tdist < 4.0) {
            tdist = (tdist + 0.5);
            glutPostRedisplay();
         }
         break;
      case 'T':
         if (tdist > -5.0) {
            tdist = (tdist - 0.5);
            glutPostRedisplay();
         }
         break;
      case 'F':
         polyfactor = polyfactor + 0.1;
	 printf ("polyfactor is %f\n", polyfactor);
         glutPostRedisplay();
         break;
      case 'f':
         polyfactor = polyfactor - 0.1;
	 printf ("polyfactor is %f\n", polyfactor);
         glutPostRedisplay();
         break;
      case 'U':
         polyunits = polyunits + 1.0;
	 printf ("polyunits is %f\n", polyunits);
         glutPostRedisplay();
         break;
      case 'u':
         polyunits = polyunits - 1.0;
	 printf ("polyunits is %f\n", polyunits);
         glutPostRedisplay();
         break;
      default:
         break;
   }
}

struct id id = {.desc = "Color material"};
