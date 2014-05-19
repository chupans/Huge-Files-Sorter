#pragma once
#include <vector>
class Utility
{
public:
  static size_t AllocateBuffer( size_t desiredSize, std::vector<unsigned int> *pBuffer);
  static size_t GetFreeSystemMemory();
};

