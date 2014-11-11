/**
 * @file NIMEX_sendTrigger.c
 *
 * @brief Sends a digital trigger from the specified digital device.
 *
 * This is a convenience function. It has no explicit connection to the NIMEX task that is passed in, this is just
 * used to resolve the namespace of the NIMEX functions.
 * The actual line used to send the pulse is taken as an argument to this function, and can be on any available subsystem.
 * The pulse itself is low-high-low, and assumes that the line is already in the low state or is tristated (high impedance).
 * The pulse is sent under CPU timing control, that is to say, it is unbuffered and uses single point timing across 3 writes.
 *
 * @note
 *  Because this function is conceptually a bit ugly, it may be deprecated sometime in the future.
 *  However, since it is simultaneously rather useful and benign, there are no immediate plans to deprecate it.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_sendTrigger(taskDefinition, digitalLine)</tt>
 *  @arg <tt>taskDefinition</tt> - Any NIMEX task, this is just to resolve the NIMEX function namespace, and is ignored in the function.
 *  @arg <tt>digitalLine</tt> - The fully-qualified NIDAQmx name of available digital line on which to send a pulse (low-high-low).
 *
 * @return A value for each property queried.
 *
 * @author Timothy O'Connor
 * @date 12/8/06
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006</em>
 */
/*
 * NOTES
 *  See TO120806B.
 *
 * CHANGES
 *  TO012407B: Needed a cast to a conditional statement to rectify types when compiled with windows.h. -- Tim O'Connor 1/24/07
 *  TO033007A: Added NIMEX_LAME to test library without running hardware (used when experiencing crashes). -- Tim O'Connor 3/30/07
 *
 */
#include "nimex.h"

#define DAQmxErrChk(functionCall) if (DAQmxFailed(error = (functionCall))) goto Error; else

/**
 * @brief Matlab entry point.
 * @callgraph
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    TaskHandle                  triggerSignal = (TaskHandle)NULL;
	int32                       error = 0;
    char*                       digitalDeviceID = (char *)NULL;
	char                        errBuff[2048] = {'\0'};
    uInt8                       digitalData1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    uInt8                       digitalData2[8] = {1, 1, 1, 1, 1, 1, 1, 1};

    if (nlhs != 0)
        mexErrMsgTxt("NIMEX_sendTrigger: No return arguments supported.");
    
    if (nrhs < 2)
        mexErrMsgTxt("NIMEX_sendTrigger: Insufficient arguments (a digital device identifier is required).");

    digitalDeviceID = mxArrayToString(prhs[1]);
    NIMEX_MACRO_VERBOSE("NIMEX_sendTrigger: Creating digital output: '%s'\n", digitalDeviceID)
    
	DAQmxErrChk(DAQmxCreateTask("", &triggerSignal));
	DAQmxErrChk(DAQmxCreateDOChan(triggerSignal, digitalDeviceID, "", DAQmx_Val_ChanForAllLines));

    NIMEX_MACRO_VERBOSE("NIMEX_sendTrigger: Sending digital trigger signal on: '%s'\n", digitalDeviceID)
    //TO033007A
    #ifndef NIMEX_LAME
    DAQmxErrChk(DAQmxStartTask(triggerSignal));
    DAQmxErrChk(DAQmxWriteDigitalLines(triggerSignal, 1, 1, 10.0, DAQmx_Val_GroupByChannel, digitalData1, NULL, NULL));
    DAQmxErrChk(DAQmxWriteDigitalLines(triggerSignal, 1, 1, 10.0, DAQmx_Val_GroupByChannel, digitalData2, NULL, NULL));
    DAQmxErrChk(DAQmxWriteDigitalLines(triggerSignal, 1, 1, 10.0, DAQmx_Val_GroupByChannel, digitalData1, NULL, NULL));
    #else
    NIMEX_MACRO_VERBOSE("  NIMEX_LAME - DAQmxStartTask & DAQmxWriteDigitalLines not executed.\n")
    #endif
    
Error:
	if (DAQmxFailed(error))
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
    
    //TO012407B - Needed a cast here after adding the include of windows.h, for some reason...
	if (((unsigned long)triggerSignal) != ((unsigned long)NULL))
    {
        NIMEX_MACRO_VERBOSE("Stopping and clearing /dev1/port0/line0:7...\n");
		DAQmxStopTask(triggerSignal);
		DAQmxClearTask(triggerSignal);
	}

	if (DAQmxFailed(error))
		mexPrintf("NIMEX_sendTrigger Error: %s\n", errBuff);
    
    return;
}
