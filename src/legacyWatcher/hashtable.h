#ifndef HASHTABLE_H
#define HASHTABLE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*hashtablecompare)(char *k, int keylen, int nbuck);
typedef void (*hashtableitem)(char *k, void *data);

typedef struct _hashbucket
{
	void *data;
	char *key;
	int keylen;
	struct _hashbucket *next;
} hashbucket;

typedef struct _hastable
{
	int nbuck;
	hashtablecompare hashfunc;
	hashbucket **buck;
	
} hashtable;

hashtable *hashtableinit(int nbucket,hashtablecompare hashfunc);
void hashtableinsert(hashtable *t,char *key, int keylen,void *dat);
void *hashtablesearch(hashtable *t,char *key, int keylen);
void hashtabledelete(hashtable *t,char *key, int keylen);
void hashtabletraverse(hashtable *t,hashtableitem itemfunc);
int hashtablehash(char *k, int keylen, int nbuck);
void hashtablefree(hashtable *t);

#ifdef __cplusplus
};
#endif

#endif
