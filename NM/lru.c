#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../packet.h"
#include "lru.h"
#include <time.h>

cache *cache_array[MAX_CACHE];

void init_cache()
{
    for (int i = 0; i < MAX_CACHE; i++)
    {
        cache_array[i] = malloc(sizeof(cache));
    }
    for (int i = 0; i < MAX_CACHE; i++)
    {
        for (int j = 0; j < MAX_PATH_LENGTH; j++)
            cache_array[i]->path[j] = '\0';
    }

    for (int i = 0; i < MAX_CACHE; i++)
    {
        cache_array[i]->storage_server_index = -1;
    }
}

int least_recently_used()
{
    int least_recently_used_index = 0;
    time_t least_recently_used_timestamp = cache_array[0]->timestamp;

    for (int i = 1; i < MAX_CACHE; i++)
    {
        if (cache_array[i]->timestamp < least_recently_used_timestamp)
        {
            least_recently_used_index = i;
            least_recently_used_timestamp = cache_array[i]->timestamp;
        }
    }

    return least_recently_used_index;
}

void add_to_cache(char *path, int storage_server_index)
{
    for (int i = 0; i < MAX_CACHE; i++)
    {
        if (cache_array[i]->storage_server_index == -1)
        {
            strcpy(cache_array[i]->path, path);
            cache_array[i]->storage_server_index = storage_server_index;
            cache_array[i]->timestamp = time(NULL);
            return;
        }
    }

    int least_recently_used_index = least_recently_used();
    strcpy(cache_array[least_recently_used_index]->path, path);
    for (int i = strlen(path); i < MAX_PATH_LENGTH; i++)
    {
        cache_array[least_recently_used_index]->path[i] = '\0';
    }
    cache_array[least_recently_used_index]->storage_server_index = storage_server_index;
    cache_array[least_recently_used_index]->timestamp = time(NULL);
}

int get_storage_server_index(char *path)
{
    for (int i = 0; i < MAX_CACHE; i++)
    {
        if (strcmp(cache_array[i]->path, path) == 0)
        {
            cache_array[i]->timestamp = time(NULL);
            return cache_array[i]->storage_server_index;
        }
    }

    return -1;
}

void remove_from_cache(char *path)
{
    int index = get_storage_server_index(path);
    if (index == -1)
    {
        return;
    }
    else
    {
        for (int i = 0; i < MAX_PATH_LENGTH; i++)
        {
            cache_array[index]->path[i] = '\0';
        }
        cache_array[index]->storage_server_index = -1;
        cache_array[index]->timestamp = 0;
    }
}