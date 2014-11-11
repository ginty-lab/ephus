/**
 * @file NIMEXEng_connectTerms.c
 *
 * @brief Configure immediate routing (non-task based) for a set of NIDAQmx terminals.
 *
 * The hardware is immediately reserved and committed, and applies to all tasks.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEXEng_connectTerms(src, dest)</tt>
 *  @arg <tt>src</tt> - The source terminal.
 *  @arg <tt>dest</tt> - The destination terminal.
 *
 *
 *  The available terminals and their meanings are hardware specific. NI-Max provides a chart
 *  that can explain the available routes for all terminals for any installed hardware.\n
 *  A very common use for this function is the routing of clocks. Note that the clock names may
 *  vary across different hardware.\n
 *
 *  As part of NI-DAQmx Help, NI provides schematics specifically for the clocks of 
 *  \ref MSeriesClocksFig "M-Series", \ref ESeriesClocksFig "E-Series", and \ref CSeriesClocksFig "C-Series" hardware.
 *  For ease of reference, those figures are reproduced here:
 *
 * @anchor MSeriesClocksFig
 * @image html NI-MSeriesClocks.PNG "Figure 1 - NI-DAQmx M-Series Clock Schematic."\n\n
 *
 * @anchor ESeriesClocksFig
 * @image html NI-ESeriesClocks.PNG "Figure 2 - NI-DAQmx E-Series Clock Schematic."\n\n
 *
 * @anchor CSeriesClocksFig
 * @image html NI-CSeriesClocks.PNG "Figure 3 - NI-DAQmx C-Series Clock Schematic."\n\n
 *
 * @see NIMEXEng_disconnectTerms.c
 * @see NI-Clocks.pdf
 * @see NI-DAQmx Help
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
      mexErrMsgTxt("NIMEXEng_connectTerms: No return arguments supported.");
    
    if (nrhs != 2)
      mexErrMsgTxt("NIMEXEng_connectTerms: Invalid number of arguments. A source and destination terminal must be specified.");

    NIMEX_MACRO_VERBOSE("NIMEXEng_connectTerms: '%s' --> '%s'\n", mxArrayToString(prhs[0]), mxArrayToString(prhs[1]))
    
    DAQmxErrChk(DAQmxConnectTerms(mxArrayToString(prhs[0]), mxArrayToString(prhs[1]), DAQmx_Val_DoNotInvertPolarity));
    
Error:
    if (DAQmxFailed(error))
    {
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
        mexErrMsgTxt(errBuff);
    }
    
    NIMEX_MACRO_POST_MEX
    
    return;
}
