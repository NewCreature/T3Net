#ifndef T3NET_INTERNAL_H
#define T3NET_INTERNAL_H

size_t t3net_internal_write_function(void * ptr, size_t size, size_t nmemb, void * stream);
int t3net_read_line(const char * data, char * output, int data_max, int output_max, unsigned int * text_pos);
int t3net_get_element(const char * data, T3NET_TEMP_ELEMENT * element, int data_max);

#endif
