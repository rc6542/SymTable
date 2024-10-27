/*--------------------------------------------------------------------*/
/* symtablehash.c                                                        */
/* Author: Ryan Chen                                                  */
/*--------------------------------------------------------------------*/

#include "symtable.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* bucket counts to expand to */
static const size_t bucketCount[] = {509, 1021, 2039, 4093, 8191, 
16381, 32749, 65521};

/* Each SymTableNode stores a key-pair pair. SymTableNodes are linked to
   form a list.  */
struct SymTableNode
{
     /* the key */
     const void *pcKey;

     /* the value */
     const void *pvValue;

     /* address of next SymTableNode */
     struct SymTableNode *psNextNode;
};

struct SymTable {
    struct SymTableNode **buckets;
    size_t bucketCount;
    size_t bindingCount;
    size_t currentBucketIndex;
};

/* hash function */
static size_t SymTable_hash(const char *pcKey, size_t uBucketCount) {
    const size_t HASH_MULTIPLIER = 65599;
    size_t u;
    size_t uHash = 0;

    assert(pcKey != NULL);

    for (u = 0; pcKey[u] != '\\0'; u++)
        uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

    return uHash % uBucketCount;
}

SymTable_T SymTable_new(void);

void SymTable_free(SymTable_T oSymTable);

size_t SymTable_getLength(SymTable_T oSymTable);

int SymTable_put(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue);

void *SymTable_replace(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue);

int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra);