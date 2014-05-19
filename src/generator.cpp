#include <stdio.h>
#include <boost/random.hpp>
#include "limits.h"

int generate(int megabytes, char *file)
{
  boost::random::mt11213b gen;
  boost::random::uniform_int_distribution<unsigned int> dist(0, UINT_MAX);
  FILE* pFile;
  pFile = fopen(file, "wb");
  unsigned int temp;
  for (int i = 0; i < megabytes; i++)
  {
    for (unsigned int j = 0; j < 1024*1024 / 4; ++j)
    {
      temp = dist(gen);
      fwrite(&temp, 1, sizeof(unsigned int), pFile);
    }
  }
  fclose(pFile);
  return 0;
}