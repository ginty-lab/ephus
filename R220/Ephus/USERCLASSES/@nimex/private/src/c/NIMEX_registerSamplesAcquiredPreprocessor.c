/**
 * @file NIMEX_registerSamplesAcquiredPreprocessor.c
 *
 * @brief Attach a Matlab function to be called when the NIMEX_samplesAcquiredCallback event occurs.
 *
 * The function must take an array of samples as its last argument and return a preprocessed array of samples.
 * The array length may change.
 * The samplesAcquired event is piggybacked on the everyN event.
 * If NIMEX_TaskDefinition::autoDispatch evaluates to false, this event does not occur.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_registerSamplesAcquiredPreprocessr(taskDefinition, channelName, callback)</tt>
 *  @arg <tt>taskDefinition</tt> - The task to which to bind a preprocessor
 *  @arg <tt>channelName</tt> - The physical name of the channel to which to bind a preprocessor.
 *  @arg <tt>callback</tt> - A Matlab function_handle or cell array whose first element is a function_handle.
 *
 * @see NIMEX_Callbacks.c
 *
 * @author Timothy O'Connor
 * @date 7/30/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 */
/*
 * CHANGES
 *
 */
#include <string.h>
#include "nimex.h"

/**
 * @brief Matlab entry point.
 * @callgraph
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    NIMEX_TaskDefinition*       taskDefinition = (NIMEX_TaskDefinition *)NULL;
    NIMEX_Callback*             callback = (NIMEX_Callback *)NULL;
    mxArray*                    persistentMatlabCallback = (mxArray *)NULL;
    char*                       channelName = (char *)NULL;
    
    NIMEX_MACRO_PRE_MEX

    if (nrhs != 5)
        mexErrMsgTxt("NIMEX_registerSamplesAcquiredPreprocessr: 5 arguments (task, channelName, callback, name, priority) are required.");
       
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_registerSamplesAcquiredListener: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

    channelName = mxArrayToString(prhs[1]);
    if (NIMEX_getChannelByName(taskDefinition, channelName) == NULL)
    {
        mexPrintf("NIMEX_registerSamplesAcquiredPreprocessr: Failed to find channel \"%s\".\n", channelName);
        mexErrMsgTxt("NIMEX_registerSamplesAcquiredPreprocessr: No such channel.");
    }

    //TO012407C
    if (taskDefinition->preprocessors == NULL)
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_registerSamplesAcquiredPreprocessr: Initializing new NIMEX_CallbackMap...\n")
        taskDefinition->preprocessors = NIMEX_CallbackMap_create();//TO062007B
    }

    //TO062007B
    NIMEX_MACRO_VERBOSE_2("NIMEX_registerSamplesAcquiredPreprocessr: Creating new NIMEX_Callback...\n")
    callback = NIMEX_Callback_create(mxArrayToString(prhs[3]), (int32)*(mxGetPr(prhs[4])), prhs[2]);
    NIMEX_MACRO_VERBOSE_2("NIMEX_registerSamplesAcquiredPreprocessr: Inserting NIMEX_Callback (@%p) into NIMEX_CallbackMap (@%p:\"%s\")...\n", callback, taskDefinition->dataDispatcher, channelName)
    NIMEX_CallbackMap_insert(taskDefinition->preprocessors, channelName, callback);

    NIMEX_MACRO_POST_MEX
    
    return;
}
