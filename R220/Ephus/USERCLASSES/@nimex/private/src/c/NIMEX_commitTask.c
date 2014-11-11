/**
 * @file NIMEX_commitTask.c
 *
 * @brief Commits the task, as per NIDAQmx documentation.
 *
 * Additional operations, such as accessing callbacks in data sources, may be performed as necessary.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_clearBuffer(taskDefinition, channelName)</tt>
 *  @arg <tt>taskDefinition</tt> - The task whose lock to acquire.
 *  @arg <tt>channelName</tt> - The physical name of the channel whose buffer to clear.
 *
 * @author Timothy O'Connor
 * @date 10/18/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 */
/*
 * NOTES
 *  See TO101708J.
 *
 * CHANGES
 *
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
    int32 error = 0;
    char errBuff[2048] = {'\0'};
    NIMEX_TaskDefinition* taskDefinition = (NIMEX_TaskDefinition *)NULL;

    NIMEX_MACRO_PRE_MEX
    
    if (nrhs != 1)
        mexErrMsgTxt("NIMEX_commitTask: Insufficient arguments (the pointer to the NIMEX_TaskDefinition is required).");
    
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_commitTask: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);

    NIMEX_MACRO_VERBOSE("NIMEX_commitTask: Committing task...\n");

    error = NIMEX_CommitTask(taskDefinition);
    
	if (DAQmxFailed(error))
    {
		DAQmxGetExtendedErrorInfo(errBuff,2048);

    	if (taskDefinition->taskHandle != NULL)
        {
    		DAQmxClearTask(*(taskDefinition->taskHandle));
            NIMEX_freePersistentMemorySegment(taskDefinition->taskHandle);
            taskDefinition->taskHandle = (TaskHandle *)NULL;
        }
    }
    
    NIMEX_releaseLock(taskDefinition);
    
    NIMEX_MACRO_POST_MEX
    
    return;
}
