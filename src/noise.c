#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cglm/cglm.h>
#include "utils.h"
#include "noise.h"

#define NOISE fbm
#define NUM_NOISE_OCTAVES 5

float hash_float(float p) 
{
	p = fmod(p * 0.011, 1); 
	p *= p + 7.5; 
	p *= p + p; 
	return fmod(p, 1); 
}

float hash_vec2(vec2 p) 
{
	//vec3 p3 = fract(vec3(p.xyx) * 0.13); 
	vec3 p3;
	p3[0] = fmod(p[0]*0.13, 1);
	p3[1] = fmod(p[1]*0.13, 1);
	p3[2] = fmod(p[0]*0.13, 1);
	//p3 += dot(p3, p3.yzx + 3.333); 
	vec3 p3_yzx;
	p3_yzx[0] = p3[1] + 3.333;
	p3_yzx[1] = p3[2] + 3.333;
	p3_yzx[2] = p3[0] + 3.333;
	float dot = glm_vec3_dot(p3, p3_yzx);
	vec3 sum;
	glm_vec3_adds(p3, dot, sum);
	return fmod((sum[0] + sum[1]) * sum[2], 1); 
}

float mix_float(float x, float y, float a)
{
	return x*(1-a) + y*a;
}

float noise(vec2 x) 
{
    	vec2 i;
        i[0] = floor(x[0]);
        i[1] = floor(x[1]);
    	vec2 f;
        f[0] = fmod(x[0], 1);
        f[1] = fmod(x[1], 1);

	// Four corners in 2D of a tile
	float a = hash_vec2(i);
	float b = hash_vec2(VEC2(i[0] + 1.0, i[0]));
    	float c = hash_vec2(VEC2(i[0], i[1] + 1.0));
    	float d = hash_vec2(VEC2(i[0] + 1.0, i[1] + 1.0));

    	//vec2 u = f * f * (3.0 - 2.0 * f);
	vec2 u;
	glm_vec2_copy(f, u);
	glm_vec2_mul(u, f, u);
	glm_vec2_scale(f, 2.0, f);
	glm_vec2_subs(f, 3.0, f);
	glm_vec2_mul(u, f, u);

	return mix_float(a, b, u[0]) + (c - a) * u[1] * (1.0 - u[0]) + (d - b) * u[0] * u[1];
}


float fbm(vec2 x) 
{
	float v = 0.0;
	float a = 0.5;
	vec2 shift;
	glm_vec2_copy(VEC2(100, 100), shift);
	// Rotate to reduce axial bias
	
    //	mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));
    	mat2 rot = {{ cos(0.5), sin(0.5) },
		    {-sin(0.5), cos(0.5) }};
	for (int i = 0; i < NUM_NOISE_OCTAVES; ++i) 
	{
		v += a * noise(x);
		//x = rot * x * 2.0 + shift;
		glm_mat2_mulv(rot, x, x);
		glm_vec2_scale(x, 2.0, x);
		glm_vec2_add(x, shift, x);
		a *= 0.5;
	}
	return v;
}

