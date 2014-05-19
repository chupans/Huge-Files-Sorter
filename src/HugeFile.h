#pragma once
#include "boost/filesystem.hpp"
#include "boost/predef.h"
#include <vector>

#ifdef BOOST_OS_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "windows.h"
#include <stdio.h>
#elif BOOST_OS_UNIX
#define _FILE_OFFSET_BITS  64
#include <stdio.h>
#else
#error Not supported OS
#endif


#define INTERNAL_BUFFER_SIZE (2 * 1024 * 1024)

//IO operations wrapper 
class CHugeFile
{
public:
  CHugeFile();
  ~CHugeFile(void);
  void CreateAndOpenForWriting(boost::filesystem::path path);
  void OpenForReading(boost::filesystem::path path);
  void CloseFile();
  uintmax_t ReadValuesIntoBuffer(std::vector<unsigned int> &buffer);
  bool GetNextVal(unsigned int &val);
  void WriteBufferIntoFile(std::vector<unsigned int> &buffer);

private:
#ifdef BOOST_OS_WINDOWS
  HANDLE m_hFile;
  std::vector<unsigned int> m_internalBuffer;
  size_t m_bufValPos;   //Internal read buffer position
  bool m_readFromBuf;   //Flag indicating if new values should be fetched to buffer
  bool m_eofHit;        //Flag indicating end of file we reading
#elif BOOST_OS_UNIX
  FILE *m_pFile;
#else
#error Not supported OS
#endif
};
