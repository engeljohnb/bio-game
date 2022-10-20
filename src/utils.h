#ifndef __UTILS_H__
#define __UTILS_H__
#include <stdint.h>
int B_load_file(const char *filename, char *buff, int size);
int maxi(int a, int b);

/* Gets the distance between two substrings in a bigger string 
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
#endif
