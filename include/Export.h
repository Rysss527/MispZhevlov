#pragma once

#ifdef _WIN32
    #ifdef DEFEN_SYNTH_EXPORTS
        #define DEFEN_SYNTH_API __declspec(dllexport)
    #else
        #define DEFEN_SYNTH_API
    #endif
#else
    #define DEFEN_SYNTH_API
#endif