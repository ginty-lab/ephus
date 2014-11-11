/**
 * @file NIMEX_display.c
 *
 * @brief Prints the current state of the object to the standard output.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_display(taskDefinition)</tt>
 *  @arg <tt>taskDefinition</tt> - The task to be displayed.
 *
 * @author Timothy O'Connor
 * @date 1/29/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 */
/*
 * CHANGES
 *  TO022807C - Added mexFunction pre and post macros. -- Tim O'Connor 2/28/07
 *  TO061207A - Port basic structures to glib. -- Tim O'Connor 6/12/07
 *  TO073007C - Watch out for the beguiling empty channel at the start of the list. -- 7/30/07
 *  TO101007B - Print out a message stating there are no channels, if that's the case. -- Tim O'Connor 10/10/07
 *
 */
#include "nimex.h"

/**
 * @brief Matlab entry point.
 * @callgraph
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    NIMEX_TaskDefinition*       taskDefinition = (NIMEX_TaskDefinition *)NULL;
    NIMEX_ChannelList*          channelDefinitionList = (NIMEX_ChannelList *)NULL;

    NIMEX_MACRO_PRE_MEX
    
    if (nlhs != 0)
        mexErrMsgTxt("NIMEX_startTask: No return arguments supported.");
    
    if (nrhs < 1)
        mexErrMsgTxt("NIMEX_startTask: Insufficient arguments (a task definition must be speficied).");
    
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_startTask: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");
    
    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);
    
    mexPrintf("NIMEX_TaskDefinition\n");
        NIMEX_displayTaskDefinition(" ", taskDefinition);

    channelDefinitionList = taskDefinition->channels;
    //TO101007B - Make things a little more informative, declaring if there are no channels at all.
    if (channelDefinitionList == NULL)
        mexPrintf("  NO_CHANNELS\n");
    else if (NIMEX_linkedList_length(channelDefinitionList) == 0)
        mexPrintf("  NO_CHANNELS\n");
    else
    {
        channelDefinitionList = NIMEX_ChannelList_first(channelDefinitionList);
        while (channelDefinitionList != NULL)
        {
            //TO061207A - No more direct access to the fields of the structure.
            if (NIMEX_ChannelList_value(channelDefinitionList) != NULL)
            {
                mexPrintf("  NIMEX_ChannelDefinition\n");//TO073007C - Moved inside the if statement.
                NIMEX_displayChannelDefinition("   ", NIMEX_ChannelList_value(channelDefinitionList));
            }
            channelDefinitionList = (NIMEX_ChannelList *)channelDefinitionList->next;
        }
    }
    
    NIMEX_MACRO_POST_MEX
    
    return;
}
