/**
 * @file NIMEXEng_disconnectTerms.c
 *
 * @brief Unconfigure immediate routing (non-task based) for a set of NIDAQmx terminals.
 *
 * The hardware is immediately reserved and committed, and applies to all tasks.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEXEng_disconnectTerms(src, dest)</tt>
 *  @arg <tt>src</tt> - The source terminal.
 *  @arg <tt>dest</tt> - The destination terminal.
 *
 * @see NIMEXEng_disconnectTerms.c
 *
 * @author Timothy O'Connor
 * @date 5/5/08
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2008</em>
 */
/*
 * CHANGES
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
	int32   error = 0;
    char    errBuff[2048] = {'\0'};
    int     bufferSize = 0;
    char*   names = (char *)NULL;
    //char	names[2048] = {'\0'};

    NIMEX_MACRO_PRE_MEX
    
    if (nlhs != 0)
      mexErrMsgTxt("NIMEXEng_disconnectTerms: No return arguments supported.");
    
    if (nrhs != 2)
      mexErrMsgTxt("NIMEXEng_disconnectTerms: Invalid number of arguments. A source and destination terminal must be specified.");

    NIMEX_MACRO_VERBOSE("NIMEXEng_disconnectTerms: '%s' --> '%s'\n", mxArrayToString(prhs[0]), mxArrayToString(prhs[1]))
    
    DAQmxErrChk(DAQmxDisconnectTerms(mxArrayToString(prhs[0]), mxArrayToString(prhs[1])));
    
Error:
    if (DAQmxFailed(error))
    {
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
        mexErrMsgTxt(errBuff);
    }
    
    NIMEX_MACRO_POST_MEX
    
    return;
}
