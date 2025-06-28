#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t3net.h"
#include "leaderboard.h"

/* leaderboard setup functions */
char * t3net_get_new_leaderboard_user_key(const char * url, const char * user_name)
{
	T3NET_ARGUMENTS * args = NULL;
	const char * val;
	T3NET_DATA * data = NULL;
	int i;
	char * ret = NULL;

	args = t3net_create_arguments();
	if(!args)
	{
		goto fail;
	}
	if(user_name && strlen(user_name) > 0)
	{
		if(!t3net_add_argument(args, "user_name", user_name))
		{
			goto fail;
		}
	}
	data = t3net_get_data(url, args, NULL);
	if(!data)
	{
		goto fail;
	}

	for(i = 0; i < data->entries; i++)
	{
		val = t3net_get_data_entry_field(data, i, "user_key");
		if(val)
		{
			ret = strdup(val);
			break;
		}
	}
	t3net_destroy_arguments(args);
	t3net_destroy_data(data);

	return ret;

	fail:
	{
		if(ret)
		{
			free(ret);
		}
		if(args)
		{
			t3net_destroy_arguments(args);
		}
		if(data)
		{
			t3net_destroy_data(data);
		}
		return NULL;
	}
}

int t3net_update_leaderboard_user_name(const char * url, const char * user_key, const char * user_name)
{
	T3NET_ARGUMENTS * args = NULL;
	T3NET_DATA * data = NULL;

	args = t3net_create_arguments();
	if(!args)
	{
		goto fail;
	}
	if(user_name)
	{
		if(!t3net_add_argument(args, "user_key", user_key))
		{
			goto fail;
		}
		if(!t3net_add_argument(args, "user_name", user_name))
		{
			goto fail;
		}
	}
	data = t3net_get_data(url, args, NULL);
	if(!data)
	{
		goto fail;
	}
	t3net_destroy_arguments(args);
	t3net_destroy_data(data);

	return 1;

	fail:
	{
		if(args)
		{
			t3net_destroy_arguments(args);
		}
		if(data)
		{
			t3net_destroy_data(data);
		}
		return 0;
	}
}

static T3NET_DATA * _retrieve_leaderboard_data(const char * url, const char * game, const char * version, const char * mode, const char * option, int entries, int ascend)
{
	T3NET_ARGUMENTS * args = NULL;
	char tnum[64] = {0};
	T3NET_DATA * data = NULL;

	sprintf(tnum, "%d", entries);
	args = t3net_create_arguments();
	if(!args)
	{
		goto fail;
	}
	if(!t3net_add_argument(args, "game", game))
	{
		goto fail;
	}
	if(version && strlen(version) > 0 && !t3net_add_argument(args, "version", version))
	{
		goto fail;
	}
	if(mode && strlen(mode) > 0 && !t3net_add_argument(args, "mode", mode))
	{
		goto fail;
	}
	if(option && strlen(option) > 0 && !t3net_add_argument(args, "mode_option", option))
	{
		goto fail;
	}
	if(ascend)
	{
		if(!t3net_add_argument(args, "ascend", "true"))
		{
			goto fail;
		}
	}
	if(!t3net_add_argument(args, "limit", tnum))
	{
		goto fail;
	}
	data = t3net_get_data(url, args, NULL);
	if(!data)
	{
		goto fail;
	}
	t3net_destroy_arguments(args);
	return data;

	fail:
	{
		if(args)
		{
			t3net_destroy_arguments(args);
		}
		if(data)
		{
			t3net_destroy_data(data);
		}
		return NULL;
	}
}

T3NET_LEADERBOARD * t3net_create_leaderboard(int entries)
{
	T3NET_LEADERBOARD * lp = NULL;
	int i;

	lp = malloc(sizeof(T3NET_LEADERBOARD));
	if(!lp)
	{
		goto fail;
	}
	memset(lp, 0, sizeof(T3NET_LEADERBOARD));
	lp->entry = malloc(sizeof(T3NET_LEADERBOARD_ENTRY *) * entries);
	if(!lp->entry)
	{
		goto fail;
	}
	memset(lp->entry, 0, sizeof(T3NET_LEADERBOARD_ENTRY *) * entries);
	for(i = 0; i < entries; i++)
	{
		lp->entry[i] = malloc(sizeof(T3NET_LEADERBOARD_ENTRY));
		if(!lp->entry[i])
		{
			goto fail;
		}
		memset(lp->entry[i], 0, sizeof(T3NET_LEADERBOARD_ENTRY));
		lp->entry[i]->score = -1;
	}
	lp->entry_size = entries;
	lp->entries = 0;

	return lp;

	fail:
	{
		t3net_destroy_leaderboard(lp);
		return NULL;
	}
}

T3NET_LEADERBOARD * t3net_get_leaderboard(const char * url, const char * game, const char * version, const char * mode, const char * option, int entries, int ascend)
{
	T3NET_LEADERBOARD * lp = NULL;
	T3NET_DATA * data = NULL;
	const char * val;
	int i;

	lp = t3net_create_leaderboard(entries);
	if(!lp)
	{
		goto fail;
	}
	data = _retrieve_leaderboard_data(url, game, version, mode, option, entries, ascend);
	if(!data)
	{
		goto fail;
	}

	/* copy leaderboard data from the data set */
	for(i = 0; i < data->entries && i < lp->entry_size; i++)
	{
		val = t3net_get_data_entry_field(data, i, "name");
		if(val)
		{
			lp->entry[i]->name = strdup(val);
			if(!lp->entry[i]->name)
			{
				goto fail;
			}
		}
		val = t3net_get_data_entry_field(data, i, "score");
		if(val)
		{
			lp->entry[i]->score = atoi(val);
		}
		val = t3net_get_data_entry_field(data, i, "extra");
		if(val)
		{
			lp->entry[i]->extra = strdup(val);
			if(!lp->entry[i]->extra)
			{
				goto fail;
			}
		}
		lp->entries++;
	}

	return lp;

	fail:
	{
		if(data)
		{
			t3net_destroy_data(data);
		}
		t3net_destroy_leaderboard(lp);
		return NULL;
	}
}

void t3net_destroy_leaderboard(T3NET_LEADERBOARD * lp)
{
	int i;

	if(lp)
	{
		if(lp->entry)
		{
			for(i = 0; i < lp->entry_size; i++)
			{
				if(lp->entry[i])
				{
					if(lp->entry[i]->name)
					{
						free(lp->entry[i]->name);
					}
					if(lp->entry[i]->extra)
					{
						free(lp->entry[i]->extra);
					}
					free(lp->entry[i]);
				}
			}
			free(lp->entry);
		}
		free(lp);
	}
}

int t3net_upload_score(const char * url, const char * game, const char * version, const char * mode, const char * option, const char * user_key, unsigned long score, const char * extra)
{
	T3NET_ARGUMENTS * args = NULL;
	T3NET_DATA * data = NULL;
	char tscore[64] = {0};

//	sprintf(url_w_arg, "%s?game=%s&version=%s&mode=%s&option=%s&name=%s&score=%lu", url, game, version, mode, option, tname, score);
	args = t3net_create_arguments();
	if(!args)
	{
		goto fail;
	}
	sprintf(tscore, "%lu", score);
	if(!t3net_add_argument(args, "game", game))
	{
		goto fail;
	}
	if(version && !t3net_add_argument(args, "version", version))
	{
		goto fail;
	}
	if(mode && !t3net_add_argument(args, "mode", mode))
	{
		goto fail;
	}
	if(option && !t3net_add_argument(args, "mode_option", option))
	{
		goto fail;
	}
	if(!t3net_add_argument(args, "user_key", user_key))
	{
		goto fail;
	}
	if(!t3net_add_argument(args, "score", tscore))
	{
		goto fail;
	}
	if(extra)
	{
		if(!t3net_add_argument(args, "extra", extra))
		{
			goto fail;
		}
	}
	data = t3net_get_data(url, args, NULL);
	if(!data)
	{
		goto fail;
	}
	t3net_destroy_arguments(args);
	t3net_destroy_data(data);
	return 1;

	fail:
	{
		if(args)
		{
			t3net_destroy_arguments(args);
		}
		if(data)
		{
			t3net_destroy_data(data);
		}
		return 0;
	}
}
