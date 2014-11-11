/**
 * @file NIMEXEng_getDeviceNames.c
 *
 * @brief Retrieves a list of all available NIDAQmx devices.
 *
 * This probes the NIDAQmx drivers and determines all devices that are available, 
 * but does not indicate anything about their state.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEXEng_getDeviceNames()</tt>
 *
 * @return A cell array of strings, containing the names of all available devices.
 *
 * @author Timothy O'Connor
 * @date 2/26/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
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

    NIMEX_MACRO_PRE_MEX
    
    if (nlhs != 1)
      mexErrMsgTxt("NIMEXEng_getDeviceNames: One, and only one, output argument must be supplied.");
    
    bufferSize = DAQmxGetSysDevNames(NULL, 0);
    names = mxCalloc(bufferSize, sizeof(char));
    DAQmxErrChk(DAQmxGetSysDevNames(names, bufferSize));
    NIMEX_MACRO_VERBOSE("NIMEXEng_getDeviceNames: %s\n", names);    
   
    plhs[0] = mxCreateString(names);
    
    mxFree(names);
   
Error:
    if (DAQmxFailed(error))
    {
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
        mexErrMsgTxt(errBuff);
    }
    
    NIMEX_MACRO_POST_MEX
    
    return;
}
