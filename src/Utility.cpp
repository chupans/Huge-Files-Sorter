#include "Utility.h"
#include "boost/predef.h"
#include <boost/thread.hpp>
#include <limits>

#define NOMINMAX

#ifdef BOOST_OS_WINDOWS
#include <windows.h>

uintmax_t Utility::GetFreeSystemMemory()
{
  MEMORYSTATUSEX status;
  status.dwLength = sizeof(status);
  GlobalMemoryStatusEx(&status);
  return status.ullAvailPhys;
}
#elif BOOST_OS_UNIX
#include <unistd.h>

uintmax_t Utility::GetFreeSystemMemory()
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

size_t Utility::ConvertLong( boost::uintmax_t val )
{
  size_t retVal;
  retVal = (size_t) val > (std::numeric_limits<size_t>::max()) ? (std::numeric_limits<size_t>::max()) : val;
  return retVal;
}

std::string Utility::GetNextTempFileName()
{
  //TODO: check if file with this name doesn't exist
  char buf[20];
  static int i = 0;
  sprintf(buf, "temp%d", i);
  i++;
  std::string filename = buf;
  return filename;
}

unsigned int Utility::GetCoresCount()
{
#ifdef ENVIRONMENT32
  return 1;
#elif ENVIRONMENT64
  return boost::thread::hardware_concurrency();
#else
#error wasnt able to check environment
#endif
}

boost::uintmax_t Utility::GetFileValuesCount( boost::filesystem::path filePath )
{
  return file_size(filePath) / sizeof(unsigned int);
}


