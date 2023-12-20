typedef struct {
  float m[16];
} mat4f;

typedef struct {
  float v[4];
} vec4f;

mat4f myFrustum(float l, float r, float b, float t, float n, float f);
mat4f myOrtho(float l, float r, float b, float t, float n, float f);
mat4f myLookAt(float eye_x, float eye_y, float eye_z, float at_x, float at_y, float at_z, float up_x, float up_y, float up_z);
void dump4m(mat4f m);
void dump4v(vec4f m);
mat4f mul4mf(mat4f m, mat4f n);
vec4f mul4vf(mat4f, vec4f);
mat4f ident4mf(void);
