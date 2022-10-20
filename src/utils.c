#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memmem.h>
#include "utils.h"

int maxi(int a, int b)
{
	if (a > b)
	{
		return a;
	}
	return b;
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
	uint8_t *data_end = memmem(data, data_length, end_key, strlen(end_key));
	uint8_t *start;
	uint8_t *end;
	int num_elements = 0;
	/* TODO: Write an uglier for loop than this */
	if (data_end == NULL)
	{
		fprintf(stderr, "get_data_after_punctuated error: data end key %s not found\n", end_key);
	}
	data_end = data+data_length;
	for (int i = 0; (start = memmem(data_iter, data_end-data_iter, search_key, strlen(search_key))) != NULL; ++i)
	{
		start += strlen(search_key);
		data_iter = start;
		num_elements++;
	}

	uint8_t **return_data = malloc(sizeof(uint8_t*)*num_elements);
	*element_sizes = malloc(sizeof(unsigned int)*num_elements);
	data_iter = data;
	for (int i = 0; i < num_elements; ++i)
	{
 		start = memmem(data_iter, data_end-data_iter, search_key, strlen(search_key));
		start += strlen(search_key);
		data_iter = start;

		end = memmem(data_iter, data_end-data_iter, search_key, strlen(search_key));
		if (end == NULL)
		{
			end = data_end;
		}
		int length = end - start;
		return_data[i] = malloc(length);
		return_data[i] = memcpy(return_data[i], start, length);

		(*element_sizes)[i] = length;
	}

	*number_of_elements = num_elements;
	return return_data;
}
	
uint8_t **get_data_after(uint8_t *data, char *search_key, unsigned int data_length, unsigned int *number_of_elements, unsigned int *element_sizes)
{
	uint8_t *data_iter = data;
	uint8_t *data_end = data+data_length;
	uint8_t *start;
	uint8_t *end;
	int num_elements = 0;
	/* TODO: Write an uglier for loop than this */
	for (int i = 0; (start = memmem(data_iter, data_end-data_iter, search_key, strlen(search_key))) != NULL; ++i)
	{
		start += strlen(search_key);
		data_iter = start;
		num_elements++;
	}

	uint8_t **return_data = malloc(sizeof(uint8_t*)*num_elements);
	element_sizes = malloc(sizeof(unsigned int)*num_elements);
	data_iter = data;
	for (int i = 0; i < num_elements; ++i)
	{
 		start = memmem(data_iter, data_end-data_iter, search_key, strlen(search_key));
		start += strlen(search_key);
		data_iter = start;

		end = memmem(data_iter, data_end-data_iter, search_key, strlen(search_key));
		if (end == NULL)
		{
			end = data_end;
		}
		int length = end - start;
		return_data[i] = malloc(length);
		return_data[i] = memcpy(return_data[i], start, length);
		element_sizes[i] = length;
		
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


