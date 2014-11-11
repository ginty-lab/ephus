/**
 * @file NIMEX_clearBuffer.c
 *
 * @brief Clears the channel's data buffer.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_clearBuffer(taskDefinition, channelName)</tt>
 *  @arg <tt>taskDefinition</tt> - The task whose lock to acquire.
 *  @arg <tt>channelName</tt> - The physical name of the channel whose buffer to clear.
 *
 * @author Timothy O'Connor
 * @date 5/3/08
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2008</em>
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

    NIMEX_MACRO_PRE_MEX

    if (nlhs != 0)
        mexErrMsgTxt("NIMEX_clearBuffer: No return arguments supported.");
    if (nrhs != 2)
        mexErrMsgTxt("NIMEX_clearBuffer: Insufficient arguments (must be a task and a channelName).");

    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_clearBuffer: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");
    channelName = mxArrayToString(prhs[1]);
    channelDefinition = NIMEX_getChannelByName(taskDefinition, channelName);

    if (channelDefinition == NULL)
        mexErrMsgTxt("NIMEX_clearBuffer: Channel not found.");

    if (channelDefinition->dataSource != NULL)
        NIMEX_DataSource_clearBuffer(channelDefinition->dataSource);
    else
    {
        NIMEX_MACRO_VERBOSE("NIMEX_clearBuffer: Call to clear buffers for '%s', but the buffer is already empty or non-existent.\n", channelName);
    }

    NIMEX_MACRO_VERBOSE("NIMEX_clearBuffer: \n")
    #ifdef NIMEX_VERBOSE
        NIMEX_displayChannelDefinition("    ", channelDefinition);
    #endif

    NIMEX_MACRO_POST_MEX

    return;
}
