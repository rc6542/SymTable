/*--------------------------------------------------------------------*/
/* symtablehash.c                                                        */
/* Author: Ryan Chen                                                  */
/*--------------------------------------------------------------------*/

#include "symtable.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Bucket counts to expand to */
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

/* SymTable represents a hash table that stores key-value pairs. Each
   entry in the hash table points to a linked list of nodes in case of
   collisions. */
struct SymTable
{
    /* array of pointers to the buckets, where each bucket holds a
       linked list of nodes with the same hash value. */
    struct SymTableNode **buckets;

    /* current number of buckets in the SymTable */
    size_t bucketCount;

    /* total number of bindings in the SymTable */
    size_t bindingCount;

    /* tracks which index the bucketCount is at in order to dynamically
       expand */
    size_t currentBucketIndex;
};

/* Function that hashes pcKey based on uBucketCount. Returns the modulus
   which equals which bucket it goes into. */
static size_t SymTable_hash(const char *pcKey, size_t uBucketCount)
{
    const size_t HASH_MULTIPLIER = 65599;
    size_t u;
    size_t uHash = 0;

    assert(pcKey != NULL);

    for (u = 0; pcKey[u] != '\0'; u++)
        uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

    return uHash % uBucketCount;
}

/* Function that takes oSymTable and expands the buckets in it if it
   reaches the max bucket count, listed in bucketCount. Index zero is
   what the starting number of buckets is, and each index after is what
   it should expand to. Stops expanding after 65521 buckets. */
static void SymTable_expand(SymTable_T oSymTable)
{
    struct SymTableNode **moreBuckets;
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psNextNode;
    size_t oldBucketCount;
    size_t newBucketCount;
    size_t bucketIndex;
    size_t newBucketIndex;

    if (oSymTable->bucketCount == 65521)
    {
        return;
    }

    oSymTable->currentBucketIndex++;
    newBucketCount = bucketCount[oSymTable->currentBucketIndex];
    oSymTable->bucketCount = newBucketCount;
    oldBucketCount = oSymTable->bucketCount;
    
    moreBuckets = (struct SymTableNode **)calloc(newBucketCount, 
                                        sizeof(struct SymTableNode *));
    if (moreBuckets == NULL)
        return; 

    /* Rehashes nodes from old buckets into the new buckets */
    for (bucketIndex = 0; bucketIndex < oldBucketCount; bucketIndex++)
    {
        for (psCurrentNode = oSymTable->buckets[bucketIndex]; 
             psCurrentNode != NULL; 
             psCurrentNode = psNextNode)
        {
            psNextNode = psCurrentNode->psNextNode;

            newBucketIndex = SymTable_hash(psCurrentNode->pcKey, 
                                                        newBucketCount);

            psCurrentNode->psNextNode = moreBuckets[newBucketIndex];
            moreBuckets[newBucketIndex] = psCurrentNode;
        }
    }

    free(oSymTable->buckets);

    oSymTable->buckets = moreBuckets;
}

SymTable_T SymTable_new(void)
{
    SymTable_T oSymTable;

    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));

    if (oSymTable == NULL)
        return NULL;

    oSymTable->buckets = (struct SymTableNode **)calloc(bucketCount[0],
                          sizeof(struct SymTableNode *));

    if (oSymTable->buckets == NULL)
    {
        free(oSymTable);
        return NULL;
    }

    oSymTable->bucketCount = bucketCount[0];
    oSymTable->bindingCount = 0;
    oSymTable->currentBucketIndex = 0;

    return oSymTable;
}

void SymTable_free(SymTable_T oSymTable)
{
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psNextNode;
    size_t bucketIndex;

    assert(oSymTable != NULL);

    for (bucketIndex = 0;
         bucketIndex < oSymTable->bucketCount;
         bucketIndex++)
    {
        for (psCurrentNode = oSymTable->buckets[bucketIndex];
             psCurrentNode != NULL;
             psCurrentNode = psNextNode)
        {
            psNextNode = psCurrentNode->psNextNode;
            free((void *)psCurrentNode->pcKey);
            free(psCurrentNode);
        }
    }
    free(oSymTable->buckets);
    free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable)
{
    assert(oSymTable != NULL);
    return oSymTable->bindingCount;
}

int SymTable_put(SymTable_T oSymTable,
                 const char *pcKey, const void *pvValue)
{
    char *keyCopy;
    struct SymTableNode *psNewNode;
    struct SymTableNode *psCurrentNode;
    size_t bucketIndex;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    if (oSymTable->bindingCount > oSymTable->bucketCount) 
    {
        SymTable_expand(oSymTable);
    }

    /* find which bucket by hashing key */
    bucketIndex = SymTable_hash(pcKey, oSymTable->bucketCount);

    for (psCurrentNode = oSymTable->buckets[bucketIndex];
         psCurrentNode != NULL;
         psCurrentNode = psCurrentNode->psNextNode)
    {
        if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
        {
            return 0;
        }
    }

    psNewNode = (struct SymTableNode *)malloc(sizeof(struct
                                                     SymTableNode));
    if (psNewNode == NULL)
        return 0;

    keyCopy = (char *)malloc(strlen(pcKey) + 1);
    if (keyCopy == NULL)
    {
        free(psNewNode);
        return 0;
    }

    strcpy(keyCopy, pcKey);
    psNewNode->pcKey = keyCopy;
    psNewNode->pvValue = pvValue;

    psNewNode->psNextNode = oSymTable->buckets[bucketIndex];
    oSymTable->buckets[bucketIndex] = psNewNode;
    oSymTable->bindingCount += 1;

    return 1;
}

void *SymTable_replace(SymTable_T oSymTable,
                       const char *pcKey, const void *pvValue)
{
    struct SymTableNode *psCurrentNode;
    void *oldValue;
    size_t bucketIndex;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    bucketIndex = SymTable_hash(pcKey, oSymTable->bucketCount);

    for (psCurrentNode = oSymTable->buckets[bucketIndex];
         psCurrentNode != NULL;
         psCurrentNode = psCurrentNode->psNextNode)
    {
        if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
        {
            oldValue = (void *)psCurrentNode->pvValue;
            psCurrentNode->pvValue = pvValue;
            return oldValue;
        }
    }

    return NULL;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey)
{
    struct SymTableNode *psCurrentNode;
    size_t bucketIndex;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    bucketIndex = SymTable_hash(pcKey, oSymTable->bucketCount);

    for (psCurrentNode = oSymTable->buckets[bucketIndex];
         psCurrentNode != NULL;
         psCurrentNode = psCurrentNode->psNextNode)
    {
        if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
            return 1;
    }

    return 0;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey)
{
    struct SymTableNode *psCurrentNode;
    size_t bucketIndex;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    bucketIndex = SymTable_hash(pcKey, oSymTable->bucketCount);

    for (psCurrentNode = oSymTable->buckets[bucketIndex];
         psCurrentNode != NULL;
         psCurrentNode = psCurrentNode->psNextNode)
    {
        if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
            return (void *)psCurrentNode->pvValue;
    }

    return NULL;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey)
{
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psPrevNode = NULL;
    void *pvValue;
    size_t bucketIndex;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    bucketIndex = SymTable_hash(pcKey, oSymTable->bucketCount);

    for (psCurrentNode = oSymTable->buckets[bucketIndex];
         psCurrentNode != NULL;
         psCurrentNode = psCurrentNode->psNextNode)
    {
        if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
        {
            pvValue = (void *)psCurrentNode->pvValue;

            if (psPrevNode == NULL)
            {
                oSymTable->buckets[bucketIndex] = 
                                            psCurrentNode->psNextNode;
            }
            else
            {
                psPrevNode->psNextNode = psCurrentNode->psNextNode;
            }

            free((void *)psCurrentNode->pcKey);
            free(psCurrentNode);

            oSymTable->bindingCount -= 1;
            return pvValue;
        }
        psPrevNode = psCurrentNode;
    }

    return NULL;
}

void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, void *pvValue,
                                  void *pvExtra),
                  const void *pvExtra)
{
    struct SymTableNode *psCurrentNode;
    size_t bucketIndex;

    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    for (bucketIndex = 0;
         bucketIndex < oSymTable->bucketCount;
         bucketIndex++)
    {

        for (psCurrentNode = oSymTable->buckets[bucketIndex];
             psCurrentNode != NULL;
             psCurrentNode = psCurrentNode->psNextNode)
        {
            (*pfApply)(psCurrentNode->pcKey,
                       (void *)psCurrentNode->pvValue, (void *)pvExtra);
        }
    }
}
