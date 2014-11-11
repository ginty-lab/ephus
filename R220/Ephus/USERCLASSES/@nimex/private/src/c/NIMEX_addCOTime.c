/**
 * @file NIMEX_addCOTime.c
 *
 * @brief Creates a new counter/timer frequency output channel definition attached to the specified task.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_addCOTime(taskDefinition, physicalName)</tt>
 *  @arg <tt>taskDefinition</tt> - The task whose lock to acquire.
 *  @arg <tt>physicalName</tt> - The fully qualified NIDAQmx device name for the new channel.
 *
 * @see NIMEX_createChannelDefinition
 *
 * @author Timothy O'Connor
 * @date  7/31/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 * @callgraph
 *
 */
/*
 * NOTES
 *  See TO073107E.
 *
 */
#include "nimex.h"

/**
 * @brief Matlab entry point.
 * @callgraph
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    NIMEX_TaskDefinition* taskDefinition = (NIMEX_TaskDefinition *)NULL;
    NIMEX_ChannelDefinition* channelDefinition = (NIMEX_ChannelDefinition *)NULL;
    
    NIMEX_MACRO_PRE_MEX

    if (nlhs != 0)
        mexErrMsgTxt("NIMEX_addCOTime: No return arguments supported.");
    
    //There must be a NIMEX_TaskDefinition, a channel name, and then an even number of key-value pairs.
    if (nrhs < 2)
        mexErrMsgTxt("NIMEX_addCOTime: Insufficient arguments (a task definition and channel specification must be provided).");

    //taskDefinition = (NIMEX_TaskDefinition *)mxGetPr(prhs[0]);
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_addCOTime: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);//TO012407B

    //TO012907D
    channelDefinition = NIMEX_createChannelDefinition(NIMEX_CO_TIME, mxArrayToString(prhs[1]));

    //Add the channel to the task definition.
    NIMEX_addChannelToList(taskDefinition, channelDefinition);

    NIMEX_MACRO_VERBOSE("NIMEX_addCOTime: \n")
    #ifdef NIMEX_VERBOSE
        NIMEX_displayTaskDefinition("    ", taskDefinition);//TO010707A
        NIMEX_displayChannelDefinition("    taskDefinition->", channelDefinition);//TO010707A
    #endif

    NIMEX_MACRO_POST_MEX
    
    return;
}
