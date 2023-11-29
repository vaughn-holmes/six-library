#pragma once

#include "config/compiler_extensions.h"

// Need to specify how this code will be consumed, either NITRO_NITFCPP_LIB_ (static library)
// or NITRO_NITFCPP_DLL_ (aka "shared" library).  For DLLs, it needs to be set for BOTH
// "exporting" (building this code) and "importing" (consuming).
//
// Use Windows naming conventions (DLL, LIB) because this really only matters for _MSC_VER, see below.
#if !defined(NITRO_NITFCPP_LIB_) && !defined(NITRO_NITFCPP_DLL_)
    #if CODA_OSS_LIBRARY_SHARED
        #if CODA_OSS_LIBRARY_STATIC
            #error "CODA_OSS_LIBRARY_SHARED already #define'd'"
        #endif
        #define NITRO_NITFCPP_DLL_ 1  // Symbols must be exported and imported (see below).
    #else
        // CODA_OSS_LIBRARY_STATIC doesn't have to be defined
        #define NITRO_NITFCPP_LIB_ 1  // Static library, all symbols visible.
    #endif
#endif
#if !defined(NITRO_NITFCPP_LIB_) && !defined(NITRO_NITFCPP_DLL_)
    #error "One of NITRO_NITFCPP_LIB_ pr NITRO_NITFCPP_DLL_ must be #define'd'"
#endif
#if defined(NITRO_NITFCPP_LIB_) && defined(NITRO_NITFCPP_DLL_)
    #error "Both NITRO_NITFCPP_LIB_ and NITRO_NITFCPP_DLL_ are #define'd'"
#endif
#if defined(NITRO_NITFCPP_EXPORTS) && defined(NITRO_NITFCPP_LIB_)
    #error "Can't export from a LIB'"
#endif

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the NITRO_NITFCPP_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// NITRO_NITFCPP_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
// https://www.gnu.org/software/gnulib/manual/html_node/Exported-Symbols-of-Shared-Libraries.html
#ifdef NITRO_NITFCPP_EXPORTS
    #define NITRO_NITFCPP_API CODA_OSS_library_export
#else
    // Either building a static library (no NITRO_NITFCPP_EXPORTS) or
    // importing (not building) a shared library.

    // We need to know whether we're consuming (importing) a DLL or static LIB
    // The default is a static LIB as that's what existing code/builds expect.
    #ifdef NITRO_NITFCPP_DLL_
        // Actually, it seems that the linker is able to figure this out from the .LIB, so 
        // there doesn't seem to be a need for __declspec(dllimport).  Clients don't
        // need to #define NITRO_NITFCPP_DLL_ ... ?  Well, almost ... it looks
        // like __declspec(dllimport) is needed to get virtual "inline"s (e.g., destructors) correct.
        #define NITRO_NITFCPP_API CODA_OSS_library_import
    #else
        #define NITRO_NITFCPP_API /* "importing" a static LIB */
    #endif
#endif

#if defined(_MSC_VER)
#pragma warning(disable: 4251) // '...' : class '...' needs to have dll-interface to be used by clients of struct '...'
#endif

