/**
 * @file NIMEX_Objects.c
 *
 * @brief  Rudimentary typesafe implementations of structure methods to act as objects
 * (constructors, destructors, accessors, mutators, etc).
 *
 * It is expected, although not explicitly required, that when using generic
 * structures in context, that these functions be used.
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
 *  TO061907E: Replaced everyNCallbacks and doneCallbacks with eventMap, added dataDispatcher and autoDispatch. -- Tim O'Connor 6/19/07
 *  TO071107A - C89 compatibility for MSVC's cl compiler. -- Tim O'Connor 7/11/07
 *  TO071207A - MSVC++'s cl deprecated strcmpi, use NIMEX_MACRO_STRCMPI instead. -- Tim O'Connor 7/12/07
 *  TO072607A - Cycle types. -- Tim O'Connor 7/26/07
 *  TO073070A: Added enable field to channels, allowing them to be defined but not started. -- Tim O'Connor 7/30/07
 *  TO073007C - Watch out for the beguiling empty channel at the start of the list. -- 7/30/07
 *  TO073107C - General clean-up done during major overhaul (TO073107B). -- Tim O'Connor 7/31/07
 *  TO101107A - Removed `char* mnemonicName` declaration when creating a channel definition. See details in function. -- Tim O'Connor 10/11/07
 *  TO101207A - Typo. Multiply by dataBufferSize, don't add. -- Tim O'Connor 10/12/07
 *  TO101207B - If no buffer is passed during dataSource creation, allocate an empty one. -- Tim O'Connor 10/12/07
 *  TO101807C - Added channel list sorting. Some devices insist upon their channels being created in-order. -- Tim O'Connor 10/18/07
 *  TO101807D - Issue errors on NULL.
 *  JL0010208A: change taskDefinition != NULL to taskDefinition->preprocessors != NULL
 *  JL011408A: Add NIMEX_DATASOURCE_TYPE_FLOAT64CALLBACK and NIMEX_DATASOURCE_TYPE_UINT32CALLBACK
 *  JL011708A: Add this to avoid double free
 *  JL011508A: callback->name has been added to persistentMemorySegment and should be released by NIMEX_freePersistentMemorySegment
 *  JL011708B: comment out because it results in system crash when double free.
 *  TO082908C - Force callbacks into cell arrays. Check that they are valid types. -- Tim O'Connor 8/29/08
 *  TO083008E - Use mexSetTrapFlag to prevent Matlab trying to terminate the mex file, should an error occur during a call to mexCalLMatlab. See TO082908A. -- Tim O'Connor 8/29/08
 *  TO083008D - Cleaned up NIMEX_DataSource_destroy, the print statements were an ugly mess. -- Tim O'Connor 8/30/08
 *  TO083108A - Fixed a check of an uninitialized field. See details below. -- Tim O'Connor 8/31/08
 *
 */
#include "nimex.h"

/*NIMEX_Event* NIMEX_EveryNEvent_create(NIMEX_TaskDefinition* taskDefinition, int32 everyNsamplesEventType, uInt32 samples)
{
    NIMEX_Event* event = (NIMEX_Event *)NULL;

    event = (NIMEX_Event *)calloc(1, sizeof(NIMEX_Event));
    event->taskDefinition = taskDefinition;//Shallow copy, do not clear when destroying event.
    event->eventID = NIMEX_CALLBACK_EVENT_EVERYN;
    event->eventData = calloc(2, uInt32);
    event->eventData[0] = everyNsamplesEventType;
    event->eventData[1] = samples;
    
    NIMEX_addDestroyableMemorySegment(event, &NIMEX_Event_destroy);
    
    return event;
}

NIMEX_Event* NIMEX_DoneEvent_create(NIMEX_TaskDefinition* taskDefinition)
{
    event = (NIMEX_Event *)calloc(1, sizeof(NIMEX_Event));
    event->taskDefinition = taskDefinition;//Shallow copy, do not clear when destroying event.
    event->eventID = NIMEX_CALLBACK_EVENT_DONE;
    event->eventData = NULL;

    NIMEX_addDestroyableMemorySegment(event, &NIMEX_Event_destroy);

    return event;
}

NIMEX_Event* NIMEX_TimerEvent_create(NIMEX_TaskDefinition* taskDefinition)
{
    event = (NIMEX_Event *)calloc(1, sizeof(NIMEX_Event));
    event->taskDefinition = taskDefinition;//Shallow copy, do not clear when destroying event.
    event->eventID = NIMEX_CALLBACK_EVENT_TIMER;
    event->eventData = NULL;
    
    NIMEX_addDestroyableMemorySegment(event, &NIMEX_Event_destroy);
    
    return event;
}

void NIMEX_Event_destroy(void* event)
{
    if (((NIMEX_Event *)event)->eventID == NIMEX_CALLBACK_EVENT_EVERYN)
        NIMEX_MACRO_FREE(((NIMEX_Event *)event)->eventData);
}*/

/**
 * @brief Adds a NIMEX_ChannelDefinition to a NIMEX_ChannelList.
 *
 * @arg <tt>list</tt> - A valid NIMEX_ChannelList.
 * @arg <tt>item</tt> - A valid NIMEX_ChannelDefinition.
 *
 * @callgraph
 * @callergraph
 */
//For type safety at compile time, these accessor/mutator methods should be used. -- TO061207A
void NIMEX_ChannelList_append(NIMEX_ChannelList* list, NIMEX_ChannelDefinition* item)
{
    NIMEX_linkedList_append(list, item);
}
/**
 * @brief Adds a NIMEX_Callback to a NIMEX_CallbackSet.
 *
 * @arg <tt>list</tt> - A valid NIMEX_CallbackSet.
 * @arg <tt>item</tt> - A valid NIMEX_Callback.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_CallbackSet_append(NIMEX_CallbackSet* list, NIMEX_Callback* item)
{
    NIMEX_linkedList_append(list, item);
}

/**
 * @brief Used by the underlying NIMEX_linkedList to automatically sort channels by terminal names.
 *
 * @arg <tt>a</tt> - A valid NIMEX_ChannelDefinition.
 * @arg <tt>b</tt> - A valid NIMEX_ChannelDefinition.
 *
 * @return Less than 0 if a comes before b, greater than 0 if b comes before a, 0 otherwise.
 *
 * @callgraph
 * @callergraph
 */
//TO101807C - Added channel list sorting. -- Tim O'Connor 10/18/07
int32 NIMEX_ChannelList_sort_compareFunc(gconstpointer a, gconstpointer b)
{
    int aId, bId, len, i, order;
    
    //Watch out for NULLs.
    if (a == NULL)
        return -2;
    else if (b == NULL)
        return 2;
    
    //TODO: This won't work well with semicolon-style channel specifications, but it'll do for now.
    NIMEX_MACRO_VERBOSE_4("NIMEX_Objects/NIMEX_ChannelList_sort_compareFunc: Comparing @%p->\"%s\" to @%p->\"%s\"...\n", 
        a, ((NIMEX_ChannelDefinition *)a)->physicalChannel, b, ((NIMEX_ChannelDefinition *)b)->physicalChannel);

    len = strlen(((NIMEX_ChannelDefinition *)a)->physicalChannel) - 1;
    for (i = len; i > 0; i--)
    {
        if (!g_ascii_isdigit(((NIMEX_ChannelDefinition *)a)->physicalChannel[i]))
            break;
    }

    if (i < len)
        i = i + 1;
    aId = atoi(&(((NIMEX_ChannelDefinition *)a)->physicalChannel[i]));

    len = strlen(((NIMEX_ChannelDefinition *)b)->physicalChannel) - 1;
    for (i = len; i > 0; i--)
    {
        if (!g_ascii_isdigit(((NIMEX_ChannelDefinition *)b)->physicalChannel[i]))
            break;
    }
    if (i < len)
        i = i + 1;
    bId = atoi(&(((NIMEX_ChannelDefinition *)b)->physicalChannel[i]));

    //They should never be equal?
    if (aId == bId)
        return 0;
    else if (aId < bId)
        return -1;
    else if (bId < aId)
        return 1;
    else
        return 0;//What happened???
}
/**
 * @brief Calls through to the underlying g_list to handle sorting.
 *
 * @arg <tt>list</tt> - A valid NIMEX_ChannelList.
 *
 * @return The sorted list (which is equal to list, as no copies are made).
 *
 * @see g_list_sort
 *
 * @callgraph
 * @callergraph
 */
NIMEX_ChannelList* NIMEX_ChannelList_sort(NIMEX_ChannelList* list)
{
    NIMEX_MACRO_VERBOSE_3("NIMEX_Objects/NIMEX_ChannelList_sort: @%p\n", list);
    return (NIMEX_ChannelList*)g_list_sort(NIMEX_linkedList_first(list), NIMEX_ChannelList_sort_compareFunc);
}

/**
 * @brief Creates a new NIMEX_Callback object.
 *
 * @arg <tt>name</tt> - The name of the callback.
 * @arg <tt>priority</tt> - The priority of the callback.
 * @arg <tt>callback</tt> - The Matlab callback.
 *
 * @return A fully initialized NIMEX_Callback, including deep copies of all data.
 *
 * @callgraph
 * @callergraph
 */
NIMEX_Callback* NIMEX_Callback_create(const char* name, const int32 priority, const mxArray* callbackToMatlab)
{
    NIMEX_Callback*       callback = (NIMEX_Callback *)NULL;
    int dims[2] = {1, 1};
    //mxArray*              persistentMatlabCallback = (mxArray *)NULL;

    NIMEX_MACRO_VERBOSE("NIMEX_Objects/NIMEX_Callback_create...\n") 
    callback = calloc(sizeof(NIMEX_Callback), 1);
    NIMEX_addDestroyableMemorySegment(callback, &NIMEX_Callback_destroy, FALSE);
    NIMEX_MACRO_VERBOSE("NIMEX_Objects/NIMEX_Callback_create added object to memory manager.\n")
//    persistentMatlabCallback = mxDuplicateArray(callbackToMatlab);//Watch out for memory leaks here.
  //  mexMakeArrayPersistent(persistentMatlabCallback);

    //TO082908C - Force callbacks into cell arrays. Check that they are valid types. -- Tim O'Connor 8/29/08
    if (mxIsCell(callbackToMatlab))
        callback->callbackToMatlab = mxDuplicateArray(callbackToMatlab);
    else if (mxIsClass(callbackToMatlab, "function_handle"))
    {
         callback->callbackToMatlab = mxCreateCellArray(2, dims);
         mxSetCell(callback->callbackToMatlab, 0, mxDuplicateArray(callbackToMatlab));
    }
    else
    {
        NIMEX_freePersistentMemorySegment(callback);
        mexErrMsgTxt("Invalid type for callback. Must be a cell array or function_handle.");
    }
    
    mexMakeArrayPersistent(callback->callbackToMatlab);  //Watch out for memory leaks here.
    callback->name = (char *)NIMEX_memcpy(name, strlen(name) + 1);//+1 for the trailing 0.
    callback->priority = priority;
    NIMEX_MACRO_VERBOSE("NIMEX_Objects/NIMEX_Callback_create: @%p\n", callback)
    NIMEX_MACRO_VERBOSE("                                      ->callbackToMatlab = @%p\n", callback->callbackToMatlab)
    NIMEX_MACRO_VERBOSE("                                      ->name = \"%s\" (@%p)\n", callback->name, callback->name)
    NIMEX_MACRO_VERBOSE("                                      ->priority = %d\n", callback->priority)
//NIMEX_MACRO_VERBOSE("callback{0} = @%p\n", mxGetCell(callback->callbackToMatlab, 0))
//NIMEX_MACRO_VERBOSE("mxGetClassName(callback{0}) = %s\n", mxGetClassName(mxGetCell(callback->callbackToMatlab, 0)))
//NIMEX_MACRO_VERBOSE("mxGetClassID(callback{0}) = %s\n", NIMEX_mxConstant_2_string(mxGetClassID(mxGetCell(callback->callbackToMatlab, 0))))

    return callback;
}

/**
 * @brief Destorys a NIMEX_Callback object.
 *
 * All memory is freed, including the deep copies of the data (which should have been made, since you used the constructor).
 *
 * @arg <tt>callback</tt> - The callback to be destroyed.
 *
 * @post The pointer that was passed in is no longer valid.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_Callback_destroy(void* callback)
{       
        if (((NIMEX_Callback *)callback)->callbackToMatlab != NULL)
        {
            NIMEX_MACRO_VERBOSE("NIMEX_Objects/NIMEX_Callback_destroy: calling mxDestroyArray for callback->callbackToMatlab ptr=%p\n", ((NIMEX_Callback *)callback)->callbackToMatlab);
            mxDestroyArray(((NIMEX_Callback *)callback)->callbackToMatlab);//TO080813A - Changed from mxFree, which is illegal to use with mxDuplicateArray allocated memory. This bug didn't manifest until post R2007b.
            ((NIMEX_Callback *)callback)->callbackToMatlab = NULL; //JL011708A: Add this to avoid double free
        }
        else 
            NIMEX_MACRO_VERBOSE("NIMEX_Objects/NIMEX_Callback_destroy: callback->callbackToMatlab was freed. ptr=%p\n", ((NIMEX_Callback *)callback)->callbackToMatlab);
    
    //JL011508A: callback->name has been added to persistentMemorySegment and should be released by NIMEX_freePersistentMemorySegment
    //NIMEX_MACRO_FREE(((NIMEX_Callback *)callback)->name);
        if (((NIMEX_Callback *)callback)->name != NULL)
        {
            NIMEX_MACRO_VERBOSE("NIMEX_Objects/NIMEX_Callback_destroy: Free callback->name ptr=%p\n", ((NIMEX_Callback *)callback)->name);
            NIMEX_freePersistentMemorySegment(((NIMEX_Callback *)callback)->name);
            ((NIMEX_Callback *)callback)->name = NULL;
        }
        else 
            NIMEX_MACRO_VERBOSE("NIMEX_Objects/NIMEX_Callback_destroy: callback->name was freed. ptr=%p\n",((NIMEX_Callback *)callback)->name);

		NIMEX_MACRO_VERBOSE_3("NIMEX_Objects/NIMEX_Callback_destroy: successfully completed.\n");
//     JL011708B: comment out because it results in system crash when double free.
//     NIMEX_MACRO_VERBOSE("NIMEX_Objects/NIMEX_Callback_destroy: Free callback\n")
//     NIMEX_MACRO_FREE(((NIMEX_Callback *)callback))
   
    return;
}

/**
 * @brief Looks up a NIMEX_ChannelDefinition in a NIMEX_TaskDefinition by the NIDAQmx physical channel name.
 *
 * @arg <tt>taskDefinition</tt> - The NIMEX task on which to perform the lookup.
 * @arg <tt>physicalChannel</tt> - The NIDAQmx physical channel name to be used for the lookup.
 *
 * @return A pointer to the named NIMEX_ChannelDefinition, NULL if not found.
 *
 * @callgraph
 * @callergraph
 */
NIMEX_ChannelDefinition* NIMEX_getChannelByName(NIMEX_TaskDefinition* taskDefinition, const char* physicalChannel)
{
    NIMEX_ChannelDefinition* channelDefinition = (NIMEX_ChannelDefinition *)NULL;
    NIMEX_ChannelList* channelList = NIMEX_ChannelList_first(taskDefinition->channels);

    NIMEX_MACRO_VERBOSE_3("NIMEX_Objects/NIMEX_getChannelByName: Looking up channel '%s' in list @%p\n", physicalChannel, channelList)

    //TO012907B
    do
    {
        channelDefinition = NIMEX_ChannelList_value(channelList);//TO010607B //TO061207A

        //TO073007C - Watch out for the beguiling empty channel at the start of the list. -- 7/30/07
        if (channelDefinition != NULL)
        {
            NIMEX_MACRO_VERBOSE_3("NIMEX_Objects/NIMEX_getChannelByName: checking channelDefinition(@%p)...\n", channelDefinition)
            if (strcmp(channelDefinition->physicalChannel, physicalChannel) == 0)
            {
                return channelDefinition;
            }
        }

        //Iterate.
        channelList = NIMEX_ChannelList_next(channelList);
    } while (channelList != NULL);

    channelDefinition = (NIMEX_ChannelDefinition *)NULL;

    return channelDefinition;
}

/**
 * @brief Adds a NIMEX_ChannelDefinition to a NIMEX_TaskDefinition.
 *
 * @arg <tt>taskDefinition</tt> - The NIMEX task in which to add the channel.
 * @arg <tt>channelDefinition</tt> - The channel to be added.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_addChannelToList(NIMEX_TaskDefinition* taskDefinition, NIMEX_ChannelDefinition* channelDefinition)
{
    //TO010607B
    if (taskDefinition->channels == NULL)
        taskDefinition->channels = NIMEX_ChannelList_create();
    
    NIMEX_linkedList_append(taskDefinition->channels, channelDefinition);
    
    //TO101807C - Added channel list sorting. Some devices insist upon their channels being created in-order. -- Tim O'Connor 10/18/07
    taskDefinition->channels = NIMEX_ChannelList_first(NIMEX_ChannelList_sort(taskDefinition->channels));
    
    NIMEX_MACRO_VERBOSE_3("NIMEX_Objects/NIMEX_addChannelToList: Adding '%s' to channel list.\n", channelDefinition->mnemonicName)

    return;
}

/**
 * @brief Removes a NIMEX_ChannelDefinition from a NIMEX_TaskDefinition.
 *
 * @arg <tt>taskDefinition</tt> - The NIMEX task from which to remove the channel.
 * @arg <tt>channelDefinition</tt> - The channel to be removed.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_removeChannelFromList(NIMEX_TaskDefinition* taskDefinition, NIMEX_ChannelDefinition* channelDefinition)
{
    taskDefinition->channels = (NIMEX_ChannelList *)NIMEX_linkedList_remove(taskDefinition->channels, channelDefinition);//TO010607B

    return;
}

/**
 * @brief Destorys a NIMEX_CallbackSet object.
 *
 * All memory is freed, and all contained NIMEX_Callback objects are destroyed.
 *
 * @arg <tt>callback</tt> - The callback set to be destroyed.
 *
 * @post The pointer that was passed in is no longer valid.
 *
 * @callgraph
 * @callergraph
 */
//TO101207D - Finally implemented this one, noticed it was missing when switching to using a DLL for all shared code, instead of linking right into each mex file. -- Tim O'Connor 10/12/07
void NIMEX_CallbackSet_destroy(void* list)
{
    if (list != NULL)
        list = NIMEX_CallbackSet_first(list);
    
    while (list != NULL)
    {
        NIMEX_destroyCallback(NIMEX_CallbackSet_value(list));
        list = NIMEX_CallbackSet_next(list);
    }
    
    NIMEX_linkedList_destroy(list);
    
    return;
}

/**
 * @brief Adds a NIMEX_Callback to a NIMEX_CallbackMap.
 *
 * @arg <tt>table</tt> - A valid callback map.
 * @arg <tt>eventName</tt> - The name of the event to which to bind a callback (does not need to already exist).
 * @arg <tt>item</tt> - The callback to be added.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_CallbackMap_insert(NIMEX_CallbackMap* table, const char* eventName, NIMEX_Callback* item)
{
    NIMEX_CallbackSet* set = (NIMEX_CallbackSet *)NULL;
    NIMEX_CallbackSet* next = (NIMEX_CallbackSet *)NULL;
    NIMEX_Callback*    value = (NIMEX_Callback *)NULL;
    
    //TO101807D - Issue errors on NULL.
    if (table == NULL)
        mexErrMsgTxt("NIMEX_Objects/NIMEX_CallbackMap_insert - Insert attempted into a NULL table.");

    NIMEX_MACRO_VERBOSE_3("NIMEX_Objects/NIMEX_CallbackMap_insert(@%p, \"%s\", @%p)\n", table, eventName, item)
    set = NIMEX_hashTable_lookup(table, eventName);
    NIMEX_MACRO_VERBOSE_3("NIMEX_Objects/NIMEX_CallbackMap_insert: NIMEX_hashTable_lookup(@%p, \"%s\") = @%p\n", table, eventName, set)
    if (set == NULL)
    {
        NIMEX_MACRO_VERBOSE_3("NIMEX_Objects/NIMEX_CallbackMap_insert: Creating new set...\n")
        set = NIMEX_CallbackSet_create();
        NIMEX_MACRO_VERBOSE_3("NIMEX_Objects/NIMEX_CallbackMap_insert: NIMEX_hashTable_insert(@%p, \"%s\", @%p)...\n", table, eventName, set)
        NIMEX_hashTable_insert(table, eventName, set);
    }
    set = NIMEX_linkedList_first(set);

    next = set;
    while (next != NULL)
    {
        value = NIMEX_CallbackSet_value(next);
        NIMEX_MACRO_VERBOSE_3("NIMEX_Objects/NIMEX_CallbackMap_insert: Checking for name match against node @%p->@%p\n", next, value)
        if (value != NULL)
        {
            if (NIMEX_MACRO_STRCMPI(value->name, item->name) == 0)
            {
                NIMEX_MACRO_VERBOSE_3("NIMEX_Objects/NIMEX_CallbackMap_insert: Found callback named \"%s\", replacing...\n", item->name)
                //Replace entry.
                NIMEX_Callback_destroy(value);
                //Do the append before the remove, because removing the only entry from a list seems to invalidate the list.
                NIMEX_linkedList_append(set, item);
                NIMEX_linkedList_remove(set, value);

                return;
            }
        }

        next = NIMEX_CallbackSet_next(next);
    }
//mexPrintf("NIMEX_CallbackMap_insert: @%p->\"%s\"->@%p->@%p\n", table, eventName, NIMEX_CallbackSet_first(set), item);
    //Append new entry.
    NIMEX_MACRO_VERBOSE_3("NIMEX_Objects/NIMEX_CallbackMap_insert: NIMEX_CallbackSet_append(@%p, @%p)...\n", set, item)
    NIMEX_CallbackSet_append(set, item);

    return;
}

/**
 * @brief Removes a NIMEX_Callback from a NIMEX_CallbackMap.
 *
 * @arg <tt>table</tt> - A valid callback map.
 * @arg <tt>eventName</tt> - The name of the event from which to remove a callback.
 * @arg <tt>name</tt> - The unique name of the callback, in case multiple callbacks are bound to the event.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_CallbackMap_removeByName(NIMEX_CallbackMap* table, const char* eventName, const char* name)
{
    NIMEX_CallbackSet* set = (NIMEX_CallbackSet *)NULL;
    NIMEX_CallbackSet* next = (NIMEX_CallbackSet *)NULL;
    NIMEX_Callback*    value = (NIMEX_Callback *)NULL;

    set = NIMEX_hashTable_lookup(table, eventName);
    if (set == NULL)
    {
      NIMEX_MACRO_VERBOSE_3("NIMEX_Objects/NIMEX_CallbackMap_removeByName: %s not found in @%p\n", eventName, table);
    }

    next = set;
    while (next != NULL)
    {
        value = NIMEX_CallbackSet_value(next);
        if (NIMEX_MACRO_STRCMPI(value->name, name) == 0)
        {
            //Remove entry.
            NIMEX_Callback_destroy(value);
            NIMEX_linkedList_remove(next, value);
            return;
        }

        next = NIMEX_CallbackSet_next(next);
    }

    NIMEX_MACRO_VERBOSE_3("NIMEX_Objects/NIMEX_CallbackMap_removeByName: %s:%s not found in @%p\n", eventName, name, table);

    return;
}

/**
 * @brief Creates a new NIMEX_ChannelDefinition object.
 *
 * @arg <tt>channelType</tt> - The type of the channel, as defined in NIMEX_Constants.h.
 * @arg <tt>physicalName</tt> - The channel's NIDAQmx physical name.
 *
 * @return A fully initialized NIMEX_ChannelDefinition.
 *
 * @callgraph
 * @callergraph
 */
NIMEX_ChannelDefinition* NIMEX_createChannelDefinition(int channelType, char* physicalName)
{
    //TO101107A - Removed `char* mnemonicName` declaration, just use the structure's field (channelDefinition->mnemonicName) directly. -- Tim O'Connor 10/11/07
    NIMEX_ChannelDefinition* channelDefinition = (NIMEX_ChannelDefinition *)NULL;
    int i = 0;//TO071107A

    channelDefinition = NIMEX_Persistent_Calloc(1, sizeof(NIMEX_ChannelDefinition));

	NIMEX_MACRO_VERBOSE("NIMEX_Objects/NIMEX_createChannelDefinition: Initializing channel '%s'.\n", physicalName)
    //Set defaults.
    channelDefinition->channelType = channelType;
    channelDefinition->terminalConfig = DAQmx_Val_Default;//DAQmx_Val_Diff;//DAQmx_Val_PseudoDiff;//NIDAQmx documentation would lead you to believe DAQmx_Val_InputTermCfg_Diff is a valid constant, it's not (see NIDAQmx.h).
    channelDefinition->units = DAQmx_Val_Volts;
    channelDefinition->minVal = -10;
    channelDefinition->maxVal = 10;
    channelDefinition->dataSource = (NIMEX_DataSource *)NULL;//This must be initialized later! //TO080107B
    channelDefinition->enable = 1;//TO073007A

    //Apparently a '/' is an illegal character in a channel name (but not a physicalChannel name).
    channelDefinition->mnemonicName = NIMEX_Persistent_Calloc(strlen(physicalName), sizeof(char));//TO101107A

    //TO010707B
    for (i = 1; i < (strlen(physicalName)); i++)
        if (physicalName[i] == '/')
            channelDefinition->mnemonicName[i - 1] = '_';//TO101107A
        else
            channelDefinition->mnemonicName[i - 1] = physicalName[i];//TO101107A

	NIMEX_MACRO_VERBOSE("NIMEX_Objects/NIMEX_createChannelDefinition: Derived mnemonic channel name from physical channel name: '%s' -> '%s'\n", physicalName, channelDefinition->mnemonicName)
    channelDefinition->physicalChannel = NIMEX_memcpy(physicalName, (strlen(physicalName) + 1) * sizeof(char));

    return channelDefinition;
}

/**
 * @brief Destroys NIMEX_ChannelDefinition object.
 *
 * Frees all associated memory, including all NIMEX_DataSource objects.
 *
 * @arg <tt>channelDefinition</tt> - The channel to be destroyed.
 *
 * @post The pointer that was passed in is no longer valid.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_destroyChannelDefinition(NIMEX_ChannelDefinition* channelDefinition)
{
    if (channelDefinition == NULL)
        mexErrMsgTxt("NIMEX_Objects/NIMEX_destroyChannelDefinition: NULL pointer (no channel to remove).\n");

    #ifdef NIMEX_VERBOSE_2
    NIMEX_MACRO_VERBOSE("NIMEX_Objects/NIMEX_destroyChannelDefinition: \n")
    NIMEX_displayChannelDefinition("\t\t", channelDefinition);
    #else
	NIMEX_MACRO_VERBOSE("NIMEX_Objects/NIMEX_destroyChannelDefinition: Releasing channel '%s'.\n", channelDefinition->physicalChannel)
    #endif
    if (channelDefinition->dataSource != NULL)
    {
        NIMEX_DataSource_destroy(channelDefinition->dataSource);
        channelDefinition->dataSource = NULL; //JL011708A: Add this to avoid double free
    }
    if (channelDefinition->mnemonicName != NULL)
    {
        NIMEX_freePersistentMemorySegment(channelDefinition->mnemonicName);
        channelDefinition->mnemonicName = NULL;//JL011708A: Add this to avoid double free
    }
    NIMEX_MACRO_VERBOSE("NIMEX_Objects/NIMEX_destroyChannelDefinition: channelDefinition->mnemonicName is  @%p'.\n",channelDefinition->mnemonicName)
    if (channelDefinition->physicalChannel != NULL)
    {
        NIMEX_freePersistentMemorySegment(channelDefinition->physicalChannel);
        channelDefinition->physicalChannel = NULL;//JL011708A: Add this to avoid double free
    }

    return;
}

/**
 * @brief Removes a channel from a task.
 *
 * Destroys the channel.
 *
 * @arg <tt>taskDefinition</tt> - The task in which to destroy a channel.
 * @arg <tt>physicalName</tt> - The NIDAQmx physical name of the channel to be destroyed.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_removeChannelDefinitionByName(NIMEX_TaskDefinition* taskDefinition, char* physicalName)
{
    NIMEX_ChannelDefinition* channelDefinition = (NIMEX_ChannelDefinition *)NULL;

    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);
    channelDefinition = NIMEX_getChannelByName(taskDefinition, physicalName);
    if (channelDefinition != NULL)
    {
        NIMEX_destroyChannelDefinition(channelDefinition);
        channelDefinition = NULL;//JL011708A: Add this to avoid double free
    }
    else
    {
        NIMEX_releaseLock(taskDefinition);
        mexPrintf("NIMEX_Objects/NIMEX_removeChannelDefinitionByName Error: Channel \"%s\" not found.\n", physicalName);
    }
    NIMEX_releaseLock(taskDefinition);

    return;
}

/**
 * @brief Destroys a NIMEX_Callback.
 *
 * Frees all memory, including deep copies of any data.
 *
 * @arg <tt>callback</tt> - The callback to be destroyed.
 *
 * @post The pointer that was passed in is no longer valid.
 *
 * @callgraph
 * @callergraph
 */
//TO040207F
void NIMEX_destroyCallback(NIMEX_Callback* callback)
{
    if (callback->name != NULL)
    {
        NIMEX_freePersistentMemorySegment((void *)callback->name);
       (void *)callback->name = NULL; //JL011708A: Add this to avoid double free
    }
    if (callback->callbackToMatlab != NULL) 
    {
        mxDestroyArray(callback->callbackToMatlab);
        callback->callbackToMatlab = NULL; //JL011708A: Add this to avoid double free
    }

    NIMEX_freePersistentMemorySegment(callback);

    return;
}

/**
 * @brief Destroys a NIMEX_TaskDefinition.
 *
 * Destroys all member structures (such as channels). Frees all memory, including deep copies of any data.
 *
 * @arg <tt>taskDefinition</tt> - The task to be destroyed.
 *
 * @post The pointer that was passed in is no longer valid.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_destroyTaskDefinition(NIMEX_TaskDefinition* taskDefinition)
{
    char                        errBuff[2048] = {'\0'};
    NIMEX_ChannelDefinition*    channelDefinition = (NIMEX_ChannelDefinition *)NULL;
    NIMEX_ChannelList*          channelList = taskDefinition->channels;
    NIMEX_CallbackSet*          callbackSet = (NIMEX_CallbackSet *)NULL;
    NIMEX_Callback*             callback = (NIMEX_Callback *)NULL;

    NIMEX_MACRO_VERBOSE("NIMEX_Objects/NIMEX_destroyTaskDefinition: \n")
    #ifdef NIMEX_VERBOSE
    NIMEX_displayTaskDefinition("\t", taskDefinition);
    #endif

    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);//TO012407B
    //Initialize...
    if (taskDefinition->taskHandle != NULL)
    {
        if (DAQmxClearTask(*(taskDefinition->taskHandle)))
        {
            DAQmxGetExtendedErrorInfo(errBuff, 2048);
            mexPrintf("NIMEX_Objects/NIMEX_destroyTaskDefinition - Failed to properly clear taskDefinition->taskHandle: %s\n", errBuff);
        }
        NIMEX_freePersistentMemorySegment(taskDefinition->taskHandle);
        taskDefinition->taskHandle = NULL; //JL011708A: Add this to avoid double free
    }

    //TO061207A - No more direct access to the structure's fields while iterating.
    while ((channelList != NULL))
    {
        if (NIMEX_ChannelList_value(channelList) != NULL)
        {
            NIMEX_destroyChannelDefinition(NIMEX_ChannelList_value(channelList));
            channelList =NULL;  //JL011708A: Add this to avoid double free
        }

        channelList = NIMEX_ChannelList_next(channelList);
    }
    if (taskDefinition->channels != NULL)
    {
        NIMEX_linkedList_destroy(taskDefinition->channels);
        taskDefinition->channels = NULL; //JL011708A: Add this to avoid double free
    }

    if (taskDefinition->clockSource != NULL)
    {
        NIMEX_freePersistentMemorySegment(taskDefinition->clockSource);
        taskDefinition->clockSource = NULL; //JL011708A: Add this to avoid double free
    }

    if (taskDefinition->userData != NULL)
    {
        mxDestroyArray(taskDefinition->userData);//TO080813A //TO073107C - Make sure this gets properly cleared later..
        taskDefinition->userData = NULL; //JL011708A: Add this to avoid double free
    }
    //TO061907E
    //Remove all callbacks, destroying the associated (persistent) mex arrays.
    if (taskDefinition->eventMap != NULL)
    {
        NIMEX_hashTable_destroy(taskDefinition->eventMap);
        taskDefinition->eventMap = NULL; //JL011708A: Add this to avoid double free
    }
    if (taskDefinition->preprocessors != NULL)
    {
        NIMEX_hashTable_destroy(taskDefinition->preprocessors);
        taskDefinition->preprocessors = NULL; //JL011708A: Add this to avoid double free
    }
    
    if (taskDefinition->dataDispatcher != NULL)
    {
        NIMEX_hashTable_destroy(taskDefinition->dataDispatcher);
        taskDefinition->dataDispatcher = NULL; //JL011708A: Add this to avoid double free
    }
    if (taskDefinition->triggerSource != NULL)
    {
        NIMEX_freePersistentMemorySegment(taskDefinition->triggerSource);
        taskDefinition->triggerSource = NULL; //JL011708A: Add this to avoid double free
    }

    NIMEX_releaseLock(taskDefinition);//TO012407B
    
    if (taskDefinition->mutex != NULL)
    {
        CloseHandle(taskDefinition->mutex);
        NIMEX_freePersistentMemorySegment(taskDefinition->mutex);
        taskDefinition->mutex = NULL; //JL011708A: Add this to avoid double free
    }

    NIMEX_freePersistentMemorySegment(taskDefinition);

    return;
}

/**
 * @brief Sets the buffer for a data source directly.
 *
 * No memory copies are performed. Any existing buffer is freed.
 *
 * @arg <tt>dataSourceStruct</tt> - The data source for which to set a buffer.
 * @arg <tt>dataBuffer</tt> - The data buffer to place into the data source. The type must be correct for the associated channel.
 * @arg <tt>dataBufferSize</tt> - The number of samples in the data buffer.
 *
 * @callgraph
 * @callergraph
 */
//TO072607A - Cycle types. -- Tim O'Connor 7/26/07
void NIMEX_DataSource_setBufferDirect(NIMEX_DataSource* dataSourceStruct, void* dataBuffer, uInt64 dataBufferSize)
{
    if (dataSourceStruct->dataBuffer != NULL)
    {
        NIMEX_freePersistentMemorySegment(dataSourceStruct->dataBuffer);
        dataSourceStruct->dataBuffer = NULL; //JL011708A: Add this to avoid double free
    }

    dataSourceStruct->dataBuffer = dataBuffer;
    dataSourceStruct->dataBufferSize = dataBufferSize;

    return;
}

/**
 * @brief Sets the buffer for a data source using an mxArray.
 *
 * The type is implied first by the channel type and secondly by the mxArray's type.
 * Data type conversions are applied, as necessary.
 * A deep copy of the data is made.
 *
 * @arg <tt>dataSourceStruct</tt> - The data source for which to set a buffer.
 * @arg <tt>dataBuffer</tt> - The mxArray to use to set the data source's buffer.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_DataSource_setBufferFromMxArray(NIMEX_DataSource* dataSourceStruct, const mxArray* dataBuffer)
{
    if (dataSourceStruct->dataBuffer != NULL){
        NIMEX_freePersistentMemorySegment(dataSourceStruct->dataBuffer);
        dataSourceStruct->dataBuffer = NULL; //JL011708A: Add this to avoid double free
     NIMEX_MACRO_VERBOSE_4("NIMEX_freePersistentMemorySegment");}

    dataSourceStruct->dataBufferSize = (uInt64)mxGetNumberOfElements(dataBuffer);
    NIMEX_MACRO_VERBOSE_4("mxGetNumberOfElements");

    if ((char)(dataSourceStruct->dataSourceType & NIMEX_DATASOURCE_TYPE_FLOAT64_BITMASK))
         {dataSourceStruct->dataBuffer = NIMEX_mxArray_2_float64(dataBuffer);
                  NIMEX_MACRO_VERBOSE_4("NIMEX_mxArray_2_float64");}
        //dataSourceStruct->dataBuffer = NIMEX_memcpy((float64 *)mxGetData(dataBuffer), dataSourceStruct->dataBufferSize * sizeof(float64));
    else if ((char)(dataSourceStruct->dataSourceType & NIMEX_DATASOURCE_TYPE_UINT32_BITMASK))
         {dataSourceStruct->dataBuffer = NIMEX_mxArray_2_uInt32(dataBuffer);
           NIMEX_MACRO_VERBOSE_4("NIMEX_mxArray_2_uInt32");}
        //dataSourceStruct->dataBuffer = NIMEX_memcpy((uInt32 *)mxGetData(dataBuffer), dataSourceStruct->dataBufferSize * sizeof(uInt32));
    else
    {
        switch (mxGetClassID(dataBuffer))
        {
            NIMEX_MACRO_VERBOSE_4("switch");
            case mxDOUBLE_CLASS:
                mexPrintf("NIMEX_Objects/NIMEX_DataSource_setBufferFromMxArray: Datasource data type being inferred from mxArray's ClassID [%s] as float64 (mxDOUBLE_CLASS).\n", NIMEX_mxConstant_2_string(mxGetClassID(dataBuffer)));
                dataSourceStruct->dataBuffer = NIMEX_memcpy((float64 *)mxGetPr(dataBuffer), dataSourceStruct->dataBufferSize * sizeof(float64));
                break;

            case mxUINT32_CLASS:
                mexPrintf("NIMEX_Objects/NIMEX_DataSource_setBufferFromMxArray: Datasource data type being inferred from mxArray's ClassID [%s] as uInt32 (mxUINT32_CLASS).\n", NIMEX_mxConstant_2_string(mxGetClassID(dataBuffer)));
                dataSourceStruct->dataBuffer = NIMEX_memcpy((uInt32 *)mxGetPr(dataBuffer), dataSourceStruct->dataBufferSize * sizeof(uInt32));
                break;

            default:
                mexPrintf("NIMEX_Objects/NIMEX_DataSource_setBufferFromMxArray: Unable to update buffer. Could not coerce data type of mxArray into a C data type.\n\t\tTry explicitly setting array type in Matlab using `double` or `uint32`.\n");
        }
    }

    return;
}

/**
 * @brief Sets the buffer for a data source using an mxArray.
 *
 * The type is implied first by the channel type and secondly by the mxArray's type.
 * Data type conversions are applied, as necessary.
 * A deep copy of the data is made.
 *
 * @arg <tt>dataSourceType</tt> - The type of the data source, as per NIMEX_Constants.h.
 * @arg <tt>dataBuffer</tt> - The data buffer to be placed into this data source. May be NULL. A deep copy will be made.
 * @arg <tt>callback</tt> - The associated Matlab callback. May be NULL. A deep copy will be made.
 * @arg <tt>dataBufferSize</tt> - The number of samples in dataBuffer. Only applies of dataBuffer is not NULL.
 * @arg <tt>name</tt> - The name of the new data source.
 *
 * @return A fully initialized data source.
 *
 * @callgraph
 * @callergraph
 */
NIMEX_DataSource* NIMEX_DataSource_create(int dataSourceType, const void* dataBuffer, const mxArray* callback, uInt64 dataBufferSize, const char* name)
{
    NIMEX_DataSource* dataSourceStruct;
    size_t copySizeInBytes = 0;

    if ((dataSourceType != NIMEX_DATASOURCE_TYPE_FLOAT64) && (dataSourceType != NIMEX_DATASOURCE_TYPE_CALLBACK) &&
        (dataSourceType != NIMEX_DATASOURCE_TYPE_UINT32) && (dataSourceType != NIMEX_DATASOURCE_TYPE_FLOAT64CALLBACK) && 
        (dataSourceType != NIMEX_DATASOURCE_TYPE_UINT32CALLBACK))
    {
      mexPrintf("Error - NIMEX_Objects/NIMEX_DataSource_create: Invalid dataSourceType field: 0x%X (%d)\n", dataSourceType);
      return NULL;
    }

    if (dataSourceType == NIMEX_DATASOURCE_TYPE_CALLBACK)
    {
        mexPrintf("NIMEX_Objects/NIMEX_DataSource_create - Warning: Using %s as dataSource type means the return value from the callback will imply the datatype."
                  "                                                 For type safety, use NIMEX_DATASOURCE_TYPE_FLOAT64CALLBACK or NIMEX_DATASOURCE_TYPE_UINT32CALLBACK instead.",
                  NIMEX_Constant_2_string(dataSourceType));
    }

    dataSourceStruct = (NIMEX_DataSource *)calloc(sizeof(NIMEX_DataSource), 1);
    NIMEX_addDestroyableMemorySegment(dataSourceStruct, &NIMEX_DataSource_destroy, FALSE);

    dataSourceStruct->name = (char *)NIMEX_memcpy(name, sizeof(char) * (strlen(name) + 1));
    if (callback != NULL)
    {
        dataSourceStruct->callback = mxDuplicateArray(callback);
        mexMakeArrayPersistent(dataSourceStruct->callback);
    }

    //TO101207A - Typo. Multiply by dataBufferSize, don't add. -- Tim O'Connor 10/12/07
    //TO083108A - This had been using dataSourceStruct->dataSourceType, instead of just dataSourceType, even though the field hasn't been initialized yet. Oops! -- Tim O'Connor 8/31/08
    if (dataSourceType & NIMEX_DATASOURCE_TYPE_FLOAT64)
        copySizeInBytes = (size_t)((uInt64)sizeof(float64) * dataBufferSize);
    else if (dataSourceType & NIMEX_DATASOURCE_TYPE_UINT32)
        copySizeInBytes = (size_t)((uInt64)sizeof(uInt32) * dataBufferSize);

    dataSourceStruct->dataSourceType = dataSourceType;
    dataSourceStruct->dataBufferSize = dataBufferSize;
    if (dataBuffer != NULL)
        NIMEX_DataSource_setBufferDirect(dataSourceStruct, NIMEX_memcpy(dataBuffer, copySizeInBytes), dataBufferSize);
    else if ( (dataSourceStruct->dataBufferSize > 0) && ((dataSourceStruct->dataSourceType & NIMEX_DATASOURCE_TYPE_CALLBACK_BITMASK) != NIMEX_DATASOURCE_TYPE_CALLBACK) )
    {
        NIMEX_MACRO_VERBOSE_3("NIMEX_Objects/NIMEX_DataSource_create - Creating empty buffer of %llu samples and type %s.\n", dataSourceStruct->dataBufferSize, NIMEX_Constant_2_string(dataSourceStruct->dataSourceType));
        //TO101207B - If no buffer is passed, allocate an empty one. -- Tim O'Connor 10/12/07
        if (dataSourceStruct->dataSourceType & NIMEX_DATASOURCE_TYPE_FLOAT64)
            dataSourceStruct->dataBuffer = NIMEX_Persistent_Calloc(dataSourceStruct->dataBufferSize, sizeof(float64));
        else if (dataSourceStruct->dataSourceType & NIMEX_DATASOURCE_TYPE_UINT32)
            dataSourceStruct->dataBuffer = NIMEX_Persistent_Calloc(dataSourceStruct->dataBufferSize, sizeof(uInt32));
    }

    return dataSourceStruct;
}

/**
 * @brief Updates the buffer of the data source using the associated callback.
 *
 * @arg <tt>taskDefinition</tt> - The task in which the data source lives. Needed for access to preprocessors.
 * @arg <tt>dataSourceStruct</tt> - The data source to be updated.
 *
 * @see NIMEX_DataSource_setBufferFromMxArray
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_DataSource_updateBufferByCallback(NIMEX_TaskDefinition* taskDefinition, NIMEX_DataSource* dataSourceStruct)
{
    int             nargs = 0;
    int             i = 0;
    int32           error = 0;
    mxArray**       plhs = (mxArray**)NULL;
    mxArray**       prhs = (mxArray**)NULL;

    NIMEX_MACRO_VERBOSE_2("NIMEX_Objects/NIMEX_DataSource_updateBufferByCallback...\n");
    if (!((char)(dataSourceStruct->dataSourceType & NIMEX_DATASOURCE_TYPE_CALLBACK_BITMASK)))
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_Objects/NIMEX_DataSource_updateBufferByCallback: Not a callback typed datasource (%s).\n", NIMEX_Constant_2_string(dataSourceStruct->dataSourceType));
        return;
    }

    if (dataSourceStruct->callback == NULL)
    {
        mexPrintf("NIMEX_Objects/NIMEX_DataSource_updateBufferByCallback: Callback does not exist.\n");
        return;
    }

    nargs = mxGetNumberOfElements(dataSourceStruct->callback);
    prhs = (mxArray**)mxCalloc(nargs, sizeof(mxArray*));
    plhs = (mxArray**)mxCalloc(1, sizeof(mxArray*));
    for (i = 0; i < nargs ; i++)
    {
        prhs[i] = mxGetCell(dataSourceStruct->callback, i);

        if (prhs[i] == 0)
        {
            mexPrintf("NIMEX_Objects/NIMEX_DataSource_updateBufferByCallback: Error - Failed to marshall dataSource callback argument from cell array element %d.\n", i);
            return;
        }
    }

    mexSetTrapFlag(1);//TO083008E
    error = mexCallMATLAB(1, plhs, nargs, prhs, "feval");//Execute Matlab function.
    mexSetTrapFlag(0);//TO083008E

    //JL0010208A: change taskDefinition != NULL to taskDefinition->preprocessors != NULL
    if (taskDefinition->preprocessors != NULL)  
        NIMEX_preprocessorCallback(taskDefinition->preprocessors, dataSourceStruct->name, plhs);//Call preprocessors for this channel.

    //TO102508F - Handle errors in preprocessor functions. -- Tim O'Connor 10/25/08
    if (plhs[0] == NULL)
    {
       mexPrintf("NIMEX_Objects/NIMEX_DataSource_updateBufferByCallback: Error - Failed to execute preprocessor(s) for dataSource \"%s\".\n", dataSourceStruct->name);
       NIMEX_DataSource_clearBuffer(dataSourceStruct);
       mxFree(prhs);
	   mxFree(plhs);//TO080813B - Why wasn't this freed before? It should be, I think. -- Tim O'Connor 8/8/13
       return;
    }

    NIMEX_DataSource_setBufferFromMxArray(dataSourceStruct, plhs[0]);

    //Should we determine the number of samples from the data source? -- Tim O'Connor 10/10/07
    //NIMEX_MACRO_VERBOSE_2("NIMEX_Objects/NIMEX_DataSource_updateBufferByCallback: Inferring @%p->sampsPerChanToAcquire from dataSource: %llu\n", taskDefinition, dataSourceStruct->dataBufferSize);
    //taskDefinition->sampsPerChanToAcquire = dataSourceStruct->dataBufferSize;
//NIMEX_MACRO_VERBOSE("NIMEX_Objects/NIMEX_DataSource_updateBufferByCallback: Destroying Matlab created array, after preprocessing has finished and the data has been moved into a C buffer.\n");
    mxDestroyArray(plhs[0]);//TO121707A - Discard this array, we're done with it now.
    mxFree(plhs);
    mxFree(prhs);

    if (error)
        mexPrintf("NIMEX_Objects/NIMEX_DataSource_updateBufferByCallback: Error - Failed to execute callback for dataSource \"%s\".\n", dataSourceStruct->name);

    return;
}

/**
 * @brief Clears the attached buffer, freeing the memory consumed by it.
 *
 * @arg <tt>dataSourceStruct</tt> - The data source whose buffer to be cleared.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_DataSource_clearBuffer(NIMEX_DataSource* dataSourceStruct)
{
    if (dataSourceStruct->dataBuffer != NULL)
    {
        NIMEX_freePersistentMemorySegment(dataSourceStruct->dataBuffer);
        dataSourceStruct->dataBuffer = NULL;
    }
    dataSourceStruct->dataBufferSize = 0;   

    return;
}

/**
 * @brief Destroys a data source.
 *
 * All memory is freed, including deep copies of data.
 *
 * @arg <tt>dataSource</tt> - The data source to be destroyed.
 *
 * @post The pointer that was passed in is no longer valid.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_DataSource_destroy(void* dataSource)
{
     //TO083008D - Cleaned all of this up, the print statements were an ugly mess. Things should be nicely formatted, with puncutation and uniformity. -- Tim O'Connor 8/30/08
     NIMEX_MACRO_VERBOSE_2("NIMEX_Objects/NIMEX_DataSource_destroy: dataSource - @%p\n", dataSource);
        
     //JL011408A: Add NIMEX_DATASOURCE_TYPE_FLOAT64CALLBACK and NIMEX_DATASOURCE_TYPE_UINT32CALLBACK
     NIMEX_MACRO_VERBOSE_2("NIMEX_Objects/NIMEX_DataSource_destroy: Free dataSource->callback - @%p\n", ((NIMEX_DataSource *)dataSource)->callback);
     if (((NIMEX_DataSource *)dataSource)->dataSourceType == NIMEX_DATASOURCE_TYPE_CALLBACK ||
     ((NIMEX_DataSource *)dataSource)->dataSourceType == NIMEX_DATASOURCE_TYPE_FLOAT64CALLBACK ||
     ((NIMEX_DataSource *)dataSource)->dataSourceType == NIMEX_DATASOURCE_TYPE_UINT32CALLBACK)
     {
        if (((NIMEX_DataSource *)dataSource)->callback != NULL)
        {
           mxDestroyArray(((NIMEX_DataSource *)dataSource)->callback);//TO080813A - see other TO080813A for details.
           ((NIMEX_DataSource *)dataSource)->callback = NULL;
        }
     }
     else if (((NIMEX_DataSource *)dataSource)->callback != NULL)
     {
        NIMEX_MACRO_VERBOSE_2("NIMEX_Objects/NIMEX_DataSource_destroy: Found callback in a non-callback dataSourceType (%s)\n", 
                              NIMEX_Constant_2_string(((NIMEX_DataSource *)dataSource)->dataSourceType));
     }
        
     NIMEX_MACRO_VERBOSE_2("NIMEX_Objects/NIMEX_DataSource_destroy: Free dataSource->dataBuffer - @%p\n", ((NIMEX_DataSource *)dataSource)->dataBuffer);        
     if ((((NIMEX_DataSource *)dataSource)->dataBuffer) != NULL)
     {
        NIMEX_freePersistentMemorySegment(((NIMEX_DataSource *)dataSource)->dataBuffer);
        ((NIMEX_DataSource *)dataSource)->dataBuffer = NULL;
     }
        
     NIMEX_MACRO_VERBOSE_2("NIMEX_Objects/NIMEX_DataSource_destroy: Free dataSource->name - @%p\n", ((NIMEX_DataSource *)dataSource)->name);
     if ((((NIMEX_DataSource *)dataSource)->name) != NULL)
     {
        NIMEX_freePersistentMemorySegment(((NIMEX_DataSource *)dataSource)->name);
        ((NIMEX_DataSource *)dataSource)->name = NULL;
     }
   
// JL011708B: comment out because it results in system crash when double free.         
//     mexPrintf("NIMEX_Objects/NIMEX_DataSource_destroy : free datasource\n");
//     NIMEX_MACRO_FREE(dataSource);
    
    return;
}

/**
 * @brief Creates an mxArray, of the correct type, to represent the data source.
 *
 * Callback-based data sources may be wrapped as a function_handle/cell array type.
 *
 * @arg <tt>dataSource</tt> - The data source to be wrapped into an mxArray.
 *
 * @return An mxArray that represents the data source.
 *
 * @callgraph
 * @callergraph
 */
mxArray* NIMEX_DataSource_toMxArray(NIMEX_DataSource* dataSource)
{
    mxArray* matlabData;
    
    if (dataSource == NULL)
        NIMEX_transientWrapArray2mxArray(&matlabData, 0, mxDOUBLE_CLASS, NULL);//TO080813C
    else if ((char)(dataSource->dataSourceType & NIMEX_DATASOURCE_TYPE_CALLBACK_BITMASK))
        matlabData = mxDuplicateArray(dataSource->callback);
    else if ((char)(dataSource->dataSourceType & NIMEX_DATASOURCE_TYPE_FLOAT64_BITMASK))
        NIMEX_transientWrapArray2mxArray(&matlabData, dataSource->dataBufferSize, mxDOUBLE_CLASS, dataSource->dataBuffer);
    else if ((char)(dataSource->dataSourceType & NIMEX_DATASOURCE_TYPE_UINT32_BITMASK))
        NIMEX_transientWrapArray2mxArray(&matlabData, dataSource->dataBufferSize, mxUINT32_CLASS, dataSource->dataBuffer);
    else
    {
        mexPrintf("NIMEX_Objects/NIMEX_DataSource_toMxArray: Warning - Could not convert NIMEX_DataSource of type \"%s\" into a Matlab type.", 
            NIMEX_Constant_2_string(dataSource->dataSourceType));
        NIMEX_transientWrapArray2mxArray(&matlabData, 0, mxDOUBLE_CLASS, NULL);
    }
    
    return matlabData;
}

/**
 * @brief Initializes a data source from an mxArray.
 *
 * The mxArray type is used to determine the type of the resulting data source.
 *
 * @arg <tt>dataSource</tt> - The location to place the pointer to the resulting data source.
 * @arg <tt>matlabData</tt> - The data that will constitute the data source, either a callback or an array of data.
 * @arg <tt>name</tt> - The name to be used for the new data source.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_DataSource_fromMxArray(NIMEX_DataSource** dataSource, const mxArray* matlabData, const char* name)
{
    if ((*dataSource) == NULL)
    {
        switch (mxGetClassID(matlabData))
        {
            case mxDOUBLE_CLASS:
                (*dataSource) = NIMEX_DataSource_create(NIMEX_DATASOURCE_TYPE_FLOAT64, mxGetPr(matlabData), NULL, mxGetNumberOfElements(matlabData), name);
                return;

            case mxUINT32_CLASS:
                (*dataSource) = NIMEX_DataSource_create(NIMEX_DATASOURCE_TYPE_UINT32, mxGetPr(matlabData), NULL, mxGetNumberOfElements(matlabData), name);
                return;

            case mxCELL_CLASS:
                (*dataSource) = NIMEX_DataSource_create(NIMEX_DATASOURCE_TYPE_CALLBACK, NULL, matlabData, 0, name);
                return;

            default:
                mexErrMsgTxt("NIMEX_Objects/NIMEX_DataSource_fromMxArray: Failed to infer dataSource type from mxArray type.");
        }
    }
    else if (mxGetClassID(matlabData) == mxCELL_CLASS)
    {
        //Turn this into a callback. Keep the raw data type, if known.
        (*dataSource)->dataSourceType = (*dataSource)->dataSourceType | NIMEX_DATASOURCE_TYPE_CALLBACK_BITMASK;//Label as a callback from now on.
        
        if ((*dataSource)->dataBuffer != NULL)
        {
            NIMEX_freePersistentMemorySegment((*dataSource)->dataBuffer);
            (*dataSource)->dataBufferSize = 0;
            (*dataSource)->dataBuffer = NULL;
        }
        (*dataSource)->callback = mxDuplicateArray(matlabData);
		mexMakeArrayPersistent((*dataSource)->callback);//TO080813A
    }
    else if (mxIsNumeric(matlabData))
    {
        //Turn this into a raw data type.
        (*dataSource)->dataSourceType = (*dataSource)->dataSourceType & ~NIMEX_DATASOURCE_TYPE_CALLBACK_BITMASK;//Label as not a callback from now on.

        if ((*dataSource)->callback != NULL)
        {
            mxDestroyArray((*dataSource)->callback);//TO080813A
            (*dataSource)->callback = NULL;
        }

        NIMEX_DataSource_setBufferFromMxArray((*dataSource), matlabData);
    }
    else
    {
        //We only support cell arrays (callbacks) and raw data types (float64 or uInt32, or their numeric equivalents).
        mexPrintf("NIMEX_Objects/NIMEX_DataSource_toMxArray: Warning - Could convert Matlab variable of type \"%s\" into NIMEX_DataSource of type \"%s\".", 
            NIMEX_mxConstant_2_string(mxGetClassID(matlabData)), NIMEX_Constant_2_string((*dataSource)->dataSourceType));
    }

    if (strcmp((*dataSource)->name, name) != 0)
    {
        NIMEX_freePersistentMemorySegment((*dataSource)->name);
        (*dataSource)->name = NIMEX_memcpy((*dataSource)->name, strlen(name + 1));
    }

    return;
}

/**
 * @todo Test/document NIMEX_CyclePosition_create.
 */
NIMEX_CyclePosition* NIMEX_CyclePosition_create(NIMEX_TaskDefinition**  taskDefinitions, int taskCount)
{
    NIMEX_CyclePosition* cyclePosition = (NIMEX_CyclePosition *)calloc(sizeof(NIMEX_CyclePosition), 1);
    NIMEX_addDestroyableMemorySegment(cyclePosition, &NIMEX_CyclePosition_destroy, FALSE);
    cyclePosition->taskDefinitions = taskDefinitions;
    cyclePosition->taskCount = taskCount;
    cyclePosition->dataSources = NIMEX_DataSourceMap_create();    

    return cyclePosition;
}

/**
 * @todo Test/document NIMEX_CyclePosition_create.
 */
void NIMEX_CyclePosition_destroy(void* cyclePosition)
{
    
    if (((NIMEX_CyclePosition *)cyclePosition)->taskDefinitions != NULL)
    {
        mexPrintf("NIMEX_Objects/NIMEX_cyclePosition_destroy :free taskDefinitions\n");
        NIMEX_freePersistentMemorySegment(((NIMEX_CyclePosition *)cyclePosition)->taskDefinitions);
        ((NIMEX_CyclePosition *)cyclePosition)->taskDefinitions = NULL;
    }
    else
        mexPrintf("NIMEX_Objects/NIMEX_cyclePosition_destroy :taskDefinition was freed.\n");
        
    if (((NIMEX_CyclePosition *)cyclePosition)->dataSources != NULL)
    {
        mexPrintf("NIMEX_Objects/NIMEX_cyclePosition_destroy :free datasources\n");
        NIMEX_DataSourceMap_destroy(((NIMEX_CyclePosition *)cyclePosition)->dataSources);
        ((NIMEX_CyclePosition *)cyclePosition)->dataSources = NULL;
    }
    else
        mexPrintf("NIMEX_Objects/NIMEX_cyclePosition_destroy :datasources was freed.\n");
// JL011708B: comment out because it results in system crash when double free. 
//     NIMEX_MACRO_FREE(cyclePosition);//JL01162008 comment this out and 
    
    return;
}

/**
 * @todo Test/document NIMEX_CyclePosition_create.
 */
void NIMEX_CyclePosition_setDataSource(const char* channelName, NIMEX_DataSource* dataSource)
{
    //Replace, if one already exists.
    mexErrMsgTxt("NIMEX_Objects/NIMEX_CyclePosition_setDataSource NOT_YET_IMPLEMENTED\n");
}

/**
 * @todo Test/document NIMEX_Cycle_create.
 */
NIMEX_Cycle* NIMEX_Cycle_create()
{
    NIMEX_Cycle*            cycle;
    
    cycle = (NIMEX_Cycle *)calloc(sizeof(NIMEX_Cycle), 1);
    NIMEX_addDestroyableMemorySegment(cycle, &NIMEX_Cycle_destroy, FALSE);
    cycle->repetitions = 0;
    cycle->currentPosition = 0;
    cycle->totalPositionsStarted = 0;
    cycle->mutex = NULL;
    cycle->timeInterval = 0;
    cycle->repeatable = 1;
    cycle->eventMap = NULL;//Initialize on first event binding.

    return cycle;
}

/**
 * @todo Test/document NIMEX_Cycle_destroy.
 */
void NIMEX_Cycle_destroy(void* cycle)
{
    //Don't forget to clear the eventMap if it exists.
    if (((NIMEX_Cycle *)cycle)->eventMap != NULL)
    {
        mexPrintf("NIMEX_Objects/NIMEX_cycle_destroy :free eventmap\n");
        NIMEX_hashTable_destroy(((NIMEX_Cycle *)cycle)->eventMap);
        ((NIMEX_Cycle *)cycle)->eventMap = NULL;
    }
    else
        mexPrintf("NIMEX_Objects/NIMEX_cycle_destroy :eventmap was freed.\n");
        
    return;
}

/**
 * @todo Implement/test/document NIMEX_Cycle_start.
 */
void NIMEX_Cycle_start(NIMEX_Cycle* cycle)
{
}

/**
 * @todo Implement/test/document NIMEX_Cycle_stop.
 */
void NIMEX_Cycle_stop(NIMEX_Cycle* cycle)
{
}

/**
 * @todo Implement/test/document NIMEX_Cycle_timerFcn.
 */
void NIMEX_Cycle_timerFcn()
{
}

/**
 * @brief Iterator used for displaying a callback map to standard out.
 */
void NIMEX_displayCallbackMap_foreach(gpointer key, gpointer value, gpointer prefix)
{
    char* nextPrefix = (char *)NULL;
    int   prefixLength = 0;
    char* prefixSuffix = "   ";

    mexPrintf("%s Event: \"%s\"\n", prefix, key);

    prefixLength = strlen(prefix);
    nextPrefix = calloc(prefixLength + 4 + 1, sizeof(char));
    memcpy(nextPrefix, prefix, strlen(prefix));
    memcpy(nextPrefix + strlen(prefix), prefixSuffix, 4);
    
    NIMEX_displayCallbackSet(nextPrefix, (NIMEX_CallbackSet *)value);
    free(nextPrefix);
    
    return;
}

/**
 * @brief Prints a callback map to standard out.
 */
void NIMEX_displayCallbackMap(char* prefix, NIMEX_CallbackMap* callbackMap)
{
    mexPrintf("%sNIMEX_CallbackMap: @%p\n", prefix, callbackMap);
    g_hash_table_foreach(callbackMap, &NIMEX_displayCallbackMap_foreach, prefix);
    
    return;
}

/**
 * @brief Prints a callback set to standard out.
 */
void NIMEX_displayCallbackSet(char* prefix, NIMEX_CallbackSet* callbackSet)
{
    char* nextPrefix = (char *)NULL;
    int   prefixLength = 0;
    char* prefixSuffix = "   ";
    NIMEX_Callback* callback;

    mexPrintf("%sNIMEX_CallbackSet: @%p\n", prefix, callbackSet);

    prefixLength = strlen(prefix);
    nextPrefix = calloc(prefixLength + 4 + 1, sizeof(char));
    memcpy(nextPrefix, prefix, strlen(prefix));
    memcpy(nextPrefix + strlen(prefix), prefixSuffix, 4);
    callbackSet = NIMEX_CallbackSet_first(callbackSet);
    while (callbackSet != NULL)
    {
        callback = NIMEX_CallbackSet_value(callbackSet);
        if (callback != NULL)
            NIMEX_displayCallback(nextPrefix, callback);
        else
            mexPrintf("%s(callback == NULL)\n", nextPrefix);
        
        callbackSet = NIMEX_CallbackSet_next(callbackSet);
    }
    
    return;
}

/**
 * @brief Prints a callback to standard out.
 */
void NIMEX_displayCallback(char* prefix, NIMEX_Callback* callback)
{
    mexPrintf("%sNIMEX_Callback: @%p\n", prefix, callback);
    mexPrintf("%s @%p->name = \"%s\"\n", prefix, callback, callback->name);
    mexPrintf("%s @%p->priority = %d\n", prefix, callback, callback->priority);
    if (callback->callbackToMatlab != NULL)
    {
        mexPrintf("%s @%p->callbackToMatlab = %d of %s\n", prefix, callback, mxGetNumberOfElements(callback->callbackToMatlab), NIMEX_mxConstant_2_string(mxGetClassID(callback->callbackToMatlab)));
    }
    else
        mexPrintf("%s @%p->callbackToMatlab = NULL\n");

    return;
}
