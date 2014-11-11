/**
 * @file NIMEX_stopTask.c
 *
 * @brief Stop a NIDAQmex task.
 *
 * No errors or warnings are given if the task is not currently running.
 * Data buffers are not automatically cleared.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_stopTask(taskDefinition)</tt>
 *  @arg <tt>taskDefinition</tt> - The task to stop.
 *
 * @author Timothy O'Connor
 * @date 11/16/06
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006</em>
 */
/*
 *
 * TODO
 *  Provide detailed error and consistency checking. 11/16/06
 *
 * CHANGES
 *  TO010707A: Created NIMEX_displayTaskDefinition and NIMEX_displayChannelDefinition. -- Tim O'Connor 1/7/07
 *  TO012407B: Added NIMEX_acquireLock & NIMEX_releaseLock for thread safety. -- Tim O'Connor 1/24/07
 *  TO012907E: Added a flag to indicate the start-status of a task. -- Tim O'Connor 1/29/07
 *  TO022807C - Added mexFunction pre and post macros. -- Tim O'Connor 2/28/07
 *  TO101207F - Use reference counting to prevent doneEvents being processed while other callbacks are outstanding. -- Tim O'Connor 10/12/07
 *  TO101607H - Globally synchronize to be safe for now, maybe remove it later. -- Tim O'Connor 10/16/07
 *
 */
#include "nimex.h"

#define DAQmxErrChk(functionCall) if (DAQmxFailed(error = (functionCall))) goto Error; else

/**
 * @brief Matlab entry point.
 * @callgraph
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	int32                    error = 0;
    char                     errBuff[2048] = {'\0'};
    NIMEX_TaskDefinition*    taskDefinition = (NIMEX_TaskDefinition *)NULL;

    NIMEX_MACRO_PRE_MEX
    
    if (nlhs != 0)
        mexErrMsgTxt("NIMEX_stopTask: No return arguments supported.");
    
    if (nrhs != 1)
        mexErrMsgTxt("NIMEX_stopTask: Invalid number of arguments (a task definition must be speficied).");
    
    //NIMEX_EnterGlobalCriticalSection();//TO101607H
    //taskDefinition = (NIMEX_TaskDefinition *)mxGetPr(prhs[0]);
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_stopTask: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);//TO012407B
    
    NIMEX_MACRO_VERBOSE("NIMEX_stopTask: Stopping NIDAQmx TaskHandle: @%p->@%p\n", taskDefinition, taskDefinition->taskHandle)
    if (taskDefinition->taskHandle != NULL)
    {
        DAQmxStopTask(*(taskDefinition->taskHandle));

        //For now, abort the task, to be totally sure there are no lingering callbacks causing issues.
        //When things are more stable on the Matlab side, this action can be dumped.
        DAQmxTaskControl((*taskDefinition->taskHandle), DAQmx_Val_Task_Abort);

        DAQmxClearTask(*(taskDefinition->taskHandle));
        NIMEX_freePersistentMemorySegment(taskDefinition->taskHandle);
        taskDefinition->taskHandle = (TaskHandle *)NULL;
    }

    taskDefinition->started = 0;//TO012907E

    Error:
    NIMEX_MACRO_VERBOSE("NIMEX_stopTask: \n")
    #ifdef NIMEX_VERBOSE
        NIMEX_displayTaskDefinition("    ", taskDefinition);//TO010707A
    #endif
    
	if (DAQmxFailed(error))
    {
        //TO022607D
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
		mexPrintf("NIMEX_stopTask failed...\n");
        NIMEX_releaseLock(taskDefinition);//TO012407B
        NIMEX_LeaveGlobalCriticalSection();//TO101607H
        mexErrMsgTxt(errBuff);
    }
    
    NIMEX_releaseLock(taskDefinition);//TO012407B
    //NIMEX_LeaveGlobalCriticalSection();//TO101607H

    NIMEX_MACRO_POST_MEX
    
    return;
}
