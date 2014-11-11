/**
 * @file NIMEX_memManagement.c
 *
 * @brief Functions for NIMEX initialization/tear down and overall memory management.
 *
 * Because Matlab requires mex files to be able to clean up at any time, due to a call to `clear mex`,
 * it is necessary to have a memory management system, which can track all allocated memory and 
 * completely unload the package on demand.
 *
 * Conceptually, there isn't a strong case to be made for doing the system initialization (such as NIMEX_Global::matlabThread)
 * in this file. It's just a convenient place to implement it.
 *
 * Originally, this had been implemented using a shared memory scheme, because each shared library (ie. DLL/mex file) has its
 * own memory space. Changing the build to produce a single shared file for the base of NIMEX alleviates this need. The only
 * concern about continuing to allow shared memory would be to have a single instance of NIMEX work across Matlab sessions.
 *
 * @author Timothy O'Connor
 * @date 1/6/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 *
 */
/*
 * CHANGES
 *  TO010707C: Created NIMEX_uint32_To_mxArray, NIMEX_int32_To_mxArray, NIMEX_float64_To_mxArray, & NIMEX_uint64_To_mxArray. -- Tim O'Connor 1/7/07
 *  TO012706A: Added logging of memory allocation/deallocation to files. -- Tim O'Connor 1/27/06
 *  TO040207D: Warn when attempting to delete a NULL pointer. -- Tim O'Connor 4/2/07
 *  TO061207A: Port basic structures to glib. -- Tim O'Connor 6/12/07
 *  TO061307A: Implement shared memory, instead of a Matlab global variable. -- Tim O'Connor 6/13/07
 *  TO101107B: Protect the memory management system from access outside of the Matlab thread. -- Tim O'Connor 10/11/07
 *  JL011408A: To detroy the destroyable objects
 *  JL011408A: To detroy the destroyable objects
 *  JL011508A: g_ptr_array_foreach failed to free all elements in GptrArray, so add this while statement
 *  JL011508B: add this to free persistent memory 
 *  JL011508C: to release global->criticalSection
 *  JL011508D: Add if statement to aviod double free.
 *  JL011108A: Add this function to freeDestroyalbeMemorySegement
 *  JL011808A: Add quickdestroy field. If quickdestroy is true, won't call NIMEX_MACRO_FREE(data->ptr) in NIMEX_GPtrArray_GFunc_Iterator_destroy. This is used for linked list and hash table.
 *  TO080813F: 64-bit pointer handling. -- Tim O'Connor 8/8/13
 *
 *  NOTES
 *   For information on shared memory in Windows (TO061307A) see - https://msdn2.microsoft.com/en-us/library/aa366556.aspx
 *   All initialization of shared memory and mutexes is protected by checks for previous initialization, and that's fine in this scenario.
 *      It is threadsafe because it's initiated from Matlab, which is single threaded. Calling these functions from another thread, before
 *      Matlab has had a chance to force their initialization is not valid and can produce race conditions.
 *
 */
#include "nimex.h"

static NIMEX_Global* NIMEX_GLOBAL = (NIMEX_Global *)NULL;
//static NIMEX_PersistenceList* NIMEX_GLOBAL_PERSISTENCE_LIST = (NIMEX_PersistenceList *)NULL;

/**
 * @brief Simply frees the memory.
 *
 * @arg <tt>ptr</tt> - A pointer to be freed.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_simpleDestructor(void* ptr)
{
    NIMEX_MACRO_FREE(ptr);
}

// #define NIMEX_DEBUG_MEM_MANAGEMENT
//TO012706A
#ifdef NIMEX_DEBUG_MEM_MANAGEMENT
    #include <stdio.h>
    
    #define NIMEX_memManagement_log(...) \
        NIMEX_memManagement_log_init; \
        fprintf(NIMEX_DEBUG_MEM_MANAGEMENT_LOGFILE, __VA_ARGS__); \
		NIMEX_memManagement_log_finalize
    void NIMEX_memManagement_log_init_impl(void);
    #define NIMEX_memManagement_log_init NIMEX_memManagement_log_init_impl();
    void NIMEX_memManagement_log_finalize_impl(void);
    #define NIMEX_memManagement_log_finalize NIMEX_memManagement_log_finalize_impl();
    
    FILE* NIMEX_DEBUG_MEM_MANAGEMENT_LOGFILE = (FILE *)NULL;
    
    //Initialize memory access logging.
    void NIMEX_memManagement_log_init_impl()
    {
        if (NIMEX_DEBUG_MEM_MANAGEMENT_LOGFILE == NULL)
        {
            //mexPrintf("Initializing log file...\n");
            NIMEX_DEBUG_MEM_MANAGEMENT_LOGFILE = fopen("C:\\temp\\NIMEX_memManagement_debug.log", "a");
        }
            
        return;
    }
    
    void NIMEX_memManagement_log_finalize_impl()
    {
        if (NIMEX_DEBUG_MEM_MANAGEMENT_LOGFILE != NULL)
        {
            //mexPrintf("Closing log file...\n");
            fclose(NIMEX_DEBUG_MEM_MANAGEMENT_LOGFILE);
			NIMEX_DEBUG_MEM_MANAGEMENT_LOGFILE = NULL;
        }
        
        return;
    }
#else
    #define NIMEX_memManagement_log(...)
    //#define NIMEX_memManagement_log_init(__VA_ARGS__)
    #define NIMEX_memManagement_log_finalize
#endif

/**
 * @brief Convert a 32-bit integer (pointer) into an mxArray.
 *
 * @arg <tt>val</tt> - A 32-bit value to be wrapped.
 *
 * @return A value suitable for passing into Matlab.
 *
 * @callgraph
 * @callergraph
 */
mxArray* NIMEX_memManagement_int32_To_mxArray(int32 val)
{
    mxArray* matlabArray;
    int32* data;
    
    matlabArray = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL);
    data = (int32 *)mxGetData(matlabArray);
    *data = val;
    
    return matlabArray;
}

/**
 * @brief Convert a 64-bit integer (pointer) into an mxArray.
 *
 * @arg <tt>val</tt> - A 64-bit value to be wrapped.
 *
 * @return A value suitable for passing into Matlab.
 *
 * @callgraph
 * @callergraph
 */
//TO080813F - 64-bit pointer handling.
mxArray* NIMEX_memManagement_int64_To_mxArray(int64 val)
{
    mxArray* matlabArray;
    int64* data;
    
    matlabArray = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
    data = (int64 *)mxGetData(matlabArray);
    *data = val;
    
    return matlabArray;
}

/**
 * @brief Convert a 32-bit or 64-bit pointer into an mxArray.
 *
 * @arg <tt>addr</tt> - A 32-bit or 64-bit pointer to be wrapped.
 *
 * @return A value suitable for passing into Matlab.
 *
 * @see NIMEX_memManagement_uint32_To_mxArray
 * @see NIMEX_memManagement_uint64_To_mxArray
 *
 * @callgraph
 * @callergraph
 */
//TO080813F - 64-bit pointer handling.
mxArray* NIMEX_packPointer(const void* addr)
{
    mxArray* matlabArray = NULL;

	if (sizeof(void *) == 4)
		matlabArray = NIMEX_memManagement_int32_To_mxArray((int32)addr);//TO010707C
	else if (sizeof(void *) == 8)
		matlabArray = NIMEX_memManagement_int64_To_mxArray((int32)addr);//TO010707C
	else
		mexErrMsgText("Compiling for an unrecognized number of bits (%d), must be 32 or 64.", sizeof(void *));
    //mexMakeArrayPersistent(matlabArray);//TO080813E - Not necessary?

    NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_packPointer - %d into Matlab array. %d->%d->%d\n", (long)addr, (long)matlabArray, (long)&addr, (long)addr)

    return matlabArray;
}

/**
 * @brief Convert an mxArray into a 32-bit pointer.
 *
 * @arg <tt>matlabArray</tt> - An mxArray which contains a C-level pointer.
 *
 * @return A pointer.
 *
 * @callgraph
 * @callergraph
 */
//TO080813F - 64-bit pointer handling.
void* NIMEX_unpackPointer(const mxArray* matlabArray)
{
    void*   ptr = NULL;
    int32*  addr32 = NULL;
	int64*  addr64 = NULL;
    
	if (sizeof(void *) == 4)
	{
		addr32 = (int32 *)mxGetData(matlabArray);
		ptr = (void *)*addr32;
	    if (addr32 == NULL)
	    {
			NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_unpackPointer - NULL out of %d\n", (long)matlabArray)
			return NULL;
		}
		NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_unpackPointer - %d from Matlab array. %d->%d->%d\n", (long)ptr, (long)matlabArray, (long)addr32, (long)ptr)
	}
	else if (sizeof(void *) == 8)
	{
		addr64 = (int64 *)mxGetData(matlabArray);
		ptr = (void *)*addr64;
	    if (addr64 == NULL)
	    {
			NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_unpackPointer - NULL out of %d\n", (long)matlabArray)
			return NULL;
		}
		NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_unpackPointer - %d from Matlab array. %d->%d->%d\n", (long)ptr, (long)matlabArray, (long)addr64, (long)ptr)
	}
	else
		mexErrMsgText("Compiling for an unrecognized number of bits (%d), must be 32 or 64.", sizeof(void *));
    
    return ptr;
}

/**
 * @brief Retrieve the memory manager from shared memory.
 *
 * @return The memory manager.
 *
 * @callgraph
 * @callergraph
 */
//TO082807A - Retrieve a NIMEX_Global pointer from shared memory, create the memory if necessary.
NIMEX_Global* NIMEX_getGlobalFromSharedMemory(void)
{
    HANDLE                  memoryMappedFile = (HANDLE)NULL;
    NIMEX_memoryMap*           memoryMap = (NIMEX_memoryMap *)NULL;

    memoryMappedFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, NIMEX_MEMORYMAP_NAME);
    if (memoryMappedFile == NULL)
        memoryMappedFile = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(NIMEX_memoryMap), NIMEX_MEMORYMAP_NAME);
    
    memoryMap = (NIMEX_memoryMap *)(MapViewOfFile(memoryMappedFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(NIMEX_memoryMap)));
    
    CloseHandle(memoryMappedFile);
    
    return memoryMap->ptr;
}

/**
 * @brief Place the memory manager int shared memory.
 *
 * @arg <tt>global</tt> - The memory manager.
 *
 * @callgraph
 * @callergraph
 */
//TO082807A - Retrieve a NIMEX_Global pointer from shared memory, create the memory if necessary.
void NIMEX_setNimexGlobalIntoSharedMemory(NIMEX_Global* global)
{
    HANDLE              memoryMappedFile = (HANDLE)NULL;
    NIMEX_memoryMap*    memoryMap = (NIMEX_memoryMap *)NULL;

    memoryMappedFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, NIMEX_MEMORYMAP_NAME);
    if (memoryMappedFile == NULL)
        memoryMappedFile = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(NIMEX_memoryMap), NIMEX_MEMORYMAP_NAME);

    memoryMap = (NIMEX_memoryMap *)(MapViewOfFile(memoryMappedFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(NIMEX_memoryMap)));
    memoryMap->ptr = global;

    CloseHandle(memoryMappedFile);

    return;
}

/**
 * @brief Creates the memory manager.
 *
 * @return The memory manager.
 *
 * @callgraph
 * @callergraph
 */
NIMEX_Global* NIMEX_createNimexGlobal(void)
{
    NIMEX_Global* global;
    
    global = calloc(1, sizeof(NIMEX_Global));
    
    //Thread synchronization primitive.
    global->criticalSection = (CRITICAL_SECTION *)calloc(sizeof(CRITICAL_SECTION), 1);
    InitializeCriticalSection(global->criticalSection);
    
    //Thread.
    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(),
        &global->matlabThread, 0, FALSE, DUPLICATE_SAME_ACCESS);
    global->matlabThreadId = GetCurrentThreadId();
    
    //Simple memory manager.
    global->memoryManager = g_ptr_array_new();
    
    //Object-oriented memory manager.
    global->objectManager = g_ptr_array_new();
    
    //Install message pump hook, to handle processing of NIMEX messages into the Matlab thread.
    //Do the install when the first callback is registered, otherwise leave it as NULL.
    //The hook will remain installed until NIMEX is freed.
    global->messagePumpHookID = NULL;//SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)<INSERT_FUNCTION_POINTER_HERE>, NULL, GetCurrentThreadId());
    
    return global;
}

/**
 * @brief Retrieve the memory manager.
 *
 * @return The memory manager.
 *
 * @callgraph
 * @callergraph
 */
//TO082807A - Retrieve a NIMEX_Global pointer from shared memory, create the memory if necessary.
NIMEX_Global* NIMEX_getNimexGlobal(void)
{
    NIMEX_Global* global = (NIMEX_Global *)NULL;
    
    if (NIMEX_GLOBAL == NULL)
    {
        NIMEX_GLOBAL = NIMEX_getGlobalFromSharedMemory();
        if (NIMEX_GLOBAL == NULL)
        {
            //Initialize.
            NIMEX_GLOBAL = NIMEX_createNimexGlobal();
            
            NIMEX_setNimexGlobalIntoSharedMemory(NIMEX_GLOBAL);
        }
    }
    
    return NIMEX_GLOBAL;
}

/**
 * @brief Register a Windows GUI hook function.
 *
 * @arg <tt>addr</tt> - A Windows GUI hook function.
 *
 * @see NIMEX_Callbacks.c
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_setWindowsHookEx(HOOKPROC hookProcedure)
{
    NIMEX_Global* global;

    global = NIMEX_getNimexGlobal();

    if (global->messagePumpHookID == NULL)
        global->messagePumpHookID = SetWindowsHookEx(WH_GETMESSAGE, hookProcedure, NULL, GetCurrentThreadId());
    else
        mexErrMsgTxt("Global Windows message pump hook already installed.");

    return;
}

/**
 * @brief Get the ID of the registered NIMEX hook function.
 *
 * @return The ID of the registered hook function.
 *
 * @see NIMEX_Callbacks.c
 *
 * @callgraph
 * @callergraph
 */
HHOOK NIMEX_getWindowsHookId()
{
    NIMEX_Global* global;
    
    global = NIMEX_getNimexGlobal();
    return global->messagePumpHookID;
}

/**
 * @brief Retrieves the handle to the main Matlab thread.
 *
 * @return The handle of the main Matlab thread.
 *
 * @callgraph
 * @callergraph
 */
HANDLE NIMEX_getMatlabThread()
{
    NIMEX_Global* global;
    
    global = NIMEX_getNimexGlobal();
    return global->matlabThread;
}

/**
 * @brief Retrieves the ID of the main Matlab thread.
 *
 * @return The ID of the main Matlab thread.
 *
 * @callgraph
 * @callergraph
 */
DWORD NIMEX_getMatlabThreadId()
{
    NIMEX_Global* global;
    
    global = NIMEX_getNimexGlobal();
    return global->matlabThreadId;
}

/**
 * @brief Enter critical section, for multithreaded access to the memory manager.
 *
 * Each call to NIMEX_EnterGlobalCriticalSection must be paired with a call to NIMEX_LeaveGlobalCriticalSection.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_EnterGlobalCriticalSection()
{
    NIMEX_Global* global;
    
    global = NIMEX_getNimexGlobal();
    EnterCriticalSection(global->criticalSection);
    NIMEX_MACRO_VERBOSE_3("NIMEX_EnterGlobalCriticalSection() - global->criticalSection = @%p->@%p\n", global, global->criticalSection);
}

/**
 * @brief Leave critical section, for multithreaded access to the memory manager.
 *
 * Each call to NIMEX_EnterGlobalCriticalSection must be paired with a call to NIMEX_LeaveGlobalCriticalSection.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_LeaveGlobalCriticalSection()
{
    NIMEX_Global* global;
    
    global = NIMEX_getNimexGlobal();
    NIMEX_MACRO_VERBOSE_3("NIMEX_LeaveGlobalCriticalSection() - global->criticalSection = @%p->@%p\n", global, global->criticalSection);
    LeaveCriticalSection(global->criticalSection);
}

/**
 * @brief Iterator function for destruction of registered objects.
 *
 * @arg <tt>data</tt> - Pointer to the object to be destroyed.
 * @arg <tt>userdata</tt> - Not used.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_GPtrArray_GFunc_Iterator_destroy(void* data, void* userdata)
{
    NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_GPtrArray_GFunc_Iterator_destroy - Calling destructor for @%p...\n", data);
    ((NIMEX_destroyableObject *)data)->destructor(((NIMEX_destroyableObject *)data)->ptr);

    //JL011608A: move the NIMEX_MACRO_FREE from each destroyableObject -> destructor to here to avoid segmentation violation caused by double free
    if (((NIMEX_destroyableObject *)data)->quickdestroy == FALSE)
    {
        NIMEX_MACRO_VERBOSE("NIMEX_memManagement: Free destroyable object - @%p\n", ((NIMEX_destroyableObject *)data)->ptr);
        
        NIMEX_MACRO_FREE(((NIMEX_destroyableObject *)data)->ptr);
        ((NIMEX_destroyableObject *)data)->ptr = NULL;
    }
    
    //JL011408A: To detroy the destroyable objects
    NIMEX_freeDestroyableMemorySegment((NIMEX_destroyableObject *)data);
}

/**
 * @brief Iterator function for freeing of registered memory.
 *
 * @arg <tt>data</tt> - Pointer to the memory to be released.
 * @arg <tt>userdata</tt> - Not used.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_GPtrArray_GFunc_Iterator_free(void* data, void* userdata)
{
    NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_GPtrArray_GFunc_Iterator_free - Freeing @%p...\n", data);
    //JL011708B use NIMEX_freePersistentMemorySegment to replace NIMEX_MACRO_FREE(data)
    //NIMEX_MACRO_FREE(data);
    NIMEX_freePersistentMemorySegment(data);
}

/**
 * @brief Destroy the memory manager.
 *
 * @arg <tt>global</tt> - The memory manager.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_destroyNimexGlobal(NIMEX_Global* global)
{
    gpointer* result1, result2;
    
    EnterCriticalSection(global->criticalSection);
    //Clean up objects.
    NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement/NIMEX_destroyNimexGlobal: Cleaning up objects...\n");
    
    //JL011508A: g_ptr_array_foreach failed to free all elements in GptrArray, so add this while statement 
    while (global->objectManager->len > 0) 
    g_ptr_array_foreach(global->objectManager, &NIMEX_GPtrArray_GFunc_Iterator_destroy, NULL);
    
    //JL011508B: add this to free persistent memory 
    while (global->memoryManager->len > 0)
    g_ptr_array_foreach(global->memoryManager, &NIMEX_GPtrArray_GFunc_Iterator_free, NULL);
    
    result1 = g_ptr_array_free(global->objectManager, TRUE);
    NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_destroyNimexGloba: result1 = @%p...\n", result1);
    
    NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement/NIMEX_destroyNimexGlobal: Clearing tracked memory segments...\n");
    result2 = g_ptr_array_free(global->memoryManager, TRUE);
    NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_destroyNimexGloba: result2 = @%p...\n", result2);

    if (global->messagePumpHookID != NULL)
    {
        NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement/NIMEX_destroyNimexGlobal: Unhooking Windows message pump hook function.\n");
        UnhookWindowsHookEx(global->messagePumpHookID);
    }

    LeaveCriticalSection(global->criticalSection);

    NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement/NIMEX_destroyNimexGlobal: Destroying global thread synchronization primitive.\n");
    DeleteCriticalSection(global->criticalSection);
    
    //JL011508C: to release global->criticalSection
    NIMEX_MACRO_FREE(global->criticalSection);
    
    NIMEX_MACRO_FREE(global);

    return;
}

/**
 * @brief Retrieve the flat memory manager.
 *
 * @return The memory manager.
 *
 * @callgraph
 * @callergraph
 */
//TO061307A - Retrieve a NIMEX_memoryManager pointer from shared memory, create the memory if necessary.
NIMEX_PersistenceList* NIMEX_getMemoryManager(void)
{
    NIMEX_Global* memoryMap = (NIMEX_Global *)NULL;
    
    memoryMap = NIMEX_getNimexGlobal();
    //TO101107B - Do not allow access to the memory manager from outside the Matlab thread. -- Tim O'Connor 10/11/07
    if (memoryMap->matlabThreadId != GetCurrentThreadId())
    {
        mexPrintf("\n*****\nWARNING: NIMEX_getMemoryManager detected attempted access to NIMEX's Matlab memory manager from outside the Matlab thread.\n"
                           "         Access denied. Returning NULL.\n*****\n");
        return NULL;
    }
    
    return memoryMap->memoryManager;
}

/**
 * @brief Retrieve the object memory manager.
 *
 * @return The object memory manager.
 *
 * @callgraph
 * @callergraph
 */
NIMEX_PersistenceList* NIMEX_getObjectManager(void)
{
    NIMEX_Global* memoryMap = (NIMEX_Global *)NULL;
    
    memoryMap = NIMEX_getNimexGlobal();
    //TO101107B - Do not allow access to the object manager from outside the Matlab thread. -- Tim O'Connor 10/11/07
    if (memoryMap->matlabThreadId != GetCurrentThreadId())
    {
        mexPrintf("\n*****\nWARNING: NIMEX_getObjectManager detected attempted access to NIMEX's Matlab object memory manager from outside the Matlab thread.\n"
                           "         Access denied. Returning NULL.\n*****\n");
        return NULL;
    }
    
    return memoryMap->objectManager;
}

/*//TO061307A - Insert a NIMEX_memoryManager pointer into shared memory, create the memory if necessary.
void NIMEX_setMemoryManager(NIMEX_PersistenceList* memoryManager)
{
    HANDLE                  memoryMappedFile = (HANDLE)NULL;
    NIMEX_memoryManager*    memoryMap = (NIMEX_memoryManager *)NULL;

    memoryMappedFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, NIMEX_MEMORYMAP_NAME);
    if (memoryMappedFile == NULL)
        memoryMappedFile = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(NIMEX_memoryManager), NIMEX_MEMORYMAP_NAME);

    memoryMap = (NIMEX_memoryManager *)(MapViewOfFile(memoryMappedFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(NIMEX_memoryManager)));
    memoryMap->ptr = memoryManager;

    CloseHandle(memoryMappedFile);

    return;
}*/

/**
 * @brief Initialize the memory manager.
 *
 * @return A newly initialized memory manager.
 *
 * @callgraph
 * @callergraph
 */
NIMEX_PersistenceList*  NIMEX_createMemoryManager(void)
{
    NIMEX_PersistenceList* memManager = g_ptr_array_new();
    g_ptr_array_add(memManager, g_ptr_array_new());//Add the object manager.
    return memManager;
}

/*
 *
 *
NIMEX_PersistenceList* NIMEX_getMemoryManager(void)
{
    //TO061307A - Heavily modified to use shared memory.
    NIMEX_PersistenceList*      destroyableObjects;
    int32*                      listAddress = 0;
    NIMEX_PersistenceList*      memoryManager = (NIMEX_PersistenceList *)NULL;

    if (NIMEX_GLOBAL == NULL)
    {
        NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_getMemoryManager - NIMEX_GLOBAL: NULL\n")
        //To ensure consistency across MEX-Files, this variable must be in some "shared" memory space.
        //This would either require (for Windows) using shared memory (which involves a memory mapped file scheme)
        //or just storing the pointer in Matlab, which seems cleanest (for now). Of course, if the global
        //space gets cleared, this will totally fail. Another (inefficient) option might be to dump to a real file.
//        globalVariable = mexGetVariable("global", "NIMEX_GLOBAL");
        memoryManager = NIMEX_getMemoryManagerFromSharedMemory();
        if (memoryManager == NULL)
        {
          NIMEX_GLOBAL->memoryManager = NIMEX_createMemoryManager();
          NIMEX_memManagement_log("allocated NIMEX_GLOBAL: @%p\n", NIMEX_GLOBAL);
          NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_getMemoryManager - Creating new NIMEX_GLOBAL in Matlab's global variable space:@%p\n", NIMEX_GLOBAL)
          NIMEX_setMemoryManager(NIMEX_GLOBAL);
        }
        else
        {
            NIMEX_GLOBAL = memoryManager;
            NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_getMemoryManager (unpacked) - NIMEX_GLOBAL_PERSISTENCE_LIST = @%p\n", NIMEX_GLOBAL_PERSISTENCE_LIST)
            if ((NIMEX_GLOBAL_PERSISTENCE_LIST == NULL) || (NIMEX_GLOBAL_PERSISTENCE_LIST == 0))
            {
                NIMEX_GLOBAL->memoryManager = NIMEX_createMemoryManager();
                NIMEX_memManagement_log("allocated NIMEX_GLOBAL_PERSISTENCE_LIST: @%p\n", NIMEX_GLOBAL_PERSISTENCE_LIST);
                NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_getMemoryManager - Replacing null-pointer for NIMEX_GLOBAL_PERSISTENCE_LIST in Matlab's global variable space: @%p\n", NIMEX_GLOBAL_PERSISTENCE_LIST)
                NIMEX_setMemoryManager(NIMEX_GLOBAL_PERSISTENCE_LIST);
            }
            else
            {
                NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_getMemoryManager - Retrieved NIMEX_GLOBAL_PERSISTENCE_LIST from shared memory space: @%p\n", NIMEX_GLOBAL_PERSISTENCE_LIST)
            }
        }
    }
    
    return NIMEX_GLOBAL_PERSISTENCE_LIST;
}*/

/**
 * @brief Register an object and its destructor.
 *
 * @arg <tt>ptr</tt> - The object to be registered.
 * @arg <tt>destructor</tt> - The destructor function for the object.
 * @arg <tt>quickdestroy</tt> - ...
 *
 * @return 0 if successful, non-zero otherwise.
 *
 * @callgraph
 * @callergraph
 */
//JL011808A: Add quickdestroy field. If quickdestroy is true, won't call NIMEX_MACRO_FREE(data->ptr) in NIMEX_GPtrArray_GFunc_Iterator_destroy. This is used for linked list and hash table.
//TO061207A
int32 NIMEX_addDestroyableMemorySegment(void* ptr, NIMEX_destructor destructor, gboolean quickdestroy)
{
    int32                       status = 0;
    NIMEX_PersistenceList*      persistenceList;
    NIMEX_destroyableObject*    destroyableObject;

    persistenceList = NIMEX_getObjectManager();
    NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_addDestroyableMemorySegment - retrieved object manager: @%p\n", persistenceList)
    if (persistenceList != NULL)
    {
        destroyableObject = (NIMEX_destroyableObject *)calloc(1, sizeof(NIMEX_destroyableObject));
        //NIMEX_MACRO_VERBOSE_4("********************************************************************************************\n")
        NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_addDestroyableMemorySegment - Created NIMEX_destroyableObject* = @%p\n", destroyableObject)
        destroyableObject->ptr = ptr;
        destroyableObject->destructor = destructor;
        destroyableObject->quickdestroy = quickdestroy;
        NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_addDestroyableMemorySegment - @%p:\n", destroyableObject)
        NIMEX_MACRO_VERBOSE_4("                                                         @%p->ptr = @%p\n", destroyableObject, destroyableObject->ptr)
        NIMEX_MACRO_VERBOSE_4("                                                         @%p->destructor = @%p\n", destroyableObject, destroyableObject->destructor)
        g_ptr_array_add(persistenceList, destroyableObject);
        NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: NIMEX_addDestroyableMemorySegment - Tracking object: @%p\n", ptr)
    }

    return status;
}

/**
 * @brief Unregister an object and its destructor.
 *
 * @arg <tt>ptr</tt> - The object to be registered.
 *
 * @return 0 if successful, non-zero otherwise.
 *
 * @callgraph
 * @callergraph
 */
//JL011108A: Add this function to freeDestroyalbeMemorySegement
int32 NIMEX_freeDestroyableMemorySegment(NIMEX_destroyableObject * ptr)
{
    int32                       status = 0;
    NIMEX_PersistenceList*      persistenceList;
    gboolean                    foundptr;

    if (ptr == NULL)
    {
        NIMEX_memManagement_log("NIMEX_memManagement/NIMEX_freeDestroyableMemorySegment: Attempt to free NULL pointer.\n");
        mexPrintf("NIMEX_memManagement/NIMEX_DestroyableMemorySegment: Attempt to free NULL pointer.\n");
    }
    
    persistenceList = NIMEX_getObjectManager();
    NIMEX_memManagement_log("NIMEX_memManagement persistenceList = @%p\n", persistenceList);
    if (persistenceList == NULL)
    {
        mexPrintf("NIMEX_DestoyableMemorySegment Warning: All memory appears to be freed.\n");
        return 0;
    }

    foundptr = g_ptr_array_remove_fast(persistenceList, ptr);
    NIMEX_memManagement_log("NIMEX_memManagement freeing @%p\n", ptr );
    //Clear the memory segment.

    if (foundptr == TRUE){
        NIMEX_MACRO_FREE(ptr);
        NIMEX_memManagement_log("NIMEX_memManagement successfully freed the destroyable ptr=@%p\n", ptr);
    }
    else{
        NIMEX_memManagement_log("NIMEX_memManagement cannot find the destroyable ptr=@%p\n", ptr);
    }
    
    
    return status;
}

/**
 * @brief Register flat memory.
 *
 * @arg <tt>ptr</tt> - The memory to be registered.
 *
 * @return 0 if successful, non-zero otherwise.
 *
 * @callgraph
 * @callergraph
 */
int32 NIMEX_addPersistentMemorySegment(void* ptr)
{
    int32                       status = 0;
    NIMEX_PersistenceList*      persistenceList;

    persistenceList = NIMEX_getMemoryManager();
    
    if (persistenceList != NULL)
    {
        g_ptr_array_add(persistenceList, ptr);
        NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: addPersistentMemory - Tracking memory segment: @%p\n", ptr)
    }
    
    return status;
}

/**
 * @brief Unregister and free flat memory.
 *
 * @arg <tt>ptr</tt> - The memory to be released.
 *
 * @return 0 if successful, non-zero otherwise.
 *
 * @callgraph
 * @callergraph
 */
int32 NIMEX_freePersistentMemorySegment(void* ptr)
{
    int32                       status = 0;
    NIMEX_PersistenceList*      persistenceList;
    mxArray*                    globalVariable = (mxArray *)NULL;
    gboolean                    foundptr;
    
    //TO040207D
    if (ptr == NULL)
    {
        NIMEX_memManagement_log("NIMEX_memManagement/NIMEX_freePersistentMemorySegment: Attempt to free NULL pointer.\n");
        mexPrintf("NIMEX_memManagement/NIMEX_freePersistentMemorySegment: Attempt to free NULL pointer.\n");
    }
    
    persistenceList = NIMEX_getMemoryManager();
    NIMEX_memManagement_log("NIMEX_memManagement persistenceList = @%p\n", persistenceList);
    if (persistenceList == NULL)
    {
        mexPrintf("NIMEX_freePersistentMemorySegment Warning: All memory appears to be freed.\n");
        return 0;
    }
    
    foundptr = g_ptr_array_remove_fast(persistenceList, ptr);
    
    NIMEX_memManagement_log("NIMEX_memManagement freeing @%p\n", ptr);
    
    //JL011508D: Add if statement to aviod double free.
    if (foundptr == TRUE){
        NIMEX_MACRO_FREE(ptr);
        NIMEX_memManagement_log("NIMEX_memManagement successfully freed. ptr=@%p\n", ptr);
    }
    else{
        NIMEX_memManagement_log("NIMEX_memManagement cannot find the ptr=@%p\n", ptr);
    }
   
    return status;
}

/**
 * @brief Allocate and register a block of memory.
 *
 * @arg <tt>n</tt> - The number of blocks to reserve.
 * @arg <tt>size</tt> - The size of each block to be allocated.
 *
 * @return The pointer to the newly allocated memory, NULL if unsuccessful.
 *
 * @callgraph
 * @callergraph
 */
void* NIMEX_Persistent_Calloc(size_t n, size_t size)
{
    void* ptr = (void *)NULL;
    ptr = calloc(n, size);
    
    //NIMEX_MACRO_VERBOSE_4("********************************************************************************************\n")
    NIMEX_memManagement_log("allocated memory: @%p\n", ptr);

    NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: Persistent_Calloc - Creating persistent memory segment: @%p\n", ptr)

    NIMEX_addPersistentMemorySegment(ptr);
    NIMEX_memManagement_log("NIMEX_Persistent_Calloc allocated: @%p\n", ptr);
    return ptr;
}

/**
 * @brief Tear down all of NIMEX.
 *
 * This is the registered with Matlab to be called on shutdown or calls to `clear mex`.
 *
 * @post All objects are destroyed and all memory is released.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_mexAtExit(void)
{
    NIMEX_Global*            global;
    NIMEX_PersistenceList*   persistenceList;
    //NIMEX_PersistenceList*   objectList;
    //mxArray*                 globalVariable = (mxArray *)NULL;

    NIMEX_MACRO_VERBOSE_1("NIMEX_memManagement: mexAtExit!\n")
    global = NIMEX_getNimexGlobal();
    if (global == NULL)
    {
        mexPrintf("NIMEX_memManagement: mexAtExit -  Warning: All memory appears to be freed.\n");
        return;
    }
    
    NIMEX_destroyNimexGlobal(global);

    NIMEX_setNimexGlobalIntoSharedMemory(NULL);
    NIMEX_GLOBAL = NULL;
    NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: mexAtExit - Set NIMEX global to NULL.\n")
    
    NIMEX_memManagement_log_finalize
    //NIMEX_MACRO_VERBOSE_4("NIMEX_memManagement: mexAtExit - NIMEX_GLOBAL_PERSISTENCE_LIST = %d\n", NIMEX_GLOBAL_PERSISTENCE_LIST)

    return;
}
/*
 *
 */
 /*
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    int     numAllocations = 10;//10000;
    int     allocationSize = 256;
    void**  pointers;
    
    pointers = mxCalloc(numAllocations, sizeof(void*));
    
    mexPrintf("NIMEX_memManagement allocating %d blocks of %d byes.\n", numAllocations, allocationSize);
    for (int i = 0; i < numAllocations; i++)
        pointers[i] = (void *)NIMEX_Persistent_Calloc(1, allocationSize);

    mexPrintf("NIMEX_memManagement deallocating %d blocks of %d byes.\n", numAllocations, allocationSize);
    for (int i = 0; i < numAllocations; i++)
        NIMEX_freePersistentMemorySegment(pointers[i]);
    
    mxFree(pointers);
    
    NIMEX_mexAtExit();
    
    return;
 }*/
