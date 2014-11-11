/**
 * @file NIMEX_updateDataSourceByCallback.c
 *
 * @brief Calls relevant callbacks (including preprocessors) to update the channel's data buffer.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_updateDataSourceByCallback(taskDefinition, channelName)</tt>
 *  @arg <tt>taskDefinition</tt> - The task whose fields to query.
 *  @arg <tt>channelName</tt> - The name of the channel whose data source to update.
 *
 * @see NIMEX_DataSource_updateBufferByCallback
 *
 * @author Timothy O'Connor
 * @date 8/14/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 */
/*
 * CHANGES
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

    NIMEX_MACRO_PRE_MEX

    if (nlhs != 0)
        mexErrMsgTxt("NIMEX_updateDataSourceByCallback: No return arguments supported.");
    if (nrhs != 2)
        mexErrMsgTxt("NIMEX_updateDataSourceByCallback: Insufficient arguments (must be a task and a channelName).");

    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_updateDataSourceByCallback: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");
    channelName = mxArrayToString(prhs[1]);
    channelDefinition = NIMEX_getChannelByName(taskDefinition, channelName);

    if (channelDefinition == NULL)
        mexErrMsgTxt("NIMEX_updateDataSourceByCallback: Channel not found.");

    if (channelDefinition->dataSource == NULL)
        mexErrMsgTxt("NIMEX_updateDataSourceByCallback: Uninitialized data source, can not update buffer by from callback.\n");

    NIMEX_DataSource_updateBufferByCallback(taskDefinition, channelDefinition->dataSource);

    NIMEX_MACRO_VERBOSE("NIMEX_updateDataSourceByCallback: \n")
    #ifdef NIMEX_VERBOSE
        NIMEX_displayChannelDefinition("    ", channelDefinition);
    #endif

    NIMEX_MACRO_POST_MEX

    return;
}
