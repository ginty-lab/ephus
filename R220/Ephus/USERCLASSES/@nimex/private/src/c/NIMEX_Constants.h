/**
 * @file NIMEX_Constants.h
 *
 * @brief All the constants used in the nimex library should be defined here.
 *
 * @author Timothy O'Connor
 * @date 11/20/06
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006</em>
 *
 * @todo Possibly switch to using enumerations.
 *
 */

/*
 * CHANGES
 *  TO061907D: Added state field to NIMEX_TaskDefinition. -- Tim O'Connor 6/19/07
 *  TO071807A: Added C-level cycle functionality. -- Tim O'Connor 7/18/07
 *  TO073107E: Added counter/timer functionality. -- Tim O'Connor 7/31/07
 *  TO080107A: Re-engineered constants and added more conversions. -- Tim O'Connor 8/1/07
 *  TO101807B: New events for abort and error. -- Tim O'Connor 10/18/07
 *
 */

#ifndef _NIMEX_CONSTANTS_h /* Multiple include protection. */
#define _NIMEX_CONSTANTS_h

    ///Standard default value.
    #define NIMEX_UNINITIALIZED                     -1

    ///Analog input for NIMEX_ChannelDefinition->channelType.
    #define NIMEX_ANALOG_INPUT                      100
    ///Analog output for NIMEX_ChannelDefinition->channelType.
    #define NIMEX_ANALOG_OUTPUT                     101
    ///Digital input for NIMEX_ChannelDefinition->channelType.
    #define NIMEX_DIGITAL_INPUT                     102
    ///Digital output for NIMEX_ChannelDefinition->channelType.
    #define NIMEX_DIGITAL_OUTPUT                    103
    ///Frequency based counter/timer for NIMEX_ChannelDefinition->channelType.
    #define NIMEX_CO_FREQUENCY                      104//TO073107E
    ///Time based counter/timer for NIMEX_ChannelDefinition->channelType.
    #define NIMEX_CO_TIME                           105//TO073107E

    ///Timeout, for use with reads/writes, in seconds.
    #define NIMEX_DEFAULT_TIMEOUT                   10.0
    ///Timeout, for use with mutex waits, in seconds.
    #define NIMEX_DEFAULT_THREAD_TIMEOUT            10000L
    
    //TO061907D
    ///NIMEX_TaskDefinition->state flag: New
    #define NIMEX_STATE_NEW                         257//00000001 00000001
    ///NIMEX_TaskDefinition->state flag: Ready
    #define NIMEX_STATE_READY                       258//00000001 00000010
    ///NIMEX_TaskDefinition->state flag: Waiting For Trigger
    #define NIMEX_STATE_WAITING_FOR_TRIGGER         260//00000001 00000100
    ///NIMEX_TaskDefinition->state flag: Running
    #define NIMEX_STATE_RUNNING                     264//00000001 00001000
    ///NIMEX_TaskDefinition->state flag: Paused
    #define NIMEX_STATE_PAUSED                      272//00000001 00010000
    ///NIMEX_TaskDefinition->state flag: Dispatching
    #define NIMEX_STATE_DISPATCHING                 288//00000001 00100000
    ///NIMEX_TaskDefinition->state flag: Waiting For Event
    #define NIMEX_STATE_WAITING_FOR_EVENT           320//00000001 01000000
    
    ///NIMEX event name: done
    #define NIMEX_EVENT_DONE                        "done"
    ///NIMEX event name: everyN
    #define NIMEX_EVENT_EVERY_N                     "everyN"
    ///NIMEX event name: cyclePositionBegin
    #define NIMEX_EVENT_CYCLE_POSITION_BEGIN        "cyclePositionBegin"
    ///NIMEX event name: cyclePositionEnd
    #define NIMEX_EVENT_CYCLE_POSITION_END          "cyclePositionEnd"
    ///NIMEX event name: cycleStart
    #define NIMEX_EVENT_CYCLE_START                 "cycleStart"
    ///NIMEX event name: cycleStop
    #define NIMEX_EVENT_CYCLE_STOP                  "cycleStop"
    ///NIMEX event name: cycleComplete
    #define NIMEX_EVENT_CYCLE_COMPLETE              "cycleComplete"
    ///NIMEX event name: abort
    #define NIMEX_EVENT_ABORT                       "abort"
    ///NIMEX event name: error
    #define NIMEX_EVENT_ERROR                       "error"

    ///NIMEX data source type indicator: float64
    #define NIMEX_DATASOURCE_TYPE_FLOAT64           301//100101101
    ///NIMEX data source type indicator: uint32
    #define NIMEX_DATASOURCE_TYPE_UINT32            302//100101110
    ///NIMEX data source type indicator: callback
    #define NIMEX_DATASOURCE_TYPE_CALLBACK          316//100111100
    ///NIMEX data source type bitmask: float64
    #define NIMEX_DATASOURCE_TYPE_FLOAT64_BITMASK   1  //000000001
    ///NIMEX data source type bitmask: float64
    #define NIMEX_DATASOURCE_TYPE_UINT32_BITMASK    2  //000000010
    ///NIMEX data source type bitmask: float64
    #define NIMEX_DATASOURCE_TYPE_CALLBACK_BITMASK  16 //000010000
    ///NIMEX data source type indicator: float64 callback
    #define NIMEX_DATASOURCE_TYPE_FLOAT64CALLBACK   317//NIMEX_DATASOURCE_TYPE_FLOAT64 & NIMEX_DATASOURCE_TYPE_CALLBACK
    ///NIMEX data source type indicator: uint32 callback
    #define NIMEX_DATASOURCE_TYPE_UINT32CALLBACK    318//NIMEX_DATASOURCE_TYPE_UINT32 & NIMEX_DATASOURCE_TYPE_CALLBACK

    ///Windows message event: everyN
    #define NIMEX_WINDOWS_MESSAGE_EVENT_EVERYN      "NIMEX_WINDOWS_MESSAGE_EVENT_EVERYN"
    ///Windows message event: done
    #define NIMEX_WINDOWS_MESSAGE_EVENT_DONE        "NIMEX_WINDOWS_MESSAGE_EVENT_DONE"
    ///Windows message event: timer
    #define NIMEX_WINDOWS_MESSAGE_EVENT_TIMER       "NIMEX_WINDOWS_MESSAGE_EVENT_TIMER"

    ///NIMEX callback event flag: everyN
    #define NIMEX_CALLBACK_EVENT_EVERYN             400
    ///NIMEX callback event flag: done
    #define NIMEX_CALLBACK_EVENT_DONE               401
    ///NIMEX callback event flag: timer
    #define NIMEX_CALLBACK_EVENT_TIMER              402

//    #define NIMEX_WM_NIDAQ_MSG       (WM_APP + 0x20)

#endif /* End multiple include protection. */
