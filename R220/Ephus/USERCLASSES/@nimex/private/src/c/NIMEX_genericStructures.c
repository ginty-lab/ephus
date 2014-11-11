/**
 * @file NIMEX_genericStructures.c
 *
 * @brief Generic data structure implementations (such as linked lists and hash tables).
 *
 * @author Timothy O'Connor
 * @date 1/6/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 *
 */
/*
 * CHANGES
 *  TO040207B - Created NIMEX_linkedList_display, for diagnostic purposes. -- Tim O'Connor 4/2/07
 *  TO040207C - Created NIMEX_linkedList_destroy. -- Tim O'Connor 4/2/07
 *  TO061207A - Port basic structures to glib. -- Tim O'Connor 6/12/07
 *  TO071107A - C89 compatibility for MSVC's cl compiler. -- Tim O'Connor 7/11/07
 *  TO072607B - Make sure not to append if the list is empty, just insert at position 0. -- Tim O'Connor 7/26/07
 *  TO101007A - Fixed the condition on which to directly insert into a linked list, so the first element will be non-NULL. -- Tim O'Connor 10/10/07
 *  TO101507A - Fleshed out implementation of NIMEX_hashTable_display, which had been a stub until now. -- Tim O'Connor 10/15/07
 *  TO101807D - Issue errors on NULL.
 *  JL011408A: use g_hash_table_new to replace g_hash_table_new_full because there is no valueDestructor yet.
 */
#include <string.h>
#include <math.h>
#include "nimex.h"
#include "glib/ghash.h"

#define NIMEX_GENERICSTRUCTURES_MACRO_HASH_2_INDEX(hash, capacity) (hash % NIMEX_MACRO_FLOAT_2_ROUNDED_LONG(capacity))

/**
 * @brief Doubly linked list constructor.
 *
 * @return A new linked list.
 */
NIMEX_linkedList* NIMEX_linkedList_create(void)
{
    NIMEX_linkedList* list;
    NIMEX_MACRO_VERBOSE_3("NIMEX_genericStructures/NIMEX_linkedList_create: ...\n")
    list = g_list_alloc();
    

    NIMEX_addDestroyableMemorySegment(list, &NIMEX_linkedList_destroy, TRUE);//Because glib is doing the allocation, it must be pushed into memory management.

    NIMEX_MACRO_VERBOSE_3("NIMEX_genericStructures/NIMEX_linkedList_create: @%p\n", list)

    return list;
}

/**
 * @brief Get the next node in a list.
 *
 * @return The next node in the linked list. NULL if at the end of the list.
 */
NIMEX_linkedList* NIMEX_linkedList_next(NIMEX_linkedList* list)
{
    NIMEX_MACRO_VERBOSE_3("NIMEX_genericStructures/NIMEX_linkedList_next: @%p\n", list)
    return (NIMEX_linkedList *)g_list_next(list);
}

/**
 * @brief Get the next node in a list.
 *
 * @arg <tt>list</tt> - The current node in the list.
 *
 * @return The next node in the linked list. NULL if at the end of the list.
 *
 * @callgraph
 * @callergraph
 */
NIMEX_linkedList* NIMEX_linkedList_end(NIMEX_linkedList* list)
{
    NIMEX_MACRO_VERBOSE_3("NIMEX_genericStructures/NIMEX_linkedList_end: @%p\n", list)
    return (NIMEX_linkedList *)g_list_last(list);
}

/**
 * @brief Get the first node in a list.
 *
 * @arg <tt>list</tt> - Any node in the list.
 *
 * @return The first node in the linked list. NULL if the list is empty.
 *
 * @callgraph
 * @callergraph
 */
NIMEX_linkedList* NIMEX_linkedList_first(NIMEX_linkedList* list)
{
    NIMEX_MACRO_VERBOSE_3("NIMEX_genericStructures/NIMEX_linkedList_first: @%p\n", list)

    //Make sure that the first node, which points to NULL is not returned.
    list = (NIMEX_linkedList *)g_list_first(list);//TO072607B
    if (g_list_length(list) >= 2)
    {
        //TO072607B - Still check for a NULL first element, although this should no longer come up.
        if (g_list_nth(list, 0) == NULL)
            return (NIMEX_linkedList *)g_list_next(list);
        else
            return list;
    }
    else
        return list;
}

/**
 * @brief Append a node to a list.
 *
 * The new node is placed at the end of the list.
 *
 * @arg <tt>list</tt> - Any node in the list.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_linkedList_append(NIMEX_linkedList* list, void* item)
{
    NIMEX_MACRO_VERBOSE_3("NIMEX_genericStructures/NIMEX_linkedList_append: @%p to @%p\n", item, list)

    //TO072607B: Make sure not to append if the list is empty, just insert. -- Tim O'Connor 7/26/07
    //TO101007A: Fixed the condition on which to directly insert, so the first element will be non-NULL. -- Tim O'Connor 10/10/07
    if ((g_list_length(list) == 0) || (g_list_nth_data(list, 0) == NULL))
{
NIMEX_MACRO_VERBOSE_4("g_list_length(@%p): %d\n", list, g_list_length(list))
NIMEX_MACRO_VERBOSE_4("g_list_nth_data(@%p, 0): @%p\n", list, g_list_nth_data(list, 0))
NIMEX_MACRO_VERBOSE_4("g_list_insert(@%p, @%p, 0)\n", list, item)
        g_list_insert(list, item, 0);
}
    else
{
NIMEX_MACRO_VERBOSE_4("g_list_append(@%p, @%p)\n", list, item)
        g_list_append(list, item);
}
NIMEX_MACRO_VERBOSE_4("NIMEX_linkedList_append done!\n");
    return;
}

/**
 * @brief Remove a node from a list.
 *
 * @arg <tt>list</tt> - Any node in the list.
 * @arg <tt>item</tt> - The item to be remoted from the list.
 *
 * @callgraph
 * @callergraph
 */
NIMEX_linkedList* NIMEX_linkedList_remove(NIMEX_linkedList* list, void* item)
{
/*    NIMEX_linkedList* elem;
    NIMEX_MACRO_VERBOSE_3("NIMEX_genericStructures/NIMEX_linkedList_remove: @%p from @%p\n", item, list)
    elem = g_list_find(list, item);
    if (elem == NULL)
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_genericStructures/NIMEX_linkedList_remove: @%p not found in @%p.\n", item, list)
        return list;
    }
    g_list_remove(list, elem);
    g_list_free_1(elem);*/
    //TO081507A - The automatic freeing of elements did something really odd, the list would just hold a NULL.
    //            Since these lists are mainly used for "objects" and they have "destructors", freeing isn't necessary here.
    g_list_remove(list, item);

    return list;
}

/**
 * @brief Remove a node from a list and deallocate its memory.
 *
 * @arg <tt>list</tt> - Any node in the list.
 * @arg <tt>item</tt> - The item to be remoted from the list.
 *
 * @callgraph
 * @callergraph
 */
NIMEX_linkedList* NIMEX_linkedList_free(NIMEX_linkedList* list, void* item)
{
    NIMEX_linkedList* elem;    
    NIMEX_MACRO_VERBOSE_3("NIMEX_genericStructures/NIMEX_linkedList_free: %d, %d\n", list, item)
    list = (NIMEX_linkedList *)NIMEX_linkedList_remove(list, item);//Remove the item from the list, as normal.
    NIMEX_freePersistentMemorySegment(item);//Free the item from memory, for convenience.
    
    return list;
}

/**
 * @brief Determine if a list is empty.
 *
 * @arg <tt>list</tt> - Any node in the list.
 *
 * @return 1 if the list is empty, 0 otherwise.
 *
 * @callgraph
 * @callergraph
 */
int NIMEX_linkedList_isempty(NIMEX_linkedList* list)
{
    int empty = 0;

    NIMEX_MACRO_VERBOSE_3("NIMEX_genericStructures/NIMEX_linkedList_isempty: @%p\n", list)

    if (g_list_length(g_list_first(list)) == 0)
      empty = 1;

    return empty;
}

/**
 * @brief Get the length of a list.
 *
 * @arg <tt>list</tt> - Any node in the list.
 *
 * @return The number of items in the list.
 *
 * @callgraph
 * @callergraph
 */
int NIMEX_linkedList_length(NIMEX_linkedList* list)
{
    int length = 0;

    NIMEX_MACRO_VERBOSE_3("NIMEX_genericStructures/NIMEX_linkedList_length: @%p\n", list)
    
    length = g_list_length(g_list_first(list)) - 1;//Subtract 1 for the first node, which points to NULL.

    return length;
}

/**
 * @brief Display a list to standard out.
 *
 * @arg <tt>list</tt> - Any node in the list.
 *
 * @callgraph
 * @callergraph
 */
//TO040207B
void NIMEX_linkedList_display(const char* prefix, NIMEX_linkedList* list)
{
    NIMEX_linkedList* root;

    root = g_list_first(list);
    mexPrintf("%s NIMEX_linkedList_display: @%p (root: @%p)\n", prefix, list, root);

    list = root;
    while ((list != NULL))
    {
        mexPrintf("%s                           @%p->@%p\n", prefix, list, g_list_nth_data(list, 0));
        list = NIMEX_linkedList_next(list);
    }
 }

/**
 * @brief Destroys a list.
 *
 * Frees all memory used in a list, frees each node.
 *
 * @arg <tt>list</tt> - Any node in the list.
 *
 * @callgraph
 * @callergraph
 */
 //TO040207C
void NIMEX_linkedList_destroy(NIMEX_linkedList* list)
{
    NIMEX_linkedList* root;

    root = g_list_first(list);
    if (root != list)
    {
      NIMEX_MACRO_VERBOSE("NIMEX_linkedList_destroy: Destroy requested from a non-root node.\n"
                          "                          list: @%p\n"
                          "                          root: @%p\n", list, root)
    }
    NIMEX_MACRO_VERBOSE_2("NIMEX_linkedList_destroy: @%p\n", list)

    g_list_free(list);
    list = NULL; //JL011708A: Add this to avoid double free
    NIMEX_MACRO_VERBOSE_3("NIMEX_hashTable_destroy done ((@%p)\n", list)
    return;
}

/**
 * @brief Retrieves a value in a list.
 *
 * Gets the value from the node.
 *
 * @arg <tt>list</tt> - The node from which to retrieve a value.
 *
 * @return The value stored in the node, as requested. When iterating, it is best to check for NULL.
 *
 * @callgraph
 * @callergraph
 */
//TO061207A - New function.
void* NIMEX_linkedList_value(NIMEX_linkedList* list)
{
    NIMEX_MACRO_VERBOSE_3("NIMEX_linkedList_value: list=@%p\n", list)
    return g_list_nth_data(list, 0);
}

/**
 * @brief Creates a hash table.
 *
 * @arg <tt>valueDestructor</tt> - The type-specific destructor for the values to be stored in the table.
 *
 * @return A newly initialized hash table.
 *
 * @callgraph
 * @callergraph
 */
NIMEX_hashTable* NIMEX_hashTable_create(NIMEX_destructor valueDestructor)
{
    NIMEX_hashTable* table;
    
    NIMEX_MACRO_VERBOSE_3("NIMEX_hashTable_create()\n");
    
    
    //JL011408A: use g_hash_table_new to replace g_hash_table_new_full
    //The key destructor just needs to free the strings. The value destructor must be specified based on the type.
//     table = g_hash_table_new_full(&g_str_hash, &g_str_equal, &NIMEX_simpleDestructor, valueDestructor);
    table = g_hash_table_new(&g_str_hash, &g_str_equal);
     
    NIMEX_addDestroyableMemorySegment(table, &NIMEX_hashTable_destroy, TRUE);//Because glib is doing the allocation, it must be pushed into memory management.
    
    return table;
}

/**
 * @brief Insert a value into a hash table.
 *
 * @arg <tt>table</tt> - The table into which to insert a value.
 * @arg <tt>key</tt> - The key associated with the value being inserted.
 * @arg <tt>value</tt> - The value to be inserted.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_hashTable_insert(NIMEX_hashTable* table, const char* key, void* value)
{
    //TO101807D
    if (table == NULL)
        mexErrMsgTxt("NIMEX_hashTable_insert: table = NULL");
    
    NIMEX_MACRO_VERBOSE_3("NIMEX_hashTable_insert(@%p, \"%s\", @%p)\n", table, key, value)
    g_hash_table_insert(table, g_strdup(key), value);
    return;
}

/**
 * @brief Remove a value from a hash table.
 *
 * @arg <tt>table</tt> - The table from which to remove a value.
 * @arg <tt>key</tt> - The key associated with the value being removed.
 *
 * @callgraph
 * @callergraph
 */
void* NIMEX_hashTable_remove(NIMEX_hashTable* table, const char* key)
{
    void* actualKey = NULL;
    void* value = NULL;
    
    //TO101807D
    if (table == NULL)
        mexErrMsgTxt("NIMEX_hashTable_remove: table = NULL");

    NIMEX_MACRO_VERBOSE_3("NIMEX_hashTable_remove(@%p, \"%s\")\n", table, key)    
    if (g_hash_table_lookup_extended(table, key, actualKey, value))
    {
        g_hash_table_remove(table, key);
        NIMEX_MACRO_FREE(actualKey);
    }
    
    return value;
}

/**
 * @brief Remove a value from a hash table and free it from memory.
 *
 * @arg <tt>table</tt> - The table from which to remove a value.
 * @arg <tt>key</tt> - The key associated with the value being removed.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_hashTable_free(NIMEX_hashTable* table, const char* key)
{
    void* actualKey = NULL;
    void* value = NULL;
    
    //TO101807D
    if (table == NULL)
        mexErrMsgTxt("NIMEX_hashTable_free: table = NULL");

    NIMEX_MACRO_VERBOSE_3("NIMEX_hashTable_free(@%p, \"%s\")\n", table, key)
    if (g_hash_table_lookup_extended(table, key, actualKey, value))
    {
        g_hash_table_remove(table, key);
        NIMEX_MACRO_FREE(actualKey);
        NIMEX_MACRO_FREE(value);
    }

    return;
}

/**
 * @brief Determine if a table is empty.
 *
 * @arg <tt>table</tt> - The table to inspect.
 *
 * @return 1 if empty, 0 otherwise.
 *
 * @callgraph
 * @callergraph
 */
int NIMEX_hashTable_isempty(NIMEX_hashTable* table)
{
    //TO101807D
    if (table == NULL)
        mexErrMsgTxt("NIMEX_hashTable_isempty: table = NULL");
    
    return (g_hash_table_size(table) == 0);
}

/**
 * @brief Determine the size of a table.
 *
 * @arg <tt>table</tt> - The table to inspect.
 *
 * @return The number of values stored in the table.
 *
 * @callgraph
 * @callergraph
 */
int NIMEX_hashTable_size(NIMEX_hashTable* table)
{
    //TO101807D
    if (table == NULL)
        mexErrMsgTxt("NIMEX_hashTable_size: table = NULL");
    
    return g_hash_table_size(table);
}

/**
 * @brief Iterator for displaying a table elements to standard out.
 *
 * @arg <tt>key</tt> - The key to display.
 * @arg <tt>value</tt> - The value to display.
 * @arg <tt>prefix</tt> - The prefix to be used, typically for indenting.
 *
 * @callgraph
 * @callergraph
 */
//TO101507A
void NIMEX_hashTable_display_GHFunc(gpointer key, gpointer value, gpointer prefix)
{
    char* prefix2;
    size_t len;
    mexPrintf("%skey=\"%s\" (@%p), value=@%p\n", (char *)prefix, (char *)key, key, value);

    if (value != NULL)
    {
        len = strlen(prefix) + 1;//Add an extra one for the NULL termination.
        prefix2 = calloc(len + 2, sizeof(char));//Add an extra one for the NULL termination.
        prefix2[0] = ' ';
        prefix2[1] = ' ';
        memcpy(&prefix2[2], prefix, len);

        NIMEX_linkedList_display(prefix2, (NIMEX_linkedList *)value);
        
        free(prefix2);
    }
    else
        mexPrintf("  %s - value = NULL", prefix);
    
    return;
}

/**
 * @brief Display a hash table to standard out.
 *
 * @arg <tt>table</tt> - The table to display.
 * @arg <tt>prefix</tt> - The prefix to be used, typically for indenting.
 *
 * @callgraph
 * @callergraph
 */
//TO101507A
void NIMEX_hashTable_display(const char* prefix, NIMEX_hashTable* table)
{
    char* prefix2;
    size_t len;
    
    //TO101807D
    if (table == NULL)
        mexErrMsgTxt("NIMEX_hashTable_display: table = NULL");
    
    len = strlen(prefix) + 1;//Add an extra one for the NULL termination.
    prefix2 = calloc(len + 2, sizeof(char));
    prefix2[0] = ' ';
    prefix2[1] = ' ';
    memcpy(&prefix2[2], prefix, len);

    mexPrintf("%sNIMEX_hashTable - @%p\n", prefix, table);
    g_hash_table_foreach(table, NIMEX_hashTable_display_GHFunc, prefix2);
    
    free(prefix2);
    
    return;
}

/**
 * @brief Destroys a hash table.
 *
 * Deeply frees all memory, calling the configured value destructor on each item.
 *
 * @arg <tt>table</tt> - The table to destroy.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_hashTable_destroy(NIMEX_hashTable* table)
{
    //TO101807D
    if (table == NULL)
        mexPrintf("Warning - NIMEX_hashTable_destroy: table = NULL");
    
    NIMEX_MACRO_VERBOSE_3("NIMEX_hashTable_destroy(@%p)\n", table)
    g_hash_table_destroy(table);
    NIMEX_MACRO_VERBOSE_3("NIMEX_hashTable_destroy done ((@%p)\n", table)
    return;
}

/**
 * @brief Retrieve a value from the table.
 *
 * @arg <tt>table</tt> - The table in which to perform a lookup.
 * @arg <tt>key</tt> - The key to look up.
 *
 * @return The value associated with the key, or NULL if not found.
 *
 * @callgraph
 * @callergraph
 */
void* NIMEX_hashTable_lookup(NIMEX_hashTable* table, const char* key)
{
    //TO101807D
    if (table == NULL)
        mexErrMsgTxt("NIMEX_hashTable_lookup: table = NULL");

    NIMEX_MACRO_VERBOSE_3("NIMEX_hashTable_lookup(@%p, \"%s\")\n", table, key)
    return g_hash_table_lookup(table, key);
}

/**
 * @brief Hash function for a string.
 *
 * @arg <tt>str</tt> - The string to be hashed.
 *
 * @return A hash value, based on the input string.
 *
 * @callgraph
 * @callergraph
 */
unsigned int NIMEX_hashTable_StringHashFcn(const char* str)
{
    unsigned int hash = 0;
    int i = 0;//TO071107A
  
    for (i = 0; i < strlen(str); i++)
    {
        hash = str[i] + (hash << 6) + (hash << 16) - hash;
    }
    NIMEX_MACRO_VERBOSE_4("NIMEX_genericStructures - NIMEX_hashTable_StringHashFcn: '%s' -> %u\n", str, hash)
    return hash;
}

void linkedListTestFcn(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    int items[] = {0, 1, 2, 3, 4, 5};
    NIMEX_linkedList* list;
    int i = 0;//TO071107A

    NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - LinkedList test: creating new list...\n")
    list = NIMEX_linkedList_create();

    for (i = 0; i < 5; i++)
    {
        NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - LinkedList test: appending %d @ %d...\n", items[i], items + i)
        NIMEX_linkedList_append(list, items + i);
    }

    NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - LinkedList test: status - \n")
    NIMEX_MACRO_VERBOSE("                                NIMEX_linkedList_isempty(@%p) = %d\n", (int)list, NIMEX_linkedList_isempty(list))
    NIMEX_MACRO_VERBOSE("                                NIMEX_linkedList_length(@%p) = %d\n", (int)list, NIMEX_linkedList_length(list))
    NIMEX_MACRO_VERBOSE("                                NIMEX_linkedList_next(@%p) = @%p\n", (int)list, (int)NIMEX_linkedList_next(list))
    
    for (i = 0; i < 5; i++)
    {
        NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - LinkedList test: removing %d @ %d from @%p...\n", items[i], items + i, (int)list)
        list = NIMEX_linkedList_remove(list, items + i);
        NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - LinkedList test: removed %d from @%p.\n", items[i], (int)list)
    }

    NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - LinkedList test: status - \n")
    NIMEX_MACRO_VERBOSE("                                NIMEX_linkedList_isempty(@%p) = %d\n", (int)list, NIMEX_linkedList_isempty(list))
    NIMEX_MACRO_VERBOSE("                                NIMEX_linkedList_length(@%p) = %d\n", (int)list, NIMEX_linkedList_length(list))
    NIMEX_MACRO_VERBOSE("                                NIMEX_linkedList_next(@%p) = @%p\n", (int)list, (int)NIMEX_linkedList_next(list))
    
    for (i = 0; i < 5; i++)
    {
        NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - LinkedList test: appending %d @ %d...\n", items[i], items + i)
        NIMEX_linkedList_append(list, items + i);
    }

    NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - LinkedList test: status - \n")
    NIMEX_MACRO_VERBOSE("                                NIMEX_linkedList_isempty(@%p) = %d\n", (int)list, NIMEX_linkedList_isempty(list))
    NIMEX_MACRO_VERBOSE("                                NIMEX_linkedList_length(@%p) = %d\n", (int)list, NIMEX_linkedList_length(list))
    NIMEX_MACRO_VERBOSE("                                NIMEX_linkedList_next(@%p) = @%p\n", (int)list, (int)NIMEX_linkedList_next(list))
    
    for (i = 4; i > -1; i--)
    {
        NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - LinkedList test: removing %d @ %d from @%p...\n", items[i], items + i, (int)list)
        list = NIMEX_linkedList_remove(list, items + i);
        NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - LinkedList test: removed %d from @%p.\n", items[i], (int)list)
    }

    NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - LinkedList test: status - \n")
    NIMEX_MACRO_VERBOSE("                                NIMEX_linkedList_isempty(@%p) = %d\n", (int)list, NIMEX_linkedList_isempty(list))
    NIMEX_MACRO_VERBOSE("                                NIMEX_linkedList_length(@%p) = %d\n", (int)list, NIMEX_linkedList_length(list))
    NIMEX_MACRO_VERBOSE("                                NIMEX_linkedList_next(@%p) = @%p\n", (int)list, (int)NIMEX_linkedList_next(list))
    
    NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - LinkedList test: FINISHED\n")
    
    return;
 }

 void hashTableTestFcn(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
 {
/*     NIMEX_hashTable* table;
     float capacity = 10.0;
     float loadFactorThreshold = 0.75;
     float resizeFactor = 2.0;
     char** strings = NULL;
*/
     /*char* str;
     unsigned int hash = NULL;
     int row = -1;

     for (int i = 0; i < nrhs; i += 2)
     {
         str = mxArrayToString(prhs[i]);
         hash = NIMEX_hashTable_StringHashFcn(str);
         row = hash % capacity;
         mexPrintf("  '%s' --> %u @ %d\n", str, hash, row);
     }
     
     return;*/
/*   
     NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - hashTableTestFcn:\n");
     table = NIMEX_hashTable_create(capacity, loadFactorThreshold, resizeFactor);

     strings = (char**)NIMEX_Persistent_Calloc(nrhs, sizeof(char*));
     for (int i = 0; i < nrhs; i++)
     {
         strings[i] = NIMEX_MACRO_MXARRAY_2_PERSISTENT_CHAR(prhs[i]);
         //NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - '%s' @%p\n", strings[i], strings[i])
     }
     
     for (int i = 0; i < nrhs; i += 2)
     {
         NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - hashTableTestFcn: Inserting '%s'@%p:'%s'@%p\n", strings[i], strings[i], strings[i + 1], strings[i + 1])
         NIMEX_hashTable_insert(table, strings[i], strings[i + 1]);
     }
     
     NIMEX_MACRO_VERBOSE_3("NIMEX_genericStructures - hashTableTestFcn: @%p\n", (int)table)
     NIMEX_MACRO_VERBOSE_3("\t\t\ttable->table=@%p\n", (int)table->table)
     NIMEX_MACRO_VERBOSE_3("\t\t\ttable->capacity=%3.1f\n", (int)table->capacity)
     NIMEX_MACRO_VERBOSE_3("\t\t\ttable->loadFactorThreshold=%3.1f\n", (float)table->loadFactorThreshold)
     NIMEX_MACRO_VERBOSE_3("\t\t\ttable->resizeFactor=%3.1f\n", (int)table->resizeFactor)
     NIMEX_MACRO_VERBOSE_3("\t\t\ttable->occupancy=%3.1f\n", (int)table->occupancy)
     
     for (int i = 0; i < nrhs; i += 2)
     {
         strings[i + 1] = (char *)NIMEX_hashTable_retrieve(table, strings[i]);
         NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - hashTableTestFcn: retrieved '%s'@%p:'%s'@%p\n", strings[i], strings[i], strings[i + 1], strings[i + 1])
     }
     
     for (int i = 0; i < nrhs; i += 2)
     {
         NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - hashTableTestFcn: Freeing '%s'@%p...\n", strings[i], strings[i])
         NIMEX_hashTable_free(table, strings[i]);
     }
     
     NIMEX_MACRO_VERBOSE_3("NIMEX_genericStructures - hashTableTestFcn: @%p\n", (int)table)
     NIMEX_MACRO_VERBOSE_3("\t\t\ttable->table=@%p\n", (int)table->table)
     NIMEX_MACRO_VERBOSE_3("\t\t\ttable->capacity=%3.1f\n", (int)table->capacity)
     NIMEX_MACRO_VERBOSE_3("\t\t\ttable->loadFactorThreshold=%3.1f\n", (float)table->loadFactorThreshold)
     NIMEX_MACRO_VERBOSE_3("\t\t\ttable->resizeFactor=%3.1f\n", (int)table->resizeFactor)
     NIMEX_MACRO_VERBOSE_3("\t\t\ttable->occupancy=%3.1f\n", (int)table->occupancy)
     
     NIMEX_freePersistentMemorySegment(table);
     NIMEX_freePersistentMemorySegment(strings);
     
     NIMEX_MACRO_VERBOSE("NIMEX_genericStructures - hashTableTestFcn: FINISHED\n");
     
     return;*/
 }
/*
 void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
 {
     //linkedListTestFcn(nlhs, plhs, nrhs, prhs);
     //hashTableTestFcn(nlhs, plhs, nrhs, prhs);
     mexPrintf("NIMEX_genericStructures\n");
     
     return;
 }*/
