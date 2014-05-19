#include "MergedFile.h"



CMergedFile::CMergedFile( const char *filePath )
{
  file.OpenForReading(filePath);
  ended = false;
  val = 0;
  ReadNextVal();
}

void CMergedFile::ReadNextVal()
{
  if (file.GetNextVal(val) == false) 
    ended = true; 
}

unsigned int CMergedFile::GetVal()
{
  return val;
}

bool CMergedFile::IsEof()
{
  return ended;
}
