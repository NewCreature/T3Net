#include "t3net.h"
#include "leaderboard.h"
#include "internal.h"

T3NET_LEADERBOARD * t3net_get_leaderboard(char * url, char * game, char * version, char * mode, char * option, int entries, int ascend)
{
	T3NET_LEADERBOARD * lp;
	int i, j;

	/* build leaderboard structure */
	lp = malloc(sizeof(T3NET_LEADERBOARD));
	if(!lp)
	{
		return NULL;
	}
	lp->entry = malloc(sizeof(T3NET_LEADERBOARD_ENTRY *) * entries);
	if(!lp->entry)
	{
		free(lp);
		return NULL;
	}
	for(i = 0; i < entries; i++)
	{
		lp->entry[i] = malloc(sizeof(T3NET_LEADERBOARD_ENTRY));
		if(!lp->entry[i])
		{
			break;
		}
		strcpy(lp->entry[i]->name, "");
		lp->entry[i]->score = -1;
	}
	if(i < entries)
	{
		for(j = 0; j < i; j++)
		{
			free(lp->entry[i]);
		}
		free(lp->entry);
		return NULL;
	}
	lp->entries = entries;
	strcpy(lp->url, url);
	strcpy(lp->game, game);
	strcpy(lp->version, version);
	strcpy(lp->mode, mode);
	strcpy(lp->option, option);
	lp->ascend = ascend;
	if(!t3net_update_leaderboard_2(lp))
	{
		t3net_destroy_leaderboard(lp);
		return NULL;
	}

	/* query the server */
	return lp;
}

int t3net_update_leaderboard_2(T3NET_LEADERBOARD * lp)
{
	CURL * curl;
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
	sprintf(url_w_arg, "%s?game=%s&version=%s&mode=%s&option=%s%s&limit=%d", lp->url, lp->game, lp->version, lp->mode, lp->option, lp->ascend ? "&ascend=true" : "", lp->entries);
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
	if(!strncmp(data, "Error", 5))
	{
		return 0;
	}

	text_pos = 0;
    text_max = strlen(data);

    /* skip first two lines */
    t3net_read_line(data, text, text_max, 256, &text_pos);
    t3net_read_line(data, text, text_max, 256, &text_pos);
	while(ecount < lp->entries)
	{
		if(t3net_read_line(data, text, text_max, 256, &text_pos))
		{
			t3net_get_element(text, &element, text_max);
			if(!strcmp(element.name, "name"))
			{
				ecount++;
				strcpy(lp->entry[ecount]->name, element.data);
			}
			else if(!strcmp(element.name, "score"))
			{
				lp->entry[ecount]->score = atoi(element.data);
			}
			else if(!strcmp(element.name, "extra"))
			{
				strcpy(lp->entry[ecount]->extra, element.data);
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

void t3net_clear_leaderboard(T3NET_LEADERBOARD * lp)
{
	lp->entries = 0;
}

void t3net_destroy_leaderboard(T3NET_LEADERBOARD * lp)
{
	int i;

	for(i = 0; i < lp->entries; i++)
	{
		free(lp->entry[i]);
	}
	free(lp->entry);
	free(lp);
}

int t3net_upload_score(char * url, char * game, char * version, char * mode, char * option, char * name, unsigned long score, char * extra)
{
	CURL * curl;
	char * data = NULL;
	char url_w_arg[1024] = {0};
	char tname[256] = {0};

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
	t3net_strcpy_url(tname, name);
	sprintf(url_w_arg, "%s?uploadScore&game=%s&version=%s&mode=%s&option=%s&name=%s&score=%lu", url, game, version, mode, option, tname, score);
	if(extra)
	{
		strcat(url_w_arg, "&extra=");
		strcat(url_w_arg, extra);
	}
//	printf("%s\n", url_w_arg);
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

    return 1;
}
