/**
 * @file NIMEXEng_tristateOutputTerm.c
 *
 * @brief Sets a terminal to high-impedance (neutral) mode.
 *
 * This effectively disables a line, so it acts as neither a source or a sink.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEXEng_tristateOutputTerm(terminalName)</tt>
 *  @arg <tt>terminalName</tt> - The terminal to be tri-stated.
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
    
    if (nlhs != 0)
      mexErrMsgTxt("NIMEXEng_tristateOutputTerm: No return arguments supported.");
    
    if (nrhs != 2)
      mexErrMsgTxt("NIMEXEng_tristateOutputTerm: Invalid number of arguments. A single terminal name must be specified.");

    NIMEX_MACRO_VERBOSE("NIMEXEng_tristateOutputTerm: '%s'\n", mxArrayToString(prhs[0]))
    
    DAQmxErrChk(DAQmxTristateOutputTerm(mxArrayToString(prhs[0])));
    
Error:
    if (DAQmxFailed(error))
    {
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
        mexErrMsgTxt(errBuff);
    }
    
    NIMEX_MACRO_POST_MEX
    
    return;
}
