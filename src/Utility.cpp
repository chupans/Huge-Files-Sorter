#include "Utility.h"
#include "boost\predef.h"

#ifdef BOOST_OS_WINDOWS
#include <windows.h>

size_t Utility::GetFreeSystemMemory()
{
  MEMORYSTATUSEX status;
  status.dwLength = sizeof(status);
  GlobalMemoryStatusEx(&status);
  return (size_t)status.ullAvailPhys;
}
#elif BOOST_OS_UNIX
#include <unistd.h>

size_t Utility::GetFreeSystemMemory()
{
#error Not a correct way to check free available memory size
  long pages = sysconf(_SC_PHYS_PAGES);
  long page_size = sysconf(_SC_PAGE_SIZE);
  return pages * page_size;
}

#else
#error Not supported OS
#endif


size_t Utility::AllocateBuffer( size_t desiredSize, std::vector<unsigned int> *pBuffer )
{
  while (pBuffer->capacity() != desiredSize)
  {
    try
    {
      pBuffer->reserve(desiredSize);
    }
    catch (std::bad_alloc& ba)
    {
      desiredSize*= 0.9f;
    }
  }
  return desiredSize;
}
