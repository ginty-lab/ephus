/**
 * @file NIMEX_setChannelProperty.c
 *
 * @brief Modifies directly accesible properties in a NIMEX_ChannelDefinition structure.
 *
 * @note Not all properties are settable.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_setChannelProperty(taskDefinition, channelName, propertyName, propertyValue, ...)</tt>
 *  @arg <tt>taskDefinition</tt> - The task whose channel to modify.
 *  @arg <tt>channelName</tt> - The physical name of the channel whose fields to set.
 *  @arg <tt>propertyName</tt> - The name of the property to be set. Multiple properties may be specified.
 *  @arg <tt>propertyValue</tt> - The value of the property to be set. Multiple properties may be specified.
 *
 * <b>Settable properties:</b>\n
 *  @li terminalConfig
 *  @li units
 *  @li minVal
 *  @li maxVal
 *  @li mnemonicName
 *  @li enable
 *  @li dataSource - <em>The type of the value used to set this field and the type of channel implies the resulting NIMEX_DataSource type.</em>
 *
 * @see NIMEX_ChannelDefinition
 * @see NIMEX_DataSource
 * @see NIMEX_DataSource_create
 *
 * @author Timothy O'Connor
 * @date 11/16/06
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006</em>
 */
/*
 * TODO
 *  Advanced argument/type checking, more detailed error messages, 
 *  and "batched" setting of properties. -- 11/16/06
 *
 * CHANGES
 *  TO010607B - Implemented NIMEX_MACRO_MXARRAY_2_PERSISTENT_CHAR. -- Tim O'Connor 1/6/07
 *  TO010707A: Created NIMEX_displayTaskDefinition and NIMEX_displayChannelDefinition. -- Tim O'Connor 1/7/07
 *  TO012907C: Moved properties from the NIMEX_ChannelDefinition to the  NIMEX_TaskDefinition, to better align with NIDAQmx. -- Tim O'Connor 1/29/07
 *  TO013007B: Clear property strings from memory on overwrite. -- Tim O'Connor 1/30/07
 *  TO022807C - Added mexFunction pre and post macros. -- Tim O'Connor 2/28/07
 *  TO040207G: Created NIMEX_MatlabVariable_2_DAQmx_Constant. -- Tim O'Connor 4/2/07
 *  TO071107A - C89 compatibility for MSVC's cl compiler. -- Tim O'Connor 7/11/07
 *  TO073070A: Added enable field to channels, allowing them to be defined but not started. -- Tim O'Connor 7/30/07
 *  TO080107B: Replace the dataBuffer with a NIMEX_DataSource object. -- Tim O'Connor 8/1/07
 *  TO100607C: Debugging dataSource functionality. -- Tim O'Connor 10/06/07
 *  TO101207C: Consider the dataSource off-limits for counter/timer channels. -- Tim O'Connor 10/12/07
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
    char* propertyName = (char *)NULL;
    int i = 0;//TO071107A
    int dataSourceTypeBitMask = 0;

    NIMEX_MACRO_PRE_MEX

    if (nlhs != 0)
        mexErrMsgTxt("NIMEX_setChannelProperty: No return arguments supported.");

    //There must be a NIMEX_TaskDefinition, a channel name, and then an even number of key-value pairs.
    if (nrhs < 4)
        mexErrMsgTxt("NIMEX_setChannelProperty: Insufficient arguments (must be at least 4).");

    //taskDefinition = (NIMEX_TaskDefinition *)mxGetPr(prhs[0]);
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_setChannelProperty: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

    channelName = mxArrayToString(prhs[1]);
    channelDefinition = NIMEX_getChannelByName(taskDefinition, channelName);

    if (channelDefinition == NULL)
        mexErrMsgTxt("NIMEX_setChannelProperty: Channel not found.");

    NIMEX_MACRO_VERBOSE("NIMEX_setChannelProperty: Setting property (or properties) for channel '%s'.\n", channelName)

    for (i = 2; i < nrhs; i += 2)
    {
        propertyName = mxArrayToString(prhs[i]);
        NIMEX_MACRO_VERBOSE("NIMEX_setChannelProperty: Setting '%s':'%s'...\n", channelName, propertyName)

        if (strcmp(propertyName, "terminalConfig") == 0)
            channelDefinition->terminalConfig = (uInt32)(NIMEX_MatlabVariable_2_DAQmx_Constant(prhs[i + 1]));//TO040207G
        else if (strcmp(propertyName, "units") == 0)
            channelDefinition->units = (int32)(NIMEX_MatlabVariable_2_DAQmx_Constant(prhs[i + 1]));//TO040207G
        else if (strcmp(propertyName, "minVal") == 0)
            channelDefinition->minVal = (float64)*(mxGetPr(prhs[i + 1]));
        else if (strcmp(propertyName, "maxVal") == 0)
            channelDefinition->maxVal = (float64)*(mxGetPr(prhs[i + 1]));
        else if (strcmp(propertyName, "mnemonicName") == 0)
        {
            //TO013007B
            if (channelDefinition->mnemonicName != NULL)
                NIMEX_freePersistentMemorySegment(channelDefinition->mnemonicName);
            channelDefinition->mnemonicName = NIMEX_MACRO_MXARRAY_2_PERSISTENT_CHAR(prhs[i + 1]);//TO010607B NIMEX_memcpy(mxArrayToString(prhs[i + 1]), (mxGetNumberOfElements(prhs[i + 1]) + 1) * sizeof(char));
        }
        else if (strcmp(propertyName, "enable") == 0)
            channelDefinition->enable = (int32)*(mxGetPr(prhs[i + 1]));//TO073070A
        else if (strcmp(propertyName, "dataSource") == 0)
        {
            //TO101207C
            if ( (channelDefinition->channelType == NIMEX_CO_TIME) || (channelDefinition->channelType == NIMEX_CO_FREQUENCY) )
                mexErrMsgTxt("NIMEX_setChannelProperty: dataSource field not supported for counter/timer channels. Use NIMEX_updateCounterOutput instead.\n");
            
            if (mxGetNumberOfElements(prhs[i + 1]) == 0)
            {
                //TO100607C - Debugging dataSource functionality. Clear data sources, when necessary.
                if (channelDefinition->dataSource != NULL)
                {
                    NIMEX_DataSource_destroy(channelDefinition->dataSource);
                    channelDefinition->dataSource = NULL;
                }
            }
            else
            {
                //TO100607C - Debugging dataSource functionality. Clear data sources, when necessary, fixed conditions.
                if (channelDefinition->dataSource != NULL)
                {
                    NIMEX_DataSource_destroy(channelDefinition->dataSource);
                    channelDefinition->dataSource = NULL;
                }

                if (mxGetClassID(prhs[i + 1]) == mxCELL_CLASS)
                    dataSourceTypeBitMask = NIMEX_DATASOURCE_TYPE_CALLBACK_BITMASK;

                if (channelDefinition->channelType == NIMEX_ANALOG_OUTPUT)
                {
                    //float64 data
                    if (dataSourceTypeBitMask)
                        channelDefinition->dataSource = NIMEX_DataSource_create(NIMEX_DATASOURCE_TYPE_FLOAT64 | dataSourceTypeBitMask,
                        NULL, prhs[i + 1], 0, channelName);
                    else
                        channelDefinition->dataSource = NIMEX_DataSource_create(NIMEX_DATASOURCE_TYPE_FLOAT64, mxGetPr(prhs[i + 1]), NULL,
                        (uInt64)mxGetNumberOfElements(prhs[i + 1]), channelName);
                }
                else if (channelDefinition->channelType == NIMEX_DIGITAL_OUTPUT)
                {
                    //uInt32 data
                    if (dataSourceTypeBitMask)
                        channelDefinition->dataSource = NIMEX_DataSource_create(NIMEX_DATASOURCE_TYPE_UINT32 | dataSourceTypeBitMask,
                        NULL, prhs[i + 1], 0, channelName);
                    else
                        channelDefinition->dataSource = NIMEX_DataSource_create(NIMEX_DATASOURCE_TYPE_UINT32, mxGetPr(prhs[i + 1]), NULL,
                        mxGetNumberOfElements(prhs[i + 1]), channelName);
                }
                else
                    mexErrMsgTxt("NIMEX_setChannelProperty: dataSource field not supported for non-output channels.");
            }
            //TO100607C - Is this necessary anymore?
            //else 
                //NIMEX_DataSource_fromMxArray(&(channelDefinition->dataSource), prhs[i + 1], channelName);//TO080107B
        }
        else
            mexPrintf("NIMEX: Warning - Unrecognized channel property name '%s'\n", propertyName);
    }

    NIMEX_MACRO_VERBOSE("NIMEX_setChannelProperty: \n")
    #ifdef NIMEX_VERBOSE_1
        NIMEX_displayChannelDefinition("    ", channelDefinition);//TO010707A
    #endif

    NIMEX_MACRO_POST_MEX

    return;
}
