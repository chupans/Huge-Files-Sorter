#pragma once
#include <vector>
#include <string>
#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>

// Check windows

#if WIN64
#define ENVIRONMENT64 64
#else
#define ENVIRONMENT32 32
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64 64
#else
#define ENVIRONMENT32 32
#endif
#endif

class Utility
{
public:
  static size_t AllocateBuffer( size_t desiredSize, std::vector<unsigned int> *pBuffer);  //Tries to allocate buffer long enough to store certain number of values. Return value - allocated buffer size
  static boost::uintmax_t GetFreeSystemMemory();
  static size_t ConvertLong(boost::uintmax_t val);
  static std::string GetNextTempFileName();
  static unsigned int GetCoresCount();
  static uintmax_t GetFileValuesCount(boost::filesystem::path filePath);
};





