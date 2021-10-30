#pragma once

#if !defined(DLLIMPORT)
    #if !defined(HOT_RELOAD)
        #define DLLIMPORT
    #else
        #if defined(_WIN32) || defined(__CYGWIN__)
            #if defined(__GNUC__)
                #define DLLIMPORT __attribute__((dllimport))
            #else
                #define DLLIMPORT __declspec(dllimport)
            #endif
        #else
            #if defined(__GNUC__) && __GNUC__ >= 4
                #define DLLIMPORT
            #else
                #define DLLIMPORT
            #endif
        #endif
    #endif
#endif

#if !defined(DLLEXPORT)
    #if !defined(HOT_RELOAD)
        #define DLLEXPORT
    #else
        #if defined(_WIN32) || defined(__CYGWIN__)
            #if defined(__GNUC__)
                #define DLLEXPORT __attribute__((dllexport))
            #else
                #define DLLEXPORT __declspec(dllexport)
            #endif
        #else
            #if defined(__GNUC__) && __GNUC__ >= 4
                #define DLLEXPORT __attribute__((visibility("default")))
            #else
                #define DLLEXPORT
            #endif
        #endif
    #endif
#endif