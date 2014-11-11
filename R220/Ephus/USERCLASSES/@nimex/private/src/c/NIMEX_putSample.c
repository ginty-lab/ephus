/**
 * @file NIMEX_putSample.c
 *
 * @brief Determines if a task is configured for output.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_putSample(taskDefinition, channelName, sample)</tt>
 *  @arg <tt>taskDefinition</tt> - The task to be tested for input/output.
 *  @arg <tt>channelName</tt> - The physical channel on which to output a sample. The channel must be configured already.
 *  @arg <tt>sample</tt> - A single sample, the data type depends on the type of channel.
 *
 * @author Timothy O'Connor
 * @date 8/14/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 */
/*
 * CHANGES
 *
 */
#include "nimex.h"

#define DAQmxErrChk(functionCall) if (DAQmxFailed(error = (functionCall))) goto Error; else
/*
 * @see NIMEX_getSample.c
 *
 * @todo Refactor this substring search out (it also appears in NIMEX_getSample.c) or use a function in an external library.
 */
int isSubstr(char* parent, char* child)
{
    int i = 0;
    int j = 0;
    int childLen = 0;
    
    childLen = strlen(child);
    
    for (i = 0; i < strlen(parent) - childLen; i++)
    {
        for (j = 0; j < childLen; j++)
        {
            if (parent[i + j] == child[j])
            {
                if (j == childLen - 1)//They all match, we've found a substring.
                    return 1;
            }
            else
                break;//parent[i + j] doesn't match child[j], so we're not a substring on this alignment.
        }
    }
    
    return 0;
}

/**
 * @brief Matlab entry point.
 * @callgraph
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    int32                       error = 0;
    int32                       written;
    float64                     timeout = NIMEX_DEFAULT_TIMEOUT;
	char                        errBuff[2048] = {'\0'};
    TaskHandle                  taskHandle = 0;
    char*                       channelName;
    int                         ao = 0;
    int                         dio = 0;

    NIMEX_MACRO_PRE_MEX
    
    if (nlhs > 1)
        mexErrMsgTxt("NIMEX_putSample: Only one return argument (number of samples written) supported.");
    
    if (nrhs < 3)
        mexErrMsgTxt("NIMEX_putSample: The arguments must be an array of data and a number of samples per channel.");

    

    //Marshall arguments.
    channelName = mxArrayToString(prhs[1]);
    if (isSubstr(channelName, "/ao"))
        ao = 1;
    if (isSubstr(channelName, "/port"))
        dio = 1;

    NIMEX_MACRO_VERBOSE("NIMEX_putSample: \"%s\"\n", channelName)
    if (ao && dio)
    {
        mexPrintf("NIMEX_putSample: Ambiguous channel name \"%s\" (appears to be both an analog and a digital specifier).\n", channelName);
        mexErrMsgTxt("NIMEX_putSample: Failed to put sample.\n");
    }

    DAQmxErrChk(DAQmxCreateTask("", &taskHandle));
    
    if (ao)
    {
        //Analog.
        DAQmxErrChk(DAQmxCreateAOVoltageChan(taskHandle, channelName, "", -10.0, 10.0, DAQmx_Val_Volts, ""));
        DAQmxErrChk(DAQmxStartTask(taskHandle));
        DAQmxErrChk(DAQmxWriteAnalogF64(taskHandle, 1, 1, 10.0, DAQmx_Val_GroupByChannel,(float64 *)mxGetPr(prhs[2]), NULL, NULL));
    }
    else if (dio)
    {
        //Digital.
        DAQmxErrChk(DAQmxCreateDOChan(taskHandle, channelName, "", DAQmx_Val_ChanForAllLines));
        DAQmxErrChk(DAQmxStartTask(taskHandle));
        DAQmxErrChk(DAQmxWriteDigitalU32(taskHandle, 1, 1, 10.0, DAQmx_Val_GroupByChannel,(uInt32 *)mxGetPr(prhs[2]), NULL, NULL));
    }
    else
    {
        //Somebody upstairs screwed up.
        DAQmxClearTask(taskHandle);
        mexPrintf("NIMEX_putSample - Error: Invalid channel \"%s\"\n", channelName);
        mexErrMsgTxt("NIMEX_putSample: Failed to put sample.\n");
    }
        
Error:
	if (DAQmxFailed(error))
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
	if (taskHandle != 0)
    {
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if (DAQmxFailed(error))
    {
		mexPrintf("NIMEX_putSample - Error: %s\n", errBuff);
        mexErrMsgTxt("NIMEX_putSample: Failed to put sample.\n");
    }
 
    NIMEX_MACRO_POST_MEX
    
    return;
}
