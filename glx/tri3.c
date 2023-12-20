#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "mymath.h"

#define c1 1.0f, 0.0f, 0.0f
#define c2 0.0f, 0.0f, 1.0f
#define c3 0.0f, 1.0f, 0.0f
#define c4 1.0f, 1.0f, 0.0f

#define v1 0.0f, 0.0f, 0.0f
#define v2 30.0f, 0.0f, 0.0f
#define v3 0.0f, 30.0f, 0.0f

GLfloat varray[] = {
  c1,  v1,  c2,  v2,  c3,  v3,
};

GLuint vertexBufferName;
GLuint fgProjectionMatrixIndex;
GLuint fgModelMatrixIndex;
GLuint fgColorIndex;
GLuint fgVertexIndex;
mat4f projMatrix;

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
   glCompileShader(shader);
   glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
   if (status == GL_FALSE)
     {
       GLint infoLogLength;
       GLchar *infoLog;
       glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
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

/* ----------------------------------------------------------------------
--
-- init
--
---------------------------------------------------------------------- */

void init()
{
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

   /* --------------------
      Generate a single buffer
      -------------------- */

   glGenBuffers(1, &vertexBufferName);

   /* --------------------
      Bind type to buffer
      -------------------- */

   glBindBuffer(GL_ARRAY_BUFFER, vertexBufferName);

   /* --------------------
      Specify buffer is STATIC (modify once) and DRAW (used for drawing in GL).
      -------------------- */

   glBufferData(GL_ARRAY_BUFFER, sizeof(varray), varray, GL_STATIC_DRAW);

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
   GLuint fragmentShader = compileShaderSource(GL_FRAGMENT_SHADER, fragmentShaderLines, fragmentShaderSource);

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
   GLuint vertexShader = compileShaderSource(GL_VERTEX_SHADER, vertexShaderLines, vertexShaderSource);
   GLuint program = createProgram (vertexShader, fragmentShader);
   glUseProgram (program);

   /* --------------------
      Link shader variables
      -------------------- */

   fgProjectionMatrixIndex = glGetUniformLocation(program, "fg_ProjectionMatrix");
   fgModelMatrixIndex = glGetUniformLocation(program, "fg_ModelMatrix");

   fgColorIndex = glGetAttribLocation(program, "fg_Color");
   glEnableVertexAttribArray(fgColorIndex);
   fgVertexIndex = glGetAttribLocation(program, "fg_Vertex");
   glEnableVertexAttribArray(fgVertexIndex);

   glClearColor(0.0, 0.0, 0.0, 0.0);
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
   glGetFloatv(GL_PROJECTION_MATRIX, projMatrix.m);
   //   dump4m(projMatrix);
   glMatrixMode(GL_MODELVIEW);
}

/* ----------------------------------------------------------------------
--
-- display
--
---------------------------------------------------------------------- */

void display()
{
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  /* --------------------
     Start with identity matrix
     -------------------- */

  mat4f model = ident4mf();
  glUniformMatrix4fv(fgModelMatrixIndex, 1, GL_FALSE, model.m);

  /* --------------------
     Take projection from reshape
     -------------------- */

  glUniformMatrix4fv(fgProjectionMatrixIndex, 1, GL_FALSE, projMatrix.m);

  /* --------------------
     Array is colour, vector, colour, vector ... so vector is offset by 3 floats
     -------------------- */

  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferName);
  glVertexAttribPointer(fgColorIndex, numColorComponents, GL_FLOAT, GL_FALSE, stride, bufferObjectPtr (0));
  glVertexAttribPointer(fgVertexIndex, numVertexComponents, GL_FLOAT, GL_FALSE, stride, bufferObjectPtr (sizeof(GLfloat) * numColorComponents));
  glDrawArrays(GL_TRIANGLES, 0, numElements);
  glFlush ();
}
