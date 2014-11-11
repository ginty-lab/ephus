/**
 * @file NIMEX_Callbacks.c
 *
 * @brief Collection of NI, NI->Matlab, OS->Matlab callback functions.
 *
 * These functions dig a little bit into the inner workings of Matlab's user interface and "OS-level" GUI programming.
 * Matlab code must be executed in the Matlab thread, which requires some trickery, since there is no official mechanism
 * for putting asynchronous calls onto the thread's queue.
 *
 * It is important to understand the typical implementation of the code that underlies a GUI. Most GUIs operate by using
 * an event queue. A thread is started, which is in an infinite loop, waiting for events in the queue. As each event becomes
 * available, it gets serviced (which is event/platform specific), then proceeds to wait for more events. An example of how this
 * works on Windows is shown in \ref MessagePump01Figure "Figure 1".
 *
 * @anchor MessagePump01Figure
 * @image html MessagePump01.png "Figure 1 - Pseudocode/flowchart of the Windows message pump."
 *
 * In Windows, the method used is to employ the GUI/thread messaging system calls. An external event, typically the result
 * of a hardware interrupt that is initiated as an asynchronous procedure call in a NIDAQmx thread, is passed into the
 * Windows GUI main loop for the Matlab process. Inside the Matlab process, a hook function has been set up, which filters all
 * incoming events, and screens for NIMEX initiated events. When a non-NIMEX event is encountered, it is allowed to progress
 * through the normal processing flow. When a NIMEX event is encountered, it gets handled directly by the hook function
 * and is not processed any further. Windows provides a mechanism for getting unique identifiers for events, so as long as
 * all other event handling code follows the rules, it is guaranteed that NIMEX events will not conflict with other events.
 * \ref NIDAQmxEventFigure "Figure 2" and \ref NIDAQmxMultithreadedEventFigure "Figure 3" illustrate the flow of event dispatching in NIMEX in a Windows environment.
 *
 * @anchor NIDAQmxEventFigure
 * @image html NIDAQmxEvent.png "Figure 2 - NIMEX event dispatching in Windows."
 *
 * It is this necessary functionality, the dispatching of events, that allows NIMEX to cleanly implement multithreaded functionality
 * even though it interacts with Matlab, which does not explicity support multithreading. Figure 3 shows an example flow
 * of an event being handled across multiple threads.
 *
 * @anchor NIDAQmxMultithreadedEventFigure
 * @image html NIDAQmxMultithreadedEvent.png "Figure 3 - Multithreaded NIMEX event dispatching in Windows."
 *
 * @see RegisterWindowMessage
 * @see PostThreadMessage
 * @see CallNextHookEx
 * @see SetWindowsHookEx
 * @see http://msdn.microsoft.com/en-us/library/ms997537.aspx
 * @see http://msdn.microsoft.com/en-us/library/ms674854(VS.85).aspx
 * @see http://msdn.microsoft.com/en-us/library/ms674848(VS.85).aspx
 * @see http://msdn.microsoft.com/en-us/library/ms632589(VS.85).aspx
 *
 * @author Timothy O'Connor
 * @date 6/19/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 *
 */
/*
 * NOTES
 *  See TO061407A, TO061907B.
 *
 * CHANGES
 *  TO061907E: Replaced everyNCallbacks and doneCallbacks with eventMap, added dataDispatcher and autoDispatch. -- Tim O'Connor 6/19/07
 *  TO071107A - C89 compatibility for MSVC's cl compiler. -- Tim O'Connor 7/11/07
 *  TO073007B - Make sure the 'started' field is updated when a task completes. -- Tim O'Connor 7/30/07
 *  TO101007F - Automatically "stop" the task in the done event, to unreserve the hardware. -- Tim O'Connor 10/10/07
 *  TO101207F - Use reference counting to prevent doneEvents being processed while other callbacks are outstanding. -- Tim O'Connor 10/12/07
 *  TO101207G - Be vewy, vewy quiet... I'm huntin' wace condishwuns. -- Tim O'Connor 10/12/07
 *  TO101507A - Make sure to start from the root node and iterate over the entire preprocessor list. -- Tim O'Connor 10/15/07
 *  TO101507B - Don't auto-clear the task, because read operations may still be necessary. -- Tim O'Connor 10/15/07
 *  TO101607D - Moved stopping of the task out of Matlab's thread, for immediate processing. -- Tim O'Connor 10/16/07
 *  TO101707B - Check for NULL callbackData in the NIMEX_EveryN event handler, even though it should never occur. -- Tim O'Connor 10/17/07
 *  TO101707C - Back out TO101607D, for now, as it potentially screws up read commands in the everyN event. -- Tim O'Connor 10/17/07
 *  TO101807A - Print a verbose warning message and quit on error, when NIDAQmx event status is non-zero. -- Tim O'Connor 10/18/07
 *  TO101907A - Automatically restart the task in the doneEvent, to get fast resets when in "external trigger" mode. -- Tim O'Connor 10/19/07
 *  TO082908A - Use mexSetTrapFlag to prevent Matlab trying to terminate the mex file, should an error occur during a call to mexCalLMatlab. -- Tim O'Connor 8/29/08
 *  TO082908B - Free the prhs array when encountering an error during argument marshalling. This was a (typically) small memory leak. -- Tim O'Connor 8/29/08
 *  TO080813A - Switch mxFree to mxDestroyArray in a few points, this was elucidated by the freeing of a callback causing unpredictable crashing, after years of stability. -- Tim O'Connor 8/8/13
 */
#include "nimex.h"

#define DAQmxErrChk(functionCall) if (DAQmxFailed(error = (functionCall))) goto Error; else

/**
 * @brief Calls straight through to RegisterWindowMessage.
 *
 * Given an event name, this returns a unique number (a DWORD, aka 32-bit unsigned integer on Win32) to be used
 * as a message ID for passing thread messages.
 *
 * @arg <tt>eventName</tt> - The string that represents this event, as defined in NIMEX_Constants.h.
 * @return A unique DWORD value.
 *
 * @callgraph
 * @callergraph
 */
DWORD getWindowsMessageID(char* eventName)
{
    return RegisterWindowMessage(eventName);
}

//From NIMEX_Utilities.h
//TO061407A - Callback execution functions that operate over NIMEX_CallbackMap objects.
/**
 * @brief Generic event processing.
 *
 * Looks up handlers by the eventName and executes them.
 *
 * @arg <tt>table</tt> - A table that maps event names to event handlers.
 * @arg <tt>eventName</tt> - The string that represents this event, as defined in NIMEX_Constants.h.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_eventCallback(NIMEX_CallbackMap* table, const char* eventName)
{
    int32                error = 0;
    mxArray**            plhs = (mxArray**)NULL;
    mxArray**            prhs = (mxArray**)NULL;
    int                  nargs = 0;
    NIMEX_CallbackSet*   eventListeners = (NIMEX_CallbackSet *)NULL;
    NIMEX_Callback*      callback = (NIMEX_Callback *)NULL;
    int                  i = 0;//TO071107A

    NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/NIMEX_eventCallback: table=@%p, event=\"%s\"\n", table, eventName)
    eventListeners = NIMEX_CallbackMap_lookup(table, eventName);
    NIMEX_MACRO_VERBOSE_2("                                     listeners=@%p\n", eventListeners)

    //No listeners.
    if (eventListeners == NULL)
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/NIMEX_eventCallback: No callbacks found for \"%s\".\n", eventName)
        return;
    }
    if (NIMEX_linkedList_isempty(eventListeners))
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/NIMEX_eventCallback: No callbacks found for \"%s\".\n", eventName)
        return;
    }

    //Call all listeners.
    NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/NIMEX_eventCallback: Iterating over listeners...\n")
    eventListeners = NIMEX_CallbackSet_first(eventListeners);
    while (eventListeners != NULL)
    {
        callback = NIMEX_CallbackSet_value(eventListeners);
        //NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/NIMEX_eventCallback: callback=@%p\n", callback)
        if (callback != NULL)
        {
            NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/NIMEX_eventCallback: Marshalling listener @%p->\"%s\" for \"%s\"...\n", callback, callback->name, eventName)
            nargs = mxGetNumberOfElements(callback->callbackToMatlab);

            prhs = mxCalloc(nargs, sizeof(mxArray*));
//NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/NIMEX_eventCallback: mexFunctionName = %s\n", mexFunctionName())
//NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/NIMEX_eventCallback: callback = {")
            for (i = 0; i < nargs; i++)
            {
                prhs[i] = mxGetCell(callback->callbackToMatlab, i);
//NIMEX_MACRO_VERBOSE_2("%s, ", mxGetClassName(prhs[i]))
//NIMEX_MACRO_VERBOSE("mxGetClassName(callback{0}) = %s\n", mxGetClassName(prhs[i]))
//NIMEX_MACRO_VERBOSE("mxGetClassID(callback{0}) = %s\n", NIMEX_mxConstant_2_string(mxGetClassID(prhs[i])))
//NIMEX_MACRO_VERBOSE_2("@%p, ", prhs[i])
                if (prhs[i] == 0)
                {
                    mexPrintf("NIMEX_Callbacks/NIMEX_eventCallback: Error - Failed to marshall callback argument from cell array element %d.\n", i);
                    mxFree(prhs);//TO082908B
                    return;
                }
            }
//NIMEX_MACRO_VERBOSE_2("}\n")
            NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/NIMEX_eventCallback: Executing listener \"%s\" for \"%s\"...\n", callback->name, eventName)
            mexSetTrapFlag(1);//TO082908A
            error = mexCallMATLAB(0, plhs, nargs, prhs, "feval");//Execute Matlab function.
            mexSetTrapFlag(0);//TO082908A
            if (error)
                mexPrintf("NIMEX_Callbacks/NIMEX_eventCallback: Error - Failed to execute callback for \"%s\": error = %d.\n", callback->name, error);
            mxFree(prhs);
        }

        //Iterate.
        eventListeners = NIMEX_CallbackSet_next(eventListeners);
    }

    NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/NIMEX_eventCallback: Complete.\n")
    return;
}

/**
 * @brief Executes chains of per-channel data preprocessors.
 *
 * Looks up preprocessors for the named channel, and executes them in order, passing the data array to each,
 * while trying to reduce memory copies as much as possible.
 *
 * @arg <tt>table</tt> - A table that maps event names to event handlers.
 * @arg <tt>channelName</tt> - The physical channel name.
 * @arg <tt>returnValue</tt> - The initial data, which will get overwritten with the preprocessed data.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_preprocessorCallback(NIMEX_CallbackMap* table, const char* channelName, mxArray** returnValue)
{
    int32                error = 0;
    mxArray**            prhs = (mxArray**)NULL;
    int                  nargs = 0;
    int                  dims[2] = {0, 0};
    NIMEX_CallbackSet*   preprocessorChain = (NIMEX_CallbackSet *)NULL;
    NIMEX_Callback*      callback = (NIMEX_Callback *)NULL;
    int                  i = 0;//TO071107A
    mxArray*             result = (mxArray *)NULL;

    preprocessorChain = NIMEX_CallbackMap_lookup(table, channelName);

    //The data is wrapped in an mxArray, no manipulation's necessary.
    if ( (preprocessorChain == NULL) || (NIMEX_linkedList_isempty(preprocessorChain)) )
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_Utilities/NIMEX_preprocessorCallback: No preprocessors for channel '%s'.\n", channelName)
        return;
    }

    preprocessorChain = NIMEX_CallbackSet_first(preprocessorChain);//TO101507A - Make sure to iterate over the whole list. -- Tim O'Connor 10/15/07

    //Run the Matlab functions, presorted by priority.
    while (preprocessorChain != NULL)
    {
        callback = NIMEX_CallbackSet_value(preprocessorChain);
        NIMEX_MACRO_VERBOSE_3("NIMEX_Callbacks/NIMEX_preprocessorCallback: callback = @%p\n", callback)
        if (callback != NULL)
        {
            NIMEX_MACRO_VERBOSE_3("NIMEX_Callbacks/NIMEX_preprocessorCallback: Preparing callback...\n")
            nargs = mxGetNumberOfElements(callback->callbackToMatlab) + 1;//Add an extra argument, the data array.
            prhs = (mxArray **)mxCalloc(nargs, sizeof(mxArray*));
            for (i = 0; i < nargs - 1; i++)
            {
                prhs[i] = mxGetCell(callback->callbackToMatlab, i);
                if (prhs[i] == 0)
                {
                    mexPrintf("NIMEX_Utilities/NIMEX_preprocessorCallback: Error - Failed to marshall callback argument from cell array element %d for '%s'.\n", i, callback->name);
                    mxFree(prhs);//TO082908B - This should be mxFree not NIMEX_MACRO_FREE.
                    return;
                }
            }
            prhs[nargs - 1] = returnValue[0];
            NIMEX_MACRO_VERBOSE_3("NIMEX_Callbacks/NIMEX_preprocessorCallback: Executing callback...\n")
            mexSetTrapFlag(1);//TO082908A
            error = mexCallMATLAB(1, returnValue, nargs, prhs, "feval");//Execute Matlab function.
            mexSetTrapFlag(0);//TO082908A
            NIMEX_MACRO_VERBOSE_3("NIMEX_Callbacks/NIMEX_preprocessorCallback: Completed callback.\n")
            if (error)
            {
NIMEX_MACRO_VERBOSE("NIMEX_Utilities/NIMEX_preprocessorCallback: Error - Failed to execute callback for \"%s\".\n", callback->name);
                mexPrintf("NIMEX_Utilities/NIMEX_preprocessorCallback: Error - Failed to execute callback for \"%s\".\n", callback->name);
                mxFree(prhs);
                if (returnValue[0] != NULL)
                   mxDestroyArray(returnValue[0]);
                returnValue[0] = NULL;
                return;
            }
            mxFree(prhs);

            //TO121307C - Destroy the return value(s) that are getting preprocessed.
            if (result != NULL)
                mxDestroyArray(result);
            result = returnValue[0];
        }

NIMEX_MACRO_VERBOSE_4("returnValue[0] = @%p (in loop)\n", returnValue[0]);
        //Iterate.
        preprocessorChain = NIMEX_CallbackSet_next(preprocessorChain);
    }
    NIMEX_MACRO_VERBOSE_3("NIMEX_Callbacks/NIMEX_preprocessorCallback: Completed iterating over callbacks.\n")
    
    //TODO - TO073107A - See if the created mxArray needs to be duplicated, persistent, or freed at all.//TO121307C - The memory handling seems to be worked out now.
NIMEX_MACRO_VERBOSE_4("returnValue[0] = @%p\n", returnValue[0]);
NIMEX_MACRO_VERBOSE_4("Duplicating returnValue[0]...\n");
    returnValue[0] = mxDuplicateArray(returnValue[0]);
NIMEX_MACRO_VERBOSE_4("Making mxArray persistent...\n")
    mexMakeArrayPersistent(returnValue[0]);
NIMEX_MACRO_VERBOSE_4("Made mxArray persistent.\n")
NIMEX_MACRO_VERBOSE_4("Deleting result...\n")
    mxDestroyArray(result);
NIMEX_MACRO_VERBOSE_4("Deleted result.\n")
    NIMEX_MACRO_VERBOSE_3("NIMEX_Callbacks/NIMEX_preprocessorCallback: FINISHED!\n")
    return;
/*
	int32                error = 0;
    mxArray**            prhs = (mxArray**)NULL;
    int                  nargs = 0;
    int                  dims[2] = {0, 0};
    mxArray*             processedMxArray = (mxArray *)NULL;
    NIMEX_CallbackSet*   preprocessorChain = (NIMEX_CallbackSet *)NULL;
    NIMEX_Callback*      callback = (NIMEX_Callback *)NULL;
    int                  i = 0;//TO071107A
    mxArray*             clearable = (mxArray *)NULL;

    preprocessorChain = NIMEX_CallbackMap_lookup(table, channelName);

    //The data is wrapped in an mxArray, no manipulation's necessary.
    if ( (preprocessorChain == NULL) || (NIMEX_linkedList_isempty(preprocessorChain)) )
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_Utilities/NIMEX_preprocessorCallback: No preprocessors for channel '%s'.\n", channelName)
        return;
    }
    preprocessorChain = NIMEX_CallbackSet_first(preprocessorChain);//TO101507A - Make sure to iterate over the whole list. -- Tim O'Connor 10/15/07

    //Run the Matlab functions, presorted by priority.
    while (preprocessorChain != NULL)
    {
        callback = NIMEX_CallbackSet_value(preprocessorChain);
        NIMEX_MACRO_VERBOSE_3("NIMEX_Callbacks/NIMEX_preprocessorCallback: callback = @%p\n", callback)
        if (callback != NULL)
        {
            NIMEX_MACRO_VERBOSE_3("NIMEX_Callbacks/NIMEX_preprocessorCallback: Preparing callback...\n")
            nargs = mxGetNumberOfElements(callback->callbackToMatlab) + 1;//Add an extra argument, the data array.
            prhs = mxCalloc(nargs, sizeof(mxArray*));
            for (i = 0; i < nargs - 1; i++)
            {
                prhs[i] = mxGetCell(callback->callbackToMatlab, i);
                if (prhs[i] == 0)
                {
                    mexPrintf("NIMEX_Utilities/NIMEX_preprocessorCallback: Error - Failed to marshall callback argument from cell array element %d.\n", i);
                    return;
                }
            }
            prhs[nargs - 1] = returnValue[0];
            NIMEX_MACRO_VERBOSE_3("NIMEX_Callbacks/NIMEX_preprocessorCallback: Executing callback...\n")
            error = mexCallMATLAB(1, returnValue, nargs, prhs, "feval");//Execute Matlab function.
            if (error)
                mexPrintf("NIMEX_Utilities/NIMEX_preprocessorCallback: Error - Failed to execute callback for \"%s\".\n", callback->name);
            mxFree(prhs);
        }
        
        //Iterate.
        preprocessorChain = NIMEX_CallbackSet_next(preprocessorChain);
        if (preprocessorChain != NULL)
            clearable = returnValue[0];
    }
    NIMEX_MACRO_VERBOSE_3("NIMEX_Callbacks/NIMEX_preprocessorCallback: Duplicate array?\n")
    //TODO - TO073107A - See if the created mxArray needs to be duplicated, persistent, or freed at all.
    returnValue[0] = mxDuplicateArray(returnValue[0]);
    mexMakeArrayPersistent(returnValue[0]);
    NIMEX_MACRO_VERBOSE_3("NIMEX_Callbacks/NIMEX_preprocessorCallback: FINISHED!\n")

    return;
*/
}

/**
 * @brief Dispatches data to listeners.
 *
 * Reads the specified number of samples from the named channel, preprocesses the data,
 * and dispatches it to all registered listeners.
 *
 * @arg <tt>table</tt> - A table that maps event names to event handlers.
 * @arg <tt>channelName</tt> - The physical channel name.
 * @arg <tt>samples</tt> - 
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_samplesAcquiredCallback(NIMEX_CallbackMap* table, const char* channelName, mxArray* samples)
{
	int32                error = 0;
    mxArray**            plhs = (mxArray**)NULL;
    mxArray**            prhs = (mxArray**)NULL;
    int                  nargs = 0;
    NIMEX_CallbackSet*   listeners = (NIMEX_CallbackSet *)NULL;
    NIMEX_Callback*      callback = (NIMEX_Callback *)NULL;
    int                  i = 0;//TO071107A

    
    listeners = NIMEX_CallbackMap_lookup(table, channelName);
    if (NIMEX_linkedList_isempty(listeners))
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_Utilities/NIMEX_samplesAcquiredCallback: No listeners.\n");
        return;
    }

    while (listeners != NULL)
    {
        callback = NIMEX_CallbackSet_value(listeners);
        if (callback != NULL)
        {
            nargs = mxGetNumberOfElements(callback->callbackToMatlab) + 1;//Add an extra argument, the data array.
            prhs = mxCalloc(nargs, sizeof(mxArray*));
            for (i = 0; i < nargs - 1; i++)
            {
                prhs[i] = mxGetCell(callback->callbackToMatlab, i);
                if (prhs[i] == 0)
                {
                    mexPrintf("NIMEX_Utilities/NIMEX_samplesAcquiredCallback: Error - Failed to marshall callback argument from cell array element %d.\n", i);
                    mxFree(prhs);//TO082908B
                    return;
                }
            }
            prhs[nargs - 1] = samples;
            mexSetTrapFlag(1);//TO082908A
            error = mexCallMATLAB(0, plhs, nargs, prhs, "feval");//Execute Matlab function.
            mexSetTrapFlag(0);//TO082908A
            mxFree(prhs);
            if (error)
                mexPrintf("NIMEX_Utilities/NIMEX_samplesAcquiredCallback: Error - Failed to execute callback for \"%s\".\n", callback->name);
        }

        //Iterate.
        listeners = NIMEX_CallbackSet_next(listeners);
    }

    return;
}

/**
 * @brief Reads, preprocesses, and dispatches data to listeners.
 *
 * Reads the specified number of samples from the named channel, preprocesses the data,
 * and dispatches it to all registered listeners.
 *
 * @arg <tt>taskDefinition</tt> - The task to be auto-dispatched.
 * @arg <tt>nSamples</tt> - The number of samples, per channel, to be dispatched per interrupt.
 *
 * @todo This function is currently untested.
 *
 * @callgraph
 * @callergraph
 */
void autoDispatch(NIMEX_TaskDefinition* taskDefinition, uInt32 nSamples)
{
    mxArray*                    samples = (mxArray *)NULL;
    NIMEX_ChannelList*          channels = (NIMEX_ChannelList *)NULL;
    NIMEX_ChannelDefinition*    channel = (NIMEX_ChannelDefinition *)NULL;
    int                         runningChannelCount = 0;
    void*                       dataBuffer = NULL;
    int                         channelType = NIMEX_ANALOG_INPUT;
    int32                       classID = mxDOUBLE_CLASS;
    int32                       read;
    int32                       bufferOffset = 0;
    int32                       error = 0;
	char                        errBuff[2048] = {'\0'};
    int32                       sampleSize = 64;
    
    if (taskDefinition->channels == NULL)
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/autoDispatch: No channels...\n")
        return;
    }
    if (taskDefinition->dataDispatcher == NULL)
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/autoDispatch: No listeners...\n")
        return;
    }
        
    channels = NIMEX_ChannelList_first(taskDefinition->channels);
    while (channels != NULL)
    {
        channel = NIMEX_ChannelList_value(channels);
        if (channel != NULL)
        {
            if (channel->enable)
            {
                switch (channel->channelType)
                {
                    case NIMEX_ANALOG_INPUT:
                        channelType = NIMEX_ANALOG_INPUT;
                        classID = mxDOUBLE_CLASS;
                        sampleSize = sizeof(float64);
                        break;
                    case NIMEX_DIGITAL_INPUT:
                        channelType = NIMEX_DIGITAL_INPUT;
                        classID = mxUINT32_CLASS;
                        sampleSize = sizeof(uInt32);
                        break;
                    case NIMEX_ANALOG_OUTPUT:
                        mexPrintf("NIMEX_Callbacks/autoDispatch: Can not autodispatch data from analog output channel \"%s\".\n", channel->physicalChannel);
                        return;
                    case NIMEX_DIGITAL_OUTPUT:
                        mexPrintf("NIMEX_Callbacks/autoDispatch: Can not autodispatch data from digital output channel \"%s\".\n", channel->physicalChannel);
                        return;
                    default:
                        mexPrintf("NIMEX_Callbacks/autoDispatch: Unsupported channel type: %d.\n", channel->channelType);
                        return;
                }
                
                runningChannelCount++;
            }
        }
        
        channels = NIMEX_ChannelList_next(channels);
    }
    
    switch (channel->channelType)
    {
        case NIMEX_ANALOG_INPUT:
            dataBuffer = calloc(nSamples * runningChannelCount, sizeof(float64));
            NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/autoDispatch: Reading %d float64 samples per channel from task @%p.\n", nSamples, taskDefinition)
            DAQmxErrChk(DAQmxReadAnalogF64(*(taskDefinition->taskHandle), nSamples, taskDefinition->timeout, DAQmx_Val_GroupByChannel, dataBuffer, nSamples * runningChannelCount, &read, NULL));
            break;
        case NIMEX_DIGITAL_INPUT:
            dataBuffer = calloc(nSamples * runningChannelCount, sizeof(uInt32));
            NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/autoDispatch: Reading %d uInt32 samples per channel from task @%p.\n", nSamples, taskDefinition)
            DAQmxErrChk(DAQmxReadDigitalU32(*(taskDefinition->taskHandle), nSamples, taskDefinition->timeout, DAQmx_Val_GroupByChannel, dataBuffer, nSamples * runningChannelCount, &read, NULL));
            break;
    }

    channels = NIMEX_ChannelList_first(taskDefinition->channels);
    while (channels != NULL)
    {
        channel = NIMEX_ChannelList_value(channels);
        if (channel != NULL)
        {
            if (channel->enable)
            {
                NIMEX_wrapArray2mxArray(&samples, nSamples, classID, (void *)(((int)dataBuffer) + (bufferOffset * sampleSize)));
                bufferOffset += nSamples;

                //Preprocess.
                if (taskDefinition->preprocessors != NULL)
                {
                    NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/autoDispatch: Preprocessing data for channel \"%s\"...\n", channel->physicalChannel)
                    NIMEX_preprocessorCallback(taskDefinition->preprocessors, channel->physicalChannel, &samples);//Watch out for samples, there's indirection and persistence issues.
                    //TO102508F - Handle errors in preprocessor functions. -- Tim O'Connor 10/25/08
                    if (samples == NULL)
                    {
                       mexPrintf("NIMEX_Callbacks/autoDispatch: Error - Failed to execute preprocessor(s) for channel \"%s\".\n", channel->physicalChannel);
                       free(dataBuffer);
                       return;
                    }
                }

                //Dispatch to listeners.
                if (taskDefinition->dataDispatcher != NULL)
                {
                    NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/autoDispatch: Dispatching data to listeners for channel \"%s\"...\n", channel->physicalChannel)
                    NIMEX_samplesAcquiredCallback(taskDefinition->dataDispatcher, channel->physicalChannel, samples);
                }
                
                //TODO - TO073107A - See if the created mxArray needs to be duplicated, persistent, or freed at all.
                //mxDestroyArray(samples);//This was made persistent when returned from the preprocessor.
            }
        }
        
        channels = NIMEX_ChannelList_next(channels);
    }
    
Error:
	if (DAQmxFailed(error))
    {
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
        if (taskDefinition->taskHandle != NULL)
        {
            //Should we stop the task here???
            DAQmxStopTask(*(taskDefinition->taskHandle));
            //TO101507B - Don't auto-clear the task, because read operations may still be necessary. -- Tim O'Connor 10/15/07
            //DAQmxClearTask(*(taskDefinition->taskHandle));
            //NIMEX_freePersistentMemorySegment(taskDefinition->taskHandle);
            //taskDefinition->taskHandle = (TaskHandle *)NULL;
        }
	}

    mxDestroyArray(samples);//TO080813A
    free(dataBuffer);
    
    return;
}

//From NIMEX_startTask.c
//int32 CVICALLBACK processEveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void* callbackData)
/**
 * @brief Handles incoming everyN events in the Matlab thread.
 *
 * @arg <tt>taskDefinition</tt> - The task to be handled.
 * @arg <tt>nSamples</tt> - The number of samples, per channel, that generates the initiating interrupt.
 *
 * @callgraph
 * @callergraph
 */
void CALLBACK processEveryNCallbackInMatlabThread(NIMEX_TaskDefinition* taskDefinition, uInt32 nSamples)
{
    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);//TO012407B
    NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/processEveryNCallbackInMatlabThread: taskDefinition=@%p\n", taskDefinition)
    #ifdef NIMEX_VERBOSE
        NIMEX_displayTaskDefinition("    ", taskDefinition);
    #endif

    //TO061907E
    //No callbacks for this task.
    if (taskDefinition->autoDispatch)
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/processEveryNCallbackInMatlabThread: Auto-dispatching data...\n");
        autoDispatch(taskDefinition, nSamples);
        NIMEX_releaseLock(taskDefinition);//TO101207G
        return;
    }
    else if (taskDefinition->eventMap == NULL)
    {
        NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/processEveryNCallbackInMatlabThread: No Callbacks - @%p->eventMap == NULL\n", taskDefinition)
        NIMEX_releaseLock(taskDefinition);//TO012407B
        return;
    }

    //TO061407A
    NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/processEveryNCallbackInMatlabThread firing event...\n")
    NIMEX_eventCallback(taskDefinition->eventMap, NIMEX_EVENT_EVERY_N);//TO062007A

    NIMEX_releaseLock(taskDefinition);//TO012407B

    return;
}

//TO101907A
#define DAQmxRestartTaskHandleErrChk(functionCall) if (DAQmxFailed(error = (functionCall))) goto RestartTaskHandleError; else

//From NIMEX_startTask.c
//TO012407C: Implemented DoneCallback, after working out the kinks on EveryNSamplesCallback. -- Tim O'Connor 1/24/07
//int32 CVICALLBACK processDoneCallbackInMatlabThread(TaskHandle taskHandle, int32 status, void* callbackData)
/**
 * @brief Handles incoming done events in the Matlab thread.
 *
 * @arg <tt>taskDefinition</tt> - The task to be handled.
 * @arg <tt>status</tt> - The NIDAQmx status flag for the task, which is passed into the interrupt.
 *
 * @callgraph
 * @callergraph
 */
void CALLBACK processDoneCallbackInMatlabThread(NIMEX_TaskDefinition* taskDefinition, int32 status)
{
    int32 error = 0;
    char* errBuff;//TO101807A
    if (status < 0)
    {
        //TO101807A - Print a verbose warning message.
        errBuff = calloc(2048, sizeof(char));
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        mexPrintf("NIMEX_Callbacks/processDoneCallbackInMatlabThread - NIDAQmx status indicates warning: %s\n", errBuff);
        if (taskDefinition->autoRestart)
        {
            mexPrintf("NIMEX_Callbacks/processDoneCallbackInMatlabThread - Disabling autoRestart for this task.\n");
            taskDefinition->autoRestart = 0;
        }
        free(errBuff);
    }
    else if (status > 0)
    {
        //TO101807A - Print a verbose warning message. Quit callback processing on error.
        errBuff = calloc(2048, sizeof(char));
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        mexPrintf("NIMEX_Callbacks/processDoneCallbackInMatlabThread - NIDAQmx status indicates error: %d\n", errBuff);
        free(errBuff);
        NIMEX_eventCallback(taskDefinition->eventMap, NIMEX_EVENT_ERROR);
        DAQmxTaskControl((*taskDefinition->taskHandle), DAQmx_Val_Task_Abort);
        NIMEX_eventCallback(taskDefinition->eventMap, NIMEX_EVENT_ABORT);
        return;
    }

    status = 0;

    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);//TO012407B

    NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/processDoneCallbackInMatlabThread: taskDefinition=@%p\n", taskDefinition)
    NIMEX_MACRO_VERBOSE("NIMEX_Callbacks/processDoneCallbackInMatlabThread: \n")
    #ifdef NIMEX_VERBOSE
        NIMEX_displayTaskDefinition("    ", taskDefinition);
    #endif

    //NIMEX_MACRO_VERBOSE_2("NIMEX_Callbacks/processDoneCallbackInMatlabThread: Already stopped & unreserved taskHandle...\n");
    //taskDefinition->started = 0;//TO073007B

    taskDefinition->done = 1;//TO101807H

    //TO061407A
    if (taskDefinition->eventMap != NULL)
        NIMEX_eventCallback(taskDefinition->eventMap, NIMEX_EVENT_DONE);//TO062007A
    //status = NIMEX_implementCallback(taskDefinition, callback);//TO012407C
    
    // Obsoleted by TO101907A
    // //TO061907E
    ////No callbacks for this task.
    //if (taskDefinition->eventMap == NULL)
    //{
    //    NIMEX_releaseLock(taskDefinition);//TO012407B
    //}

    //TO101907A - Restart the task, if requested.
    if (taskDefinition->autoRestart)
    {
        if (taskDefinition->taskHandle == NULL)
        {
            //TO121707C - This statement will come up a lot, but is generally not harmful, for now...
            //mexPrintf("NIMEX_Callbacks/processDoneCallbackInMatlabThread: Warning - taskDefinition=@%p is set to autoRestart but the task has been explicitly stopped. Not restarting.\n", taskDefinition);
        }
        else
        {
            NIMEX_MACRO_VERBOSE("NIMEX_Callbacks/processDoneCallbackInMatlabThread: Automatically restarting task @%p...\n", taskDefinition)
            DAQmxRestartTaskHandleErrChk(DAQmxStopTask(*(taskDefinition->taskHandle)));
            DAQmxRestartTaskHandleErrChk(NIMEX_CommitTask(taskDefinition));
            DAQmxRestartTaskHandleErrChk(DAQmxStartTask(*(taskDefinition->taskHandle)));
        }
    }

RestartTaskHandleError:
    if (DAQmxFailed(error))
    {
        errBuff = calloc(2048, sizeof(char));
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        if (taskDefinition->taskHandle != NULL)
        {
            DAQmxStopTask(*(taskDefinition->taskHandle));
            DAQmxClearTask(*(taskDefinition->taskHandle));
            NIMEX_freePersistentMemorySegment(taskDefinition->taskHandle);
            taskDefinition->taskHandle = (TaskHandle *)NULL;
        }
        mexPrintf("NIMEX_Callbacks/processDoneCallbackInMatlabThread: Error - Failed to autoRestart task: %s\n", errBuff);
    }

    NIMEX_releaseLock(taskDefinition);//TO012407B

    return;
}

/**
 * @brief Handles incoming everyN events in an alertable NIDAQmx thread.
 *
 * Because this is a kernel alertable thread, and outside our control, memory can not be allocated or released here.
 *
 * @arg <tt>taskHandle</tt> - The NIDAQmx task to be handled.
 * @arg <tt>everyNsamplesEventType</tt> - A NIDAQmx flag, which we don't really need.
 * @arg <tt>nSamples</tt> - The number of samples that were reached to initiate the interrupt.
 * @arg <tt>callbackData</tt> - The NIMEX task to be handled.
 *
 * @callgraph
 * @callergraph
 */
int32 CVICALLBACK NIMEX_EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void* callbackData)
{
    //TO101707B - The callbackData should never be NULL, but we might as well check here, for uniformity with NIMEX_DoneCallback, and not bother processing corrupted callbacks.
    if (callbackData != NULL)
    {
        //Pump the EveryNCallback event safely into the Matlab thread.
        NIMEX_EnterGlobalCriticalSection();//Synchronize to keep callbacks in order.
        PostThreadMessage(NIMEX_getMatlabThreadId(), getWindowsMessageID(NIMEX_WINDOWS_MESSAGE_EVENT_EVERYN), (WPARAM)callbackData, (LPARAM)nSamples);
        NIMEX_LeaveGlobalCriticalSection();
        SwitchToThread();//Yield processor.
    }

    return 0;
}

/**
 * @brief Handles incoming done events in an alertable NIDAQmx thread.
 *
 * Because this is a kernel alertable thread, and outside our control, memory can not be allocated or released here.
 *
 * @arg <tt>taskHandle</tt> - The NIDAQmx task to be handled.
 * @arg <tt>status</tt> - The NIDAQmx task status flag.
 * @arg <tt>callbackData</tt> - The NIMEX task to be handled.
 *
 * @callgraph
 * @callergraph
 */
int32 CVICALLBACK NIMEX_DoneCallback(TaskHandle taskHandle, int32 status, void* callbackData)
{
    //DAQmxTaskControl(taskHandle, DAQmx_Val_Task_Unreserve);//This will invalidate any future calls to read, so it's not cool.
    //DAQmxClearTask(*(taskDefinition->taskHandle));//TO101007F
    //TO101607E - Always process the done event, even if no callbacks are needed. Fire off the callback in the Matlab thread, if necessary.
    if (callbackData != NULL)
    {
        //Pump the DoneCallback event safely into the Matlab thread.
        NIMEX_EnterGlobalCriticalSection();//Synchronize to keep callbacks in order.
        //DAQmxStopTask(taskHandle);//TO101007F//TO101607D//TO101707C
        PostThreadMessage(NIMEX_getMatlabThreadId(), getWindowsMessageID(NIMEX_WINDOWS_MESSAGE_EVENT_DONE), (WPARAM)callbackData, (LPARAM)status);
        NIMEX_LeaveGlobalCriticalSection();
        SwitchToThread();//Yield processor.
    }
    else
    {
        //TO101707C - Back this out, for now.
        //TO101607D - Moved stopping of the task out of Matlab's thread, for immediate processing.
        //            This must still be atomic, because the task can't be stopped during an everyN's read operation.
        //NIMEX_EnterGlobalCriticalSection();//Synchronize to keep callbacks in order.
        //DAQmxStopTask(taskHandle);//TO101007F
        //NIMEX_LeaveGlobalCriticalSection();
    }

    return status;
}

/**
 * @brief Handles incoming Windows multimedia timer events in an alertable Windows timer thread.
 *
 * Because this is a kernel alertable thread, and outside our control, memory can not be allocated or released here.
 *
 * @arg <tt>taskDefinition</tt> - The NIMEX task to be handled.
 * @arg <tt>wTimerID</tt> - The handle/ID of the timer that called this function.
 *
 * @todo Implement processTimerCallbackInMatlabThread for Windows Multimedia Timer support.
 *
 * @callgraph
 * @callergraph
 */
void processTimerCallbackInMatlabThread(NIMEX_TaskDefinition* taskDefinition, UINT wTimerID)
{
    //TODO
    return;
}

/**
 * @brief Handles incoming Windows multimedia timer events in an alertable Windows timer thread.
 *
 * Because this is a kernel alertable thread, and outside our control, memory can not be allocated or released here.
 *
 * @arg <tt>wTimerID</tt> - The ID of the timer that called this function.
 * @arg <tt>msg</tt> - Reserved; do not use.
 * @arg <tt>userData</tt> - The NIMEX task to be handled.
 * @arg <tt>dw1</tt> - Reserved; do not use.
 * @arg <tt>dw2</tt> - Reserved; do not use.
 *
 * @see http://msdn.microsoft.com/en-us/library/ms712704(VS.85).aspx
 *
 * @callgraph
 * @callergraph
 */
void CALLBACK NIMEX_TimerEventCallback(UINT wTimerID, UINT msg, DWORD userData, DWORD dw1, DWORD dw2)
{
    //Pump the DoneCallback event safely into the Matlab thread.
    NIMEX_EnterGlobalCriticalSection();//Synchronize to keep callbacks in order.
    PostThreadMessage(NIMEX_getMatlabThreadId(), getWindowsMessageID(NIMEX_WINDOWS_MESSAGE_EVENT_TIMER), (WPARAM)userData, (LPARAM)wTimerID);
    NIMEX_LeaveGlobalCriticalSection();
    SwitchToThread();//Yield processor.

    return;
}

/**
 * @brief Filters GUI events for events that originated from NIMEX and dispatches them.
 *
 * This is the function that handles all the magic.
 *
 * @arg <tt>code</tt> - The ID of the timer that called this function.
 * @arg <tt>wParam</tt> - The NIMEX task to be handled.
 * @arg <tt>lParam</tt> - Event specific data.
 *
 * @callgraph
 * @callergraph
 */
LRESULT CALLBACK NIMEX_CallbackMessagePumpHook(int code, WPARAM wParam, LPARAM lParam)
{
    LRESULT result;
    MSG* msg;
    msg = (MSG *)lParam;
    if (code == HC_ACTION)
    {
        if ( (wParam != PM_NOREMOVE) )
        {
            //Dispatch the event here.
            if ( msg->message == getWindowsMessageID(NIMEX_WINDOWS_MESSAGE_EVENT_EVERYN) )
            {
                //Globally synchronize callback processing, so callbacks do not interrupt each other.
                NIMEX_EnterGlobalCriticalSection();
                processEveryNCallbackInMatlabThread((NIMEX_TaskDefinition *)msg->wParam, (uInt32)msg->lParam);
                NIMEX_LeaveGlobalCriticalSection();
            }
            else if ( msg->message == getWindowsMessageID(NIMEX_WINDOWS_MESSAGE_EVENT_DONE) )
            {
                //Globally synchronize callback processing, so callbacks do not interrupt each other.
                NIMEX_EnterGlobalCriticalSection();
                processDoneCallbackInMatlabThread((NIMEX_TaskDefinition *)msg->wParam, (int32)msg->lParam);
                NIMEX_LeaveGlobalCriticalSection();
            }
            else if ( msg->message == getWindowsMessageID(NIMEX_WINDOWS_MESSAGE_EVENT_TIMER) )
            {
                //Globally synchronize callback processing, so callbacks do not interrupt each other.
                NIMEX_EnterGlobalCriticalSection();
                processTimerCallbackInMatlabThread((NIMEX_TaskDefinition *)msg->wParam, (UINT)msg->lParam);
                NIMEX_LeaveGlobalCriticalSection();
            }
            else
                result = CallNextHookEx(NULL, code, wParam, lParam);//Pass on message to other hooks, it's not a NIMEX message.
            result = 0;//Don't process further hooks.
        }
        else
            result = CallNextHookEx(NULL, code, wParam, lParam);//This is a peek operation, let it slide.
    }
    else
        result = CallNextHookEx(NULL, code, wParam, lParam);

    SwitchToThread();//Yield processor.

    return result;
}

/**
 * @brief Sets up cross thread communication for the Matlab thread.
 *
 * This must only be called from within the Matlab thread. Otherwise it
 * will not necessarily be attaching the hook function to the GUI's main thread.
 *
 * @post NIMEX_CallbackMessagePumpHook is registered as a hook function.
 * @post Message IDs are initialized.
 *
 * @callgraph
 * @callergraph
 */
void NIMEX_initializeCallbackProcessing()
{
    if (NIMEX_getWindowsHookId() == NULL)
        NIMEX_setWindowsHookEx(NIMEX_CallbackMessagePumpHook);

    //Force initialization.
    getWindowsMessageID(NIMEX_WINDOWS_MESSAGE_EVENT_EVERYN);
    getWindowsMessageID(NIMEX_WINDOWS_MESSAGE_EVENT_DONE);
    getWindowsMessageID(NIMEX_WINDOWS_MESSAGE_EVENT_TIMER);
}
