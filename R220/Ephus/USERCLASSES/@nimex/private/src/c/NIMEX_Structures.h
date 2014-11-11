/**
 * @file NIMEX_Structures.h
 *
 * @brief Defines structures for containing all properties associated
 * with analog/digital input/output operations.
 *
 * A pointer to the NIMEX_TaskDefinition should be sufficient for
 * an entire acquisition.
 *
 * A set of pointers to NIMEX_ChannelDefinition structures should
 * be associated with an acquisition, these are the individual channel
 * settings, to be used in the overall task.
 *
 * NIMEX_LinkedList is a generic linked list structure, used as the superclass
 * for other linked list implementations.
 *
 * NIMEX_ChannelList is a linked list used for storing pointers to 
 * NIMEX_ChannelDefinition references. Adding/removing channels will
 * likely be a frequent operation, which is the motivation for using
 * something other than a simple array (using the STL might be desirable).
 *
 * NIMEX_CallbackSet defines a set of prioritized callbacks, with the
 * intent of them being processed in unspecified (but best effort) order
 * when the appropriate NIDAQmx event is spawned. It is, as of now, implemented
 * as a linked list (adding and removing may occur frequently.
 *
 * @author Timothy O'Connor
 * @date 11/16/06
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006</em>
 *
 */

/*
 *
 * Changes
 *  TO120806C: Support exporting of the clock signal on a specified terminal (clockExportTerminal). -- Tim O'Connor 12/8/06
 *  TO010607B: Created NIMEX_genericStructures. -- Tim O'Connor 1/6/07
 *  TO010607D: Created NIMEX_memManagement. -- Tim O'Connor 1/6/07
 *  TO012407A: Added userData field. -- Tim O'Connor 1/24/07
 *  TO012407B: Added mutex field for thread synchronization. -- Tim O'Connor 1/24/07
 *  TO012407D: Change dataBuffer type to void* to handle analog (64-bit float) and digital (unsigned 32-bit integer) data. -- Tim O'Connor 1/24/07
 *  TO012907C: Moved properties from the NIMEX_ChannelDefinition to the  NIMEX_TaskDefinition, to better align with NIDAQmx. -- Tim O'Connor 1/29/07
 *  TO012907E: Added a flag to indicate the start-status of a task. -- Tim O'Connor 1/29/07
 *  TO012907H: Allow user-control of the N in everyNSamples. -- Tim O'Connor 1/29/07
 *  TO022307B: Add RepeatOutput functionality. -- Tim O'Connor 2/23/07
 *  TO033007D: Added dataBufferSize to the NIMEX_ChannelDefinition structure. -- Tim O'Connor 3/30/07
 *  TO061207A: Port basic structures to glib. -- Tim O'Connor 6/12/07
 *  TO061407A: Implement events, preprocessors, and data listeners. -- Tim O'Connor 6/14/07
 *  TO061907D: Added state field to NIMEX_TaskDefinition. -- Tim O'Connor 6/19/07
 *  TO061907E: Replaced everyNCallbacks and doneCallbacks with eventMap, added autoDispatcher and autoDispatch. -- Tim O'Connor 6/19/07
 *  TO071807A: Added C-level cycle functionality. -- Tim O'Connor 7/18/07
 *  TO073070A: Added enable field to channels, allowing them to be defined but not started. -- Tim O'Connor 7/30/07
 *  TO073107E: Added counter/timer functionality. -- Tim O'Connor 7/31/07
 *  TO080107B: Replace the dataBuffer with a NIMEX_DataSource object. -- Tim O'Connor 8/1/07
 *  TO101007H: Changed mutex from a Windows mutex (of type HANDLE) to a CRITICAL_SECTION type. -- Tim O'Connor 10/10/07
 *  TO101207F - Use reference counting to prevent doneEvents being processed while other callbacks are outstanding. -- Tim O'Connor 10/12/07
 *  TO101707A: Created an atomic flag for use in critical code protection when the OS is insufficient. -- Tim O'Connor 10/17/07
 *  TO101707C: Mu! Back out TO101707A, for now, as it was based on an invalid test scenario. -- Tim O'Connor 10/17/07
 *  TO101807H: Added a done field to the taskDefinition. -- Tim O'Connor 10/18/07
 *  TO101907A: Automatically restart the task in the doneEvent, in the C-level, to get fast resets when in "external trigger" mode. -- Tim O'Connor 10/19/07
 *  TO102407A: Added forceFullBuffering, which requires dataSources to supply all data, including samples that are due to repeatOutput. -- Tim O'Connor 10/24/07
 *  TO051408A: Ported documentation to Doxygen. -- Tim O'Connor 5/14/08
 *  TO102508E: Added the sampleClockTimebaseSource property. -- Tim O'Connor 10/25/08
 *
 */

#ifndef _NIMEX_STRUCTURES_h /* Multiple include protection. */
#define _NIMEX_STRUCTURES_h

#include "nimex.h"

/**
 * @brief A representation of a Matlab callback.
 * Each callback is identified by a name, which should be globally unique, but need only be unique on a per-event basis.\n
 * Callbacks are prioritized, with lower numbers resulting in higher priorities.
 *
 * @todo Implement configurable, C-level callbacks.
 */
typedef struct
{
    ///Unique identifier for this callback.
    char*                   name;
    ///Priority for this callback, when firing the associated event. Lower numbers result in earlier execution.
    int32                   priority;// = 0;
    ///A valid Matlab function_handle, or a cell array with a function_handle as the first item.
    mxArray*                callbackToMatlab;//To be processed using feval.
} NIMEX_Callback;

/**
 * @brief A priority-sorted linked list of callback objects.
 */
typedef NIMEX_linkedList NIMEX_CallbackSet;

/**
 * @brief A mapping from event names to lists of callbacks.
 */
typedef NIMEX_hashTable NIMEX_CallbackMap;//TO061407A - New type. Each value must be a NIMEX_CallbackSet.

/**
 * @brief A mapping from event names to lists of callbacks.
 */
//TO071807
typedef struct
{
    ///The name of this data source. Usually the channel's name.
    char*                   name;
    ///A valid Matlab function_handle, or a cell array with a function_handle as the first item, which must return the output data for this channel.
    mxArray*                callback;//An executable callback.
    ///The channel's data, which may be either float64* or uint32*.
    void*                   dataBuffer;//TO012407D//Context specific: used for analog/digital input/output.
    ///The number of samples in dataBuffer, which does not account for the number of bytes per sample.
    uInt64                  dataBufferSize;//If the data source is a direct buffer, the size MUST be specified here.
    ///The channel's data type (a value from NIMEX_Constants.h).
    int                     dataSourceType;
} NIMEX_DataSource;

/**
 * @brief A definition of a NIDAQmx task-based channel.
 *
 * Channels carry mnemonic names, but are referred to by their physical name by convention. This may change in the future.\n
 * The dataBufferSize field is a remnant from the time before the existence of the NIMEX_DataSource object. This may be removed in the future.\n
 * The channelType only takes values from NIMEX_Constants.h.\n
 * The enable field allows a channel to be defined, yet remain dormant during certain executions of the task.
 */
typedef struct
{
    ///A 0 indicates the channel is disabled, non-zero otherwise.
    int32                   enable;//TO073070A
    ///The channel's type, using a value from NIMEX_Constants.h.
    int32                   channelType;
    ///Directly maps to the NIDAQmx terminalConfig. See the NIDAQmx documentation for details.
    int32                   terminalConfig;// = DAQmx_Val_InputTermCfg_Diff;
    ///Directly maps to the NIDAQmx units. See the NIDAQmx documentation for details. In practice, this will almost always be volts.
    int32                   units;// = DAQmx_Val_Volts;
    ///Sets the minimum value in the sensor's range.
    float64                 minVal;//= -10.0;
    ///Sets the maximum value in the sensor's range.
    float64                 maxVal;// = 10.0;
    ///A NIMEX_DataSource object, used to generate/process samples for this channel. @see NIMEX_DataSource.
    NIMEX_DataSource*       dataSource;//TO080107B
                            //TO073107E In the case of timer functionality, dataBuffer is an array of 3 float64 values (initialDelay, lowTime or frequency, highTime or dutyCycle).
    ///@deprecated This field has been subsumed into NIMEX_DataSource.
    uInt64                  dataBufferSize;//TO033007D: Good bookkeeping.
    ///A user-friendly name for the channel.
    char*                   mnemonicName;   //Essentially for display/ID purposes only, not for NIDAQmx use.
    ///The fully qualified NIDAQmx device specifier for this channel.
    char*                   physicalChannel;
} NIMEX_ChannelDefinition;

/**
 * @brief An unsorted linked list of channel definitions.
 */
typedef NIMEX_linkedList NIMEX_ChannelList;

/**
 * @brief The NIMEX analogue of a NIDAQmx TaskHandle.
 *
 * The actual NIDAQmx TaskHandle pointer is stored as a field in this struct.
 * All information necessary to define an entire acquisition is contained here.
 * The mutex field is used for task-level thread synchronization.
 * Field names tend to favor matching NIDAQmx terminology, but there may be some
 * influence from Matlab's daqtoolbox.
 *
 */
typedef struct 
{
    ///The underlying NIDAQmx task.
    TaskHandle*             taskHandle;
    ///The set of channels associated with this task.
    NIMEX_ChannelList*      channels;
    ///The source of the sample clock for this task.\n May be NULL. @see DAQmxCfgSampClkTiming in the NIDAQmx documentation.
    char*                   clockSource;
    ///Equivalent to the activeEdge argument in DAQmxCfgSampClkTiming.
    int32                   clockActiveEdge;// = DAQmx_Val_Rising;
    ///Export destination for this task's sample clock. @see DAQmxExportSignal @see DAQmxSetExportedSampClkOutputBehavior
    char*                   clockExportTerminal;
    ///Arbitrary user data, stored as a Matlab variable.
    mxArray*                userData;//TO012407A
    ///A platform-specific mutex, to allow for multithreading.
    #ifdef NIMEX_COMPILER_CL
    CRITICAL_SECTION*       mutex;//TO012407B
    #else
        #error "Mutex not currently defined for compilers other than CL."
    #endif
    ///A NIDAQmx terminal on which to accept a trigger signal. If NULL, no an immediate trigger is used.
    char*                   triggerSource;
    ///The task's tmeout, in seconds, for reads/writes. Defaults to NIMEX_DEFAULT_TIMEOUT.
    int32                   timeout;
    ///The NIDAQmx lineGrouping, which applies to the mapping of bits to digital lines. Defaults to DAQmx_Val_ChanForAllLines.
    int32                   lineGrouping;// = DAQmx_Val_ChanForAllLines;
    ///The subsystem's sampling rate, in samples per second.
    float64                 samplingRate;
    ///The subsystem's sample mode. May be: DAQmx_Val_FiniteSamps, DAQmx_Val_ContSamps, or DAQmx_Val_HWTimedSinglePoint
    int32                   sampleMode;//DAQmx_Val_FiniteSamps, DAQmx_Val_ContSamps, or DAQmx_Val_HWTimedSinglePoint 
    ///The number of samples to acquire per channel. This has implications for the configuration of buffers for continuously sampling tasks.
    uInt64                  sampsPerChanToAcquire;
    ///Hardware digital triggers may occur on either a rising or falling edge. Default: DAQmx_Val_Rising.
    int32                   triggerEdge;// = DAQmx_Val_Rising;
    ///The number of samples to collect before a trigger occurs.
    uInt32                  pretriggerSamples;// = 0;
    ///A flag which indicates if this task is currently started.
    int32                   started;//TO012907E
    ///A flag which indicates if this task is done. Once done, a task remains in this state until it is restarted.
    int32                   done;//TO101807H
    ///The number of samples at which to issue software interrupts, typically to generate/process some samples.
    uInt32                  everyNSamples;
    ///The number of times to repeat the output buffer(s), if this is an output task. This is ignored for input tasks.
    uInt32                  repeatOutput;//TO022307B
    ///An indicator of the task's current state. @see NIMEX_Constants.h.
    int32                   state;//TO061907D
    ///A generic binding of task-wide events to run-time configurable handlers.
    NIMEX_CallbackMap*      eventMap;//TO061907E
    ///The built-in handler for input samples.
    NIMEX_CallbackMap*      dataDispatcher;//TO061907E
    ///Per-channel data preprocessor functions.
    NIMEX_CallbackMap*      preprocessors;//TO061907E
    ///If 0, the dataDispatcher is not used, non-zero otherwise.
    int32                   autoDispatch;//TO061907E
    ///The idle state for a counter/timer task. See NIDAQmx documentation for details. @see DAQmxCreateCOPulseChanFreq @see DAQmxCreateCOPulseChanTime
    int32                   idleState;//TO073107E
    ///Matlab's main thread, used for cross thread communication.
    HANDLE                  matlabThread;//TO082707A
    ///For validation purposes, it may be useful to make sure Matlab's thread matches the matlabThread field, by comparing IDs.
    DWORD                   matlabThreadID;//TO082707A
    int32                   outstandingCallbackRefCount;
    //gint                    atomic;//TO101707A//TO101707C
    ///When non-zero, the task will automatically restart when the doneEvent is handled.
    int32                   autoRestart;//TO101907A
    ///For tasks with a repeatOutput greater than 0, if this flag is non-zero, each channel's NIMEX_dataSource must have all the samples buffered (sampsPerChanToAcqure * repeatOutput).
    int32                   forceFullBuffering;//TO102407A
    ///Specifies source terminal for the sample clock's timebase (typically a 20MHz signal), for cross-board/device synchronization. If NULL or an empty string, the onboard clock is used.
    char*                   sampleClockTimebaseSource;//TO102508E
} NIMEX_TaskDefinition;

/**
 * @brief Currently unused, may be deprecated in the future.
 */
typedef NIMEX_hashTable NIMEX_DataSourceMap;

/**
 * @brief Currently unused. C-level cycling may be based on this.
 */
typedef struct
{
    NIMEX_DataSourceMap*    dataSources;
    NIMEX_TaskDefinition**  taskDefinitions;//The same task definitions may be used for multiple positions, or differently configured ones may be used.
    int                     taskCount;
} NIMEX_CyclePosition;

/**
 * @brief Currently unused. C-level cycling may be based on this.
 */
typedef NIMEX_linkedList NIMEX_CyclePositionList;

/*//The recipient of the event must destroy the event when finished.
typedef struct
{
    NIMEX_TaskDefinition taskDefinition;
    int                  eventID;
    void*                eventData;//Context specific.
} NIMEX_Event;*/

/**
 * @brief Currently unused. C-level cycling may be based on this.
 */
typedef struct
{
    NIMEX_CallbackMap*      eventMap;
    int                     repetitions;
    int                     currentPosition;
    int                     totalPositionsStarted;//Total positions executed?
    HANDLE                  mutex;
    uInt32                  timeInterval;
    int                     repeatable;//boolean flag, true if the cycle can loop infinitely.
} NIMEX_Cycle;

#endif /* End multiple include protection. */
