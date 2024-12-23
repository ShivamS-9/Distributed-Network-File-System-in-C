#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

#define alphabets 128

typedef struct Trie
{
    struct Trie *children[alphabets];
    int Exists;
    int read_count;
    sem_t read_write_mutex;
    sem_t write_mutex;
} Trie;

Trie *InitTrie(void);

void InsertTrie(char *s, Trie *Head, int index);

void PrintTrie(Trie *Head, int depth, char *str, char **paths, int *count);

void FindallwithPrefix(char *str, Trie *Head, char **paths, int *count);

void Delete(char *str, Trie *Head);

Trie *SearchTrie(char *s, Trie *Head);
// void DeleteNode(char* s,Trie* Head,int index);
void Delete_all_with_prefix(char *str, Trie *Head);
