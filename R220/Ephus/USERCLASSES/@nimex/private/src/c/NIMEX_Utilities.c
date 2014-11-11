/**
 * @file NIMEX_Utilities.c
 *
 * @brief Implementation of helpful structures/functions throughout NIDAQmex.
 *
 * NOTE: <em>All initialization of mutexes is protected by checks for previous initialization, and that's fine in this scenario.\n
 *    It is threadsafe because it's initiated from Matlab, which is single threaded. Calling these functions from another thread, before\n
 *    Matlab has had a chance to force their initialization is not valid and can produce race conditions.</em>
 *
 * @author Timothy O'Connor 
 * @date 11/16/06
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006</em>
 *
 */
/*
 * CHANGES
 *  TO120806A: Fixed printout of samplingRate field. -- Tim O'Connor 12/8/06
 *  TO010607C: Created NIMEX_genericStructures. -- Tim O'Connor 1/6/07
 *  TO010607D: Created NIMEX_memManagement. -- Tim O'Connor 1/6/07
 *  TO010707A: Created NIMEX_displayTaskDefinition & NIMEX_displayChannelDefinition. -- Tim O'Connor 1/7/07
 *  TO010707C: Created NIMEX_uint32_To_mxArray, NIMEX_int32_To_mxArray, NIMEX_float64_To_mxArray, & NIMEX_uint64_To_mxArray. -- Tim O'Connor 1/7/07
 *  TO011707A: Improved NIMEX_displayTaskDefinition & NIMEX_displayChannelDefinition to translate constants into NIMEX_Constants.h & nidaqmx.h based #define values. -- Tim O'Connor 1/17/07
 *  TO011907A: Added NIMEX_MACRO_CONST_2_STR. -- Tim O'Connor 1/19/07
 *  TO012407B: Added NIMEX_acquireLock & NIMEX_releaseLock for thread safety. -- Tim O'Connor 1/24/07
 *  TO012407C: Created NIMEX_implementCallback to simplify standardized callback execution. -- Tim O'Connor 1/24/07
 *  TO012907B: Corrected NIMEX_getChannelByName to look for physicalChannel and do proper strcmp calls. -- Tim O'Connor 1/29/07
 *  TO012907C: Moved properties from the NIMEX_ChannelDefinition to the  NIMEX_TaskDefinition, to better align with NIDAQmx. -- Tim O'Connor 1/29/07
 *  TO012907C: Moved properties from the NIMEX_ChannelDefinition to the  NIMEX_TaskDefinition, to better align with NIDAQmx. -- Tim O'Connor 1/29/07
 *  TO012907D: Created NIMEX_createChannelDefinition utility function. -- Tim O'Connor 1/29/07
 *  TO012907E: Added a flag to indicate the start-status of a task. -- Tim O'Connor 1/29/07
 *  TO013007B: Changed display functions to calle mexPrintf directly, instead of NIMEX_MACRO_VERBOSE, since they are now accessible via Matlab commands. -- Tim O'Connor 1/30/07
 *  TO033007C: Display dataBuffer pointer as part of the channelDefinition. -- Tim O'Connor 3/30/07
 *  TO033007D: Added dataBufferSize to the NIMEX_ChannelDefinition structure. -- Tim O'Connor 3/30/07
 *  TO040207F: Added NIMEX_destroyCallback. -- Tim O'Connor 4/2/07
 *  TO040207G: Created NIMEX_MatlabVariable_2_DAQmx_Constant. -- Tim O'Connor 4/2/07
 *  TO061207A: Port basic structures to glib. -- Tim O'Connor 6/12/07
 *  TO061907A: Created NIMEX_wrapArray2mxArray. -- Tim O'Connor 6/19/07
 *  TO061907B: Created NIMEX_Callbacks.h. -- Tim O'Connor 6/19/07
 *  TO061907C: Created NIMEX_Objects.h. -- Tim O'Connor 6/19/07
 *  TO061907E: Replaced everyNCallbacks and doneCallbacks with eventMap, added dataDispatcher and autoDispatch. -- Tim O'Connor 6/19/07
 *  TO071107A - C89 compatibility for MSVC's cl compiler. -- Tim O'Connor 7/11/07
 *  TO073070A: Added enable field to channels, allowing them to be defined but not started. -- Tim O'Connor 7/30/07
 *  TO073107E: Added counter/timer functionality. -- Tim O'Connor 7/31/07
 *  TO080107A: Re-engineered constants and added more conversions. -- Tim O'Connor 8/1/07
 *  TO101007H: Changed mutex from a Windows mutex (of type HANDLE) to a CRITICAL_SECTION type. -- Tim O'Connor 10/10/07
 *  TO101707A: Created NIMEX_acquireLockExclusively/NIMEX_releaseLockExclusively, to block the Windows message pump from entering critical code while waiting for blocking calls in NIDAQmx to return. -- Tim O'Connor 10/17/07
 *  TO101707C: Mu! Back out TO101707A, for now, as it was based on an invalid test scenario. -- Tim O'Connor 10/17/07
 *  TO101708J: Allow for incremental state transitions (verify, commit, etc). -- Tim O'Connor 10/18/07
 *  TO102007B: Broke NIMEX_updateBuffersByCallbacksAndWriteToTask out into a separate function, because it's needed for setting up a retriggered acquisition. -- Tim O'Connor 10/20/07
 *  JL011808A: Change free to mxfree because concatenatedDataBuffer is allocated using mxCalloc
 *  TO033008F: The sample clock behavior is only valid for analog input tasks. -- Tim O'Connor 3/30/08
 *  TO051408A: Ported documentation to Doxygen. -- Tim O'Connor 5/14/08
 *  TO083108B: Print a warning when not enough samples are available in a dataSource, so it's clear what's going on. -- Tim O'Connor 8/31/08
 *  TO090108A: Due to incompatibilities between 2007a and 2007b regarding mexSetTrapFlag (see TO082908A), we must check the Matlab version at runtime to determine behavior. -- Tim O'Connor 9/1/08
 *  TO102508E: Added the sampleClockTimebaseSource property. -- Tim O'Connor 10/25/08
 *  TO080813C: Created NIMEX_transientWrapArray2mxArray to allow non-persistent wrapping. -- Tim O'Connor 8/8/13
 *
 */
#include "nimex.h"

static char* NIMEX_RuntimeMatlabVersionString = NULL;
static double NIMEX_RuntimeMatlabVersionNumber = 0;
static char* NIMEX_RuntimeMatlabReleaseString = NULL;

/**
 * @brief Takes a C-level array and appropriately wraps it into a Matlab compatible mxArray, based on the classID.
 * @arg <tt>wrapped</tt> - The resulting mxArray* will be placed into this pointer, which will not be persistent (see mexMakeArrayPersistent).
 * @arg <tt>arraySize</tt> - The size of the data array to be wrapped, in bytes.
 * @arg <tt>classID</tt> - The Matlab mex interface's classID value. Either mxDOUBLE_CLASS or mxUINT32_CLASS.
 * @arg <tt>data</tt> - The data array to be wrapped.
 *
 * @callergraph
 */
//TO080813C - Created to allow non-persistent wrapping.
//TO080813C - Created NIMEX_transientWrapArray2mxArray to allow non-persistent wrapping. -- Tim O'Connor 8/8/13
void NIMEX_transientWrapArray2mxArray(mxArray** wrapped, int32 arraySize, int32 classID, void* data)
{
    int                dims[2] = {0, 0};
    
    if (classID == mxDOUBLE_CLASS)
    {
        wrapped[0] = mxCreateDoubleMatrix(arraySize, 1,  mxDOUBLE_CLASS);
        memcpy(mxGetPr(*(wrapped)), data, arraySize * sizeof(float64));
    }
    else if (classID == mxUINT32_CLASS)
    {
        dims[0] = arraySize;
        wrapped[0] = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);
        memcpy(mxGetPr(*(wrapped)), data, arraySize * sizeof(int32));
    }
    else
    {
        mexPrintf("NIMEX_Utilities/NIMEX_wrapArray2mxArray: Error - Unrecognized data type %d. Returning empty array.\n", classID);
        wrapped[0] = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);//Return an empty array.
    }
    
    mexMakeArrayPersistent(wrapped[0]);
    
    return;
}

/**
 * @brief Takes a C-level array and appropriately wraps it into a Matlab compatible mxArray, based on the classID.
 * @arg <tt>wrapped</tt> - The resulting mxArray* will be placed into this pointer and will be persistent (see mexMakeArrayPersistent).
 * @arg <tt>arraySize</tt> - The size of the data array to be wrapped, in bytes.
 * @arg <tt>classID</tt> - The Matlab mex interface's classID value. Either mxDOUBLE_CLASS or mxUINT32_CLASS.
 * @arg <tt>data</tt> - The data array to be wrapped.
 *
 * @callergraph
 */
//TO061907A
//TO080813C - Created NIMEX_transientWrapArray2mxArray to allow non-persistent wrapping. -- Tim O'Connor 8/8/13
void NIMEX_wrapArray2mxArray(mxArray** wrapped, int32 arraySize, int32 classID, void* data)
{
    NIMEX_transientWrapArray2mxArray(wrapped, arraySize, classID, data);
    
    mexMakeArrayPersistent(wrapped[0]);
    
    return;
}

/**
 * @brief Convenience wrapper to the standard memcpy, which also allocates the appropriate memory
 * @arg <tt>src</tt> - A pointer to the start of the memory to be copied.
 * @arg <tt>n</tt> - The number of bytes to be copied.
 *
 * @return A newly allocated block of memory, containing the contents of <tt>src</tt>.
 *
 * @callergraph
 */
void* NIMEX_memcpy(const void* src, size_t n)
{
    void* dest = (void *)NULL;

    dest = NIMEX_Persistent_Calloc(1, n);
    if (dest == NULL)
        mexErrMsgTxt("NIMEX_memcpy: Failed to allocate memory block.\n");
    
    NIMEX_MACRO_VERBOSE_3("NIMEX_Utilities: NIMEX_memcpy - Copying %d bytes into new (non-Matlab managed) array. From @%p to @%p\n", n, src, dest)
    memcpy(dest, src, n);
    
    return dest;
}

/**
 * @brief Converts a Matlab variable to a NIDAQmx constant, converting strings as necessary.
 * @arg <tt>matlabVariable</tt> - A number (which will not be inspected) or a string (such as "<tt>DAQmx_Val_Default</tt>").
 *
 * @return If a number was input, it is returned directly. Otherwise, a look-up is performed on the supplied string, and the resulting value is returned. Returns -1 if no conversion can be found.
 *
 * @callergraph
 */
//TO040207G
int32 NIMEX_MatlabVariable_2_DAQmx_Constant(const mxArray* matlabValue)
{
    int32 nidaqmxValue = -1;
    char* constantName = (char *)NULL;
    
    if (mxGetClassID(matlabValue) == mxCHAR_CLASS)
    {
        constantName = mxArrayToString(matlabValue);

        if (strcmp(constantName, "DAQmx_Val_Default") == 0)
            nidaqmxValue = DAQmx_Val_Default;
        else if (strcmp(constantName, "DAQmx_Val_ChanForAllLines") == 0)
            nidaqmxValue = DAQmx_Val_ChanForAllLines;
        else if (strcmp(constantName, "DAQmx_Val_ContSamps") == 0)
            nidaqmxValue = DAQmx_Val_ContSamps;
        else if (strcmp(constantName, "DAQmx_Val_Diff") == 0)
            nidaqmxValue = DAQmx_Val_Diff;
        else if (strcmp(constantName, "DAQmx_Val_Falling") == 0)
            nidaqmxValue = DAQmx_Val_Falling;
        else if (strcmp(constantName, "DAQmx_Val_FiniteSamps") == 0)
            nidaqmxValue = DAQmx_Val_FiniteSamps;
        else if (strcmp(constantName, "DAQmx_Val_HWTimedSinglePoint") == 0)
            nidaqmxValue = DAQmx_Val_HWTimedSinglePoint;
        else if (strcmp(constantName, "DAQmx_Val_PseudoDiff") == 0)
            nidaqmxValue = DAQmx_Val_PseudoDiff;
        else if (strcmp(constantName, "DAQmx_Val_Rising") == 0)
            nidaqmxValue = DAQmx_Val_Rising;
        else if (strcmp(constantName, "DAQmx_Val_Volts") == 0)
            nidaqmxValue = DAQmx_Val_Volts;
        else if (strcmp(constantName, "DAQmx_Val_DynamicSignalAcquisition") == 0)
            nidaqmxValue = DAQmx_Val_DynamicSignalAcquisition;
        else if (strcmp(constantName, "DAQmx_Val_Switches") == 0)
            nidaqmxValue = DAQmx_Val_Switches;
        else if (strcmp(constantName, "DAQmx_Val_CompactDAQChassis") == 0)
            nidaqmxValue = DAQmx_Val_CompactDAQChassis;
        else if (strcmp(constantName, "DAQmx_Val_CSeriesModule") == 0)
            nidaqmxValue = DAQmx_Val_CSeriesModule;
        else if (strcmp(constantName, "DAQmx_Val_SCXIModule") == 0)
            nidaqmxValue = DAQmx_Val_SCXIModule;
        else if (strcmp(constantName, "DAQmx_Val_Unknown") == 0)
            nidaqmxValue = DAQmx_Val_Unknown;
        //TO073107E
        else if (strcmp(constantName, "DAQmx_Val_High") == 0)
            nidaqmxValue = DAQmx_Val_High;
        else if (strcmp(constantName, "DAQmx_Val_Low") == 0)
            nidaqmxValue = DAQmx_Val_Low;
        else if (strcmp(constantName, "DAQmx_Val_Seconds") == 0)
            nidaqmxValue = DAQmx_Val_Seconds;
        else if (strcmp(constantName, "DAQmx_Val_Hz") == 0)
            nidaqmxValue = DAQmx_Val_Hz;
        //TO080107A
        else if (strcmp(constantName, "DAQmx_Val_Auto") == 0)
            nidaqmxValue = DAQmx_Val_Auto;
        else if (strcmp(constantName, "DAQmx_Val_DMA") == 0)
            nidaqmxValue = DAQmx_Val_DMA;
        else if (strcmp(constantName, "DAQmx_Val_MSeriesDAQ") == 0)
            nidaqmxValue = DAQmx_Val_MSeriesDAQ;
        else if (strcmp(constantName, "DAQmx_Val_ESeriesDAQ") == 0)
            nidaqmxValue = DAQmx_Val_ESeriesDAQ;
        else if (strcmp(constantName, "DAQmx_Val_SSeriesDAQ") == 0)
            nidaqmxValue = DAQmx_Val_SSeriesDAQ;
        else if (strcmp(constantName, "DAQmx_Val_BSeriesDAQ") == 0)
            nidaqmxValue = DAQmx_Val_BSeriesDAQ;
        else if (strcmp(constantName, "DAQmx_Val_Rising") == 0)
            nidaqmxValue = DAQmx_Val_Rising;
        else if (strcmp(constantName, "DAQmx_Val_SCSeriesDAQ") == 0)
            nidaqmxValue = DAQmx_Val_SCSeriesDAQ;
        else if (strcmp(constantName, "DAQmx_Val_USBDAQ") == 0)
            nidaqmxValue = DAQmx_Val_USBDAQ;
        else if (strcmp(constantName, "DAQmx_Val_AOSeries") == 0)
            nidaqmxValue = DAQmx_Val_AOSeries;
        else if (strcmp(constantName, "DAQmx_Val_Volts") == 0)
            nidaqmxValue = DAQmx_Val_Volts;
        else if (strcmp(constantName, "DAQmx_Val_DigitalIO") == 0)
            nidaqmxValue = DAQmx_Val_DigitalIO;
        else if (strcmp(constantName, "DAQmx_Val_TIOSeries") == 0)
            nidaqmxValue = DAQmx_Val_TIOSeries;
        else
        {
            mexPrintf("NIMEX_Utilities/NIMEX_MatlabVariable_2_DAQmx_Constant - Unrecognized string: '%s'\n", constantName);
            mexErrMsgTxt("Failed to convert string into DAQmx constant.");
        }
    }
    else if (mxIsNumeric(matlabValue))
    {
        nidaqmxValue = (int32)*(mxGetPr(matlabValue));
    }
    else
        mexPrintf("NIMEX_Utilities/NIMEX_MatlabVariable_2_DAQmx_Constant - Only strings and numbers may be converted into DAQmx constants: %d\n", mxGetClassID(matlabValue));

    return nidaqmxValue;
}

/**
 * @brief Converts a NIDAQmx variable to a human interpretable string.
 * @arg <tt>value</tt> - A NIDAQmx constant.
 *
 * @return A string representation of the constant, suitable for printing.
 */
char* NIMEX_DAQmx_Constant_2_string(int32 value)
{
    //char s[35] = {'UNSPECIFIED_DAQmx_VALUE (     )'};
    
    //TO011907A
    switch (value)
    {
        case DAQmx_Val_Default:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_Default) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_Default) ")";

        case DAQmx_Val_ChanForAllLines:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_ChanForAllLines) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_ChanForAllLines) ")";

        case DAQmx_Val_ContSamps:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_ContSamps) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_ContSamps) ")";

        case DAQmx_Val_Diff:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_Diff) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_Diff) ")";

        case DAQmx_Val_Falling:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_Falling) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_Falling) ")";

        case DAQmx_Val_FiniteSamps:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_FiniteSamps) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_FiniteSamps) ")";
            
        case DAQmx_Val_HWTimedSinglePoint:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_HWTimedSinglePoint) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_HWTimedSinglePoint) ")";

        case DAQmx_Val_PseudoDiff:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_PseudoDiff) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_PseudoDiff) ")";

        case DAQmx_Val_Rising:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_Rising) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_Rising) ")";

        case DAQmx_Val_Volts:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_Volts) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_Volts) ")";

        case DAQmx_Val_DynamicSignalAcquisition:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_DynamicSignalAcquisition) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_DynamicSignalAcquisition) ")";

        case DAQmx_Val_Switches:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_Switches) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_Switches) ")";

        case DAQmx_Val_CompactDAQChassis:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_CompactDAQChassis) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_CompactDAQChassis) ")";

        case DAQmx_Val_CSeriesModule:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_CSeriesModule) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_CSeriesModule) ")";

        case DAQmx_Val_SCXIModule:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_SCXIModule) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_SCXIModule) ")";

        case DAQmx_Val_Unknown:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_Unknown) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_Unknown) ")";

        //TO073107E
        case DAQmx_Val_Hz:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_Hz) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_Hz) ")";

        case DAQmx_Val_Seconds:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_Seconds) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_Seconds) ")";

        case DAQmx_Val_High:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_High) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_High) ")";

        case DAQmx_Val_Low:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_Low) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_Low) ")";

         ////TO080107A
        //DAQmx_Val_Auto == DAQmx_Val_Default
        //case DAQmx_Val_Auto:
            //return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_Auto) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_Auto) ")";

        case DAQmx_Val_DMA:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_DMA) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_DMA) ")";
        case DAQmx_Val_MSeriesDAQ:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_MSeriesDAQ) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_MSeriesDAQ) ")";

        case DAQmx_Val_ESeriesDAQ:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_ESeriesDAQ) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_ESeriesDAQ) ")";

        case DAQmx_Val_SSeriesDAQ:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_SSeriesDAQ) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_SSeriesDAQ) ")";

        case DAQmx_Val_BSeriesDAQ:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_BSeriesDAQ) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_BSeriesDAQ) ")";

        case DAQmx_Val_SCSeriesDAQ:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_SCSeriesDAQ) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_SCSeriesDAQ) ")";

        case DAQmx_Val_USBDAQ:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_USBDAQ) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_USBDAQ) ")";

        case DAQmx_Val_AOSeries:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_AOSeries) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_AOSeries) ")";

        case DAQmx_Val_DigitalIO:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_DigitalIO) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_DigitalIO) ")";

        case DAQmx_Val_TIOSeries:
            return NIMEX_MACRO_CONST_2_NAME(DAQmx_Val_TIOSeries) " (" NIMEX_MACRO_CONST_2_VAL(DAQmx_Val_TIOSeries) ")";

        default:
            //sprintf(s, "UNSPECIFIED_DAQmx_VALUE (%d)", value);
            return "UNSPECIFIED_DAQmx_VALUE";
    }
}

/**
 * @brief Converts a Matlab variable to a NIMEX constant, converting strings as necessary.
 * @arg <tt>matlabVariable</tt> - A number (which will not be inspected) or a string (such as "<tt>NIMEX_ANALOG_INPUT</tt>").
 *
 * @return If a number was input, it is returned directly. Otherwise, a look-up is performed on the supplied string, and the resulting value is returned. Returns NIMEX_UNINITIALIZED if no conversion can be found.
 *
 * @callergraph
 */
//TO080107A
int32 NIMEX_MatlabVariable_2_NIMEX_Constant(const mxArray* matlabValue)
{
    int32 nimexValue = NIMEX_UNINITIALIZED;
    char* constantName = (char *)NULL;
    
    if (mxGetClassID(matlabValue) == mxCHAR_CLASS)
    {
        constantName = mxArrayToString(matlabValue);

        if (strcmp(constantName, "NIMEX_ANALOG_INPUT") == 0)
            nimexValue = NIMEX_ANALOG_INPUT;
        else if (strcmp(constantName, "NIMEX_ANALOG_OUTPUT") == 0)
            nimexValue = NIMEX_ANALOG_OUTPUT;
        else if (strcmp(constantName, "NIMEX_DIGITAL_INPUT") == 0)
            nimexValue = NIMEX_DIGITAL_INPUT;
        else if (strcmp(constantName, "NIMEX_DIGITAL_OUTPUT") == 0)
            nimexValue = NIMEX_DIGITAL_OUTPUT;
        else if (strcmp(constantName, "NIMEX_CO_FREQUENCY") == 0)
            nimexValue = NIMEX_CO_FREQUENCY;
        else if (strcmp(constantName, "NIMEX_CO_TIME") == 0)
            nimexValue = NIMEX_CO_TIME;
        else if (strcmp(constantName, "NIMEX_STATE_NEW") == 0)
            nimexValue = NIMEX_STATE_NEW;
        else if (strcmp(constantName, "NIMEX_STATE_READY") == 0)
            nimexValue = NIMEX_STATE_READY;
        else if (strcmp(constantName, "NIMEX_STATE_WAITING_FOR_TRIGGER") == 0)
            nimexValue = NIMEX_STATE_WAITING_FOR_TRIGGER;
        else if (strcmp(constantName, "NIMEX_STATE_RUNNING") == 0)
            nimexValue = NIMEX_STATE_RUNNING;
        else if (strcmp(constantName, "NIMEX_STATE_DISPATCHING") == 0)
            nimexValue = NIMEX_STATE_DISPATCHING;
        else if (strcmp(constantName, "NIMEX_STATE_WAITING_FOR_EVENT") == 0)
            nimexValue = NIMEX_STATE_WAITING_FOR_EVENT;
        else if (strcmp(constantName, "NIMEX_DATASOURCE_TYPE_FLOAT64") == 0)
            nimexValue = NIMEX_DATASOURCE_TYPE_FLOAT64;
        else if (strcmp(constantName, "NIMEX_DATASOURCE_TYPE_UINT32") == 0)
            nimexValue = NIMEX_DATASOURCE_TYPE_UINT32;
        else if (strcmp(constantName, "NIMEX_DATASOURCE_TYPE_CALLBACK") == 0)
            nimexValue = NIMEX_DATASOURCE_TYPE_CALLBACK;
        else if (strcmp(constantName, "NIMEX_DATASOURCE_TYPE_FLOAT64_BITMASK") == 0)
            nimexValue = NIMEX_DATASOURCE_TYPE_FLOAT64_BITMASK;
        else if (strcmp(constantName, "NIMEX_DATASOURCE_TYPE_UINT32_BITMASK") == 0)
            nimexValue = NIMEX_DATASOURCE_TYPE_UINT32_BITMASK;
        else if (strcmp(constantName, "NIMEX_DATASOURCE_TYPE_CALLBACK_BITMASK") == 0)
            nimexValue = NIMEX_DATASOURCE_TYPE_CALLBACK_BITMASK;
        else if (strcmp(constantName, "NIMEX_DATASOURCE_TYPE_FLOAT64CALLBACK") == 0)
            nimexValue = NIMEX_DATASOURCE_TYPE_FLOAT64CALLBACK;
        else if (strcmp(constantName, "NIMEX_DATASOURCE_TYPE_UINT32CALLBACK") == 0)
            nimexValue = NIMEX_DATASOURCE_TYPE_UINT32CALLBACK;
        else
        {
            mexPrintf("NIMEX_Utilities/NIMEX_MatlabVariable_2_NIMEX_Constant - Unrecognized string: '%s'\n", constantName);
            mexErrMsgTxt("Failed to convert string into NIMEX constant.");
        }
    }
    else if (mxIsNumeric(matlabValue))
    {
        nimexValue = (int32)*(mxGetPr(matlabValue));
    }
    else
        mexPrintf("NIMEX_Utilities/NIMEX_MatlabVariable_2_NIMEX_Constant - Only strings and numbers may be converted into NIMEX constants: %d\n", mxGetClassID(matlabValue));

    return nimexValue;
}

/**
 * @brief Converts a NIMEX variable to a human interpretable string.
 * @arg <tt>value</tt> - A NIMEX constant.
 *
 * @return A string representation of the constant, suitable for printing.
 *
 * @callergraph
 */
//TO080107A
char* NIMEX_Constant_2_string(int32 value)
{
    switch (value)
    {
        case NIMEX_ANALOG_INPUT:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_ANALOG_INPUT) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_ANALOG_INPUT) ")";
        
        case NIMEX_ANALOG_OUTPUT:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_ANALOG_OUTPUT) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_ANALOG_OUTPUT) ")";
            
        case NIMEX_DIGITAL_INPUT:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_DIGITAL_INPUT) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_DIGITAL_INPUT) ")";
            
        case NIMEX_DIGITAL_OUTPUT:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_DIGITAL_OUTPUT) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_DIGITAL_OUTPUT) ")";
            
        case NIMEX_CO_FREQUENCY:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_CO_FREQUENCY) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_CO_FREQUENCY) ")";
            
        case NIMEX_CO_TIME:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_CO_TIME) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_CO_TIME) ")";
            
        case NIMEX_STATE_NEW:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_STATE_NEW) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_STATE_NEW) ")";

        case NIMEX_STATE_READY:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_STATE_READY) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_STATE_READY) ")";

        case NIMEX_STATE_WAITING_FOR_TRIGGER:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_STATE_WAITING_FOR_TRIGGER) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_STATE_WAITING_FOR_TRIGGER) ")";

        case NIMEX_STATE_RUNNING:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_STATE_RUNNING) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_STATE_RUNNING) ")";

        case NIMEX_STATE_PAUSED:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_STATE_PAUSED) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_STATE_PAUSED) ")";

        case NIMEX_STATE_DISPATCHING:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_STATE_DISPATCHING) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_STATE_DISPATCHING) ")";

        case NIMEX_STATE_WAITING_FOR_EVENT:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_STATE_WAITING_FOR_EVENT) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_STATE_WAITING_FOR_EVENT) ")";

        case NIMEX_DATASOURCE_TYPE_FLOAT64:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_DATASOURCE_TYPE_FLOAT64) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_DATASOURCE_TYPE_FLOAT64) ")";

        case NIMEX_DATASOURCE_TYPE_UINT32:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_DATASOURCE_TYPE_UINT32) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_DATASOURCE_TYPE_UINT32) ")";

        case NIMEX_DATASOURCE_TYPE_CALLBACK:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_DATASOURCE_TYPE_CALLBACK) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_DATASOURCE_TYPE_CALLBACK) ")";

        case NIMEX_DATASOURCE_TYPE_FLOAT64_BITMASK:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_DATASOURCE_TYPE_FLOAT64_BITMASK) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_DATASOURCE_TYPE_FLOAT64_BITMASK) ")";

        case NIMEX_DATASOURCE_TYPE_UINT32_BITMASK:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_DATASOURCE_TYPE_UINT32_BITMASK) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_DATASOURCE_TYPE_UINT32_BITMASK) ")";

        case NIMEX_DATASOURCE_TYPE_CALLBACK_BITMASK:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_DATASOURCE_TYPE_CALLBACK_BITMASK) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_DATASOURCE_TYPE_CALLBACK_BITMASK) ")";

        case NIMEX_DATASOURCE_TYPE_FLOAT64CALLBACK:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_DATASOURCE_TYPE_FLOAT64CALLBACK) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_DATASOURCE_TYPE_FLOAT64CALLBACK) ")";

        case NIMEX_DATASOURCE_TYPE_UINT32CALLBACK:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_DATASOURCE_TYPE_UINT32CALLBACK) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_DATASOURCE_TYPE_UINT32CALLBACK) ")";

        case NULL:
            return NIMEX_MACRO_CONST_2_NAME(NULL) " (" NIMEX_MACRO_CONST_2_VAL(NULL) ")";

        case NIMEX_UNINITIALIZED:
            return NIMEX_MACRO_CONST_2_NAME(NIMEX_UNINITIALIZED) " (" NIMEX_MACRO_CONST_2_VAL(NIMEX_UNINITIALIZED) ")";

        default:
            //sprintf(s, "UNSPECIFIED_DAQmx_VALUE (%d)", value);
            return "UNKNOWN_NIMEX_VALUE";
    }
};

/**
 * @brief Converts a Matlab mex variable to a human interpretable string.
 * @arg <tt>value</tt> - A Matlab mex constant.
 *
 * @return A string representation of the constant, suitable for printing.
 *
 * @callergraph
 */
//TO080107A
char* NIMEX_mxConstant_2_string(int32 value)
{
    switch (value)
    {
        case mxDOUBLE_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxDOUBLE_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxDOUBLE_CLASS) ")";

        case mxUINT32_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxUINT32_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxUINT32_CLASS) ")";

        case mxCELL_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxCELL_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxCELL_CLASS) ")";

        case mxUNKNOWN_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxUNKNOWN_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxUNKNOWN_CLASS) ")";

        case mxSTRUCT_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxSTRUCT_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxSTRUCT_CLASS) ")";

        case mxCHAR_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxCHAR_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxCHAR_CLASS) ")";

        case mxLOGICAL_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxLOGICAL_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxLOGICAL_CLASS) ")";

        case mxSINGLE_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxSINGLE_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxSINGLE_CLASS) ")";

        case mxINT8_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxINT8_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxINT8_CLASS) ")";

        case mxUINT8_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxUINT8_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxUINT8_CLASS) ")";

        case mxINT16_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxINT16_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxINT16_CLASS) ")";

        case mxUINT16_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxUINT16_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxUINT16_CLASS) ")";

        case mxINT32_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxINT32_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxINT32_CLASS) ")";

        case mxINT64_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxINT64_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxINT64_CLASS) ")";

        case mxUINT64_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxUINT64_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxUINT64_CLASS) ")";

        case mxFUNCTION_CLASS:
            return NIMEX_MACRO_CONST_2_NAME(mxFUNCTION_CLASS);// " (" NIMEX_MACRO_CONST_2_VAL(mxFUNCTION_CLASS) ")";

        default:
            return "UNKNOWN_mx_VALUE";
    }
}

/**
 * @brief Prints a NIMEX_TaskDefinition to standard output.
 * @arg <tt>prefix</tt> - An arbitrary prefix, prepended to each line of output. Typically used to control indentation.
 * @arg <tt>task</tt> - A pointer to a NIMEX_TaskDefinition.
 */
void NIMEX_displayTaskDefinition(const char* prefix, NIMEX_TaskDefinition* task)
{
    mexPrintf("%staskDefinition: @%p\n", prefix, (long)task);
    mexPrintf("%staskDefinition->taskHandle: %d\n", prefix, task->taskHandle);
    if ((task == NULL) || (task->channels == NULL) || (NIMEX_linkedList_isempty(task->channels)))
    {
        mexPrintf("%staskDefinition->channels: NONE\n", prefix);
    }
    else
    {
        mexPrintf("%staskDefinition->channels: %d @%p\n", prefix, NIMEX_linkedList_length(task->channels), task->channels);
    }
    mexPrintf("%staskDefinition->clockSource: @%p->\"%s\"\n", prefix, (long)task->clockSource, task->clockSource);
    mexPrintf("%staskDefinition->clockActiveEdge: %d\n", prefix, NIMEX_DAQmx_Constant_2_string(task->clockActiveEdge));
    mexPrintf("%staskDefinition->clockExportTerminal: @%p->\"%s\"\n", prefix, (long)task->clockExportTerminal, task->clockExportTerminal);//TO120806C
    mexPrintf("%staskDefinition->mutex: @%p\n", prefix, task->mutex);//TO012407B
    mexPrintf("%staskDefinition->triggerSource: @%p->\"%s\"\n", prefix, (long)task->triggerSource, task->triggerSource);
    //TO061907E
    if ((task->eventMap == NULL) || (NIMEX_hashTable_isempty(task->eventMap)))
    {
        mexPrintf("%staskDefinition->eventMap: NONE\n", prefix);
    }
    else
    {
        mexPrintf("%staskDefinition->eventMap: %d @%p\n", prefix, NIMEX_hashTable_size(task->eventMap), (long)task->eventMap);
    }
    mexPrintf("%staskDefinition->everyNSamples: %u\n", prefix, task->everyNSamples);
    mexPrintf("%staskDefinition->lineGrouping: %s\n", prefix, NIMEX_DAQmx_Constant_2_string(task->lineGrouping));//TO120806A
    mexPrintf("%staskDefinition->samplingRate: %3.4f\n", prefix, task->samplingRate);//TO120806A
    mexPrintf("%staskDefinition->sampleMode: %s\n", prefix, NIMEX_DAQmx_Constant_2_string(task->sampleMode));
    mexPrintf("%staskDefinition->sampsPerChanToAcquire: %d\n", prefix, task->sampsPerChanToAcquire);
    mexPrintf("%staskDefinition->pretriggerSamples: %d\n", prefix, task->pretriggerSamples);
    mexPrintf("%staskDefinition->triggerEdge: %s\n", prefix, NIMEX_DAQmx_Constant_2_string(task->triggerEdge));//TO120806A
    mexPrintf("%staskDefinition->repeatOutput: %d\n", prefix, task->repeatOutput);//TO022307B
    mexPrintf("%staskDefinition->started: %d\n", prefix, task->started);
    //TO073107E
    mexPrintf("%staskDefinition->idleState: %s\n", prefix, NIMEX_DAQmx_Constant_2_string(task->idleState));//TO120806A
    mexPrintf("%staskDefinition->done: %d\n", prefix, task->done);
    mexPrintf("%staskDefinition->autoRestart: %d\n", prefix, task->autoRestart);
}

///@deprecated @see NIMEX_Constant_2_string @callergraph
char* NIMEX_channelType_2_string(int32 channelType)
{
    return NIMEX_Constant_2_string(channelType);//TO080107A
}

/**
 * @brief Prints a NIMEX_ChannelDefinition to standard output.
 * @arg <tt>prefix</tt> - An arbitrary prefix, prepended to each line of output. Typically used to control indentation.
 * @arg <tt>task</tt> - A pointer to a NIMEX_ChannelDefinition.
 */
void NIMEX_displayChannelDefinition(const char* prefix, NIMEX_ChannelDefinition* channel)
{
    char* nextPrefix = (char *)NULL;
    int   prefixLength = 0;
    char* prefixSuffix = "channelDefinition->";

    mexPrintf("%schannelDefinition: @%p\n", prefix, (long)channel);
    mexPrintf("%schannelDefinition->channelType: %s\n", prefix, NIMEX_channelType_2_string(channel->channelType));//TO011707A
    if (channel->enable)
        mexPrintf("%schannelDefinition->enable: %d (TRUE)\n", prefix, channel->enable);//TO073007A
    else
        mexPrintf("%schannelDefinition->enable: %d (FALSE)\n", prefix, channel->enable);//TO073007A
    //NIMEX_MACRO_VERBOSE("%schannelDefinition->channelType: %s\n", prefix, NIMEX_DAQmx_Constant_2_string(channel->channelType))//TO011707A//TO011907A
    mexPrintf("%schannelDefinition->terminalConfig: %s\n", prefix, NIMEX_DAQmx_Constant_2_string(channel->terminalConfig));
    mexPrintf("%schannelDefinition->units: %s\n", prefix, NIMEX_DAQmx_Constant_2_string(channel->units));
    mexPrintf("%schannelDefinition->minVal: %3.1f\n", prefix, channel->minVal);
    mexPrintf("%schannelDefinition->maxVal: %3.1f\n", prefix, channel->maxVal);
    mexPrintf("%schannelDefinition->mnemonicName: @%p->\"%s\"\n", prefix, &(channel->mnemonicName), channel->mnemonicName);
    mexPrintf("%schannelDefinition->physicalChannel: @%p->\"%s\"\n", prefix, &(channel->physicalChannel), channel->physicalChannel);
    //TO080107B
    prefixLength = strlen(prefix);
    nextPrefix = calloc(prefixLength + 20 + 1, sizeof(char));//strlen(prefixSuffix) = 20
    memcpy(nextPrefix, prefix, strlen(prefix));
    memcpy(nextPrefix + strlen(prefix), prefixSuffix, 20);
    NIMEX_displayDataSource(nextPrefix, channel->dataSource);
    free(nextPrefix);
    //mexPrintf("%schannelDefinition->dataBuffer: @%p\n", prefix, channel->datSource);//TO033007C
    //mexPrintf("%schannelDefinition->dataBufferSize: %d\n", prefix, channel->dataBufferSize);//TO033007D
    
}

/**
 * @brief Prints a NIMEX_DataSource to standard output.
 * @arg <tt>prefix</tt> - An arbitrary prefix, prepended to each line of output. Typically used to control indentation.
 * @arg <tt>task</tt> - A pointer to a NIMEX_DataSource.
 */
void NIMEX_displayDataSource(const char* prefix, NIMEX_DataSource* dataSource)
{
    mexPrintf("%sdataSource: @%p\n", prefix, dataSource);
    if (dataSource != NULL)
    {
        mexPrintf("%sdataSource->name: \"%s\" (@%p)\n", prefix, dataSource->name, dataSource->name);
        mexPrintf("%sdataSource->dataSourceType: %s\n", prefix, NIMEX_Constant_2_string(dataSource->dataSourceType));
        mexPrintf("%sdataSource->callback: @%p\n", prefix, dataSource->callback, dataSource->callback);
        mexPrintf("%sdataSource->dataBufferSize: %d\n", prefix, dataSource->dataBufferSize);
        mexPrintf("%sdataSource->dataBuffer: @%p\n", prefix, dataSource->dataBuffer);
    }
    
    return;
}

/**
 * @brief Wraps a variable into a Matlab compatible format.
 * @arg <tt>val</tt> - A C-level variable, to be wrapped for passing into Matlab.
 *
 * @return A newly initialized mxArray, containing the specified value.
 */
mxArray* NIMEX_uint32_To_mxArray(uInt32 val)
{
    mxArray* matlabArray;
    uInt32* data;
    
    matlabArray = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL);
    data = (uInt32 *)mxGetData(matlabArray);
    *data = val;
    
    return matlabArray;
}

/**
 * @brief Wraps a variable into a Matlab compatible format.
 * @arg <tt>val</tt> - A C-level variable, to be wrapped for passing into Matlab.
 *
 * @return A newly initialized mxArray, containing the specified value.
 */
mxArray* NIMEX_int32_To_mxArray(int32 val)
{
    mxArray* matlabArray;
    int32* data;
    
    matlabArray = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL);
    data = (int32 *)mxGetData(matlabArray);
    *data = val;
    
    return matlabArray;
}

/**
 * @brief Wraps a variable into a Matlab compatible format.
 * @arg <tt>val</tt> - A C-level variable, to be wrapped for passing into Matlab.
 *
 * @return A newly initialized mxArray, containing the specified value.
 */
mxArray* NIMEX_float64_To_mxArray(float64 val)
{
    mxArray* matlabArray;
    float64* data;
    
    matlabArray = mxCreateNumericMatrix(1, 1, mxDOUBLE_CLASS, mxREAL);
    data = (float64 *)mxGetData(matlabArray);
    *data = val;
    
    return matlabArray;
}

/**
 * @brief Wraps a variable into a Matlab compatible format.
 * @arg <tt>val</tt> - A C-level variable, to be wrapped for passing into Matlab.
 *
 * @return A newly initialized mxArray, containing the specified value.
 */
mxArray* NIMEX_uint64_To_mxArray(uInt64 val)
{
    mxArray* matlabArray;
    uInt64* data;
    
    matlabArray = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
    data = (uInt64 *)mxGetData(matlabArray);
    *data = val;
    
    return matlabArray;
}

/**
 * @brief Acquires the task's mutex.
 * This mutex MUST be released to prevent deadlocks. No other threads will be able to entered protected code until NIMEX_releaseLock is called.
 * @arg <tt>taskDefinition</tt> - The task whose mutex to acquire.
 * @arg <tt>timeOut</tt> - A maximum wait timeout, in seconds. If the mutex is not acquired in time, the acquire fails.
 *
 * @return 1 if the mutex is acquired, 0 otherwise (ie. if there is a timeout).
 *
 * @callgraph @callergraph
 */
//TO012407B
int NIMEX_acquireLock(NIMEX_TaskDefinition* taskDefinition, uInt32 timeOut)
{
    /*DWORD tickCount = 0;//TO101707A
    tickCount = GetTickCount();//TO101707A
    //TO101707A
    while (g_atomic_int_get(&(taskDefinition->atomic)) != 0)
    {
        //Implement a time out.
        if ((GetTickCount() - tickCount) >= timeOut)
            return 0;//FALSE
        
        NIMEX_MACRO_VERBOSE_4("NIMEX_Utilities/NIMEX_acquireLock: Busy-wait spinning...\n");
        SwitchToThread();//Yield processor.
    }
    timeOut = timeOut - (GetTickCount() - tickCount);//Shorten the timeout to account for any time spent in the loop.*/

    if (taskDefinition->mutex == NULL)
    {
        NIMEX_MACRO_VERBOSE_3("NIMEX_Utilities/NIMEX_acquireLock: Creating new mutex...\n")
        #ifdef NIMEX_COMPILER_CL
            //taskDefinition->mutex = NIMEX_Persistent_Calloc(1, sizeof(HANDLE));
            //taskDefinition->mutex = CreateMutex(NULL, FALSE, "Global\\NIMEX_MUTEX");
        taskDefinition->mutex = (CRITICAL_SECTION *)NIMEX_Persistent_Calloc(1, sizeof(CRITICAL_SECTION));
        InitializeCriticalSection(taskDefinition->mutex);
        #else
            #error "Mutex not defined for compilers other than CL."
        #endif
    }
    #ifdef NIMEX_COMPILER_CL
    NIMEX_MACRO_VERBOSE_3("NIMEX_Utilities/NIMEX_acquireLock: EnterCriticalSection(@%p)\n", taskDefinition->mutex)
    EnterCriticalSection(taskDefinition->mutex);
//mexPrintf("NIMEX_Utilities: EnterCriticalSection(@%p)\n", (uInt32)taskDefinition->mutex);
    return 1;
    //NIMEX_MACRO_VERBOSE_3("NIMEX_Utilities: NIMEX_acquireLock - WaitForSingleObject(@%p, %d)\n", (uInt32)taskDefinition->mutex, timeOut)
    //dwWaitResult = WaitForSingleObject(taskDefinition->mutex, (DWORD)timeOut);
    //NIMEX_MACRO_VERBOSE_3("NIMEX_Utilities: NIMEX_acquireLock - WaitForSingleObject(@%p, %d) = %d\n", (uInt32)taskDefinition->mutex, timeOut, (int32)dwWaitResult)
    //switch (dwWaitResult)
    //{
    //    case WAIT_OBJECT_0:
    //        return 1;//TRUE
    //        
    //    case WAIT_TIMEOUT:
    //        return 0;//FALSE
    //        
    //    case WAIT_ABANDONED:
    //        mexPrintf("NIMEX_Utilities/NIMEX_acquireLock Warning: WAIT_ABANDONED @%p"
    //                  "                                           Found abandoned mutex.\n"
    //                  "                                           An error has likely occurred after which resources were not properly released.\n"
    //                  "                                           Inconsistent states are possible.\n", taskDefinition->mutex);
    //        return 1;//TRUE
    //        
    //    default:
    //        return 0;//FALSE
    //}
    #else
        #error  "Mutex not defined for compilers other than CL."
    #endif
}

/*
//TO101707A: Use a busy-wait to block the Windows message pump from entering critical code while waiting for blocking calls in NIDAQmx to return. -- Tim O'Connor 10/17/07
int NIMEX_acquireLockExclusively(NIMEX_TaskDefinition* taskDefinition, uInt32 timeOut)
{
    g_atomic_int_inc(&(taskDefinition->atomic));
    if (taskDefinition->mutex == NULL)
    {
        NIMEX_MACRO_VERBOSE_3("NIMEX_Utilities/NIMEX_acquireLockExclusively: NIMEX_acquireLock - Creating new mutex...\n")
        #ifdef NIMEX_COMPILER_CL
        taskDefinition->mutex = (CRITICAL_SECTION *)NIMEX_Persistent_Calloc(1, sizeof(CRITICAL_SECTION));
        InitializeCriticalSection(taskDefinition->mutex);
        #else
            #error "Mutex not defined for compilers other than CL."
        #endif
    }
    #ifdef NIMEX_COMPILER_CL
    NIMEX_MACRO_VERBOSE_3("NIMEX_Utilities/NIMEX_acquireLockExclusively: EnterCriticalSection(@%p)\n", taskDefinition->mutex)
    EnterCriticalSection(taskDefinition->mutex);
    return 1;
    #else
        #error  "Mutex not yet defined for compilers other than CL."
    #endif
}

//TO101707A: Use a busy-wait to block the Windows message pump from entering critical code while waiting for blocking calls in NIDAQmx to return. -- Tim O'Connor 10/17/07
int NIMEX_releaseLockExclusively(NIMEX_TaskDefinition* taskDefinition)
{
    g_atomic_int_dec_and_test(&(taskDefinition->atomic));
    if (taskDefinition->mutex == NULL)
    {
        NIMEX_MACRO_VERBOSE_3("NIMEX_Utilities/NIMEX_releaseLockExclusively: NIMEX_releaseLock - Creating new mutex...\n")
        #ifdef NIMEX_COMPILER_CL
        taskDefinition->mutex = (CRITICAL_SECTION *)NIMEX_Persistent_Calloc(1, sizeof(CRITICAL_SECTION));
        InitializeCriticalSection(taskDefinition->mutex);
        #else
            #error "Mutex not defined for compilers other than CL."
        #endif
    }
    #ifdef NIMEX_COMPILER_CL
    NIMEX_MACRO_VERBOSE_3("NIMEX_Utilities/NIMEX_releaseLockExclusively: LeaveCriticalSection(@%p)\n", taskDefinition->mutex)
    LeaveCriticalSection(taskDefinition->mutex);
    return 1;
    #else
        #error  "Mutex not defined for compilers other than CL."
    #endif
}*/

/**
 * @brief Releases the task's mutex.
 * This function must not be called if the current thread does not own the mutex. Each call of this function must be paired with a call to NIMEX_acquireLock.
 * @arg <tt>taskDefinition</tt> - The task whose mutex to release.
 *
 * @return 1 if the mutex is released successfully, 0 otherwise.
 *
 * @callgraph @callergraph
 */
//TO012407B
int NIMEX_releaseLock(NIMEX_TaskDefinition* taskDefinition)
{
    if (taskDefinition->mutex == NULL)
    {
        NIMEX_MACRO_VERBOSE_3("NIMEX_Utilities/NIMEX_releaseLock: NIMEX_releaseLock - Creating new mutex...\n")
        #ifdef NIMEX_COMPILER_CL
            //taskDefinition->mutex = NIMEX_Persistent_Calloc(1, sizeof(HANDLE));
            //taskDefinition->mutex = CreateMutex(NULL, FALSE, "Global\\NIMEX_MUTEX");
        taskDefinition->mutex = (CRITICAL_SECTION *)NIMEX_Persistent_Calloc(1, sizeof(CRITICAL_SECTION));
        InitializeCriticalSection(taskDefinition->mutex);
        #else
            #error "Mutex not defined for compilers other than CL."
        #endif
    }
    #ifdef NIMEX_COMPILER_CL
    NIMEX_MACRO_VERBOSE_3("NIMEX_Utilities/NIMEX_releaseLock: LeaveCriticalSection(@%p)\n", taskDefinition->mutex)
    LeaveCriticalSection(taskDefinition->mutex);
    return 1;
    //NIMEX_MACRO_VERBOSE_3("NIMEX_Utilities: NIMEX_releaseLock - ReleaseMutex(@%p)\n", (uInt32)taskDefinition->mutex)
    //if (ReleaseMutex(taskDefinition->mutex))
    //    return 1;//TRUE
    //else
    //    return 0;//FALSE
    #else
        #error  "Mutex not defined for compilers other than CL."
    #endif
}

/**
 * @brief Processes a NIMEX_Callback for the specified task.
 * This function should never be called directly, except for the callback management and event handling code.
 * @arg <tt>taskDefinition</tt> - The task whose mutex to release.
 *
 * @return 1 if the mutex is released successfully, 0 otherwise.
 *
 * @callgraph @callergraph
 */
int32 NIMEX_implementCallback(NIMEX_TaskDefinition* taskDefinition, NIMEX_Callback* callback)
{
	int32                       error = 0;
    mxArray**                   plhs = (mxArray**)NULL;
    mxArray**                   prhs = (mxArray**)NULL;
    int                         nargs = 0;
    int                         i = 0;//TO071107A

    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);//TO012407B
    NIMEX_MACRO_VERBOSE_3("NIMEX_Utilities/NIMEX_implementCallback: Calling \"%s\"...\n", callback->name)
//mexPrintf("DEBUG: NIMEX_Utilities/NIMEX_implementCallback: Calling \"%s\"...\n", callback->name);
    nargs = mxGetNumberOfElements(callback->callbackToMatlab);
    prhs = mxCalloc(nargs, sizeof(mxArray*));
    for (i = 0; i < nargs; i++)
    {
        prhs[i] = mxGetCell(callback->callbackToMatlab, i);
        if (prhs[i] == 0)
        {
            mexPrintf("NIMEX_Utilities/NIMEX_implementCallback: Error - Failed to marshall callback argument from cell array element %d.\n", i);
            NIMEX_releaseLock(taskDefinition);//TO012407B
            return -1;
        }
    }
    error = mexCallMATLAB(0, plhs, nargs, prhs, "feval");

    mxFree(prhs);

    if (error)
        mexPrintf("NIMEX_Utilities/NIMEX_implementCallback: Error - Failed to execute callback for \"%s\".\n", callback->name);

    NIMEX_releaseLock(taskDefinition);//TO012407B

    return error;
}

/**
 * @brief Counts the enabled channels attached to a task.
 * @arg <tt>taskDefinition</tt> - A valid NIMEX task.
 *
 * @return The number of channels in the task for which the enable field evaluates true.
 *
 * @callergraph
 */
//TO073007A
int32 NIMEX_enabledChannelCount(NIMEX_TaskDefinition* taskDefinition)
{
    NIMEX_ChannelList* channels = (NIMEX_ChannelList *)NULL;
    NIMEX_ChannelDefinition* channel = (NIMEX_ChannelDefinition *)NULL;
    int32  count = 0;
NIMEX_MACRO_VERBOSE("A NIMEX_enabledChannelCount\n")
    channels = NIMEX_ChannelList_first(taskDefinition->channels);
NIMEX_MACRO_VERBOSE("B NIMEX_enabledChannelCount\n")    
    while (channels != NULL)
    {
NIMEX_MACRO_VERBOSE("C NIMEX_enabledChannelCount: channels = @%p\n", channels)
        channel = NIMEX_ChannelList_value(channels);
NIMEX_MACRO_VERBOSE("D NIMEX_enabledChannelCount: channel = @%p\n", channel)
        channels = NIMEX_ChannelList_next(channels);
NIMEX_MACRO_VERBOSE("E NIMEX_enabledChannelCount: channels = @%p\n", channels)
        if (channel != NULL)
        {
NIMEX_MACRO_VERBOSE("F NIMEX_enabledChannelCount: channel->enable = %d\n", channel->enable)
            if (channel->enable)
                count++;
        }
    }
    
    return count;
}

/**
 * @brief This is a convenience macro for implementing array copies that change data types.
 */
//The same algorithm is used across all types. It just enforces the proper coersion by the compiler.
#define NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(srcType, destType) \
    int i = 0;\
    for (i = offset; i < (offset + len); i++)\
        dest[offset + i] = (destType)src[offset + i];
//        ((destType)*(dest + (i * sizeof(destType)))) = (destType)*(src + (i * sizeof(srcType)));\

///@see NIMEX_MACRO_UTILITIES_ARRAY_CONVERT
void NIMEX_float64_2_int32(const float64* src, int32* dest, int offset, int len)
{
    NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(float64, int32)
    return;
}

///@see NIMEX_MACRO_UTILITIES_ARRAY_CONVERT
void NIMEX_float64_2_uInt32(const float64* src, uInt32* dest, int offset, int len)
{
    NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(float64, uInt32)
    return;
}

///@see NIMEX_MACRO_UTILITIES_ARRAY_CONVERT
void NIMEX_float64_2_uInt64(const float64* src, uInt64* dest, int offset, int len)
{
    NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(float64, uInt64)
    return;
}

///@see NIMEX_MACRO_UTILITIES_ARRAY_CONVERT
void NIMEX_int32_2_float64(const int32* src, float64* dest, int offset, int len)
{
    NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(int32, float64)
    return;
}

///@see NIMEX_MACRO_UTILITIES_ARRAY_CONVERT
void NIMEX_int32_2_uInt32(const int32* src, uInt32* dest, int offset, int len)
{
    NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(int32, uInt32)
    return;
}

///@see NIMEX_MACRO_UTILITIES_ARRAY_CONVERT
void NIMEX_int32_2_uInt64(const int32* src, uInt64* dest, int offset, int len)
{
    NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(int32, uInt64)
    return;
}

///@see NIMEX_MACRO_UTILITIES_ARRAY_CONVERT
void NIMEX_uInt32_2_int32(const uInt32* src, int32* dest, int offset, int len)
{
    NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(uInt32, int32)
    return;
}

///@see NIMEX_MACRO_UTILITIES_ARRAY_CONVERT
void NIMEX_uInt32_2_float64(const uInt32* src, float64* dest, int offset, int len)
{
    NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(uInt32, float64)
    return;
}

///@see NIMEX_MACRO_UTILITIES_ARRAY_CONVERT
void NIMEX_uInt32_2_uInt64(const uInt32* src, uInt64* dest, int offset, int len)
{
    NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(uInt32, uInt64)
    return;
}

///@see NIMEX_MACRO_UTILITIES_ARRAY_CONVERT
void NIMEX_int64_2_int32(const int64* src, int32* dest, int offset, int len)
{
    NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(int64, int32)
    return;
}

///@see NIMEX_MACRO_UTILITIES_ARRAY_CONVERT
void NIMEX_int64_2_float64(const int64* src, float64* dest, int offset, int len)
{
    NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(int64, float64)
    return;
}

///@see NIMEX_MACRO_UTILITIES_ARRAY_CONVERT
void NIMEX_int64_2_uInt32(const int64* src, uInt32* dest, int offset, int len)
{
    NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(int64, uInt32)
    return;
}

///@see NIMEX_MACRO_UTILITIES_ARRAY_CONVERT
void NIMEX_uInt64_2_int32(const uInt64* src, int32* dest, int offset, int len)
{
    //NIMEX_int64_2_int32((int64 *)src, dest, offset, len);
    NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(uInt64, int32);
    return;
}

///@see NIMEX_MACRO_UTILITIES_ARRAY_CONVERT
void NIMEX_uInt64_2_float64(const uInt64* src, float64* dest, int offset, int len)
{
    //NIMEX_int64_2_float64((int64 *)src, dest, offset, len);
    NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(uInt64, float64);
    return;
}

///@see NIMEX_MACRO_UTILITIES_ARRAY_CONVERT
void NIMEX_uInt64_2_uInt32(const uInt64* src, uInt32* dest, int offset, int len)
{
    //NIMEX_int64_2_uInt32((int64 *)src, dest, offset, len);
    NIMEX_MACRO_UTILITIES_ARRAY_CONVERT(uInt64, uInt32);
    return;
}

/**
 * @brief Copies an mxArray's data into a newly allocated int32 array.
 * @arg <tt>arr</tt> - A numeric Matlab array.
 *
 * @return A newly allocated int32 array.
 */
int32* NIMEX_mxArray_2_int32(const mxArray* arr)
{
    size_t numElements = 0;
    int32* val = NULL;

    numElements = mxGetNumberOfElements(arr);

    //Only limited conversions (the most common ones) are supported, for now.
    switch (mxGetClassID(arr))
    {
        case mxDOUBLE_CLASS:
            val = (int32 *)NIMEX_Persistent_Calloc(sizeof(int32), numElements);
            NIMEX_float64_2_int32((float64 *)mxGetData(arr), val, 0, numElements);
            break;

        case mxINT32_CLASS:
            val = (int32 *)NIMEX_memcpy(mxGetData(arr), numElements * sizeof(int32));
            break;

        case mxUINT32_CLASS:
            val = (int32 *)NIMEX_Persistent_Calloc(sizeof(int32), numElements);
            NIMEX_uInt32_2_int32((unsigned int *)mxGetData(arr), val, 0, numElements);
            break;

        case mxUINT64_CLASS:
            val = (int32 *)NIMEX_Persistent_Calloc(sizeof(int32), numElements);
            NIMEX_uInt64_2_int32((unsigned long long int *)mxGetData(arr), val, 0, numElements);
            break;

        default:
            mexPrintf("Warning - Unrecognized/unsupported type for conversion to int32: %s\n", NIMEX_mxConstant_2_string(mxGetClassID(arr)));
            mexErrMsgTxt("Failed to convert data type.");
            break;
    }

    return val;
}

/**
 * @brief Copies an mxArray's data into a newly allocated float64 array.
 * @arg <tt>arr</tt> - A numeric Matlab array.
 *
 * @return A newly allocated float64 array.
 */
float64* NIMEX_mxArray_2_float64(const mxArray* arr)
{
    size_t numElements = 0;
    float64* val = NULL;

    numElements = mxGetNumberOfElements(arr);

    //Only limited conversions (the most common ones) are supported, for now.
    switch (mxGetClassID(arr))
    {
        case mxDOUBLE_CLASS:
            val = (float64 *)NIMEX_memcpy(mxGetData(arr), numElements * sizeof(float64));
            break;

        case mxINT32_CLASS:
            val = (float64 *)NIMEX_Persistent_Calloc(sizeof(float64), numElements);
            NIMEX_int32_2_float64((int *)mxGetData(arr), val, 0, numElements);
            break;

        case mxUINT32_CLASS:
            val = (float64 *)NIMEX_Persistent_Calloc(sizeof(float64), numElements);
            NIMEX_uInt32_2_float64((unsigned int *)mxGetData(arr), val, 0, numElements);
            break;

        case mxUINT64_CLASS:
            val = (float64 *)NIMEX_Persistent_Calloc(sizeof(float64), numElements);
            NIMEX_uInt64_2_float64((unsigned long long int *)mxGetData(arr), val, 0, numElements);
            break;

        default:
            mexPrintf("Warning - Unrecognized/unsupported type for conversion to float64: %s\n", NIMEX_mxConstant_2_string(mxGetClassID(arr)));
            mexErrMsgTxt("Failed to convert data type.");
            break;
    }
    
    return val;
}

/**
 * @brief Copies an mxArray's data into a newly allocated uInt32 array.
 * @arg <tt>arr</tt> - A numeric Matlab array.
 *
 * @return A newly allocated uInt32 array.
 */
uInt32* NIMEX_mxArray_2_uInt32(const mxArray* arr)
{
    size_t numElements = 0;
    uInt32* val = NULL;

    numElements = mxGetNumberOfElements(arr);

    //Only limited conversions (the most common ones) are supported, for now.
    switch (mxGetClassID(arr))
    {
        case mxDOUBLE_CLASS:
            val = (uInt32 *)NIMEX_Persistent_Calloc(sizeof(uInt32), numElements);
            NIMEX_float64_2_uInt32((float64 *)mxGetData(arr), val, 0, numElements);
            break;

        case mxUINT32_CLASS:
            val = (uInt32 *)NIMEX_memcpy(mxGetData(arr), numElements * sizeof(uInt32));
            break;

        case mxINT32_CLASS:
            val = (uInt32 *)NIMEX_Persistent_Calloc(sizeof(uInt32), numElements);
            NIMEX_int32_2_uInt32((unsigned int *)mxGetData(arr), val, 0, numElements);
            break;

        case mxUINT64_CLASS:
            val = (uInt32 *)NIMEX_Persistent_Calloc(sizeof(uInt32), numElements);
            NIMEX_uInt64_2_uInt32((unsigned long long int *)mxGetData(arr), val, 0, numElements);
            break;

        default:
            mexPrintf("Warning - Unrecognized/unsupported type for conversion to uInt32: %s\n", NIMEX_mxConstant_2_string(mxGetClassID(arr)));
            mexErrMsgTxt("Failed to convert data type.");
            break;
    }
    
    return val;
}

/**
 * @brief Copies an mxArray's data into a newly allocated uInt64 array.
 * @arg <tt>arr</tt> - A numeric Matlab array.
 *
 * @return A newly allocated uInt64 array.
 */
uInt64* NIMEX_mxArray_2_uInt64(const mxArray* arr)
{
    size_t numElements = 0;
    uInt64* val = NULL;

    numElements = mxGetNumberOfElements(arr);

    //Only limited conversions (the most common ones) are supported, for now.
    switch (mxGetClassID(arr))
    {
        case mxDOUBLE_CLASS:
            val = (uInt64 *)NIMEX_Persistent_Calloc(sizeof(uInt64), numElements);
            NIMEX_float64_2_uInt64((float64 *)mxGetData(arr), val, 0, numElements);
            break;

        case mxINT32_CLASS:
            val = (uInt64 *)NIMEX_Persistent_Calloc(sizeof(uInt64), numElements);
            NIMEX_int32_2_uInt64((int *)mxGetData(arr), val, 0, numElements);
            break;

        case mxUINT32_CLASS:
            val = (uInt64 *)NIMEX_Persistent_Calloc(sizeof(uInt64), numElements);
            NIMEX_uInt32_2_uInt64((unsigned int *)mxGetData(arr), val, 0, numElements);
            break;

        case mxUINT64_CLASS:
            val = (uInt64 *)NIMEX_memcpy(mxGetData(arr), numElements * sizeof(uInt64));
            break;

        default:
            mexPrintf("Warning - Unrecognized/unsupported type for conversion to int32: %s\n", NIMEX_mxConstant_2_string(mxGetClassID(arr)));
            mexErrMsgTxt("Failed to convert data type.");
            break;
    }
    
    return val;
}

/**
 * @brief This is a convenience macro for converting an mxArray value into a specified type.
 */
//The same algorithm is used across all types. The trick is to make sure you treat the mxGetData pointer as the right type.
#define NIMEX_MACRO_UTILITIES_TYPECAST_SWITCHBLOCK(mxArray, targetType) \
    size_t numElements = 0;\
    targetType val = 0;\
    numElements = mxGetNumberOfElements(arr);\
    if (numElements > 1)\
        mexPrintf("Warning - Scalar type conversion requested on a non-scalar value. Only the first element will be converted.\n");\
    else if (numElements < 1)\
    {\
        mexPrintf("Warning - Scalar type conversion requested on an empty or invalid mxArray: numElements = %d", numElements);\
        mexErrMsgTxt("Failed to convert data type.");\
    }\
   switch (mxGetClassID(arr))\
    {\
        case mxDOUBLE_CLASS:\
            val = (targetType)*((float64 *)mxGetData(mxArray));\
            break;\
        case mxSINGLE_CLASS:\
            val = (targetType)*((float *)mxGetData(mxArray));\
            break;\
        case mxCHAR_CLASS:\
            val = (targetType)*((wchar_t *)mxGetData(mxArray));\
            break;\
        case mxLOGICAL_CLASS:\
            val = (targetType)*((bool *)mxGetData(mxArray));\
            break;\
        case mxINT8_CLASS:\
            val = (targetType)*((char *)mxGetData(mxArray));\
            break;\
        case mxUINT8_CLASS:\
            val = (targetType)*((unsigned char *)mxGetData(mxArray));\
            break;\
        case mxINT16_CLASS:\
            val = (targetType)*((short int *)mxGetData(mxArray));\
            break;\
        case mxUINT16_CLASS:\
            val = (targetType)*((unsigned short int *)mxGetData(mxArray));\
            break;\
        case mxINT32_CLASS:\
            val = (targetType)*((int *)mxGetData(mxArray));\
            break;\
        case mxUINT32_CLASS:\
            val = (targetType)*((unsigned int *)mxGetData(mxArray));\
            break;\
        case mxINT64_CLASS:\
            val = (targetType)*((long long int *)mxGetData(mxArray));\
            break;\
        case mxUINT64_CLASS:\
            val = (targetType)*((unsigned long long int *)mxGetData(mxArray));\
            break;\
        default:\
            mexPrintf("Warning - Unrecognized/unsupported type for conversion to targetType: %s\n", NIMEX_mxConstant_2_string(mxGetClassID(mxArray)));\
            mexErrMsgTxt("Failed to convert data type.");\
            break;\
    }\
    return val;

///@see NIMEX_MACRO_UTILITIES_TYPECAST_SWITCHBLOCK
int32 NIMEX_scalar_mxArray_2_int32(const mxArray* arr)
{
    NIMEX_MACRO_UTILITIES_TYPECAST_SWITCHBLOCK(arr, int32)
}

///@see NIMEX_MACRO_UTILITIES_TYPECAST_SWITCHBLOCK
float64 NIMEX_scalar_mxArray_2_float64(const mxArray* arr)
{
    NIMEX_MACRO_UTILITIES_TYPECAST_SWITCHBLOCK(arr, float64)
}

///@see NIMEX_MACRO_UTILITIES_TYPECAST_SWITCHBLOCK
uInt32 NIMEX_scalar_mxArray_2_uInt32(const mxArray* arr)
{
    NIMEX_MACRO_UTILITIES_TYPECAST_SWITCHBLOCK(arr, uInt32)
}

///@see NIMEX_MACRO_UTILITIES_TYPECAST_SWITCHBLOCK
uInt64 NIMEX_scalar_mxArray_2_uInt64(const mxArray* arr)
{
    NIMEX_MACRO_UTILITIES_TYPECAST_SWITCHBLOCK(arr, uInt64)
}

/**
 * @brief Iterate over enable channels and execute the callbacks in their dataSources, then write the updated buffers to the task.
 * @arg <tt>taskDefinition</tt> - A fully configured NIMEX_TaskDefinition, with a valid taskHandle field.
 *
 * @pre taskDefinition->taskHandle is valid.
 * @post The output buffers are ready for the task to be started.
 *
 * @callgraph @callergraph
 */
//TO102007B - Broke this out into a separate function, because it's needed for setting up a retriggered acquisition. -- Tim O'Connor 10/20/07
//Return 0 on error, otherwise it's the number of samples written.
uInt32 NIMEX_updateBuffersByCallbacksAndWriteToTask(NIMEX_TaskDefinition* taskDefinition)
{
	int32                       error = 0;
	char                        errBuff[2048] = {'\0'};
    uInt32                      written;//TO100907A
    NIMEX_ChannelDefinition*    channelDefinition = (NIMEX_ChannelDefinition *)NULL;
    NIMEX_ChannelList*          channelDefinitionList = (NIMEX_ChannelList *)NULL;
    void*                       concatenatedDataBuffer = (void *)NULL;//TO033007E
    uInt32                      concatenatedDataBufferOffset = 0;//TO033007E
    int32                       concatenatedDataBufferType = NIMEX_UNINITIALIZED;//TO033007E
    void*                       concatenatedDataBufferOffsetPointer = (void *)NULL;//TO033007E
    uInt64                      sampsPerChanToAcquire = 0;//TO102407A
    int                         i;//TO121607A

    NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: taskDefinition=@%p\n", taskDefinition)

    if (taskDefinition->repeatOutput > 1)
        sampsPerChanToAcquire = taskDefinition->sampsPerChanToAcquire * taskDefinition->repeatOutput;
    else
        sampsPerChanToAcquire = taskDefinition->sampsPerChanToAcquire;
    NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask:\n\tsampsPerChanToAcquire: %llu\n\trepeatOutput: %d\n\ttaskDefinition->sampsPerChanToAcquire: %llu\n\n", 
       sampsPerChanToAcquire, taskDefinition->repeatOutput, taskDefinition->sampsPerChanToAcquire)

    channelDefinitionList = NIMEX_ChannelList_first(taskDefinition->channels);
    while (channelDefinitionList != NULL)
    {
        channelDefinition = NIMEX_ChannelList_value(channelDefinitionList);//TO010607B //TO061207A
        channelDefinitionList = NIMEX_ChannelList_next(channelDefinitionList);//TO061207A

        if (channelDefinition != NULL)
        {
            NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: channel '%s'\n", channelDefinition->physicalChannel)
            #ifdef NIMEX_VERBOSE_2
                NIMEX_displayChannelDefinition("\t\t", channelDefinition);
            #endif

            //TO073070A
            if ( (channelDefinition->enable) && 
                 ( (channelDefinition->channelType == NIMEX_ANALOG_OUTPUT) || (channelDefinition->channelType == NIMEX_DIGITAL_OUTPUT) ) )
            {
                concatenatedDataBufferType = channelDefinition->channelType;
                if (channelDefinition->dataSource != NULL)
                {
                    //TO080107B
                    if (channelDefinition->dataSource->dataSourceType & NIMEX_DATASOURCE_TYPE_CALLBACK_BITMASK)
                    {
                        NIMEX_DataSource_updateBufferByCallback(taskDefinition, channelDefinition->dataSource);
/*if (channelDefinition->channelType == NIMEX_ANALOG_OUTPUT)
{
for (i = 0; i < channelDefinition->dataSource->dataBufferSize; i++)
{
 if (((float64*)channelDefinition->dataSource->dataBuffer)[i] > mx)
    mx = ((float64*)channelDefinition->dataSource->dataBuffer)[i];
 else if (((float64*)channelDefinition->dataSource->dataBuffer)[i] < mn)
    mn = ((float64*)channelDefinition->dataSource->dataBuffer)[i];
}
NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask - Data range ('%s' == '%s'): [%4.4f -> %4.4f]\n", channelDefinition->mnemonicName, channelDefinition->physicalChannel, mn, mx)
}*/
                    }
                    NIMEX_MACRO_VERBOSE_2("NIMEX_updateBuffersByCallbacksAndWriteToTask - updated buffer size:\n\tsampsPerChanToAcquire = %llu\n\tchannelDefinition(%s)->dataSource->dataBufferSize = %llu\n", sampsPerChanToAcquire, channelDefinition->physicalChannel, channelDefinition->dataSource->dataBufferSize)
                    if (sampsPerChanToAcquire > (uInt64)channelDefinition->dataSource->dataBufferSize) //TO100907A
                    {
                        //TO121707C - This statement will come up a lot, but is generally not harmful, for now...
                        //mexPrintf("NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Warning - sampsPerChanToAcquire (%llu) > channelDefinition->dataSource->bufferSize (%llu) for channel \"%s\".\n",
                        //    sampsPerChanToAcquire, channelDefinition->dataSource->dataBufferSize, channelDefinition->physicalChannel);//TO100907A
                        if (sampsPerChanToAcquire % channelDefinition->dataSource->dataBufferSize == 0)
                        {
                            //TO121707C - This statement will come up a lot, but is generally not harmful, for now...
                            //mexPrintf("NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Replicating data array to fill required number of samples.\n");
                        }
                        else
                        {
                            mxFree(concatenatedDataBuffer); //JL011808A: Change free to mxfree because concatenatedDataBuffer is allocated using mxCalloc
                            //TO083108B - Print a warning here, so it's clear what's going on. -- Tim O'Connor 8/31/08
                            mexPrintf("NIMEX_updateBuffersByCallbacksAndWriteToTask: Not enough samples for channel '%s' available to match sampsPerChanToAcquire. %lu available, %llu required.\n", 
                                      channelDefinition->physicalChannel, channelDefinition->dataSource->dataBufferSize, taskDefinition->sampsPerChanToAcquire);
                            return 0;
                        }
                    }
                    switch (concatenatedDataBufferType)
                    {
                        case NIMEX_ANALOG_OUTPUT:
                            //TO033007E
                            //Because this append is done on a per-channel basis, in the same order that the channels are added, it automatically groups by channel.
                            if (concatenatedDataBuffer == NULL)
                            {
                                //TO073007A
                                //For some reason the following NIMEX_MACRO_VERBOSE_2 statement is killing the program. No idea why at the moment. -- Tim O'Connor 8/31/08
                                //NIMEX_MACRO_VERBOSE_2("  NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Creating new concatenatedDataBuffer of size %llu samples (of type float64) for channel @%p->\"%s\".\n", (uInt64)(((uInt64)NIMEX_enabledChannelCount(taskDefinition)) * sampsPerChanToAcquire), channelDefinition, channelDefinition->physicalChannel) //TO100907A
                                concatenatedDataBuffer = (float64 *)mxCalloc(NIMEX_enabledChannelCount(taskDefinition) * (int32)sampsPerChanToAcquire, sizeof(float64));//TO100907A
                                NIMEX_MACRO_VERBOSE_2("  NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Creating new concatenatedDataBuffer @%p\n", concatenatedDataBuffer)
//mexPrintf("Allocating buffer of %d (channelCount) * %d (sampsPerChanToAcquire) * %d (sizeof(float64)) bytes...\n", NIMEX_enabledChannelCount(taskDefinition), (int32)sampsPerChanToAcquire, sizeof(float64));
                                concatenatedDataBufferType = NIMEX_ANALOG_OUTPUT;
                            }
                            else
                            {
                                NIMEX_MACRO_VERBOSE_2("  NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Appending %llu output samples for channel @%p->\"%s\"\n", sampsPerChanToAcquire, channelDefinition, channelDefinition->physicalChannel) //TO100907A
                                concatenatedDataBufferOffset += (uInt32)sampsPerChanToAcquire;//TO100907A
                            }
                            concatenatedDataBufferOffsetPointer = (float64 *)((uInt32)concatenatedDataBuffer + ((uInt32)(sizeof(float64) * concatenatedDataBufferOffset)));
                            NIMEX_MACRO_VERBOSE_2("  NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Copying %d bytes from @%p to @%p for \"%s\"...\n", (int)(sampsPerChanToAcquire * sizeof(float64)), channelDefinition->dataSource->dataBuffer, concatenatedDataBufferOffsetPointer, channelDefinition->physicalChannel) //TO100907A

                            //TO121607A
                            if (sampsPerChanToAcquire > (uInt64)channelDefinition->dataSource->dataBufferSize)
                            {
                                for (i = 0; i < (int)(sampsPerChanToAcquire / channelDefinition->dataSource->dataBufferSize); i++)
                                {
                                    memcpy((float64 *)concatenatedDataBufferOffsetPointer, channelDefinition->dataSource->dataBuffer, (uInt64)channelDefinition->dataSource->dataBufferSize * sizeof(float64));//TO100907A
                                    concatenatedDataBufferOffsetPointer = (float64 *)((uInt32)concatenatedDataBufferOffsetPointer + ((uInt32)(sizeof(float64) * channelDefinition->dataSource->dataBufferSize)));
                                }
                            }
                            else
                                memcpy((float64 *)concatenatedDataBufferOffsetPointer, channelDefinition->dataSource->dataBuffer, (uInt64)sampsPerChanToAcquire * sizeof(float64));//TO100907A
                            break;

                        case NIMEX_DIGITAL_OUTPUT:
                            //TO033007E
                            //Because this append is done on a per-channel basis, in the same order that the channels are added, it automatically groups by channel.
                            if (concatenatedDataBuffer == NULL)
                            {
                                //TO073007A
                                //For some reason the following NIMEX_MACRO_VERBOSE_2 statement is killing the program. No idea why at the moment. -- Tim O'Connor 8/31/08
                                //NIMEX_MACRO_VERBOSE_2("  NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Creating new concatenatedDataBuffer of size %llu samples (of type uInt32) for channel @%p->\"%s\".\n", (uInt64)(((uInt64)NIMEX_enabledChannelCount(taskDefinition)) * sampsPerChanToAcquire), channelDefinition, channelDefinition->physicalChannel) //TO100907A
                                concatenatedDataBuffer = (uInt32 *)mxCalloc(NIMEX_enabledChannelCount(taskDefinition) * (int32)sampsPerChanToAcquire, sizeof(uInt32));//TO100907A
                                concatenatedDataBufferType = NIMEX_DIGITAL_OUTPUT;
                            }
                            else
                            {
                                NIMEX_MACRO_VERBOSE_2("  NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Appending %llu output samples for channel @%p->\"%s\"\n", sampsPerChanToAcquire, channelDefinition, channelDefinition->physicalChannel) //TO100907A
                                concatenatedDataBufferOffset += (uInt32)sampsPerChanToAcquire;//TO100907A
                            }
                            concatenatedDataBufferOffsetPointer = (uInt32 *)((uInt32)concatenatedDataBuffer + ((uInt32)(sizeof(uInt32) * concatenatedDataBufferOffset)));

                            NIMEX_MACRO_VERBOSE_2("  NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Copying %d bytes from @%p to @%p for \"%s\"...\n", (int)(sampsPerChanToAcquire * sizeof(uInt32)), channelDefinition->dataSource->dataBuffer, concatenatedDataBufferOffsetPointer, channelDefinition->physicalChannel) //TO100907A
                            //TO121607A
                            if (sampsPerChanToAcquire > (uInt64)channelDefinition->dataSource->dataBufferSize)
                            {
                                for (i = 0; i < (int)(sampsPerChanToAcquire / channelDefinition->dataSource->dataBufferSize); i++)
                                {
                                    memcpy((uInt32 *)concatenatedDataBufferOffsetPointer, channelDefinition->dataSource->dataBuffer, (uInt64)channelDefinition->dataSource->dataBufferSize * sizeof(uInt32));//TO100907A
                                    concatenatedDataBufferOffsetPointer = (float64 *)((uInt32)concatenatedDataBufferOffsetPointer + ((uInt32)(sizeof(uInt32) * channelDefinition->dataSource->dataBufferSize)));
                                }
                            }
                            else
                                memcpy((uInt32 *)concatenatedDataBufferOffsetPointer, channelDefinition->dataSource->dataBuffer, (uInt64)sampsPerChanToAcquire * sizeof(uInt32));//TO100907A
                            break;

                        default:
                            mexPrintf("NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Error - Unrecognized buffer type \"%s\".\n", NIMEX_channelType_2_string(concatenatedDataBufferType));
                            free(concatenatedDataBuffer);
                            return 0;
                    }//switch (channelType
                }//if (channelDefinition->dataSource != NULL)
            }
            else if (!( (channelDefinition->channelType == NIMEX_ANALOG_OUTPUT) || (channelDefinition->channelType == NIMEX_DIGITAL_OUTPUT) ))
                return 1;//This is not an output task, so the buffer update's not necessary.
        }//if (channelDefinition != NULL)
    }//while (channelDefinitionList != NULL)

    //TO033007E
    if (concatenatedDataBuffer != NULL)
    {
        NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Attempting to write %llu cached samples to task (@%p->%p)...\n", sampsPerChanToAcquire, taskDefinition, *(taskDefinition->taskHandle)) //TO100907A
        //TO033007A
        if  (concatenatedDataBufferType == NIMEX_ANALOG_OUTPUT)
        {
            #ifndef NIMEX_LAME
            DAQmxWriteAnalogF64(*(taskDefinition->taskHandle), sampsPerChanToAcquire, FALSE, taskDefinition->timeout, DAQmx_Val_GroupByChannel, concatenatedDataBuffer, &written, NULL);//TO100907A
            if (error)
            {
                DAQmxGetExtendedErrorInfo(errBuff, 2048);
                mexPrintf("NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Error writing to analog output task - %s\n", errBuff);
            }
            #else
            NIMEX_MACRO_VERBOSE("  NIMEX_LAME - DAQmxWriteAnalogF64 not executed; written = sampsPerChanToAcquire.\n") //TO100907A
            written = sampsPerChanToAcquire;//TO100907A
            #endif
            NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Wrote %d cached float64 samples to task.\n", written)
        }
        else if (concatenatedDataBufferType == NIMEX_DIGITAL_OUTPUT)
        {
            #ifndef NIMEX_LAME
            error = DAQmxWriteDigitalU32(*(taskDefinition->taskHandle), sampsPerChanToAcquire, FALSE, taskDefinition->timeout, DAQmx_Val_GroupByChannel, concatenatedDataBuffer, &written, NULL);//TO100907A
            if (error)
            {
                DAQmxGetExtendedErrorInfo(errBuff, 2048);
                mexPrintf("NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Error writing to digital output task - %s\n", errBuff);
            }
            #else
            NIMEX_MACRO_VERBOSE("  NIMEX_LAME - DAQmxWriteDigitalU32 not executed; written = sampsPerChanToAcquire.\n") //TO100907A
            written = sampsPerChanToAcquire;//TO100907A
            #endif
            
            NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Wrote %d cached uInt32 samples to task.\n", written)
        }
        else
            mexPrintf("NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Unsupported output channel data type encountered while writing concatenated data buffer: %s\n                 Write operation aborted.\n", NIMEX_channelType_2_string(concatenatedDataBufferType));
    }

    if (concatenatedDataBuffer != NULL)
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_Utilities/NIMEX_updateBuffersByCallbacksAndWriteToTask: Free (via mxFree) concatenatedDataBuffer @%p.\n", concatenatedDataBuffer)
        mxFree(concatenatedDataBuffer);//TO080107C  //JL011808A: Change free to mxfree because concatenatedDataBuffer is allocated using mxCalloc
    }

    return written;
}

#define DAQmxCreateTaskHandleErrChk(functionCall) if (DAQmxFailed(error = (functionCall))) goto CreateTaskHandleError; else

/**
 * @brief Creates the underling NIDAQmx task for the task definition.
 * This is the function that handles all of the "late binding" aspects of NIMEX.
 * Once this is called, making changes to a task configuration requires destroying and recreating the task.
 * @arg <tt>taskDefinition</tt> - A valid and fully configured NIMEX task definition.
 *
 * @pre taskDefinition->taskHandle == NULL
 * @post taskDefinition->taskHandle is a valid NIDAQmx task.
 *
 * @see DAQmxCreateTask @see DAQmxCreateAIVoltageChan @see DAQmxCreateAOVoltageChan @see DAQmxCreateDIChan
 * @see DAQmxCreateDOChan @see DAQmxCreateCOPulseChanFreq @see DAQmxCreateCOPulseChanTime @see DAQmxCfgSampClkTiming
 * @see DAQmxCfgImplicitTiming @see DAQmxRegisterEveryNSamplesEvent @see DAQmxRegisterDoneEvent @see NIMEX_updateBuffersByCallbacksAndWriteToTask
 * @see DAQmxCfgDigEdgeStartTrig @see DAQmxSetExportedSampClkOutputBehavior @see DAQmxExportSignal
 *
 * @callgraph @callergraph
 */
//TO101708J - Taken from NIMEX_StartTask.c
void NIMEX_createTaskHandle(NIMEX_TaskDefinition* taskDefinition)
{
	int32                       error = 0;
	char                        errBuff[2048] = {'\0'};
    NIMEX_ChannelDefinition*    channelDefinition = (NIMEX_ChannelDefinition *)NULL;
    NIMEX_ChannelList*          channelDefinitionList = (NIMEX_ChannelList *)NULL;
    uInt32                      written;//TO100907A
    uInt32                      everyNSamplesOption = DAQmx_Val_Acquired_Into_Buffer;
    int                         channelType = NIMEX_UNINITIALIZED;//TO101007E
    uInt64                      sampsPerChanToAcquire;

    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);//TO012407B

    channelDefinitionList = NIMEX_ChannelList_first(taskDefinition->channels);
    if (channelDefinitionList == NULL)
    {
        NIMEX_releaseLock(taskDefinition);
        mexErrMsgTxt("NIMEX_Utilities/NIMEX_createTaskHandle: No channels configured.");
    }
    else if (NIMEX_linkedList_length(channelDefinitionList) == 0)
    {
        NIMEX_releaseLock(taskDefinition);
        mexErrMsgTxt("NIMEX_Utilities/NIMEX_createTaskHandle: No channels configured.");//TO022607D
    }
    else if (NIMEX_enabledChannelCount(taskDefinition) == 0)
    {
        NIMEX_releaseLock(taskDefinition);
        mexErrMsgTxt("NIMEX_Utilities/NIMEX_createTaskHandle: No channels enabled.");//TO022607D
    }

    NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: Allocating memory for NIDAQmx TaskHandle\n")
    taskDefinition->taskHandle = (TaskHandle *)(NIMEX_Persistent_Calloc(1, sizeof(TaskHandle)));

    NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: Creating NIDAQmx TaskHandle\n")
    DAQmxCreateTaskHandleErrChk(DAQmxCreateTask("", taskDefinition->taskHandle));
    NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: taskHandle = %d->%d\n", taskDefinition->taskHandle, *(taskDefinition->taskHandle))

    //TO121707B - Make sure to handle repeatOutput properly here.
    if (taskDefinition->repeatOutput)
        sampsPerChanToAcquire = taskDefinition->sampsPerChanToAcquire * taskDefinition->repeatOutput;
    else
        sampsPerChanToAcquire = taskDefinition->sampsPerChanToAcquire;
//mexPrintf("\nNIMEX_Utilities/NIMEX_createTaskHandle: sampsPerChanToAcquire = %llu\n", sampsPerChanToAcquire);
    while (channelDefinitionList != NULL)
    {
        channelDefinition = NIMEX_ChannelList_value(channelDefinitionList);//TO010607B //TO061207A
        channelDefinitionList = NIMEX_ChannelList_next(channelDefinitionList);//TO061207A

        if (channelDefinition != NULL)
        {
            //TO073070A
            if (channelDefinition->enable)
            {
                //Provide some last minute defaults, for now. This is really just temporary, while prototyping.
				///@todo Remove the hardcoded defaults for sampleMode, sampsPerChanToAcquire, and samplingRate.
                if (taskDefinition->sampleMode == NIMEX_UNINITIALIZED)
                    taskDefinition->sampleMode = DAQmx_Val_FiniteSamps;
                //Compiling with MSVC++'s cl causes a segfault when trying to assign a 64-bit literal if the suffix isn't used. -- Tim O'Connor 7/11/07
                if (taskDefinition->sampsPerChanToAcquire == NIMEX_UNINITIALIZED)
                #ifndef NIMEX_COMPILER_CL
                    taskDefinition->sampsPerChanToAcquire = 1024ull;//TO100907A
                #else
                    taskDefinition->sampsPerChanToAcquire = 1024Ui64;
                #endif
                if (taskDefinition->samplingRate == NIMEX_UNINITIALIZED)
                    taskDefinition->samplingRate = 1000;
                //if (taskDefinition->everyNSamples == NIMEX_UNINITIALIZED)
                  //  taskDefinition->everyNSamples = 1000;
    
                //if (channelDefinition->minVal == channelDefinition->maxVal)
                    //mexPrintf("Warning - NIMEX_Utilities/NIMEX_createTaskHandle: When compiled with Microsoft's cl compiler, 64-bit floating point values can not be defaulted (channelDefinition->minVal, channelDefinition->maxVal).\n");
    
                channelType = channelDefinition->channelType;//TO101007E
                NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: \n")
                #ifdef NIMEX_VERBOSE
                    NIMEX_displayTaskDefinition("    ", taskDefinition);//TO010707A
                    NIMEX_displayChannelDefinition("    taskDefinition->", channelDefinition);//TO010707A
                #endif
    
                //TO101007E
                switch (channelType)
                {
                    case NIMEX_ANALOG_INPUT:
                        NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: Configuring NIDAQmx AI voltage channel \"%s\".\n", channelDefinition->physicalChannel)
                        DAQmxCreateTaskHandleErrChk(DAQmxCreateAIVoltageChan(*(taskDefinition->taskHandle), 
                            channelDefinition->physicalChannel, 
                            channelDefinition->mnemonicName, 
                            channelDefinition->terminalConfig, 
                            channelDefinition->minVal, 
                            channelDefinition->maxVal, 
                            channelDefinition->units, 
                            NULL));
                        NIMEX_MACRO_VERBOSE("  NIMEX_Utilities/NIMEX_createTaskHandle: Setting AI timing - @%p\n\tclock source: \"%s\"\n\tsamplingRate: %3.4f\n\tclockActiveEdge: %d\n\tsampleMode: %d\n\tsampsPerChanToAcquire: %llu\n", 
                            taskDefinition->taskHandle,
                            taskDefinition->clockSource,
                            taskDefinition->samplingRate, 
                            taskDefinition->clockActiveEdge, 
                            taskDefinition->sampleMode, 
                            sampsPerChanToAcquire)//TO100907A
                        DAQmxCreateTaskHandleErrChk(DAQmxCfgSampClkTiming(*(taskDefinition->taskHandle), 
                            taskDefinition->clockSource, 
                            taskDefinition->samplingRate, 
                            taskDefinition->clockActiveEdge, 
                            taskDefinition->sampleMode, 
                            sampsPerChanToAcquire));//TO100907A
                        everyNSamplesOption = DAQmx_Val_Acquired_Into_Buffer;
                        break;
    
                    case NIMEX_ANALOG_OUTPUT:
                        NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: Configuring NIDAQmx AO voltage channel \"%s\".\n", channelDefinition->physicalChannel)
                        DAQmxCreateTaskHandleErrChk(DAQmxCreateAOVoltageChan(*(taskDefinition->taskHandle), 
                            channelDefinition->physicalChannel, 
                            channelDefinition->mnemonicName, 
                            channelDefinition->minVal, 
                            channelDefinition->maxVal, 
                            channelDefinition->units, 
                            NULL));
                        NIMEX_MACRO_VERBOSE("  NIMEX_Utilities/NIMEX_createTaskHandle: Setting AO timing - @%p\n\tclock source: \"%s\"\n\tsamplingRate: %3.4f\n\tclockActiveEdge: %d\n\tsampleMode: %d\n\tsampsPerChanToAcquire: %llu\n", 
                            taskDefinition->taskHandle,
                            taskDefinition->clockSource,
                            taskDefinition->samplingRate, 
                            taskDefinition->clockActiveEdge, 
                            taskDefinition->sampleMode, 
                            sampsPerChanToAcquire)//TO022307B//TO100907A
                        DAQmxCreateTaskHandleErrChk(DAQmxCfgSampClkTiming(*(taskDefinition->taskHandle), 
                            taskDefinition->clockSource, 
                            taskDefinition->samplingRate, 
                            taskDefinition->clockActiveEdge, 
                            taskDefinition->sampleMode, 
                            sampsPerChanToAcquire));//TO022307B//TO100907A
                        if (taskDefinition->triggerSource != NULL)
                        {
                            if (strlen(taskDefinition->triggerSource) > 0)
                            {
                                NIMEX_MACRO_VERBOSE("  NIMEX_Utilities/NIMEX_createTaskHandle: Setting AO digital triggering - @%d->triggerSource: \"%s\"\n", taskDefinition->taskHandle, taskDefinition->triggerSource) 
                                DAQmxCreateTaskHandleErrChk(DAQmxCfgDigEdgeStartTrig(*(taskDefinition->taskHandle),
                                    taskDefinition->triggerSource, taskDefinition->clockActiveEdge));
                            }
                        }
                        everyNSamplesOption = DAQmx_Val_Transferred_From_Buffer;
                        break;
    
                    case NIMEX_DIGITAL_INPUT:
                        NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: Configuring NIDAQmx digital input channel \"%s\".\n", channelDefinition->physicalChannel)
                        DAQmxCreateTaskHandleErrChk(DAQmxCreateDIChan(*(taskDefinition->taskHandle), 
                            channelDefinition->physicalChannel, 
                            channelDefinition->mnemonicName, 
                            taskDefinition->lineGrouping));
                        NIMEX_MACRO_VERBOSE("  NIMEX_Utilities/NIMEX_createTaskHandle: Setting digital input timing - @%p\n\tclock source: \"%s\"\n\tsamplingRate: %3.4f\n\tclockActiveEdge: %d\n\tsampleMode: %d\n\tsampsPerChanToAcquire: %llu\n", 
                            taskDefinition->taskHandle,
                            taskDefinition->clockSource,
                            taskDefinition->samplingRate, 
                            taskDefinition->clockActiveEdge, 
                            taskDefinition->sampleMode, 
                            sampsPerChanToAcquire) //TO100907A
                        DAQmxCreateTaskHandleErrChk(DAQmxCfgSampClkTiming(*(taskDefinition->taskHandle), 
                            taskDefinition->clockSource, 
                            taskDefinition->samplingRate, 
                            taskDefinition->clockActiveEdge, 
                            taskDefinition->sampleMode, 
                            sampsPerChanToAcquire));//TO100907A
                        everyNSamplesOption = DAQmx_Val_Acquired_Into_Buffer;
                        break;
                        //NIMEX_releaseLock(taskDefinition);//TO012407B
                        //mexErrMsgTxt("NIMEX_DIGITAL_INPUT not yet implemented.");
                        //break;
    
                    case NIMEX_DIGITAL_OUTPUT:
                        NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: Configuring NIDAQmx digital output channel \"%s\".\n", channelDefinition->physicalChannel)
                        DAQmxCreateTaskHandleErrChk(DAQmxCreateDOChan(*(taskDefinition->taskHandle), 
                            channelDefinition->physicalChannel, 
                            channelDefinition->mnemonicName, 
                            taskDefinition->lineGrouping));
                        NIMEX_MACRO_VERBOSE("  NIMEX_Utilities/NIMEX_createTaskHandle: Setting digital output timing - @%p\n\tclock source: \"%s\"\n\tsamplingRate: %3.4f\n\tclockActiveEdge: %d\n\tsampleMode: %d\n\tsampsPerChanToAcquire: %d\n", 
                            taskDefinition->taskHandle,
                            taskDefinition->clockSource,
                            taskDefinition->samplingRate, 
                            taskDefinition->clockActiveEdge, 
                            taskDefinition->sampleMode, 
                            sampsPerChanToAcquire)//TO022307B //TO100907A
                        DAQmxCreateTaskHandleErrChk(DAQmxCfgSampClkTiming(*(taskDefinition->taskHandle), 
                            taskDefinition->clockSource, 
                            taskDefinition->samplingRate, 
                            taskDefinition->clockActiveEdge, 
                            taskDefinition->sampleMode, 
                            sampsPerChanToAcquire));//TO022307B //TO100907A
                        everyNSamplesOption = DAQmx_Val_Transferred_From_Buffer;
                        break;
                        
                    case NIMEX_CO_FREQUENCY:
                        //TO073107E
                        if (channelDefinition->dataSource == NULL)
                        {
                            mexPrintf("NIMEX_Utilities/NIMEX_createTaskHandle: Error - No pulse data specified (dataSource == NULL) for channel \"%s\".\n", channelDefinition->physicalChannel);
                            goto CreateTaskHandleError;
                        }

                        NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: Configuring NIDAQmx frequency counter output channel \"%s\".\n", channelDefinition->physicalChannel)
                        DAQmxCreateTaskHandleErrChk(DAQmxCreateCOPulseChanFreq(*(taskDefinition->taskHandle), 
                            channelDefinition->physicalChannel, 
                            channelDefinition->mnemonicName,
                            DAQmx_Val_Hz,
                            taskDefinition->idleState,
                            ((float64 *)channelDefinition->dataSource->dataBuffer)[0], 
                            ((float64 *)channelDefinition->dataSource->dataBuffer)[1], 
                            ((float64 *)channelDefinition->dataSource->dataBuffer)[2] ));
                            //*((float64 *)channelDefinition->dataSource->dataBuffer),
                            //*((float64 *)((int)channelDefinition->dataSource->dataBuffer + sizeof(float64))),
                            //*((float64 *)((int)channelDefinition->dataSource->dataBuffer + (2 * sizeof(float64)))) ));
                        NIMEX_MACRO_VERBOSE("  NIMEX_Utilities/NIMEX_createTaskHandle: Setting counter implicit timing - @%p\n\tsampleMode: %d\n\tsampsPerChanToAcquire: %llu\n", 
                            taskDefinition->taskHandle,
                            taskDefinition->sampleMode, 
                            sampsPerChanToAcquire)//TO022307B //TO100907A
                        DAQmxCreateTaskHandleErrChk(DAQmxCfgImplicitTiming(*(taskDefinition->taskHandle), 
                            taskDefinition->sampleMode,
                            sampsPerChanToAcquire));//TO022307B //TO100907A
                        everyNSamplesOption = DAQmx_Val_Transferred_From_Buffer;
                        break;

                    case NIMEX_CO_TIME:
                        //TO073107E
                        if (channelDefinition->dataSource == NULL)
                        {
                            mexPrintf("NIMEX_Utilities/NIMEX_createTaskHandle: Error - No pulse data specified (dataSource == NULL) for channel \"%s\".\n", channelDefinition->physicalChannel);
                            goto CreateTaskHandleError;
                        }

                        NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: Configuring NIDAQmx time counter output channel \"%s\".\n", channelDefinition->physicalChannel)
                        DAQmxCreateTaskHandleErrChk(DAQmxCreateCOPulseChanTime(*(taskDefinition->taskHandle), 
                            channelDefinition->physicalChannel, 
                            channelDefinition->mnemonicName,
                            DAQmx_Val_Seconds,
                            taskDefinition->idleState,
                            ((float64 *)channelDefinition->dataSource->dataBuffer)[0], 
                            ((float64 *)channelDefinition->dataSource->dataBuffer)[1], 
                            ((float64 *)channelDefinition->dataSource->dataBuffer)[2] ));
                            //*((float64 *)channelDefinition->dataSource->dataBuffer),
                            //*((float64 *)((int)channelDefinition->dataSource->dataBuffer + sizeof(float64))),
                            //*((float64 *)((int)channelDefinition->dataSource->dataBuffer + (2 * sizeof(float64)))) ));
                        NIMEX_MACRO_VERBOSE("  NIMEX_Utilities/NIMEX_createTaskHandle: Setting counter implicit timing - @%p\n\tsampleMode: %d\n\tsampsPerChanToAcquire: %llu\n", 
                            taskDefinition->taskHandle,
                            taskDefinition->sampleMode, 
                            sampsPerChanToAcquire)//TO022307B //TO100907A
                        DAQmxCreateTaskHandleErrChk(DAQmxCfgImplicitTiming(*(taskDefinition->taskHandle), 
                            taskDefinition->sampleMode,
                            sampsPerChanToAcquire));//TO022307B //TO100907A
                        everyNSamplesOption = DAQmx_Val_Transferred_From_Buffer;
                        break;
    
                        default:
                            mexErrMsgTxt("Unsupported channel type. Must be NIMEX_ANALOG_INPUT | NIMEX_ANALOG_OUTPUT | NIMEX_DIGITAL_INPUT | NIMEX_DIGITAL_OUTPUT");
                }//switch
            }//if (channelDefinition->enabled)
        }//if (channelDefinition != NULL)
    }//while (channelDefinitionList != NULL)
    
    //TO112907D - Moved the event registration to come before updating the buffers. Setting up the doneEvent seems to clear the buffer for digital output tasks. -- Tim O'Connor 11/29/07
    //TO061907E
    if ((taskDefinition->eventMap != NULL) && (taskDefinition->everyNSamples != NIMEX_UNINITIALIZED))
    {
        //TO101007C - Use everyNSamplesOption for the event registration, it had been hardcoded just for inputs, and never tested for outputs. -- Tim O'Connor 10/10/07
        NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: Calling DAQmxRegisterEveryNSamplesEvent: @%p for N=%d\n", *(taskDefinition->taskHandle), taskDefinition->everyNSamples)
        DAQmxCreateTaskHandleErrChk(DAQmxRegisterEveryNSamplesEvent(*(taskDefinition->taskHandle), everyNSamplesOption, taskDefinition->everyNSamples, 0, NIMEX_EveryNCallback, taskDefinition));//TO010707C//TO012907H
        //TO080507B - Install the message pump hook.
        if (NIMEX_getWindowsHookId() == NULL)
            NIMEX_initializeCallbackProcessing();
    }

    //TO061907E
    if (taskDefinition->eventMap != NULL)
    {
        NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: Calling DAQmxRegisterDoneEvent: @%p\n", *(taskDefinition->taskHandle))
        DAQmxCreateTaskHandleErrChk(DAQmxRegisterDoneEvent(*(taskDefinition->taskHandle), 0, NIMEX_DoneCallback, taskDefinition));//TO010707C
        //TO080507B - Install the message pump hook.
        if (NIMEX_getWindowsHookId() == NULL)
            NIMEX_initializeCallbackProcessing();
    }
    else
    {
        //TO101607E - Always process the done event, even if no callbacks are needed, so the task is automatically stopped and hardware is unreserved. -- Tim O'Connor 10/16/07
        NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: Calling DAQmxRegisterDoneEvent: @%p - No callbacks are to be processed, just auto-stop the task.\n", *(taskDefinition->taskHandle))
        DAQmxCreateTaskHandleErrChk(DAQmxRegisterDoneEvent(*(taskDefinition->taskHandle), 0, NIMEX_DoneCallback, NULL));//TO010707C
    }

    //TO102007B
    if ( (channelType == NIMEX_ANALOG_OUTPUT || channelType == NIMEX_DIGITAL_OUTPUT) )
    {
        written = NIMEX_updateBuffersByCallbacksAndWriteToTask(taskDefinition);
        if (written == 0)
        {
            mexPrintf("NIMEX_Utilities/NIMEX_createTaskHandle: Failed to update buffer(s). Aborting task creation.\n");
            goto CreateTaskHandleError;
        }
    }

    if (taskDefinition->triggerSource != NULL)
    {
        if (strlen(taskDefinition->triggerSource) > 0)
        {
            NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: Setting digital triggering - @%p->triggerSource: \"%s\"\n", taskDefinition->taskHandle, taskDefinition->triggerSource)
            DAQmxCreateTaskHandleErrChk(DAQmxCfgDigEdgeStartTrig(*(taskDefinition->taskHandle), taskDefinition->triggerSource, taskDefinition->clockActiveEdge));
        }
    }

    //TO101007E
    if ((taskDefinition->clockExportTerminal != NULL) && !((channelType == NIMEX_DIGITAL_OUTPUT) || (channelType == NIMEX_DIGITAL_INPUT)))
    {
        NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: Exporting clock signal for @%p on \"%s\"\n", taskDefinition->taskHandle, taskDefinition->clockExportTerminal)
        if (channelType == NIMEX_ANALOG_INPUT) //TO033008F
            DAQmxCreateTaskHandleErrChk(DAQmxSetExportedSampClkOutputBehavior(*(taskDefinition->taskHandle), DAQmx_Val_Pulse));
        DAQmxCreateTaskHandleErrChk(DAQmxExportSignal(*(taskDefinition->taskHandle), DAQmx_Val_SampleClock, taskDefinition->clockExportTerminal));
    }
    
    //TO102508E
    if (taskDefinition->sampleClockTimebaseSource != NULL)
    {
        NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: Setting sample clock timebase source for @%p to \"%s\"\n", taskDefinition->taskHandle, taskDefinition->sampleClockTimebaseSource)
        DAQmxSetSampClkTimebaseSrc(*(taskDefinition->taskHandle), taskDefinition->sampleClockTimebaseSource);
    }

    NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_createTaskHandle: Created NIDAQmx TaskHandle in NIMEX_TaskDefinition: %p\n", *(taskDefinition->taskHandle))

CreateTaskHandleError:
	if (DAQmxFailed(error))
    {
		DAQmxGetExtendedErrorInfo(errBuff, 2048);

    	if ((int)(*(taskDefinition->taskHandle)) != (int)NULL)
        {
    		DAQmxStopTask(*(taskDefinition->taskHandle));
    		DAQmxClearTask(*(taskDefinition->taskHandle));
            NIMEX_freePersistentMemorySegment(taskDefinition->taskHandle);
            taskDefinition->taskHandle = (TaskHandle *)NULL;
        }
    }

    NIMEX_releaseLock(taskDefinition);//TO012407B
	if (DAQmxFailed(error))
    {
        //TO022607D
		mexPrintf("NIMEX_startTask failed...\n");
        mexErrMsgTxt(errBuff);
    }
    
    return;
}

/**
 * @brief Creates a task, if necessary, and puts it into the NIDAQmx verified state.
 * @arg <tt>taskDefinition</tt> - A fully configured NIMEX task definition.
 *
 * @post The underlying NIDAQmx task is in the verified state.
 * @see DAQmxTaskControl @see NIMEX_createTaskHandle
 * @callergraph
 */
//TO101708J
int32 NIMEX_VerifyTask(NIMEX_TaskDefinition* taskDefinition)
{
    int32 result = 0;
    
    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);
    
    if (taskDefinition->taskHandle == NULL)
        NIMEX_createTaskHandle(taskDefinition);

    if (taskDefinition->taskHandle == NULL)
        NIMEX_createTaskHandle(taskDefinition);
    
    result = DAQmxTaskControl(*(taskDefinition->taskHandle), DAQmx_Val_Task_Verify);
    
    NIMEX_releaseLock(taskDefinition);
    
    return result;
}

/**
 * @brief Creates a task, if necessary, and puts it into the NIDAQmx committed state.
 * @arg <tt>taskDefinition</tt> - A fully configured NIMEX task definition.
 *
 * @post The underlying NIDAQmx task is in the committed state.
 * @see DAQmxTaskControl @see NIMEX_createTaskHandle
 * @callergraph
 */
//TO101708J
int32 NIMEX_CommitTask(NIMEX_TaskDefinition* taskDefinition)
{
    int32 result = 0;
    uInt32 written = 0;
    
    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);
    
    if (taskDefinition->taskHandle == NULL)
        NIMEX_createTaskHandle(taskDefinition);

    //TO102007B - Update the output buffers on commit, for use in restarting tasks during loops or "external trigger" mode.
    written = NIMEX_updateBuffersByCallbacksAndWriteToTask(taskDefinition);
    if (written == 0)
    {
        mexPrintf("NIMEX_Utilities/NIMEX_CommitTask: Failed to update output buffer(s).\n");
    }
    
    result = DAQmxTaskControl(*(taskDefinition->taskHandle), DAQmx_Val_Task_Commit);
    
    NIMEX_releaseLock(taskDefinition);

    return result;
}

/**
 * @brief Creates a task, if necessary, and puts it into the NIDAQmx reserved state.
 * @arg <tt>taskDefinition</tt> - A fully configured NIMEX task definition.
 *
 * @post The underlying NIDAQmx task is in the reserved state.
 * @see DAQmxTaskControl @see NIMEX_createTaskHandle
 * @callergraph
 */
//TO101708J
int32 NIMEX_ReserveTask(NIMEX_TaskDefinition* taskDefinition)
{
    int32 result = 0;
    
    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);
    
    if (taskDefinition->taskHandle == NULL)
        NIMEX_createTaskHandle(taskDefinition);
    
    result = DAQmxTaskControl(*(taskDefinition->taskHandle), DAQmx_Val_Task_Reserve);
    
    NIMEX_releaseLock(taskDefinition);
    
    return result;
}

/**
 * @brief Probes Matlab for a version string. This is done only once per loading of the library, as the version should not change otherwise.
 * 
 * The version string is retrieved from Matlab and stored in a global variable (with scope local to this shared library). It is never
 * reloaded. The version should only change if Matlab gets restarted, in which case this variable will get reinitialized.
 *
 * The version string is stored in <tt>NIMEX_RuntimeMatlabVersionString,</tt> the version is also preprocessed into a double for ease of comparison
 * and stored as <tt>NIMEX_RuntimeMatlabVersionNumber</tt>, lastly, the release string (ie. 2007a) is stored as <tt>NIMEX_RuntimeMatlabReleaseString</tt>
 *
 * @callergraph
 */
void NIMEX_initMatlabVersionString(void)
{
   mxArray* plhs[1];
   mxArray* prhs[1];
   int i = 0;
   int dCount = 0;
   char replacementChar = 0x00;

   //Use the `ver` command to get a struct array with versions of Matlab and its toolboxes.
   //The `ver('MATLAB')` command only looks up the version of Matlab's core, which is all we are interested in.
   if ( (NIMEX_RuntimeMatlabVersionString == NULL) || (NIMEX_RuntimeMatlabReleaseString == NULL) )
   {
     prhs[0] = mxCreateString("MATLAB");
     if (mexCallMATLAB(1, plhs, 1, prhs, "ver"))
     {
        mexPrintf("NIMEX_Utilities/NIMEX_initMatlabVersionString: Failed to retrieve Matlab version string using `ver`.\n");
        mxDestroyArray(*prhs);
        return;
     }
     mxDestroyArray(*prhs);

     NIMEX_RuntimeMatlabVersionString = NIMEX_MACRO_MXARRAY_2_PERSISTENT_CHAR(mxGetField(plhs[0], 0, "Version"));
     NIMEX_RuntimeMatlabReleaseString = NIMEX_MACRO_MXARRAY_2_PERSISTENT_CHAR(mxGetField(plhs[0], 0, "Release"));
     mxDestroyArray(*plhs);
   }

   if (NIMEX_RuntimeMatlabVersionNumber <= 0)
   {
      for(i = 0; i < strlen(NIMEX_RuntimeMatlabVersionString); i++)
      {
         if (NIMEX_RuntimeMatlabVersionString[i] == '.')
            dCount++;
         if (dCount > 1)
         {
            replacementChar = NIMEX_RuntimeMatlabVersionString[i];
            NIMEX_RuntimeMatlabVersionString[i] = 0x00;
            break;
         }
      }

      NIMEX_RuntimeMatlabVersionNumber = strtod(NIMEX_RuntimeMatlabVersionString, NULL);
   
      if (replacementChar)
         NIMEX_RuntimeMatlabVersionString[i] = replacementChar;
   }

   return;
}

/**
 * @brief Returns the Matlab version string, for the session that is currently running.
 *
 * @return A pointer to a string containing the version. This string must be deallocated by the caller.
 * @see NIMEX_initMatlabVersionString
 * @callergraph
 */
//TO090108A
char* NIMEX_getMatlabVersionString(void)
{
   char* version = NULL;

   NIMEX_initMatlabVersionString();

   version = (char *)calloc(sizeof(char), strlen(NIMEX_RuntimeMatlabVersionString) + 1);
   memcpy(version, NIMEX_RuntimeMatlabVersionString, strlen(NIMEX_RuntimeMatlabVersionString));

   return version;
}

/**
 * @brief Returns the Matlab release string, for the session that is currently running.
 *
 * @return A pointer to a string containing the release. This string must be deallocated by the caller.
 * @see NIMEX_initMatlabVersionString
 * @callergraph
 */
//TO090108A
char* NIMEX_getMatlabReleaseString(void)
{
   char* release = NULL;

   NIMEX_initMatlabVersionString();

   release = (char *)calloc(sizeof(char), strlen(NIMEX_RuntimeMatlabReleaseString) + 1);
   memcpy(release, NIMEX_RuntimeMatlabReleaseString, strlen(NIMEX_RuntimeMatlabReleaseString));

   return release;
}

/**
 * @brief Returns the Matlab version number, for the session that is currently running.
 *
 * @return A double that represents the version number to one decimal place of accuracy.
 * @see NIMEX_initMatlabVersionString
 * @callergraph
 */
//TO090108A
double NIMEX_getMatlabVersionNumber(void)
{
   return NIMEX_RuntimeMatlabVersionNumber;
}

/**
 * @brief Determines if the current Matlab version is greater than a minimum version number.
 *
 * @arg <tt>desiredVersion</tt> - The Matlab version to be used for comparison.
 * @return 1 if the version is greater than <tt>desiredVersion</tt>, 0 otherwise.
 * @see NIMEX_initMatlabVersionString
 * @callergraph
 */
int32 NIMEX_isMatlabVersionGreaterThan(double desiredVersion)
{
   NIMEX_initMatlabVersionString();
   
   if (desiredVersion > NIMEX_RuntimeMatlabVersionNumber)
      return 0;
      
   return 1;
}

/**
 * @brief Determines if the current Matlab version is greater than or equal to a minimum version number.
 *
 * @arg <tt>desiredVersion</tt> - The Matlab version to be used for comparison.
 * @return 1 if the version is greater than or equal to <tt>desiredVersion</tt>, 0 otherwise.
 * @see NIMEX_initMatlabVersionString
 * @callergraph
 */
int32 NIMEX_isMatlabVersionGreaterThanOrEqual(double desiredVersion)
{
   NIMEX_initMatlabVersionString();
   
   if (desiredVersion >= NIMEX_RuntimeMatlabVersionNumber)
      return 0;
      
   return 1;
}

/**
 * @brief Determines if the current Matlab version is less than a minimum version number.
 *
 * @arg <tt>desiredVersion</tt> - The Matlab version to be used for comparison.
 * @return 1 if the version is less than <tt>desiredVersion</tt>, 0 otherwise.
 * @see NIMEX_initMatlabVersionString
 * @callergraph
 */
int32 NIMEX_isMatlabVersionLessThan(double desiredVersion)
{
   NIMEX_initMatlabVersionString();
   
   if (desiredVersion < NIMEX_RuntimeMatlabVersionNumber)
      return 0;
      
   return 1;
}

/**
 * @brief Determines if the current Matlab version is less than or equal to a minimum version number.
 *
 * @arg <tt>desiredVersion</tt> - The Matlab version to be used for comparison.
 * @return 1 if the version is less than or equal to <tt>desiredVersion</tt>, 0 otherwise.
 * @see NIMEX_initMatlabVersionString
 * @callergraph
 */
int32 NIMEX_isMatlabVersionLessThanOrEqual(double desiredVersion)
{
   NIMEX_initMatlabVersionString();
   
   if (desiredVersion <= NIMEX_RuntimeMatlabVersionNumber)
      return 0;
      
   return 1;
}
