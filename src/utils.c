/*
    Bio-Game is a game for designing your own organism. 
    Copyright (C) 2022 John Engel 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <cglm/cglm.h>
#include <limits.h>
#include "common.h"
#include "utils.h"

#define OUTSIDE 0
#define INSIDE 1

int even(int a)
{
	return (!(a % 2));
}

float lerp(float a, float b, float f)
{
    return a * (1.0 - f) + (b * f);
}

float signed_angle_vec3_xz(vec3 v0, vec3 v1)
{
	float x1 = v0[0];
	float y1 = v0[2];

	float x2 = v1[0];
	float y2 = v1[2];
	float angle = atan2(y1,x1) - atan2(y2,x2);
	if (absf(angle) > 180)
	{
	    angle = angle - 360*SIGN(angle);
	}
	return angle;
}
	
void _bilinearly_interpolate_vec3(vec3 q00,
				 vec3 q01,
				 vec3 q10,
				 vec3 q11,
				 vec3 pos,
				 vec3 dest)
{
	vec3 r0;
	vec3 r1;
	float scalar0 = (q10[0] - pos[0]) / (q10[0] - q00[0]);
	float scalar1 = (pos[0] - q00[0]) / (q10[0] - q00[0]);

	vec3 r0_part0;
	vec3 r0_part1;

	vec3 r1_part0;
	vec3 r1_part1;
	
	glm_vec3_scale(q00, scalar0, r0_part0);
	glm_vec3_scale(q10, scalar1, r0_part1);
	glm_vec3_scale(q01, scalar0, r1_part0);
	glm_vec3_scale(q11, scalar1, r1_part1);

	glm_vec3_add(r0_part0, r0_part1, r0);
	glm_vec3_add(r1_part0, r1_part1, r1);

	float r_scalar0 = (r1[2] - pos[2]) / (r1[2] - r0[2]);
	float r_scalar1 = (pos[2] - r0[2]) / (r1[2] - r0[2]);

	glm_vec3_scale(r0, r_scalar0, r0);
	glm_vec3_scale(r1, r_scalar1, r1);

	glm_vec3_add(r0, r1, dest);
}

void bilinearly_interpolate_vec3(
    const vec3 q11, const vec3 q12, 
    const vec3 q21, const vec3 q22, 
    const vec3 pos, 
    vec3 dest)
{
    // First interpolate in x direction
    vec3 tmp1, tmp2;
    for (int i = 0; i < 3; i++) {
        tmp1[i] = (1 - pos[0]) * q11[i] + pos[0] * q21[i];
        tmp2[i] = (1 - pos[0]) * q12[i] + pos[0] * q22[i];
    }

    // Then interpolate the results in y direction
    for (int i = 0; i < 3; i++) {
        dest[i] = (1 - pos[2]) * tmp1[i] + pos[2] * tmp2[i];
    }
}

float bilinearly_interpolate_float(float x0,
				   float x1,
				   float y0,
				   float y1,
				   float q00,
				   float q01,
				   float q10,
			           float q11,
			    	   float x,
				   float y)
{
	float scalar0x = (x1 - x) / (x1 - x0);
	float scalar1x = (x - x0) / (x1 - x0);

	float r0 = (q00 * scalar0x) + (q10 * scalar1x);
	float r1 = (q01 * scalar0x) + (q11 * scalar1x);

	float scalar0y = (y1 - y) / (y1 - y0);
	float scalar1y = (y - y0) / (y1 - y0);

	return (r0 * scalar0y) + (r1 * scalar1y);
}

float _bilinearly_interpolate_float(float q11, float q12, float q21, float q22, float x, float y) 
{
    // First interpolate in the x direction
    float tmp1 = (1 - x) * q11 + x * q21;
    float tmp2 = (1 - x) * q12 + x * q22;

    // Then interpolate the results in the y direction
    return (1 - y) * tmp1 + y * tmp2;
}


double bilinearly_interpolate_double(double x0,
				   double x1,
				   double y0,
				   double y1,
				   double q00,
				   double q01,
				   double q10,
			           double q11,
			    	   double x,
				   double y)
{
	double scalar0x = (x1 - x) / (x1 - x0);
	double scalar1x = (x - x0) / (x1 - x0);

	double r0 = (q00 * scalar0x) + (q10 * scalar1x);
	double r1 = (q01 * scalar0x) + (q11 * scalar1x);

	double scalar0y = (y1 - y) / (y1 - y0);
	double scalar1y = (y - y0) / (y1 - y0);

	return (r0 * scalar0y) + (r1 * scalar1y);
}
float percent(float min, float max, float x)
{
	return (x - min) / (max - min);
}

double percent_double(double min, double max, double x)
{
	return (x - min) / (max - min);
}


char *get_directory_name(const char *file)
{
	char *dir_name = BG_MALLOC(char, PATH_MAX);
	dir_name = realpath(file, dir_name);
	dir_name = dirname(dir_name);
	if (dir_name == NULL)
	{
		BG_FREE(dir_name);
		return NULL;
	}
	return dir_name;
}

int file_exists(const char *filename)
{
	return (access(filename, F_OK) == 0);
}

float absf(float value)
{
	if (value < 0)
	{
		return -value;
	}
	return value;
}
int maxi(int a, int b)
{
	if (a > b)
	{
		return a;
	}
	return b;
}

size_t mins(size_t a, size_t b)
{
	if (a < b)
	{
		return a;
	}
	return b;
}

int which_side(vec3 normal, vec3 point_on_plane, vec3 location)
{
	vec3 n_normal;
	glm_vec3_negate_to(normal, n_normal);
	float distance = glm_vec3_dot(n_normal, point_on_plane);

	return ((glm_vec3_dot(location, normal) + distance) > 0);
}

void cat_to(char *first, char *second, char *dest, size_t size)
{
	char first_copy[size];
	strncpy(first_copy, first, size);
	strncat(first_copy, second, size);
	strncpy(dest, first_copy, size);
}

void yaw(float angle, mat4 dest)
{
	mat4 destination;
	float yaw = RAD(angle);
	glm_euler(VEC3(0, yaw, 0), destination);
	if (dest != NULL)
	{
		glm_mat4_copy(destination, dest);
	}
}

void pitch(float angle, mat4 dest)
{
	mat4 destination;
	float pitch = RAD(angle);
	glm_euler(VEC3(pitch, 0, 0), destination);
	if (dest != NULL)
	{
		glm_mat4_copy(destination, dest);
	}
}

void get_rotation_matrix(float yaw, float pitch, mat4 dest)
{
	mat4 destination;

	float yaw_rad = RAD(yaw);
	float pitch_rad = RAD(pitch);

	glm_euler_yxz(VEC3(pitch_rad, yaw_rad, 0), destination);
	if (dest != NULL)
	{
		glm_mat4_copy(destination, dest);
	}
}

int is_behind_camera_2d(mat4 projection_view, vec2 pos)
{
	vec4 corners[8];
	mat4 inv_projection_view;
	glm_mat4_inv(projection_view, inv_projection_view);
	glm_frustum_corners(inv_projection_view, corners);

	vec3 plane_coords[4];
	vec3 position;
	glm_vec3_copy(VEC3(pos[0], 0, pos[1]), position);
	for (int i = 0; i < 4; ++i)
	{
		plane_coords[i][0] = corners[i][0];
		plane_coords[i][1] = 0;
		plane_coords[i][2] = corners[i][2];
	}

	vec3 b_minus_a;
	vec3 c_minus_a;
	vec3 normal;
	glm_vec3_sub(plane_coords[1], plane_coords[0], b_minus_a);
	glm_vec3_sub(plane_coords[2], plane_coords[0], c_minus_a);
	glm_vec3_cross(b_minus_a, c_minus_a, normal);

	float distance = glm_vec3_dot(position, normal);
	return (distance < 0);
}

void get_normal_vec3(vec3 a, vec3 b, vec3 c, vec3 dest)
{
	vec3 b_m_a;
	vec3 c_m_a;
	glm_vec3_sub(b, a, b_m_a);
	glm_vec3_sub(c, a, c_m_a);

	glm_vec3_cross(b_m_a, c_m_a, dest);
	glm_vec3_normalize(dest);
}

void get_frustum_corners(mat4 projection_view, vec3 dest[8])
{
	vec4 corners_vec4[8];
	mat4 inv_projection_view;
	glm_mat4_inv(projection_view, inv_projection_view);
	glm_frustum_corners(inv_projection_view, corners_vec4);

	for (int i = 0; i < 8; ++i)
	{
		glm_vec3(corners_vec4[i], dest[i]);
	}

}

void frustum_plane_index_to_string(int i, char dest[])
{
	switch (i)
	{
		case GLM_NEAR:
		{
			strncpy(dest, "NEAR", 128);
			return;
		}
		case GLM_FAR:
		{
			strncpy(dest, "FAR", 128);
			return;
		}
		case GLM_LEFT:
		{
			strncpy(dest, "LEFT", 128);
			return;
		}
		case GLM_TOP:
		{
			strncpy(dest, "TOP", 128);
			return;
		}
		case GLM_BOTTOM:
		{
			strncpy(dest, "BOTTOM", 128);
			return;
		}
		case GLM_RIGHT:
		{
			strncpy(dest, "RIGHT", 128);
			return;
		}
	}
}

void get_point_on_plane(vec4 plane, vec3 dest)
{
	/* Don't know why I need to negate the plane coefficient, 
	 * but as soon as I did, everything worked. */
	glm_vec3_scale(plane, -plane[3], dest);
}

float signed_distance_to_plane(vec3 p, vec4 plane)
{
	vec3 point_on_plane;
	get_point_on_plane(plane, point_on_plane);

	vec3 sub;
	glm_vec3_sub(p, point_on_plane, sub);

	return glm_vec3_dot(plane, sub);
}

int sphere_on_or_beyond_plane(vec3 center, float radius, vec4 plane)
{
	return signed_distance_to_plane(center, plane) > -radius;
}

int sphere_in_frustum(vec3 center, float radius, mat4 projection_view)
{
	vec4 frustum_planes[6];
	glm_frustum_planes(projection_view, frustum_planes);

	for (int i = 0; i < 6; ++i)
	{
		if (!sphere_on_or_beyond_plane(center, radius, frustum_planes[i]))
		{
			return 0;
		}
	}
	return 1;
}

float vec2_magnitude(vec2 vec)
{
	return sqrt(vec[0]*vec[0] + vec[1] * vec[1]);
}

int vec3_equal(float a[3], float b[3])
{
	return ((a[0] == b[0]) &&
		(a[1] == b[1]) &&
		(a[2] == b[2]));
}

int vec3_zero(float a[3])
{
	return ((a[0] == 0) &&
		(a[1] == 0) &&
		(a[2] == 0));
}

unsigned int length_between(char *haystack,
			    int len_haystack,
			    const char *needle1,
			    const char *needle2)
{
	char *substring1 = memmem(haystack, len_haystack, needle1, strlen(needle1));
	char *substring2 = memmem(haystack, len_haystack, needle2, strlen(needle2));
	char *start;
	char *end;

	if (&substring1 < &substring2)
	{
		start = substring1;
		start += strlen(needle1);
		end = substring2;
	}
	else
	{
		start = substring2;
		start += strlen(needle2);
		end = substring1;
	}

	return end - start;
}

// This is the ugliest  function name I've ever seen
uint8_t **get_data_after_punctuated(uint8_t *data, char *search_key, char *end_key, unsigned int data_length, unsigned int *number_of_elements, unsigned int **element_sizes)
{
	uint8_t *data_iter = data;
	uint8_t *data_end = memmem(data, data_length, end_key, strnlen(end_key, 128));
	uint8_t *start;
	uint8_t *end;
	int num_elements = 0;
	if (data_end == NULL)
	{
		fprintf(stderr, "get_data_after_punctuated error: data end key %s not found\n", end_key);
		data_end = data+data_length;
	}
	// This is the ugliest for loop I've ever seen
	for (int i = 0; (start = memmem(data_iter, data_end-data_iter, search_key, strnlen(search_key, 128))) != NULL; ++i)
	{
		start += strnlen(search_key, 128);
		data_iter = start;
		num_elements++;
	}

	uint8_t **return_data = BG_MALLOC(uint8_t*, num_elements);
	*element_sizes = BG_MALLOC(unsigned int, num_elements);
	data_iter = data;
	for (int i = 0; i < num_elements; ++i)
	{
 		start = memmem(data_iter, data_end-data_iter, search_key, strnlen(search_key, 128));
		if (start == NULL)
		{	
			break;
		}
		start += strnlen(search_key, 128);
		data_iter = start;

		end = memmem(data_iter, data_end-data_iter, search_key, strnlen(search_key, 128));
		if (end == NULL)
		{
			end = data_end;
		}
		int length = end - start;
		return_data[i] = BG_MALLOC(uint8_t, length);
		return_data[i] = memcpy(return_data[i], start, length);

		(*element_sizes)[i] = length;
	}

	*number_of_elements = num_elements;
	return return_data;
}
	
uint8_t **get_data_after(uint8_t *data, char *search_key, unsigned int data_length, unsigned int *number_of_elements, unsigned int **element_sizes)
{
	uint8_t *data_iter = data;
	uint8_t *data_end = data+data_length;
	uint8_t *start;
	uint8_t *end;
	int num_elements = 0;
	for (int i = 0; (start = memmem(data_iter, data_end-data_iter, search_key, strnlen(search_key, 128))) != NULL; ++i)
	{
		start += strnlen(search_key, 128);
		data_iter = start;
		num_elements++;
	}

	uint8_t **return_data = BG_MALLOC(uint8_t*, num_elements);
	*element_sizes = BG_MALLOC(unsigned int, num_elements);
	data_iter = data;
	for (int i = 0; i < num_elements; ++i)
	{
 		start = memmem(data_iter, data_end-data_iter, search_key, strnlen(search_key, 128));
		start += strnlen(search_key, 128);
		data_iter = start;

		end = memmem(data_iter, data_end-data_iter, search_key, strnlen(search_key, 128));
		if (end == NULL)
		{
			end = data_end;
		}
		int length = end - start;
		return_data[i] = BG_MALLOC(uint8_t, length);
		return_data[i] = memcpy(return_data[i], start, length);
		(*element_sizes)[i] = length;
		
	}

	*number_of_elements = num_elements;
	return return_data;
}

int B_load_file(const char *filename, char *buff, int size)
{

	FILE *fp = fopen(filename, "r");
	if (!fp)
	{
		fprintf(stderr, "Error: could not read file %s\n", filename);
		return -1;
	}
	fseek(fp, 0L, SEEK_END);
	int length = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	length++;
	int read_size = 0;
	if (length <= size)
	{
		read_size = length;
	}
	else
	{
		read_size = size;
	}
	memset(buff, 0, size);
	if ((fread(buff, read_size, 1, fp) == 0) && (ferror(fp)))
	{
		fclose(fp);
		fprintf(stderr, "Couldn't read file %s\n", filename);
		exit(-1);
	}

	fclose(fp);
	return 0;
}

int valid(void *ptr)
{
	return (ptr != NULL);
}


void *_bg_malloc(size_t size)
{
	void *ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
}

int _bg_free(void *ptr)
{
	if (valid(ptr))
	{
		free(ptr);
		ptr = NULL;
	}
	return 0;
}

void print_mat4(mat4 mat)
{
	fprintf(stdout, "%f\t", mat[0][0]);
	fprintf(stdout, "%f\t", mat[0][1]);
	fprintf(stdout, "%f\t", mat[0][2]);
	fprintf(stdout, "%f\t\n", mat[0][3]);

	fprintf(stdout, "%f\t", mat[1][0]);
	fprintf(stdout, "%f\t", mat[1][1]);
	fprintf(stdout, "%f\t", mat[1][2]);
	fprintf(stdout, "%f\t\n", mat[1][3]);

	fprintf(stdout, "%f\t", mat[2][0]);
	fprintf(stdout, "%f\t", mat[2][1]);
	fprintf(stdout, "%f\t", mat[2][2]);
	fprintf(stdout, "%f\t\n", mat[2][3]);

	fprintf(stdout, "%f\t", mat[3][0]);
	fprintf(stdout, "%f\t", mat[3][1]);
	fprintf(stdout, "%f\t", mat[3][2]);
	fprintf(stdout, "%f\t\n\n", mat[3][3]);
}

void print_vec4(vec4 vector)
{
	fprintf(stdout, "%f %f %f %f\n", vector[0], vector[1], vector[2], vector[3]);
}


void print_radius(float radius)
{
	fprintf(stdout, "%f\n", radius);
}
void print_center(vec3 vector)
{
	fprintf(stdout, "%f %f %f\n", vector[0], vector[1], vector[2]);
}

void print_vec3(vec3 vector)
{
	fprintf(stdout, "%f %f %f\n", vector[0], vector[1], vector[2]);
}
void print_plane(vec4 vector)
{
	fprintf(stdout, "%f %f %f %f\n", vector[0], vector[1], vector[2], vector[3]);
}

void print_vec3_indented(vec3 vector, int num_tabs)
{
	char string[512];
	memset(string, 0, 512);
	char *ptr = string;
	/* Takes about 40 characters to print a vec3 */
	if (num_tabs > (512 - 40))
	{
		fprintf(stderr, "print_vec3_tab error: too many tabs -- buffer overflow\n");
		exit(0);
	}
	for (int i = 0; i < num_tabs; ++i)
	{
		snprintf(ptr, 512-i, "\t");
		ptr++;
	}

	snprintf(ptr, 512-num_tabs, "%f %f %f\n", vector[0], vector[1], vector[2]);
	fwrite(string, 1, strnlen(string, 512), stdout);
}


void print_vec2(vec2 vector)
{
	fprintf(stdout, "%f %f\n", vector[0], vector[1]);
}
