/**
 * @file NIMEX_deleteTask.c
 *
 * @brief Remove a task and all its resources from memory.
 *
 * <b>Syntax:</b>\n
 * <tt>NIMEX_deleteTask(taskDefinition)</tt>
 *  @arg <tt>taskDefinition</tt> - The task to be destroyed.
 *
 * @author Timothy O'Connor
 * @date 4/2/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 */
/*
 * CHANGES
 *  TO071107A - C89 compatibility for MSVC's cl compiler. -- Tim O'Connor 7/11/07
 */
#include "nimex.h"

/**
 * @brief Matlab entry point.
 * @callgraph
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    NIMEX_TaskDefinition* taskDefinition = (NIMEX_TaskDefinition *)NULL;
    int i = 0;//TO071107A
    
    if (nlhs != 0)
        mexErrMsgTxt("NIMEX_deleteTask: No output arguments are supported.");
    
    if (nrhs < 1)
        mexErrMsgTxt("NIMEX_deleteTask: Insufficient arguments (there must be at least 1 task to delete).");

    //First pass, check for invalid pointers.
    for (i = 0; i < nrhs; i++)
    {
        taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[i]);
        if (taskDefinition == NULL)
            mexErrMsgTxt("NIMEX_deleteTask: Invalid (uninitialized) NIMEX_TaskDefinition pointer value: NULL");
    }

    //Second pass, perform the cleanup(s).
    for (i = 0; i < nrhs; i++)
    {
        taskDefinition = (NIMEX_TaskDefinition *)NIMEX_unpackPointer(prhs[i]);
        NIMEX_MACRO_VERBOSE("NIMEX_deleteTask: @%p\n", taskDefinition)
        NIMEX_destroyTaskDefinition(taskDefinition);
    }
}
