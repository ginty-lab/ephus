/**
 * @file NIMEX_Utilities.h
 *
 * @brief Function declarations for NIMEX_Utilities.c.
 *
 * @see NIMEX_Utilities.c
 *
 * @author Timothy O'Connor
 * @date 11/16/06
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006</em>
 *
 */
/*
 * Changes:
 *  TO010607B: Created NIMEX_genericStructures. -- Tim O'Connor 1/6/07
 *  TO010607D: Created NIMEX_memManagement. -- Tim O'Connor 1/6/07
 *  TO010707A: Created NIMEX_displayTaskDefinition & NIMEX_displayChannelDefinition. -- Tim O'Connor 1/7/07
 *  TO010707C: Created NIMEX_uint32_To_mxArray, NIMEX_int32_To_mxArray, NIMEX_float64_To_mxArray, & NIMEX_uint64_To_mxArray. -- Tim O'Connor 1/7/07
 *  TO011707A: Created NIMEX_channelType_2_string & NIMEX_DAQmx_Constant_2_string. -- Tim O'Connor 1/17/07
 *  TO012407B: Added NIMEX_acquireLock & NIMEX_releaseLock for thread safety. -- Tim O'Connor 1/24/07
 *  TO012407C: Created NIMEX_implementCallback to simplify standardized callback execution. -- Tim O'Connor 1/24/07
 *  TO012907C: Moved properties from the NIMEX_ChannelDefinition to the  NIMEX_TaskDefinition, to better align with NIDAQmx. -- Tim O'Connor 1/29/07
 *  TO012907D: Created NIMEX_createChannelDefinition utility function. -- Tim O'Connor 1/29/07
 *  TO033007F: Created NIMEX_removeChannelDefinitionByName, NIMEX_destroyChannelDefinition, and NIMEX_destroyTaskDefinition. -- Tim O'Connor 3/30/07
 *  TO061207A: Port basic structures to glib. -- Tim O'Connor 6/12/07
 *  TO061407A: Implement events, preprocessors, and data listeners. -- Tim O'Connor 6/14/07
 *  TO061907A: Created NIMEX_wrapArray2mxArray. -- Tim O'Connor 6/19/07
 *  TO061907B: Created NIMEX_Callbacks.h. -- Tim O'Connor 6/19/07
 *  TO061907C: Created NIMEX_Objects.h. -- Tim O'Connor 6/19/07
 *  TO073070A: Added enable field to channels, allowing them to be defined but not started. -- Tim O'Connor 7/30/07
 *  TO080107A: Re-engineered constants and added more conversions. -- Tim O'Connor 8/1/07
 *  TO100607D: Deep primitive type conversions. -- Tim O'Connor 10/6/07
 *  TO101707A: Created NIMEX_acquireLockExclusively/NIMEX_releaseLockExclusively, to block the Windows message pump from entering critical code while waiting for blocking calls in NIDAQmx to return. -- Tim O'Connor 10/17/07
 *  TO101707C: Mu! Back out TO101707A, for now, as it was based on an invalid test scenario. -- Tim O'Connor 10/17/07
 *  TO101708J: Allow for incremental state transitions (verify, commit, etc). -- Tim O'Connor 10/18/07
 *  TO102007B: Broke NIMEX_updateBuffersByCallbacksAndWriteToTask out into a separate function, because it's needed for setting up a retriggered acquisition. -- Tim O'Connor 10/20/07
 *  TO090108A: Due to incompatibilities between 2007a and 2007b regarding mexSetTrapFlag (see TO082908A), we must check the Matlab version at runtime to determine behavior. -- Tim O'Connor 9/1/08
 *  TO080813C: Created NIMEX_transientWrapArray2mxArray to allow non-persistent wrapping. -- Tim O'Connor 8/8/13
 */

#ifndef _NIMEX_UTILITIES_h /* Multiple include protection. */
#define _NIMEX_UTILITIES_h

#include "nimex.h"

/*
 *
 */
void NIMEX_transientWrapArray2mxArray(mxArray** wrapped, int32 arraySize, int32 classID, void* data);//TO080813C

/*
 *
 */
void NIMEX_wrapArray2mxArray(mxArray** matlabData, int32 arraySize, int32 classID, void* data);//TO061907A

/*
 *
 */
void* NIMEX_memcpy(const void* src, size_t n);

/*
 *
 */
int32 NIMEX_MatlabVariable_2_DAQmx_Constant(const mxArray* matlabValue);

/*
 *
 */
char* NIMEX_DAQmx_Constant_2_string(int32 value);

/*
 *
 */
int32 NIMEX_MatlabVariable_2_NIMEX_Constant(const mxArray* matlabValue);//TO080107A

/*
 *
 */
char* NIMEX_Constant_2_string(int32 value);//TO080107A

/*
 *
 */
char* NIMEX_mxConstant_2_string(int32 value);//TO080107A

/*
 *
 */
void NIMEX_displayTaskDefinition(const char* prefix, NIMEX_TaskDefinition* task);

/*
 *
 */
char* NIMEX_channelType_2_string(int32 channelType);

/*
 *
 */
void NIMEX_displayChannelDefinition(const char* prefix, NIMEX_ChannelDefinition* channel);

/*
 *
 */
void NIMEX_displayDataSource(const char* prefix, NIMEX_DataSource* dataSource);

/*
 *
 */
mxArray* NIMEX_uint32_To_mxArray(uInt32 val);

/*
 *
 */
mxArray* NIMEX_int32_To_mxArray(int32 val);

/*
 *
 */
mxArray* NIMEX_float64_To_mxArray(float64 val);

/*
 *
 */
mxArray* NIMEX_uint64_To_mxArray(uInt64 val);

/*
 *
 */
int NIMEX_acquireLock(NIMEX_TaskDefinition* taskDefinition, uInt32 timeOut);

//int NIMEX_acquireLockExclusively(NIMEX_TaskDefinition* taskDefinition, uInt32 timeOut);//TO101707A//TO101707C

/*
 *
 */
int NIMEX_releaseLock(NIMEX_TaskDefinition* taskDefinition);

//int NIMEX_releaseLockExclusively(NIMEX_TaskDefinition* taskDefinition);//TO101707A//TO101707C

/*
 *
 */
int32 NIMEX_implementCallback(NIMEX_TaskDefinition* taskDefinition, NIMEX_Callback* callback);

/*
 *
 */
//TO073007A
int32 NIMEX_enabledChannelCount(NIMEX_TaskDefinition* taskDefinition);

void NIMEX_float64_2_int32(const float64* src, int32* dest, int offset, int len);
void NIMEX_float64_2_uInt32(const float64* src, uInt32* dest, int offset, int len);
void NIMEX_float64_2_uInt64(const float64* src, uInt64* dest, int offset, int len);

void NIMEX_int32_2_float64(const int32* src, float64* dest, int offset, int len);
void NIMEX_int32_2_uInt32(const int32* src, uInt32* dest, int offset, int len);
void NIMEX_int32_2_uInt64(const int32* src, uInt64* dest, int offset, int len);

void NIMEX_uInt32_2_int32(const uInt32* src, int32* dest, int offset, int len);
void NIMEX_uInt32_2_float64(const uInt32* src, float64* dest, int offset, int len);
void NIMEX_uInt32_2_uInt64(const uInt32* src, uInt64* dest, int offset, int len);

void NIMEX_uInt64_2_int32(const uInt64* src, int32* dest, int offset, int len);
void NIMEX_uInt64_2_float64(const uInt64* src, float64* dest, int offset, int len);
void NIMEX_uInt64_2_uInt32(const uInt64* src, uInt32* dest, int offset, int len);

int32* NIMEX_mxArray_2_int32(const mxArray* arr);
float64* NIMEX_mxArray_2_float64(const mxArray* arr);
uInt32* NIMEX_mxArray_2_uInt32(const mxArray* arr);
uInt64* NIMEX_mxArray_2_uInt64(const mxArray* arr);

int32 NIMEX_scalar_mxArray_2_int32(const mxArray* arr);
float64 NIMEX_scalar_mxArray_2_float64(const mxArray* arr);
uInt32 NIMEX_scalar_mxArray_2_uInt32(const mxArray* arr);
uInt64 NIMEX_scalar_mxArray_2_uInt64(const mxArray* arr);

uInt32 NIMEX_updateBuffersByCallbacksAndWriteToTask(NIMEX_TaskDefinition* taskDefinition);

//TO101708J
void NIMEX_createTaskHandle(NIMEX_TaskDefinition* taskDefinition);
int32 NIMEX_VerifyTask(NIMEX_TaskDefinition* taskDefinition);
int32 NIMEX_ReserveTask(NIMEX_TaskDefinition* taskDefinition);
int32 NIMEX_CommitTask(NIMEX_TaskDefinition* taskDefinition);

//TO090108A
char* NIMEX_getMatlabVersionString(void);
char* NIMEX_getMatlabReleaseString(void);
double NIMEX_getMatlabVersionNumber(void);
int32 NIMEX_isMatlabVersionGreaterThan(double desiredVersion);
int32 NIMEX_isMatlabVersionGreaterThanOrEqual(double desiredVersion);
int32 NIMEX_isMatlabVersionLessThan(double desiredVersion);
int32 NIMEX_isMatlabVersionLessThanOrEqual(double desiredVersion);

//void dumpLastErrorStackToConsole();
//void dumpLastErrorStackToStdErr();

#endif /* End multiple include protection. */
