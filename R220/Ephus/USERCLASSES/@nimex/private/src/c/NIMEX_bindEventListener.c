/**
 * @file NIMEX_bindEventListener.c
 *
 * @brief Attach a Matlab function to be called when the specified NIMEX event occurs.
 *
 * Allowable event names are defined in NIMEX_Constants.h.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_bindEventListener(task, eventName, callback, name, priority)</tt>
 *  @arg <tt>task</tt> - The task whose lock to acquire.
 *  @arg <tt>eventName</tt> - The event to which to bind a callback.
 *  @arg <tt>callback</tt> - A valid Matlab function_handle or cell array whose first element is a function_handle.
 *  @arg <tt>name</tt> - The unique name for the callback.
 *  @arg <tt>priority</tt> - The priority of the callback, with lower numbers being higher priority.
 *
 * This should now obsolete NIMEX_bindEveryNCallback and NIMEX_bindDoneCallback.
 *
 * @see NIMEX_Callback
 * @see NIMEX_Callbacks.c
 * @see NIMEX_Constants.h
 *
 * @author Timothy O'Connor
 * @date 10/18/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 *
 */
/*
 * NOTES
 *  See TO101807B for related changes.
 *
 * CHANGES
 *
 *
 */
#include <string.h>
#include "nimex.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    NIMEX_TaskDefinition*       taskDefinition = (NIMEX_TaskDefinition *)NULL;
    NIMEX_Callback*             callback = (NIMEX_Callback *)NULL;
    mxArray*                    persistentMatlabCallback = (mxArray *)NULL;
    char*                       channelName = (char *)NULL;
    char*                       eventName;
    
    NIMEX_MACRO_PRE_MEX

    if (nrhs != 5)
        mexErrMsgTxt("NIMEX_bindEventListener: 5 arguments (task, eventName, callback, name, priority) are required.");
    
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_bindEventListener: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

    //Verify the eventName as being valid before allocating resources.
    eventName = mxArrayToString(prhs[1]);
    if ( (strcmp(eventName, NIMEX_EVENT_DONE) != 0) && (strcmp(eventName, NIMEX_EVENT_EVERY_N) != 0)
        && (strcmp(eventName, NIMEX_EVENT_CYCLE_POSITION_BEGIN) != 0) && (strcmp(eventName, NIMEX_EVENT_CYCLE_POSITION_END) != 0)
        && (strcmp(eventName, NIMEX_EVENT_CYCLE_START) != 0) && (strcmp(eventName, NIMEX_EVENT_CYCLE_STOP) != 0)
        && (strcmp(eventName, NIMEX_EVENT_CYCLE_COMPLETE) != 0) && (strcmp(eventName, NIMEX_EVENT_ABORT) != 0)
        && (strcmp(eventName, NIMEX_EVENT_ERROR) != 0) )
    {
        mexPrintf("NIMEX_bindEventListener: '%s' is not a valid event (event names are case-sensitive).\n", eventName);
        mexErrMsgTxt("NIMEX_bindEventListener: Unrecognized event name.");
    }

    //TO012407C
    //TO101807D - This had been checking for != instead of ==.
    if (taskDefinition->eventMap == NULL)
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_bindEventListener: Initializing new NIMEX_CallbackMap...\n")
        taskDefinition->eventMap = NIMEX_CallbackMap_create();//TO062007B
        //taskDefinition->everyNCallbacks = NIMEX_linkedList_create();//TO040207E
    }
    
    //TO062007B
    NIMEX_MACRO_VERBOSE_2("NIMEX_bindEventListener: Creating new NIMEX_Callback...\n")
    callback = NIMEX_Callback_create(mxArrayToString(prhs[3]), (int32)*(mxGetPr(prhs[4])), prhs[2]);
    NIMEX_MACRO_VERBOSE_2("NIMEX_bindEventListener: Inserting NIMEX_Callback (@%p) into NIMEX_CallbackMap (@%p)...\n", callback, taskDefinition->eventMap)
    
    NIMEX_CallbackMap_insert(taskDefinition->eventMap, eventName, callback);
    /*persistentMatlabCallback = mxDuplicateArray(prhs[1]);//Watch out for memory leaks here.
    mexMakeArrayPersistent(persistentMatlabCallback);
    callback->callbackToMatlab = persistentMatlabCallback;
    callback->name = NIMEX_MACRO_MXARRAY_2_PERSISTENT_CHAR(prhs[2]);
    callback->priority = (int32)*(mxGetPr(prhs[3]));
    NIMEX_linkedList_append(taskDefinition->everyNCallbacks, callback);*/

    NIMEX_MACRO_POST_MEX

    return;
}
