#include <memory.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "t3net.h"
#include "dlc.h"
#include "internal.h"

T3NET_DLC_LIST * t3net_get_dlc_list(const char * url, const char * game, int type)
{
	T3NET_DLC_LIST * lp = NULL;
	char url_w_arg[1024] = {0};
	T3NET_DATA * data = NULL;
	int loop_out = 0;
	int ecount = 0;
	unsigned int text_pos;
	int text_char;
	int text_fill_pos;
	char buf[256] = {0};
	int text_max = 0;
	char text[256];
	int ret = 0;
	char ttype[256] = {0};
	const char * val;
	int i;

	lp = malloc(sizeof(T3NET_DLC_LIST));
	if(!lp)
	{
		goto fail_out;
	}
	lp->items = 0;

	sprintf(ttype, "%d", type);
	t3net_strcpy(url_w_arg, url, 1024);
	t3net_strcat(url_w_arg, "?project_id=", 1024);
	t3net_strcat(url_w_arg, game, 1024);
	t3net_strcat(url_w_arg, "&type=", 1024);
	t3net_strcat(url_w_arg, ttype, 1024);

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
			t3net_strcpy(lp->item[i]->name, val, 256);
		}
		val = t3net_get_data_entry_field(data, i, "author");
		if(val)
		{
			t3net_strcpy(lp->item[i]->author, val, 256);
		}
		val = t3net_get_data_entry_field(data, i, "description");
		if(val)
		{
			t3net_strcpy(lp->item[i]->description, val, 256);
		}
		val = t3net_get_data_entry_field(data, i, "url");
		if(val)
		{
			t3net_strcpy(lp->item[i]->url, val, 256);
		}
		val = t3net_get_data_entry_field(data, i, "preview_url");
		if(val)
		{
			t3net_strcpy(lp->item[i]->preview_url, val, 256);
		}
		val = t3net_get_data_entry_field(data, i, "hash");
		if(val)
		{
			lp->item[i]->hash = atoi(val);
		}
	}

	lp->items = data->entries;
	t3net_destroy_data(data);

	return lp;

	fail_out:
	{
		if(lp)
		{
			free(lp);
		}
		if(data)
		{
			t3net_destroy_data(data);
		}
	}
	return NULL;
}

void t3net_destroy_dlc_list(T3NET_DLC_LIST * lp, void (*callback)(void * data))
{
	int i;

	for(i = 0; i < lp->items; i++)
	{
		if(lp->item[i]->preview && callback)
		{
			callback(lp->item[i]->preview);
		}
		free(lp->item[i]);
	}
	free(lp);
}

/* easy way to remove DLC items we already have from the list */
void t3net_remove_dlc_item(T3NET_DLC_LIST * lp, unsigned long hash)
{
	int i, j;

	for(i = 0; i < lp->items; i++)
	{
		if(lp->item[i]->hash == hash)
		{
			free(lp->item[i]);
			for(j = i; j < lp->items - 1; j++)
			{
				lp->item[j] = lp->item[j + 1];
			}
			lp->items--;
			return;
		}
	}
}

static void (*t3net_download_callback)() = NULL;

void t3net_set_download_callback(void (*callback)())
{
	t3net_download_callback = callback;
}

static size_t t3net_file_write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);
	if(t3net_download_callback)
	{
		t3net_download_callback();
	}
	return written;
}

int t3net_download_file(const char * url, const char * fn)
{
	CURL *curl;
	FILE * fp;
	CURLcode res;
	curl = curl_easy_init();
    if(curl)
    {
		fp = fopen(fn, "wb");
		if(fp)
		{
			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,   1); // follow redirects
			curl_easy_setopt(curl, CURLOPT_AUTOREFERER,      1); // set the Referer: field in requests where it follows a Location: redirect.
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS,        20);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, t3net_file_write_data);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			fclose(fp);
			if(res == 0)
			{
				return 1;
			}
		}
    }
	return 0;
}
