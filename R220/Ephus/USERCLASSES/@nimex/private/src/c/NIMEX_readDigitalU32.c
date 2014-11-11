/**
 * @file NIMEX_readDigitalU32.c
 *
 * @brief Read 32-bit unsigned integer values from a valid digital input task.
 *
 * The number of samples to read must be specified. The exact number of samples read may not equal that
 * which was requested.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_readDigitalU32(nimextask, numSamplesPerChannel)</tt>
 * <tt>NIMEX_readDigitalU32(nimextask, numSamplesPerChannel, timeout)</tt>
 *  @arg <tt>taskDefinition</tt> - The task from which to read.
 *  @arg <tt>numSamplesPerChannel</tt> - The number of samples, per channel, to read.
 *  @arg <tt>timeout</tt> - The timeout, in case all samples are not yet available, in seconds.
 *
 *  @return The requested data, grouped by channel, as an mxUINT32 array.
 *
 * @author Timothy O'Connor
 * @date 1/24/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 */
/*
 * CHANGES
 *  TO022807C - Added mexFunction pre and post macros. -- Tim O'Connor 2/28/07
 *  TO033007A: Added NIMEX_LAME to test library without running hardware (used when experiencing crashes). -- Tim O'Connor 3/30/07
 *  TO071107A - C89 compatibility for MSVC's cl compiler. -- Tim O'Connor 7/11/07
 *  TO101707A: Created NIMEX_acquireLockExclusively/NIMEX_releaseLockExclusively, to block the Windows message pump from entering critical code while waiting for blocking calls in NIDAQmx to return. -- Tim O'Connor 10/17/07
 *  TO101707C: Mu! Back out TO101707A, for now, as it was based on an invalid test scenario. -- Tim O'Connor 10/17/07
 *  TO101707C: Back out TO101607D, for now, as it potentially screws up read commands in the everyN event. -- Tim O'Connor 10/17/07
 */
#include "nimex.h"

#define DAQmxErrChk(functionCall) if (DAQmxFailed(error = (functionCall))) goto Error; else

/**
 * @brief Matlab entry point.
 * @callgraph
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    int32                       numSampsPerChan;
	int32                       error = 0;
    int32                       read;
    float64                     timeout = NIMEX_DEFAULT_TIMEOUT;
	char                        errBuff[2048] = {'\0'};
    NIMEX_TaskDefinition*       taskDefinition = (NIMEX_TaskDefinition *)NULL;
    uInt32*                     data = (uInt32 *)NULL;
    double*                     arr;
    int                         dims[2] = {1, 1};
    int                         i = 0;//TO071107A

    NIMEX_MACRO_PRE_MEX
    
    if (nlhs != 1)
        mexErrMsgTxt("NIMEX_readDigitalU32: Only 1 return argument supported.");
    
    if (nrhs < 2)
        mexErrMsgTxt("NIMEX_readDigitalU32: Insufficient arguments (a task definition and number of samples to read must be speficied).");
    
    //taskDefinition = (NIMEX_TaskDefinition *)mxGetPr(prhs[0]);
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_readDigitalU32: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);//TO012407B//TO101707A
    
    NIMEX_MACRO_VERBOSE_3("NIMEX_readDigitalU32: \n")
    #ifdef NIMEX_VERBOSE
        NIMEX_displayTaskDefinition("    ", taskDefinition);//TO010707A
    #endif
    
    if ((taskDefinition->taskHandle != NULL) && (taskDefinition->taskHandle != 0))
    {
        NIMEX_MACRO_VERBOSE_3("NIMEX_readDigitalU32: taskHandle = %d->%d\n", taskDefinition->taskHandle, *(taskDefinition->taskHandle))
    }
    else
    {
        //NIMEX_LeaveGlobalCriticalSection();//TO101607H//TO101707C
        NIMEX_releaseLock(taskDefinition);//TO012407B//TO101707A
        NIMEX_MACRO_VERBOSE_3("NIMEX_readDigitalU32: taskHandle = %d->NULL\n", taskDefinition->taskHandle)
        mexErrMsgTxt("NIMEX_readDigitalU32: Invalid NIDAQmx TaskHandle: NULL.");
    }

    if (taskDefinition->taskHandle == NULL)
    {
        //NIMEX_LeaveGlobalCriticalSection();//TO101607H//TO101707C
        NIMEX_releaseLock(taskDefinition);//TO012407B//TO101707A
        mexErrMsgTxt("NIMEX_readDigitalU32: Invalid (uninitialized) NIDAQmx TaskHandle.");
    }

    numSampsPerChan = (int32)*(mxGetPr(prhs[1]));
    
    if (nrhs >= 3)
        timeout = (float64)*(mxGetPr(prhs[2]));
	NIMEX_MACRO_VERBOSE_3("NIMEX_readDigitalU32: Creating data buffer via mxCalloc.\n")
	NIMEX_MACRO_VERBOSE_3("NIMEX_readDigitalU32: bufferSize - %d * %d = %d\n", NIMEX_linkedList_length(taskDefinition->channels), numSampsPerChan, NIMEX_linkedList_length(taskDefinition->channels) * numSampsPerChan)
    data = (uInt32 *)mxCalloc(numSampsPerChan, sizeof(uInt32));//Let MATLAB manage this temporary memory.
	NIMEX_MACRO_VERBOSE("NIMEX_readDigitalU32: Reading NIDAQmx samples...\n")
    //TO033007A
    #ifndef NIMEX_LAME
    DAQmxErrChk(DAQmxReadDigitalU32(*(taskDefinition->taskHandle), numSampsPerChan, timeout, DAQmx_Val_GroupByChannel, data, numSampsPerChan, &read, NULL));
    //NIMEX_LeaveGlobalCriticalSection();//TO101607H//TO101707C
    NIMEX_releaseLock(taskDefinition);//TO012407B
    #else
    NIMEX_MACRO_VERBOSE("  NIMEX_LAME - DAQmxReadDigitalU32 not executed; read = numSampsPerChan.\n")
    read = numSampsPerChan;
    #endif
    if (read < numSampsPerChan)
        mexPrintf("NIMEX_readDigitalU32 - Warning: Failed to read requested number of samples - %d (of %d requested).\n", read, numSampsPerChan);
    
    if (read > 0)
    {
        NIMEX_MACRO_VERBOSE_3("NIMEX_readDigitalU32: Packing samples back into MATLAB.\n")
        dims[0] = read;
        plhs[0] = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);
        arr = mxGetPr(plhs[0]);
        for (i = 0; i < read; i++)
            arr[i] = data[i];
    }

Error:
	if (DAQmxFailed(error))
    {
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
        if (taskDefinition->taskHandle != NULL)
        {
            //NIMEX_LeaveGlobalCriticalSection();//TO101607H//TO101707C
            DAQmxStopTask(*(taskDefinition->taskHandle));
            DAQmxClearTask(*(taskDefinition->taskHandle));
            NIMEX_freePersistentMemorySegment(taskDefinition->taskHandle);
            taskDefinition->taskHandle = (TaskHandle *)NULL;
            NIMEX_releaseLock(taskDefinition);//TO012407B//TO101707A
        }
	}

	if (DAQmxFailed(error))
		mexPrintf("NIMEX_readDigitalU32 Error: %s\n", errBuff);
 
    NIMEX_MACRO_POST_MEX
    
    return;
}
