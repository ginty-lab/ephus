/**
 * @file NIMEXEng_resetDevice.c
 *
 * @brief Resets a NIDAQmx device.
 *
 * This is the same as using the "Reset Device" button in NI-Max.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEXEng_resetDevice(deviceName)</tt>
 *  @arg <tt>deviceName</tt> - The device to be reset.
 *
 * @author Timothy O'Connor
 * @date 5/5/08
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2008</em>
 */
#include "nimex.h"

#define DAQmxErrChk(functionCall) if (DAQmxFailed(error = (functionCall))) goto Error; else

/**
 * @brief Matlab entry point.
 * @callgraph
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	int32   error = 0;
    char    errBuff[2048] = {'\0'};
    int     bufferSize = 0;
    char*   names = (char *)NULL;
    //char	names[2048] = {'\0'};

    //NIMEX_MACRO_PRE_MEX //This short circuits, as of 5/5/08 for cases when there's only one arg.
    
    if (nrhs != 0)
      mexErrMsgTxt("NIMEXEng_resetDevice: No return arguments supported.");
    
    if (nrhs != 2)
      mexErrMsgTxt("NIMEXEng_resetDevice: Invalid number of arguments. A single device name must be specified.");

    NIMEX_MACRO_VERBOSE("NIMEXEng_resetDevice: '%s'\n", mxArrayToString(prhs[0]))
    
    DAQmxErrChk(DAQmxResetDevice(mxArrayToString(prhs[0])));
    
Error:
    if (DAQmxFailed(error))
    {
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
        mexErrMsgTxt(errBuff);
    }
    
    NIMEX_MACRO_POST_MEX
    
    return;
}
