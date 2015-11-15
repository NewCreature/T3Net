#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
		t3net_strcpy(lp->entry[i]->name, "", 256);
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
	t3net_strcpy(lp->url, url, 1024);
	t3net_strcpy(lp->game, game, 64);
	t3net_strcpy(lp->version, version, 64);
	t3net_strcpy(lp->mode, mode, 64);
	t3net_strcpy(lp->option, option, 64);
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
	char url_w_arg[1024] = {0};
	char * data = NULL;
	int ecount = -1;
	unsigned int text_pos;
	int text_max;
	char text[256];
	char tnum[64] = {0};
	T3NET_TEMP_ELEMENT element;

	if(!lp)
	{
		return 0;
	}

	sprintf(tnum, "%d", lp->entries);
	t3net_strcpy_url(url_w_arg, lp->url, 1024);
	t3net_strcat(url_w_arg, "?game=", 1024);
	t3net_strcat(url_w_arg, lp->game, 1024);
	t3net_strcat(url_w_arg, "&version=", 1024);
	t3net_strcat(url_w_arg, lp->version, 1024);
	t3net_strcat(url_w_arg, "&mode=", 1024);
	t3net_strcat(url_w_arg, lp->mode, 1024);
	t3net_strcat(url_w_arg, "&option=", 1024);
	t3net_strcat(url_w_arg, lp->option, 1024);
	t3net_strcat(url_w_arg, lp->ascend ? "&ascend=true" : "", 1024);
	t3net_strcat(url_w_arg, "&limit=", 1024);
	t3net_strcat(url_w_arg, tnum, 1024);
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
				t3net_strcpy(lp->entry[ecount]->extra, element.data, 256);
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
	char * data = NULL;
	char url_w_arg[1024] = {0};
	char tname[256] = {0};
	char tscore[64] = {0};

	sprintf(url_w_arg, "%s?uploadScore&game=%s&version=%s&mode=%s&option=%s&name=%s&score=%lu", url, game, version, mode, option, tname, score);
	t3net_strcpy_url(tname, name, 256);
	t3net_strcpy_url(url_w_arg, url, 1024);
	sprintf(tscore, "%lu", score);
	t3net_strcat(url_w_arg, "?uploadScore&game=", 1024);
	t3net_strcat(url_w_arg, game, 1024);
	t3net_strcat(url_w_arg, "&version=", 1024);
	t3net_strcat(url_w_arg, version, 1024);
	t3net_strcat(url_w_arg, "&mode=", 1024);
	t3net_strcat(url_w_arg, mode, 1024);
	t3net_strcat(url_w_arg, "&option=", 1024);
	t3net_strcat(url_w_arg, option, 1024);
	t3net_strcat(url_w_arg, "&name=", 1024);
	t3net_strcat(url_w_arg, tname, 1024);
	t3net_strcat(url_w_arg, "&score=", 1024);
	t3net_strcat(url_w_arg, tscore, 1024);
	if(extra)
	{
		strcat(url_w_arg, "&extra=");
		strcat(url_w_arg, extra);
	}
	data = t3net_get_data(url_w_arg, 65536);
	if(!data)
	{
		return 0;
	}
	free(data);
	return 1;
}
