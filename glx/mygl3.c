#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include "mymath.h"

time_t start;
double theta;

GLuint fgProjectionMatrixIndex;
GLuint fgModelMatrixIndex;
GLuint fgColorIndex;
GLuint fgVertexIndex;
GLuint vertexBufferName;
GLfloat projectionMatrix[16];
mat4f projMatrix;

#define c1 1.0f, 0.0f, 0.0f
#define c2 0.0f, 0.0f, 1.0f
#define c3 0.0f, 1.0f, 0.0f
#define c4 1.0f, 1.0f, 0.0f

#define v1 -1.0f, -1.0f, 1.0f
#define v2  1.0f, -1.0f, 1.0f
#define v3  1.0f,  1.0f, 1.0f
#define v4 -1.0f,  1.0f, 1.0f

#define v5 -1.0f, -1.0f, -1.0f
#define v6  1.0f, -1.0f, -1.0f
#define v7  1.0f,  1.0f, -1.0f
#define v8 -1.0f,  1.0f, -1.0f

GLfloat varray[] = {
  c1, v1, c2, v2, c2, v3, c1, v4,
  c2, v3, c2, v2, c3, v6, c3, v7,
  c3, v6, c4, v5, c4, v8, c3, v7,
  c4, v8, c4, v5, c1, v1, c1, v4,
};

enum {
  numColorComponents = 3,
  numVertexComponents = 3,
  stride = sizeof(GLfloat) * (numColorComponents + numVertexComponents),
  numElements = sizeof(varray) / stride
};

const GLvoid *bufferObjectPtr (GLsizei index)
{
   return (const GLvoid *) (((char *) NULL) + index);
}

void compileAndCheck(GLuint shader)
{
   GLint status;
   glCompileShader (shader);
   glGetShaderiv (shader, GL_COMPILE_STATUS, &status);
   if (status == GL_FALSE) {
     GLint infoLogLength;
     GLchar *infoLog;
     glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &infoLogLength);
     infoLog = (GLchar*) malloc (infoLogLength);
     glGetShaderInfoLog (shader, infoLogLength, NULL, infoLog);
     fprintf (stderr, "compile log: %s\n", infoLog);
     free (infoLog);
   }
}

GLuint compileShaderSource(GLenum type, GLsizei count, const GLchar **string)
{
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, count, string, NULL);
  compileAndCheck(shader);
  return shader;
}

void linkAndCheck(GLuint program)
{
   GLint status;
   glLinkProgram (program);
   glGetProgramiv (program, GL_LINK_STATUS, &status);
   if (status == GL_FALSE) {
     GLint infoLogLength;
     GLchar *infoLog;
     glGetProgramiv (program, GL_INFO_LOG_LENGTH, &infoLogLength);
     infoLog = (GLchar*) malloc (infoLogLength);
     glGetProgramInfoLog (program, infoLogLength, NULL, infoLog);
     fprintf (stderr, "link log: %s\n", infoLog);
     free (infoLog);
   }
}

GLuint createProgram(GLuint vertexShader, GLuint fragmentShader)
{
   GLuint program = glCreateProgram ();
   if (vertexShader != 0) {
     glAttachShader (program, vertexShader);
   }
   if (fragmentShader != 0) {
      glAttachShader (program, fragmentShader);
   }
   linkAndCheck(program);
   return program;
}

void init()
 {
   start = time(0);
   theta = 0;

   GLenum err = glewInit();
   if (GLEW_OK != err)
     {
       /* Problem: glewInit failed, something is seriously wrong. */
       fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
       exit(1);
     }
   fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
   
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_MULTISAMPLE);
   //   glEnable(GL_CULL_FACE);

   glGenBuffers (1, &vertexBufferName);
   glBindBuffer (GL_ARRAY_BUFFER, vertexBufferName);
   glBufferData (GL_ARRAY_BUFFER, sizeof(varray), varray, GL_STATIC_DRAW);

   const GLchar *fragmentShaderSource[] = {
     "#version 140\n",
     "smooth in vec4 fg_SmoothColor;\n",
     "out vec4 fg_FragColor;\n",
     "void main(void)\n",
     "{\n",
     "   fg_FragColor = fg_SmoothColor;\n",
     "}\n"
   };
   const GLsizei fragmentShaderLines = sizeof(fragmentShaderSource) / sizeof(GLchar*);
   GLuint fragmentShader = compileShaderSource (GL_FRAGMENT_SHADER, fragmentShaderLines, fragmentShaderSource);

   const GLchar *vertexShaderSource[] = {
     "#version 140\n",
     "uniform mat4 fg_ProjectionMatrix;\n",
     "uniform mat4 fg_ModelMatrix;\n",
     "in vec4 fg_Color;\n",
     "in vec4 fg_Vertex;\n",
     "smooth out vec4 fg_SmoothColor;\n",
     "void main()\n",
     "{\n",
     "   fg_SmoothColor = fg_Color;\n",
     "   gl_Position = fg_ProjectionMatrix * fg_ModelMatrix * fg_Vertex;\n",
     "}\n"
   };
   
   const GLsizei vertexShaderLines = sizeof(vertexShaderSource) / sizeof(GLchar*);
   GLuint vertexShader = compileShaderSource (GL_VERTEX_SHADER, vertexShaderLines, vertexShaderSource);
   GLuint program = createProgram (vertexShader, fragmentShader);
   glUseProgram (program);

   fgProjectionMatrixIndex = glGetUniformLocation(program, "fg_ProjectionMatrix");
   fgModelMatrixIndex = glGetUniformLocation(program, "fg_ModelMatrix");

   fgColorIndex = glGetAttribLocation(program, "fg_Color");
   glEnableVertexAttribArray (fgColorIndex);
   fgVertexIndex = glGetAttribLocation(program, "fg_Vertex");
   glEnableVertexAttribArray (fgVertexIndex);

   glClearColor(0.0, 0.0, 0.0, 0.0);
 }

void reshape (int w, int h)
{
  glViewport (0, 0, (GLsizei) w, (GLsizei) h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  double s = 1.0;

  /*
  if (w <= h)
    gluOrtho2D(-s, +s, -s, s * (GLfloat) h/(GLfloat) w);
  else
    gluOrtho2D(-s, +s * (GLfloat) w/(GLfloat) h, -s, +s);
  */
  if (w <= h)
    {
      double r = (double)h / (double)w;
      glFrustum(-s, +s, -s*r, +s*r, 1.5, 10);
    }
  else
    {
      double r = (double)w / (double)h;
      glFrustum(-s*r, +s*r, -s, +s, 1.5, 10);
    }

  gluLookAt(0, 0, 4, 0, 0, 0, 0, 1, 0);
  // gluPerspective(60, 1, 1, 1);
  glGetFloatv(GL_PROJECTION_MATRIX, projMatrix.m);
  printf(" ----- Projection -----\n");
  dump4m(projMatrix);
}

/* ----------------------------------------------------------------------
--
-- display
--
---------------------------------------------------------------------- */

void display()
{
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  //  double theta = difftime(time(0), start);
  theta += 1;
  //  float phi = theta * M_PI / 180.0;
  //  rotatey(model, phi);
  //  translate(model, -5, -5, 0);

  mat4f model;

  //  glMatrixMode(GL_PROJECTION);
  //  glLoadIdentity();
  //  gluLookAt(3, 3, 3, 0, 0, 0, 0, 1, 0);
  //  glGetFloatv(GL_PROJECTION_MATRIX, projMatrix.m);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  //  glTranslatef(0.0, 0.0, -4.0);
  //  glTranslatef(2, 2, 0);
  glRotatef(theta, 0.5, 1.0, 0.0);
  glGetFloatv(GL_MODELVIEW_MATRIX, model.m);
  //  printf("------- %f -----------\n", (double)theta);
  //  dump4m(model);

  //  mat4f model = mul4f(rotatez(phi), rotatey(phi));
  //  mat4f model = ident4mf(); // mul4f(rotatey(phi), rotatez(phi));

  glUniformMatrix4fv(fgProjectionMatrixIndex, 1, GL_FALSE, projMatrix.m);
  glUniformMatrix4fv(fgModelMatrixIndex, 1, GL_FALSE, model.m);

  glBindBuffer (GL_ARRAY_BUFFER, vertexBufferName);
  glVertexAttribPointer(fgColorIndex, numColorComponents, GL_FLOAT, GL_FALSE, stride, bufferObjectPtr (0));
  glVertexAttribPointer(fgVertexIndex, numVertexComponents, GL_FLOAT, GL_FALSE, stride, bufferObjectPtr (sizeof(GLfloat) * numColorComponents));
  glDrawArrays(GL_QUADS, 0, numElements);
  glFlush ();
}

