/*--------------------------------------------------------------------*/
/* symtablelist.c                                                     */
/* Author: Ryan Chen                                                  */
/*--------------------------------------------------------------------*/

#include "symtable.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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

/* A SymTable is a structure that points to the first SymTableNode and
   stores the length of the linked list. */
struct SymTable
{
     /* address of first SymTableNode */
     struct SymTableNode *psFirstNode;

     /* stores length of the list of SymTableNodes */
     size_t length;
};

SymTable_T SymTable_new(void)
{
     SymTable_T oSymTable;

     oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));

     if (oSymTable == NULL)
          return NULL;

     oSymTable->psFirstNode = NULL;
     oSymTable->length = 0;
     return oSymTable;
}

void SymTable_free(SymTable_T oSymTable)
{
     struct SymTableNode *psCurrentNode;
     struct SymTableNode *psNextNode;

     assert(oSymTable != NULL);

     for (psCurrentNode = oSymTable->psFirstNode;
          psCurrentNode != NULL;
          psCurrentNode = psNextNode)
     {
          psNextNode = psCurrentNode->psNextNode;
          free((void *)psCurrentNode->pcKey);
          free(psCurrentNode);
     }

     free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable)
{
     assert(oSymTable != NULL);
     return oSymTable->length;
}

int SymTable_put(SymTable_T oSymTable,
                 const char *pcKey, const void *pvValue)
{
     char *keyCopy;
     struct SymTableNode *psNewNode;

     assert(oSymTable != NULL);
     assert(pcKey != NULL);

     if (SymTable_contains(oSymTable, pcKey))
          return 0;

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

     psNewNode->psNextNode = oSymTable->psFirstNode;
     oSymTable->psFirstNode = psNewNode;
     oSymTable->length += 1;

     return 1;
}

void *SymTable_replace(SymTable_T oSymTable,
                       const char *pcKey, const void *pvValue)
{
     struct SymTableNode *psCurrentNode;
     void *oldValue;

     assert(oSymTable != NULL);
     assert(pcKey != NULL);

     for (psCurrentNode = oSymTable->psFirstNode;
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

     assert(oSymTable != NULL);
     assert(pcKey != NULL);

     for (psCurrentNode = oSymTable->psFirstNode;
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

     assert(oSymTable != NULL);
     assert(pcKey != NULL);

     for (psCurrentNode = oSymTable->psFirstNode;
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
     struct SymTableNode *psCurrentNode = oSymTable->psFirstNode;
     struct SymTableNode *psPrevNode = NULL;
     void *pvValue;

     assert(oSymTable != NULL);
     assert(pcKey != NULL);

     for (psCurrentNode = oSymTable->psFirstNode;
          psCurrentNode != NULL;
          psCurrentNode = psCurrentNode->psNextNode)
     {
          if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
          {               
               pvValue = (void *)psCurrentNode->pvValue;

               if (psPrevNode == NULL)
               {
                    oSymTable->psFirstNode = psCurrentNode->psNextNode;
               }
               else
               {
                    psPrevNode->psNextNode = psCurrentNode->psNextNode;
               }

               free((void *)psCurrentNode->pcKey);
               free(psCurrentNode);

               oSymTable->length -= 1;
               return pvValue;
          }
          psPrevNode = psCurrentNode;
     }

     return NULL;
}

void SymTable_map(SymTable_T oSymTable,
     void (*pfApply)(const char *pcKey, void *pvValue, 
     void *pvExtra), const void *pvExtra)
{
     struct SymTableNode *psCurrentNode;

     assert(oSymTable != NULL);
     assert(pfApply != NULL);

     for (psCurrentNode = oSymTable->psFirstNode;
          psCurrentNode != NULL;
          psCurrentNode = psCurrentNode->psNextNode)
     {
          (*pfApply)(psCurrentNode->pcKey,
                     (void *)psCurrentNode->pvValue, (void *)pvExtra);
     }
}
