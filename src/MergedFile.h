#pragma once
#include "HugeFile.h"

//Structure used to help merging. Represents kind of file stream
struct CMergedFile
{
public:
  CMergedFile(const char *filePath);
  void ReadNextVal();
  unsigned int GetVal();
  bool IsEof();

private:
  CHugeFile file;
  unsigned int val;
  bool ended;
};
