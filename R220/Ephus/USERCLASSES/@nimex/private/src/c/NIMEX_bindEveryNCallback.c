/**
 * @file NIMEX_bindEveryNCallback.c
 * @brief Deprecated, see NIMEX_bindEventListener.
 * @deprecated Use NIMEX_bindEventListener instead.
 * @see NIMEX_bindEventListener.c
 */
/*
 * NIMEX_bindEveryNCallback.c
 *
 * Attach a Matlab function to be called when the NIDAQmx everyNCallback event occurs.
 *
 * See NIMEX_Structures.h for more information.
 *
 * CHANGES
 *  TO012407C: Finished testing initial implementation of callbacks (see TO012407B). -- Tim O'Connor 1/24/07
 *  TO012907C: Moved properties from the NIMEX_ChannelDefinition to the  NIMEX_TaskDefinition, to better align with NIDAQmx. -- Tim O'Connor 1/29/07
 *  TO022807C - Added mexFunction pre and post macros. -- Tim O'Connor 2/28/07
 *  TO040207E: Use the constructor for initializing everyNCallbacks. -- Tim O'Connor 4/2/07
 *  TO062007B: New callback handling system. Make use of the TO061907B and TO061907C changes. -- Tim O'Connor 6/20/07
 *  TO101807B: Created NIMEX_bindEventListener, which obsoletes NIMEX_bindEveryNCallback & NIMEX_bindDoneCallback. -- Tim O'Connor 10/18/07
 *
 * Created
 *  Timothy O'Connor 1/7/07
 *
 * Copyright
 *  Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007
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
    
    NIMEX_MACRO_PRE_MEX

    mexErrMsgTxt("NIMEX_bindEveryNCallback: Deprecated. Use NIMEX_bindEventListener instead.");//TO101807B
    if (nrhs != 4)
        mexErrMsgTxt("NIMEX_bindEveryNCallback: 4 arguments (task, callback, name, priority) are required.");
    
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_bindEveryNCallback: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

//    callback = (NIMEX_Callback *)NIMEX_Persistent_Calloc(1, sizeof(NIMEX_Callback));
    //TO012407C
    if (taskDefinition->eventMap == NULL)
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_bindEveryNCallback: Initializing new NIMEX_CallbackMap...\n")
        taskDefinition->eventMap = NIMEX_CallbackMap_create();//TO062007B
        //taskDefinition->everyNCallbacks = NIMEX_linkedList_create();//TO040207E
    }

    //TO062007B
    NIMEX_MACRO_VERBOSE_2("NIMEX_bindEveryNCallback: Creating new NIMEX_Callback...\n")
    callback = NIMEX_Callback_create(mxArrayToString(prhs[2]), (int32)*(mxGetPr(prhs[3])), prhs[1]);
    NIMEX_MACRO_VERBOSE_2("NIMEX_bindEveryNCallback: Inserting NIMEX_Callback (@%p) into NIMEX_CallbackMap (@%p)...\n", callback, taskDefinition->eventMap)
    
    NIMEX_CallbackMap_insert(taskDefinition->eventMap, NIMEX_EVENT_EVERY_N, callback);
    /*persistentMatlabCallback = mxDuplicateArray(prhs[1]);//Watch out for memory leaks here.
    mexMakeArrayPersistent(persistentMatlabCallback);
    callback->callbackToMatlab = persistentMatlabCallback;
    callback->name = NIMEX_MACRO_MXARRAY_2_PERSISTENT_CHAR(prhs[2]);
    callback->priority = (int32)*(mxGetPr(prhs[3]));
    NIMEX_linkedList_append(taskDefinition->everyNCallbacks, callback);*/

    NIMEX_MACRO_POST_MEX
    
    return;
}
