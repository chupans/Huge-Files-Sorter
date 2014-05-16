#include "Memory.h"
#include <boost/predef.h>


DWORD FindLargestSizeOfMemory()
{

  MEMORY_BASIC_INFORMATION mbi;
  DWORD start = 0;
  bool recording = false;
  DWORD freestart = 0, largestFreestart = 0;
  __int64 free = 0, largestFree = 0;

  while (true)
  {
    SIZE_T s = VirtualQuery((LPCVOID)start, &mbi, sizeof(mbi));
    if (s != sizeof(mbi)) break;

    if (mbi.State == MEM_FREE)
    {
      if (!recording) freestart = start;

      free += mbi.RegionSize;
      recording = true;
    }
    else
    {
      if (recording)
      {
        if (free > largestFree)
        {
          largestFree = free;
          largestFreestart = freestart;
        }
      }
      free = 0;
      recording = false;
    }
    start += mbi.RegionSize;
  }

  return largestFree;
}

#ifdef BOOST_OS_WINDOWS
#include <windows.h>

size_t getTotalSystemMemory()
{
  MEMORYSTATUSEX status;
  status.dwLength = sizeof(status);
  GlobalMemoryStatusEx(&status);
  return status.ullAvailPhys;
}
#elif BOOST_OS_UNIX
#include <unistd.h>

size_t getTotalSystemMemory()
{
  long pages = sysconf(_SC_PHYS_PAGES);
  long page_size = sysconf(_SC_PAGE_SIZE);
  return pages * page_size;
}

#else
#error Not supported OS
#endif