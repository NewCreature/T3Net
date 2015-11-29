#ifndef T3NET_H
#define T3NET_H

#define T3NET_TIMEOUT_TIME      10

/* A chunk of memory this size will be allocated when retrieving data. Each
   time the size of the data exceeds this chunk size, the chunk will be
   reallocated to add this many bytes to the chunk. */
#define T3NET_DATA_CHUNK_SIZE 4096

typedef struct
{

    char * name;
    char * data;

} T3NET_DATA_ENTRY_FIELD;

typedef struct
{

    T3NET_DATA_ENTRY_FIELD ** field;
    int fields;

} T3NET_DATA_ENTRY;

typedef struct
{

    char * header;
    T3NET_DATA_ENTRY ** entry;
    int entries;

} T3NET_DATA;

extern char t3net_server_message[1024];

/* string operations */
void t3net_set_str_functions(int (*strget_proc)(const char * s, int point, int * bytes), int (*t3net_strset_proc)(char * s, int point, int val, int size));
int t3net_strget(const char * s, int point, int * bytes);
int t3net_strset(char * s, int point, int val, int size);
int t3net_strlen(const char * s, int * bytes);
void t3net_strcpy(char * dest, const char * src, int size);
void t3net_strcpy_url(char * dest, const char * src, int size);
char * t3net_strcat(char * dest, const char * src, int size);

/* internet operations */
char * t3net_get_raw_data(const char * url);
T3NET_DATA * t3net_get_data_from_string(const char * s);
T3NET_DATA * t3net_get_data(const char * url);
void t3net_destroy_data(T3NET_DATA * data);

#endif
