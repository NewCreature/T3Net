#include "t3net.h"
#include "server_list.h"
#include "internal.h"

T3NET_SERVER_LIST * t3net_get_server_list(char * url, char * game, char * version)
{
	T3NET_SERVER_LIST * lp;

	lp = malloc(sizeof(T3NET_SERVER_LIST));
	if(!lp)
	{
		return NULL;
	}
	lp->entries = 0;
	t3net_strcpy_url(lp->url, url);
	t3net_strcpy(lp->game, game);
	t3net_strcpy(lp->version, version);
	if(!t3net_update_server_list_2(lp))
	{
		free(lp);
		return NULL;
	}
	return lp;
}

int t3net_update_server_list_2(T3NET_SERVER_LIST * lp)
{
	CURL * curl;
	char url_w_arg[1024] = {0};
	char * data = NULL;
	int ecount = 0;
	unsigned int text_pos;
	int text_max;
	char text[256];
	T3NET_TEMP_ELEMENT element;

	if(!lp)
	{
		return 0;
	}

	data = malloc(65536);
	if(!data)
	{
		return 0;
	}

	/* make HTTP request */
	curl = curl_easy_init();
	if(!curl)
	{
		free(data);
		return 0;
	}
	sprintf(url_w_arg, "%s?game=%s&version=%s", lp->url, lp->game, lp->version);
	curl_easy_setopt(curl, CURLOPT_URL, url_w_arg);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, t3net_internal_write_function);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, T3NET_TIMEOUT_TIME);
	t3net_written = 0;
	if(curl_easy_perform(curl))
	{
		curl_easy_cleanup(curl);
		free(data);
		return 0;
	}
    curl_easy_cleanup(curl);
	data[t3net_written] = 0;

	/* check for error */
	if(!strcmp(data, "Error"))
	{
		return 0;
	}

	text_pos = 0;
    text_max = t3net_strlen(data);

    /* skip first two lines */
    t3net_read_line(data, text, text_max, 256, &text_pos);
    t3net_read_line(data, text, text_max, 256, &text_pos);
	while(ecount < T3NET_MAX_SERVERS)
	{
		lp->entry[ecount] = malloc(sizeof(T3NET_SERVER_LIST_ENTRY));
		if(lp->entry[ecount])
		{
			memset(lp->entry[ecount], 0, sizeof(T3NET_SERVER_LIST_ENTRY));
			if(t3net_read_line(data, text, text_max, 256, &text_pos))
			{
				t3net_get_element(text, &element, text_max);
				if(!strcmp(element.name, "name"))
				{
					t3net_strcpy(lp->entry[ecount]->name, element.data);
				}
			}
			else
			{
				break;
			}

			if(t3net_read_line(data, text, text_max, 256, &text_pos))
			{
				t3net_get_element(text, &element, text_max);
				if(!strcmp(element.name, "ip"))
				{
					t3net_strcpy(lp->entry[ecount]->address, element.data);
				}
			}
			else
			{
				break;
			}

			if(t3net_read_line(data, text, text_max, 256, &text_pos))
			{
				t3net_get_element(text, &element, text_max);
				if(!strcmp(element.name, "port"))
				{
					lp->entry[ecount]->port = atoi(element.data);
				}
			}
			else
			{
				break;
			}

			if(!t3net_read_line(data, text, text_max, 256, &text_pos))
			{
				break;
			} // game
			if(!t3net_read_line(data, text, text_max, 256, &text_pos))
			{
				break;
			} // gametype
			if(!t3net_read_line(data, text, text_max, 256, &text_pos))
			{
				break;
			}; // mod

			if(t3net_read_line(data, text, text_max, 256, &text_pos))
			{
				t3net_get_element(text, &element, text_max);
				if(!strcmp(element.name, "capacity"))
				{
					t3net_strcpy(lp->entry[ecount]->capacity, element.data);
				}
			}
			else
			{
				break;
			}

			if(!t3net_read_line(data, text, text_max, 256, &text_pos))
			{
				break;
			} // tags
			if(!t3net_read_line(data, text, text_max, 256, &text_pos))
			{
				break;
			} // map
			if(!t3net_read_line(data, text, text_max, 256, &text_pos))
			{
				break;
			} // state
			if(!t3net_read_line(data, text, text_max, 256, &text_pos))
			{
				break;
			} // private

			ecount++;
		}

		/* get out if we've reached the end of the data */
		if(text_pos >= text_max)
		{
			break;
		}
	}
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
	CURL * curl;
	char * data = NULL;
	char url_w_arg[1024] = {0};
	char tname[256] = {0};
	int i;

	data = malloc(65536);
	if(!data)
	{
		return NULL;
	}

	/* make HTTP request */
	curl = curl_easy_init();
	if(!curl)
	{
		free(data);
		return NULL;
	}
	t3net_strcpy_url(tname, name);
	sprintf(url_w_arg, "%s?addServer&port=%d&game=%s&version=%s&name=%s%s%s%s", url, port, game, version, tname, password ? "&password=" : "", password ? password : "", permanent ? "&no_poll=1" : "");
	curl_easy_setopt(curl, CURLOPT_URL, url_w_arg);
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
	data[t3net_written] = 0;

	/* see if we got a key */
	t3net_server_key[0] = 0;
	if(!strncmp(data, "key=", 4))
	{
		for(i = 4; i < strlen(data); i++)
		{
			t3net_server_key[i - 4] = data[i];
		}
		t3net_server_key[i - 4] = '\0';
	}
	free(data);
    return t3net_server_key;
}

int t3net_update_server(char * url, int port, char * key, char * capacity)
{
	CURL * curl;
	char * data = NULL;
	char url_w_arg[1024] = {0};
	int ret = 0;
	char tcap[256] = {0};

	data = malloc(65536);
	if(!data)
	{
		return 0;
	}

	/* make HTTP request */
	curl = curl_easy_init();
	if(!curl)
	{
		free(data);
		return 0;
	}
	t3net_strcpy_url(tcap, capacity);
	sprintf(url_w_arg, "%s?pollServer&port=%d&key=%s&capacity=%s", url, port, key, tcap);
	curl_easy_setopt(curl, CURLOPT_URL, url_w_arg);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, t3net_internal_write_function);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, T3NET_TIMEOUT_TIME);
	t3net_written = 0;
    if(curl_easy_perform(curl))
    {
		curl_easy_cleanup(curl);
		free(data);
		return 0;
	}
	if(data[0] == 'a' && data[1] == 'c' && data[2] == 'k')
	{
		ret = 1;
	}
	else
	{
		ret = -1;
	}
	snprintf(t3net_server_message, 1024, "%s", data);
    curl_easy_cleanup(curl);
    free(data);
    return ret;
}

int t3net_unregister_server(char * url, int port, char * key)
{
	CURL * curl;
//	char * data = NULL;
	char url_w_arg[1024] = {0};

//	data = malloc(65536);
//	if(!data)
//	{
//		return 0;
//	}

	/* make HTTP request */
	curl = curl_easy_init();
	if(!curl)
	{
		return 0;
	}
	sprintf(url_w_arg, "%s?removeServer&port=%d&key=%s", url, port, key);
	curl_easy_setopt(curl, CURLOPT_URL, url_w_arg);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, t3net_internal_write_function);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, T3NET_TIMEOUT_TIME);
	t3net_written = 0;
    if(curl_easy_perform(curl))
    {
		curl_easy_cleanup(curl);
		return 0;
	}
    curl_easy_cleanup(curl);
    return 1;
}
