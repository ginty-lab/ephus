/**
 * @file NIMEX_acquireLock.c
 *
 * @brief Allows Matlab to access the NIMEX synchronization primitive.
 *
 * All calls to NIMEX_acquireLock must be paired with calls to NIMEX_releaseLock.
 * @warning
 *  Use of the synchronization primitives is not encouraged, as it can lead to deadlocks.
 *  Extreme care must be taken when using these functions and should only be done by
 *  people who really know what they are doing.
 *
 * The only conceivable reason for using these primitives from Matlab is if a single NIMEX
 * task is being shared across Matlab engines/sessions.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_acquireLock(taskDefinition)</tt>
 *  @arg <tt>taskDefinition</tt> - The task whose lock to acquire.
 *
 * @see NIMEX_releaseLock
 *
 * @author Timothy O'Connor
 * @date 8/15/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 * @callgraph
 *
 */
#include <string.h>
#include "nimex.h"

/**
 * @brief Matlab entry point.
 * @callgraph
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    NIMEX_TaskDefinition* taskDefinition = (NIMEX_TaskDefinition *)NULL;
    
    NIMEX_MACRO_PRE_MEX

    if (nlhs != 0)
        mexErrMsgTxt("NIMEX_acquireLock: No return arguments supported.");
    
    taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[0]);
    if (taskDefinition == NULL)
        mexErrMsgTxt("NIMEX_acquireLock: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");

    NIMEX_acquireLock(taskDefinition, NIMEX_DEFAULT_THREAD_TIMEOUT);
    
    NIMEX_MACRO_POST_MEX
    
    return;
}
