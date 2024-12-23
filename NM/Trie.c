#include "Trie.h"

Trie *InitTrie(void)
{
    Trie *Head = (Trie *)malloc(sizeof(Trie));
    for (int i = 0; i < alphabets; i++)
    {
        Head->children[i] = NULL;
    }
    Head->Exists = -1;
    return Head;
}

void InsertTrie(char *s, Trie *Head, int index)
{
    Trie *curr = Head;
    for (int i = 0; i < strlen(s); i++)
    {
        if (curr->children[s[i]] == NULL)
        {
            curr->children[s[i]] = (Trie *)malloc(sizeof(Trie));
            curr->children[s[i]]->Exists = -1;
            curr->children[s[i]]->read_count = 0;
            sem_init(&(curr->children[s[i]]->read_write_mutex), 0, 1);
            sem_init(&(curr->children[s[i]]->write_mutex), 0, 1);
            for (int j = 0; j < alphabets; j++)
            {
                curr->children[s[i]]->children[j] = NULL;
            }
        }
        curr = curr->children[s[i]];
        if (i == strlen(s) - 1)
        {
            curr->Exists = index;
        }
    }
}

Trie *SearchTrie(char *s, Trie *Head)
{
    Trie *curr = Head;
    for (int i = 0; i < strlen(s) - 1; i++)
    {
        if (curr->children[s[i]] == NULL)
        {
            return NULL;
        }
        curr = curr->children[s[i]];
    }
    if (curr->children[s[strlen(s) - 1]] == NULL)
    {
        return NULL;
    }
    if (curr->children[s[strlen(s) - 1]]->Exists == -1)
    {
        return curr->children[s[strlen(s) - 1]];
    }
    return curr->children[s[strlen(s) - 1]];
}

void PrintTrie(Trie *Head, int depth, char *str, char **paths, int *count)
{
    int flag;
    if (paths != NULL)
    {
        flag = 1;
    }
    for (int i = 0; i < alphabets; i++)
    {
        if (Head->children[i] != NULL)
        {
            str[depth] = i;
            if (Head->children[i]->Exists != -1)
            {
                str[depth + 1] = '\0';
                printf("%s\n", str);
                if (flag == 1)
                {
                    strcpy(paths[*count], str);
                    *count += 1;
                }
            }
            PrintTrie(Head->children[i], depth + 1, str, paths, count);
        }
    }
}

void FindallwithPrefix(char *str, Trie *Head, char **paths, int *count)
{
    Trie *curr = Head;
    for (int i = 0; i < strlen(str); i++)
    {
        if (curr->children[str[i]] != NULL)
        {
            curr = curr->children[str[i]];
        }
        else
        {
            return;
        }
    }
    if (curr->Exists != -1)
    {
        // strcpy(paths[*count],str);
        // *count+=1;
        printf("%s\n", str);
    }
    int depth = strlen(str);
    PrintTrie(curr, depth, str, paths, count);
}

void Delete(char *s, Trie *Head)
{
    Trie *temp = SearchTrie(s, Head);
    if (temp == NULL)
        return;
    temp->Exists = -1;
    return;
}

void Delete_all_with_prefix(char *str, Trie *Head)
{
    if (Head == NULL)
    {
        return;
    }
    Trie *temp = SearchTrie(str, Head);
    // if (temp==NULL)
    // {
    //     temp=Head;
    // }
    if (temp == NULL)
    {
        return;
    }
    char temping[256];
    strcpy(temping, str);
    // int len=strlen(temping);
    temping[1] = '\0';
    for (int i = 0; i < alphabets; i++)
    {
        temping[0] = i;
        // if (temping[0]=='p')
        // {
        //     // printf("Kya");
        // }
        // printf("%s\n",temping);
        Delete_all_with_prefix(temping, temp);
    }
    temp->Exists = -1;
    return;
}

// void DeleteNode(char* s,Trie* Head,int index)
// {
//     if(Head==NULL)
//     {return;}
//     Trie* curr=Head;
//     for (int i=0;i<strlen(s)-1;i++)
//     {
//         if (curr->children[s[i]-'a']==NULL)
//         {
//             return NULL;
//         }
//         curr=curr->children[s[i]-'a'];
//     }

// }