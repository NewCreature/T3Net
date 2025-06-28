#ifndef T3NET_LEADERBOARD_H
#define T3NET_LEADERBOARD_H

#include "t3net.h"

typedef struct
{

	char * name;
	unsigned long score;
	char * extra;

} T3NET_LEADERBOARD_ENTRY;

typedef struct
{

	T3NET_LEADERBOARD_ENTRY ** entry;
	int entry_size;
	int entries;

} T3NET_LEADERBOARD;

/* leaderboard setup functions */
char * t3net_get_new_leaderboard_user_key(const char * url, const char * user_name);
int t3net_update_leaderboard_user_name(const char * url, const char * user_key, const char * user_name);
T3NET_LEADERBOARD * t3net_create_leaderboard(int entries);

/* leaderboard download functions */
T3NET_LEADERBOARD * t3net_get_leaderboard(const char * url, const char * game, const char * version, const char * mode, const char * option, int entries, int ascend);
void t3net_destroy_leaderboard(T3NET_LEADERBOARD * lp);

/* leaderboard upload functions */
int t3net_upload_score(const char * url, const char * game, const char * version, const char * mode, const char * option, const char * user_key, unsigned long score, const char * extra);

#endif
