/**
 * @file NIMEX_isOutput.c
 *
 * @brief Determines if a task is configured for output.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_isOutput(taskDefinition)</tt>
 *  @arg <tt>taskDefinition</tt> - The task to be tested for input/output.
 *
 * @return Returns 0 if the task is an input or has no channels, 1 otherwise.
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
    int i;

    NIMEX_TaskDefinition* taskDefinition = (NIMEX_TaskDefinition *)NULL;
    NIMEX_ChannelList* channelList = (NIMEX_ChannelList *)NULL;
    NIMEX_ChannelDefinition* channelDefinition = (NIMEX_ChannelDefinition *)NULL;

    NIMEX_MACRO_PRE_MEX

    if (nlhs != 1)
        mexErrMsgTxt("NIMEX_isOutput: Must accept one return argument.");
    if (nrhs != 1)
        mexErrMsgTxt("NIMEX_isOutput: A nimex task must be supplied as an argument, no others are allowed.");

    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_isOutput: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");
    
    channelList = taskDefinition->channels;

    for (i = 0; i < NIMEX_linkedList_length(channelList); i++)
    {
        if (channelList != NULL)
        {
           channelDefinition = NIMEX_ChannelList_value(channelList);
           if (channelDefinition != NULL)
           {
               if ((channelDefinition->channelType == NIMEX_ANALOG_OUTPUT) ||
                   (channelDefinition->channelType == NIMEX_DIGITAL_OUTPUT))
               {
                  plhs[0] = mxCreateDoubleScalar(TRUE);
                  return;
               }
               else
               {
                  plhs[0] = mxCreateDoubleScalar(FALSE);
                  return;
               }
           }
        }
        
        channelList = NIMEX_ChannelList_next(channelList);
    }
    
    plhs[0] = mxCreateDoubleScalar(FALSE);

    NIMEX_MACRO_POST_MEX

    return;
}
