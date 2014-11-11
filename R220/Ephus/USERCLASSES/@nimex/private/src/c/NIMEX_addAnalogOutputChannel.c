/**
 * @file NIMEX_addAnalogOutputChannel.c
 *
 * @brief Creates a new analog output channel definition attached to the specified task.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_addAnalogOutputChannel(taskDefinition, physicalName)</tt>
 *  @arg <tt>taskDefinition</tt> - The task whose lock to acquire.
 *  @arg <tt>physicalName</tt> - The fully qualified NIDAQmx device name for the new channel.
 *
 * @see NIMEX_createChannelDefinition
 *
 * @author Timothy O'Connor
 * @date 11/29/06
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006</em>
 * @callgraph
 *
 */
/*
 *
 * CHANGES
 *  TO120806A: Fixed printout of samplingRate field. -- Tim O'Connor 12/8/06
 *  TO120806D: Added a per-channel timeout value. -- Tim O'Connor 12/8/06
 *  TO010707A: Created NIMEX_displayTaskDefinition and NIMEX_displayChannelDefinition. -- Tim O'Connor 1/7/07
 *  TO010707B: Fixed off-by-one in string conversion (a duplicate fix exists in another version that's not available here and now). -- Tim O'Connor 1/7/07
 *  TO012407B: Added NIMEX_acquireLock & NIMEX_releaseLock for thread safety. -- Tim O'Connor 1/24/07
 *  TO012907D: Created NIMEX_createChannelDefinition utility function. -- Tim O'Connor 1/29/07
 *  TO022807C - Added mexFunction pre and post macros. -- Tim O'Connor 2/28/07
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
    NIMEX_TaskDefinition* taskDefinition = (NIMEX_TaskDefinition *)NULL;
    NIMEX_ChannelDefinition* channelDefinition = (NIMEX_ChannelDefinition *)NULL;
    char* channelName = (char *)NULL;
    char* mnemonicName = (char *)NULL;

    NIMEX_MACRO_PRE_MEX
    
    if (nlhs != 0)
        mexErrMsgTxt("NIMEX_addAnalogOutputChannel: No return arguments supported.");
    
    //There must be a NIMEX_TaskDefinition, a channel name, and then an even number of key-value pairs.
    if (nrhs < 2)
        mexErrMsgTxt("NIMEX_addAnalogOutputChannel: Insufficient arguments (a task definition and channel specification must be provided).");
    
    //taskDefinition = (NIMEX_TaskDefinition *)mxGetPr(prhs[0]);
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_addAnalogOutputChannel: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);//TO012407B
    
    //TO012907D
    channelDefinition = NIMEX_createChannelDefinition(NIMEX_ANALOG_OUTPUT, mxArrayToString(prhs[1]));

    //Add the channel to the task definition.
    NIMEX_addChannelToList(taskDefinition, channelDefinition);

    NIMEX_MACRO_VERBOSE("NIMEX_addAnalogOutputChannel: \n")
    #ifdef NIMEX_VERBOSE
        NIMEX_displayTaskDefinition("    ", taskDefinition);//TO010707A
        NIMEX_displayChannelDefinition("    taskDefinition->", channelDefinition);//TO010707A
    #endif
    
    NIMEX_releaseLock(taskDefinition);//TO012407B
    
    NIMEX_MACRO_POST_MEX
    
    return;
}
