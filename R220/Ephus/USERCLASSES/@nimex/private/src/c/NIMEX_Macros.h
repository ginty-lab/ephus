/**
 * @file NIMEX_Macros.h
 *
 * @brief Commonly used C preprocessor macros for the NIMEX library.
 *
 * @author Timothy O'Connor
 * @date 11/20/06
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006</em>
 *
 */
/*
 * Object Macros
 *  NIMEX_MACRO_VERBOSE - Calls through to mexPrintf if NIMEX_VERBOSE is set.
 *   NO_OP otherwise.
 *
 * CHANGES
 *  TO010607A - Added verbosity levels.
 *  TO010607C - Implemented NIMEX_MACRO_MXARRAY_2_PERSISTENT_CHAR. -- Tim O'Connor 1/6/07
 *  TO011907A - Added NIMEX_MACRO_CONST_2_STR. -- Tim O'Connor 1/19/07
 *  TO012607A - Added conversions for datatype primitives. -- Tim O'Connor 1/26/07
 *  TO012607B - Added memory access macros for insertion of debugging code. -- Tim O'Connor 1/26/07
 *  TO022807C - Added mexFunction pre and post macros. -- Tim O'Connor 2/28/07
 *  TO061107A - Added extra version information. -- Tim O'Connor 6/11/07
 *  TO071207A - MSVC++'s cl deprecated strcmpi. -- Tim O'Connor 7/12/07
 *  TO080507A - Added macros for conversion from scalar mxArrays to native types. -- Tim O'Connor 8/5/07
 *  TO100907A - uInt64 debugging. -- Tim O'Connor 10/9/07
 *  TO101707J - Added in the NIDAQmx version info. -- Tim O'Connor 10/17/07
 *  TO080813D - Removed some stale code that hasn't been in use. -- Tim O'Connor 8/8/13
 */

#ifndef _NIMEX_MACROS_h /* Multiple include protection. */
#define _NIMEX_MACROS_h

//mexPrintf("NIMEX_VERBOSE:" __FILE__ "/" __func__ ":" __VA_ARGS__);
#include "nimex.h"

///@brief Defines a log file for debugging.
#ifdef NIMEX_DEBUG_LOG
    #include <stdio.h>
    FILE* NIMEX_DEBUG_LOGFILE;
    #ifndef NIMEX_COMPILER_CL
        ///@brief Redirected print statement, aspect-oriented programming style.
		/** NIMEX_DEBUG_LOGFILE = fopen("C:\\NIMEX_debug.log", "a"); \ */
        #define NIMEX_MACRO_PRINTF(...) \
			NIMEX_DEBUG_LOGFILE = fopen("C:\\temp\\NIMEX_debug.log", "a"); \
            fprintf(NIMEX_DEBUG_LOGFILE, __VA_ARGS__); \
            fclose(NIMEX_DEBUG_LOGFILE); \
            mexPrintf(__VA_ARGS__)
    #else
        ///@brief Redirected print statement, aspect-oriented programming style.
		/** fopen_s(&NIMEX_DEBUG_LOGFILE, "C:\\NIMEX_debug.log", "a"); \ */
        #define NIMEX_MACRO_PRINTF(...) \
            fopen_s(&NIMEX_DEBUG_LOGFILE, "C:\\temp\\NIMEX_debug.log", "a"); \
            fprintf(NIMEX_DEBUG_LOGFILE, __VA_ARGS__); \
            fclose(NIMEX_DEBUG_LOGFILE); \
            mexPrintf(__VA_ARGS__)
    #endif
#else
    ///@brief Redirected print statement, aspect-oriented programming style.
    #define NIMEX_MACRO_PRINTF(...) mexPrintf(__VA_ARGS__)
#endif

///@brief Copies an mxArray string into a persistent C-string, in newly allocated memory.
#define NIMEX_MACRO_MXARRAY_2_PERSISTENT_CHAR(mxStr) NIMEX_memcpy(mxArrayToString(mxStr), (mxGetNumberOfElements(mxStr) + 1) * sizeof(char))

///@brief Helps print the name of a constant.
#define NIMEX_MACRO_CONST_2_NAME(val) #val

///@brief Helps print the name of a constant.
#define NIMEX_MACRO_CONST_2_VAL(val) NIMEX_MACRO_CONST_2_NAME(val)

///@brief Converts a constant into a string, including the name and value.
#define NIMEX_MACRO_CONST_2_STR(val) NIMEX_MACRO_CONST_2_NAME(val) "(" NIMEX_MACRO_CONST_2_VAL(val) ")"

///@brief Regularized stamp for printing debug statements.
#define NIMEX_MACRO_SOURCE_STAMP __FILE__ "/" __func__ ": "

///@brief Hardcoded version string that depends on the compile time environment.
//TO061107A - Aggregate all the standard NIMEX_BuildInfo.h values into a single, printable string.
#define NIMEX_MACRO_VERSION_INFO "NIMEX Build Information -\n NIMEX_COMPILE_TIMESTAMP: " NIMEX_COMPILE_TIMESTAMP \
        "\n NIMEX_BUILD_SERIAL_NUMBER: " NIMEX_BUILD_SERIAL_NUMBER \
        "\n NIMEX_VERSION: " NIMEX_VERSION \
        "\n NIMEX_MAJOR_VERSION: " NIMEX_MAJOR_VERSION \
        "\n NIMEX_MINOR_VERSION: " NIMEX_MINOR_VERSION \
        "\n NIMEX_NUMERIC_VERSION: " NIMEX_NUMERIC_VERSION \
        "\n NIMEX_PROGRAMMER: " NIMEX_PROGRAMMER \
        "\n\t System Info -" \
        "\n\t NIMEX_TARGET_OS: " NIMEX_TARGET_OS \
        "\n\t NIMEX_WINDOWS_VERSION: " NIMEX_WINDOWS_VERSION \
        "\n\t NIMEX_WIN32_VERSION: " NIMEX_WIN32_VERSION \
        "\n\t NIMEX_TARGET_PLATFORM: " NIMEX_TARGET_PLATFORM \
        "\n\t NIMEX_GLIB_TARGET: " NIMEX_GLIB_VERSION "\n"
//        "\n\t NIMEX_GLIB_RUNTIME: " glib_check_version(NIMEX_GLIB_MAJOR_VERSION, NIMEX_GLIB_MINOR_VERSION, NIMEX_GLIB_MICRO_VERSION) "\n" \

/** TO080813D - These are never called, and the ones in NIMEX_Utilities have been in use for years.
//TO022707? - The equivalents in NIMEX_Utilities seemed to not be working, but it was a different issue. -- Tim O'Connor 2/27/07
///@brief Creates a new mxArray and uses it to wrap an unsigned 32-bit integer.
#define NIMEX_MACRO_uint32_To_mxArray(mxArray, val) mxArray = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL); (*((uInt32 *)mxGetPr(mxArray))) = val;
///@brief Creates a new mxArray and uses it to wrap a 32-bit integer.
#define NIMEX_MACRO_int32_To_mxArray(mxArray, val) mxArray = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL); (*((int32 *)mxGetPr(mxArray))) = val;
///@brief Creates a new mxArray and uses it to wrap a 64-bit float.
#define NIMEX_MACRO_float64_To_mxArray(mxArray, val) mxArray = mxCreateNumericMatrix(1, 1, mxDOUBLE_CLASS, mxREAL); (*((float64 *)mxGetPr(mxArray))) = val;
///@brief Creates a new mxArray and uses it to wrap an unsigned 64-bit integer.
#define NIMEX_MACRO_uint64_To_mxArray(mxArray, val) mxArray = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL); (*((uInt64 *)mxGetPr(mxArray))) = val;
*/

//TO100907A
#define NIMEX_MACRO_float64_To_uInt32(val) ((uInt32)val)
#define NIMEX_MACRO_float64_To_int32(val) ((int32)val)
#define NIMEX_MACRO_float64_To_uInt64(val) ((uInt64)val)

#define NIMEX_MACRO_uInt32_To_float64(val) ((float64)val)
#define NIMEX_MACRO_uInt32_To_int32(val) ((int32)val)
#define NIMEX_MACRO_uInt32_To_uInt64(val) ((uInt64)val)

#define NIMEX_MACRO_int32_To_uInt32(val) ((uInt32)val)
#define NIMEX_MACRO_int32_To_float64(val) ((float64)val)
#define NIMEX_MACRO_int32_To_uInt64(val) ((uInt64)val)

#define NIMEX_MACRO_uInt64_To_uInt32(val) ((uInt32)val)
#define NIMEX_MACRO_uInt64_To_float64(val) ((float64)val)
#define NIMEX_MACRO_uInt64_To_int32(val) ((int32)val)

//TO080507A - Added macros for conversion from scalar mxArrays to native types. -- Tim O'Connor 8/5/07
#define NIMEX_MACRO_mxArray_To_uInt32(mxArr) ((uInt32)*(mxGetPr(mxArr)))
#define NIMEX_MACRO_mxArray_To_int32(mxArr) ((int32)*(mxGetPr(mxArr)))
#define NIMEX_MACRO_mxArray_To_float64(mxArr) ((float64)*(mxGetPr(mxArr)))
#define NIMEX_MACRO_mxArray_To_uInt64(mxArr) ((uint64)*(mxGetPr(mxArr)))

//#define NIMEX_MACRO_PRE_MEX NIMEX_MACRO_PRINTF("NIMEX Library Information.\n  Version: %s\n  Serial #: %s\n\n", NIMEX_VERSION, NIMEX_BUILD_SERIAL_NUMBER); return;
//#define NIMEX_MACRO_PRE_MEX if (nrhs == 1) {if (mxGetClassID(prhs[0]) == mxCHAR_CLASS) {if (strcmpi("-version", mxArrayToString(prhs[0])) == 0) {NIMEX_MACRO_PRINTF(NIMEX_MACRO_VERSION_INFO); return;}}}
//TO061107A
//TO071207A - MSVC++'s cl deprecated strcmpi. -- Tim O'Connor 7/12/07
//TO101707J - Added NIDAQmx version info. -- Tim O'Connor 10/17/07
#ifdef NIMEX_COMPILER_CL
    /**
     * @brief Aspect-oriented functionality that should be executed at the beginning of all mexFunction implementations.
     *
     * Here it's used to return version information.
     */
    #define NIMEX_MACRO_PRE_MEX if (nrhs == 1) {if (mxGetClassID(prhs[0]) == mxCHAR_CLASS) \
        {if (_strcmpi("-version", mxArrayToString(prhs[0])) == 0) \
        {mexPrintf("NIMEX Library Information.\n  Version: %s\n  Serial #: %s\n  Build: %s\n  Target OS: %s\n  Target Platform: %s\n" \
                   "  Target NIDAQmx Version: %s\n", \
        NIMEX_VERSION, NIMEX_BUILD_SERIAL_NUMBER, NIMEX_NUMERIC_VERSION, NIMEX_TARGET_OS, NIMEX_TARGET_PLATFORM, NIMEX_NI_VERSION);} \
        {if (glib_check_version(NIMEX_GLIB_MAJOR_VERSION, NIMEX_GLIB_MINOR_VERSION, NIMEX_GLIB_MICRO_VERSION) != NULL) \
         {mexPrintf("  GLib Runtime: %d.%d.%d (compiled/linked against %s) [Compatibility message: \"%s\"]\n", \
         glib_major_version, glib_minor_version, glib_micro_version, \
         NIMEX_GLIB_VERSION, glib_check_version(NIMEX_GLIB_MAJOR_VERSION, NIMEX_GLIB_MINOR_VERSION, NIMEX_GLIB_MICRO_VERSION));} \
        else \
         {mexPrintf("  GLib Runtime: %d.%d.%d (NIMEX was compiled/linked against %s)\n", \
         glib_major_version, glib_minor_version, glib_micro_version, NIMEX_GLIB_VERSION);}} \
        mexPrintf("  Creator: %s\n  Maintainer: %s\n\n", NIMEX_CREATOR, NIMEX_PROGRAMMER); \
        return;}}
#else
    /**
     * @brief Aspect-oriented functionality that should be executed at the beginning of all mexFunction implementations.
     *
     * Here it's used to return version information.
     */
    #define NIMEX_MACRO_PRE_MEX if (nrhs == 1) {if (mxGetClassID(prhs[0]) == mxCHAR_CLASS) \
        {if (strcmpi("-version", mxArrayToString(prhs[0])) == 0) \
        {mexPrintf("NIMEX Library Information.\n  Version: %s\n  Serial #: %s\n  Build: %s\n  Target OS: %s\n  Target Platform: %s\n" \
                   "  Target NIDAQmx Version: %s\n", \
        NIMEX_VERSION, NIMEX_BUILD_SERIAL_NUMBER, NIMEX_NUMERIC_VERSION, NIMEX_TARGET_OS, NIMEX_TARGET_PLATFORM, NIMEX_NI_VERSION);} \
        {if (glib_check_version(NIMEX_GLIB_MAJOR_VERSION, NIMEX_GLIB_MINOR_VERSION, NIMEX_GLIB_MICRO_VERSION) != NULL) \
         {mexPrintf("  GLib Runtime: %d.%d.%d (NIMEX was compiled/linked against %s) [Compatibility message: \"%s\"]\n", \
         glib_major_version, glib_minor_version, glib_micro_version, \
         NIMEX_GLIB_VERSION, glib_check_version(NIMEX_GLIB_MAJOR_VERSION, NIMEX_GLIB_MINOR_VERSION, NIMEX_GLIB_MICRO_VERSION));} \
        else \
         {mexPrintf("  GLib Runtime: %d.%d.%d (compiled/linked against %s)\n", \
         glib_major_version, glib_minor_version, glib_micro_version, NIMEX_GLIB_VERSION);}} \
        mexPrintf("  Creator: %s\n  Maintainer: %s\n\n", NIMEX_CREATOR, NIMEX_PROGRAMMER); \
        return;}}
#endif
/**
 * @brief Aspect-oriented functionality that should be executed at the end of all mexFunction implementations.
 *
 * Currently unused.
 */
#define NIMEX_MACRO_POST_MEX

//TO100907A - uInt64 debugging.
///@brief Microsoft's compiler uses different literal suffixes.
#define NIMEX_MACRO_UINT64_LITERAL(n) n ## ull;
///@brief Microsoft's compiler uses different literal suffixes.
#define NIMEX_MACRO_INT64_LITERAL(n) n ## ll;

//TO012607A
#define NIMEX_MACRO_FLOAT_2_ROUNDED_LONG(val) (val >= 0 ? (long)(val + 0.5) : (long)(val - 0.5))

//TO012607B
//#define NIMEX_MACRO_FREE(ptr) NIMEX_MACRO_PRINTF("NIMEX_MACRO_FREE freeing @%p.\n", ptr); free(ptr);
///@brief Aspect-oriented memory free function.
#define NIMEX_MACRO_FREE(ptr) free(ptr);

//TO071207A - MSVC++'s cl deprecated strcmpi. -- Tim O'Connor 7/12/07
#ifdef NIMEX_COMPILER_CL
    ///@brief Microsoft's runtime deprecated strcmpi, so a preprocessor switch is needed.
    #define NIMEX_MACRO_STRCMPI(...)    _strcmpi(__VA_ARGS__)
#else
    ///@brief Microsoft's runtime deprecated strcmpi, so a preprocessor switch is needed.
    #define NIMEX_MACRO_STRCMPI(...)    strcmpi(__VA_ARGS__)
#endif

#ifdef NIMEX_VERBOSE_RUNTIME
	#ifndef NIMEX_VERBOSITY
        #define NIMEX_VERBOSITY 0
        //#pragma message("Compiling with no verbosity.")
    #endif
    ///@brief Debugging print statement control.
	#define NIMEX_MACRO_VERBOSE(...)   if (NIMEX_VERBOSITY != 0) NIMEX_MACRO_PRINTF(__VA_ARGS__);
    ///@brief Debugging print statement control.
	#define NIMEX_MACRO_VERBOSE_1(...)   if (NIMEX_VERBOSITY >= 1) NIMEX_MACRO_PRINTF(__VA_ARGS__);
    ///@brief Debugging print statement control.
	#define NIMEX_MACRO_VERBOSE_2(...)   if (NIMEX_VERBOSITY >= 2) NIMEX_MACRO_PRINTF(__VA_ARGS__);
    ///@brief Debugging print statement control.
	#define NIMEX_MACRO_VERBOSE_3(...)   if (NIMEX_VERBOSITY >= 3) NIMEX_MACRO_PRINTF(__VA_ARGS__);
    ///@brief Debugging print statement control.
	#define NIMEX_MACRO_VERBOSE_4(...)   if (NIMEX_VERBOSITY >= 4) NIMEX_MACRO_PRINTF(__VA_ARGS__);
#else
    #ifdef NIMEX_VERBOSE_4
        #ifndef NIMEX_VERBOSITY
            #define NIMEX_VERBOSITY 4
            //#pragma message("Compiling with NIMEX_VERBOSE_4.")
        #endif
    
        #define NIMEX_VERBOSE_3
        #define NIMEX_VERBOSE_2
        #define NIMEX_VERBOSE_1
        #define NIMEX_VERBOSE
        ///@brief Debugging print statement control.
        #define NIMEX_MACRO_VERBOSE(...)   NIMEX_MACRO_PRINTF(__VA_ARGS__);
        ///@brief Debugging print statement control.
        #define NIMEX_MACRO_VERBOSE_1(...)   NIMEX_MACRO_PRINTF(__VA_ARGS__);
        ///@brief Debugging print statement control.
        #define NIMEX_MACRO_VERBOSE_2(...)   NIMEX_MACRO_PRINTF(__VA_ARGS__);
        ///@brief Debugging print statement control.
        #define NIMEX_MACRO_VERBOSE_3(...)   NIMEX_MACRO_PRINTF(__VA_ARGS__);
        ///@brief Debugging print statement control.
        #define NIMEX_MACRO_VERBOSE_4(...)   NIMEX_MACRO_PRINTF(__VA_ARGS__);
    #else
        #ifdef NIMEX_VERBOSE_3
            #ifndef NIMEX_VERBOSITY
                #define NIMEX_VERBOSITY 3
                //#pragma message("Compiling with NIMEX_VERBOSE_3.")
            #endif
            #define NIMEX_VERBOSE_2
            #define NIMEX_VERBOSE_1
            #define NIMEX_VERBOSE
            ///@brief Debugging print statement control.
            #define NIMEX_MACRO_VERBOSE(...)   NIMEX_MACRO_PRINTF(__VA_ARGS__);
            ///@brief Debugging print statement control.
            #define NIMEX_MACRO_VERBOSE_1(...)   NIMEX_MACRO_PRINTF(__VA_ARGS__);
            ///@brief Debugging print statement control.
            #define NIMEX_MACRO_VERBOSE_2(...)   NIMEX_MACRO_PRINTF(__VA_ARGS__);
            ///@brief Debugging print statement control.
            #define NIMEX_MACRO_VERBOSE_3(...)   NIMEX_MACRO_PRINTF(__VA_ARGS__);
            ///@brief Debugging print statement control.
            #define NIMEX_MACRO_VERBOSE_4(...)
        #else
            #ifdef NIMEX_VERBOSE_2
                #ifndef NIMEX_VERBOSITY
                    #define NIMEX_VERBOSITY 2
                    //#pragma message("Compiling with NIMEX_VERBOSE_2.")
                #endif
                #define NIMEX_VERBOSE_1
                #define NIMEX_VERBOSE
                ///@brief Debugging print statement control.
                #define NIMEX_MACRO_VERBOSE(...)   NIMEX_MACRO_PRINTF(__VA_ARGS__);
                ///@brief Debugging print statement control.
                #define NIMEX_MACRO_VERBOSE_1(...)   NIMEX_MACRO_PRINTF(__VA_ARGS__);
                ///@brief Debugging print statement control.
                #define NIMEX_MACRO_VERBOSE_2(...)   NIMEX_MACRO_PRINTF(__VA_ARGS__);
                ///@brief Debugging print statement control.
                #define NIMEX_MACRO_VERBOSE_3(...)
                ///@brief Debugging print statement control.
                #define NIMEX_MACRO_VERBOSE_4(...)
            #else
                #ifdef NIMEX_VERBOSE_1
                    #ifndef NIMEX_VERBOSITY
                        #define NIMEX_VERBOSITY 1
                        //#pragma message("Compiling with NIMEX_VERBOSE_1.")
                    #endif
                    #define NIMEX_VERBOSE
                    ///@brief Debugging print statement control.
                    #define NIMEX_MACRO_VERBOSE(...)   NIMEX_MACRO_PRINTF(__VA_ARGS__);
                    ///@brief Debugging print statement control.
                    #define NIMEX_MACRO_VERBOSE_1(...)   NIMEX_MACRO_PRINTF(__VA_ARGS__);
                    ///@brief Debugging print statement control.
                    #define NIMEX_MACRO_VERBOSE_2(...)
                    ///@brief Debugging print statement control.
                    #define NIMEX_MACRO_VERBOSE_3(...)
                    ///@brief Debugging print statement control.
                    #define NIMEX_MACRO_VERBOSE_4(...)
                #else
                    #ifdef NIMEX_VERBOSE
                        #ifndef NIMEX_VERBOSITY
                            #define NIMEX_VERBOSITY 1
                            //#pragma message("Compiling with NIMEX_VERBOSE_1.")
                        #endif
                        #define NIMEX_MACRO_VERBOSE(...)   NIMEX_MACRO_PRINTF( __VA_ARGS__);
                    #else
                        #ifndef NIMEX_VERBOSITY
                            #define NIMEX_VERBOSITY 0
                            //#pragma message("Compiling with no verbosity.")
                        #endif
                        #define NIMEX_MACRO_VERBOSE(...)
                    #endif
                    ///@brief Debugging print statement control.
                    #define NIMEX_MACRO_VERBOSE_1(...)
                    ///@brief Debugging print statement control.
                    #define NIMEX_MACRO_VERBOSE_2(...)
                    ///@brief Debugging print statement control.
                    #define NIMEX_MACRO_VERBOSE_3(...)
                    ///@brief Debugging print statement control.
                    #define NIMEX_MACRO_VERBOSE_4(...)
                #endif
            #endif
    	#endif
    #endif
#endif
#endif /* End multiple include protection. */
