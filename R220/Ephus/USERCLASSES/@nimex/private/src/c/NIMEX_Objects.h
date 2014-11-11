/**
 * @file NIMEX_Objects.h
 *
 * @brief Function declarations for NIMEX_Objects.c and typesafety macros.
 *
 * Rudimentary typesafe implementations of structure methods to act as objects
 * (constructors, destructors, accessors, mutators, etc).
 *
 * It is expected, although not explicitly required, that when using generic
 * structures in context, that these functions be used.
 *
 * @see NIMEX_Objects.c
 *
 * @author Timothy O'Connor
 * @date 6/19/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 *
 */
/*
 * NOTES
 *  See TO061207A, TO061407A, TO061907C.
 *
 * CHANGES
 *  TO072607A - Cycle types. -- Tim O'Connor 7/26/07
 *  TO101807C - Added channel list sorting. -- Tim O'Connor 10/18/07
 */
 
#ifndef _NIMEX_OBJECTS_h /* Multiple include protection. */
#define _NIMEX_OBJECTS_h

#include "nimex.h"

/*NIMEX_Event* NIMEX_EveryNEvent_create(NIMEX_TaskDefinition* taskDefinition, int32 everyNsamplesEventType, uInt32 samples);
NIMEX_Event* NIMEX_DoneEvent_create(NIMEX_TaskDefinition* taskDefinition);
NIMEX_Event* NIMEX_TimerEvent_create(NIMEX_TaskDefinition* taskDefinition);
void NIMEX_Event_destroy(void* event);*/

//For type safety at compile time, these accessor/mutator methods should be used. -- TO061207A
/**
 * @brief Typesafe call to NIMEX_linkedList_create for lists of NIMEX_ChannelList type.
 */
#define NIMEX_ChannelList_create() ((NIMEX_ChannelList *)NIMEX_linkedList_create())
/**
 * @brief Typesafe call to NIMEX_linkedList_first for lists of NIMEX_ChannelList type.
 */
#define NIMEX_ChannelList_first(list) ((NIMEX_ChannelList *)NIMEX_linkedList_first(list))
/**
 * @brief Typesafe call to NIMEX_linkedList_next for lists of NIMEX_ChannelList type.
 */
#define NIMEX_ChannelList_next(list) ((NIMEX_ChannelList *)NIMEX_linkedList_next(list))
/**
 * @brief Typesafe call to NIMEX_linkedList_value for lists of NIMEX_ChannelList type.
 */
#define NIMEX_ChannelList_value(list) ((NIMEX_ChannelDefinition *)NIMEX_linkedList_value(list))
void NIMEX_ChannelList_append(NIMEX_ChannelList* list, NIMEX_ChannelDefinition* item);
NIMEX_ChannelList* NIMEX_ChannelList_sort(NIMEX_ChannelList* list);


/**
 * @brief Typesafe call to NIMEX_linkedList_create for lists of NIMEX_CallbackSet type.
 */
#define NIMEX_CallbackSet_create() ((NIMEX_CallbackSet *)NIMEX_linkedList_create())
/**
 * @brief Typesafe call to NIMEX_linkedList_first for lists of NIMEX_CallbackSet type.
 */
#define NIMEX_CallbackSet_first(list) ((NIMEX_CallbackSet *)NIMEX_linkedList_first(list))
/**
 * @brief Typesafe call to NIMEX_linkedList_next for lists of NIMEX_CallbackSet type.
 */
#define NIMEX_CallbackSet_next(list) ((NIMEX_CallbackSet *)NIMEX_linkedList_next(list))
/**
 * @brief Typesafe call to NIMEX_linkedList_value for lists of NIMEX_CallbackSet type.
 */
#define NIMEX_CallbackSet_value(list) ((NIMEX_Callback *)NIMEX_linkedList_value(list))
void NIMEX_CallbackSet_destroy(void* list);
void NIMEX_CallbackSet_append(NIMEX_CallbackSet* list, NIMEX_Callback* item);
GCompareFunc NIMEX_CallbackSet_sortCompareFunc;//int (*NIMEX_CallbackSet_sortCompareFunc) (void* a, void* b);

//TO061407A - More typesafety, for the new NIMEX_CallbackMap type.
/**
 * @brief Typesafe call to NIMEX_hashTable_create for tables of NIMEX_CallbackMap type.
 */
#define NIMEX_CallbackMap_create() ((NIMEX_CallbackMap *)NIMEX_hashTable_create(&NIMEX_linkedList_destroy))
void NIMEX_CallbackMap_insert(NIMEX_CallbackMap* table, const char* eventName, NIMEX_Callback* item);
void NIMEX_CallbackMap_removeByName(NIMEX_CallbackMap* table, const char* eventName, const char* name);
/**
 * @brief Typesafe call to NIMEX_hashTable_lookup for tables of NIMEX_CallbackMap type.
 */
#define NIMEX_CallbackMap_lookup(table, name) ((NIMEX_CallbackSet *)NIMEX_hashTable_lookup(table, name))
NIMEX_Callback* NIMEX_Callback_create(const char* name, const int32 priority, const mxArray* callbackToMatlab);
void NIMEX_Callback_destroy(void* callback);

//TO072607A - Cycle types.
NIMEX_DataSource* NIMEX_DataSource_create(int dataSourceType, const void* dataBuffer, const mxArray* callback, uInt64 dataBufferSize, const char* name);
void NIMEX_DataSource_destroy(void* dataSource);
void NIMEX_DataSource_updateBufferByCallback(NIMEX_TaskDefinition* taskDefinition, NIMEX_DataSource* dataSourceStruct);
void NIMEX_DataSource_setBufferDirect(NIMEX_DataSource* dataSourceStruct, void* dataBuffer, uInt64 dataBufferSize);//No copy is done here.
void NIMEX_DataSource_setBufferFromMxArray(NIMEX_DataSource* dataSourceStruct, const mxArray* callback);
void NIMEX_DataSource_clearBuffer(NIMEX_DataSource* dataSourceStruct);
mxArray* NIMEX_DataSource_toMxArray(NIMEX_DataSource* dataSourceStruct);
void NIMEX_DataSource_fromMxArray(NIMEX_DataSource** dataSource, const mxArray* matlabData, const char* name);

NIMEX_CyclePosition* NIMEX_CyclePosition_create(NIMEX_TaskDefinition**  taskDefinitions, int taskCount);
void NIMEX_CyclePosition_destroy(void* cyclePosition);
void NIMEX_CyclePosition_setDataSource(const char* channelName, NIMEX_DataSource* dataSource);//Replace, if one already exists.

NIMEX_Cycle* NIMEX_Cycle_create();
void NIMEX_Cycle_destroy(void* cycle);
void NIMEX_Cycle_start(NIMEX_Cycle* cycle);
void NIMEX_Cycle_stop(NIMEX_Cycle* cycle);

/**
 * @brief Typesafe call to NIMEX_hashTable_create for tables of NIMEX_DataSourceMap type.
 */
#define NIMEX_DataSourceMap_create() ((NIMEX_DataSourceMap*)NIMEX_hashTable_create(&NIMEX_DataSource_destroy))
/**
 * @brief Typesafe call to NIMEX_hashTable_insert for tables of NIMEX_DataSourceMap type.
 */
#define NIMEX_DataSourceMap_insert(dataSourceMap, channelName, dataSource) NIMEX_hashTable_insert(dataSourceMap, channelName, dataSource)
/**
 * @brief Typesafe call to NIMEX_hashTable_lookup for tables of NIMEX_DataSourceMap type.
 */
#define NIMEX_DataSourceMap_lookup(dataSourceMap, channelName) ((NIMEX_DataSource *dataSource)NIMEX_hashTable_lookup(dataSourceMap, channelName))
/**
 * @brief Typesafe call to NIMEX_hashTable_remove for tables of NIMEX_DataSourceMap type.
 */
#define NIMEX_DataSourceMap_remove(dataSourceMap, channelName) NIMEX_hashTable_remove(dataSourceMap, channelName)
/**
 * @brief Typesafe call to NIMEX_hashTable_free for tables of NIMEX_DataSourceMap type.
 */
#define NIMEX_DataSourceMap_free(dataSourceMap, channelName) NIMEX_hashTable_free(dataSourceMap, channelName)
/**
 * @brief Typesafe call to NIMEX_hashTable_destroy for tables of NIMEX_DataSourceMap type.
 */
#define NIMEX_DataSourceMap_destroy(dataSourceMap) NIMEX_hashTable_destroy(dataSourceMap)

/**
 * @brief Typesafe call to NIMEX_linkedList_create for tables of NIMEX_CyclePositionList type.
 *
 * @todo Implement C-level cycles.
 */
#define NIMEX_CyclePositionList_create() (NIMEX_CyclePositionList *)NIMEX_linkedList_create())
/**
 * @brief Typesafe call to NIMEX_linkedList_first for tables of NIMEX_CyclePositionList type.
 *
 * @todo Implement C-level cycles.
 */
#define NIMEX_CyclePositionList_first(list) ((NIMEX_CyclePositionList *)NIMEX_linkedList_first(list))
/**
 * @brief Typesafe call to NIMEX_linkedList_next for tables of NIMEX_CyclePositionList type.
 *
 * @todo Implement C-level cycles.
 */
#define NIMEX_CyclePositionList_next(list) ((NIMEX_CyclePositionList *)NIMEX_linkedList_next(list))
/**
 * @brief Typesafe call to NIMEX_linkedList_value for tables of NIMEX_CyclePositionList type.
 *
 * @todo Implement C-level cycles.
 */
#define NIMEX_CyclePositionList_value(list) ((NIMEX_CyclePositionList *)NIMEX_linkedList_value(list))
/**
 * @brief Typesafe call to NIMEX_linkedList_append for tables of NIMEX_CyclePositionList type.
 *
 * @todo Implement C-level cycles.
 */
#define NIMEX_CyclePositionList_append(list, item) NIMEX_linkedList_append(list, item)

//void NIMEX_addCallback(NIMEX_CallbackMap* table, char* eventName, char* callbackName, int priority, mxArray* callbackToMatlab);

NIMEX_ChannelDefinition* NIMEX_getChannelByName(NIMEX_TaskDefinition* taskDefinition, const char* mnemonicName);

void NIMEX_addChannelToList(NIMEX_TaskDefinition* taskDefinition, NIMEX_ChannelDefinition* channelDefinition);

void NIMEX_removeChannelFromList(NIMEX_TaskDefinition* taskDefinition, NIMEX_ChannelDefinition* channelDefinition);

NIMEX_ChannelDefinition* NIMEX_createChannelDefinition(int channelType, char* physicalName);

void NIMEX_destroyCallback(NIMEX_Callback* callback);

void NIMEX_removeChannelDefinitionByName(NIMEX_TaskDefinition* taskDefinition, char* physicalName);

void NIMEX_destroyChannelDefinition(NIMEX_ChannelDefinition* channelDefinition);

void NIMEX_destroyTaskDefinition(NIMEX_TaskDefinition* taskDefinition);

void NIMEX_displayCallbackMap(char* prefix, NIMEX_CallbackMap* callbackMap);

void NIMEX_displayCallbackSet(char* prefix, NIMEX_CallbackSet* callbackSet);

void NIMEX_displayCallback(char* prefix, NIMEX_Callback* callback);

#endif /* Multiple include protection. */
