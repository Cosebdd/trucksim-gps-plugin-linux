#ifndef SHAREDMEMORY_HPP
#define SHAREDMEMORY_HPP
#ifdef _WIN32
#include <windows.h>
typedef LPCWSTR shm_name_t;
#else
typedef const char* shm_name_t;
#endif
#include <stdio.h>
#include <stdlib.h>
#include "scs-telemetry-common.hpp"

#undef SHAREDMEM_LOGGING
#if ETS2_PLUGIN_LOGGING_ON == 1
	#if ETS2_PLUGIN_LOGGING_SHAREDMEMORY == 1
		#define SHAREDMEM_FILENAME ETS2_PLUGIN_FILENAME_PREFIX "sharedmem.txt"
		#define SHAREDMEM_LOGGING 1
	#endif
#endif

class SharedMemory
{
protected:

        shm_name_t namePtr;
        int mapsize;

		// MMF specifics
#ifdef _WIN32
        HANDLE hMapFile;
#else
        int shmFd;
#endif
        void* pBufferPtr;

		// Status about hook
        bool isSharedMemoryHooked;

		// Logging
#ifdef SHAREDMEM_LOGGING
		FILE *logFilePtr;
#endif

        void LogError(const char* logPtr);

public:
        bool Hooked() { return isSharedMemoryHooked; }
        void* GetBuffer() { return pBufferPtr; }

        SharedMemory(shm_name_t newNamePtr, unsigned int size);
        void Close();

		void* getPtrAt(int offset) { return (void*) &(((unsigned char*)pBufferPtr)[offset]); }


};

#endif
