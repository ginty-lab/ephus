/** 
 * @file nimex.h
 *
 * @brief All the basic includes required for using the nimex library.
 *
 * When developing with nimex, this is typically the only header file that will need to be included.\n
 *
 * Both the Mathworks' MEX and National Instruments' NIDAQmx libraries are loaded here.\n
 * Additionally, support libraries, such as glib and the platform/OS system calls are imported here.\n
 * Since nimex is a monolithic package, as of this writing, all the NIMEX_*.h header files\n
 * are included from here. None of the other headers need to be included directly.\n
 *
 * This file is protected against multiple includes.
 *
 * @author Timothy O'Connor
 * @date 11/20/06
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2006</em>
 *
 */
 
/*
 * Changes
 *  TO012407B - Formally include windows.h, needed for thread safety.
 *  TO022807A - Added NIMEX_BuildInfo.h to contain compile-time data (version info). -- Tim O'Connor 2/28/07
 *  TO061907B - Created NIMEX_Callbacks.h. -- Tim O'Connor 6/19/07
 *  TO061907C - Created NIMEX_Objects.h. -- Tim O'Connor 6/19/07
 *  TO090107A - Assert a minimum Windows version. Made nimex.h importable into NIMEX_BuildInfo.c. -- Tim O'Connor 9/1/07
 *  TO101107C - Moved the includes for non-NIMEX_BuildInfo.h-dependent files outside the conditional from TO090107A. -- Tim O'Connor 10/11/07
 *  TO051408A: Ported documentation to Doxygen. -- Tim O'Connor 5/14/08
 *
 */


#ifndef _nimex_h /* Multiple include protection. */
#define _nimex_h

    ///@brief A timestamp that is auto-generated at compile time.
    #define NIMEX_COMPILE_TIMESTAMP __DATE__ " - " __TIME__
    ///@brief The minimum Windows version needed. See MSDN documentation for runtime dependencies.
    #define _WIN32_WINNT 0x0400 //TO090107A - Minimum Windows version, see MSDN's SwitchToThread() documentation.
    #include "NIMEX_Constants.h"
    ///@brief A flag to indicate that the global defines are now safe to use.
    #define NIMEX_DEFINES_PROCESSED
    #include "string.h"
    #include "stdlib.h"
    #include "errno.h"
    #include "memwatch.h"
    #include "windows.h"//TO101107C
    #include "glib.h"//TO101107C
    #include <NIDAQmx.h>//TO101107C
    #include <mex.h>//TO101107C
    //TO090107A - All defines should be made outside of this block, so they may be recorded in NIMEX_BuildInfo.h.
    #ifndef  NIMEX_BuildInfo
        #include "NIMEX_BuildInfo.h"
        #include "NIMEX_memManagement.h"
        #include "NIMEX_genericStructures.h"
        #include "NIMEX_Structures.h"
        #include "NIMEX_Macros.h"
        #include "NIMEX_Utilities.h"
        #include "memwatch.h"
        #include "NIMEX_Objects.h"//TO061907C
        #include "NIMEX_Callbacks.h"//TO061907B

        ///@brief A flag to indicate that all global includes have now been processed.
        #define NIMEX_INCLUDES_PROCESSED
    #endif
#endif /* End multiple include protection. */
