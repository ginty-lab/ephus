/**
 * @file NIMEX_BuildInfo.c
 *
 * @brief Produce code for an importable header at compile time, containing current build/version information.
 *
 * The information contained in the produced header file (probably NIMEX_BuildInfo.h) is as follows:
 *      @verbinclude NIMEX_BuildInfo.h
 *
 * @author Timothy O'Connor
 * @date 2/28/07
 *
 * <em><b>Copyright</b> - Cold Spring Harbor Laboratories/Howard Hughes Medical Institute 2007</em>
 *
 */
/*
 * CHANGES
 *  TO061107A - Added extra information. -- Tim O'Connor 6/11/07
 *  TO071107A - C89 compatibility for MSVC's cl compiler. -- Tim O'Connor 7/11/07
 *  TO090107A - Made nimex.h importable into NIMEX_BuildInfo.c. Captured new variables. -- Tim O'Connor 9/1/07
 *  TO101007I - Expanded compiler-specific definitions. -- Tim O'Connor 10/10/07
 *  TO101707J - Added in the NIDAQmx version info. -- Tim O'Connor 10/17/07
 *  TO051408A - Add Doxygen style annotation in the resulting header file. -- Tim O'Connor 5/14/08
 *
 */

//First, check for compiler consistency.
#ifdef NIMEX_COMPILER_CL
    #ifndef _MSC_VER
        #error "Microsoft's cl compiler is specified as the environment, but expected _MSC_VER was not found."
    #endif
#endif

#define NIMEX_BuildInfo //TO090107A - It's not safe to import NIMEX_BuildInfo.h when compiling.
#include "nimex.h"
#undef NIMEX_BuildInfo //TO090107A - It's now safe to import NIMEX_BuildInfo.h when compiling.
#ifdef NIMEX_INCLUDES_PROCESSED
    #error Imported NIMEX headers prematurely!
#endif
#include <windows.h>//Included so any header defined variables are accessible here, actual dependencies of this file are also included below.
#include <stdio.h>
#include <time.h>

/**
 * @brief Standard executable entry point.
 *
 * COMMAND-LINE SYNTAX\n
 * @verbatim
    NIMEX_BuildInfo
    NIMEX_BuildInfo NIMEX_VERSION
    NIMEX_BuildInfo NIMEX_VERSION NIMEX_MAJOR_VERSION
    NIMEX_BuildInfo NIMEX_VERSION NIMEX_MAJOR_VERSION NIMEX_MINOR_VERSION
    NIMEX_BuildInfo NIMEX_VERSION NIMEX_MAJOR_VERSION NIMEX_MINOR_VERSION NIMEX_PROGRAMMER
    NIMEX_BuildInfo NIMEX_VERSION NIMEX_MAJOR_VERSION NIMEX_MINOR_VERSION NIMEX_PROGRAMMER NAME VALUE ...
   @endverbatim
 *  @arg <tt>NIMEX_VERSION</tt> - The NIMEX version string, to be embedded in all binaries.
 *  @arg <tt>NIMEX_MAJOR_VERSION</tt> - A 1 or 2 digit numeric value for the major version.
 *  @arg <tt>NIMEX_MINOR_VERSION</tt> - A 1 or 2 digit numeric value for the minor version.
 *  @arg <tt>NIMEX_PROGRAMMER</tt> - The name of the programmer executing the build.
 *  @arg <tt>NAME</tt> - An arbitrary variable name to be <tt>\#define</tt>d into the NIMEX namespace.
 *  @arg <tt>VALUE</tt> - An arbitrary value to be <tt>\#define</tt>d for the previously specified name in the NIMEX namespace.
 *   <em>Multiple NAME-VALUE pairs can be specified.</em>
 *
 * @return 0 if successful, non-zero otherwise. Specifically, 1 if the wall clock time can not be determined.
 *
 */
int main(int argc, char** argv)
{
    int     i = 0;//TO071107A
    char    timestamp[20] = {'\0'};
    char    numericTimestamp[16] = {'\0'};
    time_t  currentTime = (time_t)NULL;
    #ifdef NIMEX_LOCALTIME_DEPRECATED
    struct tm   secureTime;
    #endif

    #ifdef NIMEX_COMPILER_CL
    time(&currentTime);
    localtime_s(&secureTime, &currentTime);
    strftime(timestamp, 20, "%m-%d-%Y_%H-%M-%S", &secureTime);
    strftime(numericTimestamp, 16, "%m%d%Y.%H%M%S", &secureTime);
    #else
    if (time(&currentTime) < currentTime)
    {
        printf("Failed to retrieve wall clock time.\n");
        return 1;
    }
    strftime(timestamp, 20, "%m-%d-%Y_%H-%M-%S", localtime(&currentTime));
    strftime(numericTimestamp, 16, "%m%d%Y.%H%M%S", localtime(&currentTime));
    #endif

    printf("/**\n * Auto-generated at build time.\n *  @date %s\n *  Time: %s\n */\n", __DATE__, __TIME__);//TO051408A
    printf("#ifndef _NIMEX_BUILDINFO_h\n");
    printf("#define _NIMEX_BUILDINFO_h\n");
    printf("\n    //These variables are hardcoded, for the time being.\n");
    printf("    #define NIMEX_TARGET_OS \"WIN32\"\n");
    printf("    #define NIMEX_TARGET_PLATFORM \"MATLAB R2007a (7.4.0)\"\n");

    printf("\n    //Define these variables in the build environment (command line shell) to set their values in this file.\n");
    #ifndef NIMEX_BUILD_SERIAL_NUMBER
        printf("    #define NIMEX_BUILD_SERIAL_NUMBER \"%s\"\n", timestamp);
    #else
        printf("    #define NIMEX_BUILD_SERIAL_NUMBER \"%s\"\n", NIMEX_BUILD_SERIAL_NUMBER);
        timestamp = NIMEX_BUILD_SERIAL_NUMBER;
    #endif
    if (argc >= 2)
        #ifndef NIMEX_VERSION
            printf("    #define NIMEX_VERSION \"%s\"\n", argv[1]);
        #else
            printf("    #define NIMEX_VERSION \"%s\"\n", NIMEX_VERSION);
        #endif
    else
        #ifndef NIMEX_VERSION
            printf("    #define NIMEX_VERSION \"UNSPECIFIED_VERSION\"\n");
        #else
            printf("    #define NIMEX_VERSION \"%s\"\n", NIMEX_VERSION);
        #endif

    //TO061107A
    if (argc >= 3)
    {
        #ifndef NIMEX_MAJOR_VERSION
            printf("    #define NIMEX_MAJOR_VERSION \"%s\"\n", argv[2]);
        #else
            printf("    #define NIMEX_MAJOR_VERSION \"%s\"\n", NIMEX_MAJOR_VERSION);
        #endif
    }
    else
    {
        #ifndef NIMEX_MAJOR_VERSION
          #define NIMEX_MAJOR_VERSION "XX"
        #endif
        printf("    #define NIMEX_MAJOR_VERSION \"%s\"\n", NIMEX_MAJOR_VERSION);
    }

    //TO061107A
    if (argc >= 4)
    {
        #ifndef NIMEX_MINOR_VERSION
            printf("    #define NIMEX_MINOR_VERSION \"%s\"\n", argv[3]);
        #else
            printf("    #define NIMEX_MINOR_VERSION \"%s\"\n", NIMEX_MINOR_VERSION);
        #endif
    }
    else
    {
        #ifndef NIMEX_MINOR_VERSION
          #define NIMEX_MINOR_VERSION "XX"
        #endif
        printf("    #define NIMEX_MINOR_VERSION \"%s\"\n", NIMEX_MINOR_VERSION);
    }
    
    //TO061107A
    if (argc >= 4)
    {
        printf("    #define NIMEX_NUMERIC_VERSION \"%s.%s.%s\"\n", argv[2], argv[3], timestamp);
    }
    else
    {
        #ifdef NIMEX_MAJOR_VERSION
          #ifdef NIMEX_MINOR_VERSION
            printf("    #define NIMEX_NUMERIC_VERSION \"%s.%s.%s\"\n", NIMEX_MAJOR_VERSION, NIMEX_MINOR_VERSION, numericTimestamp);
          #else
            printf("    #define NIMEX_NUMERIC_VERSION \"%s.XX.%s\"\n", NIMEX_MAJOR_VERSION, numericTimestamp);
          #endif
        #else
          printf("    #define NIMEX_NUMERIC_VERSION \"XX.XX.%s\"\n", numericTimestamp);
        #endif
    }

    printf("    #define NIMEX_CREATOR \"Timothy O'Connor\"\n");
    if (argc >= 5)
    {
        #ifndef NIMEX_PROGRAMMER
            printf("    #define NIMEX_PROGRAMMER \"%s\" "
                   "//The person responsible for this build.\n", argv[4]);
        #else
            printf("    #define NIMEX_PROGRAMMER \"%s\" "
                   "//The person responsible for this build.\n", NIMEX_PROGRAMMER);
        #endif
    }
    else
    {
        #ifndef NIMEX_PROGRAMMER
            printf("    #define NIMEX_PROGRAMMER \"UNKNOWN\" "
                   "//No programmer name was specified in the build environment. For good practice, a name should be supplied.\n");
        #else
            printf("    #define NIMEX_PROGRAMMER \"%s\" "
                   "//The person responsible for this build.\n", NIMEX_PROGRAMMER);
        #endif
    }

    #ifdef NIMEX_COMPILER_LCC
        printf("    #define NIMEX_COMPILER \"lcc\"\n");
    #endif
    #ifdef NIMEX_COMPILER_GCC
        printf("    #define NIMEX_COMPILER \"gcc\"\n");
    #endif    
    #ifdef NIMEX_COMPILER_CL
        printf("    #define NIMEX_COMPILER \"cl\" //aka Microsoft's Visual Studio/Visual C/Visual C++.\n");
    #endif

    for (i = 5; i < argc - 1; i++)
        printf("    #define %s %s\n", argv[i], argv[i + 1]);

    printf("\n    //These variables are not definable from the command line and, in general, should not be modified.\n");
    printf("    //They capture information about variables defined by includes and other system specific information.\n");
    printf("    //OS/platform.\n");
    printf("      //Windows-specific information.\n");
    #ifdef WINVER
        printf("      #define NIMEX_WINDOWS_VERSION 0x%8.8X //%d\n", WINVER, WINVER);
    #else
        printf("      #define NIMEX_WINDOWS_VERSION 0x00000000 //Not defined, defaulted to 0.\n");
    #endif
    #ifdef _WIN32_WINNT
        printf("      #define NIMEX_WIN32_VERSION 0x%8.8X //%d //Minimum Windows version for which the code is targeted.\n", _WIN32_WINNT, _WIN32_WINNT);
    #else
        printf("      #define NIMEX_WIN32_VERSION 0x00000000 //Not defined, defaulted to 0.\n");
    #endif
    #ifdef NTDDI_VERSION
        printf("      #define NIMEX_NTDDI_VERSION 0x%8.8X //%d\n", NTDDI_VERSION, NTDDI_VERSION);
    #else
        printf("      #define NIMEX_NTDDI_VERSION 0x00000000 //Not defined, defaulted to 0.\n");
    #endif

    printf("      //Matlab-specific information.\n");
    #ifdef MATLAB_MEX_FILE
        printf("      #define NIMEX_MEX_FILE 1 //Compiled as a mex file.\n");
    #else
        printf("      #define NIMEX_MEX_FILE 0 //Not compiled as a mex file.\n");
    #endif
    
    //TO101707J
    printf("      //NIDAQmx information.\n");
    printf("      #define NIMEX_NI_MAJOR_VERSION 0x%8.8X //%d\n", DAQmx_Sys_NIDAQMajorVersion, DAQmx_Sys_NIDAQMajorVersion);
    printf("      #define NIMEX_NI_MINOR_VERSION 0x%8.8X //%d\n", DAQmx_Sys_NIDAQMinorVersion, DAQmx_Sys_NIDAQMinorVersion);
    printf("      #define NIMEX_NI_VERSION \"%d.%d\"\n", DAQmx_Sys_NIDAQMajorVersion, DAQmx_Sys_NIDAQMinorVersion);
    
    printf("      //GLib-specific information.\n");
    #ifdef GLIB_MAJOR_VERSION
        printf("      #define NIMEX_GLIB_MAJOR_VERSION 0x%8.8X //%d\n", GLIB_MAJOR_VERSION, GLIB_MAJOR_VERSION);
        #define NIMEX_GLIB_MAJOR_VERSION GLIB_MAJOR_VERSION
    #else
        printf("      #define NIMEX_GLIB_MAJOR_VERSION 0x00000000 //Not defined, defaulted to 0.\n");
        #define NIMEX_GLIB_MAJOR_VERSION 0x00000000
    #endif
    #ifdef GLIB_MINOR_VERSION
        printf("      #define NIMEX_GLIB_MINOR_VERSION 0x%8.8X //%d\n", GLIB_MINOR_VERSION, GLIB_MINOR_VERSION);
        #define NIMEX_GLIB_MINOR_VERSION GLIB_MINOR_VERSION
    #else
        printf("      #define NIMEX_GLIB_MINOR_VERSION 0x00000000 //Not defined, defaulted to 0.\n");
        #define NIMEX_GLIB_MINOR_VERSION 0x00000000
    #endif
    #ifdef GLIB_MICRO_VERSION
        printf("      #define NIMEX_GLIB_MICRO_VERSION 0x%8.8X //%d\n", GLIB_MICRO_VERSION, GLIB_MICRO_VERSION);
        #define NIMEX_GLIB_MICRO_VERSION GLIB_MICRO_VERSION
    #else
        printf("      #define NIMEX_GLIB_MICRO_VERSION 0x00000000 //Not defined, defaulted to 0.\n");
        #define NIMEX_GLIB_MICRO_VERSION 0x00000000
    #endif
    printf("      #define NIMEX_GLIB_VERSION \"%d.%d.%d\"\n", GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION);
    
    //TO090107A
    printf("\n    //Compiler-specific definitions.\n");
    //TO090107A
    printf("      //Standard, should apply across all C compilers.\n");
    #ifdef __STDC__
        printf("      #define NIMEX_ANSI_C_COMPILER 1 //Compiler used claims to be ANSI C compliant.\n");
    #else
        printf("      #define NIMEX_ANSI_C_COMPILER 0 //Compiler used is not ANSI C compliant.\n");
    #endif

    #ifdef _MSC_VER
        printf("      //Microsoft's cl\n");
        printf("      #define NIMEX_MSC_VER 0x%8.8X //%d\n", _MSC_VER, _MSC_VER);
        printf("      #define NIMEX_MSC_FULL_VER 0x%8.8X //%d\n", _MSC_FULL_VER, _MSC_FULL_VER);
    #else
        printf("      //Microsoft's cl\n");
        printf("      #define NIMEX_MSC_VER 0x00000000 //0 //Not defined, defaulted to 0.\n");
        printf("      #define NIMEX_MSC_FULL_VER 0x00000000 //0 //Not defined, defaulted to 0.\n");
    #endif
    #ifdef __GNUC__
        printf("      //gcc\n"); 
        printf("      #define NIMEX_GNUC 0x%8.8X //%d\n", __GNUC__, __GNUC__);
        printf("      #define NIMEX_GNUC_MINOR 0x%8.8X //%d\n", __GNUC_MINOR__, __GNUC_MINOR__);
        printf("      #define NIMEX_GNUC_PATCHLEVEL 0x%8.8X //%d\n", __GNUC_PATCHLEVEL__, __GNUC_PATCHLEVEL__);
        #if __GNUC__ == 1
            #if __GNUC_MINOR__ == 34
              printf("//Easter egg found! It's time for Towers of Hanoi, or whatever else can be found.\n#pragma\n");
            #endif
        #endif
    #else
        printf("      //gcc\n");
        printf("      #define NIMEX_GNUC 0x00000000 //0 //Not defined, defaulted to 0.\n");
        printf("      #define NIMEX_GNUC_MINOR 0x00000000 //0 //Not defined, defaulted to 0.\n");
        printf("      #define NIMEX_GNUC_PATCHLEVEL 0x00000000 //0 //Not defined, defaulted to 0.\n");
    #endif
    #ifdef __LCC__
        printf("      //lcc\n");
        printf("      #define NIMEX_LCC 0x%8.8X //%d\n", __LCC__, __LCC__);
    #else
        printf("      //lcc\n");
        printf("      #define NIMEX_LCC 0x00000000 //0 //Not defined, defaulted to 0\n");
    #endif
    
    printf("\n");

    printf("#endif\n\n");

    printf("\n//Command issued to NIMEX_BuildInfo.exe to generate this file: `");
    for (i = 0; i < argc; i++)
        printf("%s ", argv[i]);
    printf("`\n\n");
    
    return 0;
}
