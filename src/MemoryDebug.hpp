//
// Created by Candy on 1/20/2026.
//

#ifndef B_WENGINE_MEMORYDEBUG_HPP
#define B_WENGINE_MEMORYDEBUG_HPP

#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#endif

inline void PrintMemoryStatus()
{
#ifdef _WIN32
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);

    if (GlobalMemoryStatusEx(&statex))
    {
        std::cout << "[Memory] Avail Phys: "
                  << (statex.ullAvailPhys / (1024ull * 1024ull))
                  << " MB\n";

        std::cout << "[Memory] Avail Virtual: "
                  << (statex.ullAvailVirtual / (1024ull * 1024ull))
                  << " MB\n";
    }
    else
    {
        std::cout << "[Memory] GlobalMemoryStatusEx failed\n";
    }
#else
    std::cout << "[Memory] PrintMemoryStatus only implemented for Windows right now.\n";
#endif
}

#endif //B_WENGINE_MEMORYDEBUG_HPP