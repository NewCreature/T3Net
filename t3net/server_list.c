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
	T3NET_DATA * data;
	int ecount = -1;
	unsigned int text_pos;
	int text_max;
	char text[256];
	const char * val;
	int i;

	if(!lp)
	{
		return 0;
	}

	t3net_strcpy_url(url_w_arg, lp->url, 1024);
	t3net_strcat(url_w_arg, "?game=", 1024);
	t3net_strcat(url_w_arg, lp->game, 1024);
	t3net_strcat(url_w_arg, "&version=", 1024);
	t3net_strcat(url_w_arg, lp->version, 1024);
	data = t3net_get_data(url_w_arg);
	if(!data)
	{
		return 0;
	}

	for(i = 0; i < data->entries; i++)
	{
		val = t3net_get_data_entry_field(data, i, "name");
		if(val)
		{
			t3net_strcpy(lp->entry[i]->name, val, 256);
		}
		val = t3net_get_data_entry_field(data, i, "ip");
		if(val)
		{
			t3net_strcpy(lp->entry[i]->address, val, 256);
		}
		val = t3net_get_data_entry_field(data, i, "port");
		if(val)
		{
			lp->entry[i]->port = atoi(val);
		}
		val = t3net_get_data_entry_field(data, i, "capacity");
		if(val)
		{
			t3net_strcpy(lp->entry[i]->capacity, val, 256);
		}
		val = t3net_get_data_entry_field(data, i, "private");
		if(val)
		{
			lp->entry[i]->private = 0;
			if(!strcmp(val, "true"))
			{
				lp->entry[i]->private = 1;
			}
		}
	}

	lp->entries = data->entries;
	t3net_destroy_data(data);
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
	T3NET_DATA * data;
	char url_w_arg[1024] = {0};
	char tname[256] = {0};
	char tport[64] = {0};
	const char * val;
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

	data = t3net_get_data(url_w_arg);
	if(!data)
	{
		return NULL;
	}

	/* see if we got a key */
	t3net_server_key[0] = 0;
	if(data->entries > 0)
	{
		val = t3net_get_data_entry_field(data, 0, "key");
		if(val)
		{
			t3net_strcpy(t3net_server_key, val, 1024);
		}
	}
	t3net_destroy_data(data);
    return t3net_server_key;
}

int t3net_update_server(char * url, int port, char * key, char * capacity)
{
	T3NET_DATA * data = NULL;
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
	data = t3net_get_data(url_w_arg);
	if(!data)
	{
		return -1;
	}
    t3net_destroy_data(data);
    return 1;
}

int t3net_unregister_server(char * url, int port, char * key)
{
	T3NET_DATA * data = NULL;
	char url_w_arg[1024] = {0};
	char tport[256] = {0};

	sprintf(tport, "%d", port);
	t3net_strcpy(url_w_arg, url, 1024);
	t3net_strcat(url_w_arg, "?removeServer&port=", 1024);
	t3net_strcat(url_w_arg, tport, 1024);
	t3net_strcat(url_w_arg, "&key=", 1024);
	t3net_strcat(url_w_arg, key, 1024);
	data = t3net_get_data(url_w_arg);
	if(!data)
	{
		return 0;
	}
	t3net_destroy_data(data);
    return 1;
}
