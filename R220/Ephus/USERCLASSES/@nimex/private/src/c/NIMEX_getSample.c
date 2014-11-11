/**
 * @file NIMEX_getSample.c
 *
 * @brief Read a single 64-bit analog or 32-bit digital value from a task.
 *
 * The datatype that is read depends on the type of channel being measured.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_getSample(taskDefinition, channelName)</tt>
 *  @arg <tt>taskDefinition</tt> - The task from which to acquire a sample.
 *  @arg <tt>channelName</tt> - The physical name of the channel whose buffer to clear.
 *
 * @return A single sample from the specified channel, whose datatype is determined by the type of channel.
 *
 * @author Timothy O'Connor
 * @date 8/18/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 */
/*
 * CHANGES
 *
 */
#include "nimex.h"

#define DAQmxErrChk(functionCall) if (DAQmxFailed(error = (functionCall))) goto Error; else

/**
 * @brief Simple function for determining if something's a substring.
 * 
 * This should be available in various libraries, but I didn't feel like looking for it and linking it into the build.
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
    int32                       read;
    float64                     timeout = NIMEX_DEFAULT_TIMEOUT;
	char                        errBuff[2048] = {'\0'};
    int                         dims[2] = {1, 1};
    TaskHandle                  taskHandle = 0;
    char*                       channelName;
    int                         ai = 0;
    int                         dio = 0;

    NIMEX_MACRO_PRE_MEX
    
    if (nlhs > 1)
        mexErrMsgTxt("NIMEX_getSample: Only one return argument supported.");
    
    if ((nrhs < 2) || (nrhs > 3))
        mexErrMsgTxt("NIMEX_getSample: Only a task and channel name are accepted as arguments.");

    //Marshall arguments.
    channelName = mxArrayToString(prhs[1]);
    if (isSubstr(channelName, "/ai"))
        ai = 1;
    if (isSubstr(channelName, "/port"))
        dio = 1;

    NIMEX_MACRO_VERBOSE("NIMEX_getSample: \"%s\"\n", channelName)
    if (ai && dio)
    {
        mexPrintf("NIMEX_getSample: Ambiguous channel name \"%s\" (appears to be both an analog and a digital specifier).\n", channelName);
        mexErrMsgTxt("NIMEX_getSample: Failed to get sample.\n");
    }

    DAQmxErrChk(DAQmxCreateTask("", &taskHandle));

    if (ai)
    {
        //Analog.
        plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
        DAQmxErrChk(DAQmxCreateAIVoltageChan(taskHandle, channelName, "", DAQmx_Val_Default, -10.0, 10.0, DAQmx_Val_Volts, NULL));
        DAQmxErrChk(DAQmxStartTask(taskHandle));
        DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 1, 10.0, DAQmx_Val_GroupByChannel, (float64 *)mxGetPr(plhs[0]), 1, &read, NULL));
    }
    else if (dio)
    {
        //Digital.
        plhs[0] = mxCreateNumericArray(2, dims, mxUINT32_CLASS, mxREAL);
        DAQmxErrChk(DAQmxCreateDIChan(taskHandle, channelName, "", DAQmx_Val_ChanForAllLines));
        DAQmxErrChk(DAQmxStartTask(taskHandle));
        DAQmxErrChk(DAQmxReadDigitalU32(taskHandle, 1, 10.0, DAQmx_Val_GroupByChannel, (uInt32 *)mxGetPr(plhs[0]), 1, &read, NULL));
    }
    else
    {
        //Somebody upstairs screwed up.
        DAQmxClearTask(taskHandle);
        mexPrintf("NIMEX_getSample: Invalid channel \"%s\"\n", channelName);
        mexErrMsgTxt("NIMEX_getSample: Failed to get sample.\n");
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
		mexPrintf("NIMEX_getSample Error: %s\n", errBuff);
        mexErrMsgTxt("NIMEX_getSample: Failed to get sample.\n");
    }
 
    NIMEX_MACRO_POST_MEX

    return;
}
