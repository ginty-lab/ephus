/**
 * @file NIMEX_getTaskProperty.c
 *
 * @brief Retrieves accesible properties in a NIMEX_TaskDefinition structure.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_getTaskProperty(taskDefinition, propertyName, ...)</tt>
 *  @arg <tt>taskDefinition</tt> - The task whose fields to query.
 *  @arg <tt>propertyName</tt> - The name of the property to retrieve. Multiple properties may be specified.
 *
 * @return A value for each property queried.
 *
 * @see NIMEX_TaskDefinition
 *
 * @author Timothy O'Connor
 * @date 1/7/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 */
/*
 * CHANGES
 *  TO012407A: Added userData field. -- Tim O'Connor 1/24/07
 *  TO012709A: When requesting the 'channels' field, return a cell array of names. -- Tim O'Connor 1/29/07
 *  TO012907C: Moved properties from the NIMEX_ChannelDefinition to the  NIMEX_TaskDefinition, to better align with NIDAQmx. -- Tim O'Connor 1/29/07
 *  TO012907H: Allow user-control of the N in everyNSamples. -- Tim O'Connor 1/29/07
 *  TO022307B: Add RepeatOutput functionality, by hijacking everyNSamples (which then becomes unavailable). -- Tim O'Connor 2/23/07
 *  TO022607B: Bug fixes - Check for NULL pointer, typo in cell array indexing. -- Tim O'Connor 2/26/07
 *  TO022807C - Added mexFunction pre and post macros. -- Tim O'Connor 2/28/07
 *  TO061207A: Port basic structures to glib. -- Tim O'Connor 6/12/07
 *  TO071107A - C89 compatibility for MSVC's cl compiler. -- Tim O'Connor 7/11/07
 *  TO073007C - Watch out for the beguiling empty channel at the start of the list. -- 7/30/07
 *  TO073107E: Added counter/timer functionality. -- Tim O'Connor 7/31/07
 *  TO101707I: Made taskDefinition->started accessible. -- Tim O'Connor 10/17/07
 *  TO101807H: Added a done field to the taskDefinition. -- Tim O'Connor 10/18/07
 *  TO101907A: Added autoRestart field, for "external trigger" mode. -- Tim O'Connor 10/19/07
 *  TO102407A: Added forceFullBuffering, which requires dataSources to supply all data, including samples that are due to repeatOutput. -- Tim O'Connor 10/24/07
 *  TO102508E: Added the sampleClockTimebaseSource property. -- Tim O'Connor 10/25/08
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
    int dims[] = {1, 1};
    NIMEX_TaskDefinition* taskDefinition = (NIMEX_TaskDefinition *)NULL;
    char* propertyName = (char *)NULL;
    int channelListLength = 0;//TO012709A
    mxArray*    channelNameList = (mxArray *)NULL;//TO012709A
    NIMEX_ChannelDefinition*  channelDefinition = (NIMEX_ChannelDefinition *)NULL;//TO012709A
    NIMEX_ChannelList* channelList = (NIMEX_ChannelList *)NULL;//TO012709A
    mxArray* cell = (mxArray *)NULL;
    int i = 0;//TO071107A
    int j = 0;//TO071107A

    NIMEX_MACRO_PRE_MEX
    
    if (nlhs != 1)
        mexErrMsgTxt("NIMEX_getTaskProperty: Invalid number of outputs requested.");
    
    //There must be a NIMEX_TaskDefinition and then an even number of key-value pairs.
    if (nrhs < 2)
        mexErrMsgTxt("NIMEX_getTaskProperty: Insufficient arguments (must be at least 2).");
    
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_getTaskProperty: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

    NIMEX_MACRO_VERBOSE("NIMEX_getTaskProperty: Getting property (or properties) for @%p.\n", taskDefinition)

    dims[0] = nrhs - 1;//First argument is the task pointer, all others must be properties.
    plhs[0] = mxCreateCellArray(2, (int*)dims);

    for (i = 1; i < nrhs; i++)
    {
        propertyName = mxArrayToString(prhs[i]);

        NIMEX_MACRO_VERBOSE_3("NIMEX_getTaskProperty: Getting '%s'...\n", propertyName)
        if (strcmp(propertyName, "clockActiveEdge") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_int32_To_mxArray(taskDefinition->clockActiveEdge));
        else if (strcmp(propertyName, "clockSource") == 0)
            mxSetCell(plhs[0], i - 1, mxCreateString(taskDefinition->clockSource));
        else if (strcmp(propertyName, "clockExportTerminal") == 0)
            mxSetCell(plhs[0], i - 1, mxCreateString(taskDefinition->clockExportTerminal));
        else if (strcmp(propertyName, "channels") == 0)
        {
            //TO012709A//TO022607B
            if (taskDefinition->channels != NULL)
                channelListLength = NIMEX_linkedList_length(taskDefinition->channels);
            else
                channelListLength = 0;

            dims[0] = channelListLength;
            channelNameList = mxCreateCellArray(2, (int*)dims);
            channelList = NIMEX_ChannelList_first(taskDefinition->channels);
            //TO073007C - Switched from `for` to `while`.
            while (channelList != NULL)
            {
                channelDefinition = NIMEX_ChannelList_value(channelList);//TO061207A - No more direct access to the fields of the structure.
                //TO073007C - Watch out for the beguiling empty channel at the start of the list. -- 7/30/07
                if (channelDefinition != NULL)
                    mxSetCell(channelNameList, j++, mxCreateString(channelDefinition->physicalChannel));
                channelList = NIMEX_ChannelList_next(channelList);
            }
            mxSetCell(plhs[0], i - 1, channelNameList);
            //if ((taskDefinition == NULL) || (taskDefinition->channels == NULL) || (NIMEX_linkedList_isempty(taskDefinition->channels)))
            //    mxSetCell(plhs[0], i - 1, NIMEX_int32_To_mxArray(0));
            //else
            //    mxSetCell(plhs[0], i - 1, NIMEX_int32_To_mxArray(NIMEX_linkedList_length(taskDefinition->channels)));
        }
        else if (strcmp(propertyName, "userData") == 0)
        {
            //TO012407A
            if (taskDefinition->userData != NULL)
                mxSetCell(plhs[0], i - 1, taskDefinition->userData);
            else
                mxSetCell(plhs[0], i - 1, mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL));
        }
        else if (strcmp(propertyName, "samplingRate") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_float64_To_mxArray(taskDefinition->samplingRate));
        else if (strcmp(propertyName, "sampleMode") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_int32_To_mxArray(taskDefinition->sampleMode));
        else if (strcmp(propertyName, "pretriggerSamples") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_uint32_To_mxArray(taskDefinition->pretriggerSamples));
        else if (strcmp(propertyName, "sampsPerChanToAcquire") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_uint64_To_mxArray(taskDefinition->sampsPerChanToAcquire));//TO080507E - Typo, uint32 --> uint64.
        else if (strcmp(propertyName, "pretriggerSamples") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_uint32_To_mxArray(taskDefinition->pretriggerSamples));
        else if (strcmp(propertyName, "triggerEdge") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_int32_To_mxArray(taskDefinition->triggerEdge));
        else if (strcmp(propertyName, "lineGrouping") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_int32_To_mxArray(taskDefinition->lineGrouping));
        else if (strcmp(propertyName, "timeout") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_int32_To_mxArray(taskDefinition->timeout));
        else if (strcmp(propertyName, "triggerSource") == 0)
            mxSetCell(plhs[0], i - 1, mxCreateString(taskDefinition->triggerSource));
        else if (strcmp(propertyName, "everyNSamples") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_uint32_To_mxArray(taskDefinition->everyNSamples));//TO012907H
        else if (strcmp(propertyName, "repeatOutput") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_uint32_To_mxArray(taskDefinition->repeatOutput));//TO022307B
        else if (strcmp(propertyName, "autoDispatch") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_uint32_To_mxArray(taskDefinition->autoDispatch));
        else if (strcmp(propertyName, "idleState") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_uint32_To_mxArray(taskDefinition->idleState));//TO073107E
        else if (strcmp(propertyName, "started") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_int32_To_mxArray(taskDefinition->started));//TO101707I
        else if (strcmp(propertyName, "done") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_int32_To_mxArray(taskDefinition->done));//TO101807H
        else if (strcmp(propertyName, "autoRestart") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_int32_To_mxArray(taskDefinition->autoRestart));//TO101907A
        else if (strcmp(propertyName, "forceFullBuffering") == 0)
            mxSetCell(plhs[0], i - 1, NIMEX_int32_To_mxArray(taskDefinition->forceFullBuffering));//TO102407A
        else
            mexPrintf("NIMEX_getTaskProperty: Warning - Unrecognized task property name '%s'\n", propertyName);
    }
    
    NIMEX_MACRO_VERBOSE("NIMEX_getTaskProperty: \n")
    #ifdef NIMEX_VERBOSE
        NIMEX_displayTaskDefinition("    ", taskDefinition);//TO010707A
    #endif
    
    NIMEX_MACRO_POST_MEX
    
    return;
}
