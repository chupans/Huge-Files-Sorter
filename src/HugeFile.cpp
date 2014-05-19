#include "HugeFile.h"
#include "Utility.h"
#include <algorithm>
#include <assert.h>

using namespace boost;
using namespace filesystem;
using namespace std;

#ifdef BOOST_OS_WINDOWS
#define DWORD_MAX 0xffffffffUL
#define MAX_BUFFER_TO_WRITE DWORD_MAX

CHugeFile::CHugeFile()
{
  m_bufValPos = 0;
  m_readFromBuf = true;
  m_eofHit = false;
}

CHugeFile:: ~CHugeFile()
{
  if (m_hFile != NULL)
    CloseHandle(m_hFile);
}


void CHugeFile::CloseFile()
{
  if (m_hFile != NULL)
  {
    CloseHandle(m_hFile);
    m_hFile = NULL;
  }
}

void CHugeFile::CreateAndOpenForWriting(path filePath)
{
  m_hFile = CreateFile(filePath.wstring().data(),
                       GENERIC_WRITE,
                       NULL,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
}

void CHugeFile::OpenForReading(path filePath)
{
  m_hFile = CreateFile(filePath.wstring().data(),
                       GENERIC_READ,
                       NULL,
                       NULL,
                       OPEN_EXISTING,         
                       FILE_ATTRIBUTE_NORMAL, 
                       NULL);   
  auto newSize = Utility::AllocateBuffer(INTERNAL_BUFFER_SIZE, &m_internalBuffer);
  m_internalBuffer.resize(newSize);
}

uintmax_t CHugeFile::ReadValuesIntoBuffer( std::vector<unsigned int> &buffer )
{
  DWORD NumberOfBytesRead = 0;
  uintmax_t nBytesToRead = sizeof(unsigned int) * buffer.size();
  uintmax_t nOverallBytesRead = 0;
  unsigned int *pData = &buffer[0];

  if (buffer.size() == 0)
    return 0;

  //Reading more bytes than DWORD can handle
  while (nBytesToRead > 0)
  {
    ReadFile(m_hFile, pData, std::min(nBytesToRead, (boost::uintmax_t)MAX_BUFFER_TO_WRITE), &NumberOfBytesRead, NULL);
    pData += NumberOfBytesRead / sizeof(unsigned int);
    nBytesToRead -= NumberOfBytesRead;
    nOverallBytesRead += NumberOfBytesRead;
    if (NumberOfBytesRead == 0)
      return nOverallBytesRead / sizeof(unsigned int);
  }
  return nOverallBytesRead / sizeof(unsigned int); 
}

bool CHugeFile::GetNextVal(unsigned int &val)
{
  if (m_bufValPos == m_internalBuffer.size())
  {
    if (m_eofHit)
      return false;
    else
      m_readFromBuf = true;
  }

  if (m_readFromBuf)
  {
    DWORD NumberOfBytesRead = 0;
    m_bufValPos = 0;
    m_readFromBuf = false;
    auto readRet = ReadFile(m_hFile, &m_internalBuffer[0], sizeof(unsigned int) * m_internalBuffer.size(), &NumberOfBytesRead, NULL);
    size_t nValuesRead = (size_t)NumberOfBytesRead / sizeof(unsigned int);
    if (nValuesRead != m_internalBuffer.size() && readRet == TRUE)
      m_eofHit = true;
    m_internalBuffer.resize(nValuesRead);
  }


  if (m_bufValPos < m_internalBuffer.size())
  {
    val = m_internalBuffer[m_bufValPos];
    m_bufValPos++;
    return true;
  }
  else
  {
    return false;
  }
}

void CHugeFile::WriteBufferIntoFile(vector<unsigned int> &buffer)
{
  DWORD NumberOfBytesWritten = 0;
  uintmax_t nBytesToWrite = sizeof(unsigned int) * buffer.size();
  unsigned int *pData = &buffer[0];

  //Writing more bytes than DWORD can handle
  while (nBytesToWrite > 0)
  {
    WriteFile(m_hFile, pData, std::min(nBytesToWrite, (boost::uintmax_t)MAX_BUFFER_TO_WRITE), &NumberOfBytesWritten, NULL);
    if (NumberOfBytesWritten == 0)
      return;
    pData += NumberOfBytesWritten / sizeof(unsigned int);
    nBytesToWrite -= NumberOfBytesWritten;
    printf("Wrote %u MB of buffer sized %u MB\n", (unsigned int)NumberOfBytesWritten/(1024*1024), buffer.size()/(256*1024));
  }
}

#elif #ifdef BOOST_OS_LINUX

CHugeFile::CHugeFile()
{
}
CHugeFile:: ~CHugeFile()
{

}
void CHugeFile::CreateAndOpenForWriting(path filePath)
{

}
void CHugeFile::OpenForReading(path filePath)
{

}
uintmax_t CHugeFile::ReadValuesIntoBuffer(vector<unsigned int> &buffer)
{
  return 0;
}
void CHugeFile::WriteBufferIntoFile(vector<unsigned int> &buffer)
{

}
void CHugeFile::WriteValueIntoFile(unsigned int val)
{

}

#endif