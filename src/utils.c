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
#include <string.h>
#include <cglm/cglm.h>
#include "utils.h"

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

int position_equal(float a[3], float b[3])
{
	return ((a[0] == b[0]) &&
		(a[1] == b[1]) &&
		(a[2] == b[2]));
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
	/* TODO: Write an uglier for loop than this */
	if (data_end == NULL)
	{
		fprintf(stderr, "get_data_after_punctuated error: data end key %s not found\n", end_key);
		data_end = data+data_length;
	}
	for (int i = 0; (start = memmem(data_iter, data_end-data_iter, search_key, strnlen(search_key, 128))) != NULL; ++i)
	{
		start += strnlen(search_key, 128);
		data_iter = start;
		num_elements++;
	}

	uint8_t **return_data = (uint8_t **)malloc(sizeof(uint8_t*)*num_elements);
	*element_sizes = malloc(sizeof(unsigned int)*num_elements);
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
		return_data[i] = (uint8_t *)malloc(length);
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
	/* TODO: Write an uglier for loop than this */
	for (int i = 0; (start = memmem(data_iter, data_end-data_iter, search_key, strnlen(search_key, 128))) != NULL; ++i)
	{
		start += strnlen(search_key, 128);
		data_iter = start;
		num_elements++;
	}

	uint8_t **return_data = (uint8_t **)malloc(sizeof(uint8_t*)*num_elements);
	*element_sizes = (unsigned int *)malloc(sizeof(unsigned int)*num_elements);
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
		return_data[i] = (uint8_t *)malloc(length);
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

int _bg_free(void *ptr, const char *filename, unsigned int line)
{
	if (valid(ptr))
	{
		free(ptr);
		ptr = NULL;
	}
	else
	{
		fprintf(stderr, "WARNING: Attempt to free invalid pointer in file %s at line %u.\n", filename, line);
		return -1;
	}
	return 0;
}


void print_vec3(vec3 vector)
{
	fprintf(stderr, "%f %f %f\n", vector[0], vector[1], vector[2]);
}
