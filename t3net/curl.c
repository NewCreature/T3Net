#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#include <windows.h>
#endif
#include "t3net.h"

static char _t3net_curl_command[1024] = {0};
static char * _t3net_curl_temp_dir = NULL;

static int _t3net_run_system_command(char * command, const char * log_file)
{
	int ret;

	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	
		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi = {0};
		SECURITY_ATTRIBUTES sa;
		DWORD retvalue;
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;
		HANDLE log_handle = CreateFile(log_file, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.hStdInput = NULL;
		si.hStdOutput = log_handle;
		si.hStdError = log_handle;
		si.wShowWindow = SW_HIDE;
		ret = CreateProcess(NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
		WaitForSingleObject(pi.hProcess, INFINITE);
		GetExitCodeProcess(pi.hProcess, &retvalue);
		if(log_handle)
		{
			CloseHandle(log_handle);
		}
		ret = retvalue;

	#else

		char final_command[1024];
		strcpy(final_command, command);
		if(log_file)
		{
			strcat(final_command, " > \"");
			strcat(final_command, log_file);
			strcat(final_command, "\"");
		}
		ret = system(final_command);

	#endif

	return ret;
}

static int _t3net_get_post_data_length(char ** post_data)
{
	int i;
	int l = 0;

	if(post_data)
	{
		for(i = 0; post_data[i]; i++)
		{
			l += strlen(post_data[i]);
		}
	}
	return l;
}

static int _t3net_get_curl_command(char * out, int out_size)
{
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
		strcpy(out, "./curl.exe");
	#elif __APPLE__
		strcpy(out, "/usr/bin/curl");
	#else
		strcpy(out, "LD_LIBRARY_PATH=\"/lib\" curl");
	#endif
	return 1;
}

static int _t3net_append_to_command(char * out, int out_size, const char * in)
{
  if(strlen(in) + strlen(out) >= out_size - 1)
  {
    return 0;
  }
  strcat(out, in);
  return 1;
}

static char * _t3net_load_file(const char * fn)
{
	char * data = NULL;
	FILE * fp = NULL;
	int size = 0;

	fp = fopen(fn, "rb");
	if(!fp)
	{
		goto fail;
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fclose(fp);
	fp = NULL;

	data = malloc(size + 1);
	if(!data)
	{
		goto fail;
	}
	fp = fopen(fn, "rb");
	if(!fp)
	{
		goto fail;
	}
	fread(data, 1, size, fp);
	fclose(fp);
	data[size] = 0;
	remove(fn);
	return data;

	fail:
	{
		if(fp)
		{
			fclose(fp);
		}
		if(data)
		{
			free(data);
		}
		return NULL;
	}
}

static int _t3net_curl_url_runner(const char * url, char ** post_data, const char * out_path, char ** out_data)
{
	char * curl_command = malloc(strlen(url) + _t3net_get_post_data_length(post_data) + 1024);
	char temp_path[1024] = {0};
	char buf[256];
	int ret = 0;
	int i;

	if(curl_command)
	{
		if(out_path)
		{
			sprintf(temp_path, "%s", out_path);
		}
		else
		{
			sprintf(temp_path, "%st3net.out", t3net_temp_dir);
		}
    if(!_t3net_get_curl_command(curl_command, 1024))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, " -L"))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, " --connect-timeout"))
    {
      goto fail;
    }
		sprintf(buf, " %d", T3NET_TIMEOUT_TIME);
		if(!_t3net_append_to_command(curl_command, 1024, buf))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, " --silent"))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, " --output"))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, " \""))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, temp_path))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, "\""))
    {
      goto fail;
    }
		if(post_data)
		{
			for(i = 0; post_data[i]; i++)
			{
				if(!_t3net_append_to_command(curl_command, 1024, " -F \""))
        {
          goto fail;
        }
				if(!_t3net_append_to_command(curl_command, 1024, post_data[i]))
        {
          goto fail;
        }
				if(!_t3net_append_to_command(curl_command, 1024, "\""))
        {
          goto fail;
        }
			}
		}
		if(!_t3net_append_to_command(curl_command, 1024, " \""))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, url))
    {
      goto fail;
    }
		if(!_t3net_append_to_command(curl_command, 1024, "\""))
    {
      goto fail;
    }

		ret = !_t3net_run_system_command(curl_command, NULL);
		free(curl_command);
		if(out_data)
		{
			*out_data = _t3net_load_file(temp_path);
			if(!*out_data)
			{
				ret = 0;
			}
		}
	}
	return ret;

  fail:
  {
    return 0;
  }
}

static void _t3net_curl_exit_proc(void)
{
  if(_t3net_curl_temp_dir)
  {
    free(_t3net_curl_temp_dir);
  }
}

int t3net_setup_with_curl(const char * temp_dir)
{
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
