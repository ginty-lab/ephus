/**
 * @file NIMEX_updateCounterOutput.c
 *
 * @brief Set timing characteristics for a counter/timer channel.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_updateCounterOutput(taskDefinition, channelName, initialDelay, lowTime, highTime)</tt>
 * <tt>NIMEX_updateCounterOutput(taskDefinition, channelName, initialDelay, frequency, dutyCycle)</tt>
 * <tt>NIMEX_updateCounterOutput(taskDefinition, channelName, initialDelay, lowTime, highTime, timeout)</tt>
 * <tt>NIMEX_updateCounterOutput(taskDefinition, channelName, initialDelay, frequency, dutyCycle, timeout)</tt>
 *  @arg <tt>taskDefinition</tt> - The task to update.
 *  @arg <tt>channelName</tt> - The physical name of the channel to be updated.
 *  @arg <tt>initialDelay</tt> - The initial delay, in seconds, before start of the pulse.
 *  @arg <tt>lowTime</tt> - The time, in seconds, spent in the low state.\n
 *                          Only applies to time counter outputs.
 *  @arg <tt>highTime</tt> - TThe time, in seconds, spent in the high state.\n
 *                          Only applies to time counter outputs.
 *  @arg <tt>frequency</tt> - The frequency, in Hz, of the pulses.\n
 *                            Only applies to frequency counter outputs.
 *  @arg <tt>dutyCycle</tt> - The dutyCycle, ratio of pulseWidth to pulsePeriod.\n
 *                            Only applies to frequency counter outputs.
 *  @arg <tt>timeout</tt> - A timeout, after which to give up, in seconds.\n
 *
 * @return A value for each property queried.
 *
 * @see NIMEX_TaskDefinition
 *
 * @author Timothy O'Connor
 * @date 7/31/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 */
/*
 * NOTES
 *  See TO073107E.
 *
 * CHANGES
 *  TO080107B: Replace the dataBuffer with a NIMEX_DataSource object. -- Tim O'Connor 8/1/07
 *  TO101107D: Update to use the mxArray to C datatype conversions from NIMEX_Utilities. -- Tim O'Connor 10/11/07
 *  TO101107E: Instead of all the cast-in-place stuff, just make a typesafe pointer. -- Tim O'Connor 10/11/07
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
    int32                       written;
	char                        errBuff[2048] = {'\0'};
    NIMEX_TaskDefinition*       taskDefinition = (NIMEX_TaskDefinition *)NULL;
    NIMEX_ChannelDefinition*    channelDefinition = (NIMEX_ChannelDefinition *)NULL;
    char*                       channelName;
    mxArray*                    matlabData;
    float64*                    cData;//TO101107E

    NIMEX_MACRO_PRE_MEX
    
    if (nlhs > 0)
        mexErrMsgTxt("NIMEX_updateCounterOutput: No return arguments supported.");
    
    if ((nrhs < 5) || (nrhs > 6))
        mexErrMsgTxt("NIMEX_updateCounterOutput: The arguments must be a a task followed by a channel name, and then intialDelay, lowTime or frequency, highTime or dutyCycle.");
    
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_updateCounterOutput: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);//TO012407B
    
    NIMEX_MACRO_VERBOSE("NIMEX_updateCounterOutput: \n")
    #ifdef NIMEX_VERBOSE
        NIMEX_displayTaskDefinition("    ", taskDefinition);//TO010707A
    #endif

    //TO120806D
    //if (taskDefinition->taskHandle == NULL)
        //mexErrMsgTxt("NIMEX_configureTiming: Invalid (uninitialized) NIDAQmx TaskHandle.");

    //Marshall arguments.
    channelName = mxArrayToString(prhs[1]);
    NIMEX_MACRO_VERBOSE("NIMEX_updateCounterOutput: Looking up channel \"%s\"...\n", channelName)
    channelDefinition = NIMEX_getChannelByName(taskDefinition, channelName);
    if (channelDefinition == NULL)
        mexErrMsgTxt("NIMEX_updateCounterOutput: Channel not found");

    /*NIMEX_MACRO_VERBOSE("NIMEX_updateCounterOutput: \n")
    #ifdef NIMEX_VERBOSE
        NIMEX_displayChannelDefinition("      ", channelDefinition);//TO010707A
    #endif*/

    //TO101207B - No need to allocate the buffer here, only to have it copied, it will be allocated in the constructor.
    if (channelDefinition->dataSource == NULL)
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_updateCounterOutput: Creating new dataSource for channel '%s'...\n", channelDefinition->physicalChannel)
        channelDefinition->dataSource = NIMEX_DataSource_create(NIMEX_DATASOURCE_TYPE_FLOAT64, NULL, NULL, 3, channelDefinition->physicalChannel);
        //channelDefinition->dataSource = NIMEX_DataSource_create(NIMEX_DATASOURCE_TYPE_FLOAT64, NIMEX_Persistent_Calloc(3, sizeof(float64)), NULL, 3, channelDefinition->physicalChannel);
    }

    NIMEX_MACRO_VERBOSE_2("NIMEX_updateCounterOutput: Writing parameters to buffer (@%p) of counter/timer channel '%s'...\n", channelDefinition->dataSource->dataBuffer, channelDefinition->physicalChannel)
    cData = (float64 *)channelDefinition->dataSource->dataBuffer;//TO101107E
    //((float64 *)channelDefinition->dataSource->dataBuffer)[0] = NIMEX_scalar_mxArray_2_float64(prhs[2]);//TO101107D
    //((float64 *)channelDefinition->dataSource->dataBuffer)[1] = NIMEX_scalar_mxArray_2_float64(prhs[3]);//TO101107D
    //((float64 *)channelDefinition->dataSource->dataBuffer)[2] = NIMEX_scalar_mxArray_2_float64(prhs[4]);//TO101107D
    //*((float64 *)channelDefinition->dataSource->dataBuffer) = NIMEX_scalar_mxArray_2_float64(prhs[2]);//TO101107D
    //*((float64 *)(channelDefinition->dataSource->dataBuffer + sizeof(float64))) = NIMEX_scalar_mxArray_2_float64(prhs[3]);//TO101107D
    //*((float64 *)(channelDefinition->dataSource->dataBuffer + (2 * sizeof(float64)))) = NIMEX_scalar_mxArray_2_float64(prhs[4]);//TO101107D
    cData[0] = NIMEX_scalar_mxArray_2_float64(prhs[2]);
    cData[1] = NIMEX_scalar_mxArray_2_float64(prhs[3]);
    cData[2] = NIMEX_scalar_mxArray_2_float64(prhs[4]);
    NIMEX_MACRO_VERBOSE_3("NIMEX_updateCounterOutput: channelDefinition->dataSource->dataBuffer = @%p = [%3.4lf, %3.4lf, %3.4lf]\n", channelDefinition->dataSource->dataBuffer, ((float64 *)channelDefinition->dataSource->dataBuffer)[0], ((float64 *)channelDefinition->dataSource->dataBuffer)[1], ((float64 *)channelDefinition->dataSource->dataBuffer)[2]);

    if (nrhs > 5)
        taskDefinition->timeout = NIMEX_scalar_mxArray_2_float64(prhs[5]);//TO101107D
   
    if (taskDefinition->taskHandle != NULL)
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_updateCounterOutput: Writing counter timing values to board...\n")
        switch (channelDefinition->channelType)
        {
            case NIMEX_CO_FREQUENCY:
                DAQmxErrChk(DAQmxWriteCtrFreq(*(taskDefinition->taskHandle), 
                    1,//taskDefinition->sampsPerChanToAcquire, 
                    FALSE, 
                    taskDefinition->timeout,
                    DAQmx_Val_GroupByChannel,
                    &(((float64 *)channelDefinition->dataSource->dataBuffer)[1]), 
                    &(((float64 *)channelDefinition->dataSource->dataBuffer)[2]), 
//                    ((float64 *)channelDefinition->dataBuffer),
//                    ((float64 *)((int)channelDefinition->dataSource->dataBuffer + sizeof(float64))),
//                    ((float64 *)((int)channelDefinition->dataSource->dataBuffer + (2 * sizeof(float64)))),
                    &written,
                    NULL));
                break;
    
            case NIMEX_CO_TIME:
                DAQmxErrChk(DAQmxWriteCtrTime(*(taskDefinition->taskHandle), 
                    1,//taskDefinition->sampsPerChanToAcquire, 
                    FALSE, 
                    taskDefinition->timeout,
                    DAQmx_Val_GroupByChannel,
                    &(((float64 *)channelDefinition->dataSource->dataBuffer)[1]), 
                    &(((float64 *)channelDefinition->dataSource->dataBuffer)[2]), 
//                  ((float64 *)channelDefinition->dataBuffer),
//                    ((float64 *)((int)channelDefinition->dataSource->dataBuffer + sizeof(float64))),
//                    ((float64 *)((int)channelDefinition->dataSource->dataBuffer + (2 * sizeof(float64)))),
                    &written,
                    NULL));
                break;
        
            default:
                mexErrMsgTxt("NIMEX_updateCounterOutput: Specified channel is not a valid counter output type.");
        }
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
