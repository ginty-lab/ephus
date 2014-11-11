/**
 * @file NIMEX_genericStructures.h
 *
 * @brief Function declarations for NIMEX_genericStructures.c.
 *
 * @see NIMEX_genericStructures.c
 *
 * @author Timothy O'Connor
 * @date 1/6/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 *
 */
/*
 * NIMEX_genericStructures.h
 *
 * Generic data structure definitions.
 *
 * NIMEX_linkedList
 *  typedef struct
 *  {
 *      void* root;//Kept for easy record-keeping and consistency.
 *      void* ptr;
 *      void* next;
 *  } NIMEX_linkedList;
 *
 *  NIMEX_linkedList* _list NIMEX_linkedList_create(void)
 *      _list - A newly allocated linked list.
 *
 *  void NIMEX_linkedList_append(NIMEX_linkedList* list, void* item)
 *      list - The linked list to be appended to.
 *      item - The item to append to the list.
 *
 *  void NIMEX_linkedList_remove(NIMEX_linkedList* list, void* item)
 *      list - The linked list to remove from.
 *      item - The item to remove from the list.
 *
 *  void NIMEX_linkedList_free(NIMEX_linkedList* list, void* item)
 *      list - The linked list to remove from.
 *      item - The item to remove from the list and from active memory.
 *
 *  NIMEX_linkedList* _list NIMEX_linkedList_next(NIMEX_linkedList* list)
 *      list - The list to step.
 *      _list - The stepped list.
 *
 *  NIMEX_linkedList* _list NIMEX_linkedList_end(NIMEX_linkedList* list)
 *      list - The list to iterate over.
 *      _list - The last element in the list.
 *
 *  int _bool NIMEX_linkedList_isempty(NIMEX_linkedList* list)
 *      list - The list to check the state of.
 *      _bool - 0 if the list is empty, non-zero otherwise.
 *
 *  int _length NIMEX_linkedList_length(NIMEX_linkedList* list)
 *      list - The list to determine the length of.
 *      _length - The length of the list.
 *
 *  void NIMEX_linkedList_display(const char* prefix, NIMEX_linkedList* list)
 *      list - The list to be displayed.
 *      prefix - A string prepended to each line in the display.
 *
 *  void NIMEX_linkedList_destroy(NIMEX_linkedList* list)
 *      list - The list to be distroyed.
 *
 *  void* _ptr NIMEX_linkedList_value(NIMEX_linkedList* list)
 *      list - The list element to be accessed.
 *      _ptr - The value of the list at the current element.
 *
 *  unsigned long _hash NIMEX_hashTable_StringHashFcn(const char* str)
 *      str - A string to be hashed.
 *      _hash - The generated hash value.
 *
 *  int _bool NIMEX_hashTable_StringCompareFcn(const char* str1, const char* str2)
 *      str1 - String (key) 1.
 *      str2 - String (key) 2.
 *      _bool - 0 if not a match, non-zero otherwise.
 *
 * CHANGES
 *  TO061207A: Port basic structures to glib. -- Tim O'Connor 6/12/07
 *  TO101807C: Added sorting. -- Tim O'Connor 10/18/07
 *
 * Created
 *  Timothy O'Connor 1/6/07
 *
 * Copyright
 *  Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007
 *
 */

#ifndef _NIMEX_GENERIC_STRUCTURES_h /* Multiple include protection. */
#define _NIMEX_GENERIC_STRUCTURES_h

#include "nimex.h"

//TO061207A
typedef GList NIMEX_linkedList;

NIMEX_linkedList* NIMEX_linkedList_create(void);

void NIMEX_linkedList_append(NIMEX_linkedList* list, void* item);

NIMEX_linkedList* NIMEX_linkedList_remove(NIMEX_linkedList* list, void* item);

NIMEX_linkedList* NIMEX_linkedList_free(NIMEX_linkedList* list, void* item);

NIMEX_linkedList* NIMEX_linkedList_next(NIMEX_linkedList* list);

NIMEX_linkedList* NIMEX_linkedList_end(NIMEX_linkedList* list);

NIMEX_linkedList* NIMEX_linkedList_first(NIMEX_linkedList* list);

int NIMEX_linkedList_isempty(NIMEX_linkedList* list);

int NIMEX_linkedList_length(NIMEX_linkedList* list);

void NIMEX_linkedList_display(const char* prefix, NIMEX_linkedList* list);

void NIMEX_linkedList_destroy(NIMEX_linkedList* list);

void* NIMEX_linkedList_value(NIMEX_linkedList* list);//TO061207A - New function.

//TO101807C
#define NIMEX_linkedList_sort(list, compare_func) (NIMEX_linkedList *)g_list_sort(list, compare_func)

typedef GHashTable NIMEX_hashTable;

NIMEX_hashTable* NIMEX_hashTable_create(NIMEX_destructor valueDestructor);

void NIMEX_hashTable_insert(NIMEX_hashTable* table, const char* key, void* value);

void* NIMEX_hashTable_remove(NIMEX_hashTable* table, const char* key);

void NIMEX_hashTable_free(NIMEX_hashTable* table, const char* key);

int NIMEX_hashTable_isempty(NIMEX_hashTable* table);

int NIMEX_hashTable_size(NIMEX_hashTable* table);

void NIMEX_hashTable_display(const char* prefix, NIMEX_hashTable* table);

void NIMEX_hashTable_destroy(NIMEX_hashTable* table);

void* NIMEX_hashTable_lookup(NIMEX_hashTable* table, const char* key);

unsigned int NIMEX_hashTable_StringHashFcn(const char* str);

//#define NIMEX_hashTable_StringCompareFcn (!(strcmp(__VA_ARGS__)))

#endif /* End multiple include protection. */
