/**
 * @file NIMEX_Callbacks.h
 *
 * @brief Collection of NI, NI->Matlab, OS->Matlab callback functions.
 *
 * @see NIMEX_Callbacks.c
 *
 * @author Timothy O'Connor 
 * @date 6/19/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 */
/*
 * NOTES
 *  See TO061407A, TO061907B.
 */

#ifndef _NIMEX_CALLBACKS_h /* Multiple include protection. */
#define _NIMEX_CALLBACKS_h

#include "nimex.h"

//From NIMEX_Utilities.h
//TO061407A - Callback execution functions that operate over NIMEX_CallbackMap objects.
void NIMEX_eventCallback(NIMEX_CallbackMap* table, const char* eventName);
void NIMEX_preprocessorCallback(NIMEX_CallbackMap* table, const char* channelName, mxArray** returnValue);
void NIMEX_samplesAcquiredCallback(NIMEX_CallbackMap* table, const char* channelName, mxArray* samples);

//From NIMEX_startTask.c
int32 CVICALLBACK NIMEX_EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void* callbackData);//TO010707C
int32 CVICALLBACK NIMEX_DoneCallback(TaskHandle taskHandle, int32 status, void* callbackData);//TO010707C
void CALLBACK NIMEX_TimerEventCallback(UINT wTimerID, UINT msg, DWORD userData, DWORD dw1, DWORD dw2);

LRESULT CALLBACK NIMEX_CallbackMessagePumpHook(int code, WPARAM wParam, LPARAM lParam);//TO080507B

void NIMEX_initializeCallbackProcessing();

#endif /* Multiple include protection. */
