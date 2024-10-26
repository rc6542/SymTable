/*--------------------------------------------------------------------*/
/* symtable.h                                                         */
/* Author: Ryan Chen                                                  */
/*--------------------------------------------------------------------*/

#ifndef symtable
#define symtable
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* A SymTable_T object is a last-in-first-out collection of bindings. */
typedef struct SymTable *SymTable_T;

/* Return a new SymTable_T object, or NULL if insufficient memory is
   available. */
SymTable_T SymTable_new(void);

/* Free all memory associated with oSymTable. */
void SymTable_free(SymTable_T oSymTable);

/* Returns the number of bindings in the symbol table oSymTable. */ 
size_t SymTable_getLength(SymTable_T oSymTable);

/* Adds a new key-value pair, pcKey and pcValue, to oSymTable if pcKey 
   doesn't exist in oSymTable. Otherwise, leaves oSymTable unchanged. 
   Returns 1 if successful, 0 if the key already exists. */ 
int SymTable_put(SymTable_T oSymTable,
     const char *pcKey, const void *pvValue);

/* Replaces the old value associated with pcKey in oSymTable if it 
   exists with pvValue. Otherwise, leaves oSymTable unchanged. Returns 
   old value of pcKey, or NULL if the key does not exist. */ 
void *SymTable_replace(SymTable_T oSymTable,
     const char *pcKey, const void *pvValue);

/* Checks if pcKey exists in oSymTable. Returns 1 if the key exists, 0 
   otherwise. */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

/* Gets the value associated with pcKey in oSymTable. Returns the value
   associated with pcKey, or NULL if the key does not exist. */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/* Removes a binding with key pcKey in oSymTable if it exists. 
   Otherwise, leaves oSymTable unchanged. Returns the value of the 
   binding, or NULL if the key does not exist. */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/* Applies function pfApply to each binding in oSymTable, passing 
   pvExtra as an extra parameter.*/
void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra);
  
#endif