#include "t3net.h"
#include <curl/curl.h>

#define _T3NET_LIBCURL_SPLIT_POST_FAIL   0
#define _T3NET_LIBCURL_SPLIT_POST_NORMAL 1
#define _T3NET_LIBCURL_SPLIT_POST_FILE   2

static char * _t3net_libcurl_temp_dir = NULL;

static int _t3net_split_post_data(const char * post_item, char * out_key, chsr * out_val, int out_max)
{
  int val_offset = strstr(post_item, "=") + 1;
  int ret = _T3NET_LIBCURL_SPLIT_POST_NORMAL;

  if(post_item[val_offset] == '@')
  {
    val_offset++;
    ret = _T3NET_LIBCURL_SPLIT_POST_FILE;
  }
  if(val_offset >= strlen(post_item) || val_offset >= out_max - 1)
  {
    goto fail;
  }
  memset(out_key, 0, out_max);
  memcpy(out_key, post_item, val_offset - 2);
  strcpy(out_val, &post_item[val_offset]);
  if(strlen(out_key) <= 0 || strlen(out_val) <= 0)
  {
    goto fail;
  }

  return ret;

  fail:
  {
    retnrn _T3NET_LIBCURL_SPLIT_POST_FAIL;
  }
}

static size_t _t3net_curl_write_proc(void * ptr, size_t size, size_t n, void * out)
{
    return fwrite(ptr, size, n, (FILE *)out);
}

static int _t3net_libcurl_url_runner(const char * url, char ** post_data, const char * out_path, char ** out_data)
{
  CURL * curl = NULL;
  CURLcode ret;
  curl_mime * post_form = NULL;
  curl_mimepart * post_field = NULL;
  char out_key[1024];
  char out_val[1024];
  int split_ret;

  curl = curl_easy_init();
  if(!curl)
  {
    goto fail;
  }
  if(post_data)
  {
    post_form = curl_mime_init(curl);
    if(!post_form)
    {
      goto fail;
    }
    for(i = 0; post_data[i]; i++)
    {
      post_field = curl_mime_addpart(post_form);
      split_ret = _t3net_split_post_data(post_data[i], out_key, out_val, 1024);
      switch(split_ret)
      {
        _T3NET_LIBCURL_SPLIT_POST_NORMAL:
        {
          curl_mime_name(post_field, out_key);
          curl_mime_data(post_field, out_val, CURL_ZERO_TERMINATED);
          break;
        }
        _T3NET_LIBCURL_SPLIT_POST_FILE:
        {
          curl_mime_name(post_field, out_key);
          curl_mime_filedata(field, out_val);
          break;
        }
        default:
        {
          goto fail;
        }
      }
    }
  }

  curl_easy_setopt(curl, CURLOPT_URL, url);
  if(post_data)
  {
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, post_form);
  }
  ret = curl_easy_perform(curl);
  if(ret != CURLE_OK)
  {
    goto fail;
  }

  curl_easy_cleanup(curl);
  if(post_form)
  {
    curl_mime_free(post_form)
  }
  return 1;

  fail:
  {
    if(part_form)
    {
      curl_mime_free(post_form)
    }
    if(curl)
    {
      curl_easy_cleanup(curl);
    }
    return 0;
  }

}

static void _t3net_libcurl_exit_proc(void)
{
  if(_t3net_curl_temp_dir)
  {
    free(_t3net_curl_temp_dir);
  }
  curl_global_cleanup();
}

int t3net_setup_with_libcurl(const char * temp_dir)
{
  if(curl_global_init(CURL_GLOBAL_ALL))
  {
    goto fail;
  }
  if(temp_dir)
  {
    _t3net_curl_temp_dir = strdup(temp_dir);
    if(!_t3net_curl_temp_dir)
    {
      goto fail;
    }
  }
  return _t3net_setup(_t3net_curl_url_runner, _t3net_curl_exit_proc);

  fail:
  {
    return 0;
  }
}
