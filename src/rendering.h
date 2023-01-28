#ifndef __RENDER_H__
#define __RENDER_H__

#include <cglm/cglm.h>
#include <glad/glad.h>

typedef unsigned int B_Shader;
int B_check_shader(unsigned int id, const char *name, int status);
void B_set_uniform_float(B_Shader shader, char *name, float value);
void B_set_uniform_vec3(B_Shader shader, char *name, vec3 value);
void B_set_uniform_vec4(B_Shader shader, char *name, vec4 value);
void B_set_uniform_mat4(B_Shader shader, char *name, mat4 value);
void B_set_uniform_int(B_Shader shader, char *name, int value);

#endif


