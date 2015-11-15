#ifndef T3NET_H
#define T3NET_H

#define T3NET_TIMEOUT_TIME 10

extern char t3net_server_message[1024];

/* string operations */
void t3net_set_str_functions(int (*strget_proc)(const char * s, int point), int (*t3net_strset_proc)(char * s, int point, int val, int size));
int t3net_strget(const char * s, int point);
int t3net_strset(char * s, int point, int val, int size);
int t3net_strlen(const char * s);
void t3net_strcpy(char * dest, const char * src, int size);
void t3net_strcpy_url(char * dest, const char * src, int size);
char * t3net_strcat(char * dest, const char * src, int size);

/* internet operations */
char * t3net_get_data(const char * url, int data_size);

#endif
