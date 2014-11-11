/**
 * @file NIMEX_createTask.c
 *
 * @brief Allocations & initializes a NIMEX_TaskDefinition structure.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_createTask</tt>
 * @return A pointer to the structure as a MATLAB double, however this should be considered opaque.
 *
 * @author Timothy O'Connor
 * @date 11/16/06
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006</em>
 */
/*
 *
 * Changes
 *  TO120806C: Support exporting of the clock signal on a specified terminal. -- Tim O'Connor 12/8/06
 *  TO010707A: Created NIMEX_displayTaskDefinition & NIMEX_displayChannelDefinition. -- Tim O'Connor 1/7/07
 *  TO012407B: Added NIMEX_acquireLock & NIMEX_releaseLock for thread safety. -- Tim O'Connor 1/24/07
 *  TO012907C: Moved properties from the NIMEX_ChannelDefinition to the  NIMEX_TaskDefinition, to better align with NIDAQmx. -- Tim O'Connor 1/29/07
 *  TO012907E: Added a flag to indicate the start-status of a task. -- Tim O'Connor 1/29/07
 *  TO012907H: Allow user-control of the N in everyNSamples. -- Tim O'Connor 1/29/07
 *  TO022807C - Added mexFunction pre and post macros. -- Tim O'Connor 2/28/07
 *  TO061907D: Added state field to NIMEX_TaskDefinition. -- Tim O'Connor 6/19/07
 *  TO061907E: Replaced everyNCallbacks and doneCallbacks with eventMap, added autoDispatcher and autoDispatch. -- Tim O'Connor 6/19/07
 *  TO073107E: Added counter/timer functionality. -- Tim O'Connor 7/31/07
 *  TO101707A: Created an atomic flag for use in critical code protection when the OS is insufficient. -- Tim O'Connor 10/17/07
 *  TO101707C: Mu! Back out TO101707A, for now, as it was based on an invalid test scenario. -- Tim O'Connor 10/17/07
 *  TO102508E: Added the sampleClockTimebaseSource property. -- Tim O'Connor 10/25/08
 *
 */
#include "nimex.h"

/**
 * @brief Matlab entry point.
 * @callgraph
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	int32       error=0;
	NIMEX_TaskDefinition*  taskDefinition = (NIMEX_TaskDefinition *)0x00;

    NIMEX_MACRO_PRE_MEX
    
    if (nlhs != 1)
        mexErrMsgTxt("NIMEX_createTask outputs 1 (and only 1) argument.");

    NIMEX_MACRO_VERBOSE("NIMEX_createTask: Initializing new NIMEX_TaskDefinition instance.\n")
    //Use NIMEX library to allocate and register memory to be cleared upon exit.
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_Persistent_Calloc(1, sizeof(NIMEX_TaskDefinition));
    
    if (taskDefinition)
    {
        NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);//TO012407B
        //Initialize...
        taskDefinition->taskHandle = NULL;
        taskDefinition->channels = NULL;
        taskDefinition->clockSource = NIMEX_memcpy(&"", sizeof(char));//Defaults to the internal clock.
        NIMEX_MACRO_VERBOSE("NIMEX_createTask: Defaulted clock source: @%d->\"%s\"\n", (long)taskDefinition->clockSource, taskDefinition->clockSource)
        taskDefinition->clockActiveEdge = DAQmx_Val_Rising;
        taskDefinition->clockExportTerminal = NULL;//TO120806C
        //Compiling with MSVC++'s cl causes a segfault when trying to assign a 64-bit literal. -- Tim O'Connor 7/11/07
        taskDefinition->samplingRate = NIMEX_UNINITIALIZED;//No default. Flag that this has not been initialized.
        taskDefinition->sampleMode = NIMEX_UNINITIALIZED;//No default. Flag that this has not been initialized.
        taskDefinition->sampsPerChanToAcquire = NIMEX_MACRO_UINT64_LITERAL(0x7777777777777777);//No default. Flag that this has not been initialized.//TO100907A
        taskDefinition->triggerEdge = DAQmx_Val_Rising;
        taskDefinition->pretriggerSamples = 0;
        taskDefinition->lineGrouping = DAQmx_Val_ChanForAllLines;
        taskDefinition->timeout = NIMEX_DEFAULT_TIMEOUT;//TO120806D
        taskDefinition->triggerSource = (char *)NIMEX_memcpy(&"", sizeof(char));//This must be initialized later!
        taskDefinition->started = 0;//TO012907E
        taskDefinition->everyNSamples = NIMEX_UNINITIALIZED;//TO12907H
        taskDefinition->state = NIMEX_STATE_NEW;//TO061907D
        taskDefinition->eventMap = (NIMEX_CallbackMap *)NULL;//TO061907E
        taskDefinition->dataDispatcher = (NIMEX_CallbackMap *)NULL;//TO061907E
        taskDefinition->autoDispatch = 0;//TO061907E
        taskDefinition->idleState = DAQmx_Val_Default;//TO073107E
        //TO082707A
        DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), 
            &taskDefinition->matlabThread, 0, FALSE, DUPLICATE_SAME_ACCESS);//Hang on to Matlab's thread's handle.
        taskDefinition->matlabThreadID = GetCurrentThreadId();
        //g_atomic_int_set(&(taskDefinition->atomic), 0);//TO101707A//TO101707C
        //hookID = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)figureClosingBug_GetMsgProc, NULL, GetCurrentThreadId());
        taskDefinition->sampleClockTimebaseSource = NULL;//TO102508E
        
        NIMEX_MACRO_VERBOSE("NIMEX_createTask: \n")
        #ifdef NIMEX_VERBOSE
            NIMEX_displayTaskDefinition("\t", taskDefinition);
        #endif

        //Return the pointer to Matlab.
        plhs[0] = NIMEX_packPointer(taskDefinition);
        //plhs[0] = mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL);
        //mxSetPr(plhs[0], (double *)taskDefinition);
        //mxSetM(plhs[0], 1);
        //mxSetN(plhs[0], 1);
        NIMEX_releaseLock(taskDefinition);//TO012407B
    }
    else
        mexErrMsgTxt("NIDAQmex: Failed to instantiate NIMEX_TaskDefinition.");
    
    NIMEX_MACRO_POST_MEX
    
    //Make sure any memory is properly cleaned up when the NIDAQmex library is unloaded from MATLAB.
    mexAtExit(NIMEX_mexAtExit);

    return;
}
