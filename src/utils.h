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

#ifndef __UTILS_H__
#define __UTILS_H__
#include <cglm/cglm.h>
#include <glad/glad.h>
#include <stdint.h>
#include <memmem.h>

#define VEC2(x, y) (vec2){x, y}
#define VEC3(x, y, z) (vec3){x, y, z}
#define VEC4(x, y, z, w) (vec4){x, y, z, w}
#define RAD(a) a*0.0174532925
#define VEC3_ZERO (vec3){0.0, 0.0, 0.0}
#define VEC3_X_UP (vec3){1.0, 0.0, 0.0}
#define VEC3_Y_UP (vec3){0.0, 1.0, 0.0}
#define VEC3_Z_UP (vec3){0.0, 0.0, 1.0}
#define VEC3_X_DOWN (vec3){-1.0, 0.0, 0.0}
#define VEC3_Y_DOWN (vec3){0.0, -1.0, 0.0}
#define VEC3_Z_DOWN (vec3){0.0, 0.0, -1.0}
#define BG_FREE(ptr) _bg_free(ptr)
#define BG_MALLOC(type, count) (type *)_bg_malloc(sizeof(type)*count)

int B_load_file(const char *filename, char *buff, int size);
int maxi(int a, int b);
size_t mins(size_t a, size_t b);

/* Gets the distance between two substrings in a bigger string.
   Specifically, the distance between the START of the first string and the END of the second.
   length_between("FOO1234567890BAR", "FOO", "BAR") -> 10 
   Length of haystack is needed so I can use it to search binary data as well as text. */
unsigned int length_between(char *haystack,
			    int len_haystack,
			    const char *needle1,
			    const char *needle2);

/* Parses binary or string data and returns all data contained after instances of the search key. 
 *  So if the search key is "FOO" ... 
 *  "FOO 1234 4321 FOO ABCD EFGH FOO" --> { " 1234 4321 ", " ABCD EFGH ", "" } 
 *  Number of elements of the returned array will be read into number_of_elements 
 *
 * !! Caller is responsible for freeing the returned pointer and each of its elements !! 
 * !! Element sizes does not need to be allocated before calling. Caller is also responsible for freeing element_sizes !! */
uint8_t **get_data_after(uint8_t *data, char *search_key, unsigned int data_length, unsigned int *number_of_elements, unsigned int **element_sizes);

/* Very similar to get_data_after, but with an "end key" -- Data past the earliest instance of the end key won't be searched or returned
 * So if the search key is "FOO" and the end key is "END_FOO" ... 
 * "FOO 1234 FOO 5678 FOO ABCD END_FOO bar bar 420" --> { " 1234 ", " 5678 ", " ABCD " }
 * Number of elements in the returned array will be read into number_of_elements.
 * Apologies for the esoteric name. 
 *
 * !! Caller is responsible for freeing the returned pointer and each of its elements !! 
 * !! Element sizes does not need to be allocated before calling. Caller is also responsible for freeing element_sizes !! */
uint8_t **get_data_after_punctuated(uint8_t *data, char *search_key, char *end_key, unsigned int data_length, unsigned int *number_of_elements, unsigned int **element_sizes);

int vec3_equal(float a[3], float b[3]);
int vec3_zero(float a[3]);
int valid(void *ptr);
void pitch(float angle, mat4 dest);
void yaw(float angle, mat4 dest);
void get_rotation_matrix(float yaw, float pitch,  mat4 dest);

/* This is called by the macro BG_FREE. The code has changed in a way where there's no longer any reason for it to be a macro,
 * I'm just too lazy to go back and change each call.
 *
 * Besides, having BG_MALLOC and BG_FREE both be macros has a nice symmetry to it. */
int _bg_free(void *ptr);
float absf(float value);
float vec2_magnitude(vec2 vec);
void print_vec3(vec3 vector);
void print_vec2(vec2 vector);
void print_mat4(mat4 mat);
/* Appends second to first and stores the resulting string in dest. Size is the size of dest. */
void cat_to(char *first, char *second, char *dest, size_t size);
void *_bg_malloc(size_t size);
int file_exists(const char *filename);
char *get_directory_name(const char *file);
#endif
