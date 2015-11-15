#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t3net.h"
#include "server_list.h"
#include "internal.h"

static char t3net_server_key[1024] = {0};

T3NET_SERVER_LIST * t3net_get_server_list(char * url, char * game, char * version)
{
	T3NET_SERVER_LIST * lp;

	lp = malloc(sizeof(T3NET_SERVER_LIST));
	if(!lp)
	{
		return NULL;
	}
	lp->entries = 0;
	t3net_strcpy_url(lp->url, url, 1024);
	t3net_strcpy(lp->game, game, 64);
	t3net_strcpy(lp->version, version, 64);
	if(!t3net_update_server_list_2(lp))
	{
		free(lp);
		return NULL;
	}
	return lp;
}

int t3net_update_server_list_2(T3NET_SERVER_LIST * lp)
{
	char url_w_arg[1024] = {0};
	char * data = NULL;
	int ecount = -1;
	unsigned int text_pos;
	int text_max;
	char text[256];
	T3NET_TEMP_ELEMENT element;

	if(!lp)
	{
		return 0;
	}

	t3net_strcpy_url(url_w_arg, lp->url, 1024);
	t3net_strcat(url_w_arg, "?game=", 1024);
	t3net_strcat(url_w_arg, lp->game, 1024);
	t3net_strcat(url_w_arg, "&version=", 1024);
	t3net_strcat(url_w_arg, lp->version, 1024);
	data = t3net_get_data(url_w_arg, 65536);
	if(!data)
	{
		return 0;
	}

	/* check for error */
	if(!strncmp(data, "Error", 5))
	{
		free(data);
		return 0;
	}

	text_pos = 0;
    text_max = t3net_strlen(data);

    /* skip first two lines */
    t3net_read_line(data, text, text_max, 256, &text_pos);
    t3net_read_line(data, text, text_max, 256, &text_pos);
	while(ecount < T3NET_MAX_SERVERS)
	{
		if(t3net_read_line(data, text, text_max, 256, &text_pos))
		{
			t3net_get_element(text, &element, text_max);
			if(!strcmp(element.name, "name"))
			{
				ecount++;
				lp->entry[ecount] = malloc(sizeof(T3NET_SERVER_LIST_ENTRY));
				if(lp->entry[ecount])
				{
					memset(lp->entry[ecount], 0, sizeof(T3NET_SERVER_LIST_ENTRY));
					t3net_strcpy(lp->entry[ecount]->name, element.data, 256);
				}
				else
				{
					free(data);
					return 0;
				}
			}
			else if(!strcmp(element.name, "ip"))
			{
				t3net_strcpy(lp->entry[ecount]->address, element.data, 256);
			}
			else if(!strcmp(element.name, "port"))
			{
				lp->entry[ecount]->port = atoi(element.data);
			}
			else if(!strcmp(element.name, "capacity"))
			{
				t3net_strcpy(lp->entry[ecount]->capacity, element.data, 32);
			}
		}
		else
		{
			break;
		}

		/* get out if we've reached the end of the data */
		if(text_pos >= text_max)
		{
			break;
		}
	}
	ecount++;

	lp->entries = ecount;
	free(data);
	return 1;
}

void t3net_clear_server_list(T3NET_SERVER_LIST * lp)
{
	int i;

	for(i = 0; i < lp->entries; i++)
	{
		free(lp->entry[i]);
	}
	lp->entries = 0;
}

void t3net_destroy_server_list(T3NET_SERVER_LIST * lp)
{
	t3net_clear_server_list(lp);
	free(lp);
}

char * t3net_register_server(char * url, int port, char * game, char * version, char * name, char * password, int permanent)
{
	char * data = NULL;
	char url_w_arg[1024] = {0};
	char tname[256] = {0};
	char tport[64] = {0};
	int i;

	sprintf(tport, "%d", port);
	t3net_strcpy_url(tname, name, 256);
	t3net_strcpy_url(url_w_arg, url, 1024);
	t3net_strcat(url_w_arg, "?addServer&port=", 1024);
	t3net_strcat(url_w_arg, tport, 1024);
	t3net_strcat(url_w_arg, "&game=", 1024);
	t3net_strcat(url_w_arg, game, 1024);
	t3net_strcat(url_w_arg, "&version=", 1024);
	t3net_strcat(url_w_arg, version, 1024);
	t3net_strcat(url_w_arg, "&name=", 1024);
	t3net_strcat(url_w_arg, tname, 1024);
	t3net_strcat(url_w_arg, "&password=", 1024);
	t3net_strcat(url_w_arg, password ? "&password=" : "", 1024);
	t3net_strcat(url_w_arg, permanent ? "&no_poll=1" : "", 1024);

	data = t3net_get_data(url_w_arg, 65536);
	if(!data)
	{
		return NULL;
	}

	/* see if we got a key */
	t3net_server_key[0] = 0;
	if(!strncmp(data, "key=", 4))
	{
		for(i = 4; i < t3net_strlen(data); i++)
		{
			t3net_strset(t3net_server_key, i - 4, data[i], 1024);
		}
		t3net_strset(t3net_server_key, i - 4, '\0', 1024);
	}
	free(data);
    return t3net_server_key;
}

int t3net_update_server(char * url, int port, char * key, char * capacity)
{
	char * data = NULL;
	char url_w_arg[1024] = {0};
	int ret = 0;
	char tcap[256] = {0};
	char tport[256] = {0};


	sprintf(tport, "%d", port);
	t3net_strcpy_url(tcap, capacity, 256);
	t3net_strcpy_url(url_w_arg, url, 1024);
	t3net_strcat(url_w_arg, "?pollServer&port=", 1024);
	t3net_strcat(url_w_arg, tport, 1024);
	t3net_strcat(url_w_arg, "&key=", 1024);
	t3net_strcat(url_w_arg, key, 1024);
	t3net_strcat(url_w_arg, "&capacity=", 1024);
	t3net_strcat(url_w_arg, tcap, 1024);
	data = t3net_get_data(url_w_arg, 65536);
	if(data[0] == 'a' && data[1] == 'c' && data[2] == 'k')
	{
		ret = 1;
	}
	else
	{
		ret = -1;
	}
	t3net_strcpy(t3net_server_message, data, 1024);
    free(data);
    return ret;
}

int t3net_unregister_server(char * url, int port, char * key)
{
	char * data = NULL;
	char url_w_arg[1024] = {0};
	char tport[256] = {0};

	sprintf(tport, "%d", port);
	t3net_strcpy(url_w_arg, url, 1024);
	t3net_strcat(url_w_arg, "?removeServer&port=", 1024);
	t3net_strcat(url_w_arg, tport, 1024);
	t3net_strcat(url_w_arg, "&key=", 1024);
	t3net_strcat(url_w_arg, key, 1024);
	data = t3net_get_data(url_w_arg, 1024);
	if(!data)
	{
		return 0;
	}
	free(data);
    return 1;
}
