/**
 * @file NIMEX_memManagement.h
 *
 * @brief Function declarations for NIMEX_memManagement.c.
 *
 * @see NIMEX_memManagement.c
 *
 * @author Timothy O'Connor
 * @date 11/16/06
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006</em>
 *
 */
/*
 * FUNCTIONS
 *
 *  int32 NIMEX_addPersistentMemorySegment(void* ptr) - Pushes a pointer onto
 *   the NIDAQmex memory stack, makes memory persistent via mexMakeMemoryPersistent.
 *
 *  int32 NIMEX_freePersistentMemorySegment(void* ptr) - Immediately release a persistent memory segment.
 *
 *  void* NIMEX_Persistent_Calloc(size_t n, size_t size) - Convenience function,
 *   allocates memory via mxCalloc, makes it persistent and registers it for cleanup via NIMEX_addPersistentMemorySegment. 
 *
 *  void NIMEX_mexAtExit(void) - Clears all registered persistent memory in NIDAQmex.
 *
 *  mxArray* NIMEX_packPointer(const void* addr) - Converts a C pointer into an mxArray.
 *
 *  void* NIMEX_unpackPointer(const mxArray* matlabArray) - Converts an mxArray into a C pointer.
 *
 *  void NIMEX_addDestroyableMemorySegment(void* ptr, NIMEX_destructor* destructor) - Register an object that requires cleanup as a persistent memory segment.
 *
 *  void NIMEX_simpleDestructor(void* ptr) - Free the allocated memory, nothing more. Useful for strings.
 *
 *  void NIMEX_setWindowsHookEx(HOOKPROC hookProcedure) - Register to intercept Windows messages, see MSDN's SetWindowsHookEx documentation.
 *
 *  HHOOK NIMEX_getWindowsHookId() - Return the ID of the registered message pump hook (NULL if none is registered), see MSDN's SetWindowsHookEx documentation.
 *
 *  void NIMEX_EnterGlobalCriticalSection() - Provides access to the global thread synchronization primitive. See MSDN's EnterCriticalSection documentation.
 *
 *  void NIMEX_LeaveGlobalCriticalSection() - Provides access to the global thread synchronization primitive. See MSDN's LeaveCriticalSection documentation.
 *
 *  HANDLE NIMEX_getMatlabThread() - Return a handle to the Matlab thread.
 *
 *  DWORD NIMEX_getMatlabThreadId() - Return the Matlab thread's ID.
 *
 * Changes:
 *  TO010607B: Created NIMEX_genericStructures. -- Tim O'Connor 1/6/07
 *  TO010607D: Created NIMEX_memManagement. -- Tim O'Connor 1/6/07
 *  TO061207A: Port basic structures to glib. -- Tim O'Connor 6/12/07
 *  TO061307A: Implement shared memory, instead of a Matlab global variable. -- Tim O'Connor 6/13/07
 *  JL011808A: Add gboolean quickdestroy to structure NIMEX_destroyableObject
 *
 */

#ifndef _NIMEX_MEM_MANAGMENT_h /* Multiple include protection. */
#define _NIMEX_MEM_MANAGMENT_h

#include "nimex.h"

///@brief A pointer array, to track allocated memory.
//TO061207A
typedef GPtrArray NIMEX_PersistenceList;

/**
 * @brief The globally available memory manager.
 */
//TO082807A
typedef struct
{
    ///@brief Simple pointer array.
    NIMEX_PersistenceList* memoryManager;
    ///@brief Object pointer array, which only takes NIMEX_destroyableObject pointers.
    NIMEX_PersistenceList* objectManager;
    ///@brief Cross thread synchronization primitve.
    CRITICAL_SECTION*      criticalSection;
    ///@brief Main Matlab thread.
    HANDLE                 matlabThread;
    ///@brief Thread ID for the main Matlab thread, to ensure consistency.
    DWORD                  matlabThreadId;
    ///@brief ID for unregistering hook function(s).
    HHOOK                  messagePumpHookID;
} NIMEX_Global;

///@brief Memory map access. No longer used, but not yet deprecated.
#define NIMEX_MEMORYMAP_NAME "NIMEX_memManagementMap"

///@brief Equivalent to: `typedef void (*NIMEX_destructor) (void* ptr);`
typedef GDestroyNotify NIMEX_destructor;//Same as `typedef void (*NIMEX_destructor) (void* ptr);`

/**
 * @brief Generic destroyable object, which must have a custom destructor handle.
 */
//TO061207A - Allow complex structures to be managed separately (and appropriately destroyed at release).
typedef struct
{
  ///@brief Pointer to the destroyable structure.
  void* ptr;
  ///@brief Class destructor.
  NIMEX_destructor destructor;
  ///@brief Flag for quick destruction.
  ///@todo Examine the NIMEX_destroyableObject::quickdestroy scheme, which is probably unnecessary.
  gboolean quickdestroy;
} NIMEX_destroyableObject;

/**
 * @brief Memory map/shared memory access. Scheduled for deprecation.
 */
typedef struct
{
    NIMEX_Global* ptr;
} NIMEX_memoryMap;

void NIMEX_simpleDestructor(void* ptr);//Simply frees the memory.

//TO061207A
int32 NIMEX_addDestroyableMemorySegment(void* ptr, NIMEX_destructor destructor, gboolean quickdestroy);

/*
 *
 */
int32 NIMEX_addPersistentMemorySegment(void* ptr);

/*
 *
 */
int32 NIMEX_freePersistentMemorySegment(void* ptr);

/*
 *
 */
int32 NIMEX_freeDestroyableMemorySegment(void* ptr);

/*
 *
 */
void* NIMEX_Persistent_Calloc(size_t, size_t);

/*
 *
 */
void NIMEX_mexAtExit(void);

/*
 *
 */
mxArray* NIMEX_packPointer(const void* addr);

/*
 *
 */
void* NIMEX_unpackPointer(const mxArray* matlabArray);

/*
 *
 */
void NIMEX_setWindowsHookEx(HOOKPROC hookProcedure);

/*
 *
 */
HHOOK NIMEX_getWindowsHookId();

/*
 *
 */
HANDLE NIMEX_getMatlabThread();

/*
 *
 */
DWORD NIMEX_getMatlabThreadId();

/*
 *
 */
void NIMEX_EnterGlobalCriticalSection();

/*
 *
 */
void NIMEX_LeaveGlobalCriticalSection();

#endif /* End multiple include protection. */
