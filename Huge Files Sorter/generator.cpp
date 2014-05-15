#include <stdio.h>
#include <boost/random.hpp>
#include "limits.h"

int generate()
{
  boost::random::mt11213b gen;
  boost::random::uniform_int_distribution<unsigned int> dist(0, UINT_MAX);
  FILE* pFile;
  pFile = fopen("file.binary", "wb");
  unsigned int temp;
  for (unsigned int j = 0; j < 505000; ++j){
    //Some calculations to fill a[]
    temp = dist(gen);
    fwrite(&temp, 1, sizeof(unsigned int), pFile);
  }
  fclose(pFile);
  return 0;
}