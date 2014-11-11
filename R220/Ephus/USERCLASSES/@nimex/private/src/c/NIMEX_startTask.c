/**
 * @file NIMEX_startTask.c
 *
 * @brief Creates and starts a NIDAQmx TaskHandle, based on the provided NIMEX_TaskDefinition.
 *
 * The task must be fully configured when calling this function. Once called, parameters can not be changed
 * in the underlying task, without recreating it, which requires stopping the NIMEX-level task.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_startTask(taskDefinition)</tt>
 *  @arg <tt>taskDefinition</tt> - The task to start.
 *
 * @see NIMEX_TaskDefinition
 * @see NIMEX_createTaskHandle
 *
 * @author Timothy O'Connor
 * @date 11/16/06
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006</em>
 */
/*
 * TODO
 *  Provide detailed error and consistency checking. 11/16/06
 *
 * CHANGES
 *  TO120806B: Support configurable digital triggering. -- Tim O'Connor 12/8/06
 *  TO120806C: Support exporting of the clock signal on a specified terminal (clockExportTerminal). -- Tim O'Connor 12/8/06
 *  TO010607B: Created NIMEX_genericStructures. -- Tim O'Connor 1/6/07
 *  TO010707A: Created NIMEX_displayTaskDefinition and NIMEX_displayChannelDefinition. -- Tim O'Connor 1/7/07
 *  TO010707C: Implement prototype callback functionality. -- Tim O'Connor 1/7/07
 *  TO012407B: Added NIMEX_acquireLock & NIMEX_releaseLock for thread safety. -- Tim O'Connor 1/24/07
 *  TO012407C: Created NIMEX_implementCallback to simplify standardized callback execution. -- Tim O'Connor 1/24/07
 *  TO012907C: Moved properties from the NIMEX_ChannelDefinition to the  NIMEX_TaskDefinition, to better align with NIDAQmx. -- Tim O'Connor 1/29/07
 *  TO012907E: Added a flag to indicate the start-status of a task. -- Tim O'Connor 1/29/07
 *  TO012907H: Allow user-control of the N in everyNSamples. -- Tim O'Connor 1/29/07
 *  TO022307B: Add RepeatOutput functionality. -- Tim O'Connor 2/23/07
 *  TO022607D: Improve error handling. Push NIDAQmx errors into Matlab, where appropriate. -- Tim O'Connor 2/26/07
 *  TO022807C - Added mexFunction pre and post macros. -- Tim O'Connor 2/28/07
 *  TO033007A: Added NIMEX_LAME to test library without running hardware (used when experiencing crashes). -- Tim O'Connor 3/30/07
 *  TO033007E: Implemented automatic concatenation of data buffers across output channels. -- Tim O'Connor 3/30/07
 *  TO061207A: Port basic structures to glib. -- Tim O'Connor 6/12/07
 *  TO061907B: Created NIMEX_Callbacks.h. -- Tim O'Connor 6/19/07
 *  TO061907E: Replaced everyNCallbacks and doneCallbacks with eventMap, added dataDispatcher and autoDispatch. -- Tim O'Connor 6/19/07
 *  TO073070A: Added enable field to channels, allowing them to be defined but not started. -- Tim O'Connor 7/30/07
 *  TO101007C: Use everyNSamplesOption for the everyN event registration, it had been hardcoded for inputs, and never tested for outputs. -- Tim O'Connor 10/10/07
 *  TO101007E: Hold on to the channelType, because the channelDefinition may become NULL while iterating. -- Tim O'Connor 10/10/07
 *  TO101207F - Use reference counting to prevent doneEvents being processed while other callbacks are outstanding. -- Tim O'Connor 10/12/07
 *  TO101607E - Always process the done event, even if no callbacks are needed, so the task is automatically stopped and hardware is unreserved. -- Tim O'Connor 10/16/07
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
	int32                       error = 0;
	char                        errBuff[2048] = {'\0'};
    NIMEX_TaskDefinition*       taskDefinition = (NIMEX_TaskDefinition *)NULL;

    NIMEX_MACRO_PRE_MEX

    if (nlhs != 0)
        mexErrMsgTxt("NIMEX_startTask: No return arguments supported.");
    
    if (nrhs < 1)
        mexErrMsgTxt("NIMEX_startTask: Insufficient arguments (a task definition must be speficied).");

    //taskDefinition = (NIMEX_TaskDefinition *)mxGetPr(prhs[0]);
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_startTask: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);//TO012407B
    
    NIMEX_createTaskHandle(taskDefinition);
    
    taskDefinition->done = 0;//TO101807H
    NIMEX_MACRO_VERBOSE("NIMEX_startTask: Starting NIDAQmx TaskHandle (@%p->%p)\n", taskDefinition, *(taskDefinition->taskHandle))
    //TO033007A
    #ifndef NIMEX_LAME
	DAQmxErrChk(DAQmxStartTask(*(taskDefinition->taskHandle)));
    #else
    NIMEX_MACRO_VERBOSE("  NIMEX_LAME - DAQmxStartTask not executed.\n")
    #endif
    taskDefinition->started = 1;//TO012907E

//TEST: Read it immediately.
// DAQmxErrChk(DAQmxReadAnalogF64(*(taskDefinition->taskHandle), 
//   1024, 10, DAQmx_Val_GroupByChannel, data, 1024, &read, NULL));
// mexPrintf("NIMEX_startTask: Read %d samples immediately.\n", read);
Error:
	if (DAQmxFailed(error))
    {
		DAQmxGetExtendedErrorInfo(errBuff,2048);

        taskDefinition->started = 0;//TO012907E
    	if (taskDefinition->taskHandle != NULL)
        {
    		DAQmxStopTask(*(taskDefinition->taskHandle));
    		DAQmxClearTask(*(taskDefinition->taskHandle));
            NIMEX_freePersistentMemorySegment(taskDefinition->taskHandle);
            taskDefinition->taskHandle = (TaskHandle *)NULL;
        }
    }
    
    NIMEX_releaseLock(taskDefinition);//TO012407B

    if (DAQmxFailed(error))
    {
        //TO022607D
		mexPrintf("NIMEX_startTask failed...\n");
        mexErrMsgTxt(errBuff);
    }
    
    NIMEX_MACRO_VERBOSE("NIMEX_startTask: Completed succesfully.\n")
    
    NIMEX_MACRO_POST_MEX
    
    return;
}
