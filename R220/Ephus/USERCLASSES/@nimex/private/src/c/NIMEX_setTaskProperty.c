/**
 * @file NIMEX_setTaskProperty.c
 *
 * @brief Modifies directly accesible properties in a NIMEX_TaskDefinition structure.
 *
 * @note Not all properties are settable.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_setChannelProperty(taskDefinition, propertyName, propertyValue, ...)</tt>
 *  @arg <tt>taskDefinition</tt> - The task whose fields to set.
 *  @arg <tt>propertyName</tt> - The name of the property to be set. Multiple properties may be specified.
 *  @arg <tt>propertyValue</tt> - The value of the property to be set. Multiple properties may be specified.
 *
 * <b>Settable properties:</b>\n
 *  @li clockActiveEdge
 *  @li clockSource
 *  @li clockExportTerminal
 *  @li userData 
 *  @li samplingRate
 *  @li sampleMode
 *  @li sampsPerChanToAcquire
 *  @li pretriggerSamples
 *  @li triggerEdge
 *  @li lineGrouping
 *  @li triggerSource
 *  @li everyNSamples
 *  @li repeatOutput
 *  @li autoDispatch
 *  @li idleState
 *  @li autoRestart
 *  @li forceFullBuffering
 *
 * @see NIMEX_ChannelDefinition
 *
 * @author Timothy O'Connor
 * @date 12/8/06
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006</em>
 */
/*
 * CHANGES
 *  TO010607B - Implemented NIMEX_MACRO_MXARRAY_2_PERSISTENT_CHAR. -- Tim O'Connor 1/6/07
 *  TO010707A: Created NIMEX_displayTaskDefinition and NIMEX_displayChannelDefinition. -- Tim O'Connor 1/7/07
 *  TO012407A: Added userData field. -- Tim O'Connor 1/24/07
 *  TO012907C: Moved properties from the NIMEX_ChannelDefinition to the  NIMEX_TaskDefinition, to better align with NIDAQmx. -- Tim O'Connor 1/29/07
 *  TO012907H: Allow user-control of the N in everyNSamples. -- Tim O'Connor 1/29/07
 *  TO013007B: Clear property strings from memory on overwrite. -- Tim O'Connor 1/30/07
 *  TO022307B: Add RepeatOutput functionality, by hijacking everyNSamples (which then becomes unavailable). -- Tim O'Connor 2/23/07
 *  TO022807C - Added mexFunction pre and post macros. -- Tim O'Connor 2/28/07
 *  TO040207G: Created NIMEX_MatlabVariable_2_DAQmx_Constant. -- Tim O'Connor 4/2/07
 *  TO071107A - C89 compatibility for MSVC's cl compiler. -- Tim O'Connor 7/11/07
 *  TO073107C - General clean-up done during major overhaul (TO073107B). -- Tim O'Connor 7/31/07
 *  TO073107E: Added counter/timer functionality. -- Tim O'Connor 7/31/07
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
    NIMEX_TaskDefinition* taskDefinition = (NIMEX_TaskDefinition *)NULL;
    char* propertyName = (char *)NULL;
    int i = 0;//TO071107A

    NIMEX_MACRO_PRE_MEX
    
    if (nlhs != 0)
        mexErrMsgTxt("NIMEX_setTaskProperty: No return arguments supported.");
    
    //There must be a NIMEX_TaskDefinition and then an even number of key-value pairs.
    if (nrhs < 3)
        mexErrMsgTxt("NIMEX_setTaskProperty: Insufficient arguments (must be at least 3).");
    
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_setTaskProperty: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

    NIMEX_MACRO_VERBOSE("NIMEX_setTaskProperty: Setting property (or properties) for @%p.\n", (long)taskDefinition)
    for (i = 1; i < nrhs; i += 2)
    {
        propertyName = mxArrayToString(prhs[i]);

        NIMEX_MACRO_VERBOSE_3("NIMEX_setTaskProperty: Setting '%s'...\n", propertyName)
        if (strcmp(propertyName, "clockActiveEdge") == 0)
            taskDefinition->clockActiveEdge = (uInt32)(NIMEX_MatlabVariable_2_DAQmx_Constant(prhs[i + 1]));//TO040207G
        else if (strcmp(propertyName, "clockSource") == 0)
        {
            //TO013007B
            if (taskDefinition->clockSource != NULL)
                NIMEX_freePersistentMemorySegment(taskDefinition->clockSource);

            taskDefinition->clockSource = NIMEX_MACRO_MXARRAY_2_PERSISTENT_CHAR(prhs[i + 1]);//TO010607B NIMEX_memcpy(mxArrayToString(prhs[i + 1]), (mxGetNumberOfElements(prhs[i + 1]) + 1) * sizeof(char));
        }
        else if (strcmp(propertyName, "clockExportTerminal") == 0)
        {
            //TO013007B
            if (taskDefinition->clockExportTerminal != NULL)
                NIMEX_freePersistentMemorySegment(taskDefinition->clockExportTerminal);

            taskDefinition->clockExportTerminal = NIMEX_MACRO_MXARRAY_2_PERSISTENT_CHAR(prhs[i + 1]);//TO010607B NIMEX_memcpy(mxArrayToString(prhs[i + 1]), (mxGetNumberOfElements(prhs[i + 1]) + 1) * sizeof(char));
        }
        else if (strcmp(propertyName, "userData") == 0)
        {
            //TO012407A
            if (taskDefinition->userData != NULL)
            {
                mxDestroyArray(taskDefinition->userData);//TO080813A
            }
            mexMakeArrayPersistent(((mxArray *)prhs[i+1]));//Won't get modified, but the array is const. Issues a C4090 warning.
            taskDefinition->userData = mxDuplicateArray(prhs[i+1]);//TO073107C - This should be duped, I think, instead of stored directly.
            mexMakeArrayPersistent(taskDefinition->userData);//TO073107C - Make sure this gets properly cleared later...
        }
        else if (strcmp(propertyName, "samplingRate") == 0)
            taskDefinition->samplingRate = (float64)*(mxGetPr(prhs[i + 1]));
        else if (strcmp(propertyName, "sampleMode") == 0)
            taskDefinition->sampleMode = (int32)(NIMEX_MatlabVariable_2_DAQmx_Constant(prhs[i + 1]));//TO040207G
        else if (strcmp(propertyName, "sampsPerChanToAcquire") == 0)
            taskDefinition->sampsPerChanToAcquire = (int32)*(mxGetPr(prhs[i + 1]));
        else if (strcmp(propertyName, "pretriggerSamples") == 0)
            taskDefinition->pretriggerSamples = (uInt32)*(mxGetPr(prhs[i + 1]));
        else if (strcmp(propertyName, "triggerEdge") == 0)
            taskDefinition->triggerEdge = (int32)(NIMEX_MatlabVariable_2_DAQmx_Constant(prhs[i + 1]));//TO040207G
        else if (strcmp(propertyName, "lineGrouping") == 0)
            taskDefinition->lineGrouping = (int32)(NIMEX_MatlabVariable_2_DAQmx_Constant(prhs[i + 1]));//TO040207G
        else if (strcmp(propertyName, "triggerSource") == 0)
        {
            //TO013007B
            if (taskDefinition->triggerSource != NULL)
                NIMEX_freePersistentMemorySegment(taskDefinition->triggerSource);

            taskDefinition->triggerSource = NIMEX_MACRO_MXARRAY_2_PERSISTENT_CHAR(prhs[i + 1]);//TO010607B NIMEX_memcpy(mxArrayToString(prhs[i + 1]), (mxGetNumberOfElements(prhs[i + 1]) + 1) * sizeof(char));
        }
        else if (strcmp(propertyName, "everyNSamples") == 0)
            taskDefinition->everyNSamples = (uInt32)*(mxGetPr(prhs[i + 1]));//TO012907H
        else if (strcmp(propertyName, "repeatOutput") == 0)
            taskDefinition->repeatOutput = (uInt32)*(mxGetPr(prhs[i + 1]));//TO022307B
        else if (strcmp(propertyName, "autoDispatch") == 0)
            taskDefinition->autoDispatch = (int32)*(mxGetPr(prhs[i + 1]));
        else if (strcmp(propertyName, "idleState") == 0)
            taskDefinition->idleState = (int32)(NIMEX_MatlabVariable_2_DAQmx_Constant(prhs[i + 1]));//TO073107E
        else if (strcmp(propertyName, "autoRestart") == 0)
            taskDefinition->autoRestart = (int32)*(mxGetPr(prhs[i + 1]));//TO101907A
        else if (strcmp(propertyName, "forceFullBuffering") == 0)
            taskDefinition->forceFullBuffering = (int32)*(mxGetPr(prhs[i + 1]));//TO102407A
        else
            mexPrintf("NIMEX_setTaskProperty: Warning - Unrecognized task property name '%s'\n", propertyName);
    }
    
    NIMEX_MACRO_VERBOSE("NIMEX_setTaskProperty: \n")
    #ifdef NIMEX_VERBOSE
        NIMEX_displayTaskDefinition("    ", taskDefinition);//TO010707A
    #endif
 
    NIMEX_MACRO_POST_MEX
    
    return;
}
