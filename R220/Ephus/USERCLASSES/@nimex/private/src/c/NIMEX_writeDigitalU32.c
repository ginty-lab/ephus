/**
 * @file NIMEX_writeDigitalU32.c
 *
 * @brief Write 32-bit unsigned integer values to a valid digital output task.
 *
 * The number of samples to read must be specified. The exact number of samples read may not equal that
 * which was requested.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_writeDigitalU32(nimextask, channelName, data, numSamples)</tt>
 * <tt>NIMEX_writeDigitalU32(nimextask, channelName, data, numSamples, timeout)</tt>
 *  @arg <tt>taskDefinition</tt> - The task to which to write.
 *  @arg <tt>channelName</tt> - The channel for which to write data.
 *  @arg <tt>data</tt> - The samples to be written.
 *  @arg <tt>numSamplesPerChannel</tt> - The number of samples, per channel, to write.
 *  @arg <tt>timeout</tt> - A timeout, after which to give up, in seconds.
 *
 *  @return The number of samples actually written.
 *
 * @author Timothy O'Connor
 * @date 1/24/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 */
/*
 * CHANGES
 *  TO012907C: Moved properties from the NIMEX_ChannelDefinition to the  NIMEX_TaskDefinition, to better align with NIDAQmx. -- Tim O'Connor 1/29/07
 *  TO022307C: Release buffer space when overwritting. -- Tim O'Connor 2/23/07
 *  TO022807C - Added mexFunction pre and post macros. -- Tim O'Connor 2/28/07
 *  TO033007A: Added NIMEX_LAME to test library without running hardware (used when experiencing crashes). -- Tim O'Connor 3/30/07
 *  TO033007D: Added dataBufferSize to the NIMEX_ChannelDefinition structure. -- Tim O'Connor 3/30/07
 *  TO041707A: Assure data buffer size matches sample count specified. -- Tim O'Connor 4/17/07
 *  TO073107A: Preprocess data. -- Tim O'Connor 7/31/07
 *  TO080107B: Replace the dataBuffer with a NIMEX_DataSource object. -- Tim O'Connor 8/1/07
 *  TO080107C: General clean-up, small refactorizations. -- Tim O'Connor 8/1/07
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
    int32                       written;//TO100907A
    float64                     timeout = NIMEX_DEFAULT_TIMEOUT;
	char                        errBuff[2048] = {'\0'};
    NIMEX_TaskDefinition*       taskDefinition = (NIMEX_TaskDefinition *)NULL;
    NIMEX_ChannelDefinition*    channelDefinition = (NIMEX_ChannelDefinition *)NULL;
    uInt32*                     data = (uInt32 *)NULL;
    double*                     arr;
    char*                       channelName;
    mxArray*                    matlabData;

    NIMEX_MACRO_PRE_MEX
    
    if (nlhs > 1)
        mexErrMsgTxt("NIMEX_writeDigitalU32: Only one return argument (number of samples written) supported.");
    
    if (nrhs < 3)
        mexErrMsgTxt("NIMEX_writeDigitalU32: The arguments must be an array of data and a number of samples per channel.");
    
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_writeDigitalU32: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);//TO012407B
    
    NIMEX_MACRO_VERBOSE("NIMEX_writeDigitalU32: \n")
    #ifdef NIMEX_VERBOSE
        NIMEX_displayTaskDefinition("    ", taskDefinition);//TO010707A
    #endif
    
    if ((taskDefinition->taskHandle != NULL) && (taskDefinition->taskHandle != 0))
    {
        NIMEX_MACRO_VERBOSE("NIMEX_writeDigitalU32: taskHandle = %d->%d\n", taskDefinition->taskHandle, *(taskDefinition->taskHandle))
    }
    else
    {
        NIMEX_MACRO_VERBOSE("NIMEX_writeDigitalU32: taskHandle = %d->NULL\n", taskDefinition->taskHandle)
        //mexErrMsgTxt("NIMEX_writeDigitalU32: Invalid NIDAQmx TaskHandle: NULL.");//TO120806D
    }

    //TO120806D
    //if (taskDefinition->taskHandle == NULL)
        //mexErrMsgTxt("NIMEX_writeDigitalU32: Invalid (uninitialized) NIDAQmx TaskHandle.");

    //Marshall arguments.
    channelName = mxArrayToString(prhs[1]);
    channelDefinition = NIMEX_getChannelByName(taskDefinition, channelName);
    
    if (channelDefinition == NULL)
        mexErrMsgTxt("NIMEX_writeDigitalU32: Channel not found");

    taskDefinition->sampsPerChanToAcquire = NIMEX_scalar_mxArray_2_uInt64(prhs[3]);//TO100907A
    if (nrhs > 4)
        taskDefinition->timeout = NIMEX_scalar_mxArray_2_float64(prhs[4]);

    //TO041707A
    if ((taskDefinition->sampsPerChanToAcquire * NIMEX_linkedList_length(taskDefinition->channels)) != (uInt64)mxGetNumberOfElements(prhs[2]))
        mexErrMsgTxt("NIMEX_writeDigitalU32: Number of data points supplied does not match number of samples specified.\n");
    
    //TO073107A
    matlabData = (mxArray *)prhs[2];//Won't get modified, but the array is const. Issues a C4090 warning without the explicit cast.

//    NIMEX_preprocessorCallback(taskDefinition->preprocessors, channelName, &matlabData);
 
    //TO120806D: Write the data immediately if the NIDAQmx task exists.
    if (((taskDefinition->taskHandle) != 0) && ((taskDefinition->taskHandle) != NULL))
    {
    	NIMEX_MACRO_VERBOSE("NIMEX_writeDigitalU32: Writing %lld NIDAQmx samples...\n", taskDefinition->sampsPerChanToAcquire)//sampsPerChanToAcquire
        //TO033007A
        #ifndef NIMEX_LAME
        data = (uInt32 *)(mxGetData(matlabData));//TO073107A
        DAQmxErrChk(DAQmxWriteDigitalU32(*(taskDefinition->taskHandle), taskDefinition->sampsPerChanToAcquire, FALSE, taskDefinition->timeout, DAQmx_Val_GroupByChannel, data, &written, NULL));
        #else
        NIMEX_MACRO_VERBOSE("  NIMEX_LAME - DAQmxWriteDigitalU32 not executed; written = taskDefinition->sampsPerChanToAcquire.\n")
        written = NIMEX_MACRO_uInt64_To_int32(taskDefinition->sampsPerChanToAcquire);
        #endif
        //TO080107B
        NIMEX_DataSource_clearBuffer(channelDefinition->dataSource);//Don't let subsequent start commands re-use this buffer (for now).
        
        NIMEX_MACRO_VERBOSE("NIMEX_writeDigitalU32: Wrote %d NIDAQmx samples.\n", written)
        if (written < taskDefinition->sampsPerChanToAcquire)
            mexPrintf("NIMEX_writeDigitalU32 - Warning: Failed to read requested number of samples - %d (of %d requested).\n", written, taskDefinition->sampsPerChanToAcquire);
    }
    else
    {
        //TO080107B
        //TO120806D: Just cache the data until start, if necessary.
        if (channelDefinition->dataSource == NULL)
            channelDefinition->dataSource = NIMEX_DataSource_create(NIMEX_DATASOURCE_TYPE_UINT32, NULL, NULL, 0, channelName);
        NIMEX_DataSource_clearBuffer(channelDefinition->dataSource);
        NIMEX_DataSource_setBufferFromMxArray(channelDefinition->dataSource, prhs[2]);
        written = taskDefinition->sampsPerChanToAcquire;
    }
    
   if (nlhs > 0)
   {
        //TO080107C - Use the utility function here, instead of doing it the hard way.
        plhs[0] = NIMEX_int32_To_mxArray(written);//TO100907A
   }
    
Error:
	if (DAQmxFailed(error))
    {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
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
		mexPrintf("NIMEX_writeDigitalU32 Error: %s\n", errBuff);
 
    NIMEX_MACRO_POST_MEX
    
    return;
}
