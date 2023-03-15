/*
    Bio-Game is a game for designing your own microorganism. 
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
#include "utils.h"

char *get_directory_name(const char *file)
{
	char *dir_name = BG_MALLOC(char, 512);
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

int is_in_frustum_2d(mat4 projection_view, vec2 pos)
{
	vec4 corners[8];
	mat4 inv_projection_view;
	glm_mat4_inv(projection_view, inv_projection_view);
	glm_frustum_corners(inv_projection_view, corners);

	float ftlx = corners[GLM_LTF][0];
	float ftrx = corners[GLM_RTF][1];
	float ftrz = corners[GLM_RTF][2];
	float ntrz = corners[GLM_RTN][2];
	return !((pos[0] > ftlx-500) &&
		(pos[0] < ftrx+500) &&
		(pos[1] < ftrz+500) &&
		(pos[1] > ntrz-500));
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
	fread(buff, read_size, 1, fp);

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
	fprintf(stderr, "%f\t", mat[0][0]);
	fprintf(stderr, "%f\t", mat[0][1]);
	fprintf(stderr, "%f\t", mat[0][2]);
	fprintf(stderr, "%f\t\n", mat[0][3]);

	fprintf(stderr, "%f\t", mat[1][0]);
	fprintf(stderr, "%f\t", mat[1][1]);
	fprintf(stderr, "%f\t", mat[1][2]);
	fprintf(stderr, "%f\t\n", mat[1][3]);

	fprintf(stderr, "%f\t", mat[2][0]);
	fprintf(stderr, "%f\t", mat[2][1]);
	fprintf(stderr, "%f\t", mat[2][2]);
	fprintf(stderr, "%f\t\n", mat[2][3]);

	fprintf(stderr, "%f\t", mat[3][0]);
	fprintf(stderr, "%f\t", mat[3][1]);
	fprintf(stderr, "%f\t", mat[3][2]);
	fprintf(stderr, "%f\t\n\n", mat[3][3]);
}

void print_vec4(vec4 vector)
{
	fprintf(stderr, "%f %f %f %f\n", vector[0], vector[1], vector[2], vector[3]);
}

void print_vec3(vec3 vector)
{
	fprintf(stderr, "%f %f %f\n", vector[0], vector[1], vector[2]);
}

void print_vec2(vec2 vector)
{
	fprintf(stderr, "%f %f\n", vector[0], vector[1]);
}
