/**
 * @file NIMEX_getChannelProperty.c
 *
 * @brief Retrieves accessible properties in a NIMEX_ChannelDefinition structure.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_getChannelProperty(taskDefinition, channelName, propertyName, ...)</tt>
 *  @arg <tt>taskDefinition</tt> - The task whose channels to query.
 *  @arg <tt>channelName</tt> - The physical name of the channel whose fields to query.
 *  @arg <tt>propertyName</tt> - The name of the property to retrieve. Multiple property names may be specified.
 *
 *  @return One argument for each property name that has been specified.
 *
 * @see NIMEX_ChannelDefinition
 *
 * @author Timothy O'Connor
 * @date 1/7/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 */
/*
 * CHANGES
 *  TO012907C: Moved properties from the NIMEX_ChannelDefinition to the  NIMEX_TaskDefinition, to better align with NIDAQmx. -- Tim O'Connor 1/29/07
 *  TO022807C - Added mexFunction pre and post macros. -- Tim O'Connor 2/28/07
 *  TO071107A - C89 compatibility for MSVC's cl compiler. -- Tim O'Connor 7/11/07
 *  TO073070A: Added enable field to channels, allowing them to be defined but not started. -- Tim O'Connor 7/30/07
 *  TO080107B: Replace the dataBuffer with a NIMEX_DataSource object. -- Tim O'Connor 8/1/07
 *  TO102307A: Allow access to the physicalChannel field. -- Tim O'Connor 10/23/07
 */
#include <string.h>
#include "nimex.h"

/**
 * @brief Matlab entry point.
 * @callgraph
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    int dims[] = {1, 1};
    NIMEX_TaskDefinition* taskDefinition = (NIMEX_TaskDefinition *)NULL;
    NIMEX_ChannelDefinition* channelDefinition = (NIMEX_ChannelDefinition *)NULL;
    char* channelName = (char *)NULL;
    char* propertyName = (char *)NULL;
    int i = 0;//TO071107A

    NIMEX_MACRO_PRE_MEX
    
    if (nlhs != 1)
        mexErrMsgTxt("NIMEX_getChannelProperty: Invalid number of outputs requested.");
    
    //There must be a NIMEX_TaskDefinition, a channel name, and then property name(s).
    if (nrhs < 3)
        mexErrMsgTxt("NIMEX_getChannelProperty: Insufficient arguments (must be at least 3).");
    
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_getChannelProperty: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

    channelName = mxArrayToString(prhs[1]);
    channelDefinition = NIMEX_getChannelByName(taskDefinition, channelName);
    if (channelDefinition == NULL)
        mexErrMsgTxt("NIMEX_getChannelProperty: Channel not found.");
    NIMEX_MACRO_VERBOSE("NIMEX_getChannelProperty: Getting property (or properties) for channel '%s'.\n", channelName)
    
    dims[0] = nrhs - 2;//First argument is the task pointer, second is the channel name, all others must be properties.
    plhs[0] = mxCreateCellArray(2, (int*)dims);
    
    for (i = 2; i < nrhs; i++)
    {
        propertyName = mxArrayToString(prhs[i]);
        NIMEX_MACRO_VERBOSE_2("NIMEX_getChannelProperty: Getting '%s':'%s'...\n", channelName, propertyName)
        if (strcmp(propertyName, "channelType") == 0)
            mxSetCell(plhs[0], i - 2, NIMEX_int32_To_mxArray(channelDefinition->channelType));
        else if (strcmp(propertyName, "terminalConfig") == 0)
            mxSetCell(plhs[0], i - 2, NIMEX_uint32_To_mxArray(channelDefinition->terminalConfig));
        else if (strcmp(propertyName, "units") == 0)
            mxSetCell(plhs[0], i - 2, NIMEX_int32_To_mxArray(channelDefinition->units));
        else if (strcmp(propertyName, "minVal") == 0)
            mxSetCell(plhs[0], i - 2, NIMEX_float64_To_mxArray(channelDefinition->minVal));
        else if (strcmp(propertyName, "maxVal") == 0)
            mxSetCell(plhs[0], i - 2, NIMEX_float64_To_mxArray(channelDefinition->maxVal));
        else if (strcmp(propertyName, "mnemonicName") == 0)
            mxSetCell(plhs[0], i - 2, mxCreateString(channelDefinition->mnemonicName));
        else if (strcmp(propertyName, "enable") == 0)
            mxSetCell(plhs[0], i - 2, NIMEX_int32_To_mxArray(channelDefinition->enable));//TO073070A
        else if (strcmp(propertyName, "dataSource") == 0)
		    mxSetCell(plhs[0], i - 2, NIMEX_DataSource_toMxArray(channelDefinition->dataSource));//TO080107B
        else if (strcmp(propertyName, "physicalChannel") == 0)
            mxSetCell(plhs[0], i - 1, mxCreateString(channelDefinition->physicalChannel));//TO102307A
        else
            mexPrintf("NIMEX: Warning - Unrecognized channel property name '%s'\n", propertyName);
    }
    
    NIMEX_MACRO_VERBOSE("NIMEX_getChannelProperty: \n")
    #ifdef NIMEX_VERBOSE_1
	NIMEX_displayChannelDefinition("    ", channelDefinition);//TO010707A
    #endif
    
    NIMEX_MACRO_POST_MEX
    
    return;
}
