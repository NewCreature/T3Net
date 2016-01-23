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
	if(!t3net_update_leaderboard(lp))
	{
		t3net_destroy_leaderboard(lp);
		return NULL;
	}

	/* query the server */
	return lp;
}

int t3net_update_leaderboard(T3NET_LEADERBOARD * lp)
{
	char url_w_arg[1024] = {0};
	char tnum[64] = {0};
	const char * val;
	T3NET_DATA * data;
	int i;

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
		val = t3net_get_data_entry_field(data, i, "score");
		if(val)
		{
			lp->entry[i]->score = atoi(val);
		}
		val = t3net_get_data_entry_field(data, i, "extra");
		if(val)
		{
			t3net_strcpy(lp->entry[i]->extra, val, 256);
		}
	}
	lp->entries = data->entries;
	t3net_destroy_data(data);
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
	T3NET_DATA * data;
	char url_w_arg[1024] = {0};
	char tname[256] = {0};
	char tscore[64] = {0};

//	sprintf(url_w_arg, "%s?game=%s&version=%s&mode=%s&option=%s&name=%s&score=%lu", url, game, version, mode, option, tname, score);
	t3net_strcpy_url(tname, name, 256);
	t3net_strcpy_url(url_w_arg, url, 1024);
	sprintf(tscore, "%lu", score);
	t3net_strcat(url_w_arg, "?game=", 1024);
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
		t3net_strcat(url_w_arg, "&extra=", 1024);
		t3net_strcat(url_w_arg, extra, 1024);
	}
	data = t3net_get_data(url_w_arg);
	if(!data)
	{
		return 0;
	}
	t3net_destroy_data(data);
	return 1;
}
