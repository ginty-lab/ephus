/**
 * @file NIMEX_readAnalogF64.c
 *
 * @brief Read 64-bit values from a valid analog input task.
 *
 * The number of samples to read must be specified. The exact number of samples read may not equal that
 * which was requested.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_readAnalogF64(nimextask, numSamplesPerChannel)</tt>
 * <tt>NIMEX_readAnalogF64(nimextask, numSamplesPerChannel, timeout)</tt>
 * <tt>NIMEX_readAnalogF64(nimextask, numSamplesPerChannel, buff, offset)</tt>
 * <tt>NIMEX_readAnalogF64(nimextask, numSamplesPerChannel, timeout, buff, offset)</tt>
 *  @arg <tt>taskDefinition</tt> - The task from which to read.
 *  @arg <tt>numSamplesPerChannel</tt> - The number of samples, per channel, to read.
 *  @arg <tt>timeout</tt> - The timeout, in case all samples are not yet available, in seconds.
 *  @arg <tt>buff</tt> - An array of class 'double', with length greater than or equal to (numSamplesPerChannel * numChannels) + offset
 *  @arg <tt>offset</tt> - The offset in the buffer at which to begin writing samples. 0 indicates the beginning of the buffer.
 *
 *  @return The requested data, grouped by channel, if no buffer is provided. Otherwise the number of samples that were read is returned.
 *
 * @todo Test the forms of this function that supply a buffer to be filled.
 *
 * @author Timothy O'Connor
 * @date 11/16/06
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006</em>
 */
/*
 * TODO
 *  Provide detailed error and consistency checking. 11/16/06
 *  Probably quite a bit of room for optimization/refined implementation. 11/16/06
 *
 * CHANGES
 *  TO010707A: Created NIMEX_displayTaskDefinition and NIMEX_displayChannelDefinition. -- Tim O'Connor 1/7/07
 *  TO012407B: Added NIMEX_acquireLock & NIMEX_releaseLock for thread safety. -- Tim O'Connor 1/24/07
 *  TO012907F: Number of samples to read is per channel, so the buffer size must be a multiple of the number of channels. -- Tim O'Connor 1/29/07
 *  TO013007A: Allow re-use of Matlab arrays for reading data (take the buffer as an argument). -- Tim O'Connor 1/30/07
 *  TO022807C - Added mexFunction pre and post macros. -- Tim O'Connor 2/28/07
 *  TO033007A: Added NIMEX_LAME to test library without running hardware (used when experiencing crashes). -- Tim O'Connor 3/30/07
 *  TO101607H: Globally synchronize so blocking reads aren't undermined by callback processing. -- Tim O'Connor 10/16/07
 *  TO101707A: Created NIMEX_acquireLockExclusively/NIMEX_releaseLockExclusively, to block the Windows message pump from entering critical code while waiting for blocking calls in NIDAQmx to return. -- Tim O'Connor 10/17/07
 *  TO101707C: Mu! Back out TO101707A, for now, as it was based on an invalid test scenario. -- Tim O'Connor 10/17/07
 *  TO101707C: Back out TO101607D, for now, as it potentially screws up read commands in the everyN event. -- Tim O'Connor 10/17/07
 */
#include <string.h>
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
    float64*                    data = (float64 *)NULL;
    double*                     arr;
    int                         numChannels = 1;
    mxArray*                    outBuff = (mxArray *)NULL;
    int                         bufferOffset = 0;
    int32*                      readPr = (int32*)NULL;

    NIMEX_MACRO_PRE_MEX

    //NIMEX_EnterGlobalCriticalSection();//TO101607H - Globally synchronize so blocking reads aren't undermined by callback processing.

    //TO013007A
    if ((nlhs != 1))
        mexErrMsgTxt("NIMEX_readAnalogF64: Only 1 return argument supported.");

    if (nrhs < 2)
        mexErrMsgTxt("NIMEX_readAnalogF64: Insufficient arguments (a task definition and number of samples to read must be speficied).");

    //nrhs == 2: task, numSampsPerChan
    //nrhs == 3: task, numSampsPerChan, timeout
    //nrhs == 4: task, numSampsPerChan, buff, offset
    //nrhs == 5: task, numSampsPerChan, timeout, buff, offset
    if ((nrhs != 2) && (nrhs != 3) && (nrhs != 4) && (nrhs != 5))
        mexErrMsgTxt("NIMEX_readAnalogF64: Invalid arguments.\nUsage:\n\t"
                     "buff = NIMEX_readAnalogF64(task, numSampsPerChan)\n\t"
                     "buff = NIMEX_readAnalogF64(task, numSampsPerChan, timeout)\n\t"
                     "read = NIMEX_readAnalogF64(task, numSampsPerChan, buff, offset)\n\t"
                     "read = NIMEX_readAnalogF64(task, numSampsPerChan, timeout, buff, offset)\n");
    //taskDefinition = (NIMEX_TaskDefinition *)mxGetPr(prhs[0]);
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_readAnalogF64: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");
    //TO013007A
    if ((nrhs == 3) || (nrhs == 5))
        timeout = NIMEX_MACRO_mxArray_To_float64(prhs[2]);

    numSampsPerChan = NIMEX_MACRO_mxArray_To_int32(prhs[1]);
    numChannels = (int32)NIMEX_linkedList_length(taskDefinition->channels);

    //TO013007A
    if (nrhs == 4)
    {
        outBuff = (mxArray *)prhs[2];//Won't get modified, but the array is const. Issues a C4090 warning.
        bufferOffset = NIMEX_MACRO_FLOAT_2_ROUNDED_LONG(mxGetScalar(prhs[3]));
    }
    else if (nrhs == 5)
    {
        outBuff = (mxArray *)prhs[3];//Won't get modified, but the array is const. Issues a C4090 warning.
        bufferOffset = NIMEX_MACRO_FLOAT_2_ROUNDED_LONG(mxGetScalar(prhs[4]));
    }

    //TO013007A
    if (outBuff != NULL)
    {
        if (bufferOffset + (numChannels * numSampsPerChan) < mxGetNumberOfElements(outBuff))
            mexErrMsgTxt("NIMEX_readAnalogF64: Supplied output buffer is too small, must be at least (bufferOffset + (numChannels * numSampsPerChan)) in length.");
        
        if (mxGetClassID(outBuff) != mxDOUBLE_CLASS)
            mexErrMsgTxt("NIMEX_readAnalogF64: Supplied output buffer must be of type 'double'.");
    }

    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);//TO012407B//TO101707A

    NIMEX_MACRO_VERBOSE_3("NIMEX_readAnalogF64: \n")
    #ifdef NIMEX_VERBOSE
        NIMEX_displayTaskDefinition("    ", taskDefinition);//TO010707A
    #endif
    
    if ((taskDefinition->taskHandle != NULL) && (taskDefinition->taskHandle != 0))
    {
        NIMEX_MACRO_VERBOSE_3("NIMEX_readAnalogF64: taskHandle = %d->%d\n", taskDefinition->taskHandle, *(taskDefinition->taskHandle))
    }
    else
    {
        //NIMEX_LeaveGlobalCriticalSection();//TO101607H//TO101707C
        NIMEX_releaseLock(taskDefinition);//TO012407B//TO101707A
        NIMEX_MACRO_VERBOSE_3("NIMEX_readAnalogF64: taskHandle = %d->NULL\n", taskDefinition->taskHandle)
        mexErrMsgTxt("NIMEX_readAnalogF64: Invalid NIDAQmx TaskHandle: NULL.");
    }

    if (taskDefinition->taskHandle == NULL)
    {
        
        //NIMEX_LeaveGlobalCriticalSection();//TO101607H//TO101707C
        NIMEX_releaseLock(taskDefinition);//TO012407B//TO101707A
        mexErrMsgTxt("NIMEX_readAnalogF64: Invalid (uninitialized) NIDAQmx TaskHandle.");
    }
    
	NIMEX_MACRO_VERBOSE_3("NIMEX_readAnalogF64: Creating data buffer via mxCalloc.\n")
	NIMEX_MACRO_VERBOSE_3("NIMEX_readAnalogF64: bufferSize - %d * %d = %d\n", numChannels, numSampsPerChan, numChannels * numSampsPerChan)
    data = (float64 *)mxCalloc(numChannels * numSampsPerChan, sizeof(float64));//Let MATLAB manage this temporary memory.//TO012907F
    NIMEX_MACRO_VERBOSE("NIMEX_readAnalogF64: Reading NIDAQmx %d samples into buffer of size %d with a timeout of %3.4f [s]...\n", numSampsPerChan, numChannels * numSampsPerChan, timeout)//FIX_ME
    //TO033007A
    #ifndef NIMEX_LAME
    DAQmxErrChk(DAQmxReadAnalogF64(*(taskDefinition->taskHandle), numSampsPerChan, timeout, DAQmx_Val_GroupByChannel, data, numChannels * numSampsPerChan, &read, NULL));
    //NIMEX_LeaveGlobalCriticalSection();//TO101607H//TO101707C
    NIMEX_releaseLock(taskDefinition);//TO012407B
    #else
    NIMEX_MACRO_VERBOSE("  NIMEX_LAME - DAQmxReadAnalogF64 not executed; read = numSampsPerChan.\n")
    read = numSampsPerChan;
    #endif
    if (read < numSampsPerChan)
        mexPrintf("NIMEX_readAnalogF64 - Warning: Failed to read requested number of samples - %d (of %d requested).\n", read, numSampsPerChan);
    
    //TO013007A
    if (read > 0)
    {
        NIMEX_MACRO_VERBOSE("NIMEX_readAnalogF64: Packing (%d samples * %d channels) samples back into MATLAB.\n", read, numChannels)
        //TO013007A
        if (outBuff == NULL)
            outBuff = mxCreateDoubleMatrix(read * numChannels, 1,  mxREAL);
        else
        {
            plhs[0] = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL);
            readPr = (int32 *)mxGetData(plhs[0]);
            *readPr = read;
        }

        arr = mxGetPr(outBuff);
        memcpy(arr + bufferOffset, data, read * numChannels * sizeof(float64));//TO012907G - Speed this up, if possible.
        //for (int j = 0; j < numChannels; j++)
        //    for (int i = 0; i < read; i++)
        //        arr[i] = data[i];
    }
    
    if ((plhs[0] == NULL) && (outBuff != NULL))
        plhs[0] = outBuff;

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
		mexPrintf("NIMEX_readAnalogF64 Error: %s\n", errBuff);

    NIMEX_MACRO_POST_MEX

    return;
}
