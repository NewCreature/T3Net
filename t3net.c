#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "t3net.h"
#include "internal.h"

char t3net_server_message[1024] = {0};

static int t3net_stdio_strget_proc(const char * s, int point)
{
	return s[point];
}

static int t3net_stdio_strset_proc(char * dest, int point, int val, int size)
{
	if(point < size)
	{
		dest[point] = val;
	}
	return 1;
}

static int (*t3net_strget_proc)(const char * s, int point) = t3net_stdio_strget_proc;
static int (*t3net_strset_proc)(char * s, int point, int val, int size) = t3net_stdio_strset_proc;

void t3net_set_str_functions(int (*strget_proc)(const char * s, int point), int (*strset_proc)(char * s, int point, int val, int size))
{
	if(t3net_strget_proc)
	{
		t3net_strget_proc = strget_proc;
	}
	if(t3net_strset_proc)
	{
		t3net_strset_proc = strset_proc;
	}
}

int t3net_strget(const char * s, int point)
{
	return t3net_strget_proc(s, point);
}

int t3net_strset(char * s, int point, int val, int size)
{
	return t3net_strset_proc(s, point, val, size);
}

int t3net_strlen(const char * s)
{
	int c = 1;
	int pos = 0;

	while(c != '\0')
	{
		c = t3net_strget(s, pos);
		pos++;
	}
	return pos;
}

void t3net_strcpy(char * dest, const char * src, int size)
{
	int c = 1;
	int pos = 0;

	while(c != '\0')
	{
		c = t3net_strget(src, pos);
		t3net_strset(dest, pos, c, size);
		pos++;
	}
}

void t3net_strcpy_url(char * dest, const char * src, int size)
{
	int i;
	int write_pos = 0;
	int c = 1;
	int pos = 0;

	while(c != '\0')
	{
		c = t3net_strget(src, pos);
		if(c == ' ')
		{
			t3net_strset(dest, write_pos, '%', size);
			write_pos++;
			t3net_strset(dest, write_pos, '2', size);
			write_pos++;
			t3net_strset(dest, write_pos, '0', size);
			write_pos++;
		}
		else
		{
			t3net_strset(dest, pos, c, size);
			write_pos++;
		}
		pos++;
	}
}

char * t3net_strcat(char * dest, const char * src, int size)
{
	int c = 1;
	int write_pos = 0;
	int i;

	/* find end of destination string */
	while(c != '\0')
	{
		c = t3net_strget(dest, write_pos);
		write_pos++;
	}

	/* copy src */
	for(i = 0; write_pos < size - 1 && i < t3net_strlen(src); i++)
	{
		c = t3net_strget(src, i);
		t3net_strset(dest, write_pos, c, size);
		write_pos++;
	}
	t3net_strset(dest, write_pos, '\0', size);

	return dest;
}

int t3net_written = 0;
size_t t3net_internal_write_function(void * ptr, size_t size, size_t nmemb, void * stream)
{
	char * str = (char *)stream;
	if(str)
	{
		memcpy(&str[t3net_written], ptr, size * nmemb);
	}
	t3net_written += size * nmemb;
	return size * nmemb;
}

int t3net_read_line(const char * data, char * output, int data_max, int output_max, unsigned int * text_pos)
{
	int outpos = 0;
	int c;

	while(1)
	{
		c = t3net_strget(data, *text_pos);
		if(c != '\n')
		{
			t3net_strset(output, outpos, c, output_max);
		}
		else
		{
			t3net_strset(output, outpos, '\0', output_max);
			(*text_pos)++;
			return 1;
		}
		outpos++;
		if(outpos >= output_max - 1)
		{
			t3net_strset(output, outpos, '\0', output_max);
			return 1;
		}
		(*text_pos)++;
		if(*text_pos >= data_max)
		{
			return 0;
		}
	}
	return 0;
}

int t3net_get_element(const char * data, T3NET_TEMP_ELEMENT * element, int data_max)
{
	int outpos = 0;
	int c;
	int read_pos = 1; // skip first byte

	/* read element name */
	while(1)
	{
		c = t3net_strget(data, read_pos);

		if(c == ':')
		{
			read_pos += 2;
			break;
		}
		else
		{
			t3net_strset(element->name, outpos, c, 256);
			outpos++;
			t3net_strset(element->name, outpos, '\0', 256);
			read_pos++;
		}
	}

	/* read element data */
	outpos = 0;
	while(c != '\0' && read_pos < data_max)
	{
		c = t3net_strget(data, read_pos);

		t3net_strset(element->data, outpos, c, 256);
		outpos++;
		t3net_strset(element->data, outpos, '\0', 256);
		read_pos++;
	}
	return 1;
}

char * t3net_get_data(const char * url, int data_size)
{
	CURL * curl;
	char * data = NULL;

	data = malloc(data_size);
	if(!data)
	{
		return NULL;
	}
	memset(data, 0, data_size);

	/* make HTTP request */
	curl = curl_easy_init();
	if(!curl)
	{
		free(data);
		return NULL;
	}
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, t3net_internal_write_function);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, T3NET_TIMEOUT_TIME);
	t3net_written = 0;
    if(curl_easy_perform(curl))
    {
		curl_easy_cleanup(curl);
		free(data);
		return NULL;
	}
    curl_easy_cleanup(curl);
	if(t3net_written >= data_size)
	{
		t3net_written = data_size - 1;
	}
	data[t3net_written] = 0;

    return data;
}
